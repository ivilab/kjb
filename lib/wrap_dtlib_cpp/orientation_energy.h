////////////////////////////////////////////////////////////////////////////
// orientation_energy.h - compute orientation energy
// Author: Doron Tal
// Date Created: March, 2000

#ifndef _ORIENTATION_ENERGY_H
#define _ORIENTATION_ENERGY_H

#include "wrap_dtlib_cpp/img.h"

////////////////////////////////////////
//Added by Prasad

//This is to test the hypothesis of using color in contour
//Add Orientation energies obtained in color planes

//#define USE_COLOR_IN_CONTOUR

///////////////////////////////////////

namespace DTLib {

    ///////////////////////////////////////////////////////////////////////////
    // computes orientation energy for each orientation. INPUT = InSeq,
    // the sequence of all convolutions with the kernels.  OUTPUT =
    // OutSeq, a sequence of images, corresponding to orientation energies
    // at each scale and orientation of the oriented filterbank kernels
    // (outer loop over scales, inner loop over orientations, so there are
    // SCALES*ORIENTATIONS frames in this returned sequence).
    void OrientationEnergy(const int& nGaussScales,
                           const int& nGaussOrientations,
                           CImgVec<float>& ConvVec,

////////////////////////////////////////
//Added by Prasad

//This is to test the hypothesis of using color in contour
//Add Orientation energies obtained in color planes

#ifdef USE_COLOR_IN_CONTOUR

                           CImgVec<float>& AConvVec,
                           CImgVec<float>& BConvVec,

#endif
///////////////////////////////////////

                           CImgVec<float>& OEVec);

    // operations on oe sequences:

    ///////////////////////////////////////////////////////////////////////////
    // INPUT:
    // - 'OEVec' is the orientation energy sequence, i.e. for each scale, OEVec
    //   has N frames of orientation energy associated with the N orientations.
    // - 'iScale' is the scale from which we seek the successive (OE, Theta)
    //   pairss
    // - 'iTheta' is the index of the orientation for whom we want the triplet
    //   it is the index of the orientation at the center of the triplet.
    // - 'x' and 'y' are the coordinate at which we take successive values
    //
    // OUTPUT: all arguments that are not const are output - they comprise
    // the triplet of (OE, Theta) pairs centered on theta designated by
    // 'iTheta'. 'iPrevTheta' and 'iNextTheta' are returned as indices into
    // OEVec of the previous and next thetas, these indices were fixed so
    // that they are always in the range [0..m_nGaussOrientations-1]
    //
    // NB: The thetas returned are not between 0 and pi, in order to
    // ensure that parabolic interpolation treats them as points on the
    // line, rather than points on the circle.  So we add Pi to the thetas
    // to make sure that they are returned in a sorted manner (sorted
    // along the line, not just along the circle).  Please note that some
    // functions expect orientations to be in the range [0, Pi].  Such
    // functions must first use the routine FixTheta() to bring each of
    // the thetas here to that range, they can't use the 'Theta',
    // 'PrevTheta' and 'NextTheta' returned by this function directly.

    // compute indices, energies, orientations for the max,
    // previous-to-max and next-to-max of all those three,
    // making sure to take care of phase wrapping when necessary

    void GetSuccessiveOEThetaPairs(CImgVec<float>& OEVec,
                                   const int& nGaussOrientations,
                                   const int& iScale,
                                   const int& iTheta,
                                   const int&x, const int& y,
                                   int& iPrevTheta, int& iNextTheta,
                                   float& PrevTheta,
                                   float& Theta,
                                   float& NextTheta,
                                   float& PrevOE,
                                   float& OE,
                                   float& NextOE);

} // namespace DTLib {

#endif /* #ifndef _ORIENTATION_ENERGY_H */
