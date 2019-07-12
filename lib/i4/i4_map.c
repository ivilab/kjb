
/* $Id: i4_map.c 5837 2010-05-02 21:56:10Z ksimek $ */

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

#include "i4/i4_gen.h"     /* Only safe as first include in a ".c" file. */
#include "n/n_invert.h"
#include "i/i_stat.h"
#include "i/i_map.h"
#include "i4/i4_map.h"


#ifdef __cplusplus
extern "C" {
#endif

/* -------------------------------------------------------------------------- */

/* =============================================================================
 *                          change_basis_post_map_image
 *
 *
 * Index: images, image mapping, image transformation
 *
 * -----------------------------------------------------------------------------
*/

int change_basis_post_map_image
(
    KJB_image**      out_ipp,
    const KJB_image* in_ip,
    Matrix*          change_basis_mp,
    Matrix*          map_mp
)
{
    Matrix* temp_mp             = NULL;
    Matrix* inv_change_basis_mp = NULL;
    Matrix* prod_map_mp         = NULL;
    int     result;


    if (    (map_mp->num_rows != map_mp->num_cols)
         || (change_basis_mp->num_rows != change_basis_mp->num_cols)
         || (change_basis_mp->num_rows != map_mp->num_rows)
       )
    {
        SET_ARGUMENT_BUG();
        return ERROR;
    }

    ERE(get_matrix_inverse(&inv_change_basis_mp, change_basis_mp));

    result = multiply_matrices(&temp_mp, change_basis_mp, map_mp);

    if (result == ERROR)
    {
        free_matrix(inv_change_basis_mp);
        return ERROR;
    }

    result = multiply_matrices(&prod_map_mp, temp_mp, inv_change_basis_mp);

    if (result == ERROR)
    {
        free_matrix(inv_change_basis_mp);
        free_matrix(temp_mp);
        return ERROR;
    }

    result = post_map_image(out_ipp, in_ip, prod_map_mp);

    free_matrix(inv_change_basis_mp);
    free_matrix(temp_mp);

    return result;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */


/* =============================================================================
 *                       change_basis_post_map_projected_image
 *
 *
 *
 * Index: images, image mapping, image transformation
 *
 * -----------------------------------------------------------------------------
*/

int change_basis_post_map_projected_image
(
    KJB_image**      out_ipp,
    const KJB_image* in_ip,
    Matrix*          change_basis_mp,
    Matrix*          map_mp
)
{
    KJB_image* out_ip;
    Pixel*     in_pos;
    Pixel*     out_pos;
    int        i, j, num_rows, num_cols;
    double     red, blue, green;
    double     factor;
    Vector*    pixel_vp;
    Vector*    new_pixel_vp = NULL;
    Vector*    pr_pixel_vp;
    Vector*    pr_new_pixel_vp     = NULL;
    Matrix*    inv_change_basis_mp = NULL;
    Vector*    ave_rgb_vp          = NULL;


    num_rows = in_ip->num_rows;
    num_cols = in_ip->num_cols;

    ERE(get_target_image(out_ipp, num_rows, num_cols));
    out_ip = *out_ipp;

    ERE(get_matrix_inverse(&inv_change_basis_mp, change_basis_mp));

    pixel_vp        = create_vector(3);
    pr_pixel_vp     = create_vector(2);

    if (    (get_ave_rgb(&ave_rgb_vp, in_ip) == ERROR)
         || (inv_change_basis_mp == NULL)
         || (pixel_vp == NULL)
         || (pr_pixel_vp == NULL)
       )
    {
        free_vector(ave_rgb_vp);
        free_vector(pixel_vp);
        free_vector(pr_pixel_vp);
        free_matrix(inv_change_basis_mp);
        return ERROR;
    }

    factor = (ave_rgb_vp->elements)[ 2 ];
    free_vector(ave_rgb_vp);

    for (i=0; i<num_rows; i++)
    {
        in_pos  = in_ip->pixels[ i ];
        out_pos = out_ip->pixels[ i ];

        for (j=0; j<num_cols; j++)
        {
            (pixel_vp->elements)[ 0 ] = in_pos->r;
            (pixel_vp->elements)[ 1 ] = in_pos->g;
            (pixel_vp->elements)[ 2 ] = in_pos->b;

            /*
            // If this fails, we leak memory. However, it should not fail, so we
            // won't deal with it.
            */
            ERE(multiply_vector_and_matrix(&new_pixel_vp, pixel_vp,
                                           change_basis_mp));

            red   = (new_pixel_vp->elements)[ 0 ];
            green = (new_pixel_vp->elements)[ 1 ];
            blue  = (new_pixel_vp->elements)[ 2 ];

            if (blue < 1.0) blue = 1.0;

            (pr_pixel_vp->elements)[ 0 ] = red / blue;
            (pr_pixel_vp->elements)[ 1 ] = green / blue;

            /*
            // If this fails, we leak memory. However, it should not fail, so we
            // won't deal with it.
            */
            ERE(multiply_vector_and_matrix(&pr_new_pixel_vp, pr_pixel_vp,
                                           map_mp));

            (pixel_vp->elements)[ 0 ] = (pr_new_pixel_vp->elements)[ 0 ];
            (pixel_vp->elements)[ 1 ] = (pr_new_pixel_vp->elements)[ 1 ];
            (pixel_vp->elements)[ 2 ] = 1.0;

            /*
            // If this fails, we leak memory. However, it should not fail, so we
            // won't deal with it.
            */
            ERE(multiply_vector_and_matrix(&new_pixel_vp, pixel_vp,
                                           inv_change_basis_mp));

            out_pos->r = factor * (new_pixel_vp->elements)[ 0 ];
            out_pos->g = factor * (new_pixel_vp->elements)[ 1 ];
            out_pos->b = factor * (new_pixel_vp->elements)[ 2 ];

            out_pos->extra.invalid = in_pos->extra.invalid;

            out_pos->extra.invalid.r |= out_pos->extra.invalid.b;
            out_pos->extra.invalid.g |= out_pos->extra.invalid.b;

            if (out_pos->r < FLT_ZERO)
            {
                out_pos->r = FLT_ZERO;
                out_pos->extra.invalid.r |= TRUNCATED_PIXEL;
            }

            if (out_pos->g < FLT_ZERO)
            {
                out_pos->g = FLT_ZERO;
                out_pos->extra.invalid.g |= TRUNCATED_PIXEL;
            }

            if (out_pos->b < FLT_ZERO)
            {
                out_pos->b = FLT_ZERO;
                out_pos->extra.invalid.b |= TRUNCATED_PIXEL;
            }

            out_pos->extra.invalid.pixel =
                 out_pos->extra.invalid.r | out_pos->extra.invalid.g | out_pos->extra.invalid.b;

            in_pos++;
            out_pos++;
        }
    }

    free_vector(pr_pixel_vp);
    free_vector(pr_new_pixel_vp);
    free_vector(pixel_vp);
    free_vector(new_pixel_vp);
    free_matrix(inv_change_basis_mp);

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

#ifdef __cplusplus
}
#endif

