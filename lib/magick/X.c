
/* $Id: X.c 4727 2009-11-16 20:53:54Z kobus $ */

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
%                                                                             %
%                                   X   X                                     %
%                                    X X                                      %
%                                     X                                       %
%                                    X X                                      %
%                                   X   X                                     %
%                                                                             %
%                    X11 Utility Routines for ImageMagick.                    %
%                                                                             %
%                                                                             %
%                               Software Design                               %
%                                 John Cristy                                 %
%                                  July 1992                                  %
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

/* Kobus */
/* Undef to make standard display! */
#define PURE_PIXEL_SUPPORT
/* END Kobus */


/*
  State declarations.
*/
#define ControlState  0x0001
#define DefaultState  0x0000
#define ExitState  0x0002

/*
  Global declarations.
*/
static unsigned int
  xerror_alert = False;


/* Kobus */
#ifdef PURE_PIXEL_SUPPORT
    unsigned char *pure_pixels = NULL;
    static int    num_image_columns = 0;
#endif
/* END Kobus */


/*
  Function prototypes.
*/
int
  Latin1Compare _Declare((char *,char *));

static char
  **FontToList _Declare((char *));

static int
  IntensityCompare _Declare((const void *,const void *)),
  NumberLines _Declare((char *)),
  PopularityCompare _Declare((const void *,const void *)),
  SceneCompare _Declare((const void *,const void *));

static void
  Latin1Upper _Declare((char *)),
  XDitherImage _Declare((Image *,XImage *)),
  XMakeImageLSBFirst _Declare((XResourceInfo *,XWindowInfo *,Image *,XImage *,
    XImage *)),
  XMakeImageMSBFirst _Declare((XResourceInfo *,XWindowInfo *,Image *,XImage *,
    XImage *));

static Window
  XWindowByProperty _Declare((Display *,Window,Atom));

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   I s T r u e                                                               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Function IsTrue returns True if the boolean is "true", "on", "yes" or "1".
%
%  The format of the IsTrue routine is:
%
%      option=IsTrue(boolean)
%
%  A description of each parameter follows:
%
%    o option: either True or False depending on the boolean parameter.
%
%    o boolean: Specifies a pointer to a character array.
%
%
*/
unsigned int IsTrue(char *boolean)
{
  if (boolean == (char *) NULL)
    return(False);
  if (Latin1Compare(boolean,"true") == 0)
    return(True);
  if (Latin1Compare(boolean,"on") == 0)
    return(True);
  if (Latin1Compare(boolean,"yes") == 0)
    return(True);
  if (Latin1Compare(boolean,"1") == 0)
    return(True);
  return(False);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   L a t i n 1 C o m p a r e                                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Function Latin1Compare compares two null terminated Latin-1 strings,
%  ignoring case differences, and returns an integer greater than, equal
%  to, or less than 0, according to whether first is lexicographically
%  greater than, equal to, or less than second.  The two strings are
%  assumed to be encoded using ISO 8859-1.
%
%  The format of the Latin1Compare routine is:
%
%      Latin1Compare(first,second)
%
%  A description of each parameter follows:
%
%    o first: A pointer to the string to convert to Latin1 string.
%
%    o second: A pointer to the string to convert to Latin1 string.
%
%
*/
int Latin1Compare(char *first, char *second)
{
  register unsigned char
   *p,
   *q;

  p=(unsigned char *) first;
  q=(unsigned char *) second;
  while ((*p != '\0') && (*q != '\0'))
  {
    register unsigned char
      c,
      d;

    c=(*p);
    d=(*q);
    if (c != d)
      {
        /*
          Try lowercasing and try again.
        */
        if ((c >= XK_A) && (c <= XK_Z))
          c+=(XK_a-XK_A);
        else
          if ((c >= XK_Agrave) && (c <= XK_Odiaeresis))
            c+=(XK_agrave-XK_Agrave);
          else
            if ((c >= XK_Ooblique) && (c <= XK_Thorn))
              c+=(XK_oslash-XK_Ooblique);
        if ((d >= XK_A) && (d <= XK_Z))
          d+=(XK_a-XK_A);
        else
          if ((d >= XK_Agrave) && (d <= XK_Odiaeresis))
            d+=(XK_agrave-XK_Agrave);
          else if ((d >= XK_Ooblique) && (d <= XK_Thorn))
            d+=(XK_oslash-XK_Ooblique);
        if (c != d)
          return(((int) c)-((int) d));
      }
    p++;
    q++;
  }
  return(((int) *p)-((int) *q));
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   L a t i n 1 U p p e r                                                     %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Function Latin1Upper copies a null terminated string from source to
%  destination (including the null), changing all Latin-1 lowercase letters
%  to uppercase.  The string is assumed to be encoded using ISO 8859-1.
%
%  The format of the Latin1Upper routine is:
%
%      Latin1Upper(string)
%
%  A description of each parameter follows:
%
%    o string: A pointer to the string to convert to upper-case Latin1.
%
%
*/
static void Latin1Upper(char *string)
{
  unsigned char
    c;

  c=(*string);
  while (c != '\0')
  {
    if ((c >= XK_a) && (c <= XK_z))
      *string=c-(XK_a-XK_A);
    else
      if ((c >= XK_agrave) && (c <= XK_odiaeresis))
        *string=c-(XK_agrave-XK_Agrave);
      else
        if ((c >= XK_oslash) && (c <= XK_thorn))
          *string=c-(XK_oslash-XK_Ooblique);
    string++;
    c=(*string);
  }
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   X A n n o t a t e I m a g e                                               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Function XAnnotateImage annotates the image with text.
%
%  The format of the XAnnotateImage routine is:
%
%    status=XAnnotateImage(display,pixel_info,annotate_info,background,image)
%
%  A description of each parameter follows:
%
%    o status: Function XAnnotateImage returns True if the image is
%      successfully annotated with text.  False is returned is there is a
%      memory shortage.
%
%    o display: Specifies a connection to an X server;  returned from
%      XOpenDisplay.
%
%    o pixel_info: Specifies a pointer to a XPixelInfo structure.
%
%    o annotate_info: Specifies a pointer to a XAnnotateInfo structure.
%
%    o background: Specifies whether the background color is included in
%      the annotation.  Must be either True or False;
%
%    o image: Specifies a pointer to a Image structure;  returned from
%      ReadImage.
%
%
*/
unsigned int XAnnotateImage(Display *display, XPixelInfo *pixel_info, XAnnotateInfo *annotate_info, unsigned int background, Image *image)
{
  ColorPacket
    background_color;

  GC
    annotate_context;

  Image
    *annotate_image;

  int
    x,
    y;

  Pixmap
    annotate_pixmap;

  register int
    i;

  register RunlengthPacket
    *p;

  unsigned int
    depth,
    height,
    matte,
    width;

  Window
    root_window;

  XGCValues
    context_values;

  XImage
    *annotate_ximage;

  /*
    Initialize annotated image.
  */
  if (!UncompressImage(image))
    return(False);
  /*
    Initialize annotated pixmap.
  */
  root_window=XRootWindow(display,XDefaultScreen(display));
  depth=XDefaultDepth(display,XDefaultScreen(display));
  annotate_pixmap=XCreatePixmap(display,root_window,annotate_info->width,
    annotate_info->height,(int) depth);
  if (annotate_pixmap == (Pixmap) NULL)
    return(False);
  /*
    Initialize graphics info.
  */
  context_values.background=0;
  context_values.foreground=(unsigned long) (~0);
  context_values.font=annotate_info->font_info->fid;
  annotate_context=XCreateGC(display,root_window,GCBackground | GCFont |
    GCForeground,&context_values);
  if (annotate_context == (GC) NULL)
    return(False);
  /*
    Draw text to pixmap.
  */
  XDrawImageString(display,annotate_pixmap,annotate_context,0,
    (int) annotate_info->font_info->ascent,annotate_info->text,
    strlen(annotate_info->text));
  XFreeGC(display,annotate_context);
  /*
    Initialize annotated X image.
  */
  annotate_ximage=XGetImage(display,annotate_pixmap,0,0,annotate_info->width,
    annotate_info->height,AllPlanes,ZPixmap);
  if (annotate_ximage == (XImage *) NULL)
    return(False);
  XFreePixmap(display,annotate_pixmap);
  /*
    Initialize annotated image.
  */
  annotate_image=AllocateImage((ImageInfo *) NULL);
  if (annotate_image == (Image *) NULL)
    return(False);
  annotate_image->columns=annotate_info->width;
  annotate_image->rows=annotate_info->height;
  annotate_image->packets=annotate_image->columns*annotate_image->rows;
  annotate_image->pixels=(RunlengthPacket *)
    malloc((unsigned int) image->packets*sizeof(RunlengthPacket));
  if (annotate_image->pixels == (RunlengthPacket *) NULL)
    {
      DestroyImage(annotate_image);
      return(False);
    }
  /*
    Transfer annotated X image to image.
  */
  (void) XParseGeometry(annotate_info->geometry,&x,&y,&width,&height);
  p=image->pixels+y*image->columns+x;
  background_color.red=p->red;
  background_color.green=p->green;
  background_color.blue=p->blue;
  background_color.index=Transparent;
  annotate_image->matte=True;
  p=annotate_image->pixels;
  for (y=0; y < annotate_image->rows; y++)
    for (x=0; x < annotate_image->columns; x++)
    {
      p->index=(unsigned short) XGetPixel(annotate_ximage,x,y);
      if (p->index == 0)
        {
          /*
            Set this pixel to the background color.
          */
          p->red=XDownScale(pixel_info->box_color.red);
          p->green=XDownScale(pixel_info->box_color.green);
          p->blue=XDownScale(pixel_info->box_color.blue);
          p->index=Opaque;
          if (annotate_info->stencil == ForegroundStencil)
            {
              p->red=background_color.red;
              p->green=background_color.green;
              p->blue=background_color.blue;
              p->index=Transparent;
            }
        }
      else
        {
          /*
            Set this pixel to the pen color.
          */
          p->red=XDownScale(pixel_info->pen_color.red);
          p->green=XDownScale(pixel_info->pen_color.green);
          p->blue=XDownScale(pixel_info->pen_color.blue);
          p->index=Opaque;
          if (annotate_info->stencil == BackgroundStencil)
            {
              p->red=background_color.red;
              p->green=background_color.green;
              p->blue=background_color.blue;
              p->index=Transparent;
            }
        }
      p->length=0;
      p++;
    }
  XDestroyImage(annotate_ximage);
  /*
    Determine annotate geometry.
  */
  (void) XParseGeometry(annotate_info->geometry,&x,&y,&width,&height);
  if ((width != annotate_image->columns) || (height != annotate_image->rows))
    {
      char
        image_geometry[MaxTextLength];

      /*
        Scale image.
      */
      (void) sprintf(image_geometry,"%ux%u!",width,height);
      TransformImage(&annotate_image,(char *) NULL,image_geometry);
    }
  if (annotate_info->degrees != 0.0)
    {
      double
        normalized_degrees;

      Image
        *rotated_image;

      int
        rotations;

      /*
        Rotate image.
      */
      rotated_image=RotateImage(annotate_image,annotate_info->degrees,
        &background_color,False,True);
      if (rotated_image == (Image *) NULL)
        return(False);
      DestroyImage(annotate_image);
      annotate_image=rotated_image;
      /*
        Annotation is relative to the degree of rotation.
      */
      normalized_degrees=annotate_info->degrees;
      while (normalized_degrees < -45.0)
        normalized_degrees+=360.0;
      for (rotations=0; normalized_degrees > 45.0; rotations++)
        normalized_degrees-=90.0;
      switch (rotations % 4)
      {
        default:
        case 0:
          break;
        case 1:
        {
          /*
            Rotate 90 degrees.
          */
          x-=annotate_image->columns >> 1;
          y+=annotate_image->columns >> 1;
          break;
        }
        case 2:
        {
          /*
            Rotate 180 degrees.
          */
          x-=annotate_image->columns;
          break;
        }
        case 3:
        {
          /*
            Rotate 270 degrees.
          */
          x-=annotate_image->columns >> 1;
          y-=annotate_image->rows-(annotate_image->columns >> 1);
          break;
        }
      }
    }
  /*
    Composite text onto the image.
  */
  p=annotate_image->pixels;
  for (i=0; i < annotate_image->packets; i++)
  {
    if (p->index != Transparent)
      p->index=Opaque;
    p++;
  }
  matte=image->matte;
  CompositeImage(image,OverCompositeOp,annotate_image,x,y);
  image->matte=matte;
  DestroyImage(annotate_image);
  return(True);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   X B e s t F o n t                                                         %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Function XBestFont returns the "best" font.  "Best" is defined as a font
%  specified in the X resource database or a font such that the text width
%  displayed with the font does not exceed the specified maximum width.
%
%  The format of the XBestFont routine is:
%
%      font=XBestFont(display,resource_info,text_font)
%
%  A description of each parameter follows:
%
%    o font: XBestFont returns a pointer to a XFontStruct structure.
%
%    o display: Specifies a connection to an X server;  returned from
%      XOpenDisplay.
%
%    o resource_info: Specifies a pointer to a X11 XResourceInfo structure.
%
%    o text_font:  True is font should be mono-spaced (typewriter style).
%
%
*/

static char **FontToList(char *font)
{
  char
    **fontlist;

  register char
    *p,
    *q;

  register int
    i;

  unsigned int
    fonts;

  if (font == (char *) NULL)
    return((char **) NULL);
  /*
    Convert string to an ASCII list.
  */
  fonts=1;
  for (p=font; *p != '\0'; p++)
    if (*p == ':')
      fonts++;
  fontlist=(char **) malloc((fonts+1)*sizeof(char *));
  if (fontlist == (char **) NULL)
    {
      Warning("Unable to convert font","Memory allocation failed");
      return((char **) NULL);
    }
  p=font;
  for (i=0; i < fonts; i++)
  {
    for (q=p; *q != '\0'; q++)
      if (*q == ':')
        break;
    fontlist[i]=(char *) malloc((q-p+1)*sizeof(char));
    if (fontlist[i] == (char *) NULL)
      {
        Warning("Unable to convert font","Memory allocation failed");
        return((char **) NULL);
      }
    (void) strncpy(fontlist[i],p,q-p);
    fontlist[i][q-p]='\0';
    p=q+1;
  }
  fontlist[i]=(char *) NULL;
  return(fontlist);
}

XFontStruct *XBestFont(Display *display, XResourceInfo *resource_info, unsigned int text_font)
{
  static char
    *Fonts[]=
    {
      "-adobe-helvetica-medium-r-normal-*-14-*",
      "-*-helvetica-medium-r-*-*-14-*",
      "-*-lucida-medium-r-*-*-14-*",
      "8x13",
      "6x13",
      "fixed",
      "variable",
      (char *) NULL
    },
    *TextFonts[]=
    {
      "-adobe-courier-medium-r-*-*-14-*",
      "-*-fixed-medium-r-normal-*-14-*",
      "-*-fixed-medium-r-*-*-14-*",
      "8x13",
      "6x13",
      "fixed",
      (char *) NULL
    };

  char
    *font_name,
    **p;

  XFontStruct
    *font_info;

  font_info=(XFontStruct *) NULL;
  font_name=resource_info->font;
  if (text_font)
    font_name=resource_info->text_font;
  if (font_name != (char *) NULL)
    {
      char
        **fontlist;

      register int
        i;

      /*
        Load preferred font specified in the X resource database.
      */
      fontlist=FontToList(font_name);
      if (fontlist != (char **) NULL)
        {
          for (i=0; fontlist[i] != (char *) NULL; i++)
          {
            if (font_info == (XFontStruct *) NULL)
              font_info=XLoadQueryFont(display,fontlist[i]);
            free((char *) fontlist[i]);
          }
          free((char *) fontlist);
        }
      if (font_info == (XFontStruct *) NULL)
        Warning("Unable to load font",font_name);
    }
  /*
    Load fonts from list of fonts until one is found.
  */
  p=Fonts;
  if (text_font)
    p=TextFonts;
  while (*p != (char *) NULL)
  {
    if (font_info != (XFontStruct *) NULL)
      break;
    font_info=XLoadQueryFont(display,*p);
    p++;
  }
  return(font_info);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   X B e s t I c o n S i z e                                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Function XBestIconSize returns the "best" icon size.  "Best" is defined as
%  an icon size that maintains the aspect ratio of the image.  If the window
%  manager has preferred icon sizes, one of the preferred sizes is used.
%
%  The format of the XBestIconSize routine is:
%
%      XBestIconSize(display,window,image)
%
%  A description of each parameter follows:
%
%    o display: Specifies a connection to an X server;  returned from
%      XOpenDisplay.
%
%    o image: Specifies a pointer to a Image structure;  returned from
%      ReadImage.
%
%
*/
void XBestIconSize(Display *display, XWindowInfo *window, Image *image)
{
#define MaxIconSize  96

  int
    i,
    number_sizes;

  unsigned int
    height,
    icon_height,
    icon_width,
    width;

  unsigned long
    scale_factor;

  Window
    root_window;

  XIconSize
    *icon_size,
    *size_list;

  /*
    Determine if the window manager has specified preferred icon sizes.
  */
  window->width=MaxIconSize;
  window->height=MaxIconSize;
  icon_size=(XIconSize *) NULL;
  number_sizes=0;
  root_window=XRootWindow(display,window->screen);
  if (XGetIconSizes(display,root_window,&size_list,&number_sizes) != 0)
    if ((number_sizes > 0) && (size_list != (XIconSize *) NULL))
      icon_size=size_list;
  if (icon_size == (XIconSize *) NULL)
    {
      /*
        Window manager does not restrict icon size.
      */
      icon_size=XAllocIconSize();
      if (icon_size == (XIconSize *) NULL)
        {
          Warning("Unable to choose best icon size","Memory allocation failed");
          return;
        }
      icon_size->min_width=1;
      icon_size->max_width=MaxIconSize;
      icon_size->min_height=1;
      icon_size->max_height=MaxIconSize;
      icon_size->width_inc=1;
      icon_size->height_inc=1;
    }
  /*
    Determine aspect ratio of image.
  */
  width=image->columns;
  height=image->rows;
  if (window->crop_geometry)
    (void) XParseGeometry(window->crop_geometry,&i,&i,&width,&height);
  /*
    Look for an icon size that maintains the aspect ratio of image.
  */
  scale_factor=UpShift(icon_size->max_width)/width;
  if (scale_factor > (UpShift(icon_size->max_height)/height))
    scale_factor=UpShift(icon_size->max_height)/height;
  icon_width=icon_size->min_width;
  while (icon_width < icon_size->max_width)
  {
    if (icon_width >= (DownShift(width*scale_factor)))
      break;
    icon_width+=icon_size->width_inc;
  }
  icon_height=icon_size->min_height;
  while (icon_height < icon_size->max_height)
  {
    if (icon_height >= (DownShift(height*scale_factor)))
      break;
    icon_height+=icon_size->height_inc;
  }
  XFree((void *) icon_size);
  window->width=icon_width;
  window->height=icon_height;
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   X B e s t P i x e l                                                       %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Function XBestPixel returns a pixel from an array of pixels that is closest
%  to the requested color.  If the color array is NULL, the colors are obtained
%  from the X server.
%
%  The format of the XBestPixel routine is:
%
%      pixel=XBestPixel(display,colormap,colors,number_colors,color)
%
%  A description of each parameter follows:
%
%    o pixel: XBestPixel returns the pixel value closest to the requested
%      color.
%
%    o display: Specifies a connection to an X server;  returned from
%      XOpenDisplay.
%
%    o colormap: Specifies the ID of the X server colormap.
%
%    o colors: Specifies an array of XColor structures.
%
%    o number_colors: Specifies the number of XColor structures in the
%      color definition array.
%
%    o color: Specifies the desired RGB value to find in the colors array.
%
%
*/
void XBestPixel(Display *display, Colormap colormap, XColor *colors, unsigned int number_colors, XColor *color)
{
  double
    distance_squared,
    min_distance;

  int
    distance,
    query_server,
    status;

  register int
    i,
    j;

  /*
    Find closest representation for the requested RGB color.
  */
  status=XAllocColor(display,colormap,color);
  if (status != 0)
    return;
  query_server=colors == (XColor *) NULL;
  if (query_server)
    {
      /*
        Read X server colormap.
      */
      colors=(XColor *) malloc(number_colors*sizeof(XColor));
      if (colors == (XColor *) NULL)
        Error("Unable to read X server colormap","Memory allocation failed");
      for (i=0; i < number_colors; i++)
        colors[i].pixel=(unsigned long) i;
      if (number_colors > 256)
        number_colors=256;
      XQueryColors(display,colormap,colors,number_colors);
    }
  min_distance=3.0*65536.0*65536.0;
  color->pixel=colors[0].pixel;
  j=0;
  for (i=0; i < number_colors; i++)
  {
    distance=(int) colors[i].red-(int) color->red;
    distance_squared=(unsigned int) (distance*distance);
    distance=(int) colors[i].green-(int) color->green;
    distance_squared+=(unsigned int) (distance*distance);
    distance=(int) colors[i].blue-(int) color->blue;
    distance_squared+=(unsigned int) (distance*distance);
    if (distance_squared < min_distance)
      {
        min_distance=distance_squared;
        color->pixel=colors[i].pixel;
        j=i;
      }
  }
  (void) XAllocColor(display,colormap,&colors[j]);
  if (query_server)
    free((char *) colors);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   X B e s t V i s u a l I n f o                                             %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Function XBestVisualInfo returns visual information for a visual that is
%  the "best" the server supports.  "Best" is defined as:
%
%    1. Restrict the visual list to those supported by the default screen.
%
%    2. If a visual type is specified, restrict the visual list to those of
%       that type.
%
%    3. If a map type is specified, choose the visual that matches the id
%       specified by the Standard Colormap.
%
%    4  From the list of visuals, choose one that can display the most
%       simultaneous colors.  If more than one visual can display the same
%       number of simultaneous colors, one is choosen based on a rank.
%
%  The format of the XBestVisualInfo routine is:
%
%      visual_info=XBestVisualInfo(display,map_info,resource_info)
%
%  A description of each parameter follows:
%
%    o visual_info: XBestVisualInfo returns a pointer to a X11 XVisualInfo
%      structure.
%
%    o display: Specifies a connection to an X server;  returned from
%      XOpenDisplay.
%
%    o map_info: If map_type is specified, this structure is initialized
%      with info from the Standard Colormap.
%
%    o resource_info: Specifies a pointer to a X11 XResourceInfo structure.
%
%
*/
XVisualInfo *XBestVisualInfo(Display *display, XStandardColormap *map_info, XResourceInfo *resource_info)
{
#define MaxStandardColormaps  7
#define XVisualColormapSize(visual_info) \
  ((visual_info->class == TrueColor) || (visual_info->class == DirectColor) ? \
    visual_info->red_mask | visual_info->green_mask | visual_info->blue_mask : \
    visual_info->colormap_size)

  char
    *map_type,
    *visual_type;

  register int
    i;

  static int
    number_visuals;

  static XVisualInfo
    visual_template;

  unsigned int
    visual_mask;

  XVisualInfo
    *visual_info,
    *visual_list;

  /*
    Restrict visual search by screen number.
  */
  map_type=resource_info->map_type;
  visual_type=resource_info->visual_type;
  visual_mask=VisualScreenMask;
  visual_template.screen=XDefaultScreen(display);
  if (visual_type != (char *) NULL)
    {
      /*
        Restrict visual search by class or visual id.
      */
      if (Latin1Compare("staticgray",visual_type) == 0)
        {
          visual_mask|=VisualClassMask;
          visual_template.class=StaticGray;
        }
      else
        if (Latin1Compare("grayscale",visual_type) == 0)
          {
            visual_mask|=VisualClassMask;
            visual_template.class=GrayScale;
          }
        else
          if (Latin1Compare("staticcolor",visual_type) == 0)
            {
              visual_mask|=VisualClassMask;
              visual_template.class=StaticColor;
            }
          else
            if (Latin1Compare("pseudocolor",visual_type) == 0)
              {
                visual_mask|=VisualClassMask;
                visual_template.class=PseudoColor;
              }
            else
              if (Latin1Compare("truecolor",visual_type) == 0)
                {
                  visual_mask|=VisualClassMask;
                  visual_template.class=TrueColor;
                }
              else
                if (Latin1Compare("directcolor",visual_type) == 0)
                  {
                    visual_mask|=VisualClassMask;
                    visual_template.class=DirectColor;
                  }
                else
                  if (Latin1Compare("default",visual_type) == 0)
                    {
                      visual_mask|=VisualIDMask;
                      visual_template.visualid=XVisualIDFromVisual(
                        XDefaultVisual(display,XDefaultScreen(display)));
                    }
                  else
                    if (isdigit(*visual_type))
                      {
                        visual_mask|=VisualIDMask;
                        visual_template.visualid=
                          strtol(visual_type,(char **) NULL,0);
                      }
                    else
                      Warning("Invalid visual specifier",visual_type);
    }
  /*
    Get all visuals that meet our criteria so far.
  */
  number_visuals=0;
  visual_list=XGetVisualInfo(display,visual_mask,&visual_template,
    &number_visuals);
  visual_mask=VisualScreenMask | VisualIDMask;
  if ((number_visuals == 0) || (visual_list == (XVisualInfo *) NULL))
    {
      /*
        Failed to get visual;  try using the default visual.
      */
      Warning("Unable to get visual",visual_type);
      visual_template.visualid=
        XVisualIDFromVisual(XDefaultVisual(display,XDefaultScreen(display)));
      visual_list=XGetVisualInfo(display,visual_mask,&visual_template,
        &number_visuals);
      if ((number_visuals == 0) || (visual_list == (XVisualInfo *) NULL))
        return((XVisualInfo *) NULL);
      Warning("Using default visual",XVisualClassName(visual_list->class));
    }
  resource_info->color_recovery=False;
  if ((map_info != (XStandardColormap *) NULL) && (map_type != (char *) NULL))
    {
      Atom
        map_property;

      char
        map_name[MaxTextLength];

      int
        j,
        number_maps,
        status;

      Window
        root_window;

      XStandardColormap
        *map_list;

      /*
        Choose a visual associated with a standard colormap.
      */
      root_window=XRootWindow(display,XDefaultScreen(display));
      status=0;
      if (strcmp(map_type,"list") != 0)
        {
          /*
            User specified Standard Colormap.
          */
          (void) sprintf((char *) map_name,"RGB_%s_MAP",map_type);
          Latin1Upper(map_name);
          map_property=XInternAtom(display,(char *) map_name,True);
          if (map_property == (Atom) NULL)
            Error("Unable to get Standard Colormap",map_type);
          status=XGetRGBColormaps(display,root_window,&map_list,&number_maps,
            map_property);
        }
      else
        {
          static char
            *colormap[]=
            {
              "_HP_RGB_SMOOTH_MAP_LIST",
              "RGB_BEST_MAP",
              "RGB_DEFAULT_MAP",
              "RGB_GRAY_MAP",
              "RGB_RED_MAP",
              "RGB_GREEN_MAP",
              "RGB_BLUE_MAP",
            };

          /*
            Choose a standard colormap from a list.
          */
          for (i=0; i < MaxStandardColormaps; i++)
          {
            map_property=XInternAtom(display,colormap[i],True);
            if (map_property == (Atom) NULL)
              continue;
            status=XGetRGBColormaps(display,root_window,&map_list,&number_maps,
              map_property);
            if (status != 0)
              break;
          }
          resource_info->color_recovery=(i == 0);  /* _HP_RGB_SMOOTH_MAP_LIST */
        }
      if (status == 0)
        Error("Unable to get Standard Colormap",map_type);
      /*
        Search all Standard Colormaps and visuals for ids that match.
      */
      *map_info=map_list[0];
#ifndef PRE_R4_ICCCM
      visual_template.visualid=XVisualIDFromVisual(visual_list[0].visual);
      for (i=0; i < number_maps; i++)
        for (j=0; j < number_visuals; j++)
          if (map_list[i].visualid ==
              XVisualIDFromVisual(visual_list[j].visual))
            {
              *map_info=map_list[i];
              visual_template.visualid=
                XVisualIDFromVisual(visual_list[j].visual);
              break;
            }
      if (map_info->visualid != visual_template.visualid)
        Error("Unable to match visual to Standard Colormap",map_type);
#endif
      if (map_info->colormap == (Colormap) NULL)
        Error("Standard Colormap is not initialized",map_type);
      XFree((void *) map_list);
    }
  else
    {
      static unsigned int
        rank[]=
          {
            StaticGray,
            GrayScale,
            StaticColor,
            DirectColor,
            TrueColor,
            PseudoColor
          };

      XVisualInfo
        *p;

      /*
        Pick one visual that displays the most simultaneous colors.
      */
      visual_info=visual_list;
      p=visual_list;
      for (i=1; i < number_visuals; i++)
      {
        p++;
        if (XVisualColormapSize(p) > XVisualColormapSize(visual_info))
          visual_info=p;
        else
          if (XVisualColormapSize(p) == XVisualColormapSize(visual_info))
            if (rank[p->class] > rank[visual_info->class])
              visual_info=p;
      }
      visual_template.visualid=XVisualIDFromVisual(visual_info->visual);
    }
  XFree((void *) visual_list);
  /*
    Retrieve only one visual by its screen & id number.
  */
  visual_info=XGetVisualInfo(display,visual_mask,&visual_template,
    &number_visuals);
  if ((number_visuals == 0) || (visual_info == (XVisualInfo *) NULL))
    return((XVisualInfo *) NULL);
  return(visual_info);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   X C h e c k R e f r e s h W i n d o w s                                   %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Function XCheckRefreshWindows checks the X server for exposure events for
%  a particular window and updates the area associated withe exposure event.
%
%  The format of the XCheckRefreshWindows routine is:
%
%      XCheckRefreshWindows(display,windows)
%
%  A description of each parameter follows:
%
%    o display: Specifies a connection to an X server;  returned from
%      XOpenDisplay.
%
%    o windows: Specifies a pointer to a XWindows structure.
%
%
*/
void XCheckRefreshWindows(Display *display, XWindows *windows)
{
  XEvent
    event;

  XDelay(display,SuspendTime);
  while (XCheckTypedWindowEvent(display,windows->command.id,Expose,&event))
    (void) XCommandWidget(display,windows,(char **) NULL,&event);
  while (XCheckTypedWindowEvent(display,windows->image.id,Expose,&event))
    XRefreshWindow(display,&windows->image,&event);
  XDelay(display,SuspendTime << 1);
  while (XCheckTypedWindowEvent(display,windows->command.id,Expose,&event))
    (void) XCommandWidget(display,windows,(char **) NULL,&event);
  while (XCheckTypedWindowEvent(display,windows->image.id,Expose,&event))
    XRefreshWindow(display,&windows->image,&event);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   X C l i e n t M e s s a g e                                               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Function XClientMessage sends a message to a window with XSendEvent.  The
%  message is initialized with a particular protocol type and atom.
%
%  The format of the XClientMessage function is:
%
%      XClientMessage(display,window,protocol,message,timestamp)
%
%  A description of each parameter follows:
%
%    o display: Specifies a pointer to the Display structure;  returned from
%      XOpenDisplay.
%
%    o window: Specifies a pointer to a Window structure.
%
%    o protocol: Specifies an atom value.
%
%    o message: Specifies an atom value which is the message to send.
%
%    o timestamp: Specifies a value of type Time.
%
%
*/
void XClientMessage(Display *display, Window window, Atom protocol, Atom message, Time timestamp)
{
  XClientMessageEvent
    client_event;

  client_event.type=ClientMessage;
  client_event.window=window;
  client_event.message_type=protocol;
  client_event.format=32;
  client_event.data.l[0]=message;
  client_event.data.l[1]=timestamp;
  XSendEvent(display,window,False,NoEventMask,(XEvent *) &client_event);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   X C l i e n t W i n d o w                                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Function XClientWindow finds a window, at or below the specified window,
%  which has a WM_STATE property.  If such a window is found, it is returned,
%  otherwise the argument window is returned.
%
%  The format of the XClientWindow function is:
%
%      client_window=XClientWindow(display,target_window)
%
%  A description of each parameter follows:
%
%    o client_window: XClientWindow returns a window, at or below the specified
%      window, which has a WM_STATE property otherwise the argument
%      target_window is returned.
%
%    o display: Specifies a pointer to the Display structure;  returned from
%      XOpenDisplay.
%
%    o target_window: Specifies the window to find a WM_STATE property.
%
%
*/
Window XClientWindow(Display *display, Window target_window)
{
  Atom
    state,
    type;

  int
    format,
    status;

  unsigned char
    *data;

  unsigned long
    after,
    number_items;

  Window
    client_window;

  state=XInternAtom(display,"WM_STATE",True);
  if (state == (Atom) NULL)
    return(target_window);
  type=(Atom) NULL;
  status=XGetWindowProperty(display,target_window,state,0L,0L,False,
    (Atom) AnyPropertyType,&type,&format,&number_items,&after,&data);
  if ((status == Success) && (type != (Atom) NULL))
    return(target_window);
  client_window=XWindowByProperty(display,target_window,state);
  if (client_window == (Window) NULL)
    return(target_window);
  return(client_window);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   X C o n s t r a i n W i n d o w P o s i t i o n                           %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Function XConstrainWindowPosition assures a window is positioned witin the
%  X server boundaries.
%
%  The format of the XConstrainWindowPosition routine is:
%
%      XConstrainWindowPosition(display,window_info)
%
%  A description of each parameter follows:
%
%    o display: Specifies a pointer to the Display structure;  returned from
%      XOpenDisplay.
%
%    o window_info: Specifies a pointer to a XWindowInfo structure.
%
%
*/
void XConstrainWindowPosition(Display *display, XWindowInfo *window_info)
{
  unsigned int
    limit;

  limit=XDisplayWidth(display,window_info->screen)-window_info->width;
  if (window_info->x < 0)
    window_info->x=0;
  else
    if (window_info->x > limit)
      window_info->x=limit;
  limit=XDisplayHeight(display,window_info->screen)-window_info->height;
  if (window_info->y < 0)
    window_info->y=0;
  else
    if (window_info->y > limit)
      window_info->y=limit;
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   X D e l a y                                                               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Function XDelay suspends program execution for the number of milliseconds
%  specified.
%
%  The format of the Delay routine is:
%
%      XDelay(display,milliseconds)
%
%  A description of each parameter follows:
%
%    o display: Specifies a pointer to the Display structure;  returned from
%      XOpenDisplay.
%
%    o milliseconds: Specifies the number of milliseconds to delay before
%      returning.
%
%
*/
void XDelay(Display *display, long unsigned int milliseconds)
{
  (void) XFlush(display);
  if (milliseconds == 0)
    return;
#ifndef vms
#ifdef sysv
  {
#include <sys/poll.h>

    (void) poll((struct pollfd *) NULL,0,(int) milliseconds);
  }
#else
  {
    struct timeval
      timer;

    timer.tv_sec=milliseconds/1000;
    timer.tv_usec=(milliseconds % 1000)*1000;

    /* Kobus - this does not work on HP ! */
    /*
    // (void) select(0,(XFD_SET *) NULL,(XFD_SET *) NULL,(XFD_SET *) NULL,&timer);
    */
    if (timer.tv_sec > 0) sleep(timer.tv_sec);
    if (timer.tv_usec > 0) usleep(timer.tv_sec);
  }
#endif
#else
  {
    float
      timer;

    timer=milliseconds/1000.0;
    lib$wait(&timer);
  }
#endif
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   X D e s t r o y W i n d o w C o l o r s                                   %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Function XDestroyWindowColors frees X11 color resources previously saved on
%  a window by XRetainWindowColors or programs like xsetroot.
%
%  The format of the XDestroyWindowColors routine is:
%
%      XDestroyWindowColors(display,window)
%
%  A description of each parameter follows:
%
%    o display: Specifies a connection to an X server; returned from
%      XOpenDisplay.
%
%    o window: Specifies a pointer to a Window structure.
%
%
*/
void XDestroyWindowColors(Display *display, Window window)
{
  Atom
    property,
    type;

  int
    format,
    status;

  unsigned char
    *data;

  unsigned long
    after,
    length;

  /*
    If there are previous resources on the root window, destroy them.
  */
  property=XInternAtom(display,"_XSETROOT_ID",False);
  if (property == (Atom) NULL)
    {
      Warning("Unable to create X property","_XSETROOT_ID");
      return;
    }
  status=XGetWindowProperty(display,window,property,0L,1L,True,
    (Atom) AnyPropertyType,&type,&format,&length,&after,&data);
  if (status != Success)
    return;
  if ((type == XA_PIXMAP) && (format == 32) && (length == 1) && (after == 0))
    {
      XKillClient(display,(XID) (*((Pixmap *) data)));
      XDeleteProperty(display,window,property);
    }
  if (type != None)
    XFree((void *) data);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   X D i s p l a y I m a g e I n f o                                         %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Function XDisplayImageInfo displays information about an X image.
%
%  The format of the XDisplayImageInfo routine is:
%
%      XDisplayImageInfo(display,resource_info,windows,undo_image,image)
%
%  A description of each parameter follows:
%
%    o display: Specifies a connection to an X server;  returned from
%      XOpenDisplay.
%
%    o resource_info: Specifies a pointer to a X11 XResourceInfo structure.
%
%    o windows: Specifies a pointer to a XWindows structure.
%
%    o undo_image: Specifies a pointer to a Image structure;  returned from
%      ReadImage.
%
%    o image: Specifies a pointer to a Image structure;  returned from
%      ReadImage.
%
%
*/
void XDisplayImageInfo(Display *display, XResourceInfo *resource_info, XWindows *windows, Image *undo_image, Image *image)
{
  char
    *text,
    **textlist,
    title[MaxTextLength];

  int
    length;

  Image
    *p;

  unsigned int
    bytes,
    count,
    levels;

  /*
    Display information about the image in the Text View widget.
  */
  length=50*MaxTextLength;
  if (image->directory != (char *) NULL)
    length+=strlen(image->directory);
  if (image->comments != (char *) NULL)
    length+=strlen(image->comments);
  text=(char *) malloc(length*sizeof(char));
  if (text == (char *) NULL)
    {
      XNoticeWidget(display,windows,"Unable to display image info:",
        "Memory allocation failed");
      return;
    }
  *text='\0';
  /*
    Display info about the X server.
  */
  (void) sprintf(title," Image Info: %s",image->filename);
  if (resource_info->gamma_correct)
    if (resource_info->display_gamma != (char *) NULL)
      (void) sprintf(text,"%sDisplay\n  gamma: %s\n\n",text,
        resource_info->display_gamma);
  /*
    Display info about the X image.
  */
  (void) sprintf(text,"%sX\n  visual: %s\n",text,
    XVisualClassName(windows->image.class));
  (void) sprintf(text,"%s  depth: %d\n",text,windows->image.ximage->depth);
  if (windows->image.visual_info->colormap_size != 0)
    (void) sprintf(text,"%s  colormap size: %d\n",text,
      windows->image.visual_info->colormap_size);
  if (resource_info->colormap== SharedColormap)
    (void) strcat(text,"  colormap type: Shared\n");
  else
    (void) strcat(text,"  colormap type: Private\n");
  (void) sprintf(text,"%s  geometry: %dx%d\n",text,
    windows->image.ximage->width,windows->image.ximage->height);
  if (windows->image.crop_geometry != (char *) NULL)
    (void) sprintf(text,"%s  crop geometry: %s\n",text,
      windows->image.crop_geometry);
  if (windows->image.pixmap == (Pixmap) NULL)
    (void) strcat(text,"  type: X Image\n");
  else
    (void) strcat(text,"  type: Pixmap\n");
  if (windows->image.shared_memory)
    (void) strcat(text,"  shared memory: True\n");
  else
    (void) strcat(text,"  shared memory: False\n");
  (void) strcat(text,"\n");
  /*
    Display info about the undo image cache.
  */
  bytes=0;
  for (levels=0; undo_image != (Image *) NULL; levels++)
  {
    bytes+=undo_image->list->packets*sizeof(RunlengthPacket);
    undo_image=undo_image->previous;
  }
  (void) sprintf(text,"%sUndo Edit Cache\n  levels: %u\n",text,levels);
  (void) sprintf(text,"%s  bytes: %umb\n",text,(bytes+(1 << 19)) >> 20);
  (void) sprintf(text,"%s  limit: %umb\n\n",text,resource_info->undo_cache);
  /*
    Display info about the image.
  */
  (void) sprintf(text,"%sImage\n  file: %s\n",text,image->filename);
  if (image->class == DirectClass)
    (void) strcat(text,"  class: DirectClass\n");
  else
    (void) strcat(text,"  class: PseudoClass\n");
  if (image->class == PseudoClass)
    {
      ColorPacket
        *p;

      register int
        i;

      register XColorList
        *q;

      /*
        Display image colormap.
      */
      if (image->total_colors <= image->colors)
        (void) sprintf(text,"%s  colors: %u\n",text,image->colors);
      else
        (void) sprintf(text,"%s  colors: %lu=>%u\n",text,image->total_colors,
          image->colors);
      p=image->colormap;
      for (i=0; i < image->colors; i++)
      {
        (void) sprintf(text,"%s    %d: (%3d,%3d,%3d)  #%02x%02x%02x",
          text,i,p->red,p->green,p->blue,(unsigned int) p->red,
          (unsigned int) p->green,(unsigned int) p->blue);
        for (q=ColorList; q->name != (char *) NULL; q++)
          if ((DownScale(p->red) == q->red) &&
              (DownScale(p->green) == q->green) &&
              (DownScale(p->blue) == q->blue))
            (void) sprintf(text,"%s  %s",text,q->name);
        (void) strcat(text,"\n");
        p++;
      }
    }
  if (image->signature != (char *) NULL)
    (void) sprintf(text,"%s  signature: %s\n",text,image->signature);
  if (image->matte)
    (void) strcat(text,"  matte: True\n");
  else
    (void) strcat(text,"  matte: False\n");
  if (image->packets < (image->columns*image->rows))
    (void) sprintf(text,"%s  runlength packets: %u of %u\n",text,image->packets,
      image->columns*image->rows);
  (void) sprintf(text,"%s  geometry: %ux%u\n",text,
    image->columns,image->rows);
  (void) sprintf(text,"%s  depth: %u\n",text,image->depth);
  if (image->filesize != 0)
    (void) sprintf(text,"%s  bytes: %ld\n",text,image->filesize);
  if (image->matte)
    (void) strcat(text,"  interlaced: True\n");
  else
    (void) strcat(text,"  interlaced: False\n");
  if (image->gamma != 0.0)
    (void) sprintf(text,"%s  gamma: %f\n",text,image->gamma);
  (void) sprintf(text,"%s  format: %s\n",text,image->magick);
  p=image;
  while (p->previous != (Image *) NULL)
    p=p->previous;
  for (count=1; p->next != (Image *) NULL; count++)
    p=p->next;
  if (count > 1)
    (void) sprintf(text,"%s  scene: %u of %u\n",text,image->scene,count);
  else
    if (image->scene != 0)
      (void) sprintf(text,"%s  scene: %u\n",text,image->scene);
  if (image->montage != (char *) NULL)
    (void) sprintf(text,"%s  montage: %s\n",text,image->montage);
  if (image->directory != (char *) NULL)
    (void) sprintf(text,"%s  directory:\n\n%s\n",text,image->directory);
  if (image->label != (char *) NULL)
    (void) sprintf(text,"%s  label: %s\n",text,image->label);
  if (image->comments != (char *) NULL)
    (void) sprintf(text,"%s  comments:\n\n%s",text,image->comments);
  textlist=StringToList(text);
  if (textlist != (char **) NULL)
    {
      register int
        i;

      XTextViewWidget(display,resource_info,windows,True,title,textlist);
      for (i=0; textlist[i] != (char *) NULL; i++)
        free((char *) textlist[i]);
      free((char *) textlist);
    }
  free((char *) text);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%     X D i t h e r I m a g e                                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Function XDitherImage dithers the reference image as required by the HP
%  Color Recovery algorithm.  The color values are quantized to 3 bits of red
%  and green, and 2 bits of blue (3/3/2) and can be used as indices into a
%  8-bit X standard colormap.
%
%  The format of the XDitherImage routine is:
%
%      XDitherImage(image,ximage)
%
%  A description of each parameter follows:
%
%    o image: Specifies a pointer to a Image structure;  returned from
%      ReadImage.
%
%    o ximage: Specifies a pointer to a XImage structure;  returned from
%      XCreateImage.
%
%
*/
static void XDitherImage(Image *image, XImage *ximage)
{
  static short int
    dither_red[2][16]=
    {
      {-16,  4, -1, 11,-14,  6, -3,  9,-15,  5, -2, 10,-13,  7, -4,  8},
      { 15, -5,  0,-12, 13, -7,  2,-10, 14, -6,  1,-11, 12, -8,  3, -9}
    },
    dither_green[2][16]=
    {
      { 11,-15,  7, -3,  8,-14,  4, -2, 10,-16,  6, -4,  9,-13,  5, -1},
      {-12, 14, -8,  2, -9, 13, -5,  1,-11, 15, -7,  3,-10, 12, -6,  0}
    },
    dither_blue[2][16]=
    {
      { -3,  9,-13,  7, -1, 11,-15,  5, -4,  8,-14,  6, -2, 10,-16,  4},
      {  2,-10, 12, -8,  0,-12, 14, -6,  3, -9, 13, -7,  1,-11, 15, -5}
    };

  ColorPacket
    color;

  int
    value,
    y;

  register char
    *q;

  register int
    i,
    j,
    x;

  register RunlengthPacket
    *p;

  unsigned char
    *blue_map[2][16],
    *green_map[2][16],
    *red_map[2][16];

  unsigned int
    scanline_pad;

  register unsigned long
    pixel;

  if (!UncompressImage(image))
    return;
  /*
    Allocate and initialize dither maps.
  */
  for (i=0; i < 2; i++)
    for (j=0; j < 16; j++)
    {
      red_map[i][j]=(unsigned char *) malloc(256*sizeof(unsigned char));
      green_map[i][j]=(unsigned char *) malloc(256*sizeof(unsigned char));
      blue_map[i][j]=(unsigned char *) malloc(256*sizeof(unsigned char));
      if ((red_map[i][j] == (unsigned char *) NULL) ||
          (green_map[i][j] == (unsigned char *) NULL) ||
          (blue_map[i][j] == (unsigned char *) NULL))
        {
          Warning("Unable to dither image","Memory allocation failed");
          return;
        }
    }
  /*
    Initialize dither tables.
  */
  for (i=0; i < 2; i++)
    for (j=0; j < 16; j++)
      for (x=0; x < 256; x++)
      {
        value=x-16;
        if (x < 48)
          value=x/2+8;
        value+=dither_red[i][j];
        if (value < 0)
          value=0;
        else
          if (value > MaxRGB)
            value=MaxRGB;
        red_map[i][j][x]=value;
        value=x-16;
        if (x < 48)
          value=x/2+8;
        value+=dither_green[i][j];
        if (value < 0)
          value=0;
        else
          if (value > MaxRGB)
            value=MaxRGB;
        green_map[i][j][x]=value;
        value=x-32;
        if (x < 112)
          value=x/2+24;
        value+=(dither_blue[i][j] << 1);
        if (value < 0)
          value=0;
        else
          if (value > MaxRGB)
            value=MaxRGB;
        blue_map[i][j][x]=value;
      }
  /*
    Dither image.
  */
  scanline_pad=ximage->bytes_per_line-
    ((ximage->width*ximage->bits_per_pixel) >> 3);
  i=0;
  j=0;
  p=image->pixels;
  q=ximage->data;
  for (y=0; y < image->rows; y++)
  {
    for (x=0; x < image->columns; x++)
    {
      color.red=red_map[i][j][p->red];
      color.green=green_map[i][j][p->green];
      color.blue=blue_map[i][j][p->blue];
      pixel=((color.red & 0xe0) |
        ((unsigned char) (color.green & 0xe0) >> 3) |
        ((unsigned char) (color.blue & 0xc0) >> 6));
      *q++=(unsigned char) pixel;
      p++;
      j++;
      if (j == 16)
        j=0;
    }
    q+=scanline_pad;
    i++;
    if (i == 2)
      i=0;
  }
  /*
    Free allocated memory.
  */
  for (i=0; i < 2; i++)
    for (j=0; j < 16; j++)
    {
      free((char *) green_map[i][j]);
      free((char *) blue_map[i][j]);
      free((char *) red_map[i][j]);
    }
}


/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   X D r a w I m a g e                                                       %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Function XDrawImage draws a line on the image.
%
%  The format of the XDrawImage routine is:
%
%    status=XDrawImage(display,pixel_info,draw_info,image)
%
%  A description of each parameter follows:
%
%    o status: Function XDrawImage returns True if the image is
%      successfully drawd with text.  False is returned is there is a
%      memory shortage.
%
%    o display: Specifies a connection to an X server;  returned from
%      XOpenDisplay.
%
%    o pixel_info: Specifies a pointer to a XPixelInfo structure.
%
%    o draw_info: Specifies a pointer to a XDrawInfo structure.
%
%    o image: Specifies a pointer to a Image structure;  returned from
%      ReadImage.
%
%
*/
unsigned int XDrawImage(Display *display, XPixelInfo *pixel_info, XDrawInfo *draw_info, Image *image)
{
  ColorPacket
    background_color;

  GC
    draw_context;

  Image
    *draw_image;

  int
    x,
    y;

  Pixmap
    draw_pixmap;

  register int
    i;

  register RunlengthPacket
    *p,
    *q;

  unsigned int
    depth,
    height,
    matte,
    width;

  Window
    root_window;

  XGCValues
    context_values;

  XImage
    *draw_ximage;

  /*
    Initialize drawd image.
  */
  if (!UncompressImage(image))
    return(False);
  /*
    Initialize drawd pixmap.
  */
  root_window=XRootWindow(display,XDefaultScreen(display));
  depth=XDefaultDepth(display,XDefaultScreen(display));
  draw_pixmap=XCreatePixmap(display,root_window,draw_info->width,
    draw_info->height,(int) depth);
  if (draw_pixmap == (Pixmap) NULL)
    return(False);
  /*
    Initialize graphics info.
  */
  context_values.background=(unsigned long) (~0);
  context_values.foreground=0;
  context_values.line_width=draw_info->line_width;
  draw_context=XCreateGC(display,root_window,GCBackground | GCForeground |
    GCLineWidth,&context_values);
  if (draw_context == (GC) NULL)
    return(False);
  /*
    Clear pixmap.
  */
  XFillRectangle(display,draw_pixmap,draw_context,0,0,draw_info->width,
    draw_info->height);
  /*
    Draw line to pixmap.
  */
  XSetBackground(display,draw_context,0);
  XSetForeground(display,draw_context,(unsigned long) (~0));
  XSetFillStyle(display,draw_context,FillOpaqueStippled);
  XSetStipple(display,draw_context,draw_info->stipple);
  switch (draw_info->primitive)
  {
    case PointPrimitive:
    default:
    {
      XDrawLines(display,draw_pixmap,draw_context,draw_info->coordinate_info,
        draw_info->number_coordinates,CoordModeOrigin);
      break;
    }
    case LinePrimitive:
    {
      XDrawLine(display,draw_pixmap,draw_context,draw_info->line_info.x1,
        draw_info->line_info.y1,draw_info->line_info.x2,
        draw_info->line_info.y2);
      break;
    }
    case RectanglePrimitive:
    {
      XDrawRectangle(display,draw_pixmap,draw_context,
        draw_info->rectangle_info.x,draw_info->rectangle_info.y,
        draw_info->rectangle_info.width,draw_info->rectangle_info.height);
      break;
    }
    case FillRectanglePrimitive:
    {
      XFillRectangle(display,draw_pixmap,draw_context,
        draw_info->rectangle_info.x,draw_info->rectangle_info.y,
        draw_info->rectangle_info.width,draw_info->rectangle_info.height);
      break;
    }
    case EllipsePrimitive:
    {
      XDrawArc(display,draw_pixmap,draw_context,
        draw_info->rectangle_info.x,draw_info->rectangle_info.y,
        draw_info->rectangle_info.width,draw_info->rectangle_info.height,
        0,360*64);
      break;
    }
    case FillEllipsePrimitive:
    {
      XFillArc(display,draw_pixmap,draw_context,
        draw_info->rectangle_info.x,draw_info->rectangle_info.y,
        draw_info->rectangle_info.width,draw_info->rectangle_info.height,
        0,360*64);
      break;
    }
    case PolygonPrimitive:
    {
      XPoint
        *coordinate_info;

      coordinate_info=draw_info->coordinate_info;
      XDrawLines(display,draw_pixmap,draw_context,coordinate_info,
        draw_info->number_coordinates,CoordModeOrigin);
      XDrawLine(display,draw_pixmap,draw_context,
        coordinate_info[draw_info->number_coordinates-1].x,
        coordinate_info[draw_info->number_coordinates-1].y,
        coordinate_info[0].x,coordinate_info[0].y);
      break;
    }
    case FillPolygonPrimitive:
    {
      XFillPolygon(display,draw_pixmap,draw_context,draw_info->coordinate_info,
        draw_info->number_coordinates,Complex,CoordModeOrigin);
      break;
    }
  }
  XFreeGC(display,draw_context);
  /*
    Initialize X image.
  */
  draw_ximage=XGetImage(display,draw_pixmap,0,0,draw_info->width,
    draw_info->height,AllPlanes,ZPixmap);
  if (draw_ximage == (XImage *) NULL)
    return(False);
  XFreePixmap(display,draw_pixmap);
  /*
    Initialize draw image.
  */
  draw_image=AllocateImage((ImageInfo *) NULL);
  if (draw_image == (Image *) NULL)
    return(False);
  draw_image->columns=draw_info->width;
  draw_image->rows=draw_info->height;
  draw_image->packets=draw_image->columns*draw_image->rows;
  draw_image->pixels=(RunlengthPacket *)
    malloc((unsigned int) image->packets*sizeof(RunlengthPacket));
  if (draw_image->pixels == (RunlengthPacket *) NULL)
    {
      DestroyImage(draw_image);
      return(False);
    }
  /*
    Transfer drawn X image to image.
  */
  (void) XParseGeometry(draw_info->geometry,&x,&y,&width,&height);
  q=image->pixels+y*image->columns+x;
  background_color.red=q->red;
  background_color.green=q->green;
  background_color.blue=q->blue;
  background_color.index=0;
  draw_image->matte=True;
  p=draw_image->pixels;
  for (y=0; y < draw_image->rows; y++)
    for (x=0; x < draw_image->columns; x++)
    {
      p->index=(unsigned short) XGetPixel(draw_ximage,x,y);
      if (p->index == 0)
        {
          /*
            Set this pixel to the background color.
          */
          p->red=background_color.red;
          p->green=background_color.green;
          p->blue=background_color.blue;
          p->index=draw_info->stencil == OpaqueStencil ? Transparent : Opaque;
        }
      else
        {
          /*
            Set this pixel to the pen color.
          */
          p->red=XDownScale(pixel_info->pen_color.red);
          p->green=XDownScale(pixel_info->pen_color.green);
          p->blue=XDownScale(pixel_info->pen_color.blue);
          p->index=draw_info->stencil == OpaqueStencil ? Opaque : Transparent;
        }
      p->length=0;
      p++;
    }
  XDestroyImage(draw_ximage);
  /*
    Determine draw geometry.
  */
  (void) XParseGeometry(draw_info->geometry,&x,&y,&width,&height);
  if ((width != draw_image->columns) || (height != draw_image->rows))
    {
      char
        image_geometry[MaxTextLength];

      /*
        Scale image.
      */
      (void) sprintf(image_geometry,"%ux%u!",width,height);
      TransformImage(&draw_image,(char *) NULL,image_geometry);
    }
  if (draw_info->degrees != 0.0)
    {
      double
        normalized_degrees;

      Image
        *rotated_image;

      int
        rotations;

      /*
        Rotate image.
      */
      rotated_image=RotateImage(draw_image,draw_info->degrees,
        &background_color,False,True);
      if (rotated_image == (Image *) NULL)
        return(False);
      DestroyImage(draw_image);
      draw_image=rotated_image;
      /*
        Annotation is relative to the degree of rotation.
      */
      normalized_degrees=draw_info->degrees;
      while (normalized_degrees < -45.0)
        normalized_degrees+=360.0;
      for (rotations=0; normalized_degrees > 45.0; rotations++)
        normalized_degrees-=90.0;
      switch (rotations % 4)
      {
        default:
        case 0:
          break;
        case 1:
        {
          /*
            Rotate 90 degrees.
          */
          x-=draw_image->columns >> 1;
          y+=draw_image->columns >> 1;
          break;
        }
        case 2:
        {
          /*
            Rotate 180 degrees.
          */
          x-=draw_image->columns;
          break;
        }
        case 3:
        {
          /*
            Rotate 270 degrees.
          */
          x-=draw_image->columns >> 1;
          y-=draw_image->rows-(draw_image->columns >> 1);
          break;
        }
      }
    }
  /*
    Composite text onto the image.
  */
  p=draw_image->pixels;
  for (i=0; i < draw_image->packets; i++)
  {
    if (p->index != Transparent)
      p->index=Opaque;
    p++;
  }
  if (draw_info->stencil == TransparentStencil)
    CompositeImage(image,MatteReplaceCompositeOp,draw_image,x,y);
  else
    {
      matte=image->matte;
      CompositeImage(image,OverCompositeOp,draw_image,x,y);
      image->matte=matte;
    }
  DestroyImage(draw_image);
  return(True);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   X E r r o r                                                               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Function XError ignores BadWindow errors for XQueryTree and
%  XGetWindowAttributes, and ignores BadDrawable errors for XGetGeometry, and
%  ignores BadValue errors for XQueryColor.  It returns False in those cases.
%  Otherwise it returns True.
%
%  The format of the XError function is:
%
%      XError(display,error)
%
%  A description of each parameter follows:
%
%    o display: Specifies a pointer to the Display structure;  returned from
%      XOpenDisplay.
%
%    o error: Specifies the error event.
%
%
*/
int XError(Display *display, XErrorEvent *error)
{
  xerror_alert=True;
  switch (error->request_code)
  {
    case X_GetGeometry:
    {
      if (error->error_code == BadDrawable)
        return(False);
      break;
    }
    case X_GetWindowAttributes:
    case X_QueryTree:
    {
      if (error->error_code == BadWindow)
        return(False);
      break;
    }
    case X_QueryColors:
    {
      if (error->error_code == BadValue)
        return(False);
      break;
    }
  }
  return(True);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   X F r e e R e s o u r c e s                                               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Function XFreeResources frees X11 resources.
%
%  The format of the XFreeResources routine is:
%
%      XFreeResources(display,visual_info,map_info,pixel_info,font_info,
%        resource_info,window_info)
%
%  A description of each parameter follows:
%
%    o display: Specifies a connection to an X server; returned from
%      XOpenDisplay.
%
%    o visual_info: Specifies a pointer to a X11 XVisualInfo structure;
%      returned from XGetVisualInfo.
%
%    o map_info: If map_type is specified, this structure is initialized
%      with info from the Standard Colormap.
%
%    o pixel_info: Specifies a pointer to a XPixelInfo structure.
%
%    o font_info: Specifies a pointer to a XFontStruct structure.
%
%    o resource_info: Specifies a pointer to a X11 XResourceInfo structure.
%
%    o window_info: Specifies a pointer to a X11 XWindowInfo structure.
%
%
*/
void XFreeResources(Display *display, XVisualInfo *visual_info, XStandardColormap *map_info, XPixelInfo *pixel_info, XFontStruct *font_info, XResourceInfo *resource_info, XWindowInfo *window_info)
{
  if (window_info != (XWindowInfo *) NULL)
    {
      /*
        Free X image.
      */
      if (window_info->ximage != (XImage *) NULL)
        XDestroyImage(window_info->ximage);
      if (window_info->id != (Window) NULL)
        {
          /*
            Free destroy window and free cursors.
          */
          if (window_info->id != XRootWindow(display,visual_info->screen))
            XDestroyWindow(display,window_info->id);
          if (window_info->annotate_context != (GC) NULL)
            XFreeGC(display,window_info->annotate_context);
          if (window_info->highlight_context != (GC) NULL)
            XFreeGC(display,window_info->highlight_context);
          if (window_info->widget_context != (GC) NULL)
            XFreeGC(display,window_info->widget_context);
          XFreeCursor(display,window_info->cursor);
          XFreeCursor(display,window_info->busy_cursor);
        }
    }
  /*
    Free font.
  */
  if (font_info != (XFontStruct *) NULL)
    XFreeFont(display,font_info);
  if (map_info != (XStandardColormap *) NULL)
    {
      /*
        Free X Standard Colormap.
      */
      if (resource_info->map_type == (char *) NULL)
        XFreeStandardColormap(display,visual_info,map_info,pixel_info);
      XFree((void *) map_info);
    }
  /*
    Free X visual info.
  */
  if (visual_info != (XVisualInfo *) NULL)
    XFree((void *) visual_info);
  if (resource_info->close_server)
    XCloseDisplay(display);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   X F r e e S t a n d a r d C o l o r m a p                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Function XFreeStandardColormap frees an X11 colormap.
%
%  The format of the XFreeStandardColormap routine is:
%
%      XFreeStandardColormap(display,visual_info,map_info,pixel_info)
%
%  A description of each parameter follows:
%
%    o display: Specifies a connection to an X server; returned from
%      XOpenDisplay.
%
%    o visual_info: Specifies a pointer to a X11 XVisualInfo structure;
%      returned from XGetVisualInfo.
%
%    o map_info: If map_type is specified, this structure is initialized
%      with info from the Standard Colormap.
%
%    o pixel_info: Specifies a pointer to a XPixelInfo structure.
%
%
*/
void XFreeStandardColormap(Display *display, XVisualInfo *visual_info, XStandardColormap *map_info, XPixelInfo *pixel_info)
{
  /*
    Free colormap.
  */
  XFlush(display);
  if (map_info->colormap != (Colormap) NULL)
    if (map_info->colormap != XDefaultColormap(display,visual_info->screen))
      XFreeColormap(display,map_info->colormap);
    else
      if (pixel_info != (XPixelInfo *) NULL)
        if ((visual_info->class != TrueColor) &&
            (visual_info->class != DirectColor))
          XFreeColors(display,map_info->colormap,pixel_info->pixels,
            (int) pixel_info->colors,0);
  map_info->colormap=(Colormap) NULL;
  if (pixel_info != (XPixelInfo *) NULL)
    {
      if (pixel_info->gamma_map != (XColor *) NULL)
        free((char *) pixel_info->gamma_map);
      pixel_info->gamma_map=(XColor *) NULL;
      if (pixel_info->pixels != (unsigned long *) NULL)
        free((char *) pixel_info->pixels);
      pixel_info->pixels=(unsigned long *) NULL;
    }
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   X G e t A n n o t a t e I n f o                                           %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Function XGetAnnotateInfo initializes the AnnotateInfo structure.
%
%  The format of the GetAnnotateInfo routine is:
%
%      XGetAnnotateInfo(image_info)
%
%  A description of each parameter follows:
%
%    o annotate_info: Specifies a pointer to a XAnnotateInfo structure.
%
%
*/
void XGetAnnotateInfo(XAnnotateInfo *annotate_info)
{
  /*
    Initialize annotate structure.
  */
  annotate_info->x=0;
  annotate_info->y=0;
  annotate_info->width=0;
  annotate_info->height=0;
  annotate_info->stencil=ForegroundStencil;
  annotate_info->degrees=0.0;
  annotate_info->font_info=(XFontStruct *) NULL;
  annotate_info->text=(char *) NULL;
  *annotate_info->geometry='\0';
  annotate_info->previous=(XAnnotateInfo *) NULL;
  annotate_info->next=(XAnnotateInfo *) NULL;
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   X G e t M a p I n f o                                                     %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Function XGetMapInfo initializes the XStandardColormap structure.
%
%  The format of the XStandardColormap routine is:
%
%      XGetMapInfo(visual_info,colormap,map_info)
%
%  A description of each parameter follows:
%
%    o colormap: Specifies the ID of the X server colormap.
%
%    o visual_info: Specifies a pointer to a X11 XVisualInfo structure;
%      returned from XGetVisualInfo.
%
%    o map_info: Specifies a pointer to a X11 XStandardColormap structure.
%
%
*/
void XGetMapInfo(XVisualInfo *visual_info, Colormap colormap, XStandardColormap *map_info)
{
  /*
    Initialize map info.
  */
  map_info->colormap=colormap;
  map_info->red_max=visual_info->red_mask;
  map_info->red_mult=map_info->red_max != 0 ? 1 : 0;
  if (map_info->red_max != 0)
    while ((map_info->red_max & 0x01) == 0)
    {
      map_info->red_max>>=1;
      map_info->red_mult<<=1;
    }
  map_info->green_max=visual_info->green_mask;
  map_info->green_mult=map_info->green_max != 0 ? 1 : 0;
  if (map_info->green_max != 0)
    while ((map_info->green_max & 0x01) == 0)
    {
      map_info->green_max>>=1;
      map_info->green_mult<<=1;
    }
  map_info->blue_max=visual_info->blue_mask;
  map_info->blue_mult=map_info->blue_max != 0 ? 1 : 0;
  if (map_info->blue_max != 0)
    while ((map_info->blue_max & 0x01) == 0)
    {
      map_info->blue_max>>=1;
      map_info->blue_mult<<=1;
    }
  map_info->base_pixel=0;
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   X G e t M o n t a g e I n f o                                             %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Function XGetMontageInfo initializes the MontageInfo structure.
%
%  The format of the GetMontageInfo routine is:
%
%      XGetMontageInfo(montage_info)
%
%  A description of each parameter follows:
%
%    o montage_info: Specifies a pointer to a MontageInfo structure.
%
%
*/
void XGetMontageInfo(XMontageInfo *montage_info)
{
  montage_info->number_tiles=0;
  montage_info->frame=True;
  montage_info->shadow=True;
  montage_info->compose=ReplaceCompositeOp;
  montage_info->tile="5x4";
  montage_info->texture=(char *) NULL;
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   X G e t P i x e l I n f o                                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Function XGetPixelInfo initializes the PixelInfo structure.
%
%  The format of the XGetPixelInfo routine is:
%
%      XGetPixelInfo(display,visual_info,map_info,resource_info,image,
%        pixel_info)
%
%  A description of each parameter follows:
%
%    o display: Specifies a connection to an X server; returned from
%      XOpenDisplay.
%
%    o visual_info: Specifies a pointer to a X11 XVisualInfo structure;
%      returned from XGetVisualInfo.
%
%    o map_info: If map_type is specified, this structure is initialized
%      with info from the Standard Colormap.
%
%    o resource_info: Specifies a pointer to a X11 XResourceInfo structure.
%
%    o image: Specifies a pointer to a Image structure;  returned from
%      ReadImage.
%
%    o pixel_info: Specifies a pointer to a XPixelInfo structure.
%
%
*/
void XGetPixelInfo(Display *display, XVisualInfo *visual_info, XStandardColormap *map_info, XResourceInfo *resource_info, Image *image, XPixelInfo *pixel_info)
{
  static char
    *PenColors[MaxNumberPens]=
    {
      Pen1Color,
      Pen2Color,
      Pen3Color,
      Pen4Color,
      Pen5Color,
      Pen6Color,
      Pen7Color,
      Pen8Color,
      Pen9Color,
      Pen0Color,
      Pen0Color
    };

  Colormap
    colormap;

  int
    status;

  register int
    i;

  unsigned int
    packets;

  /*
    Initialize pixel info.
  */
  pixel_info->colors=0;
  if (image != (Image *) NULL)
    if (image->class == PseudoClass)
      pixel_info->colors=image->colors;
  packets=Max(pixel_info->colors,visual_info->colormap_size)+MaxNumberPens;
  if (pixel_info->pixels != (unsigned long *) NULL)
    free((char *) pixel_info->pixels);
  pixel_info->pixels=(unsigned long *) malloc(packets*sizeof(unsigned long));
  if (pixel_info->pixels == (unsigned long *) NULL)
    Error("Unable to get pixel info","Memory allocation failed");
  /*
    Set foreground color.
  */
  colormap=map_info->colormap;
  (void) XParseColor(display,colormap,ForegroundColor,
    &pixel_info->foreground_color);
  status=XParseColor(display,colormap,resource_info->foreground_color,
    &pixel_info->foreground_color);
  if (status == 0)
    Warning("Color is not known to X server",resource_info->foreground_color);
  pixel_info->foreground_color.pixel=
    XStandardPixel(map_info,pixel_info->foreground_color,16);
  pixel_info->foreground_color.flags=DoRed | DoGreen | DoBlue;
  /*
    Set background color.
  */
  (void) XParseColor(display,colormap,BackgroundColor,
    &pixel_info->background_color);
  status=XParseColor(display,colormap,resource_info->background_color,
    &pixel_info->background_color);
  if (status == 0)
    Warning("Color is not known to X server",resource_info->background_color);
  pixel_info->background_color.pixel=
    XStandardPixel(map_info,pixel_info->background_color,16);
  pixel_info->background_color.flags=DoRed | DoGreen | DoBlue;
  /*
    Set border color.
  */
  (void) XParseColor(display,colormap,BorderColor,&pixel_info->border_color);
  status=XParseColor(display,colormap,resource_info->border_color,
    &pixel_info->border_color);
  if (status == 0)
    Warning("Color is not known to X server",resource_info->border_color);
  pixel_info->border_color.pixel=
    XStandardPixel(map_info,pixel_info->border_color,16);
  pixel_info->border_color.flags=DoRed | DoGreen | DoBlue;
  /*
    Set matte color.
  */
  pixel_info->matte_color=pixel_info->background_color;
  if (resource_info->matte_color != (char *) NULL)
    {
      /*
        Matte color is specified as a X resource or command line argument.
      */
      status=XParseColor(display,colormap,resource_info->matte_color,
        &pixel_info->matte_color);
      if (status == 0)
        Warning("Color is not known to X server",resource_info->matte_color);
      pixel_info->matte_color.pixel=
        XStandardPixel(map_info,pixel_info->matte_color,16);
      pixel_info->matte_color.flags=DoRed | DoGreen | DoBlue;
    }
  /*
    Set highlight color.
  */
  pixel_info->highlight_color.red=(pixel_info->matte_color.red*
    HighlightModulate+(MaxRGB-HighlightModulate)*65535L)/MaxRGB;
  pixel_info->highlight_color.green=(pixel_info->matte_color.green*
    HighlightModulate+(MaxRGB-HighlightModulate)*65535L)/MaxRGB;
  pixel_info->highlight_color.blue=(pixel_info->matte_color.blue*
    HighlightModulate+(MaxRGB-HighlightModulate)*65535L)/MaxRGB;
  pixel_info->highlight_color.pixel=
    XStandardPixel(map_info,pixel_info->highlight_color,16);
  pixel_info->highlight_color.flags=DoRed | DoGreen | DoBlue;
  /*
    Set shadow color.
  */
  pixel_info->shadow_color.red=(unsigned int)
    (pixel_info->matte_color.red*ShadowModulate)/MaxRGB;
  pixel_info->shadow_color.green=(unsigned int)
    (pixel_info->matte_color.green*ShadowModulate)/MaxRGB;
  pixel_info->shadow_color.blue=(unsigned int)
    (pixel_info->matte_color.blue*ShadowModulate)/MaxRGB;
  pixel_info->shadow_color.pixel=
    XStandardPixel(map_info,pixel_info->shadow_color,16);
  pixel_info->shadow_color.flags=DoRed | DoGreen | DoBlue;
  /*
    Set depth color.
  */
  pixel_info->depth_color.red=(unsigned int)
    (pixel_info->matte_color.red*DepthModulate)/MaxRGB;
  pixel_info->depth_color.green=(unsigned int)
    (pixel_info->matte_color.green*DepthModulate)/MaxRGB;
  pixel_info->depth_color.blue=(unsigned int)
    (pixel_info->matte_color.blue*DepthModulate)/MaxRGB;
  pixel_info->depth_color.pixel=
    XStandardPixel(map_info,pixel_info->depth_color,16);
  pixel_info->depth_color.flags=DoRed | DoGreen | DoBlue;
  /*
    Set trough color.
  */
  pixel_info->trough_color.red=(unsigned int)
    (pixel_info->matte_color.red*TroughModulate)/MaxRGB;
  pixel_info->trough_color.green=(unsigned int)
    (pixel_info->matte_color.green*TroughModulate)/MaxRGB;
  pixel_info->trough_color.blue=(unsigned int)
    (pixel_info->matte_color.blue*TroughModulate)/MaxRGB;
  pixel_info->trough_color.pixel=
    XStandardPixel(map_info,pixel_info->trough_color,16);
  pixel_info->trough_color.flags=DoRed | DoGreen | DoBlue;
  /*
    Set pen color.
  */
  for (i=0; i < MaxNumberPens; i++)
  {
    (void) XParseColor(display,colormap,PenColors[i],
      &pixel_info->pen_colors[i]);
    status=XParseColor(display,colormap,resource_info->pen_colors[i],
      &pixel_info->pen_colors[i]);
    if (status == 0)
      Warning("Color is not known to X server",resource_info->pen_colors[i]);
    pixel_info->pen_colors[i].pixel=
      XStandardPixel(map_info,pixel_info->pen_colors[i],16);
    pixel_info->pen_colors[i].flags=DoRed | DoGreen | DoBlue;
  }
  pixel_info->box_color=pixel_info->background_color;
  pixel_info->pen_color=pixel_info->foreground_color;
  pixel_info->box_index=0;
  pixel_info->pen_index=1;
  /*
    Initialize gamma map to linear brightness.
  */
  if (pixel_info->gamma_map != (XColor *) NULL)
    free((char *) pixel_info->gamma_map);
  pixel_info->gamma_map=(XColor *) malloc((MaxRGB+1)*sizeof(XColor));
  if (pixel_info->gamma_map == (XColor *) NULL)
    Error("Unable to get pixel info","Memory allocation failed");
  for (i=0; i <= MaxRGB; i++)
  {
    pixel_info->gamma_map[i].red=i;
    pixel_info->gamma_map[i].green=i;
    pixel_info->gamma_map[i].blue=i;
  }
  if (image != (Image *) NULL)
    {
      if (resource_info->gamma_correct && (image->gamma != 0.0))
        {
          double
            blue_gamma,
            green_gamma,
            red_gamma;

          int
            count;

          /*
            Initialize map relative to display and image gamma.
          */
          red_gamma=1.0;
          green_gamma=1.0;
          blue_gamma=1.0;
          count=sscanf(resource_info->display_gamma,"%lf,%lf,%lf",
            &red_gamma,&green_gamma,&blue_gamma);
          if (count == 1)
            {
              green_gamma=red_gamma;
              blue_gamma=red_gamma;
            }
          red_gamma*=image->gamma;
          green_gamma*=image->gamma;
          blue_gamma*=image->gamma;
          for (i=0; i <= MaxRGB; i++)
          {
            pixel_info->gamma_map[i].red=
              (short unsigned int)((pow((double) i/MaxRGB,1.0/red_gamma)*MaxRGB)+0.5);
            pixel_info->gamma_map[i].green=
              (short unsigned int)((pow((double) i/MaxRGB,1.0/green_gamma)*MaxRGB)+0.5);
            pixel_info->gamma_map[i].blue=
              (short unsigned int)((pow((double) i/MaxRGB,1.0/blue_gamma)*MaxRGB)+0.5);
          }
        }
      if (image->class == PseudoClass)
        {
          register XColor
            *gamma_map;

          /*
            Initialize pixel array for images of type PseudoClass.
          */
          gamma_map=pixel_info->gamma_map;
          for (i=0; i < image->colors; i++)
            pixel_info->pixels[i]=
              XGammaPixel(map_info,gamma_map,image->colormap+i,QuantumDepth);
          for (i=0; i < MaxNumberPens; i++)
            pixel_info->pixels[image->colors+i]=pixel_info->pen_colors[i].pixel;
          pixel_info->colors+=MaxNumberPens;
        }
    }
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   X G e t R e s o u r c e C l a s s                                         %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Function XGetResourceClass queries the X server for the specified resource
%  name or class.  If the resource name or class is not defined in the
%  database, the supplied default value is returned.
%
%  The format of the XGetResourceClass routine is:
%
%      value=XGetResourceClass(database,client_name,keyword,resource_default)
%
%  A description of each parameter follows:
%
%    o value: Function XGetResourceClass returns the resource value associated
%      with the name or class.  If none is found, the supplied default value is
%      returned.
%
%    o database: Specifies a resource database; returned from
%      XrmGetStringDatabase.
%
%    o client_name:  Specifies the application name used to retrieve resource
%      info from the X server database.
%
%    o keyword: Specifies the keyword of the value being retrieved.
%
%    o resource_default: Specifies the default value to return if the query
%      fails to find the specified keyword/class.
%
%
*/
char *XGetResourceClass(XrmDatabase database, char *client_name, char *keyword, char *resource_default)
{
  char
    resource_class[MaxTextLength],
    resource_name[MaxTextLength];

  int
    status;

  static char
    *resource_type;

  XrmValue
    resource_value;

  if (database == (XrmDatabase) NULL)
    return(resource_default);
  *resource_name='\0';
  *resource_class='\0';
  if (keyword != (char *) NULL)
    {
      unsigned char
        c,
        k;

      /*
        Initialize resource keyword and class.
      */
      (void) sprintf(resource_name,"%s.%s",client_name,keyword);
      c=(*client_name);
      if ((c >= XK_a) && (c <= XK_z))
        c-=(XK_a-XK_A);
      else
        if ((c >= XK_agrave) && (c <= XK_odiaeresis))
          c-=(XK_agrave-XK_Agrave);
        else
          if ((c >= XK_oslash) && (c <= XK_thorn))
            c-=(XK_oslash-XK_Ooblique);
      k=(*keyword);
      if ((k >= XK_a) && (k <= XK_z))
        k-=(XK_a-XK_A);
      else
        if ((k >= XK_agrave) && (k <= XK_odiaeresis))
          k-=(XK_agrave-XK_Agrave);
        else
          if ((k >= XK_oslash) && (k <= XK_thorn))
            k-=(XK_oslash-XK_Ooblique);
      (void) sprintf(resource_class,"%c%s.%c%s",c,client_name+1,k,keyword+1);
    }
  status=XrmGetResource(database,resource_name,resource_class,&resource_type,
    &resource_value);
  if (status == False)
    return(resource_default);
  return(resource_value.addr);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   X G e t R e s o u r c e D a t a b a s e                                   %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Function XGetResourceDatabase creates a new resource database and
%  initializes it.
%
%  The format of the XGetResourceDatabase routine is:
%
%      database=XGetResourceDatabase(display,client_name)
%
%  A description of each parameter follows:
%
%    o database: Function XGetResourceDatabase returns the database after
%      it is initialized.
%
%    o display: Specifies a connection to an X server;  returned from
%      XOpenDisplay.
%
%    o client_name:  Specifies the application name used to retrieve resource
%      info from the X server database.
%
%
*/
XrmDatabase XGetResourceDatabase(Display *display, char *client_name)
{
  char
    filename[MaxTextLength];

  register char
    *p;

  unsigned char
    c;

  XrmDatabase
    resource_database,
    server_database;

  /*
    Initialize resource database.
  */
  XrmInitialize();
  XGetDefault(display,client_name,"dummy");
  resource_database=XrmGetDatabase(display);
  /*
    Combine application database.
  */
  if (client_name != (char *) NULL)
    {
      /*
        Get basename of client.
      */
      p=client_name+(strlen(client_name)-1);
      while ((p > client_name) && (*p != '/'))
        p--;
      if (*p == '/')
        client_name=p+1;
    }
  c=(*client_name);
  if ((c >= XK_a) && (c <= XK_z))
    c-=(XK_a-XK_A);
  else
    if ((c >= XK_agrave) && (c <= XK_odiaeresis))
      c-=(XK_agrave-XK_Agrave);
    else
      if ((c >= XK_oslash) && (c <= XK_thorn))
        c-=(XK_oslash-XK_Ooblique);
  (void) sprintf(filename,"%s%c%s",ApplicationDefaults,c,client_name+1);
  XrmCombineFileDatabase(filename,&resource_database,False);
  if (XResourceManagerString(display) != (char *) NULL)
    {
      /*
        Combine server database.
      */
      server_database=XrmGetStringDatabase(XResourceManagerString(display));
      XrmCombineDatabase(server_database,&resource_database,False);
    }
  /*
    Merge user preferences database.
  */
  (void) sprintf(filename,"%s%src",PreferencesDefaults,client_name);
  ExpandFilename(filename);
  XrmCombineFileDatabase(filename,&resource_database,False);
  return(resource_database);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   X G e t R e s o u r c e I n f o                                           %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Function XGetResourceInfo initializes the ResourceInfo structure.
%
%  The format of the XGetResourceInfo routine is:
%
%      XGetResourceInfo(resource_database,client_name,resource_info)
%
%  A description of each parameter follows:
%
%    o resource_database: Specifies a resource database; returned from
%      XrmGetStringDatabase.
%
%    o client_name:  Specifies the application name used to retrieve
%      resource info from the X server database.
%
%    o resource_info: Specifies a pointer to a X11 XResourceInfo structure.
%
%
*/
void XGetResourceInfo(XrmDatabase resource_database, char *client_name, XResourceInfo *resource_info)
{
  char
    *resource_value;

  register char
    *p;

  if (client_name != (char *) NULL)
    {
      /*
        Get basename of client.
      */
      p=client_name+(strlen(client_name)-1);
      while ((p > client_name) && (*p != '/'))
        p--;
      if (*p == '/')
        client_name=p+1;
    }
  /*
    Initialize resource info fields.
  */
  resource_info->resource_database=resource_database;
  resource_info->close_server=True;
  resource_value=
    XGetResourceClass(resource_database,client_name,"backdrop","False");
  resource_info->backdrop=IsTrue(resource_value);
  resource_info->background_color=XGetResourceInstance(resource_database,
    client_name,"background",BackgroundColor);
  resource_info->border_color=XGetResourceInstance(resource_database,
    client_name,"borderColor",BorderColor);
  resource_value=
    XGetResourceClass(resource_database,client_name,"borderWidth","2");
  resource_info->border_width=atoi(resource_value);
  resource_value=
    XGetResourceClass(resource_database,client_name,"colormap","shared");
  resource_info->colormap=UndefinedColormap;
  if (Latin1Compare("private",resource_value) == 0)
    resource_info->colormap=PrivateColormap;
  if (Latin1Compare("shared",resource_value) == 0)
    resource_info->colormap=SharedColormap;
  if (resource_info->colormap == UndefinedColormap)
    Warning("Unrecognized colormap type",resource_value);
  resource_value=
    XGetResourceClass(resource_database,client_name,"colorRecovery","False");
  resource_info->color_recovery=IsTrue(resource_value);
  resource_value=
    XGetResourceClass(resource_database,client_name,"colorspace","rgb");
  resource_info->colorspace=UndefinedColorspace;
  if (Latin1Compare("gray",resource_value) == 0)
    resource_info->colorspace=GRAYColorspace;
  if (Latin1Compare("rgb",resource_value) == 0)
    resource_info->colorspace=RGBColorspace;
  if (Latin1Compare("ohta",resource_value) == 0)
    resource_info->colorspace=OHTAColorspace;
  if (Latin1Compare("xyz",resource_value) == 0)
    resource_info->colorspace=XYZColorspace;
  if (Latin1Compare("yiq",resource_value) == 0)
    resource_info->colorspace=YIQColorspace;
  if (Latin1Compare("yuv",resource_value) == 0)
    resource_info->colorspace=YUVColorspace;
  if (resource_info->colorspace == UndefinedColorspace)
    Warning("Unrecognized colorspace type",resource_value);
  resource_value=
    XGetResourceClass(resource_database,client_name,"confirmExit","False");
  resource_info->confirm_exit=IsTrue(resource_value);
  resource_value=
    XGetResourceClass(resource_database,client_name,"debug","False");
  resource_info->debug=IsTrue(resource_value);
  resource_value=XGetResourceClass(resource_database,client_name,"delay","0");
  resource_info->delay=atoi(resource_value);
  resource_info->display_gamma=XGetResourceClass(resource_database,client_name,
    "displayGamma",DefaultDisplayGamma);
  resource_value=
    XGetResourceClass(resource_database,client_name,"dither","True");
  resource_info->dither=IsTrue(resource_value);
  resource_info->editor_command=XGetResourceClass(resource_database,client_name,
    "editorCommand",EditorCommand);
  resource_info->font=
    XGetResourceClass(resource_database,client_name,"font",(char *) NULL);
  resource_info->font=XGetResourceClass(resource_database,client_name,
    "fontList",resource_info->font);
  resource_info->font_name[0]=
    XGetResourceClass(resource_database,client_name,"font1","fixed");
  resource_info->font_name[1]=
    XGetResourceClass(resource_database,client_name,"font2","variable");
  resource_info->font_name[2]=
    XGetResourceClass(resource_database,client_name,"font3","5x8");
  resource_info->font_name[3]=
    XGetResourceClass(resource_database,client_name,"font4","6x10");
  resource_info->font_name[4]=
    XGetResourceClass(resource_database,client_name,"font5","7x13bold");
  resource_info->font_name[5]=
    XGetResourceClass(resource_database,client_name,"font6","8x13bold");
  resource_info->font_name[6]=
    XGetResourceClass(resource_database,client_name,"font7","9x15bold");
  resource_info->font_name[7]=
    XGetResourceClass(resource_database,client_name,"font8","10x20");
  resource_info->font_name[8]=
    XGetResourceClass(resource_database,client_name,"font9","12x24");
  resource_info->font_name[9]=
    XGetResourceClass(resource_database,client_name,"font0","fixed");
  resource_info->font_name[10]=
    XGetResourceClass(resource_database,client_name,"font0","fixed");
  resource_info->foreground_color=XGetResourceInstance(resource_database,
    client_name,"foreground",ForegroundColor);
  resource_value=
    XGetResourceClass(resource_database,client_name,"gammaCorrect","True");
  resource_info->gamma_correct=IsTrue(resource_value);
  resource_value=
    XGetResourceClass(resource_database,client_name,"gravity","North");
  resource_info->gravity=(-1);
  if (Latin1Compare("Forget",resource_value) == 0)
    resource_info->gravity=ForgetGravity;
  if (Latin1Compare("NorthWest",resource_value) == 0)
    resource_info->gravity=NorthWestGravity;
  if (Latin1Compare("North",resource_value) == 0)
    resource_info->gravity=NorthGravity;
  if (Latin1Compare("NorthEast",resource_value) == 0)
    resource_info->gravity=NorthEastGravity;
  if (Latin1Compare("West",resource_value) == 0)
    resource_info->gravity=WestGravity;
  if (Latin1Compare("Center",resource_value) == 0)
    resource_info->gravity=CenterGravity;
  if (Latin1Compare("East",resource_value) == 0)
    resource_info->gravity=EastGravity;
  if (Latin1Compare("SouthWest",resource_value) == 0)
    resource_info->gravity=SouthWestGravity;
  if (Latin1Compare("South",resource_value) == 0)
    resource_info->gravity=SouthGravity;
  if (Latin1Compare("SouthEast",resource_value) == 0)
    resource_info->gravity=SouthEastGravity;
  if (Latin1Compare("Static",resource_value) == 0)
    resource_info->gravity=StaticGravity;
  if (resource_info->gravity == (-1))
    {
      Warning("Unrecognized gravity type",resource_value);
      resource_info->gravity=CenterGravity;
    }
  (void) getcwd(resource_info->home_directory,MaxTextLength-1);
  resource_info->icon_geometry=XGetResourceClass(resource_database,client_name,
    "iconGeometry",(char *) NULL);
  resource_value=
    XGetResourceClass(resource_database,client_name,"iconic","False");
  resource_info->iconic=IsTrue(resource_value);
  resource_value=
    XGetResourceClass(resource_database,client_name,"immutable","False");
  resource_info->immutable=IsTrue(resource_value);
  resource_info->image_geometry=XGetResourceClass(resource_database,
    client_name,"geometry",(char *) NULL);
  resource_value=XGetResourceClass(resource_database,client_name,"magnify","3");
  resource_info->magnify=atoi(resource_value);
  resource_info->map_type=
    XGetResourceClass(resource_database,client_name,"map",(char *) NULL);
  resource_info->matte_color=XGetResourceInstance(resource_database,
    client_name,"mattecolor",(char *) NULL);
  resource_value=
    XGetResourceClass(resource_database,client_name,"monochrome","False");
  resource_info->monochrome=IsTrue(resource_value);
  resource_info->name=
    XGetResourceClass(resource_database,client_name,"name",(char *) NULL);
  resource_value=XGetResourceClass(resource_database,client_name,"colors","0");
  resource_info->number_colors=atoi(resource_value);
  resource_info->pen_colors[0]=
    XGetResourceClass(resource_database,client_name,"pen1","black");
  resource_info->pen_colors[1]=
    XGetResourceClass(resource_database,client_name,"pen2","blue");
  resource_info->pen_colors[2]=
    XGetResourceClass(resource_database,client_name,"pen3","cyan");
  resource_info->pen_colors[3]=
    XGetResourceClass(resource_database,client_name,"pen4","green");
  resource_info->pen_colors[4]=
    XGetResourceClass(resource_database,client_name,"pen5","gray");
  resource_info->pen_colors[5]=
    XGetResourceClass(resource_database,client_name,"pen6","red");
  resource_info->pen_colors[6]=
    XGetResourceClass(resource_database,client_name,"pen7","magenta");
  resource_info->pen_colors[7]=
    XGetResourceClass(resource_database,client_name,"pen8","yellow");
  resource_info->pen_colors[8]=
    XGetResourceClass(resource_database,client_name,"pen9","white");
  resource_info->pen_colors[9]=
    XGetResourceClass(resource_database,client_name,"pen0","gray");
  resource_info->pen_colors[10]=
    XGetResourceClass(resource_database,client_name,"pen0","gray");
  resource_info->print_command=XGetResourceClass(resource_database,client_name,
    "printCommand",PrintCommand);
  resource_value=XGetResourceClass(resource_database,client_name,"quantum","1");
  resource_info->quantum=atoi(resource_value);
  resource_info->server_name=
    XGetResourceClass(resource_database,client_name,"serverName",(char *) NULL);
  resource_info->text_font=
    XGetResourceClass(resource_database,client_name,"font",(char *) NULL);
  resource_info->text_font=XGetResourceClass(resource_database,client_name,
    "textFontList",resource_info->text_font);
  resource_info->title=
    XGetResourceClass(resource_database,client_name,"title",(char *) NULL);
  resource_value=
    XGetResourceClass(resource_database,client_name,"treeDepth","0");
  resource_info->tree_depth=atoi(resource_value);
  resource_value=
    XGetResourceClass(resource_database,client_name,"undoCache",UndoCache);
  resource_info->undo_cache=atoi(resource_value);
  resource_value=
    XGetResourceClass(resource_database,client_name,"update","False");
  resource_info->update=IsTrue(resource_value);
  resource_value=
    XGetResourceClass(resource_database,client_name,"usePixmap","False");
  resource_info->use_pixmap=IsTrue(resource_value);
  resource_value=
    XGetResourceClass(resource_database,client_name,"sharedMemory","True");
  resource_info->use_shared_memory=IsTrue(resource_value);
  resource_info->visual_type=
    XGetResourceClass(resource_database,client_name,"visual",(char *) NULL);
  resource_info->window_group=XGetResourceClass(resource_database,client_name,
    "windowGroup",(char *) NULL);
  resource_info->window_id=
    XGetResourceClass(resource_database,client_name,"window",(char *) NULL);
  resource_info->write_filename=XGetResourceClass(resource_database,
    client_name,"writeFilename",(char *) NULL);
  /*
    Handle side-effects.
  */
  if (resource_info->monochrome)
    {
      resource_info->number_colors=2;
      resource_info->tree_depth=8;
      resource_info->dither=True;
      resource_info->colorspace=GRAYColorspace;
    }
  if (resource_info->colorspace == GRAYColorspace)
    {
      resource_info->number_colors=256;
      resource_info->tree_depth=8;
    }
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   X G e t R e s o u r c e I n s t a n c e                                   %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Function XGetResourceInstance queries the X server for the specified
%  resource name.  If the resource name is not defined in the database, the
%  supplied default value is returned.
%
%  The format of the XGetResourceInstance routine is:
%
%      value=XGetResourceInstance(database,client_name,keyword,resource_default)
%
%  A description of each parameter follows:
%
%    o value: Function XGetResourceInstance returns the resource value
%      associated with the name or class.  If none is found, the supplied
%      default value is returned.
%
%    o database: Specifies a resource database; returned from
%      XrmGetStringDatabase.
%
%    o client_name:  Specifies the application name used to retrieve
%      resource info from the X server database.
%
%    o keyword: Specifies the keyword of the value being retrieved.
%
%    o resource_default: Specifies the default value to return if the query
%      fails to find the specified keyword/class.
%
%
*/
char *XGetResourceInstance(XrmDatabase database, char *client_name, char *keyword, char *resource_default)
{
  char
    *resource_type,
    resource_name[MaxTextLength];

  int
    status;

  XrmValue
    resource_value;

  if (database == (XrmDatabase) NULL)
    return(resource_default);
  *resource_name='\0';
  if (keyword != (char *) NULL)
    (void) sprintf(resource_name,"%s.%s",client_name,keyword);
  status=XrmGetResource(database,resource_name,"ImageMagick",&resource_type,
    &resource_value);
  if (status == False)
    return(resource_default);
  return(resource_value.addr);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   X G e t S u b w i n d o w                                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Function XGetSubwindow returns the subwindow of a window choosen the
%  user with the pointer and a button press.
%
%  The format of the XGetSubwindow routine is:
%
%      subwindow=XGetSubwindow(display,window,x,y)
%
%  A description of each parameter follows:
%
%    o subwindow: Function XGetSubwindow returns NULL if no subwindow is found
%      otherwise the subwindow is returned.
%
%    o display: Specifies a connection to an X server;  returned from
%      XOpenDisplay.
%
%    o window: Specifies a pointer to a Window.
%
%    o x: the x coordinate of the pointer relative to the origin of the
%      window.
%
%    o y: the y coordinate of the pointer relative to the origin of the
%      window.
%
%
*/
Window XGetSubwindow(Display *display, Window window, int x, int y)
{
  Window
    source_window,
    target_window;

  int
    status,
    x_offset,
    y_offset;

  source_window=XRootWindow(display,XDefaultScreen(display));
  if (window == (Window) NULL)
    return(source_window);
  target_window=window;
  for ( ; ; )
  {
    status=XTranslateCoordinates(display,source_window,window,x,y,
      &x_offset,&y_offset,&target_window);
    if (status != True)
      break;
    if (target_window == (Window) NULL)
      break;
    source_window=window;
    window=target_window;
    x=x_offset;
    y=y_offset;
  }
  if (target_window == (Window) NULL)
    target_window=window;
  return(target_window);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   X G e t W i n d o w C o l o r                                             %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Function XGetWindowColor returns the color of a pixel interactively choosen
%  from the X server.
%
%  The format of the XGetWindowColor routine is:
%
%      status=XGetWindowColor(display,name)
%
%  A description of each parameter follows:
%
%    o status: Function XGetWindowColor returns True if the color is obtained
%      from the X server.  False is returned if any errors occurs.
%
%    o display: Specifies a connection to an X server;  returned from
%      XOpenDisplay.
%
%    o name: The name of of the color if found in the X Color Database is
%      returned in this character string.
%
%
*/
unsigned int XGetWindowColor(Display *display, char *name)
{
  FILE
    *database;

  int
    x,
    y;

  RectangleInfo
    crop_info;

  unsigned int
    status;

  Window
    child,
    client_window,
    root_window,
    target_window;

  XColor
    color;

  XImage
    *ximage;

  XWindowAttributes
    window_attributes;

  /*
    Choose a pixel from the X server.
  */
  target_window=XSelectWindow(display,&crop_info);
  root_window=XRootWindow(display,XDefaultScreen(display));
  client_window=target_window;
  if (target_window != root_window)
    {
      unsigned int
        d;

      /*
        Get client window.
      */
      status=XGetGeometry(display,target_window,&root_window,&x,&x,&d,&d,&d,&d);
      if (status != 0)
        {
          client_window=XClientWindow(display,target_window);
          target_window=client_window;
        }
    }
  /*
    Verify window is viewable.
  */
  status=XGetWindowAttributes(display,target_window,&window_attributes);
  if ((status == False) || (window_attributes.map_state != IsViewable))
    return(False);
  /*
    Get window X image.
  */
  XTranslateCoordinates(display,root_window,target_window,crop_info.x,
    crop_info.y,&x,&y,&child);
  ximage=XGetImage(display,target_window,x,y,1,1,AllPlanes,ZPixmap);
  if (ximage == (XImage *) NULL)
    return(False);
  color.pixel=XGetPixel(ximage,0,0);
  XDestroyImage(ximage);
  /*
    Query X server for pixel color.
  */
  XQueryColor(display,window_attributes.colormap,&color);
  (void) sprintf(name,"#%02x%02x%02x",XDownScale(color.red),
    XDownScale(color.green),XDownScale(color.blue));
  database=fopen(RGBColorDatabase,"r");
  if (database != (FILE *) NULL)
    {
      char
        colorname[MaxTextLength],
        text[MaxTextLength];

      int
        blue,
        count,
        green,
        red;

      /*
        Match color against the X color database.
      */
      while (fgets(text,MaxTextLength-1,database) != (char *) NULL)
      {
        count=sscanf(text,"%d %d %d %[^\n]\n",&red,&green,&blue,colorname);
        if (count != 4)
          continue;
        if ((red == XDownScale(color.red)) &&
            (green == XDownScale(color.green)) &&
            (blue == XDownScale(color.blue)))
          {
            (void) strcpy(name,colorname);
            break;
          }
      }
      (void) fclose(database);
    }
  return(True);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   X G e t W i n d o w I m a g e                                             %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Function XGetWindowImage reads an image from the target X window and returns
%  it.  XGetWindowImage optionally descends the window hierarchy and overlays
%  the target image with each subwindow image.
%
%  The format of the XGetWindowImage routine is:
%
%      image=XGetWindowImage(display,window,borders,level)
%
%  A description of each parameter follows:
%
%    o image: Function XGetWindowImage returns a MIFF image if it can be
%      successfully read from the X window.  A null image is returned if
%      any errors occurs.
%
%    o display: Specifies a connection to an X server;  returned from
%      XOpenDisplay.
%
%    o window: Specifies the window to obtain the image from.
%
%    o borders: Specifies whether borders pixels are to be saved with
%      the image.
%
%    o level: Specifies an unsigned integer representing the level of
%      decent in the window hierarchy.  This value must be zero or one on
%      the initial call to XGetWindowImage.  A value of zero returns after
%      one call.  A value of one causes the function to descend the window
%      hierarchy and overlays the target image with each subwindow image.
%
%
*/
Image *XGetWindowImage(Display *display, Window window, unsigned int borders, unsigned int level)
{
#define LoadImageText  "  Loading image...  "

  typedef struct _ColormapList
  {
    Colormap
      colormap;

    XColor
      *colors;

    struct _ColormapList
      *next;
  } ColormapList;

  GC
    annotate_context;

  Image
    *image;

  int
    display_height,
    display_width,
    number_colors,
    status,
    x_offset,
    y_offset;

  RectangleInfo
    crop_info;

  register int
    i,
    x,
    y;

  register RunlengthPacket
    *p;

  register unsigned long
    pixel;

  static ColormapList
    *colormap_list = (ColormapList *) NULL;

  Window
    child,
    root_window;

  XColor
    *colors;

  XGCValues
    context_values;

  XImage
    *ximage;

  XWindowAttributes
    window_attributes;

  /*
    Verify window is viewable.
  */
  status=XGetWindowAttributes(display,window,&window_attributes);
  if ((status == False) || (window_attributes.map_state != IsViewable))
    return((Image *) NULL);
  /*
    Cropping rectangle is relative to root window.
  */
  root_window=XRootWindow(display,XDefaultScreen(display));
  XTranslateCoordinates(display,window,root_window,0,0,&x_offset,&y_offset,
    &child);
  crop_info.x=x_offset;
  crop_info.y=y_offset;
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
  /*
    Crop to root window.
  */
  if (crop_info.x < 0)
    {
      if ((crop_info.x+(int) crop_info.width) < 0)
        return((Image *) NULL);
      crop_info.width+=crop_info.x;
      crop_info.x=0;
    }
  if (crop_info.y < 0)
    {
      if ((crop_info.y+(int) crop_info.height) < 0)
        return((Image *) NULL);
      crop_info.height+=crop_info.y;
      crop_info.y=0;
    }
  display_width=XDisplayWidth(display,XDefaultScreen(display));
  if ((crop_info.x+(int) crop_info.width) > display_width)
    {
      if (crop_info.x >= display_width)
        return((Image *) NULL);
      crop_info.width=display_width-crop_info.x;
    }
  display_height=XDisplayHeight(display,XDefaultScreen(display));
  if ((crop_info.y+(int) crop_info.height) > display_height)
    {
      if (crop_info.y >= display_height)
        return((Image *) NULL);
      crop_info.height=display_height-crop_info.y;
    }
  crop_info.x-=x_offset;
  crop_info.y-=y_offset;
  /*
    Alert user we are about to get an X region by flashing a border.
  */
  context_values.background=XBlackPixel(display,XDefaultScreen(display));
  context_values.foreground=XWhitePixel(display,XDefaultScreen(display));
  context_values.function=GXinvert;
  context_values.plane_mask=
    context_values.background ^ context_values.foreground;
  context_values.subwindow_mode=IncludeInferiors;
  annotate_context=XCreateGC(display,window,GCBackground | GCForeground |
    GCFunction | GCPlaneMask | GCSubwindowMode,&context_values);
  if (annotate_context != (GC) NULL)
    {
      XHighlightRectangle(display,window,annotate_context,&crop_info);
      XDelay(display,(unsigned long) (SuspendTime << 2));
      XHighlightRectangle(display,window,annotate_context,&crop_info);
    }
  /*
    Get window X image.
  */
  ximage=XGetImage(display,window,crop_info.x,crop_info.y,crop_info.width,
    crop_info.height,AllPlanes,ZPixmap);
  if (ximage == (XImage *) NULL)
    return((Image *) NULL);
  number_colors=0;
  colors=(XColor *) NULL;
  if (window_attributes.colormap != (Colormap) NULL)
    {
      ColormapList
        *p;

      /*
        Search colormap list for window colormap.
      */
      number_colors=window_attributes.visual->map_entries;
      for (p=colormap_list; p != (ColormapList *) NULL; p=p->next)
        if (p->colormap == window_attributes.colormap)
          break;
      if (p == (ColormapList *) NULL)
        {
          /*
            Get the window colormap.
          */
          colors=(XColor *) malloc(number_colors*sizeof(XColor));
          if (colors == (XColor *) NULL)
            {
              XDestroyImage(ximage);
              return((Image *) NULL);
            }
          if ((window_attributes.visual->class != DirectColor) &&
              (window_attributes.visual->class != TrueColor))
            for (i=0; i < number_colors; i++)
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
              red_bit=window_attributes.visual->red_mask &
                (~(window_attributes.visual->red_mask)+1);
              green_bit=window_attributes.visual->green_mask &
                (~(window_attributes.visual->green_mask)+1);
              blue_bit=window_attributes.visual->blue_mask &
                (~(window_attributes.visual->blue_mask)+1);
              for (i=0; i < number_colors; i++)
              {
                colors[i].pixel=red | green | blue;
                colors[i].pad=0;
                red+=red_bit;
                if (red > window_attributes.visual->red_mask)
                  red=0;
                green+=green_bit;
                if (green > window_attributes.visual->green_mask)
                  green=0;
                blue+=blue_bit;
                if (blue > window_attributes.visual->blue_mask)
                  blue=0;
              }
            }
          XQueryColors(display,window_attributes.colormap,colors,
           (int) number_colors);
          /*
            Append colormap to colormap list.
          */
          p=(ColormapList *) malloc(sizeof(ColormapList));
          p->colormap=window_attributes.colormap;
          p->colors=colors;
          p->next=colormap_list;
          colormap_list=p;
        }
      colors=p->colors;
    }
  /*
    Allocate image structure.
  */
  image=AllocateImage((ImageInfo *) NULL);
  if (image == (Image *) NULL)
    {
      XDestroyImage(ximage);
      return((Image *) NULL);
    }
  /*
    Convert X image to MIFF format.
  */
  if ((window_attributes.visual->class != TrueColor) &&
      (window_attributes.visual->class != DirectColor))
    image->class=PseudoClass;
  image->columns=ximage->width;
  image->rows=ximage->height;
  image->packets=image->columns*image->rows;
  image->pixels=(RunlengthPacket *)
    malloc((unsigned int) image->packets*sizeof(RunlengthPacket));
  if (image->pixels == (RunlengthPacket *) NULL)
    {
      XDestroyImage(ximage);
      DestroyImage(image);
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
      red_mask=window_attributes.visual->red_mask;
      red_shift=0;
      while ((red_mask & 0x01) == 0)
      {
        red_mask>>=1;
        red_shift++;
      }
      green_mask=window_attributes.visual->green_mask;
      green_shift=0;
      while ((green_mask & 0x01) == 0)
      {
        green_mask>>=1;
        green_shift++;
      }
      blue_mask=window_attributes.visual->blue_mask;
      blue_shift=0;
      while ((blue_mask & 0x01) == 0)
      {
        blue_mask>>=1;
        blue_shift++;
      }
      /*
        Convert X image to DirectClass packets.
      */
      if ((number_colors > 0) &&
          (window_attributes.visual->class == DirectColor))
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
        }
      break;
    }
    case PseudoClass:
    {
      /*
        Create colormap.
      */
      image->colors=number_colors;
      image->colormap=(ColorPacket *) malloc(image->colors*sizeof(ColorPacket));
      if (image->colormap == (ColorPacket *) NULL)
        {
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
          pixel=XGetPixel(ximage,x,y);
          p->index=(unsigned short) pixel;
          p->length=0;
          p++;
        }
      }
      SyncImage(image);
      break;
    }
  }
  XDestroyImage(ximage);
  if (annotate_context != (GC) NULL)
    {
      /*
        Alert user we got the X region by flashing a border.
      */
      XHighlightRectangle(display,window,annotate_context,&crop_info);
      XFlush(display);
      XHighlightRectangle(display,window,annotate_context,&crop_info);
      XFreeGC(display,annotate_context);
    }
  if (level != 0)
    {
      unsigned int
        number_children;

      Window
        *children,
        parent;

      /*
        Descend the window hierarchy and overlay with each subwindow image.
      */
      status=XQueryTree(display,window,&root_window,&parent,&children,
        &number_children);
      if ((status == True) && (number_children != 0))
        {
          Image
            *child_image;

          /*
            Composite any children in back-to-front order.
          */
          for (i=0; i < number_children; i++)
          {
            child_image=XGetWindowImage(display,children[i],False,level+1);
            if (child_image != (Image *) NULL)
              {
                /*
                  Composite child window image.
                */
                XTranslateCoordinates(display,children[i],window,0,0,&x_offset,
                  &y_offset,&child);
                x_offset-=crop_info.x;
                if (x_offset < 0)
                  x_offset=0;
                y_offset-=crop_info.y;
                if (y_offset < 0)
                  y_offset=0;
                CompositeImage(image,ReplaceCompositeOp,child_image,x_offset,
                  y_offset);
                DestroyImage(child_image);
              }
          }
          XFree((void *) children);
        }
    }
  if (level <= 1)
    {
      ColormapList
        *next;

      /*
        Free resources.
      */
      while (colormap_list != (ColormapList *) NULL)
      {
        next=colormap_list->next;
        free((char *) colormap_list->colors);
        free((char *) colormap_list);
        colormap_list=next;
      }
      if (image->class == PseudoClass)
        CompressColormap(image);
    }
  return(image);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   X G e t W i n d o w I n f o                                               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Function XGetWindowInfo initializes the XWindowInfo structure.
%
%  The format of the XGetWindowInfo routine is:
%
%      XGetWindowInfo(display,visual_info,map_info,pixel_info,font_info,
%        resource_info,window)
%
%  A description of each parameter follows:
%
%    o display: Specifies a connection to an X server; returned from
%      XOpenDisplay.
%
%    o visual_info: Specifies a pointer to a X11 XVisualInfo structure;
%      returned from XGetVisualInfo.
%
%    o map_info: If map_type is specified, this structure is initialized
%      with info from the Standard Colormap.
%
%    o pixel_info: Specifies a pointer to a XPixelInfo structure.
%
%    o font_info: Specifies a pointer to a XFontStruct structure.
%
%    o resource_info: Specifies a pointer to a X11 XResourceInfo structure.
%
%
*/
void XGetWindowInfo(Display *display, XVisualInfo *visual_info, XStandardColormap *map_info, XPixelInfo *pixel_info, XFontStruct *font_info, XResourceInfo *resource_info, XWindowInfo *window)
{
  /*
    Initialize window info.
  */
  if (window->id != (Window) NULL)
    {
      XFreeCursor(display,window->cursor);
      XFreeCursor(display,window->busy_cursor);
      if (window->highlight_stipple != (Pixmap) NULL)
        XFreePixmap(display,window->highlight_stipple);
      if (window->shadow_stipple != (Pixmap) NULL)
        XFreePixmap(display,window->shadow_stipple);
    }
  else
    {
      window->id=(Window) NULL;
      window->x=XDisplayWidth(display,visual_info->screen) >> 1;
      window->y=XDisplayWidth(display,visual_info->screen) >> 1;
      window->ximage=(XImage *) NULL;
      window->matte_image=(XImage *) NULL;
      window->pixmap=(Pixmap) NULL;
      window->matte_pixmap=(Pixmap) NULL;
      window->mapped=False;
      window->stasis=False;
      window->shared_memory=False;
#ifdef HasSharedMemory
      window->shared_memory=False;
      if (resource_info->use_shared_memory)
        window->shared_memory=XShmQueryExtension(display);
      window->segment_info[0].shmid=(-1);
      window->segment_info[1].shmid=(-1);
      window->segment_info[2].shmid=(-1);
      window->segment_info[3].shmid=(-1);
#endif
    }
  window->screen=visual_info->screen;
  window->root=XRootWindow(display,visual_info->screen);
  window->visual=visual_info->visual;
  window->class=visual_info->class;
  window->depth=visual_info->depth;
  window->visual_info=visual_info;
  window->map_info=map_info;
  window->pixel_info=pixel_info;
  window->font_info=font_info;
  window->cursor=XCreateFontCursor(display,XC_left_ptr);
  window->busy_cursor=XCreateFontCursor(display,XC_watch);
  window->name="\0";
  window->geometry=(char *) NULL;
  window->icon_name="\0";
  window->icon_geometry=resource_info->icon_geometry;
  window->crop_geometry=(char *) NULL;
  window->flags=PSize;
  window->width=1;
  window->height=1;
  window->min_width=1;
  window->min_height=1;
  window->width_inc=1;
  window->height_inc=1;
  window->border_width=resource_info->border_width;
  window->annotate_context=pixel_info->annotate_context;
  window->highlight_context=pixel_info->highlight_context;
  window->widget_context=pixel_info->widget_context;
  window->shadow_stipple=(Pixmap) NULL;
  window->highlight_stipple=(Pixmap) NULL;
  window->immutable=False;
  window->data=0;
  window->mask=CWBackingStore | CWBackPixel | CWBackPixmap | CWBitGravity |
    CWBorderPixel | CWColormap | CWCursor | CWDontPropagate | CWEventMask |
    CWOverrideRedirect | CWSaveUnder | CWWinGravity;
  window->attributes.background_pixel=pixel_info->background_color.pixel;
  window->attributes.background_pixmap=(Pixmap) NULL;
  window->attributes.backing_store=NotUseful;
  window->attributes.bit_gravity=ForgetGravity;
  window->attributes.border_pixel=pixel_info->border_color.pixel;
  window->attributes.colormap=map_info->colormap;
  window->attributes.cursor=window->cursor;
  window->attributes.do_not_propagate_mask=NoEventMask;
  window->attributes.event_mask=NoEventMask;
  window->attributes.override_redirect=False;
  window->attributes.save_under=False;
  window->attributes.win_gravity=NorthWestGravity;
  window->orphan=False;
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   X H i g h l i g h t E l l i p s e                                         %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Function XHighlightEllipse puts a border on the X server around a region
%  defined by highlight_info.
%
%  The format of the XHighlightEllipse routine is:
%
%    XHighlightEllipse(display,window,annotate_context,highlight_info)
%
%  A description of each parameter follows:
%
%    o display: Specifies a connection to an X server; returned from
%      XOpenDisplay.
%
%    o window: Specifies a pointer to a Window structure.
%
%    o annotate_context: Specifies a pointer to a GC structure.
%
%    o highlight_info: Specifies a pointer to a RectangleInfo structure.  It
%      contains the extents of any highlighting rectangle.
%
%
*/
void XHighlightEllipse(Display *display, Window window, GC annotate_context, RectangleInfo *highlight_info)
{
  if ((highlight_info->width < 4) || (highlight_info->height < 4))
    return;
  XDrawArc(display,window,annotate_context,highlight_info->x,
    highlight_info->y,highlight_info->width-1,highlight_info->height-1,
    0,360*64);
  XDrawArc(display,window,annotate_context,highlight_info->x+1,
    highlight_info->y+1,highlight_info->width-3,highlight_info->height-3,
    0,360*64);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   X H i g h l i g h t L i n e                                               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Function XHighlightLine puts a border on the X server around a region
%  defined by highlight_info.
%
%  The format of the XHighlightLine routine is:
%
%    XHighlightLine(display,window,annotate_context,highlight_info)
%
%  A description of each parameter follows:
%
%    o display: Specifies a connection to an X server; returned from
%      XOpenDisplay.
%
%    o window: Specifies a pointer to a Window structure.
%
%    o annotate_context: Specifies a pointer to a GC structure.
%
%    o highlight_info: Specifies a pointer to a RectangleInfo structure.  It
%      contains the extents of any highlighting rectangle.
%
%
*/
void XHighlightLine(Display *display, Window window, GC annotate_context, XSegment *highlight_info)
{
  XDrawLine(display,window,annotate_context,highlight_info->x1,
    highlight_info->y1,highlight_info->x2,highlight_info->y2);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   X H i g h l i g h t R e c t a n g l e                                     %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Function XHighlightRectangle puts a border on the X server around a region
%  defined by highlight_info.
%
%  The format of the XHighlightRectangle routine is:
%
%    XHighlightRectangle(display,window,annotate_context,highlight_info)
%
%  A description of each parameter follows:
%
%    o display: Specifies a connection to an X server; returned from
%      XOpenDisplay.
%
%    o window: Specifies a pointer to a Window structure.
%
%    o annotate_context: Specifies a pointer to a GC structure.
%
%    o highlight_info: Specifies a pointer to a RectangleInfo structure.  It
%      contains the extents of any highlighting rectangle.
%
%
*/
void XHighlightRectangle(Display *display, Window window, GC annotate_context, RectangleInfo *highlight_info)
{
  if ((highlight_info->width < 4) || (highlight_info->height < 4))
    return;
  XDrawRectangle(display,window,annotate_context,highlight_info->x,
    highlight_info->y,highlight_info->width-1,highlight_info->height-1);
  XDrawRectangle(display,window,annotate_context,highlight_info->x+1,
    highlight_info->y+1,highlight_info->width-3,highlight_info->height-3);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   X M a k e C u r s o r                                                     %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Function XMakeCursor creates a crosshairs X11 cursor.
%
%  The format of the XMakeCursor routine is:
%
%      XMakeCursor(display,window,colormap,background_color,foreground_color)
%
%  A description of each parameter follows:
%
%    o display: Specifies a connection to an X server;  returned from
%      XOpenDisplay.
%
%    o window: Specifies the ID of the window for which the cursor is
%      assigned.
%
%    o colormap: Specifies the ID of the colormap from which the background
%      and foreground color will be retrieved.
%
%    o background_color: Specifies the color to use for the cursor background.
%
%    o foreground_color: Specifies the color to use for the cursor foreground.
%
%
*/
Cursor XMakeCursor(Display *display, Window window, Colormap colormap, char *background_color, char *foreground_color)
{
#define scope_height 17
#define scope_x_hot 8
#define scope_y_hot 8
#define scope_width 17

  static unsigned char
    scope_bits[] =
    {
      0x80, 0x03, 0x00, 0x80, 0x02, 0x00, 0x80, 0x02, 0x00, 0x80, 0x02,
      0x00, 0x80, 0x02, 0x00, 0x80, 0x02, 0x00, 0x80, 0x02, 0x00, 0x7f,
      0xfc, 0x01, 0x01, 0x00, 0x01, 0x7f, 0xfc, 0x01, 0x80, 0x02, 0x00,
      0x80, 0x02, 0x00, 0x80, 0x02, 0x00, 0x80, 0x02, 0x00, 0x80, 0x02,
      0x00, 0x80, 0x02, 0x00, 0x80, 0x03, 0x00
    };

  static unsigned char
    scope_mask_bits[] =
    {
      0xc0, 0x07, 0x00, 0xc0, 0x07, 0x00, 0xc0, 0x06, 0x00, 0xc0, 0x06,
      0x00, 0xc0, 0x06, 0x00, 0xc0, 0x06, 0x00, 0xff, 0xfe, 0x01, 0x7f,
      0xfc, 0x01, 0x03, 0x80, 0x01, 0x7f, 0xfc, 0x01, 0xff, 0xfe, 0x01,
      0xc0, 0x06, 0x00, 0xc0, 0x06, 0x00, 0xc0, 0x06, 0x00, 0xc0, 0x06,
      0x00, 0xc0, 0x07, 0x00, 0xc0, 0x07, 0x00
    };

  Cursor
    cursor;

  Pixmap
    mask,
    source;

  XColor
    background,
    foreground;

  source=XCreateBitmapFromData(display,window,(char *) scope_bits,scope_width,
    scope_height);
  mask=XCreateBitmapFromData(display,window,(char *) scope_mask_bits,
    scope_width,scope_height);
  if ((source == (Pixmap) NULL) || (mask == (Pixmap) NULL))
    Error("Unable to create pixmap",(char *) NULL);
  XParseColor(display,colormap,background_color,&background);
  XParseColor(display,colormap,foreground_color,&foreground);
  cursor=XCreatePixmapCursor(display,source,mask,&foreground,&background,
    scope_x_hot,scope_y_hot);
  XFreePixmap(display,source);
  XFreePixmap(display,mask);
  return(cursor);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   X M a k e I m a g e                                                       %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Function XMakeImage creates an X11 image.  If the image size differs from
%  the X11 image size, the image is first resized.
%
%  The format of the XMakeImage routine is:
%
%      status=XMakeImage(display,resource_info,window,image,width,height)
%
%  A description of each parameter follows:
%
%    o status: Function XMakeImage returns True if the X image is
%      successfully created.  False is returned is there is a memory shortage.
%
%    o display: Specifies a connection to an X server; returned from
%      XOpenDisplay.
%
%    o resource_info: Specifies a pointer to a X11 XResourceInfo structure.
%
%    o window: Specifies a pointer to a XWindowInfo structure.
%
%    o image: Specifies a pointer to a Image structure;  returned from
%      ReadImage.
%
%    o width: Specifies the width in pixels of the rectangular area to
%      display.
%
%    o height: Specifies the height in pixels of the rectangular area to
%      display.
%
%
*/
unsigned int XMakeImage(Display *display, XResourceInfo *resource_info, XWindowInfo *window, Image *image, unsigned int width, unsigned int height)
{
  Image
    *transformed_image;

  int
    depth,
    format;

  Pixmap
    matte_pixmap;

  XImage
    *matte_image,
    *ximage;

  if ((window->width == 0) || (window->height == 0))
    return(False);
  if (image == (Image *) NULL)
    return(False);
  /*
    Apply user transforms to the image.
  */
  XDefineCursor(display,window->id,window->busy_cursor);
  XFlush(display);
  transformed_image=image;
  if (window->crop_geometry)
    {
      Image
        *cropped_image;

      RectangleInfo
        crop_info;

      /*
        Crop image.
      */
      (void) XParseGeometry(window->crop_geometry,&crop_info.x,&crop_info.y,
        &crop_info.width,&crop_info.height);
      transformed_image->orphan=True;
      cropped_image=CropImage(transformed_image,&crop_info);
      transformed_image->orphan=False;
      if (cropped_image != (Image *) NULL)
        {
          if (transformed_image != image)
            DestroyImage(transformed_image);
          transformed_image=cropped_image;
        }
    }
  if ((width != transformed_image->columns) ||
      (height != transformed_image->rows))
    {
      Image
        *zoomed_image;

      /*
        Scale image.
      */
      transformed_image->orphan=True;
      if ((window->pixel_info->colors != 0) || transformed_image->matte)
        zoomed_image=SampleImage(transformed_image,width,height);
      else
        zoomed_image=ScaleImage(transformed_image,width,height);
      if (zoomed_image != (Image *) NULL)
        {
          if (transformed_image != image)
            DestroyImage(transformed_image);
          transformed_image=zoomed_image;
        }
      transformed_image->orphan=False;
    }
  width=transformed_image->columns;
  height=transformed_image->rows;
  depth=window->depth;
  if (transformed_image->class == PseudoClass)
    if (IsGrayImage(transformed_image) && (transformed_image->colors == 2))
      depth=1;
  /*
    Create X image.
  */
  format=(depth == 1) ? XYBitmap : ZPixmap;
#ifdef HasSharedMemory
  if (window->shared_memory)
    {
      ximage=XShmCreateImage(display,window->visual,depth,format,0,
        &window->segment_info[2],width,height);
      window->segment_info[2].shmid=shmget(IPC_PRIVATE,ximage->bytes_per_line*
        ximage->height,IPC_CREAT | 0777);
      window->shared_memory=window->segment_info[2].shmid >= 0;
      if (window->shared_memory)
        window->segment_info[2].shmaddr=(char *)
          shmat(window->segment_info[2].shmid,0,0);
    }
#endif
  if (!window->shared_memory)
    ximage=XCreateImage(display,window->visual,depth,format,0,(char *) NULL,
      width,height,XBitmapPad(display),0);
  if (ximage == (XImage *) NULL)
    {
      /*
        Unable to create X image.
      */
      if (transformed_image != image)
        DestroyImage(transformed_image);
      XDefineCursor(display,window->id,window->cursor);
      return(False);
    }
  if (resource_info->debug)
    {
      (void) fprintf(stderr,"XImage:\n");
      (void) fprintf(stderr,"  width, height: %dx%d\n",ximage->width,
        ximage->height);
      (void) fprintf(stderr,"  format: %d\n",ximage->format);
      (void) fprintf(stderr,"  Byte order: %d\n",ximage->byte_order);
      (void) fprintf(stderr,"  bitmap unit, bit order, pad: %d %d %d\n",
        ximage->bitmap_unit,ximage->bitmap_bit_order,ximage->bitmap_pad);
      (void) fprintf(stderr,"  depth: %d\n",ximage->depth);
      (void) fprintf(stderr,"  bytes per line: %d\n",ximage->bytes_per_line);
      (void) fprintf(stderr,"  bits per pixel: %d\n",ximage->bits_per_pixel);
      (void) fprintf(stderr,"  red, green, blue masks: 0x%lx 0x%lx 0x%lx\n",
        ximage->red_mask,ximage->green_mask,ximage->blue_mask);
    }
  /*
    Allocate X image pixel data.
  */
#ifdef HasSharedMemory
  if (window->shared_memory)
    {
      xerror_alert=False;
      window->segment_info[2].readOnly=False;
      XShmAttach(display,&window->segment_info[2]);
      XSync(display,False);
      shmctl(window->segment_info[2].shmid,IPC_RMID,0);
      ximage->data=window->segment_info[2].shmaddr;
      if (xerror_alert)
        {
          window->shared_memory=False;
          xerror_alert=False;
        }
    }
#endif
  if (!window->shared_memory)
    if (ximage->format == XYBitmap)
      ximage->data=(char *)
        malloc(ximage->bytes_per_line*ximage->height*ximage->depth);
    else
      ximage->data=(char *) malloc(ximage->bytes_per_line*ximage->height);
  if (ximage->data == (char *) NULL)
    {
      /*
        Unable to allocate pixel data.
      */
      if (transformed_image != image)
        DestroyImage(transformed_image);
      XDestroyImage(ximage);
      XDefineCursor(display,window->id,window->cursor);
      return(False);
    }
  if (window->ximage != (XImage *) NULL)
    {
#ifdef HasSharedMemory
      if (window->shared_memory)
        {
          XShmDetach(display,&window->segment_info[0]);
          XDestroyImage(window->ximage);
          shmdt(window->segment_info[0].shmaddr);
        }
#endif
      if (!window->shared_memory)
        XDestroyImage(window->ximage);
    }
#ifdef HasSharedMemory
  window->segment_info[0]=window->segment_info[2];
#endif
  window->ximage=ximage;
  matte_image=(XImage *) NULL;
  if (transformed_image != (Image *) NULL)
    if (transformed_image->matte)
      {
        /*
          Create matte image.
        */
        matte_image=XCreateImage(display,window->visual,1,XYBitmap,0,
          (char *) NULL,width,height,XBitmapPad(display),0);
        if (resource_info->debug)
          {
            (void) fprintf(stderr,"Matte Image:\n");
            (void) fprintf(stderr,"  width, height: %dx%d\n",matte_image->width,
              matte_image->height);
          }
        if (matte_image != (XImage *) NULL)
          {
            /*
              Allocate matte image pixel data.
            */
            matte_image->data=(char *) malloc(matte_image->bytes_per_line*
              matte_image->height*matte_image->depth);
            if (matte_image->data == (char *) NULL)
              {
                XDestroyImage(matte_image);
                matte_image=(XImage *) NULL;
              }
          }
      }
  if (window->matte_image != (XImage *) NULL)
    XDestroyImage(window->matte_image);
  window->matte_image=matte_image;
  window->stasis=False;
  /*
    Convert runlength-encoded pixels to X image data.
  */
  if ((ximage->byte_order == LSBFirst) ||
      ((ximage->format == XYBitmap) && (ximage->bitmap_bit_order == LSBFirst)))
    XMakeImageLSBFirst(resource_info,window,transformed_image,ximage,
      matte_image);
  else
    XMakeImageMSBFirst(resource_info,window,transformed_image,ximage,
      matte_image);
  matte_pixmap=window->matte_pixmap;
  if (window->matte_pixmap != (Pixmap) NULL)
    XFreePixmap(display,window->matte_pixmap);
  window->matte_pixmap=(Pixmap) NULL;
  if (window->matte_image != (XImage *) NULL)
    {
      /*
        Create matte pixmap.
      */
      window->matte_pixmap=XCreatePixmap(display,window->id,width,height,1);
      if (window->matte_pixmap != (Pixmap) NULL)
        {
          GC
            graphics_context;

          XGCValues
            context_values;

          /*
            Copy matte image to matte pixmap.
          */
          context_values.background=1;
          context_values.foreground=0;
          graphics_context=XCreateGC(display,window->matte_pixmap,GCBackground |
            GCForeground,&context_values);
          XPutImage(display,window->matte_pixmap,graphics_context,
            window->matte_image,0,0,0,0,width,height);
          XFreeGC(display,graphics_context);
        }
    }
#ifdef HasShape
  if (window->matte_pixmap != matte_pixmap)
    XShapeCombineMask(display,window->id,ShapeBounding,0,0,
      window->matte_pixmap,ShapeSet);
#endif
  if (transformed_image != image)
    DestroyImage(transformed_image);
  /*
    Restore cursor.
  */
  XDefineCursor(display,window->id,window->cursor);
  return(True);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   X M a k e I m a g e L S B F i r s t                                       %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Function XMakeImageLSBFirst initializes the pixel data of an X11 Image.
%  The X image pixels are copied in least-significant bit and Byte first
%  order.  The server's scanline pad is respected.  Rather than using one or
%  two general cases, many special cases are found here to help speed up the
%  image conversion.
%
%  The format of the XMakeImageLSBFirst routine is:
%
%      XMakeImageLSBFirst(resource_info,window,image,ximage,matte_image)
%
%  A description of each parameter follows:
%
%    o resource_info: Specifies a pointer to a X11 XResourceInfo structure.
%
%    o window: Specifies a pointer to a XWindowInfo structure.
%
%    o image: Specifies a pointer to a Image structure;  returned from
%      ReadImage.
%
%    o ximage: Specifies a pointer to a XImage structure;  returned from
%      XCreateImage.
%
%    o matte_image: Specifies a pointer to a XImage structure;  returned from
%      XCreateImage.
%
*/
static void XMakeImageLSBFirst(XResourceInfo *resource_info, XWindowInfo *window, Image *image, XImage *ximage, XImage *matte_image)
{
  register int
    i,
    j,
    x;

  register RunlengthPacket
    *p;

  register unsigned char
    bit,
    Byte,
    *q;

  register unsigned long
    pixel;

  register XColor
    *gamma_map;

  unsigned int
    scanline_pad;

  unsigned long
    *pixels;

  XStandardColormap
    *map_info;

  scanline_pad=ximage->bytes_per_line-
    ((ximage->width*ximage->bits_per_pixel) >> 3);
  map_info=window->map_info;
  pixels=window->pixel_info->pixels;
  gamma_map=window->pixel_info->gamma_map;
  p=image->pixels;
  q=(unsigned char *) ximage->data;
  x=0;
  if (ximage->format == XYBitmap)
    {
      register unsigned short
        polarity;

      unsigned char
        background,
        foreground;

      /*
        Convert image to big-endian bitmap.
      */
      background=(Intensity(window->pixel_info->foreground_color) <
        Intensity(window->pixel_info->background_color) ? 0x80 : 0x00);
      foreground=(Intensity(window->pixel_info->background_color) <
        Intensity(window->pixel_info->foreground_color) ? 0x80 : 0x00);
      polarity=Intensity(image->colormap[0]) < Intensity(image->colormap[1]);
      bit=0;
      Byte=0;
      for (i=0; i < image->packets; i++)
      {
        for (j=0; j <= ((int) p->length); j++)
        {
          Byte>>=1;
          if (p->index == polarity)
            Byte|=foreground;
          else
            Byte|=background;
          bit++;
          if (bit == 8)
            {
              *q++=Byte;
              bit=0;
              Byte=0;
            }
          x++;
          if (x == ximage->width)
            {
              /*
                Advance to the next scanline.
              */
              if (bit != 0)
                *q=Byte >> (8-bit);
              q+=scanline_pad;
              bit=0;
              Byte=0;
              x=0;
            }
        }
        p++;
      }
    }
  else
    if (window->pixel_info->colors != 0)
      switch (ximage->bits_per_pixel)
      {
        case 2:
        {
          register unsigned int
            nibble;

          /*
            Convert to 2 bit color-mapped X image.
          */
          nibble=0;
          for (i=0; i < image->packets; i++)
          {
            pixel=pixels[p->index] & 0xf;
            for (j=0; j <= ((int) p->length); j++)
            {
              switch (nibble)
              {
                case 0:
                {
                  *q=(unsigned char) pixel;
                  nibble++;
                  break;
                }
                case 1:
                {
                  *q|=(unsigned char) (pixel << 2);
                  nibble++;
                  break;
                }
                case 2:
                {
                  *q|=(unsigned char) (pixel << 4);
                  nibble++;
                  break;
                }
                case 3:
                {
                  *q|=(unsigned char) (pixel << 6);
                  q++;
                  nibble=0;
                  break;
                }
              }
              x++;
              if (x == ximage->width)
                {
                  x=0;
                  nibble=0;
                  q+=scanline_pad;
                }
            }
            p++;
          }
          break;
        }
        case 4:
        {
          register unsigned int
            nibble;

          /*
            Convert to 4 bit color-mapped X image.
          */
          nibble=0;
          for (i=0; i < image->packets; i++)
          {
            pixel=pixels[p->index] & 0xf;
            for (j=0; j <= ((int) p->length); j++)
            {
              switch (nibble)
              {
                case 0:
                {
                  *q=(unsigned char) pixel;
                  nibble++;
                  break;
                }
                case 1:
                {
                  *q|=(unsigned char) (pixel << 4);
                  q++;
                  nibble=0;
                  break;
                }
              }
              x++;
              if (x == ximage->width)
                {
                  x=0;
                  nibble=0;
                  q+=scanline_pad;
                }
            }
            p++;
          }
          break;
        }
        case 6:
        case 8:
        {
          /*
            Convert to 8 bit color-mapped X image.
          */
          if (resource_info->color_recovery && resource_info->dither)
            {
              XDitherImage(image,ximage);
              break;
            }
          for (i=0; i < image->packets; i++)
          {
            pixel=pixels[p->index];
            for (j=0; j <= ((int) p->length); j++)
            {
              *q++=(unsigned char) pixel;
              x++;
              if (x == ximage->width)
                {
                  x=0;
                  q+=scanline_pad;
                }
            }
            p++;
          }
          break;
        }
        default:
        {
          register int
            k;

          register unsigned int
            bytes_per_pixel;

          unsigned char
            channel[sizeof(unsigned long)];

          /*
            Convert to multi-Byte color-mapped X image.
          */
          bytes_per_pixel=ximage->bits_per_pixel >> 3;
          for (i=0; i < image->packets; i++)
          {
            pixel=pixels[p->index];
            for (k=0; k < bytes_per_pixel; k++)
            {
              channel[k]=(unsigned char) pixel;
              pixel>>=8;
            }
            for (j=0; j <= ((int) p->length); j++)
            {
              for (k=0; k < bytes_per_pixel; k++)
                *q++=channel[k];
              x++;
              if (x == ximage->width)
                {
                  x=0;
                  q+=scanline_pad;
                }
            }
            p++;
          }
          break;
        }
      }
    else
      switch (ximage->bits_per_pixel)
      {
        case 2:
        {
          register unsigned int
            nibble;

          /*
            Convert to contiguous 2 bit continuous-tone X image.
          */
          nibble=0;
          for (i=0; i < image->packets; i++)
          {
            pixel=XGammaPixel(map_info,gamma_map,p,QuantumDepth);
            pixel&=0xf;
            for (j=0; j <= ((int) p->length); j++)
            {
              switch (nibble)
              {
                case 0:
                {
                  *q=(unsigned char) pixel;
                  nibble++;
                  break;
                }
                case 1:
                {
                  *q|=(unsigned char) (pixel << 2);
                  nibble++;
                  break;
                }
                case 2:
                {
                  *q|=(unsigned char) (pixel << 4);
                  nibble++;
                  break;
                }
                case 3:
                {
                  *q|=(unsigned char) (pixel << 6);
                  q++;
                  nibble=0;
                  break;
                }
              }
              x++;
              if (x == ximage->width)
                {
                  x=0;
                  nibble=0;
                  q+=scanline_pad;
                }
            }
            p++;
          }
          break;
        }
        case 4:
        {
          register unsigned int
            nibble;

          /*
            Convert to contiguous 4 bit continuous-tone X image.
          */
          nibble=0;
          for (i=0; i < image->packets; i++)
          {
            pixel=XGammaPixel(map_info,gamma_map,p,QuantumDepth);
            pixel&=0xf;
            for (j=0; j <= ((int) p->length); j++)
            {
              switch (nibble)
              {
                case 0:
                {
                  *q=(unsigned char) pixel;
                  nibble++;
                  break;
                }
                case 1:
                {
                  *q|=(unsigned char) (pixel << 4);
                  q++;
                  nibble=0;
                  break;
                }
              }
              x++;
              if (x == ximage->width)
                {
                  x=0;
                  nibble=0;
                  q+=scanline_pad;
                }
            }
            p++;
          }
          break;
        }
        case 6:
        case 8:
        {
          /*
            Convert to contiguous 8 bit continuous-tone X image.
          */
          if (resource_info->color_recovery && resource_info->dither)
            {
              XDitherImage(image,ximage);
              break;
            }
          for (i=0; i < image->packets; i++)
          {
            pixel=XGammaPixel(map_info,gamma_map,p,QuantumDepth);
            for (j=0; j <= ((int) p->length); j++)
            {
              *q++=(unsigned char) pixel;
              x++;
              if (x == ximage->width)
                {
                  x=0;
                  q+=scanline_pad;
                }
            }
            p++;
          }
          break;
        }
        default:
        {
          if ((ximage->bits_per_pixel == 32) && (map_info->red_max == 255) &&
              (map_info->green_max == 255) && (map_info->blue_max == 255) &&
              (map_info->red_mult == 65536) && (map_info->green_mult == 256) &&
              (map_info->blue_mult == 1))
            {
              /*
                Convert to 32 bit continuous-tone X image.
              */
              for (i=0; i < image->packets; i++)
              {
                for (j=0; j <= ((int) p->length); j++)
                {
                  *q++=DownScale(gamma_map[p->blue].blue);
                  *q++=DownScale(gamma_map[p->green].green);
                  *q++=DownScale(gamma_map[p->red].red);
                  *q++=0;
                }
                p++;
              }
            }
          else
            if ((ximage->bits_per_pixel == 32) && (map_info->red_max == 255) &&
                (map_info->green_max == 255) && (map_info->blue_max == 255) &&
                (map_info->red_mult == 1) && (map_info->green_mult == 256) &&
                (map_info->blue_mult == 65536))
              {
                /*
                  Convert to 32 bit continuous-tone X image.
                */
                for (i=0; i < image->packets; i++)
                {
                  for (j=0; j <= ((int) p->length); j++)
                  {
                    *q++=DownScale(gamma_map[p->red].red);
                    *q++=DownScale(gamma_map[p->green].green);
                    *q++=DownScale(gamma_map[p->blue].blue);
                    *q++=0;
                  }
                  p++;
                }
              }
            else
              {
                register int
                  k;

                register unsigned int
                  bytes_per_pixel;

                unsigned char
                  channel[sizeof(unsigned long)];

                /*
                  Convert to multi-Byte continuous-tone X image.
                */
                bytes_per_pixel=ximage->bits_per_pixel >> 3;
                for (i=0; i < image->packets; i++)
                {
                  pixel=XGammaPixel(map_info,gamma_map,p,QuantumDepth);
                  for (k=0; k < bytes_per_pixel; k++)
                  {
                    channel[k]=(unsigned char) pixel;
                    pixel>>=8;
                  }
                  for (j=0; j <= ((int) p->length); j++)
                  {
                    for (k=0; k < bytes_per_pixel; k++)
                      *q++=channel[k];
                    x++;
                    if (x == ximage->width)
                      {
                        x=0;
                        q+=scanline_pad;
                      }
                  }
                  p++;
                }
              }
          break;
        }
      }
  if (matte_image != (XImage *) NULL)
    {
      /*
        Initialize matte image.
      */
      scanline_pad=matte_image->bytes_per_line-
        ((matte_image->width*matte_image->bits_per_pixel) >> 3);
      p=image->pixels;
      q=(unsigned char *) matte_image->data;
      bit=0;
      Byte=0;
      for (i=0; i < image->packets; i++)
      {
        for (j=0; j <= ((int) p->length); j++)
        {
          Byte>>=1;
          if (p->index == Transparent)
            Byte|=0x01;
          bit++;
          if (bit == 8)
            {
              *q++=Byte;
              bit=0;
              Byte=0;
            }
          x++;
          if (x == matte_image->width)
            {
              /*
                Advance to the next scanline.
              */
              if (bit != 0)
                *q=Byte >> (8-bit);
              q+=scanline_pad;
              bit=0;
              Byte=0;
              x=0;
            }
        }
        p++;
      }
    }
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   X M a k e I m a g e M S B F i r s t                                       %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Function XMakeImageMSBFirst initializes the pixel data of an X11 Image.
%  The X image pixels are copied in most-significant bit and Byte first order.
%  The server's scanline pad is also resprected. Rather than using one or two
%  general cases, many special cases are found here to help speed up the image
%  conversion.
%
%  The format of the XMakeImageMSBFirst routine is:
%
%      XMakeImageMSBFirst(resource_info,window,image,ximage,matte_image)
%
%  A description of each parameter follows:
%
%    o resource_info: Specifies a pointer to a X11 XResourceInfo structure.
%
%    o window: Specifies a pointer to a XWindowInfo structure.
%
%    o image: Specifies a pointer to a Image structure;  returned from
%      ReadImage.
%
%    o ximage: Specifies a pointer to a XImage structure;  returned from
%      XCreateImage.
%
%    o matte_image: Specifies a pointer to a XImage structure;  returned from
%      XCreateImage.
%
%
*/
static void XMakeImageMSBFirst(XResourceInfo *resource_info, XWindowInfo *window, Image *image, XImage *ximage, XImage *matte_image)
{
  register int
    i,
    j,
    x;

  register RunlengthPacket
    *p;

  register unsigned char
    bit,
    Byte,
    *q;

  register unsigned long
    pixel;

  register XColor
    *gamma_map;

  unsigned int
    scanline_pad;

  unsigned long
    *pixels;

  XStandardColormap
    *map_info;

  scanline_pad=ximage->bytes_per_line-
    ((ximage->width*ximage->bits_per_pixel) >> 3);
  map_info=window->map_info;
  pixels=window->pixel_info->pixels;
  gamma_map=window->pixel_info->gamma_map;
  p=image->pixels;
  q=(unsigned char *) ximage->data;
  x=0;
  if (ximage->format == XYBitmap)
    {
      register unsigned short
        polarity;

      unsigned char
        background,
        foreground;

      /*
        Convert image to big-endian bitmap.
      */
      background=(Intensity(window->pixel_info->foreground_color) <
        Intensity(window->pixel_info->background_color) ? 0x01 : 0x00);
      foreground=(Intensity(window->pixel_info->background_color) <
        Intensity(window->pixel_info->foreground_color) ? 0x01 : 0x00);
      polarity=Intensity(image->colormap[0]) < Intensity(image->colormap[1]);
      bit=0;
      Byte=0;
      for (i=0; i < image->packets; i++)
      {
        for (j=0; j <= ((int) p->length); j++)
        {
          Byte<<=1;
          if (p->index == polarity)
            Byte|=foreground;
          else
            Byte|=background;
          bit++;
          if (bit == 8)
            {
              *q++=Byte;
              bit=0;
              Byte=0;
            }
          x++;
          if (x == ximage->width)
            {
              /*
                Advance to the next scanline.
              */
              if (bit != 0)
                *q=Byte << (8-bit);
              q+=scanline_pad;
              bit=0;
              Byte=0;
              x=0;
            }
        }
        p++;
      }
    }
  else
    if (window->pixel_info->colors != 0)
      switch (ximage->bits_per_pixel)
      {
        case 2:
        {
          register unsigned int
            nibble;

          /*
            Convert to 2 bit color-mapped X image.
          */
          nibble=0;
          for (i=0; i < image->packets; i++)
          {
            pixel=pixels[p->index] & 0xf;
            for (j=0; j <= ((int) p->length); j++)
            {
              switch (nibble)
              {
                case 0:
                {
                  *q=(unsigned char) (pixel << 6);
                  nibble++;
                  break;
                }
                case 1:
                {
                  *q|=(unsigned char) (pixel << 4);
                  nibble++;
                  break;
                }
                case 2:
                {
                  *q|=(unsigned char) (pixel << 2);
                  nibble++;
                  break;
                }
                case 3:
                {
                  *q|=(unsigned char) pixel;
                  q++;
                  nibble=0;
                  break;
                }
              }
              x++;
              if (x == ximage->width)
                {
                  x=0;
                  nibble=0;
                  q+=scanline_pad;
                }
            }
            p++;
          }
          break;
        }
        case 4:
        {
          register unsigned int
            nibble;

          /*
            Convert to 4 bit color-mapped X image.
          */
          nibble=0;
          for (i=0; i < image->packets; i++)
          {
            pixel=pixels[p->index] & 0xf;
            for (j=0; j <= ((int) p->length); j++)
            {
              switch (nibble)
              {
                case 0:
                {
                  *q=(unsigned char) (pixel << 4);
                  nibble++;
                  break;
                }
                case 1:
                {
                  *q|=(unsigned char) pixel;
                  q++;
                  nibble=0;
                  break;
                }
              }
              x++;
              if (x == ximage->width)
                {
                  x=0;
                  nibble=0;
                  q+=scanline_pad;
                }
            }
            p++;
          }
          break;
        }
        case 6:
        case 8:
        {
          /*
            Convert to 8 bit color-mapped X image.
          */
          if (resource_info->color_recovery && resource_info->dither)
            {
              XDitherImage(image,ximage);
              break;
            }
          for (i=0; i < image->packets; i++)
          {
            pixel=pixels[p->index];
            for (j=0; j <= ((int) p->length); j++)
            {
              *q++=(unsigned char) pixel;
              x++;
              if (x == ximage->width)
                {
                  x=0;
                  q+=scanline_pad;
                }
            }
            p++;
          }
          break;
        }
        default:
        {
          register int
            k;

          register unsigned int
            bytes_per_pixel;

          unsigned char
            channel[sizeof(unsigned long)];

          /*
            Convert to 8 bit color-mapped X image.
          */
          bytes_per_pixel=ximage->bits_per_pixel >> 3;
          for (i=0; i < image->packets; i++)
          {
            pixel=pixels[p->index];
            for (k=bytes_per_pixel-1; k >= 0; k--)
            {
              channel[k]=(unsigned char) pixel;
              pixel>>=8;
            }
            for (j=0; j <= ((int) p->length); j++)
            {
              for (k=0; k < bytes_per_pixel; k++)
                *q++=channel[k];
              x++;
              if (x == ximage->width)
                {
                  x=0;
                  q+=scanline_pad;
                }
            }
            p++;
          }
          break;
        }
      }
    else
      switch (ximage->bits_per_pixel)
      {
        case 2:
        {
          register unsigned int
            nibble;

          /*
            Convert to 4 bit continuous-tone X image.
          */
          nibble=0;
          for (i=0; i < image->packets; i++)
          {
            pixel=XGammaPixel(map_info,gamma_map,p,QuantumDepth);
            pixel&=0xf;
            for (j=0; j <= ((int) p->length); j++)
            {
              switch (nibble)
              {
                case 0:
                {
                  *q=(unsigned char) (pixel << 6);
                  nibble++;
                  break;
                }
                case 1:
                {
                  *q|=(unsigned char) (pixel << 4);
                  nibble++;
                  break;
                }
                case 2:
                {
                  *q|=(unsigned char) (pixel << 2);
                  nibble++;
                  break;
                }
                case 3:
                {
                  *q|=(unsigned char) pixel;
                  q++;
                  nibble=0;
                  break;
                }
              }
              x++;
              if (x == ximage->width)
                {
                  x=0;
                  nibble=0;
                  q+=scanline_pad;
                }
            }
            p++;
          }
          break;
        }
        case 4:
        {
          register unsigned int
            nibble;

          /*
            Convert to 4 bit continuous-tone X image.
          */
          nibble=0;
          for (i=0; i < image->packets; i++)
          {
            pixel=XGammaPixel(map_info,gamma_map,p,QuantumDepth);
            pixel&=0xf;
            for (j=0; j <= ((int) p->length); j++)
            {
              switch (nibble)
              {
                case 0:
                {
                  *q=(unsigned char) (pixel << 4);
                  nibble++;
                  break;
                }
                case 1:
                {
                  *q|=(unsigned char) pixel;
                  q++;
                  nibble=0;
                  break;
                }
              }
              x++;
              if (x == ximage->width)
                {
                  x=0;
                  nibble=0;
                  q+=scanline_pad;
                }
            }
            p++;
          }
          break;
        }
        case 6:
        case 8:
        {
          /*
            Convert to 8 bit continuous-tone X image.
          */
          if (resource_info->color_recovery && resource_info->dither)
            {
              XDitherImage(image,ximage);
              break;
            }
          for (i=0; i < image->packets; i++)
          {
            pixel=XGammaPixel(map_info,gamma_map,p,QuantumDepth);
            for (j=0; j <= ((int) p->length); j++)
            {
              *q++=(unsigned char) pixel;
              x++;
              if (x == ximage->width)
                {
                  x=0;
                  q+=scanline_pad;
                }
            }
            p++;
          }
          break;
        }
        default:
        {
          if ((ximage->bits_per_pixel == 32) && (map_info->red_max == 255) &&
              (map_info->green_max == 255) && (map_info->blue_max == 255) &&
              (map_info->red_mult == 65536) && (map_info->green_mult == 256) &&
              (map_info->blue_mult == 1))
            {
              /*
                Convert to 32 bit continuous-tone X image.
              */
              for (i=0; i < image->packets; i++)
              {
                for (j=0; j <= ((int) p->length); j++)
                {
                  *q++=0;
                  *q++=DownScale(gamma_map[p->red].red);
                  *q++=DownScale(gamma_map[p->green].green);
                  *q++=DownScale(gamma_map[p->blue].blue);
                }
                p++;
              }
            }
          else
            if ((ximage->bits_per_pixel == 32) && (map_info->red_max == 255) &&
                (map_info->green_max == 255) && (map_info->blue_max == 255) &&
                (map_info->red_mult == 1) && (map_info->green_mult == 256) &&
                (map_info->blue_mult == 65536))
              {
                /*
                  Convert to 32 bit continuous-tone X image.
                */
                for (i=0; i < image->packets; i++)
                {
                  for (j=0; j <= ((int) p->length); j++)
                  {
                    *q++=0;
                    *q++=DownScale(gamma_map[p->blue].blue);
                    *q++=DownScale(gamma_map[p->green].green);
                    *q++=DownScale(gamma_map[p->red].red);
                  }
                  p++;
                }
              }
            else
              {
                register int
                  k;

                register unsigned int
                  bytes_per_pixel;

                unsigned char
                  channel[sizeof(unsigned long)];

                /*
                  Convert to multi-Byte continuous-tone X image.
                */
                bytes_per_pixel=ximage->bits_per_pixel >> 3;
                for (i=0; i < image->packets; i++)
                {
                  pixel=XGammaPixel(map_info,gamma_map,p,QuantumDepth);
                  for (k=bytes_per_pixel-1; k >= 0; k--)
                  {
                    channel[k]=(unsigned char) pixel;
                    pixel>>=8;
                  }
                  for (j=0; j <= ((int) p->length); j++)
                  {
                    for (k=0; k < bytes_per_pixel; k++)
                      *q++=channel[k];
                    x++;
                    if (x == ximage->width)
                      {
                        x=0;
                        q+=scanline_pad;
                      }
                  }
                  p++;
                }
              }
          break;
        }
      }
  if (matte_image != (XImage *) NULL)
    {
      /*
        Initialize matte image.
      */
      scanline_pad=matte_image->bytes_per_line-
        ((matte_image->width*matte_image->bits_per_pixel) >> 3);
      p=image->pixels;
      q=(unsigned char *) matte_image->data;
      bit=0;
      Byte=0;
      for (i=0; i < image->packets; i++)
      {
        for (j=0; j <= ((int) p->length); j++)
        {
          Byte<<=1;
          if (p->index == Transparent)
            Byte|=0x01;
          bit++;
          if (bit == 8)
            {
              *q++=Byte;
              bit=0;
              Byte=0;
            }
          x++;
          if (x == matte_image->width)
            {
              /*
                Advance to the next scanline.
              */
              if (bit != 0)
                *q=Byte << (8-bit);
              q+=scanline_pad;
              bit=0;
              Byte=0;
              x=0;
            }
        }
        p++;
      }
    }
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   X M a k e M a g n i f y I m a g e                                         %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Function XMakeMagnifyImage magnifies a region of an X image and displays it.
%
%  The format of the XMakeMagnifyImage routine is:
%
%      XMakeMagnifyImage(display,windows)
%
%  A description of each parameter follows:
%
%    o display: Specifies a connection to an X server;  returned from
%      XOpenDisplay.
%
%    o windows: Specifies a pointer to a XWindows structure.
%
%
*/
void XMakeMagnifyImage(Display *display, XWindows *windows)
{
#define Swap(x,y) ((x)^=(y), (y)^=(x), (x)^=(y))

  register int
    x,
    y;

  register unsigned char
    *p,
    *q;

  register unsigned int
    j,
    k,
    l;

  static char
    text[MaxTextLength];

  static unsigned int
    previous_magnify=0;

  static XWindowInfo
    magnify_window;

  unsigned int
    height,
    i,
    magnify,
    scanline_pad,
    width;

  XColor
    color;

  XImage
    *ximage;

  /* Kobus */
  extern unsigned char* pure_pixels;

  if (pure_pixels == (unsigned char *)NULL)
  {
      XInfoWidget(display,windows,"Error mapping pixels");
      XBell(display,0);
      return;
  }
  /* End Kobus */



  /*
    Check boundry conditions.
  */
  magnify=1;
  for (i=1; i < windows->magnify.data; i++)
    magnify<<=1;
  while ((magnify*windows->image.ximage->width) < windows->magnify.width)
    magnify<<=1;
  while ((magnify*windows->image.ximage->height) < windows->magnify.height)
    magnify<<=1;
  while (magnify > windows->magnify.width)
    magnify>>=1;
  while (magnify > windows->magnify.height)
    magnify>>=1;
  if (magnify != previous_magnify)
    {
      unsigned int
        status;

      XTextProperty
        window_name;

      /*
        New magnify factor:  update magnify window name.
      */
      i=0;
      while ((1 << i) <= magnify)
        i++;
      (void) sprintf(windows->magnify.name,"Magnify %uX",i);
      status=XStringListToTextProperty(&windows->magnify.name,1,&window_name);
      if (status != 0)
        {
          XSetWMName(display,windows->magnify.id,&window_name);
          XSetWMIconName(display,windows->magnify.id,&window_name);
          XFree((void *) window_name.value);
        }
    }
  previous_magnify=magnify;
  ximage=windows->image.ximage;
  width=windows->magnify.ximage->width;
  height=windows->magnify.ximage->height;
  x=windows->magnify.x-((width/magnify) >> 1);
  if (x < 0)
    x=0;
  else
    if (x > (ximage->width-(width/magnify)))
      x=ximage->width-width/magnify;
  y=windows->magnify.y-((height/magnify) >> 1);
  if (y < 0)
    y=0;
  else
    if (y > (ximage->height-(height/magnify)))
      y=ximage->height-height/magnify;
  q=(unsigned char *) windows->magnify.ximage->data;
  scanline_pad=windows->magnify.ximage->bytes_per_line-
    ((width*windows->magnify.ximage->bits_per_pixel) >> 3);
  if (ximage->bits_per_pixel < 8)
    {
      register unsigned char
        background,
        Byte,
        foreground,
        p_bit,
        q_bit;

      register unsigned int
        plane;

      XPixelInfo
        *pixel_info;

      pixel_info=windows->magnify.pixel_info;
      switch (ximage->bitmap_bit_order)
      {
        case LSBFirst:
        {
          /*
            Magnify little-endian bitmap.
          */
          background=0x00;
          foreground=0x80;
          if (ximage->format == XYBitmap)
            {
              background=(Intensity(pixel_info->foreground_color) <
                Intensity(pixel_info->background_color) ? 0x80 : 0x00);
              foreground=(Intensity(pixel_info->background_color) <
                Intensity(pixel_info->foreground_color) ? 0x80 : 0x00);
              if (windows->magnify.depth > 1)
                Swap(background,foreground);
            }
          for (i=0; i < height; i+=magnify)
          {
            /*
              Propogate pixel magnify rows.
            */
            for (j=0; j < magnify; j++)
            {
              p=(unsigned char *) ximage->data+y*ximage->bytes_per_line+
                ((x*ximage->bits_per_pixel) >> 3);
              p_bit=(x*ximage->bits_per_pixel) & 0x07;
              q_bit=0;
              Byte=0;
              for (k=0; k < width; k+=magnify)
              {
                /*
                  Propogate pixel magnify columns.
                */
                for (l=0; l < magnify; l++)
                {
                  /*
                    Propogate each bit plane.
                  */
                  for (plane=0; plane < ximage->bits_per_pixel; plane++)
                  {
                    Byte>>=1;
                    if (*p & (0x01 << (p_bit+plane)))
                      Byte|=foreground;
                    else
                      Byte|=background;
                    q_bit++;
                    if (q_bit == 8)
                      {
                        *q++=Byte;
                        q_bit=0;
                        Byte=0;
                      }
                  }
                }
                p_bit+=ximage->bits_per_pixel;
                if (p_bit == 8)
                  {
                    p++;
                    p_bit=0;
                  }
                if (q_bit != 0)
                  *q=Byte >> (8-q_bit);
                q+=scanline_pad;
              }
            }
            y++;
          }
          break;
        }
        case MSBFirst:
        default:
        {
          /*
            Magnify big-endian bitmap.
          */
          background=0x00;
          foreground=0x01;
          if (ximage->format == XYBitmap)
            {
              background=(Intensity(pixel_info->foreground_color) <
                Intensity(pixel_info->background_color) ? 0x01 : 0x00);
              foreground=(Intensity(pixel_info->background_color) <
                Intensity(pixel_info->foreground_color) ? 0x01 : 0x00);
              if (windows->magnify.depth > 1)
                Swap(background,foreground);
            }
          for (i=0; i < height; i+=magnify)
          {
            /*
              Propogate pixel magnify rows.
            */
            for (j=0; j < magnify; j++)
            {
              p=(unsigned char *) ximage->data+y*ximage->bytes_per_line+
                ((x*ximage->bits_per_pixel) >> 3);
              p_bit=(x*ximage->bits_per_pixel) & 0x07;
              q_bit=0;
              Byte=0;
              for (k=0; k < width; k+=magnify)
              {
                /*
                  Propogate pixel magnify columns.
                */
                for (l=0; l < magnify; l++)
                {
                  /*
                    Propogate each bit plane.
                  */
                  for (plane=0; plane < ximage->bits_per_pixel; plane++)
                  {
                    Byte<<=1;
                    if (*p & (0x80 >> (p_bit+plane)))
                      Byte|=foreground;
                    else
                      Byte|=background;
                    q_bit++;
                    if (q_bit == 8)
                      {
                        *q++=Byte;
                        q_bit=0;
                        Byte=0;
                      }
                  }
                }
                p_bit+=ximage->bits_per_pixel;
                if (p_bit == 8)
                  {
                    p++;
                    p_bit=0;
                  }
                if (q_bit != 0)
                  *q=Byte << (8-q_bit);
                q+=scanline_pad;
              }
            }
            y++;
          }
          break;
        }
      }
    }
  else
    switch (ximage->bits_per_pixel)
    {
      case 6:
      case 8:
      {
        /*
          Magnify 8 bit X image.
        */
        for (i=0; i < height; i+=magnify)
        {
          /*
            Propogate pixel magnify rows.
          */
          for (j=0; j < magnify; j++)
          {
            p=(unsigned char *) ximage->data+y*ximage->bytes_per_line+
              ((x*ximage->bits_per_pixel) >> 3);
            for (k=0; k < width; k+=magnify)
            {
              /*
                Propogate pixel magnify columns.
              */
              for (l=0; l < magnify; l++)
                *q++=(*p);
              p++;
            }
            q+=scanline_pad;
          }
          y++;
        }
        break;
      }
      default:
      {
        register unsigned int
          bytes_per_pixel,
          m;

        /*
          Magnify multi-Byte X image.
        */
        bytes_per_pixel=ximage->bits_per_pixel >> 3;
        for (i=0; i < height; i+=magnify)
        {
          /*
            Propogate pixel magnify rows.
          */
          for (j=0; j < magnify; j++)
          {
            p=(unsigned char *) ximage->data+y*ximage->bytes_per_line+
              ((x*ximage->bits_per_pixel) >> 3);
            for (k=0; k < width; k+=magnify)
            {
              /*
                Propogate pixel magnify columns.
              */
              for (l=0; l < magnify; l++)
                for (m=0; m < bytes_per_pixel; m++)
                  *q++=(*(p+m));
              p+=bytes_per_pixel;
            }
            q+=scanline_pad;
          }
          y++;
        }
        break;
      }
    }
  /*
    Copy X image to magnify pixmap.
  */
  x=windows->magnify.x-((width/magnify) >> 1);
  if (x < 0)
    x=(width >> 1)-windows->magnify.x*magnify;
  else
    if (x > (ximage->width-(width/magnify)))
      x=(ximage->width-windows->magnify.x)*magnify-(width >> 1);
    else
      x=0;
  y=windows->magnify.y-((height/magnify) >> 1);
  if (y < 0)
    y=(height >> 1)-windows->magnify.y*magnify;
  else
    if (y > (ximage->height-(height/magnify)))
      y=(ximage->height-windows->magnify.y)*magnify-(height >> 1);
    else
      y=0;
  if ((x != 0) || (y != 0))
    XFillRectangle(display,windows->magnify.pixmap,
      windows->magnify.annotate_context,0,0,width,height);
  XPutImage(display,windows->magnify.pixmap,windows->magnify.annotate_context,
    windows->magnify.ximage,0,0,x,y,width-x,height-y);
  if ((magnify > 1) && ((magnify <= (width >> 1)) &&
      (magnify <= (height >> 1))))
    {
      RectangleInfo
        highlight_info;

      /*
        Highlight center pixel.
      */
      highlight_info.x=windows->magnify.width >> 1;
      highlight_info.y=windows->magnify.height >> 1;
      highlight_info.width=magnify;
      highlight_info.height=magnify;
      XDrawRectangle(display,windows->magnify.pixmap,
        windows->magnify.highlight_context,highlight_info.x,highlight_info.y,
        highlight_info.width-1,highlight_info.height-1);
      if (magnify > 2)
        XDrawRectangle(display,windows->magnify.pixmap,
          windows->magnify.annotate_context,highlight_info.x+1,
          highlight_info.y+1,highlight_info.width-3,highlight_info.height-3);
    }
  /*
    Show center pixel color.
  */

#ifdef PURE_PIXEL_SUPPORT
  /* Kobus */
  {
      extern int num_image_columns;
      int x = windows->magnify.x;
      int y = windows->magnify.y;
      double r, g, b, s;
      unsigned char *pos;

      pos = pure_pixels + 3 * ((y * num_image_columns) + x);

      r = *pos;
      g = *(pos+1);
      b = *(pos+2);
      s = r + g + b;

      sprintf(text,"%d %d (%d %d %d)   [ %.3f   %.3f ]", x,y,
              *pos, *(pos+1), *(pos+2), r/s, g/s);
  }
  /* End Kobus */
#else
  color.pixel=
    XGetPixel(windows->image.ximage,windows->magnify.x,windows->magnify.y);
  XQueryColor(display,windows->image.map_info->colormap,&color);
  if (windows->magnify.depth > 12)
    (void) sprintf(text," %+d%+d  (%3u,%3u,%3u) ",
      windows->magnify.x,windows->magnify.y,XDownScale(color.red),
      XDownScale(color.green),XDownScale(color.blue));
  else
    (void) sprintf(text," %+d%+d  (%3u,%3u,%3u) %lu ",
      windows->magnify.x,windows->magnify.y,XDownScale(color.red),
      XDownScale(color.green),XDownScale(color.blue),color.pixel);
#endif

  height=windows->magnify.font_info->ascent+windows->magnify.font_info->descent;
  x=windows->magnify.font_info->max_bounds.width >> 1;
  y=windows->magnify.font_info->ascent+(height >> 2);
  XDrawImageString(display,windows->magnify.pixmap,
    windows->magnify.annotate_context,x,y,text,strlen(text));

  /*
  (void) sprintf(text," #%02x%02x%02x ",XDownScale(color.red),
    XDownScale(color.green),XDownScale(color.blue));

  y+=height;
  XDrawImageString(display,windows->magnify.pixmap,
    windows->magnify.annotate_context,x,y,text,strlen(text));
  */



  /*
    Refresh magnify window.
  */
  magnify_window=windows->magnify;
  magnify_window.x=0;
  magnify_window.y=0;
  XRefreshWindow(display,&magnify_window,(XEvent *) NULL);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   X M a k e P i x m a p                                                     %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Function XMakePixmap creates an X11 pixmap.
%
%  The format of the XMakePixmap routine is:
%
%      status=XMakePixmap(display,resource_info,window)
%
%  A description of each parameter follows:
%
%    o status: Function XMakePixmap returns True if the X pixmap is
%      successfully created.  False is returned is there is a memory shortage.
%
%    o resource_info: Specifies a pointer to a X11 XResourceInfo structure.
%
%    o display: Specifies a connection to an X server; returned from
%      XOpenDisplay.
%
%    o window: Specifies a pointer to a XWindowInfo structure.
%
%
*/
unsigned int XMakePixmap(Display *display, XResourceInfo *resource_info, XWindowInfo *window)
{
  unsigned int
    height,
    width;

  if (window->ximage == (XImage *) NULL)
    return(False);
  /*
    Display busy cursor.
  */
  XDefineCursor(display,window->id,window->busy_cursor);
  XFlush(display);
  /*
    Create pixmap.
  */
  if (window->pixmap != (Pixmap) NULL)
    {
      XFreePixmap(display,window->pixmap);
      window->pixmap=(Pixmap) NULL;
#ifdef HasSharedMemory
      if (window->shared_memory)
        if (window->segment_info[1].shmid >= 0)
          {
            XShmDetach(display,&window->segment_info[1]);
            XSync(display,False);
            shmdt(window->segment_info[1].shmaddr);
            window->segment_info[1].shmid=(-1);
          }
#endif
    }
  width=window->ximage->width;
  height=window->ximage->height;
#ifdef HasSharedMemory
  if (window->shared_memory)
    {
      window->segment_info[3].shmid=(int) shmget(IPC_PRIVATE,
        window->ximage->bytes_per_line*height,IPC_CREAT | 0777);
      if (window->segment_info[3].shmid >= 0)
        {
          xerror_alert=False;
          window->segment_info[3].shmaddr=(char *)
            shmat(window->segment_info[3].shmid,0,0);
          window->segment_info[3].readOnly=False;
          XShmAttach(display,&window->segment_info[3]);
          window->pixmap=XShmCreatePixmap(display,window->id,
            window->segment_info[3].shmaddr,&window->segment_info[3],
            width,height,window->depth);
          if (window->pixmap != (Pixmap) NULL)
            window->segment_info[1]=window->segment_info[3];
          else
            {
              shmdt(window->segment_info[3].shmaddr);
              XShmDetach(display,&window->segment_info[3]);
            }
          XSync(display,False);
          shmctl(window->segment_info[3].shmid,IPC_RMID,0);
          if (xerror_alert)
            {
              window->pixmap=(Pixmap) NULL;
              xerror_alert=False;
            }
        }
    }
#endif
  if (window->pixmap == (Pixmap) NULL)
    window->pixmap=XCreatePixmap(display,window->id,width,height,window->depth);
  if (window->pixmap == (Pixmap) NULL)
    {
      /*
        Unable to allocate pixmap.
      */
      XDefineCursor(display,window->id,window->cursor);
      return(False);
    }
  /*
    Copy X image to pixmap.
  */
#ifdef HasSharedMemory
  if (window->shared_memory)
    XShmPutImage(display,window->pixmap,window->annotate_context,window->ximage,
      0,0,0,0,width,height,True);
#endif
  if (!window->shared_memory)
    XPutImage(display,window->pixmap,window->annotate_context,window->ximage,
      0,0,0,0,width,height);
  if (resource_info->debug)
    {
      (void) fprintf(stderr,"Pixmap:\n");
      (void) fprintf(stderr,"  width, height: %ux%u\n",width,height);
    }
  /*
    Restore cursor.
  */
  XDefineCursor(display,window->id,window->cursor);
  return(True);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   X M a k e S t a n d a r d C o l o r m a p                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Function XMakeStandardColormap creates an X11 Standard Colormap.
%
%  The format of the XMakeStandardColormap routine is:
%
%      XMakeStandardColormap(display,visual_info,resource_info,image,
%        map_info,pixel_info)
%
%  A description of each parameter follows:
%
%    o display: Specifies a connection to an X server; returned from
%      XOpenDisplay.
%
%    o visual_info: Specifies a pointer to a X11 XVisualInfo structure;
%      returned from XGetVisualInfo.
%
%    o resource_info: Specifies a pointer to a X11 XResourceInfo structure.
%
%    o image: Specifies a pointer to a Image structure;  returned from
%      ReadImage.
%
%    o map_info: If a Standard Colormap type is specified, this structure is
%      initialized with info from the Standard Colormap.
%
%    o pixel_info: Specifies a pointer to a XPixelInfo structure.
%
%
*/
static int IntensityCompare(const void *x, const void *y)
{
  DiversityPacket
    *color_1,
    *color_2;

  color_1=(DiversityPacket *) x;
  color_2=(DiversityPacket *) y;
  return((int) Intensity(*color_2)-(int) Intensity(*color_1));
}

static int PopularityCompare(const void *x, const void *y)
{
  DiversityPacket
    *color_1,
    *color_2;

  color_1=(DiversityPacket *) x;
  color_2=(DiversityPacket *) y;
  return((int) color_2->count-(int) color_1->count);
}

void XMakeStandardColormap(Display *display, XVisualInfo *visual_info, XResourceInfo *resource_info, Image *image, XStandardColormap *map_info, XPixelInfo *pixel_info)
{
  Colormap
    colormap;

  int
    status;

  register int
    i;

  register XColor
    *gamma_map;

  unsigned int
    number_colors,
    retain_colors;

  unsigned short
    gray_value;

  XColor
    color,
    *colors,
    *p;

  if (resource_info->map_type != (char *) NULL)
    {
      /*
        Standard Colormap is already defined (i.e. xstdcmap).
      */
      XGetPixelInfo(display,visual_info,map_info,resource_info,image,
        pixel_info);
      number_colors=map_info->base_pixel+
        (map_info->red_max+1)*(map_info->green_max+1)*(map_info->blue_max+1);
      if ((map_info->red_max*map_info->green_max*map_info->blue_max) != 0)
        if (resource_info->dither && !resource_info->color_recovery &&
            (number_colors < MaxColormapSize) && !image->matte)
          {
            Image
              *map_image;

            register RunlengthPacket
              *p;

            /*
              Improve image appearance with error diffusion.
            */
            map_image=AllocateImage((ImageInfo *) NULL);
            if (map_image == (Image *) NULL)
              Error("Unable to dither image","Memory allocation failed");
            map_image->columns=number_colors;
            map_image->rows=1;
            map_image->packets=map_image->columns*map_image->rows;
            map_image->pixels=(RunlengthPacket *)
              malloc(map_image->packets*sizeof(RunlengthPacket));
            if (map_image->pixels == (RunlengthPacket *) NULL)
              Error("Unable to dither image","Memory allocation failed");
            /*
              Initialize colormap image.
            */
            p=map_image->pixels;
            for (i=0; i < number_colors; i++)
            {
              p->red=0;
              if (map_info->red_max != 0)
                p->red=(Quantum)
                  (((i/map_info->red_mult)*MaxRGB)/map_info->red_max);
              p->green=0;
              if (map_info->green_max != 0)
                p->green=(Quantum) ((((i/map_info->green_mult) %
                  (map_info->green_max+1))*MaxRGB)/map_info->green_max);
              p->blue=0;
              if (map_info->blue_max != 0)
                p->blue=(Quantum)
                  (((i % map_info->green_mult)*MaxRGB)/map_info->blue_max);
              p->index=0;
              p->length=0;
              p++;
            }
            MapImage(image,map_image,True);
            XGetPixelInfo(display,visual_info,map_info,resource_info,image,
              pixel_info);
            image->class=DirectClass;
            DestroyImage(map_image);
          }
      if (resource_info->debug)
        {
          (void) fprintf(stderr,"Standard Colormap:\n");
          (void) fprintf(stderr,"  colormap id: 0x%lx\n",map_info->colormap);
          (void) fprintf(stderr,"  red, green, blue max: %lu %lu %lu\n",
            map_info->red_max,map_info->green_max,map_info->blue_max);
          (void) fprintf(stderr,"  red, green, blue mult: %lu %lu %lu\n",
            map_info->red_mult,map_info->green_mult,map_info->blue_mult);
        }
      return;
    }
  if ((visual_info->class != DirectColor) && (visual_info->class != TrueColor))
    if ((image->class == DirectClass) ||
        (image->colors > visual_info->colormap_size))
      {
        /*
          Image has more colors than the visual supports.
        */
        if (image->matte)
          {
            register RunlengthPacket
              *p;

            /*
              Fake matte information.
            */
            Warning("This visual does not support an image matte",
              XVisualClassName(visual_info->class));
            p=image->pixels;
            for (i=0; i < image->packets; i++)
            {
              if (p->index == Transparent)
                {
                  p->red=XDownScale(pixel_info->background_color.red);
                  p->green=XDownScale(pixel_info->background_color.green);
                  p->blue=XDownScale(pixel_info->background_color.blue);
                }
              p++;
            }
          }
        QuantizeImage(image,(unsigned int) visual_info->colormap_size,
          resource_info->tree_depth,resource_info->dither,
          resource_info->colorspace);
        image->class=DirectClass;  /* promote to DirectClass */
      }
  /*
    Free previous and create new colormap.
  */
  XFreeStandardColormap(display,visual_info,map_info,pixel_info);
  colormap=XDefaultColormap(display,visual_info->screen);
  if (visual_info->visual != XDefaultVisual(display,visual_info->screen))
    colormap=XCreateColormap(display,XRootWindow(display,visual_info->screen),
      visual_info->visual,visual_info->class == DirectColor ?
      AllocAll : AllocNone);
  if (colormap == (Colormap) NULL)
    Error("Unable to create colormap",(char *) NULL);
  /*
    Initialize the map and pixel info structures.
  */
  XGetMapInfo(visual_info,colormap,map_info);
  XGetPixelInfo(display,visual_info,map_info,resource_info,image,pixel_info);
  gamma_map=pixel_info->gamma_map;
  /*
    Allocating colors in server colormap is based on visual class.
  */
  switch (visual_info->class)
  {
    case StaticGray:
    case StaticColor:
    {
      /*
        Define Standard Colormap for StaticGray or StaticColor visual.
      */
      number_colors=image->colors;
      colors=(XColor *) malloc(visual_info->colormap_size*sizeof(XColor));
      if (colors == (XColor *) NULL)
        Error("Unable to create colormap","Memory allocation failed");
      p=colors;
      color.flags=DoRed | DoGreen | DoBlue;
      if (visual_info->class == StaticColor)
        for (i=0; i < image->colors; i++)
        {
          color.red=XUpScale(gamma_map[image->colormap[i].red].red);
          color.green=XUpScale(gamma_map[image->colormap[i].green].green);
          color.blue=XUpScale(gamma_map[image->colormap[i].blue].blue);
          status=XAllocColor(display,colormap,&color);
          if (status == 0)
            {
              colormap=XCopyColormapAndFree(display,colormap);
              XAllocColor(display,colormap,&color);
            }
          pixel_info->pixels[i]=color.pixel;
          *p++=color;
        }
      else
        for (i=0; i < image->colors; i++)
        {
          gray_value=Intensity(gamma_map[Intensity(image->colormap[i])]);
          color.red=XUpScale(gray_value);
          color.green=XUpScale(gray_value);
          color.blue=XUpScale(gray_value);
          status=XAllocColor(display,colormap,&color);
          if (status == 0)
            {
              colormap=XCopyColormapAndFree(display,colormap);
              XAllocColor(display,colormap,&color);
            }
          pixel_info->pixels[i]=color.pixel;
          *p++=color;
        }
      break;
    }
    case GrayScale:
    case PseudoColor:
    {
      unsigned int
        colormap_type;

      /*
        Define Standard Colormap for GrayScale or PseudoColor visual.
      */
      number_colors=image->colors;
      colors=(XColor *) malloc(visual_info->colormap_size*sizeof(XColor));
      if (colors == (XColor *) NULL)
        Error("Unable to create colormap","Memory allocation failed");
      /*
        Preallocate our GUI colors.
      */
      (void) XAllocColor(display,colormap,&pixel_info->foreground_color);
      (void) XAllocColor(display,colormap,&pixel_info->background_color);
      (void) XAllocColor(display,colormap,&pixel_info->border_color);
      (void) XAllocColor(display,colormap,&pixel_info->matte_color);
      (void) XAllocColor(display,colormap,&pixel_info->highlight_color);
      (void) XAllocColor(display,colormap,&pixel_info->shadow_color);
      (void) XAllocColor(display,colormap,&pixel_info->depth_color);
      (void) XAllocColor(display,colormap,&pixel_info->trough_color);
      for (i=0; i < MaxNumberPens; i++)
        (void) XAllocColor(display,colormap,&pixel_info->pen_colors[i]);
      /*
        Determine if image colors will "fit" into X server colormap.
      */
      colormap_type=resource_info->colormap;
      status=XAllocColorCells(display,colormap,False,(unsigned long *) NULL,0,
        pixel_info->pixels,image->colors);
      if (status != 0)
        colormap_type=PrivateColormap;
      if (colormap_type == SharedColormap)
        {
          DiversityPacket
            *diversity;

          register RunlengthPacket
            *q;

          unsigned short
            index;

          /*
            Define Standard colormap for shared GrayScale or PseudoColor visual:
          */
          diversity=(DiversityPacket *)
            malloc(image->colors*sizeof(DiversityPacket));
          if (diversity == (DiversityPacket *) NULL)
            Error("Unable to create colormap","Memory allocation failed");
          for (i=0; i < image->colors; i++)
          {
            diversity[i].red=image->colormap[i].red;
            diversity[i].green=image->colormap[i].green;
            diversity[i].blue=image->colormap[i].blue;
            diversity[i].index=(unsigned short) i;
            diversity[i].count=0;
          }
          q=image->pixels;
          for (i=0; i < image->packets; i++)
          {
            diversity[q->index].count+=(q->length+1);
            q++;
          }
          /*
            Sort colors by decreasing intensity.
          */
          qsort((void *) diversity,image->colors,sizeof(DiversityPacket),
            (int (*) _Declare((const void *, const void *))) IntensityCompare);
          for (i=0; i < image->colors; i+=Max(image->colors >> 4,2))
            diversity[i].count<<=4;  /* increase this colors popularity */
          diversity[image->colors-1].count<<=4;
          qsort((void *) diversity,image->colors,sizeof(DiversityPacket),
            (int (*) _Declare((const void *, const void *))) PopularityCompare);
          /*
            Allocate colors.
          */
          p=colors;
          color.flags=DoRed | DoGreen | DoBlue;
          if (visual_info->class == PseudoColor)
            for (i=0; i < image->colors; i++)
            {
              index=diversity[i].index;
              color.red=XUpScale(gamma_map[image->colormap[index].red].red);
              color.green=
                XUpScale(gamma_map[image->colormap[index].green].green);
              color.blue=XUpScale(gamma_map[image->colormap[index].blue].blue);
              status=XAllocColor(display,colormap,&color);
              if (status == 0)
                break;
              pixel_info->pixels[index]=color.pixel;
              *p++=color;
            }
          else
            for (i=0; i < image->colors; i++)
            {
              index=diversity[i].index;
              gray_value=
                Intensity(gamma_map[Intensity(image->colormap[index])]);
              color.red=XUpScale(gray_value);
              color.green=XUpScale(gray_value);
              color.blue=XUpScale(gray_value);
              status=XAllocColor(display,colormap,&color);
              if (status == 0)
                break;
              pixel_info->pixels[index]=color.pixel;
              *p++=color;
            }
          if (i < image->colors)
            {
              register int
                j;

              XColor
                *server_colors;

              /*
                Read X server colormap.
              */
              server_colors=(XColor *)
                malloc(visual_info->colormap_size*sizeof(XColor));
              if (server_colors == (XColor *) NULL)
                Error("Unable to create colormap","Memory allocation failed");
              for (j=0; j < visual_info->colormap_size; j++)
                server_colors[j].pixel=(unsigned long) j;
              XQueryColors(display,colormap,server_colors,
                (int) Min(visual_info->colormap_size,256));
              /*
                Select remaining colors from X server colormap.
              */
              if (visual_info->class == PseudoColor)
                for (; i < image->colors; i++)
                {
                  index=diversity[i].index;
                  color.red=XUpScale(gamma_map[image->colormap[index].red].red);
                  color.green=
                    XUpScale(gamma_map[image->colormap[index].green].green);
                  color.blue=
                    XUpScale(gamma_map[image->colormap[index].blue].blue);
                  XBestPixel(display,colormap,server_colors,
                    (unsigned int) visual_info->colormap_size,&color);
                  pixel_info->pixels[index]=color.pixel;
                  *p++=color;
                }
              else
                for (; i < image->colors; i++)
                {
                  index=diversity[i].index;
                  gray_value=
                    Intensity(gamma_map[Intensity(image->colormap[index])]);
                  color.red=XUpScale(gray_value);
                  color.green=XUpScale(gray_value);
                  color.blue=XUpScale(gray_value);
                  XBestPixel(display,colormap,server_colors,
                    (unsigned int) visual_info->colormap_size,&color);
                  pixel_info->pixels[index]=color.pixel;
                  *p++=color;
                }
              if (image->colors < visual_info->colormap_size)
                {
                  /*
                    Fill up colors array-- more choices for pen colors.
                  */
                  retain_colors=
                    Min(visual_info->colormap_size-image->colors,256);
                  for (i=0; i < retain_colors; i++)
                    *p++=server_colors[i];
                  number_colors+=retain_colors;
                }
              free((char *) server_colors);
            }
          free((char *) diversity);
          break;
        }
      /*
        Define Standard colormap for private GrayScale or PseudoColor visual.
      */
      if (status == 0)
        {
          /*
            Not enough colormap entries in the colormap-- Create a new colormap.
          */
          colormap=XCreateColormap(display,
            XRootWindow(display,visual_info->screen),visual_info->visual,
            AllocNone);
          if (colormap == (Colormap) NULL)
            Error("Unable to create colormap",(char *) NULL);
          map_info->colormap=colormap;
          if (image->colors < visual_info->colormap_size)
            {
              /*
                Retain colors from the default colormap to help lessens the
                effects of colormap flashing.
              */
              retain_colors=Min(visual_info->colormap_size-image->colors,256);
              p=colors+image->colors;
              for (i=0; i < retain_colors; i++)
              {
                p->pixel=(unsigned long) i;
                p++;
              }
              XQueryColors(display,
                XDefaultColormap(display,visual_info->screen),
                colors+image->colors,(int) retain_colors);
              /*
                Transfer colors from default to private colormap.
              */
              XAllocColorCells(display,colormap,False,(unsigned long *) NULL,0,
                pixel_info->pixels,retain_colors);
              p=colors+image->colors;
              for (i=0; i < retain_colors; i++)
              {
                p->pixel=pixel_info->pixels[i];
                p++;
              }
              XStoreColors(display,colormap,colors+image->colors,retain_colors);
              number_colors+=retain_colors;
            }
          XAllocColorCells(display,colormap,False,(unsigned long *) NULL,0,
            pixel_info->pixels,image->colors);
        }
      /*
        Store the image colormap.
      */
      p=colors;
      color.flags=DoRed | DoGreen | DoBlue;
      if (visual_info->class == PseudoColor)
        for (i=0; i < image->colors; i++)
        {
          color.red=XUpScale(gamma_map[image->colormap[i].red].red);
          color.green=XUpScale(gamma_map[image->colormap[i].green].green);
          color.blue=XUpScale(gamma_map[image->colormap[i].blue].blue);
          color.pixel=pixel_info->pixels[i];
          *p++=color;
        }
      else
        for (i=0; i < image->colors; i++)
        {
          gray_value=Intensity(gamma_map[Intensity(image->colormap[i])]);
          color.red=XUpScale(gray_value);
          color.green=XUpScale(gray_value);
          color.blue=XUpScale(gray_value);
          color.pixel=pixel_info->pixels[i];
          *p++=color;
        }
      XStoreColors(display,colormap,colors,image->colors);
      break;
    }
    case TrueColor:
    case DirectColor:
    default:
    {
      unsigned int
        linear_colormap;

      /*
        Define Standard Colormap for TrueColor or DirectColor visual.
      */
      number_colors=(unsigned int) ((map_info->red_max*map_info->red_mult)+
        (map_info->green_max*map_info->green_mult)+
        (map_info->blue_max*map_info->blue_mult)+1);
      linear_colormap=(number_colors > 4096) ||
        (((map_info->red_max+1) == visual_info->colormap_size) &&
         ((map_info->green_max+1) == visual_info->colormap_size) &&
         ((map_info->blue_max+1) == visual_info->colormap_size));
      if (linear_colormap)
        number_colors=visual_info->colormap_size;
      /*
        Allocate color array.
      */
      colors=(XColor *) malloc(number_colors*sizeof(XColor));
      if (colors == (XColor *) NULL)
        Error("Unable to create colormap","Memory allocation failed");
      /*
        Initialize linear color ramp.
      */
      p=colors;
      color.flags=DoRed | DoGreen | DoBlue;
      if (linear_colormap)
        for (i=0; i < number_colors; i++)
        {
          color.blue=(unsigned short) 0;
          if (map_info->blue_max != 0)
            color.blue=(unsigned short)
              (((i % map_info->green_mult)*65535L)/map_info->blue_max);
          color.green=color.blue;
          color.red=color.blue;
          color.pixel=XStandardPixel(map_info,color,16);
          *p++=color;
        }
      else
        for (i=0; i < number_colors; i++)
        {
          color.red=(unsigned short) 0;
          if (map_info->red_max != 0)
            color.red=(unsigned short)
              (((i/map_info->red_mult)*65535L)/map_info->red_max);
          color.green=(unsigned short) 0;
          if (map_info->green_max != 0)
            color.green=(unsigned short) ((((i/map_info->green_mult) %
              (map_info->green_max+1))*65535L)/map_info->green_max);
          color.blue=(unsigned short) 0;
          if (map_info->blue_max != 0)
            color.blue=(unsigned short)
              (((i % map_info->green_mult)*65535L)/map_info->blue_max);
          color.pixel=XStandardPixel(map_info,color,16);
          *p++=color;
        }
      if ((visual_info->class == DirectColor) &&
          (colormap != XDefaultColormap(display,visual_info->screen)))
        XStoreColors(display,colormap,colors,number_colors);
      else
        for (i=0; i < number_colors; i++)
          XAllocColor(display,colormap,&colors[i]);
      break;
    }
  }
  if ((visual_info->class != DirectColor) && (visual_info->class != TrueColor))
    {
      /*
        Set foreground, background, border, etc. pixels.
      */
      XBestPixel(display,colormap,colors,number_colors,
        &pixel_info->foreground_color);
      XBestPixel(display,colormap,colors,number_colors,
        &pixel_info->background_color);
      if (pixel_info->background_color.pixel ==
          pixel_info->foreground_color.pixel)
        {
          /*
            Foreground and background colors must differ.
          */
          pixel_info->background_color.red=(~pixel_info->foreground_color.red);
          pixel_info->background_color.green=
            (~pixel_info->foreground_color.green);
          pixel_info->background_color.blue=
            (~pixel_info->foreground_color.blue);
          XBestPixel(display,colormap,colors,number_colors,
            &pixel_info->background_color);
        }
      XBestPixel(display,colormap,colors,number_colors,
        &pixel_info->border_color);
      XBestPixel(display,colormap,colors,number_colors,
        &pixel_info->matte_color);
      XBestPixel(display,colormap,colors,number_colors,
        &pixel_info->highlight_color);
      XBestPixel(display,colormap,colors,number_colors,
        &pixel_info->shadow_color);
      XBestPixel(display,colormap,colors,number_colors,
        &pixel_info->depth_color);
      XBestPixel(display,colormap,colors,number_colors,
        &pixel_info->trough_color);
      for (i=0; i < MaxNumberPens; i++)
      {
        XBestPixel(display,colormap,colors,number_colors,
          &pixel_info->pen_colors[i]);
        pixel_info->pixels[image->colors+i]=pixel_info->pen_colors[i].pixel;
      }
      pixel_info->colors=image->colors+MaxNumberPens;
    }
  free((char *) colors);
  if (resource_info->debug)
    {
      (void) fprintf(stderr,"Standard Colormap:\n");
      (void) fprintf(stderr,"  colormap id: 0x%lx\n",map_info->colormap);
      (void) fprintf(stderr,"  red, green, blue max: %lu %lu %lu\n",
        map_info->red_max,map_info->green_max,map_info->blue_max);
      (void) fprintf(stderr,"  red, green, blue mult: %lu %lu %lu\n",
        map_info->red_mult,map_info->green_mult,map_info->blue_mult);
    }
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   X M a k e W i n d o w                                                     %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Function XMakeWindow creates an X11 window.
%
%  The format of the XMakeWindow routine is:
%
%      XMakeWindow(display,parent,argv,argc,class_hint,manager_hints,
%        window_info)
%
%  A description of each parameter follows:
%
%    o display: Specifies a connection to an X server; returned from
%      XOpenDisplay.
%
%    o parent: Specifies the parent window_info.
%
%    o argv: Specifies the application's argument list.
%
%    o argc: Specifies the number of arguments.
%
%    o class_hint: Specifies a pointer to a X11 XClassHint structure.
%
%    o manager_hints: Specifies a pointer to a X11 XWMHints structure.
%
%    o window_info: Specifies a pointer to a X11 XWindowInfo structure.
%
%
*/
void XMakeWindow(Display *display, Window parent, char **argv, int argc, XClassHint *class_hint, XWMHints *manager_hints, XWindowInfo *window_info)
{
#define MinWindowSize  64

  Atom
    atom_list[2];

  int
    gravity,
    status;

  static XTextProperty
    icon_name,
    window_name;

  XSizeHints
    *size_hints;

  /*
    Set window_info hints.
  */
  size_hints=XAllocSizeHints();
  if (size_hints == (XSizeHints *) NULL)
    Error("Unable to make X window","Memory allocation failed");
  size_hints->flags=window_info->flags;
  size_hints->x=window_info->x;
  size_hints->y=window_info->y;
  size_hints->width=window_info->width;
  size_hints->height=window_info->height;
  if (window_info->immutable)
    {
      /*
        Window size cannot be changed.
      */
      size_hints->min_width=size_hints->width;
      size_hints->min_height=size_hints->height;
      size_hints->max_width=size_hints->width;
      size_hints->max_height=size_hints->height;
      size_hints->flags|=PMinSize;
      size_hints->flags|=PMaxSize;
    }
  else
    {
      /*
        Window size can be changed.
      */
      size_hints->min_width=window_info->min_width;
      size_hints->min_height=window_info->min_height;
      size_hints->flags|=PResizeInc;
      size_hints->width_inc=window_info->width_inc;
      size_hints->height_inc=window_info->height_inc;
#ifndef PRE_R4_ICCCM
      size_hints->flags|=PBaseSize;
      size_hints->base_width=size_hints->width_inc;
      size_hints->base_height=size_hints->height_inc;
#endif
    }
  gravity=NorthWestGravity;
  if (window_info->geometry != (char *) NULL)
    {
      char
        default_geometry[MaxTextLength],
        geometry[MaxTextLength];

      int
        flags;

      register char
        *p;

      /*
        User specified geometry.
      */
      (void) sprintf(default_geometry,"%dx%d",size_hints->width,
        size_hints->height);
      (void) strcpy(geometry,window_info->geometry);
      p=geometry;
      while ((int) strlen(p) > 0)
      {
        if (!isspace(*p) && (*p != '%'))
          p++;
        else
          (void) strcpy(p,p+1);
      }
      flags=XWMGeometry(display,window_info->screen,geometry,default_geometry,
        window_info->border_width,size_hints,&size_hints->x,&size_hints->y,
        &size_hints->width,&size_hints->height,&gravity);
      if ((flags & WidthValue) && (flags & HeightValue))
        size_hints->flags|=USSize;
      if ((flags & XValue) && (flags & YValue))
        {
          size_hints->flags|=USPosition;
          window_info->x=size_hints->x;
          window_info->y=size_hints->y;
        }
    }
#ifndef PRE_R4_ICCCM
  size_hints->win_gravity=gravity;
  size_hints->flags|=PWinGravity;
#endif
  if (window_info->id == (Window) NULL)
    window_info->id=XCreateWindow(display,parent,window_info->x,window_info->y,
      window_info->width,window_info->height,window_info->border_width,
      window_info->depth,InputOutput,window_info->visual,window_info->mask,
      &window_info->attributes);
  else
    {
      unsigned int
        mask;

      XEvent
        sans_event;

      XWindowChanges
        window_changes;

      /*
        Window already exists;  change relevant attributes.
      */
      XChangeWindowAttributes(display,window_info->id,window_info->mask,
        &window_info->attributes);
      mask=ConfigureNotify;
      while (XCheckTypedWindowEvent(display,window_info->id,mask,&sans_event));
      window_changes.x=window_info->x;
      window_changes.y=window_info->y;
      window_changes.width=window_info->width;
      window_changes.height=window_info->height;
      mask=CWWidth | CWHeight;
      if (window_info->flags & USPosition)
        mask|=CWX | CWY;
      XReconfigureWMWindow(display,window_info->id,window_info->screen,mask,
        &window_changes);
    }
  if (window_info->id == (Window) NULL)
    Error("Unable to create window",window_info->name);
  status=XStringListToTextProperty(&window_info->name,1,&window_name);
  if (status == 0)
    Error("Unable to create text property",window_info->name);
  status=XStringListToTextProperty(&window_info->icon_name,1,&icon_name);
  if (status == 0)
    Error("Unable to create text property",window_info->icon_name);
  if (window_info->icon_geometry != (char *) NULL)
    {
      int
        flags,
        gravity,
        height,
        width;

      /*
        User specified icon geometry.
      */
      size_hints->flags|=USPosition;
      flags=XWMGeometry(display,window_info->screen,window_info->icon_geometry,
        (char *) NULL,0,size_hints,&manager_hints->icon_x,
        &manager_hints->icon_y,&width,&height,&gravity);
      if ((flags & XValue) && (flags & YValue))
        manager_hints->flags|=IconPositionHint;
    }
  XSetWMProperties(display,window_info->id,&window_name,&icon_name,argv,argc,
    size_hints,manager_hints,class_hint);
  if (window_name.nitems != 0)
    XFree((void *) window_name.value);
  if (icon_name.nitems != 0)
    XFree((void *) icon_name.value);
  atom_list[0]=XInternAtom(display,"WM_DELETE_WINDOW",False);
  atom_list[1]=XInternAtom(display,"WM_TAKE_FOCUS",False);
  XSetWMProtocols(display,window_info->id,atom_list,2);
  XFree((void *) size_hints);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   X M o n t a g e I m a g e                                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Function XMontageImage creates a composite image by combining several
%  separate images.
%
%  The format of the MontageImage routine is:
%
%      XMontageImage(images,resource_info,montage_info,filename)
%
%  A description of each parameter follows:
%
%    o image: Specifies a pointer to an array of Image structures.
%
%    o resource_info: Specifies a pointer to a X11 XResourceInfo structure.
%
%    o montage_info: Specifies a pointer to a XMontageInfo structure.
%
%    o filename: Specifies the name of the montage image.
%
%
*/

static int NumberLines(char *label)
{
  int
    number_lines;

  if (label == (char *) NULL)
    return(0);
  for (number_lines=1; *label != '\0'; label++)
    if (*label == '\n')
      number_lines++;
  return(number_lines);
}

static int SceneCompare(const void *x, const void *y)
{
  Image
    **image_1,
    **image_2;

  image_1=(Image **) x;
  image_2=(Image **) y;
  return((int) (*image_1)->scene-(int) (*image_2)->scene);
}

Image *XMontageImage(Image **images, XResourceInfo *resource_info, XMontageInfo *montage_info, char *filename)
{
#define MontageImageText  "  Creating visual image directory...  "

  AnnotateInfo
    annotate_info;

  char
    geometry[MaxTextLength];

  ColorPacket
    background_color,
    border_color,
    highlight_color,
    matte_color,
    shadow_color;

  Display
    *display;

  Image
    *image,
    *montage_image;

  int
    flags,
    x,
    x_offset,
    y,
    y_offset;

  MonitorHandler
    handler;

  register int
    i;

  register RunlengthPacket
    *p;

  RectangleInfo
    bounding_box,
    tile_info;

  unsigned int
    border_width,
    bevel_width,
    concatenate,
    count,
    font_height,
    height,
    max_height,
    number_images,
    number_lines,
    number_tiles,
    tile,
    tiles_per_column,
    tiles_per_row,
    title_offset,
    width;

  XColor
    color;

  /*
    Sort images by increasing tile number.
  */
  number_tiles=montage_info->number_tiles;
  number_lines=0;
  count=0;
  for (tile=0; tile < number_tiles; tile++)
  {
    count+=images[tile]->scene;
    if (NumberLines(images[tile]->label) > number_lines)
      number_lines=NumberLines(images[tile]->label);
  }
  if (count != 0)
    qsort((void *) images,number_tiles,sizeof(Image *),
      (int (*) _Declare((const void *, const void *))) SceneCompare);
  /*
    Determine tiles per row and column.
  */
  tiles_per_row=1;
  tiles_per_column=1;
  while ((tiles_per_row*tiles_per_column) < number_tiles)
  {
    tiles_per_row++;
    tiles_per_column++;
  }
  if (montage_info->tile != (char *) NULL)
    {
      tiles_per_column=montage_info->number_tiles;
      XParseGeometry(montage_info->tile,&x,&y,&tiles_per_row,&tiles_per_column);
      /*
      if (tiles_per_row > number_tiles)
        tiles_per_row=number_tiles;*/
    }
  /*
    Determine tile sizes.
  */
  border_width=resource_info->border_width;
  bevel_width=0;
  if (montage_info->frame)
    {
      bevel_width=(border_width >> 2)+1;
      border_width+=bevel_width << 1;
    }
  tile_info.x=resource_info->border_width;
  tile_info.y=resource_info->border_width;
  flags=NoValue;
  if (resource_info->image_geometry != (char *) NULL)
    {
      flags=XParseGeometry(resource_info->image_geometry,&tile_info.x,
        &tile_info.y,&tile_info.width,&tile_info.height);
      if (tile_info.x < 0)
        tile_info.x=0;
      if (tile_info.y < 0)
        tile_info.y=0;
    }
  concatenate=!((flags & WidthValue) || (flags & HeightValue));
  tile_info.width=images[0]->columns;
  tile_info.height=images[0]->rows;
  for (tile=1; tile < montage_info->number_tiles; tile++)
  {
    if (images[tile]->columns > tile_info.width)
      tile_info.width=images[tile]->columns;
    if (images[tile]->rows > tile_info.height)
      tile_info.height=images[tile]->rows;
  }
  /*
    Initialize tile colors.
  */
  (void) XQueryColorDatabase(resource_info->background_color,&color);
  background_color.red=XDownScale(color.red);
  background_color.green=XDownScale(color.green);
  background_color.blue=XDownScale(color.blue);
  background_color.index=0;
  (void) XQueryColorDatabase(resource_info->border_color,&color);
  border_color.red=XDownScale(color.red);
  border_color.green=XDownScale(color.green);
  border_color.blue=XDownScale(color.blue);
  border_color.index=0;
  (void) XQueryColorDatabase(resource_info->matte_color,&color);
  matte_color.red=XDownScale(color.red);
  matte_color.green=XDownScale(color.green);
  matte_color.blue=XDownScale(color.blue);
  matte_color.index=0;
  highlight_color.red=(XDownScale(color.red)*HighlightModulate+
    (unsigned int) (MaxRGB-HighlightModulate)*MaxRGB)/MaxRGB;
  highlight_color.green=(XDownScale(color.green)*HighlightModulate+
    (unsigned int) (MaxRGB-HighlightModulate)*MaxRGB)/MaxRGB;
  highlight_color.blue=(XDownScale(color.blue)*HighlightModulate+
    (unsigned int) (MaxRGB-HighlightModulate)*MaxRGB)/MaxRGB;
  highlight_color.index=0;
  (void) XQueryColorDatabase(resource_info->matte_color,&color);
  shadow_color.red=(unsigned int)
    (XDownScale(color.red)*ShadowModulate)/MaxRGB;
  shadow_color.green=(unsigned int)
    (XDownScale(color.green)*ShadowModulate)/MaxRGB;
  shadow_color.blue=(unsigned int)
    (XDownScale(color.blue)*ShadowModulate)/MaxRGB;
  shadow_color.index=0;
  /*
    Initialize annotate info.
  */
  GetAnnotateInfo(&annotate_info);
  annotate_info.server_name=resource_info->server_name;
  annotate_info.font=resource_info->font;
  annotate_info.pen=resource_info->foreground_color;
  annotate_info.geometry=geometry;
  annotate_info.center=True;
  font_height=DefaultPointSize;
  title_offset=0;
  if (resource_info->title != (char *) NULL)
    title_offset=
      ((font_height*NumberLines(resource_info->title)) << 1)+(tile_info.y << 1);
  display=XOpenDisplay(resource_info->server_name);
  if (display != (Display *) NULL)
    {
      XFontStruct
        *font_info;

      /*
        Initialize font info.
      */
      font_info=XBestFont(display,resource_info,False);
      if (font_info == (XFontStruct *) NULL)
        {
          Warning("Unable to montage images","Memory allocation failed");
          return((Image *) NULL);
        }
      font_height=font_info->ascent+font_info->descent;
      XFreeFont(display,font_info);
      XCloseDisplay(display);
    }
  /*
    Allocate image structure.
  */
  montage_image=AllocateImage((ImageInfo *) NULL);
  if (montage_image == (Image *) NULL)
    {
      Warning("Unable to montage images","Memory allocation failed");
      return((Image *) NULL);
    }
  montage_image->scene=1;
  number_images=
    (montage_info->number_tiles-1)/(tiles_per_row*tiles_per_column)+1;
  for (i=0; i < number_images; i++)
  {
    /*
      Determine bounding box.
    */
    number_tiles=Min(montage_info->number_tiles,tiles_per_row*tiles_per_column);
    x_offset=tile_info.x;
    y_offset=title_offset+tile_info.y;
    max_height=0;
    bounding_box.width=0;
    bounding_box.height=0;
    for (tile=0; tile < number_tiles; tile++)
    {
      width=concatenate ? images[tile]->columns : tile_info.width;
      x_offset+=width+(tile_info.x+border_width)*2;
      if (x_offset > bounding_box.width)
        bounding_box.width=x_offset;
      if (images[tile]->rows > max_height)
        max_height=images[tile]->rows;
      if (((tile+1) == number_tiles) || (((tile+1) % tiles_per_row) == 0))
        {
          x_offset=tile_info.x;
          height=concatenate ? max_height : tile_info.height;
          y_offset+=
            height+(tile_info.y+border_width)*2+(font_height+4)*number_lines;
          if (y_offset > bounding_box.height)
            bounding_box.height=y_offset;
          max_height=0;
        }
    }
    /*
      Initialize Image structure.
    */
    (void) strcpy(montage_image->filename,filename);
    montage_image->columns=bounding_box.width;
    montage_image->rows=bounding_box.height;
    montage_image->packets=montage_image->columns*montage_image->rows;
    montage_image->pixels=(RunlengthPacket *)
      malloc((unsigned int) montage_image->packets*sizeof(RunlengthPacket));
    if (montage_image->pixels == (RunlengthPacket *) NULL)
      {
        Warning("Unable to montage images","Memory allocation failed");
        DestroyImages(montage_image);
        return((Image *) NULL);
      }
    if (!concatenate)
      {
        /*
          Set montage geometry.
        */
        montage_image->montage=(char *) malloc(MaxTextLength*sizeof(char));
        count=1;
        for (tile=0; tile < number_tiles; tile++)
          count+=strlen(images[tile]->filename)+1;
        montage_image->directory=(char *) malloc(count*sizeof(char));
        if ((montage_image->montage == (char *) NULL) ||
            (montage_image->directory == (char *) NULL))
          {
            Warning("Unable to montage images","Memory allocation failed");
            DestroyImages(montage_image);
            return((Image *) NULL);
          }
        x_offset=0;
        y_offset=title_offset;
        (void) sprintf(montage_image->montage,"%dx%d%+d%+d",
          (int) (tile_info.width+(tile_info.x+border_width)*2),
          (int) (tile_info.height+(tile_info.y+border_width)*2+(font_height+4)*
          number_lines),x_offset,y_offset);
        *montage_image->directory='\0';
        for (tile=0; tile < number_tiles; tile++)
        {
          (void) strcat(montage_image->directory,images[tile]->filename);
          (void) strcat(montage_image->directory,"\n");
        }
      }
    /*
      Initialize montage image to background color.
    */
    p=montage_image->pixels;
    for (x=0; x < montage_image->packets; x++)
    {
      p->red=background_color.red;
      p->green=background_color.green;
      p->blue=background_color.blue;
      p->index=0;
      p->length=0;
      p++;
    }
    if (montage_info->texture != (char *) NULL)
      TextureImage(montage_image,montage_info->texture);
    if (resource_info->title != (char *) NULL)
      {
        /*
          Annotate composite image with title.
        */
        (void) sprintf(annotate_info.geometry,"%ux%u%+d%+d",
          montage_image->columns,font_height << 1,0,tile_info.y+4);
        annotate_info.text=resource_info->title;
        AnnotateImage(montage_image,&annotate_info);
      }
    /*
      Copy tile images to the composite image.
    */
    x_offset=tile_info.x;
    y_offset=title_offset+tile_info.y;
    max_height=0;
    for (tile=0; tile < number_tiles; tile++)
    {
      /*
        Copy this tile to the composite image.
      */
      handler=SetMonitorHandler((MonitorHandler) NULL);
      image=images[tile];
      width=concatenate ? image->columns : tile_info.width;
      if (image->rows > max_height)
        max_height=image->rows;
      height=concatenate ? max_height : tile_info.height;
      if (border_width != 0)
        {
          Image
            *bordered_image;

          RectangleInfo
            border_info;

          /*
            Put a border around the image.
          */
          border_info.width=border_width;
          border_info.height=border_width;
          if (montage_info->frame)
            {
              border_info.width=(width-image->columns+1) >> 1;
              border_info.height=(height-image->rows+1) >> 1;
            }
          bordered_image=BorderImage(image,&border_info,&border_color);
          if (bordered_image != (Image *) NULL)
            {
              DestroyImage(image);
              image=bordered_image;
            }
        }
      /*
        Gravitate image as specified by the tile gravity.
      */
      switch (resource_info->gravity)
      {
        case NorthWestGravity:
        {
          x=0;
          y=0;
          break;
        }
        case NorthGravity:
        {
          x=((width+(border_width << 1))-image->columns) >> 1;
          y=0;
          break;
        }
        case NorthEastGravity:
        {
          x=(width+(border_width << 1))-image->columns;
          y=0;
          break;
        }
        case WestGravity:
        {
          x=0;
          y=((height+(border_width << 1))-image->rows) >> 1;
          break;
        }
        case ForgetGravity:
        case StaticGravity:
        case CenterGravity:
        default:
        {
          x=((width+(border_width << 1))-image->columns) >> 1;
          y=((height+(border_width << 1))-image->rows) >> 1;
          break;
        }
        case EastGravity:
        {
          x=(width+(border_width << 1))-image->columns;
          y=((height+(border_width << 1))-image->rows) >> 1;
          break;
        }
        case SouthWestGravity:
        {
          x=0;
          y=(height+(border_width << 1))-image->rows;
          break;
        }
        case SouthGravity:
        {
          x=((width+(border_width << 1))-image->columns) >> 1;
          y=(height+(border_width << 1))-image->rows;
          break;
        }
        case SouthEastGravity:
        {
          x=(width+(border_width << 1))-image->columns;
          y=(height+(border_width << 1))-image->rows;
          break;
        }
      }
      if (montage_info->frame && (bevel_width != 0))
        {
          FrameInfo
            frame_info;

          Image
            *framed_image;

          /*
            Put an ornamental border around this tile.
          */
          frame_info.matte_color=matte_color;
          frame_info.highlight_color=highlight_color;
          frame_info.shadow_color=shadow_color;
          frame_info.width=width+(border_width << 1);
          frame_info.height=height+(border_width << 1);
          frame_info.height+=(font_height+4)*number_lines;
          frame_info.x=(x > 0 ? x : border_width);
          frame_info.y=(y > 0 ? y : border_width);
          frame_info.inner_bevel=bevel_width;
          frame_info.outer_bevel=bevel_width;
          framed_image=FrameImage(image,&frame_info);
          if (framed_image != (Image *) NULL)
            {
              DestroyImage(image);
              image=framed_image;
            }
          x=0;
          y=0;
        }
      /*
        Composite background image with tile image.
      */
      CompositeImage(montage_image,montage_info->compose,image,
        x_offset+x,y_offset+y);
      if (montage_info->shadow)
        {
          ColorPacket
            shadow_color,
            trough_color;

          register int
            columns,
            rows;

          /*
            Put a shadow under the tile to show depth.
          */
          (void) XQueryColorDatabase(resource_info->background_color,&color);
          shadow_color.red=(unsigned int)
            (XDownScale(color.red)*ShadowModulate)/MaxRGB;
          shadow_color.green=(unsigned int)
            (XDownScale(color.green)*ShadowModulate)/MaxRGB;
          shadow_color.blue=(unsigned int)
            (XDownScale(color.blue)*ShadowModulate)/MaxRGB;
          (void) XQueryColorDatabase(resource_info->background_color,&color);
          trough_color.red=(unsigned int)
            (XDownScale(color.red)*TroughModulate)/MaxRGB;
          trough_color.green=(unsigned int)
            (XDownScale(color.green)*TroughModulate)/MaxRGB;
          trough_color.blue=(unsigned int)
            (XDownScale(color.blue)*TroughModulate)/MaxRGB;
          for (rows=0; rows < image->rows; rows++)
          {
            p=montage_image->pixels+montage_image->columns*(y_offset+y+rows+4)+
              x_offset+x+image->columns;
            for (columns=0; columns < Min(tile_info.x,4); columns++)
            {
              if (((columns+rows) % 2) == 0)
                {
                  p->red=shadow_color.red;
                  p->green=shadow_color.green;
                  p->blue=shadow_color.blue;
                }
              else
                {
                  p->red=trough_color.red;
                  p->green=trough_color.green;
                  p->blue=trough_color.blue;
                }
              p++;
            }
          }
          for (rows=0; rows < Min(tile_info.y,4); rows++)
          {
            p=montage_image->pixels+montage_image->columns*
              (y_offset+y+image->rows+rows)+x_offset+x+4;
            for (columns=0; columns < image->columns; columns++)
            {
              if (((columns+rows) % 2) == 0)
                {
                  p->red=shadow_color.red;
                  p->green=shadow_color.green;
                  p->blue=shadow_color.blue;
                }
              else
                {
                  p->red=trough_color.red;
                  p->green=trough_color.green;
                  p->blue=trough_color.blue;
                }
              p++;
            }
          }
        }
      if (image->label != (char *) NULL)
        {
          /*
            Annotate composite image tile with label.
          */
          (void) sprintf(annotate_info.geometry,"%ux%u%+d%+d",
            (montage_info->frame ? image->columns : width)-(border_width << 1),
            font_height,(int) (x_offset+border_width),(int)
            (montage_info->frame ? y_offset+height+(border_width << 1)-
            bevel_width-2 : y_offset+y+tile_info.y+image->rows+2));
          annotate_info.text=image->label;
          AnnotateImage(montage_image,&annotate_info);
        }
      x_offset+=width+(tile_info.x+border_width)*2;
      if (((tile+1) == number_tiles) || (((tile+1) % tiles_per_row) == 0))
        {
          x_offset=tile_info.x;
          y_offset+=
            height+(tile_info.y+border_width)*2+(font_height+4)*number_lines;
          max_height=0;
        }
      DestroyImage(image);
      (void) SetMonitorHandler(handler);
      ProgressMonitor(MontageImageText,(i+1)*tile,number_images*number_tiles);
    }
    CompressImage(montage_image);
    if ((i+1) < number_images)
      {
        /*
          Allocate next image structure.
        */
        montage_image->next=AllocateImage((ImageInfo *) NULL);
        if (montage_image->next == (Image *) NULL)
          {
            DestroyImages(montage_image);
            return((Image *) NULL);
          }
        (void) strcpy(montage_image->next->filename,filename);
        montage_image->next->file=montage_image->file;
        montage_image->next->scene=montage_image->scene+1;
        montage_image->next->previous=montage_image;
        montage_image=montage_image->next;
        images+=number_tiles;
        montage_info->number_tiles-=number_tiles;
      }
  }
  while (montage_image->previous != (Image *) NULL)
    montage_image=montage_image->previous;
  return(montage_image);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   X Q u e r y C o l o r D a t a b a s e                                     %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Function XQueryColorDatabase looks up a RGB values for a color given in the
%  target string.
%
%  The format of the XQueryColorDatabase routine is:
%
%    status=XQueryColorDatabase(target,color)
%
%  A description of each parameter follows:
%
%    o status:  Function XQueryColorDatabase returns True if the RGB values
%      of the target color is defined, otherwise False is returned.
%
%    o target: Specifies the color to lookup in the X color database.
%
%    o color: A pointer to an XColor structure.  The RGB value of the target
%      color is returned as this value.
%
%
*/
unsigned int XQueryColorDatabase(char *target, XColor *color)
{
  char
    colorname[MaxTextLength],
    text[MaxTextLength];

  Colormap
    colormap;

  FILE
    *database;

  int
    blue,
    count,
    green,
    red,
    status;

  register XColorList
    *p;

  static Display
    *display = (Display *) NULL;

  /*
    Initialize color return value.
  */
  color->red=0;
  color->green=0;
  color->blue=0;
  color->flags=DoRed | DoGreen | DoBlue;
  if (target == (char *) NULL)
    target=BackgroundColor;
  if (*target == '#')
    {
      char
        c;

      register int
        i,
        n;

      /*
        Parse RGB specification.
      */
      target++;
      n=strlen(target);
      if ((n != 3) && (n != 6) && (n != 9) && (n != 12))
        return(False);
      n/=3;
      green=0;
      blue=0;
      do
      {
        red=green;
        green=blue;
        blue=0;
        for (i=n-1; i >= 0; i--)
        {
          c=(*target++);
          blue<<=4;
          if ((c >= '0') && (c <= '9'))
            blue|=c-'0';
          else
            if ((c >= 'A') && (c <= 'F'))
              blue|=c-('A'-10);
            else
              if ((c >= 'a') && (c <= 'f'))
                blue|=c-('a'-10);
              else
                return(False);
         }
      } while (*target != '\0');
      n<<=2;
      n=16-n;
      color->red=red << n;
      color->green=green << n;
      color->blue=blue << n;
      return(True);
    }
  database=fopen(RGBColorDatabase,"r");
  if (database != (FILE *) NULL)
    {
      /*
        Match color against the X color database.
      */
      while (fgets(text,MaxTextLength-1,database) != (char *) NULL)
      {
        count=sscanf(text,"%d %d %d %[^\n]\n",&red,&green,&blue,colorname);
        if (count != 4)
          continue;
        if (Latin1Compare(colorname,target) == 0)
          {
            color->red=red << 8;
            color->green=green << 8;
            color->blue=blue << 8;
            return(True);
          }
      }
      (void) fclose(database);
    }
  /*
    Search our internal color database.
  */
  for (p=ColorList; p->name != (char *) NULL; p++)
    if (Latin1Compare((char*)p->name,target) == 0)
      {
        color->red=p->red << 8;
        color->green=p->green << 8;
        color->blue=p->blue << 8;
        return(True);
      }
  /*
    Let the X server define the color for us.
  */
  if (display == (Display *) NULL)
    display=XOpenDisplay((char *) NULL);
  if (display == (Display *) NULL)
    {
      Warning("Color is not known to X server",target);
      return(False);
    }
  colormap=XDefaultColormap(display,XDefaultScreen(display));
  status=XParseColor(display,colormap,target,color);
  if (status == False)
    Warning("Color is not known to X server",target);
  return(status != 0);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   X Q u e r y P o s i t i o n                                               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Function XQueryPosition gets the pointer coodinates relative to a window.
%
%  The format of the XQueryPosition routine is:
%
%    XQueryPosition(display,window,x,y)
%
%  A description of each parameter follows:
%
%    o display: Specifies a connection to an X server;  returned from
%      XOpenDisplay.
%
%    o window: Specifies a pointer to a Window.
%
%    o x: Return the x coordinate of the pointer relative to the origin of the
%      window.
%
%    o y: Return the y coordinate of the pointer relative to the origin of the
%      window.
%
%
*/
void XQueryPosition(Display *display, Window window, int *x, int *y)
{
  int
    x_root,
    y_root;

  unsigned int
    mask;

  Window
    root_window;

  (void) XQueryPointer(display,window,&root_window,&root_window,&x_root,&y_root,
    x,y,&mask);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   X R e f r e s h W i n d o w                                               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Function XRefreshWindow refreshes an image in a X window.
%
%  The format of the XRefreshWindow routine is:
%
%      XRefreshWindow(display,window,event)
%
%  A description of each parameter follows:
%
%    o display: Specifies a connection to an X server;  returned from
%      XOpenDisplay.
%
%    o window: Specifies a pointer to a XWindowInfo structure.
%
%    o event: Specifies a pointer to a XEvent structure.  If it is NULL,
%      the entire image is refreshed.
%
%
*/
void XRefreshWindow(Display *display, XWindowInfo *window, XEvent *event)
{
  int
    x,
    y;

  unsigned int
    height,
    width;

  if (event != (XEvent *) NULL)
    {
      /*
        Determine geometry from expose event.
      */
      x=event->xexpose.x;
      y=event->xexpose.y;
      width=event->xexpose.width;
      height=event->xexpose.height;
    }
  else
    {
      XEvent
        sans_event;

      /*
        Refresh entire window; discard outstanding expose events.
      */
      x=0;
      y=0;
      width=window->width;
      height=window->height;
      while (XCheckTypedWindowEvent(display,window->id,Expose,&sans_event));
    }
  /*
    Check boundary conditions.
  */
  if ((window->ximage->width-(x+window->x)) < width)
    width=window->ximage->width-(x+window->x);
  if ((window->ximage->height-(y+window->y)) < height)
    height=window->ximage->height-(y+window->y);
  /*
    Refresh image.
  */
  XSetClipMask(display,window->annotate_context,window->matte_pixmap);
  if (window->pixmap != (Pixmap) NULL)
    {
      if (window->depth > 1)
        XCopyArea(display,window->pixmap,window->id,window->annotate_context,
          x+window->x,y+window->y,width,height,x,y);
      else
        XCopyPlane(display,window->pixmap,window->id,window->highlight_context,
          x+window->x,y+window->y,width,height,x,y,1L);
    }
  else
    {
#ifdef HasSharedMemory
      if (window->shared_memory)
        {
          XShmPutImage(display,window->id,window->annotate_context,
            window->ximage,x+window->x,y+window->y,x,y,width,height,True);
          XSync(display,False);
        }
#endif
      if (!window->shared_memory)
        XPutImage(display,window->id,window->annotate_context,window->ximage,
          x+window->x,y+window->y,x,y,width,height);
    }
  XSetClipMask(display,window->annotate_context,None);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   X R e t a i n W i n d o w C o l o r s                                     %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Function XRetainWindowColors sets X11 color resources on a window.  This
%  perserves the colors associated with an image displayed on the window.
%
%  The format of the XRetainWindowColors routine is:
%
%      XRetainWindowColors(display,window)
%
%  A description of each parameter follows:
%
%    o display: Specifies a connection to an X server; returned from
%      XOpenDisplay.
%
%    o window: Specifies a pointer to a XWindowInfo structure.
%
%
*/
void XRetainWindowColors(Display *display, Window window)
{
  Atom
    property;

  Pixmap
    pixmap;

  /*
    Put property on the window.
  */
  property=XInternAtom(display,"_XSETROOT_ID",False);
  if (property == (Atom) NULL)
    {
      Warning("Unable to create X property","_XSETROOT_ID");
      return;
    }
  pixmap=XCreatePixmap(display,window,1,1,1);
  if (pixmap == (Pixmap) NULL)
    {
      Warning("Unable to create X pixmap",(char *) NULL);
      return;
    }
  XChangeProperty(display,window,property,XA_PIXMAP,32,PropModeReplace,
    (unsigned char *) &pixmap,1);
  XSetCloseDownMode(display,RetainPermanent);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   X S e l e c t W i n d o w                                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Function XSelectWindow allows a user to select a window using the mouse.  If
%  the mouse moves, a cropping rectangle is drawn and the extents of the
%  rectangle is returned in the crop_info structure.
%
%  The format of the XSelectWindow function is:
%
%      target_window=XSelectWindow(display,crop_info)
%
%  A description of each parameter follows:
%
%    o window: XSelectWindow returns the window id.
%
%    o display: Specifies a pointer to the Display structure;  returned from
%      XOpenDisplay.
%
%    o crop_info: Specifies a pointer to a RectangleInfo structure.  It
%      contains the extents of any cropping rectangle.
%
%
*/
Window XSelectWindow(Display *display, RectangleInfo *crop_info)
{
#define MinimumCropArea  (unsigned int) 9

  Cursor
    target_cursor;

  GC
    annotate_context;

  int
    presses,
    status,
    x_offset,
    y_offset;

  Window
    root_window,
    target_window;

  XEvent
    event;

  XGCValues
    context_values;

  /*
    Initialize graphic context.
  */
  root_window=XRootWindow(display,XDefaultScreen(display));
  context_values.background=XBlackPixel(display,XDefaultScreen(display));
  context_values.foreground=XWhitePixel(display,XDefaultScreen(display));
  context_values.function=GXinvert;
  context_values.plane_mask=
    context_values.background ^ context_values.foreground;
  context_values.subwindow_mode=IncludeInferiors;
  annotate_context=XCreateGC(display,root_window,GCBackground | GCForeground |
    GCFunction | GCPlaneMask | GCSubwindowMode,&context_values);
  if (annotate_context == (GC) NULL)
    return(False);
  /*
    Grab the pointer using target cursor.
  */
  target_cursor=XMakeCursor(display,root_window,
    XDefaultColormap(display,XDefaultScreen(display)),"white","black");
  status=XGrabPointer(display,root_window,False,(unsigned int)
    (ButtonPressMask | ButtonReleaseMask | ButtonMotionMask),GrabModeSync,
    GrabModeAsync,root_window,target_cursor,CurrentTime);
  if (status != GrabSuccess)
    Error("Unable to grab the mouse",(char *) NULL);
  /*
    Select a window.
  */
  crop_info->width=0;
  crop_info->height=0;
  presses=0;
  target_window=(Window) NULL;
  x_offset=0;
  y_offset=0;
  do
  {
    if ((crop_info->width*crop_info->height) >= MinimumCropArea)
      XDrawRectangle(display,root_window,annotate_context,crop_info->x,
        crop_info->y,crop_info->width-1,crop_info->height-1);
    /*
      Allow another event.
    */
    XAllowEvents(display,SyncPointer,CurrentTime);
    XWindowEvent(display,root_window,ButtonPressMask | ButtonReleaseMask |
      ButtonMotionMask,&event);
    if ((crop_info->width*crop_info->height) >= MinimumCropArea)
      XDrawRectangle(display,root_window,annotate_context,crop_info->x,
        crop_info->y,crop_info->width-1,crop_info->height-1);
    switch (event.type)
    {
      case ButtonPress:
      {
        target_window=XGetSubwindow(display,event.xbutton.subwindow,
          event.xbutton.x,event.xbutton.y);
        if (target_window == (Window) NULL)
          target_window=root_window;
        x_offset=event.xbutton.x_root;
        y_offset=event.xbutton.y_root;
        crop_info->x=x_offset;
        crop_info->y=y_offset;
        crop_info->width=0;
        crop_info->height=0;
        presses++;
        break;
      }
      case ButtonRelease:
      {
        presses--;
        break;
      }
      case MotionNotify:
      {
        /*
          Discard pending button motion events.
        */
        while (XCheckMaskEvent(display,ButtonMotionMask,&event));
        crop_info->x=event.xmotion.x;
        crop_info->y=event.xmotion.y;
        /*
          Check boundary conditions.
        */
        if (crop_info->x < x_offset)
          crop_info->width=(unsigned int) (x_offset-crop_info->x);
        else
          {
            crop_info->width=(unsigned int) (crop_info->x-x_offset);
            crop_info->x=x_offset;
          }
        if (crop_info->y < y_offset)
          crop_info->height=(unsigned int) (y_offset-crop_info->y);
        else
          {
            crop_info->height=(unsigned int) (crop_info->y-y_offset);
            crop_info->y=y_offset;
          }
      }
      default:
        break;
    }
  }
  while ((target_window == (Window) NULL) || (presses > 0));
  XUngrabPointer(display,CurrentTime);
  XFreeCursor(display,target_cursor);
  XFreeGC(display,annotate_context);
  if ((crop_info->width*crop_info->height) < MinimumCropArea)
    {
      crop_info->width=0;
      crop_info->height=0;
    }
  if ((crop_info->width != 0) && (crop_info->height != 0))
    target_window=root_window;
  return(target_window);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   X S e t C u r s o r S t a t e                                             %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Function XSetCursorState sets the  cursor state to busy, otherwise the
%  cursor are reset to their default.
%
%  The format of the XXSetCursorState routine is:
%
%      XSetCursorState(display,windows,state)
%
%  A description of each parameter follows:
%
%    o display: Specifies a connection to an X server;  returned from
%      XOpenDisplay.
%
%    o windows: Specifies a pointer to a XWindows structure.
%
%    o state: An unsigned integer greater than 0 sets the cursor state
%      to busy, otherwise the cursor are reset to their default.
%
%
*/
void XSetCursorState(Display *display, XWindows *windows, unsigned int state)
{
  if (state)
    {
      XDefineCursor(display,windows->image.id,windows->image.busy_cursor);
      XDefineCursor(display,windows->pan.id,windows->pan.busy_cursor);
      XDefineCursor(display,windows->magnify.id,windows->magnify.busy_cursor);
      XDefineCursor(display,windows->command.id,windows->command.busy_cursor);
    }
  else
    {
      XDefineCursor(display,windows->image.id,windows->image.cursor);
      XDefineCursor(display,windows->pan.id,windows->pan.cursor);
      XDefineCursor(display,windows->magnify.id,windows->magnify.cursor);
      XDefineCursor(display,windows->command.id,windows->command.cursor);
      XDefineCursor(display,windows->command.id,windows->widget.cursor);
      XWithdrawWindow(display,windows->info.id,windows->info.screen);
    }
  windows->info.mapped=False;
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   X V i s u a l C l a s s N a m e                                           %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Function XVisualClassName returns the visual class name as a character
%  string.
%
%  The format of the XVisualClassName routine is:
%
%      visual_type=XVisualClassName(class)
%
%  A description of each parameter follows:
%
%    o visual_type: XVisualClassName returns the visual class as a character
%      string.
%
%    o class: Specifies the visual class.
%
%
*/
char *XVisualClassName(int class)
{
  switch (class)
  {
    case StaticGray: return("StaticGray");
    case GrayScale: return("GrayScale");
    case StaticColor: return("StaticColor");
    case PseudoColor: return("PseudoColor");
    case TrueColor: return("TrueColor");
    case DirectColor: return("DirectColor");
  }
  return("unknown visual class");
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   X W i n d o w B y I D                                                     %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Function XWindowByID locates a child window with a given ID.  If not window
%  with the given name is found, 0 is returned.   Only the window specified
%  and its subwindows are searched.
%
%  The format of the XWindowByID function is:
%
%      child=XWindowByID(display,window,id)
%
%  A description of each parameter follows:
%
%    o child: XWindowByID returns the window with the specified
%      id.  If no windows are found, XWindowByID returns 0.
%
%    o display: Specifies a pointer to the Display structure;  returned from
%      XOpenDisplay.
%
%    o id: Specifies the id of the window to locate.
%
%
*/
Window XWindowByID(Display *display, Window root_window, long unsigned int id)
{
  RectangleInfo
    rectangle_info;

  register int
    i;

  unsigned int
    number_children;

  Window
    child,
    *children,
    window;

  if (id == 0)
    return(XSelectWindow(display,&rectangle_info));
  if (root_window == id)
    return(id);
  if (!XQueryTree(display,root_window,&child,&child,&children,&number_children))
    return((Window) NULL);
  window=(Window) NULL;
  for (i=0; i < number_children; i++)
  {
    /*
      Search each child and their children.
    */
    window=XWindowByID(display,children[i],id);
    if (window != (Window) NULL)
      break;
  }
  if (children != (Window *) NULL)
    XFree((void *) children);
  return(window);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   X W i n d o w B y N a m e                                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Function XWindowByName locates a window with a given name on a display.
%  If no window with the given name is found, 0 is returned. If more than
%  one window has the given name, the first one is returned.  Only root and
%  its children are searched.
%
%  The format of the XWindowByName function is:
%
%      window=XWindowByName(display,root_window,name)
%
%  A description of each parameter follows:
%
%    o window: XWindowByName returns the window id.
%
%    o display: Specifies a pointer to the Display structure;  returned from
%      XOpenDisplay.
%
%    o root_window: Specifies the id of the root window.
%
%    o name: Specifies the name of the window to locate.
%
%
*/
Window XWindowByName(Display *display, Window root_window, char *name)
{
  register int
    i;

  unsigned int
    number_children;

  Window
    *children,
    child,
    window;

  XTextProperty
    window_name;

  if (XGetWMName(display,root_window,&window_name) != 0)
    if (strcmp((char *) window_name.value,name) == 0)
      return(root_window);
  if (!XQueryTree(display,root_window,&child,&child,&children,&number_children))
    return((Window) NULL);
  window=(Window) NULL;
  for (i=0; i < number_children; i++)
  {
    /*
      Search each child and their children.
    */
    window=XWindowByName(display,children[i],name);
    if (window != (Window) NULL)
      break;
  }
  if (children != (Window *) NULL)
    XFree((void *) children);
  return(window);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   X W i n d o w B y P r o p e r y                                           %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Function XWindowByProperty locates a child window with a given property.
%  If no window with the given name is found, 0 is returned.  If more than
%  one window has the given property, the first one is returned.  Only the
%  window specified and its subwindows are searched.
%
%  The format of the XWindowByProperty function is:
%
%      child=XWindowByProperty(display,window,property)
%
%  A description of each parameter follows:
%
%    o child: XWindowByProperty returns the window id with the specified
%      property.  If no windows are found, XWindowByProperty returns 0.
%
%    o display: Specifies a pointer to the Display structure;  returned from
%      XOpenDisplay.
%
%    o property: Specifies the property of the window to locate.
%
%
*/
static Window XWindowByProperty(Display *display, Window window, Atom property)
{
  Atom
    type;

  int
    format,
    status;

  unsigned char
    *data;

  unsigned int
    i,
    number_children;

  unsigned long
    after,
    number_items;

  Window
    child,
    *children,
    parent,
    root;

  status=XQueryTree(display,window,&root,&parent,&children,&number_children);
  if (status == 0)
    return((Window) NULL);
  type=(Atom) NULL;
  child=(Window) NULL;
  for (i=0; (i < number_children) && (child == (Window) NULL); i++)
  {
    status=XGetWindowProperty(display,children[i],property,0L,0L,False,
      (Atom) AnyPropertyType,&type,&format,&number_items,&after,&data);
    if (data != NULL)
      XFree((void *) data);
    if ((status == Success) && (type != (Atom) NULL))
      child=children[i];
  }
  for (i=0; (i < number_children) && (child == (Window) NULL); i++)
    child=XWindowByProperty(display,children[i],property);
  if (children != (Window *) NULL)
    XFree((void *) children);
  return(child);
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* Kobus: new routine to get the pixels from an image.   */

void set_pure_pixels(Image *image)
{
     extern unsigned char* pure_pixels;
     extern int num_image_columns;
     int i,j;
     unsigned char *q, *q_pos;
     RunlengthPacket *p;
     int count = 0;


     if (pure_pixels != NULL) free(pure_pixels);

     num_image_columns = image-> columns;

     q = (unsigned char*)malloc(image->rows * image->columns * 3);

     if (q != NULL)
     {
         q_pos = q;

         p = image->pixels;

         for (i=0; i < image->packets; i++) {
             for (j=0; j <= ((int) p->length); j++) {
                 *q_pos = p->red;
                 q_pos++;
                 *q_pos = p->green;
                 q_pos++;
                 *q_pos = p->blue;
                 q_pos++;

                 count++;
                }
             p++;
            }
     }

     pure_pixels = q;
    }

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

#ifdef __cplusplus
}
#endif


/* -------------------------------------------------------------------------- */

#endif   /* #ifdef KJB_HAVE_X11  */

#endif   /* #ifndef __C2MAN__  */

