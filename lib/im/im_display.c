
/* $Id: im_display.c 16078 2013-11-23 21:01:35Z kobus $ */

#ifndef __C2MAN__

#ifndef lint

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%             DDDD   IIIII  SSSSS  PPPP   L       AAA   Y   Y                 %
%             D   D    I    SS     P   P  L      A   A   Y Y                  %
%             D   D    I     SSS   PPPP   L      AAAAA    Y                   %
%             D   D    I       SS  P      L      A   A    Y                   %
%             DDDD   IIIII  SSSSS  P      LLLLL  A   A    Y                   %
%                                                                             %
%                                                                             %
%          Display Machine Independent File Format Image via X11.             %
%                                                                             %
%                                                                             %
%                                                                             %
%                           Software Design                                   %
%                             John Cristy                                     %
%                              July 1992                                      %
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
%  Display is a machine architecture independent image processing
%  and display program.  It can display any image in the MIFF format on
%  any workstation display running X.  Display first determines the
%  hardware capabilities of the workstation.  If the number of unique
%  colors in the image is less than or equal to the number the workstation
%  can support, the image is displayed in an X window.  Otherwise the
%  number of colors in the image is first reduced to match the color
%  resolution of the workstation before it is displayed.
%
%  This means that a continuous-tone 24 bits-per-pixel image can display on a
%  8 bit pseudo-color device or monochrome device.  In most instances the
%  reduced color image closely resembles the original.  Alternatively, a
%  monochrome or pseudo-color image can display on a continuous-tone 24
%  bits-per-pixel device.
%
%  The Display program command syntax is:
%
%  Usage: display [options ...] file [ [options ...] file ...]
%
%  Where options include:
%    -backdrop          display image centered on a backdrop
%    -border geometry   surround image with a border of color
%    -box color         color for annotation bounding box
%    -colormap type     Shared or Private
%    -colors value      preferred number of colors in the image
%    -colorspace type   GRAY, OHTA, RGB, XYZ, YCbCr, YIQ, YPbPr, or YUV
%    -comment string    annotate image with comment",
%    -compress type     RunlengthEncoded or Zlib
%    -contrast          enhance or reduce the image contrast
%    -crop geometry     preferred size and location of the cropped image
%    -delay seconds     display the next image after pausing
%    -density geometry  vertical and horizontal density of the image
%    -despeckle         reduce the speckles within an image
%    -display server    display image to this X server
%    -dither            apply Floyd/Steinberg error diffusion to image
%    -edge factor       apply a filter to detect edges in the image
%    -enhance           apply a digital filter to enhance a noisy image
%    -equalize          perform histogram equalization to an image
%    -flip              flip image in the vertical direction
%    -flop              flop image in the horizontal direction
%    -frame geometry    surround image with an ornamental border
%    -gamma value       level of gamma correction
%    -geometry geometry preferred size and location of the Image window
%    -immutable         displayed image cannot be modified
%    -interlace type    NONE, LINE, or PLANE
%    -label name        assign a label to an image
%    -map type          display image using this Standard Colormap
%    -matte             store matte channel if the image has one
%    -modulate value    vary the brightness, saturation, and hue
%    -monochrome        transform image to black and white
%    -negate            apply color inversion to image
%    -noise             reduce noise with a noise peak elimination filter
%    -normalize         transform image to span the full the range of colors
%    -opaque color      change this color to the pen color
%    -page geometry     size and location of the Postscript page
%    -pen color         color for annotating or changing opaque color
%    -quality value     JPEG quality setting
%    -raise value       lighten/darken image edges to create a 3-D effect
%    -roll geometry     roll an image vertically or horizontally
%    -rotate degrees    apply Paeth rotation to the image
%    -sample geometry   scale image with pixel sampling
%    -scene value       image scene number
%    -segment value     segment an image
%    -sharpen factor    apply a filter to sharpen the image
%    -shear geometry    slide one edge of the image along the X or Y axis
%    -size geometry     width and height of image
%    -spread amount       displace image pixels by a random amount
%    -texture filename  name of texture to tile onto the image background
%    -treedepth value   depth of the color classification tree
%    -update seconds    detect when image file is modified and redisplay
%    -verbose           print detailed information about the image
%    -visual type       display image using this visual type
%    -window id         display image to background of this window
%    -window_group id   exit program when this window id is destroyed
%    -write filename    write image to a file
%
%  In addition to those listed above, you can specify these standard X
%  resources as command line options:  -background, -bordercolor,
%  -borderwidth, -font, -foreground, -iconGeometry, -iconic, -mattecolor,
%  -name, -shared_memory, -usePixmap, or -title.
%
%  Change '-' to '+' in any option above to reverse its effect.  For
%  example, specify +matte to store the image without its matte channel.
%
%  By default, the image format of `file' is determined by its magic
%  number.  To specify a particular image format, precede the filename
%  with an image format name and a colon (i.e. ps:image) or specify the
%  image type as the filename suffix (i.e. image.ps).  Specify 'file' as
%  '-' for standard input or output.
%
%  Buttons:
%    1    press to map or unmap the Command widget
%    2    press and drag to magnify a region of an image
%    3    press to load an image from a visual image directory
%
%
*/

/*
  Include declarations.
*/

/* TRANSITION */
#undef INT8
#undef INT16
#undef INT32

/* Kobus */
#include "im/im_gen.h"
/* END Kobus */

/*
// Kobus--moved this to im_gen.h to faciliate automatic determination of
// required archives.
//
#include "magick.h"
//
*/


/* -------------------------------------------------------------------------- */

#include "im/im_display.h"

#ifdef __cplusplus
extern "C" {
#endif

/* -------------------------------------------------------------------------- */

#ifdef KJB_HAVE_X11

#include "im/im_private.h"

/* -------------------------------------------------------------------------- */


/* Kobus */
static int fs_image_modified      = FALSE;
static int fs_horizontal_trace    = FALSE;
static int fs_vertical_trace      = FALSE;
static int fs_use_rg_chromaticity = FALSE;
static int fs_patch_output        = FALSE;
static int fs_point_output        = FALSE;
/* END Kobus */


static void update_pure_pixels
(
    Display*       display,
    XResourceInfo* resource_info,
    XWindows*      windows,
    XEvent*        event,
    Image**        image_ptr
);

static void XTraceImage
(
    Display*       display,
    XResourceInfo* resource_info,
    XWindows*      windows,
    XEvent*        event,
    Image**        image_ptr
);

static unsigned int XPatch
(
    Display*       display,
    XResourceInfo* resource_info,
    XWindows*      windows,
    XEvent*        event_ptr,
    Image*         image,
    unsigned char* pixels
);

static int XScreenEvent2(Display* display, XEvent* event, char* data);

static void process_button_three_directive(KeySym key_symbol);

static int im_do_horizontal_trace
(
    int            num_rows,
    int            num_cols,
    unsigned char* pixel_array,
    int            x,
    int            y
);

static int im_do_vertical_trace
(
    int            num_rows,
    int            num_cols,
    unsigned char* pixel_array,
    int            x,
    int            y
);

static int im_do_patch_output
(
    int            num_rows,
    int            num_cols,
    unsigned char* pixels,
    int            x1,
    int            y1,
    int            x2,
    int            y2
);

/* END Kobus */



/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   U s a g e                                                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Function Usage displays the program command syntax.
%
%  The format of the Usage routine is:
%
%      Usage(terminate)
%
%  A description of each parameter follows:
%
%    o terminate: A value other than zero is returned if the program is to
%      terminate immediately.
%
*/
static void Usage(unsigned int terminate)
{
  char
    **p;

  static char
    *buttons[]=
    {
      "1    press to map or unmap the Command widget",
      "2    press and drag to magnify a region of an image",
      "3    press to load an image from a visual image directory",
      (char *) NULL
    },
    *options[]=
    {
      "-backdrop          display image centered on a backdrop",
      "-border geometry   surround image with a border of color",
      "-box color         color for annotation bounding box",
      "-colormap type     Shared or Private",
      "-colors value      preferred number of colors in the image",
      "-colorspace type   GRAY, OHTA, RGB, XYZ, YCbCr, YIQ, YPbPr, or YUV",
      "-comment string    annotate image with comment",
      "-compress type     RunlengthEncoded or Zlib",
      "-contrast          enhance or reduce the image contrast",
      "-crop geometry     preferred size and location of the cropped image",
      "-delay seconds     display the next image after pausing",
      "-density geometry  vertical and horizontal density of the image",
      "-despeckle         reduce the speckles within an image",
      "-display server    display image to this X server",
      "-dither            apply Floyd/Steinberg error diffusion to image",
      "-edge factor       apply a filter to detect edges in the image",
      "-enhance           apply a digital filter to enhance a noisy image",
      "-equalize          perform histogram equalization to an image",
      "-flip              flip image in the vertical direction",
      "-flop              flop image in the horizontal direction",
      "-frame geometry    surround image with an ornamental border",
      "-gamma value       level of gamma correction",
      "-geometry geometry preferred size and location of the Image window",
      "-immutable         displayed image cannot be modified",
      "-interlace type    NONE, LINE, or PLANE",
      "-label name        assign a label to an image",
      "-map type          display image using this Standard Colormap",
      "-matte             store matte channel if the image has one",
      "-modulate value    vary the brightness, saturation, and hue",
      "-monochrome        transform image to black and white",
      "-negate            apply color inversion to image",
      "-noise             reduce noise with a noise peak elimination filter",
      "-normalize         transform image to span the full range of colors",
      "-opaque color      change this color to the pen color",
      "-page geometry     size and location of the Postscript page",
      "-pen color         color for annotating or changing opaque color",
      "-quality value     JPEG quality setting",
      "-raise value       lighten/darken image edges to create a 3-D effect",
      "-roll geometry     roll an image vertically or horizontally",
      "-rotate degrees    apply Paeth rotation to the image",
      "-scene value       image scene number",
      "-segment value     segment an image",
      "-sample geometry   scale image with pixel sampling",
      "-sharpen factor    apply a filter to sharpen the image",
      "-shear geometry    slide one edge of the image along the X or Y axis",
      "-size geometry     width and height of image",
      "-spread amount     displace image pixels by a random amount",
      "-texture filename  name of texture to tile onto the image background",
      "-treedepth value   depth of the color classification tree",
      "-update seconds    detect when image file is modified and redisplay",
      "-verbose           print detailed information about the image",
      "-visual type       display image using this visual type",
      "-window id         display image to background of this window",
      "-window_group id   exit program when this window id is destroyed",
      "-write filename    write image to a file",
      (char *) NULL
    };

  (void) printf("Version: %s\n\n",Version);
  (void) printf(
    "Usage: %s [-options ...] file [ [-options ...] file ...]\n",client_name);
  (void) printf("\nWhere options include: \n");
  for (p=options; *p != (char *) NULL; p++)
    (void) printf("  %s\n",*p);
  (void) printf(
    "\nIn addition to those listed above, you can specify these standard X\n");
  (void) printf(
    "resources as command line options:  -background, -bordercolor,\n");
  (void) printf(
    "-borderwidth, -font, -foreground, -iconGeometry, -iconic, -mattecolor,\n");
  (void) printf(
    "-name, -shared_memory, -usePixmap, or -title.\n");
  (void) printf(
    "\nChange '-' to '+' in any option above to reverse its effect.  For\n");
  (void) printf(
    "example, specify +matte to store the image without a matte channel.\n");
  (void) printf(
    "\nBy default, the image format of `file' is determined by its magic\n");
  (void) printf(
    "number.  To specify a particular image format, precede the filename\n");
  (void) printf(
    "with an image format name and a colon (i.e. ps:image) or specify the\n");
  (void) printf(
    "image type as the filename suffix (i.e. image.ps).  Specify 'file' as\n");
  (void) printf("'-' for standard input or output.\n");
  (void) printf("\nButtons: \n");
  for (p=buttons; *p != (char *) NULL; p++)
    (void) printf("  %s\n",*p);
  if (terminate)
    exit(1);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   X A n n o t a t e E d i t I m a g e                                       %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Function XAnnotateEditImage annotates the image with text.
%
%  The format of the XAnnotateEditImage routine is:
%
%    XAnnotateEditImage(display,resource_info,windows,image)
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
%    o image: Specifies a pointer to a Image structure; returned from
%      ReadImage.
%
*/
static unsigned int XAnnotateEditImage
(
    Display*       display,
    XResourceInfo* resource_info,
    XWindows*      windows,
    Image*         image
)
{
#define AnnotateModeNameOp  0
#define AnnotateModeFontColorOp  1
#define AnnotateModeBackgroundColorOp  2
#define AnnotateModeRotateOp  3
#define AnnotateModeHelpOp  4
#define AnnotateModeDismissOp  5
#define TextModeHelpOp  0
#define TextModeDismissOp  1

  static char
    *AnnotateModeMenu[]=
    {
      "Font Name",
      "Font Color",
      "Box Color",
      "Rotate Text",
      "Help",
      "Dismiss",
      (char *) NULL
    },
    *TextModeMenu[]=
    {
      "Help",
      "Dismiss",
      (char *) NULL
    };

  char
    *ColorMenu[MaxNumberPens+1],
    command[MaxTextLength],
    text[MaxTextLength];

  Cursor
    cursor;

  GC
    annotate_context;

  int
    id,
    pen_number,
    x,
    y;

  KeySym
    key_symbol;

  register char
    *p;

  register int
    i;

  static double
    degrees = 0.0;

  static unsigned int
    box_id = MaxNumberPens-2,
    font_id = 0,
    pen_id = 0,
    transparent_box = True,
    transparent_pen = False;

  unsigned int
    height,
    status,
    width;

  unsigned long
    state,
    x_factor,
    y_factor;

  XAnnotateInfo
    *annotate_info,
    *previous_info;

  XColor
    color;

  XFontStruct
    *font_info;

  XEvent
    event,
    text_event;

  /*
    Map Command widget.
  */
  windows->command.name="Annotate";
  windows->command.data=4;
  (void) XCommandWidget(display,windows,AnnotateModeMenu,(XEvent *) NULL);
  XMapRaised(display,windows->command.id);
  XClientMessage(display,windows->image.id,windows->im_protocols,
    windows->im_update_widget,CurrentTime);
  /*
    Track pointer until button 1 is pressed.
  */
  XQueryPosition(display,windows->image.id,&x,&y);
  XSelectInput(display,windows->image.id,windows->image.attributes.event_mask |
    PointerMotionMask);
  state=DefaultState;
  do
  {
    if (windows->info.mapped)
      {
        /*
          Display pointer position.
        */
        (void) sprintf(text," %+d%+d ",x-windows->image.x,y-windows->image.y);
        XInfoWidget(display,windows,text);
      }
    /*
      Wait for next event.
    */
    XIfEvent(display,&event,XScreenEvent,(char *) windows);
    if (event.xany.window == windows->command.id)
      {
        /*
          Select a command from the Command widget.
        */
        id=XCommandWidget(display,windows,AnnotateModeMenu,&event);
        if (id < 0)
          continue;
        switch (id)
        {
          case AnnotateModeNameOp:
          {
            char
              *FontMenu[MaxNumberFonts];

            int
              font_number;

            /*
              Initialize menu selections.
            */
            for (i=0; i < MaxNumberFonts; i++)
              FontMenu[i]=resource_info->font_name[i];
            FontMenu[MaxNumberFonts-2]="Browser...";
            FontMenu[MaxNumberFonts-1]=(char *) NULL;
            /*
              Select a font name from the pop-up menu.
            */
            font_number=XMenuWidget(display,windows,AnnotateModeMenu[id],
              FontMenu,command);
            if (font_number < 0)
              break;
            if (font_number == (MaxNumberFonts-2))
              {
                static char
                  font_name[MaxTextLength]="fixed";

                /*
                  Select a font name from a browser.
                */
                resource_info->font_name[font_number]=font_name;
                XFontBrowserWidget(display,windows,"Select",font_name);
                if (*font_name == '\0')
                  break;
              }
            /*
              Initialize font info.
            */
            font_info=
              XLoadQueryFont(display,resource_info->font_name[font_number]);
            if (font_info == (XFontStruct *) NULL)
              {
                XNoticeWidget(display,windows,"Unable to load font:",
                  resource_info->font_name[font_number]);
                break;
              }
            font_id=font_number;
            XFreeFont(display,font_info);
            break;
          }
          case AnnotateModeFontColorOp:
          {
            /*
              Initialize menu selections.
            */
            for (i=0; i < (MaxNumberPens-2); i++)
              ColorMenu[i]=resource_info->pen_colors[i];
            ColorMenu[MaxNumberPens-2]="transparent";
            ColorMenu[MaxNumberPens-1]="Browser...";
            ColorMenu[MaxNumberPens]=(char *) NULL;
            /*
              Select a pen color from the pop-up menu.
            */
            pen_number=XMenuWidget(display,windows,AnnotateModeMenu[id],
              ColorMenu,command);
            if (pen_number < 0)
              break;
            transparent_pen=pen_number == (MaxNumberPens-2);
            if (transparent_pen)
              break;
            if (pen_number == (MaxNumberPens-1))
              {
                static char
                  color_name[MaxTextLength] = "gray";

                /*
                  Select a pen color from a dialog.
                */
                resource_info->pen_colors[pen_number]=color_name;
                XColorBrowserWidget(display,windows,"Select",color_name);
                if (*color_name == '\0')
                  break;
              }
            /*
              Set pen color.
            */
            (void) XParseColor(display,windows->image.map_info->colormap,
              resource_info->pen_colors[pen_number],&color);
            XBestPixel(display,windows->image.map_info->colormap,
              (XColor *) NULL,(unsigned int) MaxColors,&color);
            windows->image.pixel_info->pen_colors[pen_number]=color;
            pen_id=pen_number;
            break;
          }
          case AnnotateModeBackgroundColorOp:
          {
            /*
              Initialize menu selections.
            */
            for (i=0; i < (MaxNumberPens-2); i++)
              ColorMenu[i]=resource_info->pen_colors[i];
            ColorMenu[MaxNumberPens-2]="transparent";
            ColorMenu[MaxNumberPens-1]="Browser...";
            ColorMenu[MaxNumberPens]=(char *) NULL;
            /*
              Select a pen color from the pop-up menu.
            */
            pen_number=XMenuWidget(display,windows,AnnotateModeMenu[id],
              ColorMenu,command);
            if (pen_number < 0)
              break;
            transparent_box=pen_number == (MaxNumberPens-2);
            if (transparent_box)
              break;
            if (pen_number == (MaxNumberPens-1))
              {
                static char
                  color_name[MaxTextLength] = "gray";

                /*
                  Select a pen color from a dialog.
                */
                resource_info->pen_colors[pen_number]=color_name;
                XColorBrowserWidget(display,windows,"Select",color_name);
                if (*color_name == '\0')
                  break;
              }
            /*
              Set pen color.
            */
            (void) XParseColor(display,windows->image.map_info->colormap,
              resource_info->pen_colors[pen_number],&color);
            XBestPixel(display,windows->image.map_info->colormap,
              (XColor *) NULL,(unsigned int) MaxColors,&color);
            windows->image.pixel_info->pen_colors[pen_number]=color;
            box_id=pen_number;
            break;
          }
          case AnnotateModeRotateOp:
          {
            int
              entry;

            static char
              angle[MaxTextLength] = "30.0",
              *RotateMenu[]=
              {
                "0",
                "45",
                "90",
                "135",
                "180",
                "225",
                "270",
                "315",
                (char *) NULL,
                (char *) NULL,
              };

            /*
              Select a command from the pop-up menu.
            */
            RotateMenu[8]="Dialog...";
            entry=XMenuWidget(display,windows,AnnotateModeMenu[id],RotateMenu,
              command);
            if (entry < 0)
              break;
            if (entry != 8)
              {
                degrees=atof(RotateMenu[entry]);
                break;
              }
            (void) XDialogWidget(display,windows,"OK","Enter rotation angle:",
              angle);
            if (*angle == '\0')
              break;
            degrees=atof(angle);
            break;
          }
          case AnnotateModeHelpOp:
          {
            XTextViewWidget(display,resource_info,windows,False,
              "Help Viewer - Image Annotation",ImageAnnotateHelp);
            break;
          }
          case AnnotateModeDismissOp:
          {
            /*
              Prematurely exit.
            */
            state|=EscapeState;
            state|=ExitState;
            break;
          }
          default:
            break;
        }
        continue;
      }
    switch (event.type)
    {
      case ButtonPress:
      {
        if (event.xbutton.window == windows->pan.id)
          {
            XPanImage(display,windows,&event);
            XInfoWidget(display,windows,text);
            break;
          }
        if (event.xbutton.window != windows->image.id)
          break;
        /*
          Change to text entering mode.
        */
        x=event.xbutton.x;
        y=event.xbutton.y;
        state|=ExitState;
        break;
      }
      case ButtonRelease:
        break;
      case Expose:
        break;
      case KeyPress:
      {
        if (event.xkey.window != windows->image.id)
          break;
        /*
          Respond to a user key press.
        */
        (void) XLookupString((XKeyEvent *) &event.xkey,command,sizeof(command),
          &key_symbol,(XComposeStatus *) NULL);
        switch (key_symbol)
        {
          case XK_Escape:
          case XK_F20:
          {
            /*
              Prematurely exit.
            */
            state|=EscapeState;
            state|=ExitState;
            break;
          }
          case XK_F1:
          case XK_Help:
          {
            XTextViewWidget(display,resource_info,windows,False,
              "Help Viewer - Image Annotation",ImageAnnotateHelp);
            break;
          }
          default:
          {
            XBell(display,0);
            break;
          }
        }
        break;
      }
      case MotionNotify:
      {
        /*
          Discard pending pointer motion events.
        */
        while (XCheckMaskEvent(display,PointerMotionMask,&event));
        x=event.xmotion.x;
        y=event.xmotion.y;
        /*
          Map and unmap Info widget as cursor crosses its boundaries.
        */
        if (windows->info.mapped)
          {
            if ((x < (windows->info.x+windows->info.width)) &&
                (y < (windows->info.y+windows->info.height)))
              XWithdrawWindow(display,windows->info.id,windows->info.screen);
          }
        else
          if ((x > (windows->info.x+windows->info.width)) ||
              (y > (windows->info.y+windows->info.height)))
            XMapWindow(display,windows->info.id);
        break;
      }
      default:
        break;
    }
  } while (!(state & ExitState));
  XSelectInput(display,windows->image.id,windows->image.attributes.event_mask);
  XWithdrawWindow(display,windows->info.id,windows->info.screen);
  if (state & EscapeState)
    return(True);
  /*
    Set font info and check boundary conditions.
  */
  font_info=XLoadQueryFont(display,resource_info->font_name[font_id]);
  if (font_info == (XFontStruct *) NULL)
    {
      XNoticeWidget(display,windows,"Unable to load font:",
        resource_info->font_name[font_id]);
      font_info=windows->image.font_info;
    }
  if ((x+font_info->max_bounds.width) >= windows->image.width)
    x=windows->image.width-font_info->max_bounds.width;
  if (y < (font_info->ascent+font_info->descent))
    y=font_info->ascent+font_info->descent;
  if ((font_info->max_bounds.width > windows->image.width) ||
      ((font_info->ascent+font_info->descent) >= windows->image.height))
    return(False);
  /*
    Initialize annotate structure.
  */
  annotate_info=(XAnnotateInfo *) malloc(sizeof(XAnnotateInfo));
  if (annotate_info == (XAnnotateInfo *) NULL)
    return(False);
  XGetAnnotateInfo(annotate_info);
  annotate_info->x=x;
  annotate_info->y=y;
  if (!transparent_box && !transparent_pen)
    annotate_info->stencil=OpaqueStencil;
  else
    if (!transparent_box)
      annotate_info->stencil=BackgroundStencil;
    else
      annotate_info->stencil=ForegroundStencil;
  annotate_info->height=font_info->ascent+font_info->descent;
  annotate_info->degrees=degrees;
  annotate_info->font_info=font_info;
  annotate_info->text=(char *) malloc(
    (windows->image.width/Max(font_info->min_bounds.width,1)+2)*sizeof(char));
  if (annotate_info->text == (char *) NULL)
    return(False);
  /*
    Create cursor and set graphic context.
  */
  cursor=XCreateFontCursor(display,XC_pencil);
  XDefineCursor(display,windows->image.id,cursor);
  annotate_context=windows->image.annotate_context;
  XSetFont(display,annotate_context,font_info->fid);
  XSetBackground(display,annotate_context,
    windows->image.pixel_info->pen_colors[box_id].pixel);
  XSetForeground(display,annotate_context,
    windows->image.pixel_info->pen_colors[pen_id].pixel);
  /*
    Begin annotating the image with text.
  */
  windows->command.name="Text";
  windows->command.data=0;
  (void) XCommandWidget(display,windows,TextModeMenu,(XEvent *) NULL);
  state=DefaultState;
  XDrawString(display,windows->image.id,annotate_context,x,y,"_",1);
  text_event.xexpose.width=(unsigned int) font_info->max_bounds.width;
  text_event.xexpose.height=font_info->max_bounds.ascent+
    font_info->max_bounds.descent;
  p=annotate_info->text;
  do
  {
    /*
      Display text cursor.
    */
    *p='\0';
    XDrawString(display,windows->image.id,annotate_context,x,y,"_",1);
    /*
      Wait for next event.
    */
    XIfEvent(display,&event,XScreenEvent,(char *) windows);
    if (event.xany.window == windows->command.id)
      {
        /*
          Select a command from the Command widget.
        */
        XSetBackground(display,annotate_context,
          windows->image.pixel_info->background_color.pixel);
        XSetForeground(display,annotate_context,
          windows->image.pixel_info->foreground_color.pixel);
        id=XCommandWidget(display,windows,AnnotateModeMenu,&event);
        XSetBackground(display,annotate_context,
          windows->image.pixel_info->pen_colors[box_id].pixel);
        XSetForeground(display,annotate_context,
          windows->image.pixel_info->pen_colors[pen_id].pixel);
        if (id < 0)
          continue;
        switch (id)
        {
          case TextModeHelpOp:
          {
            XTextViewWidget(display,resource_info,windows,False,
              "Help Viewer - Image Annotation",ImageAnnotateHelp);
            XDefineCursor(display,windows->image.id,cursor);
            break;
          }
          case TextModeDismissOp:
          {
            /*
              Finished annotating.
            */
            annotate_info->width=XTextWidth(font_info,annotate_info->text,
              strlen(annotate_info->text));
            XRefreshWindow(display,&windows->image,&text_event);
            state|=ExitState;
            break;
          }
          default:
            break;
        }
        continue;
      }
    /*
      Erase text cursor.
    */
    text_event.xexpose.x=x;
    text_event.xexpose.y=y-font_info->max_bounds.ascent;
    XClearArea(display,windows->image.id,x,text_event.xexpose.y,
      text_event.xexpose.width,text_event.xexpose.height,False);
    XRefreshWindow(display,&windows->image,&text_event);
    switch (event.type)
    {
      case ButtonPress:
      {
        if (event.xbutton.window == windows->pan.id)
          {
            XPanImage(display,windows,&event);
            break;
          }
        if (event.xbutton.button == Button2)
          {
            /*
              Request primary selection.
            */
            XConvertSelection(display,XA_PRIMARY,XA_STRING,XA_STRING,
              windows->image.id,CurrentTime);
            break;
          }
        break;
      }
      case Expose:
      {
        if (event.xexpose.count == 0)
          {
            XAnnotateInfo
              *text_info;

            /*
              Refresh Image window.
            */
            XRefreshWindow(display,&windows->image,(XEvent *) NULL);
            text_info=annotate_info;
            while (text_info != (XAnnotateInfo *) NULL)
            {
              if (annotate_info->stencil == ForegroundStencil)
                XDrawString(display,windows->image.id,annotate_context,
                  text_info->x,text_info->y,text_info->text,
                  strlen(text_info->text));
              else
                XDrawImageString(display,windows->image.id,annotate_context,
                  text_info->x,text_info->y,text_info->text,
                  strlen(text_info->text));
              text_info=text_info->previous;
            }
            XDrawString(display,windows->image.id,annotate_context,x,y,"_",1);
          }
        break;
      }
      case KeyPress:
      {
        int
          length;

        if (event.xkey.window != windows->image.id)
          break;
        /*
          Respond to a user key press.
        */
        length=XLookupString((XKeyEvent *) &event.xkey,command,sizeof(command),
          &key_symbol,(XComposeStatus *) NULL);
        *(command+length)='\0';
        if ((event.xkey.state & ControlMask) || (event.xkey.state & Mod1Mask))
          state|=ModifierState;
        if (state & ModifierState)
          switch (key_symbol)
          {
            case XK_u:
            case XK_U:
            {
              key_symbol=XK_Delete;
              break;
            }
            default:
              break;
          }
        switch (key_symbol)
        {
          case XK_BackSpace:
          {
            /*
              Erase one character.
            */
            if (p == annotate_info->text)
              if (annotate_info->previous == (XAnnotateInfo *) NULL)
                break;
              else
                {
                  /*
                    Go to end of the previous line of text.
                  */
                  annotate_info=annotate_info->previous;
                  p=annotate_info->text;
                  x=annotate_info->x+annotate_info->width;
                  y=annotate_info->y;
                  if (annotate_info->width != 0)
                    p+=strlen(annotate_info->text);
                  break;
                }
            p--;
            x-=XTextWidth(font_info,p,1);
            text_event.xexpose.x=x;
            text_event.xexpose.y=y-font_info->max_bounds.ascent;
            XRefreshWindow(display,&windows->image,&text_event);
            break;
          }
          case XK_Delete:
          {
            /*
              Erase the entire line of text.
            */
            while (p != annotate_info->text)
            {
              p--;
              x-=XTextWidth(font_info,p,1);
              text_event.xexpose.x=x;
              XRefreshWindow(display,&windows->image,&text_event);
            }
            break;
          }
          case XK_Escape:
          case XK_F20:
          {
            /*
              Finished annotating.
            */
            annotate_info->width=XTextWidth(font_info,annotate_info->text,
              strlen(annotate_info->text));
            XRefreshWindow(display,&windows->image,&text_event);
            state|=ExitState;
            break;
          }
          default:
          {
            /*
              Draw a single character on the Image window.
            */
            if (state & ModifierState)
              break;
            if (*command == '\0')
              break;
            *p=(*command);
            if (annotate_info->stencil == ForegroundStencil)
              XDrawString(display,windows->image.id,annotate_context,x,y,p,1);
            else
              XDrawImageString(display,windows->image.id,annotate_context,
                x,y,p,1);
            x+=XTextWidth(font_info,p,1);
            p++;
            if ((x+font_info->max_bounds.width) < windows->image.width)
              break;
          }
          case XK_Return:
          case XK_KP_Enter:
          {
            /*
              Advance to the next line of text.
            */
            *p='\0';
            annotate_info->width=XTextWidth(font_info,annotate_info->text,
              strlen(annotate_info->text));
            if (annotate_info->next != (XAnnotateInfo *) NULL)
              {
                /*
                  Line of text already exists.
                */
                annotate_info=annotate_info->next;
                x=annotate_info->x;
                y=annotate_info->y;
                p=annotate_info->text;
                break;
              }
            annotate_info->next=(XAnnotateInfo *) malloc(sizeof(XAnnotateInfo));
            if (annotate_info->next == (XAnnotateInfo *) NULL)
              return(False);
            *annotate_info->next=(*annotate_info);
            annotate_info->next->previous=annotate_info;
            annotate_info=annotate_info->next;
            annotate_info->text=(char *) malloc((windows->image.width/
              Max(font_info->min_bounds.width,1)+2)*sizeof(char));
            if (annotate_info->text == (char *) NULL)
              return(False);
            annotate_info->y+=annotate_info->height;
            if (annotate_info->y > windows->image.height)
              annotate_info->y=annotate_info->height;
            annotate_info->next=(XAnnotateInfo *) NULL;
            x=annotate_info->x;
            y=annotate_info->y;
            p=annotate_info->text;
            break;
          }
        }
        break;
      }
      case KeyRelease:
      {
        /*
          Respond to a user key release.
        */
        (void) XLookupString((XKeyEvent *) &event.xkey,command,sizeof(command),
          &key_symbol,(XComposeStatus *) NULL);
        state&=(~ModifierState);
        break;
      }
      case SelectionNotify:
      {
        Atom
          type;

        int
          format;

        unsigned char
          *data;

        unsigned long
          after,
          length;

        /*
          Obtain response from primary selection.
        */
        if (event.xselection.property == (Atom) None)
          break;
        status=XGetWindowProperty(display,event.xselection.requestor,
          event.xselection.property,0L,2047L,True,XA_STRING,&type,&format,
          &length,&after,&data);
        if ((status != Success) || (type != XA_STRING) || (format == 32) ||
            (length == 0))
          break;
        /*
          Annotate Image window with primary selection.
        */
        for (i=0; i < length; i++)
        {
          if (data[i] != '\n')
            {
              /*
                Draw a single character on the Image window.
              */
              *p=data[i];
              XDrawString(display,windows->image.id,annotate_context,x,y,p,1);
              x+=XTextWidth(font_info,p,1);
              p++;
              if ((x+font_info->max_bounds.width) < windows->image.width)
                continue;
            }
          /*
            Advance to the next line of text.
          */
          *p='\0';
          annotate_info->width=XTextWidth(font_info,annotate_info->text,
            strlen(annotate_info->text));
          if (annotate_info->next != (XAnnotateInfo *) NULL)
            {
              /*
                Line of text already exists.
              */
              annotate_info=annotate_info->next;
              x=annotate_info->x;
              y=annotate_info->y;
              p=annotate_info->text;
              continue;
            }
          annotate_info->next=(XAnnotateInfo *) malloc(sizeof(XAnnotateInfo));
          if (annotate_info->next == (XAnnotateInfo *) NULL)
            return(False);
          *annotate_info->next=(*annotate_info);
          annotate_info->next->previous=annotate_info;
          annotate_info=annotate_info->next;
          annotate_info->text=(char *) malloc((windows->image.width/
            Max(font_info->min_bounds.width,1)+2)*sizeof(char));
          if (annotate_info->text == (char *) NULL)
            return(False);
          annotate_info->y+=annotate_info->height;
          if (annotate_info->y > windows->image.height)
            annotate_info->y=annotate_info->height;
          annotate_info->next=(XAnnotateInfo *) NULL;
          x=annotate_info->x;
          y=annotate_info->y;
          p=annotate_info->text;
        }
        XFree((void *) data);
        break;
      }
      default:
        break;
    }
  } while (!(state & ExitState));
  XFreeCursor(display,cursor);
  /*
    Annotation is relative to image configuration.
  */
  x=0;
  y=0;
  width=image->columns;
  height=image->rows;
  if (windows->image.crop_geometry != (char *) NULL)
    (void) XParseGeometry(windows->image.crop_geometry,&x,&y,&width,&height);
  /*
    Initialize annotated image.
  */
  XSetCursorState(display,windows,True);
  XCheckRefreshWindows(display,windows);
  while (annotate_info != (XAnnotateInfo *) NULL)
  {
    if (annotate_info->width == 0)
      {
        /*
          No text on this line--  go to the next line of text.
        */
        previous_info=annotate_info->previous;
        free((char *) annotate_info->text);
        free((char *) annotate_info);
        annotate_info=previous_info;
        continue;
      }
    /*
      Determine pixel index for box and pen color.
    */
    windows->image.pixel_info->box_color=
      windows->image.pixel_info->pen_colors[box_id];
    if (windows->image.pixel_info->colors != 0)
      for (i=0; i < windows->image.pixel_info->colors; i++)
        if (windows->image.pixel_info->pixels[i] ==
            windows->image.pixel_info->pen_colors[box_id].pixel)
          {
            windows->image.pixel_info->box_index=i;
            break;
          }
    windows->image.pixel_info->pen_color=
      windows->image.pixel_info->pen_colors[pen_id];
    if (windows->image.pixel_info->colors != 0)
      for (i=0; i < windows->image.pixel_info->colors; i++)
        if (windows->image.pixel_info->pixels[i] ==
            windows->image.pixel_info->pen_colors[pen_id].pixel)
          {
            windows->image.pixel_info->pen_index=i;
            break;
          }
    /*
      Define the annotate geometry string.
    */
    x_factor=UpShift(width)/windows->image.ximage->width;
    annotate_info->x+=windows->image.x;
    annotate_info->x=DownShift(annotate_info->x*x_factor);
    y_factor=UpShift(height)/windows->image.ximage->height;
    annotate_info->y+=(windows->image.y-font_info->ascent);
    annotate_info->y=DownShift(annotate_info->y*y_factor);
    (void) sprintf(annotate_info->geometry,"%ux%u%+d%+d",
      (unsigned int) DownShift(annotate_info->width*x_factor),
      (unsigned int) DownShift(annotate_info->height*y_factor),
      annotate_info->x+x,annotate_info->y+y);
    /*
      Annotate image with text.
    */
    status=XAnnotateImage(display,windows->image.pixel_info,annotate_info,
      False,image);
    if (status == 0)
      return(False);
    /*
      Free up memory.
    */
    previous_info=annotate_info->previous;
    free((char *) annotate_info->text);
    free((char *) annotate_info);
    annotate_info=previous_info;
  }
  XSetForeground(display,annotate_context,
    windows->image.pixel_info->foreground_color.pixel);
  XSetBackground(display,annotate_context,
    windows->image.pixel_info->background_color.pixel);
  XSetFont(display,annotate_context,windows->image.font_info->fid);
  XSetCursorState(display,windows,False);
  XFreeFont(display,font_info);
  /*
    Update image configuration.
  */
  XConfigureImageColormap(display,resource_info,windows,image);
  (void) XConfigureImage(display,resource_info,windows,image);
  return(True);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   X B a c k g r o u n d I m a g e                                           %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Function XBackgroundImage displays the image in the background of a window.
%
%  The format of the XBackgroundImage routine is:
%
%    status=XBackgroundImage(display,resource_info,windows,image)
%
%  A description of each parameter follows:
%
%    o status: Function XBackgroundImage return True if the image is
%      printed.  False is returned is there is a memory shortage or if the
%      image fails to print.
%
%    o display: Specifies a connection to an X server; returned from
%      XOpenDisplay.
%
%    o resource_info: Specifies a pointer to a X11 XResourceInfo structure.
%
%    o windows: Specifies a pointer to a XWindows structure.
%
%    o image: Specifies a pointer to a Image structure;  returned from
%      ReadImage.
%
%
*/
static unsigned int XBackgroundImage
(
    Display*       display,
    XResourceInfo* resource_info,
    XWindows*      windows,
    Image**        image
)
{
#define BackgroundImageText  "  Backgrounding the image...  "

  static char
    window_id[MaxTextLength] = "root";

  XResourceInfo
    background_resources;

  unsigned int
    status;

  /*
    Put image in background.
  */
  status=XDialogWidget(display,windows,"Background",
    "Enter window id (id 0x00 selects window with pointer):",window_id);
  if (*window_id == '\0')
    return(False);
  (void) XMagickCommand(display,resource_info,windows,0,(KeySym) XK_A,image);
  XInfoWidget(display,windows,BackgroundImageText);
  XSetCursorState(display,windows,True);
  XCheckRefreshWindows(display,windows);
  background_resources=(*resource_info);
  background_resources.window_id=window_id;
  background_resources.backdrop=status;
  status=XDisplayBackgroundImage(display,&background_resources,*image);
  if (status)
    XClientMessage(display,windows->image.id,windows->im_protocols,
      windows->im_retain_colors,CurrentTime);
  XSetCursorState(display,windows,False);
  (void) XMagickCommand(display,resource_info,windows,0,(KeySym) XK_u,image);
  return(True);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   X C h o p I m a g e                                                       %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Function XChopImage chops the X image.
%
%  The format of the XChopImage routine is:
%
%    status=XChopImage(display,resource_info,windows,image)
%
%  A description of each parameter follows:
%
%    o status: Function XChopImage return True if the image is
%      cut.  False is returned is there is a memory shortage or if the
%      image fails to cut.
%
%    o display: Specifies a connection to an X server; returned from
%      XOpenDisplay.
%
%    o resource_info: Specifies a pointer to a X11 XResourceInfo structure.
%
%    o windows: Specifies a pointer to a XWindows structure.
%
%    o image: Specifies a pointer to a Image structure;  returned from
%      ReadImage.
%
%
*/
static unsigned int XChopImage
(
    Display*       display,
    XResourceInfo* resource_info,
    XWindows*      windows,
    Image**        image
)
{
#define ChopModeDirectionOp  0
#define ChopModeHelpOp  1
#define ChopModeDismissOp  2
#define HorizontalChopOp  0
#define VerticalChopOp  1

  static char
    *ChopModeMenu[]=
    {
      "Direction",
      "Help",
      "Dismiss",
      (char *) NULL
    };

  char
    text[MaxTextLength];

  Image
    *chop_image;

  int
    id,
    x,
    y;

  RectangleInfo
    chop_info;

  static unsigned int
    direction = HorizontalChopOp;

  unsigned int
    distance,
    height,
    width;

  unsigned long
    scale_factor,
    state;

  XEvent
    event;

  XSegment
    segment_info;

  /*
    Map Command widget.
  */
  windows->command.name="Chop";
  windows->command.data=1;
  (void) XCommandWidget(display,windows,ChopModeMenu,(XEvent *) NULL);
  XMapRaised(display,windows->command.id);
  XClientMessage(display,windows->image.id,windows->im_protocols,
    windows->im_update_widget,CurrentTime);
  /*
    Track pointer until button 1 is pressed.
  */
  XQueryPosition(display,windows->image.id,&x,&y);
  XSelectInput(display,windows->image.id,windows->image.attributes.event_mask |
    PointerMotionMask);
  state=DefaultState;
  do
  {
    if (windows->info.mapped)
      {
        /*
          Display pointer position.
        */
        (void) sprintf(text," %+d%+d ",x-windows->image.x,y-windows->image.y);
        XInfoWidget(display,windows,text);
      }
    /*
      Wait for next event.
    */
    XIfEvent(display,&event,XScreenEvent,(char *) windows);
    if (event.xany.window == windows->command.id)
      {
        /*
          Select a command from the Command widget.
        */
        id=XCommandWidget(display,windows,ChopModeMenu,&event);
        if (id < 0)
          continue;
        switch (id)
        {
          case ChopModeDirectionOp:
          {
            char
              command[MaxTextLength];

            static char
              *Directions[]=
              {
                "horizontal",
                "vertical",
                (char *) NULL,
              };

            /*
              Select a command from the pop-up menu.
            */
            direction=
              XMenuWidget(display,windows,ChopModeMenu[id],Directions,command);
            break;
          }
          case ChopModeHelpOp:
          {
            XTextViewWidget(display,resource_info,windows,False,
              "Help Viewer - Image Chopping",ImageChopHelp);
            break;
          }
          case ChopModeDismissOp:
          {
            /*
              Prematurely exit.
            */
            state|=EscapeState;
            state|=ExitState;
            break;
          }
          default:
            break;
        }
        continue;
      }
    switch (event.type)
    {
      case ButtonPress:
      {
        if (event.xbutton.window == windows->pan.id)
          {
            XPanImage(display,windows,&event);
            XInfoWidget(display,windows,text);
            break;
          }
        /*
          User has committed to start point of chopting line.
        */
        segment_info.x1=event.xbutton.x;
        segment_info.x2=event.xbutton.x;
        segment_info.y1=event.xbutton.y;
        segment_info.y2=event.xbutton.y;
        state|=ExitState;
        break;
      }
      case ButtonRelease:
        break;
      case Expose:
        break;
      case KeyPress:
      {
        char
          command[MaxTextLength];

        KeySym
          key_symbol;

        if (event.xkey.window != windows->image.id)
          break;
        /*
          Respond to a user key press.
        */
        (void) XLookupString((XKeyEvent *) &event.xkey,command,
          sizeof(command),&key_symbol,(XComposeStatus *) NULL);
        switch (key_symbol)
        {
          case XK_Escape:
          case XK_F20:
          {
            /*
              Prematurely exit.
            */
            state|=EscapeState;
            state|=ExitState;
            break;
          }
          case XK_F1:
          case XK_Help:
          {
            XSetFunction(display,windows->image.highlight_context,GXcopy);
            XTextViewWidget(display,resource_info,windows,False,
              "Help Viewer - Image Chopping",ImageChopHelp);
            XSetFunction(display,windows->image.highlight_context,GXinvert);
            break;
          }
          default:
          {
            XBell(display,0);
            break;
          }
        }
        break;
      }
      case MotionNotify:
      {
        /*
          Discard pending pointer motion events.
        */
        while (XCheckMaskEvent(display,PointerMotionMask,&event));
        x=event.xmotion.x;
        y=event.xmotion.y;
        /*
          Map and unmap Info widget as text cursor crosses its boundaries.
        */
        if (windows->info.mapped)
          {
            if ((x < (windows->info.x+windows->info.width)) &&
                (y < (windows->info.y+windows->info.height)))
              XWithdrawWindow(display,windows->info.id,windows->info.screen);
          }
        else
          if ((x > (windows->info.x+windows->info.width)) ||
              (y > (windows->info.y+windows->info.height)))
            XMapWindow(display,windows->info.id);
      }
    }
  } while (!(state & ExitState));
  XSelectInput(display,windows->image.id,windows->image.attributes.event_mask);
  XWithdrawWindow(display,windows->info.id,windows->info.screen);
  if (state & EscapeState)
    return(True);
  /*
    Draw line as pointer moves until the mouse button is released.
  */
  chop_info.width=0;
  chop_info.height=0;
  chop_info.x=0;
  chop_info.y=0;
  distance=0;
  XSetFunction(display,windows->image.highlight_context,GXinvert);
  state=DefaultState;
  do
  {
    if (distance > 9)
      {
        /*
          Display info and draw chopping line.
        */
        if (!windows->info.mapped)
          XMapWindow(display,windows->info.id);
        (void) sprintf(text," %ux%u%+d%+d",chop_info.width,chop_info.height,
          chop_info.x,chop_info.y);
        XInfoWidget(display,windows,text);
        XHighlightLine(display,windows->image.id,
          windows->image.highlight_context,&segment_info);
      }
    else
      if (windows->info.mapped)
        XWithdrawWindow(display,windows->info.id,windows->info.screen);
    /*
      Wait for next event.
    */
    XIfEvent(display,&event,XScreenEvent,(char *) windows);
    if (distance > 9)
      XHighlightLine(display,windows->image.id,
        windows->image.highlight_context,&segment_info);
    switch (event.type)
    {
      case ButtonPress:
      {
        segment_info.x2=event.xmotion.x;
        segment_info.y2=event.xmotion.y;
        break;
      }
      case ButtonRelease:
      {
        /*
          User has committed to chopping line.
        */
        segment_info.x2=event.xbutton.x;
        segment_info.y2=event.xbutton.y;
        state|=ExitState;
        break;
      }
      case Expose:
        break;
      case MotionNotify:
      {
        /*
          Discard pending button motion events.
        */
        while (XCheckMaskEvent(display,ButtonMotionMask,&event));
        segment_info.x2=event.xmotion.x;
        segment_info.y2=event.xmotion.y;
      }
      default:
        break;
    }
    /*
      Check boundary conditions.
    */
    if (segment_info.x2 < 0)
      segment_info.x2=0;
    else
      if (segment_info.x2 > windows->image.ximage->width)
        segment_info.x2=windows->image.ximage->width;
    if (segment_info.y2 < 0)
      segment_info.y2=0;
    else
      if (segment_info.y2 > windows->image.ximage->height)
        segment_info.y2=windows->image.ximage->height;
    distance=
      ((segment_info.x2-segment_info.x1)*(segment_info.x2-segment_info.x1))+
      ((segment_info.y2-segment_info.y1)*(segment_info.y2-segment_info.y1));
    /*
      Compute chopping geometry.
    */
    if (direction == HorizontalChopOp)
      {
        chop_info.width=segment_info.x2-segment_info.x1+1;
        chop_info.x=windows->image.x+segment_info.x1;
        chop_info.height=0;
        chop_info.y=0;
        if (segment_info.x1 > segment_info.x2)
          {
            chop_info.width=segment_info.x1-segment_info.x2+1;
            chop_info.x=windows->image.x+segment_info.x2;
          }
      }
    else
      {
        chop_info.width=0;
        chop_info.height=segment_info.y2-segment_info.y1+1;
        chop_info.x=0;
        chop_info.y=windows->image.y+segment_info.y1;
        if (segment_info.y1 > segment_info.y2)
          {
            chop_info.height=segment_info.y1-segment_info.y2+1;
            chop_info.y=windows->image.y+segment_info.y2;
          }
      }
  } while (!(state & ExitState));
  XSetFunction(display,windows->image.highlight_context,GXcopy);
  XWithdrawWindow(display,windows->info.id,windows->info.screen);
  if (distance <= 9)
    return(True);
  /*
    Image chopping is relative to image configuration.
  */
  (void) XMagickCommand(display,resource_info,windows,0,(KeySym) XK_A,image);
  XSetCursorState(display,windows,True);
  XCheckRefreshWindows(display,windows);
  windows->image.window_changes.width=
    windows->image.ximage->width-chop_info.width;
  windows->image.window_changes.height=
    windows->image.ximage->height-chop_info.height;
  x=0;
  y=0;
  width=(*image)->columns;
  height=(*image)->rows;
  if (windows->image.crop_geometry != (char *) NULL)
    (void) XParseGeometry(windows->image.crop_geometry,&x,&y,&width,&height);
  scale_factor=UpShift(width)/windows->image.ximage->width;
  chop_info.x+=x;
  chop_info.x=DownShift(chop_info.x*scale_factor);
  chop_info.width=DownShift(chop_info.width*scale_factor);
  scale_factor=UpShift(height)/windows->image.ximage->height;
  chop_info.y+=y;
  chop_info.y=DownShift(chop_info.y*scale_factor);
  chop_info.height=DownShift(chop_info.height*scale_factor);
  /*
    Chop image.
  */
  chop_image=ChopImage(*image,&chop_info);
  XSetCursorState(display,windows,False);
  if (chop_image == (Image *) NULL)
    return(False);
  DestroyImage(*image);
  *image=chop_image;
  /*
    Update image configuration.
  */
  XConfigureImageColormap(display,resource_info,windows,*image);
  (void) XConfigureImage(display,resource_info,windows,*image);
  return(True);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   X C o l o r E d i t I m a g e                                             %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Function XColorEditImage allows the user to interactively change
%  the color of one pixel for a DirectColor image or one colormap entry for
%  a PseudoClass image.  The floodfill algorithm is strongly based on a
%  similiar algorithm in "Graphics Gems" by Paul Heckbert.
%
%  The format of the XColorEditImage routine is:
%
%    XColorEditImage(display,resource_info,windows,image)
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
%    o image: Specifies a pointer to a Image structure; returned from
%      ReadImage.
%
*/

static void ColorFloodfill
(
    Image* image,
    int    x,
    int    y,
    XColor xcolor,
    int    delta
)
{
  int
    offset,
    skip,
    start,
    x1,
    x2;

  register RunlengthPacket
    *pixel;

  register XSegment
    *p;

  RunlengthPacket
    color,
    target;

  XSegment
    *segment_stack;

  /*
    Check boundary conditions.
  */
  if ((y < 0) || (y >= image->rows))
    return;
  if ((x < 0) || (x >= image->columns))
    return;
  target=image->pixels[y*image->columns+x];
  color.red=XDownScale(xcolor.red);
  color.green=XDownScale(xcolor.green);
  color.blue=XDownScale(xcolor.blue);
  if (ColorMatch(color,target,delta))
    return;
  /*
    Allocate segment stack.
  */
  segment_stack=(XSegment *) malloc(MaxStacksize*sizeof(XSegment));
  if (segment_stack == (XSegment *) NULL)
    {
      Warning("Unable to recolor image","Memory allocation failed");
      return;
    }
  /*
    Push initial segment on stack.
  */
  start=0;
  p=segment_stack;
  Push(y,x,x,1);
  Push(y+1,x,x,-1);
  while (p > segment_stack)
  {
    /*
      Pop segment off stack.
    */
    p--;
    x1=p->x1;
    x2=p->x2;
    offset=p->y2;
    y=p->y1+offset;
    /*
      Recolor neighboring pixels.
    */
    for (x=x1; x >= 0 ; x--)
    {
      pixel=image->pixels+(y*image->columns+x);
      if (!ColorMatch(*pixel,target,delta))
        break;
      pixel->red=color.red;
      pixel->green=color.green;
      pixel->blue=color.blue;
    }
    skip=x >= x1;
    if (!skip)
      {
        start=x+1;
        if (start < x1)
          Push(y,start,x1-1,-offset);
        x=x1+1;
      }
    do
    {
      if (!skip)
        {
          for ( ; x < image->columns; x++)
          {
            pixel=image->pixels+(y*image->columns+x);
            if (!ColorMatch(*pixel,target,delta))
              break;
            pixel->red=color.red;
            pixel->green=color.green;
            pixel->blue=color.blue;
          }
          Push(y,start,x-1,offset);
          if (x > (x2+1))
            Push(y,x2+1,x-1,-offset);
        }
      skip=False;
      for (x++; x <= x2 ; x++)
      {
        pixel=image->pixels+(y*image->columns+x);
        if (ColorMatch(*pixel,target,delta))
          break;
      }
      start=x;
    } while (x <= x2);
  }
  free((char *) segment_stack);
}

static unsigned int XColorEditImage
(
    Display*       display,
    XResourceInfo* resource_info,
    XWindows*      windows,
    Image**        image
)
{
#define ColorEditColorOp  0
#define ColorEditMethodOp  1
#define ColorEditDeltaOp  2
#define ColorEditUndoOp  3
#define ColorEditHelpOp  4
#define ColorEditDismissOp  5

  static char
    *ColorEditMenu[]=
    {
      "Pixel Color",
      "Method",
      "Delta",
      "Undo",
      "Help",
      "Dismiss",
      (char *) NULL
    };

  char
    command[MaxTextLength],
    text[MaxTextLength];

  Cursor
    cursor;

  int
    entry,
    id,
    x,
    x_offset,
    y,
    y_offset;

  register int
    i;

  register RunlengthPacket
    *p;

  static unsigned int
    delta = 0,
    method = PointMethodOp,
    pen_id = 0;

  unsigned int
    height,
    width;

  unsigned long
    state,
    x_factor,
    y_factor;

  XColor
    color;

  XEvent
    event;

  /*
    Map Command widget.
  */
  windows->command.name="Color Edit";
  windows->command.data=3;
  (void) XCommandWidget(display,windows,ColorEditMenu,(XEvent *) NULL);
  XMapRaised(display,windows->command.id);
  XClientMessage(display,windows->image.id,windows->im_protocols,
    windows->im_update_widget,CurrentTime);
  /*
    Make cursor.
  */
  cursor=XMakeCursor(display,windows->image.id,
    windows->image.map_info->colormap,resource_info->background_color,
    resource_info->foreground_color);
  XDefineCursor(display,windows->image.id,cursor);
  /*
    Track pointer until button 1 is pressed.
  */
  XQueryPosition(display,windows->image.id,&x,&y);
  XSelectInput(display,windows->image.id,windows->image.attributes.event_mask |
    PointerMotionMask);
  state=DefaultState;
  do
  {
    if (windows->info.mapped)
      {
        /*
          Display pointer position.
        */
        (void) sprintf(text," %+d%+d ",x-windows->image.x,y-windows->image.y);
        XInfoWidget(display,windows,text);
      }
    /*
      Wait for next event.
    */
    XIfEvent(display,&event,XScreenEvent,(char *) windows);
    if (event.xany.window == windows->command.id)
      {
        /*
          Select a command from the Command widget.
        */
        id=XCommandWidget(display,windows,ColorEditMenu,&event);
        if (id < 0)
          {
            XDefineCursor(display,windows->image.id,cursor);
            continue;
          }
        switch (id)
        {
          case ColorEditColorOp:
          {
            char
              *ColorMenu[MaxNumberPens];

            int
              pen_number;

            /*
              Initialize menu selections.
            */
            for (i=0; i < (MaxNumberPens-2); i++)
              ColorMenu[i]=resource_info->pen_colors[i];
            ColorMenu[MaxNumberPens-2]="Browser...";
            ColorMenu[MaxNumberPens-1]=(char *) NULL;
            /*
              Select a pen color from the pop-up menu.
            */
            pen_number=XMenuWidget(display,windows,ColorEditMenu[id],ColorMenu,
              command);
            if (pen_number < 0)
              break;
            if (pen_number == (MaxNumberPens-2))
              {
                static char
                  color_name[MaxTextLength] = "gray";

                /*
                  Select a pen color from a dialog.
                */
                resource_info->pen_colors[pen_number]=color_name;
                XColorBrowserWidget(display,windows,"Select",color_name);
                if (*color_name == '\0')
                  break;
              }
            /*
              Set pen color.
            */
            (void) XParseColor(display,windows->image.map_info->colormap,
              resource_info->pen_colors[pen_number],&color);
            XBestPixel(display,windows->image.map_info->colormap,
              (XColor *) NULL,(unsigned int) MaxColors,&color);
            windows->image.pixel_info->pen_colors[pen_number]=color;
            pen_id=pen_number;
            break;
          }
          case ColorEditMethodOp:
          {
            static char
              *MethodMenu[]=
              {
                "point",
                "replace",
                "floodfill",
                "reset",
                (char *) NULL,
              };

            /*
              Select a method from the pop-up menu.
            */
            entry=
              XMenuWidget(display,windows,ColorEditMenu[id],MethodMenu,command);
            if (entry >= 0)
              method=entry;
            break;
          }
          case ColorEditDeltaOp:
          {
            static char
              *DeltaMenu[]=
              {
                "0",
                "1",
                "2",
                "4",
                "8",
                "16",
                "32",
                (char *) NULL,
                (char *) NULL,
              },
              value[MaxTextLength] = "3";

            /*
              Select a delta value from the pop-up menu.
            */
            DeltaMenu[7]="Dialog...";
            entry=XMenuWidget(display,windows,ColorEditMenu[id],DeltaMenu,
              command);
            if (entry < 0)
              break;
            if (entry != 7)
              {
                delta=atoi(DeltaMenu[entry]);
                break;
              }
            (void) XDialogWidget(display,windows,"Ok","Enter delta value:",
              value);
            if (*value == '\0')
              break;
            delta=atoi(value);
            break;
          }
          case ColorEditUndoOp:
          {
            (void) XMagickCommand(display,resource_info,windows,0,
              (KeySym) XK_u,image);
            break;
          }
          case ColorEditHelpOp:
          {
            XTextViewWidget(display,resource_info,windows,False,
              "Help Viewer - Image Annotation",ImageColorEditHelp);
            break;
          }
          case ColorEditDismissOp:
          {
            /*
              Prematurely exit.
            */
            state|=EscapeState;
            state|=ExitState;
            break;
          }
        }
        XDefineCursor(display,windows->image.id,cursor);
        continue;
      }
    switch (event.type)
    {
      case ButtonPress:
      {
        if (event.xbutton.window == windows->pan.id)
          {
            XPanImage(display,windows,&event);
            XInfoWidget(display,windows,text);
            break;
          }
        /*
          Exit loop.
        */
        x=event.xbutton.x;
        y=event.xbutton.y;
        (void) XMagickCommand(display,resource_info,windows,0,
          (KeySym) XK_Select,image);
        state|=UpdateConfigurationState;
        break;
      }
      case ButtonRelease:
      {
        /*
          Update colormap information.
        */
        x=event.xbutton.x;
        y=event.xbutton.y;
        XConfigureImageColormap(display,resource_info,windows,*image);
        (void) XConfigureImage(display,resource_info,windows,*image);
        XInfoWidget(display,windows,text);
        XDefineCursor(display,windows->image.id,cursor);
        state&=(~UpdateConfigurationState);
        break;
      }
      case Expose:
        break;
      case KeyPress:
      {
        char
          command[MaxTextLength];

        KeySym
          key_symbol;

        if (event.xkey.window != windows->image.id)
          break;
        /*
          Respond to a user key press.
        */
        (void) XLookupString((XKeyEvent *) &event.xkey,command,sizeof(command),
          &key_symbol,(XComposeStatus *) NULL);
        switch (key_symbol)
        {
          case XK_Escape:
          case XK_F20:
          {
            /*
              Prematurely exit.
            */
            state|=ExitState;
            break;
          }
          case XK_F1:
          case XK_Help:
          {
            XTextViewWidget(display,resource_info,windows,False,
              "Help Viewer - Image Annotation",ImageColorEditHelp);
            break;
          }
          default:
          {
            XBell(display,0);
            break;
          }
        }
        break;
      }
      case MotionNotify:
      {
        /*
          Discard pending pointer motion events.
        */
        while (XCheckMaskEvent(display,PointerMotionMask,&event));
        x=event.xmotion.x;
        y=event.xmotion.y;
        /*
          Map and unmap Info widget as cursor crosses its boundaries.
        */
        if (windows->info.mapped)
          {
            if ((x < (windows->info.x+windows->info.width)) &&
                (y < (windows->info.y+windows->info.height)))
              XWithdrawWindow(display,windows->info.id,windows->info.screen);
          }
        else
          if ((x > (windows->info.x+windows->info.width)) ||
              (y > (windows->info.y+windows->info.height)))
            XMapWindow(display,windows->info.id);
        break;
      }
      default:
        break;
    }
    x_offset=x;
    y_offset=y;
    if (state & UpdateConfigurationState)
      {
        int
          x,
          y;

        /*
          Pixel edit is relative to image configuration.
        */
        XClearArea(display,windows->image.id,x_offset,y_offset,1,1,True);
        x=0;
        y=0;
        width=(*image)->columns;
        height=(*image)->rows;
        if (windows->image.crop_geometry != (char *) NULL)
          (void) XParseGeometry(windows->image.crop_geometry,&x,&y,&width,
            &height);
        x_factor=UpShift(width)/windows->image.ximage->width;
        x_offset=DownShift((windows->image.x+x_offset)*x_factor)+x;
        y_factor=UpShift(height)/windows->image.ximage->height;
        y_offset=DownShift((windows->image.y+y_offset)*y_factor)+y;
        color=windows->image.pixel_info->pen_colors[pen_id];
        if ((x_offset < 0) || (y_offset < 0))
          continue;
        if ((x_offset >= (*image)->columns) || (y_offset >= (*image)->rows))
          continue;
        XPutPixel(windows->image.ximage,x_offset,y_offset,color.pixel);
        switch (method)
        {
          case PointMethodOp:
          default:
          {
            /*
              Update color information using point algorithm.
            */
            (*image)->class=DirectClass;
            if (!UncompressImage(*image))
              break;
            p=(*image)->pixels+(y_offset*(*image)->columns+x_offset);
            p->red=XDownScale(color.red);
            p->green=XDownScale(color.green);
            p->blue=XDownScale(color.blue);
            break;
          }
          case ReplaceMethodOp:
          {
            RunlengthPacket
              target;

            /*
              Update color information using replace algorithm.
            */
            x=0;
            p=(*image)->pixels;
            for (i=0; i < (*image)->packets; i++)
            {
              x+=(p->length+1);
              if (x > (y_offset*(*image)->columns+x_offset))
                break;
              p++;
            }
            target=(*image)->pixels[i];
            if ((*image)->class == DirectClass)
              {
                p=(*image)->pixels;
                for (i=0; i < (*image)->packets; i++)
                {
                  if (ColorMatch(*p,target,delta))
                    {
                      p->red=XDownScale(color.red);
                      p->green=XDownScale(color.green);
                      p->blue=XDownScale(color.blue);
                    }
                  p++;
                }
              }
            else
              {
                for (i=0; i < (*image)->colors; i++)
                  if (ColorMatch((*image)->colormap[i],target,delta))
                    {
                      (*image)->colormap[i].red=XDownScale(color.red);
                      (*image)->colormap[i].green=XDownScale(color.green);
                      (*image)->colormap[i].blue=XDownScale(color.blue);
                    }
                SyncImage(*image);
              }
            break;
          }
          case FloodfillMethodOp:
          {
            /*
              Update color information using floodfill algorithm.
            */
            (*image)->class=DirectClass;
            if (!UncompressImage(*image))
              break;
            ColorFloodfill(*image,x_offset,y_offset,color,delta);
            break;
          }
          case ResetMethodOp:
          {
            /*
              Update color information using reset algorithm.
            */
            (*image)->class=DirectClass;
            p=(*image)->pixels;
            for (i=0; i < (*image)->packets; i++)
            {
              p->red=XDownScale(color.red);
              p->green=XDownScale(color.green);
              p->blue=XDownScale(color.blue);
              p++;
            }
            break;
          }
        }
        state&=(~UpdateConfigurationState);
      }
  } while (!(state & ExitState));
  XSelectInput(display,windows->image.id,windows->image.attributes.event_mask);
  XSetCursorState(display,windows,False);
  XFreeCursor(display,cursor);
  return(True);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   X C o m p o s i t e I m a g e                                             %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Function XCompositeImage requests an image name from the user, reads
%  the image and composites it with the X window image at a location the user
%  chooses with the pointer.
%
%  The format of the XCompositeImage routine is:
%
%    status=XCompositeImage(display,resource_info,windows,image)
%
%  A description of each parameter follows:
%
%    o status: Function XCompositeImage returns True if the image is
%      composited.  False is returned is there is a memory shortage or if the
%      image fails to be composited.
%
%    o display: Specifies a connection to an X server;  returned from
%      XOpenDisplay.
%
%    o resource_info: Specifies a pointer to a X11 XResourceInfo structure.
%
%    o windows: Specifies a pointer to a XWindows structure.
%
%    o image: Specifies a pointer to a Image structure; returned from
%      ReadImage.
%
*/
static unsigned int XCompositeImage
(
    Display*       display,
    XResourceInfo* resource_info,
    XWindows*      windows,
    Image*         image
)
{
#define CompositeModeOperatorsOp  0
#define CompositeModeBlendOp  1
#define CompositeModeHelpOp  2
#define CompositeModeDismissOp  3

  static char
    *CompositeModeMenu[]=
    {
      "Operators",
      "Blend",
      "Help",
      "Dismiss",
      (char *) NULL
    },
    filename[MaxTextLength] = "\0";

  char
    text[MaxTextLength];

  Cursor
    cursor;

  double
    blend;

  Image
    *composite_image;

  int
    id,
    x,
    y;

  RectangleInfo
    highlight_info,
    composite_info;

  static unsigned int
    operator = ReplaceCompositeOp;

  unsigned int
    height,
    width;

  unsigned long
    scale_factor,
    state;

  XEvent
    event;

  /*
    Request image file name from user.
  */
  XFileBrowserWidget(display,windows,"Composite",filename);
  if (*filename == '\0')
    return(True);
  /*
    Read image.
  */
  XSetCursorState(display,windows,True);
  XCheckRefreshWindows(display,windows);
  (void) strcpy(resource_info->image_info->filename,filename);
  composite_image=ReadImage(resource_info->image_info);
  XSetCursorState(display,windows,False);
  if (composite_image == (Image *) NULL)
    {
      XNoticeWidget(display,windows,"Unable to read image:",filename);
      return(False);
    }
  /*
    Map Command widget.
  */
  windows->command.name="Composite";
  windows->command.data=1;
  (void) XCommandWidget(display,windows,CompositeModeMenu,(XEvent *) NULL);
  XMapRaised(display,windows->command.id);
  XClientMessage(display,windows->image.id,windows->im_protocols,
    windows->im_update_widget,CurrentTime);
  /*
    Track pointer until button 1 is pressed.
  */
  XQueryPosition(display,windows->image.id,&x,&y);
  composite_info.x=windows->image.x+x;
  composite_info.y=windows->image.y+y;
  composite_info.width=0;
  composite_info.height=0;
  XSelectInput(display,windows->image.id,windows->image.attributes.event_mask |
    PointerMotionMask);
  cursor=XCreateFontCursor(display,XC_ul_angle);
  XSetFunction(display,windows->image.highlight_context,GXinvert);
  blend=0.0;
  state=DefaultState;
  do
  {
    if (windows->info.mapped)
      {
        /*
          Display pointer position.
        */
        (void) sprintf(text," %+d%+d ",composite_info.x,composite_info.y);
        XInfoWidget(display,windows,text);
      }
    highlight_info=composite_info;
    highlight_info.x=composite_info.x-windows->image.x;
    highlight_info.y=composite_info.y-windows->image.y;
    XHighlightRectangle(display,windows->image.id,
      windows->image.highlight_context,&highlight_info);
    /*
      Wait for next event.
    */
    XIfEvent(display,&event,XScreenEvent,(char *) windows);
    XHighlightRectangle(display,windows->image.id,
      windows->image.highlight_context,&highlight_info);
    if (event.xany.window == windows->command.id)
      {
        /*
          Select a command from the Command widget.
        */
        id=XCommandWidget(display,windows,CompositeModeMenu,&event);
        if (id < 0)
          continue;
        switch (id)
        {
          case CompositeModeOperatorsOp:
          {
            char
              command[MaxTextLength];

            static char
              *OperatorMenu[]=
              {
                "over",
                "in",
                "out",
                "atop",
                "xor",
                "plus",
                "minus",
                "add",
                "subtract",
                "difference",
                "replace",
                (char *) NULL,
              };

            /*
              Select a command from the pop-up menu.
            */
            operator=XMenuWidget(display,windows,CompositeModeMenu[id],
              OperatorMenu,command)+1;
            break;
          }
          case CompositeModeBlendOp:
          {
            static char
              factor[MaxTextLength] = "20.0";

            /*
              Blend the two images a given percent.
            */
            XSetFunction(display,windows->image.highlight_context,GXcopy);
            (void) XDialogWidget(display,windows,"Blend",
              "Enter the blend factor (0.0 - 99.9%):",factor);
            XSetFunction(display,windows->image.highlight_context,GXinvert);
            if (*factor == '\0')
              break;
            blend=atof(factor);
            break;
          }
          case CompositeModeHelpOp:
          {
            XSetFunction(display,windows->image.highlight_context,GXcopy);
            XTextViewWidget(display,resource_info,windows,False,
              "Help Viewer - Image Compositing",ImageCompositeHelp);
            XSetFunction(display,windows->image.highlight_context,GXinvert);
            break;
          }
          case CompositeModeDismissOp:
          {
            /*
              Prematurely exit.
            */
            state|=EscapeState;
            state|=ExitState;
            break;
          }
          default:
            break;
        }
        continue;
      }
    switch (event.type)
    {
      case ButtonPress:
      {
        if (resource_info->debug)
          (void) fprintf(stderr,"Button Press: 0x%lx %u +%d+%d\n",
            event.xbutton.window,event.xbutton.button,event.xbutton.x,
            event.xbutton.y);
        if (event.xbutton.window == windows->pan.id)
          {
            XPanImage(display,windows,&event);
            XInfoWidget(display,windows,text);
            break;
          }
        /*
          Change cursor.
        */
        composite_info.width=composite_image->columns;
        composite_info.height=composite_image->rows;
        XDefineCursor(display,windows->image.id,cursor);
        composite_info.x=windows->image.x+event.xbutton.x;
        composite_info.y=windows->image.y+event.xbutton.y;
        break;
      }
      case ButtonRelease:
      {
        if (resource_info->debug)
          (void) fprintf(stderr,"Button Release: 0x%lx %u +%d+%d\n",
            event.xbutton.window,event.xbutton.button,event.xbutton.x,
            event.xbutton.y);
        if ((composite_info.width != 0) && (composite_info.height != 0))
          {
            /*
              User has selected the location of the composite image.
            */
            composite_info.x=windows->image.x+event.xbutton.x;
            composite_info.y=windows->image.y+event.xbutton.y;
            state|=ExitState;
          }
        break;
      }
      case Expose:
        break;
      case KeyPress:
      {
        char
          command[MaxTextLength];

        KeySym
          key_symbol;

        int
          length;

        if (event.xkey.window != windows->image.id)
          break;
        /*
          Respond to a user key press.
        */
        length=XLookupString((XKeyEvent *) &event.xkey,command,sizeof(command),
          &key_symbol,(XComposeStatus *) NULL);
        *(command+length)='\0';
        if (resource_info->debug)
          (void) fprintf(stderr,"Key press: 0x%lx (%s)\n",key_symbol,command);
        switch (key_symbol)
        {
          case XK_Escape:
          case XK_F20:
          {
            /*
              Prematurely exit.
            */
            DestroyImage(composite_image);
            state|=EscapeState;
            state|=ExitState;
            break;
          }
          case XK_F1:
          case XK_Help:
          {
            XSetFunction(display,windows->image.highlight_context,GXcopy);
            XTextViewWidget(display,resource_info,windows,False,
              "Help Viewer - Image Compositing",ImageCompositeHelp);
            XSetFunction(display,windows->image.highlight_context,GXinvert);
            break;
          }
          default:
          {
            XBell(display,0);
            break;
          }
        }
        break;
      }
      case MotionNotify:
      {
        /*
          Discard pending pointer motion events.
        */
        while (XCheckMaskEvent(display,PointerMotionMask,&event));
        x=event.xmotion.x;
        y=event.xmotion.y;
        /*
          Map and unmap Info widget as text cursor crosses its boundaries.
        */
        if (windows->info.mapped)
          {
            if ((x < (windows->info.x+windows->info.width)) &&
                (y < (windows->info.y+windows->info.height)))
              XWithdrawWindow(display,windows->info.id,windows->info.screen);
          }
        else
          if ((x > (windows->info.x+windows->info.width)) ||
              (y > (windows->info.y+windows->info.height)))
            XMapWindow(display,windows->info.id);
        composite_info.x=windows->image.x+x;
        composite_info.y=windows->image.y+y;
        break;
      }
      default:
      {
        if (resource_info->debug)
          (void) fprintf(stderr,"Event type: %d\n",event.type);
        break;
      }
    }
  } while (!(state & ExitState));
  XSetFunction(display,windows->image.highlight_context,GXcopy);
  XSelectInput(display,windows->image.id,windows->image.attributes.event_mask);
  XSetCursorState(display,windows,False);
  XFreeCursor(display,cursor);
  if (state & EscapeState)
    return(True);
  /*
    Image compositing is relative to image configuration.
  */
  XSetCursorState(display,windows,True);
  XCheckRefreshWindows(display,windows);
  x=0;
  y=0;
  width=image->columns;
  height=image->rows;
  if (windows->image.crop_geometry != (char *) NULL)
    (void) XParseGeometry(windows->image.crop_geometry,&x,&y,&width,&height);
  scale_factor=UpShift(width)/windows->image.ximage->width;
  composite_info.x+=x;
  composite_info.x=DownShift(composite_info.x*scale_factor);
  composite_info.width=DownShift(composite_info.width*scale_factor);
  scale_factor=UpShift(height)/windows->image.ximage->height;
  composite_info.y+=y;
  composite_info.y=DownShift(composite_info.y*scale_factor);
  composite_info.height=DownShift(composite_info.height*scale_factor);
  if ((composite_info.width != composite_image->columns) ||
      (composite_info.height != composite_image->rows))
    {
      Image
        *zoomed_image;

      /*
        Scale composite image.
      */
      zoomed_image=ZoomImage(composite_image,composite_info.width,
        composite_info.height,MitchellFilter);
      DestroyImage(composite_image);
      if (zoomed_image == (Image *) NULL)
        {
          XSetCursorState(display,windows,False);
          return(False);
        }
      composite_image=zoomed_image;
    }
  if (blend != 0.0)
    {
      register int
        i;

      register RunlengthPacket
        *p;

      unsigned short
        index;

      /*
        Create mattes for blending.
      */
      index=(unsigned short)(((int) DownScale(MaxRGB)*blend)/100);
      composite_image->class=DirectClass;
      composite_image->matte=True;
      p=composite_image->pixels;
      for (i=0; i < composite_image->packets; i++)
      {
        p->index=index;
        p++;
      }
      index=(unsigned short)((int) DownScale(MaxRGB)-((int) DownScale(MaxRGB)*blend)/100);
      image->class=DirectClass;
      image->matte=True;
      p=image->pixels;
      for (i=0; i < image->packets; i++)
      {
        p->index=index;
        p++;
      }
      operator=BlendCompositeOp;
    }
  /*
    Composite image with X Image window.
  */
  CompositeImage(image,operator,composite_image,composite_info.x,
    composite_info.y);
  DestroyImage(composite_image);
  XSetCursorState(display,windows,False);
  /*
    Update image configuration.
  */
  XConfigureImageColormap(display,resource_info,windows,image);
  (void) XConfigureImage(display,resource_info,windows,image);
  return(True);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   X C o n f i g u r e I m a g e C o l o r m a p                             %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Function XConfigureImageColormap creates a new X colormap.
%
%  The format of the XConfigureImageColormap routine is:
%
%    XConfigureImageColormap(display,resource_info,windows,image)
%
%  A description of each parameter follows:
%
%    o display: Specifies a connection to an X server; returned from
%      XOpenDisplay.
%
%    o resource_info: Specifies a pointer to a X11 XResourceInfo structure.
%
%    o windows: Specifies a pointer to a XWindows structure.
%
%    o image: Specifies a pointer to a Image structure;  returned from
%      ReadImage.
%
%
*/
static void XConfigureImageColormap
(
    Display*       display,
    XResourceInfo* resource_info,
    XWindows*      windows,
    Image*         image
)
{
  Colormap
    colormap;

  /*
    Make standard colormap.
  */
  XSetCursorState(display,windows,True);
  XCheckRefreshWindows(display,windows);
  if (image->packets == (image->columns*image->rows))
    CompressImage(image);
  XMakeStandardColormap(display,windows->image.visual_info,resource_info,
    image,windows->image.map_info,windows->image.pixel_info);
  colormap=windows->image.map_info->colormap;
  XSetWindowColormap(display,windows->image.id,colormap);
  XSetWindowColormap(display,windows->command.id,colormap);
  XSetWindowColormap(display,windows->widget.id,colormap);
  if (windows->magnify.mapped)
    XSetWindowColormap(display,windows->magnify.id,colormap);
  if (windows->pan.mapped)
    XSetWindowColormap(display,windows->pan.id,colormap);
  XSetCursorState(display,windows,False);
  XClientMessage(display,windows->image.id,windows->im_protocols,
    windows->im_update_colormap,CurrentTime);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   X C o n f i g u r e I m a g e                                             %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Function XConfigureImage creates a new X image.  It also notifies the
%  window manager of the new image size and configures the transient widows.
%
%  The format of the XConfigureImage routine is:
%
%    status=XConfigureImage(display,resource_info,windows,image)
%
%  A description of each parameter follows:
%
%    o status: Function XConfigureImage returns True if the window is
%      resized.  False is returned is there is a memory shortage or if the
%      window fails to resize.
%
%    o display: Specifies a connection to an X server; returned from
%      XOpenDisplay.
%
%    o resource_info: Specifies a pointer to a X11 XResourceInfo structure.
%
%    o windows: Specifies a pointer to a XWindows structure.
%
%    o image: Specifies a pointer to a Image structure;  returned from
%      ReadImage.
%
%
*/
static unsigned int XConfigureImage
(
    Display*       display,
    XResourceInfo* resource_info,
    XWindows*      windows,
    Image*         image
)
{
  /* Kobus */
  extern int fs_image_modified;
  /* End Kobus */

  unsigned int
    height,
    mask,
    status,
    width,
    x_factor,
    y_factor;

  XSizeHints
    *size_hints;

  XWindowChanges
    window_changes;


  /* Kobus */
  fs_image_modified = True;
  /* End Kobus */

  /*
    Dismiss if window dimensions are zero.
  */
  width=windows->image.window_changes.width;
  height=windows->image.window_changes.height;
  if (resource_info->debug)
    (void) fprintf(stderr,"Configure Image: %dx%d=>%ux%u\n",
      windows->image.ximage->width,windows->image.ximage->height,width,height);
  if ((width*height) == 0)
    return(True);

  /*
    Resize image to fit Image window dimensions.
  */
  XSetCursorState(display,windows,True);
  XFlush(display);
  x_factor=UpShift(width)/windows->image.ximage->width;
  y_factor=UpShift(height)/windows->image.ximage->height;
  status=XMakeImage(display,resource_info,&windows->image,image,width,height);
  if (resource_info->use_pixmap)
    (void) XMakePixmap(display,resource_info,&windows->image);
  if (status == False)
    XNoticeWidget(display,windows,"Unable to configure X image:",
      windows->image.name);
  /*
    Notify window manager of the new configuration.
  */
  if (width > XDisplayWidth(display,windows->image.screen)) {
    width=XDisplayWidth(display,windows->image.screen);
   }

  window_changes.width=width;
  if (height > XDisplayHeight(display,windows->image.screen))
    height=XDisplayHeight(display,windows->image.screen);
  window_changes.height=height;
  mask=CWWidth | CWHeight;
  if (resource_info->backdrop)
    {
      mask|=CWX | CWY;
      window_changes.x=
        (XDisplayWidth(display,windows->image.screen) >> 1)-(width >> 1);
      window_changes.y=
        (XDisplayHeight(display,windows->image.screen) >> 1)-(height >> 1);
    }
  XReconfigureWMWindow(display,windows->image.id,windows->image.screen,mask,
    &window_changes);


  if (image->matte)
    XClearWindow(display,windows->image.id);
  if ((x_factor == UpShift(1)) || (y_factor == UpShift(1)))
    XRefreshWindow(display,&windows->image,(XEvent *) NULL);
  /*
    Update Magnify window configuration.
  */
  windows->magnify.x=DownShift(x_factor*windows->magnify.x);
  windows->magnify.y=DownShift(y_factor*windows->magnify.y);
  if (windows->magnify.mapped)
    XMakeMagnifyImage(display,windows);
  /*
    Update pan window configuration.
  */
  windows->image.x=DownShift(x_factor*windows->image.x);
  windows->image.y=DownShift(y_factor*windows->image.y);
  windows->pan.crop_geometry=windows->image.crop_geometry;
  XBestIconSize(display,&windows->pan,image);
  while ((windows->pan.width < MinPanSize) &&
         (windows->pan.height < MinPanSize))
  {
    windows->pan.width<<=1;
    windows->pan.height<<=1;
  }
  if (windows->pan.geometry != (char *) NULL)
    ParseImageGeometry(windows->pan.geometry,&windows->pan.width,
      &windows->pan.height);
  window_changes.width=windows->pan.width;
  window_changes.height=windows->pan.height;
  size_hints=XAllocSizeHints();
  if (size_hints != (XSizeHints *) NULL)
    {
      /*
        Set new size hints.
      */
      size_hints->flags=PSize | PMinSize | PMaxSize;
      size_hints->width=window_changes.width;
      size_hints->height=window_changes.height;
      size_hints->min_width=size_hints->width;
      size_hints->min_height=size_hints->height;
      size_hints->max_width=size_hints->width;
      size_hints->max_height=size_hints->height;
      XSetNormalHints(display,windows->pan.id,size_hints);
      XFree((void *) size_hints);
    }
  XReconfigureWMWindow(display,windows->pan.id,windows->pan.screen,CWWidth |
    CWHeight,&window_changes);
  if (windows->pan.mapped)
    XMakePanImage(display,resource_info,windows,image);
  /*
    Update icon window configuration.
  */
  windows->icon.crop_geometry=windows->image.crop_geometry;
  XBestIconSize(display,&windows->icon,image);
  window_changes.width=windows->icon.width;
  window_changes.height=windows->icon.height;
  XReconfigureWMWindow(display,windows->icon.id,windows->icon.screen,
    CWWidth | CWHeight,&window_changes);
  XSetCursorState(display,windows,False);
  return(status);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   X C r o p I m a g e                                                       %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Function XCropImage allows the user to select a region of the image and
%  crop, copy, or cut it.  For copy or cut, the image can subsequently be
%  composited onto the image with XPasteImage.
%
%  The format of the XCropImage routine is:
%
%    status=XCropImage(display,resource_info,windows,image,mode)
%
%  A description of each parameter follows:
%
%    o status: Function XCropImage returns True if the image is
%      copyped.  False is returned is there is a memory shortage or if the
%      image fails to be copyped.
%
%    o display: Specifies a connection to an X server; returned from
%      XOpenDisplay.
%
%    o resource_info: Specifies a pointer to a X11 XResourceInfo structure.
%
%    o windows: Specifies a pointer to a XWindows structure.
%
%    o image: Specifies a pointer to a Image structure; returned from
%      ReadImage.
%
%    o mode: This unsigned value specified whether the image should be
%      cropped, copied, or cut.
%
%
*/
static unsigned int XCropImage
(
    Display*       display,
    XResourceInfo* resource_info,
    XWindows*      windows,
    Image*         image,
    unsigned int   mode
)
{
#define CropModeHelpOp  0
#define CropModeDismissOp  1
#define RectifyModeCopyOp  0
#define RectifyModeHelpOp  1
#define RectifyModeDismissOp  2

  static char
    *CropModeMenu[]=
    {
      "Help",
      "Dismiss",
      (char *) NULL
    },
    *RectifyModeMenu[]=
    {
      "Crop",
      "Help",
      "Dismiss",
      (char *) NULL
    };

  char
    command[MaxTextLength],
    text[MaxTextLength];

  Cursor
    cursor;

  int
    id,
    x,
    y;

  KeySym
    key_symbol;

  Image
    *crop_image;

  RectangleInfo
    crop_info,
    highlight_info;

  register RunlengthPacket
    *p;

  unsigned int
    height,
    width;

  unsigned long
    scale_factor,
    state;

  XEvent
    event;

  /*
    Map Command widget.
  */
  switch (mode)
  {
    case CopyMode:
    {
      windows->command.name="Copy";
      break;
    }
    case CropMode:
    {
      windows->command.name="Crop";
      break;
    }
    case CutMode:
    {
      windows->command.name="Cut";
      break;
    }
  }
  RectifyModeMenu[0]=windows->command.name;
  windows->command.data=0;
  (void) XCommandWidget(display,windows,CropModeMenu,(XEvent *) NULL);
  XMapRaised(display,windows->command.id);
  XClientMessage(display,windows->image.id,windows->im_protocols,
    windows->im_update_widget,CurrentTime);
  /*
    Track pointer until button 1 is pressed.
  */
  XQueryPosition(display,windows->image.id,&x,&y);
  crop_info.x=windows->image.x+x;
  crop_info.y=windows->image.y+y;
  crop_info.width=0;
  crop_info.height=0;
  cursor=XCreateFontCursor(display,XC_fleur);
  XSelectInput(display,windows->image.id,windows->image.attributes.event_mask |
    PointerMotionMask);
  state=DefaultState;
  do
  {
    if (windows->info.mapped)
      {
        /*
          Display pointer position.
        */
        (void) sprintf(text," %+d%+d ",crop_info.x,crop_info.y);
        XInfoWidget(display,windows,text);
      }
    /*
      Wait for next event.
    */
    XIfEvent(display,&event,XScreenEvent,(char *) windows);
    if (event.xany.window == windows->command.id)
      {
        /*
          Select a command from the Command widget.
        */
        id=XCommandWidget(display,windows,CropModeMenu,&event);
        if (id < 0)
          continue;
        switch (id)
        {
          case CropModeHelpOp:
          {
            switch (mode)
            {
              case CopyMode:
              {
                XTextViewWidget(display,resource_info,windows,False,
                  "Help Viewer - Image Copying",ImageCopyHelp);
                break;
              }
              case CropMode:
              {
                XTextViewWidget(display,resource_info,windows,False,
                  "Help Viewer - Image Cropping",ImageCropHelp);
                break;
              }
              case CutMode:
              {
                XTextViewWidget(display,resource_info,windows,False,
                  "Help Viewer - Image Cutting",ImageCutHelp);
                break;
              }
            }
            break;
          }
          case CropModeDismissOp:
          {
            /*
              Prematurely exit.
            */
            state|=EscapeState;
            state|=ExitState;
            break;
          }
          default:
            break;
        }
        continue;
      }
    switch (event.type)
    {
      case ButtonPress:
      {
        if (event.xbutton.window == windows->pan.id)
          {
            XPanImage(display,windows,&event);
            XInfoWidget(display,windows,text);
            break;
          }
        if (event.xbutton.button == Button1)
          {
            /*
              Note first corner of cropping rectangle-- exit loop.
            */
            XDefineCursor(display,windows->image.id,cursor);
            crop_info.x=windows->image.x+event.xbutton.x;
            crop_info.y=windows->image.y+event.xbutton.y;
            state|=ExitState;
            break;
          }
        break;
      }
      case Expose:
        break;
      case KeyPress:
      {
        if (event.xkey.window != windows->image.id)
          break;
        /*
          Respond to a user key press.
        */
        (void) XLookupString((XKeyEvent *) &event.xkey,command,sizeof(command),
          &key_symbol,(XComposeStatus *) NULL);
        switch (key_symbol)
        {
          case XK_Escape:
          case XK_F20:
          {
            /*
              Prematurely exit.
            */
            state|=EscapeState;
            state|=ExitState;
            break;
          }
          case XK_F1:
          case XK_Help:
          {
            switch (mode)
            {
              case CopyMode:
              {
                XTextViewWidget(display,resource_info,windows,False,
                  "Help Viewer - Image Copying",ImageCopyHelp);
                break;
              }
              case CropMode:
              {
                XTextViewWidget(display,resource_info,windows,False,
                  "Help Viewer - Image Cropping",ImageCropHelp);
                break;
              }
              case CutMode:
              {
                XTextViewWidget(display,resource_info,windows,False,
                  "Help Viewer - Image Cutting",ImageCutHelp);
                break;
              }
            }
            break;
          }
          default:
          {
            XBell(display,0);
            break;
          }
        }
        break;
      }
      case MotionNotify:
      {
        /*
          Discard pending pointer motion events.
        */
        while (XCheckMaskEvent(display,PointerMotionMask,&event));
        x=event.xmotion.x;
        y=event.xmotion.y;
        /*
          Map and unmap Info widget as text cursor crosses its boundaries.
        */
        if (windows->info.mapped)
          {
            if ((x < (windows->info.x+windows->info.width)) &&
                (y < (windows->info.y+windows->info.height)))
              XWithdrawWindow(display,windows->info.id,windows->info.screen);
          }
        else
          if ((x > (windows->info.x+windows->info.width)) ||
              (y > (windows->info.y+windows->info.height)))
            XMapWindow(display,windows->info.id);
        crop_info.x=windows->image.x+x;
        crop_info.y=windows->image.y+y;
        break;
      }
      default:
        break;
    }
  } while (!(state & ExitState));
  XSelectInput(display,windows->image.id,windows->image.attributes.event_mask);
  if (state & EscapeState)
    {
      /*
        User want to exit without croping.
      */
      XWithdrawWindow(display,windows->info.id,windows->info.screen);
      XFreeCursor(display,cursor);
      return(True);
    }
  XSetFunction(display,windows->image.highlight_context,GXinvert);
  do
  {
    /*
      Size rectangle as pointer moves until the mouse button is released.
    */
    x=crop_info.x;
    y=crop_info.y;
    crop_info.width=0;
    crop_info.height=0;
    state=DefaultState;
    XSelectInput(display,windows->image.id,
      windows->image.attributes.event_mask | PointerMotionMask);
    do
    {
      highlight_info=crop_info;
      highlight_info.x=crop_info.x-windows->image.x;
      highlight_info.y=crop_info.y-windows->image.y;
      if ((highlight_info.width > 3) && (highlight_info.height > 3))
        {
          /*
            Display info and draw cropping rectangle.
          */
          if (!windows->info.mapped)
            XMapWindow(display,windows->info.id);
          (void) sprintf(text," %ux%u%+d%+d",crop_info.width,crop_info.height,
            crop_info.x,crop_info.y);
          XInfoWidget(display,windows,text);
          XHighlightRectangle(display,windows->image.id,
            windows->image.highlight_context,&highlight_info);
        }
      else
        if (windows->info.mapped)
          XWithdrawWindow(display,windows->info.id,windows->info.screen);
      /*
        Wait for next event.
      */
      XIfEvent(display,&event,XScreenEvent,(char *) windows);
      if ((highlight_info.width > 3) && (highlight_info.height > 3))
        XHighlightRectangle(display,windows->image.id,
          windows->image.highlight_context,&highlight_info);
      switch (event.type)
      {
        case ButtonPress:
        {
          if (event.xbutton.window == windows->pan.id)
            {
              XPanImage(display,windows,&event);
              XInfoWidget(display,windows,text);
              break;
            }
          crop_info.x=windows->image.x+event.xbutton.x;
          crop_info.y=windows->image.y+event.xbutton.y;
          break;
        }
        case ButtonRelease:
        {
          /*
            User has committed to croping rectangle.
          */
          crop_info.x=windows->image.x+event.xbutton.x;
          crop_info.y=windows->image.y+event.xbutton.y;
          XSetCursorState(display,windows,False);
          state|=ExitState;
          if (strcmp(windows->command.name,"Rectify") == 0)
            break;
          windows->command.name="Rectify";
          windows->command.data=0;
          (void) XCommandWidget(display,windows,RectifyModeMenu,
            (XEvent *) NULL);
          break;
        }
        case Expose:
          break;
        case MotionNotify:
        {
          /*
            Discard pending button motion events.
          */
          while (XCheckMaskEvent(display,ButtonMotionMask,&event));
          crop_info.x=windows->image.x+event.xmotion.x;
          crop_info.y=windows->image.y+event.xmotion.y;
        }
        default:
          break;
      }
      if (((crop_info.x != x) && (crop_info.y != y)) || (state & ExitState))
        {
          /*
            Check boundary conditions.
          */
          if (crop_info.x < 0)
            crop_info.x=0;
          else
            if (crop_info.x > windows->image.ximage->width)
              crop_info.x=windows->image.ximage->width;
          if (crop_info.x < x)
            crop_info.width=(unsigned int) (x-crop_info.x);
          else
            {
              crop_info.width=(unsigned int) (crop_info.x-x);
              crop_info.x=x;
            }
          if (crop_info.y < 0)
            crop_info.y=0;
          else
            if (crop_info.y > windows->image.ximage->height)
              crop_info.y=windows->image.ximage->height;
          if (crop_info.y < y)
            crop_info.height=(unsigned int) (y-crop_info.y);
          else
            {
              crop_info.height=(unsigned int) (crop_info.y-y);
              crop_info.y=y;
            }
        }
    } while (!(state & ExitState));
    XSelectInput(display,windows->image.id,
      windows->image.attributes.event_mask);
    /*
      Wait for user to grab a corner of the rectangle or press return.
    */
    state=DefaultState;
    do
    {
      if (windows->info.mapped)
        {
          /*
            Display pointer position.
          */
          (void) sprintf(text," %ux%u%+d%+d",crop_info.width,crop_info.height,
            crop_info.x,crop_info.y);
          XInfoWidget(display,windows,text);
        }
      highlight_info=crop_info;
      highlight_info.x=crop_info.x-windows->image.x;
      highlight_info.y=crop_info.y-windows->image.y;
      if ((highlight_info.width <= 3) || (highlight_info.height <= 3))
        {
          state|=EscapeState;
          state|=ExitState;
          break;
        }
      XHighlightRectangle(display,windows->image.id,
        windows->image.highlight_context,&highlight_info);
      XIfEvent(display,&event,XScreenEvent,(char *) windows);
      if (event.xany.window == windows->command.id)
        {
          /*
            Select a command from the Command widget.
          */
          XSetFunction(display,windows->image.highlight_context,GXcopy);
          id=XCommandWidget(display,windows,RectifyModeMenu,&event);
          XSetFunction(display,windows->image.highlight_context,GXinvert);
          XHighlightRectangle(display,windows->image.id,
            windows->image.highlight_context,&highlight_info);
          if (id >= 0)
            switch (id)
            {
              case RectifyModeCopyOp:
              {
                state|=ExitState;
                break;
              }
              case RectifyModeHelpOp:
              {
                XSetFunction(display,windows->image.highlight_context,GXcopy);
                switch (mode)
                {
                  case CopyMode:
                  {
                    XTextViewWidget(display,resource_info,windows,False,
                      "Help Viewer - Image Copying",ImageCopyHelp);
                    break;
                  }
                  case CropMode:
                  {
                    XTextViewWidget(display,resource_info,windows,False,
                      "Help Viewer - Image Cropping",ImageCropHelp);
                    break;
                  }
                  case CutMode:
                  {
                    XTextViewWidget(display,resource_info,windows,False,
                      "Help Viewer - Image Cutting",ImageCutHelp);
                    break;
                  }
                }
                XSetFunction(display,windows->image.highlight_context,GXinvert);
                break;
              }
              case RectifyModeDismissOp:
              {
                /*
                  Prematurely exit.
                */
                state|=EscapeState;
                state|=ExitState;
                break;
              }
              default:
                break;
            }
          continue;
        }
      XHighlightRectangle(display,windows->image.id,
        windows->image.highlight_context,&highlight_info);
      switch (event.type)
      {
        case ButtonPress:
        {
          if (event.xbutton.window == windows->pan.id)
            {
              XPanImage(display,windows,&event);
              XInfoWidget(display,windows,text);
              break;
            }
          if (event.xbutton.window != windows->image.id)
            break;
          x=windows->image.x+event.xbutton.x;
          y=windows->image.y+event.xbutton.y;
          if ((x < (crop_info.x+RoiDelta)) && (x > (crop_info.x-RoiDelta)) &&
              (y < (crop_info.y+RoiDelta)) && (y > (crop_info.y-RoiDelta)))
            {
              crop_info.x=crop_info.x+crop_info.width;
              crop_info.y=crop_info.y+crop_info.height;
              state|=UpdateConfigurationState;
              break;
            }
          if ((x < (crop_info.x+RoiDelta)) && (x > (crop_info.x-RoiDelta)) &&
              (y < (crop_info.y+crop_info.height+RoiDelta)) &&
              (y > (crop_info.y+crop_info.height-RoiDelta)))
            {
              crop_info.x=crop_info.x+crop_info.width;
              state|=UpdateConfigurationState;
              break;
            }
          if ((x < (crop_info.x+crop_info.width+RoiDelta)) &&
              (x > (crop_info.x+crop_info.width-RoiDelta)) &&
              (y < (crop_info.y+RoiDelta)) && (y > (crop_info.y-RoiDelta)))
            {
              crop_info.y=crop_info.y+crop_info.height;
              state|=UpdateConfigurationState;
              break;
            }
          if ((x < (crop_info.x+crop_info.width+RoiDelta)) &&
              (x > (crop_info.x+crop_info.width-RoiDelta)) &&
              (y < (crop_info.y+crop_info.height+RoiDelta)) &&
              (y > (crop_info.y+crop_info.height-RoiDelta)))
            {
              state|=UpdateConfigurationState;
              break;
            }
        }
        case ButtonRelease:
          break;
        case Expose:
        {
          if (event.xexpose.window == windows->image.id)
            if (event.xexpose.count == 0)
              {
                event.xexpose.x=highlight_info.x;
                event.xexpose.y=highlight_info.y;
                event.xexpose.width=highlight_info.width;
                event.xexpose.height=highlight_info.height;
                XRefreshWindow(display,&windows->image,&event);
              }
          if (event.xexpose.window == windows->info.id)
            if (event.xexpose.count == 0)
              XInfoWidget(display,windows,text);
          break;
        }
        case KeyPress:
        {
          if (event.xkey.window != windows->image.id)
            break;
          /*
            Respond to a user key press.
          */
          (void) XLookupString((XKeyEvent *) &event.xkey,command,
            sizeof(command),&key_symbol,(XComposeStatus *) NULL);
          switch (key_symbol)
          {
            case XK_Escape:
            case XK_F20:
              state|=EscapeState;
            case XK_Return:
            {
              state|=ExitState;
              break;
            }
            case XK_F1:
            case XK_Help:
            {
              XSetFunction(display,windows->image.highlight_context,GXcopy);
              switch (mode)
              {
                case CopyMode:
                {
                  XTextViewWidget(display,resource_info,windows,False,
                    "Help Viewer - Image Copying",ImageCopyHelp);
                  break;
                }
                case CropMode:
                {
                  XTextViewWidget(display,resource_info,windows,False,
                    "Help Viewer - Image Cropping",ImageCropHelp);
                  break;
                }
                case CutMode:
                {
                  XTextViewWidget(display,resource_info,windows,False,
                    "Help Viewer - Image Cutting",ImageCutHelp);
                  break;
                }
              }
              XSetFunction(display,windows->image.highlight_context,GXinvert);
              break;
            }
            default:
            {
              XBell(display,0);
              break;
            }
          }
          break;
        }
        case KeyRelease:
          break;
        case MotionNotify:
        {
          /*
            Discard pending pointer motion events.
          */
          while (XCheckMaskEvent(display,ButtonMotionMask,&event));
          x=event.xmotion.x;
          y=event.xmotion.y;
          /*
            Map and unmap Info widget as text cursor crosses its boundaries.
          */
          if (windows->info.mapped)
            {
              if ((x < (windows->info.x+windows->info.width)) &&
                  (y < (windows->info.y+windows->info.height)))
                XWithdrawWindow(display,windows->info.id,windows->info.screen);
            }
          else
            if ((x > (windows->info.x+windows->info.width)) ||
                (y > (windows->info.y+windows->info.height)))
              XMapWindow(display,windows->info.id);
          break;
        }
        default:
          break;
      }
      if (state & UpdateConfigurationState)
        {
          XPutBackEvent(display,&event);
          XDefineCursor(display,windows->image.id,cursor);
          break;
        }
    } while (!(state & ExitState));
  } while (!(state & ExitState));
  XSetFunction(display,windows->image.highlight_context,GXcopy);
  XSetCursorState(display,windows,False);
  if (state & EscapeState)
    return(True);
  if (mode == CropMode)
    if ((crop_info.width != windows->image.ximage->width) ||
        (crop_info.height != windows->image.ximage->height))
      {
        /*
          Reconfigure Image window as defined by cropping rectangle.
        */
        XSetCropGeometry(display,windows,&crop_info,image);
        windows->image.window_changes.width=crop_info.width;
        windows->image.window_changes.height=crop_info.height;
        (void) XConfigureImage(display,resource_info,windows,image);
        return(True);
      }
  /*
    Copy image before applying image transforms.
  */
  XSetCursorState(display,windows,True);
  XCheckRefreshWindows(display,windows);
  x=0;
  y=0;
  width=image->columns;
  height=image->rows;
  if (windows->image.crop_geometry != (char *) NULL)
    (void) XParseGeometry(windows->image.crop_geometry,&x,&y,&width,&height);
  scale_factor=UpShift(width)/windows->image.ximage->width;
  crop_info.x+=x;
  crop_info.x=DownShift(crop_info.x*scale_factor);
  crop_info.width=DownShift(crop_info.width*scale_factor);
  scale_factor=UpShift(height)/windows->image.ximage->height;
  crop_info.y+=y;
  crop_info.y=DownShift(crop_info.y*scale_factor);
  crop_info.height=DownShift(crop_info.height*scale_factor);
  crop_image=CropImage(image,&crop_info);
  XSetCursorState(display,windows,False);
  if (crop_image == (Image *) NULL)
    return(False);
  if (copy_image != (Image *) NULL)
    DestroyImage(copy_image);
  copy_image=crop_image;
  if (mode == CopyMode)
    {
      (void) XConfigureImage(display,resource_info,windows,image);
      return(True);
    }
  /*
    Cut image.
  */
  image->class=DirectClass;
  if (!image->matte)
    {
      /*
        Initialize matte data.
      */
      p=image->pixels;
      for (x=0; x < image->packets; x++)
      {
        p->index=Opaque;
        p++;
      }
      image->matte=True;
    }
  if (UncompressImage(image))
    for (y=0; y < crop_info.height; y++)
    {
      p=image->pixels+(crop_info.y+y)*image->columns+crop_info.x;
      for (x=0; x < crop_info.width; x++)
      {
        p->index=Transparent;
        p++;
      }
    }
  /*
    Update image configuration.
  */
  XConfigureImageColormap(display,resource_info,windows,image);
  (void) XConfigureImage(display,resource_info,windows,image);
  return(True);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   X D i s p l a y B a c k g r o u n d I m a g e                             %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Function XDisplayBackgroundImage displays an image in the background of a
%  window.
%
%  The format of the XDisplayBackgroundImage routine is:
%
%      status=XDisplayBackgroundImage(display,resource_info,image)
%
%  A description of each parameter follows:
%
%    o status: Function XDisplayBackgroundImage returns True if the
%      designated window is the root window.
%
%    o display: Specifies a connection to an X server;  returned from
%      XOpenDisplay.
%
%    o resource_info: Specifies a pointer to a X11 XResourceInfo structure.
%
%    o image: Specifies a pointer to a Image structure; returned from
%      ReadImage.
%
%
*/
static unsigned int XDisplayBackgroundImage
(
    Display*       display,
    XResourceInfo* resource_info,
    Image*         image
)
{
  char
    visual_type[MaxTextLength];

  unsigned int
    height,
    status,
    width;

  Window
    root_window;

  XGCValues
    context_values;

  XPixelInfo
    pixel_info;

  XResourceInfo
    resources;

  XStandardColormap
    *map_info;

  XVisualInfo
    *visual_info;

  XWindowAttributes
    window_attributes;

  XWindowInfo
    window_info;

  /*
    Determine target window.
  */
  window_info.id=(Window) NULL;
  window_info.ximage=(XImage *) NULL;
  window_info.matte_image=(XImage *) NULL;
  window_info.pixmap=(Pixmap) NULL;
  window_info.matte_pixmap=(Pixmap) NULL;
  window_info.shared_memory=False;
  root_window=XRootWindow(display,XDefaultScreen(display));
  if (Latin1Compare(resource_info->window_id,"root") == 0)
    window_info.id=root_window;
  else
    {
      if (isdigit(*resource_info->window_id))
        window_info.id=XWindowByID(display,root_window,(Window)
          strtol((char *) resource_info->window_id,(char **) NULL,0));
      if (window_info.id == (Window) NULL)
        window_info.id=
          XWindowByName(display,root_window,resource_info->window_id);
    }
  if (window_info.id == (Window) NULL)
    Error("No window with specified id exists",resource_info->window_id);
  /*
    Determine window visual id.
  */
  window_attributes.width=XDisplayWidth(display,XDefaultScreen(display));
  window_attributes.height=XDisplayHeight(display,XDefaultScreen(display));
  (void) strcpy(visual_type,"default");
  status=XGetWindowAttributes(display,window_info.id,&window_attributes);
  if (status != False)
    (void) sprintf(visual_type,"0x%lx",
      XVisualIDFromVisual(window_attributes.visual));
  /*
    Allocate standard colormap.
  */
  map_info=XAllocStandardColormap();
  if (map_info == (XStandardColormap *) NULL)
    Error("Unable to create standard colormap","Memory allocation failed");
  map_info->colormap=(Colormap) NULL;
  pixel_info.pixels=(unsigned long *) NULL;
  pixel_info.gamma_map=(XColor *) NULL;
  /*
    Initialize visual info.
  */
  resources=(*resource_info);
  resources.map_type=(char *) NULL;
  resources.visual_type=visual_type;
  visual_info=XBestVisualInfo(display,map_info,&resources);
  if (visual_info == (XVisualInfo *) NULL)
    Error("Unable to get visual",resource_info->visual_type);
  /*
    Free previous root colors.
  */
  if (window_info.id == root_window)
    XDestroyWindowColors(display,root_window);
  /*
    Initialize colormap.
  */
  resources.colormap=SharedColormap;
  XMakeStandardColormap(display,visual_info,&resources,image,map_info,
    &pixel_info);
  /*
    Graphic context superclass.
  */
  context_values.background=pixel_info.background_color.pixel;
  context_values.foreground=pixel_info.foreground_color.pixel;
  pixel_info.annotate_context=XCreateGC(display,window_info.id,GCBackground |
    GCForeground,&context_values);
  if (pixel_info.annotate_context == (GC) NULL)
    Error("Unable to create graphic context",(char *) NULL);
  /*
    Initialize Image window attributes.
  */
  XGetWindowInfo(display,visual_info,map_info,&pixel_info,(XFontStruct *) NULL,
    resource_info,&window_info);
  /*
    Create the X image.
  */
  window_info.width=image->columns;
  if (window_info.width >= window_attributes.width)
    window_info.width=window_attributes.width;
  window_info.height=image->rows;
  if (window_info.height >= window_attributes.height)
    window_info.height=window_attributes.height;
  status=XMakeImage(display,resource_info,&window_info,image,window_info.width,
    window_info.height);
  if (status == False)
    Error("Unable to create X image",(char *) NULL);
  window_info.x=0;
  window_info.y=0;
  if (resource_info->debug)
    {
      (void) fprintf(stderr,"Image: %s[%u] %ux%u ",image->filename,
        image->scene,image->columns,image->rows);
      if (image->colors != 0)
        (void) fprintf(stderr,"%uc ",image->colors);
      (void) fprintf(stderr,"%s\n",image->magick);
    }
  /*
    Adjust image dimensions as specified by backdrop or geometry options.
  */
  width=window_info.width;
  height=window_info.height;
  if (resource_info->backdrop)
    {
      /*
        Center image on root window.
      */
      window_info.x=(window_attributes.width >> 1)-
        (window_info.ximage->width >> 1);
      window_info.y=(window_attributes.height >> 1)-
        (window_info.ximage->height >> 1);
      width=window_attributes.width;
      height=window_attributes.height;
    }
  if (resource_info->image_geometry != (char *) NULL)
    {
      char
        default_geometry[MaxTextLength];

      int
        flags,
        gravity;

      XSizeHints
        *size_hints;

      /*
        User specified geometry.
      */
      size_hints=XAllocSizeHints();
      if (size_hints == (XSizeHints *) NULL)
        Error("Unable to display on window","Memory allocation failed");
      size_hints->flags=(long) NULL;
      (void) sprintf(default_geometry,"%ux%u",width,height);
      flags=XWMGeometry(display,visual_info->screen,
        resource_info->image_geometry,default_geometry,
        window_info.border_width,size_hints,&window_info.x,&window_info.y,
        (int *) &width,(int *) &height,&gravity);
      if (flags & (XValue | YValue))
        {
          width=window_attributes.width;
          height=window_attributes.height;
        }
      XFree((void *) size_hints);
    }
  /*
    Create the X pixmap.
  */
  window_info.pixmap=
    XCreatePixmap(display,window_info.id,width,height,window_info.depth);
  if (window_info.pixmap == (Pixmap) NULL)
    Error("Unable to create X pixmap",(char *) NULL);
  /*
    Display pixmap on the window.
  */
  if ((width > window_info.width) || (height > window_info.height))
    XFillRectangle(display,window_info.pixmap,window_info.annotate_context,
      0,0,width,height);
  XPutImage(display,window_info.pixmap,window_info.annotate_context,
    window_info.ximage,0,0,window_info.x,window_info.y,window_info.width,
    window_info.height);
  XSetWindowBackgroundPixmap(display,window_info.id,window_info.pixmap);
  XClearWindow(display,window_info.id);
  /*
    Free resources.
  */
  XFreePixmap(display,window_info.pixmap);
  XDestroyImage(window_info.ximage);
  XFreeGC(display,window_info.annotate_context);
  XFreeCursor(display,window_info.cursor);
  XFreeCursor(display,window_info.busy_cursor);
  if (window_info.id != root_window)
    XFreeStandardColormap(display,visual_info,map_info,&pixel_info);
  else
    {
      if (pixel_info.pixels != (unsigned long *) NULL)
        free((char *) pixel_info.pixels);
      if (pixel_info.gamma_map != (XColor *) NULL)
        free((char *) pixel_info.gamma_map);
    }
  XFree((void *) map_info);
  XFree((void *) visual_info);
  return(window_info.id == root_window);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   X D i s p l a y I m a g e                                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Function XDisplayImage displays an image via X11.  A new image is created
%  and returned if the user interactively transforms the displayed image.
%
%  The format of the XDisplayImage routine is:
%
%      loaded_image=XDisplayImage(display,resource_info,argv,argc,image,state)
%
%  A description of each parameter follows:
%
%    o loaded_image:  Function XDisplayImage returns an image when the
%      user chooses 'Load Image' from the command menu or picks a tile
%      from the image directory.  Otherwise a null image is returned.
%
%    o display: Specifies a connection to an X server;  returned from
%      XOpenDisplay.
%
%    o resource_info: Specifies a pointer to a X11 XResourceInfo structure.
%
%    o argv: Specifies the application's argument list.
%
%    o argc: Specifies the number of arguments.
%
%    o image: Specifies an address to an address of an Image structure;
%      returned from ReadImage.
%
%
*/
static Image* XDisplayImage
(
    Display*       display,
    XResourceInfo* resource_info,
    char**         argv,
    int            argc,
    Image**        image,
    long           unsigned int *state
)
{
#define MagnifySize  256  /* must be a power of 2 */
#define MagickMenus  9
#define MaxWindows  10
#define MagickTitle  "Commands"


  /* Kobus */
  extern int fs_image_modified;
  static int first_time = True;
  /* End Kobus */


  static char
    *CommandMenu[]=
    {
      "File",
      "Edit",
      "View",
      "Transform",
      "Enhance",
      "Effects",
      "Image Edit",
      "Miscellany",
      "Help",
      (char *) NULL
    },
    *FileMenu[]=
    {
      "Load...",
      "Next",
      "Former",
      "Select...",
      "Save...",
      "Print...",
      "Delete...",
      "Canvas...",
      "Visual Directory...",
      "Quit",
      (char *) NULL
    },
    *EditMenu[]=
    {
      "Undo",
      "Redo",
      "Cut",
      "Copy",
      "Paste",
      (char *) NULL
    },
    *ViewMenu[]=
    {
      "Half Size",
      "Original Size",
      "Double Size",
      "Resize...",
      "Apply",
      "Refresh",
      "Restore",
      (char *) NULL
    },
    *TransformMenu[]=
    {
      "Crop",
      "Chop",
      "Flop",
      "Flip",
      "Rotate Right",
      "Rotate Left",
      "Rotate...",
      "Shear...",
      "Trim Edges",
      (char *) NULL
    },
    *EnhanceMenu[]=
    {
      "Hue...",
      "Saturation...",
      "Brightness...",
      "Gamma...",
      "Spiff",
      "Dull",
      "Equalize",
      "Normalize",
      "Negate",
      "Grayscale",
      "Quantize...",
      (char *) NULL
    },
    *EffectsMenu[]=
    {
      "Despeckle",
      "Peak Noise",
      "Sharpen...",
      "Blur...",
      "Edge Detect",
      "Emboss",
      "Spread",
      "Oil Painting",
      "Raise...",
      "Segment...",
      (char *) NULL
    },
    *ImageEditMenu[]=
    {
      "Annotate...",
      "Draw...",
      "Color...",
      "Matte...",
      "Composite...",
      "Add Border...",
      "Add Frame...",
      "Comment...",
      "Region of Interest...",
      (char *) NULL
    },
    *MiscellanyMenu[]=
    {
      "Image Info",
      "Zoom Image",
      "Show Histogram",
      "Background...",
      "Slide Show...",
      "Preferences...",
      (char *) NULL
    },
    *HelpMenu[]=
    {
      "Overview",
      "Browse Documentation",
      "About Display",
      (char *) NULL
    },

#ifdef KEEP_SHORTCUTS     /* Kobus */
    *ShortCutsMenu[]=
    {
      "Next",
      "Load...",
      "Save...",
      "Undo",
      "Restore",
      "Crop",
      "Gamma...",
      "Image Info",
      "Quit",
      (char *) NULL
    },
#endif                      /* End Kobus */

    *ImmutableMenu[]=
    {
      "Image Info",
      "Quit",
      (char *) NULL
    };

  static char
    **Menus[MagickMenus]=
    {
      FileMenu,
      EditMenu,
      ViewMenu,
      TransformMenu,
      EnhanceMenu,
      EffectsMenu,
      ImageEditMenu,
      MiscellanyMenu,
      HelpMenu
    };

  static KeySym
    CommandKeys[]=
    {
      XK_VoidSymbol,
      XK_VoidSymbol,
      XK_VoidSymbol,
      XK_VoidSymbol,
      XK_VoidSymbol,
      XK_VoidSymbol,
      XK_VoidSymbol,
      XK_VoidSymbol,
      XK_VoidSymbol,
    },
    FileKeys[]=
    {
      XK_l,
      XK_n,
      XK_f,
      XK_F2,
      XK_s,
      XK_p,
      XK_Delete,
      XK_C,
      XK_V,
      XK_q
    },
    EditKeys[]=
    {
      XK_u,
      XK_Redo,
      XK_F3,
      XK_F4,
      XK_F5
    },
    ViewKeys[]=
    {
      XK_less,
      XK_o,
      XK_greater,
      XK_percent,
      XK_A,
      XK_at,
      XK_r
    },
    TransformKeys[]=
    {
      XK_bracketleft,
      XK_bracketright,
      XK_bar,
      XK_minus,
      XK_slash,
      XK_backslash,
      XK_asterisk,
      XK_F6,
      XK_t
    },
    EnhanceKeys[]=
    {
      XK_F7,
      XK_F8,
      XK_F9,
      XK_g,
      XK_F10,
      XK_F11,
      XK_equal,
      XK_N,
      XK_asciitilde,
      XK_G,
      XK_numbersign
    },
    EffectsKeys[]=
    {
      XK_D,
      XK_P,
      XK_S,
      XK_B,
      XK_E,
      XK_M,
      XK_F13,
      XK_O,
      XK_asciicircum,
      XK_Z
    },
    ImageEditKeys[]=
    {
      XK_a,
      XK_d,
      XK_c,
      XK_m,
      XK_x,
      XK_b,
      XK_F,
      XK_exclam,
      XK_R
    },
    MiscellanyKeys[]=
    {
      XK_i,
      XK_z,
      XK_H,
      XK_ampersand,
      XK_comma,
      XK_F12
    },
    HelpKeys[]=
    {
      XK_h,
      XK_Find,
      XK_v
    },

#ifdef KEEP_SHORTCUTS     /* Kobus */
    ShortCutsKeys[]=
    {
      XK_n,
      XK_l,
      XK_s,
      XK_u,
      XK_r,
      XK_bracketleft,
      XK_g,
      XK_i,
      XK_q
    },
#endif                      /* End Kobus */

    ImmutableKeys[]=
    {
      XK_i,
      XK_q
    };

  static KeySym
    *Keys[MagickMenus]=
    {
      FileKeys,
      EditKeys,
      ViewKeys,
      TransformKeys,
      EnhanceKeys,
      EffectsKeys,
      ImageEditKeys,
      MiscellanyKeys,
      HelpKeys
    };

  char
    command[MaxTextLength],
    resource_name[MaxTextLength];

  Image
    *displayed_image,
    *loaded_image;

  int
    entry,
    id,
    status;

  KeySym
    key_symbol;

  register int
    i;

  static char
    working_directory[MaxTextLength];

  static Window
    root_window;

  static XClassHint
    *class_hints;

  static XFontStruct
    *font_info;

  static XPixelInfo
    icon_pixel,
    pixel_info;

  static XPoint
    vid_info;

  static XResourceInfo
    icon_resources;

  static XStandardColormap
    *icon_map,
    *map_info;

  static XVisualInfo
    *icon_visual,
    *visual_info = (XVisualInfo *) NULL;

  static XWindowInfo
    *magick_windows[MaxWindows];

  static XWMHints
    *manager_hints;

  static unsigned int
    number_windows;

  struct stat
    file_info;

  time_t
    timer,
    time_stamp,
    update_time;

  unsigned int
    context_mask;

  XEvent
    event;

  XGCValues
    context_values;

  XWindowChanges
    window_changes;

  /* Kobus */
  IMPORT unsigned char *pure_pixels;
  int must_update_pure_pixels = True;
  char text[ 100 ];
  /* End Kobus */


  /* Kobus */
  if (fs_image_modified)
  {
#ifdef HOW_IT_WAS
       if (! isatty(fileno(stdout)))
#else
       if (fp_get_path_type(stdout) == PATH_IS_PIPE)
#endif
       {
           /*
           // Untested since switch to test on PIPE.  I have forgotten what, if
           // anything, we still use this for, but I vaguely recall that it was
           // explicitly when we were feeding this into another program.
           */
           UNTESTED_CODE();

           sprintf(text,"%d %d\n", EOF, EOF);
           write(fileno(stdout), text, strlen(text));
           fflush(stdout);
       }

       fs_image_modified = False;
      }
  /* End Kobus */


  if (visual_info != (XVisualInfo *) NULL)
    (void) chdir(working_directory);
  else
    {
      /*
        Allocate standard colormap.
      */
      if (resource_info->debug)
        {
          XSynchronize(display,True);
          (void) fprintf(stderr,"Version: %s\n",Version);
        }
      map_info=XAllocStandardColormap();
      icon_map=XAllocStandardColormap();
      if ((map_info == (XStandardColormap *) NULL) ||
          (icon_map == (XStandardColormap *) NULL))
        Error("Unable to create standard colormap","Memory allocation failed");
      map_info->colormap=(Colormap) NULL;
      icon_map->colormap=(Colormap) NULL;
      pixel_info.pixels=(unsigned long *) NULL;
      pixel_info.gamma_map=(XColor *) NULL;
      pixel_info.annotate_context=(GC) NULL;
      pixel_info.highlight_context=(GC) NULL;
      pixel_info.widget_context=(GC) NULL;
      font_info=(XFontStruct *) NULL;
      icon_pixel.annotate_context=(GC) NULL;
      icon_pixel.pixels=(unsigned long *) NULL;
      icon_pixel.gamma_map=(XColor *) NULL;
      /*
        Allocate visual.
      */
      icon_resources=(*resource_info);
      icon_resources.map_type=(char *) NULL;
      icon_resources.visual_type="default";
      icon_resources.colormap=SharedColormap;
      visual_info=XBestVisualInfo(display,map_info,resource_info);
      icon_visual=XBestVisualInfo(display,icon_map,&icon_resources);
      if ((visual_info == (XVisualInfo *) NULL) ||
          (icon_visual == (XVisualInfo *) NULL))
        Error("Unable to get visual",resource_info->visual_type);
      if (resource_info->debug)
        {
          (void) fprintf(stderr,"Visual:\n");
          (void) fprintf(stderr,"  visual id: 0x%lx\n",visual_info->visualid);
          (void) fprintf(stderr,"  class: %s\n",
            XVisualClassName(visual_info->class));
          (void) fprintf(stderr,"  depth: %d planes\n",visual_info->depth);
          (void) fprintf(stderr,"  size of colormap: %d entries\n",
            visual_info->colormap_size);
          (void) fprintf(stderr,"  red, green, blue masks: 0x%lx 0x%lx 0x%lx\n",
            visual_info->red_mask,visual_info->green_mask,
            visual_info->blue_mask);
          (void) fprintf(stderr,"  significant bits in color: %d bits\n",
            visual_info->bits_per_rgb);
        }
      /*
        Allocate atoms.
      */
      windows=(XWindows *) malloc(sizeof(XWindows));
      if (windows == (XWindows *) NULL)
        Error("Unable to create X windows","Memory allocation failed");
      windows->wm_protocols=XInternAtom(display,"WM_PROTOCOLS",False);
      windows->wm_delete_window=XInternAtom(display,"WM_DELETE_WINDOW",False);
      windows->wm_take_focus=XInternAtom(display,"WM_TAKE_FOCUS",False);
      windows->im_protocols=XInternAtom(display,"IM_PROTOCOLS",False);
      windows->im_update_widget=XInternAtom(display,"IM_UPDATE_WIDGET",False);
      windows->im_update_colormap=
        XInternAtom(display,"IM_UPDATE_COLORMAP",False);
      windows->im_former_image=XInternAtom(display,"IM_FORMER_IMAGE",False);
      windows->im_next_image=XInternAtom(display,"IM_NEXT_IMAGE",False);
      windows->im_retain_colors=XInternAtom(display,"IM_RETAIN_COLORS",False);
      windows->im_exit=XInternAtom(display,"IM_EXIT",False);
      if (resource_info->debug)
        {
          (void) fprintf(stderr,"Protocols:\n");
          (void) fprintf(stderr,"  Window Manager: 0x%lx\n",
            windows->wm_protocols);
          (void) fprintf(stderr,"    delete window: 0x%lx\n",
            windows->wm_delete_window);
          (void) fprintf(stderr,"    take focus: 0x%lx\n",
            windows->wm_take_focus);
          (void) fprintf(stderr,"  ImageMagick: 0x%lx\n",
            windows->im_protocols);
          (void) fprintf(stderr,"    update widget: 0x%lx\n",
            windows->im_update_widget);
          (void) fprintf(stderr,"    update colormap: 0x%lx\n",
            windows->im_update_colormap);
          (void) fprintf(stderr,"    former image: 0x%lx\n",
            windows->im_former_image);
          (void) fprintf(stderr,"    next image: 0x%lx\n",
            windows->im_next_image);
          (void) fprintf(stderr,"    retain colors: 0x%lx\n",
            windows->im_retain_colors);
          (void) fprintf(stderr,"    exit: 0x%lx\n",windows->im_exit);
        }
      /*
        Allocate class and manager hints.
      */
      class_hints=XAllocClassHint();
      manager_hints=XAllocWMHints();
      if ((class_hints == (XClassHint *) NULL) ||
          (manager_hints == (XWMHints *) NULL))
        Error("Unable to allocate X hints",(char *) NULL);
      /*
        Determine group leader if we have one.
      */
      root_window=XRootWindow(display,visual_info->screen);
      windows->group_leader.id=(Window) NULL;
      if (resource_info->window_group != (char *) NULL)
        {
          if (isdigit(*resource_info->window_group))
            windows->group_leader.id=XWindowByID(display,root_window,(Window)
              strtol((char *) resource_info->window_group,(char **) NULL,0));
          if (windows->group_leader.id == (Window) NULL)
            windows->group_leader.id=
              XWindowByName(display,root_window,resource_info->window_group);
        }
      /*
        Initialize window id's.
      */
      number_windows=0;
      magick_windows[number_windows++]=(&windows->icon);
      magick_windows[number_windows++]=(&windows->backdrop);
      magick_windows[number_windows++]=(&windows->image);
      magick_windows[number_windows++]=(&windows->info);
      magick_windows[number_windows++]=(&windows->command);
      magick_windows[number_windows++]=(&windows->widget);
      magick_windows[number_windows++]=(&windows->popup);
      magick_windows[number_windows++]=(&windows->magnify);
      magick_windows[number_windows++]=(&windows->pan);
      for (i=0; i < number_windows; i++)
        magick_windows[i]->id=(Window) NULL;
      vid_info.x=0;
      vid_info.y=0;
    }
  /*
    Initialize Standard Colormap.
  */
  loaded_image=(Image *) NULL;
  displayed_image=(*image);
  if (resource_info->debug)
    {
      (void) fprintf(stderr,"Image: %s[%u] %ux%u ",displayed_image->filename,
        displayed_image->scene,displayed_image->columns,displayed_image->rows);
      if (displayed_image->colors != 0)
        (void) fprintf(stderr,"%uc ",displayed_image->colors);
      (void) fprintf(stderr,"%s\n",displayed_image->magick);
    }
  XMakeStandardColormap(display,visual_info,resource_info,displayed_image,
    map_info,&pixel_info);
  /*
    Initialize font info.
  */
  if (font_info != (XFontStruct *) NULL)
    XFreeFont(display,font_info);
  font_info=XBestFont(display,resource_info,False);
  if (font_info == (XFontStruct *) NULL)
    Error("Unable to load font",resource_info->font);
  /*
    Initialize graphic context.
  */
  windows->context.id=(Window) NULL;
  XGetWindowInfo(display,visual_info,map_info,&pixel_info,font_info,
    resource_info,&windows->context);
  class_hints->res_name="superclass";
  class_hints->res_class="Display";
  manager_hints->flags=InputHint | StateHint;
  manager_hints->input=False;
  manager_hints->initial_state=WithdrawnState;
  XMakeWindow(display,root_window,argv,argc,class_hints,manager_hints,
    &windows->context);
  if (resource_info->debug)
    (void) fprintf(stderr,"Window id: 0x%lx (context)\n",windows->context.id);
  context_values.background=pixel_info.background_color.pixel;
  context_values.font=font_info->fid;
  context_values.foreground=pixel_info.foreground_color.pixel;
  context_values.graphics_exposures=False;
  context_mask=GCBackground | GCFont | GCForeground | GCGraphicsExposures;
  if (pixel_info.annotate_context != (GC) NULL)
    XFreeGC(display,pixel_info.annotate_context);
  pixel_info.annotate_context=
    XCreateGC(display,windows->context.id,context_mask,&context_values);
  if (pixel_info.annotate_context == (GC) NULL)
    Error("Unable to create graphic context",(char *) NULL);
  context_values.background=pixel_info.depth_color.pixel;
  if (pixel_info.widget_context != (GC) NULL)
    XFreeGC(display,pixel_info.widget_context);
  pixel_info.widget_context=
    XCreateGC(display,windows->context.id,context_mask,&context_values);
  if (pixel_info.widget_context == (GC) NULL)
    Error("Unable to create graphic context",(char *) NULL);
  context_values.background=pixel_info.foreground_color.pixel;
  context_values.foreground=pixel_info.background_color.pixel;
  context_values.plane_mask=
    context_values.background ^ context_values.foreground;
  if (pixel_info.highlight_context != (GC) NULL)
    XFreeGC(display,pixel_info.highlight_context);
  pixel_info.highlight_context=XCreateGC(display,windows->context.id,
    context_mask | GCPlaneMask,&context_values);
  if (pixel_info.highlight_context == (GC) NULL)
    Error("Unable to create graphic context",(char *) NULL);
  XDestroyWindow(display,windows->context.id);
  /*
    Initialize icon window.
  */
  XGetWindowInfo(display,icon_visual,icon_map,&icon_pixel,(XFontStruct *) NULL,
    &icon_resources,&windows->icon);
  windows->icon.geometry=resource_info->icon_geometry;
  XBestIconSize(display,&windows->icon,displayed_image);
  windows->icon.attributes.colormap=
    XDefaultColormap(display,icon_visual->screen);
  windows->icon.attributes.event_mask=ExposureMask | StructureNotifyMask;
  class_hints->res_name="icon";
  manager_hints->flags=InputHint | StateHint;
  manager_hints->input=False;
  manager_hints->initial_state=IconicState;
  XMakeWindow(display,root_window,argv,argc,class_hints,manager_hints,
    &windows->icon);
  if (resource_info->debug)
    (void) fprintf(stderr,"Window id: 0x%lx (icon)\n",windows->icon.id);
  /*
    Initialize graphic context for icon window.
  */
  if (icon_pixel.annotate_context != (GC) NULL)
    XFreeGC(display,icon_pixel.annotate_context);
  context_values.background=icon_pixel.background_color.pixel;
  context_values.foreground=icon_pixel.foreground_color.pixel;
  icon_pixel.annotate_context=XCreateGC(display,windows->icon.id,
    GCBackground | GCForeground,&context_values);
  if (icon_pixel.annotate_context == (GC) NULL)
    Error("Unable to create graphic context",(char *) NULL);
  windows->icon.annotate_context=icon_pixel.annotate_context;
  /*
    Initialize Image window.
  */
  if (windows->image.id != (Window) NULL)
    {
      free((char *) windows->image.name);
      free((char *) windows->image.icon_name);
    }
  XGetWindowInfo(display,visual_info,map_info,&pixel_info,font_info,
    resource_info,&windows->image);
  windows->image.name=(char *) malloc(MaxTextLength*sizeof(char));
  windows->image.icon_name=(char *) malloc(MaxTextLength*sizeof(char));
  if ((windows->image.name == NULL) || (windows->image.icon_name == NULL))
    Error("Unable to create Image window","Memory allocation failed");
  if ((resource_info->title != (char *) NULL) && !(*state & MontageImageState))
    {
      /*
        User specified window name.
      */
      (void) strcpy(windows->image.name,resource_info->title);
      (void) strcpy(windows->image.icon_name,resource_info->title);
    }
  else
    {
      char
        *p;

      /*
        Window name is the base of the filename.
      */
      p=displayed_image->filename+strlen(displayed_image->filename)-1;
      while ((p > displayed_image->filename) && (*(p-1) != *BasenameSeparator))
        p--;
      (void) sprintf(windows->image.name,"ImageMagick: %s[%u]",p,
        displayed_image->scene);
      if ((displayed_image->previous == (Image *) NULL) &&
          (displayed_image->next == (Image *) NULL) &&
          (displayed_image->scene == 0))
        (void) sprintf(windows->image.name,"ImageMagick: %s",p);
      (void) strcpy(windows->image.icon_name,p);
    }
  if (resource_info->immutable)
    windows->image.immutable=True;
  windows->image.geometry=resource_info->image_geometry;
  windows->image.width=displayed_image->columns;
  if (windows->image.width > XDisplayWidth(display,visual_info->screen))
    windows->image.width=XDisplayWidth(display,visual_info->screen);
  windows->image.height=displayed_image->rows;
  if (windows->image.height > XDisplayHeight(display,visual_info->screen))
    windows->image.height=XDisplayHeight(display,visual_info->screen);
  windows->image.attributes.event_mask=ButtonMotionMask | ButtonPressMask |
    ButtonReleaseMask | EnterWindowMask | ExposureMask | KeyPressMask |
    KeyReleaseMask | LeaveWindowMask | OwnerGrabButtonMask |
    StructureNotifyMask;
  XGetWindowInfo(display,visual_info,map_info,&pixel_info,font_info,
    resource_info,&windows->backdrop);
  if ((resource_info->backdrop) || (windows->backdrop.id != (Window) NULL))
    {
      /*
        Initialize backdrop window.
      */
      windows->backdrop.x=0;
      windows->backdrop.y=0;
      windows->backdrop.name="ImageMagick Backdrop";
      windows->backdrop.flags=USSize | USPosition;
      windows->backdrop.width=XDisplayWidth(display,visual_info->screen);
      windows->backdrop.height=XDisplayHeight(display,visual_info->screen);
      windows->backdrop.border_width=0;
      windows->backdrop.immutable=True;
      windows->backdrop.attributes.do_not_propagate_mask=ButtonPressMask |
        ButtonReleaseMask;
      windows->backdrop.attributes.event_mask=ButtonPressMask | KeyPressMask |
        StructureNotifyMask;
      windows->backdrop.attributes.override_redirect=True;
      class_hints->res_name="backdrop";
      manager_hints->flags=IconWindowHint | InputHint | StateHint;
      manager_hints->icon_window=windows->icon.id;
      manager_hints->input=True;
      manager_hints->initial_state=
        resource_info->iconic ? IconicState : NormalState;
      XMakeWindow(display,root_window,argv,argc,class_hints,manager_hints,
        &windows->backdrop);
      if (resource_info->debug)
        (void) fprintf(stderr,"Window id: 0x%lx (backdrop)\n",
          windows->backdrop.id);
      XMapWindow(display,windows->backdrop.id);
      XClearWindow(display,windows->backdrop.id);
      if (windows->image.id != (Window) NULL)
        {
          XDestroyWindow(display,windows->image.id);
          windows->image.id=(Window) NULL;
        }
      /*
        Position image in the center the backdrop.
      */
      windows->image.flags|=USPosition;
      windows->image.x=(XDisplayWidth(display,visual_info->screen) >> 1)-
        (windows->image.width >> 1);
      windows->image.y=(XDisplayHeight(display,visual_info->screen) >> 1)-
        (windows->image.height >> 1);
    }
  if (resource_info->name == (char *) NULL)
    class_hints->res_name=client_name;
  else
    class_hints->res_name=resource_info->name;
  manager_hints->flags=IconWindowHint | InputHint | StateHint;
  manager_hints->icon_window=windows->icon.id;
  manager_hints->input=True;
  manager_hints->initial_state=
    resource_info->iconic ? IconicState : NormalState;
  if (windows->group_leader.id != (Window) NULL)
    {
      /*
        Follow the leader.
      */
      manager_hints->flags|=WindowGroupHint;
      manager_hints->window_group=windows->group_leader.id;
      XSelectInput(display,windows->group_leader.id,StructureNotifyMask);
      if (resource_info->debug)
        (void) fprintf(stderr,"Window id: 0x%lx (group leader)\n",
          windows->group_leader.id);
    }
  XMakeWindow(display,
    (Window) (resource_info->backdrop ? windows->backdrop.id : root_window),
    argv,argc,class_hints,manager_hints,&windows->image);
  if (windows->group_leader.id != (Window) NULL)
    XSetTransientForHint(display,windows->image.id,windows->group_leader.id);
  if (resource_info->debug)
    (void) fprintf(stderr,"Window id: 0x%lx (image)\n",windows->image.id);
  /*
    Initialize X image structure.
  */
  windows->image.x=0;
  windows->image.y=0;
  status=XMakeImage(display,resource_info,&windows->image,displayed_image,
    displayed_image->columns,displayed_image->rows);
  if (status == False)
    Error("Unable to create X image",(char *) NULL);
  if (resource_info->use_pixmap)
    (void) XMakePixmap(display,resource_info,&windows->image);
  if (!windows->image.mapped || (windows->backdrop.id != (Window) NULL))
    XMapWindow(display,windows->image.id);
  if (windows->image.mapped)
    XRefreshWindow(display,&windows->image,(XEvent *) NULL);
  /*
    Initialize Info widget.
  */
  XGetWindowInfo(display,visual_info,map_info,&pixel_info,font_info,
    resource_info,&windows->info);
  windows->info.name="Info";
  windows->info.icon_name="Info";
  windows->info.border_width=1;
  windows->info.x=2;
  windows->info.y=2;
  windows->info.flags|=PPosition;
  windows->info.attributes.save_under=True;
  windows->info.attributes.win_gravity=UnmapGravity;
  windows->info.attributes.event_mask=
    ButtonPressMask | ExposureMask | StructureNotifyMask;
  class_hints->res_name="info";
  manager_hints->flags=InputHint | StateHint | WindowGroupHint;
  manager_hints->input=False;
  manager_hints->initial_state=NormalState;
  manager_hints->window_group=windows->image.id;
  XMakeWindow(display,windows->image.id,argv,argc,class_hints,manager_hints,
    &windows->info);
  XSetTransientForHint(display,windows->info.id,windows->image.id);
  if (windows->image.mapped)
    XWithdrawWindow(display,windows->info.id,windows->info.screen);
  if (resource_info->debug)
    (void) fprintf(stderr,"Window id: 0x%lx (info)\n",windows->info.id);
  /*
    Initialize Command widget.
  */
  XGetWindowInfo(display,visual_info,map_info,&pixel_info,font_info,
    resource_info,&windows->command);
  windows->command.data=MagickMenus;
  (void) XCommandWidget(display,windows,CommandMenu,(XEvent *) NULL);
  (void) sprintf(resource_name,"%s.command",client_name);
  windows->command.geometry=XGetResourceClass(resource_info->resource_database,
    resource_name,"geometry",(char *) NULL);
  windows->command.name=MagickTitle;
  windows->command.border_width=0;
  windows->command.flags|=PPosition;
  windows->command.attributes.backing_store=WhenMapped;
  windows->command.attributes.save_under=True;
  windows->command.attributes.event_mask=ButtonMotionMask | ButtonPressMask |
    ButtonReleaseMask | EnterWindowMask | ExposureMask | LeaveWindowMask |
    OwnerGrabButtonMask | StructureNotifyMask;
  class_hints->res_name="command";
  manager_hints->flags=InputHint | StateHint | WindowGroupHint;
  manager_hints->input=False;
  manager_hints->initial_state=NormalState;
  manager_hints->window_group=windows->image.id;
  XMakeWindow(display,root_window,argv,argc,class_hints,manager_hints,
    &windows->command);
  windows->command.highlight_stipple=XCreateBitmapFromData(display,
    windows->command.id,(char *) HighlightBitmap,HighlightWidth,
    HighlightHeight);
  windows->command.shadow_stipple=XCreateBitmapFromData(display,
    windows->command.id,(char *) ShadowBitmap,ShadowWidth,ShadowHeight);
  XSetTransientForHint(display,windows->command.id,windows->image.id);
  if (windows->command.mapped)
    XMapRaised(display,windows->command.id);
  if (resource_info->debug)
    (void) fprintf(stderr,"Window id: 0x%lx (command)\n",windows->command.id);
  /*
    Initialize Widget window.
  */
  if (windows->widget.id != (Window) NULL)
    free((char *) windows->widget.name);
  XGetWindowInfo(display,visual_info,map_info,&pixel_info,font_info,
    resource_info,&windows->widget);
  (void) sprintf(resource_name,"%s.widget",client_name);
  windows->widget.geometry=XGetResourceClass(resource_info->resource_database,
    resource_name,"geometry",(char *) NULL);
  windows->widget.name=(char *) malloc(MaxTextLength*sizeof(char));
  if (windows->widget.name == NULL)
    Error("Unable to create Image window","Memory allocation failed");
  *windows->widget.name='\0';
  windows->widget.border_width=0;
  windows->widget.flags|=PPosition;
  windows->widget.attributes.backing_store=WhenMapped;
  windows->widget.attributes.save_under=True;
  windows->widget.attributes.event_mask=ButtonMotionMask | ButtonPressMask |
    ButtonReleaseMask | EnterWindowMask | ExposureMask | KeyPressMask |
    KeyReleaseMask | LeaveWindowMask | StructureNotifyMask;
  class_hints->res_name="widget";
  manager_hints->flags=InputHint | StateHint | WindowGroupHint;
  manager_hints->input=True;
  manager_hints->initial_state=NormalState;
  manager_hints->window_group=windows->image.id;
  XMakeWindow(display,root_window,argv,argc,class_hints,manager_hints,
    &windows->widget);
  windows->widget.highlight_stipple=windows->command.highlight_stipple;
  windows->widget.shadow_stipple=windows->command.shadow_stipple;
  XSetTransientForHint(display,windows->widget.id,windows->image.id);
  if (resource_info->debug)
    (void) fprintf(stderr,"Window id: 0x%lx (widget)\n",windows->widget.id);
  /*
    Initialize popup window.
  */
  if (windows->popup.id != (Window) NULL)
    free((char *) windows->popup.name);
  XGetWindowInfo(display,visual_info,map_info,&pixel_info,font_info,
    resource_info,&windows->popup);
  windows->popup.name=(char *) malloc(MaxTextLength*sizeof(char));
  if (windows->popup.name == NULL)
    Error("Unable to create Image window","Memory allocation failed");
  *windows->popup.name='\0';
  windows->popup.border_width=0;
  windows->popup.flags|=PPosition;
  windows->popup.attributes.backing_store=WhenMapped;
  windows->popup.attributes.save_under=True;
  windows->popup.attributes.event_mask=ButtonMotionMask | ButtonPressMask |
    ButtonReleaseMask | EnterWindowMask | ExposureMask | KeyPressMask |
    KeyReleaseMask | LeaveWindowMask | StructureNotifyMask;
  class_hints->res_name="popup";
  manager_hints->flags=InputHint | StateHint | WindowGroupHint;
  manager_hints->input=True;
  manager_hints->initial_state=NormalState;
  manager_hints->window_group=windows->image.id;
  XMakeWindow(display,root_window,argv,argc,class_hints,manager_hints,
    &windows->popup);
  windows->popup.highlight_stipple=windows->command.highlight_stipple;
  windows->popup.shadow_stipple=windows->command.shadow_stipple;
  XSetTransientForHint(display,windows->popup.id,windows->image.id);
  if (resource_info->debug)
    (void) fprintf(stderr,"Window id: 0x%lx (pop up)\n",windows->popup.id);
  /*
    Initialize Magnify window and cursor.
  */
  if (windows->magnify.id != (Window) NULL)
    free((char *) windows->magnify.name);
  XGetWindowInfo(display,visual_info,map_info,&pixel_info,font_info,
    resource_info,&windows->magnify);
  (void) sprintf(resource_name,"%s.magnify",client_name);
  windows->magnify.geometry=XGetResourceClass(resource_info->resource_database,
    resource_name,"geometry",(char *) NULL);
  windows->magnify.name=(char *) malloc(MaxTextLength*sizeof(char));
  if (windows->magnify.name == NULL)
    Error("Unable to create Magnify window","Memory allocation failed");
  (void) sprintf(windows->magnify.name,"Magnify %uX",resource_info->magnify);
  windows->magnify.cursor=XMakeCursor(display,windows->image.id,
    map_info->colormap,resource_info->background_color,
    resource_info->foreground_color);
  if (windows->magnify.cursor == (Cursor) NULL)
    Error("Unable to create cursor",(char *) NULL);
  windows->magnify.width=MagnifySize;
  windows->magnify.height=MagnifySize;
  windows->magnify.flags|=PPosition;
  windows->magnify.min_width=MagnifySize;
  windows->magnify.min_height=MagnifySize;
  windows->magnify.width_inc=MagnifySize;
  windows->magnify.height_inc=MagnifySize;
  windows->magnify.data=resource_info->magnify;
  windows->magnify.attributes.save_under=True;
  windows->magnify.attributes.cursor=windows->magnify.cursor;
  windows->magnify.attributes.event_mask=ButtonPressMask | ButtonReleaseMask |
    ExposureMask | KeyPressMask | KeyReleaseMask | OwnerGrabButtonMask |
    StructureNotifyMask;
  class_hints->res_name="magnify";
  manager_hints->flags=InputHint | StateHint | WindowGroupHint;
  manager_hints->input=True;
  manager_hints->initial_state=NormalState;
  manager_hints->window_group=windows->image.id;
  XMakeWindow(display,root_window,argv,argc,class_hints,manager_hints,
    &windows->magnify);
  if (resource_info->debug)
    (void) fprintf(stderr,"Window id: 0x%lx (magnify)\n",windows->magnify.id);
  XSetTransientForHint(display,windows->magnify.id,windows->image.id);
  if (windows->magnify.mapped)
    XMapRaised(display,windows->magnify.id);
  /*
    Initialize panning window.
  */
  XGetWindowInfo(display,visual_info,map_info,&pixel_info,font_info,
    resource_info,&windows->pan);
  windows->pan.name="Pan Icon";
  XBestIconSize(display,&windows->pan,displayed_image);
  while ((windows->pan.width < MinPanSize) &&
         (windows->pan.height < MinPanSize))
  {
    windows->pan.width<<=1;
    windows->pan.height<<=1;
  }
  (void) sprintf(resource_name,"%s.pan",client_name);
  windows->pan.geometry=XGetResourceClass(resource_info->resource_database,
    resource_name,"geometry",(char *) NULL);
  if (windows->pan.geometry != (char *) NULL)
    ParseImageGeometry(windows->pan.geometry,&windows->pan.width,
      &windows->pan.height);
  windows->pan.flags|=PPosition;
  windows->pan.immutable=True;
  windows->pan.attributes.save_under=True;
  windows->pan.attributes.event_mask=ButtonMotionMask | ButtonPressMask |
    ButtonReleaseMask | ExposureMask | KeyPressMask | KeyReleaseMask |
    StructureNotifyMask;
  class_hints->res_name="pan";
  manager_hints->flags=InputHint | StateHint | WindowGroupHint;
  manager_hints->input=True;
  manager_hints->initial_state=NormalState;
  manager_hints->window_group=windows->image.id;
  XMakeWindow(display,root_window,argv,argc,class_hints,manager_hints,
    &windows->pan);
  if (resource_info->debug)
    (void) fprintf(stderr,"Window id: 0x%lx (pan)\n",windows->pan.id);
  XSetTransientForHint(display,windows->pan.id,windows->image.id);
  if (windows->image.mapped)
    if ((windows->image.width < windows->image.ximage->width) ||
        (windows->image.height < windows->image.ximage->height))
      XMapRaised(display,windows->pan.id);
  /*
    Respond to events.
  */
  (void) SetMonitorHandler(XProgressMonitor);
  (void) SetWarningHandler(XWarning);
  timer=time((time_t *) NULL)+resource_info->delay;
  update_time=0;
  if (resource_info->update)
    {
      /*
        Determine when file data was last modified.
      */
      status=stat(displayed_image->filename,&file_info);
      if (status == 0)
        update_time=file_info.st_mtime;
    }
  *state&=(~FormerImageState);
  *state&=(~MontageImageState);
  *state&=(~NextImageState);
  do
  {
    /*
      Handle a window event.
    */
    if (resource_info->delay != 0)
      {
        if (timer < time((time_t *) NULL))
          if (!resource_info->update)
            *state|=NextImageState | ExitState;
          else
            {
              /*
                Determine if image file was modified.
              */
              status=stat(displayed_image->filename,&file_info);
              if (status == 0)
                if (update_time != file_info.st_mtime)
                  {
                    /*
                      Redisplay image.
                    */
                    (void) strcpy(resource_info->image_info->filename,
                      displayed_image->filename);
                    loaded_image=ReadImage(resource_info->image_info);
                    if (loaded_image != (Image *) NULL)
                      *state|=NextImageState | ExitState;
                  }
              timer=time((time_t *) NULL)+resource_info->delay;
            }
        if (XEventsQueued(display,QueuedAfterFlush) == 0)
          {
            /*
              Do not block if delay > 0.
            */
            XDelay(display,SuspendTime << 2);
            continue;
          }
      }
    time_stamp=time((time_t *) NULL);
    XNextEvent(display,&event);
    if (!windows->image.stasis)
      windows->image.stasis=(time((time_t *) NULL)-time_stamp) > 0;
    if (event.xany.window == windows->command.id)
      {
        /*
          Select a command from the Command widget.
        */
        id=XCommandWidget(display,windows,CommandMenu,&event);
        if (id < 0)
          continue;
        (void) strcpy(command,CommandMenu[id]);
        key_symbol=CommandKeys[id];
        if (id < MagickMenus)
          {
            /*
              Select a command from a pop-up menu.
            */
            entry=XMenuWidget(display,windows,CommandMenu[id],Menus[id],
              command);
            if (entry < 0)
              continue;
            (void) strcpy(command,Menus[id][entry]);
            key_symbol=Keys[id][entry];
          }
        if (key_symbol != XK_VoidSymbol)
          loaded_image=XMagickCommand(display,resource_info,windows,0,
            key_symbol,&displayed_image);
        continue;
      }
    switch (event.type)
    {
      case ButtonPress:
      {
        if (resource_info->debug)
          (void) fprintf(stderr,"Button Press: 0x%lx %u +%d+%d\n",
            event.xbutton.window,event.xbutton.button,event.xbutton.x,
            event.xbutton.y);
        if ((event.xbutton.button == Button3) &&
            (event.xbutton.state & Mod1Mask))
          {
            /*
              Convert Alt-Button3 to Button2.
            */
            event.xbutton.button=Button2;
            event.xbutton.state&=(~Mod1Mask);
          }
        if (event.xbutton.window == windows->backdrop.id)
          {
            XSetInputFocus(display,event.xbutton.window,RevertToParent,
              event.xbutton.time);
            break;
          }
        if (event.xbutton.window == windows->image.id)
          {
            switch (event.xbutton.button)
            {
              case Button1:
              {
                if (resource_info->immutable)
                  {
                    /*
                      Select a command from the Immutable menu.
                    */
                    entry=XMenuWidget(display,windows,"Commands",ImmutableMenu,
                      command);
                    if (entry >= 0)
                      loaded_image=XMagickCommand(display,resource_info,
                        windows,0,ImmutableKeys[entry],&displayed_image);
                    break;
                  }
                /*
                  Map/unmap Command widget.
                */
                if (windows->command.mapped)
                  XWithdrawWindow(display,windows->command.id,
                    windows->command.screen);
                else
                  {
                    (void) XCommandWidget(display,windows,CommandMenu,
                      (XEvent *) NULL);
                    XMapRaised(display,windows->command.id);
                  }
                break;
              }
              case Button2:
              {
                /*
                  User pressed the image magnify button.
                */

               /* Kobus */
               if (must_update_pure_pixels)
               {
                   update_pure_pixels(display, resource_info, windows,&event,
                                      &displayed_image);
                   must_update_pure_pixels = FALSE;
               }
               /* END Kobus */

               (void) XMagickCommand(display,resource_info,windows,0,
                  (KeySym) XK_z,&displayed_image);
                XMagnifyImage(display,windows,&event);
                break;
              }
              case Button3:
              {
               /*
                   Kobus: Replace code for normal handling of Button3 to
                   an entirely new routine.
               */
               if (must_update_pure_pixels)
               {
                   update_pure_pixels(display, resource_info, windows,&event,
                                      &displayed_image);
                   must_update_pure_pixels = FALSE;
               }

               XTraceImage(display, resource_info, windows,&event,
                           &displayed_image);

               /* END Kobus */

               break;
              }
              default:
                break;
            }
            break;
          }
        if (event.xbutton.window == windows->magnify.id)
          {
            char
              command[MaxTextLength];

            int
              factor;

            static char
              *MagnifyMenu[]=
              {
                "2",
                "3",
                "4",
                "5",
                "6",
                "7",
                "8",
                "9",
                (char *) NULL,
              };

            static KeySym
              MagnifyKeys[]=
              {
                XK_2,
                XK_4,
                XK_5,
                XK_6,
                XK_7,
                XK_8,
                XK_9,
                XK_3
              };

            /*
              Select a magnify factor from the pop-up menu.
            */
            factor=XMenuWidget(display,windows,"Magnify",MagnifyMenu,command);
            if (factor >= 0)
              XMagnifyWindowCommand(display,windows,0,MagnifyKeys[factor]);
            break;
          }
        if (event.xbutton.window == windows->pan.id)
          {
            XPanImage(display,windows,&event);
            break;
          }
        timer=time((time_t *) NULL)+resource_info->delay;
        break;
      }
      case ButtonRelease:
        break;
      case ClientMessage:
      {
        if (resource_info->debug)
          (void) fprintf(stderr,"Client Message: 0x%lx 0x%lx %d 0x%lx\n",
            event.xclient.window,event.xclient.message_type,
            event.xclient.format,(unsigned long) event.xclient.data.l[0]);
        if (event.xclient.message_type == windows->im_protocols)
          {
            if (*event.xclient.data.l == windows->im_update_widget)
              {
                windows->command.name=MagickTitle;
                windows->command.data=MagickMenus;
                (void) XCommandWidget(display,windows,CommandMenu,
                  (XEvent *) NULL);
                break;
              }
            if (*event.xclient.data.l == windows->im_update_colormap)
              {
                /*
                  Update graphic context and window colormap.
                */
                for (i=0; i < number_windows; i++)
                {
                  if (magick_windows[i]->id == windows->icon.id)
                    continue;
                  context_values.background=pixel_info.background_color.pixel;
                  context_values.foreground=pixel_info.foreground_color.pixel;
                  XChangeGC(display,magick_windows[i]->annotate_context,
                    context_mask,&context_values);
                  XChangeGC(display,magick_windows[i]->widget_context,
                    context_mask,&context_values);
                  context_values.background=pixel_info.foreground_color.pixel;
                  context_values.foreground=pixel_info.background_color.pixel;
                  context_values.plane_mask=
                    context_values.background ^ context_values.foreground;
                  XChangeGC(display,magick_windows[i]->highlight_context,
                    context_mask | GCPlaneMask,&context_values);
                  magick_windows[i]->attributes.background_pixel=
                    pixel_info.background_color.pixel;
                  magick_windows[i]->attributes.border_pixel=
                    pixel_info.border_color.pixel;
                  magick_windows[i]->attributes.colormap=map_info->colormap;
                  XChangeWindowAttributes(display,magick_windows[i]->id,
                    magick_windows[i]->mask,&magick_windows[i]->attributes);
                }
                if (windows->pan.mapped)
                  {
                    XSetWindowBackgroundPixmap(display,windows->pan.id,
                      windows->pan.pixmap);
                    XClearWindow(display,windows->pan.id);
                    XDrawPanRectangle(display,windows);
                  }
                if (windows->backdrop.id != (Window) NULL)
                  XInstallColormap(display,map_info->colormap);
                break;
              }
            if (*event.xclient.data.l == windows->im_former_image)
              {
                *state|=FormerImageState | ExitState;
                break;
              }
            if (*event.xclient.data.l == windows->im_next_image)
              {
                *state|=NextImageState | ExitState;
                break;
              }
            if (*event.xclient.data.l == windows->im_retain_colors)
              {
                *state|=RetainColorsState;
                break;
              }
            if (*event.xclient.data.l == windows->im_exit)
              {
                *state|=ExitState;
                break;
              }
            break;
          }
        /*
          If client window delete message, exit.
        */
        if (event.xclient.message_type != windows->wm_protocols)
          break;
        if (*event.xclient.data.l != windows->wm_delete_window)
          break;
        XWithdrawWindow(display,event.xclient.window,visual_info->screen);
        if (event.xclient.window == windows->image.id)
          {
            *state|=ExitState;
            break;
          }
        if (event.xclient.window == windows->pan.id)
          {
            /*
              Restore original image size when pan window is deleted.
            */
            windows->image.window_changes.width=windows->image.ximage->width;
            windows->image.window_changes.height=windows->image.ximage->height;
            (void) XConfigureImage(display,resource_info,windows,
              displayed_image);
          }
        break;
      }
      case ConfigureNotify:
      {
        if (resource_info->debug)
          (void) fprintf(stderr,"Configure Notify: 0x%lx %dx%d+%d+%d %d\n",
            event.xconfigure.window,event.xconfigure.width,
            event.xconfigure.height,event.xconfigure.x,event.xconfigure.y,
            event.xconfigure.send_event);
        if (event.xconfigure.window == windows->image.id)
          {
            /*
              Image window has a new configuration.
            */
            if (event.xconfigure.send_event != 0)
              {
                XWindowChanges
                  window_changes;

                /*
                  Position the transient windows relative of the Image window.
                */
                if (windows->command.geometry == (char *) NULL)
                  if (!windows->command.mapped)
                    {
                      windows->command.x=
                        event.xconfigure.x-windows->command.width-25;
                      windows->command.y=event.xconfigure.y;
                      XConstrainWindowPosition(display,&windows->command);
                      window_changes.x=windows->command.x;
                      window_changes.y=windows->command.y;
                      XReconfigureWMWindow(display,windows->command.id,
                        windows->command.screen,CWX | CWY,&window_changes);
                    }
                if (windows->magnify.geometry == (char *) NULL)
                  if (!windows->magnify.mapped)
                    {
                      windows->magnify.x=
                        event.xconfigure.x+event.xconfigure.width+25;
                      windows->magnify.y=event.xconfigure.y;
                      XConstrainWindowPosition(display,&windows->magnify);
                      window_changes.x=windows->magnify.x;
                      window_changes.y=windows->magnify.y;
                      XReconfigureWMWindow(display,windows->magnify.id,
                        windows->magnify.screen,CWX | CWY,&window_changes);
                    }
                if (windows->pan.geometry == (char *) NULL)
                  if (!windows->pan.mapped)
                    {
                      windows->pan.x=
                        event.xconfigure.x+event.xconfigure.width+25;
                      windows->pan.y=
                        event.xconfigure.y+windows->magnify.height+50;
                      XConstrainWindowPosition(display,&windows->pan);
                      window_changes.x=windows->pan.x;
                      window_changes.y=windows->pan.y;
                      XReconfigureWMWindow(display,windows->pan.id,
                        windows->pan.screen,CWX | CWY,&window_changes);
                    }
              }
            if ((event.xconfigure.width == windows->image.width) &&
                (event.xconfigure.height == windows->image.height))
              {

                if (windows->image.mapped && windows->image.stasis) {
                  XRefreshWindow(display,&windows->image,(XEvent *) NULL);
                 }
                break;
              }
            windows->image.width=event.xconfigure.width;
            windows->image.height=event.xconfigure.height;
            windows->image.x=0;
            windows->image.y=0;
            if (displayed_image->montage != (char *) NULL)
              {
                windows->image.x=vid_info.x;
                windows->image.y=vid_info.y;
              }
            if (windows->image.mapped && windows->image.stasis)
              {
                /*
                  Update Image window configuration.
                */
                windows->image.window_changes.width=event.xconfigure.width;
                windows->image.window_changes.height=event.xconfigure.height;
                (void) XConfigureImage(display,resource_info,windows,
                  displayed_image);
              }
            if ((event.xconfigure.width < windows->image.ximage->width) ||
                (event.xconfigure.height < windows->image.ximage->height))
              {
                XMapRaised(display,windows->pan.id);
                XDrawPanRectangle(display,windows);
              }
            else
              if (windows->pan.mapped)
                XWithdrawWindow(display,windows->pan.id,windows->pan.screen);
            break;
          }
        if (event.xconfigure.window == windows->magnify.id)
          {
            MonitorHandler
              handler;

            unsigned int
              magnify;

            /*
              Magnify window has a new configuration.
            */
            windows->magnify.width=event.xconfigure.width;
            windows->magnify.height=event.xconfigure.height;
            if (!windows->magnify.mapped)
              break;
            magnify=1;
            while (magnify <= event.xconfigure.width)
              magnify<<=1;
            while (magnify <= event.xconfigure.height)
              magnify<<=1;
            magnify>>=1;
            if ((magnify != event.xconfigure.width) ||
                (magnify != event.xconfigure.height))
              {
                window_changes.width=magnify;
                window_changes.height=magnify;
                XReconfigureWMWindow(display,windows->magnify.id,
                  windows->magnify.screen,CWWidth | CWHeight,&window_changes);
                break;
              }
            handler=SetMonitorHandler((MonitorHandler) NULL);
            status=XMakeImage(display,resource_info,&windows->magnify,
              displayed_image,windows->magnify.width,windows->magnify.height);
            status|=XMakePixmap(display,resource_info,&windows->magnify);
            if (status == False)
              Error("Unable to create magnify image",(char *) NULL);
            XMakeMagnifyImage(display,windows);
            (void) SetMonitorHandler(handler);
            break;
          }
        if (event.xconfigure.window == windows->pan.id)
          {
            /*
              Pan icon window has a new configuration.
            */
            if (event.xconfigure.send_event != 0)
              {
                windows->pan.x=event.xconfigure.x;
                windows->pan.y=event.xconfigure.y;
              }
            windows->pan.width=event.xconfigure.width;
            windows->pan.height=event.xconfigure.height;
            break;
          }
        if (event.xconfigure.window == windows->icon.id)
          {
            /*
              Icon window has a new configuration.
            */
            windows->icon.width=event.xconfigure.width;
            windows->icon.height=event.xconfigure.height;
            break;
          }


        break;
      }
      case DestroyNotify:
      {
        /*
          Group leader has exited.
        */
        if (event.xdestroywindow.window == windows->group_leader.id)
          {
            *state|=ExitState;
            break;
          }
        break;
      }
      case EnterNotify:
      {
        /*
          Selectively install colormap.
        */
        if (map_info->colormap != XDefaultColormap(display,visual_info->screen))
          if (event.xcrossing.mode != NotifyUngrab)
            XInductColormap(display,map_info->colormap);
        break;
      }
      case Expose:
      {

       /* Kobus */
       if (first_time) {
           first_time = False;

#ifdef HOW_IT_WAS
           if (! isatty(fileno(stdout)))
#else
           if (fp_get_path_type(stdout) == PATH_IS_PIPE)
#endif
           {
               /*
               // Untested since switch to test on PIPE.  I have forgotten what,
               // if anything, we still use this for, but I vaguely recall that
               // it was explicitly when we were feeding this into another
               // program.
               */
               UNTESTED_CODE();

               sprintf(text,"\n");
               write(fileno(stdout), text, strlen(text));
               fflush(stdout);
           }
           }
        /* End Kobus */

        if (resource_info->debug)
          (void) fprintf(stderr,"Expose: 0x%lx %dx%d+%d+%d\n",
            event.xexpose.window,event.xexpose.width,event.xexpose.height,
            event.xexpose.x,event.xexpose.y);
        /*
          Refresh windows that are now exposed.
        */
        if (event.xexpose.window == windows->image.id)
          if (windows->image.mapped)
            {

              XRefreshWindow(display,&windows->image,&event);
              timer=time((time_t *) NULL)+resource_info->delay;
              break;
            }
        if (event.xexpose.window == windows->magnify.id)
          if (event.xexpose.count == 0)
            if (windows->magnify.mapped)
              {
                XMakeMagnifyImage(display,windows);
                break;
              }
        if (event.xexpose.window == windows->pan.id)
          if (event.xexpose.count == 0)
            {
              XDrawPanRectangle(display,windows);
              break;
            }
        if (event.xexpose.window == windows->icon.id)
          if (event.xexpose.count == 0)
            {
              XRefreshWindow(display,&windows->icon,&event);
              break;
            }
        break;
      }
      case KeyPress:
      {
        int
          length;

        /*
          Respond to a user key press.
        */
        length=XLookupString((XKeyEvent *) &event.xkey,command,sizeof(command),
          &key_symbol,(XComposeStatus *) NULL);
        *(command+length)='\0';
         if (resource_info->debug)
          (void) fprintf(stderr,"Key press: 0x%lx (%s)\n",key_symbol,command);


        /* Kobus: Hijack various meta's + key. We need a bunch because each OS
        //        and window manager is different.
        */

          /*
           * There is a problem with openwin and X on climb that makes it so
           * that on occasion we get ((XKeyEvent *) &event)->state == 10) no
           * matter what. The same executable always works fine with gnome. A
           * reboot fixes things. Like a restart of the X server would also. The
           * problem likely cannot be fixed here!
           *


         */

         /*
          dbp("----------------------");
          dbx(key_symbol); 
          dbx(((XKeyEvent *) &event)->state);
          dbx(Mod1Mask);
          dbx(Mod2Mask);
          dbx(Mod3Mask);
          dbx(Mod4Mask);
          dbp("----------------------");
          */

#ifdef DEF_OUT
        if (   (((XKeyEvent *) &event)->state & Mod1Mask)
            || (((XKeyEvent *) &event)->state & Mod2Mask)
            || (((XKeyEvent *) &event)->state & Mod3Mask)
            || (((XKeyEvent *) &event)->state & Mod4Mask))
           {
            process_button_three_directive(key_symbol);
            break;
           }
#endif 
        if (   (((XKeyEvent *) &event)->state & 0x8)
            || (((XKeyEvent *) &event)->state & 0x2000)
            /*
            || (((XKeyEvent *) &event)->state & Mod2Mask)
            || (((XKeyEvent *) &event)->state & Mod3Mask)
            || (((XKeyEvent *) &event)->state & Mod4Mask))
            */
           )
           {
               process_button_three_directive(key_symbol);
               break;
           }
        /* End Kobus */

        if (resource_info->immutable)
          {
            if ((key_symbol != XK_q) && (key_symbol != XK_i))
              XBell(display,0);
            else
              (void) XMagickCommand(display,resource_info,windows,0,key_symbol,
                &displayed_image);
            break;
          }
        if (event.xkey.window == windows->image.id)
          loaded_image=XMagickCommand(display,resource_info,windows,
            event.xkey.state,key_symbol,&displayed_image);
        if (event.xkey.window == windows->magnify.id)
          XMagnifyWindowCommand(display,windows,event.xkey.state,key_symbol);
        if (event.xkey.window == windows->pan.id)
          if (key_symbol == XK_q)
            XWithdrawWindow(display,windows->pan.id,windows->pan.screen);
          else
            if ((key_symbol == XK_F1) || (key_symbol == XK_Help))
              XTextViewWidget(display,resource_info,windows,False,
                "Help Viewer - Image Panning",ImagePanHelp);
            else
              XTranslateImage(display,windows,*image,key_symbol);
        timer=time((time_t *) NULL)+resource_info->delay;
        break;
      }
      case KeyRelease:
      {
        /*
          Respond to a user key release.
        */
        (void) XLookupString((XKeyEvent *) &event.xkey,command,sizeof(command),
          &key_symbol,(XComposeStatus *) NULL);
        if (resource_info->debug) 
          (void) fprintf(stderr,"Key release: 0x%lx (%c)\n",key_symbol,
            *command);
        break;
      }
      case LeaveNotify:
      {
        /*
          Selectively uninstall colormap.
        */
        if (map_info->colormap != XDefaultColormap(display,visual_info->screen))
          if (event.xcrossing.mode != NotifyUngrab)
            XUninductColormap(display,map_info->colormap);
        break;
      }
      case MapNotify:
      {
        if (resource_info->debug)
          (void) fprintf(stderr,"Map Notify: 0x%lx\n",event.xmap.window);
        if (event.xmap.window == windows->backdrop.id)
          {
            XSetInputFocus(display,event.xmap.window,RevertToParent,
              CurrentTime);
            windows->backdrop.mapped=True;
            break;
          }
        if (event.xmap.window == windows->image.id)
          {
            if (windows->backdrop.id != (Window) NULL)
              XInstallColormap(display,map_info->colormap);
            if (strcmp(displayed_image->magick,"LOGO") == 0)
              loaded_image=XLoadImage(display,resource_info,windows,False);
            if ((windows->image.width < windows->image.ximage->width) ||
                (windows->image.height < windows->image.ximage->height))
              XMapRaised(display,windows->pan.id);
            windows->image.mapped=True;
            break;
          }
        if (event.xmap.window == windows->magnify.id)
          {
            XMakeMagnifyImage(display,windows);
            windows->magnify.mapped=True;
            XWithdrawWindow(display,windows->info.id,windows->info.screen);
            break;
          }
        if (event.xmap.window == windows->pan.id)
          {
            XMakePanImage(display,resource_info,windows,displayed_image);
            windows->pan.mapped=True;
            break;
          }
        if (event.xmap.window == windows->info.id)
          {
            windows->info.mapped=True;
            break;
          }
        if (event.xmap.window == windows->icon.id)
          {
            /*
              Create an icon image.
            */
            XMakeStandardColormap(display,icon_visual,&icon_resources,
              displayed_image,icon_map,&icon_pixel);
            (void) XMakeImage(display,&icon_resources,&windows->icon,
              displayed_image,windows->icon.width,windows->icon.height);
            (void) XMakePixmap(display,&icon_resources,&windows->icon);
            XSetWindowBackgroundPixmap(display,windows->icon.id,
              windows->icon.pixmap);
            XClearWindow(display,windows->icon.id);
            windows->icon.mapped=True;
            break;
          }
        if (event.xmap.window == windows->command.id)
          {
            windows->command.mapped=True;
            break;
          }
        if (event.xmap.window == windows->popup.id)
          {
            windows->popup.mapped=True;
            break;
          }
        if (event.xmap.window == windows->widget.id)
          {
            windows->widget.mapped=True;
            break;
          }
        break;
      }
      case MappingNotify:
      {
        XRefreshKeyboardMapping(&event.xmapping);
        break;
      }
      case NoExpose:
        break;
      case ReparentNotify:
      {
        if (resource_info->debug)
          (void) fprintf(stderr,"Reparent Notify: 0x%lx=>0x%lx\n",
            event.xreparent.parent,event.xreparent.window);
        break;
      }
      case UnmapNotify:
      {
        if (resource_info->debug)
          (void) fprintf(stderr,"Unmap Notify: 0x%lx\n",event.xunmap.window);
        if (event.xunmap.window == windows->backdrop.id)
          {
            windows->backdrop.mapped=False;
            break;
          }
        if (event.xunmap.window == windows->image.id)
          {
            windows->image.mapped=False;
            break;
          }
        if (event.xunmap.window == windows->magnify.id)
          {
            windows->magnify.mapped=False;
            break;
          }
        if (event.xunmap.window == windows->pan.id)
          {
            windows->pan.mapped=False;
            break;
          }
        if (event.xunmap.window == windows->info.id)
          {
            windows->info.mapped=False;
            break;
          }
        if (event.xunmap.window == windows->icon.id)
          {
            if (map_info->colormap == icon_map->colormap)
              XConfigureImageColormap(display,resource_info,windows,
                displayed_image);
            XFreeStandardColormap(display,icon_visual,icon_map,&icon_pixel);
            windows->icon.mapped=False;
            break;
          }
        if (event.xunmap.window == windows->command.id)
          {
            windows->command.mapped=False;
            break;
          }
        if (event.xunmap.window == windows->popup.id)
          {
            if (windows->backdrop.id != (Window) NULL)
              XSetInputFocus(display,windows->image.id,RevertToParent,
                CurrentTime);
            windows->popup.mapped=False;
            break;
          }
        if (event.xunmap.window == windows->widget.id)
          {
            if (windows->backdrop.id != (Window) NULL)
              XSetInputFocus(display,windows->image.id,RevertToParent,
                CurrentTime);
            windows->widget.mapped=False;
            break;
          }
        break;
      }
      default:
      {
        if (resource_info->debug)
          (void) fprintf(stderr,"Event type: %d\n",event.type);
        break;
      }
    }

   /* Kobus */
   if (fs_image_modified)
   {
#ifdef HOW_IT_WAS
        if (! isatty(fileno(stdout)))
#else
        if (fp_get_path_type(stdout) == PATH_IS_PIPE)
#endif
        {
            /*
            // Untested since switch to test on PIPE.
            // I have forgotten what, if anything, we still
            // use this for, but I vaguely recall that it was
            // explicitly when we were feeding this into
            // another program.
            */
            UNTESTED_CODE();

            sprintf(text,"%d %d\n", EOF, EOF);
            write(fileno(stdout), text, strlen(text));
            fflush(stdout);
        }

        must_update_pure_pixels = True;
        fs_image_modified = False;
       }
   /* End Kobus */

  }
  while (!(*state & ExitState));
  if ((*state & FormerImageState) || (*state & NextImageState))
    *state&=(~ExitState);
  /*
    Withdraw pan and Magnify window.
  */
  if (windows->pan.mapped)
    XWithdrawWindow(display,windows->pan.id,windows->pan.screen);
  if (windows->magnify.mapped)
    XWithdrawWindow(display,windows->magnify.id,windows->magnify.screen);
  if (windows->command.mapped)
    XWithdrawWindow(display,windows->command.id,windows->command.screen);
  if (!resource_info->backdrop)
    if (windows->backdrop.mapped)
      {
        XWithdrawWindow(display,windows->backdrop.id,windows->backdrop.screen);
        XDestroyWindow(display,windows->backdrop.id);
        windows->backdrop.id=(Window) NULL;
        XWithdrawWindow(display,windows->image.id,windows->image.screen);
        XDestroyWindow(display,windows->image.id);
        windows->image.id=(Window) NULL;
      }
  XSetCursorState(display,windows,True);
  XCheckRefreshWindows(display,windows);
  if (!(*state & ExitState))
    {
      (void) XMagickCommand(display,resource_info,windows,0,(KeySym) XK_Break,
        &displayed_image);
      if (resource_info->write_filename != (char *) NULL)
        (void) XMagickCommand(display,resource_info,windows,0,(KeySym) XK_A,
          &displayed_image);


      /* Kobus */
      fs_image_modified = True;
      /* End Kobus */
    }
  else
    {
      /*
        Destroy X windows.
      */
      for (i=0; i < number_windows; i++)
      {
        if (magick_windows[i]->id != (Window) NULL)
          XDestroyWindow(display,magick_windows[i]->id);
        if (magick_windows[i]->ximage != (XImage *) NULL)
          XDestroyImage(magick_windows[i]->ximage);
        if (magick_windows[i]->pixmap != (Pixmap) NULL)
          XFreePixmap(display,magick_windows[i]->pixmap);
      }
      /*
        Free Standard Colormap.
      */
      if (resource_info->map_type == (char *) NULL)
        XFreeStandardColormap(display,visual_info,map_info,&pixel_info);
      XFree((void *) class_hints);
      XFree((void *) manager_hints);
      XFree((void *) icon_visual);
      XFree((void *) visual_info);
      XFree((void *) icon_map);
      XFree((void *) map_info);
      free((char *) windows->popup.name);
      free((char *) windows->widget.name);
      free((char *) windows->magnify.name);
      free((char *) windows->image.icon_name);
      free((char *) windows->image.name);
      free((char *) windows);
      visual_info=(XVisualInfo *) NULL;
      if (copy_image != (Image *) NULL)
        DestroyImage(copy_image);
    }
  /*
    Change to home directory.
  */
  (void) getcwd(working_directory,MaxTextLength-1);
  (void) chdir(resource_info->home_directory);
  *image=displayed_image;
  return(loaded_image);
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
%  Function XDrawEditImage draws a graphic primitive (point, line, rectangle,
%  etc.) on the image.
%
%  The format of the XDrawEditImage routine is:
%
%    status=XDrawEditImage(display,resource_info,windows,degrees,image)
%
%  A description of each parameter follows:
%
%    o status: Function XDrawEditImage return True if the image is drawn
%      upon.  False is returned is there is a memory shortage or if the
%      image cannot be drawn on.
%
%    o display: Specifies a connection to an X server; returned from
%      XOpenDisplay.
%
%    o resource_info: Specifies a pointer to a X11 XResourceInfo structure.
%
%    o windows: Specifies a pointer to a XWindows structure.
%
%    o image: Specifies a pointer to a Image structure;  returned from
%      ReadImage.
%
%
*/
static unsigned int XDrawEditImage
(
    Display*       display,
    XResourceInfo* resource_info,
    XWindows*      windows,
    Image**        image
)
{
#define DrawModePrimitiveOp  0
#define DrawModeColorOp  1
#define DrawModeStippleOp  2
#define DrawModeWidthOp  3
#define DrawModeUndoOp  4
#define DrawModeHelpOp  5
#define DrawModeDismissOp  6
#define Swap(x,y) ((x)^=(y), (y)^=(x), (x)^=(y))

  static char
    *DrawModeMenu[]=
    {
      "Primitive",
      "Color",
      "Stipple",
      "Width",
      "Undo",
      "Help",
      "Dismiss",
      (char *) NULL
    };

  char
    command[MaxTextLength],
    text[MaxTextLength];

  Cursor
    cursor;

  double
    degrees;

  int
    entry,
    id,
    number_coordinates,
    x,
    y;

  RectangleInfo
    rectangle_info;

  register int
    i;

  static Pixmap
    stipple = (Pixmap) NULL;

  static unsigned int
    pen_id = 0,
    primitive = PointPrimitive,
    line_width = 1;

  unsigned int
    distance,
    height,
    max_coordinates,
    status,
    width;

  unsigned long
    state,
    x_factor,
    y_factor;

  Window
    root_window;

  XDrawInfo
    draw_info;

  XEvent
    event;

  XPoint
    *coordinate_info;

  XSegment
    line_info;

  /*
    Allocate polygon info.
  */
  max_coordinates=2048;
  coordinate_info=(XPoint *) malloc(max_coordinates*sizeof(XPoint));
  if (coordinate_info == (XPoint *) NULL)
    {
      Warning("Unable to draw on image","Memory allocation failed");
      return(False);
    }
  /*
    Map Command widget.
  */
  windows->command.name="Draw";
  windows->command.data=4;
  (void) XCommandWidget(display,windows,DrawModeMenu,(XEvent *) NULL);
  XMapRaised(display,windows->command.id);
  XClientMessage(display,windows->image.id,windows->im_protocols,
    windows->im_update_widget,CurrentTime);
  /*
    Wait for first button press.
  */
  root_window=XRootWindow(display,XDefaultScreen(display));
  draw_info.stencil=OpaqueStencil;
  status=True;
  cursor=XCreateFontCursor(display,XC_tcross);
  for ( ; ; )
  {
    XQueryPosition(display,windows->image.id,&x,&y);
    XDefineCursor(display,windows->image.id,cursor);
    XSelectInput(display,windows->image.id,
      windows->image.attributes.event_mask | PointerMotionMask);
    state=DefaultState;
    do
    {
      if (windows->info.mapped)
        {
          /*
            Display pointer position.
          */
          (void) sprintf(text," %+d%+d ",x-windows->image.x,y-windows->image.y);
          XInfoWidget(display,windows,text);
        }
      /*
        Wait for next event.
      */
      XIfEvent(display,&event,XScreenEvent,(char *) windows);
      if (event.xany.window == windows->command.id)
        {
          /*
            Select a command from the Command widget.
          */
          id=XCommandWidget(display,windows,DrawModeMenu,&event);
          if (id < 0)
            continue;
          switch (id)
          {
            case DrawModePrimitiveOp:
            {
              static char
                *Primitives[]=
                {
                  "point",
                  "line",
                  "rectangle",
                  "fill rectangle",
                  "ellipse",
                  "fill ellipse",
                  "polygon",
                  "fill polygon",
                  (char *) NULL,
                };

              /*
                Select a command from the pop-up menu.
              */
              primitive=XMenuWidget(display,windows,DrawModeMenu[id],Primitives,
                command)+1;
              break;
            }
            case DrawModeColorOp:
            {
              char
                *ColorMenu[MaxNumberPens+1];

              int
                pen_number;

              unsigned int
                transparent;

              XColor
                color;

              /*
                Initialize menu selections.
              */
              for (i=0; i < (MaxNumberPens-2); i++)
                ColorMenu[i]=resource_info->pen_colors[i];
              ColorMenu[MaxNumberPens-2]="transparent";
              ColorMenu[MaxNumberPens-1]="Browser...";
              ColorMenu[MaxNumberPens]=(char *) NULL;
              /*
                Select a pen color from the pop-up menu.
              */
              pen_number=XMenuWidget(display,windows,DrawModeMenu[id],ColorMenu,
                command);
              if (pen_number < 0)
                break;
              transparent=pen_number == (MaxNumberPens-2);
              if (transparent)
                {
                  draw_info.stencil=TransparentStencil;
                  break;
                }
              if (pen_number == (MaxNumberPens-1))
                {
                  static char
                    color_name[MaxTextLength] = "gray";

                  /*
                    Select a pen color from a dialog.
                  */
                  resource_info->pen_colors[pen_number]=color_name;
                  XColorBrowserWidget(display,windows,"Select",color_name);
                  if (*color_name == '\0')
                    break;
                }
              /*
                Set pen color.
              */
              (void) XParseColor(display,windows->image.map_info->colormap,
                resource_info->pen_colors[pen_number],&color);
              XBestPixel(display,windows->image.map_info->colormap,
                (XColor *) NULL,(unsigned int) MaxColors,&color);
              windows->image.pixel_info->pen_colors[pen_number]=color;
              pen_id=pen_number;
              draw_info.stencil=OpaqueStencil;
              break;
            }
            case DrawModeStippleOp:
            {
              static char
                filename[MaxTextLength] = "\0",
                *StipplesMenu[]=
                {
                  "Brick",
                  "Diagonal",
                  "Scales",
                  "Vertical",
                  "Wavy",
                  "Translucent",
                  "Opaque",
                  (char *) NULL,
                  (char *) NULL,
                };

              /*
                Select a command from the pop-up menu.
              */
              StipplesMenu[7]="Load...";
              entry=XMenuWidget(display,windows,DrawModeMenu[id],StipplesMenu,
                command);
              if (entry < 0)
                break;
              if (stipple != (Pixmap) NULL)
                XFreePixmap(display,stipple);
              stipple=(Pixmap) NULL;
              if (entry == 6)
                break;
              if (entry != 7)
                {
                  switch (entry)
                  {
                    case 0:
                    {
                      stipple=XCreateBitmapFromData(display,root_window,
                        (char *) BricksBitmap,BricksWidth,BricksHeight);
                      break;
                    }
                    case 1:
                    {
                      stipple=XCreateBitmapFromData(display,root_window,
                        (char *) DiagonalBitmap,DiagonalWidth,DiagonalHeight);
                      break;
                    }
                    case 2:
                    {
                      stipple=XCreateBitmapFromData(display,root_window,
                        (char *) ScalesBitmap,ScalesWidth,ScalesHeight);
                      break;
                    }
                    case 3:
                    {
                      stipple=XCreateBitmapFromData(display,root_window,
                        (char *) VerticalBitmap,VerticalWidth,VerticalHeight);
                      break;
                    }
                    case 4:
                    {
                      stipple=XCreateBitmapFromData(display,root_window,
                        (char *) WavyBitmap,WavyWidth,WavyHeight);
                      break;
                    }
                    case 5:
                    default:
                    {
                      stipple=XCreateBitmapFromData(display,root_window,
                        (char *) HighlightBitmap,HighlightWidth,
                        HighlightHeight);
                      break;
                    }
                  }
                  break;
                }
              XFileBrowserWidget(display,windows,"Stipple",filename);
              if (*filename == '\0')
                break;
              status=XReadBitmapFile(display,root_window,filename,&width,
                &height,&stipple,&x,&y);
              if (status != BitmapSuccess)
                XNoticeWidget(display,windows,"Unable to read X bitmap image:",
                  filename);
              break;
            }
            case DrawModeWidthOp:
            {
              static char
                width[MaxTextLength] = "3",
                *WidthsMenu[]=
                {
                  "1",
                  "2",
                  "4",
                  "8",
                  "16",
                  (char *) NULL,
                  (char *) NULL,
                };

              /*
                Select a command from the pop-up menu.
              */
              WidthsMenu[5]="Dialog...";
              entry=XMenuWidget(display,windows,DrawModeMenu[id],WidthsMenu,
                command);
              if (entry < 0)
                break;
              if (entry != 5)
                {
                  line_width=atoi(WidthsMenu[entry]);
                  break;
                }
              (void) XDialogWidget(display,windows,"Ok","Enter line width:",
                width);
              if (*width == '\0')
                break;
              line_width=atoi(width);
              break;
            }
            case DrawModeUndoOp:
            {
              (void) XMagickCommand(display,resource_info,windows,0,
                (KeySym) XK_u,image);
              break;
            }
            case DrawModeHelpOp:
            {
              XTextViewWidget(display,resource_info,windows,False,
                "Help Viewer - Image Rotation",ImageDrawHelp);
              XDefineCursor(display,windows->image.id,cursor);
              break;
            }
            case DrawModeDismissOp:
            {
              /*
                Prematurely exit.
              */
              state|=EscapeState;
              state|=ExitState;
              break;
            }
            default:
              break;
          }
          XDefineCursor(display,windows->image.id,cursor);
          continue;
        }
      switch (event.type)
      {
        case ButtonPress:
        {
          if (event.xbutton.window == windows->pan.id)
            {
              XPanImage(display,windows,&event);
              XInfoWidget(display,windows,text);
              break;
            }
          /*
            Exit loop.
          */
          x=event.xbutton.x;
          y=event.xbutton.y;
          state|=ExitState;
          break;
        }
        case ButtonRelease:
          break;
        case Expose:
          break;
        case KeyPress:
        {
          char
            command[MaxTextLength];

          KeySym
            key_symbol;

          if (event.xkey.window != windows->image.id)
            break;
          /*
            Respond to a user key press.
          */
          (void) XLookupString((XKeyEvent *) &event.xkey,command,
            sizeof(command),&key_symbol,(XComposeStatus *) NULL);
          switch (key_symbol)
          {
            case XK_Escape:
            case XK_F20:
            {
              /*
                Prematurely exit.
              */
              state|=EscapeState;
              state|=ExitState;
              break;
            }
            case XK_F1:
            case XK_Help:
            {
              XTextViewWidget(display,resource_info,windows,False,
                "Help Viewer - Image Rotation",ImageDrawHelp);
              break;
            }
            default:
            {
              XBell(display,0);
              break;
            }
          }
          break;
        }
        case MotionNotify:
        {
          /*
            Discard pending pointer motion events.
          */
          while (XCheckMaskEvent(display,PointerMotionMask,&event));
          x=event.xmotion.x;
          y=event.xmotion.y;
          /*
            Map and unmap Info widget as text cursor crosses its boundaries.
          */
          if (windows->info.mapped)
            {
              if ((x < (windows->info.x+windows->info.width)) &&
                  (y < (windows->info.y+windows->info.height)))
                XWithdrawWindow(display,windows->info.id,windows->info.screen);
            }
          else
            if ((x > (windows->info.x+windows->info.width)) ||
                (y > (windows->info.y+windows->info.height)))
              XMapWindow(display,windows->info.id);
          break;
        }
      }
    } while (!(state & ExitState));
    XSelectInput(display,windows->image.id,
      windows->image.attributes.event_mask);
    XWithdrawWindow(display,windows->info.id,windows->info.screen);
    if (state & EscapeState)
      break;
    /*
      Draw primitive as pointer moves until the button is released.
    */
    distance=0;
    degrees=0.0;
    line_info.x1=x;
    line_info.y1=y;
    line_info.x2=x;
    line_info.y2=y;
    rectangle_info.x=x;
    rectangle_info.y=y;
    rectangle_info.width=0;
    rectangle_info.height=0;
    number_coordinates=1;
    coordinate_info->x=x;
    coordinate_info->y=y;
    XSetFunction(display,windows->image.highlight_context,GXinvert);
    state=DefaultState;
    do
    {
      switch (primitive)
      {
        case PointPrimitive:
        default:
        {
          if (number_coordinates > 1)
            {
              XDrawLines(display,windows->image.id,
                windows->image.highlight_context,coordinate_info,
                number_coordinates,CoordModeOrigin);
              (void) sprintf(text," %+d%+d",
                coordinate_info[number_coordinates-1].x,
                coordinate_info[number_coordinates-1].y);
              XInfoWidget(display,windows,text);
            }
          break;
        }
        case LinePrimitive:
        {
          if (distance > 9)
            {
              /*
                Display angle of the line.
              */
              degrees=RadiansToDegrees(-atan2((double) (line_info.y2-
                line_info.y1),(double) (line_info.x2-line_info.x1)));
              (void) sprintf(text," %.2f",degrees);
              XInfoWidget(display,windows,text);
              XHighlightLine(display,windows->image.id,
                windows->image.highlight_context,&line_info);
            }
          else
            if (windows->info.mapped)
              XWithdrawWindow(display,windows->info.id,windows->info.screen);
          break;
        }
        case RectanglePrimitive:
        case FillRectanglePrimitive:
        {
          if ((rectangle_info.width > 3) && (rectangle_info.height > 3))
            {
              /*
                Display info and draw drawing rectangle.
              */
              (void) sprintf(text," %ux%u%+d%+d",rectangle_info.width,
                rectangle_info.height,rectangle_info.x,rectangle_info.y);
              XInfoWidget(display,windows,text);
              XHighlightRectangle(display,windows->image.id,
                windows->image.highlight_context,&rectangle_info);
            }
          else
            if (windows->info.mapped)
              XWithdrawWindow(display,windows->info.id,windows->info.screen);
          break;
        }
        case EllipsePrimitive:
        case FillEllipsePrimitive:
        {
          if ((rectangle_info.width > 3) && (rectangle_info.height > 3))
            {
              /*
                Display info and draw drawing rectangle.
              */
              (void) sprintf(text," %ux%u%+d%+d",rectangle_info.width,
                rectangle_info.height,rectangle_info.x,rectangle_info.y);
              XInfoWidget(display,windows,text);
              XHighlightEllipse(display,windows->image.id,
                windows->image.highlight_context,&rectangle_info);
            }
          else
            if (windows->info.mapped)
              XWithdrawWindow(display,windows->info.id,windows->info.screen);
          break;
        }
        case PolygonPrimitive:
        case FillPolygonPrimitive:
        {
          if (number_coordinates > 1)
            XDrawLines(display,windows->image.id,
              windows->image.highlight_context,coordinate_info,
              number_coordinates,CoordModeOrigin);
          if (distance > 9)
            {
              /*
                Display angle of the line.
              */
              degrees=RadiansToDegrees(-atan2((double) (line_info.y2-
                line_info.y1),(double) (line_info.x2-line_info.x1)));
              (void) sprintf(text," %.2f",degrees);
              XInfoWidget(display,windows,text);
              XHighlightLine(display,windows->image.id,
                windows->image.highlight_context,&line_info);
            }
          else
            if (windows->info.mapped)
              XWithdrawWindow(display,windows->info.id,windows->info.screen);
          break;
        }
      }
      /*
        Wait for next event.
      */
      XIfEvent(display,&event,XScreenEvent,(char *) windows);
      switch (primitive)
      {
        case PointPrimitive:
        default:
        {
          if (number_coordinates > 1)
            XDrawLines(display,windows->image.id,
              windows->image.highlight_context,coordinate_info,
              number_coordinates,CoordModeOrigin);
          break;
        }
        case LinePrimitive:
        {
          if (distance > 9)
            XHighlightLine(display,windows->image.id,
              windows->image.highlight_context,&line_info);
          break;
        }
        case RectanglePrimitive:
        case FillRectanglePrimitive:
        {
          if ((rectangle_info.width > 3) && (rectangle_info.height > 3))
            XHighlightRectangle(display,windows->image.id,
              windows->image.highlight_context,&rectangle_info);
          break;
        }
        case EllipsePrimitive:
        case FillEllipsePrimitive:
        {
          if ((rectangle_info.width > 3) && (rectangle_info.height > 3))
            XHighlightEllipse(display,windows->image.id,
              windows->image.highlight_context,&rectangle_info);
          break;
        }
        case PolygonPrimitive:
        case FillPolygonPrimitive:
        {
          if (number_coordinates > 1)
            XDrawLines(display,windows->image.id,
              windows->image.highlight_context,coordinate_info,
              number_coordinates,CoordModeOrigin);
          if (distance > 9)
            XHighlightLine(display,windows->image.id,
              windows->image.highlight_context,&line_info);
          break;
        }
      }
      switch (event.type)
      {
        case ButtonPress:
          break;
        case ButtonRelease:
        {
          /*
            User has committed to primitive.
          */
          line_info.x2=event.xbutton.x;
          line_info.y2=event.xbutton.y;
          rectangle_info.x=event.xbutton.x;
          rectangle_info.y=event.xbutton.y;
          coordinate_info[number_coordinates].x=event.xbutton.x;
          coordinate_info[number_coordinates].y=event.xbutton.y;
          if (((primitive != PolygonPrimitive) &&
               (primitive != FillPolygonPrimitive)) || (distance <= 9))
            {
              state|=ExitState;
              break;
            }
          number_coordinates++;
          if (number_coordinates < max_coordinates)
            {
              line_info.x1=event.xbutton.x;
              line_info.y1=event.xbutton.y;
              break;
            }
          max_coordinates<<=1;

          {
              XPoint* tmp = (XPoint *)
                realloc(coordinate_info,max_coordinates*sizeof(XPoint));
              if (tmp == (XPoint *) NULL)
                Warning("Unable to draw on image","Memory allocation failed");
              else
                coordinate_info = tmp;
          }
          break;
        }
        case Expose:
          break;
        case MotionNotify:
        {
          /*
            Discard pending button motion events.
          */
          if (event.xmotion.window != windows->image.id)
            break;
          if (primitive != PointPrimitive)
            {
              while (XCheckMaskEvent(display,ButtonMotionMask,&event));
              line_info.x2=event.xmotion.x;
              line_info.y2=event.xmotion.y;
              rectangle_info.x=event.xmotion.x;
              rectangle_info.y=event.xmotion.y;
              break;
            }
          coordinate_info[number_coordinates].x=event.xbutton.x;
          coordinate_info[number_coordinates].y=event.xbutton.y;
          number_coordinates++;
          if (number_coordinates < max_coordinates)
            break;
          max_coordinates<<=1;
          {
              XPoint* tmp=(XPoint *)
                realloc(coordinate_info,max_coordinates*sizeof(XPoint));
              if (tmp == (XPoint *) NULL)
                Warning("Unable to draw on image","Memory allocation failed");
              else
                coordinate_info = tmp;
          }
          break;
        }
        default:
          break;
      }
      /*
        Check boundary conditions.
      */
      if (line_info.x2 < 0)
        line_info.x2=0;
      else
        if (line_info.x2 > windows->image.width)
          line_info.x2=windows->image.width;
      if (line_info.y2 < 0)
        line_info.y2=0;
      else
        if (line_info.y2 > windows->image.height)
          line_info.y2=windows->image.height;
      distance=
        ((line_info.x2-line_info.x1+1)*(line_info.x2-line_info.x1+1))+
        ((line_info.y2-line_info.y1+1)*(line_info.y2-line_info.y1+1));
      if (((rectangle_info.x != x) && (rectangle_info.y != y)) ||
          (state & ExitState))
        {
          if (rectangle_info.x < 0)
            rectangle_info.x=0;
          else
            if (rectangle_info.x > windows->image.width)
              rectangle_info.x=windows->image.width;
          if (rectangle_info.x < x)
            rectangle_info.width=(unsigned int) (x-rectangle_info.x);
          else
            {
              rectangle_info.width=(unsigned int) (rectangle_info.x-x);
              rectangle_info.x=x;
            }
          if (rectangle_info.y < 0)
            rectangle_info.y=0;
          else
            if (rectangle_info.y > windows->image.height)
              rectangle_info.y=windows->image.height;
          if (rectangle_info.y < y)
            rectangle_info.height=(unsigned int) (y-rectangle_info.y);
          else
            {
              rectangle_info.height=(unsigned int) (rectangle_info.y-y);
              rectangle_info.y=y;
            }
        }
    } while (!(state & ExitState));
    XSetFunction(display,windows->image.highlight_context,GXcopy);
    if ((primitive == PointPrimitive) || (primitive == PolygonPrimitive) ||
        (primitive == FillPolygonPrimitive))
      {
        /*
          Determine polygon bounding box.
        */
        rectangle_info.x=coordinate_info->x;
        rectangle_info.y=coordinate_info->y;
        x=coordinate_info->x;
        y=coordinate_info->y;
        for (i=1; i < number_coordinates; i++)
        {
          if (coordinate_info[i].x > x)
            x=coordinate_info[i].x;
          if (coordinate_info[i].y > y)
            y=coordinate_info[i].y;
          if (coordinate_info[i].x < rectangle_info.x)
            rectangle_info.x=Max(coordinate_info[i].x,0);
          if (coordinate_info[i].y < rectangle_info.y)
            rectangle_info.y=Max(coordinate_info[i].y,0);
        }
        rectangle_info.width=x-rectangle_info.x;
        rectangle_info.height=y-rectangle_info.y;
        for (i=0; i < number_coordinates; i++)
        {
          coordinate_info[i].x-=rectangle_info.x;
          coordinate_info[i].y-=rectangle_info.y;
        }
      }
    else
      if (distance <= 9)
        continue;
      else
        if ((primitive == RectanglePrimitive) ||
            (primitive == EllipsePrimitive))
          {
            rectangle_info.width--;
            rectangle_info.height--;
          }
    /*
      Drawing is relative to image configuration.
    */
    draw_info.x=rectangle_info.x;
    draw_info.y=rectangle_info.y;
    (void) XMagickCommand(display,resource_info,windows,0,(KeySym) XK_Select,
      image);
    x=0;
    y=0;
    width=(*image)->columns;
    height=(*image)->rows;
    if (windows->image.crop_geometry != (char *) NULL)
      (void) XParseGeometry(windows->image.crop_geometry,&x,&y,&width,&height);
    x_factor=UpShift(width)/windows->image.ximage->width;
    draw_info.x+=windows->image.x-(line_width >> 1);
    if (draw_info.x < 0)
      draw_info.x=0;
    draw_info.x=DownShift(draw_info.x*x_factor);
    y_factor=UpShift(height)/windows->image.ximage->height;
    draw_info.y+=windows->image.y-(line_width >> 1);
    if (draw_info.y < 0)
      draw_info.y=0;
    draw_info.y=DownShift(draw_info.y*y_factor);
    draw_info.width=rectangle_info.width+(line_width << 1);
    if (draw_info.width > (*image)->columns)
      draw_info.width=(*image)->columns;
    draw_info.height=rectangle_info.height+(line_width << 1);
    if (draw_info.height > (*image)->rows)
      draw_info.height=(*image)->rows;
    (void) sprintf(draw_info.geometry,"%ux%u%+d%+d",
      (unsigned int) DownShift(draw_info.width*x_factor),
      (unsigned int) DownShift(draw_info.height*y_factor),
      draw_info.x+x,draw_info.y+y);
    /*
      Initialize drawing attributes.
    */
    draw_info.degrees=0.0;
    draw_info.primitive=primitive;
    draw_info.stipple=stipple;
    draw_info.line_width=line_width;
    draw_info.line_info=line_info;
    if (line_info.x1 > (line_width >> 1))
      draw_info.line_info.x1=line_width >> 1;
    if (line_info.y1 > (line_width >> 1))
      draw_info.line_info.y1=line_width >> 1;
    draw_info.line_info.x2=line_info.x2-line_info.x1+(line_width >> 1);
    draw_info.line_info.y2=line_info.y2-line_info.y1+(line_width >> 1);
    if ((draw_info.line_info.x2 < 0) && (draw_info.line_info.y2 < 0))
      {
        draw_info.line_info.x2=(-draw_info.line_info.x2);
        draw_info.line_info.y2=(-draw_info.line_info.y2);
      }
    if (draw_info.line_info.x2 < 0)
      {
        draw_info.line_info.x2=(-draw_info.line_info.x2);
        Swap(draw_info.line_info.x1,draw_info.line_info.x2);
      }
    if (draw_info.line_info.y2 < 0)
      {
        draw_info.line_info.y2=(-draw_info.line_info.y2);
        Swap(draw_info.line_info.y1,draw_info.line_info.y2);
      }
    draw_info.rectangle_info=rectangle_info;
    if (draw_info.rectangle_info.x > (line_width >> 1))
      draw_info.rectangle_info.x=line_width >> 1;
    if (draw_info.rectangle_info.y > (line_width >> 1))
      draw_info.rectangle_info.y=line_width >> 1;
    draw_info.number_coordinates=number_coordinates;
    draw_info.coordinate_info=coordinate_info;
    windows->image.pixel_info->pen_color=
      windows->image.pixel_info->pen_colors[pen_id];
    /*
      Draw primitive on image.
    */
    XSetCursorState(display,windows,True);
    XCheckRefreshWindows(display,windows);
    status=XDrawImage(display,windows->image.pixel_info,&draw_info,*image);
    XSetCursorState(display,windows,False);
    /*
      Update image colormap and return to image drawing.
    */
    XConfigureImageColormap(display,resource_info,windows,*image);
    (void) XConfigureImage(display,resource_info,windows,*image);
  }
  XSetCursorState(display,windows,False);
  free((char *) coordinate_info);
  return(status);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   X D r a w P a n R e c t a n g l e                                         %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Function XDrawPanRectangle draws a rectangle in the pan window.  The pan
%  window displays a zoomed image and the rectangle shows which portion of
%  the image is displayed in the Image window.
%
%  The format of the XDrawPanRectangle routine is:
%
%    XDrawPanRectangle(display,windows)
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
static void XDrawPanRectangle(Display* display, XWindows* windows)
{
  unsigned long
    scale_factor;

  RectangleInfo
    highlight_info;

  /*
    Determine dimensions of the panning rectangle.
  */
  scale_factor=(unsigned long)
    (UpShift(windows->pan.width)/windows->image.ximage->width);
  highlight_info.x=DownShift(windows->image.x*scale_factor);
  highlight_info.width=DownShift(windows->image.width*scale_factor);
  scale_factor=(unsigned long)
    (UpShift(windows->pan.height)/windows->image.ximage->height);
  highlight_info.y=DownShift(windows->image.y*scale_factor);
  highlight_info.height=DownShift(windows->image.height*scale_factor);
  /*
    Display the panning rectangle.
  */
  XClearWindow(display,windows->pan.id);
  XHighlightRectangle(display,windows->pan.id,windows->pan.annotate_context,
    &highlight_info);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   X L o a d I m a g e                                                       %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Function XLoadImage loads an image from a file.
%
%  The format of the XLoadImage routine is:
%
%    loaded_image=XLoadImage(display,resource_info,windows,command)
%
%  A description of each parameter follows:
%
%    o loaded_image: Function XLoadImage returns an image if can be loaded
%      successfully.  Otherwise a null image is returned.
%
%    o display: Specifies a connection to an X server; returned from
%      XOpenDisplay.
%
%    o resource_info: Specifies a pointer to a X11 XResourceInfo structure.
%
%    o windows: Specifies a pointer to a XWindows structure.
%
%    o command: A value other than zero indicates that the file is selected
%      from the command line argument list.
%
%
*/
static Image* XLoadImage
(
    Display*       display,
    XResourceInfo* resource_info,
    XWindows*      windows,
    unsigned int   command
)
{

  /* Kobus */
  extern int fs_image_modified;
  /* End Kobus */

  Image
    *loaded_image;

  ImageInfo
    image_info;

  static char
    filename[MaxTextLength] = "\0";

  /*
    Request file name from user.
  */
  if (!command)
    XFileBrowserWidget(display,windows,"Load",filename);
  else
    {
      char
        **filelist,
        **files;

      int
        count,
        status;

      register int
        i,
        j;

      /*
        Select next image from the command line.
      */
      status=XGetCommand(display,windows->image.id,&files,&count);
      if (!status)
        {
          Warning("Unable to select image","XGetCommand failed");
          return((Image *) NULL);
        }
      filelist=(char **) malloc(count*sizeof(char *));
      if (filelist == (char **) NULL)
        {
          Warning("Unable to select image","Memory allocation failed");
          XFreeStringList(files);
          return((Image *) NULL);
        }
      j=0;
      for (i=1; i < count; i++)
        if (*files[i] != '-')
          filelist[j++]=files[i];
      filelist[j]=(char *) NULL;
      XListBrowserWidget(display,windows,&windows->widget,filelist,"Load",
        "Select Image to Load:",filename);
      free((char *) filelist);
      XFreeStringList(files);
    }
  if (*filename == '\0')
    return((Image *) NULL);
  GetImageInfo(&image_info);
  (void) strcpy(image_info.filename,filename);
  SetImageMagick(&image_info);
  if (strcmp(image_info.magick,"X") == 0)
    {
      char
        seconds[MaxTextLength];

      /*
        User may want to delay the X server screen grab.
      */
      (void) strcpy(seconds,"0");
      (void) XDialogWidget(display,windows,"Grab","Enter any delay in seconds:",
        seconds);
      XDelay(display,1000*atoi(seconds));
    }
  /*
    Load the image.
  */

  /* Kobus */
  fs_image_modified = True;
  /* End Kobus */

  XSetCursorState(display,windows,True);
  XCheckRefreshWindows(display,windows);
  (void) strcpy(resource_info->image_info->filename,filename);
  loaded_image=ReadImage(resource_info->image_info);
  XSetCursorState(display,windows,False);
  if (loaded_image != (Image *) NULL)
    XClientMessage(display,windows->image.id,windows->im_protocols,
      windows->im_next_image,CurrentTime);
  else
    {
      char
        *text,
        **textlist;

      FILE
        *file;

      int
        c;

      register char
        *p;

      unsigned int
        length;

      /*
        Unknown image format.
      */
      file=(FILE *) fopen(filename,"r");
      if (file == (FILE *) NULL)
        return((Image *) NULL);
      length=MaxTextLength;
      text=(char *) malloc(length*sizeof(char));
      for (p=text ; text != (char *) NULL; p++)
      {
        c=fgetc(file);
        if (c == EOF)
          break;
        if ((p-text+1) >= length)
          {
            *p='\0';
            length<<=1;

            {
              char* text_tmp =(char *) realloc((char *) text,length*sizeof(char));
              if (text_tmp == (char *) NULL)
                break;
              else
                text = text_tmp;
            }
            p=text+strlen(text);
          }
        *p=(unsigned char) c;
      }
      (void) fclose(file);
      if (text == (char *) NULL)
        return((Image *) NULL);
      *p='\0';
      textlist=StringToList(text);
      if (textlist != (char **) NULL)
        {
          char
            title[MaxTextLength];

          register int
            i;

          (void) sprintf(title,"Unknown format: %s",filename);
          XTextViewWidget(display,resource_info,windows,True,title,textlist);
          for (i=0; textlist[i] != (char *) NULL; i++)
            free((char *) textlist[i]);
          free((char *) textlist);
        }
      free((char *) text);
    }
  return(loaded_image);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   X M a g i c k C o m m a n d                                               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Function XMagickCommand makes a transform to the image or Image window
%  as specified by a user menu button or keyboard command.
%
%  The format of the XMagickCommand routine is:
%
%    loaded_image=XMagickCommand(display,resource_info,windows,state,
%      key_symbol,image)
%
%  A description of each parameter follows:
%
%    o loaded_image:  Function XMagickCommand returns an image when the
%      user chooses 'Load Image' from the command menu.  Otherwise a null
%      image is returned.
%
%    o display: Specifies a connection to an X server; returned from
%      XOpenDisplay.
%
%    o resource_info: Specifies a pointer to a X11 XResourceInfo structure.
%
%    o windows: Specifies a pointer to a XWindows structure.
%
%    o state: key mask.
%
%    o key_symbol: Specifies a command to perform.
%
%    o image: Specifies a pointer to a Image structure;  XMagickCommand
%      may transform the image and return a new image pointer.
%
%
*/
static Image* XMagickCommand
(
    Display*       display,
    XResourceInfo* resource_info,
    XWindows*      windows,
    unsigned int   state,
    KeySym         key_symbol,
    Image**        image
)
{
  char
    *argv[5],
    geometry[MaxTextLength],
    modulate_factors[MaxTextLength];

  Image
    *cache_image,
    *loaded_image;

  ImageInfo
    image_info;

  int
    x,
    y;

  static char
    color[MaxTextLength] = "gray",
    delta[MaxTextLength] = "",
    Digits[]="01234567890";

  static KeySym
    last_symbol = XK_0;

  static Image
    *redo_image = (Image *) NULL,
    *undo_image = (Image *) NULL;

  unsigned int
    height,
    status,
    width;

  XCheckRefreshWindows(display,windows);
  if ((key_symbol >= XK_0) && (key_symbol <= XK_9))
    {
      if (!((last_symbol >= XK_0) && (last_symbol <= XK_9)))
        {
          *delta='\0';
          resource_info->quantum=1;
        }
      last_symbol=key_symbol;
      delta[strlen(delta)+1]='\0';
      delta[strlen(delta)]=Digits[key_symbol-XK_0];
      resource_info->quantum=atoi(delta);
      return((Image *) NULL);
    }
  last_symbol=key_symbol;
  switch (key_symbol)
  {
    case XK_Break:
    {
      /*
        Free memory from the undo and redo cache.
      */
      while (undo_image != (Image *) NULL)
      {
        cache_image=undo_image;
        undo_image=undo_image->previous;
        DestroyImage(cache_image->list);
        DestroyImage(cache_image);
      }
      undo_image=(Image *) NULL;
      if (redo_image != (Image *) NULL)
        DestroyImage(redo_image);
      redo_image=(Image *) NULL;
      return((Image *) NULL);
    }
    case XK_u:
    case XK_Undo:
    {
      /*
        Undo the last image transformation.
      */
      if (undo_image == (Image *) NULL)
        {
          XBell(display,0);
          return((Image *) NULL);
        }
      cache_image=undo_image;
      undo_image=undo_image->previous;
      windows->image.window_changes.width=cache_image->columns;
      windows->image.window_changes.height=cache_image->rows;
      if (windows->image.crop_geometry != (char *) NULL)
        free((char *) windows->image.crop_geometry);
      windows->image.crop_geometry=cache_image->geometry;
      if (redo_image != (Image *) NULL)
        DestroyImage(redo_image);
      redo_image=(*image);
      *image=cache_image->list;
      DestroyImage(cache_image);
      if (windows->image.orphan)
        return((Image *) NULL);
      XConfigureImageColormap(display,resource_info,windows,*image);
      (void) XConfigureImage(display,resource_info,windows,*image);
      return((Image *) NULL);
    }
    case XK_Up:
    case XK_KP_Up:
    case XK_Down:
    case XK_KP_Down:
    case XK_Left:
    case XK_KP_Left:
    case XK_Right:
    case XK_KP_Right:
    {
      if (!(state & Mod1Mask))
        break;
    }
    case XK_F3:
    case XK_F5:
    case XK_A:
    case XK_less:
    case XK_o:
    case XK_greater:
    case XK_percent:
    case XK_t:
    case XK_bracketleft:
    case XK_bracketright:
    case XK_minus:
    case XK_bar:
    case XK_slash:
    case XK_backslash:
    case XK_asterisk:
    case XK_F6:
    case XK_asciitilde:
    case XK_equal:
    case XK_N:
    case XK_F7:
    case XK_F8:
    case XK_F9:
    case XK_g:
    case XK_F10:
    case XK_F11:
    case XK_D:
    case XK_P:
    case XK_S:
    case XK_B:
    case XK_E:
    case XK_M:
    case XK_F13:
    case XK_O:
    case XK_asciicircum:
    case XK_Z:
    case XK_G:
    case XK_numbersign:
    case XK_R:
    case XK_a:
    case XK_b:
    case XK_F:
    case XK_x:
    case XK_exclam:
    case XK_Select:
    case XK_Redo:
    {
      Image
        *previous_image;

      unsigned int
        bytes;

      /*
        Ensure the undo cache has enough memory available.
      */
      if (state & Mod5Mask)
        break;
      bytes=(*image)->packets*sizeof(RunlengthPacket);
      if ((bytes >> 20) > resource_info->undo_cache)
        break;
      previous_image=undo_image;
      while (previous_image != (Image *) NULL)
      {
        bytes+=previous_image->list->packets*sizeof(RunlengthPacket);
        cache_image=previous_image;
        previous_image=previous_image->previous;
        if (bytes > (resource_info->undo_cache << 20))
          {
            if (cache_image->next != (Image *) NULL)
              cache_image->next->previous=(Image *) NULL;
            DestroyImage(cache_image->list);
            DestroyImage(cache_image);
          }
      }
      /*
        Save image before transformations are applied.
      */
      cache_image=AllocateImage((ImageInfo *) NULL);
      if (cache_image == (Image *) NULL)
        break;
      XSetCursorState(display,windows,True);
      XCheckRefreshWindows(display,windows);
      (*image)->orphan=True;
      cache_image->list=
        CopyImage(*image,(*image)->columns,(*image)->rows,True);
      (*image)->orphan=False;
      XSetCursorState(display,windows,False);
      if (cache_image->list == (Image *) NULL)
        {
          DestroyImage(cache_image);
          break;
        }
      cache_image->columns=windows->image.ximage->width;
      cache_image->rows=windows->image.ximage->height;
      cache_image->geometry=windows->image.crop_geometry;
      if (windows->image.crop_geometry != (char *) NULL)
        {
          cache_image->geometry=(char *) malloc(MaxTextLength*sizeof(char));
          if (cache_image->geometry != (char *) NULL)
            (void) strcpy(cache_image->geometry,windows->image.crop_geometry);
        }
      if (undo_image == (Image *) NULL)
        {
          undo_image=cache_image;
          break;
        }
      undo_image->next=cache_image;
      undo_image->next->previous=undo_image;
      undo_image=undo_image->next;
      break;
    }
    default:
      break;
  }
  if (key_symbol ==  XK_Redo)
    {
      /*
        Redo the last image transformation.
      */
      if (redo_image == (Image *) NULL)
        {
          XBell(display,0);
          return((Image *) NULL);
        }
      windows->image.window_changes.width=redo_image->columns;
      windows->image.window_changes.height=redo_image->rows;
      if (windows->image.crop_geometry != (char *) NULL)
        free((char *) windows->image.crop_geometry);
      windows->image.crop_geometry=redo_image->geometry;
      DestroyImage(*image);
      *image=redo_image;
      redo_image=(Image *) NULL;
      if (windows->image.orphan)
        return((Image *) NULL);
      XConfigureImageColormap(display,resource_info,windows,*image);
      (void) XConfigureImage(display,resource_info,windows,*image);
      return((Image *) NULL);
    }
  /*
    Process user command.
  */
  argv[0]=client_name;
  loaded_image=(Image *) NULL;
  windows->image.window_changes.width=windows->image.ximage->width;
  windows->image.window_changes.height=windows->image.ximage->height;

  switch (key_symbol)
  {
    case XK_l:
    {
      /*
        Load image.
      */
      loaded_image=XLoadImage(display,resource_info,windows,False);
      break;
    }
    case XK_n:
    case XK_space:
    {
      /*
        Display next image.
      */
      XClientMessage(display,windows->image.id,windows->im_protocols,
        windows->im_next_image,CurrentTime);
      break;
    }
    case XK_f:
    case XK_KP_Prior:
    case XK_BackSpace:
    {
      /*
        Display former image.
      */
      XClientMessage(display,windows->image.id,windows->im_protocols,
        windows->im_former_image,CurrentTime);
      break;
    }
    case XK_F2:
    {
      /*
        Select image.
      */
      (void) chdir(resource_info->home_directory);
      loaded_image=XLoadImage(display,resource_info,windows,True);
      break;
    }
    case XK_s:
    {
      /*
        Save image.
      */
      status=XSaveImage(display,resource_info,windows,image);
      if (status == False)
        {
          XNoticeWidget(display,windows,"Unable to write X image:",
            (*image)->filename);
          break;
        }
      break;
    }
    case XK_p:
    {
      /*
        Print image.
      */
      status=XPrintImage(display,resource_info,windows,image);
      if (status == False)
        {
          XNoticeWidget(display,windows,"Unable to print X image:",
            (*image)->filename);
          break;
        }
      break;
    }
    case XK_Delete:
    {
      static char
        filename[MaxTextLength] = "\0";

      /*
        Delete image file.
      */
      XFileBrowserWidget(display,windows,"Delete",filename);
      if (*filename == '\0')
        break;
      status=unlink(filename);
      if (status != False)
        XNoticeWidget(display,windows,"Unable to delete image file:",filename);
      break;
    }
    case XK_C:
    {
      static char
        color[MaxTextLength] = "gray",
        geometry[MaxTextLength] = "640x480";

      /*
        Query user for canvas geometry.
      */
      (void) XDialogWidget(display,windows,"Canvas","Enter canvas geometry:",
        geometry);
      if (*geometry == '\0')
        break;
      XColorBrowserWidget(display,windows,"Select",color);
      if (*color == '\0')
        break;
      /*
        Create canvas.
      */
      GetImageInfo(&image_info);
      (void) sprintf(image_info.filename,"xc:%s",color);
      image_info.size=geometry;
      loaded_image=ReadImage(&image_info);
      free((char *) image_info.filename);
      XClientMessage(display,windows->image.id,windows->im_protocols,
        windows->im_next_image,CurrentTime);
      break;
    }
    case XK_comma:
    {
      static char
        delay[MaxTextLength] = "5";

      /*
        Display next image after pausing.
      */
      resource_info->delay=0;
      (void) XDialogWidget(display,windows,"Slide Show",
        "Pause how many seconds between images:",delay);
      if (*delay == '\0')
        break;
      resource_info->delay=atoi(delay);
      XClientMessage(display,windows->image.id,windows->im_protocols,
        windows->im_next_image,CurrentTime);
      break;
    }
    case XK_V:
    {
      /*
        Visual Image directory.
      */
      loaded_image=XVisualDirectoryImage(display,resource_info,windows);
      break;
    }
    case XK_F3:
    {
      /*
        Cut image.
      */
      (void) XCropImage(display,resource_info,windows,*image,CutMode);
      break;
    }
    case XK_F4:
    {
      /*
        Copy image.
      */
      (void) XCropImage(display,resource_info,windows,*image,CopyMode);
      break;
    }
    case XK_F5:
    {
      /*
        Paste image.
      */
      status=XPasteImage(display,resource_info,windows,*image);
      if (status == False)
        {
          XNoticeWidget(display,windows,"Unable to paste X image",
            (*image)->filename);
          break;
        }
      break;
    }
    case XK_at:
    {
      (void) XConfigureImage(display,resource_info,windows,*image);
      break;
    }
    case XK_F12:
    {
      /*
        Set user preferences.
      */
      status=XPreferencesWidget(display,resource_info,windows);
      if (status == False)
        break;
      (*image)->orphan=True;
      loaded_image=CopyImage(*image,(*image)->columns,(*image)->rows,True);
      (*image)->orphan=False;
      if (loaded_image != (Image *) NULL)
        XClientMessage(display,windows->image.id,windows->im_protocols,
          windows->im_next_image,CurrentTime);
      break;
    }
    case XK_A:
    {
      char
        image_geometry[MaxTextLength];

      if ((windows->image.crop_geometry == (char *) NULL) &&
          ((*image)->columns == windows->image.ximage->width) &&
          ((*image)->rows == windows->image.ximage->height) &&
          (resource_info->number_colors == 0))
        break;
      /*
        Apply size transforms to image.
      */
      XSetCursorState(display,windows,True);
      XCheckRefreshWindows(display,windows);
      /*
        Crop and/or scale displayed image.
      */
      (void) sprintf(image_geometry,"%dx%d!",windows->image.ximage->width,
        windows->image.ximage->height);
      TransformImage(image,windows->image.crop_geometry,image_geometry);
      if (windows->image.crop_geometry != (char *) NULL)
        {
          free((char *) windows->image.crop_geometry);
          windows->image.crop_geometry=(char *) NULL;
        }
      windows->image.x=0;
      windows->image.y=0;
      if (resource_info->number_colors != 0)
        {
          /*
            Reduce the number of colors in the image.
          */
          if (((*image)->class == DirectClass) ||
              ((*image)->colors > resource_info->number_colors) ||
              (resource_info->colorspace == GRAYColorspace))
            QuantizeImage(*image,resource_info->number_colors,
              resource_info->tree_depth,resource_info->dither,
              resource_info->colorspace);
          SyncImage(*image);
        }
      XConfigureImageColormap(display,resource_info,windows,*image);
      (void) XConfigureImage(display,resource_info,windows,*image);
      break;
    }
    case XK_less:
    {
      /*
        Half image size.
      */
      windows->image.window_changes.width=windows->image.ximage->width >> 1;
      windows->image.window_changes.height=windows->image.ximage->height >> 1;
      (void) XConfigureImage(display,resource_info,windows,*image);
      break;
    }
    case XK_o:
    {
      /*
        Original image size.
      */
      windows->image.window_changes.width=(*image)->columns;
      windows->image.window_changes.height=(*image)->rows;
      (void) XConfigureImage(display,resource_info,windows,*image);
      break;
    }
    case XK_greater:
    {
      /*
        Double the image size.
      */
      windows->image.window_changes.width=windows->image.ximage->width << 1;
      windows->image.window_changes.height=windows->image.ximage->height << 1;
      (void) XConfigureImage(display,resource_info,windows,*image);
      break;
    }
    case XK_percent:
    {
      unsigned int
        height,
        width;

      /*
        Resize image.
      */
      width=windows->image.ximage->width;
      height=windows->image.ximage->height;
      (void) sprintf(geometry,"%ux%u",width,height);
      status=XDialogWidget(display,windows,"Resize",
        "Enter resize geometry (e.g. 640x480, 200%):",geometry);
      if (*geometry == '\0')
        break;
      if (!status)
        (void) strcat(geometry,"!");
      ParseImageGeometry(geometry,&width,&height);
      windows->image.window_changes.width=width;
      windows->image.window_changes.height=height;
      (void) XConfigureImage(display,resource_info,windows,*image);
      break;
    }
    case XK_r:
    {
      /*
        Restore Image window to its original size.
      */
      if ((windows->image.width == (*image)->columns) &&
          (windows->image.height == (*image)->rows) &&
          (windows->image.crop_geometry == (char *) NULL))
        {
          XBell(display,0);
          break;
        }
      windows->image.window_changes.width=(*image)->columns;
      windows->image.window_changes.height=(*image)->rows;
      if (windows->image.crop_geometry != (char *) NULL)
        {
          free((char *) windows->image.crop_geometry);
          windows->image.crop_geometry=(char *) NULL;
          windows->image.x=0;
          windows->image.y=0;
        }
      XConfigureImageColormap(display,resource_info,windows,*image);
      (void) XConfigureImage(display,resource_info,windows,*image);
      break;
    }
    case XK_bracketleft:
    {
      /*
        Crop image.
      */
      (void) XCropImage(display,resource_info,windows,*image,CropMode);
      break;
    }
    case XK_bracketright:
    {
      /*
        Chop image.
      */
      status=XChopImage(display,resource_info,windows,image);
      if (status == False)
        {
          XNoticeWidget(display,windows,"Unable to cut X image",
            (*image)->filename);
          break;
        }
      break;
    }
    case XK_bar:
    {
      /*
        Flop image scanlines.
      */
      XSetCursorState(display,windows,True);
      XCheckRefreshWindows(display,windows);
      argv[1]="-flop";
      MogrifyImage(resource_info->image_info,2,argv,image);
      XSetCursorState(display,windows,False);
      if (windows->image.crop_geometry != (char *) NULL)
        {
          /*
            Flop crop geometry.
          */
          (void) XParseGeometry(windows->image.crop_geometry,&x,&y,&width,
            &height);
          (void) sprintf(windows->image.crop_geometry,"%ux%u%+d%+d",width,
            height,(int) (*image)->columns-(int) width-x,y);
        }
      if (windows->image.orphan)
        break;
      (void) XConfigureImage(display,resource_info,windows,*image);
      break;
    }
    case XK_minus:
    {
      /*
        Flip image scanlines.
      */
      XSetCursorState(display,windows,True);
      XCheckRefreshWindows(display,windows);
      argv[1]="-flip";
      MogrifyImage(resource_info->image_info,2,argv,image);
      XSetCursorState(display,windows,False);
      if (windows->image.crop_geometry != (char *) NULL)
        {
          /*
            Flip crop geometry.
          */
          (void) XParseGeometry(windows->image.crop_geometry,&x,&y,&width,
            &height);
          (void) sprintf(windows->image.crop_geometry,"%ux%u%+d%+d",width,
            height,x,(int) (*image)->rows-(int) height-y);
        }
      if (windows->image.orphan)
        break;
      (void) XConfigureImage(display,resource_info,windows,*image);
      break;
    }
    case XK_slash:
    {
      /*
        Rotate image 90 degrees clockwise.
      */
      status=XRotateImage(display,resource_info,windows,90.0,image);
      if (status == False)
        {
          XNoticeWidget(display,windows,"Unable to rotate X image",
            (*image)->filename);
          break;
        }
      break;
    }
    case XK_backslash:
    {
      /*
        Rotate image 90 degrees counter-clockwise.
      */
      status=XRotateImage(display,resource_info,windows,-90.0,image);
      if (status == False)
        {
          XNoticeWidget(display,windows,"Unable to rotate X image",
            (*image)->filename);
          break;
        }
      break;
    }
    case XK_asterisk:
    {
      /*
        Rotate image.
      */
      status=XRotateImage(display,resource_info,windows,0.0,image);
      if (status == False)
        {
          XNoticeWidget(display,windows,"Unable to rotate X image",
            (*image)->filename);
          break;
        }
      break;
    }
    case XK_F6:
    {
      static char
        geometry[MaxTextLength] = "45.0x45.0";

      /*
        Query user for shear color and geometry.
      */
      XColorBrowserWidget(display,windows,"Select",color);
      if (*color == '\0')
        break;
      (void) XDialogWidget(display,windows,"Shear","Enter shear geometry:",
        geometry);
      if (*geometry == '\0')
        break;
      /*
        Shear image.
      */
      (void) XMagickCommand(display,resource_info,windows,0,(KeySym) XK_A,
        image);
      XSetCursorState(display,windows,True);
      XCheckRefreshWindows(display,windows);
      argv[1]="-bordercolor";
      argv[2]=color;
      argv[3]="-shear";
      argv[4]=geometry;
      MogrifyImage(resource_info->image_info,5,argv,image);
      XSetCursorState(display,windows,False);
      windows->image.window_changes.width=(*image)->columns;
      windows->image.window_changes.height=(*image)->rows;
      XConfigureImageColormap(display,resource_info,windows,*image);
      (void) XConfigureImage(display,resource_info,windows,*image);
      break;
    }
    case XK_t:
    {
      /*
        Trim image.
      */
      status=XTrimImage(display,resource_info,windows,*image);
      if (status == False)
        {
          XNoticeWidget(display,windows,"Unable to trim X image",
            (*image)->filename);
          break;
        }
      break;
    }
    case XK_F7:
    {
      static char
        hue_percent[MaxTextLength] = "3";

      /*
        Query user for percent hue change.
      */
      (void) XDialogWidget(display,windows,"Apply",
        "Enter percent change in image hue:",hue_percent);
      if (*hue_percent == '\0')
        break;
      /*
        Vary the image hue.
      */
      XSetCursorState(display,windows,True);
      XCheckRefreshWindows(display,windows);
      (void) strcpy(modulate_factors,"0.0,0.0,");
      (void) strcat(modulate_factors,hue_percent);
      argv[1]="-modulate";
      argv[2]=modulate_factors;
      MogrifyImage(resource_info->image_info,3,argv,image);
      XSetCursorState(display,windows,False);
      if (windows->image.orphan)
        break;
      XConfigureImageColormap(display,resource_info,windows,*image);
      (void) XConfigureImage(display,resource_info,windows,*image);
      break;
    }
    case XK_F8:
    {
      static char
        saturation_percent[MaxTextLength] = "10";

      /*
        Query user for percent saturation change.
      */
      (void) XDialogWidget(display,windows,"Apply",
        "Enter percent change in color saturation:",saturation_percent);
      if (*saturation_percent == '\0')
        break;
      /*
        Vary color saturation.
      */
      XSetCursorState(display,windows,True);
      XCheckRefreshWindows(display,windows);
      (void) strcpy(modulate_factors,"0.0,");
      (void) strcat(modulate_factors,saturation_percent);
      argv[1]="-modulate";
      argv[2]=modulate_factors;
      MogrifyImage(resource_info->image_info,3,argv,image);
      XSetCursorState(display,windows,False);
      if (windows->image.orphan)
        break;
      XConfigureImageColormap(display,resource_info,windows,*image);
      (void) XConfigureImage(display,resource_info,windows,*image);
      break;
    }
    case XK_F9:
    {
      static char
        brightness_percent[MaxTextLength] = "3";

      /*
        Query user for percent brightness change.
      */
      (void) XDialogWidget(display,windows,"Apply",
        "Enter percent change in color brightness:",brightness_percent);
      if (*brightness_percent == '\0')
        break;
      /*
        Vary the color brightness.
      */
      XSetCursorState(display,windows,True);
      XCheckRefreshWindows(display,windows);
      (void) strcpy(modulate_factors,brightness_percent);
      argv[1]="-modulate";
      argv[2]=modulate_factors;
      MogrifyImage(resource_info->image_info,3,argv,image);
      XSetCursorState(display,windows,False);
      if (windows->image.orphan)
        break;
      XConfigureImageColormap(display,resource_info,windows,*image);
      (void) XConfigureImage(display,resource_info,windows,*image);
      break;
    }
    case XK_g:
    {
      static char
        factor[MaxTextLength] = "1.6";

      /*
        Query user for gamma value.
      */
      (void) XDialogWidget(display,windows,"Gamma","Enter gamma value:",factor);
      if (*factor == '\0')
        break;
      /*
        Gamma correct image.
      */
      XSetCursorState(display,windows,True);
      XCheckRefreshWindows(display,windows);
      argv[1]="-gamma";
      argv[2]=factor;
      MogrifyImage(resource_info->image_info,3,argv,image);
      XSetCursorState(display,windows,False);
      if (windows->image.orphan)
        break;
      XConfigureImageColormap(display,resource_info,windows,*image);
      (void) XConfigureImage(display,resource_info,windows,*image);
      break;
    }
    case XK_F10:
    {
      /*
        Sharpen the image contrast.
      */
      XSetCursorState(display,windows,True);
      XCheckRefreshWindows(display,windows);
      argv[1]="-contrast";
      MogrifyImage(resource_info->image_info,2,argv,image);
      XSetCursorState(display,windows,False);
      if (windows->image.orphan)
        break;
      XConfigureImageColormap(display,resource_info,windows,*image);
      (void) XConfigureImage(display,resource_info,windows,*image);
      break;
    }
    case XK_F11:
    {
      /*
        Dull the image contrast.
      */
      XSetCursorState(display,windows,True);
      XCheckRefreshWindows(display,windows);
      argv[1]="+contrast";
      MogrifyImage(resource_info->image_info,2,argv,image);
      XSetCursorState(display,windows,False);
      if (windows->image.orphan)
        break;
      XConfigureImageColormap(display,resource_info,windows,*image);
      (void) XConfigureImage(display,resource_info,windows,*image);
      break;
    }
    case XK_equal:
    {
      /*
        Perform histogram equalization on the image.
      */
      XSetCursorState(display,windows,True);
      XCheckRefreshWindows(display,windows);
      argv[1]="-equalize";
      MogrifyImage(resource_info->image_info,2,argv,image);
      XSetCursorState(display,windows,False);
      if (windows->image.orphan)
        break;
      XConfigureImageColormap(display,resource_info,windows,*image);
      (void) XConfigureImage(display,resource_info,windows,*image);
      break;
    }
    case XK_N:
    {
      /*
        Perform histogram normalization on the image.
      */
      XSetCursorState(display,windows,True);
      XCheckRefreshWindows(display,windows);
      argv[1]="-normalize";
      MogrifyImage(resource_info->image_info,2,argv,image);
      XSetCursorState(display,windows,False);
      if (windows->image.orphan)
        break;
      XConfigureImageColormap(display,resource_info,windows,*image);
      (void) XConfigureImage(display,resource_info,windows,*image);
      break;
    }
    case XK_asciitilde:
    {
      /*
        Negate colors in image.
      */
      XSetCursorState(display,windows,True);
      XCheckRefreshWindows(display,windows);
      argv[1]="-negate";
      MogrifyImage(resource_info->image_info,2,argv,image);
      XSetCursorState(display,windows,False);
      if (windows->image.orphan)
        break;
      XConfigureImageColormap(display,resource_info,windows,*image);
      (void) XConfigureImage(display,resource_info,windows,*image);
      break;
    }
    case XK_G:
    {
      /*
        Convert image to grayscale.
      */
      XSetCursorState(display,windows,True);
      XCheckRefreshWindows(display,windows);
      argv[1]="-colorspace";
      argv[2] = "gray";
      MogrifyImage(resource_info->image_info,3,argv,image);
      XSetCursorState(display,windows,False);
      if (windows->image.orphan)
        break;
      XConfigureImageColormap(display,resource_info,windows,*image);
      (void) XConfigureImage(display,resource_info,windows,*image);
      break;
    }
    case XK_numbersign:
    {
      static char
        colors[MaxTextLength] = "256";

      /*
        Query user for maximum number of colors.
      */
      status=XDialogWidget(display,windows,"Quantize",
        "Maximum number of colors:",colors);
      if (*colors == '\0')
        break;
      /*
        Color reduce the image.
      */
      XSetCursorState(display,windows,True);
      XCheckRefreshWindows(display,windows);
      argv[1]="-colors";
      argv[2]=colors;
      argv[3]=(char*)(status ? "-dither" : "+dither");
      MogrifyImage(resource_info->image_info,4,argv,image);
      XSetCursorState(display,windows,False);
      if (windows->image.orphan)
        break;
      XConfigureImageColormap(display,resource_info,windows,*image);
      (void) XConfigureImage(display,resource_info,windows,*image);
      break;
    }
    case XK_D:
    {
      /*
        Despeckle image.
      */
      XSetCursorState(display,windows,True);
      XCheckRefreshWindows(display,windows);
      argv[1]="-despeckle";
      MogrifyImage(resource_info->image_info,2,argv,image);
      XSetCursorState(display,windows,False);
      if (windows->image.orphan)
        break;
      XConfigureImageColormap(display,resource_info,windows,*image);
      (void) XConfigureImage(display,resource_info,windows,*image);
      break;
    }
    case XK_P:
    {
      /*
        Reduce noise in the image.
      */
      XSetCursorState(display,windows,True);
      XCheckRefreshWindows(display,windows);
      argv[1]="-noise";
      MogrifyImage(resource_info->image_info,2,argv,image);
      XSetCursorState(display,windows,False);
      if (windows->image.orphan)
        break;
      XConfigureImageColormap(display,resource_info,windows,*image);
      (void) XConfigureImage(display,resource_info,windows,*image);
      break;
    }
    case XK_S:
    {
      static char
        factor[MaxTextLength] = "60.0";

      /*
        Query user for sharpen factor.
      */
      (void) XDialogWidget(display,windows,"Sharpen",
        "Enter the sharpening factor (0.0 - 99.9%):",factor);
      if (*factor == '\0')
        break;
      /*
        Sharpen image scanlines.
      */
      XSetCursorState(display,windows,True);
      XCheckRefreshWindows(display,windows);
      argv[1]="-sharpen";
      argv[2]=factor;
      MogrifyImage(resource_info->image_info,3,argv,image);
      XSetCursorState(display,windows,False);
      if (windows->image.orphan)
        break;
      XConfigureImageColormap(display,resource_info,windows,*image);
      (void) XConfigureImage(display,resource_info,windows,*image);
      break;
    }
    case XK_B:
    {
      static char
        factor[MaxTextLength] = "60.0";

      /*
        Query user for blur factor.
      */
      (void) XDialogWidget(display,windows,"Blur",
        "Enter the blurring factor (0.0 - 99.9%):",factor);
      if (*factor == '\0')
        break;
      /*
        Blur an image.
      */
      XSetCursorState(display,windows,True);
      XCheckRefreshWindows(display,windows);
      argv[1]="-blur";
      argv[2]=factor;
      MogrifyImage(resource_info->image_info,3,argv,image);
      XSetCursorState(display,windows,False);
      if (windows->image.orphan)
        break;
      XConfigureImageColormap(display,resource_info,windows,*image);
      (void) XConfigureImage(display,resource_info,windows,*image);
      break;
    }
    case XK_E:
    {
      static char
        factor[MaxTextLength] = "50.0";

      /*
        Query user for edge factor.
      */
      (void) XDialogWidget(display,windows,"Detect Edges",
        "Enter the edge detect factor (0.0 - 99.9%):",factor);
      if (*factor == '\0')
        break;
      /*
        Detect edge in image.
      */
      XSetCursorState(display,windows,True);
      XCheckRefreshWindows(display,windows);
      argv[1]="-edge";
      argv[2]=factor;
      MogrifyImage(resource_info->image_info,3,argv,image);
      XSetCursorState(display,windows,False);
      if (windows->image.orphan)
        break;
      XConfigureImageColormap(display,resource_info,windows,*image);
      (void) XConfigureImage(display,resource_info,windows,*image);
      break;
    }
    case XK_M:
    {
      /*
        Emboss image scanlines.
      */
      XSetCursorState(display,windows,True);
      XCheckRefreshWindows(display,windows);
      argv[1]="-emboss";
      MogrifyImage(resource_info->image_info,2,argv,image);
      XSetCursorState(display,windows,False);
      if (windows->image.orphan)
        break;
      XConfigureImageColormap(display,resource_info,windows,*image);
      (void) XConfigureImage(display,resource_info,windows,*image);
      break;
    }
    case XK_F13:
    {
      static char
        amount[MaxTextLength] = "2";

      /*
        Query user for spread amount.
      */
      (void) XDialogWidget(display,windows,"Spread",
        "Enter the displacement amount:",amount);
      if (*amount == '\0')
        break;
      /*
        Displace image pixels by a random amount.
      */
      XSetCursorState(display,windows,True);
      XCheckRefreshWindows(display,windows);
      argv[1]="-spread";
      argv[2]=amount;
      MogrifyImage(resource_info->image_info,3,argv,image);
      XSetCursorState(display,windows,False);
      if (windows->image.orphan)
        break;
      XConfigureImageColormap(display,resource_info,windows,*image);
      (void) XConfigureImage(display,resource_info,windows,*image);
      break;
    }
    case XK_O:
    {
      /*
        OilPaint image scanlines.
      */
      XSetCursorState(display,windows,True);
      XCheckRefreshWindows(display,windows);
      argv[1]="-paint";
      MogrifyImage(resource_info->image_info,2,argv,image);
      XSetCursorState(display,windows,False);
      if (windows->image.orphan)
        break;
      XConfigureImageColormap(display,resource_info,windows,*image);
      (void) XConfigureImage(display,resource_info,windows,*image);
      break;
    }
    case XK_asciicircum:
    {
      static char
        bevel_width[MaxTextLength] = "10";

      /*
        Query user for bevel width.
      */
      (void) XDialogWidget(display,windows,"Raise","Bevel width:",bevel_width);
      if (*bevel_width == '\0')
        break;
      /*
        Raise an image.
      */
      XSetCursorState(display,windows,True);
      XCheckRefreshWindows(display,windows);
      argv[1]="-raise";
      argv[2]=bevel_width;
      MogrifyImage(resource_info->image_info,3,argv,image);
      XSetCursorState(display,windows,False);
      if (windows->image.orphan)
        break;
      XConfigureImageColormap(display,resource_info,windows,*image);
      (void) XConfigureImage(display,resource_info,windows,*image);
      break;
    }
    case XK_Z:
    {
      static char
        threshold[MaxTextLength] = "1.5";

      /*
        Query user for smoothing threshold.
      */
      (void) XDialogWidget(display,windows,"Segment","Smoothing threshold:",
        threshold);
      if (*threshold == '\0')
        break;
      /*
        Segment an image.
      */
      XSetCursorState(display,windows,True);
      XCheckRefreshWindows(display,windows);
      argv[1]="-segment";
      argv[2]=threshold;
      MogrifyImage(resource_info->image_info,3,argv,image);
      XSetCursorState(display,windows,False);
      if (windows->image.orphan)
        break;
      XConfigureImageColormap(display,resource_info,windows,*image);
      (void) XConfigureImage(display,resource_info,windows,*image);
      break;
    }
    case XK_R:
    {
      /*
        Apply an image processing technique to a region of interest.
      */
      (void) XROIImage(display,resource_info,windows,image);
      break;
    }
    case XK_a:
    {
      /*
        Annotate the image with text.
      */
      status=XAnnotateEditImage(display,resource_info,windows,*image);
      if (status == False)
        {
          XNoticeWidget(display,windows,"Unable to annotate X image",
            (*image)->filename);
          break;
        }
      break;
    }
    case XK_d:
    {
      /*
        Draw image.
      */
      status=XDrawEditImage(display,resource_info,windows,image);
      if (status == False)
        {
          XNoticeWidget(display,windows,"Unable to draw on the X image",
            (*image)->filename);
          break;
        }
      break;
    }
    case XK_b:
    {
      static char
        geometry[MaxTextLength] = "6x6";

      /*
        Query user for border color and geometry.
      */
      XColorBrowserWidget(display,windows,"Select",color);
      if (*color == '\0')
        break;
      (void) XDialogWidget(display,windows,"Add Border",
        "Enter border geometry:",geometry);
      if (*geometry == '\0')
        break;
      /*
        Add a border to the image.
      */
      (void) XMagickCommand(display,resource_info,windows,0,(KeySym) XK_A,
        image);
      XSetCursorState(display,windows,True);
      XCheckRefreshWindows(display,windows);
      argv[1]="-bordercolor";
      argv[2]=color;
      argv[3]="-border";
      argv[4]=geometry;
      MogrifyImage(resource_info->image_info,5,argv,image);
      XSetCursorState(display,windows,False);
      windows->image.window_changes.width=(*image)->columns;
      windows->image.window_changes.height=(*image)->rows;
      XConfigureImageColormap(display,resource_info,windows,*image);
      (void) XConfigureImage(display,resource_info,windows,*image);
      break;
    }
    case XK_F:
    {
      static char
        geometry[MaxTextLength] = "6x6";

      /*
        Query user for frame color and geometry.
      */
      XColorBrowserWidget(display,windows,"Select",color);
      if (*color == '\0')
        break;
      (void) XDialogWidget(display,windows,"Add Frame","Enter frame geometry:",
        geometry);
      if (*geometry == '\0')
        break;
      /*
        Surround image with an ornamental border.
      */
      (void) XMagickCommand(display,resource_info,windows,0,(KeySym) XK_A,
        image);
      XSetCursorState(display,windows,True);
      XCheckRefreshWindows(display,windows);
      argv[1]="-mattecolor";
      argv[2]=color;
      argv[3]="-frame";
      argv[4]=geometry;
      MogrifyImage(resource_info->image_info,5,argv,image);
      XSetCursorState(display,windows,False);
      windows->image.window_changes.width=(*image)->columns;
      windows->image.window_changes.height=(*image)->rows;
      XConfigureImageColormap(display,resource_info,windows,*image);
      (void) XConfigureImage(display,resource_info,windows,*image);
      break;
    }
    case XK_x:
    {
      /*
        Composite image.
      */
      status=XCompositeImage(display,resource_info,windows,*image);
      if (status == False)
        {
          XNoticeWidget(display,windows,"Unable to composite X image",
            (*image)->filename);
          break;
        }
      break;
    }
    case XK_c:
    {
      /*
        Color edit.
      */
      status=XColorEditImage(display,resource_info,windows,image);
      if (status == False)
        {
          XNoticeWidget(display,windows,"Unable to pixel edit X image",
            (*image)->filename);
          break;
        }
      break;
    }
    case XK_m:
    {
      /*
        Matte edit.
      */
      status=XMatteEditImage(display,resource_info,windows,image);
      if (status == False)
        {
          XNoticeWidget(display,windows,"Unable to matte edit X image",
            (*image)->filename);
          break;
        }
      break;
    }
    case XK_exclam:
    {
      char
        command[MaxTextLength],
        filename[MaxTextLength];

      FILE
        *file;

      /*
        Edit image comment.
      */
      TemporaryFilename(filename);
      if ((*image)->comments != (char *) NULL)
        {
          register char
            *p;

          file=fopen(filename,"w");
          if (file == (FILE *) NULL)
            {
              XNoticeWidget(display,windows,"Unable to edit image comment",
                filename);
              break;
            }
          for (p=(*image)->comments; *p != '\0'; p++)
            (void) putc((int) *p,file);
          (void) putc('\n',file);
          (void) fclose(file);
        }
      (void) sprintf(command,EditorCommand,filename);
      if (resource_info->editor_command != (char *) NULL)
        (void) sprintf(command,resource_info->editor_command,filename);
      XSetCursorState(display,windows,True);
      XCheckRefreshWindows(display,windows);
      status=SystemCommand(command);
      if (status)
        XNoticeWidget(display,windows,"Unable to edit image comment",command);
      else
        {
          (void) sprintf(command,"@%s",filename);
          CommentImage(*image,command);
        }
      (void) unlink(filename);
      XSetCursorState(display,windows,False);
      break;
    }
    case XK_i:
    {
      /*
        Display image info.
      */
      XSetCursorState(display,windows,True);
      XCheckRefreshWindows(display,windows);
      XDisplayImageInfo(display,resource_info,windows,undo_image,*image);
      XSetCursorState(display,windows,False);
      break;
    }
    case XK_ampersand:
    {
      /*
        Background image.
      */
      status=XBackgroundImage(display,resource_info,windows,image);
      if (status == False)
        break;
      (*image)->orphan=True;
      loaded_image=CopyImage(*image,(*image)->columns,(*image)->rows,True);
      (*image)->orphan=False;
      if (loaded_image != (Image *) NULL)
        XClientMessage(display,windows->image.id,windows->im_protocols,
          windows->im_next_image,CurrentTime);
      break;
    }
    case XK_z:
    {
      /*
        Zoom image.
      */
      if (windows->magnify.mapped)
        XRaiseWindow(display,windows->magnify.id);
      else
        {
          /*
            Make magnify image.
          */
          XSetCursorState(display,windows,True);
          status=XMakeImage(display,resource_info,&windows->magnify,*image,
            windows->magnify.width,windows->magnify.height);
          status|=XMakePixmap(display,resource_info,&windows->magnify);
          if (status == False)
            Error("Unable to create magnify image",(char *) NULL);
          XMapRaised(display,windows->magnify.id);
          XSetCursorState(display,windows,False);
        }
      break;
    }
    case XK_H:
    {
      char
        command[MaxTextLength],
        filename[MaxTextLength];

      /*
        Show image histogram.
      */
      XSetCursorState(display,windows,True);
      XCheckRefreshWindows(display,windows);
      TemporaryFilename(filename);
      (void) sprintf(command,HistogramCommand,windows->image.id,
        (*image)->filename,filename);
      (void) sprintf((*image)->filename,"histogram:%s",filename);
      GetImageInfo(&image_info);
      status=WriteImage(&image_info,*image);
      if (status)
        status=!SystemCommand(command);
      if (!status)
        {
          XNoticeWidget(display,windows,"Unable to show image histogram",
            command);
          (void) unlink(filename);
        }
      XDelay(display,1500);
      XSetCursorState(display,windows,False);
      break;
    }
    case XK_h:
    case XK_F1:
    case XK_Help:
    {
      /*
        User requested help.
      */
      XTextViewWidget(display,resource_info,windows,False,
        "Help Viewer - Display",ImageMagickHelp);
      break;
    }
    case XK_Find:
    {
      char
        command[MaxTextLength];

      /*
        Browse the entire ImageMagick document.
      */
      XSetCursorState(display,windows,True);
      XCheckRefreshWindows(display,windows);
      (void) strcpy(command,DocumentationBrowser);
      status=SystemCommand(command);
      if (status)
        XNoticeWidget(display,windows,"Unable to browse documentation",command);
      XDelay(display,1500);
      XSetCursorState(display,windows,False);
      break;
    }
    case XK_v:
    {
      XNoticeWidget(display,windows,Version,
        "Copyright 1996 E. I. du Pont de Nemours and Company");
      break;
    }
    case XK_q:
    {
      /*
        Exit program.
      */
      if (!resource_info->confirm_exit)
        XClientMessage(display,windows->image.id,windows->im_protocols,
          windows->im_exit,CurrentTime);
      else
        {
          /*
            Confirm program exit.
          */
          status=XConfirmWidget(display,windows,"Do you really want to exit",
            client_name);
          if (status == True)
            XClientMessage(display,windows->image.id,windows->im_protocols,
              windows->im_exit,CurrentTime);
        }
      break;
    }
    case XK_Next:
    case XK_Prior:
    case XK_Home:
    case XK_KP_Home:
    {
      XTranslateImage(display,windows,*image,key_symbol);
      break;
    }
    case XK_Up:
    case XK_KP_Up:
    case XK_Down:
    case XK_KP_Down:
    case XK_Left:
    case XK_KP_Left:
    case XK_Right:
    case XK_KP_Right:
    {
      if (state & Mod1Mask)
        {
          RectangleInfo
            crop_info;

          /*
            Trim one pixel from edge of image.
          */
          crop_info.x=0;
          crop_info.y=0;
          crop_info.width=windows->image.ximage->width;
          crop_info.height=windows->image.ximage->height;
          if ((key_symbol == XK_Up) || (key_symbol == XK_KP_Up))
            {
              if (resource_info->quantum >= crop_info.height)
                resource_info->quantum=crop_info.height-1;
              crop_info.height-=resource_info->quantum;
            }
          if ((key_symbol == XK_Down) || (key_symbol == XK_KP_Down))
            {
              if (resource_info->quantum >= (crop_info.height-crop_info.y))
                resource_info->quantum=crop_info.height-crop_info.y-1;
              crop_info.y+=resource_info->quantum;
              crop_info.height-=resource_info->quantum;
            }
          if ((key_symbol == XK_Left) || (key_symbol == XK_KP_Left))
            {
              if (resource_info->quantum >= crop_info.width)
                resource_info->quantum=crop_info.width-1;
              crop_info.width-=resource_info->quantum;
            }
          if ((key_symbol == XK_Right) || (key_symbol == XK_KP_Right))
            {
              if (resource_info->quantum >= (crop_info.width-crop_info.x))
                resource_info->quantum=crop_info.width-crop_info.x-1;
              crop_info.x+=resource_info->quantum;
              crop_info.width-=resource_info->quantum;
            }
          if ((windows->image.x+windows->image.width) > crop_info.width)
            windows->image.x=crop_info.width-windows->image.width;
          if ((windows->image.y+windows->image.height) > crop_info.height)
            windows->image.y=crop_info.height-windows->image.height;
          XSetCropGeometry(display,windows,&crop_info,*image);
          windows->image.window_changes.width=crop_info.width;
          windows->image.window_changes.height=crop_info.height;
          XSetWindowBackgroundPixmap(display,windows->image.id,None);
          (void) XConfigureImage(display,resource_info,windows,*image);
          break;
        }
      XTranslateImage(display,windows,*image,key_symbol);
      break;
    }
    case XK_Return:
    case XK_KP_Enter:
    case XK_Select:
      break;
    default:
    {
      if (!IsModifierKey(key_symbol))
        XBell(display,0);
      break;
    }
  }
  return(loaded_image);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   X M a g n i f y I m a g e                                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Function XMagnifyImage magnifies portions of the image as indicated
%  by the pointer.  The magnified portion is displayed in a separate window.
%
%  The format of the XMagnifyImage routine is:
%
%    XMagnifyImage(display,windows,event)
%
%  A description of each parameter follows:
%
%    o display: Specifies a connection to an X server;  returned from
%      XOpenDisplay.
%
%    o windows: Specifies a pointer to a XWindows structure.
%
%    o event: Specifies a pointer to a XEvent structure.  If it is NULL,
%      the entire image is refreshed.
%
%
*/
static void XMagnifyImage
(
    Display*  display,
    XWindows* windows,
    XEvent*   event
)
{
  char
    text[MaxTextLength];

  register int
    x,
    y;

  unsigned long
    state;

  /*
    Update magnified image until the mouse button is released.
  */
  XDefineCursor(display,windows->image.id,windows->magnify.cursor);
  state=DefaultState;
  x=event->xbutton.x;
  y=event->xbutton.y;
  windows->magnify.x=windows->image.x+x;
  windows->magnify.y=windows->image.y+y;
  do
  {
    /*
      Map and unmap Info widget as text cursor crosses its boundaries.
    */
    if (windows->info.mapped)
      {
        if ((x < (windows->info.x+windows->info.width)) &&
            (y < (windows->info.y+windows->info.height)))
          XWithdrawWindow(display,windows->info.id,windows->info.screen);
      }
    else
      if ((x > (windows->info.x+windows->info.width)) ||
          (y > (windows->info.y+windows->info.height)))
        XMapWindow(display,windows->info.id);
    if (windows->info.mapped)
      {
        /*
          Display pointer position.
        */
        (void) sprintf(text," %+d%+d ",windows->magnify.x,windows->magnify.y);
        XInfoWidget(display,windows,text);
      }
    /*
      Wait for next event.
    */
    XIfEvent(display,event,XScreenEvent,(char *) windows);
    switch (event->type)
    {
      case ButtonPress:
        break;
      case ButtonRelease:
      {
        /*
          User has finished magnifying image.
        */
        x=event->xbutton.x;
        y=event->xbutton.y;
        state|=ExitState;
        break;
      }
      case Expose:
        break;
      case MotionNotify:
      {
        /*
          Discard pending button motion events.
        */
        while (XCheckMaskEvent(display,ButtonMotionMask,event));
        x=event->xmotion.x;
        y=event->xmotion.y;
        break;
      }
      default:
        break;
    }
    /*
      Check boundary conditions.
    */
    if (x < 0)
      x=0;
    else
      if (x >= windows->image.width)
        x=windows->image.width-1;
    if (y < 0)
      y=0;
    else
     if (y >= windows->image.height)
       y=windows->image.height-1;
  } while (!(state & ExitState));
  /*
    Display magnified image.
  */
  XSetCursorState(display,windows,False);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   X M a g n i f y W i n d o w C o m m a n d                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Function XMagnifyWindowCommand moves the image within an Magnify window by
%  one pixel as specified by the key symbol.
%
%  The format of the XMagnifyWindowCommand routine is:
%
%    XMagnifyWindowCommand(display,windows,state,key_symbol)
%
%  A description of each parameter follows:
%
%    o display: Specifies a connection to an X server; returned from
%      XOpenDisplay.
%
%    o windows: Specifies a pointer to a XWindows structure.
%
%    o state: key mask.
%
%    o key_symbol: Specifies a KeySym which indicates which side of the image
%      to trim.
%
%
*/
static void XMagnifyWindowCommand
(
    Display*     display,
    XWindows*    windows,
    unsigned int state,
    KeySym       key_symbol
)
{
  unsigned int
    quantum;

  /*
    User specified a magnify factor or position.
  */
  quantum=1;
  if (state & Mod1Mask)
    quantum=10;
  switch (key_symbol)
  {
    case XK_q:
    {
      XWithdrawWindow(display,windows->magnify.id,windows->magnify.screen);
      break;
    }
    case XK_Home:
    case XK_KP_Home:
    {
      windows->magnify.x=windows->image.width >> 1;
      windows->magnify.y=windows->image.height >> 1;
      break;
    }
    case XK_Left:
    case XK_KP_Left:
    {
      if (windows->magnify.x > 0)
        windows->magnify.x-=quantum;
      break;
    }
    case XK_Up:
    case XK_KP_Up:
    {
      if (windows->magnify.y > 0)
        windows->magnify.y-=quantum;
      break;
    }
    case XK_Right:
    case XK_KP_Right:
    {
      if (windows->magnify.x < (windows->image.width-1))
        windows->magnify.x+=quantum;
      break;
    }
    case XK_Down:
    case XK_KP_Down:
    {
      if (windows->magnify.y < (windows->image.height-1))
        windows->magnify.y+=quantum;
      break;
    }
    case XK_0:
    case XK_1:
    case XK_2:
    case XK_3:
    case XK_4:
    case XK_5:
    case XK_6:
    case XK_7:
    case XK_8:
    case XK_9:
    {
      windows->magnify.data=key_symbol-XK_0;
      break;
    }
    case XK_KP_0:
    case XK_KP_1:
    case XK_KP_2:
    case XK_KP_3:
    case XK_KP_4:
    case XK_KP_5:
    case XK_KP_6:
    case XK_KP_7:
    case XK_KP_8:
    case XK_KP_9:
    {
      windows->magnify.data=key_symbol-XK_KP_0;
      break;
    }
    default:
      break;
  }
  XMakeMagnifyImage(display,windows);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   X M a k e P a n I m a g e                                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Function XMakePanImage creates a thumbnail of the image and displays it in
%  the Pan icon window.
%
%  The format of the XMakePanImage routine is:
%
%      XMakePanImage(display,resource_info,windows,image)
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
%    o image: Specifies a pointer to a Image structure;  returned from
%      ReadImage.
%
%
*/
static void XMakePanImage
(
    Display*       display,
    XResourceInfo* resource_info,
    XWindows*      windows,
    Image*         image
)
{
  unsigned int
    status;

  /*
    Create and display image for panning icon.
  */
  XSetCursorState(display,windows,True);
  XCheckRefreshWindows(display,windows);
  windows->pan.x=windows->image.x;
  windows->pan.y=windows->image.y;
  status=XMakeImage(display,resource_info,&windows->pan,image,
    windows->pan.width,windows->pan.height);
  status|=XMakePixmap(display,resource_info,&windows->pan);
  if (status == False)
    Error("Unable to create Pan icon image",(char *) NULL);
  XSetWindowBackgroundPixmap(display,windows->pan.id,windows->pan.pixmap);
  XClearWindow(display,windows->pan.id);
  XDrawPanRectangle(display,windows);
  XSetCursorState(display,windows,False);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   X M a t t a E d i t I m a g e                                             %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Function XMatteEditImage allows the user to interactively change
%  the Matte channel of an image.  If the image is PseudoClass it is promoted
%  to DirectClass before the matte information is stored.  The floodfill
%  algorithm is strongly based on a similiar algorithm in "Graphics Gems" by
%  Paul Heckbert.
%
%  The format of the XMatteEditImage routine is:
%
%    XMatteEditImage(display,resource_info,windows,image)
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
%    o image: Specifies a pointer to a Image structure; returned from
%      ReadImage.
%
*/

static void MatteFloodfill
(
    Image* image,
    int    x,
    int    y,
    int    matte,
    int    delta
)
{
  int
    offset,
    skip,
    start,
    x1,
    x2;

  register RunlengthPacket
    *pixel;

  register XSegment
    *p;

  RunlengthPacket
    target;

  XSegment
    *segment_stack;

  /*
    Check boundary conditions.
  */
  if ((y < 0) || (y >= image->rows))
    return;
  if ((x < 0) || (x >= image->columns))
    return;
  target=image->pixels[y*image->columns+x];
  if (target.index == (unsigned short) matte)
    return;
  /*
    Allocate segment stack.
  */
  segment_stack=(XSegment *) malloc(MaxStacksize*sizeof(XSegment));
  if (segment_stack == (XSegment *) NULL)
    {
      Warning("Unable to floodfill","Memory allocation failed");
      return;
    }
  /*
    Push initial segment on stack.
  */
  start=0;
  p=segment_stack;
  Push(y,x,x,1);
  Push(y+1,x,x,-1);
  while (p > segment_stack)
  {
    /*
      Pop segment off stack.
    */
    p--;
    x1=p->x1;
    x2=p->x2;
    offset=p->y2;
    y=p->y1+offset;
    /*
      Update matte information in neighboring pixels.
    */
    for (x=x1; x >= 0 ; x--)
    {
      pixel=image->pixels+(y*image->columns+x);
      if (!MatteMatch(*pixel,target,delta))
        break;
      pixel->index=(unsigned short) matte;
    }
    skip=x >= x1;
    if (!skip)
      {
        start=x+1;
        if (start < x1)
          Push(y,start,x1-1,-offset);
        x=x1+1;
      }
    do
    {
      if (!skip)
        {
          for ( ; x < image->columns; x++)
          {
            pixel=image->pixels+(y*image->columns+x);
            if (!MatteMatch(*pixel,target,delta))
              break;
            pixel->index=(unsigned short) matte;
          }
          Push(y,start,x-1,offset);
          if (x > (x2+1))
            Push(y,x2+1,x-1,-offset);
        }
      skip=False;
      for (x++; x <= x2 ; x++)
      {
        pixel=image->pixels+(y*image->columns+x);
        if (MatteMatch(*pixel,target,delta))
          break;
      }
      start=x;
    } while (x <= x2);
  }
  free((char *) segment_stack);
}

static unsigned int XMatteEditImage
(
    Display*       display,
    XResourceInfo* resource_info,
    XWindows*      windows,
    Image**        image
)
{
#define MatteEditMethodOp  0
#define MatteEditDeltaOp  1
#define MatteEditValueOp  2
#define MatteEditUndoOp  3
#define MatteEditHelpOp  4
#define MatteEditDismissOp  5

  static char
    *MatteEditMenu[]=
    {
      "Method",
      "Delta",
      "Matte Value",
      "Undo",
      "Help",
      "Dismiss",
      (char *) NULL
    };

  char
    command[MaxTextLength],
    text[MaxTextLength];

  Cursor
    cursor;

  int
    entry,
    id,
    x,
    x_offset,
    y,
    y_offset;

  register int
    i;

  register RunlengthPacket
    *p;

  static char
    matte[MaxTextLength] = "0";

  static unsigned int
    delta = 0,
    method = PointMethodOp;

  unsigned int
    height,
    width;

  unsigned long
    state,
    x_factor,
    y_factor;

  XEvent
    event;

  /*
    Map Command widget.
  */
  windows->command.name="Matte Edit";
  windows->command.data=2;
  (void) XCommandWidget(display,windows,MatteEditMenu,(XEvent *) NULL);
  XMapRaised(display,windows->command.id);
  XClientMessage(display,windows->image.id,windows->im_protocols,
    windows->im_update_widget,CurrentTime);
  /*
    Make cursor.
  */
  cursor=XMakeCursor(display,windows->image.id,
    windows->image.map_info->colormap,resource_info->background_color,
    resource_info->foreground_color);
  XDefineCursor(display,windows->image.id,cursor);
  /*
    Track pointer until button 1 is pressed.
  */
  XQueryPosition(display,windows->image.id,&x,&y);
  XSelectInput(display,windows->image.id,windows->image.attributes.event_mask |
    PointerMotionMask);
  state=DefaultState;
  do
  {
    if (windows->info.mapped)
      {
        /*
          Display pointer position.
        */
        (void) sprintf(text," %+d%+d ",x-windows->image.x,y-windows->image.y);
        XInfoWidget(display,windows,text);
      }
    /*
      Wait for next event.
    */
    XIfEvent(display,&event,XScreenEvent,(char *) windows);
    if (event.xany.window == windows->command.id)
      {
        /*
          Select a command from the Command widget.
        */
        id=XCommandWidget(display,windows,MatteEditMenu,&event);
        if (id < 0)
          {
            XDefineCursor(display,windows->image.id,cursor);
            continue;
          }
        switch (id)
        {
          case MatteEditMethodOp:
          {
            static char
              *MethodMenu[]=
              {
                "point",
                "replace",
                "floodfill",
                "reset",
                (char *) NULL,
              };

            /*
              Select a method from the pop-up menu.
            */
            entry=
              XMenuWidget(display,windows,MatteEditMenu[id],MethodMenu,command);
            if (entry >= 0)
              method=entry;
            break;
          }
          case MatteEditDeltaOp:
          {
            static char
              *DeltaMenu[]=
              {
                "0",
                "1",
                "2",
                "4",
                "8",
                "16",
                "32",
                (char *) NULL,
                (char *) NULL,
              },
              value[MaxTextLength] = "3";

            /*
              Select a delta value from the pop-up menu.
            */
            DeltaMenu[7]="Dialog...";
            entry=XMenuWidget(display,windows,MatteEditMenu[id],DeltaMenu,
              command);
            if (entry < 0)
              break;
            if (entry != 7)
              {
                delta=atoi(DeltaMenu[entry]);
                break;
              }
            (void) XDialogWidget(display,windows,"Ok","Enter delta value:",
              value);
            if (*value == '\0')
              break;
            delta=atoi(value);
            break;
          }
          case MatteEditValueOp:
          {
            /*
              Request matte value from the user.
            */
            (void) XDialogWidget(display,windows,"Matte","Enter matte value:",
              matte);
            break;
          }
          case MatteEditUndoOp:
          {
            (void) XMagickCommand(display,resource_info,windows,0,
              (KeySym) XK_u,image);
            break;
          }
          case MatteEditHelpOp:
          {
            XTextViewWidget(display,resource_info,windows,False,
              "Help Viewer - Matte Edit",ImageMatteEditHelp);
            break;
          }
          case MatteEditDismissOp:
          {
            /*
              Prematurely exit.
            */
            state|=EscapeState;
            state|=ExitState;
            break;
          }
          default:
            break;
        }
        XDefineCursor(display,windows->image.id,cursor);
        continue;
      }
    switch (event.type)
    {
      case ButtonPress:
      {
        if (event.xbutton.window == windows->pan.id)
          {
            XPanImage(display,windows,&event);
            XInfoWidget(display,windows,text);
            break;
          }
        /*
          Update matte data.
        */
        x=event.xbutton.x;
        y=event.xbutton.y;
        (void) XMagickCommand(display,resource_info,windows,0,
          (KeySym) XK_Select,image);
        state|=UpdateConfigurationState;
        break;
      }
      case ButtonRelease:
      {
        /*
          Update colormap information.
        */
        x=event.xbutton.x;
        y=event.xbutton.y;
        XConfigureImageColormap(display,resource_info,windows,*image);
        (void) XConfigureImage(display,resource_info,windows,*image);
        XInfoWidget(display,windows,text);
        XDefineCursor(display,windows->image.id,cursor);
        state&=(~UpdateConfigurationState);
        break;
      }
      case Expose:
        break;
      case KeyPress:
      {
        char
          command[MaxTextLength];

        KeySym
          key_symbol;

        if (event.xkey.window != windows->image.id)
          break;
        /*
          Respond to a user key press.
        */
        (void) XLookupString((XKeyEvent *) &event.xkey,command,sizeof(command),
          &key_symbol,(XComposeStatus *) NULL);
        switch (key_symbol)
        {
          case XK_Escape:
          case XK_F20:
          {
            /*
              Prematurely exit.
            */
            state|=ExitState;
            break;
          }
          case XK_F1:
          case XK_Help:
          {
            XTextViewWidget(display,resource_info,windows,False,
              "Help Viewer - Matte Edit",ImageMatteEditHelp);
            break;
          }
          default:
          {
            XBell(display,0);
            break;
          }
        }
        break;
      }
      case MotionNotify:
      {
        /*
          Discard pending pointer motion events.
        */
        while (XCheckMaskEvent(display,PointerMotionMask,&event));
        x=event.xmotion.x;
        y=event.xmotion.y;
        /*
          Map and unmap Info widget as cursor crosses its boundaries.
        */
        if (windows->info.mapped)
          {
            if ((x < (windows->info.x+windows->info.width)) &&
                (y < (windows->info.y+windows->info.height)))
              XWithdrawWindow(display,windows->info.id,windows->info.screen);
          }
        else
          if ((x > (windows->info.x+windows->info.width)) ||
              (y > (windows->info.y+windows->info.height)))
            XMapWindow(display,windows->info.id);
        break;
      }
      default:
        break;
    }
    x_offset=x;
    y_offset=y;
    if (state & UpdateConfigurationState)
      {
        int
         x,
         y;

        /*
          Matte edit is relative to image configuration.
        */
        XClearArea(display,windows->image.id,x_offset,y_offset,1,1,True);
        x=0;
        y=0;
        width=(*image)->columns;
        height=(*image)->rows;
        if (windows->image.crop_geometry != (char *) NULL)
          (void) XParseGeometry(windows->image.crop_geometry,&x,&y,
            &width,&height);
        x_factor=UpShift(width)/windows->image.ximage->width;
        x_offset=DownShift((windows->image.x+x_offset)*x_factor)+x;
        y_factor=UpShift(height)/windows->image.ximage->height;
        y_offset=DownShift((windows->image.y+y_offset)*y_factor)+y;
        if ((x_offset < 0) || (y_offset < 0))
          continue;
        if ((x_offset >= (*image)->columns) || (y_offset >= (*image)->rows))
          continue;
        XPutPixel(windows->image.ximage,x_offset,y_offset,
          windows->image.pixel_info->background_color.pixel);
        (*image)->class=DirectClass;
        if (!(*image)->matte)
          {
            /*
              Initialize matte data.
            */
            p=(*image)->pixels;
            for (i=0; i < (*image)->packets; i++)
            {
              p->index=Opaque;
              p++;
            }
            (*image)->matte=True;
          }
        switch (method)
        {
          case PointMethodOp:
          default:
          {
            /*
              Update matte information using point algorithm.
            */
            if (!UncompressImage(*image))
              break;
            p=(*image)->pixels+(y_offset*(*image)->columns+x_offset);
            p->index=atoi(matte) & 0xff;
            break;
          }
          case ReplaceMethodOp:
          {
            RunlengthPacket
              target;

            /*
              Update matte information using replace algorithm.
            */
            x=0;
            p=(*image)->pixels;
            for (i=0; i < (*image)->packets; i++)
            {
              x+=(p->length+1);
              if (x > (y_offset*(*image)->columns+x_offset))
                break;
              p++;
            }
            target=(*image)->pixels[i];
            p=(*image)->pixels;
            for (i=0; i < (*image)->packets; i++)
            {
              if (ColorMatch(*p,target,delta))
                p->index=atoi(matte) & 0xff;
              p++;
            }
            break;
          }
          case FloodfillMethodOp:
          {
            /*
              Update matte information using floodfill algorithm.
            */
            if (!UncompressImage(*image))
              break;
            MatteFloodfill(*image,x_offset,y_offset,atoi(matte) & 0xff,delta);
            break;
          }
          case ResetMethodOp:
          {
            /*
              Update matte information using reset algorithm.
            */
            p=(*image)->pixels;
            for (i=0; i < (*image)->packets; i++)
            {
              p->index=atoi(matte) & 0xff;
              p++;
            }
            if ((atoi(matte) & 0xff) == Opaque)
              (*image)->matte=False;
            break;
          }
        }
        state&=(~UpdateConfigurationState);
      }
  } while (!(state & ExitState));
  XSelectInput(display,windows->image.id,windows->image.attributes.event_mask);
  XSetCursorState(display,windows,False);
  XFreeCursor(display,cursor);
  return(True);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   X P a n I m a g e                                                         %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Function XPanImage pans the image until the mouse button is released.
%
%  The format of the XPanImage routine is:
%
%    XPanImage(display,windows,event)
%
%  A description of each parameter follows:
%
%    o display: Specifies a connection to an X server;  returned from
%      XOpenDisplay.
%
%    o windows: Specifies a pointer to a XWindows structure.
%
%    o event: Specifies a pointer to a XEvent structure.  If it is NULL,
%      the entire image is refreshed.
%
*/
static void XPanImage(Display* display, XWindows* windows, XEvent* event)
{
  char
    text[MaxTextLength];

  Cursor
    cursor;

  RectangleInfo
    pan_info;

  unsigned long
    state,
    x_factor,
    y_factor;

  /*
    Define cursor.
  */
  if ((windows->image.ximage->width > windows->image.width) &&
      (windows->image.ximage->height > windows->image.height))
    cursor=XCreateFontCursor(display,XC_fleur);
  else
    if (windows->image.ximage->width > windows->image.width)
      cursor=XCreateFontCursor(display,XC_sb_h_double_arrow);
    else
      if (windows->image.ximage->height > windows->image.height)
        cursor=XCreateFontCursor(display,XC_sb_v_double_arrow);
      else
        cursor=XCreateFontCursor(display,XC_arrow);
  XDefineCursor(display,windows->pan.id,cursor);
  /*
    Pan image as pointer moves until the mouse button is released.
  */
  x_factor=(unsigned long)
    UpShift(windows->image.ximage->width)/windows->pan.width;
  y_factor=(unsigned long)
    UpShift(windows->image.ximage->height)/windows->pan.height;
  pan_info.width=(unsigned int) (UpShift(windows->image.width)/x_factor);
  pan_info.height=(unsigned int) (UpShift(windows->image.height)/y_factor);
  state=UpdateConfigurationState;
  do
  {
    switch (event->type)
    {
      case ButtonPress:
      {
        /*
          User choose an initial pan location.
        */
        pan_info.x=event->xbutton.x;
        pan_info.y=event->xbutton.y;
        state|=UpdateConfigurationState;
        break;
      }
      case ButtonRelease:
      {
        /*
          User has finished panning the image.
        */
        pan_info.x=event->xbutton.x;
        pan_info.y=event->xbutton.y;
        state|=UpdateConfigurationState | ExitState;
        break;
      }
      case MotionNotify:
      {
        /*
          Discard pending button motion events.
        */
        while (XCheckMaskEvent(display,ButtonMotionMask,event));
        pan_info.x=event->xmotion.x;
        pan_info.y=event->xmotion.y;
        state|=UpdateConfigurationState;
      }
      default:
        break;
    }
    if (state & UpdateConfigurationState)
      {
        /*
          Check boundary conditions.
        */
        pan_info.x=DownShift((pan_info.x-(pan_info.width >> 1))*x_factor);
        if (pan_info.x < 0)
          pan_info.x=0;
        else
          if ((pan_info.x+windows->image.width) > windows->image.ximage->width)
            pan_info.x=windows->image.ximage->width-windows->image.width;
        pan_info.y=DownShift((pan_info.y-(pan_info.height >> 1))*y_factor);
        if (pan_info.y < 0)
          pan_info.y=0;
        else
          if ((pan_info.y+windows->image.height) >
               windows->image.ximage->height)
            pan_info.y=windows->image.ximage->height-windows->image.height;
        if ((windows->image.x != pan_info.x) ||
            (windows->image.y != pan_info.y))
          {
            /*
              Display image pan offset.
            */
            windows->image.x=pan_info.x;
            windows->image.y=pan_info.y;
            (void) sprintf(text," %ux%u%+d%+d ",windows->image.width,
              windows->image.height,windows->image.x,windows->image.y);
            XInfoWidget(display,windows,text);
            /*
              Refresh Image window.
            */
            XDrawPanRectangle(display,windows);
            XRefreshWindow(display,&windows->image,(XEvent *) NULL);
          }
        state&=(~UpdateConfigurationState);
      }
    /*
      Wait for next event.
    */
    if (!(state & ExitState))
      XIfEvent(display,event,XScreenEvent,(char *) windows);
  } while (!(state & ExitState));
  /*
    Restore cursor.
  */
  XDefineCursor(display,windows->pan.id,windows->pan.cursor);
  XFreeCursor(display,cursor);
  XWithdrawWindow(display,windows->info.id,windows->info.screen);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   X P a s t e I m a g e                                                     %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Function XPasteImage pastes an image previously saved with XCropImage
%  in the X window image at a location the user chooses with the pointer.
%
%  The format of the XPasteImage routine is:
%
%    status=XPasteImage(display,resource_info,windows,image)
%
%  A description of each parameter follows:
%
%    o status: Function XPasteImage returns True if the image is
%      pasted.  False is returned is there is a memory shortage or if the
%      image fails to be pasted.
%
%    o display: Specifies a connection to an X server;  returned from
%      XOpenDisplay.
%
%    o resource_info: Specifies a pointer to a X11 XResourceInfo structure.
%
%    o windows: Specifies a pointer to a XWindows structure.
%
%    o image: Specifies a pointer to a Image structure; returned from
%      ReadImage.
%
*/
static unsigned int XPasteImage
(
    Display*       display,
    XResourceInfo* resource_info,
    XWindows*      windows,
    Image*         image
)
{
#define PasteModeOperatorsOp  0
#define PasteModeHelpOp  1
#define PasteModeDismissOp  2

  static char
    *PasteModeMenu[]=
    {
      "Operator",
      "Help",
      "Dismiss",
      (char *) NULL
    };

  char
    text[MaxTextLength];

  Cursor
    cursor;

  Image
    *paste_image;

  int
    id,
    x,
    y;

  RectangleInfo
    highlight_info,
    paste_info;

  static unsigned int
    operator = ReplaceCompositeOp;

  unsigned int
    height,
    width;

  unsigned long
    scale_factor,
    state;

  XEvent
    event;

  /*
    Copy image.
  */
  if (copy_image == (Image *) NULL)
    return(False);
  copy_image->orphan=True;
  paste_image=CopyImage(copy_image,copy_image->columns,copy_image->rows,True);
  copy_image->orphan=False;
  /*
    Map Command widget.
  */
  windows->command.name="Paste";
  windows->command.data=1;
  (void) XCommandWidget(display,windows,PasteModeMenu,(XEvent *) NULL);
  XMapRaised(display,windows->command.id);
  XClientMessage(display,windows->image.id,windows->im_protocols,
    windows->im_update_widget,CurrentTime);
  /*
    Track pointer until button 1 is pressed.
  */
  XSetCursorState(display,windows,False);
  XQueryPosition(display,windows->image.id,&x,&y);
  paste_info.x=windows->image.x+x;
  paste_info.y=windows->image.y+y;
  paste_info.width=0;
  paste_info.height=0;
  XSelectInput(display,windows->image.id,windows->image.attributes.event_mask |
    PointerMotionMask);
  cursor=XCreateFontCursor(display,XC_ul_angle);
  XSetFunction(display,windows->image.highlight_context,GXinvert);
  state=DefaultState;
  do
  {
    if (windows->info.mapped)
      {
        /*
          Display pointer position.
        */
        (void) sprintf(text," %+d%+d ",paste_info.x,paste_info.y);
        XInfoWidget(display,windows,text);
      }
    highlight_info=paste_info;
    highlight_info.x=paste_info.x-windows->image.x;
    highlight_info.y=paste_info.y-windows->image.y;
    XHighlightRectangle(display,windows->image.id,
      windows->image.highlight_context,&highlight_info);
    /*
      Wait for next event.
    */
    XIfEvent(display,&event,XScreenEvent,(char *) windows);
    XHighlightRectangle(display,windows->image.id,
      windows->image.highlight_context,&highlight_info);
    if (event.xany.window == windows->command.id)
      {
        /*
          Select a command from the Command widget.
        */
        id=XCommandWidget(display,windows,PasteModeMenu,&event);
        if (id < 0)
          continue;
        switch (id)
        {
          case PasteModeOperatorsOp:
          {
            char
              command[MaxTextLength];

            static char
              *OperatorMenu[]=
              {
                "over",
                "in",
                "out",
                "atop",
                "xor",
                "plus",
                "minus",
                "add",
                "subtract",
                "difference",
                "replace",
                (char *) NULL,
              };

            /*
              Select a command from the pop-up menu.
            */
            operator=XMenuWidget(display,windows,PasteModeMenu[id],
              OperatorMenu,command)+1;
            break;
          }
          case PasteModeHelpOp:
          {
            XTextViewWidget(display,resource_info,windows,False,
              "Help Viewer - Image Compositing",ImagePasteHelp);
            break;
          }
          case PasteModeDismissOp:
          {
            /*
              Prematurely exit.
            */
            state|=EscapeState;
            state|=ExitState;
            break;
          }
          default:
            break;
        }
        continue;
      }
    switch (event.type)
    {
      case ButtonPress:
      {
        if (resource_info->debug)
          (void) fprintf(stderr,"Button Press: 0x%lx %u +%d+%d\n",
            event.xbutton.window,event.xbutton.button,event.xbutton.x,
            event.xbutton.y);
        if (event.xbutton.window == windows->pan.id)
          {
            XPanImage(display,windows,&event);
            XInfoWidget(display,windows,text);
            break;
          }
        /*
          Paste rectangle is relative to image configuration.
        */
        x=0;
        y=0;
        width=image->columns;
        height=image->rows;
        if (windows->image.crop_geometry != (char *) NULL)
          (void) XParseGeometry(windows->image.crop_geometry,&x,&y,
            &width,&height);
        scale_factor=UpShift(width)/windows->image.ximage->width;
        paste_info.width=DownShift(paste_image->columns*scale_factor);
        scale_factor=UpShift(height)/windows->image.ximage->height;
        paste_info.height=DownShift(paste_image->rows*scale_factor);
        XDefineCursor(display,windows->image.id,cursor);
        paste_info.x=windows->image.x+event.xbutton.x;
        paste_info.y=windows->image.y+event.xbutton.y;
        break;
      }
      case ButtonRelease:
      {
        if (resource_info->debug)
          (void) fprintf(stderr,"Button Release: 0x%lx %u +%d+%d\n",
            event.xbutton.window,event.xbutton.button,event.xbutton.x,
            event.xbutton.y);
        if ((paste_info.width != 0) && (paste_info.height != 0))
          {
            /*
              User has selected the location of the paste image.
            */
            paste_info.x=windows->image.x+event.xbutton.x;
            paste_info.y=windows->image.y+event.xbutton.y;
            state|=ExitState;
          }
        break;
      }
      case Expose:
        break;
      case KeyPress:
      {
        char
          command[MaxTextLength];

        KeySym
          key_symbol;

        int
          length;

        if (event.xkey.window != windows->image.id)
          break;
        /*
          Respond to a user key press.
        */
        length=XLookupString((XKeyEvent *) &event.xkey,command,sizeof(command),
          &key_symbol,(XComposeStatus *) NULL);
        *(command+length)='\0';
        if (resource_info->debug)
          (void) fprintf(stderr,"Key press: 0x%lx (%s)\n",key_symbol,command);
        switch (key_symbol)
        {
          case XK_Escape:
          case XK_F20:
          {
            /*
              Prematurely exit.
            */
            DestroyImage(paste_image);
            state|=EscapeState;
            state|=ExitState;
            break;
          }
          case XK_F1:
          case XK_Help:
          {
            XSetFunction(display,windows->image.highlight_context,GXcopy);
            XTextViewWidget(display,resource_info,windows,False,
              "Help Viewer - Image Compositing",ImagePasteHelp);
            XSetFunction(display,windows->image.highlight_context,GXinvert);
            break;
          }
          default:
          {
            XBell(display,0);
            break;
          }
        }
        break;
      }
      case MotionNotify:
      {
        /*
          Discard pending pointer motion events.
        */
        while (XCheckMaskEvent(display,PointerMotionMask,&event));
        x=event.xmotion.x;
        y=event.xmotion.y;
        /*
          Map and unmap Info widget as text cursor crosses its boundaries.
        */
        if (windows->info.mapped)
          {
            if ((x < (windows->info.x+windows->info.width)) &&
                (y < (windows->info.y+windows->info.height)))
              XWithdrawWindow(display,windows->info.id,windows->info.screen);
          }
        else
          if ((x > (windows->info.x+windows->info.width)) ||
              (y > (windows->info.y+windows->info.height)))
            XMapWindow(display,windows->info.id);
        paste_info.x=windows->image.x+x;
        paste_info.y=windows->image.y+y;
        break;
      }
      default:
      {
        if (resource_info->debug)
          (void) fprintf(stderr,"Event type: %d\n",event.type);
        break;
      }
    }
  } while (!(state & ExitState));
  XSetFunction(display,windows->image.highlight_context,GXcopy);
  XSelectInput(display,windows->image.id,windows->image.attributes.event_mask);
  XSetCursorState(display,windows,False);
  XFreeCursor(display,cursor);
  if (state & EscapeState)
    return(True);
  /*
    Image pasting is relative to image configuration.
  */
  XSetCursorState(display,windows,True);
  XCheckRefreshWindows(display,windows);
  x=0;
  y=0;
  width=image->columns;
  height=image->rows;
  if (windows->image.crop_geometry != (char *) NULL)
    (void) XParseGeometry(windows->image.crop_geometry,&x,&y,&width,&height);
  scale_factor=UpShift(width)/windows->image.ximage->width;
  paste_info.x+=x;
  paste_info.x=DownShift(paste_info.x*scale_factor);
  paste_info.width=DownShift(paste_info.width*scale_factor);
  scale_factor=UpShift(height)/windows->image.ximage->height;
  paste_info.y+=y;
  paste_info.y=DownShift(paste_info.y*scale_factor);
  paste_info.height=DownShift(paste_info.height*scale_factor);
  /*
    Paste image with X Image window.
  */
  CompositeImage(image,operator,paste_image,paste_info.x,paste_info.y);
  DestroyImage(paste_image);
  XSetCursorState(display,windows,False);
  /*
    Update image colormap.
  */
  XConfigureImageColormap(display,resource_info,windows,image);
  (void) XConfigureImage(display,resource_info,windows,image);
  return(True);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   X P r i n t I m a g e                                                     %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Function XPrintImage prints an image to a Postscript printer.
%
%  The format of the XPrintImage routine is:
%
%    status=XPrintImage(display,resource_info,windows,image)
%
%  A description of each parameter follows:
%
%    o status: Function XPrintImage return True if the image is
%      printed.  False is returned is there is a memory shortage or if the
%      image fails to print.
%
%    o display: Specifies a connection to an X server; returned from
%      XOpenDisplay.
%
%    o resource_info: Specifies a pointer to a X11 XResourceInfo structure.
%
%    o windows: Specifies a pointer to a XWindows structure.
%
%    o image: Specifies a pointer to a Image structure;  returned from
%      ReadImage.
%
%
*/
static unsigned int XPrintImage
(
    Display*       display,
    XResourceInfo* resource_info,
    XWindows*      windows,
    Image**        image
)
{
  char
    command[MaxTextLength],
    filename[MaxTextLength],
    geometry[MaxTextLength];

  ImageInfo
    *image_info;

  int
    status;

  /*
    Request Postscript page geometry from user.
  */
  image_info=resource_info->image_info;
  (void) sprintf(geometry,PSPageGeometry);
  if (image_info->page != (char *) NULL)
    (void) strcpy(geometry,image_info->page);
  XListBrowserWidget(display,windows,&windows->widget,PageSizes,"Select",
    "Select Postscript Page Geometry:",geometry);
  if (*geometry == '\0')
    return(True);
  image_info->page=PostscriptGeometry(geometry);
  /*
    Request file name from user.
  */
  XCheckRefreshWindows(display,windows);
  TemporaryFilename(filename);
  (void) sprintf(command,resource_info->print_command,filename);
  (void) XDialogWidget(display,windows,"Print","Print command:",command);
  if (*command == '\0')
    return(True);
  /*
    Print image.
  */
  (void) XMagickCommand(display,resource_info,windows,0,(KeySym) XK_A,image);
  XSetCursorState(display,windows,True);
  XCheckRefreshWindows(display,windows);
  (void) sprintf((*image)->filename,"ps:%s",filename);
  status=WriteImage(image_info,*image);
  status&=!SystemCommand(command);
  (void) unlink(filename);
  XSetCursorState(display,windows,False);
  (void) XMagickCommand(display,resource_info,windows,0,(KeySym) XK_u,image);
  return(status);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   X P r o g r e s s M o n i t o r                                           %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Function XProgressMonitor displays the progress a task is making in
%  completing a task.
%
%  The format of the XProgressMonitor routine is:
%
%      XProgressMonitor(task,quantum,span)
%
%  A description of each parameter follows:
%
%    o task: Identifies the task in progress.
%
%    o quantum: Specifies the quantum position within the span which represents
%      how much progress has been made in completing a task.
%
%    o span: Specifies the span relative to completing a task.
%
%
*/
static void XProgressMonitor
(
    char*        task,
    unsigned int quantum,
    unsigned int span
)
{
  XMonitorWidget(display,windows,task,quantum,span);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   X R O I I m a g e                                                         %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Function XROIImage applies an image processing technique to a region
%  of interest.
%
%  The format of the XROIImage routine is:
%
%    status=XROIImage(display,resource_info,windows,image)
%
%  A description of each parameter follows:
%
%    o status: Function XROIImage returns True if the image is
%      cropped.  False is returned is there is a memory shortage or if the
%      image fails to be cropped.
%
%    o display: Specifies a connection to an X server; returned from
%      XOpenDisplay.
%
%    o resource_info: Specifies a pointer to a X11 XResourceInfo structure.
%
%    o windows: Specifies a pointer to a XWindows structure.
%
%    o image: Specifies a pointer to a Image structure; returned from
%      ReadImage.
%
%
*/
static unsigned int XROIImage
(
    Display*       display,
    XResourceInfo* resource_info,
    XWindows*      windows,
    Image**        image
)
{
#define ApplyModeMenus  5
#define ApplyModeFileOp  0
#define ApplyModeEditOp  1
#define ApplyModeTransformOp  2
#define ApplyModeEnhanceOp  3
#define ApplyModeEffectsOp  4
#define ApplyModeHelpOp  5
#define ApplyModeDismissOp  6
#define ROIModeHelpOp  0
#define ROIModeDismissOp  1

  static char
    *ROIModeMenu[]=
    {
      "Help",
      "Dismiss",
      (char *) NULL
    },
    *ApplyModeMenu[]=
    {
      "File",
      "Edit",
      "Transform",
      "Enhance",
      "Effects",
      "Help",
      "Dismiss",
      (char *) NULL
    },
    *FileMenu[]=
    {
      "Image Info",
      (char *) NULL
    },
    *EditMenu[]=
    {
      "Undo",
      "Redo",
      (char *) NULL
    },
    *TransformMenu[]=
    {
      "Flop",
      "Flip",
      "Rotate Right",
      "Rotate Left",
      (char *) NULL
    },
    *EnhanceMenu[]=
    {
      "Hue...",
      "Saturation...",
      "Brightness...",
      "Gamma...",
      "Spiff",
      "Dull",
      "Equalize",
      "Normalize",
      "Negate",
      "Grayscale",
      "Quantize...",
      (char *) NULL
    },
    *EffectsMenu[]=
    {
      "Despeckle",
      "Peak Noise",
      "Sharpen...",
      "Blur...",
      "Edge Detect",
      "Emboss",
      "Spread",
      "Oil Painting",
      "Raise...",
      "Segment...",
      (char *) NULL
    };

  static char
    **Menus[ApplyModeMenus]=
    {
      FileMenu,
      EditMenu,
      TransformMenu,
      EnhanceMenu,
      EffectsMenu
    };

  static KeySym
    ApplyModeKeys[]=
    {
      XK_VoidSymbol,
      XK_VoidSymbol,
      XK_VoidSymbol,
      XK_VoidSymbol,
      XK_VoidSymbol,
      XK_h,
      XK_q
    },
    FileKeys[]=
    {
      XK_i
    },
    EditKeys[]=
    {
      XK_u,
      XK_Redo
    },
    TransformKeys[]=
    {
      XK_bar,
      XK_minus,
      XK_slash,
      XK_backslash
    },
    EnhanceKeys[]=
    {
      XK_F7,
      XK_F8,
      XK_F9,
      XK_g,
      XK_F10,
      XK_F11,
      XK_equal,
      XK_N,
      XK_asciitilde,
      XK_G,
      XK_numbersign
    },
    EffectsKeys[]=
    {
      XK_D,
      XK_P,
      XK_S,
      XK_B,
      XK_E,
      XK_M,
      XK_F13,
      XK_O,
      XK_asciicircum,
      XK_Z
    };

  static KeySym
    *Keys[ApplyModeMenus]=
    {
      FileKeys,
      EditKeys,
      TransformKeys,
      EnhanceKeys,
      EffectsKeys,
    };

  char
    command[MaxTextLength],
    text[MaxTextLength];

  Cursor
    cursor;

  int
    entry,
    id,
    x,
    y;

  KeySym
    key_symbol;

  MonitorHandler
    handler;

  RectangleInfo
    highlight_info,
    roi_info;

  unsigned int
    height,
    width;

  unsigned long
    scale_factor,
    state;

  XEvent
    event;

  /*
    Map Command widget.
  */
  windows->command.name="ROI";
  windows->command.data=0;
  (void) XCommandWidget(display,windows,ROIModeMenu,(XEvent *) NULL);
  XMapRaised(display,windows->command.id);
  XClientMessage(display,windows->image.id,windows->im_protocols,
    windows->im_update_widget,CurrentTime);
  /*
    Track pointer until button 1 is pressed.
  */
  XQueryPosition(display,windows->image.id,&x,&y);
  roi_info.x=windows->image.x+x;
  roi_info.y=windows->image.y+y;
  roi_info.width=0;
  roi_info.height=0;
  cursor=XCreateFontCursor(display,XC_fleur);
  XSelectInput(display,windows->image.id,windows->image.attributes.event_mask |
    PointerMotionMask);
  state=DefaultState;
  do
  {
    if (windows->info.mapped)
      {
        /*
          Display pointer position.
        */
        (void) sprintf(text," %+d%+d ",roi_info.x,roi_info.y);
        XInfoWidget(display,windows,text);
      }
    /*
      Wait for next event.
    */
    XIfEvent(display,&event,XScreenEvent,(char *) windows);
    if (event.xany.window == windows->command.id)
      {
        /*
          Select a command from the Command widget.
        */
        id=XCommandWidget(display,windows,ROIModeMenu,&event);
        if (id < 0)
          continue;
        switch (id)
        {
          case ROIModeHelpOp:
          {
            XTextViewWidget(display,resource_info,windows,False,
              "Help Viewer - Region of Interest",ImageROIHelp);
            break;
          }
          case ROIModeDismissOp:
          {
            /*
              Prematurely exit.
            */
            state|=EscapeState;
            state|=ExitState;
            break;
          }
          default:
            break;
        }
        continue;
      }
    switch (event.type)
    {
      case ButtonPress:
      {
        if (event.xbutton.window == windows->pan.id)
          {
            XPanImage(display,windows,&event);
            XInfoWidget(display,windows,text);
            break;
          }
        if (event.xbutton.button == Button1)
          {
            /*
              Note first corner of region of interest rectangle-- exit loop.
            */
            XDefineCursor(display,windows->image.id,cursor);
            roi_info.x=windows->image.x+event.xbutton.x;
            roi_info.y=windows->image.y+event.xbutton.y;
            state|=ExitState;
            break;
          }
        break;
      }
      case Expose:
        break;
      case KeyPress:
      {
        if (event.xkey.window != windows->image.id)
          break;
        /*
          Respond to a user key press.
        */
        (void) XLookupString((XKeyEvent *) &event.xkey,command,sizeof(command),
          &key_symbol,(XComposeStatus *) NULL);
        switch (key_symbol)
        {
          case XK_Escape:
          case XK_F20:
          {
            /*
              Prematurely exit.
            */
            state|=EscapeState;
            state|=ExitState;
            break;
          }
          case XK_F1:
          case XK_Help:
          {
            XTextViewWidget(display,resource_info,windows,False,
              "Help Viewer - Region of Interest",ImageROIHelp);
            break;
          }
          default:
          {
            XBell(display,0);
            break;
          }
        }
        break;
      }
      case MotionNotify:
      {
        /*
          Discard pending pointer motion events.
        */
        while (XCheckMaskEvent(display,PointerMotionMask,&event));
        x=event.xmotion.x;
        y=event.xmotion.y;
        /*
          Map and unmap Info widget as text cursor crosses its boundaries.
        */
        if (windows->info.mapped)
          {
            if ((x < (windows->info.x+windows->info.width)) &&
                (y < (windows->info.y+windows->info.height)))
              XWithdrawWindow(display,windows->info.id,windows->info.screen);
          }
        else
          if ((x > (windows->info.x+windows->info.width)) ||
              (y > (windows->info.y+windows->info.height)))
            XMapWindow(display,windows->info.id);
        roi_info.x=windows->image.x+x;
        roi_info.y=windows->image.y+y;
        break;
      }
      default:
        break;
    }
  } while (!(state & ExitState));
  XSelectInput(display,windows->image.id,windows->image.attributes.event_mask);
  if (state & EscapeState)
    {
      /*
        User want to exit without region of interest.
      */
      XWithdrawWindow(display,windows->info.id,windows->info.screen);
      XFreeCursor(display,cursor);
      return(True);
    }
  XSetFunction(display,windows->image.highlight_context,GXinvert);
  do
  {
    /*
      Size rectangle as pointer moves until the mouse button is released.
    */
    x=roi_info.x;
    y=roi_info.y;
    roi_info.width=0;
    roi_info.height=0;
    state=DefaultState;
    XSelectInput(display,windows->image.id,
      windows->image.attributes.event_mask | PointerMotionMask);
    do
    {
      highlight_info=roi_info;
      highlight_info.x=roi_info.x-windows->image.x;
      highlight_info.y=roi_info.y-windows->image.y;
      if ((highlight_info.width > 3) && (highlight_info.height > 3))
        {
          /*
            Display info and draw region of interest rectangle.
          */
          if (!windows->info.mapped)
            XMapWindow(display,windows->info.id);
          (void) sprintf(text," %ux%u%+d%+d",roi_info.width,roi_info.height,
            roi_info.x,roi_info.y);
          XInfoWidget(display,windows,text);
          XHighlightRectangle(display,windows->image.id,
            windows->image.highlight_context,&highlight_info);
        }
      else
        if (windows->info.mapped)
          XWithdrawWindow(display,windows->info.id,windows->info.screen);
      /*
        Wait for next event.
      */
      XIfEvent(display,&event,XScreenEvent,(char *) windows);
      if ((highlight_info.width > 3) && (highlight_info.height > 3))
        XHighlightRectangle(display,windows->image.id,
          windows->image.highlight_context,&highlight_info);
      switch (event.type)
      {
        case ButtonPress:
        {
          if (event.xbutton.window == windows->pan.id)
            {
              XPanImage(display,windows,&event);
              XInfoWidget(display,windows,text);
              break;
            }
          roi_info.x=windows->image.x+event.xbutton.x;
          roi_info.y=windows->image.y+event.xbutton.y;
          break;
        }
        case ButtonRelease:
        {
          /*
            User has committed to region of interest rectangle.
          */
          roi_info.x=windows->image.x+event.xbutton.x;
          roi_info.y=windows->image.y+event.xbutton.y;
          XSetCursorState(display,windows,False);
          state|=ExitState;
          if (strcmp(windows->command.name,"Apply") == 0)
            break;
          windows->command.name="Apply";
          windows->command.data=ApplyModeMenus;
          (void) XCommandWidget(display,windows,ApplyModeMenu,(XEvent *) NULL);
          break;
        }
        case Expose:
          break;
        case MotionNotify:
        {
          /*
            Discard pending button motion events.
          */
          while (XCheckMaskEvent(display,ButtonMotionMask,&event));
          roi_info.x=windows->image.x+event.xmotion.x;
          roi_info.y=windows->image.y+event.xmotion.y;
        }
        default:
          break;
      }
      if (((roi_info.x != x) && (roi_info.y != y)) || (state & ExitState))
        {
          /*
            Check boundary conditions.
          */
          if (roi_info.x < 0)
            roi_info.x=0;
          else
            if (roi_info.x > windows->image.ximage->width)
              roi_info.x=windows->image.ximage->width;
          if (roi_info.x < x)
            roi_info.width=(unsigned int) (x-roi_info.x);
          else
            {
              roi_info.width=(unsigned int) (roi_info.x-x);
              roi_info.x=x;
            }
          if (roi_info.y < 0)
            roi_info.y=0;
          else
            if (roi_info.y > windows->image.ximage->height)
              roi_info.y=windows->image.ximage->height;
          if (roi_info.y < y)
            roi_info.height=(unsigned int) (y-roi_info.y);
          else
            {
              roi_info.height=(unsigned int) (roi_info.y-y);
              roi_info.y=y;
            }
        }
    } while (!(state & ExitState));
    XSelectInput(display,windows->image.id,
      windows->image.attributes.event_mask);
    /*
      Wait for user to grab a corner of the rectangle or press return.
    */
    state=DefaultState;
    do
    {
      if (windows->info.mapped)
        {
          /*
            Display pointer position.
          */
          (void) sprintf(text," %ux%u%+d%+d",roi_info.width,roi_info.height,
            roi_info.x,roi_info.y);
          XInfoWidget(display,windows,text);
        }
      highlight_info=roi_info;
      highlight_info.x=roi_info.x-windows->image.x;
      highlight_info.y=roi_info.y-windows->image.y;
      if ((highlight_info.width <= 3) || (highlight_info.height <= 3))
        {
          state|=EscapeState;
          state|=ExitState;
          break;
        }
      if (state & UpdateRegionState)
        {
          XSetFunction(display,windows->image.highlight_context,GXcopy);
          switch (key_symbol)
          {
            case XK_i:
            case XK_u:
            case XK_Undo:
            case XK_Redo:
            {
              (void) XMagickCommand(display,resource_info,windows,0,key_symbol,
                image);
              break;
            }
            default:
            {
              Image
                *roi_image;

              RectangleInfo
                crop_info;

              /*
                Region of interest is relative to image configuration.
              */
              handler=SetMonitorHandler((MonitorHandler) NULL);
              crop_info=roi_info;
              x=0;
              y=0;
              width=(*image)->columns;
              height=(*image)->rows;
              if (windows->image.crop_geometry != (char *) NULL)
                (void) XParseGeometry(windows->image.crop_geometry,&x,&y,
                  &width,&height);
              scale_factor=UpShift(width)/windows->image.ximage->width;
              crop_info.x+=x;
              crop_info.x=DownShift(crop_info.x*scale_factor);
              crop_info.width=DownShift(crop_info.width*scale_factor);
              scale_factor=UpShift(height)/windows->image.ximage->height;
              crop_info.y+=y;
              crop_info.y=DownShift(crop_info.y*scale_factor);
              crop_info.height=DownShift(crop_info.height*scale_factor);
              roi_image=CropImage(*image,&crop_info);
              (void) SetMonitorHandler(handler);
              if (roi_image == (Image *) NULL)
                continue;
              /*
                Apply image processing technique to the region of interest.
              */
              windows->image.orphan=True;
              (void) XMagickCommand(display,resource_info,windows,Mod5Mask,
                key_symbol,&roi_image);
              handler=SetMonitorHandler((MonitorHandler) NULL);
              (void) XMagickCommand(display,resource_info,windows,0,
                (KeySym) XK_Select,image);
              windows->image.orphan=False;
              CompositeImage(*image,ReplaceCompositeOp,roi_image,
                crop_info.x,crop_info.y);
              DestroyImage(roi_image);
              (void) SetMonitorHandler(handler);
              break;
            }
          }
          if (key_symbol != XK_i)
            {
              XConfigureImageColormap(display,resource_info,windows,*image);
              (void) XConfigureImage(display,resource_info,windows,*image);
            }
          XCheckRefreshWindows(display,windows);
          XInfoWidget(display,windows,text);
          XSetFunction(display,windows->image.highlight_context,GXinvert);
          state&=(~UpdateRegionState);
        }
      XHighlightRectangle(display,windows->image.id,
        windows->image.highlight_context,&highlight_info);
      XIfEvent(display,&event,XScreenEvent,(char *) windows);
      if (event.xany.window == windows->command.id)
        {
          /*
            Select a command from the Command widget.
          */
          XSetFunction(display,windows->image.highlight_context,GXcopy);
          key_symbol=XK_VoidSymbol;
          id=XCommandWidget(display,windows,ApplyModeMenu,&event);
          if (id >= 0)
            {
              (void) strcpy(command,ApplyModeMenu[id]);
              key_symbol=ApplyModeKeys[id];
              if (id < ApplyModeMenus)
                {
                  /*
                    Select a command from a pop-up menu.
                  */
                  entry=XMenuWidget(display,windows,ApplyModeMenu[id],Menus[id],
                    command);
                  if (entry >= 0)
                    {
                      (void) strcpy(command,Menus[id][entry]);
                      key_symbol=Keys[id][entry];
                    }
                }
            }
          XSetFunction(display,windows->image.highlight_context,GXinvert);
          XHighlightRectangle(display,windows->image.id,
            windows->image.highlight_context,&highlight_info);
          if (key_symbol == XK_h)
            {
              XSetFunction(display,windows->image.highlight_context,GXcopy);
              XTextViewWidget(display,resource_info,windows,False,
                "Help Viewer - Region of Interest",ImageROIHelp);
              XSetFunction(display,windows->image.highlight_context,GXinvert);
              continue;
            }
          if (key_symbol == XK_q)
            {
              /*
                Exit.
              */
              state|=EscapeState;
              state|=ExitState;
              continue;
            }
          if (key_symbol != XK_VoidSymbol)
            state|=UpdateRegionState;
          continue;
        }
      XHighlightRectangle(display,windows->image.id,
        windows->image.highlight_context,&highlight_info);
      switch (event.type)
      {
        case ButtonPress:
        {
          if (event.xbutton.window == windows->pan.id)
            {
              XPanImage(display,windows,&event);
              XInfoWidget(display,windows,text);
              break;
            }
          if (event.xbutton.window != windows->image.id)
            break;
          x=windows->image.x+event.xbutton.x;
          y=windows->image.y+event.xbutton.y;
          if ((x < (roi_info.x+RoiDelta)) && (x > (roi_info.x-RoiDelta)) &&
              (y < (roi_info.y+RoiDelta)) && (y > (roi_info.y-RoiDelta)))
            {
              roi_info.x=roi_info.x+roi_info.width;
              roi_info.y=roi_info.y+roi_info.height;
              state|=UpdateConfigurationState;
              break;
            }
          if ((x < (roi_info.x+RoiDelta)) && (x > (roi_info.x-RoiDelta)) &&
              (y < (roi_info.y+roi_info.height+RoiDelta)) &&
              (y > (roi_info.y+roi_info.height-RoiDelta)))
            {
              roi_info.x=roi_info.x+roi_info.width;
              state|=UpdateConfigurationState;
              break;
            }
          if ((x < (roi_info.x+roi_info.width+RoiDelta)) &&
              (x > (roi_info.x+roi_info.width-RoiDelta)) &&
              (y < (roi_info.y+RoiDelta)) && (y > (roi_info.y-RoiDelta)))
            {
              roi_info.y=roi_info.y+roi_info.height;
              state|=UpdateConfigurationState;
              break;
            }
          if ((x < (roi_info.x+roi_info.width+RoiDelta)) &&
              (x > (roi_info.x+roi_info.width-RoiDelta)) &&
              (y < (roi_info.y+roi_info.height+RoiDelta)) &&
              (y > (roi_info.y+roi_info.height-RoiDelta)))
            {
              state|=UpdateConfigurationState;
              break;
            }
        }
        case ButtonRelease:
          break;
        case Expose:
        {
          if (event.xexpose.window == windows->image.id)
            if (event.xexpose.count == 0)
              {
                event.xexpose.x=highlight_info.x;
                event.xexpose.y=highlight_info.y;
                event.xexpose.width=highlight_info.width;
                event.xexpose.height=highlight_info.height;
                XRefreshWindow(display,&windows->image,&event);
              }
          if (event.xexpose.window == windows->info.id)
            if (event.xexpose.count == 0)
              XInfoWidget(display,windows,text);
          break;
        }
        case KeyPress:
        {
          if (event.xkey.window != windows->image.id)
            break;
          /*
            Respond to a user key press.
          */
          (void) XLookupString((XKeyEvent *) &event.xkey,command,
            sizeof(command),&key_symbol,(XComposeStatus *) NULL);
          for (id=0; id < ApplyModeMenus; id++)
          {
            for (entry=0; Menus[id][entry] != (char *) NULL; entry++)
              if (key_symbol == Keys[id][entry])
                {
                  state|=UpdateRegionState;
                  break;
                }
            if (state & UpdateRegionState)
              break;
          }
          if (state & UpdateRegionState)
            break;
          switch (key_symbol)
          {
            case XK_Shift_L:
            case XK_Shift_R:
              break;
            case XK_Escape:
            case XK_F20:
              state|=EscapeState;
            case XK_Return:
            {
              state|=ExitState;
              break;
            }
            case XK_F1:
            case XK_Help:
            {
              XSetFunction(display,windows->image.highlight_context,GXcopy);
              XTextViewWidget(display,resource_info,windows,False,
                "Help Viewer - Region of Interest",ImageROIHelp);
              XSetFunction(display,windows->image.highlight_context,GXinvert);
              break;
            }
            default:
            {
              XBell(display,0);
              break;
            }
          }
          break;
        }
        case KeyRelease:
          break;
        case MotionNotify:
        {
          /*
            Discard pending pointer motion events.
          */
          while (XCheckMaskEvent(display,ButtonMotionMask,&event));
          x=event.xmotion.x;
          y=event.xmotion.y;
          /*
            Map and unmap Info widget as text cursor crosses its boundaries.
          */
          if (windows->info.mapped)
            {
              if ((x < (windows->info.x+windows->info.width)) &&
                  (y < (windows->info.y+windows->info.height)))
                XWithdrawWindow(display,windows->info.id,windows->info.screen);
            }
          else
            if ((x > (windows->info.x+windows->info.width)) ||
                (y > (windows->info.y+windows->info.height)))
              XMapWindow(display,windows->info.id);
          break;
        }
        default:
          break;
      }
      if (state & UpdateConfigurationState)
        {
          XPutBackEvent(display,&event);
          XDefineCursor(display,windows->image.id,cursor);
          break;
        }
    } while (!(state & ExitState));
  } while (!(state & ExitState));
  XSetFunction(display,windows->image.highlight_context,GXcopy);
  XSetCursorState(display,windows,False);
  if (state & EscapeState)
    return(True);
  return(True);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   X R o t a t e I m a g e                                                   %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Function XRotateImage rotates the X image.  If the degrees parameter
%  if zero, the rotation angle is computed from the slope of a line drawn by
%  the user.
%
%  The format of the XRotateImage routine is:
%
%    status=XRotateImage(display,resource_info,windows,degrees,image)
%
%  A description of each parameter follows:
%
%    o status: Function XRotateImage return True if the image is
%      rotated.  False is returned is there is a memory shortage or if the
%      image fails to rotate.
%
%    o display: Specifies a connection to an X server; returned from
%      XOpenDisplay.
%
%    o resource_info: Specifies a pointer to a X11 XResourceInfo structure.
%
%    o windows: Specifies a pointer to a XWindows structure.
%
%    o degrees: Specifies the number of degrees to rotate the image.
%
%    o image: Specifies a pointer to a Image structure;  returned from
%      ReadImage.
%
%
*/
static unsigned int XRotateImage
(
    Display*       display,
    XResourceInfo* resource_info,
    XWindows*      windows,
    double         degrees,
    Image**        image
)
{
#define RotateModeColorOp  0
#define RotateModeDirectionOp  1
#define RotateModeCropOp  2
#define RotateModeSharpenOp  3
#define RotateModeHelpOp  4
#define RotateModeDismissOp  5
#define HorizontalRotateOp  0
#define VerticalRotateOp  1

  static char
    *RotateModeMenu[]=
    {
      "Pixel Color",
      "Direction",
      "Crop",
      "Sharpen",
      "Help",
      "Dismiss",
      (char *) NULL
    };

  char
    command[MaxTextLength],
    text[MaxTextLength];

  ColorPacket
    background;

  double
    normalized_degrees;

  Image
    *rotated_image;

  int
    id,
    x,
    y;

  register int
    i;

  static unsigned int
    crop = False,
    direction = HorizontalRotateOp,
    pen_id = 0,
    sharpen = True;

  unsigned int
    height,
    rotations,
    width;

  if (degrees == 0.0)
    {
      unsigned int
        distance;

      unsigned long
        state;

      XEvent
        event;

      XSegment
        rotate_info;

      /*
        Map Command widget.
      */
      windows->command.name="Rotate";
      windows->command.data=4;
      (void) XCommandWidget(display,windows,RotateModeMenu,(XEvent *) NULL);
      XMapRaised(display,windows->command.id);
      XClientMessage(display,windows->image.id,windows->im_protocols,
        windows->im_update_widget,CurrentTime);
      /*
        Wait for first button press.
      */
      XSetFunction(display,windows->image.highlight_context,GXinvert);
      XSelectInput(display,windows->image.id,
        windows->image.attributes.event_mask | PointerMotionMask);
      XQueryPosition(display,windows->image.id,&x,&y);
      rotate_info.x1=x;
      rotate_info.y1=y;
      rotate_info.x2=x;
      rotate_info.y2=y;
      state=DefaultState;
      do
      {
        XHighlightLine(display,windows->image.id,
          windows->image.highlight_context,&rotate_info);
        /*
          Wait for next event.
        */
        XIfEvent(display,&event,XScreenEvent,(char *) windows);
        XHighlightLine(display,windows->image.id,
          windows->image.highlight_context,&rotate_info);
        if (event.xany.window == windows->command.id)
          {
            /*
              Select a command from the Command widget.
            */
            id=XCommandWidget(display,windows,RotateModeMenu,&event);
            if (id < 0)
              continue;
            XSetFunction(display,windows->image.highlight_context,GXcopy);
            switch (id)
            {
              case RotateModeColorOp:
              {
                char
                  *ColorMenu[MaxNumberPens];

                int
                  pen_number;

                XColor
                  color;

                /*
                  Initialize menu selections.
                */
                for (i=0; i < (MaxNumberPens-2); i++)
                  ColorMenu[i]=resource_info->pen_colors[i];
                ColorMenu[MaxNumberPens-2]="Browser...";
                ColorMenu[MaxNumberPens-1]=(char *) NULL;
                /*
                  Select a pen color from the pop-up menu.
                */
                pen_number=XMenuWidget(display,windows,RotateModeMenu[id],
                  ColorMenu,command);
                if (pen_number < 0)
                  break;
                if (pen_number == (MaxNumberPens-2))
                  {
                    static char
                      color_name[MaxTextLength] = "gray";

                    /*
                      Select a pen color from a dialog.
                    */
                    resource_info->pen_colors[pen_number]=color_name;
                    XColorBrowserWidget(display,windows,"Select",color_name);
                    if (*color_name == '\0')
                      break;
                  }
                /*
                  Set pen color.
                */
                (void) XParseColor(display,windows->image.map_info->colormap,
                  resource_info->pen_colors[pen_number],&color);
                XBestPixel(display,windows->image.map_info->colormap,
                  (XColor *) NULL,(unsigned int) MaxColors,&color);
                windows->image.pixel_info->pen_colors[pen_number]=color;
                pen_id=pen_number;
                break;
              }
              case RotateModeDirectionOp:
              {
                static char
                  *Directions[]=
                  {
                    "horizontal",
                    "vertical",
                    (char *) NULL,
                  };

                /*
                  Select a command from the pop-up menu.
                */
                direction=XMenuWidget(display,windows,RotateModeMenu[id],
                  Directions,command);
                break;
              }
              case RotateModeCropOp:
              {
                static char
                  *Options[]=
                  {
                    "false",
                    "true",
                    (char *) NULL,
                  };

                /*
                  Select a command from the pop-up menu.
                */
                crop=XMenuWidget(display,windows,RotateModeMenu[id],
                  Options,command);
                break;
              }
              case RotateModeSharpenOp:
              {
                static char
                  *Options[]=
                  {
                    "false",
                    "true",
                    (char *) NULL,
                  };

                /*
                  Select a command from the pop-up menu.
                */
                sharpen=XMenuWidget(display,windows,RotateModeMenu[id],
                  Options,command);
                break;
              }
              case RotateModeHelpOp:
              {
                XTextViewWidget(display,resource_info,windows,False,
                  "Help Viewer - Image Rotation",ImageRotateHelp);
                break;
              }
              case RotateModeDismissOp:
              {
                /*
                  Prematurely exit.
                */
                state|=EscapeState;
                state|=ExitState;
                break;
              }
              default:
                break;
            }
            XSetFunction(display,windows->image.highlight_context,GXinvert);
            continue;
          }
        switch (event.type)
        {
          case ButtonPress:
          {
            if (event.xbutton.window == windows->pan.id)
              {
                XPanImage(display,windows,&event);
                XInfoWidget(display,windows,text);
                break;
              }
            XSetFunction(display,windows->image.highlight_context,GXcopy);
            /*
              Exit loop.
            */
            rotate_info.x1=event.xbutton.x;
            rotate_info.y1=event.xbutton.y;
            state|=ExitState;
            break;
          }
          case ButtonRelease:
            break;
          case Expose:
            break;
          case KeyPress:
          {
            char
              command[MaxTextLength];

            KeySym
              key_symbol;

            if (event.xkey.window != windows->image.id)
              break;
            /*
              Respond to a user key press.
            */
            (void) XLookupString((XKeyEvent *) &event.xkey,command,
              sizeof(command),&key_symbol,(XComposeStatus *) NULL);
            switch (key_symbol)
            {
              case XK_Escape:
              case XK_F20:
              {
                /*
                  Prematurely exit.
                */
                state|=EscapeState;
                state|=ExitState;
                break;
              }
              case XK_F1:
              case XK_Help:
              {
                XSetFunction(display,windows->image.highlight_context,GXcopy);
                XTextViewWidget(display,resource_info,windows,False,
                  "Help Viewer - Image Rotation",ImageRotateHelp);
                XSetFunction(display,windows->image.highlight_context,GXinvert);
                break;
              }
              default:
              {
                XBell(display,0);
                break;
              }
            }
            break;
          }
          case MotionNotify:
          {
            /*
              Discard pending pointer motion events.
            */
            while (XCheckMaskEvent(display,PointerMotionMask,&event));
            rotate_info.x1=event.xmotion.x;
            rotate_info.y1=event.xmotion.y;
          }
        }
        rotate_info.x2=rotate_info.x1;
        rotate_info.y2=rotate_info.y1;
        if (direction == HorizontalRotateOp)
          rotate_info.x2+=32;
        else
          rotate_info.y2-=32;
      } while (!(state & ExitState));
      XSelectInput(display,windows->image.id,
        windows->image.attributes.event_mask);
      XSetFunction(display,windows->image.highlight_context,GXcopy);
      XWithdrawWindow(display,windows->info.id,windows->info.screen);
      if (state & EscapeState)
        return(True);
      /*
        Draw line as pointer moves until the mouse button is released.
      */
      distance=0;
      XSetFunction(display,windows->image.highlight_context,GXinvert);
      state=DefaultState;
      do
      {
        if (distance > 9)
          {
            /*
              Display info and draw rotation line.
            */
            if (!windows->info.mapped)
              XMapWindow(display,windows->info.id);
            (void) sprintf(text," %.2f",
              direction == VerticalRotateOp ? degrees-90.0 : degrees);
            XInfoWidget(display,windows,text);
            XHighlightLine(display,windows->image.id,
              windows->image.highlight_context,&rotate_info);
          }
        else
          if (windows->info.mapped)
            XWithdrawWindow(display,windows->info.id,windows->info.screen);
        /*
          Wait for next event.
        */
        XIfEvent(display,&event,XScreenEvent,(char *) windows);
        if (distance > 9)
          XHighlightLine(display,windows->image.id,
            windows->image.highlight_context,&rotate_info);
        switch (event.type)
        {
          case ButtonPress:
            break;
          case ButtonRelease:
          {
            /*
              User has committed to rotation line.
            */
            rotate_info.x2=event.xbutton.x;
            rotate_info.y2=event.xbutton.y;
            state|=ExitState;
            break;
          }
          case Expose:
            break;
          case MotionNotify:
          {
            /*
              Discard pending button motion events.
            */
            while (XCheckMaskEvent(display,ButtonMotionMask,&event));
            rotate_info.x2=event.xmotion.x;
            rotate_info.y2=event.xmotion.y;
          }
          default:
            break;
        }
        /*
          Check boundary conditions.
        */
        if (rotate_info.x2 < 0)
          rotate_info.x2=0;
        else
          if (rotate_info.x2 > windows->image.width)
            rotate_info.x2=windows->image.width;
        if (rotate_info.y2 < 0)
          rotate_info.y2=0;
        else
          if (rotate_info.y2 > windows->image.height)
            rotate_info.y2=windows->image.height;
        /*
          Compute rotation angle from the slope of the line.
        */
        degrees=0.0;
        distance=
          ((rotate_info.x2-rotate_info.x1+1)*(rotate_info.x2-rotate_info.x1+1))+
          ((rotate_info.y2-rotate_info.y1+1)*(rotate_info.y2-rotate_info.y1+1));
        if (distance > 9)
          degrees=RadiansToDegrees(-atan2((double) (rotate_info.y2-
            rotate_info.y1),(double) (rotate_info.x2-rotate_info.x1)));
      } while (!(state & ExitState));
      XSetFunction(display,windows->image.highlight_context,GXcopy);
      XWithdrawWindow(display,windows->info.id,windows->info.screen);
      if (distance <= 9)
        return(True);
    }
  if (direction == VerticalRotateOp)
    degrees-=90.0;
  if (degrees == 0.0)
    return(True);
  /*
    Rotate image.
  */
  normalized_degrees=degrees;
  while (normalized_degrees < -45.0)
    normalized_degrees+=360.0;
  for (rotations=0; normalized_degrees > 45.0; rotations++)
    normalized_degrees-=90.0;
  if (normalized_degrees != 0.0)
    (void) XMagickCommand(display,resource_info,windows,0,(KeySym) XK_A,image);
  XSetCursorState(display,windows,True);
  XCheckRefreshWindows(display,windows);
  background.red=XDownScale(windows->image.pixel_info->pen_colors[pen_id].red);
  background.green=
    XDownScale(windows->image.pixel_info->pen_colors[pen_id].green);
  background.blue=
    XDownScale(windows->image.pixel_info->pen_colors[pen_id].blue);
  background.index=0;
  rotated_image=RotateImage(*image,degrees,&background,crop,sharpen);
  XSetCursorState(display,windows,False);
  if (rotated_image == (Image *) NULL)
    return(False);
  DestroyImage(*image);
  *image=rotated_image;
  if (windows->image.crop_geometry != (char *) NULL)
    {
      /*
        Rotate crop geometry.
      */
      (void) XParseGeometry(windows->image.crop_geometry,&x,&y,&width,&height);
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
          (void) sprintf(windows->image.crop_geometry,"%ux%u%+d%+d",
            height,width,(int) (*image)->columns-(int) height-y,x);
          break;
        }
        case 2:
        {
          /*
            Rotate 180 degrees.
          */
          (void) sprintf(windows->image.crop_geometry,"%ux%u%+d%+d",
            width,height,(int) width-x,(int) height-y);
          break;
        }
        case 3:
        {
          /*
            Rotate 270 degrees.
          */
          (void) sprintf(windows->image.crop_geometry,"%ux%u%+d%+d",
            height,width,y,(int) (*image)->rows-(int) width-x);
          break;
        }
      }
    }
  if (windows->image.orphan)
    return(True);
  if (normalized_degrees != 0.0)
    {
      /*
        Update image colormap.
      */
      windows->image.window_changes.width=(*image)->columns;
      windows->image.window_changes.height=(*image)->rows;
      if (windows->image.crop_geometry != (char *) NULL)
        {
          /*
            Obtain dimensions of image from crop geometry.
          */
          (void) XParseGeometry(windows->image.crop_geometry,&x,&y,
            &width,&height);
          windows->image.window_changes.width=width;
          windows->image.window_changes.height=height;
        }
      XConfigureImageColormap(display,resource_info,windows,*image);
    }
  else
    if (((rotations % 4) == 1) || ((rotations % 4) == 3))
      {
        windows->image.window_changes.width=windows->image.ximage->height;
        windows->image.window_changes.height=windows->image.ximage->width;
      }
  /*
    Update image configuration.
  */
  (void) XConfigureImage(display,resource_info,windows,*image);
  return(True);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   X S a v e I m a g e                                                       %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Function XSaveImage saves an image to a file.
%
%  The format of the XSaveImage routine is:
%
%    status=XSaveImage(display,resource_info,windows,image)
%
%  A description of each parameter follows:
%
%    o status: Function XSaveImage return True if the image is
%      written.  False is returned is there is a memory shortage or if the
%      image fails to write.
%
%    o display: Specifies a connection to an X server; returned from
%      XOpenDisplay.
%
%    o resource_info: Specifies a pointer to a X11 XResourceInfo structure.
%
%    o windows: Specifies a pointer to a XWindows structure.
%
%    o image: Specifies a pointer to a Image structure;  returned from
%      ReadImage.
%
%
*/
static unsigned int XSaveImage
(
    Display*       display,
    XResourceInfo* resource_info,
    XWindows*      windows,
    Image**        image
)
{
  char
    filename[MaxTextLength];

  ImageInfo
    *image_info;

  int
    status;

  register char
    *p;

  /*
    Request file name from user.
  */
  p=(*image)->filename+strlen((*image)->filename)-1;
  while ((p > (*image)->filename) && (*(p-1) != *BasenameSeparator))
    p--;
  (void) strcpy(filename,p);
  if (resource_info->write_filename != (char *) NULL)
    (void) strcpy(filename,resource_info->write_filename);
  XFileBrowserWidget(display,windows,"Save",filename);
  if (*filename == '\0')
    return(True);
  if (access(filename,0) == 0)
    {
      /*
        File exists-- seek user's permission before overwriting.
      */
      status=XConfirmWidget(display,windows,"Overwrite",filename);
      if (status == False)
        return(True);
    }
  image_info=resource_info->image_info;
  (void) strcpy(image_info->filename,filename);
  SetImageMagick(resource_info->image_info);
  if ((strcmp(image_info->magick,"JPEG") == 0) ||
      (strcmp(image_info->magick,"JPG") == 0))
    {
      char
        quality[MaxTextLength];

      /*
        Request JPEG quality from user.
      */
      (void) sprintf(quality,"%u",image_info->quality);
      status=XDialogWidget(display,windows,"Save","Enter JPEG quality:",
        quality);
      if (*quality == '\0')
        return(True);
      image_info->quality=atoi(quality);
      image_info->interlace=status ? PlaneInterlace : NoneInterlace;
    }
  if ((strcmp(image_info->magick,"EPS") == 0) ||
      (strcmp(image_info->magick,"PS") == 0) ||
      (strcmp(image_info->magick,"PS2") == 0))
    {
      char
        geometry[MaxTextLength];

      /*
        Request Postscript page geometry from user.
      */
      (void) sprintf(geometry,PSPageGeometry);
      if (image_info->page != (char *) NULL)
        (void) strcpy(geometry,image_info->page);
      XListBrowserWidget(display,windows,&windows->widget,PageSizes,"Select",
        "Select Postscript Page Geometry:",geometry);
      if (*geometry != '\0')
        image_info->page=PostscriptGeometry(geometry);
    }
  /*
    Write image.
  */
  (void) XMagickCommand(display,resource_info,windows,0,(KeySym) XK_A,image);
  XSetCursorState(display,windows,True);
  XCheckRefreshWindows(display,windows);
  (void) strcpy((*image)->filename,filename);
  status=WriteImage(image_info,*image);
  XSetCursorState(display,windows,False);
  (void) XMagickCommand(display,resource_info,windows,0,(KeySym) XK_u,image);
  return(status);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   X S c r e e n E v e n t                                                   %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Function XScreenEvent returns True if the certain events on the X server
%  queue is associated with the image or Magnify window.
%
%  The format of the XScreenEvent function is:
%
%      XScreenEvent(display,event,data)
%
%  A description of each parameter follows:
%
%    o display: Specifies a pointer to the Display structure;  returned from
%      XOpenDisplay.
%
%    o event: Specifies a pointer to a X11 XEvent structure.
%
%    o data: Specifies a pointer to a XWindows structure.
%
%
*/
static int XScreenEvent(Display* display, XEvent* event, char* data)
{
  register int
    x,
    y;

  register XWindows
    *windows;

  windows=(XWindows *) data;
  if (event->xany.window == windows->command.id)
    return(True);
  switch (event->type)
  {
    case ButtonPress:
    case ButtonRelease:
    {
      if ((event->xbutton.button == Button3) &&
          (event->xbutton.state & Mod1Mask))
        {
          /*
            Convert Alt-Button3 to Button2.
          */
          event->xbutton.button=Button2;
          event->xbutton.state&=(~Mod1Mask);
        }
      if (event->xbutton.window == windows->backdrop.id)
        XSetInputFocus(display,event->xbutton.window,RevertToParent,
          event->xbutton.time);
      if (event->xbutton.window == windows->image.id)
        if (windows->magnify.mapped)
          {
            /*
              Update magnified image.
            */
            x=event->xbutton.x;
            y=event->xbutton.y;
            if (x < 0)
              x=0;
            else
              if (x >= windows->image.width)
                x=windows->image.width-1;
            windows->magnify.x=windows->image.x+x;
            if (y < 0)
              y=0;
            else
             if (y >= windows->image.height)
               y=windows->image.height-1;
            windows->magnify.y=windows->image.y+y;
            XMakeMagnifyImage(display,windows);
          }
      return(True);
    }
    case ClientMessage:
    {
      /*
        If client window delete message, exit.
      */
      if (event->xclient.message_type != windows->wm_protocols)
        break;
      if (*event->xclient.data.l != windows->wm_delete_window)
        break;
      if (event->xclient.window == windows->magnify.id)
        {
          XWithdrawWindow(display,windows->magnify.id,windows->magnify.screen);
          return(True);
        }
      break;
    }
    case Expose:
    {
      if (event->xexpose.window == windows->image.id)
        {
          XRefreshWindow(display,&windows->image,event);
          return(True);
        }
      if (event->xexpose.window == windows->pan.id)
        if (event->xexpose.count == 0)
          {
            XDrawPanRectangle(display,windows);
            return(True);
          }
      break;
    }
    case KeyPress:
    {
      char
        command[MaxTextLength];

      KeySym
        key_symbol;

      /*
        Respond to a user key press.
      */
      (void) XLookupString((XKeyEvent *) &event->xkey,command,sizeof(command),
        &key_symbol,(XComposeStatus *) NULL);
      if (event->xkey.window == windows->magnify.id)
        XMagnifyWindowCommand(display,windows,event->xkey.state,key_symbol);
      return(True);
    }
    case MapNotify:
    {
      if (event->xmap.window == windows->magnify.id)
        {
          XMakeMagnifyImage(display,windows);
          windows->magnify.mapped=True;
          return(True);
        }
      if (event->xmap.window == windows->info.id)
        {
          windows->info.mapped=True;
          return(True);
        }
      break;
    }
    case MotionNotify:
    {
      if (event->xmotion.window == windows->image.id)
        if (windows->magnify.mapped)
          {
            /*
              Update magnified image.
            */
            x=event->xmotion.x;
            y=event->xmotion.y;
            if (x < 0)
              x=0;
            else
              if (x >= windows->image.width)
                x=windows->image.width-1;
            windows->magnify.x=windows->image.x+x;
            if (y < 0)
              y=0;
            else
             if (y >= windows->image.height)
               y=windows->image.height-1;
            windows->magnify.y=windows->image.y+y;
            XMakeMagnifyImage(display,windows);
          }
      return(True);
    }
    case UnmapNotify:
    {
      if (event->xunmap.window == windows->magnify.id)
        {
          windows->magnify.mapped=False;
          return(True);
        }
      if (event->xunmap.window == windows->info.id)
        {
          windows->info.mapped=False;
          return(True);
        }
      break;
    }
    case KeyRelease:
    case SelectionNotify:
      return(True);
    default:
      break;
  }
  return(False);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   X S e t C r o p G e o m e t r y                                           %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Function XSetCropGeometry accepts a cropping geometry relative to the
%  Image window and translates it to a cropping geometry relative to the
%  image.
%
%  The format of the XSetCropGeometry routine is:
%
%    XSetCropGeometry(display,windows,crop_info,image)
%
%  A description of each parameter follows:
%
%    o display: Specifies a connection to an X server; returned from
%      XOpenDisplay.
%
%    o windows: Specifies a pointer to a XWindows structure.
%
%    o crop_info:  A pointer to a RectangleInfo that defines a region of the
%      Image window to crop.
%
%    o image: Specifies a pointer to a Image structure.
%
%
*/
static void XSetCropGeometry
(
    Display*       display,
    XWindows*      windows,
    RectangleInfo* crop_info,
    Image*         image
)
{
  char
    text[MaxTextLength];

  int
    x,
    y;

  unsigned int
    height,
    width;

  unsigned long
    scale_factor;

  if (windows->info.mapped)
    {
      /*
        Display info on cropping rectangle.
      */
      (void) sprintf(text," %ux%u%+d%+d",crop_info->width,crop_info->height,
        crop_info->x,crop_info->y);
      XInfoWidget(display,windows,text);
    }
  /*
    Cropping geometry is relative to any previous crop geometry.
  */
  x=0;
  y=0;
  width=image->columns;
  height=image->rows;
  if (windows->image.crop_geometry != (char *) NULL)
    (void) XParseGeometry(windows->image.crop_geometry,&x,&y,&width,&height);
  else
    {
      /*
        Allocate crop geometry string.
      */
      windows->image.crop_geometry=(char *) malloc(MaxTextLength*sizeof(char));
      if (windows->image.crop_geometry == (char *) NULL)
        Error("Unable to crop X image",windows->image.name);
    }
  /*
    Define the crop geometry string from the cropping rectangle.
  */
  scale_factor=UpShift(width)/windows->image.ximage->width;
  if (crop_info->x > 0)
    x+=DownShift(crop_info->x*scale_factor);
  width=DownShift(crop_info->width*scale_factor);
  if (width == 0)
    width=1;
  scale_factor=UpShift(height)/windows->image.ximage->height;
  if (crop_info->y > 0)
    y+=DownShift(crop_info->y*scale_factor);
  height=DownShift(crop_info->height*scale_factor);
  if (height == 0)
    height=1;
  (void) sprintf(windows->image.crop_geometry,"%ux%u%+d%+d",width,height,x,y);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   X T i l e I m a g e                                                       %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Function XTileImage loads or deletes a selected tile from a visual
%  image directory.  The load or delete command is choosen from a menu.
%
%  The format of the XTileImage routine is:
%
%    tiled_image=XTileImage(display,resource_info,windows,image,event)
%
%  A description of each parameter follows:
%
%    o tiled_image:  XTileImage reads or deletes the tiled image
%      and returns it.  A null image is returned if an error occurs.
%
%    o display: Specifies a connection to an X server;  returned from
%      XOpenDisplay.
%
%    o resource_info: Specifies a pointer to a X11 XResourceInfo structure.
%
%    o windows: Specifies a pointer to a XWindows structure.
%
%    o image: Specifies a pointer to a Image structure; returned from
%      ReadImage.
%
%    o event: Specifies a pointer to a XEvent structure.  If it is NULL,
%      the entire image is refreshed.
%
%
*/
static Image* XTileImage
(
    Display*       display,
    XResourceInfo* resource_info,
    XWindows*      windows,
    Image*         image,
    XEvent*        event
)
{
#define LoadVerbOp  0
#define NextVerbOp  1
#define FormerVerbOp  2
#define DeleteVerbOp  3
#define UpdateVerbOp  4

  char
    command[MaxTextLength],
    filename[MaxTextLength];

  Image
    *tiled_image;

  int
    status,
    tile,
    verb,
    x,
    y;

  register char
    *p,
    *q;

  register int
    i;

  static char
    *VerbMenu[]=
    {
      "Load",
      "Next",
      "Former",
      "Delete",
      "Update",
      (char *) NULL,
    };

  unsigned int
    height,
    width;

  unsigned long
    scale_factor;

  /*
    Tile image is relative to montage image configuration.
  */
  x=0;
  y=0;
  width=image->columns;
  height=image->rows;
  if (windows->image.crop_geometry != (char *) NULL)
    (void) XParseGeometry(windows->image.crop_geometry,&x,&y,&width,&height);
  scale_factor=UpShift(width)/windows->image.ximage->width;
  event->xbutton.x+=windows->image.x;
  event->xbutton.x=DownShift(event->xbutton.x*scale_factor)+x;
  scale_factor=UpShift(height)/windows->image.ximage->height;
  event->xbutton.y+=windows->image.y;
  event->xbutton.y=DownShift(event->xbutton.y*scale_factor)+y;
  /*
    Determine size and location of each tile in the visual image directory.
  */
  x=0;
  y=0;
  width=image->columns;
  height=image->rows;
  (void) XParseGeometry(image->montage,&x,&y,&width,&height);
  tile=((event->xbutton.y-y)/height)*((image->columns-x)/width)+
    (event->xbutton.x-x)/width;
  if (tile < 0)
    {
      /*
        Button press is outside any tile.
      */
      XBell(display,0);
      return((Image *) NULL);
    }
  /*
    Determine file name from the tile directory.
  */
  p=image->directory;
  for (i=tile; (i != 0) && (*p != '\0'); )
  {
    if (*p == '\n')
      i--;
    p++;
  }
  if (*p == '\0')
    {
      /*
        Button press is outside any tile.
      */
      XBell(display,0);
      return((Image *) NULL);
    }
  /*
    Select a command from the pop-up menu.
  */
  verb=XMenuWidget(display,windows,"Tile Verb",VerbMenu,command);
  if (verb < 0)
    return((Image *) NULL);
  q=p;
  while ((*q != '\n') && (*q != '\0'))
    q++;
  (void) strncpy(filename,p,q-p);
  filename[q-p]='\0';
  /*
    Perform command for the selected tile.
  */
  XSetCursorState(display,windows,True);
  XCheckRefreshWindows(display,windows);
  tiled_image=(Image *) NULL;
  switch (verb)
  {
    case LoadVerbOp:
    {
      /*
        Load tile image.
      */
      XCheckRefreshWindows(display,windows);
      (void) strcpy(resource_info->image_info->filename,filename);
      tiled_image=ReadImage(resource_info->image_info);
      XWithdrawWindow(display,windows->info.id,windows->info.screen);
      break;
    }
    case NextVerbOp:
    {
      /*
        Display next image.
      */
      XClientMessage(display,windows->image.id,windows->im_protocols,
        windows->im_next_image,CurrentTime);
      break;
    }
    case FormerVerbOp:
    {
      /*
        Display former image.
      */
      XClientMessage(display,windows->image.id,windows->im_protocols,
        windows->im_former_image,CurrentTime);
      break;
    }
    case DeleteVerbOp:
    {
      /*
        Delete tile image.
      */
      if (access(filename,0) != 0)
        {
          XNoticeWidget(display,windows,"Image file does not exist:",filename);
          break;
        }
      status=XConfirmWidget(display,windows,"Really delete tile",filename);
      if (status == False)
        break;
      status=unlink(filename);
      if (status != False)
        {
          XNoticeWidget(display,windows,"Unable to delete image file:",
            filename);
          break;
        }
    }
    case UpdateVerbOp:
    {
      int
        x_offset,
        y_offset;

      register int
        j;

      register RunlengthPacket
        *r;

      /*
        Ensure all the images exist.
      */
      if (!UncompressImage(image))
        return((Image *) NULL);
      tile=0;
      for (p=image->directory; *p != '\0'; p++)
      {
        q=p;
        while ((*q != '\n') && (*q != '\0'))
          q++;
        (void) strncpy(filename,p,q-p);
        filename[q-p]='\0';
        p=q;
        if (access(filename,0) == 0)
          {
            tile++;
            continue;
          }
        /*
          Overwrite tile with background color.
        */
        x_offset=width*(tile % ((image->columns-x)/width))+x;
        y_offset=height*(tile/((image->columns-x)/width))+y;
        for (i=0; i < height; i++)
        {
          r=image->pixels+((y_offset+i)*image->columns+x_offset);
          for (j=0; j < width; j++)
            *r++=(*image->pixels);
        }
        tile++;
      }
      windows->image.window_changes.width=image->columns;
      windows->image.window_changes.height=image->rows;
      XConfigureImageColormap(display,resource_info,windows,image);
      (void) XConfigureImage(display,resource_info,windows,image);
      break;
    }
    default:
      break;
  }
  XSetCursorState(display,windows,False);
  return(tiled_image);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   X T r a n s l a t e I m a g e                                             %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Function XTranslateImage translates the image within an Image window
%  by one pixel as specified by the key symbol.  If the image has a `montage'
%  string the translation is respect to the width and height contained within
%  the string.
%
%  The format of the XTranslateImage routine is:
%
%    XTranslateImage(display,windows,image,key_symbol)
%
%  A description of each parameter follows:
%
%    o display: Specifies a connection to an X server; returned from
%      XOpenDisplay.
%
%    o windows: Specifies a pointer to a XWindows structure.
%
%    o image: Specifies a pointer to a Image structure;  returned from
%      ReadImage.
%
%    o key_symbol: Specifies a KeySym which indicates which side of the image
%      to trim.
%
%
*/
static void XTranslateImage
(
    Display*  display,
    XWindows* windows,
    Image*    image,
    KeySym    key_symbol
)
{
  char
    text[MaxTextLength];

  int
    x,
    y;

  unsigned int
    x_offset,
    y_offset;

  /*
    User specified a pan position offset.
  */
  x_offset=windows->image.width;
  y_offset=windows->image.height;
  if (image->montage != (char *) NULL)
    (void) XParseGeometry(image->montage,&x,&y,&x_offset,&y_offset);
  switch (key_symbol)
  {
    case XK_Home:
    case XK_KP_Home:
    {
      windows->image.x=windows->image.width >> 1;
      windows->image.y=windows->image.height >> 1;
      break;
    }
    case XK_Left:
    case XK_KP_Left:
    {
      windows->image.x-=x_offset;
      break;
    }
    case XK_Next:
    case XK_Up:
    case XK_KP_Up:
    {
      windows->image.y-=y_offset;
      break;
    }
    case XK_Right:
    case XK_KP_Right:
    {
      windows->image.x+=x_offset;
      break;
    }
    case XK_Prior:
    case XK_Down:
    case XK_KP_Down:
    {
      windows->image.y+=y_offset;
      break;
    }
    default:
      return;
  }
  /*
    Check boundary conditions.
  */
  if (windows->image.x < 0)
    windows->image.x=0;
  else
    if ((windows->image.x+windows->image.width) > windows->image.ximage->width)
      windows->image.x=windows->image.ximage->width-windows->image.width;
  if (windows->image.y < 0)
    windows->image.y=0;
  else
    if ((windows->image.y+windows->image.height) >
         windows->image.ximage->height)
      windows->image.y=windows->image.ximage->height-windows->image.height;
  /*
    Refresh Image window.
  */
  (void) sprintf(text," %ux%u%+d%+d ",windows->image.width,
    windows->image.height,windows->image.x,windows->image.y);
  XInfoWidget(display,windows,text);
  XCheckRefreshWindows(display,windows);
  XDrawPanRectangle(display,windows);
  XRefreshWindow(display,&windows->image,(XEvent *) NULL);
  XWithdrawWindow(display,windows->info.id,windows->info.screen);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   X T r i m I m a g e                                                       %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Function XTrimImage trims the edges from the Image window.
%
%  The format of the XTrimImage routine is:
%
%    status=XTrimImage(display,resource_info,windows,image)
%
%  A description of each parameter follows:
%
%    o status: Function XTrimImage returns True if the image is
%      cropped.  False is returned is there is a memory shortage or if the
%      image fails to be cropped.
%
%    o display: Specifies a connection to an X server; returned from
%      XOpenDisplay.
%
%    o resource_info: Specifies a pointer to a X11 XResourceInfo structure.
%
%    o windows: Specifies a pointer to a XWindows structure.
%    o windows: Specifies a pointer to a XWindows structure.
%
%    o image: Specifies a pointer to a Image structure.
%
%
*/
static unsigned int XTrimImage
(
    Display*       display,
    XResourceInfo* resource_info,
    XWindows*      windows,
    Image*         image
)
{
  RectangleInfo
    trim_info;

  register int
    x,
    y;

  unsigned long
    background,
    pixel;

  /*
    Trim edges from image.
  */
  XSetCursorState(display,windows,True);
  XCheckRefreshWindows(display,windows);
  /*
    Crop the left edge.
  */
  background=XGetPixel(windows->image.ximage,0,0);
  trim_info.width=windows->image.ximage->width;
  for (x=0; x < windows->image.ximage->width; x++)
  {
    for (y=0; y < windows->image.ximage->height; y++)
    {
      pixel=XGetPixel(windows->image.ximage,x,y);
      if (pixel != background)
        break;
    }
    if (y < windows->image.ximage->height)
      break;
  }
  trim_info.x=x;
  if (trim_info.x == windows->image.ximage->width)
    {
      XSetCursorState(display,windows,False);
      return(False);
    }
  /*
    Crop the right edge.
  */
  background=XGetPixel(windows->image.ximage,windows->image.ximage->width-1,0);
  for (x=windows->image.ximage->width-1; x > 0; x--)
  {
    for (y=0; y < windows->image.ximage->height; y++)
    {
      pixel=XGetPixel(windows->image.ximage,x,y);
      if (pixel != background)
        break;
    }
    if (y < windows->image.ximage->height)
      break;
  }
  trim_info.width=x-trim_info.x+1;
  /*
    Crop the top edge.
  */
  background=XGetPixel(windows->image.ximage,0,0);
  trim_info.height=windows->image.ximage->height;
  for (y=0; y < windows->image.ximage->height; y++)
  {
    for (x=0; x < windows->image.ximage->width; x++)
    {
      pixel=XGetPixel(windows->image.ximage,x,y);
      if (pixel != background)
        break;
    }
    if (x < windows->image.ximage->width)
      break;
  }
  trim_info.y=y;
  /*
    Crop the bottom edge.
  */
  background=XGetPixel(windows->image.ximage,0,windows->image.ximage->height-1);
  for (y=windows->image.ximage->height-1; y > 0; y--)
  {
    for (x=0; x < windows->image.ximage->width; x++)
    {
      pixel=XGetPixel(windows->image.ximage,x,y);
      if (pixel != background)
        break;
    }
    if (x < windows->image.ximage->width)
      break;
  }
  trim_info.height=y-trim_info.y+1;
  if ((trim_info.width != windows->image.width) ||
      (trim_info.height != windows->image.height))
    {
      /*
        Reconfigure Image window as defined by the trimming rectangle.
      */
      XSetCropGeometry(display,windows,&trim_info,image);
      windows->image.window_changes.width=trim_info.width;
      windows->image.window_changes.height=trim_info.height;
      (void) XConfigureImage(display,resource_info,windows,image);
    }
  XSetCursorState(display,windows,False);
  return(True);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   X V i s u a l D i r e c t o r y I m a g e                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Function XVisualDirectoryImage creates a Visual Image Directory.
%
%  The format of the XVisualDirectoryImage routine is:
%
%    loaded_image=XVisualDirectoryImage(display,resource_info,windows)
%
%  A description of each parameter follows:
%
%    o loaded_image: Function XVisualDirectoryImage returns a visual image
%      directory if it can be created successfully.  Otherwise a null image
%      is returned.
%
%    o display: Specifies a connection to an X server; returned from
%      XOpenDisplay.
%
%    o resource_info: Specifies a pointer to a X11 XResourceInfo structure.
%
%    o windows: Specifies a pointer to a XWindows structure.
%
%
*/
static Image* XVisualDirectoryImage
(
    Display*       display,
    XResourceInfo* resource_info,
    XWindows*      windows
)
{
#define LoadImageText  "  Loading images...  "
#define XClientName  "montage"

  char
    **filelist,
    *resource_value,
    window_id[MaxTextLength];

  Image
    *image,
    **images,
    *scaled_image;

  ImageInfo
    local_info;

  int
    number_files;

  MonitorHandler
    handler;

  register int
    i,
    j;

  static char
    filename[MaxTextLength] = "*";

  unsigned int
    backdrop,
    height,
    width;

  XMontageInfo
    vid_info;

  XResourceInfo
    background_resources,
    vid_resources;

  XrmDatabase
    resource_database;

  /*
    Request file name from user.
  */
  XFileBrowserWidget(display,windows,"Directory",filename);
  if (*filename == '\0')
    return((Image *) NULL);
  /*
    Expand the filename.
  */
  filelist=(char **) malloc(sizeof(char *));
  if (filelist == (char **) NULL)
    {
      Warning("Memory allocation error",(char *) NULL);
      return((Image *) NULL);
    }
  number_files=1;
  filelist[0]=filename;
  ExpandFilenames(&number_files,&filelist);
  if (number_files == 0)
    {
      Warning("No image files were found",filename);
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
    Get any Visual Image Directory X resources.
  */
  resource_database=resource_info->resource_database;
  vid_resources=(*resource_info);
  vid_resources.background_color=XGetResourceInstance(resource_database,
    XClientName,"background",DefaultTileBackground);
  resource_value=XGetResourceClass(resource_database,XClientName,"borderWidth",
    DefaultTileBorderwidth);
  vid_resources.border_width=atoi(resource_value);
  vid_resources.font=resource_info->image_info->font;
  vid_resources.foreground_color=XGetResourceInstance(resource_database,
    XClientName,"foreground",DefaultTileForeground);
  vid_resources.image_geometry=XGetResourceInstance(resource_database,
    XClientName,"imageGeometry",DefaultTileGeometry);
  vid_resources.matte_color=XGetResourceInstance(resource_database,
    XClientName,"mattecolor",DefaultTileMatte);
  /*
    Set image background resources.
  */
  background_resources=(*resource_info);
  background_resources.window_id=window_id;
  (void) sprintf(background_resources.window_id,"0x%lx",windows->image.id);
  background_resources.backdrop=True;
  /*
    Read each image and convert them to a tile.
  */
  XSetCursorState(display,windows,True);
  XCheckRefreshWindows(display,windows);
  backdrop=(windows->image.visual_info->class == TrueColor) ||
   (windows->image.visual_info->class == DirectColor);
  j=0;
  for (i=0; i < number_files; i++)
  {
    handler=SetMonitorHandler((MonitorHandler) NULL);
    local_info=(*resource_info->image_info);
    local_info.filename=filelist[i];
    *local_info.magick='\0';
    if (local_info.size == (char *) NULL)
      local_info.size=vid_resources.image_geometry;
    image=ReadImage(&local_info);
    if (filelist[i] != filename)
      free((char *) filelist[i]);
    if (image == (Image *) NULL)
      continue;
    image->scene=j;
    image->matte=False;
    LabelImage(image,"%f");
    /*
      Scale image.
    */
    width=image->columns;
    height=image->rows;
    ParseImageGeometry(vid_resources.image_geometry,&width,&height);
    scaled_image=ScaleImage(image,width,height);
    DestroyImage(image);
    if (scaled_image == (Image *) NULL)
      continue;
    image=scaled_image;
    if (backdrop)
      (void) XDisplayBackgroundImage(display,&background_resources,image);
    images[j]=image;
    j++;
    (void) SetMonitorHandler(handler);
    ProgressMonitor(LoadImageText,i,number_files);
  }
  free((char *) filelist);
  if (j == 0)
    {
      XSetCursorState(display,windows,False);
      Warning("No images were loaded",filename);
      free((char *) images);
      return((Image *) NULL);
    }
  /*
    Create the Visual Image Directory.
  */
  XGetMontageInfo(&vid_info);
  vid_info.number_tiles=j;
  image=XMontageImage(images,&vid_resources,&vid_info,filename);
  XSetCursorState(display,windows,False);
  free((char *) images);
  if (image != (Image *) NULL)
    XClientMessage(display,windows->image.id,windows->im_protocols,
      windows->im_next_image,CurrentTime);
  return(image);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   X W a r n i n g                                                           %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Function XWarning displays a warning message in a Notice widget.
%
%  The format of the XWarning routine is:
%
%      XWarning(message,qualifier)
%
%  A description of each parameter follows:
%
%    o message: Specifies the message to display before terminating the
%      program.
%
%    o qualifier: Specifies any qualifier to the message.
%
%
*/
static void XWarning(char* message, char* qualifier)
{
  char
    text[MaxTextLength];

  if (message == (char *) NULL)
    return;
  (void) strcpy(text,message);
  (void) strcat(text,":");
  XNoticeWidget(display,windows,text,qualifier);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%    M a i n                                                                  %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%
*/

/* Kobus -> hack main to get im_display */
int im_display(Image* image, ImageInfo* image_info_ptr)
{
  int argc;
  char *argv[ 1 ];
/* END Kobus */


#define NotInitialized  (unsigned int) (~0)

  char
    density[MaxTextLength],
    *option,
    *resource_value,
    *server_name;

  Image

#ifdef IGNORE_THIS_CODE     /* Kobus */
    *image,
#endif                      /* End Kobus */

    *next_image;

  ImageInfo
    image_info;

  int
    x;

  register int
    i,
    j;

  unsigned int
    first_scene,
    *image_marker,
    image_number,
    last_scene,
    scene,
    x_density,
    y_density;

  unsigned long
    state;

  XResourceInfo
    resource_info;

  XrmDatabase
    resource_database;


  /* Kobus */
  image_info = *image_info_ptr;
  argv[ 0 ]  = strdup("ImageMagick");
  argc=1;
  /* End Kobus */


  /*
    Set defaults.
  */
  client_name=ClientName(*argv);
  display=(Display *) NULL;
  first_scene=0;
  image_marker=(unsigned int *) malloc((argc+1)*sizeof(unsigned int));
  if (image_marker == (unsigned int *) NULL)
    Error("Unable to display image","Memory allocation failed");
  for (i=0; i <= argc; i++)
    image_marker[i]=argc;
  image_number=0;


#ifdef IGNORE_THIS_CODE     /* Kobus */
  GetImageInfo(&image_info);
#endif                      /* End Kobus */


  last_scene=0;
  resource_database=(XrmDatabase) NULL;
  server_name=(char *) NULL;
  state=DefaultState;


#ifdef IGNORE_THIS_CODE     /* Kobus */
  /*
    Check for server name specified on the command line.
  */
  ExpandFilenames(&argc,&argv);
  for (i=1; i < argc; i++)
  {
    /*
      Check command line for server name.
    */
    option=argv[i];
    if (((int) strlen(option) == 1) || ((*option != '-') && (*option != '+')))
      continue;
    if (strncmp("display",option+1,3) == 0)
      {
        /*
          User specified server name.
        */
        i++;
        if (i == argc)
          Error("Missing server name on -display",(char *) NULL);
        server_name=argv[i];
        break;
      }
    if (strncmp("help",option+1,2) == 0)
      Usage(True);
  }
#endif                      /* End Kobus */


  /*
    Open X server connection.
  */
  display=XOpenDisplay(server_name);
  if (display == (Display *) NULL)
    Error("Unable to connect to X server",XDisplayName(server_name));
  /*
    Set our forgiving error handler.
  */
  XSetErrorHandler(XError);
  /*
    Get user defaults from X resource database.
  */
  resource_database=XGetResourceDatabase(display,client_name);
  XGetResourceInfo(resource_database,client_name,&resource_info);
  x_density=(unsigned int)(((((double) XDisplayWidth(display,XDefaultScreen(display)))*25.4)/
    ((double) XDisplayWidthMM(display,XDefaultScreen(display))))+0.5);
  y_density=(unsigned int)(((((double) XDisplayHeight(display,XDefaultScreen(display)))*25.4)/
    ((double) XDisplayHeightMM(display,XDefaultScreen(display))))+0.5);
  (void) sprintf(density,"%ux%u",x_density,y_density);
  image_info.density=
    XGetResourceClass(resource_database,client_name,"density",density);
  resource_value=
    XGetResourceClass(resource_database,client_name,"interlace","none");
  image_info.interlace=UndefinedInterlace;
  if (Latin1Compare("none",resource_value) == 0)
    image_info.interlace=NoneInterlace;
  if (Latin1Compare("line",resource_value) == 0)
    image_info.interlace=LineInterlace;
  if (Latin1Compare("plane",resource_value) == 0)
    image_info.interlace=PlaneInterlace;
  if (image_info.interlace == UndefinedInterlace)
    Warning("Unrecognized interlace type",resource_value);
  image_info.page=XGetResourceClass(resource_database,client_name,
    "pageGeometry",(char *) NULL);
  resource_value=
    XGetResourceClass(resource_database,client_name,"quality","75");
  image_info.quality=atoi(resource_value);
  resource_value=
    XGetResourceClass(resource_database,client_name,"verbose","False");
  image_info.verbose=IsTrue(resource_value);


#ifdef IGNORE_THIS_CODE     /* Kobus */
  /*
    Parse command line.
  */
  for (i=1; ((i <= argc) && !(state & ExitState)); i++)
  {
    if (i < argc)
      option=argv[i];
    else
      if (image_number != 0)
        break;
      else
        if (isatty(STDIN_FILENO))
          option="logo:";
        else
          option="-";
    if (((int) strlen(option) > 1) && ((*option == '-') || (*option == '+')))
      switch (*(option+1))
      {
        case 'b':
        {
          if (strncmp("backdrop",option+1,5) == 0)
            {
              resource_info.backdrop=(*option == '-');
              break;
            }
          if ((strncmp("background",option+1,5) == 0) ||
              (strncmp("bg",option+1,2) == 0))
            {
              resource_info.background_color=(char *) NULL;
              if (*option == '-')
                {
                  i++;
                  if (i == argc)
                    Error("Missing color on -background",(char *) NULL);
                  resource_info.background_color=argv[i];
                }
              break;
            }
          if (strcmp("border",option+1) == 0)
            {
              if (*option == '-')
                {
                  i++;
                  if ((i == argc) || !sscanf(argv[i],"%d",&x))
                    Error("Missing geometry on -border",(char *) NULL);
                }
              break;
            }
          if (strncmp("bordercolor",option+1,7) == 0)
            {
              resource_info.border_color=(char *) NULL;
              if (*option == '-')
                {
                  i++;
                  if (i == argc)
                    Error("Missing color on -bordercolor",(char *) NULL);
                  resource_info.border_color=argv[i];
                }
              break;
            }
          if (strncmp("borderwidth",option+1,7) == 0)
            {
              resource_info.border_width=0;
              if (*option == '-')
                {
                  i++;
                  if ((i == argc) || !sscanf(argv[i],"%d",&x))
                    Error("Missing width on -borderwidth",(char *) NULL);
                  resource_info.border_width=atoi(argv[i]);
                }
              break;
            }
            if (strncmp("box",option+1,3) == 0)
              {
                if (*option == '-')
                  {
                    i++;
                    if (i == argc)
                      Error("Missing color on -box",(char *) NULL);
                  }
                break;
              }
          Error("Unrecognized option",option);
          break;
        }
        case 'c':
        {
          if (strncmp("colormap",option+1,6) == 0)
            {
              resource_info.colormap=PrivateColormap;
              if (*option == '-')
                {
                  i++;
                  if (i == argc)
                    Error("Missing type on -colormap",(char *) NULL);
                  option=argv[i];
                  resource_info.colormap=UndefinedColormap;
                  if (Latin1Compare("private",option) == 0)
                    resource_info.colormap=PrivateColormap;
                  if (Latin1Compare("shared",option) == 0)
                    resource_info.colormap=SharedColormap;
                  if (resource_info.colormap == UndefinedColormap)
                    Error("Invalid colormap type on -colormap",option);
                }
              break;
            }
          if (strncmp("colors",option+1,7) == 0)
            {
              resource_info.number_colors=0;
              if (*option == '-')
                {
                  i++;
                  if ((i == argc) || !sscanf(argv[i],"%d",&x))
                    Error("Missing colors on -colors",(char *) NULL);
                  resource_info.number_colors=atoi(argv[i]);
                }
              break;
            }
          if (strncmp("colorspace",option+1,7) == 0)
            {
              resource_info.colorspace=RGBColorspace;
              if (*option == '-')
                {
                  i++;
                  if (i == argc)
                    Error("Missing type on -colorspace",(char *) NULL);
                  option=argv[i];
                  resource_info.colorspace=UndefinedColorspace;
                  if (Latin1Compare("gray",option) == 0)
                    {
                      resource_info.colorspace=GRAYColorspace;
                      resource_info.number_colors=256;
                      resource_info.tree_depth=8;
                    }
                  if (Latin1Compare("ohta",option) == 0)
                    resource_info.colorspace=OHTAColorspace;
                  if (Latin1Compare("rgb",option) == 0)
                    resource_info.colorspace=RGBColorspace;
                  if (Latin1Compare("xyz",option) == 0)
                    resource_info.colorspace=XYZColorspace;
                  if (Latin1Compare("ycbcr",option) == 0)
                    resource_info.colorspace=YCbCrColorspace;
                  if (Latin1Compare("yiq",option) == 0)
                    resource_info.colorspace=YIQColorspace;
                  if (Latin1Compare("ypbpr",option) == 0)
                    resource_info.colorspace=YPbPrColorspace;
                  if (Latin1Compare("yuv",option) == 0)
                    resource_info.colorspace=YUVColorspace;
                  if (resource_info.colorspace == UndefinedColorspace)
                    Error("Invalid colorspace type on -colorspace",option);
                }
              break;
            }
          if (strncmp("comment",option+1,4) == 0)
            {
              if (*option == '-')
                {
                  i++;
                  if (i == argc)
                    Error("Missing comment on -comment",(char *) NULL);
                }
              break;
            }
          if (strncmp("compress",option+1,3) == 0)
            {
              if (*option == '-')
                {
                  i++;
                  if (i == argc)
                    Error("Missing type on -compress",(char *) NULL);
                  option=argv[i];
                  if (Latin1Compare("runlengthencoded",option) == 0)
                    break;
                  else
                    if (Latin1Compare("zlib",option) == 0)
                      break;
                    else
                      Error("Invalid compression type on -compress",option);
                }
              break;
            }
          if (strncmp("contrast",option+1,3) == 0)
            break;
          if (strncmp("crop",option+1,2) == 0)
            {
              if (*option == '-')
                {
                  i++;
                  if ((i == argc) || !sscanf(argv[i],"%d",&x))
                    Error("Missing geometry on -crop",(char *) NULL);
                }
              break;
            }
          Error("Unrecognized option",option);
          break;
        }
        case 'd':
        {
          if (strncmp("debug",option+1,3) == 0)
            {
              resource_info.debug=(*option == '-');
              break;
            }
          if (strncmp("delay",option+1,3) == 0)
            {
              resource_info.delay=0;
              if (*option == '-')
                {
                  i++;
                  if ((i == argc) || !sscanf(argv[i],"%d",&x))
                    Error("Missing seconds on -delay",(char *) NULL);
                  resource_info.delay=atoi(argv[i]);
                }
              break;
            }
          if (strncmp("density",option+1,3) == 0)
            {
              image_info.density=(char *) NULL;
              if (*option == '-')
                {
                  i++;
                  if ((i == argc) || !sscanf(argv[i],"%d",&x))
                    Error("Missing geometry on -density",(char *) NULL);
                  image_info.density=argv[i];
                }
              break;
            }
          if (strncmp("despeckle",option+1,3) == 0)
            break;
          if (strncmp("display",option+1,3) == 0)
            {
              server_name=(char *) NULL;
              if (*option == '-')
                {
                  i++;
                  if (i == argc)
                    Error("Missing server name on -display",(char *) NULL);
                  server_name=argv[i];
                }
              resource_info.server_name=server_name;
              break;
            }
          if (strncmp("dither",option+1,3) == 0)
            {
              resource_info.dither=(*option == '-');
              break;
            }
          Error("Unrecognized option",option);
          break;
        }
        case 'e':
        {
          if (strncmp("edge",option+1,2) == 0)
            {
              if (*option == '-')
                {
                  i++;
                  if ((i == argc) || !sscanf(argv[i],"%f",(float *) &x))
                    Error("Missing factor on -edge",(char *) NULL);
                }
              break;
            }
          if (strncmp("enhance",option+1,2) == 0)
            break;
          if (strncmp("equalize",option+1,2) == 0)
            break;
          Error("Unrecognized option",option);
          break;
        }
        case 'f':
        {
          if (strncmp("flip",option+1,3) == 0)
            break;
          if (strncmp("flop",option+1,3) == 0)
            break;
          if (strncmp("font",option+1,3) == 0)
            {
              resource_info.font=(char *) NULL;
              if (*option == '-')
                {
                  i++;
                  if (i == argc)
                    Error("Missing font name on -font",(char *) NULL);
                  resource_info.font=argv[i];
                }
              break;
            }
          if ((strncmp("foreground",option+1,3) == 0) ||
              (strncmp("fg",option+1,2) == 0))
           {
             resource_info.foreground_color=(char *) NULL;
             if (*option == '-')
               {
                 i++;
                 if (i == argc)
                   Error("Missing foreground on -foreground",(char *) NULL);
                 resource_info.foreground_color=argv[i];
               }
              break;
           }
          if (strncmp("frame",option+1,2) == 0)
            {
              if (*option == '-')
                {
                  i++;
                  if ((i == argc) || !sscanf(argv[i],"%d",&x))
                    Error("Missing geometry on -frame",(char *) NULL);
                }
              break;
            }
          Error("Unrecognized option",option);
          break;
        }
        case 'g':
        {
          if (strncmp("gamma",option+1,2) == 0)
            {
              if (*option == '-')
                {
                  i++;
                  if ((i == argc) || !sscanf(argv[i],"%f",(float *) &x))
                    Error("Missing value on -gamma",(char *) NULL);
                }
              break;
            }
          if (strncmp("geometry",option+1,2) == 0)
            {
              resource_info.image_geometry=(char *) NULL;
              if (*option == '-')
                {
                  i++;
                  if ((i == argc) || !sscanf(argv[i],"%d",&x))
                    Error("Missing geometry on -geometry",(char *) NULL);
                  resource_info.image_geometry=argv[i];
                }
              break;
            }
          Error("Unrecognized option",option);
          break;
        }
        case 'h':
        {
          if (strncmp("help",option+1,2) == 0)
            Usage(True);
          Error("Unrecognized option",option);
          break;
        }
        case 'i':
        {
          if (strncmp("iconGeometry",option+1,5) == 0)
            {
              resource_info.icon_geometry=(char *) NULL;
              if (*option == '-')
                {
                  i++;
                  if ((i == argc) || !sscanf(argv[i],"%d",&x))
                    Error("Missing geometry on -iconGeometry",(char *) NULL);
                  resource_info.icon_geometry=argv[i];
                }
              break;
            }
          if (strncmp("iconic",option+1,5) == 0)
            {
              resource_info.iconic=(*option == '-');
              break;
            }
          if (strncmp("immutable",option+1,5) == 0)
            {
              resource_info.immutable=(*option == '-');
              break;
            }
          if (strncmp("interlace",option+1,3) == 0)
            {
              image_info.interlace=NoneInterlace;
              if (*option == '-')
                {
                  i++;
                  if (i == argc)
                    Error("Missing type on -interlace",(char *) NULL);
                  option=argv[i];
                  image_info.interlace=UndefinedInterlace;
                  if (Latin1Compare("none",option) == 0)
                    image_info.interlace=NoneInterlace;
                  if (Latin1Compare("line",option) == 0)
                    image_info.interlace=LineInterlace;
                  if (Latin1Compare("plane",option) == 0)
                    image_info.interlace=PlaneInterlace;
                  if (image_info.interlace == UndefinedInterlace)
                    Error("Invalid interlace type on -interlace",option);
                }
              break;
            }
          Error("Unrecognized option",option);
          break;
        }
        case 'l':
        {
          if (strncmp("label",option+1,2) == 0)
            {
              if (*option == '-')
                {
                  i++;
                  if (i == argc)
                    Error("Missing label name on -label",(char *) NULL);
                }
              break;
            }
          Error("Unrecognized option",option);
          break;
        }
        case 'm':
        {
          if (strncmp("magnify",option+1,3) == 0)
            {
              resource_info.magnify=2;
              if (*option == '-')
                {
                  i++;
                  if ((i == argc) || !sscanf(argv[i],"%d",&x))
                    Error("Missing level on -magnify",(char *) NULL);
                  resource_info.magnify=atoi(argv[i]);
                }
              break;
            }
          if (strncmp("map",option+1,3) == 0)
            {
              resource_info.map_type=(char *) NULL;
              if (*option == '-')
                {
                  i++;
                  if (i == argc)
                    Error("Missing map type on -map",(char *) NULL);
                  resource_info.map_type=argv[i];
                }
              break;
            }
          if (strcmp("matte",option+1) == 0)
            break;
          if (strncmp("mattecolor",option+1,6) == 0)
            {
              resource_info.matte_color=(char *) NULL;
              if (*option == '-')
                {
                  i++;
                  if (i == argc)
                    Error("Missing color on -mattecolor",(char *) NULL);
                  resource_info.matte_color=argv[i];
                }
              break;
            }
          if (strncmp("modulate",option+1,3) == 0)
            {
              if (*option == '-')
                {
                  i++;
                  if ((i == argc) || !sscanf(argv[i],"%f",(float *) &x))
                    Error("Missing value on -modulate",(char *) NULL);
                }
              break;
            }
          if (strncmp("monochrome",option+1,3) == 0)
            {
              resource_info.monochrome=(*option == '-');
              if (resource_info.monochrome)
                {
                  resource_info.number_colors=2;
                  resource_info.tree_depth=8;
                  resource_info.colorspace=GRAYColorspace;
                }
              break;
            }
          Error("Unrecognized option",option);
          break;
        }
        case 'n':
        {
          if (strncmp("name",option+1,2) == 0)
            {
              resource_info.name=(char *) NULL;
              if (*option == '-')
                {
                  i++;
                  if (i == argc)
                    Error("Missing name on -name",(char *) NULL);
                  resource_info.name=argv[i];
                }
              break;
            }
          if (strncmp("negate",option+1,2) == 0)
            break;
          if (strncmp("noise",option+1,3) == 0)
            break;
          if (strncmp("normalize",option+1,3) == 0)
            break;
          Error("Unrecognized option",option);
          break;
        }
        case 'o':
        {
          if (strncmp("opaque",option+1,2) == 0)
            {
              if (*option == '-')
                {
                  i++;
                  if (i == argc)
                    Error("Missing color on -opaque",(char *) NULL);
                }
              break;
            }
          Error("Unrecognized option",option);
          break;
        }
        case 'p':
        {
          if (strncmp("page",option+1,3) == 0)
            {
              image_info.page=(char *) NULL;
              if (*option == '-')
                {
                  i++;
                  if (i == argc)
                    Error("Missing page geometry on -page",(char *) NULL);
                  image_info.page=PostscriptGeometry(argv[i]);
                }
              break;
            }
          if (strncmp("pen",option+1,2) == 0)
            {
              if (*option == '-')
                {
                  i++;
                  if (i == argc)
                    Error("Missing color on -pen",(char *) NULL);
                }
              break;
            }
          Error("Unrecognized option",option);
          break;
        }
        case 'q':
        {
          if (strncmp("quality",option+1,2) == 0)
            {
              image_info.quality=0;
              if (*option == '-')
                {
                  i++;
                  if ((i == argc) || !sscanf(argv[i],"%d",&x))
                    Error("Missing quality on -quality",(char *) NULL);
                  image_info.quality=atoi(argv[i]);
                }
              break;
            }
          Error("Unrecognized option",option);
          break;
        }
        case 'r':
        {
          if (strncmp("raise",option+1,2) == 0)
            {
              i++;
              if ((i == argc) || !sscanf(argv[i],"%d",&x))
                Error("Missing bevel width on -raise",(char *) NULL);
              break;
            }
          if (strncmp("roll",option+1,3) == 0)
            {
              if (*option == '-')
                {
                  i++;
                  if ((i == argc) || !sscanf(argv[i],"%d",&x))
                    Error("Missing geometry on -roll",(char *) NULL);
                }
              break;
            }
          if (strncmp("rotate",option+1,3) == 0)
            {
              if (*option == '-')
                {
                  i++;
                  if ((i == argc) || !sscanf(argv[i],"%f",(float *) &x))
                    Error("Missing degrees on -rotate",(char *) NULL);
                }
              break;
            }
          Error("Unrecognized option",option);
          break;
        }
        case 's':
        {
          if (strncmp("sample",option+1,2) == 0)
            {
              if (*option == '-')
                {
                  i++;
                  if ((i == argc) || !sscanf(argv[i],"%d",&x))
                    Error("Missing geometry on -sample",(char *) NULL);
                }
              break;
            }
          if (strncmp("scene",option+1,3) == 0)
            {
              first_scene=0;
              last_scene=0;
              if (*option == '-')
                {
                  i++;
                  if ((i == argc) || !sscanf(argv[i],"%d",&x))
                    Error("Missing scene number on -scene",(char *) NULL);
                  first_scene=atoi(argv[i]);
                  last_scene=first_scene;
                  (void) sscanf(argv[i],"%u-%u",&first_scene,&last_scene);
                }
              break;
            }
          if (strncmp("segment",option+1,3) == 0)
            {
              if (*option == '-')
                {
                  i++;
                  if ((i == argc) || !sscanf(argv[i],"%f",(float *) &x))
                    Error("Missing threshold on -segment",(char *) NULL);
                }
              break;
            }
          if (strncmp("sharpen",option+1,3) == 0)
            {
              if (*option == '-')
                {
                  i++;
                  if ((i == argc) || !sscanf(argv[i],"%f",(float *) &x))
                    Error("Missing factor on -sharpen",(char *) NULL);
                }
              break;
            }
          if (strncmp("shear",option+1,3) == 0)
            {
              if (*option == '-')
                {
                  i++;
                  if ((i == argc) || !sscanf(argv[i],"%f",(float *) &x))
                    Error("Missing shear geometry on -shear",(char *) NULL);
                }
              break;
            }
          if (strncmp("shared_memory",option+1,4) == 0)
            {
              resource_info.use_shared_memory=(*option == '-');
              break;
            }
          if (strncmp("size",option+1,2) == 0)
            {
              image_info.size=(char *) NULL;
              if (*option == '-')
                {
                  i++;
                  if ((i == argc) || !sscanf(argv[i],"%d",&x))
                    Error("Missing geometry on -size",(char *) NULL);
                  image_info.size=argv[i];
                }
              break;
            }
          if (strncmp("spread",option+1,2) == 0)
            {
              if (*option == '-')
                {
                  i++;
                  if ((i == argc) || !sscanf(argv[i],"%d",&x))
                    Error("Missing amount on -spread",(char *) NULL);
                }
              break;
            }
          Error("Unrecognized option",option);
          break;
        }
        case 't':
        {
          if (strncmp("text_font",option+1,3) == 0)
            {
              resource_info.text_font=(char *) NULL;
              if (*option == '-')
                {
                  i++;
                  if (i == argc)
                    Error("Missing font name on -text_font",(char *) NULL);
                  resource_info.text_font=argv[i];
                }
              break;
            }
          if (strncmp("texture",option+1,5) == 0)
            {
              image_info.texture=(char *) NULL;
              if (*option == '-')
                {
                  i++;
                  if (i == argc)
                    Error("Missing filename on -texture",(char *) NULL);
                  image_info.texture=argv[i];
                }
              break;
            }
          if (strncmp("title",option+1,2) == 0)
            {
              resource_info.title=(char *) NULL;
              if (*option == '-')
                {
                  i++;
                  if (i == argc)
                    Error("Missing title on -title",(char *) NULL);
                  resource_info.title=argv[i];
                }
              break;
            }
          if (strncmp("transparent",option+1,3) == 0)
            {
              if (*option == '-')
                {
                  i++;
                  if (i == argc)
                    Error("Missing color on -transparent",(char *) NULL);
                }
              break;
            }
          if (strncmp("treedepth",option+1,3) == 0)
            {
              resource_info.tree_depth=0;
              if (*option == '-')
                {
                  i++;
                  if ((i == argc) || !sscanf(argv[i],"%d",&x))
                    Error("Missing depth on -treedepth",(char *) NULL);
                  resource_info.tree_depth=atoi(argv[i]);
                }
              break;
            }
          Error("Unrecognized option",option);
          break;
        }
        case 'u':
        {
          if (strncmp("update",option+1,2) == 0)
            {
              resource_info.update=(*option == '-');
              if (*option == '-')
                {
                  i++;
                  if ((i == argc) || !sscanf(argv[i],"%d",&x))
                    Error("Missing seconds on -update",(char *) NULL);
                  resource_info.delay=atoi(argv[i]);
                }
              break;
            }
          if (strncmp("use_pixmap",option+1,2) == 0)
            {
              resource_info.use_pixmap=(*option == '-');
              break;
            }
          Error("Unrecognized option",option);
          break;
        }
        case 'v':
        {
          if (strncmp("verbose",option+1,2) == 0)
            {
              image_info.verbose=(*option == '-');
              break;
            }
          if (strncmp("visual",option+1,2) == 0)
            {
              resource_info.visual_type=(char *) NULL;
              if (*option == '-')
                {
                  i++;
                  if (i == argc)
                    Error("Missing visual class on -visual",(char *) NULL);
                  resource_info.visual_type=argv[i];
                }
              break;
            }
          Error("Unrecognized option",option);
          break;
        }
        case 'w':
        {
          if (strcmp("window",option+1) == 0)
            {
              resource_info.window_id=(char *) NULL;
              if (*option == '-')
                {
                  i++;
                  if (i == argc)
                    Error("Missing id, name, or 'root' on -window",
                      (char *) NULL);
                  resource_info.window_id=argv[i];
                }
              break;
            }
          if (strncmp("window_group",option+1,7) == 0)
            {
              resource_info.window_group=(char *) NULL;
              if (*option == '-')
                {
                  i++;
                  if (i == argc)
                    Error("Missing id, name, or 'root' on -window_group",
                      (char *) NULL);
                  resource_info.window_group=argv[i];
                }
              break;
            }
          if (strncmp("write",option+1,2) == 0)
            {
              resource_info.write_filename=(char *) NULL;
              if (*option == '-')
                {
                  i++;
                  if (i == argc)
                    Error("Missing file name on -write",(char *) NULL);
                  resource_info.write_filename=argv[i];
                  if (access(resource_info.write_filename,0) == 0)
                    {
                      char
                        answer[2];

                      (void) fprintf(stderr,"Overwrite %s? ",
                        resource_info.write_filename);
                      (void) fgets(answer,sizeof(answer)-1,stdin);
                      if (!((*answer == 'y') || (*answer == 'Y')))
                        exit(1);
                    }
                }
              break;
            }
          Error("Unrecognized option",option);
          break;
        }
        case '?':
        {
          Usage(True);
          break;
        }
        default:
        {
          Error("Unrecognized option",option);
          break;
        }
      }
    else
      for (scene=first_scene; scene <= last_scene ; scene++)
      {
        /*
          Option is a file name: begin by reading image from specified file.
        */
        (void) strcpy(image_info.filename,option);
        if (first_scene != last_scene)
          {
            char
              filename[MaxTextLength];

            /*
              Form filename for multi-part images.
            */
            (void) sprintf(filename,image_info.filename,scene);
            if (strcmp(filename,image_info.filename) == 0)
              (void) sprintf(filename,"%s.%u",image_info.filename,scene);
            (void) strcpy(image_info.filename,filename);
          }
        (void) strcpy(image_info.magick,"MIFF");
        image_info.server_name=resource_info.server_name;
        image_info.font=resource_info.font;
        image_info.dither=resource_info.dither;
        image_info.monochrome=resource_info.monochrome;
        resource_info.image_info=(&image_info);
        image=ReadImage(&image_info);
        if (image == (Image *) NULL)
          if ((i < (argc-1)) || (scene < last_scene))
            continue;
          else
            {
              state|=ExitState;
              break;
            }
        do
        {
          /*
            Transmogrify image as defined by the image processing options.
          */
          resource_info.quantum=1;
          MogrifyImage(&image_info,i,argv,&image);
          if (first_scene != last_scene)
            image->scene=scene;
          /*
            Display image to X server.
          */
          if (resource_info.window_id != (char *) NULL)
            {
              unsigned int
                status;

              /*
                Display image to a specified X window.
              */
              status=XDisplayBackgroundImage(display,&resource_info,image);
              if (status)
                state|=RetainColorsState;
              if (resource_info.delay == 0)
                state|=ExitState;
            }
          else


#endif                      /* End Kobus */


            do
            {
              Image
                *loaded_image;

              /*
                Display image to X server.
              */
              loaded_image=
                XDisplayImage(display,&resource_info,argv,argc,&image,&state);
              if (loaded_image == (Image *) NULL)
                break;
              while ((loaded_image != (Image *) NULL) && (!(state & ExitState)))
              {


#ifdef IGNORE_THIS_CODE     /* Kobus */
                if (loaded_image->montage != (char *) NULL)
                  {
                    /*
                      User selected a visual directory image (montage).
                    */
                    DestroyImages(image);
                    image=loaded_image;
                    break;
                  }
                MogrifyImage(&image_info,i,argv,&loaded_image);
                if (first_scene != last_scene)
                  image->scene=scene;
#endif                      /* End Kobus */


                next_image=XDisplayImage(display,&resource_info,argv,argc,
                  &loaded_image,&state);
                if (loaded_image != image)
                  DestroyImages(loaded_image);
                loaded_image=next_image;
              }
            } while (!(state & ExitState));


#ifdef IGNORE_THIS_CODE     /* Kobus */


          if (resource_info.write_filename != (char *) NULL)
            {
              /*
                Write image.
              */
              (void) strcpy(image->filename,resource_info.write_filename);
              (void) WriteImage(&image_info,image);
            }
          if (image_info.verbose)
            DescribeImage(image,stderr,False);
          /*
            Proceed to next/previous image.
          */
          next_image=image;
          if (state & FormerImageState)
            for (j=0; j < resource_info.quantum; j++)
            {
              next_image=next_image->previous;
              if (next_image == (Image *) NULL)
                break;
            }
          else
            for (j=0; j < resource_info.quantum; j++)
            {
              next_image=next_image->next;
              if (next_image == (Image *) NULL)
                break;
            }
          if (next_image != (Image *) NULL)
            image=next_image;
        } while ((next_image != (Image *) NULL) && !(state & ExitState));
        /*
          Free image resources.
        */
        DestroyImages(image);
        if (!(state & FormerImageState))
          image_marker[i]=image_number++;
        else
          {
            /*
              Proceed to previous image.
            */
            for (i--; i > 0; i--)
              if (image_marker[i] == (image_number-2))
                break;
            if (image_number != 0)
              image_number--;
          }
        if (state & ExitState)
          break;
      }
    /*
      Determine if we should proceed to the first image.
    */
    if (i == (argc-1))
      if (resource_info.confirm_exit && (state & NextImageState))
        {
          unsigned int
            status;

          /*
            Confirm program exit.
          */
          status=XConfirmWidget(display,windows,"Do you really want to exit",
            client_name);
          if (status == False)
            {
              i=0;
              image_number=0;
            }
        }
  }

#endif                      /* End Kobus */


/* Kobus */
  DestroyImage(image);
/* End Kobus */


  if (state & RetainColorsState)
    XRetainWindowColors(display,XRootWindow(display,XDefaultScreen(display)));
  free((char *) image_marker);
  free((char *) image_info.filename);
  XCloseDisplay(display);

#ifdef IGNORE_THIS_CODE     /* Kobus */
  exit(0);
  return(False);
#endif                      /* End Kobus */

/* Kobus */
  return NO_ERROR;
/* End Kobus */


}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* Kobus */
/*
 * Entirely new module to prepare for getting pure pixels.
 * Hacked from XMagnifyImage.
 */

static void update_pure_pixels
(
    Display*       display,
    XResourceInfo* resource_info,
    XWindows*      windows,
    XEvent*        event,
    Image**        image_ptr
)
{
    int x, y;
    unsigned long state;
    char image_geometry[MaxTextLength];


    TEST_PSO(("Updating pure pixels ... "));

    if ((windows->image.crop_geometry == (char *) NULL) &&
        ((*image_ptr)->columns == windows->image.ximage->width) &&
        ((*image_ptr)->rows == windows->image.ximage->height) &&
        (resource_info->number_colors == 0))
       {
        ;
       }
    else {

    /*
      Apply size transforms to image.
    */

    XSetCursorState(display,windows,True);
    XCheckRefreshWindows(display,windows);
    /*
      Crop and/or scale displayed image.
    */
    (void) sprintf(image_geometry,"%dx%d!",windows->image.ximage->width,
      windows->image.ximage->height);
    TransformImage(image_ptr,windows->image.crop_geometry,image_geometry);
     XSetCursorState(display,windows,False);

    if (windows->image.crop_geometry != (char *) NULL)
      {
        free((char *) windows->image.crop_geometry);
        windows->image.crop_geometry=(char *) NULL;
      }

    windows->image.x=0;
    windows->image.y=0;
    if (resource_info->number_colors != 0)
      {
        /*
          Reduce the number of colors in the image.
        */
        if (((*image_ptr)->class == DirectClass) ||
            ((*image_ptr)->colors > resource_info->number_colors) ||
            (resource_info->colorspace == GRAYColorspace))
          QuantizeImage(*image_ptr,resource_info->number_colors,
            resource_info->tree_depth,resource_info->dither,
            resource_info->colorspace);
        SyncImage(*image_ptr);
      }

    XConfigureImageColormap(display,resource_info,windows,*image_ptr);
    (void) XConfigureImage(display,resource_info,windows,*image_ptr);
    }

    set_pure_pixels(*image_ptr);
    TEST_PSO(("done.\n"));
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* Kobus */
/*
 * Entirely new module to handle new behaviour for third button.
 * Hacked from XMagnifyImage.
 */

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   X T r a c e I m a g e                                                     %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%
*/
static void XTraceImage
(
    Display*       display,
    XResourceInfo* resource_info,
    XWindows*      windows,
    XEvent*        event,
    Image**        image_ptr
)
{
    extern int fs_horizontal_trace;
    extern int fs_vertical_trace;
    extern int fs_patch_output;
    extern int fs_point_output;
    IMPORT unsigned char* pure_pixels;
    char text[MaxTextLength];
    int x, y;
    unsigned long state;


    if (pure_pixels == (unsigned char *)NULL) {
         XBell(display,0);
         set_bug("Pure pixels are not available.");
         return;
        }

    if (fs_patch_output) {
         XPatch(display,resource_info,windows,event,*image_ptr, pure_pixels);
         return;
        }

    if (fs_point_output) {
        pso("%d %d\n", event->xbutton.x, event->xbutton.y);
        return;
    }

    XDefineCursor(display,windows->image.id,windows->magnify.cursor);

    state=DefaultState;

    /*
    x = windows->image.x + event->xbutton.x;
    y = windows->image.y + event->xbutton.y;
    */
    x = event->xbutton.x;
    y = event->xbutton.y;

    do
    {
      /*
        Map and unmap Info widget as text cursor crosses its boundaries.
      */

      if (windows->info.mapped)
        {
          if ((x < (windows->info.x+windows->info.width)) &&
              (y < (windows->info.y+windows->info.height)))
            XWithdrawWindow(display,windows->info.id,windows->info.screen);
        }
      else {
        if ((x > (windows->info.x+windows->info.width)) ||
            (y > (windows->info.y+windows->info.height))) {
          XMapWindow(display,windows->info.id);
         }
       }

      if (windows->info.mapped)
        {
         int new_x, new_y;
         double r, g, b, s;
         unsigned char *pos;
          /*
            Display pointer position.
          */

          new_x = x + windows->image.x;
          new_y = y + windows->image.y;

          pos = pure_pixels + 3 * ((new_y * (*image_ptr)->columns) + new_x);

          r = *pos;
          g = *(pos+1);
          b = *(pos+2);
          s = r + g + b;

          sprintf(text,"%4d %4d (%4d %4d %4d)    [  %.3f   %.3f  ]",
                  new_x, new_y, *pos, *(pos+1), *(pos+2), r/s, g/s);

          XInfoWidget(display,windows,text);
        }

      /*
        Wait for next event.
      */

      /* Note: IM includes a fair bit of logic in the screen routines (Kobus) */
      XIfEvent(display,event,XScreenEvent2,(char *) windows);

      switch (event->type)
      {
        case ButtonRelease:
        {
          x = windows->image.x + event->xbutton.x;
          y = windows->image.y + event->xbutton.y;
          state|=ExitState;
          break;
        }
        case MotionNotify:
        {
          /*
            Discard pending button motion events.
          */
          while (XCheckMaskEvent(display,ButtonMotionMask,event));

          /*
          // x = windows->image.x + event->xbutton.x;
          // y = windows->image.y + event->xbutton.y;
          */
          x = event->xbutton.x;
          y = event->xbutton.y;
          break;
        }
        default:
          break;
      }
      /*
        Check boundary conditions.
      */
      if (x < 0)
        x=0;
      else
        if (x >= windows->image.width)
          x=windows->image.width-1;
      if (y < 0)
        y=0;
      else
       if (y >= windows->image.height)
         y=windows->image.height-1;

    } while (!(state & ExitState));

    XSetCursorState(display,windows,False);
    XWithdrawWindow(display,windows->info.id,windows->info.screen);

#ifdef HOW_IT_WAS
    if (! isatty(fileno(stdout)))
#else
    if (fp_get_path_type(stdout) == PATH_IS_PIPE)
#endif
    {
        /*
        // Untested since switch to test on PIPE.
        // I have forgotten what, if anything, we still use
        // this for, but I vaguely recall that it was
        // explicitly when we were feeding this into another
        // program.
        */
        UNTESTED_CODE();

        sprintf(text,"%d %d\n", x,y);
        write(fileno(stdout), text, strlen(text));
        fflush(stdout);
    }

    if (fs_horizontal_trace)
    {
        EPE(im_do_horizontal_trace((*image_ptr)->rows, (*image_ptr)->columns,
                                   pure_pixels, x, y));
    }

    if (fs_vertical_trace)
    {
        EPE(im_do_vertical_trace((*image_ptr)->rows, (*image_ptr)->columns,
                                 pure_pixels, x, y));
    }
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   X P a t c h                                                               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
*/

static unsigned int XPatch
(
    Display*       display,
    XResourceInfo* resource_info,
    XWindows*      windows,
    XEvent*        event_ptr,
    Image*         image,
    unsigned char* pixels
)
{

  char
    command[MaxTextLength],
    text[MaxTextLength];

  int x, y;

  KeySym
    key_symbol;

  RectangleInfo
    crop_info,
    highlight_info;

  unsigned long
    state;

  XEvent
    event;

  crop_info.x=windows->image.x+event_ptr->xbutton.x;
  crop_info.y=windows->image.y+event_ptr->xbutton.y;


  XSetFunction(display,windows->image.highlight_context,GXinvert);

  do
  {
    /*
      Size rectangle as pointer moves until the mouse button is released.
    */
    x=crop_info.x;
    y=crop_info.y;
    crop_info.width=0;
    crop_info.height=0;
    state=DefaultState;

    XSelectInput(display,windows->image.id,
                 windows->image.attributes.event_mask | PointerMotionMask);

    do
    {
      highlight_info=crop_info;
      highlight_info.x=crop_info.x-windows->image.x;
      highlight_info.y=crop_info.y-windows->image.y;


      XHighlightRectangle(display,windows->image.id,
                          windows->image.highlight_context,&highlight_info);

      XIfEvent(display,&event,XScreenEvent2,(char *) windows);

      /* Erase last highlight. (Comment added by Kobus) */
      XHighlightRectangle(display,windows->image.id,
                          windows->image.highlight_context,&highlight_info);

      switch (event.type)
      {
        case ButtonRelease:
        {
          /*
            User has committed to cropping rectangle.
          */
          crop_info.x=windows->image.x+event.xbutton.x;
          crop_info.y=windows->image.y+event.xbutton.y;
          state|=ExitState;

          break;
        }
        case Expose:
          break;
        case MotionNotify:
        {
          /*
            Discard pending button motion events.
          */
          while (XCheckMaskEvent(display,ButtonMotionMask,&event));
          crop_info.x=windows->image.x+event.xmotion.x;
          crop_info.y=windows->image.y+event.xmotion.y;

          break;
        }
        default:
          break;
      }
      if (((crop_info.x != x) && (crop_info.y != y)) || (state & ExitState))
        {
          /*
            Check boundary conditions.
          */
          if (crop_info.x < 0)
            crop_info.x=0;
          else
            if (crop_info.x > windows->image.ximage->width)
              crop_info.x=windows->image.ximage->width;
          if (crop_info.x < x)
            crop_info.width=(unsigned int) (x-crop_info.x);
          else
            {
              crop_info.width=(unsigned int) (crop_info.x-x);
              crop_info.x=x;
            }
          if (crop_info.y < 0)
            crop_info.y=0;
          else
            if (crop_info.y > windows->image.ximage->height)
              crop_info.y=windows->image.ximage->height;
          if (crop_info.y < y)
            crop_info.height=(unsigned int) (y-crop_info.y);
          else
            {
              crop_info.height=(unsigned int) (crop_info.y-y);
              crop_info.y=y;
            }
        }
    } while (!(state & ExitState));

    XInfoWidget(display,windows, "Return to accept, ESC to reject");

    XSelectInput(display,windows->image.id,
                 windows->image.attributes.event_mask);

    state=DefaultState;

    do
    {
      highlight_info=crop_info;
      highlight_info.x=crop_info.x-windows->image.x;
      highlight_info.y=crop_info.y-windows->image.y;

      XHighlightRectangle(display,windows->image.id,
                          windows->image.highlight_context,&highlight_info);

      XIfEvent(display,&event,XScreenEvent2,(char *) windows);

      /* Erase it. (Comment added by Kobus) */
      XHighlightRectangle(display,windows->image.id,
                          windows->image.highlight_context,&highlight_info);

      switch (event.type)
      {
        case Expose:
        {
          if (event.xexpose.window == windows->image.id)
            if (event.xexpose.count == 0)
              {
                event.xexpose.x=highlight_info.x;
                event.xexpose.y=highlight_info.y;
                event.xexpose.width=highlight_info.width;
                event.xexpose.height=highlight_info.height;
                XRefreshWindow(display,&windows->image,&event);
              }
          if (event.xexpose.window == windows->info.id)
            if (event.xexpose.count == 0)
              XInfoWidget(display,windows,text);
          break;
        }
        case KeyPress:
        {
          if (event.xkey.window != windows->image.id)
            break;
          /*
            Respond to a user key press.
          */
          (void) XLookupString((XKeyEvent *) &event.xkey,command,
            sizeof(command),&key_symbol,(XComposeStatus *) NULL);
          switch (key_symbol)
          {
            case XK_Escape:
              state|=ExitState;
              break;
            case XK_Return:
              EPE(im_do_patch_output(image->rows,
                                     image->columns,
                                     pixels, crop_info.x, crop_info.y,
                                     crop_info.x + crop_info.width - 1,
                                     crop_info.y + crop_info.height - 1));
              state|=ExitState;
              break;
            default:
              XBell(display,0);
              break;
          }
          break;
        }
        default:
          break;
      }


    } while (!(state & ExitState));
  } while (!(state & ExitState));


  XSetFunction(display,windows->image.highlight_context,GXcopy);

  XWithdrawWindow(display,windows->info.id,windows->info.screen);

  return(True);
}

/* ========================================================================= */

/* ========================================================================= */

/* Kobus: Hack XScreenEvent to get XScreenEvent2 used in XTraceImage.   */
/*        This is necessary because IM puts logic into XScreenEvent!    */
/*        Some cases that can't happen are handled (since it is just    */
/*        a quick hack.)                                                */

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   X S c r e e n E v e n t 2                                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
*/
static int XScreenEvent2(Display* display, XEvent* event, char* data)
{
  XWindows *windows;


  windows=(XWindows *) data;
  if (event->xany.window == windows->command.id)
    return(True);
  switch (event->type)
  {
    case ButtonPress:     /* Fall through */
    case ButtonRelease:
    {
      if ((event->xbutton.button == Button3) &&
          (event->xbutton.state & Mod1Mask))
        {
          /*
            Convert Alt-Button3 to Button2.
          */
          event->xbutton.button=Button2;
          event->xbutton.state&=(~Mod1Mask);
        }
#ifdef TRY_WITHOUT
      if (event->xbutton.window == windows->backdrop.id)
        XSetInputFocus(display,event->xbutton.window,RevertToParent,
          event->xbutton.time);
#endif
      return(True);
    }
#ifdef TRY_WITHOUT
    case ClientMessage:
    {
      /*
        If client window delete message, exit.
      */
      if (event->xclient.message_type != windows->wm_protocols)
        break;
      if (*event->xclient.data.l != windows->wm_delete_window)
        break;
      if (event->xclient.window == windows->magnify.id)
        {
          XWithdrawWindow(display,windows->magnify.id,windows->magnify.screen);
          return(True);
        }
      break;
    }
#endif
    case Expose:
    {
      if (event->xexpose.window == windows->image.id)
        {
          XRefreshWindow(display,&windows->image,event);
          return(True);
        }
      if (event->xexpose.window == windows->pan.id)
        if (event->xexpose.count == 0)
          {
            XDrawPanRectangle(display,windows);
            return(True);
          }
      break;
    }
    case KeyPress:
    {
     return True;     /* Needed for patch; want to ignore otherwise. */
    }
    case MapNotify:
    {
#ifdef TRY_WITHOUT
      if (event->xmap.window == windows->magnify.id)
        {
          XMakeMagnifyImage(display,windows);
          windows->magnify.mapped=True;
          return(True);
        }
#endif
      if (event->xmap.window == windows->info.id)
        {
          windows->info.mapped=True;
          return(True);
        }
      break;
    }
    case ConfigureNotify:
    {
      if (event->xmap.window == windows->info.id) {
          return(True);
         }
      else {
          return False;
         }
    }
    case MotionNotify:
    {
      return(True);
    }
    case UnmapNotify:
    {
#ifdef TRY_WITHOUT
      if (event->xunmap.window == windows->magnify.id)
        {
          windows->magnify.mapped=False;
          return(True);
        }
#endif
      if (event->xunmap.window == windows->info.id)
        {
          windows->info.mapped=False;
          return(True);
        }
      break;
    }
    case KeyRelease:
    case SelectionNotify:
      return(True);
    default:
      break;
  }
  return(False);
}

/* ========================================================================= */

/* Kobus: Routine to set behaviour of button 3. */

static void process_button_three_directive(KeySym key_symbol)
{
    extern int fs_horizontal_trace;
    extern int fs_vertical_trace;
    extern int fs_use_rg_chromaticity;
    extern int fs_patch_output;


    /* dbx(key_symbol); */

    switch (key_symbol)
    {
        case 0x0a5:    /* HACK for when server is HP (no longer relavent?) */
        case 0x1ff:    /* HACK for when server is Mac */
        case XK_h:     /* Fall through */
        case XK_H:
            fs_horizontal_trace = !fs_horizontal_trace;

            if (fs_horizontal_trace) 
            {
                XInfoWidget(display,windows, "Horizontal trace is now enabled.");
            }
            else
            {
                XInfoWidget(display,windows, "Horizontal trace is now disabled.");
            }
            break;

        case 0x0a7:    /* HACK for when server is HP (no longer relavent?) */
        case 0x8d6:    /* HACK for when server is Mac */
        case XK_v:     /* Fall through */
        case XK_V:
            fs_vertical_trace = !fs_vertical_trace;

            if (fs_vertical_trace) 
            {
                XInfoWidget(display,windows, "Vertical trace is now enabled.");
            }
            else
            {
                XInfoWidget(display,windows, "Vertical trace is now disabled.");
            }
            break;

        case 0x0fe:    /* HACK for when server is HP (no longer relavent?) */
        case 0x7f0:    /* HACK for when server is Mac */
        case XK_p:     /* Fall through */
        case XK_P:
            fs_patch_output = !fs_patch_output;

            if (fs_patch_output) 
            {
                XInfoWidget(display,windows, "Patch output is now enabled.");
            }
            else
            {
                XInfoWidget(display,windows, "Patch output is now disabled.");
            }
            break;

        case 0x0e7:    /* HACK for when server is HP (no longer relavent?) */
        case XK_c:     /* Fall through */
        case XK_C:
            fs_use_rg_chromaticity = !fs_use_rg_chromaticity;
            break;

        case 0x0e8:    /* HACK for when server is HP (no longer relavent?) */
        case 0x8ef:    /* HACK for when server is Mac */
        case XK_d:     /* Fall through */
        case XK_D:
            fs_point_output = !fs_point_output;
            break;

        default:
            XBell(display,0);
            break;
    }
}


/* ========================================================================= */

/* Kobus: Routine to do horizontal tracing. */

static int im_do_horizontal_trace
(
    int            num_rows,
    int            num_cols,
    unsigned char* pixel_array,
    int            x,
    int            y
)
{
    extern int fs_use_rg_chromaticity;
    static int plot_id = 0;
    unsigned char *pixel_pos;
    char title[ 200 ];
    Matrix *data_mp=NULL;
    int i;


    if (plot_id == 0)
    {
        ERE(plot_id = plot_open());
    }
    else
    {
        ERE(plot_clear(plot_id));
    }

    sprintf(title, "Horizontal trace at y=%d", y);
    ERE(plot_set_title(plot_id, title, 0, 0));

    ERE(plot_set_range(plot_id,
                       PLOT_USE_DEFAULT_RANGE, PLOT_USE_DEFAULT_RANGE,
                       PLOT_USE_DEFAULT_RANGE, PLOT_USE_DEFAULT_RANGE));

    NRE(data_mp = create_matrix(num_cols, 3));

    pixel_pos = pixel_array + 3*num_cols*y;

    for (i=0; i<num_cols; i++)
    {
        (data_mp->elements)[i][0] = *pixel_pos;
        pixel_pos++;
        (data_mp->elements)[i][1] = *pixel_pos;
        pixel_pos++;
        (data_mp->elements)[i][2] = *pixel_pos;
        pixel_pos++;
    }

    if (fs_use_rg_chromaticity)
    {
        const char* rg_names[3] = { "r=R/(R+G+B)", "g=G/(R+G+B)", "(R+G+B)/1200" };
        Matrix* rg_data_mp         = NULL;
        Matrix* rg_and_sum_data_mp = NULL;

        ERE(get_divide_by_sum_projection_matrix(&rg_data_mp, data_mp,
                                                ZERO_INVALID_CHROMATICITY,
                                                DBL_NOT_SET, DBL_NOT_SET,
                                                DBL_NOT_SET, (Matrix**)NULL));

        ERE(get_target_matrix(&rg_and_sum_data_mp, num_cols, 3));
        ERE(ow_copy_matrix(rg_and_sum_data_mp, 0, 0, rg_data_mp));

        for (i = 0; i < num_cols; i++)
        {
            rg_and_sum_data_mp->elements[ i ][ 2 ] =
                                    (data_mp->elements[ i ][ 0 ] +
                                     data_mp->elements[ i ][ 1 ] +
                                     data_mp->elements[ i ][ 2 ]   ) /  1200.0;
        }

        ERE(plot_matrix_cols(plot_id, rg_and_sum_data_mp, 0.0, 1.0, rg_names,
                             (const int*) NULL));

        free_matrix(rg_data_mp);
        free_matrix(rg_and_sum_data_mp);
    }
    else
    {
        const char *names[3] = { "red", "green", "blue" };

        ERE(plot_matrix_cols(plot_id, data_mp, 0.0, 1.0, names,
                             (const int*) NULL));
    }

    free_matrix(data_mp);

    return NO_ERROR;
}


/* ========================================================================= */

/* Kobus: Routine to do vertical tracing. */

static int im_do_vertical_trace
(
    int            num_rows,
    int            num_cols,
    unsigned char* pixel_array,
    int            x,
    int            y
)
{
    extern int fs_use_rg_chromaticity;
    static int plot_id = 0;
    unsigned char *pixel_pos;
    char title[ 200 ];
    Matrix *data_mp=NULL;
    int i;


    if (plot_id == 0)
    {
        ERE(plot_id = plot_open());
    }
    else
    {
        ERE(plot_clear(plot_id));
    }

    sprintf(title, "Vertical trace at x=%d", x);
    ERE(plot_set_title(plot_id, title, 0, 0));

    ERE(plot_set_range(plot_id,
                       PLOT_USE_DEFAULT_RANGE, PLOT_USE_DEFAULT_RANGE,
                       PLOT_USE_DEFAULT_RANGE, PLOT_USE_DEFAULT_RANGE));

    NRE(data_mp = create_matrix(num_rows, 3));

    for (i=0; i<num_rows; i++)
    {
        pixel_pos = pixel_array + 3*num_cols*i + 3*x;

        (data_mp->elements)[i][0] = *pixel_pos;
        pixel_pos++;
        (data_mp->elements)[i][1] = *pixel_pos;
        pixel_pos++;
        (data_mp->elements)[i][2] = *pixel_pos;
        pixel_pos++;
    }

    if (fs_use_rg_chromaticity)
    {
        const char*   rg_names[3] = { "r=R/(R+G+B)", "g=G/(R+G+B)", "(R+G+B)/1200" };
        Matrix* rg_data_mp         = NULL;
        Matrix* rg_and_sum_data_mp = NULL;

        ERE(get_divide_by_sum_projection_matrix(&rg_data_mp, data_mp,
                                                ZERO_INVALID_CHROMATICITY,
                                                DBL_NOT_SET, DBL_NOT_SET,
                                                DBL_NOT_SET, (Matrix**)NULL));

        ERE(get_target_matrix(&rg_and_sum_data_mp, num_rows, 3));
        ERE(ow_copy_matrix(rg_and_sum_data_mp, 0, 0, rg_data_mp));

        for (i = 0; i < num_rows; i++)
        {
            rg_and_sum_data_mp->elements[ i ][ 2 ] =
                                    (data_mp->elements[ i ][ 0 ] +
                                     data_mp->elements[ i ][ 1 ] +
                                     data_mp->elements[ i ][ 2 ]   ) /  1200.0;
        }

        ERE(plot_matrix_cols(plot_id, rg_and_sum_data_mp, 0.0, 1.0, rg_names,
                             (const int*) NULL));

        free_matrix(rg_data_mp);
        free_matrix(rg_and_sum_data_mp);
    }
    else
    {
        const char *names[3] = { "red", "green", "blue" };

        ERE(plot_matrix_cols(plot_id, data_mp, 0.0, 1.0, names,
                             (const int*) NULL));
    }

    free_matrix(data_mp);

    return NO_ERROR;
}


/* ========================================================================= */

/* Kobus: Routine to do patch output. */

static int im_do_patch_output
(
    int            num_rows,
    int            num_cols,
    unsigned char* pixels,
    int            x1,
    int            y1,
    int            x2,
    int            y2
)
{
    double r, b, g;
    double r_sum, b_sum, g_sum;
    double r_sum_sqr, b_sum_sqr, g_sum_sqr;
    double r_ave, b_ave, g_ave;
    double r_stdev, b_stdev, g_stdev;
    int i, j;
    long num_pixels = 0;
    int xmin, xmax, ymin, ymax;
    unsigned char *pixel_pos;
    double temp;


    xmin = MIN_OF(x1, x2);
    xmax = MAX_OF(x1, x2);
    ymin = MIN_OF(y1, y2);
    ymax = MAX_OF(y1, y2);

    dbi(xmin);
    dbi(xmax);
    dbi(ymin);
    dbi(ymax);

    r_sum = 0.0;
    g_sum = 0.0;
    b_sum = 0.0;

    r_sum_sqr = 0.0;
    g_sum_sqr = 0.0;
    b_sum_sqr = 0.0;

    if (fs_use_rg_chromaticity)
    {
        double sum;

        for (j=ymin; j<=ymax; j++)
        {
            pixel_pos = pixels + 3*(num_cols*j + xmin);

            for (i=xmin; i<=xmax; i++)
            {
                r = *pixel_pos++;
                g = *pixel_pos++;
                b = *pixel_pos++;

                sum = r + g + b;

                if (sum > 0.0)
                {
                    r /= sum;
                    g /= sum;

                    r_sum += r;
                    g_sum += g;

                    r_sum_sqr += r*r;
                    g_sum_sqr += g*g;

                    num_pixels++;
                }
            }
        }
    }
    else
    {
        for (j=ymin; j<=ymax; j++)
        {
            pixel_pos = pixels + 3*(num_cols*j + xmin);

            for (i=xmin; i<=xmax; i++)
            {
                r = *pixel_pos++;
                g = *pixel_pos++;
                b = *pixel_pos++;

                r_sum += r;
                g_sum += g;
                b_sum += b;

                r_sum_sqr += r*r;
                g_sum_sqr += g*g;
                b_sum_sqr += b*b;

                num_pixels++;
            }
        }
    }

    r_ave = r_sum /((double)num_pixels);
    g_ave = g_sum/((double)num_pixels);

    temp = r_sum_sqr / ((double)num_pixels);
    temp -= r_ave*r_ave;
    r_stdev = sqrt(temp);

    temp = g_sum_sqr / ((double)num_pixels);
    temp -= g_ave*g_ave;
    g_stdev = sqrt(temp);

    if (fs_use_rg_chromaticity)
    {
        pso("%5.3f %5.3f  %5.3f %5.3f  %d %d %d %d\n", r_ave, g_ave,
            r_stdev, g_stdev, xmin, ymin, xmax - xmin + 1, ymax - ymin +1);
    }
    else
    {
        b_ave = b_sum/((double)num_pixels);
        temp = b_sum_sqr / ((double)num_pixels);
        temp -= b_ave*b_ave;
        b_stdev = sqrt(temp);

        pso("%5.1f %5.1f %5.1f  %5.1f %5.1f %5.1f  %d %d %d %d\n",
            r_ave, g_ave, b_ave, r_stdev, g_stdev, b_stdev,
            xmin, ymin, xmax - xmin + 1, ymax - ymin +1);
    }
    kjb_flush();

    return NO_ERROR;
}

#endif  /* #ifdef KJB_HAVE_X11 */

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* ========================================================================= */

/* Kobus: Copy of ImageMagick display main so that we can easily implement
 *        enhanced version of ImageMagick display
 */

#ifdef KJB_HAVE_X11

int im_display_main(int argc, char** argv)
{
#define NotInitialized  (unsigned int) (~0)

  char
    density[MaxTextLength],
    *option,
    *resource_value,
    *server_name;

  Image
    *image,
    *next_image;

  ImageInfo
    image_info;

  int
    x;

  register int
    i,
    j;

  unsigned int
    first_scene,
    *image_marker,
    image_number,
    last_scene,
    scene,
    x_density,
    y_density;

  unsigned long
    state;

  XResourceInfo
    resource_info;

  XrmDatabase
    resource_database;

  /*
    Set defaults.
  */
  client_name=ClientName(*argv);
  display=(Display *) NULL;
  first_scene=0;
  image_marker=(unsigned int *) malloc((argc+1)*sizeof(unsigned int));
  if (image_marker == (unsigned int *) NULL)
    Error("Unable to display image","Memory allocation failed");
  for (i=0; i <= argc; i++)
    image_marker[i]=argc;
  image_number=0;
  GetImageInfo(&image_info);
  last_scene=0;
  resource_database=(XrmDatabase) NULL;
  server_name=(char *) NULL;
  state=DefaultState;
  /*
    Check for server name specified on the command line.
  */
  ExpandFilenames(&argc,&argv);
  for (i=1; i < argc; i++)
  {
    /*
      Check command line for server name.
    */
    option=argv[i];
    if (((int) strlen(option) == 1) || ((*option != '-') && (*option != '+')))
      continue;
    if (strncmp("display",option+1,3) == 0)
      {
        /*
          User specified server name.
        */
        i++;
        if (i == argc)
          Error("Missing server name on -display",(char *) NULL);
        server_name=argv[i];
        break;
      }
    if (strncmp("help",option+1,2) == 0)
      Usage(True);
  }
  /*
    Open X server connection.
  */
  display=XOpenDisplay(server_name);
  if (display == (Display *) NULL)
    Error("Unable to connect to X server",XDisplayName(server_name));
  /*
    Set our forgiving error handler.
  */
  XSetErrorHandler(XError);
  /*
    Get user defaults from X resource database.
  */
  resource_database=XGetResourceDatabase(display,client_name);
  XGetResourceInfo(resource_database,client_name,&resource_info);
  x_density=(unsigned int)(((((double) XDisplayWidth(display,XDefaultScreen(display)))*25.4)/
    ((double) XDisplayWidthMM(display,XDefaultScreen(display))))+0.5);
  y_density=(unsigned int)(((((double) XDisplayHeight(display,XDefaultScreen(display)))*25.4)/
    ((double) XDisplayHeightMM(display,XDefaultScreen(display))))+0.5);
  (void) sprintf(density,"%ux%u",x_density,y_density);
  image_info.density=
    XGetResourceClass(resource_database,client_name,"density",density);
  resource_value=
    XGetResourceClass(resource_database,client_name,"interlace","none");
  image_info.interlace=UndefinedInterlace;
  if (Latin1Compare("none",resource_value) == 0)
    image_info.interlace=NoneInterlace;
  if (Latin1Compare("line",resource_value) == 0)
    image_info.interlace=LineInterlace;
  if (Latin1Compare("plane",resource_value) == 0)
    image_info.interlace=PlaneInterlace;
  if (image_info.interlace == UndefinedInterlace)
    Warning("Unrecognized interlace type",resource_value);
  image_info.page=XGetResourceClass(resource_database,client_name,
    "pageGeometry",(char *) NULL);
  resource_value=
    XGetResourceClass(resource_database,client_name,"quality","75");
  image_info.quality=atoi(resource_value);
  resource_value=
    XGetResourceClass(resource_database,client_name,"verbose","False");
  image_info.verbose=IsTrue(resource_value);
  /*
    Parse command line.
  */
  for (i=1; ((i <= argc) && !(state & ExitState)); i++)
  {
    if (i < argc)
      option=argv[i];
    else
      if (image_number != 0)
        break;
      else
        if (isatty(STDIN_FILENO))
          option="logo:";
        else
          option="-";
    if (((int) strlen(option) > 1) && ((*option == '-') || (*option == '+')))
      switch (*(option+1))
      {
        case 'b':
        {
          if (strncmp("backdrop",option+1,5) == 0)
            {
              resource_info.backdrop=(*option == '-');
              break;
            }
          if ((strncmp("background",option+1,5) == 0) ||
              (strncmp("bg",option+1,2) == 0))
            {
              resource_info.background_color=(char *) NULL;
              if (*option == '-')
                {
                  i++;
                  if (i == argc)
                    Error("Missing color on -background",(char *) NULL);
                  resource_info.background_color=argv[i];
                }
              break;
            }
          if (strcmp("border",option+1) == 0)
            {
              if (*option == '-')
                {
                  i++;
                  if ((i == argc) || !sscanf(argv[i],"%d",&x))
                    Error("Missing geometry on -border",(char *) NULL);
                }
              break;
            }
          if (strncmp("bordercolor",option+1,7) == 0)
            {
              resource_info.border_color=(char *) NULL;
              if (*option == '-')
                {
                  i++;
                  if (i == argc)
                    Error("Missing color on -bordercolor",(char *) NULL);
                  resource_info.border_color=argv[i];
                }
              break;
            }
          if (strncmp("borderwidth",option+1,7) == 0)
            {
              resource_info.border_width=0;
              if (*option == '-')
                {
                  i++;
                  if ((i == argc) || !sscanf(argv[i],"%d",&x))
                    Error("Missing width on -borderwidth",(char *) NULL);
                  resource_info.border_width=atoi(argv[i]);
                }
              break;
            }
            if (strncmp("box",option+1,3) == 0)
              {
                if (*option == '-')
                  {
                    i++;
                    if (i == argc)
                      Error("Missing color on -box",(char *) NULL);
                  }
                break;
              }
          Error("Unrecognized option",option);
          break;
        }
        case 'c':
        {
          if (strncmp("colormap",option+1,6) == 0)
            {
              resource_info.colormap=PrivateColormap;
              if (*option == '-')
                {
                  i++;
                  if (i == argc)
                    Error("Missing type on -colormap",(char *) NULL);
                  option=argv[i];
                  resource_info.colormap=UndefinedColormap;
                  if (Latin1Compare("private",option) == 0)
                    resource_info.colormap=PrivateColormap;
                  if (Latin1Compare("shared",option) == 0)
                    resource_info.colormap=SharedColormap;
                  if (resource_info.colormap == UndefinedColormap)
                    Error("Invalid colormap type on -colormap",option);
                }
              break;
            }
          if (strncmp("colors",option+1,7) == 0)
            {
              resource_info.number_colors=0;
              if (*option == '-')
                {
                  i++;
                  if ((i == argc) || !sscanf(argv[i],"%d",&x))
                    Error("Missing colors on -colors",(char *) NULL);
                  resource_info.number_colors=atoi(argv[i]);
                }
              break;
            }
          if (strncmp("colorspace",option+1,7) == 0)
            {
              resource_info.colorspace=RGBColorspace;
              if (*option == '-')
                {
                  i++;
                  if (i == argc)
                    Error("Missing type on -colorspace",(char *) NULL);
                  option=argv[i];
                  resource_info.colorspace=UndefinedColorspace;
                  if (Latin1Compare("gray",option) == 0)
                    {
                      resource_info.colorspace=GRAYColorspace;
                      resource_info.number_colors=256;
                      resource_info.tree_depth=8;
                    }
                  if (Latin1Compare("ohta",option) == 0)
                    resource_info.colorspace=OHTAColorspace;
                  if (Latin1Compare("rgb",option) == 0)
                    resource_info.colorspace=RGBColorspace;
                  if (Latin1Compare("xyz",option) == 0)
                    resource_info.colorspace=XYZColorspace;
                  if (Latin1Compare("ycbcr",option) == 0)
                    resource_info.colorspace=YCbCrColorspace;
                  if (Latin1Compare("yiq",option) == 0)
                    resource_info.colorspace=YIQColorspace;
                  if (Latin1Compare("ypbpr",option) == 0)
                    resource_info.colorspace=YPbPrColorspace;
                  if (Latin1Compare("yuv",option) == 0)
                    resource_info.colorspace=YUVColorspace;
                  if (resource_info.colorspace == UndefinedColorspace)
                    Error("Invalid colorspace type on -colorspace",option);
                }
              break;
            }
          if (strncmp("comment",option+1,4) == 0)
            {
              if (*option == '-')
                {
                  i++;
                  if (i == argc)
                    Error("Missing comment on -comment",(char *) NULL);
                }
              break;
            }
          if (strncmp("compress",option+1,3) == 0)
            {
              if (*option == '-')
                {
                  i++;
                  if (i == argc)
                    Error("Missing type on -compress",(char *) NULL);
                  option=argv[i];
                  if (Latin1Compare("runlengthencoded",option) == 0)
                    break;
                  else
                    if (Latin1Compare("zlib",option) == 0)
                      break;
                    else
                      Error("Invalid compression type on -compress",option);
                }
              break;
            }
          if (strncmp("contrast",option+1,3) == 0)
            break;
          if (strncmp("crop",option+1,2) == 0)
            {
              if (*option == '-')
                {
                  i++;
                  if ((i == argc) || !sscanf(argv[i],"%d",&x))
                    Error("Missing geometry on -crop",(char *) NULL);
                }
              break;
            }
          Error("Unrecognized option",option);
          break;
        }
        case 'd':
        {
          if (strncmp("debug",option+1,3) == 0)
            {
              resource_info.debug=(*option == '-');
              break;
            }
          if (strncmp("delay",option+1,3) == 0)
            {
              resource_info.delay=0;
              if (*option == '-')
                {
                  i++;
                  if ((i == argc) || !sscanf(argv[i],"%d",&x))
                    Error("Missing seconds on -delay",(char *) NULL);
                  resource_info.delay=atoi(argv[i]);
                }
              break;
            }
          if (strncmp("density",option+1,3) == 0)
            {
              image_info.density=(char *) NULL;
              if (*option == '-')
                {
                  i++;
                  if ((i == argc) || !sscanf(argv[i],"%d",&x))
                    Error("Missing geometry on -density",(char *) NULL);
                  image_info.density=argv[i];
                }
              break;
            }
          if (strncmp("despeckle",option+1,3) == 0)
            break;
          if (strncmp("display",option+1,3) == 0)
            {
              server_name=(char *) NULL;
              if (*option == '-')
                {
                  i++;
                  if (i == argc)
                    Error("Missing server name on -display",(char *) NULL);
                  server_name=argv[i];
                }
              resource_info.server_name=server_name;
              break;
            }
          if (strncmp("dither",option+1,3) == 0)
            {
              resource_info.dither=(*option == '-');
              break;
            }
          /* Prasad */
          if (strcmp("datamode",option+1) == 0)
          {
              fs_point_output = TRUE;
              break;
          }
          /* End Prasad */
          Error("Unrecognized option",option);
          break;
        }
        case 'e':
        {
          if (strncmp("edge",option+1,2) == 0)
            {
              if (*option == '-')
                {
                  i++;
                  if ((i == argc) || !sscanf(argv[i],"%f",(float *) &x))
                    Error("Missing factor on -edge",(char *) NULL);
                }
              break;
            }
          if (strncmp("enhance",option+1,2) == 0)
            break;
          if (strncmp("equalize",option+1,2) == 0)
            break;
          Error("Unrecognized option",option);
          break;
        }
        case 'f':
        {
          if (strncmp("flip",option+1,3) == 0)
            break;
          if (strncmp("flop",option+1,3) == 0)
            break;
          if (strncmp("font",option+1,3) == 0)
            {
              resource_info.font=(char *) NULL;
              if (*option == '-')
                {
                  i++;
                  if (i == argc)
                    Error("Missing font name on -font",(char *) NULL);
                  resource_info.font=argv[i];
                }
              break;
            }
          if ((strncmp("foreground",option+1,3) == 0) ||
              (strncmp("fg",option+1,2) == 0))
           {
             resource_info.foreground_color=(char *) NULL;
             if (*option == '-')
               {
                 i++;
                 if (i == argc)
                   Error("Missing foreground on -foreground",(char *) NULL);
                 resource_info.foreground_color=argv[i];
               }
              break;
           }
          if (strncmp("frame",option+1,2) == 0)
            {
              if (*option == '-')
                {
                  i++;
                  if ((i == argc) || !sscanf(argv[i],"%d",&x))
                    Error("Missing geometry on -frame",(char *) NULL);
                }
              break;
            }
          Error("Unrecognized option",option);
          break;
        }
        case 'g':
        {
          if (strncmp("gamma",option+1,2) == 0)
            {
              if (*option == '-')
                {
                  i++;
                  if ((i == argc) || !sscanf(argv[i],"%f",(float *) &x))
                    Error("Missing value on -gamma",(char *) NULL);
                }
              break;
            }
          if (strncmp("geometry",option+1,2) == 0)
            {
              resource_info.image_geometry=(char *) NULL;
              if (*option == '-')
                {
                  i++;
                  if ((i == argc) || !sscanf(argv[i],"%d",&x))
                    Error("Missing geometry on -geometry",(char *) NULL);
                  resource_info.image_geometry=argv[i];
                }
              break;
            }
          Error("Unrecognized option",option);
          break;
        }
        case 'h':
        {
          if (strncmp("help",option+1,2) == 0)
            Usage(True);
          Error("Unrecognized option",option);
          break;
        }
        case 'i':
        {
          if (strncmp("iconGeometry",option+1,5) == 0)
            {
              resource_info.icon_geometry=(char *) NULL;
              if (*option == '-')
                {
                  i++;
                  if ((i == argc) || !sscanf(argv[i],"%d",&x))
                    Error("Missing geometry on -iconGeometry",(char *) NULL);
                  resource_info.icon_geometry=argv[i];
                }
              break;
            }
          if (strncmp("iconic",option+1,5) == 0)
            {
              resource_info.iconic=(*option == '-');
              break;
            }
          if (strncmp("immutable",option+1,5) == 0)
            {
              resource_info.immutable=(*option == '-');
              break;
            }
          if (strncmp("interlace",option+1,3) == 0)
            {
              image_info.interlace=NoneInterlace;
              if (*option == '-')
                {
                  i++;
                  if (i == argc)
                    Error("Missing type on -interlace",(char *) NULL);
                  option=argv[i];
                  image_info.interlace=UndefinedInterlace;
                  if (Latin1Compare("none",option) == 0)
                    image_info.interlace=NoneInterlace;
                  if (Latin1Compare("line",option) == 0)
                    image_info.interlace=LineInterlace;
                  if (Latin1Compare("plane",option) == 0)
                    image_info.interlace=PlaneInterlace;
                  if (image_info.interlace == UndefinedInterlace)
                    Error("Invalid interlace type on -interlace",option);
                }
              break;
            }
          Error("Unrecognized option",option);
          break;
        }
        case 'l':
        {
          if (strncmp("label",option+1,2) == 0)
            {
              if (*option == '-')
                {
                  i++;
                  if (i == argc)
                    Error("Missing label name on -label",(char *) NULL);
                }
              break;
            }
          Error("Unrecognized option",option);
          break;
        }
        case 'm':
        {
          if (strncmp("magnify",option+1,3) == 0)
            {
              resource_info.magnify=2;
              if (*option == '-')
                {
                  i++;
                  if ((i == argc) || !sscanf(argv[i],"%d",&x))
                    Error("Missing level on -magnify",(char *) NULL);
                  resource_info.magnify=atoi(argv[i]);
                }
              break;
            }
          if (strncmp("map",option+1,3) == 0)
            {
              resource_info.map_type=(char *) NULL;
              if (*option == '-')
                {
                  i++;
                  if (i == argc)
                    Error("Missing map type on -map",(char *) NULL);
                  resource_info.map_type=argv[i];
                }
              break;
            }
          if (strcmp("matte",option+1) == 0)
            break;
          if (strncmp("mattecolor",option+1,6) == 0)
            {
              resource_info.matte_color=(char *) NULL;
              if (*option == '-')
                {
                  i++;
                  if (i == argc)
                    Error("Missing color on -mattecolor",(char *) NULL);
                  resource_info.matte_color=argv[i];
                }
              break;
            }
          if (strncmp("modulate",option+1,3) == 0)
            {
              if (*option == '-')
                {
                  i++;
                  if ((i == argc) || !sscanf(argv[i],"%f",(float *) &x))
                    Error("Missing value on -modulate",(char *) NULL);
                }
              break;
            }
          if (strncmp("monochrome",option+1,3) == 0)
            {
              resource_info.monochrome=(*option == '-');
              if (resource_info.monochrome)
                {
                  resource_info.number_colors=2;
                  resource_info.tree_depth=8;
                  resource_info.colorspace=GRAYColorspace;
                }
              break;
            }
          Error("Unrecognized option",option);
          break;
        }
        case 'n':
        {
          if (strncmp("name",option+1,2) == 0)
            {
              resource_info.name=(char *) NULL;
              if (*option == '-')
                {
                  i++;
                  if (i == argc)
                    Error("Missing name on -name",(char *) NULL);
                  resource_info.name=argv[i];
                }
              break;
            }
          if (strncmp("negate",option+1,2) == 0)
            break;
          if (strncmp("noise",option+1,3) == 0)
            break;
          if (strncmp("normalize",option+1,3) == 0)
            break;
          Error("Unrecognized option",option);
          break;
        }
        case 'o':
        {
          if (strncmp("opaque",option+1,2) == 0)
            {
              if (*option == '-')
                {
                  i++;
                  if (i == argc)
                    Error("Missing color on -opaque",(char *) NULL);
                }
              break;
            }
          Error("Unrecognized option",option);
          break;
        }
        case 'p':
        {
          if (strncmp("page",option+1,3) == 0)
            {
              image_info.page=(char *) NULL;
              if (*option == '-')
                {
                  i++;
                  if (i == argc)
                    Error("Missing page geometry on -page",(char *) NULL);
                  image_info.page=PostscriptGeometry(argv[i]);
                }
              break;
            }
          if (strncmp("pen",option+1,2) == 0)
            {
              if (*option == '-')
                {
                  i++;
                  if (i == argc)
                    Error("Missing color on -pen",(char *) NULL);
                }
              break;
            }
          Error("Unrecognized option",option);
          break;
        }
        case 'q':
        {
          if (strncmp("quality",option+1,2) == 0)
            {
              image_info.quality=0;
              if (*option == '-')
                {
                  i++;
                  if ((i == argc) || !sscanf(argv[i],"%d",&x))
                    Error("Missing quality on -quality",(char *) NULL);
                  image_info.quality=atoi(argv[i]);
                }
              break;
            }
          Error("Unrecognized option",option);
          break;
        }
        case 'r':
        {
          if (strncmp("raise",option+1,2) == 0)
            {
              i++;
              if ((i == argc) || !sscanf(argv[i],"%d",&x))
                Error("Missing bevel width on -raise",(char *) NULL);
              break;
            }
          if (strncmp("roll",option+1,3) == 0)
            {
              if (*option == '-')
                {
                  i++;
                  if ((i == argc) || !sscanf(argv[i],"%d",&x))
                    Error("Missing geometry on -roll",(char *) NULL);
                }
              break;
            }
          if (strncmp("rotate",option+1,3) == 0)
            {
              if (*option == '-')
                {
                  i++;
                  if ((i == argc) || !sscanf(argv[i],"%f",(float *) &x))
                    Error("Missing degrees on -rotate",(char *) NULL);
                }
              break;
            }
          Error("Unrecognized option",option);
          break;
        }
        case 's':
        {
          if (strncmp("sample",option+1,2) == 0)
            {
              if (*option == '-')
                {
                  i++;
                  if ((i == argc) || !sscanf(argv[i],"%d",&x))
                    Error("Missing geometry on -sample",(char *) NULL);
                }
              break;
            }
          if (strncmp("scene",option+1,3) == 0)
            {
              first_scene=0;
              last_scene=0;
              if (*option == '-')
                {
                  i++;
                  if ((i == argc) || !sscanf(argv[i],"%d",&x))
                    Error("Missing scene number on -scene",(char *) NULL);
                  first_scene=atoi(argv[i]);
                  last_scene=first_scene;
                  (void) sscanf(argv[i],"%u-%u",&first_scene,&last_scene);
                }
              break;
            }
          if (strncmp("segment",option+1,3) == 0)
            {
              if (*option == '-')
                {
                  i++;
                  if ((i == argc) || !sscanf(argv[i],"%f",(float *) &x))
                    Error("Missing threshold on -segment",(char *) NULL);
                }
              break;
            }
          if (strncmp("sharpen",option+1,3) == 0)
            {
              if (*option == '-')
                {
                  i++;
                  if ((i == argc) || !sscanf(argv[i],"%f",(float *) &x))
                    Error("Missing factor on -sharpen",(char *) NULL);
                }
              break;
            }
          if (strncmp("shear",option+1,3) == 0)
            {
              if (*option == '-')
                {
                  i++;
                  if ((i == argc) || !sscanf(argv[i],"%f",(float *) &x))
                    Error("Missing shear geometry on -shear",(char *) NULL);
                }
              break;
            }
          if (strncmp("shared_memory",option+1,4) == 0)
            {
              resource_info.use_shared_memory=(*option == '-');
              break;
            }
          if (strncmp("size",option+1,2) == 0)
            {
              image_info.size=(char *) NULL;
              if (*option == '-')
                {
                  i++;
                  if ((i == argc) || !sscanf(argv[i],"%d",&x))
                    Error("Missing geometry on -size",(char *) NULL);
                  image_info.size=argv[i];
                }
              break;
            }
          if (strncmp("spread",option+1,2) == 0)
            {
              if (*option == '-')
                {
                  i++;
                  if ((i == argc) || !sscanf(argv[i],"%d",&x))
                    Error("Missing amount on -spread",(char *) NULL);
                }
              break;
            }
          Error("Unrecognized option",option);
          break;
        }
        case 't':
        {
          if (strncmp("text_font",option+1,3) == 0)
            {
              resource_info.text_font=(char *) NULL;
              if (*option == '-')
                {
                  i++;
                  if (i == argc)
                    Error("Missing font name on -text_font",(char *) NULL);
                  resource_info.text_font=argv[i];
                }
              break;
            }
          if (strncmp("texture",option+1,5) == 0)
            {
              image_info.texture=(char *) NULL;
              if (*option == '-')
                {
                  i++;
                  if (i == argc)
                    Error("Missing filename on -texture",(char *) NULL);
                  image_info.texture=argv[i];
                }
              break;
            }
          if (strncmp("title",option+1,2) == 0)
            {
              resource_info.title=(char *) NULL;
              if (*option == '-')
                {
                  i++;
                  if (i == argc)
                    Error("Missing title on -title",(char *) NULL);
                  resource_info.title=argv[i];
                }
              break;
            }
          if (strncmp("transparent",option+1,3) == 0)
            {
              if (*option == '-')
                {
                  i++;
                  if (i == argc)
                    Error("Missing color on -transparent",(char *) NULL);
                }
              break;
            }
          if (strncmp("treedepth",option+1,3) == 0)
            {
              resource_info.tree_depth=0;
              if (*option == '-')
                {
                  i++;
                  if ((i == argc) || !sscanf(argv[i],"%d",&x))
                    Error("Missing depth on -treedepth",(char *) NULL);
                  resource_info.tree_depth=atoi(argv[i]);
                }
              break;
            }
          Error("Unrecognized option",option);
          break;
        }
        case 'u':
        {
          if (strncmp("update",option+1,2) == 0)
            {
              resource_info.update=(*option == '-');
              if (*option == '-')
                {
                  i++;
                  if ((i == argc) || !sscanf(argv[i],"%d",&x))
                    Error("Missing seconds on -update",(char *) NULL);
                  resource_info.delay=atoi(argv[i]);
                }
              break;
            }
          if (strncmp("use_pixmap",option+1,2) == 0)
            {
              resource_info.use_pixmap=(*option == '-');
              break;
            }
          Error("Unrecognized option",option);
          break;
        }
        case 'v':
        {
          if (strncmp("verbose",option+1,2) == 0)
            {
              image_info.verbose=(*option == '-');
              break;
            }
          if (strncmp("visual",option+1,2) == 0)
            {
              resource_info.visual_type=(char *) NULL;
              if (*option == '-')
                {
                  i++;
                  if (i == argc)
                    Error("Missing visual class on -visual",(char *) NULL);
                  resource_info.visual_type=argv[i];
                }
              break;
            }
          Error("Unrecognized option",option);
          break;
        }
        case 'w':
        {
          if (strcmp("window",option+1) == 0)
            {
              resource_info.window_id=(char *) NULL;
              if (*option == '-')
                {
                  i++;
                  if (i == argc)
                    Error("Missing id, name, or 'root' on -window",
                      (char *) NULL);
                  resource_info.window_id=argv[i];
                }
              break;
            }
          if (strncmp("window_group",option+1,7) == 0)
            {
              resource_info.window_group=(char *) NULL;
              if (*option == '-')
                {
                  i++;
                  if (i == argc)
                    Error("Missing id, name, or 'root' on -window_group",
                      (char *) NULL);
                  resource_info.window_group=argv[i];
                }
              break;
            }
          if (strncmp("write",option+1,2) == 0)
            {
              resource_info.write_filename=(char *) NULL;
              if (*option == '-')
                {
                  i++;
                  if (i == argc)
                    Error("Missing file name on -write",(char *) NULL);
                  resource_info.write_filename=argv[i];
                  if (access(resource_info.write_filename,0) == 0)
                    {
                      char
                        answer[2];

                      (void) fprintf(stderr,"Overwrite %s? ",
                        resource_info.write_filename);
                      (void) fgets(answer,sizeof(answer)-1,stdin);
                      if (!((*answer == 'y') || (*answer == 'Y')))
                        exit(1);
                    }
                }
              break;
            }
          Error("Unrecognized option",option);
          break;
        }
        case '?':
        {
          Usage(True);
          break;
        }
        default:
        {
          Error("Unrecognized option",option);
          break;
        }
      }
    else
      for (scene=first_scene; scene <= last_scene ; scene++)
      {
        /*
          Option is a file name: begin by reading image from specified file.
        */
        (void) strcpy(image_info.filename,option);
        if (first_scene != last_scene)
          {
            char
              filename[MaxTextLength];

            /*
              Form filename for multi-part images.
            */
            (void) sprintf(filename,image_info.filename,scene);
            if (strcmp(filename,image_info.filename) == 0)
              (void) sprintf(filename,"%s.%u",image_info.filename,scene);
            (void) strcpy(image_info.filename,filename);
          }
        (void) strcpy(image_info.magick,"MIFF");
        image_info.server_name=resource_info.server_name;
        image_info.font=resource_info.font;
        image_info.dither=resource_info.dither;
        image_info.monochrome=resource_info.monochrome;
        resource_info.image_info=(&image_info);
        image=ReadImage(&image_info);
        if (image == (Image *) NULL)
          if ((i < (argc-1)) || (scene < last_scene))
            continue;
          else
            {
              state|=ExitState;
              break;
            }
        do
        {
          /*
            Transmogrify image as defined by the image processing options.
          */
          resource_info.quantum=1;
          MogrifyImage(&image_info,i,argv,&image);
          if (first_scene != last_scene)
            image->scene=scene;
          /*
            Display image to X server.
          */
          if (resource_info.window_id != (char *) NULL)
            {
              unsigned int
                status;

              /*
                Display image to a specified X window.
              */
              status=XDisplayBackgroundImage(display,&resource_info,image);
              if (status)
                state|=RetainColorsState;
              if (resource_info.delay == 0)
                state|=ExitState;
            }
          else
            do
            {
              Image
                *loaded_image;

              /*
                Display image to X server.
              */
              loaded_image=
                XDisplayImage(display,&resource_info,argv,argc,&image,&state);
              if (loaded_image == (Image *) NULL)
                break;
              while ((loaded_image != (Image *) NULL) && (!(state & ExitState)))
              {
                if (loaded_image->montage != (char *) NULL)
                  {
                    /*
                      User selected a visual directory image (montage).
                    */
                    DestroyImages(image);
                    image=loaded_image;
                    break;
                  }
                MogrifyImage(&image_info,i,argv,&loaded_image);
                if (first_scene != last_scene)
                  image->scene=scene;
                next_image=XDisplayImage(display,&resource_info,argv,argc,
                  &loaded_image,&state);
                if (loaded_image != image)
                  DestroyImages(loaded_image);
                loaded_image=next_image;
              }
            } while (!(state & ExitState));
          if (resource_info.write_filename != (char *) NULL)
            {
              /*
                Write image.
              */
              (void) strcpy(image->filename,resource_info.write_filename);
              (void) WriteImage(&image_info,image);
            }
          if (image_info.verbose)
            DescribeImage(image,stderr,False);
          /*
            Proceed to next/previous image.
          */
          next_image=image;
          if (state & FormerImageState)
            for (j=0; j < resource_info.quantum; j++)
            {
              next_image=next_image->previous;
              if (next_image == (Image *) NULL)
                break;
            }
          else
            for (j=0; j < resource_info.quantum; j++)
            {
              next_image=next_image->next;
              if (next_image == (Image *) NULL)
                break;
            }
          if (next_image != (Image *) NULL)
            image=next_image;
        } while ((next_image != (Image *) NULL) && !(state & ExitState));
        /*
          Free image resources.
        */
        DestroyImages(image);
        if (!(state & FormerImageState))
          image_marker[i]=image_number++;
        else
          {
            /*
              Proceed to previous image.
            */
            for (i--; i > 0; i--)
              if (image_marker[i] == (image_number-2))
                break;
            if (image_number != 0)
              image_number--;
          }
        if (state & ExitState)
          break;
      }
    /*
      Determine if we should proceed to the first image.
    */
    if (i == (argc-1))
      if (resource_info.confirm_exit && (state & NextImageState))
        {
          unsigned int
            status;

          /*
            Confirm program exit.
          */
          status=XConfirmWidget(display,windows,"Do you really want to exit",
            client_name);
          if (status == False)
            {
              i=0;
              image_number=0;
            }
        }
  }
  if (state & RetainColorsState)
    XRetainWindowColors(display,XRootWindow(display,XDefaultScreen(display)));
  free((char *) image_marker);
  free((char *) image_info.filename);
  XCloseDisplay(display);

  /* Kobus : change exit to a return, and removed spurious return. */
  return(0);
  /* End Kobus */
}

#else

int im_display_main(int dummy_argc, char** dummy_argv)
{
   set_error("Routine im_display_main() needs to be compiled with X11");
   return ERROR;
}

#endif 

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

#ifdef __cplusplus
}
#endif

#endif   /* #ifndef lint */

#endif   /* #ifndef __C2MAN__ */

