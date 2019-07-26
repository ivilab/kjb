
/* $Id: image.c 4727 2009-11-16 20:53:54Z kobus $ */

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
%                       IIIII  M   M   AAA   GGGG  EEEEE                      %
%                         I    MM MM  A   A G      E                          %
%                         I    M M M  AAAAA G  GG  EEE                        %
%                         I    M   M  A   A G   G  E                          %
%                       IIIII  M   M  A   A  GGGG  EEEEE                      %
%                                                                             %
%                                                                             %
%                          ImageMagick Image Routines                         %
%                                                                             %
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
  Function prototypes.
*/
static double
  (*FilterFunction) _Declare((double));

static int
  IntensityCompare _Declare((const void *,const void *)),
  NoisyCompare _Declare((const void *,const void *));

static unsigned int
  InsidePrimitive _Declare((PrimitiveInfo *,int,int));

static void
  Contrast _Declare((int,Quantum *,Quantum *,Quantum *)),
  Hull _Declare((int,int,int,unsigned int,unsigned int,Quantum *,Quantum *)),
  HSVTransform _Declare((double,double,double,Quantum *,Quantum *,Quantum *)),
  Modulate _Declare((double,double,double,Quantum *,Quantum *,Quantum *)),
  TransformHSV _Declare((unsigned int,unsigned int,unsigned int,double *,
    double *,double *));

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   A l l o c a t e I m a g e                                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Function AllocateImage allocates an Image structure and initializes each
%  field to a default value.
%
%  The format of the AllocateImage routine is:
%
%      allocated_image=AllocateImage(image_info)
%
%  A description of each parameter follows:
%
%    o allocated_image: Function AllocateImage returns a pointer to an image
%      structure initialized to default values.  A null image is returned if
%      there is a memory shortage.
%
%    o image_info: Specifies a pointer to a ImageInfo structure.
%
%
*/
Image *AllocateImage(ImageInfo *image_info)
{
  Image
    *allocated_image;

  /*
    Allocate image structure.
  */
  allocated_image=(Image *) malloc(sizeof(Image));
  if (allocated_image == (Image *) NULL)
    {
      Warning("Unable to allocate image","Memory allocation failed");
      return((Image *) NULL);
    }
  /*
    Initialize Image structure.
  */
  allocated_image->file=(FILE *) NULL;
  allocated_image->status=False;
  allocated_image->temporary=False;
  *allocated_image->filename='\0';
  allocated_image->filesize=0;
  allocated_image->pipe=False;
  (void) strcpy(allocated_image->magick,"MIFF");
  allocated_image->comments=(char *) NULL;
  allocated_image->label=(char *) NULL;
  allocated_image->text=(char *) NULL;
  allocated_image->id=UndefinedId;
  allocated_image->class=DirectClass;
  allocated_image->matte=False;
  allocated_image->compression=RunlengthEncodedCompression;
  allocated_image->columns=0;
  allocated_image->rows=0;
  allocated_image->depth=QuantumDepth;
  allocated_image->interlace=False;
  allocated_image->scene=0;
  allocated_image->units=2;  /* pixels per inch */
  allocated_image->x_resolution=72.0;
  allocated_image->y_resolution=72.0;
  allocated_image->montage=(char *) NULL;
  allocated_image->directory=(char *) NULL;
  allocated_image->colormap=(ColorPacket *) NULL;
  allocated_image->colorspace=0;
  allocated_image->colors=0;
  allocated_image->gamma=0.0;
  allocated_image->normalized_maximum_error=0.0;
  allocated_image->normalized_mean_error=0.0;
  allocated_image->mean_error_per_pixel=0;
  allocated_image->total_colors=0;
  allocated_image->signature=(char *) NULL;
  allocated_image->pixels=(RunlengthPacket *) NULL;
  allocated_image->packet=(RunlengthPacket *) NULL;
  allocated_image->packets=0;
  allocated_image->packet_size=0;
  allocated_image->packed_pixels=(unsigned char *) NULL;
  *allocated_image->magick_filename='\0';
  allocated_image->magick_columns=0;
  allocated_image->magick_rows=0;
  allocated_image->magick_time=time((time_t *) NULL);
  allocated_image->geometry=(char *) NULL;
  allocated_image->orphan=False;
  allocated_image->previous=(Image *) NULL;
  allocated_image->list=(Image *) NULL;
  allocated_image->next=(Image *) NULL;
  if (image_info != (ImageInfo *) NULL)
    {
      (void) strcpy(allocated_image->filename,image_info->filename);
      (void) strcpy(allocated_image->magick,image_info->magick);
    }
  return(allocated_image);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   A n n o t a t e I m a g e                                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Function AnnotateImage annotates an image with test.  Optionally the
%  annotation can include the image filename, type, width, height, or scene
%  number by embedding special format characters.  Embed %f for filename,
%  %m for magick, %w for width, %h for height, %s for scene number, or \n
%  for newline.  For example,
%
%     %f  %wx%h
%
%  produces an image annotation of
%
%     bird.miff  512x480
%
%  for an image titled bird.miff and whose width is 512 and height is 480.
%
%  The format of the AnnotateImage routine is:
%
%      AnnotateImage(image,annotate_info)
%
%  A description of each parameter follows:
%
%    o image: The address of a structure of type Image.
%
%    o annotate_info: The address of a AnnotateInfo structure.
%
%
*/
void AnnotateImage(Image *image, AnnotateInfo *annotate_info)
{
  char
    *text,
    **textlist;

  FILE
    *file;

  int
    flags,
    status,
    x,
    y;

  register char
    *p,
    *q;

  register int
    i,
    j;

  static Display
    *display = (Display *) NULL;

  static XPixelInfo
    pixel_info;

  unsigned int
    height,
    indirection,
    length,
    width;

  XAnnotateInfo
    xannotate_info;

  XFontStruct
    *font_info;

  XResourceInfo
    resource_info;

  XrmDatabase
    resource_database;

  XStandardColormap
    *map_info;

  XVisualInfo
    *visual_info;

  if (annotate_info->text == (char *) NULL)
    return;
  indirection=(*annotate_info->text == '@');
  if (indirection)
    {
      int
        c;

      /*
        Read text from a file.
      */
      file=(FILE *) fopen(annotate_info->text+1,"r");
      if (file == (FILE *) NULL)
        {
          Warning("Unable to read text file",annotate_info->text+1);
          return;
        }
      length=MaxTextLength;
      annotate_info->text=(char *) malloc(length);
      for (q=annotate_info->text; annotate_info->text != (char *) NULL; q++)
      {
        c=fgetc(file);
        if (c == EOF)
          break;
        if ((q-annotate_info->text+1) >= length)
          {
            *q='\0';
            length<<=1;
            annotate_info->text=(char*)realloc(annotate_info->text,length);
            if (annotate_info->text == (char *) NULL)
              break;
            q=annotate_info->text+strlen(annotate_info->text);
          }
        *q=(unsigned char) c;
      }
      (void) fclose(file);
      if (annotate_info->text == (char *) NULL)
        {
          Warning("Unable to annotate image","Memory allocation failed");
          return;
        }
      *q='\0';
    }
  /*
    Allocate and initialize image text.
  */
  p=annotate_info->text;
  length=strlen(annotate_info->text)+MaxTextLength;
  image->text=(char *) malloc(length);
  for (q=image->text; image->text != (char *) NULL; p++)
  {
    *q='\0';
    if (*p == '\0')
      break;
    if ((q-image->text+MaxTextLength) >= length)
      {
        length<<=1;
        image->text=(char *) realloc((char *) image->text,length);
        if (image->text == (char *) NULL)
          break;
        q=image->text+strlen(image->text);
      }
    /*
      Process formatting characters in text.
    */
    if ((*p == '\\') && (*(p+1) == 'n'))
      {
        *q++='\n';
        p++;
        continue;
      }
    if (*p != '%')
      {
        *q++=(*p);
        continue;
      }
    p++;
    switch (*p)
    {
      case 'f':
      {
        register char
          *p;

        /*
          Label segment is the base of the filename.
        */
        p=image->filename+strlen(image->filename)-1;
        while ((p > image->filename) && (*(p-1) != '/'))
          p--;
        (void) strcpy(q,p);
        q+=strlen(p);
        break;
      }
      case 'h':
      {
        (void) sprintf(q,"%u",image->magick_rows);
        q=image->text+strlen(image->text);
        break;
      }
      case 'm':
      {
        (void) strcpy(q,image->magick);
        q+=strlen(image->magick);
        break;
      }
      case 's':
      {
        (void) sprintf(q,"%u",image->scene);
        q=image->text+strlen(image->text);
        break;
      }
      case 'w':
      {
        (void) sprintf(q,"%u",image->magick_columns);
        q=image->text+strlen(image->text);
        break;
      }
      default:
      {
        *q++='%';
        *q++=(*p);
        break;
      }
    }
  }
  if (image->text == (char *) NULL)
    {
      Warning("Unable to annotate image","Memory allocation failed");
      return;
    }
  *q++='\0';
  *q++='\0';
  if (indirection)
    free((char *) annotate_info->text);
  textlist=StringToList(image->text);
  free(image->text);
  image->text=(char *) NULL;
  if (textlist == (char **) NULL)
    return;
  length=strlen(textlist[0]);
  for (i=0; textlist[i] != (char *) NULL; i++)
    if (strlen(textlist[i]) > length)
      length=strlen(textlist[i]);
  text=(char*)malloc(length+4);
  if (text == (char *) NULL)
    {
      Warning("Unable to annotate image","Memory allocation error");
      return;
    }
  /*
    Get annotate geometry.
  */
  x=0;
  y=0;
  width=image->columns;
  height=annotate_info->pointsize;
  flags=NoValue;
  if (annotate_info->geometry != (char *) NULL)
    {
      flags=XParseGeometry(annotate_info->geometry,&x,&y,&width,&height);
      if ((flags & XNegative) != 0)
        x+=image->columns;
      if ((flags & YNegative) != 0)
        y+=image->rows;
    }
  /*
    Open X server connection.
  */
  if (display == (Display *) NULL)
    display=XOpenDisplay(annotate_info->server_name);
  if (display == (Display *) NULL)
    {
      char
        filename[MaxTextLength],
        page[MaxTextLength];

      Image
        *annotate_image;

      ImageInfo
        image_info;

      register RunlengthPacket
        *q;

      unsigned int
        matte;

      XColor
        box_color,
        pen_color;

      /*
        X server fonts are not available, use Postscript to annotate.
      */
      (void) XQueryColorDatabase(annotate_info->box,&box_color);
      (void) XQueryColorDatabase(annotate_info->pen,&pen_color);
      GetImageInfo(&image_info);
      TemporaryFilename(filename);
      image_info.monochrome=True;
      (void) sprintf(page,"%ux%u",height*length,height << 1);
      image_info.page=page;
      if (annotate_info->font == (char *) NULL)
        annotate_info->font=DefaultFont;
      for (i=0; textlist[i] != (char *) NULL; i++)
      {
        if ((x >= image->columns) || (y >= image->rows))
          break;
        if (*textlist[i] == '\0')
          {
            free(textlist[i]);
            continue;
          }
        (void) strcpy(text,textlist[i]);
        for (j=((int) strlen(textlist[i])-1) >> 1; j >= 0; j--)
        {
          (void) strcpy(image_info.filename,filename);
          file=fopen(filename,"w");
          if (file == (FILE *) NULL)
            break;
          (void) fprintf(file,"%%!PS-Adobe-3.0\n");
          (void) fprintf(file,"/%s findfont %u scalefont setfont\n",
            annotate_info->font,height);
          (void) fprintf(file,"0 %u moveto (%s) show\n",height,text);
          (void) fprintf(file,"showpage\n");
          (void) fclose(file);
          annotate_image=ReadImage(&image_info);
          if (annotate_image == (Image *) NULL)
            break;
          TransformImage(&annotate_image,"0x0",(char *) NULL);
          if (annotate_image->columns < width)
            break;
          DestroyImage(annotate_image);
          (void) strcpy(text,textlist[i]);
          (void) strcpy(text+j,"...");
          (void) strcat(text,textlist[i]+strlen(textlist[i])-j-1);
        }
        (void) unlink(filename);
        free(textlist[i]);
        if (annotate_image == (Image *) NULL)
          {
            Warning("Unable to annotate image",(char *) NULL);
            break;
          }
        /*
          Composite text onto the image.
        */
        annotate_image->class=DirectClass;
        annotate_image->matte=True;
        q=annotate_image->pixels;
        for (j=0; j < annotate_image->packets; j++)
        {
          if (q->index != 0)
            {
              q->red=XDownScale(box_color.red);
              q->green=XDownScale(box_color.green);
              q->blue=XDownScale(box_color.blue);
              q->index=
                annotate_info->box == (char *) NULL ? Transparent : Opaque;
            }
          else
            {
              q->red=XDownScale(pen_color.red);
              q->green=XDownScale(pen_color.green);
              q->blue=XDownScale(pen_color.blue);
              q->index=
                annotate_info->pen == (char *) NULL ? Transparent : Opaque;
            }
          q++;
        }
        matte=image->matte;
        CompositeImage(image,OverCompositeOp,annotate_image,x+
          (annotate_info->center ? (width >> 1)-(annotate_image->columns >> 1) :
          0),y);
        image->matte=matte;
        y+=annotate_info->pointsize;
        DestroyImage(annotate_image);
      }
      free(text);
      for ( ; textlist[i] != (char *) NULL; i++)
        free(textlist[i]);
      free((char *) textlist);
      return;
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
  resource_info.close_server=False;
  resource_info.colormap=PrivateColormap;
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
  /*
    Initialize annotate info.
  */
  XGetAnnotateInfo(&xannotate_info);
  if (annotate_info->font != (char *) NULL)
    resource_info.font=annotate_info->font;
  font_info=XBestFont(display,&resource_info,False);
  if (font_info == (XFontStruct *) NULL)
    Warning("Unable to load font",resource_info.font);
  if ((flags & HeightValue) == 0)
    height=font_info->ascent+font_info->descent;
  xannotate_info.font_info=font_info;
  xannotate_info.text=text;
  xannotate_info.x=x;
  xannotate_info.y=y;
  xannotate_info.width=width;
  xannotate_info.height=font_info->ascent+font_info->descent;
  annotate_info->pointsize=font_info->ascent+font_info->descent;
  if ((annotate_info->pen != (char *) NULL) &&
      (annotate_info->box != (char *) NULL))
    xannotate_info.stencil=OpaqueStencil;
  else
    if (annotate_info->pen != (char *) NULL)
      xannotate_info.stencil=ForegroundStencil;
    else
      xannotate_info.stencil=BackgroundStencil;
  if ((map_info == (XStandardColormap *) NULL) ||
      (visual_info == (XVisualInfo *) NULL) ||
      (font_info == (XFontStruct *) NULL))
    {
      XFreeResources(display,visual_info,map_info,&pixel_info,
        font_info,&resource_info,(XWindowInfo *) NULL);
      DestroyImage(image);
      return;
    }
  /*
    Initialize Standard Colormap.
  */
  XGetMapInfo(visual_info,XDefaultColormap(display,visual_info->screen),
    map_info);
  if (annotate_info->box != (char *) NULL)
    resource_info.background_color=annotate_info->box;
  if (annotate_info->pen != (char *) NULL)
    resource_info.foreground_color=annotate_info->pen;
  XGetPixelInfo(display,visual_info,map_info,&resource_info,(Image *) NULL,
    &pixel_info);
  pixel_info.annotate_context=XDefaultGC(display,visual_info->screen);
  /*
    Annotate the text image.
  */
  for (i=0; textlist[i] != (char *) NULL; i++)
  {
    if ((x >= image->columns) || (y >= image->rows))
      break;
    if (*textlist[i] == '\0')
      {
        free(textlist[i]);
        continue;
      }
    (void) strcpy(xannotate_info.text,textlist[i]);
    for (j=((int) strlen(textlist[i])-1) >> 1; j >= 0; j--)
    {
      xannotate_info.width=(height*
        XTextWidth(font_info,xannotate_info.text,strlen(xannotate_info.text)))/
        xannotate_info.height;
      if (xannotate_info.width < width)
        break;
      (void) strcpy(xannotate_info.text,textlist[i]);
      (void) strcpy(xannotate_info.text+j,"...");
      (void) strcat(xannotate_info.text,textlist[i]+strlen(textlist[i])-j-1);
    }
    free(textlist[i]);
    (void) sprintf(xannotate_info.geometry,"%ux%u%+d%+d",xannotate_info.width,
      height,(int) (xannotate_info.x+(annotate_info->center ? (width >> 1)-
      (xannotate_info.width >> 1) : 0)),xannotate_info.y);
    xannotate_info.width=
      XTextWidth(font_info,xannotate_info.text,strlen(xannotate_info.text));
    status=XAnnotateImage(display,&pixel_info,&xannotate_info,False,image);
    if (status == 0)
      {
        Warning("Unable to xannotate image","Memory allocation error");
        break;
      }
    xannotate_info.y+=xannotate_info.height;
    y+=xannotate_info.height;
  }
  /*
    Free resources.
  */
  XFreeResources(display,visual_info,map_info,&pixel_info,font_info,
    &resource_info,(XWindowInfo *) NULL);
  free(xannotate_info.text);
  for ( ; textlist[i] != (char *) NULL; i++)
    free(textlist[i]);
  free((char *) textlist);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%     B l u r I m a g e                                                       %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Function BlurImage creates a new image that is a copy of an existing
%  one with the pixels blurred.  It allocates the memory necessary for the
%  new Image structure and returns a pointer to the new image.
%
%  BlurImage convolves the pixel neighborhood with this blurring mask:
%
%     1  2  1
%     2  W  2
%     1  2  1
%
%  The scan only processes pixels that have a full set of neighbors.  Pixels
%  in the top, bottom, left, and right pairs of rows and columns are omitted
%  from the scan.
%
%  The format of the BlurImage routine is:
%
%      blurred_image=BlurImage(image,factor)
%
%  A description of each parameter follows:
%
%    o blurred_image: Function BlurImage returns a pointer to the image
%      after it is blurred.  A null image is returned if there is a memory
%      shortage.
%
%    o image: The address of a structure of type Image;  returned from
%      ReadImage.
%
%    o factor:  An double value reflecting the percent weight to give to the
%      center pixel of the neighborhood.
%
%
*/
Image *BlurImage(Image *image, double factor)
{
#define Blur(weight) \
  total_red+=(weight)*(int) (s->red); \
  total_green+=(weight)*(int) (s->green); \
  total_blue+=(weight)*(int) (s->blue); \
  s++;
#define BlurImageText  "  Blurring image...  "

  Image
    *blurred_image;

  long int
    total_blue,
    total_green,
    total_red,
    weight;

  register RunlengthPacket
    *p,
    *q,
    *s,
    *s0,
    *s1,
    *s2;

  register unsigned int
    x;

  RunlengthPacket
    background_pixel,
    *scanline;

  unsigned int
    quantum,
    y;

  if ((image->columns < 3) || (image->rows < 3))
    {
      Warning("Unable to blur image","image size must exceed 3x3");
      return((Image *) NULL);
    }
  /*
    Initialize blurred image attributes.
  */
  blurred_image=CopyImage(image,image->columns,image->rows,False);
  if (blurred_image == (Image *) NULL)
    {
      Warning("Unable to enhance image","Memory allocation failed");
      return((Image *) NULL);
    }
  blurred_image->class=DirectClass;
  /*
    Allocate scan line buffer for 3 rows of the image.
  */
  scanline=(RunlengthPacket *) malloc(3*image->columns*sizeof(RunlengthPacket));
  if (scanline == (RunlengthPacket *) NULL)
    {
      Warning("Unable to enhance image","Memory allocation failed");
      DestroyImage(blurred_image);
      return((Image *) NULL);
    }
  /*
    Read the first two rows of the image.
  */
  p=image->pixels;
  image->runlength=p->length+1;
  s=scanline;
  for (x=0; x < (image->columns << 1); x++)
  {
    if (image->runlength != 0)
      image->runlength--;
    else
      {
        p++;
        image->runlength=p->length;
      }
    *s=(*p);
    s++;
  }
  /*
    Dump first scanlines of image.
  */
  background_pixel.red=0;
  background_pixel.green=0;
  background_pixel.blue=0;
  background_pixel.index=0;
  background_pixel.length=0;
  q=blurred_image->pixels;
  for (x=0; x < image->columns; x++)
  {
    *q=background_pixel;
    q++;
  }
  /*
    Convolve each row.
  */
  weight=(long int) ((100.0-factor)/2-13);
  quantum=Max(weight+12,1);
  for (y=1; y < (image->rows-1); y++)
  {
    /*
      Initialize sliding window pointers.
    */
    s0=scanline+image->columns*((y-1) % 3);
    s1=scanline+image->columns*(y % 3);
    s2=scanline+image->columns*((y+1) % 3);
    /*
      Read another scan line.
    */
    s=s2;
    for (x=0; x < image->columns; x++)
    {
      if (image->runlength != 0)
        image->runlength--;
      else
        {
          p++;
          image->runlength=p->length;
        }
      *s=(*p);
      s++;
    }
    /*
      Transfer first pixel of the scanline.
    */
    *q=background_pixel;
    q++;
    for (x=1; x < (image->columns-1); x++)
    {
      /*
        Compute weighted average of target pixel color components.
      */
      total_red=0;
      total_green=0;
      total_blue=0;
      s=s0;
      Blur(1);  Blur(2); Blur(1);
      s=s1;
      Blur(2); Blur(weight); Blur(2);
      s=s2;
      Blur(1);  Blur(2); Blur(1);
      q->red=(Quantum) ((total_red+(quantum >> 1))/quantum);
      q->green=(Quantum) ((total_green+(quantum >> 1))/quantum);
      q->blue=(Quantum) ((total_blue+(quantum >> 1))/quantum);
      q->index=s1->index;
      q->length=0;
      q++;
      s0++;
      s1++;
      s2++;
    }
    /*
      Transfer last pixel of the scanline.
    */
    *q=background_pixel;
    q++;
    ProgressMonitor(BlurImageText,y,image->rows);
  }
  /*
    Dump last scanline of pixels.
  */
  for (x=0; x < image->columns; x++)
  {
    *q=background_pixel;
    q++;
  }
  free((char *) scanline);
  return(blurred_image);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   B o r d e r I m a g e                                                     %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Function BorderImage takes an image and puts a border around it of a
%  particular color.  It allocates the memory necessary for the new Image
%  structure and returns a pointer to the new image.  Set the border and
%  highlight to the same color to get a solid border.
%
%  The format of the BorderImage routine is:
%
%      bordered_image=BorderImage(image,border_info,border_color)
%
%  A description of each parameter follows:
%
%    o bordered_image: Function BorderImage returns a pointer to the bordered
%      image.  A null image is returned if there is a a memory shortage.
%
%    o image: The address of a structure of type Image.
%
%    o border_info: Specifies a pointer to a XRectangle which defines the
%      border region.
%
%    o border_color: A pointer to a ColorPacket which contains the red,
%      green, and blue components of the border color.
%
%
*/
Image *BorderImage(Image *image, RectangleInfo *border_info, ColorPacket *border_color)
{
#define BorderImageText  "  Adding border to image...  "

  Image
    *bordered_image;

  register int
    x,
    y;

  register RunlengthPacket
    *p,
    *q;

  RunlengthPacket
    border;

  /*
    Initialize bordered image attributes.
  */
  bordered_image=CopyImage(image,image->columns+(border_info->width << 1),
    image->rows+(border_info->height << 1),False);
  if (bordered_image == (Image *) NULL)
    {
      Warning("Unable to border image","Memory allocation failed");
      return((Image *) NULL);
    }
  /*
    Initialize border color.
  */
  border.red=border_color->red;
  border.green=border_color->green;
  border.blue=border_color->blue;
  border.index=border_color->index;
  border.length=0;
  /*
    Copy image and put border around it.
  */
  q=bordered_image->pixels;
  for (y=0; y < border_info->height; y++)
    for (x=0; x < bordered_image->columns; x++)
      *q++=border;
  p=image->pixels;
  image->runlength=p->length+1;
  for (y=0; y < image->rows; y++)
  {
    /*
      Initialize scanline with border color.
    */
    for (x=0; x < border_info->width; x++)
      *q++=border;
    /*
      Transfer scanline.
    */
    for (x=0; x < image->columns; x++)
    {
      if (image->runlength != 0)
        image->runlength--;
      else
        {
          p++;
          image->runlength=p->length;
        }
      *q=(*p);
      q->length=0;
      q++;
    }
    x=0;
    while (x < (bordered_image->columns-image->columns-border_info->width))
    {
      *q++=border;
      x++;
    }
    ProgressMonitor(BorderImageText,y,image->rows);
  }
  for (y=(bordered_image->rows-image->rows-border_info->height-1); y >= 0; y--)
    for (x=0; x < bordered_image->columns; x++)
      *q++=border;
  return(bordered_image);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   C h o p I m a g e                                                         %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Function ChopImage creates a new image that is a subregion of an existing
%  one.  It allocates the memory necessary for the new Image structure and
%  returns a pointer to the new image.
%
%  The format of the ChopImage routine is:
%
%      chop_image=ChopImage(image,chop_info)
%
%  A description of each parameter follows:
%
%    o chop_image: Function ChopImage returns a pointer to the chop
%      image.  A null image is returned if there is a a memory shortage or
%      if the image width or height is zero.
%
%    o image: The address of a structure of type Image.
%
%    o chop_info: Specifies a pointer to a RectangleInfo which defines the
%      region of the image to crop.
%
%
*/
Image *ChopImage(Image *image, RectangleInfo *chop_info)
{
#define ChopImageText  "  Chopping image...  "

  Image
    *chopped_image;

  register int
    x,
    y;

  register RunlengthPacket
    *p,
    *q;

  unsigned int
    height;

  /*
    Check chop geometry.
  */
  if (((chop_info->x+(int) chop_info->width) < 0) ||
      ((chop_info->y+(int) chop_info->height) < 0) ||
      (chop_info->x > (int) image->columns) ||
      (chop_info->y > (int) image->rows))
    {
      Warning("Unable to chop image","geometry does not contain image");
      return((Image *) NULL);
    }
  if ((chop_info->x+(int) chop_info->width) > (int) image->columns)
    chop_info->width=(unsigned int) ((int) image->columns-chop_info->x);
  if ((chop_info->y+(int) chop_info->height) > (int) image->rows)
    chop_info->height=(unsigned int) ((int) image->rows-chop_info->y);
  if (chop_info->x < 0)
    {
      chop_info->width-=(unsigned int) (-chop_info->x);
      chop_info->x=0;
    }
  if (chop_info->y < 0)
    {
      chop_info->height-=(unsigned int) (-chop_info->y);
      chop_info->y=0;
    }
  /*
    Initialize chop image attributes.
  */
  chopped_image=CopyImage(image,image->columns-chop_info->width,
    image->rows-chop_info->height,False);
  if (chopped_image == (Image *) NULL)
    {
      Warning("Unable to chop image","Memory allocation failed");
      return((Image *) NULL);
    }
  /*
    Extract chop image.
  */
  p=image->pixels;
  image->runlength=p->length+1;
  q=chopped_image->pixels;
  for (y=0; y < chop_info->y; y++)
    for (x=0; x < image->columns; x++)
    {
      if (image->runlength != 0)
        image->runlength--;
      else
        {
          p++;
          image->runlength=p->length;
        }
      if ((x < chop_info->x) || (x >= (chop_info->x+chop_info->width)))
        {
          *q=(*p);
          q->length=0;
          q++;
        }
    }
  /*
    Skip pixels up to the chop image.
  */
  for (x=0; x < (chop_info->height*image->columns); x++)
    if (image->runlength != 0)
      image->runlength--;
    else
      {
        p++;
        image->runlength=p->length;
      }
  /*
    Extract chop image.
  */
  height=image->rows-(chop_info->y+chop_info->height);
  for (y=0; y < height; y++)
  {
    for (x=0; x < image->columns; x++)
    {
      if (image->runlength != 0)
        image->runlength--;
      else
        {
          p++;
          image->runlength=p->length;
        }
      if ((x < chop_info->x) || (x >= (chop_info->x+chop_info->width)))
        {
          *q=(*p);
          q->length=0;
          q++;
        }
    }
    ProgressMonitor(ChopImageText,y,height);
  }
  return(chopped_image);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   C l o s e I m a g e                                                       %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Function CloseImage closes a file associated with the image.  If the
%  filename prefix is '|', the file is a pipe and is closed with pclose.
%
%  The format of the CloseImage routine is:
%
%      CloseImage(image)
%
%  A description of each parameter follows:
%
%    o image: The address of a structure of type Image.
%
%
*/
void CloseImage(Image *image)
{
  /*
    Close image file.
  */
  if (image == (Image *) NULL)
    return;
  if (image->file == (FILE *) NULL)
    return;
  image->status=ferror(image->file);
  if (image->pipe)
    (void) pclose(image->file);
  else
    if ((image->file != stdin) && (image->file != stdout))
      (void) fclose(image->file);
  do
  {
    image->file=(FILE *) NULL;
    image=image->next;
  }
  while (image != (Image *) NULL);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   C o m m e n t I m a g e                                                   %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Function CommentImage initializes an image comment.  Optionally the
%  comment can include the image filename, type, width, height, or scene
%  number by embedding special format characters.  Embed %f for filename,
%  %m for magick, %w for width, %h for height, %s for scene number, or \n
%  for newline.  For example,
%
%     %f  %wx%h
%
%  produces an image comment of
%
%     bird.miff  512x480
%
%  for an image titled bird.miff and whose width is 512 and height is 480.
%
%  The format of the CommentImage routine is:
%
%      CommentImage(image,comments)
%
%  A description of each parameter follows:
%
%    o image: The address of a structure of type Image.
%
%    o comments: The address of a character string containing the comment
%      format.
%
%
*/
void CommentImage(Image *image, char *comments)
{
  register char
    *p,
    *q;

  unsigned int
    indirection,
    length;

  if (image->comments != (char *) NULL)
    free((char *) image->comments);
  image->comments=(char *) NULL;
  if (comments == (char *) NULL)
    return;
  indirection=(*comments == '@');
  if (indirection)
    {
      FILE
        *file;

      int
        c;

      /*
        Read comments from a file.
      */
      file=(FILE *) fopen(comments+1,"r");
      if (file == (FILE *) NULL)
        {
          Warning("Unable to read comments file",comments+1);
          return;
        }
      length=MaxTextLength;
      comments=(char *) malloc(length);
      for (q=comments; comments != (char *) NULL; q++)
      {
        c=fgetc(file);
        if (c == EOF)
          break;
        if ((q-comments+1) >= length)
          {
            *q='\0';
            length<<=1;
            comments=(char *) realloc((char *) comments,length);
            if (comments == (char *) NULL)
              break;
            q=comments+strlen(comments);
          }
        *q=(unsigned char) c;
      }
      (void) fclose(file);
      if (comments == (char *) NULL)
        {
          Warning("Unable to comments image","Memory allocation failed");
          return;
        }
      *q='\0';
    }
  /*
    Allocate and initialize image comments.
  */
  p=comments;
  length=strlen(comments)+MaxTextLength;
  image->comments=(char *) malloc(length);
  for (q=image->comments; image->comments != (char *) NULL; p++)
  {
    *q='\0';
    if (*p == '\0')
      break;
    if ((q-image->comments+MaxTextLength) >= length)
      {
        length<<=1;
        image->comments=(char *) realloc((char *) image->comments,length);
        if (image->comments == (char *) NULL)
          break;
        q=image->comments+strlen(image->comments);
      }
    /*
      Process formatting characters in comments.
    */
    if ((*p == '\\') && (*(p+1) == 'n'))
      {
        *q++='\n';
        p++;
        continue;
      }
    if (*p != '%')
      {
        *q++=(*p);
        continue;
      }
    p++;
    switch (*p)
    {
      case 'f':
      {
        register char
          *p;

        /*
          Label segment is the base of the filename.
        */
        p=image->filename+strlen(image->filename)-1;
        while ((p > image->filename) && (*(p-1) != '/'))
          p--;
        (void) strcpy(q,p);
        q+=strlen(p);
        break;
      }
      case 'h':
      {
        (void) sprintf(q,"%u",image->magick_rows);
        q=image->comments+strlen(image->comments);
        break;
      }
      case 'm':
      {
        (void) strcpy(q,image->magick);
        q+=strlen(image->magick);
        break;
      }
      case 's':
      {
        (void) sprintf(q,"%u",image->scene);
        q=image->comments+strlen(image->comments);
        break;
      }
      case 'w':
      {
        (void) sprintf(q,"%u",image->magick_columns);
        q=image->comments+strlen(image->comments);
        break;
      }
      default:
      {
        *q++='%';
        *q++=(*p);
        break;
      }
    }
  }
  if (image->comments == (char *) NULL)
    {
      Warning("Unable to comment image","Memory allocation failed");
      return;
    }
  *q='\0';
  if (indirection)
    free((char *) comments);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   C o m p r e s s C o l o r m a p                                           %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Function CompressColormap compresses an image colormap removing any
%  unused color entries.
%
%  The format of the CompressColormap routine is:
%
%      CompressColormap(image)
%
%  A description of each parameter follows:
%
%    o image: The address of a structure of type Image.
%
%
*/
void CompressColormap(Image *image)
{
  ColorPacket
    *colormap;

  int
    number_colors;

  register int
    i;

  register RunlengthPacket
    *p;

  register unsigned short
    index;

  /*
    Determine if colormap can be compressed.
  */
  if (image->class != PseudoClass)
    return;
  number_colors=image->colors;
  for (i=0; i < image->colors; i++)
    image->colormap[i].flags=False;
  image->colors=0;
  p=image->pixels;
  for (i=0; i < image->packets; i++)
  {
    if (!image->colormap[p->index].flags)
      {
        image->colormap[p->index].index=image->colors;
        image->colormap[p->index].flags=True;
        image->colors++;
      }
    p++;
  }
  if (image->colors == number_colors)
    return;  /* no unused entries */
  /*
    Compress colormap.
  */
  colormap=(ColorPacket *) malloc(image->colors*sizeof(ColorPacket));
  if (colormap == (ColorPacket *) NULL)
    {
      Warning("Unable to compress colormap","Memory allocation failed");
      image->colors=number_colors;
      return;
    }
  for (i=0; i < number_colors; i++)
    if (image->colormap[i].flags)
      {
        index=image->colormap[i].index;
        colormap[index].red=image->colormap[i].red;
        colormap[index].green=image->colormap[i].green;
        colormap[index].blue=image->colormap[i].blue;
      }
  /*
    Remap pixels.
  */
  p=image->pixels;
  for (i=0; i < image->packets; i++)
  {
    p->index=image->colormap[p->index].index;
    p++;
  }
  free((char *) image->colormap);
  image->colormap=colormap;
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   C o m p r e s s I m a g e                                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Function CompressImage compresses an image to the minimum number of
%  runlength-encoded packets.
%
%  The format of the CompressImage routine is:
%
%      CompressImage(image)
%
%  A description of each parameter follows:
%
%    o image: The address of a structure of type Image.
%
%
*/
void CompressImage(Image *image)
{
  register int
    i;

  register RunlengthPacket
    *p,
    *q;

  /*
    Compress image.
  */
  if (image == (Image *) NULL)
    return;
  p=image->pixels;
  image->runlength=p->length+1;
  image->packets=0;
  q=image->pixels;
  q->length=MaxRunlength;
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
          ((int) q->length < MaxRunlength))
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
          (p->blue == q->blue) && ((int) q->length < MaxRunlength))
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
  image->pixels=(RunlengthPacket *)
    realloc((char *) image->pixels,image->packets*sizeof(RunlengthPacket));
  /*
    Runlength-encode only if it takes up less space than no compression.
  */
  if (image->compression == RunlengthEncodedCompression)
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
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   C o m p o s i t e I m a g e                                               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Function CompositeImage returns the second image composited onto the
%  first at the specified offsets.
%
%  The format of the CompositeImage routine is:
%
%      CompositeImage(image,compose,composite_image,x_offset,y_offset)
%
%  A description of each parameter follows:
%
%    o image: The address of a structure of type Image.
%
%    o compose: Specifies an image composite operator.
%
%    o composite_image: The address of a structure of type Image.
%
%    o x_offset: An integer that specifies the column offset of the composited
%      image.
%
%    o y_offset: An integer that specifies the row offset of the composited
%      image.
%
%
*/
void CompositeImage(Image *image, unsigned int compose, Image *composite_image, int x_offset, int y_offset)
{
#define CompositeImageText  "  Compositing image...  "

  int
    blue,
    green,
    red;

  register int
    i,
    index,
    x,
    y;

  register RunlengthPacket
    *p,
    *q;

  /*
    Check composite geometry.
  */
  if (((x_offset+(int) image->columns) < 0) ||
      ((y_offset+(int) image->rows) < 0) ||
      (x_offset > (int) image->columns) || (y_offset > (int) image->rows))
    {
      Warning("Unable to composite image","geometry does not contain image");
      return;
    }
  /*
    Image must be uncompressed.
  */
  if (!UncompressImage(image))
    return;
  if (compose == ReplaceCompositeOp)
    {
      /*
        Promote image to DirectClass if colormaps differ.
      */
      if (image->class == PseudoClass)
        if (composite_image->class == DirectClass)
          image->class=DirectClass;
        else
          {
            if (image->signature == (char *) NULL)
              ColormapSignature(image);
            if (composite_image->signature == (char *) NULL)
              ColormapSignature(composite_image);
            if (strcmp(image->signature,composite_image->signature) != 0)
              image->class=DirectClass;
          }
      if (image->matte && !composite_image->matte)
        {
          p=composite_image->pixels;
          for (i=0; i < composite_image->packets; i++)
          {
            p->index=Opaque;
            p++;
          }
          composite_image->class=DirectClass;
          composite_image->matte=True;
        }
    }
  else
    {
      /*
        Initialize image matte data.
      */
      if (!image->matte)
        {
          q=image->pixels;
          red=q->red;
          green=q->green;
          blue=q->blue;
          for (i=0; i < image->packets; i++)
          {
            q->index=Opaque;
            q++;
          }
          image->class=DirectClass;
          image->matte=True;
        }
      if (!composite_image->matte)
        {
          p=composite_image->pixels;
          red=p->red;
          green=p->green;
          blue=p->blue;
          for (i=0; i < composite_image->packets; i++)
          {
            p->index=Opaque;
            if ((p->red == red) && (p->green == green) && (p->blue == blue))
              p->index=Transparent;
            p++;
          }
          composite_image->class=DirectClass;
          composite_image->matte=True;
        }
    }
  /*
    Initialize composited image.
  */
  p=composite_image->pixels;
  composite_image->runlength=p->length+1;
  for (y=0; y < composite_image->rows; y++)
  {
    if (((y_offset+y) < 0) || ((y_offset+y) >= image->rows))
      continue;
    q=image->pixels+(y_offset+y)*image->columns+x_offset;
    for (x=0; x < composite_image->columns; x++)
    {
      if (composite_image->runlength != 0)
        composite_image->runlength--;
      else
        {
          p++;
          composite_image->runlength=p->length;
        }
      if (((x_offset+x) < 0) || ((x_offset+x) >= image->columns))
        {
          q++;
          continue;
        }
      switch (compose)
      {
        case OverCompositeOp:
        default:
        {
          if (p->index == Transparent)
            {
              red=q->red;
              green=q->green;
              blue=q->blue;
              index=q->index;
            }
          else
            if (p->index == Opaque)
              {
                red=p->red;
                green=p->green;
                blue=p->blue;
                index=p->index;
              }
            else
              {
                red=(int) (p->red*Opaque+q->red*(Opaque-p->index))/Opaque;
                green=(int) (p->green*Opaque+q->green*(Opaque-p->index))/Opaque;
                blue=(int) (p->blue*Opaque+q->blue*(Opaque-p->index))/Opaque;
                index=(int) (p->index*Opaque+q->index*(Opaque-p->index))/Opaque;
              }
          break;
        }
        case InCompositeOp:
        {
          red=(int) (p->red*q->index)/Opaque;
          green=(int) (p->green*q->index)/Opaque;
          blue=(int) (p->blue*q->index)/Opaque;
          index=(int) (p->index*q->index)/Opaque;
          break;
        }
        case OutCompositeOp:
        {
          red=(int) (p->red*(Opaque-q->index))/Opaque;
          green=(int) (p->green*(Opaque-q->index))/Opaque;
          blue=(int) (p->blue*(Opaque-q->index))/Opaque;
          index=(int) (p->index*(Opaque-q->index))/Opaque;
          break;
        }
        case AtopCompositeOp:
        {
          red=(int) (p->red*q->index+q->red*(Opaque-p->index))/Opaque;
          green=(int) (p->green*q->index+q->green*(Opaque-p->index))/Opaque;
          blue=(int) (p->blue*q->index+q->blue*(Opaque-p->index))/Opaque;
          index=(int) (p->index*q->index+q->index*(Opaque-p->index))/Opaque;
          break;
        }
        case XorCompositeOp:
        {
          red=(int) (p->red*(Opaque-q->index)+q->red*(Opaque-p->index))/Opaque;
          green=(int) (p->green*(Opaque-q->index)+q->green*(Opaque-p->index))/
            Opaque;
          blue=(int) (p->blue*(Opaque-q->index)+q->blue*(Opaque-p->index))/
            Opaque;
          index=(int) (p->index*(Opaque-q->index)+q->index*(Opaque-p->index))/
            Opaque;
          break;
        }
        case PlusCompositeOp:
        {
          red=(int) p->red+(int) q->red;
          green=(int) p->green+(int) q->green;
          blue=(int) p->blue+(int) q->blue;
          index=(int) p->index+(int) q->index;
          break;
        }
        case MinusCompositeOp:
        {
          red=(int) p->red-(int) q->red;
          green=(int) p->green-(int) q->green;
          blue=(int) p->blue-(int) q->blue;
          index=Opaque;
          break;
        }
        case AddCompositeOp:
        {
          red=(int) p->red+(int) q->red;
          if (red > Opaque)
            red-=(Opaque+1);
          green=(int) p->green+(int) q->green;
          if (green > Opaque)
            green-=(Opaque+1);
          blue=(int) p->blue+(int) q->blue;
          if (blue > Opaque)
            blue-=(Opaque+1);
          index=(int) p->index+(int) q->index;
          if (index > Opaque)
            index-=(Opaque+1);
          break;
        }
        case SubtractCompositeOp:
        {
          red=(int) p->red-(int) q->red;
          if (red < 0)
            red+=(Opaque+1);
          green=(int) p->green-(int) q->green;
          if (green < 0)
            green+=(Opaque+1);
          blue=(int) p->blue-(int) q->blue;
          if (blue < 0)
            blue+=(Opaque+1);
          index=(int) p->index-(int) q->index;
          if (index < 0)
            index+=(Opaque+1);
          break;
        }
        case DifferenceCompositeOp:
        {
          red=AbsoluteValue((int) p->red-(int) q->red);
          green=AbsoluteValue((int) p->green-(int) q->green);
          blue=AbsoluteValue((int) p->blue-(int) q->blue);
          index=AbsoluteValue((int) p->index-(int) q->index);
          break;
        }
        case ReplaceCompositeOp:
        {
          red=p->red;
          green=p->green;
          blue=p->blue;
          index=p->index;
          break;
        }
        case MatteReplaceCompositeOp:
        {
          red=q->red;
          green=q->green;
          blue=q->blue;
          index=p->index;
          break;
        }
        case BlendCompositeOp:
        {
          red=(int) (p->red*p->index+q->red*q->index)/Opaque;
          green=(int) (p->green*p->index+q->green*q->index)/Opaque;
          blue=(int) (p->blue*p->index+q->blue*q->index)/Opaque;
          index=Opaque;
          break;
        }
      }
      if (red > MaxRGB)
        q->red=MaxRGB;
      else
        if (red < 0)
          q->red=0;
        else
          q->red=red;
      if (green > MaxRGB)
        q->green=MaxRGB;
      else
        if (green < 0)
          q->green=0;
        else
          q->green=green;
      if (blue > MaxRGB)
        q->blue=MaxRGB;
      else
        if (blue < 0)
          q->blue=0;
        else
          q->blue=blue;
      if (index > Opaque)
        q->index=Opaque;
      else
        if (index < Transparent)
          q->index=Transparent;
        else
          q->index=index;
      q->length=0;
      q++;
    }
    ProgressMonitor(CompositeImageText,y,composite_image->rows);
  }
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%     C o n t r a s t I m a g e                                               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Function ContrastImage enhances the intensity differences between the
%  lighter and darker elements of the image.
%
%  The format of the ContrastImage routine is:
%
%      ContrastImage(image,sharpen)
%
%  A description of each parameter follows:
%
%    o image: The address of a structure of type Image;  returned from
%      ReadImage.
%
%    o sharpen: If True, the intensity is increased otherwise it is
%      decreased.
%
%
*/

static void HSVTransform(double hue, double saturation, double brightness, Quantum *red, Quantum *green, Quantum *blue)
{
  double
    b,
    g,
    r;

  /*
    Convert HSV to RGB colorspace.
  */
  r=brightness;
  g=brightness;
  b=brightness;
  if ((hue != -1.0) && (saturation != 0.0))
    {
      double
        f,
        j,
        k,
        l,
        v;

      int
        i;

      if (hue == 360.0)
        hue=0.0;
      hue=hue/60.0;
      i=(int)floor(hue);
      if (i < 0)
        i=0;
      f=hue-i;
      j=brightness*(1.0-saturation);
      k=brightness*(1.0-(saturation*f));
      l=brightness*(1.0-(saturation*(1.0-f)));
      v=brightness;
      switch (i)
      {
        case 0:  r=v;  g=l;  b=j;  break;
        case 1:  r=k;  g=v;  b=j;  break;
        case 2:  r=j;  g=v;  b=l;  break;
        case 3:  r=j;  g=k;  b=v;  break;
        case 4:  r=l;  g=j;  b=v;  break;
        case 5:  r=v;  g=j;  b=k;  break;
      }
    }
  *red=(Quantum) floor((r*(double) MaxRGB)+0.5);
  *green=(Quantum) floor((g*(double) MaxRGB)+0.5);
  *blue=(Quantum) floor((b*(double) MaxRGB)+0.5);
}

static void TransformHSV(unsigned int red, unsigned int green, unsigned int blue, double *hue, double *saturation, double *brightness)
{
  double
    b,
    minimum,
    g,
    r;

  /*
    Convert RGB to HSV colorspace.
  */
  *hue=(-1.0);
  *saturation=0.0;
  r=(double) red/(double) MaxRGB;
  g=(double) green/(double) MaxRGB;
  b=(double) blue/(double) MaxRGB;
  if (r >= g)
    {
      if (r >= b)
        *brightness=r;
      else
        *brightness=b;
    }
  else
    if (g >= b)
      *brightness=g;
    else
      *brightness=b;
  if (r <= g)
    {
      if (r <= b)
        minimum=r;
      else
        minimum=b;
    }
  else
    if (g <= b)
      minimum=g;
    else
      minimum=b;
  if (*brightness != 0.0)
    *saturation=(*brightness-minimum)/(*brightness);
  if (*saturation != 0.0)
    {
      if (r == *brightness)
        *hue=(g-b)/(*brightness-minimum);
      else
        if (g == *brightness)
          *hue=2.0+(b-r)/(*brightness-minimum);
        else
          if (b == *brightness)
            *hue=4.0+(r-g)/(*brightness-minimum);
      *hue=(*hue)*60.0;
      if (*hue < 0.0)
        *hue+=360.0;
    }
}

static void Contrast(int sign, Quantum *red, Quantum *green, Quantum *blue)
{
  double
    brightness,
    hue,
    saturation,
    theta;

  /*
    Enhance contrast: dark color become darker, light color become lighter.
  */
  TransformHSV(*red,*green,*blue,&hue,&saturation,&brightness);
  theta=(brightness-0.5)*M_PI;
  brightness+=(((((sin(theta)+1.0))*0.5)-brightness)*sign)*0.5;
  if (brightness > 1.0)
    brightness=1.0;
  else
    if (brightness < 0)
      brightness=0.0;
  HSVTransform(hue,saturation,brightness,red,green,blue);
}

void ContrastImage(Image *image, unsigned int sharpen)
{
#define DullContrastImageText  "  Dulling image contrast...  "
#define SharpenContrastImageText  "  Sharpening image contrast...  "

  int
    sign;

  register int
    i;

  register RunlengthPacket
    *p;

  sign=sharpen ? 1 : -1;
  switch (image->class)
  {
    case DirectClass:
    {
      /*
        Contrast enhance DirectClass image.
      */
      p=image->pixels;
      for (i=0; i < image->packets; i++)
      {
        Contrast(sign,&p->red,&p->green,&p->blue);
        p++;
        if (QuantumTick(i,image))
          if (sharpen)
            ProgressMonitor(SharpenContrastImageText,i,image->packets);
          else
            ProgressMonitor(DullContrastImageText,i,image->packets);
      }
      break;
    }
    case PseudoClass:
    {
      /*
        Contrast enhance PseudoClass image.
      */
      for (i=0; i < image->colors; i++)
        Contrast(sign,&image->colormap[i].red,&image->colormap[i].green,
          &image->colormap[i].blue);
      SyncImage(image);
      break;
    }
  }
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   C o p y I m a g e                                                         %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Function CopyImage returns a copy of all fields of the input image.  The
%  the pixel memory is allocated but the pixel data is not copied.
%
%  The format of the CopyImage routine is:
%
%      copy_image=CopyImage(image,columns,rows,copy_pixels)
%
%  A description of each parameter follows:
%
%    o copy_image: Function CopyImage returns a pointer to the image after
%      copying.  A null image is returned if there is a memory shortage.
%
%    o image: The address of a structure of type Image.
%
%    o columns: An integer that specifies the number of columns in the copied
%      image.
%
%    o rows: An integer that specifies the number of rows in the copied
%      image.
%
%    o copy_pixels: Specifies whether the pixel data is copied.  Must be
%      either True or False;
%
%
*/
Image *CopyImage(Image *image, unsigned int columns, unsigned int rows, unsigned int copy_pixels)
{
  Image
    *copy_image;

  register int
    i;

  /*
    Allocate image structure.
  */
  copy_image=(Image *) malloc(sizeof(Image));
  if (copy_image == (Image *) NULL)
    return((Image *) NULL);
  *copy_image=(*image);
  if (image->comments != (char *) NULL)
    {
      /*
        Allocate and copy the image comments.
      */
      copy_image->comments=(char *)
        malloc((unsigned int) strlen(image->comments)+1);
      if (copy_image->comments == (char *) NULL)
        return((Image *) NULL);
      (void) strcpy(copy_image->comments,image->comments);
    }
  if (image->label != (char *) NULL)
    {
      /*
        Allocate and copy the image label.
      */
      copy_image->label=(char *) malloc((unsigned int) strlen(image->label)+1);
      if (copy_image->label == (char *) NULL)
        return((Image *) NULL);
      (void) strcpy(copy_image->label,image->label);
    }
  copy_image->columns=columns;
  copy_image->rows=rows;
  copy_image->montage=(char *) NULL;
  copy_image->directory=(char *) NULL;
  if (image->colormap != (ColorPacket *) NULL)
    {
      /*
        Allocate and copy the image colormap.
      */
      copy_image->colormap=(ColorPacket *)
        malloc(image->colors*sizeof(ColorPacket));
      if (copy_image->colormap == (ColorPacket *) NULL)
        return((Image *) NULL);
      for (i=0; i < image->colors; i++)
        copy_image->colormap[i]=image->colormap[i];
    }
  if (image->signature != (char *) NULL)
    {
      /*
        Allocate and copy the image signature.
      */
      copy_image->signature=(char *)
        malloc((unsigned int) strlen(image->signature)+1);
      if (copy_image->signature == (char *) NULL)
        return((Image *) NULL);
      (void) strcpy(copy_image->signature,image->signature);
    }
  /*
    Allocate the image pixels.
  */
  if (copy_pixels)
    copy_image->pixels=(RunlengthPacket *)
      malloc((unsigned int) image->packets*sizeof(RunlengthPacket));
  else
    {
      copy_image->packets=copy_image->columns*copy_image->rows;
      copy_image->pixels=(RunlengthPacket *)
        malloc((unsigned int) copy_image->packets*sizeof(RunlengthPacket));
    }
  if (copy_image->pixels == (RunlengthPacket *) NULL)
    return((Image *) NULL);
  if (copy_pixels)
    {
      register RunlengthPacket
        *p,
        *q;

      /*
        Copy the image pixels.
      */
      p=image->pixels;
      q=copy_image->pixels;
      for (i=0; i < image->packets; i++)
      {
        *q=(*p);
        p++;
        q++;
      }
    }
  if (image->orphan)
    {
      copy_image->file=(FILE *) NULL;
      copy_image->previous=(Image *) NULL;
      copy_image->next=(Image *) NULL;
    }
  else
    {
      /*
        Link image into image list.
      */
      if (copy_image->previous != (Image *) NULL)
        copy_image->previous->next=copy_image;
      if (copy_image->next != (Image *) NULL)
        copy_image->next->previous=copy_image;
    }
  copy_image->orphan=False;
  return(copy_image);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   C r o p I m a g e                                                         %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Function CropImage creates a new image that is a subregion of an existing
%  one.  It allocates the memory necessary for the new Image structure and
%  returns a pointer to the new image.  This routine is optimized to perserve
%  the runlength encoding.  That is, the cropped image will always use less
%  memory than the original.
%
%  The format of the CropImage routine is:
%
%      cropped_image=CropImage(image,crop_info)
%
%  A description of each parameter follows:
%
%    o cropped_image: Function CropImage returns a pointer to the cropped
%      image.  A null image is returned if there is a a memory shortage or
%      if the image width or height is zero.
%
%    o image: The address of a structure of type Image.
%
%    o crop_info: Specifies a pointer to a RectangleInfo which defines the
%      region of the image to crop.
%
%
*/
Image *CropImage(Image *image, RectangleInfo *crop_info)
{
#define CropImageText  "  Cropping image...  "

  Image
    *cropped_image;

  register int
    x,
    y;

  register RunlengthPacket
    *p,
    *q;

  /*
    Check crop geometry.
  */
  if (((crop_info->x+(int) crop_info->width) < 0) ||
      ((crop_info->y+(int) crop_info->height) < 0) ||
      (crop_info->x > (int) image->columns) ||
      (crop_info->y > (int) image->rows))
    {
      Warning("Unable to crop image","geometry does not contain image");
      return((Image *) NULL);
    }
  if ((crop_info->x+(int) crop_info->width) > (int) image->columns)
    crop_info->width=(unsigned int) ((int) image->columns-crop_info->x);
  if ((crop_info->y+(int) crop_info->height) > (int) image->rows)
    crop_info->height=(unsigned int) ((int) image->rows-crop_info->y);
  if (crop_info->x < 0)
    {
      crop_info->width-=(unsigned int) (-crop_info->x);
      crop_info->x=0;
    }
  if (crop_info->y < 0)
    {
      crop_info->height-=(unsigned int) (-crop_info->y);
      crop_info->y=0;
    }
  if ((crop_info->width == 0) && (crop_info->height == 0))
    {
      register int
        i;

      RunlengthPacket
        corners[4];

      /*
        Set bounding box to the image dimensions.
      */
      crop_info->width=0;
      crop_info->height=0;
      crop_info->x=image->columns;
      crop_info->y=image->rows;
      p=image->pixels;
      image->runlength=p->length+1;
      corners[0]=(*p);
      for (i=1; i <= (image->rows*image->columns); i++)
      {
        if (image->runlength != 0)
          image->runlength--;
        else
          {
            p++;
            image->runlength=p->length;
          }
        if (i == image->columns)
          corners[1]=(*p);
        if (i == (image->rows*image->columns-image->columns+1))
          corners[2]=(*p);
        if (i == (image->rows*image->columns))
          corners[3]=(*p);
      }
      p=image->pixels;
      image->runlength=p->length+1;
      for (y=0; y < image->rows; y++)
      {
        for (x=0; x < image->columns; x++)
        {
          if (image->runlength != 0)
            image->runlength--;
          else
            {
              p++;
              image->runlength=p->length;
            }
          if ((p->red != corners[0].red) ||
              (p->green != corners[0].green) ||
              (p->blue != corners[0].blue))
            if (x < crop_info->x)
              crop_info->x=x;
          if ((p->red != corners[1].red) ||
              (p->green != corners[1].green) ||
              (p->blue != corners[1].blue))
            if (x > crop_info->width)
              crop_info->width=x;
          if ((p->red != corners[0].red) ||
              (p->green != corners[0].green) ||
              (p->blue != corners[0].blue))
            if (y < crop_info->y)
              crop_info->y=y;
          if ((p->red != corners[2].red) ||
              (p->green != corners[2].green) ||
              (p->blue != corners[2].blue))
            if (y > crop_info->height)
              crop_info->height=y;
        }
      }
      crop_info->width-=crop_info->x-1;
      crop_info->height-=crop_info->y-1;
    }
  if ((crop_info->width == 0) || (crop_info->height == 0))
    {
      Warning("Unable to crop image","geometry dimensions are zero");
      return((Image *) NULL);
    }
  if ((crop_info->width == image->columns) &&
      (crop_info->height == image->rows) && (crop_info->x == 0) &&
      (crop_info->y == 0))
    return((Image *) NULL);
  /*
    Initialize cropped image attributes.
  */
  cropped_image=CopyImage(image,crop_info->width,crop_info->height,True);
  if (cropped_image == (Image *) NULL)
    {
      Warning("Unable to crop image","Memory allocation failed");
      return((Image *) NULL);
    }
  /*
    Skip pixels up to the cropped image.
  */
  p=image->pixels;
  image->runlength=p->length+1;
  for (x=0; x < (crop_info->y*image->columns+crop_info->x); x++)
    if (image->runlength != 0)
      image->runlength--;
    else
      {
        p++;
        image->runlength=p->length;
      }
  /*
    Extract cropped image.
  */
  cropped_image->packets=0;
  q=cropped_image->pixels;
  q->red=0;
  q->green=0;
  q->blue=0;
  q->index=0;
  q->length=MaxRunlength;
  for (y=0; y < (cropped_image->rows-1); y++)
  {
    /*
      Transfer scanline.
    */
    for (x=0; x < cropped_image->columns; x++)
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
          ((int) q->length < MaxRunlength))
        q->length++;
      else
        {
          if (cropped_image->packets != 0)
            q++;
          cropped_image->packets++;
          *q=(*p);
          q->length=0;
        }
    }
    /*
      Skip to next scanline.
    */
    for (x=0; x < (image->columns-cropped_image->columns); x++)
      if (image->runlength != 0)
        image->runlength--;
      else
        {
          p++;
          image->runlength=p->length;
        }
    ProgressMonitor(CropImageText,y,cropped_image->rows-1);
  }
  /*
    Transfer last scanline.
  */
  for (x=0; x < cropped_image->columns; x++)
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
        ((int) q->length < MaxRunlength))
      q->length++;
    else
      {
        if (cropped_image->packets != 0)
          q++;
        cropped_image->packets++;
        *q=(*p);
        q->length=0;
      }
  }
  cropped_image->pixels=(RunlengthPacket *) realloc((char *)
    cropped_image->pixels,cropped_image->packets*sizeof(RunlengthPacket));
  return(cropped_image);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   D e s c r i b e I m a g e                                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Function DescribeImage describes an image by printing its attributes to
%  stderr.
%
%  The format of the DescribeImage routine is:
%
%      DescribeImage(image,file,verbose)
%
%  A description of each parameter follows:
%
%    o image: The address of a structure of type Image.
%
%    o file: send the image attributes to this file.
%
%    o verbose: an unsigned value other than zero prints detailed information
%      about the image.
%
%
*/
void DescribeImage(Image *image, FILE *file, unsigned int verbose)
{
  Image
    *p;

  unsigned int
    count;

  if (!verbose)
    {
      /*
        Display detailed info about the image.
      */
      if (*image->magick_filename != '\0')
        if (strcmp(image->magick_filename,image->filename) != 0)
          (void) fprintf(file,"%s=>",image->magick_filename);
       if ((image->previous == (Image *) NULL) &&
           (image->next == (Image *) NULL) && (image->scene == 0))
        (void) fprintf(file,"%s ",image->filename);
      else
        (void) fprintf(file,"%s[%u] ",image->filename,image->scene);
      if ((image->magick_columns != 0) || (image->magick_rows != 0))
        if ((image->magick_columns != image->columns) ||
            (image->magick_rows != image->rows))
          (void) fprintf(file,"%ux%u=>",image->magick_columns,
            image->magick_rows);
      (void) fprintf(file,"%ux%u ",image->columns,image->rows);
      if (image->class == DirectClass)
        {
          (void) fprintf(file,"DirectClass ");
          if (image->total_colors != 0)
            (void) fprintf(file,"%luc ",image->total_colors);
        }
      else
        if (image->total_colors <= image->colors)
          (void) fprintf(file,"PseudoClass %uc ",image->colors);
        else
          {
            (void) fprintf(file,"PseudoClass %lu=>%uc ",image->total_colors,
              image->colors);
            (void) fprintf(file,"%u/%.6f/%.6fe ",image->mean_error_per_pixel,
              image->normalized_mean_error,image->normalized_maximum_error);
          }
      if (image->filesize != 0)
        (void) fprintf(file,"%ldb ",image->filesize);
      (void) fprintf(file,"%s %lds\n",image->magick,time((time_t *) NULL)-
        image->magick_time+1);
      return;
    }
  /*
    Display verbose info about the image.
  */
  (void) fprintf(file,"Image: %s\n",image->filename);
  if (image->class == DirectClass)
    (void) fprintf(file,"  class: DirectClass\n");
  else
    (void) fprintf(file,"  class: PseudoClass\n");
  if (image->class == DirectClass)
    {
      if (image->total_colors > 0)
        (void) fprintf(file,"  colors: %lu\n",image->total_colors);
    }
  else
    if (image->total_colors <= image->colors)
      (void) fprintf(file,"  colors: %u\n",image->colors);
    else
      (void) fprintf(file,"  colors: %lu=>%u\n",image->total_colors,
        image->colors);
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
      p=image->colormap;
      for (i=0; i < image->colors; i++)
      {
        (void) fprintf(file,"    %d: (%3d,%3d,%3d)  #%02x%02x%02x",
          i,p->red,p->green,p->blue,(unsigned int) p->red,
          (unsigned int) p->green,(unsigned int) p->blue);
        for (q=ColorList; q->name != (char *) NULL; q++)
          if ((DownScale(p->red) == q->red) &&
              (DownScale(p->green) == q->green) &&
              (DownScale(p->blue) == q->blue))
            (void) fprintf(file,"  %s",q->name);
        (void) fprintf(file,"\n");
        p++;
      }
    }
  if (image->signature != (char *) NULL)
    (void) fprintf(file,"  signature: %s\n",image->signature);
  if (image->matte)
    (void) fprintf(file,"  matte: True\n");
  else
    (void) fprintf(file,"  matte: False\n");
  if (image->packets < (image->columns*image->rows))
    (void) fprintf(file,"  runlength packets: %u of %u\n",image->packets,
      image->columns*image->rows);
  (void) fprintf(file,"  geometry: %ux%u\n",image->columns,image->rows);
  (void) fprintf(file,"  depth: %u\n",image->depth);
  if (image->filesize != 0)
    (void) fprintf(file,"  bytes: %ld\n",image->filesize);
  if (image->interlace)
    (void) fprintf(file,"  interlaced: True\n");
  else
    (void) fprintf(file,"  interlaced: False\n");
  (void) fprintf(file,"  format: %s\n",image->magick);
  p=image;
  while (p->previous != (Image *) NULL)
    p=p->previous;
  for (count=1; p->next != (Image *) NULL; count++)
    p=p->next;
  if (count > 1)
    (void) fprintf(file,"  scene: %u of %u\n",image->scene,count);
  else
    if (image->scene != 0)
      (void) fprintf(file,"  scene: %u\n",image->scene);
  if (image->montage != (char *) NULL)
    (void) fprintf(file,"  montage: %s\n",image->montage);
  if (image->directory != (char *) NULL)
    (void) fprintf(file,"  directory:\n\n%s\n",image->directory);
  if (image->label != (char *) NULL)
    (void) fprintf(file,"  label: %s\n",image->label);
  if (image->comments != (char *) NULL)
    (void) fprintf(file,"  comments:\n%s\n",image->comments);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%     D e s p e c k l e I m a g e                                             %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Function DespeckleImage creates a new image that is a copy of an existing
%  one with the speckle noise minified.  It uses the eight hull algorithm
%  described in Applied Optics, Vol. 24, No. 10, 15 May 1985, "Geometric filter
%  for Speckle Reduction", by Thomas R Crimmins.  Each pixel in the image is
%  replaced by one of its eight of its surrounding pixels using a polarity and
%  negative hull function.  DespeckleImage allocates the memory necessary for
%  the new Image structure and returns a pointer to the new image.
%
%  The format of the DespeckleImage routine is:
%
%      despeckled_image=DespeckleImage(image)
%
%  A description of each parameter follows:
%
%    o despeckled_image: Function DespeckleImage returns a pointer to the image
%      after it is despeckled.  A null image is returned if there is a memory
%      shortage.
%
%    o image: The address of a structure of type Image;  returned from
%      ReadImage.
%
%
*/

static void Hull(int x_offset, int y_offset, int polarity, unsigned int columns, unsigned int rows, Quantum *f, Quantum *g)
{
  int
    y;

  register int
    x;

  register Quantum
    *p,
    *q,
    *r,
    *s;

  Quantum
    v;

  p=f+(columns+2);
  q=g+(columns+2);
  r=p+(y_offset*((int) columns+2)+x_offset);
  for (y=0; y < rows; y++)
  {
    p++;
    q++;
    r++;
    if (polarity > 0)
      for (x=0; x < columns; x++)
      {
        v=(*p);
        if (*r > v)
          v++;
        *q=v;
        p++;
        q++;
        r++;
      }
    else
      for (x=0; x < columns; x++)
      {
        v=(*p);
        if (v > (Quantum) (*r+1))
          v--;
        *q=v;
        p++;
        q++;
        r++;
      }
    p++;
    q++;
    r++;
  }
  p=f+(columns+2);
  q=g+(columns+2);
  r=q+(y_offset*((int) columns+2)+x_offset);
  s=q-(y_offset*((int) columns+2)+x_offset);
  for (y=0; y < rows; y++)
  {
    p++;
    q++;
    r++;
    s++;
    if (polarity > 0)
      for (x=0; x < columns; x++)
      {
        v=(*q);
        if (((Quantum) (*s+1) > v) && (*r > v))
          v++;
        *p=v;
        p++;
        q++;
        r++;
        s++;
      }
    else
      for (x=0; x < columns; x++)
      {
        v=(*q);
        if (((Quantum) (*s+1) < v) && (*r < v))
          v--;
        *p=v;
        p++;
        q++;
        r++;
        s++;
      }
    p++;
    q++;
    r++;
    s++;
  }
}

Image *DespeckleImage(Image *image)
{
#define DespeckleImageText  "  Despeckling image...  "

  Image
    *despeckled_image;

  int
    x;

  Quantum
    *blue_channel,
    *buffer,
    *green_channel,
    *matte_channel,
    *red_channel;

  register int
    i,
    j;

  register RunlengthPacket
    *p,
    *q;

  static int
    X[4]= {0, 1, 1,-1},
    Y[4]= {1, 0, 1, 1};

  unsigned int
    packets;

  /*
    Allocate despeckled image.
  */
  despeckled_image=CopyImage(image,image->columns,image->rows,False);
  if (despeckled_image == (Image *) NULL)
    {
      Warning("Unable to despeckle image","Memory allocation failed");
      return((Image *) NULL);
    }
  despeckled_image->class=DirectClass;
  /*
    Allocate image buffers.
  */
  packets=(image->columns+2)*(image->rows+2);
  red_channel=(Quantum *) malloc(packets*sizeof(Quantum));
  green_channel=(Quantum *) malloc(packets*sizeof(Quantum));
  blue_channel=(Quantum *) malloc(packets*sizeof(Quantum));
  matte_channel=(Quantum *) malloc(packets*sizeof(Quantum));
  buffer=(Quantum *) malloc(packets*sizeof(Quantum));
  if ((red_channel == (Quantum *) NULL) ||
      (green_channel == (Quantum *) NULL) ||
      (blue_channel == (Quantum *) NULL) ||
      (matte_channel == (Quantum *) NULL) ||
      (buffer == (Quantum *) NULL) || !UncompressImage(image))
    {
      Warning("Unable to despeckle image","Memory allocation failed");
      DestroyImage(despeckled_image);
      return((Image *) NULL);
    }
  /*
    Zero image buffers.
  */
  for (i=0; i < packets; i++)
  {
    red_channel[i]=0;
    green_channel[i]=0;
    blue_channel[i]=0;
    matte_channel[i]=0;
    buffer[i]=0;
  }
  /*
    Copy image pixels to color component buffers
  */
  x=image->columns+2;
  p=image->pixels;
  for (j=0; j < image->rows; j++)
  {
    x++;
    for (i=0; i < image->columns; i++)
    {
      red_channel[x]=p->red;
      green_channel[x]=p->green;
      blue_channel[x]=p->blue;
      matte_channel[x]=p->index;
      x++;
      p++;
    }
    x++;
  }
  /*
    Reduce speckle in red channel.
  */
  for (i=0; i < 4; i++)
  {
    ProgressMonitor(DespeckleImageText,i,12);
    Hull(X[i],Y[i],1,image->columns,image->rows,red_channel,buffer);
    Hull(-X[i],-Y[i],1,image->columns,image->rows,red_channel,buffer);
    Hull(-X[i],-Y[i],-1,image->columns,image->rows,red_channel,buffer);
    Hull(X[i],Y[i],-1,image->columns,image->rows,red_channel,buffer);
  }
  /*
    Reduce speckle in green channel.
  */
  for (i=0; i < packets; i++)
    buffer[i]=0;
  for (i=0; i < 4; i++)
  {
    ProgressMonitor(DespeckleImageText,i+4,12);
    Hull(X[i],Y[i],1,image->columns,image->rows,green_channel,buffer);
    Hull(-X[i],-Y[i],1,image->columns,image->rows,green_channel,buffer);
    Hull(-X[i],-Y[i],-1,image->columns,image->rows,green_channel,buffer);
    Hull(X[i],Y[i],-1,image->columns,image->rows,green_channel,buffer);
  }
  /*
    Reduce speckle in blue channel.
  */
  for (i=0; i < packets; i++)
    buffer[i]=0;
  for (i=0; i < 4; i++)
  {
    ProgressMonitor(DespeckleImageText,i+8,12);
    Hull(X[i],Y[i],1,image->columns,image->rows,blue_channel,buffer);
    Hull(-X[i],-Y[i],1,image->columns,image->rows,blue_channel,buffer);
    Hull(-X[i],-Y[i],-1,image->columns,image->rows,blue_channel,buffer);
    Hull(X[i],Y[i],-1,image->columns,image->rows,blue_channel,buffer);
  }
  /*
    Copy color component buffers to despeckled image.
  */
  x=image->columns+2;
  q=despeckled_image->pixels;
  for (j=0; j < image->rows; j++)
  {
    x++;
    for (i=0; i < image->columns; i++)
    {
      q->red=red_channel[x];
      q->green=green_channel[x];
      q->blue=blue_channel[x];
      q->index=matte_channel[x];
      q->length=0;
      q++;
      x++;
    }
    x++;
  }
  /*
    Free memory.
  */
  free((char *) buffer);
  free((char *) blue_channel);
  free((char *) green_channel);
  free((char *) red_channel);
  return(despeckled_image);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   D e s t r o y I m a g e                                                   %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Function DestroyImage deallocates memory associated with an image.
%
%  The format of the DestroyImage routine is:
%
%      DestroyImage(image)
%
%  A description of each parameter follows:
%
%    o image: The address of a structure of type Image.
%
%
*/
void DestroyImage(Image *image)
{
  if (image == (Image *) NULL)
    return;
  /*
    Close image.
  */
  CloseImage(image);
  /*
    Deallocate the image comments.
  */
  if (image->comments != (char *) NULL)
    free((char *) image->comments);
  /*
    Deallocate the image label.
  */
  if (image->label != (char *) NULL)
    free((char *) image->label);
  /*
    Deallocate the image montage directory.
  */
  if (image->montage != (char *) NULL)
    free((char *) image->montage);
  if (image->directory != (char *) NULL)
    free((char *) image->directory);
  /*
    Deallocate the image colormap.
  */
  if (image->colormap != (ColorPacket *) NULL)
    free((char *) image->colormap);
  /*
    Deallocate the image signature.
  */
  if (image->signature != (char *) NULL)
    free((char *) image->signature);
  /*
    Deallocate the image pixels.
  */
  if (image->pixels != (RunlengthPacket *) NULL)
    free((char *) image->pixels);
  if (image->packed_pixels != (unsigned char *) NULL)
    free((char *) image->packed_pixels);
  /*
    Deallocate the image structure.
  */
  free((char *) image);
  image=(Image *) NULL;
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   D e s t r o y I m a g e s                                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Function DestroyImages deallocates memory associated with a linked list
%  of images.
%
%  The format of the DestroyImages routine is:
%
%      DestroyImages(image)
%
%  A description of each parameter follows:
%
%    o image: The address of a structure of type Image.
%
%
*/
void DestroyImages(Image *image)
{
  Image
    *next_image;

  if (image == (Image *) NULL)
    return;
  /*
    Proceed to the top of the image list.
  */
  while (image->previous != (Image *) NULL)
    image=image->previous;
  do
  {
    /*
      Destroy this image.
    */
    next_image=image->next;
    DestroyImage(image);
    image=next_image;
  } while (image != (Image *) NULL);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   D r a w I m a g e                                                         %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Function DrawImage draws a primitive (line, rectangle, ellipse) on the
%  image.
%
%  The format of the DrawImage routine is:
%
%      DrawImage(image,annotate_info)
%
%  A description of each parameter follows:
%
%    o image: The address of a structure of type Image.
%
%    o annotate_info: The address of a DrawInfo structure.
%
%
*/

static unsigned int InsidePrimitive(PrimitiveInfo *primitive_info, int x, int y)
{
  register unsigned int
    inside;

  register PrimitiveInfo
    *p,
    *q;

  p=primitive_info;
  while (p->primitive != UndefinedPrimitive)
  {
    q=p+p->coordinates-1;
    switch (p->primitive)
    {
      case FillRectanglePrimitive:
      default:
      {
        inside=(x >= p->x) && (y >= p->y) && (x <= q->x) && (y <= q->y);
        p+=p->coordinates;
        break;
      }
      case FillEllipsePrimitive:
      {
        inside=(((p->y-y)*(p->y-y))+((p->x-x)*(p->x-x))) <=
          (((p->y-q->y)*(p->y-q->y))+((p->x-q->x)*(p->x-q->x)));
        p+=p->coordinates;
        break;
      }
      case FillPolygonPrimitive:
      {
        int
          crossing,
          crossings;

        crossings=0;
        if ((q->y >= y) != (p->y >= y))
          {
            crossing=q->x >= x;
            if (crossing != (p->x >= x))
              crossings+=(q->x-(q->y-y)*(p->x-q->x)/(p->y-q->y)) >= x;
            else
              if (crossing)
                crossings++;
          }
        for (p++; p <= q; p++)
        {
          if ((p-1)->y >= y)
            {
              while ((p <= q) && (p->y >= y))
                p++;
              if (p > q)
                break;
              crossing=(p-1)->x >= x;
              if (crossing != (p->x >= x))
                crossings+=
                  ((p-1)->x-((p-1)->y-y)*(p->x-(p-1)->x)/(p->y-(p-1)->y)) >= x;
              else
                if (crossing)
                  crossings++;
              continue;
            }
         while ((p <= q) && (p->y < y))
           p++;
         if (p > q)
           break;
         crossing=(p-1)->x >= x;
         if (crossing != (p->x >= x))
           crossings+=
             ((p-1)->x-((p-1)->y-y)*(p->x-(p-1)->x)/(p->y-(p-1)->y)) >= x;
         else
           if (crossing)
             crossings++;
        }
        inside=crossings & 0x01;
        break;
      }
    }
    if (inside)
      return(True);
  }
  return(False);
}

void DrawImage(Image *image, AnnotateInfo *annotate_info)
{
#define DrawImageText  "  Drawing on image...  "

  char
    *stop;

  int
    y;

  PrimitiveInfo
    *primitive_info;

  register char
    *p;

  register int
    i,
    j,
    x;

  register RunlengthPacket
    *q;

  unsigned int
    indirection,
    length,
    number_coordinates,
    primitive;

  XColor
    pen_color;

  if (annotate_info->primitive == (char *) NULL)
    return;
  if (!UncompressImage(image))
    return;
  indirection=(*annotate_info->primitive == '@');
  if (indirection)
    {
      FILE
        *file;

      int
        c;

      register char
        *q;

      /*
        Read text from a file.
      */
      file=(FILE *) fopen(annotate_info->primitive+1,"r");
      if (file == (FILE *) NULL)
        {
          Warning("Unable to read primitive file",annotate_info->primitive+1);
          return;
        }
      length=MaxTextLength;
      annotate_info->primitive=(char *) malloc(length);
      q=annotate_info->primitive;
      while (annotate_info->primitive != (char *) NULL)
      {
        c=fgetc(file);
        if (c == EOF)
          break;
        if ((q-annotate_info->primitive+1) >= length)
          {
            *q='\0';
            length<<=1;
            annotate_info->text=(char*)realloc(annotate_info->primitive,length);
            if (annotate_info->primitive == (char *) NULL)
              break;
            q=annotate_info->primitive+strlen(annotate_info->primitive);
          }
        *q++=(unsigned char) c;
      }
      (void) fclose(file);
      if (annotate_info->primitive == (char *) NULL)
        {
          Warning("Unable to draw image","Memory allocation failed");
          return;
        }
      *q='\0';
    }
  /*
    Allocate primitive info memory.
  */
  number_coordinates=2048;
  primitive_info=(PrimitiveInfo *)
    malloc(number_coordinates*sizeof(PrimitiveInfo));
  if (primitive_info == (PrimitiveInfo *) NULL)
    {
      if (indirection)
        free((char *) annotate_info->primitive);
      Warning("Unable to draw image","Memory allocation failed");
      return;
    }
  /*
    Parse the primitive attributes.
  */
  (void) XQueryColorDatabase(annotate_info->pen,&pen_color);
  image->class=DirectClass;
  primitive=UndefinedPrimitive;
  p=annotate_info->primitive;
  for (i=0; *p != '\0'; )
  {
    while (isspace(*p))
      p++;
    primitive=UndefinedPrimitive;
    if (strncmp("rectangle",p,4) == 0)
      primitive=FillRectanglePrimitive;
    if (strncmp("circle",p,4) == 0)
      primitive=FillEllipsePrimitive;
    if (strncmp("polygon",p,4) == 0)
      primitive=FillPolygonPrimitive;
    if (primitive == UndefinedPrimitive)
      break;
    while (isalpha(*p))
      p++;
    j=i;
    for (x=0; *p != '\0'; x++)
    {
      while (isspace(*p))
        p++;
      if (!isdigit(*p))
        break;
      primitive_info[i].primitive=primitive;
      primitive_info[i].coordinates=0;
      primitive_info[i].x=strtol(p,&stop,0);
      p=stop;
      while (isspace(*p) || (*p == ','))
        p++;
      if (!isdigit(*p))
        break;
      primitive_info[i].y=strtol(p,&stop,0);
      p=stop;
      i++;
      if (i == (number_coordinates-1))
        {
          number_coordinates<<=1;
          primitive_info=(PrimitiveInfo *)
            realloc(primitive_info,number_coordinates*sizeof(PrimitiveInfo));
          if (primitive_info == (PrimitiveInfo *) NULL)
            {
              if (indirection)
                free((char *) annotate_info->primitive);
              Warning("Unable to draw image","Memory allocation failed");
              return;
            }
        }
    }
    primitive_info[j].coordinates=x;
  }
  primitive_info[i].primitive=UndefinedPrimitive;
  if (primitive == UndefinedPrimitive)
    {
      Warning("Unable to draw image","non-conforming primitive definition");
      free((char *) primitive_info);
      if (indirection)
        free((char *) annotate_info->primitive);
      return;
    }
  /*
    Draw the primitive on the image.
  */
  q=image->pixels;
  for (y=0; y < image->rows; y++)
  {
    for (x=0; x < image->columns; x++)
    {
      if (InsidePrimitive(primitive_info,x,y))
        {
          q->red=XDownScale(pen_color.red);
          q->green=XDownScale(pen_color.green);
          q->blue=XDownScale(pen_color.blue);
        }
      q++;
    }
    ProgressMonitor(DrawImageText,y,image->rows);
  }
  free((char *) primitive_info);
  if (indirection)
    free((char *) annotate_info->primitive);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%     E m b o s s I m a g e                                                   %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Function EmbossImage creates a new image that is a copy of an existing
%  one with the edge highlighted.  It allocates the memory necessary for the
%  new Image structure and returns a pointer to the new image.
%
%  EmbossImage convolves the pixel neighborhood with this edge detection mask:
%
%    -1 -2  0
%    -2  0  2
%     0  2  1
%
%  The scan only processes pixels that have a full set of neighbors.  Pixels
%  in the top, bottom, left, and right pairs of rows and columns are omitted
%  from the scan.
%
%  The format of the EmbossImage routine is:
%
%      embossed_image=EmbossImage(image)
%
%  A description of each parameter follows:
%
%    o embossed_image: Function EmbossImage returns a pointer to the image
%      after it is embossed.  A null image is returned if there is a memory
%      shortage.
%
%    o image: The address of a structure of type Image;  returned from
%      ReadImage.
%
%
*/
Image *EmbossImage(Image *image)
{
#define EmbossImageText  "  Embossing image...  "
#define Emboss(weight) \
  total_red+=(weight)*(int) (s->red); \
  total_green+=(weight)*(int) (s->green); \
  total_blue+=(weight)*(int) (s->blue); \
  s++;

  Image
    *embossed_image;

  long int
    total_blue,
    total_green,
    total_red;

  register RunlengthPacket
    *p,
    *q,
    *s,
    *s0,
    *s1,
    *s2;

  register unsigned int
    x;

  RunlengthPacket
    background_pixel,
    *scanline;

  unsigned int
    y;

  if ((image->columns < 3) || (image->rows < 3))
    {
      Warning("Unable to emboss image","image size must exceed 3x3");
      return((Image *) NULL);
    }
  /*
    Initialize embossed image attributes.
  */
  embossed_image=CopyImage(image,image->columns,image->rows,False);
  if (embossed_image == (Image *) NULL)
    {
      Warning("Unable to enhance image","Memory allocation failed");
      return((Image *) NULL);
    }
  embossed_image->class=DirectClass;
  /*
    Allocate scan line buffer for 3 rows of the image.
  */
  scanline=(RunlengthPacket *) malloc(3*image->columns*sizeof(RunlengthPacket));
  if (scanline == (RunlengthPacket *) NULL)
    {
      Warning("Unable to enhance image","Memory allocation failed");
      DestroyImage(embossed_image);
      return((Image *) NULL);
    }
  /*
    Read the first two rows of the image.
  */
  p=image->pixels;
  image->runlength=p->length+1;
  s=scanline;
  for (x=0; x < (image->columns << 1); x++)
  {
    if (image->runlength != 0)
      image->runlength--;
    else
      {
        p++;
        image->runlength=p->length;
      }
    *s=(*p);
    s++;
  }
  /*
    Dump first scanlines of image.
  */
  background_pixel.red=0;
  background_pixel.green=0;
  background_pixel.blue=0;
  background_pixel.index=0;
  background_pixel.length=0;
  q=embossed_image->pixels;
  for (x=0; x < image->columns; x++)
  {
    *q=background_pixel;
    q++;
  }
  /*
    Convolve each row.
  */
  for (y=1; y < (image->rows-1); y++)
  {
    /*
      Initialize sliding window pointers.
    */
    s0=scanline+image->columns*((y-1) % 3);
    s1=scanline+image->columns*(y % 3);
    s2=scanline+image->columns*((y+1) % 3);
    /*
      Read another scan line.
    */
    s=s2;
    for (x=0; x < image->columns; x++)
    {
      if (image->runlength != 0)
        image->runlength--;
      else
        {
          p++;
          image->runlength=p->length;
        }
      *s=(*p);
      s++;
    }
    /*
      Transfer first pixel of the scanline.
    */
    *q=background_pixel;
    q++;
    for (x=1; x < (image->columns-1); x++)
    {
      /*
        Compute weighted average of target pixel color components.
      */
      total_red=0;
      total_green=0;
      total_blue=0;
      s=s1+1;
      s=s0;
      Emboss(-1); Emboss(-2); Emboss( 0);
      s=s1;
      Emboss(-2); Emboss( 0); Emboss( 2);
      s=s2;
      Emboss( 0); Emboss( 2); Emboss( 1);
      total_red+=(MaxRGB+1) >> 1;
      if (total_red < 0)
        total_red=0;
      else
        if (total_red > MaxRGB)
          total_red=MaxRGB;
      total_green+=(MaxRGB+1) >> 1;
      if (total_green < 0)
        total_green=0;
      else
        if (total_green > MaxRGB)
          total_green=MaxRGB;
      total_blue+=(MaxRGB+1) >> 1;
      if (total_blue < 0)
        total_blue=0;
      else
        if (total_blue > MaxRGB)
          total_blue=MaxRGB;
      q->red=(Quantum) total_red;
      q->green=(Quantum) total_green;
      q->blue=(Quantum) total_blue;
      q->index=s1->index;
      q->length=0;
      q++;
      s0++;
      s1++;
      s2++;
    }
    /*
      Transfer last pixel of the scanline.
    */
    *q=background_pixel;
    q++;
    ProgressMonitor(EmbossImageText,y,image->rows-1);
  }
  /*
    Dump last scanline of pixels.
  */
  for (x=0; x < image->columns; x++)
  {
    *q=background_pixel;
    q++;
  }
  free((char *) scanline);
  /*
    Convert image to Grayscale and normalize.
  */
  embossed_image->class=DirectClass;
  (void) IsGrayImage(embossed_image);
  NormalizeImage(embossed_image);
  return(embossed_image);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%     E d g e I m a g e                                                       %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Function EdgeImage creates a new image that is a copy of an existing
%  one with the edges highlighted.  It allocates the memory necessary for the
%  new Image structure and returns a pointer to the new image.
%
%  EdgeImage convolves the pixel neighborhood with this edge detection mask:
%
%    -1  0 -1
%     0  W  0
%    -1  0 -1
%
%  The scan only processes pixels that have a full set of neighbors.  Pixels
%  in the top, bottom, left, and right pairs of rows and columns are omitted
%  from the scan.
%
%  The format of the EdgeImage routine is:
%
%      edged_image=EdgeImage(image,factor)
%
%  A description of each parameter follows:
%
%    o edged_image: Function EdgeImage returns a pointer to the image
%      after it is edged.  A null image is returned if there is a memory
%      shortage.
%
%    o image: The address of a structure of type Image;  returned from
%      ReadImage.
%
%    o factor:  An double value reflecting the percent weight to give to the
%      center pixel of the neighborhood.
%
%
*/
Image *EdgeImage(Image *image, double factor)
{
#define Edge(weight) \
  total_red+=((long int)((weight)*(int) (s->red))); \
  total_green+=((long int)((weight)*(int) (s->green))); \
  total_blue+=((long int)((weight)*(int) (s->blue))); \
  total_index+=((long int)((weight)*(int) (s->index))); \
  s++;
#define EdgeImageText  "  Detecting image edges...  "

  double
    weight;

  Image
    *edged_image;

  long int
    total_blue,
    total_green,
    total_index,
    total_red;

  register RunlengthPacket
    *p,
    *q,
    *s,
    *s0,
    *s1,
    *s2;

  register unsigned int
    x;

  RunlengthPacket
    background_pixel,
    *scanline;

  unsigned int
    y;

  if ((image->columns < 3) || (image->rows < 3))
    {
      Warning("Unable to detect edges","image size must exceed 3x3");
      return((Image *) NULL);
    }
  /*
    Initialize edged image attributes.
  */
  edged_image=CopyImage(image,image->columns,image->rows,False);
  if (edged_image == (Image *) NULL)
    {
      Warning("Unable to enhance image","Memory allocation failed");
      return((Image *) NULL);
    }
  edged_image->class=DirectClass;
  /*
    Allocate scan line buffer for 3 rows of the image.
  */
  scanline=(RunlengthPacket *) malloc(3*image->columns*sizeof(RunlengthPacket));
  if (scanline == (RunlengthPacket *) NULL)
    {
      Warning("Unable to enhance image","Memory allocation failed");
      DestroyImage(edged_image);
      return((Image *) NULL);
    }
  /*
    Read the first two rows of the image.
  */
  p=image->pixels;
  image->runlength=p->length+1;
  s=scanline;
  for (x=0; x < (image->columns << 1); x++)
  {
    if (image->runlength != 0)
      image->runlength--;
    else
      {
        p++;
        image->runlength=p->length;
      }
    *s=(*p);
    s++;
  }
  /*
    Dump first scanlines of image.
  */
  background_pixel.red=0;
  background_pixel.green=0;
  background_pixel.blue=0;
  background_pixel.index=0;
  background_pixel.length=0;
  q=edged_image->pixels;
  for (x=0; x < image->columns; x++)
  {
    *q=background_pixel;
    q++;
  }
  /*
    Convolve each row.
  */
  weight=((100.0-factor)/20)+1.5;
  for (y=1; y < (image->rows-1); y++)
  {
    /*
      Initialize sliding window pointers.
    */
    s0=scanline+image->columns*((y-1) % 3);
    s1=scanline+image->columns*(y % 3);
    s2=scanline+image->columns*((y+1) % 3);
    /*
      Read another scan line.
    */
    s=s2;
    for (x=0; x < image->columns; x++)
    {
      if (image->runlength != 0)
        image->runlength--;
      else
        {
          p++;
          image->runlength=p->length;
        }
      *s=(*p);
      s++;
    }
    /*
      Transfer first pixel of the scanline.
    */
    *q=background_pixel;
    q++;
    for (x=1; x < (image->columns-1); x++)
    {
      /*
        Compute weighted average of target pixel color components. 
      */
      total_red=0;
      total_green=0;
      total_blue=0;
      total_index=0;
      s=s1+1;
      s=s0;
      Edge(-weight/4); Edge( 0); Edge(-weight/4);
      s=s1;
      Edge( 0); Edge(weight); Edge( 0);
      s=s2;
      Edge(-weight/4); Edge( 0); Edge(-weight/4);
      if (total_red < 0)
        q->red=0;
      else
        if (total_red > MaxRGB)
          q->red=MaxRGB;
        else
          q->red=(Quantum) total_red;
      if (total_green < 0)
        q->green=0;
      else
        if (total_green > MaxRGB)
          q->green=MaxRGB;
        else
          q->green=(Quantum) total_green;
      if (total_blue < 0)
        q->blue=0;
      else
        if (total_blue > MaxRGB)
          q->blue=MaxRGB;
        else
          q->blue=(Quantum) total_blue;
      if (total_index < 0)
        q->index=0;
      else
        if (total_index > MaxRGB)
          q->index=MaxRGB;
        else
          q->index=(unsigned short) total_index;
      q->length=0;
      q++;
      s0++;
      s1++;
      s2++;
    }
    /*
      Transfer last pixel of the scanline.
    */
    *q=background_pixel;
    q++;
    ProgressMonitor(EdgeImageText,y,image->rows-1);
  }
  /*
    Dump last scanline of pixels.
  */
  for (x=0; x < image->columns; x++)
  {
    *q=background_pixel;
    q++;
  }
  free((char *) scanline);
  /*
    Normalize image.
  */
  NormalizeImage(edged_image);
  return(edged_image);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%     E n h a n c e I m a g e                                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Function EnhanceImage creates a new image that is a copy of an existing
%  one with the noise minified.  It allocates the memory necessary for the new
%  Image structure and returns a pointer to the new image.
%
%  EnhanceImage does a weighted average of pixels in a 5x5 cell around each
%  target pixel.  Only pixels in the 5x5 cell that are within a RGB distance
%  threshold of the target pixel are averaged.
%
%  Weights assume that the importance of neighboring pixels is negately
%  proportional to the square of their distance from the target pixel.
%
%  The scan only processes pixels that have a full set of neighbors.  Pixels
%  in the top, bottom, left, and right pairs of rows and columns are omitted
%  from the scan.
%
%  The format of the EnhanceImage routine is:
%
%      enhanced_image=EnhanceImage(image)
%
%  A description of each parameter follows:
%
%    o enhanced_image: Function EnhanceImage returns a pointer to the image
%      after it is enhanced.  A null image is returned if there is a memory
%      shortage.
%
%    o image: The address of a structure of type Image;  returned from
%      ReadImage.
%
%
*/
Image *EnhanceImage(Image *image)
{
#define Enhance(weight) \
  distance=(int) s->red-(int) red; \
  distance_squared=squares[distance]; \
  distance=(int) s->green-(int) green; \
  distance_squared+=squares[distance]; \
  distance=(int) s->blue-(int) blue; \
  distance_squared+=squares[distance]; \
  if (distance_squared < Threshold) \
    { \
      total_red+=(weight)*(s->red); \
      total_green+=(weight)*(s->green); \
      total_blue+=(weight)*(s->blue); \
      total_weight+=(weight); \
    } \
  s++;
#define EnhanceImageText  "  Enhancing image...  "
#define Threshold  2500

  double
    distance_squared;

  Image
    *enhanced_image;

  int
    distance,
    i;

  Quantum
    blue,
    green,
    red;

  register RunlengthPacket
    *p,
    *q,
    *s,
    *s0,
    *s1,
    *s2,
    *s3,
    *s4;

  register unsigned int
    *squares;

  RunlengthPacket
    *scanline;

  unsigned int
    x,
    y;

  unsigned long
    total_blue,
    total_green,
    total_red,
    total_weight;

  if ((image->columns < 5) || (image->rows < 5))
    {
      Warning("Unable to enhance image","image size must exceed 4x4");
      return((Image *) NULL);
    }
  /*
    Initialize enhanced image attributes.
  */
  enhanced_image=CopyImage(image,image->columns,image->rows,False);
  if (enhanced_image == (Image *) NULL)
    {
      Warning("Unable to enhance image","Memory allocation failed");
      return((Image *) NULL);
    }
  enhanced_image->class=DirectClass;
  /*
    Allocate scan line buffer for 5 rows of the image.
  */
  scanline=(RunlengthPacket *) malloc(5*image->columns*sizeof(RunlengthPacket));
  squares=(unsigned int *) malloc((MaxRGB+MaxRGB+1)*sizeof(unsigned int));
  if ((scanline == (RunlengthPacket *) NULL) ||
      (squares == (unsigned int *) NULL))
    {
      Warning("Unable to enhance image","Memory allocation failed");
      DestroyImage(enhanced_image);
      return((Image *) NULL);
    }
  squares+=MaxRGB;
  for (i=(-MaxRGB); i <= MaxRGB; i++)
    squares[i]=i*i;
  /*
    Read the first 4 rows of the image.
  */
  p=image->pixels;
  image->runlength=p->length+1;
  s=scanline;
  for (x=0; x < (image->columns*4); x++)
  {
    if (image->runlength != 0)
      image->runlength--;
    else
      {
        p++;
        image->runlength=p->length;
      }
    *s=(*p);
    s++;
  }
  /*
    Dump first 2 scanlines of image.
  */
  q=enhanced_image->pixels;
  s=scanline;
  for (x=0; x < (image->columns << 1); x++)
  {
    *q=(*s);
    q->length=0;
    q++;
    s++;
  }
  /*
    Enhance each row.
  */
  for (y=2; y < (image->rows-2); y++)
  {
    /*
      Initialize sliding window pointers.
    */
    s0=scanline+image->columns*((y-2) % 5);
    s1=scanline+image->columns*((y-1) % 5);
    s2=scanline+image->columns*(y % 5);
    s3=scanline+image->columns*((y+1) % 5);
    s4=scanline+image->columns*((y+2) % 5);
    /*
      Read another scan line.
    */
    s=s4;
    for (x=0; x < image->columns; x++)
    {
      if (image->runlength != 0)
        image->runlength--;
      else
        {
          p++;
          image->runlength=p->length;
        }
      *s=(*p);
      s++;
    }
    /*
      Transfer first 2 pixels of the scanline.
    */
    s=s2;
    for (x=0; x < 2; x++)
    {
      *q=(*s);
      q->length=0;
      q++;
      s++;
    }
    for (x=2; x < (image->columns-2); x++)
    {
      /*
        Compute weighted average of target pixel color components.
      */
      total_red=0;
      total_green=0;
      total_blue=0;
      total_weight=0;
      s=s2+2;
      red=s->red;
      green=s->green;
      blue=s->blue;
      s=s0;
      Enhance(5);  Enhance(8);  Enhance(10); Enhance(8);  Enhance(5);
      s=s1;
      Enhance(8);  Enhance(20); Enhance(40); Enhance(20); Enhance(8);
      s=s2;
      Enhance(10); Enhance(40); Enhance(80); Enhance(40); Enhance(10);
      s=s3;
      Enhance(8);  Enhance(20); Enhance(40); Enhance(20); Enhance(8);
      s=s4;
      Enhance(5);  Enhance(8);  Enhance(10); Enhance(8);  Enhance(5);
      q->red=(Quantum) ((total_red+(total_weight >> 1)-1)/total_weight);
      q->green= (Quantum) ((total_green+(total_weight >> 1)-1)/total_weight);
      q->blue=(Quantum) ((total_blue+(total_weight >> 1)-1)/total_weight);
      q->index=s2->index;
      q->length=0;
      q++;
      s0++;
      s1++;
      s2++;
      s3++;
      s4++;
    }
    /*
      Transfer last 2 pixels of the scanline.
    */
    s=s2;
    for (x=0; x < 2; x++)
    {
      *q=(*s);
      q->length=0;
      q++;
      s++;
    }
    ProgressMonitor(EnhanceImageText,y,image->rows-2);
  }
  /*
    Dump last 2 scanlines of pixels.
  */
  s=scanline+image->columns*(y % 5);
  for (x=0; x < (image->columns << 1); x++)
  {
    *q=(*s);
    q->length=0;
    q++;
    s++;
  }
  squares-=MaxRGB;
  free((char *) squares);
  free((char *) scanline);
  return(enhanced_image);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%     E q u a l i z e I m a g e                                               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Function EqualizeImage performs histogram equalization on the reference
%  image.
%
%  The format of the EqualizeImage routine is:
%
%      EqualizeImage(image)
%
%  A description of each parameter follows:
%
%    o image: The address of a structure of type Image;  returned from
%      ReadImage.
%
%
*/
void EqualizeImage(Image *image)
{
#define EqualizeImageText  "  Equalizing image...  "

  Quantum
    *equalize_map;

  register int
    i,
    j;

  register RunlengthPacket
    *p;

  unsigned int
    high,
    *histogram,
    low,
    *map;

  /*
    Allocate and initialize histogram arrays.
  */
  histogram=(unsigned int *) malloc((MaxRGB+1)*sizeof(unsigned int));
  map=(unsigned int *) malloc((MaxRGB+1)*sizeof(unsigned int));
  equalize_map=(Quantum *) malloc((MaxRGB+1)*sizeof(Quantum));
  if ((histogram == (unsigned int *) NULL) || (map == (unsigned int *) NULL) ||
      (equalize_map == (Quantum *) NULL))
    {
      Warning("Unable to equalize image","Memory allocation failed");
      return;
    }
  /*
    Form histogram.
  */
  for (i=0; i <= MaxRGB; i++)
    histogram[i]=0;
  p=image->pixels;
  for (i=0; i < image->packets; i++)
  {
    histogram[Intensity(*p)]+=(p->length+1);
    p++;
  }
  /*
    Integrate the histogram to get the equalization map.
  */
  j=0;
  for (i=0; i <= MaxRGB; i++)
  {
    j+=histogram[i];
    map[i]=j;
  }
  free((char *) histogram);
  if (map[MaxRGB] == 0)
    {
      free((char *) equalize_map);
      free((char *) map);
      return;
    }
  /*
    Equalize.
  */
  low=map[0];
  high=map[MaxRGB];
  for (i=0; i <= MaxRGB; i++)
    equalize_map[i]=(Quantum)
      ((((double) (map[i]-low))*MaxRGB)/Max(high-low,1));
  free((char *) map);
  /*
    Stretch the histogram.
  */
  switch (image->class)
  {
    case DirectClass:
    {
      /*
        Equalize DirectClass packets.
      */
      p=image->pixels;
      for (i=0; i < image->packets; i++)
      {
        p->red=equalize_map[p->red];
        p->green=equalize_map[p->green];
        p->blue=equalize_map[p->blue];
        p++;
        if (QuantumTick(i,image))
          ProgressMonitor(EqualizeImageText,i,image->packets);
      }
      break;
    }
    case PseudoClass:
    {
      /*
        Equalize PseudoClass packets.
      */
      for (i=0; i < image->colors; i++)
      {
        image->colormap[i].red=equalize_map[image->colormap[i].red];
        image->colormap[i].green=equalize_map[image->colormap[i].green];
        image->colormap[i].blue=equalize_map[image->colormap[i].blue];
      }
      SyncImage(image);
      break;
    }
  }
  free((char *) equalize_map);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   F l i p I m a g e                                                         %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Function FlipImage creates a new image that reflects each scanline in the
%  vertical direction It allocates the memory necessary for the new Image
%  structure and returns a pointer to the new image.
%
%  The format of the FlipImage routine is:
%
%      flipped_image=FlipImage(image)
%
%  A description of each parameter follows:
%
%    o flipped_image: Function FlipImage returns a pointer to the image
%      after reflecting.  A null image is returned if there is a memory
%      shortage.
%
%    o image: The address of a structure of type Image.
%
%
*/
Image *FlipImage(Image *image)
{
#define FlipImageText  "  Flipping image...  "

  Image
    *flipped_image;

  register RunlengthPacket
    *p,
    *q,
    *s;

  register unsigned int
    x,
    y;

  RunlengthPacket
    *scanline;

  /*
    Initialize flipped image attributes.
  */
  flipped_image=CopyImage(image,image->columns,image->rows,False);
  if (flipped_image == (Image *) NULL)
    {
      Warning("Unable to flip image","Memory allocation failed");
      return((Image *) NULL);
    }
  /*
    Allocate scan line buffer and column offset buffers.
  */
  scanline=(RunlengthPacket *) malloc(image->columns*sizeof(RunlengthPacket));
  if (scanline == (RunlengthPacket *) NULL)
    {
      Warning("Unable to reflect image","Memory allocation failed");
      DestroyImage(flipped_image);
      return((Image *) NULL);
    }
  /*
    Flip each row.
  */
  p=image->pixels;
  image->runlength=p->length+1;
  q=flipped_image->pixels+flipped_image->packets-1;
  for (y=0; y < flipped_image->rows; y++)
  {
    /*
      Read a scan line.
    */
    s=scanline;
    for (x=0; x < image->columns; x++)
    {
      if (image->runlength != 0)
        image->runlength--;
      else
        {
          p++;
          image->runlength=p->length;
        }
      *s=(*p);
      s++;
    }
    /*
      Flip each column.
    */
    s=scanline+image->columns;
    for (x=0; x < flipped_image->columns; x++)
    {
      s--;
      *q=(*s);
      q->length=0;
      q--;
    }
    ProgressMonitor(FlipImageText,y,flipped_image->rows);
  }
  free((char *) scanline);
  return(flipped_image);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   F l o p I m a g e                                                         %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Function FlopImage creates a new image that reflects each scanline in the
%  horizontal direction It allocates the memory necessary for the new Image
%  structure and returns a pointer to the new image.
%
%  The format of the FlopImage routine is:
%
%      flopped_image=FlopImage(image)
%
%  A description of each parameter follows:
%
%    o flopped_image: Function FlopImage returns a pointer to the image
%      after reflecting.  A null image is returned if there is a memory
%      shortage.
%
%    o image: The address of a structure of type Image.
%
%
*/
Image *FlopImage(Image *image)
{
#define FlopImageText  "  Flopping image...  "

  Image
    *flopped_image;

  register RunlengthPacket
    *p,
    *q,
    *s;

  register unsigned int
    x,
    y;

  RunlengthPacket
    *scanline;

  /*
    Initialize flopped image attributes.
  */
  flopped_image=CopyImage(image,image->columns,image->rows,False);
  if (flopped_image == (Image *) NULL)
    {
      Warning("Unable to reflect image","Memory allocation failed");
      return((Image *) NULL);
    }
  /*
    Allocate scan line buffer and column offset buffers.
  */
  scanline=(RunlengthPacket *) malloc(image->columns*sizeof(RunlengthPacket));
  if (scanline == (RunlengthPacket *) NULL)
    {
      Warning("Unable to reflect image","Memory allocation failed");
      DestroyImage(flopped_image);
      return((Image *) NULL);
    }
  /*
    Flop each row.
  */
  p=image->pixels;
  image->runlength=p->length+1;
  q=flopped_image->pixels;
  for (y=0; y < flopped_image->rows; y++)
  {
    /*
      Read a scan line.
    */
    s=scanline;
    for (x=0; x < image->columns; x++)
    {
      if (image->runlength != 0)
        image->runlength--;
      else
        {
          p++;
          image->runlength=p->length;
        }
      *s=(*p);
      s++;
    }
    /*
      Flop each column.
    */
    s=scanline+image->columns;
    for (x=0; x < flopped_image->columns; x++)
    {
      s--;
      *q=(*s);
      q->length=0;
      q++;
    }
    ProgressMonitor(FlopImageText,y,flopped_image->rows);
  }
  free((char *) scanline);
  return(flopped_image);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   F r a m e I m a g e                                                       %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Function FrameImage takes an image and puts a frame around it of a
%  particular color.  It allocates the memory necessary for the new Image
%  structure and returns a pointer to the new image.
%
%  The format of the FrameImage routine is:
%
%      framed_image=FrameImage(image,frame_info)
%
%  A description of each parameter follows:
%
%    o framed_image: Function FrameImage returns a pointer to the framed
%      image.  A null image is returned if there is a a memory shortage.
%
%    o image: The address of a structure of type Image.
%
%    o frame_info: Specifies a pointer to a FrameInfo structure which
%      defines the framed region.
%
%
*/
Image *FrameImage(Image *image, FrameInfo *frame_info)
{
#define FrameImageText  "  Adding frame to image...  "

  Image
    *framed_image;

  int
    height,
    width;

  register int
    x,
    y;

  register RunlengthPacket
    *p,
    *q;

  RunlengthPacket
    highlight,
    matte,
    shadow;

  unsigned int
    bevel_width;

  /*
    Check frame geometry.
  */
  if ((frame_info->outer_bevel < 0) || (frame_info->inner_bevel < 0))
    {
      Warning("Unable to frame image","bevel width is negative");
      return((Image *) NULL);
    }
  bevel_width=frame_info->outer_bevel+frame_info->inner_bevel;
  width=(int) frame_info->width-frame_info->x-bevel_width;
  height=(int) frame_info->height-frame_info->y-bevel_width;
  if ((width < image->columns) || (height < image->rows))
    {
      Warning("Unable to frame image","frame is less than image size");
      return((Image *) NULL);
    }
  /*
    Initialize framed image attributes.
  */
  framed_image=CopyImage(image,frame_info->width,frame_info->height,False);
  if (framed_image == (Image *) NULL)
    {
      Warning("Unable to frame image","Memory allocation failed");
      return((Image *) NULL);
    }
  image->class=DirectClass;
  matte.red=frame_info->matte_color.red;
  matte.green=frame_info->matte_color.green;
  matte.blue=frame_info->matte_color.blue;
  matte.index=Opaque;
  matte.length=0;
  highlight.red=frame_info->highlight_color.red;
  highlight.green=frame_info->highlight_color.green;
  highlight.blue=frame_info->highlight_color.blue;
  highlight.index=Opaque;
  highlight.length=0;
  shadow.red=frame_info->shadow_color.red;
  shadow.green=frame_info->shadow_color.green;
  shadow.blue=frame_info->shadow_color.blue;
  shadow.index=Opaque;
  shadow.length=0;
  /*
    Put an ornamental border around the image.
  */
  q=framed_image->pixels;
  for (y=0; y < frame_info->outer_bevel; y++)
  {
    for (x=0; x < (framed_image->columns-y); x++)
      *q++=highlight;
    for ( ; x < framed_image->columns; x++)
      *q++=shadow;
  }
  for (y=0; y < (frame_info->y-bevel_width); y++)
  {
    for (x=0; x < frame_info->outer_bevel; x++)
      *q++=highlight;
    for (x=0; x < (framed_image->columns-(frame_info->outer_bevel << 1)); x++)
      *q++=matte;
    for (x=0; x < frame_info->outer_bevel; x++)
      *q++=shadow;
  }
  for (y=0; y < frame_info->inner_bevel; y++)
  {
    for (x=0; x < frame_info->outer_bevel; x++)
      *q++=highlight;
    for (x=0; x < (frame_info->x-bevel_width); x++)
      *q++=matte;
    for (x=0; x < (image->columns+(frame_info->inner_bevel << 1)-y); x++)
      *q++=shadow;
    for ( ; x < (image->columns+(frame_info->inner_bevel << 1)); x++)
      *q++=highlight;
    width=frame_info->width-frame_info->x-image->columns-bevel_width;
    for (x=0; x < width; x++)
      *q++=matte;
    for (x=0; x < frame_info->outer_bevel; x++)
      *q++=shadow;
  }
  p=image->pixels;
  image->runlength=p->length+1;
  for (y=0; y < image->rows; y++)
  {
    /*
      Initialize scanline with border color.
    */
    for (x=0; x < frame_info->outer_bevel; x++)
      *q++=highlight;
    for (x=0; x < (frame_info->x-bevel_width); x++)
      *q++=matte;
    for (x=0; x < frame_info->inner_bevel; x++)
      *q++=shadow;
    /*
      Transfer scanline.
    */
    for (x=0; x < image->columns; x++)
    {
      if (image->runlength != 0)
        image->runlength--;
      else
        {
          p++;
          image->runlength=p->length;
        }
      *q=(*p);
      q->length=0;
      q++;
    }
    for (x=0; x < frame_info->inner_bevel; x++)
      *q++=highlight;
    width=frame_info->width-frame_info->x-image->columns-bevel_width;
    for (x=0; x < width; x++)
      *q++=matte;
    for (x=0; x < frame_info->outer_bevel; x++)
      *q++=shadow;
    ProgressMonitor(FrameImageText,y,image->rows);
  }
  for (y=frame_info->inner_bevel-1; y >= 0; y--)
  {
    for (x=0; x < frame_info->outer_bevel; x++)
      *q++=highlight;
    for (x=0; x < (frame_info->x-bevel_width); x++)
      *q++=matte;
    for (x=0; x < y; x++)
      *q++=shadow;
    for ( ; x < (image->columns+(frame_info->inner_bevel << 1)); x++)
      *q++=highlight;
    width=frame_info->width-frame_info->x-image->columns-bevel_width;
    for (x=0; x < width; x++)
      *q++=matte;
    for (x=0; x < frame_info->outer_bevel; x++)
      *q++=shadow;
  }
  height=frame_info->height-frame_info->y-image->rows-bevel_width;
  for (y=0; y < height; y++)
  {
    for (x=0; x < frame_info->outer_bevel; x++)
      *q++=highlight;
    for (x=0; x < (framed_image->columns-(frame_info->outer_bevel << 1)); x++)
      *q++=matte;
    for (x=0; x < frame_info->outer_bevel; x++)
      *q++=shadow;
  }
  for (y=frame_info->outer_bevel-1; y >= 0; y--)
  {
    for (x=0; x < y; x++)
      *q++=highlight;
    for ( ; x < framed_image->columns; x++)
      *q++=shadow;
  }
  return(framed_image);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%     G a m m a I m a g e                                                     %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Function GammaImage converts the reference image to gamma corrected colors.
%
%  The format of the GammaImage routine is:
%
%      GammaImage(image,gamma)
%
%  A description of each parameter follows:
%
%    o image: The address of a structure of type Image;  returned from
%      ReadImage.
%
%    o gamma: A character string indicating the level of gamma correction.
%
%
*/
void GammaImage(Image *image, char *gamma)
{
#define GammaImageText  "  Gamma correcting the image...  "

  ColorPacket
    *gamma_map;

  double
    blue_gamma,
    green_gamma,
    red_gamma;

  int
    count;

  register int
    i;

  register RunlengthPacket
    *p;

  red_gamma=1.0;
  green_gamma=1.0;
  blue_gamma=1.0;
  count=sscanf(gamma,"%lf,%lf,%lf",&red_gamma,&green_gamma,&blue_gamma);
  if (count == 1)
    {
      if (red_gamma == 1.0)
        return;
      green_gamma=red_gamma;
      blue_gamma=red_gamma;
    }
  /*
    Allocate and initialize gamma maps.
  */
  gamma_map=(ColorPacket *) malloc((MaxRGB+1)*sizeof(ColorPacket));
  if (gamma_map == (ColorPacket *) NULL)
    {
      Warning("Unable to gamma image","Memory allocation failed");
      return;
    }
  for (i=0; i <= MaxRGB; i++)
  {
    gamma_map[i].red=0;
    gamma_map[i].green=0;
    gamma_map[i].blue=0;
  }
  /*
    Initialize gamma table.
  */
  for (i=0; i <= MaxRGB; i++)
  {
    if (red_gamma != 0.0)
      gamma_map[i].red=(Quantum)
        ((pow((double) i/MaxRGB,1.0/red_gamma)*MaxRGB)+0.5);
    if (green_gamma != 0.0)
      gamma_map[i].green=(Quantum)
        ((pow((double) i/MaxRGB,1.0/green_gamma)*MaxRGB)+0.5);
    if (blue_gamma != 0.0)
      gamma_map[i].blue=(Quantum)
        ((pow((double) i/MaxRGB,1.0/blue_gamma)*MaxRGB)+0.5);
  }
  switch (image->class)
  {
    case DirectClass:
    {
      /*
        Gamma-correct DirectClass image.
      */
      p=image->pixels;
      for (i=0; i < image->packets; i++)
      {
        p->red=gamma_map[p->red].red;
        p->green=gamma_map[p->green].green;
        p->blue=gamma_map[p->blue].blue;
        p++;
        if (QuantumTick(i,image))
          ProgressMonitor(GammaImageText,i,image->packets);
      }
      break;
    }
    case PseudoClass:
    {
      /*
        Gamma-correct PseudoClass image.
      */
      for (i=0; i < image->colors; i++)
      {
        image->colormap[i].red=gamma_map[image->colormap[i].red].red;
        image->colormap[i].green=gamma_map[image->colormap[i].green].green;
        image->colormap[i].blue=gamma_map[image->colormap[i].blue].blue;
      }
      SyncImage(image);
      break;
    }
  }
  if (image->gamma != 0.0)
    image->gamma*=(red_gamma+green_gamma+blue_gamma)/3.0;
  free((char *) gamma_map);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   G e t A n n o t a t e I n f o                                             %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Function GetAnnotateInfo initializes the AnnotateInfo structure.
%
%  The format of the GetAnnotateInfo routine is:
%
%      GetAnnotateInfo(annotate_info)
%
%  A description of each parameter follows:
%
%    o annotate_info: Specifies a pointer to a AnnotateInfo structure.
%
%
*/
void GetAnnotateInfo(AnnotateInfo *annotate_info)
{
  annotate_info->server_name=(char *) NULL;
  annotate_info->font=(char *) NULL;
  annotate_info->pointsize=DefaultPointSize;
  annotate_info->box=(char *) NULL;
  annotate_info->pen=(char *) NULL;
  annotate_info->geometry=(char *) NULL;
  annotate_info->text=(char *) NULL;
  annotate_info->primitive=(char *) NULL;
  annotate_info->center=False;
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   G e t I m a g e I n f o                                                   %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Function GetImageInfo initializes the ImageInfo structure.
%
%  The format of the GetImageInfo routine is:
%
%      GetImageInfo(image_info)
%
%  A description of each parameter follows:
%
%    o image_info: Specifies a pointer to a ImageInfo structure.
%
%
*/
void GetImageInfo(ImageInfo *image_info)
{
  *image_info->magick='\0';
  image_info->filename=(char *) malloc(MaxTextLength);
  if (image_info->filename == (char *) NULL)
    Error("Unable to get image info","Memory allocation failed");
  *image_info->filename='\0';
  image_info->assert=False;
  image_info->subimage=0;
  image_info->subrange=0;
  image_info->server_name=(char *) NULL;
  image_info->font=(char *) NULL;
  image_info->size=(char *) NULL;
  image_info->density=(char *) NULL;
  image_info->page=(char *) NULL;
  image_info->texture=(char *) NULL;
  image_info->dither=True;
  image_info->interlace=DefaultInterlace;
  image_info->monochrome=False;
  image_info->quality=75;
  image_info->verbose=False;
  image_info->monochrome=False;
  image_info->undercolor=(char *) NULL;
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%     I s G r a y I m a g e                                                   %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Function IsGrayImage returns True if the image is grayscale otherwise
%  False is returned.  If the image is DirectClass and grayscale, it is demoted
%  to PseudoClass.
%
%  The format of the IsGrayImage routine is:
%
%      status=IsGrayImage(image)
%
%  A description of each parameter follows:
%
%    o status: Function IsGrayImage returns True if the image is grayscale
%      otherwise False is returned.
%
%    o image: The address of a structure of type Image;  returned from
%      ReadImage.
%
%
*/
unsigned int IsGrayImage(Image *image)
{
  register int
    i;

  unsigned int
    grayscale;

  /*
    Determine if image is grayscale.
  */
  grayscale=True;
  switch (image->class)
  {
    case DirectClass:
    {
      register RunlengthPacket
        *p;

      if (image->matte)
        return(False);
      p=image->pixels;
      for (i=0; i < image->packets; i++)
      {
        if (!IsGray(*p))
          {
            grayscale=False;
            break;
          }
        p++;
      }
      if (grayscale)
        {
          QuantizeImage(image,1 << QuantumDepth,8,False,GRAYColorspace);
          SyncImage(image);
        }
      break;
    }
    case PseudoClass:
    {
      for (i=0; i < image->colors; i++)
        if (!IsGray(image->colormap[i]))
          {
            grayscale=False;
            break;
          }
      break;
    }
  }
  return(grayscale);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   L a b e l I m a g e                                                       %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Function LabelImage initializes an image label.  Optionally the label
%  can include the image filename, type, width, height, or scene number by
%  embedding special format characters.  Embed %f for filename, %m for
%  magick, %w for width, %h for height, or %s for scene number.  For
%  example,
%
%     %f  %wx%h
%
%  produces an image label of
%
%     bird.miff  512x480
%
%  for an image titled bird.miff and whose width is 512 and height is 480.
%
%  The format of the LabelImage routine is:
%
%      LabelImage(image,label)
%
%  A description of each parameter follows:
%
%    o image: The address of a structure of type Image.
%
%    o label: The address of a character string containing the label format.
%
%
*/
void LabelImage(Image *image, char *label)
{
  register char
    *p,
    *q;

  unsigned int
    indirection,
    length;

  if (image->label != (char *) NULL)
    free((char *) image->label);
  image->label=(char *) NULL;
  if (label == (char *) NULL)
    return;
  indirection=(*label == '@');
  if (indirection)
    {
      FILE
        *file;

      int
        c;

      /*
        Read label from a file.
      */
      file=(FILE *) fopen(label+1,"r");
      if (file == (FILE *) NULL)
        {
          Warning("Unable to read label file",label+1);
          return;
        }
      length=MaxTextLength;
      label=(char *) malloc(length);
      for (q=label; label != (char *) NULL; q++)
      {
        c=fgetc(file);
        if (c == EOF)
          break;
        if ((q-label+1) >= length)
          {
            *q='\0';
            length<<=1;
            label=(char *) realloc((char *) label,length);
            if (label == (char *) NULL)
              break;
            q=label+strlen(label);
          }
        *q=(unsigned char) c;
      }
      (void) fclose(file);
      if (label == (char *) NULL)
        {
          Warning("Unable to label image","Memory allocation failed");
          return;
        }
      *q='\0';
    }
  /*
    Allocate and initialize image label.
  */
  p=label;
  length=strlen(label)+MaxTextLength;
  image->label=(char *) malloc(length);
  for (q=image->label; image->label != (char *) NULL; p++)
  {
    *q='\0';
    if (*p == '\0')
      break;
    if ((q-image->label+MaxTextLength) >= length)
      {
        length<<=1;
        image->label=(char *) realloc((char *) image->label,length);
        if (image->label == (char *) NULL)
          break;
        q=image->label+strlen(image->label);
      }
    /*
      Process formatting characters in label.
    */
    if ((*p == '\\') && (*(p+1) == 'n'))
      {
        *q++='\n';
        p++;
        continue;
      }
    if (*p != '%')
      {
        *q++=(*p);
        continue;
      }
    p++;
    switch (*p)
    {
      case 'f':
      {
        register char
          *p;

        /*
          Label segment is the base of the filename.
        */
        p=image->filename+strlen(image->filename)-1;
        while ((p > image->filename) && (*(p-1) != '/'))
          p--;
        (void) strcpy(q,p);
        q+=strlen(p);
        break;
      }
      case 'h':
      {
        (void) sprintf(q,"%u",image->magick_rows);
        q=image->label+strlen(image->label);
        break;
      }
      case 'm':
      {
        (void) strcpy(q,image->magick);
        q+=strlen(image->magick);
        break;
      }
      case 's':
      {
        (void) sprintf(q,"%u",image->scene);
        q=image->label+strlen(image->label);
        break;
      }
      case 'w':
      {
        (void) sprintf(q,"%u",image->magick_columns);
        q=image->label+strlen(image->label);
        break;
      }
      default:
      {
        *q++='%';
        *q++=(*p);
        break;
      }
    }
  }
  if (image->label == (char *) NULL)
    {
      Warning("Unable to label image","Memory allocation failed");
      return;
    }
  *q='\0';
  if (indirection)
    free((char *) label);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   M a g n i f y I m a g e                                                   %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Function MagnifyImage creates a new image that is a integral size greater
%  than an existing one.  It allocates the memory necessary for the new Image
%  structure and returns a pointer to the new image.
%
%  MagnifyImage scans the reference image to create a magnified image by
%  bilinear interpolation.  The magnified image columns and rows become:
%
%    number_columns << 1
%    number_rows << 1
%
%  The format of the MagnifyImage routine is:
%
%      magnified_image=MagnifyImage(image)
%
%  A description of each parameter follows:
%
%    o magnified_image: Function MagnifyImage returns a pointer to the image
%      after magnification.  A null image is returned if there is a a memory
%      shortage.
%
%    o image: The address of a structure of type Image.
%
%
*/
Image *MagnifyImage(Image *image)
{
#define MagnifyImageText  "  Magnifying the image...  "

  Image
    *magnified_image;

  int
    y;

  register int
    x;

  register RunlengthPacket
    *p,
    *q,
    *r;

  /*
    Initialize magnified image attributes.
  */
  magnified_image=CopyImage(image,image->columns << 1,image->rows << 1,False);
  if (magnified_image == (Image *) NULL)
    {
      Warning("Unable to zoom image","Memory allocation failed");
      return((Image *) NULL);
    }
  magnified_image->class=DirectClass;
  /*
    Initialize zoom image pixels.
  */
  p=image->pixels;
  image->runlength=p->length+1;
  q=magnified_image->pixels;
  for (y=0; y < image->rows; y++)
  {
    for (x=0; x < image->columns; x++)
    {
      if (image->runlength != 0)
        image->runlength--;
      else
        {
          p++;
          image->runlength=p->length;
        }
      *q=(*p);
      q->length=0;
      q++;
    }
    q+=image->columns;
  }
  /*
    Magnify each row.
  */
  for (y=0; y < image->rows; y++)
  {
    p=magnified_image->pixels+(image->rows-1-y)*magnified_image->columns+
      (image->columns-1);
    q=magnified_image->pixels+((image->rows-1-y) << 1)*magnified_image->columns+
      ((image->columns-1) << 1);
    *q=(*p);
    *(q+1)=(*(p));
    for (x=1; x < image->columns; x++)
    {
      p--;
      q-=2;
      *q=(*p);
      (q+1)->red=(((int) p->red)+((int) (p+1)->red)+1) >> 1;
      (q+1)->green=(((int) p->green)+((int) (p+1)->green)+1) >> 1;
      (q+1)->blue=(((int) p->blue)+((int) (p+1)->blue)+1) >> 1;
      (q+1)->index=(((int) p->index)+((int) (p+1)->index)+1) >> 1;
      (q+1)->length=0;
    }
  }
  for (y=0; y < (image->rows-1); y++)
  {
    p=magnified_image->pixels+(y << 1)*magnified_image->columns;
    q=p+magnified_image->columns;
    r=q+magnified_image->columns;
    for (x=0; x < (image->columns-1); x++)
    {
      q->red=(((int) p->red)+((int) r->red)+1) >> 1;
      q->green=(((int) p->green)+((int) r->green)+1) >> 1;
      q->blue=(((int) p->blue)+((int) r->blue)+1) >> 1;
      q->index=(((int) p->index)+((int) r->index)+1) >> 1;
      q->length=0;
      (q+1)->red=(((int) p->red)+((int) (p+2)->red)+((int) r->red)+
        ((int) (r+2)->red)+2) >> 2;
      (q+1)->green=(((int) p->green)+((int) (p+2)->green)+((int) r->green)+
        ((int) (r+2)->green)+2) >> 2;
      (q+1)->blue=(((int) p->blue)+((int) (p+2)->blue)+((int) r->blue)+
        ((int) (r+2)->blue)+2) >> 2;
      (q+1)->index=(((int) p->index)+((int) (p+2)->index)+((int) r->index)+
        ((int) (r+2)->index)+2) >> 2;
      (q+1)->length=0;
      q+=2;
      p+=2;
      r+=2;
    }
    q->red=(((int) p->red)+((int) r->red)+1) >> 1;
    q->green=(((int) p->green)+((int) r->green)+1) >> 1;
    q->blue=(((int) p->blue)+((int) r->blue)+1) >> 1;
    q->index=(((int) p->index)+((int) r->index)+1) >> 1;
    q->length=0;
    p++;
    q++;
    r++;
    q->red=(((int) p->red)+((int) r->red)+1) >> 1;
    q->green=(((int) p->green)+((int) r->green)+1) >> 1;
    q->blue=(((int) p->blue)+((int) r->blue)+1) >> 1;
    q->index=(((int) p->index)+((int) r->index)+1) >> 1;
    q->length=0;
    p++;
    q++;
    r++;
    ProgressMonitor(MagnifyImageText,y,image->rows);
  }
  p=magnified_image->pixels+(2*image->rows-2)*magnified_image->columns;
  q=magnified_image->pixels+(2*image->rows-1)*magnified_image->columns;
  for (x=0; x < image->columns; x++)
  {
    *q++=(*p++);
    *q++=(*p++);
  }
  return(magnified_image);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   M i n i f y I m a g e                                                     %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Function MinifyImage creates a new image that is a integral size less than
%  an existing one.  It allocates the memory necessary for the new Image
%  structure and returns a pointer to the new image.
%
%  MinifyImage scans the reference image to create a minified image by computing
%  the weighted average of a 4x4 cell centered at each reference pixel.  The
%  target pixel requires two columns and two rows of the reference pixels.
%  Therefore the minified image columns and rows become:
%
%    number_columns/2
%    number_rows/2
%
%  Weights assume that the importance of neighboring pixels is negately
%  proportional to the square of their distance from the target pixel.
%
%  The scan only processes pixels that have a full set of neighbors.  Pixels
%  in the top, bottom, left, and right pairs of rows and columns are omitted
%  from the scan.
%
%  The format of the MinifyImage routine is:
%
%      minified_image=MinifyImage(image)
%
%  A description of each parameter follows:
%
%    o minified_image: Function MinifyImage returns a pointer to the image
%      after reducing.  A null image is returned if there is a a memory
%      shortage or if the image size is less than IconSize*2.
%
%    o image: The address of a structure of type Image.
%
%
*/
Image *MinifyImage(Image *image)
{
#define Minify(weight) \
  total_red+=(weight)*(s->red); \
  total_green+=(weight)*(s->green); \
  total_blue+=(weight)*(s->blue); \
  total_matte+=(weight)*(s->index); \
  s++;
#define MinifyImageText  "  Minifying image...  "

  Image
    *minified_image;

  register RunlengthPacket
    *p,
    *q,
    *s,
    *s0,
    *s1,
    *s2,
    *s3;

  register unsigned int
    x;

  RunlengthPacket
    *scanline;

  unsigned int
    y;

  unsigned int
    blue,
    green,
    packets,
    red;

  unsigned long
    total_matte,
    total_blue,
    total_green,
    total_red;

  unsigned short
    index;

  if ((image->columns < 4) || (image->rows < 4))
    {
      Warning("Unable to reduce image","image size must exceed 3x3");
      return((Image *) NULL);
    }
  /*
    Initialize minified image attributes.
  */
  packets=Max(image->packets >> 2,1);
  minified_image=CopyImage(image,packets,1,False);
  if (minified_image == (Image *) NULL)
    {
      Warning("Unable to reduce image","Memory allocation failed");
      return((Image *) NULL);
    }
  minified_image->class=DirectClass;
  minified_image->columns=image->columns >> 1;
  minified_image->rows=image->rows >> 1;
  minified_image->packets=0;
  /*
    Allocate image buffer and scanline buffer for 4 rows of the image.
  */
  scanline=(RunlengthPacket *)
    malloc(4*(image->columns+1)*sizeof(RunlengthPacket));
  if (scanline == (RunlengthPacket *) NULL)
    {
      Warning("Unable to reduce image","Memory allocation failed");
      DestroyImage(minified_image);
      return((Image *) NULL);
    }
  /*
    Preload the first 2 rows of the image.
  */
  p=image->pixels;
  image->runlength=p->length+1;
  for (x=0; x < (4*(image->columns+1)); x++)
    scanline[x]=(*p);
  s=scanline;
  for (x=0; x < (image->columns << 1); x++)
  {
    if (image->runlength != 0)
      image->runlength--;
    else
      {
        p++;
        image->runlength=p->length;
      }
    *s=(*p);
    s++;
  }
  /*
    Reduce each row.
  */
  p=image->pixels;
  image->runlength=p->length+1;
  q=minified_image->pixels;
  q->red=0;
  q->green=0;
  q->blue=0;
  q->index=0;
  q->length=MaxRunlength;
  for (y=0; y < (image->rows-1); y+=2)
  {
    /*
      Initialize sliding window pointers.
    */
    s0=scanline+image->columns*((y+0) % 4);
    s1=scanline+image->columns*((y+1) % 4);
    s2=scanline+image->columns*((y+2) % 4);
    s3=scanline+image->columns*((y+3) % 4);
    /*
      Read another scan line.
    */
    s=s2;
    for (x=0; x < image->columns; x++)
    {
      if (image->runlength != 0)
        image->runlength--;
      else
        {
          p++;
          image->runlength=p->length;
        }
      *s=(*p);
      s++;
    }
    /*
      Read another scan line.
    */
    s=s3;
    for (x=0; x < image->columns; x++)
    {
      if (image->runlength != 0)
        image->runlength--;
      else
        {
          p++;
          image->runlength=p->length;
        }
      *s=(*p);
      s++;
    }
    for (x=0; x < (image->columns-1); x+=2)
    {
      /*
        Compute weighted average of target pixel color components.

        These particular coefficients total to 128.  Use 128/2-1 or 63 to
        insure correct round off.
      */
      total_red=0;
      total_green=0;
      total_blue=0;
      total_matte=0;
      s=s0;
      Minify(3); Minify(7);  Minify(7);  Minify(3);
      s=s1;
      Minify(7); Minify(15); Minify(15); Minify(7);
      s=s2;
      Minify(7); Minify(15); Minify(15); Minify(7);
      s=s3;
      Minify(3); Minify(7);  Minify(7);  Minify(3);
      s0+=2;
      s1+=2;
      s2+=2;
      s3+=2;
      red=(Quantum) ((total_red+63) >> 7);
      green=(Quantum) ((total_green+63) >> 7);
      blue=(Quantum) ((total_blue+63) >> 7);
      index=(unsigned short) ((total_matte+63) >> 7);
      if ((red == q->red) && (green == q->green) && (blue == q->blue) &&
          (index == q->index) && ((int) q->length < MaxRunlength))
        q->length++;
      else
        {
          if (minified_image->packets != 0)
            q++;
          minified_image->packets++;
          if (minified_image->packets == packets)
            {
              packets<<=1;
              minified_image->pixels=(RunlengthPacket *) realloc((char *)
                minified_image->pixels,packets*sizeof(RunlengthPacket));
              if (minified_image->pixels == (RunlengthPacket *) NULL)
                {
                  Warning("Unable to reduce image","Memory allocation failed");
                  DestroyImage(minified_image);
                  return((Image *) NULL);
                }
              q=minified_image->pixels+minified_image->packets-1;
            }
          q->red=red;
          q->green=green;
          q->blue=blue;
          q->index=index;
          q->length=0;
        }
    }
    ProgressMonitor(MinifyImageText,y,image->rows-1);
  }
  minified_image->pixels=(RunlengthPacket *) realloc((char *)
    minified_image->pixels,minified_image->packets*sizeof(RunlengthPacket));
  free((char *) scanline);
  return(minified_image);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%     M o d u l a t e I m a g e                                               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Function ModulateImage modulates the hue, saturation, and brightness of an
%  image.
%
%  The format of the ModulateImage routine is:
%
%      ModulateImage(image,modulate)
%
%  A description of each parameter follows:
%
%    o image: The address of a structure of type Image;  returned from
%      ReadImage.
%
%    o modulate: A character string indicating the percent change in hue,
%      saturation, and brightness.
%
%
*/

static void Modulate(double percent_hue, double percent_saturation, double percent_brightness, Quantum *red, Quantum *green, Quantum *blue)
{
  double
    brightness,
    hue,
    saturation;

  /*
    Increase or decrease color brightness, saturation, or hue.
  */
  TransformHSV(*red,*green,*blue,&hue,&saturation,&brightness);
  brightness+=percent_brightness/100.0;
  if (brightness < 0.0)
    brightness=0.0;
  else
    if (brightness > 1.0)
      brightness=1.0;
  saturation+=percent_saturation/100.0;
  if (saturation < 0.0)
    saturation=0.0;
  else
    if (saturation > 1.0)
      saturation=1.0;
  if (hue != -1.0)
    {
      hue+=360.0*percent_hue/100.0;
      if (hue < 0.0)
        hue+=360.0;
      else
        if (hue > 360.0)
          hue-=360.0;
    }
  HSVTransform(hue,saturation,brightness,red,green,blue);
}

void ModulateImage(Image *image, char *modulate)
{
#define ModulateImageText  "  Modulating image...  "

  double
    percent_brightness,
    percent_hue,
    percent_saturation;

  register int
    i;

  register RunlengthPacket
    *p;

  /*
    Initialize gamma table.
  */
  percent_hue=0.0;
  percent_brightness=0.0;
  percent_saturation=0.0;
  (void) sscanf(modulate,"%lf,%lf,%lf",&percent_brightness,&percent_saturation,
    &percent_hue);
  switch (image->class)
  {
    case DirectClass:
    {
      /*
        Modulate the color for a DirectClass image.
      */
      p=image->pixels;
      for (i=0; i < image->packets; i++)
      {
        Modulate(percent_hue,percent_saturation,percent_brightness,
          &p->red,&p->green,&p->blue);
        p++;
        if (QuantumTick(i,image))
          ProgressMonitor(ModulateImageText,i,image->packets);
      }
      break;
    }
    case PseudoClass:
    {
      /*
        Modulate the color for a PseudoClass image.
      */
      for (i=0; i < image->colors; i++)
        Modulate(percent_hue,percent_saturation,percent_brightness,
          &image->colormap[i].red,&image->colormap[i].green,
          &image->colormap[i].blue);
      SyncImage(image);
      break;
    }
  }
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%     M o g r i f y I m a g e                                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Function MogrifyImage applies image processing options to an image as
%  prescribed by command line options.
%
%  The format of the MogrifyImage routine is:
%
%      MogrifyImage(image_info,argc,argv,image)
%
%  A description of each parameter follows:
%
%    o image_info: Specifies a pointer to a ImageInfo structure.
%
%    o argc: Specifies a pointer to an integer describing the number of
%      elements in the argument vector.
%
%    o argv: Specifies a pointer to a text array containing the command line
%      arguments.
%
%    o image: The address of a structure of type Image;  returned from
%      ReadImage.
%
%
*/
void MogrifyImage(ImageInfo *image_info, int argc, char **argv, Image **image)
{
  AnnotateInfo
    annotate_info;

  ColorPacket
    border_color,
    matte_color;

  char
    *option;

  int
    flags,
    x,
    y;

  register int
    i;

  unsigned int
    colorspace,
    number_colors,
    tree_depth;

  XColor
    color;

  if (*image == (Image *) NULL)
    return;
  /*
    Initialize routine variables.
  */
  GetAnnotateInfo(&annotate_info);
  (void) XQueryColorDatabase(BorderColor,&color);
  border_color.red=XDownScale(color.red);
  border_color.green=XDownScale(color.green);
  border_color.blue=XDownScale(color.blue);
  border_color.index=0;
  (void) XQueryColorDatabase(MatteColor,&color);
  matte_color.red=XDownScale(color.red);
  matte_color.green=XDownScale(color.green);
  matte_color.blue=XDownScale(color.blue);
  number_colors=0;
  tree_depth=0;
  colorspace=RGBColorspace;
  if (image_info->monochrome)
    if (!IsGrayImage(*image) || ((*image)->colors != 2))
      {
        number_colors=2;
        tree_depth=8;
        colorspace=GRAYColorspace;
      }
  /*
    Transmogrify the image.
  */
  for (i=1; i < argc; i++)
  {
    option=argv[i];
    if (((int) strlen(option) <= 1) || ((*option != '-') && (*option != '+')))
      continue;
    if (strncmp("annotate",option+1,2) == 0)
      {
        annotate_info.text=argv[++i];
        AnnotateImage(*image,&annotate_info);
        continue;
      }
    if (strncmp("-blur",option,3) == 0)
      {
        double
          factor;

        Image
          *blurred_image;

        /*
          Blur an image.
        */
        factor=atof(argv[++i]);
        blurred_image=BlurImage(*image,factor);
        if (blurred_image != (Image *) NULL)
          {
            DestroyImage(*image);
            *image=blurred_image;
          }
        continue;
      }
    if (strcmp("-border",option) == 0)
      {
        Image
          *bordered_image;

        RectangleInfo
          border_info;

        /*
          Surround image with a border of solid color.
        */
        border_info.width=0;
        border_info.height=0;
        flags=XParseGeometry(argv[++i],&border_info.x,&border_info.y,
          &border_info.width,&border_info.height);
        if ((flags & HeightValue) == 0)
          border_info.height=border_info.width;
        bordered_image=BorderImage(*image,&border_info,&border_color);
        if (bordered_image != (Image *) NULL)
          {
            DestroyImage(*image);
            bordered_image->class=DirectClass;
            *image=bordered_image;
          }
        continue;
      }
    if (strncmp("-bordercolor",option,8) == 0)
      {
        XColor
          target_color;

        /*
          Determine RGB values of the border color.
        */
        (void) XQueryColorDatabase(argv[++i],&target_color);
        border_color.red=XDownScale(target_color.red);
        border_color.green=XDownScale(target_color.green);
        border_color.blue=XDownScale(target_color.blue);
        continue;
      }
    if (strncmp("-box",option,3) == 0)
      {
        annotate_info.box=argv[++i];
        continue;
      }
    if (strncmp("colors",option+1,7) == 0)
      {
        number_colors=atoi(argv[++i]);
        continue;
      }
    if (strncmp("-colorspace",option,8) == 0)
      {
        i++;
        option=argv[i];
        if (Latin1Compare("gray",option) == 0)
          {
            colorspace=GRAYColorspace;
            if (number_colors == 0)
              number_colors=256;
            tree_depth=8;
          }
        if (Latin1Compare("ohta",option) == 0)
          colorspace=OHTAColorspace;
        if (Latin1Compare("rgb",option) == 0)
          colorspace=RGBColorspace;
        if (Latin1Compare("xyz",option) == 0)
          colorspace=XYZColorspace;
        if (Latin1Compare("ycbcr",option) == 0)
          colorspace=YCbCrColorspace;
        if (Latin1Compare("yiq",option) == 0)
          colorspace=YIQColorspace;
        if (Latin1Compare("ypbpr",option) == 0)
          colorspace=YPbPrColorspace;
        if (Latin1Compare("yuv",option) == 0)
          colorspace=YUVColorspace;
        continue;
      }
    if (strncmp("comment",option+1,4) == 0)
      {
        if (*option == '-')
          CommentImage(*image,argv[++i]);
        else
          CommentImage(*image,(char *) NULL);
        continue;
      }
    if (strncmp("compress",option+1,3) == 0)
      {
        (*image)->compression=NoCompression;
        if (*option == '-')
          {
            i++;
            if (Latin1Compare("runlengthencoded",argv[i]) == 0)
              (*image)->compression=RunlengthEncodedCompression;
            else
              if (Latin1Compare("zlib",argv[i]) == 0)
                (*image)->compression=ZlibCompression;
          }
        continue;
      }
    if (strncmp("contrast",option+1,3) == 0)
      {
        ContrastImage(*image,(unsigned int) (*option == '-'));
        continue;
      }
    if ((strncmp("-crop",option,3) == 0) ||
        (strncmp("-clip",option,4) == 0))
      {
        TransformImage(image,argv[++i],(char *) NULL);
        continue;
      }
    if (strncmp("-despeckle",option,4) == 0)
      {
        Image
          *despeckled_image;

        /*
          Reduce the speckles within an image.
        */
        despeckled_image=DespeckleImage(*image);
        if (despeckled_image != (Image *) NULL)
          {
            DestroyImage(*image);
            *image=despeckled_image;
          }
        continue;
      }
    if (strncmp("-display",option,4) == 0)
      {
        annotate_info.server_name=argv[++i];
        continue;
      }
    if (strncmp("draw",option+1,2) == 0)
      {
        annotate_info.primitive=argv[++i];
        DrawImage(*image,&annotate_info);
        continue;
      }
    if (strncmp("-edge",option,3) == 0)
      {
        double
          factor;

        Image
          *edged_image;

        /*
          Detect edges in the image.
        */
        factor=atof(argv[++i]);
        edged_image=EdgeImage(*image,factor);
        if (edged_image != (Image *) NULL)
          {
            DestroyImage(*image);
            *image=edged_image;
          }
        continue;
      }
    if (strncmp("-emboss",option,3) == 0)
      {
        Image
          *embossed_image;

        /*
          Emboss image.
        */
        embossed_image=EmbossImage(*image);
        if (embossed_image != (Image *) NULL)
          {
            DestroyImage(*image);
            *image=embossed_image;
          }
        continue;
      }
    if (strncmp("-enhance",option,3) == 0)
      {
        Image
          *enhanced_image;

        /*
          Enhance image.
        */
        enhanced_image=EnhanceImage(*image);
        if (enhanced_image != (Image *) NULL)
          {
            DestroyImage(*image);
            *image=enhanced_image;
          }
        continue;
      }
    if (strncmp("-equalize",option,3) == 0)
      {
        EqualizeImage(*image);
        continue;
      }
    if (strncmp("-flip",option,4) == 0)
      {
        Image
          *flipped_image;

        /*
          Flip image scanlines.
        */
        flipped_image=FlipImage(*image);
        if (flipped_image != (Image *) NULL)
          {
            DestroyImage(*image);
            *image=flipped_image;
          }
        continue;
      }
    if (strncmp("-flop",option,4) == 0)
      {
        Image
          *flopped_image;

        /*
          Flop image scanlines.
        */
        flopped_image=FlopImage(*image);
        if (flopped_image != (Image *) NULL)
          {
            DestroyImage(*image);
            *image=flopped_image;
          }
        continue;
      }
    if (strncmp("-font",option,3) == 0)
      {
        annotate_info.font=argv[++i];
        continue;
      }
    if (strcmp("-frame",option) == 0)
      {
        Image
          *framed_image;

        FrameInfo
          frame_info;

        /*
          Surround image with an ornamental border.
        */
        frame_info.width=0;
        frame_info.height=0;
        flags=XParseGeometry(argv[++i],&frame_info.outer_bevel,
          &frame_info.inner_bevel,&frame_info.width,&frame_info.height);
        if ((flags & HeightValue) == 0)
          frame_info.height=frame_info.width;
        if ((flags & XValue) == 0)
          frame_info.outer_bevel=(frame_info.width >> 2)+1;
        if ((flags & YValue) == 0)
          frame_info.inner_bevel=frame_info.outer_bevel;
        frame_info.x=frame_info.width;
        frame_info.y=frame_info.height;
        frame_info.width=(*image)->columns+(frame_info.width << 1);
        frame_info.height=(*image)->rows+(frame_info.height << 1);
        frame_info.matte_color=matte_color;
        frame_info.highlight_color.red=(matte_color.red*HighlightModulate+
          (MaxRGB-HighlightModulate)*65535L)/MaxRGB;
        frame_info.highlight_color.green=(matte_color.green*HighlightModulate+
          (MaxRGB-HighlightModulate)*65535L)/MaxRGB;
        frame_info.highlight_color.blue=(matte_color.blue*HighlightModulate+
          (MaxRGB-HighlightModulate)*65535L)/MaxRGB;
        frame_info.shadow_color.red=
          (unsigned int) (matte_color.red*ShadowModulate)/MaxRGB;
        frame_info.shadow_color.green=
          (unsigned int) (matte_color.green*ShadowModulate)/MaxRGB;
        frame_info.shadow_color.blue=
          (unsigned int) (matte_color.blue*ShadowModulate)/MaxRGB;
        framed_image=FrameImage(*image,&frame_info);
        if (framed_image != (Image *) NULL)
          {
            DestroyImage(*image);
            framed_image->class=DirectClass;
            *image=framed_image;
          }
        continue;
      }
    if (strncmp("-gamma",option,3) == 0)
      {
        GammaImage(*image,argv[++i]);
        continue;
      }
    if (strncmp("-geometry",option,4) == 0)
      {
        TransformImage(image,(char *) NULL,argv[++i]);
        annotate_info.geometry=argv[i];
        continue;
      }
    if (strncmp("interlace",option+1,3) == 0)
      {
        image_info->interlace=NoneInterlace;
        if (*option == '-')
          {
            option=argv[++i];
            if (Latin1Compare("none",option) == 0)
              image_info->interlace=NoneInterlace;
            if (Latin1Compare("line",option) == 0)
              image_info->interlace=LineInterlace;
            if (Latin1Compare("plane",option) == 0)
              image_info->interlace=PlaneInterlace;
          }
        continue;
      }
    if (strncmp("label",option+1,2) == 0)
      {
        if (*option == '-')
          LabelImage(*image,argv[++i]);
        else
          LabelImage(*image,(char *) NULL);
        continue;
      }
    if (strcmp("matte",option+1) == 0)
      {
        (*image)->matte=(*option == '-');
        continue;
      }
    if (strncmp("-mattecolor",option,7) == 0)
      {
        XColor
          target_color;

        /*
          Determine RGB values of the border color.
        */
        (void) XQueryColorDatabase(argv[++i],&target_color);
        matte_color.red=XDownScale(target_color.red);
        matte_color.green=XDownScale(target_color.green);
        matte_color.blue=XDownScale(target_color.blue);
        continue;
      }
    if (strncmp("-modulate",option,4) == 0)
      {
        ModulateImage(*image,argv[++i]);
        continue;
      }
    if (strncmp("-negate",option,4) == 0)
      {
        NegateImage(*image);
        continue;
      }
    if (strncmp("-noise",option,4) == 0)
      {
        Image
          *noisy_image;

        /*
          Reduce noise in image.
        */
        noisy_image=NoisyImage(*image);
        if (noisy_image != (Image *) NULL)
          {
            DestroyImage(*image);
            *image=noisy_image;
          }
        continue;
      }
    if (strncmp("-normalize",option,4) == 0)
      {
        NormalizeImage(*image);
        continue;
      }
    if (strncmp("-opaque",option,3) == 0)
      {
        OpaqueImage(*image,argv[++i],annotate_info.pen);
        continue;
      }
    if (strncmp("-paint",option,4) == 0)
      {
        Image
          *painted_image;

        /*
          Oil paint image.
        */
        painted_image=OilPaintImage(*image);
        if (painted_image != (Image *) NULL)
          {
            DestroyImage(*image);
            *image=painted_image;
          }
        continue;
      }
    if (strncmp("-pen",option,3) == 0)
      {
        annotate_info.pen=argv[++i];
        continue;
      }
    if (strncmp("-pointsize",option,3) == 0)
      {
        annotate_info.pointsize=atoi(argv[++i]);
        continue;
      }
    if (strncmp("raise",option+1,2) == 0)
      {
        RaiseImage(*image,*option == '-' ? 1 : -1,atoi(argv[++i]));
        continue;
      }
    if (strncmp("-roll",option,4) == 0)
      {
        Image
          *rolled_image;

        unsigned int
          height,
          width;

        /*
          Roll image.
        */
        x=0;
        y=0;
        flags=XParseGeometry(argv[++i],&x,&y,&width,&height);
        rolled_image=RollImage(*image,x,y);
        if (rolled_image != (Image *) NULL)
          {
            DestroyImage(*image);
            *image=rolled_image;
          }
        continue;
      }
    if (strncmp("-rotate",option,4) == 0)
      {
        double
          degrees;

        Image
          *rotated_image;

        /*
          Rotate image.
        */
        degrees=atof(argv[++i]);
        rotated_image=RotateImage(*image,degrees,&border_color,False,True);
        if (rotated_image != (Image *) NULL)
          {
            DestroyImage(*image);
            *image=rotated_image;
          }
        continue;
      }
    if (strncmp("-sample",option,3) == 0)
      {
        Image
          *sampled_image;

        unsigned int
          height,
          width;

        /*
          Sample image with pixel replication.
        */
        width=(*image)->columns;
        height=(*image)->rows;
        ParseImageGeometry(argv[++i],&width,&height);
        sampled_image=SampleImage(*image,width,height);
        if (sampled_image != (Image *) NULL)
          {
            DestroyImage(*image);
            *image=sampled_image;
          }
        continue;
      }
    if (strncmp("scene",option+1,3) == 0)
      {
        (*image)->scene=atoi(argv[++i]);
        continue;
      }
    if (strncmp("segment",option+1,3) == 0)
      {
        SegmentImage(*image,colorspace,image_info->verbose,SmoothingThreshold,
          atof(argv[++i]));
        SyncImage(*image);
        continue;
      }
    if (strncmp("-sharpen",option,4) == 0)
      {
        double
          factor;

        Image
          *sharpened_image;

        /*
          Sharpen an image.
        */
        factor=atof(argv[++i]);
        sharpened_image=SharpenImage(*image,factor);
        if (sharpened_image != (Image *) NULL)
          {
            DestroyImage(*image);
            *image=sharpened_image;
          }
        continue;
      }
    if (strncmp("-shear",option,4) == 0)
      {
        float
          x_shear,
          y_shear;

        Image
          *sheared_image;

        /*
          Shear image.
        */
        x_shear=0.0;
        y_shear=0.0;
        (void) sscanf(argv[++i],"%fx%f",&x_shear,&y_shear);
        sheared_image=ShearImage(*image,(double) x_shear,
          (double) y_shear,&border_color,False);
        if (sheared_image != (Image *) NULL)
          {
            DestroyImage(*image);
            sheared_image->class=DirectClass;
            *image=sheared_image;
          }
        continue;
      }
    if (strncmp("-spread",option,3) == 0)
      {
        unsigned int
          amount;

        Image
          *spread_image;

        /*
          Spread an image.
        */
        amount=atoi(argv[++i]);
        spread_image=SpreadImage(*image,amount);
        if (spread_image != (Image *) NULL)
          {
            DestroyImage(*image);
            *image=spread_image;
          }
        continue;
      }
    if (strncmp("-transparency",option,4) == 0)
      {
        TransparentImage(*image,argv[++i]);
        continue;
      }
    if (strncmp("-treedepth",option,4) == 0)
      {
        tree_depth=atoi(argv[++i]);
        continue;
      }
  }
  if (number_colors != 0)
    {
      /*
        Reduce the number of colors in the image.
      */
      if (((*image)->class == DirectClass) ||
          ((*image)->colors > number_colors) || (colorspace == GRAYColorspace))
        QuantizeImage(*image,number_colors,tree_depth,image_info->dither,
          colorspace);
      /*
        Measure quantization error.
      */
      if (image_info->verbose)
        QuantizationError(*image);
      SyncImage(*image);
    }
  if ((*image)->packets == ((*image)->columns*(*image)->rows))
    CompressImage(*image);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%     N e g a t e I m a g e                                                   %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Function NegateImage negates the colors in the reference image.
%
%  The format of the NegateImage routine is:
%
%      NegateImage(image)
%
%  A description of each parameter follows:
%
%    o image: The address of a structure of type Image;  returned from
%      ReadImage.
%
%
*/
void NegateImage(Image *image)
{
#define NegateImageText  "  Negating the image colors...  "

  register int
    i;

  register RunlengthPacket
    *p;

  switch (image->class)
  {
    case DirectClass:
    {
      /*
        Negate DirectClass packets.
      */
      p=image->pixels;
      for (i=0; i < image->packets; i++)
      {
        p->red=(~p->red);
        p->green=(~p->green);
        p->blue=(~p->blue);
        p++;
        if (QuantumTick(i,image))
          ProgressMonitor(NegateImageText,i,image->packets);
      }
      break;
    }
    case PseudoClass:
    {
      /*
        Negate PseudoClass packets.
      */
      for (i=0; i < image->colors; i++)
      {
        image->colormap[i].red=(~image->colormap[i].red);
        image->colormap[i].green=(~image->colormap[i].green);
        image->colormap[i].blue=(~image->colormap[i].blue);
      }
      SyncImage(image);
      break;
    }
  }
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%     N o i s y I m a g e                                                     %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Function NoisyImage creates a new image that is a copy of an existing
%  one with the noise minified with a noise peak elimination filter.  It
%  allocates the memory necessary for the new Image structure and returns a
%  pointer to the new image.
%
%  The principal function of noise peak elimination filter is to smooth the
%  objects within an image without losing edge information and without
%  creating undesired structures.  The central idea of the algorithm is to
%  replace a pixel with its next neighbor in value within a 3 x 3 window,
%  if this pixel has been found to be noise.  A pixel is defined as noise
%  if and only if this pixel is a maximum or minimum within the 3 x 3
%  window.
%
%  The format of the NoisyImage routine is:
%
%      noisy_image=NoisyImage(image)
%
%  A description of each parameter follows:
%
%    o noisy_image: Function NoisyImage returns a pointer to the image after
%      the noise is minified.  A null image is returned if there is a memory
%      shortage.
%
%    o image: The address of a structure of type Image;  returned from
%      ReadImage.
%
%
*/
static int NoisyCompare(const void *x, const void *y)
{
  ColorPacket
    *color_1,
    *color_2;

  color_1=(ColorPacket *) x;
  color_2=(ColorPacket *) y;
  return((int) Intensity(*color_1)-(int) Intensity(*color_2));
}

Image *NoisyImage(Image *image)
{
#define NoisyImageText  "  Reducing the image noise...  "

  Image
    *noisy_image;

  int
    i;

  register RunlengthPacket
    *p,
    *q,
    *s,
    *s0,
    *s1,
    *s2;

  register unsigned int
    x;

  RunlengthPacket
    pixel,
    *scanline,
    window[9];

  unsigned int
    y;

  if ((image->columns < 3) || (image->rows < 3))
    {
      Warning("Unable to reduce noise","the image size must exceed 2x2");
      return((Image *) NULL);
    }
  /*
    Initialize noisy image attributes.
  */
  noisy_image=CopyImage(image,image->columns,image->rows,False);
  if (noisy_image == (Image *) NULL)
    {
      Warning("Unable to reduce noise","Memory allocation failed");
      return((Image *) NULL);
    }
  /*
    Allocate scanline buffer for 3 rows of the image.
  */
  scanline=(RunlengthPacket *) malloc(3*image->columns*sizeof(RunlengthPacket));
  if (scanline == (RunlengthPacket *) NULL)
    {
      Warning("Unable to reduce noise","Memory allocation failed");
      DestroyImage(noisy_image);
      return((Image *) NULL);
    }
  /*
    Preload the first 2 rows of the image.
  */
  p=image->pixels;
  image->runlength=p->length+1;
  s=scanline;
  for (x=0; x < (image->columns << 1); x++)
  {
    if (image->runlength != 0)
      image->runlength--;
    else
      {
        p++;
        image->runlength=p->length;
      }
    *s=(*p);
    s++;
  }
  /*
    Dump first scanline of image.
  */
  q=noisy_image->pixels;
  s=scanline;
  for (x=0; x < image->columns; x++)
  {
    *q=(*s);
    q->length=0;
    q++;
    s++;
  }
  /*
    Reduce noise in each row.
  */
  for (y=1; y < (image->rows-1); y++)
  {
    /*
      Initialize sliding window pointers.
    */
    s0=scanline+image->columns*((y-1) % 3);
    s1=scanline+image->columns*(y % 3);
    s2=scanline+image->columns*((y+1) % 3);
    /*
      Read another scan line.
    */
    s=s2;
    for (x=0; x < image->columns; x++)
    {
      if (image->runlength != 0)
        image->runlength--;
      else
        {
          p++;
          image->runlength=p->length;
        }
      *s=(*p);
      s++;
    }
    /*
      Transfer first pixel of the scanline.
    */
    s=s1;
    *q=(*s);
    q->length=0;
    q++;
    for (x=1; x < (image->columns-1); x++)
    {
      /*
        Sort window pixels by increasing intensity.
      */
      s=s0;
      window[0]=(*s++);
      window[1]=(*s++);
      window[2]=(*s++);
      s=s1;
      window[3]=(*s++);
      window[4]=(*s++);
      window[5]=(*s++);
      s=s2;
      window[6]=(*s++);
      window[7]=(*s++);
      window[8]=(*s++);
      pixel=window[4];
      qsort((void *) window,9,sizeof(RunlengthPacket),
        (int (*) _Declare((const void *, const void *))) NoisyCompare);
      if (Intensity(pixel) == Intensity(window[0]))
        {
          /*
            Pixel is minimum noise; replace with next neighbor in value.
          */
          for (i=1; i < 8; i++)
            if (Intensity(window[i]) != Intensity(window[0]))
              break;
          pixel=window[i];
        }
      else
        if (Intensity(pixel) == Intensity(window[8]))
          {
            /*
              Pixel is maximum noise; replace with next neighbor in value.
            */
            for (i=7; i > 0; i--)
              if (Intensity(window[i]) != Intensity(window[8]))
                break;
            pixel=window[i];
          }
      *q=pixel;
      q->length=0;
      q++;
      s0++;
      s1++;
      s2++;
    }
    /*
      Transfer last pixel of the scanline.
    */
    s=s1;
    *q=(*s);
    q->length=0;
    q++;
    ProgressMonitor(NoisyImageText,y,image->rows-1);
  }
  /*
    Dump last scanline of pixels.
  */
  s=scanline+image->columns*(y % 3);
  for (x=0; x < image->columns; x++)
  {
    *q=(*s);
    q->length=0;
    q++;
    s++;
  }
  free((char *) scanline);
  return(noisy_image);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%     N o r m a l i z e I m a g e                                             %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Function NormalizeImage normalizes the pixel values to span the full
%  range of color values.  This is a contrast enhancement technique.
%
%  The format of the NormalizeImage routine is:
%
%      NormalizeImage(image)
%
%  A description of each parameter follows:
%
%    o image: The address of a structure of type Image;  returned from
%      ReadImage.
%
%
*/
void NormalizeImage(Image *image)
{
#define NormalizeImageText  "  Normalizing image...  "

  int
    histogram[MaxRGB+1],
    threshold_intensity;

  Quantum
    gray_value,
    normalize_map[MaxRGB+1];

  register int
    i,
    intensity;

  register RunlengthPacket
    *p;

  unsigned int
    high,
    low;

  /*
    Form histogram.
  */
  for (i=0; i <= MaxRGB; i++)
    histogram[i]=0;
  p=image->pixels;
  for (i=0; i < image->packets; i++)
  {
    gray_value=Intensity(*p);
    histogram[gray_value]+=p->length+1;
    p++;
  }
  /*
    Find the histogram boundaries by locating the 1 percent levels.
  */
  threshold_intensity=(image->columns*image->rows)/100;
  intensity=0;
  for (low=0; low < MaxRGB; low++)
  {
    intensity+=histogram[low];
    if (intensity > threshold_intensity)
      break;
  }
  intensity=0;
  for (high=MaxRGB; high != 0; high--)
  {
    intensity+=histogram[high];
    if (intensity > threshold_intensity)
      break;
  }
  if (low == high)
    {
      /*
        Unreasonable contrast;  use zero threshold to determine boundaries.
      */
      threshold_intensity=0;
      intensity=0;
      for (low=0; low < MaxRGB; low++)
      {
        intensity+=histogram[low];
        if (intensity > threshold_intensity)
          break;
      }
      intensity=0;
      for (high=MaxRGB; high != 0; high--)
      {
        intensity+=histogram[high];
        if (intensity > threshold_intensity)
          break;
      }
      if (low == high)
        return;  /* zero span bound */
    }
  /*
    Stretch the histogram to create the normalized image mapping.
  */
  for (i=0; i <= MaxRGB; i++)
    if (i < (int) low)
      normalize_map[i]=0;
    else
      if (i > (int) high)
        normalize_map[i]=MaxRGB-1;
      else
        normalize_map[i]=(MaxRGB-1)*(i-low)/(high-low);
  /*
    Normalize the image.
  */
  switch (image->class)
  {
    case DirectClass:
    {
      /*
        Normalize DirectClass image.
      */
      p=image->pixels;
      for (i=0; i < image->packets; i++)
      {
        p->red=normalize_map[p->red];
        p->green=normalize_map[p->green];
        p->blue=normalize_map[p->blue];
        p++;
        if (QuantumTick(i,image))
          ProgressMonitor(NormalizeImageText,i,image->packets);
      }
      break;
    }
    case PseudoClass:
    {
      /*
        Normalize PseudoClass image.
      */
      for (i=0; i < image->colors; i++)
      {
        image->colormap[i].red=normalize_map[image->colormap[i].red];
        image->colormap[i].green=normalize_map[image->colormap[i].green];
        image->colormap[i].blue=normalize_map[image->colormap[i].blue];
      }
      SyncImage(image);
      break;
    }
  }
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%     O p a g u e I m a g e                                                   %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Function OpaqueImage changes the color of an opaque pixel to the pen color.
%
%  The format of the OpaqueImage routine is:
%
%      OpaqueImage(image,opaque_color,pen_colors)
%
%  A description of each parameter follows:
%
%    o image: The address of a structure of type Image;  returned from
%      ReadImage.
%
%    o opaque_color,
%      pen_colors: A character string that contain an X11 color string.
%
%
*/
void OpaqueImage(Image *image, char *opaque_color, char *pen_colors)
{
#define DeltaX  16
#define OpaqueImageText  "  Setting opaque color in the image...  "

  Quantum
    blue,
    green,
    red;

  register int
    i;

  register RunlengthPacket
    *p;

  unsigned int
    status;

  XColor
    target_color;

  /*
    Determine RGB values of the opaque color.
  */
  status=XQueryColorDatabase(opaque_color,&target_color);
  if (status == False)
    return;
  red=XDownScale(target_color.red);
  green=XDownScale(target_color.green);
  blue=XDownScale(target_color.blue);
  status=XQueryColorDatabase(pen_colors,&target_color);
  if (status == False)
    return;
  /*
    Make image color opaque.
  */
  p=image->pixels;
  switch (image->class)
  {
    case DirectClass:
    {
      /*
        Make DirectClass image opaque.
      */
      for (i=0; i < image->packets; i++)
      {
        if (((int) p->red < (int) (red+DeltaX)) &&
            ((int) p->red > (int) (red-DeltaX)) &&
            ((int) p->green < (int) (green+DeltaX)) &&
            ((int) p->green > (int) (green-DeltaX)) &&
            ((int) p->blue < (int) (blue+DeltaX)) &&
            ((int) p->blue > (int) (blue-DeltaX)))
          {
            p->red=XDownScale(target_color.red);
            p->green=XDownScale(target_color.green);
            p->blue=XDownScale(target_color.blue);
          }
        p++;
        if (QuantumTick(i,image))
          ProgressMonitor(OpaqueImageText,i,image->packets);
      }
      break;
    }
    case PseudoClass:
    {
      double
        distance_squared,
        min_distance;

      int
        distance;

      register int
        index;

      /*
        Find closest color.
      */
      min_distance=3.0*(MaxRGB+1)*(MaxRGB+1);
      index=0;
      for (i=0; i < image->colors; i++)
      {
        distance=(int) red-(int) image->colormap[i].red;
        distance_squared=(unsigned int) (distance*distance);
        distance=(int) green-(int) image->colormap[i].green;
        distance_squared+=(unsigned int) (distance*distance);
        distance=(int) blue-(int) image->colormap[i].blue;
        distance_squared+=(unsigned int) (distance*distance);
        if (distance_squared < min_distance)
          {
            min_distance=distance_squared;
            index=i;
          }
      }
      /*
        Make PseudoClass image opaque.
      */
      image->colormap[index].red=XDownScale(target_color.red);
      image->colormap[index].green=XDownScale(target_color.green);
      image->colormap[index].blue=XDownScale(target_color.blue);
      break;
    }
  }
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   O p e n I m a g e                                                         %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Function OpenImage open a file associated with the image.  A file name of
%  '-' sets the file to stdin for type 'r' and stdout for type 'w'.  If the
%  filename suffix is '.gz' or '.Z', the image is decompressed for type 'r'
%  and compressed for type 'w'.  If the filename prefix is '|', it is piped
%  to or from a system command.
%
%  The format of the OpenImage routine is:
%
%      OpenImage(image,type)
%
%  A description of each parameter follows:
%
%    o image: The address of a structure of type Image.
%
%    o type: 'r' for reading; 'w' for writing.
%
*/
void OpenImage(Image *image, char *type)
{
  char
    filename[MaxTextLength];

  (void) strcpy(filename,image->filename);
  if (*filename != '|')
    if (((int) strlen(filename) > 3) &&
        (strcmp(filename+strlen(filename)-3,".gz") == 0))
      {
        /*
          Uncompress/compress image file with GNU compress utilities.
        */
        if (*type == 'r')
          (void) sprintf(filename,GunzipCommand,image->filename);
        else
          (void) sprintf(filename,GzipCommand,image->filename);
      }
    else
      if (((int) strlen(filename) > 2) &&
          (strcmp(filename+strlen(filename)-2,".Z") == 0))
        {
          /*
            Uncompress/compress image file with UNIX compress utilities.
          */
          if (*type == 'r')
            (void) sprintf(filename,UncompressCommand,image->filename);
          else
            (void) sprintf(filename,CompressCommand,image->filename);
        }
  /*
    Open image file.
  */
  image->pipe=False;
  if (strcmp(filename,"-") == 0)
    image->file=(*type == 'r') ? stdin : stdout;
  else
    if (*filename != '|')
      {
        if (*type == 'w')
          if ((image->previous != (Image *) NULL) ||
              (image->next != (Image *) NULL))
            {
              /*
                Form filename for multi-part images.
              */
              (void) sprintf(filename,image->filename,image->scene);
              if (strcmp(filename,image->filename) == 0)
                (void) sprintf(filename,"%s.%u",image->filename,image->scene);
              if (image->next != (Image *) NULL)
                (void) strcpy(image->next->magick,image->magick);
              (void) strcpy(image->filename,filename);
            }
        image->file=(FILE *) fopen(filename,type);
        if (image->file != (FILE *) NULL)
          {
            (void) fseek(image->file,0L,2);
            image->filesize=ftell(image->file);
            (void) fseek(image->file,0L,0);
          }
      }
    else
      {
        /*
          Pipe image to or from a system command.
        */
        if (*type == 'w')
          (void) signal(SIGPIPE,SIG_IGN);
        image->file=(FILE *) popen(filename+1,type);
        image->pipe=True;
      }
  image->status=False;
  if (*type == 'r')
    {
      image->next=(Image *) NULL;
      image->previous=(Image *) NULL;
    }
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   P a r s e I m a g e G e o m e t r y                                       %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Function ParseImageGeometry parse a geometry specification and returns the
%  width and height values.
%
%  The format of the ParseImageGeometry routine is:
%
%      ParseImageGeometry(image_geometry,width,height)
%
%  A description of each parameter follows:
%
%    o image_geometry:  Specifies a character string representing the geometry
%      specification.
%
%    o width:  A pointer to an unsigned integer.  The width as determined by
%      the geometry specification is returned here.
%
%    o height:  A pointer to an unsigned integer.  The height as determined by
%      the geometry specification is returned here.
%
%
*/
void ParseImageGeometry(char *image_geometry, unsigned int *width, unsigned int *height)
{
  char
    geometry[MaxTextLength];

  int
    flags,
    x,
    y;

  register char
    *p;

  unsigned int
    aspect_ratio,
    former_height,
    former_width,
    greater,
    less,
    percentage;

  unsigned long
    scale_factor;

  if (image_geometry == (char *) NULL)
    return;
  /*
    Remove whitespaces and % and ! characters from geometry specification.
  */
  (void) strcpy(geometry,image_geometry);
  aspect_ratio=True;
  greater=False;
  less=False;
  percentage=False;
  p=geometry;
  while ((int) strlen(p) > 0)
  {
    if (isspace(*p))
      (void) strcpy(p,p+1);
    else
      switch (*p)
      {
        case '%':
        {
          percentage=True;
          (void) strcpy(p,p+1);
          break;
        }
        case '!':
        {
          aspect_ratio=False;
          (void) strcpy(p,p+1);
          break;
        }
        case '<':
        {
          less=True;
          (void) strcpy(p,p+1);
          break;
        }
        case '>':
        {
          greater=True;
          (void) strcpy(p,p+1);
          break;
        }
        default:
          p++;
      }
  }
  /*
    Parse geometry using XParseGeometry.
  */
  former_width=(*width);
  former_height=(*height);
  flags=XParseGeometry(geometry,&x,&y,width,height);
  if (((flags & WidthValue) != 0) && (flags & HeightValue) == 0)
    *height=(*width);
  if (percentage)
    {
      /*
        Geometry is a percentage of the image size.
      */
      *width=(former_width*(*width))/100;
      *height=(former_height*(*height))/100;
      former_width=(*width);
      former_height=(*height);
    }
  if (aspect_ratio)
    {
      /*
        Respect aspect ratio of the image.
      */
      scale_factor=UpShift(*width)/former_width;
      if (scale_factor > (UpShift(*height)/former_height))
        scale_factor=UpShift(*height)/former_height;
      *width=DownShift(former_width*scale_factor);
      *height=DownShift(former_height*scale_factor);
    }
  if (greater)
    if ((former_width < *width) && (former_height < *height))
      {
        *width=former_width;
        *height=former_height;
      }
  if (less)
    if ((former_width > *width) && (former_height > *height))
      {
        *width=former_width;
        *height=former_height;
      }
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%     O i l P a i n t I m a g e                                               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Function OilPaintImage creates a new image that is a copy of an existing
%  one with each pixel component replaced with the color of greatest number in
%  a 5x5 neighborhood.
%
%  The format of the OilPaintImage routine is:
%
%      painted_image=OilPaintImage(image)
%
%  A description of each parameter follows:
%
%    o painted_image: Function OilPaintImage returns a pointer to the image
%      after it is `painted'.  A null image is returned if there is a memory
%      shortage.
%
%    o image: The address of a structure of type Image;  returned from
%      ReadImage.
%
%
*/
Image *OilPaintImage(Image *image)
{
#define OilPaintImageText  "  Oil painting image...  "

  Image
    *painted_image;

  int
    count;

  register int
    i,
    j;

  register RunlengthPacket
    *p,
    *q,
    *s,
    *s0,
    *s1,
    *s2,
    *s3,
    *s4;

  register unsigned int
    x;

  RunlengthPacket
    pixel,
    *scanline,
    window[25];

  unsigned int
    *histogram,
    y;

  if ((image->columns < 5) || (image->rows < 5))
    {
      Warning("Unable to oil paint","the image size must exceed 4x4");
      return((Image *) NULL);
    }
  /*
    Initialize painted image attributes.
  */
  painted_image=CopyImage(image,image->columns,image->rows,False);
  if (painted_image == (Image *) NULL)
    {
      Warning("Unable to oil paint","Memory allocation failed");
      return((Image *) NULL);
    }
  painted_image->class=DirectClass;
  /*
    Allocate histogram and scanline.
  */
  histogram=(unsigned int *) malloc((MaxRGB+1)*sizeof(unsigned int));
  scanline=(RunlengthPacket *) malloc(5*image->columns*sizeof(RunlengthPacket));
  if ((histogram == (unsigned int *) NULL) ||
      (scanline == (RunlengthPacket *) NULL))
    {
      Warning("Unable to oil paint","Memory allocation failed");
      DestroyImage(painted_image);
      return((Image *) NULL);
    }
  /*
    Preload the first 4 rows of the image.
  */
  p=image->pixels;
  image->runlength=p->length+1;
  s=scanline;
  for (x=0; x < (image->columns << 2); x++)
  {
    if (image->runlength != 0)
      image->runlength--;
    else
      {
        p++;
        image->runlength=p->length;
      }
    *s=(*p);
    s++;
  }
  /*
    Dump first scanline of image.
  */
  q=painted_image->pixels;
  s=scanline;
  for (x=0; x < (image->columns << 1); x++)
  {
    *q=(*s);
    q->length=0;
    q++;
    s++;
  }
  /*
    Paint each row of the image.
  */
  for (y=2; y < (image->rows-2); y++)
  {
    /*
      Initialize sliding window pointers.
    */
    s0=scanline+image->columns*((y-2) % 5);
    s1=scanline+image->columns*((y-1) % 5);
    s2=scanline+image->columns*(y % 5);
    s3=scanline+image->columns*((y+1) % 5);
    s4=scanline+image->columns*((y+2) % 5);
    /*
      Read another scan line.
    */
    s=s4;
    for (x=0; x < image->columns; x++)
    {
      if (image->runlength != 0)
        image->runlength--;
      else
        {
          p++;
          image->runlength=p->length;
        }
      *s=(*p);
      s++;
    }
    /*
      Transfer first pixel of the scanline.
    */
    s=s2;
    *q=(*s++);
    q->length=0;
    q++;
    *q=(*s++);
    q->length=0;
    q++;
    for (x=2; x < (image->columns-2); x++)
    {
      /*
        Note each pixel in a 5x5 neighborhood.
      */
      s=s0;
      window[0]=(*s++);
      window[1]=(*s++);
      window[2]=(*s++);
      window[3]=(*s++);
      window[4]=(*s++);
      s=s1;
      window[5]=(*s++);
      window[6]=(*s++);
      window[7]=(*s++);
      window[8]=(*s++);
      window[9]=(*s++);
      s=s2;
      window[10]=(*s++);
      window[11]=(*s++);
      window[12]=(*s++);
      window[13]=(*s++);
      window[14]=(*s++);
      s=s3;
      window[15]=(*s++);
      window[16]=(*s++);
      window[17]=(*s++);
      window[18]=(*s++);
      window[19]=(*s++);
      s=s4;
      window[20]=(*s++);
      window[21]=(*s++);
      window[22]=(*s++);
      window[23]=(*s++);
      window[24]=(*s++);
      /*
        Determine most frequent color.
      */
      count=0;
      for (i=0; i < (MaxRGB+1); i++)
        histogram[i]=0;
      for (i=0; i < 25; i++)
      {
         j=Intensity(window[i]) >> 2;
         histogram[j]++;
         j=histogram[j];
         if (j > count)
           {
             pixel=window[i];
             count=j;
           }
      }
      *q=pixel;
      q->length=0;
      q++;
      s0++;
      s1++;
      s2++;
      s3++;
      s4++;
    }
    /*
      Transfer last pixel of the scanline.
    */
    s=s2;
    *q=(*s++);
    q->length=0;
    q++;
    *q=(*s++);
    q->length=0;
    q++;
    ProgressMonitor(OilPaintImageText,y,image->rows-2);
  }
  /*
    Dump last scanline of pixels.
  */
  s=scanline+image->columns*(y % 5);
  for (x=0; x < (image->columns << 1); x++)
  {
    *q=(*s);
    q->length=0;
    q++;
    s++;
  }
  free((char *) histogram);
  free((char *) scanline);
  return(painted_image);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   R a i s e I m a g e                                                       %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Function RaiseImage lightens and darkens the edges of an image to give a
%  3-D raised or lower effect.
%
%  The format of the RaiseImage routine is:
%
%      RaiseImage(image,raised,bevel_width)
%
%  A description of each parameter follows:
%
%    o image: The address of a structure of type Image.
%
%    o raised: A value other than zero causes the image to have a 3-D raised
%      effect, otherwise it has a lowered effect.
%
%    o bevel_width: This unsigned integer defines the width of the 3-D effect.
%
%
*/
void RaiseImage(Image *image, int raised, unsigned int bevel_width)
{
#define HighlightLeft  UpScale(160)
#define HighlightTop  UpScale(180)
#define RaiseImageText  "  Raising image...  "
#define ShadowRight  UpScale(180)
#define ShadowBottom  UpScale(160)

  Quantum
    foreground,
    background;

  register int
    x,
    y;

  register RunlengthPacket
    *p;

  unsigned int
    height;

  if ((image->columns < (bevel_width << 1)) &&
      (image->rows < (bevel_width << 1)))
    {
      Warning("Unable to raise image","image size must exceed bevel width");
      return;
    }
  if (!UncompressImage(image))
    return;
  foreground=MaxRGB;
  background=0;
  if (!raised)
    {
      foreground=0;
      background=MaxRGB;
    }
  image->class=DirectClass;
  p=image->pixels;
  for (y=0; y < bevel_width; y++)
  {
    for (x=0; x < y; x++)
    {
      p->red=(unsigned int)
        (p->red*HighlightLeft+foreground*(MaxRGB-HighlightLeft))/MaxRGB;
      p->green=(unsigned int)
        (p->green*HighlightLeft+foreground*(MaxRGB-HighlightLeft))/MaxRGB;
      p->blue=(unsigned int)
        (p->blue*HighlightLeft+foreground*(MaxRGB-HighlightLeft))/MaxRGB;
      p++;
    }
    for (x=0; x < (image->columns-(y << 1)); x++)
    {
      p->red=(unsigned int)
        (p->red*HighlightTop+foreground*(MaxRGB-HighlightTop))/MaxRGB;
      p->green=(unsigned int)
        (p->green*HighlightTop+foreground*(MaxRGB-HighlightTop))/MaxRGB;
      p->blue=(unsigned int)
        (p->blue*HighlightTop+foreground*(MaxRGB-HighlightTop))/MaxRGB;
      p++;
    }
    for (x=0; x < y; x++)
    {
      p->red=(unsigned int)
        (p->red*ShadowRight+background*(MaxRGB-ShadowRight))/MaxRGB;
      p->green=(unsigned int)
        (p->green*ShadowRight+background*(MaxRGB-ShadowRight))/MaxRGB;
      p->blue=(unsigned int)
        (p->blue*ShadowRight+background*(MaxRGB-ShadowRight))/MaxRGB;
      p++;
    }
  }
  height=image->rows-(bevel_width << 1);
  for (y=0; y < height; y++)
  {
    for (x=0; x < bevel_width; x++)
    {
      p->red=(unsigned int)
        (p->red*HighlightLeft+foreground*(MaxRGB-HighlightLeft))/MaxRGB;
      p->green=(unsigned int)
        (p->green*HighlightLeft+foreground*(MaxRGB-HighlightLeft))/MaxRGB;
      p->blue=(unsigned int)
        (p->blue*HighlightLeft+foreground*(MaxRGB-HighlightLeft))/MaxRGB;
      p++;
    }
    for (x=0; x < (image->columns-(bevel_width << 1)); x++)
      p++;
    for (x=0; x < bevel_width; x++)
    {
      p->red=(unsigned int)
        (p->red*ShadowRight+background*(MaxRGB-ShadowRight))/MaxRGB;
      p->green=(unsigned int)
        (p->green*ShadowRight+background*(MaxRGB-ShadowRight))/MaxRGB;
      p->blue=(unsigned int)
        (p->blue*ShadowRight+background*(MaxRGB-ShadowRight))/MaxRGB;
      p++;
    }
    ProgressMonitor(RaiseImageText,y,height);
  }
  for (y=0; y < bevel_width; y++)
  {
    for (x=0; x < (bevel_width-y); x++)
    {
      p->red=(unsigned int)
        (p->red*HighlightLeft+foreground*(MaxRGB-HighlightLeft))/MaxRGB;
      p->green=(unsigned int)
        (p->green*HighlightLeft+foreground*(MaxRGB-HighlightLeft))/MaxRGB;
      p->blue=(unsigned int)
        (p->blue*HighlightLeft+foreground*(MaxRGB-HighlightLeft))/MaxRGB;
      p++;
    }
    for (x=0; x < (image->columns-((bevel_width-y) << 1)); x++)
    {
      p->red=(unsigned int)
        (p->red*ShadowBottom+background*(MaxRGB-ShadowBottom))/MaxRGB;
      p->green=(unsigned int)
        (p->green*ShadowBottom+background*(MaxRGB-ShadowBottom))/MaxRGB;
      p->blue=(unsigned int)
        (p->blue*ShadowBottom+background*(MaxRGB-ShadowBottom))/MaxRGB;
      p++;
    }
    for (x=0; x < (bevel_width-y); x++)
    {
      p->red=(unsigned int)
        (p->red*ShadowRight+background*(MaxRGB-ShadowRight))/MaxRGB;
      p->green=(unsigned int)
        (p->green*ShadowRight+background*(MaxRGB-ShadowRight))/MaxRGB;
      p->blue=(unsigned int)
        (p->blue*ShadowRight+background*(MaxRGB-ShadowRight))/MaxRGB;
      p++;
    }
  }
  return;
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%     R G B T r a n s f o r m I m a g e                                       %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Function RGBTransformImage converts the reference image from RGB to
%  an alternate colorspace.  The transformation matrices are not the standard
%  ones: the weights are rescaled to normalized the range of the transformed
%  values to be [0..MaxRGB].
%
%  The format of the RGBTransformImage routine is:
%
%      RGBTransformImage(image,colorspace)
%
%  A description of each parameter follows:
%
%    o image: The address of a structure of type Image;  returned from
%      ReadImage.
%
%    o colorspace: An unsigned integer value that indicates which colorspace
%      to transform the image.
%
%
*/
void RGBTransformImage(Image *image, unsigned int colorspace)
{
#define RGBTransformImageText  "  Transforming image colors...  "
#define X 0
#define Y (MaxRGB+1)
#define Z (MaxRGB+1)*2

  long
    tx,
    ty,
    tz,
    *x,
    *y,
    *z;

  Quantum
    *range_table;

  register int
    blue,
    green,
    i,
    red;

  register Quantum
    *range_limit;

  register RunlengthPacket
    *p;

  if (colorspace == RGBColorspace)
    return;
  if (colorspace == GRAYColorspace)
    {
      /*
        Return if the image is already grayscale.
      */
      p=image->pixels;
      for (i=0; i < image->packets; i++)
      {
        if ((p->red != p->green) || (p->green != p->blue))
          break;
        p++;
      }
      if (i == image->packets)
        return;
    }
  /*
    Allocate the tables.
  */
  x=(long *) malloc(3*(MaxRGB+1)*sizeof(long));
  y=(long *) malloc(3*(MaxRGB+1)*sizeof(long));
  z=(long *) malloc(3*(MaxRGB+1)*sizeof(long));
  range_table=(Quantum *) malloc(3*(MaxRGB+1)*sizeof(Quantum));
  if ((x == (long *) NULL) || (y == (long *) NULL) ||
      (z == (long *) NULL) || (range_table == (Quantum *) NULL))
    {
      Warning("Unable to transform color space","Memory allocation failed");
      return;
    }
  /*
    Pre-compute conversion tables.
  */
  for (i=0; i <= MaxRGB; i++)
  {
    range_table[i]=0;
    range_table[i+(MaxRGB+1)]=(Quantum) i;
    range_table[i+(MaxRGB+1)*2]=MaxRGB;
  }
  range_limit=range_table+(MaxRGB+1);
  tx=0;
  ty=0;
  tz=0;
  switch (colorspace)
  {
    case GRAYColorspace:
    {
      /*
        Initialize GRAY tables:

          G = 0.29900*R+0.58600*G+0.11400*B
      */
      for (i=0; i <= MaxRGB; i++)
      {
        x[i+X]=UpShifted(0.29900)*i;
        y[i+X]=UpShifted(0.58600)*i;
        z[i+X]=UpShifted(0.11400)*i;
        x[i+Y]=UpShifted(0.29900)*i;
        y[i+Y]=UpShifted(0.58600)*i;
        z[i+Y]=UpShifted(0.11400)*i;
        x[i+Z]=UpShifted(0.29900)*i;
        y[i+Z]=UpShifted(0.58600)*i;
        z[i+Z]=UpShifted(0.11400)*i;
      }
      break;
    }
    case OHTAColorspace:
    {
      /*
        Initialize OHTA tables:

          I1 = 0.33333*R+0.33334*G+0.33333*B
          I2 = 0.50000*R+0.00000*G-0.50000*B
          I3 =-0.25000*R+0.50000*G-0.25000*B

        I and Q, normally -0.5 through 0.5, are normalized to the range 0
        through MaxRGB.
      */
      ty=UpShifted((MaxRGB+1) >> 1);
      tz=UpShifted((MaxRGB+1) >> 1);
      for (i=0; i <= MaxRGB; i++)
      {
        x[i+X]=UpShifted(0.33333)*i;
        y[i+X]=UpShifted(0.33334)*i;
        z[i+X]=UpShifted(0.33333)*i;
        x[i+Y]=UpShifted(0.50000)*i;
        y[i+Y]=0;
        z[i+Y]=(-UpShifted(0.50000))*i;
        x[i+Z]=(-UpShifted(0.25000))*i;
        y[i+Z]=UpShifted(0.50000)*i;
        z[i+Z]=(-UpShifted(0.25000))*i;
      }
      break;
    }
    case XYZColorspace:
    {
      /*
        Initialize CIE XYZ tables:

          X = 0.412453*X+0.357580*Y+0.180423*Z
          Y = 0.212671*X+0.715160*Y+0.072169*Z
          Z = 0.019334*X+0.119193*Y+0.950227*Z
      */
      for (i=0; i <= MaxRGB; i++)
      {
        x[i+X]=UpShifted(0.412453)*i;
        y[i+X]=UpShifted(0.357580)*i;
        z[i+X]=UpShifted(0.180423)*i;
        x[i+Y]=UpShifted(0.212671)*i;
        y[i+Y]=UpShifted(0.715160)*i;
        z[i+Y]=UpShifted(0.072169)*i;
        x[i+Z]=UpShifted(0.019334)*i;
        y[i+Z]=UpShifted(0.119193)*i;
        z[i+Z]=UpShifted(0.950227)*i;
      }
      break;
    }
    case YCbCrColorspace:
    {
      /*
        Initialize YCbCr tables:

          Y =  0.299000*R+0.586000*G+0.114000*B
          Cb= -0.172586*R-0.338828*G+0.511414*B
          Cr=  0.511414*R-0.428246*G-0.083168*B

        Cb and Cr, normally -0.5 through 0.5, are normalized to the range 0
        through MaxRGB.
      */
      ty=UpShifted((MaxRGB+1) >> 1);
      tz=UpShifted((MaxRGB+1) >> 1);
      for (i=0; i <= MaxRGB; i++)
      {
        x[i+X]=UpShifted(0.299000)*i;
        y[i+X]=UpShifted(0.586000)*i;
        z[i+X]=UpShifted(0.114000)*i;
        x[i+Y]=(-UpShifted(0.172586))*i;
        y[i+Y]=(-UpShifted(0.338828))*i;
        z[i+Y]=UpShifted(0.511414)*i;
        x[i+Z]=UpShifted(0.511414)*i;
        y[i+Z]=(-UpShifted(0.428246))*i;
        z[i+Z]=(-UpShifted(0.083168))*i;
      }
      break;
    }
    case YCCColorspace:
    {
      /*
        Initialize YCC tables:

          Y = 0.29900*R+0.58600*G+0.11400*B
          C1=-0.29900*R-0.58600*G+0.88600*B
          C2= 0.70100*R-0.58600*G+0.11400*B

        YCC is scaled by 1.3584.  C1 zero is 156 and C2 is at 137.
      */
      ty=UpShifted((unsigned int) UpScale(156));
      tz=UpShifted((unsigned int) UpScale(137));
      for (i=0; i <= MaxRGB; i++)
      {
        x[i+X]=UpShifted(0.29900)*i;
        y[i+X]=UpShifted(0.58600)*i;
        z[i+X]=UpShifted(0.11400)*i;
        x[i+Y]=(-UpShifted(0.29900))*i;
        y[i+Y]=(-UpShifted(0.58600))*i;
        z[i+Y]=UpShifted(0.88600)*i;
        x[i+Z]=UpShifted(0.70100)*i;
        y[i+Z]=(-UpShifted(0.58600))*i;
        z[i+Z]=(-UpShifted(0.11400))*i;
      }
      break;
    }
    case YIQColorspace:
    {
      /*
        Initialize YIQ tables:

          Y = 0.29900*R+0.58600*G+0.11400*B
          I = 0.50000*R-0.23000*G-0.27000*B
          Q = 0.20200*R-0.50000*G+0.29800*B

        I and Q, normally -0.5 through 0.5, are normalized to the range 0
        through MaxRGB.
      */
      ty=UpShifted((MaxRGB+1) >> 1);
      tz=UpShifted((MaxRGB+1) >> 1);
      for (i=0; i <= MaxRGB; i++)
      {
        x[i+X]=UpShifted(0.29900)*i;
        y[i+X]=UpShifted(0.58600)*i;
        z[i+X]=UpShifted(0.11400)*i;
        x[i+Y]=UpShifted(0.50000)*i;
        y[i+Y]=(-UpShifted(0.23000))*i;
        z[i+Y]=(-UpShifted(0.27000))*i;
        x[i+Z]=UpShifted(0.20200)*i;
        y[i+Z]=(-UpShifted(0.50000))*i;
        z[i+Z]=UpShifted(0.29800)*i;
      }
      break;
    }
    case YPbPrColorspace:
    {
      /*
        Initialize YPbPr tables:

          Y =  0.299000*R+0.587000*G+0.114000*B
          Pb= -0.168736*R-0.331264*G+0.500000*B
          Pr=  0.500000*R-0.418688*G-0.081312*B

        Pb and Pr, normally -0.5 through 0.5, are normalized to the range 0
        through MaxRGB.
      */
      ty=UpShifted((MaxRGB+1) >> 1);
      tz=UpShifted((MaxRGB+1) >> 1);
      for (i=0; i <= MaxRGB; i++)
      {
        x[i+X]=UpShifted(0.299000)*i;
        y[i+X]=UpShifted(0.587000)*i;
        z[i+X]=UpShifted(0.114000)*i;
        x[i+Y]=(-UpShifted(0.168736))*i;
        y[i+Y]=(-UpShifted(0.331264))*i;
        z[i+Y]=UpShifted(0.500000)*i;
        x[i+Z]=UpShifted(0.500000)*i;
        y[i+Z]=(-UpShifted(0.418688))*i;
        z[i+Z]=(-UpShifted(0.081312))*i;
      }
      break;
    }
    case YUVColorspace:
    default:
    {
      /*
        Initialize YUV tables:

          Y =  0.29900*R+0.58600*G+0.11400*B
          U = -0.14740*R-0.28950*G+0.43690*B
          V =  0.61500*R-0.51500*G-0.10000*B

        U and V, normally -0.5 through 0.5, are normalized to the range 0
        through MaxRGB.  Note that U = 0.493*(B-Y), V = 0.877*(R-Y).
      */
      ty=UpShifted((MaxRGB+1) >> 1);
      tz=UpShifted((MaxRGB+1) >> 1);
      for (i=0; i <= MaxRGB; i++)
      {
        x[i+X]=UpShifted(0.29900)*i;
        y[i+X]=UpShifted(0.58600)*i;
        z[i+X]=UpShifted(0.11400)*i;
        x[i+Y]=(-UpShifted(0.14740))*i;
        y[i+Y]=(-UpShifted(0.28950))*i;
        z[i+Y]=UpShifted(0.43690)*i;
        x[i+Z]=UpShifted(0.61500)*i;
        y[i+Z]=(-UpShifted(0.51500))*i;
        z[i+Z]=(-UpShifted(0.10000))*i;
      }
      break;
    }
  }
  /*
    Convert from RGB.
  */
  switch (image->class)
  {
    case DirectClass:
    {
      /*
        Convert DirectClass image.
      */
      p=image->pixels;
      for (i=0; i < image->packets; i++)
      {
        red=p->red;
        green=p->green;
        blue=p->blue;
        p->red=range_limit[DownShift(x[red+X]+y[green+X]+z[blue+X]+tx)];
        p->green=range_limit[DownShift(x[red+Y]+y[green+Y]+z[blue+Y]+ty)];
        p->blue=range_limit[DownShift(x[red+Z]+y[green+Z]+z[blue+Z]+tz)];
        p++;
        if (QuantumTick(i,image))
          ProgressMonitor(RGBTransformImageText,i,image->packets);
      }
      break;
    }
    case PseudoClass:
    {
      /*
        Convert PseudoClass image.
      */
      for (i=0; i < image->colors; i++)
      {
        red=image->colormap[i].red;
        green=image->colormap[i].green;
        blue=image->colormap[i].blue;
        image->colormap[i].red=
          range_limit[DownShift(x[red+X]+y[green+X]+z[blue+X]+tx)];
        image->colormap[i].green=
          range_limit[DownShift(x[red+Y]+y[green+Y]+z[blue+Y]+ty)];
        image->colormap[i].blue=
          range_limit[DownShift(x[red+Z]+y[green+Z]+z[blue+Z]+tz)];
      }
      SyncImage(image);
      break;
    }
  }
  /*
    Free allocated memory.
  */
  free((char *) range_table);
  free((char *) z);
  free((char *) y);
  free((char *) x);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   R o l l I m a g e                                                         %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Function RollImage rolls an image vertically and horizontally.  It
%  allocates the memory necessary for the new Image structure and returns a
%  pointer to the new image.
%
%  The format of the RollImage routine is:
%
%      rolled_image=RollImage(image,columns,rows)
%
%  A description of each parameter follows:
%
%    o rolled_image: Function RollImage returns a pointer to the image after
%      rolling.  A null image is returned if there is a memory shortage.
%
%    o image: The address of a structure of type Image.
%
%    o x_offset: An integer that specifies the number of columns to roll
%      in the horizontal direction.
%
%    o y_offset: An integer that specifies the number of rows to roll in the
%      vertical direction.
%
%
*/
Image *RollImage(Image *image, int x_offset, int y_offset)
{
#define RollImageText  "  Rolling image...  "

  Image
    *rolled_image;

  register RunlengthPacket
    *p,
    *q;

  register unsigned int
    packets,
    x;

  unsigned int
    y;

  /*
    Initialize rolled image attributes.
  */
  rolled_image=CopyImage(image,image->columns,image->rows,False);
  if (rolled_image == (Image *) NULL)
    {
      Warning("Unable to roll image","Memory allocation failed");
      return((Image *) NULL);
    }
  /*
    Roll image.
  */
  p=image->pixels;
  image->runlength=p->length+1;
  packets=image->columns*image->rows;
  for (y=0; y < image->rows; y++)
  {
    /*
      Transfer scanline.
    */
    for (x=0; x < image->columns; x++)
    {
      if (image->runlength != 0)
        image->runlength--;
      else
        {
          p++;
          image->runlength=p->length;
        }
      q=rolled_image->pixels+(y_offset+y)*image->columns+x+x_offset;
      if (q < rolled_image->pixels)
        q+=packets;
      else
        if (q >= (rolled_image->pixels+packets))
          q-=packets;
      *q=(*p);
      q->length=0;
    }
    ProgressMonitor(RollImageText,y,image->rows);
  }
  return(rolled_image);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   S a m p l e I m a g e                                                     %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Function SampleImage creates a new image that is a scaled size of an
%  existing one using pixel sampling.  It allocates the memory necessary
%  for the new Image structure and returns a pointer to the new image.
%
%  The format of the SampleImage routine is:
%
%      sampled_image=SampleImage(image,columns,rows)
%
%  A description of each parameter follows:
%
%    o sampled_image: Function SampleImage returns a pointer to the image after
%      scaling.  A null image is returned if there is a memory shortage.
%
%    o image: The address of a structure of type Image.
%
%    o columns: An integer that specifies the number of columns in the sampled
%      image.
%
%    o rows: An integer that specifies the number of rows in the sampled
%      image.
%
%
*/
Image *SampleImage(Image *image, unsigned int columns, unsigned int rows)
{
#define SampleImageText  "  Sampling image...  "

  Image
    *sampled_image;

  int
    y;

  register RunlengthPacket
    *p,
    *q,
    *s;

  register int
    x;

  RunlengthPacket
    *scanline;

  unsigned int
    *x_offset,
    *y_offset;

  unsigned long
    scale_factor;

  if ((columns == 0) || (rows == 0))
    {
      Warning("Unable to sample image","image dimensions are zero");
      return((Image *) NULL);
    }
  /*
    Initialize sampled image attributes.
  */
  sampled_image=CopyImage(image,columns,rows,False);
  if (sampled_image == (Image *) NULL)
    {
      Warning("Unable to sample image","Memory allocation failed");
      return((Image *) NULL);
    }
  /*
    Allocate scan line buffer and column offset buffers.
  */
  scanline=(RunlengthPacket *) malloc(image->columns*sizeof(RunlengthPacket));
  x_offset=(unsigned int *) malloc(sampled_image->columns*sizeof(unsigned int));
  y_offset=(unsigned int *) malloc(sampled_image->rows*sizeof(unsigned int));
  if ((scanline == (RunlengthPacket *) NULL) ||
      (x_offset == (unsigned int *) NULL) ||
      (y_offset == (unsigned int *) NULL))
    {
      Warning("Unable to sample image","Memory allocation failed");
      DestroyImage(sampled_image);
      return((Image *) NULL);
    }
  /*
    Initialize column pixel offsets.
  */
  scale_factor=UpShift(image->columns-1)/sampled_image->columns;
  columns=0;
  for (x=0; x < sampled_image->columns; x++)
  {
    x_offset[x]=DownShift((x+1)*scale_factor)-(int) columns;
    columns+=x_offset[x];
  }
  /*
    Initialize row pixel offsets.
  */
  scale_factor=UpShift(image->rows-1)/sampled_image->rows;
  rows=0;
  for (y=0; y < sampled_image->rows; y++)
  {
    y_offset[y]=DownShift((y+1)*scale_factor)-(int) rows;
    rows+=y_offset[y];
  }
  y_offset[sampled_image->rows-1]=0;
  /*
    Preload first scanline.
  */
  p=image->pixels;
  image->runlength=p->length+1;
  s=scanline;
  for (x=0; x < image->columns; x++)
  {
    if (image->runlength != 0)
      image->runlength--;
    else
      {
        p++;
        image->runlength=p->length;
      }
    *s=(*p);
    s->length=0;
    s++;
  }
  /*
    Sample each row.
  */
  q=sampled_image->pixels;
  for (y=0; y < sampled_image->rows; y++)
  {
    /*
      Sample each column.
    */
    s=scanline;
    for (x=0; x < sampled_image->columns; x++)
    {
      *q=(*s);
      q++;
      s+=x_offset[x];
    }
    if (y_offset[y] != 0)
      {
        /*
          Skip a scan line.
        */
        if (y_offset[y] > 1)
          for (x=0; x < (image->columns*(y_offset[y]-1)); x++)
            if (image->runlength != 0)
              image->runlength--;
            else
              {
                p++;
                image->runlength=p->length;
              }
        /*
          Read a scan line.
        */
        s=scanline;
        for (x=0; x < image->columns; x++)
        {
          if (image->runlength != 0)
            image->runlength--;
          else
            {
              p++;
              image->runlength=p->length;
            }
          *s=(*p);
          s->length=0;
          s++;
        }
      }
    ProgressMonitor(SampleImageText,y,sampled_image->rows);
  }
  free((char *) scanline);
  free((char *) x_offset);
  free((char *) y_offset);
  return(sampled_image);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   S c a l e I m a g e                                                       %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Function ScaleImage creates a new image that is a scaled size of an
%  existing one.  It allocates the memory necessary for the new Image
%  structure and returns a pointer to the new image.  To scale a scanline
%  from x pixels to y pixels, each new pixel represents x/y old pixels.  To
%  read x/y pixels, read (x/y rounded up) pixels but only count the required
%  fraction of the last old pixel read in your new pixel.  The remainder
%  of the old pixel will be counted in the next new pixel.
%
%  The scaling algorithm was suggested by rjohnson@shell.com and is adapted
%  from pnmscale(1) of PBMPLUS by Jef Poskanzer.
%
%  The format of the ScaleImage routine is:
%
%      scaled_image=ScaleImage(image,columns,rows)
%
%  A description of each parameter follows:
%
%    o scaled_image: Function ScaleImage returns a pointer to the image after
%      scaling.  A null image is returned if there is a memory shortage.
%
%    o image: The address of a structure of type Image.
%
%    o columns: An integer that specifies the number of columns in the scaled
%      image.
%
%    o rows: An integer that specifies the number of rows in the scaled
%      image.
%
%
*/
Image *ScaleImage(Image *image, unsigned int columns, unsigned int rows)
{
#define ScaleImageText  "  Scaling image...  "

  typedef struct ScaledPacket
  {
    long
      red,
      green,
      blue,
      index;
  } ScaledPacket;

  Image
    *scaled_image;

  int
    next_row,
    number_rows;

  long
    x_scale,
    x_span;

  register RunlengthPacket
    *p,
    *q;

  register ScaledPacket
    *s,
    *t;

  register unsigned int
    x;

  ScaledPacket
    *scaled_scanline,
    *scanline,
    *y_vector,
    *x_vector;

  unsigned int
    packets,
    y;

  unsigned long
    blue,
    green,
    index,
    red,
    scale_factor;

  if ((columns == 0) || (rows == 0))
    {
      Warning("Unable to scale image","image dimensions are zero");
      return((Image *) NULL);
    }
  /*
    Initialize scaled image attributes.
  */
  scale_factor=UpShift(columns*rows)/(image->columns*image->rows);
  packets=Max(DownShift(image->packets*scale_factor),1);
  scaled_image=CopyImage(image,packets,1,False);
  if (scaled_image == (Image *) NULL)
    {
      Warning("Unable to scale image","Memory allocation failed");
      return((Image *) NULL);
    }
  scaled_image->class=DirectClass;
  scaled_image->columns=columns;
  scaled_image->rows=rows;
  scaled_image->packets=0;
  /*
    Allocate memory.
  */
  x_vector=(ScaledPacket *) malloc(image->columns*sizeof(ScaledPacket));
  scanline=x_vector;
  if (scaled_image->rows != image->rows)
    scanline=(ScaledPacket *) malloc(image->columns*sizeof(ScaledPacket));
  scaled_scanline=(ScaledPacket *)
    malloc(scaled_image->columns*sizeof(ScaledPacket));
  y_vector=(ScaledPacket *) malloc(image->columns*sizeof(ScaledPacket));
  if ((x_vector == (ScaledPacket *) NULL) ||
      (scanline == (ScaledPacket *) NULL) ||
      (scaled_scanline == (ScaledPacket *) NULL) ||
      (y_vector == (ScaledPacket *) NULL))
    {
      Warning("Unable to scale image","Memory allocation failed");
      DestroyImage(scaled_image);
      return((Image *) NULL);
    }
  /*
    Scale image.
  */
  number_rows=0;
  next_row=True;
  x_scale=UpShift(scaled_image->rows)/image->rows;
  x_span=UpShift(1);
  for (x=0; x < image->columns; x++)
  {
    y_vector[x].red=0;
    y_vector[x].green=0;
    y_vector[x].blue=0;
    y_vector[x].index=0;
  }
  p=image->pixels;
  image->runlength=p->length+1;
  q=scaled_image->pixels;
  q->red=0;
  q->green=0;
  q->blue=0;
  q->index=0;
  q->length=MaxRunlength;
  for (y=0; y < scaled_image->rows; y++)
  {
    if (scaled_image->rows == image->rows)
      for (x=0; x < image->columns; x++)
      {
        /*
          Read a new scanline.
        */
        if (image->runlength != 0)
          image->runlength--;
        else
          {
            p++;
            image->runlength=p->length;
          }
        x_vector[x].red=p->red;
        x_vector[x].green=p->green;
        x_vector[x].blue=p->blue;
        x_vector[x].index=p->index;
      }
    else
      {
        /*
          Scale Y direction.
        */
        while (x_scale < x_span)
        {
          if (next_row && (number_rows < image->rows))
            {
              /*
                Read a new scanline.
              */
              for (x=0; x < image->columns; x++)
              {
                if (image->runlength != 0)
                  image->runlength--;
                else
                  {
                    p++;
                    image->runlength=p->length;
                  }
                x_vector[x].red=p->red;
                x_vector[x].green=p->green;
                x_vector[x].blue=p->blue;
                x_vector[x].index=p->index;
              }
              number_rows++;
            }
          for (x=0; x < image->columns; x++)
          {
            y_vector[x].red+=x_scale*x_vector[x].red;
            y_vector[x].green+=x_scale*x_vector[x].green;
            y_vector[x].blue+=x_scale*x_vector[x].blue;
            y_vector[x].index+=x_scale*x_vector[x].index;
          }
          x_span-=x_scale;
          x_scale=UpShift(scaled_image->rows)/image->rows;
          next_row=True;
        }
        if (next_row && (number_rows < image->rows))
          {
            /*
              Read a new scanline.
            */
            for (x=0; x < image->columns; x++)
            {
              if (image->runlength != 0)
                image->runlength--;
              else
                {
                  p++;
                  image->runlength=p->length;
                }
              x_vector[x].red=p->red;
              x_vector[x].green=p->green;
              x_vector[x].blue=p->blue;
              x_vector[x].index=p->index;
            }
            number_rows++;
            next_row=False;
          }
        s=scanline;
        for (x=0; x < image->columns; x++)
        {
          red=DownShift(y_vector[x].red+x_span*x_vector[x].red);
          green=DownShift(y_vector[x].green+x_span*x_vector[x].green);
          blue=DownShift(y_vector[x].blue+x_span*x_vector[x].blue);
          index=DownShift(y_vector[x].index+x_span*x_vector[x].index);
          s->red=red > MaxRGB ? MaxRGB : red;
          s->green=green > MaxRGB ? MaxRGB : green;
          s->blue=blue > MaxRGB ? MaxRGB : blue;
          s->index=index > MaxColormapSize ? MaxColormapSize : index;
          s++;
          y_vector[x].red=0;
          y_vector[x].green=0;
          y_vector[x].blue=0;
          y_vector[x].index=0;
        }
        x_scale-=x_span;
        if (x_scale == 0)
          {
            x_scale=UpShift(scaled_image->rows)/image->rows;
            next_row=True;
          }
        x_span=UpShift(1);
      }
    if (scaled_image->columns == image->columns)
      {
        /*
          Transfer scanline to scaled image.
        */
        s=scanline;
        for (x=0; x < scaled_image->columns; x++)
        {
          if ((s->red == q->red) && (s->green == q->green) &&
              (s->blue == q->blue) && (s->index == q->index) &&
              ((int) q->length < MaxRunlength))
            q->length++;
          else
            {
              if (scaled_image->packets != 0)
                q++;
              scaled_image->packets++;
              if (scaled_image->packets == packets)
                {
                  packets<<=1;
                  scaled_image->pixels=(RunlengthPacket *) realloc((char *)
                    scaled_image->pixels,packets*sizeof(RunlengthPacket));
                  if (scaled_image->pixels == (RunlengthPacket *) NULL)
                    {
                      Warning("Unable to scale image",
                        "Memory allocation failed");
                      DestroyImage(scaled_image);
                      return((Image *) NULL);
                    }
                  q=scaled_image->pixels+scaled_image->packets-1;
                }
              q->red=s->red;
              q->green=s->green;
              q->blue=s->blue;
              q->index=s->index;
              q->length=0;
            }
          s++;
        }
      }
    else
      {
        int
          next_column;

        long int
          y_scale,
          y_span;

        /*
          Scale X direction.
        */
        red=0;
        green=0;
        blue=0;
        next_column=False;
        y_span=UpShift(1);
        s=scanline;
        t=scaled_scanline;
        for (x=0; x < image->columns; x++)
        {
          y_scale=UpShift(scaled_image->columns)/image->columns;
          while (y_scale >= y_span)
          {
            if (next_column)
              {
                red=0;
                green=0;
                blue=0;
                index=0;
                t++;
              }
            red=DownShift(red+y_span*s->red);
            green=DownShift(green+y_span*s->green);
            blue=DownShift(blue+y_span*s->blue);
            index=DownShift(index+y_span*s->index);
            t->red=red > MaxRGB ? MaxRGB : red;
            t->green=green > MaxRGB ? MaxRGB : green;
            t->blue=blue > MaxRGB ? MaxRGB : blue;
            t->index=index > MaxColormapSize ? MaxColormapSize : index;
            y_scale-=y_span;
            y_span=UpShift(1);
            next_column=True;
          }
        if (y_scale > 0)
          {
            if (next_column)
              {
                red=0;
                green=0;
                blue=0;
                index=0;
                next_column=False;
                t++;
              }
            red+=y_scale*s->red;
            green+=y_scale*s->green;
            blue+=y_scale*s->blue;
            index+=y_scale*s->index;
            y_span-=y_scale;
          }
        s++;
      }
      if (y_span > 0)
        {
          s--;
          red+=y_span*s->red;
          green+=y_span*s->green;
          blue+=y_span*s->blue;
          index+=y_span*s->index;
        }
      if (!next_column)
        {
          red=DownShift(red);
          green=DownShift(green);
          blue=DownShift(blue);
          index=DownShift(index);
          t->red=red > MaxRGB ? MaxRGB : red;
          t->green=green > MaxRGB ? MaxRGB : green;
          t->blue=blue > MaxRGB ? MaxRGB : blue;
          t->index=index > MaxRGB ? MaxRGB : index;
        }
      /*
        Transfer scanline to scaled image.
      */
      t=scaled_scanline;
      for (x=0; x < scaled_image->columns; x++)
      {
        if ((t->red == q->red) && (t->green == q->green) &&
            (t->blue == q->blue) && (t->index == q->index) &&
            ((int) q->length < MaxRunlength))
          q->length++;
        else
          {
            if (scaled_image->packets != 0)
              q++;
            scaled_image->packets++;
            if (scaled_image->packets == packets)
              {
                packets<<=1;
                scaled_image->pixels=(RunlengthPacket *) realloc((char *)
                  scaled_image->pixels,packets*sizeof(RunlengthPacket));
                if (scaled_image->pixels == (RunlengthPacket *) NULL)
                  {
                    Warning("Unable to scale image","Memory allocation failed");
                    DestroyImage(scaled_image);
                    return((Image *) NULL);
                  }
                q=scaled_image->pixels+scaled_image->packets-1;
              }
            q->red=t->red;
            q->green=t->green;
            q->blue=t->blue;
            q->index=t->index;
            q->length=0;
          }
        t++;
      }
    }
    ProgressMonitor(ScaleImageText,y,scaled_image->rows);
  }
  scaled_image->pixels=(RunlengthPacket *) realloc((char *)
    scaled_image->pixels,scaled_image->packets*sizeof(RunlengthPacket));
  /*
    Free allocated memory.
  */
  free((char *) y_vector);
  free((char *) scaled_scanline);
  if (scanline != x_vector)
    free((char *) scanline);
  free((char *) x_vector);
  return(scaled_image);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   S e t I m a g e M a g i c k                                               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Function SetImageMagick initializes the `magick' field of the ImageInfo
%  structure.  It is set to a type of image format based on the prefix or
%  suffix of the filename.  For example, `ps:image' returns PS indicating
%  a Postscript image.  JPEG is returned for this filename: `image.jpg'.
%  The filename prefix has precedance over the suffix.  Use an optional index
%  enclosed in brackets after a file name to specify a desired subimage of a
%  multi-resolution image format like Photo CD (e.g. img0001.pcd[4]).
%
%  The format of the SetImageMagick routine is:
%
%      SetImageMagick(image_info)
%
%  A description of each parameter follows:
%
%    o image_info: Specifies a pointer to a ImageInfo structure.
%
%
*/
void SetImageMagick(ImageInfo *image_info)
{
  static char
    *ImageFormats[]=
    {
      "AVS",
      "BIE",
      "BMP",
      "CMYK",
      "EPS",
      "EPSF",
      "EPSI",
      "FAX",
      "FITS",
      "FTP",
      "GIF",
      "GIF87",
      "GOPHER",
      "GRAY",
      "G3",
      "HDF",
      "HIPS",          /* Kobus */
      "HISTOGRAM",
      "HTML",
      "HTTP",
      "JBIG",
      "JPEG",
      "JPG",
      "LOGO",
      "MAP",
      "MATTE",
      "MIFF",
      "MPG",
      "MPEG",
      "MTV",
      "NULL",
      "PBM",
      "PCD",
      "PCX",
      "PDF",
      "PGM",
      "PICT",
      "PM",
      "PNG",
      "PPM",
      "PNM",
      "PS",
      "PS2",
      "RAD",
      "RAS",
      "RGB",
      "RGBA",
      "RLE",
      "SGI",
      "SUN",
      "TGA",
      "TEXT",
      "TIF",
      "TIFF",
      "TILE",
      "TMP",
      "UYVY",
      "VICAR",
      "VID",
      "VIFF",
      "X",
      "XBM",
      "XC",
      "XPM",
      "XV",
      "XWD",
      "YUV",
      "YUV3",
      (char *) NULL,
    };

  char
    c,
    magick[MaxTextLength];

  register char
    *p,
    *q;

  register int
    i;

  /*
    Look for 'image.format' in filename.
  */
  *magick='\0';
  p=image_info->filename+strlen(image_info->filename)-1;
  if (*p == ']')
    {
      /*
        Look for sub-image enclosed in brackets (e.g. img0001.pcd[4]).
      */
      for (q=p-1; q > image_info->filename; q--)
        if (!isdigit(*q) && (*q != '-'))
          break;
      if (*q == '[')
        {
          p=q++;
          image_info->subimage=atoi(q);
          image_info->subrange=atoi(q);
          (void) sscanf(q,"%u-%u",&image_info->subimage,&image_info->subrange);
          image_info->subrange-=image_info->subimage-1;
          *p='\0';
        }
    }
  while ((*p != '.') && (p > image_info->filename))
    p--;
  if ((strcmp(p,".gz") == 0) || (strcmp(p,".Z") == 0))
    do
    {
      p--;
    } while ((*p != '.') && (p > image_info->filename));
  if ((*p == '.') && (strlen(p) < sizeof(magick)))
    {
      /*
        User specified image format.
      */
      (void) strcpy(magick,p+1);
      for (q=magick; *q != '\0'; q++)
      {
        if (*q == '.')
          {
            *q='\0';
            break;
          }
        c=(*q);
        if (isascii(c) && islower(c))
          *q=toupper(c);
      }
      for (i=0; ImageFormats[i] != (char *) NULL; i++)
        if (strcmp(magick,ImageFormats[i]) == 0)
          {
            /*
              SGI and RGB are ambiguous.
            */
            if ((strncmp(image_info->magick,"SGI",3) != 0) ||
                (strcmp(ImageFormats[i],"RGB") != 0))
              (void) strcpy(image_info->magick,magick);
            break;
          }
    }
  /*
    Look for explicit 'format:image' in filename.
  */
  image_info->assert=False;
  p=image_info->filename;
  while ((*p != ':') && (*p != '\0'))
    p++;
  if ((*p == ':') && ((p-image_info->filename) < sizeof(magick)))
    {
      /*
        User specified image format.
      */
      (void) strncpy(magick,image_info->filename,p-image_info->filename);
      magick[p-image_info->filename]='\0';
      for (q=magick; *q != '\0'; q++)
      {
        c=(*q);
        if (isascii(c) && islower(c))
          *q=toupper(c);
      }
      for (i=0; ImageFormats[i] != (char *) NULL; i++)
        if (strcmp(magick,ImageFormats[i]) == 0)
          {
            /*
              Strip off image format prefix.
            */
            p++;
            (void) strcpy(image_info->filename,p);
            (void) strcpy(image_info->magick,magick);
            if (strcmp(magick,"TMP") != 0)
              image_info->assert=True;
            break;
          }
    }
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%     S h a r p e n I m a g e                                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Function SharpenImage creates a new image that is a copy of an existing
%  one with the pixels sharpened.  It allocates the memory necessary for the
%  new Image structure and returns a pointer to the new image.
%
%  SharpenImage convolves the pixel neighborhood with this sharpening mask:
%
%    -1 -2 -1
%    -2  W -2
%    -1 -2 -1
%
%  The scan only processes pixels that have a full set of neighbors.  Pixels
%  in the top, bottom, left, and right pairs of rows and columns are omitted
%  from the scan.
%
%  The format of the SharpenImage routine is:
%
%      sharpened_image=SharpenImage(image,factor)
%
%  A description of each parameter follows:
%
%    o sharpened_image: Function SharpenImage returns a pointer to the image
%      after it is sharpened.  A null image is returned if there is a memory
%      shortage.
%
%    o image: The address of a structure of type Image;  returned from
%      ReadImage.
%
%    o factor:  An double value reflecting the percent weight to give to the
%      center pixel of the neighborhood.
%
%
*/
Image *SharpenImage(Image *image, double factor)
{
#define Sharpen(weight) \
  total_red+=(weight)*(int) (s->red); \
  total_green+=(weight)*(int) (s->green); \
  total_blue+=(weight)*(int) (s->blue); \
  total_index+=(weight)*(int) (s->index); \
  s++;
#define SharpenImageText  "  Sharpening image...  "

  Image
    *sharpened_image;

  long int
    total_blue,
    total_green,
    total_index,
    total_red,
    weight;

  register RunlengthPacket
    *p,
    *q,
    *s,
    *s0,
    *s1,
    *s2;

  register unsigned int
    x;

  RunlengthPacket
    *scanline;

  unsigned int
    quantum,
    y;

  if ((image->columns < 3) || (image->rows < 3))
    {
      Warning("Unable to sharpen image","image size must exceed 3x3");
      return((Image *) NULL);
    }
  /*
    Initialize sharpened image attributes.
  */
  sharpened_image=CopyImage(image,image->columns,image->rows,False);
  if (sharpened_image == (Image *) NULL)
    {
      Warning("Unable to enhance image","Memory allocation failed");
      return((Image *) NULL);
    }
  sharpened_image->class=DirectClass;
  /*
    Allocate scan line buffer for 3 rows of the image.
  */
  scanline=(RunlengthPacket *)
    malloc(3*(image->columns+1)*sizeof(RunlengthPacket));
  if (scanline == (RunlengthPacket *) NULL)
    {
      Warning("Unable to enhance image","Memory allocation failed");
      DestroyImage(sharpened_image);
      return((Image *) NULL);
    }
  /*
    Read the first two rows of the image.
  */
  p=image->pixels;
  image->runlength=p->length+1;
  for (x=0; x < (3*(image->columns+1)); x++)
    scanline[x]=(*p);
  s=scanline;
  for (x=0; x < (image->columns << 1); x++)
  {
    if (image->runlength != 0)
      image->runlength--;
    else
      {
        p++;
        image->runlength=p->length;
      }
    *s=(*p);
    s++;
  }
  /*
    Dump first scanline of image.
  */
  q=sharpened_image->pixels;
  s=scanline;
  for (x=0; x < image->columns; x++)
  {
    *q=(*s);
    q->index=0;
    q->length=0;
    q++;
    s++;
  }
  /*
    Convolve each row.
  */
  weight=(long int) ((100.0-factor)/2+13);
  quantum=Max(weight-12,1);
  for (y=1; y < (image->rows-1); y++)
  {
    /*
      Initialize sliding window pointers.
    */
    s0=scanline+image->columns*((y-1) % 3);
    s1=scanline+image->columns*(y % 3);
    s2=scanline+image->columns*((y+1) % 3);
    /*
      Read another scan line.
    */
    s=s2;
    for (x=0; x < image->columns; x++)
    {
      if (image->runlength != 0)
        image->runlength--;
      else
        {
          p++;
          image->runlength=p->length;
        }
      *s=(*p);
      s++;
    }
    /*
      Transfer first pixel of the scanline.
    */
    *q=(*s1);
    q->length=0;
    q++;
    for (x=1; x < (image->columns-1); x++)
    {
      /*
        Compute weighted average of target pixel color components.
      */
      total_red=0;
      total_green=0;
      total_blue=0;
      total_index=0;
      s=s0;
      Sharpen(-1); Sharpen(-2); Sharpen(-1);
      s=s1;
      Sharpen(-2); Sharpen(weight); Sharpen(-2);
      s=s2;
      Sharpen(-1); Sharpen(-2); Sharpen(-1);
      if (total_red < 0)
        q->red=0;
      else
        if (total_red > (MaxRGB*quantum))
          q->red=MaxRGB;
        else
          q->red=(Quantum) ((total_red+(quantum >> 1))/quantum);
      if (total_green < 0)
        q->green=0;
      else
        if (total_green > (MaxRGB*quantum))
          q->green=MaxRGB;
        else
          q->green=(Quantum) ((total_green+(quantum >> 1))/quantum);
      if (total_blue < 0)
        q->blue=0;
      else
        if (total_blue > (MaxRGB*quantum))
          q->blue=MaxRGB;
        else
          q->blue=(Quantum) ((total_blue+(quantum >> 1))/quantum);
      if (total_index < 0)
        q->index=0;
      else
        if (total_index > (MaxRGB*quantum))
          q->index=MaxRGB;
        else
          q->index=(unsigned short) ((total_index+(quantum >> 1))/quantum);
      q->length=0;
      q++;
      s0++;
      s1++;
      s2++;
    }
    /*
      Transfer last pixel of the scanline.
    */
    s1++;
    *q=(*s1);
    q->length=0;
    q++;
    ProgressMonitor(SharpenImageText,y,image->rows-1);
  }
  /*
    Dump last scanline of pixels.
  */
  s=scanline+image->columns*(y % 3);
  for (x=0; x < image->columns; x++)
  {
    *q=(*s);
    q->length=0;
    q++;
    s++;
  }
  free((char *) scanline);
  return(sharpened_image);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   S o r t C o l o r m a p B y I n t e n t s i t y                           %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Function SortColormapByIntensity sorts the colormap of a PseudoClass image
%  by decreasing color intensity.
%
%  The format of the SortColormapByIntensity routine is:
%
%      SortColormapByIntensity(image)
%
%  A description of each parameter follows:
%
%    o image: A pointer to a Image structure.
%
%
*/
static int IntensityCompare(const void *x, const void *y)
{
  ColorPacket
    *color_1,
    *color_2;

  color_1=(ColorPacket *) x;
  color_2=(ColorPacket *) y;
  return((int) Intensity(*color_2)-(int) Intensity(*color_1));
}

void SortColormapByIntensity(Image *image)
{
  register int
    i;

  register RunlengthPacket
    *p;

  register unsigned short
    index;

  unsigned short
    *pixels;

  if (image->class != PseudoClass)
    return;
  /*
    Allocate memory for pixel indexes.
  */
  pixels=(unsigned short *) malloc(image->colors*sizeof(unsigned short));
  if (pixels == (unsigned short *) NULL)
    {
      Warning("Unable to sort colormap","Memory allocation failed");
      return;
    }
  /*
    Assign index values to colormap entries.
  */
  for (i=0; i < image->colors; i++)
    image->colormap[i].index=(unsigned short) i;
  /*
    Sort image colormap by decreasing color popularity.
  */
  qsort((void *) image->colormap,(int) image->colors,sizeof(ColorPacket),
    (int (*) _Declare((const void *, const void *))) IntensityCompare);
  /*
    Update image colormap indexes to sorted colormap order.
  */
  for (i=0; i < image->colors; i++)
    pixels[image->colormap[i].index]=(unsigned short) i;
  p=image->pixels;
  for (i=0; i < image->packets; i++)
  {
    index=pixels[p->index];
    p->red=image->colormap[index].red;
    p->green=image->colormap[index].green;
    p->blue=image->colormap[index].blue;
    p->index=index;
    p++;
  }
  free((char *) pixels);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%     S p r e a d I m a g e                                                   %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Function SpreadImage creates a new image that is a copy of an existing
%  one with the image pixels randomly displaced.  It allocates the memory
%  necessary for the new Image structure and returns a pointer to the new
%  image.
%
%  The format of the SpreadImage routine is:
%
%      spread_image=SpreadImage(image,amount)
%
%  A description of each parameter follows:
%
%    o spread_image: Function SpreadImage returns a pointer to the image
%      after it is spread.  A null image is returned if there is a memory
%      shortage.
%
%    o image: The address of a structure of type Image;  returned from
%      ReadImage.
%
%    o amount:  An unsigned value constraining the "vicintity" for choosing
%      a random pixel to swap.
%
%
*/
Image *SpreadImage(Image *image, unsigned int amount)
{
#define SpreadImageText  "  Spreading image...  "

  Image
    *spread_image;

  long
    quantum,
    x_distance,
    y_distance;

  register RunlengthPacket
    *p,
    *q;

  register unsigned int
    x;

  unsigned int
    y;

  if ((image->columns < 3) || (image->rows < 3))
    {
      Warning("Unable to spread image","image size must exceed 3x3");
      return((Image *) NULL);
    }
  if (!UncompressImage(image))
    return((Image *) NULL);
  /*
    Initialize spread image attributes.
  */
  spread_image=CopyImage(image,image->columns,image->rows,False);
  if (spread_image == (Image *) NULL)
    {
      Warning("Unable to enhance image","Memory allocation failed");
      return((Image *) NULL);
    }
  spread_image->class=DirectClass;
  /*
    Convolve each row.
  */
  srand(time((time_t *) NULL));
  amount++;
  quantum=amount >> 1;
  q=spread_image->pixels;
  for (y=0; y < image->rows; y++)
  {
    for (x=0; x < image->columns; x++)
    {
      x_distance=(rand() & amount)-quantum;
      y_distance=(rand() & amount)-quantum;
      p=image->pixels+(y+y_distance)*image->columns+(x+x_distance);
      if ((p > image->pixels) && (p < (image->pixels+image->packets)))
        *q=(*p);
      q++;
    }
    ProgressMonitor(SpreadImageText,y,image->rows);
  }
  return(spread_image);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   S t e r e o I m a g e                                                     %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Function StereoImage combines two images and produces a single image that
%  is the composite of a left and right image of a stereo pair.  The left
%  image is converted to grayscale and written to the red channel of the
%  stereo image.  The right image is converted to grayscale and written to the
%  blue channel of the stereo image.  View the composite image with red-blue
%  glasses to create a stereo effect.
%
%  The format of the StereoImage routine is:
%
%      stereo_image=StereoImage(left_image,right_image)
%
%  A description of each parameter follows:
%
%    o stereo_image: Function StereoImage returns a pointer to the stereo
%      image.  A null image is returned if there is a memory shortage.
%
%    o left_image: The address of a structure of type Image.
%
%    o right_image: The address of a structure of type Image.
%
%
*/
Image *StereoImage(Image *left_image, Image *right_image)
{
#define StereoImageText  "  Stereo image...  "

  Image
    *stereo_image;

  int
    y;

  register int
    x;

  register RunlengthPacket
    *p,
    *q,
    *r;

  if ((left_image->columns != right_image->columns) ||
      (left_image->rows != right_image->rows))
    {
      Warning("Unable to create stereo image",
        "left and right image sizes differ");
      return((Image *) NULL);
    }
  /*
    Initialize stereo image attributes.
  */
  stereo_image=CopyImage(left_image,left_image->columns,left_image->rows,False);
  if (stereo_image == (Image *) NULL)
    {
      Warning("Unable to create stereo image","Memory allocation failed");
      return((Image *) NULL);
    }
  stereo_image->class=DirectClass;
  /*
    Copy left image to red channel and right image to blue channel.
  */
  QuantizeImage(left_image,256,8,False,GRAYColorspace);
  SyncImage(left_image);
  p=left_image->pixels;
  left_image->runlength=p->length+1;
  QuantizeImage(right_image,256,8,False,GRAYColorspace);
  SyncImage(right_image);
  q=right_image->pixels;
  right_image->runlength=q->length+1;
  r=stereo_image->pixels;
  for (y=0; y < stereo_image->rows; y++)
  {
    for (x=0; x < stereo_image->columns; x++)
    {
      if (left_image->runlength != 0)
        left_image->runlength--;
      else
        {
          p++;
          left_image->runlength=p->length;
        }
      if (right_image->runlength != 0)
        right_image->runlength--;
      else
        {
          q++;
          right_image->runlength=q->length;
        }
      r->red=(unsigned int) (p->red*12) >> 4;
      r->green=0;
      r->blue=q->blue;
      r->index=0;
      r->length=0;
      r++;
    }
    ProgressMonitor(StereoImageText,y,stereo_image->rows);
  }
  return(stereo_image);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   S y n c I m a g e                                                         %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Function SyncImage initializes the red, green, and blue intensities of each
%  pixel as defined by the colormap index.
%
%  The format of the SyncImage routine is:
%
%      SyncImage(image)
%
%  A description of each parameter follows:
%
%    o image: The address of a structure of type Image.
%
%
*/
void SyncImage(Image *image)
{
  register int
    i;

  register RunlengthPacket
    *p;

  register unsigned short
    index;

  for (i=0; i < image->colors; i++)
  {
    image->colormap[i].index=0;
    image->colormap[i].flags=0;
  }
  p=image->pixels;
  for (i=0; i < image->packets; i++)
  {
    index=p->index;
    p->red=image->colormap[index].red;
    p->green=image->colormap[index].green;
    p->blue=image->colormap[index].blue;
    p++;
  }
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%     T e x t u r e I m a g e                                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Function TextureImage layers a texture onto the background of an image.
%
%  The format of the TextureImage routine is:
%
%      TextureImage(image,filename)
%
%  A description of each parameter follows:
%
%    o image: The address of a structure of type Image;  returned from
%      ReadImage.
%
%    o filename: This file contains the texture to layer on the background.
%
%
*/
void TextureImage(Image *image, char *filename)
{
#define TextureImageText  "  Appling image texture...  "

  Image
    *texture_image;

  ImageInfo
    texture_info;

  int
    x,
    y;

  if (filename == (char *) NULL)
    return;
  /*
    Read the texture image.
  */
  GetImageInfo(&texture_info);
  (void) strcpy(texture_info.filename,filename);
  texture_image=ReadImage(&texture_info);
  if (texture_image == (Image *) NULL)
    return;
  /*
    Tile texture onto the image background.
  */
  for (y=0; y < image->rows; y+=texture_image->rows)
  {
    for (x=0; x < image->columns; x+=texture_image->columns)
      CompositeImage(image,ReplaceCompositeOp,texture_image,x,y);
    ProgressMonitor(TextureImageText,y,image->rows);
  }
  DestroyImage(texture_image);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   T r a n s f o r m I m a g e                                               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Function TransformImage creates a new image that is a transformed size of
%  of existing one as specified by the crop and image geometries.  It
%  allocates the memory necessary for the new Image structure and returns a
%  pointer to the new image.
%
%  If a crop geometry is specified a subregion of the image is obtained.
%  If the specified image size, as defined by the image and scale geometries,
%  is smaller than the actual image size, the image is first minified to an
%  integral of the specified image size with an antialias digital filter.  The
%  image is then scaled to the exact specified image size with pixel
%  replication.  If the specified image size is greater than the actual image
%  size, the image is first enlarged to an integral of the specified image
%  size with bilinear interpolation.  The image is then scaled to the exact
%  specified image size with pixel replication.
%
%  The format of the TransformImage routine is:
%
%      TransformImage(image,crop_geometry,image_geometry)
%
%  A description of each parameter follows:
%
%    o image: The address of an address of a structure of type Image.  The
%      transformed image is returned as this parameter.
%
%    o crop_geometry: Specifies a pointer to a crop geometry string.
%      This geometry defines a subregion of the image.
%
%    o image_geometry: Specifies a pointer to a image geometry string.
%      The specified width and height of this geometry string are absolute.
%
%
*/
void TransformImage(Image **image, char *crop_geometry, char *image_geometry)
{
  Image
    *transformed_image;

  int
    flags;

  unsigned int
    height,
    sharpen,
    width;

  transformed_image=(*image);
  if (crop_geometry != (char *) NULL)
    {
      Image
        *cropped_image;

      RectangleInfo
        crop_info;

      /*
        Crop image to a user specified size.
      */
      crop_info.x=0;
      crop_info.y=0;
      flags=
        XParseGeometry(crop_geometry,&crop_info.x,&crop_info.y,&width,&height);
      if ((flags & WidthValue) == 0)
        width=(unsigned int) ((int) transformed_image->columns-crop_info.x);
      if ((flags & HeightValue) == 0)
        height=(unsigned int) ((int) transformed_image->rows-crop_info.y);
      if ((flags & XNegative) != 0)
        crop_info.x+=transformed_image->columns-width;
      if ((flags & YNegative) != 0)
        crop_info.y+=transformed_image->rows-height;
      if (strchr(crop_geometry,'%') != (char *) NULL)
        {
          /*
            Crop geometry is relative to image size.
          */
          ParseImageGeometry(crop_geometry,&width,&height);
          if (width > transformed_image->columns)
            width=transformed_image->columns;
          if (height > transformed_image->rows)
            height=transformed_image->rows;
          crop_info.x=width >> 1;
          crop_info.y=height >> 1;
          width=transformed_image->columns-width;
          height=transformed_image->rows-height;
        }
      crop_info.width=width;
      crop_info.height=height;
      if ((width == 0) || (height == 0) ||
          ((flags & XValue) != 0) || ((flags & YValue) != 0))
        cropped_image=CropImage(transformed_image,&crop_info);
      else
        {
          Image
            *next_image;

          register int
            x,
            y;

          /*
            Crop repeatedly to create uniform subimages.
          */
          cropped_image=(Image *) NULL;
          for (y=0; y < transformed_image->rows; y+=height)
          {
            for (x=0; x < transformed_image->columns; x+=width)
            {
              crop_info.width=width;
              crop_info.height=height;
              crop_info.x=x;
              crop_info.y=y;
              next_image=CropImage(transformed_image,&crop_info);
              if (next_image == (Image *) NULL)
                break;
              if (cropped_image == (Image *) NULL)
                {
                  cropped_image=next_image;
                  continue;
                }
              cropped_image->next=next_image;
              next_image->previous=cropped_image;
              cropped_image=cropped_image->next;
            }
            if (next_image == (Image *) NULL)
              break;
          }
        }
      if (cropped_image != (Image *) NULL)
        {
          DestroyImage(transformed_image);
          while (cropped_image->previous != (Image *) NULL)
            cropped_image=cropped_image->previous;
          transformed_image=cropped_image;
        }
    }
  /*
    Scale image to a user specified size.
  */
  width=transformed_image->columns;
  height=transformed_image->rows;
  ParseImageGeometry(image_geometry,&width,&height);
  sharpen=(width*height) < (transformed_image->rows*transformed_image->columns);
  if ((transformed_image->columns != width) ||
      (transformed_image->rows != height))
    {
      Image
        *zoomed_image;

      /*
        Zoom image.
      */
      zoomed_image=ZoomImage(transformed_image,width,height,MitchellFilter);
      if (zoomed_image == (Image *) NULL)
        zoomed_image=ScaleImage(transformed_image,width,height);
      if (zoomed_image != (Image *) NULL)
        {
          DestroyImage(transformed_image);
          transformed_image=zoomed_image;
        }
    }
  if (sharpen)
    if ((transformed_image->columns >= 3) && (transformed_image->rows >= 3))
      {
        Image
          *sharpened_image;

        /*
          Sharpen image.
        */
        sharpened_image=SharpenImage(transformed_image,SharpenFactor);
        if (sharpened_image != (Image *) NULL)
          {
            DestroyImage(transformed_image);
            transformed_image=sharpened_image;
          }
      }
  *image=transformed_image;
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%     T r a n s f o r m R G B I m a g e                                       %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Function TransformRGBImage converts the reference image from an alternate
%  colorspace.  The transformation matrices are not the standard ones:  the
%  weights are rescaled to normalized the range of the transformed values to
%  be [0..MaxRGB].
%
%  The format of the TransformRGBImage routine is:
%
%      TransformRGBImage(image,colorspace)
%
%  A description of each parameter follows:
%
%    o image: The address of a structure of type Image;  returned from
%      ReadImage.
%
%    o colorspace: An unsigned integer value that indicates the colorspace
%      the image is currently in.  On return the image is in the RGB
%      color space.
%
%
*/
void TransformRGBImage(Image *image, unsigned int colorspace)
{
#define B (MaxRGB+1)*2
#define G (MaxRGB+1)
#define R 0
#define TransformRGBImageText  "  Transforming image colors...  "

/*
// Kobus, July 13, 2002.
//
// It seems that the handling of PCD changed from this obsolete version, and
// lead to inconsistency between kjb_display and kjb_image, as the later either
// uses the updated version of "convert", or optionally does its own
// conversion. Therefore, we will try to replace the old PCD reader with the
// new one taken from IM version 5.7.
*/
#ifdef HOW_IT_WAS
  static Quantum
    PCDMap[348] =  /* Photo CD information beyond 100% white, Gamma 2.2 */
    {
        0,   1,   2,   3,   4,   5,   6,   7,   8,   9,  11,  12,  13,  14,
       15,  16,  17,  18,  19,  20,  21,  22,  23,  24,  25,  26,  27,  28,
       29,  30,  31,  32,  33,  34,  35,  36,  37,  38,  39,  40,  41,  42,
       43,  44,  45,  46,  47,  48,  49,  50,  51,  52,  53,  54,  55,  55,
       56,  57,  58,  59,  60,  61,  62,  63,  64,  65,  66,  66,  67,  68,
       69,  70,  71,  72,  73,  74,  75,  76,  76,  77,  78,  79,  80,  81,
       82,  83,  84,  84,  85,  86,  87,  88,  89,  90,  91,  92,  92,  93,
       94,  95,  96,  97,  98,  99,  99, 100, 101, 102, 103, 104, 105, 106,
      106, 107, 108, 109, 110, 111, 112, 113, 114, 114, 115, 116, 117, 118,
      119, 120, 121, 122, 122, 123, 124, 125, 126, 127, 128, 129, 129, 130,
      131, 132, 133, 134, 135, 136, 136, 137, 138, 139, 140, 141, 142, 142,
      143, 144, 145, 146, 147, 148, 148, 149, 150, 151, 152, 153, 153, 154,
      155, 156, 157, 158, 158, 159, 160, 161, 162, 163, 164, 165, 165, 166,
      167, 168, 169, 170, 171, 172, 173, 173, 174, 175, 176, 177, 178, 178,
      179, 180, 181, 182, 182, 183, 184, 185, 186, 186, 187, 188, 189, 190,
      191, 192, 193, 194, 194, 195, 196, 197, 198, 199, 200, 201, 202, 203,
      204, 205, 205, 206, 207, 208, 209, 210, 210, 211, 212, 213, 214, 215,
      216, 216, 217, 218, 219, 220, 221, 221, 222, 223, 224, 225, 225, 226,
      227, 228, 228, 229, 230, 230, 231, 232, 233, 233, 234, 235, 235, 236,
      237, 237, 238, 239, 239, 240, 241, 241, 242, 242, 243, 243, 244, 244,
      245, 245, 245, 246, 246, 247, 247, 247, 248, 248, 248, 249, 249, 249,
      250, 250, 250, 250, 251, 251, 251, 251, 252, 252, 252, 252, 252, 252,
      253, 253, 253, 253, 253, 253, 253, 253, 253, 253, 254, 254, 254, 254,
      254, 254, 254, 254, 254, 254, 254, 254, 254, 254, 254, 254, 254, 254,
      254, 254, 254, 254, 254, 254, 255, 255, 255, 255, 255, 255
    };
#else
  static Quantum
    PCDMap[351] =  /* Photo CD information beyond 100% white, Gamma 2.2 */
    {
      0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18,
      19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 32, 33, 34, 35,
      36, 37, 38, 39, 40, 41, 42, 43, 45, 46, 47, 48, 49, 50, 51, 52,
      53, 54, 56, 57, 58, 59, 60, 61, 62, 63, 64, 66, 67, 68, 69, 70,
      71, 72, 73, 74, 76, 77, 78, 79, 80, 81, 82, 83, 84, 86, 87, 88,
      89, 90, 91, 92, 93, 94, 95, 97, 98, 99, 100, 101, 102, 103, 104,
      105, 106, 107, 108, 110, 111, 112, 113, 114, 115, 116, 117, 118,
      119, 120, 121, 122, 123, 124, 125, 126, 127, 129, 130, 131, 132,
      133, 134, 135, 136, 137, 138, 139, 140, 141, 142, 143, 144, 145,
      146, 147, 148, 149, 150, 151, 152, 153, 154, 155, 156, 157, 158,
      159, 160, 161, 162, 163, 164, 165, 166, 167, 168, 169, 170, 171,
      172, 173, 174, 175, 176, 176, 177, 178, 179, 180, 181, 182, 183,
      184, 185, 186, 187, 188, 189, 190, 191, 192, 193, 193, 194, 195,
      196, 197, 198, 199, 200, 201, 201, 202, 203, 204, 205, 206, 207,
      207, 208, 209, 210, 211, 211, 212, 213, 214, 215, 215, 216, 217,
      218, 218, 219, 220, 221, 221, 222, 223, 224, 224, 225, 226, 226,
      227, 228, 228, 229, 230, 230, 231, 232, 232, 233, 234, 234, 235,
      236, 236, 237, 237, 238, 238, 239, 240, 240, 241, 241, 242, 242,
      243, 243, 244, 244, 245, 245, 245, 246, 246, 247, 247, 247, 248,
      248, 248, 249, 249, 249, 249, 250, 250, 250, 250, 251, 251, 251,
      251, 251, 252, 252, 252, 252, 252, 253, 253, 253, 253, 253, 253,
      253, 253, 253, 253, 253, 253, 253, 254, 254, 254, 254, 254, 254,
      254, 254, 254, 254, 254, 254, 254, 254, 254, 254, 254, 254, 254,
      255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
      255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
      255, 255, 255, 255, 255, 255
    };
#endif

  long
    *blue,
    *green,
    *red;

  Quantum
    *range_table;

  register int
    i,
    x,
    y,
    z;

  register Quantum
    *range_limit;

  register RunlengthPacket
    *p;

  if ((colorspace == RGBColorspace) || (colorspace == GRAYColorspace))
    return;
  /*
    Allocate the tables.
  */
  red=(long *) malloc(3*(MaxRGB+1)*sizeof(long));
  green=(long *) malloc(3*(MaxRGB+1)*sizeof(long));
  blue=(long *) malloc(3*(MaxRGB+1)*sizeof(long));
  range_table=(Quantum *) malloc(3*(MaxRGB+1)*sizeof(Quantum));
  if ((red == (long *) NULL) || (green == (long *) NULL) ||
      (blue == (long *) NULL) || (range_table == (Quantum *) NULL))
    {
      Warning("Unable to transform color space","Memory allocation failed");
      return;
    }
  /*
    Initialize tables.
  */
  for (i=0; i <= MaxRGB; i++)
  {
    range_table[i]=0;
    range_table[i+(MaxRGB+1)]=(Quantum) i;
    range_table[i+(MaxRGB+1)*2]=MaxRGB;
  }
  range_limit=range_table+(MaxRGB+1);
  switch (colorspace)
  {
    case OHTAColorspace:
    {
      /*
        Initialize OHTA tables:

          R = I1+1.00000*I2-0.66668*I3
          G = I1+0.00000*I2+1.33333*I3
          B = I1-1.00000*I2-0.66668*I3

        I and Q, normally -0.5 through 0.5, must be normalized to the range 0
        through MaxRGB.
      */
      for (i=0; i <= MaxRGB; i++)
      {
        red[i+R]=UpShifted(1.00000)*i;
        green[i+R]=UpShifted(1.0000*0.5)*((i << 1)-MaxRGB);
        blue[i+R]=(-UpShifted(0.66668*0.5))*((i << 1)-MaxRGB);
        red[i+G]=UpShifted(1.00000)*i;
        green[i+G]=0;
        blue[i+G]=UpShifted(1.33333*0.5)*((i << 1)-MaxRGB);
        red[i+B]=UpShifted(1.00000)*i;
        green[i+B]=(-UpShifted(1.00000*0.5))*((i << 1)-MaxRGB);
        blue[i+B]=(-UpShifted(0.66668*0.5))*((i << 1)-MaxRGB);
      }
      break;
    }
    case XYZColorspace:
    {
      /*
        Initialize CIE XYZ tables:

          R =  3.240479*R-1.537150*G-0.498535*B
          G = -0.969256*R+1.875992*G+0.041556*B
          B =  0.055648*R-0.204043*G+1.057311*B
      */
      for (i=0; i <= MaxRGB; i++)
      {
        red[i+R]=UpShifted(3.240479)*i;
        green[i+R]=(-UpShifted(1.537150))*i;
        blue[i+R]=(-UpShifted(0.498535))*i;
        red[i+G]=(-UpShifted(0.969256))*i;
        green[i+G]=UpShifted(1.875992)*i;
        blue[i+G]=UpShifted(0.041556)*i;
        red[i+B]=UpShifted(0.055648)*i;
        green[i+B]=(-UpShifted(0.204043))*i;
        blue[i+B]=UpShifted(1.057311)*i;
      }
      break;
    }
    case YCbCrColorspace:
    {
      /*
        Initialize YCbCr tables:

          R = Y            +1.370707*Cr
          G = Y-0.336453*Cb-0.698195*Cr
          B = Y+1.732445*Cb

        Cb and Cr, normally -0.5 through 0.5, must be normalized to the range 0
        through MaxRGB.
      */
      for (i=0; i <= MaxRGB; i++)
      {
        red[i+R]=UpShifted(1.000000)*i;
        green[i+R]=0;
        blue[i+R]=UpShifted(1.370707*0.5)*((i << 1)-MaxRGB);
        red[i+G]=UpShifted(1.000000)*i;
        green[i+G]=(-UpShifted(0.336453*0.5))*((i << 1)-MaxRGB);
        blue[i+G]=(-UpShifted(0.698195*0.5))*((i << 1)-MaxRGB);
        red[i+B]=UpShifted(1.000000)*i;
        green[i+B]=UpShifted(1.732445*0.5)*((i << 1)-MaxRGB);
        blue[i+B]=0;
      }
      break;
    }
    case YCCColorspace:
    {
      /*
        Initialize YCC tables:

          R = Y            +1.340762*C2
          G = Y-0.317038*C1-0.682243*C2
          B = Y+1.632639*C1

        YCC is scaled by 1.3584.  C1 zero is 156 and C2 is at 137.
      */
      for (i=0; i <= MaxRGB; i++)
      {
        red[i+R]=UpShifted(1.3584)*i;
        green[i+R]=0;
        blue[i+R]=UpShifted(1.8215)*(i-UpScale(137));
        red[i+G]=UpShifted(1.3584)*i;
        green[i+G]=(-UpShifted(0.194*2.2179))*(i-UpScale(156));
        blue[i+G]=(-UpShifted(0.509*1.8215))*(i-UpScale(137));
        red[i+B]=UpShifted(1.3584)*i;
        green[i+B]=UpShifted(2.2179)*(i-UpScale(156));
        blue[i+B]=0;
        range_table[i+(MaxRGB+1)]=(Quantum) UpScale(PCDMap[DownScale(i)]);
      }
#ifdef HOW_IT_WAS
      for ( ; i < UpScale(348); i++)
        range_table[i+(MaxRGB+1)]=(Quantum) UpScale(PCDMap[DownScale(i)]);
#else
      for ( ; i < UpScale(351); i++)
      {
          /* This looks BUGGY to me because UpScale(351) could be greater than
          // the size of range table which is 3*(MaxRGB+1). However, if UpScale
          // is a no op, then we should be OK. I think that this is the case if
          // QuantumLeap is not defined. So lets make sure that it is not.
          */
#ifdef QuantumLeap
          BARF
#endif
          range_table[i+(MaxRGB+1)]=(Quantum) UpScale(PCDMap[DownScale(i)]);
      }
#endif
      break;
    }
    case YIQColorspace:
    {
      /*
        Initialize YIQ tables:

          R = 0.97087*Y+1.17782*I+0.59800*Q
          G = 0.97087*Y-0.28626*I-0.72851*Q
          B = 0.97087*Y-1.27870*I+1.72801*Q

        I and Q, normally -0.5 through 0.5, must be normalized to the range 0
        through MaxRGB.
      */
      for (i=0; i <= MaxRGB; i++)
      {
        red[i+R]=UpShifted(0.97087)*i;
        green[i+R]=UpShifted(1.17782*0.5)*((i << 1)-MaxRGB);
        blue[i+R]=UpShifted(0.59800*0.5)*((i << 1)-MaxRGB);
        red[i+G]=UpShifted(0.97087)*i;
        green[i+G]=(-UpShifted(0.28626*0.5))*((i << 1)-MaxRGB);
        blue[i+G]=(-UpShifted(0.72851*0.5))*((i << 1)-MaxRGB);
        red[i+B]=UpShifted(0.97087)*i;
        green[i+B]=(-UpShifted(1.27870*0.5))*((i << 1)-MaxRGB);
        blue[i+B]=UpShifted(1.72801*0.5)*((i << 1)-MaxRGB);
      }
      break;
    }
    case YPbPrColorspace:
    {
      /*
        Initialize YPbPr tables:

          R = Y            +1.402000*C2
          G = Y-0.344136*C1+0.714136*C2
          B = Y+1.772000*C1

        Pb and Pr, normally -0.5 through 0.5, must be normalized to the range 0
        through MaxRGB.
      */
      for (i=0; i <= MaxRGB; i++)
      {
        red[i+R]=UpShifted(1.000000)*i;
        green[i+R]=0;
        blue[i+R]=UpShifted(1.402000*0.5)*((i << 1)-MaxRGB);
        red[i+G]=UpShifted(1.000000)*i;
        green[i+G]=(-UpShifted(0.344136*0.5))*((i << 1)-MaxRGB);
        blue[i+G]=UpShifted(0.714136*0.5)*((i << 1)-MaxRGB);
        red[i+B]=UpShifted(1.000000)*i;
        green[i+B]=UpShifted(1.772000*0.5)*((i << 1)-MaxRGB);
        blue[i+B]=0;
      }
      break;
    }
    case YUVColorspace:
    default:
    {
      /*
        Initialize YUV tables:

          R = Y          +1.13980*V
          G = Y-0.39380*U-0.58050*V
          B = Y+2.02790*U

        U and V, normally -0.5 through 0.5, must be normalized to the range 0
        through MaxRGB.
      */
      for (i=0; i <= MaxRGB; i++)
      {
        red[i+R]=UpShifted(1.00000)*i;
        green[i+R]=0;
        blue[i+R]=UpShifted(1.13980*0.5)*((i << 1)-MaxRGB);
        red[i+G]=UpShifted(1.00000)*i;
        green[i+G]=(-UpShifted(0.39380*0.5))*((i << 1)-MaxRGB);
        blue[i+G]=(-UpShifted(0.58050*0.5))*((i << 1)-MaxRGB);
        red[i+B]=UpShifted(1.00000)*i;
        green[i+B]=UpShifted(2.02790*0.5)*((i << 1)-MaxRGB);
        blue[i+B]=0;
      }
      break;
    }
  }
  /*
    Convert to RGB.
  */
  switch (image->class)
  {
    case DirectClass:
    {
      /*
        Convert DirectClass image.
      */
      p=image->pixels;
      for (i=0; i < image->packets; i++)
      {
        x=p->red;
        y=p->green;
        z=p->blue;
        p->red=range_limit[DownShift(red[x+R]+green[y+R]+blue[z+R])];
        p->green=range_limit[DownShift(red[x+G]+green[y+G]+blue[z+G])];
        p->blue=range_limit[DownShift(red[x+B]+green[y+B]+blue[z+B])];
        p++;
        if (QuantumTick(i,image))
          ProgressMonitor(TransformRGBImageText,i,image->packets);
      }
      break;
    }
    case PseudoClass:
    {
      /*
        Convert PseudoClass image.
      */
      for (i=0; i < image->colors; i++)
      {
        x=image->colormap[i].red;
        y=image->colormap[i].green;
        z=image->colormap[i].blue;
        image->colormap[i].red=
          range_limit[DownShift(red[x+R]+green[y+R]+blue[z+R])];
        image->colormap[i].green=
          range_limit[DownShift(red[x+G]+green[y+G]+blue[z+G])];
        image->colormap[i].blue=
          range_limit[DownShift(red[x+B]+green[y+B]+blue[z+B])];
      }
      p=image->pixels;
      for (i=0; i < image->packets; i++)
      {
        x=p->red;
        y=p->green;
        z=p->blue;
        p->red=range_limit[DownShift(red[x+R]+green[y+R]+blue[z+R])];
        p->green=range_limit[DownShift(red[x+G]+green[y+G]+blue[z+G])];
        p->blue=range_limit[DownShift(red[x+B]+green[y+B]+blue[z+B])];
        p++;
      }
      break;
    }
  }
  /*
    Free allocated memory.
  */
  free((char *) range_table);
  free((char *) blue);
  free((char *) green);
  free((char *) red);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%     T r a n s p a r e n t I m a g e                                         %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Function TransparentImage creates a matte image associated with the
%  image.  All pixel locations are initially set to opaque.  Any pixel
%  that matches the specified color are set to transparent.
%
%  The format of the TransparentImage routine is:
%
%      TransparentImage(image,color)
%
%  A description of each parameter follows:
%
%    o image: The address of a structure of type Image;  returned from
%      ReadImage.
%
%    o color: A character string that contain an X11 color string.
%
%
*/
void TransparentImage(Image *image, char *color)
{
#define DeltaX  16
#define TransparentImageText  "  Setting transparent color in the image...  "

  register int
    i;

  Quantum
    blue,
    green,
    red;

  register RunlengthPacket
    *p;

  unsigned int
    status;

  XColor
    target_color;

  /*
    Determine RGB values of the transparent color.
  */
  status=XQueryColorDatabase(color,&target_color);
  if (status == False)
    return;
  red=XDownScale(target_color.red);
  green=XDownScale(target_color.green);
  blue=XDownScale(target_color.blue);
  /*
    Make image color transparent.
  */
  p=image->pixels;
  switch (image->class)
  {
    case DirectClass:
    {
      /*
        Make DirectClass image transparent.
      */
      if (!image->matte)
        {
          /*
            Initialize image matte to opaque.
          */
          for (i=0; i < image->packets; i++)
          {
            p->index=Opaque;
            p++;
          }
          image->matte=True;
          p=image->pixels;
        }
      for (i=0; i < image->packets; i++)
      {
        if (((int) p->red < (int) (red+DeltaX)) &&
            ((int) p->red > (int) (red-DeltaX)) &&
            ((int) p->green < (int) (green+DeltaX)) &&
            ((int) p->green > (int) (green-DeltaX)) &&
            ((int) p->blue < (int) (blue+DeltaX)) &&
            ((int) p->blue > (int) (blue-DeltaX)))
          p->index=Transparent;
        p++;
        if (QuantumTick(i,image))
          ProgressMonitor(TransparentImageText,i,image->packets);
      }
      break;
    }
    case PseudoClass:
    {
      double
        distance_squared,
        min_distance;

      int
        distance;

      register int
        index;

      /*
        Find closest color.
      */
      min_distance=3.0*(MaxRGB+1)*(MaxRGB+1);
      index=0;
      for (i=0; i < image->colors; i++)
      {
        distance=(int) red-(int) image->colormap[i].red;
        distance_squared=(unsigned int) (distance*distance);
        distance=(int) green-(int) image->colormap[i].green;
        distance_squared+=(unsigned int) (distance*distance);
        distance=(int) blue-(int) image->colormap[i].blue;
        distance_squared+=(unsigned int) (distance*distance);
        if (distance_squared < min_distance)
          {
            min_distance=distance_squared;
            index=i;
          }
      }
      /*
        Make PseudoClass image transparent.
      */
      image->class=DirectClass;
      image->matte=True;
      for (i=0; i < image->packets; i++)
      {
        if (p->index == index)
          p->index=Transparent;
        else
          p->index=Opaque;
        p++;
      }
      break;
    }
  }
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   U n C o m p r e s s I m a g e                                             %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Function UncompressImage uncompresses runlength-encoded pixels packets to
%  a rectangular array of pixels.
%
%  The format of the UncompressImage routine is:
%
%      status=UncompressImage(image)
%
%  A description of each parameter follows:
%
%    o status: Function UncompressImage returns True if the image is
%      uncompressed otherwise False.
%
%    o image: The address of a structure of type Image.
%
%
*/
unsigned int UncompressImage(Image *image)
{
  register int
    i,
    j,
    length;

  register RunlengthPacket
    *p,
    *q;

  RunlengthPacket
    *uncompressed_pixels;

  if (image->packets == (image->columns*image->rows))
    return(True);
  /*
    Uncompress runlength-encoded packets.
  */
  uncompressed_pixels=(RunlengthPacket *) realloc((char *) image->pixels,
    image->columns*image->rows*sizeof(RunlengthPacket));
  if (uncompressed_pixels == (RunlengthPacket *) NULL)
    return(False);
  image->pixels=uncompressed_pixels;
  p=image->pixels+image->packets-1;
  q=uncompressed_pixels+image->columns*image->rows-1;
  for (i=0; i < image->packets; i++)
  {
    length=p->length;
    for (j=0; j <= length; j++)
    {
      *q=(*p);
      q->length=0;
      q--;
    }
    p--;
  }
  image->packets=image->columns*image->rows;
  return(True);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   Z o o m I m a g e                                                         %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Function ZoomImage creates a new image that is a scaled size of an
%  existing one.  It allocates the memory necessary for the new Image
%  structure and returns a pointer to the new image.  The Point filter gives
%  fast pixel replication, Triangle is equivalent to bi-linear interpolation,
%  and Mitchel giver slower, very high-quality results.
%
%  The format of the ZoomImage routine is:
%
%      zoomed_image=ZoomImage(image,columns,rows,filter)
%
%  A description of each parameter follows:
%
%    o zoomed_image: Function ZoomImage returns a pointer to the image after
%      scaling.  A null image is returned if there is a memory shortage.
%
%    o image: The address of a structure of type Image.
%
%    o columns: An integer that specifies the number of columns in the zoomed
%      image.
%
%    o rows: An integer that specifies the number of rows in the scaled
%      image.
%
%    o filter: This unsigned integer is the filter type to used to zoom
%      the image.
%
%
*/

static double Box(double x)
{
  if ((x > -0.5) && (x <= 0.5))
    return(1.0);
  return(0.0);
}

static double Mitchell(double x)
{
  double
    b,
    c;

  b=1.0/3.0;
  c=1.0/3.0;
  if (x < 0)
    x=(-x);
  if (x < 1.0)
    {
      x=(((12.0-9.0*b-6.0*c)*(x*x*x))+((-18.0+12.0*b+6.0*c)*x*x)+(6.0-2.0*b))/
        6.0;
      return(x);
    }
 if (x < 2.0)
   {
     x=(((-1.0*b-6.0*c)*(x*x*x))+((6.0*b+30.0*c)*x*x)+((-12.0*b-48.0*c)*x)+
       (8.0*b+24.0*c))/6.0;
     return(x);
   }
  return(0.0);
}

static double Triangle(double x)
{
  if (x < 0.0)
    x=(-x);
  if (x < 1.0)
    return(1.0-x);
  return(0.0);
}

Image *ZoomImage(Image *image, unsigned int columns, unsigned int rows, unsigned int filter)
{
#define ZoomImageText  "  Zooming image...  "

  typedef struct _ContributionInfo
  {
    int
      pixel;

    long
      weight;
  } ContributionInfo;

  ContributionInfo
    *contribution_info;

  double
    center,
    filter_width,
    scale_factor,
    width,
    x_factor,
    y_factor;

  Image
    *source_image,
    *zoomed_image;

  int
    n,
    y;

  long
    blue_weight,
    green_weight,
    index_weight,
    red_weight,
    weight;

  Quantum
    *range_table;

  register int
    i,
    j,
    x;

  register Quantum
    *range_limit;

  register RunlengthPacket
    *p,
    *q;

  if ((columns == 0) || (rows == 0))
    {
      Warning("Unable to zoom image","image dimensions are zero");
      return((Image *) NULL);
    }
  /*
    Image must be uncompressed.
  */
  if (!UncompressImage(image))
    return((Image *) NULL);
  /*
    Initialize zoomed image attributes.
  */
  zoomed_image=CopyImage(image,columns,rows,False);
  if (zoomed_image == (Image *) NULL)
    {
      Warning("Unable to zoom image","Memory allocation failed");
      return((Image *) NULL);
    }
  zoomed_image->class=DirectClass;
  source_image=CopyImage(image,zoomed_image->columns,image->rows,False);
  if (source_image == (Image *) NULL)
    {
      Warning("Unable to zoom image","Memory allocation failed");
      DestroyImage(zoomed_image);
      return((Image *) NULL);
    }
  /*
    Allocate the range table.
  */
  range_table=(Quantum *) malloc(3*(MaxRGB+1)*sizeof(Quantum));
  if (range_table == (Quantum *) NULL)
    {
      Warning("Unable to zoom image","Memory allocation failed");
      DestroyImage(source_image);
      DestroyImage(zoomed_image);
      return((Image *) NULL);
    }
  /*
    Pre-compute conversion tables.
  */
  for (i=0; i <= MaxRGB; i++)
  {
    range_table[i]=0;
    range_table[i+(MaxRGB+1)]=(Quantum) i;
    range_table[i+(MaxRGB+1)*2]=MaxRGB;
  }
  range_limit=range_table+(MaxRGB+1);
  /*
    Allocate filter info list.
  */
  switch (filter)
  {
    case BoxFilter:
    {
      FilterFunction=Box;
      filter_width=0.5;
      break;
    }
    case TriangleFilter:
    {
      FilterFunction=Triangle;
      filter_width=1.0;
      break;
    }
    case MitchellFilter:
    default:
    {
      FilterFunction=Mitchell;
      filter_width=2.0;
      break;
    }
  }
  x_factor=(double) zoomed_image->columns/(double) image->columns;
  y_factor=(double) zoomed_image->rows/(double) image->rows;
  width=Max(filter_width/x_factor,filter_width/y_factor);
  if (width < filter_width)
    width=filter_width;
  contribution_info=(ContributionInfo *)
    malloc((int) (width*2+1)*sizeof(ContributionInfo));
  if (contribution_info == (ContributionInfo *) NULL)
    {
      Warning("Unable to zoom image","Memory allocation failed");
      free((char *) range_table);
      DestroyImage(source_image);
      DestroyImage(zoomed_image);
      return((Image *) NULL);
    }
  /*
    Apply filter to zoom horizontally from image to source.
  */
  width=filter_width;
  scale_factor=1.0;
  if (x_factor < 1.0)
    {
      width/=x_factor;
      scale_factor/=x_factor;
    }
  for (x=0; x < source_image->columns; x++)
  {
    n=0;
    center=(double) (x+0.5)/x_factor;
    for (i=(int) (center-width+0.5); i < (int) (center+width+0.5); i++)
    {
      j=i;
      if (j < 0)
        j=(-j);
      else
        if (j >= image->columns)
          j=(image->columns << 1)-j-1;
      contribution_info[n].pixel=j;
      contribution_info[n].weight=
        UpShifted((*FilterFunction)((i-center+0.5)/scale_factor)/scale_factor);
      n++;
    }
    q=source_image->pixels+x;
    for (y=0; y < source_image->rows; y++)
    {
      red_weight=0;
      green_weight=0;
      blue_weight=0;
      index_weight=0;
      for (i=0; i < n; i++)
      {
        weight=contribution_info[i].weight;
        p=image->pixels+(y*image->columns)+contribution_info[i].pixel;
        red_weight+=weight*p->red;
        green_weight+=weight*p->green;
        blue_weight+=weight*p->blue;
        index_weight+=weight*p->index;
      }
      q->red=range_limit[DownShift(red_weight)];
      q->green=range_limit[DownShift(green_weight)];
      q->blue=range_limit[DownShift(blue_weight)];
      q->index=range_limit[DownShift(index_weight)];
      q->length=0;
      q+=source_image->columns;
    }
    ProgressMonitor(ZoomImageText,x,source_image->columns+zoomed_image->rows);
  }
  /*
    Apply filter to zoom vertically from source to destination.
  */
  width=filter_width;
  scale_factor=1.0;
  if (y_factor < 1.0)
    {
      width/=y_factor;
      scale_factor/=y_factor;
    }
  q=zoomed_image->pixels;
  for (y=0; y < zoomed_image->rows; y++)
  {
    n=0;
    center=(double) (y+0.5)/y_factor;
    for (i=(int) (center-width+0.5); i < (int) (center+width+0.5); i++)
    {
      j=i;
      if (j < 0)
        j=(-j);
      else
       if (j >= source_image->rows)
         j=(source_image->rows << 1)-j-1;
      contribution_info[n].pixel=j;
      contribution_info[n].weight=
        UpShifted((*FilterFunction)((i-center+0.5)/scale_factor)/scale_factor);
      n++;
    }
    for (x=0; x < zoomed_image->columns; x++)
    {
      red_weight=0;
      green_weight=0;
      blue_weight=0;
      index_weight=0;
      for (i=0; i < n; i++)
      {
        weight=contribution_info[i].weight;
        p=source_image->pixels+
          (contribution_info[i].pixel*source_image->columns)+x;
        red_weight+=weight*p->red;
        green_weight+=weight*p->green;
        blue_weight+=weight*p->blue;
        index_weight+=weight*p->index;
      }
      q->red=range_limit[DownShift(red_weight)];
      q->green=range_limit[DownShift(green_weight)];
      q->blue=range_limit[DownShift(blue_weight)];
      q->index=range_limit[DownShift(index_weight)];
      q->length=0;
      q++;
    }
    ProgressMonitor(ZoomImageText,y+x,zoomed_image->rows+zoomed_image->rows);
  }
  /*
    Free allocated memory.
  */
  free((char *) contribution_info);
  free((char *) range_table);
  DestroyImage(source_image);
  return(zoomed_image);
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

#ifdef __cplusplus
}
#endif


/* -------------------------------------------------------------------------- */

#endif   /* #ifdef KJB_HAVE_X11  */

#endif   /* #ifndef __C2MAN__  */

