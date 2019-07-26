
/* $Id: i_median_filter.h 4723 2009-11-16 18:57:09Z kobus $ */

/* =========================================================================== *
|
|  Copyright (c) 1994-2009 by Kobus Barnard.
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
#ifndef I_MEDIAN_FILTER_INCLUDED
#define I_MEDIAN_FILTER_INCLUDED

#include "i/i_float.h"
#include "i/i_vector.h"
#include "l/l_def.h"
#include "l/l_int_vector.h"

#ifdef __cplusplus
extern "C" {
#ifdef COMPILING_CPLUSPLUS_SOURCE
namespace kjb_c {
#endif
#endif

int  mode_filter
(
   KJB_image_vector *ivp,
   KJB_image **output_image
);


#ifdef __cplusplus
#ifdef COMPILING_CPLUSPLUS_SOURCE
}
#endif
}
#endif

#endif
