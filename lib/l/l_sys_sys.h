
/* $Id: l_sys_sys.h 22174 2018-07-01 21:49:18Z kobus $ */

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

#ifndef L_SYS_SYS_INCLUDED
#define L_SYS_SYS_INCLUDED

/*
 * This file MUST be included BEFORE any system files. 
 *
 * This file should NOT include any other files.
 *
 * This file just makes sure that the preferred system symbol is defined for
 * the given system.  
 *
 * Defines in all caps will invariably be from compile lines. It is normally
 * possible to determine which environment one is on by compiler provided
 * defines, but this is system/compiler dependent. This file tries to determine
 * which environment we are in, but the safest approach is to define one of the
 * uppercase symbols on the compile line. This way, when you switch compilers
 * things will work.
*/



#ifdef __cplusplus
extern "C" {
#ifdef COMPILING_CPLUSPLUS_SOURCE
namespace kjb_c {
#endif
#endif


/* 
 * Keeping c2man happy is tricky business. It is a very old program, so it it
 * trips up on newer features in header files. We simplify debugging by
 * preprocessing first, and then handing off to c2man. This is via the
 * environment variable C2MAN_CPP_COMMAND defined in init_compile. 
 *
 * Like many items in this file, the __C2MAN__ stuff needs to be included before
 * system includes. 
*/

#ifdef __C2MAN__
#    define __attribute__(X)
#    define __restrict
#    define __asm(X)

#    ifdef MAC_OSX
        typedef void* __builtin_va_list;
        typedef void* __gnuc_va_list;
#       define __GNUC_VA_LIST

        typedef enum
        {
            false = 0,
            true = 1
        }
        _Bool;

        typedef _Bool bool;
#    endif 
#endif    /* __C2MAN__ */

/*
// The following entry for SUN5 must be before that for SUN4.
*/
#ifndef SUN5               /* SUN5 */
#    ifdef __sun
#        ifdef __SVR4
#            define SUN5
#        endif
#    endif
#endif


#ifndef SUN4               /* SUN4 */
#    ifndef SUN5
#        ifdef sun
#            define SUN4
#        endif
#    endif
#endif


#ifndef HPUX               /* HPUX */
#    ifdef __hpux
#        define HPUX
#    endif
#endif


#ifndef SGI                 /* SGI */
#    ifdef sgi
#        define SGI
#    endif
#endif


#ifndef NEXT                /* NEXT */
#    ifdef NeXT
#        define NEXT
#    endif
#endif


#ifndef AIX
#    ifdef _AIX
#        define AIX
#    endif
#endif


#ifndef VMS                 /* VMS */
#    ifdef vms
#        define VMS
#    endif
#endif

#ifndef DOS_REAL_16
#    ifdef __MSDOS__
#        ifndef _Windows
#            ifndef __DPMI16__
#                define DOS_REAL_16
#            endif
#        endif
#    endif
#endif

#ifndef DOS_PROTECTED_16
#    ifdef __MSDOS__
#        ifndef _Windows
#            ifdef __DPMI16__
#                define DOS_PROTECTED_16
#            endif
#        endif
#    endif
#endif

#ifndef DOS_PROTECTED_32
#    ifdef __DPMI32__
#        ifdef __CONSOLE__
#            define DOS_PROTECTED_32
#        endif
#    endif
#endif

#ifndef WIN16
#    ifdef _Windows
#        ifndef __WIN32__
#           define WIN_16
#        endif
#    endif
#endif

#ifndef WIN32
#    ifdef __WIN32__
#        ifndef DOS_PROTECTED_32
#             define WIN32
#        endif 
#    endif
#endif

#ifndef WIN32
#    ifdef __WIN32
#        ifndef DOS_PROTECTED_32
#             define WIN32
#        endif 
#    endif
#endif

#ifndef WIN32
#    ifdef _WIN32
#        ifndef DOS_PROTECTED_32
#             define WIN32
#        endif 
#    endif
#endif


/* -------------------------------------------------------------------------- */


/*
// Now define additional symbols which should be define before l_sys_std.h is
// included.
*/


/* -------------------------------------------------------------------------- */
#ifdef MAC_OSX                      /*   MAC_OSX    */

#    ifndef __EXTENSIONS__
#        define __EXTENSIONS__
#    endif

#    ifndef BSD_SYSTEM
#        define BSD_SYSTEM
#    endif

#    ifndef UNIX_SYSTEM
#        define UNIX_SYSTEM
#    endif

#    ifndef _BSD_SOURCE
#        define _BSD_SOURCE
#    endif

#    ifndef _XOPEN_SOURCE
#        define _XOPEN_SOURCE
#    endif

#ifdef HOW_IT_WAS_JAN_12_2017
     /* Currently, isnand maps to either __isnand or __isnanf depending on the
     // size of the argument, but it seems broken, so we will just go for the
     // correct answer.
     //
     // (How it was):
     //   #    define isnand isnan
     */
    extern int __isnand(double x);
    extern int __isnanf(float x);

#ifdef COMPILING_CPLUSPLUS_SOURCE
#    define isnand ::kjb_c::__isnand
#    define isnanf ::kjb_c::__isnanf
#else
#    define isnand __isnand
#    define isnanf __isnanf
#endif
#else 
    /* It seems that both C and C++ do the right thing with isnan as a function
     * of their arugumenta. 
    */
#    define isnand isnan
#    define isnanf isnan
#endif 

#    define isfinited isfinite
#    define isfinitef isfinite

#    define KJB_HAVE_FINITE
#    define KJB_HAVE_ISNAN

#endif   /* MAC_OSX */


/* ----------------------------- LINUX_386 ------------------------------ */

/* Backwards compatabilty. We have been loosely referring to LINUX_X86_32 as
 * LINUX_386.
*/
#ifdef LINUX_X86_64
#   define LINUX_X86
#else
#   ifdef LINUX_386
#       ifndef LINUX_X86_32
#           define LINUX_X86_32
#       endif

#       undef LINUX_386   /* This should be purged from the code. */
#       define LINUX_X86  /* A better name. */
#   endif
#endif

/* -------------------------------------------------------------------------- */

#ifdef LINUX_X86               /*   LINUX_X86    */

#    ifndef __EXTENSIONS__
#        define __EXTENSIONS__
#    endif

#    ifndef BSD_SYSTEM
#        define BSD_SYSTEM
#    endif

#    ifndef UNIX_SYSTEM
#        define UNIX_SYSTEM
#    endif

#    ifndef _BSD_SOURCE
#        define _BSD_SOURCE
#    endif

#    ifndef _POSIX_SOURCE
#        define _POSIX_SOURCE
#    endif

#    ifndef _XOPEN_SOURCE
#        define _XOPEN_SOURCE
#    endif

#    define isnand isnan

    /* Linux has a well defined isnanf(), and the following is wrong 
#    define isnanf isnan
    */

#    define isfinited finite
#    define isfinitef finitef

#    define KJB_HAVE_FINITE
#    define KJB_HAVE_ISNAN

#endif   /* LINUX_X86 */

/* -------------------------------------------------------------------------- */

#ifdef SUN5                      /*   SUN5    */

#    ifndef __EXTENSIONS__
#        define __EXTENSIONS__
#    endif

#    ifndef SUN_SYSTEM
#        define SUN_SYSTEM
#    endif

#    ifndef SYSV_SYSTEM
#        define SYSV_SYSTEM
#    endif

#    ifndef UNIX_SYSTEM
#        define UNIX_SYSTEM
#    endif

#    define KJB_HAVE_ISNAN
#    define KJB_HAVE_FINITE

#endif   /* SUN5 */

/* ------------------------------------------------------------------------- */

#ifdef SUN4                       /*   SUN4    */

#    ifndef SUN_SYSTEM
#        define SUN_SYSTEM
#    endif

#    ifndef BSD_SYSTEM
#        define BSD_SYSTEM
#    endif

#    ifndef UNIX_SYSTEM
#        define UNIX_SYSTEM
#    endif

#endif   /* SUN4 */

/* ------------------------------------------------------------------------- */

#ifdef HPUX                        /*   HPUX    */

#    ifndef SYSV_SYSTEM
#        define SYSV_SYSTEM
#    endif

#    ifndef UNIX_SYSTEM
#        define UNIX_SYSTEM
#    endif

#define _INCLUDE_HPUX_SOURCE
#define _INCLUDE_POSIX_SOURCE
#define _INCLUDE_XOPEN_SOURCE
#define _INCLUDE_XOPEN_SOURCE_EXTENDED

#define isnand isnan

#    define KJB_HAVE_ISNAN
#    define KJB_HAVE_FINITE


#endif   /* HPUX */

/* ------------------------------------------------------------------------- */

#ifdef SGI                       /*   SGI    */

#    ifndef SYSV_SYSTEM
#        define SYSV_SYSTEM
#    endif

#    ifndef UNIX_SYSTEM
#        define UNIX_SYSTEM
#    endif

#    define SGI_NEXT

#    define KJB_HAVE_ISNAN
#    define KJB_HAVE_FINITE

#    ifndef __LONGLONG
#        define __LONGLONG
#    endif

#endif   /* SGI */

/* ------------------------------------------------------------------------- */

#ifdef NEXT                        /*   NEXT    */

#    ifndef BSD_SYSTEM
#        define BSD_SYSTEM
#    endif

#    ifndef UNIX_SYSTEM
#        define UNIX_SYSTEM
#    endif

#endif  /* NEXT */

/* ------------------------------------------------------------------------- */

#ifdef AIX                         /* AIX */

   /*
    * Aix provides both bsd and sysv like facilities. However, it seems that the
    * the bsd facilities exist for compatibility reasons. Thus we will attempt
    * to force the use of the sysv facilities.
    */

#    ifndef SYSV_SYSTEM
#        define SYSV_SYSTEM
#    endif

#    ifndef UNIX_SYSTEM
#        define UNIX_SYSTEM
#    endif


#endif   /* AIX */

/* ------------------------------------------------------------------------- */

#ifdef VMS                         /* VMS */

#endif   /*   VMS   */

/* ------------------------------------------------------------------------- */

#ifdef DOS_REAL_16                /* DOS_REAL_16 */

#    ifndef MS_OS
#        define MS_OS
#    endif

#    ifndef MS_16_BIT_OS
#        define MS_16_BIT_OS
#    endif

#endif

/* ------------------------------------------------------------------------- */

#ifdef DOS_PROTECTED_16           /* DOS_PROTECTED_16 */

#    ifndef MS_OS
#        define MS_OS
#    endif

#    ifndef MS_16_BIT_OS
#        define MS_16_BIT_OS
#    endif

#endif

/* ------------------------------------------------------------------------- */

#ifdef DOS_PROTECTED_32           /* DOS_PROTECTED_32 */

#    ifndef MS_OS
#        define MS_OS
#    endif

     /*
     // CHECK -- still may behave as 16 bit? Can we allocate large chunks easily
     */
#    ifndef MS_32_BIT_OS
#        define MS_32_BIT_OS
#    endif

#endif

/* ------------------------------------------------------------------------- */

#ifdef WIN16                         /* WIN16 */

#    ifndef MS_OS
#        define MS_OS
#    endif

#    ifndef MS_16_BIT_OS
#        define MS_16_BIT_OS
#    endif

#endif

/* ------------------------------------------------------------------------- */

#ifdef WIN32                       /* WIN32 */

#    ifndef MS_OS
#        define MS_OS
#    endif

     /*
     // CHECK -- still may behave as 16 bit? Can we allocate large chunks easily
     */
#    ifndef MS_32_BIT_OS
#        define MS_32_BIT_OS
#    endif

#endif
/* ------------------------------------------------------------------------- */


/* ------------------------------------------------------------------------- */
/*
                 -----------------------------------------
                 | Standard/Non-standard related defines |
                 -----------------------------------------
                                   ||
                                  \||/
                                   \/

  Setup stuff so we can deal with both ANSI and non-ANSI C. (THe latter to be
  phased out eventually).

*/

/* ------------------------------- Standard C ------------------------------- */

#ifdef MS_OS

    /* 
     * For now, lets treat the question of being standard on MS OS as being
     * driven by the system header files which are best to be interpreted with
     * __STDC__. 
    */
    
#    ifndef __STDC__
#        define __STDC__
#    endif
     

     /*
      * On MS_OS, POSIX leaves out too many unix function defs 
     */
     /*
#    ifndef _POSIX_
#        define _POSIX_
#    endif
     */

#    ifndef _M_IX86
         /* Presumably, 300 means P3. */
#        define _M_IX86 300
#    endif 

#endif 

#ifndef SUN4
#    define KJB_HAVE_ATEXIT
#endif


#ifdef __C2MAN__
#    ifndef __STDC__
#        define __STDC__
#    endif
#endif

#ifdef __STDC__

#    define SINGLE_PRECISION_CONSTANT_OK
#    define HASH_MARK_SUBSTITUTION_OK
#    define FUNCTION_PROTOTYPE_OK
#    define STANDARD_VAR_ARGS

     /*
     // LONG_DOUBLE_OK
     //
     // The determination that sgi's can't handle long double occured some
     // time ago now. I would bet that they can do it now.      CHECK
     //
     // Currently long doubles break gnu's variable arg on the HP. There may be
     // a way to fix this if we really need to use them.  Alternately,
     // kjb_vfprintf and kjb_vsprintf could be changed so that they set_bug if
     // we get a 'L' on these systems (and the routines that actually print
     // them would have to be adjusted to print doubles instead. Note that
     // currently using fprintf does not solve the problem.
     */
#    ifndef SGI

#        ifndef HPUX
#            define LONG_DOUBLE_OK
#        else
#            ifndef __GNUC__
#                define LONG_DOUBLE_OK
#            endif
#        endif
#    endif

#else

/* We need Borland's extensions, but then they don't define __STDC__. */
#ifdef __BORLANDC__

#    define SINGLE_PRECISION_CONSTANT_OK  /* CHECK */
#    define HASH_MARK_SUBSTITUTION_OK
#    define FUNCTION_PROTOTYPE_OK
#    define STANDARD_VAR_ARGS
#    define LONG_DOUBLE_OK


/* ----------------------------- Non - Standard C --------------------------- */
#else

#    define signed
#    define volatile
#    define const

#endif
#endif
/* -------------------------------------------------------------------------- */

/*
// LONG_DOUBLE_OK: Often (always?) quad precision will be done in software,
// slowing the code down a lot!. Therefore, we only do it if USE_LONG_DOUBLE
// has been specified on the compile line.
*/

#ifndef USE_LONG_DOUBLE
#    ifdef LONG_DOUBLE_OK
#        undef LONG_DOUBLE_OK
#    endif
#endif

/*
// I am not sure this is the best location for these, as we have yet to include
// the standard include files, and it may be possible to figure out in some
// cases whether these have in fact been defined. It also means that when we
// have it wrong, then the compile bonks (which is a good thing for developers,
// bad thing for those simply trying to compile some of our code.)
*/

#ifndef KJB_HAVE_ISNAN
#    define isnand(x) 0
#    define isnanf(x) 0
#endif

#ifndef KJB_HAVE_FINITE
#    define finite(x) 1
#endif


#ifdef __cplusplus
#ifdef COMPILING_CPLUSPLUS_SOURCE
}
#endif
}
#endif

#endif   /* Include this file */

