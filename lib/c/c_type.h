
/* $Id: c_type.h 20918 2016-10-31 22:08:27Z kobus $ */

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

#ifndef C_TYPE_INCLUDED
#define C_TYPE_INCLUDED


#include "l/l_def.h"
#include "c/c_def.h"

#ifdef __cplusplus
extern "C" {
#ifdef COMPILING_CPLUSPLUS_SOURCE
namespace kjb_c {
#endif
#endif


typedef enum Projection_method
{
    /*
    // The first four are set based on backwards compatability. Hopefully all
    // the dependencies on the specific values have been purged, but by using
    // the same numeric values as before, things won't break regardless.
    */
    DIVIDE_BY_RED   = RED_INDEX,    /* 0 */
    DIVIDE_BY_GREEN = GREEN_INDEX,  /* 1 */
    DIVIDE_BY_BLUE  = BLUE_INDEX,   /* 2 */
    DIVIDE_BY_SUM   = 3,
    ONTO_RG_PLANE,
    ONTO_RB_PLANE,
    ONTO_GB_PLANE,
    ONTO_UNIT_SPHERE,
    INVALID_PROJECTION_METHOD = ERROR
}
Projection_method;


#ifdef __cplusplus
#ifdef COMPILING_CPLUSPLUS_SOURCE
}
#endif
}
#endif

#endif

