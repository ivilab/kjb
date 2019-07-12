////////////////////////////////////////////////////////////////////////////
// orientation_energy.h - compute orientation energy
// Author: Doron Tal
// Date Created: March, 2000

#include "l/l_sys_debug.h"  /* For ASSERT. */
#include <wrap_dtlib_cpp/texture.h>
#include "wrap_dtlib_cpp/orientation_energy.h"
#include "wrap_dtlib_cpp/getfeatures.h"
#include "wrap_dtlib_cpp/colorconv.h"
#include <fcntl.h>
#include <l_cpp/l_int_matrix.h>
#include <iostream>
#include <limits>


DTLib::CImg<DTLib::FloatCHistogramPtr>* DTLib::ReadHistoImg(const std::string & file_name)
{
	using namespace std;
	ifstream ifs(file_name.c_str());
	int Width, Height;
	ifs >> Width;
	ifs >> Height;
	CImg<FloatCHistogramPtr>* pHistoImg =
			new CImg<FloatCHistogramPtr>(Width, Height);
	FloatCHistogramPtr* ppHisto = pHistoImg->pBuffer();
	for (int p = 0; p < pHistoImg->nPixels(); p++, ppHisto++)
	{
		*ppHisto = new CHistogram<float>(ifs);
	}
	ifs.close();
	return pHistoImg;
}

DTLib::CImg<DTLib::FloatCHistogramPtr>* DTLib::ReadTextonHistoImg(const std::string & file_name)
{
    using namespace std;
	int numBins = 0;
	int Width = 0;
	int Height = 0;
    ifstream ifs(file_name.c_str());
    ifs >> Width;
    ifs >> Height;
    ifs >> numBins;
    ifs.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

    std::cout << file_name << std::endl;
    std::cout << "W - H :" << Width << " - " << Height << std::endl;
    std::cout << "NBINS: " << numBins << std::endl;

    CImg<FloatCHistogramPtr>* pHistoImg =
    			new CImg<FloatCHistogramPtr>(Width, Height);
    int buf_size = ((sizeof(float)*numBins)*Width);
    char _buffer[buf_size + 1];
    FloatCHistogramPtr* ppHisto = pHistoImg->pBuffer();
    for(unsigned int i = 0; i < Height ; i++)
    {
        ifs.read(_buffer, buf_size);
        ASSERT(!ifs.fail());
        ASSERT(!ifs.eof());
        float * pointer = (float *) _buffer;
        for(unsigned int j = 0; j < Width; j++)
        {
        	*ppHisto = new CHistogram<float>();

        	(*ppHisto)->Setup(numBins);
        	(*ppHisto)->SetValues(pointer);
        	pointer += numBins;
        	ppHisto++;
        }
    }
    ifs.close();
    return pHistoImg;
}

DTLib::CImg<DTLib::FloatCHistogramPtr>* DTLib::ReadColorHistoImg(const std::string & file_name)
{
    using namespace std;
    int _numBins = 0;
	int numBinsX = 0;
	int numBinsY = 0;
	int numBinsZ = 0;
	float minX = 0.0;
	float maxX = 0.0;
	float minY = 0.0;
	float maxY = 0.0;
	float minZ = 0.0;
	float maxZ = 0.0;
	int Width = 0;
	int Height = 0;
    ifstream ifs(file_name.c_str());
    ifs >> Width;
    ifs >> Height;
    ifs >> numBinsX;
    ifs >> numBinsY;
    ifs >> numBinsZ;
    ifs >> minX;
    ifs >> maxX;
    ifs >> minY;
    ifs >> maxY;
    ifs >> minZ;
    ifs >> maxZ;
    std::cout << file_name << std::endl;
    std::cout << "W - H :" << Width << " - " << Height << std::endl;
    std::cout << "NBINS: " << numBinsX << std::endl;
    ifs.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    int numBins = numBinsX*numBinsY*numBinsZ;
    bool threed = true;
    if( (numBinsY == 0) || (numBinsZ == 0))
    {
        threed = false;
        numBins = numBinsX;
    }

    CImg<FloatCHistogramPtr>* pHistoImg =
    			new CImg<FloatCHistogramPtr>(Width, Height);
    int buf_size = ((sizeof(float)*numBins)*Width);
    char _buffer[buf_size + 1];
    FloatCHistogramPtr* ppHisto = pHistoImg->pBuffer();
    for(unsigned int i = 0; i < Height ; i++)
    {
        ifs.read(_buffer, buf_size);
        ASSERT(!ifs.fail());
        ASSERT(!ifs.eof());
        float * pointer = (float *) _buffer;
        for(unsigned int j = 0; j < Width; j++)
        {
        	*ppHisto = new CHistogram<float>();

        	if(threed)
        	{
        	    (*ppHisto)->Setup(numBinsX, numBinsY, numBinsZ, minX, maxX, minY, maxY, minZ, maxZ);
        	}
        	else
        	{
        		(*ppHisto)->Setup(numBinsX, minX, maxX);
        	}
        	(*ppHisto)->SetValues(pointer);
        	ppHisto++;
        	pointer += numBins;
        }
    }
    ifs.close();
    return pHistoImg;
}

void DTLib::WriteTextonHistoImg
(
	const std::string & file_name,
	CImg<FloatCHistogramPtr>* pHistoImg
)
{
    using namespace std;
    ofstream ofs(file_name.c_str());
    ofs << pHistoImg->Width() << std::endl;
    ofs << pHistoImg->Height() << std::endl;
    ofs << pHistoImg->GetPix(0)->nBins() << std::endl;

    int buf_size = ((sizeof(float)*(pHistoImg->GetPix(0)->nBins()))*pHistoImg->Width());
    char _buffer[buf_size + 1];
    FloatCHistogramPtr* ppHisto = pHistoImg->pBuffer();
    for(unsigned int i = 0; i < pHistoImg->Height() ; i++)
    {
    	float *pointer = (float *) _buffer;
    	for(unsigned int j = 0; j < pHistoImg->Width(); j++)
    	{
    		(*ppHisto)->GetValues(pointer);
    		ppHisto++;
    		pointer += pHistoImg->GetPix(0)->nBins();
    	}
    	ofs.write(_buffer, buf_size);
    }
    ofs.close();
}

void DTLib::WriteColorHistoImg
(
	const std::string & file_name,
	CImg<FloatCHistogramPtr>* pHistoImg
)
{
    using namespace std;
    ofstream ofs(file_name.c_str());
    ofs << pHistoImg->Width() << std::endl;
    ofs << pHistoImg->Height() << std::endl;
    ofs << pHistoImg->GetPix(0)->m_nBinsX << std::endl;
    ofs << pHistoImg->GetPix(0)->m_nBinsY << std::endl;
    ofs << pHistoImg->GetPix(0)->m_nBinsZ << std::endl;
    ofs << pHistoImg->GetPix(0)->m_XRangeMin << std::endl;
    ofs << pHistoImg->GetPix(0)->m_XRangeMax << std::endl;
    ofs << pHistoImg->GetPix(0)->m_YRangeMin << std::endl;
    ofs << pHistoImg->GetPix(0)->m_YRangeMax << std::endl;
    ofs << pHistoImg->GetPix(0)->m_ZRangeMin << std::endl;
    ofs << pHistoImg->GetPix(0)->m_ZRangeMax << std::endl;

    int buf_size = ((sizeof(float)*(pHistoImg->GetPix(0)->nBins()))*pHistoImg->Width());
    char _buffer[buf_size + 1];
    FloatCHistogramPtr* ppHisto = pHistoImg->pBuffer();
    for(unsigned int i = 0; i < pHistoImg->Height() ; i++)
    {
    	float *pointer = (float *) _buffer;
    	for(unsigned int j = 0; j < pHistoImg->Width(); j++)
    	{
    		(*ppHisto)->GetValues(pointer);
    		ppHisto++;
    		pointer += pHistoImg->GetPix(0)->nBins();
    	}
    	ofs.write(_buffer, buf_size);
    }
    ofs.close();
}

void DTLib::WriteHistoImg
(
	const std::string & file_name,
	CImg<FloatCHistogramPtr> * pHistoImg
)
{
	using namespace std;
	ofstream ofs(file_name.c_str());
	ofs << pHistoImg->Width() << std::endl;
	ofs << pHistoImg->Height() << std::endl;
	FloatCHistogramPtr* ppHisto = pHistoImg->pBuffer();
	for (int p = 0; p < pHistoImg->nPixels(); p++, ppHisto++)
	{
		(*ppHisto)->Write(ofs);
	}
	ofs.close();
}

void DTLib::FilterbankConvolve
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
)
{
	CFilterbank* m_pFilterbank = NULL;
	FloatCImgVecPtr m_pConvVec = NULL;
	ASSERT(m_pInputImage != NULL); // input

    // allocate m_pFilterbank
    m_pFilterbank = new CFilterbank;
    m_pFilterbank->Setup(m_nGaussScales, m_nGaussOrientations,
                         m_GaussSigmaY, m_GaussX2YSigmaRatio,
                         m_nDOGScales, m_DOGExcitSigma,
                         m_DOGInhibSigmaRatio1, m_DOGInhibSigmaRatio2);
    int m_nFilterbankKernels = m_pFilterbank->nKernels();

    // allocate m_pConvVec
    m_pConvVec = new CImgVec<float>;
    m_pConvVec->Allocate(m_nFilterbankKernels, m_pInputImage->Width(),m_pInputImage->Height());

    cerr << "Computing Filterbank Convolutions" << endl;
    m_pFilterbank->Convolve(*m_pInputImage, *m_pConvVec);
    *(m_pFilterbank_ptr) = m_pFilterbank;
    *(m_pConvVec_ptr) = m_pConvVec;
}


void DTLib::ComputeOrientationEnergy
(
	FloatCImgPtr m_pInputImage,
	CFilterbank* m_pFilterbank,
	FloatCImgVecPtr m_pConvVec,
	FloatCImgVecPtr * m_pOEVec_ptr,
	int m_nGaussScales,
	int m_nGaussOrientations
)
{
	FloatCImgVecPtr m_pOEVec = NULL;
	ASSERT(m_pFilterbank != NULL); // input
    ASSERT(m_pConvVec != NULL); // input

    m_pOEVec = new CImgVec<float>;
    m_pOEVec->Allocate(m_nGaussScales*m_nGaussOrientations,
    		m_pInputImage->Width(),m_pInputImage->Height(), true);

	cerr << "Computing Orientation Energy" << endl;
	OrientationEnergy(m_nGaussScales, m_nGaussOrientations, *m_pConvVec, *m_pOEVec);
	*(m_pOEVec_ptr) = m_pOEVec;
}

void DTLib::extract_texture
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
)
{
	FloatCImgPtr m_pAImg = NULL;
	FloatCImgPtr m_pBImg = NULL;
	bool m_bColor;
	FloatCImgPtr m_pInputImage = convert_kjb_to_CImg(img, &m_pAImg, &m_pBImg, m_nCroppedPixels, m_bColor);
	CFilterbank * m_pFilterbank = NULL;
	FloatCImgVecPtr m_pOEVec = NULL;
	FloatCImgVecPtr m_pConvVec = NULL;
	FilterbankConvolve(m_pInputImage, &m_pFilterbank, &m_pConvVec, m_nGaussScales, m_nGaussOrientations, m_GaussSigmaY,
			m_GaussX2YSigmaRatio, m_nDOGScales, m_DOGExcitSigma, m_DOGInhibSigmaRatio1, m_DOGInhibSigmaRatio2);
    ComputeOrientationEnergy(m_pInputImage, m_pFilterbank, m_pConvVec, &m_pOEVec, m_nGaussScales, m_nGaussOrientations);
    GetTextureFeatures(*(seg_map.get_c_matrix()->elements),m_pInputImage->Width(),m_pInputImage->Height(),
           m_nGaussScales, m_nGaussOrientations, *m_pOEVec, *m_pConvVec, m_nDOGScales,
           Oe_mean, Oe_var, DOG_mean, DOG_var);
    zap(m_pFilterbank);
    zap(m_pConvVec);
    zap(m_pOEVec);
    zap(m_pInputImage);

}

DTLib::FloatCImgPtr DTLib::convert_kjb_to_CImg
(
	const kjb::Image & img,
	FloatCImgPtr * m_pAImg,
	FloatCImgPtr * m_pBImg,
    int m_nCroppedPixels,
    bool & m_bColor
)
{
	m_bColor = true;
	FloatCImgPtr m_pInputImage = NULL;
	CJPEG* pJPEG = new CJPEG;
	pJPEG->Convert_from_kjb(img.c_ptr());

    const int OrigWidth = pJPEG->Width();
    const int OrigHeight = pJPEG->Height();

    if ((m_nCroppedPixels*2 >= OrigWidth) ||
        (m_nCroppedPixels*2 >= OrigHeight)) {
        KJB_THROW_2(kjb::KJB_error, "Converion from kjb to Cimg, requested cropped area is larger than the entire image");
    }

    // if BPP == 3 && color --> get LAB
    // if BPP == 3 && !color --> get L of LAB *or* RGB2GRAY (needs testing)
    // if BPP == 1 && !color --> just grab gray from jpeg bufffer

    if (pJPEG->nBytesPerPixel() == 3) {

        const int nColorPixels = OrigWidth*OrigHeight*3;
        float* pLABBuf = new float[nColorPixels];

        //BGR2Srg((pJPEG->pBuffer()), pLABBuf, OrigWidth, OrigHeight);
        BGR2LAB(pJPEG->pBuffer(), pLABBuf, OrigWidth, OrigHeight);

        m_pInputImage = new CImg<float>(OrigWidth, OrigHeight);
        (*m_pAImg) = new CImg<float>(OrigWidth, OrigHeight);
        (*m_pBImg) = new CImg<float>(OrigWidth, OrigHeight);

        Interleaved2Planar(pLABBuf, m_pInputImage->pBuffer(),
                           (*m_pAImg)->pBuffer(), (*m_pBImg)->pBuffer(),
                           OrigWidth, OrigHeight);
        zap(pLABBuf);

        cerr << "Input Width = " << m_pInputImage->Width()
             << ", Height = " << m_pInputImage->Height() << endl << endl;
        m_pInputImage->Crop(m_nCroppedPixels);
        (*m_pAImg)->Crop(m_nCroppedPixels);
        (*m_pBImg)->Crop(m_nCroppedPixels);

        float minA, maxA, minB, maxB;
        (*m_pAImg)->MinMax(minA, maxA);
        (*m_pBImg)->MinMax(minB, maxB);
        const float rangeA = maxA-minA;
        const float rangeB = maxB-minB;

      std::cout << "Range a:" << rangeA << std::endl;
      std::cout << "Range b:" << rangeB << std::endl;
      if ((rangeA < 1.0f) || (rangeB < 1.0f))
      {
            //m_bColor = false;
      }


    } // if (pJPEG->nBytesPerPixel() == 3) {
    else
    {
    	m_bColor = false;
    	std::cout << "Only a channel" << std::endl;
    	ASSERT(pJPEG->nBytesPerPixel() == 1);
        ByteCImgPtr pByteImg = new CImg<BYTE>((unsigned char*)(pJPEG->pBuffer()), OrigWidth,
                                              OrigHeight, false);

        pByteImg->ReduceROI(m_nCroppedPixels);

        ASSERT(m_pInputImage == NULL);
        m_pInputImage = new CImg<float>(pByteImg->ROIWidth(),
                                        pByteImg->ROIHeight());
        pByteImg->b2f(*m_pInputImage);

        zap(pByteImg);
    } // else

    m_pInputImage->ChangeRange(0.0f, 1.0f);

    zap(pJPEG);

    if (m_nCroppedPixels != 0)
    {
        cerr << "Cropping input image inwards by " << m_nCroppedPixels
             << " Pixels\n";
        cerr << "Input Width = " << m_pInputImage->Width()
             << ", Height = " << m_pInputImage->Height() << endl << endl;
    }

    return m_pInputImage;
}


void DTLib::do_textons
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
)
{
	ASSERT(m_pConvVec != NULL); // input

	int nKMFrms = m_nGaussScales*m_nGaussOrientations/2+m_nDOGScales;

	// the following loop only picks half the orientations to do
	// k-means on, before we feed them to k-means, set up ppBuffers.
	float** ppBuffers = new float*[nKMFrms];
	int i, iFull = 0, iHalf = 0;
	for (i = 0; i < m_nGaussScales; i++) {
		for (int j = 0; j < m_nGaussOrientations; j ++, iFull++) {
			if (j % 2) { // only do 1/2 of all orientations
				ppBuffers[iHalf] = (m_pConvVec->GetImg(iFull))->pBuffer();
				iHalf++;
			} // if (j % 2)
		} // for j
	} // for i
	for (i = 0; i < m_nDOGScales; i++, iFull++, iHalf++) {
		ppBuffers[iHalf] = (m_pConvVec->GetImg(iFull))->pBuffer();
	}


	ASSERT(iHalf == nKMFrms);
	CKMeans KM(m_InputWidth*m_InputHeight,
			   m_nTextureKMeansK, nKMFrms, ppBuffers);
	zap(ppBuffers); // no longer need this, bec. transposed it in KM
	KM.Init();
	// KM.RefinedInit(m_InputWidth*m_InputHeight/10, 20);
	KM.Iterate(m_nTextureKMeansIterations, -1);
	// K-Means prunning now done on textons!!!
	KM.Prune(1.1f);
	// remember prunned version
	m_nTextureKMeansK = KM.nK();
	*m_pTextonsImg = new CImg<kjb_int32>(KM.pPtClustersDetach(),
								   m_InputWidth, m_InputHeight, false);
}


void DTLib::ComputeTextonHistoImg
(
	LongCImgPtr m_pTextonsImg,
	FloatCImgPtr m_pTextureScaleImg,
	FloatCImgPtr m_pPTextureImg,
	DTLib::CCircleMasks* m_pCircleMasks,
	CImg<FloatCHistogramPtr>** m_pTextonHistoImg,
    int m_InputWidth,
    int m_InputHeight,
    int m_nTextureKMeansK
)
{
    ASSERT(m_pTextureScaleImg != NULL); // input
    ASSERT(m_pTextonsImg != NULL); // input
    ASSERT(m_pPTextureImg != NULL); // input

    (*m_pTextonHistoImg) = MakeEmptyTextonHistoImg(m_InputWidth, m_InputHeight,
                                                m_nTextureKMeansK);

    ComputeTextonHistogramImg(m_pCircleMasks, m_pTextureScaleImg,
                              m_pTextonsImg, m_pPTextureImg, *m_pTextonHistoImg);
}

DTLib::CCircleMasks* DTLib::ComputeCircleMasks
(
    int m_minTextureScale,
    int m_maxTextureScale
)
{
    ASSERT(m_minTextureScale != 0);
    ASSERT(m_maxTextureScale != 0);
    return new CCircleMasks(m_minTextureScale, m_maxTextureScale);
}

DTLib::FloatCImgPtr DTLib::ComputeTextureScale
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
)
{
    ASSERT(m_pTextonsImg != NULL); // input
    FloatCImgPtr m_pTextureScaleImg = NULL;
    m_pTextureScaleImg = new CImg<float>(m_InputWidth, m_InputHeight);

	(*m_pTS) = new CTextureScale(m_InputWidth*m_InputHeight);
	CImg<BYTE> TextonChannelImg(m_InputWidth, m_InputHeight, false, false);
	CImg<float> TextureScalePreMedianImg(m_InputWidth, m_InputHeight,
										 true, true);
	CImgVec<BYTE> TextonChannelVec;
	TextonChannelVec.Allocate(m_nTextureKMeansK, m_InputWidth,
							  m_InputHeight);

	SeparateChannels(*m_pTextonsImg, TextonChannelVec, m_nTextureKMeansK);
	for (int k = 0; k < m_nTextureKMeansK; k++) {
		BYTE* pTextonChannelBuffer = TextonChannelVec.pBuffer(k);
		TextonChannelImg.Attach(pTextonChannelBuffer);
		// DeLaunay won't work with less than 3 points:
		(*m_pTS)->ComputeScaleImg(TextonChannelImg,
							   TextureScalePreMedianImg,
							   m_TextureMinDist, m_TextureMaxDist,
							   m_TextureAlphaScaleFactor);
	}
	MedianFilter(TextureScalePreMedianImg, *m_pTextureScaleImg, 2);

    float MinTextureScale, MaxTextureScale;
    m_pTextureScaleImg->MinMax(MinTextureScale, MaxTextureScale);

    m_minTextureScale = (int)MinTextureScale;
    m_maxTextureScale = (int)MaxTextureScale;
    std::cout << "m_minTextureScale: " << m_minTextureScale << std::endl;
    std::cout << "m_maxTextureScale: " << m_maxTextureScale << std::endl;

    return m_pTextureScaleImg;
}

DTLib::FloatCImgPtr DTLib::ComputePTexture
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
)
{

	FloatCImgPtr m_pPTextureImg = new CImg<float>(m_InputWidth, m_InputHeight);
	ASSERT(m_pCircleMasks != NULL);

	ASSERT(m_minTextureScale != 0);
	ASSERT(m_maxTextureScale != 0);
	ASSERT(m_pTextureScaleImg != NULL);
	ASSERT(m_pTextonsImg != NULL);
	ASSERT(m_pNMSCombImg != NULL);
	ASSERT(m_pOriCombImg != NULL);

	ComputePTextureImg(*m_pCircleMasks,
					   *m_pNMSCombImg, *m_pOriCombImg,
					   *m_pTextureScaleImg,
					   *m_pTextonsImg, m_nTextureKMeansK,
					   m_TextureDiskMiddleWidth,
					   m_TextureTau, m_TextureBeta,
					   *m_pPTextureImg);
	return m_pPTextureImg;
}


void DTLib::ComputeNonmaximaSuppression
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
)
{
    ASSERT(m_pOEVec != NULL); // input
    ASSERT(m_pFilterbank != NULL); // input

    (*m_pNMSVec) = new CImgVec<BYTE>;
    (*m_pEnergyVec) = new CImgVec<float>;
    (*m_pOriVec) = new CImgVec<float>;
    (*m_pXLocVec) = new CImgVec<float>;
    (*m_pYLocVec) = new CImgVec<float>;
    (*m_pErrVec) = new CImgVec<float>;

    (*m_pNMSVec)->Allocate(m_nGaussScales, m_InputWidth, m_InputHeight);
    (*m_pEnergyVec)->Allocate(m_nGaussScales, m_InputWidth, m_InputHeight, true);
    (*m_pOriVec)->Allocate(m_nGaussScales, m_InputWidth, m_InputHeight);
    (*m_pXLocVec)->Allocate(m_nGaussScales, m_InputWidth, m_InputHeight);
    (*m_pYLocVec)->Allocate(m_nGaussScales, m_InputWidth, m_InputHeight);
    (*m_pErrVec)->Allocate(m_nGaussScales, m_InputWidth, m_InputHeight);

    // try opening (first image of) all six input squences
	cerr << "Computing Nonmaxima Suppression (1) "
		 << "Parabolic Orientations Fit" << endl;

	ByteCImgVecPtr pOrientationIdxVec = new CImgVec<BYTE>; // temporary
	pOrientationIdxVec->Allocate(m_nGaussScales, m_InputWidth,
								 m_InputHeight);

	ParabolicOrientationsFit(m_nGaussScales, m_nGaussOrientations,
							 *m_pOEVec, **m_pEnergyVec, **m_pOriVec,
							 *pOrientationIdxVec);

	cerr << "Computing Nonmaxima Suppression (2) Subpixel "
		 << "Localization" << endl;
	SubpixelLocalization(m_nGaussScales, m_nGaussOrientations,
						 *m_pOEVec, *pOrientationIdxVec,
						 **m_pNMSVec, **m_pEnergyVec,
						 **m_pOriVec, **m_pXLocVec,
						 **m_pYLocVec, **m_pErrVec);

	zap(pOrientationIdxVec);

	(*m_pOriVec)->FixThetaRanges(true);
}

void DTLib::CombineScales
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
)
{
    ASSERT(m_pNMSVec != NULL); // input
    ASSERT(m_pEnergyVec != NULL); // input
    ASSERT(m_pOriVec != NULL); // input

    (*m_pNMSCombImg) = new CImg<BYTE>(m_InputWidth, m_InputHeight);
    (*m_pEnergyCombImg) = new CImg<float>(m_InputWidth, m_InputHeight);
    (*m_pOriCombImg) = new CImg<float>(m_InputWidth, m_InputHeight);

	CombineScalesNMSRhoTheta(m_nGaussScales, *m_pNMSVec, *m_pEnergyVec,
							 *m_pOriVec, **m_pNMSCombImg, **m_pEnergyCombImg,
							 **m_pOriCombImg);
}

void DTLib::ComputeWeberLaw
(
	CFilterbank* m_pFilterbank,
	FloatCImgVecPtr m_pConvVec,
	float m_TextureWeberLawConst
)
{
    ASSERT(m_pFilterbank != NULL); // input
    ASSERT(m_pConvVec != NULL); // input
	cerr << "Computing Weber Law on convolutions" << endl;
	WeberLaw(*m_pConvVec, m_TextureWeberLawConst);
}


void DTLib::Compute_Textons_Histograms
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
)
{
    //LoadParameters();

	FloatCImgPtr m_pAImg = NULL;
	FloatCImgPtr m_pBImg = NULL;
	FloatCImgPtr m_pInputImage = convert_kjb_to_CImg(img, &m_pAImg, &m_pBImg, m_nCroppedPixels, m_bColor);
	CFilterbank * m_pFilterbank = NULL;
	FloatCImgVecPtr m_pOEVec = NULL;
	FloatCImgVecPtr m_pConvVec = NULL;
	CImgVec<BYTE> * m_pNMSVec = NULL;
	CImgVec<float> * m_pEnergyVec = NULL;
	CImgVec<float> * m_pOriVec = NULL;
	CImgVec<float> * m_pXLocVec = NULL;
	CImgVec<float> * m_pYLocVec = NULL;
	CImgVec<float> * m_pErrVec = NULL;
	bool is_colour = true;
	if(kjb_c::is_black_and_white(img.c_ptr()))
	{
		is_colour = false;
	}
    std::cout << " is color: " << is_colour << std::endl;
	FilterbankConvolve(m_pInputImage, &m_pFilterbank, &m_pConvVec, m_nGaussScales, m_nGaussOrientations, m_GaussSigmaY,
			m_GaussX2YSigmaRatio, m_nDOGScales, m_DOGExcitSigma, m_DOGInhibSigmaRatio1, m_DOGInhibSigmaRatio2);
	ComputeOrientationEnergy(m_pInputImage, m_pFilterbank, m_pConvVec, &m_pOEVec, m_nGaussScales, m_nGaussOrientations);

	ComputeNonmaximaSuppression(m_pFilterbank, m_pOEVec, &m_pNMSVec, &m_pEnergyVec, &m_pOriVec, &m_pXLocVec, &m_pYLocVec, &m_pErrVec,
			m_nGaussScales, m_nGaussOrientations, m_pInputImage->Width(), m_pInputImage->Height());

	ByteCImgPtr  m_pNMSCombImg = NULL;
	FloatCImgPtr m_pOriCombImg = NULL;
	FloatCImgPtr m_pEnergyCombImg = NULL;
	CombineScales(m_pNMSVec, m_pEnergyVec, m_pOriVec, &m_pNMSCombImg, &m_pOriCombImg, &m_pEnergyCombImg,
			m_pInputImage->Width(), m_pInputImage->Height(), m_nGaussScales);
	ComputeWeberLaw(m_pFilterbank, m_pConvVec, m_TextureWeberLawConst);

	LongCImgPtr m_pTextonsImg = NULL;
	do_textons(m_pConvVec, &m_pTextonsImg, m_nGaussScales, m_nGaussOrientations, m_nDOGScales,
			m_pInputImage->Width(), m_pInputImage->Height(), m_nTextureKMeansK, m_nTextureKMeansIterations);

	CTextureScale* m_pTS = NULL;
    int m_minTextureScale = 0;
    int m_maxTextureScale = 0;
    std::cout << "Compute texture scale img" << std::endl;
	FloatCImgPtr m_pTextureScaleImg = ComputeTextureScale(m_pTextonsImg, &m_pTS, m_pInputImage->Width(), m_pInputImage->Height(),
	    m_nTextureKMeansK, m_TextureMinDist, m_TextureMaxDist, m_TextureAlphaScaleFactor, m_minTextureScale, m_maxTextureScale);
	std::cout << "Compute circle masks" << std::endl;
	CCircleMasks* m_pCircleMasks =  ComputeCircleMasks(m_minTextureScale, m_maxTextureScale);
	std::cout << "Compute ptextrue" << std::endl;
	FloatCImgPtr m_pPTextureImg = ComputePTexture(m_pCircleMasks, m_pTextureScaleImg, m_pTextonsImg, m_pNMSCombImg, m_pOriCombImg,
			m_pInputImage->Width(), m_pInputImage->Height(), m_minTextureScale, m_maxTextureScale,
			m_nTextureKMeansK, m_TextureDiskMiddleWidth, m_TextureTau, m_TextureBeta);
	std::cout << "Compute texton histo img" << std::endl;
	ComputeTextonHistoImg(m_pTextonsImg, m_pTextureScaleImg, m_pPTextureImg, m_pCircleMasks, m_pTextonHistoImg,
			m_pInputImage->Width(), m_pInputImage->Height(), m_nTextureKMeansK);

	std::cout << "Compute color hist masks" << std::endl;
	(*ColorHist) = ComputeColorHistoImg(m_pTextureScaleImg, m_pCircleMasks, m_pInputImage, m_pAImg, m_pBImg,
		m_pInputImage->Width(), m_pInputImage->Height(), m_nBinsA, m_nBinsB, m_nBinsC, m_ColorHistoSoftBinSigma, is_colour);

	std::cout << "Compute dual lattice" << std::endl;
	ComputeDualLattice(m_pEnergyVec, m_pOriVec, m_pNMSVec, m_pXLocVec, m_pYLocVec, m_pPTextureImg, m_pDualHImg, m_pDualVImg,
	    m_pInputImage->Width(), m_pInputImage->Height(), m_nGaussScales, m_EdgelLength, m_InterveningContourSigma);

	(*m_pSparsePatImg) = ComputeSparsePattern(m_SparsePatternDenseRad, m_SparsePatternMaxRad, m_nSparsePatternSamples);

	/** TODO ZAP ALL YOU DO NOT NEED */
    zap(m_pAImg);
	zap(m_pBImg);
	zap(m_pInputImage);
	zap(m_pFilterbank);
	zap(m_pOEVec);
	zap(m_pConvVec);
	zap(m_pNMSVec);
	zap(m_pEnergyVec);
	zap(m_pOriVec);
	zap(m_pXLocVec);
	zap(m_pYLocVec);
	zap(m_pErrVec);
	zap(m_pNMSCombImg);
	zap(m_pOriCombImg);
	zap(m_pEnergyCombImg);
	zap(m_pTextonsImg);
	zap(m_pTS);
	zap(m_pTextureScaleImg);
	zap(m_pCircleMasks);
	zap(m_pPTextureImg);
}

void DTLib::ComputeColorHistoImg
(
	const kjb::Image & img,
    CImg<FloatCHistogramPtr> ** ColorHist,
	int m_nBinsA,
	int m_nBinsB,
	int m_nBinsC,
	float m_ColorHistoSoftBinSigma,
    int m_nCroppedPixels,
    int m_nGaussScales,
    int m_nGaussOrientations,
    float m_GaussSigmaY,
    float m_GaussX2YSigmaRatio,
    int m_nDOGScales,
    float m_DOGExcitSigma,
    float m_DOGInhibSigmaRatio1,
    float m_DOGInhibSigmaRatio2,
    float m_TextureMinDist,
    float m_TextureMaxDist,
    float m_TextureAlphaScaleFactor,
    float m_TextureDiskMiddleWidth
)
{
	FloatCImgPtr m_pAImg = NULL;
	FloatCImgPtr m_pBImg = NULL;
    FloatCImgPtr m_pInputImage = NULL;
	CFilterbank * m_pFilterbank = NULL;
	FloatCImgVecPtr m_pConvVec = NULL;
    FloatCImgPtr m_pTextureScaleImg = NULL;
	CCircleMasks* m_pCircleMasks = NULL;
	CTextureScale* m_pTS = NULL;

	bool m_bColor = false;

    m_pInputImage = convert_kjb_to_CImg(img, &m_pAImg, &m_pBImg, m_nCroppedPixels, m_bColor);

    bool is_colour = true;
	if(kjb_c::is_black_and_white(img.c_ptr()))
	{
		is_colour = false;
	}
    
    //std::cout << " Compute Filter bank " << std::endl;
    //kjb_c::init_cpu_time();
	FilterbankConvolve(m_pInputImage, &m_pFilterbank, &m_pConvVec, 
            m_nGaussScales, m_nGaussOrientations, 
            m_GaussSigmaY, m_GaussX2YSigmaRatio, 
            m_nDOGScales, m_DOGExcitSigma, 
            m_DOGInhibSigmaRatio1, m_DOGInhibSigmaRatio2);
    //double time = kjb_c::get_cpu_time();
    //std::cout << " computing time: " << time/1000.0 << std::endl;

    int m_nTextureKMeansK = 36;
    //int m_nTextureKMeansIterations = 30;
    int m_nTextureKMeansIterations = 10;
	LongCImgPtr m_pTextonsImg = NULL;

    //std::cout << " Compute textons" << std::endl;
    //kjb_c::init_cpu_time();
	do_textons(m_pConvVec, &m_pTextonsImg, 
            m_nGaussScales, m_nGaussOrientations, m_nDOGScales,
			m_pInputImage->Width(), m_pInputImage->Height(), 
            m_nTextureKMeansK, m_nTextureKMeansIterations);

    //time = kjb_c::get_cpu_time();
    //std::cout << " computing time: " << time/1000.0<< std::endl;

    // Compute texture scale 
    int m_minTextureScale = 0;
    int m_maxTextureScale = 0;

    //std::cout << "Compute texture scale img" << std::endl;
    //kjb_c::init_cpu_time();
	m_pTextureScaleImg = ComputeTextureScale(m_pTextonsImg, &m_pTS, 
            m_pInputImage->Width(), m_pInputImage->Height(),
            m_nTextureKMeansK, m_TextureMinDist, m_TextureMaxDist, 
            m_TextureAlphaScaleFactor, m_minTextureScale, m_maxTextureScale);
    //time = kjb_c::get_cpu_time();
    //std::cout << " computing time: " << time/1000.0 << std::endl;

	//std::cout << "Compute circle masks" << std::endl;
    kjb_c::init_cpu_time();
    m_pCircleMasks =  ComputeCircleMasks(m_minTextureScale, m_maxTextureScale);
    //time = kjb_c::get_cpu_time();
    //std::cout << " computing time: " << time/1000.0 << std::endl;

    //std::cout << "Compute Color histogram" << std::endl;
    //kjb_c::init_cpu_time();
	(*ColorHist) = ComputeColorHistoImg(m_pTextureScaleImg, 
            m_pCircleMasks, m_pInputImage, m_pAImg, m_pBImg,
            m_pInputImage->Width(), m_pInputImage->Height(), 
            m_nBinsA, m_nBinsB, m_nBinsC, 
            m_ColorHistoSoftBinSigma, is_colour);
    //time = kjb_c::get_cpu_time();
    //std::cout << " computing time: " << time/1000.0 << std::endl;

    zap(m_pAImg);
    zap(m_pBImg);
    zap(m_pInputImage);
    zap(m_pFilterbank);
    zap(m_pConvVec);
    zap(m_pCircleMasks);
    zap(m_pTextureScaleImg);
    zap(m_pTS);
}


DTLib::CImg<DTLib::FloatCHistogramPtr>* DTLib::ComputeColorHistoImg
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
)
{
	cerr << "Computing image of color histograms" << endl;

	ASSERT(m_pTextureScaleImg != NULL); // input
	ASSERT(m_pCircleMasks != NULL); // input
	ASSERT(m_pAImg != NULL);
	ASSERT(m_pBImg != NULL);

	float minA, maxA, minB, maxB, minC, maxC;
	m_pAImg->MinMax(minA, maxA);
	m_pBImg->MinMax(minB, maxB);
	m_pCImg->MinMax(minC, maxC);

	CImg<FloatCHistogramPtr>* m_pColorHistoImg;
	if(is_colour)
	{
	    m_pColorHistoImg = MakeEmptyColorHistoImg(m_InputWidth, m_InputHeight,
											  m_nBinsA, m_nBinsB, m_nBinsC,
											  minA, maxA, minB, maxB, minC, maxC);

	    ComputeColorHistogramImgNew(m_pCircleMasks, m_pTextureScaleImg,
							 m_pAImg, m_pBImg, m_pCImg, m_pColorHistoImg, NULL,
							 m_ColorHistoSoftBinSigma);
	}
	else
	{
		m_nBinsA = (int)(m_nBinsA*1.5);
		m_pColorHistoImg = MakeBWEmptyColorHistoImg(m_InputWidth, m_InputHeight,
													  m_nBinsA, minA, maxA);

		 ComputeColorHistogramImgBWNew(m_pCircleMasks, m_pTextureScaleImg,
							 m_pAImg, m_pColorHistoImg, NULL,
							 sqrt(m_ColorHistoSoftBinSigma));
	}
    /*ComputeColorHistogramImg2Old(m_pCircleMasks, m_pTextureScaleImg,
			 m_pAImg, m_pBImg, m_pColorHistoImg, NULL,
			 m_ColorHistoSoftBinSigma);*/

	return m_pColorHistoImg;
}


void DTLib::ComputeDualLattice
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
)
{
    ASSERT(m_pEnergyVec != NULL); // input
    ASSERT(m_pOriVec != NULL); // input
    ASSERT(m_pNMSVec != NULL); // input
    ASSERT(m_pXLocVec != NULL); // input
    ASSERT(m_pYLocVec != NULL); // input
    ASSERT(m_pPTextureImg != NULL); // input

    (*m_pDualHImg) = new CImg<float>(m_InputWidth, m_InputHeight);
    (*m_pDualVImg) = new CImg<float>(m_InputWidth, m_InputHeight);

	MakeDualLattice(m_EdgelLength, m_InterveningContourSigma, m_nGaussScales, *m_pNMSVec, *m_pEnergyVec,
                        *m_pOriVec, *m_pXLocVec, *m_pYLocVec, *m_pPTextureImg, **m_pDualHImg, **m_pDualVImg);
}

DTLib::ByteCImgPtr DTLib::ComputeSparsePattern
(
	int m_SparsePatternDenseRad,
	int m_SparsePatternMaxRad,
	int m_nSparsePatternSamples
)
{
    return (ByteCImgPtr) (new CSparsePatImg(m_SparsePatternDenseRad, m_SparsePatternMaxRad,
                              m_nSparsePatternSamples, true, true));
}

void DTLib::PrepareHeatMap
(
	kjb::Image & img,
	CImg<FloatCHistogramPtr>* pHistoImg,
	int padding,
	int x_position,
	int y_position
)
{
    //img.create_zero_image(pHistoImg->Height() + (padding*2), pHistoImg->Width() + (padding*2));
	x_position -= padding;
    y_position -= padding;
    if((x_position < 0) || (y_position < 0))
    {
    	std::cout << "Requested pixel is part of the padding" << std::endl;
    	return;
    }

    if((x_position >= pHistoImg->Width()) || (y_position >= pHistoImg->Height()))
    {
    	std::cout << "Requested pixel is part of the padding" << std::endl;
    	return;
    }

    int position = (y_position*pHistoImg->Width()) + x_position;
    FloatCHistogramPtr ppHisto = pHistoImg->GetPix(position);

    for(unsigned int i = 0; i < pHistoImg->Height(); i++ )
    {
    	for(unsigned int j = 0; j < pHistoImg->Width(); j++ )
    	{
    		int compare_position = (i*pHistoImg->Width()) + j;
    		FloatCHistogramPtr compareHisto = pHistoImg->GetPix(compare_position);
    		double color = compareHisto->ChiSquareCompare(ppHisto);
    		//double color = compareHisto->NormalCompare(ppHisto);
    		img(i + padding, j + padding, 0) = (1.0 - color)*255.0;
    	}
    }

}

void DTLib::Compute_All_Histograms
(
    const kjb::Image & img,
    CImg<FloatCHistogramPtr> ** m_pTextonHistoImg,
    CImg<FloatCHistogramPtr> ** ColorHist
)
{
    FloatCImgPtr m_pDualHImg = NULL;
    FloatCImgPtr m_pDualVImg = NULL;
    ByteCImgPtr m_pSparsePatImg = NULL;
	int m_nGaussScales = 4;
	int m_nGaussOrientations = 12;
	float m_GaussSigmaY = 1.41;
	float m_GaussX2YSigmaRatio = 3.0;
	int m_nDOGScales = 4;
	float m_DOGExcitSigma = 1.41;
	float m_DOGInhibSigmaRatio1 = 0.62;
	float m_DOGInhibSigmaRatio2 = 1.6;
    int m_nCroppedPixels = 10;
    float m_TextureWeberLawConst = 0.01;
    int m_nTextureKMeansK = 36;
    int m_nTextureKMeansIterations = 30;
    float m_TextureMinDist = 3.0;
    float m_TextureMaxDist = 0.1;
    float m_TextureAlphaScaleFactor = 1.5;
	float m_TextureDiskMiddleWidth = 3.0;
	float m_TextureTau = 0.3;
	float m_TextureBeta = 0.04;
	int m_nBinsA = 8;
	int m_nBinsB = 8;
	int m_nBinsC = 8;
	float m_ColorHistoSoftBinSigma = 1.8;
	float m_EdgelLength = 2.0;
	float m_InterveningContourSigma = 0.016;
	int m_SparsePatternDenseRad = 10;
	int m_SparsePatternMaxRad = 30;
	int m_nSparsePatternSamples = 400;
	bool m_bColor = false;
    Compute_Textons_Histograms(img, m_pTextonHistoImg, ColorHist, &m_pDualHImg, &m_pDualVImg, & m_pSparsePatImg,
    	m_nGaussScales, m_nGaussOrientations, m_GaussSigmaY, m_GaussX2YSigmaRatio, m_nDOGScales,
    	m_DOGExcitSigma, m_DOGInhibSigmaRatio1, m_DOGInhibSigmaRatio2, m_nCroppedPixels, m_TextureWeberLawConst,
        m_nTextureKMeansK, m_nTextureKMeansIterations, m_TextureMinDist, m_TextureMaxDist,
        m_TextureAlphaScaleFactor, m_TextureDiskMiddleWidth, m_TextureTau, m_TextureBeta,
        m_nBinsA, m_nBinsB, m_nBinsC, m_ColorHistoSoftBinSigma, m_EdgelLength, m_InterveningContourSigma,
    	m_SparsePatternDenseRad, m_SparsePatternMaxRad, m_nSparsePatternSamples, m_bColor);

    //zap(m_pTextonHistoImg);
    //zap(ColorHist);
    zap(m_pDualHImg);
    zap(m_pDualVImg);
    zap(m_pSparsePatImg);
}

