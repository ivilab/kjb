/////////////////////////////////////////////////////////////////////////////
// texturescale.h - compute texture scale
// Author: Doron Tal
// Date created: April, 2000

#ifndef _TEXTURESCALE_H
#define _TEXTURESCALE_H

#define REAL float /* required, for triangle.h */

#include "wrap_dtlib_cpp/img.h"
#include "wrap_dtlib_cpp/triangle.h"

namespace DTLib {

    // *** TODO: *** need comments

    class CTextureScale
    {
    public:

        CTextureScale(const int maxPoints);
        ~CTextureScale();

        void ComputeScaleImg(CImg<BYTE> &Img, CImg<float> &ScaleImg,
                             const float& minDist, const float& maxDist,
                             const float& MedianMultiplier);

    private:
        REAL* m_pInputPts;
        int* m_pEdges;
    };

} // namespace DTLib {

#endif /* #ifndef _TEXTURESCALE_H */
