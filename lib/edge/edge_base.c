/* $Id: edge_base.c 21545 2017-07-23 21:57:31Z kobus $ */
/**
 * This work is licensed under a Creative Commons 
 * Attribution-Noncommercial-Share Alike 3.0 United States License.
 * 
 *    http://creativecommons.org/licenses/by-nc-sa/3.0/us/
 * 
 * You are free:
 * 
 *    to Share - to copy, distribute, display, and perform the work
 *    to Remix - to make derivative works
 * 
 * Under the following conditions:
 * 
 *    Attribution. You must attribute the work in the manner specified by the
 *    author or licensor (but not in any way that suggests that they endorse you
 *    or your use of the work).
 * 
 *    Noncommercial. You may not use this work for commercial purposes.
 * 
 *    Share Alike. If you alter, transform, or build upon this work, you may
 *    distribute the resulting work only under the same or similar license to
 *    this one.
 * 
 * For any reuse or distribution, you must make clear to others the license
 * terms of this work. The best way to do this is with a link to this web page.
 * 
 * Any of the above conditions can be waived if you get permission from the
 * copyright holder.
 * 
 * Apart from the remix rights granted under this license, nothing in this
 * license impairs or restricts the author's moral rights.
 */

/* =========================================================================== *
   |
   |  Copyright (c) 1994-2010 by Kobus Barnard (author)
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
   |  Author: Joe Schlecht, Kyle Simek
 * =========================================================================== */


#include <l/l_error.h>
#include <stdlib.h>
#include <stdio.h>
#include <inttypes.h>
#include <assert.h>
#include <math.h>
#include <string.h>
#include <errno.h>

#include "l/l_incl.h"
#include "i/i_float.h"
#include "i/i_float_io.h"
#include "m/m_vector.h"
#include "i/i_matrix.h"
#include "m/m_matrix.h"
#include "m/m_mat_basic.h"
#include "m/m_convolve.h"
#include "m2/m2_ncc.h"
#include "edge/edge_base.h"


#define KJB_EDGE_MAX_RGB_VALUE 255

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                                free_edge_list
 *
 * Linked list node containing an edge point.
 *
 * =============================================================================
 */
typedef struct Edge_list
{
    /** @brief Edge point in the list node. */
    Edge_point* pt;

    /** @brief Next edge list node. */
    struct Edge_list* next;
}
Edge_list;


/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                                free_edge_list
 *
 * Frees an edge_list
 *
 * =============================================================================
 */
static void free_edge_list(Edge_list* edges)
{
    Edge_list* next;

    next = edges->next;

    kjb_free(edges->pt);
    kjb_free(edges);

    if (next)
    {
        free_edge_list(next);
    }
}


/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                                copy_edge_list_into_set
 *
 * Copies a linked list of edge points as an edge into the set of
 * edges.
 *
 * =============================================================================
 */
static void copy_edge_list_into_set(Edge_set* set, Edge_list* list)
{
    uint32_t      num_pts;
    uint32_t      e;
    Edge_list*  iter;
    Edge_point* pt;
    Edge_point* pts;

    set->num_edges++;

/*    assert(set->total_num_pts = kjb_realloc(set->total_num_pts, */
/*                set->num_edges * sizeof(uint32_t))); */

    num_pts = 0;
    iter = list;
    while (iter != NULL)
    {
        num_pts++;
        iter = iter->next;
    }
    set->total_num_pts += num_pts;

    if (set->num_edges == 1)
    {
        set->edges = (Edge*) kjb_malloc(set->num_edges * sizeof(Edge));
        assert(set->edges);
        set->edges[0].points = (Edge_point*) kjb_malloc(set->total_num_pts * sizeof(Edge_point));
        assert(set->edges[0].points);
    }
    else
    {
        set->edges = (Edge*) kjb_realloc(set->edges, set->num_edges * sizeof(Edge));
        assert(set->edges);

        set->edges[0].points = (Edge_point*) kjb_realloc(set->edges[0].points, 
                    set->total_num_pts * sizeof(Edge_point));
        assert(set->edges[0].points);

        pts = set->edges[0].points;
        for (e = 1; e < set->num_edges; e++)
        {
            pts += set->edges[ e - 1 ].num_points;
            set->edges[ e ].points = pts;
        }
    }

    set->edges[ set->num_edges - 1 ].num_points = num_pts;

    pt   = set->edges[ set->num_edges - 1 ].points;
    iter = list;
    while (iter != NULL)
    {
        *pt = *(iter->pt);

        pt++;
        iter = iter->next;
    }
}


 /*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                                merge_edge_lists
 *
 * Merges two linked lists of edge points into one.
 * At least one of the lists must have an element. The two lists are merged at
 * their heads. So the order of the left list is tail-to-head and the order of
 * the right is head-to-tail.
 * =============================================================================
 */
static Edge_list* merge_edge_lists
(
    Edge_list* left, 
    Edge_list* right
)
{
    Edge_list* iter;
    Edge_list* next;
    Edge_list* prev;

    assert(left != NULL || right != NULL);

    if (left == NULL)
    {
        return right;
    }
    else if (right == NULL)
    {
        return left;
    }

    prev = right;
    iter = left;
    while (iter != NULL)
    {
        next = iter->next;
        iter->next = prev;
        prev = iter;
        iter = next;
    }

    return prev;
}



/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                                Gradient
 *
 * This structure is stored to used the Gradient of an image intensity detected
 * using the partial derivative masks. dcol and drow are normalize. Their
 * original mangitude is stored in mag. The field marked is needed by several
 * algorithms in this file
 *
 * =============================================================================
 */
typedef struct
{
    /** @brief Rate of change in brightness along the columns at the point. */
    float dcol;

    /** @brief Rate of change in brightness along the rows at the point. */
    float drow;

    /** @brief Magnitude of the gradient. */
    float mag;

    /** @brief Boolean used for the edge detection algorithm. */
    uint8_t marked;
}
Gradient;

void update_gradient_map_for_noiseless_data(Gradient **map, const Matrix *m, unsigned int padding);

#include "i/i_float.h"
#include "i/i_float_io.h"

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                         create_gradient_map
 *
 *  Creates a gradient map from a matrix.
 *  It first computes the partial derivatives along x and y.
 *  Then it computes the gradient magnitude and direction for each
 *  pixel, and stores it into map_out
 *
 *  @param map_out Will contain the output gradient information
 *  @param m The input image stored in a matrix
 *  @param sigma the standard deviation of the gaussian used to blur the image
 *  @param use_fourier specifies whether to use Fast Fourier Transform to do
 *         convolution
 *
 * =============================================================================
 */
static int create_gradient_map
(
    Gradient***   map_out,
    const Matrix* m,
    float           sigma,
    unsigned char use_fourier
)
{
    uint32_t num_rows, num_cols;
    uint32_t row, col;
    float    dcol, drow;

    Matrix*    gauss  = NULL;
    Matrix*    m_dcol = NULL;
    Matrix*    m_drow = NULL;
    Gradient** map;
    Gradient*  map_elts;

    num_rows = m->num_rows;
    num_cols = m->num_cols;

    assert(*map_out == NULL);

#ifdef KJB_HAVE_FFTW
    if(use_fourier)
    {
    ERE(get_2D_gaussian_dx_mask(
                &gauss, 
                num_rows, num_cols,
                sigma, sigma));

    assert(fourier_convolve_matrix(&m_dcol, m, gauss) == NO_ERROR);

    ERE(get_2D_gaussian_dy_mask(&gauss, num_rows, num_cols, sigma, sigma));
    assert(fourier_convolve_matrix(&m_drow, m, gauss) == NO_ERROR);
    }
    else /* !use_fourier */
#endif
    {
        int mask_size;

        mask_size = ceil(sqrt(2) * 6 * sigma);
        mask_size += 1 - mask_size % 2;

        assert(mask_size % 2 == 1);
        ERE(get_2D_gaussian_dx_mask(&gauss, mask_size, mask_size, sigma, sigma));

        assert(convolve_matrix(&m_dcol, m, gauss) == NO_ERROR);

        ERE(get_2D_gaussian_dy_mask(&gauss, mask_size, mask_size, sigma, sigma));

        assert(convolve_matrix(&m_drow, m, gauss) == NO_ERROR);
    }

    free_matrix(gauss);

    map_elts = (Gradient*) kjb_malloc(num_rows*num_cols*sizeof(Gradient));
    assert(map_elts);

    *map_out = (Gradient**) kjb_malloc(num_rows*sizeof(Gradient*));
    assert(*map_out);

    map = *map_out;

    for (row = 0; row < num_rows; row++)
    {
        map[ row ] = &(map_elts[ row*num_cols ]);
    }

    for (row = 0; row < num_rows; row++)
    {
        for (col = 0; col < num_cols; col++)
        {
            dcol = m_dcol->elements[ row ][ col ];
            drow = m_drow->elements[ row ][ col ];

            map[ row ][ col ].dcol   = dcol;
            map[ row ][ col ].drow   = drow;
            map[ row ][ col ].mag    = sqrt(dcol*dcol + drow*drow);
            map[ row ][ col ].marked = 0;

            if (map[ row ][ col ].mag >= 1.0e-8)
            {
                map[ row ][ col ].dcol /= map[ row ][ col ].mag;
                map[ row ][ col ].drow /= map[ row ][ col ].mag;
            }
        }
    }

    free_matrix(m_dcol);
    free_matrix(m_drow);

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                              follow_gradient
 *
 *  Follow the gradient onto the image along the specified direction
 *
 * =============================================================================
 */
static Gradient* follow_gradient
(
    int32_t*     grad_row_in_out, 
    int32_t*     grad_col_in_out,
    float        dir_x,
    float        dir_y,
    Gradient** map,
    uint32_t     num_rows,
    uint32_t     num_cols,
    uint32_t     padding
)
{
    static const int neighbor_positions[ 8 ][ 2 ] = { {  1,  0 }, 
                                                {  1,  1 }, 
                                                {  0,  1 }, 
                                                { -1,  1 }, 
                                                { -1,  0 }, 
                                                { -1, -1 }, 
                                                {  0, -1 }, 
                                                {  1, -1 } };
    static const float neighbor_vectors[ 8 ][ 2 ] = { {  1, 0 }, 
                                                {  0.70710678f,  0.70710678f },
                                                {  0,            1 },
                                                { -0.70710678f,  0.70710678f },
                                                { -1,            0 },
                                                { -0.70710678f, -0.70710678f },
                                                {  0,           -1 },
                                                {  0.70710678f, -0.70710678f }};

    uint8_t     i;
    uint8_t     max_neighbor;
    int32_t     grad_col, grad_row;
    float       max_dp;
    float       dp;

    Gradient* grad;

    grad_col = *grad_col_in_out;
    grad_row = *grad_row_in_out;

    max_neighbor = 0;
    max_dp       = -1.0f;
    grad         = &map[ grad_row ][ grad_col ];

    for (i = 0; i < 8; i++)
    {
        dp = dir_x * neighbor_vectors[ i ][ 0 ] +
             dir_y * neighbor_vectors[ i ][ 1 ];

        /* Should have some stochastic tie-breaker here. */
        if (dp >= max_dp)
        {
            max_neighbor = i;
            max_dp       = dp;
        }
    }

    grad_row += neighbor_positions[ max_neighbor ][ 1 ];
    grad_col += neighbor_positions[ max_neighbor ][ 0 ];

    if (grad_row == (int32_t) padding-1) 
        grad_row = padding;
    else if (grad_row == (int32_t) (num_rows-padding))
        grad_row = num_rows-padding-1;
   
    if (grad_col == (int32_t) padding-1) 
        grad_col = padding;
    else if (grad_col == (int32_t)(num_cols-padding))
        grad_col = num_cols-padding-1;

    *grad_col_in_out = grad_col;
    *grad_row_in_out = grad_row;

    return &map[ grad_row ][ grad_col ];
}


/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                              get_maximal_along_gradient
 *
 *  Performs non maximal suppression along the gradient direction
 *
 * =============================================================================
 */
static Gradient* get_maximal_along_gradient
(
    int32_t*     grad_row_in_out, 
    int32_t*     grad_col_in_out, 
    Gradient** map,
    uint32_t     num_rows,
    uint32_t     num_cols,
    uint32_t     padding
) 
{
    int32_t grad_row, grad_col;
    int32_t curr_grad_row, curr_grad_col;
    int32_t next_grad_row, next_grad_col;
    int32_t prev_grad_row, prev_grad_col;

    Gradient* curr_grad;
    Gradient* next_grad;
    Gradient* prev_grad;
    Gradient* max_grad;

    grad_row = *grad_row_in_out;
    grad_col = *grad_col_in_out;

    curr_grad_row = grad_row;
    curr_grad_col = grad_col;
    curr_grad     = &map[ curr_grad_row ][ curr_grad_col ];

    prev_grad_row = curr_grad_row;
    prev_grad_col = curr_grad_col;
    prev_grad     = follow_gradient(&prev_grad_row, &prev_grad_col,
                                      -(curr_grad->dcol), -(curr_grad->drow),
                                      map, num_rows, num_cols, padding);

    next_grad_row = curr_grad_row;
    next_grad_col = curr_grad_col;
    next_grad     = follow_gradient(&next_grad_row, &next_grad_col,
                                      curr_grad->dcol, curr_grad->drow,
                                      map, num_rows, num_cols, padding);

    max_grad = NULL;
    while (max_grad == NULL)
    {
        if (curr_grad->mag >= next_grad->mag && 
            curr_grad->mag >= prev_grad->mag)
        {
            max_grad = curr_grad;
        }
        else if (curr_grad->mag >= prev_grad->mag)
        {
            curr_grad_row = next_grad_row;
            curr_grad_col = next_grad_col;
            curr_grad     = next_grad;
        }
        else
        {
            curr_grad_row = prev_grad_row;
            curr_grad_col = prev_grad_col;
            curr_grad     = prev_grad;
        }

        prev_grad_row = curr_grad_row;
        prev_grad_col = curr_grad_col;
        prev_grad     = follow_gradient(&prev_grad_row, &prev_grad_col,
                -(curr_grad->dcol), -(curr_grad->drow), map, 
                num_rows, num_cols, padding);

        next_grad_row = curr_grad_row;
        next_grad_col = curr_grad_col;
        next_grad     = follow_gradient(&next_grad_row, &next_grad_col,
                                          curr_grad->dcol, curr_grad->drow, 
                                          map, num_rows, num_cols, padding);
    }

    *grad_row_in_out = curr_grad_row;
    *grad_col_in_out = curr_grad_col;

    return max_grad;
}


/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                              follow_edge_right
 *
 *  Forms an edge by following the gradient using hysteresis (to the right)
 *
 * =============================================================================
 */
static Edge_list* follow_edge_right
(
    float        dir_x, 
    float        dir_y, 
    int32_t      grad_row,
    int32_t      grad_col, 
    Gradient** map,
    uint32_t     num_rows,
    uint32_t     num_cols,
    uint32_t     padding,
    float        thresh
)
{
    Gradient*  grad_curr = NULL;
    Gradient*  grad_next = NULL;
    Edge_list* node = NULL;

    grad_curr = &(map[ grad_row ][ grad_col ]);
    
    follow_gradient(&grad_row, &grad_col, dir_x, dir_y, map, 
            num_rows, num_cols, padding);

    grad_next = get_maximal_along_gradient(&grad_row, &grad_col, map, 
            num_rows, num_cols, padding);

    if (!grad_next->marked && grad_next->mag >= thresh &&
        grad_row >= (int32_t) padding && grad_row < (int32_t) (num_rows-padding) &&
        grad_col >= (int32_t) padding && grad_col < (int32_t) (num_cols-padding))
    {
        grad_next->marked = 1;

        node = (Edge_list*) kjb_malloc(sizeof(Edge_list));
        assert(node);

        node->pt = (Edge_point*) kjb_malloc(sizeof(Edge_point));
        assert(node);

        node->pt->row  = grad_row - padding;
        node->pt->col  = grad_col - padding;
        node->pt->dcol = grad_next->dcol;
        node->pt->drow = grad_next->drow;
        node->pt->mag  = grad_next->mag;

        dir_x =  grad_next->drow;
        dir_y = -grad_next->dcol;

        node->next = follow_edge_right(dir_x, dir_y, grad_row, grad_col, map, 
                num_rows, num_cols, padding, thresh);
    }

    return node;
}


/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                              follow_edge_left
 *
 *  Forms an edge by following the gradient using hysteresis (to the left)
 *
 * =============================================================================
 */
static Edge_list* follow_edge_left
(
    float        dir_x, 
    float        dir_y, 
    int32_t      grad_row,
    int32_t      grad_col, 
    Gradient** map,
    uint32_t     num_rows,
    uint32_t     num_cols,
    uint32_t     padding,
    float        thresh
)
{
    Gradient*  grad_curr = NULL;
    Gradient*  grad_next = NULL;
    Edge_list* node = NULL;
    
    grad_curr = &(map[ grad_row ][ grad_col ]);
    
    follow_gradient(&grad_row, &grad_col, dir_x, dir_y, map, 
            num_rows, num_cols, padding);

    grad_next = get_maximal_along_gradient(&grad_row, &grad_col, map, 
            num_rows, num_cols, padding);

    if (!grad_next->marked && grad_next->mag >= thresh &&
        grad_row >= (int32_t)padding && grad_row < (int32_t)num_rows-(int32_t)padding &&
        grad_col >= (int32_t)padding && grad_col < (int32_t)num_cols-(int32_t)padding)
    {
        grad_next->marked = 1;

        node = (Edge_list*) kjb_malloc(sizeof(Edge_list));
        assert(node);

        node->pt = (Edge_point*) kjb_malloc(sizeof(Edge_point));
        assert(node->pt);

        node->pt->row  = grad_row - padding;
        node->pt->col  = grad_col - padding;
        node->pt->dcol = grad_next->dcol;
        node->pt->drow = grad_next->drow;
        node->pt->mag  = grad_next->mag;

        dir_x = -grad_next->drow;
        dir_y =  grad_next->dcol;

        node->next = follow_edge_left(dir_x, dir_y, grad_row, grad_col, map, 
                num_rows, num_cols, padding, thresh);
    }

    return node;
}

Edge_set * create_edge_set
(
    unsigned int num_points,
    unsigned int num_edges,
    const Vector * lengths,
    unsigned int num_rows,
    unsigned int num_cols
)
{
    unsigned int i = 0;
    Edge_set * edge_set = NULL;
    assert(lengths);
    assert( (unsigned int) lengths->length == num_edges);
    edge_set = (Edge_set*) kjb_malloc(sizeof(Edge_set));
    assert(edge_set);
    edge_set->num_edges = num_edges;
    edge_set->total_num_pts = num_points;
    edge_set->edges = NULL;
    if(num_edges == 0)
    {
        return edge_set;
    }
    edge_set->edges = (Edge*) kjb_malloc(num_edges * sizeof(Edge));
    edge_set->edges[0].points = (Edge_point*) kjb_malloc(edge_set->total_num_pts * sizeof(Edge_point));
    assert(edge_set->edges[0].points);
    edge_set->edges[0].num_points = lengths->elements[0];
    edge_set->num_cols = num_cols;
    edge_set->num_rows = num_rows;

    for(i = 1; i < num_edges; i++)
    {
        edge_set->edges[i].num_points = (int)lengths->elements[i];
        edge_set->edges[i].points = edge_set->edges[i-1].points;
        edge_set->edges[i].points += (int)lengths->elements[i-1];
    }
    return edge_set;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                              follow_edge_right
 *
 *  Finds a sets of edges from a gradient map using the Canny algorithm
 *  http://en.wikipedia.org/wiki/Canny_edge_detector
 *
 * =============================================================================
 */
static Edge_set* get_edge_set_from_gradient_map
(
    Gradient**   map, 
    uint32_t       num_rows, 
    uint32_t       num_cols,
    uint32_t       padding,
    float          begin_thresh, 
    float          end_thresh
)
{
    int32_t  grad_row;
    int32_t  grad_col;
    uint32_t row, col;
    float    dir_x;
    float    dir_y;

    Gradient*   grad;
    Gradient*   max_grad;
    Edge_list*  node;
    Edge_list*  right;
    Edge_list*  left;
    Edge_list*  edge_list;
    Edge_set*   edge_set;

    edge_set = (Edge_set*) kjb_malloc(sizeof(Edge_set));
    assert(edge_set);

    edge_set->num_edges = 0;
    edge_set->total_num_pts = 0;
    edge_set->edges = NULL;

    for (row = padding; row < num_rows-padding; row++)
    {
        for (col = padding; col < num_cols-padding; col++)
        {
            grad = &map[ row ][ col ];

            if (!grad->marked && grad->mag >= begin_thresh)
            {
                grad_row = row;
                grad_col = col;

                max_grad = get_maximal_along_gradient(&grad_row, &grad_col, 
                        map, num_rows, num_cols, padding);

                if (!max_grad->marked && 
                    grad_row >= (int32_t)padding && grad_row < (int32_t)num_rows-(int32_t)padding &&
                    grad_col >= (int32_t)padding && grad_col < (int32_t)num_cols-(int32_t)padding)
                {
                    max_grad->marked = 1;

                    node = (Edge_list*) kjb_malloc(sizeof(Edge_list));
                    assert(node);
                    node->next = NULL;

                    node->pt = (Edge_point*) kjb_malloc(sizeof(Edge_point));
                    assert(node->pt);

                    node->pt->row   = grad_row - padding;
                    node->pt->col   = grad_col - padding;
                    node->pt->dcol  = grad->dcol;
                    node->pt->drow  = grad->drow;
                    node->pt->mag   = grad->mag;

                    dir_x =  max_grad->drow;
                    dir_y = -max_grad->dcol;
                    left  = follow_edge_right(dir_x, dir_y, grad_row,
                            grad_col, map, num_rows, num_cols, padding,
                            end_thresh);

                    dir_x = -max_grad->drow;
                    dir_y =  max_grad->dcol;
                    right = follow_edge_left(dir_x, dir_y, grad_row, grad_col,
                            map, num_rows, num_cols, padding, end_thresh);

                    edge_list = merge_edge_lists(node, right);
                    edge_list = merge_edge_lists(left, edge_list);
                    copy_edge_list_into_set(edge_set, edge_list);
                    free_edge_list(edge_list);
                }
            }
        }
    }

    return edge_set;
}



/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                              detect_image_edge_set
 *
 * Smoothed partial derivative kernels, non-maximal suppression, and hysteresis 
 * are used to trace out the edge points from the input image. 
 * http://en.wikipedia.org/wiki/Canny_edge_detector
 *
 * @param  r_edges_out    Result parameter. If @em *r_edges_out is NULL, an
 *                        array of edge points is allocated; otherwise its space
 *                        is re-used.
 * @param  g_edges_out    Result parameter. If @em *g_edges_out is NULL, an 
 *                        array of edge points is allocated; otherwise its space
 *                        is re-used.
 * @param  b_edges_out    Result parameter. If @em *b_edges_out is NULL, an 
 *                        array of edge points is allocated; otherwise its space
 *                        is re-used.
 * @param  img            Image to get the edge points of.
 * @param  sigma          Parameter for the Gaussian kernel used for smoothing
 *                        and edge detection.
 * @param  begin_thresh   Used when scanning the image to look for gradient 
 *                        magnitudes to follow under non-maximal suppression.
 * @param  end_thresh     Once an edge is found under non-maximal 
 *                        suppression, it is followed (perpendicular to the 
 *                        gradient) until the gradient magnitude falls below 
 *                        this threshold.
 * @param  padding        Amount of padding to use around the border of the
 *                        image. The padding is created by extending the image.
 * @param fourier         Specifies whether to use fast fourier transform for
 *                        convolutions or not
 * @param noiseless_data  If set to true, it means that there is no noise, so
 *                        every pixel whose intensity is bigger than 0 is an edge
 *                        point. The blurred image is still used to compute the
 *                        gradient, but edge points are detected in the blur free
 *                        image, by labeling as edge points all pixels in the
 *                        image whose intensity is bigger than 0
 *
 * @return On success, NULL is returned. On error, an Error is returned and @em
 * *[rgb]_edges_out are freed and set to NULL.
 * - @link Error_types::ERROR_INV_ARG ERROR_INV_ARG @endlink 
 *   Sigma is too small.
 *
 * =============================================================================
 */
int detect_image_edge_set
(
    Edge_set**   edges_out,
    const KJB_image* img,
    float          sigma, 
    float          begin_thresh, 
    float          end_thresh,
    uint32_t       padding,
    unsigned char  use_fourier,
    unsigned char  noiseless_data
)
{
    Matrix* m = NULL;

    image_to_matrix_2(img, 0.3, 0.59, 0.11, &m);
    ERE(detect_matrix_edge_set(edges_out, m, sigma, begin_thresh, end_thresh, padding, use_fourier, noiseless_data));

    free_matrix(m);
    
    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/**
 * This image is noise free, meaning that every pixel whose intensity
 * is bigger than 0 is an edge point. Therefore we label as edge points all
 * the pixels in the blur free image whose intensity is bigger than 0.
 * The blurred image is still used to compute the
 * gradient, but we set to 0 the gradient for points that are not edge
 * points in the blur free image, so that they will not be considered when
 * linking edge points into edges
 *
 * @param map The gradient map computed from the blurred image
 * @param m   The original image (prior to blurring)
 * @param padding  Amount of padding added around the border of the
 *                 original image when creating the gradient map
 */
void update_gradient_map_for_noiseless_data(Gradient **map, const Matrix *m, unsigned int padding)
{
    unsigned int i = 0;
    unsigned int j = 0;
    unsigned int num_rows = m->num_rows;
    unsigned int num_cols = m->num_cols;

    for(i = 0; i < num_rows; i++)
    {
        for(j = 0; j < num_cols; j++)
        {
            if(m->elements[i][j] < FLT_EPSILON)
            {
                map[i+padding][j+padding].dcol = 0.0;
                map[i+padding][j+padding].drow = 0.0;
                map[i+padding][j+padding].mag = 0.0;
                map[i+padding][j+padding].marked = 0;
            }
        }
    }
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                              detect_matrix_edge_set
 *
 * Smoothed partial derivative kernels, non-maximal suppression, and hysteresis 
 * are used to trace out the edge points from the input image, stored in a
 *  matrix.
 *
 * @param  edges_out     Result parameter. If @em *edges_out is NULL, an array
 *                       of edge points is allocated; otherwise its space is 
 *                       re-used.
 * @param  m             Matrix to get the edge points of.
 * @param  sigma         Parameter for the Gaussian kernel used for smoothing
 *                       and edge detection.
 * @param  begin_thresh  Used when scanning the image to look for gradient 
 *                       magnitudes to follow under non-maximal suppression.
 * @param  end_thresh    Once an edge is found under non-maximal 
 *                       suppression, it is followed (perpendicular to the 
 *                       gradient) until the gradient magnitude falls below 
 *                       this threshold.
 * @param  padding       Amount of padding to use around the border of the
 *                       matrix. The padding is created by extending the matrix.
 * @param fourier        Specifies whether to use fast fourier transform for
 *                       convolutions or not
 * @param noiseless_data If set to true, it means that there is no noise, so
 *                       every pixel whose intensity is bigger than 0 is an edge
 *                       point. The blurred image is still used to compute the
 *                       gradient, but edge points are detected in the blur free
 *                       image, by labeling as edge points all pixels in the
 *                       image whose intensity is bigger than 0
 *
 *
 * @return On success, NULL is returned. On error, an Error is returned and @em
 * *edges_out are freed and set to NULL.
 * - @link Error_types::ERROR_INV_ARG ERROR_INV_ARG @endlink 
 *   Sigma is too small.
 *
 * =============================================================================
 */
int detect_matrix_edge_set
(
    Edge_set**    edges_out,
    const Matrix* m,
    float           sigma, 
    float           begin_thresh, 
    float           end_thresh,
    uint32_t        padding,
    unsigned char   use_fourier,
    unsigned char   noiseless_data
)
{
    Matrix*    padded_m = NULL;
    Gradient** map = NULL;
    int       e;

    if (*edges_out != NULL)
    {
        free_edge_set(*edges_out);
        *edges_out = NULL;
    }

    ERE(pad_matrix_by_extending(&padded_m, m, padding, padding, padding, padding));

    if ((e =  create_gradient_map(&map, padded_m, sigma, use_fourier)) != NO_ERROR)
    {
        free_matrix(padded_m);
        return e;
    }

    if(noiseless_data)
    {
        update_gradient_map_for_noiseless_data(map, m, padding);
    }

    *edges_out = get_edge_set_from_gradient_map(map, padded_m->num_rows,
            padded_m->num_cols, padding, begin_thresh, end_thresh);

    (*edges_out)->num_rows = m->num_rows;
    (*edges_out)->num_cols = m->num_cols;

    free_matrix(padded_m);
    kjb_free(*map);
    kjb_free(map);

    return NO_ERROR;
}

/*@}*/


/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                              free_edge_set
 *
 * Frees the memory allocated to an edge set
 * =============================================================================
 */
void free_edge_set(Edge_set* edge_set)
{
    if (edge_set == NULL)
    {
        return;
    }

    if (edge_set->total_num_pts > 0)
    {
        kjb_free(edge_set->edges[0].points);
    }
    kjb_free(edge_set->edges);
    kjb_free(edge_set);
}

/*@}*/



/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                             remove_short_edges
 *
 * Removes from the edge set edges whose length is less than min_len
 *
 *
 * @param  edges    Edge set to remove short edges from.
 * @param  min_len  Minimum number of points for an edge. Edges with fewer 
 *                  points are removed.
 *
 * =============================================================================
 */
void remove_short_edges(Edge_set* edges, uint32_t min_len)
{
    uint32_t  i, j, k;
    uint32_t  num_pts_to_remove;
    uint32_t  num_edges_to_remove;

    Edge_point*  pt;
    Edge* old_edges;

    if(edges == NULL) return;

    num_pts_to_remove = 0;
    num_edges_to_remove = 0;

    for (i = 0; i < edges->num_edges; i++)
    {
        if (edges->edges[ i ].num_points < min_len)
        {
            num_pts_to_remove += edges->edges[ i ].num_points;
            num_edges_to_remove++;
        }
    }

    if (num_edges_to_remove == 0)
    {
        assert(num_pts_to_remove == 0);
        return;
    }

    old_edges = edges->edges;

    edges->num_edges -= num_edges_to_remove;
    edges->total_num_pts -= num_pts_to_remove;

    edges->edges = (Edge*) kjb_malloc(edges->num_edges * sizeof(Edge));
    assert(edges->edges);

    edges->edges[0].points = (Edge_point*) kjb_malloc(edges->total_num_pts * sizeof(Edge_point));
    assert(edges->edges[0].points);

    i = 0;
    pt = edges->edges[0].points;

    for (j = 0; j < edges->num_edges + num_edges_to_remove; j++)
    {
        if (old_edges[ j ].num_points >= min_len)
        {
            edges->edges[i].num_points = old_edges[ j ].num_points;

            edges->edges[ i ].points = pt;

            for (k = 0; k < edges->edges[ i ].num_points; k++)
            {
                *pt = old_edges[ j ].points[ k ];
                pt++;
            }

            i++;
        }
    }

    kjb_free(old_edges[0].points);
    kjb_free(old_edges);
}

/*@}*/



/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                        recursively_break_edges_at_corners
 *
 * Recursively breaks apart edges where the gradient direction changes
 * significantly. It does so by finding the edge point with the largest gradient
 * difference on either side, breaks the edge, and recurses on the two newly
 * created edges.
 *
 * @param  edges    Edge set to break apart.
 * @param  i        The index in the edge set to recurse on.
 * @param  thresh   If the dot product of the averaged vectors is less than 
 *                  this threshold, the edge is broken.
 * @param  num_avg  Number of gradient vectors to average into one when 
 *                  computing the dot product.
 * =============================================================================
 */
static void recursively_break_edges_at_corners
(
    Edge_set* edges, 
    uint32_t    i, 
    float       thresh, 
    uint32_t    num_avg
)
{
    uint32_t j, k;
    uint32_t min_j = 0;

    float  dcol_1, drow_1;
    float  dcol_2, drow_2;
    float  dp;
    float  min_dp;

    assert(num_avg > 0);

    if (edges->edges[ i ].num_points < 2*num_avg)
        return;

    min_dp = thresh;

    for (j = num_avg-1; j < edges->edges[ i ].num_points - num_avg; j++)
    {
        dcol_1 = drow_1 = 0;
        dcol_2 = drow_2 = 0;

        for (k = 0; k < num_avg; k++)
        {
            dcol_1 += edges->edges[ i ].points[ j-k ].dcol;
            drow_1 += edges->edges[ i ].points[ j-k ].drow;
            dcol_2 += edges->edges[ i ].points[ j+k+1 ].dcol;
            drow_2 += edges->edges[ i ].points[ j+k+1 ].drow;
        }

        dcol_1 /= num_avg;
        drow_1 /= num_avg;
        dcol_2 /= num_avg;
        drow_2 /= num_avg;

        dp = fabs(dcol_1*dcol_2 + drow_1*drow_2);
        if (dp < min_dp)
        {
            min_j = j;
            min_dp = dp;
        }
    }

    if (min_dp < thresh)
    {
        edges->num_edges++;

        edges->edges = (Edge*) kjb_realloc(edges->edges,
                edges->num_edges * sizeof(Edge));
        assert(edges->edges);

        for (j = edges->num_edges-1; j > i; j--)
        {
            Edge* cur_edge  = &edges->edges[ j ];
            Edge* prev_edge = &edges->edges[ j - 1];

            cur_edge->points = prev_edge->points;
            cur_edge->num_points = prev_edge->num_points;
        }

        edges->edges[i].num_points      = min_j+1;
        edges->edges[ i+1 ].num_points -= min_j+1;
        edges->edges[ i+1 ].points     += min_j+1;

        recursively_break_edges_at_corners(edges, i+1, thresh, num_avg);
        recursively_break_edges_at_corners(edges, i, thresh, num_avg);
    }
}


/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                          break_edges_at_corners
 *
 * Breaks apart edges where the gradient direction changes
 * significantly. It does so by finding the edge point with the largest gradient
 * difference on either side, breaks the edge, and recurses on the two newly
 * created edges.
 *
 * @param  edges    Edge set to break apart.
 * @param  thresh   If the dot product of the averaged vectors is less than 
 *                  this threshold, the edge is broken.
 * @param  num_avg  Number of gradient vectors to average into one when 
 *                  computing the dot product.
 *
 * =============================================================================
 */
void break_edges_at_corners(Edge_set* edges, float thresh, uint32_t num_avg)
{
    uint32_t i;

    if(edges == NULL) 
        return;

    if (num_avg == 0)
        return;

    for (i = 0; i < edges->num_edges; i++)
    {
        recursively_break_edges_at_corners(edges, i, thresh, num_avg);
    }
}

/*@}*/

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                            sample_edge_set
 *
 * Randomly samples edge points from an array of them.
 *
 * @param  edges_out    Result parameter. If @em *edges_out is NULL, a set
 *                      of edges is allocated; otherwise its space is 
 *                      re-used.
 * @param  edges_in     Set of edges to sample from.
 * @param  p            Bernoulli parameter; probability of sampling an edge 
 *                      point.
 *
 * @note If @em *edges_out == @em edges_in, then @em edges_in is overwritten.
 *
 * =============================================================================
 */
void sample_edge_set
(
    Edge_set**      edges_out, 
    const Edge_set* edges_in, 
    float             p
)
{
    uint32_t      e, pt;
    Edge* cur_edge;
    Edge_point* pts;
    Edge_set*   edges;

    if(edges_in == NULL)
    {
        free_edge_set(*edges_out);
        *edges_out = NULL;
    }
        

    assert(edges_in->total_num_pts > 0);

    if (*edges_out == NULL || *edges_out == edges_in)
    {
        edges = (Edge_set*) kjb_malloc(sizeof(Edge_set));
        assert(edges);

        edges->edges = (Edge*) kjb_malloc(edges_in->num_edges * sizeof(Edge));
        assert(edges->edges);

        edges->edges[0].points = (Edge_point*) kjb_malloc(edges_in->total_num_pts * 
                sizeof(Edge_point));
        assert(edges->edges[0].points);
    }
    else
    {
        edges = *edges_out;

        edges->edges = (Edge*) kjb_realloc(edges->edges, 
                edges_in->num_edges * sizeof(Edge));
        if (edges_in->num_edges > 0) assert(edges->edges);

        edges->edges[0].points = (Edge_point*) kjb_realloc(edges->edges[0].points, 
                edges_in->total_num_pts * sizeof(Edge_point));
        if (edges_in->total_num_pts > 0) assert(edges->edges[0].points);
    }

    pts = edges->edges[0].points;
    cur_edge = &edges->edges[0];

    edges->total_num_pts = 0;
    edges->num_edges = 0;
    edges->num_rows = edges_in->num_rows;
    edges->num_cols = edges_in->num_cols;

    /* randomly sample edge points */
    for (e = 0; e < edges_in->num_edges; e++)
    {
        cur_edge->num_points = 0;

        for (pt = 0; pt < edges_in->edges[e].num_points; pt++)
        {
            if ((kjb_rand() < p))
            {
                pts->row  = edges_in->edges[ e ].points[ pt ].row;
                pts->col  = edges_in->edges[ e ].points[ pt ].col;
                pts->dcol = edges_in->edges[ e ].points[ pt ].dcol;
                pts->drow = edges_in->edges[ e ].points[ pt ].drow;
                pts->mag  = edges_in->edges[ e ].points[ pt ].mag;

                cur_edge->num_points++;

                edges->total_num_pts++;

                pts++;
            }
        }

        /* if any points were sampled from this edge, add it to the set */
        if (cur_edge->num_points != 0)
        {
            cur_edge++;
            edges->num_edges++;
        }
    }

    /* compact unused allocated space */
    edges->edges = (Edge*) kjb_realloc(edges->edges, 
            edges->num_edges * sizeof(Edge));
    if (edges->num_edges > 0) assert(edges->edges);

    edges->edges[0].points = (Edge_point*) kjb_realloc(edges->edges[0].points, 
            edges->total_num_pts * sizeof(Edge_point));
    if (edges->total_num_pts > 0) assert(edges->edges[0].points);

    /* set up edge pointers */
    pts = edges->edges[0].points;
    for (e = 1; e < edges->num_edges; e++)
    {
        pts += edges->edges[ e-1 ].num_points;
        edges->edges[ e ].points = pts;
    }

    if (*edges_out == edges_in)
    {
        /* This makes edges_in point to an invalid memory address. */
        /* TODO: Consider refactoring. */
        free_edge_set(*edges_out);
    }

    *edges_out = edges;
}

/*@}*/


/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                            color_edge_set
 *
 * Colors all the edges in the edge set with the specified color
 *
 *
 * @param  img_out  Result parameter. If @em *img_out is NULL, an image is 
 *                  allocated; otherwise its space is re-used.
 * @param  img_in   Image to color the edge points on.
 * @param  edges    Edge set to use.
 * @param  pxl      Pixel to use for coloring the edge points.
 *
 * @return On success, NULL is returned. On error, an Error is returned and @em
 * *img_out is freed and set to NULL.
 * - Error_types::ERROR_INV_ARG  Edge points indexed outside @em img_in.
 *   The file to read from is not formatted properly.
 *
 * =============================================================================
 */
int color_edge_set
(
    KJB_image**         img_out,
    const KJB_image*    img_in,
    const Edge_set* edges, 
    const Pixel*    pxl
)
{
    uint32_t e;


    if (*img_out != img_in)
    {
        kjb_copy_image(img_out, img_in);
    }

    if(img_in == NULL)
        return NO_ERROR;

    for (e = 0; e < edges->num_edges; e++)
    {
        ERE(color_edge_points(img_out, *img_out, edges->edges[ e ].points,
                edges->edges[ e ].num_points, pxl));
    }

    return NO_ERROR;
}

/*@}*/

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                            randomly_color_edge_set
 *
 * Colors each edge in the edge set with a different random color
 *
 * @param  img_out  Result parameter. If @em *img_out is NULL, an image is 
 *                  allocated; otherwise its space is re-used.
 * @param  img_in   Image to color the edge points on.
 * @param  edges    Edge set to use.
 *
 * @return On success, NULL is returned. On error, an Error is returned and @em
 * *img_out is freed and set to NULL.
 * - Error_types::ERROR_INV_ARG  Edge points indexed outside @em img_in.
 *   The file to read from is not formatted properly.
 *
 * =============================================================================
 */
int randomly_color_edge_set
(
    KJB_image**         img_out,
    const KJB_image*    img_in,
    const Edge_set* edges
)
{
    uint32_t e;
    int err;

    if (*img_out != img_in)
    {
        kjb_copy_image(img_out, img_in);
    }

    if(edges == NULL)
        return NO_ERROR;

    for (e = 0; e < edges->num_edges; e++)
    {
        if ((err = randomly_color_edge_points(img_out, *img_out, 
                        edges->edges[ e ].points, edges->edges[ e ].num_points)))
        {
            return err;
        }
    }

    return NO_ERROR;
}

/*@}*/



/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                            color_edge_set
 *
 * Colors edge points in an image using the input color
 *
 * @param  img_out  Result parameter. If @em *img_out is NULL, an image is 
 *                  allocated; otherwise its space is re-used.
 * @param  img_in   Image to color the edge points on.
 * @param  pts      Edge points to use.
 * @param  num_pts  Number of elements in @em pts.
 * @param  pxl      Pixel to use for coloring the edge points.
 *
 * @return On success, NULL is returned. On error, an Error is returned and @em
 * *img_out is freed and set to NULL.
 * - Error_types::ERROR_INV_ARG  Edge points indexed outside @em img_in.
 *   The file to read from is not formatted properly.
 *
 * =============================================================================
 */
int color_edge_points
(
    KJB_image**           img_out,
    const KJB_image*      img_in,
    const Edge_point* pts, 
    uint32_t            num_pts,
    const Pixel*      pxl
)
{
    uint32_t pt;
    int row, col;
    KJB_image* img;

    if (*img_out != img_in)
    {
        kjb_copy_image(img_out, img_in);
    }
    img = *img_out;

    for (pt = 0; pt < num_pts; pt++)
    {
        row = pts[ pt ].row;
        col = pts[ pt ].col;

        if (row >= img_in->num_rows || col >= img_in->num_cols)
        {
            if (*img_out != img_in)
            {
                kjb_free_image(*img_out);
                *img_out = 0;
            }

            set_error("%s:%d %s - %s", __FILE__, __LINE__, "color_edge_points", "Edge points outside image to color in");
            return ERROR;
        }

        img->pixels[ row ][ col ] = *pxl;
    }

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                            color_edge_set
 *
 * Colors edge points in an image using the input color
 *
 * @param  img_out  Result parameter. If @em *img_out is NULL, an image is
 *                  allocated; otherwise its space is re-used.
 * @param  img_in   Image to color the edge points on.
 * @param  pt       Edge points to draw.
 * @param  pxl      Pixel to use for coloring the edge point.
 *
 * @return On success, NULL is returned. On error, an Error is returned and @em
 * *img_out is freed and set to NULL.
 * - Error_types::ERROR_INV_ARG  Edge points indexed outside @em img_in.
 *   The file to read from is not formatted properly.
 *
 * =============================================================================
 */
int color_edge_point
(
    KJB_image**           img_out,
    const KJB_image*      img_in,
    const Edge_point* pt,
    const Pixel*      pxl
)
{
    int row, col;
    KJB_image* img;

    if (*img_out != img_in)
    {
        kjb_copy_image(img_out, img_in);
    }
    img = *img_out;

    row = pt->row;
    col = pt->col;

    if (row >= img_in->num_rows || col >= img_in->num_cols)
    {
        if (*img_out != img_in)
        {
            kjb_free_image(*img_out);
            *img_out = 0;
        }

        set_error("%s:%d %s - %s", __FILE__, __LINE__, "color_edge_points", "Edge points outside image to color in");
        return ERROR;
    }

    img->pixels[ row ][ col ] = *pxl;

    return NO_ERROR;
}

/*@}*/

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                         randomly_color_edge_set
 *
 * Randomly colors edge points in an image with a randomly selected color.
 *
 * @param  img_out  Result parameter. If @em *img_out is NULL, an image is 
 *                  allocated; otherwise its space is re-used.
 * @param  img_in   Image to color the edge points on.
 * @param  pts      Edge points to use.
 * @param  num_pts  Number of elements in @em pts.
 *
 * @return On success, NULL is returned. On error, an Error is returned and @em
 * *img_out is freed and set to NULL.
 * - Error_types::ERROR_INV_ARG  Edge points indexed outside @em img_in.
 *   The file to read from is not formatted properly.
 *
 * =============================================================================
 */
int randomly_color_edge_points
(
    KJB_image**           img_out,
    const KJB_image*      img_in,
    const Edge_point* pts, 
    uint32_t            num_pts
)
{
    Pixel pxl;

    pxl.r = (float)kjb_rand()*KJB_EDGE_MAX_RGB_VALUE;
    pxl.g = (float)kjb_rand()*KJB_EDGE_MAX_RGB_VALUE;
    pxl.b = (float)kjb_rand()*KJB_EDGE_MAX_RGB_VALUE;

    return color_edge_points(img_out, img_in, pts, num_pts, &pxl);
}

/*@}*/


/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                             read_edge_set
 *
 * Reads a set of edges in ASCII format from a file.
 * The first line of the file must contains the number of edges, the second has
 * the total number of edge points and the following lines have the number of
 * points in each edge followed by the actual points. For example,
 *
 * @code
 * num_rows=100
 * num_cols=100
 * num_edges=2
 * total_num_points=5
 * num_pts=3
 * col=0 row=0 dcol=1 drow=1 mag=1
 * col=9 row=8 dcol=4 drow=1 mag=1
 * col=1 row=2 dcol=2 drow=1 mag=1
 * num_pts=2
 * col=9 row=8 dcol=4 drow=1 mag=1
 * col=1 row=2 dcol=2 drow=1 mag=1
 * @endcode
 *
 * @param  edges_out  Result parameter. If @em *edges_out is NULL, an edge set 
 *                    is allocated; otherwise its space is re-used. 
 * @param  fname      Name of the file to read from.
 *
 * @return On success, NULL is returned. On error, an Error is returned and @em
 * *edges_out is freed and set to NULL.
 * - @link Error_types::ERROR_INV_ARG ERROR_INV_ARG @endlink 
 *   The file to read from is not formatted properly.
 * - @link Error_types::ERROR_IO ERROR_IO @endlink 
 *   Could not read from @em fname.
 *
 * =============================================================================
 */
int read_edge_set
(
    Edge_set** edges_out,
    const char*  fname
)
{
    int err = NO_ERROR;
    char* buffer;
    size_t buffer_size;
    size_t bytes_read = 0;
    FILE* fp = NULL;

    if ((fp = fopen(fname, "r")) == NULL)
    {
            set_error("%s:%d %s - %s",
                    __FILE__,
                    __LINE__,
                    "read_edge_set",
                    strerror(errno));
            return ERROR;
    }

    fseek(fp, 0, SEEK_END);
    buffer_size = ftell(fp);
    rewind(fp);

    buffer = (char*) kjb_malloc(sizeof(char) * buffer_size+1);

    if(buffer == NULL)
    {
        set_error("%s:%d %s",
                __FILE__,
                __LINE__,
                "Memory allocation error.");
        err = ERROR;
        goto cleanup;
    }

    /* read entire file into buffer */
    bytes_read = fread(buffer, 1, buffer_size, fp);
    buffer[buffer_size] = '\0';

    if(ferror(fp))
    {
        kjb_free(buffer);
        set_error("%s:%d %s - %s",
                __FILE__,
                __LINE__,
                "read_edge_set",
                strerror(errno));
        err = ERROR;
        goto cleanup;
    }

    EGC(err = unserialize_edge_set(edges_out, buffer));

cleanup:
    if (fp && fclose(fp) != 0)
    {
        free_edge_set(*edges_out); *edges_out = NULL;
        kjb_free(buffer);

        set_error(
                "%s:%d %s - %s",
                __FILE__,
                __LINE__,
                "read_edge_set",
                strerror(errno));
        err = ERROR;
    }

    kjb_free(buffer);
    return err;
}


/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                             unserialize_edge_set
 *
 * @precondition buffer contains the entire serialized edge set
 *
 * =============================================================================
 */
int unserialize_edge_set
(
    Edge_set** edges_out,
    const char* buffer
)
{
    uint32_t i, j;

    Edge_set*  edges;
    Edge_point* pt;

    char error = 0;

    if(*edges_out != NULL)
    {
        free_edge_set(*edges_out);
    }

    *edges_out = (Edge_set*) kjb_malloc(sizeof(Edge_set));
    assert(*edges_out);

    edges = *edges_out;
    memset(edges, 0, sizeof(Edge_set));

    if(sscanf(buffer, "num_rows=%u\n", &(edges->num_rows)) != 1 )
    {
        error = 1;
        goto finish;
    }

    buffer = strchr(buffer, '\n')  + 1;

    if(sscanf(buffer, "num_cols=%u\n", &(edges->num_cols)) != 1 )
    {
        error = 1;
        goto finish;
    }

    buffer = strchr(buffer, '\n') + 1;

    if(sscanf(buffer, "num_edges=%u\n", &(edges->num_edges)) != 1 )
    {
        error = 1;
        goto finish;
    }

    buffer = strchr(buffer, '\n') + 1;

    if(sscanf(buffer, "total_num_pts=%u\n", &(edges->total_num_pts)) != 1)
    {
        error = 1;
        goto finish;
    }

    buffer = strchr(buffer, '\n') + 1;

    edges->edges = (Edge*) kjb_malloc(edges->num_edges * sizeof(Edge));
    assert(edges->edges);

    assert(edges->num_edges > 0);
    edges->edges[0].points = (Edge_point*) kjb_malloc(edges->total_num_pts * sizeof(Edge_point));
    assert(edges->edges[0].points);

    pt = edges->edges[0].points;

    for (i = 0; i < edges->num_edges; i++)
    {
        if (sscanf(buffer, "num_pts=%u\n", &(edges->edges[ i ].num_points)) != 1)
        {
            error = 1;
            goto finish;
        }

        buffer = strchr(buffer, '\n') + 1;

        edges->edges[ i ].points = pt;

        for (j = 0; j < edges->edges[ i ].num_points; j++)
        {
            if (sscanf(buffer, "col=%u row=%u dcol=%lf drow=%lf mag=%lf\n", 
                        &(pt->col), &(pt->row), &(pt->dcol), &(pt->drow),
                        &(pt->mag)) != 5)
            {
                error = 1;
                goto finish;
            }

            buffer = strchr(buffer, '\n') + 1;

            pt++;
        }
    }

finish:
    if(error)
    {
        set_error(
                "%s:%d %s - %s",
                __FILE__,
                __LINE__,
                "read_edge_points",
                "File not properly formatted to read edge points");
        free_edge_set(*edges_out); *edges_out = NULL;
        return ERROR;
    }

    return NO_ERROR;
}

/*@}*/



/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                            write_edge_set
 *
 * Writes a set of edges in ASCII format to a file.
 * The first line of the file contains the number of edges, the second has the
 * total number of edge points and the following lines have the number of points
 * in each edge followed by the actual points. For example,
 *
 * @code
 * num_rows=100
 * num_cols=100
 * num_edges=2
 * total_num_points=5
 * num_pts=3
 * col=0 row=0 dcol=1 drow=1 mag=1
 * col=9 row=8 dcol=4 drow=1 mag=1
 * col=1 row=2 dcol=2 drow=1 mag=1
 * num_pts=2
 * col=9 row=8 dcol=4 drow=1 mag=1
 * col=1 row=2 dcol=2 drow=1 mag=1
 * @endcode
 *
 * @param  edges  Edge set to write.
 * @param  fname  Name of the file to write to.
 *
 * @return On success, NULL is returned. On error, an Error is returned.
 * - @link Error_types::ERROR_IO ERROR_IO @endlink 
 *   Could not read from @em fname.
 *
 * =============================================================================
 */
int write_edge_set
(
    const Edge_set* edges,
    const char*       fname
)
{
    char* _buffer;
    size_t length;
    int err = NO_ERROR;
    FILE* fp;


    if ((fp = fopen(fname, "w")) == NULL)
    {
        set_error(
                "%s:%d %s - %s",
                __FILE__,
                __LINE__,
                "write_edge_set",
                strerror(errno));
        return ERROR;
    }

    serialize_edge_set(edges, &_buffer, &length);

    fwrite(_buffer, length, 1, fp);

    if (fclose(fp) != 0)
    {
        kjb_free(_buffer);
        set_error(
                "%s:%d %s - %s",
                __FILE__,
                __LINE__,
                "write_edge_set",
                strerror(errno));
        err = ERROR;
        goto cleanup;
    }

cleanup:
    kjb_free(_buffer);
    return err;
}

#define ENSURE_CAPACITY(buf, buflen, used, length) \
    { \
    while(buflen <= used + length) \
    { \
        buflen *= 2; \
    } \
    NRE(buf = (char*) kjb_realloc(buf, sizeof(char) * buflen)); \
\
    } \
    /* will #undef at end of function */

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                            write_edge_set
 *
 * Serializes an edge_set
 * =============================================================================
 */
int serialize_edge_set
(
    const Edge_set* edges,
    char** out_buffer,
    size_t* length
)
{
    uint32_t i, j;
    size_t buffer_size;
    size_t cur_length = 0;

    const Edge_point* pt;

    /* These are ESTIMATES */
    const size_t SET_HEADER_SIZE = 80;
    const size_t EDGE_HEADER_SIZE = 20;
    const size_t POINT_HEADER_SIZE = 80;
    const size_t CUSHION = 64; /* All calls to sprintf are assumed to be shorter than this */

    /* Initial guess as to buffer size */
    buffer_size = SET_HEADER_SIZE +
        edges->num_edges * EDGE_HEADER_SIZE +
        edges->total_num_pts * POINT_HEADER_SIZE + 
        + CUSHION;

    NRE(*out_buffer = (char*) kjb_malloc(sizeof(char) * buffer_size));


    ENSURE_CAPACITY(*out_buffer, buffer_size, cur_length, 4 * CUSHION);
    cur_length += sprintf(*out_buffer + cur_length, "num_rows=%u\n", edges->num_rows);
    cur_length += sprintf(*out_buffer + cur_length, "num_cols=%u\n", edges->num_cols);
    cur_length += sprintf(*out_buffer + cur_length, "num_edges=%u\n", edges->num_edges);
    cur_length += sprintf(*out_buffer + cur_length, "total_num_pts=%u\n", edges->total_num_pts);

    for (i = 0; i < edges->num_edges; i++)
    {
        ENSURE_CAPACITY(*out_buffer, buffer_size, cur_length, CUSHION);
        cur_length += sprintf(*out_buffer + cur_length, "num_pts=%u\n", edges->edges[ i ].num_points);

        for (j = 0; j < edges->edges[ i ].num_points; j++)
        {
            pt = &(edges->edges[ i ].points[ j ]);

            ENSURE_CAPACITY(*out_buffer, buffer_size, cur_length, 3 * CUSHION);
            cur_length += sprintf(*out_buffer + cur_length, "col=%u row=%u", pt->col, pt->row);
            cur_length += sprintf(*out_buffer + cur_length, " dcol=%f drow=%f", pt->dcol, pt->drow);
            cur_length += sprintf(*out_buffer + cur_length, " mag=%f\n", pt->mag);
        }
    }

    /* shrink to size */
    *out_buffer = (char*) kjb_realloc(*out_buffer, sizeof(char) * cur_length);
    *length = cur_length;

    return NO_ERROR;

}
#undef ENSURE_CAPACITY

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                            copy_edge_set
 *
 * Copies edge_set edges_in into edge_set an edges_out
 * =============================================================================
 */
int copy_edge_set(Edge_set** edges_out, const Edge_set* edges_in)
{
    Edge_set* out = NULL;
    Edge_point* pt_ptr = NULL;
    size_t e;

    UNTESTED_CODE();

    if(*edges_out != NULL)
    {
        free_edge_set(*edges_out);
        *edges_out = NULL;
    }
    
    if(edges_in == NULL)
    {
        return NO_ERROR;
    }

    *edges_out = (Edge_set*) kjb_malloc(sizeof(Edge_set));
    out = *edges_out;

    *out = *edges_in;
    out->edges = NULL;

    if(edges_in->num_edges == 0)
        return NO_ERROR;

    out->edges = (Edge*) kjb_malloc(sizeof(Edge) * edges_in->num_edges);

    kjb_memcpy(out->edges,
            edges_in->edges,
            sizeof(Edge) * edges_in->num_edges);

    out->edges[0].points = (Edge_point*) kjb_malloc(sizeof(Edge_point) * edges_in->total_num_pts);

    kjb_memcpy(out->edges[0].points,
            edges_in->edges[0].points,
            sizeof(Edge_point) * edges_in->total_num_pts);

    pt_ptr = out->edges[0].points;

    for(e = 0; e < edges_in->num_edges; e++)
    {
        out->edges[e].points = pt_ptr;
        pt_ptr += edges_in->edges[e].num_points;
    }

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                            remove_edge
 *
 * Removes the edge at index edge_id
 * =============================================================================
 */
/**
 *  @param edges the edge_set
 *  @param edge_id the index of the edge to remove
 *  @return ERROR if the edge index is out of bounds
 */
int remove_edge(Edge_set* edges, unsigned int edge_id)
{
    uint32_t  i, j, k;
    uint32_t  num_pts_to_remove;
    uint32_t  num_edges_to_remove = 1;

    Edge_point*  pt;
    Edge* old_edges;

    if(edges == NULL) return ERROR;
    if(edges->num_edges <= edge_id)
    {
        return ERROR;
    }

    num_pts_to_remove = edges->edges[ edge_id ].num_points;
    num_edges_to_remove = 1;

    old_edges = edges->edges;

    edges->num_edges -= num_edges_to_remove;
    edges->total_num_pts -= num_pts_to_remove;

    edges->edges = (Edge*) kjb_malloc(edges->num_edges * sizeof(Edge));
    assert(edges->edges);

    edges->edges[0].points = (Edge_point*) kjb_malloc(edges->total_num_pts * sizeof(Edge_point));
    assert(edges->edges[0].points);

    i = 0;
    pt = edges->edges[0].points;

    for (j = 0; j < (edges->num_edges + 1); j++)
    {
        if ( j != edge_id)
        {
            edges->edges[i].num_points = old_edges[ j ].num_points;

            edges->edges[ i ].points = pt;

            for (k = 0; k < edges->edges[ i ].num_points; k++)
            {
                *pt = old_edges[ j ].points[ k ];
                pt++;
            }

            i++;
        }
    }

    kjb_free(old_edges[0].points);
    kjb_free(old_edges);
    return NO_ERROR;
}


/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                            find_edge_index
 *
 * Finds the index where the Edge pointed by iedge is stored in the
 * input edge_set. Notice that this is based on) pointers (the pointer address
 * is compared, not the edge content)
 * =============================================================================
 */
/**
 *  @param edges the edge_set
 *  @param iedge the edge to find
 *  @param index will contain the index where iedge is stored
 *  @return ERROR if the edge is not found in the edge set
 */
int find_edge_index(const Edge_set* edges, const Edge * iedge, unsigned int *index)
{
    unsigned int i = 0;
    if( (index == NULL) || (edges == NULL) )
    {
        return ERROR;
    }
    for( i = 0; i < edges->num_edges ; i++)
    {
        if( (&(edges->edges[i])) == iedge)
        {
            *index = i;
            return NO_ERROR;
        }
    }
    return ERROR;
}


void find_edge_length_from_end_points
(
    double x_0,
    double y_0,
    double x_1,
    double y_1,
    unsigned int * count
)
{

     int steep = 0;
     int y;
     int delta_x, delta_y, error, x;
     int y_step = -1;

     (*count) = 0;

     if(fabs(y_1 - y_0) > fabs(x_1 - x_0))
     {
        steep = 1;
     }
     if(steep)
     {
         double temp = x_0;
         x_0 = y_0;
         y_0 = temp;
         temp = x_1;
         x_1 = y_1;
         y_1 = temp;
     }
     if(x_0 > x_1)
     {
         double temp = x_0;
         x_0 = x_1;
         x_1 = temp;
         temp = y_0;
         y_0 = y_1;
         y_1 = temp;
     }
     delta_x = kjb_rint(x_1 - x_0);
     delta_y = kjb_rint(fabs(y_1 - y_0));
     error = delta_x / 2;
     y = y_0;
     if(y_0 < y_1)
     {
         y_step = 1;
     }
     for(x=x_0; x <= x_1; x++)
     {
         (*count)++;
         error -= delta_y;
         if(error < 0)
         {
             y += y_step;
             error += delta_x;
         }
     }
}

void create_edge_from_end_points
(
    double x_0,
    double y_0,
    double x_1,
    double y_1,
    Edge_set * edge_set,
    int index
)
{

     int steep = 0;
     int y;
     int delta_x, delta_y, error, x;
     int y_step = -1;
     int count = 0;
     double dcol, drow, mag;
     drow = -(x_1 - x_0);
     dcol = (y_1 - y_0);

     mag = sqrt( (dcol*dcol) + (drow*drow) );
     dcol /= mag;
     drow /= mag;

     if(fabs(y_1 - y_0) > fabs(x_1 - x_0))
     {
        steep = 1;
     }
     if(steep)
     {
         double temp = x_0;
         x_0 = y_0;
         y_0 = temp;
         temp = x_1;
         x_1 = y_1;
         y_1 = temp;
     }
     if(x_0 > x_1)
     {
         double temp = x_0;
         x_0 = x_1;
         x_1 = temp;
         temp = y_0;
         y_0 = y_1;
         y_1 = temp;
     }
     delta_x = kjb_rint(x_1 - x_0);
     delta_y = kjb_rint(fabs(y_1 - y_0));
     error = delta_x / 2;
     y = y_0;
     if(y_0 < y_1)
     {
         y_step = 1;
     }
     for(x=x_0; x <= x_1; x++)
     {
         if(steep)
         {
             if(y < 0 )
             {
                 edge_set->edges[index].points[count].col = 0;
             }
             else if(y >= (int)(edge_set->num_cols) )
             {
                 edge_set->edges[index].points[count].col = edge_set->num_cols -1;
             }
             else
             {
                 edge_set->edges[index].points[count].col = y;
             }
             if(x < 0)
             {
                 edge_set->edges[index].points[count].row = 0;
             }
             else if( x >= (int)(edge_set->num_rows) )
             {
                 edge_set->edges[index].points[count].row = edge_set->num_rows -1;
             }
             else
             {
                 edge_set->edges[index].points[count].row = x;
             }
             edge_set->edges[index].points[count].dcol = dcol;
             edge_set->edges[index].points[count].drow = drow;
             edge_set->edges[index].points[count].mag = 0.0;
             count++;
             /*do y,x*/
         }
         else
         {
             /*do x,y*/
             if(x < 0)
             {
                 edge_set->edges[index].points[count].col = 0;
             }
             else if(x >= (int)(edge_set->num_cols) )
             {
                 edge_set->edges[index].points[count].col = edge_set->num_cols-1;
             }
             else
             {
                 edge_set->edges[index].points[count].col = x;
             }
             if(y < 0)
             {
                 edge_set->edges[index].points[count].row = 0;
             }
             else if(y >= (int)(edge_set->num_rows))
             {
                 edge_set->edges[index].points[count].row = edge_set->num_rows -1;
             }
             else
             {
                 edge_set->edges[index].points[count].row = y;
             }
             edge_set->edges[index].points[count].dcol = dcol;
             edge_set->edges[index].points[count].drow = drow;
             edge_set->edges[index].points[count].mag = 0.0;
             count++;
         }
         error -= delta_y;
         if(error < 0)
         {
             y += y_step;
             error += delta_x;
         }
     }
}
