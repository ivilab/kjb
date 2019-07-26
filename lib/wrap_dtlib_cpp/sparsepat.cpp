/////////////////////////////////////////////////////////////////////////////
// sparsepat.cpp - CImg derived class to generate sparse pattern images
// Author: Doron Tal
// Date Created: April, 2000

#include "l/l_sys_debug.h"  /* For ASSERT. */
#include <string.h>
#include "wrap_dtlib_cpp/sparsepat.h"
#include "wrap_dtlib_cpp/circle.h"

////////////////////////////////////////////////
//Added by Prasad

//This is to test the hypothesis of uniform density of sampling
//from dense radius to max radius

//#define USE_UNIFORM_SAMPLING_DENSE_TO_MAX

////////////////////////////////////////////////

////////////////////////////////////////////////
//Added by Prasad

//This is to test the hypothesis of uniform density of sampling
//from dense radius to max radius

//#define USE_INVERSERADIUS_SAMPLING_DENSE_TO_MAX

////////////////////////////////////////////////


using namespace DTLib;

/////////////////////////////////////////////////////////////////////////////

DTLib::CSparsePatImg::CSparsePatImg(const int& DenseRad, const int& MaxRad,
                                    const int& nTotalSamples,
                                    const bool& bHalf,
                                    const bool& bCenterPixOn)
{
    const int Width = MaxRad*2+1;
    CImg<BYTE> OutImg(Width, Width, true, true);

    // compute the maximum number of pixels a circumference will ever have:
    int* pDirs = ComputeCircum(MaxRad, Width);
    const int MaxCirc = *pDirs;
    zap(pDirs);
    BYTE* aRandom = new BYTE[MaxCirc];
    ASSERT(aRandom != NULL);

    // first, fill up the pattern up to DenseRad, counting pts in cSamples
    const float FRad = (float)(DenseRad+1);
    BYTE* pBuffer = OutImg.pBuffer();
    int cSamples = 0;
    for (int y = 0, yc = -MaxRad; y < Width; y++, yc++) {
        for (int x = 0, xc = -MaxRad; x < Width; x++, xc++, pBuffer++) {
            float Dist = (float)sqrt((double)(SQR(xc)+SQR(yc)));
            if (Dist < FRad) {
                OutImg.SetPix(x, y, (BYTE)255);
                cSamples++;
            } // if (Dist < FRad) {
        } // for x
    } // for y


////////////////////////////////////////////////////////////////
//Added by Prasad

//This is to test the hypothesis of uniform density of sampling
//from dense radius to max radius

#if defined(USE_UNIFORM_SAMPLING_DENSE_TO_MAX)

//Compute the number of pixels in the circle corresponding to DenseRad
     int* pDirs1 = ComputeCircum(DenseRad, Width);
     const int DenseCirc = *pDirs1;
     zap(pDirs1);

//Fill the pattern from DenseRad to MaxRad uniformly

     for (int Rad = DenseRad+1; Rad <= MaxRad; Rad++) {
        // ComputeCircum() computes a chain code for a
        // circumference traversal of circumference corresponding
        // to a radius 'Rad' in an image with width 'Width':
        int* pDirs = ComputeCircum(Rad, Width);
        const int nCircumPix = *pDirs; // # of pixels in the current circum

            ASSERT( nCircumPix >= DenseCirc );

            int nRandSamples = 0;
            const float RandFactor = (float)nCircumPix/(float)RAND_MAX;
            memset(aRandom, 0, nCircumPix*sizeof(BYTE));
            while (nRandSamples < DenseCirc) {
                const int iRand =  (int)((float)rand()*RandFactor);
                if (aRandom[iRand] == 0) {
                    aRandom[iRand] = 1;
                    nRandSamples++;
                } // if (aRandom[iRand] == 0) {
            } // while (nRandSamples < nSamplesPerCircum) {

        // "lay" this vector upon the current circumference, i.e.
        // draw along the current circumference the randomly picked
        // nonzero pixels, according to aRandom, which determines
        // whether a pixel on the circumference should be non-zero or not
        BYTE* pCircum = (BYTE*)(OutImg.pBuffer()+MaxRad*Width+MaxRad-Rad);
        int* pDir = pDirs+1; // next direction to go to
        for (int c = 0; c < nCircumPix; c++, pDir++) {
            if (aRandom[c] != 0) {
                *pCircum = (BYTE)255;
                cSamples++;
            } // if (aRandom[c] != 0) {
            pCircum += *pDir;
        } // for (int c = 0; c < nCircumPix; c++, pDir++) {
        zap(pDirs);
    } // for (int Rad = DenseRad+1; Rad <= MaxRad; Rad++) {

    //Debugging
    cout << "Using uniform sampling from dense to max radius" << endl;
    cerr << "Total No. of samples used in the sparse pattern = " << cSamples << endl;

    // no longer need aRandom
    zap(aRandom);


//////////////////////////////////////////////////////////////////////
//This is to test the hypothesis of density of sampling
//from dense radius to max radius inversely proportaional to radius

#elif defined(USE_INVERSERADIUS_SAMPLING_DENSE_TO_MAX)

//Compute the number of pixels in the circle corresponding to DenseRad
       int* pDirs2 = ComputeCircum(DenseRad, Width);
       const int DenseCirc = *pDirs2;
       zap(pDirs2);

//Fill the pattern from DenseRad to MaxRad uniformly

     for (int Rad = DenseRad+1; Rad <= MaxRad; Rad++) {
        // ComputeCircum() computes a chain code for a
        // circumference traversal of circumference corresponding
        // to a radius 'Rad' in an image with width 'Width':
        int* pDirs = ComputeCircum(Rad, Width);
        const int nCircumPix = *pDirs; // # of pixels in the current circum

           ASSERT( nCircumPix >= DenseCirc );

       //Compute the number of pixels to be filled in the circle corresponding to current radius
       //according to inverse of (Rad - DenseRad)
            const int nSamplesRequired = (int)DenseCirc/(Rad - DenseRad);

            int nRandSamples = 0;
            const float RandFactor = (float)nCircumPix/(float)RAND_MAX;
            memset(aRandom, 0, nCircumPix*sizeof(BYTE));
            while (nRandSamples < nSamplesRequired) {
                const int iRand =  (int)((float)rand()*RandFactor);
                if (aRandom[iRand] == 0) {
                    aRandom[iRand] = 1;
                    nRandSamples++;
                } // if (aRandom[iRand] == 0) {
            } // while (nRandSamples < nSamplesPerCircum) {

        // "lay" this vector upon the current circumference, i.e.
        // draw along the current circumference the randomly picked
        // nonzero pixels, according to aRandom, which determines
        // whether a pixel on the circumference should be non-zero or not
        BYTE* pCircum = (BYTE*)(OutImg.pBuffer()+MaxRad*Width+MaxRad-Rad);
        int* pDir = pDirs+1; // next direction to go to
        for (int c = 0; c < nCircumPix; c++, pDir++) {
            if (aRandom[c] != 0) {
                *pCircum = (BYTE)255;
                cSamples++;
            } // if (aRandom[c] != 0) {
            pCircum += *pDir;
        } // for (int c = 0; c < nCircumPix; c++, pDir++) {
        zap(pDirs);
    } // for (int Rad = DenseRad+1; Rad <= MaxRad; Rad++) {

    //Debugging
    cout << "Using inverse radius sampling from dense to max radius" << endl;
    cerr << "Total No. of samples used in the sparse pattern = " << cSamples << endl;

    // no longer need aRandom
    zap(aRandom);

////////////////////////////////////////////////////////////////////////////////

#else

    for (int Rad = DenseRad+1; Rad <= MaxRad; Rad++) {
        // ComputeCircum() computes a chain code for a
        // circumference traversal of circumference corresponding
        // to a radius 'Rad' in an image with width 'Width':
        int* pDirs = ComputeCircum(Rad, Width);
        const int nCircumPix = *pDirs; // # of pixels in the current circum

        const int cSamplesLeftToTake = nTotalSamples-cSamples;
        const int cRadiiLeft = MaxRad-Rad+1;
        int nSamplesPerRad = cSamplesLeftToTake/cRadiiLeft;
        const int nRemainderSamples =
            cSamplesLeftToTake-nSamplesPerRad*cRadiiLeft;

        if (nRemainderSamples != 0) nSamplesPerRad++;
        if (nSamplesPerRad >= nCircumPix) {
            // total fillup
            memset(aRandom, 1, nCircumPix*sizeof(BYTE));
        }
        else {
            // partial fillup
            int nRandSamples = 0;
            const float RandFactor = (float)nCircumPix/(float)RAND_MAX;
            memset(aRandom, 0, nCircumPix*sizeof(BYTE));
            while (nRandSamples < nSamplesPerRad) {
                const int iRand =  (int)((float)rand()*RandFactor);
                if (aRandom[iRand] == 0) {
                    aRandom[iRand] = 1;
                    nRandSamples++;
                } // if (aRandom[iRand] == 0) {
            } // while (nRandSamples < nSamplesPerCircum) {
        } // else

        // "lay" this vector upon the current circumference, i.e.
        // draw along the current circumference the randomly picked
        // nonzero pixels, according to aRandom, which determines
        // whether a pixel on the circumference should be non-zero or not
        BYTE* pCircum = (BYTE*)(OutImg.pBuffer()+MaxRad*Width+MaxRad-Rad);
        int* pDir = pDirs+1; // next direction to go to
        for (int c = 0; c < nCircumPix; c++, pDir++) {
            if (aRandom[c] != 0) {
                *pCircum = (BYTE)255;
                cSamples++;
            } // if (aRandom[c] != 0) {
            pCircum += *pDir;
        } // for (int c = 0; c < nCircumPix; c++, pDir++) {
        zap(pDirs);
    } // for (int Rad = DenseRad+1; Rad <= MaxRad; Rad++) {

    // must have gone through all samples

    ASSERT(cSamples == nTotalSamples);

    // no longer need aRandom
    zap(aRandom);

#endif
////////////////////////////////////////////////


    const int HalfWidth = Width/2;
    const int iEndPixToZero = HalfWidth*Width+HalfWidth;

    if (bHalf) {
        for (int i = 0; i < iEndPixToZero; i++) {
            OutImg.pBuffer()[i] = 0;
        }
    }

    if (!bCenterPixOn) { // zero center pix
        OutImg.pBuffer()[iEndPixToZero] = 0;
    }

    Attach(OutImg.Detach(), Width, Width, true);
}
