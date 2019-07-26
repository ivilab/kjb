/////////////////////////////////////////////////////////////////////////////
// jpeg.cpp - wrapper class for reading a jpeg image
// Author: Doron Tal
// Date created: March, 2000

#include <stdio.h>
#include "wrap_dtlib_cpp/jpeg.h"
#include "jpeglib.h"
#include "jerror.h"
#include "wrap_dtlib_cpp/utils.h"

//////////////////////////////////////////////
//Added by Prasad
//This is to allow for reading image formats other than .jpeg
//using routines from the KJB library

#define ENABLE_READING_OTHER_FORMATS
/////////////////////////////////////////////

#ifdef ENABLE_READING_OTHER_FORMATS
#include "i/i_incl.h"
using namespace kjb_c;
#endif 

////////////////////////////////////////////
//Added by Prasad
//To allow for float precision for image pixels
//NOTE***********Make sure the following is undefined when using any 
//of the following functions:
//BGR2Gray, BGR2L, BGR2LAB
//because they assume char precision for image pixels***************

#define ENABLE_FLOAT_PRECISION
////////////////////////////////////////////

using namespace DTLib;

/////////////////////////////////////////////////////////////////////////////

CJPEG::CJPEG()
{
    m_bDelete = true;
    m_pBuffer = NULL;
}

/////////////////////////////////////////////////////////////////////////////

CJPEG::~CJPEG()
{
    if (m_bDelete) zap(m_pBuffer);
}

/////////////////////////////////////////////////////////////////////////////

bool CJPEG::Convert_from_kjb(const kjb_c::KJB_image * ip)
{
	delete[] m_pBuffer;
	m_pBuffer = new float[ip->num_rows*ip->num_cols*3];

    int k = 0;
    for (int i = 0; i < ip->num_rows; i++){
         for (int j = 0; j < ip->num_cols; j++){
              m_pBuffer[k] = (float) (ip->pixels[i][j].b); k++;
              m_pBuffer[k] = (float) (ip->pixels[i][j].g); k++;
              m_pBuffer[k] = (float) (ip->pixels[i][j].r); k++;
         }
    }

    m_Width = ip->num_cols;
    m_Height = ip->num_rows;
    m_BytesPerPixel = 3;
    return true;
}

bool CJPEG::ReadImg(const char* filename)
{
//////////////////////////////////////////////
//Added by Prasad
//This is to allow for reading image formats other than .jpeg
//using routines from the KJB library

//NOTE*****if ENABLE_FLOAT_PRECISION is defined, then m_BytesPerPixel 
//means each pixel is represented by (m_BytesPerPixel*size(float)) bytes
//per pixel*************************************************************

#ifdef ENABLE_READING_OTHER_FORMATS

    KJB_image* ip = NULL;

    EPETE(kjb_read_image_2(&ip, filename));

    m_BytesPerPixel = 3;
    m_Width = ip->num_cols;
    m_Height = ip->num_rows;

    zap(m_pBuffer);
//////////////////////////////
#ifdef ENABLE_FLOAT_PRECISION 
    m_pBuffer = new float[m_Width*m_Height*m_BytesPerPixel];
#else   
    m_pBuffer = new unsigned char[m_Width*m_Height*m_BytesPerPixel];
#endif
//////////////////////////////
    int k = 0;
    for (int i = 0; i < m_Height; i++){
         for (int j = 0; j < m_Width; j++){

//////////////////////////////
#ifdef ENABLE_FLOAT_PRECISION 
              m_pBuffer[k] = (float) (ip->pixels[i][j].b); k++;
              m_pBuffer[k] = (float) (ip->pixels[i][j].g); k++;
              m_pBuffer[k] = (float) (ip->pixels[i][j].r); k++;
#else   
              m_pBuffer[k] = (unsigned char) (ip->pixels[i][j].b); k++;
              m_pBuffer[k] = (unsigned char) (ip->pixels[i][j].g); k++;
              m_pBuffer[k] = (unsigned char) (ip->pixels[i][j].r); k++;
#endif
//////////////////////////////

         }// for (int j = 0....
    }// for (int i = 0....

    kjb_free_image(ip);
/////////////////////////////////////////////

#else

    FILE* file = fopen(filename, "rb");
    if (!file) return(false);
    jpeg_decompress_struct cinfo;
    jpeg_error_mgr jerr;
    cinfo.err = jpeg_std_error(&jerr);
    jpeg_create_decompress(&cinfo);
    jpeg_stdio_src(&cinfo, file);
    jpeg_read_header(&cinfo, (boolean)true);
    if(cinfo.out_color_space == JCS_GRAYSCALE) {
        cinfo.output_components = 1;
        m_BytesPerPixel = 1;
    }
    else { // convert to 24 bit
        cinfo.out_color_space = JCS_RGB;
        cinfo.output_components = 3;
        m_BytesPerPixel = 3;
    }
    jpeg_calc_output_dimensions(&cinfo);
    jpeg_start_decompress(&cinfo);
    m_Width = cinfo.output_width;
    m_Height = cinfo.output_height;
    zap(m_pBuffer);
    m_pBuffer = new unsigned char[m_Width*m_Height*m_BytesPerPixel];
    const int LineSize = m_Width*m_BytesPerPixel;
    unsigned char *pData = m_pBuffer;
    while(cinfo.output_scanline < cinfo.output_height) {
        jpeg_read_scanlines(&cinfo, &pData, 1);
        pData += LineSize;
    }
    jpeg_finish_decompress(&cinfo);
    jpeg_destroy_decompress(&cinfo);
    fclose(file);

#endif

    return true;
}
