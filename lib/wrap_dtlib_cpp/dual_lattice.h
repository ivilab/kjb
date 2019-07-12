/////////////////////////////////////////////////////////////////////////////
// dual_lattice.h - building the dual lattice for later building sparse mat
// Author: Doron Tal
// Date Created: May, 2000

#ifndef _DUAL_LATTICE_H
#define _DUAL_LATTICE_H

#include "wrap_dtlib_cpp/img.h"

namespace DTLib {

    // NOTE: a non-zero pixel in InDualHImg at position (x0, y0) represents
    // the edge between pixel (x0, y0) in the original image and its
    // neighboring pixel to the right (x0+1, y0), similarly a non-zero
    // pixel position (x0, y0) in DualVImg represents an edge in the
    // original image between pixel (x0, y0) and it's neighbor one pixel
    // below, at (x0, y0+1).

    void MakeDualLattice(const float& EdgelLength,
                         const float& SigmaIC,
                         const int& nGaussScales,
                         CImgVec<BYTE>& InNMSVec,
                         CImgVec<float>& InEnergyVec,
                         CImgVec<float>& InOriVec,
                         CImgVec<float>& InXLocVec,
                         CImgVec<float>& InYLocVec,
                         CImg<float>& InPTextureImg,
                         CImg<float>& OutDualHImg,
                         CImg<float>& OutDualVImg);

} // namespace DTLib {

#endif /* #ifndef _DUAL_LATTICE_H */
