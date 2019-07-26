
/* $Id: m_mat_stat.c 20654 2016-05-05 23:13:43Z kobus $ */

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
#include "m/m_vec_stat.h"
#include "m/m_mat_stat.h"
#include "m/m_missing.h"

#ifdef __cplusplus
extern "C" {
#endif

/* -------------------------------------------------------------------------- */

#ifdef TEST
#ifdef PROGRAMMER_IS_kobus
#define REPORT_MISSING_VALUE_ARITHMETIC
#endif
#endif

/* -------------------------------------------------------------------------- */

double get_matrix_mean(const Matrix* mp)
{
    int count = (mp->num_rows)*(mp->num_cols);


    return sum_matrix_elements(mp) / count;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*
 * =============================================================================
 *                  average_matrix_vector_rows
 *
 * Averages matrix rows
 *
 * This routine averages the rows of a matrix putting the result into the vector
 * pointed to by *output_vpp (length num_cols).
 *
 * If we are respecting missing values, then they are excluded from the average.
 * If all values for a colum are missing then the average is set to DBL_MISSING.
 *
 * If *output_vpp is NULL, then a vector of the appropriate size is created.  If
 * it is the wrong size, it is resized. Finally, if it is the right size, then
 * the storage is recycled, as is.
 *
 * Returns:
 *     NO_ERROR on success, and ERROR on failure, with an error message being
 *     set.
 *
 * Index: matrices, matrix statistics, missing values
 *
 * -----------------------------------------------------------------------------
 */

int average_matrix_vector_rows
(
    Vector**      output_vpp,
    const Matrix_vector* input_mvp
)
{
    int length = input_mvp->length;
    int num_cols = NOT_SET;
    int num_rows;
    int col;
    int row;
    int i;


    for (i = 0; i < length; i++)
    {
        if (input_mvp->elements[ i ] != NULL)
        {
            if (num_cols == NOT_SET)
            {
                num_cols = input_mvp->elements[ i ]->num_cols;
            }
            else if (num_cols != input_mvp->elements[ i ]->num_cols)
            {
                set_error("Attempt to average the rows matrices with non-uniform number of rows.");
                return ERROR;
            }
        }
    }

    ERE(get_target_vector(output_vpp, num_cols));

    for (col = 0; col < num_cols; col++)
    {
        int count = 0;
        double sum = 0.0;

        for (i = 0; i < length; i++)
        {
            Matrix* input_mp = input_mvp->elements[ i ];

            if (input_mp == NULL) continue;

            num_rows = input_mp->num_rows;

            for (row = 0; row < num_rows; row++)
            {
                double val = input_mp->elements[ row ][ col ];

                if (    ( ! respect_missing_values())
                     || (IS_NOT_MISSING_DBL(val))
                   )
                {
                    sum += val;
                    count++;
                }
            }
        }

        if (count > 0)
        {
            (*output_vpp)->elements[ col ] = sum / count;
        }
        else
        {
            (*output_vpp)->elements[ col ] = DBL_MISSING;
        }
    }

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*
 * =============================================================================
 *                            average_matrix_rows
 *
 * Averages matrix rows.
 *
 * This routine averages the rows of a matrix, putting the result into the
 * vector pointed to by *output_vpp (length num_cols). If *output_vpp is
 * NULL, then a vector of the appropriate size is created. If it is the wrong
 * size, it is resized. Finally, if it is the right size, then the storage is
 * recycled, as is.
 *
 * If we are respecting missing values, then they are excluded from the average.
 * If all values for a colum are missing then the average is set to DBL_MISSING.
 *
 * Returns:
 *     NO_ERROR on success, and ERROR on failure, with an error message being
 *     set.
 *
 * Index: matrices, matrix statistics
 *
 * -----------------------------------------------------------------------------
 */

int average_matrix_rows(Vector** output_vpp, const Matrix* input_mp)
{


    if (respect_missing_values())
    {
        return average_matrix_rows_without_missing(output_vpp, input_mp);
    }

    ERE(sum_matrix_rows(output_vpp, input_mp));
    ERE(ow_divide_vector_by_scalar(*output_vpp, (double)input_mp->num_rows));

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*
 * =============================================================================
 *                  average_matrix_rows_without_missing
 *
 * Averages matrix rows with negative values ignored
 *
 * This routine averages the rows of a matrix putting the result into the vector
 * pointed to by *output_vpp (length num_cols).  Missing values, identified by
 * being close to DBL_MISSING are ignored. If all values for a colum are missing
 * then the average is DBL_MISSING.
 *
 * If *output_vpp is NULL, then a vector of the appropriate size is created.  If
 * it is the wrong size, it is resized. Finally, if it is the right size, then
 * the storage is recycled, as is.
 *
 * Returns:
 *     NO_ERROR on success, and ERROR on failure, with an error message being
 *     set.
 *
 * Index: matrices, matrix statistics, missing values
 *
 * -----------------------------------------------------------------------------
 */

int average_matrix_rows_without_missing
(
    Vector**      output_vpp,
    const Matrix* input_mp
)
{
    int num_cols = input_mp->num_cols;
    int col;
    int num_rows = input_mp->num_rows;
    int row;
    Vector* output_vp;


    ERE(get_zero_vector(output_vpp, num_cols));
    output_vp = *output_vpp;

    for (col = 0; col < num_cols; col++)
    {
        int count = 0;

        for (row = 0; row < num_rows; row++)
        {
            double val = input_mp->elements[ row ][ col ];

            if (IS_NOT_MISSING_DBL(val))
            {
                output_vp->elements[ col ] += val;
                count++;
            }
        }

        if (count > 0)
        {
            output_vp->elements[ col ] /= (double)count;
        }
        else
        {
            output_vp->elements[ col ] = DBL_MISSING;
        }
    }

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*
 * =============================================================================
 *                  average_matrix_rows_without_negatives
 *
 * Averages matrix rows with negative values ignored
 *
 * This routine averages the rows of a matrix putting the result into the vector
 * pointed to by *output_vpp (length num_cols). Negative values are ignored. If
 * all values for a colum are negative then the average is negative (specially,
 * DBL_MISSING). If *output_vpp is NULL, then a vector of the appropriate size
 * is created. If it is the wrong size, it is resized. Finally, if it is the
 * right size, then the storage is recycled, as is.
 *
 * Returns:
 *     NO_ERROR on success, and ERROR on failure, with an error message being
 *     set.
 *
 * Index: matrices, matrix statistics
 *
 * -----------------------------------------------------------------------------
 */

int average_matrix_rows_without_negatives
(
    Vector**      output_vpp,
    const Matrix* input_mp
)
{
    int num_cols = input_mp->num_cols;
    int col;
    int num_rows = input_mp->num_rows;
    int row;
    Vector* output_vp;


    ERE(get_zero_vector(output_vpp, num_cols));
    output_vp = *output_vpp;

    for (col = 0; col < num_cols; col++)
    {
        int count = 0;

        for (row = 0; row < num_rows; row++)
        {
            double val = input_mp->elements[ row ][ col ];

            if (val >= 0.0)
            {
                output_vp->elements[ col ] += val;
                count++;
            }
        }

        if (count > 0)
        {
            output_vp->elements[ col ] /= (double)count;
        }
        else
        {
            output_vp->elements[ col ] = DBL_MISSING;
        }
    }

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*
 * =============================================================================
 *                               sum_matrix_rows
 *
 * Sums matrix rows
 *
 * This routine sums the rows of a matrix putting the result into the vector
 * pointed to by *output_vpp which becomes a vector of size num_cols.
 *
 * If we are respecting missing values, then they are excluded from the average.
 * If all values for a colum are missing then the average is set to DBL_MISSING.
 *
 * If *output_vpp is NULL, then a vector of the appropriate size is created. If
 * it is the wrong size, it is resized. Finally, if it is the right size, then
 * the storage is recycled, as is.
 *
 * Returns:
 *     NO_ERROR on success, and ERROR on failure, with an error message being
 *     set.
 *
 * Index: matrices, matrix statistics
 *
 * -----------------------------------------------------------------------------
 */

int sum_matrix_rows(Vector** output_vpp, const Matrix* input_mp)
{


    if (respect_missing_values())
    {
        return sum_matrix_rows_without_missing(output_vpp, input_mp);
    }

    ERE(get_target_vector(output_vpp, input_mp->num_cols));

    return ow_sum_matrix_rows(*output_vpp, input_mp);
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*
 * An ow_ version of sum_matrix_rows() does not really make sense, but it is the
 * easiest solution to forcing the answer to go into a specified matrix which is
 * done in annotate.c to put the results into a subset of a vector.
*/

int ow_sum_matrix_rows(Vector* output_vp, const Matrix* input_mp)
{
    int     i;
    int     j;
    double*   row_pos;
    double*   out_pos;


    ERE(ow_zero_vector(output_vp));

    for (i=0; i<input_mp->num_rows; i++)
    {
        row_pos = input_mp->elements[ i ];
        out_pos = output_vp->elements;

        for (j=0; j<input_mp->num_cols; j++)
        {
            *out_pos += *row_pos;
            out_pos++;
            row_pos++;
        }
    }

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*
 * =============================================================================
 *                      sum_matrix_rows_without_missing
 *
 * Sums matrix rows with missing values ignored
 *
 * This routine sums the rows of a matrix putting the result into the vector
 * pointed to by *output_vpp which becomes a vector of size num_cols.  Missing
 * values, identified by being close to DBL_MISSING are ignored. If all values
 * for a colum are missing then the average is DBL_MISSING.
 *
 * If *output_vpp is NULL, then a vector of the appropriate size is created. If
 * it is the wrong size, it is resized. Finally, if it is the right size, then
 * the storage is recycled, as is.
 *
 * Returns:
 *     NO_ERROR on success, and ERROR on failure, with an error message being
 *     set.
 *
 * Index: matrices, matrix statistics, missing values
 *
 * -----------------------------------------------------------------------------
 */

int sum_matrix_rows_without_missing
(
    Vector**      output_vpp,
    const Matrix* input_mp
)
{
    int num_cols = input_mp->num_cols;
    int col;
    int num_rows = input_mp->num_rows;
    int row;
    Vector* output_vp;


    ERE(get_zero_vector(output_vpp, num_cols));
    output_vp = *output_vpp;

    for (col = 0; col < num_cols; col++)
    {
        int count = 0;

        for (row = 0; row < num_rows; row++)
        {
            double val = input_mp->elements[ row ][ col ];

            if (IS_NOT_MISSING_DBL(val))
            {
                output_vp->elements[ col ] += val;
                count++;
            }
        }

        if (count <= 0)
        {
            output_vp->elements[ col ] = DBL_MISSING;
        }
    }

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*
 * =============================================================================
 *                  sum_matrix_rows_without_negatives
 *
 * Sums matrix rows with negative values ignored
 *
 * This routine sums the rows of a matrix putting the result into the vector
 * pointed to by *output_vpp (length num_cols). Negative values are ignored. If
 * all values for a colum are negative then the average is negative (specially,
 * DBL_MISSING). If *output_vpp is NULL, then a vector of the appropriate size
 * is created. If it is the wrong size, it is resized. Finally, if it is the
 * right size, then the storage is recycled, as is.
 *
 * Returns:
 *     NO_ERROR on success, and ERROR on failure, with an error message being
 *     set.
 *
 * Index: matrices, matrix statistics
 *
 * -----------------------------------------------------------------------------
 */

int sum_matrix_rows_without_negatives
(
    Vector**      output_vpp,
    const Matrix* input_mp
)
{
    int num_cols = input_mp->num_cols;
    int col;
    int num_rows = input_mp->num_rows;
    int row;
    Vector* output_vp;


    ERE(get_zero_vector(output_vpp, num_cols));
    output_vp = *output_vpp;

    for (col = 0; col < num_cols; col++)
    {
        int count = 0;

        for (row = 0; row < num_rows; row++)
        {
            double val = input_mp->elements[ row ][ col ];

            if (val >= 0.0)
            {
                output_vp->elements[ col ] += val;
                count++;
            }
        }

        if (count <= 0)
        {
            output_vp->elements[ col ] = DBL_MISSING;
        }
    }

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*
 * =============================================================================
 *                  get_matrix_row_stats
 *
 * Computes mean and stdev of matrix rows
 *
 * This routine computes the means and stdev of the rows of a matrix and puts
 * the result into the vectors pointed to by *mean_vpp and *stdev_vpp
 * respetively.
 *
 * If we are respecting missing values, then they are excluded from the average.
 * If all values for a colum are missing then the average is set to DBL_MISSING.
 *
 * If either the maean or the standard deviation is not needed, then mean_vpp or
 * stdev_vpp can be NULL.
 *
 * If *mean_vpp or *stdev_vpp are NULL, then a vector of the appropriate size is
 * created.  If it is the wrong size, it is resized. Finally, if it is the right
 * size, then the storage is recycled, as is.
 *
 * Returns:
 *     NO_ERROR on success, and ERROR on failure, with an error message being
 *     set.
 *
 * Index: matrices, matrix statistics, missing values
 *
 * -----------------------------------------------------------------------------
 */

int get_matrix_row_stats
(
    const Matrix* mp,
    Vector**      mean_vpp,
    Vector**      stdev_vpp
)
{
    int num_rows = mp->num_rows;
    int num_cols = mp->num_cols;
    int          i, j;
    Vector* mean_storage_vp = NULL;
    Vector* stdev_vp = NULL;
    Vector* mean_vp;
    Int_vector* counts_vp = NULL;
    int result = NO_ERROR;


    /*
    // We don't use the computational trick involving subtracting sums of
    // squares and squares of sums, because of numerical stabilty risks. Better
    // to be a bit slower.
    */

    if (mean_vpp != NULL)
    {
        ERE(average_matrix_rows(mean_vpp, mp));
        mean_vp = *mean_vpp;
    }
    else
    {
        ERE(average_matrix_rows(&mean_storage_vp, mp));
        mean_vp = mean_storage_vp;
    }

    if (stdev_vpp == NULL)
    {
        free_vector(mean_storage_vp);
        return NO_ERROR;
    }

    if (num_rows < 2)
    {
        set_error("Cannot compute the variance of a matrix with less than two rows.");
        free_vector(mean_storage_vp);
        return ERROR;
    }

    result = get_zero_vector(stdev_vpp, num_cols);

    if (result != ERROR)
    {
        result = get_zero_int_vector(&counts_vp, num_cols);
    }

    if (result != ERROR)
    {
        stdev_vp = *stdev_vpp;

        for (i=0; i<num_rows; i++)
        {
            double* stdev_ptr = stdev_vp->elements;

            for (j = 0; j < num_cols; j++)
            {
                double t;

                if (     (  ! respect_missing_values())
                     ||  (    (IS_NOT_MISSING_DBL(mp->elements[ i ][ j ]))
                           && (IS_NOT_MISSING_DBL(mean_vp->elements[ j ]))
                         )
                   )
                {
                    t = mp->elements[ i ][ j ] - mean_vp->elements[ j ];
                    (*stdev_ptr) += t*t;
                    (counts_vp->elements[ j ])++;
                }

                stdev_ptr++;
            }
        }

        for (j = 0; j < num_cols; j++)
        {
            if (counts_vp->elements[ j ] > 1)
            {
                stdev_vp->elements[ j ] /= (counts_vp->elements[ j ] - 1.0);
            }
            else
            {
                stdev_vp->elements[ j ] = DBL_MISSING;
            }
        }
    }

    if (result != ERROR)
    {
        result = ow_sqrt_vector(stdev_vp);
    }

    free_vector(mean_storage_vp);
    free_int_vector(counts_vp);

    return result;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*
 * =============================================================================
 *                  get_matrix_row_stats_2
 *
 * Computes mean, and covariance matrix rows
 *
 * This routine computes the means and covariance of the rows of a matrix and
 * puts the result into the vectors pointed to by *mean_vpp and *cov_mpp
 * respetively.
 *
 * If we are respecting missing values, then they are excluded from the average.
 * If all values for a colum are missing then the average is set to DBL_MISSING.
 *
 * If either the maean or the covariance is not needed, then mean_vpp or
 * cov_mpp can be NULL.
 *
 * If *mean_vpp is not NULL then a vector of the appropriate size is created.
 * If it is the wrong size, it is resized. Finally, if it is the right size,
 * then the storage is recycled, as is.
 *
 * If *cov_mpp is NULL, then a matrix of the appropriate size is created.  If it
 * is the wrong size, it is resized. Finally, if it is the right size, then the
 * storage is recycled, as is.
 *
 * Returns:
 *     NO_ERROR on success, and ERROR on failure, with an error message being
 *     set.
 *
 * Index: matrices, matrix statistics, missing values
 *
 * -----------------------------------------------------------------------------
 */

int get_matrix_row_stats_2
(
    const Matrix* mp,
    Vector**      mean_vpp,
    Matrix**      cov_mpp
)
{
    int num_rows = mp->num_rows;
    int num_cols = mp->num_cols;
    int          i, j;
    Vector* mean_storage_vp = NULL;
    Vector* mean_vp;
    int count = 0; 
    int result = NO_ERROR;
    Vector* matrix_row_vp = NULL;
    Matrix* outer_prod_mp = NULL; 


    /*
    // We don't use the computational trick involving subtracting sums of
    // squares and squares of sums, because of numerical stabilty risks. Better
    // to be a bit slower.
    */

    if (mean_vpp != NULL)
    {
        ERE(average_matrix_rows(mean_vpp, mp));
        mean_vp = *mean_vpp;
    }
    else
    {
        ERE(average_matrix_rows(&mean_storage_vp, mp));
        mean_vp = mean_storage_vp;
    }

    if (cov_mpp == NULL)
    {
        free_vector(mean_storage_vp);
        return NO_ERROR;
    }

    if (num_rows < 2)
    {
        set_error("Cannot compute the covariance of a matrix with less than two rows.");
        free_vector(mean_storage_vp);
        return ERROR;
    }

    result = get_zero_matrix(cov_mpp, num_cols, num_cols);

    if (result != ERROR)
    {
        for (i=0; i<num_rows; i++)
        {
            result = get_matrix_row(&matrix_row_vp, mp, i);
            if (result == ERROR) { NOTE_ERROR(); break; }

            if (respect_missing_values())
            {
                int has_missing = FALSE;

                UNTESTED_CODE();

                for (j = 0; j < num_cols; j++)
                {
                    if (IS_MISSING_DBL(matrix_row_vp->elements[ j ]))
                    {
                        has_missing = TRUE;
                        break;
                    }
                }

                if (has_missing) 
                {
                    continue;
                }
            }

            result = ow_subtract_vectors(matrix_row_vp, mean_vp); 
            if (result == ERROR) { NOTE_ERROR(); break; }


            result = get_vector_outer_product(&outer_prod_mp, matrix_row_vp, matrix_row_vp); 
            if (result == ERROR) { NOTE_ERROR(); break; }

            result = ow_add_matrices(*cov_mpp, outer_prod_mp); 
            if (result == ERROR) { NOTE_ERROR(); break; }

            count++; 
        }

        if (count > 1)
        {
            result = ow_divide_matrix_by_scalar(*cov_mpp, (double)(count - 1));
        }
        else
        {
            result = ow_set_matrix(*cov_mpp, DBL_MISSING); 
        }
    }

    free_vector(matrix_row_vp);
    free_vector(mean_storage_vp);
    free_matrix(outer_prod_mp); 

    return result;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*
 * =============================================================================
 *                            average_matrix_cols
 *
 * Averages matrix columns.
 *
 * This routine averages the columns of a matrix, putting the result into the
 * vector pointed to by *output_vpp which becomes a vector of size num_rows. If
 * *output_vpp is NULL, then a vector of the appropriate size is created. If it
 * is the wrong size, it is resized. Finally, if it is the right size, then the
 * storage is recycled, as is.
 *
 * If we are respecting missing values, then they are excluded from the average.
 * If all values for a colum are missing then the average is set to DBL_MISSING.
 *
 * Returns:
 *     NO_ERROR on success, and ERROR on failure, with an error message being
 *     set.
 *
 * Index: matrices, matrix statistics
 *
 * -----------------------------------------------------------------------------
 */

int average_matrix_cols(Vector** output_vpp, const Matrix* input_mp)
{


    if (respect_missing_values())
    {
        return average_matrix_cols_without_missing(output_vpp, input_mp);
        UNTESTED_CODE();
    }

    ERE(sum_matrix_cols(output_vpp, input_mp));
    ERE(ow_divide_vector_by_scalar(*output_vpp, (double)input_mp->num_cols));

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*
 * =============================================================================
 *                  average_matrix_cols_without_missing
 *
 * Averages matrix columns with negative values ignored
 *
 * This routine averages the colums of a matrix putting the result into the
 * vector pointed to by *output_vpp  which becomes a vector of size num_rows.
 * Missing values, identified by being close to DBL_MISSING are ignored. If all
 * values for a colum are missing then the average is DBL_MISSING.
 *
 * If *output_vpp is NULL, then a vector of the appropriate size is created. If
 * it is the wrong size, it is resized.  Finally, if it is the right size, then
 * the storage is recycled, as is.
 *
 * Returns:
 *     NO_ERROR on success, and ERROR on failure, with an error message being
 *     set.
 *
 * Index: matrices, matrix statistics, missing values
 *
 * -----------------------------------------------------------------------------
 */

int average_matrix_cols_without_missing
(
    Vector**      output_vpp,
    const Matrix* input_mp
)
{
    int     num_rows  = input_mp->num_rows;
    int     num_cols  = input_mp->num_cols;
    int     col, row;
    Vector* output_vp;


    ERE(get_zero_vector(output_vpp, num_rows));
    output_vp = *output_vpp;

    for (row = 0; row < num_rows; row++)
    {
        int count = 0;

        for (col = 0; col < num_cols; col++)
        {
            double val = input_mp->elements[ row ][ col ];

            if (IS_NOT_MISSING_DBL(val))
            {
                output_vp->elements[ row ] += val;
                count++;
            }
        }

        if (count > 0)
        {
            output_vp->elements[ row ] /= (double)count;
        }
        else
        {
            output_vp->elements[ row ] = DBL_MISSING;
        }
    }

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*
 * =============================================================================
 *                  average_matrix_cols_without_negatives
 *
 * Averages matrix columns with negative values ignored
 *
 * This routine averages the columss of a matrix putting the result into the
 * vector pointed to by *output_vpp which becomes a vector of size num_rows.
 * Negative values are ignored. If all values for a colum are negative then the
 * average is negative (specially, DBL_MISSING). If *output_vpp is NULL, then a
 * vector of the appropriate size is created. If it is the wrong size, it is
 * resized.  Finally, if it is the right size, then the storage is recycled, as
 * is.
 *
 * Returns:
 *     NO_ERROR on success, and ERROR on failure, with an error message being
 *     set.
 *
 * Index: matrices, matrix statistics
 *
 * -----------------------------------------------------------------------------
 */

int average_matrix_cols_without_negatives
(
    Vector**      output_vpp,
    const Matrix* input_mp
)
{
    int     num_rows  = input_mp->num_rows;
    int     num_cols  = input_mp->num_cols;
    int     col, row;
    Vector* output_vp;


    ERE(get_zero_vector(output_vpp, num_rows));
    output_vp = *output_vpp;

    for (row = 0; row < num_rows; row++)
    {
        int count = 0;

        for (col = 0; col < num_cols; col++)
        {
            double val = input_mp->elements[ row ][ col ];

            if (val >= 0.0)
            {
                output_vp->elements[ row ] += val;
                count++;
            }
        }

        if (count > 0)
        {
            output_vp->elements[ row ] /= (double)count;
        }
        else
        {
            output_vp->elements[ row ] = DBL_MISSING;
        }
    }

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*
 * =============================================================================
 *                            sum_matrix_cols
 *
 * Sums matrix cols
 *
 * This routine sums the colums of a matrix putting the result into the vector
 * pointed to by *output_vpp which will be a vector with length num_rows.  If
 * *output_vpp is NULL, then a vector of the appropriate size is created. If it
 * is the wrong size, it is resized.  Finally, if it is the right size, then the
 * storage is recycled, as is.
 *
 * If we are respecting missing values, then they are excluded from the average.
 * If all values for a colum are missing then the average is set to DBL_MISSING.
 *
 * Returns:
 *     NO_ERROR on success, and ERROR on failure, with an error message being
 *     set.
 *
 * Index: matrices, matrix statistics
 *
 * -----------------------------------------------------------------------------
 */

int sum_matrix_cols(Vector** output_vpp, const Matrix* input_mp)
{
    Vector* output_vp;
    int     i;
    int     j;
    double*   row_pos;
    double    sum;


    if (respect_missing_values())
    {
        return sum_matrix_cols_without_missing(output_vpp, input_mp);
    }

    ERE(get_target_vector(output_vpp, input_mp->num_rows));
    output_vp = *output_vpp;

    for (i=0; i<input_mp->num_rows; i++)
    {
        sum = 0.0;
        row_pos = input_mp->elements[ i ];

        for (j=0; j<input_mp->num_cols; j++)
        {
            sum += *row_pos++;
        }

        output_vp->elements[ i ] = sum;
    }

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*
 * =============================================================================
 *                  sum_matrix_cols_without_missing
 *
 * Sums matrix cols with missing values ignored
 *
 * This routine sums the columns of a matrix putting the result into the
 * vector pointed to by *output_vpp which will be a vector with length num_rows.
 * Missing values, identified by being close to DBL_MISSING are ignored. If all
 * values for a colum are missing then the average is DBL_MISSING.
 *
 * If *output_vpp is NULL, then a vector of the appropriate size is created. If
 * it is the wrong size, it is resized.  Finally, if it is the right size, then
 * the storage is recycled, as is.
 *
 * Returns:
 *     NO_ERROR on success, and ERROR on failure, with an error message being
 *     set.
 *
 * Index: matrices, matrix statistics, missing values
 *
 * -----------------------------------------------------------------------------
 */

int sum_matrix_cols_without_missing
(
    Vector**      output_vpp,
    const Matrix* input_mp
)
{
    int     num_rows  = input_mp->num_rows;
    int     num_cols  = input_mp->num_cols;
    int     col, row;
    Vector* output_vp;


    ERE(get_zero_vector(output_vpp, num_rows));
    output_vp = *output_vpp;

    for (row = 0; row < num_rows; row++)
    {
        int count = 0;

        for (col = 0; col < num_cols; col++)
        {
            double val = input_mp->elements[ row ][ col ];

            if (IS_NOT_MISSING_DBL(val))
            {
                output_vp->elements[ row ] += val;
                count++;
            }
        }

        if (count <= 0)
        {
            output_vp->elements[ row ] = DBL_MISSING;
        }
    }

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*
 * =============================================================================
 *                  sum_matrix_cols_without_negatives
 *
 * Sums matrix cols with negative values ignored
 *
 * This routine sums the columns of a matrix putting the result into the
 * vector pointed to by *output_vpp which will be a vector with length num_rows.
 * Negative values are ignored. If all values for a colum are negative then the
 * average is negative (specially, DBL_MISSING). If *output_vpp is NULL, then a
 * vector of the appropriate size is created. If it is the wrong size, it is
 * resized.  Finally, if it is the right size, then the storage is recycled, as
 * is.
 *
 * Returns:
 *     NO_ERROR on success, and ERROR on failure, with an error message being
 *     set.
 *
 * Index: matrices, matrix statistics
 *
 * -----------------------------------------------------------------------------
 */

int sum_matrix_cols_without_negatives
(
    Vector**      output_vpp,
    const Matrix* input_mp
)
{
    int     num_rows  = input_mp->num_rows;
    int     num_cols  = input_mp->num_cols;
    int     col, row;
    Vector* output_vp;


    ERE(get_zero_vector(output_vpp, num_rows));
    output_vp = *output_vpp;

    for (row = 0; row < num_rows; row++)
    {
        int count = 0;

        for (col = 0; col < num_cols; col++)
        {
            double val = input_mp->elements[ row ][ col ];

            if (val >= 0.0)
            {
                output_vp->elements[ row ] += val;
                count++;
            }
        }

        if (count <= 0)
        {
            output_vp->elements[ row ] = DBL_MISSING;
        }
    }

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *
 *                  get_fixed_clustering_of_3D_data
 *
 *  Puts data into bins and averages the bins
 *
 *  Puts data into bins and averages the bins. The result data is all the
 *  averages. Where there was no data in a bin, there is no resulting data
 *  point. Thus there is a maximum of resolution^3 data points, and often quite
 *  a bit less.
 *
 * -----------------------------------------------------------------------------
*/

int get_fixed_clustering_of_3D_data
(
    Matrix**      result_mpp,
    const Matrix* mp,
    int           resolution
)
{
    int       num_rows, num_cols;
    int***    counts;
    double*** x_scores;
    double*** y_scores;
    double*** z_scores;
    Vector*   offset_vp    = NULL;
    Vector*   range_vp     = NULL;
    Vector*   step_vp      = NULL;
    int       i, j, k;
    int       count;


    num_rows = mp->num_rows;
    num_cols = mp->num_cols;

    if ((num_cols != 3) || (num_rows < 1) || (resolution < 1))
    {
        SET_ARGUMENT_BUG();
        return ERROR;
    }

    NRE(counts = allocate_3D_int_array(resolution, resolution, resolution));
    NRE(x_scores = allocate_3D_double_array(resolution, resolution,resolution));
    NRE(y_scores = allocate_3D_double_array(resolution, resolution,resolution));
    NRE(z_scores = allocate_3D_double_array(resolution, resolution,resolution));

    for (i=0; i<resolution; i++)
    {
        for (j=0; j<resolution; j++)
        {
            for (k=0; k<resolution; k++)
            {
                counts[ i ][ j ][ k ] = 0;

                x_scores[ i ][ j ][ k ] = 0.0;
                y_scores[ i ][ j ][ k ] = 0.0;
                z_scores[ i ][ j ][ k ] = 0.0;
            }
        }
    }

    ERE(get_max_matrix_col_elements(&range_vp, mp));

    /* Just in case one of the elements is zero (which is a problem if the min
    // is zero also).
    */
    ERE(ow_add_scalar_to_vector(range_vp, DBL_EPSILON));

    /*
    // Expand the range in case max and min are the same, and to guard against
    // the max bining one too high.
    */
    ERE(ow_multiply_vector_by_scalar(range_vp, 1.0 + 0.5 / resolution));

    ERE(get_min_matrix_col_elements(&offset_vp, mp));

    /* Just in case one of the elements is zero (which is a problem if the max
    // is zero also).
    */
    ERE(ow_subtract_scalar_from_vector(offset_vp, DBL_EPSILON));
    ERE(ow_subtract_vectors(range_vp, offset_vp));

    /*
    // Expand the range in case max and min are the same, and to guard against
    // the max bining one too high.
    */
    ERE(ow_multiply_vector_by_scalar(range_vp, 1.0 + 0.5 / resolution));

    ERE(divide_vector_by_scalar(&step_vp, range_vp, (double)resolution));

    for (count = 0; count < num_rows; count++)
    {
        i = (int)((mp->elements[ count ][ 0 ] - offset_vp->elements[ 0 ]) /
                                                        step_vp->elements[ 0 ]);
        j = (int)((mp->elements[ count ][ 1 ] - offset_vp->elements[ 1 ]) /
                                                        step_vp->elements[ 1 ]);
        k = (int)((mp->elements[ count ][ 2 ] - offset_vp->elements[ 2 ]) /
                                                        step_vp->elements[ 2 ]);

        x_scores[ i ][ j ][ k ] += mp->elements[ count ][ 0 ];
        y_scores[ i ][ j ][ k ] += mp->elements[ count ][ 1 ];
        z_scores[ i ][ j ][ k ] += mp->elements[ count ][ 2 ];

        counts[ i ][ j ][ k ]++;
    }

    count = 0;

    /* Dry run */

    for (i=0; i<resolution; i++)
    {
        for (j=0; j<resolution; j++)
        {
            for (k=0; k<resolution; k++)
            {
                if (counts[ i ][ j ][ k ] > 0)
                {
                    count++;
                }
            }
        }
    }

    if (count <= 0)
    {
        SET_CANT_HAPPEN_BUG();
        return ERROR;
    }


    ERE(get_target_matrix(result_mpp, count, 3));
    count = 0;

    for (i=0; i<resolution; i++)
    {
        for (j=0; j<resolution; j++)
        {
            for (k=0; k<resolution; k++)
            {
                if (counts[ i ][ j ][ k ] > 0)
                {
                    (*result_mpp)->elements[ count ][ 0 ] =
                            x_scores[ i ][ j ][ k ] / counts[ i ][ j ][ k ];

                    (*result_mpp)->elements[ count ][ 1 ] =
                            y_scores[ i ][ j ][ k ] / counts[ i ][ j ][ k ];

                    (*result_mpp)->elements[ count ][ 2 ] =
                            z_scores[ i ][ j ][ k ] / counts[ i ][ j ][ k ];
                    count++;
                }
            }
        }
    }

    free_vector(offset_vp);
    free_vector(step_vp);
    free_vector(range_vp);
    free_3D_int_array(counts);
    free_3D_double_array(x_scores);
    free_3D_double_array(y_scores);
    free_3D_double_array(z_scores);

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                         get_fixed_cluster_average_of_3D_data
 *
 *  Average of the data reduced by get_fixed_clustering_of_3D_data
 *
 *  Basically returns the average of the data reduced by
 *  get_fixed_clustering_of_3D_data, but is implemented completely separately,
 *  because if we only want the average, then it can be computed faster.
 *
 * -----------------------------------------------------------------------------
*/

int get_fixed_cluster_average_of_3D_data
(
    Vector**      result_vpp,
    const Matrix* mp,
    int           resolution
)
{
    int       num_rows, num_cols;
    int***    counts;
    double*** x_scores;
    double*** y_scores;
    double*** z_scores;
    Vector*   offset_vp    = NULL;
    Vector*   range_vp     = NULL;
    Vector*   step_vp      = NULL;
    int       result;
    int       i, j, k;
    int       count;


    num_rows = mp->num_rows;
    num_cols = mp->num_cols;

    if ((num_cols != 3) || (num_rows < 1) || (resolution < 1))
    {
        SET_ARGUMENT_BUG();
        return ERROR;
    }

    ERE(get_zero_vector(result_vpp, num_cols));

    NRE(counts = allocate_3D_int_array(resolution, resolution, resolution));
    NRE(x_scores = allocate_3D_double_array(resolution, resolution,resolution));
    NRE(y_scores = allocate_3D_double_array(resolution, resolution,resolution));
    NRE(z_scores = allocate_3D_double_array(resolution, resolution,resolution));

    for (i=0; i<resolution; i++)
    {
        for (j=0; j<resolution; j++)
        {
            for (k=0; k<resolution; k++)
            {
                counts[ i ][ j ][ k ] = 0;

                x_scores[ i ][ j ][ k ] = 0.0;
                y_scores[ i ][ j ][ k ] = 0.0;
                z_scores[ i ][ j ][ k ] = 0.0;
            }
        }
    }

    ERE(get_max_matrix_col_elements(&range_vp, mp));

    /* Just in case one of the elements is zero (which is a problem if the min
    // is zero also).
    */
    ERE(ow_add_scalar_to_vector(range_vp, DBL_EPSILON));

    /*
    // Expand the range in case max and min are the same, and to guard against
    // the max bining one too high.
    */
    ERE(ow_multiply_vector_by_scalar(range_vp, 1.0 + 0.5 / resolution));

    ERE(get_min_matrix_col_elements(&offset_vp, mp));

    /* Just in case one of the elements is zero (which is a problem if the max
    // is zero also).
    */
    ERE(ow_subtract_scalar_from_vector(offset_vp, DBL_EPSILON));
    ERE(ow_subtract_vectors(range_vp, offset_vp));

    /*
    // Expand the range in case max and min are the same, and to guard against
    // the max bining one too high.
    */
    ERE(ow_multiply_vector_by_scalar(range_vp, 1.0 + 0.5 / resolution));

    ERE(divide_vector_by_scalar(&step_vp, range_vp, (double)resolution));

    for (count = 0; count < num_rows; count++)
    {
        i = (int)((mp->elements[ count ][ 0 ] - offset_vp->elements[ 0 ]) /
                                                        step_vp->elements[ 0 ]);
        j = (int)((mp->elements[ count ][ 1 ] - offset_vp->elements[ 1 ]) /
                                                        step_vp->elements[ 1 ]);
        k = (int)((mp->elements[ count ][ 2 ] - offset_vp->elements[ 2 ]) /
                                                        step_vp->elements[ 2 ]);

        x_scores[ i ][ j ][ k ] += mp->elements[ count ][ 0 ];
        y_scores[ i ][ j ][ k ] += mp->elements[ count ][ 1 ];
        z_scores[ i ][ j ][ k ] += mp->elements[ count ][ 2 ];

        counts[ i ][ j ][ k ]++;
    }

    count = 0;

    for (i=0; i<resolution; i++)
    {
        for (j=0; j<resolution; j++)
        {
            for (k=0; k<resolution; k++)
            {
                if (counts[ i ][ j ][ k ] > 0)
                {
                    (*result_vpp)->elements[ 0 ] +=
                            x_scores[ i ][ j ][ k ] / counts[ i ][ j ][ k ];

                    (*result_vpp)->elements[ 1 ] +=
                            y_scores[ i ][ j ][ k ] / counts[ i ][ j ][ k ];

                    (*result_vpp)->elements[ 2 ] +=
                            z_scores[ i ][ j ][ k ] / counts[ i ][ j ][ k ];

                    count++;
                }
            }
        }
    }

    if (count > 0)
    {
        result = ow_divide_vector_by_scalar(*result_vpp, (double)count);
    }
    else
    {
        SET_CANT_HAPPEN_BUG();
        result = ERROR;
    }


    free_vector(offset_vp);
    free_vector(step_vp);
    free_vector(range_vp);
    free_3D_int_array(counts);
    free_3D_double_array(x_scores);
    free_3D_double_array(y_scores);
    free_3D_double_array(z_scores);

    return result;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*
 * =============================================================================
 *                      average_matrix_vector_elements
 *
 * Averages matrices in a matrix vector
 *
 * This routine averages the matrices in mvp and puts the average matrix in
 * target_mpp. The matrices in mvp must be the same size.
 *
 * If *target_mpp is NULL, then a matrix of the appropriate size is created.  If
 * it is the wrong size, it is resized. Finally, if it is the right size, then
 * the storage is recycled, as is.
 *
 * Returns:
 *     NO_ERROR on success, and ERROR on failure, with an error message being
 *     set.
 *
 * Index: matrices, matrix statistics
 *
 * Author: Ernesto Brau
 *
 * Documentor: Ernesto Brau
 *
 * -----------------------------------------------------------------------------
 */

int average_matrix_vector_elements(Matrix** target_mpp, const Matrix_vector* mvp)
{
    int m, n;
    int i;

    if(mvp == NULL)
    {
        SET_ARGUMENT_BUG();
        return ERROR;
    }

    m = mvp->elements[0]->num_rows;
    n = mvp->elements[0]->num_cols;

    for(i = 1; i < mvp->length; i++)
    {
        if(mvp->elements[i]->num_rows != m || mvp->elements[i]->num_cols != n)
        {
            SET_ARGUMENT_BUG();
            return ERROR;
        }
    }

    copy_matrix(target_mpp, mvp->elements[0]);

    if(mvp->length == 1)
    {
        return NO_ERROR;
    }

    for(i = 1; i < mvp->length; i++)
    {
        ow_add_matrices(*target_mpp, mvp->elements[i]);
    }

    ow_divide_matrix_by_scalar(*target_mpp, (double)mvp->length);

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*
 * =============================================================================
 *                      is_matrix_row_stochastic
 *
 * Checks if a matrix is row stochastic.
 *
 * This routine checks if a matrix is row stochastic. A row stochastic matrix 
 * is a matrix whose rows consist of nonnegative real numbers that sum to one.
 * Note that the matrix does not have to be square; i.e., this routine does not
 * check for right-stochastic-ness.
 *
 * Returns:
 *     TRUE if the M is row stochastic and FALSE if not. If M is NULL, this routine
 *     returns ERROR.
 *
 * Index: matrices, matrix statistics
 *
 * Author: Ernesto Brau
 *
 * Documentor: Ernesto Brau
 *
 * -----------------------------------------------------------------------------
 */

int is_matrix_row_stochastic(const Matrix* M)
{
    int i;
    const double DISTRIBUTION_EPSILON = 0.00001;

    if(M == NULL)
    {
        SET_ARGUMENT_BUG();
        return ERROR;
    }

    for(i = 0; i < M->num_rows; i++)
    {
        if(fabs(sum_matrix_row_elements(M, i) - 1.0) > DISTRIBUTION_EPSILON)
        {
            return FALSE;
        }
    }

    return TRUE;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

#ifdef __cplusplus
}
#endif

