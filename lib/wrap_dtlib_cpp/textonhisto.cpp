/////////////////////////////////////////////////////////////////////////////
// textonhisto.cpp - compute an image of texton histograms
// Author: Doron Tal
// Date Created: April, 2000

#include "l/l_sys_debug.h"  /* For ASSERT. */
#include "wrap_dtlib_cpp/textonhisto.h"

/*
 * Kobus: We have run into trouble with 32 bit centric code in this
 * distribution. I have changed some long's to kjb_int32's.  The immediate
 * problem is that the segmentation maps can get written out as 64 bit integers. 
*/
#include "l/l_sys_def.h"
#include <iostream>
#include <fstream>

using namespace DTLib;
using namespace kjb_c;

/////////////////////////////////////////////////////////////////////////////

CImg<FloatCHistogramPtr>* DTLib::MakeEmptyTextonHistoImg(const int& Width, 
                                                         const int& Height,
                                                         const int& K)
{
    CImg<FloatCHistogramPtr>* pTextonHistoImg = 
        new CImg<FloatCHistogramPtr>(Width, Height);
    FloatCHistogramPtr* ppHisto = pTextonHistoImg->pBuffer();
    for (int p = 0; p < pTextonHistoImg->nPixels(); p++, ppHisto++) {
        *ppHisto = new CHistogram<float>();
        (*ppHisto)->Setup(K+1);
    }
    return pTextonHistoImg;
}

/////////////////////////////////////////////////////////////////////////////

void DTLib::ComputeTextonHistogramImg(CCircleMasks* pCircleMasks,
                                      FloatCImgPtr pTextureScaleImg,
                                      LongCImgPtr pTextonsImg,
                                      FloatCImgPtr pPTextureImg,
                                      CImg<FloatCHistogramPtr>* 
                                      pTextonHistoImg,
                                      LongCImgPtr pPresegImg)
{
    const int Width = pPTextureImg->Width();
    const int Height = pPTextureImg->Height();
    
    // create padded reflected texton membership image
    const int Padding = pCircleMasks->GetMaxRad();
    const int PaddedWidth = Width+Padding*2;
    const int PaddedHeight = Height+Padding*2;
    
    const int StartX = Padding, EndX = Padding+Width;
    const int StartY = Padding, EndY = Padding+Height;

    CImg<kjb_int32> PaddedTextonsImg(PaddedWidth, PaddedHeight);
    PaddedTextonsImg.ChangeROI(StartX, EndX, StartY, EndY);
    PaddedTextonsImg.CopyFromBuf(pTextonsImg->pBuffer());
    PaddedTextonsImg.ReflectToROI(); // here is where the mirror's done!

    // create padded reflected ptexture image
    CImg<float> PaddedPTextureImg(PaddedWidth, PaddedHeight);
    PaddedPTextureImg.ChangeROI(StartX, EndX, StartY, EndY);
    PaddedPTextureImg.CopyFromBuf(pPTextureImg->pBuffer());
    PaddedPTextureImg.ReflectToROI(); // here is where the mirror's done!
  
    const bool bUpdating = (pPresegImg != NULL);
    const bool bNotUpdating = !bUpdating;

    const int Skip = PaddedTextonsImg.ROISkipCols();
    kjb_int32* pTexton = PaddedTextonsImg.pROI();
    float* pPTexture = PaddedPTextureImg.pROI();
    float* pTextureScale = pTextureScaleImg->pBuffer();

    FloatCHistogramPtr* ppHisto = pTextonHistoImg->pBuffer();

    int InRegion = -1;
    for (int y = 0; y < Height; y++, pTexton += Skip, pPTexture += Skip) {
        for (int x = 0; x < Width; x++, pTexton++, pPTexture++,
                 pTextureScale++, ppHisto++) {
            const int Rad = (int)(*pTextureScale);
            ASSERT(Rad >= pCircleMasks->GetMinRad());
            ASSERT(Rad <= pCircleMasks->GetMaxRad());
            BYTE* pCircle = pCircleMasks->GetCircleMaskBuffer(Rad);
            FloatCHistogramPtr pHisto = *ppHisto;
            if (bUpdating) {
                pHisto->Zero(); // updating: redo histograms
                InRegion = pPresegImg->GetPix(x, y);
            }
            kjb_int32* pTextonYOffset;
            float* pPTextureYOffset;
            float BinZeroSum = 0.0f;
            for (int yy = -Rad, yoffset = -Rad*PaddedWidth; yy <= Rad; yy++, 
                     yoffset += PaddedWidth) {
                pTextonYOffset = pTexton+yoffset;
                pPTextureYOffset = pPTexture+yoffset;
                for (int xx = -Rad; xx <= Rad; xx++, pCircle++) {
                    const int xcoord = x+xx;
                    const int ycoord = y+yy;
                    if ((*pCircle != 0) && // gotta be in the circle, and..
                        (bNotUpdating ||   // either we're not updating
                         (                 // or updating, do additional tests
                          (xcoord >= 0) && (xcoord < Width) &&
                          (ycoord >= 0) && (ycoord < Height) && // in bounds
                          (InRegion ==     // collecting from same region?
                           pPresegImg->GetPix(xcoord, ycoord))))) {
                        // add 1 to iTexton, bec. we'll use the zero bin
                        const int iTexton = *(pTextonYOffset+xx)+1;
                        const float PTextureJ = *(pPTextureYOffset+xx);
                        ASSERT((PTextureJ >= 0.0f) && (PTextureJ <= 1.0f));
                        ASSERT(iTexton > 0);
                        pHisto->IncrementBin(iTexton, PTextureJ);
                        BinZeroSum += (1.0f-PTextureJ);
                    } // if ((*pCircle ..
                } // for xx
            } // for yy
            // ASSERT(!pHisto->IsEmpty());
            ASSERT(*(pHisto->m_pBins) == 0);
            // now add the zero'th bin
            *(pHisto->m_pBins) = (float)BinZeroSum;
            pHisto->Normalize();
        } // for (int x = 0; x < Width; x++, ...
    } // for (int y = 0; y < Height; y++, pTexton += Skip) {
}
