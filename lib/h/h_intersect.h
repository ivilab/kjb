
/* $Id: h_intersect.h 4727 2009-11-16 20:53:54Z kobus $ */

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

#ifndef H_INTERSECT_INCLUDED
#define H_INTERSECT_INCLUDED


#include "h/h_gen.h"

#ifdef __cplusplus
extern "C" {
#ifdef COMPILING_CPLUSPLUS_SOURCE
namespace kjb_c {
#endif
#endif


int set_hull_intersect_options(const char* option, const char* value);

int intersect_positive_hulls
(
    Queue_element* hull_list_head, /* Address of start of hull list. */
    unsigned long  options,        /* Returned data structure flags. */
    Hull**         result_hp_ptr   /* Address of resulting hull. */
);

int intersect_hulls
(
    Queue_element* hull_list_head, /* Address of start of hull list. */
    unsigned long  options,        /* Returned data structure flags. */
    Hull**         result_hp_ptr   /* Address of resulting hull. */
);

int original_intersect_hulls
(
    Queue_element* hull_list_head, /* Address of start of hull list. */
    unsigned long  options,        /* Returned data structure flags. */
    Hull**         result_hp_ptr   /* Address of resulting hull. */
);

int approximation_intersect_hulls
(
    Queue_element* hull_list_head,
    unsigned long  options,
    Hull**         result_hp_ptr
);

int dual_space_intersect_hulls
(
    Queue_element* hull_list_head,
    unsigned long  options,
    Hull**         result_hp_ptr
);

int find_point_in_2D_hull_intersection
(
    Queue_element* hull_list_head,
    int            resolution,
    Vector**       result_ptr,
    Matrix**       normal_mpp,
    Vector**       constrant_vpp
);

int find_point_in_3D_hull_intersection
(
    Queue_element* hull_list_head,
    int            resolution,
    Vector**       result_ptr,
    Matrix**       normal_mpp,
    Vector**       constrant_vpp
);


#ifdef __cplusplus
#ifdef COMPILING_CPLUSPLUS_SOURCE
}
#endif
}
#endif

#endif

