
/* $Id: l_4D_arr.h 4727 2009-11-16 20:53:54Z kobus $ */

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

#ifndef L_4D_ARRAY_INCLUDED
#define L_4D_ARRAY_INCLUDED


#include "l/l_def.h"
#include "l/l_sys_mal.h"

#ifdef __cplusplus
extern "C" {
#ifdef COMPILING_CPLUSPLUS_SOURCE
namespace kjb_c {
#endif
#endif


double**** allocate_4D_double_array
(
    int num_blocks,
    int num_planes,
    int num_rows,
    int num_cols
);

float**** allocate_4D_float_array
(
    int num_blocks,
    int num_planes,
    int num_rows,
    int num_cols
);

void***** allocate_4D_ptr_array
(
    int num_blocks,
    int num_planes,
    int num_rows,
    int num_cols
);

void free_4D_ptr_array(void***** array);
void free_4D_double_array(double**** array);
void free_4D_float_array(float**** array);


#ifdef __cplusplus
#ifdef COMPILING_CPLUSPLUS_SOURCE
}
#endif
}
#endif

#endif

