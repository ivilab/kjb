/////////////////////////////////////////////////////////////////////////////
// texturescale.cpp - compute texture scale
// Author: Doron Tal
// Date created: April, 2000

#include "l/l_sys_debug.h"  /* For ASSERT. */
#include <vector>
#include <algorithm>
#include <iostream>
#include <errno.h>
#include "wrap_dtlib_cpp/texturescale.h"
#include "l_cpp/l_exception.h"

using namespace std;
using namespace DTLib;

/////////////////////////////////////////////////////////////////////////////

CTextureScale::CTextureScale(const int maxPoints)
{
    m_pInputPts = new REAL[2*maxPoints];
    m_pEdges = new int[4*maxPoints];
}

/////////////////////////////////////////////////////////////////////////////

CTextureScale::~CTextureScale()
{
    zap(m_pInputPts);
    zap(m_pEdges);
}

/////////////////////////////////////////////////////////////////////////////
// Img is a texton channel
// ScaleImg is the result scale image

void CTextureScale::ComputeScaleImg(CImg<BYTE> &Img, CImg<float> &ScaleImg,
                                    const float& minDist, const float& maxDist,
                                    const float& MedianMultiplier)
{
    const int xstart = Img.ROIStartX(), xend = Img.ROIEndX();
    const int ystart = Img.ROIStartY(), yend = Img.ROIEndY();
    const int sk = Img.ROISkipCols();

    // fill up, read-in, the input points
    int nPoints = 0;
    BYTE *pBuf = Img.pROI();
    REAL *pInputPts = m_pInputPts;
    for (int y = ystart; y < yend; y++, pBuf += sk)
        for (int x = xstart; x < xend; x++, pBuf++)
            if (*pBuf) { 
                *pInputPts++ = (float)x;
                *pInputPts++ = (float)y;
                nPoints++;
            } // if (*pBuf) {

    if (nPoints < 3) {
        pInputPts = m_pInputPts;
        for (int i = 0; i < nPoints; i++) {
            const int X = F2I(*pInputPts++);
            const int Y = F2I(*pInputPts++);
            ScaleImg.SetPix(X, Y, minDist*MedianMultiplier);
        } // for (i = 0; i < nPoints; i++) {
    }
    else { // do DeLaunay
                
        // set up input and output interface to triangle
        struct triangulateio in, out;
        in.numberofpointattributes = 0;
        in.pointlist = m_pInputPts;
        in.pointmarkerlist = (int *)NULL;
        in.numberofpoints = nPoints;
        out.edgelist = m_pEdges;

        // Delaunay triangulation
        errno = 0;
        triangulate("QBPNEIze", &in, &out, NULL);

        if (errno != 0) {
        	KJB_THROW_2(kjb::KJB_error, "error in triangle.c:triangulate()");
        }
        
        vector<vector<float>*> vecDists;
        
        // nMaxEdges must be a number larger than the max poss. # of edges
        const int nMaxEdges = nPoints*6;// nPoints*3-6; // assume planar graph
        vecDists.resize(nMaxEdges);
        int i; 
        for (i = 0; i < nMaxEdges; i++) vecDists[i] = new vector<float>;
       
        int nEdges = out.numberofedges;
        for (i = 0; i < nEdges; i++) {
            const int iOut = 2*i;
            
            const int oe1 = out.edgelist[iOut];
            const int oe2 = out.edgelist[iOut+1];
            
            const int i1 = 2*oe1;    
            const int x1 = (int)in.pointlist[i1];
            const int y1 = (int)in.pointlist[i1+1];
    
            const int i2 = 2*oe2;
            const int x2 = (int)in.pointlist[i2];
            const int y2 = (int)in.pointlist[i2+1];

            int dxsqr = x2-x1; dxsqr *= dxsqr;
            int dysqr = y2-y1; dysqr *= dysqr;
    
            const float dist = (float)sqrt((float)dxsqr+(float)dysqr);

            // if there are pixels within the doughnut, push them in,
            // otherwise, we want to push only one: the max distance.

            if ((dist >= minDist) && (dist <= maxDist)) {
                // we're in the range..
                vecDists[oe1]->push_back(dist);
                vecDists[oe2]->push_back(dist);
            } // if ((dist >= minDist) && (dist <= maxDist)) {

        } // for (i = 0; i < nEdges; i++) {

        for (i = 0; i < nPoints; i++) {
            const int i2 = i*2;
            const int X = (int)(in.pointlist[i2]);
            const int Y = (int)(in.pointlist[i2+1]);

            if (vecDists[i]->size() == 0) {
                ScaleImg.SetPix(X, Y, minDist*MedianMultiplier);
            }
            else {
                sort(vecDists[i]->begin(), vecDists[i]->end());
                const int iMedian = vecDists[i]->size()/2;
                const float medianDist = (*vecDists[i])[iMedian];
                const float TextureScale = medianDist*MedianMultiplier;
                ASSERT(TextureScale != 0.0f);
                ScaleImg.SetPix(X, Y, TextureScale);
            } // else
        } // for (i = 0; i < nPoints; i++) {

        for (i = 0; i < nMaxEdges; i++) zap(vecDists[i]);        
    }
}
