/////////////////////////////////////////////////////////////////////////////
// subpix_localize.h - sub-pixel localization of edges
// Author: Doron Tal
// Date created: April, 2000

#ifndef _SUBPIX_LOCALIZE_H
#define _SUBPIX_LOCALIZE_H

#include "wrap_dtlib_cpp/img.h"

namespace DTLib {

    // *** TODO: *** need comment
    void SubpixelLocalization(const int& nGaussScales,
                              const int& nGaussOrientations,
                              CImgVec<float>& OESeq,
                              CImgVec<BYTE>& ThetaIdxSeq,
                              CImgVec<BYTE>& MaximaSeq,
                              CImgVec<float>& RhoStarSeq,
                              CImgVec<float>& ThetaSeq,
                              CImgVec<float>& XLocSeq,
                              CImgVec<float>& YLocSeq,
                              CImgVec<float>& ErrSeq);

} // namespace DTLib {

#endif /* #ifndef _SUBPIX_LOCALIZE_H */
