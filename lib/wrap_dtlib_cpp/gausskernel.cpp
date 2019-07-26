/////////////////////////////////////////////////////////////////////////////
// gausskernel.cpp - generate 2D rotated Gaussian (derivative) kenels
// Author: Doron Tal
// Date created: March, 2000

#include <math.h>
#include "wrap_dtlib_cpp/gausskernel.h"
#include "wrap_dtlib_cpp/img.h"
#include "wrap_dtlib_cpp/utils.h"
#include "wrap_dtlib_cpp/rotate.h"
#include "wrap_dtlib_cpp/hilbert.h"

using namespace DTLib;

/////////////////////////////////////////////////////////////////////////////
// PRECOND: 'Size' must be odd (so that we can center the kernel perfectly).
// POSTCOND: becomes a float CImg with the kernel in it

DTLib::CGaussKernel::CGaussKernel(const int& Size, const float& SigmaX, 
                                  const float& SigmaY, const int& OrderY, 
                                  const float& Theta, const bool& bHilbert)
    : CImg<float>(Size, Size)
{
    Draw(SigmaX, SigmaY,OrderY, Theta, bHilbert);
}

/////////////////////////////////////////////////////////////////////////////

DTLib::CGaussKernel::CGaussKernel(const int& Size)
    : CImg<float>(Size, Size)
{
    ;
}

/////////////////////////////////////////////////////////////////////////////

void DTLib::CGaussKernel::Draw(const float& SigmaX, const float& SigmaY,
                               const int& OrderY, const float& Theta,
                               const bool& bHilbert)
{
    if (Theta != 0.0f) { 
        int NewSize = F2I(1.5f*(float)m_Width);
        ODDIFY(NewSize);
        const int HalfSizeDiff = (NewSize-m_Width)/2;
        CGaussKernel BigImg(NewSize);
        CGaussKernel BigImg2(NewSize); // wasteful!!!
        // draw first into a bigger image, for rotation to come out nicely
        BigImg.DrawNoRot(SigmaX, SigmaY,OrderY, bHilbert);
        // rotate kernel by 'Theta'
        Rotate(BigImg, BigImg2, (double)-Theta, 0.0f);
        BigImg2.Extract(*this, HalfSizeDiff, HalfSizeDiff);
    }
    else DrawNoRot(SigmaX, SigmaY, OrderY, bHilbert);

    // subtract the mean to make absolutely sure filter has 0 mean
    // even though it should already be zero mean (but sampling errors
    // due to the discreteness of the image call for doing this again here)
    Sub(MeanLuminance()); 

    NormalizeL1(); // L1 Norm
}    

/////////////////////////////////////////////////////////////////////////////

void DTLib::CGaussKernel::DrawNoRot(const float& SigmaX, const float& SigmaY,
                                    const int& OrderY, const bool& bHilbert)
{
    m_SigmaX = SigmaX;
    m_SigmaY = SigmaY;
    m_OrderY = OrderY;
    const int HalfSize = m_Width/2;
    const float Norm = 1.0f/(SigmaX*SigmaY*CONST_2PI);
    const float SigmaSqrX = SigmaX*SigmaX;
    const float SigmaSqrY = SigmaY*SigmaY;
    const float RecipTwoSigmaSqrX = 1.0f/(2.0f*SigmaSqrX);
    const float RecipTwoSigmaSqrY = 1.0f/(2.0f*SigmaSqrY);
    const float RecipSigmaSqrY = 1.0f/SigmaSqrY;
    const float RecipSigmaFourthY = 1.0f/(SigmaSqrY*SigmaSqrY);
    float* aY = new float[m_Width]; // compute Y-Gaussian (or Hilbert of) here
    float* pY = aY;
    int y;
    for (y = -HalfSize; y <= HalfSize; y++, pY++) {
        const float YSqr = (float)(y*y);
        float fy = (float)exp(-YSqr*RecipTwoSigmaSqrY); // zeroth derivative
        if (OrderY == 1) // first derivative
            fy *=  -(float)y*RecipSigmaSqrY;
        else if (OrderY == 2) // second drivative
            fy *= RecipSigmaSqrY*(YSqr*RecipSigmaSqrY-1);
        else if (OrderY == 3) // third derivative
            fy *= RecipSigmaFourthY*(float)y*(3-YSqr*RecipSigmaSqrY);
        *pY = fy;
    } // for (int y = -HalfSize; y <= HalfSize; y++, pY++) {
    if (bHilbert) Hilbert(aY, m_Width);
    float *pBuf = pBuffer();
    pY = aY;
    for (y = -HalfSize; y <= HalfSize; y++, pY++)
        for (int x = -HalfSize; x <= HalfSize; x++, pBuf++) {
            const float XSqr = (float)(x*x);
            const float fx = (float)exp(-XSqr*RecipTwoSigmaSqrX);
            *pBuf = fx*(*pY)*Norm;
        } // for (int x = -HalfSize; x < HalfSize; x++) {
    zap(aY);
    if (OrderY == 0) {
        // *** TODO: *** test this code in this if stmt
        // isotropic Gaussian kernel
        ChangeRange(0.0f, 1.0f);
    }
}
