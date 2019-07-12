/////////////////////////////////////////////////////////////////////////////
// convolve.cpp - image convolution class
// Author: Doron Tal
// Date created: April, 2000

#include "l/l_sys_debug.h"  /* For ASSERT. */
#include "wrap_dtlib_cpp/convolve.h"
#include "wrap_dtlib_cpp/img.h"

using namespace DTLib;

namespace {

/* The following function tests whether the given image Img has had its ROI set
 * up with its top, bottom, left and right edges far enough away from the image
 * edges, so that you can safely convolve the image with a kernel of the given
 * width and height.  If the margins are too small, Convolve would (except for
 * this check) index the image outside of its legitimate boundaries, possibly
 * causing a segfault or garbage values in the output.
 * (Andrew Predoehl, 21 Oct 2009)
 */
bool ROI_margins_are_ok(
    CImg<float>& Img,
    int KernelWidth,
    int KernelHeight )
{
    ASSERT( 0 < KernelWidth && 0 < KernelHeight );

    /* The following four constants describe the kernel image dimensions
     * relative to a roughly-centered "origin" pixel.  The centering is rough
     * since the width or height might be an even number; in that case the
     * origin pixel is below or to the right of center (both in this routine
     * and in Convolve() below).
     */
    const int KerRowsAboveOrigin = KernelHeight/2;
    const int KerRowsBelowOrigin = KernelHeight - KerRowsAboveOrigin - 1;
    const int KerColsLeftOfOrigin = KernelWidth/2;
    const int KerColsRightOfOrigin = KernelWidth - KerColsLeftOfOrigin - 1;

    return      Img.ROIStartY() >= KerRowsAboveOrigin
            &&  Img.Height() - Img.ROIEndY() >= KerRowsBelowOrigin
            &&  Img.ROIStartX() >= KerColsLeftOfOrigin
            &&  Img.Width() - Img.ROIEndX() >= KerColsRightOfOrigin;
}

}

////////////////////////////////////////////////////////////

void DTLib::Convolve(CImg<float>& InImg, 
                     CImg<float>& KernelImg,
                     CImg<float>& OutImg)
{
    /* This assertion was added on 21 Oct 2009 by AP. */
    ASSERT( ROI_margins_are_ok(InImg, KernelImg.Width(), KernelImg.Height()) );

    register int InWidth = InImg.Width();

    /* Modified on 21 Oct 2009 by AP.
     * The old loop bounds for the kx, ky loops were incorrect when the kernel
     * had even width or height.  The limits here will work for any parity.
     */
    const int KernelXBegin = -KernelImg.Width()/2;
    const int KernelXEnd = KernelXBegin + KernelImg.Width();
    const int KernelYBegin = -KernelImg.Height()/2;
    const int KernelYEnd = KernelYBegin + KernelImg.Height();

    const int StartX = InImg.ROIStartX();
    const int StartY = InImg.ROIStartY();
    const int EndX = InImg.ROIEndX();
    const int EndY = InImg.ROIEndY();
    const int InSkip = InImg.ROISkipCols();
    const int OutSkip = OutImg.ROISkipCols();
    float* pInROI = InImg.pROI();
    float* pOutROI = OutImg.pBuffer();
    float* pKernelImg = KernelImg.pBuffer();
    register int InitialYOffset = KernelYBegin * InWidth;
    for (int y = StartY; y < EndY; y++, pInROI += InSkip, pOutROI += OutSkip) {
        for (int x = StartX; x < EndX; x++, pInROI++, pOutROI++) {
            register float* pKernel = pKernelImg;
            register float accumulator = 0.0f;
            register int YOffset = InitialYOffset;
            for (register int ky = KernelYBegin;
                 ky < KernelYEnd; ky++, YOffset += InWidth){
                register float* pInYOffset = pInROI+YOffset;
                for (register int kx = KernelXBegin;
                     kx < KernelXEnd; kx++, pKernel++) {
                    accumulator += (*pKernel)*(*(pInYOffset+kx));
                } // kx
            } // ky
            *pOutROI = accumulator;
        } // x
    } // y
} // void Convolve( ..

