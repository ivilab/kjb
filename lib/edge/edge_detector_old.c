
/* $Id: edge_detector.c 5725 2010-03-18 01:31:44Z ksimek $ */

/**
 * @file
 *
 * @author Joseph Schlecht
 *
 * @brief Definitions for the edge detector.
 *
 * Copyright (c) 2003-2008 by members of University of Arizona Computer Vision
 * group. For use outside the University of Arizona Computer Vision group
 * please contact Joseph Schlecht.
 */

#include "edge/edge_detector_old.h"

#include "l/l_sys_mal.h"
#include "l/l_math.h"
#include "m/m_matrix.h"
#include "m/m_mat_arith.h"
#include "i/i_colour.h"

#include <stdlib.h>
#include <math.h>

#ifdef __cplusplus
extern "C" {
#endif

#define Edge_point Edge_point_DEPRECATED

/* -------------------------------------------------------------------------- */

typedef struct Gradient
{
    float dx;
    float dy;
    float mag;
    int marked;
}
Gradient;

typedef struct JWS_Mask
{
    Matrix* mp;
    double sigma;
    int width;
    int size;
    double sum;
}
JWS_Mask;

/**
 * @brief Creates a 2-Dimensional Gaussian mask with a partial derivative in
 * the x-direction.
 */
static void create_2D_gaussian_mask_dx
(
    JWS_Mask** maskpp,
    double sigma
)
{
    JWS_Mask*  maskp;
    int r, c;
    int mask_center;
    double dist_sqr;
    double sigma_sqr;
    double gauss;

    if ( *maskpp == NULL )
    {
        *maskpp = (JWS_Mask*) kjb_malloc( sizeof( JWS_Mask ) );
        (*maskpp)->mp = NULL;
    }
    maskp = *maskpp;

    maskp->sigma = sigma;
    maskp->width = kjb_rint(1.0 + 2.0 * sigma);
    maskp->size  = 1 + 2 * maskp->width;
    maskp->sum   = 0.0;

    get_zero_matrix( &maskp->mp, maskp->size, maskp->size );

    sigma_sqr = sigma * sigma;

    mask_center = maskp->size / 2;

    for ( r = 0; r < maskp->size; r++ )
    {
        for ( c = 0; c < maskp->size; c++ )
        {
            dist_sqr = (r - mask_center)*(r - mask_center) +
                                            (c - mask_center)*(c - mask_center);

            gauss = -1 * (c - mask_center) * exp( -0.5 * dist_sqr / sigma_sqr );
            maskp->mp->elements[ r ][ c ] = gauss;
            maskp->sum += gauss;
        }
    }

    ow_divide_matrix_by_scalar( maskp->mp, 2 * M_PI * sigma_sqr * sigma_sqr );
}

/**
 * @brief Creates a 2-Dimensional Gaussian mask with a partial derivative in
 * the x-direction.
 */
static void create_2D_gaussian_mask_dy
(
    JWS_Mask** maskpp,
    double sigma
)
{
    JWS_Mask*  maskp;
    int    r, c;
    int    mask_center;
    double dist_sqr;
    double sigma_sqr;
    double gauss;

    if (*maskpp == NULL)
    {
        *maskpp = (JWS_Mask*) kjb_malloc( sizeof( JWS_Mask ) );
        (*maskpp)->mp = NULL;
    }
    maskp = *maskpp;

    maskp->sigma = sigma;
    maskp->width = kjb_rint(1.0 + 2.0 * sigma);
    maskp->size  = 1 + 2 * maskp->width;
    maskp->sum   = 0.0;

    get_zero_matrix( &maskp->mp, maskp->size, maskp->size );

    sigma_sqr = sigma * sigma;

    mask_center = maskp->size / 2;

    for ( r = 0; r < maskp->size; r++ )
    {
        for ( c = 0; c < maskp->size; c++ )
        {
            dist_sqr =(r - mask_center)*(r - mask_center) +
                                            (c - mask_center)*(c - mask_center);

            gauss = -1 * (r - mask_center) * exp( -0.5 * dist_sqr / sigma_sqr );
            maskp->mp->elements[ r ][ c ] = gauss;
            maskp->sum += gauss;
        }
    }

    ow_divide_matrix_by_scalar( maskp->mp, 2 * M_PI * sigma_sqr * sigma_sqr );
}

/**
 * @brief Zeros all pixels that are within @em width of the border of the
 * gradient map.
 */
static void zero_gradient_map_border
(
    Gradient** gradient_map,
    int num_rows,
    int num_cols,
    int width
)
{
    Gradient* gradient;
    int i, j;

    for ( i = 0; i < num_rows; i++ )
    {
        if ( i < width || i > num_rows - width - 1 )
        {
            for ( j = 0; j < num_cols; j++ )
            {
                gradient = &gradient_map[ i ][ j ];
                gradient->dx = 0.0f;
                gradient->dy = 0.0f;
                gradient->mag = 0.0f;
                gradient->marked = FALSE;
            }
        }
        else
        {
            for ( j = 0; j < width; j++ )
            {
                gradient = &gradient_map[ i ][ j ];
                gradient->dx = 0.0f;
                gradient->dy = 0.0f;
                gradient->mag = 0.0f;
                gradient->marked = FALSE;

                gradient = &gradient_map[ i ][ num_cols - j - 1 ];
                gradient->dx = 0.0f;
                gradient->dy = 0.0f;
                gradient->mag = 0.0f;
                gradient->marked = FALSE;
            }
        }
    }
}

/**
 * @brief Zeros all pixels that are within @em width of the border of the
 * image.
 */
static void zero_image_border( KJB_image* img, int width )
{
    int num_rows = img->num_rows;
    int num_cols = img->num_cols;
    int i, j;

    for ( i = 0; i < num_rows; i++ )
    {
        if ( i < width || i > num_rows - width - 1 )
        {
            for ( j = 0; j < num_cols; j++ )
            {
                img->pixels[ i ][ j ].r = 0;
                img->pixels[ i ][ j ].g = 0;
                img->pixels[ i ][ j ].b = 0;
            }
        }
        else
        {
            for ( j = 0; j < width; j++ )
            {
                img->pixels[ i ][ j ].r = 0;
                img->pixels[ i ][ j ].g = 0;
                img->pixels[ i ][ j ].b = 0;
                img->pixels[ i ][ num_cols - j - 1 ].r = 0;
                img->pixels[ i ][ num_cols - j - 1 ].g = 0;
                img->pixels[ i ][ num_cols - j - 1 ].b = 0;
            }
        }
    }
}

/**
 * @brief Convolves @em img with the two masks and creates a gradient map.
 */
static void convolve_and_create_image_gradient_map
(
    Gradient*** gradient_map,
    KJB_image* img,
    JWS_Mask* gauss_dx_maskp,
    JWS_Mask* gauss_dy_maskp
)
{
    int        i, j;
    int        mi, mj;
    int        m, n;
    double     r_sum_x, g_sum_x, b_sum_x;
    double     r_sum_y, g_sum_y, b_sum_y;
    double     weight_x, weight_y;
    int num_rows = img->num_rows;
    int num_cols = img->num_cols;

    /* FOR USE IN CREATING VISUALIZATION OF THE GRADIENT
    float      red_v[2] = {0.0f, -1.0f};
    float      green_v[2] = {-0.866025404f, 0.5f};
    float      blue_v[2] = {0.866025404f, 0.5f};
    float      dotprod;
    */

    ow_make_black_and_white_image( img );


    if ( *gradient_map == NULL )
    {
        *gradient_map = (Gradient**) kjb_calloc( num_rows,
                                                          sizeof( Gradient* ) );
        for ( i = 0; i < num_rows; i++ )
        {
            (*gradient_map)[ i ] = (Gradient*) kjb_calloc(num_cols,
                                                              sizeof(Gradient));
        }
    }

    {
        KJB_image* img_dxdy = kjb_create_image( num_rows, num_cols );

        for ( i = 0; i < num_rows; i++ )
        {
            for ( j = 0; j < num_cols; j++ )
            {
                r_sum_x = g_sum_x = b_sum_x = 0.0;
                r_sum_y = g_sum_y = b_sum_y = 0.0;

                for ( mi = 0; mi < gauss_dx_maskp->size; mi++ )
                {
                    for ( mj = 0; mj < gauss_dx_maskp->size; mj++ )
                    {
                        m = i + mi - gauss_dx_maskp->width;
                        n = j + mj - gauss_dx_maskp->width;

                        if ( (m >= 0) && (m < num_rows) &&
                             (n >= 0) && (n < num_cols) )
                        {
                            weight_x = gauss_dx_maskp->mp->elements[ mi ][ mj ];

                            r_sum_x += img->pixels[ m ][ n ].r * weight_x;
                            g_sum_x += img->pixels[ m ][ n ].g * weight_x;
                            b_sum_x += img->pixels[ m ][ n ].b * weight_x;

                            weight_y = gauss_dy_maskp->mp->elements[ mi ][ mj ];

                            r_sum_y += img->pixels[ m ][ n ].r * weight_y;
                            g_sum_y += img->pixels[ m ][ n ].g * weight_y;
                            b_sum_y += img->pixels[ m ][ n ].b * weight_y;
                        }
                    }
                }

                /* the image is grayscale... so this is ok */
                (*gradient_map)[ i ][ j ].dx = r_sum_x;
                (*gradient_map)[ i ][ j ].dy = r_sum_y;
                (*gradient_map)[ i ][ j ].mag = sqrt( r_sum_x * r_sum_x + r_sum_y *
                                                      r_sum_y );
                (*gradient_map)[i][j].marked = FALSE;

                if ( (*gradient_map)[ i ][ j ].mag != 0.0f )
                {
                    (*gradient_map)[ i ][ j ].dx /= (*gradient_map)[ i ][ j ].mag;
                    (*gradient_map)[ i ][ j ].dy /= (*gradient_map)[ i ][ j ].mag;
                }

                (*gradient_map)[ i ][ j ].mag = 255 * sqrt( r_sum_x * r_sum_x +
                                                            r_sum_y * r_sum_y ) / 50.0f;

                img_dxdy->pixels[ i ][ j ].r = (*gradient_map)[ i ][ j ].mag;
                img_dxdy->pixels[ i ][ j ].g = (*gradient_map)[ i ][ j ].mag;
                img_dxdy->pixels[ i ][ j ].b = (*gradient_map)[ i ][ j ].mag;

                /* FOR USE IN CREATING VISUALIZATION OF THE GRADIENT
                   dotprod = (*gradient_map)[i][j].dx * red_v[0] +
                   (*gradient_map)[i][j].dy * red_v[1];
                   img_dxdy->pixels[i][j].r = 255.0f * (*gradient_map)[i][j].mag /
                   50.0f * (3 * dotprod + 1) / 4.0f;

                   dotprod = (*gradient_map)[i][j].dx * green_v[0] +
                   (*gradient_map)[i][j].dy * green_v[1];
                   img_dxdy->pixels[i][j].g = 255.0f * (*gradient_map)[i][j].mag /
                   50.0f * (3 * dotprod + 1) / 4.0f;

                   dotprod = (*gradient_map)[i][j].dx * blue_v[0] +
                   (*gradient_map)[i][j].dy * blue_v[1];
                   img_dxdy->pixels[i][j].b = 255.0f * (*gradient_map)[i][j].mag /
                   50.0f * (3 * dotprod + 1) / 4.0f;
                 */

                img_dxdy->pixels[ i ][ j ].extra.invalid = img->pixels[ i ][ j ].extra.invalid;
            }
        }

        kjb_copy_image( &img, img_dxdy );
        kjb_free_image( img_dxdy );
    }

    zero_gradient_map_border( *gradient_map, num_rows, num_cols,
                              gauss_dx_maskp->width );
    zero_image_border( img, gauss_dx_maskp->width );
}

/**
 * @brief Get the next gradient along the specified gradient.
 */
static Gradient* follow_gradient
(
    int* gradient_row,
    int* gradient_col,
    float direction_x,
    float direction_y,
    Gradient** gradient_map
)
{
    static int neighbor_positions[ 8 ][ 2 ] = { {  1,  0 },
                                                {  1,  1 },
                                                {  0,  1 },
                                                { -1,  1 },
                                                { -1,  0 },
                                                { -1, -1 },
                                                {  0, -1 },
                                                {  1, -1 } };
    static float neighbor_vectors[ 8 ][ 2 ] = { {  1, 0 },
                                                {  0.70710678f,  0.70710678f },
                                                {  0,            1 },
                                                { -0.70710678f,  0.70710678f },
                                                { -1,            0 },
                                                { -0.70710678f, -0.70710678f },
                                                {  0,           -1 },
                                                {  0.70710678f, -0.70710678f }};

    int i;
    int max_neighbor = 0;
    float max_dotprod = -1.0f;
    Gradient* gradient = &gradient_map[ *gradient_row ][ *gradient_col ];

    for ( i = 0; i < 8; i++ )
    {
        float dotprod = direction_x * neighbor_vectors[ i ][ 0 ] +
                        direction_y * neighbor_vectors[ i ][ 1 ];

        /* Should have some stochastic tie-breaker here. */
        if ( dotprod >= max_dotprod )
        {
            max_dotprod = dotprod;
            max_neighbor = i;
        }
    }

    *gradient_row += neighbor_positions[ max_neighbor ][ 1 ];
    *gradient_col += neighbor_positions[ max_neighbor ][ 0 ];
    gradient = &gradient_map[ *gradient_row ][ *gradient_col ];

    return gradient;
}

/**
 * @brief Non-maximal suppression.
 */
static Gradient* get_maximal_along_gradient
(
    int* gradient_row,
    int* gradient_col,
    Gradient** gradient_map
)
{
    Gradient* maximal_gradient = NULL;
    Gradient* curr_gradient;
    int curr_gradient_row;
    int curr_gradient_col;
    Gradient* next_gradient;
    int next_gradient_row;
    int next_gradient_col;
    Gradient* prev_gradient;
    int prev_gradient_row;
    int prev_gradient_col;

    curr_gradient_row = *gradient_row;
    curr_gradient_col = *gradient_col;
    curr_gradient = &gradient_map[curr_gradient_row][curr_gradient_col];

    prev_gradient_row = curr_gradient_row;
    prev_gradient_col = curr_gradient_col;
    prev_gradient = follow_gradient( &prev_gradient_row, &prev_gradient_col,
                                     -(curr_gradient->dx), -(curr_gradient->dy),
                                     gradient_map );

    next_gradient_row = curr_gradient_row;
    next_gradient_col = curr_gradient_col;
    next_gradient = follow_gradient( &next_gradient_row, &next_gradient_col,
                                     curr_gradient->dx, curr_gradient->dy,
                                     gradient_map );

    while ( maximal_gradient == NULL )
    {
        if ( curr_gradient->mag >= next_gradient->mag &&
             curr_gradient->mag >= prev_gradient->mag )
        {
            maximal_gradient = curr_gradient;
        }
        else if ( curr_gradient->mag >= prev_gradient->mag )
        {
            curr_gradient_row = next_gradient_row;
            curr_gradient_col = next_gradient_col;
            curr_gradient = next_gradient;
        }
        else
        {
            curr_gradient_row = prev_gradient_row;
            curr_gradient_col = prev_gradient_col;
            curr_gradient = prev_gradient;
        }

        prev_gradient_row = curr_gradient_row;
        prev_gradient_col = curr_gradient_col;
        prev_gradient = follow_gradient( &prev_gradient_row, &prev_gradient_col,
                                     -(curr_gradient->dx), -(curr_gradient->dy),
                                         gradient_map );

        next_gradient_row = curr_gradient_row;
        next_gradient_col = curr_gradient_col;
        next_gradient = follow_gradient( &next_gradient_row, &next_gradient_col,
                                         curr_gradient->dx, curr_gradient->dy,
                                         gradient_map );
    }

    *gradient_row = curr_gradient_row;
    *gradient_col = curr_gradient_col;

    return maximal_gradient;
}

/**
 * @brief Hysterisis to the right.
 */
static void follow_edge_right
(
    float direction_x,
    float direction_y,
    int gradient_row,
    int gradient_col,
    Gradient** gradient_map,
    double end_threshold
)
{
    Gradient* gradient = follow_gradient( &gradient_row, &gradient_col,
                                       direction_x, direction_y, gradient_map );

    gradient = get_maximal_along_gradient( &gradient_row, &gradient_col,
                                                                 gradient_map );

    if ( !gradient->marked && gradient->mag >= end_threshold )
    {
        gradient->marked = TRUE;

        direction_x = gradient->dy;
        direction_y = - gradient->dx;
        follow_edge_right( direction_x, direction_y, gradient_row, gradient_col,
                                                  gradient_map, end_threshold );
    }
}

/**
 * @brief Hysterisis to the left.
 */
static void follow_edge_left
(
    float direction_x,
    float direction_y,
    int gradient_row,
    int gradient_col,
    Gradient** gradient_map,
    double end_threshold
)
{
    Gradient* gradient = follow_gradient( &gradient_row, &gradient_col,
                                       direction_x, direction_y, gradient_map );

    gradient = get_maximal_along_gradient( &gradient_row, &gradient_col,
                                                                 gradient_map );

    if ( !gradient->marked && gradient->mag >= end_threshold )
    {
        gradient->marked = TRUE;

        direction_x = - gradient->dy;
        direction_y = gradient->dx;
        follow_edge_left( direction_x, direction_y, gradient_row, gradient_col,
                                                  gradient_map, end_threshold );
    }
}

/**
 * @brief Creates a new Edge_point.
 */
static Edge_point* create_point
(
    double x,
    double y,
    const Pixel* px,
    const Gradient* gradient,
    Edge_point* next
)
{
    Edge_point* point;

    point = (Edge_point*) kjb_malloc( sizeof( Edge_point ) );

    point->x = x;
    point->y = y;

    if (px != NULL)
    {
        point->r = px->r;
        point->g = px->g;
        point->b = px->b;
    }
    else
    {
        point->r = 0;
        point->g = 0;
        point->b = 0;
    }

    if ( gradient != NULL )
    {
        point->dx = gradient->dx;
        point->dy = gradient->dy;
        point->gradient_mag = gradient->mag;
    }
    else
    {
        point->dx = 0;
        point->dy = 0;
        point->gradient_mag = 0;
    }

    point->marked = FALSE;

    point->next = next;

    return point;
}

/**
 * @brief Uses non-maximal suppression and hysterisis to get the edge points
 * from the image.
 *
 * @return Number of points found.
 */
static int get_edge_points_from_gradient_map
(
    Edge_point** points_out,
    const KJB_image* img,
    Gradient** gradient_map,
    double begin_threshold,
    double end_threshold
)
{
    int num_rows = img->num_rows;
    int num_cols = img->num_cols;
    int num_points = 0;
    int gradient_row;
    int gradient_col;
    int i, j;
    float direction_x;
    float direction_y;

    Edge_point* point_list = NULL;
    Edge_point* point = NULL;
    Gradient* gradient;
    Gradient* maximal_gradient;

    for ( i = 0; i < num_rows; i++ )
    {
        for ( j = 0; j < num_cols; j++ )
        {
            gradient = &gradient_map[ i ][ j ];

            if ( !gradient->marked && gradient->mag >= begin_threshold )
            {
                gradient_row = i;
                gradient_col = j;

                maximal_gradient = get_maximal_along_gradient( &gradient_row,
                                                  &gradient_col, gradient_map );

                if (!maximal_gradient->marked)
                {
                    maximal_gradient->marked = TRUE;

                    direction_x = maximal_gradient->dy;
                    direction_y = - maximal_gradient->dx;
                    follow_edge_right( direction_x, direction_y, gradient_row,
                                     gradient_col, gradient_map, end_threshold);

                    direction_x = - maximal_gradient->dy;
                    direction_y = maximal_gradient->dx;
                    follow_edge_left( direction_x, direction_y, gradient_row,
                                     gradient_col, gradient_map, end_threshold);
                }
            }
        }
    }

    point_list = create_point( -1, -1, NULL, NULL, NULL );
    point = point_list;

    for ( i = 0; i < num_rows; i++ )
    {
        for ( j = 0; j < num_cols; j++ )
        {
            gradient = &gradient_map[ i ][ j ];

            if (gradient->marked)
            {
                point->next = create_point( j, i, &(img->pixels[ i ][ j ]),
                                                               gradient, NULL );
                point = point->next;
                num_points++;
            }
        }
    }

    *points_out = point_list;

    return num_points;
}

/**
 * Smoothed partial derivative kernels, non-maximal suppression, and hysteresis
 * are used to trace out the edge points from the input image.
 *
 * @param points_out Result parameter containing an Edge_point linked list. If
 * @em *points_out is not NULL when the function is called, its space will be
 * freed. When the function returns @em *points_out points to a NULL-terminated
 * Edge_point linked. The first element of the linked list is not a real point,
 * it is only there to facility easy iteration over the points, e.g.
 *
 * @code
 * Edge_point* point = point_list;
 *
 * while( (point = point->next) != NULL )
 * { ... }
 * @endcode
 *
 * @param img_out Result parameter. If @em *img_out is NULL, an image is
 * allocated. If it is not NULL, its space is re-used. When the function
 * returns, @em *img_out points to the gradient magnitude of @em img_in. If @em
 * img_out is NULL, no gradient image is returned.
 *
 * @param img_in Input image to get the edge points of.
 *
 * @param sigma Parameter for the smoothing Gaussian.
 *
 * @param begin_threshold Used when scanning the image to look for gradient
 * magnitudes to follow under non-maximal suppression.
 *
 * @param end_threshold Once an edge is found under non-maximal suppression,
 * it is followed (perpendicular to the gradient) until the gradient magnitude
 * falls below this threshold.
 *
 * @note If @em *img_out == @em img_in, @em img_in will be overwritten by its
 * gradient magnitude image.
 */
void detect_edge_points_DEPRECATED
(
    Edge_point** points_out,
    KJB_image** img_out,
    const KJB_image* img_in,
    double sigma,
    double begin_threshold,
    double end_threshold
)
{
    JWS_Mask* gauss_dx_maskp = NULL;
    JWS_Mask* gauss_dy_maskp = NULL;
    Gradient** gradient_map = NULL;
    KJB_image* img = NULL;
    int i;

    create_2D_gaussian_mask_dx( &gauss_dx_maskp, sigma );
    create_2D_gaussian_mask_dy( &gauss_dy_maskp, sigma );

    if ( img_out == NULL)
    {
        kjb_copy_image( &img, img_in );
    }
    else if (*img_out == img_in)
    {
        img = *img_out;
    }
    else
    {
        kjb_copy_image( img_out, img_in );
        img = *img_out;
    }

    convolve_and_create_image_gradient_map( &gradient_map, img, gauss_dx_maskp,
                                                               gauss_dy_maskp );

    get_edge_points_from_gradient_map( points_out, img, gradient_map,
                                               begin_threshold, end_threshold );

    free_matrix( gauss_dx_maskp->mp );
    free_matrix( gauss_dy_maskp->mp );
    kjb_free( gauss_dx_maskp );
    kjb_free( gauss_dy_maskp );

    for ( i = 0; i < img_in->num_rows; i++ )
    {
        kjb_free( gradient_map[ i ] );
    }
    kjb_free( gradient_map );

    if ( img_out == NULL )
    {
        kjb_free_image( img );
        img = NULL;
    }
}

/**
 * @param points Edge_point linked list to free.
 */
void free_edge_points_DEPRECATED( Edge_point* points )
{
    Edge_point* point;
    Edge_point* point_prev;

    if(points == NULL) return;

    point = points;
    point_prev = point;


    while ( ( point = point->next ) != NULL)
    {
        kjb_free( point_prev );
        point_prev = point;
    }

    kjb_free( point_prev );
}

/**
 * @return Euclidean distance between @em p1 and @em p2.
 */
double point_distance_DEPRECATED( const Edge_point* p1, const Edge_point* p2 )
{
    double xdiff, ydiff;

    xdiff = p1->x - p2->x;
    ydiff = p1->y - p2->y;

    return sqrt( xdiff * xdiff + ydiff * ydiff );
}

/**
 * Wow, this is inefficient.
 */
void sample_edge_points_DEPRECATED
(
    Edge_point* points,
    double radius
)
{
    int num_points = 0;
    int num_points_marked = 0;

    Edge_point* point = points;

    while ( (point = point->next ) != NULL )
    {
        num_points++;
    }

    {
        Edge_point* max_point = points->next;

        while ( num_points != num_points_marked )
        {
            int max_intensity = INT_MIN;
            point = points;

            while ( (point = point->next) != NULL )
            {
                if ( !point->marked && point->r > max_intensity )
                {
                    max_intensity = kjb_rint(point->r);
                    max_point = point;
                }
            }

            max_point->marked = TRUE;
            num_points_marked++;

            point = points;

            {
                Edge_point* point_prev = point;

                while ( (point = point->next) != NULL )
                {
                    if ( !point->marked &&
                         point_distance_DEPRECATED( max_point, point ) <= radius )
                    {
                        point_prev->next = point->next;
                        kjb_free(point);
                        point = point_prev;
                        num_points--;
                    }
                    else
                    {
                        point_prev = point;
                    }
                }
            }
        }
    }
}

/**
 * No comment.
 */
void color_edge_points_DEPRECATED
(
    KJB_image* img,
    Edge_point* points,
    float r,
    float g,
    float b
)
{
    Edge_point* point;
    int x, y;

    point = points;

    while ( (point = point->next) != NULL )
    {
        /* Kobus: not all systems have rintf(). Wrapped it. 
         *
            x = rintf( point->x );
            y = rintf( point->y );
        */
        x = kjb_rintf( point->x );
        y = kjb_rintf( point->y );

        img->pixels[ y ][ x ].r *= r;
        img->pixels[ y ][ x ].g *= g;
        img->pixels[ y ][ x ].b *= b;
    }
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

#undef Edge_point

#ifdef __cplusplus
}
#endif

