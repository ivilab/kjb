
/* $Id: l_gen.h 21593 2017-07-30 16:48:05Z kobus $ */

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

/* This file should only include other files which are protected for C++ use. */

#ifdef WAS_MAKEDEPEND /* Now obsolete. */
#    ifndef MAKE_DEPEND_L_GEN_H
#        ifndef MAKE_DEPEND_DIR
#            define L_GEN_INCLUDED
#        endif
#    endif
#endif


#ifndef L_GEN_INCLUDED
#define L_GEN_INCLUDED



/* The following might no longer be true, as we have worked towards making
 * everything include what it needs to compile standalone. 
*/
/* The order of these "includes" counts! (But should it?)*/


/* The first 3 are included by the 4th, but leave it here to remmember this.*/
#include "l/l_sys_sys.h"
#include "l/l_sys_std.h"
#include "l/l_sys_def.h"
#include "l/l_def.h"

#include "l/l_type.h"
#include "l/l_global.h"
#include "l/l_debug.h"
#include "l/l_sys_debug.h"
#include "l/l_sys_err.h"
#include "l/l_sys_io.h"
#include "l/l_sys_lib.h"
#include "l/l_sys_mal.h"
#include "l/l_sys_sig.h"  /* Already included by l_global.h. Add it here to make */
                          /* sure that we don't forget this dependency.  */
#include "l/l_sys_term.h"
#include "l/l_sys_time.h"
#include "l/l_sys_str.h"

#include "l/l_error.h"
#include "l/l_parse.h"
#include "l/l_string.h"
#include "l/l_verbose.h"
#include "l/l_init.h"



#endif

