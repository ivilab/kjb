
/* $Id: edge_detector.h 5725 2010-03-18 01:31:44Z ksimek $ */

/**
 * @file
 *
 * @author Joseph Schlecht
 *
 * @brief Declarations for the edge detector.
 *
 * Copyright (c) 2003-2008 by members of University of Arizona Computer Vision
 * group. For use outside the University of Arizona Computer Vision group
 * please contact Joseph Schlecht.
 */

#ifndef EDGE_DETECTOR_H
#define EDGE_DETECTOR_H


#include "i/i_float.h"

#ifdef __cplusplus
extern "C" {
#ifdef COMPILING_CPLUSPLUS_SOURCE
namespace kjb_c {
#endif
#endif


/**
 * @brief Edge point in an image.
 *
 * Can be linked together in a list.
 */
typedef struct Edge_point_DEPRECATED
{
    /**
     * @brief Image column the point lies in.
     *
     * Location 0,0 is the upper left-hand corner of the image.
     */
    float x;

    /**
     * @brief Image row the point lies in.
     *
     * Location 0,0 is the upper left-hand corner of the image.
     */
    float y;

    /**
     * @brief Value of the red channel at the edge point.
     */
    float r;

    /**
     * @brief Value of the green channel at the edge point.
     */
    float g;

    /**
     * @brief Value of the blue channel at the edge point.
     */
    float b;

    /**
     * @brief Rate of change in brightness in the x-direction.
     */
    float dx;

    /**
     * @brief Rate of change in brightness in the y-direction.
     */
    float dy;

    /**
     * @brief Magnitude of the gradient at the edge point.
     */
    float gradient_mag;

    /**
     * @brief Boolean used for the edge detection algorithm.
     */
    int marked;

    /**
     * @brief Link to another edge point.
     *
     * Used to create a linked list of edge points. If the edge point is the
     * end of the list, the field is set to NULL.
     */
    struct Edge_point_DEPRECATED* next;
}
Edge_point_DEPRECATED;

/**
 * @brief Detects the edge points in an image.
 */
void detect_edge_points_DEPRECATED
(
    Edge_point_DEPRECATED** points_out,
    KJB_image** img_out,
    const KJB_image* img_in,
    double sigma,
    double begin_threshold,
    double end_threshold
);

/**
 * @brief Frees an Edge_point linked list.
 */
void free_edge_points_DEPRECATED( Edge_point_DEPRECATED* points ) ;

/**
 * @brief Returns the Euclidean distance between two edge points.
 */
double point_distance_DEPRECATED( const Edge_point_DEPRECATED* p1, const Edge_point_DEPRECATED* p2 ) ;


/**
 * @brief Eliminates some of the edge points by samping from them at the
 * specified radius.
 */
void sample_edge_points_DEPRECATED( Edge_point_DEPRECATED* points, double radius ) ;

/**
 * @brief Color the edge points in an image.
 */
void color_edge_points_DEPRECATED
(
    KJB_image* img,
    Edge_point_DEPRECATED* points,
    float r,
    float g,
    float b
) ;


#ifdef __cplusplus
#ifdef COMPILING_CPLUSPLUS_SOURCE
}
#endif
}
#endif

#endif

