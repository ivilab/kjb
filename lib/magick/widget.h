
/* $Id: widget.h 4727 2009-11-16 20:53:54Z kobus $ */

#ifndef __C2MAN__     

/*
 * Make sure that these files are not a liability when there is no X11. If this
 * is the case, then comment out the whole thing.
*/
#ifdef KJB_HAVE_X11 

/* -------------------------------------------------------------------------- */


#ifndef MAGICK_WIDGET_INCLUDED   /* Kobus */
#define MAGICK_WIDGET_INCLUDED   /* Kobus */

#ifdef __cplusplus
extern "C" {
#ifdef COMPILING_CPLUSPLUS_SOURCE
namespace kjb_c {
#endif
#endif



/*
  Define declarations.
*/
#define BorderOffset  4
#define DoubleClick  250

/*
  Typedef declarations.
*/
typedef struct _XWidgetInfo
{
  int
    id;

  unsigned int
    bevel_width,
    width,
    height;

  int
    x,
    y,
    min_y,
    max_y;

  unsigned int
    raised,
    active,
    center,
    trough,
    highlight;

  char
    *text,
    *cursor,
    *marker;
} XWidgetInfo;

/*
  X utilities routines.
*/
extern int
  XCommandWidget _Declare((Display *,XWindows *,char **,XEvent *)),
  XDialogWidget _Declare((Display *,XWindows *,char *,char *,char *)),
  XMenuWidget _Declare((Display *,XWindows *,char *,char **,char *));

extern unsigned int
  XConfirmWidget _Declare((Display *,XWindows *,char *,char *)),
  XPreferencesWidget _Declare((Display *,XResourceInfo *,XWindows *));

extern void
  XColorBrowserWidget _Declare((Display *,XWindows *,char *,char *)),
  XFileBrowserWidget _Declare((Display *,XWindows *,char *,char *)),
  XFontBrowserWidget _Declare((Display *,XWindows *,char *,char *)),
  XInfoWidget _Declare((Display *,XWindows *,char *)),
  XListBrowserWidget _Declare((Display *,XWindows *,XWindowInfo *,char **,
    char *,char *,char *)),
  XMonitorWidget _Declare((Display *,XWindows *,char *,unsigned int,
    unsigned int)),
  XNoticeWidget _Declare((Display *,XWindows *,char *,char *)),
  XTextViewWidget _Declare((Display *,XResourceInfo *,XWindows *,unsigned int,
    char *,char **));


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

