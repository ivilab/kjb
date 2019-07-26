/////////////////////////////////////////////////////////////////////////////
// colorconv2.h -- Doron Tal

#ifndef _COLORCONV2_H
#define _COLORCONV2_H

////////////////////////////////////////////
//Added by Prasad
//To allow for float precision for image pixels
//NOTE***********Make sure the following is undefined when using any
//of the following functions:
//BGR2Gray, BGR2L, BGR2LAB
//because they assume char precision for image pixels***************
#define ENABLE_FLOAT_PRECISION
////////////////////////////////////////////

// #include "ncuts.h"

// Get grayscale from RGB, returns grayscale Value.  Both the commented out
// '#define' below and the non-commented out macros give the same value, but
// the uncommented macro should be faster on most architectures.
// #define RGB2GRAY(R, G, B) ((BYTE)(0.15625f*(2.2f*R+3.2f*G+B)))

#define RGB2GRAY(R, G, B) \
        ((unsigned char)((5120L*(long)B+(((long)G)<<14)+11264L*(long)R)>>15))

// default gamma value by which we assume the image has been corrected
// make sure this is float
#define DEFAULT_GAMMA 2.2f

// point considered white in RGB space, Kodak uses (200,200,200), which
// would translate to putting 200.0f here.  NOTE: this must be a float
// number.
#define RGB_WHITE_PT 255.0f

// the nonlinear CIE-XYZ to LAB function
#define LAB_F(x) ((x > 0.008856f) ? (float)pow(x, 1.0/3.0) : \
                     7.787f*x+16.0f/116.0f)

// the xyz transformation matrix

// X (0), as a linear function of R, G & B (0,1 & 2)
#define XYZ_00 0.4124f
#define XYZ_01 0.3576f
#define XYZ_02 0.1805f
// Y (1), as a linear function of R, G & B (0,1 & 2)
#define XYZ_10 0.2126f
#define XYZ_11 0.7152f
#define XYZ_12 0.0722f
// Z (2), as a linear function of R, G & B (0,1 & 2)
#define XYZ_20 0.0193f
#define XYZ_21 0.1192f
#define XYZ_22 0.9505f

///////////////////////////////////////////////////////////////////////////

namespace DTLib {

    ///////////////////////////////////////////////////////////////////////////
    // can be done in-place
    void BGR2Gray(unsigned char* pInBuf, unsigned char* pOutBuf,
                  const int& Width, const int& Height);

    ///////////////////////////////////////////////////////////////////////////

    /*void BGR2LAB(unsigned char* pInBuf, float* pOutBuf,
                 const int& Width, const int& Height,
                 const float& Gamma = DEFAULT_GAMMA);*/

    void BGR2LAB(float* pInBuf, float* pOutBuf,
                   const int& Width, const int& Height,
                   const float& Gamma = DEFAULT_GAMMA);

    void BGR2L(unsigned char* pInBuf, float* pOutBuf,
               const int& Width, const int& Height,
               const float& Gamma = DEFAULT_GAMMA);

//Added by Prasad
//Color conversion from RGB to Srg
////////////////////////////////////////////
//Added by Prasad
//To allow for float precision for image pixels

#ifdef ENABLE_FLOAT_PRECISION
   void BGR2Srg(float* pInBuf, float* pOutBuf,
                 const int& Width, const int& Height);
#else
    void BGR2Srg(unsigned char* pInBuf, float* pOutBuf,
                 const int& Width, const int& Height);
#endif
////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////
    // Change the channel ordering from pixel to planar, i.e.
    // if the original image is, for example, one big buffer of
    // BGR, BGR,... fill up three buffers B, G and R
    template <class T>
    inline void Interleaved2Planar(T* pIn, T* pOut1, T* pOut2, T* pOut3,
                                   const int& Width, const int& Height) {
        T *pI = pIn, *pO1 = pOut1, *pO2 = pOut2, *pO3 = pOut3;
        for (int y = 0; y < Height; y++) {
            for (int x = 0; x < Width; x++) {
                *pO1++ = *pI++; *pO2++ = *pI++; *pO3++ = *pI++;
            }
        }
    };

} // namespace DTLib

#endif /* #ifndef _COLORCONV2_H */
