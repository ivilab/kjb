
/* $Id: i_draw.c 16804 2014-05-15 19:55:04Z predoehl $ */

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

#ifdef __cplusplus
extern "C" {
#endif

/* -------------------------------------------------------------------------- */

#define NUM_POINTS_IN_LINE   2000

/* -------------------------------------------------------------------------- */

static int image_draw_gradient_2_helper
(
    KJB_image* ip,
    int        i_beg,
    int        j_beg,
    int        i_end,
    int        j_end,
    int        width,
    int        r1,
    int        g1,
    int        b1,
    int        r2,
    int        g2,
    int        b2,
    int        i_is_i
);

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* helper function for image_draw_circle routines */
static void circle_points
(
    KJB_image* ip,
    int        i,
    int        j,
    int        x,
    int        y,
    int        lw,
    float      r,
    float      g,
    float      b
);

/* -------------------------------------------------------------------------- */

/* =============================================================================
 *                             image_draw_contour
 *
 * Draws a contour on an image
 *
 * This routine draws a contour onto an image which is specified by the point
 * pairs in the matrix argument. The routine image_draw_segment() is used to
 * draw lines between consecutive points. The matrix (x,y) is mapped into image
 * (i,j).
 *
 * Note:
 *    If displayed, image (i,j) generally results in the first coordinate
 *    indexing row number downwards, and the second coordinate indexing column.
 *    Thus the X and Y axes swap, and the vertical axis goes in the opposite
 *    direction than in the XY coordinate system.
 *
 * Index: images, image output
 *
 * -----------------------------------------------------------------------------
*/

int image_draw_contour
(
    KJB_image* ip,
    Matrix*    mp,
    int        width,
    int        r,
    int        g,
    int        b
)
{
    Vector* vp1    = NULL;
    Vector* vp2    = NULL;
    int     i;
    int     result = NO_ERROR;


    for (i = 0; i<mp->num_rows; i++)
    {
        if (result == ERROR) break;

        result = get_matrix_row(&vp1, mp, i);


        if (result != ERROR)
        {
             result = get_matrix_row(&vp2, mp,
                                     ((i + 1) == mp->num_rows) ? 0 : i + 1);
        }

        if (result != ERROR)
        {
            image_draw_segment(ip, vp1, vp2, width, r, g, b);
        }
    }

    free_vector(vp1);
    free_vector(vp2);

    return result;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                             image_draw_segment
 *
 * Draws a line segment on an image
 *
 * This routine sets the pixels of "ip" in the segment joining p1_vp
 * and p2_vp to (r,g,b). If the coordinates are not integers, then we simply
 * truncate to get integral endpoints. If width is greater than 1, then instead
 * of filling in pixels, we fill in squares of size 1+2*width. We don't use a
 * particular intelligent algorithm for smoothing (anti-aliasing), as this
 * routine is mostly used for debugging.
 *
 * Note:
 *    If displayed, image (i,j) generally results in the first coordinate
 *    indexing row number downwards, and the second coordinate indexing column.
 *    Thus the X and Y axes swap, and the vertical axis goes in the opposite
 *    direction than in the XY coordinate system.
 *
 * Index: images, image output
 *
 * -----------------------------------------------------------------------------
*/

int image_draw_segment
(
    KJB_image* ip,
    Vector*    p1_vp,
    Vector*    p2_vp,
    int        width,
    int        r,
    int        g,
    int        b
)
{
    return image_draw_gradient(ip, p1_vp, p2_vp, width, r, g, b, r, g, b);
}

/* =============================================================================
 *                             image_draw_segment_2
 *
 * Draws a line segment on an image
 *
 * This routine sets the pixels of "ip" in the segment joining (i_beg,
 * j_beg) and (i_end, j_end) to (r,g,b).
 *
 * Note:
 *    If displayed, image (i,j) generally results in the first coordinate
 *    indexing row number downwards, and the second coordinate indexing column.
 *    Thus the X and Y axes swap, and the vertical axis goes in the opposite
 *    direction than in the XY coordinate system.
 *
 * Index: images, image output
 *
 * -----------------------------------------------------------------------------
*/

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

int image_draw_segment_2
(
    KJB_image* ip,
    int        i_beg,
    int        j_beg,
    int        i_end,
    int        j_end,
    int        width,
    int        r,
    int        g,
    int        b
)
{
    return image_draw_gradient_2(ip, i_beg, j_beg, i_end, j_end, width,
                                 r, g, b, r, g, b);
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                             image_draw_gradient
 *
 * Draws a line segment with gradient on an image
 *
 * This routine sets the pixels of "ip" in the segment joining p1_vp
 * and p2_vp to a shade of (r1,g1,b1) and (r2,g2,b2).
 *
 * Note:
 *    If displayed, image (i,j) generally results in the first coordinate
 *    indexing row number downwards, and the second coordinate indexing column.
 *    Thus the X and Y axes swap, and the vertical axis goes in the opposite
 *    direction than in the XY coordinate system.
 *
 * Index: images, image output
 *
 * -----------------------------------------------------------------------------
*/

int image_draw_gradient
(
    KJB_image* ip,
    Vector*    p1_vp,
    Vector*    p2_vp,
    int        width,
    int        r1,
    int        g1,
    int        b1,
    int        r2,
    int        g2,
    int        b2
)
{
    if ((ip == NULL) || (p1_vp == NULL) || (p2_vp == NULL)) return NO_ERROR;

    return image_draw_gradient_2(ip,
                                 (int)p1_vp->elements[ 0 ],
                                 (int)p1_vp->elements[ 1 ],
                                 (int)p2_vp->elements[ 0 ],
                                 (int)p2_vp->elements[ 1 ],
                                 width, r1, g1, b1, r2, g2, b2);
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                             image_draw_gradient_2
 *
 * Draws a line segment with gradient on an image
 *
 * This routine sets the pixels of "ip" in the segment joining (i_beg, i_end) to
 * (j_beg, j_end) to a shade of (r1,g1,b1) and (r2,g2,b2).
 *
 * Index: images, image output
 *
 * -----------------------------------------------------------------------------
*/

int image_draw_gradient_2
(
    KJB_image* ip,
    int        i_beg,
    int        j_beg,
    int        i_end,
    int        j_end,
    int        width,
    int        r1,
    int        g1,
    int        b1,
    int        r2,
    int        g2,
    int        b2
)
{
    if (ip == NULL) return NO_ERROR;

    /*
     * Line drawing is simpler if we do slopes greater than one differently than
     * the slopes less than one. Negative versus positive slopes is dealt with
     * later.
    */

    if (ABS_OF(j_end - j_beg) > ABS_OF(i_end - i_beg))
    {
        /* Reverse y and x. */
        return image_draw_gradient_2_helper(ip, j_beg, i_beg, j_end, i_end,
                                            width, r1, g1, b1, r2, g2, b2,
                                            FALSE);
    }
    else
    {
        return image_draw_gradient_2_helper(ip, i_beg, j_beg, i_end, j_end,
                                            width, r1, g1, b1, r2, g2, b2,
                                            TRUE);
    }
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

static int image_draw_gradient_2_helper
(
    KJB_image* ip,
    int        i_beg,
    int        j_beg,
    int        i_end,
    int        j_end,
    int        width,
    int        r1,
    int        g1,
    int        b1,
    int        r2,
    int        g2,
    int        b2,
    int        i_is_i
)
{

    /*
     * Kobus, Sept 2004: In sympathy with my graphics students, this routine,
     * which is the guts of line drawing, was changed to reflect the algorithm
     * developed in that class.
    */

    if (ip == NULL) return NO_ERROR;

    if (i_beg == i_end)
    {
        ASSERT(j_beg == j_end);
        return image_draw_point(ip, i_beg, j_beg, width,
                                (r1 + r2) / 2, (g1 + g2) / 2, (b1 + b2) / 2);
    }
    else if (i_beg > i_end)
    {
        /* Switch so we go increase x on the recursive call. */
        return image_draw_gradient_2_helper(ip, i_end, j_end, i_beg, j_beg,
                                            width, r2, g2, b2, r1, g1, b1,
                                            i_is_i);
    }
    else
    {
        int    di = i_end - i_beg;
        int    dj = j_end - j_beg;
        float  f_r     = MIN_OF(255, MAX_OF(0, r1));
        float  f_g     = MIN_OF(255, MAX_OF(0, g1));
        float  f_b     = MIN_OF(255, MAX_OF(0, b1));
        float  r_range = MIN_OF(255, MAX_OF(0, r2)) - r1;
        float  g_range = MIN_OF(255, MAX_OF(0, g2)) - g1;
        float  b_range = MIN_OF(255, MAX_OF(0, b2)) - b1;
        float  r_step  = r_range / (float)di;
        float  g_step  = g_range / (float)di;
        float  b_step  = b_range / (float)di;
        int    i = i_beg;
        int    j = j_beg;
        int    p = 2 * dj - di;

        width = MAX_OF(0, width);

        if (i_is_i)
        {
            ERE(image_draw_point_2(ip, i, j, width, f_r, f_g, f_b));
        }
        else
        {
            ERE(image_draw_point_2(ip, j, i, width, f_r, f_g, f_b));
        }

        /*
         * Pretend that we care about speed and break into the two cases
         * (postive and negative slope) outside the loop.
        */
        if (dj < 0)
        {
            while (i < i_end)
            {
                i++;

                f_r += r_step;
                f_g += g_step;
                f_b += b_step;

                if (p < 0)
                {
                    j--;
                    p += 2*di;
                }

                p += 2*dj;

                if (i_is_i)
                {
                    ERE(image_draw_point_2(ip, i, j, width, f_r, f_g, f_b));
                }
                else
                {
                    ERE(image_draw_point_2(ip, j, i, width, f_r, f_g, f_b));
                }
            }
        }
        else
        {
            while (i < i_end)
            {
                i++;

                f_r += r_step;
                f_g += g_step;
                f_b += b_step;

                if (p > 0)
                {
                    j++;
                    p -= 2*di;
                }

                p += 2*dj;

                if (i_is_i)
                {
                    ERE(image_draw_point_2(ip, i, j, width, f_r, f_g, f_b));
                }
                else
                {
                    ERE(image_draw_point_2(ip, j, i, width, f_r, f_g, f_b));
                }
            }
        }
    }

    return NO_ERROR;
}

/* =============================================================================
 *                             image_draw_points
 *
 * Draws a set of points on an image
 *
 * This routine sets the pixels of "ip" whose coordinates are in the matrix "mp"
 * to (r,g,b).
 * If the image uses alpha, the alpha channel is set to 255.
 *
 * Note:
 *    If displayed, image (i,j) generally results in the first coordinate
 *    indexing row number downwards, and the second coordinate indexing column.
 *    Thus the X and Y axes swap, and the vertical axis goes in the opposite
 *    direction than in the XY coordinate system.
 *
 * Index: images, image output
 *
 * -----------------------------------------------------------------------------
*/

int image_draw_points(KJB_image* ip, Matrix* mp, int r, int g, int b)
{
    int    num_rows = ip->num_rows;
    int    num_cols = ip->num_cols;
    int    i, j;
    int    count;
    float  f_r      = r;
    float  f_g      = g;
    float  f_b      = b;
    double x, y;


    if ((ip == NULL) || (mp == NULL)) return NO_ERROR;

    for (count=0; count<mp->num_rows; count++)
    {
        x = mp->elements[ count ][ 0 ];
        y = mp->elements[ count ][ 1 ];
        i = MAX_OF(0, MIN_OF(num_rows - 1, (int)(x + 0.5)));
        j = MAX_OF(0, MIN_OF(num_cols - 1, (int)(y + 0.5)));

        ip->pixels[ i ][ j ].r = f_r;
        ip->pixels[ i ][ j ].g = f_g;
        ip->pixels[ i ][ j ].b = f_b;


        if (ip->flags & HAS_ALPHA_CHANNEL)
        {
            ip->pixels[ i ][ j ].extra.alpha = 255;
        }
        else
        {
            ip->pixels[ i ][ j ].extra.invalid.r = VALID_PIXEL;
            ip->pixels[ i ][ j ].extra.invalid.g = VALID_PIXEL;
            ip->pixels[ i ][ j ].extra.invalid.b = VALID_PIXEL;
            ip->pixels[ i ][ j ].extra.invalid.pixel = VALID_PIXEL;
        }
    }

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                             image_draw_pixels
 *
 * Draws a set of pixels on an image
 *
 * This routine sets the pixels of "ip" whose coordinates are in "pixels"
 * to (r,g,b).
 * If the image uses alpha, the alpha channel is set to 255.
 *
 * Note:
 *    If displayed, image (i,j) generally results in the first coordinate
 *    indexing row number downwards, and the second coordinate indexing column.
 *    Thus the X and Y axes swap, and the vertical axis goes in the opposite
 *    direction than in the XY coordinate system.
 *
 * Index: images, image output
 *
 * -----------------------------------------------------------------------------
*/

int image_draw_pixels
(
    KJB_image*  ip,
    int         num_pixels,
    Pixel_info* pixels,
    int         r,
    int         g,
    int         b
)
{
    int   i, j;
    int   count;
    float f_r   = r;
    float f_g   = g;
    float f_b   = b;

    if ((ip == NULL) || (pixels == NULL)) return NO_ERROR;

    for (count = 0; count < num_pixels; count++)
    {
        i = pixels[ count ].i;
        j = pixels[ count ].j;

        ip->pixels[ i ][ j ].r = f_r;
        ip->pixels[ i ][ j ].g = f_g;
        ip->pixels[ i ][ j ].b = f_b;

        if (ip->flags & HAS_ALPHA_CHANNEL)
        {
            ip->pixels[ i ][ j ].extra.alpha = 255;
        }
        else
        {
            ip->pixels[ i ][ j ].extra.invalid.pixel = VALID_PIXEL;
            ip->pixels[ i ][ j ].extra.invalid.r = VALID_PIXEL;
            ip->pixels[ i ][ j ].extra.invalid.g = VALID_PIXEL;
            ip->pixels[ i ][ j ].extra.invalid.b = VALID_PIXEL;
        }
    }

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                             image_draw_point
 *
 * Draws a point on an image
 *
 * This routine sets the pixels (i += width, j += width) of "ip" to (r,g,b). The
 * color triple is specified using integers. See image_draw_point_2() for a
 * similar routine using floating point (r,g,b).
 *
 * Note:
 *    If displayed, image (i,j) generally results in the first coordinate
 *    indexing row number downwards, and the second coordinate indexing column.
 *    Thus the X and Y axes swap, and the vertical axis goes in the opposite
 *    direction than in the XY coordinate system.
 *
 * Index: images, image output
 *
 * -----------------------------------------------------------------------------
*/

int image_draw_point
(
    KJB_image* ip,
    int        i,
    int        j,
    int        width,
    int        r,
    int        g,
    int        b
)
{
    float f_r = r;
    float f_g = g;
    float f_b = b;

    return image_draw_point_2(ip, i, j, width, f_r, f_g, f_b);
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                             image_draw_point_2
 *
 * Draws a point on an image
 *
 * This routine sets the pixels (i += width, j += width) of "ip" to (r,g,b) in
 * floating point. See image_draw_point() for a similar routine with integral
 * colors.  If the image uses alpha, the alpha channel is set to 255.
 *
 * Note:
 *    If displayed, image (i,j) generally results in the first coordinate
 *    indexing row number downwards, and the second coordinate indexing column.
 *    Thus the X and Y axes swap, and the vertical axis goes in the opposite
 *    direction than in the XY coordinate system.
 *
 * Index: images, image output
 *
 * -----------------------------------------------------------------------------
*/

int image_draw_point_2
(
    KJB_image* ip,
    int        i,
    int        j,
    int        width,
    float      f_r,
    float      f_g,
    float      f_b
)
{
    int   di, dj;

    if (ip == NULL) return NO_ERROR;

    for (di=-width; di<=width; di++)
    {
        for (dj=-width; dj<=width; dj++)
        {
            if (    (i + di >= 0)
                 && (j + dj >= 0)
                 && (i + di < ip->num_rows)
                 && (j + dj < ip->num_cols)
               )
            {
                ip->pixels[ i + di ][ j + dj ].r = f_r;
                ip->pixels[ i + di ][ j + dj ].g = f_g;
                ip->pixels[ i + di ][ j + dj ].b = f_b;

                if (ip->flags & HAS_ALPHA_CHANNEL)
                {
                    ip->pixels[i + di][j + dj].extra.alpha = 255;
                }
                else
                {
                    ip->pixels[i + di][j + dj].extra.invalid.pixel=VALID_PIXEL;
                    ip->pixels[i + di][j + dj].extra.invalid.r = VALID_PIXEL;
                    ip->pixels[i + di][j + dj].extra.invalid.g = VALID_PIXEL;
                    ip->pixels[i + di][j + dj].extra.invalid.b = VALID_PIXEL;
                }
            }
        }
    }

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                             image_draw_add_to_point
 *
 * Adds (r,g,b) to a point on an image
 *
 * This routine adds (r,g,b) the pixels (i += width, j += width) of "ip". The
 * color triple is specified using integers. See image_add_to_point_2() for a
 * similar routine using floating point (r,g,b), and image_blend_with_point()
 * for a crude way to blend the (r,g,b).
 *
 * Note:
 *    If displayed, image (i,j) generally results in the first coordinate
 *    indexing row number downwards, and the second coordinate indexing column.
 *    Thus the X and Y axes swap, and the vertical axis goes in the opposite
 *    direction than in the XY coordinate system.
 *
 * Index: images, image output
 *
 * -----------------------------------------------------------------------------
*/

int image_draw_add_to_point
(
    KJB_image* ip,
    int        i,
    int        j,
    int        width,
    int        r,
    int        g,
    int        b
)
{
    float f_r = r;
    float f_g = g;
    float f_b = b;

    return image_draw_add_to_point_2(ip, i, j, width, f_r, f_g, f_b);
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                           image_draw_add_to_point_2
 *
 * Bleands (r,g,b) with a point on an image
 *
 * This routine crudely blends (r,g,b) to the pixels (i += width, j += width) of
 * "ip". The color triple is specified using integers. See
 * image_blend_with_point_2() for a similar routine using floating point
 * (r,g,b), and image_add_to_point() to force adding the (r,g,b), and
 * image_draw_point() to overwrite the pixel entirely.
 *
 * Note:
 *    If displayed, image (i,j) generally results in the first coordinate
 *    indexing row number downwards, and the second coordinate indexing column.
 *    Thus the X and Y axes swap, and the vertical axis goes in the opposite
 *    direction than in the XY coordinate system.
 *
 * Index: images, image output
 *
 * -----------------------------------------------------------------------------
*/

int image_draw_add_to_point_2
(
    KJB_image* ip,
    int        i,
    int        j,
    int        width,
    float      f_r,
    float      f_g,
    float      f_b
)
{
    int   di, dj;

    if (ip == NULL) return NO_ERROR;

    for (di=-width; di<=width; di++)
    {
        for (dj=-width; dj<=width; dj++)
        {
            if (    (i + di >= 0)
                 && (j + dj >= 0)
                 && (i + di < ip->num_rows)
                 && (j + dj < ip->num_cols)
               )
            {
                ip->pixels[ i + di ][ j + dj ].r += f_r;
                ip->pixels[ i + di ][ j + dj ].g += f_g;
                ip->pixels[ i + di ][ j + dj ].b += f_b;
            }
        }
    }

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

int image_draw_blend_with_point
(
    KJB_image* ip,
    int        i,
    int        j,
    int        width,
    int        r,
    int        g,
    int        b
)
{
    float f_r = r;
    float f_g = g;
    float f_b = b;

    return image_draw_blend_with_point_2(ip, i, j, width, f_r, f_g, f_b);
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

int image_draw_blend_with_point_2
(
    KJB_image* ip,
    int        i,
    int        j,
    int        width,
    float      f_r,
    float      f_g,
    float      f_b
)
{
    int   di, dj;

    if (ip == NULL) return NO_ERROR;

    for (di=-width; di<=width; di++)
    {
        for (dj=-width; dj<=width; dj++)
        {
            if (    (i + di >= 0)
                 && (j + dj >= 0)
                 && (i + di < ip->num_rows)
                 && (j + dj < ip->num_cols)
               )
            {
                if (ip->pixels[ i + di ][ j + dj ].r > 128.0)
                {
                    ip->pixels[ i + di ][ j + dj ].r -= f_g / 2.0;
                    ip->pixels[ i + di ][ j + dj ].r -= f_b / 2.0;
                }
                else
                {
                    ip->pixels[ i + di ][ j + dj ].r += f_r;
                }

                if (ip->pixels[ i + di ][ j + dj ].g > 128.0)
                {
                    ip->pixels[ i + di ][ j + dj ].g -= f_r / 2.0;
                    ip->pixels[ i + di ][ j + dj ].g -= f_b / 2.0;
                }
                else
                {
                    ip->pixels[ i + di ][ j + dj ].g += f_g;
                }

                if (ip->pixels[ i + di ][ j + dj ].b > 128.0)
                {
                    ip->pixels[ i + di ][ j + dj ].b -= f_g / 2.0;
                    ip->pixels[ i + di ][ j + dj ].b -= f_r / 2.0;
                }
                else
                {
                    ip->pixels[ i + di ][ j + dj ].b += f_b;
                }
            }
        }
    }

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                             image_draw_image
 *
 * Draws an image onto an image
 *
 * This routine draws an image ip onto canvas_ip  at location (i, j), reducing
 * the size of ip by scale. The image is clipped if part (or all) of it lies
 * outsize canvas_ip.
 *
 * Note:
 *    If displayed, image (i,j) generally results in the first coordinate
 *    indexing row number downwards, and the second coordinate indexing column.
 *    Thus the X and Y axes swap, and the vertical axis goes in the opposite
 *    direction than in the XY coordinate system.
 *
 * Index: images, image output
 *
 * -----------------------------------------------------------------------------
*/

int image_draw_image
(
    KJB_image*       canvas_ip,
    const KJB_image* ip,
    int              i,
    int              j,
    int              scale
)
{
    int di, dj;
    int set_alpha;
    int ii = i;

    /* If the canvas is using alpha but the input image is not, we must assign
     * an alpha value.  In other words, we need an alpha value, but we cannot
     * get it from the input.
     */
    if (   (canvas_ip->flags & HAS_ALPHA_CHANNEL)
        && !(ip->flags & HAS_ALPHA_CHANNEL)
       )
    {
        set_alpha = 1;
    }
    else
    {
        /* Either the canvas does not use alpha, or we should copy from input.
         */
        set_alpha = 0;
    }

    if (ip == NULL) return NO_ERROR;

    for (di = 0; di  < ip->num_rows; di += scale)
    {
        int jj = j;

        for (dj = 0; dj < ip->num_cols; dj += scale)
        {
            if (    (ii >= 0)
                 && (jj >= 0)
                 && (ii < canvas_ip->num_rows)
                 && (jj < canvas_ip->num_cols)
               )
            {
                canvas_ip->pixels[ ii ][ jj ] = ip->pixels[ di ][ dj ];

                if (set_alpha)
                {
                    canvas_ip->pixels[ ii ][ jj ].extra.alpha = 255;
                }
            }

            jj++;
        }

        ii++;
    }

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                             image_draw_rectangle
 *
 * Draws a rectangle on an image
 *
 * This routine draws a rectangle of dimensions (width, height) at location
 * (i,j) onto an image.
 *
 * See image_draw_rectangle_2() for a version that takes floating point RGB.
 *
 * Note:
 *    If displayed, image (i,j) generally results in the first coordinate
 *    indexing row number downwards, and the second coordinate indexing column.
 *    Thus the X and Y axes swap, and the vertical axis goes in the opposite
 *    direction than in the XY coordinate system.
 *
 * Index: images, image output
 *
 * -----------------------------------------------------------------------------
*/

int image_draw_rectangle
(
    KJB_image* ip,
    int        i,
    int        j,
    int        height,
    int        width,
    int        r,
    int        g,
    int        b
)
{

    return image_draw_rectangle_2(ip, i, j, height, width, (float)r, (float)g, (float)b);
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                             image_draw_rectangle_2
 *
 * Draws a rectangle on an image
 *
 * This routine draws a rectangle of dimensions (width, height) at location
 * (i,j) onto an image. It is similar to image_draw_rectangle() except that it
 * takes floating point RGB.
 * If the image uses alpha, the alpha channel is set to 255.
 *
 * Note:
 *    If displayed, image (i,j) generally results in the first coordinate
 *    indexing row number downwards, and the second coordinate indexing column.
 *    Thus the X and Y axes swap, and the vertical axis goes in the opposite
 *    direction than in the XY coordinate system.
 *
 * Index: images, image output
 *
 * -----------------------------------------------------------------------------
*/

int image_draw_rectangle_2
(
    KJB_image* ip,
    int        i,
    int        j,
    int        height,
    int        width,
    float      f_r,
    float      f_g,
    float      f_b
)
{
    int   di, dj;
    int   ii  = i;

    if (ip == NULL) return NO_ERROR;

    for (di=0; di<height; di++)
    {
        int jj = j;

        for (dj=0; dj<width; dj++)
        {
            if (    (ii >= 0)
                 && (jj >= 0)
                 && (ii < ip->num_rows)
                 && (jj < ip->num_cols)
               )
            {
                ip->pixels[ ii ][ jj ].r = f_r;
                ip->pixels[ ii ][ jj ].g = f_g;
                ip->pixels[ ii ][ jj ].b = f_b;

                if (ip->flags & HAS_ALPHA_CHANNEL)
                {
                    ip->pixels[ ii ][ jj ].extra.alpha = 255;
                }
                else
                {
                    ip->pixels[ ii ][ jj ].extra.invalid.pixel = VALID_PIXEL;
                    ip->pixels[ ii ][ jj ].extra.invalid.r = VALID_PIXEL;
                    ip->pixels[ ii ][ jj ].extra.invalid.g = VALID_PIXEL;
                    ip->pixels[ ii ][ jj ].extra.invalid.b = VALID_PIXEL;
                }
            }

            jj++;
        }
        ii++;
    }

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

int image_draw_box(KJB_image* out_ip, int i, int j, int half_size,
                   int width, int r, int g, int b)
{

    ERE(image_draw_segment_2(out_ip,
                             i - half_size, j - half_size,
                             i - half_size, j + half_size,
                             width, r, g, b));
    ERE(image_draw_segment_2(out_ip,
                             i - half_size, j + half_size,
                             i + half_size, j + half_size,
                             width, r, g, b));
    ERE(image_draw_segment_2(out_ip,
                             i + half_size, j + half_size,
                             i + half_size, j - half_size,
                             width, r, g, b));
    ERE(image_draw_segment_2(out_ip,
                             i + half_size, j - half_size,
                             i - half_size, j - half_size,
                             width, r, g, b));

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                             image_draw_circle
 *
 * Draws a circle on an image
 *
 * This routine draws a circle of radius 'radius' at location (i,j) onto an image.
 *
 * See image_draw_circle_2() for a version that takes floating point RGB.
 *
 * Note:
 *    If displayed, image (i,j) generally results in the first coordinate
 *    indexing row number downwards, and the second coordinate indexing column.
 *    Thus the X and Y axes swap, and the vertical axis goes in the opposite
 *    direction than in the XY coordinate system.
 *
 * Index: images, image output
 *
 * -----------------------------------------------------------------------------
*/

int image_draw_circle
(
    KJB_image* ip,
    int        i,
    int        j,
    int        radius,
    int        line_width,
    int        r,
    int        g,
    int        b
)
{

    return image_draw_circle_2(ip, i, j, radius, line_width, (float)r, (float)g, (float)b);
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                             image_draw_circle_2
 *
 * Draws a circle on an image
 *
 * This routine draws a circle of radius 'radius' at location (i,j) onto an image. 
 * It is similar to image_draw_rectangle() except that it takes floating point RGB.
 *
 * Note:
 *    If displayed, image (i,j) generally results in the first coordinate
 *    indexing row number downwards, and the second coordinate indexing column.
 *    Thus the X and Y axes swap, and the vertical axis goes in the opposite
 *    direction than in the XY coordinate system.
 *
 * Index: images, image output
 *
 * -----------------------------------------------------------------------------
*/

int image_draw_circle_2
(
    KJB_image* ip,
    int        i,
    int        j,
    int        radius,
    int        line_width,
    float      f_r,
    float      f_g,
    float      f_b
)
{
    int x = 0;
    int y = radius;
    int p = (5 - radius * 4) / 4;
    /* int row = j;
    int col = i; */

    circle_points(ip, i, j, x, y, line_width, f_r, f_g, f_b);

    while(x < y)
    {
        x++;
        if(p < 0)
        {
            p += 2 * x + 1;
        }
        else
        {
            y--;
            p += 2 * (x - y) + 1;
        }
        circle_points(ip, i, j, x, y, line_width, f_r, f_g, f_b);
    }

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                             image_draw_disk
 *
 * Draws a disk (a filled circle) on an image
 *
 * This routine draws a disk of radius 'radius' at location (i,j) onto an image.
 *
 * See image_draw_disk_2() for a version that takes floating point RGB.
 *
 * Note:
 *    If displayed, image (i,j) generally results in the first coordinate
 *    indexing row number downwards, and the second coordinate indexing column.
 *    Thus the X and Y axes swap, and the vertical axis goes in the opposite
 *    direction than in the XY coordinate system.
 *
 * Index: images, image output
 *
 * -----------------------------------------------------------------------------
*/

int image_draw_disk
(
    KJB_image* ip,
    int        i,
    int        j,
    int        radius,
    int        r,
    int        g,
    int        b
)
{

    return image_draw_disk_2(ip, i, j, radius, (float)r, (float)g, (float)b);
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* draw a 1-pixel horizontal line */
static void draw_raster
(
    KJB_image* ip,
    int        i,
    int        j_min,
    int        j_max,
    float      f_r,
    float      f_g,
    float      f_b
)
{
    while (j_min <= j_max)
    {
        image_draw_point_2(ip, i, j_min++, 0, f_r, f_g, f_b);
    }
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                             image_draw_disk_2
 *
 * Draws a disk (a filled circle) on an image
 *
 * This routine draws a disk of radius 'radius' at location (i,j) onto an image.
 * It is similar to image_draw_disk() except that it takes floating point RGB.
 *
 * As usual, clipping is silent, not an error.
 *
 * Note:
 *    If displayed, image (i,j) generally results in the first coordinate
 *    indexing row number downwards, and the second coordinate indexing column.
 *    Thus the X and Y axes swap, and the vertical axis goes in the opposite
 *    direction than in the XY coordinate system.
 *
 * Index: images, image output
 *
 * -----------------------------------------------------------------------------
*/

int image_draw_disk_2
(
    KJB_image* ip,
    int        i,
    int        j,
    int        radius,
    float      f_r,
    float      f_g,
    float      f_b
)
{
    int row_c, col_reach, left, right, r2, num_rows, max_col;

    if (ip == NULL) return NO_ERROR;

    num_rows = ip->num_rows;
    max_col = ip->num_cols - 1; /* index of last valid column */

    /* coarse check for total clipping based on disk bounding box */
    if (    j+radius <= 0 || j-radius >= max_col
         || i+radius <= 0 || i-radius > num_rows )
    {
        return NO_ERROR;
    }

    /* Trace outline of circle using existing routine.  The fill code below is
       a little bit wonky due to the rounding of sqrt(), and this step gives
       superior aesthetics to the results.
       Also, this step is only linear time in the size of the radius, which is
       insignificant compared to the quadratic time of the fill code below.
     */
    ERE(image_draw_circle_2(ip, i, j, radius, 0, f_r, f_g, f_b));

    r2 = radius * radius;

    /* Advance row cursor from zero to just shy of the radius. */
    for (row_c = 0; row_c < radius; ++row_c)
    {
        /* Check whether horizontal central strip is clipped by T or B edge. */
        if (i+row_c < 0 || num_rows <= i-row_c) continue;

        /* Find column-reach: how long are the horiz. rasters for this iter. */
        col_reach = sqrt(r2 - row_c * row_c);

        /* Find extents of horz. rasters by clipping against L and R edges. */
        left = MAX_OF(0, j - col_reach);
        right = MIN_OF(max_col, j + col_reach);

        /* Check whether rasters are totally clipped by L or R edge. */
        if (right < 0 || max_col < left) continue;

        /* Draw rasters. */
        if (i - row_c < num_rows)
        {
            draw_raster(ip, i - row_c, left, right, f_r, f_g, f_b);
        }
        if (row_c != 0 && i + row_c >= 0)
        {
            draw_raster(ip, i + row_c, left, right, f_r, f_g, f_b);
        }
    }

    return NO_ERROR;
}


/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

static void circle_points
(
    KJB_image* ip,
    int        i,
    int        j,
    int        x,
    int        y,
    int        lw,
    float      r,
    float      g,
    float      b
)
{
    if(x == 0)
    {
        image_draw_point_2(ip, i, j + y, lw, r, g, b);
        image_draw_point_2(ip, i, j - y, lw, r, g, b);
        image_draw_point_2(ip, i + y, j, lw, r, g, b);
        image_draw_point_2(ip, i - y, j, lw, r, g, b);

        /* ip->pixels[i][j + y].r = r; ip->pixels[i][j + y].g = g; ip->pixels[i][j + y].b = b;
        ip->pixels[i][j - y].r = r; ip->pixels[i][j - y].g = g; ip->pixels[i][j - y].b = b;
        ip->pixels[i + y][j].r = r; ip->pixels[i + y][j].g = g; ip->pixels[i + y][j].b = b;
        ip->pixels[i - y][j].r = r; ip->pixels[i - y][j].g = g; ip->pixels[i - y][j].b = b; */
    }
    else if(x == y)
    {
        image_draw_point_2(ip, i + x, j + y, lw, r, g, b);
        image_draw_point_2(ip, i - x, j + y, lw, r, g, b);
        image_draw_point_2(ip, i + x, j - y, lw, r, g, b);
        image_draw_point_2(ip, i - x, j - y, lw, r, g, b);

        /* ip->pixels[i + x][j + y].r = r; ip->pixels[i + x][j + y].g = g; ip->pixels[i + x][j + y].b = b;
        ip->pixels[i - x][j + y].r = r; ip->pixels[i - x][j + y].g = g; ip->pixels[i - x][j + y].b = b;
        ip->pixels[i + x][j - y].r = r; ip->pixels[i + x][j - y].g = g; ip->pixels[i + x][j - y].b = b;
        ip->pixels[i - x][j - y].r = r; ip->pixels[i - x][j - y].g = g; ip->pixels[i - x][j - y].b = b; */
    }
    else if(x < y)
    {
        image_draw_point_2(ip, i + x, j + y, lw, r, g, b);
        image_draw_point_2(ip, i - x, j + y, lw, r, g, b);
        image_draw_point_2(ip, i + x, j - y, lw, r, g, b);
        image_draw_point_2(ip, i - x, j - y, lw, r, g, b);

        image_draw_point_2(ip, i + y, j + x, lw, r, g, b);
        image_draw_point_2(ip, i - y, j + x, lw, r, g, b);
        image_draw_point_2(ip, i + y, j - x, lw, r, g, b);
        image_draw_point_2(ip, i - y, j - x, lw, r, g, b);

        /* ip->pixels[i + x][j + y].r = r; ip->pixels[i + x][j + y].g = g; ip->pixels[i + x][j + y].b = b;
        ip->pixels[i - x][j + y].r = r; ip->pixels[i - x][j + y].g = g; ip->pixels[i - x][j + y].b = b;
        ip->pixels[i + x][j - y].r = r; ip->pixels[i + x][j - y].g = g; ip->pixels[i + x][j - y].b = b;
        ip->pixels[i - x][j - y].r = r; ip->pixels[i - x][j - y].g = g; ip->pixels[i - x][j - y].b = b;

        ip->pixels[i + y][j + x].r = r; ip->pixels[i + y][j + x].g = g; ip->pixels[i + y][j + x].b = b;
        ip->pixels[i - y][j + x].r = r; ip->pixels[i - y][j + x].g = g; ip->pixels[i - y][j + x].b = b;
        ip->pixels[i + y][j - x].r = r; ip->pixels[i + y][j - x].g = g; ip->pixels[i + y][j - x].b = b;
        ip->pixels[i - y][j - x].r = r; ip->pixels[i - y][j - x].g = g; ip->pixels[i - y][j - x].b = b; */
    }
}




/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

#ifdef __cplusplus
}
#endif

