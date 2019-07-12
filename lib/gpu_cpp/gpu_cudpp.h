/* $Id: gpu_cudpp.h 10607 2011-09-29 19:50:31Z predoehl $ */
/* {{{=========================================================================== *
   |
   |  Copyright (c) 1994-2010 by Kobus Barnard (author)
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
   |  Author:  Kyle Simek
 * =========================================================================== }}}*/

// vim: tabstop=4 shiftwidth=4 foldmethod=marker
#ifndef KJB_CPP_CUDPP_H
#define KJB_CPP_CUDPP_H

#include <l_cpp/l_exception.h>

#ifdef KJB_HAVE_CUDPP
#include <cudpp.h>

// TODO Create a CUDPP error class and convert error codes into meaningful strings
#define CUDPP_ETX(a) \
{ \
    CUDPPResult err = a; \
    if(err) \
    { \
        KJB_THROW_3(Runtime_error, "CUDPP error: %d", (err)); \
    } \
}

#endif

#endif 
