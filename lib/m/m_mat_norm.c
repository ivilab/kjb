
/* $Id: m_mat_norm.c 20654 2016-05-05 23:13:43Z kobus $ */

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
#include "m/m_missing.h"
#include "m/m_mat_norm.h"

#ifdef __cplusplus
extern "C" {
#endif

/* -------------------------------------------------------------------------- */

/* =============================================================================
 *                         frobenius_matrix_norm
 *
 * Returns the Frobenius norm of a matrix
 *
 * This routine calculates the Frobenius norm of a matrix. The Frobenius norm is
 * simply the square root of the sum of the squares of the matrix elements.
 *
 * Returns:
 *    The Frobenius norm or a negative number if there is an error (not very
 *    likely--currently impossible).
 *
 * Index: matrices, matrix measures, matrix normalization
 *
 * -----------------------------------------------------------------------------
*/

double frobenius_matrix_norm(const Matrix* mp)
{
    int   i;
    int   j;
    int   num_rows;
    int   num_cols;
    double* row_pos;
    double  sum_of_squares = 0.0;


    num_rows = mp->num_rows;
    num_cols = mp->num_cols;

    for (i=0; i<num_rows; i++)
    {
        row_pos = mp->elements[ i ];

        for (j=0; j<num_cols; j++)
        {
            sum_of_squares += (*row_pos) * (*row_pos);
            row_pos++;
        }
    }

    return sqrt(sum_of_squares);
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                         get_matrix_col_norms
 *
 * Calculates the norms of matrix columns.
 *
 * This routine calculates the norms of the columns of a matrix. The
 * norm of each column is returned as an element of the vector pointed to by
 * result_vpp. That vector is created or resized as need be.
 *
 * Returns:
 *    NO_ERROR on success, and ERROR if there is a problem with the error
 *    message being set.
 *
 * Index: matrices, matrix measures
 *
 * -----------------------------------------------------------------------------
*/

int get_matrix_col_norms(Vector** result_vpp, const Matrix* mp)
{
    int       i;
    int       j;
    int       num_rows;
    int       num_cols;
    Vector*   result_vp;


    num_rows = mp->num_rows;
    num_cols = mp->num_cols;

    ERE(get_target_vector(result_vpp, num_cols));

    result_vp = *result_vpp;

    for (j=0; j<num_cols; j++)
    {
        double sum_of_squares = 0.0;


        for (i=0; i<num_rows; i++)
        {
            double temp;

            temp = mp->elements[ i ][ j ];
            sum_of_squares += temp * temp;
        }

        result_vp->elements[ j ] = sqrt(sum_of_squares);
    }

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                         get_matrix_row_norms
 *
 * Calculates the norms of matrix rows.
 *
 * This routine calculates the norms of the rows of a matrix. The
 * norm of each row is returned as an element of the vector pointed to by
 * result_vpp. That vector is created or resized as need be.
 *
 * Returns:
 *    NO_ERROR on success, and ERROR if there is a problem with the error
 *    message being set.
 *
 * Index: matrices, matrix measures
 *
 * -----------------------------------------------------------------------------
*/

int get_matrix_row_norms(Vector** result_vpp, const Matrix* mp)
{
    int       i;
    int       j;
    int       num_rows;
    int       num_cols;
    Vector*   result_vp;


    num_rows = mp->num_rows;
    num_cols = mp->num_cols;

    ERE(get_target_vector(result_vpp, num_rows));

    result_vp = *result_vpp;

    for (i=0; i<num_rows; i++)
    {
        double sum_of_squares = 0.0;


        for (j=0; j<num_cols; j++)
        {
            double temp;

            temp = mp->elements[ i ][ j ];
            sum_of_squares += temp * temp;
        }

        result_vp->elements[ i ] = sqrt(sum_of_squares);
    }

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

int ow_scale_matrix_by_sum(Matrix* mp)
{
    int  i;
    int  j;
    double sum;
    double min;


    ERE(min = min_matrix_element(mp));

    ERE(sum = sum_matrix_elements(mp));

    if (min < 0.0)
    {
        set_error("Unable to scale matrix by sum.");
        add_error("The matrix must be non-negative.");
        return ERROR;
    }

    if (sum <= 0.0)
    {
        set_error("Unable to scale matrix by sum.");
        add_error("The matrix sum must be positive.");
        return ERROR;
    }

    for (i=0; i<mp->num_rows; i++)
    {
        for (j =0; j<mp->num_cols; j++)
        {
            (mp->elements)[i][j] /= sum;
        }
    }

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

int ow_scale_matrix_by_max(Matrix* mp)
{
    int  i;
    int  j;
    double max;


    ERE(max = max_matrix_element(mp));

    if (max <= 0.0)
    {
        set_error("Unable to scale matrix by max.");
        add_error("The matrix max must be positive, but it is %.2e.", max);
        return ERROR;
    }

#ifdef USE_IS_ZERO_DBL_FOR_DIVIDE_BY_ZERO
    if (IS_ZERO_DBL(max))
    {
        set_error("Attempt to scale matrix by max value failed.");
        add_error("The max value is too close to zero.");
        return ERROR;
    }
#else
    /*
    // Exact compare is OK because overflow is handled below.
    */
    if (max == 0.0)
    {
        set_error("Attempt to scale matrix by max value failed.");
        add_error("The max value is zero.");
        return ERROR;
    }
#endif

    for (i=0; i<mp->num_rows; i++)
    {
        for (j =0; j<mp->num_cols; j++)
        {
            (mp->elements)[i][j] /= max;

#ifdef USE_COMPARE_WITH_REAL_MAX_FOR_DIVIDE_BY_ZERO
            if (    ((mp->elements)[i][j] > DBL_MOST_POSITIVE)
                 || ((mp->elements)[i][j] < DBL_MOST_NEGATIVE)
               )
            {
                /*
                // How can scaling by max cause overflow?
                */
                SET_CANT_HAPPEN_BUG();
                return ERROR;
            }
#endif
        }
    }

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

int ow_scale_matrix_by_max_abs(Matrix* mp)
{
    int  i;
    int  j;
    double max_abs;


    ERE(max_abs = max_abs_matrix_element(mp));

#ifdef USE_IS_ZERO_DBL_FOR_DIVIDE_BY_ZERO
    if (IS_ZERO_DBL(max_abs))
    {
        set_error("Attempt to scale matrix by max absolute value failed.");
        add_error("The max absolute value is too close to zero.");
        return ERROR;
    }
#else
    /*
    // Exact compare is OK because overflow is handled separately.
    */
    if (max_abs == 0.0)
    {
        set_error("Attempt to scale matrix by max absolute value failed.");
        add_error("The max absolute value is zero.");
        return ERROR;
    }
#endif

    for (i=0; i<mp->num_rows; i++)
    {
        for (j =0; j<mp->num_cols; j++)
        {
            (mp->elements)[i][j] /= max_abs;

#ifdef USE_COMPARE_WITH_REAL_MAX_FOR_DIVIDE_BY_ZERO
            if (    ((mp->elements)[i][j] > DBL_MOST_POSITIVE)
                 || ((mp->elements)[i][j] < DBL_MOST_NEGATIVE)
               )
            {
                /*
                // How can scaling by max cause overflow?
                */
                SET_CANT_HAPPEN_BUG();
                return ERROR;
            }
#endif
        }
    }

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

#ifdef TEST

int debug_safe_ow_scale_matrix_row_by_sum
(
    Matrix*     mp,
    int         row,
    const char* file,
    int         line
)
{
    Vector temp_vector;


    if (mp == NULL)
    {
        SET_ARGUMENT_BUG();
        return ERROR;
    }

     temp_vector.length = mp->num_cols;
     temp_vector.elements = (mp->elements)[row];

     if (ow_scale_vector_by_sum(&temp_vector) == ERROR)
     {
         warn_pso("Scale of matrix row %d failed. Making it uniform.\n", row);
         warn_pso("(Called from line %d of %s)\n\n", line, file);
         ERE(ow_set_vector(&temp_vector, 1.0 / mp->num_cols));
     }

    return NO_ERROR;
}

#else

int safe_ow_scale_matrix_row_by_sum(Matrix* mp, int row)
{
    Vector temp_vector;


    if (mp == NULL)
    {
        SET_ARGUMENT_BUG();
        return ERROR;
    }

     temp_vector.length = mp->num_cols;
     temp_vector.elements = (mp->elements)[row];

     if (ow_scale_vector_by_sum(&temp_vector) == ERROR)
     {
         warn_pso("Scale of matrix row %d failed. Making it uniform.\n", row);
         ERE(ow_set_vector(&temp_vector, 1.0 / mp->num_cols));
     }

    return NO_ERROR;
}

#endif

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

#ifdef TEST

int debug_safe_ow_scale_matrix_rows_by_sums
(
    Matrix*     mp,
    const char* file,
    int         line
)
{
    int    i;
    Vector temp_vector;


    if (mp == NULL)
    {
        SET_ARGUMENT_BUG();
        return ERROR;
    }

     temp_vector.length = mp->num_cols;

     for (i=0; i<mp->num_rows; i++)
     {
         temp_vector.elements = (mp->elements)[i];

         if (ow_scale_vector_by_sum(&temp_vector) == ERROR)
         {
             warn_pso("Scale of matrix row %d failed. Making it uniform.\n", i);
             warn_pso("(Called from line %d of %s)\n\n", line, file);
             ERE(ow_set_vector(&temp_vector, 1.0 / mp->num_cols));
         }
     }

    return NO_ERROR;
}

#else

int safe_ow_scale_matrix_rows_by_sums(Matrix* mp)
{
    int    i;
    Vector temp_vector;


    if (mp == NULL)
    {
        SET_ARGUMENT_BUG();
        return ERROR;
    }

     temp_vector.length = mp->num_cols;

     for (i=0; i<mp->num_rows; i++)
     {
         temp_vector.elements = (mp->elements)[i];

         if (ow_scale_vector_by_sum(&temp_vector) == ERROR)
         {
             warn_pso("Scale of matrix row %d failed. Making it uniform.\n", i);
             ERE(ow_set_vector(&temp_vector, 1.0 / mp->num_cols));
         }
     }

    return NO_ERROR;
}

#endif

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

int ow_scale_matrix_rows_by_sums(Matrix* mp)
{
    int    i;
    Vector temp_vector;


    if (mp == NULL)
    {
        SET_ARGUMENT_BUG();
        return ERROR;
    }

     temp_vector.length = mp->num_cols;

     for (i=0; i<mp->num_rows; i++)
     {
         temp_vector.elements = (mp->elements)[i];
         ERE(ow_scale_vector_by_sum(&temp_vector));
     }

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

int ow_scale_matrix_rows_by_max_abs(Matrix* mp)
{
    int    i;
    Vector temp_vector;


    if (mp == NULL)
    {
        SET_ARGUMENT_BUG();
        return ERROR;
    }

     temp_vector.length = mp->num_cols;

     for (i=0; i<mp->num_rows; i++)
     {
         temp_vector.elements = (mp->elements)[i];
         ERE(ow_scale_vector_by_max_abs(&temp_vector));
     }

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

int ow_scale_matrix_rows_by_max(Matrix* mp)
{
    int    i;
    Vector temp_vector;


    if (mp == NULL)
    {
        SET_ARGUMENT_BUG();
        return ERROR;
    }

     temp_vector.length = mp->num_cols;

     for (i=0; i<mp->num_rows; i++)
     {
         temp_vector.elements = (mp->elements)[i];

         if (ow_scale_vector_by_max(&temp_vector) == ERROR)
         {
             add_error("Error occured scaling row %d of a %d by %d matrix.",
                       i + 1, mp->num_rows, mp->num_cols);
             return ERROR;
         }
     }

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

int safe_ow_scale_matrix_cols_by_sums(Matrix* mp)
{
    int try_count, i, j;


    for (j=0; j<mp->num_cols; j++)
    {
        double sum = 0.0;

        for (i = 0; i < mp->num_rows; i++)
        {
            sum += mp->elements[ i ][ j ];
        }

        if (sum == 0.0)
        {
            warn_pso("Unable to scale matrix column by sum.\n");
            warn_pso("Sum of col %d of a %dx%d matrix is zero.\n",
                     j + 1, mp->num_rows, mp->num_cols);
            warn_pso("Setting all elements in col %d to be equal.\n",
                     j + 1);
            warn_pso("\n");

            for (i = 0; i < mp->num_rows; i++)
            {
                mp->elements[ i ][ j ] = 1.0;
                sum += 1.0;
            }
        }

        for (try_count = 0; try_count < 2; try_count++)
        {
            int success = TRUE;

            for (i = 0; i < mp->num_rows; i++)
            {
                mp->elements[ i ][ j ] /= sum;

                if (    (mp->elements[ i ][ j ] > DBL_MOST_POSITIVE)
                     || (mp->elements[ i ][ j ] < DBL_MOST_NEGATIVE)
                   )
                {
                    warn_pso("Unable to scale matrix column by sum.\n");
                    warn_pso("Division by sum of col %d of a %dx%d leads to overflow.\n",
                               j + 1, mp->num_rows, mp->num_cols);
                    warn_pso("Setting all elements in col %d to be equal.\n",
                             j + 1);
                    warn_pso("\n");

                    success = FALSE;
                    break;
                }
            }

            if (success) break;

            for (i = 0; i < mp->num_rows; i++)
            {
                mp->elements[ i ][ j ] = 1.0;
                sum += 1.0;
            }
        }
    }

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

int ow_scale_matrix_cols_by_sums(Matrix* mp)
{
    int    i, j;


    for (j=0; j<mp->num_cols; j++)
    {
        double sum = 0.0;

        for (i = 0; i < mp->num_rows; i++)
        {
            sum += mp->elements[ i ][ j ];
        }

        if (sum == 0.0)
        {
            set_error("Unable to scale matrix column by sum.");
            add_error("Sum of col %d of a %dx%d matrix is zero.",
                       j + 1, mp->num_rows, mp->num_cols);
            return ERROR;
        }

        for (i = 0; i < mp->num_rows; i++)
        {
            mp->elements[ i ][ j ] /= sum;

            if (    (mp->elements[ i ][ j ] > DBL_MOST_POSITIVE)
                 || (mp->elements[ i ][ j ] < DBL_MOST_NEGATIVE)
               )
            {
                set_error("Unable to scale matrix column by sum.");
                add_error("Division by sum of col %d of a %dx%d leads ",
                           j + 1, mp->num_rows, mp->num_cols);
                cat_error("to overflow");
                return ERROR;
            }
        }
    }

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

int ow_scale_matrix_row_by_sum(Matrix* mp, int row)
{
    int     j;
    int     num_cols = mp->num_cols;
    double* pos      = mp->elements[ row ];
    double  sum      = 0;



    for (j = 0; j < num_cols; j++)
    {
        if (*pos < 0.0)
        {
            set_error("Attempt to scale a matrix row by its sum invalid.");
            add_error("Element %d of row %d is negative (%.2e).",
                      j + 1, row + 1, *pos);
            return ERROR;
        }

        sum += (*pos);
        pos++;
    }

    if (sum < MIN_ABS_NORMALIZATION_VALUE)
    {
        set_error("Matrix row sum is too close to zero for normalization.");
        return ERROR;
    }

    pos = mp->elements[ row ];

    for (j = 0; j < num_cols; j++)
    {
        *pos /= sum;
        pos++;
    }

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

int ow_scale_matrix_col_by_sum(Matrix* mp, int col)
{
    int     i;
    int     num_rows = mp->num_rows;
    double  sum      = 0;


    UNTESTED_CODE();

    for (i = 0; i < num_rows; i++)
    {
        double element = mp->elements[ i ][ col ];

        if (element  < 0.0)
        {
            set_error("Attempt to scale a matrix cpl by its sum invalid.");
            add_error("Element %d of col %d is negative (%.2e).",
                      i + 1, col + 1, element);
            return ERROR;
        }

        sum += element;
    }

    if (sum < MIN_ABS_NORMALIZATION_VALUE)
    {
        set_error("Matrix column sum is too close to zero for normalization.");
        return ERROR;
    }

    for (i = 0; i < num_rows; i++)
    {
        (mp->elements[ i ][ col ]) /= sum;
    }

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                           min_abs_matrix_element
 *
 * Returns the matrix element with the minimum absolute value
 *
 * This routine returns the element with the minimum absolute value contained
 * in the matrix specified by "mp".
 *
 * Returns:
 *     Maximum absolute value of all elements.
 *
 * Index: matrices
 *
 * -----------------------------------------------------------------------------
*/
double min_abs_matrix_element(const Matrix* mp)
{
    int    i;
    int    j;
    double   min;
    double   abs_val;
    double** row_ptr;
    double*  element_ptr;


    if (mp->num_rows == 0) return 0.0;
    if (mp->num_cols == 0) return 0.0;

    min = fabs(mp->elements[0][0]);

    row_ptr =  mp->elements;

    for (i = 0; i<mp->num_rows; i++)
    {
        element_ptr = *row_ptr;

        for (j = 0; j<mp->num_cols; j++)
        {
            abs_val = fabs(*element_ptr);

            if (abs_val < min)
            {
                min = abs_val;
            }
            element_ptr++;
        }
        row_ptr++;
    }

    return min;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                               min_matrix_element
 *
 * Returns the smallest element of a matrix
 *
 * This routine returns the smallest element in the matrix specified by "mp".
 *
 * Returns:
 *     smallest matrix element.
 *
 * Index: matrices
 *
 * -----------------------------------------------------------------------------
*/
double min_matrix_element(const Matrix* mp)
{
    int    i;
    int    j;
    double   min;
    double** row_ptr;
    double*  element_ptr;


    if (mp->num_rows == 0) return 0.0;
    if (mp->num_cols == 0) return 0.0;

    min = mp->elements[0][0];

    row_ptr = mp->elements;

    for (i = 0; i<mp->num_rows; i++)
    {
        element_ptr = *row_ptr;

        for (j = 0; j<mp->num_cols; j++)
        {
            if (*element_ptr < min)
            {
                min = *element_ptr;
            }
            element_ptr++;
        }
        row_ptr++;
    }

    return min;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                          get_min_matrix_element
 *
 * Finds the smallest element of a matrix
 *
 * This routine finds the smallest element in the matrix specified by "mp", and
 * puts it into *min_ptr. The row and column of the element are returned in
 * *row_ptr and *col_ptr, respectively. Any of these three values can be NULL,
 * if you are not interested.
 *
 * Returns:
 *     NO_ERROR on sucess or ERROR if there are problems.
 *
 * Index: matrices
 *
 * -----------------------------------------------------------------------------
*/

int get_min_matrix_element
(
    const Matrix* mp,
    double*       min_ptr,
    int*          min_i_ptr,
    int*          min_j_ptr
)
{
    int    i;
    int    j;
    double   min;
    double** row_ptr;
    double*  element_ptr;
    int    min_i, min_j;


    if (    (mp == NULL)
         || (mp->num_rows == 0)
         || (mp->num_cols == 0)
       )
     {
         /*
         // This could be made to simply set an error message and return error,
         // but for now, treat it as a bug.
         */
         SET_ARGUMENT_BUG();
         return ERROR;
     }

    min = mp->elements[0][0];
    min_i = 0;
    min_j = 0;

    row_ptr =  mp->elements;

    for (i = 0; i<mp->num_rows; i++)
    {
        element_ptr = *row_ptr;

        for (j = 0; j<mp->num_cols; j++)
        {
            if (*element_ptr < min)
            {
                min = *element_ptr;
                min_i = i;
                min_j = j;
            }
            element_ptr++;
        }
        row_ptr++;
    }

    if (min_ptr != NULL) *min_ptr = min;
    if (min_i_ptr != NULL) *min_i_ptr = min_i;
    if (min_j_ptr != NULL) *min_j_ptr = min_j;

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                           max_abs_matrix_element
 *
 * Returns the matrix element with the maximum absolute value
 *
 * This routine returns the element with the maximum absolute value contained
 * in the matrix specified by "vp".
 *
 * Returns:
 *     Maximum absolute value of all elements.
 *
 * Index: matrices
 *
 * -----------------------------------------------------------------------------
*/
double max_abs_matrix_element(const Matrix* mp)
{
    int    i;
    int    j;
    double   max;
    double   abs_val;
    double** row_ptr;
    double*  element_ptr;


    if (mp->num_rows == 0) return 0.0;
    if (mp->num_cols == 0) return 0.0;

    max = fabs(mp->elements[0][0]);

    row_ptr =  mp->elements;

    for (i = 0; i<mp->num_rows; i++)
    {
        element_ptr = *row_ptr;

        for (j = 0; j<mp->num_cols; j++)
        {
            abs_val = fabs(*element_ptr);

            if (abs_val > max)
            {
                max = abs_val;
            }
            element_ptr++;
        }
        row_ptr++;
    }

    return max;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                               max_matrix_element
 *
 * Returns the largest element of a matrix
 *
 * This routine returns the largest element in the matrix specified by "mp".
 *
 * Returns:
 *     largest matrix element.
 *
 * Index: matrices
 *
 * -----------------------------------------------------------------------------
*/
double max_matrix_element(const Matrix* mp)
{
    int    i;
    int    j;
    double   max;
    double** row_ptr;
    double*  element_ptr;


    if (mp->num_rows == 0) return 0.0;
    if (mp->num_cols == 0) return 0.0;

    max = mp->elements[0][0];

    row_ptr =  mp->elements;

    for (i = 0; i<mp->num_rows; i++)
    {
        element_ptr = *row_ptr;

        for (j = 0; j<mp->num_cols; j++)
        {
            if (*element_ptr > max)
            {
                max = *element_ptr;
            }
            element_ptr++;
        }
        row_ptr++;
    }

    return max;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                          get_max_matrix_element
 *
 * Finds the largest element of a matrix
 *
 * This routine finds the largest element in the matrix specified by "mp", and
 * puts it into *max_ptr. The row and column of the element are returned in
 * *row_ptr and *col_ptr, respectively. Any of these three values can be NULL,
 * if you are not interested.
 *
 * Returns:
 *     NO_ERROR on sucess or ERROR if there are problems.
 *
 * Index: matrices
 *
 * -----------------------------------------------------------------------------
*/

int get_max_matrix_element
(
    const Matrix* mp,
    double*       max_ptr,
    int*          max_i_ptr,
    int*          max_j_ptr
)
{
    int      i, j;
    double   max;
    double** row_ptr;
    double*  element_ptr;
    int      max_i;
    int      max_j;


    if (    (mp == NULL)
         || (mp->num_rows == 0)
         || (mp->num_cols == 0)
       )
     {
         /*
         // This could be made to simply set an error message and return error,
         // but for now, treat it as a bug.
         */
         SET_ARGUMENT_BUG();
         return ERROR;
     }

    max = mp->elements[0][0];
    max_i = 0;
    max_j = 0;

    row_ptr =  mp->elements;

    for (i = 0; i<mp->num_rows; i++)
    {
        element_ptr = *row_ptr;

        for (j = 0; j<mp->num_cols; j++)
        {
            if (*element_ptr > max)
            {
                max = *element_ptr;
                max_i = i;
                max_j = j;
            }
            element_ptr++;
        }
        row_ptr++;
    }

    if (max_ptr != NULL) *max_ptr = max;
    if (max_i_ptr != NULL) *max_i_ptr = max_i;
    if (max_j_ptr != NULL) *max_j_ptr = max_j;

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

int get_min_matrix_col_elements(Vector** result_vpp, const Matrix* mp)
{
    Vector* min_col_elements_vp;
    int     num_rows;
    int     num_cols;
    int     i;
    int     j;
    double**  row_ptr;
    double*   element_ptr;
    double*   min_col_ptr;


    num_rows = mp->num_rows;
    num_cols = mp->num_cols;

    if ((num_rows <= 0) || (num_cols <= 0))
    {
        SET_ARGUMENT_BUG();
        return ERROR;
    }

    ERE(get_target_vector(result_vpp, num_cols));
    min_col_elements_vp = *result_vpp;

    min_col_ptr = (min_col_elements_vp->elements);

    for (j=0; j<num_cols; j++)
    {
        *min_col_ptr = (mp->elements)[ 0 ][j];
        min_col_ptr++;
    }

    row_ptr = (mp->elements) + 1;

    for (i = 1; i<num_rows; i++)
    {
        element_ptr = *row_ptr;
        min_col_ptr = (min_col_elements_vp->elements);

        for (j=0; j<num_cols; j++)
        {
            if ((respect_missing_values()) && (IS_MISSING_DBL(*min_col_ptr)))
            {
                *min_col_ptr = *element_ptr;
            }
            else if ((respect_missing_values()) && (IS_MISSING_DBL(*element_ptr)))
            {
                /*EMPTY*/
                ; /* Do nothing. */
            }
            else if (*element_ptr < *min_col_ptr)
            {
                *min_col_ptr = *element_ptr;
            }
            element_ptr++;
            min_col_ptr++;
        }
        row_ptr++;
    }

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

int get_max_matrix_col_elements(Vector** result_vpp, const Matrix* mp)
{
    Vector* max_col_elements_vp;
    int     num_rows;
    int     num_cols;
    int     i;
    int     j;
    double**  row_ptr;
    double*   element_ptr;
    double*   max_col_ptr;


    num_rows = mp->num_rows;
    num_cols = mp->num_cols;

    if ((num_rows <= 0) || (num_cols <= 0))
    {
        SET_ARGUMENT_BUG();
        return ERROR;
    }

    ERE(get_target_vector(result_vpp, num_cols));
    max_col_elements_vp = *result_vpp;

    max_col_ptr = (max_col_elements_vp->elements);

    for (j=0; j<num_cols; j++)
    {
        *max_col_ptr = (mp->elements)[ 0 ][j];
        max_col_ptr++;
    }

    row_ptr = (mp->elements) + 1;

    for (i = 1; i<num_rows; i++)
    {
        element_ptr = *row_ptr;
        max_col_ptr = (max_col_elements_vp->elements);

        for (j=0; j<num_cols; j++)
        {
            if ((respect_missing_values()) && (IS_MISSING_DBL(*max_col_ptr)))
            {
                *max_col_ptr = *element_ptr;
            }
            else if ((respect_missing_values()) && (IS_MISSING_DBL(*element_ptr)))
            {
                /*EMPTY*/
                ; /* Do nothing. */
            }
            else if (*element_ptr > *max_col_ptr)
            {
                *max_col_ptr = *element_ptr;
            }
            element_ptr++;
            max_col_ptr++;
        }
        row_ptr++;
    }

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                         get_max_matrix_row_elements
 *
 * Retrieves a vector with the max element from each matrix row.
 *
 * This routine extracts the max elements the rows of a matrix. The
 * max element for each row is returned as an element of the vector pointed to
 * by result_vpp. That vector is created or resized as need be.
 *
 * Returns:
 *    NO_ERROR on success, and ERROR if there is a problem with the error
 *    message being set.
 *
 * Index: matrices
 *
 * -----------------------------------------------------------------------------
*/

int get_max_matrix_row_elements(Vector** result_vpp, const Matrix* mp)
{


    return get_max_matrix_row_elements_2(result_vpp, (Int_vector**)NULL, mp);
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                         get_max_int_matrix_row_elements
 *
 * Retrieves a vector with the max element from each of the Int_matrix rows.
 *
 * This routine extracts the max elements the rows of a matrix. The
 * max element for each row is returned as an element of the vector pointed to
 * by result_vpp. That vector is created or resized as need be.
 *
 * Returns:
 *    NO_ERROR on success, and ERROR if there is a problem with the error
 *    message being set.
 *
 * Index: integer matrices
 *
 * -----------------------------------------------------------------------------
*/

int get_max_int_matrix_row_elements(Int_vector** result_ivpp, const Int_matrix* imp)
{


    return get_max_int_matrix_row_elements_2(result_ivpp, (Int_vector**)NULL, imp);
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

int get_min_matrix_row_elements(Vector** result_vpp, const Matrix* mp)
{

    UNTESTED_CODE();

    return get_min_matrix_row_elements_2(result_vpp, (Int_vector**)NULL, mp);
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

int get_max_matrix_row_elements_2
(
    Vector**      result_vpp,
    Int_vector**  index_vpp,
    const Matrix* mp
)
{
    int     num_rows, num_cols, i, j;
    double  max;
    int     index;
    double* max_col_ptr = NULL;
    int*    index_ptr   = NULL;


    num_rows = mp->num_rows;
    num_cols = mp->num_cols;

    if ((num_rows <= 0) || (num_cols <= 0))
    {
        SET_ARGUMENT_BUG();
        return ERROR;
    }

    if (result_vpp != NULL)
    {
        ERE(get_target_vector(result_vpp, num_rows));
        max_col_ptr = (*result_vpp)->elements;
    }

    if (index_vpp != NULL)
    {
        ERE(get_target_int_vector(index_vpp, num_rows));
        index_ptr = (*index_vpp)->elements;
    }

    for (i=0; i<num_rows; i++)
    {
        max = (mp->elements)[ i ][ 0 ];
        index = 0;

        for (j=1; j<num_cols; j++)
        {
            if ((mp->elements)[ i ][ j ] > max)
            {
                max = (mp->elements)[ i ][ j ];
                index = j;
            }
        }

        if (max_col_ptr != NULL)
        {
            *max_col_ptr = max;
            max_col_ptr++;
        }

        if (index_ptr != NULL)
        {
            *index_ptr = index;
            index_ptr++;
        }
    }

    return NO_ERROR;
}
/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

int get_max_int_matrix_row_elements_2
(
    Int_vector**  result_ivpp,
    Int_vector**  index_vpp,
    const Int_matrix* imp
)
{
    int     num_rows, num_cols, i, j;
    double  max;
    int     index;
    int*    max_col_ptr = NULL;
    int*    index_ptr   = NULL;


    num_rows = imp->num_rows;
    num_cols = imp->num_cols;

    if ((num_rows <= 0) || (num_cols <= 0))
    {
        SET_ARGUMENT_BUG();
        return ERROR;
    }

    if (result_ivpp != NULL)
    {
        ERE(get_target_int_vector(result_ivpp, num_rows));
        max_col_ptr = (*result_ivpp)->elements;
    }

    if (index_vpp != NULL)
    {
        ERE(get_target_int_vector(index_vpp, num_rows));
        index_ptr = (*index_vpp)->elements;
    }

    for (i=0; i<num_rows; i++)
    {
        max = (imp->elements)[ i ][ 0 ];
        index = 0;

        for (j=1; j<num_cols; j++)
        {
            if ((imp->elements)[ i ][ j ] > max)
            {
                max = (imp->elements)[ i ][ j ];
                index = j;
            }
        }

        if (max_col_ptr != NULL)
        {
            *max_col_ptr = max;
            max_col_ptr++;
        }

        if (index_ptr != NULL)
        {
            *index_ptr = index;
            index_ptr++;
        }
    }

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */


/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

int get_min_matrix_row_elements_2
(
    Vector**      result_vpp,
    Int_vector**  index_vpp,
    const Matrix* mp
)
{
    int     num_rows, num_cols, i, j;
    double  min;
    int     index;
    double* min_col_ptr = NULL;
    int*    index_ptr   = NULL;


    num_rows = mp->num_rows;
    num_cols = mp->num_cols;

    if ((num_rows <= 0) || (num_cols <= 0))
    {
        SET_ARGUMENT_BUG();
        return ERROR;
    }

    if (result_vpp != NULL)
    {
        ERE(get_target_vector(result_vpp, num_rows));
        min_col_ptr = (*result_vpp)->elements;
    }

    if (index_vpp != NULL)
    {
        ERE(get_target_int_vector(index_vpp, num_rows));
        index_ptr = (*index_vpp)->elements;
    }

    for (i=0; i<num_rows; i++)
    {
        min = (mp->elements)[ i ][ 0 ];
        index = 0;

        for (j=1; j<num_cols; j++)
        {
            if ((mp->elements)[ i ][ j ] < min)
            {
                min = (mp->elements)[ i ][ j ];
                index = j;
            }
        }

        if (min_col_ptr != NULL)
        {
            *min_col_ptr = min;
            min_col_ptr++;
        }

        if (index_ptr != NULL)
        {
            *index_ptr = index;
            index_ptr++;
        }
    }

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

int get_max_matrix_row_sum(const Matrix* mp, double* max_sum_ptr)
{
    int     num_rows;
    int     num_cols;
    int     i;
    int     j;
    double**  row_ptr;
    double*   element_ptr;
    double    sum;
    int     index;


    num_rows = mp->num_rows;
    num_cols = mp->num_cols;

    if ((num_rows <= 0) || (num_cols <= 0))
    {
        SET_ARGUMENT_BUG();
        return ERROR;
    }

    sum = 0.0;

    for (j=0; j<num_cols; j++)
    {
        sum += (mp->elements)[ 0 ][j];
    }

    index = 0;
    *max_sum_ptr = sum;

    row_ptr = (mp->elements) + 1;

    for (i = 1; i<num_rows; i++)
    {
        sum = 0.0;
        element_ptr = *row_ptr;

        for (j=0; j<num_cols; j++)
        {
            sum += *element_ptr;
            element_ptr++;
        }

        if (sum > *max_sum_ptr)
        {
            *max_sum_ptr = sum;
            index = i;
        }

        row_ptr++;
    }

    return index;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

int get_max_matrix_row_product(const Matrix* mp, double* max_product_ptr)
{
    int    num_rows, num_cols, i, j;
    double   max_product;
    double   product;
    int    index;
    double** row_ptr;
    double*  elem_ptr;


    UNTESTED_CODE();

    if ((mp == NULL) || (mp->num_rows <= 0) || (mp->num_cols <= 0))
    {
        SET_ARGUMENT_BUG();
        return ERROR;
    }

    num_rows = mp->num_rows;
    num_cols = mp->num_cols;

    row_ptr = mp->elements;

    elem_ptr = *row_ptr;
    product = 1.0;

    for (j=0; j<num_cols; j++)
    {
        product *= (*elem_ptr);
        elem_ptr++;
    }

    max_product = product;
    index = 0;

    row_ptr++;

    for (i=1; i<num_rows; i++)
    {
        elem_ptr = *row_ptr;
        product = 1.0;

        for (j=0; j<num_cols; j++)
        {
            product *= (*elem_ptr);
            elem_ptr++;
        }

        if (product > max_product)
        {
            max_product = product;
            index = i;
        }

        row_ptr++;
    }

    *max_product_ptr = max_product;

    return index;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                            min_thresh_matrix
 *
 * Limits matrix elements to a minimum value
 *
 * This routine sets all elements less than the specified threshold to the
 * threshold value. Elements in the matrix specified by the pointer to a matrix
 * object "mp" are compared to the threshold value "min". All elements less
 * than "min" are set equal to "min". The result is placed into the matrix
 * specified by "target_mpp", leaving the contents of "mp" unchanged.
 *
 * Returns:
 *     NO_ERROR on success, ERROR on failure.
 *
 * Documentor:
 *     Lindsay Martin
 *
 * Related:
 *     ow_min_thresh_matrix, max_thresh_matrix
 *
 * Index: matrices, matrix normalization
 *
 * -----------------------------------------------------------------------------
*/

int min_thresh_matrix
(
    Matrix**      target_mpp,
    const Matrix* source_mp,
    double        min_val
)
{
    Matrix* target_mp;
    int    i;
    int    j;
    int    num_rows;
    int    num_cols;
    double** in_row_ptr;
    double** out_row_ptr;
    double*  in_ptr;
    double*  out_ptr;


    ERE(get_target_matrix(target_mpp, source_mp->num_rows,
                          source_mp->num_cols));
    target_mp = *target_mpp;

    num_rows = source_mp->num_rows;
    num_cols = source_mp->num_cols;

    in_row_ptr = source_mp->elements;
    out_row_ptr = target_mp->elements;

    for (i=0; i<num_rows; i++)
    {
        in_ptr = *in_row_ptr;
        out_ptr = *out_row_ptr;

        for (j=0; j<num_cols; j++)
        {
            if (*in_ptr < min_val)
            {
                *out_ptr = min_val;
            }
            else
            {
                *out_ptr = *in_ptr;
            }

            in_ptr++;
            out_ptr++;
        }

        in_row_ptr++;
        out_row_ptr++;
    }

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                            ow_min_abs_thresh_matrix
 *
 * Limits matrix elements to a minimum absolute value
 *
 * This routine sets all elements with absolute value less than the specified
 * threshold to the threshold value. Elements in the matrix specified by the
 * pointer to a matrix object "mp" are compared to the threshold value "min".
 * All elements less than "min" are set equal to "min". The result is placed
 * back into the matrix specified by "mp", overwriting the original contents.
 *
 * Returns:
 *     NO_ERROR on success, ERROR on failure.
 *
 * Documentor:
 *     Lindsay Martin
 *
 * Related:
 *     min_thresh_matrix, ow_max_thresh_matrix
 *
 * Index: matrices, matrix normalization
 *
 * -----------------------------------------------------------------------------
*/

int ow_min_abs_thresh_matrix(Matrix* source_mp, double min_val)
{
    int    i;
    int    j;
    int    num_rows;
    int    num_cols;
    double** in_row_ptr;
    double*  in_ptr;


    num_rows = source_mp->num_rows;
    num_cols = source_mp->num_cols;

    in_row_ptr = source_mp->elements;

    for (i=0; i<num_rows; i++)
    {
        in_ptr = *in_row_ptr;

        for (j=0; j<num_cols; j++)
        {
            if (ABS_OF(*in_ptr) < min_val)
            {
                if (*in_ptr < 0.0)
                {
                    *in_ptr = -min_val;
                }
                else
                {
                    *in_ptr = min_val;
                }
            }

            in_ptr++;
        }

        in_row_ptr++;
    }

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                            ow_min_thresh_matrix
 *
 * Limits matrix elements to a minimum value
 *
 * This routine sets all elements less than the specified threshold to the
 * threshold value. Elements in the matrix specified by the pointer to a matrix
 * object "mp" are compared to the threshold value "min". All elements less
 * than "min" are set equal to "min". The result is placed back into the matrix
 * specified by "mp", overwriting the original contents.
 *
 * Returns:
 *     NO_ERROR on success, ERROR on failure.
 *
 * Documentor:
 *     Lindsay Martin
 *
 * Related:
 *     min_thresh_matrix, ow_max_thresh_matrix
 *
 * Index: matrices, matrix normalization
 *
 * -----------------------------------------------------------------------------
*/

int ow_min_thresh_matrix(Matrix* source_mp, double min_val)
{
    int    i;
    int    j;
    int    num_rows;
    int    num_cols;
    double** in_row_ptr;
    double*  in_ptr;


    num_rows = source_mp->num_rows;
    num_cols = source_mp->num_cols;

    in_row_ptr = source_mp->elements;

    for (i=0; i<num_rows; i++)
    {
        in_ptr = *in_row_ptr;

        for (j=0; j<num_cols; j++)
        {
            if (*in_ptr < min_val)
            {
                *in_ptr = min_val;
            }

            in_ptr++;
        }

        in_row_ptr++;
    }

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

int ow_min_thresh_matrix_col(Matrix* source_mp, int col, double min_val)
{
    int    i;
    int    num_rows;
    int    num_cols;
    double** in_row_ptr;
    double*  in_ptr;


    num_rows = source_mp->num_rows;
    num_cols = source_mp->num_cols;

    if (col >= num_cols)
    {
        SET_ARGUMENT_BUG();
        return ERROR;
    }

    in_row_ptr = source_mp->elements;

    for (i=0; i<num_rows; i++)
    {
        in_ptr = *in_row_ptr;

        if (in_ptr[ col ] < min_val)
        {
            in_ptr[ col ] = min_val;
        }

        in_row_ptr++;
    }

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                            max_thresh_matrix
 *
 * Limits matrix elements to a maximum value
 *
 * This routine sets all elements greater than the specified threshold to the
 * threshold value. Elements in the matrix specified by the pointer to a matrix
 * object "mp" are compared to the threshold value "max". All elements greater
 * than "max" are set equal to "max". The result is placed into the matrix
 * specified by "target_mpp", leaving the contents of "mp" unchanged.
 *
 * Returns:
 *     NO_ERROR on success, ERROR on failure.
 *
 * Documentor:
 *     Lindsay Martin
 *
 * Related:
 *     ow_max_thresh_matrix, min_thresh_matrix
 *
 * Index: matrices, matrix normalization
 *
 * -----------------------------------------------------------------------------
*/

int max_thresh_matrix
(
    Matrix**      target_mpp,
    const Matrix* source_mp,
    double        max_val
)
{
    Matrix* target_mp;
    int    i;
    int    j;
    int    num_rows;
    int    num_cols;
    double** in_row_ptr;
    double** out_row_ptr;
    double*  in_ptr;
    double*  out_ptr;


    ERE(get_target_matrix(target_mpp, source_mp->num_rows,
                          source_mp->num_cols));
    target_mp = *target_mpp;

    num_rows = source_mp->num_rows;
    num_cols = source_mp->num_cols;

    in_row_ptr = source_mp->elements;
    out_row_ptr = target_mp->elements;

    for (i=0; i<num_rows; i++)
    {
        in_ptr = *in_row_ptr;
        out_ptr = *out_row_ptr;

        for (j=0; j<num_cols; j++)
        {
            if (*in_ptr > max_val)
            {
                *out_ptr = max_val;
            }
            else
            {
                *out_ptr = *in_ptr;
            }

            in_ptr++;
            out_ptr++;
        }

        in_row_ptr++;
        out_row_ptr++;
    }

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                            ow_max_thresh_matrix
 *
 * Limits matrix elements to a maximum value
 *
 * This routine sets all elements greater than the specified threshold to the
 * threshold value. Elements in the matrix specified by the pointer to a matrix
 * object "mp" are compared to the threshold value "max". All elements greater
 * than "max" are set equal to "max". The result is placed back into the matrix
 * specified by "mp", overwriting the original contents.
 *
 * Returns:
 *     NO_ERROR on success, ERROR on failure.
 *
 * Documentor:
 *     Lindsay Martin
 *
 * Related:
 *     max_thresh_matrix, ow_min_thresh_matrix
 *
 * Index: matrices, matrix normalization
 *
 * -----------------------------------------------------------------------------
*/

int ow_max_thresh_matrix(Matrix* source_mp, double max_val)
{
    int    i;
    int    j;
    int    num_rows;
    int    num_cols;
    double** in_row_ptr;
    double*  in_ptr;


    num_rows = source_mp->num_rows;
    num_cols = source_mp->num_cols;

    in_row_ptr = source_mp->elements;

    for (i=0; i<num_rows; i++)
    {
        in_ptr = *in_row_ptr;

        for (j=0; j<num_cols; j++)
        {
            if (*in_ptr > max_val)
            {
                *in_ptr = max_val;
            }

            in_ptr++;
        }

        in_row_ptr++;
    }

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

int ow_max_thresh_matrix_col(Matrix* source_mp, int col, double max_val)
{
    int    i;
    int    num_rows;
    int    num_cols;
    double** in_row_ptr;
    double*  in_ptr;


    UNTESTED_CODE();

    num_rows = source_mp->num_rows;
    num_cols = source_mp->num_cols;

    if (col >= num_cols)
    {
        SET_ARGUMENT_BUG();
        return ERROR;
    }

    in_row_ptr = source_mp->elements;

    for (i=0; i<num_rows; i++)
    {
        in_ptr = *in_row_ptr;

        if (in_ptr[ col ] > max_val)
        {
            in_ptr[ col ] = max_val;
        }

        in_row_ptr++;
    }

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

int normalize_matrix_rows(Matrix** target_mpp, const Matrix* input_mp)
{

    ERE(copy_matrix(target_mpp, input_mp));

    return ow_normalize_matrix_rows(*target_mpp);
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

int ow_normalize_matrix_rows(Matrix* mp)
{
    int i;
    int j;
    double mag;
    int num_rows;
    int num_cols;
    double* row_pos;


    num_rows = mp->num_rows;
    num_cols = mp->num_cols;

    for (i = 0; i<num_rows; i++)
    {
        row_pos = mp->elements[ i ];
        mag = 0.0;

        for (j = 0; j<num_cols; j++)
        {
            mag += (*row_pos) * (*row_pos);
            row_pos++;
        }

        mag = sqrt(mag);

#ifdef USE_IS_ZERO_DBL_FOR_DIVIDE_BY_ZERO
        if ( ! IS_POSITIVE_DBL(mag))
        {
            set_error("Normalization of matrix rows failed.");
            add_error("Magnitide of matrix row is too close to zero.");
            return ERROR;
        }
#else
        /*
        // Exact compare is OK because overflow is handled separately.
        */
        if (mag == 0.0)
        {
            set_error("Normalization of matrix rows failed.");
            add_error("Magnitide of matrix row %d is zero.", i + 1);
            return ERROR;
        }
#endif

        row_pos = mp->elements[ i ];

        for (j = 0; j<num_cols; j++)
        {
            *row_pos /= mag;

#ifdef USE_COMPARE_WITH_REAL_MAX_FOR_DIVIDE_BY_ZERO
            if (    (*row_pos > DBL_MOST_POSITIVE)
                 || (*row_pos < DBL_MOST_NEGATIVE)
               )
            {
                set_error("Normalization of matrix rows failed.");
                add_error("Overflow occured normalizing row %d.", i + 1);
                add_error("Overflow occured dividing %.2e by %.2e.",
                          *row_pos, mag);
                return ERROR;
            }
#endif
            row_pos++;
        }
    }

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

int normalize_matrix_cols(Matrix** target_mpp, const Matrix* input_mp)
{

    ERE(copy_matrix(target_mpp, input_mp));

    return ow_normalize_matrix_cols(*target_mpp);
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

int ow_normalize_matrix_cols(Matrix* mp)
{
    int i;
    int j;
    double mag;
    int num_rows;
    int num_cols;


    num_rows = mp->num_rows;
    num_cols = mp->num_cols;

    for (j = 0; j<num_cols; j++)
    {
        mag = 0.0;

        for (i = 0; i<num_rows; i++)
        {
            mag += (mp->elements[ i ][ j ]) * (mp->elements[ i ][ j ]);
        }

        mag = sqrt(mag);

#ifdef USE_IS_ZERO_DBL_FOR_DIVIDE_BY_ZERO
        if ( ! IS_POSITIVE_DBL(mag))
        {
            set_error("Normalization of matrix column failed.");
            add_error("Magnitide of matrix column is too close to zero.");
            return ERROR;
        }
#else
        /*
        // Exact compare is OK because overflow is handled separately.
        */
        if (mag == 0.0)
        {
            set_error("Normalization of matrix column failed.");
            add_error("Magnitide of matrix column %d is zero.", j + 1);
            return ERROR;
        }
#endif

        for (i = 0; i<num_rows; i++)
        {
            mp->elements[ i ][ j ] /= mag;

#ifdef USE_COMPARE_WITH_REAL_MAX_FOR_DIVIDE_BY_ZERO
            if (    (mp->elements[ i ][ j ] > DBL_MOST_POSITIVE)
                 || (mp->elements[ i ][ j ] < DBL_MOST_NEGATIVE)
               )
            {
                set_error("Normalization of matrix columns failed.");
                add_error("Overflow occured normalizing column %d.", i + 1);
                add_error("Overflow occured dividing %.2e by %.2e.",
                           mp->elements[ i ][ j ], mag);
                return ERROR;
            }
#endif
        }
    }

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

#ifdef __cplusplus
}
#endif

