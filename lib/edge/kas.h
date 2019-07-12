
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

#ifndef KAS_H
#define KAS_H


#include "edge/line_segment.h"

#ifdef __cplusplus
extern "C" {
#ifdef COMPILING_CPLUSPLUS_SOURCE
namespace kjb_c {
#endif
#endif


/**
 * @brief Chain of connected line segments in an image.
 *
 */
typedef struct Kas_s
{
    /**
     * @brief X coordinate of the centre of this kAS
     */
    double centre_x;

    /**
     *  @brief Y coordinate of the centre of this kAS
     */
    double centre_y;

    /**
     * @brief The number of line segments in this kAS
     */
    size_t k;
    
    /**
     * @brief Pointers to the line segments contained in this kAS
     */

    Line_segment_s ** segments;

    /**
     * @brief scale of the kAS
     */
    double scale;

    /**
     * @brief Magnitude of the gradient averaged over the segments
     */
    double strength;

 
}
Kas_s;

#ifdef __cplusplus
#ifdef COMPILING_CPLUSPLUS_SOURCE
}
#endif
}
#endif

#endif

