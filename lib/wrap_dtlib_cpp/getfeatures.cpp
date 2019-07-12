
/////////////////////////////////////////////////////////////////////////////
// getfeatures.cpp - compute texture features 
// Author: Pinar Duygulu
// Date Created: May, 2001 

#define MAXREGNO 500

//added by PINAR
#include <stdio.h>
#include <math.h>
#include <string.h> 

/*
 * Kobus: We have run into trouble with 32 bit centric code in this
 * distribution. I have changed some long's to kjb_int32's.  The immediate
 * problem is that the segmentation maps can get written out as 64 bit integers. 
*/
#include "l/l_sys_def.h"

#include "wrap_dtlib_cpp/getfeatures.h"
#include "wrap_dtlib_cpp/img.h"
#include "wrap_dtlib_cpp/utils.h"
#include "wrap_dtlib_cpp/matlab.h"

using namespace DTLib;
using namespace kjb_c;

/////////////////////////////////////////////////////////////////////////////
// added by PINAR
void DTLib::GetTextureFeatures(int *pSegBuf,
                               int Width, 
                               int Height, 
                               int nGaussScales,
                               int nGaussOrientations,
                               CImgVec<float>& OEVec,
                               CImgVec<float>& ConvVec,
                               int nDOGScales,
                               kjb::Matrix & Oe_mean,
                               kjb::Matrix & Oe_var,
                               kjb::Matrix & DOG_mean,
                               kjb::Matrix & DOG_var
)
{

    int *pSeg;
    int counts[MAXREGNO]; 

    int Regno = 0;
    for (int i=0;i<MAXREGNO;i++)
      counts[i] = 0;
    pSeg = pSegBuf;
    for (int y = 0; y < Height; y++)
    {
        for (int x = 0; x < Width; x++)
        {
            if (*pSeg >0)
            {
              counts[*pSeg] ++;
              if (*pSeg > Regno) 
                 Regno = *pSeg;
            }
            pSeg ++;
        }
    }

#ifdef LONG_IS_64_BITS
    printf("\nRegion no = %d ", Regno);
#else
    printf("\nRegion no = %ld ", Regno);
#endif 

    float *pOE;
    float val;

    Oe_mean.zero_out(Regno, nGaussScales*nGaussOrientations);
    Oe_var.zero_out(Regno, nGaussScales*nGaussOrientations);
    DOG_mean.zero_out(Regno, nDOGScales);
    DOG_var.zero_out(Regno, nDOGScales);

    for (int i=0;i<Regno;i++)
    {  
        for (int j=0;j<nGaussScales*nGaussOrientations;j++)
        {
    	    Oe_mean(i, j) = 0.0;
    	    Oe_var(i, j) = 0.0;
        }
        for (int j = 0; j<nDOGScales; j++)
        {
            DOG_mean(i, j) = 0.0;
            DOG_var(i, j) = 0.0;
        }
        
    }

    FloatImgVecIter ppOEImg = OEVec.itImgs();
    for (int iScale = 0; iScale < nGaussScales; iScale ++)
    { 
        for (int iTheta = 0; iTheta < nGaussOrientations;iTheta++,ppOEImg++)
        {
            int OEIndex = iScale*nGaussOrientations+iTheta;
            //printf("%d ", OEIndex);
            pOE = (*ppOEImg)->pBuffer();
            pSeg = pSegBuf;
            for (int y = 0; y < Height; y++) {
                for (int x = 0; x < Width; x++) {
                   if (*pSeg > 0)
                   {
                    val = *pOE;
                    Oe_mean((*pSeg)-1, OEIndex) += val;
                    Oe_var((*pSeg)-1, OEIndex) += SQR(val);
                   }
                   pSeg++;
                   pOE++;
                     
                } // for (int x
            } // for (int y
            for (int i=0;i<Regno;i++)
            {
			  Oe_mean(i, OEIndex) /= ((float)counts[i+1]);
			  Oe_var(i, OEIndex) = (float)sqrt((double)((float)Oe_var(i, OEIndex)/counts[i+1]-SQR(Oe_mean(i, OEIndex))));
            }
        } // for (iTheta
    }


    FloatImgVecIter ppConvImg = ConvVec.itImgs();
    ppConvImg += (nGaussScales*nGaussOrientations*2);
    for (int iDOGScale = 0; iDOGScale < nDOGScales; iDOGScale++)
    {
        float *pDOG = (*ppConvImg)->pBuffer();
        pSeg = pSegBuf;
        for (int y = 0; y < Height; y++) {
          for (int x = 0; x < Width; x++) {
              if (*pSeg > 0)
              {
                val = *pDOG;
                DOG_mean((*pSeg)-1, iDOGScale) += val;
                DOG_var((*pSeg)-1, iDOGScale) += SQR(val);
              }
              pSeg++;
              pDOG++;
          } // for (int x
        } // for (int y

        for (int i=0;i<Regno;i++)
        {
            DOG_mean(i,iDOGScale) /= ((float)counts[i+1]);
            DOG_var(i, iDOGScale) = (float)sqrt((double)(DOG_var(i, iDOGScale)/counts[i+1]-SQR(DOG_mean(i, iDOGScale))));
        } 

        ppConvImg++;
    }

}

