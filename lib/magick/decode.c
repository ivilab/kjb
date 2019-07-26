
/* $Id: decode.c 21596 2017-07-30 23:33:36Z kobus $ */

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
%                                                                             %
%                   DDDD   EEEEE   CCCC   OOO   DDDD   EEEEE                  %
%                   D   D  E      C      O   O  D   D  E                      %
%                   D   D  EEE    C      O   O  D   D  EEE                    %
%                   D   D  E      C      O   O  D   D  E                      %
%                   DDDD   EEEEE   CCCC   OOO   DDDD   EEEEE                  %
%                                                                             %
%                                                                             %
%                    Utility Routines to Read Image Formats                   %
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
#define LoadImageText  "  Loading image...  "
#define PrematureExit(message,image) \
{ \
  Warning(message,image->filename); \
  DestroyImages(image); \
  return((Image *) NULL); \
}

/*
  Function prototypes.
*/
Image
  *ReadImage _Declare((ImageInfo *));

static Image
/* Kobus: Forward declare here, as ReadHIPSImage definition is at the end of
   this file. It is at the end because the HIPS include files typedef "Byte"
   which is used as a variable in some of the code.
*/
  *ReadHIPSImage _Declare((ImageInfo *)),
/* End Kobus */

  *OverviewImage _Declare((ImageInfo *,Image *,unsigned int)),
  *ReadAVSImage _Declare((ImageInfo *)),
  *ReadBMPImage _Declare((ImageInfo *)),
  *ReadCMYKImage _Declare((ImageInfo *)),
  *ReadDPSImage _Declare((ImageInfo *)),
  *ReadFAXImage _Declare((ImageInfo *)),
  *ReadFITSImage _Declare((ImageInfo *)),
  *ReadGIFImage _Declare((ImageInfo *)),
  *ReadGRAYImage _Declare((ImageInfo *)),
  *ReadHDFImage _Declare((ImageInfo *)),
  *ReadHISTOGRAMImage _Declare((ImageInfo *)),
  *ReadHTMLImage _Declare((ImageInfo *)),
  *ReadJBIGImage _Declare((ImageInfo *)),
  *ReadJPEGImage _Declare((ImageInfo *)),
  *ReadLOGOImage _Declare((ImageInfo *)),
  *ReadMAPImage _Declare((ImageInfo *)),
  *ReadMATTEImage _Declare((ImageInfo *)),
  *ReadMIFFImage _Declare((ImageInfo *)),
  *ReadMPEGImage _Declare((ImageInfo *)),
  *ReadMTVImage _Declare((ImageInfo *)),
  *ReadNULLImage _Declare((ImageInfo *)),
  *ReadPCDImage _Declare((ImageInfo *)),
  *ReadPCXImage _Declare((ImageInfo *)),
  *ReadPDFImage _Declare((ImageInfo *)),
  *ReadPICTImage _Declare((ImageInfo *)),
  *ReadPNGImage _Declare((ImageInfo *)),
  *ReadPNMImage _Declare((ImageInfo *)),
  *ReadPSImage _Declare((ImageInfo *)),
  *ReadRADIANCEImage _Declare((ImageInfo *)),
  *ReadRGBImage _Declare((ImageInfo *)),
  *ReadRLEImage _Declare((ImageInfo *)),
  *ReadSGIImage _Declare((ImageInfo *)),
  *ReadSUNImage _Declare((ImageInfo *)),
  *ReadTARGAImage _Declare((ImageInfo *)),
  *ReadTEXTImage _Declare((ImageInfo *)),
  *ReadTIFFImage _Declare((ImageInfo *)),
  *ReadTILEImage _Declare((ImageInfo *)),
  *ReadUYVYImage _Declare((ImageInfo *)),
  *ReadVICARImage _Declare((ImageInfo *)),
  *ReadVIDImage _Declare((ImageInfo *)),
  *ReadVIFFImage _Declare((ImageInfo *)),
  *ReadXBMImage _Declare((ImageInfo *)),
  *ReadXCImage _Declare((ImageInfo *)),
  *ReadXPMImage _Declare((ImageInfo *)),
  *ReadXWDImage _Declare((ImageInfo *)),
  *ReadYUVImage _Declare((ImageInfo *)),
  *ReadYUV3Image _Declare((ImageInfo *));

static int
  XBMInteger _Declare((FILE *,short int *));

static unsigned int
  PNMInteger _Declare((Image *,unsigned int));

static void
  SGIDecode _Declare((unsigned char *,unsigned char *)),
  Upsample _Declare((unsigned int, unsigned int, unsigned int,unsigned char *));

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%  R e a d A V S I m a g e                                                    %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Function ReadAVSImage reads a AVS X image file and returns it.  It
%  allocates the memory necessary for the new Image structure and returns a
%  pointer to the new image.
%
%  The format of the ReadAVSImage routine is:
%
%      image=ReadAVSImage(image_info)
%
%  A description of each parameter follows:
%
%    o image:  Function ReadAVSImage returns a pointer to the image after
%      reading. A null image is returned if there is a a memory shortage or if
%      the image cannot be read.
%
%    o image_info: Specifies a pointer to an ImageInfo structure.
%
%
*/
static Image *ReadAVSImage(ImageInfo *image_info)
{
  typedef struct _AVSHeader
  {
    int
      width,
      height;
  } AVSHeader;

  AVSHeader
    avs_header;

  Image
    *image;

  Quantum
    blue,
    green,
    red;

  register int
    x,
    y;

  register RunlengthPacket
    *q;

  unsigned int
    packets,
    status;

  unsigned short
    index;

  /*
    Allocate image structure.
  */
  image=AllocateImage(image_info);
  if (image == (Image *) NULL)
    return((Image *) NULL);
  /*
    Open image file.
  */
  OpenImage(image,ReadBinaryType);
  if (image->file == (FILE *) NULL)
    PrematureExit("Unable to open file",image);
  /*
    Read AVS image.
  */
  status=ReadData((char *) &avs_header,1,(unsigned int) sizeof(AVSHeader),
    image->file);
  if (status == False)
    PrematureExit("Not a AVS image file",image);
  do
  {
    /*
      Initialize image structure.
    */
    image->matte=True;
    image->columns=avs_header.width;
    image->rows=avs_header.height;
    image->packets=0;
    packets=Max((image->columns*image->rows+4) >> 3,1);
    image->pixels=(RunlengthPacket *) malloc(packets*sizeof(RunlengthPacket));
    if (image->pixels == (RunlengthPacket *) NULL)
      PrematureExit("Unable to allocate memory",image);
    /*
      Convert AVS raster image to runlength-encoded packets.
    */
    q=image->pixels;
    q->length=MaxRunlength;
    for (y=0; y < image->rows; y++)
    {
      for (x=0; x < image->columns; x++)
      {
        index=Opaque-fgetc(image->file);
        red=UpScale(fgetc(image->file));
        green=UpScale(fgetc(image->file));
        blue=UpScale(fgetc(image->file));
        if ((red == q->red) && (green == q->green) && (blue == q->blue) &&
            (index == q->index) && ((int) q->length < MaxRunlength))
          q->length++;
        else
          {
            if (image->packets != 0)
              q++;
            image->packets++;
            if (image->packets == packets)
              {
                packets<<=1;
                image->pixels=(RunlengthPacket *) realloc((char *)
                  image->pixels,packets*sizeof(RunlengthPacket));
                if (image->pixels == (RunlengthPacket *) NULL)
                  PrematureExit("Unable to allocate memory",image);
                q=image->pixels+image->packets-1;
              }
            q->red=red;
            q->green=green;
            q->blue=blue;
            q->index=index;
            q->length=0;
          }
      }
      ProgressMonitor(LoadImageText,y,image->rows);
    }
    image->pixels=(RunlengthPacket *)
      realloc((char *) image->pixels,image->packets*sizeof(RunlengthPacket));
    status=ReadData((char *) &avs_header,1,(unsigned int) sizeof(AVSHeader),
      image->file);
    if (status == True)
      {
        /*
          Allocate image structure.
        */
        image->next=AllocateImage(image_info);
        if (image->next == (Image *) NULL)
          {
            DestroyImages(image);
            return((Image *) NULL);
          }
        (void) strcpy(image->next->filename,image_info->filename);
        image->next->file=image->file;
        image->next->scene=image->scene+1;
        image->next->previous=image;
        image=image->next;
      }
  } while (status == True);
  while (image->previous != (Image *) NULL)
    image=image->previous;
  CloseImage(image);
  return(image);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%  R e a d B M P I m a g e                                                    %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Function ReadBMPImage reads a Microsoft Windows bitmap image file and
%  returns it.  It allocates the memory necessary for the new Image structure
%  and returns a pointer to the new image.
%
%  The format of the ReadBMPImage routine is:
%
%      image=ReadBMPImage(image_info)
%
%  A description of each parameter follows:
%
%    o image:  Function ReadBMPImage returns a pointer to the image after
%      reading.  A null image is returned if there is a a memory shortage or
%      if the image cannot be read.
%
%    o image_info: Specifies a pointer to an ImageInfo structure.
%
%
*/
static Image *ReadBMPImage(ImageInfo *image_info)
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

  Image
    *image;

  register int
    bit,
    i,
    x,
    y;

  register RunlengthPacket
    *q;

  register unsigned char
    *p;

  unsigned char
    *bmp_data,
    *bmp_pixels,
    magick[12];

  unsigned int
    bytes_per_line,
    start_position,
    status;

  /*
    Allocate image structure.
  */
  image=AllocateImage(image_info);
  if (image == (Image *) NULL)
    return((Image *) NULL);
  /*
    Open image file.
  */
  OpenImage(image,ReadBinaryType);
  if (image->file == (FILE *) NULL)
    PrematureExit("Unable to open file",image);
  /*
    Determine if this is a BMP file.
  */
  status=ReadData((char *) magick,1,2,image->file);
  do
  {
    /*
      Verify BMP identifier.
    */
    start_position=ftell(image->file)-2;
    if ((status == False) || (strncmp((char *) magick,"BM",2) != 0))
      PrematureExit("Not a BMP image file",image);
    bmp_header.file_size=LSBFirstReadLong(image->file);
    bmp_header.reserved[0]=LSBFirstReadShort(image->file);
    bmp_header.reserved[1]=LSBFirstReadShort(image->file);
    bmp_header.offset_bits=LSBFirstReadLong(image->file);
    bmp_header.size=LSBFirstReadLong(image->file);
    if (bmp_header.size == 12)
      {
        /*
          OS/2 BMP image file.
        */
        bmp_header.width=(unsigned long) LSBFirstReadShort(image->file);
        bmp_header.height=(unsigned long) LSBFirstReadShort(image->file);
        bmp_header.planes=LSBFirstReadShort(image->file);
        bmp_header.bit_count=LSBFirstReadShort(image->file);
        bmp_header.number_colors=0;
        bmp_header.compression=0;
        bmp_header.image_size=0;
      }
    else
      {
        /*
          Microsoft Windows BMP image file.
        */
        bmp_header.width=LSBFirstReadLong(image->file);
        bmp_header.height=LSBFirstReadLong(image->file);
        bmp_header.planes=LSBFirstReadShort(image->file);
        bmp_header.bit_count=LSBFirstReadShort(image->file);
        bmp_header.compression=LSBFirstReadLong(image->file);
        bmp_header.image_size=LSBFirstReadLong(image->file);
        bmp_header.x_pixels=LSBFirstReadLong(image->file);
        bmp_header.y_pixels=LSBFirstReadLong(image->file);
        bmp_header.number_colors=LSBFirstReadLong(image->file);
        bmp_header.colors_important=LSBFirstReadLong(image->file);
        for (i=0; i < ((int) bmp_header.size-40); i++)
          (void) fgetc(image->file);
      }
    if (bmp_header.bit_count < 24)
      {
        unsigned char
          *bmp_colormap;

        unsigned int
          packet_size;

        /*
          Read BMP raster colormap.
        */
        image->class=PseudoClass;
        image->colors=bmp_header.number_colors;
        if (image->colors == 0)
          image->colors=1 << bmp_header.bit_count;
        image->colormap=(ColorPacket *)
          malloc(image->colors*sizeof(ColorPacket));
        bmp_colormap=(unsigned char *)
          malloc(4*image->colors*sizeof(unsigned char));
        if ((image->colormap == (ColorPacket *) NULL) ||
            (bmp_colormap == (unsigned char *) NULL))
          PrematureExit("Unable to allocate memory",image);
        packet_size=4;
        if (bmp_header.size == 12)
          packet_size=3;
        (void) ReadData((char *) bmp_colormap,packet_size,image->colors,
          image->file);
        p=bmp_colormap;
        for (i=0; i < image->colors; i++)
        {
          image->colormap[i].blue=UpScale(*p++);
          image->colormap[i].green=UpScale(*p++);
          image->colormap[i].red=UpScale(*p++);
          if (bmp_header.size != 12)
            p++;
        }
        free((char *) bmp_colormap);
      }
    /*
      Read image data.
    */
    while (ftell(image->file) < (start_position+bmp_header.offset_bits))
      (void) fgetc(image->file);
    if (bmp_header.image_size == 0)
      bmp_header.image_size=
        ((bmp_header.width*bmp_header.bit_count+31)/32)*4*bmp_header.height;
    bmp_data=(unsigned char *)
      malloc(bmp_header.image_size*sizeof(unsigned char));
    if (bmp_data == (unsigned char *) NULL)
      PrematureExit("Unable to allocate memory",image);
    (void) ReadData((char *) bmp_data,1,(unsigned int) bmp_header.image_size,
      image->file);
    bmp_pixels=bmp_data;
    if (bmp_header.compression != 0)
      {
        unsigned int
          packets;

        /*
          Convert run-length encoded raster pixels.
        */
        packets=
          ((bmp_header.width*bmp_header.bit_count+31)/32)*4*bmp_header.height;
        if (bmp_header.compression == 2)
          packets<<=1;
        bmp_pixels=(unsigned char *) malloc(packets*sizeof(unsigned char));
        if (bmp_pixels == (unsigned char *) NULL)
          PrematureExit("Unable to allocate memory",image);
        (void) BMPDecodeImage(bmp_data,bmp_pixels,
          (unsigned int) bmp_header.compression,(unsigned int) bmp_header.width,
          (unsigned int) bmp_header.height);
        if (bmp_header.compression == 2)
          bmp_header.bit_count<<=1;
        free((char *) bmp_data);
      }
    /*
      Initialize image structure.
    */
    image->columns=bmp_header.width;
    image->rows=bmp_header.height;
    image->packets=image->columns*image->rows;
    image->pixels=(RunlengthPacket *)
      malloc(image->packets*sizeof(RunlengthPacket));
    if (image->pixels == (RunlengthPacket *) NULL)
      PrematureExit("Unable to allocate memory",image);
    /*
      Convert BMP raster image to runlength-encoded packets.
    */
    bytes_per_line=((image->columns*bmp_header.bit_count+31)/32)*4;
    switch (bmp_header.bit_count)
    {
      case 1:
      {
        /*
          Convert bitmap scanline to runlength-encoded color packets.
        */
        for (y=image->rows-1; y >= 0; y--)
        {
          p=bmp_pixels+(image->rows-y-1)*bytes_per_line;
          q=image->pixels+(y*image->columns);
          for (x=0; x < (image->columns-7); x+=8)
          {
            for (bit=0; bit < 8; bit++)
            {
              q->index=((*p) & (0x80 >> bit) ? 0x01 : 0x00);
              q->length=0;
              q++;
            }
            p++;
          }
          if ((image->columns % 8) != 0)
            {
              for (bit=0; bit < (image->columns % 8); bit++)
              {
                q->index=((*p) & (0x80 >> bit) ? 0x01 : 0x00);
                q->length=0;
                q++;
              }
              p++;
            }
          ProgressMonitor(LoadImageText,image->rows-y,image->rows);
        }
        SyncImage(image);
        break;
      }
      case 4:
      {
        /*
          Convert PseudoColor scanline to runlength-encoded color packets.
        */
        for (y=image->rows-1; y >= 0; y--)
        {
          p=bmp_pixels+(image->rows-y-1)*bytes_per_line;
          q=image->pixels+(y*image->columns);
          for (x=0; x < (image->columns-1); x+=2)
          {
            q->index=(*p >> 4) & 0xf;
            q->length=0;
            q++;
            q->index=(*p) & 0xf;
            q->length=0;
            p++;
            q++;
          }
          if ((image->columns % 2) != 0)
            {
              q->index=(*p >> 4) & 0xf;
              q->length=0;
              q++;
              p++;
            }
          ProgressMonitor(LoadImageText,image->rows-y,image->rows);
        }
        SyncImage(image);
        CompressColormap(image);
        break;
      }
      case 8:
      {
        /*
          Convert PseudoColor scanline to runlength-encoded color packets.
        */
        if (bmp_header.compression == 1)
          bytes_per_line=image->columns;
        for (y=image->rows-1; y >= 0; y--)
        {
          p=bmp_pixels+(image->rows-y-1)*bytes_per_line;
          q=image->pixels+(y*image->columns);
          for (x=0; x < image->columns; x++)
          {
            q->index=(*p++);
            q->length=0;
            q++;
          }
          ProgressMonitor(LoadImageText,image->rows-y,image->rows);
        }
        SyncImage(image);
        CompressColormap(image);
        break;
      }
      case 24:
      {
        /*
          Convert DirectColor scanline to runlength-encoded color packets.
        */
        for (y=image->rows-1; y >= 0; y--)
        {
          p=bmp_pixels+(image->rows-y-1)*bytes_per_line;
          q=image->pixels+(y*image->columns);
          for (x=0; x < image->columns; x++)
          {
            q->index=(unsigned short) (image->matte ? (*p++) : 0);
            q->blue=UpScale(*p++);
            q->green=UpScale(*p++);
            q->red=UpScale(*p++);
            q->length=0;
            q++;
          }
          ProgressMonitor(LoadImageText,image->rows-y,image->rows);
        }
        break;
      }
      default:
        PrematureExit("Not a BMP image file",image);
    }
    free((char *) bmp_pixels);
    CompressImage(image);
    /*
      Proceed to next image.
    */
    status=ReadData((char *) magick,1,2,image->file);
    if ((status == True) && (strncmp((char *) magick,"BM",2) == 0))
      {
        /*
          Allocate image structure.
        */
        image->next=AllocateImage(image_info);
        if (image->next == (Image *) NULL)
          {
            DestroyImages(image);
            return((Image *) NULL);
          }
        (void) strcpy(image->next->filename,image_info->filename);
        image->next->file=image->file;
        image->next->scene=image->scene+1;
        image->next->previous=image;
        image=image->next;
      }
  } while ((status == True) && (strncmp((char *) magick,"BM",2) == 0));
  while (image->previous != (Image *) NULL)
    image=image->previous;
  CloseImage(image);
  return(image);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%  R e a d C M Y K I m a g e                                                  %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Function ReadCMYKImage reads an image of raw cyan, magenta, yellow, and
%  black bytes and returns it.  It allocates the memory necessary for the new
%  Image structure and returns a pointer to the new image.
%
%  The format of the ReadCMYKImage routine is:
%
%      image=ReadCMYKImage(image_info)
%
%  A description of each parameter follows:
%
%    o image:  Function ReadCMYKImage returns a pointer to the image after
%      reading.  A null image is returned if there is a a memory shortage or
%      if the image cannot be read.
%
%    o image_info: Specifies a pointer to an ImageInfo structure.
%
%
*/
static Image *ReadCMYKImage(ImageInfo *image_info)
{
  Image
    *image;

  int
    x,
    y;

  register int
    i;

  register RunlengthPacket
    *q;

  register unsigned char
    *p;

  unsigned char
    *cmyk_pixels;

  unsigned int black,
    cyan,
    height,
    magenta,
    yellow,
    width;

  unsigned short
    value;

  /*
    Allocate image structure.
  */
  image=AllocateImage(image_info);
  if (image == (Image *) NULL)
    return((Image *) NULL);
  /*
    Open image file.
  */
  OpenImage(image,ReadBinaryType);
  if (image->file == (FILE *) NULL)
    PrematureExit("Unable to open file",image);
  /*
    Determine width and height, e.g. 640x512.
  */
  width=512;
  height=512;
  x=0;
  if (image_info->size != (char *) NULL)
    (void) XParseGeometry(image_info->size,&x,&y,&width,&height);
  for (i=0; i < x; i++)
    (void) fgetc(image->file);
  /*
    Initialize image structure.
  */
  image->columns=width;
  image->rows=height;
  image->packets=image->columns*image->rows;
  cmyk_pixels=(unsigned char *) malloc(4*image->packets*sizeof(unsigned char));
  image->pixels=(RunlengthPacket *)
    malloc(image->packets*sizeof(RunlengthPacket));
  if ((cmyk_pixels == (unsigned char *) NULL) ||
      (image->pixels == (RunlengthPacket *) NULL))
    PrematureExit("Unable to allocate memory",image);
  /*
    Convert raster image to runlength-encoded packets.
  */
  (void) ReadData((char *) cmyk_pixels,4,image->packets,image->file);
  p=cmyk_pixels;
  switch (image_info->interlace)
  {
    case NoneInterlace:
    default:
    {
      /*
        No interlacing:  CMYKCMYKCMYKCMYKCMYKCMYK...
      */
      q=image->pixels;
      for (y=0; y < image->rows; y++)
      {
        for (x=0; x < image->columns; x++)
        {
          ReadQuantum(q->red,p);
          ReadQuantum(q->green,p);
          ReadQuantum(q->blue,p);
          ReadQuantum(q->index,p);
          q->length=0;
          q++;
        }
        ProgressMonitor(LoadImageText,y,image->rows);
      }
      break;
    }
    case LineInterlace:
    {
      /*
        Line interlacing:  CCC...MMM...YYY...KKK...CCC...MMM...YYY...KKK...
      */
      for (y=0; y < image->rows; y++)
      {
        q=image->pixels+y*image->columns;
        for (x=0; x < image->columns; x++)
        {
          ReadQuantum(q->red,p);
          q->length=0;
          q++;
        }
        q=image->pixels+y*image->columns;
        for (x=0; x < image->columns; x++)
        {
          ReadQuantum(q->green,p);
          q++;
        }
        q=image->pixels+y*image->columns;
        for (x=0; x < image->columns; x++)
        {
          ReadQuantum(q->blue,p);
          q++;
        }
        q=image->pixels+y*image->columns;
        for (x=0; x < image->columns; x++)
        {
          ReadQuantum(q->index,p);
          q++;
        }
        ProgressMonitor(LoadImageText,y,image->rows);
      }
      break;
    }
    case PlaneInterlace:
    {
      /*
        Plane interlacing:  CCCCCC...MMMMMM...YYYYYY...KKKKKK...
      */
      q=image->pixels;
      for (i=0; i < image->packets; i++)
      {
        ReadQuantum(q->red,p);
        q->length=0;
        q++;
      }
      ProgressMonitor(LoadImageText,100,400);
      q=image->pixels;
      for (i=0; i < image->packets; i++)
      {
        ReadQuantum(q->green,p);
        q++;
      }
      ProgressMonitor(LoadImageText,200,400);
      q=image->pixels;
      for (i=0; i < image->packets; i++)
      {
        ReadQuantum(q->blue,p);
        q++;
      }
      ProgressMonitor(LoadImageText,300,400);
      q=image->pixels;
      for (i=0; i < image->packets; i++)
      {
        ReadQuantum(q->index,p);
        q++;
      }
      ProgressMonitor(LoadImageText,400,400);
      break;
    }
  }
  /*
    Transform image from CMYK to RGB.
  */
  q=image->pixels;
  for (y=0; y < image->rows; y++)
  {
    for (x=0; x < image->columns; x++)
    {
      cyan=q->red;
      magenta=q->green;
      yellow=q->blue;
      black=q->index;
      if ((cyan+black) > MaxRGB)
        q->red=0;
      else
        q->red=MaxRGB-(cyan+black);
      if ((magenta+black) > MaxRGB)
        q->green=0;
      else
        q->green=MaxRGB-(magenta+black);
      if ((yellow+black) > MaxRGB)
        q->blue=0;
      else
        q->blue=MaxRGB-(yellow+black);
      q->index=0;
      q->length=0;
      q++;
    }
  }
  free((char *) cmyk_pixels);
  CompressImage(image);
  CloseImage(image);
  return(image);
}
/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%  R e a d D P S I m a g e                                                    %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Function ReadDPSImage reads a Adobe Postscript image file and returns it.  It
%  allocates the memory necessary for the new Image structure and returns a
%  pointer to the new image.
%
%  The format of the ReadDPSImage routine is:
%
%      image=ReadDPSImage(image_info)
%
%  A description of each parameter follows:
%
%    o image:  Function ReadDPSImage returns a pointer to the image after
%      reading.  A null image is returned if there is a a memory shortage or
%      if the image cannot be read.
%
%    o image_info: Specifies a pointer to an ImageInfo structure.
%
%
*/
#ifdef HasDPS
static Image *ReadDPSImage(image_info)
ImageInfo
  *image_info;
{
#include <DPS/dpsXclient.h>
#include <DPS/dpsXpreview.h>

  Display
    *display;

  float
    pixels_per_point;

  Image
    *image;

  int
    sans,
    status,
    x,
    y;

  Pixmap
    pixmap;

  register int
    i;

  register RunlengthPacket
    *p;

  register unsigned long
    pixel;

  Screen
    *screen;

  XColor
    *colors;

  XImage
    *dps_image;

  XRectangle
    bounding_box,
    pixel_size;

  XResourceInfo
    resource_info;

  XrmDatabase
    resource_database;

  XStandardColormap
    *map_info;

  XVisualInfo
    *visual_info;

  /*
    Open X server connection.
  */
  display=XOpenDisplay(image_info->server_name);
  if (display == (Display *) NULL)
    return((Image *) NULL);
  /*
    Set our forgiving error handler.
  */
  XSetErrorHandler(XError);
  /*
    Allocate image structure.
  */
  image=AllocateImage(image_info);
  if (image == (Image *) NULL)
    return((Image *) NULL);
  /*
    Open image file.
  */
  OpenImage(image,ReadBinaryType);
  if (image->file == (FILE *) NULL)
    return((Image *) NULL);
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
  else
    {
      /*
        Initialize visual info.
      */
      resource_info.visual_type="default";
      visual_info=XBestVisualInfo(display,map_info,&resource_info);
      map_info->colormap=(Colormap) NULL;
    }
  if ((map_info == (XStandardColormap *) NULL) ||
      (visual_info == (XVisualInfo *) NULL))
    {
      DestroyImage(image);
      XFreeResources(display,visual_info,map_info,(XPixelInfo *) NULL,
        (XFontStruct *) NULL,&resource_info,(XWindowInfo *) NULL);
      return((Image *) NULL);
    }
  /*
    Create a pixmap the appropriate size for the image.
  */
  screen=ScreenOfDisplay(display,visual_info->screen);
  pixels_per_point=XDPSPixelsPerPoint(screen);
  if (image_info->density != (char *) NULL)
    {
      unsigned int
        height,
        width;

      width=72*0*XDPSPixelsPerPoint(screen);
      (void) XParseGeometry(image_info->density,&x,&y,&width,&height);
      pixels_per_point=width/72.0;
    }
  status=XDPSCreatePixmapForEPSF((DPSContext) NULL,screen,image->file,
    visual_info->depth,pixels_per_point,&pixmap,&pixel_size,&bounding_box);
  if ((status == dps_status_failure) || (status == dps_status_no_extension))
    {
      DestroyImage(image);
      XFreeResources(display,visual_info,map_info,(XPixelInfo *) NULL,
        (XFontStruct *) NULL,&resource_info,(XWindowInfo *) NULL);
      return((Image *) NULL);
    }
  /*
    Rasterize the file into the pixmap.
  */
  status=XDPSImageFileIntoDrawable((DPSContext) NULL,screen,pixmap,image->file,
    pixel_size.height,visual_info->depth,&bounding_box,-bounding_box.x,
    -bounding_box.y,pixels_per_point,True,False,True,&sans);
  if (status != dps_status_success)
    {
      DestroyImage(image);
      XFreeResources(display,visual_info,map_info,(XPixelInfo *) NULL,
        (XFontStruct *) NULL,&resource_info,(XWindowInfo *) NULL);
      return((Image *) NULL);
    }
  /*
    Initialize DPS X image.
  */
  dps_image=XGetImage(display,pixmap,0,0,pixel_size.width,pixel_size.height,
    AllPlanes,ZPixmap);
  XFreePixmap(display,pixmap);
  if (dps_image == (XImage *) NULL)
    {
      DestroyImage(image);
      XFreeResources(display,visual_info,map_info,(XPixelInfo *) NULL,
        (XFontStruct *) NULL,&resource_info,(XWindowInfo *) NULL);
      return((Image *) NULL);
    }
  /*
    Get the colormap colors.
  */
  colors=(XColor *) malloc(visual_info->colormap_size*sizeof(XColor));
  if (colors == (XColor *) NULL)
    {
      DestroyImage(image);
      XDestroyImage(dps_image);
      XFreeResources(display,visual_info,map_info,(XPixelInfo *) NULL,
        (XFontStruct *) NULL,&resource_info,(XWindowInfo *) NULL);
      return((Image *) NULL);
    }
  if ((visual_info->class != DirectColor) && (visual_info->class != TrueColor))
    for (i=0; i < visual_info->colormap_size; i++)
    {
      colors[i].pixel=i;
      colors[i].pad=0;
    }
  else
    {
      unsigned long
        blue,
        blue_bit,
        green,
        green_bit,
        red,
        red_bit;

      /*
        DirectColor or TrueColor visual.
      */
      red=0;
      green=0;
      blue=0;
      red_bit=visual_info->red_mask & (~(visual_info->red_mask)+1);
      green_bit=visual_info->green_mask & (~(visual_info->green_mask)+1);
      blue_bit=visual_info->blue_mask & (~(visual_info->blue_mask)+1);
      for (i=0; i < visual_info->colormap_size; i++)
      {
        colors[i].pixel=red | green | blue;
        colors[i].pad=0;
        red+=red_bit;
        if (red > visual_info->red_mask)
          red=0;
        green+=green_bit;
        if (green > visual_info->green_mask)
          green=0;
        blue+=blue_bit;
        if (blue > visual_info->blue_mask)
          blue=0;
      }
    }
  XQueryColors(display,XDefaultColormap(display,visual_info->screen),colors,
    visual_info->colormap_size);
  /*
    Convert X image to MIFF format.
  */
  if ((visual_info->class != TrueColor) && (visual_info->class != DirectColor))
    image->class=PseudoClass;
  image->columns=dps_image->width;
  image->rows=dps_image->height;
  image->packets=image->columns*image->rows;
  image->pixels=(RunlengthPacket *)
    malloc(image->packets*sizeof(RunlengthPacket));
  if (image->pixels == (RunlengthPacket *) NULL)
    {
      DestroyImage(image);
      free((char *) colors);
      XDestroyImage(dps_image);
      XFreeResources(display,visual_info,map_info,(XPixelInfo *) NULL,
        (XFontStruct *) NULL,&resource_info,(XWindowInfo *) NULL);
      return((Image *) NULL);
    }
  p=image->pixels;
  switch (image->class)
  {
    case DirectClass:
    {
      register unsigned long
        color,
        index;

      unsigned long
        blue_mask,
        blue_shift,
        green_mask,
        green_shift,
        red_mask,
        red_shift;

      /*
        Determine shift and mask for red, green, and blue.
      */
      red_mask=visual_info->red_mask;
      red_shift=0;
      while ((red_mask & 0x01) == 0)
      {
        red_mask>>=1;
        red_shift++;
      }
      green_mask=visual_info->green_mask;
      green_shift=0;
      while ((green_mask & 0x01) == 0)
      {
        green_mask>>=1;
        green_shift++;
      }
      blue_mask=visual_info->blue_mask;
      blue_shift=0;
      while ((blue_mask & 0x01) == 0)
      {
        blue_mask>>=1;
        blue_shift++;
      }
      /*
        Convert X image to DirectClass packets.
      */
      if ((visual_info->colormap_size > 0) &&
          (visual_info->class == DirectColor))
        for (y=0; y < image->rows; y++)
        {
          for (x=0; x < image->columns; x++)
          {
            pixel=XGetPixel(dps_image,x,y);
            index=(pixel >> red_shift) & red_mask;
            p->red=XDownScale(colors[index].red);
            index=(pixel >> green_shift) & green_mask;
            p->green=XDownScale(colors[index].green);
            index=(pixel >> blue_shift) & blue_mask;
            p->blue=XDownScale(colors[index].blue);
            p->index=0;
            p->length=0;
            p++;
          }
          ProgressMonitor(LoadImageText,y,image->rows);
        }
      else
        for (y=0; y < image->rows; y++)
        {
          for (x=0; x < image->columns; x++)
          {
            pixel=XGetPixel(dps_image,x,y);
            color=(pixel >> red_shift) & red_mask;
            p->red=XDownScale((color*65535L)/red_mask);
            color=(pixel >> green_shift) & green_mask;
            p->green=XDownScale((color*65535L)/green_mask);
            color=(pixel >> blue_shift) & blue_mask;
            p->blue=XDownScale((color*65535L)/blue_mask);
            p->index=0;
            p->length=0;
            p++;
          }
          ProgressMonitor(LoadImageText,y,image->rows);
        }
      break;
    }
    case PseudoClass:
    {
      /*
        Create colormap.
      */
      image->colors=visual_info->colormap_size;
      image->colormap=(ColorPacket *) malloc(image->colors*sizeof(ColorPacket));
      if (image->colormap == (ColorPacket *) NULL)
        {
          DestroyImage(image);
          free((char *) colors);
          XDestroyImage(dps_image);
          XFreeResources(display,visual_info,map_info,(XPixelInfo *) NULL,
            (XFontStruct *) NULL,&resource_info,(XWindowInfo *) NULL);
          return((Image *) NULL);
        }
      for (i=0; i < image->colors; i++)
      {
        image->colormap[colors[i].pixel].red=XDownScale(colors[i].red);
        image->colormap[colors[i].pixel].green=XDownScale(colors[i].green);
        image->colormap[colors[i].pixel].blue=XDownScale(colors[i].blue);
      }
      /*
        Convert X image to PseudoClass packets.
      */
      for (y=0; y < image->rows; y++)
      {
        for (x=0; x < image->columns; x++)
        {
          p->index=(unsigned short) XGetPixel(dps_image,x,y);
          p->length=0;
          p++;
        }
        ProgressMonitor(LoadImageText,y,image->rows);
      }
      SyncImage(image);
      break;
    }
  }
  if (image->class == PseudoClass)
    CompressColormap(image);
  free((char *) colors);
  XDestroyImage(dps_image);
  /*
    Rasterize matte image.
  */
  status=XDPSCreatePixmapForEPSF((DPSContext) NULL,screen,image->file,1,
    pixels_per_point,&pixmap,&pixel_size,&bounding_box);
  if ((status != dps_status_failure) && (status != dps_status_no_extension))
    {
      status=XDPSImageFileIntoDrawable((DPSContext) NULL,screen,pixmap,
        image->file,pixel_size.height,1,&bounding_box,-bounding_box.x,
        -bounding_box.y,pixels_per_point,True,True,True,&sans);
      if (status == dps_status_success)
        {
          XImage
            *matte_image;

          /*
            Initialize image matte.
          */
          matte_image=XGetImage(display,pixmap,0,0,pixel_size.width,
            pixel_size.height,AllPlanes,ZPixmap);
          XFreePixmap(display,pixmap);
          if (matte_image != (XImage *) NULL)
            {
              image->class=DirectClass;
              image->matte=True;
              p=image->pixels;
              for (y=0; y < image->rows; y++)
                for (x=0; x < image->columns; x++)
                {
                  p->index=Opaque;
                  if (!XGetPixel(matte_image,x,y))
                    p->index=Transparent;
                  p++;
                }
              XDestroyImage(matte_image);
            }
        }
    }
  /*
    Free resources.
  */
  XFreeResources(display,visual_info,map_info,(XPixelInfo *) NULL,
    (XFontStruct *) NULL,&resource_info,(XWindowInfo *) NULL);
  CompressImage(image);
  CloseImage(image);
  return(image);
}
#else
static Image *ReadDPSImage(ImageInfo *image_info)
{
  return((Image *) NULL);
}
#endif

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%  R e a d F A X I m a g e                                                    %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Function ReadFAXImage reads a Group 3 FAX image file and returns it.  It
%  allocates the memory necessary for the new Image structure and returns a
%  pointer to the new image.
%
%  The format of the ReadFAXImage routine is:
%
%      image=ReadFAXImage(image_info)
%
%  A description of each parameter follows:
%
%    o image:  Function ReadFAXImage returns a pointer to the image after
%      reading.  A null image is returned if there is a a memory shortage or
%      if the image cannot be read.
%
%    o image_info: Specifies a pointer to an ImageInfo structure.
%
%
*/
static Image *ReadFAXImage(ImageInfo *image_info)
{
  Image
    *image;

  unsigned int
    status;

  /*
    Allocate image structure.
  */
  image=AllocateImage(image_info);
  if (image == (Image *) NULL)
    return((Image *) NULL);
  /*
    Open image file.
  */
  OpenImage(image,ReadBinaryType);
  if (image->file == (FILE *) NULL)
    PrematureExit("Unable to open file",image);
  /*
    Initialize image structure.
  */
  image->class=PseudoClass;
  image->columns=1728;
  image->rows=2156;
  image->packets=Max((image->columns*image->rows+8) >> 4,1);
  image->pixels=(RunlengthPacket *)
    malloc(image->packets*sizeof(RunlengthPacket));
  image->colors=2;
  image->colormap=(ColorPacket *) malloc(image->colors*sizeof(ColorPacket));
  if ((image->pixels == (RunlengthPacket *) NULL) ||
      (image->colormap == (ColorPacket *) NULL))
    PrematureExit("Unable to allocate memory",image);
  /*
    Monochrome colormap.
  */
  image->colormap[0].red=MaxRGB;
  image->colormap[0].green=MaxRGB;
  image->colormap[0].blue=MaxRGB;
  image->colormap[1].red=0;
  image->colormap[1].green=0;
  image->colormap[1].blue=0;
  status=HuffmanDecodeImage(image);
  if (status == False)
    PrematureExit("Unable to read image data",image);
  CloseImage(image);
  return(image);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   R e a d F I T S I m a g e                                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Function ReadFITSImage reads a FITS image file and returns it.  It
%  allocates the memory necessary for the new Image structure and returns a
%  pointer to the new image.
%
%  The format of the ReadFITSImage routine is:
%
%      image=ReadFITSImage(image_info)
%
%  A description of each parameter follows:
%
%    o image: Function ReadFITSImage returns a pointer to the image after
%      reading.  A null image is returned if there is a a memory shortage or if
%      the image cannot be read.
%
%    o filename: Specifies the name of the image to read.
%
%
*/
static Image *ReadFITSImage(ImageInfo *image_info)
{
  typedef struct _FITSHeader
  {
    unsigned int
      simple;

    int
      bits_per_pixel;

    unsigned int
      number_of_axis,
      columns,
      rows,
      depth;

    double
      min_data,
      max_data,
      zero,
      scale;
  } FITSHeader;

  char
    keyword[MaxTextLength],
    value[MaxTextLength];

  double
    pixel,
    scale,
    scaled_pixel;

  FITSHeader
    fits_header;

  Image
    *image;

  int
    j,
    packet_size,
    y;

  long
    count,
    quantum;

  register int
    c,
    i,
    x;

  register RunlengthPacket
    *q;

  register unsigned char
    *p;

  unsigned char
    *fits_pixels;

  unsigned int
    status,
    value_expected;

  /*
    Allocate image structure.
  */
  image=AllocateImage(image_info);
  if (image == (Image *) NULL)
    return((Image *) NULL);
  /*
    Open image file.
  */
  OpenImage(image,ReadBinaryType);
  if (image->file == (FILE *) NULL)
    PrematureExit("Unable to open file",image);
  /*
    Initialize image header.
  */
  fits_header.simple=False;
  fits_header.bits_per_pixel=8;
  fits_header.columns=1;
  fits_header.rows=1;
  fits_header.depth=1;
  fits_header.min_data=0.0;
  fits_header.max_data=0.0;
  fits_header.zero=0.0;
  fits_header.scale=1.0;
  /*
    Decode image header.
  */
  c=fgetc(image->file);
  count=1;
  if (c == EOF)
    {
      DestroyImage(image);
      return((Image *) NULL);
    }
  for ( ; ; )
  {
    if (!isalnum(c))
      {
        c=fgetc(image->file);
        count++;
      }
    else
      {
        register char
          *p;

        /*
          Determine a keyword and its value.
        */
        p=keyword;
        do
        {
          if ((p-keyword) < (MaxTextLength-1))
            *p++=(char) c;
          c=fgetc(image->file);
          count++;
        } while (isalnum(c) || (c == '_'));
        *p='\0';
        if (strcmp(keyword,"END") == 0)
          break;
        value_expected=False;
        while (isspace(c) || (c == '='))
        {
          if (c == '=')
            value_expected=True;
          c=fgetc(image->file);
          count++;
        }
        if (value_expected == False)
          continue;
        p=value;
        while (isalnum(c) || (c == '-') || (c == '+') || (c == '.'))
        {
          if ((p-value) < (MaxTextLength-1))
            *p++=(char) c;
          c=fgetc(image->file);
          count++;
        }
        *p='\0';
        /*
          Assign a value to the specified keyword.
        */
        if (strcmp(keyword,"SIMPLE") == 0)
          fits_header.simple=(*value == 'T') || (*value == 't');
        if (strcmp(keyword,"BITPIX") == 0)
          fits_header.bits_per_pixel=(unsigned int) atoi(value);
        if (strcmp(keyword,"NAXIS") == 0)
          fits_header.number_of_axis=(unsigned int) atoi(value);
        if (strcmp(keyword,"NAXIS1") == 0)
          fits_header.columns=(unsigned int) atoi(value);
        if (strcmp(keyword,"NAXIS2") == 0)
          fits_header.rows=(unsigned int) atoi(value);
        if (strcmp(keyword,"NAXIS3") == 0)
          fits_header.depth=(unsigned int) atoi(value);
        if (strcmp(keyword,"DATAMAX") == 0)
          fits_header.max_data=atof(value);
        if (strcmp(keyword,"DATAMIN") == 0)
          fits_header.min_data=atof(value);
        if (strcmp(keyword,"BZERO") == 0)
          fits_header.zero=atof(value);
        if (strcmp(keyword,"BSCALE") == 0)
          fits_header.scale=atof(value);
      }
    while (isspace(c))
    {
      c=fgetc(image->file);
      count++;
    }
  }
  while (count > 2880)
    count-=2880;
  for ( ; count < 2880; count++)
    (void) fgetc(image->file);
  /*
    Verify that required image information is defined.
  */
  if ((!fits_header.simple) || (fits_header.number_of_axis < 1) ||
      (fits_header.number_of_axis > 3) ||
      (fits_header.columns*fits_header.rows) == 0)
    PrematureExit("image type not supported",image);
  /*
    Create linear colormap.
  */
  image->columns=fits_header.columns;
  image->rows=fits_header.rows;
  image->class=PseudoClass;
  image->colors=MaxRGB+1;
  image->colormap=(ColorPacket *) malloc(image->colors*sizeof(ColorPacket));
  if (image->colormap == (ColorPacket *) NULL)
    PrematureExit("Unable to open file",image);
  for (i=0; i < image->colors; i++)
  {
    image->colormap[i].red=(Quantum) UpScale(i);
    image->colormap[i].green=(Quantum) UpScale(i);
    image->colormap[i].blue=(Quantum) UpScale(i);
  }
  /*
    Initialize image structure.
  */
  image->packets=image->columns*image->rows;
  image->pixels=(RunlengthPacket *)
    malloc(image->packets*sizeof(RunlengthPacket));
  packet_size=fits_header.bits_per_pixel/8;
  if (packet_size < 0)
    packet_size=(-packet_size);
  fits_pixels=(unsigned char *)
    malloc(image->packets*packet_size*sizeof(unsigned char));
  if ((image->pixels == (RunlengthPacket *) NULL) ||
      (fits_pixels == (unsigned char *) NULL))
    PrematureExit("Unable to allocate memory",image);
  /*
    Convert FITS pixels to runlength-encoded packets.
  */
  status=ReadData((char *) fits_pixels,(unsigned int) packet_size,
    image->packets,image->file);
  if (status == False)
    Warning("Insufficient image data in file",image->filename);
  if ((fits_header.min_data == 0.0) && (fits_header.max_data == 0.0))
    {
      /*
        Determine minimum and maximum intensity.
      */
      p=fits_pixels;
      quantum=(*p++);
      for (j=0; j < (packet_size-1); j++)
        quantum=(quantum << 8) | (*p++);
      pixel=(double) quantum;
      if (fits_header.bits_per_pixel == -32)
        pixel=(double) (*((float *) &quantum));
      fits_header.min_data=pixel*fits_header.scale+fits_header.zero;
      fits_header.max_data=pixel*fits_header.scale+fits_header.zero;
      for (i=1; i < image->packets; i++)
      {
        quantum=(*p++);
        for (j=0; j < (packet_size-1); j++)
          quantum=(quantum << 8) | (*p++);
        pixel=(double) quantum;
        if (fits_header.bits_per_pixel == -32)
          pixel=(double) (*((float *) &quantum));
        scaled_pixel=pixel*fits_header.scale+fits_header.zero;
        if (scaled_pixel < fits_header.min_data)
          fits_header.min_data=scaled_pixel;
        if (scaled_pixel > fits_header.max_data)
          fits_header.max_data=scaled_pixel;
      }
    }
  /*
    Convert FITS pixels to runlength-encoded packets.
  */
  scale=1.0;
  if (fits_header.min_data != fits_header.max_data)
    scale=(MaxRGB+1)/(fits_header.max_data-fits_header.min_data);
  p=fits_pixels;
  q=image->pixels;
  for (y=0; y < image->rows; y++)
  {
    for (x=0; x < image->columns; x++)
    {
      quantum=(*p++);
      for (j=0; j < (packet_size-1); j++)
        quantum=(quantum << 8) | (*p++);
      pixel=(double) quantum;
      if (fits_header.bits_per_pixel == -32)
        pixel=(double) (*((float *) &quantum));
      scaled_pixel=scale*
        (pixel*fits_header.scale+fits_header.zero-fits_header.min_data);
      while (scaled_pixel < 0)
        scaled_pixel+=(MaxRGB+1);
      while (scaled_pixel > MaxRGB)
        scaled_pixel-=(MaxRGB+1);
      q->index=(unsigned short) scaled_pixel;
      q->length=0;
      q++;
    }
    ProgressMonitor(LoadImageText,y,image->rows);
  }
  free((char *) fits_pixels);
  SyncImage(image);
  CompressColormap(image);
  CompressImage(image);
  CloseImage(image);
  return(image);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%  R e a d G I F I m a g e                                                    %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Function ReadGIFImage reads a Compuserve Graphics image file and returns it.
%  It allocates the memory necessary for the new Image structure and returns a
%  pointer to the new image.
%
%  The format of the ReadGIFImage routine is:
%
%      image=ReadGIFImage(image_info)
%
%  A description of each parameter follows:
%
%    o image:  Function ReadGIFImage returns a pointer to the image after
%      reading.  A null image is returned if there is a a memory shortage or
%      an error occurs.
%
%    o image_info: Specifies a pointer to an ImageInfo structure.
%
%
*/
static Image *ReadGIFImage(ImageInfo *image_info)
{
#define BitSet(Byte,bit)  (((Byte) & (bit)) == (bit))
#define LSBFirstOrder(x,y)  (((y) << 8) | (x))

  Image
    *image;

  int
    status,
    x,
    y;

  register int
    i;

  register RunlengthPacket
    *q;

  register unsigned char
    *p;

  short int
    transparency_index;

  unsigned char
    c,
    *global_colormap,
    header[MaxTextLength],
    magick[12];

  unsigned int
    global_colors,
    image_count,
    local_colormap;

  /*
    Allocate image structure.
  */
  image=AllocateImage(image_info);
  if (image == (Image *) NULL)
    return((Image *) NULL);
  /*
    Open image file.
  */
  OpenImage(image,ReadBinaryType);
  if (image->file == (FILE *) NULL)
    PrematureExit("Unable to open file",image);
  /*
    Determine if this is a GIF file.
  */
  status=ReadData((char *) magick,1,6,image->file);
  status|=ReadData((char *) header,1,7,image->file);
  if ((status == False) || ((strncmp((char *) magick,"GIF87",5) != 0) &&
      (strncmp((char *) magick,"GIF89",5) != 0)))
    PrematureExit("Not a GIF image file",image);
  global_colors=0;
  global_colormap=(unsigned char *) NULL;
  if (BitSet(header[4],0x80))
    {
      /*
        Read global colormap.
      */
      global_colors=1 << ((header[4] & 0x07)+1);
      global_colormap=(unsigned char *)
        malloc(3*global_colors*sizeof(unsigned char));
      if (global_colormap == (unsigned char *) NULL)
        PrematureExit("Unable to read image colormap file",image);
      (void) ReadData((char *) global_colormap,3,global_colors,image->file);
    }
  transparency_index=(-1);
  image_count=0;
  for ( ; ; )
  {
    status=ReadData((char *) &c,1,1,image->file);
    if (status == False)
      break;
    if (c == ';')
      break;  /* terminator */
    if (c == '!')
      {
        /*
          GIF Extension block.
        */
        status=ReadData((char *) &c,1,1,image->file);
        if (status == False)
          PrematureExit("Unable to read extention block",image);
        switch (c)
        {
          case 0xf9:
          {
            /*
              Transparency extension block.
            */
            while (ReadDataBlock((char *) header,image->file) > 0);
            if ((header[0] & 0x01) == 1)
              transparency_index=header[3];
            break;
          }
          case 0xfe:
          {
            int
              length;

            /*
              Comment extension block.
            */
            for ( ; ; )
            {
              length=ReadDataBlock((char *) header,image->file);
              if (length <= 0)
                break;
              if (image->comments != (char *) NULL)
                image->comments=(char *) realloc((char *) image->comments,
                  (strlen(image->comments)+length+1)*sizeof(char));
              else
                {
                  image->comments=(char *) malloc((length+1)*sizeof(char));
                  if (image->comments != (char *) NULL)
                    *image->comments='\0';
                }
              if (image->comments == (char *) NULL)
                PrematureExit("Unable to allocate memory",image);
              header[length]='\0';
              (void) strcat(image->comments,(char *) header);
            }
            break;
          }
          default:
          {
            while (ReadDataBlock((char *) header,image->file) > 0);
            break;
          }
        }
      }
    if (c != ',')
      continue;
    /*
      Read image attributes.
    */
    if (image_count != 0)
      {
        /*
          Allocate image structure.
        */
        image->next=AllocateImage(image_info);
        if (image->next == (Image *) NULL)
          {
            DestroyImages(image);
            return((Image *) NULL);
          }
        (void) strcpy(image->next->filename,image_info->filename);
        image->next->file=image->file;
        image->next->scene=image->scene+1;
        image->next->previous=image;
        image=image->next;
      }
    image_count++;
    status=ReadData((char *) header,1,9,image->file);
    if (status == False)
      PrematureExit("Unable to read left/top/width/height",image);
    image->interlace=BitSet(header[8],0x40);
    local_colormap=BitSet(header[8],0x80);
    /*
      Allocate image.
    */
    image->columns=LSBFirstOrder(header[4],header[5]);
    image->rows=LSBFirstOrder(header[6],header[7]);
    image->packets=image->columns*image->rows;
    if (image->packets == 0)
      PrematureExit("image size is 0",image);
    if (image->pixels != (RunlengthPacket *) NULL)
      free((char *) image->pixels);
    image->pixels=(RunlengthPacket *)
      malloc(image->packets*sizeof(RunlengthPacket));
    if (image->pixels == (RunlengthPacket *) NULL)
      PrematureExit("Unable to allocate memory",image);
    /*
      Inititialize colormap.
    */
    image->class=PseudoClass;
    image->colors=!local_colormap ? global_colors : 1 << ((header[8] & 0x07)+1);
    image->colormap=(ColorPacket *) malloc(image->colors*sizeof(ColorPacket));
    if (image->colormap == (ColorPacket *) NULL)
      PrematureExit("Unable to allocate memory",image);
    if (!local_colormap)
      {
        /*
          Use global colormap.
        */
        p=global_colormap;
        for (i=0; i < image->colors; i++)
        {
          image->colormap[i].red=UpScale(*p++);
          image->colormap[i].green=UpScale(*p++);
          image->colormap[i].blue=UpScale(*p++);
        }
      }
    else
      {
        unsigned char
          *colormap;

        /*
          Read local colormap.
        */
        colormap=(unsigned char *)
          malloc(3*image->colors*sizeof(unsigned char));
        if (colormap == (unsigned char *) NULL)
          PrematureExit("Unable to allocate memory",image);
        (void) ReadData((char *) colormap,3,image->colors,image->file);
        p=colormap;
        for (i=0; i < image->colors; i++)
        {
          image->colormap[i].red=UpScale(*p++);
          image->colormap[i].green=UpScale(*p++);
          image->colormap[i].blue=UpScale(*p++);
        }
        free((char *) colormap);
      }
    /*
      Decode image.
    */
    status=GIFDecodeImage(image);
    if (image->interlace)
      {
        Image
          *interlaced_image;

        int
          pass;

        register RunlengthPacket
          *p;

        static int
          interlace_rate[4] = { 8, 8, 4, 2 },
          interlace_start[4] = { 0, 4, 2, 1 };

        /*
          Interlace image.
        */
        image_info->interlace=LineInterlace;
        image->orphan=True;
        interlaced_image=CopyImage(image,image->columns,image->rows,True);
        image->orphan=False;
        if (interlaced_image == (Image *) NULL)
          PrematureExit("Unable to allocate memory",image);
        p=interlaced_image->pixels;
        q=image->pixels;
        for (pass=0; pass < 4; pass++)
        {
          y=interlace_start[pass];
          while (y < image->rows)
          {
            q=image->pixels+(y*image->columns);
            for (x=0; x < image->columns; x++)
            {
              *q=(*p);
              p++;
              q++;
            }
            y+=interlace_rate[pass];
          }
        }
        DestroyImage(interlaced_image);
      }
    if (transparency_index >= 0)
      {
        /*
          Create matte channel.
        */
        q=image->pixels;
        for (i=0; i < image->packets; i++)
        {
          if (q->index != (unsigned short) transparency_index)
            q->index=Opaque;
          else
            q->index=Transparent;
          q++;
        }
        transparency_index=(-1);
        image->class=DirectClass;
        image->matte=True;
      }
    if (status == False)
      {
        Warning("Corrupt GIF image",image->filename);
        break;
      }
    CompressImage(image);
  }
  if (global_colormap != (unsigned char *) NULL)
    free((char *) global_colormap);
  if (image->pixels == (RunlengthPacket *) NULL)
    PrematureExit("Corrupt GIF image",image);
  while (image->previous != (Image *) NULL)
    image=image->previous;
  CloseImage(image);
  return(image);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%  R e a d G R A Y I m a g e                                                  %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Function ReadGRAYImage reads an image of raw grayscale bytes and returns it.
%  It allocates the memory necessary for the new Image structure and returns a
%  pointer to the new image.
%
%  The format of the ReadGRAYImage routine is:
%
%      image=ReadGRAYImage(image_info)
%
%  A description of each parameter follows:
%
%    o image:  Function ReadGRAYImage returns a pointer to the image after
%      reading.  A null image is returned if there is a a memory shortage or
%      if the image cannot be read.
%
%    o image_info: Specifies a pointer to an ImageInfo structure.
%
%
*/
static Image *ReadGRAYImage(ImageInfo *image_info)
{
  Image
    *image;

  int
    x,
    y;

  register int
    i;

  register RunlengthPacket
    *q;

  unsigned int
    height,
    packets,
    width;

  unsigned short
    index;

  /*
    Allocate image structure.
  */
  image=AllocateImage(image_info);
  if (image == (Image *) NULL)
    return((Image *) NULL);
  /*
    Open image file.
  */
  OpenImage(image,ReadBinaryType);
  if (image->file == (FILE *) NULL)
    PrematureExit("Unable to open file",image);
  /*
    Create linear colormap.
  */
  image->class=PseudoClass;
  image->colors=1 << QuantumDepth;
  image->colormap=(ColorPacket *) malloc(image->colors*sizeof(ColorPacket));
  if (image->colormap == (ColorPacket *) NULL)
    PrematureExit("Unable to allocate memory",image);
  for (i=0; i < image->colors; i++)
  {
    image->colormap[i].red=(Quantum) i;
    image->colormap[i].green=(Quantum) i;
    image->colormap[i].blue=(Quantum) i;
  }
  /*
    Determine width and height, e.g. 640x512.
  */
  width=512;
  height=512;
  x=0;
  if (image_info->size != (char *) NULL)
    (void) XParseGeometry(image_info->size,&x,&y,&width,&height);
  for (i=0; i < x; i++)
    (void) fgetc(image->file);
  /*
    Initialize image structure.
  */
  image->columns=width;
  image->rows=height;
  image->packets=0;
  packets=Max((image->columns*image->rows+4) >> 3,1);
  image->pixels=(RunlengthPacket *) malloc(packets*sizeof(RunlengthPacket));
  if (image->pixels == (RunlengthPacket *) NULL)
    PrematureExit("Unable to allocate memory",image);
  /*
    Convert raster image to runlength-encoded packets.
  */
  q=image->pixels;
  q->length=MaxRunlength;
  for (y=0; y < image->rows; y++)
  {
    for (x=0; x < image->columns; x++)
    {
      ReadQuantumFile(index);
      if ((index == q->index) && ((int) q->length < MaxRunlength))
        q->length++;
      else
        {
          if (image->packets != 0)
            q++;
          image->packets++;
          if (image->packets == packets)
            {
              packets<<=1;
              image->pixels=(RunlengthPacket *)
                realloc((char *) image->pixels,packets*sizeof(RunlengthPacket));
              if (image->pixels == (RunlengthPacket *) NULL)
                PrematureExit("Unable to allocate memory",image);
              q=image->pixels+image->packets-1;
            }
          q->index=index;
          q->length=0;
        }
      }
    ProgressMonitor(LoadImageText,y,image->rows);
  }
  image->pixels=(RunlengthPacket *)
    realloc((char *) image->pixels,image->packets*sizeof(RunlengthPacket));
  SyncImage(image);
  CompressColormap(image);
  CloseImage(image);
  return(image);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%  R e a d H D F I m a g e                                                    %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Function ReadHDFImage reads a Hierarchical Data Format image file and
%  returns it.  It allocates the memory necessary for the new Image structure
%  and returns a pointer to the new image.
%
%  The format of the ReadHDFImage routine is:
%
%      image=ReadHDFImage(image_info)
%
%  A description of each parameter follows:
%
%    o image:  Function ReadHDFImage returns a pointer to the image after
%      reading.  A null image is returned if there is a a memory shortage or
%      if the image cannot be read.
%
%    o image_info: Specifies a pointer to an ImageInfo structure.
%
%
*/
#ifdef HasHDF
static Image *ReadHDFImage(image_info)
ImageInfo
  *image_info;
{
#include "hdf.h"

  Image
    *image;

  int
    interlace,
    is_palette,
    status,
    y;

  int32
    height,
    length,
    width;

  register int
    i,
    x;

  register unsigned char
    *p;

  register RunlengthPacket
    *q;

  uint16
    reference;

  unsigned char
    *hdf_pixels;

  unsigned int
    class,
    packet_size;

  /*
    Allocate image structure.
  */
  image=AllocateImage(image_info);
  if (image == (Image *) NULL)
    return((Image *) NULL);
  /*
    Open image file.
  */
  OpenImage(image,ReadBinaryType);
  if (image->file == (FILE *) NULL)
    PrematureExit("Unable to open file",image);
  CloseImage(image);
  /*
    Read HDF image.
  */
  class=DirectClass;
  status=DF24getdims(image->filename,&width,&height,&interlace);
  if (status == -1)
    {
      class=PseudoClass;
      status=DFR8getdims(image->filename,&width,&height,&is_palette);
    }
  if (status == -1)
    PrematureExit("Image file or does not contain any image data",image);
  do
  {
    /*
      Initialize image structure.
    */
    image->class=class;
    image->columns=width;
    image->rows=height;
    image->packets=image->columns*image->rows;
    packet_size=1;
    if (image->class == DirectClass)
      packet_size=3;
    hdf_pixels=(unsigned char *)
      malloc(packet_size*image->packets*sizeof(unsigned char));
    image->pixels=(RunlengthPacket *)
      malloc(image->packets*sizeof(RunlengthPacket));
    if ((hdf_pixels == (unsigned char *) NULL) ||
        (image->pixels == (RunlengthPacket *) NULL))
      PrematureExit("Unable to allocate memory",image);
    q=image->pixels;
    if (image->class == PseudoClass)
      {
        unsigned char
          *hdf_palette;

        /*
          Create colormap.
        */
        hdf_palette=(unsigned char *) malloc(768*sizeof(unsigned char));
        image->colors=256;
        image->colormap=(ColorPacket *)
          malloc(image->colors*sizeof(ColorPacket));
        if ((hdf_palette == (unsigned char *) NULL) ||
            (image->colormap == (ColorPacket *) NULL))
          PrematureExit("Unable to allocate memory",image);
        (void) DFR8getimage(image->filename,hdf_pixels,(int) image->columns,
          (int) image->rows,hdf_palette);
        reference=DFR8lastref();
        /*
          Convert HDF raster image to PseudoClass runlength-encoded packets.
        */
        p=hdf_palette;
        if (is_palette)
          for (i=0; i < 256; i++)
          {
            image->colormap[i].red=UpScale(*p++);
            image->colormap[i].green=UpScale(*p++);
            image->colormap[i].blue=UpScale(*p++);
          }
        else
          for (i=0; i < image->colors; i++)
          {
            image->colormap[i].red=(Quantum) UpScale(i);
            image->colormap[i].green=(Quantum) UpScale(i);
            image->colormap[i].blue=(Quantum) UpScale(i);
          }
        free((char *) hdf_palette);
        p=hdf_pixels;
        for (y=0; y < image->rows; y++)
        {
          for (x=0; x < image->columns; x++)
          {
            q->index=(*p++);
            q->length=0;
            q++;
          }
          ProgressMonitor(LoadImageText,y,image->rows);
        }
        SyncImage(image);
      }
    else
      {
        int
          x,
          y;

        /*
          Convert HDF raster image to DirectClass runlength-encoded packets.
        */
        (void) DF24getimage(image->filename,(void *) hdf_pixels,image->columns,
          image->rows);
        reference=DF24lastref();
        p=hdf_pixels;
        image_info->interlace=interlace+1;
        switch (image_info->interlace)
        {
          case NoneInterlace:
          default:
          {
            /*
              No interlacing:  RGBRGBRGBRGBRGBRGB...
            */
            q=image->pixels;
            for (y=0; y < image->rows; y++)
            {
              for (x=0; x < image->columns; x++)
              {
                q->red=UpScale(*p++);
                q->green=UpScale(*p++);
                q->blue=UpScale(*p++);
                q->index=0;
                q->length=0;
                q++;
              }
              ProgressMonitor(LoadImageText,y,image->rows);
            }
            break;
          }
          case LineInterlace:
          {
            /*
              Line interlacing:  RRR...GGG...BBB...RRR...GGG...BBB...
            */
            for (y=0; y < image->rows; y++)
            {
              q=image->pixels+y*image->columns;
              for (x=0; x < image->columns; x++)
              {
                q->red=UpScale(*p++);
                q->index=0;
                q->length=0;
                q++;
              }
              q=image->pixels+y*image->columns;
              for (x=0; x < image->columns; x++)
              {
                q->green=UpScale(*p++);
                q++;
              }
              q=image->pixels+y*image->columns;
              for (x=0; x < image->columns; x++)
              {
                q->blue=UpScale(*p++);
                q++;
              }
              ProgressMonitor(LoadImageText,y,image->rows);
            }
            break;
          }
          case PlaneInterlace:
          {
            /*
              Plane interlacing:  RRRRRR...GGGGGG...BBBBBB...
            */
            ProgressMonitor(LoadImageText,100,400);
            q=image->pixels;
            for (i=0; i < image->packets; i++)
            {
              q->red=UpScale(*p++);
              q->index=0;
              q->length=0;
              q++;
            }
            ProgressMonitor(LoadImageText,200,400);
            q=image->pixels;
            for (i=0; i < image->packets; i++)
            {
              q->green=UpScale(*p++);
              q++;
            }
            ProgressMonitor(LoadImageText,300,400);
            q=image->pixels;
            for (i=0; i < image->packets; i++)
            {
              q->blue=UpScale(*p++);
              q++;
            }
            ProgressMonitor(LoadImageText,400,400);
            break;
          }
        }
      }
    length=DFANgetlablen(image->filename,DFTAG_RIG,reference);
    if (length > 0)
      {
        /*
          Read the image label.
        */
        length+=MaxTextLength;
        image->label=malloc(length*sizeof(char));
        if (image->label != (char *) NULL)
          DFANgetlabel(image->filename,DFTAG_RIG,reference,image->label,length);
      }
    length=DFANgetdesclen(image->filename,DFTAG_RIG,reference);
    if (length > 0)
      {
        /*
          Read the image comments.
        */
        length+=MaxTextLength;
        image->comments=malloc(length*sizeof(char));
        if (image->comments != (char *) NULL)
          DFANgetdesc(image->filename,DFTAG_RIG,reference,image->comments,
            length);
      }
    free((char *) hdf_pixels);
    CompressImage(image);
    class=DirectClass;
    status=DF24getdims(image->filename,&width,&height,&interlace);
    if (status == -1)
      {
        class=PseudoClass;
        status=DFR8getdims(image->filename,&width,&height,&is_palette);
      }
    if (status != -1)
      {
        /*
          Allocate image structure.
        */
        image->next=AllocateImage(image_info);
        if (image->next == (Image *) NULL)
          {
            DestroyImages(image);
            return((Image *) NULL);
          }
        (void) strcpy(image->next->filename,image_info->filename);
        image->next->file=image->file;
        image->next->scene=image->scene+1;
        image->next->previous=image;
        image=image->next;
      }
  } while (status != -1);
  while (image->previous != (Image *) NULL)
    image=image->previous;
  return(image);
}
#else
static Image *ReadHDFImage(ImageInfo *image_info)
{
  Warning("HDF library is not available",image_info->filename);
  return(ReadMIFFImage(image_info));
}
#endif

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%  R e a d H I S T O G R A M I m a g e                                        %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Function ReadHISTOGRAMImage reads a HISTOGRAM image file and returns it.  It
%  allocates the memory necessary for the new Image structure and returns a
%  pointer to the new image.
%
%  The format of the ReadHISTOGRAMImage routine is:
%
%      image=ReadHISTOGRAMImage(image_info)
%
%  A description of each parameter follows:
%
%    o image:  Function ReadHISTOGRAMImage returns a pointer to the image after
%      reading.  A null image is returned if there is a a memory shortage or
%      if the image cannot be read.
%
%    o image_info: Specifies a pointer to an ImageInfo structure.
%
%
*/
static Image *ReadHISTOGRAMImage(ImageInfo *image_info)
{
  Image
    *image;

  image=ReadMIFFImage(image_info);
  return(image);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%  R e a d H T M L I m a g e                                                  %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Function ReadHTMLImage reads a HTML image file and returns it.  It
%  allocates the memory necessary for the new Image structure and returns a
%  pointer to the new image.
%
%  The format of the ReadHTMLImage routine is:
%
%      image=ReadHTMLImage(image_info)
%
%  A description of each parameter follows:
%
%    o image:  Function ReadHTMLImage returns a pointer to the image after
%      reading.  A null image is returned if there is a a memory shortage or
%      if the image cannot be read.
%
%    o image_info: Specifies a pointer to an ImageInfo structure.
%
%
*/
static Image *ReadHTMLImage(ImageInfo *image_info)
{
  Image
    *image;

  Warning("Cannot read HTML images",image_info->filename);
  image=ReadMIFFImage(image_info);
  return(image);
}

#ifdef HasJBIG
#include "jbig.h"
/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%  R e a d J B I G I m a g e                                                  %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Function ReadJBIGImage reads a JBIG image file and returns it.  It
%  allocates the memory necessary for the new Image structure and returns a
%  pointer to the new image.
%
%  The format of the ReadJBIGImage routine is:
%
%      image=ReadJBIGImage(image_info)
%
%  A description of each parameter follows:
%
%    o image:  Function ReadJBIGImage returns a pointer to the image after
%      reading.  A null image is returned if there is a a memory shortage or
%      if the image cannot be read.
%
%    o image_info: Specifies a pointer to an ImageInfo structure.
%
%
*/
static Image *ReadJBIGImage(image_info)
ImageInfo
  *image_info;
{
#define MaxBufferSize  8192

  Image
    *image;

  int
    length,
    status,
    x,
    y;

  register RunlengthPacket
    *q;

  register unsigned char
    *p;

  register unsigned short
    index;

  size_t
    count;

  struct jbg_dec_state
    jbig_info;

  unsigned char
    bit,
    buffer[MaxBufferSize];

  unsigned int
    Byte,
    height,
    packets,
    width;

  /*
    Allocate image structure.
  */
  image=AllocateImage(image_info);
  if (image == (Image *) NULL)
    return((Image *) NULL);
  /*
    Open image file.
  */
  OpenImage(image,ReadBinaryType);
  if (image->file == (FILE *) NULL)
    PrematureExit("Unable to open file",image);
  /*
    Determine maximum width and height, e.g. 640x512.
  */
  width=65535L;
  height=65535L;
  if (image_info->size != (char *) NULL)
    (void) XParseGeometry(image_info->size,&x,&y,&width,&height);
  /*
    Read JBIG file.
  */
  jbg_dec_init(&jbig_info);
  jbg_dec_maxsize(&jbig_info,(unsigned long) width,(unsigned long) height);
  status=JBG_EAGAIN;
  do
  {
    length=fread(buffer,1,MaxBufferSize,image->file);
    if (length == 0)
      break;
    p=buffer;
    count=0;
    while ((length > 0) && ((status == JBG_EAGAIN) || (status == JBG_EOK)))
    {
      status=jbg_dec_in(&jbig_info,p,length,&count);
      p+=count;
      length-=count;
    }
  } while ((status == JBG_EAGAIN) || (status == JBG_EOK));
  /*
    Initialize image structure.
  */
  image->columns=jbg_dec_getwidth(&jbig_info);
  image->rows=jbg_dec_getheight(&jbig_info);
  image->packets=0;
  packets=Max((image->columns*image->rows+8) >> 4,1);
  image->pixels=(RunlengthPacket *) malloc(packets*sizeof(RunlengthPacket));
  if (image->pixels == (RunlengthPacket *) NULL)
    PrematureExit("Unable to allocate memory",image);
  /*
    Create colormap.
  */
  image->class=PseudoClass;
  image->colors=2;
  image->colormap=(ColorPacket *) malloc(image->colors*sizeof(ColorPacket));
  if (image->colormap == (ColorPacket *) NULL)
    PrematureExit("Unable to allocate memory",image);
  image->colormap[0].red=0;
  image->colormap[0].green=0;
  image->colormap[0].blue=0;
  image->colormap[1].red=MaxRGB;
  image->colormap[1].green=MaxRGB;
  image->colormap[1].blue=MaxRGB;
  /*
    Convert X bitmap image to runlength-encoded packets.
  */
  p=jbg_dec_getimage(&jbig_info,0);
  q=image->pixels;
  q->length=MaxRunlength;
  for (y=0; y < image->rows; y++)
  {
    bit=0;
    for (x=0; x < image->columns; x++)
    {
      if (bit == 0)
        Byte=(*p++);
      index=(Byte & 0x80) ? 0 : 1;
      if ((index == q->index) && ((int) q->length < MaxRunlength))
        q->length++;
      else
        {
          if (image->packets != 0)
            q++;
          image->packets++;
          if (image->packets == packets)
            {
              packets<<=1;
              image->pixels=(RunlengthPacket *) realloc((char *) image->pixels,
                packets*sizeof(RunlengthPacket));
              if (image->pixels == (RunlengthPacket *) NULL)
                {
                  jbg_dec_free(&jbig_info);
                  PrematureExit("Unable to allocate memory",image);
                }
              q=image->pixels+image->packets-1;
            }
          q->index=index;
          q->length=0;
        }
      bit++;
      Byte<<=1;
      if (bit == 8)
        bit=0;
    }
    ProgressMonitor(LoadImageText,y,image->rows);
  }
  jbg_dec_free(&jbig_info);
  SyncImage(image);
  image->pixels=(RunlengthPacket *)
    realloc((char *) image->pixels,image->packets*sizeof(RunlengthPacket));
  CloseImage(image);
  return(image);
}
#else
static Image *ReadJBIGImage(ImageInfo *image_info)
{
  Warning("JBIG library is not available",image_info->filename);
  return(ReadMIFFImage(image_info));
}
#endif

#ifdef HasJPEG
#include <setjmp.h>
#include "jpeglib.h"
#include "jerror.h"

static Image
  *image;

static jmp_buf
  error_recovery;

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%  R e a d J P E G I m a g e                                                  %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Function ReadJPEGImage reads a JPEG image file and returns it.  It allocates
%  the memory necessary for the new Image structure and returns a pointer to
%  the new image.
%
%  The format of the ReadJPEGImage routine is:
%
%      image=ReadJPEGImage(image_info)
%
%  A description of each parameter follows:
%
%    o image:  Function ReadJPEGImage returns a pointer to the image after
%      reading.  A null image is returned if there is a a memory shortage or
%      if the image cannot be read.
%
%    o filename:  Specifies the name of the jpeg image to read.
%
%
*/
static int
  CommentHandler _Declare((j_decompress_ptr));

static unsigned int
  GetCharacter _Declare((j_decompress_ptr));

static void
  EmitMessage _Declare((j_common_ptr,int));

static unsigned int GetCharacter(j_decompress_ptr jpeg_info)
{
  struct jpeg_source_mgr
    *data;

  data=jpeg_info->src;
  if (data->bytes_in_buffer == 0)
    (*data->fill_input_buffer) (jpeg_info);
  data->bytes_in_buffer--;
  return(GETJOCTET(*data->next_input_byte++));
}

static int CommentHandler(j_decompress_ptr jpeg_info)
{
  long int
    length;

  register char
    *p;

  /*
    Determine length of comment.
  */
  length=GetCharacter(jpeg_info) << 8;
  length+=GetCharacter(jpeg_info);
  length-=2;
  if (image->comments != (char *) NULL)
    image->comments=(char *) realloc((char *) image->comments,
      (unsigned int) (strlen(image->comments)+length+1)*sizeof(char));
  else
    {
      image->comments=(char *)
        malloc((unsigned int) (length+1)*sizeof(char));
      if (image->comments != (char *) NULL)
        *image->comments='\0';
    }
  if (image->comments == (char *) NULL)
    {
      Warning("Memory allocation error",(char *) NULL);
      return(False);
    }
  /*
    Read comment.
  */
  p=image->comments+strlen(image->comments);
  while (--length >= 0)
    *p++=GetCharacter(jpeg_info);
  *p='\0';
  return(True);
}

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
      if ((jpeg_error->num_warnings == 0) || (jpeg_error->trace_level >= 3))
        Warning((char *) message,image->filename);
      jpeg_error->num_warnings++;
    }
  else
    if (jpeg_error->trace_level >= level)
      Warning((char *) message,image->filename);
}

static void ErrorExit(j_common_ptr jpeg_info)
{
  EmitMessage(jpeg_info,0);
  longjmp(error_recovery,1);
}

static Image *ReadJPEGImage(ImageInfo *image_info)
{
  int
    y;

  JSAMPLE
    *jpeg_pixels;

  JSAMPROW
    scanline[1];

  Quantum
    blue,
    green,
    red;

  register int
    i,
    x;

  register JSAMPLE
    *p;

  register RunlengthPacket
    *q;

  struct jpeg_decompress_struct
    jpeg_info;

  struct jpeg_error_mgr
    jpeg_error;

  unsigned int
    packets;

  unsigned short
    index;

  /*
    Allocate image structure.
  */
  image=AllocateImage(image_info);
  if (image == (Image *) NULL)
    return((Image *) NULL);
  /*
    Open image file.
  */
  OpenImage(image,ReadBinaryType);
  if (image->file == (FILE *) NULL)
    PrematureExit("Unable to open file",image);
  /*
    Initialize image structure.
  */
  jpeg_info.err=jpeg_std_error(&jpeg_error);
  jpeg_info.err->emit_message=EmitMessage;
  jpeg_info.err->error_exit=ErrorExit;
  image->pixels=(RunlengthPacket *) NULL;
  jpeg_pixels=(JSAMPLE *) NULL;
  if (setjmp(error_recovery))
    {
      /*
        JPEG image is corrupt.
      */
      if (jpeg_pixels != (JSAMPLE *) NULL)
        {
          free((char *) jpeg_pixels);
          jpeg_destroy_decompress(&jpeg_info);
        }
      DestroyImage(image);
      return((Image *) NULL);
    }
  jpeg_create_decompress(&jpeg_info);
#ifdef KJB_CPLUSPLUS
#warning "Commenting out code just so we can compile with c++"
#else 
  jpeg_set_marker_processor(&jpeg_info,JPEG_COM,CommentHandler);
#endif 

  jpeg_stdio_src(&jpeg_info,image->file);
  (void) jpeg_read_header(&jpeg_info,True);
  if (image_info->size != (char *) NULL)
    {
      unsigned int
        height,
        width;

      unsigned long
        scale_factor;

      /*
        Let the JPEG library subsample for us.
      */
      jpeg_calc_output_dimensions(&jpeg_info);
      image->magick_columns=jpeg_info.output_width;
      image->magick_rows=jpeg_info.output_height;
      width=jpeg_info.output_width;
      height=jpeg_info.output_height;
      ParseImageGeometry(image_info->size,&width,&height);
      if (width == 0)
        width=1;
      scale_factor=UpShift(jpeg_info.output_width)/width;
      if (height == 0)
        height=1;
      if (scale_factor > (UpShift(jpeg_info.output_height)/height))
        scale_factor=UpShift(jpeg_info.output_height)/height;
      jpeg_info.scale_denom=DownShift(scale_factor);
      jpeg_calc_output_dimensions(&jpeg_info);
    }
#ifdef DCT_FLOAT_SUPPORTED
  jpeg_info.dct_method=JDCT_FLOAT;
#endif
  jpeg_start_decompress(&jpeg_info);
  image->columns=jpeg_info.output_width;
  image->rows=jpeg_info.output_height;
  image->packets=0;
  packets=Max((image->columns*image->rows+2) >> 2,1);
  image->pixels=(RunlengthPacket *) malloc(packets*sizeof(RunlengthPacket));
  jpeg_pixels=(JSAMPLE *)
    malloc(jpeg_info.output_components*image->columns*sizeof(JSAMPLE));
  if ((image->pixels == (RunlengthPacket *) NULL) ||
      (jpeg_pixels == (JSAMPLE *) NULL))
    PrematureExit("Unable to allocate memory",image);
  if (jpeg_info.out_color_space == JCS_GRAYSCALE)
    {
      /*
        Initialize grayscale colormap.
      */
      image->class=PseudoClass;
      image->colors=256;
      image->colormap=(ColorPacket *) malloc(image->colors*sizeof(ColorPacket));
      if (image->colormap == (ColorPacket *) NULL)
        PrematureExit("Unable to allocate memory",image);
      for (i=0; i < image->colors; i++)
      {
        image->colormap[i].red=UpScale(i);
        image->colormap[i].green=UpScale(i);
        image->colormap[i].blue=UpScale(i);
      }
    }
  /*
    Convert JPEG pixels to runlength-encoded packets.
  */
  red=0;
  green=0;
  blue=0;
  index=0;
  scanline[0]=(JSAMPROW) jpeg_pixels;
  q=image->pixels;
  q->length=MaxRunlength;
  for (y=0; y < image->rows; y++)
  {
    (void) jpeg_read_scanlines(&jpeg_info,scanline,1);
    p=jpeg_pixels;
    for (x=0; x < image->columns; x++)
    {
      if (jpeg_info.data_precision > QuantumDepth)
        {
          if (jpeg_info.out_color_space == JCS_GRAYSCALE)
            index=GETJSAMPLE(*p++) >> 4;
          else
            {
              red=(Quantum) (GETJSAMPLE(*p++) >> 4);
              green=(Quantum) (GETJSAMPLE(*p++) >> 4);
              blue=(Quantum) (GETJSAMPLE(*p++) >> 4);
              if (jpeg_info.out_color_space == JCS_CMYK)
                index=(Quantum) (GETJSAMPLE(*p++) >> 4);
            }
         }
       else
         if (jpeg_info.out_color_space == JCS_GRAYSCALE)
           index=GETJSAMPLE(*p++);
         else
           {
             red=(Quantum) UpScale(GETJSAMPLE(*p++));
             green=(Quantum) UpScale(GETJSAMPLE(*p++));
             blue=(Quantum) UpScale(GETJSAMPLE(*p++));
             if (jpeg_info.out_color_space == JCS_CMYK)
               index=(Quantum) UpScale(GETJSAMPLE(*p++));
           }
      if (jpeg_info.out_color_space == JCS_CMYK)
        {
          index=MAXJSAMPLE-index;
          if ((int) (red-index) < 0)
            red=0;
          else
            red-=index;
          if ((int) (green-index) < 0)
            green=0;
          else
            green-=index;
          if ((int) (blue-index) < 0)
            blue=0;
          else
            blue-=index;
          index=0;
        }
      if ((red == q->red) && (green == q->green) && (blue == q->blue) &&
          (index == q->index) && ((int) q->length < MaxRunlength))
        q->length++;
      else
        {
          if (image->packets != 0)
            q++;
          image->packets++;
          if (image->packets == packets)
            {
              packets<<=1;
              image->pixels=(RunlengthPacket *)
                realloc((char *) image->pixels,packets*sizeof(RunlengthPacket));
              if (image->pixels == (RunlengthPacket *) NULL)
                {
                  free((char *) jpeg_pixels);
                  jpeg_destroy_decompress(&jpeg_info);
                  PrematureExit("Unable to allocate memory",image);
                }
              q=image->pixels+image->packets-1;
            }
          q->red=red;
          q->green=green;
          q->blue=blue;
          q->index=index;
          q->length=0;
        }
    }
    ProgressMonitor(LoadImageText,y,image->rows);
  }
  image->pixels=(RunlengthPacket *)
    realloc((char *) image->pixels,image->packets*sizeof(RunlengthPacket));
  if (image->class == PseudoClass)
    {
      SyncImage(image);
      CompressColormap(image);
    }
  /*
    Free memory.
  */
  free((char *) jpeg_pixels);
  (void) jpeg_finish_decompress(&jpeg_info);
  jpeg_destroy_decompress(&jpeg_info);
  CloseImage(image);
  return(image);
}
#else
static Image *ReadJPEGImage(ImageInfo* image_info)
{
  Warning("JPEG library is not available",image_info->filename);
  return(ReadMIFFImage(image_info));
}
#endif

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%  R e a d L O G O I m a g e                                                  %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Function ReadLOGOImage reads a LOGO image file and returns it.  It
%  allocates the memory necessary for the new Image structure and returns a
%  pointer to the new image.
%
%  The format of the ReadLOGOImage routine is:
%
%      image=ReadLOGOImage(image_info)
%
%  A description of each parameter follows:
%
%    o image:  Function ReadLOGOImage returns a pointer to the image after
%      reading.  A null image is returned if there is a a memory shortage or
%      if the image cannot be read.
%
%    o image_info: Specifies a pointer to an ImageInfo structure.
%
%
*/
static Image *ReadLOGOImage(ImageInfo *image_info)
{
#include "magick/logo.h"

  char
    *filename,
    logo_filename[MaxTextLength];

  FILE
    *file;

  Image
    *image;

  register int
    i;

  register unsigned char
    *p;

  /*
    Open temporary output file.
  */
  TemporaryFilename(logo_filename);
  file=fopen(logo_filename,"w");
  if (file == (FILE *) NULL)
    {
      Warning("Unable to write file",logo_filename);
      return(ReadXCImage(image_info));
    }
  p=LogoImage;
  for (i=0; i < LogoImageLength; i++)
  {
    (void) fputc((char) *p,file);
    p++;
  }
  if (ferror(file))
    {
      Warning("An error has occurred writing to file",logo_filename);
      (void) fclose(file);
      (void) unlink(logo_filename);
      return(ReadXCImage(image_info));
    }
  (void) fclose(file);
  filename=image_info->filename;
  image_info->filename=logo_filename;
  image=ReadMIFFImage(image_info);
  image_info->filename=filename;
  if (image != (Image *) NULL)
    (void) strcpy(image->filename,image_info->filename);
  (void) unlink(logo_filename);
  return(image);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%  R e a d M A P I m a g e                                                    %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Function ReadMAPImage reads an image of raw RGB colormap and colormap index
%  bytes and returns it.  It allocates the memory necessary for the new Image
%  structure and returns a pointer to the new image.
%
%  The format of the ReadMAPImage routine is:
%
%      image=ReadMAPImage(image_info)
%
%  A description of each parameter follows:
%
%    o image:  Function ReadMAPImage returns a pointer to the image after
%      reading.  A null image is returned if there is a a memory shortage or
%      if the image cannot be read.
%
%    o image_info: Specifies a pointer to an ImageInfo structure.
%
%
*/
static Image *ReadMAPImage(ImageInfo *image_info)
{
  Image
    *image;

  int
    colors;

  register int
    i;

  register unsigned char
    *p;

  unsigned char
    *colormap;

  unsigned int
    height,
    packet_size,
    status,
    width;

  unsigned short
    value;

  /*
    Allocate image structure.
  */
  image=AllocateImage(image_info);
  if (image == (Image *) NULL)
    return((Image *) NULL);
  /*
    Open image file.
  */
  OpenImage(image,ReadBinaryType);
  if (image->file == (FILE *) NULL)
    PrematureExit("Unable to open file",image);
  /*
    Determine width, height, and number of colors, e.g. 640x512+256.
  */
  width=512;
  height=512;
  colors=256;
  if (image_info->size != (char *) NULL)
    (void) XParseGeometry(image_info->size,&colors,&colors,&width,&height);
  /*
    Initialize image structure.
  */
  image->class=PseudoClass;
  image->compression=NoCompression;
  image->columns=width;
  image->rows=height;
  image->colors=colors;
  image->packets=image->columns*image->rows;
  packet_size=3*(image->depth >> 3);
  colormap=(unsigned char *)
    malloc(packet_size*image->colors*sizeof(unsigned char));
  image->colormap=(ColorPacket *) malloc(image->colors*sizeof(ColorPacket));
  image->packed_pixels=(unsigned char *)
    malloc(image->packets*packet_size*(image->depth >> 3));
  if ((colormap == (unsigned char *) NULL) ||
      (image->colormap == (ColorPacket *) NULL))
    PrematureExit("Unable to allocate memory",image);
  /*
    Read image colormap.
  */
  (void) ReadData((char *) colormap,1,image->colors*packet_size,image->file);
  p=colormap;
  for (i=0; i < image->colors; i++)
  {
    ReadQuantum(image->colormap[i].red,p);
    ReadQuantum(image->colormap[i].green,p);
    ReadQuantum(image->colormap[i].blue,p);
  }
  free((char *) colormap);
  /*
    Convert raster image to runlength-encoded packets.
  */
  packet_size=1;
  if (image->colors > 256)
    packet_size++;
  if (image->packed_pixels != (unsigned char *) NULL)
    free((char *) image->packed_pixels);
  image->packed_pixels=(unsigned char *)
    malloc(image->packets*packet_size);
  if (image->packed_pixels == (unsigned char *) NULL)
    PrematureExit("Unable to allocate memory",image);
  (void) ReadData((char *) image->packed_pixels,packet_size,image->packets,
    image->file);
  status=RunlengthDecodeImage(image);
  if (status == False)
    {
      DestroyImages(image);
      return((Image *) NULL);
    }
  CompressImage(image);
  CloseImage(image);
  return(image);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%  R e a d M A T T E I m a g e                                                %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Function ReadMATTEImage reads an image of raw matte bytes and returns it.
%  It allocates the memory necessary for the new Image structure and returns a
%  pointer to the new image.
%
%  The format of the ReadMATTEImage routine is:
%
%      image=ReadMATTEImage(image_info)
%
%  A description of each parameter follows:
%
%    o image:  Function ReadMATTEImage returns a pointer to the image after
%      reading.  A null image is returned if there is a a memory shortage or
%      if the image cannot be read.
%
%    o image_info: Specifies a pointer to an ImageInfo structure.
%
%
*/
static Image *ReadMATTEImage(ImageInfo *image_info)
{
  Image
    *image;

  int
    x,
    y;

  register int
    i;

  register RunlengthPacket
    *q;

  register unsigned char
    index,
    *p;

  unsigned char
    *matte_pixels;

  unsigned int
    height,
    width;

  /*
    Allocate image structure.
  */
  image=AllocateImage(image_info);
  if (image == (Image *) NULL)
    return((Image *) NULL);
  /*
    Open image file.
  */
  OpenImage(image,ReadBinaryType);
  if (image->file == (FILE *) NULL)
    PrematureExit("Unable to open file",image);
  /*
    Determine width and height, e.g. 640x512.
  */
  width=512;
  height=512;
  x=0;
  if (image_info->size != (char *) NULL)
    (void) XParseGeometry(image_info->size,&x,&y,&width,&height);
  for (i=0; i < x; i++)
    (void) fgetc(image->file);
  /*
    Initialize image structure.
  */
  image->matte=True;
  image->columns=width;
  image->rows=height;
  image->packets=image->columns*image->rows;
  matte_pixels=(unsigned char *) malloc(image->packets*sizeof(unsigned char));
  image->pixels=(RunlengthPacket *)
    malloc(image->packets*sizeof(RunlengthPacket));
  if ((matte_pixels == (unsigned char *) NULL) ||
      (image->pixels == (RunlengthPacket *) NULL))
    PrematureExit("Unable to allocate memory",image);
  /*
    Convert raster image to runlength-encoded packets.
  */
  (void) ReadData((char *) matte_pixels,1,image->packets,image->file);
  p=matte_pixels;
  q=image->pixels;
  for (y=0; y < image->rows; y++)
  {
    for (x=0; x < image->columns; x++)
    {
      index=(*p++);
      q->red=0;
      q->green=0;
      q->blue=0;
      q->index=(unsigned short) index;
      q->length=0;
      q++;
    }
    ProgressMonitor(LoadImageText,y,image->rows);
  }
  free((char *) matte_pixels);
  CompressImage(image);
  CloseImage(image);
  return(image);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   R e a d M I F F I m a g e                                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Function ReadMIFFImage reads a MIFF image file and returns it.  It
%  allocates the memory necessary for the new Image structure and returns a
%  pointer to the new image.
%
%  The format of the ReadMIFFImage routine is:
%
%      image=ReadMIFFImage(filename)
%
%  A description of each parameter follows:
%
%    o image: Function ReadMIFFImage returns a pointer to the image after
%      reading.  A null image is returned if there is a a memory shortage or
%      if the image cannot be read.
%
%    o image_info: Specifies a pointer to an ImageInfo structure.
%
%
*/
#ifdef HasPNG
#include "zlib.h"
#endif

static Image *ReadMIFFImage(ImageInfo *image_info)
{
  char
    keyword[MaxTextLength],
    value[MaxTextLength];

  Image
    *image;

  register int
    c,
    i;

  register unsigned char
    *p;

  unsigned int
    length,
    packet_size,
    status;

  unsigned long
    count,
    packets;


  /*
    Allocate image structure.
  */
  image=AllocateImage(image_info);
  if (image == (Image *) NULL)
    return((Image *) NULL);
  /*
    Open image file.
  */
  OpenImage(image,ReadBinaryType);
  if (image->file == (FILE *) NULL)
    PrematureExit("Unable to open file",image);
  image->depth=8;
  /*
    Decode image header;  header terminates one character beyond a ':'.
  */
  c=fgetc(image->file);
  if (c == EOF)
    {
      DestroyImage(image);
      return((Image *) NULL);
    }
  do
  {
    /*
      Decode image header;  header terminates one character beyond a ':'.
    */
    image->compression=NoCompression;
    while (isgraph(c) && (c != ':'))
    {
      register char
        *p;

      if (c == '{')
        {
          /*
            Read comment-- any text between { }.
          */
          if (image->comments != (char *) NULL)
            {
              length=strlen(image->comments);
              p=image->comments+length;
            }
          else
            {
              length=MaxTextLength;
              image->comments=(char *) malloc(length*sizeof(char));
              p=image->comments;
            }
          for ( ; image->comments != (char *) NULL; p++)
          {
            c=fgetc(image->file);
            if ((c == EOF) || (c == '}'))
              break;
            if ((p-image->comments+1) >= length)
              {
                *p='\0';
                length<<=1;
                image->comments=(char *)
                  realloc((char *) image->comments,length*sizeof(char));
                if (image->comments == (char *) NULL)
                  break;
                p=image->comments+strlen(image->comments);
              }
            *p=(unsigned char) c;
          }
          if (image->comments == (char *) NULL)
            PrematureExit("Unable to allocate memory",image);
          *p='\0';
          c=fgetc(image->file);
        }
      else
        if (isalnum(c))
          {
            /*
              Determine a keyword and its value.
            */
            p=keyword;
            do
            {
              if ((p-keyword) < (MaxTextLength-1))
                *p++=(char) c;
              c=fgetc(image->file);
            } while (isalnum(c));
            *p='\0';
            while (isspace(c) || (c == '='))
              c=fgetc(image->file);
            p=value;
            while (!isspace(c) && (c != EOF))
            {
              if ((p-value) < (MaxTextLength-1))
                *p++=(char) c;
              c=fgetc(image->file);
            }
            *p='\0';
            /*
              Assign a value to the specified keyword.
            */
            if (strcmp(keyword,"class") == 0)
              if (strcmp(value,"PseudoClass") == 0)
                image->class=PseudoClass;
              else
                if (strcmp(value,"DirectClass") == 0)
                  image->class=DirectClass;
                else
                  image->class=UndefinedClass;
            if (strcmp(keyword,"colors") == 0)
              image->colors=(unsigned int) atoi(value);
            if (strcmp(keyword,"compression") == 0)
              if (strcmp(value,"Zlib") == 0)
                image->compression=ZlibCompression;
              else
                if (strcmp(value,"RunlengthEncoded") == 0)
                  image->compression=RunlengthEncodedCompression;
                else
                  image->compression=UndefinedCompression;
            if (strcmp(keyword,"columns") == 0)
              image->columns=(unsigned int) atoi(value);
            if (strcmp(keyword,"depth") == 0)
              image->depth=atoi(value) <= 8 ? 8 : 16;
            if (strcmp(keyword,"gamma") == 0)
              image->gamma=atof(value);
            if (strcmp(keyword,"id") == 0)
              if (strcmp(value,"ImageMagick") == 0)
                image->id=ImageMagickId;
              else
                image->id=UndefinedId;
            if ((strcmp(keyword,"matte") == 0) ||
                (strcmp(keyword,"alpha") == 0))
              if ((strcmp(value,"True") == 0) || (strcmp(value,"true") == 0))
                image->matte=True;
              else
                image->matte=False;
            if (strcmp(keyword,"montage") == 0)
              {
                image->montage=(char *) malloc(strlen(value)+1*sizeof(char));
                if (image->montage == (char *) NULL)
                  PrematureExit("Unable to allocate memory",image);
                (void) strcpy(image->montage,value);
              }
            if (strcmp(keyword,"packets") == 0)
              image->packets=(unsigned int) atoi(value);
            if (strcmp(keyword,"rows") == 0)
              image->rows=(unsigned int) atoi(value);
            if (strcmp(keyword,"scene") == 0)
              image->scene=(unsigned int) atoi(value);
            if (strcmp(keyword,"signature") == 0)
              {
                image->signature=(char *)
                  malloc((strlen(value)+1)*sizeof(char));
                if (image->signature == (char *) NULL)
                  PrematureExit("Unable to allocate memory",image);
                (void) strcpy(image->signature,value);
              }
          }
        else
          c=fgetc(image->file);
      while (isspace(c))
        c=fgetc(image->file);
    }
    (void) fgetc(image->file);
    /*
      Verify that required image information is defined.
    */
    if ((image->id == UndefinedId) || (image->class == UndefinedClass) ||
        (image->compression == UndefinedCompression) || (image->columns == 0) ||
        (image->rows == 0))
      PrematureExit("Incorrect image header in file",image);
    if (image->montage != (char *) NULL)
      {
        register char
          *p;

        /*
          Image directory.
        */
        image->directory=(char *) malloc(MaxTextLength*sizeof(char));
        if (image->directory == (char *) NULL)
          PrematureExit("Unable to read image data",image);
        p=image->directory;
        do
        {
          *p='\0';
          if ((((int) strlen(image->directory)+1) % MaxTextLength) == 0)
            {
              /*
                Allocate more memory for the image directory.
              */
              image->directory=(char *) realloc((char *) image->directory,
                (strlen(image->directory)+MaxTextLength+1)*sizeof(char));
              if (image->directory == (char *) NULL)
                PrematureExit("Unable to read image data",image);
              p=image->directory+strlen(image->directory);
            }
          c=fgetc(image->file);
          *p++=(unsigned char) c;
        } while (c != '\0');
      }
    if (image->class == PseudoClass)
      {
        unsigned int
          colors;

        unsigned short
          value;

        /*
          PseudoClass image cannot have matte data.
        */
        if (image->matte)
          PrematureExit("Matte images must be DirectClass",image);
        /*
          Create image colormap.
        */
        colors=image->colors;
        if (colors == 0)
          colors=256;
        image->colormap=(ColorPacket *) malloc(colors*sizeof(ColorPacket));
        if (image->colormap == (ColorPacket *) NULL)
          PrematureExit("Unable to allocate memory",image);
        if (image->colors == 0)
          for (i=0; i < colors; i++)
          {
            image->colormap[i].red=(Quantum) UpScale(i);
            image->colormap[i].green=(Quantum) UpScale(i);
            image->colormap[i].blue=(Quantum) UpScale(i);
            image->colors++;
          }
        else
          {
            unsigned char
              *colormap;

            /*
              Read image colormap from file.
            */
            packet_size=3*(image->depth >> 3);
            colormap=(unsigned char *)
              malloc(packet_size*image->colors*sizeof(unsigned char));
            if (colormap == (unsigned char *) NULL)
              PrematureExit("Unable to allocate memory",image);
            (void) ReadData((char *) colormap,1,packet_size*image->colors,
              image->file);
            p=colormap;
            for (i=0; i < image->colors; i++)
            {
              ReadQuantum(image->colormap[i].red,p);
              ReadQuantum(image->colormap[i].green,p);
              ReadQuantum(image->colormap[i].blue,p);
            }
            free((char *) colormap);
          }
      }
    /*
      Determine packed packet size.
    */
    if (image->class == PseudoClass)
      {
        image->packet_size=1;
        if (image->colors > 256)
          image->packet_size++;
      }
    else
      {
        image->packet_size=3*(image->depth >> 3);
        if (image->matte)
          image->packet_size++;
      }
    if (image->compression == RunlengthEncodedCompression)
      image->packet_size++;
    packet_size=image->packet_size;
    if (image->compression == ZlibCompression)
      packet_size=1;
    /*
      Allocate image pixels.
    */
    if (image->compression == NoCompression)
      image->packets=image->columns*image->rows;
    packets=image->packets;
    if (image->packets == 0)
      packets=image->columns*image->rows;
    image->packed_pixels=(unsigned char *)
      malloc((unsigned int) packets*packet_size*sizeof(unsigned char));
    if (image->packed_pixels == (unsigned char *) NULL)
      PrematureExit("Unable to allocate memory",image);
    /*
      Read image pixels from file.
    */
    if ((image->compression != RunlengthEncodedCompression) ||
        (image->packets != 0))
      (void) ReadData((char *) image->packed_pixels,1,(unsigned int)
        packets*packet_size,image->file);
    else
      {
        /*
          Number of runlength packets is unspecified.
        */
        count=0;
        p=image->packed_pixels;
        do
        {
          (void) ReadData((char *) p,1,packet_size,image->file);
          image->packets++;
          p+=(packet_size-1);
          count+=(*p+1);
          p++;
        }
        while (count < (image->columns*image->rows));
      }
    if (image->compression ==  ZlibCompression)
      {
        int
          status;

        unsigned char
          *compressed_pixels;

        /*
          Uncompress image pixels with Zlib encoding.
        */
        image->packets=image->columns*image->rows;
        compressed_pixels=image->packed_pixels;
        packets=image->packets*image->packet_size;
        image->packed_pixels=(unsigned char *)
          malloc(packets*sizeof(unsigned char));
        if (image->packed_pixels == (unsigned char *) NULL)
          PrematureExit("Unable to allocate memory",image);
        status=True;
#ifdef HasPNG
        status=uncompress(image->packed_pixels,&packets,compressed_pixels,
          image->packets);
#endif
        free((char *) compressed_pixels);
        if (status)
          PrematureExit("Unable to Zlib uncompress image",image);
      }
    /*
      Unpack the packed image pixels into runlength-encoded pixel packets.
    */
    status=RunlengthDecodeImage(image);
    if (status == False)
      {
        DestroyImages(image);
        return((Image *) NULL);
      }
    /*
      Proceed to next image.
    */
    do
    {
      c=fgetc(image->file);
    } while (!isgraph(c) && (c != EOF));
    if (c != EOF)
      {
        /*
          Allocate image structure.
        */
        image->next=AllocateImage(image_info);
        if (image->next == (Image *) NULL)
          {
            DestroyImages(image);
            return((Image *) NULL);
          }
        (void) strcpy(image->next->filename,image_info->filename);
        image->next->file=image->file;
        image->next->scene=image->scene+1;
        image->next->previous=image;
        image=image->next;
      }
  } while (c != EOF);
  while (image->previous != (Image *) NULL)
    image=image->previous;
  CloseImage(image);
  return(image);
}
#ifdef HasMPEG
#undef BitmapPad
#include "mpeg.h"

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%  R e a d M P E G I m a g e                                                  %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Function ReadMPEGImage reads a MPEG image file and returns it.  It
%  allocates the memory necessary for the new Image structure and returns a
%  pointer to the new image.
%
%  The format of the ReadMPEGImage routine is:
%
%      image=ReadMPEGImage(image_info)
%
%  A description of each parameter follows:
%
%    o image:  Function ReadMPEGImage returns a pointer to the image after
%      reading. A null image is returned if there is a a memory shortage or if
%      the image cannot be read.
%
%    o image_info: Specifies a pointer to an ImageInfo structure.
%
%
*/
static Image *ReadMPEGImage(image_info)
ImageInfo
  *image_info;
{
  Image
    *image;

  ImageDesc
    mpeg_info;

  int
    y;

  register int
    i,
    x;

  register unsigned char
    *p;

  register RunlengthPacket
    *q;

  unsigned char
    *mpeg_pixels;

  unsigned int
    number_frames,
    status;

  unsigned long
    lsb_first;

  /*
    Allocate image structure.
  */
  image=AllocateImage(image_info);
  if (image == (Image *) NULL)
    return((Image *) NULL);
  /*
    Open image file.
  */
  OpenImage(image,ReadBinaryType);
  if (image->file == (FILE *) NULL)
    PrematureExit("Unable to open file",image);
  /*
    Allocate MPEG pixels.
  */
  (void) OpenMPEG(image->file,&mpeg_info);
  number_frames=8*image->filesize/mpeg_info.Width/mpeg_info.Height;
  mpeg_pixels=(unsigned char *) malloc(mpeg_info.Size*sizeof(unsigned char));
  if (mpeg_pixels == (unsigned char *) NULL)
    PrematureExit("Unable to allocate memory",image);
  /*
    Read MPEG image.
  */
  status=GetMPEGFrame((char *) mpeg_pixels);
  if (image_info->subimage != 0)
    while (image->scene < image_info->subimage)
    {
       image->scene++;
       status=GetMPEGFrame((char *) mpeg_pixels);
       if (status == False)
         break;
       ProgressMonitor(LoadImageText,image->scene,number_frames);
    }
  if (status == False)
    {
      free((char *) mpeg_pixels);
      PrematureExit("Corrupt MPEG image",image);
    }
  while (status == True)
  {
    /*
      Initialize image structure.
    */
    image->columns=mpeg_info.Width;
    image->rows=mpeg_info.Height-8;
    image->packets=image->columns*image->rows;
    image->pixels=(RunlengthPacket *)
      malloc(image->packets*sizeof(RunlengthPacket));
    if (image->pixels == (RunlengthPacket *) NULL)
      PrematureExit("Unable to allocate memory",image);
    /*
      Convert MPEG raster image to runlength-encoded packets.
    */
    p=mpeg_pixels;
    q=image->pixels;
    lsb_first=1;
    if (*(char *) &lsb_first)
      for (y=0; y < image->rows; y++)
      {
        for (x=0; x < image->columns; x++)
        {
          q->red=UpScale(*p++);
          q->green=UpScale(*p++);
          q->blue=UpScale(*p++);
          q->length=0;
          p++;
          q++;
        }
      }
    else
      for (y=0; y < image->rows; y++)
      {
        for (x=0; x < image->columns; x++)
        {
          p++;
          q->blue=UpScale(*p++);
          q->green=UpScale(*p++);
          q->red=UpScale(*p++);
          q->length=0;
          q++;
        }
      }
    if (image_info->verbose)
      DescribeImage(image,stderr,False);
    CompressImage(image);
    if (image_info->subimage != 0)
      if (image->scene >= (image_info->subimage+image_info->subrange-1))
        break;
    status=GetMPEGFrame((char *) mpeg_pixels);
    if (status == True)
      {
        /*
          Allocate image structure.
        */
        image->next=AllocateImage(image_info);
        if (image->next == (Image *) NULL)
          {
            DestroyImages(image);
            return((Image *) NULL);
          }
        (void) strcpy(image->next->filename,image_info->filename);
        image->next->file=image->file;
        image->next->scene=image->scene+1;
        image->next->previous=image;
        image=image->next;
      }
    if ((status == False) || (image->scene == number_frames))
      number_frames=image->scene+1;
    ProgressMonitor(LoadImageText,image->scene,number_frames);
  }
  free((char *) mpeg_pixels);
  while (image->previous != (Image *) NULL)
    image=image->previous;
  CloseImage(image);
  return(image);
}
#else
static Image *ReadMPEGImage(ImageInfo *image_info)
{
  Warning("MPEG library is not available",image_info->filename);
  return(ReadMIFFImage(image_info));
}
#endif

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%  R e a d M T V I m a g e                                                    %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Function ReadMTVImage reads a MTV image file and returns it.  It allocates
%  the memory necessary for the new Image structure and returns a pointer to
%  the new image.
%
%  The format of the ReadMTVImage routine is:
%
%      image=ReadMTVImage(image_info)
%
%  A description of each parameter follows:
%
%    o image:  Function ReadMTVImage returns a pointer to the image after
%      reading.  A null image is returned if there is a a memory shortage or
%      if the image cannot be read.
%
%    o image_info: Specifies a pointer to an ImageInfo structure.
%
%
*/
static Image *ReadMTVImage(ImageInfo *image_info)
{
  Image
    *image;

  int
    count,
    y;

  Quantum
    blue,
    green,
    red;

  register int
    x;

  register RunlengthPacket
    *q;

  unsigned int
    columns,
    packets,
    rows;

  /*
    Allocate image structure.
  */
  image=AllocateImage(image_info);
  if (image == (Image *) NULL)
    return((Image *) NULL);
  /*
    Open image file.
  */
  OpenImage(image,ReadBinaryType);
  if (image->file == (FILE *) NULL)
    PrematureExit("Unable to open file",image);
  /*
    Read MTV image.
  */
  count=fscanf(image->file,"%u %u\n",&columns,&rows);
  if (count == 0)
    PrematureExit("Not a MTV image file",image);
  do
  {
    /*
      Initialize image structure.
    */
    image->columns=columns;
    image->rows=rows;
    image->packets=0;
    packets=Max((image->columns*image->rows+4) >> 3,1);
    image->pixels=(RunlengthPacket *) malloc(packets*sizeof(RunlengthPacket));
    if (image->pixels == (RunlengthPacket *) NULL)
      PrematureExit("Unable to allocate memory",image);
    /*
      Convert MTV raster image to runlength-encoded packets.
    */
    q=image->pixels;
    q->length=MaxRunlength;
    for (y=0; y < image->rows; y++)
    {
      for (x=0; x < image->columns; x++)
      {
        red=UpScale(fgetc(image->file));
        green=UpScale(fgetc(image->file));
        blue=UpScale(fgetc(image->file));
        if ((red == q->red) && (green == q->green) && (blue == q->blue) &&
            ((int) q->length < MaxRunlength))
          q->length++;
        else
          {
            if (image->packets != 0)
              q++;
            image->packets++;
            if (image->packets == packets)
              {
                packets<<=1;
                image->pixels=(RunlengthPacket *) realloc((char *)
                  image->pixels,packets*sizeof(RunlengthPacket));
                if (image->pixels == (RunlengthPacket *) NULL)
                  PrematureExit("Unable to allocate memory",image);
                q=image->pixels+image->packets-1;
              }
            q->red=red;
            q->green=green;
            q->blue=blue;
            q->index=0;
            q->length=0;
          }
      }
      ProgressMonitor(LoadImageText,y,image->rows);
    }
    image->pixels=(RunlengthPacket *)
      realloc((char *) image->pixels,image->packets*sizeof(RunlengthPacket));
    /*
      Proceed to next image.
    */
    count=fscanf(image->file,"%u %u\n",&columns,&rows);
    if (count > 0)
      {
        /*
          Allocate next image structure.
        */
        image->next=AllocateImage(image_info);
        if (image->next == (Image *) NULL)
          {
            DestroyImages(image);
            return((Image *) NULL);
          }
        (void) strcpy(image->next->filename,image_info->filename);
        image->next->file=image->file;
        image->next->scene=image->scene+1;
        image->next->previous=image;
        image=image->next;
      }
  } while (count > 0);
  while (image->previous != (Image *) NULL)
    image=image->previous;
  CloseImage(image);
  return(image);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%  R e a d N U L L I m a g e                                                  %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Function ReadNULLImage reads a NULL image file and returns it.  It
%  allocates the memory necessary for the new Image structure and returns a
%  pointer to the new image.
%
%  The format of the ReadNULLImage routine is:
%
%      image=ReadNULLImage(image_info)
%
%  A description of each parameter follows:
%
%    o image:  Function ReadNULLImage returns a pointer to the image after
%      reading.  A null image is returned if there is a a memory shortage or
%      if the image cannot be read.
%
%    o image_info: Specifies a pointer to an ImageInfo structure.
%
%
*/
static Image *ReadNULLImage(ImageInfo *image_info)
{
  Image
    *image;

  Warning("Cannot read NULL images",image_info->filename);
  image=ReadMIFFImage(image_info);
  return(image);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%  R e a d P C D I m a g e                                                    %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Function ReadPCDImage reads a Photo CD image file and returns it.  It
%  allocates the memory necessary for the new Image structure and returns a
%  pointer to the new image.  Much of the PCD decoder was derived from
%  the program hpcdtoppm(1) by Hadmut Danisch.
%
%  The format of the ReadPCDImage routine is:
%
%      image=ReadPCDImage(image_info)
%
%  A description of each parameter follows:
%
%    o image:  Function ReadPCDImage returns a pointer to the image after
%      reading.  A null image is returned if there is a a memory shortage or
%      if the image cannot be read.
%
%    o image_info: Specifies a pointer to an ImageInfo structure.
%
%
*/

static void Upsample(unsigned int width, unsigned int height, unsigned int scaled_width, unsigned char *pixels)
{
  register int
    x,
    y;

  register unsigned char
    *p,
    *q,
    *r;

  /*
    Create a new image that is a integral size greater than an existing one.
  */
  for (y=0; y < height; y++)
  {
    p=pixels+(height-1-y)*scaled_width+(width-1);
    q=pixels+((height-1-y) << 1)*scaled_width+((width-1) << 1);
    *q=(*p);
    *(q+1)=(*(p));
    for (x=1; x < width; x++)
    {
      p--;
      q-=2;
      *q=(*p);
      *(q+1)=(((int) *p)+((int) *(p+1))+1) >> 1;
    }
  }
  for (y=0; y < (height-1); y++)
  {
    p=pixels+(y << 1)*scaled_width;
    q=p+scaled_width;
    r=q+scaled_width;
    for (x=0; x < (width-1); x++)
    {
      *q=(((int) *p)+((int) *r)+1) >> 1;
      *(q+1)=(((int) *p)+((int) *(p+2))+((int) *r)+((int) *(r+2))+2) >> 2;
      q+=2;
      p+=2;
      r+=2;
    }
    *q++=(((int) *p++)+((int) *r++)+1) >> 1;
    *q++=(((int) *p++)+((int) *r++)+1) >> 1;
  }
  p=pixels+(2*height-2)*scaled_width;
  q=pixels+(2*height-1)*scaled_width;
  for (x=0; x < width; x++)
  {
    *q++=(*p++);
    *q++=(*p++);
  }
}

static Image *OverviewImage(ImageInfo *image_info, Image *image, unsigned int number_images)
{
#define ClientName  "montage"

  char
    *resource_value;

  Display
    *display;

  Image
    **images;

  register int
    i,
    j;

  XMontageInfo
    montage_info;

  XResourceInfo
    resource_info;

  XrmDatabase
    resource_database;

  /*
    Allocate images array.
  */
  images=(Image **) malloc(number_images*sizeof(Image *));
  if (images == (Image **) NULL)
    {
      Warning("Memory allocation error",(char *) NULL);
      return((Image *) NULL);
    }
  /*
    Open X server connection.
  */
  resource_info.background_color=DefaultTileBackground;
  resource_info.border_color=BorderColor;;
  resource_info.border_width=atoi(DefaultTileBorderwidth);
  resource_info.foreground_color=DefaultTileForeground;
  resource_info.gravity=CenterGravity;
  resource_info.image_geometry=DefaultTileGeometry;
  resource_info.matte_color=DefaultTileMatte;
  resource_info.title=(char *) NULL;
  display=XOpenDisplay(image_info->server_name);
  if (display != (Display *) NULL)
    {
      /*
        Set our forgiving error handler.
      */
      XSetErrorHandler(XError);
      /*
        Get user defaults from X resource database.
      */
      resource_database=XGetResourceDatabase(display,client_name);
      XGetResourceInfo(resource_database,ClientName,&resource_info);
      resource_info.background_color=XGetResourceInstance(resource_database,
        ClientName,"background",DefaultTileBackground);
      resource_value=XGetResourceClass(resource_database,ClientName,
        "borderWidth",DefaultTileBorderwidth);
      resource_info.border_width=atoi(resource_value);
      resource_info.font=image_info->font;
      resource_info.foreground_color=XGetResourceInstance(resource_database,
        ClientName,"foreground",DefaultTileForeground);
      resource_info.image_geometry=XGetResourceInstance(resource_database,
        ClientName,"imageGeometry",DefaultTileGeometry);
      resource_info.matte_color=XGetResourceInstance(resource_database,
        ClientName,"mattecolor",DefaultTileMatte);
      XCloseDisplay(display);
    }
  /*
    Read each image and convert them to a tile.
  */
  j=0;
  for (i=0; i < number_images; i++)
  {
    LabelImage(image,"%f");
    TransformImage(&image,(char *) NULL,resource_info.image_geometry);
    if (image_info->verbose)
      DescribeImage(image,stderr,False);
    images[j]=image;
    image=image->next;
    j++;
  }
  /*
    Create the PCD Overview image.
  */
  XGetMontageInfo(&montage_info);
  montage_info.number_tiles=j;
  if (montage_info.number_tiles != 0)
    image=
      XMontageImage(images,&resource_info,&montage_info,image_info->filename);
  free((char *) images);
  if (image == (Image *) NULL)
    PrematureExit("Unable to allocate memory",image);
  CompressImage(image);
  return(image);
}

static Image *ReadPCDImage(ImageInfo *image_info)
{
  extern int kjb_pcd_size;

  Image
    *image;

  long int
    offset;

  register int
    i;

  register RunlengthPacket
    *p;

  register unsigned char
    *c1,
    *c2,
    *y;

  unsigned char
    *chroma1,
    *chroma2,
    *header,
    *luma;

  unsigned int
    height,
    number_images,
    overview,
    rotate,
    status,
    subimage,
    width;

  /*
    Allocate image structure.
  */
  image=AllocateImage(image_info);
  if (image == (Image *) NULL)
    return((Image *) NULL);
  /*
    Open image file.
  */
  OpenImage(image,ReadBinaryType);
  if (image->file == (FILE *) NULL)
    PrematureExit("Unable to open file",image);
  /*
    Determine if this is a PCD file.
  */
  header=(unsigned char *) malloc(3*0x800*sizeof(unsigned char));
  if (header == (unsigned char *) NULL)
    PrematureExit("Unable to allocate memory",image);
  status=ReadData((char *) header,1,3*0x800,image->file);
  overview=strncmp((char *) header,"PCD_OPA",7) == 0;
  if ((status == False) ||
      ((strncmp((char *) header+0x800,"PCD",3) != 0) && !overview))
    PrematureExit("Not a PCD image file",image);
  rotate=header[0x0e02] & 0x03;
  number_images=(header[10] << 8) | header[11];
  free((char *) header);

  subimage=3;
  if (image_info->subimage != 0)
    subimage=image_info->subimage;
  if (image_info->size != (char *) NULL)
    {
      int
        x,
        y;

      /*
        Determine which image size to extract.
      */
      width=768;
      height=512;
      (void) XParseGeometry(image_info->size,&x,&y,&width,&height);
      for (subimage=1; subimage <= 6; subimage++)
      {
        if ((width <= 192) && (height <= 128))
          break;
        width>>=1;
        height>>=1;
      }
    }
  if (overview)
    subimage=1;

  /*
    Initialize image structure.
  */
  width=192;
  height=128;
  for (i=1; i < Min(subimage,3); i++)
  {
    width<<=1;
    height<<=1;
  }
  image->columns=width;
  image->rows=height;
  for ( ; i < subimage; i++)
  {
    image->columns<<=1;
    image->rows<<=1;
  }
  /*
    Allocate luma and chroma memory.
  */
  image->packets=image->columns*image->rows;
  chroma1=(unsigned char *) malloc((image->packets+1)*sizeof(unsigned char));
  chroma2=(unsigned char *) malloc((image->packets+1)*sizeof(unsigned char));
  luma=(unsigned char *) malloc((image->packets+1)*sizeof(unsigned char));
  if ((chroma1 == (unsigned char *) NULL) ||
      (chroma2 == (unsigned char *) NULL) || (luma == (unsigned char *) NULL))
    PrematureExit("Unable to allocate memory",image);
  /*
    Advance to image data.
  */
  offset=93;
  if (overview)
    offset=2;
  else
    if (subimage == 2)
      offset=20;
    else
      if (subimage == 1)
        offset=1;
  for (i=0; i < (offset*0x800); i++)
    (void) fgetc(image->file);
  if (overview)
    {
      Image
        *overview_image;

      register int
        j;

      /*
        Read thumbnails from overview image.
      */
      for (j=1; j <= number_images; j++)
      {
        (void) sprintf(image->filename,"img%04d.pcd",j);
        image->scene=j;
        image->columns=width;
        image->rows=height;
        image->packets=image->columns*image->rows;
        image->pixels=(RunlengthPacket *)
          malloc(image->packets*sizeof(RunlengthPacket));
        if (image->pixels == (RunlengthPacket *) NULL)
          PrematureExit("Unable to allocate memory",image);
        y=luma;
        c1=chroma1;
        c2=chroma2;
        for (i=0; i < height; i+=2)
        {
          (void) ReadData((char *) y,1,width,image->file);
          y+=image->columns;
          (void) ReadData((char *) y,1,width,image->file);
          y+=image->columns;
          (void) ReadData((char *) c1,1,width >> 1,image->file);
          c1+=image->columns;
          (void) ReadData((char *) c2,1,width >> 1,image->file);
          c2+=image->columns;
        }
        Upsample(image->columns >> 1,image->rows >> 1,image->columns,chroma1);
        Upsample(image->columns >> 1,image->rows >> 1,image->columns,chroma2);
        /*
          Transfer luminance and chrominance channels.
        */
        p=image->pixels;
        y=luma;
        c1=chroma1;
        c2=chroma2;
        for (i=0; i < image->packets; i++)
        {
          p->red=UpScale(*y++);
          p->green=UpScale(*c1++);
          p->blue=UpScale(*c2++);
          p->index=0;
          p->length=0;
          p++;
        }
        TransformRGBImage(image,YCCColorspace);
        CompressImage(image);
        if (image_info->verbose)
          DescribeImage(image,stderr,False);
        if (j < number_images)
          {
            /*
              Allocate image structure.
            */
            image->next=AllocateImage(image_info);
            if (image->next == (Image *) NULL)
              {
                DestroyImages(image);
                return((Image *) NULL);
              }
            image->next->file=image->file;
            image->next->previous=image;
            image=image->next;
          }
        ProgressMonitor(LoadImageText,j-1,number_images);
      }
      free(chroma2);
      free(chroma1);
      free(luma);
      while (image->previous != (Image *) NULL)
        image=image->previous;
      CloseImage(image);
      overview_image=OverviewImage(image_info,image,number_images);
      return(overview_image);
    }
  /*
    Allocate image pixels.
  */
  image->pixels=(RunlengthPacket *)
    malloc(image->packets*sizeof(RunlengthPacket));
  if (image->pixels == (RunlengthPacket *) NULL)
    PrematureExit("Unable to allocate memory",image);
  /*
    Read interleaved image.
  */
  y=luma;
  c1=chroma1;
  c2=chroma2;
  for (i=0; i < height; i+=2)
  {
    (void) ReadData((char *) y,1,width,image->file);
    y+=image->columns;
    (void) ReadData((char *) y,1,width,image->file);
    y+=image->columns;
    (void) ReadData((char *) c1,1,width >> 1,image->file);
    c1+=image->columns;
    (void) ReadData((char *) c2,1,width >> 1,image->file);
    c2+=image->columns;
  }
  if (subimage >= 4)
    {
      /*
        Recover luminance deltas for 1536x1024 image.
      */
      Upsample(768,512,image->columns,luma);
      Upsample(384,256,image->columns,chroma1);
      Upsample(384,256,image->columns,chroma2);
      image->rows=1024;
      for (i=0; i < (4*0x800); i++)
        (void) fgetc(image->file);
      status=PCDDecodeImage(image,luma,chroma1,chroma2);
      if ((subimage >= 5) && status)
        {
          /*
            Recover luminance deltas for 3072x2048 image.
          */
          Upsample(1536,1024,image->columns,luma);
          Upsample(768,512,image->columns,chroma1);
          Upsample(768,512,image->columns,chroma2);
          image->rows=2048;
          offset=ftell(image->file)/0x800+12;
          (void) fseek(image->file,offset*0x800,0);
          status=PCDDecodeImage(image,luma,chroma1,chroma2);
          if ((subimage >= 6) && status)
            {
              /*
                Recover luminance deltas for 6144x4096 image (vaporware).
              */
              Upsample(3072,2048,image->columns,luma);
              Upsample(1536,1024,image->columns,chroma1);
              Upsample(1536,1024,image->columns,chroma2);
              image->rows=4096;
            }
        }
    }
  Upsample(image->columns >> 1,image->rows >> 1,image->columns,chroma1);
  Upsample(image->columns >> 1,image->rows >> 1,image->columns,chroma2);
  /*
    Transfer luminance and chrominance channels.
  */
  p=image->pixels;
  y=luma;
  c1=chroma1;
  c2=chroma2;
  for (i=0; i < image->packets; i++)
  {
    p->red=UpScale(*y++);
    p->green=UpScale(*c1++);
    p->blue=UpScale(*c2++);
    p->index=0;
    p->length=0;
    p++;
    if (QuantumTick(i,image))
      ProgressMonitor(LoadImageText,i,image->packets);
  }
  free(chroma2);
  free(chroma1);
  free(luma);
  TransformRGBImage(image,YCCColorspace);
  if ((rotate == 1) || (rotate == 3))
    {
      double
        degrees;

      Image
        *rotated_image;

      /*
        Rotate image.
      */
      degrees=rotate == 1 ? -90.0 : 90.0;
      image->orphan=True;
      rotated_image=RotateImage(image,degrees,(ColorPacket *) NULL,False,True);
      image->orphan=False;
      if (rotated_image != (Image *) NULL)
        {
          DestroyImage(image);
          image=rotated_image;
        }
    }
  CompressImage(image);
  CloseImage(image);
  return(image);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%  R e a d P C X I m a g e                                                    %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Function ReadPCXImage reads a ZSoft IBM PC Paintbrush file and returns it.
%  It allocates the memory necessary for the new Image structure and returns
%  a pointer to the new image.
%
%  The format of the ReadPCXImage routine is:
%
%      image=ReadPCXImage(image_info)
%
%  A description of each parameter follows:
%
%    o image:  Function ReadPCXImage returns a pointer to the image after
%      reading.  A null image is returned if there is a a memory shortage or
%      if the image cannot be read.
%
%    o image_info: Specifies a pointer to an ImageInfo structure.
%
%
*/
static Image *ReadPCXImage(ImageInfo *image_info)
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

  Image
    *image;

  int
    count,
    packets;

  register int
    i,
    x,
    y;

  register RunlengthPacket
    *q;

  register unsigned char
    *p;

  unsigned char
    packet,
    *pcx_colormap,
    *pcx_pixels;

  unsigned int
    status;

  /*
    Allocate image structure.
  */
  image=AllocateImage(image_info);
  if (image == (Image *) NULL)
    return((Image *) NULL);
  /*
    Open image file.
  */
  OpenImage(image,ReadBinaryType);
  if (image->file == (FILE *) NULL)
    PrematureExit("Unable to open file",image);
  /*
    Determine if this is a PCX file.
  */
  status=ReadData((char *) &pcx_header.identifier,1,1,image->file);
  do
  {
    /*
      Verify PCX identifier.
    */
    if ((status == False) || (pcx_header.identifier != 0x0a))
      PrematureExit("Not a PCX image file",image);
    (void) ReadData((char *) &pcx_header.version,1,1,image->file);
    (void) ReadData((char *) &pcx_header.encoding,1,1,image->file);
    (void) ReadData((char *) &pcx_header.bits_per_pixel,1,1,image->file);
    pcx_header.left=LSBFirstReadShort(image->file);
    pcx_header.top=LSBFirstReadShort(image->file);
    pcx_header.right=LSBFirstReadShort(image->file);
    pcx_header.bottom=LSBFirstReadShort(image->file);
    pcx_header.horizontal_resolution=LSBFirstReadShort(image->file);
    pcx_header.vertical_resolution=LSBFirstReadShort(image->file);
    /*
      Read PCX raster colormap.
    */
    image->columns=(pcx_header.right-pcx_header.left)+1;
    image->rows=(pcx_header.bottom-pcx_header.top)+1;
    image->class=PseudoClass;
    image->colors=16;
    image->colormap=(ColorPacket *) malloc(256*sizeof(ColorPacket));
    pcx_colormap=(unsigned char *) malloc(3*256*sizeof(unsigned char));
    if ((image->colormap == (ColorPacket *) NULL) ||
        (pcx_colormap == (unsigned char *) NULL))
      PrematureExit("Unable to allocate memory",image);
    (void) ReadData((char *) pcx_colormap,3,image->colors,image->file);
    p=pcx_colormap;
    for (i=0; i < image->colors; i++)
    {
      image->colormap[i].red=UpScale(*p++);
      image->colormap[i].green=UpScale(*p++);
      image->colormap[i].blue=UpScale(*p++);
    }
    (void) ReadData((char *) &pcx_header.reserved,1,1,image->file);
    (void) ReadData((char *) &pcx_header.planes,1,1,image->file);
    pcx_header.bytes_per_line=LSBFirstReadShort(image->file);
    pcx_header.palette_info=LSBFirstReadShort(image->file);
    for (i=0; i < 58; i++)
      (void) fgetc(image->file);
    /*
      Read image data.
    */
    packets=image->rows*pcx_header.bytes_per_line*pcx_header.planes;
    pcx_pixels=(unsigned char *) malloc(packets*sizeof(unsigned char));
    if (pcx_pixels == (unsigned char *) NULL)
      PrematureExit("Unable to allocate memory",image);
    /*
      Uncompress image data.
    */
    p=pcx_pixels;
    while (packets > 0)
    {
      packet=fgetc(image->file);
      if ((packet & 0xc0) != 0xc0)
        {
          *p++=packet;
          packets--;
          continue;
        }
      count=packet & 0x3f;
      for (packet=fgetc(image->file); count > 0; count--)
      {
        *p++=packet;
        packets--;
        if (packets == 0)
          break;
      }
    }
    image->colors=1 << (pcx_header.bits_per_pixel*pcx_header.planes);
    if (image->colors > 256)
      PrematureExit("PCX colormap exceeded 256 colors",image);
    if (image->colors > 16)
      {
        /*
          256 color images have their color map at the end of the file.
        */
        (void) ReadData((char *) &pcx_header.colormap_signature,1,1,
          image->file);
        (void) ReadData((char *) pcx_colormap,3,image->colors,image->file);
        p=pcx_colormap;
        for (i=0; i < image->colors; i++)
        {
          image->colormap[i].red=UpScale(*p++);
          image->colormap[i].green=UpScale(*p++);
          image->colormap[i].blue=UpScale(*p++);
        }
      }
    else
      if (image->colors == 2)
        if (Intensity(image->colormap[0]) == Intensity(image->colormap[1]))
          {
            /*
              Monochrome colormap.
            */
            image->colormap[0].red=MaxRGB;
            image->colormap[0].green=MaxRGB;
            image->colormap[0].blue=MaxRGB;
            image->colormap[1].red=0;
            image->colormap[1].green=0;
            image->colormap[1].blue=0;
          }
    free((char *) pcx_colormap);
    /*
      Convert PCX raster image to runlength-encoded packets.
    */
    if (pcx_header.planes > 1)
      {
        register int
          bits,
          mask;

        /*
          Initialize image structure.
        */
        image->packets=image->columns*image->rows;
        image->pixels=(RunlengthPacket *)
          malloc((image->packets+image->columns)*sizeof(RunlengthPacket));
        if (image->pixels == (RunlengthPacket *) NULL)
          PrematureExit("Unable to allocate memory",image);
        /*
          Convert multi-plane format into runlength-encoded pixels.
        */
        q=image->pixels;
        for (i=0; i < image->packets; i++)
        {
          q->index=0;
          q->length=0;
          q++;
        }
        for (y=0; y < image->rows; y++)
        {
          p=pcx_pixels+(y*pcx_header.bytes_per_line*pcx_header.planes);
          for (i=0; i < (int) pcx_header.planes; i++)
          {
            q=image->pixels+y*image->columns;
            for (x=0; x < pcx_header.bytes_per_line; x++)
            {
              bits=(*p++);
              for (mask=0x80; mask != 0; mask>>=1)
              {
                if (bits & mask)
                  q->index|=1 << i;
                q++;
              }
            }
          }
          ProgressMonitor(LoadImageText,y,image->rows);
        }
        SyncImage(image);
        CompressColormap(image);
        CompressImage(image);
      }
    else
      {
        register unsigned char
          *r;

        unsigned char
          *scanline;

        unsigned short
          index;

        /*
          Initialize image structure.
        */
        image->packets=0;
        packets=Max((image->columns*image->rows+4) >> 3,1);
        if (pcx_header.bits_per_pixel == 1)
          packets=Max((image->columns*image->rows+8) >> 4,1);
        image->pixels=(RunlengthPacket *)
          malloc(packets*sizeof(RunlengthPacket));
        scanline=(unsigned char *)
          malloc(image->columns*sizeof(unsigned char));
        if ((image->pixels == (RunlengthPacket *) NULL) ||
            (scanline == (unsigned char *) NULL))
          PrematureExit("Unable to allocate memory",image);
        q=image->pixels;
        q->length=MaxRunlength;
        for (y=0; y < image->rows; y++)
        {
          p=pcx_pixels+y*pcx_header.bytes_per_line;
          r=scanline;
          switch (pcx_header.bits_per_pixel)
          {
            case 1:
            {
              register int
                bit;

              for (x=0; x < (image->columns-7); x+=8)
              {
                for (bit=7; bit >= 0; bit--)
                  *r++=((*p) & (0x01 << bit) ? 0x01 : 0x00);
                p++;
              }
              if ((image->columns % 8) != 0)
                {
                  for (bit=7; bit >= (8-(image->columns % 8)); bit--)
                    *r++=((*p) & (0x01 << bit) ? 0x01 : 0x00);
                  p++;
                }
              break;
            }
            case 2:
            {
              for (x=0; x < (image->columns-3); x+=4)
              {
                *r++=(*p >> 6) & 0x3;
                *r++=(*p >> 4) & 0x3;
                *r++=(*p >> 2) & 0x3;
                *r++=(*p) & 0x3;
                p++;
              }
              if ((image->columns % 4) != 0)
                {
                  for (i=3; i >= (4-(image->columns % 4)); i--)
                    *r++=(*p >> (i*2)) & 0x03;
                  p++;
                }
              break;
            }
            case 4:
            {
              for (x=0; x < (image->columns-1); x+=2)
              {
                *r++=(*p >> 4) & 0xf;
                *r++=(*p) & 0xf;
                p++;
              }
              if ((image->columns % 2) != 0)
                *r++=(*p++ >> 4) & 0xf;
              break;
            }
            case 8:
            {
              for (x=0; x < image->columns; x++)
                *r++=(*p++);
              break;
            }
            default:
              break;
          }
          /*
            Transfer image scanline.
          */
          r=scanline;
          for (x=0; x < image->columns; x++)
          {
            index=(*r++);
            if ((index == q->index) && ((int) q->length < MaxRunlength))
              q->length++;
            else
              {
                if (image->packets != 0)
                  q++;
                image->packets++;
                if (image->packets == packets)
                  {
                    packets<<=1;
                    image->pixels=(RunlengthPacket *) realloc((char *)
                      image->pixels,packets*sizeof(RunlengthPacket));
                    if (image->pixels == (RunlengthPacket *) NULL)
                      {
                        free((char *) scanline);
                        PrematureExit("Unable to allocate memory",image);
                      }
                    q=image->pixels+image->packets-1;
                  }
                q->index=index;
                q->length=0;
              }
          }
          ProgressMonitor(LoadImageText,y,image->rows);
        }
        SyncImage(image);
        CompressColormap(image);
        image->pixels=(RunlengthPacket *) realloc((char *) image->pixels,
          image->packets*sizeof(RunlengthPacket));
        free((char *) scanline);
      }
    free((char *) pcx_pixels);
    /*
      Proceed to next image.
    */
    status=ReadData((char *) &pcx_header.identifier,1,1,image->file);
    if ((status == True) && (pcx_header.identifier == 0x0a))
      {
        /*
          Allocate image structure.
        */
        image->next=AllocateImage(image_info);
        if (image->next == (Image *) NULL)
          {
            DestroyImages(image);
            return((Image *) NULL);
          }
        (void) strcpy(image->next->filename,image_info->filename);
        image->next->file=image->file;
        image->next->scene=image->scene+1;
        image->next->previous=image;
        image=image->next;
      }
  } while ((status == True) && (pcx_header.identifier == 0x0a));
  while (image->previous != (Image *) NULL)
    image=image->previous;
  CloseImage(image);
  return(image);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%  R e a d P D F I m a g e                                                    %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Function ReadPDFImage reads a Portable Document Format image file and
%  returns it.  It allocates the memory necessary for the new Image structure
%  and returns a pointer to the new image.
%
%  The format of the ReadPDFImage routine is:
%
%      image=ReadPDFImage(image_info)
%
%  A description of each parameter follows:
%
%    o image:  Function ReadPDFImage returns a pointer to the image after
%      reading.  A null image is returned if there is a a memory shortage or
%      if the image cannot be read.
%
%    o image_info: Specifies a pointer to an ImageInfo structure.
%
%
*/
static Image *ReadPDFImage(ImageInfo *image_info)
{
#define MediaBox  "/MediaBox ["

  char
    command[MaxTextLength],
    *device,
    filename[MaxTextLength],
    options[MaxTextLength],
    postscript_filename[MaxTextLength];

  FILE
    *file;

  float
    lower_x,
    lower_y,
    upper_x,
    upper_y;

  Image
    *image,
    *next_image;

  int
    count,
    flags,
    status,
    x,
    y;

  long int
    filesize;

  register char
    *p;

  register int
    c,
    i;

  unsigned int
    dx_resolution,
    dy_resolution,
    height,
    width,
    x_resolution,
    y_resolution;

  /*
    Allocate image structure.
  */
  image=AllocateImage(image_info);
  if (image == (Image *) NULL)
    return((Image *) NULL);
  /*
    Open image file.
  */
  OpenImage(image,ReadBinaryType);
  if (image->file == (FILE *) NULL)
    PrematureExit("Unable to open file",image);
  /*
    Open temporary output file.
  */
  TemporaryFilename(postscript_filename);
  file=fopen(postscript_filename,"w");
  if (file == (FILE *) NULL)
    PrematureExit("Unable to write file",image);
  /*
    Set the page geometry.
  */
  *options='\0';
  if (image_info->page != (char *) NULL)
    {
      (void) strcat(options," -g");
      (void) strcat(options,image_info->page);
    }
  else
    for (p=command; ; )
    {
      c=fgetc(image->file);
      if (c == EOF)
        break;
      (void) fputc(c,file);
      *p++=c;
      if ((c != '\n') && (c != '\r') && ((p-command) < (MaxTextLength-1)))
        continue;
      *p='\0';
      p=command;
      /*
        Continue unless this is a MediaBox statement.
      */
      if (strncmp(MediaBox,command,strlen(MediaBox)) != 0)
        continue;
      count=sscanf(command,"/MediaBox [ %f %f %f %f",&lower_x,&lower_y,
        &upper_x,&upper_y);
      if (count != 4)
        continue;
      if ((lower_x > upper_x) || (lower_y > upper_y))
        continue;
      /*
        Determine bounding box.
      */
      dx_resolution=72;
      dy_resolution=72;
      (void) XParseGeometry(PSDensityGeometry,&x,&y,&x_resolution,
        &y_resolution);
      flags=NoValue;
      if (image_info->density != (char *) NULL)
        flags=XParseGeometry(image_info->density,&x,&y,&x_resolution,
          &y_resolution);
      if ((flags & HeightValue) == 0)
        y_resolution=x_resolution;
      if (image_info->page != (char *) NULL)
        continue;
      /*
        Set Postscript render geometry.
      */
      width=(unsigned int)(upper_x-lower_x);
      if ((float) ((int) upper_x) != upper_x)
        width++;
      height=(unsigned int)(upper_y-lower_y);
      if ((float) ((int) upper_y) != upper_y)
        height++;
      (void) sprintf(options,"-g%ux%u",
        (width*x_resolution+(dx_resolution >> 1))/dx_resolution,
        (height*y_resolution+(dy_resolution >> 1))/dy_resolution);
      break;
    }
  for ( ; ; )
  {
    c=fgetc(image->file);
    if (c == EOF)
      break;
    (void) fputc(c,file);
  }
  if (ferror(file))
    {
      Warning("An error has occurred writing to file",postscript_filename);
      (void) fclose(file);
      return((Image *) NULL);
    }
  (void) fclose(file);
  CloseImage(image);
  filesize=image->filesize;
  DestroyImage(image);
  /*
    Determine if the density options is specified.
  */
  (void) strcat(options," -r");
  if (image_info->density == (char *) NULL)
    (void) strcat(options,PSDensityGeometry);
  else
    (void) strcat(options,image_info->density);
  if (image_info->subimage != 0)
    {
      (void) sprintf(options,"%s -dFirstPage=%u",options,image_info->subimage);
      (void) sprintf(options,"%s -dLastPage=%u",options,image_info->subimage+
        image_info->subrange-1);
    }
  /*
    Use Ghostscript to convert Postscript image.
  */
  device=PostscriptColorDevice;
  if (image_info->monochrome)
    device=PostscriptMonoDevice;
  (void) strcpy(filename,image_info->filename);
  for (i=0; i < 50; i++)
  {
    /*
      Ghostscript eats % characters.
    */
    TemporaryFilename(image_info->filename);
    if (strchr(image_info->filename,'%') == (char *) NULL)
      break;
  }
  (void) sprintf(command,PostscriptCommand,device,options,image_info->filename,
    postscript_filename);
  status=SystemCommand(command);
  if (status)
    {
      /*
        Pre GS 3.51 does not support the pnmraw device.
      */
      (void) sprintf(command,PostscriptCommand,"ppmraw",options,
        image_info->filename,postscript_filename);
      status=SystemCommand(command);
    }
  if (status)
    {
      Warning("Portable Document translation failed",image_info->filename);
      (void) unlink(postscript_filename);
      return((Image *) NULL);
    }
  image=ReadPNMImage(image_info);
  (void) unlink(postscript_filename);
  (void) unlink(image_info->filename);
  if (image == (Image *) NULL)
    {
      Warning("Portable Document translation failed",image_info->filename);
      return((Image *) NULL);
    }
  do
  {
    (void) strcpy(image->filename,filename);
    image->filesize=filesize;
    next_image=image->next;
    if (next_image != (Image *) NULL)
      image=next_image;
  } while (next_image != (Image *) NULL);
  while (image->previous != (Image *) NULL)
    image=image->previous;
  return(image);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%  R e a d P I C T I m a g e                                                  %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Function ReadPICTImage reads an Apple Macintosh QuickDraw/PICT image file
%  and returns it.  It allocates the memory necessary for the new Image
%  structure and returns a pointer to the new image.
%
%  The format of the ReadPICTImage routine is:
%
%      image=ReadPICTImage(image_info)
%
%  A description of each parameter follows:
%
%    o image:  Function ReadPICTImage returns a pointer to the image after
%      reading.  A null image is returned if there is a a memory shortage or
%      if the image cannot be read.
%
%    o image_info: Specifies a pointer to an ImageInfo structure.
%
%
*/
static Image *ReadPICTImage(ImageInfo *image_info)
{
  char
    filename[MaxTextLength];

  Image
    *image,
    *next_image,
    *proxy_image;

  /*
    Allocate image structure.
  */
  proxy_image=AllocateImage(image_info);
  if (proxy_image == (Image *) NULL)
    return((Image *) NULL);
  /*
    Open image file.
  */
  OpenImage(proxy_image,ReadBinaryType);
  if (proxy_image->file == (FILE *) NULL)
    PrematureExit("Unable to open file",proxy_image);
  CloseImage(proxy_image);
  DestroyImage(proxy_image);
  /*
    Use picttoppm to convert Macintosh PICT image.
  */
  (void) strcpy(filename,image_info->filename);
  (void) sprintf(image_info->filename,"|imconv %s -outformat pnm -",filename);
  image=ReadPNMImage(image_info);
  if (image == (Image *) NULL)
    {
      (void) sprintf(image_info->filename,"|picttoppm %s",filename);
      image=ReadPNMImage(image_info);
    }
  if (image == (Image *) NULL)
    {
      Warning("PICT translation failed",image_info->filename);
      return((Image *) NULL);
    }
  /*
    Assign proper filename.
  */
  do
  {
    (void) strcpy(image->filename,filename);
    next_image=image->next;
    if (next_image != (Image *) NULL)
      image=next_image;
  } while (next_image != (Image *) NULL);
  while (image->previous != (Image *) NULL)
    image=image->previous;
  return(image);
}

#ifdef HasPNG
#include "png.h"
/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%  R e a d P N G I m a g e                                                    %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Function ReadPNGImage reads a Portable Network Graphics image file and
%  returns it.  It allocates the memory necessary for the new Image structure
%  and returns a pointer to the new image.
%
%  The format of the ReadPNGImage routine is:
%
%      image=ReadPNGImage(image_info)
%
%  A description of each parameter follows:
%
%    o image:  Function ReadPNGImage returns a pointer to the image after
%      reading.  A null image is returned if there is a a memory shortage or
%      if the image cannot be read.
%
%    o image_info: Specifies a pointer to an ImageInfo structure.
%
%
*/
static Image *ReadPNGImage(image_info)
ImageInfo
  *image_info;
{
  Image
    *image;

  register int
    i,
    x,
    y;

  register unsigned char
    *p;

  register RunlengthPacket
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
    packets;

  unsigned short
    index,
    value;

  /*
    Allocate image structure.
  */
  image=AllocateImage(image_info);
  if (image == (Image *) NULL)
    return((Image *) NULL);
  /*
    Open image file.
  */
  OpenImage(image,ReadBinaryType);
  if (image->file == (FILE *) NULL)
    PrematureExit("Unable to open file",image);
  /*
    Allocate the PNG structures
  */
  ping=(png_struct *) malloc(sizeof(png_struct));
  ping_info=(png_info *) malloc(sizeof(png_info));
  if ((ping == (png_struct *) NULL) || (ping_info == (png_info *) NULL))
    PrematureExit("Unable to allocate memory",image);
  image->pixels=(RunlengthPacket *) NULL;
  png_pixels=(unsigned char *) NULL;
  scanlines=(unsigned char **) NULL;
  if (setjmp(ping->jmpbuf))
    {
      /*
        PNG image is corrupt.
      */
      png_read_destroy(ping,ping_info,(png_info *) NULL);
      if (scanlines != (unsigned char **) NULL)
        free((char *) scanlines);
      if (png_pixels != (unsigned char *) NULL)
        free((char *) png_pixels);
      free((char *) ping);
      free((char *) ping_info);
      CloseImage(image);
      if ((image->columns == 0) || (image->rows == 0))
        {
          DestroyImage(image);
          return((Image *) NULL);
        }
      return(image);
    }
  /*
    Prepare PNG for reading.
  */
  png_info_init(ping_info);
  png_read_init(ping);
  png_init_io(ping,image->file);
  png_read_info(ping,ping_info);
  if (ping_info->bit_depth > QuantumDepth)
    {
      png_set_strip_16(ping);
      ping_info->bit_depth=8;
    }
  if (ping_info->bit_depth < 16)
    image->depth=8;
  /*
    Initialize image structure.
  */
  image->columns=ping_info->width;
  image->rows=ping_info->height;
  image->packets=0;
  packets=Max((image->columns*image->rows+4) >> 3,1);
  if (ping_info->bit_depth == 1)
    packets=Max((image->columns*image->rows+8) >> 4,1);
  image->pixels=(RunlengthPacket *) malloc(packets*sizeof(RunlengthPacket));
  bytes_per_line=
    Max((int) ping_info->bit_depth >> 3,1)*ping_info->channels*image->columns;
  png_pixels=(unsigned char *)
    malloc(bytes_per_line*image->rows*sizeof(Quantum));
  scanlines=(unsigned char **) malloc(image->rows*sizeof(unsigned char *));
  if ((image->pixels == (RunlengthPacket *) NULL) ||
      (png_pixels == (unsigned char *) NULL) ||
      (scanlines == (unsigned char **) NULL))
    PrematureExit("Unable to allocate memory",image);
  if ((ping_info->color_type == PNG_COLOR_TYPE_PALETTE) ||
      (ping_info->color_type == PNG_COLOR_TYPE_GRAY))
    {
      /*
        Initialize image colormap.
      */
      image->class=PseudoClass;
      image->colors=1 << ping_info->bit_depth;
      if (ping_info->color_type == PNG_COLOR_TYPE_PALETTE)
        image->colors=ping_info->num_palette;
      image->colormap=(ColorPacket *) malloc(image->colors*sizeof(ColorPacket));
      if (image->colormap == (ColorPacket *) NULL)
        PrematureExit("Unable to allocate memory",image);
      if (ping_info->color_type == PNG_COLOR_TYPE_PALETTE)
        for (i=0; i < image->colors; i++)
        {
          image->colormap[i].red=UpScale(ping_info->palette[i].red);
          image->colormap[i].green=UpScale(ping_info->palette[i].green);
          image->colormap[i].blue=UpScale(ping_info->palette[i].blue);
        }
      else
        for (i=0; i < image->colors; i++)
        {
          image->colormap[i].red=(MaxRGB*i)/(image->colors-1);
          image->colormap[i].green=(MaxRGB*i)/(image->colors-1);
          image->colormap[i].blue=(MaxRGB*i)/(image->colors-1);
        }
    }
  /*
    Read image scanlines.
  */
  if (image->class == DirectClass)
    if (ping_info->bit_depth < 8)
      png_set_packing(ping);
  for (i=0; i < image->rows; i++)
    scanlines[i]=png_pixels+(bytes_per_line*i);
  png_read_image(ping,scanlines);
  png_read_end(ping,ping_info);
  /*
    Convert PNG pixels to runlength-encoded packets.
  */
  q=image->pixels;
  q->length=MaxRunlength;
  if (image->class == DirectClass)
    {
      Quantum
        blue,
        green,
        red;

      /*
        Convert image to DirectClass runlength-encoded packets.
      */
      if ((ping_info->color_type == PNG_COLOR_TYPE_RGB_ALPHA) ||
          (ping_info->color_type == PNG_COLOR_TYPE_GRAY_ALPHA))
        image->matte=True;
      for (y=0; y < image->rows; y++)
      {
        p=scanlines[y];
        for (x=0; x < image->columns; x++)
        {
          ReadQuantum(red,p);
          green=red;
          blue=red;
          if (ping_info->color_type != PNG_COLOR_TYPE_GRAY_ALPHA)
            {
              ReadQuantum(green,p);
              ReadQuantum(blue,p);
            }
          index=0;
          if (image->matte)
            ReadQuantum(index,p);
          if ((red == q->red) && (green == q->green) && (blue == q->blue) &&
              (index == q->index) && ((int) q->length < MaxRunlength))
            q->length++;
          else
            {
              if (image->packets != 0)
                q++;
              image->packets++;
              if (image->packets == packets)
                {
                  packets<<=1;
                  image->pixels=(RunlengthPacket *) realloc((char *)
                    image->pixels,packets*sizeof(RunlengthPacket));
                  if (image->pixels == (RunlengthPacket *) NULL)
                    {
                      free((char *) png_pixels);
                      PrematureExit("Unable to allocate memory",image);
                    }
                  q=image->pixels+image->packets-1;
                }
              q->red=red;
              q->green=green;
              q->blue=blue;
              q->index=index;
              q->length=0;
            }
        }
        ProgressMonitor(LoadImageText,y,image->rows);
      }
    }
  else
    {
      Quantum
        *quantum_scanline;

      register Quantum
        *r;

      /*
        Convert image to PseudoClass runlength-encoded packets.
      */
      quantum_scanline=(Quantum *) malloc(image->columns*sizeof(Quantum));
      if (quantum_scanline == (Quantum *) NULL)
        PrematureExit("Unable to allocate memory",image);
      for (y=0; y < image->rows; y++)
      {
        p=scanlines[y];
        r=quantum_scanline;
        switch (ping_info->bit_depth)
        {
          case 1:
          {
            register int
              bit;

            for (x=0; x < ((int) image->columns-7); x+=8)
            {
              for (bit=7; bit >= 0; bit--)
                *r++=((*p) & (0x01 << bit) ? 0x01 : 0x00);
              p++;
            }
            if ((image->columns % 8) != 0)
              {
                for (bit=7; bit >= (8-(image->columns % 8)); bit--)
                  *r++=((*p) & (0x01 << bit) ? 0x01 : 0x00);
                p++;
              }
            break;
          }
          case 2:
          {
            for (x=0; x < ((int) image->columns-3); x+=4)
            {
              *r++=(*p >> 6) & 0x3;
              *r++=(*p >> 4) & 0x3;
              *r++=(*p >> 2) & 0x3;
              *r++=(*p) & 0x3;
              p++;
            }
            if ((image->columns % 4) != 0)
              {
                for (i=3; i >= (4-(image->columns % 4)); i--)
                  *r++=(*p >> (i*2)) & 0x03;
                p++;
              }
            break;
          }
          case 4:
          {
            for (x=0; x < ((int) image->columns-1); x+=2)
            {
              *r++=(*p >> 4) & 0xf;
              *r++=(*p) & 0xf;
              p++;
            }
            if ((image->columns % 2) != 0)
              *r++=(*p++ >> 4) & 0xf;
            break;
          }
          case 8:
          {
            for (x=0; x < image->columns; x++)
              *r++=(*p++);
            break;
          }
          case 16:
          {
            for (x=0; x < image->columns; x++)
            {
              ReadQuantum(*r,p);
              r++;
            }
            break;
          }
          default:
            break;
        }
        /*
          Transfer image scanline.
        */
        r=quantum_scanline;
        for (x=0; x < image->columns; x++)
        {
          index=(*r++);
          if ((index == q->index) && ((int) q->length < MaxRunlength))
            q->length++;
          else
            {
              if (image->packets != 0)
                q++;
              image->packets++;
              if (image->packets == packets)
                {
                  packets<<=1;
                  image->pixels=(RunlengthPacket *) realloc((char *)
                    image->pixels,packets*sizeof(RunlengthPacket));
                  if (image->pixels == (RunlengthPacket *) NULL)
                    {
                      free((char *) quantum_scanline);
                      PrematureExit("Unable to allocate memory",image);
                    }
                  q=image->pixels+image->packets-1;
                }
              q->index=index;
              q->length=0;
            }
        }
        ProgressMonitor(LoadImageText,y,image->rows);
      }
      if (image->class == PseudoClass)
        SyncImage(image);
      free((char *) quantum_scanline);
      if (ping_info->valid & PNG_INFO_tRNS)
        {
          /*
            Image has a transparent background.
          */
          image->class=DirectClass;
          image->matte=True;
          q=image->pixels;
          for (i=0; i < image->packets; i++)
          {
            index=q->index;
            q->index=Opaque;
            if (ping_info->color_type != PNG_COLOR_TYPE_PALETTE)
              {
                if (index == ping_info->trans_values.gray)
                  q->index=Transparent;
              }
            else
              if (index < ping_info->num_trans)
                q->index=ping_info->trans[index];
            q++;
          }
        }
      if (image->class == PseudoClass)
        CompressColormap(image);
    }
  image->pixels=(RunlengthPacket *)
    realloc((char *) image->pixels,image->packets*sizeof(RunlengthPacket));
  if (ping_info->valid & PNG_INFO_gAMA)
    image->gamma=ping_info->gamma;
  if (ping_info->num_text > 0)
    {
      unsigned int
        length;

      /*
        Initialize image comment.
      */
      length=1;
      for (i=0; i < ping_info->num_text; i++)
        length+=strlen(ping_info->text[i].key)+ping_info->text[i].text_length+4;
      image->comments=malloc(length);
      if (image->comments == (char *) NULL)
        PrematureExit("Unable to allocate memory",image);
      *image->comments='\0';
      for (i=0; i < ping_info->num_text; i++)
      {
        (void) strcat(image->comments,ping_info->text[i].key);
        (void) strcat(image->comments,": ");
        p=(unsigned char *) image->comments+strlen(image->comments);
        (void) strncat(image->comments,ping_info->text[i].text,
          ping_info->text[i].text_length);
        p+=ping_info->text[i].text_length;
        if (i < (ping_info->num_text-1))
          *p++='\n';
        *p='\0';
      }
    }
  /*
    Free memory.
  */
  png_read_destroy(ping,ping_info,(png_info *) NULL);
  free((char *) png_pixels);
  free((char *) ping);
  free((char *) ping_info);
  CloseImage(image);
  return(image);
}
#else
static Image *ReadPNGImage(ImageInfo *image_info)
{
  Warning("PNG library is not available",image_info->filename);
  return(ReadMIFFImage(image_info));
}
#endif

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%  R e a d P N M I m a g e                                                    %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Function ReadPNMImage reads a Portable Anymap image file and returns it.
%  It allocates the memory necessary for the new Image structure and returns
%  a pointer to the new image.
%
%  The format of the ReadPNMImage routine is:
%
%      image=ReadPNMImage(image_info)
%
%  A description of each parameter follows:
%
%    o image:  Function ReadPNMImage returns a pointer to the image after
%      reading.  A null image is returned if there is a a memory shortage or
%      if the image cannot be read.
%
%    o image_info: Specifies a pointer to an ImageInfo structure.
%
%
*/

static unsigned int PNMInteger(Image *image, unsigned int base)
{
#define MaxRawValue  255

  int
    c;

  unsigned int
    value;

  /*
    Skip any leading whitespace.
  */
  do
  {
    c=fgetc(image->file);
    if (c == EOF)
      return(0);
    if (c == '#')
      {
        register char
          *p;

        unsigned int
          length;

        /*
          Read comment.
        */
        if (image->comments != (char *) NULL)
          {
            length=strlen(image->comments);
            p=image->comments+length;
          }
        else
          {
            length=MaxTextLength;
            image->comments=(char *) malloc(length*sizeof(char));
            p=image->comments;
          }
        for ( ; image->comments != (char *) NULL; p++)
        {
          if ((p-image->comments+2) >= length)
            {
              *p='\0';
              length<<=1;
              image->comments=(char *)
                realloc((char *) image->comments,length*sizeof(char));
              if (image->comments == (char *) NULL)
                break;
              p=image->comments+strlen(image->comments);
            }
          c=fgetc(image->file);
          if ((c == EOF) || (c == '\n'))
            break;
          *p=(unsigned char) c;
        }
        if (image->comments == (char *) NULL)
          {
            Warning("Memory allocation error",(char *) NULL);
            return(0);
          }
        *p++='\n';
        *p='\0';
      }
  } while (!isdigit(c));
  if (base == 2)
    return(c-'0');
  /*
    Evaluate number.
  */
  value=0;
  do
  {
    value*=10;
    value+=c-'0';
    c=fgetc(image->file);
    if (c == EOF)
      return(0);
  }
  while (isdigit(c));
  return(value);
}

static Image *ReadPNMImage(ImageInfo *image_info)
{
  char
    format;

  Image
    *image;

  int
    y;

  Quantum
    blue,
    green,
    red,
    *scale;

  register int
    i,
    x;

  register RunlengthPacket
    *q;

  unsigned int
    max_value,
    packets,
    status;

  unsigned short
    index;

  /*
    Allocate image structure.
  */
  image=AllocateImage(image_info);
  if (image == (Image *) NULL)
    return((Image *) NULL);
  /*
    Open image file.
  */
  OpenImage(image,"r");
  if (image->file == (FILE *) NULL)
    PrematureExit("Unable to open file",image);
  /*
    Read PNM image.
  */
  status=ReadData((char *) &format,1,1,image->file);
  do
  {
    /*
      Verify PNM identifier.
    */
    if ((status == False) || (format != 'P'))
      PrematureExit("Not a PNM image file",image);
    /*
      Initialize image structure.
    */
    format=fgetc(image->file);
    if (format == '7')
      (void) PNMInteger(image,10);
    image->columns=PNMInteger(image,10);
    image->rows=PNMInteger(image,10);
    if ((image->columns*image->rows) == 0)
      PrematureExit("Unable to read image: image dimensions are zero",image);
    image->packets=0;
    packets=Max((image->columns*image->rows+4) >> 3,1);
    if ((format == '1') || (format == '4'))
      packets=Max((image->columns*image->rows+8) >> 4,1);
    image->pixels=(RunlengthPacket *) malloc(packets*sizeof(RunlengthPacket));
    if (image->pixels == (RunlengthPacket *) NULL)
      PrematureExit("Unable to allocate memory",image);
    if ((format == '1') || (format == '4'))
      max_value=1;  /* bitmap */
    else
      max_value=PNMInteger(image,10);
    scale=(Quantum *) NULL;
    if ((format != '3') && (format != '6'))
      {
        /*
          Create colormap.
        */
        image->class=PseudoClass;
        image->colors=Min(max_value,MaxRGB)+1;
        image->colormap=(ColorPacket *)
          malloc(image->colors*sizeof(ColorPacket));
        if (image->colormap == (ColorPacket *) NULL)
          PrematureExit("Unable to allocate memory",image);
        if (format != '7')
          for (i=0; i < image->colors; i++)
          {
            image->colormap[i].red=(MaxRGB*i)/(image->colors-1);
            image->colormap[i].green=(MaxRGB*i)/(image->colors-1);
            image->colormap[i].blue=(MaxRGB*i)/(image->colors-1);
          }
        else
          {
            /*
              Initialize 332 colormap.
            */
            i=0;
            for (red=0; red < 8; red++)
              for (green=0; green < 8; green++)
                for (blue=0; blue < 4; blue++)
                {
                  image->colormap[i].red=(Quantum) (red*MaxRGB)/7;
                  image->colormap[i].green=(Quantum) (green*MaxRGB)/7;
                  image->colormap[i].blue=(Quantum) (blue*MaxRGB)/3;
                  i++;
                }
          }
      }
    if (max_value != MaxRGB)
      {
        /*
          Compute pixel scaling table.
        */
        scale=(Quantum *) malloc((max_value+1)*sizeof(Quantum));
        if (scale == (Quantum *) NULL)
          PrematureExit("Unable to allocate memory",image);
        for (i=0; i <= max_value; i++)
          scale[i]=(Quantum) ((i*MaxRGB+(max_value >> 1))/max_value);
      }
    /*
      Convert PNM pixels to runlength-encoded MIFF packets.
    */
    q=image->pixels;
    q->length=MaxRunlength;
    switch (format)
    {
      case '1':
      {
        /*
          Convert PBM image to runlength-encoded packets.
        */
        for (y=0; y < image->rows; y++)
        {
          for (x=0; x < image->columns; x++)
          {
            index=!PNMInteger(image,2);
            if ((index == q->index) && ((int) q->length < MaxRunlength))
              q->length++;
            else
              {
                if (image->packets != 0)
                  q++;
                image->packets++;
                if (image->packets == packets)
                  {
                    packets<<=1;
                    image->pixels=(RunlengthPacket *) realloc((char *)
                      image->pixels,packets*sizeof(RunlengthPacket));
                    if (image->pixels == (RunlengthPacket *) NULL)
                      PrematureExit("Unable to allocate memory",image);
                    q=image->pixels+image->packets-1;
                  }
                q->index=index;
                q->length=0;
              }
          }
          ProgressMonitor(LoadImageText,y,image->rows);
        }
        break;
      }
      case '2':
      {
        /*
          Convert PGM image to runlength-encoded packets.
        */
        for (y=0; y < image->rows; y++)
        {
          for (x=0; x < image->columns; x++)
          {
            index=PNMInteger(image,10);
            if (scale != (Quantum *) NULL)
              index=scale[index];
            if ((index == q->index) && ((int) q->length < MaxRunlength))
              q->length++;
            else
              {
                if (image->packets != 0)
                  q++;
                image->packets++;
                if (image->packets == packets)
                  {
                    packets<<=1;
                    image->pixels=(RunlengthPacket *) realloc((char *)
                      image->pixels,packets*sizeof(RunlengthPacket));
                    if (image->pixels == (RunlengthPacket *) NULL)
                      PrematureExit("Unable to allocate memory",image);
                    q=image->pixels+image->packets-1;
                  }
                q->index=index;
                q->length=0;
              }
          }
          ProgressMonitor(LoadImageText,y,image->rows);
        }
        break;
      }
      case '3':
      {
        /*
          Convert PNM image to runlength-encoded packets.
        */
        for (y=0; y < image->rows; y++)
        {
          for (x=0; x < image->columns; x++)
          {
            red=PNMInteger(image,10);
            green=PNMInteger(image,10);
            blue=PNMInteger(image,10);
            if (scale != (Quantum *) NULL)
              {
                red=scale[red];
                green=scale[green];
                blue=scale[blue];
              }
            if ((red == q->red) && (green == q->green) && (blue == q->blue) &&
                ((int) q->length < MaxRunlength))
              q->length++;
            else
              {
                if (image->packets != 0)
                  q++;
                image->packets++;
                if (image->packets == packets)
                  {
                    packets<<=1;
                    image->pixels=(RunlengthPacket *) realloc((char *)
                      image->pixels,packets*sizeof(RunlengthPacket));
                    if (image->pixels == (RunlengthPacket *) NULL)
                      PrematureExit("Unable to allocate memory",image);
                    q=image->pixels+image->packets-1;
                  }
                q->red=red;
                q->green=green;
                q->blue=blue;
                q->index=0;
                q->length=0;
            }
          }
          ProgressMonitor(LoadImageText,y,image->rows);
        }
        if (IsPseudoClass(image))
          QuantizeImage(image,MaxRGB+1,8,False,RGBColorspace);
        break;
      }
      case '4':
      {
        unsigned char
          bit,
          Byte;

        unsigned int
          x,
          y;

        /*
          Convert PBM raw image to runlength-encoded packets.
        */
        for (y=0; y < image->rows; y++)
        {
          bit=0;
          Byte=0;
          for (x=0; x < image->columns; x++)
          {
            if (bit == 0)
              Byte=fgetc(image->file);
            index=(Byte & 0x80) ? 0 : 1;
            if ((index == q->index) && ((int) q->length < MaxRunlength))
              q->length++;
            else
              {
                if (image->packets != 0)
                  q++;
                image->packets++;
                if (image->packets == packets)
                  {
                    packets<<=1;
                    image->pixels=(RunlengthPacket *) realloc((char *)
                      image->pixels,packets*sizeof(RunlengthPacket));
                    if (image->pixels == (RunlengthPacket *) NULL)
                      PrematureExit("Unable to allocate memory",image);
                    q=image->pixels+image->packets-1;
                  }
                q->index=index;
                q->length=0;
              }
            bit++;
            if (bit == 8)
              bit=0;
            Byte<<=1;
          }
          ProgressMonitor(LoadImageText,y,image->rows);
        }
        break;
      }
      case '5':
      case '7':
      {
        /*
          Convert PGM raw image to runlength-encoded packets.
        */
        for (y=0; y < image->rows; y++)
        {
          for (x=0; x < image->columns; x++)
          {
            if (max_value <= MaxRawValue)
              index=fgetc(image->file);
            else
              index=LSBFirstReadShort(image->file);
            if ((index == q->index) && ((int) q->length < MaxRunlength))
              q->length++;
            else
              {
                if (image->packets != 0)
                  q++;
                image->packets++;
                if (image->packets == packets)
                  {
                    packets<<=1;
                    image->pixels=(RunlengthPacket *) realloc((char *)
                      image->pixels,packets*sizeof(RunlengthPacket));
                    if (image->pixels == (RunlengthPacket *) NULL)
                      PrematureExit("Unable to allocate memory",image);
                    q=image->pixels+image->packets-1;
                  }
                q->index=index;
                q->length=0;
              }
          }
          ProgressMonitor(LoadImageText,y,image->rows);
        }
        break;
      }
      case '6':
      {
        /*
          Convert PNM raster image to runlength-encoded packets.
        */
        for (y=0; y < image->rows; y++)
        {
          for (x=0; x < image->columns; x++)
          {
            if (max_value <= MaxRawValue)
              {
                red=fgetc(image->file);
                green=fgetc(image->file);
                blue=fgetc(image->file);
              }
            else
              {
                red=LSBFirstReadShort(image->file);
                green=LSBFirstReadShort(image->file);
                blue=LSBFirstReadShort(image->file);
              }
            if (scale != (Quantum *) NULL)
              {
                red=scale[red];
                green=scale[green];
                blue=scale[blue];
              }
            if ((red == q->red) && (green == q->green) && (blue == q->blue) &&
                ((int) q->length < MaxRunlength))
              q->length++;
            else
              {
                if (image->packets != 0)
                  q++;
                image->packets++;
                if (image->packets == packets)
                  {
                    packets<<=1;
                    image->pixels=(RunlengthPacket *) realloc((char *)
                      image->pixels,packets*sizeof(RunlengthPacket));
                    if (image->pixels == (RunlengthPacket *) NULL)
                      PrematureExit("Unable to allocate memory",image);
                      q=image->pixels+image->packets-1;
                  }
                q->red=red;
                q->green=green;
                q->blue=blue;
                q->index=0;
                q->length=0;
              }
          }
          ProgressMonitor(LoadImageText,y,image->rows);
        }
        if (IsPseudoClass(image))
          QuantizeImage(image,MaxRGB+1,8,False,RGBColorspace);
        break;
      }
      default:
        PrematureExit("Not a PNM image file",image);
    }
    if (scale != (Quantum *) NULL)
      free((char *) scale);
    if (image->class == PseudoClass)
      {
        SyncImage(image);
        CompressColormap(image);
      }
    image->pixels=(RunlengthPacket *)
      realloc((char *) image->pixels,image->packets*sizeof(RunlengthPacket));
    /*
      Proceed to next image.
    */
    if ((format == '1') || (format == '2') || (format == '3'))
      do
      {
        /*
          Skip to end of line.
        */
        status=ReadData(&format,1,1,image->file);
        if (status == False)
          break;
      } while (format != '\n');
    status=ReadData((char *) &format,1,1,image->file);
    if ((status == True) && (format == 'P'))
      {
        /*
          Allocate image structure.
        */
        image->next=AllocateImage(image_info);
        if (image->next == (Image *) NULL)
          {
            DestroyImages(image);
            return((Image *) NULL);
          }
        (void) strcpy(image->next->filename,image_info->filename);
        image->next->file=image->file;
        image->next->scene=image->scene+1;
        image->next->previous=image;
        image=image->next;
      }
  } while ((status == True) && (format == 'P'));
  while (image->previous != (Image *) NULL)
    image=image->previous;
  CloseImage(image);
  return(image);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%  R e a d P S I m a g e                                                      %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Function ReadPSImage reads a Adobe Postscript image file and returns it.  It
%  allocates the memory necessary for the new Image structure and returns a
%  pointer to the new image.
%
%  The format of the ReadPSImage routine is:
%
%      image=ReadPSImage(image_info)
%
%  A description of each parameter follows:
%
%    o image:  Function ReadPSImage returns a pointer to the image after
%      reading.  A null image is returned if there is a a memory shortage or
%      if the image cannot be read.
%
%    o image_info: Specifies a pointer to an ImageInfo structure.
%
%
*/
static Image *ReadPSImage(ImageInfo *image_info)
{
#define BoundingBox  "%%BoundingBox:"
#define DocumentMedia  "%%DocumentMedia:"
#define PageBoundingBox  "%%PageBoundingBox:"

  char
    command[MaxTextLength],
    *device,
    filename[MaxTextLength],
    options[MaxTextLength],
    postscript_filename[MaxTextLength],
    translate_geometry[MaxTextLength];

  FILE
    *file;

  float
    lower_x,
    lower_y,
    upper_x,
    upper_y;

  Image
    *image,
    *next_image;

  int
    c,
    count,
    flags,
    status,
    x,
    y;

  long int
    filesize;

  register char
    *p;

  register int
    i;

  unsigned int
    dx_resolution,
    dy_resolution,
    height,
    width,
    x_resolution,
    y_resolution;

  /*
    Allocate image structure.
  */
  image=AllocateImage(image_info);
  if (image == (Image *) NULL)
    return((Image *) NULL);
  /*
    Open image file.
  */
  OpenImage(image,ReadBinaryType);
  if (image->file == (FILE *) NULL)
    PrematureExit("Unable to open file",image);
  /*
    Open temporary output file.
  */
  TemporaryFilename(postscript_filename);
  file=fopen(postscript_filename,"w");
  if (file == (FILE *) NULL)
    PrematureExit("Unable to write file",image);
  (void) sprintf(translate_geometry,"%f %f translate\n              ",0.0,0.0);
  (void) fputs(translate_geometry,file);
  /*
    Set the page geometry.
  */
  lower_x=0;
  lower_y=0;
  *options='\0';
  if (image_info->page != (char *) NULL)
    {
      (void) strcat(options," -g");
      (void) strcat(options,image_info->page);
    }
  else
    for (p=command; ; )
    {
      c=fgetc(image->file);
      if (c == EOF)
        break;
      (void) fputc(c,file);
      *p++=c;
      if ((c != '\n') && (c != '\r') && ((p-command) < (MaxTextLength-1)))
        continue;
      *p='\0';
      p=command;
      /*
        Parse a bounding box statement.
      */
      count=0;
      if (strncmp(BoundingBox,command,strlen(BoundingBox)) == 0)
        count=sscanf(command,"%%%%BoundingBox: %f %f %f %f",&lower_x,&lower_y,
          &upper_x,&upper_y);
      if (strncmp(DocumentMedia,command,strlen(DocumentMedia)) == 0)
        count=
          sscanf(command,"%%%%DocumentMedia: %*s %f %f",&upper_x,&upper_y)+2;
      if (strncmp(PageBoundingBox,command,strlen(PageBoundingBox)) == 0)
        count=sscanf(command,"%%%%PageBoundingBox: %f %f %f %f",
          &lower_x,&lower_y,&upper_x,&upper_y);
      if (count != 4)
        continue;
      if ((lower_x > upper_x) || (lower_y > upper_y))
        continue;
      /*
        Determine bounding box.
      */
      dx_resolution=72;
      dy_resolution=72;
      (void) XParseGeometry(PSDensityGeometry,&x,&y,&x_resolution,
        &y_resolution);
      flags=NoValue;
      if (image_info->density != (char *) NULL)
        flags=XParseGeometry(image_info->density,&x,&y,&x_resolution,
          &y_resolution);
      if ((flags & HeightValue) == 0)
        y_resolution=x_resolution;
      /*
        Set Postscript render geometry.
      */
      (void) sprintf(translate_geometry,"%f %f translate\n",-lower_x,-lower_y);
      width=(unsigned int)(upper_x-lower_x);
      if ((float) ((int) upper_x) != upper_x)
        width++;
      height=(unsigned int)(upper_y-lower_y);
      if ((float) ((int) upper_y) != upper_y)
        height++;
      (void) sprintf(options,"-g%ux%u",
        (width*x_resolution+(dx_resolution >> 1))/dx_resolution,
        (height*y_resolution+(dy_resolution >> 1))/dy_resolution);
      break;
    }
  for ( ; ; )
  {
    c=fgetc(image->file);
    if (c == EOF)
      break;
    (void) fputc(c,file);
  }
  if (ferror(file))
    {
      Warning("An error has occurred writing to file",postscript_filename);
      (void) fclose(file);
      return((Image *) NULL);
    }
  (void) fseek(file,0,0);
  (void) fputs(translate_geometry,file);
  (void) fclose(file);
  CloseImage(image);
  filesize=image->filesize;
  DestroyImage(image);
  /*
    Determine if density options is specified.
  */
  (void) strcat(options," -r");
  if (image_info->density == (char *) NULL)
    (void) strcat(options,PSDensityGeometry);
  else
    (void) strcat(options,image_info->density);
  /*
    Use Ghostscript to convert Postscript image.
  */
  device=PostscriptColorDevice;
  if (image_info->monochrome)
    device=PostscriptMonoDevice;
  (void) strcpy(filename,image_info->filename);
  for (i=0; i < 50; i++)
  {
    /*
      Ghostscript eats % characters.
    */
    TemporaryFilename(image_info->filename);
    if (strchr(image_info->filename,'%') == (char *) NULL)
      break;
  }
  (void) sprintf(command,PostscriptCommand,device,options,image_info->filename,
    postscript_filename);
  status=SystemCommand(command);
  if (status)
    {
      /*
        Pre GS 3.51 does not support the pnmraw device.
      */
      (void) sprintf(command,PostscriptCommand,"ppmraw",options,
        image_info->filename,postscript_filename);
      status=SystemCommand(command);
    }
  if (access(image_info->filename,0) != 0)
    {
      /*
        Ghostscript requires a showpage operator.
      */
      file=fopen(postscript_filename,"a");
      if (file == (FILE *) NULL)
        PrematureExit("Unable to write file",image);
      (void) fputs("showpage\n",file);
      (void) fclose(file);
      status=SystemCommand(command);
    }
  (void) unlink(postscript_filename);
  if (status)
    {
      /*
        Ghostscript has failed-- try the Display Postscript Extension.
      */
      (void) strcpy(image_info->filename,filename);
      image=ReadDPSImage(image_info);
      if (image != (Image *) NULL)
        return(image);
      Warning("Postscript translation failed",image_info->filename);
      return((Image *) NULL);
    }
  image=ReadPNMImage(image_info);
  (void) unlink(image_info->filename);
  if (image == (Image *) NULL)
    {
      Warning("Postscript translation failed",image_info->filename);
      return((Image *) NULL);
    }
  do
  {
    (void) strcpy(image->filename,filename);
    image->filesize=filesize;
    next_image=image->next;
    if (next_image != (Image *) NULL)
      image=next_image;
  } while (next_image != (Image *) NULL);
  while (image->previous != (Image *) NULL)
    image=image->previous;
  return(image);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%  R e a d R A D I A N C E I m a g e                                          %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Function ReadRADIANCEImage reads a RADIANCE image file and returns it.
%  It allocates the memory necessary for the new Image structure and returns
%  a pointer to the new image.
%
%  The format of the ReadRADIANCEImage routine is:
%
%      image=ReadRADIANCEImage(image_info)
%
%  A description of each parameter follows:
%
%    o image:  Function ReadRADIANCEImage returns a pointer to the image after
%      reading.  A null image is returned if there is a a memory shortage or
%      if the image cannot be read.
%
%    o image_info: Specifies a pointer to an ImageInfo structure.
%
%
*/
static Image *ReadRADIANCEImage(ImageInfo *image_info)
{
  char
    command[MaxTextLength],
    filename[MaxTextLength];

  Image
    *image,
    *next_image;

  int
    status;

  /*
    Allocate image structure.
  */
  image=AllocateImage(image_info);
  if (image == (Image *) NULL)
    return((Image *) NULL);
  /*
    Open image file.
  */
  OpenImage(image,ReadBinaryType);
  if (image->file == (FILE *) NULL)
    PrematureExit("Unable to open file",image);
  CloseImage(image);
  /*
    Use ra_ppm to convert RADIANCE image.
  */
  (void) strcpy(filename,image_info->filename);
  TemporaryFilename(image_info->filename);
  (void) sprintf(command,"ra_ppm %s %s",filename,image_info->filename);
  status=SystemCommand(command);
  if (status)
    PrematureExit("RADIANCE translation failed",image);
  DestroyImage(image);
  image=ReadPNMImage(image_info);
  (void) unlink(image_info->filename);
  if (image == (Image *) NULL)
    PrematureExit("RADIANCE translation failed",image);
  /*
    Assign proper filename.
  */
  do
  {
    (void) strcpy(image->filename,filename);
    next_image=image->next;
    if (next_image != (Image *) NULL)
      image=next_image;
  } while (next_image != (Image *) NULL);
  while (image->previous != (Image *) NULL)
    image=image->previous;
  return(image);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%  R e a d R G B I m a g e                                                    %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Function ReadRGBImage reads an image of raw red, green, and blue bytes and
%  returns it.  It allocates the memory necessary for the new Image structure
%  and returns a pointer to the new image.
%
%  The format of the ReadRGBImage routine is:
%
%      image=ReadRGBImage(image_info)
%
%  A description of each parameter follows:
%
%    o image:  Function ReadRGBImage returns a pointer to the image after
%      reading.  A null image is returned if there is a a memory shortage or
%      if the image cannot be read.
%
%    o image_info: Specifies a pointer to an ImageInfo structure.
%
%
*/
static Image *ReadRGBImage(ImageInfo *image_info)
{
  Image
    *image;

  int
    x,
    y;

  register int
    i;

  register RunlengthPacket
    *q;

  register unsigned char
    *p;

  unsigned char
    *rgb_pixels;

  unsigned int
    height,
    packet_size,
    width;

  unsigned short
    value;

  /*
    Allocate image structure.
  */
  image=AllocateImage(image_info);
  if (image == (Image *) NULL)
    return((Image *) NULL);
  /*
    Open image file.
  */
  OpenImage(image,ReadBinaryType);
  if (image->file == (FILE *) NULL)
    PrematureExit("Unable to open file",image);
  /*
    Determine width and height, e.g. 640x512.
  */
  width=512;
  height=512;
  x=0;
  if (image_info->size != (char *) NULL)
    (void) XParseGeometry(image_info->size,&x,&y,&width,&height);
  for (i=0; i < x; i++)
    (void) fgetc(image->file);
  /*
    Initialize image structure.
  */
  packet_size=3;
  if (strcmp(image_info->magick,"RGBA") == 0)
    {
      image->matte=True;
      packet_size=4;
    }
  image->columns=width;
  image->rows=height;
  image->packets=image->columns*image->rows;
  rgb_pixels=(unsigned char *)
    malloc(packet_size*image->packets*sizeof(unsigned char));
  image->pixels=(RunlengthPacket *)
    malloc(image->packets*sizeof(RunlengthPacket));
  if ((rgb_pixels == (unsigned char *) NULL) ||
      (image->pixels == (RunlengthPacket *) NULL))
    PrematureExit("Unable to allocate memory",image);
  /*
    Convert raster image to runlength-encoded packets.
  */
  (void) ReadData((char *) rgb_pixels,3,image->packets,image->file);
  p=rgb_pixels;
  switch (image_info->interlace)
  {
    case NoneInterlace:
    default:
    {
      /*
        No interlacing:  RGBRGBRGBRGBRGBRGB...
      */
      q=image->pixels;
      for (y=0; y < image->rows; y++)
      {
        for (x=0; x < image->columns; x++)
        {
          ReadQuantum(q->red,p);
          ReadQuantum(q->green,p);
          ReadQuantum(q->blue,p);
          q->index=0;
          if (image->matte)
            q->index=(*p++);
          q->length=0;
          q++;
        }
        ProgressMonitor(LoadImageText,y,image->rows);
      }
      break;
    }
    case LineInterlace:
    {
      /*
        Line interlacing:  RRR...GGG...BBB...RRR...GGG...BBB...
      */
      for (y=0; y < image->rows; y++)
      {
        q=image->pixels+y*image->columns;
        for (x=0; x < image->columns; x++)
        {
          ReadQuantum(q->red,p);
          q->index=0;
          q->length=0;
          q++;
        }
        q=image->pixels+y*image->columns;
        for (x=0; x < image->columns; x++)
        {
          ReadQuantum(q->green,p);
          q++;
        }
        q=image->pixels+y*image->columns;
        for (x=0; x < image->columns; x++)
        {
          ReadQuantum(q->blue,p);
          q++;
        }
        q=image->pixels+y*image->columns;
        if (image->matte)
          for (x=0; x < image->columns; x++)
          {
            ReadQuantum(q->index,p);
            q++;
          }
        ProgressMonitor(LoadImageText,y,image->rows);
      }
      break;
    }
    case PlaneInterlace:
    {
      /*
        Plane interlacing:  RRRRRR...GGGGGG...BBBBBB...
      */
      q=image->pixels;
      for (i=0; i < image->packets; i++)
      {
        ReadQuantum(q->red,p);
        q->index=0;
        q->length=0;
        q++;
      }
      ProgressMonitor(LoadImageText,100,400);
      q=image->pixels;
      for (i=0; i < image->packets; i++)
      {
        ReadQuantum(q->green,p);
        q++;
      }
      ProgressMonitor(LoadImageText,200,400);
      q=image->pixels;
      for (i=0; i < image->packets; i++)
      {
        ReadQuantum(q->blue,p);
        q++;
      }
      ProgressMonitor(LoadImageText,300,400);
      q=image->pixels;
      if (image->matte)
        for (i=0; i < image->packets; i++)
        {
          ReadQuantum(q->index,p);
          q++;
        }
      ProgressMonitor(LoadImageText,400,400);
      break;
    }
  }
  free((char *) rgb_pixels);
  CompressImage(image);
  CloseImage(image);
  return(image);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%  R e a d R L E I m a g e                                                    %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Function ReadRLEImage reads a run-length encoded Utah Raster Toolkit
%  image file and returns it.  It allocates the memory necessary for the new
%  Image structure and returns a pointer to the new image.
%
%  The format of the ReadRLEImage routine is:
%
%      image=ReadRLEImage(image_info)
%
%  A description of each parameter follows:
%
%    o image:  Function ReadRLEImage returns a pointer to the image after
%      reading.  A null image is returned if there is a a memory shortage or
%      if the image cannot be read.
%
%    o image_info: Specifies a pointer to an ImageInfo structure.
%
%
*/
static Image *ReadRLEImage(ImageInfo *image_info)
{
#define SkipLinesOp  0x01
#define SetColorOp  0x02
#define SkipPixelsOp  0x03
#define ByteDataOp  0x05
#define RunDataOp  0x06
#define EOFOp  0x07

  char
    magick[12];

  Image
    *image;

  int
    opcode,
    operand,
    status,
    x,
    y;

  register int
    i,
    j;

  register RunlengthPacket
    *q;

  register unsigned char
    *p;

  unsigned char
    background_color[256],
    *colormap,
    pixel,
    plane,
    *rle_pixels;

  unsigned int
    bits_per_pixel,
    flags,
    map_length,
    number_colormaps,
    number_planes;

  /*
    Allocate image structure.
  */
  image=AllocateImage(image_info);
  if (image == (Image *) NULL)
    return((Image *) NULL);
  /*
    Open image file.
  */
  OpenImage(image,ReadBinaryType);
  if (image->file == (FILE *) NULL)
    PrematureExit("Unable to open file",image);
  /*
    Determine if this is a RLE file.
  */
  status=ReadData((char *) magick,1,2,image->file);
  if ((status == False) || (strncmp(magick,"\122\314",2) != 0))
    PrematureExit("Not a RLE image file",image);
  do
  {
    /*
      Read image header.
    */
    (void) LSBFirstReadShort(image->file);
    (void) LSBFirstReadShort(image->file);
    image->columns=LSBFirstReadShort(image->file);
    image->rows=LSBFirstReadShort(image->file);
    image->packets=image->columns*image->rows;
    flags=fgetc(image->file);
    image->matte=flags & 0x04;
    number_planes=fgetc(image->file);
    bits_per_pixel=fgetc(image->file);
    number_colormaps=fgetc(image->file);
    map_length=1 << fgetc(image->file);
    if ((number_planes == 0) || (number_planes == 2) || (bits_per_pixel != 8) ||
        (image->columns == 0))
      PrematureExit("Unsupported RLE image file",image);
    if (flags & 0x02)
      {
        /*
          No background color-- initialize to black.
        */
        for (i=0; i < number_planes; i++)
          background_color[i]=(unsigned char) 0;
        (void) fgetc(image->file);
      }
    else
      {
        /*
          Initialize background color.
        */
        p=background_color;
        for (i=0; i < number_planes; i++)
          *p++=(unsigned char) fgetc(image->file);
      }
    if ((number_planes & 0x01) == 0)
      (void) fgetc(image->file);
    colormap=(unsigned char *) NULL;
    if (number_colormaps != 0)
      {
        /*
          Read image colormaps.
        */
        colormap=(unsigned char *)
          malloc(number_colormaps*map_length*sizeof(unsigned char));
        if (colormap == (unsigned char *) NULL)
          PrematureExit("Unable to allocate memory",image);
        p=colormap;
        for (i=0; i < number_colormaps; i++)
          for (j=0; j < map_length; j++)
            *p++=XDownScale(LSBFirstReadShort(image->file));
      }
    if (flags & 0x08)
      {
        unsigned int
          length;

        /*
          Read image comment.
        */
        length=LSBFirstReadShort(image->file);
        image->comments=(char *) malloc(length*sizeof(char));
        if (image->comments == (char *) NULL)
          PrematureExit("Unable to allocate memory",image);
        (void) ReadData((char *) image->comments,1,length-1,image->file);
        image->comments[length-1]='\0';
        if ((length & 0x01) == 0)
          (void) fgetc(image->file);
      }
    /*
      Allocate RLE pixels.
    */
    if (image->matte)
      number_planes++;
    rle_pixels=(unsigned char *)
      malloc(image->packets*number_planes*sizeof(unsigned char));
    if (rle_pixels == (unsigned char *) NULL)
      PrematureExit("Unable to allocate memory",image);
    if ((flags & 0x01) && !(flags & 0x02))
      {
        /*
          Set background color.
        */
        p=rle_pixels;
        for (i=0; i < image->packets; i++)
        {
          if (!image->matte)
            for (j=0; j < number_planes; j++)
              *p++=background_color[j];
          else
            {
              for (j=0; j < (number_planes-1); j++)
                *p++=background_color[j];
              *p++=0;  /* initialize matte channel */
            }
        }
      }
    /*
      Read runlength-encoded image.
    */
    plane=0;
    x=0;
    y=0;
    opcode=fgetc(image->file);
    do
    {
      switch (opcode & 0x3f)
      {
        case SkipLinesOp:
        {
          operand=fgetc(image->file);
          if (opcode & 0x40)
            operand=LSBFirstReadShort(image->file);
          x=0;
          y+=operand;
          break;
        }
        case SetColorOp:
        {
          operand=fgetc(image->file);
          plane=operand;
          if (plane == 255)
            plane=number_planes-1;
          x=0;
          break;
        }
        case SkipPixelsOp:
        {
          operand=fgetc(image->file);
          if (opcode & 0x40)
            operand=LSBFirstReadShort(image->file);
          x+=operand;
          break;
        }
        case ByteDataOp:
        {
          operand=fgetc(image->file);
          if (opcode & 0x40)
            operand=LSBFirstReadShort(image->file);
          p=rle_pixels+((image->rows-y-1)*image->columns*number_planes)+
            x*number_planes+plane;
          operand++;
          for (i=0; i < operand; i++)
          {
            pixel=fgetc(image->file);
            if ((y < image->rows) && ((x+i) < image->columns))
              *p=pixel;
            p+=number_planes;
          }
          if (operand & 0x01)
            (void) fgetc(image->file);
          x+=operand;
          break;
        }
        case RunDataOp:
        {
          operand=fgetc(image->file);
          if (opcode & 0x40)
            operand=LSBFirstReadShort(image->file);
          pixel=fgetc(image->file);
          (void) fgetc(image->file);
          operand++;
          p=rle_pixels+((image->rows-y-1)*image->columns*number_planes)+
            x*number_planes+plane;
          for (i=0; i < operand; i++)
          {
            if ((y < image->rows) && ((x+i) < image->columns))
              *p=pixel;
            p+=number_planes;
          }
          x+=operand;
          break;
        }
        default:
          break;
      }
      opcode=fgetc(image->file);
    } while (((opcode & 0x3f) != EOFOp) && (opcode != EOF));
    if (number_colormaps != 0)
      {
        unsigned int
          mask;

        /*
          Apply colormap transformation to image.
        */
        mask=(map_length-1);
        p=rle_pixels;
        if (number_colormaps == 1)
          for (i=0; i < image->packets; i++)
          {
            *p=(unsigned char) colormap[*p & mask];
            p++;
          }
        else
          if ((number_planes >= 3) && (number_colormaps >= 3))
            for (i=0; i < image->packets; i++)
              for (j=0; j < number_planes; j++)
              {
                *p=(unsigned char) colormap[j*map_length+(*p & mask)];
                p++;
              }
      }
    /*
      Initialize image structure.
    */
    image->pixels=(RunlengthPacket *)
      malloc(image->packets*sizeof(RunlengthPacket));
    if (image->pixels == (RunlengthPacket *) NULL)
      PrematureExit("Unable to allocate memory",image);
    q=image->pixels;
    if (number_planes >= 3)
      {
        /*
          Convert raster image to DirectClass runlength-encoded packets.
        */
        p=rle_pixels;
        for (y=0; y < image->rows; y++)
        {
          for (x=0; x < image->columns; x++)
          {
            q->red=UpScale(*p++);
            q->green=UpScale(*p++);
            q->blue=UpScale(*p++);
            q->index=(unsigned short) (image->matte ? (*p++) : 0);
            q->length=0;
            q++;
          }
          ProgressMonitor(LoadImageText,y,image->rows);
        }
      }
    else
      {
        /*
          Create colormap.
        */
        image->class=PseudoClass;
        if (number_colormaps == 0)
          map_length=256;
        image->colors=map_length;
        image->colormap=(ColorPacket *)
          malloc(image->colors*sizeof(ColorPacket));
        if (image->colormap == (ColorPacket *) NULL)
          PrematureExit("Unable to allocate memory",image);
        p=colormap;
        if (number_colormaps == 0)
          for (i=0; i < image->colors; i++)
          {
            /*
              Grayscale.
            */
            image->colormap[i].red=(MaxRGB*i)/(image->colors-1);
            image->colormap[i].green=(MaxRGB*i)/(image->colors-1);
            image->colormap[i].blue=(MaxRGB*i)/(image->colors-1);
          }
        else
          if (number_colormaps == 1)
            for (i=0; i < image->colors; i++)
            {
              /*
                Pseudocolor.
              */
              image->colormap[i].red=(Quantum) UpScale(i);
              image->colormap[i].green=(Quantum) UpScale(i);
              image->colormap[i].blue=(Quantum) UpScale(i);
            }
          else
            for (i=0; i < image->colors; i++)
            {
              image->colormap[i].red=UpScale(*p);
              image->colormap[i].green=UpScale(*(p+map_length));
              image->colormap[i].blue=UpScale(*(p+map_length*2));
              p++;
            }
        p=rle_pixels;
        if (!image->matte)
          {
            /*
              Convert raster image to PseudoClass runlength-encoded packets.
            */
            for (y=0; y < image->rows; y++)
            {
              for (x=0; x < image->columns; x++)
              {
                q->index=(unsigned short) (*p++);
                q->length=0;
                q++;
              }
              ProgressMonitor(LoadImageText,y,image->rows);
            }
            SyncImage(image);
          }
        else
          {
            /*
              Image has a matte channel-- promote to DirectClass.
            */
            for (y=0; y < image->rows; y++)
            {
              for (x=0; x < image->columns; x++)
              {
                q->red=image->colormap[*p].red;
                q->green=image->colormap[*p].green;
                q->blue=image->colormap[*p++].blue;
                q->index=(unsigned short) (*p++);
                q->length=0;
                q++;
              }
              ProgressMonitor(LoadImageText,y,image->rows);
            }
            free(image->colormap);
            image->colormap=(ColorPacket *) NULL;
            image->class=DirectClass;
            image->colors=0;
          }
      }
    if (number_colormaps != 0)
      free((char *) colormap);
    free((char *) rle_pixels);
    CompressImage(image);
    /*
      Proceed to next image.
    */
    (void) fgetc(image->file);
    status=ReadData((char *) magick,1,2,image->file);
    if ((status == True) && (strncmp(magick,"\122\314",2) == 0))
      {
        /*
          Allocate next image structure.
        */
        image->next=AllocateImage(image_info);
        if (image->next == (Image *) NULL)
          {
            DestroyImages(image);
            return((Image *) NULL);
          }
        (void) strcpy(image->next->filename,image_info->filename);
        image->next->file=image->file;
        image->next->scene=image->scene+1;
        image->next->previous=image;
        image=image->next;
      }
  } while ((status == True) && (strncmp(magick,"\122\314",2) == 0));
  while (image->previous != (Image *) NULL)
    image=image->previous;
  CloseImage(image);
  return(image);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%  R e a d S G I I m a g e                                                    %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Function ReadSGIImage reads a SGI RGB image file and returns it.  It
%  allocates the memory necessary for the new Image structure and returns a
%  pointer to the new image.
%
%  The format of the ReadSGIImage routine is:
%
%      image=ReadSGIImage(image_info)
%
%  A description of each parameter follows:
%
%    o image:  Function ReadSGIImage returns a pointer to the image after
%      reading.  A null image is returned if there is a a memory shortage or
%      if the image cannot be read.
%
%    o image_info: Specifies a pointer to an ImageInfo structure.
%
%
*/

static void SGIDecode(unsigned char *packets, unsigned char *pixels)
{
  unsigned char
    count,
    pixel;

  for ( ; ;)
  {
    pixel=(*packets++);
    count=pixel & 0x7f;
    if (count == 0)
      break;
    if (pixel & 0x80)
      for ( ; count != 0; count--)
      {
        *pixels=(*packets++);
        pixels+=4;
      }
    else
      {
        pixel=(*packets++);
        for ( ; count != 0; count--)
        {
          *pixels=pixel;
          pixels+=4;
        }
      }
  }
}

static Image *ReadSGIImage(ImageInfo *image_info)
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

  Image
    *image;

  SGIHeader
    iris_header;

  register int
    i,
    x,
    y,
    z;

  register RunlengthPacket
    *q;

  register unsigned char
    *p;

  unsigned char
    *iris_pixels;

  /*
    Allocate image structure.
  */
  image=AllocateImage(image_info);
  if (image == (Image *) NULL)
    return((Image *) NULL);
  /*
    Open image file.
  */
  OpenImage(image,ReadBinaryType);
  if (image->file == (FILE *) NULL)
    PrematureExit("Unable to open file",image);
  /*
    Read SGI raster header.
  */
  iris_header.magic=MSBFirstReadShort(image->file);
  do
  {
    /*
      Verify SGI identifier.
    */
    if (iris_header.magic != 0x01DA)
      PrematureExit("Not a SGI RGB image",image);
    iris_header.storage=fgetc(image->file);
    iris_header.bytes_per_pixel=fgetc(image->file);
    if (iris_header.bytes_per_pixel != 1)
      PrematureExit("Image must have 1 Byte per pixel channel",image);
    iris_header.dimension=MSBFirstReadShort(image->file);
    iris_header.columns=MSBFirstReadShort(image->file);
    iris_header.rows=MSBFirstReadShort(image->file);
    iris_header.depth=MSBFirstReadShort(image->file);
    iris_header.minimum_value=MSBFirstReadLong(image->file);
    iris_header.maximum_value=MSBFirstReadLong(image->file);
    (void) ReadData((char *) iris_header.filler,1,
      (unsigned int) sizeof(iris_header.filler),image->file);
    /*
      Allocate SGI pixels.
    */
    iris_pixels=(unsigned char *)
      malloc(4*iris_header.columns*iris_header.rows*sizeof(unsigned char));
    if (iris_pixels == (unsigned char *) NULL)
      PrematureExit("Unable to allocate memory",image);
    if (iris_header.storage != 0x01)
      {
        unsigned char
          *scanline;

        /*
          Read standard image format.
        */
        scanline=(unsigned char *)
          malloc(iris_header.columns*sizeof(unsigned char));
        if (scanline == (unsigned char *) NULL)
          PrematureExit("Unable to allocate memory",image);
        for (z=0; z < (int) iris_header.depth; z++)
        {
          p=iris_pixels+z;
          for (y=0; y < (int) iris_header.rows; y++)
          {
            (void) ReadData((char *) scanline,1,iris_header.columns,
              image->file);
            for (x=0; x < (int) iris_header.columns; x++)
            {
              *p=scanline[x];
              p+=4;
            }
          }
        }
        free((char *) scanline);
      }
    else
      {
        unsigned char
          *packets;

        unsigned int
          data_order;

        unsigned long
          offset,
          *offsets,
          *runlength;

        /*
          Read runlength-encoded image format.
        */
        offsets=(unsigned long *)
          malloc(iris_header.rows*iris_header.depth*sizeof(unsigned long));
        packets=(unsigned char *)
          malloc(((iris_header.columns << 1)+10)*sizeof(unsigned char));
        runlength=(unsigned long *)
          malloc(iris_header.rows*iris_header.depth*sizeof(unsigned long));
        if ((offsets == (unsigned long *) NULL) ||
            (packets == (unsigned char *) NULL) ||
            (runlength == (unsigned long *) NULL))
          PrematureExit("Unable to allocate memory",image);
        for (i=0; i < (int) (iris_header.rows*iris_header.depth); i++)
          offsets[i]=MSBFirstReadLong(image->file);
        for (i=0; i < (int) (iris_header.rows*iris_header.depth); i++)
          runlength[i]=MSBFirstReadLong(image->file);
        /*
          Check data order.
        */
        offset=0;
        data_order=0;
        for (y=0; ((y < (int) iris_header.rows) && !data_order); y++)
          for (z=0; ((z < (int) iris_header.depth) && !data_order); z++)
          {
            if (offsets[y+z*iris_header.rows] < offset)
              data_order=1;
            offset=offsets[y+z*iris_header.rows];
          }
        offset=512+4*((iris_header.rows*iris_header.depth) << 1);
        if (data_order == 1)
          {
            for (z=0; z < (int) iris_header.depth; z++)
            {
              p=iris_pixels;
              for (y=0; y < (int) iris_header.rows; y++)
              {
                if (offset != offsets[y+z*iris_header.rows])
                  {
                    offset=offsets[y+z*iris_header.rows];
                    (void) fseek(image->file,(int) offset,0);
                  }
                (void) ReadData((char *) packets,1,
                  (unsigned int) runlength[y+z*iris_header.rows],image->file);
                offset+=runlength[y+z*iris_header.rows];
                SGIDecode(packets,p+z);
                p+=(iris_header.columns*4);
              }
            }
          }
        else
          {
            p=iris_pixels;
            for (y=0; y < (int) iris_header.rows; y++)
            {
              for (z=0; z < (int) iris_header.depth; z++)
              {
                if (offset != offsets[y+z*iris_header.rows])
                  {
                    offset=offsets[y+z*iris_header.rows];
                    (void) fseek(image->file,(int) offset,0);
                  }
                (void) ReadData((char *) packets,1,
                  (unsigned int) runlength[y+z*iris_header.rows],image->file);
                offset+=runlength[y+z*iris_header.rows];
                SGIDecode(packets,p+z);
              }
              p+=(iris_header.columns*4);
            }
          }
        free(runlength);
        free(packets);
        free(offsets);
      }
    /*
      Initialize image structure.
    */
    image->matte=iris_header.depth == 4;
    image->columns=iris_header.columns;
    image->rows=iris_header.rows;
    image->packets=image->columns*image->rows;
    image->pixels=(RunlengthPacket *)
      malloc(image->packets*sizeof(RunlengthPacket));
    if (image->pixels == (RunlengthPacket *) NULL)
      PrematureExit("Unable to allocate memory",image);
    /*
      Convert SGI raster image to runlength-encoded packets.
    */
    q=image->pixels;
    if (iris_header.depth >= 3)
      {
        /*
          Convert SGI image to DirectClass runlength-encoded packets.
        */
        for (y=0; y < image->rows; y++)
        {
          p=iris_pixels+((image->rows-1)-y)*(image->columns*4);
          for (x=0; x < image->columns; x++)
          {
            q->red=UpScale(*p);
            q->green=UpScale(*(p+1));
            q->blue=UpScale(*(p+2));
            q->index=(*(p+3));
            q->length=0;
            p+=4;
            q++;
          }
          ProgressMonitor(LoadImageText,y,image->rows);
        }
      }
    else
      {
        unsigned short
          index;

        /*
          Create grayscale map.
        */
        image->class=PseudoClass;
        image->colors=256;
        image->colormap=(ColorPacket *)
          malloc(image->colors*sizeof(ColorPacket));
        if (image->colormap == (ColorPacket *) NULL)
          PrematureExit("Unable to allocate memory",image);
        for (i=0; i < image->colors; i++)
        {
          image->colormap[i].red=(Quantum) UpScale(i);
          image->colormap[i].green=(Quantum) UpScale(i);
          image->colormap[i].blue=(Quantum) UpScale(i);
        }
        /*
          Convert SGI image to PseudoClass runlength-encoded packets.
        */
        for (y=0; y < image->rows; y++)
        {
          p=iris_pixels+((image->rows-1)-y)*(image->columns*4);
          for (x=0; x < image->columns; x++)
          {
            index=(unsigned short) (*p);
            q->index=index;
            q->length=0;
            p+=4;
            q++;
          }
          ProgressMonitor(LoadImageText,y,image->rows);
        }
        SyncImage(image);
      }
    free((char *) iris_pixels);
    CompressImage(image);
    /*
      Proceed to next image.
    */
    iris_header.magic=MSBFirstReadShort(image->file);
    if (iris_header.magic == 0x01DA)
      {
        /*
          Allocate image structure.
        */
        image->next=AllocateImage(image_info);
        if (image->next == (Image *) NULL)
          {
            DestroyImages(image);
            return((Image *) NULL);
          }
        (void) strcpy(image->next->filename,image_info->filename);
        image->next->file=image->file;
        image->next->scene=image->scene+1;
        image->next->previous=image;
        image=image->next;
      }
  } while (iris_header.magic == 0x01DA);
  while (image->previous != (Image *) NULL)
    image=image->previous;
  CloseImage(image);
  return(image);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%  R e a d S U N I m a g e                                                    %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Function ReadSUNImage reads a SUN image file and returns it.  It allocates
%  the memory necessary for the new Image structure and returns a pointer to
%  the new image.
%
%  The format of the ReadSUNImage routine is:
%
%      image=ReadSUNImage(image_info)
%
%  A description of each parameter follows:
%
%    o image:  Function ReadSUNImage returns a pointer to the image after
%      reading.  A null image is returned if there is a a memory shortage or
%      if the image cannot be read.
%
%    o image_info: Specifies a pointer to an ImageInfo structure.
%
%
*/
static Image *ReadSUNImage(ImageInfo *image_info)
{
#define RMT_EQUAL_RGB  1
#define RMT_NONE  0
#define RMT_RAW  2
#define RT_STANDARD  1
#define RT_ENCODED  2
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

  Image
    *image;

  register int
    bit,
    i,
    x,
    y;

  register RunlengthPacket
    *q;

  register unsigned char
    *p;

  SUNHeader
    sun_header;

  unsigned char
    *sun_data,
    *sun_pixels;

  unsigned int
    status;

  /*
    Allocate image structure.
  */
  image=AllocateImage(image_info);
  if (image == (Image *) NULL)
    return((Image *) NULL);
  /*
    Open image file.
  */
  OpenImage(image,ReadBinaryType);
  if (image->file == (FILE *) NULL)
    PrematureExit("Unable to open file",image);
  /*
    Read SUN raster header.
  */
  sun_header.magic=MSBFirstReadLong(image->file);
  do
  {
    /*
      Verify SUN identifier.
    */
    if (sun_header.magic != 0x59a66a95)
      PrematureExit("Not a SUN raster image",image);
    sun_header.width=MSBFirstReadLong(image->file);
    sun_header.height=MSBFirstReadLong(image->file);
    sun_header.depth=MSBFirstReadLong(image->file);
    sun_header.length=MSBFirstReadLong(image->file);
    sun_header.type=MSBFirstReadLong(image->file);
    sun_header.maptype=MSBFirstReadLong(image->file);
    sun_header.maplength=MSBFirstReadLong(image->file);
    switch (sun_header.maptype)
    {
      case RMT_NONE:
      {
        if (sun_header.depth < 24)
          {
            /*
              Create linear color ramp.
            */
            image->colors=1 << sun_header.depth;
            image->colormap=(ColorPacket *)
              malloc(image->colors*sizeof(ColorPacket));
            if (image->colormap == (ColorPacket *) NULL)
              PrematureExit("Unable to allocate memory",image);
            for (i=0; i < image->colors; i++)
            {
              image->colormap[i].red=(MaxRGB*i)/(image->colors-1);
              image->colormap[i].green=(MaxRGB*i)/(image->colors-1);
              image->colormap[i].blue=(MaxRGB*i)/(image->colors-1);
            }
          }
        break;
      }
      case RMT_EQUAL_RGB:
      {
        unsigned char
          *sun_colormap;

        /*
          Read SUN raster colormap.
        */
        image->colors=sun_header.maplength/3;
        image->colormap=(ColorPacket *)
          malloc(image->colors*sizeof(ColorPacket));
        sun_colormap=(unsigned char *)
          malloc(image->colors*sizeof(unsigned char));
        if ((image->colormap == (ColorPacket *) NULL) ||
            (sun_colormap == (unsigned char *) NULL))
          PrematureExit("Unable to allocate memory",image);
        (void) ReadData((char *) sun_colormap,1,image->colors,image->file);
        for (i=0; i < image->colors; i++)
          image->colormap[i].red=UpScale(sun_colormap[i]);
        (void) ReadData((char *) sun_colormap,1,image->colors,image->file);
        for (i=0; i < image->colors; i++)
          image->colormap[i].green=UpScale(sun_colormap[i]);
        (void) ReadData((char *) sun_colormap,1,image->colors,image->file);
        for (i=0; i < image->colors; i++)
          image->colormap[i].blue=UpScale(sun_colormap[i]);
        free((char *) sun_colormap);
        break;
      }
      case RMT_RAW:
      {
        unsigned char
          *sun_colormap;

        /*
          Read SUN raster colormap.
        */
        sun_colormap=(unsigned char *)
          malloc(sun_header.maplength*sizeof(unsigned char));
        if (sun_colormap == (unsigned char *) NULL)
          PrematureExit("Unable to allocate memory",image);
        (void) ReadData((char *) sun_colormap,1,(unsigned int)
          sun_header.maplength,image->file);
        free((char *) sun_colormap);
        break;
      }
      default:
        PrematureExit("Colormap type is not supported",image);
    }
    sun_data=(unsigned char *) malloc(sun_header.length*sizeof(unsigned char));
    if (sun_data == (unsigned char *) NULL)
      PrematureExit("Unable to allocate memory",image);
    status=ReadData((char *) sun_data,1,(unsigned int) sun_header.length,
      image->file);
    if ((status == False) && (sun_header.type != RT_ENCODED))
      PrematureExit("Unable to read image data",image);
    sun_pixels=sun_data;
    if (sun_header.type == RT_ENCODED)
      {
        unsigned int
          width,
          height;

        /*
          Read run-length encoded raster pixels.
        */
        width=sun_header.width*(((sun_header.depth-1) >> 3)+1);
        height=sun_header.height;
        sun_pixels=(unsigned char *) malloc(width*height*sizeof(unsigned char));
        if (sun_pixels == (unsigned char *) NULL)
          PrematureExit("Unable to allocate memory",image);
        (void) SUNDecodeImage(sun_data,sun_pixels,width,height);
        free((char *) sun_data);
      }
    /*
      Initialize image structure.
    */
    image->matte=(sun_header.depth == 32);
    image->class=(sun_header.depth < 24 ? PseudoClass : DirectClass);
    image->columns=sun_header.width;
    image->rows=sun_header.height;
    image->packets=image->columns*image->rows;
    image->pixels=(RunlengthPacket *)
      malloc(image->packets*sizeof(RunlengthPacket));
    if (image->pixels == (RunlengthPacket *) NULL)
      PrematureExit("Unable to allocate memory",image);
    /*
      Convert SUN raster image to runlength-encoded packets.
    */
    p=sun_pixels;
    q=image->pixels;
    if (sun_header.depth == 1)
      for (y=0; y < image->rows; y++)
      {
        /*
          Convert bitmap scanline to runlength-encoded color packets.
        */
        for (x=0; x < (image->columns >> 3); x++)
        {
          for (bit=7; bit >= 0; bit--)
          {
            q->index=((*p) & (0x01 << bit) ? 0x00 : 0x01);
            q->length=0;
            q++;
          }
          p++;
        }
        if ((image->columns % 8) != 0)
          {
            for (bit=7; bit >= (8-(image->columns % 8)); bit--)
            {
              q->index=((*p) & (0x01 << bit) ? 0x00 : 0x01);
              q->length=0;
              q++;
            }
            p++;
          }
        if ((((image->columns/8)+(image->columns % 8 ? 1 : 0)) % 2) != 0)
          p++;
        ProgressMonitor(LoadImageText,y,image->rows);
      }
    else
      if (image->class == PseudoClass)
        for (y=0; y < image->rows; y++)
        {
          /*
            Convert PseudoColor scanline to runlength-encoded color packets.
          */
          for (x=0; x < image->columns; x++)
          {
            q->index=(*p++);
            q->length=0;
            q++;
          }
          if ((image->columns % 2) != 0)
            p++;
          ProgressMonitor(LoadImageText,y,image->rows);
        }
      else
        for (y=0; y < image->rows; y++)
        {
          /*
            Convert DirectColor scanline to runlength-encoded color packets.
          */
          for (x=0; x < image->columns; x++)
          {
            q->index=(unsigned short) (image->matte ? (*p++) : 0);
            if (sun_header.type == RT_STANDARD)
              {
                q->blue=UpScale(*p++);
                q->green=UpScale(*p++);
                q->red=UpScale(*p++);
              }
            else
              {
                q->red=UpScale(*p++);
                q->green=UpScale(*p++);
                q->blue=UpScale(*p++);
              }
            if (image->colors != 0)
              {
                q->red=image->colormap[q->red].red;
                q->green=image->colormap[q->green].green;
                q->blue=image->colormap[q->blue].blue;
              }
            q->length=0;
            q++;
          }
          if (((image->columns % 2) != 0) && (image->matte == False))
            p++;
          ProgressMonitor(LoadImageText,y,image->rows);
        }
    free((char *) sun_pixels);
    if (image->class == PseudoClass)
      {
        SyncImage(image);
        CompressColormap(image);
      }
    CompressImage(image);
    /*
      Proceed to next image.
    */
    sun_header.magic=MSBFirstReadLong(image->file);
    if (sun_header.magic == 0x59a66a95)
      {
        /*
          Allocate image structure.
        */
        image->next=AllocateImage(image_info);
        if (image->next == (Image *) NULL)
          {
            DestroyImages(image);
            return((Image *) NULL);
          }
        (void) strcpy(image->next->filename,image_info->filename);
        image->next->file=image->file;
        image->next->scene=image->scene+1;
        image->next->previous=image;
        image=image->next;
      }
  } while (sun_header.magic == 0x59a66a95);
  while (image->previous != (Image *) NULL)
    image=image->previous;
  CloseImage(image);
  return(image);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%  R e a d T A R G A I m a g e                                                %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Function ReadTARGAImage reads a Truevision Targa image file and returns
%  it.  It allocates the memory necessary for the new Image structure and
%  returns a pointer to the new image.
%
%  The format of the ReadTARGAImage routine is:
%
%      image=ReadTARGAImage(image_info)
%
%  A description of each parameter follows:
%
%    o image:  Function ReadTARGAImage returns a pointer to the image after
%      reading.  A null image is returned if there is a a memory shortage or
%      if the image cannot be read.
%
%    o image_info: Specifies a pointer to an ImageInfo structure.
%
%
*/
static Image *ReadTARGAImage(ImageInfo *image_info)
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
    *image;

  Quantum
    blue,
    green,
    red;

  register int
    i,
    x,
    y;

  register RunlengthPacket
    *q;

  TargaHeader
    targa_header;

  unsigned char
    j,
    k,
    runlength;

  unsigned int
    base,
    flag,
    real,
    skip,
    status,
    im_true;

  unsigned short
    index;

  /*
    Allocate image structure.
  */
  image=AllocateImage(image_info);
  if (image == (Image *) NULL)
    return((Image *) NULL);
  /*
    Open image file.
  */
  OpenImage(image,ReadBinaryType);
  if (image->file == (FILE *) NULL)
    PrematureExit("Unable to open file",image);
  /*
    Read TARGA header information.
  */
  status=ReadData((char *) &targa_header.id_length,1,1,image->file);
  targa_header.colormap_type=fgetc(image->file);
  targa_header.image_type=fgetc(image->file);
  do
  {
    if ((status == False) || (targa_header.image_type == 0) ||
        (targa_header.image_type > 11))
      PrematureExit("Not a TARGA image file",image);
    targa_header.colormap_index=LSBFirstReadShort(image->file);
    targa_header.colormap_length=LSBFirstReadShort(image->file);
    targa_header.colormap_size=fgetc(image->file);
    targa_header.x_origin=LSBFirstReadShort(image->file);
    targa_header.y_origin=LSBFirstReadShort(image->file);
    targa_header.width=LSBFirstReadShort(image->file);
    targa_header.height=LSBFirstReadShort(image->file);
    targa_header.pixel_size=fgetc(image->file);
    targa_header.attributes=fgetc(image->file);
    /*
      Initialize image structure.
    */
    image->matte=targa_header.pixel_size == 32;
    image->columns=targa_header.width;
    image->rows=targa_header.height;
    image->packets=image->columns*image->rows;
    image->pixels=(RunlengthPacket *)
      malloc(image->packets*sizeof(RunlengthPacket));
    if (image->pixels == (RunlengthPacket *) NULL)
      PrematureExit("Unable to allocate memory",image);
    if (targa_header.id_length != 0)
      {
        /*
          TARGA image comment.
        */
        image->comments=(char *)
          malloc((targa_header.id_length+1)*sizeof(char));
        if (image->comments == (char *) NULL)
          PrematureExit("Unable to allocate memory",image);
        (void) ReadData(image->comments,1,targa_header.id_length,image->file);
        image->comments[targa_header.id_length]='\0';
      }
    if (targa_header.colormap_type != 0)
      {
        /*
          Read TARGA raster colormap.
        */
        if ((targa_header.image_type == TargaRLEColormap) ||
            (targa_header.image_type == TargaRLERGB))
          image->class=PseudoClass;
        image->colors=targa_header.colormap_length;
        image->colormap=(ColorPacket *)
          malloc(image->colors*sizeof(ColorPacket));
        if (image->colormap == (ColorPacket *) NULL)
          PrematureExit("Unable to allocate memory",image);
        for (i=0; i < image->colors; i++)
        {
          switch (targa_header.colormap_size)
          {
            case 8:
            default:
            {
              /*
                Gray scale.
              */
              red=UpScale(fgetc(image->file));
              green=red;
              blue=red;
              break;
            }
            case 15:
            case 16:
            {
              /*
                5 bits each of red green and blue.
              */
              j=fgetc(image->file);
              k=fgetc(image->file);
              red=(Quantum) ((MaxRGB*((int) (k & 0x7c) >> 2))/31);
              green=(Quantum)
                ((MaxRGB*(((int) (k & 0x03) << 3)+((int) (j & 0xe0) >> 5)))/31);
              blue=(Quantum) ((MaxRGB*((int) (j & 0x1f)))/31);
              break;
            }
            case 32:
            case 24:
            {
              /*
                8 bits each of blue green and red.
              */
              blue=UpScale(fgetc(image->file));
              green=UpScale(fgetc(image->file));
              red=UpScale(fgetc(image->file));
              break;
            }
          }
          image->colormap[i].red=red;
          image->colormap[i].green=green;
          image->colormap[i].blue=blue;
        }
      }
    /*
      Convert TARGA pixels to runlength-encoded packets.
    */
    base=0;
    flag=0;
    index=0;
    skip=False;
    real=0;
    runlength=0;
    im_true=0;
    for (y=0; y < image->rows; y++)
    {
      real=im_true;
      if (((unsigned char) (targa_header.attributes & 0x20) >> 5) == 0)
        real=image->rows-real-1;
      q=image->pixels+(real*image->columns);
      for (x=0; x < image->columns; x++)
      {
        if ((targa_header.image_type == TargaRLEColormap) ||
            (targa_header.image_type == TargaRLERGB) ||
            (targa_header.image_type == TargaRLEMonochrome))
          if (runlength != 0)
            {
              runlength--;
              skip=flag != 0;
            }
          else
            {
              status=ReadData((char *) &runlength,1,1,image->file);
              if (status == False)
                PrematureExit("Unable to read image data",image);
              flag=runlength & 0x80;
              if (flag != 0)
                runlength-=128;
              skip=False;
            }
        if (!skip)
          switch (targa_header.pixel_size)
          {
            case 8:
            default:
            {
              /*
                Gray scale.
              */
              index=fgetc(image->file);
              if (targa_header.colormap_type == 0)
                {
                  red=(Quantum) UpScale(index);
                  green=(Quantum) UpScale(index);
                  blue=(Quantum) UpScale(index);
                }
              else
                {
                  red=image->colormap[index].red;
                  green=image->colormap[index].green;
                  blue=image->colormap[index].blue;
                }
              break;
            }
            case 15:
            case 16:
            {
              /*
                5 bits each of red green and blue.
              */
              j=fgetc(image->file);
              k=fgetc(image->file);
              red=(Quantum) ((MaxRGB*((int) (k & 0x7c) >> 2))/31);
              green=(Quantum)
                ((MaxRGB*(((int) (k & 0x03) << 3)+((int) (j & 0xe0) >> 5)))/31);
              blue=(Quantum) ((MaxRGB*((int) (j & 0x1f)))/31);
              index=((unsigned short) k << 8)+j;
              break;
            }
            case 24:
            case 32:
            {
              /*
                8 bits each of blue green and red.
              */
              blue=UpScale(fgetc(image->file));
              green=UpScale(fgetc(image->file));
              red=UpScale(fgetc(image->file));
              if (targa_header.pixel_size == 32)
                index=fgetc(image->file);
              break;
            }
          }
        if (status == False)
          PrematureExit("Unable to read image data",image);
        q->red=red;
        q->green=green;
        q->blue=blue;
        q->index=index;
        q->length=0;
        q++;
      }
      if (((unsigned char) (targa_header.attributes & 0xc0) >> 6) == 4)
        im_true+=4;
      else
        if (((unsigned char) (targa_header.attributes & 0xc0) >> 6) == 2)
          im_true+=2;
        else
          im_true++;
      if (im_true >= image->rows)
        {
          base++;
          im_true=base;
        }
      ProgressMonitor(LoadImageText,y,image->rows);
    }
    (void) IsGrayImage(image);
    if (image->class == PseudoClass)
      SyncImage(image);
    CompressImage(image);
    /*
      Proceed to next image.
    */
    status=ReadData((char *) &targa_header.id_length,1,1,image->file);
    targa_header.colormap_type=fgetc(image->file);
    targa_header.image_type=fgetc(image->file);
    status&=((targa_header.image_type != 0) && (targa_header.image_type <= 11));
    if (status == True)
      {
        /*
          Allocate image structure.
        */
        image->next=AllocateImage(image_info);
        if (image->next == (Image *) NULL)
          {
            DestroyImages(image);
            return((Image *) NULL);
          }
        (void) strcpy(image->next->filename,image_info->filename);
        image->next->file=image->file;
        image->next->scene=image->scene+1;
        image->next->previous=image;
        image=image->next;
      }
  } while (status == True);
  while (image->previous != (Image *) NULL)
    image=image->previous;
  CloseImage(image);
  return(image);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%  R e a d T E X T I m a g e                                                  %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Function ReadTEXTImage reads a text file and returns it as an image.  It
%  allocates the memory necessary for the new Image structure and returns a
%  pointer to the new image.
%
%  The format of the ReadTEXTImage routine is:
%
%      image=ReadTEXTImage(image_info)
%
%  A description of each parameter follows:
%
%    o image:  Function ReadTEXTImage returns a pointer to the image after
%      reading. A null image is returned if there is a a memory shortage or if
%      the image cannot be read.
%
%    o image_info: Specifies a pointer to an ImageInfo structure.
%
%
*/
static Image *ReadTEXTImage(ImageInfo *image_info)
{
  AnnotateInfo
    annotate_info;

  char
    geometry[MaxTextLength],
    text[MaxTextLength],
    *text_status;

  Display
    *display;

  Image
    *image;

  int
    offset,
    x,
    y;

  register int
    i;

  register RunlengthPacket
    *p;

  RunlengthPacket
    background_color;

  unsigned int
    height,
    width;

  XColor
    color;

  XResourceInfo
    resource_info;

  /*
    Allocate image structure.
  */
  image=AllocateImage(image_info);
  if (image == (Image *) NULL)
    return((Image *) NULL);
  /*
    Open image file.
  */
  OpenImage(image,"r");
  if (image->file == (FILE *) NULL)
    PrematureExit("Unable to open file",image);
  /*
    Initialize Image structure.
  */
  (void) XParseGeometry(TextPageGeometry,&x,&y,&width,&height);
  if (image_info->size != (char *) NULL)
    (void) XParseGeometry(image_info->size,&x,&y,&width,&height);
  image->columns=width;
  image->rows=height;
  image->packets=image->columns*image->rows;
  image->pixels=(RunlengthPacket *)
    malloc(image->packets*sizeof(RunlengthPacket));
  if (image->pixels == (RunlengthPacket *) NULL)
    PrematureExit("Unable to allocate memory",image);
  /*
    Initialize image annotation info.
  */
  GetAnnotateInfo(&annotate_info);
  annotate_info.server_name=image_info->server_name;
  annotate_info.font=image_info->font;
  annotate_info.text=text;
  annotate_info.geometry=geometry;
  annotate_info.pen=ForegroundColor;
  (void) XQueryColorDatabase(BackgroundColor,&color);
  display=XOpenDisplay(image_info->server_name);
  if (display != (Display *) NULL)
    {
      XrmDatabase
        resource_database;

      /*
        Set background and pen color.
      */
      XSetErrorHandler(XError);
      resource_database=XGetResourceDatabase(display,client_name);
      XGetResourceInfo(resource_database,ClientName,&resource_info);
      (void) XQueryColorDatabase(resource_info.background_color,&color);
      annotate_info.pen=resource_info.foreground_color;
      XCloseDisplay(display);
    }
  /*
    Initialize text image to background color.
  */
  background_color.red=XDownScale(color.red);
  background_color.green=XDownScale(color.green);
  background_color.blue=XDownScale(color.blue);
  background_color.index=0;
  p=image->pixels;
  for (i=0; i < image->packets; i++)
    *p++=background_color;
  if (image_info->texture != (char *) NULL)
    TextureImage(image,image_info->texture);
  /*
    Annotate the text image.
  */
  offset=0;
  for ( ; ; )
  {
    /*
      Annotate image with text.
    */
    ProgressMonitor(LoadImageText,ftell(image->file),image->filesize);
    text_status=fgets(text,MaxTextLength-1,image->file);
    if (text_status == (char *) NULL)
      break;
    if ((int) strlen(annotate_info.text) > 0)
      annotate_info.text[strlen(annotate_info.text)-1]='\0';
    (void) sprintf(annotate_info.geometry,"+%d%+d",x,y+offset);
    AnnotateImage(image,&annotate_info);
    offset+=annotate_info.pointsize;
    if (((y << 1)+offset+annotate_info.pointsize) < image->rows)
      continue;
    /*
      Page is full-- allocate next image structure.
    */
    CompressImage(image);
    if (image_info->texture == (char *) NULL)
      QuantizeImage(image,2,8,False,RGBColorspace);
    image->orphan=True;
    image->next=CopyImage(image,image->columns,image->rows,False);
    image->orphan=False;
    if (image->next == (Image *) NULL)
      {
        Warning("Unable to annotate image","Memory allocation error");
        break;
      }
    (void) strcpy(image->next->filename,image_info->filename);
    image->next->file=image->file;
    image->next->filesize=image->filesize;
    image->next->scene=image->scene+1;
    image->next->previous=image;
    image=image->next;
    /*
      Initialize text image to background color.
    */
    p=image->pixels;
    for (i=0; i < image->packets; i++)
      *p++=background_color;
    if (image_info->texture != (char *) NULL)
      TextureImage(image,image_info->texture);
    offset=0;
  }
  CompressImage(image);
  if (image_info->texture == (char *) NULL)
    QuantizeImage(image,2,8,False,RGBColorspace);
  while (image->previous != (Image *) NULL)
    image=image->previous;
  CloseImage(image);
  return(image);
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
#include "l/l_sys_def.h"
/* End Kobus */


/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%  R e a d T I F F I m a g e                                                  %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Function ReadTIFFImage reads a Tagged image file and returns it.  It
%  allocates the memory necessary for the new Image structure and returns a
%  pointer to the new image.
%
%  The format of the ReadTIFFImage routine is:
%
%      image=ReadTIFFImage(image_info)
%
%  A description of each parameter follows:
%
%    o image:  Function ReadTIFFImage returns a pointer to the image after
%      reading.  A null image is returned if there is a a memory shortage or
%      if the image cannot be read.
%
%    o image_info: Specifies a pointer to an ImageInfo structure.
%
%
*/

static void TIFFWarningMessage(const char *module, const char *format, va_list warning)
{
  char
    message[MaxTextLength];

  register char
    *p;

  p=message;
  if (module != (char *) NULL)
    {
      (void) sprintf(p,"%s: ",module);
      p+=strlen(message);
    }
  vsprintf(p,format,warning);
  strcat(p,".");
  Warning(message,(char *) NULL);
}

static Image *ReadTIFFImage(ImageInfo *image_info)
{
  char
    *comment;

  Image
    *image;

  int
    range;

  Quantum
    blue,
    green,
    red;

  register int
    i,
    x,
    y;

  register RunlengthPacket
    *q;

  register unsigned char
    *p;

  TIFF
    *tiff;

  uint16
    extra_samples,
    *sample_info;

  unsigned char
    *scanline;

  unsigned int
    height,
    method,
    packets,
    status,
    width;

  unsigned short
    bits_per_sample,
    index,
    max_sample_value,
    min_sample_value,
    photometric,
    samples_per_pixel,
    value;

  /*
    Allocate image structure.
  */
  image=AllocateImage(image_info);
  if (image == (Image *) NULL)
    return((Image *) NULL);
  /*
    Open image.
  */
  OpenImage(image,ReadBinaryType);
  if (image->file == (FILE *) NULL)
    PrematureExit("Unable to open file",image);
  if ((image->file == stdin) || image->pipe)
    {
      FILE
        *file;

      int
        c;

      /*
        Copy standard input or pipe to temporary file.
      */
      TemporaryFilename(image_info->filename);
      file=fopen(image_info->filename,WriteBinaryType);
      if (file == (FILE *) NULL)
        PrematureExit("Unable to write file",image);
      c=fgetc(image->file);
      while (c != EOF)
      {
        (void) putc(c,file);
        c=fgetc(image->file);
      }
      (void) fclose(file);
      (void) strcpy(image->filename,image_info->filename);
      image->temporary=True;
    }
  CloseImage(image);
  TIFFSetErrorHandler(TIFFWarningMessage);

  /*
  // Kobus
  //
  // TIFFSetWarningHandler(TIFFWarningMessage);
  //
  */
  TIFFSetWarningHandler(NULL);

  tiff=TIFFOpen(image->filename,ReadBinaryType);
  if (tiff == (TIFF *) NULL)
    PrematureExit("Unable to open file",image);
  if (image_info->subimage != 0)
    while (image->scene < image_info->subimage)
    {
       image->scene++;
       status=TIFFReadDirectory(tiff);
       if (status == False)
         PrematureExit("Unable to read subimage",image);
    }
  do
  {
    if (image_info->verbose)
      TIFFPrintDirectory(tiff,stderr,False);
    TIFFGetField(tiff,TIFFTAG_IMAGEWIDTH,&width);
    TIFFGetField(tiff,TIFFTAG_IMAGELENGTH,&height);
    TIFFGetFieldDefaulted(tiff,TIFFTAG_BITSPERSAMPLE,&bits_per_sample);
    TIFFGetFieldDefaulted(tiff,TIFFTAG_MINSAMPLEVALUE,&min_sample_value);
    TIFFGetFieldDefaulted(tiff,TIFFTAG_MAXSAMPLEVALUE,&max_sample_value);
    TIFFGetFieldDefaulted(tiff,TIFFTAG_PHOTOMETRIC,&photometric);
    TIFFGetFieldDefaulted(tiff,TIFFTAG_SAMPLESPERPIXEL,&samples_per_pixel);
    TIFFGetFieldDefaulted(tiff,TIFFTAG_RESOLUTIONUNIT,&image->units);
    TIFFGetFieldDefaulted(tiff,TIFFTAG_XRESOLUTION,&image->x_resolution);
    TIFFGetFieldDefaulted(tiff,TIFFTAG_YRESOLUTION,&image->y_resolution);
    /*
      Allocate memory for the image and pixel buffer.
    */
    image->columns=width;
    image->rows=height;
    image->depth=QuantumDepth;
    if (bits_per_sample <= 8)
      image->depth=8;
    image->packets=0;
    packets=Max((image->columns*image->rows+4) >> 3,1);
    if (bits_per_sample == 1)
      packets=Max((image->columns*image->rows+8) >> 4,1);
    image->pixels=(RunlengthPacket *) malloc(packets*sizeof(RunlengthPacket));
    if (image->pixels == (RunlengthPacket *) NULL)
      {
        TIFFClose(tiff);
        PrematureExit("Unable to allocate memory",image);
      }
    comment=(char *) NULL;
    TIFFGetField(tiff,TIFFTAG_IMAGEDESCRIPTION,&comment);
    if (comment != (char *) NULL)
      if ((int) strlen(comment) > 4)
        {
          image->comments=(char *)
            malloc((unsigned int) (strlen(comment)+1)*sizeof(char));
          if (image->comments == (char *) NULL)
            {
              TIFFClose(tiff);
              PrematureExit("Unable to allocate memory",image);
            }
          (void) strcpy(image->comments,comment);
        }
    range=max_sample_value-min_sample_value;
    if (range < 0)
      range=max_sample_value;
    q=image->pixels;
    q->length=MaxRunlength;
    method=0;
    if ((samples_per_pixel > 1) || TIFFIsTiled(tiff))
      {
        method=2;
        if ((samples_per_pixel >= 3) && (photometric == PHOTOMETRIC_RGB))
          method=1;
      }
    switch (method)
    {
      case 0:
      {
        Quantum
          *quantum_scanline;

        register Quantum
          *r;

        /*
          Convert TIFF image to PseudoClass MIFF image.
        */
        image->class=PseudoClass;
        image->colors=range+1;
        image->colormap=(ColorPacket *)
          malloc(image->colors*sizeof(ColorPacket));
        quantum_scanline=(Quantum *) malloc(width*sizeof(Quantum));
        scanline=(unsigned char *) malloc(TIFFScanlineSize(tiff)+1);
        if ((image->colormap == (ColorPacket *) NULL) ||
            (quantum_scanline == (Quantum *) NULL) ||
            (scanline == (unsigned char *) NULL))
          {
            TIFFClose(tiff);
            PrematureExit("Unable to allocate memory",image);
          }
        /*
          Create colormap.
        */
        switch (photometric)
        {
          case PHOTOMETRIC_MINISBLACK:
          {
            for (i=0; i < image->colors; i++)
            {
              image->colormap[i].red=(MaxRGB*i)/range;
              image->colormap[i].green=(MaxRGB*i)/range;
              image->colormap[i].blue=(MaxRGB*i)/range;
            }
            break;
          }
          case PHOTOMETRIC_MINISWHITE:
          {
            for (i=0; i < image->colors; i++)
            {
              image->colormap[i].red=((range-i)*MaxRGB)/range;
              image->colormap[i].green=((range-i)*MaxRGB)/range;
              image->colormap[i].blue=((range-i)*MaxRGB)/range;
            }
            break;
          }
          case PHOTOMETRIC_PALETTE:
          {
            unsigned short
              *blue_colormap,
              *green_colormap,
              *red_colormap;

            TIFFGetField(tiff,TIFFTAG_COLORMAP,&red_colormap,&green_colormap,
              &blue_colormap);
            for (i=0; i < image->colors; i++)
            {
              image->colormap[i].red=(unsigned int)
                (red_colormap[i]*MaxRGB)/65535L;
              image->colormap[i].green=(unsigned int)
                (green_colormap[i]*MaxRGB)/65535L;
              image->colormap[i].blue=(unsigned int)
                (blue_colormap[i]*MaxRGB)/65535L;
            }
            break;
          }
          default:
            break;
        }
        /*
          Convert image to PseudoClass runlength-encoded packets.
        */
        for (y=0; y < image->rows; y++)
        {
          TIFFReadScanline(tiff,(char *) scanline,y,0);
          p=scanline;
          r=quantum_scanline;
          switch (bits_per_sample)
          {
            case 1:
            {
              register int
                bit;

              for (x=0; x < ((int) width-7); x+=8)
              {
                for (bit=7; bit >= 0; bit--)
                  *r++=((*p) & (0x01 << bit) ? 0x01 : 0x00);
                p++;
              }
              if ((width % 8) != 0)
                {
                  for (bit=7; bit >= (8-(width % 8)); bit--)
                    *r++=((*p) & (0x01 << bit) ? 0x01 : 0x00);
                  p++;
                }
              break;
            }
            case 2:
            {
              for (x=0; x < ((int) width-3); x+=4)
              {
                *r++=(*p >> 6) & 0x3;
                *r++=(*p >> 4) & 0x3;
                *r++=(*p >> 2) & 0x3;
                *r++=(*p) & 0x3;
                p++;
              }
              if ((width % 4) != 0)
                {
                  for (i=3; i >= (4-(width % 4)); i--)
                    *r++=(*p >> (i*2)) & 0x03;
                  p++;
                }
              break;
            }
            case 4:
            {
              for (x=0; x < ((int) width-1); x+=2)
              {
                *r++=(*p >> 4) & 0xf;
                *r++=(*p) & 0xf;
                p++;
              }
              if ((width % 2) != 0)
                *r++=(*p++ >> 4) & 0xf;
              break;
            }
            case 8:
            {
              for (x=0; x < width; x++)
                *r++=(*p++);
              break;
            }
            case 16:
            {
              for (x=0; x < image->columns; x++)
              {
                ReadQuantum(*r,p);
                r++;
              }
              break;
            }
            default:
              break;
          }
          /*
            Transfer image scanline.
          */
          r=quantum_scanline;
          for (x=0; x < image->columns; x++)
          {
            index=(*r++);
            if ((index == q->index) && ((int) q->length < MaxRunlength))
              q->length++;
            else
              {
                if (image->packets != 0)
                  q++;
                image->packets++;
                if (image->packets == packets)
                  {
                    packets<<=1;
                    image->pixels=(RunlengthPacket *) realloc((char *)
                      image->pixels,packets*sizeof(RunlengthPacket));
                    if (image->pixels == (RunlengthPacket *) NULL)
                      {
                        free((char *) scanline);
                        free((char *) quantum_scanline);
                        TIFFClose(tiff);
                        PrematureExit("Unable to allocate memory",image);
                      }
                    q=image->pixels+image->packets-1;
                  }
                q->index=index;
                q->length=0;
              }
          }
          ProgressMonitor(LoadImageText,y,image->rows);
        }
        free((char *) scanline);
        free((char *) quantum_scanline);
        if (image->class == PseudoClass)
          {
            SyncImage(image);
            CompressColormap(image);
          }
        break;
      }
      case 1:
      {
        /*
          Convert TIFF image to DirectClass MIFF image.
        */
        scanline=(unsigned char *) malloc((TIFFScanlineSize(tiff) << 1)+1);
        if (scanline == (unsigned char *) NULL)
          {
            TIFFClose(tiff);
            PrematureExit("Unable to allocate memory",image);
          }
        TIFFGetFieldDefaulted(tiff,TIFFTAG_EXTRASAMPLES,&extra_samples,
          &sample_info);
        image->matte=
          ((extra_samples == 1) && (sample_info[0] == EXTRASAMPLE_ASSOCALPHA));
        for (y=0; y < image->rows; y++)
        {
          TIFFReadScanline(tiff,(char *) scanline,y,0);
          if (bits_per_sample == 4)
            {
              register unsigned char
                *r;

              width=TIFFScanlineSize(tiff);
              p=scanline+width-1;
              r=scanline+(width << 1)-1;
              for (x=0; x < (int) width; x++)
              {
                *r--=((*p) & 0xf) << 4;
                *r--=((*p >> 4) & 0xf) << 4;
                p--;
              }
            }
          p=scanline;
          for (x=0; x < image->columns; x++)
          {
            ReadQuantum(red,p);
            ReadQuantum(green,p);
            ReadQuantum(blue,p);
            index=0;
            if (image->matte)
              ReadQuantum(index,p);
            if ((red == q->red) && (green == q->green) && (blue == q->blue) &&
                (index == q->index) && ((int) q->length < MaxRunlength))
              q->length++;
            else
              {
                if (image->packets != 0)
                  q++;
                image->packets++;
                if (image->packets == packets)
                  {
                    packets<<=1;
                    image->pixels=(RunlengthPacket *) realloc((char *)
                      image->pixels,packets*sizeof(RunlengthPacket));
                    if (image->pixels == (RunlengthPacket *) NULL)
                      {
                        TIFFClose(tiff);
                        free((char *) scanline);
                        PrematureExit("Unable to allocate memory",image);
                      }
                    q=image->pixels+image->packets-1;
                  }
                q->red=red;
                q->green=green;
                q->blue=blue;
                q->index=index;
                q->length=0;
              }
          }
          ProgressMonitor(LoadImageText,y,image->rows);
        }
        free((char *) scanline);
        break;
      }
      case 2:
      default:
      {
        register uint32
          *p,
          *pixels;

        /*
          Convert TIFF image to DirectClass MIFF image.
        */
        TIFFGetFieldDefaulted(tiff,TIFFTAG_EXTRASAMPLES,&extra_samples,
          &sample_info);
        image->matte=
          ((extra_samples == 1) && (sample_info[0] == EXTRASAMPLE_ASSOCALPHA));
        pixels=(uint32 *)
          malloc((image->columns*image->rows+image->columns)*sizeof(uint32));
        if (pixels == (uint32 *) NULL)
          {
            TIFFClose(tiff);
            PrematureExit("Unable to allocate memory",image);
          }
        status=TIFFReadRGBAImage(tiff,image->columns,image->rows,pixels,0);
        if (status == False)
          {
            free((char *) pixels);
            TIFFClose(tiff);
            PrematureExit("Unable to read image",image);
          }
        /*
          Convert image to DirectClass runlength-encoded packets.
        */
        for (y=image->rows-1; y >= 0; y--)
        {
          p=pixels+y*image->columns;
          for (x=0; x < image->columns; x++)
          {
            red=UpScale(TIFFGetR(*p));
            green=UpScale(TIFFGetG(*p));
            blue=UpScale(TIFFGetB(*p));
            index=(unsigned short) (image->matte ? TIFFGetA(*p) : 0);
            if ((red == q->red) && (green == q->green) && (blue == q->blue) &&
                (index == q->index) && ((int) q->length < MaxRunlength))
              q->length++;
            else
              {
                if (image->packets != 0)
                  q++;
                image->packets++;
                if (image->packets == packets)
                  {
                    packets<<=1;
                    image->pixels=(RunlengthPacket *) realloc((char *)
                      image->pixels,packets*sizeof(RunlengthPacket));
                    if (image->pixels == (RunlengthPacket *) NULL)
                      {
                        free((char *) pixels);
                        TIFFClose(tiff);
                        PrematureExit("Unable to allocate memory",image);
                      }
                    q=image->pixels+image->packets-1;
                  }
                q->red=red;
                q->green=green;
                q->blue=blue;
                q->index=index;
                q->length=0;
              }
            p++;
          }
          ProgressMonitor(LoadImageText,image->rows-y,image->rows);
        }
        free((char *) pixels);
        if (IsPseudoClass(image))
          QuantizeImage(image,(unsigned int) range,8,False,RGBColorspace);
        break;
      }
    }
    image->pixels=(RunlengthPacket *)
      realloc((char *) image->pixels,image->packets*sizeof(RunlengthPacket));
    /*
      Proceed to next image.
    */
    if (image_info->subimage != 0)
      if (image->scene >= (image_info->subimage+image_info->subrange-1))
        break;
    status=TIFFReadDirectory(tiff);
    if (status == True)
      {
        /*
          Allocate image structure.
        */
        image->next=AllocateImage(image_info);
        if (image->next == (Image *) NULL)
          {
            DestroyImages(image);
            return((Image *) NULL);
          }
        (void) strcpy(image->next->filename,image_info->filename);
        image->next->file=image->file;
        image->next->scene=image->scene+1;
        image->next->previous=image;
        image=image->next;
      }
  } while (status == True);
  TIFFClose(tiff);
  if (image->temporary)
    (void) unlink(image_info->filename);
  while (image->previous != (Image *) NULL)
    image=image->previous;
  return(image);
}
#else

static Image *ReadTIFFImage(ImageInfo* image_info)
{
  Warning("TIFF library is not available",image_info->filename);
  return(ReadMIFFImage(image_info));
}
#endif

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%  R e a d T I L E I m a g e                                                  %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Function ReadTILEImage tiles a texture on an image.  It allocates the
%  memory necessary for the new Image structure and returns a pointer to the
%  new image.
%
%  The format of the ReadTILEImage routine is:
%
%      image=ReadTILEImage(image_info)
%
%  A description of each parameter follows:
%
%    o image:  Function ReadTILEImage returns a pointer to the image after
%      reading.  A null image is returned if there is a a memory shortage or
%      if the image cannot be read.
%
%    o image_info: Specifies a pointer to an ImageInfo structure.
%
%
*/
static Image *ReadTILEImage(ImageInfo *image_info)
{
  Image
    *image,
    *tiled_image;

  int
    x,
    y;

  unsigned int
    height,
    width;

  tiled_image=ReadImage(image_info);
  if (tiled_image == (Image *) NULL)
    return((Image *) NULL);
  /*
    Allocate image structure.
  */
  image=AllocateImage(image_info);
  if (image == (Image *) NULL)
    return((Image *) NULL);
  /*
    Determine width and height, e.g. 640x512.
  */
  width=512;
  height=512;
  if (image_info->size != (char *) NULL)
    (void) XParseGeometry(image_info->size,&x,&y,&width,&height);
  /*
    Initialize Image structure.
  */
  (void) strcpy(image->filename,image_info->filename);
  image->columns=width;
  image->rows=height;
  image->packets=image->columns*image->rows;
  image->pixels=(RunlengthPacket *)
    malloc(image->packets*sizeof(RunlengthPacket));
  if (image->pixels == (RunlengthPacket *) NULL)
    PrematureExit("Unable to allocate memory",image);
  /*
    Tile texture onto image.
  */
  ProgressMonitor(LoadImageText,200,400);
  for (y=0; y < image->rows; y+=tiled_image->rows)
    for (x=0; x < image->columns; x+=tiled_image->columns)
      CompositeImage(image,ReplaceCompositeOp,tiled_image,x,y);
  ProgressMonitor(LoadImageText,400,400);
  return(image);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%  R e a d U Y V Y I m a g e                                                  %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Function ReadUYVYImage reads an image in the UYVY (16bit/pixel) format
%  and returns it.  It allocates the memory necessary for the new Image
%  structure and returns a pointer to the new image.
%
%  The format of the ReadYUVImage routine is:
%
%      image=ReadUYVYImage(image_info)
%
%  A description of each parameter follows:
%
%    o image:  Function ReadYUVImage returns a pointer to the image after
%      reading.  A null image is returned if there is a a memory shortage or
%      if the image cannot be read.
%
%    o image_info: Specifies a pointer to an ImageInfo structure.
%
%
*/
static Image *ReadUYVYImage(ImageInfo *image_info)
{
  Image
    *image;

  int
    x,
    y;

  register int
    i;

  register RunlengthPacket
    *q;

  register unsigned char
    *p;

  unsigned char
    *uyvy_pixels;

  unsigned int
    height,
    width;

  /*
    Allocate image structure.
  */
  image=AllocateImage(image_info);
  if (image == (Image *) NULL)
    return((Image *) NULL);
  /*
    Open image file.
  */
  (void) strcpy(image->filename,image_info->filename);
  OpenImage(image,ReadBinaryType);
  if (image->file == (FILE *) NULL)
    PrematureExit("Unable to open file",image)
  /*
    Determine width and height, e.g. 640x512.
  */
  width=512;
  height=512;
  x=0;
  if (image_info->size != (char *) NULL)
    (void) XParseGeometry(image_info->size,&x,&y,&width,&height);
  for (i=0; i < x; i++)
    (void) fgetc(image->file);
  /*
    Read data.
  */
  image->columns=width;
  image->rows=height;
  image->packets=image->columns*image->rows;
  uyvy_pixels=(unsigned char *)
    malloc((2*width*height)*sizeof(unsigned char));
  image->pixels=(RunlengthPacket *)
    malloc(image->packets*sizeof(RunlengthPacket));
  if ((uyvy_pixels == (unsigned char *) NULL) ||
      (image->pixels == (RunlengthPacket *) NULL))
    PrematureExit("Unable to allocate memory",image);
  (void) ReadData((char *) uyvy_pixels,1,2*width*height,image->file);
  /*
    Accumulate UYVY, then upack into two pixels.
  */
  p=uyvy_pixels;
  q=image->pixels;
  for (i=0; i < (image->packets >> 1); i++)
  {
    q->red=UpScale(p[1]);
    q->green=UpScale(p[0]);
    q->blue=UpScale(p[2]);
    q->index=0;
    q->length=0;
    q++;
    q->red=UpScale(p[3]);
    q->green=UpScale(p[0]);
    q->blue=UpScale(p[2]);
    q->index=0;
    q->length=0;
    q++;
    p+=4;
    if (QuantumTick(i,image))
      ProgressMonitor(LoadImageText,i,image->packets >> 1);
  }
  free((char *) uyvy_pixels);
  TransformRGBImage(image,YCbCrColorspace);
  CompressImage(image);
  return(image);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   R e a d V I C A R I m a g e                                               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Function ReadVICARImage reads a VICAR image file and returns it.  It
%  allocates the memory necessary for the new Image structure and returns a
%  pointer to the new image.
%
%  The format of the ReadVICARImage routine is:
%
%      image=ReadVICARImage(image_info)
%
%  A description of each parameter follows:
%
%    o image: Function ReadVICARImage returns a pointer to the image after
%      reading.  A null image is returned if there is a a memory shortage or if
%      the image cannot be read.
%
%    o filename: Specifies the name of the image to read.
%
%
*/
static Image *ReadVICARImage(ImageInfo *image_info)
{
  char
    keyword[MaxTextLength],
    value[MaxTextLength];

  Image
    *image;

  int
    c,
    y;

  long
    count;

  register int
    i,
    x;

  register RunlengthPacket
    *q;

  register unsigned char
    *p;

  unsigned char
    *vicar_pixels;

  unsigned int
    header_length,
    status,
    value_expected;

  /*
    Allocate image structure.
  */
  image=AllocateImage(image_info);
  if (image == (Image *) NULL)
    return((Image *) NULL);
  /*
    Open image file.
  */
  OpenImage(image,ReadBinaryType);
  if (image->file == (FILE *) NULL)
    PrematureExit("Unable to open file",image);
  /*
    Decode image header.
  */
  c=fgetc(image->file);
  count=1;
  if (c == EOF)
    {
      DestroyImage(image);
      return((Image *) NULL);
    }
  header_length=0;
  while (isgraph(c) && ((image->columns*image->rows) == 0))
  {
    if (!isalnum(c))
      {
        c=fgetc(image->file);
        count++;
      }
    else
      {
        register char
          *p;

        /*
          Determine a keyword and its value.
        */
        p=keyword;
        do
        {
          if ((p-keyword) < (MaxTextLength-1))
            *p++=(char) c;
          c=fgetc(image->file);
          count++;
        } while (isalnum(c) || (c == '_'));
        *p='\0';
        value_expected=False;
        while (isspace(c) || (c == '='))
        {
          if (c == '=')
            value_expected=True;
          c=fgetc(image->file);
          count++;
        }
        if (value_expected == False)
          continue;
        p=value;
        while (isalnum(c))
        {
          if ((p-value) < (MaxTextLength-1))
            *p++=(char) c;
          c=fgetc(image->file);
          count++;
        }
        *p='\0';
        /*
          Assign a value to the specified keyword.
        */
        if (strcmp(keyword,"LABEL_RECORDS") == 0)
          header_length=(unsigned int) atoi(value);
        if (strcmp(keyword,"LBLSIZE") == 0)
          header_length=(unsigned int) atoi(value);
        if (strcmp(keyword,"RECORD_BYTES") == 0)
          image->columns=(unsigned int) atoi(value);
        if (strcmp(keyword,"NS") == 0)
          image->columns=(unsigned int) atoi(value);
        if (strcmp(keyword,"LINES") == 0)
          image->rows=(unsigned int) atoi(value);
        if (strcmp(keyword,"NL") == 0)
          image->rows=(unsigned int) atoi(value);
      }
    while (isspace(c))
    {
      c=fgetc(image->file);
      count++;
    }
  }
  /*
    Read the rest of the header.
  */
  while (count < header_length)
  {
    c=fgetc(image->file);
    count++;
  }
  /*
    Verify that required image information is defined.
  */
  if ((image->columns*image->rows) == 0)
    PrematureExit("image size is zero",image);
  /*
    Create linear colormap.
  */
  image->class=PseudoClass;
  image->colors=256;
  image->colormap=(ColorPacket *) malloc(image->colors*sizeof(ColorPacket));
  if (image->colormap == (ColorPacket *) NULL)
    PrematureExit("Unable to allocate memory",image);
  for (i=0; i < image->colors; i++)
  {
    image->colormap[i].red=(Quantum) UpScale(i);
    image->colormap[i].green=(Quantum) UpScale(i);
    image->colormap[i].blue=(Quantum) UpScale(i);
  }
  /*
    Initialize image structure.
  */
  image->packets=image->columns*image->rows;
  image->pixels=(RunlengthPacket *)
    malloc(image->packets*sizeof(RunlengthPacket));
  vicar_pixels=(unsigned char *) malloc(image->packets*sizeof(unsigned char));
  if ((image->pixels == (RunlengthPacket *) NULL) ||
      (vicar_pixels == (unsigned char *) NULL))
    PrematureExit("Unable to read image data",image);
  /*
    Convert VICAR pixels to runlength-encoded packets.
  */
  status=ReadData((char *) vicar_pixels,1,image->packets,image->file);
  if (status == False)
    PrematureExit("Insufficient image data in file",image);
  /*
    Convert VICAR pixels to runlength-encoded packets.
  */
  p=vicar_pixels;
  q=image->pixels;
  for (y=0; y < image->rows; y++)
  {
    for (x=0; x < image->columns; x++)
    {
      q->index=(unsigned short) *p;
      q->length=0;
      p++;
      q++;
    }
    ProgressMonitor(LoadImageText,y,image->rows);
  }
  free((char *) vicar_pixels);
  SyncImage(image);
  CompressColormap(image);
  CompressImage(image);
  CloseImage(image);
  return(image);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%  R e a d V I D I m a g e                                                    %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Function ReadVIDImage reads one of more images and creates a Visual Image
%  Directory file.  It allocates the memory necessary for the new Image
%  structure and returns a pointer to the new image.
%
%  The format of the ReadVIDImage routine is:
%
%      image=ReadVIDImage(image_info)
%
%  A description of each parameter follows:
%
%    o image:  Function ReadVIDImage returns a pointer to the image after
%      reading.  A null image is returned if there is a a memory shortage or
%      if the image cannot be read.
%
%    o image_info: Specifies a pointer to an ImageInfo structure.
%
%
*/
static Image *ReadVIDImage(ImageInfo *image_info)
{
#define ClientName  "montage"

  char
    **filelist,
    **list,
    *resource_value;

  Display
    *display;

  Image
    *image,
    **images;

  ImageInfo
    local_info;

  int
    number_files;

  register int
    i,
    j;

  XMontageInfo
    montage_info;

  XResourceInfo
    resource_info;

  XrmDatabase
    resource_database;

  /*
    Expand the filename.
  */
  list=(char **) malloc(sizeof(char *));
  if (list == (char **) NULL)
    {
      Warning("Memory allocation error",(char *) NULL);
      return((Image *) NULL);
    }
  list[0]=(char *) malloc(strlen(image_info->filename)+1);
  if (list[0] == (char *) NULL)
    {
      Warning("Memory allocation error",(char *) NULL);
      return((Image *) NULL);
    }
  (void) strcpy(list[0],image_info->filename);
  number_files=1;
  filelist=list;
  ExpandFilenames(&number_files,&filelist);
  if (number_files == 0)
    {
      Warning("VID translation failed",image_info->filename);
      return((Image *) NULL);
    }
  /*
    Allocate images array.
  */
  images=(Image **) malloc(number_files*sizeof(Image *));
  if (images == (Image **) NULL)
    {
      Warning("Memory allocation error",(char *) NULL);
      for (i=0; i < number_files; i++)
        free((char *) filelist[i]);
      free((char *) filelist);
      return((Image *) NULL);
    }
  /*
    Open X server connection.
  */
  resource_info.background_color=DefaultTileBackground;
  resource_info.border_color=DefaultTileBorder;
  resource_info.border_width=atoi(DefaultTileBorderwidth);
  resource_info.foreground_color=DefaultTileForeground;
  resource_info.gravity=CenterGravity;
  resource_info.image_geometry=DefaultTileGeometry;
  resource_info.matte_color=DefaultTileMatte;
  resource_info.title=(char *) NULL;
  display=XOpenDisplay(image_info->server_name);
  if (display != (Display *) NULL)
    {
      /*
        Set our forgiving error handler.
      */
      XSetErrorHandler(XError);
      /*
        Get user defaults from X resource database.
      */
      resource_database=XGetResourceDatabase(display,client_name);
      XGetResourceInfo(resource_database,ClientName,&resource_info);
      resource_info.background_color=XGetResourceInstance(resource_database,
        ClientName,"background",DefaultTileBackground);
      resource_value=XGetResourceClass(resource_database,ClientName,
        "borderWidth",DefaultTileBorderwidth);
      resource_info.border_width=atoi(resource_value);
      resource_info.font=image_info->font;
      resource_info.foreground_color=XGetResourceInstance(resource_database,
        ClientName,"foreground",DefaultTileForeground);
      resource_info.image_geometry=XGetResourceInstance(resource_database,
        ClientName,"imageGeometry",DefaultTileGeometry);
      resource_info.matte_color=XGetResourceInstance(resource_database,
        ClientName,"mattecolor",DefaultTileMatte);
      XCloseDisplay(display);
    }
  /*
    Read each image and convert them to a tile.
  */
  j=0;
  for (i=0; i < number_files; i++)
  {
    local_info=(*image_info);
    if (local_info.size == (char *) NULL)
      local_info.size=resource_info.image_geometry;
    local_info.filename=filelist[i];
    *local_info.magick='\0';
    image=ReadImage(&local_info);
    free((char *) filelist[i]);
    if (image == (Image *) NULL)
      continue;
    image->scene=j;
    LabelImage(image,"%f");
    TransformImage(&image,(char *) NULL,resource_info.image_geometry);
    if (image_info->verbose)
      DescribeImage(image,stderr,False);
    images[j]=image;
    j++;
    ProgressMonitor(LoadImageText,i,number_files);
  }
  free((char *) filelist);
  /*
    Create the visual image directory.
  */
  XGetMontageInfo(&montage_info);
  montage_info.number_tiles=j;
  image=(Image *) NULL;
  if (montage_info.number_tiles != 0)
    image=
      XMontageImage(images,&resource_info,&montage_info,image_info->filename);
  free((char *) images);
  if (image == (Image *) NULL)
    {
      Warning("VID translation failed",image_info->filename);
      return((Image *) NULL);
    }
  free((char *) list[0]);
  free((char *) list);
  return(image);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   R e a d V I F F I m a g e                                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Function ReadVIFFImage reads a Khoros Visualization image file and returns
%  it.  It allocates the memory necessary for the new Image structure and
%  returns a pointer to the new image.
%
%  The format of the ReadVIFFImage routine is:
%
%      image=ReadVIFFImage(image_info)
%
%  A description of each parameter follows:
%
%    o image: Function ReadVIFFImage returns a pointer to the image after
%      reading.  A null image is returned if there is a a memory shortage or if
%      the image cannot be read.
%
%    o filename: Specifies the name of the image to read.
%
%
*/
static Image *ReadVIFFImage(ImageInfo *image_info)
{
#define VFF_CM_genericRGB  15
#define VFF_CM_ntscRGB  1
#define VFF_CM_NONE  0
#define VFF_DEP_DECORDER  0x4
#define VFF_DEP_NSORDER  0x8
#define VFF_DES_RAW  0
#define VFF_LOC_IMPLICIT  1
#define VFF_MAPTYP_NONE  0
#define VFF_MAPTYP_1_BYTE  1
#define VFF_MS_NONE  0
#define VFF_MS_ONEPERBAND  1
#define VFF_MS_SHARED  3
#define VFF_TYP_BIT  0
#define VFF_TYP_1_BYTE  1
#define VFF_TYP_2_BYTE  2
#define VFF_TYP_4_BYTE  4

  typedef struct _ViffHeader
  {
    unsigned char
      identifier,
      file_type,
      release,
      version,
      machine_dependency,
      reserve[3];

    char
      comment[512];

    unsigned long
      rows,
      columns,
      subrows;

    long
      x_offset,
      y_offset;

    float
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

  Image
    *image;

  register int
    bit,
    i,
    x,
    y;

  register Quantum
    *p;

  register RunlengthPacket
    *q;

  unsigned char
    buffer[7],
    *viff_pixels;

  unsigned int
    bytes_per_pixel,
    status;

  unsigned long
    packets;

  ViffHeader
    viff_header;

  /*
    Allocate image structure.
  */
  image=AllocateImage(image_info);
  if (image == (Image *) NULL)
    return((Image *) NULL);
  /*
    Open image file.
  */
  OpenImage(image,ReadBinaryType);
  if (image->file == (FILE *) NULL)
    PrematureExit("Unable to open file",image);
  /*
    Read VIFF header (1024 bytes).
  */
  status=ReadData((char *) &viff_header.identifier,1,1,image->file);
  do
  {
    /*
      Verify VIFF identifier.
    */
    if ((status == False) || ((unsigned char) viff_header.identifier != 0xab))
      PrematureExit("Not a VIFF raster",image);
    /*
      Initialize VIFF image.
    */
    (void) ReadData((char *) buffer,1,7,image->file);
    viff_header.file_type=buffer[0];
    viff_header.release=buffer[1];
    viff_header.version=buffer[2];
    viff_header.machine_dependency=buffer[3];
    (void) ReadData((char *) viff_header.comment,1,512,image->file);
    viff_header.comment[511]='\0';
    if ((int) strlen(viff_header.comment) > 4)
      {
        image->comments=(char *)
          malloc((unsigned int) (strlen(viff_header.comment)+1)*sizeof(char));
        if (image->comments == (char *) NULL)
          PrematureExit("Unable to allocate memory",image);
        (void) strcpy(image->comments,viff_header.comment);
      }
    if ((viff_header.machine_dependency == VFF_DEP_DECORDER) ||
        (viff_header.machine_dependency == VFF_DEP_NSORDER))
      {
        viff_header.rows=LSBFirstReadLong(image->file);
        viff_header.columns=LSBFirstReadLong(image->file);
        viff_header.subrows=LSBFirstReadLong(image->file);
        viff_header.x_offset=LSBFirstReadLong(image->file);
        viff_header.y_offset=LSBFirstReadLong(image->file);
        viff_header.x_pixel_size=(float) LSBFirstReadLong(image->file);
        viff_header.y_pixel_size=(float) LSBFirstReadLong(image->file);
        viff_header.location_type=LSBFirstReadLong(image->file);
        viff_header.location_dimension=LSBFirstReadLong(image->file);
        viff_header.number_of_images=LSBFirstReadLong(image->file);
        viff_header.number_data_bands=LSBFirstReadLong(image->file);
        viff_header.data_storage_type=LSBFirstReadLong(image->file);
        viff_header.data_encode_scheme=LSBFirstReadLong(image->file);
        viff_header.map_scheme=LSBFirstReadLong(image->file);
        viff_header.map_storage_type=LSBFirstReadLong(image->file);
        viff_header.map_rows=LSBFirstReadLong(image->file);
        viff_header.map_columns=LSBFirstReadLong(image->file);
        viff_header.map_subrows=LSBFirstReadLong(image->file);
        viff_header.map_enable=LSBFirstReadLong(image->file);
        viff_header.maps_per_cycle=LSBFirstReadLong(image->file);
        viff_header.color_space_model=LSBFirstReadLong(image->file);
      }
    else
      {
        viff_header.rows=MSBFirstReadLong(image->file);
        viff_header.columns=MSBFirstReadLong(image->file);
        viff_header.subrows=MSBFirstReadLong(image->file);
        viff_header.x_offset=MSBFirstReadLong(image->file);
        viff_header.y_offset=MSBFirstReadLong(image->file);
        viff_header.x_pixel_size=(float) MSBFirstReadLong(image->file);
        viff_header.y_pixel_size=(float) MSBFirstReadLong(image->file);
        viff_header.location_type=MSBFirstReadLong(image->file);
        viff_header.location_dimension=MSBFirstReadLong(image->file);
        viff_header.number_of_images=MSBFirstReadLong(image->file);
        viff_header.number_data_bands=MSBFirstReadLong(image->file);
        viff_header.data_storage_type=MSBFirstReadLong(image->file);
        viff_header.data_encode_scheme=MSBFirstReadLong(image->file);
        viff_header.map_scheme=MSBFirstReadLong(image->file);
        viff_header.map_storage_type=MSBFirstReadLong(image->file);
        viff_header.map_rows=MSBFirstReadLong(image->file);
        viff_header.map_columns=MSBFirstReadLong(image->file);
        viff_header.map_subrows=MSBFirstReadLong(image->file);
        viff_header.map_enable=MSBFirstReadLong(image->file);
        viff_header.maps_per_cycle=MSBFirstReadLong(image->file);
        viff_header.color_space_model=MSBFirstReadLong(image->file);
      }
    for (i=0; i < 420; i++)
      (void) fgetc(image->file);
    /*
      Verify that we can read this VIFF image.
    */
    if ((viff_header.columns*viff_header.rows) == 0)
      PrematureExit("Image column or row size is not supported",image);
    if ((viff_header.data_storage_type != VFF_TYP_BIT) &&
        (viff_header.data_storage_type != VFF_TYP_1_BYTE) &&
        (viff_header.data_storage_type != VFF_TYP_2_BYTE) &&
        (viff_header.data_storage_type != VFF_TYP_4_BYTE))
      PrematureExit("Data storage type is not supported",image);
    if (viff_header.data_encode_scheme != VFF_DES_RAW)
      PrematureExit("Data encoding scheme is not supported",image);
    if ((viff_header.map_storage_type != VFF_MAPTYP_NONE) &&
        (viff_header.map_storage_type != VFF_MAPTYP_1_BYTE))
      PrematureExit("Map storage type is not supported",image);
    if ((viff_header.color_space_model != VFF_CM_NONE) &&
        (viff_header.color_space_model != VFF_CM_ntscRGB) &&
        (viff_header.color_space_model != VFF_CM_genericRGB))
      PrematureExit("Colorspace model is not supported",image);
    if (viff_header.location_type != VFF_LOC_IMPLICIT)
      {
        Warning("Location type is not supported",image->filename);
        DestroyImages(image);
        return((Image *) NULL);
      }
    if (viff_header.number_of_images != 1)
      PrematureExit("Number of images is not supported",image);
    switch (viff_header.map_scheme)
    {
      case VFF_MS_NONE:
      {
        if (viff_header.number_data_bands < 3)
          {
            /*
              Create linear color ramp.
            */
            if (viff_header.data_storage_type == VFF_TYP_BIT)
              image->colors=2;
            else
              image->colors=1 << (viff_header.number_data_bands*QuantumDepth);
            image->colormap=(ColorPacket *)
              malloc(image->colors*sizeof(ColorPacket));
            if (image->colormap == (ColorPacket *) NULL)
              PrematureExit("Unable to allocate memory",image);
            for (i=0; i < image->colors; i++)
            {
              image->colormap[i].red=(MaxRGB*i)/(image->colors-1);
              image->colormap[i].green=(MaxRGB*i)/(image->colors-1);
              image->colormap[i].blue=(MaxRGB*i)/(image->colors-1);
            }
          }
        break;
      }
      case VFF_MS_ONEPERBAND:
      case VFF_MS_SHARED:
      {
        unsigned char
          *viff_colormap;

        /*
          Read VIFF raster colormap.
        */
        image->colors=viff_header.map_columns;
        image->colormap=(ColorPacket *)
          malloc(image->colors*sizeof(ColorPacket));
        viff_colormap=(unsigned char *)
          malloc(image->colors*sizeof(unsigned char));
        if ((image->colormap == (ColorPacket *) NULL) ||
            (viff_colormap == (unsigned char *) NULL))
          PrematureExit("Unable to allocate memory",image);
        (void) ReadData((char *) viff_colormap,1,image->colors,image->file);
        for (i=0; i < image->colors; i++)
        {
          image->colormap[i].red=UpScale(viff_colormap[i]);
          image->colormap[i].green=UpScale(viff_colormap[i]);
          image->colormap[i].blue=UpScale(viff_colormap[i]);
        }
        if (viff_header.map_rows > 1)
          {
            (void) ReadData((char *) viff_colormap,1,image->colors,image->file);
            for (i=0; i < image->colors; i++)
              image->colormap[i].green=UpScale(viff_colormap[i]);
          }
        if (viff_header.map_rows > 2)
          {
            (void) ReadData((char *) viff_colormap,1,image->colors,image->file);
            for (i=0; i < image->colors; i++)
              image->colormap[i].blue=UpScale(viff_colormap[i]);
          }
        free((char *) viff_colormap);
        break;
      }
      default:
        PrematureExit("Colormap type is not supported",image);
    }
    /*
      Allocate VIFF pixels.
    */
    bytes_per_pixel=1;
    if (viff_header.data_storage_type == VFF_TYP_2_BYTE)
      bytes_per_pixel=2;
    if (viff_header.data_storage_type == VFF_TYP_4_BYTE)
      bytes_per_pixel=4;
    if (viff_header.data_storage_type == VFF_TYP_BIT)
      packets=((viff_header.columns+7) >> 3)*viff_header.rows;
    else
      packets=
        viff_header.columns*viff_header.rows*viff_header.number_data_bands;
    viff_pixels=(unsigned char *)
      malloc(bytes_per_pixel*packets*sizeof(Quantum));
    if (viff_pixels == (unsigned char *) NULL)
      PrematureExit("Unable to allocate memory",image);
    (void) ReadData((char *) viff_pixels,bytes_per_pixel,(unsigned int) packets,
      image->file);
    switch (viff_header.data_storage_type)
    {
      int
        max_value,
        min_value,
        value;

      register Quantum
        *q;

      unsigned long
        scale_factor;

      case VFF_TYP_1_BYTE:
      {
        register unsigned char
          *p;

        if (QuantumDepth == 8)
          break;
        /*
          Scale integer pixels to [0..MaxRGB].
        */
        p=viff_pixels;
        q=(Quantum *) viff_pixels;
        p+=packets-1;
        q+=packets-1;
        for (i=0; i < packets; i++)
        {
          value=UpScale(*p);
          *q=(Quantum) value;
          p--;
          q--;
        }
        break;
      }
      case VFF_TYP_2_BYTE:
      {
        register short int
          *p;

        /*
          Ensure the header Byte-order is most-significant Byte first.
        */
        if ((viff_header.machine_dependency == VFF_DEP_DECORDER) ||
            (viff_header.machine_dependency == VFF_DEP_NSORDER))
          MSBFirstOrderShort((char *) &viff_header,
            (unsigned int) (bytes_per_pixel*packets));
        /*
          Determine scale factor.
        */
        p=(short int *) viff_pixels;
        max_value=(*p);
        min_value=(*p);
        for (i=0; i < packets; i++)
        {
          if (*p > max_value)
            max_value=(*p);
          else
            if (*p < min_value)
              min_value=(*p);
          p++;
        }
        if ((min_value == 0) && (max_value == 0))
          scale_factor=0;
        else
          if (min_value == max_value)
            {
              scale_factor=UpShift(MaxRGB)/min_value;
              min_value=0;
            }
          else
            scale_factor=UpShift(MaxRGB)/(max_value-min_value);
        /*
          Scale integer pixels to [0..MaxRGB].
        */
        p=(short int *) viff_pixels;
        q=(Quantum *) viff_pixels;
        for (i=0; i < packets; i++)
        {
          value=DownShift((*p-min_value)*scale_factor);
          if (value > MaxRGB)
            value=MaxRGB;
          else
            if (value < 0)
              value=0;
          *q=(Quantum) value;
          p++;
          q++;
        }
        break;
      }
      case VFF_TYP_4_BYTE:
      {
        register int
          *p;

        /*
          Ensure the header Byte-order is most-significant Byte first.
        */
        if ((viff_header.machine_dependency == VFF_DEP_DECORDER) ||
            (viff_header.machine_dependency == VFF_DEP_NSORDER))
          MSBFirstOrderLong((char *) &viff_header,
            (unsigned int) (bytes_per_pixel*packets));
        /*
          Determine scale factor.
        */
        p=(int *) viff_pixels;
        max_value=(*p);
        min_value=(*p);
        for (i=0; i < packets; i++)
        {
          if (*p > max_value)
            max_value=(*p);
          else
            if (*p < min_value)
              min_value=(*p);
          p++;
        }
        if ((min_value == 0) && (max_value == 0))
          scale_factor=0;
        else
          if (min_value == max_value)
            {
              scale_factor=UpShift(MaxRGB)/min_value;
              min_value=0;
            }
          else
            scale_factor=UpShift(MaxRGB)/(max_value-min_value);
        /*
          Scale integer pixels to [0..MaxRGB].
        */
        p=(int *) viff_pixels;
        q=(Quantum *) viff_pixels;
        for (i=0; i < packets; i++)
        {
          value=DownShift((*p-min_value)*scale_factor);
          if (value > MaxRGB)
            value=MaxRGB;
          else
            if (value < 0)
              value=0;
          *q=(unsigned char) value;
          p++;
          q++;
        }
        break;
      }
    }
    /*
      Initialize image structure.
    */
    image->matte=(viff_header.number_data_bands == 4);
    image->class=
      (viff_header.number_data_bands < 3 ? PseudoClass : DirectClass);
    image->columns=viff_header.rows;
    image->rows=viff_header.columns;
    image->packets=image->columns*image->rows;
    image->pixels=(RunlengthPacket *)
      malloc(image->packets*sizeof(RunlengthPacket));
    if (image->pixels == (RunlengthPacket *) NULL)
      PrematureExit("Unable to allocate memory",image);
    /*
      Convert VIFF raster image to runlength-encoded packets.
    */
    p=(Quantum *) viff_pixels;
    q=image->pixels;
    if (viff_header.data_storage_type == VFF_TYP_BIT)
      {
        unsigned int
          polarity;

        /*
          Convert bitmap scanline to runlength-encoded color packets.
        */
        polarity=(viff_header.machine_dependency == VFF_DEP_DECORDER) ||
          (viff_header.machine_dependency == VFF_DEP_NSORDER);
        for (y=0; y < image->rows; y++)
        {
          /*
            Convert bitmap scanline to runlength-encoded color packets.
          */
          for (x=0; x < (image->columns >> 3); x++)
          {
            for (bit=0; bit < 8; bit++)
            {
              q->index=((*p) & (0x01 << bit) ? polarity : !polarity);
              q->length=0;
              q++;
            }
            p++;
          }
          if ((image->columns % 8) != 0)
            {
              for (bit=0; bit < (image->columns % 8); bit++)
              {
                q->index=((*p) & (0x01 << bit) ? polarity : !polarity);
                q->length=0;
                q++;
              }
              p++;
            }
          ProgressMonitor(LoadImageText,y,image->rows);
        }
      }
    else
      if (image->class == PseudoClass)
        for (y=0; y < image->rows; y++)
        {
          /*
            Convert PseudoColor scanline to runlength-encoded color packets.
          */
          for (x=0; x < image->columns; x++)
          {
            q->index=(*p++);
            q->length=0;
            q++;
          }
          ProgressMonitor(LoadImageText,y,image->rows);
        }
      else
        {
          unsigned long
            offset;

          /*
            Convert DirectColor scanline to runlength-encoded color packets.
          */
          offset=image->columns*image->rows;
          for (y=0; y < image->rows; y++)
          {
            for (x=0; x < image->columns; x++)
            {
              q->red=(*p);
              q->green=(*(p+offset));
              q->blue=(*(p+offset*2));
              if (image->colors != 0)
                {
                  q->red=image->colormap[q->red].red;
                  q->green=image->colormap[q->green].green;
                  q->blue=image->colormap[q->blue].blue;
                }
              q->index=(unsigned short) (image->matte ? (*(p+offset*3)) : 0);
              q->length=0;
              p++;
              q++;
            }
            ProgressMonitor(LoadImageText,y,image->rows);
          }
        }
    free((char *) viff_pixels);
    if (image->class == PseudoClass)
      {
        SyncImage(image);
        CompressColormap(image);
      }
    CompressImage(image);
    /*
      Proceed to next image.
    */
    status=ReadData((char *) &viff_header.identifier,1,1,image->file);
    if ((status == True) && (viff_header.identifier == 0xab))
      {
        /*
          Allocate image structure.
        */
        image->next=AllocateImage(image_info);
        if (image->next == (Image *) NULL)
          {
            DestroyImages(image);
            return((Image *) NULL);
          }
        (void) strcpy(image->next->filename,image_info->filename);
        image->next->file=image->file;
        image->next->scene=image->scene+1;
        image->next->previous=image;
        image=image->next;
      }
  } while ((status == True) && (viff_header.identifier == 0xab));
  while (image->previous != (Image *) NULL)
    image=image->previous;
  CloseImage(image);
  return(image);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   R e a d X I m a g e                                                       %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Procedure ReadXImage reads an image from an X window.
%
%  The format of the ReadXImage routine is:
%
%      image=ReadXImage(image_info,frame,borders,screen,descend)
%
%  A description of each parameter follows:
%
%    o image_info: Specifies a pointer to an ImageInfo structure.
%
%    o frame: Specifies whether to include the window manager frame with the
%      image.
%
%    o borders: Specifies whether borders pixels are to be saved with
%      the image.
%
%    o screen: Specifies whether the GetImage request used to obtain the image
%      should be done on the root window, rather than directly on the specified
%      window.
%
%    o descend: If this option is zero, check to see if the WM_COLORMAP_WINDOWS
%      property is set or if XListInstalledColormaps returns more than one
%      colormap.  If so, the image is obtained by descending the window
%      hierarchy and reading each subwindow and its colormap.
%
%
*/
Image *ReadXImage(ImageInfo *image_info, unsigned int frame, unsigned int borders, unsigned int screen, unsigned int descend)
{
  Colormap
    *colormaps;

  Display
    *display;

  Image
    *image;

  int
    status,
    x;

  RectangleInfo
    crop_info;

  Window
    *children,
    client,
    prior_target,
    root,
    target;

  XTextProperty
    window_name;

  /*
    Open X server connection.
  */
  display=XOpenDisplay(image_info->server_name);
  if (display == (Display *) NULL)
    {
      Warning("Unable to connect to X server",
        XDisplayName(image_info->server_name));
      return((Image *) NULL);
    }
  /*
    Set our forgiving error handler.
  */
  XSetErrorHandler(XError);
  /*
    Select target window.
  */
  crop_info.x=0;
  crop_info.y=0;
  crop_info.width=0;
  crop_info.height=0;
  root=XRootWindow(display,XDefaultScreen(display));
  target=(Window) NULL;
  if ((image_info->filename != (char *) NULL) &&
      (*image_info->filename != '\0'))
    if (Latin1Compare(image_info->filename,"root") == 0)
      target=root;
    else
      {
        /*
          Select window by ID or name.
        */
        if (isdigit(*image_info->filename))
          target=XWindowByID(display,root,(Window) strtol(image_info->filename,
            (char **) NULL,0));
        if (target == (Window) NULL)
          target=XWindowByName(display,root,image_info->filename);
        if (target == (Window) NULL)
          Warning("No window with specified id exists",image_info->filename);
      }

  /*
    If target window is not defined, interactively select one.
  */
  prior_target=target;
  if (target == (Window) NULL)
    target=XSelectWindow(display,&crop_info);
  client=target;   /* obsolete */
  if (target != root)
    {
      unsigned int
        d;

      status=XGetGeometry(display,target,&root,&x,&x,&d,&d,&d,&d);
      if (status != 0)
        {
          for ( ; ; )
          {
            Window
              parent;

            /*
              Find window manager frame.
            */
            status=XQueryTree(display,target,&root,&parent,&children,&d);
            if (status && (children != (Window *) NULL))
              XFree((char *) children);
            if (!status || (parent == (Window) NULL) || (parent == root))
              break;
            target=parent;
          }
          /*
            Get client window.
          */
          client=XClientWindow(display,target);
          if (!frame)
            target=client;
          if (!frame && prior_target)
            target=prior_target;
        }
    }
  if (screen)
    {
      int
        y;

      Window
        child;

      XWindowAttributes
        window_attributes;

      /*
        Obtain window image directly from screen.
      */
      status=XGetWindowAttributes(display,target,&window_attributes);
      if (status == False)
        {
          Warning("Unable to read X window attributes",image_info->filename);
          XCloseDisplay(display);
          return((Image *) NULL);
        }
      XTranslateCoordinates(display,target,root,0,0,&x,&y,&child);
      crop_info.x=x;
      crop_info.y=y;
      crop_info.width=window_attributes.width;
      crop_info.height=window_attributes.height;
      if (borders)
        {
          /*
            Include border in image.
          */
          crop_info.x-=window_attributes.border_width;
          crop_info.y-=window_attributes.border_width;
          crop_info.width+=window_attributes.border_width << 1;
          crop_info.height+=window_attributes.border_width << 1;
        }
      target=root;
    }
  if (descend)
    {
      int
        number_colormaps,
        number_windows;

      /*
        If WM_COLORMAP_WINDOWS property is set or multiple colormaps, descend.
      */
      descend=False;
      number_windows=0;
      status=XGetWMColormapWindows(display,target,&children,&number_windows);
      if ((status == True) && (number_windows > 0))
        {
          descend=True;
          XFree ((char *) children);
        }
      colormaps=XListInstalledColormaps(display,target,&number_colormaps);
      if (number_colormaps > 0)
        {
          if (number_colormaps > 1)
            descend=True;
          XFree((char *) colormaps);
        }
    }
  /*
    Alert the user not to alter the screen.
  */
  XBell(display,0);
  /*
    Get image by window id.
  */
  XGrabServer(display);
  image=XGetWindowImage(display,target,borders,descend);
  XUngrabServer(display);
  if (image == (Image *) NULL)
    Warning("Unable to read X window image",image_info->filename);
  else
    {
      (void) strcpy(image->filename,image_info->filename);
      if ((crop_info.width != 0) && (crop_info.height != 0))
        {
          Image
            *cropped_image;

          /*
            Crop image as defined by the cropping rectangle.
          */
          cropped_image=CropImage(image,&crop_info);
          if (cropped_image != (Image *) NULL)
            {
              DestroyImage(image);
              image=cropped_image;
            }
        }
      status=XGetWMName(display,target,&window_name);
      if (status == True)
        {
          if ((image_info->filename != (char *) NULL) &&
              (*image_info->filename == '\0'))
            {
              /*
                Initialize image filename.
              */
              (void) strncpy(image->filename,(char *) window_name.value,
                (int) window_name.nitems);
              image->filename[window_name.nitems]='\0';
            }
          XFree((void *) window_name.value);
        }
    }
  /*
    Alert the user we're done.
  */
  XBell(display,0);
  XBell(display,0);
  CompressImage(image);
  XCloseDisplay(display);
  return(image);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%  R e a d X B M I m a g e                                                    %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Function ReadXBMImage reads an X11 bitmap image file and returns it.  It
%  allocates the memory necessary for the new Image structure and returns a
%  pointer to the new image.
%
%  The format of the ReadXBMImage routine is:
%
%      image=ReadXBMImage(image_info)
%
%  A description of each parameter follows:
%
%    o image:  Function ReadXBMImage returns a pointer to the image after
%      reading.  A null image is returned if there is a a memory shortage or
%      if the image cannot be read.
%
%    o image_info: Specifies a pointer to an ImageInfo structure.
%
%
*/
static int XBMInteger(FILE *file, short int *hex_digits)
{
  int
    c,
    flag,
    value;

  value=0;
  flag=0;
  for ( ; ; )
  {
    c=getc(file);
    if (c == EOF)
      {
        value=(-1);
        break;
      }
    c&=0xff;
    if (isascii(c) && isxdigit(c))
      {
        value=(value << 4)+hex_digits[c];
        flag++;
        continue;
      }
    if ((hex_digits[c]) < 0 && flag)
      break;
  }
  return(value);
}

static Image *ReadXBMImage(ImageInfo *image_info)
{
  char
    buffer[MaxTextLength],
    name[MaxTextLength];

  Image
    *image;

  register int
    x,
    y;

  register RunlengthPacket
    *q;

  register unsigned char
    *p;

  register unsigned short
    index;

  short int
    hex_digits[256];

  unsigned char
    bit,
    *data;

  unsigned int
    Byte,
    bytes_per_line,
    packets,
    padding,
    value,
    version;

  /*
    Allocate image structure.
  */
  image=AllocateImage(image_info);
  if (image == (Image *) NULL)
    return((Image *) NULL);
  /*
    Open image file.
  */
  OpenImage(image,ReadBinaryType);
  if (image->file == (FILE *) NULL)
    PrematureExit("Unable to open file",image);
  /*
    Read X bitmap header.
  */
  while (fgets(buffer,MaxTextLength-1,image->file) != (char *) NULL)
    if (sscanf(buffer,"#define %*32s %u",&image->columns) == 1)
      break;
  while (fgets(buffer,MaxTextLength-1,image->file) != (char *) NULL)
    if (sscanf(buffer,"#define %*32s %u",&image->rows) == 1)
      break;
  /*
    Scan until hex digits.
  */
  version=11;
  while (fgets(buffer,MaxTextLength-1,image->file) != (char *) NULL)
  {
    if (sscanf(buffer,"static short %s = {",name) == 1)
      version=10;
    else
      if (sscanf(buffer,"static unsigned char %s = {",name) == 1)
        version=11;
      else
        if (sscanf(buffer,"static char %s = {",name) == 1)
          version=11;
        else
          continue;
    p=(unsigned char *) strrchr(name,'_');
    if (p == (unsigned char *) NULL)
      p=(unsigned char *) name;
    else
      p++;
    if (strcmp("bits[]",(char *) p) == 0)
      break;
  }
  if ((image->columns == 0) || (image->rows == 0) || feof(image->file))
    PrematureExit("XBM file is not in the correct format",image);
  /*
    Initialize image structure.
  */
  image->packets=image->columns*image->rows;
  image->pixels=(RunlengthPacket *)
    malloc(image->packets*sizeof(RunlengthPacket));
  padding=0;
  if ((image->columns % 16) && ((image->columns % 16) < 9)  && (version == 10))
    padding=1;
  bytes_per_line=(image->columns+7)/8+padding;
  packets=bytes_per_line*image->rows;
  data=(unsigned char *) malloc(packets*sizeof(unsigned char *));
  if ((image->pixels == (RunlengthPacket *) NULL) ||
      (data == (unsigned char *) NULL))
    PrematureExit("Unable to allocate memory",image);
  /*
    Create colormap.
  */
  image->class=PseudoClass;
  image->colors=2;
  image->colormap=(ColorPacket *) malloc(image->colors*sizeof(ColorPacket));
  if (image->colormap == (ColorPacket *) NULL)
    PrematureExit("Unable to allocate memory",image);
  image->colormap[0].red=0;
  image->colormap[0].green=0;
  image->colormap[0].blue=0;
  image->colormap[1].red=MaxRGB;
  image->colormap[1].green=MaxRGB;
  image->colormap[1].blue=MaxRGB;
  /*
    Initialize hex values.
  */
  hex_digits['0']=0;
  hex_digits['1']=1;
  hex_digits['2']=2;
  hex_digits['3']=3;
  hex_digits['4']=4;
  hex_digits['5']=5;
  hex_digits['6']=6;
  hex_digits['7']=7;
  hex_digits['8']=8;
  hex_digits['9']=9;
  hex_digits['A']=10;
  hex_digits['B']=11;
  hex_digits['C']=12;
  hex_digits['D']=13;
  hex_digits['E']=14;
  hex_digits['F']=15;
  hex_digits['a']=10;
  hex_digits['b']=11;
  hex_digits['c']=12;
  hex_digits['d']=13;
  hex_digits['e']=14;
  hex_digits['f']=15;
  hex_digits[' ']=(-1);
  hex_digits[',']=(-1);
  hex_digits['}']=(-1);
  hex_digits['\n']=(-1);
  hex_digits['\t']=(-1);
  /*
    Read hex image data.
  */
  p=data;
  if (version == 10)
    for (x=0; x < packets; (x+=2))
    {
      value=XBMInteger(image->file,hex_digits);
      *p++=value;
      if (!padding || ((x+2) % bytes_per_line))
        *p++=value >> 8;
    }
  else
    for (x=0; x < packets; x++)
    {
      value=XBMInteger(image->file,hex_digits);
      *p++=value;
    }
  /*
    Convert X bitmap image to runlength-encoded packets.
  */
  p=data;
  q=image->pixels;
  for (y=0; y < image->rows; y++)
  {
    bit=0;
    for (x=0; x < image->columns; x++)
    {
      if (bit == 0)
        Byte=(*p++);
      index=(Byte & 0x01) ? 0 : 1;
      q->index=index;
      q->length=0;
      q++;
      bit++;
      Byte>>=1;
      if (bit == 8)
        bit=0;
    }
    ProgressMonitor(LoadImageText,y,image->rows);
  }
  SyncImage(image);
  CompressImage(image);
  CloseImage(image);
  free((char *) data);
  return(image);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%  R e a d X C I m a g e                                                      %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Function ReadXCImage creates a constant image and initializes to the
%  X server color as specified by the filename.  It allocates the memory
%  necessary for the new Image structure and returns a pointer to the new
%  image.
%
%  The format of the ReadXCImage routine is:
%
%      image=ReadXCImage(image_info)
%
%  A description of each parameter follows:
%
%    o image:  Function ReadXCImage returns a pointer to the image after
%      creating it. A null image is returned if there is a a memory shortage
%      or if the image cannot be read.
%
%    o image_info: Specifies a pointer to an ImageInfo structure.
%
%
*/
static Image *ReadXCImage(ImageInfo *image_info)
{
  Image
    *image;

  int
    x,
    y;

  register int
    i;

  register RunlengthPacket
    *q;

  unsigned int
    height,
    width;

  XColor
    color;

  /*
    Allocate image structure.
  */
  image=AllocateImage(image_info);
  if (image == (Image *) NULL)
    return((Image *) NULL);
  /*
    Determine width and height, e.g. 640x512.
  */
  width=512;
  height=512;
  if (image_info->size != (char *) NULL)
    (void) XParseGeometry(image_info->size,&x,&y,&width,&height);
  /*
    Initialize Image structure.
  */
  (void) strcpy(image->filename,image_info->filename);
  image->columns=width;
  image->rows=height;
  image->packets=((image->columns*image->rows-1)/(MaxRunlength+1))+1;
  image->pixels=(RunlengthPacket *)
    malloc(image->packets*sizeof(RunlengthPacket));
  image->class=PseudoClass;
  image->colors=1;
  image->colormap=(ColorPacket *) malloc(image->colors*sizeof(ColorPacket));
  if ((image->pixels == (RunlengthPacket *) NULL) ||
      (image->colormap == (ColorPacket *) NULL))
    PrematureExit("Unable to allocate memory",image);
  /*
    Initialize colormap.
  */
  (void) XQueryColorDatabase(image_info->filename,&color);
  image->colormap[0].red=XDownScale(color.red);
  image->colormap[0].green=XDownScale(color.green);
  image->colormap[0].blue=XDownScale(color.blue);
  q=image->pixels;
  for (i=0; i < image->packets; i++)
  {
    q->index=0;
    q->length=MaxRunlength;
    q++;
    if (QuantumTick(i,image))
      ProgressMonitor(LoadImageText,i,image->packets);
  }
  q--;
  q->length=image->columns*image->rows-(MaxRunlength+1)*(image->packets-1)-1;
  SyncImage(image);
  return(image);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%  R e a d X P M I m a g e                                                    %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Function ReadXBMImage reads an X11 pixmap image file and returns it.  It
%  allocates the memory necessary for the new Image structure and returns a
%  pointer to the new image.
%
%  The format of the ReadXPMImage routine is:
%
%      image=ReadXPMImage(image_info)
%
%  A description of each parameter follows:
%
%    o image:  Function ReadXPMImage returns a pointer to the image after
%      creating it. A null image is returned if there is a a memory shortage
%      or if the image cannot be read.
%
%    o image_info: Specifies a pointer to an ImageInfo structure.
%
%
*/
#ifdef HasXPM
#include "xpm.h"
static Image *ReadXPMImage(image_info)
ImageInfo
  *image_info;
{
  char
    *xpm_buffer;

  Display
    *display;

  Image
    *image;

  int
    length,
    status,
    x,
    y;

  register char
    *q;

  register int
    i;

  register RunlengthPacket
    *p;

  register unsigned long
    pixel;

  XColor
    *colors;

  XImage
    *matte_image,
    *ximage;

  XpmAttributes
    xpm_attributes;

  XResourceInfo
    resource_info;

  XrmDatabase
    resource_database;

  XStandardColormap
    *map_info;

  XVisualInfo
    *visual_info;

  /*
    Allocate image structure.
  */
  image=AllocateImage(image_info);
  if (image == (Image *) NULL)
    return((Image *) NULL);
  /*
    Open image file.
  */
  OpenImage(image,"r");
  if (image->file == (FILE *) NULL)
    PrematureExit("Unable to open file",image);
  /*
    Read XPM file.
  */
  length=MaxTextLength;
  xpm_buffer=(char *) malloc(length*sizeof(char));
  if (xpm_buffer != (char *) NULL)
    {
      q=xpm_buffer;
      while (fgets(q,MaxTextLength-1,image->file) != (char *) NULL)
      {
        q+=strlen(q);
        if ((q-xpm_buffer+MaxTextLength) > length)
          {
            *q='\0';
            length<<=1;
            xpm_buffer=(char *)
              realloc((char *) xpm_buffer,length*sizeof(char));
            if (xpm_buffer == (char *) NULL)
              break;
            q=xpm_buffer+strlen(xpm_buffer);
          }
      }
    }
  if (xpm_buffer == (char *) NULL)
    PrematureExit("Unable to allocate memory",image);
  CloseImage(image);
  /*
    Open X server connection.
  */
  display=XOpenDisplay(image_info->server_name);
  if (display == (Display *) NULL)
    {
      Warning("Unable to connect to X server",
        XDisplayName(image_info->server_name));
      PrematureExit("Unable to create XPM image",image);
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
  else
    {
      /*
        Initialize visual info.
      */
      visual_info=XBestVisualInfo(display,map_info,&resource_info);
      if (visual_info == (XVisualInfo *) NULL)
        Warning("Unable to get visual",resource_info.visual_type);
      map_info->colormap=(Colormap) NULL;
    }
  if ((map_info == (XStandardColormap *) NULL) ||
      (visual_info == (XVisualInfo *) NULL))
    {
      XFreeResources(display,visual_info,map_info,(XPixelInfo *) NULL,
        (XFontStruct *) NULL,&resource_info,(XWindowInfo *) NULL);
      PrematureExit("Unable to create XPM image",image);
    }
  /*
    Initialize X colormap.
  */
  map_info->colormap=XCreateColormap(display,
    XRootWindow(display,visual_info->screen),visual_info->visual,
    visual_info->class == DirectColor ? AllocAll : AllocNone);
  if (map_info->colormap == (Colormap) NULL)
    {
      XFreeResources(display,visual_info,map_info,(XPixelInfo *) NULL,
        (XFontStruct *) NULL,&resource_info,(XWindowInfo *) NULL);
      PrematureExit("Unable to create colormap",image);
    }
  /*
    Initialize XPM attributes.
  */
  xpm_attributes.valuemask=XpmColorKey | XpmColormap | XpmDepth | XpmVisual;
  xpm_attributes.visual=visual_info->visual;
  xpm_attributes.colormap=map_info->colormap;
  xpm_attributes.depth=visual_info->depth;
  xpm_attributes.color_key=XPM_COLOR;
  /*
    Read in a file in the XPM format into a X image structure.
  */
  status=XpmCreateImageFromBuffer(display,xpm_buffer,&ximage,&matte_image,
    &xpm_attributes);
  if (status != XpmSuccess)
    {
      XFreeResources(display,visual_info,map_info,(XPixelInfo *) NULL,
        (XFontStruct *) NULL,&resource_info,(XWindowInfo *) NULL);
      PrematureExit("Unable to create XPM image",image);
    }
  free((char *) xpm_buffer);
  XpmFreeAttributes(&xpm_attributes);
  /*
    Get the colormap colors.
  */
  colors=(XColor *) malloc(visual_info->colormap_size*sizeof(XColor));
  if (colors == (XColor *) NULL)
    {
      XFreeResources(display,visual_info,map_info,(XPixelInfo *) NULL,
        (XFontStruct *) NULL,&resource_info,(XWindowInfo *) NULL);
      XDestroyImage(ximage);
      PrematureExit("Unable to read X colormap",image);
    }
  if ((visual_info->class != DirectColor) && (visual_info->class != TrueColor))
    for (i=0; i < visual_info->colormap_size; i++)
    {
      colors[i].pixel=i;
      colors[i].pad=0;
    }
  else
    {
      unsigned long
        blue,
        blue_bit,
        green,
        green_bit,
        red,
        red_bit;

      /*
        DirectColor or TrueColor visual.
      */
      red=0;
      green=0;
      blue=0;
      red_bit=visual_info->red_mask & (~(visual_info->red_mask)+1);
      green_bit=visual_info->green_mask & (~(visual_info->green_mask)+1);
      blue_bit=visual_info->blue_mask & (~(visual_info->blue_mask)+1);
      for (i=0; i < visual_info->colormap_size; i++)
      {
        colors[i].pixel=red | green | blue;
        colors[i].pad=0;
        red+=red_bit;
        if (red > visual_info->red_mask)
          red=0;
        green+=green_bit;
        if (green > visual_info->green_mask)
          green=0;
        blue+=blue_bit;
        if (blue > visual_info->blue_mask)
          blue=0;
      }
    }
  XQueryColors(display,map_info->colormap,colors,visual_info->colormap_size);
  /*
    Convert X image to MIFF format.
  */
  if ((visual_info->class != TrueColor) && (visual_info->class != DirectColor))
    image->class=PseudoClass;
  image->columns=ximage->width;
  image->rows=ximage->height;
  image->packets=image->columns*image->rows;
  image->pixels=(RunlengthPacket *)
    malloc(image->packets*sizeof(RunlengthPacket));
  if (image->pixels == (RunlengthPacket *) NULL)
    {
      free((char *) colors);
      XFreeResources(display,visual_info,map_info,(XPixelInfo *) NULL,
        (XFontStruct *) NULL,&resource_info,(XWindowInfo *) NULL);
      XDestroyImage(ximage);
      PrematureExit("Unable to allocate memory",image);
    }
  p=image->pixels;
  switch (image->class)
  {
    case DirectClass:
    {
      register unsigned long
        color,
        index;

      unsigned long
        blue_mask,
        blue_shift,
        green_mask,
        green_shift,
        red_mask,
        red_shift;

      /*
        Determine shift and mask for red, green, and blue.
      */
      red_mask=visual_info->red_mask;
      red_shift=0;
      while ((red_mask & 0x01) == 0)
      {
        red_mask>>=1;
        red_shift++;
      }
      green_mask=visual_info->green_mask;
      green_shift=0;
      while ((green_mask & 0x01) == 0)
      {
        green_mask>>=1;
        green_shift++;
      }
      blue_mask=visual_info->blue_mask;
      blue_shift=0;
      while ((blue_mask & 0x01) == 0)
      {
        blue_mask>>=1;
        blue_shift++;
      }
      /*
        Convert X image to DirectClass packets.
      */
      if ((visual_info->colormap_size > 0) &&
          (visual_info->class == DirectColor))
        for (y=0; y < image->rows; y++)
        {
          for (x=0; x < image->columns; x++)
          {
            pixel=XGetPixel(ximage,x,y);
            index=(pixel >> red_shift) & red_mask;
            p->red=XDownScale(colors[index].red);
            index=(pixel >> green_shift) & green_mask;
            p->green=XDownScale(colors[index].green);
            index=(pixel >> blue_shift) & blue_mask;
            p->blue=XDownScale(colors[index].blue);
            p->index=0;
            p->length=0;
            p++;
          }
          ProgressMonitor(LoadImageText,y,image->rows);
        }
      else
        for (y=0; y < image->rows; y++)
        {
          for (x=0; x < image->columns; x++)
          {
            pixel=XGetPixel(ximage,x,y);
            color=(pixel >> red_shift) & red_mask;
            p->red=XDownScale((color*65535L)/red_mask);
            color=(pixel >> green_shift) & green_mask;
            p->green=XDownScale((color*65535L)/green_mask);
            color=(pixel >> blue_shift) & blue_mask;
            p->blue=XDownScale((color*65535L)/blue_mask);
            p->index=0;
            p->length=0;
            p++;
          }
          ProgressMonitor(LoadImageText,y,image->rows);
        }
      break;
    }
    case PseudoClass:
    {
      /*
        Create colormap.
      */
      image->colors=visual_info->colormap_size;
      image->colormap=(ColorPacket *) malloc(image->colors*sizeof(ColorPacket));
      if (image->colormap == (ColorPacket *) NULL)
        {
          free((char *) colors);
          XFreeResources(display,visual_info,map_info,(XPixelInfo *) NULL,
            (XFontStruct *) NULL,&resource_info,(XWindowInfo *) NULL);
          XDestroyImage(ximage);
          DestroyImage(image);
          return((Image *) NULL);
        }
      for (i=0; i < image->colors; i++)
      {
        image->colormap[colors[i].pixel].red=XDownScale(colors[i].red);
        image->colormap[colors[i].pixel].green=XDownScale(colors[i].green);
        image->colormap[colors[i].pixel].blue=XDownScale(colors[i].blue);
      }
      /*
        Convert X image to PseudoClass packets.
      */
      for (y=0; y < image->rows; y++)
      {
        for (x=0; x < image->columns; x++)
        {
          p->index=(unsigned short) XGetPixel(ximage,x,y);
          p->length=0;
          p++;
        }
        ProgressMonitor(LoadImageText,y,image->rows);
      }
      SyncImage(image);
      break;
    }
  }
  if (matte_image != (XImage *) NULL)
    {
      /*
        Initialize image matte.
      */
      image->class=DirectClass;
      image->matte=True;
      p=image->pixels;
      for (y=0; y < image->rows; y++)
        for (x=0; x < image->columns; x++)
        {
          p->index=Opaque;
          if (!XGetPixel(matte_image,x,y))
            p->index=Transparent;
          p++;
        }
      XDestroyImage(matte_image);
    }
  if (image->class == PseudoClass)
    CompressColormap(image);
  /*
    Free resources.
  */
  free((char *) colors);
  XDestroyImage(ximage);
  XFreeResources(display,visual_info,map_info,(XPixelInfo *) NULL,
    (XFontStruct *) NULL,&resource_info,(XWindowInfo *) NULL);
  CompressImage(image);
  return(image);
}
#else
static Image *ReadXPMImage(ImageInfo *image_info)
{
  Warning("XPM library is not available",image_info->filename);
  return(ReadMIFFImage(image_info));
}
#endif

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%  R e a d X W D I m a g e                                                    %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Function ReadXWDImage reads an X Window System window dump image file and
%  returns it.  It allocates the memory necessary for the new Image structure
%  and returns a pointer to the new image.
%
%  The format of the ReadXWDImage routine is:
%
%      image=ReadXWDImage(image_info)
%
%  A description of each parameter follows:
%
%    o image:  Function ReadXWDImage returns a pointer to the image after
%      reading.  A null image is returned if there is a a memory shortage or
%      if the image cannot be read.
%
%    o image_info: Specifies a pointer to an ImageInfo structure.
%
%
*/
static Image *ReadXWDImage(ImageInfo *image_info)
{
  Image
    *image;

  int
    status,
    x,
    y;

  register int
    i;

  register RunlengthPacket
    *q;

  register unsigned long
    pixel;

  unsigned long
    lsb_first;

  unsigned int
    packets;

  unsigned short
    index;

  XColor
    *colors;

  XImage
    *ximage;

  XWDFileHeader
    header;

  /*
    Allocate image structure.
  */
  image=AllocateImage(image_info);
  if (image == (Image *) NULL)
    return((Image *) NULL);
  /*
    Open image file.
  */
  OpenImage(image,ReadBinaryType);
  if (image->file == (FILE *) NULL)
    PrematureExit("Unable to open file",image);
  /*
     Read in header information.
  */
  status=ReadData((char *) &header,sz_XWDheader,1,image->file);
  if (status == False)
    PrematureExit("Unable to read dump file header",image);
  /*
    Ensure the header Byte-order is most-significant Byte first.
  */
  lsb_first=1;
  if (*(char *) &lsb_first)
    MSBFirstOrderLong((char *) &header,sz_XWDheader);
  /*
    Check to see if the dump file is in the proper format.
  */
  if (header.file_version != XWD_FILE_VERSION)
    PrematureExit("XWD file format version mismatch",image);
  if (header.header_size < sz_XWDheader)
    PrematureExit("XWD header size is too small",image);
  packets=(header.header_size-sz_XWDheader);
  image->comments=(char *) malloc((packets+1)*sizeof(char));
  if (image->comments == (char *) NULL)
    PrematureExit("Unable to allocate memory",image);
  status=ReadData((char *) image->comments,1,packets,image->file);
  image->comments[packets]='\0';
  if (status == False)
    PrematureExit("Unable to  read window name from dump file",image);
  /*
    Initialize the X image.
  */
  ximage=(XImage *) malloc(sizeof(XImage));
  if (ximage == (XImage *) NULL)
    PrematureExit("Unable to allocate memory",image);
  ximage->depth=header.pixmap_depth;
  ximage->format=header.pixmap_format;
  ximage->xoffset=header.xoffset;
  ximage->data=(char *) NULL;
  ximage->width=header.pixmap_width;
  ximage->height=header.pixmap_height;
  ximage->bitmap_pad=header.bitmap_pad;
  ximage->bytes_per_line=header.bytes_per_line;
  ximage->byte_order=header.byte_order;
  ximage->bitmap_unit=header.bitmap_unit;
  ximage->bitmap_bit_order=header.bitmap_bit_order;
  ximage->bits_per_pixel=header.bits_per_pixel;
  ximage->red_mask=header.red_mask;
  ximage->green_mask=header.green_mask;
  ximage->blue_mask=header.blue_mask;
  status=XInitImage(ximage);
  if (status == False)
    PrematureExit("Invalid XWD header",image);
  /*
    Read colormap.
  */
  colors=(XColor *) NULL;
  if (header.ncolors != 0)
    {
      XWDColor
        color;

      colors=(XColor *) malloc((unsigned int) header.ncolors*sizeof(XColor));
      if (colors == (XColor *) NULL)
        PrematureExit("Unable to allocate memory",image);
      for (i=0; i < header.ncolors; i++)
      {
        status=ReadData((char *) &color,sz_XWDColor,1,image->file);
        if (status == False)
          PrematureExit("Unable to read color map from dump file",image);
        colors[i].pixel=color.pixel;
        colors[i].red=color.red;
        colors[i].green=color.green;
        colors[i].blue=color.blue;
        colors[i].flags=color.flags;
      }
      /*
        Ensure the header Byte-order is most-significant Byte first.
      */
      lsb_first=1;
      if (*(char *) &lsb_first)
        for (i=0; i < header.ncolors; i++)
        {
          MSBFirstOrderLong((char *) &colors[i].pixel,sizeof(unsigned long));
          MSBFirstOrderShort((char *) &colors[i].red,3*sizeof(unsigned short));
        }
    }
  /*
    Allocate the pixel buffer.
  */
  if (ximage->format == ZPixmap)
    packets=ximage->bytes_per_line*ximage->height;
  else
    packets=ximage->bytes_per_line*ximage->height*ximage->depth;
  ximage->data=(char *) malloc(packets*sizeof(unsigned char));
  if (ximage->data == (char *) NULL)
    PrematureExit("Unable to allocate memory",image);
  status=ReadData(ximage->data,1,packets,image->file);
  if (status == False)
    PrematureExit("Unable to read dump pixmap",image);
  /*
    Convert image to MIFF format.
  */
  image->columns=ximage->width;
  image->rows=ximage->height;
  if ((colors == (XColor *) NULL) || (ximage->red_mask != 0) ||
      (ximage->green_mask != 0) || (ximage->blue_mask != 0))
    image->class=DirectClass;
  else
    image->class=PseudoClass;
  image->colors=header.ncolors;
  image->packets=0;
  packets=Max((image->columns*image->rows+4) >> 3,1);
  image->pixels=(RunlengthPacket *) malloc(packets*sizeof(RunlengthPacket));
  if (image->pixels == (RunlengthPacket *) NULL)
    PrematureExit("Unable to allocate memory",image);
  q=image->pixels;
  q->length=MaxRunlength;
  switch (image->class)
  {
    case DirectClass:
    {
      register unsigned long
        color;

      unsigned int
        blue,
        green,
        red;

      unsigned long
        blue_mask,
        blue_shift,
        green_mask,
        green_shift,
        red_mask,
        red_shift;

      /*
        Determine shift and mask for red, green, and blue.
      */
      red_mask=ximage->red_mask;
      red_shift=0;
      while ((red_mask & 0x01) == 0)
      {
        red_mask>>=1;
        red_shift++;
      }
      green_mask=ximage->green_mask;
      green_shift=0;
      while ((green_mask & 0x01) == 0)
      {
        green_mask>>=1;
        green_shift++;
      }
      blue_mask=ximage->blue_mask;
      blue_shift=0;
      while ((blue_mask & 0x01) == 0)
      {
        blue_mask>>=1;
        blue_shift++;
      }
      /*
        Convert X image to DirectClass packets.
      */
      if (image->colors != 0)
        for (y=0; y < image->rows; y++)
        {
          for (x=0; x < image->columns; x++)
          {
            pixel=XGetPixel(ximage,x,y);
            index=(unsigned short) ((pixel >> red_shift) & red_mask);
            red=XDownScale(colors[index].red);
            index=(unsigned short) ((pixel >> green_shift) & green_mask);
            green=XDownScale(colors[index].green);
            index=(unsigned short) ((pixel >> blue_shift) & blue_mask);
            blue=XDownScale(colors[index].blue);
            if ((red == q->red) && (green == q->green) && (blue == q->blue) &&
                ((int) q->length < MaxRunlength))
              q->length++;
            else
              {
                if (image->packets != 0)
                  q++;
                image->packets++;
                if (image->packets == packets)
                  {
                    packets<<=1;
                    image->pixels=(RunlengthPacket *) realloc((char *)
                      image->pixels,packets*sizeof(RunlengthPacket));
                    if (image->pixels == (RunlengthPacket *) NULL)
                      PrematureExit("Unable to allocate memory",image);
                    q=image->pixels+image->packets-1;
                  }
                q->red=red;
                q->green=green;
                q->blue=blue;
                q->index=0;
                q->length=0;
              }
          }
          ProgressMonitor(LoadImageText,y,image->rows);
        }
      else
        for (y=0; y < image->rows; y++)
        {
          for (x=0; x < image->columns; x++)
          {
            pixel=XGetPixel(ximage,x,y);
            color=(pixel >> red_shift) & red_mask;
            red=XDownScale((color*65535L)/red_mask);
            color=(pixel >> green_shift) & green_mask;
            green=XDownScale((color*65535L)/green_mask);
            color=(pixel >> blue_shift) & blue_mask;
            blue=XDownScale((color*65535L)/blue_mask);
            if ((red == q->red) && (green == q->green) && (blue == q->blue) &&
                ((int) q->length < MaxRunlength))
              q->length++;
            else
              {
                if (image->packets != 0)
                  q++;
                image->packets++;
                if (image->packets == packets)
                  {
                    packets<<=1;
                    image->pixels=(RunlengthPacket *) realloc((char *)
                      image->pixels,packets*sizeof(RunlengthPacket));
                    if (image->pixels == (RunlengthPacket *) NULL)
                      PrematureExit("Unable to allocate memory",image);
                    q=image->pixels+image->packets-1;
                  }
                q->red=red;
                q->green=green;
                q->blue=blue;
                q->index=0;
                q->length=0;
              }
          }
          ProgressMonitor(LoadImageText,y,image->rows);
        }
      break;
    }
    case PseudoClass:
    {
      /*
        Convert X image to PseudoClass packets.
      */
      image->colormap=(ColorPacket *) malloc(image->colors*sizeof(ColorPacket));
      if (image->colormap == (ColorPacket *) NULL)
        PrematureExit("Unable to allocate memory",image);
      for (i=0; i < image->colors; i++)
      {
        image->colormap[i].red=XDownScale(colors[i].red);
        image->colormap[i].green=XDownScale(colors[i].green);
        image->colormap[i].blue=XDownScale(colors[i].blue);
      }
      for (y=0; y < image->rows; y++)
      {
        for (x=0; x < image->columns; x++)
        {
          pixel=XGetPixel(ximage,x,y);
          index=pixel;
          if ((index == q->index) && ((int) q->length < MaxRunlength))
            q->length++;
          else
            {
              if (image->packets != 0)
                q++;
              image->packets++;
              if (image->packets == packets)
                {
                  packets<<=1;
                  image->pixels=(RunlengthPacket *) realloc((char *)
                    image->pixels,packets*sizeof(RunlengthPacket));
                  if (image->pixels == (RunlengthPacket *) NULL)
                    PrematureExit("Unable to allocate memory",image);
                  q=image->pixels+image->packets-1;
                }
              q->index=index;
              q->length=0;
            }
        }
        ProgressMonitor(LoadImageText,y,image->rows);
      }
      SyncImage(image);
      CompressColormap(image);
      break;
    }
  }
  image->pixels=(RunlengthPacket *)
    realloc((char *) image->pixels,image->packets*sizeof(RunlengthPacket));
  /*
    Free image and colormap.
  */
  if (header.ncolors != 0)
    free((char *) colors);
  free(ximage->data);
  free(ximage);
  CloseImage(image);
  return(image);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%  R e a d Y U V I m a g e                                                    %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Function ReadYUVImage reads an image with digital YUV (CCIR 601 4:1:1) bytes
%  and returns it.  It allocates the memory necessary for the new Image
%  structure and returns a pointer to the new image.
%
%  The format of the ReadYUVImage routine is:
%
%      image=ReadYUVImage(image_info)
%
%  A description of each parameter follows:
%
%    o image:  Function ReadYUVImage returns a pointer to the image after
%      reading.  A null image is returned if there is a a memory shortage or
%      if the image cannot be read.
%
%    o image_info: Specifies a pointer to an ImageInfo structure.
%
%
*/
static Image *ReadYUVImage(ImageInfo *image_info)
{
  Image
    *image,
    *zoomed_image;

  int
    x,
    y;

  register int
    i;

  register RunlengthPacket
    *q;

  register unsigned char
    *p;

  unsigned char
    *yuv_pixels;

  unsigned int
    height,
    width;

  /*
    Allocate image structure.
  */
  zoomed_image=AllocateImage(image_info);
  if (zoomed_image == (Image *) NULL)
    return((Image *) NULL);
  /*
    Open image file.
  */
  (void) strcpy(zoomed_image->filename,image_info->filename);
  OpenImage(zoomed_image,ReadBinaryType);
  if (zoomed_image->file == (FILE *) NULL)
    PrematureExit("Unable to open file",zoomed_image)
  /*
    Determine width and height, e.g. 640x512.
  */
  width=512;
  height=512;
  x=0;
  if (image_info->size != (char *) NULL)
    (void) XParseGeometry(image_info->size,&x,&y,&width,&height);
  for (i=0; i < x; i++)
    (void) fgetc(zoomed_image->file);
  /*
    Read Y channel.
  */
  zoomed_image->columns=width >> 1;
  zoomed_image->rows=height >> 1;
  zoomed_image->packets=zoomed_image->columns*zoomed_image->rows;
  yuv_pixels=(unsigned char *)
    malloc((3*(width*height)/2)*sizeof(unsigned char));
  zoomed_image->pixels=(RunlengthPacket *)
    malloc(zoomed_image->packets*sizeof(RunlengthPacket));
  if ((yuv_pixels == (unsigned char *) NULL) ||
      (zoomed_image->pixels == (RunlengthPacket *) NULL))
    PrematureExit("Unable to allocate memory",zoomed_image);
  (void) ReadData((char *) yuv_pixels,1,3*(width*height)/2,zoomed_image->file);
  /*
    Initialize U channel.
  */
  p=yuv_pixels+(width*height);
  q=zoomed_image->pixels;
  for (i=0; i < zoomed_image->packets; i++)
  {
    q->green=UpScale(*p);
    q->index=0;
    q->length=0;
    p++;
    q++;
  }
  /*
    Initialize V channel.
  */
  ProgressMonitor(LoadImageText,100,400);
  q=zoomed_image->pixels;
  for (i=0; i < zoomed_image->packets; i++)
  {
    q->blue=UpScale(*p);
    p++;
    q++;
  }
  /*
    Scale image.
  */
  ProgressMonitor(LoadImageText,250,400);
  zoomed_image->orphan=True;
  image=MagnifyImage(zoomed_image);
  zoomed_image->orphan=False;
  CloseImage(zoomed_image);
  DestroyImage(zoomed_image);
  if (image == (Image *) NULL)
    PrematureExit("Unable to allocate memory",image);
  p=yuv_pixels;
  q=image->pixels;
  for (i=0; i < (image->columns*image->rows); i++)
  {
    q->red=UpScale(*p);
    p++;
    q++;
  }
  free((char *) yuv_pixels);
  TransformRGBImage(image,YCbCrColorspace);
  CompressImage(image);
  ProgressMonitor(LoadImageText,400,400);
  return(image);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%  R e a d Y U V 3 I m a g e                                                  %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Function ReadYUV3Image reads an image with digital YUV (CCIR 601 2:1:1)
%  bytes and returns it.  It allocates the memory necessary for the new Image
%  structure and returns a pointer to the new image.  This function differs
%  from ReadYUVImage in that it reads the Y, U, and V planes from separate
%  files (image.Y, image.U, and image.V).
%
%  The format of the ReadYUV3Image routine is:
%
%      image=ReadYUV3Image(image_info)
%
%  A description of each parameter follows:
%
%    o image:  Function ReadYUV3Image returns a pointer to the image after
%      reading.  A null image is returned if there is a a memory shortage or
%      if the image cannot be read.
%
%    o image_info: Specifies a pointer to an ImageInfo structure.
%
%
*/
static Image *ReadYUV3Image(ImageInfo *image_info)
{
  char
    filename[MaxTextLength];

  Image
    *image,
    *zoomed_image;

  int
    x,
    y;

  register int
    i;

  register RunlengthPacket
    *q;

  register unsigned char
    *p;

  unsigned char
    *uv_pixels,
    *y_pixels;

  unsigned int
    height,
    width;

  /*
    Allocate image structure.
  */
  zoomed_image=AllocateImage(image_info);
  if (zoomed_image == (Image *) NULL)
    return((Image *) NULL);
  /*
    Open image file.
  */
  (void) strcpy(filename,image_info->filename);
  (void) strcpy(zoomed_image->filename,filename);
  if (strcmp(zoomed_image->filename,"-") != 0)
    (void) strcat(zoomed_image->filename,".Y");
  OpenImage(zoomed_image,ReadBinaryType);
  if (zoomed_image->file == (FILE *) NULL)
    PrematureExit("Unable to open file",zoomed_image)
  /*
    Determine width and height, e.g. 640x512.
  */
  width=512;
  height=512;
  x=0;
  if (image_info->size != (char *) NULL)
    (void) XParseGeometry(image_info->size,&x,&y,&width,&height);
  for (i=0; i < x; i++)
    (void) fgetc(zoomed_image->file);
  /*
    Read Y channel.
  */
  zoomed_image->columns=width >> 1;
  zoomed_image->rows=height >> 1;
  zoomed_image->packets=zoomed_image->columns*zoomed_image->rows;
  uv_pixels=(unsigned char *)
    malloc(zoomed_image->packets*sizeof(unsigned char));
  y_pixels=(unsigned char *)
    malloc(4*zoomed_image->packets*sizeof(unsigned char));
  zoomed_image->pixels=(RunlengthPacket *)
    malloc(zoomed_image->packets*sizeof(RunlengthPacket));
  if ((uv_pixels == (unsigned char *) NULL) ||
      (y_pixels == (unsigned char *) NULL) ||
      (zoomed_image->pixels == (RunlengthPacket *) NULL))
    PrematureExit("Unable to allocate memory",zoomed_image);
  (void) ReadData((char *) y_pixels,4,zoomed_image->packets,zoomed_image->file);
  CloseImage(zoomed_image);
  /*
    Read U channel.
  */
  ProgressMonitor(LoadImageText,100,400);
  (void) strcpy(zoomed_image->filename,filename);
  if (strcmp(zoomed_image->filename,"-") != 0)
    (void) strcat(zoomed_image->filename,".U");
  OpenImage(zoomed_image,ReadBinaryType);
  if (zoomed_image->file == (FILE *) NULL)
    PrematureExit("Unable to open file",zoomed_image)
  (void) ReadData((char *) uv_pixels,1,zoomed_image->packets,
    zoomed_image->file);
  p=uv_pixels;
  q=zoomed_image->pixels;
  for (i=0; i < zoomed_image->packets; i++)
  {
    q->green=UpScale(*p);
    q->index=0;
    q->length=0;
    p++;
    q++;
  }
  CloseImage(zoomed_image);
  /*
    Read V channel.
  */
  ProgressMonitor(LoadImageText,200,400);
  (void) strcpy(zoomed_image->filename,filename);
  if (strcmp(zoomed_image->filename,"-") != 0)
    (void) strcat(zoomed_image->filename,".V");
  OpenImage(zoomed_image,ReadBinaryType);
  if (zoomed_image->file == (FILE *) NULL)
    PrematureExit("Unable to open file",zoomed_image)
  (void) ReadData((char *) uv_pixels,1,zoomed_image->packets,
    zoomed_image->file);
  p=uv_pixels;
  q=zoomed_image->pixels;
  for (i=0; i < zoomed_image->packets; i++)
  {
    q->blue=UpScale(*p);
    p++;
    q++;
  }
  CloseImage(zoomed_image);
  free((char *) uv_pixels);
  /*
    Scale image.
  */
  ProgressMonitor(LoadImageText,300,400);
  zoomed_image->orphan=True;
  image=MagnifyImage(zoomed_image);
  zoomed_image->orphan=False;
  CloseImage(zoomed_image);
  DestroyImage(zoomed_image);
  if (image == (Image *) NULL)
    PrematureExit("Unable to allocate memory",image);
  p=y_pixels;
  q=image->pixels;
  for (i=0; i < (image->columns*image->rows); i++)
  {
    q->red=UpScale(*p);
    p++;
    q++;
  }
  free((char *) y_pixels);
  TransformRGBImage(image,YCbCrColorspace);
  (void) strcpy(image->filename,filename);
  CompressImage(image);
  ProgressMonitor(LoadImageText,400,400);
  return(image);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   R e a d I m a g e                                                         %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Function ReadImage reads an image and returns it.  It allocates
%  the memory necessary for the new Image structure and returns a pointer to
%  the new image.  By default, the image format is determined by its magic
%  number. To specify a particular image format, precede the filename with an
%  explicit image format name and a colon (i.e.  ps:image) or as the filename
%  suffix  (i.e. image.ps).
%
%  The format of the ReadImage routine is:
%
%      image=ReadImage(image_info)
%
%  A description of each parameter follows:
%
%    o image: Function ReadImage returns a pointer to the image after
%      reading.  A null image is returned if there is a a memory shortage or
%      if the image cannot be read.
%
%    o image_info: Specifies a pointer to an ImageInfo structure.
%
%
*/
Image *ReadImage(ImageInfo *image_info)
{
  char
    magic_number[MaxTextLength];

  Image
    decode_image,
    *image;

  ImageInfo
    decode_info;

  register char
    *p;

  register int
    i;

  SetImageMagick(image_info);
  decode_info=(*image_info);
  decode_image.temporary=strcmp(decode_info.magick,"TMP") == 0;
  if ((strncmp(decode_info.magick,"FTP",3) == 0) ||
      (strncmp(decode_info.magick,"HTTP",4) == 0) ||
      (strncmp(decode_info.magick,"GOPHER",6) == 0))
    {
      char
        command[MaxTextLength],
        filename[MaxTextLength];

      /*
        Retrieve image as specified with a WWW uniform resource locator.
      */
      decode_image.temporary=True;
      TemporaryFilename(filename);
      (void) sprintf(command,WWWCommand,decode_info.magick,decode_info.filename,
        filename);
      (void) SystemCommand(command);
      SetImageMagick(&decode_info);
      (void) strcpy(decode_info.filename,filename);
    }
  if (!decode_info.assert || (strncmp(decode_info.magick,"SGI",3) == 0) ||
      (strncmp(decode_info.magick,"PCD",3) == 0))
    {
      /*
        Determine type from image magic number.
      */
      for (i=0 ; i < sizeof(magic_number); i++)
        magic_number[i]='\0';
      (void) strcpy(decode_image.filename,decode_info.filename);
      OpenImage(&decode_image,ReadBinaryType);
      if (decode_image.file != (FILE *) NULL)
        if ((decode_image.file == stdin) || decode_image.pipe)
          {
            FILE
              *file;

            int
              c;

            /*
              Copy standard input or pipe to temporary file.
            */
            decode_image.temporary=True;
            TemporaryFilename(decode_image.filename);
            decode_info.filename=decode_image.filename;
            file=fopen(decode_image.filename,WriteBinaryType);
            if (file == (FILE *) NULL)
              {
                Warning("Unable to write file",decode_info.filename);
                return((Image *) NULL);
              }
            c=fgetc(decode_image.file);
            while (c != EOF)
            {
              (void) putc(c,file);
              c=fgetc(decode_image.file);
            }
            (void) fclose(file);
            CloseImage(&decode_image);
            OpenImage(&decode_image,ReadBinaryType);
          }
      if (decode_image.file != (FILE *) NULL)
        {
          /*
            Read magic number.
          */
          (void) ReadData(magic_number,(unsigned int) sizeof(char),
            (unsigned int) sizeof(magic_number),decode_image.file);
          if (((unsigned char) magic_number[0] == 0xff) &&
              ((unsigned char) magic_number[1] == 0xff))
            {
              register int
                i;

              /*
                For PCD image type, skip to Byte 2048.
              */
              for (i=0; i < (int) (0x800-sizeof(magic_number)); i++)
                (void) fgetc(decode_image.file);
              (void) ReadData(magic_number,(unsigned int) sizeof(char),
                (unsigned int) sizeof(magic_number),decode_image.file);
            }
          CloseImage(&decode_image);
        }
      /*
        Determine the image format.
      */
      magic_number[MaxTextLength-1]='\0';
      if (strncmp(magic_number,"BM",2) == 0)
        (void) strcpy(decode_info.magick,"BMP");
      if (strncmp(magic_number,"IT0",3) == 0)
        (void) strcpy(decode_info.magick,"FITS");
      if (strncmp(magic_number,"SIMPLE",6) == 0)
        (void) strcpy(decode_info.magick,"FITS");
      if (strncmp(magic_number,"GIF8",4) == 0)
        (void) strcpy(decode_info.magick,"GIF");
      if (strncmp(magic_number,"\016\003\023\001",4) == 0)
        (void) strcpy(decode_info.magick,"HDF");

      /* Kobus */
      if (strncmp(magic_number,"HIPS",4) == 0)  {
        (void) strcpy(image_info->magick,"HIPS");
       }
      /* End Kobus */

      if (strncmp(magic_number,"\001\332",2) == 0)
        (void) strcpy(decode_info.magick,"SGI");
      if (strncmp(magic_number,"\377\330\377\340",4) == 0)
        (void) strcpy(decode_info.magick,"JPEG");
      if (strncmp(magic_number,"id=ImageMagick",14) == 0)
        (void) strcpy(decode_info.magick,"MIFF");
      if (strncmp(magic_number,"PCD_",4) == 0)
        (void) strcpy(decode_info.magick,"PCD");
      if ((unsigned char) *magic_number == 0x0a)
        (void) strcpy(decode_info.magick,"PCX");
      if (strncmp(magic_number,"%!PDF",5) == 0)
        (void) strcpy(decode_info.magick,"PDF");
      if ((*magic_number == 'P') && isdigit(magic_number[1]))
        (void) strcpy(decode_info.magick,"PNM");
      if (strncmp(magic_number,"\211PNG\r\n\032\n",8) == 0)
        (void) strcpy(decode_info.magick,"PNG");
      if (strncmp(magic_number,"%!",2) == 0)
        (void) strcpy(decode_info.magick,"PS");
      if (strncmp(magic_number,"#?RADIANCE",10) == 0)
        (void) strcpy(decode_info.magick,"RAD");
      if (strncmp(magic_number,"\122\314",2) == 0)
        (void) strcpy(decode_info.magick,"RLE");
      if (strncmp(magic_number,"\131\246\152\225",4) == 0)
        (void) strcpy(decode_info.magick,"SUN");
      if ((strncmp(magic_number,"\115\115\000\052",4) == 0) ||
          (strncmp(magic_number,"\111\111\052\000",4) == 0))
        (void) strcpy(decode_info.magick,"TIFF");
      if ((strncmp(magic_number,"LBLSIZE",7) == 0) ||
          (strncmp(magic_number,"NJPL1I",6) == 0))
        (void) strcpy(decode_info.magick,"VICAR");
      if ((unsigned char) *magic_number == 0xab)
        (void) strcpy(decode_info.magick,"VIFF");
      p=strchr(magic_number,'#');
      if (p != (char *) NULL)
        if (strncmp(p,"#define",7) == 0)
          (void) strcpy(decode_info.magick,"XBM");
      if (strncmp(magic_number,"/* XPM */",9) == 0)
        (void) strcpy(decode_info.magick,"XPM");
      if ((magic_number[1] == 0x00) && (magic_number[2] == 0x00))
        if ((magic_number[5] == 0x00) && (magic_number[6] == 0x00))
          if ((magic_number[4] == 0x07) || (magic_number[7] == 0x07))
            (void) strcpy(decode_info.magick,"XWD");
    }
  /*
    Call appropriate image reader based on image type.
  */

  switch (*decode_info.magick)
  {
    case 'A':
    {
      if (strcmp(decode_info.magick,"AVS") == 0)
        {
          image=ReadAVSImage(&decode_info);
          break;
        }
      image=ReadMIFFImage(&decode_info);
      break;
    }
    case 'B':
    {
      if (strcmp(decode_info.magick,"BIE") == 0)
        {
          image=ReadJBIGImage(&decode_info);
          break;
        }
      if (strcmp(decode_info.magick,"BMP") == 0)
        {
          image=ReadBMPImage(&decode_info);
          break;
        }
      image=ReadMIFFImage(&decode_info);
      break;
    }
    case 'C':
    {
      if (strcmp(decode_info.magick,"CMYK") == 0)
        {
          image=ReadCMYKImage(&decode_info);
          break;
        }
      image=ReadMIFFImage(&decode_info);
      break;
    }
    case 'E':
    {
      if (strncmp(decode_info.magick,"EPS",3) == 0)
        {
          image=ReadPSImage(&decode_info);
          break;
        }
      image=ReadMIFFImage(&decode_info);
      break;
    }
    case 'F':
    {
      if (strcmp(decode_info.magick,"FAX") == 0)
        {
          image=ReadFAXImage(&decode_info);
          break;
        }
      if (strcmp(decode_info.magick,"FITS") == 0)
        {
          image=ReadFITSImage(&decode_info);
          break;
        }
      image=ReadMIFFImage(&decode_info);
      break;
    }
    case 'G':
    {
      if (strncmp(decode_info.magick,"GIF",3) == 0)
        {
          image=ReadGIFImage(&decode_info);
          break;
        }
      if (strcmp(decode_info.magick,"GRAY") == 0)
        {
          image=ReadGRAYImage(&decode_info);
          break;
        }
      image=ReadMIFFImage(&decode_info);
      break;
    }
    case 'H':
    {
      if (strcmp(decode_info.magick,"HDF") == 0)
        {
          image=ReadHDFImage(&decode_info);
          break;
        }

      /* Kobus */
      else if (strcmp(image_info->magick,"HIPS") == 0) {
          image=ReadHIPSImage(image_info);
          break;
	 }
      /* End Kobus*/

      if (strncmp(decode_info.magick,"HISTOGRAM",4) == 0)
        {
          image=ReadHISTOGRAMImage(&decode_info);
          break;
        }
      if (strcmp(decode_info.magick,"HTML") == 0)
        {
          image=ReadHTMLImage(&decode_info);
          break;
        }
      image=ReadMIFFImage(&decode_info);
      break;
    }
    case 'J':
    {
      if (strcmp(decode_info.magick,"JBIG") == 0)
        {
          image=ReadJBIGImage(&decode_info);
          break;
        }
      /* Kobus: Added "JPG" possibility" */
      if (    (strcmp(decode_info.magick,"JPEG") == 0)
           || (strcmp(decode_info.magick,"JPG") == 0)
         )
        {
          image=ReadJPEGImage(&decode_info);
          break;
        }
      image=ReadMIFFImage(&decode_info);
      break;
    }
    case 'L':
    {
      if (strcmp(decode_info.magick,"LOGO") == 0)
        {
          image=ReadLOGOImage(&decode_info);
          break;
        }
      image=ReadMIFFImage(&decode_info);
      break;
    }
    case 'M':
    {
      if (strcmp(decode_info.magick,"MAP") == 0)
        {
          image=ReadMAPImage(&decode_info);
          break;
        }
      if (strcmp(decode_info.magick,"MATTE") == 0)
        {
          image=ReadMATTEImage(&decode_info);
          break;
        }
      if ((strcmp(decode_info.magick,"MPEG") == 0) ||
          (strcmp(decode_info.magick,"MPG") == 0))
        {
          image=ReadMPEGImage(&decode_info);
          break;
        }
      if (strcmp(decode_info.magick,"MTV") == 0)
        {
          image=ReadMTVImage(&decode_info);
          break;
        }
      image=ReadMIFFImage(&decode_info);
      break;
    }
    case 'N':
    {
      if (strcmp(decode_info.magick,"NULL") == 0)
        {
          image=ReadNULLImage(&decode_info);
          break;
        }
      image=ReadMIFFImage(&decode_info);
      break;
    }
    case 'P':
    {
      if ((strcmp(decode_info.magick,"PBM") == 0) ||
          (strcmp(decode_info.magick,"PGM") == 0) ||
          (strcmp(decode_info.magick,"PNM") == 0) ||
          (strcmp(decode_info.magick,"PPM") == 0))
        {
          image=ReadPNMImage(&decode_info);
          break;
        }
      if (strcmp(decode_info.magick,"PCD") == 0)
        {
          image=ReadPCDImage(&decode_info);
          break;
        }
      if (strcmp(decode_info.magick,"PCX") == 0)
        {
          image=ReadPCXImage(&decode_info);
          break;
        }
      if (strcmp(decode_info.magick,"PDF") == 0)
        {
          image=ReadPDFImage(&decode_info);
          break;
        }
      if (strcmp(decode_info.magick,"PICT") == 0)
        {
          image=ReadPICTImage(&decode_info);
          break;
        }
      if (strcmp(decode_info.magick,"PCD") == 0)
        {
          image=ReadPCDImage(&decode_info);
          break;
        }
      if (strcmp(decode_info.magick,"PM") == 0)
        {
          image=ReadXPMImage(&decode_info);
          break;
        }
      if (strcmp(decode_info.magick,"PNG") == 0)
        {
          image=ReadPNGImage(&decode_info);
          break;
        }
      if (strncmp(decode_info.magick,"PS",2) == 0)
        {
          image=ReadPSImage(&decode_info);
          break;
        }
      image=ReadMIFFImage(&decode_info);
      break;
    }
    case 'R':
    {
      if (strcmp(decode_info.magick,"RAD") == 0)
        {
          image=ReadRADIANCEImage(&decode_info);
          break;
        }
      if (strcmp(decode_info.magick,"RAS") == 0)
        {
          image=ReadSUNImage(&decode_info);
          break;
        }
      if (strncmp(decode_info.magick,"RGB",3) == 0)
        {
          image=ReadRGBImage(&decode_info);
          break;
        }
      if (strcmp(decode_info.magick,"RLE") == 0)
        {
          image=ReadRLEImage(&decode_info);
          break;
        }
      image=ReadMIFFImage(&decode_info);
      break;
    }
    case 'S':
    {
      if (strcmp(decode_info.magick,"SGI") == 0)
        {
          image=ReadSGIImage(&decode_info);
          break;
        }
      if (strcmp(decode_info.magick,"SUN") == 0)
        {
          image=ReadSUNImage(&decode_info);
          break;
        }
      image=ReadMIFFImage(&decode_info);
      break;
    }
    case 'T':
    {
      if (strcmp(decode_info.magick,"TEXT") == 0)
        {
          image=ReadTEXTImage(&decode_info);
          break;
        }
      if (strcmp(decode_info.magick,"TGA") == 0)
        {
          image=ReadTARGAImage(&decode_info);
          break;
        }
      if (strncmp(decode_info.magick,"TIF",3) == 0)
        {
          image=ReadTIFFImage(&decode_info);
          break;
        }
      if (strcmp(decode_info.magick,"TILE") == 0)
        {
          image=ReadTILEImage(&decode_info);
          break;
        }
      image=ReadMIFFImage(&decode_info);
      break;
    }
    case 'U':
    {
      if (strcmp(decode_info.magick,"UYVY") == 0)
        {
          image=ReadUYVYImage(&decode_info);
          break;
        }
      image=ReadMIFFImage(&decode_info);
      break;
    }
    case 'V':
    {
      if (strcmp(decode_info.magick,"VICAR") == 0)
        {
          image=ReadVICARImage(&decode_info);
          break;
        }
      if (strcmp(decode_info.magick,"VID") == 0)
        {
          if (decode_info.assert)
            image=ReadVIDImage(&decode_info);
          else
            image=ReadMIFFImage(&decode_info);
          break;
        }
      if (strcmp(decode_info.magick,"VIFF") == 0)
        {
          image=ReadVIFFImage(&decode_info);
          break;
        }
      image=ReadMIFFImage(&decode_info);
      break;
    }
    case 'X':
    {
      if (strcmp(decode_info.magick,"X") == 0)
        {
          image=ReadXImage(&decode_info,False,False,False,False);
          break;
        }
      if (strcmp(decode_info.magick,"XC") == 0)
        {
          image=ReadXCImage(&decode_info);
          break;
        }
      if (strcmp(decode_info.magick,"XBM") == 0)
        {
          image=ReadXBMImage(&decode_info);
          break;
        }
      if (strcmp(decode_info.magick,"XPM") == 0)
        {
          image=ReadXPMImage(&decode_info);
          break;
        }
      if (strcmp(decode_info.magick,"XV") == 0)
        {
          image=ReadVIFFImage(&decode_info);
          break;
        }
      if (strcmp(decode_info.magick,"XWD") == 0)
        {
          image=ReadXWDImage(&decode_info);
          break;
        }
      image=ReadMIFFImage(&decode_info);
      break;
    }
    case 'Y':
    {
      if (strcmp(decode_info.magick,"YUV") == 0)
        {
          image=ReadYUVImage(&decode_info);
          break;
        }
      if (strcmp(decode_info.magick,"YUV3") == 0)
        {
          image=ReadYUV3Image(&decode_info);
          break;
        }
      image=ReadMIFFImage(&decode_info);
      break;
    }
    default:
    {
      (void) strcpy(decode_info.magick,"MIFF");
      image=ReadMIFFImage(&decode_info);
    }
  }

end_of_switch:

  if (decode_image.temporary)
    (void) unlink(decode_info.filename);
  if (image != (Image *) NULL)
    {
      if (image->status)
        Warning("An error has occurred reading from file",image->filename);
      if (decode_image.temporary)
        (void) strcpy(image->filename,image_info->filename);
      if (image->comments == (char *) NULL)
        CommentImage(image,"  Imported from %m image: %f");
      (void) strcpy(image->magick_filename,image_info->filename);
      if (image->magick_columns == 0)
        image->magick_columns=image->columns;
      if (image->magick_rows == 0)
        image->magick_rows=image->rows;
      image_info->interlace=decode_info.interlace;
    }
  return(image);
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
%   R e a d H I P S I m a g e                                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Function ReadHIPSImage reads a HIPS image file and returns it.  It
%  allocates the memory necessary for the new Image structure and returns a
%  pointer to the new image.
%
%  The format of the ReadHIPSImage routine is:
%
%      image=ReadHIPSImage(image_info)
%
%  A description of each parameter follows:
%
%    o image: Function ReadHIPSImage returns a pointer to the image after
%      reading.  A null image is returned if there is a a memory shortage or
%      if the image cannot be read.
%
%    o image_info: Specifies a pointer to an ImageInfo structure.
%
%
*/

#ifdef HasHIPS
#include "hips/hipl_format.h"

static Image *ReadHIPSImage(image_info)
ImageInfo
  *image_info;
{
  char
    keyword[MaxTextLength],
    value[MaxTextLength];

  Image
    *image;

  register int
    c,
    i;

  register unsigned char
    *p;

  unsigned int
    length,
    packet_size,
    status;

  unsigned long
    count,
    packets;

  int save_hipserrlev, save_hipserrprt;
  int method, fr, f;
  struct header hd, hdp;
  int types[2];
  int color_image, num_bytes;
  unsigned char *hips_image_pos, *display_image_pos;


  Progname = strsave("Imagemagick");

  /*
    Allocate image structure.
  */
  image=AllocateImage(image_info);
  if (image == (Image *) NULL)
    return((Image *) NULL);
  /*
    Open image file.
  */
  OpenImage(image,"rb");
  if (image->file == (FILE *) NULL)
    {
      Warning("Unable to open file",image->filename);
      DestroyImage(image);
      return((Image *) NULL);
    }

  rewind(image->file);

  fread_hdr_a(image->file, &hd, image->filename);

  save_hipserrlev = hipserrlev;
  save_hipserrprt = hipserrprt;
  hipserrlev = hipserrprt = 10000;   /* Big Number */

  types[0] = PFRGB;
  types[1] = LASTTYPE;

  method = fset_conversion(&hd, &hdp, types, image->filename);

  if (method != HIPS_ERROR) {
      fread_imagec(image->file, &hd, &hdp, method, 0, image->filename);
      color_image = TRUE;
     }
  else {
      types[0] = PFBYTE;
      types[1] = LASTTYPE;

      method = fset_conversion(&hd, &hdp, types, image->filename);

      if (method != HIPS_ERROR) {
          fread_imagec(image->file, &hd, &hdp, method, 0, image->filename);
          color_image = FALSE;
         }
      else {
          Warning("Unable to convert HIPS file", image->filename);
          DestroyImage(image);
          return((Image *) NULL);
	 }
     }

  hipserrlev = save_hipserrlev;
  hipserrprt = save_hipserrprt;

  image->columns=(unsigned int) hd.cols;
  image->rows=(unsigned int) hd.rows;
  packet_size = image->packet_size=3;
  packets = image->packets=image->columns*image->rows;


    num_bytes = packets*packet_size*sizeof(unsigned char);

    image->packed_pixels=(unsigned char *) malloc((unsigned int) num_bytes);

    if (image->packed_pixels == (unsigned char *) NULL)
      {
        Warning("Unable to read image","Memory allocation failed");
        DestroyImages(image);
        return((Image *) NULL);
      }

    if (color_image == TRUE) {
        memcpy(image->packed_pixels, hdp.image, num_bytes);
       }
    else {
	hips_image_pos =  (unsigned char*) hdp.image;
	display_image_pos = (unsigned char*) (image->packed_pixels);

	for (i=0; i<packets; i++) {
	    *display_image_pos = *hips_image_pos;
	    display_image_pos++;
	    *display_image_pos = *hips_image_pos;
	    display_image_pos++;
	    *display_image_pos = *hips_image_pos;
	    display_image_pos++;
	    hips_image_pos++;
	   }
       }
    /*
      Unpack the packed image pixels into runlength-encoded pixel packets.
    */

    image -> class = DirectClass;
    image -> compression = NoCompression;

    status=RunlengthDecodeImage(image);

    if (status == False)
      {
        DestroyImages(image);
        return((Image *) NULL);
      }

  CloseImage(image);
  return(image);
}
#else
static Image *ReadHIPSImage(ImageInfo *image_info)
{
  Warning("HIPS format is not available",image_info->filename);
  return(ReadMIFFImage(image_info));
}
#endif

/*  End Kobus's changes */

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

#ifdef __cplusplus
}
#endif


/* -------------------------------------------------------------------------- */

#endif   /* #ifdef KJB_HAVE_X11  */

#endif   /* #ifndef __C2MAN__  */

