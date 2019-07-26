
/* $Id: curv_type.h 22174 2018-07-01 21:49:18Z kobus $ */

/* =========================================================================== *
|                                                                              |
|  Copyright (c) 2003-2008 by members of University of Arizona Computer Vision     |
|  group (the authors) including                                               ||
|        Kobus Barnard.                                                        |
|                                                                              |
|  For use outside the University of Arizona Computer Vision group please      |
|  contact Kobus Barnard.                                                      |
|                                                                              |
* =========================================================================== */


/*
// Put defines here which are comon to more than one library module. Otherwise
// put it in the specific .h if it is exported, or the .c file if it is not.
*/

#ifndef CURV_TYPE_INCLUDED
#define CURV_TYPE_INCLUDED

#include "l/l_def.h"

#ifdef __cplusplus
extern "C" {
#ifdef COMPILING_CPLUSPLUS_SOURCE
namespace kjb_c {
#endif
#endif


typedef struct ALP_Point
{
  /* the indices of this point */
  int i; 
  int j;
  /* the t-value associated with it, for solving parameterized representation */
  float t;
} ALP_Point;


typedef enum Curve_res
{
    CURVE_NO_ERROR = NO_ERROR,
    CURVE_ERROR    = ERROR,
    WRONG_NUMBER_OF_NEIGHBORS = ERROR - 1,
    NOT_LONG_ENOUGH = ERROR - 2,
    DEGENERATE_SYSTEM = ERROR - 3
}
Curve_res;


#ifdef __cplusplus
#ifdef COMPILING_CPLUSPLUS_SOURCE
}
#endif
}
#endif

#endif    /*   #include this file            */

