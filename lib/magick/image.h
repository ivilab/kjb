
/* $Id: image.h 4727 2009-11-16 20:53:54Z kobus $ */

#ifndef __C2MAN__     

/*
 * Make sure that these files are not a liability when there is no X11. If this
 * is the case, then comment out the whole thing.
*/
#ifdef KJB_HAVE_X11 

/* -------------------------------------------------------------------------- */


#ifndef MAGICK_IMAGE_INCLUDED   /* Kobus */
#define MAGICK_IMAGE_INCLUDED   /* Kobus */

#ifdef __cplusplus
extern "C" {
#ifdef COMPILING_CPLUSPLUS_SOURCE
namespace kjb_c {
#endif
#endif


/*
  Image define declarations.
*/
#define AbsoluteValue(x)  ((x) < 0 ? -(x) : (x))
#define DegreesToRadians(x) ((x)*M_PI/180.0)
#define Intensity(color)  \
  ((unsigned int) ((color).red*77+(color).green*150+(color).blue*29) >> 8)
#define IsGray(color)  \
  (((color).red == (color).green) && ((color).green == (color).blue))
#define MaxColormapSize  65535L
#define MaxTextLength  2048
#define Opaque  255
#define RadiansToDegrees(x) ((x)*180/M_PI)
#define ReadQuantum(quantum,p)  \
{  \
  if (image->depth == 8) \
    quantum=UpScale(*p++); \
  else \
    { \
      value=(*p++) << 8;  \
      value|=(*p++);  \
      quantum=value >> (image->depth-QuantumDepth); \
    } \
}
#define ReadQuantumFile(quantum)  \
{  \
  if (image->depth == 8) \
    quantum=UpScale(fgetc(image->file)); \
  else \
    quantum=MSBFirstReadShort(image->file) >> (image->depth-QuantumDepth); \
}
#define SharpenFactor  70.0
#define SmoothingThreshold  1.5
#define Transparent  0
#define WriteQuantum(quantum,q)  \
{  \
  if (image->depth == 8) \
    *q++=DownScale(quantum); \
  else \
    { \
      value=(quantum); \
      if ((QuantumDepth-image->depth) > 0) \
        value*=257; \
      *q++=value >> 8; \
      *q++=value; \
    } \
}
#define WriteQuantumFile(quantum)  \
{  \
  if (image->depth == 8) \
    (void) fputc(DownScale(quantum),image->file); \
  else \
    if ((QuantumDepth-image->depth) > 0) \
      MSBFirstWriteShort((quantum)*257,image->file); \
    else \
      MSBFirstWriteShort(quantum,image->file); \
}

/*
  Image Id's
*/
#define UndefinedId  0
#define ImageMagickId  1
/*
  Image classes:
*/
#define UndefinedClass  0
#define DirectClass  1
#define PseudoClass  2
/*
  Image filters:
*/
#define BoxFilter  0
#define MitchellFilter  1
#define TriangleFilter  2
/*
  Drawing primitives.
*/
#define UndefinedPrimitive  0
#define PointPrimitive  1
#define LinePrimitive  2
#define RectanglePrimitive  3
#define FillRectanglePrimitive  4
#define EllipsePrimitive  5
#define FillEllipsePrimitive  6
#define PolygonPrimitive  7
#define FillPolygonPrimitive  8

#ifdef QuantumLeap
/*
  Color quantum is [0..65535].
*/
#define DownScale(quantum)  (((unsigned int) (quantum)) >> 8)
#define MaxRGB  65535L
#define MaxRunlength  65535L
#define QuantumDepth  16
#define UpScale(quantum)  (((unsigned int) (quantum))*257)
#define XDownScale(color)  ((unsigned int) (color))
#define XUpScale(color)  ((unsigned int) (color))

typedef unsigned short Quantum;
#else
/*
  Color quantum is [0..255].
*/
#define DownScale(quantum)  ((unsigned int) (quantum))
#define MaxRGB  255
#define MaxRunlength  255
#define QuantumDepth  8
#define UpScale(quantum)  ((unsigned int) (quantum))
#define XDownScale(color)  (((unsigned int) (color)) >> 8)
#define XUpScale(color)  (((unsigned int) (color))*257)

typedef unsigned char Quantum;
#endif

/*
  Typedef declarations.
*/
typedef struct _AnnotateInfo
{
  char
    *server_name,
    *font;

  unsigned int
    pointsize;

  char
    *box,
    *pen,
    *geometry,
    *text,
    *primitive;

  unsigned int
    center;
} AnnotateInfo;

typedef struct _ColorPacket
{
  Quantum
    red,
    green,
    blue;

  unsigned char
    flags;

  unsigned short
    index;
} ColorPacket;

typedef struct _FrameInfo
{
  int
    x,
    y;

  unsigned int
    width,
    height;

  int
    inner_bevel,
    outer_bevel;

  ColorPacket
    matte_color,
    highlight_color,
    shadow_color;
} FrameInfo;

typedef struct _ImageInfo
{
  char
    *filename,
    magick[MaxTextLength];

  unsigned int
    assert,
    subimage,
    subrange;

  char
    *server_name,
    *font,
    *size,
    *density,
    *page,
    *texture;

  unsigned int
    dither,
    interlace,
    monochrome,
    quality,
    verbose;

  char
    *undercolor;
} ImageInfo;

typedef struct _PrimitiveInfo
{
  unsigned int
    primitive;

  unsigned int
    coordinates;

  int
    x,
    y;
} PrimitiveInfo;

typedef struct _RectangleInfo
{
  unsigned int
    width,
    height;

  int
    x,
    y;
} RectangleInfo;

typedef struct _RunlengthPacket
{
  Quantum
    red,
    green,
    blue,
    length;

  unsigned short
    index;
} RunlengthPacket;

typedef struct _Image
{
  FILE
    *file;

  int
    status,
    temporary;

  char
    filename[MaxTextLength];

  long int
    filesize;

  int
    pipe;

  char
    magick[MaxTextLength],
    *comments,
    *label,
    *text;

  unsigned int
    id,
    class,
    matte,
    compression,
    columns,
    rows,
    depth,
    interlace,
    scene;

  char
    *montage,
    *directory;

  ColorPacket
    *colormap;

  unsigned int
    colorspace,
    colors;

  double
    gamma;

  short int
    units;

  float
    x_resolution,
    y_resolution;

  unsigned int
    mean_error_per_pixel;

  double
    normalized_mean_error,
    normalized_maximum_error;

  unsigned long
    total_colors;

  char
    *signature;

  RunlengthPacket
    *pixels,
    *packet;

  unsigned int
    packets,
    runlength,
    packet_size;

  unsigned char
    *packed_pixels;

  long int
    magick_time;

  char
    magick_filename[MaxTextLength];

  unsigned int
    magick_columns,
    magick_rows;

  char
    *geometry;

  unsigned int
    orphan;

  struct _Image
    *previous,
    *list,
    *next;
} Image;

/*
  Image utilities routines.
*/
extern void
  CommentImage _Declare((Image *,char *)),
  LabelImage _Declare((Image *,char *));

extern Image
  *AllocateImage _Declare((ImageInfo *)),
  *BorderImage _Declare((Image *,RectangleInfo *,ColorPacket *)),
  *BlurImage _Declare((Image *,double)),
  *ChopImage _Declare((Image *,RectangleInfo *)),
  *CopyImage _Declare((Image *,unsigned int,unsigned int,unsigned int)),
  *CropImage _Declare((Image *,RectangleInfo *)),
  *DespeckleImage _Declare((Image *)),
  *EdgeImage _Declare((Image *,double)),
  *EmbossImage _Declare((Image *)),
  *EnhanceImage _Declare((Image *)),
  *FlipImage _Declare((Image *)),
  *FlopImage _Declare((Image *)),
  *FrameImage _Declare((Image *,FrameInfo *)),
  *MagnifyImage _Declare((Image *)),
  *MinifyImage _Declare((Image *)),
  *NoisyImage _Declare((Image *)),
  *OilPaintImage _Declare((Image *)),
  *ReadImage _Declare((ImageInfo *)),
  *RollImage _Declare((Image *,int,int)),
  *RotateImage _Declare((Image *,double,ColorPacket *,unsigned int,
    unsigned int)),
  *SampleImage _Declare((Image *,unsigned int,unsigned int)),
  *ScaleImage _Declare((Image *,unsigned int,unsigned int)),
  *SharpenImage _Declare((Image *,double)),
  *ShearImage _Declare((Image *,double,double,ColorPacket *,unsigned int)),
  *SpreadImage _Declare((Image *,unsigned int)),
  *StereoImage _Declare((Image *,Image *)),
  *ZoomImage _Declare((Image *,unsigned int,unsigned int,unsigned int));

extern unsigned int
  IsGrayImage _Declare((Image *)),
  IsPseudoClass _Declare((Image *)),
  UncompressImage _Declare((Image *)),
  WriteImage _Declare((ImageInfo *,Image *));

extern void
  AnnotateImage _Declare((Image *,AnnotateInfo *)),
  CloseImage _Declare((Image *)),
  ColormapSignature _Declare((Image *)),
  CompositeImage _Declare((Image *,unsigned int,Image *,int,int)),
  CompressColormap _Declare((Image *)),
  CompressImage _Declare((Image *)),
  ContrastImage _Declare((Image *,unsigned int)),
  DescribeImage _Declare((Image *,FILE *,unsigned int)),
  DestroyImage _Declare((Image *)),
  DestroyImages _Declare((Image *)),
  DrawImage _Declare((Image *,AnnotateInfo *)),
  EqualizeImage _Declare((Image *)),
  GammaImage _Declare((Image *,char *)),
  GetAnnotateInfo _Declare((AnnotateInfo *)),
  GetImageInfo _Declare((ImageInfo *)),
  MapImage _Declare((Image *,Image *,unsigned int)),
  ModulateImage _Declare((Image *,char *)),
  MogrifyImage _Declare((ImageInfo *,int,char **,Image **)),
  NegateImage _Declare((Image *)),
  NormalizeImage _Declare((Image *)),
  NumberColors _Declare((Image *,FILE *)),
  OpaqueImage _Declare((Image *,char *,char *)),
  OpenImage _Declare((Image *,char *)),
  ParseImageGeometry _Declare((char *,unsigned int *,unsigned int *)),
  QuantizationError _Declare((Image *)),
  QuantizeImage _Declare((Image *,unsigned int,unsigned int,unsigned int,
    unsigned int)),
  QuantizeImages _Declare((Image **,unsigned int,unsigned int,unsigned int,
    unsigned int,unsigned int)),
  RaiseImage _Declare((Image *,int,unsigned int)),
  RGBTransformImage _Declare((Image *,unsigned int)),
  SegmentImage _Declare((Image *,unsigned int,unsigned int,double,double)),
  SetImageMagick _Declare((ImageInfo *)),
  SortColormapByIntensity _Declare((Image *)),
  SyncImage _Declare((Image *)),
  TextureImage _Declare((Image *,char *)),
  TransformImage _Declare((Image **,char *,char *)),
  TransformRGBImage _Declare((Image *,unsigned int)),
  TransparentImage _Declare((Image *,char *));


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

