/////////////////////////////////////////////////////////////////////////////
// subpix_localize.cpp - sub-pixel localization of edges
// Author: Doron Tal
// Date created: April, 2000

#include "l/l_sys_debug.h"  /* For ASSERT. */
#include "wrap_dtlib_cpp/subpix_localize.h"
#include "wrap_dtlib_cpp/orientation_energy.h"
#include "wrap_dtlib_cpp/nr.h"

using namespace DTLib;

/////////////////////////////////////////////////////////////////////////////

void DTLib::SubpixelLocalization(const int& nGaussScales,
                                 const int& nGaussOrientations,
                                 CImgVec<float>& OEVec,
                                 CImgVec<BYTE>& ThetaIdxVec,
                                 CImgVec<BYTE>& MaxVec,
                                 CImgVec<float>& RhoStarVec,
                                 CImgVec<float>& ThetaStarVec,
                                 CImgVec<float>& XLocVec,
                                 CImgVec<float>& YLocVec,
                                 CImgVec<float>& ErrVec)
{
    const int Width = OEVec.Width(), Height = OEVec.Height();

    ByteImgVecIter ppMaxImg = MaxVec.itImgs();
    FloatImgVecIter ppRhoStarImg = RhoStarVec.itImgs();
    FloatImgVecIter ppThetaStarImg = ThetaStarVec.itImgs();
    ByteImgVecIter ppThetaIdxImg = ThetaIdxVec.itImgs();
    FloatImgVecIter ppXLocImg = XLocVec.itImgs();
    FloatImgVecIter ppYLocImg = YLocVec.itImgs();
    FloatImgVecIter ppErrImg = ErrVec.itImgs();

    // initialize vectors & matrices
    float* y_vector = fvector(1, 9);
    float* d_vector = fvector(1, 9);
    float* dsqr_vector = fvector(1, 9);
    float** a_matrix = matrix(1, 9, 1, 3);
    float** lsqr_matrix = matrix(1, 3, 1, 9);
    float* res_vector = fvector(1, 3);
    float* est_vector = fvector(1, 9);

    for (int iScale = 0; iScale < nGaussScales; iScale++, ppMaxImg++,
             ppRhoStarImg++, ppThetaStarImg++, ppThetaIdxImg++, ppXLocImg++,
             ppYLocImg++, ppErrImg++) {

        const int OEScaleOffset = iScale*nGaussOrientations;

        BYTE* pMax = (*ppMaxImg)->pBuffer();
        float* pRhoStar = (*ppRhoStarImg)->pBuffer();
        float* pThetaStar = (*ppThetaStarImg)->pBuffer();
        BYTE* pThetaIdx = (*ppThetaIdxImg)->pBuffer();
        float* pXLoc = (*ppXLocImg)->pBuffer();
        float* pYLoc = (*ppYLocImg)->pBuffer();
        float* pErr = (*ppErrImg)->pBuffer();

        int x, y;
        float MaxRhoStar = CONST_MIN_FLOAT;
        for (y = 0; y < Height; y++) {
            const int OEYPixelOffset = y*Width;
            for (x = 0; x < Width; x++, pMax++, pRhoStar++, pThetaStar++,
                     pThetaIdx++, pXLoc++, pYLoc++, pErr++) {
                /*
                if ((x == 0) || (x == Width-1) ||
                    (y == 0) || (y == Height-1)) {
                    *pMax = (BYTE)255;                    
                    *pXLoc = 0.0f;
                    *pYLoc = 0.0f;
                    *pErr = 0.0f;
                }

                else {
                    // ((y > 0) && (x > 0) && (y < Height-1) && (x < Width-1))

                */
                if ((y > 0) && (x > 0) && (y < Height-1) && (x < Width-1)) {
                    const int OEPixelOffset = OEYPixelOffset+x;
                    const float ThetaStar = *pThetaStar;

                    float FixedThetaStar = ThetaStar; 
                    FixThetaRange(FixedThetaStar);

                    const int  iTheta = (int)*pThetaIdx;

                    float PrevTheta, Theta, NextTheta, PrevOE, OE, NextOE;
                    int iPrevTheta, iNextTheta;
                    GetSuccessiveOEThetaPairs(OEVec, nGaussOrientations,
                                              iScale, iTheta, x, y, 
                                              iPrevTheta, iNextTheta, 
                                              PrevTheta, Theta, NextTheta, 
                                              PrevOE, OE, NextOE);

                    ASSERT(ThetaStar >= PrevTheta);
                    ASSERT(ThetaStar <= NextTheta);

                    const float CosThetaStar = (float)cos(FixedThetaStar);
                    const float SinThetaStar = (float)sin(FixedThetaStar);
                    float sumNumer = 0.0f, sumDenom = 0.0f;
                    const float Beta = 2.0f*SinThetaStar*SinThetaStar*
                        CosThetaStar*CosThetaStar+1.0f;
                    const float Beta2 = 2.0f*Beta;
                    const float Beta3 = 3.0f*Beta;
                    const float det_recip =
                        1.0f/(18.0f*Beta-12.0f+CONST_EPSILON);
                    int i = 1; // i counts sample points
                    float ahatsum = 0.0f;
                    for (int yy = -1, w = -Width; yy <= 1; yy++, w += Width) {
                        const int PixYoffset = OEPixelOffset+w;
                        for (int xx = -1; xx <= 1; xx++, i++) {
                            const int PixOffset = PixYoffset+xx;
                            const float di = 
                                (float)yy*CosThetaStar-(float)xx*SinThetaStar;
                            const float disqr = di*di;

                            int iOEImg; 

                            iOEImg = OEScaleOffset+iTheta;
                            const float OE =
                                *(OEVec.pBuffer(iOEImg)+PixOffset);
                            // 
                            iOEImg = OEScaleOffset+iPrevTheta;
                            const float PrevOE = 
                                *(OEVec.pBuffer(iOEImg)+PixOffset);
                            // 
                            iOEImg = OEScaleOffset+iNextTheta;
                            const float NextOE =
                                *(OEVec.pBuffer(iOEImg)+PixOffset);

                            // NB: assertion below isn't always true!
                            // ASSERT((OE >= PrevOE) && (OE >= NextOE));

                            float a = 0, b = 0, c = 0;
                            ParabolicInterpolation(-1.0f, PrevOE, 0.0f, OE,
                                                   1.0f, NextOE, a, b, c);
              
                            const float x = (ThetaStar-PrevTheta)/
                                (NextTheta-PrevTheta)-1.0f;
              
                            const float yi = a*SQR(x)+b*x+c;
              
                            if ((xx == 0) && (yy == 0)) {
                                ASSERT(yi >= 0.0f);
                                *pRhoStar = yi;

                                if (*pRhoStar > MaxRhoStar)
                                    MaxRhoStar = *pRhoStar;

                            }
              
                            d_vector[i] = di;
                            dsqr_vector[i] = disqr;
                            y_vector[i] = yi;
            
                            a_matrix[i][1] = 1;
                            a_matrix[i][2] = di;
                            a_matrix[i][3] = disqr;
                            
                            ahatsum += yi*(3.0f*disqr-2.0f);

                            lsqr_matrix[1][i] = (3.0f*disqr-2.0f)*det_recip;
                            lsqr_matrix[2][i] = ((Beta3-2.0f)*di)*det_recip;
                            lsqr_matrix[3][i] = (-2.0f*disqr+Beta2)*det_recip;
            
                            sumNumer += di*yi;
                            sumDenom += (3.0f*disqr-2.0f)*yi;
                        } // for xx
                    } // for yy

                    ahatsum *= det_recip;

                    const float delta = -0.5f*(3.0f*Beta-2.0f)*sumNumer/
                        (sumDenom+CONST_EPSILON);

                    mat_vec_prod(lsqr_matrix, y_vector, res_vector,1, 3, 1, 9);
                    mat_vec_prod(a_matrix, res_vector, est_vector, 1, 9, 1, 3);
        
                    float est_vec_ss = 0.0f;
                    float y_vec_ss = 0.0f;
                    for(i  = 1; i <= 9; i++) {
                        const float ei = est_vector[i];
                        const float yi = y_vector[i];
                        est_vec_ss += SQR(ei);
                        y_vec_ss += SQR(yi);
                    }

                    // NB: NormErr below is a correction over Perona&Malik'90,
                    // which was: 1.0f-sqrt(est_vec_ss)/sqrt(y_vec_ss). -DT
                    // const float EstNorm = sqrt(est_vec_ss);
                    // const float ObsNorm = sqrt(y_vec_ss);
                    // const float NormErr = fabs(ObsNorm-EstNorm)/ObsNorm;

                    const float NormErr = 1.0f-sqrt(est_vec_ss)/sqrt(y_vec_ss);

                    *pErr = NormErr;
                    // *pErr = ahatsum; // for display..

                    *pXLoc = -SinThetaStar*delta;
                    *pYLoc = CosThetaStar*delta;
                    /*ASSERT(!((*pRhoStar != 0.0f) && (*pXLoc == 0.0f) &&
                             (*pYLoc == 0.0f)));*/
                    if(((*pRhoStar != 0.0f) && (*pXLoc == 0.0f) &&
                                                 (*pYLoc == 0.0f)))
                    {
                    	*pRhoStar = 0.0;
                    }
                    ASSERT(!((*pRhoStar != 0.0f) && (*pXLoc == 0.0f) &&
                                                 (*pYLoc == 0.0f)));

                    // rho threshold to get rid of noise Rho

                    if (// (fabs(delta) <= 0.71) &&
                        (fabs(*pXLoc) <= 0.51f) &&
                        (fabs(*pYLoc) <= 0.51f) &&
                        (*pRhoStar > 0.00000001f) &&
                        (ahatsum < 0.0f)) {
                        *pMax = (BYTE)255;
                    }

                    else {
                        *pXLoc = 0.0f;
                        *pYLoc = 0.0f;
                        *pMax = (BYTE)0;
                        *pThetaStar = 0.0f; // just for display purposes
                    } // if (fabs(delta) <= 0.5f) {
                } // if 1 pixel in bounds

                else { // not 1 pixel in bounds
                    *pXLoc = 0.0f;
                    *pYLoc = 0.0f;
                    *pErr = 0.0f;
                    *pMax = (BYTE)0;
                    *pThetaStar = 0.0f; // just for display purposes
                } // else

            } // for (x ..
        } // for (y .. 

        pMax = (*ppMaxImg)->pBuffer();
        pRhoStar = (*ppRhoStarImg)->pBuffer();
        // const float RhoFrac = 0.005f;
        const float RhoFrac = 0.001f; // 0.001 --> tenth of a percent thresh
        const float RhoThresh = MaxRhoStar*RhoFrac;
        for (y = 0; y < Height; y++) {
            for (x = 0; x < Width; x++, pMax++, pRhoStar++) {
                if ((*pMax != 0) && (*pRhoStar <= RhoThresh))
                    *pMax = 0;
            } // for x
        } // for y

    } // for (iScale .. 
    free_fvector(y_vector, 1, 9);
    free_fvector(d_vector, 1, 9);
    free_fvector(dsqr_vector, 1, 9);
    free_matrix(a_matrix, 1, 9, 1, 3);
    free_matrix(lsqr_matrix, 1, 3, 1, 9);
    free_fvector(res_vector, 1, 3);
    free_fvector(est_vector, 1, 9);
}
