/////////////////////////////////////////////////////////////////////////////
// circle.cpp - routine for fastest possible circle circumference traversal
// Author: Doron Tal
// Date created: January, 1993

#include "l/l_sys_debug.h"  /* For ASSERT. */
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "wrap_dtlib_cpp/img.h"
#include "wrap_dtlib_cpp/circle.h"
#include "l_cpp/l_exception.h"

using namespace DTLib;

/////////////////////////////////////////////////////////////////////////////
// CIRCLE MASKS
/////////////////////////////////////////////////////////////////////////////

DTLib::CCircleMasks::CCircleMasks(const int& MinRad, const int& MaxRad)
{
    m_MinRad = MinRad;
    m_nFrames = MaxRad-MinRad+1;

    for (int Rad = MinRad; Rad <= MaxRad; Rad++) {
        const int Width = 2*Rad+1;
        // allocate space
        ByteCImgPtr pCircleImg = new CImg<BYTE>(Width, Width, true, true);
        m_vecImgs.push_back(pCircleImg);
        BYTE* pCircle = pCircleImg->pBuffer();
        const float FRad = (float)Rad;
        for (int y = -Rad; y <= Rad; y++) {
            for (int x = -Rad; x <= Rad; x++, pCircle++) {
                const float Dist = (float)sqrt((double)(SQR(x)+SQR(y)));
                if (Dist <= FRad) { // in the circle
                    *pCircle = (BYTE)255;
                } // if (Dist <= FRad) { // in the circle
            } // for (int x = -Rad; x <= Rad; x++) {
        } // for (int y = -Rad; y <= Rad; y++) {
    } // for (int Rad = MinRad; Rad <= MaxRad; Rad++, iImg++) {
}

/////////////////////////////////////////////////////////////////////////////

BYTE* DTLib::CCircleMasks::GetCircleMaskBuffer(const int& Rad)
{
    return (m_vecImgs[Rad-m_MinRad])->pBuffer();
}

/////////////////////////////////////////////////////////////////////////////
// FAST CIRCLE CIRCUMFERECE TRAVERSAL
/////////////////////////////////////////////////////////////////////////////

#define ONE -1
#define TWO -Width-1
#define THREE -Width
#define FOUR -Width+1
#define FIVE 1
#define SIX Width+1
#define SEVEN Width
#define EIGHT Width-1

#define goto1 { tmp[numpix] = 1; cx--; }
#define goto2 { tmp[numpix] = 2; cx--; cy--; }
#define goto3 { tmp[numpix] = 3; cy--; }
#define goto4 { tmp[numpix] = 4; cx++; cy--; }
#define goto5 { tmp[numpix] = 5; cx++; }
#define goto6 { tmp[numpix] = 6; cx++; cy++; }
#define goto7 { tmp[numpix] = 7; cy++; } 
#define goto8 { tmp[numpix] = 8; cx--;cy++; }

/////////////////////////////////////////////////////////////////////////////

int DTLib::CircumLength(const int& radius)
{
    ASSERT(radius < 501);

    int pnPixels[501] =
    {0,8,12,16,24,28,32,40,44,52,56,64,
         68,72,80,84,92,96,100,108,112,120,124,132,136,140,148,152,
         160,164,168,176,180,188,192,196,204,208,216,220,228,232,236,
         244,248,256,260,264,272,276,284,288,296,300,304,312,316,324,
         328,332,340,344,352,356,364,368,372,380,384,392,396,400,408,
         412,420,424,428,436,440,448,452,460,464,468,476,480,488,492,
         496,504,508,516,520,528,532,536,544,548,556,560,564,572,576,
         584,588,592,600,604,612,616,624,628,632,640,644,652,656,660,
         668,672,680,684,692,696,700,708,712,720,724,728,736,740,748,
         752,760,764,768,776,780,788,792,796,804,808,816,820,824,832,
         836,844,848,856,860,864,872,876,884,888,892,900,904,912,916,
         924,928,932,940,944,952,956,960,968,972,980,984,988,996,1000,
         1008,1012,1020,1024,1028,1036,1040,1048,1052,1056,1064,1068,
         1076,1080,1088,1092,1096,1104,1108,1116,1120,1124,1132,1136,
         1144,1148,1152,1160,1164,1172,1176,1184,1188,1192,1200,1204,
         1212,1216,1220,1228,1232,1240,1244,1252,1256,1260,1268,1272,
         1280,1284,1288,1296,1300,1308,1312,1320,1324,1328,1336,1340,
         1348,1352,1356,1364,1368,1376,1380,1384,1392,1396,1404,1408,
         1416,1420,1424,1432,1436,1444,1448,1452,1460,1464,1472,1476,
         1484,1488,1492,1500,1504,1512,1516,1520,1528,1532,1540,1544,
         1548,1556,1560,1568,1572,1580,1584,1588,1596,1600,1608,1612,
         1616,1624,1628,1636,1640,1648,1652,1656,1664,1668,1676,1680,
         1684,1692,1696,1704,1708,1716,1720,1724,1732,1736,1744,1748,
         1752,1760,1764,1772,1776,1780,1788,1792,1800,1804,1812,1816,
         1820,1828,1832,1840,1844,1848,1856,1860,1868,1872,1880,1884,
         1888,1896,1900,1908,1912,1916,1924,1928,1936,1940,1944,1952,
         1956,1964,1968,1976,1980,1984,1992,1996,2004,2008,2012,2020,
         2024,2032,2036,2044,2048,2052,2060,2064,2072,2076,2080,2088,
         2092,2100,2104,2112,2116,2120,2128,2132,2140,2144,2148,2156,
         2160,2168,2172,2176,2184,2188,2196,2200,2208,2212,2216,2224,
         2228,2236,2240,2244,2252,2256,2264,2268,2276,2280,2284,2292,
         2296,2304,2308,2312,2320,2324,2332,2336,2340,2348,2352,2360,
         2364,2372,2376,2380,2388,2392,2400,2404,2408,2416,2420,2428,
         2432,2440,2444,2448,2456,2460,2468,2472,2476,2484,2488,2496,
         2500,2504,2512,2516,2524,2528,2536,2540,2544,2552,2556,2564,
         2568,2572,2580,2584,2592,2596,2604,2608,2612,2620,2624,2632,
         2636,2640,2648,2652,2660,2664,2672,2676,2680,2688,2692,2700,
         2704,2708,2716,2720,2728,2732,2736,2744,2748,2756,2760,2768,
         2772,2776,2784,2788,2796,2800,2804,2812,2816,2824,2828};

    return pnPixels[radius];
}

/////////////////////////////////////////////////////////////////////////////

int* DTLib::ComputeCircum (const int& radius, const int& Width)
{
    int x, y, cx, cy, *tmp, *resptr, numpix;
    double xsqr, ysqr, d1, d2, d3, d4, d5, d6, d7, d8;
  
    const int Length = CircumLength(radius)+1;
    
    if ((tmp = new int[Length]) == NULL) {
    	KJB_THROW_2(kjb::KJB_error, "circle.cpp:ComputeCircum()");
    }    
    // radius = 1 is a special case because we take all 8 neighbors then
    if (radius == 1) {
        numpix = 8;
        tmp[1] = 3;
        tmp[2] = 5;
        tmp[3] = 5;
        tmp[4] = 7;
        tmp[5] = 7;
        tmp[6] = 1;
        tmp[7] = 1;
        tmp[8] = 3;
    }
    else {
        // inv: (cx, cy) is the point you're at; tmp[numpix] is the
        // direction. NB: tmp is 1-indexed!  (direction is in
        // {1,..,8}) you've just gone to arrive at (cx, cy)

        // loop initialization: 
        cx = -radius;
        cy = 0;
        // compute tmp[1] "manually", initially it is only possible to 
        // go in directions 3 or 4 
        x = cx;
        y = cy-1;
        xsqr = SQR(x);
        ysqr = SQR(y);
        d3 = fabs(sqrt(xsqr+ysqr)-radius);
        x++;
        xsqr = SQR(x);
        d4 = fabs(sqrt(xsqr+ysqr)-radius);
    
        numpix = 1;
        if (d3 < d4) goto3 else goto4;
        while ((cx != -radius) || (cy != 0)) {
            switch(tmp[numpix]) {
            case 1 : 
                // just went to 1, can only go to 8, 1 or 2 
                x = cx-1;
                y = cy+1;
                xsqr = SQR(x);
                ysqr = SQR(y);
                d8 = fabs(sqrt(xsqr+ysqr)-radius);
                y--;
                ysqr = SQR(y);
                d1 = fabs(sqrt(xsqr+ysqr)-radius);
                y--;
                ysqr = SQR(y);
                d2 = fabs(sqrt(xsqr+ysqr)-radius);
                numpix++;
                if ((d8 < d1) && (d8 < d2)) goto8
                if ((d1 < d2) && (d1 < d8)) goto1
                if ((d2 < d1) && (d2 < d8)) goto2
                break;
            case 2 : 
                // can only go to 1 2 or 3 
                x = cx-1;
                y = cy;
                xsqr = SQR(x);
                ysqr = SQR(y);
                d1 = fabs(sqrt(xsqr+ysqr)-radius);
                y--;
                ysqr = SQR(y);
                d2 = fabs(sqrt(xsqr+ysqr)-radius);
                x++;
                xsqr = SQR(x);
                d3 = fabs(sqrt(xsqr+ysqr)-radius);
                numpix++;
                if ((d1 < d2) && (d1 < d3)) goto1
                if ((d2 < d1) && (d2 < d3)) goto2
                if ((d3 < d1) && (d3 < d2)) goto3
                break;
            case 3 : 
                // can only go to 2, 3 or 4
                x = cx-1;
                y = cy-1;
                xsqr = SQR(x);
                ysqr = SQR(y);
                d2 = fabs(sqrt(xsqr+ysqr)-radius);
                x++;
                xsqr = SQR(x);
                d3 = fabs(sqrt(xsqr+ysqr)-radius);
                x++;
                xsqr = SQR(x);
                d4 = fabs(sqrt(xsqr+ysqr)-radius);
                numpix++;
                if ((d2 < d3) && (d2 < d4)) goto2
                if ((d3 < d2) && (d3 < d4)) goto3
                if ((d4 < d2) && (d4 < d3)) goto4
                break;
            case 4 : 
                // can only go to 3, 4 or 5 
                x = cx;
                y = cy-1;
                xsqr = SQR(x);
                ysqr = SQR(y);
                d3 = fabs(sqrt(xsqr+ysqr)-radius);
                x++;
                xsqr = SQR(x);
                d4 = fabs(sqrt(xsqr+ysqr)-radius);
                y++;
                ysqr = SQR(y);
                d5 = fabs(sqrt(xsqr+ysqr)-radius);
                numpix++;
                if ((d3 < d4) && (d3 < d5)) goto3
                if ((d4 < d3) && (d4 < d5)) goto4
                if ((d5 < d3) && (d5 < d4)) goto5
                break;
            case 5 : 
                // can only go to 4, 5 or 6 
                x = cx+1;
                y = cy-1;
                xsqr = SQR(x);
                ysqr = SQR(y);
                d4 = fabs(sqrt(xsqr+ysqr)-radius);
                y++;
                ysqr = SQR(y);
                d5 = fabs(sqrt(xsqr+ysqr)-radius);
                y++;
                ysqr = SQR(y);
                d6 = fabs(sqrt(xsqr+ysqr)-radius);
                numpix++;
                if ((d4 < d5) && (d4 < d6)) goto4
                if ((d5 < d4) && (d5 < d6)) goto5
                if ((d6 < d4) && (d6 < d5)) goto6
                break;
            case 6 : 
                // can only go to 5, 6 or 7 
                x = cx+1;
                y = cy;
                xsqr = SQR(x);
                ysqr = SQR(y);
                d5 = fabs(sqrt(xsqr+ysqr)-radius);
                y++;
                ysqr = SQR(y);
                d6 = fabs(sqrt(xsqr+ysqr)-radius);
                x--;
                xsqr = SQR(x);
                d7 = fabs(sqrt(xsqr+ysqr)-radius);
                numpix++;
                if ((d5 < d6) && (d5 < d7)) goto5
                if ((d6 < d5) && (d6 < d7)) goto6
                if ((d7 < d5) && (d7 < d6)) goto7
                break;
            case 7 : 
                // can only go to 6, 7 or 8 
                x = cx+1;
                y = cy+1;
                xsqr = SQR(x);
                ysqr = SQR(y);
                d6 = fabs(sqrt(xsqr+ysqr)-radius);
                x--;
                xsqr = SQR(x);
                d7 = fabs(sqrt(xsqr+ysqr)-radius);
                x--;
                xsqr = SQR(x);
                d8 = fabs(sqrt(xsqr+ysqr)-radius);
                numpix++;
                if ((d6 < d7) && (d6 < d8)) goto6
                if ((d7 < d6) && (d7 < d8)) goto7
                if ((d8 < d6) && (d8 < d7)) goto8
                break;
            case 8 : 
                // can only go to 7, 8 or 1 
                x = cx;
                y = cy+1;
                xsqr = SQR(x);
                ysqr = SQR(y);
                d7 = fabs(sqrt(xsqr+ysqr)-radius);
                x--;
                xsqr = SQR(x);
                d8 = fabs(sqrt(xsqr+ysqr)-radius);
                y--;
                ysqr = SQR(y);
                d1 = fabs(sqrt(xsqr+ysqr)-radius);
                numpix++;
                if ((d7 < d8) && (d7 < d1)) goto7
                if ((d8 < d7) && (d8 < d1)) goto8
                if ((d1 < d7) && (d1 < d8)) goto1
                break;
            }
        }
    }
    *tmp = numpix;
    resptr = tmp+1;
    for (x = 0; x < numpix; x++, resptr++)
        switch(*resptr) {
        case 1 : *resptr = ONE; break;
        case 2 : *resptr = TWO; break;
        case 3 : *resptr = THREE; break;
        case 4 : *resptr = FOUR; break;
        case 5 : *resptr = FIVE; break;
        case 6 : *resptr = SIX; break;
        case 7 : *resptr = SEVEN; break; 
        case 8 : *resptr = EIGHT; break;
        }
    return (int*)tmp;
}
