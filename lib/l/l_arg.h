
/* $Id: l_arg.h 4727 2009-11-16 20:53:54Z kobus $ */

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

#ifndef L_ARG_INCLUDED 
#define L_ARG_INCLUDED 

#ifdef __cplusplus
#endif 


#include "l/l_def.h"
#include "l/l_getopt.h"

#ifdef __cplusplus
extern "C" {
#ifdef COMPILING_CPLUSPLUS_SOURCE
namespace kjb_c {
#endif
#endif


/* ------------------------------------------------------------------------- */

#define LONG_OPTION_SET         0
#define NON_OPTION_ARGUMENT     1

/* -------------------------------------------------------------------------- */

void free_args(int, char***);
int print_args(FILE*, int, char**);
char* unparse_prog_args(int, char**);

int get_string_arg 
(
    int*        argc_ptr,
    char***     argv_ptr,
    const char* prompt_str,
    const char* default_str,
    char*       buff,
    size_t      buff_size 
);

int get_integer_arg
(
    int*        argc_ptr,
    char***     argv_ptr,
    const char* prompt_str,
    int         default_value,
    int*        int_ptr 
);

int get_boolean_arg
(
    int*        argc_ptr,
    char***     argv_ptr,
    const char* prompt_str,
    int         default_value,
    int*        boolean_ptr 
);

int kjb_getopts    
(
    int                  argc,
    char*                argv[],
    const char*          opt_str,
    const struct Option* long_options,
    char*                value_buff,
    size_t               value_buff_size 
);


#ifdef __cplusplus
#ifdef COMPILING_CPLUSPLUS_SOURCE
}
#endif
}
#endif 

#endif 

