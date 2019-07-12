
/* $Id: i3_draw_hull.c 4727 2009-11-16 20:53:54Z kobus $ */


/*
     Copyright (c) 1994-2008 by Kobus Barnard (author).

     Personal and educational use of this code is granted, provided
     that this header is kept intact, and that the authorship is not
     misrepresented. Commercial use is not permitted.
*/

#include "i/i_incl.h"     /* Only safe as first include in a ".c" file. */
#include "i3/i3_draw_hull.h"

#ifdef __cplusplus
extern "C" {
#endif

/* -------------------------------------------------------------------------- */

/* =============================================================================
 *                             image_draw_hull_interior
 *
 * Draws a convex hull on an image
 *
 * This routine sets the pixels of "ip" which are inside the convex
 * hull "hp" to (r,g,b).
 *
 * Index: images, image output
 *
 * -----------------------------------------------------------------------------
*/

int image_draw_hull_interior
(
    KJB_image*  ip,
    const Hull* hp,
    int         r,
    int         g,
    int         b
)
{
    static double test_vector_elements[ 2 ];
    static Vector  test_vector = {2, 2, test_vector_elements};
    static Vector* test_vp     = &test_vector;
    int            i, j;
    float          f_r         = r;
    float          f_g         = g;
    float          f_b         = b;
    int            i_min;
    int            i_max;
    int            j_min;
    int            j_max;


    if ((ip == NULL) || (hp == NULL)) return NO_ERROR;

    i_min = MAX_OF(0, (int)hp->min_x);
    i_max = MIN_OF(ip->num_rows - 1, (int)hp->max_x);

    j_min = MAX_OF(0, (int)hp->min_y);
    j_max = MIN_OF(ip->num_cols - 1, (int)hp->max_y);

    for (i = i_min; i <= i_max; i++)
    {
        for (j = j_min; j <= j_max; j++)
        {
            int is_inside;

            test_vp->elements[ 0 ] = (double)i;
            test_vp->elements[ 1 ] = (double)j;

            ERE(is_inside = is_point_inside_hull(hp, test_vp));

            if (is_inside)
            {
                ip->pixels[ i ][ j ].r = f_r;
                ip->pixels[ i ][ j ].g = f_g;
                ip->pixels[ i ][ j ].b = f_b;
            }
        }
    }

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                             image_draw_hull_boundary
 *
 * Draws the boundary of a convex hull on an image
 *
 * This routine sets the pixels of "ip" which are on the boundary of the convex
 * hull "hp" to (r,g,b).
 *
 * Index: images, image output
 *
 * -----------------------------------------------------------------------------
*/

int image_draw_hull_boundary
(
    KJB_image*  ip,
    const Hull* hp,
    int         r,
    int         g,
    int         b
)
{
    int i;


    if ((ip == NULL) || (hp == NULL)) return NO_ERROR;

    for (i = 0; i < hp->num_facets; i++)
    {
        Matrix* facet_mp = hp->facets->elements[ i ];

        ASSERT(facet_mp->num_rows == 2);
        ASSERT(facet_mp->num_cols == 2);

        ERE(image_draw_contour(ip, facet_mp, 1, r, g, b));
    }

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

#ifdef __cplusplus
}
#endif

