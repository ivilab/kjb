
/* $Id: i_byte_io.h 18256 2014-11-20 17:55:02Z ksimek $ */

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

#ifndef I_BYTE_IO_INCLUDED
#define I_BYTE_IO_INCLUDED


#include "l/l_def.h"
#include "i/i_type.h"
#include "i/i_byte.h"

#ifdef __cplusplus
extern "C" {
#ifdef COMPILING_CPLUSPLUS_SOURCE
namespace kjb_c {
#endif
#endif


int read_byte_image(Byte_image** ip, char* file_name);
int write_byte_image(const Byte_image* ip, char* file_name);
int display_byte_image(const Byte_image* ip, char* title);


#ifdef __cplusplus
#ifdef COMPILING_CPLUSPLUS_SOURCE
}
#endif
}
#endif

#endif

