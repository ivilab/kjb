// ======================================================================*
// Source for module FFT from package GO.
// Retrieved from NETLIB on Wed Jul  5 11:50:07 1995.
// ======================================================================*
// Converted to C++ and extended by Doron Tal.

/*--------------------------------*-C-*---------------------------------*
 * File:
 *  fftn.h
 * ---------------------------------------------------------------------*
 * Re[]:  real Value array
 * Im[]:  imaginary Value array
 * nTotal:  total number of complex Values
 * nPass: number of elements involved in this pass of transform
 * nSpan: nspan/nPass = number of bytes to increment pointer
 *    in Re[] and Im[]
 * isign: exponent: +1 = forward  -1 = reverse
 * scaling: normalizing constant by which the final result is *divided*
 *  scaling == -1, normalize by total dimension of the transform
 *  scaling <  -1, normalize by the square-root of the total dimension
 *
 * ----------------------------------------------------------------------
 * See the comments in the code for correct usage!
 */

#ifndef _FFT_H
#define _FFT_H

namespace DTLib {

    void fft_free ();

    /* double precision routine */
    int fftn (int ndim, const int dims[], double Re[], double Im[],
              int isign, double scaling);

    /* float precision routine */
    int fftnf (int ndim, const int dims[], float Re[], float Im[],
               int isign, double scaling);

    // 1-D fft interface -DT
    void FwdFFT(float *aRe, float *aIm, const int Size);
    void InvFFT(float *aRe, float *aIm, const int Size);

} // namespace DTLib {

#endif  /* _FFT_H */
