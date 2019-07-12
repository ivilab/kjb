
/* $Id $ */

/**
 * @file
 *
 * @author Luca Del Pero
 *
 * @brief Declarations for Line segment.
 *
 * Copyright (c) 2003-2008 by members of University of Arizona Computer Vision
 * group. For use outside the University of Arizona Computer Vision group
 * please contact Kobus Barnard.
 */

#ifndef LINE_SEGMENT_H
#define LINE_SEGMENT_H


#include "i/i_float.h"
#include "edge/edge_base.h"

#ifdef __cplusplus
extern "C" {
#ifdef COMPILING_CPLUSPLUS_SOURCE
namespace kjb_c {
#endif
#endif


/**
 * @brief Line segment in an image.
 *
 */
typedef struct Line_segment_s
{
    /**
     * @brief X coordinate of the centre of this line segment
     */
    double centre_x;

    /**
     *  @brief Y coordinate of the centre of this line segment
     */
    double centre_y;

    /**
     * @brief Orientation of this line segment
     */
    double orientation;
    
    /**
     * @brief length of the line segment 
     */
    double length;

    /**
     * @brief Magnitude of the gradient averaged over the edge points
     */
    double strength;
 
}
Line_segment_s;

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
);


/** @brief Compute the average gradient of the line segment */
double compute_average_gradient(const Edge* edge_p);

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
);
#ifdef __cplusplus
#ifdef COMPILING_CPLUSPLUS_SOURCE
}
#endif
}
#endif

#endif

