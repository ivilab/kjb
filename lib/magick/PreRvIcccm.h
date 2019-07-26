
/* $Id: PreRvIcccm.h 4727 2009-11-16 20:53:54Z kobus $ */

#ifndef __C2MAN__     

/*
 * Make sure that these files are not a liability when there is no X11. If this
 * is the case, then comment out the whole thing.
*/
#ifdef KJB_HAVE_X11 

/* -------------------------------------------------------------------------- */


#ifndef MAGICK_PRERVICCCM_INCLUDED   /* Kobus */
#define MAGICK_PRERVICCCM_INCLUDED   /* Kobus */

#ifdef __cplusplus
extern "C" {
#ifdef COMPILING_CPLUSPLUS_SOURCE
namespace kjb_c {
#endif
#endif


#ifdef PRE_R6_ICCCM
/*
  Compatability defines for pre X11R6 ICCCM.
*/
#define XK_KP_Home              0xFF95
#define XK_KP_Left              0xFF96
#define XK_KP_Up                0xFF97
#define XK_KP_Right             0xFF98
#define XK_KP_Down              0xFF99
#define XK_KP_Prior             0xFF9A
#define XK_KP_Page_Up           0xFF9A
#define XK_KP_Next              0xFF9B
#define XK_KP_Page_Down         0xFF9B
#define XK_KP_End               0xFF9C

extern Status
  XInitImage();
#endif

#ifdef PRE_R5_ICCCM
/*
  Compatability defines for pre X11R5 ICCCM.
*/
extern XrmDatabase
  XrmGetDatabase();
#endif

#ifdef PRE_R4_ICCCM
/*
  Compatability defines for pre X11R4 ICCCM.
*/
#ifdef vms
#define XMaxRequestSize(display)  16384
#endif

#define WithdrawnState  0
#define XInductColormap(display,colormap)  XInstallColormap(display,colormap)
#define XUninductColormap(display,colormap)  XUninstallColormap(display,colormap)

typedef struct _XTextProperty
{
  unsigned char
    *value;

  Atom
    encoding;

  int
    format;

  unsigned long
    nitems;
} XTextProperty;

/*
  Pre R4 ICCCM compatibility routines.
*/
char
  *XResourceManagerString();

extern int
  XWMGeometry();

extern Status
  XGetRGBColormaps(),
  XGetWMName(),
  XReconfigureWMWindow(),
  XSetWMProtocols(),
  XWithdrawWindow();

extern XClassHint
  *XAllocClassHint();

extern XIconSize
  *XAllocIconSize();

extern XSizeHints
  *XAllocSizeHints();

extern XStandardColormap
  *XAllocStandardColormap();

extern XWMHints
  *XAllocWMHints();

extern VisualID
  XVisualIDFromVisual();

extern void
  XrmDestroyDatabase(),
  XSetWMIconName(),
  XSetWMName(),
  XSetWMProperties();
#else
#define XInductColormap(display,colormap)
#define XUninductColormap(display,colormap)
#endif


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

