
/* $Id: i_int_matrix.c 4727 2009-11-16 20:53:54Z kobus $ */

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
#include "i/i_draw.h"
#include "i/i_int_matrix.h"

#ifdef __cplusplus
extern "C" {
#endif

/* -------------------------------------------------------------------------- */

static int map_integer_to_RGB
(
    kjb_uint32 int_val,
    int*   r_ptr,
    int*   g_ptr,
    int*   b_ptr
);

/* -------------------------------------------------------------------------- */

/* =============================================================================
 *                             make_int_matrix_image
 *
 * Makes an image from an integer matrix.
 *
 * This routine makes an image from an integer matrix, mostly for
 * debuging/visualization purposes. The image created has blocks for each
 * element of the matrix. There are two different kinds of encodings: color and
 * black and white. To specify which are wanted, there are two corresponding
 * width arguments, color_width and bw_width. If either is zero or less, than
 * the corresponding encoding is ignored. However, at least one has to be
 * positive. The width of the blocks corresponding to an element in the integer
 * array is the maximum of the two widths.
 *
 * If both widths are none zero, the smaller of the two widths becoms a block
 * inside the larger. It does not make sense to have the sizes the same (the
 * black and white encoding overwrites the color one). Further, if both widths
 * are non zero, it is recomended that they are
 * both odd or even.
 *
 * The color encoding is adjusted so that their is at least on color with one of
 * R,G,B at roughly 255.
 *
 * The black and white encoding is the value scaled by bw_brightness_factor. If
 * bw_brightness_factor is 1 or less, then 1 is used.
 *
 * A typical use of this is to have a small black and white square whose RGB
 * value can be used to look up the value in the array inside a larger color
 * squares which are coded to easily visualize where values are different.
 *
 * Index: images, image output, integer matrices, visualization
 *
 * -----------------------------------------------------------------------------
*/

int make_int_matrix_image
(
    KJB_image**       ipp,
    const Int_matrix* mp,
    int               color_width,
    int               bw_width,
    int               bw_brightness_factor
)
{
    int num_rows = mp->num_rows;
    int num_cols = mp->num_cols;
    int width = MAX_OF(color_width, bw_width);
    int i, j;
    int pass;


    if ((bw_width < 1) && (color_width < 1))
    {
        SET_ARGUMENT_BUG();
        return ERROR;
    }

    if (bw_brightness_factor < 1) bw_brightness_factor = 1;

    ERE(get_zero_image(ipp, width * num_rows, width * num_cols));

    for (pass = 0; pass < 2; pass++)
    {
        if (    ((pass == 0) && (color_width > bw_width) && (color_width > 0))
             || ((pass == 1) && (bw_width > color_width) && (color_width > 0))
           )
        {
            /* Do color. */
            int offset = MAX_OF(0, bw_width - color_width) / 2;
            int max_val = 0;

            /* Dry run to set scale. We have to do it this way because we
             * modulate brightness differently for color and black and white.
            */
            for (i = 0; i < num_rows; i++)
            {
                for (j = 0; j < num_cols; j++)
                {
                    kjb_uint32 int_val = mp->elements[ i ][ j ];
                    int r, g, b;

                    if (int_val == 0) continue;

                    ERE(map_integer_to_RGB(int_val, &r, &g, &b));

                    max_val = MAX_OF(max_val, MAX_OF(r, MAX_OF(g, b)));
                }
            }

            if (max_val > 0)
            {
                for (i = 0; i < num_rows; i++)
                {
                    for (j = 0; j < num_cols; j++)
                    {
                        kjb_uint32 int_val = mp->elements[ i ][ j ];
                        int r, g, b;

                        if (int_val == 0) continue;

                        ERE(map_integer_to_RGB(int_val, &r, &g, &b));

                        ERE(image_draw_rectangle_2(*ipp,
                                                   i * width + offset,
                                                   j * width + offset,
                                                   color_width, color_width,
                                                   (255.0f * (float)r) / (float)max_val,
                                                   (255.0f * (float)g) / (float)max_val,
                                                   (255.0f * (float)b) / (float)max_val));
                    }
                }
            }

            if (max_val > 0)
            {
                /* Scale to just under 255.0 */
                ERE(ow_scale_image((*ipp), 254.9 / (double)max_val));
            }
        }
        else if (bw_width > 0)
        {
            /* Now black and white. */
            int offset = MAX_OF(0, color_width - bw_width) / 2;

            for (i = 0; i < num_rows; i++)
            {
                for (j = 0; j < num_cols; j++)
                {
                    int val = mp->elements[ i ][ j ];

                    if (val == 0) continue;

                    ERE(image_draw_rectangle(*ipp,
                                             i * width + offset,
                                             j * width + offset,
                                             bw_width, bw_width,
                                             val * bw_brightness_factor,
                                             val * bw_brightness_factor,
                                             val * bw_brightness_factor));
                }
            }
        }
    }

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

static int map_integer_to_RGB
(
    kjb_uint32 int_val,
    int*       r_ptr,
    int*       g_ptr,
    int*       b_ptr
)
{
    int r = 0, g = 0, b = 0;
    int i;

    for (i = 0; i < 24; i++)
    {
        int new_bit = int_val % 2;

        if ((i > 0) && (i % 3 == 0))
        {
            r *= 2;
            g *= 2;
            b *= 2;
        }

        if (i % 3 == 0)
        {
            r += new_bit;
        }
        else if (i % 3 == 1)
        {
            g += new_bit;
        }
        else
        {
            b += new_bit;
        }

        int_val /= 2;
    }

    *r_ptr = r;
    *g_ptr = g;
    *b_ptr = b;

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

#ifdef __cplusplus
}
#endif

