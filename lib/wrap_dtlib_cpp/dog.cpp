/////////////////////////////////////////////////////////////////////////////
// dog.cpp - generate 2D difference of Gaussian kernels.
// Author: Doron Tal
// Date created: April, 2000

#include "wrap_dtlib_cpp/dog.h"
#include "wrap_dtlib_cpp/gausskernel.h"

using namespace DTLib;

CDOG::CDOG(const int& Size, const float& ExcitSigma, const float& InhibSigma1, 
           const float& InhibSigma2)
    : CGaussKernel(Size, ExcitSigma, ExcitSigma, 0, 0.0f, false)
{
    CGaussKernel gk_i1(Size, InhibSigma1, InhibSigma1);
    CGaussKernel gk_i2(Size, InhibSigma2, InhibSigma2);
    
    Mul(2.0f);
    Sub(gk_i1);
    Sub(gk_i2);
    
    Sub(MeanLuminance());
    NormalizeL1();
}
