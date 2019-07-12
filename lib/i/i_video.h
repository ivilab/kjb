
/* $Id: i_video.h 4727 2009-11-16 20:53:54Z kobus $ */

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

#ifndef I_VIDEO_INCLUDED
#define I_VIDEO_INCLUDED


#include "l/l_def.h"
#include "i/i_type.h"
#include "i/i_float.h"


#ifdef KJB_HAVE_VIDEO
#    ifndef USE_KJB
#        define USE_KJB
#    endif

#    include "mike/libMBvg-big1.h"
#endif

#ifdef __cplusplus
extern "C" {
#ifdef COMPILING_CPLUSPLUS_SOURCE
namespace kjb_c {
#endif
#endif


/* -------------------------------------------------------------------------- */


#define FULL_VIDEO_NUM_ROWS  478
#define FULL_VIDEO_NUM_COLS  638

#define STRIP_TOP      8
#define STRIP_BOTTOM   2
#define STRIP_LEFT     1
#define STRIP_RIGHT    0

#define STRIPPED_VIDEO_NUM_ROWS    \
                        (FULL_VIDEO_NUM_ROWS - STRIP_TOP - STRIP_BOTTOM)

#define STRIPPED_VIDEO_NUM_COLS   \
                        (FULL_VIDEO_NUM_COLS - STRIP_LEFT - STRIP_RIGHT)

/* -------------------------------------------------------------------------- */

int set_video_options(const char* option, const char* value);
int set_capture_frame_count(int new_value);
int get_capture_frame_count(void);
int start_video_grabber(void);
int stop_video_grabber(void);

int capture_image(KJB_image** ipp, Image_window* image_window_ptr);


#ifdef __cplusplus
#ifdef COMPILING_CPLUSPLUS_SOURCE
}
#endif
}
#endif

#endif


