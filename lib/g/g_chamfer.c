/* $Id */

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


#include "l/l_sys_debug.h"   /* For ASSERT */
#include "i/i_float.h"
#include "g/g_chamfer.h"
#include "edge/edge_base.h"

#define CHAMFER_DISTANCE 1
#define MEAN_DISTANCE 1
#define SUM_SQ_DISTANCE 2
#define MEAN_SQ_DISTANCE 3
#define SUM_DISTANCE 4

#ifdef __cplusplus
extern "C" {
#endif

static int oriented_edge_distance(
        int method,
        Edge_point const*** image_edge_map,
        int num_rows,
        int num_cols,
        const Edge_set* tmplate,
        int offset_row,
        int offset_col,
        double* distance,
        size_t* point_count_out);

static int edge_distance(
        int method,
        const Matrix* chamfer_image,
        const Edge_set* tmplate,
        int offset_row,
        int offset_col,
        double dist_bound,
        double* distance_out,
        size_t* point_count_out);

/* -------------------------------------------------------------------------- */

/* =============================================================================
 *                     chamfer_transform
 *
 * Computes the chamfer distance transform of a binary edge image.
 *
 * Input is a binary edge image represented as a Matrix, and output is a map of
 * the distance and location of the nearest edge point in the input image.
 * The local maxima of the distance map is the voronoi diagram of the edge
 * points. The size parameter sets the neighborhood size when propagating
 * distance values; higher value will result in lower approximation error but
 * increases running time.  Valid values for size are 3, 5, or 7.
 *
 * After returning, distances_out will hold the distance map, and locations_out
 * will hold two matrices representing the x and y locations of the nearest edge
 * point, respectively.
 *
 * Warning
 *
 * As the exact distance transform is expensive to compute, the chamfer distance
 * transform is a fast approximation.  When the voronoi diagram has corners with
 * very acute angles, the chamfer transform may report the nearest edge point
 * incorrectly.  Also, distances are not computed directly but instead are
 * propogated from neighboring pixels and are subject to approximation error.
 * Using higher size values diminshes this effect somewhat.  These errors are
 * generally of little concern, but do be aware of them.
 *
 * Returns:
 *     NO_ERROR on success, and ERROR on failure, with an error message being
 *     set.
 *
 * Author: Kyle Simek
 *
 * Related:
 *    image_to_matrix
 *
 * Index: geometry, images, chamfer transform
 *
 * -----------------------------------------------------------------------------
*/

int chamfer_transform(
        const Matrix* in,
        int size,
        Matrix** distances_out,
        Int_matrix_vector** locations_out)
{
    Matrix* filter = NULL;
    int error = NO_ERROR;
    int pass;
    int cur_row, cur_col;
    int i,j;

    switch(size)
    {
        /* distance constants are that of Butt, Maragos '98 */
        /* Value of -1 in filter below means "ignore" */
        case 3:
        {
            /*
//            int a = b = 1; // chessboard distance
//            int a = 1; b = 2; // manhattan distance
//            int a = 1; b = 1.33; // hybrid (euclidean approximation)
//            */
            float a = 0.96194;
            float b = 1.3604; /* Butt's approximation */

            ERE(get_initialized_matrix(&filter, 3, 3, 0.0));

            filter->elements[0][1] = filter->elements[1][0] =
            filter->elements[1][2] = filter->elements[2][1] = a;

            filter->elements[0][0] = filter->elements[2][0] =
            filter->elements[0][2] = filter->elements[2][2] = b;
            break;
        }
        case 5:
        {
            float a = 0.9866, b = 1.4142, c = 2.2062; /* Butt's approximation */
            ERE(get_initialized_matrix(&filter, 5, 5, 0.0));

            filter->elements[0][0] = -1; filter->elements[0][1] =  c; filter->elements[0][2] = -1; filter->elements[0][3] =  c; filter->elements[0][4] = -1;
            filter->elements[1][0] =  c; filter->elements[1][1] =  b; filter->elements[1][2] =  a; filter->elements[1][3] =  b; filter->elements[1][4] =  c;
            filter->elements[2][0] = -1; filter->elements[2][1] =  a; filter->elements[2][2] =  0; filter->elements[2][3] =  a; filter->elements[2][4] = -1;
            filter->elements[3][0] =  c; filter->elements[3][1] =  b; filter->elements[3][2] =  a; filter->elements[3][3] =  b; filter->elements[3][4] =  c;
            filter->elements[4][0] = -1; filter->elements[4][1] =  c; filter->elements[4][2] = -1; filter->elements[4][3] =  c; filter->elements[4][4] = -1;

            break;
        }
        case 7:
        {
            float a = .9935, b = 1.4142, c = 2.2361, d = 3.1419, e = 3.6056; /* Butt's approximation */
            ERE(get_initialized_matrix(&filter, 7, 7, 0.0));

            filter->elements[0][0] = -1; filter->elements[0][1] =  e; filter->elements[0][2] =  d; filter->elements[0][3] = -1; filter->elements[0][4] =  d; filter->elements[0][5] =  e; filter->elements[0][6] = -1;
            filter->elements[1][0] =  e; filter->elements[1][1] = -1; filter->elements[1][2] =  c; filter->elements[1][3] = -1; filter->elements[1][4] =  c; filter->elements[1][5] = -1; filter->elements[1][6] =  e;
            filter->elements[2][0] =  d; filter->elements[2][1] =  c; filter->elements[2][2] =  b; filter->elements[2][3] =  a; filter->elements[2][4] =  b; filter->elements[2][5] =  c; filter->elements[2][6] =  d;
            filter->elements[3][0] = -1; filter->elements[3][1] = -1; filter->elements[3][2] =  a; filter->elements[3][3] =  0; filter->elements[3][4] =  a; filter->elements[3][5] = -1; filter->elements[3][6] = -1;
            filter->elements[4][0] =  d; filter->elements[4][1] =  c; filter->elements[4][2] =  b; filter->elements[4][3] =  a; filter->elements[4][4] =  b; filter->elements[4][5] =  c; filter->elements[4][6] =  d;
            filter->elements[5][0] =  e; filter->elements[5][1] = -1; filter->elements[5][2] =  c; filter->elements[5][3] = -1; filter->elements[5][4] =  c; filter->elements[5][5] = -1; filter->elements[5][6] =  e;
            filter->elements[6][0] = -1; filter->elements[6][1] =  e; filter->elements[6][2] =  d; filter->elements[6][3] = -1; filter->elements[6][4] =  d; filter->elements[6][5] =  e; filter->elements[6][6] = -1;

            break;
        }
        default:
           set_error("Chamfer transform: invalid size: %d.  Valid sizes are 3, 5, or 7.", size);

            break;

    }

    if((error = get_initialized_matrix(distances_out, in->num_rows, in->num_cols, FLT_MAX))) goto cleanup;

    if((error = get_target_int_matrix_vector(locations_out, 2))) goto cleanup;
    for(i = 0; i < 2; i++)
    {
        if((error = get_initialized_int_matrix(&(*locations_out)->elements[i], in->num_rows, in->num_cols, INT_MAX))) goto cleanup;
    }

    for(pass = 1; pass <= 2; pass++)
    {
        int first_row, first_col;

        if(pass == 1)
        {
            first_row = 0;
            first_col = 0;
        } else
        {
            first_row = in->num_rows - 1;
            first_col = in->num_cols - 1;
        }

        cur_row = first_row;
        for(i = 0; i < in->num_rows; i++)
        {
            cur_col = first_col;

            for(j = 0; j < in->num_cols; j++)
            {
                double* cur_distance;
                int* cur_u;
                int* cur_v;

                int f_row_midpoint = (filter->num_rows - 1) / 2;
                int f_col_midpoint = (filter->num_cols - 1) / 2;

                int row_offset = -f_row_midpoint;
                int col_offset = -f_col_midpoint;

                int cur_f_row;

                if(j != 0)
                {
                    /* this normally would go at the end of the loop, */
                    /* but the "continue" statements skip it there */
                    if(pass == 1)
                        cur_col++;
                    else
                        cur_col--;
                }

                /* get pointers to destination rgb for convenience */
                cur_distance = &(*distances_out)->elements[cur_row][cur_col];
                cur_u =        &(*locations_out)->elements[0]->elements[cur_row][cur_col];
                cur_v =        &(*locations_out)->elements[1]->elements[cur_row][cur_col];

                /* if background pixel, distance is zero and we're done */
                if(in->elements[cur_row][cur_col] == 0)
                {
                    *cur_distance = 0;
                    *cur_u = cur_col;
                    *cur_v = cur_row;
                    continue;
                }

                /* When applying filter value (0,0), which reference value to look up? */
                /* (assumes odd filter size.  In practice, always 3x3, 5x5, or 7x7) */

                /* iterate over filter values */
                for(cur_f_row = 0; cur_f_row < filter->num_rows; cur_f_row++)
                {
                    int cur_f_col;
                    /* compute reference row */
                    int cur_ref_row = cur_row + row_offset + cur_f_row;
                    if(cur_ref_row < 0 || cur_ref_row >= in->num_rows) continue;

                    /* top rows on first pass, bottom on second pass, middle on both */
                    if(cur_f_row > f_row_midpoint)
                    {
                        if(pass == 1) continue;
                    }

                    if(cur_f_row < f_row_midpoint)
                    {
                        if(pass == 2) continue;
                    }

                    for(cur_f_col = 0; cur_f_col < filter->num_cols; cur_f_col++)
                    {
                        int cur_ref_col;
                        double ref_distance;
                        int ref_u, ref_v;

                        /* process left cols of middle row on first pass, right cols on second */
                        if(cur_f_row == f_row_midpoint)
                        {
                            if(cur_f_col >= f_col_midpoint)
                            {
                                if(pass == 1) continue;
                            } else {
                                if(pass == 2) continue;
                            }
                        }

                        /* -1 in filter indicates "ignore" */
                        if(filter->elements[cur_f_row][cur_f_col] < 0) continue;

                        /* reference col */
                        cur_ref_col = cur_col + col_offset + cur_f_col;

                        if(cur_ref_col < 0 || cur_ref_col >= in->num_cols) continue;

                        ref_distance = (*distances_out)->elements[cur_ref_row][cur_ref_col];
                        ref_u = (*locations_out)->elements[0]->elements[cur_ref_row][cur_ref_col];
                        ref_v = (*locations_out)->elements[1]->elements[cur_ref_row][cur_ref_col];

                        if(*cur_distance > ref_distance + filter->elements[cur_f_row][cur_f_col])
                        {
                            *cur_distance = ref_distance + filter->elements[cur_f_row][cur_f_col];
                            *cur_u        = ref_u;
                            *cur_v        = ref_v;
                        }
                    }
                }
            }

            if(pass == 1)
                cur_row++;
            else
                cur_row--;
        }
    }

cleanup:
    free_matrix(filter);
    return error;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */


/* =============================================================================
 *                     chamfer_transform_2
 *
 * Alternate chamfer transform
 *
 * Alternate form of chamfer_transform the gives more informative output when
 * a fill edge point list is available instead of just a binary edge image.
 *
 * Input is a linked-list of edge points, the dimensions of the original
 * image, and a neighborhood size (see chamfer_transform for explanation of
 * neighborhood size).
 *
 * Output is a matrix of distances to the nearest edge point, and a 2D array
 * of pointers to the nearest edge point.  The 2D pointer array can be freed
 * using free_2D_ptr_array().  This does not free the edge points in "points",
 * which must be freed separately.
 *
 * Index: geometry, images, chamfer transform
 *
 * Author: Kyle Simek
 * -----------------------------------------------------------------------------
 */
int chamfer_transform_2(
        const Edge_set* points,
        int num_rows,
        int num_cols,
        int size,
        Matrix** distances_out,
        Edge_point **** edge_map)
{
    /* create edge image */
    int err = NO_ERROR;
    Matrix* edge_img = NULL;
    Int_matrix_vector* locations = NULL;

    int counter = 0;
    int row,col;
    size_t i;

    ERE(get_initialized_matrix(&edge_img, num_rows, num_cols, 1.0));

    NRE(*edge_map = (Edge_point ***) allocate_2D_ptr_array(num_rows, num_cols));

    for(i = 0; i < points->total_num_pts; i++)
    {
        Edge_point* cur_pt = &points->edges[0].points[i];
        int row = kjb_rintf(cur_pt->row);
        int col = kjb_rintf(cur_pt->col);
        edge_img->elements[row][col] = 0.0;
        (*edge_map)[row][col] = cur_pt;

        counter++;
    }

    err = chamfer_transform(edge_img, size, distances_out, &locations);
    if(err) goto cleanup;

    for(row = 0; row < num_rows; row++)
    {
        for(col = 0; col < num_cols; col++)
        {

            /* "locations" refers to locations of the nearest edge point in (u,v) coordinates */
            int u = locations->elements[0]->elements[row][col];
            int v = locations->elements[1]->elements[row][col];

            ASSERT( u < INT_MAX);
            ASSERT( v < INT_MAX);


            (*edge_map)[row][col] = (*edge_map)[v][u];
        }
    }

cleanup:
    if(edge_img) free_matrix(edge_img);
    if(locations) free_int_matrix_vector(locations);
    return err;
}
/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*
 * =============================================================================
 *                     sum_sq_distance
 *
 * Compute sum of square distance
 *
 * Compute the sum of square distances distance to the between template edge points and the nearest input edge point.  I.e.  \sum_1^n d(e_i)^2.  Compare this to chamfer distance wich is  1/n \sum_1^n d(e_i).
 *
 * Image must be input as a chamfer transformed image output by chamfer_transform().
 *
 * Template must be input as an Edge_point linked list like that output by detect_edge_points.
 *
 * Offset_{x,y} is the position of the top-left of the template.
 *
 * dist_bound truncates overly large distances so edge pixels missing due to noise don't overly affect the distance measure.  Setting dist_bound to 0 disables truncation.
 *
 * Distance_out is the chamfer distance.  If dist_bound > 0, the result is homogenized to [0, dist_bound]
 *
 * Point_count_out is the number of edge points used to compute the distance.  Useful if you'll be taking an average later on.  Passing NULL ignores this argument.
 *
 * Index: geometry, images, chamfer transform
 *
 * Author: Kyle Simek
 *
 * -----------------------------------------------------------------------------
 */

int sum_sq_distance(
        const Matrix* chamfer_image,
        const Edge_set* tmplate,
        int offset_row,
        int offset_col,
        double dist_bound,
        double* distance_out,
        size_t* point_count_out)
{
    return edge_distance(
        SUM_SQ_DISTANCE,
        chamfer_image,
        tmplate,
        offset_row,
        offset_col,
        dist_bound,
        distance_out,
        point_count_out
    );
}

/* =============================================================================
 *                     chamfer_distance
 *
 * Compute the chamfer distance between an input image and an edge template.
 *
 * Image must be input as a chamfer transformed image output by chamfer_transform().
 *
 * Template must be input as an Edge_point linked list like that output by detect_edge_points.
 *
 * Offset_{x,y} is the position of the top-left of the template.
 *
 * dist_bound truncates overly large distances so edge pixels missing due to noise don't overly affect the distance measure.  Setting dist_bound to 0 disables truncation.
 *
 * Distance_out is the chamfer distance.  If dist_bound > 0, the result is homogenized to [0, dist_bound]
 *
 * Point_count_out is the number of edge points used to compute the distance.  Useful if you'll be taking an average later on.  Passing NULL ignores this argument.
 *
 * Index: chamfer transform
 *
 * Author: Kyle Simek
 * -----------------------------------------------------------------------------
 */
int chamfer_distance(
        const Matrix* chamfer_image,
        const Edge_set* tmplate,
        int offset_row,
        int offset_col,
        double dist_bound,
        double* distance_out,
        size_t* point_count_out)
{
    return edge_distance(
        CHAMFER_DISTANCE,
        chamfer_image,
        tmplate,
        offset_row,
        offset_col,
        dist_bound,
        distance_out,
        point_count_out
    );
}

/*
 * method can be SUM_DISTANCE, SUM_SQ_DISTANCE, MEAN_SQ_DISTANCE, or CHAMFER_DISTANCE (aka MEAN_DISTANCE)
 **/
static int edge_distance(
        int method,
        const Matrix* chamfer_image,
        const Edge_set* tmplate,
        int offset_row,
        int offset_col,
        double dist_bound,
        double* distance_out,
        size_t* point_count_out)
{
    double total = 0;

    int num_edge_points = 0;

    double cur_dist;

    int square_distances, take_average;

    size_t i;

    square_distances = (method == SUM_SQ_DISTANCE || method == MEAN_SQ_DISTANCE);
    take_average = (method == CHAMFER_DISTANCE || method == MEAN_SQ_DISTANCE || method == MEAN_DISTANCE);

    for(i = 0; i < tmplate->total_num_pts; i++)
    {
        Edge_point* cur_pt = &tmplate->edges[0].points[i];

        int cur_row = cur_pt->row + offset_row;
        int cur_col = cur_pt->col + offset_col;

        if(cur_col >= chamfer_image->num_cols ||
           cur_row >= chamfer_image->num_rows ||
           cur_col < 0 ||
           cur_row < 0)
        {
            continue;
        }

        if(dist_bound)
        {
            cur_dist = MIN(dist_bound, chamfer_image->elements[cur_row][cur_col]);
        }
        else
        {
            cur_dist = chamfer_image->elements[cur_row][cur_col];
        }

        if(square_distances)
        {
            cur_dist *= cur_dist;
        }


        if(dist_bound)
        {
            cur_dist /= dist_bound;

            if(square_distances)
            {
                cur_dist /= dist_bound;
            }
        }
        total += cur_dist;

        num_edge_points++;
    }

    if(take_average)
    {
        if(num_edge_points == 0)
        {
            set_error("Mean distance: needs at least one point.");
            return ERROR;
        }

        total /= num_edge_points;
    }

    *distance_out = total;

    if(point_count_out != NULL)
    {
        *point_count_out = num_edge_points;
    }

    return NO_ERROR;
}
/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */


/* =============================================================================
 *                     oriented_sum_sq_distance
 *
 * Find distance using sum-of-squares
 *
 * Compute the oriented sum-of-squares distance between an input image and an edge template, using distance metric described in Shotton, Blake, Cipolla 08.
 *
 * Image must be input as a matrix of Edge_point pointers to the nearest edge point at each position.  This is output by chamfer_transform_2.
 *
 * num_rows and num_cols describes the size of image_edge_map
 *
 * Template must be input as an Edge_point linked list like that output by detect_edge_points.
 *
 * Offset_{x,y} is the position of the top-left of the template.
 *
 * Distance_out is the computed chamfer distance.
 *
 * Index: geometry, images, chamfer transform
 *
 * Author: Kyle Simek
 * -----------------------------------------------------------------------------
 */
int oriented_sum_sq_distance(
    Edge_point const*** image_edge_map,
    int num_rows,                          /* rows of image_edge_map    */
    int num_cols,                          /* columns of image_edge_map */
    const Edge_set* tmplate,
    int offset_row,                        /* top of template           */
    int offset_col,                        /* left edge of template     */
    double* distance,                      /* output chamfer distance   */
    size_t* point_count_out
)
{
    return oriented_edge_distance(
            SUM_SQ_DISTANCE,
            image_edge_map,
            num_rows,
            num_cols,
            tmplate,
            offset_row,
            offset_col,
            distance,
            point_count_out);

}

/* =============================================================================
 *                     oriented_chamfer_distance
 *
 * Find distance using chamfer metric
 *
 * Compute the oriented chamfer distance between an input image and an edge
 * template, as described in Shotton, Blake, Cipolla 08, but using the absolute
 * value of the cosine of the difference between the two angles, instead of the
 * absolute difference.
 *
 * Image must be input as a matrix of Edge_point pointers to the nearest edge
 * point at each position.  This is output by chamfer_transform_2.
 *
 * num_rows and num_cols describes the size of image_edge_map
 *
 * Template must be input as an Edge_point linked list like that output by
 * detect_edge_points.
 *
 * Offset_{x,y} is the position of the top-left of the template.
 *
 * Distance_out is the computed chamfer distance.
 *
 * Index: geometry, images, chamfer transform
 *
 * Author: Kyle Simek
 * -----------------------------------------------------------------------------
 */
int oriented_chamfer_distance(
    Edge_point const*** image_edge_map,
    int num_rows,                          /* rows of image_edge_map    */
    int num_cols,                          /* columns of image_edge_map */
    const Edge_set* tmplate,
    int offset_row,                        /* top of template           */
    int offset_col,                        /* left edge of template     */
    double* distance,                      /* output chamfer distance   */
    size_t* point_count_out
)
{
    return oriented_edge_distance(
            CHAMFER_DISTANCE,
            image_edge_map,
            num_rows,
            num_cols,
            tmplate,
            offset_row,
            offset_col,
            distance,
            point_count_out);

}

static int oriented_edge_distance(
        int method,
        Edge_point const*** image_edge_map,
        int num_rows,
        int num_cols,
        const Edge_set* tmplate,
        int offset_row,
        int offset_col,
        double* distance,
        size_t* point_count_out)
{
    double total = 0;

    int num_edge_points = 0;
    int square_distances, take_average;

    size_t i;

    square_distances = (method == SUM_SQ_DISTANCE || method == MEAN_SQ_DISTANCE);
    take_average = (method == CHAMFER_DISTANCE || method == MEAN_SQ_DISTANCE || method == MEAN_DISTANCE);

    for(i = 0; i < tmplate->total_num_pts; i++)
    {
        Edge_point* cur_pt = &tmplate->edges[0].points[i];

        int cur_row = (int) cur_pt->row + offset_row;
        int cur_col = (int) cur_pt->col + offset_col;

        const Edge_point* nearest_pt;
        double phi_nearest;
        double phi_cur;
        double cur_dist;

        if(cur_col >= num_cols ||
           cur_row >= num_rows ||
           cur_col < 0 ||
           cur_row < 0)
        {
            continue;
        }

        nearest_pt = image_edge_map[cur_row][cur_col];

        phi_nearest = atan2(nearest_pt->drow, nearest_pt->dcol);
        phi_cur = atan2(cur_pt->drow, cur_pt->dcol);

        /* should we use linear angle difference instead of cos?
         * Both are monotonic, but cos penalizes far values more severely
         */
        cur_dist = 1.0 - fabs(cos(phi_nearest - phi_cur));

        if(square_distances)
        {
            cur_dist *= cur_dist;
        }

        total += cur_dist;
        num_edge_points++;
    }

    *distance = total;

    if(take_average)
    {
        if(num_edge_points == 0)
        {
            set_error("Mean distance: needs at least one point.");
            return ERROR;
        }
        *distance = *distance / num_edge_points;
    }

    if(point_count_out != NULL)
    {
        *point_count_out = num_edge_points;
    }


    return NO_ERROR;

}
/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */
#ifdef __cplusplus
}
#endif
