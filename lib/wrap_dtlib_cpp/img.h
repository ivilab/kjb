/////////////////////////////////////////////////////////////////////////////
// img.h - general support for images and image sequences
// Author: Doron Tal
// Date Created: May, 1993
// Date Last Modified: Aug, 2000

/////////////////////////////////////////////////////////////////////////////
//
// Both classes in this file are template classes, therefore this
// header file contains implementation as well as declarations, no
// associated '.cpp' file exists.
//
// Author: Doron Tal
// Date Created: June, 1994

#ifndef _IMG_H
#define _IMG_H

#include <assert.h>
#include <memory.h>
#include <stdlib.h>
#include <vector>
#include <string>
#include "wrap_dtlib_cpp/utils.h"

/*
 * Kobus: We have run into trouble with 32 bit centric code in this
 * distribution. I have changed some long's to kjb_int32's.  The immediate
 * problem is that the segmentation maps can get written out as 64 bit integers. 
*/
#include "l/l_sys_def.h"

#warning "[Code police] Do not put 'using namespace' in global scope of header."
using namespace std;
#warning "[Code police] Do not put 'using namespace' in global scope of header."
using namespace kjb_c;


namespace DTLib {

    // forward declaration
    template <class T> class CImgVec;

    ///////////////////////////////////////////////////////////////////////////
    // Class CImg is a data structure for raster images supporting
    // regions of interest.  A few miscellaneous functions are
    // included: initialization, indexing, I/O, type conversions (byte
    // and float only are supported), basic math and thresholding
    // functions and more.

    template <class T> class CImg
    {
    public:
        ///////////////////////////////////////////////////////////////////////
        // CONSTRUCTORS / DESTRUCTORS
        ///////////////////////////////////////////////////////////////////////

        // Generic constructor. Create an empty image object.
        CImg();

        // Destructor
        ~CImg();

        // Create an image and allocate space for its buffer ('Width' x
        // 'Height).  The region of interest is the entire image.  If
        // 'bMakeBuf' is true, allocate image buffer memory without
        // initializing, otherwise do not allocate memory.  If 'bZero' is
        // true, and you are allocating memory, zero the memory.
        CImg(const int Width, const int Height,
             const bool bMakeBuf = true, const bool bZero = false);

        // Create image from a supplied Buffer 'pBuf' (size 'Height' x
        // 'Width').  If 'bCopy' is true, allocate a buffer and copy
        // 'pBuf' over, otherwise, just make the pointer to this's buffer
        // point to 'pBuf' and do not allocate.
        CImg(const T* pBuf, const int Width, const int Height,
             const bool bCopy = false);

        // Copy constructor - copy from given BYTE image 'Img'.  Create a
        // float or BYTE Img from a BYTE Img, 'iImg'.  If 'bCopy' is true,
        // copy (ROI only) from 'Img' into this, otherwise, allocate
        // memory for a new buffer without initializing it, just creating
        // an image that's the same in every respect (but its buffer) as
        // 'Img'.
        //        CImg(CImg<BYTE>& Img, const bool bCopy);

        // float version of above
        //        CImg(CImg<float>& Img, const bool bCopy);

        // as above, but generic
        CImg(CImg<T>& Img, const bool bCopy);

        // Copy constructor - create an image from the ROI of 'InImg',
        // taking as much from InImg as possible outside of this ROI so as
        // to pad e.g. if ROI of InImg is whole image, this will be just a
        // perfect replica of InImg.
        CImg(CImg<T> &InImg, const int& Padding = 0);

        // construct from a single frame of supplied sequence 'ImgVec'
        // 'bCopy' as above
        CImg(CImgVec<T>& ImgVec, const int& iFrame,
             const bool& bCopy = false);

        // concatenate together all frames of sequence and construct from
        // that, copying over data
        CImg(CImgVec<T>& ImgVec, const int&  nCols, const int& nRows);

        ///////////////////////////////////////////////////////////////////////
        // ACCESS FUNCTIONS
        ///////////////////////////////////////////////////////////////////////

        // Get a pixel Value, given coords.
        inline T GetPix(const int& x, const int& y);

        // get ith pixel, treating entire image as a vector
        inline T GetPix(const int& i);

        // Set a pixel Value, given coords.
        inline void SetPix(const int& x, const int& y, const T& Val);

        // same as above but using 'i', the pixel index into m_pBuffer,
        // PRECOND: 'i' in [0, m_nPixels)
        inline void SetPix(const int& i, const T& Val);

        inline void AddToPix(const int& x, const int& y, const T& Val);

        // Sets the pointer to the image buffer 'pBuf' (of size 'Height' x
        // 'Width').  If 'bCopy' is true, allocate a buffer and copy
        // 'pBuf' over, otherwise, just make the pointer to this's buffer
        // point to 'pBuf' and do not allocate.
        void Set_pBuffer(const T* pBuf, const int Width, const int Height,
                         const bool bCopy = false);

        // returns image Width
        inline int Width() const { return m_Width; }

        // returns image Height
        inline int Height() const { return m_Height; }

        // returns the number of pixels in the image
        inline int nPixels() const { return m_nPixels; }

        // returns whether this image's buffer should be deleted upon
        // exit or not
        inline bool bDelete() { return m_bDelete; }

        // returns pointer to the image buffer
        inline T* pBuffer() { return m_pBuffer; }

        // returns a pointer to the image region of interest (ROI)
        inline T* pROI() { return m_pROI; }

        // The following example will clarify the five member access
        // functions below -- they are used to construct loops that traverse
        // the ROI, loops on ROI should look like:
        //
        // T* pROI = Img.pROI();
        // for (y = Img.ROIStartY(); y < Img.ROIEndY();
        //      y++, pROI += Img.ROISkipCOls()) {
        //     for (x = Img.ROIStartX(); y < Img.ROIEndX(); x++, pROI++) {
        // ...
        inline int ROIStartX() { return m_ROIStartX; }
        inline int ROIEndX() { return m_ROIEndX; }
        inline int ROIStartY() { return m_ROIStartY; }
        inline int ROIEndY() { return m_ROIEndY; }
        inline int ROISkipCols() { return m_ROISkipCols; }

        // returns width of the region of interest
        inline int ROIWidth() { return m_ROIWidth; }

        // returns height of the region of interest
        inline int ROIHeight() { return m_ROIHeight; }

        // returns number we need to add to the image buffer's pointer
        // in order to point to the first pixel in the roi
        inline int ROIOffset() { return m_ROIOffset; }

        // returns the ROI size
        inline int ROISize() { return m_ROISize; }

        ///////////////////////////////////////////////////////////////////////
        // GENERAL DIAGNOSTICS FUNCTIONS
        ///////////////////////////////////////////////////////////////////////

        // report some info about *this
        void Report();

        // Is this Img empty or not?  'empty' means that the memory for the
        // Img Buffer (m_pBuffer) has not allocated.
        inline bool IsEmpty() { return (m_pBuffer == NULL); }
        inline bool IsNotEmpty() { return (m_pBuffer != NULL); }

        ///////////////////////////////////////////////////////////////////////
        // INPUT / OUTPUT FUNCTIONS
        ///////////////////////////////////////////////////////////////////////

        // Read and write from/to a file, return success status
        bool ReadRaw(const char* strFilename);
        bool WriteRaw(const char* strFilename);

        ///////////////////////////////////////////////////////////////////////
        // REGION OF INTEREST FUNCTIONS
        ///////////////////////////////////////////////////////////////////////

        // Clear region of interest (ROI).
        void ClearROI();

        // Change region of interest (ROI).
        void ChangeROI(const int StartX, const int EndX,
                       const int StartY, const int EndY);

        // Change region of interest (ROI).  If the ROI is outside the
        // Img, we crop it to fit the Img.
        void SafeChangeROI(const int StartX, const int EndX,
                           const int StartY, const int EndY);

        // Reduce ROI inwards from all sides by N pixels
        void ReduceROI(int N = 1);

        // Set the pixel Values of the background (outside of ROI) to 'Val'
        void SetOutROIVal(const T& Val);

        // As Above, but set separately for each rectangle
        void SetOutROIVal(const T& TopVal, const T& BotVal,
                          const T& LeftVal, const T& RightVal);

        // reflect image inside ROI into region outside ROI, reflecting
        // diagonally at the corners
        void ReflectToROI();

        // Set the pixel Values in ROI to 'Val'
        void SetROIVal(const T& Val);

        // Enlarge ROI as much as possible
        void OutPadROI(const int& Padding);

        ///////////////////////////////////////////////////////////////////////
        // IMAGE TYPE OPERATIONS
        ///////////////////////////////////////////////////////////////////////

        // PRECOND: ROI of this is the same width & height as ROI of
        // 'ByteImg'.
        // POSTCOND: Convert float image (this) into byte image 'ByteImg',
        // scaling everything to be in [0, 255].  Only works on float
        // images.  BGVal is assigned to all pixels in 'ByteImg' that are
        // not in its ROI.
        void f2b(CImg<BYTE> &ByteImg, BYTE BGVal = 0);

        // Same as Above, but here, instead of computing the Min and Max
        // of the float Img, we supply them as arguments, for speed.
        void f2b(CImg<BYTE> &ByteImg, float Min, float Max, BYTE BGVal = 0);

        // As Above, but just using casting, no rescaling done in
        // conversion.
        void f2bNoscale(CImg<BYTE> &ByteImg, BYTE BGVal = 0);

        // Convert byte image (this) into float image 'ByteImg' by casting.
        void b2f(CImg<float> &FloatImg, float BGVal = 0.0f);

        // Convert int image (this) into float image 'ByteImg' by casting.
        void i2f(CImg<float> &FloatImg, float BGVal = 0.0f);

        // ***TODO***: get rid of above two functions everywhere in the
        // library. this subsumes the above two functions
        void ToFloat(CImg<float> &FloatImg, float BGVal = 0.0f);

        ///////////////////////////////////////////////////////////////////////
        // IMAGE BUFFER OPERATIONS
        ///////////////////////////////////////////////////////////////////////

        // this can be used to reduce the size  of the image without having
        // to reallocate the buffer, using old buffer, just reset all other
        // image parameters to pretend we have a smaller buffer.  also, sets
        // ROI to full image.
        void ReduceSize(const int& NewWidth, const int& NewHeight);

        // reduce size of image inwards by N pixels
        void Crop(const int& N);

        // Clear out the Img - deallocate Buffer.
        void FreeMemory();

        // Copy pixel Values from a Buffer, ROI must agree with Buffer's
        // size, Height and Width.
        void CopyFromBuf(T* pBuf);

        // as above, but only copying while withing supplied bounds
        void CopyFromBuf(T* pBuf, const int& StartX, const int& EndX,
                         const int& StartY, const int& EndY);

        // Copy pixel Values to a Buffer, ROI must agree with Buffer's size,
        // Height and Width.
        void CopyToBuf(T* pBuf);

        // Attach the Img to a Buffer according to Img's member
        // functions assumes m_pBuffer doesn't point to an Img that needs
        // to be freed later.
        void Attach(T* pBuf);

        // As Above, but resetting all Img members
        void Attach(T* pBuf, const int Width, const int Height,
                    const bool bDelete = false);

        // returns the pointer to the image, sets the current pointer to
        // the image (i.e. this->m_pBuffer) to NULL
        T* Detach();

        ///////////////////////////////////////////////////////////////////////
        // VARIOUS STATISTICS
        ///////////////////////////////////////////////////////////////////////

        // Return the Minimum pixel index in 'x' and 'y' and the Value of
        // that pixel in 'Val' - searching in ROI only.
        void Minpix(int &x, int &y, T& Val);

        // Return the Maximum pixel index in 'x' and 'y' and the Value of
        // that pixel in 'Val' - searching in ROI only.
        void Maxpix(int &x, int &y, T& Val);

        // Same as Above, but doesn't return coordinates.
        void Maxpix(T& Val);

        // Return Both the Minimum and the Maximum Values (no coordinates).
        void MinMax(T& Min, T& Max);

        void ReportMinMax();

        // Return the mean Value of the ROI pixels.
        float MeanLuminance();

        // Return the mean luMinance in 'mu' and the variance in 'sig'
        // of the ROI pixels.
        void MuSig(float &mu, float &sig);

        // Returns the sum of absolute Values of all filters.
        float SumAbs();

        ///////////////////////////////////////////////////////////////////////
        // NORMALIZATION FUNCTIONS
        ///////////////////////////////////////////////////////////////////////

        // Normalize the Img (floats only) ROI by subtracting the ROI mean
        // from each pixel and dividing by the ROI Sigma.
        void Normalize(bool full_range = false);

        // Change the range of pixel Values in the Img linearly.
        void ChangeRange(const BYTE& NewMin, const BYTE& NewMax);
        void ChangeRange(const float& NewMin, const float& NewMax);

        void FixThetaRanges(const bool& bHalfPhase = true);

        // PRECOND: MuImg is the same as TImg w/no ROI - Both are size of
        // this's ROI.  Result is in MuImg = the mean filter of this of size
        // dx * dy.  This method uses (a) separable x and y mean filter
        // kernels, (b) running averages to compute the mean.
        void MuFilter(CImg<float> &MuImg, CImg<float> &TImg, int dx, int dy);

        // Wrapper for Above if you're only doing Above once in your program
        // and you don't need to reuse the temporary storage that's supplied
        // Above.
        void MuFilter(CImg<float> &MuImg, int dx, int dy);

        // Same as Above, but returns Sigma Img in SigImg as well, 'UImg'
        // is another temporary Img.
        void MuSigFilter(CImg<float> &MuImg, CImg<float> &SigImg,
                         CImg<float> &TImg, CImg<float> &UImg,
                         int dx, int dy);

        // Wrapper for Above if you're only doing Above once in your program
        // and you don't need to reuse the temporary storage that's supplied
        // Above.
        void MuSigFilter(CImg<float> &MuImg, CImg<float> &SigImg,
                         int dx,int dy);

        // Take this Img's and pad it with 'Left' zeros on the Left and
        // 'Top' zeros from the Top with Value 'Val'.  Now embed the Img
        // in 'DestImg' and zero-out the Bottom and Right areas where this
        // Img doesn't reach.
        void Pad(CImg<T>& DestImg, const int Left, const int Top,
                 const T& Val = (T)0);

        // Extract a portion of this Img, starting at x = 'Left' and y =
        // 'Top' and going to the Right 'DestImg.Width()' pixels and to the
        // Bottom 'DestImg.Height()' pixels.  The extracted region fills up
        // DestImg completely and it is Left with full ROI.
        void Extract(CImg<T> &DestImg, const int& Left = 0, const int& Top =0);

        void Square();

        void Zero();

        // Multiply 'Img' with this one (looking at ROI only). PRECOND: ROI
        // of 'Img' must be the same as this's ROI.  Result is returned in
        // 'Img'.  Works for float images only.
        void Mul(CImg<T>& Img);
        // Multiply all Values of this Img by Val and return result in
        // Val.  Works for float images only.
        void Mul(const T& Val);

        // Divide this by Img Img.
        void Div(CImg<T>& Img);
        // Divide this by Value Val.
        void Div(const T& Val);

        // Add Img to this.
        void Add(CImg<T>& Img);
        // Add Val to this.
        void Add(const T& Val);

        // Subtract Img from this.
        void Sub(CImg<T>& Img);
        // Subtract Val from this.
        void Sub(const T& Val);

        // Negate in place.
        void Neg();

        // Compute log(a+z), where 'a' and 'z' are *real* numbers and
        // 'a' is Value of each pixel in ROI, and 'z' is a user
        // supplied argument.
        void Log(const float& z = 0.0f);

        // Or 'this' with 'Img' (so ROIs must be the same size) and
        // return the result in this.  If the Img is a float Img, then
        // a Value exactly equal to 0.0 is considered false while
        // anything else is true.  If, at any pixel, one of the images
        // ('this' or 'Img') is nonzero, then asign this pixel in
        // 'this' the Value Val.
        void HardOR(CImg<T>& Img, const T& Val = (T)255);

        // Same as HardOR(), but rather than setting a true pixel to
        // some Value, keep the Value that made it true.
        void SoftOR(CImg<T>& Img);

        // Same as 'HardOR()' but it's an AND.
        void HardAND(CImg<T>& Img, const T& Val = (T)255);

        // Same as 'SoftOR()' but it's an AND.
        void SoftAND(CImg<T>& Img);

        // Pixels in 'Img' whose Value is Below 'Thresh' are set to
        // 'Below', other pixels are Left untouched.  ROI is
        // considered. In place version.
        //void SoftThresh(const T Thresh, const T Below = (T)0);

        // Pixels in 'Img' whose Value is Below 'Thresh' are set to
        // 'Below', other pixels are set to 'Above'.  ROI is
        // considered.  In place version.
        //void HardThresh(const T Thresh, const T Below = (T)0,
        //                const T Above = (T)255);

        // Just swap the pointers with the input Img's pointers, keep
        // all else the same.
        void SwapPointers(CImg<T> &InImg);

        // resize and affine transform to new size (with bilin. interp.)
        void SwapAll(CImg<T>& InImg);

        // Normalize ROI of Img by L1-norm of ROI
        void NormalizeL1();

        // Return the  L2-norm of pixels in ROI.
        float L2Norm();

        // Returns the number of non-zero elements in ROI
        kjb_int32 nNonZeros();

        // sqrt all values of image pixels in ROI
        void Sqrt();

        // converting values between 0 and infinity to a probability
        // bet. 0 and 1
        void Val2Probability(const float& Sigma);

        void CopyPixelsFromImgROI(CImg<T>& Img);

    protected:

        T* m_pBuffer;        // pointer to the Img
        int m_Width;         // number of columns in Img
        int m_Height;        // number of rows in Img
        int m_nPixels;       // number of pixels in Img
        bool m_bDelete;      // should we dealocate Buffer on destroy?

        // Region of Interest (ROI) data members
        T* m_pROI;           // points to ROI first pix (m_pBuffer+m_ROIOffset)
        int m_ROIStartX;     // first ROI col (indexed)
        int m_ROIStartY;     // first ROI row (indexed)
        int m_ROIEndX;       // last ROI column (not indexed)
        int m_ROIEndY;       // last ROI row (not indexed, for loops)
        int m_ROISkipCols;   // # of pixels to skip at end of y-loop

        int m_ROIWidth;      // number of columns in ROI
        int m_ROIHeight;     // number of rows in ROI
        int m_ROIOffset;     // ROI start-offset from first pixel of an Img
        int m_ROISize;       // number of pixels in ROI
    };

    ///////////////////////////////////////////////////////////////////////////
    // IMAGE VECTOR CLASS -- A SEQUENCE OF IMAGES
    ///////////////////////////////////////////////////////////////////////////

    // the class CImgVec is a much more basic data structure, used for
    // image sequences.

    template <class T> class CImgVec
    {
    public:
        CImgVec();

        CImgVec(T** ppBuffers, const int nFrames, const int Width,
                const int Height, bool bDelete = false);

        // subsection 'InImg' into nCols x nRows parts and create a
        // sequence out of the sections if 'Padding' is nonzero, add a
        // padded region around each stored image of the sequence, but
        // only if the padding extends into the image of 'InImg', and
        // when it extends out of the boundaries of InImg, then don't
        // pad there.  If 'Padding' is nonzero, the amount of padding
        // as described above is specified 'Padding', also, the ROIs
        // of the images in the sequence are set such that these ROIs
        // comprise exactly the image of 'InImg'.
        CImgVec(CImg<T>& InImg, const int& nCols, const int& nRows,
                const int& Padding = 0);

        ~CImgVec();

        ///////////////////////////////////////////////////////////////////////
        // ACCESS FUNCTIONS
        ///////////////////////////////////////////////////////////////////////

        inline int Width() { return m_Width; }
        inline int Height() { return m_Height; }
        inline int nFrames() {
            assert(m_nFrames == (int)m_vecImgs.size()); return m_nFrames; }
        inline int FrameSize() { return m_FrameSize; }
        inline int TotalSize() { return m_TotalSize; }

        inline CImg<T>* GetImg(const int& i) { return m_vecImgs[i]; }
        inline T* pBuffer(const int& i) {return m_vecImgs[i]->pBuffer(); }
        inline typename vector<CImg<T>*>::iterator itImgs() {return m_vecImgs.begin();}

        ///////////////////////////////////////////////////////////////////////

        void Empty();

        void Attach(T** ppBuffers, const int nFrames, const int Width,
                    const int Height, bool bDelete = false);

        bool Allocate(const int nFrames, const int Width, const int Height,
                      bool bZero = false);

        void FreeMemory();

        void CopyROI(CImg<T>& Img);

        // Change the range of pixel Values in the image linearly.
        void ChangeRange(const T& NewMin, const T& NewMax);

        void FixThetaRanges(const bool& bHalfPhase = true);

        bool IsEmpty();

    protected:

        int m_Width;
        int m_Height;
        int m_nFrames;
        int m_FrameSize;
        int m_TotalSize;

        vector<CImg<T>*> m_vecImgs;

        // Region of Interest (ROI) data members
        int m_ROIStartX;     // first ROI col (indexed)
        int m_ROIStartY;     // first ROI row (indexed)
        int m_ROIEndX;       // last ROI column (not indexed)
        int m_ROIEndY;       // last ROI row (not indexed, for loops)
        int m_ROIWidth;      // number of columns in ROI
        int m_ROIHeight;     // number of rows in ROI
        int m_ROISkipCols;   // # of pixels to skip at end of y-loop
        int m_ROIOffset;     // ROI start-offset from first pixel of an Img
        int m_ROISize;       // number of pixels in ROI

    };

    ///////////////////////////////////////////////////////////////////////////
    /// IMPLEMENTATION
    ///////////////////////////////////////////////////////////////////////////

    template <class T>
    void CImg<T>::Set_pBuffer(const T* pBuf, const int Width,
                              const int Height, const bool bCopy)
    {
        m_ROISize = m_nPixels = Height*Width;
        if (bCopy) {
            m_pBuffer = new T[m_nPixels];
            T* ip = (T*)pBuf;
            T* op = (T*)m_pBuffer;
            for (int i = 0; i < m_nPixels; i++, ip++, op++) *op = *ip;
            m_bDelete = true;
        } // if bCopy
        else {
            m_pBuffer = (T*)pBuf;
            m_bDelete = false;
        } // else
        m_pROI = m_pBuffer;
        m_ROIHeight = m_ROIEndY = m_Height = Height;
        m_ROIWidth = m_ROIEndX = m_Width = Width;
        m_ROISize = m_nPixels = Height*Width;
        m_ROIOffset = m_ROISkipCols = m_ROIStartX = m_ROIStartY = 0;
    }

    template <class T>
    CImg<T>::CImg()
    {
        m_Width = m_Height = m_ROIStartX = m_ROIEndX = m_ROIWidth =
            m_ROIStartY = m_ROIEndY = m_ROIHeight = m_ROISize =
            m_ROIOffset = m_ROISkipCols = m_nPixels = 0;
        m_bDelete = false;
        m_pBuffer = m_pROI = NULL;
    }

    template <class T>
    CImg<T>::~CImg()
    {
        if (m_bDelete) zap(m_pBuffer);
        
        ////////////////////////////////////////////
        //Added by Prasad
        //This is to check the correct usage of delete operator for arrays.
        //if (m_bDelete)
        //{
        //    if (m_pBuffer)
        //    {
        //        delete[] m_pBuffer;
        //        m_pBuffer = NULL;
        //        m_bDelete = false;
        //    }
        //}
    }

    ///////////////////////////////////////////////////////////////////////////
    // Create an image and allocate space for its Buffer Construct a new
    // 'Width' x 'Height' Img, with ROI = entire Img.  If 'bMakeBuf'
    // is true, allocate Img memory without initializing, otherwise
    // leave the m_pBuffer and m_pROI NULL;

    template <class T>
    CImg<T>::CImg(const int Width, const int Height,
                  const bool bMakeBuf, const bool bZero)
    {
        m_ROIHeight = m_ROIEndY = m_Height = Height;
        m_ROIWidth = m_ROIEndX = m_Width = Width;
        m_ROISize = m_nPixels = Height*Width;
        m_ROIOffset = m_ROISkipCols = m_ROIStartX = m_ROIStartY = 0;
        if (bMakeBuf) {
            m_pROI = (m_pBuffer = new T[m_nPixels]);
            m_bDelete = true;
            if (bZero) memset(m_pBuffer, 0, m_nPixels*sizeof(T));
        } // if bMakeBuf
        else {
            m_bDelete = false;
            m_pROI = (m_pBuffer = NULL);
        } // if bMakeBuf .. else
    }

    ///////////////////////////////////////////////////////////////////////////
    // Create Img from a supplied Buffer 'Buf' of Height 'Height' &
    // Width 'Width'. If 'bCopy' is true, first allocate a new Img
    // Buffer and then bCopy over the Img pointed to by 'pROI' into
    // it, otherwise, just make the created Img's pointer point to the
    // Buffer supplied in 'Buf'.  When the Img is destroyed, its
    // Buffer will not be deallocated if 'bCopy' == false, but will be
    // deallocated if 'bCopy' == true.

    template <class T>
    CImg<T>::CImg(const T* pBuf, const int Width, const int Height,
                  const bool bCopy)
    {
        Set_pBuffer(pBuf, Width, Height, bCopy);
    }

    ///////////////////////////////////////////////////////////////////////////
    // Copy constructor - copy from given BYTE Img 'Img'.  Create a
    // float or BYTE Img from a BYTE Img, 'iImg'.  If 'bCopy' is
    // true, copy (ROI only) from 'Img' into this, otherwise, allocate
    // memory for an Img without initializing it.
    // float version - copy from float Img to (this) float Img
    template <class T>
    inline CImg<T>::CImg(CImg<T>& TImg, const bool bCopy)
    {
        m_Height = TImg.Height();
        m_Width = TImg.Width();
        m_nPixels = TImg.nPixels();
        m_ROIStartY = TImg.ROIStartY();
        m_ROIStartX = TImg.ROIStartX();
        m_ROIEndY = TImg.ROIEndY();
        m_ROIEndX = TImg.ROIEndX();
        m_ROIHeight = TImg.ROIHeight();
        m_ROIWidth = TImg.ROIWidth();
        m_ROISkipCols = TImg.ROISkipCols();
        m_ROIOffset = TImg.ROIOffset();
        m_ROISize = TImg.ROISize();
        m_pBuffer = new T[m_nPixels];
        m_pROI = (T*)m_pBuffer+m_ROIOffset;
        m_bDelete = true;
        if (bCopy) {
            // copy the ROI of 'TImg' over, converting to Ts
            T *fp = TImg.pROI();
            T *myfp = m_pROI;
            for (int y = m_ROIStartY; y < m_ROIEndY;
                 y++, fp += m_ROISkipCols, myfp += m_ROISkipCols)
                for (int x = m_ROIStartX; x < m_ROIEndX;
                     x++, fp++, myfp++)
                    *myfp = *fp;
        } // if bCopy
    }

    ///////////////////////////////////////////////////////////////////////////

    // ***TODO***: get rid of code repetition bet. this function and one
    // below

    template <class T>
    inline CImg<T>::CImg(CImg<T>& InImg, const int& Padding)
    {
        const int InStartX = InImg.ROIStartX(), InStartY = InImg.ROIStartY();
        const int InEndX = InImg.ROIEndX(), InEndY = InImg.ROIEndY();

        // the following variables tell us how much (up to Padding) we are
        // allowed to go left, right, top, bottom in our padding
        int LeftPad,  TopPad, RightPad, BotPad;
        LeftPad = TopPad = RightPad = BotPad = Padding; // start assuming this
        if (m_ROIStartX-Padding < 0) LeftPad = m_ROIStartX; // too much to left
        if (m_ROIStartY-Padding < 0) TopPad = m_ROIStartY; //too much above
        if (m_ROIEndX+Padding > m_Width) RightPad = m_Width-m_ROIEndX; // right
        if (m_ROIEndY+Padding> m_Height) BotPad = m_Height-m_ROIEndY; //  below

        const int InROIWidth = InImg.ROIWidth();
        const int InROIHeight = InImg.ROIHeight();
        m_Width = InROIWidth+LeftPad+RightPad;
        m_Height = InROIHeight+TopPad+BotPad;
        m_nPixels = m_Width*m_Height;
        m_pBuffer = new T[m_nPixels];
        m_bDelete = true;
        ChangeROI(0, m_Width, 0, m_Height);
        InImg.ChangeROI(InStartX-LeftPad, InEndX+RightPad,
                        InStartY-TopPad, InEndY+BotPad);
        InImg.CopyToBuf(m_pBuffer);
        // change InImg ROI back to what it was..
        InImg.ChangeROI(InStartX, InEndX, InStartY, InEndY);
        // set the ROI of this to be the right one
        ChangeROI(LeftPad, LeftPad+InROIWidth, TopPad, TopPad+InROIHeight);
    }

    ///////////////////////////////////////////////////////////////////////////

    template <class T>
    inline void CImg<T>::OutPadROI(const int& Padding)
    {
        // the following variables tell us how much (up to Padding) we are
        // allowed to go left, right, top, bottom in our padding
        int LeftPad, TopPad, RightPad, BotPad;
        LeftPad = TopPad = RightPad = BotPad = Padding; // start assuming this
        if (m_ROIStartX-Padding < 0) LeftPad = m_ROIStartX; // too much to left
        if (m_ROIStartY-Padding < 0) TopPad = m_ROIStartY; //too much above
        if (m_ROIEndX+Padding > m_Width) RightPad = m_Width-m_ROIEndX; // right
        if (m_ROIEndY+Padding> m_Height) BotPad = m_Height-m_ROIEndY; //  below

        ChangeROI(m_ROIStartX-LeftPad, m_ROIEndX+RightPad,
                  m_ROIStartY-TopPad, m_ROIEndY+BotPad);
    }

    ///////////////////////////////////////////////////////////////////////////
    // concatenate together all frames of sequence and construct from
    // that, copying over data

    template <class T>
    inline CImg<T>::CImg(CImgVec<T>& ImgVec, const int& iFrame,
                         const bool& bCopy)
    {
        m_Height = ImgVec.Height();
        m_Width = ImgVec.Width();
        m_nPixels = m_Width*m_Height;
        m_ROIStartY = ImgVec.ROIStartY();
        m_ROIStartX = ImgVec.ROIStartX();
        m_ROIEndY = ImgVec.ROIEndY();
        m_ROIEndX = ImgVec.ROIEndX();
        m_ROIHeight = ImgVec.ROIHeight();
        m_ROIWidth = ImgVec.ROIWidth();
        m_ROISkipCols = ImgVec.ROISkipCols();
        m_ROIOffset = ImgVec.ROIOffset();
        m_ROISize = ImgVec.ROISize();
        if (bCopy) {
            // create buffer if we're to copy stuff
            m_pBuffer = new T[m_nPixels];
            // copy the ROI of 'ImgVec' over, converting to floats
            T* pSeq = ImgVec.m_vecImgs[iFrame]->m_pBuffer;
            T* pImg = m_pROI;
            for (int y = m_ROIStartY; y < m_ROIEndY;
                 y++, pSeq += m_ROISkipCols, pImg += m_ROISkipCols)
                for (int x = m_ROIStartX; x < m_ROIEndX;
                     x++, pSeq++, pImg++)
                    *pImg = *pSeq;
            m_bDelete = true;
        } // if bCopy
        else {
            m_pBuffer = ImgVec.m_vecImgs[iFrame]->m_pBuffer;
            m_bDelete = false;
        }
        m_pROI = m_pBuffer+m_ROIOffset;
    }

    ///////////////////////////////////////////////////////////////////////////

    // concatenate together all frames of sequence and construct from
    // that, copying over data

    template <class T>
    inline CImg<T>::CImg(CImgVec<T>& ImgVec,
                         const int& nCols, const int& nRows)
    {
        m_Width = 0;
        m_Height = 0;
        int i = 0;
        for (int row = 0; row < nRows; row++) {
            for (int col = 0; col < nCols; col++, i++) {
                if (row == 0) m_Width += (ImgVec.GetImg(i))->ROIWidth();
                if (col == 0) m_Height += (ImgVec.GetImg(i))->ROIHeight();
            }
        }

        m_nPixels = m_Width*m_Height;
        m_pBuffer = new T[m_nPixels];
        m_bDelete = true;
        m_pROI = m_pBuffer;
        m_ROIHeight = m_ROIEndY = m_Height;
        m_ROIWidth = m_ROIEndX = m_Width;
        m_ROISize = m_nPixels;
        m_ROIOffset = m_ROISkipCols = m_ROIStartX = m_ROIStartY = 0;
        i = 0;
        int StartX = 0, StartY = 0, EndX, EndY;
        for (int row = 0; row < nRows; row++) {
            for (int col = 0; col < nCols; col++, i++) {

                const int InROIWidth = (ImgVec.GetImg(i))->ROIWidth();
                const int InROIHeight = (ImgVec.GetImg(i))->ROIHeight();

                EndX = StartX+InROIWidth;
                EndY = StartY+InROIHeight;
                ChangeROI(StartX, EndX, StartY, EndY);

                const int InStartX = (ImgVec.GetImg(i))->ROIStartX();
                const int InStartY = (ImgVec.GetImg(i))->ROIStartY();
                const int InEndX = (ImgVec.GetImg(i))->ROIEndX();
                const int InEndY = (ImgVec.GetImg(i))->ROIEndY();
                const int InSkip = (ImgVec.GetImg(i))->ROISkipCols();
                T* pInROI = (ImgVec.GetImg(i))->pROI();
                T* pOutROI = m_pROI;
                for (int y = InStartY; y < InEndY; y++, pInROI += InSkip,
                         pOutROI += m_ROISkipCols) {
                    for (int x = InStartX; x < InEndX; x++, pInROI++,
                             pOutROI++) {
                        *pOutROI = *pInROI;
                    } // for (x ..
                } // for (y ..
                StartX = EndX;
            } // for (col ..
            StartX = 0;
            StartY = EndY;
        }
    }

    ///////////////////////////////////////////////////////////////////////////
    
    void report_int(const std::string& msg, int value);

    ///////////////////////////////////////////////////////////////////////////

    template <class T>
    inline void CImg<T>::Report()
    {
        report_int("m_Width = ",m_Width);
        report_int("m_Height = ",m_Height);
        report_int("m_nPixels = ",m_nPixels);
        report_int("m_ROIStartY = ",m_ROIStartY);
        report_int("m_ROIStartX = ",m_ROIStartX);
        report_int("m_ROIEndY = ",m_ROIEndY);
        report_int("m_ROIEndX = ",m_ROIEndX);
        report_int("m_ROIHeight = ",m_ROIHeight);
        report_int("m_ROIWidth = ",m_ROIWidth);
        report_int("m_ROISkipCols = ",m_ROISkipCols);
        report_int("m_ROIOffset = ",m_ROIOffset);
        report_int("m_ROISize = ",m_ROISize);
        report_int("m_pBuffer = ",m_pBuffer);
        report_int("m_bDelete = ",m_bDelete);
    }

    ///////////////////////////////////////////////////////////////////////////

    template <class T>
    T CImg<T>::GetPix(const int& x, const int& y)
    {
        return m_pBuffer[y*m_Width+x];
    }

    template <class T>
    T CImg<T>::GetPix(const int& i)
    {
        return m_pBuffer[i];
    }


    ///////////////////////////////////////////////////////////////////////////

    template <class T>
    void CImg<T>::SetPix(const int& x, const int& y, const T& Val)
    {
        m_pBuffer[y*m_Width+x] = Val;
    }

    ///////////////////////////////////////////////////////////////////////////

    template <class T>
    void CImg<T>::SetPix(const int& i, const T& Val)
    {
        m_pBuffer[i] = Val;
    }

    ///////////////////////////////////////////////////////////////////////////

    template <class T>
    void CImg<T>::AddToPix(const int& x, const int& y, const T& Val)
    {
        m_pBuffer[y*m_Width+x] += Val;
    }

    ///////////////////////////////////////////////////////////////////////////

    template <class T>
    void CImg<T>::FreeMemory()
    {
        if (IsNotEmpty()) {
            zap(m_pBuffer);
            m_pBuffer = m_pROI = NULL;
            m_bDelete = false;
        }
    }

    ///////////////////////////////////////////////////////////////////////////

    // Read and write from/to a file, return success status

    template <class T>
    bool CImg<T>::ReadRaw(const char* strFilename)
    {
        FILE *fh;
        if ((fh = fopen(strFilename, "r")) == NULL) return false;
        else return ((int)fread(m_pBuffer, sizeof(T), m_nPixels, fh) ==
                     m_nPixels);
    }

    template <class T>
    bool CImg<T>::WriteRaw(const char* strFilename)
    {
        FILE *fh;
        if ((fh = fopen(strFilename, "w")) == NULL) return false;
        else return ((int)fwrite(m_pBuffer,sizeof(T), m_nPixels, fh) ==
                     m_nPixels);
    }

    ///////////////////////////////////////////////////////////////////////////
    // Clear region of interest (ROI).

    template <class T>
    void CImg<T>::ClearROI()
    {
        m_ROIHeight = m_ROIEndY = m_Height;
        m_ROIWidth = m_ROIEndX = m_Width;
        m_nPixels = m_Width*m_Height;
        m_ROISize = m_nPixels;
        m_ROIOffset = m_ROISkipCols = m_ROIStartX = m_ROIStartY = 0;
        m_pROI = m_pBuffer;
    }

    ///////////////////////////////////////////////////////////////////////////

    template <class T>
    void CImg<T>::ReduceSize(const int& NewWidth, const int& NewHeight)
    {
        assert(m_Width >= NewWidth);
        assert(m_Height >= NewHeight);

        m_Height = m_ROIHeight = m_ROIEndY = NewHeight;
        m_Width = m_ROIWidth = m_ROIEndX = NewWidth;
        m_nPixels = m_ROISize = NewWidth*NewHeight;
        m_ROIOffset = m_ROISkipCols = m_ROIStartX = m_ROIStartY = 0;
        m_pROI = m_pBuffer;
    }

    ///////////////////////////////////////////////////////////////////////////
    // Change region of interest (ROI).

    template <class T>
    void CImg<T>::ChangeROI(const int StartX, const int EndX,
                            const int StartY, const int EndY)
    {
        m_ROIStartX = StartX;
        m_ROIEndX = EndX;
        m_ROIStartY = StartY;
        m_ROIEndY = EndY;
        m_ROIHeight = EndY-StartY;
        m_ROIWidth = EndX-StartX;
        m_ROISkipCols = m_Width-m_ROIWidth;
        m_ROIOffset = StartY*m_Width+StartX;
        m_ROISize = m_ROIHeight*m_ROIWidth;
        m_pROI = (T*)m_pBuffer+m_ROIOffset;
    }

    ///////////////////////////////////////////////////////////////////////////
    //Change region of interest (ROI).  If the ROI is outside the Img,
    //we crop it to fit the Img.

    template <class T>
    void CImg<T>::SafeChangeROI(int StartX, int EndX,
                                int StartY, int EndY)
    {
        if (StartX < 0) StartX = 0;
        if (EndX > m_Width) EndX = m_Width;
        if (StartY < 0) StartY = 0;
        if (EndY > m_Height) EndY = m_Height;
        m_ROIStartX = StartX;
        m_ROIEndX = EndX;
        m_ROIStartY = StartY;
        m_ROIEndY = EndY;
        m_ROIHeight = EndY-StartY;
        m_ROIWidth = EndX-StartX;
        m_ROISkipCols = m_Width-m_ROIWidth;
        m_ROIOffset = StartY*m_Width+StartX;
        m_ROISize = m_ROIHeight*m_ROIWidth;
        m_pROI = (T*)m_pBuffer+m_ROIOffset;
    }

    ///////////////////////////////////////////////////////////////////////////
    // Reduce ROI inwards from all sides by N pixels

    template <class T>
    void CImg<T>::ReduceROI(int N)
    {
        ChangeROI(m_ROIStartX+N, m_ROIEndX-N, m_ROIStartY+N, m_ROIEndY-N);
    }

    ///////////////////////////////////////////////////////////////////////////
    // Reduce ROI inwards from all sides by N pixels

    template <class T>
    void CImg<T>::Crop(const int& N)
    {
        ReduceROI(N);
        CImg<T> CroppedImg(m_ROIWidth, m_ROIHeight);
        CopyToBuf(CroppedImg.m_pBuffer);
        SwapPointers(CroppedImg);
        ReduceSize(m_ROIWidth, m_ROIHeight);
    }

    ///////////////////////////////////////////////////////////////////////////
    // Set the pixel Values of the ROI to 'Val'

    template <class T>
    void CImg<T>::SetROIVal(const T& Val)
    {
        T* pROI = m_pROI;
        for (int y = m_ROIStartY; y < m_ROIEndY;
             y++, pROI += m_ROISkipCols)
            for (int x = m_ROIStartX; x < m_ROIEndX; x++, pROI++)
                *pROI = Val;
    }

    ///////////////////////////////////////////////////////////////////////////
    // Set the pixel Values of the region outside the ROI to 'Val'

    template <class T>
    void CImg<T>::SetOutROIVal(const T& Val)
    {
        int yoffset, x, y;
        for (y = 0; y < m_ROIStartY; y++) { // Top horiz. rectangle
            yoffset = y*m_Width;
            for (x = 0; x < m_Width; x++)
                m_pBuffer[yoffset+x] = Val;
        }
        for (y = m_ROIEndY; y < m_Height; y++) { // Bot horiz. rectangle
            yoffset = y*m_Width;
            for (x = 0; x < m_Width; x++)
                m_pBuffer[yoffset+x] = Val;
        }
        for (y = m_ROIStartY; y < m_ROIEndY; y++) { // Left vert. rectangle
            yoffset = y*m_Width;
            for (x = 0; x < m_ROIStartX; x++)
                m_pBuffer[yoffset+x] = Val;
        }
        for (y = m_ROIStartY; y < m_ROIEndY; y++) { // Right vert. rectangle
            yoffset = y*m_Width;
            for (x = m_ROIEndX; x < m_Width; x++)
                m_pBuffer[yoffset+x] = Val;
        }
    }

    template <class T>
    void CImg<T>::SetOutROIVal(const T& TopVal, const T& BotVal,
                               const T& LeftVal, const T& RightVal)
    {
        int yoffset, x, y;
        for (y = 0; y < m_ROIStartY; y++) { // Top horiz. rectangle
            yoffset = y*m_Width;
            for (x = 0; x < m_Width; x++)
                m_pBuffer[yoffset+x] = TopVal;
        }
        for (y = m_ROIEndY; y < m_Height; y++) { // Bot horiz. rectangle
            yoffset = y*m_Width;
            for (x = 0; x < m_Width; x++)
                m_pBuffer[yoffset+x] = BotVal;
        }
        for (y = m_ROIStartY; y < m_ROIEndY; y++) { // Left vert. rectangle
            yoffset = y*m_Width;
            for (x = 0; x < m_ROIStartX; x++)
                m_pBuffer[yoffset+x] = LeftVal;
        }
        for (y = m_ROIStartY; y < m_ROIEndY; y++) { // Right vert. rectangle
            yoffset = y*m_Width;
            for (x = m_ROIEndX; x < m_Width; x++)
                m_pBuffer[yoffset+x] = RightVal;
        }
    }

    ///////////////////////////////////////////////////////////////////////////

    template <class T>
    void CImg<T>::ReflectToROI()
    {
        int yoffset, yoffset2, x, x2, y;
        const int StartX2 = m_ROIStartX+m_ROIStartX;
        const int StartY2 = m_ROIStartY+m_ROIStartY;
        const int EndX2 = m_ROIEndX+m_ROIEndX;
        const int EndY2 = m_ROIEndY+m_ROIEndY;

        for (y = 0; y < m_ROIStartY; y++) { // Top horiz. rectangle
            yoffset = y*m_Width;
            yoffset2 = (StartY2-y-1)*m_Width;
            for (x = 0; x < m_Width; x++)
                m_pBuffer[yoffset+x] = m_pBuffer[yoffset2+x];
        }
        for (y = m_ROIEndY; y < m_Height; y++) { // Bot horiz. rectangle
            yoffset = y*m_Width;
            yoffset2 = (EndY2-y-1)*m_Width;
            for (x = 0; x < m_Width; x++)
                m_pBuffer[yoffset+x] = m_pBuffer[yoffset2+x];
        }
        for (y = m_ROIStartY; y < m_ROIEndY; y++) { // Left vert. rectangle
            yoffset = y*m_Width;
            for (x = 0; x < m_ROIStartX; x++) {
                x2 = StartX2-x-1;
                m_pBuffer[yoffset+x] = m_pBuffer[yoffset+x2];
            }
        }
        for (y = m_ROIStartY; y < m_ROIEndY; y++) { // Right vert. rectangle
            yoffset = y*m_Width;
            for (x = m_ROIEndX; x < m_Width; x++) {
                x2 = EndX2-x-1;
                m_pBuffer[yoffset+x] = m_pBuffer[yoffset+x2];
            }
        }

        //////////////////////////////////////////////////////////////////

        for (y = 0; y < m_ROIStartY; y++) { // Top left box
            yoffset = y*m_Width;
            yoffset2 = (StartY2-y-1)*m_Width;
            for (x = 0; x < m_ROIStartX; x++) {
                x2 = StartX2-x-1;
                m_pBuffer[yoffset+x] = m_pBuffer[yoffset2+x2];
            }
        }

        for (y = m_ROIEndY; y < m_Height; y++) { // Bot left box
            yoffset = y*m_Width;
            yoffset2 = (EndY2-y-1)*m_Width;
            for (x = 0; x < m_ROIStartX; x++) {
                x2 = StartX2-x-1;
                m_pBuffer[yoffset+x] = m_pBuffer[yoffset2+x2];
            }
        }

        for (y = 0; y < m_ROIStartY; y++) { // Top right box
            yoffset = y*m_Width;
            yoffset2 = (StartY2-y-1)*m_Width;
            for (x = m_ROIEndX; x < m_Width; x++) {
                x2 = EndX2-x-1;
                m_pBuffer[yoffset+x] = m_pBuffer[yoffset2+x2];
            }
        }

        for (y = m_ROIEndY; y < m_Height; y++) { // Bot right box
            yoffset = y*m_Width;
            yoffset2 = (EndY2-y-1)*m_Width;
            for (x = m_ROIEndX; x < m_Width; x++) {
                x2 = EndX2-x-1;
                m_pBuffer[yoffset+x] = m_pBuffer[yoffset2+x2];
            }
        }
    }

    ///////////////////////////////////////////////////////////////////////////
    // Copy pixel Values from a Buffer, ROI must agree with Buffer's
    // size, Height and Width.

    template <class T>
    void CImg<T>::CopyFromBuf(T* pBuf)
    {
        T* pFrmBuf = pBuf;
        T* pROI = m_pROI;
        for (int y = m_ROIStartY; y < m_ROIEndY; y++, pROI += m_ROISkipCols)
            for (int x = m_ROIStartX; x < m_ROIEndX; x++, pROI++, pFrmBuf++)
                *pROI = *pFrmBuf;
    }

    ///////////////////////////////////////////////////////////////////////////

    template <class T>
    void CImg<T>::CopyFromBuf(T* pBuf, const int& StartX, const int& EndX,
                              const int& StartY, const int& EndY)
    {
        T* pFrmBuf = pBuf;
        T* pROI = m_pROI;
        for (int y = m_ROIStartY; y < m_ROIEndY; y++, pROI += m_ROISkipCols) {
            for (int x = m_ROIStartX; x < m_ROIEndX; x++, pROI++, pFrmBuf++) {
                if ((x >= StartX) &&
                    (x < EndX) &&
                    (y >= StartY) &&
                    (y < EndY)) {
                    *pROI = *pFrmBuf;
                }
            }
        }
    }

    ///////////////////////////////////////////////////////////////////////////
    // Copy pixel Values to a Buffer, ROI must agree with Buffer's
    // size, Height and Width.

    template <class T>
    void CImg<T>::CopyToBuf(T* pBuf)
    {
        T* pBuf2 = pBuf;
        T* pROI = m_pROI;
        for (int y = m_ROIStartY; y < m_ROIEndY; y++, pROI += m_ROISkipCols)
            for (int x = m_ROIStartX; x < m_ROIEndX; x++, pROI++, pBuf2++)
                *pBuf2 = *pROI;
    }

    ///////////////////////////////////////////////////////////////////////////
    // Attach the Img to a Buffer according to Img's member
    // functions assumes m_pBuffer doesn't point to an Img that needs
    // to be freed later.

    template <class T>
    inline void CImg<T>::Attach(T* pBuf)
    {
        m_pBuffer = pBuf;
        m_pROI = (T*)m_pBuffer+m_ROIOffset;
        m_bDelete = false;
    }

    ///////////////////////////////////////////////////////////////////////////
    // As Above, but resetting all Img members.

    template <class T>
    inline void CImg<T>::Attach(T* pBuf, const int Width, const int Height,
                                const bool bDelete)
    {
        m_ROIHeight = m_ROIEndY = m_Height = Height;
        m_ROIWidth = m_ROIEndX = m_Width = Width;
        m_ROISize = m_nPixels = Height*Width;
        m_ROIOffset = m_ROISkipCols = m_ROIStartX = m_ROIStartY = 0;
        if (m_bDelete) zap(m_pBuffer);
        m_pROI = m_pBuffer = pBuf;
        m_bDelete = bDelete;
    }

    ///////////////////////////////////////////////////////////////////////////

    template <class T>
    inline T* CImg<T>::Detach()
    {
        m_bDelete = false;
        return m_pBuffer;
    }

    ///////////////////////////////////////////////////////////////////////////
    // Convert float Img (this) into byte Img 'ByteImg', scaling according
    // to Max and Min Values.  Only works on float images, set pixels outside
    // ROI to 'BGVal'.

    template <> inline void CImg<float>::f2b(CImg<BYTE> &ByteImg,BYTE BGVal)
    {
        // compute Max and Min
        float *inp = m_pROI;
        float Max = CONST_MIN_FLOAT, Min = CONST_MAX_FLOAT, TmpF;
        int x, y;
        // compute Min and Max
        for (y = m_ROIStartY; y < m_ROIEndY; y++, inp += m_ROISkipCols)
            for (x = m_ROIStartX; x < m_ROIEndX; x++, inp++) {
                if ((TmpF = *inp) > Max) Max = TmpF;
                if (TmpF < Min) Min = TmpF;
            }
        // scale and convert to bytes
        inp = m_pROI;
        BYTE *pOut = ByteImg.pROI();
        const int pOut_rskip = ByteImg.ROISkipCols();
        float factor = 255.0f/(Max-Min);
        for (y = m_ROIStartY; y < m_ROIEndY;
             y++, inp += m_ROISkipCols, pOut += pOut_rskip)
            for (x = m_ROIStartX; x < m_ROIEndX; x++, inp++, pOut++)
                *pOut = (BYTE)((*inp-Min)*factor);
        ByteImg.SetOutROIVal(BGVal);
    }

    ///////////////////////////////////////////////////////////////////////////
    // As Above, but using only casting, no rescaling done in conversion.

    template <> inline void CImg<float>::f2bNoscale(CImg<BYTE> &ByteImg,BYTE BGVal)
    {
        // compute Max and Min
        float *inp = m_pROI;
        BYTE *pOut = ByteImg.pROI();
        int x, y;
        for (y = m_ROIStartY; y < m_ROIEndY;
             y++, inp += m_ROISkipCols, pOut += m_ROISkipCols)
            for (x = m_ROIStartX; x < m_ROIEndX; x++, inp++, pOut++)
                *pOut = (BYTE)*inp;
        ByteImg.SetOutROIVal(BGVal);
    }

    ///////////////////////////////////////////////////////////////////////////
    // Same as Above, but here, instead of computing the Min and Max of the
    // float Img, we supply them as arguments

    template <> inline void CImg<float>::f2b(CImg<BYTE> &ByteImg, float Min,
                                 float Max, BYTE BGVal)
    {
        // scale and convert to bytes
        float *inp = m_pROI;
        BYTE *pOut = ByteImg.pROI();
        float factor = 255.0f/(Max-Min);
        float m = Min;
        for (int y = m_ROIStartY; y < m_ROIEndY;
             y++, inp += m_ROISkipCols, pOut += m_ROISkipCols)
            for (int x = m_ROIStartX; x < m_ROIEndX; x++, inp++, pOut++) {
                *pOut = (BYTE)((*inp-m)*factor);
            }
        ByteImg.SetOutROIVal(BGVal);
    }

    ///////////////////////////////////////////////////////////////////////////
    // Convert byte Img (this) into float Img 'ByteImg' by casting.

    template <> inline void CImg<BYTE>::b2f(CImg<float> &FloatImg, float BGVal)
    {
        BYTE *inp = m_pROI;
        float *pOut = FloatImg.pROI();
        const int OutSkip = FloatImg.ROISkipCols();
        for (int y = m_ROIStartY; y < m_ROIEndY; y++, inp += m_ROISkipCols,
                 pOut += OutSkip)
            for (int x = m_ROIStartX; x < m_ROIEndX; x++, inp++, pOut++)
                *pOut = (float)*inp;
        FloatImg.SetOutROIVal(BGVal);
    }

    ///////////////////////////////////////////////////////////////////////////
    // Convert int Img (this) into float Img by casting.

    template <> inline void CImg<int>::i2f(CImg<float> &FloatImg, float BGVal)
    {
        int* pIn = m_pROI;
        float* pOut = FloatImg.pROI();
        for (int y = m_ROIStartY; y < m_ROIEndY;
             y++, pIn += m_ROISkipCols,
                 pOut += m_ROISkipCols)
            for (int x = m_ROIStartX; x < m_ROIEndX; x++, pIn++, pOut++)
                *pOut = (float)*pIn;
        FloatImg.SetOutROIVal(BGVal);
    }

    ///////////////////////////////////////////////////////////////////////////

    template <class T>
    void CImg<T>::ToFloat(CImg<float> &FloatImg, float BGVal)
    {
        T* pIn = m_pROI;
        float* pOut = FloatImg.pROI();
        for (int y = m_ROIStartY; y < m_ROIEndY; y++, pIn += m_ROISkipCols,
                 pOut += m_ROISkipCols)
            for (int x = m_ROIStartX; x < m_ROIEndX; x++, pIn++, pOut++)
                *pOut = (float)*pIn;
        FloatImg.SetOutROIVal(BGVal);
    }

    ///////////////////////////////////////////////////////////////////////////
    // Return the Minimum pixel index in 'x' and 'y' and the Value
    // of that pixel in 'Val' - searching in ROI only

    template <class T>
    void CImg<T>::Minpix(int &x, int &y, T& Val)
    {
        Val = *m_pROI; // make Val the first element in the ROI
        T* pROI = m_pROI;
        for (int yy = m_ROIStartY; yy < m_ROIEndY;
             yy++, pROI += m_ROISkipCols)
            for (int xx = m_ROIStartX; xx < m_ROIEndX; xx++, pROI++) {
                T* Tmp = *pROI;
                if (Tmp < Val) {
                    Val = Tmp;
                    x = xx;
                    y = yy;
                } // if
            } // for xx
    }

    ///////////////////////////////////////////////////////////////////////////
    // Return the Maximum pixel index in 'x' and 'y' and the Value
    // of that pixel in 'Val' - searching in ROI only

    template <class T>
    void CImg<T>::Maxpix(int &x, int &y, T& Val)
    {
        Val = *m_pROI; // make Val the first element in the ROI
        T* pROI = m_pROI;
        for (int yy = m_ROIStartY; yy < m_ROIEndY;
             yy++, pROI += m_ROISkipCols)
            for (int xx = m_ROIStartX; xx < m_ROIEndX; xx++, pROI++) {
                T Tmp = *pROI;
                if (Tmp > Val) {
                    Val = Tmp;
                    x = xx;
                    y = yy;
                } // if
            } // for xx
    }

    ///////////////////////////////////////////////////////////////////////////

    // Same as Above, but doesn't return coordinates.

    template <class T>
    void CImg<T>::Maxpix(T& Val)
    {
        Val = *m_pROI; // make Val the first element in the ROI
        T Tmp, *pROI = m_pROI;
        for (int yy = m_ROIStartY; yy < m_ROIEndY;
             yy++, pROI += m_ROISkipCols)
            for (int xx = m_ROIStartX; xx < m_ROIEndX; xx++, pROI++)
                if ((Tmp = *pROI) > Val) Val = Tmp;
    }

    ///////////////////////////////////////////////////////////////////////////
    // Return Both the Minimum and the Maximum Values (no coordinates).

    template <class T>
    void CImg<T>::MinMax(T& Min, T& Max)
    {
        Min = *m_pROI; // make Val the first element in the ROI
        Max = *m_pROI; // make Val the first element in the ROI
        T* pROI = m_pROI;
        for (int y = m_ROIStartY; y < m_ROIEndY;
             y++, pROI += m_ROISkipCols)
            for (int x = m_ROIStartX; x < m_ROIEndX; x++, pROI++) {
                T Tmp = *pROI;
                if (Tmp >= Max) Max = Tmp;
                if (Tmp < Min) Min = Tmp;
            } // for x
    }

    ///////////////////////////////////////////////////////////////////////////

    template <class T>
    void CImg<T>::ReportMinMax()
    {
        T Min = *m_pROI; // make Val the first element in the ROI
        T Max = *m_pROI; // make Val the first element in the ROI
        T* pROI = m_pROI;
        for (int y = m_ROIStartY; y < m_ROIEndY;
             y++, pROI += m_ROISkipCols) {
            for (int x = m_ROIStartX; x < m_ROIEndX; x++, pROI++) {
                T Tmp = *pROI;
                if (Tmp >= Max) Max = Tmp;
                if (Tmp < Min) Min = Tmp;
            } // for x
        }
        report_int("Min = ", Min);
        report_int("Max = ", Max);
    }

    ///////////////////////////////////////////////////////////////////////////
    // Return the mean Value of the ROI pixels.

    // BYTE version
    template <> inline float CImg<BYTE>::MeanLuminance()
    {
        BYTE* pROI = m_pROI;
        int sum = 0;
        for (int y = m_ROIStartY; y < m_ROIEndY;
             y++, pROI += m_ROISkipCols)
            for (int x = m_ROIStartX; x < m_ROIEndX; x++, pROI++)
                sum += (int)*pROI;
        return (float)sum/(float)m_ROISize;
    }

    // float version
    template <> inline float CImg<float>::MeanLuminance()
    {
        float* pROI = m_pROI;
        float sum = 0;
        for (int y = m_ROIStartY; y < m_ROIEndY;
             y++, pROI += m_ROISkipCols)
            for (int x = m_ROIStartX; x < m_ROIEndX; x++, pROI++)
                sum += *pROI;
        return sum/(float)m_ROISize;
    }

    ///////////////////////////////////////////////////////////////////////////
    // Return the mean luMinance in 'mu' and the variance in 'sig'
    // of the ROI pixels.

    // BYTE version
    template <> inline void CImg<BYTE>::MuSig(float &mu, float &sig)
    {
        BYTE* pROI = m_pROI;
        int imu = 0, isigsqr = 0;
        for (int yy = m_ROIStartY; yy < m_ROIEndY;
             yy++, pROI += m_ROISkipCols)
            for (int xx = m_ROIStartX; xx < m_ROIEndX; xx++, pROI++) {
                int TmpI = *pROI;
                imu += TmpI;
                isigsqr += SQR(TmpI);
            } // for x
        float count = (float)m_ROISize;
        mu = (float)imu/count;
        sig = (float)sqrt((double)((float)isigsqr/count-SQR(mu)));
    }

    // float version
    template <> inline void CImg<float>::MuSig(float &mu, float &sig)
    {
        float* pROI = m_pROI;
        float fmu = 0, fsigsqr = 0;
        for (int y = m_ROIStartY; y < m_ROIEndY;
             y++, pROI += m_ROISkipCols)
            for (int x = m_ROIStartX; x < m_ROIEndX; x++, pROI++) {
                float TmpF = *pROI;
                fmu += TmpF;
                fsigsqr += SQR(TmpF);
            } // for x
        float count = (float)m_ROISize;
        mu = fmu/count;
        sig = (float)sqrt((double)(fsigsqr/count-SQR(mu)));
    }

    ///////////////////////////////////////////////////////////////////////////
    // Return the mean luMinance in 'mu' and the variance in 'sig'
    // of the ROI pixels.

    // BYTE version
    template <> inline float CImg<BYTE>::SumAbs()
    {
        BYTE* pROI = m_pROI;
        kjb_int32 Sum;
        for (int yy = m_ROIStartY; yy < m_ROIEndY;
             yy++, pROI += m_ROISkipCols)
            for (int xx = m_ROIStartX; xx < m_ROIEndX; xx++, pROI++)
            {
                /* Kobus. Before 06/11/18, there was no "*" in front of pROI.
                 * This looks like a latent bug. so I added the "*".
                */
                Sum += (kjb_int32)(*pROI);
            }
                                     
        return (float)Sum;
    }

    // float version
    template <> inline float CImg<float>::SumAbs()
    {
        float* pROI = m_pROI;
        float Sum = 0.0f;
        for (int y = m_ROIStartY; y < m_ROIEndY;
             y++, pROI += m_ROISkipCols)
            for (int x = m_ROIStartX; x < m_ROIEndX; x++, pROI++) {
                float TmpF = *pROI;
                if (TmpF > 0.0f) Sum += TmpF;
                else Sum -= TmpF;
            } // for (int x = m_ROIStartX; x < m_ROIEndX; x++, pROI++) {
        return Sum;
    }

    ///////////////////////////////////////////////////////////////////////////
    // Normalize the Img (floats only) ROI by subtracting the ROI mean
    // from each pixel and dividing by the ROI Sigma.

    // float version
    template <> inline void CImg<float>::Normalize(bool full_range)
    {
        float mu, sig;
        MuSig(mu, sig);
        float* pROI = m_pROI;
        float factor = 1.0f/sig;
        for (int y = m_ROIStartY; y < m_ROIEndY;
             y++, pROI += m_ROISkipCols)
            for (int x = m_ROIStartX; x < m_ROIEndX; x++, pROI++)
                *pROI = (*pROI-mu)*factor;
    }

    // BYTE version
    template <> inline void CImg<BYTE>::Normalize(bool full_range)
    {
        if (full_range) { // not getting rid of any Values
            CImg<float> TmpFloatImg(m_Width, m_Height);
            TmpFloatImg.ChangeROI(m_ROIStartX, m_ROIEndX,
                                  m_ROIStartY, m_ROIEndY);
            b2f(TmpFloatImg);
            TmpFloatImg.Normalize(); // float normalization
            // now make sure that 0.0f in the float Img is 127.5 in the
            // byte Img, and stretch the Values as much as possible in
            // the byte Img so that the entire 0-255 range is used, at least
            // in one direction (Above or Below 127)
            float Min, Max; TmpFloatImg.MinMax(Min, Max);
            if (-Min > Max) Max = -Min; // now Max = largest-magnitude flank
            float* fp = TmpFloatImg.pROI();
            BYTE *bp = m_pROI;
            const float factor = 127.5f/Max;
            for (int y = m_ROIStartY; y < m_ROIEndY;
                 y++, fp += m_ROISkipCols, bp += m_ROISkipCols)
                for (int x = m_ROIStartX; x < m_ROIEndX; x++, fp++, bp++)
                    *bp = (BYTE)(*fp*factor+127.5f);
        } // if (full_range) ..
        else { // another rescaling method from floats to bytes, faster
            // but you may lose some pixels
            float mu, sig;
            MuSig(mu, sig);
            BYTE* pROI = m_pROI;
            float factor = 50.0f/sig;
            for (int y = m_ROIStartY; y < m_ROIEndY;
                 y++, pROI += m_ROISkipCols)
                for (int x = m_ROIStartX; x < m_ROIEndX; x++, pROI++) {
                    float TmpF = ((float)*pROI-mu)*factor+127.5f;
                    if (TmpF < 0.0f) *pROI = 0;
                    else if (TmpF > 255.0f) *pROI = 255;
                    else *pROI = (BYTE)TmpF;
                } // for x
        } // else
    }

    ///////////////////////////////////////////////////////////////////////////
    // Change the range of pixel Values in the Img linearly.

    // byte version *** UNTESTED ***
    template <> inline void CImg<BYTE>:: ChangeRange(const BYTE& NewMin,
                                         const BYTE& NewMax)
    {
        BYTE OldMinByte, OldMaxByte;
        float OldMin, NewMinR, OldMax, Factor;
        float OldRange, NewRange;
        BYTE* pROI;

        NewMinR = (float)NewMin;
        MinMax(OldMinByte, OldMaxByte);
        OldMin = (float)OldMinByte;
        OldMax = (float)OldMaxByte;
        OldRange = OldMax-OldMin;

        if (OldRange == 0) { // one-valued image
            pROI = m_pROI;
            const BYTE Val = (BYTE)((float)(NewMax-NewMin)*0.5f);
            for (int y = m_ROIStartY; y < m_ROIEndY;
                 y++, pROI += m_ROISkipCols)
                for (int x = m_ROIStartX; x < m_ROIEndX; x++, pROI++)
                    *pROI = Val;
        }
        else {
            NewRange = (float)NewMax-(float)NewMin;
            Factor = NewRange/OldRange;
            pROI = m_pROI;
            for (int y = m_ROIStartY; y < m_ROIEndY;
                 y++, pROI += m_ROISkipCols)
                for (int x = m_ROIStartX; x < m_ROIEndX; x++, pROI++)
                    *pROI = (BYTE)((*pROI-OldMin)*Factor+NewMin);
        }
    }

    // float version
    template <> inline void CImg<float>:: ChangeRange(const float& NewMin,
                                          const float& NewMax)
    {
        float OldMin, NewMinR, OldMax, Factor;
        float OldRange, NewRange;
        float* pROI;

        NewMinR = NewMin;
        MinMax(OldMin, OldMax);
        OldRange = OldMax-OldMin;

        if (OldRange == 0) { // one-valued image
            pROI = m_pROI;
            Factor = (NewMax-NewMin)/2.0f; // average of requested range
            for (int y = m_ROIStartY; y < m_ROIEndY;
                 y++, pROI += m_ROISkipCols)
                for (int x = m_ROIStartX; x < m_ROIEndX; x++, pROI++)
                    *pROI = Factor;
        }
        else {
            NewRange = NewMax-NewMin;
            Factor = NewRange/OldRange;
            pROI = m_pROI;
            for (int y = m_ROIStartY; y < m_ROIEndY;
                 y++, pROI += m_ROISkipCols)
                for (int x = m_ROIStartX; x < m_ROIEndX; x++, pROI++)
                    *pROI = (*pROI-OldMin)*Factor+NewMin;
        }
    }

    ///////////////////////////////////////////////////////////////////////////

    // float version
    template <> inline void CImg<float>:: FixThetaRanges(const bool& bHalfPhase)
    {
        float* pROI = m_pROI;
        for (int y = m_ROIStartY; y < m_ROIEndY;
             y++, pROI += m_ROISkipCols)
            for (int x = m_ROIStartX; x < m_ROIEndX; x++, pROI++)
                FixThetaRange(*pROI, bHalfPhase);
    }

    ///////////////////////////////////////////////////////////////////////////
    // PRECOND: MuImg is the same as TImg w/no ROI - Both are size of
    // this's ROI, TImg is a temporary integer Img.  Result is in
    // MuImg = the mean filter of this of size dx * dy.  This method
    // uses (a) separable x and y mean filter kernels, (b) running
    // averages to compute the mean.

    template <class T>
    void CImg<T>::MuFilter(CImg<float> &MuImg,
                           CImg<float> &TImg, int dx, int dy)
    {
        const int hdx = dx/2, hdx2 = (dx-1)/2, hdy = dy/2, hdy2 = (dy-1)/2;;
        T* ip = m_pROI;
        float *tp = TImg.m_pBuffer;
        for (int y = m_ROIStartY; y < m_ROIEndY;
             y++, ip += m_Width, tp += m_ROIWidth) {
            T* ip2 = ip;
            float mean = 0.0f;
            for (int i = 0; i < dx; i++, ip2++) mean += (float)*ip2;
            float *tp2 = tp+hdx;
            int x, xe = m_ROIEndX-hdx2;
            for (x = m_ROIStartX+hdx; x < xe; x++,  ip2++, tp2++) {
                *tp2 = mean;
                mean -= *(ip2-dx);
                mean += *ip2;
            } // for x
        } // for y
        tp = TImg.m_pBuffer;
        float *mup = MuImg.m_pBuffer;
        const float boxarea = (float)(dx*dy);
        for (int x = m_ROIStartX; x < m_ROIEndX; x++, tp++, mup++) {
            float *tp2 = tp, mean =  0.0f;
            int ys = m_ROIStartY+hdy, ye = m_ROIEndY-hdy2, i;
            for (i = 0; i < dy; i++, tp2 += m_ROIWidth) mean += (float)*tp2;
            float *mup2 = mup+hdy*m_ROIWidth;
            for (int y = ys; y < ye;
                 y++, tp2 += m_ROIWidth, mup2 += m_ROIWidth) {
                const int dyoff = dy*m_ROIWidth;
                *mup2 = (float)mean/boxarea;
                mean -= *(tp2-dyoff);
                mean += *tp2;
            } // for x
        } // for y
        // clear borders
        MuImg.ChangeROI(hdx, m_ROIWidth-hdx2, hdy, m_ROIHeight-hdy2);
        MuImg.SetOutROIVal(0.0f);
        MuImg.ChangeROI(0, m_ROIWidth, 0, m_ROIHeight);
    }

    // Wrapper for Above if you're only doing Above once in your program
    // and you don't need to reuse the temporary storage that's supplied
    // Above.

    template <class T>
    void CImg<T>::MuFilter(CImg<float> &MuImg, int dx, int dy)
    {
        CImg<float> TImg(MuImg.Width(), MuImg.Height());
        MuFilter(MuImg, TImg, dx, dy);
    }

    ///////////////////////////////////////////////////////////////////////////
    // Same as Above, but returns Sigma Img in SigImg as well, 'UImg' is
    // another temporary Img.

    template <class T>
    inline void CImg<T>::MuSigFilter(CImg<float> &MuImg, CImg<float> &SigImg,
                                     CImg<float> &TImg, CImg<float> &UImg,
                                     int dx, int dy)
    {
        const int hdx = dx/2, hdy = dy/2;
        T* ip = m_pROI;
        float *tp = TImg.m_pBuffer, *up = UImg.m_pBuffer;;
        for (int y = m_ROIStartY; y < m_ROIEndY;
             y++, ip += m_Width, tp += m_ROIWidth, up += m_ROIWidth) {
            T* ip2 = ip;
            float mean = 0.0f, sig = 0.0f;
            for (int i = 0; i < dx; i++, ip2++) {
                float TmpF = (float)*ip2;
                mean += TmpF;
                sig += SQR(TmpF);
            } // for i
            float *tp2 = tp+hdx;
            float *up2 = up+hdx;
            int x, xe = m_ROIEndX-dx;
            for (x = 0; x < xe; x++,  ip2++, tp2++, up2++) {
                *tp2 = mean;
                *up2 = sig;
                float before = (float)*(ip2-dx), after = (float)*ip2;
                mean -= before;
                mean += after;
                sig -= SQR(before);
                sig += SQR(after);
            } // for x
        } // for y
        tp = TImg.m_pBuffer;
        up = UImg.m_pBuffer;
        float *mup = MuImg.m_pBuffer;
        float *sigp = SigImg.m_pBuffer;
        const float boxarea = (float)(dx*dy);
        for (int x = m_ROIStartX; x < m_ROIEndX; x++, tp++,mup++,up++,sigp++) {
            float *tp2 = tp, *up2 = up, mean =  0.0f, sig = 0.0f;
            int ye = m_ROIEndY-dy, i;
            for (i = 0; i < dy; i++, tp2 += m_ROIWidth, up2 += m_ROIWidth) {
                mean += *tp2;
                sig += *up2;
            } // for i
            float *mup2 = mup+hdy*m_ROIWidth;
            float *sigp2 = sigp+hdy*m_ROIWidth;
            for (int y = 0; y < ye; y++, tp2 += m_ROIWidth,mup2 += m_ROIWidth,
                     up2 += m_ROIWidth, sigp2 += m_ROIWidth) {
                const int dyoff = dy*m_ROIWidth;
                float mu = (float)mean/boxarea;
                *mup2 = mu;
                *sigp2 = (float)sqrt((double)(sig/boxarea-SQR(mu)));
                mean -= *(tp2-dyoff);
                mean += *tp2;
                sig -= *(up2-dyoff);
                sig += *up2;
            } // for x
        } // for y
        // clear borders
        MuImg.ChangeROI(hdx, m_ROIWidth-dx+hdx, hdy, m_ROIHeight-dy+hdy);
        MuImg.SetOutROIVal(0.0f);
        MuImg.ChangeROI(0, m_ROIWidth, 0, m_ROIHeight);
        SigImg.ChangeROI(hdx, m_ROIWidth-dx+hdx, hdy, m_ROIHeight-dy+hdy);
        SigImg.SetOutROIVal(0.0f);
        SigImg.ChangeROI(0, m_ROIWidth, 0, m_ROIHeight);
    }

    // Wrapper for Above if you're only doing Above once in your program
    // and you don't need to reuse the temporary storage that's supplied
    // Above.

    template <class T>
    void CImg<T>::MuSigFilter(CImg<float> &MuImg, CImg<float> &SigImg,
                              int dx, int dy)
    {
        int w = MuImg.Width(), h = MuImg.Height();
        CImg<float> TImg(w, h);
        CImg<float> UImg(w, h);
        MuSigFilter(MuImg, SigImg, TImg, UImg, dx, dy);
    }

    ///////////////////////////////////////////////////////////////////////////
    // Take this Img's and pad it with 'Left' zeros on the Left
    // and 'Top' zeros from the Top with Value 'Val'.  Now embed the
    // Img in 'DestImg' and zero-out the Bottom and Right areas
    // where this Img doesn't reach.

    template <class T>
    void CImg<T>::Pad(CImg<T> &DestImg, const int Left, const int Top,
                      const T& Val)
    {
        const int StartX = DestImg.ROIStartX(), StartY = DestImg.ROIStartY(),
            EndX = DestImg.ROIEndX(), EndY = DestImg.ROIEndY();
        DestImg.ChangeROI(Left, Left+m_Width, Top, Top+m_Height);
        DestImg.CopyFromBuf(m_pBuffer);
        DestImg.SetOutROIVal(Val);
        DestImg.ChangeROI(StartX, EndX, StartY, EndY);
    }

    ///////////////////////////////////////////////////////////////////////////
    // Extract a portion of this Img, starting at x = 'Left' and
    // y = 'Top' and going to the Right 'DestImg.Width()' pixels
    // and to the Bottom 'DestImg.Height()' pixels.  The extracted
    // region fills up DestImg completely and it is Left with full ROI.

    template <class T>
    void CImg<T>::Extract(CImg<T> &DestImg, const int& Left, const int& Bot)
    {
        const int StartX = m_ROIStartX, StartY = m_ROIStartY,
            EndX = m_ROIEndX, EndY = m_ROIEndY;
        ChangeROI(Left, Left+DestImg.Width(), Bot, Bot+DestImg.Height());
        CopyToBuf(DestImg.m_pBuffer);
        ChangeROI(StartX, EndX, StartY, EndY);
    }

    ///////////////////////////////////////////////////////////////////////////

    template <> inline void CImg<float>::Square()
    {
        float *pROI = m_pROI;
        const int sk = m_ROISkipCols;
        for (int y = m_ROIStartY; y < m_ROIEndY; y++, pROI += sk)
            for (int x = m_ROIStartX; x < m_ROIEndX; x++, pROI++) {
                const float Val = *pROI;
                *pROI *= Val;;
            }
    }

    ///////////////////////////////////////////////////////////////////////////

    template <class T>
    void CImg<T>::Zero()
    {
        memset(m_pBuffer, 0, m_Width*m_Height*sizeof(T));
    }

    ///////////////////////////////////////////////////////////////////////////
    // Multiply 'Img' with this one (looking at ROI only). PRECOND:
    // ROI of 'Img' must be the same Width and Height as this's ROI.
    // Result is returned in this.  Works for T images only.

    template <class T>
    void CImg<T>::Mul(CImg<T>& Img)
    {
        T *pROI = m_pROI, *pOtherROI= Img.pROI();
        int x, y, EndX = m_ROIEndX, sk1 = m_ROISkipCols,
            sk2 = Img.ROISkipCols();
        for (y = m_ROIStartY; y < m_ROIEndY; y++, pROI += sk1, pOtherROI+= sk2)
            for (x = m_ROIStartX; x < EndX; x++, pROI++, pOtherROI++)
                *pROI *= *pOtherROI;
    }

    ///////////////////////////////////////////////////////////////////////////
    // Multiply all Values of this Img by Val and return result in
    // this.  Works for T images only.

    template <class T>
    void CImg<T>::Mul(const T& Val)
    {
        T *pROI = m_pROI;
        int x, y, EndX = m_ROIEndX, EndY = m_ROIEndY, sk = m_ROISkipCols;
        for (y = m_ROIStartY; y < EndY; y++, pROI += sk)
            for (x = m_ROIStartX; x < EndX; x++, pROI++)
                *pROI *= Val;
    }

    ///////////////////////////////////////////////////////////////////////////
    // Divide this by Img Img.

    template <class T>
    void CImg<T>::Div(CImg<T>& Img)
    {
        T *pROI = m_pROI, *pOtherROI= Img.pROI();
        int x, y, EndX = m_ROIEndX,
            sk1 = m_ROISkipCols, sk2 = Img.ROISkipCols();
        for (y = m_ROIStartY; y < m_ROIEndY; y++, pROI += sk1, pOtherROI+= sk2)
            for (x = m_ROIStartX; x < EndX; x++, pROI++, pOtherROI++)
                *pROI /= *pOtherROI;
    }

    ///////////////////////////////////////////////////////////////////////////
    // Divide this by Value Val.

    template <class T>
    void CImg<T>::Div(const T& Val)
    {
        T *pROI = m_pROI;
        int x, y, EndX = m_ROIEndX, EndY = m_ROIEndY, sk = m_ROISkipCols;
        const float factor = 1.0f/Val;
        for (y = m_ROIStartY; y < EndY; y++, pROI += sk)
            for (x = m_ROIStartX; x < EndX; x++, pROI++)
                *pROI *= factor;
    }

    ///////////////////////////////////////////////////////////////////////////
    // Add Img to this.

    template <class T>
    void CImg<T>::Add(CImg<T>& Img)
    {
        T *pROI = m_pROI, *pOtherROI= Img.pROI();
        int x, y, EndX = m_ROIEndX,
            sk1 = m_ROISkipCols, sk2 = Img.ROISkipCols();
        for (y = m_ROIStartY; y < m_ROIEndY; y++, pROI += sk1, pOtherROI+= sk2)
            for (x = m_ROIStartX; x < EndX; x++, pROI++, pOtherROI++)
                *pROI += *pOtherROI;
    }

    ///////////////////////////////////////////////////////////////////////////
    // Add Val to this.

    template <class T>
    void CImg<T>::Add(const T& Val)
    {
        T *pROI = m_pROI;
        int x, y, EndX = m_ROIEndX, EndY = m_ROIEndY, sk = m_ROISkipCols;
        for (y = m_ROIStartY; y < EndY; y++, pROI += sk)
            for (x = m_ROIStartX; x < EndX; x++, pROI++)
                *pROI += Val;
    }

    ///////////////////////////////////////////////////////////////////////////
    // Subtract Img from this.

    template <class T>
    void CImg<T>::Sub(CImg<T>& Img)
    {
        T *pROI = m_pROI, *pOtherROI= Img.pROI();
        int x, y, EndX = m_ROIEndX,
            sk1 = m_ROISkipCols, sk2 = Img.ROISkipCols();
        for (y = m_ROIStartY; y < m_ROIEndY; y++, pROI += sk1, pOtherROI+= sk2)
            for (x = m_ROIStartX; x < EndX; x++, pROI++, pOtherROI++)
                *pROI -= *pOtherROI;
    }

    ///////////////////////////////////////////////////////////////////////////
    // Subtract Val from this.

    template <class T>
    void CImg<T>::Sub(const T& Val)
    {
        T *pROI = m_pROI;
        int x, y, EndX = m_ROIEndX, EndY = m_ROIEndY, sk = m_ROISkipCols;
        for (y = m_ROIStartY; y < EndY; y++, pROI += sk)
            for (x = m_ROIStartX; x < EndX; x++, pROI++)
                *pROI -= Val;
    }

    ///////////////////////////////////////////////////////////////////////////
    // Negate in place.

    // BYTE version (untested)
    template <> inline void CImg<BYTE>::Neg()
    {
        BYTE* pROI = m_pROI;
        for (int y = m_ROIStartY; y < m_ROIEndY;
             y++, pROI += m_ROISkipCols)
            for (int x = m_ROIStartX; x < m_ROIEndX; x++, pROI++)
                *pROI = (BYTE)255-(*pROI);
    }

    // general version
    template <class T>
    void CImg<T>::Neg()
    {
        T* pROI = m_pROI;
        for (int y = m_ROIStartY; y < m_ROIEndY;
             y++, pROI += m_ROISkipCols)
            for (int x = m_ROIStartX; x < m_ROIEndX; x++, pROI++)
                *pROI = -(*pROI);
    }

    ///////////////////////////////////////////////////////////////////////////
    // Compute log(a+z), where 'a' is Value of each pixel in ROI, and 'z'
    // is a user supplied argument.

    // float version
    template <> inline void CImg<float>::Log(const float& z)
    {
        float* pROI = m_pROI;
        for (int y = m_ROIStartY; y < m_ROIEndY;
             y++, pROI += m_ROISkipCols)
            for (int x = m_ROIStartX; x < m_ROIEndX; x++, pROI++) {
                float Val = *pROI+z;
                if (Val <= 0.0f) *pROI = -99999.9f;
                else *pROI = (float)log(Val);
            }
    }

    ///////////////////////////////////////////////////////////////////////////
    // Or 'this' with 'Img' (so ROIs must be the same size) and return the
    // result in this.  If the Img is a float Img, then a Value exactly
    // equal to 0.0 is considered false while anything else is true.  If,
    // at any pixel, one of the images ('this' or 'Img') is nonzero, then
    // asign this pixel in 'this' the Value Val.

    template <class T>
    void CImg<T>::HardOR(CImg<T>& Img, const T& Val)
    {
        T* pROI = m_pROI, *pOtherROI= Img.pROI(), ZeroVal = (T)0;
        int x, y, EndX = m_ROIEndX, sk1 = m_ROISkipCols,
            sk2 = Img.ROISkipCols();
        for (y = m_ROIStartY; y < m_ROIEndY; y++, pROI += sk1, pOtherROI+= sk2)
            for (x = m_ROIStartX; x < EndX; x++, pROI++, pOtherROI++)
                if ((*pROI != ZeroVal) || (*pOtherROI!= ZeroVal)) *pROI = Val;
        // no need for 'else *pROI = (T)0;' bec. we're putting things
        // back in 'this' which is already 0 in these places
    }

    ///////////////////////////////////////////////////////////////////////////
    // Same as HardOR(), but rather than setting a true pixel to some
    // Value, keep the Value that made it true.

    template <class T>
    void CImg<T>::SoftOR(CImg<T>& Img)
    {
        T* pROI = m_pROI, *pOtherROI= Img.pROI(), ZeroVal = (T)0;
        int x, y, EndX = m_ROIEndX, sk1 = m_ROISkipCols,
            sk2 = Img.ROISkipCols();
        for (y = m_ROIStartY; y < m_ROIEndY; y++, pROI += sk1, pOtherROI+= sk2)
            for (x = m_ROIStartX; x < EndX; x++, pROI++, pOtherROI++)
                if ((*pROI == ZeroVal) && (*pOtherROI != ZeroVal))
                    *pROI = *pOtherROI;
        // no need for 'else *pROI = (T)0;' bec. we're putting things
        // back in 'this' which is already 0 in these places
    }

    ///////////////////////////////////////////////////////////////////////////
    // Same as 'HardOR()' but it's an AND

    template <class T>
    void CImg<T>::HardAND(CImg<T>& Img, const T& Val)
    {
        T* pROI = m_pROI, *pOtherROI= Img.pROI(), ZeroVal = (T)0;
        int x, y, EndX = m_ROIEndX, sk1 = m_ROISkipCols,
            sk2 = Img.ROISkipCols();
        for (y = m_ROIStartY; y < m_ROIEndY; y++, pROI += sk1, pOtherROI+= sk2)
            for (x = m_ROIStartX; x < EndX; x++, pROI++, pOtherROI++)
                if ((*pROI != ZeroVal) && (*pOtherROI!= ZeroVal)) *pROI = Val;
                else *pROI = (T)0;
    }

    ///////////////////////////////////////////////////////////////////////////
    // Same as 'HardOR()' but it's an AND

    template <class T>
    void CImg<T>::SoftAND(CImg<T>& Img)
    {
        T* pROI = m_pROI, *pOtherROI= Img.pROI(), ZeroVal = (T)0;
        int x, y, EndX = m_ROIEndX, sk1 = m_ROISkipCols,
            sk2 = Img.ROISkipCols();
        for (y = m_ROIStartY; y < m_ROIEndY; y++, pROI += sk1, pOtherROI+= sk2)
            for (x = m_ROIStartX; x < EndX; x++, pROI++, pOtherROI++)
                if (*pOtherROI== ZeroVal) *pROI = (T)0;
    }

    ///////////////////////////////////////////////////////////////////////////
    // Pixels in 'Img' whose Value is Below 'Thresh' are set to 'Below',
    // other pixels are Left untouched.  ROI is considered. In place version.

    /*template <class T>
    void CImg<T>::SoftThresh(const T Thresh, const T Below)
    {
        T* pROI = m_pROI;
        int x, y, EndX = m_ROIEndX, EndY = m_ROIEndY, sk = m_ROISkipCols;
        for (y = m_ROIStartY; y < EndY; y++, pROI += sk)
            for (x = m_ROIStartX; x < EndX; x++, pROI++)
                if (*pROI <= Thresh) *pROI = Below;
    }*/

    ///////////////////////////////////////////////////////////////////////////
    // Pixels in 'Img' whose Value is Below 'Thresh' are set to 'Below',
    // other pixels are set to 'Above'.  ROI is considered.  In place version.

    /*template <class T>
    void CImg<T>::HardThresh(const T Thresh, const T Below, const T Above)
    {
        T* pROI = m_pROI;
        int x, y, EndX = m_ROIEndX, EndY = m_ROIEndY, sk = m_ROISkipCols;
        for (y = m_ROIStartY; y < EndY; y++, pROI += sk)
            for (x = m_ROIStartX; x < m_ROIEndX; x++, pROI++)
                if (*pROI > Thresh) *pROI = Above;
                else *pROI = Below;
    }*/

    ///////////////////////////////////////////////////////////////////////////
    // Just swap the pointers with the input Img's pointers, keep all
    // else the same.

    template <class T>
    void CImg<T>::SwapPointers(CImg<T> &InImg)
    {
        T* pTmp;
        pTmp = m_pBuffer;
        m_pBuffer = InImg.m_pBuffer;
        InImg.m_pBuffer = pTmp;
        pTmp = m_pROI;
        m_pROI = InImg.pROI();
        InImg.m_pROI = pTmp;
    }

    ///////////////////////////////////////////////////////////////////////////

    template <class T>
    void CImg<T>::SwapAll(CImg<T>& InImg)
    {
        SwapPointers(InImg);
        m_Width = InImg.m_Width;
        m_Height = InImg.m_Height;
        m_nPixels = InImg.m_nPixels;
        m_bDelete = InImg.m_bDelete;
        m_ROIStartX = InImg.m_ROIStartX;
        m_ROIStartY = InImg.m_ROIStartY;
        m_ROIEndX = InImg.m_ROIEndX;
        m_ROIEndY = InImg.m_ROIEndY;
        m_ROISkipCols = InImg.m_ROISkipCols;
        m_ROIWidth = InImg.m_ROIWidth;
        m_ROIHeight = InImg.m_ROIHeight;
        m_ROIOffset = InImg.m_ROIOffset;
        m_ROISize = InImg.m_ROISize;
    }

    ///////////////////////////////////////////////////////////////////////////
    // Normalize ROI of Img by L1-norm of ROI
    // float version only

    template <> inline void CImg<float>::NormalizeL1()
    {
        Div(SumAbs());
    }

    ///////////////////////////////////////////////////////////////////////////
    // Return the  L2-norm of pixels in ROI.

    // BYTE version
    template <> inline float CImg<BYTE>::L2Norm()
    {
        BYTE* pROI = m_pROI;
        kjb_int32 sumofsquares = 0;
        for (int y = m_ROIStartY; y < m_ROIEndY;
             y++, pROI += m_ROISkipCols)
            for (int x = m_ROIStartX; x < m_ROIEndX; x++, pROI++) {
                const kjb_int32 Val = (kjb_int32)*pROI;
                sumofsquares += Val*Val;
            }
        return (float)sqrt((float)sumofsquares);
    }

    // float version
    template <> inline float CImg<float>::L2Norm()
    {
        float* pROI = m_pROI;
        float sumofsquares = 0;
        for (int y = m_ROIStartY; y < m_ROIEndY;
             y++, pROI += m_ROISkipCols)
            for (int x = m_ROIStartX; x < m_ROIEndX; x++, pROI++) {
                const float Val = *pROI;
                sumofsquares += Val*Val;
            }
        return (float)sqrt(sumofsquares);
    }

    ///////////////////////////////////////////////////////////////////////////

    template <class T>
    kjb_int32 CImg<T>::nNonZeros()
    {
        const int StartX = m_ROIStartX, EndX = m_ROIEndX,
            StartY = m_ROIStartY, EndY = m_ROIEndY, sk = m_ROISkipCols;
        T* pBuffer = m_pBuffer;
        kjb_int32 nPoints = 0;
        for (int y = StartY; y < EndY; y++, pBuffer += sk)
            for (int x = StartX; x < EndX; x++, pBuffer++)
                if (*pBuffer  != (T)0) nPoints++;
        return nPoints;
    }

    ///////////////////////////////////////////////////////////////////////////

    template <> inline void CImg<float>::Sqrt()
    {
        const int StartX = m_ROIStartX, EndX = m_ROIEndX,
            StartY = m_ROIStartY, EndY = m_ROIEndY, sk = m_ROISkipCols;
        float* pBuffer = m_pBuffer;
        for (int y = StartY; y < EndY; y++, pBuffer += sk)
            for (int x = StartX; x < EndX; x++, pBuffer++)
                if (*pBuffer) *pBuffer = (float)sqrt((float)*pBuffer);
    }

    ///////////////////////////////////////////////////////////////////////////

    // float version only
    template <> inline void CImg<float>::Val2Probability(const float& Sigma)
    {
        const float MinusOneOverSigma = -1.0f/Sigma;
        const int StartX = m_ROIStartX, EndX = m_ROIEndX,
            StartY = m_ROIStartY, EndY = m_ROIEndY, sk = m_ROISkipCols;
        float* pBuffer = m_pBuffer;
        for (int y = StartY; y < EndY; y++, pBuffer += sk)
            for (int x = StartX; x < EndX; x++, pBuffer++)
                if (*pBuffer)
                    *pBuffer = 1.0f-(float)exp(*pBuffer*MinusOneOverSigma);
    }

    ///////////////////////////////////////////////////////////////////////////

    template <class T>
    void CImg<T>::CopyPixelsFromImgROI(CImg<T>& Img)
    {
        const int InStartX = Img.ROIStartX(), InStartY = Img.ROIStartY();
        const int InEndX = Img.ROIEndX(), InEndY = Img.ROIEndY();
        const int InSkip = Img.ROISkipCols();

        T* pOutROI = m_pROI;
        T* pInROI = Img.pROI();
        for (int y = InStartY; y < InEndY; y++, pInROI += InSkip,
                 pOutROI += m_ROISkipCols) {
            for (int x = InStartX; x < InEndX; x++, pInROI++, pOutROI++) {
                *pOutROI = *pInROI;
            }
        }
    }

    ///////////////////////////////////////////////////////////////////////////

    template <class T>
    CImgVec<T>::CImgVec()
    {
        Empty();
    }

    ///////////////////////////////////////////////////////////////////////////

    template <class T>
    CImgVec<T>::CImgVec(T** ppBuffers, const int nFrames, const int Width,
                        const int Height, bool bDelete)
    {
        Attach(ppBuffers, nFrames, Width, Height, bDelete);
    }

    ///////////////////////////////////////////////////////////////////////////

    template <class T>
    CImgVec<T>::CImgVec(CImg<T>& InImg, const int& nCols, const int& nRows,
                        const int& Padding)
    {
        m_nFrames = nRows*nCols;

        m_ROIWidth = m_ROIEndX = m_Width = 0;
        m_ROIHeight = m_ROIEndY = m_Height = 0;
        m_ROISize = m_FrameSize = 0;
        m_ROIOffset = m_ROISkipCols = m_ROIStartX = m_ROIStartY = 0;
        m_TotalSize = 0;

        const int InOrigStartX = InImg.ROIStartX();
        const int InOrigStartY =  InImg.ROIStartY();
        const int InOrigEndX = InImg.ROIEndX();
        const int InOrigEndY =  InImg.ROIEndY();

        const int InWidth = InImg.Width(), InHeight = InImg.Height();
        const int AvSubWidth = InWidth/nCols, AvSubHeight = InHeight/nRows;

        const int HalfAvSubWidth = AvSubWidth/2;
        const int HalfAvSubHeight = AvSubHeight/2;

        for (int row = 0; row < nRows; row++) {
            int InStartX, InStartY, InEndX, InEndY;

            InStartY = row*AvSubHeight;
            InEndY = (row+1)*AvSubHeight;
            if (InEndY > InHeight-HalfAvSubHeight) InEndY = InHeight;

            for (int col = 0; col < nCols; col++) {
                InStartX = col*AvSubWidth;
                InEndX = (col+1)*AvSubWidth;
                if (InEndX > InWidth-HalfAvSubWidth) InEndX = InWidth;

                if (InEndX > InWidth-HalfAvSubWidth) InEndX = InWidth;
                InImg.ChangeROI(InStartX, InEndX, InStartY, InEndY);

                // cerr << "row = " << row << "; col = " << col << endl;
                // InImg.Report();
                CImg<float>* pSubImg = new CImg<float>(InImg, Padding);
                // pSubImg->Report();
                // cerr << "-----------------------------" << endl;
                m_vecImgs.push_back(pSubImg);
            } // for (col ..
        } // for (row ..
        // restore InImg ROI
        InImg.ChangeROI(InOrigStartX, InOrigEndX, InOrigStartY, InOrigEndY);
    }

    ///////////////////////////////////////////////////////////////////////////

    template <class T>
    CImgVec<T>::~CImgVec()
    {
        FreeMemory();
    }

    ///////////////////////////////////////////////////////////////////////////

    template <class T>
    void CImgVec<T>::Empty()
    {
        m_ROIWidth = m_ROIEndX = m_Width = 0;
        m_ROIHeight = m_ROIEndY = m_Height = 0;
        m_ROISize = m_FrameSize = 0;
        m_ROIOffset = m_ROISkipCols = m_ROIStartX = m_ROIStartY = 0;
        m_TotalSize = 0;
        m_nFrames = 0;
        FreeMemory();
    }

    ///////////////////////////////////////////////////////////////////////////

    template <class T>
    void CImgVec<T>:: Attach(T** ppBuffers, const int nFrames, const int Width,
                             const int Height, bool bDelete)
    {
        m_ROIWidth = m_ROIEndX = m_Width = Width;
        m_ROIHeight = m_ROIEndY = m_Height = Height;
        m_ROISize = m_FrameSize = Width*Height;
        m_ROIOffset = m_ROISkipCols = m_ROIStartX = m_ROIStartY = 0;
        m_TotalSize = m_FrameSize*nFrames;
        m_nFrames = nFrames;
        for (int i = 0; i < nFrames; i++)
            m_vecImgs.push_back(new CImg<T>(ppBuffers[i], Width, Height,
                                            bDelete));
    }

    ///////////////////////////////////////////////////////////////////////////

    template <class T>
    void CImgVec<T>::FreeMemory()
    {
        if (!m_vecImgs.empty()) {
            for (int i = 0; i < m_nFrames; i++)
                zap(m_vecImgs[i]);
            m_vecImgs.clear();
        }
    }

    ///////////////////////////////////////////////////////////////////////////

    template <class T>
    bool CImgVec<T>::Allocate (const int nFrames,const int Width,
                               const int Height, bool bZero)
    {
        m_ROIWidth = m_ROIEndX = m_Width = m_Width = Width;
        m_ROIHeight = m_ROIEndY = m_Height = Height;
        m_ROISize = m_FrameSize = Width*Height;
        m_ROIOffset = m_ROISkipCols = m_ROIStartX = m_ROIStartY = 0;
        m_TotalSize = m_FrameSize*nFrames;
        m_nFrames = nFrames;
        //    m_vecImgs.reserve(nFrames);
        for (int i = 0; i < nFrames; i++) {
            CImg<T>* pImg = new CImg<T>(Width, Height, true, bZero);
            if (pImg == NULL) return false;
            m_vecImgs.push_back(pImg);
        }
        return true;
    }

    ///////////////////////////////////////////////////////////////////////////

    template <class T>
    void CImgVec<T>::CopyROI(CImg<T>& Img)
    {
        m_ROIStartX = Img.ROIStartX();
        m_ROIStartY = Img.ROIStartY();
        m_ROIEndX = Img.ROIEndX();
        m_ROIEndY = Img.ROIEndY();
        m_ROIWidth = Img.ROIWidth();
        m_ROIHeight = Img.ROIHeight();
        m_ROISkipCols = Img.ROISkipCols();
        m_ROIOffset = Img.ROIOffset();
        m_ROISize = Img.ROISize();
    }

    ///////////////////////////////////////////////////////////////////////////

    template <class T>
    inline void CImgVec<T>::ChangeRange(const T& NewMin, const T& NewMax)
    {
        for (int i = 0; i < m_nFrames; i++)
            m_vecImgs[i]->ChangeRange(NewMin, NewMax);
    }

    ///////////////////////////////////////////////////////////////////////////

    template <> inline void CImgVec<float>::FixThetaRanges(const bool& bHalfPhase)
    {
        for (int i = 0; i < m_nFrames; i++)
            m_vecImgs[i]->FixThetaRanges(bHalfPhase);
    }

    ///////////////////////////////////////////////////////////////////////////

    template <class T>
    bool CImgVec<T>::IsEmpty()
    {
        return (m_nFrames == 0);
    }

    ///////////////////////////////////////////////////////////////////////////
    /// TYPES
    ///////////////////////////////////////////////////////////////////////////

    typedef CImg<BYTE>* ByteCImgPtr;
    typedef CImg<kjb_int32>* LongCImgPtr;
    typedef CImg<float>* FloatCImgPtr;
    typedef CImg<int>* IntCImgPtr;

    typedef CImgVec<BYTE>* ByteCImgVecPtr;
    typedef CImgVec<kjb_int32>* LongCImgVecPtr;
    typedef CImgVec<float>* FloatCImgVecPtr;
    typedef CImg<int>* IntCImgPtr;

    typedef vector<CImg<BYTE>*>::iterator ByteImgVecIter;
    typedef vector<CImg<kjb_int32>*>::iterator LongImgVecIter;
    typedef vector<CImg<float>*>::iterator FloatImgVecIter;
    typedef vector<CImg<int>*>::iterator IntImgVecIter;

} // namespace DTLib {

#endif /* #ifndef _IMG_H */
