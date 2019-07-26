
/* $Id: l_sys_std.h 22174 2018-07-01 21:49:18Z kobus $ */

/* =========================================================================== *
|
|  Copyright (c) 1994-2008 by Kobus Barnard (author).
|
|  Personal and educational use of this code is granted, provided that this
|  header is kept intact, and that the authorship is not misrepresented, that
|  its use is acknowledged in publications, and relevant papers are cited.
|
|  For other use contact the author (kobus AT cs DOT arizona DOT edu).
|
|  Please note that the code in this file has not necessarily been adequately
|  tested. Naturally, there is no guarantee of performance, support, or fitness
|  for any particular task. Nonetheless, I am interested in hearing about
|  problems that you encounter.
|
* =========================================================================== */

#ifdef MAKE_DEPEND
#    ifndef L_SYS_STANDARD_INCLUDED
#        define L_SYS_STANDARD_INCLUDED
#    endif
#endif

#ifndef L_SYS_STANDARD_INCLUDED
#define L_SYS_STANDARD_INCLUDED

/* The file l/l_sys_sys.h canonicalizes some definitions. It does not include
// any system includes, so we are still set up for including system includes
// after this point.
*/
#include "l/l_sys_sys.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifdef __GNUC__
/*
// Gcc does some fancy optimizations inline on some string functions like
// "strcpy". At a glance, however, it does not necessarily look like an
// improvment in the case of strcpy (bench marking would be interesting), and
// causes the compiler to emit warnings. Since we do not normally spend a lot of
// CPU doing string stuff, we disable this behaviour (only active when
// optimizing) with the following define.
*/
#ifndef __NO_STRING_INLINES
#    define __NO_STRING_INLINES
#endif 

#ifndef __cplusplus
#    define CV_INLINE  __attribute__ ((unused)) static
#endif


#else     /* Case NOT __GNUC__ follows. */
#    define __attribute__(X)
#endif


/* Integer types defined by the C99 language standard.  */
#  include <inttypes.h>


/*
// --- HPUX --- HPUX --- HPUX --- HPUX --- HPUX --- HPUX --- HPUX --- HPUX ---
//
// FIX -- Try and do without this hack ASAP.
//
// Prepare gcc include of non-gcc header in the case of HPUX. What happens is
// that gcc uses its own type.h which does not define blkcnt_t, off32_t, etc.
// I'm not sure if this is a problem with how we installed gcc, or if they just
// blew it. Nonetheless, there is no <sys/stat.h> in the gcc include directory,
// and so the <sys/stat.h> directory below uses /usr/include/sys/stat.h. The
// gcc include dir is:
// /opt/gcc/lib/gcc-lib/hppa1.1-hp-hpux10.10/2.7.2.1/include Perhaps the
// problem is that we are using hp-10.20, and the gcc we have is for 10.10.
// Another way to fix this problem is to use -I/usr/include but this means all
// users of this code will have to remember to do this, so I don't think it is
// a good idea.
*/
#ifdef HPUX
#ifdef __GNUC__
#  include <sys/sigevent.h>

#  ifndef _BLKCNT_T
#    define _BLKCNT_T
#      ifdef _FILE64
         typedef int64_t blkcnt_t;        /* 64-bit # of blocks */
#      else
         typedef int32_t blkcnt_t;        /* 32-bit # of blocks */
#    endif
#  endif /* _BLKCNT_T */

#  ifndef _OFF32_T
#    define _OFF32_T
        typedef int32_t off32_t;    /* 32bit offsets and sizes */
#  endif /* _OFF32_T */
#endif
#endif


#ifdef SUN4
#ifdef __GNUC__
#define __USE_FIXED_PROTOTYPES__
#endif
#endif

#ifdef STANDARD_VAR_ARGS
#    include <stdarg.h>
#else
#    include <varargs.h>
#endif

#ifdef MAC_OSX
#ifdef __va_list__
#ifndef _VA_LIST_T
#define _VA_LIST_T
#endif 
#endif 
#endif 

#include <sys/types.h>

#ifdef  MS_OS
#    ifdef __STDC__
#        define  off_t _off_t
#        define  dev_t _dev_t
#        define  ino_t _ino_t
#    endif  
#endif 

#ifdef SUN5
#if __STDC__ - 0 == 1
/*
// Maximally conforming ANSI does not have long long, and so <sys/types.h>
// defines longlong_t and u_longlong_t in order to support ANSI level 1 as well
// as 0. However, int64_t and uint64_t do not get defined. This is a problem,
// because some include files do not prepare for this case. As of 13/07/99, the
// only problematic file required is <sys/kstat.h> called from <sys/vm.h> called
// from <sys/swap.h> called from Mike's videograb code (actually, code that
// Parallax suppled.) Likely, once that stuff is turfed, the following lines can
// also be turfed.
*/
typedef longlong_t int64_t;
typedef u_longlong_t uint64_t;
#endif
#endif

#include <ctype.h>

#include <stdio.h>

#ifdef  MS_OS
#    ifdef __STDC__
#        define NEED_MS_OS_STDIO_FUNCTIONS
#    endif 
    
#    ifdef _POSIX_
#        define NEED_MS_OS_STDIO_FUNCTIONS
#    endif 

#    ifdef NEED_MS_OS_STDIO_FUNCTIONS
          /*
           * I just copied the ones that we want to export. Some others are
           * similarly handled in l_sys_io.c. 
          */
#         define fileno _fileno

#         undef NEED_MS_OS_STDIO_FUNCTIONS
#    endif  

#endif 


#include <string.h>
#include <math.h>
#include <stdlib.h>

#ifdef  MS_OS
/* 
 * The header file says that "sleep" is obsolete and that the Win32 API function
 * "Sleep" should be used instead. But lets go with the obsolete one for now, to
 * avoid getting too intimate with the Win32 API> 
*/
#     define sleep _sleep
#endif 


#include <errno.h>
#include <limits.h>

#ifdef WIN32
#    include <string.h>
#else
#    include <strings.h>
#endif 


#ifdef __STDC__
#    include <float.h>
#endif


/*  ---------------------------------- Unix --------------------------------- */
#ifdef UNIX_SYSTEM

#ifdef NEXT
#    include <libc.h>
#else
#    include <unistd.h>
#endif

/*  ---------------------------------- Unix - MAC_OSX  ------------------ */

#ifdef  MAC_OSX
#    include <sys/param.h>
#endif

/*  ---------------------------------- Unix - LINUX_X86  ------------------ */

#ifdef  LINUX_X86
#    include <stdint.h>
#    include <stddef.h>
#endif

/*  ---------------------------------- Unix - DEBIAN ----------------------- */

#ifdef  DEBIAN_386
#    include <ioctls.h>
#    include <ioctl-types.h>
#endif

/*  ---------------------------------- Unix - Solaris ----------------------- */

#ifdef  SUN5
#    include <stddef.h>
#    include <ieeefp.h>
#endif

/*  ---------------------------------- Unix - HPUX -------------------------- */

#ifdef  HPUX
/* Keep empty #ifdef. */
#endif

/*  ---------------------------------- Unix - Sun --------------------------- */

#ifdef  SUN4
/* Keep empty #ifdef. */
#endif

/*  ---------------------------------- Unix - NeXT -------------------------- */
#ifdef  NEXT
/* Keep empty #ifdef. */
#endif

/*  ---------------------------------- Unix - Sgi  -------------------------- */

#ifdef  SGI
/* Keep empty #ifdef. */
#endif

/*  ---------------------------------- Unix - Sysv -------------------------- */

#ifdef SYSV_SYSTEM
/* Keep empty #ifdef. */
#endif

/*  ------------------------------------------------------------------------- */
#endif                             /* End of unix */
/*  ------------------------------------------------------------------------- */


/*  -------------------------------- MS__OS --------------------------------- */

#ifdef  MS_OS

#include <sys/stat.h>
#include <io.h>

#endif   /* End of MS_OS */

/*  ------------------------------------------------------------------------- */


/*  -------------------------------------- VMS ------------------------------ */
#ifdef VMS

#include <unixlib.h>
#include <stat.h>
#include <iodef.h>
#include <descrip.h>

#endif  /* End of VMS */

/*  --------------------------------------------------------------------- */

/* We want to make it so that everybody defines the __sparcv8 symbol the same
// way to avoid messages about it being different.
*/
#ifdef FAST_INCLUDE_PP
#    ifdef __sparcv8
#        undef __sparcv8
#        define __sparcv8 1
#    endif
#endif

#ifdef __cplusplus
}
#endif

#endif    /* Include this file. */

