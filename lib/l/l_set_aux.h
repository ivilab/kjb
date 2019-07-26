
/* $Id: l_set_aux.h 4727 2009-11-16 20:53:54Z kobus $ */

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

#ifndef L_SET_AUX_INCLUDED
#define L_SET_AUX_INCLUDED


#include "l/l_def.h"

#ifdef __cplusplus
extern "C" {
#ifdef COMPILING_CPLUSPLUS_SOURCE
namespace kjb_c {
#endif
#endif


typedef struct Method_option
{
    const char* long_name;
    const char* short_name;
    /*
    int (*fn)();
    */
    int (*fn)();
}
Method_option;


int process_option_string
(
    const char* arg,                                     /* String of options as {<opt>=<val>}   */
    int         (*option_fn) (const char*, const char*)
);

int call_set_fn
(
    int         num_set_fn,
    int         (*set_fn_list[])(const char*, const char*),
    const char* title,
    const char* option,
    const char* value
);

int parse_method_option
(
    Method_option* methods,
    int            num_methods,
    const char*    method_option_str,
    const char*    method_message_str,
    const char*    value,
    int*           method_ptr
);


#ifdef __cplusplus
#ifdef COMPILING_CPLUSPLUS_SOURCE
}
#endif
}
#endif

#endif


