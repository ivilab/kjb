
/* $Id: m_convolve.c 21160 2017-01-29 22:58:17Z kobus $ */

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

#include "m/m_gen.h"     /* Only safe as first include in a ".c" file. */
#include "m/m_convolve.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
 * The following macro implements "reflection" of index i to within the range
 * 0..(N-1).  Argument i must be an int lvalue.  Argument N must be a positive
 * const int.  The macro changes the value stored in i; thus, calling this
 * macro creates a statement, not an expression.
 *
 * Implementation notes:
 * Each parameter is evaluated multiple times, so don't get fancy when calling.
 * A previous implementation had an "else," but that was a bug:  both "if"
 * conditions must be checked, and both can execute, for example if i == -2*N.
 * The inner "if" is necessary, too, e.g., in the case of i==9, N==4.
 * This reflection idiom is used several times in the convolution code,
 * but we do not want to incur the overhead of an extra function call.
 * That is the rationale for using a macro.
 * Note that C89 says the results of integer division and modulo are
 * implementation-defined, if either operand is negative!  Thus we have to work
 * around that ambiguity.  Hence, we check the sign of i first.
 *
 * A similar macro occurs in i/i_convolve.c so if a bug is found here, please
 * fix it and propagate the fix to that other file too.
 */
#define REFLECT_INBOUNDS(i,N)                        \
    do                                               \
    {                                                \
        if ((i) < 0)                                 \
        {                                            \
            (i) = -(i) - 1;                          \
        }                                            \
        if ((i) >= (N))                              \
        {                                            \
            (i) %= 2*(N);                            \
            if ((i) >= (N)) (i) = 2*(N) - (i) - 1;   \
        }                                            \
    }                                                \
    while(0)

int get_2D_gaussian_mask_dispatch
(
    Matrix** mask_mpp,
    int      num_rows,
    int      num_cols,
    double   row_sigma,
    double   col_sigma,
    int      x_derivitive,
    int      y_derivitive
);

/* -------------------------------------------------------------------------- */

/* =============================================================================
 *                           gauss_convolve_matrix
 *
 * Convolves matrix with Gaussian mask
 *
 * This routine convolves the matrix pointed to by in_mp with a Gaussian mask
 * with the specified sigma, putting the result into *out_mpp.  The mask
 * size is automatically chosen based on the value of sigma.
 *
 * If *out_mpp is NULL, then an matrix of the appropriate size is created, if
 * it is the wrong size, then it is resized, and if it is the right size, the
 * storage is recycled.
 *
 * The results near the boundaries are computed by assuming the input matrix
 * reflects itself at the edges.
 *
 * Returns:
 *    NO_ERROR on success, and ERROR on failure, with an appropriate error
 *    message being set.
 *
 * Index: matrices, convolution
 *
 * -----------------------------------------------------------------------------
*/

int gauss_convolve_matrix
(
    Matrix**      out_mpp,
    const Matrix* in_mp,
    double        sigma
)
{
    int        mask_width;
    int        mask_size;
    int        status_code;
    Vector*    mask_vp = NULL;
    Matrix*    x_convolve_mp = NULL;


    mask_width = (int)(1.0 + 3.0 * sigma);
    mask_size  = 1 + 2 * mask_width;

    ERE(get_1D_gaussian_mask(&mask_vp, mask_size, sigma));

    if (ERROR == x_convolve_matrix(&x_convolve_mp, in_mp, mask_vp))
    {
        free_vector(mask_vp);
        return ERROR;
    }

    status_code = y_convolve_matrix(out_mpp, x_convolve_mp, mask_vp);
    free_matrix(x_convolve_mp);
    free_vector(mask_vp);
    return status_code;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                                convolve_matrix
 *
 * Convolves matrix with an arbitrary mask
 *
 * This routine convolves the matrix pointed to by in_mp with the mask mask_mp,
 * putting the result into *out_mpp.
 *
 * If *out_mpp is NULL, then an matrix of the appropriate size is created, if
 * it is the wrong size, then it is resized, and if it is the right size, the
 * storage is recycled.
 *
 * The results near the boundaries are computed by assuming the input matrix
 * reflects itself at the edges.
 *
 * Returns:
 *    NO_ERROR on success, and ERROR on failure, with an appropriate error
 *    message being set.
 *
 * Index: matrices, convolution
 *
 * -----------------------------------------------------------------------------
*/

int convolve_matrix
(
    Matrix**      out_mpp,
    const Matrix* in_mp,
    const Matrix* mask_mp
)
{
    Matrix* out_mp;
    int     num_rows, num_cols, i, j, mi, mj;
    int     mask_rows, mask_cols, mask_row_offset, mask_col_offset;

    NRE(in_mp);
    NRE(mask_mp);
    NRE(out_mpp);

    mask_rows = mask_mp->num_rows;
    mask_cols = mask_mp->num_cols;
    num_rows = in_mp->num_rows;
    num_cols = in_mp->num_cols;

    if (mask_rows >= num_rows || mask_cols >= num_cols)
    {
        set_error("Mask dimensions (%d x %d) exceed one or both\nrespective "
                "input matrix dimensions (%d x %d).\nCannot convolve:  "
                "code relies on the assumption\nthat the mask is smaller "
                "in each dimension.",
                mask_rows, mask_cols, num_rows, num_cols);
        return ERROR;
    }

    mask_row_offset = mask_rows / 2;
    mask_col_offset = mask_cols / 2;

    ERE(get_target_matrix(out_mpp, num_rows, num_cols));
    out_mp = *out_mpp;

    for (i=0; i<num_rows; i++)
    {
        for (j=0; j<num_cols; j++)
        {
            double sum      = 0.0;
            double weight;

            for (mi = 0; mi < mask_rows; mi++)
            {
                for (mj = 0; mj < mask_cols; mj++)
                {
                    int m = i + mask_row_offset - mi;
                    int n = j + mask_col_offset - mj;

                    /*
                     * TODO: Easy performance gain: Handle the boundary cases
                     * separately.
                     */
                    REFLECT_INBOUNDS(m, num_rows);
                    REFLECT_INBOUNDS(n, num_cols);

                    weight = mask_mp->elements[mi][mj];

                    sum += in_mp->elements[m][n] * weight;
                }
            }
            out_mp->elements[i][j] = sum;
        }
    }

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                                x_convolve_matrix
 *
 * Convolves matrix with a vector in the x direction
 *
 * This routine convolves the matrix pointed to by in_mp with the vector
 * *mask_vp, putting the result into *out_mpp.
 *
 * If *out_mpp is NULL, then a matrix of the appropriate size is created, if
 * it is the wrong size, then it is resized, and if it is the right size, the
 * storage is recycled.
 *
 * The results near the boundaries are computed by assuming the input matrix
 * reflects itself at the edges.
 *
 * Returns:
 *    NO_ERROR on success, and ERROR on failure, with an appropriate error
 *    message being set.
 *
 * Index: matrices, convolution
 *
 * -----------------------------------------------------------------------------
*/

int x_convolve_matrix
(
    Matrix**      out_mpp,
    const Matrix* in_mp,
    const Vector* mask_vp
)
{
    Matrix* out_mp;
    int     num_rows, num_cols, i, j, mj, n;
    int     mask_cols = mask_vp->length;
    int     mask_col_offset = mask_cols / 2;


    num_rows = in_mp->num_rows;
    num_cols = in_mp->num_cols;

    ERE(get_target_matrix(out_mpp, num_rows, num_cols));
    out_mp = *out_mpp;

    for (i=0; i<num_rows; i++)
    {
        for (j=0; j<num_cols; j++)
        {
            double sum      = 0.0;
            double weight;

            for (mj = 0; mj < mask_cols; mj++)
            {
                n = j + mask_col_offset - mj;

                REFLECT_INBOUNDS(n, num_cols);

                weight = mask_vp->elements[mj];

                sum += in_mp->elements[i][n] * weight;

            }

            out_mp->elements[i][j] = sum;
        }
    }

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                                y_convolve_matrix
 *
 * Convolves matrix with a vector in the y direction
 *
 * This routine convolves the matrix pointed to by in_mp with the
 * vector mask_vp, putting the result into *out_mpp.
 *
 * If *out_mpp is NULL, then an matrix of the appropriate size is created, if
 * it is the wrong size, then it is resized, and if it is the right size, the
 * storage is recycled.
 *
 * The results near the boundaries are computed by assuming the input matrix
 * reflects itself at the edges.
 *
 * Returns:
 *    NO_ERROR on success, and ERROR on failure, with an appropriate error
 *    message being set.
 *
 * Index: matrices, convolution
 *
 * -----------------------------------------------------------------------------
*/

int y_convolve_matrix
(
    Matrix**      out_mpp,
    const Matrix* in_mp,
    const Vector* mask_vp
)
{
    Matrix* out_mp;
    int     num_rows, num_cols, i, j, mi, m;
    int     mask_rows  = mask_vp->length;
    int     mask_row_offset = mask_rows / 2;


    num_rows = in_mp->num_rows;
    num_cols = in_mp->num_cols;

    ERE(get_target_matrix(out_mpp, num_rows, num_cols));
    out_mp = *out_mpp;

    for (i=0; i<num_rows; i++)
    {
        for (j=0; j<num_cols; j++)
        {
            double sum      = 0.0;
            double weight;

            for (mi = 0; mi < mask_rows; mi++)
            {
                m = i + mask_row_offset - mi;

                REFLECT_INBOUNDS(m, num_rows);

                weight = mask_vp->elements[mi];

                sum += in_mp->elements[m][j] * weight;
            }

            out_mp->elements[i][j] = sum;
        }
    }

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                                convolve_vector
 *
 * Convolves vector with a vector (mask)
 *
 * This routine convolves the vector pointed to by in_vp with the
 * vector mask_vp, putting the result into *out_vpp.
 *
 * If *out_vpp is NULL, then a vector of the appropriate size is created, if
 * it is the wrong size, then it is resized, and if it is the right size, the
 * storage is recycled.
 *
 * Returns:
 *    NO_ERROR on success, and ERROR on failure, with an appropriate error
 *    message being set.
 *
 * Index: matrices, convolution
 *
 * -----------------------------------------------------------------------------
*/

int convolve_vector
(
    Vector**      out_vpp,
    const Vector* in_vp,
    const Vector* mask_vp
)
{
    Vector* out_vp;
    int     length, j, mj, n;
    int     mask_length = mask_vp->length;
    int     mask_offset = mask_length / 2;


    length = in_vp->length;

    ERE(get_target_vector(out_vpp, length));
    out_vp = *out_vpp;

    for (j=0; j<length; j++)
    {
        double sum      = 0.0;
        double weight;

        for (mj = 0; mj < mask_length; mj++)
        {
            n = j + mask_offset - mj;

            REFLECT_INBOUNDS(n, length);

            weight = mask_vp->elements[mj];

            sum += in_vp->elements[n] * weight;

        }

        out_vp->elements[j] = sum;
    }

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* Computes a number of different gaussian masks */
int get_2D_gaussian_mask_dispatch
(
    Matrix** mask_mpp,  /* Output gaussian smoothing mask.   */
    int      num_rows,  /* Number of rows in mask.      */
    int      num_cols,  /* Number of cols in mask.      */
    double   row_sigma,  /* Standard deviation in row direction in bin units. */
    double   col_sigma,  /* Standard deviation in column direction bin units. */
    int      x_derivitive,  /* add x-derivitive mask? */
    int      y_derivitive   /* add y-derivitive mask? */
)
{
    int r, c;
    int    row_center;
    int    col_center;
    double row_dist_sqr;
    double col_dist_sqr;
    double row_sigma_sqr;
    double col_sigma_sqr;
    double gauss;
    double mask_sum;
    double target_sum;

    if (num_rows < 0 || num_cols < 0)
    {
        SET_ARGUMENT_BUG();
        return ERROR;
    }

    if( x_derivitive && y_derivitive )
    {
        /* this might be desirable, but we'd need to handle normalization,
         * which isn't done currently
         */
        SET_ARGUMENT_BUG();
        return ERROR;
    }

    row_center = num_rows / 2;
    col_center = num_cols / 2;

    if (IS_EVEN(num_rows) )
    {
         row_center--;
    }

    if (IS_EVEN(num_cols) )
    {
         col_center--;
    }

    row_sigma_sqr = row_sigma * row_sigma;
    col_sigma_sqr = col_sigma * col_sigma;

    mask_sum  = 0.0;

    ERE(get_zero_matrix(mask_mpp, num_rows, num_cols));

    for (r = 0; r < num_rows; r++)
    {
        for (c = 0; c < num_cols; c++)
        {
            row_dist_sqr = (r - row_center)*(r - row_center);
            col_dist_sqr = (c - col_center)*(c - col_center);

            gauss = exp( -0.5 * (row_dist_sqr / row_sigma_sqr
                                             + col_dist_sqr / col_sigma_sqr));

            if(x_derivitive)
            {
                gauss *= (c - col_center);
            }

            if(y_derivitive)
            {
                gauss *= (r - row_center);
            }

            (*mask_mpp)->elements[r][c] = gauss;

            /* keep a running sum for normalization later */
            if(x_derivitive)
            {
                /* This works out to finite integration two times:
                 * The first integration undoes the derivitive, and
                 * the second integration sums the original gaussian
                 */
                mask_sum += (num_cols - c) * gauss;
            }
            else if(y_derivitive)
            {
                mask_sum += (num_rows - r) * gauss;
            }
            else
            {
                mask_sum += gauss;
            }
        }
    }

    target_sum = 1;

    ERE(ow_multiply_matrix_by_scalar(*mask_mpp,  target_sum / mask_sum));

    return NO_ERROR;
}


/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                                get_2D_gaussian_mask
 *
 * Constructs a 2D Gaussian mask
 *
 * This routine constructs a 2D Gaussian mask, putting the result into *out_mpp.
 * If the size of the mask is odd, then the center of the Gaussian is in the
 * center of the mask. If it is even, the center is as though the mask was one
 * larger. For example, if your mask size is 6, then the center is the third
 * pixel, which is the same pixel as it it would be if the mask size was 5.
 *
 * If you want a have a mask which contains most of the Gaussian (excluded
 * values are close to zero), then you need to make the mask size at least 6
 * times sigma. Regardless of the size and sigma, the mask is normalized so that
 * its sum 1.
 *
 * If *out_vpp is NULL, then a vector of the appropriate size is created, if
 * it is the wrong size, then it is resized, and if it is the right size, the
 * storage is recycled.
 *
 * Returns:
 *    NO_ERROR on success, and ERROR on failure, with an appropriate error
 *    message being set.
 *
 * TODO:
 *   It seems that the handling of even size masks is due to lazy programming.
 *   We might consider making it so that values for the center pair of pixels
 *   are the same, and the true center value is not represented as it is between
 *   them in real valued coordinates.
 *
 * Index: matrices, convolution
 *
 * -----------------------------------------------------------------------------
*/

int get_2D_gaussian_mask
(
    Matrix** mask_mpp, /* Output gaussian smoothing mask.   */
    int      mask_size,/* Number of elements per axis.      */
    double   sigma     /* Standard deviation in bin units.  */
)
{
#ifdef REFACTORED_GET_2D_GAUSSIAN_MASK
    // remove the ifdef above once we know these are equivalent
    return get_2D_gaussian_mask_dispatch(mask_mpp, mask_size, mask_size,
                                         sigma, sigma, 0, 0);
#else
    int r, c;
    int    mask_center;
    double dist_sqr;
    double sigma_sqr;
    double gauss;
    double mask_sum;

    if (mask_size < 0 )
    {
        SET_ARGUMENT_BUG();
        return ERROR;
    }

    mask_center = mask_size / 2;

    if (IS_EVEN(mask_size) )
    {
         mask_center--;
    }

    sigma_sqr = sigma * sigma;

    mask_sum  = 0.0;

    ERE(get_zero_matrix(mask_mpp, mask_size, mask_size));

    for (r = 0; r < mask_size; r++)
    {
        for (c = 0; c < mask_size; c++)
        {
            dist_sqr = (r - mask_center)*(r - mask_center) +
                                            (c - mask_center)*(c - mask_center);

            gauss = exp( -0.5 * dist_sqr / sigma_sqr );
            (*mask_mpp)->elements[r][c] = gauss;
            mask_sum += gauss;
        }
    }

    ERE(ow_divide_matrix_by_scalar(*mask_mpp,  mask_sum));

    return NO_ERROR;
#endif
}


/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                                get_2D_gaussian_mask_2
 *
 * Constructs a 2D Gaussian mask.
 *
 * This routine constructs a 2D Gaussian mask, putting the result into *out_mpp.
 * If the size of the mask is odd, then the center of the Gaussian is in the
 * center of the mask. If it is even, the the center is as thought the mask was
 * one larger.  If you want a have a mask which contains most of the Gaussian
 * (excluded values are close to zero), then you need to make the mask size at
 * least 6 times sigma. Regardless of the size and sigma, the mask is normalized
 * so that its sum 1.
 *
 * If *out_vpp is NULL, then a vector of the appropriate size is created, if
 * it is the wrong size, then it is resized, and if it is the right size, the
 * storage is recycled.
 *
 * Returns:
 *    NO_ERROR on success, and ERROR on failure, with an appropriate error
 *    message being set.
 *
 * Index: matrices, convolution
 *
 * -----------------------------------------------------------------------------
*/
int get_2D_gaussian_mask_2
(
    Matrix** mask_mpp,  /* Output gaussian smoothing mask.   */
    int      num_rows,  /* Number of rows in mask.      */
    int      num_cols,  /* Number of cols in mask.      */
    double   row_sigma,  /* Standard deviation in row direction in bin units. */
    double   col_sigma   /* Standard deviation in column direction bin units. */
)
{
    return get_2D_gaussian_mask_dispatch(
            mask_mpp,
            num_rows,
            num_cols,
            row_sigma,
            col_sigma,
            0, 0);
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                                get_2D_gaussian_dx_mask
 *
 * Constructs a 2D Gaussian mask with partial derivative in the x-direction.
 *
 * This routine constructs a 2D Gaussian mask combined with a partial derivative
 * in the x-direction, putting the result into *out_mpp.
 * If the size of the mask is odd, then the center of the Gaussian is in the
 * center of the mask. If it is even, the the center is as thought the mask was
 * one larger.  If you want a have a mask which contains most of the Gaussian
 * (excluded values are close to zero), then you need to make the mask size at
 * least 6 times sigma. Regardless of the size and sigma, the mask is normalized
 * so that its sum 1.
 *
 * If *out_vpp is NULL, then a vector of the appropriate size is created, if
 * it is the wrong size, then it is resized, and if it is the right size, the
 * storage is recycled.
 *
 * Returns:
 *    NO_ERROR on success, and ERROR on failure, with an appropriate error
 *    message being set.
 *
 * Index: matrices, convolution
 *
 * -----------------------------------------------------------------------------
*/

int get_2D_gaussian_dx_mask
(
    Matrix** mask_mpp,  /* Output gaussian smoothing mask.   */
    int      num_rows,  /* Number of rows in mask.      */
    int      num_cols,  /* Number of cols in mask.      */
    double   row_sigma, /* Standard deviation in row direction in bin units.  */
    double   col_sigma  /* Standard deviation in column direction bin units.  */
)
{
    ERE(get_2D_gaussian_mask_dispatch(
            mask_mpp,
            num_rows,
            num_cols,
            row_sigma,
            col_sigma,
            1, 0));


    return NO_ERROR;
}


/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                                get_2D_gaussian_dy_mask
 *
 * Constructs a 2D Gaussian mask with partial derivative in the y-direction.
 *
 * This routine constructs a 2D Gaussian mask combined with a partial derivative
 * in the y-direction, putting the result into *out_mpp.
 * If the size of the mask is odd, then the center of the Gaussian is in the
 * center of the mask. If it is even, the the center is as thought the mask was
 * one larger.  If you want a have a mask which contains most of the Gaussian
 * (excluded values are close to zero), then you need to make the mask size at
 * least 6 times sigma. Regardless of the size and sigma, the mask is normalized
 * so that its sum 1.
 *
 * If *out_vpp is NULL, then a vector of the appropriate size is created, if
 * it is the wrong size, then it is resized, and if it is the right size, the
 * storage is recycled.
 *
 * Returns:
 *    NO_ERROR on success, and ERROR on failure, with an appropriate error
 *    message being set.
 *
 * Index: matrices, convolution
 *
 * -----------------------------------------------------------------------------
*/
int get_2D_gaussian_dy_mask
(
    Matrix** mask_mpp,  /* Output gaussian smoothing mask.   */
    int      num_rows,  /* Number of rows in mask.      */
    int      num_cols,  /* Number of cols in mask.      */
    double   row_sigma, /* Standard deviation in row direction in bin units.  */
    double   col_sigma  /* Standard deviation in column direction bin units.  */
)
{
    return get_2D_gaussian_mask_dispatch(
            mask_mpp,
            num_rows,
            num_cols,
            row_sigma,
            col_sigma,
            0, 1);
}

/* /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\  */

/* ============================================================================
 *                                get_1D_gaussian_mask
 *
 * Constructs a 1D Gaussian mask
 *
 * This routine constructs a 1D Gaussian mask, putting the result into *out_vpp.
 * If the size of the mask is odd, then the center of the Gaussian is in the
 * center of the mask.  If it is even, the center is as though the mask was one
 * larger. For example, if your mask size is 6, then the center is the third
 * pixel, which is the same pixel as it it would be if the mask size was 5.
 *
 * If you want a have a mask which contains most of the Gaussian If you want a
 * have a mask which contains most of the Gaussian (excluded values are close to
 * zero), then you need to make the mask size at least 6 times sigma. Regardless
 * of the size and sigma, the mask is normalized so that its sum 1.
 *
 * If *out_vpp is NULL, then a vector of the appropriate size is created, if
 * it is the wrong size, then it is resized, and if it is the right size, the
 * storage is recycled.
 *
 * Returns:
 *    NO_ERROR on success, and ERROR on failure, with an appropriate error
 *    message being set.
 *
 * TODO:
 *   It seems that the handling of even size masks is due to lazy programming.
 *   We might consider making it so that values for the center pair of pixels
 *   are the same, and the true center value is not represented as it is between
 *   them in real valued coordinates.
 *
 * Index: matrices, convolution
 *
 * -----------------------------------------------------------------------------
*/

int get_1D_gaussian_mask
(
    Vector** mask_vpp, /* Output gaussian smoothing mask.           */
    int      mask_size,/* Number of elements per axis. Must be odd. */
    double   sigma     /* Standard deviation in bin units.         */
)
{
    int c;
    int    mask_center;
    double dist_sqr;
    double sigma_sqr;
    double gauss;
    double mask_sum;

    if (mask_size < 0 )
    {
        SET_ARGUMENT_BUG();
        return ERROR;
    }

    mask_center = mask_size / 2;

    if (IS_EVEN(mask_size) )
    {
         mask_center--;
    }

    sigma_sqr = sigma * sigma;

    mask_sum  = 0.0;

    ERE(get_zero_vector(mask_vpp, mask_size));

        for (c = 0; c < mask_size; c++)
        {
            dist_sqr = (c - mask_center)*(c - mask_center);
            gauss = exp( -0.5 * dist_sqr / sigma_sqr );
            (*mask_vpp)->elements[c] = gauss;
            mask_sum += gauss;
        }

    ERE(ow_divide_vector_by_scalar(*mask_vpp,  mask_sum));

    return NO_ERROR;
}

/* /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\  */

#ifdef __cplusplus
}
#endif


