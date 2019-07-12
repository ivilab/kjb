
/* $Id: m_mat_metric.c 20654 2016-05-05 23:13:43Z kobus $ */

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
#include "m/m_mat_metric.h"

#ifdef __cplusplus
extern "C" {
#endif

/* -------------------------------------------------------------------------- */

/* =============================================================================
 *                          max_rel_matrix_difference
 *
 * Returns the max relative difference between two matrices
 *
 * This routine calculates the maximum of
 *     2 * | M(i,j) - N(i,j) | / |M(i,j)| + |N(i,j)|
 * for two dimension compatable matrices.
 *
 * If 2 elements are zero, their relative difference is considered 0.
 *
 * To support test program, if both matrices are NULL, the different is zero.
 *
 * Returns:
 *    The distance between the two matrices, or a negative number if there is an
 *    error (not very likely--currently impossible). Note that sending in
 *    matrices with incompatable dimensions is regarded as a bug. (See
 *    set_bug(3)).
 *
 * Index: matrices, matrix measures
 *
 * -----------------------------------------------------------------------------
*/

double max_rel_matrix_difference
(
    const Matrix* first_mp,
    const Matrix* second_mp
)
{
    int   i;
    int   j;
    int   num_rows;
    int   num_cols;
    double* first_pos;
    double* second_pos;
    double  max_diff   = 0.0;


    /*
     * Return is a bit awkward due to function semantics, so we opt for calling
     * dimension problems a bug. The ERROR return gets cast into a double which
     * is fine as this routine should return a non-negative value.
    */
    ESBRE(check_same_matrix_dimensions(first_mp, second_mp,
                                       "max_rel_matrix_difference"));

    if ((first_mp == NULL) && (second_mp == NULL))
    {
        return 0.0;
    }

    num_rows = first_mp->num_rows;
    num_cols = first_mp->num_cols;

    for (i=0; i<num_rows; i++)
    {
        first_pos = first_mp->elements[ i ];
        second_pos = second_mp->elements[ i ];

        for (j=0; j<num_cols; j++)
        {
            double diff;

            diff = ABS_OF(*first_pos - *second_pos);

            if (diff > 0.0)
            {
                diff *= 2.0;
                diff /= (ABS_OF(*first_pos) + ABS_OF(*second_pos));
            }

            if (diff > max_diff) max_diff = diff;

            first_pos++;
            second_pos++;
        }
    }

    return max_diff;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                          max_abs_matrix_difference
 *
 * Returns the max absolute difference between two matrices
 *
 * This routine calculates the maximum of the absolute value of the difference
 * of corrresponding elements of two dimension compatable matrices.
 *
 * Note that the abs in the name refers to the "not relative" sense of absolute,
 * but the value computed is also the maximum absolute value.
 *
 * To support test program, if both matrices are NULL, the different is zero.
 *
 * Returns:
 *    The distance between the two matrices, or a negative number if there is an
 *    error (not very likely--currently impossible). Note that sending in
 *    matrices with incompatable dimensions is regarded as a bug. (See
 *    set_bug(3)).
 *
 * Index: matrices, matrix measures
 *
 * -----------------------------------------------------------------------------
*/

double max_abs_matrix_difference
(
    const Matrix* first_mp,
    const Matrix* second_mp
)
{
    int   i;
    int   j;
    int   num_rows;
    int   num_cols;
    double* first_pos;
    double* second_pos;
    double  max_diff   = 0.0;


    /*
     * Return is a bit awkward due to function semantics, so we opt for calling
     * dimension problems a bug. The ERROR return gets cast into a double which
     * is fine as this routine should return a non-negative value.
    */
    ESBRE(check_same_matrix_dimensions(first_mp, second_mp,
                                       "max_abs_matrix_difference"));

    if ((first_mp == NULL) && (second_mp == NULL))
    {
        return 0.0;
    }

    num_rows = first_mp->num_rows;
    num_cols = first_mp->num_cols;

    for (i=0; i<num_rows; i++)
    {
        first_pos = first_mp->elements[ i ];
        second_pos = second_mp->elements[ i ];

        for (j=0; j<num_cols; j++)
        {
            double diff;


            diff = ABS_OF(*first_pos - *second_pos);

            if (diff > max_diff) max_diff = diff;

            first_pos++;
            second_pos++;
        }
    }

    return max_diff;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                         rms_matrix_row_difference
 *
 * Returns the RMS difference of rows of two matrices
 *
 * This routine calculates the RMS of the norms of the differences of rows of
 * two dimension compatable matrices. This in turn is simply the Frobenius
 * difference normalized by the square root of the number of rows.
 *
 * To support test program, if both matrices are NULL, the different is zero.
 *
 * Returns:
 *    The distance between the two matrices or a negative number if there is an
 *    error (not very likely--currently impossible). Not that sending in
 *    matrices with incompatable dimensions is regarded as a bug. (See
 *    set_bug(3)).
 *
 * Index: matrices, matrix measures
 *
 * -----------------------------------------------------------------------------
*/

double rms_matrix_row_difference
(
    const Matrix* first_mp,
    const Matrix* second_mp
)
{
    int         i, j, num_rows, num_cols;
    double*     first_pos;
    double*     second_pos;
    long_double sum_of_squares = 0.0;


    /*
     * Return is a bit awkward due to function semantics, so we opt for calling
     * dimension problems a bug. The ERROR return gets cast into a double which
     * is fine as this routine should return a non-negative value.
    */
    ESBRE(check_same_matrix_dimensions(first_mp, second_mp,
                                       "rms_matrix_row_difference"));

    if ((first_mp == NULL) && (second_mp == NULL))
    {
        return 0.0;
    }

    num_rows = first_mp->num_rows;
    num_cols = first_mp->num_cols;

    for (i=0; i<num_rows; i++)
    {
        first_pos = first_mp->elements[ i ];
        second_pos = second_mp->elements[ i ];

        for (j=0; j<num_cols; j++)
        {
            double diff;


            diff = *first_pos - *second_pos;
            sum_of_squares += diff * diff;

            first_pos++;
            second_pos++;
        }
    }

    return sqrt((double)(sum_of_squares/num_rows));
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                         rms_matrix_difference
 *
 * Returns the RMS difference between two matrices
 *
 * This routine calculates the RMS difference of all the elements of two
 * dimension compatable matrices. This in turn is simply the Frobenius
 * difference normalized by the square root of the number of elements.
 *
 * To support test program, if both matrices are NULL, the different is zero.
 *
 * Returns:
 *    The distance between the two matrices or a negative number if there is an
 *    error (not very likely--currently impossible). Note that sending in
 *    matrices with incompatable dimensions is regarded as a bug. (See
 *    set_bug(3)).
 *
 * Index: matrices, matrix measures
 *
 * -----------------------------------------------------------------------------
*/

double rms_matrix_difference
(
    const Matrix* first_mp,
    const Matrix* second_mp
)
{
    int         i, j, num_rows, num_cols;
    double*     first_pos;
    double*     second_pos;
    long_double sum_of_squares = 0.0;


    /*
     * Return is a bit awkward due to function semantics, so we opt for calling
     * dimension problems a bug. The ERROR return gets cast into a double which
     * is fine as this routine should return a non-negative value.
    */
    ESBRE(check_same_matrix_dimensions(first_mp, second_mp,
                                       "rms_matrix_difference"));

    if ((first_mp == NULL) && (second_mp == NULL))
    {
        return 0.0;
    }

    num_rows = first_mp->num_rows;
    num_cols = first_mp->num_cols;

    for (i=0; i<num_rows; i++)
    {
        first_pos = first_mp->elements[ i ];
        second_pos = second_mp->elements[ i ];

        for (j=0; j<num_cols; j++)
        {
            double diff;


            diff = *first_pos - *second_pos;
            sum_of_squares += diff * diff;

            first_pos++;
            second_pos++;
        }
    }

    return sqrt((double)(sum_of_squares/(num_rows*num_cols)));
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                         frobenius_matrix_difference
 *
 * Returns the Frobenius norm of the difference of two matrices
 *
 * This routine calculates the Frobenius norm of the difference of two dimension
 * compatable matrices. The Frobenius norm is simply the square root of the sum
 * of the squares of the matrix elements.
 *
 * To support test program, if both matrices are NULL, the different is zero.
 *
 * Returns:
 *    The distance between the two matrices or a negative number if there is an
 *    error (not very likely--currently impossible). Note that sending in
 *    matrices with incompatable dimensions is regarded as a bug. (See
 *    set_bug(3)).
 *
 * Index: matrices, matrix measures
 *
 * -----------------------------------------------------------------------------
*/

double frobenius_matrix_difference
(
    const Matrix* first_mp,
    const Matrix* second_mp
)
{
    int         i, j, num_rows, num_cols;
    double*     first_pos;
    double*     second_pos;
    long_double sum_of_squares = 0.0;


    /*
     * Return is a bit awkward due to function semantics, so we opt for calling
     * dimension problems a bug. The ERROR return gets cast into a double which
     * is fine as this routine should return a non-negative value.
    */
    ESBRE(check_same_matrix_dimensions(first_mp, second_mp,
                                      "frobenius_matrix_difference"));

    if ((first_mp == NULL) && (second_mp == NULL))
    {
        return 0.0;
    }

    num_rows = first_mp->num_rows;
    num_cols = first_mp->num_cols;

    for (i=0; i<num_rows; i++)
    {
        first_pos = first_mp->elements[ i ];
        second_pos = second_mp->elements[ i ];

        for (j=0; j<num_cols; j++)
        {
            double diff;


            diff = *first_pos - *second_pos;
            sum_of_squares += diff * diff;

            first_pos++;
            second_pos++;
        }
    }

    return sqrt((double)sum_of_squares);
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

#ifdef __cplusplus
}
#endif

