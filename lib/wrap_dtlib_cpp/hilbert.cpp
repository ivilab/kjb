/////////////////////////////////////////////////////////////////////////////
// hilbert.cpp - hilbert transform
// Author: Doron Tal (this code is based on the matlab (v5.3) 'hilbert.m')
// Date created: March, 2000

#include <string.h>
#include "wrap_dtlib_cpp/hilbert.h"
#include "wrap_dtlib_cpp/utils.h"
#include "wrap_dtlib_cpp/fft.h"

using namespace DTLib;

/////////////////////////////////////////////////////////////////////////////
// In-place hilbert transform on aInVector.

void DTLib::Hilbert(float *aInVector, const int Length)
{
    float *aRe, *aIm;
    // convert to a power of 2 sequence to speed up fft
    const int LengthFFT = (int)pow(2.0, (double)NextPowerOfTwo(Length));
    bool bAllocatedRe; // remember so that we can free it later if necc.
    if (LengthFFT == Length) { 
        aRe = aInVector; // input is real
        aIm = new float[Length];
    }
    else { // need to enlarge & copy sequences
        bAllocatedRe = true;
        aRe = new float[LengthFFT];
        aIm = new float[LengthFFT];
        memcpy(aRe, aInVector, Length*sizeof(float)); // input is real
        memset(aRe+Length, 0, (LengthFFT-Length)*sizeof(float)); // zero-pad
    }
    memset(aIm, 0, LengthFFT*sizeof(float)); // imaginary is zero
    // convolve with Hilbert kernel in Fourier domain:
    FwdFFT(aRe, aIm, LengthFFT);
    const int nHilbertTwos = (LengthFFT/2)-1; // # of twos in hilbert kernel
    float *pRe = aRe+1, *pIm = aIm+1; // 1st elt untouched
    for (int i = 0; i < nHilbertTwos; i++, pRe++, pIm++) {
        *pRe *= 2.0f;
        *pIm *= 2.0f;
    }
    pRe++; pIm++; // next elt untouched
    memset(pRe, 0, nHilbertTwos*sizeof(float)); // the rest are zero
    memset(pIm, 0, nHilbertTwos*sizeof(float));
    InvFFT(aRe, aIm, LengthFFT);
    // copy over the result (aIm) into aInVector
    memcpy(aInVector, aIm, Length*sizeof(float));
    zap(aRe);
    zap(aIm);
}
