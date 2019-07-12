
/* $Id: stat_test.h 4727 2009-11-16 20:53:54Z kobus $ */

/* =========================================================================== *
|                                                                              |
|  Copyright (c) by members of University of Arizona Computer Vision Group     |
|  (the authors) including                                                     |
|        Kobus Barnard.                                                        |
|                                                                              |
|  For use outside the University of Arizona Computer Vision group please      |
|  contact Kobus Barnard.                                                      |
|                                                                              |
* =========================================================================== */


#ifndef STAT_TEST_INCLUDED
#define STAT_TEST_INCLUDED

#ifdef __cplusplus
extern "C" {
#ifdef COMPILING_CPLUSPLUS_SOURCE
namespace kjb_c {
#endif
#endif


int welch_t_test_one_sided
(
    double  m1,
    double  m2,
    double  v1,
    double  v2,
    int     n1,
    int     n2,
    double* t_prime_ptr,
    int*    df_ptr,
    double* affect_size_ptr,
    double* p_value_ptr 
);


int welch_t_statistic
(
    double  u1,
    double  u2,
    double  v1,
    double  v2,
    int     n1,
    int     n2,
    double* t_prime_ptr,
    int*    df_ptr, 
    double* affect_size_ptr
);


#ifdef __cplusplus
#ifdef COMPILING_CPLUSPLUS_SOURCE
}
#endif
}
#endif

#endif    /*   #include this file            */


