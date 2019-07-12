
/* $Id: m_mat_error.c 20654 2016-05-05 23:13:43Z kobus $ */

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
#include "m/m_mat_error.h"

#ifdef __cplusplus
extern "C" {
#endif

/* -------------------------------------------------------------------------- */

/* =============================================================================
 *                           get_rms_row_error
 *
 * Returns the RMS error of rows
 *
 * This routine calculates the RMS error of the rows of two dimension compatable
 * matrices. Corresponding rows are treated as vectors. For each such pair we
 * calcluate the error which is the standard vector norm of their difference.
 * The RMS value of all such errors is put into *error_ptr. In addition, if
 * result_vpp is not NULL, then the error each componant of the rows are
 * calculated and placed into the vector pointed to by *result_vpp.  The target
 * vector *result_vpp is created or resized if requred.
 *
 * Returns:
 *    NO_ERROR on success, and ERROR on failure (not very likely--currently
 *    impossible). Note that sending in matrices with incompatable dimensions is
 *    regarded as a bug. (See set_bug(3)).
 *
 * Index: matrices, matrix measures, matrix errors, error measures
 *
 * -----------------------------------------------------------------------------
*/

int get_rms_row_error
(
    Vector**      result_vpp,
    const Matrix* first_mp,
    const Matrix* second_mp,
    double*       error_ptr
)
{
    int  num_rows;
    int  num_cols;
    double sum      = 0.0;
    int  i;
    int  j;


    ERE(check_same_matrix_dimensions(first_mp, second_mp, "get_rms_row_error"));

    num_rows = first_mp->num_rows;
    num_cols = first_mp->num_cols;

    if (result_vpp != NULL)
    {
        Vector* result_vp;


        ERE(get_zero_vector(result_vpp, num_rows));
        result_vp = *result_vpp;

        for (i=0; i<num_rows; i++)
        {
            double* first_row_pos  = (first_mp->elements)[i];
            double* second_row_pos = (second_mp->elements)[i];
            double  temp;
            double  sqr_sum = 0.0;


            for (j=0; j<num_cols; j++)
            {
                temp = (*first_row_pos) - (*second_row_pos);
                sqr_sum += temp * temp;

                first_row_pos++;
                second_row_pos++;
            }

            result_vp->elements[ i ] = sqrt(sqr_sum / num_cols);
        }
    }

    if  (error_ptr != NULL)
    {
        /*
        // We do this directly, as opposed to using result_vp, as the routine
        // user may not have asked for result_vpp,
        */
        for (i=0; i<num_rows; i++)
        {
            double* first_row_pos  = (first_mp->elements)[i];
            double* second_row_pos = (second_mp->elements)[i];
            double  temp;


            for (j=0; j<num_cols; j++)
            {
                temp = (*first_row_pos) - (*second_row_pos);
                sum += temp * temp;

                first_row_pos++;
                second_row_pos++;
            }
        }

        *error_ptr = sqrt(sum / (double)(num_rows * num_cols));
    }

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                        get_rms_relative_row_error
 *
 * Returns the RMS relative error of rows and/or entire matrices
 *
 * This routine calculates the RMS relative error of the rows of two dimension
 * compatable matrices and/or the overall RMS relative error. If result_vpp is
 * not NULL, then the RMS relative error of corresponding rows are put into
 * (*result_vpp) which is created or resized as needed. If error_ptr is not
 * NULL, then the RMS relative error of the entire matrix is put into
 * (*error_ptr). The relative error of corresponding pairs of number is computed
 * by their difference normalized by their average.
 *
 * Returns:
 *    NO_ERROR on success, and ERROR on failure (not very likely--currently
 *    impossible). Note that sending in matrices with incompatable dimensions is
 *    regarded as a bug. (See set_bug(3)).
 *
 * Index: matrices, matrix measures, matrix errors, error measures
 *
 * -----------------------------------------------------------------------------
*/

int get_rms_relative_row_error
(
    Vector**      result_vpp,
    const Matrix* first_mp,
    const Matrix* second_mp,
    double*       error_ptr
)
{
    int  num_rows;
    int  num_cols;
    double sum      = 0.0;
    int  i;
    int  j;


    ERE(check_same_matrix_dimensions(first_mp, second_mp,
                                     "get_rms_relative_row_error"));

    num_rows = first_mp->num_rows;
    num_cols = first_mp->num_cols;

    /*
    // Just do the cases for result_vpp and error_ptr separately. There is some
    // dupliclated effort that we don't worry about for now.
    */

    if (result_vpp != NULL)
    {
        Vector* result_vp;


        ERE(get_zero_vector(result_vpp, num_rows));
        result_vp = *result_vpp;

        for (i=0; i<num_rows; i++)
        {
            double* first_row_pos  = (first_mp->elements)[i];
            double* second_row_pos = (second_mp->elements)[i];
            double  temp;
            double sqr_sum = 0.0;


            for (j=0; j<num_cols; j++)
            {
                temp = (*first_row_pos) - (*second_row_pos);
                temp /= ((*first_row_pos) + (*second_row_pos));
                temp *= 2.0;

                sqr_sum += temp * temp;

                first_row_pos++;
                second_row_pos++;
            }

            result_vp->elements[ i ] = sqrt(sqr_sum / num_cols);
        }
    }

    if (error_ptr != NULL)
    {
        for (i=0; i<num_rows; i++)
        {
            double* first_row_pos  = (first_mp->elements)[i];
            double* second_row_pos = (second_mp->elements)[i];
            double  temp;


            for (j=0; j<num_cols; j++)
            {
                temp = (*first_row_pos) - (*second_row_pos);
                temp /= ((*first_row_pos) + (*second_row_pos));
                temp *= 2.0;
                sum += temp * temp;

                first_row_pos++;
                second_row_pos++;
            }
        }

        *error_ptr = sqrt(sum / (double)(num_cols * num_rows));
    }

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                           get_rms_col_error
 *
 * Computes the RMS error of columns and/or whole matrix
 *
 * This routine calculates the RMS error of the columns of two dimension
 * compatable matrices. Corresponding columns are treated as vectors. For each
 * such pair we calcluate the error which is the standard vector norm of their
 * difference.  The RMS value of all such errors is put into *error_ptr. In
 * addition, if result_vpp is not NULL, then the error each componant of the
 * column are calculated and placed into the vector pointed to by *result_vpp.
 * The target vector *result_vpp is created or resized if requred.
 *
 * Returns:
 *    NO_ERROR on success, and ERROR on failure (not very likely--currently
 *    impossible). Note that sending in matrices with incompatable dimensions is
 *    regarded as a bug. (See set_bug(3)).
 *
 * Index: matrices, matrix measures, matrix errors, error measures
 *
 * -----------------------------------------------------------------------------
*/

int get_rms_col_error
(
    Vector**      result_vpp,
    const Matrix* first_mp,
    const Matrix* second_mp,
    double*       error_ptr
)
{
    int  num_rows;
    int  num_cols;
    double sum      = 0.0;
    int  i;
    int  j;


    ERE(check_same_matrix_dimensions(first_mp, second_mp, "get_rms_col_error"));

    num_rows = first_mp->num_rows;
    num_cols = first_mp->num_cols;

    if (result_vpp != NULL)
    {
        Vector* result_vp;

        ERE(get_zero_vector(result_vpp, num_cols));
        result_vp = *result_vpp;

        for (i=0; i<num_rows; i++)
        {
            double* first_row_pos  = (first_mp->elements)[i];
            double* second_row_pos = (second_mp->elements)[i];
            double* result_pos     = result_vp->elements;
            double  temp;


            for (j=0; j<num_cols; j++)
            {
                temp = (*first_row_pos) - (*second_row_pos);
                *result_pos += temp * temp;

                first_row_pos++;
                second_row_pos++;
                result_pos++;
            }
        }

        ERE(ow_divide_vector_by_scalar(result_vp, (double)num_rows));

        for (i=0; i<num_cols; i++)
        {
            result_vp->elements[ i ] = sqrt(result_vp->elements[ i ]);
        }
    }

    if  (error_ptr != NULL)
    {
        /*
        // We do this directly, as opposed to using result_vp, as the routine
        // user may not have asked for result_vpp,
        */
        for (i=0; i<num_rows; i++)
        {
            double* first_row_pos  = (first_mp->elements)[i];
            double* second_row_pos = (second_mp->elements)[i];
            double  temp;


            for (j=0; j<num_cols; j++)
            {
                temp = (*first_row_pos) - (*second_row_pos);
                sum += temp * temp;

                first_row_pos++;
                second_row_pos++;
            }
        }

        *error_ptr = sqrt(sum / (double)(num_rows * num_cols));
    }

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                        get_rms_relative_col_error
 *
 * Returns the RMS relative error of rows and/or entire matrices
 *
 * This routine calculates the RMS relative error of the columns of two
 * dimension compatable matrices and/or the overall RMS relative error. If
 * result_vpp is not NULL, then the RMS relative error of corresponding column
 * are put into (*result_vpp) which is created or resized as needed. If
 * error_ptr is not NULL, then the RMS relative error of the entire matrix is
 * put into (*error_ptr). The relative error of corresponding pairs of number is
 * computed by their difference normalized by their average.
 *
 * Returns:
 *    NO_ERROR on success, and ERROR on failure (not very likely--currently
 *    impossible). Note that sending in matrices with incompatable dimensions is
 *    regarded as a bug. (See set_bug(3)).
 *
 * Index: matrices, matrix measures, matrix errors, error measures
 *
 * -----------------------------------------------------------------------------
*/

int get_rms_relative_col_error
(
    Vector**      result_vpp,
    const Matrix* first_mp,
    const Matrix* second_mp,
    double*       error_ptr
)
{
    int  num_rows;
    int  num_cols;
    double sum      = 0.0;
    int  i;
    int  j;


    ERE(check_same_matrix_dimensions(first_mp, second_mp,
                                     "get_rms_relative_col_error"));

    num_rows = first_mp->num_rows;
    num_cols = first_mp->num_cols;

    /*
    // Just do the cases for result_vpp and error_ptr separately. There is some
    // dupliclated effort that we don't worry about for now.
    */

    if (result_vpp != NULL)
    {
        Vector* result_vp;


        ERE(get_zero_vector(result_vpp, num_cols));
        result_vp = *result_vpp;

        for (i=0; i<num_rows; i++)
        {
            double* first_row_pos  = (first_mp->elements)[i];
            double* second_row_pos = (second_mp->elements)[i];
            double* result_pos     = result_vp->elements;
            double  temp;


            for (j=0; j<num_cols; j++)
            {
                temp = (*first_row_pos) - (*second_row_pos);
                temp /= ((*first_row_pos) + (*second_row_pos));
                temp *= 2.0;

                *result_pos += temp * temp;

                first_row_pos++;
                second_row_pos++;
                result_pos++;
            }
        }

        ERE(ow_divide_vector_by_scalar(result_vp, (double)num_rows));

        for (i=0; i<num_cols; i++)
        {
            result_vp->elements[ i ] = sqrt(result_vp->elements[ i ]);
        }
    }

    if (error_ptr != NULL)
    {
        for (i=0; i<num_rows; i++)
        {
            double* first_row_pos  = (first_mp->elements)[i];
            double* second_row_pos = (second_mp->elements)[i];
            double  temp;


            for (j=0; j<num_cols; j++)
            {
                temp = (*first_row_pos) - (*second_row_pos);
                temp /= ((*first_row_pos) + (*second_row_pos));
                temp *= 2.0;
                sum += temp * temp;

                first_row_pos++;
                second_row_pos++;
            }
        }

        *error_ptr = sqrt(sum / (double)(num_cols * num_rows));
    }

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

#ifdef __cplusplus
}
#endif

