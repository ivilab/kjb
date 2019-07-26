
/* $Id: axis_camera.h 4727 2009-11-16 20:53:54Z kobus $ */

#ifndef AXIS_CAMERA_H_INCLUDED
#define AXIS_CAMERA_H_INCLUDED


#include "camera/camera_bw_byte_image.h"

#ifdef __cplusplus
extern "C" {
#ifdef COMPILING_CPLUSPLUS_SOURCE
namespace kjb_c {
#endif
#endif


typedef struct Axis_camera
{
    char *hostname;
    char *username;
    char *password;
    int camera_number;
} Axis_camera;

int set_zoom ( Axis_camera * camera, int zoom ); /* 0 to 1000 */
int set_pan ( Axis_camera * camera, int pan ); /* 0 to 1000 */
int set_tilt ( Axis_camera * camera, int tilt ); /* 0 to 1000 */

int get_image_from_axis_camera ( Camera_bw_byte_image ** image, Axis_camera * camera );

#ifdef __cplusplus
#ifdef COMPILING_CPLUSPLUS_SOURCE
}
#endif
}
#endif

#endif
