/* $Id: gpu_cuda_debug.cpp 21596 2017-07-30 23:33:36Z kobus $ */
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

#include "gpu_cpp/gpu_cuda_debug.h"
#include "m_cpp/m_vector.h"

#ifdef KJB_HAVE_CUDA
kjb::Vector kjb_gpu_debug_to_vector_uint(CUdeviceptr ptr, int width, int height, int pitch)
{
   unsigned int* o_data = new unsigned int[width * height];


   CUDA_MEMCPY2D cpy_meta;
   cpy_meta.srcXInBytes = 0;
   cpy_meta.srcY = 0;
   cpy_meta.srcMemoryType = CU_MEMORYTYPE_DEVICE;
   cpy_meta.srcHost = 0;
   cpy_meta.srcDevice = ptr;
   cpy_meta.srcArray = 0;
   cpy_meta.srcPitch = pitch;
   cpy_meta.dstMemoryType = CU_MEMORYTYPE_HOST;
   cpy_meta.dstXInBytes = 0;
   cpy_meta.dstY = 0;
   cpy_meta.dstHost = o_data;
   cpy_meta.dstDevice = 0;
   cpy_meta.dstArray = 0;
   cpy_meta.dstPitch = 0;
   cpy_meta.WidthInBytes = width * sizeof(unsigned int);
   cpy_meta.Height = height;

   CU_ETX(cuMemcpy2D(&cpy_meta));

   kjb::Vector result( width * height, 123.0);

   for(int i = 0; i < width * height; i++)
   {
       result[i] = o_data[i];
   }

    delete[] o_data;

    return result;
}
#endif
