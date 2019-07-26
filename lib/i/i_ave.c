
/* $Id: i_ave.c 21712 2017-08-20 18:21:41Z kobus $ */

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
#include "i/i_ave.h"
#include "i/i_valid.h"


#ifdef __cplusplus
extern "C" {
#endif

/* -------------------------------------------------------------------------- */

static int fs_average_dark_pixels = FALSE;

/* -------------------------------------------------------------------------- */

/* =============================================================================
 *                          set_image_average_options
 *
 *
 *
 * -----------------------------------------------------------------------------
*/

int set_image_average_options(const char* option, const char* value)
{
    char lc_option[ 100 ];
    int  temp_boolean_value;
    int  result             = NOT_FOUND;


    EXTENDED_LC_BUFF_CPY(lc_option, option);

    if (    (lc_option[ 0 ] == '\0')
         || (match_pattern(lc_option, "average-dark-pixels"))
       )
    {
        if (value == NULL)
        {
            return NO_ERROR;
        }
        else if (value[ 0 ] == '?')
        {
            ERE(pso("average-dark-pixels = \n",
                    fs_average_dark_pixels ? "t" : "f")); 
        }
        else if (value[ 0 ] == '\0')
        {
            if (fs_average_dark_pixels)
            {
                ERE(pso("Dark pixels are averaged and validity is based "));
                ERE(pso("on the average.\n"));
            }
            else
            {
                ERE(pso("Dark pixels are not averaged "));
                ERE(pso("(like other invalid pixels).\n"));
            }
        }
        else
        {
            ERE(temp_boolean_value = get_boolean_value(value));
            fs_average_dark_pixels = temp_boolean_value;
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

int magnify_image
(
    KJB_image**      out_ipp,
    const KJB_image* in_ip,
    int              row_count,
    int              col_count
)
{
    KJB_image*       out_ip;
    int              num_in_rows;
    int              num_in_cols;
    int              num_out_rows;
    int              num_out_cols;
    int              i, j, m, n;


    if ((row_count < 0) || (col_count < 0))
    {
        SET_ARGUMENT_BUG();
        return ERROR;
    }

    num_in_rows = in_ip->num_rows;
    num_in_cols = in_ip->num_cols;

    num_out_rows = num_in_rows * row_count;
    num_out_cols = num_in_cols * col_count;

    ERE(get_target_image(out_ipp, num_out_rows, num_out_cols));
    out_ip = *out_ipp;

    for (i=0; i<num_in_rows; i++)
    {
        for (j=0; j<num_in_cols; j++)
        {
            for (m=0; m<row_count; m++)
            {
                for (n=0; n<col_count; n++)
                {
                    out_ip->pixels[ i * row_count + m ][ j * col_count + n ] =
                        in_ip->pixels[ i ][ j ];
                }
            }
        }
    }

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                              ave_image
 *
 * Averages an image
 *
 * This routine averages each row_count by col_count rectangle of an images.
 * Invalid pixels are used in the computation, but if any channel of a pixel is
 * invalid, then the average value for that channel is set invalid, as is the
 * pixel itself.
 *
 * The output is put into the *out_ipp. If *out_ipp is NULL, then it is
 * created. If it is the wrong size, then it is resized. If it is the right
 * size, the storage is recycled as is.
 *
 * Returns:
 *    NO_ERROR on success and ERROR on failure, with an error message being set.
 *
 * Index:
 *    images, image average, image sampling, image smoothing
 *
 * -----------------------------------------------------------------------------
*/

int ave_image
(
    KJB_image**      out_ipp,
    const KJB_image* in_ip,
    int              row_count,
    int              col_count
)
{
    KJB_image*       out_ip;
    KJB_image*       no_dark_in_ip = NULL;
    const KJB_image* derived_in_ip;
    int              num_in_rows;
    int              num_in_cols;
    int              num_out_rows;
    int              num_out_cols;
    Pixel*           temp_row;
    Pixel*           temp_row_pos;
    Pixel*           in_pos;
    Pixel*           out_pos;
    float            ave_count;
    int              i, j, m, n;


    if ((row_count < 0) || (col_count < 0))
    {
        SET_ARGUMENT_BUG();
        return ERROR;
    }

    num_in_rows = in_ip->num_rows;
    num_in_cols = in_ip->num_cols;

    num_out_rows = num_in_rows / row_count;
    num_out_cols = num_in_cols / col_count;

    ave_count = row_count * col_count;

    ERE(get_target_image(out_ipp, num_out_rows, num_out_cols));
    out_ip = *out_ipp;

    if (fs_average_dark_pixels)
    {
        UNTESTED_CODE();

        ERE(kjb_copy_image(&no_dark_in_ip, in_ip));
        ERE(unmark_dark_pixels(no_dark_in_ip));
        derived_in_ip = no_dark_in_ip;
    }
    else
    {
        derived_in_ip = in_ip;
    }

    NRE(temp_row = N_TYPE_MALLOC(Pixel, num_out_cols));

    for (i=0; i<num_out_rows; i++)
    {
        out_pos = out_ip->pixels[ i ];
        temp_row_pos = temp_row;

        for (m=0; m<num_out_cols; m++)
        {
            temp_row_pos->r = 0.0;
            temp_row_pos->g = 0.0;
            temp_row_pos->b = 0.0;

            temp_row_pos->extra.invalid.r     = VALID_PIXEL;
            temp_row_pos->extra.invalid.g     = VALID_PIXEL;
            temp_row_pos->extra.invalid.b     = VALID_PIXEL;
            temp_row_pos->extra.invalid.pixel = VALID_PIXEL;

            temp_row_pos++;
        }

        for (j=0; j<row_count; j++)
        {
            in_pos = derived_in_ip->pixels[ i*row_count + j ];
            temp_row_pos = temp_row;

            for (m=0; m<num_out_cols; m++)
            {
                for (n=0; n<col_count; n++)
                {
                    temp_row_pos->r += in_pos->r;
                    temp_row_pos->g += in_pos->g;
                    temp_row_pos->b += in_pos->b;
                    temp_row_pos->extra.invalid.r |= in_pos->extra.invalid.r;
                    temp_row_pos->extra.invalid.g |= in_pos->extra.invalid.g;
                    temp_row_pos->extra.invalid.b |= in_pos->extra.invalid.b;
                    temp_row_pos->extra.invalid.pixel |= in_pos->extra.invalid.pixel;

                    in_pos++;
                }

                temp_row_pos++;
            }
        }

        temp_row_pos = temp_row;

        for (m=0; m<num_out_cols; m++)
        {
            out_pos->r = temp_row_pos->r / ave_count;
            out_pos->g = temp_row_pos->g / ave_count;
            out_pos->b = temp_row_pos->b / ave_count;
            out_pos->extra.invalid = temp_row_pos->extra.invalid;

            out_pos++;
            temp_row_pos++;
        }
    }

    kjb_free_image(no_dark_in_ip);
    kjb_free(temp_row);

    if (fs_average_dark_pixels)
    {
        ERE(mark_dark_pixels(out_ip));
    }

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                          ave_image_without_invalid
 *
 * Averages an image
 *
 * This routine averages each row_count by col_count rectangle of an images,
 * ignoring the data from invalid pixels, unless all pixels in the block are
 * invalid, in which case the invalid pixel values are used to compute the
 * invalid average. The average will also be invalid if less than
 * min_good_pixels have all valid components. If min_good_pixels is less than
 * one, one is used. If it is more than row_count*col_count, then
 * row_count*col_count is used.
 *
 * Thus, unlike ave_image(), this routine can output a valid pixel provided that
 * some of the pixels in a block are OK. Also unlike ave_image(), this routine
 * only uses invalid pixels if all pixels in the block are invalid.
 *
 * The output is put into the *out_ipp. If *out_ipp is NULL, then it is
 * created. If it is the wrong size, then it is resized. If it is the right
 * size, the storage is recycled as is.
 *
 * Returns:
 *    NO_ERROR on success and ERROR on failure, with an error message being set.
 *
 * Index:
 *    images, image average, image sampling, image smoothing
 *
 * -----------------------------------------------------------------------------
*/

int ave_image_without_invalid
(
    KJB_image**      out_ipp,
    const KJB_image* in_ip,
    int              row_count,
    int              col_count,
    int              min_good_pixels
)
{
    int              cell_size;
    KJB_image*       no_dark_in_ip    = NULL;
    const KJB_image* derived_in_ip;
    KJB_image*       out_ip;
    int              num_in_rows = in_ip->num_rows;
    int              num_in_cols = in_ip->num_cols;
    int              num_out_rows;
    int              num_out_cols;
    Pixel*           temp_row;
    Pixel*           temp_row_pos;
    Pixel*           temp_invalid_row;
    Pixel*           temp_invalid_row_pos;
    Pixel*           in_pos;
    Pixel*           out_pos;
    int*             ave_count_array;
    int*             ave_count_array_pos;
    int              i, j, m, n;


    if ((row_count <= 0) || (col_count <= 0))
    {
        SET_ARGUMENT_BUG();
        return ERROR;
    }

    if (row_count > num_in_rows)
    {
        row_count = num_in_rows;
    }

    if (col_count > num_in_cols)
    {
        col_count = num_in_cols;
    }

    cell_size = row_count * col_count;

    if (min_good_pixels > cell_size) min_good_pixels = cell_size;
    if (min_good_pixels < 1)         min_good_pixels = 1;

    num_out_rows = num_in_rows / row_count;
    num_out_cols = num_in_cols / col_count;

    ERE(get_target_image(out_ipp, num_out_rows, num_out_cols));
    out_ip = *out_ipp;

    if (fs_average_dark_pixels)
    {
        ERE(kjb_copy_image(&no_dark_in_ip, in_ip));
        ERE(unmark_dark_pixels(no_dark_in_ip));
        derived_in_ip = no_dark_in_ip;
    }
    else
    {
        derived_in_ip = in_ip;
    }

    temp_row          = N_TYPE_MALLOC(Pixel, num_out_cols);
    temp_invalid_row  = N_TYPE_MALLOC(Pixel, num_out_cols);
    ave_count_array   = INT_MALLOC(num_out_cols);

    if (    (temp_row == NULL)
         || (temp_invalid_row == NULL)
         || (ave_count_array == NULL)
       )
    {
        kjb_free(temp_row);
        kjb_free(temp_invalid_row);
        kjb_free(ave_count_array);
        return ERROR;
    }

    for (i=0; i<num_out_rows; i++)
    {
        out_pos = out_ip->pixels[ i ];
        temp_row_pos = temp_row;
        temp_invalid_row_pos = temp_invalid_row;

        ave_count_array_pos   = ave_count_array;

        for (m=0; m<num_out_cols; m++)
        {
            temp_row_pos->r = 0.0;
            temp_row_pos->g = 0.0;
            temp_row_pos->b = 0.0;
            temp_row_pos++;

            temp_invalid_row_pos->r = 0.0;
            temp_invalid_row_pos->g = 0.0;
            temp_invalid_row_pos->b = 0.0;

            temp_invalid_row_pos->extra.invalid.r     = VALID_PIXEL;
            temp_invalid_row_pos->extra.invalid.g     = VALID_PIXEL;
            temp_invalid_row_pos->extra.invalid.b     = VALID_PIXEL;
            temp_invalid_row_pos->extra.invalid.pixel = VALID_PIXEL;

            temp_invalid_row_pos++;

            *ave_count_array_pos++   = 0;
        }

        for (j=0; j<row_count; j++)
        {
            in_pos = derived_in_ip->pixels[ i*row_count + j ];
            temp_row_pos = temp_row;
            temp_invalid_row_pos = temp_invalid_row;

            ave_count_array_pos = ave_count_array;

            for (m=0; m<num_out_cols; m++)
            {
                for (n=0; n<col_count; n++)
                {
                    if (in_pos->extra.invalid.pixel)
                    {
                        temp_invalid_row_pos->r += in_pos->r;
                        temp_invalid_row_pos->g += in_pos->g;
                        temp_invalid_row_pos->b += in_pos->b;

                        /*
                        // Collect validity info in case we don't get enough
                        // valid pixels.
                        */
                        temp_invalid_row_pos->extra.invalid.r |= in_pos->extra.invalid.r;
                        temp_invalid_row_pos->extra.invalid.g |= in_pos->extra.invalid.g;
                        temp_invalid_row_pos->extra.invalid.b |= in_pos->extra.invalid.b;

                        temp_invalid_row_pos->extra.invalid.pixel |=
                                                        in_pos->extra.invalid.pixel;
                    }
                    else
                    {
                        temp_row_pos->r += in_pos->r;
                        temp_row_pos->g += in_pos->g;
                        temp_row_pos->b += in_pos->b;

                        (*ave_count_array_pos)++;
                    }

                    in_pos++;
                }

                temp_row_pos++;
                temp_invalid_row_pos++;

                ave_count_array_pos++;
            }
        }

        temp_row_pos = temp_row;
        temp_invalid_row_pos = temp_invalid_row;

        ave_count_array_pos = ave_count_array;

        for (m=0; m<num_out_cols; m++)
        {
            if (*ave_count_array_pos < 1)
            {
                out_pos->r = (temp_invalid_row_pos->r) / cell_size;
                out_pos->g = (temp_invalid_row_pos->g) / cell_size;
                out_pos->b = (temp_invalid_row_pos->b) / cell_size;
            }
            else
            {
                out_pos->r = (temp_row_pos->r) / (*ave_count_array_pos);
                out_pos->g = (temp_row_pos->g) / (*ave_count_array_pos);
                out_pos->b = (temp_row_pos->b) / (*ave_count_array_pos);
            }

            if (*ave_count_array_pos < min_good_pixels)
            {
                out_pos->extra.invalid = temp_invalid_row_pos->extra.invalid;
            }
            else
            {
                out_pos->extra.invalid.r     = VALID_PIXEL;
                out_pos->extra.invalid.g     = VALID_PIXEL;
                out_pos->extra.invalid.b     = VALID_PIXEL;
                out_pos->extra.invalid.pixel = VALID_PIXEL;
            }

            out_pos++;
            temp_row_pos++;
            temp_invalid_row_pos++;

            ave_count_array_pos++;
        }
    }

    kjb_free_image(no_dark_in_ip);

    kjb_free(temp_row);
    kjb_free(temp_invalid_row);
    kjb_free(ave_count_array);

    if (fs_average_dark_pixels)
    {
        ERE(mark_dark_pixels(out_ip));
    }

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* Kobus: 2017, I have no idea what this is for. 
 * FIXME CHECK where used, consider making it obsolete if it is not. 
*/

int ave_image_where_uniform
(
    KJB_image**      out_ipp,
    const KJB_image* in_ip,
    int              row_count,
    int              col_count,
    int              min_good_pixels,
    double           thresh
)
{
    KJB_image* out_ip;
    int        num_rows, num_cols;
    Pixel*     in_pos;
    int        i, j, m, n;
    int        excluded_count = 0;


    UNTESTED_CODE();

    ERE(ave_image_without_invalid(out_ipp, in_ip, row_count, col_count,
                                  min_good_pixels));
    if (row_count * col_count < 2)
    {
        TEST_PSO(("Skipping exclusion due to variance because block size is too small.\n"));
        return NO_ERROR;
    }
    else if (thresh <= 0.0)
    {
        TEST_PSO(("Skipping exclusion due to variance because threshold is non-positive.\n"));
        return NO_ERROR;
    }

    out_ip = * out_ipp;
    num_rows = out_ip->num_rows;
    num_cols = out_ip->num_cols;

    for (i=0; i<num_rows; i++)
    {
        for (j=0; j<num_cols; j++)
        {
            if (out_ip->pixels[ i ][ j ].extra.invalid.pixel != VALID_PIXEL)
            {
                continue;
            }
            else
            {
#ifdef TEST
                float r_ave   = out_ip->pixels[ i ][ j ].r;
                float g_ave   = out_ip->pixels[ i ][ j ].g;
                float b_ave   = out_ip->pixels[ i ][ j ].b;
#endif
                int   ii      = i * row_count;
                int   jj      = j * col_count;
                float sum_r   = FLT_ZERO;
                float sum_g   = FLT_ZERO;
                float sum_b   = FLT_ZERO;
                float sum_r_2 = FLT_ZERO;
                float sum_g_2 = FLT_ZERO;
                float sum_b_2 = FLT_ZERO;
                int   count   = 0;
                int excluded_pixel = FALSE;

                for (m=0; m<row_count; m++)
                {
                    in_pos = in_ip->pixels[ ii + m ] + jj;

                    for (n=0; n<col_count; n++)
                    {
                        if (in_pos->extra.invalid.pixel == VALID_PIXEL)
                        {
                            float r = in_pos->r;
                            float g = in_pos->g;
                            float b = in_pos->b;

                            count++;

#ifdef REALLY_TEST
                            if ((i==20) && (j == 20))
                            {
                                dbf(r);
                                dbf(g);
                                dbf(b);
                            }
#endif

                            sum_r += r;
                            sum_r_2 += r * r;

                            sum_g += g;
                            sum_g_2 += g * g;

                            sum_b += b;
                            sum_b_2 += b * b;
                        }

                        in_pos++;
                    }
                }

                ASSERT(IS_EQUAL_FLT(r_ave, sum_r / count));
                ASSERT(IS_EQUAL_FLT(g_ave, sum_g / count));
                ASSERT(IS_EQUAL_FLT(b_ave, sum_b / count));

                if (count < 2)
                {
                    out_ip->pixels[ i ][ j ].extra.invalid.pixel |= NOISY_PIXEL;
                    excluded_pixel = TRUE;
                }
                else
                {
                    double temp, stdev_r, stdev_g, stdev_b;

                    temp = sum_r_2 - sum_r * sum_r / count;
                    stdev_r = sqrt(temp /(count - 1));

                    temp = sum_g_2 - sum_g * sum_g / count;
                    stdev_g = sqrt(temp /(count - 1));

                    temp = sum_b_2 - sum_b * sum_b / count;
                    stdev_b = sqrt(temp /(count - 1));

                    if (stdev_r > thresh)
                    {
                        excluded_pixel = TRUE;
                        out_ip->pixels[ i ][ j ].extra.invalid.pixel |= NOISY_PIXEL;
                        out_ip->pixels[ i ][ j ].extra.invalid.r |= NOISY_PIXEL;
                    }
                    if (stdev_g > thresh)
                    {
                        excluded_pixel = TRUE;
                        out_ip->pixels[ i ][ j ].extra.invalid.pixel |= NOISY_PIXEL;
                        out_ip->pixels[ i ][ j ].extra.invalid.g |= NOISY_PIXEL;
                    }
                    if (stdev_b > thresh)
                    {
                        excluded_pixel = TRUE;
                        out_ip->pixels[ i ][ j ].extra.invalid.pixel |= NOISY_PIXEL;
                        out_ip->pixels[ i ][ j ].extra.invalid.b |= NOISY_PIXEL;
                    }

#ifdef REALLY_TEST
                    if ((i==20) && (j == 20))
                    {
                        dbf(stdev_r);
                        dbf(stdev_g);
                        dbf(stdev_b);
                    }
#endif

                }

                if (excluded_pixel)
                {
                    excluded_count++;
                }
            }
        }
    }

    verbose_pso(3, "%d pixels excluded due to stdev over %.1f.\n",
                excluded_count, thresh);

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                          sample_image
 *
 * Selects middle pixel from image blocks
 *
 * This routine selects the middle pixel of each row_count by col_count
 * rectangle of an image.
 *
 * The output is put into the *out_ipp. If *out_ipp is NULL, then it is
 * created. If it is the wrong size, then it is resized. If it is the right
 * size, the storage is recycled as is.
 *
 * Returns:
 *    NO_ERROR on success and ERROR on failure, with an error message being set.
 *
 * Index:
 *    images, image average, image sampling, image smoothing
 *
 * -----------------------------------------------------------------------------
*/


int sample_image
(
    KJB_image**      out_ipp,
    const KJB_image* in_ip,
    int              row_count,
    int              col_count
)
{
    int        num_in_rows;
    int        num_in_cols;
    int        num_out_rows;
    int        num_out_cols;
    Pixel*     in_pos;
    Pixel*     out_pos;
    int        i, j;
    KJB_image* out_ip;


    UNTESTED_CODE();

    if ((row_count < 0) || (col_count < 0))
    {
        SET_ARGUMENT_BUG();
        return ERROR;
    }

    num_in_rows = in_ip->num_rows;
    num_in_cols = in_ip->num_cols;

    num_out_rows = num_in_rows / row_count;
    num_out_cols = num_in_cols / col_count;

    ERE(get_target_image(out_ipp, num_out_rows, num_out_cols));
    out_ip = *out_ipp;

    for (i=0; i<num_out_rows; i++)
    {
        in_pos = in_ip->pixels[ i*row_count + row_count/2 ] + col_count/2;
        out_pos = out_ip->pixels[ i ];

        for (j=0; j<num_out_cols; j++)
        {
            *out_pos = *in_pos;

            out_pos++;
            in_pos += col_count;
        }
    }

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

#ifdef MEDIAN_FILTER_WITH_MOVING_WINDOW

/* =============================================================================
 *                                median_filter_image
 *
 * Does median filtering of an image
 *
 * Returns:
 *    NO_ERROR on success and ERROR on failure, with an error message being set.
 *
 * Index:
 *    images, image average, image sampling, image smoothing
 *
 * -----------------------------------------------------------------------------
*/

int median_filter_image
(
    KJB_image**      out_ipp,
    const KJB_image* in_ip,
    int              block_size
)
{
    KJB_image*      out_ip;
    int             num_rows, num_cols;
    Pixel*          in_pos;
    Pixel*          out_pos;
    int             i, j, m, n;
    Indexed_vector* r_vp     = NULL;
    Indexed_vector* g_vp     = NULL;
    Indexed_vector* b_vp     = NULL;
    int             num_pixels_in_block = block_size * block_size;
    int             half_width = block_size / 2;


    if ((block_size < 0) || (IS_EVEN(block_size)))
    {
        set_error("%d is in invalid median filter size.",
                  block_size);
        add_error("An odd integer larger than one is needed.");
        return ERROR;
    }

    ERE(get_target_indexed_vector(&r_vp, num_pixels_in_block));
    ERE(get_target_indexed_vector(&g_vp, num_pixels_in_block));
    ERE(get_target_indexed_vector(&b_vp, num_pixels_in_block));

    num_rows = in_ip->num_rows;
    num_cols = in_ip->num_cols;

    ERE(get_target_image(out_ipp, num_rows, num_cols));
    out_ip = *out_ipp;

    for (i=0; i<num_rows; i++)
    {
        out_pos = (*out_ipp)->pixels[ i ];

        for (j=0; j<num_cols; j++)
        {
            int count = 0;

            for (m=0; m<block_size; m++)
            {
                int mm = i + m - half_width;

                if ((mm < 0) || (mm >= num_rows)) continue;

                in_pos = in_ip->pixels[ mm ];

                for (n=0; n<block_size; n++)
                {
                    int nn = j + n - half_width;

                    if ((nn < 0) || (nn >= num_cols)) continue;
                    if (in_pos[ nn ].extra.invalid.pixel) continue;

                    ASSERT(count < num_pixels_in_block);

                    r_vp->elements[ count ].element = in_pos[ nn ].r;
                    g_vp->elements[ count ].element = in_pos[ nn ].g;
                    b_vp->elements[ count ].element = in_pos[ nn ].b;

                    count++;
                }
            }

            if (count > 1)
            {
                r_vp->length = count;
                g_vp->length = count;
                b_vp->length = count;

                ERE(ascend_sort_indexed_vector(r_vp));
                ERE(ascend_sort_indexed_vector(g_vp));
                ERE(ascend_sort_indexed_vector(b_vp));

                out_pos->r = r_vp->elements[ count / 2 ].element;
                out_pos->g = g_vp->elements[ count / 2 ].element;
                out_pos->b = b_vp->elements[ count / 2 ].element;
            }
            else
            {
                out_pos->r = in_ip->pixels[ i ][ j ].r;
                out_pos->g = in_ip->pixels[ i ][ j ].g;
                out_pos->b = in_ip->pixels[ i ][ j ].b;
            }

            out_pos->extra.invalid = in_ip->pixels[ i ][ j ].extra.invalid;

            out_pos++;
        }
    }

    free_indexed_vector(r_vp);
    free_indexed_vector(g_vp);
    free_indexed_vector(b_vp);

    return NO_ERROR;
}

#else

/* =============================================================================
 *                                median_filter_image
 *
 * Does median filtering of an image
 *
 * Returns:
 *    NO_ERROR on success and ERROR on failure, with an error message being set.
 *
 * Index:
 *    images, image average, image sampling, image smoothing
 *
 * -----------------------------------------------------------------------------
*/

int median_filter_image
(
    KJB_image**      out_ipp,
    const KJB_image* in_ip,
    int              block_size
)
{
    KJB_image*       out_ip;
    int              num_in_rows;
    int              num_in_cols;
    int              num_out_rows;
    int              num_out_cols;
    Pixel*           in_pos;
    Pixel*           out_pos;
    int              i, j, m, n;
    Indexed_vector* r_vp = NULL;
    Indexed_vector* g_vp = NULL;
    Indexed_vector* b_vp = NULL;
    int num_pixels_in_block = block_size * block_size;
    int median = num_pixels_in_block / 2;


    TEST_PSO(("Validity processing for median filtering not yet finished.\n"));

    if ((block_size < 0) || (block_size < 0))
    {
        SET_ARGUMENT_BUG();
        return ERROR;
    }

    ERE(get_target_indexed_vector(&r_vp, num_pixels_in_block));
    ERE(get_target_indexed_vector(&g_vp, num_pixels_in_block));
    ERE(get_target_indexed_vector(&b_vp, num_pixels_in_block));

    num_in_rows = in_ip->num_rows;
    num_in_cols = in_ip->num_cols;

    num_out_rows = num_in_rows / block_size;
    num_out_cols = num_in_cols / block_size;

    ERE(get_target_image(out_ipp, num_out_rows, num_out_cols));
    out_ip = *out_ipp;

    for (i=0; i<num_out_rows; i++)
    {
        out_pos = (*out_ipp)->pixels[ i ];

        dbi(i);

        for (j=0; j<num_out_cols; j++)
        {
            int count = 0;

            for (m=0; m<block_size; m++)
            {
                ASSERT( i * block_size + m < num_in_rows );
                ASSERT( j * block_size     < num_in_cols );

                in_pos = in_ip->pixels[ i*block_size + m ] + j * block_size;

                for (n=0; n<block_size; n++)
                {
                    ASSERT(count < num_pixels_in_block);

                    r_vp->elements[ count ].element = in_pos->r;
                    g_vp->elements[ count ].element = in_pos->g;
                    b_vp->elements[ count ].element = in_pos->b;

                    in_pos++;
                    count++;
                }
            }

            ERE(ascend_sort_indexed_vector(r_vp));
            ERE(ascend_sort_indexed_vector(g_vp));
            ERE(ascend_sort_indexed_vector(b_vp));

            out_pos->r = r_vp->elements[ median ].element;
            out_pos->g = g_vp->elements[ median ].element;
            out_pos->b = b_vp->elements[ median ].element;

            /*
            // FIX
            */
            out_pos->extra.invalid.r = VALID_PIXEL;
            out_pos->extra.invalid.g = VALID_PIXEL;
            out_pos->extra.invalid.b = VALID_PIXEL;
            out_pos->extra.invalid.pixel = VALID_PIXEL;

            out_pos++;
        }
    }

    free_indexed_vector(r_vp);
    free_indexed_vector(g_vp);
    free_indexed_vector(b_vp);

    return NO_ERROR;
}

#endif

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

#ifdef __cplusplus
}
#endif

