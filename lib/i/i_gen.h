
/* $Id: i_gen.h 7987 2010-12-15 09:32:33Z kobus $ */

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
#    ifndef MAKE_DEPEND_I_GEN_H
#        ifndef MAKE_DEPEND_DIR
#            define I_GEN_INCLUDED
#        endif
#    endif
#endif


#ifndef I_GEN_INCLUDED
#define I_GEN_INCLUDED


#include "l/l_incl.h"
#include "m/m_incl.h"
#include "c/c_gen.h"

#include "i/i_def.h"
#include "i/i_type.h"

#include "i/i_global.h"

#include "i/i_float.h"
#include "i/i_lib.h"
#include "i/i_error.h"
#include "i/i_arithmetic.h"
#include "i/i_ave.h"
#include "i/i_float_io.h"
#include "i/i_gamma.h"
#include "i/i_float_io.h"
#include "i/i_set.h"
#include "i/i_stat.h"


#endif

