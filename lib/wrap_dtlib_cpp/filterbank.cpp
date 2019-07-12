/////////////////////////////////////////////////////////////////////////////
// filterbank.cpp - filterbank convolution class
// Author: Doron Tal
// Date created: April, 2000

#include "l/l_sys_debug.h"  /* For ASSERT. */
#include <iostream>
#include "wrap_dtlib_cpp/jpeg.h"
#include "wrap_dtlib_cpp/filterbank.h"
#include "wrap_dtlib_cpp/gausskernel.h"
#include "wrap_dtlib_cpp/dog.h"
#include "wrap_dtlib_cpp/img.h"
#include "wrap_dtlib_cpp/convolve.h"
#include "wrap_dtlib_cpp/reflect.h"

using namespace std;
using namespace DTLib;

/////////////////////////////////////////////////////////////////////////////

CFilterbank::CFilterbank()
{
    m_nGaussScales = 0;
    m_nGaussOrientations = 0;
    m_nDOGScales = 0;
    m_nKernels = 0;
}

/////////////////////////////////////////////////////////////////////////////

CFilterbank::~CFilterbank()
{
    FreeMemory();
}

/////////////////////////////////////////////////////////////////////////////

void CFilterbank::FreeMemory()
{
    if (!m_vecKernels.empty())
        for (int i = 0; i < (int)m_vecKernels.size(); i++) 
            delete m_vecKernels[i];
}

/////////////////////////////////////////////////////////////////////////////

bool CFilterbank::Setup(const int nGaussScales, 
                        const int nGaussOrientations,
                        const float GaussSigmaY, 
                        const float GaussX2YSigmaRatio,
                        const int nDOGScales,
                        const float DOGExcitSigma,
                        const float DOGInhibSigmaRatio1, 
                        const float DOGInhibSigmaRatio2)
{
    // remember constructor args
    m_nGaussScales = nGaussScales; 
    m_nGaussOrientations = nGaussOrientations; 
    m_GaussSigmaY = GaussSigmaY; 
    m_GaussX2YSigmaRatio = GaussX2YSigmaRatio; 
    m_nDOGScales = nDOGScales; 
    m_DOGExcitSigma = DOGExcitSigma; 
    m_DOGInhibSigmaRatio1 = DOGInhibSigmaRatio1; 
    m_DOGInhibSigmaRatio2 = DOGInhibSigmaRatio2; 
    // allocate space for kernel pointers
    m_nKernels = m_nGaussScales*m_nGaussOrientations*2+m_nDOGScales;

    FreeMemory();
    m_vecKernels.reserve(m_nKernels);

    // set up elongated Gaussian Kernels
    for (int iGaussScale = 0; iGaussScale < m_nGaussScales; iGaussScale++) {
        const float SigmaY = (float)pow((double)m_GaussSigmaY,
                                        (double)(iGaussScale));
        //                              (double)(iGaussScale+1));
        const float SigmaX = SigmaY*m_GaussX2YSigmaRatio;

        const float MaxSigma = MAX(SigmaX, SigmaY);
        int SideLength = F2I(KERNEL_SIDE_FACTOR*MaxSigma);
        ODDIFY(SideLength);
        for (int iTheta = 0; iTheta < m_nGaussOrientations; iTheta++) {
            const float Theta = 
                CONST_PI*(float)iTheta/(float)m_nGaussOrientations;
            CGaussKernel EvenKernel(SideLength, SigmaX,
                                    SigmaY, 2, Theta, false);
            CGaussKernel OddKernel(SideLength, SigmaX,
                                   SigmaY, 2, Theta, true);

            CImg<float>* pEvenImg = 
                new CImg<float>(EvenKernel.pBuffer(), 
                                SideLength, SideLength, true);
            if (pEvenImg == NULL) return false;
            CImg<float>* pOddImg = 
                new CImg<float>(OddKernel.pBuffer(),
                                SideLength, SideLength, true);
            if (pOddImg == NULL) return false;
            m_vecKernels.push_back(pEvenImg);
            m_vecKernels.push_back(pOddImg);

        } // for (iTheta..
    } // for (iGaussScale..
    // set up DOG kernels
    for (int iDOGScale = 0; iDOGScale < m_nDOGScales; iDOGScale++) {
        const float ExcitSigma = 
            (float)pow((double)m_DOGExcitSigma, (double)(iDOGScale));
        //  (float)pow((double)m_DOGExcitSigma, (double)(iDOGScale+1));
        const float InhibSigma1 = ExcitSigma*m_DOGInhibSigmaRatio1;
        const float InhibSigma2 = ExcitSigma*m_DOGInhibSigmaRatio2;

        float MaxSigma = MAX(ExcitSigma, InhibSigma1);
        MaxSigma = MAX(MaxSigma, InhibSigma2);
        int SideLength = F2I(KERNEL_SIDE_FACTOR*MaxSigma);
        ODDIFY(SideLength);
        CDOG DOGKernel(SideLength, ExcitSigma, InhibSigma1, InhibSigma2);
        CImg<float>* pDOGImg =
            new CImg<float>(DOGKernel.pBuffer(),
                            SideLength, SideLength, true);
        if (pDOGImg == NULL) return false;
        m_vecKernels.push_back(pDOGImg);
    } // for (iDOTScale..

    ASSERT(m_nKernels == (int)m_vecKernels.size());
    return true;
}

/////////////////////////////////////////////////////////////////////////////

void CFilterbank::Convolve(CImg<float> &InImg, CImgVec<float> &OutConvVec)
{
    int i = 0;
    for (int iGaussScale = 0; iGaussScale < m_nGaussScales; iGaussScale++) {
        for (int iTheta = 0; iTheta < m_nGaussOrientations; iTheta++) {
            for (int j = 0; j < 2; j++, i++) {
                CImg<float>* pKernelImg =  OutConvVec.GetImg(i);
                const int KernelSide = pKernelImg->Width();
                CImg<float> InImgPadded(InImg.Width()+KernelSide, 
                                        InImg.Height()+KernelSide);
                Reflect(InImg, InImgPadded);
                cerr << ".";
                DTLib::Convolve(InImgPadded, *(m_vecKernels[i]),
                                *pKernelImg);
            } // for (int j
        } // for (int iTheta
    } // for (int iGaussScale
    for (int iDOGScale = 0; iDOGScale < m_nDOGScales; iDOGScale++, i++) {
        CImg<float>* pKernelImg =  OutConvVec.GetImg(i);
        const int KernelSide = pKernelImg->Width();
        CImg<float> InImgPadded(InImg.Width()+KernelSide, 
                                InImg.Height()+KernelSide);
        Reflect(InImg, InImgPadded);
        cerr << ".";
		DTLib::Convolve(InImgPadded, 
                           *(m_vecKernels[i]), 
                           *(OutConvVec.GetImg(i)));
    } // for (int iDOGScale
    cerr << endl;
}
