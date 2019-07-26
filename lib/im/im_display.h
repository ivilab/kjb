
/* $Id: im_display.h 8067 2010-12-23 21:11:58Z kobus $ */

#ifndef IM_DISPLAY_INCLUDED
#define IM_DISPLAY_INCLUDED

#ifndef __C2MAN__

#ifdef __cplusplus
extern "C" {
#ifdef COMPILING_CPLUSPLUS_SOURCE
namespace kjb_c {
#endif
#endif

#ifdef KJB_HAVE_X11
#include "magick/magick.h"
int im_display(Image* image, ImageInfo* image_info_ptr);
#endif 

int im_display_main(int argc, char** argv);

#ifdef __cplusplus
#ifdef COMPILING_CPLUSPLUS_SOURCE
}
#endif
}
#endif
/* END Kobus */


#endif   /* #ifndef __C2MAN__ */

#endif   /* Include this file. */


