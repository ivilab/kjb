/////////////////////////////////////////////////////////////////////////////
// utils.cpp - miscellaneous utilities (mostly math)
// Author: Doron Tal
// Date Created: January, 1993
// Date Last Modified: Aug 15, 2000.

#include "l/l_sys_debug.h"  /* For ASSERT. */
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include "wrap_dtlib_cpp/utils.h"

using namespace DTLib;

/////////////////////////////////////////////////////////////////////////////
// MISCELLANEOUS MATH FUNCTIONS
/////////////////////////////////////////////////////////////////////////////

double DTLib::AngleDiff(double a1, double a2)
{
    register double diff = a1-a2;
    diff = fabs(diff);
    if (diff < CONST_PI) return(diff);
    else return(CONST_2PI-diff);
}

/////////////////////////////////////////////////////////////////////////////

float DTLib::AngleDiff(float a1, float a2)
{
    register float diff = a1-a2;
    diff = (float)fabs((double)diff);
    if (diff < CONST_PI) return(diff);
    else return(CONST_2PI-diff);
}

/////////////////////////////////////////////////////////////////////////////

double DTLib::AngleSum(double a1, double a2)
{
    register double Sum = a1+a2;
    if (Sum > CONST_PI) Sum -= CONST_2PI;
    else if (Sum < -CONST_PI) Sum += CONST_2PI;
    return Sum;
}

/////////////////////////////////////////////////////////////////////////////

float DTLib::AngleSum(float a1, float a2)
{
    register float Sum = a1+a2;
    if (Sum > CONST_PI) Sum -= CONST_2PI;
    else if (Sum < -CONST_PI) Sum += CONST_2PI;
    return Sum;
}

/////////////////////////////////////////////////////////////////////////////

float DTLib::BiGauss(float mu1, float mu2, float mu12,
                     float sigma1, float sigma2, float sigma12,
                     float x1, float x2)
{
    float sigma1_times_sigma2 = sigma1*sigma2;
    float rho = (mu12-mu1*mu2)/sigma1_times_sigma2;
    float x1_minus_mu1 = x1-mu1;
    float x2_minus_mu2 = x2-mu2;
    float z = SQR(x1_minus_mu1)/SQR(sigma1)-
        2.0f*rho*x1_minus_mu1*x2_minus_mu2/sigma1_times_sigma2+
        SQR(x2_minus_mu2)/SQR(sigma2);
    float one_minus_rhosqr = 1.0f-SQR(rho);
    return (1.0f/(CONST_2PI*sigma1_times_sigma2*
                  (float)sqrt((double)one_minus_rhosqr)))*
        (float)exp((double)(-z/(2.0f*one_minus_rhosqr)));
}

/////////////////////////////////////////////////////////////////////////////

float DTLib::FuzzySigmoid(float g, float a, float b, float c)
{
    if(g <= a) return 0.0;
    if(g >= b) return 1.0;
    if(g <= c)
        return (float) (2.0f*((g-a)*(g-a))/((b-a)*(b-a)));
    else
        return (float) (1.0f-2.0f*((g-b)*(g-b))/((b-a)*(b-a)));
}

/////////////////////////////////////////////////////////////////////////////

float DTLib::Max(float* pBuf, int Size)
{
    register float max = CONST_MIN_FLOAT, *pB = pBuf;
    for (register int i = 0; i < Size; i++, pB++) if (*pB > max) max = *pB;
    return max;
}

int DTLib::Max(int* pBuf, int Size)
{
    register int max = *pBuf, *pB = pBuf;
    for (register int i = 0; i < Size; i++, pB++) if (*pB > max) max = *pB;
    return max;
}


/////////////////////////////////////////////////////////////////////////////

float DTLib::Min(float* pBuf, int Size)
{
    register float min = CONST_MAX_FLOAT, *pB = pBuf;
    for (register int i = 0; i < Size; i++, pB++) if (*pB < min) min = *pB;
    return min;
}

/////////////////////////////////////////////////////////////////////////////

int DTLib::iMin(float* pBuf, int Size)
{
    register float min = CONST_MAX_FLOAT, *pB = pBuf;
    register int iMin = -1;
    for (register int i = 0; i < Size; i++, pB++) if (*pB < min) {
        min = *pB;
        iMin = i;
    }
    ASSERT(iMin != -1);
    return iMin;
}

/////////////////////////////////////////////////////////////////////////////

double DTLib::logbase(const double base, const double argument)
{
    return log(argument)/log(base);
}

/////////////////////////////////////////////////////////////////////////////
// Next higher power of 2.  Returns the first P such that 2^P >= abs(N). 

int DTLib::NextPowerOfTwo(const int x)
{
    int exp;
    double f = frexp(fabs((double)x), &exp);
    if (f == 0.5) exp--;
    return exp;
}

/////////////////////////////////////////////////////////////////////////////
// routines for converting from floats to ints for when using 
// integer arithmetic, or histograms, for example

void DTLib::FloatToInt(const float& Xfloat, int& Xint, 
                       const float& MinRange, const float& MaxRange,
                       const int& Resolution) {
    const float Range = MaxRange-MinRange;
    const float Factor = (float)(Resolution-1)/Range;
    Xint = F2I((Xfloat-MinRange)*Factor);
    ASSERT(Xint < Resolution);
    ASSERT(Xint >= 0);
};

void DTLib::IntToFloat(const int& Xint, float& Xfloat, 
                       const float& MinRange, const float& MaxRange,
                       const int& Resolution) {
    const float Range = MaxRange-MinRange;
    Xfloat = MinRange+Range*Xint/(float)Resolution;
    ASSERT(Xfloat >= MinRange);
    ASSERT(Xfloat <= MaxRange);
};


/////////////////////////////////////////////////////////////////////////////

bool DTLib::IsAccute(const float& Theta) {
    return ((Theta < CONST_PI_4) || (Theta >= CONST_3PI_4));
};

/////////////////////////////////////////////////////////////////////////////
// interpolate.. Theta2 is the center theta, Theta1 is the one before,
// Theta3 is the one after.  RhoX is the associated orientation energy.

void DTLib::ParabolicOrientationInterpolation(const float& Theta1,
                                              const float& Rho1,
                                              const float& Theta2,
                                              const float& Rho2,
                                              const float& Theta3,
                                              const float& Rho3,
                                              float& InterpolatedThetaResult)
{
    // the fitting assumes thetas are evenly distributed on the real # line
    const float a = -1.0f, b = 0.0f, c = 1.0f;
    const float fa = Rho1, fb = Rho2, fc = Rho3;
  
    ASSERT((fb >= fa) && (fb >= fc));
  
    const float b_minus_a = b-a;
    const float b_minus_c = b-c;
  
    const float fb_minus_fa = fb-fa;
    const float fb_minus_fc = fb-fc;  
  
    const float numerator =
        SQR(b_minus_a)*fb_minus_fc-SQR(b_minus_c)*fb_minus_fa;
    const float denominator = b_minus_a*fb_minus_fc-b_minus_c*fb_minus_fa;
  
    // result, on the interval [-1, 1]
    const float new_b = b-0.5f*numerator/denominator;
  
    const float ThetaDiff1 = (float)fabs(Theta2-Theta1);
    const float ThetaDiff2 = (float)fabs(Theta2-Theta3);
  
    const float ThetaDiff = MIN(ThetaDiff1, ThetaDiff2);
    const float ThetaInc = new_b*ThetaDiff;
  
    InterpolatedThetaResult = Theta2+ThetaInc;
    if (InterpolatedThetaResult > CONST_PI) 
        InterpolatedThetaResult -= CONST_PI;
    else if (InterpolatedThetaResult < 0.0f)
        InterpolatedThetaResult += CONST_PI;
}

/////////////////////////////////////////////////////////////////////////////

float DTLib::ParabolicInverseInterpolation(const float& x0, const float& y0,
                                           const float& x1, const float& y1,
                                           const float& x2, const float& y2)
{
    ASSERT((x2 > x1) && (x1 > x0));
    ASSERT((y1 >= y0) && (y1 >= y2) && !((y1 == y0) && (y1 == y2)));
  
    const float x1Mx0 = x1-x0;
    const float x1Mx2 = x1-x2;
  
    const float y1My0 = y1-y0;
    const float y1My2 = y1-y2;  
  
    const float numerator= SQR(x1Mx0)*y1My2-SQR(x1Mx2)*y1My0;
    const float denominator = x1Mx0*y1My2-x1Mx2*y1My0;
  
    return (x1-0.5f*numerator/denominator);
}

/////////////////////////////////////////////////////////////////////////////

// PRECOND: x1-x0 == x2-x1

void DTLib::ParabolicInterpolation(const float& x0, const float& y0,
                                   const float& x1, const float& y1,
                                   const float& x2, const float& y2,
                                   float& a, float& b, float& c)
{
    ASSERT((x2 > x1) && (x1 > x0));
    // ASSERT((y1 >= y0) && (y1 >= y2) && !((y1 == y0) && (y1 == y2)));

    const float xoffset = x1-x0;
    a = ((y2+y0)/2.0f-y1)/SQR(xoffset);
    b = (y2-y0)/(2.0f*xoffset);
    c = y1;
}

/////////////////////////////////////////////////////////////////////////////

void DTLib::FixThetaRange(float& Theta, const bool& bHalfPhase)
{
    if (bHalfPhase) {
        Theta = (float)fmod(Theta, CONST_PI);
        if (Theta < 0.0f) Theta += CONST_PI;
        ASSERT((Theta >= 0.0f) && (Theta <= CONST_PI));
    }
    else { // full phase
        Theta = (float)fmod(Theta, CONST_2PI);
        if (Theta < 0.0f) Theta += CONST_2PI;
        ASSERT((Theta >= 0.0f) && (Theta <= CONST_2PI));
    }
}

