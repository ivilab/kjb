/////////////////////////////////////////////////////////////////////////////
// channels.h - separate an image of longs into many binary images
// Author: Doron Tal
// Date Created: March, 2000

#ifndef _CHANNELS_H
#define _CHANNELS_H

#include "wrap_dtlib_cpp/img.h"

/*
 * Kobus: We have run into trouble with 32 bit centric code in this
 * distribution. I have changed some long's to kjb_int32's.  The immediate
 * problem is that the segmentation maps can get written out as 64 bit integers. 
*/
#include "l/l_sys_def.h"

#warning "[Code police] Do not put 'using namespace' in global scope of header."
using namespace kjb_c;


namespace DTLib {

    // PRECOND: (1) 'CombImg' is zero indexed and has consecutive integers
    // at each pixel, each pixel's integer designates the index of the region
    // to which that pixel belongs; (2) 'ChannelVec' is a BYTE image sequence
    // that has as many images as the # of regions in 'CombImg' and whose
    // images are all the same size as 'CombImg'.
    // POSTCOND: Fills up 'ChannelVec''s images with a single region per
    // image, all regions taken from 'CombImg', such that the region's
    // pixels are nonzero and everything else is zero.
    void SeparateChannels(CImg<kjb_int32>& InCombImg, CImgVec<BYTE>& OutChannelVec,
                          const kjb_int32& nSegments);

} // namespace DTLib {

#endif /* #ifndef _CHANNELS_H */
