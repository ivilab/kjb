
/* $Id: error.h 4727 2009-11-16 20:53:54Z kobus $ */

#ifndef __C2MAN__     

/*
 * Make sure that these files are not a liability when there is no X11. If this
 * is the case, then comment out the whole thing.
*/
#ifdef KJB_HAVE_X11 

/* -------------------------------------------------------------------------- */


#ifndef MAGICK_ERROR_INCLUDED   /* Kobus */
#define MAGICK_ERROR_INCLUDED   /* Kobus */

#ifdef __cplusplus
extern "C" {
#ifdef COMPILING_CPLUSPLUS_SOURCE
namespace kjb_c {
#endif
#endif


/*
  Error typedef declarations.
*/
typedef void
  (*ErrorHandler) _Declare((char *,char *));

/*
  Error declarations.
*/
extern ErrorHandler
  SetErrorHandler _Declare((ErrorHandler)),
  SetWarningHandler _Declare((ErrorHandler));

extern void
  Error _Declare((char *,char *)),
  Warning _Declare((char *,char *));


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

