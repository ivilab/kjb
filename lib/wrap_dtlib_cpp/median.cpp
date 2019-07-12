/////////////////////////////////////////////////////////////////////////////
// median.cpp - median filtering on an image
// Author: Doron Tal
// Date created: May, 2000

#include "l/l_sys_debug.h"  /* For ASSERT. */
#include <vector>
#include <algorithm>
#include "wrap_dtlib_cpp/median.h"

using namespace DTLib;

/////////////////////////////////////////////////////////////////////////////

void DTLib::MedianFilter(CImg<float>& InImg, 
                         CImg<float>& OutImg,
                         const int& Radius)
{
    const int Width = InImg.Width(), Height = InImg.Height();
    float *pIn = InImg.pBuffer(), *pOut = OutImg.pBuffer();
    vector<float> NeighborsVec;
    NeighborsVec.reserve(SQR(Radius));
    for (int y = 0; y < Height; y++) {
        for (int x = 0; x < Width; x++, pIn++, pOut++) {
            float* pOffset;
            for (int yy = -Radius; yy <= Radius; yy++) {
                const int YCoord = y+yy;
                if ((YCoord >= 0) && (YCoord < Height)) {
                    pOffset = pIn+yy*Width;
                    for (int xx = -Radius; xx <= Radius; xx++) {
                        const int XCoord = x+xx;
                        if ((XCoord >= 0) && (XCoord < Width)) {
                            // in bounds
                            NeighborsVec.push_back(*(pOffset+xx));
                        } // if ((XCoord >= 0) && (XCoord < Height)) {
                    } // for (int xx = -Radius; xx <= Radius; xx++) {
                } // if ((YCoord >= 0) && (YCoord < Height)) {
            } // for (int yy = -Radius; yy <= Radius; yy++) {
            ASSERT(NeighborsVec.size() > 0);
            const int nNbrs = NeighborsVec.size();
            const int iMedian = nNbrs/2;
            sort(NeighborsVec.begin(), NeighborsVec.end());
            *pOut = NeighborsVec[iMedian];
            if (IS_EVEN(nNbrs)) { // pick avg of two mid vals if even # elts
                *pOut += NeighborsVec[iMedian-1];
                *pOut *= 0.5;
            }
            NeighborsVec.clear();
        } // for (int x = 0; x < Width; x++) {
    } // for (int y = 0; y < Height; y++) {
}
