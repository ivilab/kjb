/* $Id$ */
/* {{{=========================================================================== *
   |
   |  Copyright (c) 1994-2011 by Kobus Barnard (author)
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

#ifndef KJB_CUDA_RUNTIME_H
#define KJB_CUDA_RUNTIME_H

#ifdef KJB_HAVE_CUDART
#include <cuda_runtime.h>

#define CUDA_ETX(x) \
{ \
    cudaError_t err = (x); \
    if(err)  \
    { \
        std::cerr << "cuda_error: " << err << std::endl; \
        KJB_THROW_2(kjb::Exception, cudaGetErrorString(err)); \
    } \
}

#define CUDA_EPETE(x) \
    cudaError_t err = (x); \
    if(err)  \
    { \
        std::cerr << "cuda_error: " << err << std::endl; \
        exit(err); \
    }

#endif /* KJB_HAVE_CUDART */

#endif
