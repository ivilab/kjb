
/* $Id: t2_segment.h 4727 2009-11-16 20:53:54Z kobus $ */


/*
    Copyright (c) 1994-2008 by Kobus Barnard (author).

    Personal and educational use of this code is granted, provided
    that this header is kept intact, and that the authorship is not
    misrepresented. Commercial use is not permitted.
*/

#ifndef T2_SEGMENT_INCLUDED
#define T2_SEGMENT_INCLUDED


#include "l/l_def.h"
#include "i/i_gen.h"

#ifdef __cplusplus
extern "C" {
#ifdef COMPILING_CPLUSPLUS_SOURCE
namespace kjb_c {
#endif
#endif


/* -------------------------------------------------------------------------- */

typedef struct Segment_t2
{
    int         segment_number;
    int         num_pixels;
    int         num_boundary_pixels;
    int         num_neighbours;
    double      R_ave;
    double      G_ave;
    double      B_ave;
    double      r_chrom_ave;
    double      g_chrom_ave;
    double      sum_RGB_ave;
    double      i_CM;
    double      j_CM;
    int         i_inside;
    int         j_inside;
    double      first_moment;
    double      second_moment;
    int         i_min;
    int         i_max;
    int         j_min;
    int         j_max;
    double      boundary_len;
    double      outside_boundary_len;
    Pixel_info* pixels;
    Pixel_info* boundary_pixels;
    Matrix*     outside_boundary_mp;
    int*        neighbours;
    int         pixel_count;          /* Private, should be same as num_pixels. */
    int         boundary_pixel_count; /* Private */
}
Segment_t2;


/* =============================================================================
 *                             Segmentation_t2
 *
 * Type for segmentation results
 *
 * This type is a representation of a segmented image.
 *
 * Index: images
 *
 * -----------------------------------------------------------------------------
*/

/* Changes here must be reflected in the copy routine, etc. */
typedef struct Segmentation_t2
{
    int          num_rows;
    int          num_cols;
    int          num_segments;
    Segment_t2** segments;
    int          negative_num_segments;
    Segment_t2** negative_segments;
    int**        seg_map;
}
Segmentation_t2;


/* -------------------------------------------------------------------------- */

int t2_set_segmentation_options(const char* option, const char* value);

int t2_segment_image
(
    const KJB_image* ip,
    Segmentation_t2**   segment_info_ptr_ptr
);

void t2_free_segmentation(Segmentation_t2* segmentation_ptr);


#ifdef __cplusplus
#ifdef COMPILING_CPLUSPLUS_SOURCE
}
#endif
}
#endif

#endif


