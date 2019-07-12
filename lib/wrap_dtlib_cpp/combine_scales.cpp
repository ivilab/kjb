/////////////////////////////////////////////////////////////////////////////
// combine_scales.cpp - combine several scales into one
// Author: Doron Tal
// Date Created: June, 2000

#include "l/l_sys_debug.h"  /* For ASSERT. */
#include "wrap_dtlib_cpp/combine_scales.h"

using namespace DTLib;

// fill up OutEnergyCombImg and OutOriCombImg with max from all scales,
// combining scales

void DTLib::CombineScalesNMSRhoTheta(const int& nGaussScales,
                                     CImgVec<BYTE>& InNMSVec, 
                                     CImgVec<float>& InEnergyVec, 
                                     CImgVec<float>& InOriVec, 
                                     CImg<BYTE>& OutNMSCombImg, 
                                     CImg<float>& OutEnergyCombImg, 
                                     CImg<float>& OutOriCombImg)
{
    ASSERT(!OutEnergyCombImg.IsEmpty() && 
           !OutOriCombImg.IsEmpty() &&
           !OutNMSCombImg.IsEmpty());

    const int Width = OutEnergyCombImg.Width();
    const int Height = OutEnergyCombImg.Height();

    OutEnergyCombImg.Zero();
    OutOriCombImg.Zero();
    OutNMSCombImg.Zero();

    FloatImgVecIter ppOutEnergyCombImg = InEnergyVec.itImgs();
    FloatImgVecIter ppOutOriCombImg = InOriVec.itImgs();
    ByteImgVecIter ppOutNMSCombImg = InNMSVec.itImgs();
    for (int iScale = 0; iScale < nGaussScales; iScale++,
             ppOutEnergyCombImg++, ppOutOriCombImg++, ppOutNMSCombImg++) {

        float* pInEnergy = (*ppOutEnergyCombImg)->pBuffer(); 
        float* pOutRho = OutEnergyCombImg.pBuffer();

        float* pInOri = (*ppOutOriCombImg)->pBuffer();
        float* pOutTheta = OutOriCombImg.pBuffer();

        BYTE* pInNMS = (*ppOutNMSCombImg)->pBuffer();
        BYTE* pOutNMS = OutNMSCombImg.pBuffer();

        for (int y = 0; y < Height; y++) {
            for (int x = 0; x < Width; x++, pInEnergy++, pInOri++,
                     pOutRho++, pOutTheta++, pInNMS++, pOutNMS++) {
                if (*pInNMS && (*pInEnergy > *pOutRho)) {
                    *pOutRho = *pInEnergy;
                    *pOutTheta = *pInOri;
                    *pOutNMS = (BYTE)255;
                } // if (*pInNMS && (*pInEnergy > *pOutRho)) {
            } // for (int x ...
        } // for (int y = 0; y < Height; y++) {
    } // for (int iScale = 0; iScale < nGaussScales;
}
