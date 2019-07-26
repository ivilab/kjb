
/* $Id: l_sys_def.h 21937 2017-11-20 00:22:57Z kobus $ */

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

#ifndef L_SYS_DEF_INCLUDED
#define L_SYS_DEF_INCLUDED

#ifdef __cplusplus
#    ifndef KJB_CPLUSPLUS
#        define KJB_CPLUSPLUS
#    endif
#endif

#ifdef c_plusplus
#    ifndef KJB_CPLUSPLUS
#        define KJB_CPLUSPLUS
#        define __cplusplus
#    endif
#endif


#include "l/l_sys_std.h"

#ifdef __cplusplus
extern "C" {
#ifdef COMPILING_CPLUSPLUS_SOURCE
namespace kjb_c {
#endif
#endif


typedef double Always_double;

#ifdef DOUBLE_IS_FLOAT
#    define double float
#    define log logf
#    define exp expf
#endif

/*
//  This file defines system dependent macros. It is assumed that the system
//  type has been properly assigned (by including l_sys_sys.h) and that the
//  basic system supplied include files have been included. The later is
//  important because we define many of the same things, provided that they are
//  not avalable on the current system.
*/

/* -------------------------------------------------------------------------- */

/*
// We did not define common define's like UNIX, SYSV, etc, in l_sys_sys.h to
// avoid name conflicts with standard header files. However, we defined
// corresponding names like UNIX_SYSTEM so that they are available for use
// in l_sys_std.h and other places where we need to know the system, before we
// can include this file.
*/
#ifdef UNIX_SYSTEM
#    ifndef UNIX
#        define UNIX
#    endif
#endif

#ifdef SYSV_SYSTEM
#    ifndef SYSV
#        define SYSV
#    endif
#endif

#ifdef BSD_SYSTEM
#    ifndef BSD
#        define BSD 1
#    endif
#endif

#ifdef SUN_SYSTEM
#    ifndef SUN
#        define SUN
#    endif
#endif

#ifdef SYSV
#    ifndef SYSV_SIGNALS
#         define SYSV_SIGNALS
#    endif
#endif

/* -------------------------------------------------------------------------- */
/*
              ----------------------------------------------
              | Operating System sub-class related defines |
              ----------------------------------------------
                                   ||
                                  \||/
                                   \/


  System characteristics that should be explicitly set for each type of system
  that follows. It is better to set the characteristics for the specific
  system to ensure that the code breaks if they are not checked. For example,
  although most systems other than DOS use 32 bit integers, we do NOT want to
  set integer size to 32 bits with "#ifndef MSDOS".  Thus it needs to be
  explicitly set for each case, so a new system will break the compile unless
  we have entered it (hopefully correctly).

*/

/* ----------------------------- MAC_OSX -------------------------------- */

#ifdef MAC_OSX

#    ifndef MAC
#       define MAC
#    endif

#    ifdef MAC_OSX_PPC
#        ifndef MSB_FIRST
#            define MSB_FIRST
#        endif
#    endif

#    ifdef MAC_OSX_PPC64
#        ifndef MSB_FIRST
#            define MSB_FIRST
#        endif
#    endif

#    ifdef MAC_OSX_I386
#        ifndef LSB_FIRST
#            define LSB_FIRST
#        endif
#    endif

#    ifdef MAC_OSX_I386_XEON
#        ifndef LSB_FIRST
#            define LSB_FIRST
#        endif
#    endif

#    ifdef MAC_OSX_I386_I5
#        ifndef LSB_FIRST
#            define LSB_FIRST
#        endif
#    endif

#    ifdef MAC_OSX_I386_I7
#        ifndef LSB_FIRST
#            define LSB_FIRST
#        endif
#    endif

#    ifdef MAC_OSX_I386_DUO
#        ifndef LSB_FIRST
#            define LSB_FIRST
#        endif
#    endif

#    ifdef MAC_OSX_X86_64
#        ifndef LSB_FIRST
#            define LSB_FIRST
#        endif
#    endif

#    ifndef SYSV_SIGNALS
#         define SYSV_SIGNALS
#    endif

#    ifndef STACK_SIZE
         /* Default modern mac stack is 8000 K. It can be pushed to 64000 K, but no
          * more. Hence this number should not be too big. About 2000 K seems
          * sensible. 
         */
#        define STACK_SIZE (2000000)
#    endif

/*
 * We cannot reliably guess whether the mac is running ILP32 or LP64 from the
 * processor returned from /bin/arch. In fact, most processers seem to be able
 * to do either. We will have to rely on system includes here.
*/
#ifndef __LP64__
#   ifdef _LP64
#      define __LP64__
#   endif 
#endif 


#ifdef __LP64__
#    define LONG_IS_64_BITS
#    define PTR_IS_64_BITS
#    define SIZE_T_IS_64_BITS
#else
#    define LONG_IS_32_BITS
#    define PTR_IS_32_BITS
#    define SIZE_T_IS_32_BITS
#endif 


#    define INT_IS_32_BITS
#    define SHORT_IS_16_BITS
#    define CHAR_IS_8_BITS
#    define CHAR_IS_SIGNED


#    define SIZE_T_IS_UNSIGNED

     typedef size_t Malloc_size;

#    ifdef SIZE_T_IS_64_BITS
#        define MALLOC_SIZE_IS_64_BITS
#    else 
#        define MALLOC_SIZE_IS_32_BITS
#    endif 

#    define IOCTL_REQUEST_TYPE  unsigned long 

#    define NETWORKING_AVAILABLE


#    define SYSTEM_NAME            "unix"
#    define SYSTEM_NAME_TWO        "mac_osx"

#    define IMPORT                 extern
#    define EXPORT

#    define NON_BLOCKING_READ_FAILURE_ERROR_NUM  EAGAIN

#    define SYS_ERRLIST_TYPE const char* const

#    define mktemp  mkstemp

#    define KJB_HAVE_NATIVE_FFLUSH_NULL

#    define KJB_HAVE_STRERROR

#    define KJB_HAVE_QSORT 1

#endif   /* MAC_OSX */

/* ----------------------------- LINUX_X86_64 ------------------------------ */

#ifdef LINUX_X86_64

#    ifndef LINUX_X86
#        define LINUX_X86
#    endif

#    ifndef LINUX
#       define LINUX
#    endif

#    ifndef LSB_FIRST
#        define LSB_FIRST
#    endif

#    ifndef SYSV_SIGNALS
#         define SYSV_SIGNALS
#    endif

#    ifndef STACK_SIZE
#        define STACK_SIZE (20000000)
#    endif

#    define LONG_IS_64_BITS
#    define INT_IS_32_BITS
#    define SHORT_IS_16_BITS
#    define CHAR_IS_8_BITS
#    define CHAR_IS_SIGNED

#    define PTR_IS_64_BITS

#    define SIZE_T_IS_64_BITS
#    define SIZE_T_IS_UNSIGNED

     typedef size_t Malloc_size;
#    define MALLOC_SIZE_IS_64_BITS

#    define IOCTL_REQUEST_TYPE  unsigned long 

#    define NETWORKING_AVAILABLE

#    define SYSTEM_NAME            "unix"
#    define SYSTEM_NAME_TWO        "linux_X86_64"

#    define IMPORT                 extern
#    define EXPORT

#    define NON_BLOCKING_READ_FAILURE_ERROR_NUM  EAGAIN

#    define SYS_ERRLIST_TYPE const char* const

#    define mktemp  mkstemp

#    define KJB_HAVE_NATIVE_FFLUSH_NULL

#    define KJB_HAVE_STRERROR

#    define KJB_HAVE_QSORT 1

#endif   /* LINUX_X86_64 */

/* ----------------------------- LINUX_X86_32 ------------------------------- */

#ifdef LINUX_X86_32

#    ifndef LINUX
#       define LINUX
#    endif

#    ifndef LINUX_X86
#        define LINUX_X86
#    endif

#    ifndef LSB_FIRST
#        define LSB_FIRST
#    endif

#    ifndef SYSV_SIGNALS
#         define SYSV_SIGNALS
#    endif

#    ifndef STACK_SIZE
#        define STACK_SIZE (10000000)
#    endif

#    define LONG_IS_32_BITS
#    define INT_IS_32_BITS
#    define SHORT_IS_16_BITS
#    define CHAR_IS_8_BITS
#    define CHAR_IS_SIGNED

#    define PTR_IS_32_BITS

#    define SIZE_T_IS_32_BITS
#    define SIZE_T_IS_UNSIGNED

     typedef size_t Malloc_size;
#    define MALLOC_SIZE_IS_32_BITS

#    define IOCTL_REQUEST_TYPE  int
    
#    define NETWORKING_AVAILABLE

#    define SYSTEM_NAME            "unix"
#    define SYSTEM_NAME_TWO        "linux_386"

#    define IMPORT                 extern
#    define EXPORT

#    define NON_BLOCKING_READ_FAILURE_ERROR_NUM  EAGAIN

#    define SYS_ERRLIST_TYPE const char* const

#    define mktemp  mkstemp

#    define KJB_HAVE_NATIVE_FFLUSH_NULL

#    define KJB_HAVE_STRERROR

#    define KJB_HAVE_QSORT 1

#endif   /* LINUX_X86_32 */

/* ------------------------------ SUN5  ------------------------------- */

#ifdef SUN5

/*
//   Valid at SFU. Hopefully typical. Note that STACK_SIZE is only used in
//   expressions which in essence ask "is it on the big size?". So if the
//   stack truly is large, then the exact big number is not important.
*/
#    ifndef STACK_SIZE
#        define STACK_SIZE (2097148 * 1024)
#    endif

#    ifndef MSB_FIRST
#        define MSB_FIRST
#    endif

#    define LONG_IS_32_BITS
#    define INT_IS_32_BITS
#    define SHORT_IS_16_BITS
#    define CHAR_IS_8_BITS
#    define CHAR_IS_SIGNED

#    define PTR_IS_32_BITS

#    define SIZE_T_IS_32_BITS
#    define SIZE_T_IS_UNSIGNED

     typedef size_t Malloc_size;
#    define MALLOC_SIZE_IS_32_BITS

     /* Not checked. */
#    define IOCTL_REQUEST_TYPE  int 
    
#    define NETWORKING_AVAILABLE

#    define SYSTEM_NAME            "unix"
#    define SYSTEM_NAME_TWO        "sun5"

#    define IMPORT                 extern
#    define EXPORT

#    define NON_BLOCKING_READ_FAILURE_ERROR_NUM  EAGAIN

#    define SYS_ERRLIST_TYPE char*

#    define mktemp  mkstemp

#    define KJB_HAVE_NATIVE_FFLUSH_NULL

#    define KJB_HAVE_STRERROR

#    define KJB_HAVE_QSORT 1

#endif   /* SUN5 */
/* ------------------------------------------------------------------------- */


/* ------------------------------   SUN4  ----------------------------------- */
#ifdef SUN4

/*
//   Valid at SFU. Hopefully typical. Note that STACK_SIZE is only used in
//   expressions which in essence ask "is it on the big size?". So if the
//   stack truly is large, then the exact big number is not important.
*/
#    ifndef STACK_SIZE
#        define STACK_SIZE (393216 * 1024)
#    endif

#    ifndef MSB_FIRST
#        define MSB_FIRST
#    endif

#    define LONG_IS_32_BITS
#    define INT_IS_32_BITS
#    define SHORT_IS_16_BITS
#    define CHAR_IS_8_BITS
#    define CHAR_IS_SIGNED

#    define SIZE_T_IS_SIGNED
#    define PTR_IS_32_BITS

#    define SIZE_T_IS_32_BITS

     typedef unsigned int Malloc_size;
#    define MALLOC_SIZE_IS_32_BITS

#    define NETWORKING_AVAILABLE

#    define SYSTEM_NAME     "unix"
#    define SYSTEM_NAME_TWO "sun4"

#    define IMPORT          extern
#    define EXPORT


#    define NON_BLOCKING_READ_FAILURE_ERROR_NUM  EWOULDBLOCK

#    define SYS_ERRLIST_TYPE char*

#    define KJB_HAVE_QSORT 1

#endif   /* SUN4 */
/* ------------------------------------------------------------------------- */


/* ------------------------------   HPUX  ----------------------------------- */
#ifdef HPUX

/*
//   Valid at SFU. Hopefully typical. Note that STACK_SIZE is only used in
//   expressions which in essence ask "is it on the big size?". So if the
//   stack truly is large, then the exact big number is not important.
*/
#    ifndef STACK_SIZE
#        define STACK_SIZE (16384 * 1024)
#    endif

#    ifndef MSB_FIRST
#        define MSB_FIRST
#    endif

#    define LONG_IS_32_BITS
#    define INT_IS_32_BITS
#    define SHORT_IS_16_BITS
#    define CHAR_IS_8_BITS
#    define CHAR_IS_SIGNED

#    define PTR_IS_32_BITS

#    define SIZE_T_IS_32_BITS
#    define SIZE_T_IS_UNSIGNED

     typedef size_t Malloc_size;
#    define MALLOC_SIZE_IS_32_BITS

     /* Not checked. */
#    define IOCTL_REQUEST_TYPE  int
    
#    define NETWORKING_AVAILABLE


#    define SYSTEM_NAME            "unix"
#    define SYSTEM_NAME_TWO        "hpux"

#    define IMPORT                 extern
#    define EXPORT

#    define NON_BLOCKING_READ_FAILURE_ERROR_NUM  EAGAIN   /* CHECK */

#    define SYS_ERRLIST_TYPE char*

#    define KJB_HAVE_NATIVE_FFLUSH_NULL

#    define KJB_HAVE_QSORT 1

#endif   /* HPUX */
/* ------------------------------------------------------------------------- */


/* ------------------------------   SGI  ------------------------------------ */
#ifdef SGI

/*
//   Valid at SFU. Hopefully typical. Note that STACK_SIZE is only used in
//   expressions which in essence ask "is it on the big size?". So if the
//   stack truly is large, then the exact big number is not important.
*/
#    ifndef STACK_SIZE
#        define STACK_SIZE (524288 * 1024)
#    endif

#    ifndef MSB_FIRST
#        define MSB_FIRST
#    endif

#    define LONG_IS_32_BITS
#    define INT_IS_32_BITS
#    define SHORT_IS_16_BITS
#    define CHAR_IS_8_BITS

/* CHECK which one.
#    define CHAR_IS_SIGNED
#    define CHAR_IS_UNSIGNED
*/

#    define PTR_IS_32_BITS

#    define SIZE_T_IS_32_BITS
#    define SIZE_T_IS_UNSIGNED

     typedef size_t Malloc_size;
#    define MALLOC_SIZE_IS_32_BITS

     /* Not checked. */
#    define IOCTL_REQUEST_TYPE  int
    
#    define NETWORKING_AVAILABLE


#    define SYSTEM_NAME            "unix"
#    define SYSTEM_NAME_TWO        "sgi"

#    define IMPORT                 extern
#    define EXPORT

#    define NON_BLOCKING_READ_FAILURE_ERROR_NUM  EWOULDBLOCK

#    define SYS_ERRLIST_TYPE char*

#    define KJB_HAVE_QSORT 1

#endif   /* SGI */
/* ------------------------------------------------------------------------- */


/* ------------------------------   NEXT  ----------------------------------- */
#ifdef NEXT

/*
// Note: Don't define "next", as it is a common variable name in code.
*/
#    ifndef NeXT
#        define NeXT
#    endif

     /* Currently NOT intell hardwar. */
#    ifndef MSB_FIRST
#        define MSB_FIRST
#    endif

/*
//   Actually, it seems that this can be upped quite a bit at SFU with the limit
//   command, but the handling of the limit (with the limit and "limit -h"
//   command) is odd, and I have not yet checked that the limit actually makes
//   a difference. (The qhull code is still bombing unexpectedly.)
*/
#    ifndef STACK_SIZE
#        define STACK_SIZE    (512 * 1024)
#    endif

#    define LONG_IS_32_BITS
#    define INT_IS_32_BITS
#    define SHORT_IS_16_BITS
#    define CHAR_IS_8_BITS

/* CHECK
#    define CHAR_IS_SIGNED
#    define CHAR_IS_UNSIGNED
*/

#    define PTR_IS_32_BITS

#    define SIZE_T_IS_32_BITS
#    define SIZE_T_IS_UNSIGNED

     typedef size_t Malloc_size;
     typedef int pid_t;
#    define MALLOC_SIZE_IS_32_BITS

     /* Not checked. */
#    define IOCTL_REQUEST_TYPE  int
    
#    define NETWORKING_AVAILABLE

#    define SYSTEM_NAME            "unix"
#    define SYSTEM_NAME_TWO        "next"

#    define IMPORT                 extern
#    define EXPORT

#    define NON_BLOCKING_READ_FAILURE_ERROR_NUM  EWOULDBLOCK

#    define SYS_ERRLIST_TYPE char*

#    define KJB_HAVE_QSORT 1

#endif  /* NEXT */
/* ------------------------------------------------------------------------- */


/* ------------------------------   AIX   ----------------------------------- */
#ifdef AIX

     /*
     //   Conservative guess! Must CHECK
     */
#    ifndef STACK_SIZE
#        define STACK_SIZE (10 * 1024 * 1024)
#    endif

#    ifndef MSB_FIRST
#        define MSB_FIRST
#    endif

#    define LONG_IS_32_BITS
#    define INT_IS_32_BITS
#    define SHORT_IS_16_BITS
#    define CHAR_IS_8_BITS

/* CHECK
#    define CHAR_IS_SIGNED
#    define CHAR_IS_UNSIGNED
*/

#    define PTR_IS_32_BITS

#    define SIZE_T_IS_32_BITS
#    define SIZE_T_IS_UNSIGNED

     typedef size_t Malloc_size;
#    define MALLOC_SIZE_IS_32_BITS

     /* Not checked. */
#    define IOCTL_REQUEST_TYPE  int

#    define NETWORKING_AVAILABLE

#    define SYSTEM_NAME            "unix"
#    define SYSTEM_NAME_TWO        "AIX"

#    define IMPORT                 extern
#    define EXPORT

#    define NON_BLOCKING_READ_FAILURE_ERROR_NUM  EAGAIN

#    define SYS_ERRLIST_TYPE char*

#    define KJB_HAVE_QSORT 1

#endif   /* AIX */
/* ------------------------------------------------------------------------- */

/* ------------------------------   DOS_REAL_16  ---------------------------- */
#ifdef DOS_REAL_16

#    ifndef STACK_SIZE
#        define STACK_SIZE (32 * 1024)
#    endif

#    ifndef LSB_FIRST
#        define LSB_FIRST
#    endif

     /*   CHECK: 
      *
      *   We make assumptions so that we can at least compile! 
     */
#    define LONG_IS_32_BITS
#    define INT_IS_16_BITS
#    define SHORT_IS_16_BITS
#    define CHAR_IS_8_BITS

/*   CHECK
*      #    define CHAR_IS_SIGNED
*      #    define CHAR_IS_UNSIGNED
*/

#    define PTR_IS_16_BITS

#    define SIZE_T_IS_16_BITS
#    define SIZE_T_IS_UNSIGNED

     typedef size_t Malloc_size;
#    define MALLOC_SIZE_IS_16_BITS

    
     /* Not checked. */
#    define IOCTL_REQUEST_TYPE  int 
    
     typedef int pid_t;


#    define SYSTEM_NAME            "msdos"
#    define SYSTEM_NAME_TWO        "dos"

#    define IMPORT                 extern
#    define EXPORT

#    define SYS_ERRLIST_TYPE char*

#    define KJB_HAVE_QSORT 1

#endif
/* ------------------------------------------------------------------------- */


/* ----------------------------   DOS_PROTECTED_16  ------------------------- */
#ifdef DOS_PROTECTED_16

#    ifndef STACK_SIZE
#        define STACK_SIZE (32 * 1024)
#    endif

#    ifndef LSB_FIRST
#        define LSB_FIRST
#    endif

#    define LONG_IS_32_BITS
#    define INT_IS_16_BITS

     /*   CHECK: 
      *
      *   We make assumptions so that we can at least compile! 
     */
#    define SHORT_IS_16_BITS
#    define CHAR_IS_8_BITS

/*   CHECK
*      #    define CHAR_IS_SIGNED
*      #    define CHAR_IS_UNSIGNED
*/

#    define PTR_IS_16_BITS
#    define SIZE_T_IS_16_BITS
#    define SIZE_T_IS_UNSIGNED

     typedef size_t Malloc_size;
#    define MALLOC_SIZE_IS_16_BITS

     /* Not checked. */
#    define IOCTL_REQUEST_TYPE  int 
    
     typedef int pid_t;


#    define SYSTEM_NAME            "msdos"
#    define SYSTEM_NAME_TWO        "dos"

#    define IMPORT                 extern
#    define EXPORT

#    define SYS_ERRLIST_TYPE char*

#    define KJB_HAVE_QSORT 1

#endif
/* ------------------------------------------------------------------------- */


/* ----------------------------   DOS_PROTECTED_32  ------------------------- */
#ifdef DOS_PROTECTED_32

/*
//   Probably is larger. Find out! Use regular DOS values for now. MUST CHECK.
*/
#    ifndef STACK_SIZE
#        define STACK_SIZE (32 * 1024)
#    endif

#    ifndef LSB_FIRST
#        define LSB_FIRST
#    endif

     /*   CHECK: 
      *
      *   We make assumptions so that we can at least compile! 
     */

#    define LONG_IS_32_BITS
#    define INT_IS_32_BITS
#    define SHORT_IS_16_BITS

/*   CHECK
*      #    define CHAR_IS_SIGNED
*      #    define CHAR_IS_UNSIGNED
*/

#    define CHAR_IS_8_BITS
#    define PTR_IS_32_BITS

#    define SIZE_T_IS_32_BITS
#    define SIZE_T_IS_UNSIGNED

     typedef size_t Malloc_size;
#    define MALLOC_SIZE_IS_16_BITS

     /* Not checked. */
#    define IOCTL_REQUEST_TYPE  int 
    
     typedef int pid_t;

#    define SYSTEM_NAME            "msdos"
#    define SYSTEM_NAME_TWO        "dos"

#    define IMPORT                 extern
#    define EXPORT

#    define SYS_ERRLIST_TYPE char*

#    define KJB_HAVE_QSORT 1

#endif
/* ------------------------------------------------------------------------- */


/* ------------------------------   WIN_16  --------------------------------- */
#ifdef WIN16

#    ifndef STACK_SIZE
#        define STACK_SIZE (32 * 1024)
#    endif

#    ifndef LSB_FIRST
#        define LSB_FIRST
#    endif

#    define LONG_IS_32_BITS
#    define INT_IS_16_BITS

     /*   CHECK: 
      *
      *   We make assumptions so that we can at least compile! 
     */
#    define SHORT_IS_16_BITS
#    define CHAR_IS_8_BITS

/*   CHECK
*      #    define CHAR_IS_SIGNED
*      #    define CHAR_IS_UNSIGNED
*/

#    define PTR_IS_16_BITS
#    define SIZE_T_IS_16_BITS
#    define SIZE_T_IS_UNSIGNED

     typedef size_t Malloc_size;
#    define MALLOC_SIZE_IS_16_BITS

     /* Not checked. */
#    define IOCTL_REQUEST_TYPE  int 
    
     typedef int pid_t;

#    define SYSTEM_NAME            "windows-16"
#    define SYSTEM_NAME_TWO        "win16"

#    define IMPORT                 extern
#    define EXPORT

#    define SYS_ERRLIST_TYPE char*

#    define KJB_HAVE_QSORT 1

#endif
/* ------------------------------------------------------------------------- */


/* --------------------------------   WIN_32  ------------------------------ */
#ifdef WIN32

/*
//   Probably is larger. Find out! Use regular DOS values for now. MUST CHECK.
*/
#    ifndef STACK_SIZE
#        define STACK_SIZE (32 * 1024)
#    endif

#    ifndef LSB_FIRST
#        define LSB_FIRST
#    endif

     /*   CHECK: 
      *
      *   We make assumptions so that we can at least compile! 
     */
#    define LONG_IS_32_BITS
#    define INT_IS_32_BITS
#    define SHORT_IS_16_BITS
#    define CHAR_IS_8_BITS

/*   CHECK
*      #    define CHAR_IS_SIGNED
*      #    define CHAR_IS_UNSIGNED
*/
#    define PTR_IS_32_BITS
#    define SIZE_T_IS_32_BITS
#    define SIZE_T_IS_UNSIGNED

     typedef size_t Malloc_size;
#    define MALLOC_SIZE_IS_16_BITS

     /* Not checked. */
#    define IOCTL_REQUEST_TYPE  int 
    
     typedef int pid_t;


#    define SYSTEM_NAME            "windows-32"
#    define SYSTEM_NAME_TWO        "win32"

#    define IMPORT                 extern
#    define EXPORT

#    define SYS_ERRLIST_TYPE char*

#    define KJB_HAVE_QSORT 1

#endif
/* ------------------------------------------------------------------------- */


/* --------------------------------   VMS   -------------------------------- */
#ifdef VMS

/*
//   Conservative guess! Must CHECK
*/
#    ifndef STACK_SIZE
#        define STACK_SIZE        (10 * 512 * 1024)
#    endif

#    ifndef MSB_FIRST   /* I think, CHECK */
#        define MSB_FIRST
#    endif

#    define LONG_IS_32_BITS
#    define INT_IS_32_BITS
#    define SHORT_IS_16_BITS
#    define CHAR_IS_8_BITS

#    define PTR_IS_32_BITS

#    define SIZE_T_IS_32_BITS
#    define SIZE_T_IS_UNSIGNED

     /* CHECK */
     typedef size_t Malloc_size;
#    define MALLOC_SIZE_IS_16_BITS

     /* Not checked. */
#    define IOCTL_REQUEST_TYPE  int 
    
/*   CHECK which one.
*      #    define CHAR_IS_SIGNED
*      #    define CHAR_IS_UNSIGNED
*/

/*   CHECK (I think we have work to do before can define this).
*      #    define NETWORKING_AVAILABLE
*/


#    define SYSTEM_NAME               "vax"
#    define SYSTEM_NAME_TWO           "vms"

#    define IMPORT   globalref
#    define EXPORT   globaldef

#    define SYS_ERRLIST_TYPE char*

#    define KJB_HAVE_QSORT 1

#endif                         /*   VMS   */
/* ------------------------------------------------------------------------- */


/* -------------------------------------------------------------------------- */
/*
              --------------------------------------------------
              | Operating System general-class related defines |
              --------------------------------------------------
                                     ||
                                    \||/
                                     \/
*/

/* ------------------------------  UNIX  -----------------------------------  */

#ifdef UNIX

#    define DIR_STR     "/"
#    define DIR_CHAR    '/'
#    define HOME_STR    "~"
#    define HOME_DIR    "~"
#    define PARENT_DIR  ".."
#    define TEMP_DIR    "/tmp"

#    define MY_GROUP         (getpgrp( ))
#    define MY_PID           (getpid( ))

#    define SET_GROUP(x)     (setpgid(0, x ))

#    ifndef EXIT_FAILURE
#        define EXIT_FAILURE           1
#    endif

#    ifndef EXIT_SUCCESS
#        define EXIT_SUCCESS           0
#    endif

#endif     /* UNIX */
/* ---------------------------------------------------------------------- */

#ifdef MS_OS                  /* MS_OS */

#    define DIR_STR      "\\"
#    define DIR_CHAR     '\\'
#    define HOME_DIR     "C:"
#    define PARENT_DIR   ".."

#ifndef HOME_STR  /* Presumably, this will be quite compile dependent, often
                     being a network drive?
                  */
#    define HOME_STR     "U:"
#endif

#    define TEMP_DIR     "C:\\tmp\\"

/*
// May need to be fixed in the case of NT
*/
#    define MY_PID              1
#    define MY_GROUP            1

#    ifndef EXIT_FAILURE
#        define EXIT_FAILURE    1
#    endif

#    ifndef EXIT_SUCCESS
#        define EXIT_SUCCESS    0
#    endif

#    ifdef __BORLANDC__
#        define sys_nerr _sys_nerr
#        define sys_errlist _sys_errlist
#    endif

#endif   /* MS_OS */
/* ------------------------------------------------------------------------- */

#ifdef VMS                        /*   VMS  */

/*
// FIX these "defaults"
*/
#    define MY_PID              1
#    define MY_GROUP            1

/* CHECK (I forget what it is!) */
*    #    define DIR_STR     "."
*    #    define DIR_CHAR    '.'
*    #    define HOME_DIR
*    #    define HOME_STR
*    #    define TEMP_DIR
*/

#    ifndef EXIT_FAILURE
#        define EXIT_FAILURE           4
#    endif

#    ifndef EXIT_SUCCESS
#        define EXIT_SUCCESS           1
#    endif

#endif   /*   VMS  */
/* -------------------------------------------------------------------------- */


/* -------------------------------------------------------------------------- */
/*
                 -----------------------------------------
                 | Standard/Non-standard related defines |
                 -----------------------------------------
                                   ||
                                  \||/
                                   \/

  Setup stuff so we can deal with both ANSI and non-ANSI C. (THe latter to be
  phased out eventually). The general categories such as FUNCTION_PROTOTYPE_OK
  have been sorted out in l_sys_sys.h, since we need to do that before
  including l_sys_std.h.

*/


#if 0 /* was ifdef HOW_IT_WAS_AUGUST_2002 */

#ifdef FUNCTION_PROTOTYPE_OK
#    define PROTO(x) x
#else
#    define PROTO(x) ()
#endif

#endif


#ifndef   __DATE__
#define   __DATE__         ""
#endif

#ifndef   __TIME__
#define   __TIME__         ""
#endif

#ifndef   __FILE__
#define   __FILE__         "<unknown>"
#endif

#ifndef   __LINE__
#define   __LINE__          (-1)
#endif

#ifndef offsetof
#define offsetof(s, m)  (size_t)(&(((s *)0)->m))
#endif

/* -------------------------------------------------------------------------- */
/*
                            -----------------
                            | Integer types |
                            -----------------
                                   ||
                                  \||/
                                   \/
*/


/* ----------------------------  64 bit longs     --------------------------- */

#ifdef LONG_IS_64_BITS
#    ifndef LONG_MIN
#        define LONG_MIN   (-9223372036854775808L)
#    endif

#    ifndef LONG_MAX
#        define LONG_MAX   9223372036854775807L
#    endif

#    ifndef ULONG_MAX
#        define ULONG_MAX  18446744073709551615
#    endif
#endif

/* ----------------------------  32 bit longs     --------------------------- */

#ifdef LONG_IS_32_BITS
#    ifndef LONG_MIN
#        define LONG_MIN   (-2147483648L)
#    endif

#    ifndef LONG_MAX
#        define LONG_MAX   2147483647L
#    endif

#    ifndef ULONG_MAX
#        define ULONG_MAX  4294967295UL
#    endif
#endif

/* ----------------------------  32 bit integers  --------------------------- */

#ifdef INT_IS_32_BITS
#    ifndef INT_MIN
#        define INT_MIN    (-2147483648)
#    endif

#    ifndef INT_MAX
#        define INT_MAX    2147483647
#    endif

#    ifndef UINT_MAX
#        define UINT_MAX   4294967295U
#    endif
#endif

/* ----------------------------  16 bit integers  --------------------------- */

#ifdef INT_IS_16_BITS
#    ifndef INT_MIN
#        define INT_MIN (-32768)
#    endif

#    ifndef INT_MAX
#        define INT_MAX  32767
#    endif

#    ifndef UINT_MAX
#        define UINT_MAX 65535
#    endif
#endif


/* ----------------------------  16 bit shorts    --------------------------- */

#ifdef SHORT_IS_16_BITS
#    ifndef SHRT_MIN
#        define SHRT_MIN   (-32768)
#    endif

#    ifndef SHRT_MAX
#        define SHRT_MAX   32767
#    endif

#    ifndef USHRT_MAX
#        define USHRT_MAX  65535
#    endif
#endif

/* ----------------------------  8 bit characters   ------------------------- */

#ifdef CHAR_IS_8_BITS
#    ifndef CHAR_BIT
#        define CHAR_BIT   8
#    endif

#    ifndef SCHAR_MIN
#        define SCHAR_MIN  (-128)
#    endif

#    ifndef SCHAR_MAX
#        define SCHAR_MAX  127
#    endif

#    ifndef UCHAR_MAX
#        define UCHAR_MAX  255
#    endif
#endif

/* ----------------------------  Signed characters  ------------------------- */

#ifdef CHAR_IS_SIGNED
#    ifndef CHAR_MIN
#        define CHAR_MIN   SCHAR_MIN
#    endif

#    ifndef CHAR_MAX
#        define CHAR_MAX   SCHAR_MAX
#    endif
#endif

/* ----------------------------- Unsigned characters  ----------------------- */

#ifdef CHAR_IS_UNSIGNED
#    ifndef CHAR_MIN
#        define CHAR_MIN   0
#    endif

#    ifndef CHAR_MAX
#        define CHAR_MAX   UCHAR_MAX
#    endif
#endif

/* ----------------------  Supplementary integer types  --------------------- */

#ifdef LONG_IS_64_BITS   
#   define INT64_IS_LONG
#   define HAVE_64_BIT_INT
#else
#ifdef INT_IS_64_BITS   /* Pretty rare. */
#   define INT64_IS_INT
#   define HAVE_64_BIT_INT
#endif 
#endif 

#ifdef _UINT64_T
    typedef uint64_t kjb_uint64;
    #ifndef HAVE_64_BIT_INT
        #define HAVE_64_BIT_INT
    #endif 
#else 
#ifdef LONG_IS_64_BITS   
    typedef unsigned long kjb_uint64;
#else
#ifdef INT_IS_64_BITS   /* Pretty rare. */
    typedef unsigned int kjb_uint64;
#endif
#endif
#endif

#ifdef _INT64_T
    typedef int64_t kjb_int64;
    #ifndef HAVE_64_BIT_INT
        #define HAVE_64_BIT_INT
    #endif 
#else 
#ifdef LONG_IS_64_BITS   
    typedef long kjb_int64;
#else
#ifdef INT_IS_64_BITS   /* Pretty rare. */
    typedef int kjb_int64;
#endif
#endif
#endif


#ifdef LONG_IS_32_BITS
#   define INT32_IS_LONG
#else
#ifdef INT_IS_32_BITS
#   define INT32_IS_INT
#endif
#endif


#ifdef _UINT32_T
    typedef uint32_t kjb_uint32;
    #ifndef HAVE_32_BIT_INT
        #define HAVE_32_BIT_INT
    #endif 
#else  
#ifdef LONG_IS_32_BITS
    typedef unsigned long kjb_uint32;
#else
#ifdef INT_IS_32_BITS
    typedef unsigned int kjb_uint32;
#endif
#endif
#endif


#ifdef _INT32_T
    typedef int32_t kjb_int32;
    #ifndef HAVE_32_BIT_INT
        #define HAVE_32_BIT_INT
    #endif 
#else  
#ifdef LONG_IS_32_BITS
    typedef long kjb_int32;
#else
#ifdef INT_IS_32_BITS
    typedef int kjb_int32;
#endif
#endif
#endif

#ifdef SHORT_IS_16_BITS
#   define INT16_IS_SHORT
#   define HAVE_16_BIT_INT
#else
#ifdef INT_IS_16_BITS
#   define INT16_IS_INT
#   define HAVE_16_BIT_INT
#endif
#endif

#ifdef _UINT16_T
    typedef uint16_t kjb_uint16;
#   ifndef HAVE_16_BIT_INT
#       define HAVE_16_BIT_INT
#    endif 
#else  
#ifdef LONG_IS_16_BITS   /* I doubt this ever happens. */
    typedef unsigned long kjb_uint16;
#else
#ifdef INT_IS_16_BITS
    typedef unsigned int kjb_uint16;
#else 
#ifdef SHORT_IS_16_BITS
    typedef unsigned short kjb_uint16;
#endif
#endif
#endif
#endif


#ifdef _INT16_T
    typedef int16_t kjb_int16;
    #ifndef HAVE_16_BIT_INT
        #define HAVE_16_BIT_INT
    #endif 
#else  
#ifdef LONG_IS_16_BITS   /* I doubt this ever happens. */
    typedef long kjb_int16;
#else
#ifdef INT_IS_16_BITS
    typedef int kjb_int16;
#else 
#ifdef SHORT_IS_16_BITS
    typedef short kjb_int16;
#endif
#endif
#endif
#endif

typedef signed char kjb_int8;
typedef unsigned char kjb_uint8;

/* --------------------------------------------------- */

/* Only deal with numbers this big on 64 bit machines. */
#ifdef LONG_IS_64_BITS  
#    ifndef INT64_MAX
#        define INT64_MAX    (9223372036854775807)
#    endif

#    ifndef INT64_MIN
#        define INT64_MIN   (-9223372036854775807-1) 
#    endif

#    ifndef UINT64_MAX
#        define UINT64_MAX  (18446744073709551615)
#    endif
#endif

/* ------------------------------------------------- */

#ifndef INT32_MAX
#    define INT32_MAX   (2147483647)
#endif

#ifndef INT32_MIN
#    define INT32_MIN  (-2147483647-1)
#endif

#ifndef UINT32_MAX
/* Was with "U" at the end, but none of the others have it. Try without. */
/* #    define UINT32_MAX  (4294967295U)   */
#    define UINT32_MAX  (4294967295)
#endif

/* ---------------------------------- */

#ifndef INT16_MAX
#    define INT16_MAX    (32767)
#endif

#ifndef INT16_MIN
#    define INT16_MIN   (-32767-1)
#endif

#ifndef UINT16_MAX
#    define UINT16_MAX   (65535)
#endif

/* ---------------------------------- */

#ifndef INT8_MAX
#    define INT8_MAX     (127)
#endif

#ifndef INT8_MIN
#    define INT8_MIN    (-128)
#endif

#ifndef UINT8_MAX
#    define UINT8_MAX    (255)
#endif


/* -------------------------------------------------------------------------- */
/*
                         ------------------------
                         |  Memory Alloaction   |
                         ------------------------
                                   ||
                                  \||/
                                   \/
*/

/*
#define TRACK_MEMORY_ALLOCATION
*/

/*
// Update c2man comment blocks if conditions under which TRACK_MEMORY_ALLOCATION
// gets defined change.
*/
#ifdef TEST
#    ifdef UNIX
#        ifndef TRACK_MEMORY_ALLOCATION
#            define TRACK_MEMORY_ALLOCATION
#        endif
#    endif
#endif


#ifdef EMULATE_MS_16_BIT_OS
#    define MAX_MALLOC_SIZE  (0xffff)
#endif

/* -------------------------------------------------------------------------- */
/*
                         ------------------------
                         | Buffer and IO sizes  |
                         ------------------------
                                   ||
                                  \||/
                                   \/
*/


/* -------------------------------------------------------------------------- */
/*
                      ----------------------------
                      | Compiler related defines |
                      ----------------------------
                                   ||
                                  \||/
                                   \/
*/

#ifdef __BORLANDC__
    typedef unsigned int uint_t;
#endif

/* -------------------------------------------------------------------------- */
/*
                ---------------------------------------
                | Additional system dependent symbols |
                ---------------------------------------
                                   ||
                                  \||/
                                   \/
*/

/* -------------------------------------------------------------------------- */


#ifdef __cplusplus
#ifdef COMPILING_CPLUSPLUS_SOURCE
}
#endif
}
#endif

#endif   /* Include this file */

