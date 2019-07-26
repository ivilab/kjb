/////////////////////////////////////////////////////////////////////////////
// colorconv.cpp -- Doron Tal

#include "wrap_dtlib_cpp/colorconv.h"
#include <math.h>

////////////////////////////////////////////
//Added by Prasad
//To allow for float precision for image pixels
//NOTE***********Make sure the following is undefined when using any 
//of the following functions:
//BGR2Gray, BGR2L, BGR2LAB
//because they assume char precision for image pixels***************

#define ENABLE_FLOAT_PRECISION
////////////////////////////////////////////

////////////////////////////////////////////
//Added by Prasad
//To allow for processing of linear images directly without
//applying a reverse gamma correction

//#define DISABLE_REVERSE_GAMMA_CORRECTION
////////////////////////////////////////////

using namespace DTLib;

///////////////////////////////////////////////////////////////////////////

void DTLib::BGR2Gray(unsigned char* pInBuf, unsigned char* pOutBuf,
                     const int& Width, const int& Height)
{
    unsigned char *pIn = pInBuf, *pOut = pOutBuf;
    for (int y = 0; y < Height; y++) {
        for (int x = 0; x < Width; x++) {
            const unsigned char B = *pIn++;
            const unsigned char G = *pIn++;
            const unsigned char R = *pIn++;
            *pOut++ = RGB2GRAY(R, G, B);
        }
    }
}

///////////////////////////////////////////////////////////////////////////

void DTLib::BGR2LAB(float* pInBuf, float* pOutBuf,
                    const int& Width, const int& Height,
                    const float& Gamma)
{
    const float XnRecip =
        1.0f/(XYZ_00*RGB_WHITE_PT+XYZ_01*RGB_WHITE_PT+XYZ_02*RGB_WHITE_PT);
    const float YnRecip =
        1.0f/(XYZ_10*RGB_WHITE_PT+XYZ_11*RGB_WHITE_PT+XYZ_12*RGB_WHITE_PT);
    const float ZnRecip =
        1.0f/(XYZ_20*RGB_WHITE_PT+XYZ_21*RGB_WHITE_PT+XYZ_22*RGB_WHITE_PT);

    float* pIn = pInBuf;
    float* pOut = pOutBuf;
    for (int y = 0; y < Height; y++) {
        for (int x = 0; x < Width; x++) {
            const float B = (float)(*pIn++);
            const float G = (float)(*pIn++);
            const float R = (float)(*pIn++);

////////////////////////////////////////////
//Added by Prasad
//To allow for processing of linear images directly without
//applying a reverse gamma correction

#ifdef DISABLE_REVERSE_GAMMA_CORRECTION
            const float fR = (float) R;
	    const float fG = (float) G;
	    const float fB = (float) B;
#else 
            // reverse gamma correction RGB in [0,255], fRfGfB in [0,255]
            const float fR = (float)pow(R/255.0f, DEFAULT_GAMMA)*255.0f;
            const float fG = (float)pow(G/255.0f, DEFAULT_GAMMA)*255.0f;
            const float fB = (float)pow(B/255.0f, DEFAULT_GAMMA)*255.0f;
#endif
////////////////////////////////////////////

            // convert rgb to xyz
            const float X = XYZ_00*fR+XYZ_01*fG+XYZ_02*fB;
            const float Y = XYZ_10*fR+XYZ_11*fG+XYZ_12*fB;
            const float Z = XYZ_20*fR+XYZ_21*fG+XYZ_22*fB;

            // convert rgb to lab
            const float f_x = LAB_F(X*XnRecip);
            const float f_y = LAB_F(Y*YnRecip);
            const float f_z = LAB_F(Z*ZnRecip);

            *pOut++ = 116.0f*f_y-16.0f; // L
            *pOut++ = 500.0f*(f_x-f_y); // A
            *pOut++ = 200.0f*(f_y-f_z); // B
        } // for x
    } // for y
}

///////////////////////////////////////////////////////////////////////////

void DTLib::BGR2L(unsigned char* pInBuf, float* pOutBuf,
                  const int& Width, const int& Height,
                  const float& Gamma)
{
    const float YnRecip = 1.0f/(XYZ_10*RGB_WHITE_PT+
                                XYZ_11*RGB_WHITE_PT+
                                XYZ_12*RGB_WHITE_PT);
    unsigned char* pIn = pInBuf;
    float* pOut = pOutBuf;
    for (int y = 0; y < Height; y++) {
        for (int x = 0; x < Width; x++) {
            const float B = (float)(*pIn++);
            const float G = (float)(*pIn++);
            const float R = (float)(*pIn++);

////////////////////////////////////////////
//Added by Prasad
//To allow for processing of linear images directly without
//applying a reverse gamma correction

#ifdef DISABLE_REVERSE_GAMMA_CORRECTION
            const float fR = (float) R;
	    const float fG = (float) G;
	    const float fB = (float) B;
#else 
            // reverse gamma correction RGB in [0,255], fRfGfB in [0,255]
            const float fR = (float)pow(R/255.0f, DEFAULT_GAMMA)*255.0f;
            const float fG = (float)pow(G/255.0f, DEFAULT_GAMMA)*255.0f;
            const float fB = (float)pow(B/255.0f, DEFAULT_GAMMA)*255.0f;
#endif
////////////////////////////////////////////

	    const float Y = XYZ_10*fR+XYZ_11*fG+XYZ_12*fB;
            const float f_y = LAB_F(Y*YnRecip);
            *pOut++ = 116.0f*f_y-16.0f; // L
        } // for x
    } // for y
}

///////////////////////////////////////////////////////////////////////////
//Added by Prasad
//Color conversion from RGB to Srg

////////////////////////////////////////////
//Added by Prasad
//To allow for float precision for image pixels

#ifdef ENABLE_FLOAT_PRECISION
void DTLib::BGR2Srg(float* pInBuf, float* pOutBuf,
                    const int& Width, const int& Height)
#else
void DTLib::BGR2Srg(unsigned char* pInBuf, float* pOutBuf,
                    const int& Width, const int& Height)
#endif
////////////////////////////////////////////
{
////////////////////////////////////////////
//Added by Prasad
//To allow for float precision for image pixels

#ifdef ENABLE_FLOAT_PRECISION
    float* pIn = pInBuf;
#else
    unsigned char* pIn = pInBuf;
#endif
////////////////////////////////////////////
    float* pOut = pOutBuf;

    for (int y = 0; y < Height; y++) {
         for (int x = 0; x < Width; x++) {
             const float B = (float)(*pIn++);
             const float G = (float)(*pIn++);
             const float R = (float)(*pIn++);
////////////////////////////////////////////
//Added by Prasad
//To allow for processing of linear images directly without
//applying a reverse gamma correction

#ifdef DISABLE_REVERSE_GAMMA_CORRECTION
            const float fR = (float) R;
	    const float fG = (float) G;
	    const float fB = (float) B;
#else 
            // reverse gamma correction RGB in [0,255], fRfGfB in [0,255]
            const float fR = (float)pow(R/255.0f, DEFAULT_GAMMA)*255.0f;
            const float fG = (float)pow(G/255.0f, DEFAULT_GAMMA)*255.0f;
            const float fB = (float)pow(B/255.0f, DEFAULT_GAMMA)*255.0f;
#endif
////////////////////////////////////////////

            //Conversion from fRfGfB to rgS
            float S = fR+fG+fB;
            float r;
            float g;
            //Calculate rg only when S>0, otherwise you get a NaN (zero divided by zero)
            if (S>0) {
                       r = fR/S;
                       g = fG/S;
            }// if (S>0)

            else {
                   r = 0.0f;
                   g = 0.0f;
            }// else

            *pOut++ = S; //S
            *pOut++ = r; //r
            *pOut++ = g; //g
         }  //for x
    }  //for y
}
