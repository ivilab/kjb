
/* $Id: i_byte.h 22174 2018-07-01 21:49:18Z kobus $ */

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

#ifndef I_BYTE_INCLUDED
#define I_BYTE_INCLUDED


#include "i/i_gen.h"

#ifdef __cplusplus
extern "C" {
#ifdef COMPILING_CPLUSPLUS_SOURCE
namespace kjb_c {
#endif
#endif


/* -------------------------------------------------------------------------- */

typedef struct Byte_pixel
{
    unsigned char r;
    unsigned char g;
    unsigned char b;
}
Byte_pixel;


/* =============================================================================
 *                             Byte_image
 *
 * Type for images based on Byte_pixels
 *
 * This is a stub for documentation.  If you can help improve this description,
 * please do.
 *
 * The sizes (num_rows and num_cols) must both be nonnegative.
 * Conventionally, num_rows and num_cols are both positive;
 * however, it is also acceptable to have num_rows and num_cols both zero.
 * Anything else (such as a mix of positive and zero sizes) is regarded as a
 * bug in the calling program -- set_bug(3) is called and (if it returns) 
 * ERROR is returned.
 *
 * Index: images
 *
 * -----------------------------------------------------------------------------
*/
typedef struct Byte_image
{
    int          num_rows;
    int          num_cols;
    int          read_only;
    Byte_pixel** pixels;
}
Byte_image;

/* -------------------------------------------------------------------------- */

Byte_image* create_byte_image(int num_rows, int num_cols);
void free_byte_image(Byte_image* ip);

int get_target_byte_image
(
    Byte_image** out_ipp,
    int          num_rows,
    int          num_cols
);


#ifdef __cplusplus
#ifdef COMPILING_CPLUSPLUS_SOURCE
}
#endif
}
#endif

#endif

