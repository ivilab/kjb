/////////////////////////////////////////////////////////////////////////////
// orientation_energy.cpp - compute orientation energy
// Author: Doron Tal
// Date Created: March, 2000

#include "l/l_sys_debug.h"  /* For ASSERT. */
#include "wrap_dtlib_cpp/orientation_energy.h"
#include "wrap_dtlib_cpp/img.h"
#include "wrap_dtlib_cpp/utils.h"

////////////////////////////////////////
//Added by Prasad

//This is to test the hypothesis of using color in contour
//Add Orientation energies obtained in color planes
//#define SUM_COLOR_OE

///////////////////////////////////////

////////////////////////////////////////
//Added by Prasad

//This is to test the hypothesis of using color in contour
//Choose the maximum of orientation energies computed in texture and color planes
//#define MAX_COLOR_TEXTURE_OE

///////////////////////////////////////


using namespace DTLib;

/////////////////////////////////////////////////////////////////////////////

void DTLib::OrientationEnergy(const int& nGaussScales,
                              const int& nGaussOrientations,
                              CImgVec<float>& ConvVec,
////////////////////////////////////////
//Added by Prasad

//This is to test the hypothesis of using color in contour

#if defined(SUM_COLOR_OE) || defined(MAX_COLOR_TEXTURE_OE)

                              CImgVec<float>& AConvVec,
                              CImgVec<float>& BConvVec,

#endif
///////////////////////////////////////

                              CImgVec<float>& OEVec)
{
    const int Width = ConvVec.Width(), Height = ConvVec.Height();

    ASSERT(OEVec.Width() == Width);
    ASSERT(OEVec.Height() == Height);
////////////////////////////////////////
//Added by Prasad

//This is to test the hypothesis of using color in contour

#ifdef SUM_COLOR_OE
   cout << "Using Sum of the orientation energies in the brightness and color planes" << endl;
#endif

#ifdef MAX_COLOR_TEXTURE_OE
   cout << "Using Max of the orientation energies among the brightness and color planes" << endl;
#endif

#if defined(SUM_COLOR_OE) || defined(MAX_COLOR_TEXTURE_OE)

   ASSERT(AConvVec.Width() == Width);
   ASSERT(AConvVec.Height() == Height);

   ASSERT(BConvVec.Width() == Width);
   ASSERT(BConvVec.Height() == Height);

   ASSERT(AConvVec.nFrames() >= OEVec.nFrames()*2);
   ASSERT(BConvVec.nFrames() >= OEVec.nFrames()*2);

   ASSERT(AConvVec.FrameSize() > 0);
   ASSERT(BConvVec.FrameSize() > 0);

#endif
///////////////////////////////////////

    ASSERT(ConvVec.nFrames() >= OEVec.nFrames()*2);
    ASSERT(OEVec.nFrames() == nGaussOrientations*nGaussScales);

    ASSERT(ConvVec.FrameSize() > 0);
    ASSERT(OEVec.FrameSize() > 0);

    FloatImgVecIter ppConvImg = ConvVec.itImgs();

////////////////////////////////////////
//Added by Prasad

//This is to test the hypothesis of using color in contour

#if defined(SUM_COLOR_OE) || defined(MAX_COLOR_TEXTURE_OE)

    FloatImgVecIter ppAConvImg = AConvVec.itImgs();
    FloatImgVecIter ppBConvImg = BConvVec.itImgs();

#endif
///////////////////////////////////////

    FloatImgVecIter ppOEImg = OEVec.itImgs();
    for (int iScale = 0; iScale < nGaussScales; iScale++) {
        for (int iTheta = 0; iTheta < nGaussOrientations; iTheta++) {
            float* pEven = (*ppConvImg)->pBuffer(); ppConvImg++;
            float* pOdd = (*ppConvImg)->pBuffer(); ppConvImg++;

////////////////////////////////////////
//Added by Prasad

//This is to test the hypothesis of using color in contour

#if defined(SUM_COLOR_OE) || defined(MAX_COLOR_TEXTURE_OE)

            float* pAEven = (*ppAConvImg)->pBuffer(); ppAConvImg++;
            float* pAOdd = (*ppAConvImg)->pBuffer(); ppAConvImg++;

            float* pBEven = (*ppBConvImg)->pBuffer(); ppBConvImg++;
            float* pBOdd = (*ppBConvImg)->pBuffer(); ppBConvImg++;

#endif
///////////////////////////////////////

            FloatCImgPtr pOEImg = *ppOEImg;
            float* pOE = pOEImg->pBuffer(); ppOEImg++;
            for (int y = 0; y < Height; y++) {
                for (int x = 0; x < Width; x++, pEven++, pOdd++,

////////////////////////////////////////
//Added by Prasad

//This is to test the hypothesis of using color in contour

#if defined(SUM_COLOR_OE) || defined(MAX_COLOR_TEXTURE_OE)

                                                pAEven++, pAOdd++,
                                                pBEven++, pBOdd++,

#endif
///////////////////////////////////////

                                                   pOE++) {
                    const float Even = *pEven;
                    const float Odd = *pOdd;

////////////////////////////////////////
//Added by Prasad

//This is to test the hypothesis of using color in contour

#if defined(SUM_COLOR_OE) || defined(MAX_COLOR_TEXTURE_OE)

                    const float AEven = *pAEven;
                    const float AOdd = *pAOdd;

                    ASSERT(IS_A_FLOAT(AEven));
                    ASSERT(IS_A_FLOAT(AOdd));

                    const float BEven = *pBEven;
                    const float BOdd = *pBOdd;

                    ASSERT(IS_A_FLOAT(BEven));
                    ASSERT(IS_A_FLOAT(BOdd));

#endif
///////////////////////////////////////

                    ASSERT(IS_A_FLOAT(Even));
                    ASSERT(IS_A_FLOAT(Odd));

////////////////////////////////////////
//Added by Prasad

//This is to test the hypothesis of using color in contour

//Add Orientation energies obtained in color planes
#if defined(SUM_COLOR_OE)

                    const float SS = SQR(Even)+SQR(Odd)+SQR(AEven)+SQR(AOdd)+SQR(BEven)+SQR(BOdd);

//Choose the maximum of orientation energies computed in texture and color planes
#elif defined(MAX_COLOR_TEXTURE_OE)

                          float SS = 0.0f;
                    const float SSL = SQR(Even)+SQR(Odd);
                    const float SSA = SQR(AEven)+SQR(AOdd);
                    const float SSB = SQR(BEven)+SQR(BOdd);

                    ASSERT(IS_A_FLOAT(SSL));
                    ASSERT(IS_A_FLOAT(SSA));
                    ASSERT(IS_A_FLOAT(SSB));

                    if(SSL > SS) SS = SSL;
                    if(SSA > SS) SS = SSA;
                    if(SSB > SS) SS = SSB;

#else

                    const float SS = SQR(Even)+SQR(Odd);

#endif
///////////////////////////////////////

                    ASSERT(IS_A_FLOAT(SS));
                    *pOE = SS;
                } // for (x..
            } // for (y..
        } // for (iTheta..
    } // for (iScale..
}

/////////////////////////////////////////////////////////////////////////////

void DTLib::GetSuccessiveOEThetaPairs(CImgVec<float>& OEVec,
                                      const int& nGaussOrientations,
                                      const int& iScale,
                                      const int& iTheta,
                                      const int&x, const int& y,
                                      int& iPrevTheta, int& iNextTheta,
                                      float& PrevTheta,
                                      float& Theta,
                                      float& NextTheta,
                                      float& PrevOE,
                                      float& OE,
                                      float& NextOE)
{
    iPrevTheta = iTheta-1; // could be out of range here..
    iNextTheta = iTheta+1;

    const float N = (float)nGaussOrientations;

    PrevTheta = CONST_PI*(float)iPrevTheta/N;
    Theta = CONST_PI*(float)iTheta/N;
    NextTheta = CONST_PI*(float)iNextTheta/N;

    // fix for phase wrapping
    if (iPrevTheta < 0) { // decremented too much
        iPrevTheta += nGaussOrientations;
        PrevTheta += CONST_PI;
        Theta += CONST_PI;
        NextTheta += CONST_PI;
    }
    if (iNextTheta == nGaussOrientations) // incremented too much
        iNextTheta = 0;

    ASSERT((Theta > PrevTheta) && (NextTheta > Theta));

    ASSERT(iPrevTheta >= 0);
    ASSERT(iTheta >= 0);
    ASSERT(iNextTheta >= 0);

    const int iOEScale = iScale*nGaussOrientations;

    const int iPrevThetaOE = iPrevTheta+iOEScale;
    const int iThetaOE = iTheta+iOEScale;
    const int iNextThetaOE = iNextTheta+iOEScale;

    ASSERT(iPrevThetaOE < OEVec.nFrames());
    ASSERT(iThetaOE < OEVec.nFrames());
    ASSERT(iNextThetaOE < OEVec.nFrames());

    const int PixOffset = y*OEVec.Width()+x;

    PrevOE = *(OEVec.pBuffer(iPrevThetaOE)+PixOffset);
    OE = *(OEVec.pBuffer(iThetaOE)+PixOffset);
    NextOE = *(OEVec.pBuffer(iNextThetaOE)+PixOffset);

    ASSERT(PrevOE >= 0.0f);
    ASSERT(OE >= 0.0f);
    ASSERT(NextOE >= 0.0f);

    ASSERT(IS_A_FLOAT(PrevOE));
    ASSERT(IS_A_FLOAT(OE));
    ASSERT(IS_A_FLOAT(NextOE));

    ASSERT((OE >= PrevOE) && (OE >= NextOE));
}
