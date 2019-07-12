
/* $Id: compress.c 4727 2009-11-16 20:53:54Z kobus $ */

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
%           CCCC   OOO   M   M  PPPP   RRRR   EEEEE   SSSSS  SSSSS            %
%          C      O   O  MM MM  P   P  R   R  E       SS     SS               %
%          C      O   O  M M M  PPPP   RRRR   EEE      SSS    SSS             %
%          C      O   O  M   M  P      R R    E          SS     SS            %
%           CCCC   OOO   M   M  P      R  R   EEEEE   SSSSS  SSSSS            %
%                                                                             %
%                                                                             %
%                  Image Compression/Decompression Coders                     %
%                                                                             %
%                                                                             %
%                                                                             %
%                           Software Design                                   %
%                             John Cristy                                     %
%                              May  1993                                      %
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
%  in an action of contract, negligence or other tortious action, arising     %
%  out of or in connection with the use or performance of this software.      %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%
%
*/

/*
  Include declarations.
*/
#include "magick/magick.h"

#ifdef __cplusplus
extern "C" {
#endif

/* -------------------------------------------------------------------------- */


/*
  Define declarations.
*/
#define LoadImageText  "  Loading image...  "
#define SaveImageText  "  Saving image...  "

/*
  Typedef declarations.
*/
typedef struct HuffmanTable
{
  int
    id,
    code,
    length,
    count;
} HuffmanTable;

typedef struct _ScanlinePacket
{
  unsigned char
    pixel;

  int
    state;
} ScanlinePacket;

/*
  Huffman coding declarations.
*/
#define TWId  23
#define MWId  24
#define TBId  25
#define MBId  26
#define EXId  27

static HuffmanTable
  MBTable[]=
  {
    { MBId, 0x0f, 10, 64 }, { MBId, 0xc8, 12, 128 },
    { MBId, 0xc9, 12, 192 }, { MBId, 0x5b, 12, 256 },
    { MBId, 0x33, 12, 320 }, { MBId, 0x34, 12, 384 },
    { MBId, 0x35, 12, 448 }, { MBId, 0x6c, 13, 512 },
    { MBId, 0x6d, 13, 576 }, { MBId, 0x4a, 13, 640 },
    { MBId, 0x4b, 13, 704 }, { MBId, 0x4c, 13, 768 },
    { MBId, 0x4d, 13, 832 }, { MBId, 0x72, 13, 896 },
    { MBId, 0x73, 13, 960 }, { MBId, 0x74, 13, 1024 },
    { MBId, 0x75, 13, 1088 }, { MBId, 0x76, 13, 1152 },
    { MBId, 0x77, 13, 1216 }, { MBId, 0x52, 13, 1280 },
    { MBId, 0x53, 13, 1344 }, { MBId, 0x54, 13, 1408 },
    { MBId, 0x55, 13, 1472 }, { MBId, 0x5a, 13, 1536 },
    { MBId, 0x5b, 13, 1600 }, { MBId, 0x64, 13, 1664 },
    { MBId, 0x65, 13, 1728 }, { MBId, 0x00, 0, 0 }
  };

static HuffmanTable
  EXTable[]=
  {
    { EXId, 0x08, 11, 1792 }, { EXId, 0x0c, 11, 1856 },
    { EXId, 0x0d, 11, 1920 }, { EXId, 0x12, 12, 1984 },
    { EXId, 0x13, 12, 2048 }, { EXId, 0x14, 12, 2112 },
    { EXId, 0x15, 12, 2176 }, { EXId, 0x16, 12, 2240 },
    { EXId, 0x17, 12, 2304 }, { EXId, 0x1c, 12, 2368 },
    { EXId, 0x1d, 12, 2432 }, { EXId, 0x1e, 12, 2496 },
    { EXId, 0x1f, 12, 2560 }, { EXId, 0x00, 0, 0 }
  };

static HuffmanTable
  MWTable[]=
  {
    { MWId, 0x1b, 5, 64 }, { MWId, 0x12, 5, 128 },
    { MWId, 0x17, 6, 192 }, { MWId, 0x37, 7, 256 },
    { MWId, 0x36, 8, 320 }, { MWId, 0x37, 8, 384 },
    { MWId, 0x64, 8, 448 }, { MWId, 0x65, 8, 512 },
    { MWId, 0x68, 8, 576 }, { MWId, 0x67, 8, 640 },
    { MWId, 0xcc, 9, 704 }, { MWId, 0xcd, 9, 768 },
    { MWId, 0xd2, 9, 832 }, { MWId, 0xd3, 9, 896 },
    { MWId, 0xd4, 9, 960 }, { MWId, 0xd5, 9, 1024 },
    { MWId, 0xd6, 9, 1088 }, { MWId, 0xd7, 9, 1152 },
    { MWId, 0xd8, 9, 1216 }, { MWId, 0xd9, 9, 1280 },
    { MWId, 0xda, 9, 1344 }, { MWId, 0xdb, 9, 1408 },
    { MWId, 0x98, 9, 1472 }, { MWId, 0x99, 9, 1536 },
    { MWId, 0x9a, 9, 1600 }, { MWId, 0x18, 6, 1664 },
    { MWId, 0x9b, 9, 1728 }, { MWId, 0x00, 0, 0 }
  };

static HuffmanTable
  TBTable[]=
  {
    { TBId, 0x37, 10, 0 }, { TBId, 0x02, 3, 1 }, { TBId, 0x03, 2, 2 },
    { TBId, 0x02, 2, 3 }, { TBId, 0x03, 3, 4 }, { TBId, 0x03, 4, 5 },
    { TBId, 0x02, 4, 6 }, { TBId, 0x03, 5, 7 }, { TBId, 0x05, 6, 8 },
    { TBId, 0x04, 6, 9 }, { TBId, 0x04, 7, 10 }, { TBId, 0x05, 7, 11 },
    { TBId, 0x07, 7, 12 }, { TBId, 0x04, 8, 13 }, { TBId, 0x07, 8, 14 },
    { TBId, 0x18, 9, 15 }, { TBId, 0x17, 10, 16 }, { TBId, 0x18, 10, 17 },
    { TBId, 0x08, 10, 18 }, { TBId, 0x67, 11, 19 }, { TBId, 0x68, 11, 20 },
    { TBId, 0x6c, 11, 21 }, { TBId, 0x37, 11, 22 }, { TBId, 0x28, 11, 23 },
    { TBId, 0x17, 11, 24 }, { TBId, 0x18, 11, 25 }, { TBId, 0xca, 12, 26 },
    { TBId, 0xcb, 12, 27 }, { TBId, 0xcc, 12, 28 }, { TBId, 0xcd, 12, 29 },
    { TBId, 0x68, 12, 30 }, { TBId, 0x69, 12, 31 }, { TBId, 0x6a, 12, 32 },
    { TBId, 0x6b, 12, 33 }, { TBId, 0xd2, 12, 34 }, { TBId, 0xd3, 12, 35 },
    { TBId, 0xd4, 12, 36 }, { TBId, 0xd5, 12, 37 }, { TBId, 0xd6, 12, 38 },
    { TBId, 0xd7, 12, 39 }, { TBId, 0x6c, 12, 40 }, { TBId, 0x6d, 12, 41 },
    { TBId, 0xda, 12, 42 }, { TBId, 0xdb, 12, 43 }, { TBId, 0x54, 12, 44 },
    { TBId, 0x55, 12, 45 }, { TBId, 0x56, 12, 46 }, { TBId, 0x57, 12, 47 },
    { TBId, 0x64, 12, 48 }, { TBId, 0x65, 12, 49 }, { TBId, 0x52, 12, 50 },
    { TBId, 0x53, 12, 51 }, { TBId, 0x24, 12, 52 }, { TBId, 0x37, 12, 53 },
    { TBId, 0x38, 12, 54 }, { TBId, 0x27, 12, 55 }, { TBId, 0x28, 12, 56 },
    { TBId, 0x58, 12, 57 }, { TBId, 0x59, 12, 58 }, { TBId, 0x2b, 12, 59 },
    { TBId, 0x2c, 12, 60 }, { TBId, 0x5a, 12, 61 }, { TBId, 0x66, 12, 62 },
    { TBId, 0x67, 12, 63 }, { TBId, 0x00, 0, 0 }
  };

static HuffmanTable
  TWTable[]=
  {
    { TWId, 0x35, 8, 0 }, { TWId, 0x07, 6, 1 }, { TWId, 0x07, 4, 2 },
    { TWId, 0x08, 4, 3 }, { TWId, 0x0b, 4, 4 }, { TWId, 0x0c, 4, 5 },
    { TWId, 0x0e, 4, 6 }, { TWId, 0x0f, 4, 7 }, { TWId, 0x13, 5, 8 },
    { TWId, 0x14, 5, 9 }, { TWId, 0x07, 5, 10 }, { TWId, 0x08, 5, 11 },
    { TWId, 0x08, 6, 12 }, { TWId, 0x03, 6, 13 }, { TWId, 0x34, 6, 14 },
    { TWId, 0x35, 6, 15 }, { TWId, 0x2a, 6, 16 }, { TWId, 0x2b, 6, 17 },
    { TWId, 0x27, 7, 18 }, { TWId, 0x0c, 7, 19 }, { TWId, 0x08, 7, 20 },
    { TWId, 0x17, 7, 21 }, { TWId, 0x03, 7, 22 }, { TWId, 0x04, 7, 23 },
    { TWId, 0x28, 7, 24 }, { TWId, 0x2b, 7, 25 }, { TWId, 0x13, 7, 26 },
    { TWId, 0x24, 7, 27 }, { TWId, 0x18, 7, 28 }, { TWId, 0x02, 8, 29 },
    { TWId, 0x03, 8, 30 }, { TWId, 0x1a, 8, 31 }, { TWId, 0x1b, 8, 32 },
    { TWId, 0x12, 8, 33 }, { TWId, 0x13, 8, 34 }, { TWId, 0x14, 8, 35 },
    { TWId, 0x15, 8, 36 }, { TWId, 0x16, 8, 37 }, { TWId, 0x17, 8, 38 },
    { TWId, 0x28, 8, 39 }, { TWId, 0x29, 8, 40 }, { TWId, 0x2a, 8, 41 },
    { TWId, 0x2b, 8, 42 }, { TWId, 0x2c, 8, 43 }, { TWId, 0x2d, 8, 44 },
    { TWId, 0x04, 8, 45 }, { TWId, 0x05, 8, 46 }, { TWId, 0x0a, 8, 47 },
    { TWId, 0x0b, 8, 48 }, { TWId, 0x52, 8, 49 }, { TWId, 0x53, 8, 50 },
    { TWId, 0x54, 8, 51 }, { TWId, 0x55, 8, 52 }, { TWId, 0x24, 8, 53 },
    { TWId, 0x25, 8, 54 }, { TWId, 0x58, 8, 55 }, { TWId, 0x59, 8, 56 },
    { TWId, 0x5a, 8, 57 }, { TWId, 0x5b, 8, 58 }, { TWId, 0x4a, 8, 59 },
    { TWId, 0x4b, 8, 60 }, { TWId, 0x32, 8, 61 }, { TWId, 0x33, 8, 62 },
    { TWId, 0x34, 8, 63 }, { TWId, 0x00, 0, 0 }
  };

/*
  Function prototypes.
*/
static char
  *Ascii85Tuple _Declare((unsigned char *));

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   A S C I I 8 5 E n c o d e                                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Function ASCII85Encode encodes data in ASCII base-85 format.  ASCII base-85
%  encoding produces five ASCII printing characters from every four bytes of
%  binary data.
%
%  The format of the ASCII85Encode routine is:
%
%      ASCII85Encode(code,file)
%
%  A description of each parameter follows:
%
%    o code: a binary unsigned char to encode to ASCII 85.
%
%    o file: write the encoded ASCII character to this file.
%
%
*/
#define MaxLineLength  36

static int
  count,
  line_break;

static unsigned char
  ascii85_buffer[10];

static char *Ascii85Tuple(unsigned char *data)
{
  static char
    tuple[6];

  register unsigned int
    word,
    x;

  register unsigned short
    y;

  word=(((data[0] << 8) | data[1]) << 16) | (data[2] << 8) | data[3];
  if (word == 0L)
    {
      tuple[0]='z';
      tuple[1]='\0';
      return(tuple);
    }
  x=word/(85L*85*85*85);
  tuple[0]=x+'!';
  word-=x*(85L*85*85*85);
  x=word/(85L*85*85);
  tuple[1]=x+'!';
  word-=x*(85L*85*85);
  x=word/(85*85);
  tuple[2]=x+'!';
  y=(unsigned short) (word-x*(85L*85));
  tuple[3]=(y/85)+'!';
  tuple[4]=(y % 85)+'!';
  tuple[5]='\0';
  return(tuple);
}

void Ascii85Initialize(void)
{
  line_break=MaxLineLength << 1;
  count=0;
}

void Ascii85Flush(FILE *file)
{
  register char
    *tuple;

  if (count > 0)
    {
      ascii85_buffer[count]=0;
      ascii85_buffer[count+1]=0;
      ascii85_buffer[count+2]=0;
      tuple=Ascii85Tuple(ascii85_buffer);
      (void) fputs(*tuple == 'z' ? "!!!!" : tuple,file);
    }
  (void) fputc('~',file);
  (void) fputc('>',file);
}

void Ascii85Encode(unsigned int code, FILE *file)
{
  int
    n;

  register char
    *q;

  register unsigned char
    *p;

  ascii85_buffer[count]=code;
  count++;
  if (count < 4)
    return;
  p=ascii85_buffer;
  for (n=count; n >= 4; n-=4)
  {
    for (q=Ascii85Tuple(p); *q; q++)
    {
      (void) fputc(*q,file);
      line_break--;
      if (line_break == 0)
        {
          (void) fputc('\n',file);
          line_break=2*MaxLineLength;
        }
    }
    p+=8;
  }
  count=n;
  p-=4;
  for (n=0; n < 4; n++)
    ascii85_buffer[n]=(*p++);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   B M P D e c o d e I m a g e                                               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Function BMPDecodeImage unpacks the packed image pixels into
%  runlength-encoded pixel packets.
%
%  The format of the BMPDecodeImage routine is:
%
%      status=BMPDecodeImage(compressed_pixels,pixels,compression,
%        number_columns,number_rows)
%
%  A description of each parameter follows:
%
%    o status:  Function BMPDecodeImage returns True if all the pixels are
%      uncompressed without error, otherwise False.
%
%    o compressed_pixels:  The address of a Byte (8 bits) array of compressed
%      pixel data.
%
%    o pixels:  The address of a Byte (8 bits) array of pixel data created by
%      the decoding process.
%
%    o compression:  A value of 1 means the compressed pixels are runlength
%      encoded for a 256-color bitmap.  A value of 2 means a 16-color bitmap.
%
%    o number_columns:  An integer value that is the number of columns or
%      width in pixels of your source image.
%
%    o number_rows:  An integer value that is the number of rows or
%      heigth in pixels of your source image.
%
%
*/
unsigned int BMPDecodeImage(unsigned char *compressed_pixels, unsigned char *pixels, unsigned int compression, unsigned int number_columns, unsigned int number_rows)
{
  register int
    i,
    x,
    y;

  register unsigned char
    *p,
    *q;

  unsigned char
    Byte,
    count;

  p=compressed_pixels;
  q=pixels;
  x=0;
  for (y=0; y < number_rows; )
  {
    count=(*p++);
    if (count != 0)
      {
        /*
          Encoded mode.
        */
        Byte=(*p++);
        for (i=0; i < (int) count; i++)
        {
          if (compression == 1)
            *q++=Byte;
          else
            *q++=(i & 0x01) ? (Byte & 0x0f) : ((Byte >> 4) & 0x0f);
          x++;
        }
      }
    else
      {
        /*
          Escape mode.
        */
        count=(*p++);
        if (count == 0x01)
          return(True);
        switch (count)
        {
          case 0x00:
          {
            /*
              End of line.
            */
            x=0;
            y++;
            q=pixels+y*number_columns;
            break;
          }
          case 0x02:
          {
            /*
              Delta mode.
            */
            x+=(*p++);
            y+=(*p++);
            q=pixels+y*number_columns+x;
            break;
          }
          default:
          {
            /*
              Absolute mode.
            */
            for (i=0; i < (int) count; i++)
            {
              if (compression == 1)
                *q++=(*p++);
              else
                {
                  if ((i & 0x01) == 0)
                    Byte=(*p++);
                  *q++=(i & 0x01) ? (Byte & 0x0f) : ((Byte >> 4) & 0x0f);
                }
              x++;
            }
            /*
              Read pad Byte.
            */
            if (compression == 1)
              {
                if (count & 0x01)
                  p++;
              }
            else
              if (((count & 0x03) == 1) || ((count & 0x03) == 2))
                p++;
            break;
          }
        }
      }
    ProgressMonitor(LoadImageText,y,number_rows);
  }
  return(False);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   B M P E n c o d e I m a g e                                               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Function BMPEncodeImage compresses pixels using a runlength encoded format.
%
%  The format of the BMPEncodeImage routine is:
%
%      status=BMPEncodeImage(pixels,compressed_pixels,number_columns,
%        number_rows)
%
%  A description of each parameter follows:
%
%    o status:  Function BMPEncodeImage returns the number of bytes in the
%      runlength encoded compress_pixels array.
%
%    o pixels:  The address of a Byte (8 bits) array of pixel data created by
%      the compression process.
%
%    o compressed_pixels:  The address of a Byte (8 bits) array of compressed
%      pixel data.
%
%    o number_columns:  An integer value that is the number of columns or
%      width in pixels of your source image.
%
%    o number_rows:  An integer value that is the number of rows or
%      heigth in pixels of your source image.
%
%
*/
unsigned int BMPEncodeImage(unsigned char *pixels, unsigned char *compressed_pixels, unsigned int number_columns, unsigned int number_rows)
{
  register int
    i,
    x,
    y;

  register unsigned char
    *p,
    *q;

  /*
    Runlength encode pixels.
  */
  p=pixels;
  q=compressed_pixels;
  i=0;
  for (y=0; y < number_rows; y++)
  {
    for (x=0; x < number_columns; x+=i)
    {
      /*
        Determine runlength.
      */
      for (i=1; ((x+i) < number_columns); i++)
        if ((*(p+i) != *p) || (i == 255))
          break;
      *q++=i;
      *q++=(*p);
      p+=i;
    }
    /*
      End of line.
    */
    *q++=0;
    *q++=0x00;
    ProgressMonitor(SaveImageText,y,number_rows);
  }
  /*
    End of bitmap.
  */
  *q++=0;
  *q++=0x01;
  return(q-compressed_pixels);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   G I F D e c o d e I m a g e                                               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Function GIFDecodeImage uncompresses an image via GIF-coding.
%
%  The format of the GIFDecodeImage routine is:
%
%      status=GIFDecodeImage(image)
%
%  A description of each parameter follows:
%
%    o status:  Function GIFDecodeImage returns True if all the pixels are
%      uncompressed without error, otherwise False.
%
%    o image: The address of a structure of type Image.
%
%
*/
unsigned int GIFDecodeImage(Image *image)
{
#define MaxStackSize  4096
#define NullCode  (-1)

  int
    available,
    clear,
    code_mask,
    code_size,
    end_of_information,
    in_code,
    old_code,
    status;

  register int
    bits,
    code,
    count,
    i;

  register RunlengthPacket
    *p;

  register unsigned char
    *c;

  register unsigned int
    datum;

  short
    *prefix;

  unsigned char
    data_size,
    first,
    *packet,
    *pixel_stack,
    *suffix,
    *top_stack;

  /*
    Allocate decoder tables.
  */
  packet=(unsigned char *) malloc(256*sizeof(unsigned char));
  prefix=(short *) malloc(MaxStackSize*sizeof(short));
  suffix=(unsigned char *) malloc(MaxStackSize*sizeof(unsigned char));
  pixel_stack=(unsigned char *) malloc((MaxStackSize+1)*sizeof(unsigned char));
  if ((packet == (unsigned char *) NULL) ||
      (prefix == (short *) NULL) ||
      (suffix == (unsigned char *) NULL) ||
      (pixel_stack == (unsigned char *) NULL))
    return(False);
  /*
    Initialize GIF data stream decoder.
  */
  data_size=fgetc(image->file);
  clear=1 << data_size;
  end_of_information=clear+1;
  available=clear+2;
  old_code=NullCode;
  code_size=data_size+1;
  code_mask=(1 << code_size)-1;
  for (code=0; code < clear; code++)
  {
    prefix[code]=0;
    suffix[code]=code;
  }
  /*
    Decode GIF pixel stream.
  */
  datum=0;
  bits=0;
  c=0;
  count=0;
  first=0;
  top_stack=pixel_stack;
  p=image->pixels;
  for (i=0; i < image->packets; )
  {
    if (top_stack == pixel_stack)
      {
        if (bits < code_size)
          {
            /*
              Load bytes until there is enough bits for a code.
            */
            if (count == 0)
              {
                /*
                  Read a new data block.
                */
                count=ReadDataBlock((char *) packet,image->file);
                if (count <= 0)
                  break;
                c=packet;
              }
            datum+=(*c) << bits;
            bits+=8;
            c++;
            count--;
            continue;
          }
        /*
          Get the next code.
        */
        code=datum & code_mask;
        datum>>=code_size;
        bits-=code_size;
        /*
          Interpret the code
        */
        if ((code > available) || (code == end_of_information))
          break;
        if (code == clear)
          {
            /*
              Reset decoder.
            */
            code_size=data_size+1;
            code_mask=(1 << code_size)-1;
            available=clear+2;
            old_code=NullCode;
            continue;
          }
        if (old_code == NullCode)
          {
            *top_stack++=suffix[code];
            old_code=code;
            first=code;
            continue;
          }
        in_code=code;
        if (code == available)
          {
            *top_stack++=first;
            code=old_code;
          }
        while (code > clear)
        {
          *top_stack++=suffix[code];
          code=prefix[code];
        }
        first=suffix[code];
        /*
          Add a new string to the string table,
        */
        if (available >= MaxStackSize)
          break;
        *top_stack++=first;
        prefix[available]=old_code;
        suffix[available]=first;
        available++;
        if (((available & code_mask) == 0) && (available < MaxStackSize))
          {
            code_size++;
            code_mask+=available;
          }
        old_code=in_code;
      }
    /*
      Pop a pixel off the pixel stack.
    */
    top_stack--;
    p->index=(unsigned short) *top_stack;
    p->length=0;
    p++;
    i++;
    if (QuantumTick(i,image))
      ProgressMonitor(LoadImageText,i,image->packets);
  }
  /*
    Initialize any remaining color packets to a known color.
  */
  status=i == image->packets;
  for ( ; i < image->packets; i++)
  {
    p->index=0;
    p->length=0;
    p++;
  }
  SyncImage(image);
  /*
    Free decoder memory.
  */
  free((char *) pixel_stack);
  free((char *) suffix);
  free((char *) prefix);
  free((char *) packet);
  return(status);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   G I F E n c o d e I m a g e                                               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Function GIFEncodeImage compresses an image via GIF-coding.
%
%  The format of the GIFEncodeImage routine is:
%
%      status=GIFEncodeImage(image,data_size)
%
%  A description of each parameter follows:
%
%    o status:  Function GIFEncodeImage returns True if all the pixels are
%      compressed without error, otherwise False.
%
%    o image: The address of a structure of type Image.
%
%
*/
unsigned int GIFEncodeImage(Image *image, unsigned int data_size)
{
#define MaxCode(number_bits)  ((1 << (number_bits))-1)
#define MaxHashTable  5003
#define MaxGIFBits  12
#define MaxGIFTable  (1 << MaxGIFBits)
#define GIFOutputCode(code) \
{ \
  /*  \
    Emit a code. \
  */ \
  if (bits > 0) \
    datum|=((long) code << bits); \
  else \
    datum=(long) code; \
  bits+=number_bits; \
  while (bits >= 8) \
  { \
    /*  \
      Add a character to current packet. \
    */ \
    packet[byte_count++]=(unsigned char) (datum & 0xff); \
    if (byte_count >= 254) \
      { \
        (void) fputc(byte_count,image->file); \
        (void) fwrite((char *) packet,1,byte_count,image->file); \
        byte_count=0; \
      } \
    datum>>=8; \
    bits-=8; \
  } \
  if (free_code > max_code)  \
    { \
      number_bits++; \
      if (number_bits == MaxGIFBits) \
        max_code=MaxGIFTable; \
      else \
        max_code=MaxCode(number_bits); \
    } \
}

  int
    bits,
    byte_count,
    number_bits;

  long
    datum;

  register int
    i,
    j;

  register RunlengthPacket
    *p;

  short
    clear_code,
    end_of_information_code,
    free_code,
    *hash_code,
    *hash_prefix,
    index,
    max_code,
    waiting_code;

  unsigned char
    *packet,
    *hash_suffix;

  /*
    Uncompress image.
  */
  if (!UncompressImage(image))
    return(False);
  /*
    Allocate encoder tables.
  */
  packet=(unsigned char *) malloc(256*sizeof(unsigned char));
  hash_code=(short *) malloc(MaxHashTable*sizeof(short));
  hash_prefix=(short *) malloc(MaxHashTable*sizeof(short));
  hash_suffix=(unsigned char *) malloc(MaxHashTable*sizeof(unsigned char));
  if ((packet == (unsigned char *) NULL) || (hash_code == (short *) NULL) ||
      (hash_prefix == (short *) NULL) ||
      (hash_suffix == (unsigned char *) NULL))
    return(False);
  /*
    Initialize GIF encoder.
  */
  number_bits=data_size;
  max_code=MaxCode(number_bits);
  clear_code=((short) 1 << (data_size-1));
  end_of_information_code=clear_code+1;
  free_code=clear_code+2;
  byte_count=0;
  datum=0;
  bits=0;
  for (i=0; i < MaxHashTable; i++)
    hash_code[i]=0;
  GIFOutputCode(clear_code);
  /*
    Encode pixels.
  */
  p=image->pixels;
  waiting_code=p->index;
  for (i=1; i < (image->columns*image->rows); i++)
  {
    /*
      Probe hash table.
    */
    p++;
    index=p->index & 0xff;
    j=(int) ((int) index << (MaxGIFBits-8))+waiting_code;
    if (j >= MaxHashTable)
      j-=MaxHashTable;
    GIFOutputCode(waiting_code);
    if (free_code < MaxGIFTable)
      {
        hash_code[j]=free_code++;
        hash_prefix[j]=waiting_code;
        hash_suffix[j]=index;
      }
    else
      {
        /*
          Fill the hash table with empty entries.
        */
        for (j=0; j < MaxHashTable; j++)
          hash_code[j]=0;
        /*
          Reset compressor and issue a clear code.
        */
        free_code=clear_code+2;
        GIFOutputCode(clear_code);
        number_bits=data_size;
        max_code=MaxCode(number_bits);
      }
    waiting_code=index;
    if (QuantumTick(i,image))
      ProgressMonitor(SaveImageText,i,image->columns*image->rows);
  }
  /*
    Flush out the buffered code.
  */
  GIFOutputCode(waiting_code);
  GIFOutputCode(end_of_information_code);
  if (bits > 0)
    {
      /*
        Add a character to current packet.
      */
      packet[byte_count++]=(unsigned char) (datum & 0xff);
      if (byte_count >= 254)
        {
          (void) fputc(byte_count,image->file);
          (void) fwrite((char *) packet,1,byte_count,image->file);
          byte_count=0;
        }
    }
  /*
    Flush accumulated data.
  */
  if (byte_count > 0)
    {
      (void) fputc(byte_count,image->file);
      (void) fwrite((char *) packet,1,byte_count,image->file);
    }
  /*
    Free encoder memory.
  */
  free((char *) hash_suffix);
  free((char *) hash_prefix);
  free((char *) hash_code);
  free((char *) packet);
  if (i < image->packets)
    return(False);
  return(True);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   H u f f m a n D e c o d e I m a g e                                       %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Function HuffmanDecodeImage uncompresses an image via Huffman-coding.
%
%  The format of the HuffmanDecodeImage routine is:
%
%      status=HuffmanDecodeImage(image)
%
%  A description of each parameter follows:
%
%    o status:  Function HuffmanDecodeImage returns True if all the pixels are
%      compressed without error, otherwise False.
%
%    o image: The address of a structure of type Image.
%
%
*/
unsigned int HuffmanDecodeImage(Image *image)
{
#define HashSize  1021
#define MBHashA  293
#define MBHashB  2695
#define MWHashA  3510
#define MWHashB  1178

#define InitializeHashTable(hash,table,a,b) \
{  \
  entry=table;  \
  while (entry->code != 0)  \
  {  \
    hash[((entry->length+a)*(entry->code+b)) % HashSize]=entry;  \
    entry++;  \
  }  \
}

#define InputBit(bit)  \
{  \
  if ((mask & 0xff) == 0)  \
    {  \
      Byte=getc(image->file);  \
      mask=0x80;  \
    }  \
  runlength++;  \
  bit=Byte & mask ? 1 : 0; \
  mask>>=1;  \
  if (bit)  \
    runlength=0;  \
}

  HuffmanTable
    *entry,
    **mb_hash,
    **mw_hash;

  int
    code,
    color,
    count,
    length,
    null_lines,
    runlength,
    x,
    y;

  register int
    i;

  register RunlengthPacket
    *q;

  register unsigned char
    *p;

  unsigned char
    bit,
    Byte,
    mask,
    *scanline;

  unsigned int
    packets;

  unsigned short
    index;

  /*
    Allocate buffers.
  */
  mb_hash=(HuffmanTable **) malloc(HashSize*sizeof(HuffmanTable *));
  mw_hash=(HuffmanTable **) malloc(HashSize*sizeof(HuffmanTable *));
  scanline=(unsigned char *) malloc(image->columns*sizeof(unsigned char));
  if ((mb_hash == (HuffmanTable **) NULL) ||
      (mw_hash == (HuffmanTable **) NULL) ||
      (scanline == (unsigned char *) NULL))
    {
      Warning("Unable to allocate memory",(char *) NULL);
      return(False);
    }
  /*
    Initialize Huffman tables.
  */
  for (i=0; i < HashSize; i++)
  {
    mb_hash[i]=(HuffmanTable *) NULL;
    mw_hash[i]=(HuffmanTable *) NULL;
  }
  InitializeHashTable(mw_hash,TWTable,MWHashA,MWHashB);
  InitializeHashTable(mw_hash,MWTable,MWHashA,MWHashB);
  InitializeHashTable(mw_hash,EXTable,MWHashA,MWHashB);
  InitializeHashTable(mb_hash,TBTable,MBHashA,MBHashB);
  InitializeHashTable(mb_hash,MBTable,MBHashA,MBHashB);
  InitializeHashTable(mb_hash,EXTable,MBHashA,MBHashB);
  /*
    Uncompress 1D Huffman to runlength encoded pixels.
  */
  mask=0;
  null_lines=0;
  packets=image->packets;
  image->packets=0;
  q=image->pixels;
  q->length=MaxRunlength;
  for (y=0; ((y < image->rows) && (null_lines < 3)); y++)
  {
    /*
      Initialize scanline to white.
    */
    p=scanline;
    for (x=0; x < image->columns; x++)
      *p++=0;
    /*
      Decode Huffman encoded scanline.
    */
    color=True;
    code=0;
    count=0;
    length=0;
    runlength=0;
    for (x=0; x < image->columns; )
    {
      do
      {
        if (runlength < 11)
          InputBit(bit)
        else
          {
            InputBit(bit);
            if (bit)
              length=13;
          }
        code=(code << 1)+bit;
        length++;
      } while ((code <= 0) && (length < 14));
      if (length > 13)
        break;
      if (color)
        {
          if (length < 4)
            continue;
          entry=mw_hash[((length+MWHashA)*(code+MWHashB)) % HashSize];
        }
      else
        {
          if (length < 2)
            continue;
          entry=mb_hash[((length+MBHashA)*(code+MBHashB)) % HashSize];
        }
      if (!entry)
        continue;
      else
        if ((entry->length != length) || (entry->code != code))
          continue;
      switch (entry->id)
      {
        case TWId:
        case TBId:
        {
          count+=entry->count;
          if ((x+count) > image->columns)
            count=image->columns-x;
          if (count > 0)
            if (color)
              {
                x+=count;
                count=0;
              }
            else
              for ( ; count > 0; count--)
                scanline[x++]=1;
          color=!color;
          break;
        }
        case MWId:
        case MBId:
        {
          count+=entry->count;
          break;
        }
        case EXId:
        {
          count+=entry->count;
          break;
        }
        default:
          break;
      }
      code=0;
      length=0;
    }
    null_lines++;
    if (x != 0)
      {
        /*
          Skip to end of scanline.
        */
        while (runlength < 11)
          InputBit(bit);
        do
        {
          InputBit(bit);
        } while (bit == 0);
        null_lines=0;
      }
    /*
      Transfer scanline to image pixels.
    */
    p=scanline;
    for (x=0; x < image->columns; x++)
    {
      index=(unsigned short) (*p++);
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
                {
                  Warning("Unable to allocate memory",(char *) NULL);
                  return(False);
                }
              q=image->pixels+image->packets-1;
            }
          q->index=index;
          q->length=0;
        }
    }
    ProgressMonitor(LoadImageText,y,image->rows);
  }
  image->rows=y-1;
  SyncImage(image);
  image->pixels=(RunlengthPacket *)
    realloc((char *) image->pixels,image->packets*sizeof(RunlengthPacket));
  /*
    Free decoder memory.
  */
  free((char *) mw_hash);
  free((char *) mb_hash);
  free((char *) scanline);
  return(True);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   H u f f m a n E n c o d e I m a g e                                       %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Function HuffmanEncodeImage compresses an image via Huffman-coding.
%
%  The format of the HuffmanEncodeImage routine is:
%
%      status=HuffmanEncodeImage(image)
%
%  A description of each parameter follows:
%
%    o status:  Function HuffmanEncodeImage returns True if all the pixels are
%      compressed without error, otherwise False.
%
%    o image: The address of a structure of type Image.
%
%
*/
unsigned int HuffmanEncodeImage(Image *image)
{
#define HuffmanOutputCode(entry)  \
{  \
  mask=1 << (entry->length-1);  \
  while (mask != 0)  \
  {  \
    OutputBit((entry->code & mask ? 1 : 0));  \
    mask>>=1;  \
  }  \
}

#define OutputBit(count)  \
{  \
  if(count > 0)  \
    Byte=Byte | bit;  \
  bit>>=1;  \
  if ((bit & 0xff) == 0)   \
    {  \
      (void) fputc((char) Byte,image->file);  \
      Byte=0;  \
      bit=0x80;  \
    }  \
}

  HuffmanTable
    *entry;

  int
    i,
    k,
    runlength;

  register int
    j,
    n,
    x;

  register RunlengthPacket
    *p;

  register unsigned char
    *q;

  register unsigned short
    polarity;

  unsigned char
    bit,
    Byte,
    *scanline;

  unsigned int
    mask;

  /*
    Allocate scanline buffer.
  */
  scanline=(unsigned char *)
    malloc((Max(image->columns,1728)+1)*sizeof(unsigned char));
  if (scanline == (unsigned char *) NULL)
    {
      Warning("Unable to allocate memory",(char *) NULL);
      return(False);
    }
  /*
    Compress runlength encoded to 1D Huffman pixels.
  */
  polarity=0;
  if (image->colors == 2)
    polarity=(Intensity(image->colormap[0]) >
      Intensity(image->colormap[1]) ? 0 : 1);
  q=scanline;
  for (i=0; i < (Max(image->columns,1728)+1); i++)
    *q++=polarity;
  Byte=0;
  bit=0x80;
  p=image->pixels;
  q=scanline;
  x=0;
  for (i=0; i < image->packets; i++)
  {
    for (j=0; j <= ((int) p->length); j++)
    {
      *q++=(unsigned char) (p->index == polarity ? polarity : !polarity);
      x++;
      if (x < image->columns)
        continue;
      /*
        Huffman encode scanline.
      */
      q=scanline;
      for (n=Max(image->columns,1728); n > 0; )
      {
        /*
          Output white run.
        */
        for (runlength=0; ((*q == polarity) && (n > 0)); n--)
        {
          q++;
          runlength++;
        }
        if (runlength >= 64)
          {
            entry=MWTable+((runlength/64)-1);
            runlength-=entry->count;
            HuffmanOutputCode(entry);
          }
        entry=TWTable+runlength;
        HuffmanOutputCode(entry);
        if (n != 0)
          {
            /*
              Output black run.
            */
            for (runlength=0; ((*q != polarity) && (n > 0)); n--)
            {
              q++;
              runlength++;
            }
            if (runlength >= 64)
              {
                entry=MBTable+((runlength/64)-1);
                runlength-=entry->count;
                HuffmanOutputCode(entry);
              }
            entry=TBTable+runlength;
            HuffmanOutputCode(entry);
          }
      }
      /*
        End of line.
      */
      for (k=0; k < 11; k++)
        OutputBit(0);
      OutputBit(1);
      x=0;
      q=scanline;
    }
    p++;
    if (QuantumTick(i,image))
      ProgressMonitor(SaveImageText,i,image->packets);
  }
  /*
    End of page.
  */
  for (i=0; i < 6; i++)
  {
    for (k=0; k < 11; k++)
      OutputBit(0);
    OutputBit(1);
  }
  /*
    Flush bits.
  */
  if (bit != 0x80)
    (void) fputc((char) Byte,image->file);
  free((char *) scanline);
  return(True);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   P a c k b i t s E n c o d e I m a g e                                     %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Function PackbitsEncodeImage compresses an image via Macintosh Packbits
%  encoding specific to Postscript Level II or Portable Document Format.  To
%  ensure portability, the binary Packbits bytes are encoded as ASCII Base-85.
%
%  The format of the PackbitsEncodeImage routine is:
%
%      status=PackbitsEncodeImage(file,pixels,number_pixels)
%
%  A description of each parameter follows:
%
%    o status:  Function PackbitsEncodeImage returns True if all the pixels are
%      compressed without error, otherwise False.
%
%    o file: The address of a structure of type FILE.  LZW encoded pixels
%      are written to this file.
%
%    o pixels: The address of an unsigned array of characters containing the
%      pixels to compress.
%
%    o number_pixels:  An unsigned interger that specifies the number of
%      pixels to compress.
%
%
*/
unsigned int PackbitsEncodeImage(FILE *file, unsigned char *pixels, unsigned int number_pixels)
{
  register int
    count,
    i;

  unsigned char
    *packbits;

  /*
    Compress pixels with Packbits encoding.
  */
  packbits=(unsigned char *) malloc(128*sizeof(unsigned char));
  if (packbits == (unsigned char *) NULL)
    {
      Warning("Memory allocation error",(char *) NULL);
      return(False);
    }
  Ascii85Initialize();
  while (number_pixels != 0)
  {
    switch (number_pixels)
    {
      case 1:
      {
        number_pixels--;
        Ascii85Encode(0,file);
        Ascii85Encode(*pixels,file);
        break;
      }
      case 2:
      {
        number_pixels-=2;
        Ascii85Encode(1,file);
        Ascii85Encode(*pixels,file);
        Ascii85Encode(pixels[1],file);
        break;
      }
      case 3:
      {
        number_pixels-=3;
        if ((*pixels == *(pixels+1)) && (*(pixels+1) == *(pixels+2)))
          {
            Ascii85Encode((256-3)+1,file);
            Ascii85Encode(*pixels,file);
            break;
          }
        Ascii85Encode(2,file);
        Ascii85Encode(*pixels,file);
        Ascii85Encode(pixels[1],file);
        Ascii85Encode(pixels[2],file);
        break;
      }
      default:
      {
        if ((*pixels == *(pixels+1)) && (*(pixels+1) == *(pixels+2)))
          {
            /*
              Packed run.
            */
            count=3;
            while ((count < number_pixels) && (*pixels == *(pixels+count)))
            {
              count++;
              if (count >= 127)
                break;
            }
            number_pixels-=count;
            Ascii85Encode((256-count)+1,file);
            Ascii85Encode(*pixels,file);
            pixels+=count;
            break;
          }
        /*
          Literal run.
        */
        count=0;
        while ((*(pixels+count) != *(pixels+count+1)) ||
               (*(pixels+count+1) != *(pixels+count+2)))
        {
          packbits[count+1]=pixels[count];
          count++;
          if ((count >= (number_pixels-3)) || (count >= 127))
            break;
        }
        number_pixels-=count;
        *packbits=count-1;
        for (i=0; i <= count; i++)
          Ascii85Encode(packbits[i],file);
        pixels+=count;
        break;
      }
    }
  }
  Ascii85Encode(128,file);  /* EOD marker */
  Ascii85Flush(file);
  free((char *) packbits);
  return(True);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   P C D D e c o d e I m a g e                                               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Function PCDDecodeImage recovers the Huffman encoded luminance and
%  chrominance deltas.
%
%  The format of the PCDDecodeImage routine is:
%
%      status=PCDDecodeImage(image,luma,chroma1,chroma2)
%
%  A description of each parameter follows:
%
%    o status:  Function PCDDecodeImage returns True if all the deltas are
%      recovered without error, otherwise False.
%
%    o image: The address of a structure of type Image.
%
%    o luma: The address of a character buffer that contains the
%      luminance information.
%
%    o chroma1: The address of a character buffer that contains the
%      chrominance information.
%
%    o chroma2: The address of a character buffer that contains the
%      chrominance information.
%
%
%
*/
unsigned int PCDDecodeImage(Image *image, unsigned char *luma, unsigned char *chroma1, unsigned char *chroma2)
{
#define IsSync  ((accumulator & 0xffffff00) == 0xfffffe00)
#define PCDDecodeImageText  "  PCD decode image...  "
#define PCDGetBits(n) \
{  \
  accumulator=(accumulator << n) & 0xffffffff; \
  bits-=n; \
  while (bits <= 24) \
  { \
    if (p >= (buffer+0x800)) \
      { \
        (void) ReadData((char *) buffer,1,0x800,image->file); \
        p=buffer; \
      } \
    accumulator|=((unsigned int) (*p) << (24-bits)); \
    bits+=8; \
    p++; \
  } \
}

  typedef struct PCDTable
  {
    unsigned int
      length;

    unsigned long
      sequence;

    unsigned char
      key;

    unsigned long
      mask;
  } PCDTable;

  int
    count;

  PCDTable
    *pcd_table[3];

  register int
    i,
    j;

  register PCDTable
    *r;

  register Quantum
    *range_limit;

  register unsigned char
    *p,
    *q;

  unsigned char
    *buffer;

  unsigned int
    accumulator,
    bits,
    length,
    pcd_length[3],
    plane,
    row;

  /*
    Initialize Huffman tables.
  */
  buffer=(unsigned char *) malloc(0x800*sizeof(unsigned char));
  if (buffer == (unsigned char *) NULL)
    {
      Warning("Memory allocation error",(char *) NULL);
      return(False);
    }
  (void) ReadData((char *) buffer,1,0x800,image->file);
  p=buffer;
  for (i=0; i < (image->columns > 1536 ? 3 : 1); i++)
  {
    length=(*p++)+1;
    pcd_table[i]=(PCDTable *) malloc(length*sizeof(PCDTable));
    if (pcd_table[i] == (PCDTable *) NULL)
      {
        Warning("Memory allocation error",(char *) NULL);
        free((char *) buffer);
        return(False);
      }
    r=pcd_table[i];
    for (j=0; j < length; j++)
    {
      r->length=(*p++)+1;
      r->sequence=(((unsigned int) *p++) << 24);
      r->sequence|=(((unsigned int) *p++) << 16);
      r->key=(*p++);
      r->mask=(~((((unsigned int) 1) << (32-r->length))-1));
      r++;
    }
    pcd_length[i]=length;
  }
  /*
    Initialize range limits.
  */
  range_limit=(Quantum *) malloc(3*(MaxRGB+1)*sizeof(Quantum));
  if (range_limit == (Quantum *) NULL)
    {
      Warning("Memory allocation error",(char *) NULL);
      free((char *) buffer);
      return(False);
    }
  for (i=0; i <= MaxRGB; i++)
  {
    range_limit[i]=0;
    range_limit[i+(MaxRGB+1)]=(Quantum) i;
    range_limit[i+(MaxRGB+1)*2]=MaxRGB;
  }
  range_limit+=(MaxRGB+1);
  /*
    Search for Sync Byte.
  */
  accumulator=0;
  bits=32;
  PCDGetBits(16);
  PCDGetBits(16);
  while ((accumulator & 0x00fff000) != 0x00fff000)
    PCDGetBits(8);
  while (!IsSync)
    PCDGetBits(1);
  /*
    Recover the Huffman encoded luminance and chrominance deltas.
  */
  for ( ; ; )
  {
    if (IsSync)
      {
        /*
          Determine plane and row number.
        */
        PCDGetBits(24);
        row=(accumulator >> 17) & 0x1fff;
        plane=accumulator >> 30;
        PCDGetBits(16);
        ProgressMonitor(PCDDecodeImageText,row,image->rows);
        if (row >= image->rows)
          break;
        switch (plane)
        {
          case 0:
          {
            q=luma+row*image->columns;
            count=image->columns;
            break;
          }
          case 2:
          {
            q=chroma1+(row >> 1)*image->columns;
            count=image->columns >> 1;
            plane--;
            break;
          }
          case 3:
          {
            q=chroma2+(row >> 1)*image->columns;
            count=image->columns >> 1;
            plane--;
            break;
          }
          default:
          {
            Warning("Corrupt PCD image",image->filename);
            return(False);
          }
        }
        length=pcd_length[plane];
        continue;
      }
    /*
      Decode luminance or chrominance deltas.
    */
    r=pcd_table[plane];
    for (i=0; ((i < length) && ((accumulator & r->mask) != r->sequence)); i++)
      r++;
    if (r == (PCDTable *) NULL)
      {
        Warning("Corrupt PCD image, skipping to sync Byte",image->filename);
        while ((accumulator & 0x00fff000) != 0x00fff000)
          PCDGetBits(8);
        while (!IsSync)
          PCDGetBits(1);
        continue;
      }
    if (count >= 0)
      if (r->key < 128)
        *q++=range_limit[(int) *q+(int) r->key];
      else
        *q++=range_limit[(int) *q+(int) r->key-256];
    PCDGetBits(r->length);
    count--;
  }
  /*
    Free memory.
  */
  for (i=0; i < (image->columns > 1536 ? 3 : 1); i++)
    free((char *) pcd_table[i]);
  range_limit-=(MaxRGB+1);
  free((char *) range_limit);
  free((char *) buffer);
  return(True);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   P I C T E n c o d e I m a g e                                             %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Function PICTEncodeImage compresses an image via Macintosh pack bits
%  encoding for Macintosh PICT images.
%
%  The format of the PICTEncodeImage routine is:
%
%      status=PICTEncodeImage(image,scanline,packbits)
%
%  A description of each parameter follows:
%
%    o status:  Function PICTEncodeImage returns True if all the pixels are
%      compressed without error, otherwise False.
%
%    o image: The address of a structure of type Image.
%
%    o scanline: A pointer to an array of characters to pack.
%
%    o packbits: A pointer to an array of characters where the packed
%      characters are stored.
%
%
*/
unsigned int PICTEncodeImage(Image *image, unsigned char *scanline, unsigned char *packbits)
{
#define MaxCount  128
#define MaxPackbitsRunlength  128

  int
    count,
    number_packets,
    repeat_count,
    runlength;

  register int
    i;

  register unsigned char
    *p,
    *q;

  unsigned char
    index;

  unsigned int
    bytes_per_line;

  /*
    Pack scanline.
  */
  bytes_per_line=image->columns;
  if (image->class == DirectClass)
    bytes_per_line*=3;
  count=0;
  runlength=0;
  p=scanline+(bytes_per_line-1);
  q=packbits;
  index=(*p);
  for (i=bytes_per_line-1; i >= 0; i--)
  {
    if (index == *p)
      runlength++;
    else
      {
        if (runlength < 3)
          while (runlength > 0)
          {
            *q++=index;
            runlength--;
            count++;
            if (count == MaxCount)
              {
                *q++=MaxCount-1;
                count-=MaxCount;
              }
          }
        else
          {
            if (count > 0)
              *q++=count-1;
            count=0;
            while (runlength > 0)
            {
              repeat_count=runlength;
              if (repeat_count > MaxPackbitsRunlength)
                repeat_count=MaxPackbitsRunlength;
              *q++=index;
              *q++=257-repeat_count;
              runlength-=repeat_count;
            }
          }
        runlength=1;
      }
    index=(*p);
    p--;
  }
  if (runlength < 3)
    while (runlength > 0)
    {
      *q++=index;
      runlength--;
      count++;
      if (count == MaxCount)
        {
          *q++=MaxCount-1;
          count-=MaxCount;
        }
    }
  else
    {
      if (count > 0)
        *q++=count-1;
      count=0;
      while (runlength > 0)
      {
        repeat_count=runlength;
        if (repeat_count > MaxPackbitsRunlength)
          repeat_count=MaxPackbitsRunlength;
        *q++=index;
        *q++=257-repeat_count;
        runlength-=repeat_count;
      }
    }
  if (count > 0)
    *q++=count-1;
  /*
    Write the number of and the packed packets.
  */
  number_packets=q-packbits;
  if ((bytes_per_line-1) > 250)
    {
      MSBFirstWriteShort((unsigned short) number_packets,image->file);
      number_packets+=2;
    }
  else
    {
      index=number_packets;
      (void) fputc((char) index,image->file);
      number_packets++;
    }
  while (q != packbits)
  {
    q--;
    (void) fputc((char) *q,image->file);
  }
  return(number_packets);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   R u n l e n g t h D e c o d e I m a g e                                   %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Function RunlengthDecodeImage unpacks the packed image pixels into
%  runlength-encoded pixel packets.  The packed image pixel memory is then
%  freed.
%
%  The format of the RunlengthDecodeImage routine is:
%
%      status=RunlengthDecodeImage(image)
%
%  A description of each parameter follows:
%
%    o status: Function RunlengthDecodeImage return True if the image is
%      decoded.  False is returned if there is an error occurs.
%
%    o image: The address of a structure of type Image.
%
%
*/
unsigned int RunlengthDecodeImage(Image *image)
{
  register int
    i;

  register RunlengthPacket
    *q;

  register unsigned char
    *p;

  unsigned long
    count;

  unsigned short
    value;

  if (image->packed_pixels == (unsigned char *) NULL)
    return(True);
  /*
    Allocate pixels.
  */
  if (image->pixels != (RunlengthPacket *) NULL)
    free((char *) image->pixels);
  image->pixels=(RunlengthPacket *)
    malloc((unsigned int) image->packets*sizeof(RunlengthPacket));
  if (image->pixels == (RunlengthPacket *) NULL)
    {
      Warning("Unable to unpack pixels","Memory allocation failed");
      return(False);
    }
  /*
    Unpack the packed image pixels into runlength-encoded pixel packets.
  */
  p=image->packed_pixels;
  q=image->pixels;
  count=0;
  if (image->class == DirectClass)
    {
      if (image->compression == RunlengthEncodedCompression)
        for (i=0; i < image->packets; i++)
        {
          ReadQuantum(q->red,p);
          ReadQuantum(q->green,p);
          ReadQuantum(q->blue,p);
          q->index=(unsigned short) (image->matte ? (*p++) : 0);
          q->length=(*p++);
          count+=(q->length+1);
          q++;
          if (QuantumTick(i,image))
            ProgressMonitor(LoadImageText,i,image->packets);
        }
      else
        for (i=0; i < image->packets; i++)
        {
          ReadQuantum(q->red,p);
          ReadQuantum(q->green,p);
          ReadQuantum(q->blue,p);
          q->index=(unsigned short) (image->matte ? (*p++) : 0);
          q->length=0;
          count++;
          q++;
          if (QuantumTick(i,image))
            ProgressMonitor(LoadImageText,i,image->packets);
        }
    }
  else
    {
      register unsigned short
        index;

      if (image->compression == RunlengthEncodedCompression)
        {
          if (image->colors <= 256)
            for (i=0; i < image->packets; i++)
            {
              q->index=(unsigned short) (*p++);
              q->length=(*p++);
              count+=(q->length+1);
              q++;
              if (QuantumTick(i,image))
                ProgressMonitor(LoadImageText,i,image->packets);
            }
          else
            for (i=0; i < image->packets; i++)
            {
              index=(*p++) << 8;
              index|=(*p++);
              q->index=index;
              q->length=(*p++);
              count+=(q->length+1);
              q++;
              if (QuantumTick(i,image))
                ProgressMonitor(LoadImageText,i,image->packets);
            }
        }
      else
        if (image->colors <= 256)
          for (i=0; i < image->packets; i++)
          {
            q->index=(unsigned short) (*p++);
            q->length=0;
            count++;
            q++;
            if (QuantumTick(i,image))
              ProgressMonitor(LoadImageText,i,image->packets);
          }
        else
          for (i=0; i < image->packets; i++)
          {
            index=(*p++) << 8;
            index|=(*p++);
            q->index=index;
            q->length=0;
            count++;
            q++;
            if (QuantumTick(i,image))
              ProgressMonitor(LoadImageText,i,image->packets);
          }
      SyncImage(image);
    }
  /*
    Free packed pixels memory.
  */
  free((char *) image->packed_pixels);
  image->packed_pixels=(unsigned char *) NULL;
  /*
    Guarentee the correct number of pixel packets.
  */
  if (count > (image->columns*image->rows))
    {
      Warning("insufficient image data in file",image->filename);
      return(False);
    }
  else
    if (count < (image->columns*image->rows))
      {
        Warning("too much image data in file",image->filename);
        return(False);
      }
  return(True);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   R u n l e n g t h E n c o d e I m a g e                                   %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Function RunlengthEncodeImage packs the runlength-encoded pixel packets
%  into the minimum number of bytes.
%
%  The format of the RunlengthEncodeImage routine is:
%
%      status=RunlengthEncodeImage(image)
%
%  A description of each parameter follows:
%
%    o status: Function RunlengthEncodeImage return True if the image is
%      packed.  False is returned if an error occurs.
%
%    o image: The address of a structure of type Image.
%
%
*/
unsigned int RunlengthEncodeImage(Image *image)
{
#define SpecialRunlength  255

  register int
    i,
    j;

  register RunlengthPacket
    *p;

  register unsigned char
    *q;

  unsigned long
    count,
    packets;

  unsigned short
    value;

  if (image == (Image *) NULL)
    return(False);
  if (image->pixels == (RunlengthPacket *) NULL)
    {
      Warning("Unable to pack pixels","no image pixels");
      return(False);
    }
  if (image->compression == RunlengthEncodedCompression)
    {
      register RunlengthPacket
        *q;

      /*
        Compress image.
      */
      p=image->pixels;
      for (i=0; i < image->packets; i++)
      {
        if (p->length > SpecialRunlength)
          {
            /*
              Uncompress image to allow in-place compression.
            */
            if (!UncompressImage(image))
              return(False);
            break;
          }
        p++;
      }
      p=image->pixels;
      image->runlength=p->length+1;
      image->packets=0;
      q=image->pixels;
      q->length=SpecialRunlength;
      if (image->matte)
        for (i=0; i < (image->columns*image->rows); i++)
        {
          if (image->runlength != 0)
            image->runlength--;
          else
            {
              p++;
              image->runlength=p->length;
            }
          if ((p->red == q->red) && (p->green == q->green) &&
              (p->blue == q->blue) && (p->index == q->index) &&
              (q->length < SpecialRunlength))
            q->length++;
          else
            {
              if (image->packets != 0)
                q++;
              image->packets++;
              *q=(*p);
              q->length=0;
            }
        }
      else
        for (i=0; i < (image->columns*image->rows); i++)
        {
          if (image->runlength != 0)
            image->runlength--;
          else
            {
              p++;
              image->runlength=p->length;
            }
          if ((p->red == q->red) && (p->green == q->green) &&
              (p->blue == q->blue) && (q->length < SpecialRunlength))
            q->length++;
          else
            {
              if (image->packets != 0)
                q++;
              image->packets++;
              *q=(*p);
              q->length=0;
            }
        }
      image->pixels=(RunlengthPacket *) realloc((char *) image->pixels,
        image->packets*sizeof(RunlengthPacket));
      /*
        Runlength-encode only if it consumes less memory than no compression.
      */
      if (image->class == DirectClass)
        {
          if (image->packets >= ((image->columns*image->rows*3) >> 2))
            image->compression=NoCompression;
        }
      else
        if (image->packets >= ((image->columns*image->rows) >> 1))
          image->compression=NoCompression;
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
  /*
    Allocate packed pixel memory.
  */
  if (image->packed_pixels != (unsigned char *) NULL)
    free((char *) image->packed_pixels);
  packets=image->packets;
  if (image->compression != RunlengthEncodedCompression)
    packets=image->columns*image->rows;
  image->packed_pixels=(unsigned char *)
    malloc((unsigned int) packets*image->packet_size*sizeof(unsigned char));
  if (image->packed_pixels == (unsigned char *) NULL)
    {
      Warning("Unable to pack pixels","Memory allocation failed");
      return(False);
    }
  /*
    Packs the runlength-encoded pixel packets into the minimum number of bytes.
  */
  p=image->pixels;
  q=image->packed_pixels;
  count=0;
  if (image->class == DirectClass)
    {
      if (image->compression == RunlengthEncodedCompression)
        for (i=0; i < image->packets; i++)
        {
          WriteQuantum(p->red,q);
          WriteQuantum(p->green,q);
          WriteQuantum(p->blue,q);
          if (image->matte)
            *q++=(unsigned char) p->index;
          *q++=p->length;
          count+=(p->length+1);
          p++;
          if (QuantumTick(i,image))
            ProgressMonitor(SaveImageText,i,image->packets);
        }
      else
        for (i=0; i < image->packets; i++)
        {
          for (j=0; j <= ((int) p->length); j++)
          {
            WriteQuantum(p->red,q);
            WriteQuantum(p->green,q);
            WriteQuantum(p->blue,q);
            if (image->matte)
              *q++=(unsigned char) p->index;
          }
          count+=(p->length+1);
          p++;
          if (QuantumTick(i,image))
            ProgressMonitor(SaveImageText,i,image->packets);
        }
    }
  else
    if (image->compression == RunlengthEncodedCompression)
      {
        if (image->colors <= 256)
          for (i=0; i < image->packets; i++)
          {
            *q++=(unsigned char) p->index;
            *q++=p->length;
            count+=(p->length+1);
            p++;
            if (QuantumTick(i,image))
              ProgressMonitor(SaveImageText,i,image->packets);
          }
        else
          for (i=0; i < image->packets; i++)
          {
            *q++=p->index >> 8;
            *q++=(unsigned char) p->index;
            *q++=p->length;
            count+=(p->length+1);
            p++;
            if (QuantumTick(i,image))
              ProgressMonitor(SaveImageText,i,image->packets);
          }
      }
    else
      if (image->colors <= 256)
        for (i=0; i < image->packets; i++)
        {
          for (j=0; j <= ((int) p->length); j++)
            *q++=p->index;
          count+=(p->length+1);
          p++;
          if (QuantumTick(i,image))
            ProgressMonitor(SaveImageText,i,image->packets);
        }
      else
        {
          register unsigned char
            xff00,
            xff;

          for (i=0; i < image->packets; i++)
          {
            xff00=p->index >> 8;
            xff=p->index;
            for (j=0; j <= ((int) p->length); j++)
            {
              *q++=xff00;
              *q++=xff;
            }
            count+=(p->length+1);
            p++;
            if (QuantumTick(i,image))
              ProgressMonitor(SaveImageText,i,image->packets);
          }
        }
  image->packets=packets;
  /*
    Guarentee the correct number of pixel packets.
  */
  if (count < (image->columns*image->rows))
    {
      Warning("insufficient image data in",image->filename);
      return(False);
    }
  else
    if (count > (image->columns*image->rows))
      {
        Warning("too much image data in",image->filename);
        return(False);
      }
  return(True);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   S U N D e c o d e I m a g e                                               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Function SUNDecodeImage unpacks the packed image pixels into
%  runlength-encoded pixel packets.
%
%  The format of the SUNDecodeImage routine is:
%
%      status=SUNDecodeImage(compressed_pixels,pixels,number_columns,
%        number_rows)
%
%  A description of each parameter follows:
%
%    o status:  Function SUNDecodeImage returns True if all the pixels are
%      uncompressed without error, otherwise False.
%
%    o compressed_pixels:  The address of a Byte (8 bits) array of compressed
%      pixel data.
%
%    o pixels:  The address of a Byte (8 bits) array of pixel data created by
%      the uncompression process.  The number of bytes in this array
%      must be at least equal to the number columns times the number of rows
%      of the source pixels.
%
%    o number_columns:  An integer value that is the number of columns or
%      width in pixels of your source image.
%
%    o number_rows:  An integer value that is the number of rows or
%      heigth in pixels of your source image.
%
%
*/
unsigned int SUNDecodeImage(unsigned char *compressed_pixels, unsigned char *pixels, unsigned int number_columns, unsigned int number_rows)
{
  register int
    count;

  register unsigned char
    *p,
    *q;

  unsigned char
    Byte;

  p=compressed_pixels;
  q=pixels;
  while ((q-pixels) <= (number_columns*number_rows))
  {
    Byte=(*p++);
    if (Byte != 128)
      *q++=Byte;
    else
      {
        /*
          Runlength-encoded packet: <count><Byte>
        */
        count=(*p++);
        if (count > 0)
          Byte=(*p++);
        while (count >= 0)
        {
          *q++=Byte;
          count--;
        }
     }
  }
  return(True);
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

#ifdef __cplusplus
}
#endif


/* -------------------------------------------------------------------------- */

#endif   /* #ifdef KJB_HAVE_X11  */

#endif   /* #ifndef __C2MAN__  */

