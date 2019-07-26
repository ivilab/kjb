
/* $Id: curv_curvature.h 16712 2014-04-23 19:50:40Z qtung $ */

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

#ifndef CURV_CURVATURE_INCLUDED
#define CURV_CURVATURE_INCLUDED


#include "curv/curv_gen.h"

#ifdef __cplusplus
extern "C" {
#ifdef COMPILING_CPLUSPLUS_SOURCE
namespace kjb_c {
#endif
#endif


double image_curvature(const KJB_image *image, 
                       Matrix** curvature_mpp,
                       int num_output_images, 
                       KJB_image** out_ip_list,
                       const char* file);

double image_curvature_tangent_weight(const KJB_image *image, 
                                      Matrix** curvature_mpp,
                                      Matrix** curvature_tangent_weight_vector_mpp,
                                      int num_output_images, 
                                      KJB_image** out_ip_list,
                                      const char* file);


#ifdef __cplusplus
#ifdef COMPILING_CPLUSPLUS_SOURCE
}
#endif
}
#endif

#endif

