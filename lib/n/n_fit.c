
/* $Id: n_fit.c 16766 2014-05-09 23:26:39Z ksimek $ */

/* =========================================================================== *
|
|  Copyright (c) 1994-2008 by Kobus Barnard (author).
|
|  Personal and educational use of this code is granted, provided that this
|  header is kept intact, and that the authorship is not misrepresented, that
|  its use is acknowledged in publications, and relevant papers are cited.
|
|  For other use contact the author (kobus AT cs DOT arizona DOT edu).
|
|  Please note that the code in this file has not necessarily been adequately
|  tested. Naturally, there is no guarantee of performance, support, or fitness
|  for any particular task. Nonetheless, I am interested in hearing about
|  problems that you encounter.
|
* =========================================================================== */

#include "n/n_gen.h"      /*  Only safe if first #include in a ".c" file  */

#include "n/n_fit.h"
#include "n/n_invert.h"
#include "n/n_svd.h"

/* For declared function pointers. Generally harmless. */
#ifdef __cplusplus
extern "C" {
#endif

/* -------------------------------------------------------------------------- */

#define MAX_NUM_TWO_MODE_ITERATIONS 100
#define TWO_MODE_CONVERGENCE_THRESHOLD  0.0001

#define TRY_IT_DIFFERENTLY

/* -------------------------------------------------------------------------- */

static int get_best_post_map_guts
(
    Matrix**      X_mpp,
    const Matrix* A_mp,
    const Matrix* B_mp
);

static int pseudo_inverse_least_squares
(
    Matrix**      X_mpp,
    const Matrix* mp,
    const Matrix* vp
);

static int svd_least_squares
(
    Matrix**      X_mpp,
    const Matrix* A_mp,
    const Matrix* B_mp
);

#ifdef DO_BIMODAL_ANALYSIS
static int do_two_mode_step
(
    Matrix**      basis_mpp,
    const Matrix* response_mp,
    const Matrix* cur_basis_mp,
    int           num_basis_vectors
);

static int get_two_mode_transpose(Matrix** target_mpp, Matrix* mp);

static int get_single_mode_basis_for_rows
(
    Matrix**      basis_mpp,
    const Matrix* response_mp,
    int           num_basis_rows
);

static int compute_two_mode_error
(
    const Matrix* P_mp,
    int           num_A_basis_vectors,
    const Matrix* A_mp,
    const Matrix* A_basis_mp,
    int           num_B_basis_vectors,
    const Matrix* B_mp,
    const Matrix* B_basis_mp,
    double*       total_error_ptr
);
#endif

/* -------------------------------------------------------------------------- */

static Method_option fs_least_squares_methods[ ] =
{
    { "svd",            "svd", (int(*)())svd_least_squares },
    { "pseudo-inverse", "pi",  (int(*)())pseudo_inverse_least_squares}
};

static const int fs_num_least_squares_methods =
                                sizeof(fs_least_squares_methods) /
                                          sizeof(fs_least_squares_methods[ 0 ]);
static int         fs_least_squares_method                  = 0;
static const char* fs_least_squares_method_option_short_str = "least-squares";
static const char* fs_least_squares_method_option_long_str  =
                                                         "least-squares-method";


/* -------------------------------------------------------------------------- */

int set_least_squares_options(const char* option, const char* value)
{
    char lc_option[ 100 ];
    int  result           = NOT_FOUND;


    EXTENDED_LC_BUFF_CPY(lc_option, option);

    if (    (lc_option[ 0 ] == '\0')
          || match_pattern(lc_option, fs_least_squares_method_option_short_str)
          || match_pattern(lc_option, fs_least_squares_method_option_long_str)
       )
    {
        if (value == NULL) return NO_ERROR;

        ERE(parse_method_option(fs_least_squares_methods,
                                fs_num_least_squares_methods,
                                fs_least_squares_method_option_long_str,
                                "least squares method", value,
                                &fs_least_squares_method));
        result = NO_ERROR;
    }

    return result;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                           least_squares
 *
 * Solves Ax=b in the least squares sense.
 *
 * This routine solves Ax=b in the least squares sense. 
 *
 * It is like least_squares_2() except that the fitting error is not returned. 
 *
 * Put differently, this routine calculates
 * |
 * |  MIN { || Ax - b ||  }
 * |   x                2
 * |
 *
 * The result is put into the vector pointed to by *result_vpp, which is
 * created if it is NULL, resized if it is the wrong sized, and reused
 * otherwise. The number of rows in the matrix pointed to by the argument "mp"
 * (i.e, A, above) must be the same as the length of the vector pointed to by
 * the argument "vp" (i.e. b, above). In addition, the number of columns of
 * of *mp (i.e, number of unknowns) cannot exceed the number of rows of *mp
 * (number of equations).
 *
 * This routine relies on the user settable run-time option
 * "least-squares-method" which currently has two possible values ("svd", or
 * "pseudo-inverse") with "svd" being the default. However, neither of these is
 * likely what we want to do for large problems. (UNDER CONSTRUCTION). 
 *
 * Returns:
 *     NO_ERROR on success, and ERROR on failure, with an appropriate error
 *     message being set.
 *
 * Index: least squares, data fitting, optimization
 *
 * -----------------------------------------------------------------------------
*/

int least_squares
(
    Vector**      result_vpp,
    const Matrix* A_mp,
    const Vector* b_vp
)
{
    return least_squares_2(result_vpp, A_mp, b_vp, NULL);
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                           least_squares_2
 *
 * Solves Ax=b in the least squares sense.
 *
 * This routine solves Ax=b in the least squares sense. 
 *
 * It is like least_squares() except that it allows for returning fitting error.
 *
 * Put differently, this routine calculates
 * |
 * |  MIN { || Ax - b ||  }
 * |   x                2
 * |
 *
 * The result is put into the vector pointed to by *result_vpp, which is
 * created if it is NULL, resized if it is the wrong sized, and reused
 * otherwise. The number of rows in the matrix pointed to by the argument "mp"
 * (i.e, A, above) must be the same as the length of the vector pointed to by
 * the argument "vp" (i.e. b, above). In addition, the number of columns of
 * of *mp (i.e, number of unknowns) cannot exceed the number of rows of *mp
 * (number of equations).
 *
 * If the argument error_ptr is not NULL, then the RMS fitting error is returned
 * via that pointer.
 * 
 * Returns:
 *     NO_ERROR on success, and ERROR on failure, with an appropriate error
 *     message being set.
 *
 * Index: least squares, data fitting, optimization
 *
 * -----------------------------------------------------------------------------
*/

int least_squares_2
(
    Vector**      result_vpp,
    const Matrix* A_mp,
    const Vector* b_vp,
    double*       error_ptr
)
{
    int     result;
    int     num_eq;
    int     num_unknowns;
    Matrix* B_mp         = NULL;
    Matrix* X_mp         = NULL;


    num_eq = A_mp->num_rows;
    num_unknowns = A_mp->num_cols;

    if (    (fs_least_squares_method < 0)
         || (fs_least_squares_method >= fs_num_least_squares_methods)
       )
    {
        SET_CANT_HAPPEN_BUG();
        result = ERROR;
    }
    else if (num_eq != b_vp->length)
    {
        add_error("Argument error in least_squares");
        set_error("The number of matrix rows does not match vector length.");
        result = ERROR;
    }
    else if (num_unknowns > num_eq)
    {
        set_bug("More unknowns than equations in least_squares.");
        result = ERROR;
    }
    else
    {
        result = get_target_matrix(&B_mp, b_vp->length, 1);

        if (result != ERROR)
        {
            result = put_matrix_col(B_mp, b_vp, 0);
        }

        if (result != ERROR)
        {
            result = get_best_post_map_guts(&X_mp, A_mp, B_mp);
        }

        if (result != ERROR)
        {
            result = get_matrix_col(result_vpp, X_mp, 0);
        }

        if (result == ERROR)
        {
            add_error("Error occured solving Ax=b in least squares sense.");
            add_error("[ A is %d by %d. ]", A_mp->num_rows, A_mp->num_cols);
        }
    }

    free_matrix(B_mp);
    free_matrix(X_mp);

    if (result == NO_ERROR) 
    {
        /* Null error_ptr is OK. */
        result = get_linear_equation_rms_error(A_mp, *result_vpp, b_vp, error_ptr);
    }

    return result;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                           get_linear_equation_rms_errror
 *
 * Returns the RMS error in Ax~b
 *
 * This routine computes the RMS error between Ax and b. This error is what is
 * minimized by least squares fitting.
 *
 * Returns:
 *     NO_ERROR on success, and ERROR on failure, with an appropriate error
 *     message being set.
 *
 * Index: least squares, data fitting, optimization
 *
 * -----------------------------------------------------------------------------
*/

int get_linear_equation_rms_error
(
    const Matrix* a_mp,
    const Vector* x_vp,
    const Vector* b_vp,
    double*       error_ptr 
)
{
    Vector* est_vp = NULL; 

    if (error_ptr != NULL) 
    {
        ERE(multiply_matrix_and_vector(&est_vp, a_mp, x_vp));

        *error_ptr = rms_absolute_error_between_vectors(est_vp, b_vp);

        free_vector(est_vp);
    }

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                           get_best_diagonal_post_map
 *
 * Finds the best post diagonal map between two matrices
 *
 * This routine finds the diagonal matrix which, when used to post multiply the
 * matrix pointed to by in_mp, gives the closest matrix in the least squares
 * sense to the matrix pointed to by out_mp.
 *
 * Put differently, this routine calculates
 * |
 * |  MIN { || AX - B ||   }, over all diagonal matrices, X
 * |                    2
 * |
 *
 * The diagonal map is put into the vector pointed to by *result_vpp, which
 * is created if *result_vpp is NULL, resized if *result_vpp is the wrong
 * sized, and reused otherwise.  The dimensions of *in_mp and *out_mp must be
 * the same.
 *
 * Returns:
 *     NO_ERROR on success, and ERROR on failure, with an appropriate error
 *     message being set.
 *
 * Index: least squares, optimization, data fitting
 *
 * -----------------------------------------------------------------------------
*/

int get_best_diagonal_post_map
(
    Vector**      result_vpp,
    const Matrix* in_mp,
    const Matrix* out_mp
)
{
    int     num_rows, num_cols;
    Vector* result_vp;
    double* result_pos;
    double  sum1;
    double  sum2;
    int     i, j;
    double  temp1;
    double  temp2;


    ERE(check_same_matrix_dimensions(in_mp, out_mp,
                                     "get_best_diagonal_post_map"));

    ERE(get_target_vector(result_vpp, in_mp->num_cols));
    result_vp = *result_vpp;

    num_rows = in_mp->num_rows;
    num_cols = in_mp->num_cols;

    result_pos = result_vp->elements;

    for (j=0; j<num_cols; j++)
    {
        sum1 = 0.0;
        sum2 = 0.0;

        for (i=0; i<num_rows; i++)
        {
            temp1 = (in_mp->elements)[i][j];
            temp2 = (out_mp->elements)[i][j];

            sum1 += temp1 * temp2;
            sum2 += temp1 * temp1;

        }

#ifdef USE_IS_ZERO_DBL_FOR_DIVIDE_BY_ZERO
        if ( ! IS_POSITIVE_DBL(sum2) )
        {
            set_error("Attempt to find a transformation from a zero vector.");
            return ERROR;
        }
#else
        /* Exact test is OK, because case of small sum2 is handled below. */
        if (sum2 == 0.0)
        {
            set_error("Attempt to find a transformation from a zero vector.");
            return ERROR;
        }
#endif

        *result_pos = sum1 / sum2;

#ifdef USE_COMPARE_WITH_REAL_MAX_FOR_DIVIDE_BY_ZERO
        if (    (*result_pos > DBL_HALF_MOST_POSITIVE)
             || (*result_pos < DBL_HALF_MOST_NEGATIVE)
           )
        {
            set_error("Problem computing a best post diagonal map");
            add_error("Overflow occured dividing %.2e by %.2e.",
                      sum1, sum2);
            return ERROR;
        }
#endif
        result_pos++;
    }

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                             get_diagonal_post_map_error
 *
 * Finds the error of a post diagonal map between two matrices
 *
 * This routine finds the error incurred when the input diagonal map is used to
 * post multiply the matrix pointed to by in_mp, as an approximation of the
 * matrix pointed to by out_mp.
 *
 * Put differently, this routine calculates
 * |
 * |       || (*in_mp) x diag(*diag_trans_vp) - (*out_mp) ||
 * |
 *
 * The result is put into the argument *error_ptr.
 *
 * Returns:
 *     NO_ERROR on success, and ERROR on failure, with an appropriate error
 *     message being set.
 *
 * Index: least squares, optimization, data fitting
 *
 * -----------------------------------------------------------------------------
*/

int get_diagonal_post_map_error
(
    const Vector* diag_trans_vp,
    const Matrix* in_mp,
    const Matrix* out_mp,
    double*       error_ptr
)
{
    int    num_rows, num_cols;
    double sum;
    int    i, j;
    double temp1;
    double temp2;


    ERE(check_same_matrix_dimensions(in_mp, out_mp,
                                     "get_diagonal_post_map_error"));

    if (diag_trans_vp->length != in_mp->num_cols)
    {
        set_bug("Incorrect vector length in get_diagonal_post_map_error.");
        return ERROR;
    }

    num_rows = in_mp->num_rows;
    num_cols = in_mp->num_cols;

    sum = 0.0;

    for (i=0; i<num_rows; i++)
    {
        for (j=0; j<num_cols; j++)
        {

            temp1 = (in_mp->elements)[i][j] * (diag_trans_vp->elements)[j];
            temp2 = (out_mp->elements)[i][j] - temp1;

            sum += temp2 * temp2;
        }
     }

     sum /= (num_rows * num_cols);

     *error_ptr = sqrt(sum);

     return NO_ERROR;
 }

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

int get_diagonal_post_map_relative_row_error
(
    const Vector* diag_trans_vp,
    const Matrix* in_mp,
    const Matrix* out_mp,
    double*       error_ptr
)
{
    Matrix* result_mp = NULL;
    int     result;


    UNTESTED_CODE();

    ERE(multiply_matrix_by_row_vector_ew(&result_mp, in_mp, diag_trans_vp));

    result = get_rms_relative_row_error((Vector**)NULL, result_mp, out_mp,
                                        error_ptr);

    free_matrix(result_mp);

    return result;
 }

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                                 get_best_map
 *
 * Finds the best map between two matrices
 *
 * This routine finds the matrix which, when multiplied by the matrix pointed to
 * by in_mp, gives the closest matrix in the least squares sense to the matrix
 * pointed to by out_mp.
 *
 * Put differently, this routine calculates
 * |
 * |  MIN { || XA - B ||   }, over all matrices, X
 * |                    2
 * |
 *
 * The map is put into the matrix pointed to by *best_map_mpp, which is
 * created if it is NULL, resized if it is the wrong sized, and reused
 * otherwise.  The dimensions of *in_mp and *out_mp must be the same.
 *
 * Returns:
 *     NO_ERROR on success, and ERROR on failure, with an appropriate error
 *     message being set.
 *
 * Index: least squares, optimization, data fitting
 *
 * -----------------------------------------------------------------------------
*/

int get_best_map
(
    Matrix**      best_map_mpp,
    const Matrix* in_mp,
    const Matrix* out_mp
)
{
    Matrix* in_trans_mp;
    Matrix* out_trans_mp;
    Matrix* best_map_trans_mp = NULL;
    int     result = NO_ERROR;


    ERE(check_same_matrix_dimensions(in_mp, out_mp, "get_best_map"));

    NRE(in_trans_mp = create_transpose(in_mp));

    if ((out_trans_mp = create_transpose(out_mp)) == NULL)
    {
        result = ERROR;
    }

    if (result != ERROR)
    {
        result = get_best_post_map(&best_map_trans_mp, in_trans_mp,
                                   out_trans_mp);
    }

    if (result != ERROR)
    {
        result = get_transpose(best_map_mpp, best_map_trans_mp);
    }

    free_matrix(in_trans_mp);
    free_matrix(out_trans_mp);
    free_matrix(best_map_trans_mp);

    if (result == ERROR)
    {
        add_error("Error occurred computing best post linear map.");
        add_error("Matrix to be mapped is %d by %d.", in_mp->num_rows,
                  in_mp->num_cols);
    }

    return result;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                           get_best_post_map
 *
 * Finds the best post map between two matrices
 *
 * This routine finds the matrix which, when used to post multiply the matrix
 * pointed to by in_mp, gives the closest matrix in the least squares sense to
 * the matrix pointed to by out_mp.
 *
 * Put differently, this routine calculates
 * |
 * |  MIN { || AX - B ||   }, over all matrices, X
 * |                    2
 *
 * The map is put into the matrix pointed to by *best_map_mpp, which is
 * created if it is NULL, resized if it is the wrong sized, and reused
 * otherwise.  The dimensions of *in_mp and *out_mp must be the same.
 *
 * Returns:
 *     NO_ERROR on success, and ERROR on failure, with an appropriate error
 *     message being set.
 *
 * Index: least squares, optimization, data fitting
 *
 * -----------------------------------------------------------------------------
*/

int get_best_post_map
(
    Matrix**      best_map_mpp,
    const Matrix* in_mp,
    const Matrix* out_mp
)
{
    Matrix*       in_MP_inv_mp          = NULL;
    int           num_rows              = in_mp->num_rows;
    int           num_cols              = in_mp->num_cols;
    int           i;
    int           result                = NO_ERROR;
    Matrix*       constructed_in_mp     = NULL;
    Matrix*       constructed_out_mp    = NULL;
    const Matrix* non_degenerate_in_mp;
    const Matrix* non_degenerate_out_mp;


    ERE(check_same_matrix_dimensions(in_mp, out_mp, "get_best_post_map"));

    if (    (num_rows == 1)
         && (min_abs_matrix_element(in_mp) > DBL_EPSILON)
       )
    {
        /* This used to be get_best_map, now we are switching rows for cols. */
        UNTESTED_CODE();

        ERE(get_zero_matrix(best_map_mpp, num_cols, num_cols));

        for (i=0; i<num_cols; i++)
        {
            (*best_map_mpp)->elements[ i ][ i ] =
                out_mp->elements[ 0 ][ i ] / in_mp->elements[ 0 ][ i ];
        }

        return NO_ERROR;

    }
    else if (num_rows < num_cols)
    {
        /* This used to be get_best_map, now we are switching rows for cols. */
        UNTESTED_CODE();

        ERE(get_identity_matrix(&constructed_in_mp, num_cols));

        constructed_in_mp->num_rows = num_rows;
        result = copy_matrix(&constructed_in_mp, in_mp);
        constructed_in_mp->num_rows = num_cols;

        if (result != ERROR)
        {
            result = get_identity_matrix(&constructed_out_mp, num_cols);
        }

        if (result != ERROR)
        {
            constructed_out_mp->num_rows = num_rows;
            result = copy_matrix(&constructed_out_mp, out_mp);
            constructed_out_mp->num_rows = num_cols;
        }

        non_degenerate_out_mp = constructed_out_mp;
        non_degenerate_in_mp  = constructed_in_mp;
    }
    else
    {
        non_degenerate_out_mp = out_mp;
        non_degenerate_in_mp  = in_mp;
    }

    if (result != ERROR)
    {
        result = get_MP_inverse(&in_MP_inv_mp, non_degenerate_in_mp);
    }

    if (result != ERROR)
    {
        result = multiply_matrices(best_map_mpp, in_MP_inv_mp,
                                   non_degenerate_out_mp);
    }

    free_matrix(in_MP_inv_mp);
    free_matrix(constructed_in_mp);
    free_matrix(constructed_out_mp);

    if (result == ERROR)
    {
        add_error("Error occurred computing best linear map.");
        add_error("Matrix to be mapped is %d by %d.", in_mp->num_rows,
                  in_mp->num_cols);
    }

    return result;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                             get_post_map_error
 *
 * Finds the error of a post map between two matrices
 *
 * This routine finds the error incurred when the matrix pointed to by map_mp
 * is used to post multiply the matrix pointed to by in_mp, as an approximation
 * of the matrix pointed to by out_mp.
 *
 * Put differently, this routine calculates
 * |
 * |       || (*in_mp) x (*map_mp) - (*out_mp) ||
 * |
 *
 * The result is put into the argument *error_ptr.
 *
 * Returns:
 *     NO_ERROR on success, and ERROR on failure, with an appropriate error
 *     message being set.
 *
 * Index: least squares, optimization, data fitting
 *
 * -----------------------------------------------------------------------------
*/

int get_post_map_error
(
    const Matrix* map_mp,
    const Matrix* in_mp,
    const Matrix* out_mp,
    double*       error_ptr
)
{
    Matrix* result_mp = NULL;
    double  error;


    ERE(check_same_matrix_dimensions(in_mp, out_mp, "get_post_map_error"));

    if (map_mp->num_cols != map_mp->num_rows)
    {
        set_bug("Map matrix is not square in get_post_map_error.");
        return ERROR;
    }

    if (map_mp->num_cols != in_mp->num_cols)
    {
        set_bug("Incorrect map matrix size in get_post_map_error.");
        return ERROR;
    }

    ERE(multiply_matrices(&result_mp, in_mp, map_mp));

    error = rms_matrix_difference(result_mp, out_mp);

    free_matrix(result_mp);

    if (error < 0.0)
    {
        return ERROR;
    }
    else
    {
        *error_ptr = error;
        return NO_ERROR;
    }
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

int get_post_map_relative_row_error
(
    const Matrix* map_mp,
    const Matrix* in_mp,
    const Matrix* out_mp,
    double*       error_ptr
)
{
    Matrix* result_mp = NULL;
    int     result;


    UNTESTED_CODE();

    ERE(check_same_matrix_dimensions(in_mp, out_mp,
                                     "get_post_map_relative_row_error"));

    if (    (map_mp->num_cols != map_mp->num_rows)
         || (map_mp->num_cols != in_mp->num_cols)
       )
    {
        SET_ARGUMENT_BUG();
        return ERROR;
    }

    ERE(multiply_matrices(&result_mp, in_mp, map_mp));

    result = get_rms_relative_row_error((Vector**)NULL, result_mp, out_mp,
                                        error_ptr);

    free_matrix(result_mp);

    return result;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

static int get_best_post_map_guts
(
    Matrix**      X_mpp,
    const Matrix* A_mp,
    const Matrix* B_mp
)
{
    int result;


    if (    (fs_least_squares_method < 0)
         || (fs_least_squares_method >= fs_num_least_squares_methods)
       )
    {
        SET_CANT_HAPPEN_BUG();
        result = ERROR;
    }
    else
    {
        int (*least_squares_fn) (Matrix**, const Matrix*, const Matrix*) =
            /* Kobus: 05/01/22: Be very pedantic here, to keep C++ happy.  */
            (int(*)(Matrix**, const Matrix*, const Matrix*))
                fs_least_squares_methods[ fs_least_squares_method ].fn;

        result = least_squares_fn(X_mpp, A_mp, B_mp);

        if (result == ERROR)
        {
            add_error("Error occured solving AX=B in least squares sense.");
            add_error("[ A is %d by %d. ]", A_mp->num_rows, A_mp->num_cols);
            add_error("[ B is %d by %d. ]", B_mp->num_rows, B_mp->num_cols);
        }
    }

    return result;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

static int pseudo_inverse_least_squares
(
    Matrix**      X_mpp,
    const Matrix* A_mp,
    const Matrix* B_mp
)
{
    Matrix* inv_A_mp = NULL;
    int     result;


    result = get_MP_inverse(&inv_A_mp, A_mp);

    if (result != ERROR)
    {
        result = multiply_matrices(X_mpp, inv_A_mp, B_mp);
    }

    free_matrix(inv_A_mp);

    if (result == ERROR)
    {
        add_error("Error occurred using Moore-Penrose pseudo-inverse for ");
        cat_error("least squares.");
    }

    return result;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

static int svd_least_squares
(
    Matrix**      X_mpp,
    const Matrix* A_mp,
    const Matrix* B_mp
)
{
    Matrix* u_mp       = NULL;
    Matrix* temp_mp    = NULL;
    Matrix* v_trans_mp = NULL;
    Matrix* v_mp       = NULL;
    Vector* d_vp       = NULL;
    int     rank;
    int     result;


    result = do_svd(A_mp, &u_mp, &d_vp, &v_trans_mp, &rank);

    if ((result != ERROR) && (rank < A_mp->num_cols))
    {
        set_error("Least squares using SVD failed.");
        add_error("Matrix does not have full rank.");
        add_error("Rank of %d by %d matrix is estimated as %d.",
                  A_mp->num_rows, A_mp->num_cols, rank);
        add_error("Rank estimation is modifible with ");
        cat_error("the max-matrix-condition-number option.");

        result = ERROR;
    }

    if (result != ERROR)
    {
        result = get_transpose(&v_mp, v_trans_mp);
    }

    if (result != ERROR)
    {
        result = ow_divide_matrix_by_row_vector(v_mp, d_vp);
    }


    if (result != ERROR)
    {
        result = multiply_by_transpose(&temp_mp, v_mp, u_mp);
    }

    if (result != ERROR)
    {
        result = multiply_matrices(X_mpp, temp_mp, B_mp);
    }

    free_matrix(u_mp);
    free_matrix(v_mp);
    free_matrix(v_trans_mp);
    free_matrix(temp_mp);
    free_vector(d_vp);

    if (result == ERROR)
    {
        add_error("Error occurred using SVD for least squares.");
    }

    return result;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                             get_svd_basis_for_rows
 *
 * Finds an ordered orthogonal basis according to variance
 *
 * This is essentially PCA assuming that the data has mean 0. The basis vectors
 * are the rows of the basis matrix.
 *
 * Returns:
 *     NO_ERROR on success, and ERROR on failure, with an appropriate error
 *     message being set.
 *
 * Warning:
 *     Due to historical reasons, the basis vectors are the rows. (It is likely
 *     more comon to expect them as columns).
 *
 * Index: least squares, optimization, data fitting, SVD, PCA
 *
 * -----------------------------------------------------------------------------
*/

int get_svd_basis_for_rows
(
    const Matrix* mp,
    Matrix**      basis_mpp,
    Vector**      singular_vpp
)
{
    int result = NO_ERROR;


    if (do_svd(mp, (Matrix**)NULL, singular_vpp, basis_mpp, (int*)NULL)
        == ERROR
       )
    {
        add_error("Unable to do svd to get principal componants.");
        result = ERROR;
    }

    return result;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                             get_row_fits
 *
 * Computes fitting error of basis functions
 *
 * This routine computes the fitting error using num_PC of the matrix with rows
 * of basis functions PC_mp. The reported error is a relative fit.
 *
 * Returns:
 *     NO_ERROR on success, and ERROR on failure, with an appropriate error
 *     message being set.
 *
 * Note:
 *     This routine works even if the basis vectors are not orthogonal.
 *
 * Index: least squares, optimization, data fitting
 *
 * -----------------------------------------------------------------------------
*/

int get_row_fits
(
    Matrix**      estimate_mpp,
    const Matrix* observed_mp,
    int           num_PC,
    const Matrix* PC_mp,
    double*       error_ptr
)
{
    double  temp_error;
    double  temp_norm;
    int     result;
    Matrix* reduced_PC_mp   = NULL;
    Matrix* inv_PC_trans_mp = NULL;
    Matrix* projector_mp    = NULL;


    ERE(copy_matrix(&reduced_PC_mp, PC_mp));
    reduced_PC_mp->num_rows = num_PC;

    result = get_MP_inverse_of_transpose(&inv_PC_trans_mp, reduced_PC_mp);

    if (result != ERROR)
    {
        result = multiply_with_transpose(&projector_mp, inv_PC_trans_mp,
                                         reduced_PC_mp);
    }

    if (result != ERROR)
    {
        result = multiply_matrices(estimate_mpp, observed_mp, projector_mp);
    }

    if ((result != ERROR) && (error_ptr != NULL))
    {
        temp_error = frobenius_matrix_difference(*estimate_mpp, observed_mp);
        temp_norm  = frobenius_matrix_norm(observed_mp);

        if (((temp_error < 0.0)) || ((temp_norm < 0.0)))
        {
            result = ERROR;
        }
        else
        {
            *error_ptr = temp_error / temp_norm;
        }
    }

    free_matrix(projector_mp);
    free_matrix(inv_PC_trans_mp);
    free_matrix(reduced_PC_mp);

    return result;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                             project_rows_onto_basis
 *
 * Projects rows into onto linear subspace defined by basis matrix.
 *  
 * If P is the projected data, the output `error` value is 
 * |      error = norm(P - D)/norm(D)
 * where norm() is the matrix  Frobenius norm.
 *
 * Returns:
 *     NO_ERROR on success, and ERROR on failure, with an appropriate error
 *     message being set.
 *
 * Index: least squares, optimization, data fitting, SVD, PCA
 *
 * -----------------------------------------------------------------------------
*/
int project_rows_onto_basis
(
    Matrix**      estimate_mpp,
    const Matrix* observed_mp,
    const Matrix* basis_mp,
    double*       error_ptr
)
{
    int    result;
    double temp_error;
    double temp_norm;
    Matrix* inv_basis_trans_mp = NULL;
    Matrix* projector_mp       = NULL;


    result = get_MP_inverse_of_transpose(&inv_basis_trans_mp, basis_mp);

    if (result != ERROR)
    {
        result = multiply_with_transpose(&projector_mp, inv_basis_trans_mp,
                                         basis_mp);
    }

    if (result != ERROR)
    {
        result = multiply_matrices(estimate_mpp, observed_mp, projector_mp);
    }

    if (result != ERROR)
    {
        temp_error = frobenius_matrix_difference(*estimate_mpp, observed_mp);
        temp_norm  = frobenius_matrix_norm(observed_mp);

        if (((temp_error < 0.0)) || ((temp_norm < 0.0)))
        {
            result = ERROR;
        }
        else
        {
            *error_ptr = temp_error / temp_norm;
        }
    }

    free_matrix(projector_mp);
    free_matrix(inv_basis_trans_mp);

    return result;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                           get_best_linear_fit
 *
 * Finds the best linear fit of data
 *
 * It is like get_best_linear_fit_2() except that the fitting error is not returned. 
 *
 * Given input vectors, X, and Y, this routine finds (a,b) such that the the
 * vector aX + b is as close to Y as possible in the least squares sens.
 *
 * Put differently, this routine calculates
 * |
 * |  MIN { || aX + b - Y ||   }, over a and b
 * |                        2
 * |
 *
 * The result is put into the vector pointed to by *result_vpp, which is
 * created if it is NULL, resized if it is the wrong sized, and reused
 * otherwise.  The lenghts of *x_vp and *y_vp must be the same.
 *
 * Returns:
 *     NO_ERROR on success, and ERROR on failure, with an appropriate error
 *     message being set.
 *
 * Index: least squares, optimization, data fitting
 *
 * -----------------------------------------------------------------------------
*/

int get_best_linear_fit
(
    Vector**      result_vpp,
    const Vector* x_vp,
    const Vector* y_vp
)
{
    return get_best_linear_fit_2(result_vpp, x_vp, y_vp, NULL);
}

/* =============================================================================
 *                           get_best_linear_fit_2
 *
 * Finds the best linear fit of data
 *
 * Given input vectors, X, and Y, this routine finds (a,b) such that the the
 * vector aX + b is as close to Y as possible in the least squares sens.
 *
 * Put differently, this routine calculates
 * |
 * |  MIN { || aX + b - Y ||   }, over a and b
 * |                        2
 * |
 *
 * The result is put into the vector pointed to by *result_vpp, which is
 * created if it is NULL, resized if it is the wrong sized, and reused
 * otherwise.  The lenghts of *x_vp and *y_vp must be the same.
 *
 * If the argument error_ptr is not NULL, then the RMS fitting error is returned
 * via that pointer.
 *
 * Returns:
 *     NO_ERROR on success, and ERROR on failure, with an appropriate error
 *     message being set.
 *
 * Index: least squares, optimization, data fitting
 *
 * -----------------------------------------------------------------------------
*/

int get_best_linear_fit_2
(
    Vector**      result_vpp,
    const Vector* x_vp,
    const Vector* y_vp,
    double* error_ptr
)
{
    Matrix* mp = NULL;
    int     result;


    ERE(check_same_vector_lengths(x_vp, y_vp, "get_best_linear_fit_2"));
    ERE(get_initialized_matrix(&mp, x_vp->length, 2, 1.0));


    result = put_matrix_col(mp, x_vp, 0);

    if (result != ERROR)
    {
         result = least_squares_2(result_vpp, mp, y_vp, error_ptr);
    }

    free_matrix(mp);

    return result;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

#ifdef DO_BIMODAL_ANALYSIS

/* =============================================================================
 *                              get_two_mode_basis_for_rows
 *
 * Does bi-modal analysis of data.
 *
 * This routine does bi-modal analysis of data. Normally, for colour vision
 * applications, the projection matrix P is camera sensors, the matrix A is
 * illuminants, and the matrix B is reflectances.
 *
 * This routine finds the best num_A_basis_vectors and num_B_basis_vectors,
 * forming the basis such that the approximation to the data P(A.*B) is optimal
 * in the least squars sense. See the paper by Marimont and Wandell for
 * details.
 *
 * Note:
 *      I AM NOT SURE IF THIS IS WORKING PROPERLY!
 *
 *      Problems include: The paper does not specify how to get the basis
 *      functions from the weights, and I am not sure if my simplistic method
 *      to do so is correct. Furthermore, the method seems to converge in ONE
 *      step, which is strange. I take this as indicating that I don't have it
 *      quite right.
 *
 *      On the other hand: The error of interest tends to be less than that
 *      obtained by regular SVD, so even if the routine is not doing what
 *      Marimont and Wandell were doing, it could still be useful.
 *
 * Returns:
 *     NO_ERROR on success, and ERROR on failure, with an appropriate error
 *     message being set.
 *
 * Index: least squares, optimization, data fitting, SVD, PCA, bi-modal fiting
 *
 * -----------------------------------------------------------------------------
*/

int get_two_mode_basis_for_rows
(
    const Matrix* P_mp,
    int           num_A_basis_vectors,
    const Matrix* A_mp,
    Matrix**      A_basis_mpp,
    int           num_B_basis_vectors,
    const Matrix* B_mp,
    Matrix**      B_basis_mpp
)
{
    Matrix* ew_product_mp  = NULL;
    Matrix* P_response_mp  = NULL;
    Matrix* B_response_mp  = NULL;
    Matrix* A_response_mp  = NULL;
    Matrix* B_weight_mp    = NULL;
    Matrix* A_weight_mp    = NULL;
    Matrix* inv_W_trans_mp = NULL;
    int     num_A          = A_mp->num_rows;
    int     num_B          = B_mp->num_rows;
    int     i, j, k;
    int     result;
    double  error;
    double  new_error;
    int     count;


    ERE(multiply_matrix_rows(&ew_product_mp, A_mp, B_mp));

    result = multiply_by_transpose(&P_response_mp, ew_product_mp, P_mp);

    if (result != ERROR)
    {
        result = get_target_matrix(&B_response_mp, 3 * num_A, num_B);
    }

    if (result != ERROR)
    {
        for (i=0; i<num_A; i++)
        {
            for (j=0; j<num_B; j++)
            {
                for (k=0; k<3; k++)
                {
                     B_response_mp->elements[ 3*i + k ][ j ] =
                                 P_response_mp->elements[ i * num_B + j ][ k ];
                }
            }
        }

        result = get_single_mode_basis_for_rows(&B_weight_mp, B_response_mp,
                                                num_B_basis_vectors);
    }

    if (result != ERROR)
    {
        result = get_two_mode_transpose(&A_response_mp, B_response_mp);

    }

    if (result != ERROR)
    {
        result = do_two_mode_step(&A_weight_mp, B_response_mp,
                                  B_weight_mp, num_A_basis_vectors);
    }

    if (result != ERROR)
    {
        result = get_MP_inverse_of_transpose(&inv_W_trans_mp, A_weight_mp);
    }

    if (result != ERROR)
    {
        result = multiply_matrices(A_basis_mpp, inv_W_trans_mp, A_mp);
    }

    if (result != ERROR)
    {
        result = get_MP_inverse_of_transpose(&inv_W_trans_mp, B_weight_mp);
    }

    if (result != ERROR)
    {
        result = multiply_matrices(B_basis_mpp, inv_W_trans_mp, B_mp);
    }

    if (result != ERROR)
    {
        result = compute_two_mode_error(P_mp,
                                        num_A_basis_vectors, A_mp,
                                        *A_basis_mpp,
                                        num_B_basis_vectors, B_mp,
                                        *B_basis_mpp,
                                        &error);
    }

    for (count = 0; count < MAX_NUM_TWO_MODE_ITERATIONS; count++)
    {
        if (result == ERROR) break;

        if (result != ERROR)
        {
            result = do_two_mode_step(&B_weight_mp, A_response_mp,
                                      A_weight_mp, num_B_basis_vectors);
        }

        if (result != ERROR)
        {

            result = do_two_mode_step(&A_weight_mp, B_response_mp,
                                      B_weight_mp, num_A_basis_vectors);
        }

        if (result != ERROR)
        {
            result = get_MP_inverse_of_transpose(&inv_W_trans_mp, A_weight_mp);
        }

        if (result != ERROR)
        {
            result = multiply_matrices(A_basis_mpp, inv_W_trans_mp, A_mp);
        }

        if (result != ERROR)
        {
            result = get_MP_inverse_of_transpose(&inv_W_trans_mp, B_weight_mp);
        }

        if (result != ERROR)
        {
            result = multiply_matrices(B_basis_mpp, inv_W_trans_mp, B_mp);
        }

        if (result != ERROR)
        {
            result = compute_two_mode_error(P_mp,
                                            num_A_basis_vectors, A_mp,
                                            *A_basis_mpp,
                                            num_B_basis_vectors, B_mp,
                                            *B_basis_mpp,
                                            &new_error);
        }

        if (ABS_OF((new_error - error) / new_error) < TWO_MODE_CONVERGENCE_THRESHOLD)
        {
            break;
        }

        error = new_error;
    }

    if (ABS_OF((new_error - error) / new_error) > TWO_MODE_CONVERGENCE_THRESHOLD)
    {
        warn_pso("Two-mode analysis did not converge after %d iterations.\n",
                 MAX_NUM_TWO_MODE_ITERATIONS);
        warn_pso("Current relative change in error is %.3e.\n",
                 ((new_error - error) / new_error));
        warn_pso("Current minimum change in error is %.3e.\n",
                 TWO_MODE_CONVERGENCE_THRESHOLD);
    }

    free_matrix(ew_product_mp);
    free_matrix(P_response_mp);
    free_matrix(A_response_mp);
    free_matrix(B_response_mp);
    free_matrix(A_weight_mp);
    free_matrix(B_weight_mp);
    free_matrix(inv_W_trans_mp);

    return result;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

static int do_two_mode_step
(
    Matrix**      weight_mpp,
    const Matrix* response_mp,
    const Matrix* cur_weight_mp,
    int           num_basis_vectors
)
{
    Matrix* temp_mp            = NULL;
    Matrix* transposed_temp_mp = NULL;
    int     result;

    ERE(multiply_by_transpose(&temp_mp, response_mp, cur_weight_mp));

    result = get_two_mode_transpose(&transposed_temp_mp, temp_mp);

    if (result != ERROR)
    {
        result = get_single_mode_basis_for_rows(weight_mpp,
                                                transposed_temp_mp,
                                                num_basis_vectors);
    }

    free_matrix(temp_mp);
    free_matrix(transposed_temp_mp);

    return result;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

static int get_two_mode_transpose(Matrix** target_mpp, Matrix* mp)
{
    Matrix*  target_mp;
    int      num_A = mp->num_rows / 3;
    int      num_B = mp->num_cols;
    int      num_target_rows = 3 * num_B;
    int      num_target_cols = num_A;
    int      i, j, k;


    ERE(get_target_matrix(target_mpp, num_target_rows, num_target_cols));
    target_mp = *target_mpp;

    for (i=0; i<num_B; i++)
    {
        for (j=0; j<num_A; j++)
        {
            for (k=0; k<3; k++)
            {
                target_mp->elements[ 3 * i + k ][ j ] =
                                           mp->elements[ 3 * j + k ][ i ];
            }
        }
    }

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

static int get_single_mode_basis_for_rows
(
    Matrix**      weight_mpp,
    const Matrix* response_mp,
    int           num_basis_rows
)
{
    int result = NO_ERROR;


    if (do_svd(response_mp, (Matrix**)NULL, (Vector**)NULL, weight_mpp,
               (int*)NULL)
        == ERROR
       )
    {
        add_error("Unable to do svd to get principal componants.");
        result = ERROR;
    }

#ifndef TRY_IT_DIFFERENTLY
    if (result != ERROR)
    {
        (*weight_mpp)->num_rows = num_basis_rows;
    }
#endif

    return result;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

static int compute_two_mode_error
(
    const Matrix* P_mp,
    int           num_A_basis_vectors,
    const Matrix* A_mp,
    const Matrix* A_basis_mp,
    int           num_B_basis_vectors,
    const Matrix* B_mp,
    const Matrix* B_basis_mp,
    double*       total_error_ptr
)
{
    Matrix* ew_product_mp = NULL;
    Matrix* est_mp        = NULL;
    Matrix* obs_mp        = NULL;
    Matrix* A_fit_mp      = NULL;
    Matrix* B_fit_mp      = NULL;
    Vector* error_vp      = NULL;
    int     result;


    ERE(get_row_fits(&A_fit_mp, A_mp, num_A_basis_vectors, A_basis_mp,
                     (double*)NULL));

    result = get_row_fits(&B_fit_mp, B_mp, num_B_basis_vectors, B_basis_mp,
                          (double*)NULL);

    if (result != ERROR)
    {
        result = multiply_matrix_rows(&ew_product_mp, A_fit_mp, B_fit_mp);
    }

    if (result != ERROR)
    {
        result = multiply_by_transpose(&est_mp, ew_product_mp, P_mp);
    }

    if (result != ERROR)
    {
        result = multiply_matrix_rows(&ew_product_mp, A_mp, B_mp);
    }

    if (result != ERROR)
    {
        result = multiply_by_transpose(&obs_mp, ew_product_mp, P_mp);
    }

    ERE(get_rms_col_error(&error_vp, obs_mp, est_mp, total_error_ptr));

    verbose_pso(10, "Absolute error\n");
    verbose_pso(10, "    Num A basis vectors : %d\n", num_A_basis_vectors);
    verbose_pso(10, "    Num B basis vectors : %d\n", num_B_basis_vectors);
    verbose_pso(10, "        Total : %.3f\n", *total_error_ptr);
    verbose_pso(10, "        Red   : %.3f\n", error_vp->elements[ 0 ]);
    verbose_pso(10, "        Green : %.3f\n", error_vp->elements[ 1 ]);
    verbose_pso(10, "        Blue  : %.3f\n\n", error_vp->elements[ 2 ]);

    free_matrix(ew_product_mp);
    free_matrix(A_fit_mp);
    free_matrix(B_fit_mp);
    free_matrix(obs_mp);
    free_matrix(est_mp);
    free_vector(error_vp);

    return result;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

#endif

#ifdef __cplusplus
}
#endif

