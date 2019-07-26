/////////////////////////////////////////////////////////////////////////////
// utils.h - miscellaneous utilities (mostly math)
// Author: Doron Tal
// Date Created: January, 1993
// Date Last Modified: Aug 15, 2000.

#ifndef _UTILS_H
#define _UTILS_H

#include <math.h>

/////////////////////////////////////////////////////////////////////////////
// CONSTANTS
/////////////////////////////////////////////////////////////////////////////

#undef  CONST_MAX_FLOAT
#define CONST_MAX_FLOAT     (float)1.0e38

#undef  CONST_MIN_FLOAT
#define CONST_MIN_FLOAT     (float)-1.0e38

#undef  CONST_EPSILON
#define CONST_EPSILON       (float)0.00000000001

#undef  CONST_E
#define CONST_E             (float)2.7182818284590452354

#undef  CONST_LOG2E
#define CONST_LOG2E         (float)1.4426950408889634074

#undef  CONST_LOG10E
#define CONST_LOG10E        (float)0.43429448190325182765

#undef  CONST_LN2
#define CONST_LN2           (float)0.69314718055994530942

#undef  CONST_LN10
#define CONST_LN10          (float)2.30258509299404568402

#undef  CONST_PI
#define CONST_PI            (float)3.14159265358979323846

#undef  CONST_2PI
#define CONST_2PI           (float)6.28318530717958647692

#undef  CONST_PI_2
#define CONST_PI_2          (float)1.57079632679489661923

#undef  CONST_PI_4
#define CONST_PI_4          (float)0.78539816339744830962

#undef  CONST_3PI_4
#define CONST_3PI_4         (float)2.35619449019234492885

#undef  CONST_ONE_PI
#define CONST_ONE_PI        (float)0.31830988618379067154

#undef  CONST_TWO_PI
#define CONST_TWO_PI        (float)0.63661977236758134308

#undef  CONST_TWO_SQRTPI
#define CONST_TWO_SQRTPI    (float)1.12837916709551257390

#undef  CONST_180_PI
#define CONST_180_PI        (float)57.2957795130823208768

#undef  CONST_SQRT2
#define CONST_SQRT2         (float)1.41421356237309504880

#undef  CONST_SQRT1_2
#define CONST_SQRT1_2       (float)0.70710678118654752440

#undef  CONST_SQRT3OVER2
#define CONST_SQRT3OVER2    (float)0.86602540378443864676

#undef  CONST_3PI_2
#define CONST_3PI_2         (float)4.71238898038468985769

#undef  CONST_PI_8
#define CONST_PI_8          (float)0.39269908169872415480

#undef  CONST_3PI_8
#define CONST_3PI_8         (float)1.17809724509617246442

#undef  CONST_5PI_8
#define CONST_5PI_8         (float)1.96349540849362077403

#undef  CONST_7PI_8
#define CONST_7PI_8         (float)2.74889357189106908365

///////////////////////////////////////////////////////////////////////////
// MACROS
///////////////////////////////////////////////////////////////////////////

#undef  BYTE
#define BYTE            unsigned char

#undef  SQR
#define SQR(x)          ((x)*(x))

#undef  IS_ODD
#define IS_ODD(x)       ((x) % 2)     /* only use these on ints */

#undef  IS_EVEN
#define IS_EVEN(x)      (!IS_ODD(x))

#undef  ODDIFY
#define ODDIFY(x)       if (IS_EVEN(x)) x++

#undef  F2I
#define F2I(x)          ((x > 0.0f) ? (int)(x+0.5f) : (int)(x-0.5f))

#undef  APPXEQL
#define APPXEQL(x,y,e)  (fabs((x)-(y)) <= e)

#undef  SGN
#define SGN(x)          (((x) > 0) ? 1 : (((x) == 0) ? 0 : -1))

#undef SAME_SGN
#define SAME_SGN(x, y)  (((x) > 0) ? ((y) > 0) : (y <= 0))

#undef MAX
#define MAX(a,b)        ((a) > (b) ? (a) : (b))

#undef MIN
#define MIN(a,b)        ((a) < (b) ? (a) : (b))

#undef ABS
#define ABS(x)          ((x) < 0 ? -(x) : (x))


////////////////////////////////////////////
//Modified by Prasad

//This is to check the correct usage of delete operator for arrays.                                           
#undef zap
#define zap(x)          do { if (x) { delete(x); x = 0; } } while (0)
//#define zap(x)          if (x) { delete[] x; x = NULL; }

#undef IS_A_FLOAT
#define IS_A_FLOAT(x)   ((x >= CONST_MIN_FLOAT) && (x <= CONST_MAX_FLOAT))

/////////////////////////////////////////////////////////////////////////////
// MISCELLANEOUS FUNCTIONS
/////////////////////////////////////////////////////////////////////////////

namespace DTLib {

    ///////////////////////////////////////////////////////////////////////////
    // Returns difference between angles a1 and a2.  There are always two
    // Values for the difference of two angles.  The sum of these two
    // difference-Values is always 2pi.  This returns the smaller of the
    // two Values.  Returns in range [0, PI].

    double AngleDiff(double a1, double a2);

    float AngleDiff(float a1, float a2);

    ///////////////////////////////////////////////////////////////////////////
    // Returns in range [-PI, PI].

    double AngleSum(double a1, double a2);

    float AngleSum(float a1, float a2);

    ///////////////////////////////////////////////////////////////////////////
    // Bivariate Gaussian distribution.  The six parameters of the
    // Gaussian are: mu1, mu2, mu12, sigma1, sigma2, sigma12 Given the
    // above parameters and Values for x1, and x2, returns the probability
    // P(x1, x2).

    float BiGauss(float mu1, float mu2, float mu12,
                  float sigma1, float sigma2, float sigma12,
                  float x1, float x2);

    ///////////////////////////////////////////////////////////////////////////

    float FuzzySigmoid(float g, float a, float b, float c);

    ///////////////////////////////////////////////////////////////////////////
    // ***TODO:*** add this sigmoid, and add this one reversed..
    // WijTX = 1.0f-
    //    1.0f/(1.0f+exp(-20.0f*(ChiSqr-SigmaTX)));

    ///////////////////////////////////////////////////////////////////////////
    // Maximum element of pBuf, a float array

    float Max(float* pBuf, int Size);

    int Max(int* pBuf, int Size);

    ///////////////////////////////////////////////////////////////////////////
    // Minimum element of pBuf

    float Min(float* pBuf, int Size);

    // same as above, but returns the index of minimum element
    int iMin(float* pBuf, int Size);


    ///////////////////////////////////////////////////////////////////////////
    // log to any base

    double logbase(const double base, const double argument);

    ///////////////////////////////////////////////////////////////////////////
    // Next higher power of 2.  Returns the first P such that 2^P >= abs(N).

    int NextPowerOfTwo(const int x);

    ///////////////////////////////////////////////////////////////////////////
    // routines for converting between floats and ints, for interger arithmetic

    void FloatToInt(const float& Xfloat, int& Xint,
                    const float& MinRange, const float& MaxRange,
                    const int& Resolution);

    void IntToFloat(const int& Xint, float& Xfloat,
                    const float& MinRange, const float& MaxRange,
                    const int& Resolution);

    ///////////////////////////////////////////////////////////////////////////
    // is an angle 'accute' or not?

    bool IsAccute(const float& Theta);

    ///////////////////////////////////////////////////////////////////////////
    // returns abscissa location of parabola's max, given three points
    // to describe the parabola

    float ParabolicInverseInterpolation(const float& x0, const float& y0,
                                        const float& x1, const float& y1,
                                        const float& x2, const float& y2);

    ///////////////////////////////////////////////////////////////////////////
    // same as above, but for (orientation, amplitude) pairs, corrects
    // orientations at the end to be in [0, pi] PRECONDITION:
    // |Theta1-Theta2| == |Theta2-Theta3| returns the interpolated
    // orientation in 'InterpolatedThetaResult'

    void ParabolicOrientationInterpolation(const float& Theta1,
                                           const float& Rho1,
                                           const float& Theta2,
                                           const float& Rho2,
                                           const float& Theta3,
                                           const float& Rho3,
                                           float& InterpolatedThetaResult);

    ///////////////////////////////////////////////////////////////////////////
    // given three arbitrary points (x0,y0), (x1,y1), (x2,y2) that are
    // assumed to be points on a parabola y=ax^2+bx+c, returns the parabola
    // coefficients in 'a', 'b' and 'c' PRECOND: x1-x0 == x2-x1

    void ParabolicInterpolation(const float& x0, const float& y0,
                                const float& x1, const float& y1,
                                const float& x2, const float& y2,
                                float& a, float& b, float& c);

    ///////////////////////////////////////////////////////////////////////////
    // Fixes orientation range to be [0.0, MaxTheta], where 'MaxTheta'
    // = Pi if 'bHalfPhase' is true and 'MaxTheta' = 2Pi if
    // 'bHalfPhase' is false.

    void FixThetaRange(float& Theta, const bool& bHalfPhase = true);

    // swapping of pointers
    template <class T>
    inline void Swap(T& p1, T& p2) {
        T pTmp = p1;
        p1 = p2;
        p2 = pTmp;
    }

    template <class T>
    inline T SSD(const T& x1, const T& y1, const T& x2, const T& y2) {
        const T dx = x1-x2;
        const T dy = y1-y2;
        return (SQR(dx)+SQR(dy));
    }

} // namespace DTLib {

#endif /* _UTILS_H */
