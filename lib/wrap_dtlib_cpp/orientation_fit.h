/////////////////////////////////////////////////////////////////////////////
// orientation_fit.h - parabolic orientations fit, orientation interpolation
// Author: Doron Tal
// Date Created: April, 2000

#ifndef _PARAB_ORI_FIT_H
#define _PARAB_ORI_FIT_H

#include "wrap_dtlib_cpp/img.h"

namespace DTLib {

///////////////////////////////////////////////////////////////////////////
// INPUT: orientation energy per scale per orientation of the
// elongated gaussians
// OUTPUT: a sequence of scales*2 frames, for each scale,
// two frames are returned: (a) the maximum orientation energy
// (b) the better-fitted, interpolated, "winner" orientation for
// that scale and maximum energy.   NOTE!  the energy returned
// is the energy of the filterbank convolution that was max at
// each pixel, the orientation is not the orientation that produced
// this max, but one interpolated parabolically against its two
// neighboring orientations.  the OE, however, is not interpolated!
// (you may want to change this and try interpolating the OE as well).
void ParabolicOrientationsFit(const int& nGaussScales,
                              const int& nGaussOrientations,
                              CImgVec<float>& OEVec,
                              CImgVec<float>& RhoVec,
                              CImgVec<float>& ThetaVec,
                              CImgVec<BYTE>& ThetaIdxVec);

void InterpolateOrientations(const int& iScale,
                             const int& nGaussOrientations,
                             CImgVec<float>& OEVec,
                             CImgVec<BYTE>& ThetaIdxVec,
                             CImgVec<float>& ThetaVec);

} // namespace DTLib {

#endif /* #ifndef _PARAB_ORI_FIT_H */
