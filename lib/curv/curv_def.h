
/* $Id: curv_def.h 4727 2009-11-16 20:53:54Z kobus $ */

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


#ifndef CURV_DEF_INCLUDED
#define CURV_DEF_INCLUDED

#ifdef __cplusplus
extern "C" {
#ifdef COMPILING_CPLUSPLUS_SOURCE
namespace kjb_c {
#endif
#endif


#define NEW_IMAGE_WAY

#define CURV_NUM_DIRECTIONS 8


#define IS_NEURON_PIXEL(ip,i,j) \
    ((i>= 0) && (i < ip->num_rows) && (j >= 0) && (j < ip->num_cols) && (ip->pixels[ i ][ j ].r > 128.0))

#define PIXEL_IS_ON(mp,i,j)  \
    ((i>= 0) && (i < mp->num_rows) && (j >= 0) && (j < mp->num_cols) && (mp->elements[ i ][ j ]))


#ifdef __cplusplus
#ifdef COMPILING_CPLUSPLUS_SOURCE
}
#endif
}
#endif

#endif    /*   #include this file            */

