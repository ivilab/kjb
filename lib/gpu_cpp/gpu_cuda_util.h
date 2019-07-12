/* $Id: gpu_cuda_util.h 10606 2011-09-29 19:50:30Z predoehl $ */
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

#ifndef KJB_CUDA_UTIL_H
#define KJB_CUDA_UTIL_H

#include <string>
#include <vector>

#include <boost/scoped_array.hpp>
#include <boost/shared_array.hpp>

#include <m_cpp/m_vector.h>
#include <l_cpp/l_exception.h>

#include <gpu_cpp/gpu_cuda.h>

#undef major
#undef minor

namespace kjb
{
namespace gpu
{


/**
 * A set of miscelaneous cuda kernels perform simple tasks.
 *
 */
#ifdef KJB_HAVE_CUDA
class Cuda_utility_module : public Cuda_base_module
{
private:
typedef Cuda_base_module Base;

public:
    Cuda_utility_module();

    void load_functions_()
    {
#define KJB_LOAD_FUNCTION__(a) cuModuleGetFunction(&a##_func, get_handle_(), #a);
        CU_ETX(KJB_LOAD_FUNCTION__(ow_ew_multiply_float));
        CU_ETX(KJB_LOAD_FUNCTION__(ow_ew_multiply_uint));
        CU_ETX(KJB_LOAD_FUNCTION__(ow_ew_multiply_int));
        CU_ETX(KJB_LOAD_FUNCTION__(ow_ew_multiply_uint_float));
        CU_ETX(KJB_LOAD_FUNCTION__(detect_changes_float));
        CU_ETX(KJB_LOAD_FUNCTION__(detect_changes_uint));
        CU_ETX(KJB_LOAD_FUNCTION__(detect_changes_int));
#undef KJB_LOAD_FUNCTION__
    }



    CUfunction get_ow_ew_multiply_func(const unsigned int&, const float&)
    {
        return ow_ew_multiply_uint_float_func;
    }

    CUfunction get_ow_ew_multiply_func(const float&, const float&)
    {
        return ow_ew_multiply_float_func;
    }

    CUfunction get_ow_ew_multiply_func(const unsigned int&, const unsigned int&)
    {
        return ow_ew_multiply_uint_func;
    }

    CUfunction get_ow_ew_multiply_func(const int&, const int&)
    {
        return ow_ew_multiply_int_func;
    }



    CUfunction get_detect_changes_func(const float&)
    {
        return detect_changes_float_func;
    }

    CUfunction get_detect_changes_func(const unsigned int&)
    {
        return detect_changes_uint_func;
    }

    CUfunction get_detect_changes_func(const int&)
    {
        return detect_changes_int_func;
    }



    template <class T1, class T2>
    void ow_ew_multiply(CUdeviceptr v1, CUdeviceptr v2, unsigned int N)
    {
        using boost::scoped_array;
        using kjb_c::kjb_debug_level;
        using kjb_c::add_error;

        CUcontext ctx;
        cuCtxAttach(&ctx, 0);

        size_t blocks, threads;

        threads = 512;
        blocks = N / threads + (N % threads ? 1 : 0);

        assert(blocks < MAX_BLOCKS);

        // function
        CUfunction function = get_ow_ew_multiply_func(T1(), T2());

        /* Call Kernel */
        #define ALIGN_UP(offset, alignment) \
            (offset) = ((offset) + (alignment) - 1) & ~((alignment) - 1)
        {


            int offset = 0;
            void* ptr;

            ptr = (void*)(size_t) v1;
            ALIGN_UP(offset, __alignof(ptr));
            ETX(cuParamSetv(function, offset, &ptr, sizeof(ptr)));
            offset += sizeof(ptr);

            ptr = (void*)(size_t) v2;
            ALIGN_UP(offset, __alignof(ptr));
            ETX(cuParamSetv(function, offset, &ptr, sizeof(ptr)));
            offset += sizeof(ptr);

            ALIGN_UP(offset, __alignof(N));
            ETX(cuParamSeti(function, offset, N));
            offset += sizeof(N);

            ETX(cuParamSetSize(function, offset));

            ETX(cuFuncSetBlockShape(function, threads, 1, 1));
            ETX(cuLaunchGrid(function, blocks, 1));
        }
        #undef ALIGN_UP

        ETX(cuCtxDetach(ctx));
    }

    template <class T>
    void detect_changes(CUdeviceptr d_in, unsigned int N)
    {
        using boost::scoped_array;
        using kjb_c::kjb_debug_level;
        using kjb_c::add_error;

        CUcontext ctx;
        cuCtxAttach(&ctx, 0);

        size_t blocks, threads;

        threads = 512;
        blocks = N / threads + (N % threads ? 1 : 0);

        assert(blocks < MAX_BLOCKS);

        // function
        CUfunction function = get_detect_changes_func(T());

        /* Call Kernel */
        #define ALIGN_UP(offset, alignment) \
            (offset) = ((offset) + (alignment) - 1) & ~((alignment) - 1)
        {
            int offset = 0;
            void* ptr;

            ptr = (void*)(size_t) d_in;
            ALIGN_UP(offset, __alignof(ptr));
            ETX(cuParamSetv(function, offset, &ptr, sizeof(ptr)));
            offset += sizeof(ptr);

            ALIGN_UP(offset, __alignof(N));
            ETX(cuParamSeti(function, offset, N));
            offset += sizeof(N);

            ETX(cuParamSetSize(function, offset));

            ETX(cuFuncSetBlockShape(function, threads, 1, 1));
            ETX(cuLaunchGrid(function, blocks, 1));
        }
        #undef ALIGN_UP

        ETX(cuCtxDetach(ctx));
    }
private:


    CUfunction ow_ew_multiply_uint_float_func;

    CUfunction ow_ew_multiply_float_func;
    CUfunction ow_ew_multiply_uint_func;
    CUfunction ow_ew_multiply_int_func;

    CUfunction detect_changes_float_func;
    CUfunction detect_changes_uint_func;
    CUfunction detect_changes_int_func;


    // TODO: Make these adjustible, or at least check device capabilities for these.
    // They are taken from my Quadro NV 140M and provide reasonable lower bounds, though.
    static const size_t MAX_THREADS = 512;
    static const size_t MAX_BLOCKS = 65535;
};
#endif


} // namespace kjb
} // namespace gpu



#endif


// vim: tabstop=4 shiftwidth=4 foldmethod=marker

