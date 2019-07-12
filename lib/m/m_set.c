
/* $Id: m_set.c 20654 2016-05-05 23:13:43Z kobus $ */

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

#include "m/m_gen.h"     /* Only safe as first include in a ".c" file. */

#include "m/m_set.h"


#ifdef __cplusplus
extern "C" {
#endif

/* -------------------------------------------------------------------------- */

/*
 * =============================================================================
 *                                  kjb_m_set
 *
 *
 *
 *
 * Index:
 *     set option handling 
 *
 * -----------------------------------------------------------------------------
 */

int kjb_m_set(const char* option, const char* value)
{
    int         temp_result;
    int         result = NOT_FOUND;
    int         (**set_fn_ptr)(const char*, const char*);
    /*
    // Note: Some option functions are not called here, as they force the load
    // of the extra libraries (or are only relavent to those that do. These
    // include:
    //     set_hull_options
    //     set_qhull_options
    //     set_cluster_options
    //     set_cluster_library_options
    //     set_hc_options
    //     set_hc_library_options
    */
    static int (*set_fn[])(const char*, const char*) =
                                    {
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

