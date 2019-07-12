/////////////////////////////////////////////////////////////////////////////
// channels.cpp - separate a long image into many binary images 
// Author: Doron Tal
// Date Created: March, 2000

#include "l/l_sys_debug.h"  /* For ASSERT. */
/*
 * Kobus: We have run into trouble with 32 bit centric code in this
 * distribution. I have changed some long's to kjb_int32's.  The immediate
 * problem is that the segmentation maps can get written out as 64 bit integers. 
*/
#include "l/l_sys_def.h"

#include "wrap_dtlib_cpp/channels.h"

using namespace DTLib;
using namespace kjb_c;

/* -------------------------------------------------------------------------- */

void DTLib::SeparateChannels(CImg<kjb_int32>& InCombImg,
                             CImgVec<BYTE>& OutChannelVec,
                             const kjb_int32& nChannels)
{
    const int Width = InCombImg.Width();
    const int Height = InCombImg.Height();
    for (kjb_int32 i = 0; i < nChannels; i++) {
        BYTE* pChannel = OutChannelVec.pBuffer(i);
        kjb_int32* pComb = InCombImg.pBuffer();
        const kjb_int32 K = i;
        for (int y = 0; y < Height; y++) {
            for (int x = 0; x < Width; x++, pComb++, pChannel++) {
                if (*pComb == K) *pChannel = 1;
                else *pChannel = 0;
            } // for x
        } // for y
    } // for i
}
