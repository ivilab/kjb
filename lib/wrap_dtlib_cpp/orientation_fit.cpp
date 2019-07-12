/////////////////////////////////////////////////////////////////////////////
// orientation_fit.cpp - orientations parabolic fit & interpolation
// Author: Doron Tal
// Date Created: April, 2000

#include "l/l_sys_debug.h"  /* For ASSERT. */
#include "wrap_dtlib_cpp/orientation_fit.h"
#include "wrap_dtlib_cpp/orientation_energy.h"

using namespace DTLib;

/////////////////////////////////////////////////////////////////////////////

// amplitude is not interpolated, just MAX over orientations
void DTLib::ParabolicOrientationsFit(const int& nGaussScales,
                                     const int& nGaussOrientations,
                                     CImgVec<float>& OEVec,
                                     CImgVec<float>& RhoVec,
                                     CImgVec<float>& ThetaVec,
                                     CImgVec<BYTE>& ThetaIdxVec)
{
    ASSERT(OEVec.FrameSize() > 0);
    ASSERT(OEVec.nFrames() > 0);

    const int Width = OEVec.Width(), Height = OEVec.Height();
    // NB: line below assumes we'll never use more than 256 orientations,
    // but who would *ever* want to do that... NEVER!
    FloatImgVecIter ppOEImg = OEVec.itImgs();
    FloatImgVecIter ppRhoImg = RhoVec.itImgs();
    FloatImgVecIter ppThetaImg = ThetaVec.itImgs();
    ByteImgVecIter ppThetaIdxImg = ThetaIdxVec.itImgs();
    for (int iScale = 0; iScale < nGaussScales;
         iScale++, ppRhoImg++, ppThetaImg++, ppThetaIdxImg++) {
        (*ppRhoImg)->Zero();
        for (int iTheta = 0; iTheta < nGaussOrientations;iTheta++,ppOEImg++){
            float* pOE = (*ppOEImg)->pBuffer();
            float* pRho = (*ppRhoImg)->pBuffer();
            BYTE* pThetaIdx = (*ppThetaIdxImg)->pBuffer();
            for (int y = 0; y < Height; y++) {
                for (int x = 0; x < Width; x++, pOE++, pRho++, pThetaIdx++) {
                    const float OE = *pOE;
                    if (*pRho <= OE) {
                        *pRho = OE;
                        *pThetaIdx = (BYTE)iTheta;
                    } // if (*pRho < OE) {
                } // for (int x
            } // for (int y
        } // for (iTheta
        // now do orientation fitting, using the theta index, fixing theta
        // rho is already all set
        InterpolateOrientations(iScale, nGaussOrientations, OEVec, 
                                ThetaIdxVec, ThetaVec);
    }
}

/////////////////////////////////////////////////////////////////////////////

void DTLib::InterpolateOrientations(const int& iScale, 
                                    const int& nGaussOrientations,
                                    CImgVec<float>& OEVec,
                                    CImgVec<BYTE>& ThetaIdxVec,
                                    CImgVec<float>& ThetaVec)
{
    const int Width = OEVec.Width(), Height = OEVec.Height();
    float* pTheta = ThetaVec.pBuffer(iScale);
    BYTE* pThetaIdx = ThetaIdxVec.pBuffer(iScale);
    for (int y = 0; y < Height; y++) {
        for (int x = 0; x < Width; x++, pThetaIdx++, pTheta++) {
            float PrevTheta, Theta, NextTheta, PrevOE, OE, NextOE;
            int iPrevTheta, iNextTheta;
            GetSuccessiveOEThetaPairs(OEVec, nGaussOrientations, iScale, 
                                      (int)*pThetaIdx, x, y, 
                                      iPrevTheta, iNextTheta, PrevTheta, Theta,
                                      NextTheta, PrevOE, OE, NextOE);
            if ((PrevOE != 0.0f) || (OE != 0.0f) || (NextOE != 0.0f)) {
                float InterpolatedTheta = 
                    ParabolicInverseInterpolation(PrevTheta, PrevOE, Theta, OE,
                                                  NextTheta, NextOE);
                ASSERT(InterpolatedTheta >= PrevTheta);
                ASSERT(InterpolatedTheta <= NextTheta);

                *pTheta = InterpolatedTheta; // the punchline..
            } // if .. 
            else *pTheta = Theta; // no interpolation if all OE vals were 0
        } // for x
    } // for y
}
