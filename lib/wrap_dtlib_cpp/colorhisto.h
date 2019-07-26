/////////////////////////////////////////////////////////////////////////////
// colorhisto.h -- Doron Tal

#ifndef _COLORHISTO_H
#define _COLORHISTO_H

#include "wrap_dtlib_cpp/img.h"
#include "wrap_dtlib_cpp/histogram.h"
#include "wrap_dtlib_cpp/circle.h"
#include "string"

namespace DTLib {

    CImg<FloatCHistogramPtr>* MakeEmptyColorHistoImg(const int& Width,
                                                     const int& Height,
                                                     const int& nAs,
                                                     const int& nBs,
                                                     const int& nCs,
                                                     const float& minA,
                                                     const float& maxA,
                                                     const float& minB,
                                                     const float& maxB,
                                                     const float& minC,
													 const float& maxC);

    CImg<FloatCHistogramPtr>* MakeBWEmptyColorHistoImg
    (
    	const int& Width,
        const int& Height,
    	const int& nAs,
    	const float& minA,
    	const float& maxA
    );

    void ComputeColorHistogramImg(CCircleMasks* pCircleMasks,
                                  FloatCImgPtr pTextureScaleImg,
                                  FloatCImgPtr pColorAImg,
                                  FloatCImgPtr pColorBImg,
                                  CImg<FloatCHistogramPtr>* pColorHistoImg,
                                  LongCImgPtr pPresegImg,
                                  const float& SoftUpdateSigma);

    void ComputeColorHistogramImgNew(CCircleMasks* pCircleMasks,
									 FloatCImgPtr pTextureScaleImg,
									 FloatCImgPtr pColorAImg,
									 FloatCImgPtr pColorBImg,
									 FloatCImgPtr pColorCImg,
									 CImg<FloatCHistogramPtr>* pColorHistoImg,
									 LongCImgPtr pPresegImg,
									 const float& SoftUpdateSigma);
    void ComputeColorHistogramImg2Old
    (
    	CCircleMasks* pCircleMasks,
    	FloatCImgPtr pTextureScaleImg,
    	FloatCImgPtr pColorAImg,
    	FloatCImgPtr pColorBImg,
        CImg<FloatCHistogramPtr>* pColorHistoImg,
    	LongCImgPtr pPresegImg,
    	const float& SoftUpdateSigma
    );

    void ComputeColorHistogramImgBWNew
    (
    	CCircleMasks* pCircleMasks,
        FloatCImgPtr pTextureScaleImg,
        FloatCImgPtr pColorAImg,
        CImg<FloatCHistogramPtr>* pColorHistoImg,
        LongCImgPtr pPresegImg,
        const float& SoftUpdateSigma
    );

} // namespace DTLib {

#endif /* #ifndef _COLORHISTO_H */
