
/* $Id: l_set.c 20919 2016-10-31 22:09:08Z kobus $ */

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
#include "l/l_verbose.h"
#include "l/l_io.h"
#include "l/l_sys_scan.h"
#include "l/l_sys_term.h"
#include "l/l_sys_io.h"
#include "l/l_sys_lib.h"
#include "l/l_sys_rand.h"
#include "l/l_debug.h"
#include "l/l_set.h"


#ifdef __cplusplus
extern "C" {
#endif

/* -------------------------------------------------------------------------- */

/*
 * =============================================================================
 *                                  kjb_l_set
 *
 *
 *
 *
 * Index:
 *     set option handling 
 *
 * -----------------------------------------------------------------------------
 */

int kjb_l_set(const char* option, const char* value)
{
    int         temp_result;
    int         result = NOT_FOUND;
    int         (**set_fn_ptr) (const char*, const char*);
    static int  (*set_fn[]) (const char*, const char*) =
                                    {
                                        set_heap_options,
                                        set_low_level_scan_options,
                                        set_low_level_io_options,
                                        set_term_io_options,
                                        set_lock_options,
                                        set_random_options,
                                        set_verbose_options,
                                        set_debug_options,
                                        set_io_options,
                                        NULL
                                    };

    set_fn_ptr = set_fn;

    while (*set_fn_ptr != NULL)
    {
        ERE(temp_result = (**set_fn_ptr)(option, value));

        if (temp_result == NO_ERROR) result = NO_ERROR;

        set_fn_ptr++;
    }

    return result;
}


/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

#ifdef __cplusplus
}
#endif

