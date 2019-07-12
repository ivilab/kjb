
/* $Id: i_collage.c 21712 2017-08-20 18:21:41Z kobus $ */

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
#include "i/i_float_io.h"
#include "i/i_collage.h"
#include "i/i_draw.h"

#ifdef __cplusplus
extern "C" {
#endif

/* -------------------------------------------------------------------------- */

static Pixel fs_collage_border_colour =
{
    0.0, 0.0, 0.0,
    {{ VALID_PIXEL, VALID_PIXEL, VALID_PIXEL, VALID_PIXEL }}
};

static Pixel fs_collage_background_colour =
{
    200.0, 200.0, 200.0,
    {{ VALID_PIXEL, VALID_PIXEL, VALID_PIXEL, VALID_PIXEL }}
};

static int fs_collage_division_border_width   = 5;
static int fs_collage_outside_border_width    = 5;
static int fs_collage_horizontal_block_size    = 1;
static int fs_collage_vertical_block_size      = 1;
static int fs_collage_square_images           = FALSE;

/* -------------------------------------------------------------------------- */

int set_collage_options(const char* option, const char* value)
{
    int  temp_int;
    int  temp_boolean;
    char lc_option[ 100 ];
    int  result           = NOT_FOUND;


    EXTENDED_LC_BUFF_CPY(lc_option, option);

    if (    (lc_option[ 0 ] == '\0')
         || (match_pattern(lc_option, "collage-square-images"))
       )
    {
        if (value == NULL)
        {
            return NO_ERROR;
        }
        else if (value[ 0 ] == '?')
        {
            ERE(pso("collage-square-images = %s\n",
                    fs_collage_square_images ? "t" : "f"));
        }
        else if (value[ 0 ] == '\0')
        {
            ERE(pso("Collage images %s forced to be square.\n",
                    fs_collage_square_images ? "are" : "are not"));
        }
        else
        {
            ERE(temp_boolean = get_boolean_value(value));
            fs_collage_square_images = temp_boolean;
        }
        result = NO_ERROR;
    }

    if (    (lc_option[ 0 ] == '\0')
         || (match_pattern(lc_option, "collage-vertical-block-size"))
       )
    {
        if (value == NULL)
        {
            return NO_ERROR;
        }
        else if (value[ 0 ] == '?')
        {
            ERE(pso("collage-vertical-block-size = %d\n",
                    fs_collage_vertical_block_size));
        }
        else if (value[ 0 ] == '\0')
        {
            ERE(pso("Collage vertical block size is %d.\n",
                    fs_collage_vertical_block_size));
        }
        else
        {
            ERE(ss1i(value, &temp_int));
            fs_collage_vertical_block_size = temp_int;
        }
        result = NO_ERROR;
    }

    if (    (lc_option[ 0 ] == '\0')
         || (match_pattern(lc_option, "collage-horizontal-block-size"))
       )
    {
        if (value == NULL)
        {
            return NO_ERROR;
        }
        else if (value[ 0 ] == '?')
        {
            ERE(pso("collage-horizontal-block-size = %d\n",
                    fs_collage_horizontal_block_size));
        }
        else if (value[ 0 ] == '\0')
        {
            ERE(pso("Collage horizontal block size is %d.\n",
                    fs_collage_horizontal_block_size));
        }
        else
        {
            ERE(ss1i(value, &temp_int));
            fs_collage_horizontal_block_size = temp_int;
        }
        result = NO_ERROR;
    }

    if (    (lc_option[ 0 ] == '\0')
         || (match_pattern(lc_option, "collage-outside-border-width"))
       )
    {
        if (value == NULL)
        {
            return NO_ERROR;
        }
        else if (value[ 0 ] == '?')
        {
            ERE(pso("collage-outside-border-width = %d\n",
                    fs_collage_outside_border_width));
        }
        else if (value[ 0 ] == '\0')
        {
            ERE(pso("Collage outside border width is %d.\n",
                    fs_collage_outside_border_width));
        }
        else
        {
            ERE(ss1i(value, &temp_int));
            fs_collage_outside_border_width = temp_int;
        }
        result = NO_ERROR;
    }

    if (    (lc_option[ 0 ] == '\0')
         || (match_pattern(lc_option, "collage-division-border-width"))
       )
    {
        if (value == NULL)
        {
            return NO_ERROR;
        }
        else if (value[ 0 ] == '?')
        {
            ERE(pso("collage-division-border-width = %d\n",
                    fs_collage_division_border_width));
        }
        else if (value[ 0 ] == '\0')
        {
            ERE(pso("Collage division border width is %d.\n",
                    fs_collage_division_border_width));
        }
        else
        {
            ERE(ss1i(value, &temp_int));
            fs_collage_division_border_width = temp_int;
        }
        result = NO_ERROR;
    }

    if (    (lc_option[ 0 ] == '\0')
         || (match_pattern(lc_option, "collage-border-colour"))
       )
    {
        if (value == NULL)
        {
            return NO_ERROR;
        }
        else if (value[ 0 ] == '?')
        {
            ERE(pso("Under construction\n"));
        }
        else if (value[ 0 ] == '\0')
        {
            ERE(pso("Collage border colour is (%d, %d, %d).\n",
                    kjb_rintf(fs_collage_border_colour.r),
                    kjb_rintf(fs_collage_border_colour.g),
                    kjb_rintf(fs_collage_border_colour.b)));
        }
        else
        {
            ERE(string_scan_float_rgb(value, &fs_collage_border_colour));
        }
        result = NO_ERROR;
    }

    if (    (lc_option[ 0 ] == '\0')
         || (match_pattern(lc_option, "collage-background-colour"))
       )
    {
        if (value == NULL)
        {
            return NO_ERROR;
        }
        else if (value[ 0 ] == '?')
        {
            ERE(pso("Under construction\n"));
        }
        else if (value[ 0 ] == '\0')
        {
            ERE(pso("Collage background colour is (%d, %d, %d).\n",
                    kjb_rintf(fs_collage_background_colour.r),
                    kjb_rintf(fs_collage_background_colour.g),
                    kjb_rintf(fs_collage_background_colour.b)));
        }
        else
        {
            ERE(string_scan_float_rgb(value, &fs_collage_background_colour));
        }
        result = NO_ERROR;
    }

    return result;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                             make_image_collage
 *
 * Build a simple image collage
 *
 * This builds a collage using a retangular layout as indicated by the
 * num_horizontal and num_vertical parameters (which must be positive).
 * If you want to control things like borders, colors, and other details, try
 * make_image_collage_2 instead.
 *
 * The number of images in the input array must be equal to or more than
 * the product num_horizontal X num_vertical.
 *
 * Returns:
 *      ERROR or NO_ERROR as appropriate.
 *
 * Documenter:  Andrew Predoehl
 * -----------------------------------------------------------------------------
*/

int make_image_collage
(
    KJB_image** out_ipp,
    int         num_horizontal,
    int         num_vertical,
    const KJB_image* const* ip_list
)
{
    return make_image_collage_2(out_ipp,
                                num_horizontal, num_vertical, ip_list,
                                &fs_collage_background_colour,
                                &fs_collage_border_colour,
                                fs_collage_outside_border_width,
                                fs_collage_division_border_width,
                                fs_collage_horizontal_block_size,
                                fs_collage_vertical_block_size,
                                fs_collage_square_images);
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                             make_image_collage_2
 *
 * This will build a single collage of a series of input images, with control
 * over the border size, colors, and other layout variables.
 * This function needs better documentation -- please expand.
 *
 * The number of images in the input array must be equal to or more than
 * the product num_horizontal X num_vertical.
 *
 * Returns:
 *      ERROR or NO_ERROR as appropriate.
 *
 * Documenter:  Andrew Predoehl
 *
 * -----------------------------------------------------------------------------
*/

int make_image_collage_2
(
    KJB_image** out_ipp,
    int         num_horizontal,
    int         num_vertical,
    const KJB_image* const* ip_list,
    Pixel*      background_colour_ptr,
    Pixel*      border_colour_ptr,
    int         outside_border_width,
    int         division_border_width,
    int         horizontal_block_size_arg,
    int         vertical_block_size_arg,
    int         square_images
)
{
    int num_rows, num_cols, i, j, k, m, n;
    int max_num_cols = 0;
    int max_num_rows = 0;
    KJB_image const* ip;
    Pixel*     out_pos;
    int        num_out_cols;
    int        num_out_rows;
    Pixel**    out_row;
    int        image_count  = 0;
    int        vertical_block_size   = 1;
    int        horizontal_block_size = 1;
    Pixel      border_colour;
    Pixel      background_colour;
#ifdef TEST
    Pixel*     save_out_pos;
    int        row_count            = 0;
#endif

    if (square_images < 0) square_images = fs_collage_square_images;

    if (vertical_block_size_arg < 0)
    {
        vertical_block_size_arg = fs_collage_vertical_block_size;
    }

    if (horizontal_block_size_arg < 0)
    {
        horizontal_block_size_arg = fs_collage_horizontal_block_size;
    }

    if (division_border_width < 0)
    {
        division_border_width = fs_collage_division_border_width;
    }

    if (outside_border_width < 0)
    {
        outside_border_width = fs_collage_outside_border_width;
    }

    if (border_colour_ptr == NULL)
    {
        border_colour = fs_collage_border_colour;
    }
    else
    {
        border_colour = *border_colour_ptr;
    }

    if (background_colour_ptr == NULL)
    {
        background_colour = fs_collage_background_colour;
    }
    else
    {
        background_colour = *background_colour_ptr;
    }

    if (division_border_width > 0)
    {
        if (vertical_block_size_arg > 0)
        {
            vertical_block_size = MIN_OF(vertical_block_size_arg,
                                        num_vertical);
        }
        else
        {
            vertical_block_size = num_vertical;
        }

        if (horizontal_block_size > 0)
        {
            horizontal_block_size = MIN_OF(horizontal_block_size_arg,
                                          num_horizontal);
        }
        else
        {
            horizontal_block_size = num_horizontal;
        }
    }

    for (i = 0; i < num_horizontal; i++)
    {
        for (j = 0; j < num_vertical; j++)
        {
            if (ip_list[ i * num_vertical + j ] != NULL)
            {
                int cur_num_rows = ip_list[ i * num_vertical + j ]->num_rows;
                int cur_num_cols = ip_list[ i * num_vertical + j ]->num_cols;

                if (cur_num_rows > max_num_rows) max_num_rows = cur_num_rows;
                if (cur_num_cols > max_num_cols) max_num_cols = cur_num_cols;

                image_count++;
            }
        }
    }

    if ((image_count == 0) || (max_num_rows == 0) || (max_num_cols == 0))
    {
        kjb_free_image(*out_ipp);
        *out_ipp = NULL;
        return ERROR;
    }

    if (square_images)
    {
        max_num_rows = MAX_OF(max_num_rows, max_num_cols);
        max_num_cols = max_num_rows;
    }

    num_out_rows = num_horizontal * max_num_rows;
    num_out_rows += 2 * outside_border_width;
    num_out_rows += ((num_horizontal - 1) / horizontal_block_size) * division_border_width;

    num_out_cols = num_vertical * max_num_cols;
    num_out_cols += 2 * outside_border_width;
    num_out_cols += ((num_vertical - 1) / vertical_block_size) * division_border_width;

    ERE(get_target_image(out_ipp, num_out_rows, num_out_cols));
    out_row = (*out_ipp)->pixels;

    if (outside_border_width > 0)
    {
        for (m=0; m<outside_border_width; m++)
        {
            out_pos = *out_row;
#ifdef TEST
            save_out_pos = out_pos;
#endif

            for (n=0; n<num_out_cols; n++)
            {
                out_pos->r = border_colour.r;
                out_pos->g = border_colour.g;
                out_pos->b = border_colour.b;

                out_pos->extra.invalid.r     = VALID_PIXEL;
                out_pos->extra.invalid.g     = VALID_PIXEL;
                out_pos->extra.invalid.b     = VALID_PIXEL;
                out_pos->extra.invalid.pixel = VALID_PIXEL;

                out_pos++;
            }
            out_row++;
#ifdef TEST
            row_count++;
            ASSERT(row_count <= num_out_rows);
            ASSERT((out_pos - save_out_pos) == num_out_cols);
#endif
        }
    }

    for (i=0; i<num_horizontal; i++)
    {
        if (    (i > 0)
             && (division_border_width > 0)
             && (i % horizontal_block_size == 0)
           )
        {
            for (m=0; m<division_border_width; m++)
            {
                out_pos = *out_row;
#ifdef TEST
                save_out_pos = out_pos;
#endif

                for (n=0; n<num_out_cols; n++)
                {
                    out_pos->r = border_colour.r;
                    out_pos->g = border_colour.g;
                    out_pos->b = border_colour.b;

                    out_pos->extra.invalid.r     = VALID_PIXEL;
                    out_pos->extra.invalid.g     = VALID_PIXEL;
                    out_pos->extra.invalid.b     = VALID_PIXEL;
                    out_pos->extra.invalid.pixel = VALID_PIXEL;

                    out_pos++;
                }
                out_row++;
#ifdef TEST
                row_count++;
                ASSERT(row_count <= num_out_rows);
                ASSERT((out_pos - save_out_pos) == num_out_cols);
#endif
            }
        }

        for (j=0; j<max_num_rows; j++)
        {
            out_pos = *out_row;
#ifdef TEST
            save_out_pos = out_pos;
#endif

            if (outside_border_width > 0)
            {
                for (m=0; m<division_border_width; m++)
                {
                    out_pos->r = border_colour.r;
                    out_pos->g = border_colour.g;
                    out_pos->b = border_colour.b;

                    out_pos->extra.invalid.r     = VALID_PIXEL;
                    out_pos->extra.invalid.g     = VALID_PIXEL;
                    out_pos->extra.invalid.b     = VALID_PIXEL;
                    out_pos->extra.invalid.pixel = VALID_PIXEL;

                    out_pos++;
                }
            }

            for (k=0; k<num_vertical; k++)
            {
                if (    (k > 0)
                     && (division_border_width > 0)
                     && (k % vertical_block_size == 0)
                   )
                {
                    for (m=0; m<division_border_width; m++)
                    {
                        out_pos->r = border_colour.r;
                        out_pos->g = border_colour.g;
                        out_pos->b = border_colour.b;

                        out_pos->extra.invalid.r     = VALID_PIXEL;
                        out_pos->extra.invalid.g     = VALID_PIXEL;
                        out_pos->extra.invalid.b     = VALID_PIXEL;
                        out_pos->extra.invalid.pixel = VALID_PIXEL;

                        out_pos++;
                    }
                }

                ip = ip_list[i*num_vertical + k];

                num_rows = (ip == NULL) ? 0 : ip->num_rows;
                num_cols = (ip == NULL) ? 0 : ip->num_cols;

                if (num_rows > j)
                {
                    Pixel* ip_row_pos = (ip->pixels)[j];

                    for (m=0; m<num_cols; m++)
                    {
                        *out_pos = *ip_row_pos;
                        ip_row_pos++;
                        out_pos++;
                    }

                    for (m=num_cols; m<max_num_cols; m++)
                    {
                        out_pos->r = background_colour.r;
                        out_pos->g = background_colour.g;
                        out_pos->b = background_colour.b;

                        out_pos->extra.invalid.r     = VALID_PIXEL;
                        out_pos->extra.invalid.g     = VALID_PIXEL;
                        out_pos->extra.invalid.b     = VALID_PIXEL;
                        out_pos->extra.invalid.pixel = VALID_PIXEL;

                        out_pos++;
                    }
                }
                else
                {
                    for (m=0; m<max_num_cols; m++)
                    {
                        out_pos->r = background_colour.r;
                        out_pos->g = background_colour.g;
                        out_pos->b = background_colour.b;

                        out_pos->extra.invalid.r     = VALID_PIXEL;
                        out_pos->extra.invalid.g     = VALID_PIXEL;
                        out_pos->extra.invalid.b     = VALID_PIXEL;
                        out_pos->extra.invalid.pixel = VALID_PIXEL;

                        out_pos++;
                    }
                }
            }

            if (outside_border_width > 0)
            {
                for (m=0; m<outside_border_width; m++)
                {
                    out_pos->r = border_colour.r;
                    out_pos->g = border_colour.g;
                    out_pos->b = border_colour.b;

                    out_pos->extra.invalid.r     = VALID_PIXEL;
                    out_pos->extra.invalid.g     = VALID_PIXEL;
                    out_pos->extra.invalid.b     = VALID_PIXEL;
                    out_pos->extra.invalid.pixel = VALID_PIXEL;

                    out_pos++;
                }
            }

            ASSERT((out_pos - save_out_pos) == num_out_cols);

            out_row++;
#ifdef TEST
            row_count++;
            ASSERT(row_count <= num_out_rows);
#endif
        }
    }

    if (outside_border_width > 0)
    {
        for (m=0; m<division_border_width; m++)
        {
            out_pos = *out_row;
#ifdef TEST
            save_out_pos = out_pos;
#endif

            for (n=0; n<num_out_cols; n++)
            {
                out_pos->r = border_colour.r;
                out_pos->g = border_colour.g;
                out_pos->b = border_colour.b;

                out_pos->extra.invalid.r     = VALID_PIXEL;
                out_pos->extra.invalid.g     = VALID_PIXEL;
                out_pos->extra.invalid.b     = VALID_PIXEL;
                out_pos->extra.invalid.pixel = VALID_PIXEL;

                out_pos++;
            }
            out_row++;
#ifdef TEST
            row_count++;
            ASSERT(row_count <= num_out_rows);
            ASSERT((out_pos - save_out_pos) == num_out_cols);
#endif
        }
    }

#ifdef TEST
    ASSERT(row_count == num_out_rows);
#endif

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

int make_compact_image_collage
(
    KJB_image** out_ipp,
    int         num_horizontal,
    int         num_vertical,
    KJB_image** ip_list
)
{
    return make_compact_image_collage_2(out_ipp, num_horizontal, num_vertical,
                                        ip_list, (Int_matrix**)NULL);
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

int make_compact_image_collage_2
(
    KJB_image**  out_ipp,
    int          num_horizontal,
    int          num_vertical,
    KJB_image**  ip_list,
    Int_matrix** image_coords_mpp
)
{
    int num_rows, num_cols, i, j, k, m, n;
    int max_num_cols = 0;
    KJB_image* ip;
    Pixel*     out_pos;
    int        num_out_cols;
    int        num_out_rows = 0;
    Pixel**    out_row;
    int        image_count  = 0;
    Int_matrix* image_coords_mp = NULL;
    int out_i = 0;
    int out_j = 0;
    int image_index;


    for (i = 0; i < num_horizontal; i++)
    {
        int max_num_rows = 0;

        for (j = 0; j < num_vertical; j++)
        {
            if (ip_list[ i * num_vertical + j ] != NULL)
            {
                int cur_num_rows = ip_list[ i * num_vertical + j ]->num_rows;
                int cur_num_cols = ip_list[ i * num_vertical + j ]->num_cols;

                if (cur_num_rows > max_num_rows) max_num_rows = cur_num_rows;
                if (cur_num_cols > max_num_cols) max_num_cols = cur_num_cols;

                image_count++;
            }
        }

        if (    (num_out_rows > 0)  /* Have at least one row. */
             && (max_num_rows > 0)  /* This row has some meat. */
             && (fs_collage_division_border_width > 0)
            )
        {
            num_out_rows += fs_collage_division_border_width;
        }

        num_out_rows += max_num_rows;
    }

    if ((image_count == 0) || (num_out_rows == 0) || (max_num_cols == 0))
    {
        kjb_free_image(*out_ipp);
        *out_ipp = NULL;
        return 0;
    }

    num_out_cols = num_vertical * max_num_cols;

    if (fs_collage_division_border_width > 0)
    {
        num_out_cols += fs_collage_division_border_width * (num_vertical - 1);
    }

    ERE(get_target_image(out_ipp, num_out_rows, num_out_cols));
    out_row = (*out_ipp)->pixels;

    if (image_coords_mpp != NULL)
    {
        ERE(get_initialized_int_matrix(image_coords_mpp,
                                       num_horizontal * num_vertical,
                                       4, NOT_SET));
        image_coords_mp = *image_coords_mpp;
    }

    for (i=0; i<num_horizontal; i++)
    {
        int max_num_rows = 0;

        for (j = 0; j < num_vertical; j++)
        {
            if (ip_list[ i * num_vertical + j ] != NULL)
            {
                int cur_num_rows = ip_list[ i * num_vertical + j ]->num_rows;

                if (cur_num_rows > max_num_rows) max_num_rows = cur_num_rows;
            }
        }

        if (max_num_rows == 0) continue;

        if ((i > 0) && (fs_collage_division_border_width > 0))
        {
            for (m=0; m<fs_collage_division_border_width; m++)
            {
                out_pos = *out_row;

                for (n=0; n<num_out_cols; n++)
                {
                    out_pos->r = fs_collage_border_colour.r;
                    out_pos->g = fs_collage_border_colour.g;
                    out_pos->b = fs_collage_border_colour.b;

                    out_pos->extra.invalid.r     = VALID_PIXEL;
                    out_pos->extra.invalid.g     = VALID_PIXEL;
                    out_pos->extra.invalid.b     = VALID_PIXEL;
                    out_pos->extra.invalid.pixel = VALID_PIXEL;

                    out_pos++;
                }
                out_row++;
                out_i++;
            }
        }

        for (j=0; j<max_num_rows; j++)
        {
            out_pos = *out_row;
            out_j = 0;

            for (k=0; k<num_vertical; k++)
            {
                if ((k > 0) && (fs_collage_division_border_width > 0))
                {
                    for (m=0; m<fs_collage_division_border_width; m++)
                    {
                        out_pos->r = fs_collage_border_colour.r;
                        out_pos->g = fs_collage_border_colour.g;
                        out_pos->b = fs_collage_border_colour.b;

                        out_pos->extra.invalid.r     = VALID_PIXEL;
                        out_pos->extra.invalid.g     = VALID_PIXEL;
                        out_pos->extra.invalid.b     = VALID_PIXEL;
                        out_pos->extra.invalid.pixel = VALID_PIXEL;

                        out_pos++;
                        out_j++;
                    }
                }

                image_index = i*num_vertical + k;
                ip = ip_list[image_index];

                num_rows = (ip == NULL) ? 0 : ip->num_rows;
                num_cols = (ip == NULL) ? 0 : ip->num_cols;

                if (    (image_coords_mp != NULL)
                     && (image_coords_mp->elements[ image_index ][ 0 ] == NOT_SET)
                   )
                {
                    image_coords_mp->elements[ image_index ][ 0 ] = out_i;
                    image_coords_mp->elements[ image_index ][ 1 ] = out_j;
                    image_coords_mp->elements[ image_index ][ 2 ] = out_i + num_rows;
                    image_coords_mp->elements[ image_index ][ 3 ] = out_j + num_cols;
                }

                if (num_rows > j)
                {
                    Pixel* ip_row_pos = (ip->pixels)[j];

                    for (m=0; m<num_cols; m++)
                    {
                        *out_pos = *ip_row_pos;
                        ip_row_pos++;
                        out_pos++;
                        out_j++;
                    }

                    for (m=num_cols; m<max_num_cols; m++)
                    {
                        out_pos->r = fs_collage_background_colour.r;
                        out_pos->g = fs_collage_background_colour.g;
                        out_pos->b = fs_collage_background_colour.b;

                        out_pos->extra.invalid.r     = VALID_PIXEL;
                        out_pos->extra.invalid.g     = VALID_PIXEL;
                        out_pos->extra.invalid.b     = VALID_PIXEL;
                        out_pos->extra.invalid.pixel = VALID_PIXEL;

                        out_pos++;
                        out_j++;
                    }
                }
                else
                {
                    for (m=0; m<max_num_cols; m++)
                    {
                        out_pos->r = fs_collage_background_colour.r;
                        out_pos->g = fs_collage_background_colour.g;
                        out_pos->b = fs_collage_background_colour.b;

                        out_pos->extra.invalid.r     = VALID_PIXEL;
                        out_pos->extra.invalid.g     = VALID_PIXEL;
                        out_pos->extra.invalid.b     = VALID_PIXEL;
                        out_pos->extra.invalid.pixel = VALID_PIXEL;

                        out_pos++;
                        out_j++;
                    }
                }
            }
            out_row++;
            out_i++;
        }
    }

    return image_count;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

#ifdef HOW_IT_WAS 

/*
// NOW SEE i2_collage.c
*/

#define MAX_NUM_MONTAGE_IMAGES  200


int ip_output_montage(output_file, num_images,
                      num_montage_rows, num_montage_cols, images, labels,
                      extra)
    const char* output_file;
    int         num_images;
    int         num_montage_rows;
    int         num_montage_cols;
    KJB_image** images;
    char        labels[][ XXX_MAX_MONTAGE_IMAGE_LABEL_SIZE ];
    const char* extra;
{
    int i, num_rows, num_cols;
    int max_num_rows = 0;
    int max_num_cols = 0;
    char geometry_str[ 100 ];
    char temp_names[ MAX_NUM_MONTAGE_IMAGES ][ MAX_FILE_NAME_SIZE ];
    char temp_labels[ MAX_NUM_MONTAGE_IMAGES ][ XXX_MAX_MONTAGE_IMAGE_LABEL_SIZE ];
    int  num_images_written = 0;
    int  result  = NO_ERROR;
    int  max_dim;
    int  count   = 0;

    if (num_images > MAX_NUM_MONTAGE_IMAGES)
    {
        SET_ARGUMENT_BUG();
        return ERROR;
    }

    for (i = 0; i<num_images; i++)
    {
        if (images[ i ] != NULL)
        {
            num_rows = images[ i ]->num_rows;
            num_cols = images[ i ]->num_cols;

            if (num_rows > max_num_rows) max_num_rows = num_rows;
            if (num_cols > max_num_cols) max_num_cols = num_cols;

            count++;

            if (count >= num_montage_rows * num_montage_cols) break;
        }
    }

    max_dim = MAX_OF(max_num_rows, max_num_cols);

    ERE(kjb_sprintf(geometry_str, sizeof(geometry_str), "%dx%d",
                    max_dim, max_dim));

    for (i = 0; i<num_images; i++)
    {
        dbi();
        if (images[ i ] != NULL)
        {
            ERE(BUFF_GET_TEMP_FILE_NAME(temp_names[ num_images_written ]));

            result = kjb_write_image(images[ i ],
                                     temp_names[ num_images_written ]);
            if (result == ERROR) break;

            BUFF_CPY(temp_labels[ num_images_written ], labels[ i ]);
            dbs(temp_labels[ num_images_written ]);

            num_images_written++;

            if (num_images_written >= num_montage_rows * num_montage_cols)
            {
                break;
            }
        }
    }

    if ((result != ERROR) && (num_images_written > 0))
    {
        result = output_montage_2(output_file, geometry_str, num_images_written,
                                  num_montage_rows, num_montage_cols,
                                  temp_names, temp_labels, extra);
    }

    for (i = 0; i<num_images_written; i++)
    {
        EPE(kjb_unlink(temp_names[ i ]));
    }

    return result;
}

#else

/*
// NOW SEE i2_collage.c
*/

#endif

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

int output_montage
(
    const char* output_file,
    int         num_images,
    int         num_montage_rows,
    int         num_montage_cols,
    char        image_names[][ MAX_FILE_NAME_SIZE ],
    char**      labels,
    const char* extra
)
{
    int i, num_rows, num_cols;
    int max_num_rows = 0;
    int max_num_cols = 0;
    char geometry_str[ 100 ];
    KJB_image* ip     = NULL;
    int        result = NO_ERROR;


    for (i = 0; i<num_images; i++)
    {
        result = kjb_read_image(&ip, image_names[ i ]);

        if (result == ERROR) break;

        num_rows = ip->num_rows;
        num_cols = ip->num_cols;

        if (num_rows > max_num_rows) max_num_rows = num_rows;
        if (num_cols > max_num_cols) max_num_cols = num_cols;
    }

    if (result != ERROR)
    {
        result = kjb_sprintf(geometry_str, sizeof(geometry_str), "%dx%d",
                             max_num_rows, max_num_cols);
    }

    if (result != ERROR)
    {
        result = output_montage_2(output_file, geometry_str, num_images,
                                  num_montage_rows, num_montage_cols,
                                  image_names, labels, extra);
    }

    kjb_free_image(ip);

    return result;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

int output_montage_2
(
    const char* output_file,
    const char* geometry_str,
    int         num_images,
    int         num_montage_rows,
    int         num_montage_cols,
    char        image_names[][ MAX_FILE_NAME_SIZE ],
    char**      labels,
    const char* extra
)
{
    char tile_str[ 100 ];
    char montage_cmd[ 100000 ];
    char buff[ 10000 ];
    int  count;
    int  full_num_images = 0;


    if ((num_montage_rows <= 0) || (num_montage_cols <= 0))
    {
        num_montage_rows = 0;
        num_montage_cols = 0;

        while (num_images > full_num_images)
        {
            num_montage_rows++;
            num_montage_cols = (int)((double)num_montage_rows * 4.0 / 3.0 + 0.0000001);
            full_num_images = num_montage_rows * num_montage_cols;
        }
    }

    ERE(kjb_sprintf(tile_str, sizeof(tile_str), "%dx%d",
                    num_montage_cols, num_montage_rows));

    BUFF_CPY(montage_cmd, "montage ");

    if (geometry_str != NULL)
    {
        BUFF_CAT(montage_cmd, "-geometry ");
        BUFF_CAT(montage_cmd, geometry_str);
    }

    BUFF_CAT(montage_cmd, " +frame +shadow ");
    BUFF_CAT(montage_cmd, "-font -*-times-medium-r-normal--12-* ");
    BUFF_CAT(montage_cmd, "-background white ");
    BUFF_CAT(montage_cmd, "-tile " );
    BUFF_CAT(montage_cmd, tile_str );
    BUFF_CAT(montage_cmd, " " );

    if (extra != NULL)
    {
        BUFF_CAT(montage_cmd, extra);
        BUFF_CAT(montage_cmd, " " );
    }

    for (count = 0; count < num_images; count++)
    {
        ERE(kjb_sprintf(buff, sizeof(buff), "-label \"%s\" %s ",
                        (labels == NULL) ? "" : (labels[ count ] == NULL) ? "" : labels[ count ],
                        image_names[ count ]));
        BUFF_CAT(montage_cmd, buff);
    }

    BUFF_CAT(montage_cmd, output_file);

    p_stderr(montage_cmd);
    p_stderr("\n");

    return kjb_system(montage_cmd);

}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

#ifdef __cplusplus
}
#endif

