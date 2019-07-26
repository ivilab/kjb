
/* $Id: camera_bw_byte_image_io.h 4727 2009-11-16 20:53:54Z kobus $ */

/* =========================================================================== *
|                                                                              |
|  Copyright (c) by members of University of Arizona Computer Vision Group     |
|  (the authors) including                                                     |
|        Kobus Barnard.                                                        |
|        Kate Spriggs                                                          |
|                                                                              |
|  For use outside the University of Arizona Computer Vision group please      |
|  contact Kobus Barnard.                                                      |
|                                                                              |
* =========================================================================== */

#ifndef I_CAMERA_BW_BYTE_IO_H_INCLUDED
#define I_CAMERA_BW_BYTE_IO_H_INCLUDED

  
#include "camera/camera_bw_byte_image.h"  
#include "i/i_bw_byte_io.h"

#ifdef __cplusplus
extern "C" {
#ifdef COMPILING_CPLUSPLUS_SOURCE
namespace kjb_c {
#endif
#endif


  int write_camera_bw_byte_image(Camera_bw_byte_image *image, const char * file_name);
  int read_camera_bw_byte_image(Camera_bw_byte_image **image, const char * file_name);
 
  
#ifdef __cplusplus
#ifdef COMPILING_CPLUSPLUS_SOURCE
}
#endif
}
#endif

#endif

