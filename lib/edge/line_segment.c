
/* $Id $ */

/**
 * @file
 *
 * @author Luca Del Pero
 *
 * @brief Line segment
 *
 * Copyright (c) 2003-2008 by members of University of Arizona Computer Vision
 * group. For use outside the University of Arizona Computer Vision group
 * please contact Kobus Barnard.
 */

#include "edge/line_segment.h"
#include "l/l_math.h"

#ifdef __cplusplus
extern "C" {
#endif
/**
 * This function will fit a line segment from a detected collection of edge points
 * I am waiting for Kyle to re-implement edge before writing it
 */                                        
/**
 * @brief This function fits a line segment to a detected edge by least sqaure 
 * @return the least square fitting error
 */
double fit_line_segment_to_edge_by_least_square
(
    Line_segment_s*         line_p, 
    const Edge*             edge_p 
)
{
    double sum_x = 0.0;
    double sum_x_square = 0.0;
    double sum_y = 0.0;
    double sum_xy = 0.0;
    double epsilon = 1e-100;
    double error = 0.0;
    uint32_t num_points = edge_p->num_points;
    const Edge_point* edge_point_p = edge_p->points;
    uint32_t i, row, col;
    double denominator, b, appr;
    double beg_x = edge_point_p[ 0 ].col;
    double beg_y = edge_point_p[ 0 ].row;
    double end_x = edge_point_p[ num_points-1 ].col;
    double end_y = edge_point_p[ num_points-1 ].row;

    line_p->length = sqrt(pow((beg_x - end_x), 2) + pow((beg_y - end_y),2));
    line_p->centre_x = (beg_x + end_x)/2;
    line_p->centre_y = (beg_y + end_y)/2;
    line_p->strength = compute_average_gradient(edge_p);

    /* Using least square to fit the edge to a line */
    for(i = 0; i < num_points; i++)
    {
         row = edge_point_p[ i ].row;
         col = edge_point_p[ i ].col;
         sum_x += col;
         sum_y += row;
         sum_x_square += col*col;
         sum_xy += row*col;
    }
    denominator = (num_points*sum_x_square) - (sum_x*sum_x);
    if(fabs(denominator) < epsilon)
    {
        line_p->orientation = M_PI/2;
        /*HORRIBLE HACK*/
        b = (edge_point_p[ i-1 ]).col; /*Do we still need b since we know the center point?*/ 
    }
    else
    {
        b = ( (sum_x_square*sum_y) - (sum_x*sum_xy) ) / denominator;
        line_p->orientation = atan(( (num_points*sum_xy) - (sum_x*sum_y) ) / denominator);        
    }

    /*Compute the error of line least square fitting*/ 
    for( i = 0; i < num_points; i++)
    {
         row = edge_point_p[ i ].row;
         col = edge_point_p[ i ].col;
         appr = col * tan(line_p->orientation) + b;
         error += fabs(row - appr);
    }

    error /= num_points;
   /* printf("length: %f\n", line_p->length);
    printf("orientation: %f\n", line_p->orientation);
    printf("center_x: %f\n", line_p->centre_x);
    printf("center_y: %f\n", line_p->centre_x);
    */
    return error;
    
}

/** @brief Compute the average gradient of the line segment */
double compute_average_gradient(const Edge* edge_p)
{
    double avg_gradient = 0.0;
    uint32_t num_points = edge_p->num_points;
    Edge_point* edge_point_p = edge_p->points;
    uint32_t i;
    for(i = 0; i < num_points; i++)
    {
        avg_gradient += edge_point_p[ i ].mag;
    }
    return ( avg_gradient / ((double) num_points)); 
}


/** @brief Compute the starting and ending point of the line segment
 *  This is moved from the Line_segment class to here
 */
void compute_extrema
(
    double*                 start_x,
    double*                 start_y,
    double*                 end_x,
    double*                 end_y,
    const Line_segment_s*   ls
)
{
    double angle = 0;
    double base, half_base, m, half_height;
    
    if(ls->orientation < M_PI_2 )
    {
            angle = M_PI - ls->orientation;
    }
    else if( ls->orientation <= M_PI )
    {
            angle = ls->orientation;
    }
    else if( ls->orientation <= (M_PI*1.5) )
    {
        angle = ls->orientation - M_PI;
    }
    else
    {
            angle =  M_2_TIMES_PI - ls->orientation;

    }
    /*angle = ls->orientation;*/
    base = fabs(ls->length * cos(angle));
    half_base = base/2.0;
    if( (ls->centre_x - half_base) > 0)
    {
            *start_x = ls->centre_x - half_base;
    }
    else
    {
            *start_x = 0;
    }
    
    if(*start_x < 0)
    {
            *start_x = 0;
    }
    *end_x =  (int32_t)(ls->centre_x + half_base);
  

    m = tan(ls->orientation);
    half_height = fabs((ls->length*sin(angle)))/2.0; 
    if(m > 0.0)
    {
            *start_y =ls->centre_y - half_height;
            *end_y = ls->centre_y + half_height;
    }
    else
    {
            *start_y = ls->centre_y + half_height;
            *end_y = ls->centre_y - half_height;
    }
   

}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

#ifdef __cplusplus
}
#endif
