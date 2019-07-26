
/* $Id: i3_draw_hull.h 4727 2009-11-16 20:53:54Z kobus $ */

/*
    Copyright (c) 1994-2008 by Kobus Barnard (author).

    Personal and educational use of this code is granted, provided
    that this header is kept intact, and that the authorship is not
    misrepresented. Commercial use is not permitted.
*/

#ifndef I3_DRAW_HULL_INCLUDED
#define I3_DRAW_HULL_INCLUDED


#include "m/m_incl.h"
#include "h/h_gen.h"
#include "i/i_type.h"
#include "i/i_float.h"

#ifdef __cplusplus
extern "C" {
#ifdef COMPILING_CPLUSPLUS_SOURCE
namespace kjb_c {
#endif
#endif


int image_draw_hull_interior
(
    KJB_image*  ip,
    const Hull* hp,
    int         r,
    int         g,
    int         b
);

int image_draw_hull_boundary
(
    KJB_image*  ip,
    const Hull* hp,
    int         r,
    int         g,
    int         b
);


#ifdef __cplusplus
#ifdef COMPILING_CPLUSPLUS_SOURCE
}
#endif
}
#endif

#endif


