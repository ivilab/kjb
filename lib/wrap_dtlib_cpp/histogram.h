/////////////////////////////////////////////////////////////////////////////
// histogram.h - generic histogram functionality template class
// Author: Doron Tal
// Date Created: June, 1996
// Date Last Modified: Aug 5, 2000
//
// Both interface and implementation are in this file.  tested only on
// float and unsigned char template arguments.

#ifndef _HISTOGRAM_H
#define _HISTOGRAM_H

#include <assert.h>
#include "wrap_dtlib_cpp/img.h"
#include "wrap_dtlib_cpp/utils.h"
#include "string"
#include <fstream>

namespace DTLib {

    template <class T> class CHistogram
    {
    public:
        CHistogram();

        CHistogram(const std::string & file_name);
        CHistogram(std::istream & ifs);
        ~CHistogram();

        void GetValues(float * pointer);

        void SetValues(float * pointer);

        // if you use this setup function, you can't call Update(),
        // but you must call IncrementBin() instead.
        void Setup(const int& nBins);

        // Initializes the histogram, allocating and zeroing bins.
        // 1-D version
        void Setup(const int& nBins, const T& RangeMin, const T& RangeMax);

        // 2-D version of above
        void Setup(const int& nBinsX, const int& nBinsY, const T& XRangeMin,
                   const T& XRangeMax, const T& YRangeMin, const T& YRangeMax);

        // 3-D version of above
	   void Setup(const int& nBinsX, const int& nBinsY, const int& nBinsZ, const T& XRangeMin,
				  const T& XRangeMax, const T& YRangeMin, const T& YRangeMax, const T& ZRangeMin, const T& ZRangeMax);

        // Increment one of the histogram bins in a 1-D histogram acc. to Val
        // i.e. first map from Val into the bin's index, then increment that
        // bin by 1.
        void Update(const T& Val);

        // same as above, but in 2-D
        void Update(const T& XVal, const T& YVal);

        void Read(const std::string & file_name);

        void Read(std::istream & ifs);

        void Write(const std::string & file_name) const;

        void Write(std::ostream & ofs) const;

        // 2-D histogram soft binning: smooth before you bin and vote into
        // neighboring bins according to your smoothing function, which is
        // a Kernel provided by 'SmoothFun' (a float Img object), centered
        // at the bin in which ('XVal', 'YVal') falls, this Img object
        // is square, i.e. Width == Height and its width is odd.
        void SoftUpdate(const T& XVal, const T& YVal, const float& Sigma);

        void SoftUpdate1D(const T& XVal, const float& Sigma);

        // #-D histogram soft binning: smooth before you bin and vote into
	    // neighboring bins according to your smoothing function, which is
	    // a Kernel provided by 'SmoothFun' (a float Img object), centered
	    // at the bin in which ('XVal', 'YVal') falls, this Img object
	    // is square, i.e. Width == Height and its width is odd.
	    void SoftUpdate3D(const T& XVal, const T& YVal, const T& ZVal, const float& Sigma);

//////////////////////////////////////////////////////////////////////////////////////
//Added by Prasad
        // Same as above but avoids iterative exponential calculations
        // because it accepts Gaussian Kernel as a function parameter
        void SoftUpdate_fast(const T& XVal, const T& YVal, const int& Rad, const FloatCImgPtr pGauss_Window);
//////////////////////////////////////////////////////////////////////////////////////

        // update a certain bin, indexed by iBin, by incrementing its
        // value by Val
        void IncrementBin(const int& iBin, const T& Val);

        // Normalize the histogram so that its area = 'Area'
        void Normalize(const float& Area = 1.0f);

        // Normalize the histogram so that its maximum element = 'Max'
        void NormalizeToMax(const float& Max = 255.0f);

        // Return the Prob, 1-D Version, the returned Value is just the number
        // of hits in the bin that corresponds to the range containing 'Val'
        float GetProb(const T& Val);

        // 2-D Version
        float GetProb(const T& XVal, const T& YVal);

        // precond: must be normalized first
        inline float GetProbOfBin(int iBin) { return m_pBins[iBin]; };

        // Use this histogram to get a probability distribution score-
        // map, i.e. run through Img 'InImg' (in the 1-D case, that
        // is.. if you're dealing with 2-D histograms, then you'll be
        // running through Imgs InImg1 and InImg2 simulataneously),
        // and get the probability associated with each pixel (which is
        // this histogram's number of hits in the bin that corresponds to
        // the range containing each pixel's Value.  Return all these
        // probabilities in the Img 'PDistImg'
        // PRECOND: (1)  InImg1 and InImg2 and PDistImg have the same ROI;
        // (2) Normalize() has been called on 'this'
        // 1-D version
        void GetProbDist(CImg<T>& InImg, CImg<float>& PDistImg);

        // 2-D version
        void GetProbDist(CImg<T>& InImg1, CImg<T>& InImg2,
                         CImg<float>& PDistImg);

        // Zero histogram bins
        void Zero();

        // return true if the histogram's bins are all zero, false otherwise
        bool IsEmpty();

        // return the maximum value of each bin
        float MaxBinVal();

        // Subtract the Value 'Value' from each bin, rectifying at 0.
        void AdjustDown(const float& Value);

        void Add(const CHistogram<T>& OtherHisto);

        void ScalarMultiply(const float& Multiplier);

        // show the histogram in an Img. if the histogram is 1-D, a
        // bar-graph is shown (a binary Img, x-axis is bin index,
        // y-axis is number of hits, and the Height of each bar represents
        // the number of hits); if the histogram is 2-D, then the Img
        // is broken up into boxes, each corresponding to a bin.  The
        // number of hits per bin of the 2-D histograms displayed is
        // represented by the grayscale Value (brighter = more hits/bin).
        void Display(CImg<BYTE>& InImg);

        // given the bin #, returns the range associated with that bin#
        // (but doesn't return full range, only lower boudns on the range,
        // to get the upper bound, just call this again with 'BinIndex'+1
        // as the argument.
        T BinIndexToValue(const int& BinIndex);

        // compare with another histogram via chi square
        float ChiSquareCompare(CHistogram<T> *OtherHisto);

        // compare with another histogram via chi square
        float NormalCompare(CHistogram<T> *OtherHisto);

        int nBins() { return m_nBins; }
        float* pBins() { return m_pBins; }

        // ***TODO***: make member variables protected and
        // use access functions
    public:
        int m_nBinsX;
        int m_nBinsY;
        int m_nBinsZ;
        int m_nBins; // # of elements (size) of histogram
        T m_XRangeMin;
        T m_XRangeMax;
        T m_YRangeMin;
        T m_YRangeMax;
        T m_ZRangeMin;
        T m_ZRangeMax;
        float *m_pBins; // the histogram (1d or 2d)
        float m_XFactor;
        float m_YFactor;
        float m_ZFactor;
    };

    ///////////////////////////////////////////////////////////////////////////
    /// TYPES
    ///////////////////////////////////////////////////////////////////////////

    typedef CHistogram<float>* FloatCHistogramPtr;
    typedef CHistogram<int>* IntCHistogramPtr;
    typedef CHistogram<long>* LongCHistogramPtr;

    ///////////////////////////////////////////////////////////////////////////
    // END OF INTERFACE -- IMPLEMENTATION STARTS HERE:
    ///////////////////////////////////////////////////////////////////////////

    template <class T>
    CHistogram<T>::CHistogram()
    {
        m_pBins = NULL;
    }

    template <class T>
    CHistogram<T>::CHistogram(const std::string & file_name)
    {
    	m_pBins = NULL;
    	Read(file_name);
    }

    template <class T>
    CHistogram<T>::CHistogram(std::istream & ifs)
    {
    	m_pBins = NULL;
    	Read(ifs);
    }

    ///////////////////////////////////////////////////////////////////////////

    template <class T>
    CHistogram<T>::~CHistogram()
    {
        zap(m_pBins);
    }

    template <class T>
	void CHistogram<T>::GetValues(float * pointer)
	{
    	for(unsigned int i = 0; i < m_nBins; i++)
    	{
    		pointer[i] = m_pBins[i];
    	}
	}

    template <class T>
   	void CHistogram<T>::SetValues(float * pointer)
   	{
    	for(unsigned int i = 0; i < m_nBins; i++)
    	{
    		m_pBins[i] = pointer[i];
    	}
   	}

    template <class T>
    void CHistogram<T>::Read(const std::string & file_name)
    {
        std::ifstream ifs(file_name.c_str());
        Read(ifs);
        ifs.close();
    }

    template <class T>
    void CHistogram<T>::Read(std::istream & ifs)
    {
    	if(m_pBins)
    	{
    		zap(m_pBins);
    	}
    	m_pBins = NULL;
    	ifs >> m_nBins;
        ifs >> m_nBinsX;
        ifs >> m_nBinsY;
        ifs >> m_XRangeMin;
        ifs >> m_XRangeMax;
        ifs >> m_YRangeMin;
        ifs >> m_YRangeMax;
        ifs >> m_XFactor;
        ifs >> m_YFactor;

        if(m_nBinsY == 0)
        {
        	assert(m_nBinsX == m_nBins);
        }
        else
        {
        	assert( (m_nBinsX*m_nBinsY) == m_nBins);
        }

        if(m_nBins != 0)
        {
            m_pBins = new float[m_nBins];
        }
        float tempf = 0.0;
        for(unsigned int i = 0; i < m_nBins; i++)
        {
        	ifs >> tempf;
        	m_pBins[i] = tempf;
        }
    }

    template <class T>
    void CHistogram<T>::Write(const std::string & file_name) const
    {
        using namespace std;
        ofstream ofs(file_name.c_str());
        Write(ofs);
        ofs.close();
    }

    template <class T>
    void CHistogram<T>::Write(std::ostream & ofs) const
    {
        ofs << m_nBins << std::endl;
        ofs << m_nBinsX << std::endl;
        ofs << m_nBinsY << std::endl;
        ofs << m_XRangeMin << std::endl;
        ofs << m_XRangeMax << std::endl;
        ofs << m_YRangeMin << std::endl;
        ofs << m_YRangeMax << std::endl;
        ofs << m_XFactor << std::endl;
        ofs << m_YFactor << std::endl;
        for(unsigned int i = 0; i < m_nBins; i++)
        {
        	ofs << m_pBins[i] << " ";
        }
        ofs << std::endl;
    }

    ///////////////////////////////////////////////////////////////////////////

    template <class T>
    void CHistogram<T>::Setup(const int& nBins)
    {
        zap(m_pBins);

        m_nBinsX = m_nBins = nBins;
        m_nBinsY = 0;
        m_nBinsZ = 0;
        m_XRangeMin = -1.0f;
        m_XRangeMax = -1.0f;
        m_YRangeMin = -1.0f;
        m_YRangeMax = -1.0f;
        m_ZRangeMin = -1.0f;
	    m_ZRangeMax = -1.0f;
        m_XFactor = -1.0f;
        m_YFactor = -1.0f;
        m_pBins = new float[m_nBins];
        Zero();
    }


    // 1-D version
    template <class T>
    void CHistogram<T>::Setup(const int& nBins, const T& RangeMin,
                              const T& RangeMax)
    {
        zap(m_pBins);
        m_nBinsX = m_nBins = nBins;
        m_nBinsY = 0;
        m_XRangeMin = RangeMin;
        m_XRangeMax = RangeMax;

        // cout << "SETUP" << RangeMin << " " << RangeMax << endl;

        const float Range = (float)(RangeMax-RangeMin);
        m_XFactor = (float)(nBins-1)/Range;
        m_YFactor = 0.0f;
        m_pBins = new float[m_nBins];
        Zero();
    }

    // 3-D version
    template <class T>
    void CHistogram<T>::Setup(const int& nBinsX, const int& nBinsY, const int& nBinsZ,
                              const T& XRangeMin, const T& XRangeMax,
                              const T& YRangeMin, const T& YRangeMax,
                              const T& ZRangeMin, const T& ZRangeMax)
    {
        zap(m_pBins);
        m_nBinsX = nBinsX;
        m_nBinsY = nBinsY;
        m_nBinsZ = nBinsZ;
        m_XRangeMin = XRangeMin;
        m_XRangeMax = XRangeMax;
        m_YRangeMin = YRangeMin;
        m_YRangeMax = YRangeMax;
        m_ZRangeMin = ZRangeMin;
	    m_ZRangeMax = ZRangeMax;
        const float XRange = (float)(m_XRangeMax-m_XRangeMin);
        const float YRange = (float)(m_YRangeMax-m_YRangeMin);
        const float ZRange = (float)(m_ZRangeMax-m_ZRangeMin);
        m_XFactor = (float)(nBinsX-1)/XRange;
        m_YFactor = (float)(nBinsY-1)/YRange;
        m_ZFactor = (float)(nBinsZ-1)/ZRange;
        m_nBins = nBinsX*nBinsY*nBinsZ;
        m_pBins = new float[m_nBins];
        Zero();
    }

    // 2-D version
    template <class T>
    void CHistogram<T>::Setup(const int& nBinsX, const int& nBinsY,
                              const T& XRangeMin, const T& XRangeMax,
                              const T& YRangeMin, const T& YRangeMax)
    {
        zap(m_pBins);
        m_nBinsX = nBinsX;
        m_nBinsY = nBinsY;
        m_nBinsZ = 0;
        m_XRangeMin = XRangeMin;
        m_XRangeMax = XRangeMax;
        m_YRangeMin = YRangeMin;
        m_YRangeMax = YRangeMax;
        m_ZRangeMin = -1.0f;
	    m_ZRangeMax = -1.0f;
        const float XRange = (float)(m_XRangeMax-m_XRangeMin);
        const float YRange = (float)(m_YRangeMax-m_YRangeMin);
        m_XFactor = (float)(nBinsX-1)/XRange;
        m_YFactor = (float)(nBinsY-1)/YRange;
        m_nBins = nBinsX*nBinsY;
        m_pBins = new float[m_nBins];
        Zero();
    }

    ///////////////////////////////////////////////////////////////////////////

    // 1-D version
    template <class T>
    inline void CHistogram<T>::Update(const T& Val)
    {
        assert(Val >= m_XRangeMin);
        assert(Val <= m_XRangeMax);
        (*(m_pBins+F2I((float)(Val-m_XRangeMin)*m_XFactor)))++;
    }

    // 2-D version
    template <class T>
    inline void CHistogram<T>::Update(const T& XVal, const T& YVal)
    {
        assert(XVal >= m_XRangeMin);
        assert(XVal <= m_XRangeMax);
        assert(YVal >= m_YRangeMin);
        assert(YVal <= m_YRangeMax);

        const int X = F2I((float)(XVal-(float)m_XRangeMin)*m_XFactor);
        const int Y = F2I((float)(YVal-(float)m_YRangeMin)*m_YFactor);
        (*(m_pBins+Y*m_nBinsX+X))++;
    }

    ///////////////////////////////////////////////////////////////////////////
    // 2-D histogram soft binning
    // ARGUMENTS:
    // ----------
    // 'XVal' - value along first dimension
    // 'YVal' - value along second dimension
    // 'Sigma' - Sigma of Gaussian Kernel, in units of this histogram's
    //           bin width

    template <class T>
    inline void CHistogram<T>::SoftUpdate(const T& XVal, const T& YVal,
                                          const float& Sigma)
    {
       //Debugging
       //if (XVal < m_XRangeMin || XVal > m_XRangeMax || YVal < m_YRangeMin || YVal > m_YRangeMax){
           //cout << " Faulty A value = " << XVal << endl;
           //cout << " Faulty B value = " << YVal << endl;
       //}// if (XVal....


        assert(XVal >= m_XRangeMin);
        assert(XVal <= m_XRangeMax);
        assert(YVal >= m_YRangeMin);
        assert(YVal <= m_YRangeMax);

        // RealX and RealY are how far we are from the zero of
        // each dimension, i.e.  (RealX, RealY) is the floating point
        // coordinate of which bin we fell into
        const float RealX = (XVal-m_XRangeMin)*m_XFactor;
        const float RealY = (YVal-m_YRangeMin)*m_YFactor;

        // we need to decide where to CENTER the Gaussian kernel's
        // window, so for our (integer) bin coordinates we choose the
        // integer coordinates closest to (RealX, RealY):
        const int xw_center = F2I(RealX);
        const int yw_center = F2I(RealY);

        // now we need to determine a window width over which
        // we'll soft-bin, the radius of this window should
        // be enough to encompass most of our Gaussian kernel,
        // so we'll use 3.0 * Sigma.  (FWHM=2.35sigma)
        const int Rad = F2I(3.0f*Sigma);

        const float RecipSigmaSqr = 1.0f/SQR(Sigma);
        for (int yw = -Rad; yw <= Rad; yw++) {
            for (int xw = -Rad; xw <= Rad; xw++) {
                const int binXcoord = xw_center+xw;
                const int binYcoord = yw_center+yw;
                if ((binXcoord >= 0) && (binXcoord < m_nBinsX) &&
                    (binYcoord >= 0) && (binYcoord < m_nBinsY)) {
                    const float DeltaX = RealX-(float)binXcoord;
                    const float DeltaY = RealY-(float)binYcoord;
                    const float SquaredSum =  SQR(DeltaX)+SQR(DeltaY);
                    const float Weight = exp(-SquaredSum*RecipSigmaSqr);
                    const int iBin = binYcoord*m_nBinsX+binXcoord;
                    m_pBins[iBin] += Weight;
                } // if
            } // for xw
        } // for yw
    }

    template <class T>
    inline void CHistogram<T>::SoftUpdate1D(const T& XVal, const float& Sigma)
    {
       //Debugging
       //if (XVal < m_XRangeMin || XVal > m_XRangeMax || YVal < m_YRangeMin || YVal > m_YRangeMax){
           //cout << " Faulty A value = " << XVal << endl;
           //cout << " Faulty B value = " << YVal << endl;
       //}// if (XVal....


        assert(XVal >= m_XRangeMin);
        assert(XVal <= m_XRangeMax);

        // RealX and RealY are how far we are from the zero of
        // each dimension, i.e.  (RealX, RealY) is the floating point
        // coordinate of which bin we fell into
        const float RealX = (XVal-m_XRangeMin)*m_XFactor;

        // we need to decide where to CENTER the Gaussian kernel's
        // window, so for our (integer) bin coordinates we choose the
        // integer coordinates closest to (RealX, RealY):
        const int xw_center = F2I(RealX);

        // now we need to determine a window width over which
        // we'll soft-bin, the radius of this window should
        // be enough to encompass most of our Gaussian kernel,
        // so we'll use 3.0 * Sigma.  (FWHM=2.35sigma)
        const int Rad = F2I(3.0f*Sigma);

        const float RecipSigmaSqr = 1.0f/Sigma;
		for (int xw = -Rad; xw <= Rad; xw++) {
			const int binXcoord = xw_center+xw;
			if ((binXcoord >= 0) && (binXcoord < m_nBinsX)) {
				const float DeltaX = RealX-(float)binXcoord;
				const float SquaredSum =  SQR(DeltaX);
				const float Weight = exp(-SquaredSum*RecipSigmaSqr);
				const int iBin = binXcoord;
				m_pBins[iBin] += Weight;
			} // if
		} // for xw
    }

    template <class T>
    inline void CHistogram<T>::SoftUpdate3D(const T& XVal, const T& YVal, const T& ZVal,
                                          const float& Sigma)
    {
       //Debugging
       //if (XVal < m_XRangeMin || XVal > m_XRangeMax || YVal < m_YRangeMin || YVal > m_YRangeMax){
           //cout << " Faulty A value = " << XVal << endl;
           //cout << " Faulty B value = " << YVal << endl;
       //}// if (XVal....


        assert(XVal >= m_XRangeMin);
        assert(XVal <= m_XRangeMax);
        assert(YVal >= m_YRangeMin);
        assert(YVal <= m_YRangeMax);
        assert(ZVal >= m_ZRangeMin);
        assert(ZVal <= m_ZRangeMax);

        // RealX and RealY are how far we are from the zero of
        // each dimension, i.e.  (RealX, RealY) is the floating point
        // coordinate of which bin we fell into
        const float RealX = (XVal-m_XRangeMin)*m_XFactor;
        const float RealY = (YVal-m_YRangeMin)*m_YFactor;
        const float RealZ = (ZVal-m_ZRangeMin)*m_ZFactor;

        // we need to decide where to CENTER the Gaussian kernel's
        // window, so for our (integer) bin coordinates we choose the
        // integer coordinates closest to (RealX, RealY):
        const int xw_center = F2I(RealX);
        const int yw_center = F2I(RealY);
        const int zw_center = F2I(RealZ);

        // now we need to determine a window width over which
        // we'll soft-bin, the radius of this window should
        // be enough to encompass most of our Gaussian kernel,
        // so we'll use 3.0 * Sigma.  (FWHM=2.35sigma)
        const int Rad = F2I(3.0f*Sigma);

        //const float RecipSigmaSqr = 1.0f/SQR(Sigma);
        const float RecipSigmaSqr = 1.0f/pow((double) Sigma,1.0/3.0);
        for (int yw = -Rad; yw <= Rad; yw++) {
            for (int xw = -Rad; xw <= Rad; xw++) {
                for (int zw = -Rad; zw <= Rad; zw++) {
                    const int binXcoord = xw_center+xw;
                    const int binYcoord = yw_center+yw;
                    const int binZcoord = zw_center+zw;
                    if ((binXcoord >= 0) && (binXcoord < m_nBinsX) &&
                        (binYcoord >= 0) && (binYcoord < m_nBinsY) &&
                        (binZcoord >= 0) && (binZcoord < m_nBinsZ)) {
                        const float DeltaX = RealX-(float)binXcoord;
                        const float DeltaY = RealY-(float)binYcoord;
                        const float DeltaZ = RealZ-(float)binZcoord;
                        const float SquaredSum =  SQR(DeltaX)+SQR(DeltaY)+SQR(DeltaZ);
                        const float Weight = exp(-SquaredSum*RecipSigmaSqr);
                        const int iBin = m_nBinsZ*(binYcoord*m_nBinsX) + (binXcoord)*m_nBinsZ + binZcoord;
                        m_pBins[iBin] += Weight;
                    } // if
                } // for zw
            } // for xw
        } // for yw
    }


/////////////////////////////////////////////////////////////////////////////////////////
//Added by Prasad
//This routine does 2-D histogram soft-binning but does not involve the Gaussian
//exponential calculations within it. It instead accepts the Gaussian weight values
//as a function parameter

    // 2-D histogram soft binning
    // ARGUMENTS:
    // ----------
    // 'XVal' - value along first dimension
    // 'YVal' - value along second dimension
    // 'Rad' - Radius of Gaussian Kernel
    // 'pGauss_Window' - Matrix containing the Gaussian weight

    template <class T>
    inline void CHistogram<T>::SoftUpdate_fast(const T& XVal, const T& YVal,
                                          const int& Rad, const FloatCImgPtr pGauss_Window)
    {
       //Debugging
       //if (XVal < m_XRangeMin || XVal > m_XRangeMax || YVal < m_YRangeMin || YVal > m_YRangeMax){
           //cout << " Faulty A value = " << XVal << endl;
           //cout << " Faulty B value = " << YVal << endl;
       //}// if (XVal....


        assert(XVal >= m_XRangeMin);
        assert(XVal <= m_XRangeMax);
        assert(YVal >= m_YRangeMin);
        assert(YVal <= m_YRangeMax);

        // RealX and RealY are how far we are from the zero of
        // each dimension, i.e.  (RealX, RealY) is the floating point
        // coordinate of which bin we fell into
        const float RealX = (XVal-m_XRangeMin)*m_XFactor;
        const float RealY = (YVal-m_YRangeMin)*m_YFactor;

        // we need to decide where to CENTER the Gaussian kernel's
        // window, so for our (integer) bin coordinates we choose the
        // integer coordinates closest to (RealX, RealY):
        const int xw_center = F2I(RealX);
        const int yw_center = F2I(RealY);

        // now we need to determine a window width over which
        // we'll soft-bin, the radius of this window should
        // be enough to encompass most of our Gaussian kernel,
        // so we'll use 3.0 * Sigma.  (FWHM=2.35sigma)
        //const int Rad = F2I(3.0f*Sigma);

        //const float RecipSigmaSqr = 1.0f/SQR(Sigma);
        float* pWindow = pGauss_Window->pBuffer();

       for (int yw = -Rad; yw <= Rad; yw++) {
            for (int xw = -Rad; xw <= Rad; xw++, pWindow++) {
                const int binXcoord = xw_center+xw;
                const int binYcoord = yw_center+yw;
                if ((binXcoord >= 0) && (binXcoord < m_nBinsX) &&
                    (binYcoord >= 0) && (binYcoord < m_nBinsY)) {
                    //const float DeltaX = RealX-(float)binXcoord;
                    //const float DeltaY = RealY-(float)binYcoord;
                    //const float SquaredSum =  SQR(DeltaX)+SQR(DeltaY);
                    //const float Weight = exp(-SquaredSum*RecipSigmaSqr);
                    const int iBin = binYcoord*m_nBinsX+binXcoord;
                    m_pBins[iBin] += *pWindow;
                } // if
            } // for xw
        } // for yw
    }

/////////////////////////////////////////////////////////////////////////////////////////


    ///////////////////////////////////////////////////////////////////////////
    // Update the i'th bin 'iBin' by incrementing it by a value 'Val'

    template<class T>
    inline void CHistogram<T>::IncrementBin(const int& iBin, const T& Val)
    {
        *(m_pBins+iBin) += Val;
    }

    ///////////////////////////////////////////////////////////////////////////

    template <class T>
    void CHistogram<T>::Normalize(const float& Area)
    {
        // doesn't rely on anything but m_nBins and m_pBins
        T *pBin = m_pBins;
        float EltSum = 0.0f;
        int i;
        for (i = 0; i < m_nBins; i++, pBin++) {
            assert(*pBin >= 0.0f);
            EltSum += (float)*pBin;
        }
        // assert(EltSum >= 0.0f);

        if (EltSum > 0.0f) {
            const float Factor = Area/EltSum;

            pBin = m_pBins;
            for (i = 0; i < m_nBins; i++, pBin++) {
                *pBin = (T)((float)*pBin*(float)Factor);
            }
        }
    }

    ///////////////////////////////////////////////////////////////////////////

    template <class T>
    void CHistogram<T>::NormalizeToMax(const float& NMax)
    {
        const float HistoMax = Max(m_pBins, m_nBins);
        const float Factor = NMax/HistoMax;
        float *pBin = m_pBins;
        for (int i = 0; i < m_nBins; i++, pBin++)
            *pBin *= Factor;
    }

    ///////////////////////////////////////////////////////////////////////////

    // 1-D version
    template <class T>
    float CHistogram<T>::GetProb(const T& Val)
    {
        return *(m_pBins+F2I((float)(Val-m_XRangeMin)*m_XFactor));
    }

    // 2-D version
    template <class T>
    float CHistogram<T>::GetProb(const T& XVal, const T& YVal)
    {
        const int X = F2I((float)(XVal-m_XRangeMin)*m_XFactor);
        const int Y = F2I((float)(YVal-m_YRangeMin)*m_YFactor);
        return *(m_pBins+Y*m_nBinsX+X);
    }

    ///////////////////////////////////////////////////////////////////////////

    template <class T>
    void CHistogram<T>::GetProbDist(CImg<T>& InImg, CImg<float>& PDistImg)
    {
        PDistImg.SetROIVal(0.0f); // zero out initially
        const int ystart = InImg.ROIStartY(), yend = InImg.ROIEndY(),
            xstart = InImg.ROIStartX(), xend = InImg.ROIEndX(),
            sk = InImg.ROISkipCols();
        T *pBuf = InImg.pROI();
        float *pDistBuf = PDistImg.pROI();
        for (int y = ystart; y < yend; y++, pBuf += sk, pDistBuf += sk)
            for (int x = xstart; x < xend; x++, pBuf++, pDistBuf++)
                *pDistBuf = GetProb(*pBuf);
    }

    ///////////////////////////////////////////////////////////////////////////

    template <class T>
    void CHistogram<T>::GetProbDist(CImg<T>& InImg1,  CImg<T>& InImg2,
                                    CImg<float>& PDistImg)
    {
        PDistImg.SetROIVal(0.0f); // zero out initially
        const int ystart = InImg1.ROIStartY(), yend = InImg1.ROIEndY(),
            xstart = InImg1.ROIStartX(), xend = InImg1.ROIEndX(),
            sk = InImg1.ROISkipCols();
        T *pBuf1 = InImg1.pROI(), *pBuf2 = InImg2.pROI();
        float *pDistBuf = PDistImg.pROI();
        for (int y = ystart; y < yend;
             y++, pBuf1 += sk, pBuf2 += sk, pDistBuf += sk)
            for (int x = xstart; x < xend;
                 x++, pBuf1++, pBuf2++, pDistBuf++)
                *pDistBuf = GetProb(*pBuf1, *pBuf2);
    }

    ///////////////////////////////////////////////////////////////////////////

    template <class T>
    void CHistogram<T>::Zero()
    {
        memset(m_pBins, 0, sizeof(float)*m_nBins);
    }

    ///////////////////////////////////////////////////////////////////////////

    template <class T>
    bool CHistogram<T>::IsEmpty()
    {
        bool bIsEmpty = true;
        for (int i = 0; i < m_nBins; i++) {
            if (m_pBins[i] != (T)0) {
                bIsEmpty = false;
                break;
            }
        }
        return bIsEmpty;
    }

    ///////////////////////////////////////////////////////////////////////////

    template <class T>
    float CHistogram<T>::MaxBinVal()
    {
        float MaxVal = CONST_MIN_FLOAT;
        for (int i = 0; i < m_nBins; i++)
            if (m_pBins[i] > MaxVal)
                MaxVal = m_pBins[i];
        return MaxVal;
    }

    ///////////////////////////////////////////////////////////////////////////

    template <class T>
    void CHistogram<T>::AdjustDown(const float& Value)
    {
        float *pBin = m_pBins;
        for (int i = 0; i < m_nBins; i++, pBin++) {
            *pBin -= Value;
            if (*pBin < 0.0f) *pBin = 0.0f;
        }
    }

    ///////////////////////////////////////////////////////////////////////////

    template <class T>
    void CHistogram<T>::Add(const CHistogram<T>& OtherHisto)
    {
        T *pBin = m_pBins;
        T *pOtherBin = OtherHisto.m_pBins;
        int nElts = m_nBins;
        for (int i = 0; i < nElts; i++, pBin++, pOtherBin++)
            *pBin += *pOtherBin;
    }

    ///////////////////////////////////////////////////////////////////////////

    template <class T>
    void CHistogram<T>::ScalarMultiply(const float& Multiplier)
    {
        T *pBin = m_pBins;
        int nElts = m_nBins;
        for (int i = 0; i < nElts; i++, pBin++)
            *pBin *= Multiplier;
    }

    ///////////////////////////////////////////////////////////////////////////

    template <class T>
    T CHistogram<T>::BinIndexToValue(const int& BinIndex)
    {
        return (T)((float)(m_XRangeMax-m_XRangeMin)*(float)BinIndex/
                   (float)m_nBinsX+(float)m_XRangeMin);
    }

    ///////////////////////////////////////////////////////////////////////////

    template <class T>
    float CHistogram<T>::ChiSquareCompare(CHistogram<T> *OtherHisto)
    {
        T *pBin = m_pBins;
        T *pOtherBin = OtherHisto->m_pBins;
        float Sum = 0.0f;
        for (int i = 0; i < m_nBins; i++, pBin++, pOtherBin++) {
            float Val = (float)*pBin;
            float OtherVal = (float)*pOtherBin;
            float diffsqr = Val-OtherVal; diffsqr *= diffsqr;
            float ValSum = Val+OtherVal;
            if (ValSum != 0.0f) Sum += diffsqr/ValSum;
        } // for (int i = 0; i < m_nBins; i++, pBin++, pOtherBin++) {
        Sum *= 0.5f;
        // assert((Sum >= 0.0f) && (Sum <= 1.0f));
        // assert(Sum >= 0.0f);
        if (Sum > 1.0f) {
            // cout << Sum << endl;
            assert(Sum < 1.001f);
            Sum = 1.0f;
        }
        return Sum;
    }

    // compare with another histogram via normal compare
    template <class T>
    float CHistogram<T>:: NormalCompare(CHistogram<T> *OtherHisto)
    {
    	double total = 0.0;
    	for(int i = 0; i < m_nBins; i++)
    	{
    		double val1 = (float)m_pBins[i];
    		double val2 = (float)OtherHisto->m_pBins[i];
    		double diff = fabs(val1 - val2);
    		total += diff;
    	}
    	total /= m_nBins;
        return float(total);
    }

} // namespace DTLib {

#endif /* #ifndef _HISTOGRAM_H */
