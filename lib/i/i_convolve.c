
/* $Id: i_convolve.c 20654 2016-05-05 23:13:43Z kobus $ */

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

#include "i/i_gen.h"     /* Only safe as first include in a ".c" file. */

#include "i/i_convolve.h"

#define OBSOLETE_NORMALIZE_CONVOLUTIONS


#ifdef __cplusplus
extern "C" {
#endif

/* This macro is the same as one found in m/m_convolve.c and is documented
 * in that file.  If a bug is found in this macro (but see the comments there;
 * it is trickier than it appears), please fix the bug in BOTH files.  Thanks.
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


/* -------------------------------------------------------------------------- */

/* =============================================================================
 *                           ow_gauss_convolve_image
 *
 * Convolves image with Gaussian mask
 *
 * This routine convolves and image with a Gaussian mask, overwriting the image
 * in the process. The mask size is automatically chosen based on the value of
 * sigma.
 *
 * Index: images, convolution, image transformation, image smoothing
 *
 * -----------------------------------------------------------------------------
*/

int ow_gauss_convolve_image(KJB_image* ip, double sigma)
{
    KJB_image* out_ip = NULL;
    int          num_rows = ip->num_rows;
    int          num_cols = ip->num_cols;
    int i, j;

    ERE(gauss_convolve_image(&out_ip, ip, sigma));

    for (i=0; i<num_rows; i++)
    {
        for (j=0; j<num_cols; j++)
        {
            ip->pixels[ i ][ j ] = out_ip->pixels[ i ][ j ];
        }
    }

    kjb_free_image(out_ip);

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                           gauss_convolve_image
 *
 * Convolves image with Gaussian mask
 *
 * This routine convolves the image pointed to by in_ip with a Gaussian mask
 * with the specified sigma, putting the result into *out_ipp.  The mask
 * size is automatically chosen based on the value of sigma.
 *
 * If *out_ipp is NULL, then an image of the appropriate size is created, if
 * it is the wrong size, then it is resized, and if it is the right size, the
 * storage is recycled.
 *
 * Index: images, convolution, image transformation, image smoothing
 *
 * -----------------------------------------------------------------------------
*/

int gauss_convolve_image
(
    KJB_image**      out_ipp,
    const KJB_image* in_ip,
    double           sigma
)
{
    int        mask_width;
    int        mask_size;
    int        status_code;
    Vector*    mask_vp = NULL;
    KJB_image* x_convolve_ip = NULL;


#if 0
    /* HOW IT WAS 23 JULY 2013 (changed by predoehl) */
    mask_width = (int)(1.0 + 2.0 * sigma);
#else
    mask_width = (int)(1.0 + 3.0 * sigma);
#endif
    mask_size  = 1 + 2 * mask_width;

    ERE(get_1D_gaussian_mask(&mask_vp, mask_size, sigma));

    if (ERROR == x_convolve_image(&x_convolve_ip, in_ip, mask_vp))
    {
        free_vector(mask_vp);
        return ERROR;
    }

    status_code = y_convolve_image(out_ipp, x_convolve_ip, mask_vp);
    kjb_free_image(x_convolve_ip);
    free_vector(mask_vp);
    return status_code;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                                convolve_image
 *
 * Convolves image with an arbitrary mask
 *
 * This routine convolves the image pointed to by in_ip with the mask mask_mp,
 * putting the result into *out_ipp.
 *
 * If *out_ipp is NULL, then an image of the appropriate size is created, if
 * it is the wrong size, then it is resized, and if it is the right size, the
 * storage is recycled.
 *
 * Index: images, convolution, image transformation, image smoothing
 *
 * -----------------------------------------------------------------------------
*/

int convolve_image
(
    KJB_image**      out_ipp,
    const KJB_image* in_ip,
    const Matrix*    mask_mp
)
{
    KJB_image* out_ip;
    int        num_rows, num_cols, i, j, mi, mj, m, n;
    int     mask_rows  = mask_mp->num_rows;
    int     mask_cols  = mask_mp->num_cols;
    int     mask_row_offset = mask_rows / 2;
    int     mask_col_offset = mask_cols / 2;

    /* TODO: Better error checking here, not just ASSERT. */
    ASSERT(mask_mp->num_rows < in_ip->num_rows);
    ASSERT(mask_mp->num_cols < in_ip->num_cols);

    num_rows = in_ip->num_rows;
    num_cols = in_ip->num_cols;

    ERE(get_target_image(out_ipp, num_rows, num_cols));
    out_ip = *out_ipp;

    for (i=0; i<num_rows; i++)
    {
        for (j=0; j<num_cols; j++)
        {
            double r_sum      = 0.0;
            double g_sum      = 0.0;
            double b_sum      = 0.0;
            double weight;

            for (mi = 0; mi < mask_rows; mi++)
            {
                for (mj = 0; mj < mask_cols; mj++)
                {
                    m = i + mask_row_offset - mi;
                    n = j + mask_col_offset - mj;

                    /*
                     * TODO: Easy performance gain: Handle the boundary cases
                     * separately.
                     */
                    REFLECT_INBOUNDS(m, num_rows);
                    REFLECT_INBOUNDS(n, num_cols);

                    weight = mask_mp->elements[mi][mj];

                    r_sum += in_ip->pixels[m][n].r * weight;
                    g_sum += in_ip->pixels[m][n].g * weight;
                    b_sum += in_ip->pixels[m][n].b * weight;
                }
            }
            out_ip->pixels[i][j].r = r_sum;
            out_ip->pixels[i][j].g = g_sum;
            out_ip->pixels[i][j].b = b_sum;

            out_ip->pixels[i][j].extra.invalid = in_ip->pixels[i][j].extra.invalid;
        }
    }

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                                x_convolve_image
 *
 * Convolves image with a vector in the x direction
 *
 * This routine convolves the image pointed to by in_ip with the vector mask_vp,
 * putting the result into *out_ipp.
 *
 * If *out_ipp is NULL, then an image of the appropriate size is created, if
 * it is the wrong size, then it is resized, and if it is the right size, the
 * storage is recycled.
 *
 * Index: images, convolution, image transformation, image smoothing
 *
 * -----------------------------------------------------------------------------
*/

int x_convolve_image
(
    KJB_image**      out_ipp,
    const KJB_image* in_ip,
    const Vector*    mask_vp
)
{
    KJB_image* out_ip;
    int        num_rows, num_cols, i, j, mj, n;
    int     mask_cols = mask_vp->length;
    int     mask_col_offset = mask_cols / 2;


    num_rows = in_ip->num_rows;
    num_cols = in_ip->num_cols;

    ERE(get_target_image(out_ipp, num_rows, num_cols));
    out_ip = *out_ipp;

    for (i=0; i<num_rows; i++)
    {
        for (j=0; j<num_cols; j++)
        {
            double r_sum      = 0.0;
            double g_sum      = 0.0;
            double b_sum      = 0.0;
            double weight;

            for (mj = 0; mj < mask_cols; mj++)
            {
                n = j + mask_col_offset - mj;

                REFLECT_INBOUNDS(n, num_cols);

                weight = mask_vp->elements[mj];

                r_sum += in_ip->pixels[i][n].r * weight;
                g_sum += in_ip->pixels[i][n].g * weight;
                b_sum += in_ip->pixels[i][n].b * weight;

            }

            out_ip->pixels[i][j].r = r_sum;
            out_ip->pixels[i][j].g = g_sum;
            out_ip->pixels[i][j].b = b_sum;

            out_ip->pixels[i][j].extra.invalid = in_ip->pixels[i][j].extra.invalid;
        }
    }

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                                y_convolve_image
 *
 * Convolves image with a vector in the x direction
 *
 * This routine convolves the image pointed to by in_ip with the vector mask_vp,
 * putting the result into *out_ipp.
 *
 * If *out_ipp is NULL, then an image of the appropriate size is created, if
 * it is the wrong size, then it is resized, and if it is the right size, the
 * storage is recycled.
 *
 * Index: images, convolution, image transformation, image smoothing
 *
 * -----------------------------------------------------------------------------
*/

int y_convolve_image
(
    KJB_image**      out_ipp,
    const KJB_image* in_ip,
    const Vector*    mask_vp
)
{
    KJB_image* out_ip;
    int        num_rows, num_cols, i, j, mi, m;
    int     mask_rows  = mask_vp->length;
    int     mask_row_offset = mask_rows / 2;


    num_rows = in_ip->num_rows;
    num_cols = in_ip->num_cols;

    ERE(get_target_image(out_ipp, num_rows, num_cols));
    out_ip = *out_ipp;

    for (i=0; i<num_rows; i++)
    {
        for (j=0; j<num_cols; j++)
        {
            double r_sum      = 0.0;
            double g_sum      = 0.0;
            double b_sum      = 0.0;
            double weight;

            for (mi = 0; mi < mask_rows; mi++)
            {
                m = i + mask_row_offset - mi;

                REFLECT_INBOUNDS(m, num_rows);

                weight = mask_vp->elements[mi];

                r_sum += in_ip->pixels[m][j].r * weight;
                g_sum += in_ip->pixels[m][j].g * weight;
                b_sum += in_ip->pixels[m][j].b * weight;
            }

            out_ip->pixels[i][j].r = r_sum;
            out_ip->pixels[i][j].g = g_sum;
            out_ip->pixels[i][j].b = b_sum;

            out_ip->pixels[i][j].extra.invalid = in_ip->pixels[i][j].extra.invalid;
        }
    }

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                           gauss_sample_image
 *
 * Takes gaussian samples of an image
 *
 * This routine reduces image resolution by taking averages with Gaussian
 * weights.  The 'resolution' argument is equal to the linear shrinkage factor.
 * For example, if resolution=3 then the output image will have one-third the
 * number of rows and one-third the number of columns of the input image.
 * This is also known as the decimation factor.
 *
 * The 'sigma' argument obviously controls the antialiasing kernel size.
 * As a rule of thumb, the value of sigma should be around 0.5 to 1.0 times
 * the value of resolution.
 *
 * If *out_ipp is NULL, then an image of the appropriate size is created, if
 * it is the wrong size, then it is resized, and if it is the right size, the
 * storage is recycled.
 *
 * Index: images, convolution, image transformation, image smoothing
 *
 * -----------------------------------------------------------------------------
*/

int gauss_sample_image
(
    KJB_image**      out_ipp,
    const KJB_image* in_ip,
    int              resolution,
    double           sigma
)
{
    KJB_image* out_ip;
    int        num_rows, num_cols, oi, oj, i, j, mi, mj, m, n;
    int        num_out_rows;
    int        num_out_cols;
    int        mask_width;
    int        mask_size;
    Matrix*    mask_mp      = NULL;


    num_rows = in_ip->num_rows;
    num_cols = in_ip->num_cols;

    num_out_rows = num_rows / resolution;
    num_out_cols = num_cols / resolution;

    ERE(get_target_image(out_ipp, num_out_rows, num_out_cols));
    out_ip = *out_ipp;

#if 0
    /* HOW_IT_WAS_JULY_13_2002 */
    /* EXTRA_FOR_ROUNDING added July 14, 2002. */
    mask_width = 1.0 + 2.0 * sigma;
#elif 0
    /* HOW IT WAS 23 JULY 2013 (changed by predoehl) */
    mask_width = (int)(1.0 + 2.0 * sigma + EXTRA_FOR_ROUNDING);
#else
    mask_width = (int)(1.0 + 3.0 * sigma + EXTRA_FOR_ROUNDING);
#endif
    mask_size  = 1 + 2 * mask_width;

    ERE(get_2D_gaussian_mask(&mask_mp, mask_size, sigma));

    i = resolution / 2;

    for (oi=0; oi<num_out_rows; oi++)
    {
        j = resolution / 2;

        for (oj=0; oj<num_out_cols; oj++)
        {
            double r_sum      = 0.0;
            double g_sum      = 0.0;
            double b_sum      = 0.0;
            double weight_sum = 0.0;
            double weight;

            for (mi = 0; mi < mask_size; mi++)
            {
                for (mj = 0; mj < mask_size; mj++)
                {
                    m = i + mi - mask_width;
                    n = j + mj - mask_width;

                    if (   (m >= 0) && (m < num_rows)
                        && (n >= 0) && (n < num_cols)
                        /*
                        && ( ! in_ip->pixels[m][n].extra.invalid.pixel)
                        */
                       )
                    {
                        weight = mask_mp->elements[mi][mj];

                        r_sum += in_ip->pixels[m][n].r * weight;
                        g_sum += in_ip->pixels[m][n].g * weight;
                        b_sum += in_ip->pixels[m][n].b * weight;

                        weight_sum += weight;
                    }
                }
            }

            if (weight_sum > 0.0)
            {
                r_sum /= weight_sum;
                g_sum /= weight_sum;
                b_sum /= weight_sum;

                out_ip->pixels[oi][oj].r = r_sum;
                out_ip->pixels[oi][oj].g = g_sum;
                out_ip->pixels[oi][oj].b = b_sum;

                out_ip->pixels[oi][oj].extra.invalid.pixel = VALID_PIXEL;
                out_ip->pixels[oi][oj].extra.invalid.r = VALID_PIXEL;
                out_ip->pixels[oi][oj].extra.invalid.g = VALID_PIXEL;
                out_ip->pixels[oi][oj].extra.invalid.b = VALID_PIXEL;
            }
            else
            {
                out_ip->pixels[oi][oj].r = in_ip->pixels[i][j].r;
                out_ip->pixels[oi][oj].g = in_ip->pixels[i][j].g;
                out_ip->pixels[oi][oj].b = in_ip->pixels[i][j].b;

                out_ip->pixels[oi][oj].extra.invalid.pixel = INVALID_PIXEL;
                out_ip->pixels[oi][oj].extra.invalid.r = INVALID_PIXEL;
                out_ip->pixels[oi][oj].extra.invalid.g = INVALID_PIXEL;
                out_ip->pixels[oi][oj].extra.invalid.b = INVALID_PIXEL;
            }

            j += resolution;
        }

        i += resolution;
    }

    free_matrix(mask_mp);

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

#ifdef __cplusplus
}
#endif

