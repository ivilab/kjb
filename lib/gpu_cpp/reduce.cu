/*
 * Copyright 1993-2010 NVIDIA Corporation.  All rights reserved.
 *
 * Please refer to the NVIDIA end user license agreement (EULA) associated
 * with this source code for terms and conditions that govern your use of
 * this software. Any use, reproduction, disclosure, or distribution of
 * this software and related documentation outside the terms of the EULA
 * is strictly prohibited.
 *
 */

/*
    Parallel reduction kernels
*/

#ifndef _REDUCE_KERNEL_H_
#define _REDUCE_KERNEL_H_

#include <stdio.h>
#include <assert.h>

#ifdef __DEVICE_EMULATION__
#define EMUSYNC __syncthreads()
#else
#define EMUSYNC
#endif

static texture<float, 2, cudaReadModeElementType> tex_ref_1;
static texture<float, 2, cudaReadModeElementType> tex_ref_2;

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

template <unsigned int blockSize>
__device__ void
reduceBlock(volatile float *sdata, float mySum, const unsigned int tid)
{
    sdata[tid] = mySum;
    __syncthreads();

    // do reduction in shared mem
    if (blockSize >= 512) { if (tid < 256) { sdata[tid] = mySum = mySum + sdata[tid + 256]; } __syncthreads(); }
    if (blockSize >= 256) { if (tid < 128) { sdata[tid] = mySum = mySum + sdata[tid + 128]; } __syncthreads(); }
    if (blockSize >= 128) { if (tid <  64) { sdata[tid] = mySum = mySum + sdata[tid +  64]; } __syncthreads(); }

#ifndef __DEVICE_EMULATION__
    if (tid < 32)
#endif
    {
        if (blockSize >=  64) { sdata[tid] = mySum = mySum + sdata[tid + 32]; EMUSYNC; }
        if (blockSize >=  32) { sdata[tid] = mySum = mySum + sdata[tid + 16]; EMUSYNC; }
        if (blockSize >=  16) { sdata[tid] = mySum = mySum + sdata[tid +  8]; EMUSYNC; }
        if (blockSize >=   8) { sdata[tid] = mySum = mySum + sdata[tid +  4]; EMUSYNC; }
        if (blockSize >=   4) { sdata[tid] = mySum = mySum + sdata[tid +  2]; EMUSYNC; }
        if (blockSize >=   2) { sdata[tid] = mySum = mySum + sdata[tid +  1]; EMUSYNC; }
    }
}

template<bool square>
__device__ void
chamfer_and_reduce_dispatch(float *g_idata_1, float *g_idata_2, float *g_odata, unsigned int n)
{
    float *sdata = SharedMemory<float>();

    // perform first level of reduction,
    // reading from global memory, writing to shared memory
    const unsigned int blockSize = 256;
    unsigned int tid = threadIdx.x;
    unsigned int i = blockIdx.x*blockSize*2 + threadIdx.x;
    unsigned int gridSize = blockSize*2*gridDim.x;
    
    float mySum = 0;

    // we reduce multiple elements per thread.  The number is determined by the 
    // number of active thread blocks (via gridDim).  More blocks will result
    // in a larger gridSize and therefore fewer elements per thread
    while (i < n)
    { 
        if(square)
        {
            float result = g_idata_1[i] * g_idata_2[i];
            mySum += result * result;
        }
        else
        {
            mySum += g_idata_1[i] * g_idata_2[i];
        }

        // ensure we don't read out of bounds -- this is optimized away for powerOf2 sized arrays
        if (i + blockSize < n) 
        {
            if(square)
            {
                float result = g_idata_1[i+blockSize] * g_idata_2[i+blockSize];
                result *= result;
                mySum += result;
            }
            else
            {
                mySum += g_idata_1[i+blockSize] * g_idata_2[i+blockSize];  
            }
        }
        i += gridSize;
    } 

    reduceBlock<blockSize>(sdata, mySum, tid);

    // write result for this block to global mem 
    if (tid == 0) 
        g_odata[blockIdx.x] = sdata[0];
}


extern "C" __global__ void
chamfer_and_reduce(float *g_idata_1, float *g_idata_2, float *g_odata, unsigned int n)
{
    chamfer_and_reduce_dispatch<false>(g_idata_1, g_idata_2, g_odata, n);
}

extern "C" __global__ void
squared_chamfer_and_reduce(float *g_idata_1, float *g_idata_2, float *g_odata, unsigned int n)
{
    chamfer_and_reduce_dispatch<true>(g_idata_1, g_idata_2, g_odata, n);
}


/*
    This version adds multiple elements per thread sequentially.  This reduces the overall
    cost of the algorithm while keeping the work complexity O(n) and the step complexity O(log n).
    (Brent's Theorem optimization)

    Note, this kernel needs a minimum of 64*sizeof(T) bytes of shared memory. 
    In other words if blockSize <= 32, allocate 64*sizeof(T) bytes.  
    If blockSize > 32, allocate blockSize*sizeof(T) bytes.
*/
template <class T_in, unsigned int blockSize, bool nIsPow2>
__device__ void
reduce(T_in *g_idata, float *g_odata, unsigned int n)
{
    float *sdata = SharedMemory<float>();

    // perform first level of reduction,
    // reading from global memory, writing to shared memory
    unsigned int tid = threadIdx.x;
    unsigned int i = blockIdx.x*blockSize*2 + threadIdx.x;
    unsigned int gridSize = blockSize*2*gridDim.x;
    
    float mySum = 0;

    // we reduce multiple elements per thread.  The number is determined by the 
    // number of active thread blocks (via gridDim).  More blocks will result
    // in a larger gridSize and therefore fewer elements per thread
    while (i < n)
    {         
        mySum += g_idata[i];
        // ensure we don't read out of bounds -- this is optimized away for powerOf2 sized arrays
        if (nIsPow2 || i + blockSize < n) 
            mySum += g_idata[i+blockSize];  
        i += gridSize;
    } 

    reduceBlock<blockSize>(sdata, mySum, tid);

    // write result for this block to global mem 
    if (tid == 0) 
        g_odata[blockIdx.x] = sdata[0];
}

/**
  Convert a linear index to a 2D texture coordinate
  */
__device__ inline void index_to_texcoord(unsigned int i, float* u, float* v, unsigned int stride)
{
    // TODO: Test with width and height as stride
    // TODO: Test with u and v swapped
        // no affect

    // TODO: implement as z-order curve
    // TODO: test order of z-order curve
        // this is too expensive to make useful

    // TODO: try to index nearby
        // can get time down from 11s to 8s, but the indexing isn't right (didn't try to get it right)
    *u = i % stride;
    *v = i / stride;
}

//template <class T_in, unsigned int blockSize, bool nIsPow2> // T_in == float always
/**
 * @param n The width * height of the texture.
 * @param stride Length or Width of texture.  which dimension is used will determine how texture lookups occur; swapping may improve cache locality
 */
template <unsigned int blockSize, bool nIsPow2>
__device__ void
tex_reduce(float *g_odata, unsigned int n, unsigned int stride)
{
    float *sdata = SharedMemory<float>();

    // perform first level of reduction,
    // reading from global memory, writing to shared memory
    unsigned int tid = threadIdx.x;
    unsigned int i = blockIdx.x*blockSize*2 + threadIdx.x;
    float u, v;

    unsigned int gridSize = blockSize*2*gridDim.x;
    
    float mySum = 0;

    // we reduce multiple elements per thread.  The number is determined by the 
    // number of active thread blocks (via gridDim).  More blocks will result
    // in a larger gridSize and therefore fewer elements per thread
    while (i < n)
    {
        index_to_texcoord(i, &u, &v, stride);
        mySum += tex2D(tex_ref_1, u, v);
        // ensure we don't read out of bounds -- this is optimized away for powerOf2 sized arrays
        if (nIsPow2 || i + blockSize < n) 
        {
            index_to_texcoord(i+blockSize, &u, &v, stride);
            mySum += tex2D(tex_ref_1, u, v);
        }
        i += gridSize;
    } 

    reduceBlock<blockSize>(sdata, mySum, tid);

    // write result for this block to global mem 
    if (tid == 0) 
        g_odata[blockIdx.x] = sdata[0];
}

//template <class T_in, unsigned int blockSize, bool nIsPow2> // T_in == float always
/**
 * Multiply a two textures and then reduce.  This will give the unnormalized chamfer distance
 * if one texture is a distance map and the other is an edge mask containing 1.0's and 0.0's.  To get the normalization factor, pass the edge mask texture to tex_count.
 *
 * @param n The width * height of the texture.
 * @param stride Length or Width of texture.  which dimension is used will determine how texture lookups occur; swapping may improve cache locality
 */
template <unsigned int blockSize, bool nIsPow2>
__device__ void
chamfer_reduce(float *g_odata, unsigned int n, unsigned int stride)
{
    float *sdata = SharedMemory<float>();

    // perform first level of reduction,
    // reading from global memory, writing to shared memory
    unsigned int tid = threadIdx.x;
    unsigned int i = blockIdx.x*blockSize*2 + threadIdx.x;
    float u, v;

    unsigned int gridSize = blockSize*2*gridDim.x;
    
    float mySum = 0;

    // we reduce multiple elements per thread.  The number is determined by the 
    // number of active thread blocks (via gridDim).  More blocks will result
    // in a larger gridSize and therefore fewer elements per thread
    while (i < n)
    {
        index_to_texcoord(i, &u, &v, stride);
        mySum += tex2D(tex_ref_1, u, v) * tex2D(tex_ref_2, u, v);
        // ensure we don't read out of bounds -- this is optimized away for powerOf2 sized arrays
        if (nIsPow2 || i + blockSize < n) 
        {
            index_to_texcoord(i+blockSize, &u, &v, stride);
            mySum += tex2D(tex_ref_1, u, v) * tex2D(tex_ref_2, u, v);
        }
        i += gridSize;
    } 

    reduceBlock<blockSize>(sdata, mySum, tid);

    // write result for this block to global mem 
    if (tid == 0) 
        g_odata[blockIdx.x] = sdata[0];
}

//template <class T_in, unsigned int blockSize, bool nIsPow2> // T_in == float always
/**
 * Count the number of non-zero elements in a texture.
 * 
 * @param n The width * height of the texture.
 * @param stride Length or Width of texture.  which dimension is used will determine how texture lookups occur; swapping may improve cache locality
 */
template <unsigned int blockSize, bool nIsPow2>
__device__ void
tex_count(float *g_odata, unsigned int n, unsigned int stride)
{
    float *sdata = SharedMemory<float>();

    // perform first level of reduction,
    // reading from global memory, writing to shared memory
    unsigned int tid = threadIdx.x;
    unsigned int i = blockIdx.x*blockSize*2 + threadIdx.x;
    float u, v;

    unsigned int gridSize = blockSize*2*gridDim.x;
    
    float mySum = 0;

    // we reduce multiple elements per thread.  The number is determined by the 
    // number of active thread blocks (via gridDim).  More blocks will result
    // in a larger gridSize and therefore fewer elements per thread
    while (i < n)
    {
        index_to_texcoord(i, &u, &v, stride);
        if(tex2D(tex_ref_1, u, v) != 0.0)
            mySum += 1;;

        // ensure we don't read out of bounds -- this is optimized away for powerOf2 sized arrays
        if (nIsPow2 || i + blockSize < n) 
        {
            index_to_texcoord(i+blockSize, &u, &v, stride);
            if(tex2D(tex_ref_1, u, v) != 0.0)
                mySum += 1;
        }

        i += gridSize;
    } 

    reduceBlock<blockSize>(sdata, mySum, tid);

    // write result for this block to global mem 
    if (tid == 0) 
        g_odata[blockIdx.x] = sdata[0];
}


/*
 * Since we can't call template kernels from the Cuda Driver API,
 * we wrap them here as C functions, then call those through the Driver.
 */

/*********************************
 * The 'float' versions.
 *********************************/

extern "C" __global__ void reduce_float_1_true(float *g_idata, float *g_odata, unsigned int n)
{
    reduce<float, 1, true>(g_idata, g_odata, n);
}

extern "C" __global__ void reduce_float_2_true(float *g_idata, float *g_odata, unsigned int n)
{
    reduce<float, 2, true>(g_idata, g_odata, n);
}

extern "C" __global__ void reduce_float_4_true(float *g_idata, float *g_odata, unsigned int n)
{
    reduce<float, 4, true>(g_idata, g_odata, n);
}

extern "C" __global__ void reduce_float_8_true(float *g_idata, float *g_odata, unsigned int n)
{
    reduce<float, 8, true>(g_idata, g_odata, n);
}

extern "C" __global__ void reduce_float_16_true(float *g_idata, float *g_odata, unsigned int n)
{
    reduce<float, 16, true>(g_idata, g_odata, n);
}

extern "C" __global__ void reduce_float_32_true(float *g_idata, float *g_odata, unsigned int n)
{
    reduce<float, 32, true>(g_idata, g_odata, n);
}

extern "C" __global__ void reduce_float_64_true(float *g_idata, float *g_odata, unsigned int n)
{
    reduce<float, 64, true>(g_idata, g_odata, n);
}

extern "C" __global__ void reduce_float_128_true(float *g_idata, float *g_odata, unsigned int n)
{
    reduce<float, 128, true>(g_idata, g_odata, n);
}

extern "C" __global__ void reduce_float_256_true(float *g_idata, float *g_odata, unsigned int n)
{
    reduce<float, 256, true>(g_idata, g_odata, n);
}

extern "C" __global__ void reduce_float_512_true(float *g_idata, float *g_odata, unsigned int n)
{
    reduce<float, 512, true>(g_idata, g_odata, n);
}


extern "C" __global__ void reduce_float_1_false(float *g_idata, float *g_odata, unsigned int n)
{
    reduce<float, 1, false>(g_idata, g_odata, n);
}

extern "C" __global__ void reduce_float_2_false(float *g_idata, float *g_odata, unsigned int n)
{
    reduce<float, 2, false>(g_idata, g_odata, n);
}

extern "C" __global__ void reduce_float_4_false(float *g_idata, float *g_odata, unsigned int n)
{
    reduce<float, 4, false>(g_idata, g_odata, n);
}

extern "C" __global__ void reduce_float_8_false(float *g_idata, float *g_odata, unsigned int n)
{
    reduce<float, 8, false>(g_idata, g_odata, n);
}

extern "C" __global__ void reduce_float_16_false(float *g_idata, float *g_odata, unsigned int n)
{
    reduce<float, 16, false>(g_idata, g_odata, n);
}

extern "C" __global__ void reduce_float_32_false(float *g_idata, float *g_odata, unsigned int n)
{
    reduce<float, 32, false>(g_idata, g_odata, n);
}

extern "C" __global__ void reduce_float_64_false(float *g_idata, float *g_odata, unsigned int n)
{
    reduce<float, 64, false>(g_idata, g_odata, n);
}

extern "C" __global__ void reduce_float_128_false(float *g_idata, float *g_odata, unsigned int n)
{
    reduce<float, 128, false>(g_idata, g_odata, n);
}

extern "C" __global__ void reduce_float_256_false(float *g_idata, float *g_odata, unsigned int n)
{
    reduce<float, 256, false>(g_idata, g_odata, n);
}

extern "C" __global__ void reduce_float_512_false(float *g_idata, float *g_odata, unsigned int n)
{
    reduce<float, 512, false>(g_idata, g_odata, n);
}

/******************
  Texture version (experimental)
*********************/

extern "C" __global__ void tex_reduce_256_false(float *g_odata, unsigned int n, unsigned int stride)
{
    tex_reduce<256, false>(g_odata, n, stride);
}

extern "C" __global__ void tex_count_256_false(float *g_odata, unsigned int n, unsigned int stride)
{
    tex_count<256, false>(g_odata, n, stride);
}

extern "C" __global__ void chamfer_reduce_256_false(float *g_odata, unsigned int n, unsigned int stride)
{
    chamfer_reduce<256, false>(g_odata, n, stride);
}

/*********************************
 * The 'uchar' versions.
 *
 * These receive an array of unsigned chars, but 
 * return an array of ints, since one byte will
 * overflow too quickly.
 *********************************/

extern "C" __global__ void reduce_uchar_1_true(unsigned char *g_idata, float *g_odata, unsigned int n)
{
    reduce<unsigned char, 1, true>(g_idata, g_odata, n);
}

extern "C" __global__ void reduce_uchar_2_true(unsigned char *g_idata, float *g_odata, unsigned int n)
{
    reduce<unsigned char, 2, true>(g_idata, g_odata, n);
}

extern "C" __global__ void reduce_uchar_4_true(unsigned char *g_idata, float *g_odata, unsigned int n)
{
    reduce<unsigned char, 4, true>(g_idata, g_odata, n);
}

extern "C" __global__ void reduce_uchar_8_true(unsigned char *g_idata, float *g_odata, unsigned int n)
{
    reduce<unsigned char, 8, true>(g_idata, g_odata, n);
}

extern "C" __global__ void reduce_uchar_16_true(unsigned char *g_idata, float *g_odata, unsigned int n)
{
    reduce<unsigned char, 16, true>(g_idata, g_odata, n);
}

extern "C" __global__ void reduce_uchar_32_true(unsigned char *g_idata, float *g_odata, unsigned int n)
{
    reduce<unsigned char, 32, true>(g_idata, g_odata, n);
}

extern "C" __global__ void reduce_uchar_64_true(unsigned char *g_idata, float *g_odata, unsigned int n)
{
    reduce<unsigned char, 64, true>(g_idata, g_odata, n);
}

extern "C" __global__ void reduce_uchar_128_true(unsigned char *g_idata, float *g_odata, unsigned int n)
{
    reduce<unsigned char, 128, true>(g_idata, g_odata, n);
}

extern "C" __global__ void reduce_uchar_256_true(unsigned char *g_idata, float *g_odata, unsigned int n)
{
    reduce<unsigned char, 256, true>(g_idata, g_odata, n);
}

extern "C" __global__ void reduce_uchar_512_true(unsigned char *g_idata, float *g_odata, unsigned int n)
{
    reduce<unsigned char, 512, true>(g_idata, g_odata, n);
}


extern "C" __global__ void reduce_uchar_1_false(unsigned char *g_idata, float *g_odata, unsigned int n)
{
    reduce<unsigned char, 1, false>(g_idata, g_odata, n);
}

extern "C" __global__ void reduce_uchar_2_false(unsigned char *g_idata, float *g_odata, unsigned int n)
{
    reduce<unsigned char, 2, false>(g_idata, g_odata, n);
}

extern "C" __global__ void reduce_uchar_4_false(unsigned char *g_idata, float *g_odata, unsigned int n)
{
    reduce<unsigned char, 4, false>(g_idata, g_odata, n);
}

extern "C" __global__ void reduce_uchar_8_false(unsigned char *g_idata, float *g_odata, unsigned int n)
{
    reduce<unsigned char, 8, false>(g_idata, g_odata, n);
}

extern "C" __global__ void reduce_uchar_16_false(unsigned char *g_idata, float *g_odata, unsigned int n)
{
    reduce<unsigned char, 16, false>(g_idata, g_odata, n);
}

extern "C" __global__ void reduce_uchar_32_false(unsigned char *g_idata, float *g_odata, unsigned int n)
{
    reduce<unsigned char, 32, false>(g_idata, g_odata, n);
}

extern "C" __global__ void reduce_uchar_64_false(unsigned char *g_idata, float *g_odata, unsigned int n)
{
    reduce<unsigned char, 64, false>(g_idata, g_odata, n);
}

extern "C" __global__ void reduce_uchar_128_false(unsigned char *g_idata, float *g_odata, unsigned int n)
{
    reduce<unsigned char, 128, false>(g_idata, g_odata, n);
}

extern "C" __global__ void reduce_uchar_256_false(unsigned char *g_idata, float *g_odata, unsigned int n)
{
    reduce<unsigned char, 256, false>(g_idata, g_odata, n);
}

extern "C" __global__ void reduce_uchar_512_false(unsigned char *g_idata, float *g_odata, unsigned int n)
{
    reduce<unsigned char, 512, false>(g_idata, g_odata, n);
}


/* **********************************************************
   PACKED_FLOAT_REDUCE()

   This version of reduce() is for arrays of float values packed
   into RGBA colors.  This is the case when an RGBA texture is used
   to store the float values.  It tries to handle the case where the CPU
   and GPU have different endian-ness by swapping the bytes, but
   that branch of the code is currently untested, so proceed with caution.

 **********************************************************/


/**
* See the other reduce() kernel for usage notes.

* @tparam blockSize The number of threads per block.  Valid values are 2^m where m is between 0 and 9.
* @tparam nIsPow2 Set this to true if n is a power of 2.  The code is optimized for this case.
* @tparam odd_endian_parity Set this to true when the endian-ness of the CPU and GPU differ. (UNTESTED)
*
* @param g_idata Input data.  The array of bytes representing the float values.  Every chunk of 4 bytes stores a float value.
* @param g_odata Output data.  Result is output as a float.
* @param n the number of float elements in g_idata.  Note that the number of byte elements in g_idata is 4*n.
*/
template <unsigned int blockSize, bool nIsPow2, bool odd_endian_parity>
__device__ void
packed_float_reduce(unsigned char *g_idata, float *g_odata, unsigned int n)
{
    float *sdata = SharedMemory<float>();

    // perform first level of reduction,
    // reading from global memory, writing to shared memory
    unsigned int tid = threadIdx.x;
    unsigned int i = blockIdx.x*blockSize*2 + threadIdx.x;
    unsigned int gridSize = blockSize*2*gridDim.x;
    
    float mySum = 0;

    // we reduce multiple elements per thread.  The number is determined by the 
    // number of active thread blocks (via gridDim).  More blocks will result
    // in a larger gridSize and therefore fewer elements per thread
    while (i < n)
    {
        unsigned char address = i * 4; // could compute this when i is created to minimize register usage

        if(odd_endian_parity)
        {
            // UNTESTED!!!  50% chance that this will give the wrong answer!
            // also, it may be easier to handle this in host code, by passing GL_UINT_8_8_8_8_REV
            // to the glReadPixels call for the PBO.
                union {
                    unsigned char bytes[4];
                    float value;
                } pack;
                pack.bytes[3] = g_idata[address];
                pack.bytes[2] = g_idata[address + 1];
                pack.bytes[1] = g_idata[address + 2];
                pack.bytes[0] = g_idata[address + 3];
                mySum = pack.value;
        }
        else
        {
            mySum += *(float*)(&g_idata[address]);
        }


        // ensure we don't read out of bounds -- this is optimized away for powerOf2 sized arrays
        if (nIsPow2 || i + blockSize < n) 
        {
            address = (i + blockSize) * 4;
            if(odd_endian_parity)
            {
                union {
                    unsigned char bytes[4];
                    float value;
                } pack;
                pack.bytes[3] = g_idata[address];
                pack.bytes[2] = g_idata[address + 1];
                pack.bytes[1] = g_idata[address + 2];
                pack.bytes[0] = g_idata[address + 3];
                mySum = pack.value;
            }
            else
            {
                mySum += *(float*)(&g_idata[address]);
            }
        }
        i += gridSize;
    } 

    // each thread puts its local sum into shared memory 
    sdata[tid] = mySum;
    __syncthreads();


    // do reduction in shared mem
    if (blockSize >= 512) { if (tid < 256) { sdata[tid] = mySum = mySum + sdata[tid + 256]; } __syncthreads(); }
    if (blockSize >= 256) { if (tid < 128) { sdata[tid] = mySum = mySum + sdata[tid + 128]; } __syncthreads(); }
    if (blockSize >= 128) { if (tid <  64) { sdata[tid] = mySum = mySum + sdata[tid +  64]; } __syncthreads(); }
    
#ifndef __DEVICE_EMULATION__
    if (tid < 32)
#endif
    {
        // now that we are using warp-synchronous programming (below)
        // we need to declare our shared memory volatile so that the compiler
        // doesn't reorder stores to it and induce incorrect behavior.
        volatile float* smem = sdata;
        if (blockSize >=  64) { smem[tid] = mySum = mySum + smem[tid + 32]; EMUSYNC; }
        if (blockSize >=  32) { smem[tid] = mySum = mySum + smem[tid + 16]; EMUSYNC; }
        if (blockSize >=  16) { smem[tid] = mySum = mySum + smem[tid +  8]; EMUSYNC; }
        if (blockSize >=   8) { smem[tid] = mySum = mySum + smem[tid +  4]; EMUSYNC; }
        if (blockSize >=   4) { smem[tid] = mySum = mySum + smem[tid +  2]; EMUSYNC; }
        if (blockSize >=   2) { smem[tid] = mySum = mySum + smem[tid +  1]; EMUSYNC; }
    }
    
    // write result for this block to global mem 
    if (tid == 0) 
        g_odata[blockIdx.x] = sdata[0];
}

extern "C" __global__ void packed_float_reduce_1_false_false(unsigned char *g_idata, float *g_odata, unsigned int n)
{ packed_float_reduce<1, true, true>(g_idata, g_odata, n); }
extern "C" __global__ void packed_float_reduce_1_false_true(unsigned char *g_idata, float *g_odata, unsigned int n)
{ packed_float_reduce<1, true, true>(g_idata, g_odata, n); }
extern "C" __global__ void packed_float_reduce_1_true_false(unsigned char *g_idata, float *g_odata, unsigned int n)
{ packed_float_reduce<1, true, true>(g_idata, g_odata, n); }
extern "C" __global__ void packed_float_reduce_1_true_true(unsigned char *g_idata, float *g_odata, unsigned int n)
{ packed_float_reduce<1, true, true>(g_idata, g_odata, n); }

extern "C" __global__ void packed_float_reduce_2_false_false(unsigned char *g_idata, float *g_odata, unsigned int n)
{ packed_float_reduce<2, true, true>(g_idata, g_odata, n); }
extern "C" __global__ void packed_float_reduce_2_false_true(unsigned char *g_idata, float *g_odata, unsigned int n)
{ packed_float_reduce<2, true, true>(g_idata, g_odata, n); }
extern "C" __global__ void packed_float_reduce_2_true_false(unsigned char *g_idata, float *g_odata, unsigned int n)
{ packed_float_reduce<2, true, true>(g_idata, g_odata, n); }
extern "C" __global__ void packed_float_reduce_2_true_true(unsigned char *g_idata, float *g_odata, unsigned int n)
{ packed_float_reduce<2, true, true>(g_idata, g_odata, n); }

extern "C" __global__ void packed_float_reduce_4_false_false(unsigned char *g_idata, float *g_odata, unsigned int n)
{ packed_float_reduce<4, true, true>(g_idata, g_odata, n); }
extern "C" __global__ void packed_float_reduce_4_false_true(unsigned char *g_idata, float *g_odata, unsigned int n)
{ packed_float_reduce<4, true, true>(g_idata, g_odata, n); }
extern "C" __global__ void packed_float_reduce_4_true_false(unsigned char *g_idata, float *g_odata, unsigned int n)
{ packed_float_reduce<4, true, true>(g_idata, g_odata, n); }
extern "C" __global__ void packed_float_reduce_4_true_true(unsigned char *g_idata, float *g_odata, unsigned int n)
{ packed_float_reduce<4, true, true>(g_idata, g_odata, n); }

extern "C" __global__ void packed_float_reduce_8_false_false(unsigned char *g_idata, float *g_odata, unsigned int n)
{ packed_float_reduce<8, true, true>(g_idata, g_odata, n); }
extern "C" __global__ void packed_float_reduce_8_false_true(unsigned char *g_idata, float *g_odata, unsigned int n)
{ packed_float_reduce<8, true, true>(g_idata, g_odata, n); }
extern "C" __global__ void packed_float_reduce_8_true_false(unsigned char *g_idata, float *g_odata, unsigned int n)
{ packed_float_reduce<8, true, true>(g_idata, g_odata, n); }
extern "C" __global__ void packed_float_reduce_8_true_true(unsigned char *g_idata, float *g_odata, unsigned int n)
{ packed_float_reduce<8, true, true>(g_idata, g_odata, n); }

extern "C" __global__ void packed_float_reduce_16_false_false(unsigned char *g_idata, float *g_odata, unsigned int n)
{ packed_float_reduce<16, true, true>(g_idata, g_odata, n); }
extern "C" __global__ void packed_float_reduce_16_false_true(unsigned char *g_idata, float *g_odata, unsigned int n)
{ packed_float_reduce<16, true, true>(g_idata, g_odata, n); }
extern "C" __global__ void packed_float_reduce_16_true_false(unsigned char *g_idata, float *g_odata, unsigned int n)
{ packed_float_reduce<16, true, true>(g_idata, g_odata, n); }
extern "C" __global__ void packed_float_reduce_16_true_true(unsigned char *g_idata, float *g_odata, unsigned int n)
{ packed_float_reduce<16, true, true>(g_idata, g_odata, n); }

extern "C" __global__ void packed_float_reduce_32_false_false(unsigned char *g_idata, float *g_odata, unsigned int n)
{ packed_float_reduce<32, true, true>(g_idata, g_odata, n); }
extern "C" __global__ void packed_float_reduce_32_false_true(unsigned char *g_idata, float *g_odata, unsigned int n)
{ packed_float_reduce<32, true, true>(g_idata, g_odata, n); }
extern "C" __global__ void packed_float_reduce_32_true_false(unsigned char *g_idata, float *g_odata, unsigned int n)
{ packed_float_reduce<32, true, true>(g_idata, g_odata, n); }
extern "C" __global__ void packed_float_reduce_32_true_true(unsigned char *g_idata, float *g_odata, unsigned int n)
{ packed_float_reduce<32, true, true>(g_idata, g_odata, n); }

extern "C" __global__ void packed_float_reduce_64_false_false(unsigned char *g_idata, float *g_odata, unsigned int n)
{ packed_float_reduce<64, true, true>(g_idata, g_odata, n); }
extern "C" __global__ void packed_float_reduce_64_false_true(unsigned char *g_idata, float *g_odata, unsigned int n)
{ packed_float_reduce<64, true, true>(g_idata, g_odata, n); }
extern "C" __global__ void packed_float_reduce_64_true_false(unsigned char *g_idata, float *g_odata, unsigned int n)
{ packed_float_reduce<64, true, true>(g_idata, g_odata, n); }
extern "C" __global__ void packed_float_reduce_64_true_true(unsigned char *g_idata, float *g_odata, unsigned int n)
{ packed_float_reduce<64, true, true>(g_idata, g_odata, n); }

extern "C" __global__ void packed_float_reduce_128_false_false(unsigned char *g_idata, float *g_odata, unsigned int n)
{ packed_float_reduce<128, true, true>(g_idata, g_odata, n); }
extern "C" __global__ void packed_float_reduce_128_false_true(unsigned char *g_idata, float *g_odata, unsigned int n)
{ packed_float_reduce<128, true, true>(g_idata, g_odata, n); }
extern "C" __global__ void packed_float_reduce_128_true_false(unsigned char *g_idata, float *g_odata, unsigned int n)
{ packed_float_reduce<128, true, true>(g_idata, g_odata, n); }
extern "C" __global__ void packed_float_reduce_128_true_true(unsigned char *g_idata, float *g_odata, unsigned int n)
{ packed_float_reduce<128, true, true>(g_idata, g_odata, n); }

extern "C" __global__ void packed_float_reduce_256_false_false(unsigned char *g_idata, float *g_odata, unsigned int n)
{ packed_float_reduce<256, true, true>(g_idata, g_odata, n); }
extern "C" __global__ void packed_float_reduce_256_false_true(unsigned char *g_idata, float *g_odata, unsigned int n)
{ packed_float_reduce<256, true, true>(g_idata, g_odata, n); }
extern "C" __global__ void packed_float_reduce_256_true_false(unsigned char *g_idata, float *g_odata, unsigned int n)
{ packed_float_reduce<256, true, true>(g_idata, g_odata, n); }
extern "C" __global__ void packed_float_reduce_256_true_true(unsigned char *g_idata, float *g_odata, unsigned int n)
{ packed_float_reduce<256, true, true>(g_idata, g_odata, n); }

extern "C" __global__ void packed_float_reduce_512_false_false(unsigned char *g_idata, float *g_odata, unsigned int n)
{ packed_float_reduce<512, true, true>(g_idata, g_odata, n); }
extern "C" __global__ void packed_float_reduce_512_false_true(unsigned char *g_idata, float *g_odata, unsigned int n)
{ packed_float_reduce<512, true, true>(g_idata, g_odata, n); }
extern "C" __global__ void packed_float_reduce_512_true_false(unsigned char *g_idata, float *g_odata, unsigned int n)
{ packed_float_reduce<512, true, true>(g_idata, g_odata, n); }
extern "C" __global__ void packed_float_reduce_512_true_true(unsigned char *g_idata, float *g_odata, unsigned int n)
{ packed_float_reduce<512, true, true>(g_idata, g_odata, n); }




extern "C"
bool isPow2(unsigned int x);


/*
////////////////////////////////////////////////////////////////////////////////
// Wrapper function for kernel launch
////////////////////////////////////////////////////////////////////////////////
template <class T>
void 
cuda_reduce_host(int size, int threads, int blocks, 
       T *d_idata, T *d_odata)
{
    dim3 dimBlock(threads, 1, 1);
    dim3 dimGrid(blocks, 1, 1);

    // someday more threads may be available, but this function will have to be
    // expanded to accomodate
    assert(threads <= 512);

    // TODO check that size is less than or equal to the devices max
    // TODO check that blocks are less than or equal to the device max

    // when there is only one warp per block, we need to allocate two warps 
    // worth of shared memory so that we don't index shared memory out of bounds
    int smemSize = (threads <= 32) ? 2 * threads * sizeof(T) : threads * sizeof(T);

    if (isPow2(size))
    {
        switch (threads)
        {
        case 512:
            reduce<T, 512, true><<< dimGrid, dimBlock, smemSize >>>(d_idata, d_odata, size); break;
        case 256:
            reduce<T, 256, true><<< dimGrid, dimBlock, smemSize >>>(d_idata, d_odata, size); break;
        case 128:
            reduce<T, 128, true><<< dimGrid, dimBlock, smemSize >>>(d_idata, d_odata, size); break;
        case 64:
            reduce<T,  64, true><<< dimGrid, dimBlock, smemSize >>>(d_idata, d_odata, size); break;
        case 32:
            reduce<T,  32, true><<< dimGrid, dimBlock, smemSize >>>(d_idata, d_odata, size); break;
        case 16:
            reduce<T,  16, true><<< dimGrid, dimBlock, smemSize >>>(d_idata, d_odata, size); break;
        case  8:
            reduce<T,   8, true><<< dimGrid, dimBlock, smemSize >>>(d_idata, d_odata, size); break;
        case  4:
            reduce<T,   4, true><<< dimGrid, dimBlock, smemSize >>>(d_idata, d_odata, size); break;
        case  2:
            reduce<T,   2, true><<< dimGrid, dimBlock, smemSize >>>(d_idata, d_odata, size); break;
        case  1:
            reduce<T,   1, true><<< dimGrid, dimBlock, smemSize >>>(d_idata, d_odata, size); break;
        }
    }
    else
    {
        switch (threads)
        {
        case 512:
            reduce<T, 512, false><<< dimGrid, dimBlock, smemSize >>>(d_idata, d_odata, size); break;
        case 256:
            reduce<T, 256, false><<< dimGrid, dimBlock, smemSize >>>(d_idata, d_odata, size); break;
        case 128:
            reduce<T, 128, false><<< dimGrid, dimBlock, smemSize >>>(d_idata, d_odata, size); break;
        case 64:
            reduce<T,  64, false><<< dimGrid, dimBlock, smemSize >>>(d_idata, d_odata, size); break;
        case 32:
            reduce<T,  32, false><<< dimGrid, dimBlock, smemSize >>>(d_idata, d_odata, size); break;
        case 16:
            reduce<T,  16, false><<< dimGrid, dimBlock, smemSize >>>(d_idata, d_odata, size); break;
        case  8:
            reduce<T,   8, false><<< dimGrid, dimBlock, smemSize >>>(d_idata, d_odata, size); break;
        case  4:
            reduce<T,   4, false><<< dimGrid, dimBlock, smemSize >>>(d_idata, d_odata, size); break;
        case  2:
            reduce<T,   2, false><<< dimGrid, dimBlock, smemSize >>>(d_idata, d_odata, size); break;
        case  1:
            reduce<T,   1, false><<< dimGrid, dimBlock, smemSize >>>(d_idata, d_odata, size); break;
        }
    }
}

// Instantiate the reduction function for 3 types
template void 
cuda_reduce_host<int>(int size, int threads, int blocks, 
            int *d_idata, int *d_odata);

template void 
cuda_reduce_host<float>(int size, int threads, int blocks, 
              float *d_idata, float *d_odata);

template void 
cuda_reduce_host<double>(int size, int threads, int blocks, 
               double *d_idata, double *d_odata);
*/

#endif // #ifndef _REDUCE_KERNEL_H_
