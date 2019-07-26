
/* $Id: wrap_svm_set.c 8780 2011-02-27 23:42:02Z predoehl $ */

/* =========================================================================== *
|                                                                              |
|  Copyright (c) 2005-2008 by members of University of Arizona Computer Vision|
|  group (the authors) including                                               |
|        Ranjini Swaminathan
|        Kobus Barnard.                                                        |
|                                                                              |
|  (Copyright only applies to the wrapping code, not the wrapped code).        |
|                                                                              |
|  Personal and educational use of this code is granted, provided that this    |
|  header is kept intact, and that the authorship is not misrepresented, that  |
|  its use is acknowledged in publications, and relevant papers are cited.      |
|                                                                              |
|  For other use contact the author (kobus AT cs DOT arizona DOT edu).         |
|                                                                              |
|  Please note that the code in this file has not necessarily been adequately  |
|  tested. Naturally, there is no guarantee of performance, support, or fitness|
|  for any particular task. Nonetheless, I am interested in hearing about      |
|  problems that you encounter.                                                |
|                                                                              |
* =========================================================================== */




#include "wrap_svm/wrap_svm_set.h"

#include "wrap_svm/wrap_svm.h"

#ifdef __cplusplus
extern "C" {
#endif

/* -------------------------------------------------------------------------- */

/*
 * =============================================================================
 *                                  kjb_svm_set
 *
 *
 *
 *
 * -----------------------------------------------------------------------------
 */

int kjb_svm_set(const char* option, const char* value)
{
    int         temp_result;
    int         result = NOT_FOUND;
    int         (**set_fn_ptr) ( const char*, const char* );
    static int  (*set_fn[]) ( const char*, const char* ) =
                                    {
                                        set_svm_options,
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

