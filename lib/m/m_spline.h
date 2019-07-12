
/* $Id: m_spline.h 4727 2009-11-16 20:53:54Z kobus $ */

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

#ifndef M_SPLINE_INCLUDED
#define M_SPLINE_INCLUDED


#include "m/m_gen.h"

#ifdef __cplusplus
extern "C" {
#ifdef COMPILING_CPLUSPLUS_SOURCE
namespace kjb_c {
#endif
#endif


Vector* cs_interpolate_vector
(
    Vector* ,
    double  ,
    double  ,
    Vector* ,
    int     ,
    double  ,
    double
);

Vector* interpolate_vector
(
    Vector* ,
    double  ,
    double  ,
    Vector* ,
    int     ,
    double  ,
    double
);

int uniform_x_cubic_spline
(
    int     ,
    double  ,
    double  ,
    double* ,
    int     ,
    double  ,
    double  ,
    double*
);

int cubic_spline(int, double*, double*, int, double*, double*);

int get_spline_parms
(
    int     ,
    double* ,
    double* ,
    double* ,
    double* ,
    double*
);

double eval_cubic(double, double, double, double, double);


#ifdef __cplusplus
#ifdef COMPILING_CPLUSPLUS_SOURCE
}
#endif
}
#endif

#endif

