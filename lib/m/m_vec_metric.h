
/* $Id: m_vec_metric.h 20840 2016-09-04 18:27:14Z kobus $ */

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

#ifndef M_VEC_METRIC_INCLUDED
#define M_VEC_METRIC_INCLUDED


#include "m/m_gen.h"

#ifdef __cplusplus
extern "C" {
#ifdef COMPILING_CPLUSPLUS_SOURCE
namespace kjb_c {
#endif
#endif


double max_abs_vector_difference
(
    const Vector* first_vp,
    const Vector* second_vp
);

int get_vector_angle_in_degrees
(
    const Vector* vp1,
    const Vector* vp2,
    double*       result_ptr
);

int get_vector_angle_in_radians
(
    const Vector* vp1,
    const Vector* vp2,
    double*       result_ptr
);

int get_dot_product
(
    const Vector* vp1,
    const Vector* vp2,
    double*       result_ptr
);

double vector_distance(const Vector* vp1, const Vector* vp2);

double vector_distance_sqrd(const Vector* vp1, const Vector* vp2);

int first_vector_is_less
(
    const Vector* vp1,
    const Vector* vp2,
    double        relative_tolerance,
    double        absolute_tolerance
);

int first_vector_is_less_2
(
    const Vector* vp1,
    const Vector* vp2,
    double        relative_tolerance,
    double        absolute_tolerance,
    int*          index_for_biggest_difference_ptr,
    double*       biggest_difference_ptr
);

double rms_absolute_error_between_vectors
(
    const Vector* vp1,
    const Vector* vp2
);

double normalized_rms_error_between_vectors
(
    const Vector* vp1,
    const Vector* vp2
);

double rms_relative_error_between_vectors
(
    const Vector* vp1,
    const Vector* vp2
);


#ifdef __cplusplus
#ifdef COMPILING_CPLUSPLUS_SOURCE
}
#endif
}
#endif

#endif

