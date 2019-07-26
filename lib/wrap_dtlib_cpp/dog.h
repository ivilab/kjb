/////////////////////////////////////////////////////////////////////////////
// dog.h - generate 2D difference of Gaussian kernels
// Author: Doron Tal
// Date created: April, 2000

#ifndef _DOG_H
#define _DOG_H

#include "wrap_dtlib_cpp/gausskernel.h"

namespace DTLib {

    // From an isotropic Gaussian of variance sigma, subtract *two*
    // other isotropic Gaussians, usually, of larger variance each.
    // derived from a Gaussian Kernel class which in turn is derived
    // from Img class.  Constructor produces a DOG image that can be
    // used like a CImg<float>

    class CDOG : public CGaussKernel
    {
    public:
        // PRECOND: 'Size' must be odd.
        // POSTCOND: becomes a float CImg with the kernel in it
        CDOG(const int& Size, const float& ExcitSigma,
             const float& InhibSigma1, const float& InhibSigma2);
    }; // class CDOG

} // namespace DTLib {

#endif /* #ifndef _DOG_H */
