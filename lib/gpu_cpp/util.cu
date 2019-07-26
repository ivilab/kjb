/*
    General-purpose math kernels.
*/

#ifndef _LIKELIHOOD_KERNEL_H_
#define _LIKELIHOOD_KERNEL_H_

#include <stdio.h>
#include <assert.h>

#ifdef __DEVICE_EMULATION__
#define EMUSYNC __syncthreads()
#else
#define EMUSYNC
#endif

// Utility class used to avoid linker errors with extern
// unsized shared memory arrays with templated type
template<class T>
struct SharedMemory
{
    __device__ inline operator       T*()
    {
        extern __shared__ int __smem[];
        return (T*)__smem;
    }

    __device__ inline operator const T*() const
    {
        extern __shared__ int __smem[];
        return (T*)__smem;
    }
};

// specialize for double to avoid unaligned memory 
// access compile errors
template<>
struct SharedMemory<double>
{
    __device__ inline operator       double*()
    {
        extern __shared__ double __smem_d[];
        return (double*)__smem_d;
    }

    __device__ inline operator const double*() const
    {
        extern __shared__ double __smem_d[];
        return (double*)__smem_d;
    }
};

/**
  OverWriting, Element-Wise Multiply kernel.  This tenmplated kernel is 
  called from a non-template kernel for ease of calling from the driver api
  */
template <class T1, class T2>
__device__ void
ow_ew_multiply_dispatch(T1 *data1, T2 *data2, unsigned int n)
{
    // perform first level of reduction,
    // reading from global memory, writing to shared memory
    unsigned int i = blockIdx.x*blockDim.x+ threadIdx.x;
    
    T1 tmp;
    if(i < n)
    {
        tmp = data1[i];
        tmp = (T1) (tmp * data2[i]);
        data1[i] = tmp;
    }
}

extern "C" __global__ void 
ow_ew_multiply_uint_float(unsigned int* data1, float* data2, unsigned int n)
{
    ow_ew_multiply_dispatch<unsigned int, float>(data1, data2, n);
}

extern "C" __global__ void 
ow_ew_multiply_float(float* data1, float* data2, unsigned int n)
{
    ow_ew_multiply_dispatch<float, float>(data1, data2, n);
}

extern "C" __global__ void 
ow_ew_multiply_uint(unsigned int* data1, unsigned int* data2, unsigned int n)
{
    ow_ew_multiply_dispatch<unsigned int, unsigned int>(data1, data2, n);
}

extern "C" __global__ void 
ow_ew_multiply_int(int* data1, int* data2, unsigned int n)
{
    ow_ew_multiply_dispatch<int, int>(data1, data2, n);
}


/**
  Creates an array with zeros everywhere except when the current
  element and the previous element differ
  */
template <class T>
__device__ void
detect_changes_dispatch(T *data, unsigned int n)
{
    unsigned int i = blockIdx.x*blockDim.x + threadIdx.x;
    
    T tmp;
    T tmp2;

    if(i == 0)
    {
        data[i] = 0;
        return;
    }

    if(i+1 < n)
    {
        tmp  = data[i];
        tmp2 = data[i+1];

        if(tmp == tmp2)
            data[i+1] = 0;
        else
            data[i+1] = 1;
    }

}

extern "C" __global__ void
detect_changes_float(float *data, unsigned int n)
{
    detect_changes_dispatch<float>(data, n);
}

extern "C" __global__ void
detect_changes_int(int *data, unsigned int n)
{
    detect_changes_dispatch<int>(data, n);
}


extern "C" __global__ void
detect_changes_uint(unsigned int *data, unsigned int n)
{
    detect_changes_dispatch<unsigned int>(data, n);
}


#endif // #ifndef _LIKELIHOOD_KERNEL_H_
