/////////////////////////////////////////////////////////////////////////////
// reflect.h - reflect any type image into its region of interest
// Author: Doron Tal
// Date created: March, 2000

#ifndef _REFLECT_H
#define _REFLECT_H

#include <assert.h>
#include "wrap_dtlib_cpp/img.h"

namespace DTLib {

    // POSTCOND: centers InImg in OutImg and refelcts it, leaving
    // OutImg with the ROI set to exactly contain InImg.
    template <class T>
    void Reflect(CImg<T>& InImg, CImg<T>& OutImg) {
        const int InWidth = InImg.Width();
        const int InHeight = InImg.Height();
        const int WidthDiff = OutImg.Width()-InWidth;
        const int HeightDiff = OutImg.Height()-InHeight;
        assert(WidthDiff >= 0);
        assert(HeightDiff >= 0);
        const int HalfWidthDiff = WidthDiff/2;
        const int HalfHeightDiff = HeightDiff/2;
        OutImg.ChangeROI(HalfWidthDiff, HalfWidthDiff+InWidth,
                         HalfHeightDiff, HalfHeightDiff+InHeight);
        OutImg.CopyFromBuf(InImg.pBuffer());
        OutImg.ReflectToROI(); // here is where the reflect's done!
    };

} // namespace DTLib {

#endif /* #ifndef _REFLECT_H */
