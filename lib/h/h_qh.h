
/* $Id: h_qh.h 4727 2009-11-16 20:53:54Z kobus $ */

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

#ifndef H_QH_INCLUDED
#define H_QH_INCLUDED


#include "m/m_gen.h"

#ifdef __cplusplus
extern "C" {
#ifdef COMPILING_CPLUSPLUS_SOURCE
namespace kjb_c {
#endif
#endif


int set_qhull_options(const char* option, const char* value);

int qh_find_convex_hull
(
    Matrix*         points_mp,
    int*            num_vertices_ptr,
    int*            num_facets_ptr,
    Matrix**        vertex_mpp,
    Matrix**        normal_mpp,
    Vector**        b_value_vpp,
    Matrix_vector** facet_mp_list_ptr,
    FILE*           geom_view_geometry_fp
);


#ifdef __cplusplus
#ifdef COMPILING_CPLUSPLUS_SOURCE
}
#endif
}
#endif

#endif

