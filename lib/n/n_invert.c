
/* $Id: n_invert.c 15478 2013-10-03 00:06:51Z predoehl $ */

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
#include "n/n_invert.h"
#include "n/n_svd.h"
#include "wrap_lapack/wrap_lapack.h"

/* For declared function pointers. Generally harmless. */
#ifdef __cplusplus
extern "C" {
#endif

/* -------------------------------------------------------------------------- */

static Method_option fs_matrix_inversion_methods[ ] =
{
    { "lapack",              "lapack", (int(*)())do_lapack_matrix_inversion},
    { "svd",                 "svd",    (int(*)())do_svd_matrix_inversion},
    { "gaussian-elimination", "ge",    (int(*)())do_gaussian_elimination_matrix_inversion}
};

static const int fs_num_matrix_inversion_methods =
                                sizeof(fs_matrix_inversion_methods) /
                                          sizeof(fs_matrix_inversion_methods[ 0 ]);
static int         fs_matrix_inversion_method                  = 0;
static const char* fs_matrix_inversion_method_option_short_str =
                                                       "matrix-inversion";
static const char* fs_matrix_inversion_method_option_long_str =
                                                      "matrix-inversion-method";


/* -------------------------------------------------------------------------- */

int set_matrix_inversion_options(const char* option, const char* value)
{
    char lc_option[ 100 ];
    int  result           = NOT_FOUND;


    EXTENDED_LC_BUFF_CPY(lc_option, option);

    if (    (lc_option[ 0 ] == '\0')
          || match_pattern(lc_option,
                           fs_matrix_inversion_method_option_short_str)
          || match_pattern(lc_option,
                           fs_matrix_inversion_method_option_long_str)
       )
    {
        if (value == NULL) return NO_ERROR;

        ERE(parse_method_option(fs_matrix_inversion_methods,
                                fs_num_matrix_inversion_methods,
                                fs_matrix_inversion_method_option_long_str,
                                "matrix inversion method", value,
                                &fs_matrix_inversion_method));
        result = NO_ERROR;
    }

    return result;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*
 * =============================================================================
 *                              get_matrix_inverse
 *
 * Inverts a matrix
 *
 * This routine computes the inverse of a matrix.
 *
 * The first argument is the adress of the target matrix. If the target matrix
 * itself is NULL, then a matrix of the appropriate size is created. If the
 * target matrix is the wrong size, it is resized. Finally, if it is the right
 * size, then the storage is recycled, as is.
 *
 * The inverse may be calculated in one of several ways. The method used can be
 * set using the option "matrix-inversion-method" which is normally exposed to
 * the user. If this options is "svd" (the default), then we compute the inverse
 * using SVD. This method has the advantage that we have precise control over
 * the degree of permisable singularity through the option
 * "max-matrix-condition-number" which is normally exposed to the user. I would
 * also expect that the SVD method is more stable and accurate (but I am not
 * sure). The disadvantage of this method is that it is, in theory, a bit slower
 * then the other method implemented which is using Gaussian elimination
 * ("gaussian-elimination").  However, this since we normally use a nicely
 * optimized routine for SVD, this advantage is not likely to be true in
 * practice.
 *
 * Returns:
 *     NO_ERROR on success, and ERROR on failure, with an error message being
 *     set. This routine will fail if the matrix is close to singulare, or if
 *     memory allocation fails.
 *
 * Index: matrices, matrix arithmetic
 *
 * -----------------------------------------------------------------------------
 */

int get_matrix_inverse(Matrix** target_mpp, const Matrix* input_mp)
{
    int result;



    if (    (fs_matrix_inversion_method < 0)
         || (fs_matrix_inversion_method >= fs_num_matrix_inversion_methods)
       )
    {
        SET_CANT_HAPPEN_BUG();
        result = ERROR;
    }
    else if (    (input_mp == NULL)
              || (input_mp->num_cols < 1)
              || (input_mp->num_rows < 1)
              || (input_mp->num_rows != input_mp->num_cols)
            )
    {
        SET_ARGUMENT_BUG();
        result = ERROR;
    }
    else
    {
        int (*inversion_fn) (Matrix**, const Matrix*)
        /* Kobus: 05/01/22: Be very pedantic here, to keep C++ happy.  */
            = (int (*) (Matrix**, const Matrix*)) fs_matrix_inversion_methods[ fs_matrix_inversion_method ].fn;

        result = inversion_fn(target_mpp, input_mp);

        if (result == ERROR)
        {
            add_error("Error occured while inverting a %d by %d matrix.",
                      input_mp->num_rows, input_mp->num_cols);
        }
    }

    return result;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

#ifdef OLDER_AND_SLOWER_IN_THEORY

/* This routine was tuned up a bit in oder to try to figure out why it is so
 * slow on AMD. The tuning does make it faster without optimization turned on,
 * but they do not affect the optimized version much. (Thus it would seem that
 * the optimizer is smart enough). The mystery why the routine is fast on XEON
 * and/or slow on AMD was not resolved.
*/

/*
 * NOTE--this is not generally considered a good way to invert matrices, but can
 * be helpful for debugging and benchmarking.
*/

int do_gaussian_elimination_matrix_inversion
(
    Matrix**      target_mpp,
    const Matrix* input_mp
)
{
    double    big;
    double    pivinv;
    int     i;
    int     row;
    int     col;
    int     pivrow;
    Matrix* input_copy_mp = NULL;
    Matrix* inverse_mp;


    ERE(get_identity_matrix(target_mpp, input_mp->num_rows));
    inverse_mp = *target_mpp;

    ERE(copy_matrix(&input_copy_mp, input_mp));

    for (row = 0; row < input_copy_mp->num_rows; row++)
    {
        /* look for pivot row */
        pivrow = row;
        big = fabs(input_copy_mp->elements[row][row]);

        for (i = row + 1; i < input_copy_mp->num_rows; i++)
        {
            if (fabs(input_copy_mp->elements[i][row]) > big)
            {
                pivrow = i;
                big = fabs(input_copy_mp->elements[i][row]);
            }
        }

        /* perform pivoting */

        if (big > 10 * DBL_EPSILON)
        {
            ERE(swap_matrix_rows(input_copy_mp, row, pivrow));
            ERE(swap_matrix_rows(inverse_mp, row, pivrow));
        }
        else    /* singular */
        {
            set_error("Matrix inversion failed -- probably singular.");
            free_matrix(input_copy_mp);
            return ERROR;
        }

        /* divide pivot row by pivot element */
        pivinv = 1.0 / input_copy_mp->elements[row][row];

        input_copy_mp->elements[row][row] = 1.0;

        for (i = row + 1; i < input_copy_mp->num_cols; i++)
        {
            input_copy_mp->elements[row][i] *= pivinv;
        }

        for (i = 0; i < input_copy_mp->num_cols; i++)
        {
            inverse_mp->elements[row][i] *= pivinv;
        }

        /* reduce other rows */
        for (i = 0; i < input_copy_mp->num_rows; i++)
        {
            if (i != row)
            {
                for (col = 0; col < input_copy_mp->num_cols; col++)
                {
                    inverse_mp->elements[i][col] -=
                        inverse_mp->elements[row][col]
                        * input_copy_mp->elements[i][row];
                }
                for (col = row + 1; col < input_copy_mp->num_cols; col++)
                {
                    input_copy_mp->elements[i][col] -=
                        input_copy_mp->elements[row][col]
                        * input_copy_mp->elements[i][row];
                }
                input_copy_mp->elements[i][row] = 0.0;
            }
        }
    }

    free_matrix(input_copy_mp);

    return NO_ERROR;
}

#else

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*
 * NOTE--this is not generally considered a good way to invert matrices, but can
 * be helpful for debugging and benchmarking.
*/

int do_gaussian_elimination_matrix_inversion
(
    Matrix**      target_mpp,
    const Matrix* input_mp
)
{
    double    big;
    double    pivinv;
    register int     i;
    register int     row;
    register int     col;
    register int     pivrow;
    Matrix* input_copy_mp = NULL;
    Matrix* inverse_mp;
    double* row_pos;
    int dim = input_mp->num_rows;


    ERE(get_identity_matrix(target_mpp, dim));
    inverse_mp = *target_mpp;

    ERE(copy_matrix(&input_copy_mp, input_mp));

    for (row = 0; row < dim; row++)
    {
        /* look for pivot row */
        pivrow = row;
        big = fabs(input_copy_mp->elements[row][row]);

        for (i = row + 1; i < dim; i++)
        {
            if (ABS_OF(input_copy_mp->elements[i][row]) > big)
            {
                pivrow = i;
                big = ABS_OF(input_copy_mp->elements[i][row]);
            }
        }

        /* perform pivoting */

        if (big > 10 * DBL_EPSILON)
        {
            ERE(swap_matrix_rows(input_copy_mp, row, pivrow));
            ERE(swap_matrix_rows(inverse_mp, row, pivrow));
        }
        else    /* singular */
        {
            set_error("Matrix inversion failed -- probably singular.");
            free_matrix(input_copy_mp);
            return ERROR;
        }

        /* divide pivot row by pivot element */
        pivinv = 1.0 / input_copy_mp->elements[row][row];

        input_copy_mp->elements[row][row] = 1.0;

        row_pos = input_copy_mp->elements[row] + row + 1;

        for (i = row + 1; i < dim; i++)
        {
            /*
            input_copy_mp->elements[row][i] *= pivinv;
            */
            *row_pos *= pivinv;
            row_pos++;
        }

        row_pos = inverse_mp->elements[row];

        for (i = 0; i < dim; i++)
        {
            /*
            inverse_mp->elements[row][i] *= pivinv;
            */
            *row_pos *= pivinv;
            row_pos++;
        }

        /* reduce other rows */
        for (i = 0; i < dim; i++)
        {
            if (i != row)
            {
                register double temp = input_copy_mp->elements[i][row];
                register double* p1 = inverse_mp->elements[i];
                register double* p2 = inverse_mp->elements[row];

                for (col = 0; col < dim; col++)
                {
                    /*
                    inverse_mp->elements[i][col] -=
                        inverse_mp->elements[row][col]
                        * input_copy_mp->elements[i][row];
                    */
                    *p1 -= (*p2)*temp;
                    p1++;
                    p2++;
                }

                p1 = input_copy_mp->elements[i] + row + 1;
                p2 = input_copy_mp->elements[row] + row + 1;

                for (col = row + 1; col < dim; col++)
                {
                    /*
                    input_copy_mp->elements[i][col] -=
                        input_copy_mp->elements[row][col]
                        * input_copy_mp->elements[i][row];
                    */
                    *p1 -= (*p2)*temp;
                    p1++;
                    p2++;
                }
                input_copy_mp->elements[i][row] = 0.0;
            }
        }
    }

    free_matrix(input_copy_mp);

    return NO_ERROR;
}

#endif

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

int do_svd_matrix_inversion(Matrix** target_mpp, const Matrix* input_mp)
{

    Matrix* u_mp       = NULL;
    Matrix* v_trans_mp = NULL;
    Vector* d_vp       = NULL;
    int     rank;
    int     result;


    result = do_svd(input_mp, &u_mp, &d_vp, &v_trans_mp, &rank);

    if ((result != ERROR) && (rank < input_mp->num_rows))
    {
        db_rv(d_vp);
        set_error("Matrix inversion using SVD failed.");
        add_error("Matrix does not have full rank.");
        add_error("Rank of %d by %d matrix is estimated as %d.",
                  input_mp->num_rows, input_mp->num_cols, rank);
        add_error("Rank estimation is modifible with ");
        cat_error("the max-matrix-condition-number option.");

        result = ERROR;
    }

    if (result != ERROR)
    {
        result = do_svd_matrix_inversion_2(target_mpp, u_mp, d_vp,
                                           v_trans_mp);
    }

    free_matrix(v_trans_mp);
    free_matrix(u_mp);
    free_vector(d_vp);

    return result;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

int do_svd_matrix_inversion_2
(
    Matrix**      target_mpp,
    const Matrix* u_mp,
    const Vector* d_vp,
    const Matrix* v_trans_mp
)
{

    Matrix* v_mp       = NULL;
    int     result;


    result = get_transpose(&v_mp, v_trans_mp);

    if (result != ERROR)
    {
        result = ow_divide_matrix_by_row_vector(v_mp, d_vp);
    }

    if (result != ERROR)
    {
        result = multiply_by_transpose(target_mpp, v_mp, u_mp);
    }

    free_matrix(v_mp);

    return result;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                             get_MP_inverse
 *
 * Calculates the Moore-Penrose inverse
 *
 * This routine calculates the Moore-Penrose inverse of a matrix. If A is the
 * matrix pointed to by mp, then the MP inverse is
 * |
 * |                                 -1
 * |                            (A'A)  A'         (' means transpose).
 * |
 *
 * The result is put into the matrix pointed to by *result_mpp, which is
 * created if it is NULL, resized if it is the wrong sized, and reused
 * otherwise.
 *
 * Returns:
 *     NO_ERROR on success, and ERROR on failure, with an appropriate error
 *     message being set.
 *
 * Index: least squares
 *
 * -----------------------------------------------------------------------------
*/

int get_MP_inverse(Matrix** result_mpp, const Matrix* mp)
{
    Matrix* prod_mp   = NULL;
    Matrix* inv_mp    = NULL;
    int     result;


    result = multiply_with_transpose(&prod_mp, mp, mp);

    if (result != ERROR)
    {
        result = get_matrix_inverse(&inv_mp, prod_mp);
    }

    if (result != ERROR)
    {
        result = multiply_by_transpose(result_mpp, inv_mp, mp);
    }

    free_matrix(prod_mp);
    free_matrix(inv_mp);

    if (result == ERROR)
    {
        add_error("Error occurred computing Moore-Penrose inverse ");
        cat_error("of a %d by %d matrix.", mp->num_rows, mp->num_cols);
    }

    return result;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                         get_MP_inverse_of_transpose
 *
 * Calculates the Moore-Penrose inverse of transpose
 *
 * This routine calculates the Moore-Penrose inverse of the transpose of a matrix. If A is the
 * matrix pointed to by mp, then the MP inverse of the transpose is
 * |
 * |                                 -1
 * |                            (AA')  A         (' means transpose).
 * |
 *
 * The result is put into the matrix pointed to by *result_mpp, which is
 * created if it is NULL, resized if it is the wrong sized, and reused
 * otherwise.
 *
 * Returns:
 *     NO_ERROR on success, and ERROR on failure, with an appropriate error
 *     message being set.
 *
 * Index: least squares
 *
 * -----------------------------------------------------------------------------
*/

int get_MP_inverse_of_transpose(Matrix** result_mpp, const Matrix* mp)
{
    Matrix* prod_mp   = NULL;
    Matrix* inv_mp    = NULL;
    int     result;


    result = multiply_by_transpose(&prod_mp, mp, mp);

    if (result != ERROR)
    {
        result = get_matrix_inverse(&inv_mp, prod_mp);
    }

    if (result != ERROR)
    {
        result = multiply_matrices(result_mpp, inv_mp, mp);
    }

    free_matrix(prod_mp);
    free_matrix(inv_mp);

    if (result == ERROR)
    {
        add_error("Error occurred computing Moore-Penrose inverse of ");
        cat_error("the transpose of a %d by %d matrix.",
                  mp->num_rows, mp->num_cols);
    }

    return result;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

#ifdef __cplusplus
}
#endif




