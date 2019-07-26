
/* $Id: i_ras.h 4727 2009-11-16 20:53:54Z kobus $ */

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

#ifndef I_RAS_INCLUDED
#define I_RAS_INCLUDED


#include "l/l_def.h"

#ifdef __cplusplus
extern "C" {
#ifdef COMPILING_CPLUSPLUS_SOURCE
namespace kjb_c {
#endif
#endif


#define RMT_NONE       0
#define RMT_EQUAL_RGB  1
#define RMT_RAW        2

#define RT_STANDARD    1
#define RT_ENCODED     2
#define RT_FORMAT_RGB  3

#define RASTER_MAGIC_NUM 0x59a66a95


typedef struct Sun_header
{
    kjb_int32 magic;
    kjb_int32 width;
    kjb_int32 height;
    kjb_int32 depth;
    kjb_int32 length;
    kjb_int32 type;
    kjb_int32 maptype;
    kjb_int32 maplength;
}
Sun_header;


#ifdef __cplusplus
#ifdef COMPILING_CPLUSPLUS_SOURCE
}
#endif
}
#endif

#endif


