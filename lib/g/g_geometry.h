
/* $Id: g_geometry.h 4727 2009-11-16 20:53:54Z kobus $ */

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

#ifndef G_GEOMETRY_INCLUDED
#define G_GEOMETRY_INCLUDED


#include "m/m_gen.h"

#ifdef __cplusplus
extern "C" {
#ifdef COMPILING_CPLUSPLUS_SOURCE
namespace kjb_c {
#endif
#endif


int get_polygon_CM_and_area
(
    const Matrix* points_mp,
    Vector**      cm_vpp,
    double*       area_ptr
);

int get_ordered_polygon_CM_and_area
(
    const Matrix* points_mp,
    Vector**      cm_vpp,
    double*       area_ptr
);

int is_point_in_segment
(
    const Vector* p1_vp,
    const Vector* p2_vp,
    const Vector* test_point_vp
);

int is_point_in_polygon
(
    const Matrix* points_mp,
    const Vector* test_point_vp
);

int order_planer_points
(
    Matrix**      ordered_points_mpp,
    const Matrix* points_mp
);

double z_component_of_cross_product(const Vector* vp1, const Vector* vp2);


#ifdef __cplusplus
#ifdef COMPILING_CPLUSPLUS_SOURCE
}
#endif
}
#endif

#endif

