/* $Id: gpu_opencl.h 17393 2014-08-23 20:19:14Z predoehl $ */
/* {{{=========================================================================== *
   |
   |  Copyright (c) 1994-2012 by Kobus Barnard (author)
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

#ifndef KJB_CPP_GPU_OPENCL_H
#define KJB_CPP_GPU_OPENCL_H

#include <l_cpp/l_exception.h>

#ifdef KJB_HAVE_OPENCL
    #ifdef MAC_OSX
        #include <OpenCL/opencl.h>
    #else
        #include <CL/cl.h>
    #endif

// "On error throw exception" -- in this case, "error" is ANY nonzero value.
#define CL_ETX(a) \
{ \
    cl_int __err = a; \
    if(__err) \
    { \
        throw ::kjb::gpu::Opencl_error(__err, __FILE__, __LINE__); \
    } \
}

#define CL_ETX_2(a, msg) \
{ \
    cl_int __err = a; \
    if(__err) \
    { \
        throw ::kjb::gpu::Opencl_error(__err, msg, __FILE__, __LINE__); \
    } \
}

// "On error throw exception" -- in this case, "error" is ANY nonzero value.
#define CL_EPETE(a) \
{ \
    cl_int __err = a; \
    if(__err) \
    { \
        std::cerr << "OpenCL error: " << kjb::gpu::get_opencl_error_string(__err) << std::endl; \
        abort(); \
    } \
}

namespace kjb
{
namespace gpu
{


const char* get_opencl_error_string(cl_int err);

class Opencl_error : public kjb::Runtime_error 
{
public:
    Opencl_error(cl_int code, const char* file, int line) :
        Runtime_error(get_opencl_error_string(code), file, line),
        code_(code)
    {}

    Opencl_error(cl_int code, const std::string& message, const char* file, int line) :
        Runtime_error(message, file, line),
        code_(code)
    {}

    Opencl_error(const std::string& message, const char* file, int line) :
        Runtime_error(message, file, line),
        code_(0)
    {}

    cl_int get_error_code() const
    {
        return code_;
    }

private:
    cl_int code_;
};

}
}
#endif /* KJB_HAVE_OPENCL */
#endif
