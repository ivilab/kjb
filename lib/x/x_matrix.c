
/* $Id: x_matrix.c 4727 2009-11-16 20:53:54Z kobus $ */

/* =========================================================================== *
|
|  Copyright (c) 2003-2008 by Kobus Barnard (author).
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

#include "x/x_gen.h"
#include "x/x_matrix.h"

#ifdef __cplusplus
extern "C" {
#endif

/* -------------------------------------------------------------------------- */

/*
 * =============================================================================
 *                          complex_multiply_matrices_ew
 *
 * Multiplies pairs of matrices representing complex matrices
 *
 * This routine multiplies pairs of matrices representing complex matrices.  All
 * non-null matrices must be the same size. If one of a pair of matrices are
 * NULL, then it is treated as zero. If both matrices of a pair are NULL, then
 * the result is two NULL matrices.
 *
 * The first two arguments are pointers to target matrices. If they are null,
 * then matrices of the appropriate size is created. If they are of the wrong
 * size, it is resized. Finally, if they are the right size, then the storage is
 * recycled, as is.
 *
 * Returns:
 *     NO_ERROR on success, and ERROR on failure, with an error message being
 *     set.
 *
 * Index: matrices, matrix arithmetic, complex numbers
 *
 * -----------------------------------------------------------------------------
 */

int complex_multiply_matrices_ew
(
    Matrix**      out_re_mpp,
    Matrix**      out_im_mpp,
    const Matrix* in_re1_mp,
    const Matrix* in_im1_mp,
    const Matrix* in_re2_mp,
    const Matrix* in_im2_mp
)
{
    int     num_rows, num_cols, i, j;
    double* re1_ptr;
    double* re2_ptr;
    double* im1_ptr;
    double* im2_ptr;
    double* out_re_ptr;
    double* out_im_ptr;
    double  re1, im1, re2, im2;

    if (in_re1_mp != NULL)
    {
        num_rows = in_re1_mp->num_rows;
        num_cols = in_re1_mp->num_cols;
    }
    else if (in_im1_mp != NULL)
    {
        num_rows = in_im1_mp->num_rows;
        num_cols = in_im1_mp->num_cols;
    }
    else
    {
        free_matrix(*out_re_mpp);
        *out_re_mpp = NULL;
        free_matrix(*out_im_mpp);
        *out_im_mpp = NULL;
        return NO_ERROR;
    }

    if (in_re2_mp != NULL)
    {
        if (    (in_re2_mp->num_rows != num_rows)
             || (in_re2_mp->num_cols != num_cols)
           )
        {
            SET_ARGUMENT_BUG();
            return ERROR;
        }
    }
    else if (in_im2_mp != NULL)
    {
        if (    (in_im2_mp->num_rows != num_rows)
             || (in_im2_mp->num_cols != num_cols)
           )
        {
            SET_ARGUMENT_BUG();
            return ERROR;
        }
    }
    else
    {
        free_matrix(*out_re_mpp);
        *out_re_mpp = NULL;
        free_matrix(*out_im_mpp);
        *out_im_mpp = NULL;
        return NO_ERROR;
    }

    ERE(get_target_matrix(out_re_mpp, num_rows, num_cols));
    ERE(get_target_matrix(out_im_mpp, num_rows, num_cols));

    out_re_ptr = (*out_re_mpp)->elements[ 0 ];
    out_im_ptr = (*out_im_mpp)->elements[ 0 ];

    re1_ptr = (in_re1_mp == NULL) ? NULL : in_re1_mp->elements[ 0 ];
    im1_ptr = (in_im1_mp == NULL) ? NULL : in_im1_mp->elements[ 0 ];
    re2_ptr = (in_re2_mp == NULL) ? NULL : in_re2_mp->elements[ 0 ];
    im2_ptr = (in_im2_mp == NULL) ? NULL : in_im2_mp->elements[ 0 ];

    /*
     * Fancy case: One of the matrices is NULL. This will slow things down
     * compared with the standard case. The fancy case could be made faster, but
     * it is not likely to be called often.
     */
    if (    (re1_ptr == NULL)
         || (im1_ptr == NULL)
         || (re2_ptr == NULL)
         || (im2_ptr == NULL)
       )
    {
        for (i = 0; i < num_rows; i++)
        {
            for (j = 0; j < num_cols; j++)
            {
                re1 = (re1_ptr == NULL) ? 0.0 : *re1_ptr;
                im1 = (im1_ptr == NULL) ? 0.0 : *im1_ptr;
                re2 = (re2_ptr == NULL) ? 0.0 : *re2_ptr;
                im2 = (im2_ptr == NULL) ? 0.0 : *im2_ptr;

                *out_re_ptr = re1*re2 - im1*im2;
                *out_im_ptr = re1*im2 + im1*re2;

                out_re_ptr++;
                out_im_ptr++;

                if (re1_ptr != NULL) re1_ptr++;
                if (im1_ptr != NULL) im1_ptr++;
                if (re2_ptr != NULL) re2_ptr++;
                if (im2_ptr != NULL) im2_ptr++;
            }
        }
    }
    else
    {
        for (i = 0; i < num_rows; i++)
        {
            for (j = 0; j < num_cols; j++)
            {
                re1 = *re1_ptr;
                im1 = *im1_ptr;
                re2 = *re2_ptr;
                im2 = *im2_ptr;

                *out_re_ptr = re1*re2 - im1*im2;
                *out_im_ptr = re1*im2 + im1*re2;

                out_re_ptr++;
                out_im_ptr++;

                re1_ptr++;
                im1_ptr++;
                re2_ptr++;
                im2_ptr++;
            }
        }
    }

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*
 * =============================================================================
 *                          complex_divide_matrices_ew
 *
 * Divides pairs of matrices representing complex matrices
 *
 * This routine divides pairs of matrices representing complex matrices.  All
 * non-null matrices must be the same size. If one of a pair of matrices are
 * NULL, then it is treated as zero. If both matrices of a pair are NULL, then
 * the result is two NULL matrices.
 *
 * The first two arguments are pointers to target matrices. If they are null,
 * then matrices of the appropriate size is created. If they are of the wrong
 * size, it is resized. Finally, if they are the right size, then the storage is
 * recycled, as is.
 *
 * Returns:
 *     NO_ERROR on success, and ERROR on failure, with an error message being
 *     set.
 *
 * Index: matrices, matrix arithmetic, complex numbers
 *
 * -----------------------------------------------------------------------------
 */

int complex_divide_matrices_ew
(
    Matrix**      out_re_mpp,
    Matrix**      out_im_mpp,
    const Matrix* in_re1_mp,
    const Matrix* in_im1_mp,
    const Matrix* in_re2_mp,
    const Matrix* in_im2_mp
)
{
    int     num_rows, num_cols, i, j;
    double* re1_ptr;
    double* re2_ptr;
    double* im1_ptr;
    double* im2_ptr;
    double* out_re_ptr;
    double* out_im_ptr;
    double  re1, im1, re2, im2;
    double denom, temp;

    if (in_re1_mp != NULL)
    {
        num_rows = in_re1_mp->num_rows;
        num_cols = in_re1_mp->num_cols;
    }
    else if (in_im1_mp != NULL)
    {
        num_rows = in_im1_mp->num_rows;
        num_cols = in_im1_mp->num_cols;
    }
    else
    {
        free_matrix(*out_re_mpp);
        *out_re_mpp = NULL;
        free_matrix(*out_im_mpp);
        *out_im_mpp = NULL;
        return NO_ERROR;
    }

    if (in_re2_mp != NULL)
    {
        if (    (in_re2_mp->num_rows != num_rows)
             || (in_re2_mp->num_cols != num_cols)
           )
        {
            SET_ARGUMENT_BUG();
            return ERROR;
        }
    }
    else if (in_im2_mp != NULL)
    {
        if (    (in_im2_mp->num_rows != num_rows)
             || (in_im2_mp->num_cols != num_cols)
           )
        {
            SET_ARGUMENT_BUG();
            return ERROR;
        }
    }
    else
    {
        free_matrix(*out_re_mpp);
        *out_re_mpp = NULL;
        free_matrix(*out_im_mpp);
        *out_im_mpp = NULL;
        return NO_ERROR;
    }

    ERE(get_target_matrix(out_re_mpp, num_rows, num_cols));
    ERE(get_target_matrix(out_im_mpp, num_rows, num_cols));

    out_re_ptr = (*out_re_mpp)->elements[ 0 ];
    out_im_ptr = (*out_im_mpp)->elements[ 0 ];

    re1_ptr = (in_re1_mp == NULL) ? NULL : in_re1_mp->elements[ 0 ];
    im1_ptr = (in_im1_mp == NULL) ? NULL : in_im1_mp->elements[ 0 ];
    re2_ptr = (in_re2_mp == NULL) ? NULL : in_re2_mp->elements[ 0 ];
    im2_ptr = (in_im2_mp == NULL) ? NULL : in_im2_mp->elements[ 0 ];

    /*
     * Fancy case: One of the matrices is NULL. This will slow things down
     * compared with the standard case. The fancy case could be made faster, but
     * it is not likely to be called often.
     */
    if (    (re1_ptr == NULL)
         || (im1_ptr == NULL)
         || (re2_ptr == NULL)
         || (im2_ptr == NULL)
       )
    {
        for (i = 0; i < num_rows; i++)
        {
            for (j = 0; j < num_cols; j++)
            {
                re1 = (re1_ptr == NULL) ? 0.0 : *re1_ptr;
                im1 = (im1_ptr == NULL) ? 0.0 : *im1_ptr;
                re2 = (re2_ptr == NULL) ? 0.0 : *re2_ptr;
                im2 = (im2_ptr == NULL) ? 0.0 : *im2_ptr;

                if (ABS_OF(re2) >= ABS_OF(im2))
                {
                    temp = im2 / re2;
                    denom = re2 + temp*im2;
                    *out_re_ptr = (re1 + temp*im1)/denom;
                    *out_im_ptr = (im1 - temp*re1)/denom;
                }
                else
                {
                    temp = re2 / im2;
                    denom = im2 + temp * re2;
                    *out_re_ptr = (re1 * temp + im1) / denom;
                    *out_im_ptr = (im1 * temp - re1) / denom;
                }

                out_re_ptr++;
                out_im_ptr++;

                if (re1_ptr != NULL) re1_ptr++;
                if (im1_ptr != NULL) im1_ptr++;
                if (re2_ptr != NULL) re2_ptr++;
                if (im2_ptr != NULL) im2_ptr++;
            }
        }
    }
    else
    {
        for (i = 0; i < num_rows; i++)
        {
            for (j = 0; j < num_cols; j++)
            {
                re1 = *re1_ptr;
                im1 = *im1_ptr;
                re2 = *re2_ptr;
                im2 = *im2_ptr;

                if (ABS_OF(re2) >= ABS_OF(im2))
                {
                    temp = im2 / re2;
                    ASSERT_IS_NUMBER_DBL(temp);
                    ASSERT_IS_FINITE_DBL(temp);
                    denom = re2 + temp*im2;
                    ASSERT_IS_NUMBER_DBL(denom);
                    ASSERT_IS_FINITE_DBL(denom);
                    *out_re_ptr = (re1 + temp*im1)/denom;
                    ASSERT_IS_NUMBER_DBL(*out_re_ptr);
                    ASSERT_IS_FINITE_DBL(*out_re_ptr);
                    *out_im_ptr = (im1 - temp*re1)/denom;
                    ASSERT_IS_NUMBER_DBL(*out_im_ptr);
                    ASSERT_IS_FINITE_DBL(*out_im_ptr);
                }
                else
                {
                    temp = re2 / im2;
                    ASSERT_IS_NUMBER_DBL(temp);
                    ASSERT_IS_FINITE_DBL(temp);
                    denom = im2 + temp * re2;
                    ASSERT_IS_NUMBER_DBL(denom);
                    ASSERT_IS_FINITE_DBL(denom);
                    *out_re_ptr = (re1 * temp + im1) / denom;
                    ASSERT_IS_NUMBER_DBL(*out_re_ptr);
                    ASSERT_IS_FINITE_DBL(*out_re_ptr);
                    *out_im_ptr = (im1 * temp - re1) / denom;
                    ASSERT_IS_NUMBER_DBL(*out_im_ptr);
                    ASSERT_IS_FINITE_DBL(*out_im_ptr);
                }


                out_im_ptr++;
                out_re_ptr++;

                re1_ptr++;
                im1_ptr++;
                re2_ptr++;
                im2_ptr++;
            }
        }
    }

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*
 * =============================================================================
 *                          complex_get_magnitude_matrix_ew
 *
 * Computes magnitude of a complex matrix
 *
 * This routine computes the magnitude of a complex matrix represented as a pair
 * of matrices. All non-null matrices must be the same size. If one of a pair of
 * matrices are NULL, then it is treated as zero. If both matrices of a pair are
 * NULL, then the result is two NULL matrices.
 *
 * The first argument is a pointer to target a matrix. If it is null, then a
 * matrix of the appropriate size is created. If it is of the wrong size, it is
 * resized. Finally, if they are the right size, then the storage is recycled,
 * as is.
 *
 * Returns:
 *     NO_ERROR on success, and ERROR on failure, with an error message being
 *     set.
 *
 * Index: matrices, matrix arithmetic, complex numbers
 *
 * -----------------------------------------------------------------------------
 */

int complex_get_magnitude_matrix_ew
(
    Matrix**      out_mpp,
    const Matrix* in_re_mp,
    const Matrix* in_im_mp
)
{
    int     num_rows, num_cols, i, j;
    double* re_ptr;
    double* im_ptr;
    double* out_ptr;
    double  re, im;


    /*
     * If both are NULL, then the result is NULL because NULL copies. 
    */
    if (in_im_mp == NULL)
    {
        return copy_matrix(out_mpp, in_re_mp);
    }
    else if (in_re_mp == NULL)
    {
        return copy_matrix(out_mpp, in_im_mp);
    }

    num_rows = in_re_mp->num_rows;
    num_cols = in_re_mp->num_cols;

    if (    (in_im_mp->num_rows != num_rows)
         || (in_im_mp->num_cols != num_cols)
       )
    {
        SET_ARGUMENT_BUG();
        return ERROR;
    }

    ERE(get_target_matrix(out_mpp, num_rows, num_cols));

    out_ptr = (*out_mpp)->elements[ 0 ];
    re_ptr = in_re_mp->elements[ 0 ];
    im_ptr = in_im_mp->elements[ 0 ];

    for (i = 0; i < num_rows; i++)
    {
        for (j = 0; j < num_cols; j++)
        {
            re = *re_ptr;
            im = *im_ptr;

#ifdef HOW_IT_WAS
            *out_ptr = sqrt(re*re + im*im);
#else
            *out_ptr = magnitude_of_complex_2(re, im);
#endif 

            ASSERT_IS_NUMBER_DBL(*out_ptr);
            ASSERT_IS_FINITE_DBL(*out_ptr);

            out_ptr++;
            re_ptr++;
            im_ptr++;
        }
    }

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*
 * =============================================================================
 *                          complex_get_phase_matrix_ew
 *
 * Computes phase of a complex matrix
 *
 * This routine computes the phase of a complex matrix represented as a pair of
 * matrices. All non-null matrices must be the same size. If one of a pair of
 * matrices are NULL, then it is treated as zero. If both matrices of a pair are
 * NULL, then the result is two NULL matrices.
 *
 * The first argument is a pointer to target a matrix. If it is null, then a
 * matrix of the appropriate size is created. If it is of the wrong size, it is
 * resized. Finally, if they are the right size, then the storage is recycled,
 * as is.
 *
 * Returns:
 *     NO_ERROR on success, and ERROR on failure, with an error message being
 *     set.
 *
 * Index: matrices, matrix arithmetic, complex numbers
 *
 * -----------------------------------------------------------------------------
 */

int complex_get_phase_matrix_ew
(
    Matrix**      out_mpp,
    const Matrix* in_re_mp,
    const Matrix* in_im_mp
)
{
    int     num_rows, num_cols, i, j;
    double* re_ptr;
    double* im_ptr;
    double* out_ptr;
#ifdef HOW_IT_WAS
    double  re, im, phase;
#endif 


    if ((in_im_mp == NULL) && (in_re_mp == NULL))
    {
        UNTESTED_CODE();

        free_matrix(*out_mpp); 
        *out_mpp = NULL;
    }
    else if (in_im_mp == NULL)
    {
        UNTESTED_CODE();

        num_rows = in_re_mp->num_rows;
        num_cols = in_re_mp->num_cols;

        ERE(get_target_matrix(out_mpp, num_rows, num_cols));

        out_ptr = (*out_mpp)->elements[ 0 ];
        re_ptr = in_re_mp->elements[ 0 ];

        for (i = 0; i < num_rows; i++)
        {
            for (j = 0; j < num_cols; j++)
            {
                if (*re_ptr >= 0.0)
                {
                    *out_ptr = 0.0;
                }
                else
                {
                    *out_ptr = M_PI; 
                }
                out_ptr++;
                re_ptr++;
            }
        }
    }
    else if (in_re_mp == NULL)
    {
        UNTESTED_CODE();

        num_rows = in_im_mp->num_rows;
        num_cols = in_im_mp->num_cols;

        ERE(get_target_matrix(out_mpp, num_rows, num_cols));

        out_ptr = (*out_mpp)->elements[ 0 ];
        im_ptr = in_im_mp->elements[ 0 ];

        for (i = 0; i < num_rows; i++)
        {
            for (j = 0; j < num_cols; j++)
            {
                if (*im_ptr > 0.0)
                {
                    *out_ptr = M_PI / 2.0;
                }
                else if (*im_ptr < 0.0)
                {
                    *out_ptr = -M_PI / 2.0; 
                }
                else 
                {
                    *out_ptr = 0.0;
                }
                out_ptr++;
                im_ptr++;
            }
        }
    }
    else 
    {
        UNTESTED_CODE();

        num_rows = in_re_mp->num_rows;
        num_cols = in_re_mp->num_cols;

        if (    (in_im_mp->num_rows != num_rows)
             || (in_im_mp->num_cols != num_cols)
           )
        {
            SET_ARGUMENT_BUG();
            return ERROR;
        }

        ERE(get_target_matrix(out_mpp, num_rows, num_cols));

        out_ptr = (*out_mpp)->elements[ 0 ];
        re_ptr = in_re_mp->elements[ 0 ];
        im_ptr = in_im_mp->elements[ 0 ];

        for (i = 0; i < num_rows; i++)
        {
            for (j = 0; j < num_cols; j++)
            {
                *out_ptr = angle_of_complex_2(*re_ptr, *im_ptr);

                ASSERT_IS_NUMBER_DBL(*out_ptr);
                ASSERT_IS_FINITE_DBL(*out_ptr);

                out_ptr++;
                re_ptr++;
                im_ptr++;
            }
        }
    }

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*
 * =============================================================================
 *                          complex_multiply_matrices
 *
 * Multiplies pairs of matrices representing complex matrices
 *
 * This routine multiplies pairs of matrices representing complex matrices.  If
 * one of a pair of matrices are NULL, then it is treated as zero. If both
 * matrices of a pair are NULL, then the result is two NULL matrices.
 *
 * The first two arguments are pointers to target matrices. If they are null,
 * then matrices of the appropriate size is created. If they are of the wrong
 * size, it is resized. Finally, if they are the right size, then the storage is
 * recycled, as is.
 *
 * Returns:
 *     NO_ERROR on success, and ERROR on failure, with an error message being
 *     set.
 *
 * Index: matrices, matrix arithmetic, complex
 *
 * -----------------------------------------------------------------------------
*/

int complex_multiply_matrices
(
    Matrix**      out_re_mpp,
    Matrix**      out_im_mpp,
    const Matrix* in_re1_mp,
    const Matrix* in_im1_mp,
    const Matrix* in_re2_mp,
    const Matrix* in_im2_mp
)
{
    int     num_rows, num_cols, len, i, j, k;
    Matrix* zero_1_mp = NULL;
    Matrix* zero_2_mp = NULL;



    if (in_re1_mp != NULL)
    {
        if (    (in_im1_mp != NULL)
             && (    (in_re1_mp->num_rows != in_im1_mp->num_rows)
                  || (in_re1_mp->num_cols != in_im1_mp->num_cols)
                )
           )
        {
            SET_ARGUMENT_BUG();
            return ERROR;
        }
        num_rows = in_re1_mp->num_rows;
        len = in_re1_mp->num_cols;

    }
    else if (in_im1_mp != NULL)
    {
        num_rows = in_im1_mp->num_rows;
        len = in_im1_mp->num_cols;
    }
    else
    {
        free_matrix(*out_re_mpp);
        *out_re_mpp = NULL;
        free_matrix(*out_im_mpp);
        *out_im_mpp = NULL;
        return NO_ERROR;
    }

    if (in_re2_mp != NULL)
    {
        if (    (in_im2_mp != NULL)
             && (    (in_im2_mp->num_rows != in_re2_mp->num_rows)
                  || (in_im2_mp->num_cols != in_re2_mp->num_cols)
                )
           )
        {
            SET_ARGUMENT_BUG();
            return ERROR;
        }

        num_cols = in_re2_mp->num_cols;

        if (in_re2_mp->num_rows != len)
        {
            set_error("Incompatable dimensions for matrix multiplication.");
            add_error("Attempt to multiply a %d by %d complex matrix with a %d by %d one.",
                      num_rows, len,
                      in_re2_mp->num_rows, num_cols);
            return ERROR;
        }

    }
    else if (in_im2_mp != NULL)
    {
        num_cols = in_im2_mp->num_cols;

        if (in_im2_mp->num_rows != len)
        {
            set_error("Incompatable dimensions for matrix multiplication.");
            add_error("Attempt to multiply a %d by %d complex matrix with a %d by %d one.",
                      num_rows, len,
                      in_im2_mp->num_rows, num_cols);
        }
    }
    else
    {
        free_matrix(*out_re_mpp);
        *out_re_mpp = NULL;
        free_matrix(*out_im_mpp);
        *out_im_mpp = NULL;
        return NO_ERROR;
    }

    ERE(get_zero_matrix(out_re_mpp, num_rows, num_cols));
    ERE(get_zero_matrix(out_im_mpp, num_rows, num_cols));

    /*
     * Case that both matrices are NULL has been handled above.
    */
    if (in_re1_mp == NULL)
    {
        ERE(get_zero_matrix(&zero_1_mp, num_rows, len));
        in_re1_mp = zero_1_mp;
    }
    else if (in_im1_mp == NULL)
    {
        ERE(get_zero_matrix(&zero_1_mp, num_rows, len));
        in_im1_mp = zero_1_mp;
    }

    /*
     * Case that both matrices are NULL has been handled above.
    */
    if (in_re2_mp == NULL)
    {
        ERE(get_zero_matrix(&zero_2_mp, len, num_cols));
        in_re2_mp = zero_2_mp;
    }
    else if (in_im2_mp == NULL)
    {
        ERE(get_zero_matrix(&zero_2_mp, len, num_cols));
        in_im2_mp = zero_2_mp;
    }

    for (i = 0; i < num_rows; i++)
    {
        for (j = 0; j < num_cols; j++)
        {
            double re_sum = 0.0;
            double im_sum = 0.0;

            for (k = 0; k < len; k++)
            {
                double re1 = in_re1_mp->elements[ i ][ k ];
                double im1 = in_im1_mp->elements[ i ][ k ];
                double re2 = in_re2_mp->elements[ k ][ j ];
                double im2 = in_im2_mp->elements[ k ][ j ];
                double re = re1*re2 - im1*im2;
                double im = re1*im2 + im1*re2;

                re_sum += re;
                im_sum += im;
            }

            (*out_re_mpp)->elements[ i ][ j ] = re_sum;
            (*out_im_mpp)->elements[ i ][ j ] = im_sum;
        }
    }

    free_matrix(zero_1_mp);
    free_matrix(zero_2_mp);

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

#ifdef __cplusplus
}
#endif

