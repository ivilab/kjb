/////////////////////////////////////////////////////////////////////////////
// convolve.h - image convolution
// Author: Doron Tal
// Date created: April, 2000

#include "wrap_dtlib_cpp/img.h"

#ifndef _CONVOLVE_H
#define _CONVOLVE_H

namespace DTLib {

    void Convolve(CImg<float>& InImg,
                  CImg<float>& KernelImg,
                  CImg<float>& OutImg);

};

#endif
