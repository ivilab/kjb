/////////////////////////////////////////////////////////////////////////////
// combine_scales.h - combine several scales into one
// Author: Doron Tal
// Date Created: June, 2000

#ifndef _COMBINE_SCALES_H
#define _COMBINE_SCALES_H

#include "wrap_dtlib_cpp/img.h"

namespace DTLib {

    // fill up RhoImg and ThetaImg with max from all scales, combining scales
    void CombineScalesNMSRhoTheta(const int& nGaussScales,
                                  CImgVec<BYTE>& InNMSVec,
                                  CImgVec<float>& InEnergyVec,
                                  CImgVec<float>& InOriVec,
                                  CImg<BYTE>& OutNMSCombImg,
                                  CImg<float>& OutEnergyCombImg,
                                  CImg<float>& OutOriCombImg);

} // namespace DTLib {

#endif /* #ifndef _COMBINE_SCALES_H */
