/////////////////////////////////////////////////////////////////////////////
// dual_lattice.cpp - building the dual lattice for later building sparse mat
// Author: Doron Tal
// Date Created: May, 2000

#include "wrap_dtlib_cpp/dual_lattice.h"
#include "wrap_dtlib_cpp/line.h"

//////////////////////////////////////////////////////
//Added by Prasad
//This is to test the hypothesis of disabling texture gating
//to see if there are problems caused by this when color
//is used both in textons and contour
//#define DISABLE_TEXTURE_GATING

/////////////////////////////////////////////////////

using namespace DTLib;

/////////////////////////////////////////////////////////////////////////////

void DTLib::MakeDualLattice(const float& EdgelLength,
                            const float& SigmaIC,
                            const int& nGaussScales,
                            CImgVec<BYTE>& InNMSVec,
                            CImgVec<float>& InEnergyVec,
                            CImgVec<float>& InOriVec,
                            CImgVec<float>& InXLocVec,
                            CImgVec<float>& InYLocVec,
                            CImg<float>& InPTextureImg,
                            CImg<float>& OutDualHImg,
                            CImg<float>& OutDualVImg)
{
    const int Width = InEnergyVec.Width(), Height = InEnergyVec.Height();
    const float MinusOneOverSigmaIC = -1.0f/SigmaIC;

    OutDualHImg.SetROIVal(0.0f);
    OutDualVImg.SetROIVal(0.0f);

    ByteImgVecIter ppMaxImg = InNMSVec.itImgs();
    FloatImgVecIter ppRhoImg = InEnergyVec.itImgs();
    FloatImgVecIter ppThetaImg = InOriVec.itImgs();
    FloatImgVecIter ppXLocImg = InXLocVec.itImgs();
    FloatImgVecIter ppYLocImg = InYLocVec.itImgs();

//////////////////////////////////////////////////////
//Added by Prasad
//This is to test the hypothesis of disabling texture gating
//to see if there are problems caused by this when color
//is used both in textons and contour

#ifdef DISABLE_TEXTURE_GATING
                    cout << "Texture gating operation is disabled, i.e., P_B = P_con" << endl;
#endif
//////////////////////////////////////////////////////

    for (int iScale = 0; iScale < nGaussScales; iScale++,
             ppMaxImg++, ppRhoImg++, ppThetaImg++, ppXLocImg++, ppYLocImg++) {
        BYTE* pMax = (*ppMaxImg)->pBuffer();
        float* pRho = (*ppRhoImg)->pBuffer();
        float* pTheta = (*ppThetaImg)->pBuffer();
        float* pXLoc = (*ppXLocImg)->pBuffer();
        float* pYLoc = (*ppYLocImg)->pBuffer();
        float* pPTexture = InPTextureImg.pBuffer();
        const float HalfEdgelLength = EdgelLength*0.5f;
        for (int y = 0; y < Height; y++) {
            for (int x = 0; x < Width; x++,
                     pMax++, pRho++,pTheta++,pXLoc++,pYLoc++,pPTexture++) {
                if (*pMax != 0.0f) {
                    const float Theta = *pTheta;
                    const float CosThetaE2 = cos(Theta)*HalfEdgelLength;
                    const float SinThetaE2 = sin(Theta)*HalfEdgelLength;
                    const float xloc = *pXLoc, yloc = *pYLoc;

                    // determine the two coordinates of the edgelet ends
                    const float x1 = xloc+CosThetaE2;
                    const float y1 = yloc+SinThetaE2;

                    const float x2 = xloc-CosThetaE2;
                    const float y2 = yloc-SinThetaE2;

                    const float SqrtOE = (float)sqrt((double)*pRho);
                    const float P_con =
                        1.0f-(float)exp(SqrtOE*MinusOneOverSigmaIC);
                    const float P_texture = *pPTexture;

/////////////////////////////////////////////////
//Added by Prasad

#ifdef DISABLE_TEXTURE_GATING
                    const float P_B = P_con;
#else
                    const float P_B = (1.0f-P_texture)*P_con;
#endif
/////////////////////////////////////////////////

                    // ***TODO***: turn into a function most of what you
                    // can from inside this loop and plug into other version
                    // (contour only) of this as well

                    if (LinesIntersect(x1, y1, x2, y2,
                                       0.0f, 0.0f, 1.0f, 0.0f) &&
                        OutDualHImg.GetPix(x, y) < P_B) {
                        OutDualHImg.SetPix(x, y, P_B);
                    }
                    if (LinesIntersect(x1, y1, x2, y2,
                                       0.0f, 0.0f, 0.0f, 1.0f) &&
                        OutDualVImg.GetPix(x, y) < P_B) {
                        OutDualVImg.SetPix(x, y, P_B);
                    }
                    if (x > 0) {
                        if (LinesIntersect(x1, y1, x2, y2,
                                           0.0f, 0.0f, -1.0f, 0.0f) &&
                            OutDualHImg.GetPix(x-1, y) < P_B) {
                            OutDualHImg.SetPix(x-1, y, P_B);
                        }
                    }
                    if (y > 0) {
                        if (LinesIntersect(x1, y1, x2, y2,
                                           0.0f, 0.0f, 0.0f, -1.0f) &&
                            OutDualVImg.GetPix(x, y-1) < P_B) {
                            OutDualVImg.SetPix(x, y-1, P_B);
                        }
                    }
                } // if (*pXLoc != 0.0f) {
            } // for (int x = 0; x < Width; x++) {
        } // for (int y = 0; y < Height; y++) {
    } // for (int iScale = 0; iScale < nGaussScales; iScale++,
}
