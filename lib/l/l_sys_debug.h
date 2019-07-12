
/* $Id: l_sys_debug.h 21712 2017-08-20 18:21:41Z kobus $ */

 
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

#ifndef L_SYS_DBUG_INCLUDED
#define L_SYS_DBUG_INCLUDED


#include "l/l_def.h"

#include "l/l_sys_lib.h"
#include "l/l_sys_io.h"

#ifdef __C2MAN__
#    include "l/l_int_vector.h"
#    include "l/l_int_matrix.h"
#endif



/* -------------------------------------------------------------------------- */

/* =============================================================================
 *                             dbw
 *
 * (MACRO) Prints file and line number
 *
 * This macro prints the file and line number, on standard error. Its behaviour
 * is a function of the debugging level, which can be set through the option
 * "debug" if the KJB library options are being made available to the user. The
 * macro is available for both development and production code, but since the
 * default debug level is different in the two cases, the behaviour is
 * different. The default level for development code is 2, whereas for
 * production code it is 0.
 *
 * If the debug level is 2 (or more), then the output is printed to standard
 * error using regular KJB library output routines.
 *
 * If the debug level is 1, then the output is printed to standard error using
 * standard C library routines instead of regular KJB library routines. This
 * facility is provided for KJB library I/O routines themselves.
 *
 * If the debug level is 0 (default for production code), then the output is
 * disabled.
 *
 * Index: debugging
 *
 * -----------------------------------------------------------------------------
*/
#ifdef __C2MAN__
    void dbw(void);
#endif

#ifndef __C2MAN__  /* Just doing "else" confuses ctags for the first such line.
                      I have no idea what the problem is. */
#define  dbw()                                                                 \
    if (    (kjb_debug_level == 1)                                             \
         || ((kjb_cleanup_started) && (kjb_debug_level > 0))                   \
       )                                                                       \
    {                                                                          \
        fprintf(stderr,                                                        \
                "(no-kjb) Process %d (fork depth %d) at line %d of %s (ignore line for regression testing)\n", \
                MY_PID, kjb_fork_depth, __LINE__, __FILE__);                    \
    }                                                                          \
    else if (kjb_debug_level >= 2)                                             \
    {                                                                          \
        p_stderr("Process %d (fork depth %d) at line %d of %s (ignore line for regression testing)\n", \
                 MY_PID, kjb_fork_depth, __LINE__, __FILE__);                   \
    }

#endif

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                             dbr
 *
 * (MACRO) Debug printing status code
 *
 * This macro prints a tag corresponding to the KJB library status meaning of
 * an integer, along with the file and line number, on standard error. Its
 * behaviour is a function of the debugging level, which can be set through the
 * option "debug" if the KJB library options are being made available to the
 * user. The macro is available for both development and production code, but
 * since the default debug level is different in the two cases, the behaviour is
 * different. The default level for development code is 2, whereas for
 * production code it is 0.
 *
 * If the debug level is 2 (or more), then the output is printed to standard
 * error using regular KJB library output routines.
 *
 * If the debug level is 1, then the output is printed to standard error using
 * standard C library routines instead of regular KJB library routines. This
 * facility is provided for KJB library I/O routines themselves.
 *
 * If the debug level is 0 (default for production code), then the output is
 * disabled.
 *
 * Index: debugging
 *
 * -----------------------------------------------------------------------------
*/

#ifdef __C2MAN__
    void dbr(int i);
#else

#define dbr(X)                                                                 \
    if (    (kjb_debug_level == 1)                                             \
         || ((kjb_cleanup_started) && (kjb_debug_level > 0))                   \
       )                                                                       \
    {                                                                          \
        fprintf(stderr,                                                        \
                "(no-kjb) dbr: Status is %ld on source line %d of %s\n",       \
                (long)(X), __LINE__, __FILE__);                                \
    }                                                                          \
    else if (kjb_debug_level >= 2)                                             \
    {                                                                          \
        char buff[ 100 ];                                                      \
                                                                               \
        kjb_sprintf(buff, sizeof(buff), "%R", X);                              \
        kjb_fprintf(stderr, "dbr: Status is %s on source line %d of %s\n",     \
                    buff, __LINE__, __FILE__);                                 \
    }

#endif    /*   #ifdef __C2MAN__ */

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                             dbp
 *
 * (MACRO) Debug printing of a string
 *
 * This macro prints a string on standard error. Unlike the other "db" macros,
 * dbp does NOT output the file or the line number. It is normally used to tidy
 * up debugging output.
 *
 * The behaviour of dbp() is a function of the debugging level, which can be set
 * through the option "debug" if the KJB library options are being made
 * available to the user. The macro is available for both development and
 * production code, but since the default debug level is different in the two
 * cases, the behaviour is different. The default level for development code is
 * 2, whereas for production code it is 0.
 *
 * If the debug level is 2 (or more), then the output is printed to standard
 * error using regular KJB library output routines.
 *
 * If the debug level is 1, then the output is printed to standard error using
 * standard C library routines instead of regular KJB library routines. This
 * facility is provided for KJB library I/O routines themselves.
 *
 * If the debug level is 0 (default for production code), then the output is
 * disabled.
 *
 * Index: debugging
 *
 * -----------------------------------------------------------------------------
*/

#ifdef __C2MAN__
    void dbp(char* text_to_print);
#else

#define dbp(X) \
    if (    (kjb_debug_level == 1)                                             \
         || ((kjb_cleanup_started) && (kjb_debug_level > 0))                   \
       )                                                                       \
    {                                                                          \
        fprintf(stderr, "%s\n", (X));                                          \
    }                                                                          \
    else if (kjb_debug_level >= 2)                                             \
    {                                                                          \
        kjb_fprintf(stderr, "%s\n", (X));                                      \
    }

#endif    /*   #ifdef __C2MAN__ */

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                             dbx
 *
 * (MACRO) Debug printing of integer in hex
 *
 * This macro prints an integer (or long) in hex along with the variable name,
 * file, and line number on standard error. Its behaviour is a function of the
 * debugging level, which can be set through the option "debug" if the KJB
 * library options are being made available to the user. The macro is available
 * for both development and production code, but since the default debug level
 * is different in the two cases, the behaviour is different. The default level
 * for development code is 2, whereas for production code it is 0.
 *
 * If the debug level is 2 (or more), then the output is printed to standard
 * error using regular KJB library output routines.
 *
 * If the debug level is 1, then the output is printed to standard error using
 * standard C library routines instead of regular KJB library routines. This
 * facility is provided for KJB library I/O routines themselves.
 *
 * If the debug level is 0 (default for production code), then the output is
 * disabled.
 *
 * Note :
 *    The argument is cast to long, and thus integer and long will give similar
 *    results.
 *
 * Index: debugging
 *
 * -----------------------------------------------------------------------------
*/

#ifdef __C2MAN__
    void dbx(int i);
#else

#ifdef HASH_MARK_SUBSTITUTION_OK

#define dbx(X)                                                                 \
    if (    (kjb_debug_level == 1)                                             \
         || ((kjb_cleanup_started) && (kjb_debug_level > 0))                   \
       )                                                                       \
    {                                                                          \
        fprintf(stderr, "(no-kjb) dbx: "#X" is %lx on source line %d of %s\n", \
                (unsigned long)(X), __LINE__, __FILE__);                       \
        fflush(stderr);                                                        \
    }                                                                          \
    else if (kjb_debug_level >= 2)                                             \
    {                                                                          \
        kjb_fprintf(stderr, "dbx: "#X" is %lx on source line %d of %s\n",      \
                    (unsigned long)(X), __LINE__, __FILE__);                   \
        kjb_fflush(stderr);                                                    \
    }

#else    /*   #ifdef HASH_MARK_SUBSTITUTION_OK ....     */

#define dbx(X)                                                                 \
    if (    (kjb_debug_level == 1)                                             \
         || ((kjb_cleanup_started) && (kjb_debug_level > 0))                   \
       )                                                                       \
    {                                                                          \
        fprintf(stderr, "(no-kjb) dbx: X is %lx on source line %d of %s\n",    \
                (unsigned long)(X), __LINE__, __FILE__);                       \
        fflush(stderr);                                                        \
    }                                                                          \
    else if (kjb_debug_level >= 2)                                             \
    {                                                                          \
        kjb_fprintf(stderr, "dbx: X is %lx on source line %d of %s\n",         \
                    (unsigned long)(X), __LINE__, __FILE__);                   \
        kjb_fflush(stderr);                                                    \
    }

#endif    /*   #ifdef HASH_MARK_SUBSTITUTION_OK ... #else  .....   */

#endif    /*   #ifdef __C2MAN__ */

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                             dbo
 *
 * (MACRO) Debug printing of integer in octal
 *
 * This macro prints an integer (or long) in octal along with the variable name,
 * file, and line number on standard error. Its behaviour is a function of the
 * debugging level, which can be set through the option "debug" if the KJB
 * library options are being made available to the user. The macro is available
 * for both development and production code, but since the default debug level
 * is different in the two cases, the behaviour is different. The default level
 * for development code is 2, whereas for production code it is 0.
 *
 * If the debug level is 2 (or more), then the output is printed to standard
 * error using regular KJB library output routines.
 *
 * If the debug level is 1, then the output is printed to standard error using
 * standard C library routines instead of regular KJB library routines. This
 * facility is provided for KJB library I/O routines themselves.
 *
 * If the debug level is 0 (default for production code), then the output is
 * disabled.
 *
 * Note :
 *    The argument is cast to long, and thus integer and long will give similar
 *    results.
 *
 * Index: debugging
 *
 * -----------------------------------------------------------------------------
*/

#ifdef __C2MAN__
    void dbo(int i);
#else

#ifdef HASH_MARK_SUBSTITUTION_OK

#define dbo(X)                                                                 \
    if (    (kjb_debug_level == 1)                                             \
         || ((kjb_cleanup_started) && (kjb_debug_level > 0))                   \
       )                                                                       \
    {                                                                          \
        fprintf(stderr, "(no-kjb) dbo: "#X" is %lo on source line %d of %s\n",          \
                (long)(X), __LINE__, __FILE__);                                \
    }                                                                          \
    else if (kjb_debug_level >= 2)                                             \
    {                                                                          \
        kjb_fprintf(stderr, "dbo: "#X" is %lo on source line %d of %s\n",      \
                    (long)(X), __LINE__, __FILE__);                            \
    }

#else    /*   #ifdef HASH_MARK_SUBSTITUTION_OK ....     */

#define dbo(X)                                                                 \
    if (    (kjb_debug_level == 1)                                             \
         || ((kjb_cleanup_started) && (kjb_debug_level > 0))                   \
       )                                                                       \
    {                                                                          \
        fprintf(stderr, "(no-kjb) dbo: X is %lo on source line %d of %s\n",             \
                (long)(X), __LINE__, __FILE__);                                \
    }                                                                          \
    else if (kjb_debug_level >= 2)                                             \
    {                                                                          \
        kjb_fprintf(stderr, "dbo: X is %lo on source line %d of %s\n",         \
                    (long)(X), __LINE__, __FILE__);                            \
    }

#endif    /*   #ifdef HASH_MARK_SUBSTITUTION_OK ... #else  .....   */

#endif    /*   #ifdef __C2MAN__ */

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                             dbi
 *
 * (MACRO) Debug printing of integer
 *
 * This macro prints an integer (or long) along with the variable name, file,
 * and line number, on standard error. Its behaviour is a function of the
 * debugging level, which can be set through the option "debug" if the KJB
 * library options are being made available to the user. The macro is available
 * for both development and production code, but since the default debug level
 * is different in the two cases, the behaviour is different. The default level
 * for development code is 2, whereas for production code it is 0.
 *
 * If the debug level is 2 (or more), then the output is printed to standard
 * error using regular KJB library output routines.
 *
 * If the debug level is 1, then the output is printed to standard error using
 * standard C library routines instead of regular KJB library routines. This
 * facility is provided for KJB library I/O routines themselves.
 *
 * If the debug level is 0 (default for production code), then the output is
 * disabled.
 *
 * Note :
 *    The argument is cast to long, and thus integer and long will give similar
 *    results.
 *
 * Index: debugging
 *
 * -----------------------------------------------------------------------------
*/

#ifdef __C2MAN__
    void dbi(int i);
#else

#ifdef HASH_MARK_SUBSTITUTION_OK

#define dbi(X)                                                                 \
    if (    (kjb_debug_level == 1)                                             \
         || ((kjb_cleanup_started) && (kjb_debug_level > 0))                   \
       )                                                                       \
    {                                                                          \
        fprintf(stderr,                                                        \
                "(no-kjb) dbi: "#X" is %ld on source line %d of %s\n",         \
                (long)(X), __LINE__, __FILE__);                                \
    }                                                                          \
    else if (kjb_debug_level >= 2)                                             \
    {                                                                          \
        kjb_fprintf(stderr, "dbi: "#X" is %ld on source line %d of %s\n",      \
                    (long)(X), __LINE__, __FILE__);                            \
    }

#else    /*   #ifdef HASH_MARK_SUBSTITUTION_OK ....     */

#define dbi(X)                                                                 \
    if (    (kjb_debug_level == 1)                                             \
         || ((kjb_cleanup_started) && (kjb_debug_level > 0))                   \
       )                                                                       \
    {                                                                          \
        fprintf(stderr, "(no-kjb) dbi: X is %ld on source line %d of %s\n",    \
                (long)(X), __LINE__, __FILE__);                                \
    }                                                                          \
    else if (kjb_debug_level >= 2)                                             \
    {                                                                          \
        kjb_fprintf(stderr, "dbi: X is %ld on source line %d of %s\n",         \
                    (long)(X),  __LINE__, __FILE__);                           \
    }

#endif    /*   #ifdef HASH_MARK_SUBSTITUTION_OK ... #else  .....   */

#endif    /*   #ifdef __C2MAN__ */

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                             dbj
 *
 * (MACRO) Debug printing of integer with commas
 *
 * This is an alternative to dbi where the integer format includes commas when
 * available, which is normally the case if the debug level is 2 or more, and
 * not the case if debug level is 0 (nothing is printed) or 1 (comma format is
 * not available).
 *
 * Note :
 *    The argument is cast to long, and thus integer and long will give similar
 *    results.
 *
 * Index: debugging
 *
 * -----------------------------------------------------------------------------
*/

#ifdef __C2MAN__
    void dbj(int i);
#else

#ifdef HASH_MARK_SUBSTITUTION_OK

#define dbj(X)                                                                 \
    if (    (kjb_debug_level == 1)                                             \
         || ((kjb_cleanup_started) && (kjb_debug_level > 0))                   \
       )                                                                       \
    {                                                                          \
        fprintf(stderr,                                                        \
                "(no-kjb) dbj: "#X" is %ld on source line %d of %s\n",         \
                (long)(X), __LINE__, __FILE__);                                \
    }                                                                          \
    else if (kjb_debug_level >= 2)                                             \
    {                                                                          \
        kjb_fprintf(stderr, "dbj: "#X" is %,ld on source line %d of %s\n",     \
                    (long)(X), __LINE__, __FILE__);                            \
    }

#else    /*   #ifdef HASH_MARK_SUBSTITUTION_OK ....     */

#define dbj(X)                                                                 \
    if (    (kjb_debug_level == 1)                                             \
         || ((kjb_cleanup_started) && (kjb_debug_level > 0))                   \
       )                                                                       \
    {                                                                          \
        fprintf(stderr, "(no-kjb) dbj: X is %ld on source line %d of %s\n",    \
                (long)(X), __LINE__, __FILE__);                                \
    }                                                                          \
    else if (kjb_debug_level >= 2)                                             \
    {                                                                          \
        kjb_fprintf(stderr, "dbj: X is %,ld on source line %d of %s\n",        \
                    (long)(X),  __LINE__, __FILE__);                           \
    }

#endif    /*   #ifdef HASH_MARK_SUBSTITUTION_OK ... #else  .....   */

#endif    /*   #ifdef __C2MAN__ */

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                             dbu
 *
 * (MACRO) Debug printing of unsigned integer with commas
 *
 * This is a similar to dbj except that it is assumed that the integer to be
 * printed is unsignned.
 *
 * Note :
 *    The argument is cast to long, and thus integer and long will give similar
 *    results.
 *
 * Index: debugging
 *
 * -----------------------------------------------------------------------------
*/

#ifdef __C2MAN__
    void dbu(unsigned int i);
#else

#ifdef HASH_MARK_SUBSTITUTION_OK

#define dbu(X)                                                                 \
    if (    (kjb_debug_level == 1)                                             \
         || ((kjb_cleanup_started) && (kjb_debug_level > 0))                   \
       )                                                                       \
    {                                                                          \
        fprintf(stderr,                                                        \
                "(no-kjb) dbu: "#X" is %lu on source line %d of %s\n",         \
                (unsigned long)(X),                                            \
               __LINE__, __FILE__);                                            \
    }                                                                          \
    else if (kjb_debug_level >= 2)                                             \
    {                                                                          \
        kjb_fprintf(stderr, "dbu: "#X" is %,lu on source line %d of %s\n",     \
                    (unsigned long)(X), __LINE__, __FILE__);                   \
    }

#else    /*   #ifdef HASH_MARK_SUBSTITUTION_OK ....     */

#define dbu(X)                                                                 \
    if (    (kjb_debug_level == 1)                                             \
         || ((kjb_cleanup_started) && (kjb_debug_level > 0))                   \
       )                                                                       \
    {                                                                          \
        fprintf(stderr, "(no-kjb) dbu: X is %lu on source line %d of %s\n",    \
                (unsigned long)(X), __LINE__, __FILE__);                       \
    }                                                                          \
    else if (kjb_debug_level >= 2)                                             \
    {                                                                          \
        kjb_fprintf(stderr, "dbu: X is %,lu on source line %d of %s\n",        \
                    (unsigned long)(X),  __LINE__, __FILE__);                  \
    }

#endif    /*   #ifdef HASH_MARK_SUBSTITUTION_OK ... #else  .....   */

#endif    /*   #ifdef __C2MAN__ */

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                             dbf
 *
 * (MACRO) Debug printing of floating point
 *
 * This macro prints a floating point number in fixed format, along with the
 * variable name, file, and line number, on standard error. Its behaviour is a
 * function of the debugging level, which can be set through the option "debug"
 * if the KJB library options are being made available to the user. The macro is
 * available for both development and production code, but since the default
 * debug level is different in the two cases, the behaviour is different. The
 * default level for development code is 2, whereas for production code it is 0.
 *
 * If the debug level is 2 (or more), then the output is printed to standard
 * error using regular KJB library output routines.
 *
 * If the debug level is 1, then the output is printed to standard error using
 * standard C library routines instead of regular KJB library routines. This
 * facility is provided for KJB library I/O routines themselves.
 *
 * If the debug level is 0 (default for production code), then the output is
 * disabled.
 *
 * Note :
 *    The argument is cast to double, and thus float, double, or double
 *    arguments are all OK.
 *
 * Index: debugging
 *
 * -----------------------------------------------------------------------------
*/

#ifdef __C2MAN__
    void dbf(double float_num);
#else

#ifdef HASH_MARK_SUBSTITUTION_OK

#define dbf(X)                                                                 \
    if (    (kjb_debug_level == 1)                                             \
         || ((kjb_cleanup_started) && (kjb_debug_level > 0))                   \
       )                                                                       \
    {                                                                          \
        fprintf(stderr, "(no-kjb) dbf: "#X" is %f on source line %d of %s\n",  \
               (double)(X), __LINE__, __FILE__);                               \
    }                                                                          \
    else if (kjb_debug_level >= 2)                                             \
    {                                                                          \
        kjb_fprintf(stderr, "dbf: "#X" is %f on source line %d of %s\n",       \
                    (double)(X), __LINE__, __FILE__);                          \
    }

#else    /*   #ifdef HASH_MARK_SUBSTITUTION_OK ....     */

#define dbf(X) \
    if (    (kjb_debug_level == 1)                                             \
         || ((kjb_cleanup_started) && (kjb_debug_level > 0))                   \
       )                                                                       \
    {                                                                          \
        fprintf(stderr, "(no-kjb) dbf: X is %f on source line %d of %s\n",     \
                (double)(X), __LINE__, __FILE__);                              \
    }                                                                          \
    else if (kjb_debug_level >= 2)                                             \
    {                                                                          \
        kjb_fprintf(stderr, "dbf: X is %f on source line %d of %s\n",          \
                    (double)(X), __LINE__, __FILE__);                          \
    }

#endif    /*   #ifdef HASH_MARK_SUBSTITUTION_OK ... #else  .....   */

#endif    /*   #ifdef __C2MAN__ */

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                             dbe
 *
 * (MACRO) Debug printing of floating point
 *
 * This macro prints a floating point number in scientific notatation, along
 * with the variable name, file, and line number, on standard error. Its
 * behaviour is a function of the debugging level, which can be set through the
 * option "debug" if the KJB library options are being made available to the
 * user. The macro is available for both development and production code, but
 * since the default debug level is different in the two cases, the behaviour is
 * different. The default level for development code is 2, whereas for
 * production code it is 0.
 *
 * If the debug level is 2 (or more), then the output is printed to standard
 * error using regular KJB library output routines.
 *
 * If the debug level is 1, then the output is printed to standard error using
 * standard C library routines instead of regular KJB library routines. This
 * facility is provided for KJB library I/O routines themselves.
 *
 * If the debug level is 0 (default for production code), then the output is
 * disabled.
 *
 * Note :
 *    The argument is cast to double, and thus float, double, or double
 *    arguments are all OK.
 *
 * Index: debugging
 *
 * -----------------------------------------------------------------------------
*/

#ifdef __C2MAN__
    void dbe(double float_num);
#else

#ifdef HASH_MARK_SUBSTITUTION_OK

#define dbe(X)                                                                 \
    if (    (kjb_debug_level == 1)                                             \
         || ((kjb_cleanup_started) && (kjb_debug_level > 0))                   \
       )                                                                       \
    {                                                                          \
        fprintf(stderr, "(no-kjb) dbe: "#X" is %e on source line %d of %s\n",  \
                (double)(X), __LINE__, __FILE__);                              \
    }                                                                          \
    else if (kjb_debug_level >= 2)                                             \
    {                                                                          \
        kjb_fprintf(stderr, "dbe: "#X" is %e on source line %d of %s\n",       \
                    (double)(X), __LINE__, __FILE__);                          \
    }

#else    /*   #ifdef HASH_MARK_SUBSTITUTION_OK ....     */

#define dbe(X)                                                                 \
    if (    (kjb_debug_level == 1)                                             \
         || ((kjb_cleanup_started) && (kjb_debug_level > 0))                   \
       )                                                                       \
    {                                                                          \
        fprintf(stderr, "(no-kjb) dbe: X is %e on source line %d of %s\n",     \
                (double)(X), __LINE__, __FILE__);                              \
    }                                                                          \
    else if (kjb_debug_level >= 2)                                             \
    {                                                                          \
        kjb_fprintf(stderr, "dbe: X is %e on source line %d of %s\n",          \
                    (double)(X), __LINE__, __FILE__);                          \
    }

#endif    /*   #ifdef HASH_MARK_SUBSTITUTION_OK ... #else  .....   */

#endif    /*   #ifdef __C2MAN__ */

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                             dbc
 *
 * (MACRO) Debug printing of a character
 *
 * This macro prints a character as a character, in hex, and in regular integer
 * format, along with the variable name, file, and line number, on standard
 * error. Its behaviour is a function of the debugging level, which can be set
 * through the option "debug" if the KJB library options are being made
 * available to the user. The macro is available for both development and
 * production code, but since the default debug level is different in the two
 * cases, the behaviour is different. The default level for development code is
 * 2, whereas for production code it is 0.
 *
 * If the debug level is 2 (or more), then the output is printed to standard
 * error using regular KJB library output routines.
 *
 * If the debug level is 1, then the output is printed to standard error using
 * standard C library routines instead of regular KJB library routines. This
 * facility is provided for KJB library I/O routines themselves.
 *
 * If the debug level is 0 (default for production code), then the output is
 * disabled.
 *
 * Note :
 *    The argument is cast to int, and thus char or int arguments are all OK.
 *
 * Index: debugging
 *
 * -----------------------------------------------------------------------------
*/

#ifdef __C2MAN__
    void dbc(char c);
#else

#ifdef HASH_MARK_SUBSTITUTION_OK

#define dbc(X)                                                                 \
    if (    (kjb_debug_level == 1)                                             \
         || ((kjb_cleanup_started) && (kjb_debug_level > 0))                   \
       )                                                                       \
    {                                                                          \
        fprintf(stderr,                                                        \
                "(no-kjb) dbc: "#X" is ->%c<- (0x%x, %d) on source line %d of %s\n", \
                (int)(X), (int)(X), (int)(X), __LINE__, __FILE__);             \
    }                                                                          \
    else if (kjb_debug_level >= 2)                                             \
    {                                                                          \
        kjb_fprintf(stderr,                                                    \
                    "dbc: "#X" is ->%c<- (0x%x, %d) on source line %d of %s\n",\
                    (int)(X), (int)(X), (int)(X), __LINE__, __FILE__);         \
    }

#else    /*   #ifdef HASH_MARK_SUBSTITUTION_OK ....     */

#define dbc(X)                                                                 \
    if (    (kjb_debug_level == 1)                                             \
         || ((kjb_cleanup_started) && (kjb_debug_level > 0))                   \
       )                                                                       \
    {                                                                          \
        fprintf(stderr,                                                        \
                "(no-kjb) dbc: X is ->%c<- (0x%x, %d) on source line %d of %s\n", \
                 (int)(X), (int)(X), (int)(X), __LINE__, __FILE__);            \
    }                                                                          \
    else if (kjb_debug_level >= 2)                                             \
    {                                                                          \
        kjb_fprintf(stderr,                                                    \
                    "dbc: X is ->%c<- (0x%x, %d) on source line %d of %s\n",   \
                    (int)(X), (int)(X), (int)(X), __LINE__, __FILE__);         \
    }

#endif    /*   #ifdef HASH_MARK_SUBSTITUTION_OK ... #else  .....   */

#endif    /*   #ifdef __C2MAN__ */

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                             dbnc
 *
 * (MACRO) Debug printing of n characters
 *
 * This macro prints n characters as a string, along with the variable name,
 * file, and line number on standard error. Its behaviour is a function of the
 * debugging level, which can be set through the option "debug" if the KJB
 * library options are being made available to the user. The macro is available
 * for both development and production code, but since the default debug level
 * is different in the two cases, the behaviour is different. The default level
 * for development code is 2, whereas for production code it is 0.
 *
 * If the debug level is 2 (or more), then the output is printed to standard
 * error using regular KJB library output routines.
 *
 * If the debug level is 1, then the output is printed to standard error using
 * standard C library routines instead of regular KJB library routines. This
 * facility is provided for KJB library I/O routines themselves.
 *
 * If the debug level is 0 (default for production code), then the output is
 * disabled.
 *
 * Index: debugging
 *
 * -----------------------------------------------------------------------------
*/

#ifdef __C2MAN__
    void dbnc(char* buff, int count);
#else

#ifdef HASH_MARK_SUBSTITUTION_OK

#define dbnc(X, Y)                                                             \
    if (kjb_debug_level>0)                                                     \
     {                                                                         \
         char dbnc_buff[ 200 ];                                                \
         long len;                                                             \
                                                                               \
         len = MIN_OF(200 - ROOM_FOR_NULL, (long)(Y ));                        \
         kjb_strncpy(dbnc_buff, (X), (unsigned int)(len + ROOM_FOR_NULL));     \
                                                                               \
         if (    (kjb_debug_level == 1)                                        \
              || ((kjb_cleanup_started) && (kjb_debug_level > 0))              \
            )                                                                  \
         {                                                                     \
             fprintf(stderr,                                                   \
                     "(no-kjb) dbnc: "#X"(1, "#Y") is ->%s<- on source line %d of %s\n",\
                     dbnc_buff, __LINE__, __FILE__);                           \
         }                                                                     \
         else if (kjb_debug_level >= 2)                                        \
         {                                                                     \
             kjb_fprintf(stderr,                                               \
                    "dbnc: "#X"(1, "#Y") is ->%s<- on source line %d of %s\n", \
                    dbnc_buff, __LINE__, __FILE__);                            \
         }                                                                     \
     }

#else    /*   #ifdef HASH_MARK_SUBSTITUTION_OK ....     */

#define dbnc(X, Y)                                                             \
    if (kjb_debug_level > 0)                                                   \
    {                                                                          \
        char dbnc_buff[ 200 ];                                                 \
        long len;                                                              \
                                                                               \
         len = MIN_OF(200 - ROOM_FOR_NULL, (long)(Y ));                        \
         len = MAX_OF(len, 0);                                                 \
         kjb_strncpy(dbnc_buff, (X), len + ROOM_FOR_NULL);                     \
                                                                               \
         if (    (kjb_debug_level == 1)                                        \
              || ((kjb_cleanup_started) && (kjb_debug_level > 0))              \
            )                                                                  \
         {                                                                     \
             fprintf(stderr,                                                   \
                     "(no-kjb) dbnc: X(1, Y) is ->%s<- on source line %d of %s\n", \
                     dbnc_buff, __LINE__, __FILE__);                           \
         }                                                                     \
         else if (kjb_debug_level >= 2)                                        \
         {                                                                     \
             kjb_fprintf(stderr,                                               \
                         "dbnc: X(1, Y) is ->%s<- on source line %d of %s\n",  \
                         dbnc_buff, __LINE__, __FILE__);                       \
         }                                                                     \
     }

#endif    /*   #ifdef HASH_MARK_SUBSTITUTION_OK ... #else  .....   */

#endif    /*   #ifdef __C2MAN__ */

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                             dbs
 *
 * (MACRO) Debug printing of a string
 *
 * This macro prints a string, along with the variable name, file, and line
 * number, on standard error. The string is delimited by -> and <-. Its
 * behaviour is a function of the debugging level, which can be set through the
 * option "debug" if the KJB library options are being made available to the
 * user. The macro is available for both development and production code, but
 * since the default debug level is different in the two cases, the behaviour is
 * different. The default level for development code is 2, whereas for
 * production code it is 0.
 *
 * If the debug level is 2 (or more), then the output is printed to standard
 * error using regular KJB library output routines.
 *
 * If the debug level is 1, then the output is printed to standard error using
 * standard C library routines instead of regular KJB library routines. This
 * facility is provided for KJB library I/O routines themselves.
 *
 * If the debug level is 0 (default for production code), then the output is
 * disabled.
 *
 * NOTE:
 *     This macro can be called with a buffer (array of char) as an argument,
 *     but "dbb" is preferred in that case. 
 *
 * Index: debugging
 *
 * -----------------------------------------------------------------------------
*/

#ifdef __C2MAN__
    void dbs(char* str);
#else


#ifdef HASH_MARK_SUBSTITUTION_OK

#define dbs(X)                                                                 \
    if (    (kjb_debug_level == 1)                                             \
         || ((kjb_cleanup_started) && (kjb_debug_level > 0))                   \
       )                                                                       \
    {                                                                          \
        fprintf(stderr,                                                        \
                "(no-kjb) dbs: "#X" is ->%s<- on source line %d of %s\n",      \
                (X) ? (X) : "NULL", __LINE__, __FILE__);                       \
    }                                                                          \
    else if (kjb_debug_level >= 2)                                             \
    {                                                                          \
        kjb_fprintf(stderr, "dbs: "#X" is ->%s<- on source line %d of %s\n",   \
                    (X) ? (X) : "NULL",  __LINE__, __FILE__);                  \
    }

#else    /*   #ifdef HASH_MARK_SUBSTITUTION_OK ....     */

#define dbs(X)                                                                 \
    if (    (kjb_debug_level == 1)                                             \
         || ((kjb_cleanup_started) && (kjb_debug_level > 0))                   \
       )                                                                       \
    {                                                                          \
        fprintf(stderr, "(no-kjb) dbs: X is ->%s<- on source line %d of %s\n", \
                (X) ? (X) : "NULL", __LINE__, __FILE__);                       \
    }                                                                          \
    else if (kjb_debug_level >= 2)                                             \
    {                                                                          \
        kjb_fprintf(stderr, "dbs: X is ->%s<- on source line %d of %s\n",      \
                    (X) ? (X) : "NULL", __LINE__, __FILE__);                   \
    }

#endif    /*   #ifdef HASH_MARK_SUBSTITUTION_OK ... #else  .....   */

#endif    /*   #ifdef __C2MAN__ */

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                             dbb
 *
 * (MACRO) Debug printing of a buffer (array of char)
 *
 * This macro prints a string inside an array of charaters, along with the
 * variable name, file, and line number, on standard error. The string is
 * delimited by -> and <-. Its behaviour is a function of the debugging level,
 * which can be set through the option "debug" if the KJB library options are
 * being made available to the user. The macro is available for both development
 * and production code, but since the default debug level is different in the
 * two cases, the behaviour is different. The default level for development code
 * is 2, whereas for production code it is 0.
 *
 * If the debug level is 2 (or more), then the output is printed to standard
 * error using regular KJB library output routines.
 *
 * If the debug level is 1, then the output is printed to standard error using
 * standard C library routines instead of regular KJB library routines. This
 * facility is provided for KJB library I/O routines themselves.
 *
 * If the debug level is 0 (default for production code), then the output is
 * disabled.
 *
 * Index: debugging
 *
 * -----------------------------------------------------------------------------
*/

#ifdef __C2MAN__
    void dbb(char* str);
#else


#ifdef HASH_MARK_SUBSTITUTION_OK

#define dbb(X)                                                                 \
    if (    (kjb_debug_level == 1)                                             \
         || ((kjb_cleanup_started) && (kjb_debug_level > 0))                   \
       )                                                                       \
    {                                                                          \
        fprintf(stderr,                                                        \
                "(no-kjb) dbs: "#X" is ->%s<- on source line %d of %s\n",      \
                (X), __LINE__, __FILE__);                                      \
    }                                                                          \
    else if (kjb_debug_level >= 2)                                             \
    {                                                                          \
        kjb_fprintf(stderr, "dbs: "#X" is ->%s<- on source line %d of %s\n",   \
                 (X), __LINE__, __FILE__);                                     \
    }

#else    /*   #ifdef HASH_MARK_SUBSTITUTION_OK ....     */

#define dbb(X)                                                                 \
    if (    (kjb_debug_level == 1)                                             \
         || ((kjb_cleanup_started) && (kjb_debug_level > 0))                   \
       )                                                                       \
    {                                                                          \
        fprintf(stderr, "(no-kjb) dbs: X is ->%s<- on source line %d of %s\n", \
                (X), __LINE__, __FILE__);                                      \
    }                                                                          \
    else if (kjb_debug_level >= 2)                                             \
    {                                                                          \
        kjb_fprintf(stderr, "dbs: X is ->%s<- on source line %d of %s\n",      \
                 (X), __LINE__, __FILE__);                                     \
    }

#endif    /*   #ifdef HASH_MARK_SUBSTITUTION_OK ... #else  .....   */

#endif    /*   #ifdef __C2MAN__ */

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                             dbm
 *
 * (MACRO) Debug printing of a message
 *
 * This macro prints a message, along with the file and line number on standard
 * error. Its behaviour is a function of the debugging level, which can be set
 * through the option "debug" if the KJB library options are being made
 * available to the user. The macro is available for both development and
 * production code, but since the default debug level is different in the two
 * cases, the behaviour is different. The default level for development code is
 * 2, whereas for production code it is 0.
 *
 * If the debug level is 2 (or more), then the output is printed to standard
 * error using regular KJB library output routines.
 *
 * If the debug level is 1, then the output is printed to standard error using
 * standard C library routines instead of regular KJB library routines. This
 * facility is provided for KJB library I/O routines themselves.
 *
 * If the debug level is 0 (default for production code), then the output is
 * disabled.
 *
 * Note :
 *    The argument is cast to double, and thus float, double, or double
 *    arguments are all OK.
 *
 * Index: debugging
 *
 * -----------------------------------------------------------------------------
*/

#ifdef __C2MAN__
    void dbm(char* msg);
#else

#ifdef HASH_MARK_SUBSTITUTION_OK

#define dbm(X) \
    if (    (kjb_debug_level == 1)                                             \
         || ((kjb_cleanup_started) && (kjb_debug_level > 0))                   \
       )                                                                       \
    {                                                                          \
        fprintf(stderr, "(no-kjb) message: %s (from source line %d of %s)\n",  \
                (X),  __LINE__, __FILE__);                                     \
    }                                                                          \
    else if (kjb_debug_level >= 2)                                             \
    {                                                                          \
        kjb_fprintf(stderr, "message: %s (from source line %d of %s)\n", (X),  \
                    __LINE__, __FILE__);                                       \
    }

#else    /*   #ifdef HASH_MARK_SUBSTITUTION_OK ....     */

#define dbm(X)                                                                 \
    if (    (kjb_debug_level == 1)                                             \
         || ((kjb_cleanup_started) && (kjb_debug_level > 0))                   \
       )                                                                       \
    {                                                                          \
        fprintf(stderr, "(no-kjb) message: %s (from source line %d of %s)\n",  \
                (X), __LINE__, __FILE__);                                      \
    }                                                                          \
    else if (kjb_debug_level >= 2)                                             \
    {                                                                          \
        kjb_fprintf(stderr, "message: %s (from source line %d of %s)\n", (X),  \
                    __LINE__, __FILE__);                                       \
    }

#endif    /*   #ifdef HASH_MARK_SUBSTITUTION_OK ... #else  .....   */

#endif    /*   #ifdef __C2MAN__ */

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                             db_irv
 *
 * (MACRO) Debug printing of an integer vector
 *
 * This macro prints an integer vector as a row vector, along with the variable
 * name, file, and line number on standard error. Its behaviour is a function
 * of the debugging level, which can be set through the option "debug" if the
 * KJB library options are being made available to the user. The macro is
 * available for both development and production code, but since the default
 * debug level is different in the two cases, the behaviour is different. The
 * default level for development code is 2, whereas for production code it is
 * 0.
 *
 * If the debug level is 2 (or more), then the output is printed to standard
 * error using regular KJB library output routines.
 *
 * If the debug level is 1, then the output is printed to standard error using
 * standard C library routines instead of regular KJB library routines. This
 * facility is provided for KJB library I/O routines themselves.
 *
 * If the debug level is 0 (default for production code), then the output is
 * disabled.
 *
 * Index: debugging
 *
 * -----------------------------------------------------------------------------
*/

#ifdef __C2MAN__
    void db_irv(const Int_vector *vp);
#else

#ifdef HASH_MARK_SUBSTITUTION_OK

#define db_irv(X)                                                              \
    if (kjb_debug_level >= 1)                                                  \
    {                                                                          \
        fp_write_row_int_vector_with_title(X, stderr, ""#X"");                 \
    }


#else    /*   #ifdef HASH_MARK_SUBSTITUTION_OK ....     */

#define db_irv(X)                                                              \
    if (kjb_debug_level >= 1)                                                  \
    {                                                                          \
        fp_write_row_int_vector_with_title(X, stderr, "X");                    \
    }


#endif    /*   #ifdef HASH_MARK_SUBSTITUTION_OK ... #else  .....   */

#endif    /*   #ifdef __C2MAN__ */

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                             db_icv
 *
 * (MACRO) Debug printing of an integer vector
 *
 * This macro prints an integer vector as a column vector, along with the
 * variable name, file, and line number on standard error. Its behaviour is a
 * function of the debugging level, which can be set through the option "debug"
 * if the KJB library options are being made available to the user. The macro
 * is available for both development and production code, but since the default
 * debug level is different in the two cases, the behaviour is different. The
 * default level for development code is 2, whereas for production code it is
 * 0.
 *
 * If the debug level is 2 (or more), then the output is printed to standard
 * error using regular KJB library output routines.
 *
 * If the debug level is 1, then the output is printed to standard error using
 * standard C library routines instead of regular KJB library routines. This
 * facility is provided for KJB library I/O routines themselves.
 *
 * If the debug level is 0 (default for production code), then the output is
 * disabled.
 *
 * Index: debugging
 *
 * -----------------------------------------------------------------------------
*/

#ifdef __C2MAN__
    void db_icv(const Int_vector *vp);
#else

#ifdef HASH_MARK_SUBSTITUTION_OK

#define db_icv(X)                                                              \
    if (kjb_debug_level >= 1)                                                  \
    {                                                                          \
        fp_write_col_int_vector_with_title(X, stderr, ""#X"");                              \
    }

#else    /*   #ifdef HASH_MARK_SUBSTITUTION_OK ....     */

#define db_icv(X)                                                              \
   if (kjb_debug_level >= 1)                                                   \
   {                                                                           \
       fp_write_col_int_vector_with_title(X, stderr, "X");                                  \
   }

#endif    /*   #ifdef HASH_MARK_SUBSTITUTION_OK ... #else  .....   */

#endif    /*   #ifdef __C2MAN__ */

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                             file_db_icv
 *
 * (MACRO) Debug printing an integer vector to a file
 *
 * This macro prints an integer vector as a column vector, to a file in the
 * current directory determined by the variable name.  Its behaviour is a
 * function of the debugging level, which can be set through the option "debug"
 * if the KJB library options are being made available to the user. The macro
 * is available for both development and production code, but since the default
 * debug level is different in the two cases, the behaviour is different. The
 * default level for development code is 2, whereas for production code it is
 * 0.
 *
 * If the debug level is 1 (or more), then the matrix is output to a file which
 * has the same name as the variable.
 *
 * Notes:
 *    This macro is only avaible with compilers which have "hash mark
 *    substitution. With compilers that don't, a message to this effect is
 *    printed instead.
 *
 * Index: debugging
 *
 * -----------------------------------------------------------------------------
*/

#ifdef __C2MAN__
    void file_db_icv(const Int_vector *vp);
#else

#ifdef HASH_MARK_SUBSTITUTION_OK

#define file_db_icv(X)                                                               \
    if (kjb_debug_level >= 1)                                                  \
    {                                                                          \
        FILE* fp;                                                              \
                                                                               \
        p_stderr("Writing vector "#X" from line %d of %s to file.    \n",      \
                 __LINE__, __FILE__);                                          \
        fp = kjb_fopen(""#X"", "w");                                           \
                                                                               \
        if (fp == NULL)                                                        \
        {                                                                      \
            kjb_print_error();                                                 \
        }                                                                      \
        else                                                                   \
        {                                                                      \
            EPE(fp_write_col_int_vector_with_title(X, fp, ""#X""));            \
            EPE(kjb_fclose(fp));                                               \
        }                                                                      \
    }

#else    /*   #ifdef HASH_MARK_SUBSTITUTION_OK ....     */

#define file_db_icv(X)                                                          \
    p_stderr("Macro file_db_icv() is not available with this compiler.\n");

#endif    /*   #ifdef HASH_MARK_SUBSTITUTION_OK ... #else  .....   */

#endif    /*   #ifdef __C2MAN__ */

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                             dbi_mat
 *
 * (MACRO) Debug printing of an integer matrix
 *
 * This macro prints an integer matrix, along with the variable name, file, and
 * line number on standard error. Its behaviour is a function of the debugging
 * level, which can be set through the option "debug" if the KJB library
 * options are being made available to the user. The macro is available for
 * both development and production code, but since the default debug level is
 * different in the two cases, the behaviour is different. The default level
 * for development code is 2, whereas for production code it is 0.
 *
 * If the debug level is 1 (or more), then the output is printed to standard
 * error using regular KJB library output routines. Note that this is different
 * from more basic debug macros where a debug level of 1 is used to print using
 * system routines and a debug level of 2 or more is used for printing using
 * KJB library routines. (The special behavour of debug level 1 in the case of
 * the more basic debug macros is provided to help debug the KJB library I/O
 * routines themselve).
 *
 * If the debug level is 0 (default for production code), then the output is
 * disabled.
 *
 * Index: debugging
 *
 * -----------------------------------------------------------------------------
*/

#ifdef __C2MAN__
    void dbi_mat(const Int_matrix *mp);
#else

#ifdef HASH_MARK_SUBSTITUTION_OK

#define dbi_mat(X)                                                             \
    if (kjb_debug_level >= 1)                                                  \
    {                                                                          \
        fp_write_int_matrix_with_title(X, stderr, ""#X"");                        \
    }

#else    /*   #ifdef HASH_MARK_SUBSTITUTION_OK ....     */

#define dbi_mat(X)                                                             \
    if (kjb_debug_level >= 1)                                                  \
    {                                                                          \
        fp_write_int_matrix_with_title(X, stderr, "X");                           \
    }

#endif    /*   #ifdef HASH_MARK_SUBSTITUTION_OK ... #else  .....   */

#endif    /*   #ifdef __C2MAN__ */

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                             file_dbi_mat
 *
 * (MACRO) Debug printing of an integer matrix
 *
 * This macro prints a matrix to a file in the current directory with the same
 * name as the variable.  The file and line number of the matrix being output to
 * the are printed on standard error.  Its behaviour is a function of the
 * debugging level, which can be set through the option "debug" if the KJB
 * library options are being made available to the user. The macro is available
 * for both development and production code, but since the default debug level
 * is different in the two cases, the default behaviour is different. The
 * default level for development code is 2, whereas for production code it is 0.
 *
 * If the debug level is 1 (or more), then the matrix is output to a file which
 * has the same name as the variable.
 *
 * Notes:
 *    This macro is only avaible with compilers which have "hash mark
 *    substitution. With compilers that don't, a message to this effect is
 *    printed instead.
 *
 * Index: debugging
 *
 * -----------------------------------------------------------------------------
*/

#ifdef __C2MAN__
    void file_dbi_mat(const Int_matrix *mp);
#else

#ifdef HASH_MARK_SUBSTITUTION_OK

#define file_dbi_mat(X)                                                        \
    if (kjb_debug_level >= 1)                                                  \
    {                                                                          \
        FILE* fp;                                                              \
                                                                               \
        p_stderr("Writing matrix "#X" from line %d of %s to file.    \n",      \
                 __LINE__, __FILE__);                                          \
        fp = kjb_fopen(""#X"", "w");                                           \
                                                                               \
        if (fp == NULL)                                                        \
        {                                                                      \
            kjb_print_error();                                                 \
        }                                                                      \
        else                                                                   \
        {                                                                      \
            if (kjb_debug_level >= 3)                                          \
            {                                                                  \
                EPE(fp_write_int_matrix_with_title(X, fp, ""#X""));    \
            }                                                                  \
            else                                                               \
            {                                                                  \
                EPE(fp_write_int_matrix_with_title(X, fp, ""#X""));            \
            }                                                                  \
            EPE(kjb_fclose(fp));                                               \
        }                                                                      \
    }

#else    /*   #ifdef HASH_MARK_SUBSTITUTION_OK ....     */

#define file_dbi_mat(X)                                                         \
    p_stderr("Macro file_dbi_mat() is not available with this compiler.\n");

#endif    /*   #ifdef HASH_MARK_SUBSTITUTION_OK ... #else  .....   */

#endif    /*   #ifdef __C2MAN__ */

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                             dbw_no_kjb
 *
 * (MACRO) Prints file and line number
 *
 * This macro prints the file and line number, on standard error without using
 * the KJB library. This routine is thus normally used only to help debug the
 * lowest level KJB library routines (all in "l") in conjunction with other code
 * (so that kjb_debug_level) does not need to be set/reset.
 *
 * Index: debugging
 *
 * -----------------------------------------------------------------------------
*/
#ifdef __C2MAN__
    void dbw_no_kjb(void);
#else

#define  dbw_no_kjb()                                                          \
    fprintf(stderr, "At line %d of %s\n", __LINE__, __FILE__);

#endif

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                             dbr_no_kjb
 *
 * (MACRO) Debug printing status code
 *
 * This macro prints an tag corresponding to the KJB library status meaning of
 * an integer, along with the file and line number, on standard error. KJB
 * library facilities are not used. This routine is thus normally used only to
 * help debug the lowest level KJB library routines (all in "l") in conjunction
 * with other code (so that kjb_debug_level) does not need to be set/reset.
 *
 * Index: debugging
 *
 * -----------------------------------------------------------------------------
*/

#ifdef __C2MAN__
    void dbr_no_kjb(int i);
#else

#define dbr_no_kjb(X)                                                          \
        fprintf(stderr,                                                        \
                "dbr_no_kjb: Status is %ld on source line %d of %s\n",         \
                (long)(X), __LINE__, __FILE__);

#endif    /*   #ifdef __C2MAN__ */

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                             dbp_no_kjb
 *
 * (MACRO) Debug printing of a string
 *
 * This macro prints a string on standard error. Unlike the other "db" macros,
 * dbp does NOT output the file or the line number. It is normally used to tidy
 * up debugging output.  KJB library facilities are not used. This routine is
 * thus normally used only to help debug the lowest level KJB library routines
 * (all in "l") in conjunction with other code (so that kjb_debug_level) does
 * not need to be set/reset.
 *
 * Index: debugging
 *
 * -----------------------------------------------------------------------------
*/

#ifdef __C2MAN__
    void dbp_no_kjb(char* text_to_print);
#else

#define dbp_no_kjb(X) fprintf(stderr, "%s\n", (X));

#endif    /*   #ifdef __C2MAN__ */

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                             dbx_no_kjb
 *
 * (MACRO) Debug printing of integer in hex
 *
 * This macro prints an integer (or long) in hex along with the variable name,
 * file, and line number on standard error. KJB library facilities are not used.
 * This routine is thus normally used only to help debug the lowest level KJB
 * library routines (all in "l") in conjunction with other code (so that
 * kjb_debug_level) does not need to be set/reset.
 *
 * Index: debugging
 *
 * -----------------------------------------------------------------------------
*/

#ifdef __C2MAN__
    void dbx_no_kjb(int i);
#else

#ifdef HASH_MARK_SUBSTITUTION_OK

#define dbx_no_kjb(X)                                                          \
        fprintf(stderr, "dbx_no_kjb: "#X" is %lx on source line %d of %s\n",   \
                (unsigned long)(X), __LINE__, __FILE__);

#else    /*   #ifdef HASH_MARK_SUBSTITUTION_OK ....     */

#define dbx_no_kjb(X)                                                          \
        fprintf(stderr, "db_no_kjbx: X is %lx on source line %d of %s\n",      \
                (unsigned long)(X), __LINE__, __FILE__);

#endif    /*   #ifdef HASH_MARK_SUBSTITUTION_OK ... #else  .....   */

#endif    /*   #ifdef __C2MAN__ */

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                             dbo_no_kjb
 *
 * (MACRO) Debug printing of integer in octal
 *
 * This macro prints an integer (or long) in octal along with the variable name,
 * file, and line number on standard error. KJB library facilities are not
 * used. This routine is thus normally used only to help debug the lowest level
 * KJB library routines (all in "l") in conjunction with other code (so that
 * kjb_debug_level) does not need to be set/reset.
 *
 * Index: debugging
 *
 * -----------------------------------------------------------------------------
*/

#ifdef __C2MAN__
    void dbo_no_kjb(int i);
#else

#ifdef HASH_MARK_SUBSTITUTION_OK

#define dbo_no_kjb(X)                                                          \
        fprintf(stderr, "dbo_no_kjb: "#X" is %lo on source line %d of %s\n",   \
                (long)(X), __LINE__, __FILE__);

#else    /*   #ifdef HASH_MARK_SUBSTITUTION_OK ....     */

#define dbo_no_kjb(X)                                                          \
        fprintf(stderr, "dbo_no_kjb: X is %lo on source line %d of %s\n",      \
                (long)(X), __LINE__, __FILE__);

#endif    /*   #ifdef HASH_MARK_SUBSTITUTION_OK ... #else  .....   */

#endif    /*   #ifdef __C2MAN__ */

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                             dbi_no_kjb
 *
 * (MACRO) Debug printing of integer
 *
 * This macro prints an integer (or long) along with the variable name, file,
 * and line number, on standard error. KJB library facilities are not
 * used. This routine is thus normally used only to help debug the lowest level
 * KJB library routines (all in "l") in conjunction with other code (so that
 * kjb_debug_level) does not need to be set/reset.
 *
 * Index: debugging
 *
 * -----------------------------------------------------------------------------
*/

#ifdef __C2MAN__
    void dbi_no_kjb(int i);
#else

#ifdef HASH_MARK_SUBSTITUTION_OK

#define dbi_no_kjb(X)                                                          \
        fprintf(stderr,                                                        \
                "dbi_no_kjb: "#X" is %ld on source line %d of %s\n", (long)(X),\
               __LINE__, __FILE__);

#else    /*   #ifdef HASH_MARK_SUBSTITUTION_OK ....     */

#define dbi_no_kjb(X)                                                          \
        fprintf(stderr, "dbi_no_kjb: X is %ld on source line %d of %s\n",      \
                (long)(X), __LINE__, __FILE__);

#endif    /*   #ifdef HASH_MARK_SUBSTITUTION_OK ... #else  .....   */

#endif    /*   #ifdef __C2MAN__ */

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                             dbf_no_kjb
 *
 * (MACRO) Debug printing of floating point
 *
 * This macro prints a floating point number in fixed format, along with the
 * variable name, file, and line number, on standard error.  KJB library
 * facilities are not used. This routine is thus normally used only to help
 * debug the lowest level KJB library routines (all in "l") in conjunction with
 * other code (so that kjb_debug_level) does not need to be set/reset.
 *
 * Index: debugging
 *
 * -----------------------------------------------------------------------------
*/

#ifdef __C2MAN__
    void dbf_no_kjb(double float_num);
#else

#ifdef HASH_MARK_SUBSTITUTION_OK

#define dbf_no_kjb(X)                                                          \
        fprintf(stderr, "dbf_no_kjb: "#X" is %f on source line %d of %s\n",    \
               (double)(X), __LINE__, __FILE__);

#else    /*   #ifdef HASH_MARK_SUBSTITUTION_OK ....     */

#define dbf_no_kjb(X) \
        fprintf(stderr, "dbf_no_kjb: X is %f on source line %d of %s\n",       \
                (double)(X), __LINE__, __FILE__);

#endif    /*   #ifdef HASH_MARK_SUBSTITUTION_OK ... #else  .....   */

#endif    /*   #ifdef __C2MAN__ */

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                             dbe_no_kjb
 *
 * (MACRO) Debug printing of floating point
 *
 * This macro prints a floating point number in scientific notatation, along
 * with the variable name, file, and line number, on standard error. KJB library
 * facilities are not used. This routine is thus normally used only to help
 * debug the lowest level KJB library routines (all in "l") in conjunction with
 * other code (so that kjb_debug_level) does not need to be set/reset.
 *
 * Index: debugging
 *
 * -----------------------------------------------------------------------------
*/

#ifdef __C2MAN__
    void dbe_no_kjb(double float_num);
#else

#ifdef HASH_MARK_SUBSTITUTION_OK

#define dbe_no_kjb(X)                                                          \
        fprintf(stderr, "dbe_no_kjb: "#X" is %e on source line %d of %s\n",    \
                (double)(X), __LINE__, __FILE__);

#else    /*   #ifdef HASH_MARK_SUBSTITUTION_OK ....     */

#define dbe_no_kjb(X)                                                          \
        fprintf(stderr, "dbe_no_kjb: X is %e on source line %d of %s\n",       \
                (double)(X), __LINE__, __FILE__);

#endif    /*   #ifdef HASH_MARK_SUBSTITUTION_OK ... #else  .....   */

#endif    /*   #ifdef __C2MAN__ */

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                             dbc_no_kjb
 *
 * (MACRO) Debug printing of a character
 *
 * This macro prints a character as a character, in hex, and in regular integer
 * format, along with the variable name, file, and line number, on standard
 * error.  KJB library facilities are not used. This routine is thus normally
 * used only to help debug the lowest level KJB library routines (all in "l") in
 * conjunction with other code (so that kjb_debug_level) does not need to be
 * set/reset.
 *
 * Index: debugging
 *
 * -----------------------------------------------------------------------------
*/

#ifdef __C2MAN__
    void dbc_no_kjb(char c);
#else

#ifdef HASH_MARK_SUBSTITUTION_OK

#define dbc_no_kjb(X)                                                          \
        fprintf(stderr,                                                        \
             "dbc_no_kjb: "#X" is ->%c<- (0x%x, %d) on source line %d of %s\n",\
                (int)(X), (int)(X), (int)(X), __LINE__, __FILE__);

#else    /*   #ifdef HASH_MARK_SUBSTITUTION_OK ....     */

#define dbc_no_kjb(X)                                                          \
        fprintf(stderr,                                                        \
                "dbc_no_kjb: X is ->%c<- (0x%x, %d) on source line %d of %s\n",\
                 (int)(X), (int)(X), (int)(X), __LINE__, __FILE__);

#endif    /*   #ifdef HASH_MARK_SUBSTITUTION_OK ... #else  .....   */

#endif    /*   #ifdef __C2MAN__ */

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                             dbnc_no_kjb
 *
 * (MACRO) Debug printing of n characters
 *
 * This macro prints n characters as a string, along with the variable name,
 * file, and line number on standard error. KJB library facilities are not used.
 * This routine is thus normally used only to help debug the lowest level KJB
 * library routines (all in "l") in conjunction with other code (so that
 * kjb_debug_level) does not need to be set/reset.
 *
 * Index: debugging
 *
 * -----------------------------------------------------------------------------
*/

#ifdef __C2MAN__
    void dbnc_no_kjb(char* buff, int count);
#else

#ifdef HASH_MARK_SUBSTITUTION_OK

#define dbnc_no_kjb(X, Y)                                                      \
     {                                                                         \
         char dbnc_buff[ 200 ];                                                \
         long len;                                                             \
                                                                               \
         len = MIN_OF(200 - ROOM_FOR_NULL, (long)(Y ));                        \
         kjb_strncpy(dbnc_buff, (X), len + ROOM_FOR_NULL);                     \
                                                                               \
         fprintf(stderr,                                                       \
              "dbnc_no_kjb: "#X"(1, "#Y") is ->%s<- on source line %d of %s\n",\
                 dbnc_buff, __LINE__, __FILE__);                               \
     }

#else    /*   #ifdef HASH_MARK_SUBSTITUTION_OK ....     */

#define dbnc_no_kjb(X, Y)                                                      \
    {                                                                          \
        char dbnc_buff[ 200 ];                                                 \
        long len;                                                              \
                                                                               \
         len = MIN_OF(200 - ROOM_FOR_NULL, (long)(Y ));                        \
         len = MAX_OF(len, 0);                                                 \
         kjb_strncpy(dbnc_buff, (X), len + ROOM_FOR_NULL);                     \
                                                                               \
         fprintf(stderr,                                                       \
                 "dbnc_no_kjb: X(1, Y) is ->%s<- on source line %d of %s\n",   \
                 dbnc_buff, __LINE__, __FILE__);                               \
     }

#endif    /*   #ifdef HASH_MARK_SUBSTITUTION_OK ... #else  .....   */

#endif    /*   #ifdef __C2MAN__ */

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                             dbs_no_kjb
 *
 * (MACRO) Debug printing of a string
 *
 * This macro prints a string, along with the variable name, file, and line
 * number, on standard error. The string is delimited by -> and <-.  KJB library
 * facilities are not used. This routine is thus normally used only to help
 * debug the lowest level KJB library routines (all in "l") in conjunction with
 * other code (so that kjb_debug_level) does not need to be set/reset.
 *
 * Index: debugging
 *
 * -----------------------------------------------------------------------------
*/

#ifdef __C2MAN__
    void dbs_no_kjb(char* str);
#else


#ifdef HASH_MARK_SUBSTITUTION_OK

#define dbs_no_kjb(X)                                                          \
        fprintf(stderr, "dbs_no_kjb: "#X" is ->%s<- on source line %d of %s\n",\
                (X) ? (X) : "NULL", __LINE__, __FILE__);

#else    /*   #ifdef HASH_MARK_SUBSTITUTION_OK ....     */

#define dbs_no_kjb(X)                                                          \
        fprintf(stderr, "dbs_no_kjb: X is ->%s<- on source line %d of %s\n",   \
                (X) ? (X) : "NULL", __LINE__, __FILE__);

#endif    /*   #ifdef HASH_MARK_SUBSTITUTION_OK ... #else  .....   */

#endif    /*   #ifdef __C2MAN__ */

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                             dbm_no_kjb
 *
 * (MACRO) Debug printing of a message
 *
 * This macro prints a message, along with the file and line number on standard
 * error. KJB library facilities are not used. This routine is thus normally
 * used only to help debug the lowest level KJB library routines (all in "l") in
 * conjunction with other code (so that kjb_debug_level) does not need to be
 * set/reset.
 *
 * Index: debugging
 *
 * -----------------------------------------------------------------------------
*/

#ifdef __C2MAN__
    void dbm_no_kjb(char* msg);
#else

#ifdef HASH_MARK_SUBSTITUTION_OK

#define dbm_no_kjb(X) \
        fprintf(stderr, "message: %s (from source line %d of %s)\n", (X),      \
                __LINE__, __FILE__);

#else    /*   #ifdef HASH_MARK_SUBSTITUTION_OK ....     */

#define dbm_no_kjb(X) \
        fprintf(stderr, "message: %s (from source line %d of %s)\n", (X),      \
                __LINE__, __FILE__);

#endif    /*   #ifdef HASH_MARK_SUBSTITUTION_OK ... #else  .....   */

#endif    /*   #ifdef __C2MAN__ */

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                             ASSERT
 *
 * (MACRO) Calls kjb_abort if argument is false
 *
 * This macro only does something in development code. In production code (i.e.
 * code compiled without -DTEST, this macro evaluates to null.
 *
 * If the argument "boolean_expression" is false, then this macro prints the
 * assertion, together with the file and line number, on standard error. It then
 * calls kjb_abort.
 *
 * If you use this macro, you'll also need to include the header defining
 * the p_stderr function, since this header does not include that one (since
 * you might include its header but not use the macro).
 *
 * Index: debugging, Macros, asserts, output
 *
 * -----------------------------------------------------------------------------
*/

#ifdef __C2MAN__    /* Pretend we are a ".c" file to document MACROS. */

    void ASSERT(int boolean_expression);

#endif

#ifndef __C2MAN__  /* Just doing "else" confuses ctags for the first such line.
                      I have no idea what the problem is. */
#ifdef TEST
#    ifdef HASH_MARK_SUBSTITUTION_OK
#        define ASSERT(X)                                                      \
                    if (!(X))                                                  \
                    {                                                          \
                          USING_KJB_C();                                       \
                          p_stderr("Assert ("#X") failed on line %d of %s\n",  \
                                     __LINE__, __FILE__);                      \
                          p_stderr("Calling kjb_abort.\n");              \
                          kjb_abort();                                   \
                    }

#    else

#        define ASSERT(X)                                                      \
                    if (!(X))                                                  \
                    {                                                          \
                         p_stderr("Assert failed on line %d of %s\n",          \
                                      __LINE__, __FILE__);                     \
                         p_stderr("Calling kjb_abort.\n");                     \
                         kjb_abort();                                          \
                    }
#    endif

#else

#    define ASSERT(X)

#endif

#endif

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

#ifdef TEST
#ifndef DEFINE_ASSERTS
#define DEFINE_ASSERTS
#endif
#else
#ifdef DEBUG_OPTIMIZED_CODE
#ifndef DEFINE_ASSERTS
#define DEFINE_ASSERTS
#endif
#endif
#endif


#define USE_ASSERT_WRAPPERS

#ifdef USE_ASSERT_WRAPPERS

/* =============================================================================
 *                             ASSERT_IS_BOOL
 *
 * (MACRO) Calls kjb_abort if the integer argument is not 0 or 1
 *
 * This macro only does something in development code. In production code (i.e.
 * code compiled without -DTEST, this macro evaluates to null.
 *
 * If the argument "boolean_expression" is false, then this macro prints the
 * assertion, together with the file and line number, on standard error. It then
 * calls kjb_abort.
 *
 * Index: debugging, Macros, asserts
 *
 * -----------------------------------------------------------------------------
*/
#ifdef __C2MAN__    /* Pretend we are a ".c" file to document MACROS. */

    void ASSERT_IS_BOOL(int x);

#else
#ifdef DEFINE_ASSERTS
#define ASSERT_IS_BOOL(x)  \
if ((x != FALSE) || (x != TRUE))  \
{ \
    dbp("====================================="); \
    dbp("ASSERT_IS_BOOL failed"); \
    dbi(x); \
    term_beep_beep(3, 1000); \
    kjb_abort(); \
    dbp("====================================="); \
}
#else
#define ASSERT_IS_BOOL(x)
#endif
#endif


/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                             ASSERT_IS_EQUAL_INT
 *
 * (MACRO) Calls kjb_abort if the two integer arguments are not the same
 *
 * This macro only does something in development code. In production code (i.e.
 * code compiled without -DTEST, this macro evaluates to null.
 *
 * If the argument "boolean_expression" is false, then this macro prints the
 * assertion, together with the file and line number, on standard error. It then
 * calls kjb_abort.
 *
 * Index: debugging, Macros, asserts
 *
 * -----------------------------------------------------------------------------
*/
#ifdef __C2MAN__    /* Pretend we are a ".c" file to document MACROS. */

    void ASSERT_IS_EQUAL_INT(int x, int y);

#else
#ifdef DEFINE_ASSERTS
#define ASSERT_IS_EQUAL_INT(x,y)  \
if (x != y)  \
{ \
    dbp("====================================="); \
    dbp("ASSERT_IS_EQUAL_INT failed"); \
    dbi(x); \
    dbi(y); \
    term_beep_beep(3, 1000); \
    kjb_abort(); \
    dbp("====================================="); \
}
#else
#define ASSERT_IS_EQUAL_INT(x,y)
#endif
#endif


/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                             ASSERT_IS_NON_NEGATIVE_INT
 *
 * (MACRO) Calls kjb_abort if the integer argument is negative
 *
 * This macro only does something in development code. In production code (i.e.
 * code compiled without -DTEST, this macro evaluates to null.
 *
 * If the argument "boolean_expression" is false, then this macro prints the
 * assertion, together with the file and line number, on standard error. It then
 * calls kjb_abort.
 *
 * Index: debugging, Macros, asserts
 *
 * -----------------------------------------------------------------------------
*/
#ifdef __C2MAN__    /* Pretend we are a ".c" file to document MACROS. */
    void ASSERT_IS_NON_NEGATIVE_INT(int x);
#else
#ifdef DEFINE_ASSERTS
#define ASSERT_IS_NON_NEGATIVE_INT(x)  \
if (x < 0)  \
{ \
    dbp("====================================="); \
    dbp("ASSERT_IS_NON_NEGATIVE_INT failed"); \
    dbi(x); \
    term_beep_beep(3, 1000); \
    kjb_abort(); \
    dbp("====================================="); \
}
#else
#define ASSERT_IS_NON_NEGATIVE_INT(x)
#endif
#endif

/* =============================================================================
 *                             ASSERT_IS_POSITIVE_INT
 *
 * (MACRO) Calls kjb_abort if argument is argument is not postive
 *
 * This macro only does something in development code. In production code (i.e.
 * code compiled without -DTEST, this macro evaluates to null.
 *
 * If the argument integer_expression is not positive, then this macro prints
 * the assertion, together with the file and line number, on standard error. It
 * then calls kjb_abort.
 *
 * Index: debugging, Macros, asserts
 *
 * -----------------------------------------------------------------------------
*/
#ifdef __C2MAN__    /* Pretend we are a ".c" file to document MACROS. */
    void ASSERT_IS_POSITIVE_INT(int integer_expression);
#else
#ifdef DEFINE_ASSERTS
#define ASSERT_IS_POSITIVE_INT(x)  \
if (x <= 0)  \
{ \
    dbp("====================================="); \
    dbp("ASSERT_IS_POSITIVE_INT failed"); \
    dbi(x); \
    term_beep_beep(3, 1000); \
    kjb_abort(); \
    dbp("====================================="); \
}
#else
#define ASSERT_IS_POSITIVE_INT(x)
#endif
#endif


/* =============================================================================
 *                             ASSERT_IS_NOT_EQUAL_INT
 *
 * (MACRO) Calls kjb_abort if the two integer arguments are not equal
 *
 * This macro only does something in development code. In production code (i.e.
 * code compiled without -DTEST, this macro evaluates to null.
 *
 * If the two integer arguments are not equal, then this macro prints the
 * assertion, together with the file and line number, on standard error. It then
 * calls kjb_abort.
 *
 * Index: debugging, Macros, asserts
 *
 * -----------------------------------------------------------------------------
*/

#ifdef __C2MAN__    /* Pretend we are a ".c" file to document MACROS. */
    void ASSERT_IS_NOT_EQUAL_INT(int integer_1, int integer_2);
#else
#ifdef DEFINE_ASSERTS
#define ASSERT_IS_NOT_EQUAL_INT(x,y)  \
if (IS_EQUAL_INT(x,y))  \
{ \
    dbp("====================================="); \
    dbp("ASSERT_IS_NOT_EQUAL_INT failed"); \
    dbi(x); \
    dbi(y); \
    term_beep_beep(3, 1000); \
    kjb_abort(); \
    dbp("====================================="); \
}
#else
#define ASSERT_IS_NOT_EQUAL_INT(x,y)
#endif
#endif

/* =============================================================================
 *                             ASSERT_IS_GREATER_INT
 *
 * (MACRO) Calls kjb_abort if first argument is less than or equal the second.
 *
 * This macro only does something in development code. In production code (i.e.
 * code compiled without -DTEST, this macro evaluates to null.
 *
 * If the first integer argument is less than or equal than the second, then
 * this macro prints the assertion, together with the file and line number, on
 * standard error. It then calls kjb_abort.
 *
 * Index: debugging, Macros, asserts
 *
 * -----------------------------------------------------------------------------
*/
#ifdef __C2MAN__    /* Pretend we are a ".c" file to document MACROS. */
    void ASSERT_IS_GREATER_INT(int integer_1, int integer_2);
#else
#ifdef DEFINE_ASSERTS
#define ASSERT_IS_GREATER_INT(x,y)  \
if (x <= y)  \
{ \
    dbp("====================================="); \
    dbp("ASSERT_IS_GREATER_INT failed"); \
    dbi(x); \
    dbi(y); \
    term_beep_beep(3, 1000); \
    kjb_abort(); \
    dbp("====================================="); \
}
#else
#define ASSERT_IS_GREATER_INT(x,y)
#endif
#endif

/* =============================================================================
 *                             ASSERT_IS_LESS_INT
 *
 * (MACRO) Calls kjb_abort if first argument greater than or equal the second.
 *
 * This macro only does something in development code. In production code (i.e.
 * code compiled without -DTEST, this macro evaluates to null.
 *
 * If the first integer argument  greater than or equal the second, then this
 * macro prints the assertion, together with the file and line number, on
 * standard error. It then calls kjb_abort.
 *
 * Index: debugging, Macros, asserts
 *
 * -----------------------------------------------------------------------------
*/
#ifdef __C2MAN__    /* Pretend we are a ".c" file to document MACROS. */
    void ASSERT_IS_LESS_INT(int integer_1, int integer_2);
#else
#ifdef DEFINE_ASSERTS
#define ASSERT_IS_LESS_INT(x,y)  \
if (x >= y)  \
{ \
    dbp("====================================="); \
    dbp("ASSERT_IS_LESS_INT failed"); \
    dbi(x); \
    dbi(y); \
    term_beep_beep(3, 1000); \
    kjb_abort(); \
    dbp("====================================="); \
}
#else
#define ASSERT_IS_LESS_INT(x,y)
#endif
#endif


/* =============================================================================
 *                             ASSERT_IS_NOT_GREATER_INT
 *
 * (MACRO) Calls kjb_abort if first argument is greater than second
 *
 * This macro only does something in development code. In production code (i.e.
 * code compiled without -DTEST, this macro evaluates to null.
 *
 * If the first integer argument is greater than the second, then this macro
 * prints the assertion, together with the file and line number, on standard
 * error. It then calls kjb_abort.
 *
 * Index: debugging, Macros, asserts
 *
 * -----------------------------------------------------------------------------
*/
#ifdef __C2MAN__    /* Pretend we are a ".c" file to document MACROS. */
    void ASSERT(int boolean_expression);
#else
#ifdef DEFINE_ASSERTS
#define ASSERT_IS_NOT_GREATER_INT(x,y)  \
if (x > y)  \
{ \
    dbp("====================================="); \
    dbp("ASSERT_IS_NOT_GREATER_INT failed"); \
    dbi(x); \
    dbi(y); \
    term_beep_beep(3, 1000); \
    kjb_abort(); \
    dbp("====================================="); \
}
#else
#define ASSERT_IS_NOT_GREATER_INT(x,y)
#endif
#endif


/* =============================================================================
 *                             ASSERT_IS_NOT_LESS_INT
 *
 * (MACRO) Calls kjb_abort if first argument is less than second
 *
 * This macro only does something in development code. In production code (i.e.
 * code compiled without -DTEST, this macro evaluates to null.
 *
 * If the first integer argument is less than the second, then this macro prints
 * the assertion, together with the file and line number, on standard error. It
 * then calls kjb_abort.
 *
 * Index: debugging, Macros, asserts
 *
 * -----------------------------------------------------------------------------
*/
#ifdef __C2MAN__    /* Pretend we are a ".c" file to document MACROS. */
    void ASSERT(int boolean_expression);
#else
#ifdef DEFINE_ASSERTS
#define ASSERT_IS_NOT_LESS_INT(x,y)  \
if (x < y)  \
{ \
    dbp("====================================="); \
    dbp("ASSERT_IS_NOT_LESS_INT failed"); \
    dbi(x); \
    dbi(y); \
    term_beep_beep(3, 1000); \
    kjb_abort(); \
    dbp("====================================="); \
}
#else
#define ASSERT_IS_NOT_LESS_INT(x,y)
#endif
#endif


/* =============================================================================
 *                             ASSERT_IS_NUMBER_DBL
 *
 * (MACRO) Calls kjb_abort if double precesion argument is NaN
 *
 * This macro only does something in development code. In production code (i.e.
 * code compiled without -DTEST, this macro evaluates to null.
 *
 * If the double precesion argument is not a number (NaN), then this macro
 * prints the assertion, together with the file and line number, on standard
 * error. It then calls kjb_abort.
 *
 * Index: debugging, Macros, asserts
 *
 * -----------------------------------------------------------------------------
*/
#ifdef __C2MAN__    /* Pretend we are a ".c" file to document MACROS. */
    void ASSERT_IS_NUMBER_DBL(double x);
#else
#ifdef DEFINE_ASSERTS
#define ASSERT_IS_NUMBER_DBL(x)  \
if (isnand(x))  \
{ \
    dbp("====================================="); \
    dbp("ASSERT_IS_NUMBER_DBL failed"); \
    dbe(x); \
    dbx(*((int*)&x)); \
    dbx(*(1 + (int*)&x)); \
    term_beep_beep(3, 1000); \
    kjb_abort(); \
    dbp("====================================="); \
}
#else
#define ASSERT_IS_NUMBER_DBL(x)
#endif
#endif


/* =============================================================================
 *                             ASSERT_IS_FINITE_DBL
 *
 * (MACRO) Calls kjb_abort if double precesion argument is Inf
 *
 * This macro only does something in development code. In production code (i.e.
 * code compiled without -DTEST, this macro evaluates to null.
 *
 * If the double precesion argument is infinite (Inf), then this macro prints
 * the assertion, together with the file and line number, on standard error. It
 * then calls kjb_abort.
 *
 * Index: debugging, Macros, asserts
 *
 * -----------------------------------------------------------------------------
*/
#ifdef __C2MAN__    /* Pretend we are a ".c" file to document MACROS. */
    void ASSERT_IS_FINITE_DBL(double x);
#else
#ifdef DEFINE_ASSERTS
#define ASSERT_IS_FINITE_DBL(x)  \
if (!isfinited(x))  \
{ \
    dbp("====================================="); \
    dbp("ASSERT_IS_FINITE_DBL failed"); \
    dbe(x); \
    term_beep_beep(3, 1000); \
    kjb_abort(); \
    dbp("====================================="); \
}
#else
#define ASSERT_IS_FINITE_DBL(x)
#endif
#endif


/* =============================================================================
 *                             ASSERT_IS_NON_NEGATIVE_DBL
 *
 * (MACRO) Calls kjb_abort if argument is negative
 *
 * This macro only does something in development code. In production code (i.e.
 * code compiled without -DTEST, this macro evaluates to null.
 *
 * If the double precision argument is negative, then this macro prints the
 * assertion, together with the file and line number, on standard error. It then
 * calls kjb_abort.
 *
 * Index: debugging, Macros, asserts
 *
 * -----------------------------------------------------------------------------
*/
#ifdef __C2MAN__    /* Pretend we are a ".c" file to document MACROS. */
    void ASSERT_IS_NON_NEGATIVE_DBL(double x);
#else
#ifdef DEFINE_ASSERTS
#define ASSERT_IS_NON_NEGATIVE_DBL(x)  \
if (x < 0.0)  \
{ \
    dbp("====================================="); \
    dbp("ASSERT_IS_NON_NEGATIVE_DBL failed"); \
    dbe(x); \
    term_beep_beep(3, 1000); \
    kjb_abort(); \
    dbp("====================================="); \
}
#else
#define ASSERT_IS_NON_NEGATIVE_DBL(x)
#endif
#endif


/* =============================================================================
 *                             ASSERT_IS_ZERO_DBL
 *
 * (MACRO) Calls kjb_abort if argument is not approximately zero
 *
 * This macro only does something in development code. In production code (i.e.
 * code compiled without -DTEST, this macro evaluates to null.
 *
 * If the double precision argument is approximately zero, then this macro
 * prints the assertion, together with the file and line number, on standard
 * error. It then calls kjb_abort.
 *
 * Index: debugging, Macros, asserts
 *
 * -----------------------------------------------------------------------------
*/
#ifdef __C2MAN__    /* Pretend we are a ".c" file to document MACROS. */
    void ASSERT_IS_ZERO_DBL(double x);
#else
#ifdef DEFINE_ASSERTS
#define ASSERT_IS_ZERO_DBL(x)  \
if (! IS_ZERO_DBL(x))  \
{ \
    dbp("====================================="); \
    dbp("ASSERT_IS_ZERO_DBL failed"); \
    dbe(x); \
    term_beep_beep(3, 1000); \
    kjb_abort(); \
    dbp("====================================="); \
}
#else
#define ASSERT_IS_ZERO_DBL(x)
#endif
#endif


/* =============================================================================
 *                             ASSERT_IS_NOT_ZERO_DBL
 *
 * (MACRO) Calls kjb_abort if argument is not approximately zero
 *
 * This macro only does something in development code. In production code (i.e.
 * code compiled without -DTEST, this macro evaluates to null.
 *
 * If the double precision argument is not approximately zero, then this macro
 * prints the assertion, together with the file and line number, on standard
 * error. It then calls kjb_abort.
 *
 * Index: debugging, Macros, asserts
 *
 * -----------------------------------------------------------------------------
*/
#ifdef __C2MAN__    /* Pretend we are a ".c" file to document MACROS. */
    void ASSERT_IS_NOT_ZERO_DBL(double x);
#else
#ifdef DEFINE_ASSERTS
#define ASSERT_IS_NOT_ZERO_DBL(x)  \
if (IS_ZERO_DBL(x))  \
{ \
    dbp("====================================="); \
    dbp("ASSERT_IS_ZERO_DBL failed"); \
    dbe(x); \
    term_beep_beep(3, 1000); \
    kjb_abort(); \
    dbp("====================================="); \
}
#else
#define ASSERT_IS_NOT_ZERO_DBL(x)
#endif
#endif


/* =============================================================================
 *                             ASSERT_IS_POSITIVE_DBL
 *
 * (MACRO) Calls kjb_abort if argument is zero or less
 *
 * This macro only does something in development code. In production code (i.e.
 * code compiled without -DTEST, this macro evaluates to null.
 *
 * If the double precision argument is zero or less, then this macro prints the
 * assertion, together with the file and line number, on standard error. It then
 * calls kjb_abort.
 *
 * Index: debugging, Macros, asserts
 *
 * -----------------------------------------------------------------------------
*/
#ifdef __C2MAN__    /* Pretend we are a ".c" file to document MACROS. */
    void ASSERT_IS_POSITIVE_DBL(double x);
#else
#ifdef DEFINE_ASSERTS
#define ASSERT_IS_POSITIVE_DBL(x)  \
if (x <= 0.0)  \
{ \
    dbp("====================================="); \
    dbp("ASSERT_IS_POSITIVE_DBL failed"); \
    dbe(x); \
    term_beep_beep(3, 1000); \
    kjb_abort(); \
    dbp("====================================="); \
}
#else
#define ASSERT_IS_POSITIVE_DBL(x)
#endif
#endif


/* =============================================================================
 *                             ASSERT_IS_PROB_DBL
 *
 * (MACRO) Calls kjb_abort if argument is not a valid probability
 *
 * This macro only does something in development code. In production code (i.e.
 * code compiled without -DTEST, this macro evaluates to null.
 *
 * If the double precision argument is outside [0,1] then this macro prints the
 * assertion, together with the file and line number, on standard error. It then
 * calls kjb_abort.
 *
 * Index: debugging, Macros, asserts
 *
 * -----------------------------------------------------------------------------
*/
#ifdef __C2MAN__    /* Pretend we are a ".c" file to document MACROS. */
    void ASSERT_IS_PROB_DBL(double x);
#else
#ifdef DEFINE_ASSERTS
#define ASSERT_IS_PROB_DBL(x)  \
if ((x > 1.0) || (x < 0.0))  \
{ \
    dbp("====================================="); \
    dbp("ASSERT_IS_PROB_DBL failed"); \
    dbe(x); \
    term_beep_beep(3, 1000); \
    kjb_abort(); \
    dbp("====================================="); \
}
#else
#define ASSERT_IS_PROB_DBL(x)
#endif
#endif


/* =============================================================================
 *                             ASSERT_IS_EQUAL_DBL
 *
 * (MACRO) Calls kjb_abort if arguments are not almost exactly equal
 *
 * This macro only does something in development code. In production code (i.e.
 * code compiled without -DTEST, this macro evaluates to null.
 *
 * If the double precision arguments are not eqaul, then this macro prints the
 * assertion, together with the file and line number, on standard error. It then
 * calls kjb_abort.
 *
 * This assert only makes sense if the numbers are meant to be exactly equal.
 * Otherwise, use IS_NEARLY_EQUAL_DBL().
 *
 * Index: debugging, Macros, asserts
 *
 * -----------------------------------------------------------------------------
*/
#ifdef __C2MAN__    /* Pretend we are a ".c" file to document MACROS. */
    void ASSERT_IS_EQUAL_DBL(double x, double y);
#else
#ifdef DEFINE_ASSERTS
#define ASSERT_IS_EQUAL_DBL(x,y)  \
if ( ! IS_EQUAL_DBL(x,y))  \
{ \
    dbp("====================================="); \
    dbp("ASSERT_IS_EQUAL_DBL failed"); \
    dbe(x); \
    dbe(y); \
    dbe(x - y); \
    term_beep_beep(3, 1000); \
    kjb_abort(); \
    dbp("====================================="); \
}
#else
#define ASSERT_IS_EQUAL_DBL(x,y)
#endif
#endif


/* =============================================================================
 *                             ASSERT_IS_NEARLY_EQUAL_DBL
 *
 * (MACRO) Calls kjb_abort if arguments are not equal within tolerence
 *
 * This macro only does something in development code. In production code (i.e.
 * code compiled without -DTEST, this macro evaluates to null.
 *
 * If the first two arguments are not nearly equal given the third, relative
 * precision argument, then this macro prints the assertion, together with the
 * file and line number, on standard error. It then calls kjb_abort.
 *
 * Index: debugging, Macros, asserts
 *
 * -----------------------------------------------------------------------------
*/
#ifdef __C2MAN__    /* Pretend we are a ".c" file to document MACROS. */
    void ASSERT(double x, double y, double relative_error);
#else
#ifdef DEFINE_ASSERTS
#define ASSERT_IS_NEARLY_EQUAL_DBL(x,y,e)  \
if ( ! IS_NEARLY_EQUAL_DBL(x,y,e))  \
{ \
    dbp("====================================="); \
    dbp("ASSERT_IS_NEARLY_EQUAL_DBL failed"); \
    dbe(x); \
    dbe(y); \
    dbe(x - y); \
    term_beep_beep(3, 1000); \
    kjb_abort(); \
    dbp("====================================="); \
}
#else
#define ASSERT_IS_NEARLY_EQUAL_DBL(x,y,e)
#endif
#endif


/* =============================================================================
 *                             ASSERT_IS_NOT_EQUAL_DBL
 *
 * (MACRO) Calls kjb_abort if argument arguments are nearly equal
 *
 * This macro only does something in development code. In production code (i.e.
 * code compiled without -DTEST, this macro evaluates to null.
 *
 * If the two arguments are equal, then this macro prints the assertion,
 * together with the file and line number, on standard error. It then calls
 * kjb_abort.
 *
 * Index: debugging, Macros, asserts
 *
 * -----------------------------------------------------------------------------
*/
#ifdef __C2MAN__    /* Pretend we are a ".c" file to document MACROS. */
    void ASSERT(double x, double y);
#else
#ifdef DEFINE_ASSERTS
#define ASSERT_IS_NOT_EQUAL_DBL(x,y)  \
if (IS_EQUAL_DBL(x,y))  \
{ \
    dbp("====================================="); \
    dbp("ASSERT_IS_NOT_EQUAL_DBL failed"); \
    dbe(x); \
    dbe(y); \
    term_beep_beep(3, 1000); \
    kjb_abort(); \
    dbp("====================================="); \
}
#else
#define ASSERT_IS_NOT_EQUAL_DBL(x,y)
#endif
#endif


/* =============================================================================
 *                             ASSERT_IS_GREATER_DBL
 *
 * (MACRO) Calls kjb_abort if first argument is less than or equal the second
 *
 * This macro only does something in development code. In production code (i.e.
 * code compiled without -DTEST, this macro evaluates to null.
 *
 * If the first double precision argument is less than or equal the second, then
 * this macro prints the assertion, together with the file and line number, on
 * standard error. It then calls kjb_abort.
 *
 * Index: debugging, Macros, asserts
 *
 * -----------------------------------------------------------------------------
*/
#ifdef __C2MAN__    /* Pretend we are a ".c" file to document MACROS. */
    void ASSERT_IS_GREATER_DBL(double x, double y);
#else
#ifdef DEFINE_ASSERTS
#define ASSERT_IS_GREATER_DBL(x,y)  \
if (x <= y)  \
{ \
    dbp("====================================="); \
    dbp("ASSERT_IS_GREATER_DBL failed"); \
    dbe(x); \
    dbe(y); \
    term_beep_beep(3, 1000); \
    kjb_abort(); \
    dbp("====================================="); \
}
#else
#define ASSERT_IS_GREATER_DBL(x,y)
#endif
#endif


/* =============================================================================
 *                             ASSERT_IS_LESS_DBL
 *
 * (MACRO) Calls kjb_abort if first argument is greater than or equal the second
 *
 * This macro only does something in development code. In production code (i.e.
 * code compiled without -DTEST, this macro evaluates to null.
 *
 * If the first double precision argument is greater than or equal the secon,
 * then this macro prints the assertion, together with the file and line number,
 * on standard error. It then calls kjb_abort.
 *
 * Index: debugging, Macros, asserts
 *
 * -----------------------------------------------------------------------------
*/
#ifdef __C2MAN__    /* Pretend we are a ".c" file to document MACROS. */
    void ASSERT_IS_LESS_DBL(double x, double y);
#else
#ifdef DEFINE_ASSERTS
#define ASSERT_IS_LESS_DBL(x,y)  \
if (x >= y)  \
{ \
    dbp("====================================="); \
    dbp("ASSERT_IS_LESS_DBL failed"); \
    dbe(x); \
    dbe(y); \
    term_beep_beep(3, 1000); \
    kjb_abort(); \
    dbp("====================================="); \
}
#else
#define ASSERT_IS_LESS_DBL(x,y)
#endif
#endif


/* =============================================================================
 *                             ASSERT_IS_NOT_GREATER_DBL
 *
 * (MACRO) Calls kjb_abort if first argument is greater than the second
 *
 * This macro only does something in development code. In production code (i.e.
 * code compiled without -DTEST, this macro evaluates to null.
 *
 * If the first double precision argument is greater than the second, then this
 * macro prints the assertion, together with the file and line number, on
 * standard error. It then calls kjb_abort.
 *
 * Index: debugging, Macros, asserts
 *
 * -----------------------------------------------------------------------------
*/
#ifdef __C2MAN__    /* Pretend we are a ".c" file to document MACROS. */
    void ASSERT_IS_NOT_GREATER_DBL(double x, double y);
#else
#ifdef DEFINE_ASSERTS
#define ASSERT_IS_NOT_GREATER_DBL(x,y)  \
if (x > y)  \
{ \
    dbp("====================================="); \
    dbp("ASSERT_IS_NOT_GREATER_DBL failed"); \
    dbe(x); \
    dbe(y); \
    term_beep_beep(3, 1000); \
    kjb_abort(); \
    dbp("====================================="); \
}
#else
#define ASSERT_IS_NOT_GREATER_DBL(x,y)
#endif
#endif


/* =============================================================================
 *                             ASSERT_IS_NOT_LESS_DBL
 *
 * (MACRO) Calls kjb_abort if first argument is less than the second
 *
 * This macro only does something in development code. In production code (i.e.
 * code compiled without -DTEST, this macro evaluates to null.
 *
 * If the first double precision argument is less than second, then
 * this macro prints the assertion, together with the file and line number, on
 * standard error. It then calls kjb_abort.
 *
 * Index: debugging, Macros, asserts
 *
 * -----------------------------------------------------------------------------
*/
#ifdef __C2MAN__    /* Pretend we are a ".c" file to document MACROS. */
    void ASSERT_IS_NOT_LESS_DBL(int boolean_expression);
#else
#ifdef DEFINE_ASSERTS
#define ASSERT_IS_NOT_LESS_DBL(x,y)  \
if (x < y)  \
{ \
    dbp("====================================="); \
    dbp("ASSERT_IS_NOT_LESS_DBL failed"); \
    dbe(x); \
    dbe(y); \
    term_beep_beep(3, 1000); \
    kjb_abort(); \
    dbp("====================================="); \
}
#else
#define ASSERT_IS_NOT_LESS_DBL(x,y)
#endif
#endif


#else

#ifdef DEFINE_ASSERTS
#define ASSERT_IS_BOOL(x) ASSERT((x==0) ||  (x==1))
#else
#define  ASSERT_IS_BOOL(x)
#endif

#ifdef DEFINE_ASSERTS
#define ASSERT_IS_EQUAL_INT(x,y) ASSERT(x==y)
#else
#define  ASSERT_IS_EQUAL_INT(x,y)
#endif

#ifdef DEFINE_ASSERTS
#define ASSERT_IS_NON_NEGATIVE_INT(x) ASSERT(x >= 0)
#else
#define ASSERT_IS_NON_NEGATIVE_INT(x)
#endif

#ifdef DEFINE_ASSERTS
#define ASSERT_IS_POSITIVE_INT(x) ASSERT(x > 0)
#else
#define ASSERT_IS_POSITIVE_INT(x)
#endif

#ifdef DEFINE_ASSERTS
#define ASSERT_IS_NOT_EQUAL_INT(x,y) ASSERT( ! IS_EQUAL_INT(x,y))
#else
#define ASSERT_IS_NOT_EQUAL_INT(x,y)
#endif

#ifdef DEFINE_ASSERTS
#define ASSERT_IS_GREATER_INT(x,y) ASSERT(x > y)
#else
#define ASSERT_IS_GREATER_INT(x,y)
#endif

#ifdef DEFINE_ASSERTS
#define ASSERT_IS_LESS_INT(x,y) ASSERT(x < y)
#else
#define ASSERT_IS_LESS_INT(x,y)
#endif

#ifdef DEFINE_ASSERTS
#define ASSERT_IS_NOT_GREATER_INT(x,y) ASSERT(x <= y)
#else
#define ASSERT_IS_NOT_GREATER_INT(x,y)
#endif

#ifdef DEFINE_ASSERTS
#define ASSERT_IS_NOT_LESS_INT(x,y)  ASSERT(x >= y)
#else
#define ASSERT_IS_NOT_LESS_INT(x,y)
#endif


#ifdef DEFINE_ASSERTS
#define ASSERT_IS_NUMBER_DBL(x) ASSERT(!isnand(x))
#else
#define ASSERT_IS_NUMBER_DBL(x)
#endif


#ifdef DEFINE_ASSERTS
#define ASSERT_IS_FINITE_DBL(x) ASSERT(isfinited(x))
#else
#define ASSERT_IS_FINITE_DBL(x)
#endif

#ifdef DEFINE_ASSERTS
#define ASSERT_IS_NON_NEGATIVE_DBL(x) ASSERT(x >= 0.0)
#else
#define ASSERT_IS_NON_NEGATIVE_DBL(x)
#endif

#ifdef DEFINE_ASSERTS
#define ASSERT_IS_POSITIVE_DBL(x) ASSERT(x > 0.0)
#else
#define ASSERT_IS_POSITIVE_DBL(x)
#endif

#ifdef DEFINE_ASSERTS
#define ASSERT_IS_ZERO_DBL(x) ASSERT(IS_ZERO_DBL(x))
#else
#define ASSERT_IS_ZERO_DBL(x)
#endif

#ifdef DEFINE_ASSERTS
#define ASSERT_IS_NOT_ZERO_DBL(x) ASSERT( ! IS_ZERO_DBL(x))
#else
#define ASSERT_IS_NOT_ZERO_DBL(x)
#endif

#ifdef DEFINE_ASSERTS
#define ASSERT_IS_PROB_DBL(x) ASSERT((x >= 0.0) && (x <= 1.0))
#else
#define ASSERT_IS_PROB_DBL(x)
#endif

#ifdef DEFINE_ASSERTS
#define ASSERT_IS_EQUAL_DBL(x,y) ASSERT(IS_EQUAL_DBL(x,y))
#else
#define ASSERT_IS_EQUAL_DBL(x,y)
#endif

#ifdef DEFINE_ASSERTS
#define ASSERT_IS_NOT_EQUAL_DBL(x,y) ASSERT( ! IS_EQUAL_DBL(x,y))
#else
#define ASSERT_IS_NOT_EQUAL_DBL(x,y)
#endif

#ifdef DEFINE_ASSERTS
#define ASSERT_IS_GREATER_DBL(x,y) ASSERT(x > y)
#else
#define ASSERT_IS_GREATER_DBL(x,y)
#endif

#ifdef DEFINE_ASSERTS
#define ASSERT_IS_LESS_DBL(x,y) ASSERT(x < y)
#else
#define ASSERT_IS_LESS_DBL(x,y)
#endif

#ifdef DEFINE_ASSERTS
#define ASSERT_IS_NOT_GREATER_DBL(x,y) ASSERT(x <= y)
#else
#define ASSERT_IS_NOT_GREATER_DBL(x,y)
#endif

#ifdef DEFINE_ASSERTS
#define ASSERT_IS_NOT_LESS_DBL(x,y)  ASSERT(x >= y)
#else
#define ASSERT_IS_NOT_LESS_DBL(x,y)
#endif


#endif


/* -------------------------------------------------------------------------- */

#endif


