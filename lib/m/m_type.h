
/* $Id: m_type.h 20918 2016-10-31 22:08:27Z kobus $ */

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

#ifndef M_TYPE_INCLUDED
#define M_TYPE_INCLUDED

#include "l/l_def.h"

#ifdef __cplusplus
extern "C" {
#ifdef COMPILING_CPLUSPLUS_SOURCE
namespace kjb_c {
#endif
#endif


typedef enum Norm_method
{
    DONT_NORMALIZE = 0,
    NORMALIZE_BY_MAGNITUDE,
    NORMALIZE_BY_MEAN,
    NORMALIZE_BY_SUM,
    NORMALIZE_BY_MAX_ABS_VALUE,
    NORMALIZATION_METHOD_ERROR = ERROR
}
Norm_method;


#ifdef __cplusplus
#ifdef COMPILING_CPLUSPLUS_SOURCE
}
#endif
}
#endif

#endif

