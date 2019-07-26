/////////////////////////////////////////////////////////////////////////////
// colorhisto.cpp -- Doron Tal

#include "l/l_sys_debug.h"  /* For ASSERT. */
#include "wrap_dtlib_cpp/colorhisto.h"
#include <iostream>
#include <fstream>

using namespace DTLib;

/////////////////////////////////////////////////
//Added by Prasad
//This is to test if the soft-updating process speeds up
//by having a pre-calculated Gaussian window instead of having to calculate
//it iteratively for every update in the image

#define SOFT_UPDATE_FAST

/////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////

CImg<FloatCHistogramPtr>* DTLib::MakeEmptyColorHistoImg
(
	const int& Width,
    const int& Height,
	const int& nAs,
	const int& nBs,
	const int& nCs,
	const float& minA,
	const float& maxA,
	const float& minB,
	const float& maxB,
	const float& minC,
	const float& maxC
)
{
    CImg<FloatCHistogramPtr>* pColorHistoImg =
        new CImg<FloatCHistogramPtr>(Width, Height);
    FloatCHistogramPtr* ppHisto = pColorHistoImg->pBuffer();
    for (int p = 0; p < pColorHistoImg->nPixels(); p++, ppHisto++) {
        *ppHisto = new CHistogram<float>();
        //(*ppHisto)->Setup(nAs, nBs, minA, maxA, minB, maxB);
        (*ppHisto)->Setup(nAs, nBs, nCs, minA, maxA, minB, maxB, minC, maxC);
    }
    return pColorHistoImg;
}

/////////////////////////////////////////////////////////////////////////////

CImg<FloatCHistogramPtr>* DTLib::MakeBWEmptyColorHistoImg
(
	const int& Width,
    const int& Height,
	const int& nAs,
	const float& minA,
	const float& maxA
)
{
	std::cout << "Nas  is " << nAs << std::endl;
    CImg<FloatCHistogramPtr>* pColorHistoImg =
        new CImg<FloatCHistogramPtr>(Width, Height);
    FloatCHistogramPtr* ppHisto = pColorHistoImg->pBuffer();
    for (int p = 0; p < pColorHistoImg->nPixels(); p++, ppHisto++) {
        *ppHisto = new CHistogram<float>();
        //(*ppHisto)->Setup(nAs, nBs, minA, maxA, minB, maxB);
        (*ppHisto)->Setup(nAs, minA, maxA);
    }
    return pColorHistoImg;
}

/////////////////////////////////////////////////////////////////////////////



void DTLib::ComputeColorHistogramImg(CCircleMasks* pCircleMasks,
                                     FloatCImgPtr pTextureScaleImg,
                                     FloatCImgPtr pColorAImg,
                                     FloatCImgPtr pColorBImg,
                                     CImg<FloatCHistogramPtr>* pColorHistoImg,
                                     LongCImgPtr pPresegImg,
                                     const float& SoftUpdateSigma)
{
    const int Width = pColorAImg->Width();
    const int Height = pColorAImg->Height();

    // create padded reflected color membership image
    const int Padding = pCircleMasks->GetMaxRad();
    const int PaddedWidth = Width+Padding*2;
    const int PaddedHeight = Height+Padding*2;

    const int StartX = Padding, EndX = Padding+Width;
    const int StartY = Padding, EndY = Padding+Height;

    CImg<float> PaddedColorAImg(PaddedWidth, PaddedHeight);
    PaddedColorAImg.ChangeROI(StartX, EndX, StartY, EndY);
    PaddedColorAImg.CopyFromBuf(pColorAImg->pBuffer());
    PaddedColorAImg.ReflectToROI(); // here is where the mirror's done!

    CImg<float> PaddedColorBImg(PaddedWidth, PaddedHeight);
    PaddedColorBImg.ChangeROI(StartX, EndX, StartY, EndY);
    PaddedColorBImg.CopyFromBuf(pColorBImg->pBuffer());
    PaddedColorBImg.ReflectToROI(); // here is where the mirror's done!

    const bool bUpdating = (pPresegImg != NULL);
    const bool bNotUpdating = !bUpdating;

    const int Skip = PaddedColorAImg.ROISkipCols();
    float* pColorA = PaddedColorAImg.pROI();
    float* pColorB = PaddedColorBImg.pROI();
    float* pTextureScale = pTextureScaleImg->pBuffer();
    FloatCHistogramPtr* ppHisto = pColorHistoImg->pBuffer();

/////////////////////////////////////////////////
//Added by Prasad
//This is to test if the soft-updating process speeds up
//by having a pre-calculated Gaussian window instead of having to calculate
//it iteratively for every update in the image

/* For reference

            ---------------------> A or r (x) axis
            |      |     |
	    |   lt |  rt |
	    |      |     |
	    |------c-----|         (a single bin)
	    |      |     |
	    |   lb |  rb |
	    |      |     |
	    |------------|
            |
	    |
	    |
	    |
            V

	 B or g (y) axis

*/

#ifdef SOFT_UPDATE_FAST
    cout << "Using faster soft update" << endl;

    const int Gauss_Rad = F2I(3.0f*SoftUpdateSigma);
    const float RecipSigmaSqr = 1.0f/SQR(SoftUpdateSigma);

    //Defining 5 Gaussian windows depending on the center location in the bin(reference above diagram)
    FloatCImgPtr m_pGauss_Window_c = new CImg<float>( (2*Gauss_Rad)+1, (2*Gauss_Rad)+1 );
    FloatCImgPtr m_pGauss_Window_lt = new CImg<float>( (2*Gauss_Rad)+1, (2*Gauss_Rad)+1 );
    FloatCImgPtr m_pGauss_Window_rt = new CImg<float>( (2*Gauss_Rad)+1, (2*Gauss_Rad)+1 );
    FloatCImgPtr m_pGauss_Window_lb = new CImg<float>( (2*Gauss_Rad)+1, (2*Gauss_Rad)+1 );
    FloatCImgPtr m_pGauss_Window_rb = new CImg<float>( (2*Gauss_Rad)+1, (2*Gauss_Rad)+1 );

    //Gaussian window centered in the middle
    float* pWindow = m_pGauss_Window_c->pBuffer();

    for (int y = -Gauss_Rad; y <= Gauss_Rad; y++){
        for (int x = -Gauss_Rad; x <= Gauss_Rad; x++, pWindow++){
             const float SquaredSum = SQR(x)+SQR(y);
             *pWindow = exp(-SquaredSum*RecipSigmaSqr);

             ASSERT( (*pWindow >= 0.0f) && (*pWindow <= 1.0f) );
        }//for (int x....
    }//for (int y....

    //Gaussian window centered left-top(lt)
    pWindow = m_pGauss_Window_lt->pBuffer();

    for (int y = -Gauss_Rad; y <= Gauss_Rad; y++){
        for (int x = -Gauss_Rad; x <= Gauss_Rad; x++, pWindow++){
             const float SquaredSum = SQR(x+0.25)+SQR(y+0.25);
             *pWindow = exp(-SquaredSum*RecipSigmaSqr);

             ASSERT( (*pWindow >= 0.0f) && (*pWindow <= 1.0f) );
        }//for (int x....
    }//for (int y....

    //Gaussian window centered right-top(rt)
    pWindow = m_pGauss_Window_rt->pBuffer();

    for (int y = -Gauss_Rad; y <= Gauss_Rad; y++){
        for (int x = -Gauss_Rad; x <= Gauss_Rad; x++, pWindow++){
             const float SquaredSum = SQR(x-0.25)+SQR(y+0.25);
             *pWindow = exp(-SquaredSum*RecipSigmaSqr);

             ASSERT( (*pWindow >= 0.0f) && (*pWindow <= 1.0f) );
        }//for (int x....
    }//for (int y....

    //Gaussian window centered left-bottom(lb)
    pWindow = m_pGauss_Window_lb->pBuffer();

    for (int y = -Gauss_Rad; y <= Gauss_Rad; y++){
        for (int x = -Gauss_Rad; x <= Gauss_Rad; x++, pWindow++){
             const float SquaredSum = SQR(x+0.25)+SQR(y-0.25);
             *pWindow = exp(-SquaredSum*RecipSigmaSqr);

             ASSERT( (*pWindow >= 0.0f) && (*pWindow <= 1.0f) );
        }//for (int x....
    }//for (int y....

    //Gaussian window centered right-bottom(rb)
    pWindow = m_pGauss_Window_rb->pBuffer();

    for (int y = -Gauss_Rad; y <= Gauss_Rad; y++){
        for (int x = -Gauss_Rad; x <= Gauss_Rad; x++, pWindow++){
             const float SquaredSum = SQR(x-0.25)+SQR(y-0.25);
             *pWindow = exp(-SquaredSum*RecipSigmaSqr);

             ASSERT( (*pWindow >= 0.0f) && (*pWindow <= 1.0f) );
        }//for (int x....
    }//for (int y....


#endif
/////////////////////////////////////////////////


    int InRegion = -1;
    for (int y = 0; y < Height; y++, pColorA += Skip, pColorB += Skip) {
        for (int x = 0; x < Width; x++, pColorA++, pColorB++,
                 pTextureScale++, ppHisto++) {
            const int Rad = (int)(*pTextureScale);

            ASSERT(Rad >= pCircleMasks->GetMinRad());
            ASSERT(Rad <= pCircleMasks->GetMaxRad());

            float* pColorAYOffset;
            float* pColorBYOffset;

            BYTE* pCircle = pCircleMasks->GetCircleMaskBuffer(Rad);

            FloatCHistogramPtr pHisto = *ppHisto;
            if (bUpdating) {
                pHisto->Zero(); // updating: redo histograms
                InRegion = pPresegImg->GetPix(x, y);
            }
            for (int yy = -Rad, yoffset = -Rad*PaddedWidth; yy <= Rad; yy++,
                     yoffset += PaddedWidth) {
                pColorAYOffset = pColorA+yoffset;
                pColorBYOffset = pColorB+yoffset;
                for (int xx = -Rad; xx <= Rad; xx++, pCircle++) {
                    const int xcoord = x+xx;
                    const int ycoord = y+yy;
                    if ((*pCircle != 0) && // gotta be in the circle, and..
                        (bNotUpdating ||   // either we're not updating
                         (// or we're updating, do appt. tests
                          (xcoord >= 0) && (xcoord < Width) &&
                          (ycoord >= 0) && (ycoord < Height) && // in bounds
                          (InRegion ==  // collecting from same region?
                           pPresegImg->GetPix(xcoord, ycoord))))) {
                        const float ColorA = *(pColorAYOffset+xx);
                        const float ColorB = *(pColorBYOffset+xx);

/////////////////////////////////////////////////
//Added by Prasad
//This is to test if the soft-updating process speeds up
//by having a pre-calculated Gaussian window instead of having to calculate
//it iteratively for every update in the image

#ifdef SOFT_UPDATE_FAST
                        const float RealX = (ColorA - (pHisto->m_XRangeMin))*(pHisto->m_XFactor);
                        const float RealY = (ColorB - (pHisto->m_YRangeMin))*(pHisto->m_YFactor);

                        const int IntX = F2I(RealX);
                        const int IntY = F2I(RealY);

                        const float DeltaX = RealX - (float)IntX;
                        const float DeltaY = RealY - (float)IntY;

                        //Debugging
                        //cerr << "m_XRangeMin = " << pHisto->m_XRangeMin << endl;
                        //cerr << "m_XRangeMax = " << pHisto->m_XRangeMax << endl;
                        //cerr << "m_YRangeMin = " << pHisto->m_YRangeMin << endl;
                        //cerr << "m_YRangeMax = " << pHisto->m_YRangeMax << endl;
                        //cerr << "m_XFactor = " << pHisto->m_XFactor << endl;
                        //cerr << "m_YFactor = " << pHisto->m_YFactor << endl;
                        //cerr << "DeltaX = " << DeltaX << endl;
                        //cerr << "DeltaY = " << DeltaY << endl;

                        ASSERT( DeltaX >= -0.5f && DeltaX <= 0.5f );
                        ASSERT( DeltaY >= -0.5f && DeltaY <= 0.5f );

                        if (DeltaX < 0 && DeltaY < 0)
                        {
                            //Debugging
                            //cerr << "LT" << endl;
                            pHisto->SoftUpdate_fast(ColorA, ColorB, Gauss_Rad, m_pGauss_Window_lt);
                        }
                        else if (DeltaX > 0 && DeltaY < 0)
                        {
                            //Debugging
                            //cerr << "RT" << endl;
                            pHisto->SoftUpdate_fast(ColorA, ColorB, Gauss_Rad, m_pGauss_Window_rt);
                        }
                        else if (DeltaX < 0 && DeltaY > 0)
                        {
                            //Debugging
                            //cerr << "LB" << endl;
                            pHisto->SoftUpdate_fast(ColorA, ColorB, Gauss_Rad, m_pGauss_Window_lb);
                        }
                        else if (DeltaX > 0 && DeltaY > 0)
                        {
                            //Debugging
                            //cerr << "RB" << endl;
                            pHisto->SoftUpdate_fast(ColorA, ColorB, Gauss_Rad, m_pGauss_Window_rb);
                        }
                        else
                        {
                            //Debugging
                            //cerr << "C" << endl;
                            pHisto->SoftUpdate_fast(ColorA, ColorB, Gauss_Rad, m_pGauss_Window_c);
                        }
#else
                        pHisto->SoftUpdate(ColorA, ColorB, SoftUpdateSigma);
#endif
/////////////////////////////////////////////////
                    } // if ((*pCircle ..
                } // for xx
            } // for yy
            ASSERT(!pHisto->IsEmpty());
            pHisto->Normalize();
        } // for (int x = 0; x < Width; x++, ...
    } // for (int y = 0; y < Height; y++, pTexton += Skip) {
}


void DTLib::ComputeColorHistogramImgNew(CCircleMasks* pCircleMasks,
                                     FloatCImgPtr pTextureScaleImg,
                                     FloatCImgPtr pColorAImg,
                                     FloatCImgPtr pColorBImg,
                                     FloatCImgPtr pColorCImg,
                                     CImg<FloatCHistogramPtr>* pColorHistoImg,
                                     LongCImgPtr pPresegImg,
                                     const float& SoftUpdateSigma)
{
    const int Width = pColorAImg->Width();
    const int Height = pColorAImg->Height();

    // create padded reflected color membership image
    const int Padding = pCircleMasks->GetMaxRad();
    const int PaddedWidth = Width+Padding*2;
    const int PaddedHeight = Height+Padding*2;

    const int StartX = Padding, EndX = Padding+Width;
    const int StartY = Padding, EndY = Padding+Height;

    CImg<float> PaddedColorAImg(PaddedWidth, PaddedHeight);
    PaddedColorAImg.ChangeROI(StartX, EndX, StartY, EndY);
    PaddedColorAImg.CopyFromBuf(pColorAImg->pBuffer());
    PaddedColorAImg.ReflectToROI(); // here is where the mirror's done!

    CImg<float> PaddedColorBImg(PaddedWidth, PaddedHeight);
    PaddedColorBImg.ChangeROI(StartX, EndX, StartY, EndY);
    PaddedColorBImg.CopyFromBuf(pColorBImg->pBuffer());
    PaddedColorBImg.ReflectToROI(); // here is where the mirror's done!

    CImg<float> PaddedColorCImg(PaddedWidth, PaddedHeight);
    PaddedColorCImg.ChangeROI(StartX, EndX, StartY, EndY);
    PaddedColorCImg.CopyFromBuf(pColorCImg->pBuffer());
    PaddedColorCImg.ReflectToROI(); // here is where the mirror's done!

    const bool bUpdating = (pPresegImg != NULL);
    const bool bNotUpdating = !bUpdating;

    const int Skip = PaddedColorAImg.ROISkipCols();
    std::cout << "Skip is:" << Skip << std::endl;
    float* pColorA = PaddedColorAImg.pROI();
    float* pColorB = PaddedColorBImg.pROI();
    float* pColorC = PaddedColorCImg.pROI();
    float* pTextureScale = pTextureScaleImg->pBuffer();
    FloatCHistogramPtr* ppHisto = pColorHistoImg->pBuffer();

    int InRegion = -1;
    for (int y = 0; y < Height; y++, pColorA += Skip, pColorB += Skip, pColorC += Skip) {
        for (int x = 0; x < Width; x++, pColorA++, pColorB++, pColorC++,
                 pTextureScale++, ppHisto++) {
            const int Rad = (int)(*pTextureScale);

            ASSERT(Rad >= pCircleMasks->GetMinRad());
            ASSERT(Rad <= pCircleMasks->GetMaxRad());

            float* pColorAYOffset;
            float* pColorBYOffset;
            float* pColorCYOffset;

            BYTE* pCircle = pCircleMasks->GetCircleMaskBuffer(Rad);

            FloatCHistogramPtr pHisto = *ppHisto;
            if (bUpdating) {
                pHisto->Zero(); // updating: redo histograms
                InRegion = pPresegImg->GetPix(x, y);
            }
            for (int yy = -Rad, yoffset = -Rad*PaddedWidth; yy <= Rad; yy++,
                     yoffset += PaddedWidth) {
                pColorAYOffset = pColorA+yoffset;
                pColorBYOffset = pColorB+yoffset;
                pColorCYOffset = pColorC+yoffset;
                for (int xx = -Rad; xx <= Rad; xx++, pCircle++) {
                    const int xcoord = x+xx;
                    const int ycoord = y+yy;
                    if ((*pCircle != 0) && // gotta be in the circle, and..
                        (bNotUpdating ||   // either we're not updating
                         (// or we're updating, do appt. tests
                          (xcoord >= 0) && (xcoord < Width) &&
                          (ycoord >= 0) && (ycoord < Height) && // in bounds
                          (InRegion ==  // collecting from same region?
                           pPresegImg->GetPix(xcoord, ycoord))))) {
                        const float ColorA = *(pColorAYOffset+xx);
                        const float ColorB = *(pColorBYOffset+xx);
                        const float ColorC = *(pColorCYOffset+xx);
                        pHisto->SoftUpdate3D(ColorA, ColorB, ColorC, SoftUpdateSigma);
                    } // if ((*pCircle ..
                } // for xx
            } // for yy
            ASSERT(!pHisto->IsEmpty());
            pHisto->Normalize();
        } // for (int x = 0; x < Width; x++, ...
    } // for (int y = 0; y < Height; y++, pTexton += Skip) {
}

void DTLib::ComputeColorHistogramImgBWNew
(
	CCircleMasks* pCircleMasks,
    FloatCImgPtr pTextureScaleImg,
    FloatCImgPtr pColorAImg,
    CImg<FloatCHistogramPtr>* pColorHistoImg,
    LongCImgPtr pPresegImg,
    const float& SoftUpdateSigma
)
{
    const int Width = pColorAImg->Width();
    const int Height = pColorAImg->Height();

    // create padded reflected color membership image
    const int Padding = pCircleMasks->GetMaxRad();
    const int PaddedWidth = Width+Padding*2;
    const int PaddedHeight = Height+Padding*2;

    const int StartX = Padding, EndX = Padding+Width;
    const int StartY = Padding, EndY = Padding+Height;

    CImg<float> PaddedColorAImg(PaddedWidth, PaddedHeight);
    PaddedColorAImg.ChangeROI(StartX, EndX, StartY, EndY);
    PaddedColorAImg.CopyFromBuf(pColorAImg->pBuffer());
    PaddedColorAImg.ReflectToROI(); // here is where the mirror's done!

    const bool bUpdating = (pPresegImg != NULL);
    const bool bNotUpdating = !bUpdating;

    const int Skip = PaddedColorAImg.ROISkipCols();
    std::cout << "Skip is:" << Skip << std::endl;
    float* pColorA = PaddedColorAImg.pROI();
    float* pTextureScale = pTextureScaleImg->pBuffer();
    FloatCHistogramPtr* ppHisto = pColorHistoImg->pBuffer();

    int InRegion = -1;
    for (int y = 0; y < Height; y++, pColorA += Skip) {
        for (int x = 0; x < Width; x++, pColorA++,
                 pTextureScale++, ppHisto++) {
            const int Rad = (int)(*pTextureScale);

            ASSERT(Rad >= pCircleMasks->GetMinRad());
            ASSERT(Rad <= pCircleMasks->GetMaxRad());

            float* pColorAYOffset;

            BYTE* pCircle = pCircleMasks->GetCircleMaskBuffer(Rad);

            FloatCHistogramPtr pHisto = *ppHisto;
            if (bUpdating) {
                pHisto->Zero(); // updating: redo histograms
                InRegion = pPresegImg->GetPix(x, y);
            }
            for (int yy = -Rad, yoffset = -Rad*PaddedWidth; yy <= Rad; yy++,
                     yoffset += PaddedWidth) {
                pColorAYOffset = pColorA+yoffset;
                for (int xx = -Rad; xx <= Rad; xx++, pCircle++) {
                    const int xcoord = x+xx;
                    const int ycoord = y+yy;
                    if ((*pCircle != 0) && // gotta be in the circle, and..
                        (bNotUpdating ||   // either we're not updating
                         (// or we're updating, do appt. tests
                          (xcoord >= 0) && (xcoord < Width) &&
                          (ycoord >= 0) && (ycoord < Height) && // in bounds
                          (InRegion ==  // collecting from same region?
                           pPresegImg->GetPix(xcoord, ycoord))))) {
                        const float ColorA = *(pColorAYOffset+xx);
                        pHisto->SoftUpdate1D(ColorA, SoftUpdateSigma);
                        //pHisto->Update(ColorA);
                    } // if ((*pCircle ..
                } // for xx
            } // for yy
            ASSERT(!pHisto->IsEmpty());
            pHisto->Normalize();
        } // for (int x = 0; x < Width; x++, ...
    } // for (int y = 0; y < Height; y++, pTexton += Skip) {
}

void DTLib::ComputeColorHistogramImg2Old
(
	CCircleMasks* pCircleMasks,
	FloatCImgPtr pTextureScaleImg,
	FloatCImgPtr pColorAImg,
	FloatCImgPtr pColorBImg,
    CImg<FloatCHistogramPtr>* pColorHistoImg,
	LongCImgPtr pPresegImg,
	const float& SoftUpdateSigma
)
{
    const int Width = pColorAImg->Width();
    const int Height = pColorAImg->Height();

    // create padded reflected color membership image
    const int Padding = pCircleMasks->GetMaxRad();
    const int PaddedWidth = Width+Padding*2;
    const int PaddedHeight = Height+Padding*2;

    const int StartX = Padding, EndX = Padding+Width;
    const int StartY = Padding, EndY = Padding+Height;

    CImg<float> PaddedColorAImg(PaddedWidth, PaddedHeight);
    PaddedColorAImg.ChangeROI(StartX, EndX, StartY, EndY);
    PaddedColorAImg.CopyFromBuf(pColorAImg->pBuffer());
    PaddedColorAImg.ReflectToROI(); // here is where the mirror's done!

    CImg<float> PaddedColorBImg(PaddedWidth, PaddedHeight);
    PaddedColorBImg.ChangeROI(StartX, EndX, StartY, EndY);
    PaddedColorBImg.CopyFromBuf(pColorBImg->pBuffer());
    PaddedColorBImg.ReflectToROI(); // here is where the mirror's done!

    const bool bUpdating = (pPresegImg != NULL);
    const bool bNotUpdating = !bUpdating;

    const int Skip = PaddedColorAImg.ROISkipCols();
    float* pColorA = PaddedColorAImg.pROI();
    float* pColorB = PaddedColorBImg.pROI();
    float* pTextureScale = pTextureScaleImg->pBuffer();
    FloatCHistogramPtr* ppHisto = pColorHistoImg->pBuffer();

    int InRegion = -1;
    for (int y = 0; y < Height; y++, pColorA += Skip, pColorB += Skip) {
        for (int x = 0; x < Width; x++, pColorA++, pColorB++,
                 pTextureScale++, ppHisto++) {
            const int Rad = (int)(*pTextureScale);

            ASSERT(Rad >= pCircleMasks->GetMinRad());
            ASSERT(Rad <= pCircleMasks->GetMaxRad());

            float* pColorAYOffset;
            float* pColorBYOffset;

            BYTE* pCircle = pCircleMasks->GetCircleMaskBuffer(Rad);

            FloatCHistogramPtr pHisto = *ppHisto;
            if (bUpdating) {
                pHisto->Zero(); // updating: redo histograms
                InRegion = pPresegImg->GetPix(x, y);
            }
            for (int yy = -Rad, yoffset = -Rad*PaddedWidth; yy <= Rad; yy++,
                     yoffset += PaddedWidth) {
                pColorAYOffset = pColorA+yoffset;
                pColorBYOffset = pColorB+yoffset;
                for (int xx = -Rad; xx <= Rad; xx++, pCircle++) {
                    const int xcoord = x+xx;
                    const int ycoord = y+yy;
                    if ((*pCircle != 0) && // gotta be in the circle, and..
                        (bNotUpdating ||   // either we're not updating
                         (// or we're updating, do appt. tests
                          (xcoord >= 0) && (xcoord < Width) &&
                          (ycoord >= 0) && (ycoord < Height) && // in bounds
                          (InRegion ==  // collecting from same region?
                           pPresegImg->GetPix(xcoord, ycoord))))) {
                        const float ColorA = *(pColorAYOffset+xx);
                        const float ColorB = *(pColorBYOffset+xx);

                        pHisto->SoftUpdate(ColorA, ColorB, SoftUpdateSigma);
                    } // if ((*pCircle ..
                } // for xx
            } // for yy
            ASSERT(!pHisto->IsEmpty());
            pHisto->Normalize();
        } // for (int x = 0; x < Width; x++, ...
    } // for (int y = 0; y < Height; y++, pTexton += Skip) {
}
