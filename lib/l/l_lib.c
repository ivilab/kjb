
/* $Id: l_lib.c 4727 2009-11-16 20:53:54Z kobus $ */

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

#include "l/l_gen.h"     /* Only safe as first include in a ".c" file. */

#include "l/l_string.h"
#include "l/l_lib.h"

#ifdef __cplusplus
extern "C" {
#endif

/* -------------------------------------------------------------------------- */

#ifndef MAX_PROGRAM_NAME_SIZE
#    define MAX_PROGRAM_NAME_SIZE  100
#endif

#ifndef SHARED_HOME_DIR
#    define SHARED_HOME_DIR "iis"
#endif

#ifndef PROGRAMMERS_HOME_DIR
#    define PROGRAMMERS_HOME_DIR "kobus"
#endif

/* -------------------------------------------------------------------------- */

static char fs_kjb_program_name[ MAX_PROGRAM_NAME_SIZE ] = "";

/* -------------------------------------------------------------------------- */

int set_program_name(char* program_name)
{


    BUFF_CPY(fs_kjb_program_name, program_name);

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

int get_program_name(char* buff, size_t buff_size)
{


    kjb_strncpy(buff, fs_kjb_program_name, buff_size);

    if (fs_kjb_program_name[ 0 ] == '\0')
    {
        return ERROR;
    }
    else
    {
        return NO_ERROR;
    }
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

void reverse_four_bytes(void* input, void* output)
{
    int i;

    for (i=0; i < 4; i++)
    {
        ((char*)output)[ i ] = ((char*)input)[ 3 - i ];
    }
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* Lindsay - Sept 28, 1999*/
void reverse_two_bytes(void* input, void* output)
{
    int i;

    for (i=0; i < 2; i++)
    {
        ((char*)output)[ i ] = ((char*)input)[ 1 - i ];
    }
}
/* End Lindsay - Sept 28, 1999*/

#ifdef __cplusplus
}
#endif

