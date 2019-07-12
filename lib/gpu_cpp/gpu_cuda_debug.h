/* $Id: gpu_cuda_debug.h 10605 2011-09-29 19:50:29Z predoehl $ */
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

#ifndef KJB_CPP_CUDA_DEBUG
#define KJB_CPP_CUDA_DEBUG

#include <gpu_cpp/gpu_cuda.h>

#ifdef KJB_HAVE_CUDA
kjb::Vector kjb_gpu_debug_to_vector_uint(CUdeviceptr ptr, int width, int height, int pitch);
#endif
#endif
