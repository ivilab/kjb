
/* $Id: bttv_camera.h 4727 2009-11-16 20:53:54Z kobus $ */

/* =========================================================================== *
|                                                                              |
|  Copyright (c) by members of University of Arizona Computer Vision Group     |
|  (the authors) including                                                     |
|        Kobus Barnard.                                                        |
|        Kate Spriggs                                                          |
|        Justin Cappos                                                         |
|                                                                              |
|  For use outside the University of Arizona Computer Vision group please      |
|  contact Kobus Barnard.                                                      |
|                                                                              |
* =========================================================================== */

#ifndef BTTV_CAMERA_H_INCLUDED
#define BTTV_CAMERA_H_INCLUDED


#include "i/i_bw_byte.h"
  /*for camera bw_byte_image*/
#include "camera/camera_bw_byte_image.h"
  /*for the camera position*/
#include "m/m_vector.h"
  /*for warn_pso and file io*/
#include "l/l_incl.h"

#ifdef LINUX

#ifdef __STRICT_ANSI__
#define FOO__STRICT_ANSI__
#undef __STRICT_ANSI__
#endif

#include <linux/videodev.h>

#ifdef FOO__STRICT_ANSI__
#define __STRICT_ANSI__
#undef FOO__STRICT_ANSI__
#endif

#endif 

#ifdef __cplusplus
extern "C" {
#ifdef COMPILING_CPLUSPLUS_SOURCE
namespace kjb_c {
#endif
#endif


/* This is the number of CAMERA_BUFFERS we can use */
#define CAMERA_BUFFERS 4
#define CAMERA_CHANNELS 10

/* Camera numbers start from 0 */
#define MAXIMUM_NUMBER_OF_CAMERAS 4

  /*Location of configuration file, the file name is: {CAM_ID}_initial*/
#define CONFIG_FILE_DIR "/net/v05/data/VR/cameras"
#define CONFIG_FILE_SUFFIX "_initial"

/* =============================================================================
 *                               Bttv_camera
 *
 * KJB Library bttv camera type
 *
 * This type is the bttv camera type for the KJB library. It stores the camera
 * number that is being queried in camera_number, and the resolution the camera
 * should be taking images with, in width and height. It also store whether the
 * camera is upside down and needs to be flipped.
 *
 * Index:
 *     cameras 
 *
 * Author: 
 *     Kate Spriggs
 *
 * -----------------------------------------------------------------------------
 */

/* A bunch of internal parameters for the camera */
typedef struct Bttv_camera_info
{
  int v_filedesc;
  void *v_map;
  char *frame_stream;
  int cur_buf;
  struct video_capability *v_capability;
  struct video_channel *v_channel[CAMERA_CHANNELS];
  struct video_mbuf *v_mbuf;
  struct video_picture *v_picture;
  struct video_mmap *v_mmap[CAMERA_BUFFERS];
}Bttv_camera_info;

/* Camera structure */
typedef struct Bttv_camera
{
  char camera_id[128];
  int width;
  int height;
  /*this is set to 0 if camera is normal way up, or to 1 if it is upside-down*/
  int camera_is_upside_down;

  char host_machine[128];
  char device[128];

  Vector *position_vp;
  /* int frame_rate; -- not used currently */
  Bttv_camera_info* camera_info;
} Bttv_camera;

  /* for bw_byte_image only*/
int init_bttv_camera(Bttv_camera **my_camera, int width, int height, char* camera_id, char* machine, char* device, int usd);
/*int init_bttv_camera(Bttv_camera **camera, int camera_number, int width, int height);*/
int init_bttv_camera_from_config_file(Bttv_camera **camera, int width, int height, const char* file_name);
/*int init_bttv_camera_from_config_file(Bttv_camera **camera, int camera_number, int width, int height, const char* file_name);*/
int get_image_from_bttv_camera(Bw_byte_image * image, Bttv_camera *camera);
int get_images_from_all_bttv_cameras(Bw_byte_image *image[], Bttv_camera *camera[], int num_cameras);
/*int get_images_from_all_bttv_cameras(Bw_byte_image * image[], Bttv_camera *camera[]);*/
void free_bttv_camera(Bttv_camera *camera);

/* for camera_bw_byte_image only - records the timestamp in the "image" struct*/
int get_image_from_bttv_camera_2(Camera_bw_byte_image *image, Bttv_camera *camera);
int get_images_from_all_bttv_cameras_2(Camera_bw_byte_image * image[], Bttv_camera *camera[], int num_cameras);


/* displaying the images from all cameras*/
void display_cameras_opengl(int argc, char **argv, int my_width, int my_height, Bttv_camera *all_cameras_in[], int num_cams);

#ifdef __cplusplus
#ifdef COMPILING_CPLUSPLUS_SOURCE
}
#endif
}
#endif

#endif



