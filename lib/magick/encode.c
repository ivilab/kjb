
/* $Id: encode.c 4727 2009-11-16 20:53:54Z kobus $ */

#ifndef __C2MAN__     

/*
 * Make sure that these files are not a liability when there is no X11. If this
 * is the case, then comment out the whole thing.
*/
#ifdef KJB_HAVE_X11 

/* -------------------------------------------------------------------------- */

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                   EEEEE  N   N   CCCC   OOO   DDDD   EEEEE                  %
%                   E      NN  N  C      O   O  D   D  E                      %
%                   EEE    N N N  C      O   O  D   D  EEE                    %
%                   E      N  NN  C      O   O  D   D  E                      %
%                   EEEEE  N   N   CCCC   OOO   DDDD   EEEEE                  %
%                                                                             %
%                                                                             %
%                    Utility Routines to Write Image Formats                  %
%                                                                             %
%                                                                             %
%                                                                             %
%                             Software Design                                 %
%                               John Cristy                                   %
%                              January 1992                                   %
%                                                                             %
%                                                                             %
%  Copyright 1996 E. I. du Pont de Nemours and Company                        %
%                                                                             %
%  Permission to use, copy, modify, distribute, and sell this software and    %
%  its documentation for any purpose is hereby granted without fee,           %
%  provided that the above Copyright notice appear in all copies and that     %
%  both that Copyright notice and this permission notice appear in            %
%  supporting documentation, and that the name of E. I. du Pont de Nemours    %
%  and Company not be used in advertising or publicity pertaining to          %
%  distribution of the software without specific, written prior               %
%  permission.  E. I. du Pont de Nemours and Company makes no representations %
%  about the suitability of this software for any purpose.  It is provided    %
%  "as is" without express or implied warranty.                               %
%                                                                             %
%  E. I. du Pont de Nemours and Company disclaims all warranties with regard  %
%  to this software, including all implied warranties of merchantability      %
%  and fitness, in no event shall E. I. du Pont de Nemours and Company be     %
%  liable for any special, indirect or consequential damages or any           %
%  damages whatsoever resulting from loss of use, data or profits, whether    %
%  in an action of contract, negligence or other tortuous action, arising     %
%  out of or in connection with the use or performance of this software.      %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Functions in this library convert to and from `alien' image formats to the
%  MIFF image format.
%
%
*/

/*
  Include declarations.
*/
#include "magick/magick.h"
#include "magick/XWDFile.h"

#ifdef __cplusplus
extern "C" {
#endif

/* -------------------------------------------------------------------------- */


/*
  Define declarations.
*/
#define SaveImageText  "  Saving image...  "
#define PrematureExit(message,image) \
{ \
  Warning(message,image->filename); \
  return(False); \
}

/*
  Function prototypes.
*/
static int
  SGIEncode _Declare((unsigned char *,int,unsigned char *));

static unsigned int

/* Kobus : Forward declare here, as WriteHIPSImage definition is at the end
   of this file. It is at the end because the HIPS include files typedef "Byte"
   which is used as a variable in some of the code.
*/
  WriteHIPSImage _Declare((ImageInfo *,Image *)),

/* End Kobus */

  WriteAVSImage _Declare((ImageInfo *,Image *)),
  WriteBMPImage _Declare((ImageInfo *,Image *)),
  WriteCMYKImage _Declare((ImageInfo *,Image *)),
  WriteFAXImage _Declare((ImageInfo *,Image *)),
  WriteFITSImage _Declare((ImageInfo *,Image *)),
  WriteGIFImage _Declare((ImageInfo *,Image *)),
  WriteGRAYImage _Declare((ImageInfo *,Image *)),
  WriteHDFImage _Declare((ImageInfo *,Image *)),
  WriteHISTOGRAMImage _Declare((ImageInfo *,Image *)),
  WriteHTMLImage _Declare((ImageInfo *,Image *)),
  WriteJBIGImage _Declare((ImageInfo *,Image *)),
  WriteJPEGImage _Declare((ImageInfo *,Image *)),
  WriteLOGOImage _Declare((ImageInfo *,Image *)),
  WriteMAPImage _Declare((ImageInfo *,Image *)),
  WriteMATTEImage _Declare((ImageInfo *,Image *)),
  WriteMIFFImage _Declare((ImageInfo *,Image *)),
  WriteMPEGImage _Declare((ImageInfo *,Image *)),
  WriteMTVImage _Declare((ImageInfo *,Image *)),
  WritePCDImage _Declare((ImageInfo *,Image *)),
  WritePCXImage _Declare((ImageInfo *,Image *)),
  WritePDFImage _Declare((ImageInfo *,Image *)),
  WritePICTImage _Declare((ImageInfo *,Image *)),
  WritePNGImage _Declare((ImageInfo *,Image *)),
  WritePNMImage _Declare((ImageInfo *,Image *)),
  WritePSImage _Declare((ImageInfo *,Image *)),
  WritePS2Image _Declare((ImageInfo *,Image *)),
  WriteRADIANCEImage _Declare((ImageInfo *,Image *)),
  WriteRGBImage _Declare((ImageInfo *,Image *)),
  WriteRLEImage _Declare((ImageInfo *,Image *)),
  WriteSGIImage _Declare((ImageInfo *,Image *)),
  WriteSUNImage _Declare((ImageInfo *,Image *)),
  WriteTARGAImage _Declare((ImageInfo *,Image *)),
  WriteTEXTImage _Declare((ImageInfo *,Image *)),
  WriteTIFFImage _Declare((ImageInfo *,Image *)),
  WriteTILEImage _Declare((ImageInfo *,Image *)),
  WriteUYVYImage _Declare((ImageInfo *,Image *)),
  WriteVICARImage _Declare((ImageInfo *,Image *)),
  WriteVIFFImage _Declare((ImageInfo *,Image *)),
  WriteXImage _Declare((ImageInfo *,Image *)),
  WriteXBMImage _Declare((ImageInfo *,Image *)),
  WriteXCImage _Declare((ImageInfo *,Image *)),
  WriteXPMImage _Declare((ImageInfo *,Image *)),
  WriteXWDImage _Declare((ImageInfo *,Image *)),
  WriteYUVImage _Declare((ImageInfo *,Image *)),
  WriteYUV3Image _Declare((ImageInfo *,Image *));

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   W r i t e A V S I m a g e                                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Function WriteAVSImage writes an image to a file in AVS X image format.
%
%  The format of the WriteAVSImage routine is:
%
%      status=WriteAVSImage(image_info,image)
%
%  A description of each parameter follows.
%
%    o status: Function WriteAVSImage return True if the image is written.
%      False is returned is there is a memory shortage or if the image file
%      fails to write.
%
%    o image_info: Specifies a pointer to an ImageInfo structure.
%
%    o image:  A pointer to a Image structure.
%
%
*/
static unsigned int WriteAVSImage(ImageInfo *image_info, Image *image)
{
  typedef struct _AVSHeader
  {
    int
      width,
      height;
  } AVSHeader;

  AVSHeader
    avs_header;

  register int
    i,
    j;

  register RunlengthPacket
    *p;

  /*
    Open output image file.
  */
  OpenImage(image,WriteBinaryType);
  if (image->file == (FILE *) NULL)
    PrematureExit("Unable to open file",image);
  /*
    Initialize raster file header.
  */
  avs_header.width=image->columns;
  avs_header.height=image->rows;
  (void) fwrite((char *) &avs_header,sizeof(AVSHeader),1,image->file);
  p=image->pixels;
  for (i=0; i < image->packets; i++)
  {
    for (j=0; j <= (int) p->length; j++)
    {
      (void) fputc(image->matte ? p->index : Opaque,image->file);
      (void) fputc(DownScale(p->red),image->file);
      (void) fputc(DownScale(p->green),image->file);
      (void) fputc(DownScale(p->blue),image->file);
    }
    p++;
    if (QuantumTick(i,image))
      ProgressMonitor(SaveImageText,i,image->packets);
  }
  CloseImage(image);
  return(True);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   W r i t e B M P I m a g e                                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Function WriteBMPImage writes an image in Microsoft Windows bitmap encoded
%  image format.
%
%  The format of the WriteBMPImage routine is:
%
%      status=WriteBMPImage(image_info,image)
%
%  A description of each parameter follows.
%
%    o status: Function WriteBMPImage return True if the image is written.
%      False is returned is there is a memory shortage or if the image file
%      fails to write.
%
%    o image_info: Specifies a pointer to an ImageInfo structure.
%
%    o image:  A pointer to a Image structure.
%
%
*/
static unsigned int WriteBMPImage(ImageInfo *image_info, Image *image)
{
  typedef struct _BMPHeader
  {
    unsigned long
      file_size;

    unsigned short
      reserved[2];

    unsigned long
      offset_bits,
      size,
      width,
      height;

    unsigned short
      planes,
      bit_count;

    unsigned long
      compression,
      image_size,
      x_pixels,
      y_pixels,
      number_colors,
      colors_important;
  } BMPHeader;

  BMPHeader
    bmp_header;

  register int
    i,
    j,
    x,
    y;

  register RunlengthPacket
    *p;

  register unsigned char
    *q;

  unsigned char
    *bmp_data,
    *bmp_pixels;

  unsigned int
    bytes_per_line;

  /*
    Open output image file.
  */
  OpenImage(image,WriteBinaryType);
  if (image->file == (FILE *) NULL)
    PrematureExit("Unable to open file",image);
  /*
    Initialize BMP raster file header.
  */
  bmp_header.file_size=14+40;
  bmp_header.offset_bits=14+40;
  if (!IsPseudoClass(image) && !IsGrayImage(image))
    {
      /*
        Full color BMP raster.
      */
      bmp_header.bit_count=24;
      bmp_header.number_colors=0;
    }
  else
    {
      /*
        Colormapped BMP raster.
      */
      bmp_header.bit_count=8;
      if (IsGrayImage(image) && (image->colors == 2))
        bmp_header.bit_count=1;
      bmp_header.file_size+=4*(1 << bmp_header.bit_count);
      bmp_header.offset_bits+=4*(1 << bmp_header.bit_count);
      bmp_header.number_colors=1 << bmp_header.bit_count;
    }
  bmp_header.reserved[0]=0;
  bmp_header.reserved[1]=0;
  bmp_header.size=40;
  bmp_header.width=image->columns;
  bmp_header.height=image->rows;
  bmp_header.planes=1;
  bmp_header.compression=0;
  bytes_per_line=((image->columns*bmp_header.bit_count+31)/32)*4;
  bmp_header.image_size=bytes_per_line*image->rows;
  bmp_header.file_size+=bmp_header.image_size;
  bmp_header.x_pixels=75*39;
  bmp_header.y_pixels=75*39;
  bmp_header.colors_important=bmp_header.number_colors;
  /*
    Convert MIFF to BMP raster pixels.
  */
  bmp_pixels=(unsigned char *)
    malloc(bmp_header.image_size*sizeof(unsigned char));
  if (bmp_pixels == (unsigned char *) NULL)
    PrematureExit("Unable to allocate memory",image);
  x=0;
  y=image->rows-1;
  switch (bmp_header.bit_count)
  {
    case 1:
    {
      register unsigned char
        bit,
        Byte,
        polarity;

      /*
        Convert PseudoClass image to a BMP monochrome image.
      */
      p=image->pixels;
      polarity=0;
      if (image->colors == 2)
        polarity=Intensity(image->colormap[1]) < Intensity(image->colormap[0]);
      bit=0;
      Byte=0;
      q=bmp_pixels+y*bytes_per_line;
      for (i=0; i < image->packets; i++)
      {
        for (j=0; j <= (int) p->length; j++)
        {
          Byte<<=1;
          if (p->index == polarity)
            Byte|=0x01;
          bit++;
          if (bit == 8)
            {
              *q++=Byte;
              bit=0;
              Byte=0;
            }
          x++;
          if (x == image->columns)
            {
              /*
                Advance to the next scanline.
              */
              if (bit != 0)
                *q++=Byte << (8-bit);
              bit=0;
              Byte=0;
              x=0;
              y--;
              q=bmp_pixels+y*bytes_per_line;
           }
        }
        p++;
        if (QuantumTick(i,image))
          ProgressMonitor(SaveImageText,i,image->packets);
      }
      break;
    }
    case 8:
    {
      /*
        Convert PseudoClass packet to BMP pixel.
      */
      if (image->compression == RunlengthEncodedCompression)
        CompressImage(image);
      if (image->compression == RunlengthEncodedCompression)
        bytes_per_line=image->columns;
      p=image->pixels;
      q=bmp_pixels+y*bytes_per_line;
      for (i=0; i < image->packets; i++)
      {
        for (j=0; j <= (int) p->length; j++)
        {
          *q++=p->index;
          x++;
          if (x == image->columns)
            {
              x=0;
              y--;
              q=bmp_pixels+y*bytes_per_line;
            }
        }
        p++;
        if (QuantumTick(i,image))
          ProgressMonitor(SaveImageText,i,image->packets);
      }
      break;
    }
    case 24:
    {
      /*
        Convert DirectClass packet to BMP RGB pixel.
      */
      p=image->pixels;
      q=bmp_pixels+y*bytes_per_line;
      for (i=0; i < image->packets; i++)
      {
        for (j=0; j <= (int) p->length; j++)
        {
          *q++=DownScale(p->blue);
          *q++=DownScale(p->green);
          *q++=DownScale(p->red);
          x++;
          if (x == image->columns)
            {
              x=0;
              y--;
              q=bmp_pixels+y*bytes_per_line;
            }
        }
        p++;
        if (QuantumTick(i,image))
          ProgressMonitor(SaveImageText,i,image->packets);
      }
      break;
    }
  }
  if (bmp_header.bit_count == 8)
    if (image->compression == RunlengthEncodedCompression)
      {
        unsigned int
          packets;

        /*
          Convert run-length encoded raster pixels.
        */
        packets=(bytes_per_line*(bmp_header.height+2)+1) << 1;
        bmp_data=(unsigned char *) malloc(packets*sizeof(unsigned char));
        if (bmp_pixels == (unsigned char *) NULL)
          {
            Warning("Memory allocation error",(char *) NULL);
            free((char *) bmp_pixels);
            return(False);
          }
        bmp_header.image_size=
          BMPEncodeImage(bmp_pixels,bmp_data,image->columns,image->rows);
        free((char *) bmp_pixels);
        bmp_pixels=bmp_data;
        bmp_header.compression=1;
      }
  /*
    Write BMP header.
  */
  (void) fwrite("BM",1,2,image->file);
  LSBFirstWriteLong(bmp_header.file_size,image->file);
  LSBFirstWriteShort(bmp_header.reserved[0],image->file);
  LSBFirstWriteShort(bmp_header.reserved[1],image->file);
  LSBFirstWriteLong(bmp_header.offset_bits,image->file);
  LSBFirstWriteLong(bmp_header.size,image->file);
  LSBFirstWriteLong(bmp_header.width,image->file);
  LSBFirstWriteLong(bmp_header.height,image->file);
  LSBFirstWriteShort(bmp_header.planes,image->file);
  LSBFirstWriteShort(bmp_header.bit_count,image->file);
  LSBFirstWriteLong(bmp_header.compression,image->file);
  LSBFirstWriteLong(bmp_header.image_size,image->file);
  LSBFirstWriteLong(bmp_header.x_pixels,image->file);
  LSBFirstWriteLong(bmp_header.y_pixels,image->file);
  LSBFirstWriteLong(bmp_header.number_colors,image->file);
  LSBFirstWriteLong(bmp_header.colors_important,image->file);
  if (image->class == PseudoClass)
    {
      unsigned char
        *bmp_colormap;

      /*
        Dump colormap to file.
      */
      bmp_colormap=(unsigned char *)
        malloc(4*(1 << bmp_header.bit_count)*sizeof(unsigned char));
      if (bmp_colormap == (unsigned char *) NULL)
        PrematureExit("Unable to allocate memory",image);
      q=bmp_colormap;
      for (i=0; i < image->colors; i++)
      {
        *q++=DownScale(image->colormap[i].blue);
        *q++=DownScale(image->colormap[i].green);
        *q++=DownScale(image->colormap[i].red);
        q++;
      }
      for ( ; i < (int) (1 << bmp_header.bit_count); i++)
      {
        *q++=0;
        *q++=0;
        *q++=0;
        q++;
      }
      (void) fwrite((char *) bmp_colormap,4,1 << bmp_header.bit_count,
        image->file);
      free((char *) bmp_colormap);
    }
  (void) fwrite((char *) bmp_pixels,1,(int) bmp_header.image_size,image->file);
  free((char *) bmp_pixels);
  CloseImage(image);
  return(True);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   W r i t e C M Y K I m a g e                                               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Function WriteCMYKImage writes an image to a file in cyan, magenta,
%  yellow, and black rasterfile format.
%
%  The format of the WriteCMYKImage routine is:
%
%      status=WriteCMYKImage(image_info,image)
%
%  A description of each parameter follows.
%
%    o status: Function WriteCMYKImage return True if the image is written.
%      False is returned is there is a memory shortage or if the image file
%      fails to write.
%
%    o image_info: Specifies a pointer to an ImageInfo structure.
%
%    o image:  A pointer to a Image structure.
%
%
*/
static unsigned int WriteCMYKImage(ImageInfo *image_info, Image *image)
{
  float
    black_generation,
    undercolor;

  int
    black,
    cyan,
    magenta,
    yellow;

  register int
    i,
    j;

  register RunlengthPacket
    *p;

  /*
    Open output image file.
  */
  OpenImage(image,WriteBinaryType);
  if (image->file == (FILE *) NULL)
    PrematureExit("Unable to open file",image);
  /*
    Convert MIFF to CMYK raster pixels.
  */
  undercolor=1.0;
  black_generation=1.0;
  if (image_info->undercolor != (char *) NULL)
    {
      (void) sscanf(image_info->undercolor,"%fx%f",&undercolor,
        &black_generation);
      if (black_generation == 1.0)
        black_generation=undercolor;
    }
  switch (image_info->interlace)
  {
    case NoneInterlace:
    default:
    {
      /*
        No interlacing:  CMYKCMYKCMYKCMYKCMYKCMYK...
      */
      p=image->pixels;
      for (i=0; i < image->packets; i++)
      {
        for (j=0; j <= ((int) p->length); j++)
        {
          cyan=MaxRGB-p->red;
          magenta=MaxRGB-p->green;
          yellow=MaxRGB-p->blue;
          black=cyan;
          if (magenta < black)
            black=magenta;
          if (yellow < black)
            black=yellow;
          WriteQuantumFile((unsigned int) (cyan-undercolor*black));
          WriteQuantumFile((unsigned int) (magenta-undercolor*black));
          WriteQuantumFile((unsigned int) (yellow-undercolor*black));
          WriteQuantumFile((unsigned int) (black_generation*black));
        }
        p++;
        if (QuantumTick(i,image))
          ProgressMonitor(SaveImageText,i,image->packets);
      }
      break;
    }
    case LineInterlace:
    {
      register int
        x,
        y;

      /*
        Line interlacing:  CCC...MMM...YYY...CCC...MMM...YYY...
      */
      if (!UncompressImage(image))
        return(False);
      for (y=0; y < image->rows; y++)
      {
        p=image->pixels+(y*image->columns);
        for (x=0; x < image->columns; x++)
        {
          cyan=MaxRGB-p->red;
          magenta=MaxRGB-p->green;
          yellow=MaxRGB-p->blue;
          black=cyan;
          if (magenta < black)
            black=magenta;
          if (yellow < black)
            black=yellow;
          WriteQuantumFile((unsigned int) (cyan-undercolor*black));
          p++;
        }
        p=image->pixels+(y*image->columns);
        for (x=0; x < image->columns; x++)
        {
          cyan=MaxRGB-p->red;
          magenta=MaxRGB-p->green;
          yellow=MaxRGB-p->blue;
          black=cyan;
          if (magenta < black)
            black=magenta;
          if (yellow < black)
            black=yellow;
          WriteQuantumFile((unsigned int) (magenta-undercolor*black));
          p++;
        }
        p=image->pixels+(y*image->columns);
        for (x=0; x < image->columns; x++)
        {
          cyan=MaxRGB-p->red;
          magenta=MaxRGB-p->green;
          yellow=MaxRGB-p->blue;
          black=cyan;
          if (magenta < black)
            black=magenta;
          if (yellow < black)
            black=yellow;
          WriteQuantumFile((unsigned int) (yellow-undercolor*black));
          p++;
        }
        p=image->pixels+(y*image->columns);
        for (x=0; x < image->columns; x++)
        {
          cyan=MaxRGB-p->red;
          magenta=MaxRGB-p->green;
          yellow=MaxRGB-p->blue;
          black=cyan;
          if (magenta < black)
            black=magenta;
          if (yellow < black)
            black=yellow;
          WriteQuantumFile((unsigned int) (black_generation*black));
          p++;
        }
        ProgressMonitor(SaveImageText,y,image->rows);
      }
      break;
    }
    case PlaneInterlace:
    {
      /*
        Plane interlacing:  CCCCCC...MMMMMM...YYYYYY...KKKKKK...
      */
      p=image->pixels;
      for (i=0; i < image->packets; i++)
      {
        for (j=0; j <= ((int) p->length); j++)
        {
          cyan=MaxRGB-p->red;
          magenta=MaxRGB-p->green;
          yellow=MaxRGB-p->blue;
          black=cyan;
          if (magenta < black)
            black=magenta;
          if (yellow < black)
            black=yellow;
          WriteQuantumFile((unsigned int) (cyan-undercolor*black));
        }
        p++;
      }
      ProgressMonitor(SaveImageText,100,400);
      p=image->pixels;
      for (i=0; i < image->packets; i++)
      {
        for (j=0; j <= ((int) p->length); j++)
        {
          cyan=MaxRGB-p->red;
          magenta=MaxRGB-p->green;
          yellow=MaxRGB-p->blue;
          black=cyan;
          if (magenta < black)
            black=magenta;
          if (yellow < black)
            black=yellow;
          WriteQuantumFile((unsigned int) (magenta-undercolor*black));
        }
        p++;
      }
      ProgressMonitor(SaveImageText,200,400);
      p=image->pixels;
      for (i=0; i < image->packets; i++)
      {
        for (j=0; j <= ((int) p->length); j++)
        {
           cyan=MaxRGB-p->red;
           magenta=MaxRGB-p->green;
           yellow=MaxRGB-p->blue;
           black=cyan;
           if (magenta < black)
             black=magenta;
           if (yellow < black)
             black=yellow;
          WriteQuantumFile((unsigned int) (yellow-undercolor*black));
        }
        p++;
      }
      ProgressMonitor(SaveImageText,300,400);
      p=image->pixels;
      for (i=0; i < image->packets; i++)
      {
        for (j=0; j <= ((int) p->length); j++)
        {
          cyan=MaxRGB-p->red;
          magenta=MaxRGB-p->green;
          yellow=MaxRGB-p->blue;
          black=cyan;
          if (magenta < black)
            black=magenta;
          if (yellow < black)
            black=yellow;
          WriteQuantumFile((unsigned int) (black_generation*black));
        }
        p++;
      }
      ProgressMonitor(SaveImageText,400,400);
      break;
    }
  }
  CloseImage(image);
  return(True);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   W r i t e F A X I m a g e                                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Procedure WriteFAXImage writes an image to a file in 1 dimensional Huffman
%  encoded format.
%
%  The format of the WriteFAXImage routine is:
%
%      status=WriteFAXImage(image_info,image)
%
%  A description of each parameter follows.
%
%    o status: Function WriteFAXImage return True if the image is written.
%      False is returned is there is a memory shortage or if the image file
%      fails to write.
%
%    o image_info: Specifies a pointer to an ImageInfo structure.
%
%    o image:  A pointer to a Image structure.
%
%
*/
static unsigned int WriteFAXImage(ImageInfo *image_info, Image *image)
{
  unsigned int
    status;

  /*
    Open output image file.
  */
  OpenImage(image,WriteBinaryType);
  if (image->file == (FILE *) NULL)
    PrematureExit("Unable to open file",image);
  /*
    Convert MIFF to monochrome.
  */
  if (!IsGrayImage(image) || (image->colors != 2))
    {
      QuantizeImage(image,2,8,image_info->dither,GRAYColorspace);
      SyncImage(image);
    }
  status=HuffmanEncodeImage(image);
  CloseImage(image);
  return(status);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   W r i t e F I T S I m a g e                                               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Function WriteFITSImage writes a Flexible Image Transport System image to a
%  file as gray scale intensities [0..255].
%
%  The format of the WriteFITSImage routine is:
%
%      status=WriteFITSImage(image_info,image)
%
%  A description of each parameter follows.
%
%    o status: Function WriteFITSImage return True if the image is written.
%      False is returned is there is a memory shortage or if the image file
%      fails to write.
%
%    o image_info: Specifies a pointer to an ImageInfo structure.
%
%    o image:  A pointer to a Image structure.
%
%
*/
static unsigned int WriteFITSImage(ImageInfo *image_info, Image *image)
{
  char
    buffer[81],
    *fits_header;

  register int
    i,
    j;

  register RunlengthPacket
    *p;


  /*
    Open output image file.
  */
  OpenImage(image,WriteBinaryType);
  if (image->file == (FILE *) NULL)
    PrematureExit("Unable to open file",image);
  /*
    Allocate image header.
  */
  fits_header=(char *) malloc(2880*sizeof(unsigned char));
  if (fits_header == (char *) NULL)
    PrematureExit("Unable to allocate memory",image);
  /*
    Initialize image header.
  */
  for (i=0; i < 2880; i++)
    fits_header[i]=' ';
  (void) strcpy(buffer,"SIMPLE  =                    T");
  (void) strncpy(fits_header+0,buffer,strlen(buffer));
  (void) strcpy(buffer,"BITPIX  =                    8");
  (void) strncpy(fits_header+80,buffer,strlen(buffer));
  (void) strcpy(buffer,"NAXIS   =                    2");
  (void) strncpy(fits_header+160,buffer,strlen(buffer));
  (void) sprintf(buffer,"NAXIS1  =           %10u",image->columns);
  (void) strncpy(fits_header+240,buffer,strlen(buffer));
  (void) sprintf(buffer,"NAXIS2  =           %10u",image->rows);
  (void) strncpy(fits_header+320,buffer,strlen(buffer));
  (void) strcpy(buffer,"HISTORY Created by ImageMagick.");
  (void) strncpy(fits_header+400,buffer,strlen(buffer));
  (void) strcpy(buffer,"END");
  (void) strncpy(fits_header+480,buffer,strlen(buffer));
  (void) fwrite((char *) fits_header,1,2880,image->file);
  free((char *) fits_header);
  /*
    Convert image to fits scale PseudoColor class.
  */
  p=image->pixels;
  for (i=0; i < image->packets; i++)
  {
    for (j=0; j <= ((int) p->length); j++)
      (void) fputc(DownScale(Intensity(*p)),image->file);
    p++;
    if (QuantumTick(i,image))
      ProgressMonitor(SaveImageText,i,image->packets);
  }
  CloseImage(image);
  return(True);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   W r i t e G I F I m a g e                                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Function WriteGIFImage writes an image to a file in the Compuserve Graphics
%  image format.
%
%  The format of the WriteGIFImage routine is:
%
%      status=WriteGIFImage(image_info,image)
%
%  A description of each parameter follows.
%
%    o status: Function WriteGIFImage return True if the image is written.
%      False is returned is there is a memory shortage or if the image file
%      fails to write.
%
%    o image_info: Specifies a pointer to an ImageInfo structure.
%
%    o image:  A pointer to a Image structure.
%
%
*/
static unsigned int WriteGIFImage(ImageInfo *image_info, Image *image)
{
  register int
    i,
    x;

  register RunlengthPacket
    *p;

  unsigned char
    bits_per_pixel,
    c,
    *matte_image;

  unsigned int
    colors,
    status;

  /*
    Open output image file.
  */
  OpenImage(image,WriteBinaryType);
  if (image->file == (FILE *) NULL)
    PrematureExit("Unable to open file",image);
  colors=256;
  matte_image=(unsigned char *) NULL;
  if (image->matte)
    {
      /*
        Allocate and initialize matte image.
      */
      if (!UncompressImage(image))
        return(False);
      p=image->pixels;
      matte_image=(unsigned char *)
        malloc(image->columns*image->rows*sizeof(unsigned char));
      if (matte_image == (unsigned char *) NULL)
        PrematureExit("Unable to allocate memory",image)
      else
        for (x=0; x < (image->columns*image->rows); x++)
        {
          matte_image[x]=(unsigned char) (p->index & 0xff);
          p++;
        }
      colors--;
    }
  if ((image->class == DirectClass) || (image->colors > colors))
    {
      /*
        Demote DirectClass to PseudoClass.
      */
      QuantizeImage(image,colors,8,image_info->dither,RGBColorspace);
      SyncImage(image);
      if (!UncompressImage(image))
        return(False);
    }
  colors=image->colors;
  if (matte_image != (unsigned char *) NULL)
    {
      CompressColormap(image);
      colors++;
    }
  for (bits_per_pixel=1; bits_per_pixel < 8; bits_per_pixel++)
    if ((1 << bits_per_pixel) >= colors)
      break;
  /*
    Write GIF header.
  */
  if ((matte_image == (unsigned char *) NULL) &&
      (image->comments == (char *) NULL))
    (void) fwrite("GIF87a",1,6,image->file);
  else
    if (strcmp(image_info->magick,"GIF87") == 0)
      (void) fwrite("GIF87a",1,6,image->file);
    else
      (void) fwrite("GIF89a",1,6,image->file);
  LSBFirstWriteShort(image->columns,image->file);
  LSBFirstWriteShort(image->rows,image->file);
  c=0x80;  /* global colormap */
  c|=(8-1) << 4;  /* color resolution */
  c|=(bits_per_pixel-1);   /* size of global colormap */
  (void) fputc((char) c,image->file);
  (void) fputc(0x0,image->file);  /* background color */
  (void) fputc(0x0,image->file);  /* reserved */
  /*
    Write colormap.
  */
  for (i=0; i < image->colors; i++)
  {
    (void) fputc(DownScale(image->colormap[i].red),image->file);
    (void) fputc(DownScale(image->colormap[i].green),image->file);
    (void) fputc(DownScale(image->colormap[i].blue),image->file);
  }
  if (matte_image != (unsigned char *) NULL)
    {
      /*
        Write color of transparent pixel.
      */
      p=image->pixels;
      for (x=0; x < (image->columns*image->rows); x++)
      {
        if (matte_image[x] == Transparent)
          break;
        p++;
      }
      (void) fputc(DownScale(image->colormap[p->index].red),image->file);
      (void) fputc(DownScale(image->colormap[p->index].green),image->file);
      (void) fputc(DownScale(image->colormap[p->index].blue),image->file);
      i++;
    }
  for ( ; i < (int) (1 << bits_per_pixel); i++)
  {
    (void) fputc(0x0,image->file);
    (void) fputc(0x0,image->file);
    (void) fputc(0x0,image->file);
  }
  if (strcmp(image_info->magick,"GIF87") != 0)
    {
      if (image->comments != (char *) NULL)
        {
          register char
            *p;

          register unsigned int
            count;

          /*
            Write comment extension block.
          */
          (void) fputc(0x21,image->file);
          (void) fputc(0xfe,image->file);
          p=image->comments;
          while ((int) strlen(p) > 0)
          {
            count=Min((int) strlen(p),255);
            (void) fputc(count,image->file);
            for (i=0; i < count; i++)
              (void) fputc(*p++,image->file);
          }
          (void) fputc(0x0,image->file);
        }
      if (matte_image != (unsigned char *) NULL)
        {
          /*
            Write out extension for transparent color index.
          */
          (void) fputc(0x21,image->file);
          (void) fputc(0xf9,image->file);
          (void) fputc(0x4,image->file);
          (void) fputc(0x1,image->file);
          (void) fputc(0x0,image->file);
          (void) fputc(0x0,image->file);
          (void) fputc((char) image->colors,image->file);
          (void) fputc(0x0,image->file);
          /*
            Set transparent pixels to the transparent color index.
          */
          p=image->pixels;
          for (x=0; x < (image->columns*image->rows); x++)
          {
            if (matte_image[x] == Transparent)
              p->index=image->colors;
            p++;
          }
          free((char *) matte_image);
        }
    }
  (void) fputc(',',image->file);  /* image separator */
  /*
    Write the image header.
  */
  LSBFirstWriteShort(0,image->file);
  LSBFirstWriteShort(0,image->file);
  LSBFirstWriteShort(image->columns,image->file);
  LSBFirstWriteShort(image->rows,image->file);
  c=0x00;
  if (image_info->interlace != NoneInterlace)
    c|=0x40;  /* pixel data is interlaced */
  (void) fputc((char) c,image->file);
  c=Max(bits_per_pixel,2);
  (void) fputc((char) c,image->file);
  if (image_info->interlace == NoneInterlace)
    status=GIFEncodeImage(image,Max(bits_per_pixel,2)+1);
  else
    {
      Image
        *interlaced_image;

      int
        pass,
        y;

      register RunlengthPacket
        *q;

      static int
        interlace_rate[4] = { 8, 8, 4, 2 },
        interlace_start[4] = { 0, 4, 2, 1 };

      /*
        Interlace image.
      */
      if (!UncompressImage(image))
        return(False);
      image->orphan=True;
      interlaced_image=CopyImage(image,image->columns,image->rows,False);
      image->orphan=False;
      if (interlaced_image == (Image *) NULL)
        PrematureExit("Unable to allocate memory",image);
      p=image->pixels;
      q=interlaced_image->pixels;
      for (pass=0; pass < 4; pass++)
      {
        y=interlace_start[pass];
        while (y < image->rows)
        {
          p=image->pixels+(y*image->columns);
          for (x=0; x < image->columns; x++)
          {
            *q=(*p);
            p++;
            q++;
          }
          y+=interlace_rate[pass];
        }
      }
      interlaced_image->file=image->file;
      status=GIFEncodeImage(interlaced_image,Max(bits_per_pixel,2)+1);
      interlaced_image->file=(FILE *) NULL;
      DestroyImage(interlaced_image);
    }
  if (status == False)
    PrematureExit("Unable to allocate memory",image);
  (void) fputc(0x0,image->file);
  (void) fputc(';',image->file); /* terminator */
  CloseImage(image);
  return(True);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   W r i t e G R A Y I m a g e                                               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Function WriteGRAYImage writes an image to a file as gray scale intensity
%  values.
%
%  The format of the WriteGRAYImage routine is:
%
%      status=WriteGRAYImage(image_info,image)
%
%  A description of each parameter follows.
%
%    o status: Function WriteGRAYImage return True if the image is written.
%      False is returned is there is a memory shortage or if the image file
%      fails to write.
%
%    o image_info: Specifies a pointer to an ImageInfo structure.
%
%    o image:  A pointer to a Image structure.
%
%
*/
static unsigned int WriteGRAYImage(ImageInfo *image_info, Image *image)
{
  register int
    i,
    j;

  register RunlengthPacket
    *p;

  /*
    Open output image file.
  */
  OpenImage(image,WriteBinaryType);
  if (image->file == (FILE *) NULL)
    PrematureExit("Unable to open file",image);
  /*
    Convert image to gray scale PseudoColor class.
  */
  p=image->pixels;
  for (i=0; i < image->packets; i++)
  {
    for (j=0; j <= ((int) p->length); j++)
      WriteQuantumFile(Intensity(*p));
    p++;
    if (QuantumTick(i,image))
      ProgressMonitor(SaveImageText,i,image->packets);
  }
  CloseImage(image);
  return(True);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   W r i t e H D F I m a g e                                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Function WriteHDFImage writes an image in the Hierarchial Data Format image
%  format.
%
%  The format of the WriteHDFImage routine is:
%
%      status=WriteHDFImage(image_info,image)
%
%  A description of each parameter follows.
%
%    o status: Function WriteHDFImage return True if the image is written.
%      False is returned is there is a memory shortage or if the image file
%      fails to write.
%
%    o image_info: Specifies a pointer to an ImageInfo structure.
%
%    o image:  A pointer to a Image structure.
%
%
*/
#ifdef HasHDF
static unsigned int WriteHDFImage(image_info,image)
ImageInfo
  *image_info;

Image
  *image;
{
#include "hdf.h"

  int
    status;

  register int
    i,
    j;

  register RunlengthPacket
    *p;

  register unsigned char
    *q;

  uint16
    reference;

  unsigned char
    *hdf_pixels;

  unsigned int
    packet_size;

  /*
    Open output image file.
  */
  OpenImage(image,WriteBinaryType);
  if (image->file == (FILE *) NULL)
    PrematureExit("Unable to open file",image);
  CloseImage(image);
  /*
    Initialize raster file header.
  */
  packet_size=1;
  if (image->class == DirectClass)
    packet_size=3;
  hdf_pixels=(unsigned char *)
    malloc(packet_size*image->columns*image->rows*sizeof(unsigned char));
  if (hdf_pixels == (unsigned char *) NULL)
    PrematureExit("Unable to allocate memory",image);
  p=image->pixels;
  if (!IsPseudoClass(image) && !IsGrayImage(image))
    {
      /*
        Convert DirectClass packet to HDF pixels.
      */
      q=hdf_pixels;
      switch (image_info->interlace)
      {
        case NoneInterlace:
        default:
        {
          /*
            No interlacing:  RGBRGBRGBRGBRGBRGB...
          */
          for (i=0; i < image->packets; i++)
          {
            for (j=0; j <= ((int) p->length); j++)
            {
              *q++=DownScale(p->red);
              *q++=DownScale(p->green);
              *q++=DownScale(p->blue);
            }
            p++;
            if (QuantumTick(i,image))
              ProgressMonitor(SaveImageText,i,image->packets);
          }
          break;
        }
        case LineInterlace:
        {
          register int
            x,
            y;

          /*
            Line interlacing:  RRR...GGG...BBB...RRR...GGG...BBB...
          */
          if (!UncompressImage(image))
            return(False);
          for (y=0; y < image->rows; y++)
          {
            p=image->pixels+(y*image->columns);
            for (x=0; x < image->columns; x++)
            {
              *q++=DownScale(p->red);
              p++;
            }
            p=image->pixels+(y*image->columns);
            for (x=0; x < image->columns; x++)
            {
              *q++=DownScale(p->green);
              p++;
            }
            p=image->pixels+(y*image->columns);
            for (x=0; x < image->columns; x++)
            {
              *q++=DownScale(p->blue);
              p++;
            }
            ProgressMonitor(SaveImageText,y,image->rows);
          }
          break;
        }
        case PlaneInterlace:
        {
          /*
            Plane interlacing:  RRRRRR...GGGGGG...BBBBBB...
          */
          for (i=0; i < image->packets; i++)
          {
            for (j=0; j <= ((int) p->length); j++)
              *q++=DownScale(p->red);
            p++;
          }
          ProgressMonitor(SaveImageText,100,400);
          p=image->pixels;
          for (i=0; i < image->packets; i++)
          {
            for (j=0; j <= ((int) p->length); j++)
              *q++=DownScale(p->green);
            p++;
          }
          ProgressMonitor(SaveImageText,250,400);
          p=image->pixels;
          for (i=0; i < image->packets; i++)
          {
            for (j=0; j <= ((int) p->length); j++)
              *q++=DownScale(p->blue);
            p++;
          }
          ProgressMonitor(SaveImageText,400,400);
          break;
        }
      }
      DF24setil(image_info->interlace-1);
      status=DF24putimage(image->filename,(void *) hdf_pixels,image->columns,
        image->rows);
      reference=DF24lastref();
    }
  else
    {
      /*
        Convert PseudoClass packet to HDF pixels.
      */
      q=hdf_pixels;
      if (IsGrayImage(image))
        for (i=0; i < image->packets; i++)
        {
          for (j=0; j <= (int) p->length; j++)
            *q++=DownScale(p->red);
          p++;
          if (QuantumTick(i,image))
            ProgressMonitor(SaveImageText,i,image->packets);
        }
      else
        {
          unsigned char
            *hdf_palette;

          hdf_palette=(unsigned char *) malloc(768*sizeof(unsigned char));
          if (hdf_palette == (unsigned char *) NULL)
            PrematureExit("Unable to allocate memory",image);
          q=hdf_palette;
          for (i=0; i < image->colors; i++)
          {
            *q++=DownScale(image->colormap[i].red);
            *q++=DownScale(image->colormap[i].green);
            *q++=DownScale(image->colormap[i].blue);
          }
          (void) DFR8setpalette(hdf_palette);
          free(hdf_palette);
          q=hdf_pixels;
          for (i=0; i < image->packets; i++)
          {
            for (j=0; j <= (int) p->length; j++)
              *q++=p->index;
            p++;
            if (QuantumTick(i,image))
              ProgressMonitor(SaveImageText,i,image->packets);
          }
        }
      status=DFR8putimage(image->filename,(void *) hdf_pixels,image->columns,
        image->rows,image->compression == NoCompression ? 0 : 11);
      reference=DFR8lastref();
    }
  if (image->label != (char *) NULL)
    (void) DFANputlabel(image->filename,DFTAG_RIG,reference,image->label);
  if (image->comments != (char *) NULL)
    (void) DFANputdesc(image->filename,DFTAG_RIG,reference,image->comments,
      strlen(image->comments)+1);
  free(hdf_pixels);
  return(status != -1);
}
#else
static unsigned int WriteHDFImage(ImageInfo *image_info, Image *image)
{
  unsigned int
    status;

  Warning("HDF library is not available",image->filename);
  status=WriteMIFFImage(image_info,image);
  return(status);
}
#endif

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   W r i t e H I S T O G R A M I m a g e                                     %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Function WriteHISTOGRAMImage writes an image to a file in HISTOGRAM format.
%  The image shows a histogram of the color (or gray) values in the image.  The
%  image consists of three overlaid histograms:  a red one for the red channel,
%  a green one for the green channel, and a blue one for the blue channel.  The
%  image comment contains a list of unique pixel values and the number of times
%  each occurs in the image.
%
%  This routine is strongly based on a similiar one written by
%  muquit@warm.semcor.com which in turn is based on ppmhistmap of netpbm.
%
%  The format of the WriteHISTOGRAMImage routine is:
%
%      status=WriteHISTOGRAMImage(image_info,image)
%
%  A description of each parameter follows.
%
%    o status: Function WriteHISTOGRAMImage return True if the image is written.
%      False is returned is there is a memory shortage or if the image file
%      fails to write.
%
%    o image_info: Specifies a pointer to an ImageInfo structure.
%
%    o image:  A pointer to a Image structure.
%
%
*/
static unsigned int WriteHISTOGRAMImage(ImageInfo *image_info, Image *image)
{
#define HistogramDensity  "256x200"

  char
    filename[MaxTextLength];

  double
    scale;

  FILE
    *file;

  Image
    *histogram_image;

  int
    *blue,
    *green,
    maximum,
    *red,
    sans_offset;

  register RunlengthPacket
    *p,
    *q;

  register int
    i,
    j;

  unsigned int
    height,
    status,
    width;

  /*
    Allocate histogram image.
  */
  (void) XParseGeometry(HistogramDensity,&sans_offset,&sans_offset,
    &width,&height);
  if (image_info->density != (char *) NULL)
    (void) XParseGeometry(image_info->density,&sans_offset,&sans_offset,
      &width,&height);
  image->orphan=True;
  histogram_image=CopyImage(image,width,height,False);
  image->orphan=False;
  if (histogram_image == (Image *) NULL)
    PrematureExit("Unable to allocate memory",image);
  histogram_image->class=DirectClass;
  /*
    Allocate histogram count arrays.
  */
  red=(int *) malloc (histogram_image->columns*sizeof(int));
  green=(int *) malloc (histogram_image->columns*sizeof(int));
  blue=(int *) malloc (histogram_image->columns*sizeof(int));
  if ((red == (int *) NULL) || (green == (int *) NULL) ||
      (blue == (int *) NULL))
    {
      DestroyImage(histogram_image);
      PrematureExit("Unable to allocate memory",image);
    }
  /*
    Initialize histogram count arrays.
  */
  for (i=0; i < histogram_image->columns; i++)
  {
    red[i]=0;
    green[i]=0;
    blue[i]=0;
  }
  p=image->pixels;
  for (i=0; i < image->packets; i++)
  {
    red[DownScale(p->red)]+=(p->length+1);
    green[DownScale(p->green)]+=(p->length+1);
    blue[DownScale(p->blue)]+=(p->length+1);
    p++;
  }
  maximum=0;
  for (i=0; i < histogram_image->columns; i++)
  {
    if (maximum < red[i])
      maximum=red[i];
    if (maximum < green[i])
      maximum=green[i];
    if (maximum < blue[i])
      maximum=blue[i];
  }
  for (i=0; i < histogram_image->columns; i++)
  {
    if (red[i] > maximum)
      red[i]=maximum;
    if (green[i] > maximum)
      green[i]=maximum;
    if (blue[i] > maximum)
      blue[i]=maximum;
  }
  /*
    Initialize histogram image.
  */
  q=histogram_image->pixels;
  for (i=0; i < histogram_image->packets; i++)
  {
    q->red=0;
    q->green=0;
    q->blue=0;
    q->index=0;
    q->length=0;
    q++;
  }
  scale=(double) histogram_image->rows/maximum;
  q=histogram_image->pixels;
  for (i=0; i < histogram_image->columns; i++)
  {
    j=histogram_image->rows-(int) (scale*red[i]);
    while (j < histogram_image->rows)
    {
      q=histogram_image->pixels+(j*histogram_image->columns+i);
      q->red=MaxRGB;
      j++;
    }
    j=histogram_image->rows-(int) (scale*green[i]);
    while (j < histogram_image->rows)
    {
      q=histogram_image->pixels+(j*histogram_image->columns+i);
      q->green=MaxRGB;
      j++;
    }
    j=histogram_image->rows-(int) (scale*blue[i]);
    while (j < histogram_image->rows)
    {
      q=histogram_image->pixels+(j*histogram_image->columns+i);
      q->blue=MaxRGB;
      j++;
    }
    ProgressMonitor(SaveImageText,i,histogram_image->columns);
  }
  free ((char *) blue);
  free ((char *) green);
  free ((char *) red);
  TemporaryFilename(filename);
  file=fopen(filename,"w");
  if (file != (FILE *) NULL)
    {
      char
        command[MaxTextLength];

      /*
        Add a histogram as an image comment.
      */
      (void) fprintf(file,"%s\n",image->comments);
      NumberColors(image,file);
      (void) fclose(file);
      (void) sprintf(command,"@%s",filename);
      CommentImage(histogram_image,command);
      (void) unlink(filename);
    }
  /*
    Write HISTOGRAM image as MIFF.
  */
  status=WriteMIFFImage(image_info,histogram_image);
  DestroyImage(histogram_image);
  return(status);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   W r i t e H T M L I m a g e                                               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Function WriteHTMLImage writes an image in the HTML encoded image format.
%
%  The format of the WriteHTMLImage routine is:
%
%      status=WriteHTMLImage(image_info,image)
%
%  A description of each parameter follows.
%
%    o status: Function WriteHTMLImage return True if the image is written.
%      False is returned is there is a memory shortage or if the image file
%      fails to write.
%
%    o image_info: Specifies a pointer to an ImageInfo structure.
%
%    o image:  A pointer to a Image structure.
%
%
*/
static unsigned int WriteHTMLImage(ImageInfo *image_info, Image *image)
{
  char
    color[MaxTextLength],
    filename[MaxTextLength];

  int
    x,
    y;

  register char
    *p;

  unsigned int
    height,
    status,
    width;

  /*
    Open output image file.
  */
  OpenImage(image,WriteBinaryType);
  if (image->file == (FILE *) NULL)
    PrematureExit("Unable to open file",image);
  (void) strcpy(filename,image->filename);
  if (*image->magick_filename != '\0')
    (void) strcpy(image->filename,image->magick_filename);
  (void) fprintf(image->file,"<html version=\"2.0\">\n");
  (void) fprintf(image->file,"<head>\n");
  (void) fprintf(image->file,"<title>%s</title>\n",image->filename);
  (void) fprintf(image->file,"</head>\n");
  (void) fprintf(image->file,"<body>\n");
  (void) fprintf(image->file,"<center>\n");
  (void) fprintf(image->file,"<h1>%s</h1>\n",image->filename);
  (void) fprintf(image->file,"<br><br>\n");
  (void) strcpy(filename,image->filename);
  AppendImageFormat("gif",filename);
  (void) fprintf(image->file,"<img src=\"%s\" border=none >\n",filename);
  (void) fprintf(image->file,"</center>\n");
  (void) fprintf(image->file,"<br><br><pre>\n");
  DescribeImage(image,image->file,True);
  (void) fprintf(image->file,"</pre>\n");
  (void) fprintf(image->file,"</body>\n");
  status=fprintf(image->file,"</html>\n");
  CloseImage(image);
  if (image->montage != (char *) NULL)
    {
      /*
        Open image map.
      */
      (void) strcpy(image->filename,filename);
      AppendImageFormat("map",image->filename);
      OpenImage(image,WriteBinaryType);
      if (image->file == (FILE *) NULL)
        PrematureExit("Unable to open file",image);
      /*
        Determine the size and location of each image tile.
      */
      x=0;
      y=0;
      width=image->columns;
      height=image->rows;
      (void) XParseGeometry(image->montage,&x,&y,&width,&height);
      /*
        Write an image map.
      */
      (void) fprintf(image->file,"default default.html\n");
      (void) fprintf(image->file,"rect ");
      for (p=image->directory; *p != '\0'; p++)
        if (*p != '\n')
          (void) fputc(*p,image->file);
        else
          {
            (void) fprintf(image->file," %d,%d %d,%d\n",x,y,
              x+(int) width-1,y+(int) height-1);
            if (*(p+1) != '\0')
              (void) fprintf(image->file,"rect ");
            x+=width;
            if (x >= image->columns)
              {
                x=0;
                y+=height;
              }
          }
      (void) sprintf(color,"#%02x%02x%02x",(unsigned int) image->pixels[0].red,
        (unsigned int) image->pixels[0].green,(unsigned int)
        image->pixels[0].blue);
      TransparentImage(image,color);
      CloseImage(image);
    }
  if (strcmp(image_info->magick,"GIF") != 0)
    {
      /*
        Write the image as transparent GIF.
      */
      AppendImageFormat("gif",image->filename);
      status|=WriteGIFImage(image_info,image);
    }
  (void) strcpy(image->filename,filename);
  return(status);
}

#ifdef HasJBIG
#include "jbig.h"

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   W r i t e J B I G I m a g e                                               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Function WriteJBIGImage writes an image in the JBIG encoded image format.
%
%  The format of the WriteJBIGImage routine is:
%
%      status=WriteJBIGImage(image_info,image)
%
%  A description of each parameter follows.
%
%    o status: Function WriteJBIGImage return True if the image is written.
%      False is returned is there is a memory shortage or if the image file
%      fails to write.
%
%    o image_info: Specifies a pointer to an ImageInfo structure.
%
%    o image:  A pointer to a Image structure.
%
%
*/

void JBIGEncode(start,length,file)
unsigned char
   *start;

size_t
  length;

void
   *file;
{
  (void) fwrite(start,length,1,(FILE *) file);
  return;
}

static unsigned int WriteJBIGImage(image_info,image)
ImageInfo
  *image_info;

Image
  *image;
{
  int
    sans_offset;

  register int
    i,
    j;

  register RunlengthPacket
    *p;

  register unsigned char
    bit,
    *q;

  struct jbg_enc_state
    jbig_info;

  unsigned char
    *pixels,
    polarity;

  unsigned int
    Byte,
    number_packets,
    x,
    x_resolution,
    y_resolution;

  /*
    Open image file.
  */
  OpenImage(image,WriteBinaryType);
  if (image->file == (FILE *) NULL)
    PrematureExit("Unable to open file",image);
  /*
    Allocate pixel data.
  */
  number_packets=((image->columns+7) >> 3)*image->rows;
  pixels=(unsigned char *) malloc(number_packets*sizeof(unsigned char));
  if (pixels == (unsigned char *) NULL)
    PrematureExit("Unable to allocate memory",image);
  /*
    Convert Runlength encoded pixels to a bitmap.
  */
  if (!IsGrayImage(image) || (image->colors != 2))
    {
      QuantizeImage(image,2,8,image_info->dither,GRAYColorspace);
      SyncImage(image);
    }
  polarity=0;
  if (image->colors == 2)
    polarity=Intensity(image->colormap[0]) > Intensity(image->colormap[1]);
  bit=0;
  Byte=0;
  x=0;
  p=image->pixels;
  q=pixels;
  for (i=0; i < image->packets; i++)
  {
    for (j=0; j <= (int) p->length; j++)
    {
      Byte<<=1;
      if (p->index == polarity)
        Byte|=0x01;
      bit++;
      if (bit == 8)
        {
          *q++=Byte;
          bit=0;
          Byte=0;
        }
      x++;
      if (x == image->columns)
        {
          /*
            Advance to the next scanline.
          */
          if (bit != 0)
            *q++=Byte << (8-bit);
          bit=0;
          Byte=0;
          x=0;
       }
    }
    p++;
    if (QuantumTick(i,image))
      ProgressMonitor(SaveImageText,i,image->packets);
  }
  /*
    Initialize JBIG info structure.
  */
  jbg_enc_init(&jbig_info,image->columns,image->rows,1,&pixels,
    (void (*) _Declare((unsigned char *,size_t,void *))) JBIGEncode,
    image->file);
  x_resolution=640;
  y_resolution=480;
  (void) XParseGeometry(image_info->density,&sans_offset,&sans_offset,
    &x_resolution,&y_resolution);
  if (image_info->subimage != 0)
    jbg_enc_layers(&jbig_info,image_info->subimage);
  else
    jbg_enc_lrlmax(&jbig_info,x_resolution,y_resolution);
  jbg_enc_lrange(&jbig_info,-1,-1);
  jbg_enc_options(&jbig_info,JBG_ILEAVE | JBG_SMID,JBG_TPDON | JBG_TPBON |
    JBG_DPON,-1,-1,-1);
  /*
    Write JBIG image.
  */
  jbg_enc_out(&jbig_info);
  jbg_enc_free(&jbig_info);
  free((char *) pixels);
  CloseImage(image);
  return(True);
}
#else
static unsigned int WriteJBIGImage(ImageInfo *image_info, Image *image)
{
  unsigned int
    status;

  Warning("JBIG library is not available",image->filename);
  status=WriteMIFFImage(image_info,image);
  return(status);
}
#endif

#ifdef HasJPEG
#include "jpeglib.h"
#include "jerror.h"

static Image
  *image;

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%  W r i t e J P E G I m a g e                                                %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Function WriteJPEGImage writes a JPEG image file and returns it.  It
%  allocates the memory necessary for the new Image structure and returns a
%  pointer to the new image.
%
%  The format of the WriteJPEGImage routine is:
%
%      status=WriteJPEGImage(image_info,image)
%
%  A description of each parameter follows:
%
%    o status:  Function WriteJPEGImage return True if the image is written.
%      False is returned is there is of a memory shortage or if the image
%      file cannot be opened for writing.
%
%    o image_info: Specifies a pointer to an ImageInfo structure.
%
%    o jpeg_image:  A pointer to a Image structure.
%
%
*/

static void EmitMessage(j_common_ptr jpeg_info, int level)
{
  char
    message[JMSG_LENGTH_MAX];

  struct jpeg_error_mgr
    *jpeg_error;

  jpeg_error=jpeg_info->err;
  (jpeg_error->format_message) (jpeg_info,message);
  if (level < 0)
    {
      if (jpeg_error->num_warnings == 0 || jpeg_error->trace_level >= 3)
        Warning((char *) message,image->filename);
      jpeg_error->num_warnings++;
    }
  else
    if (jpeg_error->trace_level >= level)
      Warning((char *) message,image->filename);
}

static unsigned int WriteJPEGImage(ImageInfo *image_info, Image *image)
{
  JSAMPLE
    *jpeg_pixels;

  JSAMPROW
    scanline[1];

  register int
    i,
    j,
    x;

  register JSAMPLE
    *q;

  register RunlengthPacket
    *p;

  struct jpeg_compress_struct
    jpeg_info;

  struct jpeg_error_mgr
    jpeg_error;

  unsigned int
    packets;

  /*
    Open image file.
  */
  OpenImage(image,WriteBinaryType);
  if (image->file == (FILE *) NULL)
    PrematureExit("Unable to open file",image);
  /*
    Initialize JPEG parameters.
  */
  jpeg_info.err=jpeg_std_error(&jpeg_error);
  jpeg_info.err->emit_message=EmitMessage;
  jpeg_create_compress(&jpeg_info);
  jpeg_stdio_dest(&jpeg_info,image->file);
  jpeg_info.image_width=image->columns;
  jpeg_info.image_height=image->rows;
  jpeg_info.input_components=3;
  jpeg_info.in_color_space=JCS_RGB;
  if (IsGrayImage(image))
    {
      jpeg_info.input_components=1;
      jpeg_info.in_color_space=JCS_GRAYSCALE;
    }
  jpeg_set_defaults(&jpeg_info);
  for (i=0; i < MAX_COMPONENTS; i++)
  {
    jpeg_info.comp_info[i].h_samp_factor=1;
    jpeg_info.comp_info[i].v_samp_factor=1;
  }
  jpeg_set_quality(&jpeg_info,image_info->quality,True);
  jpeg_info.optimize_coding=True;
#ifdef DCT_FLOAT_SUPPORTED
  jpeg_info.dct_method=JDCT_FLOAT;
#endif
#ifdef C_PROGRESSIVE_SUPPORTED
  if (image_info->interlace != NoneInterlace)
    jpeg_simple_progression(&jpeg_info);
#endif
  jpeg_start_compress(&jpeg_info,True);
  if (image->comments != (char *) NULL)
    for (i=0; i < (int) strlen(image->comments); i+=65533)
      jpeg_write_marker(&jpeg_info,JPEG_COM,(unsigned char *) image->comments+i,
        (unsigned int) Min((int) strlen(image->comments+i),65533));
  /*
    Convert MIFF to JPEG raster pixels.
  */
  packets=jpeg_info.input_components*image->columns;
  jpeg_pixels=(JSAMPLE *) malloc(packets*sizeof(JSAMPLE));
  if (jpeg_pixels == (JSAMPLE *) NULL)
    PrematureExit("Unable to allocate memory",image);
  p=image->pixels;
  q=jpeg_pixels;
  x=0;
  scanline[0]=(JSAMPROW) jpeg_pixels;
  if ((jpeg_info.data_precision > 8) && (QuantumDepth > 8))
    {
      if (jpeg_info.in_color_space == JCS_GRAYSCALE)
        for (i=0; i < image->packets; i++)
        {
          for (j=0; j <= (int) p->length; j++)
          {
            *q++=(JSAMPLE) (Intensity(*p) >> 4);
            x++;
            if (x == image->columns)
              {
                (void) jpeg_write_scanlines(&jpeg_info,scanline,1);
                q=jpeg_pixels;
                x=0;
              }
          }
          p++;
          if (QuantumTick(i,image))
            ProgressMonitor(SaveImageText,i,image->packets);
        }
      else
        for (i=0; i < image->packets; i++)
        {
          for (j=0; j <= (int) p->length; j++)
          {
            *q++=(JSAMPLE) (p->red >> 4);
            *q++=(JSAMPLE) (p->green >> 4);
            *q++=(JSAMPLE) (p->blue >> 4);
            x++;
            if (x == image->columns)
              {
                (void) jpeg_write_scanlines(&jpeg_info,scanline,1);
                q=jpeg_pixels;
                x=0;
              }
          }
          p++;
          if (QuantumTick(i,image))
            ProgressMonitor(SaveImageText,i,image->packets);
        }
    }
  else
    if (jpeg_info.in_color_space == JCS_GRAYSCALE)
      for (i=0; i < image->packets; i++)
      {
        for (j=0; j <= (int) p->length; j++)
        {
          *q++=(JSAMPLE) DownScale(Intensity(*p));
          x++;
          if (x == image->columns)
            {
              (void) jpeg_write_scanlines(&jpeg_info,scanline,1);
              q=jpeg_pixels;
              x=0;
            }
        }
        p++;
        if (QuantumTick(i,image))
          ProgressMonitor(SaveImageText,i,image->packets);
      }
    else
      for (i=0; i < image->packets; i++)
      {
        for (j=0; j <= (int) p->length; j++)
        {
          *q++=(JSAMPLE) DownScale(p->red);
          *q++=(JSAMPLE) DownScale(p->green);
          *q++=(JSAMPLE) DownScale(p->blue);
          x++;
          if (x == image->columns)
            {
              (void) jpeg_write_scanlines(&jpeg_info,scanline,1);
              q=jpeg_pixels;
              x=0;
            }
        }
        p++;
        if (QuantumTick(i,image))
          ProgressMonitor(SaveImageText,i,image->packets);
      }
  jpeg_finish_compress(&jpeg_info);
  /*
    Free memory.
  */
  jpeg_destroy_compress(&jpeg_info);
  free((char *) jpeg_pixels);
  CloseImage(image);
  return(True);
}
#else
static unsigned int WriteJPEGImage(ImageInfo* image_info, Image* image)
{
  unsigned int
    status;

  Warning("JPEG library is not available",image->filename);
  status=WriteMIFFImage(image_info,image);
  return(status);
}
#endif

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   W r i t e L O G O I m a g e                                               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Function WriteLOGOImage writes an image in the LOGO encoded image format.
%
%  The format of the WriteLOGOImage routine is:
%
%      status=WriteLOGOImage(image_info,image)
%
%  A description of each parameter follows.
%
%    o status: Function WriteLOGOImage return True if the image is written.
%      False is returned is there is a memory shortage or if the image file
%      fails to write.
%
%    o image_info: Specifies a pointer to an ImageInfo structure.
%
%    o image:  A pointer to a Image structure.
%
%
*/
static unsigned int WriteLOGOImage(ImageInfo *image_info, Image *image)
{
  unsigned int
    status;

  Warning("Cannot write LOGO images",image->filename);
  status=WriteMIFFImage(image_info,image);
  return(status);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   W r i t e M A P I m a g e                                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Function WriteMAPImage writes an image to a file as red, green, and blue
%  colormap bytes followed by the colormap indexes.
%
%  The format of the WriteMAPImage routine is:
%
%      status=WriteMAPImage(image_info,image)
%
%  A description of each parameter follows.
%
%    o status: Function WriteMAPImage return True if the image is written.
%      False is returned is there is a memory shortage or if the image file
%      fails to write.
%
%    o image_info: Specifies a pointer to an ImageInfo structure.
%
%    o image:  A pointer to a Image structure.
%
%
*/
static unsigned int WriteMAPImage(ImageInfo *image_info, Image *image)
{
  register int
    i;

  register unsigned char
    *q;

  unsigned char
    *colormap;

  unsigned int
    packet_size;

  unsigned short
    value;

  /*
    Open output image file.
  */
  OpenImage(image,WriteBinaryType);
  if (image->file == (FILE *) NULL)
    PrematureExit("Unable to open file",image);
  /*
    Allocate colormap.
  */
  if (image->class == DirectClass)
    {
      /*
        Demote DirectClass to PseudoClass.
      */
      QuantizeImage(image,MaxColormapSize,8,image_info->dither,RGBColorspace);
      SyncImage(image);
    }
  packet_size=3*(image->depth >> 3);
  colormap=(unsigned char *)
    malloc(packet_size*image->colors*sizeof(unsigned char));
  if (colormap == (unsigned char *) NULL)
    PrematureExit("Unable to allocate memory",image);
  /*
    Write colormap to file.
  */
  q=colormap;
  for (i=0; i < image->colors; i++)
  {
    WriteQuantum(image->colormap[i].red,q);
    WriteQuantum(image->colormap[i].green,q);
    WriteQuantum(image->colormap[i].blue,q);
  }
  (void) fwrite((char *) colormap,1,(int) image->colors*packet_size,
    image->file);
  free((char *) colormap);
  /*
    Write image pixels to file.
  */
  image->compression=NoCompression;
  (void) RunlengthEncodeImage(image);
  (void) fwrite((char *) image->packed_pixels,(int) image->packet_size,
    (int) image->packets,image->file);
  free((char *) image->packed_pixels);
  image->packed_pixels=(unsigned char *) NULL;
  CloseImage(image);
  return(True);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   W r i t e M A T T E I m a g e                                             %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Function WriteMATTEImage writes an mage of matte bytes to a file.  It
%  consists of data from the matte component of the image [0..255].
%
%  The format of the WriteMATTEImage routine is:
%
%      status=WriteMATTEImage(image_info,image)
%
%  A description of each parameter follows.
%
%    o status: Function WriteMATTEImage return True if the image is written.
%      False is returned is there is a memory shortage or if the image file
%      fails to write.
%
%    o image_info: Specifies a pointer to an ImageInfo structure.
%
%    o image:  A pointer to a Image structure.
%
%
*/
static unsigned int WriteMATTEImage(ImageInfo *image_info, Image *image)
{
  register int
    i,
    j;

  register RunlengthPacket
    *p;

  if (!image->matte)
    PrematureExit("Image does not have a matte channel",image);
  /*
    Open output image file.
  */
  OpenImage(image,WriteBinaryType);
  if (image->file == (FILE *) NULL)
    PrematureExit("Unable to open file",image);
  /*
    Allocate matte pixels.
  */
  p=image->pixels;
  for (i=0; i < image->packets; i++)
  {
    for (j=0; j <= ((int) p->length); j++)
      (void) fputc(DownScale(p->index),image->file);
    p++;
    if (QuantumTick(i,image))
      ProgressMonitor(SaveImageText,i,image->packets);
  }
  CloseImage(image);
  return(True);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   W r i t e M I F F I m a g e                                               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Function WriteMIFFImage writes an image to a file.
%
%  The format of the WriteMIFFImage routine is:
%
%      status=WriteMIFFImage(image_info,image)
%
%  A description of each parameter follows:
%
%    o status: Function WriteMIFFImage return True if the image is written.
%      False is returned if there is a memory shortage or if the image file
%      fails to write.
%
%    o image_info: Specifies a pointer to an ImageInfo structure.
%
%    o image: A pointer to a Image structure.
%
%
*/
#ifdef HasPNG
#include "zlib.h"
#endif

static unsigned int WriteMIFFImage(ImageInfo *image_info, Image *image)
{
  register int
    i;

  unsigned int
    compression;

  unsigned long
    packets;

  if ((image->class != DirectClass) && (image->class != PseudoClass))
    PrematureExit("Unknown image class",image);
  if ((image->compression != RunlengthEncodedCompression) &&
      (image->compression != ZlibCompression) &&
      (image->compression != NoCompression))
    PrematureExit("Unknown image compression",image);
  /*
    Open output image file.
  */
  OpenImage(image,WriteBinaryType);
  if (image->file == (FILE *) NULL)
    PrematureExit("Unable to open file",image);
  (void) strcpy(image_info->magick,"MIFF");
  /*
    Pack image pixels.
  */
  compression=image->compression;
  (void) RunlengthEncodeImage(image);
  packets=image->packets;
  if (compression == ZlibCompression)
    {
      int
        status;

      unsigned char
        *compressed_pixels;

      unsigned int
        compressed_packets;

      /*
        Compress image pixels with Zlib encoding.
      */
      compressed_packets=((packets*image->packet_size*110)/100)+12;
      compressed_pixels=(unsigned char *)
        malloc(compressed_packets*sizeof(unsigned char));
      if (compressed_pixels == (unsigned char *) NULL)
        PrematureExit("Unable to allocate memory",image);
      status=True;
#ifdef HasPNG
      status=compress(compressed_pixels,&packets,image->packed_pixels,
        image->packets*image->packet_size);
#endif
      if (status)
        {
          Warning("Unable to Zlib compress image",image->filename);
          image->compression=NoCompression;
        }
      else
        {
          free((char *) image->packed_pixels);
          image->packed_pixels=compressed_pixels;
          image->packet_size=1;
        }
    }
  if (image->class == PseudoClass)
    ColormapSignature(image);
  /*
    Write header to file.
  */
  (void) fprintf(image->file,"id=ImageMagick\n");
  if (image->class == PseudoClass)
    (void) fprintf(image->file,"class=PseudoClass  colors=%u  signature=%s\n",
      image->colors,image->signature);
  else
    if (image->matte)
      (void) fprintf(image->file,"class=DirectClass  matte=True\n");
    else
      (void) fprintf(image->file,"class=DirectClass\n");
  if (image->compression == RunlengthEncodedCompression)
    (void) fprintf(image->file,"compression=RunlengthEncoded  packets=%lu\n",
      packets);
  else
    if (image->compression == ZlibCompression)
      (void) fprintf(image->file,"compression=Zlib  packets=%lu\n",packets);
  (void) fprintf(image->file,"columns=%u  rows=%u  depth=%u\n",image->columns,
    image->rows,image->depth);
  if (image->scene != 0)
    (void) fprintf(image->file,"scene=%u\n",image->scene);
  if (image->gamma != 0.0)
    (void) fprintf(image->file,"gamma=%f\n",image->gamma);
  if (image->montage != (char *) NULL)
    (void) fprintf(image->file,"montage=%s\n",image->montage);
  if (image->comments != (char *) NULL)
    (void) fprintf(image->file,"{\n%s\n}\n",image->comments);
  (void) fprintf(image->file,"\f\n:\n");
  if (image->montage != (char *) NULL)
    {
      /*
        Write montage tile directory.
      */
      if (image->directory != (char *) NULL)
        (void) fprintf(image->file,"%s",image->directory);
      (void) fputc('\0',image->file);
    }
  if (image->class == PseudoClass)
    {
      register unsigned char
        *q;

      unsigned char
        *colormap;

      unsigned int
        packet_size;

      unsigned short
        value;

      /*
        Allocate colormap.
      */
      packet_size=3*(image->depth >> 3);
      colormap=(unsigned char *)
        malloc(packet_size*image->colors*sizeof(unsigned char));
      if (colormap == (unsigned char *) NULL)
        PrematureExit("Unable to allocate memory",image);
      q=colormap;
      for (i=0; i < image->colors; i++)
      {
        WriteQuantum(image->colormap[i].red,q);
        WriteQuantum(image->colormap[i].green,q);
        WriteQuantum(image->colormap[i].blue,q);
      }
      /*
        Write colormap to file.
      */
      (void) fwrite((char *) colormap,1,(int) image->colors*packet_size,
        image->file);
      free((char *) colormap);
    }
  /*
    Write image pixels to file.
  */
  (void) fwrite((char *) image->packed_pixels,(int) image->packet_size,
    (int) packets,image->file);
  CloseImage(image);
  return(True);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   W r i t e M P E G I m a g e                                               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Function WriteMPEGImage writes an image in the MPEG encoded image format.
%
%  The format of the WriteMPEGImage routine is:
%
%      status=WriteMPEGImage(image_info,image)
%
%  A description of each parameter follows.
%
%    o status: Function WriteMPEGImage return True if the image is written.
%      False is returned is there is a memory shortage or if the image file
%      fails to write.
%
%    o image_info: Specifies a pointer to an ImageInfo structure.
%
%    o image:  A pointer to a Image structure.
%
%
*/
static unsigned int WriteMPEGImage(ImageInfo *image_info, Image *image)
{
  char
    basename[MaxTextLength],
    command[MaxTextLength],
    filename[MaxTextLength];

  Image
    component_image;

  MonitorHandler
    handler;

  register int
    i;

  unsigned int
    number_images,
    scene,
    status;

  /*
    Write component images.
  */
  scene=image->scene;
  for (number_images=0; image->next != (Image *) NULL; number_images++)
    image=image->next;
  number_images++;
  while (image->previous != (Image *) NULL)
    image=image->previous;
  TemporaryFilename(basename);
  (void) strcpy(filename,basename);
  (void) strcat(filename,"%d");
  for (i=0; i < number_images; i++)
  {
    handler=SetMonitorHandler((MonitorHandler) NULL);
    component_image=(*image);
    component_image.previous=(Image *) NULL;
    component_image.next=(Image *) NULL;
    (void) sprintf(component_image.filename,filename,i);
    (void) WriteYUV3Image(image_info,&component_image);
    if (image->next != (Image *) NULL)
      image=image->next;
    (void) SetMonitorHandler(handler);
    ProgressMonitor(SaveImageText,i,number_images);
  }
  /*
    Write MPEG image.
  */
  (void) sprintf(command,"mpeg -a %d -b %d -h %d -v %d -PF %s -s %s",scene,
    image->scene,image->columns,image->rows,basename,image->filename);
  status=SystemCommand(command);
  /*
    Remove component files.
  */
  for (i=0; i < number_images; i++)
  {
    (void) sprintf(component_image.filename,filename,i);
    (void) strcat(component_image.filename,"Y");
    (void) unlink(component_image.filename);
    (void) sprintf(component_image.filename,filename,i);
    (void) strcat(component_image.filename,"U");
    (void) unlink(component_image.filename);
    (void) sprintf(component_image.filename,filename,i);
    (void) strcat(component_image.filename,"V");
    (void) unlink(component_image.filename);
  }
  return(status);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   W r i t e M T V I m a g e                                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Function WriteMTVImage writes an image to a file in red, green, and blue
%  MTV rasterfile format.
%
%  The format of the WriteMTVImage routine is:
%
%      status=WriteMTVImage(image_info,image)
%
%  A description of each parameter follows.
%
%    o status: Function WriteMTVImage return True if the image is written.
%      False is returned is there is a memory shortage or if the image file
%      fails to write.
%
%    o image_info: Specifies a pointer to an ImageInfo structure.
%
%    o image:  A pointer to a Image structure.
%
%
*/
static unsigned int WriteMTVImage(ImageInfo *image_info, Image *image)
{
  register int
    i,
    j;

  register RunlengthPacket
    *p;

  /*
    Open output image file.
  */
  OpenImage(image,WriteBinaryType);
  if (image->file == (FILE *) NULL)
    PrematureExit("Unable to open file",image);
  /*
    Convert MIFF to MTV raster pixels.
  */
  (void) fprintf(image->file,"%u %u\n",image->columns,image->rows);
  p=image->pixels;
  for (i=0; i < image->packets; i++)
  {
    for (j=0; j <= ((int) p->length); j++)
    {
      (void) fputc(DownScale(p->red),image->file);
      (void) fputc(DownScale(p->green),image->file);
      (void) fputc(DownScale(p->blue),image->file);
    }
    p++;
    if (QuantumTick(i,image))
      ProgressMonitor(SaveImageText,i,image->packets);
  }
  CloseImage(image);
  return(True);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   W r i t e P C D I m a g e                                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Function WritePCDImage writes an image in the Photo CD encoded image
%  format.
%
%  The format of the WritePCDImage routine is:
%
%      status=WritePCDImage(image_info,image)
%
%  A description of each parameter follows.
%
%    o status: Function WritePCDImage return True if the image is written.
%      False is returned is there is a memory shortage or if the image file
%      fails to write.
%
%    o image_info: Specifies a pointer to an ImageInfo structure.
%
%    o image:  A pointer to a Image structure.
%
%
*/
static unsigned int WritePCDImage(ImageInfo *image_info, Image *image)
{
  unsigned int
    status;

  Warning("Cannot write PCD images",image->filename);
  status=WriteMIFFImage(image_info,image);
  return(status);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   W r i t e P C X I m a g e                                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Function WritePCXImage writes an image in the ZSoft IBM PC Paintbrush file
%  format.
%
%  The format of the WritePCXImage routine is:
%
%      status=WritePCXImage(image_info,image)
%
%  A description of each parameter follows.
%
%    o status: Function WritePCXImage return True if the image is written.
%      False is returned is there is a memory shortage or if the image file
%      fails to write.
%
%    o image_info: Specifies a pointer to an ImageInfo structure.
%
%    o image:  A pointer to a Image structure.
%
%
*/
static unsigned int WritePCXImage(ImageInfo *image_info, Image *image)
{
  typedef struct _PCXHeader
  {
    unsigned char
      identifier,
      version,
      encoding,
      bits_per_pixel;

    short int
      left,
      top,
      right,
      bottom,
      horizontal_resolution,
      vertical_resolution;

    unsigned char
      reserved,
      planes;

    short int
      bytes_per_line,
      palette_info;

    unsigned char
      colormap_signature;
  } PCXHeader;

  PCXHeader
    pcx_header;

  register int
    i,
    j,
    x,
    y;

  register RunlengthPacket
    *p;

  register unsigned char
    *q;

  unsigned char
    count,
    packet,
    *pcx_colormap,
    *pcx_pixels,
    previous;

  /*
    Open output image file.
  */
  OpenImage(image,WriteBinaryType);
  if (image->file == (FILE *) NULL)
    PrematureExit("Unable to open file",image);
  /*
    Initialize PCX raster file header.
  */
  if ((image->class == DirectClass) || (image->colors > 256))
    {
      /*
        Demote DirectClass to PseudoClass.
      */
      QuantizeImage(image,256,8,image_info->dither,RGBColorspace);
      SyncImage(image);
    }
  pcx_header.identifier=0x0a;
  pcx_header.version=5;
  pcx_header.encoding=1;
  pcx_header.bits_per_pixel=8;
  if (IsGrayImage(image) && (image->colors == 2))
    pcx_header.bits_per_pixel=1;
  pcx_header.left=0;
  pcx_header.top=0;
  pcx_header.right=image->columns-1;
  pcx_header.bottom=image->rows-1;
  pcx_header.horizontal_resolution=image->columns;
  pcx_header.vertical_resolution=image->rows;
  pcx_header.reserved=0;
  pcx_header.planes=1;
  pcx_header.bytes_per_line=(image->columns*pcx_header.bits_per_pixel+7)/8;
  pcx_header.palette_info=1;
  pcx_header.colormap_signature=0x0c;
  /*
    Write PCX header.
  */
  (void) fwrite(&pcx_header.identifier,1,1,image->file);
  (void) fwrite(&pcx_header.version,1,1,image->file);
  (void) fwrite(&pcx_header.encoding,1,1,image->file);
  (void) fwrite(&pcx_header.bits_per_pixel,1,1,image->file);
  LSBFirstWriteShort((unsigned int) pcx_header.left,image->file);
  LSBFirstWriteShort((unsigned int) pcx_header.top,image->file);
  LSBFirstWriteShort((unsigned int) pcx_header.right,image->file);
  LSBFirstWriteShort((unsigned int) pcx_header.bottom,image->file);
  LSBFirstWriteShort((unsigned int) pcx_header.horizontal_resolution,
    image->file);
  LSBFirstWriteShort((unsigned int) pcx_header.vertical_resolution,image->file);
  /*
    Dump colormap to file.
  */
  pcx_colormap=(unsigned char *) malloc(3*256*sizeof(unsigned char));
  if (pcx_colormap == (unsigned char *) NULL)
    PrematureExit("Unable to allocate memory",image);
  q=pcx_colormap;
  for (i=0; i < image->colors; i++)
  {
    *q++=DownScale(image->colormap[i].red);
    *q++=DownScale(image->colormap[i].green);
    *q++=DownScale(image->colormap[i].blue);
  }
  (void) fwrite((char *) pcx_colormap,3,16,image->file);
  (void) fwrite(&pcx_header.reserved,1,1,image->file);
  (void) fwrite(&pcx_header.planes,1,1,image->file);
  LSBFirstWriteShort((unsigned int) pcx_header.bytes_per_line,image->file);
  LSBFirstWriteShort((unsigned int) pcx_header.palette_info,image->file);
  for (i=0; i < 58; i++)
    (void) fwrite("\0",1,1,image->file);
  /*
    Convert MIFF to PCX raster pixels.
  */
  pcx_pixels=(unsigned char *)
    malloc(pcx_header.bytes_per_line*image->rows*sizeof(unsigned char));
  if (pcx_pixels == (unsigned char *) NULL)
    PrematureExit("Unable to allocate memory",image);
  x=0;
  y=0;
  p=image->pixels;
  q=pcx_pixels;
  if (pcx_header.bits_per_pixel > 1)
    for (i=0; i < image->packets; i++)
    {
      for (j=0; j <= (int) p->length; j++)
      {
        *q++=p->index;
        x++;
        if (x == image->columns)
          {
            x=0;
            y++;
            q=pcx_pixels+y*pcx_header.bytes_per_line;
          }
      }
      p++;
    }
  else
    {
      register unsigned char
        bit,
        Byte,
        polarity;

      /*
        Convert PseudoClass image to a PCX monochrome image.
      */
      polarity=0;
      if (image->colors == 2)
        polarity=Intensity(image->colormap[0]) < Intensity(image->colormap[1]);
      bit=0;
      Byte=0;
      for (i=0; i < image->packets; i++)
      {
        for (j=0; j <= (int) p->length; j++)
        {
          Byte<<=1;
          if (p->index == polarity)
            Byte|=0x01;
          bit++;
          if (bit == 8)
            {
              *q++=Byte;
              bit=0;
              Byte=0;
            }
          x++;
          if (x == image->columns)
            {
              /*
                Advance to the next scanline.
              */
              if (bit != 0)
                *q++=Byte << (8-bit);
              bit=0;
              Byte=0;
              x=0;
              y++;
              q=pcx_pixels+y*pcx_header.bytes_per_line;
           }
        }
        p++;
      }
    }
  /*
    Runlength-encoded PCX pixels.
  */
  for (y=0; y < image->rows; y++)
  {
    q=pcx_pixels+y*pcx_header.bytes_per_line;
    previous=(*q++);
    count=1;
    for (x=0; x < (pcx_header.bytes_per_line-1); x++)
    {
      packet=(*q++);
      if ((packet == previous) && (count < 63))
        {
          count++;
          continue;
        }
      if ((count > 1) || ((previous & 0xc0) == 0xc0))
        {
          count|=0xc0;
          (void) fwrite(&count,1,1,image->file);
        }
      (void) fwrite(&previous,1,1,image->file);
      previous=packet;
      count=1;
    }
    if ((count > 1) || ((previous & 0xc0) == 0xc0))
      {
        count|=0xc0;
        (void) fwrite(&count,1,1,image->file);
      }
    (void) fwrite(&previous,1,1,image->file);
    ProgressMonitor(SaveImageText,y,image->rows);
  }
  (void) fwrite(&pcx_header.colormap_signature,1,1,image->file);
  (void) fwrite((char *) pcx_colormap,3,256,image->file);
  free((char *) pcx_pixels);
  free((char *) pcx_colormap);
  CloseImage(image);
  return(True);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   W r i t e P D F I m a g e                                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Function WritePDFImage writes an image in the Portable Document image
%  format.
%
%  The format of the WritePDFImage routine is:
%
%      status=WritePDFImage(image_info,image)
%
%  A description of each parameter follows.
%
%    o status: Function WritePDFImage return True if the image is written.
%      False is returned is there is a memory shortage or if the image file
%      fails to write.
%
%    o image_info: Specifies a pointer to an ImageInfo structure.
%
%    o image:  A pointer to a Image structure.
%
%
*/
static unsigned int WritePDFImage(ImageInfo *image_info, Image *image)
{
#define DefaultThumbnailGeometry  "106x106"

  char
    date[MaxTextLength];

  int
    delta_x,
    delta_y,
    flags,
    sans_offset,
    x,
    xref[15],
    y;

  Image
    encode_image,
    *tile_image;

  register RunlengthPacket
    *p;

  register unsigned char
    *q;

  register int
    i,
    j;

  time_t
    timer;

  unsigned char
    *pixels;

  unsigned int
    dx_resolution,
    dy_resolution,
    height,
    length,
    number_objects,
    number_packets,
    page_height,
    page_width,
    text_size,
    x_resolution,
    y_resolution,
    width;

  /*
    Open output image file.
  */
  OpenImage(image,"w");
  if (image->file == (FILE *) NULL)
    PrematureExit("Unable to open file",image);
  if ((image->file == stdout) || image->pipe)
    {
      /*
        Write standard output or pipe to temporary file.
      */
      encode_image=(*image);
      TemporaryFilename(image->filename);
      image->temporary=True;
      OpenImage(image,"w");
      if (image->file == (FILE *) NULL)
        PrematureExit("Unable to open file",image);
    }
  /*
    Scale image to size of Postscript page.
  */
  text_size=image->label == (char *) NULL ? 0 : 36;
  x=0;
  y=0;
  width=image->columns;
  height=image->rows;
  /*
    Center image on Portable Document page.
  */
  (void) XParseGeometry(PSPageGeometry,&x,&y,&page_width,&page_height);
  flags=XParseGeometry(image_info->page,&x,&y,&page_width,&page_height);
  if (((page_width-(x << 1)) < width) ||
      ((page_height-(y << 1)-text_size) < height))
    {
      unsigned long
        scale_factor;

      /*
        Scale image relative to Portable Document page.
      */
      scale_factor=UpShift(page_width-(x << 1))/width;
      if (scale_factor > (UpShift(page_height-(y << 1)-text_size)/height))
        scale_factor=UpShift(page_height-(y << 1)-text_size)/height;
      width=DownShift(width*scale_factor);
      height=DownShift(height*scale_factor);
    }
  if ((flags & XValue) == 0)
    {
      /*
        Center image in the X direction.
      */
      delta_x=page_width-(width+(x << 1));
      if (delta_x >= 0)
        x=(delta_x >> 1)+x;
    }
  if ((flags & YValue) == 0)
    {
      /*
        Center image in the X direction.
      */
      delta_y=page_height-(height+(y << 1))-text_size;
      if (delta_y >= 0)
        y=(delta_y >> 1)+y;
    }
  /*
    Scale relative to dots-per-inch.
  */
  (void) XParseGeometry(PSDensityGeometry,&sans_offset,&sans_offset,
    &dx_resolution,&dy_resolution);
  x_resolution=dx_resolution;
  y_resolution=dy_resolution;
  (void) XParseGeometry(image_info->density,&sans_offset,&sans_offset,
    &x_resolution,&y_resolution);
  width=(width*dx_resolution)/x_resolution;
  height=(height*dy_resolution)/y_resolution;
  /*
    Write Info object.
  */
  (void) fprintf(image->file,"%%PDF-1.1 \n");
  xref[0]=ftell(image->file);
  (void) fprintf(image->file,"1 0 obj\n");
  (void) fprintf(image->file,"<<\n");
  timer=time((time_t *) NULL);
  (void) localtime(&timer);
  (void) strcpy(date,ctime(&timer));
  date[strlen(date)-1]='\0';
  (void) fprintf(image->file,"/CreationDate (%s)\n",date);
  (void) fprintf(image->file,"/Producer ImageMagick\n");
  (void) fprintf(image->file,">>\n");
  (void) fprintf(image->file,"endobj\n");
  /*
    Write Catalog object.
  */
  xref[1]=ftell(image->file);
  (void) fprintf(image->file,"2 0 obj\n");
  (void) fprintf(image->file,"<<\n");
  (void) fprintf(image->file,"/Type /Catalog\n");
  (void) fprintf(image->file,"/Pages 3 0 R\n");
  (void) fprintf(image->file,">>\n");
  (void) fprintf(image->file,"endobj\n");
  /*
    Write Pages object.
  */
  xref[2]=ftell(image->file);
  (void) fprintf(image->file,"3 0 obj\n");
  (void) fprintf(image->file,"<<\n");
  (void) fprintf(image->file,"/Type /Pages\n");
  (void) fprintf(image->file,"/Count 1\n");
  (void) fprintf(image->file,"/Kids [ 4 0 R ]\n");
  (void) fprintf(image->file,">>\n");
  (void) fprintf(image->file,"endobj\n");
  /*
    Write Page object.
  */
  xref[3]=ftell(image->file);
  (void) fprintf(image->file,"4 0 obj\n");
  (void) fprintf(image->file,"<<\n");
  (void) fprintf(image->file,"/Type /Page\n");
  (void) fprintf(image->file,"/Parent 3 0 R\n");
  (void) fprintf(image->file,"/Resources <<\n");
  (void) fprintf(image->file,"/Font << /F0 8 0 R >>\n");
  (void) fprintf(image->file,"/XObject << /Im0 9 0 R >>\n");
  (void) fprintf(image->file,"/ProcSet 7 0 R >>\n");
  (void) fprintf(image->file,"/MediaBox [ 0 0 %u %u ]\n",
    page_width,page_height);
  (void) fprintf(image->file,"/Contents 5 0 R\n");
  (void) fprintf(image->file,"/Thumb 12 0 R\n");
  (void) fprintf(image->file,">>\n");
  (void) fprintf(image->file,"endobj\n");
  /*
    Write Contents object.
  */
  xref[4]=ftell(image->file);
  (void) fprintf(image->file,"5 0 obj\n");
  (void) fprintf(image->file,"<<\n");
  (void) fprintf(image->file,"/Length 6 0 R\n");
  (void) fprintf(image->file,">>\n");
  (void) fprintf(image->file,"stream\n");
  length=ftell(image->file);
  (void) fprintf(image->file,"q\n");
  (void) fprintf(image->file,"BT\n");
  (void) fprintf(image->file,"/F0 24 Tf\n");
  (void) fprintf(image->file,"%d %d Td\n",x >> 1,y+(int) height+(y >> 1)+12);
  if (image->label == (char *) NULL)
    (void) fprintf(image->file,"() Tj\n");
  else
    (void) fprintf(image->file,"(%s) Tj\n",image->label);
  (void) fprintf(image->file,"ET\n");
  (void) fprintf(image->file,"%d 0 0 %d %d %d cm\n",x+(int) width,
    y+(int) height,x >> 1,y >> 1);
  (void) fprintf(image->file,"/Im0 Do\n");
  (void) fprintf(image->file,"Q\n");
  length=ftell(image->file)-length;
  (void) fprintf(image->file,"endstream\n");
  (void) fprintf(image->file,"endobj\n");
  /*
    Write Length object.
  */
  xref[5]=ftell(image->file);
  (void) fprintf(image->file,"6 0 obj\n");
  (void) fprintf(image->file,"%u\n",length);
  (void) fprintf(image->file,"endobj\n");
  /*
    Write Procset object.
  */
  xref[6]=ftell(image->file);
  (void) fprintf(image->file,"7 0 obj\n");
  if (!IsPseudoClass(image) && !IsGrayImage(image))
    (void) fprintf(image->file,"[ /PDF /Text /ImageC");
  else
    if (IsGrayImage(image) && (image->colors == 2))
      (void) fprintf(image->file,"[ /PDF /Text /ImageB");
    else
      (void) fprintf(image->file,"[ /PDF /Text /ImageI");
  (void) fprintf(image->file," ]\n");
  (void) fprintf(image->file,"endobj\n");
  /*
    Write Font object.
  */
  xref[7]=ftell(image->file);
  (void) fprintf(image->file,"8 0 obj\n");
  (void) fprintf(image->file,"<<\n");
  (void) fprintf(image->file,"/Type /Font\n");
  (void) fprintf(image->file,"/Subtype /Type1\n");
  (void) fprintf(image->file,"/Name /F0\n");
  (void) fprintf(image->file,"/BaseFont /Helvetica\n");
  (void) fprintf(image->file,"/Encoding /MacRomanEncoding\n");
  (void) fprintf(image->file,">>\n");
  (void) fprintf(image->file,"endobj\n");
  /*
    Write XObject object.
  */
  xref[8]=ftell(image->file);
  (void) fprintf(image->file,"9 0 obj\n");
  (void) fprintf(image->file,"<<\n");
  (void) fprintf(image->file,"/Type /XObject\n");
  (void) fprintf(image->file,"/Subtype /Image\n");
  (void) fprintf(image->file,"/Name /Im0\n");
  if (image->compression == NoCompression)
    (void) fprintf(image->file,"/Filter /ASCII85Decode\n");
  else
    (void) fprintf(image->file,"/Filter [ /ASCII85Decode /RunLengthDecode ]\n");
  (void) fprintf(image->file,"/Width %u\n",image->columns);
  (void) fprintf(image->file,"/Height %u\n",image->rows);
  (void) fprintf(image->file,"/ColorSpace 11 0 R\n");
  (void) fprintf(image->file,"/BitsPerComponent %d\n",
    IsGrayImage(image) && (image->colors == 2) ? 1 : 8);
  (void) fprintf(image->file,"/Length 10 0 R\n");
  (void) fprintf(image->file,">>\n");
  (void) fprintf(image->file,"stream\n");
  length=ftell(image->file);
  p=image->pixels;
  if (!IsPseudoClass(image) && !IsGrayImage(image))
    switch (image->compression)
    {
      case RunlengthEncodedCompression:
      default:
      {
        /*
          Allocate pixel array.
        */
        number_packets=3*image->columns*image->rows;
        pixels=(unsigned char *) malloc(number_packets*sizeof(unsigned char));
        if (pixels == (unsigned char *) NULL)
          PrematureExit("Unable to allocate memory",image);
        /*
          Dump runlength encoded pixels.
        */
        q=pixels;
        for (i=0; i < image->packets; i++)
        {
          for (j=0; j <= (int) p->length; j++)
          {
            if (image->matte && (p->index == Transparent))
              {
                *q++=DownScale(MaxRGB);
                *q++=DownScale(MaxRGB);
                *q++=DownScale(MaxRGB);
              }
            else
              {
                *q++=DownScale(p->red);
                *q++=DownScale(p->green);
                *q++=DownScale(p->blue);
              }
          }
          p++;
          if (QuantumTick(i,image))
            ProgressMonitor(SaveImageText,i,image->packets);
        }
        (void) PackbitsEncodeImage(image->file,pixels,number_packets);
        free((char *) pixels);
        break;
      }
      case NoCompression:
      {
        /*
          Dump uncompressed DirectColor packets.
        */
        Ascii85Initialize();
        for (i=0; i < image->packets; i++)
        {
          for (j=0; j <= ((int) p->length); j++)
          {
            if (image->matte && (p->index == Transparent))
              {
                Ascii85Encode(DownScale(MaxRGB),image->file);
                Ascii85Encode(DownScale(MaxRGB),image->file);
                Ascii85Encode(DownScale(MaxRGB),image->file);
              }
            else
              {
                Ascii85Encode(DownScale(p->red),image->file);
                Ascii85Encode(DownScale(p->green),image->file);
                Ascii85Encode(DownScale(p->blue),image->file);
              }
          }
          p++;
          if (QuantumTick(i,image))
            ProgressMonitor(SaveImageText,i,image->packets);
        }
        Ascii85Flush(image->file);
        break;
      }
    }
  else
    if (IsGrayImage(image) && (image->colors == 2))
      {
        register unsigned char
          bit,
          Byte,
          polarity;

        polarity=Intensity(image->colormap[0]) < Intensity(image->colormap[1]);
        bit=0;
        Byte=0;
        x=0;
        switch (image->compression)
        {
          case RunlengthEncodedCompression:
          default:
          {
            /*
              Allocate pixel array.
            */
            number_packets=((image->columns+7) >> 3)*image->rows;
            pixels=(unsigned char *)
              malloc(number_packets*sizeof(unsigned char));
            if (pixels == (unsigned char *) NULL)
              PrematureExit("Unable to allocate memory",image);
            /*
              Dump Runlength encoded pixels.
            */
            q=pixels;
            for (i=0; i < image->packets; i++)
            {
              for (j=0; j <= (int) p->length; j++)
              {
                Byte<<=1;
                if (p->index == polarity)
                  Byte|=0x01;
                bit++;
                if (bit == 8)
                  {
                    *q++=Byte;
                    bit=0;
                    Byte=0;
                  }
                x++;
                if (x == image->columns)
                  {
                    /*
                      Advance to the next scanline.
                    */
                    if (bit != 0)
                      *q++=Byte << (8-bit);
                    bit=0;
                    Byte=0;
                    x=0;
                    y++;
                 }
              }
              p++;
              if (QuantumTick(i,image))
                ProgressMonitor(SaveImageText,i,image->packets);
            }
            (void) PackbitsEncodeImage(image->file,pixels,number_packets);
            free((char *) pixels);
            break;
          }
          case NoCompression:
          {
            /*
              Dump uncompressed PseudoColor packets.
            */
            Ascii85Initialize();
            for (i=0; i < image->packets; i++)
            {
              for (j=0; j <= (int) p->length; j++)
              {
                Byte<<=1;
                if (p->index == polarity)
                  Byte|=0x01;
                bit++;
                if (bit == 8)
                  {
                    Ascii85Encode(Byte,image->file);
                    bit=0;
                    Byte=0;
                  }
                x++;
                if (x == image->columns)
                  {
                    /*
                      Advance to the next scanline.
                    */
                    if (bit != 0)
                      Ascii85Encode(Byte << (8-bit),image->file);
                    bit=0;
                    Byte=0;
                    x=0;
                    y++;
                 }
              }
              p++;
              if (QuantumTick(i,image))
                ProgressMonitor(SaveImageText,i,image->packets);
            }
            Ascii85Flush(image->file);
            break;
          }
        }
      }
    else
      {
        /*
          Dump number of colors and colormap.
        */
        switch (image->compression)
        {
          case RunlengthEncodedCompression:
          default:
          {
            /*
              Allocate pixel array.
            */
            number_packets=image->columns*image->rows;
            pixels=(unsigned char *)
              malloc(number_packets*sizeof(unsigned char));
            if (pixels == (unsigned char *) NULL)
              PrematureExit("Unable to allocate memory",image);
            /*
              Dump GIF encoded pixels.
            */
            q=pixels;
            for (i=0; i < image->packets; i++)
            {
              for (j=0; j <= (int) p->length; j++)
                *q++=(unsigned char) p->index;
              p++;
              if (QuantumTick(i,image))
                ProgressMonitor(SaveImageText,i,image->packets);
            }
            (void) PackbitsEncodeImage(image->file,pixels,number_packets);
            free((char *) pixels);
            break;
          }
          case NoCompression:
          {
            /*
              Dump uncompressed PseudoColor packets.
            */
            Ascii85Initialize();
            for (i=0; i < image->packets; i++)
            {
              for (j=0; j <= ((int) p->length); j++)
                Ascii85Encode((unsigned char) p->index,image->file);
              p++;
              if (QuantumTick(i,image))
                ProgressMonitor(SaveImageText,i,image->packets);
            }
            Ascii85Flush(image->file);
            break;
          }
        }
      }
  length=ftell(image->file)-length;
  (void) fprintf(image->file,"\nendstream\n");
  (void) fprintf(image->file,"endobj\n");
  /*
    Write Length object.
  */
  xref[9]=ftell(image->file);
  (void) fprintf(image->file,"10 0 obj\n");
  (void) fprintf(image->file,"%u\n",length);
  (void) fprintf(image->file,"endobj\n");
  /*
    Write Colorspace object.
  */
  xref[10]=ftell(image->file);
  (void) fprintf(image->file,"11 0 obj\n");
  if (!IsPseudoClass(image) && !IsGrayImage(image))
    (void) fprintf(image->file,"/DeviceRGB\n");
  else
    if (IsGrayImage(image) && (image->colors == 2))
      (void) fprintf(image->file,"/DeviceGray\n");
    else
      (void) fprintf(image->file,"[ /Indexed /DeviceRGB %u 14 0 R ]\n",
        image->colors-1);
  (void) fprintf(image->file,"endobj\n");
  /*
    Write Thumb object.
  */
  image->orphan=True;
  ParseImageGeometry(DefaultThumbnailGeometry,&width,&height);
  if (image->class == PseudoClass)
    tile_image=SampleImage(image,width,height);
  else
    tile_image=ZoomImage(image,width,height,MitchellFilter);
  image->orphan=False;
  if (tile_image == (Image *) NULL)
    PrematureExit("Unable to allocate memory",image);
  xref[11]=ftell(image->file);
  (void) fprintf(image->file,"12 0 obj\n");
  (void) fprintf(image->file,"<<\n");
  if (image->compression == NoCompression)
    (void) fprintf(image->file,"/Filter /ASCII85Decode\n");
  else
    (void) fprintf(image->file,"/Filter [ /ASCII85Decode /RunLengthDecode ]\n");
  (void) fprintf(image->file,"/Width %u\n",tile_image->columns);
  (void) fprintf(image->file,"/Height %u\n",tile_image->rows);
  (void) fprintf(image->file,"/ColorSpace 11 0 R\n");
  (void) fprintf(image->file,"/BitsPerComponent %d\n",
    IsGrayImage(tile_image) && (tile_image->colors == 2) ? 1 : 8);
  (void) fprintf(image->file,"/Length 13 0 R\n");
  (void) fprintf(image->file,">>\n");
  (void) fprintf(image->file,"stream\n");
  length=ftell(image->file);
  p=tile_image->pixels;
  if (!IsPseudoClass(tile_image))
    switch (tile_image->compression)
    {
      case RunlengthEncodedCompression:
      default:
      {
        /*
          Allocate pixel array.
        */
        number_packets=3*tile_image->columns*tile_image->rows;
        pixels=(unsigned char *) malloc(number_packets*sizeof(unsigned char));
        if (pixels == (unsigned char *) NULL)
          {
            DestroyImage(tile_image);
            PrematureExit("Unable to allocate memory",image);
          }
        /*
          Dump runlength encoded pixels.
        */
        q=pixels;
        for (i=0; i < tile_image->packets; i++)
        {
          for (j=0; j <= (int) p->length; j++)
          {
            if (tile_image->matte && (p->index == Transparent))
              {
                *q++=DownScale(MaxRGB);
                *q++=DownScale(MaxRGB);
                *q++=DownScale(MaxRGB);
              }
            else
              {
                *q++=DownScale(p->red);
                *q++=DownScale(p->green);
                *q++=DownScale(p->blue);
              }
          }
          p++;
        }
        (void) PackbitsEncodeImage(image->file,pixels,number_packets);
        free((char *) pixels);
        break;
      }
      case NoCompression:
      {
        /*
          Dump uncompressed DirectColor packets.
        */
        Ascii85Initialize();
        for (i=0; i < tile_image->packets; i++)
        {
          for (j=0; j <= ((int) p->length); j++)
          {
            if (tile_image->matte && (p->index == Transparent))
              {
                Ascii85Encode(DownScale(MaxRGB),image->file);
                Ascii85Encode(DownScale(MaxRGB),image->file);
                Ascii85Encode(DownScale(MaxRGB),image->file);
              }
            else
              {
                Ascii85Encode(DownScale(p->red),image->file);
                Ascii85Encode(DownScale(p->green),image->file);
                Ascii85Encode(DownScale(p->blue),image->file);
              }
          }
          p++;
        }
        Ascii85Flush(image->file);
        break;
      }
    }
  else
    if (IsGrayImage(tile_image) && (tile_image->colors == 2))
      {
        register unsigned char
          bit,
          Byte,
          polarity;

        polarity=Intensity(tile_image->colormap[0]) <
          Intensity(tile_image->colormap[1]);
        bit=0;
        Byte=0;
        x=0;
        switch (tile_image->compression)
        {
          case RunlengthEncodedCompression:
          default:
          {
            /*
              Allocate pixel array.
            */
            number_packets=((tile_image->columns+7) >> 3)*tile_image->rows;
            pixels=(unsigned char *)
              malloc(number_packets*sizeof(unsigned char));
            if (pixels == (unsigned char *) NULL)
              {
                DestroyImage(tile_image);
                PrematureExit("Unable to allocate memory",image);
              }
            /*
              Dump GIF encoded pixels.
            */
            q=pixels;
            for (i=0; i < tile_image->packets; i++)
            {
              for (j=0; j <= (int) p->length; j++)
              {
                Byte<<=1;
                if (p->index == polarity)
                  Byte|=0x01;
                bit++;
                if (bit == 8)
                  {
                    *q++=Byte;
                    bit=0;
                    Byte=0;
                  }
                x++;
                if (x == tile_image->columns)
                  {
                    /*
                      Advance to the next scanline.
                    */
                    if (bit != 0)
                      *q++=Byte << (8-bit);
                    bit=0;
                    Byte=0;
                    x=0;
                    y++;
                 }
              }
              p++;
            }
            (void) PackbitsEncodeImage(image->file,pixels,number_packets);
            free((char *) pixels);
            break;
          }
          case NoCompression:
          {
            /*
              Dump uncompressed PseudoColor packets.
            */
            Ascii85Initialize();
            for (i=0; i < tile_image->packets; i++)
            {
              for (j=0; j <= (int) p->length; j++)
              {
                Byte<<=1;
                if (p->index == polarity)
                  Byte|=0x01;
                bit++;
                if (bit == 8)
                  {
                    Ascii85Encode(Byte,image->file);
                    bit=0;
                    Byte=0;
                  }
                x++;
                if (x == tile_image->columns)
                  {
                    /*
                      Advance to the next scanline.
                    */
                    if (bit != 0)
                      Ascii85Encode(Byte << (8-bit),image->file);
                    bit=0;
                    Byte=0;
                    x=0;
                    y++;
                 }
              }
              p++;
            }
            Ascii85Flush(image->file);
            break;
          }
        }
      }
    else
      {
        /*
          Dump number of colors and colormap.
        */
        switch (tile_image->compression)
        {
          case RunlengthEncodedCompression:
          default:
          {
            /*
              Allocate pixel array.
            */
            number_packets=tile_image->columns*tile_image->rows;
            pixels=(unsigned char *)
              malloc(number_packets*sizeof(unsigned char));
            if (pixels == (unsigned char *) NULL)
              {
                DestroyImage(tile_image);
                PrematureExit("Unable to allocate memory",image);
              }
            /*
              Dump GIF encoded pixels.
            */
            q=pixels;
            for (i=0; i < tile_image->packets; i++)
            {
              for (j=0; j <= (int) p->length; j++)
                *q++=(unsigned char) p->index;
              p++;
            }
            (void) PackbitsEncodeImage(image->file,pixels,number_packets);
            free((char *) pixels);
            break;
          }
          case NoCompression:
          {
            /*
              Dump uncompressed PseudoColor packets.
            */
            Ascii85Initialize();
            for (i=0; i < tile_image->packets; i++)
            {
              for (j=0; j <= ((int) p->length); j++)
                Ascii85Encode((unsigned char) p->index,image->file);
              p++;
            }
            Ascii85Flush(image->file);
            break;
          }
        }
      }
  DestroyImage(tile_image);
  length=ftell(image->file)-length;
  (void) fprintf(image->file,"\nendstream\n");
  (void) fprintf(image->file,"endobj\n");
  /*
    Write Length object.
  */
  xref[12]=ftell(image->file);
  (void) fprintf(image->file,"13 0 obj\n");
  (void) fprintf(image->file,"%u\n",length);
  (void) fprintf(image->file,"endobj\n");
  number_objects=13;
  if (image->class == PseudoClass)
    if (!IsGrayImage(image) || (image->colors != 2))
      {
        /*
          Write Colormap object.
        */
        number_objects+=2;
        xref[13]=ftell(image->file);
        (void) fprintf(image->file,"14 0 obj\n");
        (void) fprintf(image->file,"<<\n");
        (void) fprintf(image->file,"/Filter /ASCII85Decode \n");
        (void) fprintf(image->file,"/Length 15 0 R\n");
        (void) fprintf(image->file,">>\n");
        (void) fprintf(image->file,"stream\n");
        length=ftell(image->file);
        Ascii85Initialize();
        for (i=0; i < image->colors; i++)
        {
          Ascii85Encode(DownScale(image->colormap[i].red),image->file);
          Ascii85Encode(DownScale(image->colormap[i].green),image->file);
          Ascii85Encode(DownScale(image->colormap[i].blue),image->file);
        }
        Ascii85Flush(image->file);
        length=ftell(image->file)-length;
        (void) fprintf(image->file,"\nendstream\n");
        (void) fprintf(image->file,"endobj\n");
        /*
          Write Length object.
        */
        xref[14]=ftell(image->file);
        (void) fprintf(image->file,"15 0 obj\n");
        (void) fprintf(image->file,"%u\n",length);
        (void) fprintf(image->file,"endobj\n");
      }
  /*
    Write Xref object.
  */
  length=ftell(image->file)-xref[0]+10;
  (void) fprintf(image->file,"xref\n");
  (void) fprintf(image->file,"0 %u\n",number_objects+1);
  (void) fprintf(image->file,"0000000000 65535 f \n");
  for (i=0; i < number_objects; i++)
    (void) fprintf(image->file,"%010d 00000 n \n",xref[i]);
  (void) fprintf(image->file,"trailer\n");
  (void) fprintf(image->file,"<<\n");
  (void) fprintf(image->file,"/Size %u\n",number_objects+1);
  (void) fprintf(image->file,"/Info 1 0 R\n");
  (void) fprintf(image->file,"/Root 2 0 R\n");
  (void) fprintf(image->file,">>\n");
  (void) fprintf(image->file,"startxref\n");
  (void) fprintf(image->file,"%u\n",length);
  (void) fprintf(image->file,"%%%%EOF\n");
  CloseImage(image);
  if (image->temporary)
    {
      FILE
        *file;

      int
        c;

      /*
        Copy temporary file to standard output or pipe.
      */
      file=fopen(image->filename,ReadBinaryType);
      if (file == (FILE *) NULL)
        PrematureExit("Unable to open file",image);
      c=fgetc(file);
      while (c != EOF)
      {
        (void) putc(c,encode_image.file);
        c=fgetc(file);
      }
      (void) fclose(file);
      (void) unlink(image->filename);
      CloseImage(&encode_image);
    }
  return(True);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   W r i t e P I C T I m a g e                                               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Function WritePICTImage writes an image to a file in the Apple Macintosh
%  QuickDraw/PICT image format.
%
%  The format of the WritePICTImage routine is:
%
%      status=WritePICTImage(image_info,image)
%
%  A description of each parameter follows.
%
%    o status: Function WritePICTImage return True if the image is written.
%      False is returned is there is a memory shortage or if the image file
%      fails to write.
%
%    o image_info: Specifies a pointer to an ImageInfo structure.
%
%    o image:  A pointer to a Image structure.
%
%
*/
static unsigned int WritePICTImage(ImageInfo *image_info, Image *image)
{
#define MaxCount  128
#define PictCropRegionOp  0x01
#define PictEndOfPictureOp  0xff
#define PictHeaderOp  0x0C00
#define PictHeaderSize  512
#define PictPixmapOp  0x9A
#define PictPICTOp  0x98
#define PictVersion  0x11

  typedef struct _PICTRectangle
  {
    unsigned short
      top,
      left,
      bottom,
      right;
  } PICTRectangle;

  typedef struct _PICTPixmap
  {
    unsigned short
      base_address,
      row_bytes;

    PICTRectangle
      bounds;

    unsigned short
      version,
      pack_type;

    unsigned long
      pack_size,
      horizontal_resolution,
      vertical_resolution;

    unsigned short
      pixel_type,
      pixel_size,
      component_count,
      component_size;

    unsigned short
      plane_bytes,
      table,
      reserved;
  } PICTPixmap;

  int
    count;

  PICTPixmap
    pixmap;

  PICTRectangle
    crop_rectangle,
    destination_rectangle,
    frame_rectangle,
    size_rectangle,
    source_rectangle;

  register int
    i,
    j,
    x;

  register RunlengthPacket
    *p;

  unsigned char
    *buffer,
    *packed_scanline,
    *scanline;

  unsigned int
    bytes_per_line;

  unsigned short
    transfer_mode;

  unsigned long
    horizontal_resolution,
    vertical_resolution;

  /*
    Open output image file.
  */
  OpenImage(image,WriteBinaryType);
  if (image->file == (FILE *) NULL)
    PrematureExit("Unable to open file",image);
  /*
    Allocate memory.
  */
  bytes_per_line=image->columns;
  if (image->class == DirectClass)
    bytes_per_line*=3;
  buffer=(unsigned char *) malloc(PictHeaderSize*sizeof(unsigned char));
  packed_scanline=(unsigned char *)
    malloc((bytes_per_line+bytes_per_line/MaxCount+1)*sizeof(unsigned char));
  scanline=(unsigned char *) malloc(bytes_per_line*sizeof(unsigned char));
  if ((buffer == (unsigned char *) NULL) ||
      (packed_scanline == (unsigned char *) NULL) ||
      (scanline == (unsigned char *) NULL))
    PrematureExit("Unable to allocate memory",image);
  /*
    Initialize image info.
  */
  size_rectangle.top=0;
  size_rectangle.left=0;
  size_rectangle.right=image->rows;
  size_rectangle.bottom=image->columns;
  frame_rectangle=size_rectangle;
  crop_rectangle=size_rectangle;
  source_rectangle=size_rectangle;
  destination_rectangle=size_rectangle;
  horizontal_resolution=0x00480000;
  vertical_resolution=0x00480000;
  pixmap.base_address=0xff;
  pixmap.row_bytes=
    ((image->class == DirectClass ? 4 : 1)*image->columns) | 0x8000;
  pixmap.version=0;
  pixmap.bounds.top=0;
  pixmap.bounds.left=0;
  pixmap.bounds.right=image->rows;
  pixmap.bounds.bottom=image->columns;
  pixmap.pack_type=(image->class == DirectClass ? 0x4 : 0x0);
  pixmap.pack_size=0;
  pixmap.horizontal_resolution=horizontal_resolution;
  pixmap.vertical_resolution=vertical_resolution;
  pixmap.pixel_type=(image->class == DirectClass ? 16 : 0);
  pixmap.pixel_size=(image->class == DirectClass ? 32 : 8);
  pixmap.component_count=(image->class == DirectClass ? 3 : 1);
  pixmap.component_size=8;
  pixmap.plane_bytes=0;
  pixmap.table=0;
  pixmap.reserved=0;
  transfer_mode=(image->class == DirectClass ? 0x40 : 0);
  /*
    Write header, header size, size bounding box, version, and reserved.
  */
  for (i=0; i < PictHeaderSize; i++)
    buffer[i]=0;
  (void) fwrite((char *) buffer,1,PictHeaderSize,image->file);
  MSBFirstWriteShort(0,image->file);
  MSBFirstWriteShort(size_rectangle.top,image->file);
  MSBFirstWriteShort(size_rectangle.left,image->file);
  MSBFirstWriteShort(size_rectangle.right,image->file);
  MSBFirstWriteShort(size_rectangle.bottom,image->file);
  MSBFirstWriteShort(PictVersion,image->file);
  MSBFirstWriteShort(0x02FF,image->file);
  MSBFirstWriteShort(PictHeaderOp,image->file);
  /*
    Write full size of the file, resolution, frame bounding box, and reserved.
  */
  MSBFirstWriteLong(0xFFFE0000L,image->file);
  MSBFirstWriteLong(horizontal_resolution,image->file);
  MSBFirstWriteLong(vertical_resolution,image->file);
  MSBFirstWriteShort(frame_rectangle.top,image->file);
  MSBFirstWriteShort(frame_rectangle.left,image->file);
  MSBFirstWriteShort(frame_rectangle.right,image->file);
  MSBFirstWriteShort(frame_rectangle.bottom,image->file);
  MSBFirstWriteLong(0L,image->file);
  /*
    Write crop region opcode and crop bounding box.
  */
  MSBFirstWriteShort(PictCropRegionOp,image->file);
  MSBFirstWriteShort(0xA,image->file);
  MSBFirstWriteShort(crop_rectangle.top,image->file);
  MSBFirstWriteShort(crop_rectangle.left,image->file);
  MSBFirstWriteShort(crop_rectangle.right,image->file);
  MSBFirstWriteShort(crop_rectangle.bottom,image->file);
  /*
    Write picture opcode, row bytes, and picture bounding box, and version.
  */
  if (image->class == PseudoClass)
    MSBFirstWriteShort(PictPICTOp,image->file);
  else
    {
      MSBFirstWriteShort(PictPixmapOp,image->file);
      MSBFirstWriteLong((unsigned long) pixmap.base_address,image->file);
    }
  MSBFirstWriteShort(pixmap.row_bytes | 0x8000,image->file);
  MSBFirstWriteShort(pixmap.bounds.top,image->file);
  MSBFirstWriteShort(pixmap.bounds.left,image->file);
  MSBFirstWriteShort(pixmap.bounds.right,image->file);
  MSBFirstWriteShort(pixmap.bounds.bottom,image->file);
  MSBFirstWriteShort(pixmap.version,image->file);
  /*
    Write pack type, pack size, resolution, pixel type, and pixel size.
  */
  MSBFirstWriteShort(pixmap.pack_type,image->file);
  MSBFirstWriteLong(pixmap.pack_size,image->file);
  MSBFirstWriteLong(pixmap.horizontal_resolution,image->file);
  MSBFirstWriteLong(pixmap.vertical_resolution,image->file);
  MSBFirstWriteShort(pixmap.pixel_type,image->file);
  MSBFirstWriteShort(pixmap.pixel_size,image->file);
  /*
    Write component count, size, plane bytes, table size, and reserved.
  */
  MSBFirstWriteShort(pixmap.component_count,image->file);
  MSBFirstWriteShort(pixmap.component_size,image->file);
  MSBFirstWriteLong((unsigned long) pixmap.plane_bytes,image->file);
  MSBFirstWriteLong((unsigned long) pixmap.table,image->file);
  MSBFirstWriteLong((unsigned long) pixmap.reserved,image->file);
  if (image->class == PseudoClass)
    {
      unsigned short
        red,
        green,
        blue;

      /*
        Write image colormap.
      */
      MSBFirstWriteLong(0L,image->file);  /* color seed */
      MSBFirstWriteShort(0L,image->file);  /* color flags */
      MSBFirstWriteShort((unsigned short) (image->colors-1),image->file);
      for (i=0; i < image->colors; i++)
      {
        red=(unsigned int) (image->colormap[i].red*65535L)/MaxRGB;
        green=(unsigned int) (image->colormap[i].green*65535L)/MaxRGB;
        blue=(unsigned int) (image->colormap[i].blue*65535L)/MaxRGB;
        MSBFirstWriteShort((unsigned int) i,image->file);
        MSBFirstWriteShort(red,image->file);
        MSBFirstWriteShort(green,image->file);
        MSBFirstWriteShort(blue,image->file);
      }
    }
  /*
    Write source and destination rectangle.
  */
  MSBFirstWriteShort(source_rectangle.top,image->file);
  MSBFirstWriteShort(source_rectangle.left,image->file);
  MSBFirstWriteShort(source_rectangle.right,image->file);
  MSBFirstWriteShort(source_rectangle.bottom,image->file);
  MSBFirstWriteShort(destination_rectangle.top,image->file);
  MSBFirstWriteShort(destination_rectangle.left,image->file);
  MSBFirstWriteShort(destination_rectangle.right,image->file);
  MSBFirstWriteShort(destination_rectangle.bottom,image->file);
  MSBFirstWriteShort(transfer_mode,image->file);
  /*
    Write picture data.
  */
  count=0;
  x=0;
  p=image->pixels;
  if (image->class == PseudoClass)
    {
      register unsigned char
        *index;

      index=scanline;
      for (i=0; i < image->packets; i++)
      {
        for (j=0; j <= (int) p->length; j++)
        {
          *index++=(unsigned char) p->index;
          x++;
          if (x == image->columns)
            {
              count+=PICTEncodeImage(image,scanline,packed_scanline);
              index=scanline;
              x=0;
            }
        }
        p++;
        if (QuantumTick(i,image))
          ProgressMonitor(SaveImageText,i,image->packets);
      }
    }
  else
    {
      register unsigned char
        *blue,
        *red,
        *green;

      red=scanline;
      green=scanline+image->columns;
      blue=scanline+(image->columns << 1);
      for (i=0; i < image->packets; i++)
      {
        for (j=0; j <= (int) p->length; j++)
        {
          *red++=DownScale(p->red);
          *green++=DownScale(p->green);
          *blue++=DownScale(p->blue);
          x++;
          if (x == image->columns)
            {
              red=scanline;
              green=scanline+image->columns;
              blue=scanline+(image->columns << 1);
              count+=PICTEncodeImage(image,red,packed_scanline);
              x=0;
            }
        }
        p++;
        if (QuantumTick(i,image))
          ProgressMonitor(SaveImageText,i,image->packets);
      }
    }
  if (count & 0x1)
    (void) fputc('\0',image->file);
  MSBFirstWriteShort(PictEndOfPictureOp,image->file);
  free((char *) scanline);
  free((char *) packed_scanline);
  free((char *) buffer);
  CloseImage(image);
  return(True);
}

#ifdef HasPNG
#include "png.h"
/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   W r i t e P N G I m a g e                                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Function WritePNGImage writes an image in the Portable Network Graphics
%  encoded image format.
%
%  The format of the WritePNGImage routine is:
%
%      status=WritePNGImage(image_info,image)
%
%  A description of each parameter follows.
%
%    o status: Function WritePNGImage return True if the image is written.
%      False is returned is there is a memory shortage or if the image file
%      fails to write.
%
%    o image_info: Specifies a pointer to an ImageInfo structure.
%
%    o image:  A pointer to a Image structure.
%
%
*/
static unsigned int WritePNGImage(image_info,image)
ImageInfo
  *image_info;

Image
  *image;
{
  int
    length;

  register int
    i,
    j;

  register RunlengthPacket
    *p;

  register unsigned char
    *q;

  png_info
    *ping_info;

  png_struct
    *ping;

  unsigned char
    *png_pixels,
    **scanlines;

  unsigned int
    bytes_per_line,
    x,
    y;

  unsigned short
    value;

  /*
    Open image file.
  */
  OpenImage(image,WriteBinaryType);
  if (image->file == (FILE *) NULL)
    PrematureExit("Unable to open file",image);
  /*
    Allocate the PNG structures
  */
  ping=(png_struct *) malloc(sizeof(png_struct));
  ping_info=(png_info *) malloc(sizeof(png_info));
  if ((ping == (png_struct *) NULL) || (ping_info == (png_info *) NULL))
    PrematureExit("Unable to allocate memory",image);
  /*
    Prepare PNG for writing.
  */
  png_info_init(ping_info);
  png_write_init(ping);
  png_init_io(ping,image->file);
  ping_info->width=image->columns;
  ping_info->height=image->rows;
  ping_info->bit_depth=8;
  if ((image->class == DirectClass) && (image->depth == 16))
    ping_info->bit_depth=16;
  ping_info->color_type=PNG_COLOR_TYPE_RGB;
  if (image->matte)
    ping_info->color_type=PNG_COLOR_TYPE_RGB_ALPHA;
  ping_info->bit_depth=image->depth;
  ping_info->num_palette=0;
  if (IsGrayImage(image))
    ping_info->color_type=PNG_COLOR_TYPE_GRAY;
  else
    if ((image->class == PseudoClass) && (image->colors <= 256))
      {
        /*
          Set image palette.
        */
        ping_info->color_type=PNG_COLOR_TYPE_PALETTE;
        ping_info->num_palette=image->colors;
        ping_info->valid|=PNG_INFO_PLTE;
        ping_info->palette=(png_color *)
          malloc(image->columns*sizeof(png_color));
        if (ping_info->palette == (png_color *) NULL)
          PrematureExit("Unable to allocate memory",image);
        for (i=0; i < image->colors; i++)
        {
          ping_info->palette[i].red=DownScale(image->colormap[i].red);
          ping_info->palette[i].green=DownScale(image->colormap[i].green);
          ping_info->palette[i].blue=DownScale(image->colormap[i].blue);
        }
        ping_info->bit_depth=1;
        while ((1 << ping_info->bit_depth) < image->colors)
          ping_info->bit_depth<<=1;
      }
  if (image->gamma != 0.0)
    {
      /*
        Note image gamma.
      */
      ping_info->valid|=PNG_INFO_gAMA;
      ping_info->gamma=image->gamma;
    }
  ping_info->interlace_type=image_info->interlace != NoneInterlace;
  png_write_info(ping,ping_info);
  png_set_packing(ping);
  /*
    Allocate memory.
  */
  bytes_per_line=
    Max((int) ping_info->bit_depth >> 3,1)*ping->channels*image->columns;
  png_pixels=(unsigned char *)
    malloc(bytes_per_line*image->rows*sizeof(unsigned char));
  scanlines=(unsigned char **) malloc(image->rows*sizeof(unsigned char *));
  if ((png_pixels == (unsigned char *) NULL) ||
      (scanlines == (unsigned char **) NULL))
    PrematureExit("Unable to allocate memory",image);
  /*
    Initialize image scanlines.
  */
  for (i=0; i < image->rows; i++)
    scanlines[i]=png_pixels+(bytes_per_line*i);
  x=0;
  y=0;
  p=image->pixels;
  q=scanlines[y];
  if (IsGrayImage(image))
    for (i=0; i < image->packets; i++)
    {
      for (j=0; j <= ((int) p->length); j++)
      {
        WriteQuantum(Intensity(*p),q);
        x++;
        if (x == image->columns)
          {
            x=0;
            y++;
            q=scanlines[y];
          }
      }
      p++;
      if (QuantumTick(i,image))
        ProgressMonitor(SaveImageText,i,image->packets);
    }
  else
    if ((image->class == DirectClass) || (image->colors > 256))
      for (i=0; i < image->packets; i++)
      {
        for (j=0; j <= ((int) p->length); j++)
        {
          WriteQuantum(p->red,q);
          WriteQuantum(p->green,q);
          WriteQuantum(p->blue,q);
          if (image->matte)
            WriteQuantum(p->index,q);
          x++;
          if (x == image->columns)
            {
              x=0;
              y++;
              q=scanlines[y];
            }
        }
        p++;
        if (QuantumTick(i,image))
          ProgressMonitor(SaveImageText,i,image->packets);
      }
    else
      for (i=0; i < image->packets; i++)
      {
        for (j=0; j <= ((int) p->length); j++)
        {
          *q++=p->index;
          x++;
          if (x == image->columns)
            {
              x=0;
              y++;
              q=scanlines[y];
            }
        }
        p++;
        if (QuantumTick(i,image))
          ProgressMonitor(SaveImageText,i,image->packets);
      }
  /*
    Write image scanlines.
  */
  png_write_image(ping,scanlines);
  if (image->comments != (char *) NULL)
    {
      /*
        Write image comment.
      */
      length=strlen(image->comments);
      if (length < MaxTextLength)
        (void) png_write_tEXt(ping,"Comment",image->comments,length);
      else
        (void) png_write_zTXt(ping,"Comment",image->comments,length,0);
    }
  png_write_end(ping,ping_info);
  /*
    Free memory.
  */
  png_write_destroy(ping);
  if (ping->palette)
    free(ping->palette);
  free(ping);
  free(ping_info);
  CloseImage(image);
  return(True);
}
#else
static unsigned int WritePNGImage(ImageInfo *image_info, Image *image)
{
  unsigned int
    status;

  Warning("PNG library is not available",image->filename);
  status=WriteMIFFImage(image_info,image);
  return(status);
}
#endif

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   W r i t e P N M I m a g e                                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Procedure WritePNMImage writes an image to a file in the PNM rasterfile
%  format.
%
%  The format of the WritePNMImage routine is:
%
%      status=WritePNMImage(image_info,image)
%
%  A description of each parameter follows.
%
%    o status: Function WritePNMImage return True if the image is written.
%      False is returned is there is a memory shortage or if the image file
%      fails to write.
%
%    o image_info: Specifies a pointer to an ImageInfo structure.
%
%    o image:  A pointer to a Image structure.
%
%
*/
static unsigned int WritePNMImage(ImageInfo *image_info, Image *image)
{
#define MaxRawValue  255

  register int
    i,
    j;

  register RunlengthPacket
    *p;

  unsigned char
    format;

  /*
    Open output image file.
  */
  OpenImage(image,WriteBinaryType);
  if (image->file == (FILE *) NULL)
    PrematureExit("Unable to open file",image);
  /*
    Promote/Demote image based on image type.
  */
  if (strcmp(image_info->magick,"PPM") == 0)
    image->class=DirectClass;
  else
    if (strcmp(image_info->magick,"PGM") == 0)
      RGBTransformImage(image,GRAYColorspace);
    else
      if (strcmp(image_info->magick,"PBM") == 0)
        if (!IsGrayImage(image) || (image->colors != 2))
          {
            QuantizeImage(image,2,8,image_info->dither,GRAYColorspace);
            SyncImage(image);
          }
  /*
    Write PNM file header.
  */
  if (!IsPseudoClass(image) && !IsGrayImage(image))
    {
      /*
        Full color PNM image.
      */
      format='6';
      if (MaxRGB > MaxRawValue)
        format='3';
    }
  else
    {
      /*
        Colormapped PNM image.
      */
      format='6';
      if (MaxRGB > MaxRawValue)
        format='3';
      if (IsGrayImage(image) && (strcmp(image_info->magick,"PPM") != 0))
        {
          /*
            Grayscale PNM image.
          */
          format='5';
          if (MaxRGB > MaxRawValue)
            format='2';
          if (strcmp(image_info->magick,"PGM") != 0)
            if (image->colors == 2)
              format='4';
        }
    }
  (void) fprintf(image->file,"P%c\n",format);
  if (image->comments != (char *) NULL)
    {
      register char
        *p;

      /*
        Write comments to file.
      */
      (void) fprintf(image->file,"# ");
      for (p=image->comments; *p != '\0'; p++)
      {
        (void) fputc(*p,image->file);
        if (*p == '\n')
          (void) fprintf(image->file,"# ");
      }
      (void) fputc('\n',image->file);
    }
  (void) fprintf(image->file,"%u %u\n",image->columns,image->rows);
  /*
    Convert runlength encoded to PNM raster pixels.
  */
  p=image->pixels;
  switch (format)
  {
    case '2':
    {
      register int
        x;

      /*
        Convert image to a PGM image.
      */
      (void) fprintf(image->file,"%d\n",MaxRGB);
      x=0;
      for (i=0; i < image->packets; i++)
      {
        for (j=0; j <= ((int) p->length); j++)
        {
          (void) fprintf(image->file,"%5d ",p->red);
          x++;
          if (x == 12)
            {
              (void) fprintf(image->file,"\n");
              x=0;
            }
        }
        p++;
        if (QuantumTick(i,image))
          ProgressMonitor(SaveImageText,i,image->packets);
      }
      break;
    }
    case '3':
    {
      register int
        x;

      /*
        Convert image to a PNM image.
      */
      (void) fprintf(image->file,"%d\n",MaxRGB);
      x=0;
      for (i=0; i < image->packets; i++)
      {
        for (j=0; j <= ((int) p->length); j++)
        {
          (void) fprintf(image->file,"%5d %5d %5d ",p->red,p->green,p->blue);
          x++;
          if (x == 4)
            {
              (void) fprintf(image->file,"\n");
              x=0;
            }
        }
        p++;
        if (QuantumTick(i,image))
          ProgressMonitor(SaveImageText,i,image->packets);
      }
      break;
    }
    case '4':
    {
      register unsigned char
        bit,
        Byte,
        polarity;

      unsigned int
        x;

      /*
        Convert image to a PBM image.
      */
      polarity=0;
      if (image->colors == 2)
        polarity=Intensity(image->colormap[0]) > Intensity(image->colormap[1]);
      bit=0;
      Byte=0;
      x=0;
      for (i=0; i < image->packets; i++)
      {
        for (j=0; j <= (int) p->length; j++)
        {
          Byte<<=1;
          if (p->index == polarity)
            Byte|=0x01;
          bit++;
          if (bit == 8)
            {
              (void) fputc(Byte,image->file);
              bit=0;
              Byte=0;
            }
          x++;
          if (x == image->columns)
            {
              /*
                Advance to the next scanline.
              */
              if (bit != 0)
                (void) fputc(Byte << (8-bit),image->file);
              bit=0;
              Byte=0;
              x=0;
           }
        }
        p++;
        if (QuantumTick(i,image))
          ProgressMonitor(SaveImageText,i,image->packets);
      }
      break;
    }
    case '5':
    {
      /*
        Convert image to a PGM image.
      */
      (void) fprintf(image->file,"%u\n",DownScale(MaxRGB));
      for (i=0; i < image->packets; i++)
      {
        for (j=0; j <= ((int) p->length); j++)
          (void) fputc(DownScale(p->red),image->file);
        p++;
        if (QuantumTick(i,image))
          ProgressMonitor(SaveImageText,i,image->packets);
      }
      break;
    }
    case '6':
    {
      /*
        Convert image to a PNM image.
      */
      (void) fprintf(image->file,"%u\n",DownScale(MaxRGB));
      for (i=0; i < image->packets; i++)
      {
        for (j=0; j <= ((int) p->length); j++)
        {
          (void) fputc(DownScale(p->red),image->file);
          (void) fputc(DownScale(p->green),image->file);
          (void) fputc(DownScale(p->blue),image->file);
        }
        p++;
        if (QuantumTick(i,image))
          ProgressMonitor(SaveImageText,i,image->packets);
      }
      break;
    }
  }
  CloseImage(image);
  return(True);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   W r i t e P S I m a g e                                                   %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Function WritePSImage translates an image to encapsulated Postscript
%  Level I for printing.  If the supplied geometry is null, the image is
%  centered on the Postscript page.  Otherwise, the image is positioned as
%  specified by the geometry.
%
%  The format of the WritePSImage routine is:
%
%      status=WritePSImage(image_info,image)
%
%  A description of each parameter follows:
%
%    o status: Function WritePSImage return True if the image is printed.
%      False is returned if the image file cannot be opened for printing.
%
%    o image_info: Specifies a pointer to an ImageInfo structure.
%
%    o image: The address of a structure of type Image;  returned from
%      ReadImage.
%
%
*/
static unsigned int WritePSImage(ImageInfo *image_info, Image *image)
{
  static char
    *Postscript[]=
    {
      "%%BeginProlog",
      "%",
      "% Display a color image.  The image is displayed in color on",
      "% Postscript viewers or printers that support color, otherwise",
      "% it is displayed as grayscale.",
      "%",
      "/buffer 512 string def",
      "/Byte 1 string def",
      "/color_packet 3 string def",
      "/pixels 768 string def",
      "",
      "/DirectClassPacket",
      "{",
      "  %",
      "  % Get a DirectClass packet.",
      "  %",
      "  % Parameters:",
      "  %   red.",
      "  %   green.",
      "  %   blue.",
      "  %   length: number of pixels minus one of this color (optional).",
      "  %",
      "  currentfile color_packet readhexstring pop pop",
      "  compression 0 gt",
      "  {",
      "    /number_pixels 3 def",
      "  }",
      "  {",
      "    currentfile Byte readhexstring pop 0 get",
      "    /number_pixels exch 1 add 3 mul def",
      "  } ifelse",
      "  0 3 number_pixels 1 sub",
      "  {",
      "    pixels exch color_packet putinterval",
      "  } for",
      "  pixels 0 number_pixels getinterval",
      "} bind def",
      "",
      "/DirectClassImage",
      "{",
      "  %",
      "  % Display a DirectClass image.",
      "  %",
      "  systemdict /colorimage known",
      "  {",
      "    columns rows 8",
      "    [",
      "      columns 0 0",
      "      rows neg 0 rows",
      "    ]",
      "    { DirectClassPacket } false 3 colorimage",
      "  }",
      "  {",
      "    %",
      "    % No colorimage operator;  convert to grayscale.",
      "    %",
      "    columns rows 8",
      "    [",
      "      columns 0 0",
      "      rows neg 0 rows",
      "    ]",
      "    { GrayDirectClassPacket } image",
      "  } ifelse",
      "} bind def",
      "",
      "/GrayDirectClassPacket",
      "{",
      "  %",
      "  % Get a DirectClass packet;  convert to grayscale.",
      "  %",
      "  % Parameters:",
      "  %   red",
      "  %   green",
      "  %   blue",
      "  %   length: number of pixels minus one of this color (optional).",
      "  %",
      "  currentfile color_packet readhexstring pop pop",
      "  color_packet 0 get 0.299 mul",
      "  color_packet 1 get 0.587 mul add",
      "  color_packet 2 get 0.114 mul add",
      "  cvi",
      "  /gray_packet exch def",
      "  compression 0 gt",
      "  {",
      "    /number_pixels 1 def",
      "  }",
      "  {",
      "    currentfile Byte readhexstring pop 0 get",
      "    /number_pixels exch 1 add def",
      "  } ifelse",
      "  0 1 number_pixels 1 sub",
      "  {",
      "    pixels exch gray_packet put",
      "  } for",
      "  pixels 0 number_pixels getinterval",
      "} bind def",
      "",
      "/GrayPseudoClassPacket",
      "{",
      "  %",
      "  % Get a PseudoClass packet;  convert to grayscale.",
      "  %",
      "  % Parameters:",
      "  %   index: index into the colormap.",
      "  %   length: number of pixels minus one of this color (optional).",
      "  %",
      "  currentfile Byte readhexstring pop 0 get",
      "  /offset exch 3 mul def",
      "  /color_packet colormap offset 3 getinterval def",
      "  color_packet 0 get 0.299 mul",
      "  color_packet 1 get 0.587 mul add",
      "  color_packet 2 get 0.114 mul add",
      "  cvi",
      "  /gray_packet exch def",
      "  compression 0 gt",
      "  {",
      "    /number_pixels 1 def",
      "  }",
      "  {",
      "    currentfile Byte readhexstring pop 0 get",
      "    /number_pixels exch 1 add def",
      "  } ifelse",
      "  0 1 number_pixels 1 sub",
      "  {",
      "    pixels exch gray_packet put",
      "  } for",
      "  pixels 0 number_pixels getinterval",
      "} bind def",
      "",
      "/PseudoClassPacket",
      "{",
      "  %",
      "  % Get a PseudoClass packet.",
      "  %",
      "  % Parameters:",
      "  %   index: index into the colormap.",
      "  %   length: number of pixels minus one of this color (optional).",
      "  %",
      "  currentfile Byte readhexstring pop 0 get",
      "  /offset exch 3 mul def",
      "  /color_packet colormap offset 3 getinterval def",
      "  compression 0 gt",
      "  {",
      "    /number_pixels 3 def",
      "  }",
      "  {",
      "    currentfile Byte readhexstring pop 0 get",
      "    /number_pixels exch 1 add 3 mul def",
      "  } ifelse",
      "  0 3 number_pixels 1 sub",
      "  {",
      "    pixels exch color_packet putinterval",
      "  } for",
      "  pixels 0 number_pixels getinterval",
      "} bind def",
      "",
      "/PseudoClassImage",
      "{",
      "  %",
      "  % Display a PseudoClass image.",
      "  %",
      "  % Parameters:",
      "  %   class: 0-PseudoClass or 1-Grayscale.",
      "  %",
      "  currentfile buffer readline pop",
      "  token pop /class exch def pop",
      "  class 0 gt",
      "  {",
      "    currentfile buffer readline pop",
      "    token pop /depth exch def pop",
      "    /grays columns string def",
      "    columns rows depth",
      "    [",
      "      columns 0 0",
      "      rows neg 0 rows",
      "    ]",
      "    { currentfile grays readhexstring pop } image",
      "  }",
      "  {",
      "    %",
      "    % Parameters:",
      "    %   colors: number of colors in the colormap.",
      "    %   colormap: red, green, blue color packets.",
      "    %",
      "    currentfile buffer readline pop",
      "    token pop /colors exch def pop",
      "    /colors colors 3 mul def",
      "    /colormap colors string def",
      "    currentfile colormap readhexstring pop pop",
      "    systemdict /colorimage known",
      "    {",
      "      columns rows 8",
      "      [",
      "        columns 0 0",
      "        rows neg 0 rows",
      "      ]",
      "      { PseudoClassPacket } false 3 colorimage",
      "    }",
      "    {",
      "      %",
      "      % No colorimage operator;  convert to grayscale.",
      "      %",
      "      columns rows 8",
      "      [",
      "        columns 0 0",
      "        rows neg 0 rows",
      "      ]",
      "      { GrayPseudoClassPacket } image",
      "    } ifelse",
      "  } ifelse",
      "} bind def",
      "",
      "/DisplayImage",
      "{",
      "  %",
      "  % Display a DirectClass or PseudoClass image.",
      "  %",
      "  % Parameters:",
      "  %   x & y translation.",
      "  %   x & y scale.",
      "  %   image label.",
      "  %   image columns & rows.",
      "  %   class: 0-DirectClass or 1-PseudoClass.",
      "  %   compression: 0-RunlengthEncodedCompression or 1-NoCompression.",
      "  %   hex color packets.",
      "  %",
      "  gsave",
      "  currentfile buffer readline pop",
      "  token pop /x exch def",
      "  token pop /y exch def pop",
      "  x y translate",
      "  currentfile buffer readline pop",
      "  token pop /x exch def",
      "  token pop /y exch def pop",
      "  /NewCenturySchlbk-Roman findfont 24 scalefont setfont",
      "  currentfile buffer readline pop",
      "  0 y 12 add moveto buffer show pop",
      "  x y scale",
      "  currentfile buffer readline pop",
      "  token pop /columns exch def",
      "  token pop /rows exch def pop",
      "  currentfile buffer readline pop",
      "  token pop /class exch def pop",
      "  currentfile buffer readline pop",
      "  token pop /compression exch def pop",
      "  class 0 gt { PseudoClassImage } { DirectClassImage } ifelse",
      "  grestore",
      (char *) NULL
    };

  char
    date[MaxTextLength],
    *label,
    **q;

  int
    length,
    sans_offset,
    x,
    y;

  register RunlengthPacket
    *p;

  register int
    i,
    j;

  time_t
    timer;

  unsigned int
    bit,
    Byte,
    count,
    dx_resolution,
    dy_resolution,
    height,
    polarity,
    text_size,
    x_resolution,
    y_resolution,
    width;

  /*
    Open output image file.
  */
  OpenImage(image,"w");
  if (image->file == (FILE *) NULL)
    PrematureExit("Unable to open file",image);
  /*
    Scale image to size of Postscript page.
  */
  text_size=image->label == (char *) NULL ? 0 : 36;
  x=0;
  y=0;
  width=image->columns;
  height=image->rows;
  if (strcmp(image_info->magick,"PS") == 0)
    {
      int
        delta_x,
        delta_y,
        flags;

      unsigned int
        page_height,
        page_width;

      /*
        Center image on Postscript page.
      */
      (void) XParseGeometry(PSPageGeometry,&x,&y,&page_width,&page_height);
      flags=NoValue;
      if (image_info->page != (char *) NULL)
        flags=XParseGeometry(image_info->page,&x,&y,&page_width,&page_height);
      if (((page_width-(x << 1)) < width) ||
          ((page_height-(y << 1)-text_size) < height))
        {
          unsigned long
            scale_factor;

          /*
            Scale image relative to Postscript page.
          */
          scale_factor=UpShift(page_width-(x << 1))/width;
          if (scale_factor > (UpShift(page_height-(y << 1)-text_size)/height))
            scale_factor=UpShift(page_height-(y << 1)-text_size)/height;
          width=DownShift(width*scale_factor);
          height=DownShift(height*scale_factor);
        }
      if ((flags & XValue) == 0)
        {
          /*
            Center image in the X direction.
          */
          delta_x=page_width-(width+(x << 1));
          if (delta_x >= 0)
            x=(delta_x >> 1)+x;
        }
      if ((flags & YValue) == 0)
        {
          /*
            Center image in the X direction.
          */
          delta_y=page_height-(height+(y << 1))-text_size;
          if (delta_y >= 0)
            y=(delta_y >> 1)+y;
        }
    }
  /*
    Scale relative to dots-per-inch.
  */
  (void) XParseGeometry(PSDensityGeometry,&sans_offset,&sans_offset,
    &dx_resolution,&dy_resolution);
  x_resolution=dx_resolution;
  y_resolution=dy_resolution;
  if (image_info->density != (char *) NULL)
    (void) XParseGeometry(image_info->density,&sans_offset,&sans_offset,
      &x_resolution,&y_resolution);
  width=(width*dx_resolution)/x_resolution;
  height=(height*dy_resolution)/y_resolution;
  /*
    Output Postscript header.
  */
  if (strcmp(image_info->magick,"PS") == 0)
    (void) fprintf(image->file,"%%!PS-Adobe-3.0\n");
  else
    (void) fprintf(image->file,"%%!PS-Adobe-3.0 EPSF-3.0\n");
  (void) fprintf(image->file,"%%%%Creator: (ImageMagick)\n");
  (void) fprintf(image->file,"%%%%Title: (%s)\n",image->filename);
  timer=time((time_t *) NULL);
  (void) localtime(&timer);
  (void) strcpy(date,ctime(&timer));
  date[strlen(date)-1]='\0';
  (void) fprintf(image->file,"%%%%CreationDate: (%s)\n",date);
  (void) fprintf(image->file,"%%%%BoundingBox: %d %d %d %d\n",x,y,
    x+(int) width-1,y+(int) (height+text_size)-1);
  if (image->label != (char *) NULL)
    (void) fprintf(image->file,
      "%%%%DocumentNeededResources: font NewCenturySchlbk-Roman\n");
  (void) fprintf(image->file,"%%%%DocumentData: Clean7Bit\n");
  (void) fprintf(image->file,"%%%%LanguageLevel: 1\n");
  if (strcmp(image_info->magick,"PS") == 0)
    {
      (void) fprintf(image->file,"%%%%Orientation: Portrait\n");
      (void) fprintf(image->file,"%%%%PageOrder: Ascend\n");
    }
  (void) fprintf(image->file,"%%%%Pages: %d\n",
    strcmp(image_info->magick,"PS") == 0);
  (void) fprintf(image->file,"%%%%EndComments\n");
  (void) fprintf(image->file,"\n%%%%BeginDefaults\n");
  (void) fprintf(image->file,"%%%%PageOrientation: Portrait\n");
  (void) fprintf(image->file,"%%%%EndDefaults\n\n");
  if (strcmp(image_info->magick,"EPSI") == 0)
    {
      Image
        *preview_image;

      /*
        Create preview image.
      */
      image->orphan=True;
      preview_image=CopyImage(image,image->columns,image->rows,True);
      image->orphan=False;
      if (preview_image == (Image *) NULL)
        PrematureExit("Unable to allocate memory",image);
      /*
        Dump image as bitmap.
      */
      if (!IsGrayImage(image) || (image->colors != 2))
        {
          QuantizeImage(preview_image,2,8,image_info->dither,GRAYColorspace);
          SyncImage(preview_image);
        }
      polarity=0;
      if (image->colors == 2)
        polarity=Intensity(image->colormap[0]) > Intensity(image->colormap[1]);
      bit=0;
      Byte=0;
      count=0;
      x=0;
      p=preview_image->pixels;
      (void) fprintf(image->file,"%%%%BeginPreview: %u %u %u %u\n%%  ",
        preview_image->columns,preview_image->rows,(unsigned int) 1,
        (((preview_image->columns+7) >> 3)*preview_image->rows+35)/36);
      for (i=0; i < preview_image->packets; i++)
      {
        for (j=0; j <= ((int) p->length); j++)
        {
          Byte<<=1;
          if (p->index == polarity)
            Byte|=0x01;
          bit++;
          if (bit == 8)
            {
              (void) fprintf(image->file,"%02x",Byte & 0xff);
              count++;
              if (count == 36)
                {
                  (void) fprintf(image->file,"\n%%  ");
                  count=0;
                };
              bit=0;
              Byte=0;
            }
          x++;
          if (x == preview_image->columns)
            {
              if (bit != 0)
                {
                  Byte<<=(8-bit);
                  (void) fprintf(image->file,"%02x",Byte & 0xff);
                  count++;
                  if (count == 36)
                    {
                      (void) fprintf(image->file,"\n%%  ");
                      count=0;
                    };
                  bit=0;
                  Byte=0;
                };
              x=0;
            }
          }
          p++;
        }
        (void) fprintf(image->file,"\n%%%%EndPreview\n");
        DestroyImage(preview_image);
      }
  /*
    Output Postscript commands.
  */
  for (q=Postscript; *q; q++)
    (void) fprintf(image->file,"%s\n",*q);
  if (strcmp(image_info->magick,"PS") == 0)
    (void) fprintf(image->file,"  showpage\n");
  (void) fprintf(image->file,"} bind def\n");
  (void) fprintf(image->file,"%%%%EndProlog\n");
  (void) fprintf(image->file,"%%%%Page:  1 1\n");
  (void) fprintf(image->file,"%%%%PageBoundingBox: %d %d %d %d\n",x,y,
    x+(int) width-1,y+(int) (height+text_size)-1);
  if (image->label != (char *) NULL)
    (void) fprintf(image->file,
      "%%%%PageResources: font NewCenturySchlbk-Roman\n");
  if (strcmp(image_info->magick,"PS") != 0)
    (void) fprintf(image->file,"userdict begin\n");
  (void) fprintf(image->file,"%%%%BeginData:\n");
  (void) fprintf(image->file,"DisplayImage\n");
  /*
    Output image data.
  */
  if (image->compression == RunlengthEncodedCompression)
    CompressImage(image);
  label=image->label;
  if (label == (char *) NULL)
    label=" ";
  p=image->pixels;
  switch (image->class)
  {
    case DirectClass:
    {
      (void) fprintf(image->file,"%d %d\n%u %u\n%s           \n%u %u\n%d\n%d\n",
        x,y,width,height,label,image->columns,image->rows,
        image->class == PseudoClass,image->compression == NoCompression);
      switch (image->compression)
      {
        case RunlengthEncodedCompression:
        default:
        {
          /*
            Dump runlength-encoded DirectColor packets.
          */
          x=0;
          for (i=0; i < image->packets; i++)
          {
            for (length=p->length; length >= 0; length-=256)
            {
              if (image->matte && (p->index == Transparent))
                (void) fprintf(image->file,"ffffff%02x",(unsigned int)
                  Min(length,0xff));
              else
                (void) fprintf(image->file,"%02x%02x%02x%02x",DownScale(p->red),
                  DownScale(p->green),DownScale(p->blue),(unsigned int)
                  Min(length,0xff));
              x++;
              if (x == 9)
                {
                  x=0;
                  (void) fprintf(image->file,"\n");
                }
            }
            p++;
            if (QuantumTick(i,image))
              ProgressMonitor(SaveImageText,i,image->packets);
          }
          break;
        }
        case NoCompression:
        {
          /*
            Dump uncompressed DirectColor packets.
          */
          x=0;
          for (i=0; i < image->packets; i++)
          {
            for (j=0; j <= ((int) p->length); j++)
            {
              if (image->matte && (p->index == Transparent))
                (void) fprintf(image->file,"ffffff");
              else
                (void) fprintf(image->file,"%02x%02x%02x",DownScale(p->red),
                  DownScale(p->green),DownScale(p->blue));
              x++;
              if (x == 12)
                {
                  x=0;
                  (void) fprintf(image->file,"\n");
                }
            }
            p++;
            if (QuantumTick(i,image))
              ProgressMonitor(SaveImageText,i,image->packets);
          }
          break;
        }
      }
      break;
    }
    case PseudoClass:
    {
      unsigned int
        grayscale;

      (void) fprintf(image->file,"%d %d\n%u %u\n%s           \n%u %u\n%d\n%d\n",
        x,y,width,height,label,image->columns,image->rows,
        image->class == PseudoClass,image->compression == NoCompression);
      grayscale=IsGrayImage(image) && (image->compression == NoCompression);
      (void) fprintf(image->file,"%u\n",grayscale);
      if (grayscale)
        (void) fprintf(image->file,"%d\n",image->colors == 2 ? 1 : 8);
      else
        {
          /*
            Dump number of colors and colormap.
          */
          (void) fprintf(image->file,"%u\n",image->colors);
          for (i=0; i < image->colors; i++)
            (void) fprintf(image->file,"%02x%02x%02x\n",
              DownScale(image->colormap[i].red),
              DownScale(image->colormap[i].green),
              DownScale(image->colormap[i].blue));
        }
      switch (image->compression)
      {
        case RunlengthEncodedCompression:
        default:
        {
          /*
            Dump runlength-encoded PseudoColor packets.
          */
          x=0;
          for (i=0; i < image->packets; i++)
          {
            for (length=p->length; length >= 0; length-=256)
            {
              (void) fprintf(image->file,"%02x%02x",(unsigned int) p->index,
                (unsigned int) Min(length,0xff));
              x++;
              if (x == 18)
                {
                  x=0;
                  (void) fprintf(image->file,"\n");
                }
            }
            p++;
            if (QuantumTick(i,image))
              ProgressMonitor(SaveImageText,i,image->packets);
          }
          break;
        }
        case NoCompression:
        {
          /*
            Dump uncompressed PseudoColor packets.
          */
          x=0;
          if (!grayscale)
            for (i=0; i < image->packets; i++)
            {
              for (j=0; j <= ((int) p->length); j++)
              {
                (void) fprintf(image->file,"%02x",(unsigned int) p->index);
                x++;
                if (x == 36)
                  {
                    x=0;
                    (void) fprintf(image->file,"\n");
                  }
              }
              p++;
              if (QuantumTick(i,image))
                ProgressMonitor(SaveImageText,i,image->packets);
            }
          else
            if (image->colors != 2)
              for (i=0; i < image->packets; i++)
              {
                for (j=0; j <= ((int) p->length); j++)
                {
                  (void) fprintf(image->file,"%02x",DownScale(p->red));
                  x++;
                  if (x == 36)
                    {
                      x=0;
                      (void) fprintf(image->file,"\n");
                    }
                }
                p++;
                if (QuantumTick(i,image))
                  ProgressMonitor(SaveImageText,i,image->packets);
              }
            else
              {
                /*
                  Dump image as bitmap.
                */
                polarity=
                  Intensity(image->colormap[1]) > Intensity(image->colormap[0]);
                bit=0;
                Byte=0;
                count=0;
                x=0;
                p=image->pixels;
                for (i=0; i < image->packets; i++)
                {
                  for (j=0; j <= ((int) p->length); j++)
                  {
                    Byte<<=1;
                    if (p->index == polarity)
                      Byte|=0x01;
                    bit++;
                    if (bit == 8)
                      {
                        (void) fprintf(image->file,"%02x",Byte & 0xff);
                        count++;
                        if (count == 36)
                          {
                            (void) fprintf(image->file,"\n");
                            count=0;
                          };
                        bit=0;
                        Byte=0;
                      }
                    x++;
                    if (x == image->columns)
                      {
                        if (bit != 0)
                          {
                            Byte<<=(8-bit);
                            (void) fprintf(image->file,"%02x",Byte & 0xff);
                            count++;
                            if (count == 36)
                              {
                                (void) fprintf(image->file,"\n");
                                count=0;
                              };
                            bit=0;
                            Byte=0;
                          };
                        x=0;
                      }
                    }
                    p++;
                    if (QuantumTick(i,image))
                      ProgressMonitor(SaveImageText,i,image->packets);
                  }
                }
          break;
        }
      }
    }
  }
  (void) fprintf(image->file,"\n");
  (void) fprintf(image->file,"%%%%EndData\n");
  if (strcmp(image_info->magick,"PS") != 0)
    (void) fprintf(image->file,"end\n");
  (void) fprintf(image->file,"%%%%PageTrailer\n");
  (void) fprintf(image->file,"%%%%Trailer\n");
  (void) fprintf(image->file,"%%%%EOF\n");
  CloseImage(image);
  return(True);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   W r i t e P S 2 I m a g e                                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Function WritePS2Image translates an image to encapsulated Postscript
%  Level II for printing.  If the supplied geometry is null, the image is
%  centered on the Postscript page.  Otherwise, the image is positioned as
%  specified by the geometry.
%
%  The format of the WritePS2Image routine is:
%
%      status=WritePS2Image(image_info,image)
%
%  A description of each parameter follows:
%
%    o status: Function WritePS2Image return True if the image is printed.
%      False is returned if the image file cannot be opened for printing.
%
%    o image_info: Specifies a pointer to an ImageInfo structure.
%
%    o image: The address of a structure of type Image;  returned from
%      ReadImage.
%
%
*/
static unsigned int WritePS2Image(ImageInfo *image_info, Image *image)
{
  static char
    *Postscript[]=
    {
      "%%BeginProlog",
      "%",
      "% Display a color image.  The image is displayed in color on",
      "% Postscript viewers or printers that support color, otherwise",
      "% it is displayed as grayscale.",
      "%",
      "/buffer 512 string def",
      "",
      "/DirectClassImage",
      "{",
      "  %",
      "  % Display a DirectClass image.",
      "  %",
      "  /DeviceRGB setcolorspace",
      "  <<",
      "    /ImageType 1",
      "    /Interpolate true",
      "    /Width columns",
      "    /Height rows",
      "    /BitsPerComponent 8",
      "    /Decode [0 1 0 1 0 1]",
      "    /ImageMatrix [columns 0 0 rows neg 0 rows]",
      "    compression 0 gt",
      "    {",
      "      /DataSource currentfile /ASCII85Decode filter",
      "    }",
      "    {",
      "      /DataSource currentfile /ASCII85Decode filter /RunLengthDecode filter",
      "    } ifelse",
      "  >> image",
      "} bind def",
      "",
      "/PseudoClassImage",
      "{",
      "  %",
      "  % Display a PseudoClass image.",
      "  %",
      "  % Parameters:",
      "  %   colors: number of colors in the colormap.",
      "  %",
      "  currentfile buffer readline pop",
      "  token pop /colors exch def pop",
      "  colors 0 eq",
      "  {",
      "    %",
      "    % Image is grayscale.",
      "    %",
      "    /DeviceGray setcolorspace",
      "    <<",
      "      /ImageType 1",
      "      /Interpolate true",
      "      /Width columns",
      "      /Height rows",
      "      /BitsPerComponent 1",
      "      /Decode [0 1]",
      "      /ImageMatrix [columns 0 0 rows neg 0 rows]",
      "      compression 0 gt",
      "      {",
      "        /DataSource currentfile /ASCII85Decode filter",
      "      }",
      "      {",
      "        /DataSource currentfile /ASCII85Decode filter /RunLengthDecode filter",
      "      } ifelse",
      "    >> image",
      "  }",
      "  {",
      "    %",
      "    % Parameters:",
      "    %   colormap: red, green, blue color packets.",
      "    %",
      "    /colormap colors 3 mul string def",
      "    currentfile colormap readhexstring pop pop",
      "    [ /Indexed /DeviceRGB colors 1 sub colormap ] setcolorspace",
      "    <<",
      "      /ImageType 1",
      "      /Interpolate true",
      "      /Width columns",
      "      /Height rows",
      "      /BitsPerComponent 8",
      "      /Decode [0 255]",
      "      /ImageMatrix [columns 0 0 rows neg 0 rows]",
      "      compression 0 gt",
      "      {",
      "        /DataSource currentfile /ASCII85Decode filter",
      "      }",
      "      {",
      "        /DataSource currentfile /ASCII85Decode filter /RunLengthDecode filter",
      "      } ifelse",
      "    >> image",
      "  } ifelse",
      "} bind def",
      "",
      "/DisplayImage",
      "{",
      "  %",
      "  % Display a DirectClass or PseudoClass image.",
      "  %",
      "  % Parameters:",
      "  %   x & y translation.",
      "  %   x & y scale.",
      "  %   image label.",
      "  %   image columns & rows.",
      "  %   class: 0-DirectClass or 1-PseudoClass.",
      "  %   compression: 0-RunlengthEncodedCompression or 1-NoCompression.",
      "  %   hex color packets.",
      "  %",
      "  gsave",
      "  currentfile buffer readline pop",
      "  token pop /x exch def",
      "  token pop /y exch def pop",
      "  x y translate",
      "  currentfile buffer readline pop",
      "  token pop /x exch def",
      "  token pop /y exch def pop",
      "  /NewCenturySchlbk-Roman findfont 24 scalefont setfont",
      "  currentfile buffer readline pop",
      "  0 y 12 add moveto buffer show pop",
      "  x y scale",
      "  currentfile buffer readline pop",
      "  token pop /columns exch def",
      "  token pop /rows exch def pop",
      "  currentfile buffer readline pop",
      "  token pop /class exch def pop",
      "  currentfile buffer readline pop",
      "  token pop /compression exch def pop",
      "  class 0 gt { PseudoClassImage } { DirectClassImage } ifelse",
      "  grestore",
      "  showpage",
      "} bind def",
      "%%EndProlog",
      "%%Page:  1 1",
      NULL
    };

  char
    date[MaxTextLength],
    *label,
    **q;

  int
    delta_x,
    delta_y,
    flags,
    sans_offset,
    x,
    y;

  register RunlengthPacket
    *p;

  register int
    i,
    j;

  time_t
    timer;

  unsigned char
    *pixels;

  unsigned int
    dx_resolution,
    dy_resolution,
    height,
    number_packets,
    page_height,
    page_width,
    text_size,
    x_resolution,
    y_resolution,
    width;

  /*
    Open output image file.
  */
  OpenImage(image,"w");
  if (image->file == (FILE *) NULL)
    PrematureExit("Unable to open file",image);
  /*
    Scale image to size of Postscript page.
  */
  text_size=image->label == (char *) NULL ? 0 : 36;
  x=0;
  y=0;
  width=image->columns;
  height=image->rows;
  /*
    Center image on Postscript page.
  */
  (void) XParseGeometry(PSPageGeometry,&x,&y,&page_width,&page_height);
  flags=NoValue;
  if (image_info->page != (char *) NULL)
    flags=XParseGeometry(image_info->page,&x,&y,&page_width,&page_height);
  if (((page_width-(x << 1)) < width) ||
      ((page_height-(y << 1)-text_size) < height))
    {
      unsigned long
        scale_factor;

      /*
        Scale image relative to Postscript page.
      */
      scale_factor=UpShift(page_width-(x << 1))/width;
      if (scale_factor > (UpShift(page_height-(y << 1)-text_size)/height))
        scale_factor=UpShift(page_height-(y << 1)-text_size)/height;
      width=DownShift(width*scale_factor);
      height=DownShift(height*scale_factor);
    }
  if ((flags & XValue) == 0)
    {
      /*
        Center image in the X direction.
      */
      delta_x=page_width-(width+(x << 1));
      if (delta_x >= 0)
        x=(delta_x >> 1)+x;
    }
  if ((flags & YValue) == 0)
    {
      /*
        Center image in the X direction.
      */
      delta_y=page_height-(height+(y << 1))-text_size;
      if (delta_y >= 0)
        y=(delta_y >> 1)+y;
    }
  /*
    Scale relative to dots-per-inch.
  */
  (void) XParseGeometry(PSDensityGeometry,&sans_offset,&sans_offset,
    &dx_resolution,&dy_resolution);
  x_resolution=dx_resolution;
  y_resolution=dy_resolution;
  if (image_info->density != (char *) NULL)
    (void) XParseGeometry(image_info->density,&sans_offset,&sans_offset,
      &x_resolution,&y_resolution);
  width=(width*dx_resolution)/x_resolution;
  height=(height*dy_resolution)/y_resolution;
  /*
    Output Postscript header.
  */
  if (strcmp(image_info->magick,"PS") == 0)
    (void) fprintf(image->file,"%%!PS-Adobe-3.0\n");
  else
    (void) fprintf(image->file,"%%!PS-Adobe-3.0 EPSF-3.0\n");
  (void) fprintf(image->file,"%%%%Creator: (ImageMagick)\n");
  (void) fprintf(image->file,"%%%%Title: (%s)\n",image->filename);
  timer=time((time_t *) NULL);
  (void) localtime(&timer);
  (void) strcpy(date,ctime(&timer));
  date[strlen(date)-1]='\0';
  (void) fprintf(image->file,"%%%%CreationDate: (%s)\n",date);
  (void) fprintf(image->file,"%%%%BoundingBox: %d %d %d %d\n",x,y,
    x+(int) width-1,y+(int) (height+text_size)-1);
  if (image->label != (char *) NULL)
    (void) fprintf(image->file,
      "%%%%DocumentNeededResources: font NewCenturySchlbk-Roman\n");
  (void) fprintf(image->file,"%%%%LanguageLevel: 2\n");
  if (strcmp(image_info->magick,"PS") == 0)
    {
      (void) fprintf(image->file,"%%%%Orientation: Portrait\n");
      (void) fprintf(image->file,"%%%%PageOrder: Ascend\n");
    }
  (void) fprintf(image->file,"%%%%Pages: %d\n",
    strcmp(image_info->magick,"PS") == 0);
  (void) fprintf(image->file,"%%%%EndComments\n");
  (void) fprintf(image->file,"\n%%%%BeginDefaults\n");
  (void) fprintf(image->file,"%%%%PageOrientation: Portrait\n");
  (void) fprintf(image->file,"%%%%EndDefaults\n\n");
  /*
    Output Postscript commands.
  */
  for (q=Postscript; *q; q++)
    (void) fprintf(image->file,"%s\n",*q);
  (void) fprintf(image->file,"%%%%PageBoundingBox: %d %d %d %d\n",x,y,
    x+(int) width-1,y+(int) (height+text_size)-1);
  if (image->label != (char *) NULL)
    (void) fprintf(image->file,
      "%%%%PageResources: font NewCenturySchlbk-Roman\n");
  if (strcmp(image_info->magick,"PS") != 0)
    (void) fprintf(image->file,"userdict begin\n");
  (void) fprintf(image->file,"%%%%BeginData:\n");
  (void) fprintf(image->file,"DisplayImage\n");
  /*
    Output image data.
  */
  label=image->label;
  if (label == (char *) NULL)
    label=" ";
  (void) fprintf(image->file,"%d %d\n%u %u\n%s           \n%u %u\n%d\n%d\n",x,y,
    width,height,label,image->columns,image->rows,image->class == PseudoClass,
    image->compression == NoCompression);
  p=image->pixels;
  if (!IsPseudoClass(image) && !IsGrayImage(image))
    switch (image->compression)
    {
      case RunlengthEncodedCompression:
      default:
      {
        register unsigned char
          *q;

        /*
          Allocate pixel array.
        */
        number_packets=3*image->columns*image->rows;
        pixels=(unsigned char *) malloc(number_packets*sizeof(unsigned char));
        if (pixels == (unsigned char *) NULL)
          PrematureExit("Unable to allocate memory",image);
        /*
          Dump Packbit encoded pixels.
        */
        q=pixels;
        for (i=0; i < image->packets; i++)
        {
          for (j=0; j <= (int) p->length; j++)
          {
            if (image->matte && (p->index == Transparent))
              {
                *q++=DownScale(MaxRGB);
                *q++=DownScale(MaxRGB);
                *q++=DownScale(MaxRGB);
              }
            else
              {
                *q++=DownScale(p->red);
                *q++=DownScale(p->green);
                *q++=DownScale(p->blue);
              }
          }
          p++;
          if (QuantumTick(i,image))
            ProgressMonitor(SaveImageText,i,image->packets);
        }
        (void) PackbitsEncodeImage(image->file,pixels,number_packets);
        free((char *) pixels);
        break;
      }
      case NoCompression:
      {
        /*
          Dump uncompressed DirectColor packets.
        */
        Ascii85Initialize();
        for (i=0; i < image->packets; i++)
        {
          for (j=0; j <= ((int) p->length); j++)
          {
            if (image->matte && (p->index == Transparent))
              {
                Ascii85Encode(DownScale(MaxRGB),image->file);
                Ascii85Encode(DownScale(MaxRGB),image->file);
                Ascii85Encode(DownScale(MaxRGB),image->file);
              }
            else
              {
                Ascii85Encode(DownScale(p->red),image->file);
                Ascii85Encode(DownScale(p->green),image->file);
                Ascii85Encode(DownScale(p->blue),image->file);
              }
          }
          p++;
          if (QuantumTick(i,image))
            ProgressMonitor(SaveImageText,i,image->packets);
        }
        Ascii85Flush(image->file);
        break;
      }
    }
  else
    if (IsGrayImage(image) && (image->colors == 2))
      {
        register unsigned char
          bit,
          Byte,
          polarity;

        polarity=Intensity(image->colormap[0]) < Intensity(image->colormap[1]);
        bit=0;
        Byte=0;
        (void) fprintf(image->file,"0\n");
        switch (image->compression)
        {
          case RunlengthEncodedCompression:
          default:
          {
            register unsigned char
              *q;

            /*
              Allocate pixel array.
            */
            number_packets=((image->columns+7) >> 3)*image->rows;
            pixels=(unsigned char *)
              malloc(number_packets*sizeof(unsigned char));
            if (pixels == (unsigned char *) NULL)
              PrematureExit("Unable to allocate memory",image);
            /*
              Dump GIF encoded pixels.
            */
            q=pixels;
            for (i=0; i < image->packets; i++)
            {
              for (j=0; j <= (int) p->length; j++)
              {
                Byte<<=1;
                if (p->index == polarity)
                  Byte|=0x01;
                bit++;
                if (bit == 8)
                  {
                    *q++=Byte;
                    bit=0;
                    Byte=0;
                  }
                x++;
                if (x == image->columns)
                  {
                    /*
                      Advance to the next scanline.
                    */
                    if (bit != 0)
                      *q++=Byte << (8-bit);
                    bit=0;
                    Byte=0;
                    x=0;
                    y++;
                 }
              }
              p++;
              if (QuantumTick(i,image))
                ProgressMonitor(SaveImageText,i,image->packets);
            }
            (void) PackbitsEncodeImage(image->file,pixels,number_packets);
            free((char *) pixels);
            break;
          }
          case NoCompression:
          {
            /*
              Dump uncompressed PseudoColor packets.
            */
            Ascii85Initialize();
            for (i=0; i < image->packets; i++)
            {
              for (j=0; j <= (int) p->length; j++)
              {
                Byte<<=1;
                if (p->index == polarity)
                  Byte|=0x01;
                bit++;
                if (bit == 8)
                  {
                    Ascii85Encode(Byte,image->file);
                    bit=0;
                    Byte=0;
                  }
                x++;
                if (x == image->columns)
                  {
                    /*
                      Advance to the next scanline.
                    */
                    if (bit != 0)
                      Ascii85Encode(Byte << (8-bit),image->file);
                    bit=0;
                    Byte=0;
                    x=0;
                    y++;
                 }
              }
              p++;
              if (QuantumTick(i,image))
                ProgressMonitor(SaveImageText,i,image->packets);
            }
            Ascii85Flush(image->file);
            break;
          }
        }
      }
    else
      {
        /*
          Dump number of colors and colormap.
        */
        (void) fprintf(image->file,"%u\n",image->colors);
        for (i=0; i < image->colors; i++)
          (void) fprintf(image->file,"%02x%02x%02x\n",
            DownScale(image->colormap[i].red),
            DownScale(image->colormap[i].green),
            DownScale(image->colormap[i].blue));
        switch (image->compression)
        {
          case RunlengthEncodedCompression:
          default:
          {
            register unsigned char
              *q;

            /*
              Allocate pixel array.
            */
            number_packets=image->columns*image->rows;
            pixels=(unsigned char *)
              malloc(number_packets*sizeof(unsigned char));
            if (pixels == (unsigned char *) NULL)
              PrematureExit("Unable to allocate memory",image);
            /*
              Dump GIF encoded pixels.
            */
            q=pixels;
            for (i=0; i < image->packets; i++)
            {
              for (j=0; j <= (int) p->length; j++)
                *q++=(unsigned char) p->index;
              p++;
              if (QuantumTick(i,image))
                ProgressMonitor(SaveImageText,i,image->packets);
            }
            (void) PackbitsEncodeImage(image->file,pixels,number_packets);
            free((char *) pixels);
            break;
          }
          case NoCompression:
          {
            /*
              Dump uncompressed PseudoColor packets.
            */
            Ascii85Initialize();
            for (i=0; i < image->packets; i++)
            {
              for (j=0; j <= ((int) p->length); j++)
                Ascii85Encode((unsigned char) p->index,image->file);
              p++;
              if (QuantumTick(i,image))
                ProgressMonitor(SaveImageText,i,image->packets);
            }
            Ascii85Flush(image->file);
            break;
          }
        }
      }
  (void) fprintf(image->file,"\n");
  (void) fprintf(image->file,"%%%%EndData\n");
  if (strcmp(image_info->magick,"PS") != 0)
    (void) fprintf(image->file,"end\n");
  (void) fprintf(image->file,"%%%%PageTrailer\n");
  (void) fprintf(image->file,"%%%%Trailer\n");
  (void) fprintf(image->file,"%%%%EOF\n");
  CloseImage(image);
  return(True);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   W r i t e R A D I A N C E I m a g e                                       %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Function WriteRADIANCEImage writes an image in the RADIANCE encoded image
%  format.
%
%  The format of the WriteRADIANCEImage routine is:
%
%      status=WriteRADIANCEImage(image_info,image)
%
%  A description of each parameter follows.
%
%    o status: Function WriteRADIANCEImage return True if the image is written.
%      False is returned is there is a memory shortage or if the image file
%      fails to write.
%
%    o image_info: Specifies a pointer to an ImageInfo structure.
%
%    o image:  A pointer to a Image structure.
%
%
*/
static unsigned int WriteRADIANCEImage(ImageInfo *image_info, Image *image)
{
  unsigned int
    status;

  Warning("Cannot write RADIANCE images",image->filename);
  status=WriteMIFFImage(image_info,image);
  return(status);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   W r i t e R G B I m a g e                                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Function WriteRGBImage writes an image to a file in red, green, and
%  blue rasterfile format.
%
%  The format of the WriteRGBImage routine is:
%
%      status=WriteRGBImage(image_info,image)
%
%  A description of each parameter follows.
%
%    o status: Function WriteRGBImage return True if the image is written.
%      False is returned is there is a memory shortage or if the image file
%      fails to write.
%
%    o image_info: Specifies a pointer to an ImageInfo structure.
%
%    o image:  A pointer to a Image structure.
%
%
*/
static unsigned int WriteRGBImage(ImageInfo *image_info, Image *image)
{
  register int
    i,
    j;

  register RunlengthPacket
    *p;

  /*
    Open output image file.
  */
  OpenImage(image,WriteBinaryType);
  if (image->file == (FILE *) NULL)
    PrematureExit("Unable to open file",image);
  /*
    Convert MIFF to RGB raster pixels.
  */
  if (strcmp(image_info->magick,"RGB") == 0)
    image->matte=False;
  switch (image_info->interlace)
  {
    case NoneInterlace:
    default:
    {
      /*
        No interlacing:  RGBRGBRGBRGBRGBRGB...
      */
      p=image->pixels;
      for (i=0; i < image->packets; i++)
      {
        for (j=0; j <= ((int) p->length); j++)
        {
          WriteQuantumFile(p->red);
          WriteQuantumFile(p->green);
          WriteQuantumFile(p->blue);
          if (image->matte)
            WriteQuantumFile(p->index);
        }
        p++;
        if (QuantumTick(i,image))
          ProgressMonitor(SaveImageText,i,image->packets);
      }
      break;
    }
    case LineInterlace:
    {
      register int
        x,
        y;

      /*
        Line interlacing:  RRR...GGG...BBB...RRR...GGG...BBB...
      */
      if (!UncompressImage(image))
        return(False);
      for (y=0; y < image->rows; y++)
      {
        p=image->pixels+(y*image->columns);
        for (x=0; x < image->columns; x++)
        {
          WriteQuantumFile(p->red);
          p++;
        }
        p=image->pixels+(y*image->columns);
        for (x=0; x < image->columns; x++)
        {
          WriteQuantumFile(p->green);
          p++;
        }
        p=image->pixels+(y*image->columns);
        for (x=0; x < image->columns; x++)
        {
          WriteQuantumFile(p->blue);
          p++;
        }
        p=image->pixels+(y*image->columns);
        if (image->matte)
          for (x=0; x < image->columns; x++)
          {
            WriteQuantumFile(p->index);
            p++;
          }
        ProgressMonitor(SaveImageText,y,image->rows);
      }
      break;
    }
    case PlaneInterlace:
    {
      /*
        Plane interlacing:  RRRRRR...GGGGGG...BBBBBB...
      */
      p=image->pixels;
      for (i=0; i < image->packets; i++)
      {
        for (j=0; j <= ((int) p->length); j++)
          WriteQuantumFile(p->red);
        p++;
      }
      ProgressMonitor(SaveImageText,100,400);
      p=image->pixels;
      for (i=0; i < image->packets; i++)
      {
        for (j=0; j <= ((int) p->length); j++)
          WriteQuantumFile(p->green);
        p++;
      }
      ProgressMonitor(SaveImageText,200,400);
      p=image->pixels;
      for (i=0; i < image->packets; i++)
      {
        for (j=0; j <= ((int) p->length); j++)
          WriteQuantumFile(p->blue);
        p++;
      }
      ProgressMonitor(SaveImageText,300,400);
      p=image->pixels;
      if (image->matte)
        for (i=0; i < image->packets; i++)
        {
          for (j=0; j <= ((int) p->length); j++)
            WriteQuantumFile(p->index);
          p++;
        }
      ProgressMonitor(SaveImageText,400,400);
      break;
    }
  }
  CloseImage(image);
  return(True);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   W r i t e R L E I m a g e                                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Function WriteRLEImage writes an image in the Utah Run length encoded image
%  format.
%
%  The format of the WriteRLEImage routine is:
%
%      status=WriteRLEImage(image_info,image)
%
%  A description of each parameter follows.
%
%    o status: Function WriteRLEImage return True if the image is written.
%      False is returned is there is a memory shortage or if the image file
%      fails to write.
%
%    o image_info: Specifies a pointer to an ImageInfo structure.
%
%    o image:  A pointer to a Image structure.
%
%
*/
static unsigned int WriteRLEImage(ImageInfo *image_info, Image *image)
{
  char
    command[MaxTextLength];

  Image
    *flipped_image;

  ImageInfo
    rle_info;

  unsigned int
    packet_size,
    status;

  /*
    Flip image.
  */
  image->orphan=True;
  flipped_image=FlipImage(image);
  image->orphan=False;
  if (flipped_image == (Image *) NULL)
    PrematureExit("Unable to flip image",image);
  rle_info=(*image_info);
  rle_info.interlace=NoneInterlace;
  TemporaryFilename(flipped_image->filename);
  status=WriteRGBImage(&rle_info,flipped_image);
  if (status)
    {
      packet_size=3;
      if (image->matte)
        packet_size=4;
      (void) sprintf(command,"rawtorle -w %u -h %u -n %u -o %s %s\n",
        image->columns,image->rows,packet_size,image_info->filename,
        flipped_image->filename);
      status=SystemCommand(command);
      (void) unlink(flipped_image->filename);
      if (access(image_info->filename,0) != 0)
        Warning("RLE translation failed",command);
    }
  DestroyImage(flipped_image);
  return(status);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   W r i t e S G I I m a g e                                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Function WriteSGIImage writes an image in SGI RGB encoded image format.
%
%  The format of the WriteSGIImage routine is:
%
%      status=WriteSGIImage(image_info,image)
%
%  A description of each parameter follows.
%
%    o status: Function WriteSGIImage return True if the image is written.
%      False is returned is there is a memory shortage or if the image file
%      fails to write.
%
%    o image_info: Specifies a pointer to an ImageInfo structure.
%
%    o image:  A pointer to a Image structure.
%
%
*/

static int SGIEncode(unsigned char *pixels, int count, unsigned char *packets)
{
  short
    runlength;

  unsigned char
    *limit,
    *mark,
    *p,
    *q;

  p=pixels;
  limit=p+count*4;
  q=packets;
  while (p < limit)
  {
    mark=p;
    p+=8;
    while ((p < limit) && ((*(p-8) != *(p-4)) || (*(p-4) != *p)))
      p+=4;
    p-=8;
    count=((p-mark) >> 2);
    while (count)
    {
      runlength=count > 126 ? 126 : count;
      count-=runlength;
      *q++=0x80 | runlength;
      for ( ; runlength > 0; runlength--)
      {
        *q++=(*mark);
        mark+=4;
      }
    }
    mark=p;
    p+=4;
    while ((p < limit) && (*p == *mark))
      p+=4;
    count=((p-mark) >> 2);
    while (count)
    {
      runlength=count > 126 ? 126 : count;
      count-=runlength;
      *q++=runlength;
      *q++=(*mark);
    }
  }
  *q++=0;
  return(q-packets);
}

static unsigned int WriteSGIImage(ImageInfo *image_info, Image *image)
{
  typedef struct _SGIHeader
  {
    unsigned short
      magic;

    unsigned char
      storage,
      bytes_per_pixel;

    unsigned short
      dimension,
      columns,
      rows,
      depth;

    unsigned long
      minimum_value,
      maximum_value;

    unsigned char
      filler[492];
  } SGIHeader;

  SGIHeader
    iris_header;

  register int
    i,
    j,
    x,
    y,
    z;

  register RunlengthPacket
    *p;

  register unsigned char
    *q;

  unsigned char
    *iris_pixels,
    *packets;

  /*
    Open output image file.
  */
  OpenImage(image,WriteBinaryType);
  if (image->file == (FILE *) NULL)
    PrematureExit("Unable to open file",image);
  /*
    Initialize SGI raster file header.
  */
  iris_header.magic=0x01DA;
  if (image->compression == NoCompression)
    iris_header.storage=0x00;
  else
    iris_header.storage=0x01;
  iris_header.bytes_per_pixel=1;  /* one Byte per pixel */
  iris_header.dimension=3;
  iris_header.columns=image->columns;
  iris_header.rows=image->rows;
  iris_header.depth=image->matte ? 4 : 3;
  if (IsGrayImage(image))
    {
      iris_header.dimension=2;
      iris_header.depth=1;
    }
  iris_header.minimum_value=0;
  iris_header.maximum_value=MaxRGB;
  for (i=0; i < sizeof(iris_header.filler); i++)
    iris_header.filler[i]=0;
  /*
    Write SGI header.
  */
  MSBFirstWriteShort(iris_header.magic,image->file);
  (void) fputc(iris_header.storage,image->file);
  (void) fputc(iris_header.bytes_per_pixel,image->file);
  MSBFirstWriteShort(iris_header.dimension,image->file);
  MSBFirstWriteShort(iris_header.columns,image->file);
  MSBFirstWriteShort(iris_header.rows,image->file);
  MSBFirstWriteShort(iris_header.depth,image->file);
  MSBFirstWriteLong(iris_header.minimum_value,image->file);
  MSBFirstWriteLong(iris_header.maximum_value,image->file);
  (void) fwrite(iris_header.filler,1,sizeof(iris_header.filler),image->file);
  /*
    Allocate SGI pixels.
  */
  iris_pixels=(unsigned char *)
    malloc(4*image->columns*image->rows*sizeof(unsigned char));
  if (iris_pixels == (unsigned char *) NULL)
    PrematureExit("Unable to allocate memory",image);
  /*
    Convert runlength-encoded packets to uncompressed SGI pixels.
  */
  x=0;
  y=0;
  p=image->pixels;
  q=iris_pixels+(iris_header.rows-1)*(iris_header.columns*4);
  for (i=0; i < image->packets; i++)
  {
    for (j=0; j <= ((int) p->length); j++)
    {
      *q++=DownScale(p->red);
      *q++=DownScale(p->green);
      *q++=DownScale(p->blue);
      *q++=(unsigned char) p->index;
      x++;
      if (x == image->columns)
        {
          y++;
          q=iris_pixels+((iris_header.rows-1)-y)*(iris_header.columns*4);
          x=0;
        }
    }
    p++;
  }
  if (image->compression == NoCompression)
    {
      unsigned char
        *scanline;

      /*
        Write uncompressed SGI pixels.
      */
      scanline=(unsigned char *)
        malloc(iris_header.columns*sizeof(unsigned char));
      if (scanline == (unsigned char *) NULL)
        PrematureExit("Unable to allocate memory",image);
      for (z=0; z < (int) iris_header.depth; z++)
      {
        q=iris_pixels+z;
        for (y=0; y < (int) iris_header.rows; y++)
        {
          for (x=0; x < (int) iris_header.columns; x++)
          {
            scanline[x]=(*q);
            q+=4;
          }
          (void) fwrite(scanline,sizeof(unsigned char),iris_header.columns,
            image->file);
        }
        ProgressMonitor(SaveImageText,z,iris_header.depth);
      }
      free(scanline);
    }
  else
    {
      unsigned long
        length,
        number_packets,
        offset,
        *offsets,
        *runlength;

      /*
        Convert SGI uncompressed pixels to runlength-encoded pixels.
      */
      offsets=(unsigned long *)
        malloc(iris_header.rows*iris_header.depth*sizeof(unsigned long));
      packets=(unsigned char *) malloc(4*((iris_header.columns << 1)+10)*
        image->rows*sizeof(unsigned char));
      runlength=(unsigned long *)
        malloc(iris_header.rows*iris_header.depth*sizeof(unsigned long));
      if ((offsets == (unsigned long *) NULL) ||
          (packets == (unsigned char *) NULL) ||
          (runlength == (unsigned long *) NULL))
        PrematureExit("Unable to allocate memory",image);
      offset=512+4*((iris_header.rows*iris_header.depth) << 1);
      number_packets=0;
      q=iris_pixels;
      for (y=0; y < (int) iris_header.rows; y++)
      {
        for (z=0; z < (int) iris_header.depth; z++)
        {
          length=
            SGIEncode(q+z,(int) iris_header.columns,packets+number_packets);
          number_packets+=length;
          offsets[y+z*iris_header.rows]=offset;
          runlength[y+z*iris_header.rows]=length;
          offset+=length;
        }
        q+=(iris_header.columns*4);
        ProgressMonitor(SaveImageText,y,iris_header.rows);
      }
      /*
        Write out line start and length tables and runlength-encoded pixels.
      */
      for (i=0; i < (int) (iris_header.rows*iris_header.depth); i++)
        MSBFirstWriteLong(offsets[i],image->file);
      for (i=0; i < (int) (iris_header.rows*iris_header.depth); i++)
        MSBFirstWriteLong(runlength[i],image->file);
      (void) fwrite(packets,sizeof(unsigned char),number_packets,image->file);
      /*
        Free memory.
      */
      free(runlength);
      free(packets);
      free(offsets);
    }
  free(iris_pixels);
  CloseImage(image);
  return(True);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   W r i t e S U N I m a g e                                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Function WriteSUNImage writes an image in the SUN rasterfile format.
%
%  The format of the WriteSUNImage routine is:
%
%      status=WriteSUNImage(image_info,image)
%
%  A description of each parameter follows.
%
%    o status: Function WriteSUNImage return True if the image is written.
%      False is returned is there is a memory shortage or if the image file
%      fails to write.
%
%    o image_info: Specifies a pointer to an ImageInfo structure.
%
%    o image:  A pointer to a Image structure.
%
%
*/
static unsigned int WriteSUNImage(ImageInfo *image_info, Image *image)
{
#define RMT_EQUAL_RGB  1
#define RMT_NONE  0
#define RMT_RAW  2
#define RT_STANDARD  1
#define RT_FORMAT_RGB  3

  typedef struct _SUNHeader
  {
    unsigned long
      magic,
      width,
      height,
      depth,
      length,
      type,
      maptype,
      maplength;
  } SUNHeader;

  register int
    i,
    j,
    x;

  register RunlengthPacket
    *p;

  SUNHeader
    sun_header;

  /*
    Open output image file.
  */
  OpenImage(image,WriteBinaryType);
  if (image->file == (FILE *) NULL)
    PrematureExit("Unable to open file",image);
  /*
    Initialize SUN raster file header.
  */
  sun_header.magic=0x59a66a95;
  sun_header.width=image->columns;
  sun_header.height=image->rows;
  sun_header.type=(image->class == DirectClass ? RT_FORMAT_RGB : RT_STANDARD);
  sun_header.maptype=RMT_NONE;
  sun_header.maplength=0;
  if (!IsPseudoClass(image) && !IsGrayImage(image))
    {
      /*
        Full color SUN raster.
      */
      sun_header.depth=(image->matte ? 32 : 24);
      sun_header.length=image->columns*image->rows*(image->matte ? 4 : 3);
      sun_header.length+=image->columns % 2 ? image->rows : 0;
    }
  else
    if (IsGrayImage(image) && (image->colors == 2))
      {
        /*
          Monochrome SUN raster.
        */
        sun_header.depth=1;
        sun_header.length=((image->columns+7) >> 3)*image->rows;
        sun_header.length+=((image->columns/8)+(image->columns % 8 ? 1 : 0)) %
          2 ? image->rows : 0;
      }
    else
      {
        /*
          Colormapped SUN raster.
        */
        sun_header.depth=8;
        sun_header.length=image->columns*image->rows;
        sun_header.length+=image->columns % 2 ? image->rows : 0;
        sun_header.maptype=RMT_EQUAL_RGB;
        sun_header.maplength=image->colors*3;
      }
  /*
    Write SUN header.
  */
  MSBFirstWriteLong(sun_header.magic,image->file);
  MSBFirstWriteLong(sun_header.width,image->file);
  MSBFirstWriteLong(sun_header.height,image->file);
  MSBFirstWriteLong(sun_header.depth,image->file);
  MSBFirstWriteLong(sun_header.length,image->file);
  MSBFirstWriteLong(sun_header.type,image->file);
  MSBFirstWriteLong(sun_header.maptype,image->file);
  MSBFirstWriteLong(sun_header.maplength,image->file);
  /*
    Convert MIFF to SUN raster pixels.
  */
  p=image->pixels;
  x=0;
  if (!IsPseudoClass(image) && !IsGrayImage(image))
    {
      /*
        Convert DirectClass packet to SUN RGB pixel.
      */
      for (i=0; i < image->packets; i++)
      {
        for (j=0; j <= (int) p->length; j++)
        {
          if (image->matte)
            (void) fputc(p->index,image->file);
          (void) fputc(DownScale(p->red),image->file);
          (void) fputc(DownScale(p->green),image->file);
          (void) fputc(DownScale(p->blue),image->file);
          x++;
          if (x == image->columns)
            {
              if ((image->columns % 2) != 0)
                (void) fputc(0,image->file); /* pad scanline */
              x=0;
            }
        }
        p++;
        if (QuantumTick(i,image))
          ProgressMonitor(SaveImageText,i,image->packets);
      }
    }
  else
    if (IsGrayImage(image) && (image->colors == 2))
      {
        register unsigned char
          bit,
          Byte,
          polarity;

        /*
          Convert PseudoClass image to a SUN monochrome image.
        */
        polarity=Intensity(image->colormap[0]) > Intensity(image->colormap[1]);
        bit=0;
        Byte=0;
        for (i=0; i < image->packets; i++)
        {
          for (j=0; j <= (int) p->length; j++)
          {
            Byte<<=1;
            if (p->index == polarity)
              Byte|=0x01;
            bit++;
            if (bit == 8)
              {
                (void) fputc(Byte,image->file);
                bit=0;
                Byte=0;
              }
            x++;
            if (x == image->columns)
              {
                /*
                  Advance to the next scanline.
                */
                if (bit != 0)
                  (void) fputc(Byte << (8-bit),image->file);
                if ((((image->columns/8)+
                    (image->columns % 8 ? 1 : 0)) % 2) != 0)
                  (void) fputc(0,image->file);  /* pad scanline */
                bit=0;
                Byte=0;
                x=0;
             }
          }
          p++;
          if (QuantumTick(i,image))
            ProgressMonitor(SaveImageText,i,image->packets);
        }
      }
    else
      {
        /*
          Dump colormap to file.
        */
        for (i=0; i < image->colors; i++)
          (void) fputc(DownScale(image->colormap[i].red),image->file);
        for (i=0; i < image->colors; i++)
          (void) fputc(DownScale(image->colormap[i].green),image->file);
        for (i=0; i < image->colors; i++)
          (void) fputc(DownScale(image->colormap[i].blue),image->file);
        /*
          Convert PseudoClass packet to SUN colormapped pixel.
        */
        for (i=0; i < image->packets; i++)
        {
          for (j=0; j <= (int) p->length; j++)
          {
            (void) fputc(p->index,image->file);
            x++;
            if (x == image->columns)
              {
                if ((image->columns % 2) != 0)
                  (void) fputc(0,image->file);  /* pad scanline */
                x=0;
              }
          }
          p++;
          if (QuantumTick(i,image))
            ProgressMonitor(SaveImageText,i,image->packets);
        }
      }
  CloseImage(image);
  return(True);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   W r i t e T A R G A I m a g e                                             %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Function WriteTARGAImage writes a image in the Truevision Targa rasterfile
%  format.
%
%  The format of the WriteTARGAImage routine is:
%
%      status=WriteTARGAImage(image_info,image)
%
%  A description of each parameter follows.
%
%    o status: Function WriteTARGAImage return True if the image is written.
%      False is returned is there is a memory shortage or if the image file
%      fails to write.
%
%    o image_info: Specifies a pointer to an ImageInfo structure.
%
%    o image:  A pointer to a Image structure.
%
%
*/
static unsigned int WriteTARGAImage(ImageInfo *image_info, Image *image)
{
#define TargaColormap 1
#define TargaRGB 2
#define TargaMonochrome 3
#define TargaRLEColormap  9
#define TargaRLERGB  10
#define TargaRLEMonochrome  11

  typedef struct _TargaHeader
  {
    unsigned char
      id_length,
      colormap_type,
      image_type;

    unsigned short
      colormap_index,
      colormap_length;

    unsigned char
      colormap_size;

    unsigned short
      x_origin,
      y_origin,
      width,
      height;

    unsigned char
      pixel_size,
      attributes;
  } TargaHeader;

  Image
    *flopped_image;

  int
    count,
    runlength;

  register int
    i,
    j;

  register RunlengthPacket
    *p;

  register unsigned char
    *q,
    *r;

  TargaHeader
    targa_header;

  unsigned char
    *targa_pixels;

  /*
    Open output image file.
  */
  OpenImage(image,WriteBinaryType);
  if (image->file == (FILE *) NULL)
    PrematureExit("Unable to open file",image);
  /*
    Flop image.
  */
  image->orphan=True;
  flopped_image=FlopImage(image);
  image->orphan=False;
  if (flopped_image == (Image *) NULL)
    PrematureExit("Unable to flop image",image);
  if (flopped_image->compression == RunlengthEncodedCompression)
    CompressImage(flopped_image);
  /*
    Initialize TARGA raster file header.
  */
  targa_header.id_length=0;
  if (flopped_image->comments != (char *) NULL)
    targa_header.id_length=Min((int) strlen(flopped_image->comments),255);
  targa_header.colormap_type=0;
  targa_header.colormap_index=0;
  targa_header.colormap_length=0;
  targa_header.colormap_size=0;
  targa_header.x_origin=0;
  targa_header.y_origin=0;
  targa_header.width=flopped_image->columns;
  targa_header.height=flopped_image->rows;
  targa_header.pixel_size=8;
  targa_header.attributes=0;
  if (!IsPseudoClass(flopped_image))
    {
      /*
        Full color TARGA raster.
      */
      targa_header.image_type=TargaRGB;
      if (flopped_image->compression == RunlengthEncodedCompression)
        targa_header.image_type=TargaRLERGB;
      targa_header.pixel_size=flopped_image->matte ? 32 : 24;
    }
  else
    {
      /*
        Colormapped TARGA raster.
      */
      targa_header.image_type=TargaColormap;
      if (flopped_image->compression == RunlengthEncodedCompression)
        targa_header.image_type=TargaRLEColormap;
      if (!IsGrayImage(flopped_image) || (flopped_image->colors != 2))
        {
          targa_header.colormap_type=1;
          targa_header.colormap_index=0;
          targa_header.colormap_length=flopped_image->colors;
          targa_header.colormap_size=24;
        }
      else
        {
          /*
            Monochrome TARGA raster.
          */
          targa_header.image_type=TargaMonochrome;
          if (flopped_image->compression == RunlengthEncodedCompression)
            targa_header.image_type=TargaRLEMonochrome;
        }
    }
  /*
    Write TARGA header.
  */
  (void) fputc((char) targa_header.id_length,image->file);
  (void) fputc((char) targa_header.colormap_type,image->file);
  (void) fputc((char) targa_header.image_type,image->file);
  LSBFirstWriteShort(targa_header.colormap_index,image->file);
  LSBFirstWriteShort(targa_header.colormap_length,image->file);
  (void) fputc((char) targa_header.colormap_size,image->file);
  LSBFirstWriteShort(targa_header.x_origin,image->file);
  LSBFirstWriteShort(targa_header.y_origin,image->file);
  LSBFirstWriteShort(targa_header.width,image->file);
  LSBFirstWriteShort(targa_header.height,image->file);
  (void) fputc((char) targa_header.pixel_size,image->file);
  (void) fputc((char) targa_header.attributes,image->file);
  if (targa_header.id_length != 0)
    (void) fwrite((char *) flopped_image->comments,1,targa_header.id_length,
      image->file);
  /*
    Convert MIFF to TARGA raster pixels.
  */
  count=(unsigned int)
    (targa_header.pixel_size*targa_header.width*targa_header.height) >> 3;
  if (flopped_image->compression == RunlengthEncodedCompression)
    count+=(count/128)+1;
  targa_pixels=(unsigned char *) malloc(count*sizeof(unsigned char));
  if (targa_pixels == (unsigned char *) NULL)
    PrematureExit("Unable to allocate memory",flopped_image);
  p=flopped_image->pixels+(flopped_image->packets-1);
  q=targa_pixels;
  if (!IsPseudoClass(flopped_image))
    {
      /*
        Convert DirectClass packet to TARGA RGB pixel.
      */
      if (flopped_image->compression != RunlengthEncodedCompression)
        for (i=0; i < flopped_image->packets; i++)
        {
          for (j=0; j <= (int) p->length; j++)
          {
            *q++=DownScale(p->blue);
            *q++=DownScale(p->green);
            *q++=DownScale(p->red);
            if (flopped_image->matte)
              *q++=p->index;
          }
          p--;
          if (QuantumTick(i,flopped_image))
            ProgressMonitor(SaveImageText,i,flopped_image->packets);
        }
      else
        for (i=0; i < flopped_image->packets; i++)
        {
          for (runlength=p->length+1; runlength > 128; runlength-=128)
          {
            *q++=0xff;
            *q++=DownScale(p->blue);
            *q++=DownScale(p->green);
            *q++=DownScale(p->red);
            if (flopped_image->matte)
              *q++=p->index;
          }
          r=q;
          *q++=0x80+(runlength-1);
          *q++=DownScale(p->blue);
          *q++=DownScale(p->green);
          *q++=DownScale(p->red);
          if (flopped_image->matte)
            *q++=p->index;
          if (runlength != 1)
            p--;
          else
            {
              for ( ; i < flopped_image->packets; i++)
              {
                p--;
                if ((p->length != 0) || (runlength == 128))
                  break;
                *q++=DownScale(p->blue);
                *q++=DownScale(p->green);
                *q++=DownScale(p->red);
                if (flopped_image->matte)
                  *q++=p->index;
                runlength++;
              }
              *r=runlength-1;
            }
          if (QuantumTick(i,flopped_image))
            ProgressMonitor(SaveImageText,i,flopped_image->packets);
        }
    }
  else
    if (!IsGrayImage(flopped_image) || (flopped_image->colors != 2))
      {
        unsigned char
          *targa_colormap;

        /*
          Dump colormap to file (blue, green, red Byte order).
        */
        if (flopped_image->colors > 256)
          QuantizeImage(flopped_image,256,8,image_info->dither,RGBColorspace);
        targa_colormap=(unsigned char *)
          malloc(3*targa_header.colormap_length*sizeof(unsigned char));
        if (targa_colormap == (unsigned char *) NULL)
          PrematureExit("Unable to allocate memory",flopped_image);
        q=targa_colormap;
        for (i=0; i < flopped_image->colors; i++)
        {
          *q++=DownScale(flopped_image->colormap[i].blue);
          *q++=DownScale(flopped_image->colormap[i].green);
          *q++=DownScale(flopped_image->colormap[i].red);
        }
        (void) fwrite((char *) targa_colormap,1,
          (int) 3*targa_header.colormap_length,image->file);
        free((char *) targa_colormap);
        /*
          Convert PseudoClass packet to TARGA colormapped pixel.
        */
        q=targa_pixels;
        if (flopped_image->compression != RunlengthEncodedCompression)
          for (i=0; i < flopped_image->packets; i++)
          {
            for (j=0; j <= (int) p->length; j++)
              *q++=p->index;
            p--;
            if (QuantumTick(i,flopped_image))
              ProgressMonitor(SaveImageText,i,flopped_image->packets);
          }
        else
          for (i=0; i < flopped_image->packets; i++)
          {
            for (runlength=p->length+1; runlength > 128; runlength-=128)
            {
              *q++=0xff;
              *q++=p->index;
            }
            r=q;
            *q++=0x80+(runlength-1);
            *q++=p->index;
            if (runlength != 1)
              p--;
            else
              {
                for ( ; i < flopped_image->packets; i++)
                {
                  p--;
                  if ((p->length != 0) || (runlength == 128))
                    break;
                  *q++=p->index;
                  runlength++;
                }
                *r=runlength-1;
              }
            if (QuantumTick(i,flopped_image))
              ProgressMonitor(SaveImageText,i,flopped_image->packets);
          }
      }
    else
      {
        unsigned int
          polarity;

        /*
          Convert PseudoClass flopped_image to a TARGA monochrome flopped_image.
        */
        polarity=0;
        if (flopped_image->colors == 2)
          polarity=Intensity(flopped_image->colormap[0]) >
            Intensity(flopped_image->colormap[1]);
        if (flopped_image->compression != RunlengthEncodedCompression)
          for (i=0; i < flopped_image->packets; i++)
          {
            for (j=0; j <= (int) p->length; j++)
              *q++=p->index == polarity ? 0 : DownScale(MaxRGB);
            p--;
            if (QuantumTick(i,flopped_image))
              ProgressMonitor(SaveImageText,i,flopped_image->packets);
          }
        else
          for (i=0; i < flopped_image->packets; i++)
          {
            for (runlength=p->length+1; runlength > 128; runlength-=128)
            {
              *q++=0xff;
              *q++=p->index == polarity ? 0 : DownScale(MaxRGB);
            }
            r=q;
            *q++=0x80+(runlength-1);
            *q++=p->index == polarity ? 0 : DownScale(MaxRGB);
            if (runlength != 1)
              p--;
            else
              {
                for ( ; i < flopped_image->packets; i++)
                {
                  p--;
                  if ((p->length != 0) || (runlength == 128))
                    break;
                  *q++=p->index == polarity ? 0 : DownScale(MaxRGB);
                  runlength++;
                }
                *r=runlength-1;
              }
            if (QuantumTick(i,flopped_image))
              ProgressMonitor(SaveImageText,i,flopped_image->packets);
          }
      }
  (void) fwrite((char *) targa_pixels,1,(int) (q-targa_pixels),image->file);
  DestroyImage(flopped_image);
  free((char *) targa_pixels);
  CloseImage(image);
  return(True);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   W r i t e T E X T I m a g e                                               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Function WriteTEXTImage writes an image in the TEXT image forma.
%
%  The format of the WriteTEXTImage routine is:
%
%      status=WriteTEXTImage(image_info,image)
%
%  A description of each parameter follows.
%
%    o status: Function WriteTEXTImage return True if the image is written.
%      False is returned is there is a memory shortage or if the image file
%      fails to write.
%
%    o image_info: Specifies a pointer to an ImageInfo structure.
%
%    o image:  A pointer to a Image structure.
%
%
*/
static unsigned int WriteTEXTImage(ImageInfo *image_info, Image *image)
{
  unsigned int
    status;

  Warning("Cannot write TEXT images",image->filename);
  status=WriteMIFFImage(image_info,image);
  return(status);
}

#ifdef HasTIFF
#ifndef va_start
#if defined(__STDC__) && !defined(NOSTDHDRS)
#include <stdarg.h>
#else
#include <varargs.h>
#endif
#endif
#include "tiffio.h"

/* Kobus */
#define uint16 unsigned short
#define uint32 unsigned long
/* End Kobus */

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   W r i t e T I F F I m a g e                                               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Function WriteTIFFImage writes an image in the Tagged image file format.
%
%  The format of the WriteTIFFImage routine is:
%
%      status=WriteTIFFImage(image_info,image)
%
%  A description of each parameter follows:
%
%    o status:  Function WriteTIFFImage return True if the image is written.
%      False is returned is there is of a memory shortage or if the image
%      file cannot be opened for writing.
%
%    o image_info: Specifies a pointer to an ImageInfo structure.
%
%    o image:  A pointer to a Image structure.
%
%
*/
static unsigned int WriteTIFFImage(ImageInfo *image_info, Image *image)
{
#ifndef TIFFDefaultStripSize
#define TIFFDefaultStripSize(tiff,request)  ((8*1024)/TIFFScanlineSize(tiff))
#endif

  Image
    encode_image;

  int
    flags,
    sans_offset;

  register RunlengthPacket
    *p;

  register int
    i,
    j,
    x,
    y;

  register unsigned char
    *q;

  TIFF
    *tiff;

  uint16
    compression,
    photometric;

  unsigned char
    *scanline;

  unsigned int
    x_resolution,
    y_resolution;

  unsigned short
    value;

  /*
    Open TIFF file.
  */
  OpenImage(image,WriteBinaryType);
  if (image->file == (FILE *) NULL)
    PrematureExit("Unable to open file",image);
  if ((image->file != stdout) && !image->pipe)
    (void) unlink(image->filename);
  else
    {
      /*
        Write standard output or pipe to temporary file.
      */
      encode_image=(*image);
      TemporaryFilename(image->filename);
      image->temporary=True;
    }
  CloseImage(image);
  tiff=TIFFOpen(image->filename,WriteBinaryType);
  if (tiff == (TIFF *) NULL)
    return(False);
  /*
    Initialize TIFF fields.
  */
  TIFFSetField(tiff,TIFFTAG_DOCUMENTNAME,image->filename);
  TIFFSetField(tiff,TIFFTAG_SOFTWARE,"ImageMagick");
  if (image->comments != (char *) NULL)
    TIFFSetField(tiff,TIFFTAG_IMAGEDESCRIPTION,image->comments);
  TIFFSetField(tiff,TIFFTAG_BITSPERSAMPLE,8);
  if (image->depth == 16)
    TIFFSetField(tiff,TIFFTAG_BITSPERSAMPLE,16);
  if (!IsPseudoClass(image) && !IsGrayImage(image))
    {
      /*
        Full color TIFF raster.
      */
      photometric=PHOTOMETRIC_RGB;
      TIFFSetField(tiff,TIFFTAG_SAMPLESPERPIXEL,(image->matte ? 4 : 3));
      if (image->matte)
        {
          uint16
            extra_samples,
            sample_info[1];

          /*
            TIFF has a matte channel.
          */
          extra_samples=1;
          sample_info[0]=EXTRASAMPLE_ASSOCALPHA;
          TIFFSetField(tiff,TIFFTAG_EXTRASAMPLES,extra_samples,&sample_info);
        }
    }
  else
    {
      /*
        Colormapped TIFF raster.
      */
      photometric=PHOTOMETRIC_PALETTE;
      TIFFSetField(tiff,TIFFTAG_SAMPLESPERPIXEL,1);
      if (IsGrayImage(image))
        {
          /*
            Grayscale TIFF raster.
          */
          photometric=PHOTOMETRIC_MINISBLACK;
          if (image->colors == 2)
            TIFFSetField(tiff,TIFFTAG_BITSPERSAMPLE,1);
        }
    }
  TIFFSetField(tiff,TIFFTAG_PHOTOMETRIC,photometric);
  TIFFSetField(tiff,TIFFTAG_IMAGELENGTH,(uint32) image->rows);
  TIFFSetField(tiff,TIFFTAG_IMAGEWIDTH,(uint32) image->columns);
  TIFFSetField(tiff,TIFFTAG_FILLORDER,FILLORDER_MSB2LSB);
  TIFFSetField(tiff,TIFFTAG_ORIENTATION,ORIENTATION_TOPLEFT);
  TIFFSetField(tiff,TIFFTAG_PLANARCONFIG,PLANARCONFIG_CONTIG);
  TIFFSetField(tiff,TIFFTAG_RESOLUTIONUNIT,(uint16) image->units);
  flags=NoValue;
  if (image_info->density != (char *) NULL)
    flags=XParseGeometry(image_info->density,&sans_offset,&sans_offset,
      &x_resolution,&y_resolution);
  if (flags & WidthValue)
    image->x_resolution=x_resolution;
  if (flags & HeightValue)
    image->y_resolution=y_resolution;
  TIFFSetField(tiff,TIFFTAG_XRESOLUTION,image->x_resolution);
  TIFFSetField(tiff,TIFFTAG_YRESOLUTION,image->y_resolution);
  compression=COMPRESSION_LZW;
  if (!image->matte)
    if (IsGrayImage(image) && (image->colors == 2))
      compression=COMPRESSION_CCITTFAX4;
  if (image->compression == NoCompression)
    compression=COMPRESSION_NONE;
  TIFFSetField(tiff,TIFFTAG_COMPRESSION,compression);
  TIFFSetField(tiff,TIFFTAG_ROWSPERSTRIP,
    TIFFDefaultStripSize(tiff,(uint32) (-1)));
  scanline=(unsigned char *) malloc(TIFFScanlineSize(tiff));
  if (scanline == (unsigned char *) NULL)
    PrematureExit("Unable to allocate memory",image);
  p=image->pixels;
  q=scanline;
  x=0;
  y=0;
  if (photometric == PHOTOMETRIC_RGB)
    for (i=0; i < image->packets; i++)
    {
      for (j=0; j <= (int) p->length; j++)
      {
        /*
          Convert DirectClass packets to contiguous RGB scanlines.
        */
        WriteQuantum(p->red,q);
        WriteQuantum(p->green,q);
        WriteQuantum(p->blue,q);
        if (image->matte)
          WriteQuantum(p->index,q);
        x++;
        if (x == image->columns)
          {
            if (TIFFWriteScanline(tiff,(char *) scanline,y,0) < 0)
              break;
            q=scanline;
            x=0;
            y++;
          }
      }
      p++;
      if (QuantumTick(i,image))
        ProgressMonitor(SaveImageText,i,image->packets);
    }
  else
    if (photometric == PHOTOMETRIC_PALETTE)
      {
        unsigned short
          *blue,
          *green,
          *red;

        /*
          Allocate TIFF colormap.
        */
        blue=(unsigned short *) malloc(image->colors*sizeof(unsigned short));
        green=(unsigned short *) malloc(image->colors*sizeof(unsigned short));
        red=(unsigned short *) malloc(image->colors*sizeof(unsigned short));
        if ((blue == (unsigned short *) NULL) ||
            (green == (unsigned short *) NULL) ||
            (red == (unsigned short *) NULL))
          PrematureExit("Unable to allocate memory",image);
        /*
          Initialize TIFF colormap.
        */
        for (i=0; i < image->colors; i++)
        {
          red[i]=(unsigned int) (image->colormap[i].red*65535L)/MaxRGB;
          green[i]=(unsigned int) (image->colormap[i].green*65535L)/MaxRGB;
          blue[i]=(unsigned int) (image->colormap[i].blue*65535L)/MaxRGB;
        }
        TIFFSetField(tiff,TIFFTAG_COLORMAP,red,green,blue);
        /*
          Convert PseudoClass packets to contiguous colormap indexed scanlines.
        */
        for (i=0; i < image->packets; i++)
        {
          for (j=0; j <= (int) p->length; j++)
          {
            WriteQuantum(p->index,q);
            x++;
            if (x == image->columns)
              {
                if (TIFFWriteScanline(tiff,(char *) scanline,y,0) < 0)
                  break;
                q=scanline;
                x=0;
                y++;
              }
          }
          p++;
          if (QuantumTick(i,image))
            ProgressMonitor(SaveImageText,i,image->packets);
        }
        free((char *) red);
        free((char *) green);
        free((char *) blue);
      }
    else
      if (image->colors > 2)
        for (i=0; i < image->packets; i++)
        {
          for (j=0; j <= (int) p->length; j++)
          {
            /*
              Convert PseudoClass packets to contiguous grayscale scanlines.
            */
            WriteQuantum(Intensity(*p),q);
            x++;
            if (x == image->columns)
              {
                if (TIFFWriteScanline(tiff,(char *) scanline,y,0) < 0)
                  break;
                q=scanline;
                x=0;
                y++;
              }
          }
          p++;
          if (QuantumTick(i,image))
            ProgressMonitor(SaveImageText,i,image->packets);
        }
      else
        {
          register unsigned char
            bit,
            Byte,
            polarity;

          /*
            Convert PseudoClass packets to contiguous monochrome scanlines.
          */
          polarity=0;
          if (image->colors == 2)
            polarity=
              Intensity(image->colormap[0]) < Intensity(image->colormap[1]);
          bit=0;
          Byte=0;
          x=0;
          for (i=0; i < image->packets; i++)
          {
            for (j=0; j <= (int) p->length; j++)
            {
              Byte<<=1;
              if (p->index == polarity)
                Byte|=0x01;
              bit++;
              if (bit == 8)
                {
                  *q++=Byte;
                  bit=0;
                  Byte=0;
                }
              x++;
              if (x == image->columns)
                {
                  /*
                    Advance to the next scanline.
                  */
                  if (bit != 0)
                    *q++=Byte << (8-bit);
                  if (TIFFWriteScanline(tiff,(char *) scanline,y,0) < 0)
                    break;
                  q=scanline;
                  bit=0;
                  Byte=0;
                  x=0;
                  y++;
               }
            }
            p++;
            if (QuantumTick(i,image))
              ProgressMonitor(SaveImageText,i,image->packets);
          }
        }
  free((char *) scanline);
  (void) TIFFFlushData(tiff);
  if (image_info->verbose == True)
    TIFFPrintDirectory(tiff,stderr,False);
  (void) TIFFClose(tiff);
  if (image->temporary)
    {
      FILE
        *file;

      int
        c;

      /*
        Copy temporary file to standard output or pipe.
      */
      file=fopen(image->filename,ReadBinaryType);
      if (file == (FILE *) NULL)
        PrematureExit("Unable to open file",image);
      c=fgetc(file);
      while (c != EOF)
      {
        (void) putc(c,encode_image.file);
        c=fgetc(file);
      }
      (void) fclose(file);
      (void) unlink(image->filename);
      CloseImage(&encode_image);
    }
  return(True);
}
#else
static unsigned int WriteTIFFImage(ImageInfo* image_info, Image* image)
{
  unsigned int
    status;

  Warning("TIFF library is not available",image->filename);
  status=WriteMIFFImage(image_info,image);
  return(status);
}
#endif

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   W r i t e T I L E I m a g e                                               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Function WriteTILEImage writes an image in the TILE encoded image format.
%
%  The format of the WriteTILEImage routine is:
%
%      status=WriteTILEImage(image_info,image)
%
%  A description of each parameter follows.
%
%    o status: Function WriteTILEImage return True if the image is written.
%      False is returned is there is a memory shortage or if the image file
%      fails to write.
%
%    o image_info: Specifies a pointer to an ImageInfo structure.
%
%    o image:  A pointer to a Image structure.
%
%
*/
static unsigned int WriteTILEImage(ImageInfo *image_info, Image *image)
{
  unsigned int
    status;

  Warning("Cannot write TILE images",image->filename);
  status=WriteMIFFImage(image_info,image);
  return(status);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   W r i t e U Y V Y I m a g e                                               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Function WriteUYVYImage writes an image to a file in the digital UYVY
%  (16bit/pixel) format.  This format, used by AccomWSD, is not dramatically
%  higher quality than the 12bit/pixel YUV format, but has better locality.
%
%  The format of the WriteUYVYImage routine is:
%
%      status=WriteUYVYImage(image_info,image)
%
%  A description of each parameter follows.
%
%    o status: Function WriteUYVYImage return True if the image is written.
%      False is returned is there is a memory shortage or if the image file
%      fails to write.
%
%    o image_info: Specifies a pointer to an ImageInfo structure.
%
%    o image:  A pointer to a Image structure.
%      Implicit assumption: number of columns is even.
%
*/
static unsigned int WriteUYVYImage(ImageInfo *image_info, Image *image)
{
  register int
    i,
    j;

  register RunlengthPacket
    *p;

  unsigned int
    full,
    y,
    u,
    v;

  /*
    Open output image file.
  */
  if (!UncompressImage(image))
    return(False);
  OpenImage(image,WriteBinaryType);
  if (image->file == (FILE *) NULL)
    PrematureExit("Unable to open file",image);
  /*
    Convert to YUV, at full resolution.
  */
  RGBTransformImage(image,YCbCrColorspace);
  /*
    Accumulate two pixels, then output.
  */
  full=False;
  u=0;
  v=0;
  y=0;
  full=False;
  full=False;
  p=image->pixels;
  for (i=0; i < image->packets; i++)
  {
    for (j=0; j <= (int) p->length; j++)
    {
      if (full)
        {
          (void) fputc(DownScale((u+p->green) >> 1),image->file);
          (void) fputc(DownScale(y),image->file);
          (void) fputc(DownScale((v+p->blue) >> 1),image->file);
          (void) fputc(DownScale(p->red),image->file);
          full=False;
        }
      else
        {
          y=p->red;
          u=p->green;
          v=p->blue;
          full=True;
        }
    }
    p++;
    if (QuantumTick(i,image))
      ProgressMonitor(SaveImageText,i,image->packets);
  }
  TransformRGBImage(image,YCbCrColorspace);
  return(True);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   W r i t e V I C A R I m a g e                                             %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Function WriteVICARImage writes an image in the VICAR rasterfile format.
%  Vicar files contain a text header, followed by one or more planes of binary
%  grayscale image data.  Vicar files are designed to allow many planes to be
%  stacked together to form image cubes.  This routine only writes a single
%  grayscale plane.
%
%  Function WriteVICARImage was written contributed by
%  gorelick@esther.la.asu.edu.
%
%  The format of the WriteVICARImage routine is:
%
%      status=WriteVICARImage(image_info,image)
%
%  A description of each parameter follows.
%
%    o status: Function WriteVICARImage return True if the image is written.
%      False is returned is there is a memory shortage or if the image file
%      fails to write.
%
%    o image_info: Specifies a pointer to an ImageInfo structure.
%
%    o image:  A pointer to a Image structure.
%
%
*/
static unsigned int WriteVICARImage(ImageInfo *image_info, Image *image)
{
  char
    header[MaxTextLength],
    label[16];

  int
    label_size;

  register int
    i,
    j;

  register RunlengthPacket
    *p;

  /*
    Open output image file.
  */
  OpenImage(image,WriteBinaryType);
  if (image->file == (FILE *) NULL)
    PrematureExit("Unable to open file",image);
  /*
    Make a header.
  */
  (void) sprintf(header,"LBLSIZE=            FORMAT='BYTE'  TYPE='IMAGE'");
  (void) sprintf(header+strlen(header),"  BUFSIZE=20000  DIM=2  EOL=0");
  (void) sprintf(header+strlen(header),
    "  RECSIZE=%u  ORG='BSQ'  NL=%u  NS=%u  NB=1",image->columns,image->rows,
    image->columns);
  (void) sprintf(header+strlen(header),
    "  N1=0  N2=0  N3=0  N4=0  NBB=0  NLB=0");
  (void) sprintf(header+strlen(header),"  TASK='ImageMagick'");
  /*
    Compute the size of the label.
  */
  label_size=(strlen(header)+image->columns-1)/image->columns*image->columns;
  (void) sprintf(label,"%d",label_size);
  for (i=0 ; i < (int) strlen(label); i++)
    header[i+8]=label[i];
  /*
    Print the header and enough spaces to pad to label size.
  */
  (void) fprintf(image->file, "%-*s",label_size,header);
  /*
    Convert MIFF to VICAR raster pixels.
  */
  RGBTransformImage(image,GRAYColorspace);
  p=image->pixels;
  for (i=0; i < image->packets; i++)
  {
    for (j=0; j <= ((int) p->length); j++)
      (void) fputc(p->red,image->file);
    p++;
    if (QuantumTick(i,image))
      ProgressMonitor(SaveImageText,i,image->packets);
  }
  CloseImage(image);
  return(True);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   W r i t e V I F F I m a g e                                               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Function WriteVIFFImage writes an image to a file in the VIFF image format.
%
%  The format of the WriteVIFFImage routine is:
%
%      status=WriteVIFFImage(image_info,image)
%
%  A description of each parameter follows.
%
%    o status: Function WriteVIFFImage return True if the image is written.
%      False is returned is there is a memory shortage or if the image file
%      fails to write.
%
%    o image_info: Specifies a pointer to an ImageInfo structure.
%
%    o image:  A pointer to a Image structure.
%
%
*/
static unsigned int WriteVIFFImage(ImageInfo *image_info, Image *image)
{
#define VFF_CM_genericRGB  15
#define VFF_CM_NONE  0
#define VFF_DEP_IEEEORDER  0x2
#define VFF_DES_RAW  0
#define VFF_LOC_IMPLICIT  1
#define VFF_MAPTYP_NONE  0
#define VFF_MAPTYP_1_BYTE  1
#define VFF_MS_NONE  0
#define VFF_MS_ONEPERBAND  1
#define VFF_TYP_BIT  0
#define VFF_TYP_1_BYTE  1

  typedef struct _ViffHeader
  {
    char
      identifier,
      file_type,
      release,
      version,
      machine_dependency,
      reserve[3],
      comment[512];

    unsigned long
      rows,
      columns,
      subrows;

    long
      x_offset,
      y_offset;

    unsigned int
      x_pixel_size,
      y_pixel_size;

    unsigned long
      location_type,
      location_dimension,
      number_of_images,
      number_data_bands,
      data_storage_type,
      data_encode_scheme,
      map_scheme,
      map_storage_type,
      map_rows,
      map_columns,
      map_subrows,
      map_enable,
      maps_per_cycle,
      color_space_model;
  } ViffHeader;

  register int
    i,
    j;

  register RunlengthPacket
    *p;

  register unsigned char
    *q;

  unsigned char
    buffer[8],
    *viff_pixels;

  unsigned long
    packets;

  ViffHeader
    viff_header;

  /*
    Open output image file.
  */
  OpenImage(image,WriteBinaryType);
  if (image->file == (FILE *) NULL)
    PrematureExit("Unable to open file",image);
  /*
    Initialize VIFF image structure.
  */
  viff_header.identifier=0xab;
  viff_header.file_type=1;
  viff_header.release=1;
  viff_header.version=3;
  viff_header.machine_dependency=VFF_DEP_IEEEORDER;  /* IEEE Byte ordering */
  *viff_header.comment='\0';
  if (image->comments != (char *) NULL)
    {
      (void) strncpy(viff_header.comment,image->comments,
        Min((int) strlen(image->comments),511));
      viff_header.comment[Min((int) strlen(image->comments),511)]='\0';
    }
  viff_header.rows=image->columns;
  viff_header.columns=image->rows;
  viff_header.subrows=0;
  viff_header.x_offset=(~0);
  viff_header.y_offset=(~0);
  viff_header.x_pixel_size=0;
  viff_header.y_pixel_size=0;
  viff_header.location_type=VFF_LOC_IMPLICIT;
  viff_header.location_dimension=0;
  viff_header.number_of_images=1;
  viff_header.data_encode_scheme=VFF_DES_RAW;
  viff_header.map_scheme=VFF_MS_NONE;
  viff_header.map_storage_type=VFF_MAPTYP_NONE;
  viff_header.map_rows=0;
  viff_header.map_columns=0;
  viff_header.map_subrows=0;
  viff_header.map_enable=1;  /* no colormap */
  viff_header.maps_per_cycle=0;
  if (!IsPseudoClass(image) && !IsGrayImage(image))
    {
      /*
        Full color VIFF raster.
      */
      viff_header.number_data_bands=image->matte ? 4 : 3;
      viff_header.color_space_model=VFF_CM_genericRGB;
      viff_header.data_storage_type=VFF_TYP_1_BYTE;
      packets=image->columns*image->rows*viff_header.number_data_bands;
    }
  else
    {
      viff_header.number_data_bands=1;
      viff_header.color_space_model=VFF_CM_NONE;
      viff_header.data_storage_type=VFF_TYP_1_BYTE;
      packets=image->columns*image->rows;
      if (!IsGrayImage(image))
        {
          /*
            Colormapped VIFF raster.
          */
          viff_header.map_scheme=VFF_MS_ONEPERBAND;
          viff_header.map_storage_type=VFF_MAPTYP_1_BYTE;
          viff_header.map_rows=3;
          viff_header.map_columns=image->colors;
        }
      else
        if (image->colors == 2)
          {
            /*
              Monochrome VIFF raster.
            */
            viff_header.data_storage_type=VFF_TYP_BIT;
            packets=((image->columns+7) >> 3)*image->rows;
          }
    }
  /*
    Write VIFF image header (pad to 1024 bytes).
  */
  buffer[0]=viff_header.identifier;
  buffer[1]=viff_header.file_type;
  buffer[2]=viff_header.release;
  buffer[3]=viff_header.version;
  buffer[4]=viff_header.machine_dependency;
  buffer[5]=viff_header.reserve[0];
  buffer[6]=viff_header.reserve[1];
  buffer[7]=viff_header.reserve[2];
  (void) fwrite((char *) buffer,1,8,image->file);
  (void) fwrite((char *) viff_header.comment,1,512,image->file);
  MSBFirstWriteLong(viff_header.rows,image->file);
  MSBFirstWriteLong(viff_header.columns,image->file);
  MSBFirstWriteLong(viff_header.subrows,image->file);
  MSBFirstWriteLong((unsigned long) viff_header.x_offset,image->file);
  MSBFirstWriteLong((unsigned long) viff_header.y_offset,image->file);
  MSBFirstWriteLong((unsigned long) viff_header.x_pixel_size,image->file);
  MSBFirstWriteLong((unsigned long) viff_header.y_pixel_size,image->file);
  MSBFirstWriteLong(viff_header.location_type,image->file);
  MSBFirstWriteLong(viff_header.location_dimension,image->file);
  MSBFirstWriteLong(viff_header.number_of_images,image->file);
  MSBFirstWriteLong(viff_header.number_data_bands,image->file);
  MSBFirstWriteLong(viff_header.data_storage_type,image->file);
  MSBFirstWriteLong(viff_header.data_encode_scheme,image->file);
  MSBFirstWriteLong(viff_header.map_scheme,image->file);
  MSBFirstWriteLong(viff_header.map_storage_type,image->file);
  MSBFirstWriteLong(viff_header.map_rows,image->file);
  MSBFirstWriteLong(viff_header.map_columns,image->file);
  MSBFirstWriteLong(viff_header.map_subrows,image->file);
  MSBFirstWriteLong(viff_header.map_enable,image->file);
  MSBFirstWriteLong(viff_header.maps_per_cycle,image->file);
  MSBFirstWriteLong(viff_header.color_space_model,image->file);
  for (i=0; i < 420; i++)
    (void) fputc('\0',image->file);
  /*
    Convert MIFF to VIFF raster pixels.
  */
  viff_pixels=(unsigned char *) malloc(packets*sizeof(unsigned char));
  if (viff_pixels == (unsigned char *) NULL)
    PrematureExit("Unable to allocate memory",image);
  p=image->pixels;
  q=viff_pixels;
  if (!IsPseudoClass(image) && !IsGrayImage(image))
    {
      unsigned long
        offset;

      /*
        Convert DirectClass packet to VIFF RGB pixel.
      */
      offset=image->columns*image->rows;
      for (i=0; i < image->packets; i++)
      {
        for (j=0; j <= (int) p->length; j++)
        {
          *q=DownScale(p->red);
          *(q+offset)=DownScale(p->green);
          *(q+offset*2)=DownScale(p->blue);
          if (image->matte)
            *(q+offset*3)=(unsigned char) p->index;
          q++;
        }
        p++;
        if (QuantumTick(i,image))
          ProgressMonitor(SaveImageText,i,image->packets);
      }
    }
  else
    if (!IsGrayImage(image))
      {
        unsigned char
          *viff_colormap;

        /*
          Dump colormap to file.
        */
        viff_colormap=(unsigned char *)
          malloc(image->colors*3*sizeof(unsigned char));
        if (viff_colormap == (unsigned char *) NULL)
          PrematureExit("Unable to allocate memory",image);
        q=viff_colormap;
        for (i=0; i < image->colors; i++)
          *q++=DownScale(image->colormap[i].red);
        for (i=0; i < image->colors; i++)
          *q++=DownScale(image->colormap[i].green);
        for (i=0; i < image->colors; i++)
          *q++=DownScale(image->colormap[i].blue);
        (void) fwrite((char *) viff_colormap,1,(int) image->colors*3,
          image->file);
        free((char *) viff_colormap);
        /*
          Convert PseudoClass packet to VIFF colormapped pixels.
        */
        q=viff_pixels;
        for (i=0; i < image->packets; i++)
        {
          for (j=0; j <= (int) p->length; j++)
            *q++=p->index;
          p++;
          if (QuantumTick(i,image))
            ProgressMonitor(SaveImageText,i,image->packets);
        }
      }
    else
      if (image->colors == 2)
        {
          register unsigned char
            bit,
            Byte,
            polarity;

          register int
            x;

          /*
            Convert PseudoClass image to a VIFF monochrome image.
          */
          polarity=
            Intensity(image->colormap[0]) > Intensity(image->colormap[1]);
          x=0;
          bit=0;
          Byte=0;
          for (i=0; i < image->packets; i++)
          {
            for (j=0; j <= (int) p->length; j++)
            {
              Byte>>=1;
              if (p->index == polarity)
                Byte|=0x80;
              bit++;
              if (bit == 8)
                {
                  *q++=Byte;
                  bit=0;
                  Byte=0;
                }
              x++;
              if (x == image->columns)
                {
                  /*
                    Advance to the next scanline.
                  */
                  if (bit != 0)
                    *q++=Byte >> (8-bit);
                  bit=0;
                  Byte=0;
                  x=0;
               }
            }
            p++;
            if (QuantumTick(i,image))
              ProgressMonitor(SaveImageText,i,image->packets);
          }
        }
      else
        {
          /*
            Convert PseudoClass packet to VIFF grayscale pixel.
          */
          for (i=0; i < image->packets; i++)
          {
            for (j=0; j <= (int) p->length; j++)
              *q++=p->red;
            p++;
            if (QuantumTick(i,image))
              ProgressMonitor(SaveImageText,i,image->packets);
          }
        }
  (void) fwrite((char *) viff_pixels,1,(int) packets,image->file);
  free((char *) viff_pixels);
  CloseImage(image);
  return(True);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   W r i t e X I m a g e                                                     %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Function WriteXImage writes an image to an X server.
%
%  The format of the WriteXImage routine is:
%
%      status=WriteXImage(image_info,image)
%
%  A description of each parameter follows.
%
%    o status: Function WriteXImage return True if the image is displayed on
%      the X server.  False is returned is there is a memory shortage or if
%      the image file fails to write.
%
%    o image_info: Specifies a pointer to an ImageInfo structure.
%
%    o image:  A pointer to a Image structure.
%
%
*/
static unsigned int WriteXImage(ImageInfo *image_info, Image *image)
{
  Atom
    wm_delete_window,
    wm_protocols;

  char
    name[MaxTextLength];

  Display
    *display;

  register char
    *p;

  unsigned int
    status;

  XGCValues
    context_values;

  XResourceInfo
    resource_info;

  XrmDatabase
    resource_database;

  XEvent
    event;

  XPixelInfo
    pixel_info;

  XStandardColormap
    *map_info;

  XVisualInfo
    *visual_info;

  XWindowInfo
    window_info;

  /*
    Open X server connection.
  */
  display=XOpenDisplay(image_info->server_name);
  if (display == (Display *) NULL)
    {
      Warning("Unable to connect to X server",
        XDisplayName(image_info->server_name));
      return(False);
    }
  /*
    Set our forgiving error handler.
  */
  XSetErrorHandler(XError);
  /*
    Get user defaults from X resource database.
  */
  resource_database=XGetResourceDatabase(display,client_name);
  XGetResourceInfo(resource_database,client_name,&resource_info);
  /*
    Allocate standard colormap.
  */
  map_info=XAllocStandardColormap();
  if (map_info == (XStandardColormap *) NULL)
    Warning("Unable to create standard colormap","Memory allocation failed");
  /*
    Initialize visual info.
  */
  visual_info=XBestVisualInfo(display,map_info,&resource_info);
  if (visual_info == (XVisualInfo *) NULL)
    Warning("Unable to get visual",resource_info.visual_type);
  map_info->colormap=(Colormap) NULL;
  pixel_info.pixels=(unsigned long *) NULL;
  pixel_info.gamma_map=(XColor *) NULL;
  if ((map_info == (XStandardColormap *) NULL) ||
      (visual_info == (XVisualInfo *) NULL))
    {
      XFreeResources(display,visual_info,map_info,&pixel_info,
        (XFontStruct *) NULL,&resource_info,(XWindowInfo *) NULL);
      DestroyImage(image);
      return(False);
    }
  /*
    Initialize Standard Colormap.
  */
  ProgressMonitor(SaveImageText,100,400);
  XMakeStandardColormap(display,visual_info,&resource_info,image,map_info,
    &pixel_info);
  /*
    Initialize window info structure.
  */
  window_info.id=(Window) NULL;
  XGetWindowInfo(display,visual_info,map_info,&pixel_info,(XFontStruct *) NULL,
    &resource_info,&window_info);
  window_info.name=name;
  p=image->filename+strlen(image->filename)-1;
  while ((p > image->filename) && (*(p-1) != *BasenameSeparator))
    p--;
  (void) sprintf(window_info.name,"ImageMagick: %s[%u]",p,image->scene);
  if (image->scene == 0)
    (void) sprintf(window_info.name,"ImageMagick: %s",p);
  window_info.width=image->columns;
  window_info.height=image->rows;
  window_info.attributes.event_mask=ButtonPressMask | ExposureMask;
  XMakeWindow(display,XRootWindow(display,visual_info->screen),(char **) NULL,0,
    (XClassHint *) NULL,(XWMHints *) NULL,&window_info);
  window_info.x=0;
  window_info.y=0;
  window_info.shared_memory=False;
  /*
    Graphic context.
  */
  context_values.background=pixel_info.background_color.pixel;
  context_values.foreground=pixel_info.foreground_color.pixel;
  pixel_info.annotate_context=XCreateGC(display,window_info.id,GCBackground |
    GCForeground,&context_values);
  if (pixel_info.annotate_context == (GC) NULL)
    Error("Unable to create graphic context",(char *) NULL);
  window_info.annotate_context=pixel_info.annotate_context;
  context_values.background=pixel_info.foreground_color.pixel;
  context_values.foreground=pixel_info.background_color.pixel;
  pixel_info.highlight_context=XCreateGC(display,window_info.id,GCBackground |
    GCForeground,&context_values);
  if (pixel_info.annotate_context == (GC) NULL)
    Error("Unable to create graphic context",(char *) NULL);
  window_info.highlight_context=pixel_info.highlight_context;
  pixel_info.widget_context=(GC) NULL;
  window_info.widget_context=(GC) NULL;
  /*
    Initialize X image.
  */
  ProgressMonitor(SaveImageText,250,400);
  status=XMakeImage(display,&resource_info,&window_info,image,image->columns,
    image->rows);
  if (status == False)
    {
      XFreeResources(display,visual_info,map_info,&pixel_info,
        (XFontStruct *) NULL,&resource_info,&window_info);
      PrematureExit("Unable to make X image",image);
    }
  free((char *) image->pixels);
  image->pixels=(RunlengthPacket *) NULL;
  (void) XMakePixmap(display,&resource_info,&window_info);
  free(window_info.ximage->data);
  window_info.ximage->data=(char *) NULL;
  /*
    Display image and wait for button press to exit.
  */
  ProgressMonitor(SaveImageText,400,400);
  wm_protocols=XInternAtom(display,"WM_PROTOCOLS",False);
  wm_delete_window=XInternAtom(display,"WM_DELETE_WINDOW",False);
  XMapWindow(display,window_info.id);
  for ( ; ; )
  {
    XNextEvent(display,&event);
    if (event.type == ButtonPress)
      break;
    if (event.type == ClientMessage)
      if (event.xclient.message_type == wm_protocols)
        if (*event.xclient.data.l == wm_delete_window)
          if (event.xclient.window == window_info.id)
            break;
    if (event.type == Expose)
      XRefreshWindow(display,&window_info,&event);
  }
  XWithdrawWindow(display,window_info.id,window_info.screen);
  /*
    Free X resources.
  */
  XFreeResources(display,visual_info,map_info,&pixel_info,(XFontStruct *) NULL,
    &resource_info,&window_info);
  return(True);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   W r i t e X B M I m a g e                                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Procedure WriteXBMImage writes an image to a file in the X bitmap format.
%
%  The format of the WriteXBMImage routine is:
%
%      status=WriteXBMImage(image_info,image)
%
%  A description of each parameter follows.
%
%    o status: Function WriteXBMImage return True if the image is written.
%      False is returned is there is a memory shortage or if the image file
%      fails to write.
%
%    o image_info: Specifies a pointer to an ImageInfo structure.
%
%    o image:  A pointer to a Image structure.
%
%
*/
static unsigned int WriteXBMImage(ImageInfo *image_info, Image *image)
{
  char
    name[MaxTextLength];

  register int
    i,
    j,
    x;

  register char
    *q;

  register RunlengthPacket
    *p;

  register unsigned char
    bit,
    Byte,
    polarity;

  unsigned int
    count;

  /*
    Open output image file.
  */
  OpenImage(image,"w");
  if (image->file == (FILE *) NULL)
    PrematureExit("Unable to open file",image);
  /*
    Write X bitmap header.
  */
  (void) strcpy(name,image->filename);
  q=name;
  while ((*q != '.') && (*q != '\0'))
    q++;
  if (*q == '.')
    *q='\0';
  (void) fprintf(image->file,"#define %s_width %u\n",name,image->columns);
  (void) fprintf(image->file,"#define %s_height %u\n",name,image->rows);
  (void) fprintf(image->file,"static char %s_bits[] = {\n",name);
  (void) fprintf(image->file," ");
  /*
    Convert MIFF to X bitmap pixels.
  */
  if (!IsGrayImage(image) || (image->colors != 2))
    {
      QuantizeImage(image,2,8,image_info->dither,GRAYColorspace);
      SyncImage(image);
    }
  polarity=0;
  if (image->colors == 2)
    polarity=Intensity(image->colormap[0]) > Intensity(image->colormap[1]);
  bit=0;
  Byte=0;
  count=0;
  x=0;
  p=image->pixels;
  (void) fprintf(image->file," ");
  for (i=0; i < image->packets; i++)
  {
    for (j=0; j <= ((int) p->length); j++)
    {
      Byte>>=1;
      if (p->index == polarity)
        Byte|=0x80;
      bit++;
      if (bit == 8)
        {
          /*
            Write a bitmap Byte to the image file.
          */
          (void) fprintf(image->file,"0x%02x, ",(unsigned int) (Byte & 0xff));
          count++;
          if (count == 12)
            {
              (void) fprintf(image->file,"\n  ");
              count=0;
            };
          bit=0;
          Byte=0;
        }
      x++;
      if (x == image->columns)
        {
          if (bit != 0)
            {
              /*
                Write a bitmap Byte to the image file.
              */
              Byte>>=(8-bit);
              (void) fprintf(image->file,"0x%02x, ",(unsigned int)
                (Byte & 0xff));
              count++;
              if (count == 12)
                {
                  (void) fprintf(image->file,"\n  ");
                  count=0;
                };
              bit=0;
              Byte=0;
            };
          x=0;
        }
    }
    p++;
    if (QuantumTick(i,image))
      ProgressMonitor(SaveImageText,i,image->packets);
  }
  (void) fprintf(image->file,"};\n");
  CloseImage(image);
  return(True);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   W r i t e X C I m a g e                                                   %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Function WriteXCImage writes an image in the X constant image format.
%
%  The format of the WriteXCImage routine is:
%
%      status=WriteXCImage(image_info,image)
%
%  A description of each parameter follows.
%
%    o status: Function WriteXCImage return True if the image is written.
%      False is returned is there is a memory shortage or if the image file
%      fails to write.
%
%    o image_info: Specifies a pointer to an ImageInfo structure.
%
%    o image:  A pointer to a Image structure.
%
%
*/
static unsigned int WriteXCImage(ImageInfo *image_info, Image *image)
{
  unsigned int
    status;

  Warning("Cannot write XC images",image->filename);
  status=WriteMIFFImage(image_info,image);
  return(status);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   W r i t e X P M I m a g e                                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Procedure WriteXPMImage writes an image to a file in the X pixmap format.
%
%  The format of the WriteXPMImage routine is:
%
%      status=WriteXPMImage(image_info,image)
%
%  A description of each parameter follows.
%
%    o status: Function WriteXPMImage return True if the image is written.
%      False is returned is there is a memory shortage or if the image file
%      fails to write.
%
%    o image_info: Specifies a pointer to an ImageInfo structure.
%
%    o image:  A pointer to a Image structure.
%
%
*/
#ifdef HasXPM
#include "xpm.h"
static unsigned int WriteXPMImage(image_info,image)
ImageInfo
  *image_info;

Image
  *image;
{
  char
    *xpm_buffer;

  Display
    *display;

  int
    status;

  register char
    *p;

  XResourceInfo
    resource_info;

  XrmDatabase
    resource_database;

  XImage
    *matte_image;

  XPixelInfo
    pixel_info;

  XpmAttributes
    xpm_attributes;

  XStandardColormap
    *map_info;

  XVisualInfo
    *visual_info;

  XWindowInfo
    window_info;

  /*
    Open output image file.
  */
  OpenImage(image,"w");
  if (image->file == (FILE *) NULL)
    PrematureExit("Unable to open file",image);
  /*
    Open X server connection.
  */
  display=XOpenDisplay(image_info->server_name);
  if (display == (Display *) NULL)
    {
      Warning("Unable to connect to X server",
        XDisplayName(image_info->server_name));
      return(False);
    }
  /*
    Set our forgiving error handler.
  */
  XSetErrorHandler(XError);
  /*
    Get user defaults from X resource database.
  */
  resource_database=XGetResourceDatabase(display,client_name);
  XGetResourceInfo(resource_database,client_name,&resource_info);
  resource_info.colormap=PrivateColormap;
  /*
    Allocate standard colormap.
  */
  map_info=XAllocStandardColormap();
  if (map_info == (XStandardColormap *) NULL)
    Warning("Unable to create standard colormap","Memory allocation failed");
  /*
    Initialize visual info.
  */
  visual_info=XBestVisualInfo(display,map_info,&resource_info);
  if (visual_info == (XVisualInfo *) NULL)
    Warning("Unable to get visual",resource_info.visual_type);
  map_info->colormap=(Colormap) NULL;
  pixel_info.pixels=(unsigned long *) NULL;
  pixel_info.gamma_map=(XColor *) NULL;
  if ((map_info == (XStandardColormap *) NULL) ||
      (visual_info == (XVisualInfo *) NULL))
    {
      XFreeResources(display,visual_info,map_info,&pixel_info,
        (XFontStruct *) NULL,&resource_info,(XWindowInfo *) NULL);
      DestroyImage(image);
      return(False);
    }
  matte_image=(XImage *) NULL;
  if (image->matte)
    {
      /*
        Create X shape image.
      */
      matte_image=XCreateImage(display,visual_info->visual,1,XYBitmap,0,
        (char *) NULL,image->columns,image->rows,XBitmapPad(display),0);
      if (matte_image == (XImage *) NULL)
        Warning("Unable to create matte image",(char *) NULL);
      else
        {
          /*
            Allocate X shape image data.
          */
          matte_image->data=(char *)
            malloc(matte_image->bytes_per_line*matte_image->height);
          if (matte_image->data == (char *) NULL)
            Warning("Unable to create matte image","Memory allocation failed");
          else
            {
              int
                x,
                y;

              register int
                i,
                j;

              register RunlengthPacket
                *p;

              /*
                Initialize X shape image.
              */
              x=0;
              y=0;
              p=image->pixels;
              for (i=0; i < image->packets; i++)
              {
                for (j=0; j <= (int) p->length; j++)
                {
                  XPutPixel(matte_image,x,y,p->index == Transparent ? 0 : 1);
                  x++;
                  if (x == image->columns)
                    {
                      x=0;
                      y++;
                    }
                }
                p++;
              }
            }
        }
    }
  /*
    Initialize Standard Colormap.
  */
  ProgressMonitor(SaveImageText,100,400);
  if ((image->class == DirectClass) || (image->colors > 256))
    {
      /*
        Demote DirectClass to PseudoClass.
      */
      QuantizeImage(image,256,8,image_info->dither,RGBColorspace);
      SyncImage(image);
    }
  XMakeStandardColormap(display,visual_info,&resource_info,image,map_info,
    &pixel_info);
  pixel_info.annotate_context=(GC) NULL;
  pixel_info.highlight_context=(GC) NULL;
  pixel_info.widget_context=(GC) NULL;
  /*
    Initialize window info structure.
  */
  window_info.id=(Window) NULL;
  XGetWindowInfo(display,visual_info,map_info,&pixel_info,(XFontStruct *) NULL,
    &resource_info,&window_info);
  p=image->filename+strlen(image->filename)-1;
  while ((p > image->filename) && (*(p-1) != '/'))
    p--;
  window_info.name=p;
  window_info.width=image->columns;
  window_info.height=image->rows;
  XMakeWindow(display,XRootWindow(display,visual_info->screen),(char **) NULL,0,
    (XClassHint *) NULL,(XWMHints *) NULL,&window_info);
  window_info.shared_memory=False;
  /*
    Initialize X image.
  */
  ProgressMonitor(SaveImageText,250,400);
  status=XMakeImage(display,&resource_info,&window_info,image,image->columns,
    image->rows);
  if (status == False)
    {
      XFreeResources(display,visual_info,map_info,&pixel_info,
        (XFontStruct *) NULL,&resource_info,&window_info);
      PrematureExit("Unable to make X image",image);
    }
  /*
    Intialize XPM attributes.
  */
  xpm_attributes.valuemask=XpmColormap | XpmDepth | XpmSize | XpmVisual;
  xpm_attributes.visual=visual_info->visual;
  xpm_attributes.depth=visual_info->depth;
  xpm_attributes.colormap=map_info->colormap;
  xpm_attributes.width=image->columns;
  xpm_attributes.height=image->rows;
  status=XpmCreateBufferFromImage(display,&xpm_buffer,window_info.ximage,
    matte_image,&xpm_attributes);
  /*
    Free X resources.
  */
  if (matte_image != (XImage *) NULL)
    XDestroyImage(matte_image);
  XpmFreeAttributes(&xpm_attributes);
  XFreeResources(display,visual_info,map_info,&pixel_info,(XFontStruct *) NULL,
    &resource_info,&window_info);
  if (status != XpmSuccess)
    PrematureExit("Unable to write image",image);
  /*
    Write XPM image.
  */
  (void) fwrite((char *) xpm_buffer,1,strlen(xpm_buffer),image->file);
  free((char *) xpm_buffer);
  CloseImage(image);
  ProgressMonitor(SaveImageText,400,400);
  return(True);
}
#else
static unsigned int WriteXPMImage(ImageInfo *image_info, Image *image)
{
  unsigned int
    status;

  Warning("XPM library is not available",image->filename);
  status=WriteMIFFImage(image_info,image);
  return(status);
}
#endif

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   W r i t e Y U V I m a g e                                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Function WriteYUVImage writes an image to a file in the digital YUV
%  (CCIR 601 4:1:1) format.
%
%  The format of the WriteYUVImage routine is:
%
%      status=WriteYUVImage(image_info,image)
%
%  A description of each parameter follows.
%
%    o status: Function WriteYUVImage return True if the image is written.
%      False is returned is there is a memory shortage or if the image file
%      fails to write.
%
%    o image_info: Specifies a pointer to an ImageInfo structure.
%
%    o image:  A pointer to a Image structure.
%
%
*/
static unsigned int WriteYUVImage(ImageInfo *image_info, Image *image)
{
  Image
    *reduced_image;

  register int
    i,
    j;

  register RunlengthPacket
    *p;

  /*
    Open output image file.
  */
  OpenImage(image,WriteBinaryType);
  if (image->file == (FILE *) NULL)
    PrematureExit("Unable to open file",image);
  /*
    Initialize Y channel.
  */
  RGBTransformImage(image,YCbCrColorspace);
  p=image->pixels;
  for (i=0; i < image->packets; i++)
  {
    for (j=0; j <= (int) p->length; j++)
      (void) fputc(DownScale(p->red),image->file);
    p++;
  }
  /*
    Scale image.
  */
  ProgressMonitor(SaveImageText,100,400);
  image->orphan=True;
  reduced_image=MinifyImage(image);
  image->orphan=False;
  if (reduced_image == (Image *) NULL)
    PrematureExit("Unable to scale image",image);
  /*
    Initialize U channel.
  */
  ProgressMonitor(SaveImageText,200,400);
  p=reduced_image->pixels;
  for (i=0; i < reduced_image->packets; i++)
  {
    for (j=0; j <= (int) p->length; j++)
      (void) fputc(DownScale(p->green),image->file);
    p++;
  }
  /*
    Initialize V channel.
  */
  ProgressMonitor(SaveImageText,400,400);
  p=reduced_image->pixels;
  for (i=0; i < reduced_image->packets; i++)
  {
    for (j=0; j <= (int) p->length; j++)
      (void) fputc(DownScale(p->blue),image->file);
    p++;
  }
  DestroyImage(reduced_image);
  TransformRGBImage(image,YCbCrColorspace);
  ProgressMonitor(SaveImageText,400,400);
  return(True);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   W r i t e Y U V 3 I m a g e                                               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Function WriteYUV3Image writes an image to a file in the digital YUV
%  (CCIR 601 4:1:1) format.  This function differs from WriteYUVImage in that
%  it write the Y, U, and V planes to separate files (image.Y, image.U, and
%  image.V).
%
%  The format of the WriteYUV3Image routine is:
%
%      status=WriteYUV3Image(image_info,image)
%
%  A description of each parameter follows.
%
%    o status: Function WriteYUV3Image return True if the image is written.
%      False is returned is there is a memory shortage or if the image file
%      fails to write.
%
%    o image_info: Specifies a pointer to an ImageInfo structure.
%
%    o image:  A pointer to a Image structure.
%
%
*/
static unsigned int WriteYUV3Image(ImageInfo *image_info, Image *image)
{
  char
    filename[MaxTextLength];

  Image
    *reduced_image;

  register int
    i,
    j;

  register RunlengthPacket
    *p;

  /*
    Open output image file.
  */
  (void) strcpy(filename,image->filename);
  if (strcmp(image->filename,"-") != 0)
    (void) strcat(image->filename,".Y");
  OpenImage(image,WriteBinaryType);
  if (image->file == (FILE *) NULL)
    PrematureExit("Unable to open file",image);
  /*
    Write Y channel.
  */
  RGBTransformImage(image,YCbCrColorspace);
  p=image->pixels;
  for (i=0; i < image->packets; i++)
  {
    for (j=0; j <= (int) p->length; j++)
      (void) fputc(DownScale(p->red),image->file);
    p++;
  }
  CloseImage(image);
  /*
    Scale image.
  */
  ProgressMonitor(SaveImageText,100,400);
  image->orphan=True;
  reduced_image=MinifyImage(image);
  image->orphan=False;
  if (reduced_image == (Image *) NULL)
    PrematureExit("Unable to reduce image",image);
  /*
    Write U channel.
  */
  ProgressMonitor(SaveImageText,200,400);
  (void) strcpy(reduced_image->filename,filename);
  if (strcmp(reduced_image->filename,"-") != 0)
    (void) strcat(reduced_image->filename,".U");
  OpenImage(reduced_image,WriteBinaryType);
  if (reduced_image->file == (FILE *) NULL)
    {
      DestroyImage(reduced_image);
      PrematureExit("Unable to open file",reduced_image);
    }
  p=reduced_image->pixels;
  for (i=0; i < reduced_image->packets; i++)
  {
    for (j=0; j <= (int) p->length; j++)
      (void) fputc(DownScale(p->green),reduced_image->file);
    p++;
  }
  CloseImage(reduced_image);
  /*
    Write V channel.
  */
  ProgressMonitor(SaveImageText,300,400);
  (void) strcpy(reduced_image->filename,filename);
  if (strcmp(reduced_image->filename,"-") != 0)
    (void) strcat(reduced_image->filename,".V");
  OpenImage(reduced_image,WriteBinaryType);
  if (reduced_image->file == (FILE *) NULL)
    {
      DestroyImage(reduced_image);
      PrematureExit("Unable to open file",reduced_image);
    }
  p=reduced_image->pixels;
  for (i=0; i < reduced_image->packets; i++)
  {
    for (j=0; j <= (int) p->length; j++)
      (void) fputc(DownScale(p->blue),reduced_image->file);
    p++;
  }
  CloseImage(reduced_image);
  DestroyImage(reduced_image);
  TransformRGBImage(image,YCbCrColorspace);
  (void) sprintf(image->filename,filename);
  ProgressMonitor(SaveImageText,400,400);
  return(True);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   W r i t e X W D I m a g e                                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Function WriteXWDImage writes an image to a file in X window dump
%  rasterfile format.
%
%  The format of the WriteXWDImage routine is:
%
%      status=WriteXWDImage(image_info,image)
%
%  A description of each parameter follows.
%
%    o status: Function WriteXWDImage return True if the image is written.
%      False is returned is there is a memory shortage or if the image file
%      fails to write.
%
%    o image_info: Specifies a pointer to an ImageInfo structure.
%
%    o image:  A pointer to a Image structure.
%
%
*/
static unsigned int WriteXWDImage(ImageInfo *image_info, Image *image)
{
  int
    x;

  register int
    i,
    j,
    k;

  register RunlengthPacket
    *p;

  unsigned int
    bits_per_pixel,
    bytes_per_line,
    scanline_pad;

  unsigned long
    lsb_first;

  XWDFileHeader
    xwd_header;

  /*
    Open output image file.
  */
  OpenImage(image,WriteBinaryType);
  if (image->file == (FILE *) NULL)
    PrematureExit("Unable to open file",image);
  /*
    Initialize XWD file header.
  */
  xwd_header.header_size=sz_XWDheader+strlen(image->filename)+1;
  xwd_header.file_version=(unsigned long) XWD_FILE_VERSION;
  xwd_header.pixmap_format=(unsigned long) ZPixmap;
  xwd_header.pixmap_depth=(unsigned long)
    (image->class == DirectClass ? 24 : 8);
  xwd_header.pixmap_width=(unsigned long) image->columns;
  xwd_header.pixmap_height=(unsigned long) image->rows;
  xwd_header.xoffset=(unsigned long) 0;
  xwd_header.byte_order=(unsigned long) MSBFirst;
  xwd_header.bitmap_unit=(unsigned long) (image->class == DirectClass ? 32 : 8);
  xwd_header.bitmap_bit_order=(unsigned long) MSBFirst;
  xwd_header.bitmap_pad=(unsigned long) (image->class == DirectClass ? 32 : 8);
  bits_per_pixel=(image->class == DirectClass ? 24 : 8);
  xwd_header.bits_per_pixel=(unsigned long) bits_per_pixel;
  bytes_per_line=(unsigned long) ((((xwd_header.bits_per_pixel*
    xwd_header.pixmap_width)+((xwd_header.bitmap_pad)-1))/
    (xwd_header.bitmap_pad))*((xwd_header.bitmap_pad) >> 3));
  xwd_header.bytes_per_line=(unsigned long) bytes_per_line;
  xwd_header.visual_class=(unsigned long)
    (image->class == DirectClass ? DirectColor : PseudoColor);
  xwd_header.red_mask=(unsigned long)
    (image->class == DirectClass ? 0xff0000 : 0);
  xwd_header.green_mask=(unsigned long)
    (image->class == DirectClass ? 0xff00 : 0);
  xwd_header.blue_mask=(unsigned long) (image->class == DirectClass ? 0xff : 0);
  xwd_header.bits_per_rgb=(unsigned long)
    (image->class == DirectClass ? 24 : 8);
  xwd_header.colormap_entries=(unsigned long)
    (image->class == DirectClass ? 256 : image->colors);
  xwd_header.ncolors=(image->class == DirectClass ? 0 : image->colors);
  xwd_header.window_width=(unsigned long) image->columns;
  xwd_header.window_height=(unsigned long) image->rows;
  xwd_header.window_x=0;
  xwd_header.window_y=0;
  xwd_header.window_bdrwidth=(unsigned long) 0;
  /*
    Write XWD header.
  */
  lsb_first=1;
  if (*(char *) &lsb_first)
    MSBFirstOrderLong((char *) &xwd_header,sizeof(xwd_header));
  (void) fwrite((char *) &xwd_header,sz_XWDheader,1,image->file);
  (void) fwrite((char *) image->filename,1,strlen(image->filename)+1,
    image->file);
  if (image->class == PseudoClass)
    {
      XColor
        *colors;

      XWDColor
        color;

      /*
        Dump colormap to file.
      */
      colors=(XColor *) malloc(image->colors*sizeof(XColor));
      if (colors == (XColor *) NULL)
        PrematureExit("Unable to allocate memory",image);
      for (i=0; i < image->colors; i++)
      {
        colors[i].pixel=i;
        colors[i].red=XUpScale(image->colormap[i].red);
        colors[i].green=XUpScale(image->colormap[i].green);
        colors[i].blue=XUpScale(image->colormap[i].blue);
        colors[i].flags=DoRed | DoGreen | DoBlue;
        colors[i].pad=0;
        if (*(char *) &lsb_first)
          {
            MSBFirstOrderLong((char *) &colors[i].pixel,sizeof(long));
            MSBFirstOrderShort((char *) &colors[i].red,3*sizeof(short));
          }
      }
      for (i=0; i < image->colors; i++)
      {
        color.pixel=colors[i].pixel;
        color.red=colors[i].red;
        color.green=colors[i].green;
        color.blue=colors[i].blue;
        color.flags=colors[i].flags;
        (void) fwrite((char *) &color,sz_XWDColor,1,image->file);
      }
      free((char *) colors);
    }
  /*
    Convert MIFF to XWD raster pixels.
  */
  scanline_pad=(unsigned int)
    (bytes_per_line-((image->columns*bits_per_pixel) >> 3));
  x=0;
  p=image->pixels;
  for (i=0; i < image->packets; i++)
  {
    for (j=0; j <= ((int) p->length); j++)
    {
      if (image->class == PseudoClass)
        (void) fputc(p->index,image->file);
      else
        {
          (void) fputc(DownScale(p->red),image->file);
          (void) fputc(DownScale(p->green),image->file);
          (void) fputc(DownScale(p->blue),image->file);
        }
      x++;
      if (x == image->columns)
        {
          for (k=0; k < scanline_pad; k++)
            (void) fputc(0,image->file);
          x=0;
        }
    }
    p++;
    if (QuantumTick(i,image))
      ProgressMonitor(SaveImageText,i,image->packets);
  }
  CloseImage(image);
  return(True);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   W r i t e I m a g e                                                       %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Function WriteImage writes an image to a file.  You can specify a
%  particular image format by prefixing the file with the image type and a
%  colon (i.e. ps:image) or specify the image type as the filename suffix
%  (i.e. image.ps).
%
%  The format of the WriteImage routine is:
%
%      status=WriteImage(image_info,image)
%
%  A description of each parameter follows:
%
%    o status: Function WriteImage return True if the image is written.
%      False is returned is there is a memory shortage or if the image file
%      fails to write.
%
%    o image_info: Specifies a pointer to an ImageInfo structure.
%
%    o image: A pointer to a Image structure.
%
%
*/
unsigned int WriteImage(ImageInfo *image_info, Image *image)
{
  unsigned int
    status;

  /*
    Call appropriate image writer based on image type.
  */
  (void) strcpy(image_info->filename,image->filename);
  (void) strcpy(image_info->magick,image->magick);
  SetImageMagick(image_info);
  (void) strcpy(image->filename,image_info->filename);
  switch (*image_info->magick)
  {
    case 'A':
    {
      status=WriteAVSImage(image_info,image);
      break;
    }
    case 'B':
    {
      if (strcmp(image_info->magick,"BIE") == 0)
        status=WriteJBIGImage(image_info,image);
      else
        status=WriteBMPImage(image_info,image);
      break;
    }
    case 'C':
    {
      status=WriteCMYKImage(image_info,image);
      break;
    }
    case 'E':
    {
      status=WritePSImage(image_info,image);
      break;
    }
    case 'F':
    {
      if (strcmp(image_info->magick,"FAX") == 0)
        status=WriteFAXImage(image_info,image);
      else
        status=WriteFITSImage(image_info,image);
      break;
    }
    case 'G':
    {
      if (strncmp(image_info->magick,"GIF",3) == 0)
        status=WriteGIFImage(image_info,image);
      else
        if (strcmp(image_info->magick,"GRAY") == 0)
          status=WriteGRAYImage(image_info,image);
        else
          status=WriteFAXImage(image_info,image);
      break;
    }
    case 'H':
    {
      if (strcmp(image_info->magick,"HDF") == 0)
        status=WriteHDFImage(image_info,image);

      /* Kobus */
      else if (strcmp(image_info->magick,"HIPS") == 0)
          status=WriteHIPSImage(image_info, image);
      /* End Kobus */

      else
        if (strcmp(image_info->magick,"HISTOGRAM") == 0)
          status=WriteHISTOGRAMImage(image_info,image);
        else
          status=WriteHTMLImage(image_info,image);
      break;
    }
    case 'J':
    {
      if (strcmp(image_info->magick,"JBIG") == 0)
        status=WriteJBIGImage(image_info,image);
      else
        status=WriteJPEGImage(image_info,image);
      break;
    }
    case 'L':
    {
      status=WriteLOGOImage(image_info,image);
      break;
    }
    case 'M':
    {
      if (strcmp(image_info->magick,"MAP") == 0)
        status=WriteMAPImage(image_info,image);
      else
        if (strcmp(image_info->magick,"MATTE") == 0)
          status=WriteMATTEImage(image_info,image);
        else
          if (strcmp(image_info->magick,"MIFF") == 0)
            status=WriteMIFFImage(image_info,image);
          else
            if ((strcmp(image_info->magick,"MPEG") == 0) ||
                (strcmp(image_info->magick,"MPG") == 0))
              status=WriteMPEGImage(image_info,image);
            else
              status=WriteMTVImage(image_info,image);
      break;
    }
    case 'N':
      break;
    case 'P':
    {
      if (strcmp(image_info->magick,"PCD") == 0)
        status=WritePCDImage(image_info,image);
      else
        if (strcmp(image_info->magick,"PCX") == 0)
          status=WritePCXImage(image_info,image);
        else
          if (strcmp(image_info->magick,"PDF") == 0)
            status=WritePDFImage(image_info,image);
          else
            if (strcmp(image_info->magick,"PICT") == 0)
              status=WritePICTImage(image_info,image);
            else
              if (strcmp(image_info->magick,"PM") == 0)
                status=WriteXPMImage(image_info,image);
              else
                if (strcmp(image_info->magick,"PNG") == 0)
                  status=WritePNGImage(image_info,image);
                else
                  if (strcmp(image_info->magick,"PS") == 0)
                    status=WritePSImage(image_info,image);
                  else
                    if (strcmp(image_info->magick,"PS2") == 0)
                      status=WritePS2Image(image_info,image);
                    else
                      status=WritePNMImage(image_info,image);
      break;
    }
    case 'R':
    {
      if (strcmp(image_info->magick,"RAD") == 0)
        status=WriteRADIANCEImage(image_info,image);
      else
        if (strcmp(image_info->magick,"RAS") == 0)
          status=WriteSUNImage(image_info,image);
        else
          if (strncmp(image_info->magick,"RGB",3) == 0)
            status=WriteRGBImage(image_info,image);
          else
            status=WriteRLEImage(image_info,image);
      break;
    }
    case 'S':
    {
      if (strcmp(image_info->magick,"SGI") == 0)
        status=WriteSGIImage(image_info,image);
      else
        status=WriteSUNImage(image_info,image);
      break;
    }
    case 'T':
    {
      if (strcmp(image_info->magick,"TEXT") == 0)
        status=WriteTEXTImage(image_info,image);
      else
        if (strcmp(image_info->magick,"TGA") == 0)
          status=WriteTARGAImage(image_info,image);
        else
          if (strncmp(image_info->magick,"TIF",3) == 0)
            status=WriteTIFFImage(image_info,image);
          else
            status=WriteTILEImage(image_info,image);
      break;
    }
    case 'U':
    {
      if (strcmp(image_info->magick,"UYVY") == 0)
        status=WriteUYVYImage(image_info,image);
      break;
    }
    case 'V':
    {
      if (strcmp(image_info->magick,"VICAR") == 0)
        status=WriteVICARImage(image_info,image);
      else
        if (strcmp(image_info->magick,"VID") == 0)
          status=WriteMIFFImage(image_info,image);
        else
          status=WriteVIFFImage(image_info,image);
      break;
    }
    case 'X':
    {
      if (strcmp(image_info->magick,"X") == 0)
        status=WriteXImage(image_info,image);
      else
        if (strcmp(image_info->magick,"XBM") == 0)
          status=WriteXBMImage(image_info,image);
        else
          if (strcmp(image_info->magick,"XC") == 0)
            status=WriteXCImage(image_info,image);
          else
            if (strcmp(image_info->magick,"XPM") == 0)
              status=WriteXPMImage(image_info,image);
            else
              if (strcmp(image_info->magick,"XV") == 0)
                status=WriteVIFFImage(image_info,image);
              else
                status=WriteXWDImage(image_info,image);
      break;
    }
    case 'Y':
    {
      if (strcmp(image_info->magick,"YUV") == 0)
        status=WriteYUVImage(image_info,image);
      else
        status=WriteYUV3Image(image_info,image);
      break;
    }
    default:
      status=WriteMIFFImage(image_info,image);
  }
  if (image->status)
    {
      Warning("An error has occurred writing to file",image->filename);
      return(False);
    }
  (void) strcpy(image->magick,image_info->magick);
  return(status);
}

/*
   Kobus

   Note: This module is last because the HIPS typedefs "Byte" which is used
   as a variable in the code.
*/

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   W r i t e H I P S I m a g e                                               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Function WriteHIPSImage writes an image to a file.
%
%  The format of the WriteHIPSImage routine is:
%
%      status=WriteHIPSImage(image_info,image)
%
%  A description of each parameter follows:
%
%    o status: Function WriteHIPSImage return True if the image is written.
%      False is returned if there is a memory shortage or if the image file
%      fails to write.
%
%    o image_info: Specifies a pointer to an ImageInfo structure.
%
%    o image: A pointer to a Image structure.
%
%
*/

#ifdef HasHIPS

#include "hips/hipl_format.h"

static unsigned int WriteHIPSImage(image_info,image)

ImageInfo
  *image_info;

Image
  *image;
{
  register int
    i;

  unsigned long
    packets;

  struct header hd;
  int save_hipserrlev, save_hipserrprt;


  Progname = strsave("ImageMagick");

  /*
    Open output image file.
  */
  OpenImage(image,"wb");
  if (image->file == (FILE *) NULL)
    {
      Warning("Unable to open file",image->filename);
      return(False);
    }

  image->compression = NoCompression;
  image->class=DirectClass;

  (void) RunlengthEncodeImage(image);

  hd.orig_name = "Exported by ImageMagick";
  hd.seq_name = "";
  hd.num_frame = 1;
  hd.orig_date = "";
  hd.orows = image->rows;
  hd.ocols = image->columns;
  hd.rows = image->rows;
  hd.cols = image->columns;
  hd.frow = 0;
  hd.fcol = 0;
  hd.pixel_format = PFRGB;
  hd.numcolor = 1;

  /*
     Can't be zero length string due to defect in hips code.
  */
  hd.sizehist = 6;
  hd.seq_history = "Dummy\n";

  /*
     Can't be zero length string due to defect in hips code.
  */
  hd.sizedesc = 6;
  hd.seq_desc = "Dummy\n";

  hd.ondealloc = TRUE;
  hd.sndealloc = TRUE;
  hd.oddealloc = TRUE;
  hd.numpix = (hd.rows)*(hd.cols);
  hd.sizepix = 3;
  hd.sizeimage = (hd.sizepix)*(hd.numpix);
  hd.image = (Byte*)image->packed_pixels;
  hd.imdealloc = TRUE;
  hd.firstpix = (Byte*)image->packed_pixels;
  hd.histdealloc = TRUE;
  hd.seqddealloc = TRUE;
  hd.numparam = 0;
  hd.paramdealloc = TRUE;
  hd.params = NULL;


  save_hipserrlev = hipserrlev;
  save_hipserrprt = hipserrprt;
  hipserrlev = hipserrprt = 10000;   /* Big Number */

  if (fwrite_header(image->file, &hd, image->filename) != HIPS_OK) {
      Warning("Unable to write hips header",image->filename);

      (void) free((char *) image->packed_pixels);
      image->packed_pixels=(unsigned char *) NULL;
      CloseImage(image);

      return(False);
    }

  if (fwrite_image(image->file, &hd, 0, image->filename) != HIPS_OK) {
      Warning("Unable to write hips image",image->filename);

      (void) free((char *) image->packed_pixels);
      image->packed_pixels=(unsigned char *) NULL;
      CloseImage(image);

      return(False);
    }

  hipserrlev = save_hipserrlev;
  hipserrprt = save_hipserrprt;

  (void) free((char *) image->packed_pixels);
  image->packed_pixels=(unsigned char *) NULL;

  CloseImage(image);
  return(True);
}

#else
static unsigned int WriteHIPSImage(ImageInfo *image_info, Image *image)
{
  unsigned int
    status;

  Warning("HIPS format is not available",image->filename);
  status=WriteMIFFImage(image_info,image);
  return(status);
}
#endif

/* End Kobus */

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

#ifdef __cplusplus
}
#endif


/* -------------------------------------------------------------------------- */

#endif   /* #ifdef KJB_HAVE_X11  */

#endif   /* #ifndef __C2MAN__  */

