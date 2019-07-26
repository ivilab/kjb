////////////////////////////////////////////////////////////////////////////
// orientation_energy.h - compute orientation energy
// Author: Doron Tal
// Date Created: March, 2000

#ifndef _TEXTURE_CPP_H
#define _TEXTURE_CPP_H


#include <string>
#include "wrap_dtlib_cpp/img.h"
#include "wrap_dtlib_cpp/filterbank.h"
#include "m_cpp/m_matrix.h"
#include "i_cpp/i_image.h"
#include "wrap_dtlib_cpp/jpeg.h"
#include "wrap_dtlib_cpp/kmeans.h"
#include "wrap_dtlib_cpp/textonhisto.h"
#include "wrap_dtlib_cpp/texturescale.h"
#include "wrap_dtlib_cpp/channels.h"
#include "wrap_dtlib_cpp/median.h"
#include "wrap_dtlib_cpp/ptexture.h"
#include "wrap_dtlib_cpp/orientation_fit.h"
#include "wrap_dtlib_cpp/subpix_localize.h"
#include "wrap_dtlib_cpp/combine_scales.h"
#include "wrap_dtlib_cpp/weberlaw.h"
#include "wrap_dtlib_cpp/colorhisto.h"
#include "wrap_dtlib_cpp/histogram.h"
#include "wrap_dtlib_cpp/dual_lattice.h"
#include "wrap_dtlib_cpp/sparsepat.h"

namespace DTLib
{

void FilterbankConvolve
(
	FloatCImgPtr m_pInputImage,
	CFilterbank** m_pFilterbank_ptr,
	FloatCImgVecPtr * m_pConvVec_ptr,
	int m_nGaussScales,
	int m_nGaussOrientations,
    float m_GaussSigmaY,
    float m_GaussX2YSigmaRatio,
    int m_nDOGScales,
    float m_DOGExcitSigma,
    float m_DOGInhibSigmaRatio1,
    float m_DOGInhibSigmaRatio2
);

void ComputeOrientationEnergy
(
	FloatCImgPtr m_pInputImage,
	CFilterbank* m_pFilterbank,
	FloatCImgVecPtr m_pConvVec,
	FloatCImgVecPtr * m_pOEVec,
	int m_nGaussScales,
	int m_nGaussOrientations
);

void extract_texture
(
	const kjb::Image & img,
	const kjb::Int_matrix & seg_map,
    kjb::Matrix & Oe_mean,
    kjb::Matrix & Oe_var,
    kjb::Matrix & DOG_mean,
    kjb::Matrix & DOG_var,
	int m_nGaussScales,
	int m_nGaussOrientations,
    float m_GaussSigmaY,
    float m_GaussX2YSigmaRatio,
    int m_nDOGScales,
    float m_DOGExcitSigma,
    float m_DOGInhibSigmaRatio1,
    float m_DOGInhibSigmaRatio2,
    int m_nCroppedPixels
);

FloatCImgPtr convert_kjb_to_CImg
(
	const kjb::Image & img,
	FloatCImgPtr * m_pAImg,
	FloatCImgPtr * m_pBImg,
    int m_nCroppedPixels,
    bool & m_bColor
);

void do_textons
(
	FloatCImgVecPtr m_pConvVec,
	LongCImgPtr * m_pTextonsImg,
    int m_nGaussScales,
    int m_nGaussOrientations,
    int m_nDOGScales,
    int m_InputWidth,
    int m_InputHeight,
    int m_nTextureKMeansK,
    int m_nTextureKMeansIterations
);

void ComputeTextonHistoImg();

CCircleMasks* ComputeCircleMasks
(
    int m_minTextureScale,
    int m_maxTextureScale
);

FloatCImgPtr ComputeTextureScale
(
    LongCImgPtr m_pTextonsImg,
    CTextureScale** m_pTS,
    int m_InputWidth,
    int m_InputHeight,
    int m_nTextureKMeansK,
    float m_TextureMinDist,
    float m_TextureMaxDist,
    float m_TextureAlphaScaleFactor,
    int & m_minTextureScale,
    int & m_maxTextureScale
);

FloatCImgPtr ComputePTexture
(
	DTLib::CCircleMasks* m_pCircleMasks,
	FloatCImgPtr m_pTextureScaleImg,
	LongCImgPtr m_pTextonsImg,
	ByteCImgPtr m_pNMSCombImg,
	FloatCImgPtr m_pOriCombImg,
	int m_InputWidth,
	int m_InputHeight,
	int m_minTextureScale,
	int m_maxTextureScale,
	int m_nTextureKMeansK,
	float m_TextureDiskMiddleWidth,
	float m_TextureTau,
	float m_TextureBeta
);

void ComputeNonmaximaSuppression
(
	CFilterbank * m_pFilterbank,
	FloatCImgVecPtr m_pOEVec,
	CImgVec<BYTE> ** m_pNMSVec,
	CImgVec<float> ** m_pEnergyVec,
	CImgVec<float> ** m_pOriVec,
	CImgVec<float> ** m_pXLocVec,
	CImgVec<float> ** m_pYLocVec,
	CImgVec<float> ** m_pErrVec,
	int m_nGaussScales,
	int m_nGaussOrientations,
	int m_InputWidth,
	int m_InputHeight
);

void CombineScales
(
	CImgVec<BYTE> * m_pNMSVec,
	CImgVec<float> * m_pEnergyVec,
	CImgVec<float> * m_pOriVec,
	ByteCImgPtr  * m_pNMSCombImg,
	FloatCImgPtr * m_pOriCombImg,
	FloatCImgPtr * m_pEnergyCombImg,
	int m_InputWidth,
	int m_InputHeight,
	int m_nGaussScales
);

void ComputeTextonHistoImg
(
	LongCImgPtr m_pTextonsImg,
	FloatCImgPtr m_pTextureScaleImg,
	FloatCImgPtr m_pPTextureImg,
	DTLib::CCircleMasks* m_pCircleMasks,
	CImg<FloatCHistogramPtr>** m_pTextonHistoImg,
    int m_InputWidth,
    int m_InputHeight,
    int m_nTextureKMeansK
);

void ComputeWeberLaw
(
	CFilterbank* m_pFilterbank_ptr,
	FloatCImgVecPtr m_pConvVec_ptr,
	float m_TextureWeberLawConst
);


void Compute_Textons_Histograms
(
	const kjb::Image & img,
	CImg<FloatCHistogramPtr>** m_pTextonHistoImg,
	CImg<FloatCHistogramPtr>** ColorHist,
    FloatCImgPtr * m_pDualHImg,
    FloatCImgPtr * m_pDualVImg,
    ByteCImgPtr * m_pSparsePatImg,
	int m_nGaussScales,
	int m_nGaussOrientations,
	float m_GaussSigmaY,
	float m_GaussX2YSigmaRatio,
	int m_nDOGScales,
	float m_DOGExcitSigma,
	float m_DOGInhibSigmaRatio1,
	float m_DOGInhibSigmaRatio2,
    int m_nCroppedPixels,
    float m_TextureWeberLawConst,
    int m_nTextureKMeansK,
    int m_nTextureKMeansIterations,
    float m_TextureMinDist,
    float m_TextureMaxDist,
    float m_TextureAlphaScaleFactor,
	float m_TextureDiskMiddleWidth,
	float m_TextureTau,
	float m_TextureBeta,
	int m_nBinsA,
	int m_nBinsB,
	int m_nBinsC,
	float m_ColorHistoSoftBinSigma,
	float m_EdgelLength,
	float m_InterveningContourSigma,
	int m_SparsePatternDenseRad,
	int m_SparsePatternMaxRad,
	int m_nSparsePatternSamples,
	bool & m_bColor
);

void Compute_All_Histograms
(
	const kjb::Image & img,
    CImg<FloatCHistogramPtr> ** m_pTextonHistoImg,
    CImg<FloatCHistogramPtr> ** ColorHist
);

void ComputeColorHistoImg
(
	const kjb::Image & img,
    CImg<FloatCHistogramPtr> ** ColorHist,
	int m_nBinsA = 8,
	int m_nBinsB = 8,
	int m_nBinsC = 8,
	float m_ColorHistoSoftBinSigma = 1.8f,
    int m_nCroppedPixels = 10,
    int m_nGaussScales = 3,
    int m_nGaussOrientations = 12,
    float m_GaussSigmaY = 1.41f,
    float m_GaussX2YSigmaRatio = 3.0f,
    int m_nDOGScales = 3,
    float m_DOGExcitSigma = 1.41f,
    float m_DOGInhibSigmaRatio1 = 0.62f,
    float m_DOGInhibSigmaRatio2 = 1.6f,
    float m_TextureMinDist = 3.0f,
    float m_TextureMaxDist = 0.1f,
    float m_TextureAlphaScaleFactor = 1.5f,
    float m_TextureDiskMiddleWidth = 3.0f
);

CImg<FloatCHistogramPtr>* ComputeColorHistoImg
(
	FloatCImgPtr m_pTextureScaleImg,
	CCircleMasks* m_pCircleMasks,
	FloatCImgPtr m_pAImg,
	FloatCImgPtr m_pBImg,
	FloatCImgPtr m_pCImg,
	int m_InputWidth,
	int m_InputHeight,
	int m_nBinsA,
	int m_nBinsB,
	int m_nBinsC,
	float m_ColorHistoSoftBinSigma,
	bool is_colour
);

void ComputeDualLattice
(
	CImgVec<float> * m_pEnergyVec,
	CImgVec<float> * m_pOriVec,
	CImgVec<BYTE> * m_pNMSVec,
	CImgVec<float> * m_pXLocVec,
	CImgVec<float> * m_pYLocVec,
	FloatCImgPtr m_pPTextureImg,
    FloatCImgPtr * m_pDualHImg,
    FloatCImgPtr * m_pDualVImg,
	int m_InputWidth,
	int m_InputHeight,
	int m_nGaussScales,
	float m_EdgelLength,
	float m_InterveningContourSigma
);

ByteCImgPtr ComputeSparsePattern
(
	int m_SparsePatternDenseRad,
	int m_SparsePatternMaxRad,
	int m_nSparsePatternSamples
);

CImg<FloatCHistogramPtr>* ReadHistoImg(const std::string & file_name);

void WriteHistoImg
(
	const std::string & file_name,
	CImg<FloatCHistogramPtr> * pHistoImg
);

CImg<FloatCHistogramPtr>* ReadTextonHistoImg(const std::string & file_name);

CImg<FloatCHistogramPtr>* ReadColorHistoImg(const std::string & file_name);

void WriteTextonHistoImg
(
	const std::string & file_name,
	CImg<FloatCHistogramPtr>* pHistoImg
);

void WriteColorHistoImg
(
	const std::string & file_name,
	CImg<FloatCHistogramPtr>* pHistoImg
);

void PrepareHeatMap
(
	kjb::Image & img,
	CImg<FloatCHistogramPtr>* pHistoImg,
	int padding,
	int x_position,
	int y_position
);


}
#endif /* #ifndef _ORIENTATION_ENERGY_H */
