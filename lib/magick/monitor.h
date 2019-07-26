
/* $Id: monitor.h 4727 2009-11-16 20:53:54Z kobus $ */

#ifndef __C2MAN__     

/*
 * Make sure that these files are not a liability when there is no X11. If this
 * is the case, then comment out the whole thing.
*/
#ifdef KJB_HAVE_X11 

/* -------------------------------------------------------------------------- */


#ifndef MAGICK_MONITOR_INCLUDED   /* Kobus */
#define MAGICK_MONITOR_INCLUDED   /* Kobus */

#ifdef __cplusplus
extern "C" {
#ifdef COMPILING_CPLUSPLUS_SOURCE
namespace kjb_c {
#endif
#endif


/*
  Monitor typedef declarations.
*/
typedef void
  (*MonitorHandler) _Declare((char *,unsigned int,unsigned int));

/*
  Monitor declarations.
*/
extern MonitorHandler
  SetMonitorHandler _Declare((MonitorHandler));

extern void
  ProgressMonitor _Declare((char *,unsigned int,unsigned int));


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

