
/* $Id: i_map.c 10922 2011-10-28 00:22:05Z kobus $ */

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

#include "i/i_map.h"
#include "i/i_stat.h"

#ifdef __cplusplus
extern "C" {
#endif

/* -------------------------------------------------------------------------- */

/* =============================================================================
 *                               pre_map_image
 *
 *
 * Index: images, image mapping, image transformation
 *
 * -----------------------------------------------------------------------------
*/

int pre_map_image
(
    KJB_image**      out_ipp,
    const KJB_image* in_ip,
    Matrix*          map_mp
)
{
    KJB_image* out_ip;
    int        num_rows, num_cols;
    Pixel*     in_pos;
    Pixel*     out_pos;
    int        i, j;
    Vector*    pixel_vp;
    Vector*    new_pixel_vp = NULL;


    if (    (map_mp->num_rows != 3)
         || ((map_mp->num_cols != 3) && (map_mp->num_cols != 4))
       )
    {
        SET_ARGUMENT_BUG();
        return ERROR;
    }

    num_rows = in_ip->num_rows;
    num_cols = in_ip->num_cols;

    ERE(get_target_image(out_ipp, num_rows, num_cols));
    out_ip = *out_ipp;

    NRE(pixel_vp = create_vector(map_mp->num_cols));

    for (i=0; i<num_rows; i++)
    {
        in_pos  = in_ip->pixels[ i ];
        out_pos = out_ip->pixels[ i ];

        for (j=0; j<num_cols; j++)
        {
            (pixel_vp->elements)[ 0 ] = in_pos->r;
            (pixel_vp->elements)[ 1 ] = in_pos->g;
            (pixel_vp->elements)[ 2 ] = in_pos->b;

            if (map_mp->num_cols == 4)
            {
                (pixel_vp->elements)[ 3 ] = 1.0;
            }

            ERE(multiply_matrix_and_vector(&new_pixel_vp, map_mp, pixel_vp));

            out_pos->r = (new_pixel_vp->elements)[ 0 ];
            out_pos->g = (new_pixel_vp->elements)[ 1 ];
            out_pos->b = (new_pixel_vp->elements)[ 2 ];

            out_pos->extra.invalid = in_pos->extra.invalid;

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

            out_pos++;
            in_pos++;
        }
    }

    free_vector(pixel_vp);
    free_vector(new_pixel_vp);

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */


/* =============================================================================
 *                               post_map_image
 *
 *
 * Index: images, image mapping, image transformation
 *
 * -----------------------------------------------------------------------------
*/

int post_map_image
(
    KJB_image**      out_ipp,
    const KJB_image* in_ip,
    Matrix*          map_mp
)
{
    KJB_image* out_ip;
    int        num_rows, num_cols;
    Pixel*     in_pos;
    Pixel*     out_pos;
    int        i, j;
    Vector*    pixel_vp;
    Vector*    new_pixel_vp = NULL;


    if (    (map_mp->num_cols != 3)
         || ((map_mp->num_rows != 3) && (map_mp->num_rows != 4))
       )
    {
        SET_ARGUMENT_BUG();
        return ERROR;
    }

    num_rows = in_ip->num_rows;
    num_cols = in_ip->num_cols;

#ifdef TEST
    if (map_mp->num_rows == 4)
    {
        UNTESTED_CODE();
    }
#endif

    ERE(get_target_image(out_ipp, num_rows, num_cols));
    out_ip = *out_ipp;

    NRE(pixel_vp = create_vector(map_mp->num_rows));

    for (i=0; i<num_rows; i++)
    {
        in_pos  = in_ip->pixels[ i ];
        out_pos = out_ip->pixels[ i ];

        for (j=0; j<num_cols; j++)
        {
            (pixel_vp->elements)[ 0 ] = in_pos->r;
            (pixel_vp->elements)[ 1 ] = in_pos->g;
            (pixel_vp->elements)[ 2 ] = in_pos->b;

            if (map_mp->num_rows == 4)
            {
                (pixel_vp->elements)[ 3 ] = 1.0;
            }

            ERE(multiply_vector_and_matrix(&new_pixel_vp, pixel_vp, map_mp));

            out_pos->r = (new_pixel_vp->elements)[ 0 ];
            out_pos->g = (new_pixel_vp->elements)[ 1 ];
            out_pos->b = (new_pixel_vp->elements)[ 2 ];

            out_pos->extra.invalid = in_pos->extra.invalid;

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

            out_pos++;
            in_pos++;
        }
    }

    free_vector(pixel_vp);
    free_vector(new_pixel_vp);

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */


/* =============================================================================
 *                             post_map_projected_image
 *
 *
 * Index: images, image mapping, image transformation
 *
 * -----------------------------------------------------------------------------
*/

int post_map_projected_image
(
    KJB_image**      out_ipp,
    const KJB_image* in_ip,
    Matrix*          map_mp
)
{
    KJB_image* out_ip;
    int        num_rows, num_cols;
    Pixel*     in_pos;
    Pixel*     out_pos;
    int        i, j;
    double     red, blue, green;
    Vector*    pixel_vp;
    Vector*    new_pixel_vp = NULL;


    if ((map_mp->num_rows != 2) || (map_mp->num_cols != 2) )
    {
        SET_ARGUMENT_BUG();
        return ERROR;
    }

    num_rows = in_ip->num_rows;
    num_cols = in_ip->num_cols;

    ERE(get_target_image(out_ipp, num_rows, num_cols));
    out_ip = *out_ipp;

    NRE(pixel_vp = create_vector(2));

    for (i=0; i<num_rows; i++)
    {
        in_pos  = in_ip->pixels[ i ];
        out_pos = out_ip->pixels[ i ];

        for (j=0; j<num_cols; j++)
        {
            red   = in_pos->r;
            green = in_pos->g;
            blue  = in_pos->b;


            if (blue < 1.0) blue = 1.0;

            (pixel_vp->elements)[ 0 ] = red / blue;
            (pixel_vp->elements)[ 1 ] = green / blue;

            ERE(multiply_vector_and_matrix(&new_pixel_vp, pixel_vp, map_mp));

            out_pos->r = blue * (new_pixel_vp->elements)[ 0 ];
            out_pos->g = blue * (new_pixel_vp->elements)[ 1 ];
            out_pos->b = blue;

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

            out_pos++;
            in_pos++;
        }
    }

    free_vector(pixel_vp);
    free_vector(new_pixel_vp);

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

int scale_image_by_max(KJB_image** ipp, const KJB_image* ip)
{
    int   i, j, num_rows, num_cols;
    float max_rgb  = FLT_MOST_NEGATIVE;
    float factor;


    ERE(kjb_copy_image(ipp, ip)); 

    if (ip == NULL) 
    {
        return NO_ERROR;
    }

    num_rows = ip->num_rows;
    num_cols = ip->num_cols;

    for (i = 0; i < num_rows; i++)
    {
        for (j = 0; j < num_cols; j++)
        {
            if (ip->pixels[ i ][ j ].extra.invalid.pixel == VALID_PIXEL) 
            {
                max_rgb = MAX_OF(max_rgb, ip->pixels[ i ][ j ].r); 
                max_rgb = MAX_OF(max_rgb, ip->pixels[ i ][ j ].g); 
                max_rgb = MAX_OF(max_rgb, ip->pixels[ i ][ j ].b); 
            }
        }
    }

    /* If we did not find any valid pixels, then do nothing. */

    if (max_rgb < FLT_HALF_MOST_NEGATIVE)
    {
        return NO_ERROR;
    }

    factor = 255.0f / max_rgb; 

    for (i = 0; i < num_rows; i++)
    {
        for (j = 0; j < num_cols; j++)
        {
            if ((*ipp)->pixels[ i ][ j ].extra.invalid.pixel == VALID_PIXEL) 
            {
                (*ipp)->pixels[ i ][ j ].r *= factor;  
                (*ipp)->pixels[ i ][ j ].g *= factor;  
                (*ipp)->pixels[ i ][ j ].b *= factor;  
            }
        }
    }

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

int ow_scale_image_by_max(KJB_image* ip)
{
    int   i, j, num_rows, num_cols;
    float max_rgb  = FLT_MOST_NEGATIVE;
    float factor;


    if (ip == NULL) 
    {
        return NO_ERROR;
    }

    num_rows = ip->num_rows;
    num_cols = ip->num_cols;

    for (i = 0; i < num_rows; i++)
    {
        for (j = 0; j < num_cols; j++)
        {
            if (ip->pixels[ i ][ j ].extra.invalid.pixel == VALID_PIXEL) 
            {
                max_rgb = MAX_OF(max_rgb, ip->pixels[ i ][ j ].r); 
                max_rgb = MAX_OF(max_rgb, ip->pixels[ i ][ j ].g); 
                max_rgb = MAX_OF(max_rgb, ip->pixels[ i ][ j ].b); 
            }
        }
    }

    /* If we did not find any valid pixels, then do nothing. */

    if (max_rgb < FLT_HALF_MOST_NEGATIVE)
    {
        return NO_ERROR;
    }

    factor = 255.0f / max_rgb; 

    for (i = 0; i < num_rows; i++)
    {
        for (j = 0; j < num_cols; j++)
        {
            if (ip->pixels[ i ][ j ].extra.invalid.pixel == VALID_PIXEL) 
            {
                ip->pixels[ i ][ j ].r *= factor;  
                ip->pixels[ i ][ j ].g *= factor;  
                ip->pixels[ i ][ j ].b *= factor;  
            }
        }
    }

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

int adjust_image_range(KJB_image** ipp, const KJB_image* ip)
{
    int   i, j, num_rows, num_cols;
    float max_rgb  = FLT_MOST_NEGATIVE;
    float min_rgb  = FLT_MOST_POSITIVE;
    float diff; 
    float factor;


    ERE(kjb_copy_image(ipp, ip)); 

    if (ip == NULL) 
    {
        return NO_ERROR;
    }

    num_rows = ip->num_rows;
    num_cols = ip->num_cols;

    for (i = 0; i < num_rows; i++)
    {
        for (j = 0; j < num_cols; j++)
        {
            if (ip->pixels[ i ][ j ].extra.invalid.pixel == VALID_PIXEL) 
            {
                max_rgb = MAX_OF(max_rgb, ip->pixels[ i ][ j ].r); 
                max_rgb = MAX_OF(max_rgb, ip->pixels[ i ][ j ].g); 
                max_rgb = MAX_OF(max_rgb, ip->pixels[ i ][ j ].b); 

                min_rgb = MIN_OF(min_rgb, ip->pixels[ i ][ j ].r); 
                min_rgb = MIN_OF(min_rgb, ip->pixels[ i ][ j ].g); 
                min_rgb = MIN_OF(min_rgb, ip->pixels[ i ][ j ].b); 
            }
        }
    }

    /* If we did not find any valid pixels, then do nothing. */

    if (    (max_rgb < FLT_HALF_MOST_NEGATIVE)
         || (min_rgb > FLT_HALF_MOST_POSITIVE)
       )
    {
        return NO_ERROR;
    }

    diff = max_rgb - min_rgb;

    if (diff <= 0.0f) return NO_ERROR;

    factor = 255.0f / diff; 

    for (i = 0; i < num_rows; i++)
    {
        for (j = 0; j < num_cols; j++)
        {
            if ((*ipp)->pixels[ i ][ j ].extra.invalid.pixel == VALID_PIXEL) 
            {
                (*ipp)->pixels[ i ][ j ].r -= min_rgb;
                (*ipp)->pixels[ i ][ j ].r *= factor;  

                (*ipp)->pixels[ i ][ j ].g -= min_rgb;
                (*ipp)->pixels[ i ][ j ].g *= factor;  

                (*ipp)->pixels[ i ][ j ].b -= min_rgb;
                (*ipp)->pixels[ i ][ j ].b *= factor;  
            }
        }
    }

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

#ifdef __cplusplus
}
#endif

