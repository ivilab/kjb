/////////////////////////////////////////////////////////////////////////////
// affine.h - image affine transform class
// Author: Doron Tal
// Date created: February, 1996

#ifndef _AFFINE_H
#define _AFFINE_H

#include "wrap_dtlib_cpp/img.h"

namespace DTLib {

    // NOTE: affine transform performed by this class is followed
    // by a bilinear interpolation step.
    // NOTE: template functions must be inlined here, but non-template
    // functions still reside in affine.cpp.

    class CAffine
    {
    public:
        // To use, construct an affine object, then call
        // Setup(), then call Transform().
        CAffine();

        // Precompute affine transform parameters
        void Setup(const int& in_x1, const int& in_y1,
                   const int& in_x2, const int& in_y2,
                   const int& out_x1, const int& out_y1,
                   const int& out_x2, const int& out_y2);

        // Using Affine coefficients of the last call to Setup() or
        // Transform(), returns the point (ix, iy) in the input Img
        // that maps into the given point (ox, oy) in the output Img.
        // I.e. suppose we've transformed one image into another using
        // CAffine and we want to know for some point in the transformed
        // image where it came from, this function will give you the
        // coordinates in the pre-transformed image of a point in the
        // transformed image.
        // POSTCOND: result is in 'ix' and 'iy'
        inline void Out2In(const int& out_x, const int& out_y,
                           int& in_x, int& in_y) {
            in_x = F2I(m_rct*out_x-m_rst*out_y+m_C);
            in_y = F2I(m_rst*out_x+m_rct*out_y+m_c);
        };

        // This is the function that does the work (main loop).
        // PRECOND: you've called Setup() before
        // POSTCODN: result of transform is in 'OutImg'
        template <class T>
        void Transform(CImg<T>& InImg, CImg<T>& OutImg, const T& BGVal = 0) {
            const float oWidth = (float)OutImg.Width(),
                oHeight = (float)OutImg.Height();
            const int iWidth = InImg.Width(), iHeight = InImg.Height();
            const int iWidthM1 = iWidth-1, iHeightM1 = iHeight-1;
            T *pOut = OutImg.pBuffer(), *pIn = InImg.pBuffer();
            for (float oy = 0.0f; oy < oHeight; oy++) {
                for (float ox = 0.0f; ox < oWidth; ox++, pOut++) {
                    float ix = m_rct*ox-m_rst*oy+m_C;
                    float iy = m_rst*ox+m_rct*oy+m_c;
                    int ix_i = (int)ix;
                    int iy_i = (int)iy;
                    if ((ix_i < 0) ||
                        (ix_i > iWidthM1) ||
                        (iy_i < 0) ||
                        (iy_i > iHeightM1)) {
                        *pOut = BGVal;
                        continue;
                    }
                    float dx = ix-(float)ix_i;
                    float dx1 = 1.0f-dx;
                    float dy = iy-(float)iy_i;
                    float dy1 = 1.0f-dy;
                    T *pUL = pIn+iy_i*iWidth+ix_i, *pLL, *pUR, *pLR;
                    if (ix_i == iWidthM1) pUR = pUL;
                    else pUR = pUL+1;
                    if (iy_i == iHeightM1) { pLL = pUL; pLR = pUR; }
                    else {pLL = pUL+iWidth; pLR = pUR+iWidth; }
                    *pOut = (T)((float)*pUL*dx1*dy1+(float)*pUR*dx*dy1+
                                (float)*pLL*dx1*dy+(float)*pLR*dx*dy);
                } // for ox
            } // for oy
        };

    private:

        float m_rho;   // degree of dilation/expansion (in/out size change)
        float m_theta; // degree of rotation
        float m_rct;   // rho*cos(Theta)
        float m_rst;   // rho*sin(Theta)
        float m_C;     // translation in x
        float m_c;     // translation in y
    }; // class CAffine
} // namespace DTLib {

#endif /* #ifndef _AFFINE_H */
