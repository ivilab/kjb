
/* $Id: utility.h 4727 2009-11-16 20:53:54Z kobus $ */

#ifndef __C2MAN__     

/*
 * Make sure that these files are not a liability when there is no X11. If this
 * is the case, then comment out the whole thing.
*/
#ifdef KJB_HAVE_X11 

/* -------------------------------------------------------------------------- */


#ifndef MAGICK_UTILITY_INCLUDED   /* Kobus */
#define MAGICK_UTILITY_INCLUDED   /* Kobus */


#ifndef vms
#ifndef NoDIRENT
#include <dirent.h>
#else
#include <sys/dir.h>
#ifndef dirent
#define dirent direct
#endif
#endif
#include <pwd.h>
#else
#include "magick/vms.h"
#endif

#ifdef __cplusplus
extern "C" {
#ifdef COMPILING_CPLUSPLUS_SOURCE
namespace kjb_c {
#endif
#endif


#ifndef S_ISDIR
#define S_ISDIR(mode) (((mode) & S_IFMT) == S_IFDIR)
#endif

/*
  Utility define declarations.
*/
#ifndef vms
#define IsGlob(text) \
  ((strchr(text,'*') != (char *) NULL) || \
   (strchr(text,'?') != (char *) NULL) || \
   (strchr(text,'{') != (char *) NULL) || \
   (strchr(text,'}') != (char *) NULL) || \
   (strchr(text,'[') != (char *) NULL) || \
   (strchr(text,']') != (char *) NULL))
#define BasenameSeparator  "/"
#define DirectorySeparator  "/"
#define SystemCommand(command)  system(command)
#define TemporaryDirectory  "/tmp"
#define TemporaryTemplate  "%s/magickXXXXXX"
#else
#define IsGlob(text) \
  ((strchr(text,'*') != (char *) NULL) || \
   (strchr(text,'?') != (char *) NULL) || \
   (strchr(text,'{') != (char *) NULL) || \
   (strchr(text,'}') != (char *) NULL))
#define BasenameSeparator  "]"
#define DirectorySeparator  ""
#define SystemCommand(command)  (!system(command))
#define TemporaryDirectory  "SYS$Disk:[]"
#define TemporaryTemplate  "%smagickXXXXXX."
#endif

/*
  Utilities routines.
*/
extern char
  *ClientName _Declare((char *)),
  **ListColors _Declare((char *,int *)),
  **ListFiles _Declare((char *,char *,int *)),
  *PostscriptGeometry _Declare((char *)),
  **StringToList _Declare((char *));

extern int
  GlobExpression _Declare((char *,char *)),
  ReadDataBlock _Declare((char *,FILE *));

extern unsigned int
  ReadData _Declare((char *,unsigned int,unsigned int,FILE *));

extern unsigned long
  LSBFirstReadLong _Declare((FILE *)),
  MSBFirstReadLong _Declare((FILE *));

extern unsigned short
  LSBFirstReadShort _Declare((FILE *)),
  MSBFirstReadShort _Declare((FILE *));

extern void
  AppendImageFormat _Declare((char *,char *)),
  ExpandFilename _Declare((char *)),
  ExpandFilenames _Declare((int *,char ***)),
  LSBFirstWriteLong _Declare((unsigned long,FILE *)),
  LSBFirstWriteShort _Declare((unsigned int,FILE *)),
  MSBFirstOrderLong _Declare((char *,unsigned int)),
  MSBFirstOrderShort _Declare((char *,unsigned int)),
  MSBFirstWriteLong _Declare((unsigned long,FILE *)),
  MSBFirstWriteShort _Declare((unsigned int,FILE *)),
  TemporaryFilename _Declare((char *));


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

