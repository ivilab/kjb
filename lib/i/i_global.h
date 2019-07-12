
/* $Id: i_global.h 4727 2009-11-16 20:53:54Z kobus $ */

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

#ifndef I_GLOBAL_INCLUDED
#define I_GLOBAL_INCLUDED


#include "i/i_gen.h"
#include "i/i_float.h"

#ifdef __cplusplus
extern "C" {
#ifdef COMPILING_CPLUSPLUS_SOURCE
namespace kjb_c {
#endif
#endif


#ifdef GLOBAL_DEF
#    undef GLOBAL_DEF
#endif

#ifdef GLOBAL_INIT
#    undef GLOBAL_INIT
#endif

#ifdef I_ALLOCATE_GLOBAL_STORAGE
#    define GLOBAL_DEF  EXPORT
#    define GLOBAL_INIT(x) x
#else
#    define GLOBAL_DEF  IMPORT
#    define GLOBAL_INIT(x)
#endif


/*
 * I am not sure why C++ does not want to have a global that is declared
 * constant.
*/
#ifdef KJB_CPLUSPLUS
#    define GLOBAL_CONST
#else
#    define GLOBAL_CONST const
#endif

GLOBAL_DEF GLOBAL_CONST Invalid_pixel valid_pixel_constant GLOBAL_INIT( = VALID_RGB );


/*
// For robotics. Can kill later.
*/
GLOBAL_DEF int export_strip_top    GLOBAL_INIT( = 0 ); /* Ignore lint */
GLOBAL_DEF int export_strip_bottom GLOBAL_INIT( = 0 ); /* Ignore lint */
GLOBAL_DEF int export_strip_left   GLOBAL_INIT( = 0 ); /* Ignore lint */
GLOBAL_DEF int export_strip_right  GLOBAL_INIT( = 0 ); /* Ignore lint */


#undef GLOBAL_DEF
#undef GLOBAL_INIT


#ifdef __cplusplus
#ifdef COMPILING_CPLUSPLUS_SOURCE
}
#endif
}
#endif

#endif


