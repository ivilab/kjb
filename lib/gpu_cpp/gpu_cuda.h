/* $Id: gpu_cuda.h 10604 2011-09-29 19:50:28Z predoehl $ */
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

#ifndef KJB_CUDA_H
#define KJB_CUDA_H

#include <string>
#include <vector>

#include <boost/scoped_array.hpp>
#include <boost/shared_array.hpp>

#include <m_cpp/m_vector.h>
#include <l_cpp/l_exception.h>

#ifdef KJB_HAVE_CUDA
#include <cuda.h>
#if CUDA_VERSION < 3020
typedef unsigned int Cuda_size_t;
#else
typedef size_t Cuda_size_t;
#endif
#endif

#undef major
#undef minor


namespace kjb
{
namespace gpu
{

#ifdef KJB_HAVE_CUDA
// "On error throw exception" -- in this case, "error" is ANY nonzero value.
#define CU_ETX(a) \
{ \
    CUresult err = a; \
    if(err) \
    { \
        throw ::kjb::gpu::Cuda_error(err, __FILE__, __LINE__); \
    } \
}

// "On error throw exception" -- in this case, "error" is ANY nonzero value.
#define CU_EPETE(a) \
{ \
    CUresult err = a; \
    if(err) \
    { \
        std::cerr << "Cuda error: " << get_cuda_error_string(err) << std::endl; \
        abort(); \
    } \
}
#endif


#ifdef KJB_HAVE_CUDA
const char* get_cuda_error_string(const CUresult& err);
#endif

#ifdef KJB_HAVE_CUDA
class Cuda_error : public kjb::Runtime_error 
{
public:
    Cuda_error(CUresult error_code, const char* file, int line) :
        Runtime_error(get_cuda_error_string(error_code), file, line),
        code_(error_code)
    {}

    Cuda_error(CUresult error_code, const std::string& msg, char* file, int line) :
        Runtime_error(msg, file, line),
        code_(error_code)
    {}

    Cuda_error(const std::string& msg, const char* file, int line) :
        Runtime_error(msg, file, line),
        code_(CUDA_ERROR_UNKNOWN)
    {}


    CUresult get_cuda_error()
    {
        return code_;
    }
private:
    CUresult code_;
};
#endif

/**
 * A simple struct to hold the Cuda compute capability.
 *
 * Eventually, we may add methods to this, like:
 *      bool supports_double();
 *
 * @author Kyle Simek
 */
#ifdef KJB_HAVE_CUDA
struct Cuda_compute_capability
{
    Cuda_compute_capability() :
        major(0),
        minor(0)
    {}

    Cuda_compute_capability(int major_, int minor_) :
        major(major_),
        minor(minor_)
    {}

    Cuda_compute_capability(const Cuda_compute_capability& other) :
        major(other.major),
        minor(other.minor)
    {}

    Cuda_compute_capability& operator=(const Cuda_compute_capability& other)
    {
        major = other.major;
        minor = other.minor;

        return *this;
    }

    bool operator==(const Cuda_compute_capability& other)
    {
        return major == other.major && minor == other.minor;
    }

    bool operator<(const Cuda_compute_capability& other)
    {
        if(major < other.major) return true;
        if(major == other.major && minor < other.minor) return true;
        return false;

    }


    int major;
    int minor;
};
#endif

/**
 * A cuda device.  
 *
 * Contains device properties, the Cuda compute capability version, plus methods to query available memory.
 *
 * @author Kyle Simek
 */
#ifdef KJB_HAVE_CUDA
class Cuda_device
{
public:
    friend class Cuda;
    Cuda_device(const Cuda_device& other) :
        handle_(other.handle_),
        capability_(other.capability_),
        name_(other.name_),
        properties_(other.properties_)
    {}

    Cuda_device& operator=(const Cuda_device& other)
    {
        handle_ = other.handle_;
        capability_ = other.capability_;
        name_ = other.name_;
        properties_ = other.properties_;

        return *this;
    }

    bool operator==(const Cuda_device& other)
    {
        return handle_ == other.handle_;
    }

    operator CUdevice() const
    {
        return handle_;
    }

    /**
     * Query cuda for attributes of this device.  
     *
     * @note For device properties, you should use the
     * spefific accessor methods for the property.  This is intended for more 
     * obscure attributes, or for attributes that may be added to cuda in the future.
     * */
    int get_attribute(CUdevice_attribute attrib) const
    {
        int result;
        CU_ETX(cuDeviceGetAttribute(&result, attrib, handle_));
        return result;
    }

    /**
     * returns the Cuda compute capability of this device
     */
    const Cuda_compute_capability& compute_capability() const
    {
        return capability_;
    }

    const std::string& name() const
    {
        return name_;
    }

    /**
     * The maximum number of threads per block.  
     *
     * @note If you are using
     * 2D or 3D block-shapes, you should use the version of this method
     * that receives the dimension as input, because different dimensions
     * may have lower maximums than others.
     */
    int max_threads() const
    {
        return properties_.maxThreadsPerBlock;
    }

    /**
     * Return maximum threads in dimension d of a block.  Some
     * devices might support more threads in the X dimension 
     * (for example) than the Y or Z dimension.
     *
     * @param d The block dimension. May be 0, 1, or 2
     */
    int max_threads(int d) const
    {
        assert(d >= 0 && d < 3);
        return properties_.maxThreadsDim[d];
    }

    /**
     * Return the maximum blocks in dimension d of a grid.
     * Some devices might support more blocks in the X dimension
     * (for example) than the Y or Z dimensions.
     *
     * @param d The grid dimension.  May be 0, 1, or 2.
     */
    int max_blocks(int d) const
    {
        assert(d >= 0 && d < 3);
        return properties_.maxGridSize[d];
    }

    /**
     * The total amount of shared memory available per block (in bytes)
     */
    int shared_memory_size() const
    {
        return properties_.sharedMemPerBlock;
    }

    /**
     * The total amount of constant memory available per block (in bytes)
     */
    int const_memory_size() const
    {
        return properties_.totalConstantMemory;
    }

    /**
     * Returns the number of threads per warp (i.e. the SIMD width)
     */
    int warp_size() const
    {
        return properties_.SIMDWidth;
    }

    /**
     * returns the maximum pitch allowed by the memory copy functions that
     * involve memory regions allocated through cuMemAllocPitch()
     */
    int memory_pitch() const
    {
        return properties_.memPitch;
    }

    /**
     * The total number of registers available per block
     */
    int register_count() const
    {
        return properties_.regsPerBlock;
    }

    /**
     * the Clock rate in kilohertz
     */
    int clock_rate() const
    {
        return properties_.clockRate;
    }

    /**
     * The alignment requirement; 
     * If texture_alignment returns N, texture base addresses that 
     * are aligned to N bytes do not need an offset applied
     * to texture fetches.
     */
    int texture_alignment() const
    {
        return properties_.textureAlign;
    }

    /**
     * Get available memory on the device
     */
    int available_memory() const
    {
#if CUDA_VERSION < 3020
        unsigned int bytes;
#else
        size_t bytes;
#endif

        CU_ETX(cuDeviceTotalMem(&bytes, handle_));
        return bytes;
    }



private:
    /// This should only be constructed by Cuda::get_device(i);
    Cuda_device(CUdevice handle) :
        handle_(handle),
        capability_(),
        name_(),
        properties_()
    {
        CU_ETX(cuDeviceComputeCapability(&capability_.major, &capability_.minor, handle_));

        const size_t BUF_SIZE = 256;
        char name_buff[BUF_SIZE];
        CU_ETX(cuDeviceGetName(name_buff, BUF_SIZE, handle_));
        name_ = std::string(name_buff);

        CU_ETX(cuDeviceGetProperties(&properties_, handle_));
    }

private:
    CUdevice handle_;
    Cuda_compute_capability capability_;
    std::string name_;
    CUdevprop properties_;

};
#endif


#ifdef KJB_HAVE_CUDA
class Cuda
{
public:

    /**
     * Return the number of cuda-enabled devices found in the system
     */
    static int get_num_devices();

    /**
     * Return an object describing the i-th cuda device on the system.
     *
     * @see get_num_devices
     */
    static Cuda_device get_device(int i);

    static bool is_initialized()
    {
        return initialized_;
    }

    static void ensure_initialized()
    {
        if(!is_initialized())
            init_(0);
    }

    /* read and jit-compile file 
     * @param mod_fname The filename of the module file.  Currently ptx is supported; adding bin support should be trivial if you need it.
     * @param max_registers  The maximum number of registers this module will use.  Defualt is 32; this seems to be what all the Nvidia examples use.*/
    static CUmodule load_module(const char* mod_fname, int max_registers = 32);
    static CUmodule load_module_file(const std::string& mod_fname, int max_registers = 32);

    /**
     * Default is 1024.
     * Set to zero to suppress output of JIT compiler.
     */
    static void set_jit_log_buffer_size(size_t size)
    {
        jit_log_buffer_size_ = size;
    }

private:

    static void init_(unsigned int flags)
    {
        if(initialized_)
        {
            KJB_THROW_2(Runtime_error, "Cuda already initialized, cannot reinitialize.");
        }

        initialized_ = true;
        cuInit(flags);
    }

    static int jit_log_buffer_size_;
    static bool initialized_;
};
#endif

// TODO: how to handle contexts?  C-style handles it pretty well currently.  what added value do we get?  automatic garbage collection; RAII semantics.  so just have a really simple context, whhich destroys itself and can return the CUcontext object...  Not copyable, not default constructible.
#ifdef KJB_HAVE_CUDA
class Cuda_context
{
public:
    Cuda_context(unsigned int flags, const Cuda_device& device) :
        context_(NULL)
    {
        Cuda::ensure_initialized();

        cuCtxCreate(&context_, flags, device);
    }

    Cuda_context(const Cuda_device& device) :
        context_(NULL)
    {
        Cuda::ensure_initialized();

        cuCtxCreate(&context_, 0, device);
    }

    /**
     * Get the raw cuda context object
     */
    CUcontext get()
    {
        return context_;
    }

    size_t get_free_memory()
    {
#if CUDA_VERSION < 3020
       unsigned int free, total;
#else
       size_t free, total;
#endif
       CU_EPETE(cuMemGetInfo(&free, &total));

       return free;
    }
        

    ~Cuda_context()
    {
        cuCtxDestroy(context_);
    }

    operator CUcontext() const
    {
        return context_;
    }

private:
    // no copying contexts
    Cuda_context(const Cuda_context& /*other*/) {}

    CUcontext context_;
};
#endif


/**
 * Abstract base class for loading modules and the exposing
 * kernels within.
 *
 * Follows RAII, i.e.  module is unloaded in CUDA on destruction.
 *
 * TODO: handle multiple GPU devices or cuda contexts.  or document to make sure all calls to this module happen with the same context active.
 *
 * TODO: make constructor private.  get_instance() checks the current context, and returns the module if it's laredy
 * been created, otherwise create it, register it to this context, and return it.
 *
 * @author Kyle simek
 */
#ifdef KJB_HAVE_CUDA
class Cuda_base_module
{
public:
    /**
     * Loads the binary CUDA module, or JIT-compile the ptx code file.
     */
    Cuda_base_module(const std::string& mod_fname, int max_registers = -1) :
        handle_(Cuda::load_module_file(mod_fname, max_registers))
    {
    }

    Cuda_base_module(const char* mod_code, int max_registers = -1) :
        handle_(Cuda::load_module(mod_code, max_registers))
    {
    }

    virtual ~Cuda_base_module()
    {
        cuModuleUnload(handle_);
    }


protected:
    CUmodule& get_handle_() { return handle_; }
private:

    CUmodule handle_;

};
#endif

/**
 * Loads and stores the "Reduce" cuda module, and provides an interface
 * to its functionality.
 */
#ifdef KJB_HAVE_CUDA
class Cuda_reduce_module : public Cuda_base_module
{
private:
typedef Cuda_base_module Base;

    /// This maps types to array indeces in type_strings
    template<class T>
    int type_index() { KJB_THROW(Not_implemented); }

    /**************
     * ADDING SUPPORT FOR NEW TYPES
     *
     * Currently, float and unsigned char are supported. Adding support for new types
     * requires changes in multiple places:
     *  
     *  1. reduce.cu:  Create a set of wrapper functions for your new type
     *  2. Add a new "Support for XXX" section like the ones for float and unsigned char later in this file.
     *  3. Add an entry to the type_strings array in load_functions.
     *  4. Increment the NUM_TYPES constant below.
     */
//
// The constants below aid in building the name strings of cuda kernel functions in reduce.cu
//
    static const int NUM_TYPES = 2; // length of type_strings

    // 1, 2, ..., 128, 512  -- total of 10; one for each function in reduce.cu
    static const int NUM_SIZES = 10;
public:
    Cuda_reduce_module();

    /**
     * A special version of reduce() that treats the byte array in d_in as
     * the packed bytes of a float value.
     */
//    float packed_float_reduce(CUdeviceptr d_in, int N);

    /**
     * Sum a float array of size N stored at location d_in on the 
     * currently active GPU device.
     *
     * This interface allows users to use already-allocated device
     * memory, which is particularly useful when processing something
     * that OpenGL rendered without copying data between GPU device and
     * main memory.
     *
     * usage reduce_module.reduce<float>(d_in, N); // where d_in points to a float array
     *
     * @tparam T Type of array to reduce.  Return type is usually also T, except 
     *           when T is too small to hold the result (e.g. unsigned char)
     * @param d_in Cuda device pointer to array in device memory.
     * @param N Number of elements in array
     */
    template <class T>
    double reduce(CUdeviceptr d_in, int N);

    template <class T>
    double reduce(CUarray* da_in, int width, int height);

    /**
     * Sum a float array of size N store in main memory at location h_in.
     *
     * This is useful when the data already resides in main memory, but is 
     * slower than the version that takes a CUdeviceptr, because it requires
     * allocating and writing to GPU device memory.
     *
     * @tparam T Type of array to reduce.  Return type is usually also T, except 
     *           when T is too small to hold the result (e.g. unsigned char)
     *
     * @param h_in a pointer to a float array in main memory (on host).
     * @param N size of the array
     *
     * @return The sum of elements in h_in.
     */
    template <class T>
    double reduce(T* h_in, int N);

    /**
     * Sum a kjb array of size N store in main memory at location h_in.
     *
     * This is useful when the data already resides in main memory, but is 
     * slower than the version that takes a CUdeviceptr, because it requires
     * allocating and writing to GPU device memory.
     *
     * @note GPU operations use floats, but Vector uses double so some precision will be lost, 
     * and it will cost some time to convert all values to float before copying to the GPU device.
     */
    float reduce(const kjb::Vector& , int /*N*/)
    {
        // TODO
        KJB_THROW(Not_implemented);
    }

    float reduce(const std::vector<float>&, int /*N*/)
    {
        // TODO
        KJB_THROW(Not_implemented);
    }

    double tex_reduce(CUarray da_in, int width, int height);
    double tex_count(CUarray da_in, int width, int height);
    double chamfer_reduce(CUdeviceptr d_in_1, CUdeviceptr d_in_2, int N, bool square);
    double chamfer_reduce(CUarray tex_1, CUarray tex_2, int width, int height);

    double tex_function(CUarray da_in, int width, int height, CUfunction func);


    void get_num_blocks_and_threads_(int n, int maxBlocks, int maxThreads, int& blocks, int& threads)
    {
#ifndef MIN
#define MIN(x,y) ((x < y) ? x : y)
#endif

        threads = (n < maxThreads*2) ? nextPow2((n + 1)/ 2) : maxThreads;
        blocks = (n + (threads * 2 - 1)) / (threads * 2);

        blocks = MIN(maxBlocks, blocks);
    }

    int get_function_index_(int type_index, int threads, bool is_pow_2);

    /**
     * Called during initialization, this method should retreive
     * handles to all module functions and store them in private members.
     */
    virtual void load_functions();

    static int is_pow_2(unsigned int x)
    {
        return ((x&(x-1))==0);
    }

    static unsigned int nextPow2( unsigned int x ) 
    {
        --x;
        x |= x >> 1;
        x |= x >> 2;
        x |= x >> 4;
        x |= x >> 8;
        x |= x >> 16;
        return ++x;
    }
private:
    static const int NUM_FUNCTIONS = NUM_SIZES * NUM_TYPES * 2;

    CUfunction functions_[NUM_FUNCTIONS];
    CUfunction tex_reduce_function;
    CUfunction tex_count_function;
    CUfunction chamfer_and_reduce_function; // testing
    CUfunction squared_chamfer_and_reduce_function; // testing
    CUfunction chamfer_reduce_function;

    CUtexref tex_ref_1;
    CUtexref tex_ref_2;

    // TODO: Make these adjustible, or at least check device capabilities for these.
    // They are reasonable lower bounds, though.
    static const size_t MAX_THREADS = 256;
    static const size_t MAX_BLOCKS = 64;


};
#endif


#ifdef KJB_HAVE_CUDA
template <class T>
double Cuda_reduce_module::reduce(CUdeviceptr d_in, int N)
{
    using boost::scoped_array;
    using kjb_c::kjb_debug_level;
    using kjb_c::add_error;

    CUcontext ctx;
    cuCtxAttach(&ctx, 0);

    int blocks, threads;

    get_num_blocks_and_threads_(N, MAX_BLOCKS, MAX_THREADS, blocks, threads);

    scoped_array<float> h_out(new float[blocks]);
    double result;
    CUdeviceptr d_out = NULL;

    CUresult err = CUDA_SUCCESS;


    // TODO: make this allocation exception-safe
    EGC(err = cuMemAlloc(&d_out, sizeof(float) * blocks));

    // function
    CUfunction function;

    // get the reduction function designed for 2^thread_i threads
    {
        const int type_index_ = type_index<T>();
        const int i = get_function_index_(type_index_, threads, is_pow_2(N));
        function = functions_[i];
    }

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

        ptr = (void*)(size_t) d_in;
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
#endif

#ifdef KJB_HAVE_CUDA
template <class T>
double Cuda_reduce_module::reduce(T* h_in, int N)
{

    using kjb_c::kjb_debug_level;
    using kjb_c::add_error;

    double result;
    CUresult err = CUDA_SUCCESS;

    // allocate memory on device
    CUdeviceptr d_in = NULL;
    const size_t size = N * sizeof(T);

    CUcontext ctx = NULL;
    EGC(err = cuCtxAttach(&ctx, 0));

    // TODO: Make this exception safe
    EGC( err = cuMemAlloc(&d_in, size));

    // copy data to device
    EGC(err = cuMemcpyHtoD(d_in, h_in, size));


    // do reduction
    try{
        result = reduce<T>(d_in, N);
    }
    catch(Cuda_error& ex)
    {
        // please, god, lets add exception safety so these kludges aren't necessary!
        err = ex.get_cuda_error();
        goto cleanup;
    }

    // cleanup
cleanup:
    if(d_in)
        cuMemFree(d_in);

    cuCtxDetach(ctx);

    if(err != CUDA_SUCCESS)
    {
        throw Cuda_error(err, __FILE__, __LINE__);
    }

    return result;
}
#endif


#ifdef KJB_HAVE_CUDA
/* *************
 * Support for float 
 * *************/

/// This maps types to array indeces in type_strings
template<>
int Cuda_reduce_module::type_index<float>();
#endif

#ifdef KJB_HAVE_CUDA
/* *************
 * Support for unsigned char
 * *************/
/// This maps types to array indeces in type_strings
template<>
int Cuda_reduce_module::type_index<unsigned char>();
#endif

/* ************
 * end unsigned char
 * ************/


/* ************
 * UTILITY FUNCTIONS
 * ************/

#ifdef KJB_HAVE_CUDA
/**
 * Create a cuda array and initialize it with the contents of a matrix.  Values will be cast to float.  
 *
 * @flip_y If true, the rows of m will be reversed in order, which is useful when interoperating with opengl, whose color buffers map to CUarray's in a similar way.
 */
CUarray create_cuda_array(const Matrix& m, bool flip_y);

/**
 * unwrap matrix into a pitched array.  Pitch may be larger
 * than width of matrix; if so, the extra margin is filled with
 * zeros.  This shouldn't be called directly; it is a utility method of create_cuda_pitch.
 *
 * @tparam Matrix_type Either Matrix or Int_matrix
 * @tparam Dest_type The type to cast matrix values to.
 */
template <class Matrix_type, class Dest_type>
Dest_type* to_pitch(const Matrix_type& m, size_t pitch, bool flip_y)
{
    typedef Dest_type T;

    // convert from bytes into elements
    pitch /= sizeof(T);

    assert(pitch >= (size_t) m.get_num_cols());

    T* out = new T[pitch * m.get_num_rows()];

    T* cur = out;

    if(flip_y)
    {
        for(int row = m.get_num_rows() - 1; row >= 0; row--)
        for(size_t col = 0; col < pitch; col++)
        {
            if(col >= (size_t) m.get_num_cols())
                *cur++ = 0.0;
            else
                *cur++ = (T) m(row,col);
        }
    }
    else
    {
        for(int row = 0; row < m.get_num_rows(); row++)
        for(size_t col = 0; col < pitch; col++)
        {
            if(col >= (size_t) m.get_num_cols())
                *cur++ = 0.0;
            else
                *cur++ = (T) m(row,col);
        }

    }

    return out;
}

/**
 * copy matrix to a pitched array in cuda.  "Gutter" elements
 * are initialized to zero.
 *
 * @tparam Matrix_type Either Matrix or Int_matrix
 * @tparam Dest_type The type to cast matrix values to.  Useful when your GPU doesn't support doubles.
 * @param pitch  outputs the array pitch. 
 * */
template <class Matrix_type, class Dest_type>
#if CUDA_VERSION < 3020
CUdeviceptr create_cuda_pitch(const Matrix_type& m, unsigned int& pitch, bool flip_y)
#else
CUdeviceptr create_cuda_pitch(const Matrix_type& m, size_t& pitch, bool flip_y)
#endif
{
   typedef Dest_type T;

   int width = m.get_num_cols();
   int height = m.get_num_rows();

   CUdeviceptr result;

   // I put this in for debugging, and may need it again someday soon...
//   Cuda_size_t free, total;
//   CU_EPETE(cuMemGetInfo(&free, &total));
//   std::cout << "Available memory: " << free << "/" << total << std::endl;
   
    CU_ETX(cuMemAllocPitch(
            &result,
            &pitch, // pitch
            width * sizeof(T),
            height,
            sizeof(T)));

   boost::scoped_array<T> data(to_pitch<Matrix_type, T>(m, pitch, flip_y));

   CUDA_MEMCPY2D cpy_meta;
   cpy_meta.srcMemoryType = CU_MEMORYTYPE_HOST;
   cpy_meta.srcXInBytes = 0;
   cpy_meta.srcY = 0;
   cpy_meta.srcHost = data.get();
   cpy_meta.srcDevice = 0;
   cpy_meta.srcArray = 0;
   cpy_meta.srcPitch = pitch;
   cpy_meta.dstXInBytes = 0;
   cpy_meta.dstY = 0;
   cpy_meta.dstMemoryType = CU_MEMORYTYPE_DEVICE;
   cpy_meta.dstHost = 0;
   cpy_meta.dstDevice = result;
   cpy_meta.dstArray = 0;
   cpy_meta.dstPitch = pitch;
   cpy_meta.WidthInBytes = pitch; //width * sizeof(float);
   cpy_meta.Height = height;

   CU_ETX(cuMemcpy2D(&cpy_meta));

#ifdef TEST
   boost::scoped_array<T> o_data(new T[width * height]);

   cpy_meta.srcXInBytes = 0;
   cpy_meta.srcY = 0;
   cpy_meta.srcMemoryType = CU_MEMORYTYPE_DEVICE;
   cpy_meta.srcHost = 0;
   cpy_meta.srcDevice = result;
   cpy_meta.srcArray = 0;
   cpy_meta.srcPitch = pitch;
   cpy_meta.dstMemoryType = CU_MEMORYTYPE_HOST;
   cpy_meta.dstXInBytes = 0;
   cpy_meta.dstY = 0;
   cpy_meta.dstHost = o_data.get();
   cpy_meta.dstDevice = 0;
   cpy_meta.dstArray = 0;
   cpy_meta.dstPitch = 0;
   cpy_meta.WidthInBytes = width * sizeof(T);
   cpy_meta.Height = height;

   CU_ETX(cuMemcpy2D(&cpy_meta));


    for(int row = 0; row < height; row++)
    for(int col = 0; col < width; col++)
    {
        int rev_row = height - row - 1;
        int in_i = row * pitch / sizeof(T) + col;
        int out_i = row * width + col;
        assert(data[in_i] == o_data[out_i]);
        if(flip_y)
            assert(fabs(m(rev_row, col) - o_data[out_i]) <= fabs(m(rev_row, col)) * FLT_EPSILON);
        else
            assert(fabs(m(row, col) -  o_data[out_i]) <= fabs(m(row, col)) * FLT_EPSILON);
    }
#endif


   return result;
}
#endif








//const size_t Cuda_reduce_module MAX_THREADS = 256;
//const size_t Cuda_reduce_module MAX_BLOCKS = 64;
} // namespace kjb
} // namespace gpu





#endif


// vim: tabstop=4 shiftwidth=4 foldmethod=marker

