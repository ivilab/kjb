
/* $Id: wrap_gandalf.c 4727 2009-11-16 20:53:54Z kobus $ */

/* =========================================================================== *
|                                                                              |
|  Copyright (c) 2003-2008 by members of University of Arizona Computer Vision|
|  group (the authors) including                                               |
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


#include "m/m_incl.h"

#ifdef __cplusplus
extern "C" {
#endif

/* -------------------------------------------------------------------------- */

/*
 * IMPORTANT: Each exported routine needs to have a version for both with and
 * withtout GANDALF.
*/

#ifdef KJB_HAVE_GANDALF

/* -----------------------------------------------------------------------------
|                                 GANDALF
|                                  ||
|                                 \||/
|                                  \/
*/

/*
 * Check for include file conflicts or problems.
*/

#include "gandalf/vision.h"
#include "gandalf/image.h"
#include "gandalf/common.h"
#include "gandalf/linalg.h"



#else

/* -----------------------------------------------------------------------------
|                               no GANDALF
|                                  ||
|                                 \||/
|                                  \/
*/

static void set_dont_have_gandalf_error(void)
{
    set_error("Operation failed because the program was built without the ");
    add_error("gandalf libraries readily available.");
    add_error("Appropriate installation, file manipulation and re-compiling ");
    add_error("is needed to fix this.)");
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */


/*
|                                  /\
|                                 /||\
|                                  ||
|                               no GANDALF
----------------------------------------------------------------------------- */

#endif

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

#ifdef __cplusplus
}
#endif


