/* $Id: gpu_cuda.cpp 21596 2017-07-30 23:33:36Z kobus $ */
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

#include "l/l_sys_debug.h"   /* For ASSERT */
#include "gpu_cpp/gpu_cuda.h"

// the file below defines:
// static const char reduce_ptx[] = {...}
// reduce_ptx contains the bytecode for the reduce module.
#include "gpu_cpp/reduce_ptx.h"

#include "m_cpp/m_matrix.h"

#include <sstream>

#ifdef KJB_HAVE_CUDA
#include <cuda.h>
//#endif

using namespace std;

namespace kjb
{
namespace gpu
{


static boost::shared_array<char> read_file(const std::string& fname)
{
    using boost::shared_array;

    FILE *fp = fopen(fname.c_str(), "rb");
    int file_size;

    if(!fp)
    {
        KJB_THROW_3(IO_error, "Couldn't read file: %s", (fname.c_str()));
    }

    fseek(fp, 0, SEEK_END);
    file_size = ftell(fp);

    shared_array<char> file(new char[file_size+1]);

    fseek(fp, 0, SEEK_SET);
    fread(file.get(), sizeof(char), file_size, fp);
    fclose(fp);
    file[file_size] = '\0';

    return file;
}


const char* get_cuda_error_string(const CUresult& err)
{
    switch(err)
    {
    /**
     * The API call returned with no errors. In the case of query calls, this
     * can also mean that the operation being queried is complete (see
     * ::cuEventQuery() and ::cuStreamQuery()).
     */
        case CUDA_SUCCESS: return "SUCCESS";

    /**
     * This indicates that one or more of the parameters passed to the API call
     * is not within an acceptable range of values.
     */
        case CUDA_ERROR_INVALID_VALUE: return "ERROR_INVALID_VALUE";

    /**
     * The API call failed because it was unable to allocate enough memory to
     * perform the requested operation.
     */
        case CUDA_ERROR_OUT_OF_MEMORY: return "ERROR_OUT_OF_MEMORY";

    /**
     * This indicates that the CUDA driver has not been initialized with
     * ::cuInit() or that initialization has failed.
     */
        case CUDA_ERROR_NOT_INITIALIZED: return "ERROR_NOT_INITIALIZED";

    /**
     * This indicates that the CUDA driver is in the process of shutting down.
     */
        case CUDA_ERROR_DEINITIALIZED: return "ERROR_DEINITIALIZED";


    /**
     * This indicates that no CUDA-capable devices were detected by the installed
     * CUDA driver.
     */
        case CUDA_ERROR_NO_DEVICE: return "ERROR_NO_DEVICE";

    /**
     * This indicates that the device ordinal supplied by the user does not
     * correspond to a valid CUDA device.
     */
        case CUDA_ERROR_INVALID_DEVICE: return "ERROR_INVALID_DEVICE";


    /**
     * This indicates that the device kernel image is invalid. This can also
     * indicate an invalid CUDA module.
     */
        case CUDA_ERROR_INVALID_IMAGE: return "ERROR_INVALID_IMAGE";

    /**
     * This most frequently indicates that there is no context bound to the
     * current thread. This can also be returned if the context passed to an
     * API call is not a valid handle (such as a context that has had
     * ::cuCtxDestroy() invoked on it). This can also be returned if a user
     * mixes different API versions (i.e. 3010 context with 3020 API calls).
     * See ::cuCtxGetApiVersion() for more details.
     */
        case CUDA_ERROR_INVALID_CONTEXT: return "ERROR_INVALID_CONTEXT";

    /**
     * This indicated that the context being supplied as a parameter to the
     * API call was already the active context.
     * \deprecated
     * This error return is deprecated as of CUDA 3.2. It is no longer an
     * error to attempt to push the active context via ::cuCtxPushCurrent().
     */
        case CUDA_ERROR_CONTEXT_ALREADY_CURRENT: return "ERROR_CONTEXT_ALREADY_CURRENT";

    /**
     * This indicates that a map or register operation has failed.
     */
        case CUDA_ERROR_MAP_FAILED: return "ERROR_MAP_FAILED";

    /**
     * This indicates that an unmap or unregister operation has failed.
     */
        case CUDA_ERROR_UNMAP_FAILED: return "ERROR_UNMAP_FAILED";

    /**
     * This indicates that the specified array is currently mapped and thus
     * cannot be destroyed.
     */
        case CUDA_ERROR_ARRAY_IS_MAPPED: return "ERROR_ARRAY_IS_MAPPED";

    /**
     * This indicates that the resource is already mapped.
     */
        case CUDA_ERROR_ALREADY_MAPPED: return "ERROR_ALREADY_MAPPED";

    /**
     * This indicates that there is no kernel image available that is suitable
     * for the device. This can occur when a user specifies code generation
     * options for a particular CUDA source file that do not include the
     * corresponding device configuration.
     */
        case CUDA_ERROR_NO_BINARY_FOR_GPU: return "ERROR_NO_BINARY_FOR_GPU";

    /**
     * This indicates that a resource has already been acquired.
     */
        case CUDA_ERROR_ALREADY_ACQUIRED: return "ERROR_ALREADY_ACQUIRED";

    /**
     * This indicates that a resource is not mapped.
     */
        case CUDA_ERROR_NOT_MAPPED: return "ERROR_NOT_MAPPED";

    /**
     * This indicates that a mapped resource is not available for access as an
     * array.
     */
        case CUDA_ERROR_NOT_MAPPED_AS_ARRAY: return "ERROR_NOT_MAPPED_AS_ARRAY";

    /**
     * This indicates that a mapped resource is not available for access as a
     * pointer.
     */
        case CUDA_ERROR_NOT_MAPPED_AS_POINTER: return "ERROR_NOT_MAPPED_AS_POINTER";

    /**
     * This indicates that an uncorrectable ECC error was detected during
     * execution.
     */
        case CUDA_ERROR_ECC_UNCORRECTABLE: return "ERROR_ECC_UNCORRECTABLE";

    /**
     * This indicates that the ::CUlimit passed to the API call is not
     * supported by the active device.
     */
        case CUDA_ERROR_UNSUPPORTED_LIMIT: return "ERROR_UNSUPPORTED_LIMIT";


    /**
     * This indicates that the device kernel source is invalid.
     */
        case CUDA_ERROR_INVALID_SOURCE: return "ERROR_INVALID_SOURCE";

    /**
     * This indicates that the file specified was not found.
     */
        case CUDA_ERROR_FILE_NOT_FOUND: return "ERROR_FILE_NOT_FOUND";

    /**
     * This indicates that a link to a shared object failed to resolve.
     */
        case CUDA_ERROR_SHARED_OBJECT_SYMBOL_NOT_FOUND: return "ERROR_SHARED_OBJECT_SYMBOL_NOT_FOUND";

    /**
     * This indicates that initialization of a shared object failed.
     */
        case CUDA_ERROR_SHARED_OBJECT_INIT_FAILED: return "ERROR_SHARED_OBJECT_INIT_FAILED";

    /**
     * This indicates that an OS call failed.
     */
#if CUDA_VERSION >= 3020
        case CUDA_ERROR_OPERATING_SYSTEM: return "ERROR_OPERATING_SYSTEM";
#endif


    /**
     * This indicates that a resource handle passed to the API call was not
     * valid. Resource handles are opaque types like ::CUstream and ::CUevent.
     */
        case CUDA_ERROR_INVALID_HANDLE: return "ERROR_INVALID_HANDLE";


    /**
     * This indicates that a named symbol was not found. Examples of symbols
     * are global/constant variable names, texture names, and surface names.
     */
        case CUDA_ERROR_NOT_FOUND: return "ERROR_NOT_FOUND";


    /**
     * This indicates that asynchronous operations issued previously have not
     * completed yet. This result is not actually an error, but must be indicated
     * differently than ::CUDA_SUCCESS (which indicates completion). Calls that
     * may return this value include ::cuEventQuery() and ::cuStreamQuery().
     */
        case CUDA_ERROR_NOT_READY: return "ERROR_NOT_READY";


    /**
     * An exception occurred on the device while executing a kernel. Common
     * causes include dereferencing an invalid device pointer and accessing
     * out of bounds shared memory. The context cannot be used, so it must
     * be destroyed (and a new one should be created). All existing device
     * memory allocations from this context are invalid and must be
     * reconstructed if the program is to continue using CUDA.
     */
        case CUDA_ERROR_LAUNCH_FAILED: return "ERROR_LAUNCH_FAILED";

    /**
     * This indicates that a launch did not occur because it did not have
     * appropriate resources. This error usually indicates that the user has
     * attempted to pass too many arguments to the device kernel, or the
     * kernel launch specifies too many threads for the kernel's register
     * count. Passing arguments of the wrong size (i.e. a 64-bit pointer
     * when a 32-bit int is expected) is equivalent to passing too many
     * arguments and can also result in this error.
     */
        case CUDA_ERROR_LAUNCH_OUT_OF_RESOURCES: return "ERROR_LAUNCH_OUT_OF_RESOURCES";

    /**
     * This indicates that the device kernel took too long to execute. This can
     * only occur if timeouts are enabled - see the device attribute
     * ::CU_DEVICE_ATTRIBUTE_KERNEL_EXEC_TIMEOUT for more information. The
     * context cannot be used (and must be destroyed similar to
     * ::CUDA_ERROR_LAUNCH_FAILED). All existing device memory allocations from
     * this context are invalid and must be reconstructed if the program is to
     * continue using CUDA.
     */
        case CUDA_ERROR_LAUNCH_TIMEOUT: return "ERROR_LAUNCH_TIMEOUT";

    /**
     * This error indicates a kernel launch that uses an incompatible texturing
     * mode.
     */
        case CUDA_ERROR_LAUNCH_INCOMPATIBLE_TEXTURING: return "ERROR_LAUNCH_INCOMPATIBLE_TEXTURING";


#if CUDA_VERSION < 3020
        // Attempted to retrieve 64-bit pointer via 32-bit API function
        case CUDA_ERROR_POINTER_IS_64BIT: return "ERROR_POINTER_IS_64BIT";
        // Attempted to retrieve 64-bit pointer via 32-bit API function
        case CUDA_ERROR_SIZE_IS_64BIT: return "ERROR_SIZE_IS_64BIT";
#endif
    /**
     * This indicates that an unknown internal error has occurred.
     */
        default:
        case CUDA_ERROR_UNKNOWN: 
            return "Unknown error";
    }
}


static float* to_float_array(const Matrix& m, bool flip_y)
{
    return ::kjb::gpu::to_pitch<Matrix, float>(m, m.get_num_cols() * sizeof(float), flip_y);
}

CUarray create_cuda_array(const Matrix& m, bool flip_y)
{
    boost::scoped_array<float> data(to_float_array(m, flip_y));

   int width = m.get_num_cols();
   int height = m.get_num_rows();

   CUarray result;

   CUDA_ARRAY_DESCRIPTOR array_meta;
   array_meta.Width = width;
   array_meta.Height = height;
   array_meta.Format = CU_AD_FORMAT_FLOAT;
   array_meta.NumChannels = 1;

   // I put this in for debugging, and may need it again someday soon...
//   size_t free, total;
//   CU_EPETE(cuMemGetInfo(&free, &total));
//   cout << "Available memory: " << free << "/" << total << endl;
   
   CU_EPETE(cuArrayCreate(&result, &array_meta));
//   CU_ETX(cuArrayCreate(&result, &array_meta));

   CUDA_MEMCPY2D cpy_meta;
   cpy_meta.srcMemoryType = CU_MEMORYTYPE_HOST;
   cpy_meta.srcXInBytes = 0;
   cpy_meta.srcY = 0;
   cpy_meta.srcHost = data.get();
   cpy_meta.srcDevice = 0;
   cpy_meta.srcArray = 0;
   cpy_meta.srcPitch = width * sizeof(float);
   cpy_meta.dstXInBytes = 0;
   cpy_meta.dstY = 0;
   cpy_meta.dstMemoryType = CU_MEMORYTYPE_ARRAY;
   cpy_meta.dstHost = 0;
   cpy_meta.dstDevice = 0;
   cpy_meta.dstArray = result;
   cpy_meta.dstPitch = 0;
   cpy_meta.WidthInBytes = width * sizeof(float);
   cpy_meta.Height = height;

   CU_ETX(cuMemcpy2D(&cpy_meta));

   // DEBUGGING TEST

   boost::scoped_array<float> o_data(new float[width * height]);

   cpy_meta.srcXInBytes = 0;
   cpy_meta.srcY = 0;
   cpy_meta.srcMemoryType = CU_MEMORYTYPE_ARRAY;
   cpy_meta.srcHost = 0;
   cpy_meta.srcDevice = 0;
   cpy_meta.srcArray = result;
   cpy_meta.srcPitch = 0;
   cpy_meta.dstMemoryType = CU_MEMORYTYPE_HOST;
   cpy_meta.dstXInBytes = 0;
   cpy_meta.dstY = 0;
   cpy_meta.dstHost = o_data.get();
   cpy_meta.dstDevice = 0;
   cpy_meta.dstArray = 0;
   cpy_meta.dstPitch = 0;
   cpy_meta.WidthInBytes = width * sizeof(float);
   cpy_meta.Height = height;

   CU_ETX(cuMemcpy2D(&cpy_meta));

    for(int row = 0; row < height; row++)
    for(int col = 0; col < width; col++)
    {
        int i = row * width + col;
        ASSERT(data[i] == o_data[i]);
    }

   return result;
}



int Cuda::jit_log_buffer_size_ = 1024;
bool Cuda::initialized_ = false;

int Cuda::get_num_devices()
{
    ensure_initialized();

    static int count = -1;
    if(count < 0)
        CU_ETX(cuDeviceGetCount(&count));

    return count;
}


Cuda_device Cuda::get_device(int i)
{
    ensure_initialized();

    CUdevice handle;
    CU_ETX(cuDeviceGet(&handle, i));
    return Cuda_device(handle);
}


CUmodule Cuda::load_module(const char* mod_code, int max_registers)
{
    using boost::scoped_array;
    using boost::shared_array;

    ensure_initialized();

    // You'll notice this looks mostly like C code, that's because it mostly is.
    // I've ported this from C, adding C++ idioms where appropriate, but mostly
    // trying to alter it as little as possible
    // --Kyle, Sept 27. 2010

    // decide how many options we will send to the JIT compiler
    int jitNumOptions = 0;

    if(jit_log_buffer_size_ > 0)
    {
        jitNumOptions++; // log size
        jitNumOptions++; // log buffer

    }

    if(max_registers > 0)
        jitNumOptions++; // max registers

    // we use smart pointers to guarantee no memory leaks in the event 
    // of a Cuda exception being thrown.
    scoped_array<CUjit_option> jitOptions(new CUjit_option[jitNumOptions]);
    scoped_array<void*> jitOptVals(new void*[jitNumOptions]);

    scoped_array<char> jitLogBuffer(new char[jit_log_buffer_size_]);


    // SET UP JIT OPTIONS
    int i = 0;

    if(jit_log_buffer_size_ > 0)
    {
        jitOptions[i] = CU_JIT_INFO_LOG_BUFFER_SIZE_BYTES;
        jitOptVals[i] = (void*)(size_t)jit_log_buffer_size_;

        i++;

        jitOptions[i] = CU_JIT_INFO_LOG_BUFFER;
        jitOptVals[i] = jitLogBuffer.get();

        i++;
    }

    if(max_registers >= 0)
    {
        jitOptions[i] = CU_JIT_MAX_REGISTERS;
        jitOptVals[i] = (void *)(size_t)max_registers;

        i++;
    }

    // COMPILE MODULE AND RETREIVE HANDLE
    
    CUmodule module_handle;
    CUresult error = cuModuleLoadDataEx(&module_handle, mod_code, jitNumOptions, jitOptions.get(), jitOptVals.get());

    if(error)
    {
        std::stringstream str;
        str << "Error loading cuda module.";

        // remember, this is leak-free because we used smart pointers
        throw Cuda_error(error, __FILE__, __LINE__);
    }

    if(jit_log_buffer_size_ > 0)
        printf("> PTXyy JIT log:\n%s\n", jitLogBuffer.get());

    return module_handle;

}

CUmodule Cuda::load_module_file(const std::string& mod_fname, int max_registers)
{
    boost::shared_array<char> file = read_file(mod_fname);
    return load_module(file.get(), max_registers);
}

Cuda_reduce_module::Cuda_reduce_module() :
    Base(reduce_ptx, 32),
    functions_()
{
    load_functions();
}

double Cuda_reduce_module::tex_reduce(CUarray da_in, int width, int height)
{
    return tex_function(da_in, width, height, tex_reduce_function);
}

double Cuda_reduce_module::tex_count(CUarray da_in, int width, int height)
{
    return tex_function(da_in, width, height, tex_count_function);
}

double Cuda_reduce_module::chamfer_reduce(CUdeviceptr d_in_1, CUdeviceptr d_in_2, int N, bool square)
{
    using boost::scoped_array;
    using kjb_c::kjb_debug_level;
    using kjb_c::add_error;

    CUcontext ctx;
    cuCtxAttach(&ctx, 0);

    int blocks, threads;

    blocks = 64;
    threads = 256;

    scoped_array<float> h_out(new float[blocks]);
    double result;
    CUdeviceptr d_out = NULL;

    CUresult err = CUDA_SUCCESS;

    // function
    CUfunction function;
    if(square)
        function = squared_chamfer_and_reduce_function;
    else
        function = chamfer_and_reduce_function;


    // TODO: make this allocation exception-safe
    EGC(err = cuMemAlloc(&d_out, sizeof(float) * blocks));


    /* Call Kernel */
    #define ALIGN_UP(offset, alignment) \
        (offset) = ((offset) + (alignment) - 1) & ~((alignment) - 1)
    {


    /* when there is only one warp per block, we need to allocate two warps */
    /* worth of shared memory so that we don't index shared memory out of bounds */
        // TODO: make warp size adjustible
        const int WARP_SIZE = 32;
        const int smemSize = (threads <= WARP_SIZE) ? 2 * threads * sizeof(float) : threads * sizeof(float);
        cuFuncSetSharedSize(function, smemSize);

        int offset = 0;

        void* ptr;
        ptr = (void*)(size_t) d_in_1;
        ALIGN_UP(offset, __alignof(ptr));
        EGC(err = cuParamSetv(function, offset, &ptr, sizeof(ptr)));
        offset += sizeof(ptr);

        ptr = (void*)(size_t) d_in_2;
        ALIGN_UP(offset, __alignof(ptr));
        EGC(err = cuParamSetv(function, offset, &ptr, sizeof(ptr)));
        offset += sizeof(ptr);

        ptr = (void*)(size_t) d_out;
        ALIGN_UP(offset, __alignof(ptr));
        EGC(err = cuParamSetv(function, offset, &ptr, sizeof(ptr)));
        offset += sizeof(ptr);

        ALIGN_UP(offset, __alignof(N));
        EGC(err = cuParamSeti(function, offset, N));
        offset += sizeof(N);

        EGC(err = cuParamSetSize(function, offset));

        EGC(err = cuFuncSetBlockShape(function, threads, 1, 1));
        EGC(err = cuLaunchGrid(function, blocks, 1));
    }
    #undef ALIGN_UP

    // TODO: iteratively call kernel until 1 item left
    /* copy result from device to host */

    EGC(err = cuMemcpyDtoH(h_out.get(), d_out, blocks * sizeof(float)));

    {
        // manually sum up the block results.

//        double total = 0;
        double gpu_total = 0;
        for(int i = 0; i < blocks; i++)
        {
/*            printf("\t%f", h_out[i]); */
            gpu_total += h_out[i];
        }

        /* begin debug */
/*        total = reduceCPU(h_in, N); */
    /*    for(i = 0; i < N; i++) */
    /*    { */
    /*    total += h_in[i]; */
    /*    } */
/*        printf("\nCPU total: %f\nGPU total: %f\n", total, gpu_total); */
        /* end debug */

        result = gpu_total;
    }

cleanup:
    // this C-style cleanup is necessary because allocated device memory is not exception safe.
    if(d_out)
        cuMemFree(d_out);

    err = cuCtxDetach(ctx);

    if(err != CUDA_SUCCESS)
    {
        throw Cuda_error(err, __FILE__, __LINE__);
    }

    return result;
}

double Cuda_reduce_module::chamfer_reduce(CUarray texture_1, CUarray texture_2, int width, int height)
{
    using kjb_c::kjb_debug_level;
    using kjb_c::add_error;

    double result;

    CUresult err = CUDA_SUCCESS;

    CUcontext ctx;
    cuCtxAttach(&ctx, 0);

        EGC(err = cuTexRefSetAddressMode(tex_ref_2, 0, CU_TR_ADDRESS_MODE_CLAMP));
        EGC(err = cuTexRefSetAddressMode(tex_ref_2, 1, CU_TR_ADDRESS_MODE_CLAMP));
        EGC(err = cuTexRefSetFilterMode(tex_ref_2, CU_TR_FILTER_MODE_POINT));
//        EGC(err = cuTexRefSetFlags(tex_ref_2, CU_TRSF_READ_AS_INTEGER));
        EGC(err = cuTexRefSetFormat(tex_ref_2, CU_AD_FORMAT_FLOAT, 1));

        EGC(err = cuTexRefSetArray(tex_ref_2, texture_2, CU_TRSA_OVERRIDE_FORMAT));

    result = tex_function(texture_1, width, height, chamfer_reduce_function);

cleanup:
    err = cuCtxDetach(ctx);
    if(err != CUDA_SUCCESS)
    {
        throw Cuda_error(err, __FILE__, __LINE__);
    }

    return result;
}

/**
 * Utility function for calling tex_reduce and tex_count kernels.  Kernel signatures are the same, so this code is identical for each, except the function handle to use.
 */
double Cuda_reduce_module::tex_function(CUarray da_in, int width, int height, CUfunction function)
{

    // TODO merge identical parts with reduce()
    using boost::scoped_array;
    using kjb_c::kjb_debug_level;
    using kjb_c::add_error;

    CUcontext ctx;
    cuCtxAttach(&ctx, 0);

    // TODO: try height here
    const int stride = width;
    const int N = width * height;

    int blocks, threads;

    // hard-coded for now
//    get_num_blocks_and_threads_(N, MAX_BLOCKS, MAX_THREADS, blocks, threads);
    blocks = 64;
    threads = 256;

    scoped_array<float> h_out(new float[blocks]);
    double result;
    CUdeviceptr d_out = NULL;

    CUresult err = CUDA_SUCCESS;


    // TODO: make this allocation exception-safe
    EGC(err = cuMemAlloc(&d_out, sizeof(float) * blocks));

    /* Call Kernel */
    #define ALIGN_UP(offset, alignment) \
        (offset) = ((offset) + (alignment) - 1) & ~((alignment) - 1)
    {


    /* when there is only one warp per block, we need to allocate two warps */
    /* worth of shared memory so that we don't index shared memory out of bounds */
        // TODO: make warp size adjustible
        const int WARP_SIZE = 32;
        const int smemSize = (threads <= WARP_SIZE) ? 2 * threads * sizeof(float) : threads * sizeof(float);
        cuFuncSetSharedSize(function, smemSize);

        int offset = 0;

        EGC(err = cuTexRefSetAddressMode(tex_ref_1, 0, CU_TR_ADDRESS_MODE_CLAMP));
        EGC(err = cuTexRefSetAddressMode(tex_ref_1, 1, CU_TR_ADDRESS_MODE_CLAMP));
        EGC(err = cuTexRefSetFilterMode(tex_ref_1, CU_TR_FILTER_MODE_POINT));
//        EGC(err = cuTexRefSetFlags(tex_ref_1, CU_TRSF_READ_AS_INTEGER));
        EGC(err = cuTexRefSetFormat(tex_ref_1, CU_AD_FORMAT_FLOAT, 1));

        EGC(err = cuTexRefSetArray(tex_ref_1, da_in, CU_TRSA_OVERRIDE_FORMAT));

        void* ptr;
        ptr = (void*)(size_t) d_out;
        ALIGN_UP(offset, __alignof(ptr));
        EGC(err = cuParamSetv(function, offset, &ptr, sizeof(ptr)));
        offset += sizeof(ptr);

        ALIGN_UP(offset, __alignof(N));
        EGC(err = cuParamSeti(function, offset, N));
        offset += sizeof(N);

        ALIGN_UP(offset, __alignof(stride));
        EGC(err = cuParamSeti(function, offset, stride));
        offset += sizeof(stride);

        EGC(err = cuParamSetSize(function, offset));

        EGC(err = cuFuncSetBlockShape(function, threads, 1, 1));
        EGC(err = cuLaunchGrid(function, blocks, 1));
    }
    #undef ALIGN_UP

    // TODO: iteratively call kernel until 1 item left
    /* copy result from device to host */

    EGC(err = cuMemcpyDtoH(h_out.get(), d_out, blocks * sizeof(float)));

    {
        // manually sum up the block results.

//        double total = 0;
        double gpu_total = 0;
        for(int i = 0; i < blocks; i++)
        {
/*            printf("\t%f", h_out[i]); */
            gpu_total += h_out[i];
        }

        /* begin debug */
/*        total = reduceCPU(h_in, N); */
    /*    for(i = 0; i < N; i++) */
    /*    { */
    /*    total += h_in[i]; */
    /*    } */
/*        printf("\nCPU total: %f\nGPU total: %f\n", total, gpu_total); */
        /* end debug */

        result = gpu_total;
    }

cleanup:
    // this C-style cleanup is necessary because allocated device memory is not exception safe.
    if(d_out)
        cuMemFree(d_out);

    err = cuCtxDetach(ctx);

    if(err != CUDA_SUCCESS)
    {
        throw Cuda_error(err, __FILE__, __LINE__);
    }

    return result;
}


int Cuda_reduce_module::get_function_index_(int type_index_, int threads, bool is_pow_2_)
{
    int thread_i;
    // find log_2(threads)
    switch(threads)
    {
        case   1: thread_i = 0; break; 
        case   2: thread_i = 1; break; 
        case   4: thread_i = 2; break; 
        case   8: thread_i = 3; break; 
        case  16: thread_i = 4; break; 
        case  32: thread_i = 5; break; 
        case  64: thread_i = 6; break; 
        case 128: thread_i = 7; break; 
        case 256: thread_i = 8; break; 
        case 512: thread_i = 9; break; 
        default:  KJB_THROW(Runtime_error);
    }

    int i = thread_i * (2 * NUM_TYPES) + 2 * type_index_ + (is_pow_2_ ? 1 : 0);
#ifdef TEST
    ASSERT(i < NUM_FUNCTIONS);
#endif
    return i;
}


void Cuda_reduce_module::load_functions()
{
//        int pow_of_2 = is_pow_2(N);
    // get retrieve all twenty reduction function:
    // For both power-of-two size memory and non-power-of-two sized
    // and for every thread count:  1, 2, 4, 8, ..., 512 
    
    
    // type_strings -- 
    //      These strings correspond to the function names in reduce.cu.
    //      The order of these MUST correspond to the order in type_index()
    static const char* type_strings[NUM_TYPES] = {"float", "uchar"};
    
    int i = 0;
    for(int size_i = 0, size = 1; size_i < NUM_SIZES; size_i++, size <<= 1)
    for(int type_i = 0; type_i < NUM_TYPES; type_i++)
    {
        {
        // "false" version
        std::stringstream func_name;

        func_name << "reduce_" << type_strings[type_i] << "_" << size << "_false";

        ASSERT(i == get_function_index_(type_i, size, false));
        ASSERT(i < NUM_FUNCTIONS);
        CU_ETX(cuModuleGetFunction(&functions_[i], get_handle_(), func_name.str().c_str()));
        }

        i++;

        {
        // "true" version
        std::stringstream func_name;

        func_name << "reduce_" << type_strings[type_i] << "_" << size << "_true";

        ASSERT(i == get_function_index_(type_i, size, true));
        ASSERT(i < NUM_FUNCTIONS);
        CU_ETX(cuModuleGetFunction(&functions_[i], get_handle_(), func_name.str().c_str()));
        }

        i++;
    }

    const char* tex_reduce_str = "tex_reduce_256_false";
    const char* tex_count_str= "tex_count_256_false";
    CU_ETX(cuModuleGetFunction(&tex_reduce_function, get_handle_(), tex_reduce_str));
    CU_ETX(cuModuleGetFunction(&tex_count_function, get_handle_(), tex_count_str));
    CU_ETX(cuModuleGetFunction(&chamfer_reduce_function, get_handle_(), "chamfer_reduce_256_false"));
    CU_ETX(cuModuleGetFunction(&chamfer_and_reduce_function, get_handle_(), "chamfer_and_reduce"));
    CU_ETX(cuModuleGetFunction(&squared_chamfer_and_reduce_function, get_handle_(), "squared_chamfer_and_reduce"));

    CU_ETX(cuModuleGetTexRef(&tex_ref_1, get_handle_(), "tex_ref_1"));
    CU_ETX(cuModuleGetTexRef(&tex_ref_2, get_handle_(), "tex_ref_2"));

}

template<>
int Cuda_reduce_module::type_index<float>() { return 0; }
template<>
int Cuda_reduce_module::type_index<unsigned char>() { return 1; }


} // namespace gpu
} // namespace kjb

#endif

