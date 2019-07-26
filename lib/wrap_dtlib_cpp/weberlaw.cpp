/////////////////////////////////////////////////////////////////////////////
// weberlaw.cpp - weber law on filterbank convolutions (for textons)
// Author: Doron Tal
// Date Created: April, 2000

#include "wrap_dtlib_cpp/weberlaw.h"

using namespace DTLib;

/////////////////////////////////////////////////////////////////////////////

void DTLib::WeberLaw(CImgVec<float>& ConvVec, const float& WeberConst)
{
    // first, compute sum of squares along each column
    const int nFrames = ConvVec.nFrames(), 
        Width = ConvVec.Width(), Height = ConvVec.Height();
    CImg<float> ConvImg(Width, Height, false, false);
    CImg<float> SumOfSquaresImg(Width, Height, true, true);
    register int i, x, y;
    register float *pSumOfSquares, *pConv;
    for (i = 0; i < nFrames; i++) {
        pSumOfSquares = SumOfSquaresImg.pBuffer();
        pConv = ConvVec.GetImg(i)->pBuffer();
        for (y = 0; y < Height; y++) {
            for (x = 0; x < Width; x++, pSumOfSquares++, pConv++) {
                const float ConvPix = *pConv;
                *pSumOfSquares += SQR(ConvPix);
            } // for (int x = 0; x < Width; x++ pSumOfSquares++, pConv++) {
        } // for (int y = 0; y < Height; y++) {
    } // for (i = 0; i < nFrames; i++) {
    // now apply weber's law to each pixel in the convolution image
    for (i = 0; i < nFrames; i++) {
        pSumOfSquares = SumOfSquaresImg.pBuffer();
        pConv = (ConvVec.GetImg(i))->pBuffer();
        for (y = 0; y < Height; y++) {
            for (x = 0; x < Width; x++, pSumOfSquares++, pConv++) {
                const float L2norm = sqrt(*pSumOfSquares);
                const float Factor = (float)log(1.0f+L2norm/WeberConst)/
                    (L2norm+CONST_EPSILON);
                *pConv *= Factor;
            } // for (x = 0; x < Width; x++ pSumOfSquares++, pConv++) {
        } // for (y = 0; y < Height; y++) {
    } // for (i = 0; i < nFrames; i++) {
}
