
/* $Id: wrap_X11.c 16823 2014-05-17 02:14:09Z kobus $ */

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

#include "l/l_gen.h"           /* Only safe as first include in a ".c" file. */

#ifdef KJB_HAVE_X11

     /*
      * C2man cannot handle the X11 headers. We can ignore them as long as we
      * protect c2man from X11 types. 
     */
#    ifndef __C2MAN__
#        ifndef MAKE_DEPEND  /* Protect against makedpend also. */
#            include <X11/Xlib.h>
#        endif 
#    endif 
#endif

#include "wrap_X11/wrap_X11.h"  

#ifdef __cplusplus
extern "C" {
#endif

/* -------------------------------------------------------------------------- */

#ifdef NOT_IN_L_SYS_X11

/* =============================================================================
 *                             X11_is_ok
 *
 * Returns true if code was compiled with X11
 *
 * This routine returns true if code was compiled with X11.
 *
 * Returns:
 *    TRUE  if X11 seesm to work. 
 *    FALSE otherwise. 
 *
 * Index:
 *     X11 
 *
 * -----------------------------------------------------------------------------
*/

int X11_is_ok(void)
{
#ifdef KJB_HAVE_X11
    static int cached_result = NOT_SET;
    Display *display;



    if (cached_result == NOT_SET)
    {
        display = XOpenDisplay((char*)NULL);

        if (display == NULL)
        {
            cached_result = FALSE;
        }
        else
        {
            XCloseDisplay(display);
            cached_result = TRUE;
        }
    }

    return cached_result;
#else
    return FALSE;
#endif
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

int ring_X11_bell(void)
{
#ifdef KJB_HAVE_X11
    Display *display;


    display = XOpenDisplay((char*)NULL);

    if (display == NULL)
    {
        return ERROR;
    }
    else
    {
        XBell(display, 100);
        XCloseDisplay(display);
    }

    return NO_ERROR;
#else
    return ERROR;
#endif
}

#endif 

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

#ifdef __cplusplus
}
#endif

