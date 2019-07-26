/////////////////////////////////////////////////////////////////////////////
// ptexture.h - 1D/2D function determining the probability of texture
// Author: Doron Tal
// Date created: May, 2000

#ifndef _PTEXTURE_H
#define _PTEXTURE_H

#include "wrap_dtlib_cpp/img.h"
#include "wrap_dtlib_cpp/circle.h"

#warning "[Code police] Do not put 'using namespace' in global scope of header."
using namespace std;

namespace DTLib {

    // INPUTS:
    // 'CircleMasks' - images with circles of increasing sizes drawn in them
    // *** TODO: *** finish this comment!

    void ComputePTextureImg(CCircleMasks& CircleMasks,
                            CImg<BYTE>& NMSCombImg,
                            CImg<float>& ThetaStarImg,
                            CImg<float>& TextureScaleImg,
                            CImg<kjb_int32>& TextonMembershipImg,
                            const int& K,
                            const float& DiskMiddleBandThickness,
                            const float& Tau, const float& Beta,
                            CImg<float>& PTextureImg);

} // namespace DTLib {

#endif
