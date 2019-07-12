
/* $Id: im_io.h 4727 2009-11-16 20:53:54Z kobus $ */

#ifndef IM_IO_INCLUDED
#define IM_IO_INCLUDED

#ifndef __C2MAN__

/*
    Copyright (c) 1994-2008 by Kobus Barnard (author).

    Personal and educational use of this code is granted, provided
    that this header is kept intact, and that the authorship is not
    misrepresented. Commercial use is not permitted.
*/


#include "im/im_gen.h"

#ifdef __cplusplus
extern "C" {
#ifdef COMPILING_CPLUSPLUS_SOURCE
namespace kjb_c {
#endif
#endif


#define MAX_MAGICK_LEN  100

typedef struct Image_file_info
{
    char magick[ MAX_MAGICK_LEN ];
}
Image_file_info;


typedef struct IM_displayed_image
{
    int pid;
    int read_des;
    char *temp_file_name;
}
IM_displayed_image;

/* ------------------------------------------------------------------------- */

void im_warning_handler(char* message, char* qualifier);

Byte_image* im_read_byte_image
(
    Byte_image*      ip,
    char*            file_name,
    Image_file_info* image_file_info_ptr
);

int im_write_byte_image
(
    Byte_image*      ip,
    char*            file_name,
    Image_file_info* image_file_info_ptr
);

IM_displayed_image* im_display_byte_image
(
    Byte_image* ip,
    char*       dummy_title,
    int         pipe_needed
);

int im_fork_close(IM_displayed_image* displayed_image_ptr);

int im_fork_close_all(void);


#ifdef __cplusplus
#ifdef COMPILING_CPLUSPLUS_SOURCE
}
#endif
}
#endif

#endif   /* #ifndef __C2MAN__ */

#endif   /* Include this file. */



