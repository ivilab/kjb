
/* $Id: i_lib.h 4727 2009-11-16 20:53:54Z kobus $ */

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

#ifndef I_LIB_INCLUDED
#define I_LIB_INCLUDED


#include "l/l_def.h"
#include "l/l_word_list.h"

#ifdef __cplusplus
extern "C" {
#ifdef COMPILING_CPLUSPLUS_SOURCE
namespace kjb_c {
#endif
#endif


int get_image_file_base_path
(
    const char* file_path,
    char*       base_path,
    size_t      base_path_size,
    char*       suffix,
    size_t      suffix_size
);

int get_image_file_base_name
(
    const char* file_name,
    char*       dir_str,
    size_t      dir_str_size,
    char*       base_name,
    size_t      base_name_size,
    char*       suffix,
    size_t      suffix_size
);

int get_image_files
(
    Word_list** paths_ptr_ptr, 
    Word_list** base_names_ptr_ptr,
    const char* dir, 
    const char* suffix
);


#ifdef __cplusplus
#ifdef COMPILING_CPLUSPLUS_SOURCE
}
#endif
}
#endif

#endif

