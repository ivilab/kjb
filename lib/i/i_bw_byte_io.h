
/* $Id: i_bw_byte_io.h 4727 2009-11-16 20:53:54Z kobus $ */

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


#ifndef I_BW_BYTE_IO_H_INCLUDED
#define I_BW_BYTE_IO_H_INCLUDED

  
#include "i/i_bw_byte.h"  
#include "i/i_ras.h"

#ifdef __cplusplus
extern "C" {
#ifdef COMPILING_CPLUSPLUS_SOURCE
namespace kjb_c {
#endif
#endif


  int write_bw_byte_image(Bw_byte_image *image, const char * file_name);
  int read_bw_byte_image(Bw_byte_image **bw_byte_image_pp, const char * file_name);
  
#ifdef __cplusplus
#ifdef COMPILING_CPLUSPLUS_SOURCE
}
#endif
}
#endif

#endif

