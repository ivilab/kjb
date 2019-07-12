/////////////////////////////////////////////////////////////////////////////
// hilbert.h - hilbert transform
// Author: Doron Tal (this code is based on the matlab (v5.3) 'hilbert.m')
// Date created: March, 2000

#ifndef _HILBERT_H
#define _HILBERT_H

namespace DTLib {

    // In-place hilbert transform on aInputVector.
    void Hilbert(float *aInVector, const int Length);

} // namespace DTLib {

#endif /* #ifndef _HILBERT_H */
