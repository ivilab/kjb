/////////////////////////////////////////////////////////////////////////////
// weberlaw.h - weber law on filterbank convolutions (for textons)
// Author: Doron Tal
// Date Created: April, 2000

#ifndef _WEBERLAW_H
#define _WEBERLAW_H

#include "wrap_dtlib_cpp/img.h"

namespace DTLib {

    // perform Weber's Law, or contrast normalization, on the result
    // of filterbank convolutions 'InOutSeq'.  IN PLACE PROCESSING: the
    // input sequence is permanently modified.
    void WeberLaw(CImgVec<float> &InOutSeq, const float& WeberConst);

} // namespace DTLib {

#endif /* #ifndef _WEBERLAW_H */
