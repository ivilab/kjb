
/* $Id: i_arithmetic.c 5831 2010-05-02 21:52:24Z ksimek $ */

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
#include "i/i_arithmetic.h"

#ifdef __cplusplus
extern "C" {
#endif
 
/* -------------------------------------------------------------------------- */

/* =============================================================================
 *                             power_image
 *
 * Raises image values to a power
 *
 * This routine raises all image pixels to the exponant in argument "power".
 *
 * Returns:
 *     NO_ERROR on sucess and ERROR on failure (unlikely!).
 *
 * Index: images, image arithmetic
 *
 * -----------------------------------------------------------------------------
*/

int power_image
(
    KJB_image**      out_ipp,
    const KJB_image* in_ip,
    double           power
)
{
    Pixel* in_pos;
    Pixel* out_pos;
    int    i, j, num_rows, num_cols;


    num_rows = in_ip->num_rows;
    num_cols = in_ip->num_cols;

    ERE(get_target_image(out_ipp, num_rows, num_cols));

    for (i=0; i<num_rows; i++)
    {
        in_pos  = in_ip->pixels[ i ];
        out_pos  = (*out_ipp)->pixels[ i ];

        for (j=0; j<num_cols; j++)
        {
            double temp;

            temp = in_pos->r;
            out_pos->r = pow(temp, power);

            temp = in_pos->g;
            out_pos->g = pow(temp, power);

            temp = in_pos->b;
            out_pos->b = pow(temp, power);

            in_pos++;
            out_pos++;
        }
    }

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                             ow_power_image
 *
 * Raises image values to a power
 *
 * This routine raises all image pixels to the exponant in argument "power".
 * The calculations are done in place.
 *
 * Returns:
 *     NO_ERROR on sucess and ERROR on failure (unlikely!).
 *
 * Index: images, image arithmetic
 *
 * -----------------------------------------------------------------------------
*/

int ow_power_image(KJB_image* in_ip, double power)
{
    Pixel* in_pos;
    int    i, j, num_rows, num_cols;


    num_rows = in_ip->num_rows;
    num_cols = in_ip->num_cols;

    for (i=0; i<num_rows; i++)
    {
        in_pos  = in_ip->pixels[ i ];

        for (j=0; j<num_cols; j++)
        {
            double temp;

            temp = in_pos->r;
            in_pos->r = pow(temp, power);

            temp = in_pos->g;
            in_pos->g = pow(temp, power);

            temp = in_pos->b;
            in_pos->b = pow(temp, power);

            in_pos++;
        }
    }

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                             ow_scale_image
 *
 * Scales an image by a factor
 *
 * This routine multiplies all image pixels by the factor in argument "factor".
 * The scaling is done in place.
 *
 * Returns:
 *     NO_ERROR on sucess and ERROR on failure (unlikely!).
 *
 * Index: images, image arithmetic
 *
 * -----------------------------------------------------------------------------
*/

int ow_scale_image(KJB_image* in_ip, double factor)
{
    Pixel* in_pos;
    int    i, j, num_rows, num_cols;


    num_rows = in_ip->num_rows;
    num_cols = in_ip->num_cols;

    for (i=0; i<num_rows; i++)
    {
        in_pos  = in_ip->pixels[ i ];

        for (j=0; j<num_cols; j++)
        {
            in_pos->r *= factor;
            in_pos->g *= factor;
            in_pos->b *= factor;

            in_pos++;
        }
    }

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                                  scale_image
 *
 * Scales an image by a factor
 *
 * This routine computes multiplies all image pixels by the factor in argument
 * "factor". The result is put into a second image which is created as needed.
 * The original image is not touched.
 *
 * Returns:
 *     NO_ERROR on sucess and ERROR on failure.
 *
 * Index: images, image arithmetic
 *
 * -----------------------------------------------------------------------------
*/

int scale_image
(
    KJB_image**      out_ipp,
    const KJB_image* in_ip,
    double           factor
)
{
    KJB_image* out_ip;
    Pixel*     in_pos;
    Pixel*     out_pos;
    int        i, j, num_rows, num_cols;


    num_rows = in_ip->num_rows;
    num_cols = in_ip->num_cols;

    ERE(get_target_image(out_ipp, num_rows, num_cols));
    out_ip = *out_ipp;

    for (i=0; i<num_rows; i++)
    {
        in_pos  = in_ip->pixels[ i ];
        out_pos = out_ip->pixels[ i ];

        for (j=0; j<num_cols; j++)
        {
            out_pos->r = in_pos->r * factor;
            out_pos->g = in_pos->g * factor;
            out_pos->b = in_pos->b * factor;
            out_pos->extra.invalid = in_pos->extra.invalid;

            in_pos++;
            out_pos++;
        }
    }

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                            ow_scale_image_by_channel
 *
 * Scales each channel of an image
 *
 * This routine scales each image channel by a different scale factor. The scale
 * factors are in the vector argument "scale_vp". The scaling is done in place,
 * changing the input image.
 *
 * Returns:
 *     NO_ERROR on sucess and ERROR on failure.
 *
 * Index: images, image arithmetic
 *
 * -----------------------------------------------------------------------------
*/

int ow_scale_image_by_channel(KJB_image* in_ip, Vector* scale_vp)
{
    int    num_rows, num_cols, i, j;
    Pixel* in_pos;
    float  red_factor;
    float  green_factor;
    float  blue_factor;


    num_rows = in_ip->num_rows;
    num_cols = in_ip->num_cols;

    red_factor   = scale_vp->elements[ 0 ];
    green_factor = scale_vp->elements[ 1 ];
    blue_factor  = scale_vp->elements[ 2 ];

    for (i=0; i<num_rows; i++)
    {
        in_pos  = in_ip->pixels[ i ];

        for (j=0; j<num_cols; j++)
        {
            in_pos->r *= red_factor;
            in_pos->g *= green_factor;
            in_pos->b *= blue_factor;

            in_pos++;
        }
    }

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                            scale_image_by_channel
 *
 * Scales each channel of an image
 *
 * This routine scales each image channel by a different scale factor. The scale
 * factors are in the vector argument "scale_vp". The scaled image is put into a
 * second image, *out_ipp, which is created if necessary. The input image is
 * not touched.
 *
 * Returns:
 *     NO_ERROR on sucess and ERROR on failure.
 *
 * Index: images, image arithmetic
 *
 * -----------------------------------------------------------------------------
*/

int scale_image_by_channel
(
    KJB_image**      out_ipp,
    const KJB_image* in_ip,
    Vector*          scale_vp
)
{
    KJB_image* out_ip;
    int        num_rows, num_cols, i, j;
    Pixel*     in_pos;
    Pixel*     out_pos;
    float      red_factor;
    float      green_factor;
    float      blue_factor;


    num_rows = in_ip->num_rows;
    num_cols = in_ip->num_cols;

    ERE(get_target_image(out_ipp, num_rows, num_cols));
    out_ip = *out_ipp;

    red_factor   = scale_vp->elements[ 0 ];
    green_factor = scale_vp->elements[ 1 ];
    blue_factor  = scale_vp->elements[ 2 ];

    for (i=0; i<num_rows; i++)
    {
        in_pos  = in_ip->pixels[ i ];
        out_pos = out_ip->pixels[ i ];

        for (j=0; j<num_cols; j++)
        {
            out_pos->r = in_pos->r * red_factor;
            out_pos->g = in_pos->g * green_factor;
            out_pos->b = in_pos->b * blue_factor;
            out_pos->extra.invalid = in_pos->extra.invalid;

            in_pos++;
            out_pos++;
        }
    }

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                                   subtract_images
 *
 * Computes the difference of two images.
 *
 * This routine computes the difference of two images. The result is put into
 * *out_ipp which is created or resized if necessary.
 *
 * Returns:
 *     NO_ERROR on sucess and ERROR on failure.
 *
 * Index: images, image arithmetic
 *
 * -----------------------------------------------------------------------------
*/

int subtract_images
(
    KJB_image**      out_ipp,
    const KJB_image* in1_ip,
    const KJB_image* in2_ip
)
{
    KJB_image* out_ip;
    Pixel*     in1_pos;
    Pixel*     in2_pos;
    Pixel*     out_pos;
    int        i, j, num_rows, num_cols;


    ERE(check_same_size_image(in1_ip, in2_ip));

    num_rows = in1_ip->num_rows;
    num_cols = in1_ip->num_cols;

    ERE(get_target_image(out_ipp, num_rows, num_cols));
    out_ip = *out_ipp;

    for (i=0; i<num_rows; i++)
    {
        in1_pos  = in1_ip->pixels[ i ];
        in2_pos  = in2_ip->pixels[ i ];
        out_pos = out_ip->pixels[ i ];

        for (j=0; j<num_cols; j++)
        {
            out_pos->r = in1_pos->r - in2_pos->r;
            out_pos->g = in1_pos->g - in2_pos->g;
            out_pos->b = in1_pos->b - in2_pos->b;

            out_pos->extra.invalid.r = in1_pos->extra.invalid.r | in2_pos->extra.invalid.r;
            out_pos->extra.invalid.g = in1_pos->extra.invalid.r | in2_pos->extra.invalid.r;
            out_pos->extra.invalid.b = in1_pos->extra.invalid.r | in2_pos->extra.invalid.r;
            out_pos->extra.invalid.pixel = in1_pos->extra.invalid.pixel |
                                                        in2_pos->extra.invalid.pixel;

            in1_pos++;
            in2_pos++;
            out_pos++;
        }
    }

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                                ow_subtract_images
 *
 * Subtracts two images
 *
 * This routine subtracts the image pointed to by in2_ip from that pointed to by
 * in1_ip. The image pointed to by in1_ip is modified.
 *
 * Returns:
 *     NO_ERROR on sucess and ERROR on failure.
 *
 * Index: images, image arithmetic
 *
 * -----------------------------------------------------------------------------
*/

int ow_subtract_images(KJB_image* in1_ip, const KJB_image* in2_ip)
{
    Pixel* in1_pos;
    Pixel* in2_pos;
    int    i, j, num_rows, num_cols;


    ERE(check_same_size_image(in1_ip, in2_ip));

    num_rows = in1_ip->num_rows;
    num_cols = in1_ip->num_cols;

    for (i=0; i<num_rows; i++)
    {
        in1_pos  = in1_ip->pixels[ i ];
        in2_pos  = in2_ip->pixels[ i ];

        for (j=0; j<num_cols; j++)
        {
            in1_pos->r -= in2_pos->r;
            in1_pos->g -= in2_pos->g;
            in1_pos->b -= in2_pos->b;

            in1_pos->extra.invalid.r     |= in2_pos->extra.invalid.r;
            in1_pos->extra.invalid.r     |= in2_pos->extra.invalid.r;
            in1_pos->extra.invalid.r     |= in2_pos->extra.invalid.r;
            in1_pos->extra.invalid.pixel |= in2_pos->extra.invalid.pixel;

            in1_pos++;
            in2_pos++;
        }
    }

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                                   multiply_images
 *
 * Computes the product of two images.
 *
 * This routine computes the product of two images. The result is put into
 * *out_ipp which is created or resized if necessary.
 *
 * Returns:
 *     NO_ERROR on sucess and ERROR on failure.
 *
 * Index: images, image arithmetic
 *
 * -----------------------------------------------------------------------------
*/

int multiply_images
(
    KJB_image**      out_ipp,
    const KJB_image* in1_ip,
    const KJB_image* in2_ip
)
{
    KJB_image* out_ip;
    Pixel*     in1_pos;
    Pixel*     in2_pos;
    Pixel*     out_pos;
    int        i, j, num_rows, num_cols;


    UNTESTED_CODE();

    ERE(check_same_size_image(in1_ip, in2_ip));

    num_rows = in1_ip->num_rows;
    num_cols = in1_ip->num_cols;

    ERE(get_target_image(out_ipp, num_rows, num_cols));
    out_ip = *out_ipp;

    for (i=0; i<num_rows; i++)
    {
        in1_pos  = in1_ip->pixels[ i ];
        in2_pos  = in2_ip->pixels[ i ];
        out_pos = out_ip->pixels[ i ];

        for (j=0; j<num_cols; j++)
        {
            out_pos->r = in1_pos->r * in2_pos->r;
            out_pos->g = in1_pos->g * in2_pos->g;
            out_pos->b = in1_pos->b * in2_pos->b;

            out_pos->extra.invalid.r = in1_pos->extra.invalid.r | in2_pos->extra.invalid.r;
            out_pos->extra.invalid.g = in1_pos->extra.invalid.r | in2_pos->extra.invalid.r;
            out_pos->extra.invalid.b = in1_pos->extra.invalid.r | in2_pos->extra.invalid.r;
            out_pos->extra.invalid.pixel = in1_pos->extra.invalid.pixel |
                                                        in2_pos->extra.invalid.pixel;

            in1_pos++;
            in2_pos++;
            out_pos++;
        }
    }

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                                ow_multiply_images
 *
 * Multiplies two images
 *
 * This routine multiplys the image pointed to by in2_ip from that pointed to
 * by in1_ip. The image pointed to by in1_ip is modified.
 *
 * Returns:
 *     NO_ERROR on sucess and ERROR on failure.
 *
 * Index: images, image arithmetic
 *
 * -----------------------------------------------------------------------------
*/

int ow_multiply_images(KJB_image* in1_ip, const KJB_image* in2_ip)
{
    Pixel* in1_pos;
    Pixel* in2_pos;
    int    i, j, num_rows, num_cols;


    UNTESTED_CODE();

    ERE(check_same_size_image(in1_ip, in2_ip));

    num_rows = in1_ip->num_rows;
    num_cols = in1_ip->num_cols;

    for (i=0; i<num_rows; i++)
    {
        in1_pos  = in1_ip->pixels[ i ];
        in2_pos  = in2_ip->pixels[ i ];

        for (j=0; j<num_cols; j++)
        {
            in1_pos->r *= in2_pos->r;
            in1_pos->g *= in2_pos->g;
            in1_pos->b *= in2_pos->b;

            in1_pos->extra.invalid.r     |= in2_pos->extra.invalid.r;
            in1_pos->extra.invalid.r     |= in2_pos->extra.invalid.r;
            in1_pos->extra.invalid.r     |= in2_pos->extra.invalid.r;
            in1_pos->extra.invalid.pixel |= in2_pos->extra.invalid.pixel;

            in1_pos++;
            in2_pos++;
        }
    }

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                                   divide_images
 *
 * Computes the quotient of two images.
 *
 * This routine computes the quotient of two images. The result is put into
 * *out_ipp which is created or resized if necessary.
 *
 * Returns:
 *     NO_ERROR on sucess and ERROR on failure.
 *
 * Index: images, image arithmetic
 *
 * -----------------------------------------------------------------------------
*/

int divide_images
(
    KJB_image**      out_ipp,
    const KJB_image* in1_ip,
    const KJB_image* in2_ip
)
{
    KJB_image* out_ip;
    Pixel*     in1_pos;
    Pixel*     in2_pos;
    Pixel*     out_pos;
    int        i, j, num_rows, num_cols;


    UNTESTED_CODE();

    ERE(check_same_size_image(in1_ip, in2_ip));

    num_rows = in1_ip->num_rows;
    num_cols = in1_ip->num_cols;

    ERE(get_target_image(out_ipp, num_rows, num_cols));
    out_ip = *out_ipp;

    for (i=0; i<num_rows; i++)
    {
        in1_pos  = in1_ip->pixels[ i ];
        in2_pos  = in2_ip->pixels[ i ];
        out_pos = out_ip->pixels[ i ];

        for (j=0; j<num_cols; j++)
        {
            out_pos->r = MAX_OF(1e-6, in1_pos->r / in2_pos->r);
            out_pos->g = MAX_OF(1e-6, in1_pos->g / in2_pos->g);
            out_pos->b = MAX_OF(1e-6, in1_pos->b / in2_pos->b);

            out_pos->extra.invalid.r = in1_pos->extra.invalid.r | in2_pos->extra.invalid.r;
            out_pos->extra.invalid.g = in1_pos->extra.invalid.r | in2_pos->extra.invalid.r;
            out_pos->extra.invalid.b = in1_pos->extra.invalid.r | in2_pos->extra.invalid.r;
            out_pos->extra.invalid.pixel = in1_pos->extra.invalid.pixel |
                                                        in2_pos->extra.invalid.pixel;

            in1_pos++;
            in2_pos++;
            out_pos++;
        }
    }

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                                ow_divide_images
 *
 * Multiplies two images
 *
 * This routine divides the image pointed to by in2_ip from that pointed to
 * by in1_ip. The image pointed to by in1_ip is modified.
 *
 * Returns:
 *     NO_ERROR on sucess and ERROR on failure.
 *
 * Index: images, image arithmetic
 *
 * -----------------------------------------------------------------------------
*/

int ow_divide_images(KJB_image* in1_ip, const KJB_image* in2_ip)
{
    Pixel* in1_pos;
    Pixel* in2_pos;
    int    i, j, num_rows, num_cols;


    UNTESTED_CODE();

    ERE(check_same_size_image(in1_ip, in2_ip));

    num_rows = in1_ip->num_rows;
    num_cols = in1_ip->num_cols;

    for (i=0; i<num_rows; i++)
    {
        in1_pos  = in1_ip->pixels[ i ];
        in2_pos  = in2_ip->pixels[ i ];

        for (j=0; j<num_cols; j++)
        {
            in1_pos->r /= MAX_OF(1e-6, in2_pos->r);
            in1_pos->g /= MAX_OF(1e-6, in2_pos->g);
            in1_pos->b /= MAX_OF(1e-6, in2_pos->b);

            in1_pos->extra.invalid.r     |= in2_pos->extra.invalid.r;
            in1_pos->extra.invalid.r     |= in2_pos->extra.invalid.r;
            in1_pos->extra.invalid.r     |= in2_pos->extra.invalid.r;
            in1_pos->extra.invalid.pixel |= in2_pos->extra.invalid.pixel;

            in1_pos++;
            in2_pos++;
        }
    }

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                                ow_min_of_images
 *
 * Takes the min two images
 *
 * This routine takes the min of the images pointed to by in2_ip and in1_ip.
 * The image pointed to by in1_ip is modified.
 *
 * Returns:
 *     NO_ERROR on sucess and ERROR on failure.
 *
 * Index: images, image arithmetic
 *
 * -----------------------------------------------------------------------------
*/

int ow_min_of_images(KJB_image* in1_ip, const KJB_image* in2_ip)
{
    Pixel* in1_pos;
    Pixel* in2_pos;
    int    i, j, num_rows, num_cols;


    UNTESTED_CODE();

    ERE(check_same_size_image(in1_ip, in2_ip));

    num_rows = in1_ip->num_rows;
    num_cols = in1_ip->num_cols;

    for (i=0; i<num_rows; i++)
    {
        in1_pos  = in1_ip->pixels[ i ];
        in2_pos  = in2_ip->pixels[ i ];

        for (j=0; j<num_cols; j++)
        {
            in1_pos->r = MIN_OF(in1_pos->r, in2_pos->r);
            in1_pos->g = MIN_OF(in1_pos->g, in2_pos->g);
            in1_pos->b = MIN_OF(in1_pos->b, in2_pos->b);

            in1_pos->extra.invalid.r     |= in2_pos->extra.invalid.r;
            in1_pos->extra.invalid.r     |= in2_pos->extra.invalid.r;
            in1_pos->extra.invalid.r     |= in2_pos->extra.invalid.r;
            in1_pos->extra.invalid.pixel |= in2_pos->extra.invalid.pixel;

            in1_pos++;
            in2_pos++;
        }
    }

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                                   add_images
 *
 * Computes the sum of two images.
 *
 * This routine computes the sum of two images. The result is put into
 * *out_ipp which is created or resized if necessary.
 *
 * Returns:
 *     NO_ERROR on sucess and ERROR on failure.
 *
 * Index: images, image arithmetic
 *
 * -----------------------------------------------------------------------------
*/

int add_images
(
    KJB_image**      out_ipp,
    const KJB_image* in1_ip,
    const KJB_image* in2_ip
)
{
    KJB_image* out_ip;
    Pixel*     in1_pos;
    Pixel*     in2_pos;
    Pixel*     out_pos;
    int        i, j, num_rows, num_cols;


    ERE(check_same_size_image(in1_ip, in2_ip));

    num_rows = in1_ip->num_rows;
    num_cols = in1_ip->num_cols;

    ERE(get_target_image(out_ipp, num_rows, num_cols));
    out_ip = *out_ipp;

    for (i=0; i<num_rows; i++)
    {
        in1_pos  = in1_ip->pixels[ i ];
        in2_pos  = in2_ip->pixels[ i ];
        out_pos = out_ip->pixels[ i ];

        for (j=0; j<num_cols; j++)
        {
            out_pos->r = in1_pos->r + in2_pos->r;
            out_pos->g = in1_pos->g + in2_pos->g;
            out_pos->b = in1_pos->b + in2_pos->b;

            out_pos->extra.invalid.r = in1_pos->extra.invalid.r | in2_pos->extra.invalid.r;
            out_pos->extra.invalid.g = in1_pos->extra.invalid.r | in2_pos->extra.invalid.r;
            out_pos->extra.invalid.b = in1_pos->extra.invalid.r | in2_pos->extra.invalid.r;
            out_pos->extra.invalid.pixel = in1_pos->extra.invalid.pixel |
                                                        in2_pos->extra.invalid.pixel;

            in1_pos++;
            in2_pos++;
            out_pos++;
        }
    }

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                                ow_add_images
 *
 * Adds two images
 *
 * This routine adds the image pointed to by in2_ip from that pointed to by
 * in1_ip. The image pointed to by in1_ip is modified.
 *
 * Returns:
 *     NO_ERROR on sucess and ERROR on failure.
 *
 * Index: images, image arithmetic
 *
 * -----------------------------------------------------------------------------
*/

int ow_add_images(KJB_image* in1_ip, const KJB_image* in2_ip)
{
    Pixel* in1_pos;
    Pixel* in2_pos;
    int    i, j, num_rows, num_cols;


    ERE(check_same_size_image(in1_ip, in2_ip));

    num_rows = in1_ip->num_rows;
    num_cols = in1_ip->num_cols;

    for (i=0; i<num_rows; i++)
    {
        in1_pos  = in1_ip->pixels[ i ];
        in2_pos  = in2_ip->pixels[ i ];

        for (j=0; j<num_cols; j++)
        {
            in1_pos->r += in2_pos->r;
            in1_pos->g += in2_pos->g;
            in1_pos->b += in2_pos->b;

            in1_pos->extra.invalid.r     |= in2_pos->extra.invalid.r;
            in1_pos->extra.invalid.r     |= in2_pos->extra.invalid.r;
            in1_pos->extra.invalid.r     |= in2_pos->extra.invalid.r;
            in1_pos->extra.invalid.pixel |= in2_pos->extra.invalid.pixel;

            in1_pos++;
            in2_pos++;
        }
    }

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                          ow_subtract_vector_from_image
 *
 * Subtracts an RGB from each pixel in an image
 *
 * This routine subtracts the RGB in the vector pointed to by the input argument
 * "vp" from each pixel in the image pointed to by "ip".
 *
 * Returns:
 *     NO_ERROR on sucess and ERROR on failure.
 *
 * Index: images, image arithmetic
 *
 * -----------------------------------------------------------------------------
*/

int ow_subtract_vector_from_image(KJB_image* ip, const Vector* vp)
{
    Pixel* in_pos;
    int    i, j, num_rows, num_cols;
    float  r, g, b;


    r = vp->elements[ 0 ];
    g = vp->elements[ 1 ];
    b = vp->elements[ 2 ];

    num_rows = ip->num_rows;
    num_cols = ip->num_cols;

    for (i=0; i<num_rows; i++)
    {
        in_pos  = ip->pixels[ i ];

        for (j=0; j<num_cols; j++)
        {
            in_pos->r -= r;
            in_pos->g -= g;
            in_pos->b -= b;

            in_pos++;
        }
    }

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                          ow_add_vector_from_image
 *
 * Adds an RGB from each pixel in an image
 *
 * This routine adds the RGB in the vector pointed to by the input argument
 * "vp" to each pixel in the image pointed to by "ip".
 *
 * Returns:
 *     NO_ERROR on sucess and ERROR on failure.
 *
 * Index: images, image arithmetic
 *
 * -----------------------------------------------------------------------------
*/

int ow_add_vector_to_image(KJB_image* ip, const Vector* vp)
{
    Pixel* in_pos;
    int    i, j, num_rows, num_cols;
    float  r, g, b;


    r = vp->elements[ 0 ];
    g = vp->elements[ 1 ];
    b = vp->elements[ 2 ];

    num_rows = ip->num_rows;
    num_cols = ip->num_cols;

    for (i=0; i<num_rows; i++)
    {
        in_pos = ip->pixels[ i ];

        for (j=0; j<num_cols; j++)
        {
            in_pos->r += r;
            in_pos->g += g;
            in_pos->b += b;

            in_pos++;
        }
    }

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                          ow_min_thresh_image
 *
 * Thresholds pixel components
 *
 * This routine sets all pixel components less than the threshold "min" to min,
 * leaving the others untouched.
 *
 * Returns:
 *     NO_ERROR on sucess and ERROR on failure.
 *
 * Index: images, image arithmetic
 *
 * -----------------------------------------------------------------------------
*/

int ow_min_thresh_image(KJB_image* ip, double min)
{
    Pixel* in_pos;
    int    i, j, num_rows, num_cols;
    float  float_min = min;


    num_rows = ip->num_rows;
    num_cols = ip->num_cols;

    for (i=0; i<num_rows; i++)
    {
        in_pos  = ip->pixels[ i ];

        for (j=0; j<num_cols; j++)
        {
            if (in_pos->r < float_min)
            {
                in_pos->r = float_min;
                in_pos->extra.invalid.r |= TRUNCATED_PIXEL;
            }

            if (in_pos->g < float_min)
            {
                in_pos->g = float_min;
                in_pos->extra.invalid.g |= TRUNCATED_PIXEL;
            }

            if (in_pos->b < float_min)
            {
                in_pos->b = float_min;
                in_pos->extra.invalid.b |= TRUNCATED_PIXEL;
            }

            in_pos->extra.invalid.pixel =
                    in_pos->extra.invalid.r | in_pos->extra.invalid.g | in_pos->extra.invalid.b;

            in_pos++;

        }

    }

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                          ow_max_thresh_image
 *
 * Thresholds pixel components
 *
 * This routine sets all pixel components greather than the threshold "max" to
 * max, leaving the others untouched.
 *
 * Returns:
 *     NO_ERROR on sucess and ERROR on failure.
 *
 * Index: images, image arithmetic
 *
 * -----------------------------------------------------------------------------
*/

int ow_max_thresh_image(KJB_image* ip, double max)
{
    Pixel* in_pos;
    int    i, j, num_rows, num_cols;
    float  float_max = max;


    num_rows = ip->num_rows;
    num_cols = ip->num_cols;

    for (i=0; i<num_rows; i++)
    {
        in_pos  = ip->pixels[ i ];

        for (j=0; j<num_cols; j++)
        {
            if (in_pos->r > float_max)
            {
                in_pos->r = float_max;
                in_pos->extra.invalid.r |= TRUNCATED_PIXEL;
            }

            if (in_pos->g > float_max)
            {
                in_pos->g = float_max;
                in_pos->extra.invalid.g |= TRUNCATED_PIXEL;
            }

            if (in_pos->b > float_max)
            {
                in_pos->b = float_max;
                in_pos->extra.invalid.b |= TRUNCATED_PIXEL;
            }

            in_pos->extra.invalid.pixel =
                    in_pos->extra.invalid.r | in_pos->extra.invalid.g | in_pos->extra.invalid.b;

            in_pos++;
        }
    }

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                             log_one_plus_image
 *
 * Computes log(x + 1) for each channel
 *
 * This routine computes the log of each channel, putting the result into
 * *out_ipp which is created or resized as needed.
 *
 * Returns:
 *     NO_ERROR on sucess and ERROR on failure.
 *
 * Index: images, image arithmetic
 *
 * -----------------------------------------------------------------------------
*/

int log_one_plus_image(KJB_image** out_ipp, const KJB_image* in_ip)
{
    KJB_image* out_ip;
    Pixel*     in_pos;
    Pixel*     out_pos;
    int        i, j, num_rows, num_cols;
    double     temp;


    num_rows = in_ip->num_rows;
    num_cols = in_ip->num_cols;

    ERE(get_target_image(out_ipp, num_rows, num_cols));
    out_ip = *out_ipp;

    for (i=0; i<num_rows; i++)
    {
        in_pos  = in_ip->pixels[ i ];
        out_pos = out_ip->pixels[ i ];

        for (j=0; j<num_cols; j++)
        {
            out_pos->extra.invalid = in_pos->extra.invalid;

            temp = in_pos->r;

            if (temp < 0.0)
            {
                temp = 0.0;
                out_pos->extra.invalid.r     |= ILLEGAL_MATH_OP_PIXEL;
                out_pos->extra.invalid.pixel |= ILLEGAL_MATH_OP_PIXEL;
            }

            out_pos->r = log(1.0 + temp);


            temp = in_pos->g;

            if (temp < 0.0)
            {
                temp = 0.0;
                out_pos->extra.invalid.g     |= ILLEGAL_MATH_OP_PIXEL;
                out_pos->extra.invalid.pixel |= ILLEGAL_MATH_OP_PIXEL;
            }

            out_pos->g = log(1.0 + temp);


            temp = in_pos->b;

            if (temp < 0.0)
            {
                temp = 0.0;
                out_pos->extra.invalid.b     |= ILLEGAL_MATH_OP_PIXEL;
                out_pos->extra.invalid.pixel |= ILLEGAL_MATH_OP_PIXEL;
            }

            out_pos->b = log(1.0 + temp);

            in_pos++;
            out_pos++;
        }
    }

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                             log_one_plus_image
 *
 * Computes log(x + 1) for each channel
 *
 * This routine computes the log of each channel in place (the input is
 * overwritten with the result).
 *
 * Returns:
 *     NO_ERROR on sucess and ERROR on failure.
 *
 * Index: images, image arithmetic
 *
 * -----------------------------------------------------------------------------
*/

int ow_log_one_plus_image(KJB_image* in_ip)
{
    Pixel* in_pos;
    int    i, j, num_rows, num_cols;
    double temp;


    num_rows = in_ip->num_rows;
    num_cols = in_ip->num_cols;

    for (i=0; i<num_rows; i++)
    {
        in_pos = in_ip->pixels[ i ];

        for (j=0; j<num_cols; j++)
        {
            temp = in_pos->r;

            if (temp < 0.0)
            {
                temp = 0.0;
                in_pos->extra.invalid.r     |= ILLEGAL_MATH_OP_PIXEL;
                in_pos->extra.invalid.pixel |= ILLEGAL_MATH_OP_PIXEL;
            }

            in_pos->r = log(1.0 + temp);


            temp = in_pos->g;

            if (temp < 0.0)
            {
                temp = 0.0;
                in_pos->extra.invalid.g     |= ILLEGAL_MATH_OP_PIXEL;
                in_pos->extra.invalid.pixel |= ILLEGAL_MATH_OP_PIXEL;
            }

            in_pos->g = log(1.0 + temp);


            temp = in_pos->b;

            if (temp < 0.0)
            {
                temp = 0.0;
                in_pos->extra.invalid.b     |= ILLEGAL_MATH_OP_PIXEL;
                in_pos->extra.invalid.pixel |= ILLEGAL_MATH_OP_PIXEL;
            }

            in_pos->b = log(1.0 + temp);

            in_pos++;
        }
    }

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                              ow_log_brightness_image
 *
 *
 *
 * -----------------------------------------------------------------------------
*/

int ow_log_brightness_image
(
    KJB_image* in_ip,
    double     (*brightness_fn) (double, double, double),
    double     power
)
{
    Pixel* in_pos;
    double  temp_result;
    double r, g, b;
    int    i, j, num_rows, num_cols;
    double brightness;


    num_rows = in_ip->num_rows;
    num_cols = in_ip->num_cols;

    for (i=0; i<num_rows; i++)
    {
        in_pos  = in_ip->pixels[ i ];

        for (j=0; j<num_cols; j++)
        {
            r = in_pos->r;
            g = in_pos->g;
            b = in_pos->b;

            brightness = (*brightness_fn)(r, g, b);

            if (brightness > 0.0)
            {
                temp_result = log(1.0 + brightness);

                if (! IS_EQUAL_DBL(power, 1.0))
                {
                    temp_result = pow(temp_result, power);
                }

                temp_result /= brightness;

                in_pos->r *= temp_result;
                in_pos->g *= temp_result;
                in_pos->b *= temp_result;
            }

            in_pos++;
        }
    }

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                             ow_exponantiate_image
 *
 *
 *
 * -----------------------------------------------------------------------------
*/

int ow_exponantiate_image(KJB_image* in_ip)
{
    Pixel* in_pos;
    int    i, j, num_rows, num_cols;
    double d_temp;


    num_rows = in_ip->num_rows;
    num_cols = in_ip->num_cols;

    for (i=0; i<num_rows; i++)
    {
        in_pos = in_ip->pixels[ i ];

        for (j=0; j<num_cols; j++)
        {
            d_temp = in_pos->r;
            in_pos->r = exp(d_temp);

            d_temp = in_pos->g;
            in_pos->g = exp(d_temp);

            d_temp = in_pos->b;
            in_pos->b = exp(d_temp);

            in_pos++;
        }
    }

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

#ifdef __cplusplus
}
#endif

