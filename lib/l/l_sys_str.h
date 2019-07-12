
/* $Id: l_sys_str.h 4727 2009-11-16 20:53:54Z kobus $ */

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

#ifndef L_SYSTEM_STRING_INCLUDED
#define L_SYSTEM_STRING_INCLUDED


#include "l/l_def.h"

#ifdef __cplusplus
extern "C" {
#ifdef COMPILING_CPLUSPLUS_SOURCE
namespace kjb_c {
#endif
#endif


#ifdef STANDARD_VAR_ARGS
    /*PRINTFLIKE3*/
    long kjb_sprintf
    (
        char*       buff,
        size_t      max_len,
        const char* format_str,
        ...
    );
#else
    long kjb_sprintf(void);
#endif

long kjb_vsprintf
(
    char*       buff,
    size_t      max_len,
    const char* format_str,
    va_list     ap
);

#ifdef __cplusplus
#ifdef COMPILING_CPLUSPLUS_SOURCE
}
#endif
}
#endif

#endif

