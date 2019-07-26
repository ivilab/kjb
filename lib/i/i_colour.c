
/* $Id: i_colour.c 5831 2010-05-02 21:52:24Z ksimek $ */

/* =========================================================================== *
|
|  Copyright (c) 1994-2008 by Kobus Barnard (author).
|
|  Personal and educational use of this code is granted, provided that this
|  header is kept intact, and that the authorship is not misrepresented, that
|  its use is acknowleged in publications, and relevant papers are cited.
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
#include "i/i_colour.h"
#include "i/i_arithmetic.h"
#include "i/i_map.h"
#include "c/c_colour_space.h"


#ifdef __cplusplus
extern "C" {
#endif

/* -------------------------------------------------------------------------- */

/* =============================================================================
 *                              ow_match_brightness
 *
 *
 *
 * -----------------------------------------------------------------------------
*/

int ow_match_brightness
(
    KJB_image*       in_ip,
    const KJB_image* brightness_ip,
    double           (*brightness_fn) (double, double, double)
)
{
    Pixel* in_pos;
    Pixel* match_pos;
    double brightness_factor;
    double in_r;
    double in_g;
    double in_b;
    double in_brightness;
    double match_r;
    double match_g;
    double match_b;
    double match_brightness;
    int    i, j, num_rows, num_cols;


    num_rows = in_ip->num_rows;
    num_cols = in_ip->num_cols;

    if (    (brightness_ip->num_rows != num_rows)
         || (brightness_ip->num_cols != num_cols)
       )
    {
        set_error("Luminance image dimensions do not match input image.");
        return ERROR;
    }

    for (i=0; i<num_rows; i++)
    {
        in_pos  = in_ip->pixels[ i ];
        match_pos = brightness_ip->pixels[ i ];

        for (j=0; j<num_cols; j++)
        {
            in_r = in_pos->r;
            in_g = in_pos->g;
            in_b = in_pos->b;

            match_r = match_pos->r;
            match_g = match_pos->g;
            match_b = match_pos->b;

            in_pos->extra.invalid.r     |= match_pos->extra.invalid.r;
            in_pos->extra.invalid.g     |= match_pos->extra.invalid.g;
            in_pos->extra.invalid.b     |= match_pos->extra.invalid.b;
            in_pos->extra.invalid.pixel |= match_pos->extra.invalid.pixel;

            in_brightness = (*brightness_fn)(in_r, in_g, in_b);
            match_brightness = (*brightness_fn)(match_r, match_g, match_b);

            if ((match_brightness >= 0.0) && (in_brightness > 0.0))
            {
                brightness_factor = match_brightness / in_brightness;

                in_pos->r *= brightness_factor;
                in_pos->g *= brightness_factor;
                in_pos->b *= brightness_factor;
            }
            else
            {
                in_pos->extra.invalid.r     |= ILLEGAL_MATH_OP_PIXEL;
                in_pos->extra.invalid.g     |= ILLEGAL_MATH_OP_PIXEL;
                in_pos->extra.invalid.b     |= ILLEGAL_MATH_OP_PIXEL;
                in_pos->extra.invalid.pixel |= ILLEGAL_MATH_OP_PIXEL;

                in_pos->r = FLT_ZERO;
                in_pos->g = FLT_ZERO;
                in_pos->b = FLT_ZERO;
            }

            in_pos++;
            match_pos++;
        }
    }

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                              ow_match_chromaticity
 *
 *
 *
 * -----------------------------------------------------------------------------
*/

int ow_match_chromaticity(KJB_image* in_ip, const KJB_image* chrom_ip)
{
    Pixel* in_pos;
    Pixel* chrom_pos;
    float  in_r;
    float  in_g;
    float  in_b;
    float  in_sum;
    float  chrom_r;
    float  chrom_g;
    float  chrom_b;
    float  chrom_sum;
    int    i, j, num_rows, num_cols;


    num_rows = in_ip->num_rows;
    num_cols = in_ip->num_cols;

    if (    (chrom_ip->num_rows != num_rows)
         || (chrom_ip->num_cols != num_cols)
       )
    {
        set_error("Chromaticity image dimensions do not match input image.");
        return ERROR;
    }

    for (i=0; i<num_rows; i++)
    {
        in_pos  = in_ip->pixels[ i ];
        chrom_pos = chrom_ip->pixels[ i ];

        for (j=0; j<num_cols; j++)
        {
            in_r = in_pos->r;
            in_g = in_pos->g;
            in_b = in_pos->b;

            chrom_r = chrom_pos->r;
            chrom_g = chrom_pos->g;
            chrom_b = chrom_pos->b;

            in_pos->extra.invalid.r     |= chrom_pos->extra.invalid.r;
            in_pos->extra.invalid.g     |= chrom_pos->extra.invalid.g;
            in_pos->extra.invalid.b     |= chrom_pos->extra.invalid.b;
            in_pos->extra.invalid.pixel |= chrom_pos->extra.invalid.pixel;

            chrom_sum = chrom_r + chrom_g + chrom_b;

            if (    (chrom_r >= FLT_ZERO)
                 && (chrom_g >= FLT_ZERO)
                 && (chrom_b >= FLT_ZERO)
                 && (in_r >= FLT_ZERO)
                 && (in_g >= FLT_ZERO)
                 && (in_b >= FLT_ZERO)
                 && (chrom_sum > FLT_EPSILON)
               )
            {

                in_sum = in_r + in_g + in_b;

                in_pos->r = chrom_r * in_sum / chrom_sum;
                in_pos->g = chrom_g * in_sum / chrom_sum;
                in_pos->b = chrom_b * in_sum / chrom_sum;
            }
            else
            {
                in_pos->extra.invalid.r     |= ILLEGAL_MATH_OP_PIXEL;
                in_pos->extra.invalid.g     |= ILLEGAL_MATH_OP_PIXEL;
                in_pos->extra.invalid.b     |= ILLEGAL_MATH_OP_PIXEL;
                in_pos->extra.invalid.pixel |= ILLEGAL_MATH_OP_PIXEL;

                in_pos->r = 0.0;
                in_pos->g = 0.0;
                in_pos->b = 0.0;
            }

            in_pos++;
            chrom_pos++;
        }
    }

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

double sRGB_Y_brightness(double r, double g, double b)
{
    return 0.2126*r + 0.7152*g + 0.0722*b;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

double rgb_sum_brightness(double r, double g, double b)
{
    return r + g + b;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

double rgb_gm_brightness(double r, double g, double b)
{
    return pow(r*g*b, 1.0 / 3.0);
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                         make_chromaticity_image
 *
 *
 * Index: images, chromaticity image, image transformation
 *
 * -----------------------------------------------------------------------------
*/

int make_chromaticity_image
(
    KJB_image**      out_ipp,
    const KJB_image* in_ip,
    double           intensity
)
{
    KJB_image* out_ip;
    int        num_rows, num_cols, i, j;
    Pixel*     out_pos;
    Pixel*     in_pos;
    float      factor;
    double     sum;
    float      temp_r;
    float      temp_g;
    float      temp_b;


    UNTESTED_CODE();    /* Since adjustments to handling of divide by zero. */

    num_rows = in_ip->num_rows;
    num_cols = in_ip->num_cols;

    ERE(get_target_image(out_ipp, num_rows, num_cols));
    out_ip = *out_ipp;

    for (i=0; i<num_rows; i++)
    {
        in_pos = in_ip->pixels[ i ];
        out_pos = out_ip->pixels[ i ];

        for (j=0; j<num_cols; j++)
        {
            int invalid_pixel = FALSE;

            out_pos->extra.invalid = in_pos->extra.invalid;

            temp_r = in_pos->r;
            temp_g = in_pos->g;
            temp_b = in_pos->b;

            sum = temp_r + temp_g + temp_b;

            if (    (temp_r >= FLT_ZERO)
                 && (temp_g >= FLT_ZERO)
                 && (temp_b >= FLT_ZERO)
                 && (sum    >= FLT_TEN * FLT_MIN)
               )
            {
                factor = intensity / sum;

                out_pos->r = in_pos->r * factor;
                out_pos->g = in_pos->g * factor;
                out_pos->b = in_pos->b * factor;

                if (    (out_pos->r > FLT_HALF_MOST_POSITIVE)
                     || (out_pos->r < FLT_HALF_MOST_NEGATIVE)
                     || (out_pos->g > FLT_HALF_MOST_POSITIVE)
                     || (out_pos->g < FLT_HALF_MOST_NEGATIVE)
                     || (out_pos->b > FLT_HALF_MOST_POSITIVE)
                     || (out_pos->b < FLT_HALF_MOST_NEGATIVE)
                   )
                {
                    invalid_pixel = TRUE;
                }
            }
            else
            {
                invalid_pixel = TRUE;
            }

            if (invalid_pixel)
            {
                out_pos->extra.invalid.r     |= ILLEGAL_MATH_OP_PIXEL;
                out_pos->extra.invalid.g     |= ILLEGAL_MATH_OP_PIXEL;
                out_pos->extra.invalid.b     |= ILLEGAL_MATH_OP_PIXEL;
                out_pos->extra.invalid.pixel |= ILLEGAL_MATH_OP_PIXEL;

                out_pos->r = FLT_ZERO;
                out_pos->g = FLT_ZERO;
                out_pos->b = FLT_ZERO;
            }

            in_pos++;
            out_pos++;
        }
    }

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                         ow_make_chromaticity_image
 *
 *
 * Index: images, chromaticity image, image transformation
 *
 * -----------------------------------------------------------------------------
*/

int ow_make_chromaticity_image(KJB_image* ip, double intensity)
{
    int    num_rows, num_cols, i, j;
    Pixel* pos;
    float  factor;
    double sum;
    float  temp_r;
    float  temp_g;
    float  temp_b;


    UNTESTED_CODE();    /* Since adjustments to handling of divide by zero. */

    num_rows = ip->num_rows;
    num_cols = ip->num_cols;

    for (i=0; i<num_rows; i++)
    {
        pos = ip->pixels[ i ];

        for (j=0; j<num_cols; j++)
        {
            int invalid_pixel = FALSE;

            temp_r = pos->r;
            temp_g = pos->g;
            temp_b = pos->b;

            sum = temp_r + temp_g + temp_b;

            if (    (temp_r >= FLT_ZERO)
                 && (temp_g >= FLT_ZERO)
                 && (temp_b >= FLT_ZERO)
                 && (sum    >= FLT_TEN * FLT_MIN)
               )
            {
                factor = intensity / sum;

                pos->r *= factor;
                pos->g *= factor;
                pos->b *= factor;

                if (    (pos->r > FLT_HALF_MOST_POSITIVE)
                     || (pos->r < FLT_HALF_MOST_NEGATIVE)
                     || (pos->g > FLT_HALF_MOST_POSITIVE)
                     || (pos->g < FLT_HALF_MOST_NEGATIVE)
                     || (pos->b > FLT_HALF_MOST_POSITIVE)
                     || (pos->b < FLT_HALF_MOST_NEGATIVE)
                   )
                {
                    invalid_pixel = TRUE;
                }
            }
            else
            {
                invalid_pixel = TRUE;
            }

            if (invalid_pixel)
            {
                pos->extra.invalid.r     |= ILLEGAL_MATH_OP_PIXEL;
                pos->extra.invalid.g     |= ILLEGAL_MATH_OP_PIXEL;
                pos->extra.invalid.b     |= ILLEGAL_MATH_OP_PIXEL;
                pos->extra.invalid.pixel |= ILLEGAL_MATH_OP_PIXEL;

                pos->r = FLT_ZERO;
                pos->g = FLT_ZERO;
                pos->b = FLT_ZERO;
            }

            pos++;
        }
    }

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                         make_black_and_white_image
 *
 * Makes a black and white image from a color one
 *
 * This functiion makes  a black and white image from a color one by simply
 * averaging the three channels. If you want to create a black and white image
 * using L*a*b luminince, then convert to L*a*b. (Likely an alternative routine
 * to this one will be forthcomming at some point).
 *
 * If *out_ipp is NULL, then an image of the appropriate size is created, if it
 * is the wrong size, then it is resized, and if it is the right size, the
 * storage is recycled.
 *
 * Index: images, black and white image, image transformation
 *
 * -----------------------------------------------------------------------------
*/

int make_black_and_white_image
(
    KJB_image**      out_ipp,
    const KJB_image* in_ip
)
{
    KJB_image* out_ip;
    int        num_rows, num_cols, i, j;
    Pixel*     out_pos;
    Pixel*     in_pos;
    float      luminance;
    float      temp_r;
    float      temp_g;
    float      temp_b;


    num_rows = in_ip->num_rows;
    num_cols = in_ip->num_cols;

    ERE(get_target_image(out_ipp, num_rows, num_cols));
    out_ip = *out_ipp;

    for (i=0; i<num_rows; i++)
    {
        in_pos = in_ip->pixels[ i ];
        out_pos = out_ip->pixels[ i ];

        for (j=0; j<num_cols; j++)
        {
            out_pos->extra.invalid = in_pos->extra.invalid;

            temp_r = in_pos->r;
            temp_g = in_pos->g;
            temp_b = in_pos->b;

            luminance = (temp_r + temp_g + temp_b) / (float)3.0;;

            out_pos->r = luminance;
            out_pos->g = luminance;
            out_pos->b = luminance;

            in_pos++;
            out_pos++;
        }
    }

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                         ow_make_black_and_white_image
 *
 * Makes a black and white image from a color one
 *
 * This functiion makes  a black and white image from a color one by simply
 * averaging the three channels. The original image is overwritten. If you want
 * to create a black and white image using L*a*b luminince, then convert to
 * L*a*b. (Likely an alternative routine to this one will be forthcomming at
 * some point).
 *
 * Index: images, black and white image, image transformation
 *
 * -----------------------------------------------------------------------------
*/

int ow_make_black_and_white_image(KJB_image* ip)
{
    int    num_rows, num_cols, i, j;
    Pixel* pos;
    float  luminance;


    num_rows = ip->num_rows;
    num_cols = ip->num_cols;

    for (i=0; i<num_rows; i++)
    {
        pos = ip->pixels[ i ];

        for (j=0; j<num_cols; j++)
        {
            luminance = (pos->r + pos->g + pos->b) / (float)3.0;;

            pos->r = luminance;
            pos->g = luminance;
            pos->b = luminance;

            pos++;
        }
    }

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                             convert_image_rgb_to_lab
 *
 * Converts image RGB to L*a*b.
 *
 * This routine converts image RGB to L*a*b. This conversion uses an RGB to XYZ
 * matrix. If KJB library options are being made available to the user, then
 * this matrix can be set using the option "rgb-to-xyz-file".  If no conversion
 * file has been set, than a default one is used. If no default is available,
 * then sRGB is used. The contents of the third argument, white_rgb_vp, is used
 * as the RGB of the white point. If it is NULL, then RGB=(255,255,255) is used.
 *
 * The output is put into *out_ipp. If *out_ipp is NULL, then it is
 * created. If it is the wrong size, then it is resized. If it is the right
 * size, the storage is recycled.
 *
 * Returns:
 *    NO_ERROR on success and ERROR on failure, with an error message being set.
 *
 * Index: images, RGB colour, L*a*b colour, colour spaces
 *
 * -----------------------------------------------------------------------------
*/

int convert_image_rgb_to_lab
(
    KJB_image**      out_ipp,
    const KJB_image* in_ip,
    const Vector*    white_rgb_vp
)
{
    Vector* reciprocal_of_white_xyz_vp = NULL;
    int     num_rows, num_cols, i, j;


    ERE(get_reciprocal_of_white_xyz(&reciprocal_of_white_xyz_vp,
                                    white_rgb_vp));

    ERE(convert_image_rgb_to_xyz(out_ipp, in_ip));

    /*
    // Make image of ( X/Xn, Y/Yn, Z/Zn ).
    */
    ERE(ow_scale_image_by_channel(*out_ipp, reciprocal_of_white_xyz_vp));
    free_vector(reciprocal_of_white_xyz_vp);

    num_rows = (*out_ipp)->num_rows;
    num_cols = (*out_ipp)->num_cols;

    for (i=0; i<num_rows; i++)
    {
        for (j=0; j<num_cols; j++)
        {
            double f_x = LAB_F((double)(*out_ipp)->pixels[ i ][ j ].r);
            double f_y = LAB_F((double)(*out_ipp)->pixels[ i ][ j ].g);
            double f_z = LAB_F((double)(*out_ipp)->pixels[ i ][ j ].b);

            (*out_ipp)->pixels[ i ][ j ].r = 116.0 * f_y - 16.0;
            (*out_ipp)->pixels[ i ][ j ].g = 500 * (f_x - f_y);
            (*out_ipp)->pixels[ i ][ j ].b = 200 * (f_y - f_z);
        }
    }

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                        convert_image_rgb_to_xyz
 *
 * Converts image RGB to XYZ
 *
 * This routine converts the image RGB to XYZ based on an RGB to XYZ matrix. If
 * KJB library options are being made available to the user, then this matrix
 * can be set using the option "rgb-to-xyz-file".  If no conversion file has
 * been set, than a default one is used. If no default is available, then sRGB
 * is used.
 *
 * The output is put into *out_ipp. If *out_ipp is NULL, then it is
 * created. If it is the wrong size, then it is resized. If it is the right
 * size, the storage is recycled.
 *
 * Returns:
 *    NO_ERROR on success and ERROR on failure, with an error message being set.
 *
 * Index: images, RGB colour, XYZ colour, colour spaces
 *
 * -----------------------------------------------------------------------------
*/

int convert_image_rgb_to_xyz(KJB_image** out_ipp, const KJB_image* in_ip)
{
    int     result;
    Matrix* rgb_to_xyz_mp = NULL;


    UNTESTED_CODE();

    ERE(get_rgb_to_xyz_matrix(&rgb_to_xyz_mp));

    result = pre_map_image(out_ipp, in_ip, rgb_to_xyz_mp);

    free_matrix(rgb_to_xyz_mp);

    return result;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */



#ifdef __cplusplus
}
#endif

