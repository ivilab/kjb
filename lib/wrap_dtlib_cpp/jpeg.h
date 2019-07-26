/////////////////////////////////////////////////////////////////////////////
// jpeg.h - wrapper class for reading a jpeg image
// Author: Doron Tal
// Date created: March, 2000

#ifndef _JPEG_H
#define _JPEG_H

////////////////////////////////////////////
//Added by Prasad
//To allow for float precision for image pixels
//NOTE***********Make sure the following is undefined when using any
//of the following functions:
//BGR2Gray, BGR2L, BGR2L
//because they assume char precision for image pixels***************

#define ENABLE_FLOAT_PRECISION
////////////////////////////////////////////

#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <i/i_float.h>


namespace DTLib {

    class CJPEG {

    public:
        CJPEG();
        ~CJPEG();

        // reads a Jpeg Image from file designated by 'filename'
        bool ReadImg(const char* filename);

        // returns Width/Height of image read by ReadImg()
        int Width() { return m_Width; }
        int Height() { return m_Height; }

        // returns number of bytes per pixel read by prev. call to ReadImg()
        int nBytesPerPixel() { return m_BytesPerPixel; }

        bool Convert_from_kjb(const kjb_c::KJB_image * ip);


////////////////////////////////////////////
//Added by Prasad
//To allow for float precision for image pixels

#ifdef ENABLE_FLOAT_PRECISION
        // returns the buffer read by prev. call to ReadImg()
        float* pBuffer() { return m_pBuffer; }
#else
        // returns the buffer read by prev. call to ReadImg()
        unsigned char* pBuffer() { return m_pBuffer; }
#endif

////////////////////////////////////////////
        // if you call this function with 'bDelete' == false,
        // the destructor of this class will not free m_pBuffer, otherwise
        // the default behavior is to free m_pBuffer.
        void Deallocate(bool bDelete) { m_bDelete = bDelete; }

    private:

        int m_Width;
        int m_Height;
        int m_BytesPerPixel;
////////////////////////////////////////////
//Added by Prasad
//To allow for float precision for image pixels

#ifdef ENABLE_FLOAT_PRECISION
        float* m_pBuffer;
#else
        unsigned char* m_pBuffer;
#endif
////////////////////////////////////////////

        bool m_bDelete;
    };

} // namespace DTLib {

#endif /*#ifndef _JPEG_H */
