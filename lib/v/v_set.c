
/* $Id: v_set.c 4727 2009-11-16 20:53:54Z kobus $ */

/*
   Copyright (c) 1994-2008 by Kobus Barnard (author).

   Personal and educational use of this code is granted, provided
   that this header is kept intact, and that the authorship is not
   misrepresented. Commercial use is not permitted.
*/


#include "v/v_gen.h"     /* Only safe as first include in a ".c" file. */

#include "v/v_set.h"
#include "v/v_ellipse.h"
#include "v/v_chunk.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ------------------------------------------------------------------------- */

/*
 * =============================================================================
 *                                  kjb_v_set
 *
 *
 *
 *
 * Index:
 *     set option handling 
 *
 * -----------------------------------------------------------------------------
 */

int kjb_v_set(const char* option, const char* value)
{
    int         temp_result;
    int         result = NOT_FOUND;
    int         (**set_fn_ptr)(const char*, const char*);
    static int  (*set_fn[])(const char*, const char*) =
                                    {
                                        set_chunk_options,
                                        set_image_ellipse_options,
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

