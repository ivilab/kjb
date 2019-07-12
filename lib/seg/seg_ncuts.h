
/* $Id: seg_ncuts.h 4727 2009-11-16 20:53:54Z kobus $ */

/* =========================================================================== *
|                                                                              |
|  Copyright (c) 2003-2008 by members of University of Arizona Computer Vision     |
|  group (the authors) including                                               ||
|        Kobus Barnard.                                                        |
|        Prasad Gabbur.                                                        |
|  For use outside the University of Arizona Computer Vision group please      |
|  contact Kobus Barnard.                                                      |
|                                                                              |
* =========================================================================== */


#ifndef NCUT_MODULE_INCLUDED
#define NCUT_MODULE_INCLUDED


/*
// Exported interfaces go here.
*/

#include "m/m_incl.h"
#include "n/n_incl.h"

#ifdef __cplusplus
extern "C" {
#ifdef COMPILING_CPLUSPLUS_SOURCE
namespace kjb_c {
#endif
#endif

int ncut_dense_bipartition       
(
    Vector**      softpartition_vpp,
    const Matrix* weight_mp 
);

#ifdef __cplusplus
#ifdef COMPILING_CPLUSPLUS_SOURCE
}
#endif
}
#endif

#endif    /*   #include this file            */


