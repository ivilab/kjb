
/* $Id: i_stat.c 21744 2017-09-03 08:15:30Z jiachengz $ */

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
#include "i/i_stat.h"

#ifdef __cplusplus
extern "C" {
#endif

/* -------------------------------------------------------------------------- */

/* =============================================================================
 *                                get_ave_rgb
 *
 * Computes the average of R, G, and B of an image
 *
 * This routine computes the overall average of R, G, and B of an image. The
 * result is put into *out_rgb_vpp which is created if need be.
 *
 * Returns:
 *     NO_ERROR on sucess and ERROR on failure.
 *
 * Index: images, image arithmetic
 *
 * -----------------------------------------------------------------------------
*/

int get_ave_rgb(Vector** out_rgb_vpp, const KJB_image* in_ip)
{
    Vector* out_rgb_vp;
    int     num_rows, num_cols;
    Pixel*  in_pos;
    int     i, j;
    double  sum_r;
    double  sum_g;
    double  sum_b;
    int     count_r    = 0;
    int     count_g    = 0;
    int     count_b    = 0;


    ERE(get_target_vector(out_rgb_vpp, 3));
    out_rgb_vp = *out_rgb_vpp;

    num_rows = in_ip->num_rows;
    num_cols = in_ip->num_cols;

    sum_r = 0.0;
    sum_g = 0.0;
    sum_b = 0.0;

    for (i=0; i<num_rows; i++)
    {
        in_pos = in_ip->pixels[ i ];

        for (j=0; j<num_cols; j++)
        {
            if ( ! in_pos->extra.invalid.r)
            {
                sum_r += in_pos->r;
                count_r++;
            }

            if ( ! in_pos->extra.invalid.g)
            {
                sum_g += in_pos->g;
                count_g++;
            }

            if ( ! in_pos->extra.invalid.b)
            {
                sum_b += in_pos->b;
                count_b++;
            }

            in_pos++;
        }
    }

    if ((count_r == 0) || (count_g == 0) || (count_b == 0))
    {
        set_error("Not enough valid pixels for image operation (RGB average).");
        add_error("A positive number of valid pixels is required ");
        cat_error("for R, G, and B ");
        add_error("The counts were %d for red, %d for green, and %d for blue.",
                  count_r, count_g, count_b);
        return ERROR;
    }

    out_rgb_vp->elements[ 0 ] = sum_r / (double)count_r;
    out_rgb_vp->elements[ 1 ] = sum_g / (double)count_g;
    out_rgb_vp->elements[ 2 ] = sum_b / (double)count_b;

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                                get_max_rgb
 *
 * Computes the overall max of R, G, and B of an image
 *
 * This routine computes the overall max of R, G, and B of an image. The result
 * is put into *out_rgb_vpp which is created if need be.
 *
 * Returns:
 *     NO_ERROR on sucess and ERROR on failure.
 *
 * Index: images, image arithmetic
 *
 * -----------------------------------------------------------------------------
*/

int get_max_rgb(Vector** out_rgb_vpp, const KJB_image* in_ip)
{
    Vector* out_rgb_vp;
    int     num_rows, num_cols;
    Pixel*  in_pos;
    int     i, j;
    float   max_r;
    float   max_g;
    float   max_b;


    ERE(get_target_vector(out_rgb_vpp, 3));
    out_rgb_vp = *out_rgb_vpp;

    num_rows = in_ip->num_rows;
    num_cols = in_ip->num_cols;

    max_r = -FLT_MAX;
    max_g = -FLT_MAX;
    max_b = -FLT_MAX;

    for (i=0; i<num_rows; i++)
    {
        in_pos = in_ip->pixels[ i ];

        for (j=0; j<num_cols; j++)
        {
            if (in_pos->r > max_r) max_r = in_pos->r;
            if (in_pos->g > max_g) max_g = in_pos->g;
            if (in_pos->b > max_b) max_b = in_pos->b;
            in_pos++;
        }
    }

    if (    (max_r <= -FLT_MAX/2)
         || (max_g <= -FLT_MAX/2)
         || (max_b <= -FLT_MAX/2)
       )
    {
        set_error("Not enough valid pixels for image operation ");
        cat_error("(RGB maximums).");

        if (max_r < -FLT_MAX/2)
        {
            add_error("No pixels with valid red value found.");
        }
        else if (max_g < -FLT_MAX/2)
        {
            add_error("No pixels with valid green value found.");
        }
        else if (max_b < -FLT_MAX/2)
        {
            add_error("No pixels with valid blue value found.");
        }

        return ERROR;
    }

    out_rgb_vp->elements[ 0 ] = max_r;
    out_rgb_vp->elements[ 1 ] = max_g;
    out_rgb_vp->elements[ 2 ] = max_b;

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*
 * =============================================================================
 *                             get_image_stats
 *
 * Computes some image statistics
 *
 * This routine computes some image statistics. The number of valid pixes, the
 * image mean, and the image standard deviation are put into
 * *num_valid_pixels_ptr, *mean_vpp, and *stdev_vpp, respectively. If
 * *mean_vpp or *stdev_vpp are NULL, or are the wrong size, then they are
 * created or resizes as need be. Any of the three output arguments may be set
 * to NULL if you are not interested in that particular statistic. Statistics
 * are only computed over valid pixels, and if there are not enough valid pixels
 * to compute a requested statistic, then ERROR is returned.
 *
 * Returns:
 *     NO_ERROR on success, and ERROR on failure, with an appropriate error
 *     message being set.
 *
 * Index: images, image stats
 *
 * -----------------------------------------------------------------------------
 */

int get_image_stats
(
    int*             num_valid_pixels_ptr,
    Vector**         mean_vpp,
    Vector**         stdev_vpp,
    const KJB_image* source_ip
)
{


    return get_image_window_stats(num_valid_pixels_ptr,
                                  mean_vpp, stdev_vpp, source_ip, 0, 0,
                                  source_ip->num_rows, source_ip->num_cols);
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*
 * =============================================================================
 *                             get_image_window_stats
 *
 * Computes some statistics over an image window
 *
 * This routine computes some statistics over an image window. The number of
 * valid pixes, the image mean, and the image standard deviation are put into
 * *num_valid_pixels_ptr, *mean_vpp, and *stdev_vpp, respectively. If
 * *mean_vpp or *stdev_vpp are NULL, or are the wrong size, then they are
 * created or resizes as need be. Any of the three output arguments may be set
 * to NULL if you are not interested in that particular statistic. Statistics
 * are only computed over valid pixels, and if there are not enough valid pixels
 * to compute a requested statistic, then ERROR is returned, with an appropriate
 * error message being set.
 *
 * The image window is specified by its minium row and minimum column (arguments
 * "row_offset" and "col_offset" respectively), and its dimensions
 * ("num_target_rows" and "num_target_cols"). If the window exceeds the image
 * boundaries, then EROR is returned, with an appropriate error message being
 * set.
 *
 * Returns:
 *     NO_ERROR on success, and ERROR on failure, with an appropriate error
 *     message being set.
 *
 * Index: images, image stats
 *
 * -----------------------------------------------------------------------------
 */

int get_image_window_stats
(
    int*             num_valid_pixels_ptr,
    Vector**         mean_vpp,
    Vector**         stdev_vpp,
    const KJB_image* source_ip,
    int              row_offset,
    int              col_offset,
    int              num_target_rows,
    int              num_target_cols
)
{
    Vector* mean_vp;
    Vector* stdev_vp;
    int     num_source_rows;
    int     num_source_cols;
    int     i, j;
    Pixel*  source_pos;
    float   r, g, b;
    double  rgb_sum[ 3 ];
    double  rgb_square_sum[ 3 ];
    double  temp;
    int     counts[ 3 ];
    int     num_valid_pixels = 0;


    if (mean_vpp != NULL)
    {
        ERE(get_target_vector(mean_vpp, 3));
        mean_vp = *mean_vpp;
    }
    else
    {
        mean_vp = NULL;
    }

    if (stdev_vpp != NULL)
    {
        ERE(get_target_vector(stdev_vpp, 3));
        stdev_vp = *stdev_vpp;
    }
    else
    {
        stdev_vp = NULL;
    }

    num_source_rows = source_ip->num_rows;
    num_source_cols = source_ip->num_cols;

    if (    (num_source_rows < row_offset + num_target_rows)
         || (num_source_cols < row_offset + num_target_cols)
       )
    {
        SET_ARGUMENT_BUG();
        return ERROR;
    }

    /* ini */
    for (i=0; i<3; i++)
    {
        rgb_sum[ i ] = 0.0;
        rgb_square_sum[ i ] = 0.0;
        counts[ i ] = 0;
    }

    /* collect sum of rgb from valid pixels */
    for (i=0; i<num_target_rows; i++)
    {
        source_pos = source_ip->pixels[ row_offset + i ];
        source_pos += col_offset;

        for (j=0; j<num_target_cols; j++)
        {
            if ( ! source_pos->extra.invalid.r)
            {
                r = source_pos->r;
                rgb_sum[ 0 ] += r;  /* Ignore lint */
                rgb_square_sum[ 0 ] += r*r; /* Ignore lint */
                counts[ 0 ]++;
            }

            if ( ! source_pos->extra.invalid.g)
            {
                g = source_pos->g;
                rgb_sum[ 1 ] += g;  /* Ignore lint */
                rgb_square_sum[ 1 ] += g*g;
                counts[ 1 ]++;
            }

            if ( ! source_pos->extra.invalid.b)
            {
                b = source_pos->b;
                rgb_sum[ 2 ] += b;  /* Ignore lint */

                rgb_square_sum[ 2 ] += b*b;
                counts[ 2 ]++;
            }

            if ( ! source_pos->extra.invalid.pixel)
            {
                num_valid_pixels++;
            }

            source_pos++;
        }
    }

    if (mean_vp != NULL)
    {
        for (i=0; i<3; i++)
        {
            if (counts[ i ] == 0)
            {
                set_error("Insufficient number of valid points for average.");
                add_error("No valid points in channel %d found.", i + 1);
                return ERROR;
            }
            mean_vp->elements[ i ] = rgb_sum[ i ] / counts[ i ];
        }
    }

    if (stdev_vp != NULL)
    {
        for (i=0; i<3; i++)
        {
            if (counts[ i ] <= 1)
            {
                set_error("Insufficient number of valid points for ");
                cat_error("standard deviation.");
                add_error("%s in channel %d found.",
                          (counts[ i ] == 0)
                              ? "No valid points"
                              : "Only one valid point",
                          i + 1);
                return ERROR;
            }

            temp = rgb_square_sum[ i ];
            temp -= (rgb_sum[ i ] * rgb_sum[ i ]) / counts[ i ];
            temp /= (counts[ i ] - 1);

            stdev_vp->elements[ i ] = sqrt(temp);
        }
    }

    if (num_valid_pixels_ptr != NULL)
    {
        *num_valid_pixels_ptr = num_valid_pixels;
    }

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                           get_ave_ratio_without_invalid
 *
 * Calculate the average of ratio of r/g/b values between two same sized image.
 *
 * You can provide the threshold of min r/g/b to be included in calculaton.
 * Ratio of R/G/B values of pixels with the same coordinates will be included  
 * in calculation if and only if:
 *      1. both R/G/B values are valid
 *      2. both R/G/B values are larger or equal to the given thresholder
 * 
 * If at least one of average of ratio of R/G/B values has sample size which 
 * smaller than min_num_good_points, an error will be reported.
 *
 * Returns:
 *     NO_ERROR on success, and ERROR on failure, with an appropriate error
 *     message being set.
 *
 * Index: images, image stats
 * -----------------------------------------------------------------------------
*/

int get_ave_ratio_without_invalid
(
    Vector**         out_vpp,
    const KJB_image* in1_ip,
    const KJB_image* in2_ip,
    double           threshold,
    int              min_num_good_points
)
{
    Pixel* in1_pos;
    Pixel* in2_pos;
    int    i, j, num_rows, num_cols;
    double sum_r    = 0.0;
    double sum_g    = 0.0;
    double sum_b    = 0.0;
    int    count_r  = 0;
    int    count_g  = 0;
    int    count_b  = 0;


    ERE(check_same_size_image(in1_ip, in2_ip));

    if (threshold < 0.0) threshold = 0.0;

    if (min_num_good_points < 1) min_num_good_points = 1;

    num_rows = in1_ip->num_rows;
    num_cols = in1_ip->num_cols;

    ERE(get_target_vector(out_vpp, 3));

    for (i=0; i<num_rows; i++)
    {
        in1_pos  = in1_ip->pixels[ i ];
        in2_pos  = in2_ip->pixels[ i ];

        for (j=0; j<num_cols; j++)
        {
            if (    (in1_pos->r > threshold) && (in2_pos->r > threshold)
                 && ( ! in1_pos->extra.invalid.r ) && ( ! in2_pos->extra.invalid.r)
               )
            {
                sum_r += (in1_pos->r / in2_pos->r);
                count_r++;
            }

            if (    (in1_pos->g > threshold) && (in2_pos->g > threshold)
                 && ( ! in1_pos->extra.invalid.g ) && ( ! in2_pos->extra.invalid.g)
               )
            {
                sum_g += (in1_pos->g / in2_pos->g);
                count_g++;
            }

            if (    (in1_pos->b > threshold) && (in2_pos->b > threshold)
                 && ( ! in1_pos->extra.invalid.b ) && ( ! in2_pos->extra.invalid.b)
               )
            {
                sum_b += (in1_pos->b / in2_pos->b);
                count_b++;
            }

            in1_pos++;
            in2_pos++;
        }
    }

    if (   (count_r < min_num_good_points)
        || (count_g < min_num_good_points)
        || (count_b < min_num_good_points)
       )
    {
        set_error("Not enough valid corresponding pixels for image ratio.");
        return ERROR;
    }
    else
    {
        ERE(verbose_pso(1, "Number of points for red channel ratio:   %d.\n",
                        count_r));
        ERE(verbose_pso(1, "Number of points for green channel ratio: %d.\n",
                        count_g));
        ERE(verbose_pso(1, "Number of points for blue channel ratio:  %d.\n",
                        count_b));

        (*out_vpp)->elements[ RED_INDEX   ] = sum_r / count_r;
        (*out_vpp)->elements[ GREEN_INDEX ] = sum_g / count_g;
        (*out_vpp)->elements[ BLUE_INDEX  ] = sum_b / count_b;
        return NO_ERROR;
    }
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                           get_ave_sum_ratio_without_invalid
 *
 * Calculate the average of ratio of sum of RGB values of pixels with the same
 * coordinate. 
 *
 * Two images need to be the same sized.
 * You can provide the threshold of min r/g/b to be included in calculaton.
 * The sum of R/G/B values of pixels with the same coordinates will be included  
 * in calculation if and only if:
 *      1. All the R/G/B values of both pixels are valid
 *      2. All the R/G/B values of both pixels are larger or equal to the 
 *          given thresholder.
 *
 * An error will be reported if sample size of the average value is smaller 
 * than min_num_good_points.
 *
 * Returns:
 *     NO_ERROR on success, and ERROR on failure, with an appropriate error
 *     message being set.
 *
 * Index: images, image stats
 * -----------------------------------------------------------------------------
*/

int get_ave_sum_ratio_without_invalid
(
    double*          ratio_ptr,
    const KJB_image* in1_ip,
    const KJB_image* in2_ip,
    double           threshold,
    int              min_num_good_points
)
{
    Pixel* in1_pos;
    Pixel* in2_pos;
    int    i, j, num_rows, num_cols;
    float  temp1;
    float  temp2;
    double sum      = 0.0;
    int    count    = 0;


    ERE(check_same_size_image(in1_ip, in2_ip));

    if (threshold < 0.0) threshold = 0.0;

    if (min_num_good_points < 1) min_num_good_points = 1;

    num_rows = in1_ip->num_rows;
    num_cols = in1_ip->num_cols;

    for (i=0; i<num_rows; i++)
    {
        in1_pos  = in1_ip->pixels[ i ];
        in2_pos  = in2_ip->pixels[ i ];

        for (j=0; j<num_cols; j++)
        {
            if (    (in1_pos->r > threshold) && (in2_pos->r > threshold)
                 && (in1_pos->g > threshold) && (in2_pos->g > threshold)
                 && (in1_pos->b > threshold) && (in2_pos->b > threshold)
                 && ( ! in1_pos->extra.invalid.r ) && ( ! in2_pos->extra.invalid.r)
                 && ( ! in1_pos->extra.invalid.g ) && ( ! in2_pos->extra.invalid.g)
                 && ( ! in1_pos->extra.invalid.b ) && ( ! in2_pos->extra.invalid.b)
               )
            {
                temp1 = in1_pos->r + in1_pos->g + in1_pos->b;
                temp2 = in2_pos->r + in2_pos->g + in2_pos->b;
                sum += (temp1 / temp2);
                count++;
            }

            in1_pos++;
            in2_pos++;
        }
    }

    if (count < min_num_good_points)
    {
        set_error("Not enough valid corresponding pixels for image ratio.");
        return ERROR;
    }
    else
    {
        ERE(verbose_pso(1, "Number of points for image ratio: %d.\n", count));

        *ratio_ptr = sum / count;
        return NO_ERROR;
    }
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

#ifdef __cplusplus
}
#endif

