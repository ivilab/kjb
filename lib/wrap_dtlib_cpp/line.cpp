/////////////////////////////////////////////////////////////////////////////
// line.cpp - utils for drawing or traversing lines 
// Author: Franklin Antonio -- the integer routine was adapted from
// Graphics Gems, by Doron Tal
// Date last modified: April, 2000

#include "wrap_dtlib_cpp/line.h"

#include "wrap_dtlib_cpp/utils.h" /* for FloatToInt(...) */
#include <algorithm>
#include <vector>

using namespace std;
using namespace DTLib;

/* Faster Line Segment Intersection   */
/* Franklin Antonio                   */

/* return values */
#define DONT_INTERSECT 0
#define DO_INTERSECT   1
#define PARALLEL       2

bool DTLib::LinesIntersect(const int& x1, const int& y1, 
                    const int& x2, const int& y2,
                    const int& x3, const int& y3,
                    const int& x4, const int& y4)
{

    register int Ax,Bx,Cx,Ay,By,Cy,d,e,f, x1lo,x1hi,y1lo,y1hi;;

    Ax = x2-x1;
    Bx = x3-x4;

    if(Ax<0) {                                            /* X bound box test*/
        x1lo=x2; x1hi=x1;
    } else {
        x1hi=x2; x1lo=x1;
    }
    if(Bx>0) {
        if(x1hi < x4 || x3 < x1lo) {
            return false; } // result = DONT_INTERSECT; return; }
      
    } else {
        if(x1hi < x3 || x4 < x1lo) {
            return false; } // result = DONT_INTERSECT; return; }
    }

    Ay = y2-y1;
    By = y3-y4;

    if(Ay<0) {                                            /* Y bound box test*/
        y1lo=y2; y1hi=y1;
    } else {
        y1hi=y2; y1lo=y1;
    }
    if(By>0) {
        if(y1hi < y4 || y3 < y1lo) {
            return false; } // result = DONT_INTERSECT; return; }
    } else {
        if(y1hi < y3 || y4 < y1lo) {
            return false; } // result = DONT_INTERSECT; return; }
    }


    Cx = x1-x3;
    Cy = y1-y3;
    d = By*Cx - Bx*Cy;                                    /* alpha numerator*/
    f = Ay*Bx - Ax*By;                                    /* both denominator*/
    if(f>0) {                                             /* alpha tests*/
        if(d<0 || d>f) { return false; } // result = DONT_INTERSECT; return; }
    } else {
        if(d>0 || d<f) { return false; } // result = DONT_INTERSECT; return; }
    }

    e = Ax*Cy - Ay*Cx;                                    /* beta numerator*/
    if(f>0) {                                             /* beta tests*/
        if(e<0 || e>f) { return false; } // result = DONT_INTERSECT; return; }
    } else {
        if(e>0 || e<f) { return false; } // result = DONT_INTERSECT; return; }
    }

    if(f==0) { return false; } // result = PARALLEL; return; }

    return true;
}

/////////////////////////////////////////////////////////////////////////////
// uses the int routine above to do things in float

bool DTLib::LinesIntersect(const float& x1, const float& y1, 
                    const float& x2, const float& y2,
                    const float& x3, const float& y3,
                    const float& x4, const float& y4)
{
    vector<float> vecXCoords(4);
    vector<float> vecYCoords(4);

    vecXCoords[0] = x1;
    vecXCoords[1] = x2;
    vecXCoords[2] = x3;
    vecXCoords[3] = x4;

    vecYCoords[0] = y1;
    vecYCoords[1] = y2;
    vecYCoords[2] = y3;
    vecYCoords[3] = y4;

    vector<float>::iterator iIn;

    iIn = min_element(vecXCoords.begin(), vecXCoords.end());
    const float minX = *iIn;

    iIn = max_element(vecXCoords.begin(), vecXCoords.end());
    const float maxX = *iIn;

    iIn = min_element(vecYCoords.begin(), vecYCoords.end());
    const float minY = *iIn;

    iIn = max_element(vecYCoords.begin(), vecYCoords.end());
    const float maxY = *iIn;

    const int Resolution = 1024; // 1024 is max possible int resolution

    int x1i, x2i, x3i, x4i;
    int y1i, y2i, y3i, y4i;

    FloatToInt(x1, x1i, minX, maxX, Resolution);
    FloatToInt(x2, x2i, minX, maxX, Resolution);
    FloatToInt(x3, x3i, minX, maxX, Resolution);
    FloatToInt(x4, x4i, minX, maxX, Resolution);

    FloatToInt(y1, y1i, minY, maxY, Resolution);
    FloatToInt(y2, y2i, minY, maxY, Resolution);
    FloatToInt(y3, y3i, minY, maxY, Resolution);
    FloatToInt(y4, y4i, minY, maxY, Resolution);

    return LinesIntersect(x1i, y1i, x2i, y2i, x3i, y3i, x4i, y4i);
}

