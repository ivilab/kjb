/////////////////////////////////////////////////////////////////////////////
// sparsepat.h - CImg derived class to generate sparse sampling patterns
// Author: Doron Tal
// Date Created: April, 2000

#ifndef _SPARSEPAT_H
#define _SPARSEPAT_H

#include "wrap_dtlib_cpp/img.h"

namespace DTLib {

    class CSparsePatImg : public CImg<BYTE>
    {
    public:
        CSparsePatImg(const int& DenseRad, const int& MaxRad,
                      const int& nTotalSamples, const bool& bHalf = true,
                      const bool& bCenterPixOn = true);
    };

} // namespace DTLib {

#endif /* #ifndef _SPARSEPATTERN_H */
