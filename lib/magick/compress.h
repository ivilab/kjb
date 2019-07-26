
/* $Id: compress.h 4727 2009-11-16 20:53:54Z kobus $ */

#ifndef __C2MAN__     

/*
 * Make sure that these files are not a liability when there is no X11. If this
 * is the case, then comment out the whole thing.
*/
#ifdef KJB_HAVE_X11 

/* -------------------------------------------------------------------------- */


#ifndef MAGICK_COMPRESS_INCLUDED   /* Kobus */
#define MAGICK_COMPRESS_INCLUDED   /* Kobus */

#ifdef __cplusplus
extern "C" {
#ifdef COMPILING_CPLUSPLUS_SOURCE
namespace kjb_c {
#endif
#endif


/*
  Compress utility routines.
*/
extern unsigned int
  BMPDecodeImage _Declare((unsigned char *,unsigned char *,unsigned int,
    unsigned int,unsigned int)),
  BMPEncodeImage _Declare((unsigned char *,unsigned char *,unsigned int,
    unsigned int)),
  HuffmanDecodeImage _Declare((Image *)),
  HuffmanEncodeImage _Declare((Image *)),
  GIFDecodeImage _Declare((Image *)),
  GIFEncodeImage _Declare((Image *,unsigned int)),
  LZWEncodeImage _Declare((FILE *,unsigned char *,unsigned int)),
  PackbitsEncodeImage _Declare((FILE *,unsigned char *,unsigned int)),
  PCDDecodeImage _Declare((Image *,unsigned char *,unsigned char *,
    unsigned char *)),
  PICTEncodeImage _Declare((Image *,unsigned char *,unsigned char *)),
  QDecodeImage _Declare((unsigned char *,unsigned char *,unsigned int,
    unsigned int)),
  QEncodeImage _Declare((unsigned char *,unsigned char *,unsigned int,
    unsigned int)),
  RunlengthDecodeImage _Declare((Image *)),
  RunlengthEncodeImage _Declare((Image *)),
  SUNDecodeImage _Declare((unsigned char *,unsigned char *,unsigned int,
    unsigned int));

extern void
  Ascii85Encode _Declare((unsigned int,FILE *)),
  Ascii85Flush _Declare((FILE *)),
  Ascii85Initialize _Declare((void));


#ifdef __cplusplus
#ifdef COMPILING_CPLUSPLUS_SOURCE
}
#endif
}
#endif

#endif


/* -------------------------------------------------------------------------- */

#endif   /* #ifdef KJB_HAVE_X11  */

#endif   /* #ifndef __C2MAN__  */

