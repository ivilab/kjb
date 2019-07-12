/////////////////////////////////////////////////////////////////////////////
// gausskernel.h - generate 2D rotated Gaussian (derivative) kernels
// Author: Doron Tal
// Date created: March, 2000

#ifndef _GAUSSKERNEL_H
#define _GAUSSKERNEL_H

#include "wrap_dtlib_cpp/img.h"

namespace DTLib {

    // derived from a float image class, which here represents a
    // convolution kernel, this class instantiates into 2-D Gaussian
    // (derivative) kernels, this has mainly been used for drawing
    // elongated convolution kernels used in a filterbank.  For
    // isotropic mexican-hat type kernels, see dog.h(.cpp).
    class CGaussKernel : public CImg<float>
    {
    public:
        // initialize drawing the kernel
        // 'SigmaX' - sigma in horizontal direction
        // 'SigmaY' - sigma in vertical direction
        // 'Size' - size of the image
        // 'OrderY' - 0 = Gaussian, 1 = 1st Gauss. deriv, 2 = 2nd deriv. etc,
        // (NOTE: 'OrderX' is always 0)
        // 'bHilbert' - if true, return Hilbert transform of kernel
        CGaussKernel(const int& Size, const float& SigmaX, const float& SigmaY,
                     const int& OrderY = 0, const float& Theta = 0.0f,
                     const bool& bHilbert = false);

        ///////////////////////////////////////////////////////////////////////
        ///// END OF PRIVATE PART - PROGRAMMERS NEED NOT LOOK BEYOND HERE /////
        ///////////////////////////////////////////////////////////////////////

    protected:

        // null constructor - initialize without drawing the kernel,
        // but do allocate the image
        CGaussKernel(const int& Size);

        // draw the kernel with rotation 'Theta', given new
        // parameters, in an already allocated kernel
        void Draw(const float& SigmaX, const float& SigmaY, const int& OrderY,
                   const float& Theta, const bool& bHilbert);

        // draw kernel without rotating
        void DrawNoRot(const float& SigmaX, const float& SigmaY,
                       const int& OrderY, const bool& bHilbert);

        float m_SigmaX;
        float m_SigmaY;
        int m_OrderY;
    };

} // namespace DTLib {

#endif /* #ifndef _GAUSSKERNEL_H */
