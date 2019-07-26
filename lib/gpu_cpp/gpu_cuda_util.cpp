/* $Id: gpu_cuda_util.cpp 21596 2017-07-30 23:33:36Z kobus $ */
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

#include "gpu_cpp/gpu_cuda_util.h"
#include "gpu_cpp/util_ptx.h"

namespace kjb {
namespace gpu {
#ifdef KJB_HAVE_CUDA
Cuda_utility_module::Cuda_utility_module() :
    Base(util_ptx, 32),
    ow_ew_multiply_float_func(),
    ow_ew_multiply_uint_func(),
    ow_ew_multiply_int_func(),
    detect_changes_float_func(),
    detect_changes_uint_func(),
    detect_changes_int_func()
{
    load_functions_();
}
#endif

} // namespace gpu 
} // namespace kjb
