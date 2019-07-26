
/* $Id: m_error.c 20840 2016-09-04 18:27:14Z kobus $ */

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

#include "m/m_gen.h"      /*  Only safe if first #include in a ".c" file  */

#include "m/m_vec_metric.h"
#include "m/m_error.h"

#ifdef __cplusplus
extern "C" {
#endif

/* -------------------------------------------------------------------------- */

/* =============================================================================
 *                         check_same_matrix_vector_dimensions
 *
 * Checks that two matrix vectors have the same dimensions
 *
 * This routine checks that two matrix vectors have the same dimensions. It they
 * do not, then the an error message is set and ERROR is returned. The argument
 * context_str can be used to add more information; typically, it is the name of
 * the calling routine, but it could be some other string which works well as
 * the sequal to the message "failed in".  If context_str is NULL, then it is
 * not used.
 *
 * Macros:
 *     If different dimensions are likely due to a bug, then you may want to
 *     wrap this routine in the macro ESBRE(), which prints the error, calls
 *     set_bug(), (the "SB"), and then returns ERROR. If different dimensions
 *     are simply an error that the calling routine should deal with, then
 *     wrapping the call to this routine in ERE() saves typing.
 *
 * Returns:
 *    NO_ERROR if the matrices have the same dimensions, and ERROR otherwise.
 *    Depending on the bug handler in place, this routine may not return at all.
 *
 * Index: error handling, debugging
 *
 * -----------------------------------------------------------------------------
*/

#ifdef TEST
int debug_check_same_matrix_vector_dimensions
#else
int check_same_matrix_vector_dimensions
#endif
(
    const Matrix_vector* first_mvp,
    const Matrix_vector* second_mvp,
#ifdef TEST
    int           line_number,
    const char*   file_name,
#endif
    const char*   context_str
)
{
    int first_matrix_vector_length = NOT_SET;
    int second_matrix_vector_length = NOT_SET;
    int result = NO_ERROR;
    int i;


    if (first_mvp == NULL)
    {
        set_error("Null first matrix vector in dimension equality check.");
        result = ERROR;
    }

    if ((result != ERROR) && (second_mvp == NULL))
    {
        set_error("Null second matrix vector in dimension equality check.");
        result = ERROR;
    }

    if (result != ERROR)
    {
        first_matrix_vector_length = first_mvp->length;
        second_matrix_vector_length = second_mvp->length;

        if (first_matrix_vector_length != second_matrix_vector_length)
        {
            set_error("Mismatch in matrix vector lengths (%d versus %d).",
                      first_matrix_vector_length, second_matrix_vector_length);
            result = ERROR;
        }
    }

    if (result != ERROR)
    {
        for (i = 0; i < first_matrix_vector_length; i++)
        {
#ifdef TEST
            if (debug_check_same_matrix_dimensions(first_mvp->elements[ i ],
                                                   second_mvp->elements[ i ],
                                                   line_number,
                                                   file_name,
                                                   context_str)
#else
            if (check_same_matrix_dimensions(first_mvp->elements[ i ],
                                             second_mvp->elements[ i ],
                                             context_str)
#endif
                == ERROR)
            {
                add_error("Dimensions of matrix vector elements %d are not the same.",
                          i + 1);
                result = ERROR;
            }
        }
    }

    if (result == ERROR)
    {
#ifdef TEST
        add_error("Failed dimension check called from line %d of file %s.",
                  line_number, file_name);
#endif

        if (context_str != NULL)
        {
            add_error("Matrix vector dimension equality check failed in %s.", context_str);
        }
        else
        {
            add_error("Matrix vector dimension equality check failed.");
        }

    }

    return result;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                         check_same_matrix_dimensions
 *
 * Checks that two matrices have the same dimensions
 *
 * This routine checks that two matrices have the same dimensions. It the
 * matrices do not have the same dimension, then the an error message is set and
 * ERROR is returned. The argument context_str can be used to add more
 * information; typically, it is the name of the calling routine, but it could
 * be some other string which works well as the sequal to the message "failed
 * in".  If context_str is NULL, then it is not used.
 *
 * Macros:
 *     If different matrix dimensions are likely due to a bug, then you may want
 *     to wrap this routine in the macro ESBRE(), which prints the error, calls
 *     set_bug(), (the "SB"), and then returns ERROR. If different matrix
 *     dimensions are simply an error that the calling routine should deal with,
 *     then wrapping the call to this routine in ERE() saves typing.
 *
 * Returns:
 *    NO_ERROR if the matrices have the same dimensions, and ERROR otherwise.
 *    Depending on the bug handler in place, this routine may not return at all.
 *
 * Index: error handling, debugging
 *
 * -----------------------------------------------------------------------------
*/

#ifdef TEST
int debug_check_same_matrix_dimensions
#else
int check_same_matrix_dimensions
#endif
(
    const Matrix* first_mp,
    const Matrix* second_mp,
#ifdef TEST
    int           line_number,
    const char*   file_name,
#endif
    const char*   context_str
)
{
    int first_matrix_num_rows, second_matrix_num_rows;
    int first_matrix_num_cols, second_matrix_num_cols;
    int result = NO_ERROR;


    if ((first_mp == NULL) && (second_mp == NULL))
    {
        return NO_ERROR;
    }

    if (first_mp == NULL)
    {
        set_error("Null first matrix but not second in dimension equality check.");
        return ERROR;
    }

    if ((result != ERROR) && (second_mp == NULL))
    {
        set_error("Null second matrix but not first in dimension equality check.");
        return ERROR;
    }

    if (result != ERROR)
    {
        first_matrix_num_rows = first_mp->num_rows;
        second_matrix_num_rows = second_mp->num_rows;

        if (first_matrix_num_rows != second_matrix_num_rows)
        {
            set_error("Mismatch in matrix row counts (%d versus %d).",
                      first_matrix_num_rows, second_matrix_num_rows);
            result = ERROR;
        }
    }

    if (result != ERROR)
    {
        first_matrix_num_cols = first_mp->num_cols;
        second_matrix_num_cols = second_mp->num_cols;

        if (first_matrix_num_cols != second_matrix_num_cols)
        {
            set_error("Mismatch in matrix columm counts (%d versus %d).",
                     first_matrix_num_cols, second_matrix_num_cols);
            result = ERROR;
        }
    }

    if (result == ERROR)
    {
#ifdef TEST
        add_error("Failed dimension check called from line %d of file %s.",
                  line_number, file_name);
#endif

        if (context_str != NULL)
        {
            add_error("Matrix dimension equality check failed in %s.", context_str);
        }
        else
        {
            add_error("Matrix dimension equality check failed.");
        }

    }

    return result;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                         check_same_vector_lengths
 *
 * Checks that two vectors have the same length
 *
 * This routine checks that two vectors have the same length. If they do
 * not,then the an error message is set and ERROR is returned. The argument
 * context_str can be used to add more information; typically, it is the name of
 * the calling routine, but it could be some other string which works well as
 * the sequal to the message "failed in".  If context_str is NULL, then it is
 * not used.
 *
 * Macros:
 *     If different vector lengths are likely due to a bug, then you may want to
 *     wrap this routine in the macro ESBRE(), which prints the error, calls
 *     set_bug(), (the "SB"), and then returns ERROR. If different vector
 *     lengths are simply an error that the calling routine should deal with,
 *     then wrapping the call to this routine in ERE() saves typing.
 *
 * Returns:
 *    NO_ERROR if the matrices have the same dimensions, and ERROR otherwise.
 *    Depending on the bug handler in place, this routine may not return at all.
 *
 * Index: error handling, debugging
 *
 * -----------------------------------------------------------------------------
*/

#ifdef TEST
int debug_check_same_vector_lengths
#else
int check_same_vector_lengths
#endif
(
    const Vector* first_vp,
    const Vector* second_vp,
#ifdef TEST
    int           line_number,
    const char*   file_name,
#endif
    const char*   context_str
)
{
    int first_vector_length, second_vector_length;
    int result = NO_ERROR;


    if ((first_vp == NULL) && (second_vp == NULL))
    {
        return NO_ERROR;
    }
    else if (first_vp == NULL)
    {
        set_error("Null first vector but non-null second vector sent to dimension equality check.");
        result = ERROR;
    }
    else if (second_vp == NULL)
    {
        set_error("Null second vector but non-null first vector sent to dimension equality check.");
        result = ERROR;
    }

    if (result != ERROR)
    {
        first_vector_length = first_vp->length;
        second_vector_length = second_vp->length;

        if (first_vector_length != second_vector_length)
        {
            set_error("Mismatch in vector length (%d versus %d).",
                      first_vector_length, second_vector_length);
            result = ERROR;
        }
    }

    if (result == ERROR)
    {
#ifdef TEST
        add_error("Failed dimension check called from line %d of file %s.",
                  line_number, file_name);
#endif

        if (context_str != NULL)
        {
            add_error("Vector dimension equality check failed in %s.", context_str);
        }
        else
        {
            add_error("Vector dimension equality check failed.");
        }

    }

    return result;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

int get_expanded_error_box(Matrix** expanded_mpp, const Matrix* mp)
{
    int count, i, j, k;
    int num_box_rows;
    int num_box_cols;
    int num_expanded_rows;
    int expansion_factor;
    int dim;
    Matrix* expanded_mp;
    int temp;


    num_box_rows = mp->num_rows;
    num_box_cols = mp->num_cols;

    /*
    // The input to this routine is a matrix with twice as many columns as the
    // dimension because each original point (X1, X2, ... ) became
    // (X1-error, X1+error, X2-error, X2+error,
    */
    dim = num_box_cols/2;

    expansion_factor = (1 << dim);
    num_expanded_rows = num_box_rows * expansion_factor;

    ERE(get_target_matrix(expanded_mpp, num_expanded_rows, dim));
    expanded_mp = *expanded_mpp;

    count = 0;

    for (i=0; i<num_box_rows; i++)
    {
        for (j=0; j<expansion_factor; j++)
        {
            for (k=0; k<dim; k++)
            {
                temp = j >> k;

                if (IS_EVEN(temp))
                {
                    (expanded_mp->elements)[count][k] = (mp->elements)[i][2*k];
                }
                else
                {
                    (expanded_mp->elements)[count][k] =(mp->elements)[i][2*k+1];
                }
            }
            count++;
        }
    }

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

Matrix* expand_error_box(const Matrix* mp)
{
    int count, i, j, k;
    int num_box_rows;
    int num_box_cols;
    int num_expanded_rows;
    int expansion_factor;
    int dim;
    Matrix* expanded_mp;
    int temp;


    num_box_rows = mp->num_rows;
    num_box_cols = mp->num_cols;

    /*
    // The input to this routine is a matrix with twice as many columns as the
    // dimension because each original point (X1, X2, ... ) became
    // (X1-error, X1+error, X2-error, X2+error,
    */
    dim = num_box_cols/2;

    expansion_factor = (1 << dim);
    num_expanded_rows = num_box_rows * expansion_factor;

    NRN(expanded_mp = create_matrix(num_expanded_rows, dim));

    count = 0;

    for (i=0; i<num_box_rows; i++)
    {
        for (j=0; j<expansion_factor; j++)
        {
            for (k=0; k<dim; k++)
            {
                temp = j >> k;

                if (IS_EVEN(temp))
                {
                    (expanded_mp->elements)[count][k] = (mp->elements)[i][2*k];
                }
                else
                {
                    (expanded_mp->elements)[count][k] =(mp->elements)[i][2*k+1];
                }
            }
            count++;
        }
    }

    return expanded_mp;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

Matrix* create_error_points(const Matrix* mp, double error)
{
    Matrix* error_box_mp, *new_mp;
    int i,j;


    NRN(error_box_mp = create_matrix(mp->num_rows, 2*(mp->num_cols)));

    for (i=0; i<mp->num_rows; i++)
    {
        for (j=0; j<mp->num_cols; j++)
        {
            (error_box_mp->elements)[i][ 2*j ] = (mp->elements)[i][j] + error;
            (error_box_mp->elements)[i][2*j+1] = (mp->elements)[i][j] - error;
        }
    }

    NRN(new_mp = expand_error_box(error_box_mp));

    free_matrix(error_box_mp);

    return new_mp;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                             get_points_with_error
 *
 * Computes points with added error
 *
 * This routine adds error to matrix rows. If abs_err is greater than 0, abs_err
 * absolute error is added. If rel_err is greater than 0, then rel_err relative
 * error is added. If both are positive, then both are used, with the absolute
 * error being added first. Finally, if min_valid_value is greater than or equal
 * to zero, the points computed will be truncated at that value.
 *
 * Returns:
 *     NO_ERROR on success, and ERROR on failure, with an error message being
 *     set. This routine will fail if storage allocation fails.
 *
 * Index: data error
 *
 * -----------------------------------------------------------------------------
*/

int get_points_with_error
(
    Matrix**      result_mpp,
    const Matrix* mp,
    double        abs_err,
    double        rel_err,
    double        min_valid_value
)
{
    Matrix* error_box_mp;
    int i,j;
    int result;


    NRE(error_box_mp = create_matrix(mp->num_rows, 2*(mp->num_cols)));

    for (i=0; i<mp->num_rows; i++)
    {
        for (j=0; j<mp->num_cols; j++)
        {
            (error_box_mp->elements)[i][ 2*j ] = (mp->elements)[i][j];

            (error_box_mp->elements)[i][2*j+1] = (mp->elements)[i][j];

            if (abs_err > 0.0)
            {
                (error_box_mp->elements)[i][ 2*j ] += abs_err;
                (error_box_mp->elements)[i][2*j+1] -= abs_err;
            }

            if (rel_err > 0.0)
            {
                (error_box_mp->elements)[i][ 2*j ]   *= (1.0 + rel_err);
                (error_box_mp->elements)[i][ 2*j+1 ] *= (1.0 - rel_err);
            }

            if (min_valid_value >= 0.0)
            {
                (error_box_mp->elements)[i][ 2*j ] =
                    MAX_OF((error_box_mp->elements)[i][ 2*j ], min_valid_value);
            }
        }
    }

    result = get_expanded_error_box(result_mpp, error_box_mp);

    free_matrix(error_box_mp);

    return result;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

#ifdef TEST
void set_divide_by_zero_error(int line, const char* file)
{
    IMPORT int abort_interactive_math_errors;


    if ((abort_interactive_math_errors) && (is_interactive()))
    {
        set_bug("Division by zero on line %d of file %s.", line, file);
    }
    else
    {
        set_error("Division by zero on line %d of file %s.", line, file);
    }
}

#else

void set_divide_by_zero_error(void)
{


    set_error("Division by zero.");
}

#endif

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

#ifdef TEST
void set_overflow_error(int line, const char* file)
{
    IMPORT int abort_interactive_math_errors;


    if ((abort_interactive_math_errors) && (is_interactive()))
    {
        set_bug("Arithmetic overflow detected on line %d of file %s.",
                line, file);
    }
    else
    {
        set_error("Arithmetic overflow detected on line %d of file %s.",
                  line, file);
    }
}

#else
void set_overflow_error(void)
{


    set_error("Arithmetic overflow detected.");
}
#endif

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

int check_quadratic_constraints
(
    const Vector* result_vp,
    const Matrix* le_constraint_mp,
    const Vector* le_constraint_col_vp,
    const Vector* lb_constraint_vp,
    const Vector* ub_constraint_vp
)
{
    Vector* constraint_result_vp = NULL;
    int     result               = NO_ERROR;
    double    relative_tolerance   = 0.05;
    double    absolute_tolerance   = 0.05;
    double    biggest_deviation;
    int       index_for_biggest_deviation; 


    if (    (ub_constraint_vp != NULL)
         && (ub_constraint_vp->length > 0)
         && ( ! (first_vector_is_less(result_vp, ub_constraint_vp,
                                      relative_tolerance, absolute_tolerance)))

       )
    {
        set_error("Upper-bound constraint not satisfied.");
        add_error("Relative tolerance is %e and absolute tolerance is %e.",
                  relative_tolerance, absolute_tolerance);
        add_error("Both must be broken for this message to occur."); 
        return ERROR;
    }

    if (    (lb_constraint_vp != NULL)
         && (lb_constraint_vp->length > 0)
         && ( ! (first_vector_is_less_2(lb_constraint_vp, result_vp,
                                        relative_tolerance, absolute_tolerance,
                                        &index_for_biggest_deviation,
                                        &biggest_deviation)))
       )
    {
        set_error("Lower-bound constraint not satisfied.");
        add_error("Relative tolerance is %e and absolute tolerance is %e.",
                  relative_tolerance, absolute_tolerance);
        add_error("Both must be broken for this message to occur."); 
        add_error("Worst offending value is %e due to element %d.",
                  biggest_deviation, index_for_biggest_deviation); 
        dbi(index_for_biggest_deviation); 
        add_error("Corresponding bound element is %e and corresponding result element is %e.",
                  lb_constraint_vp->elements[ index_for_biggest_deviation ],
                  result_vp->elements[ index_for_biggest_deviation ]); 
        
        return ERROR;
    }

    if ((le_constraint_mp != NULL) && (le_constraint_mp->num_rows > 0))
    {
        ERE(multiply_matrix_and_vector(&constraint_result_vp, le_constraint_mp,
                                       result_vp));

        if ( ! (first_vector_is_less(constraint_result_vp, le_constraint_col_vp,
                                     relative_tolerance, absolute_tolerance))
           )
        {
            set_error("Inequality constraint not satisfied.");
            add_error("Relative tolerance is %e and absolute tolerance is %e.",
                      relative_tolerance, absolute_tolerance);
            add_error("Both must be broken for this message to occur."); 
            result = ERROR;
        }
    }

    free_vector(constraint_result_vp);

    return result;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

#ifdef __cplusplus
}
#endif

