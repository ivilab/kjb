/////////////////////////////////////////////////////////////////////////////
// rotate.h - rotate any type image
// Author: Doron Tal
// Date Created: 1994.

#ifndef _ROTATE_H
#define _ROTATE_H

#include "wrap_dtlib_cpp/img.h"
#include "wrap_dtlib_cpp/affine.h"

namespace DTLib {

    // rotate 'InImg' into 'OutImg' by angle 'Theta' (in radians)
    // if pixels 'OutImg' get no pixels from 'InImg' then assign
    // them the value 'BGVal'.

    template <class T>
        void Rotate(CImg<T>& InImg, CImg<T>& OutImg,
                    const double& Theta, const T& BGVal = 0) {

        const int InputWidth = InImg.Width();
        const int InputHeight = InImg.Height();

        const int OutputWidth = OutImg.Width();
        const int OutputHeight = OutImg.Height();

        const int HalfInputWidth = InputWidth/2;
        const int HalfInputHeight = InputHeight/2;

        const int HalfOutputWidth = OutputWidth/2;
        const int HalfOutputHeight = OutputHeight/2;

        const float K = (float)(InputWidth+OutputWidth+
                                InputHeight+OutputHeight);

        const int ix1 = HalfInputWidth;
        const int iy1 = HalfInputHeight;

        const int ox1 = HalfOutputWidth;
        const int oy1 = HalfOutputHeight;

        const int ix2 = (int)K+HalfInputWidth;
        const int iy2 = HalfInputHeight;

        const int ox2 = F2I(K*(float)cos(Theta))+HalfOutputWidth;
        const int oy2 = F2I(-K*(float)sin(Theta))+HalfOutputHeight;

        CAffine ca;
        ca.Setup(ix1, iy1, ix2, iy2, ox1, oy1, ox2, oy2);
        ca.Transform(InImg, OutImg, BGVal);
    }; // Rotate

} // namespace DTLib;

#endif /* #ifndef _ROTATE_H */
