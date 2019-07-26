
/* $Id: c_def.h 4727 2009-11-16 20:53:54Z kobus $ */

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

#ifndef C_DEF_INCLUDED 
#define C_DEF_INCLUDED 

#ifdef __cplusplus 
extern "C" {
#ifdef COMPILING_CPLUSPLUS_SOURCE
namespace kjb_c {
#endif
#endif 


#define NUM_SENSORS 3

#define RED_INDEX   0
#define GREEN_INDEX 1
#define BLUE_INDEX  2


#ifdef __cplusplus
#ifdef COMPILING_CPLUSPLUS_SOURCE
}
#endif
}
#endif 

#endif 

