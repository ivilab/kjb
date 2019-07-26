/////////////////////////////////////////////////////////////////////////////
// textonhisto.h - compute an image of texton histograms
// Author: Doron Tal
// Date Created: April, 2000

#ifndef _TEXTONHISTO_H
#define _TEXTONHISTO_H

#include "wrap_dtlib_cpp/img.h"
#include "wrap_dtlib_cpp/histogram.h"
#include "wrap_dtlib_cpp/circle.h"
#include <string>

/*
 * Kobus: We have run into trouble with 32 bit centric code in this
 * distribution. I have changed some long's to kjb_int32's.  The immediate
 * problem is that the segmentation maps can get written out as 64 bit integers. 
*/
#include "l/l_sys_def.h"

#warning "[Code police] Do not put 'using namespace' in global scope of header."
using namespace kjb_c;

namespace DTLib {

    // *** TODO: *** need comments

    CImg<FloatCHistogramPtr>* MakeEmptyTextonHistoImg(const int& Width,
                                                      const int& Height,
                                                      const int& K);

    void ComputeTextonHistogramImg(CCircleMasks* pCircleMasks,
                                   FloatCImgPtr pTextureScaleImg,
                                   CImg<kjb_int32>* pTextonsImg,
                                   FloatCImgPtr pPTextureImg,
                                   CImg<FloatCHistogramPtr>* pTextonHistoImg,
                                   CImg<kjb_int32>* pPresegImg = NULL);

} // namespace DTLib {

#endif /* #ifndef _TEXTONHISTO_H */
