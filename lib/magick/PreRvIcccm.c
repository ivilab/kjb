
/* $Id: PreRvIcccm.c 4727 2009-11-16 20:53:54Z kobus $ */

#ifndef __C2MAN__     

/*
 * Make sure that these files are not a liability when there is no X11. If this
 * is the case, then comment out the whole thing.
*/
#ifdef KJB_HAVE_X11 

/* -------------------------------------------------------------------------- */

#include "magick/magick.h"

#ifdef __cplusplus
extern "C" {
#endif

/* -------------------------------------------------------------------------- */

#ifdef PRE_R6_ICCCM
/*
  Compatibility routines for pre X11R6 ICCCM.
*/
Status XInitImage(ximage)
XImage
  *ximage;
{
  Display
    display;

  ScreenFormat
    screen_format;

  XImage
    *created_ximage,
    target_ximage;

  /*
    Initialize the X image.
  */
  screen_format.depth=ximage->depth;
  screen_format.bits_per_pixel=(int) ximage->bits_per_pixel;
  display.byte_order=ximage->byte_order;
  display.bitmap_unit=ximage->bitmap_unit;
  display.bitmap_bit_order=ximage->bitmap_bit_order;
  display.pixmap_format=(&screen_format);
  display.nformats=1;
  created_ximage=XCreateImage(&display,(Visual *) NULL,ximage->depth,
    ximage->format,ximage->xoffset,(char *) NULL,ximage->width,ximage->height,
    ximage->bitmap_pad,ximage->bytes_per_line);
  if (created_ximage == (XImage *) NULL)
    return(0);
  target_ximage=(*ximage);
  *ximage=(*created_ximage);
  created_ximage->data=(char *) NULL;
  XDestroyImage(created_ximage);
  ximage->red_mask=target_ximage.red_mask;
  ximage->green_mask=target_ximage.green_mask;
  ximage->blue_mask=target_ximage.blue_mask;
  return(1);
}
#endif

#ifdef PRE_R5_ICCCM
/*
  Compatibility routines for pre X11R5 ICCCM.
*/
void XrmCombineDatabase(source,target,override)
XrmDatabase
  source,
  *target;

Bool
  override;
{
  XrmMergeDatabases(source,target);
}

Status XrmCombineFileDatabase(filename,target,override)
const char
  *filename;

XrmDatabase
  *target;

Bool
  override;
{
  XrmDatabase
    *combined_database,
    source;

  source=XrmGetFileDatabase(filename);
  if (!override)
    XrmMergeDatabases(source,target);
  return(1);
}

XrmDatabase XrmGetDatabase(display)
Display
  *display;
{
  return(display->db);
}
#endif

#ifdef PRE_R4_ICCCM
/*
  Compatibility routines for pre X11R4 ICCCM.
*/
XClassHint *XAllocClassHint()
{
  return((XClassHint *) malloc(sizeof(XClassHint)));
}

XIconSize *XAllocIconSize()
{
  return((XIconSize *) malloc(sizeof(XIconSize)));
}

XSizeHints *XAllocSizeHints()
{
  return((XSizeHints *) malloc(sizeof(XSizeHints)));
}

Status XReconfigureWMWindow(display,window,screen_number,value_mask,values)
Display
  *display;

Window
  window;

int
  screen_number;

unsigned int
  value_mask;

XWindowChanges
  *values;
{
  return(XConfigureWindow(display,window,value_mask,values));
}

XStandardColormap *XAllocStandardColormap()
{
  return((XStandardColormap *) malloc(sizeof(XStandardColormap)));
}

XWMHints *XAllocWMHints()
{
  return((XWMHints *) malloc(sizeof(XWMHints)));
}

Status XGetGCValues(display,gc,mask,values)
Display
  *display;

GC
  gc;

unsigned long
  mask;

XGCValues
  *values;
{
  return(True);
}

Status XGetRGBColormaps(display,window,colormap,count,property)
Display
  *display;

Window
  window;

XStandardColormap
  **colormap;

int
  *count;

Atom
  property;
{
  *count=1;
  return(XGetStandardColormap(display,window,*colormap,property));
}

Status XGetWMColormapWindows(display,window,colormap_windows,number_windows)
Display
  *display;

Window
  window,
  **colormap_windows;

int
  *number_windows;
{
  Atom
    actual_type,
    *data,
    property;

  int
    actual_format,
    status;

  unsigned long
    leftover,
    number_items;

  property=XInternAtom(display,"WM_COLORMAP_WINDOWS",False);
  if (property == None)
    return(False);
  /*
    Get the window property.
  */
  *data=(Atom) NULL;
  status=XGetWindowProperty(display,window,property,0L,1000000L,False,
    XA_WINDOW,&actual_type,&actual_format,&number_items,&leftover,
    (unsigned char **) &data);
  if (status != Success)
    return(False);
  if ((actual_type != XA_WINDOW) || (actual_format != 32))
    {
      if (data != (Atom *) NULL)
        XFree((char *) data);
      return(False);
    }
  *colormap_windows=(Window *) data;
  *number_windows=(int) number_items;
  return(True);
}

Status XGetWMName(display,window,text_property)
Display
  *display;

Window
  window;

XTextProperty
  *text_property;
{
  char
    *window_name;

  if (XFetchName(display,window,&window_name) == 0)
    return(False);
  text_property->value=(unsigned char *) window_name;
  text_property->encoding=XA_STRING;
  text_property->format=8;
  text_property->nitems=strlen(window_name);
  return(True);
}

char *XResourceManagerString(display)
Display
  *display;
{
  return(display->xdefaults);
}

void XrmDestroyDatabase(database)
XrmDatabase
  database;
{
}

void XSetWMIconName(display,window,property)
Display
  *display;

Window
  window;

XTextProperty
  *property;
{
  XSetIconName(display,window,property->value);
}

void XSetWMName(display,window,property)
Display
  *display;

Window
  window;

XTextProperty
  *property;
{
  XStoreName(display,window,property->value);
}

int XStringListToTextProperty(argv,argc,property)
char
  **argv;

int
  argc;

XTextProperty
  *property;
{
  register int
    i;

  register unsigned int
    number_bytes;

  XTextProperty
     protocol;

  number_bytes=0;
  for (i=0; i < argc; i++)
    number_bytes+=(unsigned int) ((argv[i] ? strlen(argv[i]) : 0)+1);
  protocol.encoding=XA_STRING;
  protocol.format=8;
  protocol.nitems=0;
  if (number_bytes)
    protocol.nitems=number_bytes-1;
  protocol.value=NULL;
  if (number_bytes <= 0)
    {
      protocol.value=(unsigned char *) malloc(sizeof(char));
      if (!protocol.value)
        return(False);
      *protocol.value='\0';
    }
  else
    {
      register char
        *buffer;

      buffer=(char *) malloc(number_bytes*sizeof(char));
      if (buffer == (char *) NULL)
        return(False);
      protocol.value=(unsigned char *) buffer;
      for (i=0; i < argc; i++)
      {
        char
          *argument;

        argument=argv[i];
        if (!argument)
          *buffer++='\0';
        else
          {
            (void) strcpy(buffer,argument);
            buffer+=(strlen(argument)+1);
          }
      }
    }
  *property=protocol;
  return(True);
}

void XSetWMProperties(display,window,window_name,icon_name,argv,argc,
  size_hints,manager_hints,class_hint)
Display
  *display;

Window
  window;

XTextProperty
  *window_name,
  *icon_name;

char
  **argv;

int
  argc;

XSizeHints
  *size_hints;

XWMHints
  *manager_hints;

XClassHint
  *class_hint;
{
  XSetStandardProperties(display,window,window_name->value,icon_name->value,
    None,argv,argc,size_hints);
  XSetWMHints(display,window,manager_hints);
  XSetClassHint(display,window,class_hint);
}

Status XSetWMProtocols(display,window,protocols,count)
Display
  *display;

Window
  window;

Atom
  *protocols;

int
  count;
{
  Atom
    wm_protocols;

  wm_protocols=XInternAtom(display,"WM_PROTOCOLS",False);
  XChangeProperty(display,window,wm_protocols,XA_ATOM,32,PropModeReplace,
    (unsigned char *) protocols, count);
  return(True);
}

VisualID XVisualIDFromVisual(visual)
Visual
  *visual;
{
  return(visual->visualid);
}

Status XWithdrawWindow(display,window,screen)
Display
  *display;

Window
  window;

int
  screen;
{
  return(XUnmapWindow(display,window));
}

int XWMGeometry(display,screen,user_geometry,default_geometry,border_width,
  size_hints,x,y,width,height,gravity)
Display
  *display;

int
  screen;

char
  *user_geometry,
  *default_geometry;

unsigned int
  border_width;

XSizeHints
  *size_hints;

int
  *x,
  *y,
  *width,
  *height,
  *gravity;
{
  int
    status;

  status=XGeometry(display,screen,user_geometry,default_geometry,border_width,
    0,0,0,0,x,y,width,height);
  *gravity=NorthWestGravity;
  return(status);
}

#endif

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

#ifdef __cplusplus
}
#endif

/* -------------------------------------------------------------------------- */

#endif   /* #ifdef KJB_HAVE_X11  */

#endif   /* #ifndef __C2MAN__  */

