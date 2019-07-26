/////////////////////////////////////////////////////////////////////////////
// median.h - median filtering on an image
// Author: Doron Tal
// Date created: May, 2000

#ifndef _MEDIAN_H
#define _MEDIAN_H

#include "wrap_dtlib_cpp/img.h"

#warning "[Code police] Do not put 'using namespace' in global scope of header."
using namespace std;

namespace DTLib {

    void MedianFilter(CImg<float>& InImg,
                      CImg<float>& OutImg,
                      const int& Radius);

} // namespace DTLib {

#endif /* #ifndef _MEDIAN_H */
