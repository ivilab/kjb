
/* $Id: curv_lib.h 11295 2011-12-12 20:01:06Z predoehl $ */

/* =========================================================================== *
|                                                                              |
|  Copyright (c) 2003-2008 by members of University of Arizona Computer Vision     |
|  group (the authors) including                                               |
|        Kobus Barnard                                                         |
|        Amy Platt                                                             |
|                                                                              |
|  For use outside the University of Arizona Computer Vision group please      |
|  contact Kobus Barnard.                                                      |
|                                                                              |
* =========================================================================== */

#ifndef CURV_LIB_INCLUDED
#define CURV_LIB_INCLUDED


#include "m/m_gen.h"

#ifdef __cplusplus
extern "C" {
#ifdef COMPILING_CPLUSPLUS_SOURCE
namespace kjb_c {
#endif
#endif


int thin_pixels_not_needed_for_contiguity(Int_matrix* image_mp);

int count_neighbors
(
    const Int_matrix* image_mp,
    int               i,
    int               j
);

int fit_parametric_cubic_known_time
(
    const Vector* t_vp, 
    const Matrix* data_mp,
    const Vector* weight_vp,
    Matrix** new_c_mpp,
    double* error_ptr
);

int fit_parametric_cubic
(
    const Vector* t_vp,
    const Matrix* data_mp,
    const Vector* weight_vp,
    const Matrix* c_mp,
    Vector**      new_t_vpp,
    Matrix**      new_c_mpp,
    double*       error_ptr
);


#ifdef __cplusplus
#ifdef COMPILING_CPLUSPLUS_SOURCE
}
#endif
}
#endif

#endif

