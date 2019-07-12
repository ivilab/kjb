
/* $Id: magick.h 21596 2017-07-30 23:33:36Z kobus $ */

#ifndef __C2MAN__     

/*
 * Make sure that these files are not a liability when there is no X11. If this
 * is the case, then comment out the whole thing.
*/
#ifdef KJB_HAVE_X11 

/* -------------------------------------------------------------------------- */


#ifndef MAGICK_MAGICK_INCLUDED   /* Kobus */
#define MAGICK_MAGICK_INCLUDED   /* Kobus */


/*
// Kobus
*/

#ifdef KJB_HAVE_TIFF
#    define HasTIFF
#endif

#ifdef KJB_HAVE_JPEG
#    define HasJPEG
#endif

#include "l/l_incl.h"

#ifdef KJB_CPLUSPLUS
#    define class c_class
#    define operator c_operator
#endif

/* End Kobus's changes. */


/*
  Include declarations.
*/
#ifdef hpux
#define _HPUX_SOURCE 1
#endif
#include <stdio.h>
#if defined(__STDC__) || defined(sgi) || defined(_AIX)
#include <stdlib.h>
#include <unistd.h>
#else
#ifdef vms
#include <stdlib.h>
#else
#include <malloc.h>
#include <memory.h>
#endif
#endif
#include <ctype.h>
#include <string.h>
#include <math.h>
#include <signal.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#undef index
#undef assert

/*
  ImageMagick include declarations.
*/
#if __STDC__ || defined(sgi) || defined(_AIX) || __cplusplus
#define _Declare(formal_parameters) formal_parameters
#else
#define const
#define _Declare(formal_parameters) ()
#endif
#include "magick/image.h"
#include "magick/compress.h"
#include "magick/utility.h"
#include "magick/monitor.h"
#include "magick/error.h"
#include "magick/X.h"
#include "magick/widget.h"
#include "magick/PreRvIcccm.h"

#ifdef __cplusplus
extern "C" {
#ifdef COMPILING_CPLUSPLUS_SOURCE
namespace kjb_c {
#endif
#endif


/*
  Define declarations.
*/
#define DownShift(x) (((int) ((x)+(1L << 13))) >> 14)
#define False  0
#define Max(x,y)  (((x) > (y)) ? (x) : (y))
#define Min(x,y)  (((x) < (y)) ? (x) : (y))
#ifndef M_PI
#define M_PI  3.14159265358979323846
#endif
#define QuantumTick(i,image) \
  (((i+1) == image->packets) || ((i % image->rows) == 0))
#ifndef STDIN_FILENO
#define STDIN_FILENO  0
#endif
#define True  1
#define UpShift(x) ((int) (x) << 14)
#define UpShifted(x) ((int) ((x)*(1L << 14)+0.5))

/*
  Review these definitions and change them to suit your local requirements.
*/
#define CompressCommand  "|compress -c > %s"
#define DefaultFont  "Helvetica"
#define DefaultPointSize  12
#define GunzipCommand  "|gzip -cdfq %s"
#define GzipCommand  "|gzip -cf > %s"
#define PostscriptColorDevice  "pnmraw"
#define PostscriptMonoDevice  "pbmraw"
#define WWWCommand  "/usr/local/bin/get -q %s:%s > %s"
#define ReadBinaryType  "rb"
#define WriteBinaryType  "wb"
#define UncompressCommand  "|uncompress -c %s"
#define UndoCache  "16"

#ifndef vms
#define ApplicationDefaults  "/usr/lib/X11/app-defaults/"
#define DocumentationBrowser \
  "netscape http://www.wizards.dupont.com/cristy/ImageMagick.html &"
#define EditorCommand  "xterm -title \"Edit Image Comment\" -e vi %s"
#define HistogramCommand \
  "display -immutable -window_group 0x%lx -title \"Histogram of %s\" tmp:%s &"
#define PostscriptCommand \
  "gs -sDEVICE=%s -q -dNOPAUSE -dSAFER %s -sOutputFile=%s -- %s < /dev/null > /dev/null"
#define PreferencesDefaults  "~/."
#define PrintCommand  "lpr %s"
#define RGBColorDatabase  "/usr/lib/X11/rgb.txt"
#else
#define ApplicationDefaults  "decw$system_defaults:"
#define DocumentationBrowser \
  "mosaic http://www.wizards.dupont.com/cristy/ImageMagick.html"
#define EditorCommand  "cre/term/wait edit/tpu %s"
#define HistogramCommand \
  "display -immutable -window_group 0x%lx -title \"Histogram of %s\" tmp:%s"
#define pclose(file)  fclose(file)
#define popen(command,mode)  fopen(command,mode)
#define PostscriptCommand \
  "gs \"-sDEVICE=%s\" -q \"-dNOPAUSE\" \"-dSAFER\" \"%s\" \"-sOutputFile=%s\" -- \"%s\""
#define PreferencesDefaults  "decw$user_defaults:"
#define PrintCommand  "print %s"
#define RGBColorDatabase  "sys$common:[sysmgr]decw$rgb.dat"
#define unlink(file)  remove(file)
#endif

/*
  Image colorspaces:
*/
#define UndefinedColorspace  0
#define RGBColorspace  1
#define GRAYColorspace 2
#define OHTAColorspace  3
#define XYZColorspace  4
#define YCbCrColorspace  5
#define YCCColorspace  6
#define YIQColorspace  7
#define YPbPrColorspace  8
#define YUVColorspace  9
/*
  Image compression algorithms:
*/
#define UndefinedCompression  0
#define NoCompression  1
#define RunlengthEncodedCompression  2
#define ZlibCompression  3
/*
  Image interlace:
*/
#define UndefinedInterlace  0
#define NoneInterlace  1
#define LineInterlace  2
#define PlaneInterlace  3
#define DefaultInterlace  PlaneInterlace
/*
  Image compositing operations:
*/
#define UndefinedCompositeOp  0
#define OverCompositeOp  1
#define InCompositeOp  2
#define OutCompositeOp  3
#define AtopCompositeOp  4
#define XorCompositeOp  5
#define PlusCompositeOp  6
#define MinusCompositeOp  7
#define AddCompositeOp  8
#define SubtractCompositeOp  9
#define DifferenceCompositeOp  10
#define ReplaceCompositeOp  11
#define MatteReplaceCompositeOp  12
#define BlendCompositeOp  13
/*
  Image color matching algorithms:
*/
#define PointMethodOp  0
#define ReplaceMethodOp  1
#define FloodfillMethodOp  2
#define ResetMethodOp  3
/*
  Page geometries:
*/
#define PSDensityGeometry  "72x72"
#define PSPageGeometry  "612x792+43+43"
#define TextPageGeometry  "612x792+43+43"
/*
  3D effects:
*/
#define HighlightModulate  UpScale(125)
#define ShadowModulate  UpScale(135)
#define DepthModulate  UpScale(185)
#define TroughModulate  UpScale(110)

/*
  Variable declarations.
*/
static char
  Version[]="@(#)ImageMagick 3.7.1 96/01/01 cristy@dupont.com";


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

