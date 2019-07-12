
/* $Id: wrap_gsl_sf.h 11801 2012-02-27 05:17:45Z predoehl $ */

/* =========================================================================== *
|                                                                              |
|  Copyright (c) 2003-2008 by members of University of Arizona Computer Vision |
|  group (the authors) including                                               |
|        Andrew Predoehl                                                       |
|        Kobus Barnard                                                         |
|                                                                              |
|  (Copyright only applies to the wrapping code, not the wrapped code).        |
|                                                                              |
|  Personal and educational use of this code is granted, provided that this    |
|  header is kept intact, and that the authorship is not misrepresented, that  |
|  its use is acknowledged in publications, and relevant papers are cited.     |
|                                                                              |
|  For other use contact the author (kobus AT cs DOT arizona DOT edu).         |
|                                                                              |
|  Please note that the code in this file has not necessarily been adequately  |
|  tested. Naturally, there is no guarantee of performance, support, or fitness|
|  for any particular task. Nonetheless, I am interested in hearing about      |
|  problems that you encounter.                                                |
|                                                                              |
* =========================================================================== */


/*
 * This file is for wrappers for the gsl library. It needs to be included
 * specifically because it implies construction of that library when we do a
 * "make depend" which we only want to do when it is needed.
 */

#ifndef WRAP_GSL_SF_INCLUDED
#define WRAP_GSL_SF_INCLUDED

#ifdef __cplusplus
extern "C" {
#ifdef COMPILING_CPLUSPLUS_SOURCE
namespace kjb_c {
#endif
#endif


int kjb_erf(double* P_ptr, double x);

int kjb_bessel_I0( double* P_ptr, double x );

int kjb_scaled_bessel_I0( double* P_ptr, double x );

int kjb_scaled_bessel_I1( double* P_ptr, double x );

#ifdef __cplusplus
#ifdef COMPILING_CPLUSPLUS_SOURCE
}
#endif
}
#endif

#endif 

