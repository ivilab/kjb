
/* $Id: i_valid.c 7206 2010-10-27 06:49:06Z kobus $ */

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

#include "i/i_set_aux.h"
#include "i/i_valid.h"

#ifdef __cplusplus
extern "C" {
#endif

/* -------------------------------------------------------------------------- */

/* Kobus, 09-09-11:
 *    Make it so the default is not to clip, so that all operations in
 *    kjb_read_image() are "opt in". The easiest way to do so is to set the clip
 *    very high. 
 *
static float fs_image_clip_point      = 254.5;
*/
static float fs_image_clip_point      = FLT_MAX; 
static float fs_min_valid_red_value   = 0.0;
static float fs_min_valid_green_value = 0.0;
static float fs_min_valid_blue_value  = 0.0;
static float fs_min_valid_sum_value   = 0.0;

/* -------------------------------------------------------------------------- */

int set_image_validity_options(const char* option, const char* value)
{
    char   lc_option[ 100 ];
    double   temp_real;
    int    result             = NOT_FOUND;


    EXTENDED_LC_BUFF_CPY(lc_option, option);

    if (    (lc_option[ 0 ] == '\0')
         || (match_pattern(lc_option, "clip-point"))
         || (match_pattern(lc_option, "image-clip-point"))
       )
    {
        if (value == NULL)
        {
            return NO_ERROR;
        }
        else if (value[ 0 ] == '?')
        {
            ERE(pso("clip-point = %.1f\n", fs_image_clip_point));
        }
        else if (value[ 0 ] == '\0')
        {
            ERE(pso("Images are clipped at %.1f\n", fs_image_clip_point));
        }
        else
        {
            ERE(ss1d(value, &temp_real));
            fs_image_clip_point = temp_real;
        }
        result = NO_ERROR;
    }

    if (    (lc_option[ 0 ] == '\0')
         || (match_pattern(lc_option, "mrv"))
         || (match_pattern(lc_option, "min-red-value"))
       )
    {
        if (value == NULL)
        {
            return NO_ERROR;
        }
        else if (value[ 0 ] == '?')
        {
            ERE(pso("min-red-value = %4.1f\n", fs_min_valid_red_value));
        }
        else if (value[ 0 ] == '\0')
        {
            ERE(pso("Minimum image red value (mrv) is %4.1f.\n",
                    fs_min_valid_red_value));
        }
        else
        {
            ERE(ss1d(value, &temp_real));
            fs_min_valid_red_value = temp_real;
        }
        result = NO_ERROR;
    }

    if (    (lc_option[ 0 ] == '\0')
         || (match_pattern(lc_option, "mgv"))
         || (match_pattern(lc_option, "min-green-value"))
       )
    {
        if (value == NULL)
        {
            return NO_ERROR;
        }
        else if (value[ 0 ] == '?')
        {
            ERE(pso("min-green-value = %4.1f\n", fs_min_valid_green_value));
        }
        else if (value[ 0 ] == '\0')
        {
            ERE(pso("Minimum image green value (mgv) is %4.1f.\n",
                    fs_min_valid_green_value));
        }
        else
        {
            ERE(ss1d(value, &temp_real));
            fs_min_valid_green_value = temp_real;
        }
        result = NO_ERROR;
    }

    if (    (lc_option[ 0 ] == '\0')
         || (match_pattern(lc_option, "mbv"))
         || (match_pattern(lc_option, "min-blue-value"))
       )
    {
        if (value == NULL)
        {
            return NO_ERROR;
        }
        else if (value[ 0 ] == '?')
        {
            ERE(pso("min-blue-value = %4.1f\n", fs_min_valid_blue_value));
        }
        else if (value[ 0 ] == '\0')
        {
            ERE(pso("Minimum image blue value (mbv) is %4.1f.\n",
                    fs_min_valid_blue_value));
        }
        else
        {
            ERE(ss1d(value, &temp_real));
            fs_min_valid_blue_value = temp_real;
        }
        result = NO_ERROR;
    }

    if (    (lc_option[ 0 ] == '\0')
         || (match_pattern(lc_option, "msv"))
         || (match_pattern(lc_option, "min-sum-value"))
       )
    {
        if (value == NULL)
        {
            return NO_ERROR;
        }
        else if (value[ 0 ] == '?')
        {
            ERE(pso("min-sum-value = %4.1f\n", fs_min_valid_sum_value));
        }
        else if (value[ 0 ] == '\0')
        {
            ERE(pso("Minimum image sum value (msv) is %4.1f.\n",
                    fs_min_valid_sum_value));
        }
        else
        {
            ERE(ss1d(value, &temp_real));
            fs_min_valid_sum_value = temp_real;
        }
        result = NO_ERROR;
    }

    if ((result == NO_ERROR) && (value[ 0 ] != '\0'))
    {
        call_image_data_invalidation_fn();
    }

    return result;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                         mark_clipped_pixels
 *
 *
 *
 * Returns:
 *     NO_ERROR on sucess and ERROR on failure.
 *
 * Index: images, float images, image pixel validity
 *
 * -----------------------------------------------------------------------------
*/

int mark_clipped_pixels(KJB_image* ip)
{
    int    i, j, num_rows, num_cols;
    Pixel* pos;
    int    count    = 0;
    int    pixel_is_clipped;


    /* Kobus, 09-09-11:
     *    Since the default is now to not mark clipped pixels, we gamble on an
     *    early return. 
    */
    if (fs_image_clip_point >= DBL_HALF_MOST_POSITIVE) 
    {
        verbose_pso(5, "Fast return from mark_clipped_pixels() because the threshold is very high.\n"); 
        return NO_ERROR;
    }

    num_rows = ip->num_rows;
    num_cols = ip->num_cols;

    for(i=0; i<num_rows; i++)
    {
        pos = ip->pixels[ i ];

        for(j=0; j<num_cols; j++)
        {
            pixel_is_clipped = FALSE;

            if (pos->r > fs_image_clip_point)
            {
                pos->extra.invalid.r |= CLIPPED_PIXEL;
                pixel_is_clipped = TRUE;
            }
            else
            {
                pos->extra.invalid.r &= ~CLIPPED_PIXEL;
            }

            if (pos->g > fs_image_clip_point)
            {
                pos->extra.invalid.g |= CLIPPED_PIXEL;
                pixel_is_clipped = TRUE;
            }
            else
            {
                pos->extra.invalid.g &= ~CLIPPED_PIXEL;
            }

            if (pos->b > fs_image_clip_point)
            {
                pos->extra.invalid.b |= CLIPPED_PIXEL;
                pixel_is_clipped = TRUE;
            }
            else
            {
                pos->extra.invalid.b &= ~CLIPPED_PIXEL;
            }

            pos->extra.invalid.pixel =
                              pos->extra.invalid.r | pos->extra.invalid.g | pos->extra.invalid.b;

            if (pixel_is_clipped)
            {
                count++;
            }
            pos++;
        }
    }

    verbose_pso(5, "%d pixels marked as clipped.\n", count);

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                         mark_dark_pixels
 *
 *
 *
 *
 * Returns:
 *     NO_ERROR on sucess and ERROR on failure.
 *
 * Index: images, float images, image pixel validity
 *
 * -----------------------------------------------------------------------------
*/

int mark_dark_pixels(KJB_image* ip)
{
    int    i, j, num_rows, num_cols;
    Pixel* pos;
    int    count    = 0;
    int    pixel_is_dark;


    num_rows = ip->num_rows;
    num_cols = ip->num_cols;

    for(i=0; i<num_rows; i++)
    {
        pos = ip->pixels[ i ];

        for(j=0; j<num_cols; j++)
        {
            pixel_is_dark = FALSE;

            if (pos->r < fs_min_valid_red_value)
            {
                pixel_is_dark = TRUE;
                pos->extra.invalid.r |= DARK_PIXEL;
            }
            else
            {
                pos->extra.invalid.r &= ~DARK_PIXEL;
            }

            if (pos->g < fs_min_valid_green_value)
            {
                pixel_is_dark = TRUE;
                pos->extra.invalid.g |= DARK_PIXEL;
            }
            else
            {
                pos->extra.invalid.g &= ~DARK_PIXEL;
            }

            if (pos->b < fs_min_valid_blue_value)
            {
                pixel_is_dark = TRUE;
                pos->extra.invalid.b |= DARK_PIXEL;
            }
            else
            {
                pos->extra.invalid.b &= ~DARK_PIXEL;
            }

            if (pos->r + pos->g + pos->b < fs_min_valid_sum_value)
            {
                pixel_is_dark = TRUE;
                pos->extra.invalid.r |= DARK_PIXEL;
                pos->extra.invalid.g |= DARK_PIXEL;
                pos->extra.invalid.b |= DARK_PIXEL;
            }

            pos->extra.invalid.pixel =
                              pos->extra.invalid.r | pos->extra.invalid.g | pos->extra.invalid.b;

            if (pixel_is_dark)
            {
                count++;
            }

            pos++;
        }
    }

    verbose_pso(5, "%d pixels marked as dark.\n", count);

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                         unmark_dark_pixels
 *
 *
 *
 * Returns:
 *     NO_ERROR on sucess and ERROR on failure.
 *
 * Index: images, float images, image pixel validity
 *
 * -----------------------------------------------------------------------------
*/

int unmark_dark_pixels(KJB_image* ip)
{
    int    i, j, num_rows, num_cols;
    Pixel* pos;


    num_rows = ip->num_rows;
    num_cols = ip->num_cols;

    for(i=0; i<num_rows; i++)
    {
        pos = ip->pixels[ i ];

        for(j=0; j<num_cols; j++)
        {
            pos->extra.invalid.r &= ~DARK_PIXEL;
            pos->extra.invalid.g &= ~DARK_PIXEL;
            pos->extra.invalid.b &= ~DARK_PIXEL;

            pos->extra.invalid.pixel =
                              pos->extra.invalid.r | pos->extra.invalid.g | pos->extra.invalid.b;

            pos++;
        }
    }

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                            mark_blooming_candidates
 *
 *
 *
 * -----------------------------------------------------------------------------
*/


int mark_blooming_candidates(KJB_image** out_ipp, const KJB_image* in_ip)
{
    KJB_image* out_ip;
    Pixel* out_pos;
    int    i, j, num_rows, num_cols;


    UNTESTED_CODE();   /* Untesed since change to validity. */

    ERE(kjb_copy_image(out_ipp, in_ip));

    num_rows = in_ip->num_rows;
    num_cols = in_ip->num_cols;

    out_ip = *out_ipp;

    for (i=0; i<num_rows; i++)
    {
        for (j=0; j<num_cols; j++)
        {
            if (in_ip->pixels[ i ][ j ].extra.invalid.r & CLIPPED_PIXEL)
            {
                if (i > 0)
                {
                    if (j > 0)
                    {
                        out_ip->pixels[ i-1 ][ j-1 ].extra.invalid.r |= CLIPPED_PIXEL;
                    }
                    out_ip->pixels[ i-1 ][ j ].extra.invalid.r |= CLIPPED_PIXEL;

                    if (j < num_cols-1)
                    {
                        out_ip->pixels[ i-1 ][ j+1 ].extra.invalid.r |= CLIPPED_PIXEL;
                    }
                }

                if (j > 0)
                {
                    out_ip->pixels[ i ][ j-1 ].extra.invalid.r |= CLIPPED_PIXEL;
                }

                if (j < num_cols-1)
                {
                    out_ip->pixels[ i ][ j+1 ].extra.invalid.r |= CLIPPED_PIXEL;
                }

                if (i < num_rows-1)
                {
                    if (j > 0)
                    {
                        out_ip->pixels[ i+1 ][ j-1 ].extra.invalid.r |= CLIPPED_PIXEL;
                    }

                    out_ip->pixels[ i+1 ][ j ].extra.invalid.r |= CLIPPED_PIXEL;

                    if (j < num_cols-1)
                    {
                        out_ip->pixels[ i+1 ][ j+1 ].extra.invalid.r |= CLIPPED_PIXEL;
                    }
                }
            }

            if (in_ip->pixels[ i ][ j ].extra.invalid.g & CLIPPED_PIXEL)
            {
                if (i > 0)
                {
                    if (j > 0)
                    {
                        out_ip->pixels[ i-1 ][ j-1 ].extra.invalid.g |= CLIPPED_PIXEL;
                    }
                    out_ip->pixels[ i-1 ][ j ].extra.invalid.g |= CLIPPED_PIXEL;

                    if (j < num_cols-1)
                    {
                        out_ip->pixels[ i-1 ][ j+1 ].extra.invalid.g |= CLIPPED_PIXEL;
                    }
                }

                if (j > 0)
                {
                    out_ip->pixels[ i ][ j-1 ].extra.invalid.g |= CLIPPED_PIXEL;
                }

                if (j < num_cols-1)
                {
                    out_ip->pixels[ i ][ j+1 ].extra.invalid.g |= CLIPPED_PIXEL;
                }

                if (i < num_rows-1)
                {
                    if (j > 0)
                    {
                        out_ip->pixels[ i+1 ][ j-1 ].extra.invalid.g |= CLIPPED_PIXEL;
                    }

                    out_ip->pixels[ i+1 ][ j ].extra.invalid.g |= CLIPPED_PIXEL;

                    if (j < num_cols-1)
                    {
                        out_ip->pixels[ i+1 ][ j+1 ].extra.invalid.g |= CLIPPED_PIXEL;
                    }
                }
            }

            if (in_ip->pixels[ i ][ j ].extra.invalid.b & CLIPPED_PIXEL)
            {
                if (i > 0)
                {
                    if (j > 0)
                    {
                        out_ip->pixels[ i-1 ][ j-1 ].extra.invalid.b |= CLIPPED_PIXEL;
                    }
                    out_ip->pixels[ i-1 ][ j ].extra.invalid.b |= CLIPPED_PIXEL;

                    if (j < num_cols-1)
                    {
                        out_ip->pixels[ i-1 ][ j+1 ].extra.invalid.b |= CLIPPED_PIXEL;
                    }
                }

                if (j > 0)
                {
                    out_ip->pixels[ i ][ j-1 ].extra.invalid.b |= CLIPPED_PIXEL;
                }

                if (j < num_cols-1)
                {
                    out_ip->pixels[ i ][ j+1 ].extra.invalid.b |= CLIPPED_PIXEL;
                }

                if (i < num_rows-1)
                {
                    if (j > 0)
                    {
                        out_ip->pixels[ i+1 ][ j-1 ].extra.invalid.b |= CLIPPED_PIXEL;
                    }

                    out_ip->pixels[ i+1 ][ j ].extra.invalid.b |= CLIPPED_PIXEL;

                    if (j < num_cols-1)
                    {
                        out_ip->pixels[ i+1 ][ j+1 ].extra.invalid.b |= CLIPPED_PIXEL;
                    }
                }
            }
        }
    }

    out_ip = *out_ipp;

    for (i=0; i<num_rows; i++)
    {
        out_pos = out_ip->pixels[ i ];

        for (j=0; j<num_cols; j++)
        {
            out_pos->extra.invalid.pixel =
                   out_pos->extra.invalid.r | out_pos->extra.invalid.g | out_pos->extra.invalid.b;
            out_pos++;
        }
    }

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                         mark_pixels_above_threshold
 *
 *
 *
 * -----------------------------------------------------------------------------
*/

int mark_pixels_above_threshold(KJB_image* ip, double max)
{
    int    i, j, num_rows, num_cols;
    Pixel* pos;
    float  f_max       = max;
    unsigned char  new_invalid;
    int    invalid_count = 0;


    num_rows = ip->num_rows;
    num_cols = ip->num_cols;

    for(i=0; i<num_rows; i++)
    {
        pos = ip->pixels[ i ];

        for(j=0; j<num_cols; j++)
        {
            if (pos->r > f_max)
            {
                pos->extra.invalid.r |= CLIPPED_PIXEL;
            }

            if (pos->g > f_max)
            {
                pos->extra.invalid.g |= CLIPPED_PIXEL;
            }

            if (pos->b > f_max)
            {
                pos->extra.invalid.b |= CLIPPED_PIXEL;
            }

            new_invalid = pos->extra.invalid.r | pos->extra.invalid.g | pos->extra.invalid.b;

            if (new_invalid && ! pos->extra.invalid.pixel)
            {
                invalid_count++;
            }

            pos->extra.invalid.pixel = new_invalid;

            pos++;
        }
    }

    verbose_pso(3,
                "%d image pixels marked as invalid because they exceed %.1f.\n",
                invalid_count, max);

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                         mark_pixels_below_threshold
 *
 *
 *
 * -----------------------------------------------------------------------------
*/

int mark_pixels_below_threshold(KJB_image* ip, double min)
{
    int    i, j, num_rows, num_cols;
    Pixel* pos;
    float  f_min       = min;
    unsigned char  new_invalid;
    int    invalid_count = 0;


    num_rows = ip->num_rows;
    num_cols = ip->num_cols;

    for(i=0; i<num_rows; i++)
    {
        pos = ip->pixels[ i ];

        for(j=0; j<num_cols; j++)
        {
            if (pos->r < f_min)
            {
                pos->extra.invalid.r |= DARK_PIXEL;
            }

            if (pos->g < f_min)
            {
                pos->extra.invalid.g |= DARK_PIXEL;
            }

            if (pos->b < f_min)
            {
                pos->extra.invalid.b |= DARK_PIXEL;
            }

            new_invalid = pos->extra.invalid.r | pos->extra.invalid.g | pos->extra.invalid.b;

            if (new_invalid && ! pos->extra.invalid.pixel)
            {
                invalid_count++;
            }

            pos->extra.invalid.pixel = new_invalid;

            pos++;
        }
    }

    verbose_pso(3,
           "%d image pixels marked as invalid because they were below %.1f.\n",
            invalid_count, min);

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                         count_invalid_pixels
 *
 *
 *
 * Returns:
 *     The number of invalid pixels in an image
 *
 * Index: images, float images, image pixel validity
 *
 * -----------------------------------------------------------------------------
*/

int count_invalid_pixels(const KJB_image* ip)
{
    int    i, j, num_rows, num_cols;
    Pixel* pos;
    int    invalid_count = 0;


    num_rows = ip->num_rows;
    num_cols = ip->num_cols;

    for(i=0; i<num_rows; i++)
    {
        pos = ip->pixels[ i ];

        for(j=0; j<num_cols; j++)
        {
            if (pos->extra.invalid.pixel)
            {
                invalid_count++;
            }

            pos++;
        }
    }

    return invalid_count;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

#ifdef __cplusplus
}
#endif

