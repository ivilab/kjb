/////////////////////////////////////////////////////////////////////////////
// filterbank.h - filterbank convolution class
// Author: Doron Tal
// Date created: April, 2000

#ifndef _FILTERBANK_H
#define _FILTERBANK_H

#include "wrap_dtlib_cpp/img.h"
#include "wrap_dtlib_cpp/gausskernel.h"

// 'KERNEL_SIDE_FACTOR' is how much we multiply the largest sigma of our
// filterbank gaussians by in order to get the Kernel side.  FWHM is 2.35
// sigma, so use 3.0 to be safe (but 3.0 is multiplied by sqrt(2) to get
// diagonal cases right..)

#define KERNEL_SIDE_FACTOR 4.2426f

namespace DTLib {

    // filterbank class 'CFilterbank':
    // after constructing it and setting it up with Setup(..), it's
    // ready to perform convolutions using Convolve().
    // Separating the setup stage from convolution stage allows us
    // to perform repetitive convolutions without the setup overhead.

    class CFilterbank
    {
    public:
        CFilterbank();

        ~CFilterbank();

        // returns the number of kernels
        inline int nKernels() { return m_nKernels; }

        // returns a pointer to a vector of pointers to float images,
        // which are the kernels ***TODO***: change this to use a
        // CImgVec object instead.
        inline vector<CImg<float>*>* pvecKernels() { return &m_vecKernels; }

        ///////////////////////////////////////////////////////////////////////
        // sets up the kernels

        bool Setup(const int nGaussScales,
                   const int nGaussOrientations,
                   const float GaussSigmaY,
                   const float GaussX2YSigmaRatio,
                   const int nDOGScales,
                   const float DOGExcitSigma,
                   const float DOGInhibSigmaRatio1,
                   const float DOGInhibSigmaRatio2);

        ///////////////////////////////////////////////////////////////////////
        // convolutions with input image 'InImg'.  The results are returned
        // in the image sequence 'OutSeq', where elongated kernels come
        // first (outer loop over scales, inner loop over orientations,
        // inner loop over phase, so there are
        // m_nGaussScales*m_nGaussOrientations*2 frames for the oriented
        // Gaussian convolutions, plus m_nDOGScales frames for the isotropic
        // DOG convolutions.  scale and orientations are always increasing
        // in the loops above.
        void Convolve(CImg<float> &InImg, CImgVec<float> &OutConvVec);

        ///////////////////////////////////////////////////////////////////////
        // END OF INTERFACE
        ///////////////////////////////////////////////////////////////////////

    protected:
        int m_nKernels;

        vector<CImg<float>*> m_vecKernels;

        // input parameters:
        int m_nGaussScales;
        int m_nGaussOrientations;
        float m_GaussSigmaY;
        float m_GaussX2YSigmaRatio;
        int m_nDOGScales;
        float m_DOGExcitSigma;
        float m_DOGInhibSigmaRatio1;
        float m_DOGInhibSigmaRatio2;

    private:
        void FreeMemory();

    };

} // namespace DTLib {

#endif /* #ifndef _FILTERBANK_H */
