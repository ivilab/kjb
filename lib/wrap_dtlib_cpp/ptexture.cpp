/////////////////////////////////////////////////////////////////////////////
// ptexture.cpp - 1D/2D function determining the probability of texture
// Author: Doron Tal
// Date created: May, 2000

#include "l/l_sys_debug.h"  /* For ASSERT. */
#include <iostream>
#include "wrap_dtlib_cpp/ptexture.h"
#include "wrap_dtlib_cpp/histogram.h"

/*
 * Kobus: We have run into trouble with 32 bit centric code in this
 * distribution. I have changed some long's to kjb_int32's.  The immediate
 * problem is that the segmentation maps can get written out as 64 bit integers. 
*/
#include "l/l_sys_def.h"

using namespace std;
using namespace DTLib;
using namespace kjb_c;


/////////////////////////////////////////////////////////////////////////////

void DTLib::ComputePTextureImg(CCircleMasks& CircleMasks,
                               CImg<BYTE>& NMSCombImg,
                               CImg<float>& ThetaStarImg,
                               CImg<float>& TextureScaleImg,
                               CImg<kjb_int32>& TextonMembershipImg,
                               const int& K,
                               const float& DiskMiddleBandThickness,
                               const float& Tau, const float& Beta,
                               CImg<float>& PTextureImg)
{
    // go through theta star image, and scale image, 
    // set up a circle on each pixel -- need to worry about boundary
    // conditions, so we'll reflect the texton membership image first
    // so the first thing we need to do is compute min & max of 
    // scales, so we know how much to reflect
    // that's it!!!
    ASSERT(ThetaStarImg.Width() == TextonMembershipImg.Width());
    ASSERT(ThetaStarImg.Height() == TextonMembershipImg.Height());
    const int Width = ThetaStarImg.Width();
    const int Height = ThetaStarImg.Height();

    const int Padding = CircleMasks.GetMaxRad()+1; // +1 for safety..
    const int PaddedWidth = Width+Padding*2;
    const int PaddedHeight = Height+Padding*2;

    const int StartX = Padding, EndX = Padding+Width;
    const int StartY = Padding, EndY = Padding+Height;

    CImg<kjb_int32> PaddedMemberImg(PaddedWidth, PaddedHeight);
    PaddedMemberImg.ChangeROI(StartX, EndX, StartY, EndY);
    PaddedMemberImg.CopyFromBuf(TextonMembershipImg.pBuffer());
    PaddedMemberImg.ReflectToROI(); // here is where the mirror's done!

    CImg<BYTE> PaddedNMSCombImg(PaddedWidth, PaddedHeight);
    PaddedNMSCombImg.ChangeROI(StartX, EndX, StartY, EndY);
    PaddedNMSCombImg.CopyFromBuf(NMSCombImg.pBuffer());
    PaddedNMSCombImg.ReflectToROI(); // here is where the mirror's done!

    const int Skip = PaddedMemberImg.ROISkipCols();
    kjb_int32* pMemb = PaddedMemberImg.pROI();
    BYTE* pNMSComb = PaddedNMSCombImg.pROI();
    float* pThetaStar = ThetaStarImg.pBuffer();
    float* pTextureScale = TextureScaleImg.pBuffer();
    float* pPTexture = PTextureImg.pBuffer();

    CHistogram<float> HistoL;
    CHistogram<float> HistoC;
    CHistogram<float> HistoR;
    CHistogram<float> HistoTmp;

    HistoL.Setup(K, 0, K-1);
    HistoC.Setup(K, 0, K-1);
    HistoR.Setup(K, 0, K-1);
    HistoTmp.Setup(K, 0, K-1);

    const float HalfMidbandThickness = DiskMiddleBandThickness*0.5f;
    for (int y = 0; y < Height; y++, pMemb += Skip, pNMSComb += Skip) {
        for (int x = 0; x < Width; x++, pThetaStar++, pTextureScale++, 
                 pMemb++, pPTexture++, pNMSComb++) {
            // if (*pNMSComb) {
            if (true) { // computing ptexture everywhere now
                HistoL.Zero();
                HistoC.Zero();
                HistoR.Zero();
                HistoTmp.Zero();        
                const float ThetaStar = *pThetaStar;
                const float CosThetaStar = (float)cos((double)ThetaStar);
                const float MinusSinThetaStar = -(float)sin((double)ThetaStar);
                const int Rad = (int)(*pTextureScale);
                // ByteCImgPtr pCircleImg = m_vecCircleImgs[Rad-m_MinRad];
                // BYTE* pCircle = pCircleImg->pBuffer();
                // BYTE* pCircle = cCir.GetCircleMaskBuffer(Rad);

                BYTE* pCircle = CircleMasks.GetCircleMaskBuffer(Rad);

                kjb_int32* pMembYOffset;
                for (int yy_i = -Rad; yy_i <= Rad; yy_i++) {
                    const float yy = (float)yy_i;
                    pMembYOffset = pMemb+yy_i*PaddedWidth;
                    for (int xx_i = -Rad; xx_i <= Rad; xx_i++, pCircle++) {
                        const float xx = (float)xx_i;
                        if (*pCircle) {
                            const float DotProd =
                                xx*MinusSinThetaStar+yy*CosThetaStar;
                            const float AbsDotProd = (float)fabs(DotProd);
                            const kjb_int32 iTexton = *(pMembYOffset+xx_i);
                            ASSERT((iTexton >= 0) && (iTexton < K));
                            if (AbsDotProd < HalfMidbandThickness) {
                                HistoC.IncrementBin((int)iTexton, 1.0f);
                                //HistoC.Update(iTexton); // wer're in midband
                            }
                            else if (DotProd > 0.0f) {
                                HistoL.IncrementBin((int)iTexton, 1.0f);
                                // HistoL.Update(iTexton); // we're in part 1
                            }
                            else {
                                HistoR.IncrementBin((int)iTexton, 1.0f);
                                // HistoR.Update(iTexton); // we're in part 2
                            }
                        } // if (*pCircle) {
                    } // for (float xx = Start; xx <= End; xx++, pCircle++) {
                } // for (float yy = Start; yy <= End; yy++) {

                HistoTmp.Add(HistoL);
                HistoTmp.Add(HistoC); // we'll chi-square HistoTmp vs. HistoR

                HistoC.Add(HistoR); // we'll chi-square HistoC vs. HistoL

                HistoL.Normalize();
                HistoR.Normalize();

                HistoTmp.Normalize();
                HistoC.Normalize();

                const float ChiSqr1 = HistoL.ChiSquareCompare(&HistoC);
                const float ChiSqr2 = HistoR.ChiSquareCompare(&HistoTmp);
                const float ChiSqr = MAX(ChiSqr1, ChiSqr2);

                *pPTexture = 1.0f-(1.0f/(1.0f+exp(-(ChiSqr-Tau)/Beta)));

            } // if (*pNMSComb)
            else *pPTexture = 0.0f;
            ASSERT((*pPTexture >= 0.0f) && (*pPTexture <= 1.0f));
        } // for (int x = 0; x < Width; x++, ...
    } // for (int y = 0; y < Height; y++, pMemb += Skip) {
}
