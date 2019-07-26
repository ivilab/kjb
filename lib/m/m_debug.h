
/* $Id: m_debug.h 21614 2017-08-03 17:57:12Z kobus $ */

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

#ifndef M_DEBUG_INCLUDED
#define M_DEBUG_INCLUDED


#include "m/m_gen.h"
#include "m/m_mat_io.h"

#ifdef __cplusplus
extern "C" {
#ifdef COMPILING_CPLUSPLUS_SOURCE
namespace kjb_c {
#endif
#endif


/* -------------------------------------------------------------------------- */

IMPORT int kjb_debug_level;

/* -------------------------------------------------------------------------- */

/* =============================================================================
 *                             db_rv
 *
 * (MACRO) Debug printing of a vector
 *
 * This macro prints a vector as a row vector, along with the variable name,
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
 * Related:
 *    dbm, dbo, dbp, dbx, dbc, dbf, dbi, dbr, dbw, dbe, dbs
 *
 * Index: debugging
 *
 * -----------------------------------------------------------------------------
*/

#ifdef __C2MAN__
    void db_rv(const Vector *vp);
#else

#ifdef HASH_MARK_SUBSTITUTION_OK

#define db_rv(X)                                                               \
    if (kjb_debug_level >= 1)                                                  \
    {                                                                          \
        EPE(fp_write_row_vector_with_title(X, stderr, ""#X""));                   \
    }


#else    /*   #ifdef HASH_MARK_SUBSTITUTION_OK ....     */

#define db_rv(X)                                                               \
    if (kjb_debug_level >= 1)                                                  \
    {                                                                          \
        EPE(fp_write_row_vector_with_title(X, stderr, "X"));                      \
    }


#endif    /*   #ifdef HASH_MARK_SUBSTITUTION_OK ... #else  .....   */

#endif    /*   #ifdef __C2MAN__ */

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                             db_cv
 *
 * (MACRO) Debug printing of a vector
 *
 * This macro prints a vector as a column vector, along with the variable name,
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
 * Related:
 *    dbm, dbo, dbp, dbx, dbc, dbf, dbi, dbr, dbw, dbe, dbs
 *
 * Index: debugging
 *
 * -----------------------------------------------------------------------------
*/

#ifdef __C2MAN__
    void db_cv(const Vector *vp);
#else

#ifdef HASH_MARK_SUBSTITUTION_OK

#define db_cv(X)                                                               \
    if (kjb_debug_level >= 1)                                                  \
    {                                                                          \
        fp_write_col_vector_with_title(X, stderr, ""#X"");                        \
    }

#else    /*   #ifdef HASH_MARK_SUBSTITUTION_OK ....     */

#define db_cv(X)                                                               \
   if (kjb_debug_level >= 1)                                                   \
   {                                                                           \
       fp_write_col_vector_with_title(X, stderr, "X");                            \
   }

#endif    /*   #ifdef HASH_MARK_SUBSTITUTION_OK ... #else  .....   */

#endif    /*   #ifdef __C2MAN__ */

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                             file_db_cv
 *
 * (MACRO) Debug printing of a vector to a file
 *
 * This macro prints a vector as a column vector, to a file in the current
 * directory determined by the variable name.  Its behaviour is a function of
 * the debugging level, which can be set through the option "debug" if the KJB
 * library options are being made available to the user. The macro is available
 * for both development and production code, but since the default debug level
 * is different in the two cases, the behaviour is different. The default level
 * for development code is 2, whereas for production code it is 0.
 *
 * If the debug level is 1 (or more), then the matrix is output to a file which
 * has the same name as the variable.
 *
 * Notes:
 *    This macro is only avaible with compilers which have "hash mark
 *    substitution. With compilers that don't, a message to this effect is
 *    printed instead.
 *
 * Related:
 *    dbm, dbo, dbp, dbx, dbc, dbf, dbi, dbr, dbw, dbe, dbs
 *
 * Index: debugging
 *
 * -----------------------------------------------------------------------------
*/

#ifdef __C2MAN__
    void file_db_cv(const Vector *vp);
#else

#ifdef HASH_MARK_SUBSTITUTION_OK

#define file_db_cv(X)                                                               \
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
            EPE(fp_write_col_vector_with_title(X, fp, ""#X""));                   \
            EPE(kjb_fclose(fp));                                               \
        }                                                                      \
    }

#else    /*   #ifdef HASH_MARK_SUBSTITUTION_OK ....     */

#define file_db_cv(X)                                                          \
    p_stderr("Macro file_db_cv() is not availblae with this compiler.\n");

#endif    /*   #ifdef HASH_MARK_SUBSTITUTION_OK ... #else  .....   */

#endif    /*   #ifdef __C2MAN__ */

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                             db_mat
 *
 * (MACRO) Debug printing of a matrix
 *
 * This macro prints a matrix, along with the variable name, file, and line
 * number on standard error. Its behaviour is a function of the debugging level,
 * which can be set through the option "debug" if the KJB library options are
 * being made available to the user. The macro is available for both development
 * and production code, but since the default debug level is different in the
 * two cases, the behaviour is different. The default level for development code
 * is 2, whereas for production code it is 0.
 *
 * If the debug level is 1 (or more), then the output is printed to standard
 * error using regular KJB library output routines. Note that this is different
 * from more basic debug macros where a debug level of 1 is used to print using
 * system routines and a debug level of 2 or more is used for printing using KJB
 * library routines. (The special behavour of debug level 1 in the case of the
 * more basic debug macros is provided to help debug the KJB library I/O
 * routines themselve).
 *
 * If the debug level is 0 (default for production code), then the output is
 * disabled.
 *
 * Related:
 *    dbm, dbo, dbp, dbx, dbc, dbf, dbi, dbr, dbw, dbe, dbs
 *
 * Index: debugging
 *
 * -----------------------------------------------------------------------------
*/

#ifdef __C2MAN__
    void db_mat(const Matrix *mp);
#else

#ifdef HASH_MARK_SUBSTITUTION_OK

#define db_mat(X)                                                              \
    if (kjb_debug_level >= 1)                                                  \
    {                                                                          \
        EPE(fp_write_matrix_with_title(X, stderr, ""#X""));                    \
    }

#else    /*   #ifdef HASH_MARK_SUBSTITUTION_OK ....     */

#define db_mat(X)                                                              \
    if (kjb_debug_level >= 1)                                                  \
    {                                                                          \
        EPE(fp_write_matrix_with_title(X, stderr, "X"));                          \
    }

#endif    /*   #ifdef HASH_MARK_SUBSTITUTION_OK ... #else  .....   */

#endif    /*   #ifdef __C2MAN__ */

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                             file_db_mat
 *
 * (MACRO) Debug printing of a matrix
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
 * has the same name as the variable. If the debug level is 3 (or more) the
 * output is full precision.
 *
 * Notes:
 *    This macro is only avaible with compilers which have "hash mark
 *    substitution. With compilers that don't, a message to this effect is
 *    printed instead.
 *
 * Related:
 *    db_mat, db_rv, db_cv, dbm, dbo, dbp, dbx, dbc, dbf, dbi, dbr, dbw, dbe, dbs
 *
 * Index: debugging
 *
 * -----------------------------------------------------------------------------
*/

#ifdef __C2MAN__
    void file_db_mat(const Matrix *mp);
#else

#ifdef HASH_MARK_SUBSTITUTION_OK

#define file_db_mat(X)                                                         \
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
                EPE(fp_write_matrix_full_precision_with_title(X, fp, ""#X""));    \
            }                                                                  \
            else                                                               \
            {                                                                  \
                EPE(fp_write_matrix_with_title(X, fp, ""#X""));                   \
            }                                                                  \
            EPE(kjb_fclose(fp));                                               \
        }                                                                      \
    }

#else    /*   #ifdef HASH_MARK_SUBSTITUTION_OK ....     */

#define file_db_mat(X)                                                         \
    p_stderr("Macro file_db_mat() is not available with this compiler.\n");

#endif    /*   #ifdef HASH_MARK_SUBSTITUTION_OK ... #else  .....   */

#endif    /*   #ifdef __C2MAN__ */

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */


#ifdef __cplusplus
#ifdef COMPILING_CPLUSPLUS_SOURCE
}
#endif
}
#endif

#endif


