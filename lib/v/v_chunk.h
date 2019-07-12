
/* $Id: v_chunk.h 8124 2010-12-27 06:32:18Z kobus $ */

/*
    Copyright (c) 1994-2008 by Kobus Barnard (author).

    Personal and educational use of this code is granted, provided
    that this header is kept intact, and that the authorship is not
    misrepresented. Commercial use is not permitted.
*/

#ifndef V_CHUNK_INCLUDED
#define V_CHUNK_INCLUDED

#include "i/i_gen.h"

#ifdef __cplusplus
extern "C" {
#ifdef COMPILING_CPLUSPLUS_SOURCE
namespace kjb_c {
#endif
#endif

/* -------------------------------------------------------------------------- */

/* =============================================================================
 *                             Image_chunk
 *
 * Type for chunking results
 *
 * This type was introduced to be part of the Image_chunk_info type. The KJB
 * library uses the term "chunk" to refer to an image piece surounded by boundry
 * pixels, but the type could be used for similar purposes.
 *
 * Index: images
 *
 * -----------------------------------------------------------------------------
*/

typedef struct Image_chunk
{
    int         chunk_number;
    int         num_pixels;
    int         num_boundary_pixels;
    float       i_CM;
    float       j_CM;
    Pixel_info* pixels;
    Pixel_info* boundary_pixels;
}
Image_chunk;


/* =============================================================================
 *                             Image_chunk_info
 *
 * Type for chunking results
 *
 * This type is a representation of an image which has been segmented into
 * pieces surround by boundary.
 *
 * Index: images
 *
 * -----------------------------------------------------------------------------
*/

typedef struct Image_chunk_info
{
    int          num_chunks;
    Image_chunk** chunks;
}
Image_chunk_info;


/* -------------------------------------------------------------------------- */

int set_chunk_options(const char* option, const char* value);

int segment_from_background
(
    const KJB_image*   in_ip,
    int                (*is_background_pixel_fn) (Pixel),
    Image_chunk_info** image_chunk_info_ptr_ptr
);

void free_image_chunk_info(Image_chunk_info* image_chunk_info_ptr);


#ifdef __cplusplus
#ifdef COMPILING_CPLUSPLUS_SOURCE
}
#endif
}
#endif

#endif


