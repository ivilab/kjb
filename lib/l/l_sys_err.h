
/* $Id: l_sys_err.h 21310 2017-03-19 20:58:40Z kobus $ */

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

#ifndef L_SYSTEM_ERROR_INCLUDED
#define L_SYSTEM_ERROR_INCLUDED


#include "l/l_def.h"

#ifdef __cplusplus
extern "C" {
#ifdef COMPILING_CPLUSPLUS_SOURCE
namespace kjb_c {
#endif
#endif


typedef enum Error_action
{
    SET_ERROR_ON_ERROR,
    IGNORE_ERROR_ON_ERROR,
    FORCE_ADD_ERROR_ON_ERROR
}
Error_action;


/*
// We may undef this for products which ship to to the masse, or to save a few
// bytes and cycles, but for everyday use, it is best to report everthing, even
// if it is a "production" version.
*/

#ifndef REPORT_ALL_BUG_INFO
#    define REPORT_ALL_BUG_INFO
#endif

/*
// Even if not defined above (to specify production behaviour), if TEST is
// defined, we need REPORT_ALL_BUG_INFO defined regardless.
*/
#ifdef TEST
#    ifndef REPORT_ALL_BUG_INFO
#        define REPORT_ALL_BUG_INFO
#    endif
#endif


/*
// In order to maintain proper message reporting under bad circumstances,
// (EG no heap space left), the storage for messages is allocated statically
// in advance. The maximun number of message strings is set here, as is the
// maximum size of each string.
*/

#define MAX_NUM_ERROR_MESS          1000        /* MAGIC NUMBER */
#define ERROR_MESS_BUFF_SIZE        2000       /* MAGIC NUMBER */

/* -------------------------------------------------------------------------- */

/* =============================================================================
 *                             SET_BUFFER_OVERFLOW_BUG
 *
 * (MACRO) Sets up call to set_bug
 *
 * This macro sets up the call to set_bug when a problem can be described as
 * an attempt to overflow a buffer. In development code (i.e. when TEST is
 * defined), a generic message this condition is passed to the routine
 * set_bug() together with the file and line number. In non-development code, a
 * more user oriented message is passed (depending on the setting of the symbol
 * "REPORT_ALL_BUG_INFO)". Note that the behaviour of set_bug() itself is
 * dependent on the bug_handler, and the behaviour default bug handler is
 * dependent on whether or not the code is development code.
 *
 * Note :
 *    It is advisable to expect SET_BUFFER_OVERFLOW_BUG to return, even though
 *    under many circumstances it will not. Normally the statement after
 *    SET_BUFFER_OVERFLOW_BUG is an error return.
 *
 * Related:
 *   set_bug, set_bug_handler, default_bug_handler
 *
 * Index: debugging, Macros
 *
 * -----------------------------------------------------------------------------
*/

#ifdef __C2MAN__    /* Pretend we are a ".c" file to document MACROS. */

    void SET_BUFFER_OVERFLOW_BUG(void);

#else
#ifdef REPORT_ALL_BUG_INFO
#    define SET_BUFFER_OVERFLOW_BUG() \
                         test_set_buffer_overflow_bug(__LINE__, __FILE__)
#else
#    define SET_BUFFER_OVERFLOW_BUG()   set_buffer_overflow_bug()
#endif
#endif

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                             SET_FORMAT_STRING_BUG
 *
 * (MACRO) Sets up call to set_bug
 *
 * This macro sets up the call to set_bug when the problem can be described as
 * an attempt to use an invalid format string. In development code (i.e. when
 * TEST is defined), a generic message this condition is passed to the routine
 * set_bug together with the file and line number . In non-development code, a
 * more user oriented message is passed (depending on the setting of the symbol
 * REPORT_ALL_BUG_INFO). Note that the behaviour of set_bug() itself is
 * dependent on the bug_handler, and the behaviour default bug handler is
 * dependent on whether or not the code is development code.
 *
 * Note :
 *    It is advisable to expect SET_FORMAT_STRING_BUG to return, even though
 *    under many circumstances it will not. Normally the statement after
 *    SET_FORMAT_STRING_BUG is an error return.
 *
 * Related:
 *   set_bug, set_bug_handler, default_bug_handler
 *
 * Index: debugging, Macros
 *
 * -----------------------------------------------------------------------------
*/

#ifdef __C2MAN__    /* Pretend we are a ".c" file to document MACROS. */

    void SET_FORMAT_STRING_BUG(void);

#else
#ifdef REPORT_ALL_BUG_INFO
#    define SET_FORMAT_STRING_BUG() \
                         test_set_format_string_bug(__LINE__, __FILE__)

#else
#    define SET_FORMAT_STRING_BUG()     set_format_string_bug()
#endif
#endif

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                             SET_ARGUMENT_BUG
 *
 * (MACRO) Sets up call to set_bug
 *
 * This macro sets up the call to set_bug when a problem can be described as
 * an attempt call a function with an invalid argument. In development code
 * (i.e. when TEST is defined), a generic message this condition is passed to
 * the routine set_bug() together with the file and line number. In
 * non-development code, a more user oriented message is passed (depending on
 * the setting of the symbol REPORT_ALL_BUG_INFO). Note that the behaviour of
 * set_bug() itself is dependent on the bug_handler, and the behaviour default
 * bug handler is dependent on whether or not the code is development code.
 *
 * Note :
 *    It is advisable to expect SET_ARGUMENT_BUG to return, even though
 *    under many circumstances it will not. Normally the statement after
 *    SET_ARGUMENT_BUG is an error return.
 *
 * Related:
 *   set_bug, set_bug_handler, default_bug_handler
 *
 * Index: debugging, Macros
 *
 * -----------------------------------------------------------------------------
*/

#ifdef __C2MAN__    /* Pretend we are a ".c" file to document MACROS. */

    void SET_ARGUMENT_BUG(void);

#else
#ifdef REPORT_ALL_BUG_INFO
#    define SET_ARGUMENT_BUG() \
                         test_set_argument_bug(__LINE__, __FILE__)

#else
#    define SET_ARGUMENT_BUG()          set_argument_bug()
#endif
#endif

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                             SET_NAN_DOUBLE_BUG
 *
 * (MACRO) Sets up call to set_bug
 *
 * This macro sets up the call to set_bug when a problem can be described as
 * noting that what should be a valid double is NaN. In development code
 * (i.e. when TEST is defined), a generic message for this condition is passed to
 * the routine set_bug() together with the file and line number. In
 * non-development code, a more user oriented message is passed (depending on
 * the setting of the symbol REPORT_ALL_BUG_INFO). Note that the behaviour of
 * set_bug() itself is dependent on the bug_handler, and the behaviour default
 * bug handler is dependent on whether or not the code is development code.
 *
 * Note :
 *    It is advisable to expect SET_NAN_DOUBLE_BUG to return, even though
 *    under many circumstances it will not. Normally the statement after
 *    SET_NAN_DOUBLE_BUG is an error return or program exit.
 *
 * Related:
 *   set_bug, set_bug_handler, default_bug_handler
 *
 * Index: debugging, Macros
 *
 * -----------------------------------------------------------------------------
*/

#ifdef __C2MAN__    /* Pretend we are a ".c" file to document MACROS. */

    void SET_NAN_DOUBLE_BUG(void);

#else
#ifdef REPORT_ALL_BUG_INFO
#    define SET_NAN_DOUBLE_BUG() \
                         test_set_nan_double_bug(__LINE__, __FILE__)

#else
#    define SET_NAN_DOUBLE_BUG()    set_nan_double_bug()
#endif
#endif

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                             SET_INFINITE_DOUBLE_BUG
 *
 * (MACRO) Sets up call to set_bug
 *
 * This macro sets up the call to set_bug when a problem can be described as
 * noting that what should be a valid double is infinite. In development code
 * (i.e. when TEST is defined), a generic message for this condition is passed to
 * the routine set_bug() together with the file and line number. In
 * non-development code, a more user oriented message is passed (depending on
 * the setting of the symbol REPORT_ALL_BUG_INFO). Note that the behaviour of
 * set_bug() itself is dependent on the bug_handler, and the behaviour default
 * bug handler is dependent on whether or not the code is development code.
 *
 * Note :
 *    It is advisable to expect SET_INFINITE_DOUBLE_BUG to return, even though
 *    under many circumstances it will not. Normally the statement after
 *    SET_INFINITE_DOUBLE_BUG is an error return or program exit.
 *
 * Related:
 *   set_bug, set_bug_handler, default_bug_handler
 *
 * Index: debugging, Macros
 *
 * -----------------------------------------------------------------------------
*/

#ifdef __C2MAN__    /* Pretend we are a ".c" file to document MACROS. */

    void SET_INFINITE_DOUBLE_BUG(void);

#else
#ifdef REPORT_ALL_BUG_INFO
#    define SET_INFINITE_DOUBLE_BUG() \
                         test_set_infinite_double_bug(__LINE__, __FILE__)

#else
#    define SET_INFINITE_DOUBLE_BUG()    set_infinite_double_bug()
#endif
#endif

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                             SET_NAN_FLOAT_BUG
 *
 * (MACRO) Sets up call to set_bug
 *
 * This macro sets up the call to set_bug when a problem can be described as
 * noting that what should be a valid float is NaN. In development code
 * (i.e. when TEST is defined), a generic message for this condition is passed to
 * the routine set_bug() together with the file and line number. In
 * non-development code, a more user oriented message is passed (depending on
 * the setting of the symbol REPORT_ALL_BUG_INFO). Note that the behaviour of
 * set_bug() itself is dependent on the bug_handler, and the behaviour default
 * bug handler is dependent on whether or not the code is development code.
 *
 * Note :
 *    It is advisable to expect SET_NAN_FLOAT_BUG to return, even though
 *    under many circumstances it will not. Normally the statement after
 *    SET_NAN_FLOAT_BUG is an error return or program exit.
 *
 * Related:
 *   set_bug, set_bug_handler, default_bug_handler
 *
 * Index: debugging, Macros
 *
 * -----------------------------------------------------------------------------
*/

#ifdef __C2MAN__    /* Pretend we are a ".c" file to document MACROS. */

    void SET_NAN_FLOAT_BUG(void);

#else
#ifdef REPORT_ALL_BUG_INFO
#    define SET_NAN_FLOAT_BUG() \
                         test_set_nan_float_bug(__LINE__, __FILE__)

#else
#    define SET_NAN_FLOAT_BUG()    set_nan_float_bug()
#endif
#endif

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                             SET_INFINITE_FLOAT_BUG
 *
 * (MACRO) Sets up call to set_bug
 *
 * This macro sets up the call to set_bug when a problem can be described as
 * noting that what should be a valid float is infinite. In development code
 * (i.e. when TEST is defined), a generic message for this condition is passed to
 * the routine set_bug() together with the file and line number. In
 * non-development code, a more user oriented message is passed (depending on
 * the setting of the symbol REPORT_ALL_BUG_INFO). Note that the behaviour of
 * set_bug() itself is dependent on the bug_handler, and the behaviour default
 * bug handler is dependent on whether or not the code is development code.
 *
 * Note :
 *    It is advisable to expect SET_INFINITE_FLOAT_BUG to return, even though
 *    under many circumstances it will not. Normally the statement after
 *    SET_INFINITE_FLOAT_BUG is an error return or program exit.
 *
 * Related:
 *   set_bug, set_bug_handler, default_bug_handler
 *
 * Index: debugging, Macros
 *
 * -----------------------------------------------------------------------------
*/

#ifdef __C2MAN__    /* Pretend we are a ".c" file to document MACROS. */

    void SET_INFINITE_FLOAT_BUG(void);

#else
#ifdef REPORT_ALL_BUG_INFO
#    define SET_INFINITE_FLOAT_BUG() \
                         test_set_infinite_float_bug(__LINE__, __FILE__)

#else
#    define SET_INFINITE_FLOAT_BUG()    set_infinite_float_bug()
#endif
#endif

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                             SET_CANT_HAPPEN_BUG
 *
 * (MACRO) Sets up call to set_bug
 *
 * This macro sets up the call to set_bug when a problem can be described as
 * an attempt to execute code which in theory cannot be executed. In
 * development code (i.e. when TEST is defined), a generic message this
 * condition is passed to the routine set_bug() together with the file and line
 * number. In non-development code, a more user oriented message is passed
 * (depending on the setting of the symbol REPORT_ALL_BUG_INFO). Note that the
 * behaviour of set_bug() itself is dependent on the bug_handler, and the
 * behaviour default bug handler is dependent on whether or not the code is
 * development code.
 *
 * Note :
 *    It is advisable to expect SET_CANT_HAPPEN_BUG to return, even though
 *    under many circumstances it will not. Normally the statement after
 *    SET_CANT_HAPPEN_BUG is an error return.
 *
 * Related:
 *   set_bug, set_bug_handler, default_bug_handler
 *
 * Index: debugging, Macros
 *
 * -----------------------------------------------------------------------------
*/

#ifdef __C2MAN__    /* Pretend we are a ".c" file to document MACROS. */

    void SET_CANT_HAPPEN_BUG(void);

#else
#ifdef REPORT_ALL_BUG_INFO
#    define SET_CANT_HAPPEN_BUG() \
                         test_set_cant_happen_bug(__LINE__, __FILE__)

#else
#    define SET_CANT_HAPPEN_BUG()       set_cant_happen_bug()
#endif
#endif

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                             SET_BOUNDS_BUG
 *
 * (MACRO) Sets up call to set_bug
 *
 * This macro sets up the call to set_bug when a problem can be described as
 * an attempt to exceed array bounds. In development code (i.e. when TEST is
 * defined), a generic message this condition is passed to the routine set_bug()
 * together with the file and line number. In non-development code, a more user
 * oriented message is passed (depending on the setting of the symbol
 * REPORT_ALL_BUG_INFO). Note that the behaviour of set_bug() itself is
 * dependent on the bug_handler, and the behaviour default bug handler is
 * dependent on whether or not the code is development code.
 *
 * Note :
 *    It is advisable to expect SET_BOUNDS_BUG to return, even though under many
 *    circumstances it will not. Normally the statement after SET_BOUNDS_BUG is
 *    an error return.
 *
 * Related:
 *   set_bug, set_bug_handler, default_bug_handler
 *
 * Index: debugging, Macros
 *
 * -----------------------------------------------------------------------------
*/

#ifdef __C2MAN__    /* Pretend we are a ".c" file to document MACROS. */

    void SET_BOUNDS_BUG(void);

#else
#ifdef REPORT_ALL_BUG_INFO
#    define SET_BOUNDS_BUG() \
                         test_set_bounds_bug(__LINE__, __FILE__)

#else
#    define SET_BOUNDS_BUG()       set_bounds_bug()
#endif
#endif

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                             SET_SORT_BUG
 *
 * (MACRO) Sets up call to set_bug
 *
 * This macro sets up the call to set_bug when a problem can be described as
 * an object that should be sorted was found out not to be. 
 *
 * Note :
 *    It is advisable to expect SET_SORT_BUG to return, even though under many
 *    circumstances it will not. Normally the statement after SET_SORT_BUG is an
 *    error return.
 *
 * Related:
 *   set_bug, set_bug_handler, default_bug_handler
 *
 * Index: debugging, Macros
 *
 * -----------------------------------------------------------------------------
*/

#ifdef __C2MAN__    /* Pretend we are a ".c" file to document MACROS. */

    void SET_SORT_BUG(void);

#else
#ifdef REPORT_ALL_BUG_INFO
#    define SET_SORT_BUG() \
                         test_set_sort_bug(__LINE__, __FILE__)

#else
#    define SET_SORT_BUG()       set_sort_bug()
#endif
#endif

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                             SET_OVERFLOW_BUG
 *
 * (MACRO) Sets up call to set_bug
 *
 * This macro sets up the call to set_bug when a problem can be described as
 * leading to overflow such as attempting to put a large double into an integer.
 *
 * Note :
 *    It is advisable to expect SET_ARGUMENT_BUG to return, even though
 *    under many circumstances it will not. Normally the statement after
 *    SET_OVERFLOW_BUG is an error return.
 *
 * Related:
 *   set_bug, set_bug_handler, default_bug_handler
 *
 * Index: debugging, Macros
 *
 * -----------------------------------------------------------------------------
*/

#ifdef __C2MAN__    /* Pretend we are a ".c" file to document MACROS. */

    void SET_OVERFLOW_BUG(void);

#else
#ifdef REPORT_ALL_BUG_INFO
#    define SET_OVERFLOW_BUG() \
                         test_set_overflow_bug(__LINE__, __FILE__)

#else
#    define SET_OVERFLOW_BUG()          set_overflow_bug()
#endif
#endif

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                             SET_UNDERFLOW_BUG
 *
 * (MACRO) Sets up call to set_bug
 *
 * This macro sets up the call to set_bug when a problem can be described as
 * leading to underflow such as attempting to put a large double into an integer.
 *
 * Note :
 *    It is advisable to expect SET_ARGUMENT_BUG to return, even though
 *    under many circumstances it will not. Normally the statement after
 *    SET_UNDERFLOW_BUG is an error return.
 *
 * Related:
 *   set_bug, set_bug_handler, default_bug_handler
 *
 * Index: debugging, Macros
 *
 * -----------------------------------------------------------------------------
*/

#ifdef __C2MAN__    /* Pretend we are a ".c" file to document MACROS. */

    void SET_UNDERFLOW_BUG(void);

#else
#ifdef REPORT_ALL_BUG_INFO
#    define SET_UNDERFLOW_BUG() \
                         test_set_underflow_bug(__LINE__, __FILE__)

#else
#    define SET_UNDERFLOW_BUG()          set_underflow_bug()
#endif
#endif

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

void         push_error_action   (Error_action error_action);
void         pop_error_action    (void);
void         set_error_action    (Error_action error_action);
Error_action get_error_action    (void);
void         set_bug_handler     (void (*bug_handler)(const char*));
void         default_bug_handler (const char* message);
void         kjb_print_error     (void);
void         kjb_get_error       (char*, size_t);
int          kjb_get_strlen_error(void);
void         kjb_clear_error     (void);

#ifdef STANDARD_VAR_ARGS
#ifndef SGI   /* SGI peaks, and gives spurious messages. */
    /*PRINTFLIKE1*/
#endif
    void set_error(const char*, ...);

#ifndef SGI   /* SGI peaks, and gives spurious messages. */
    /*PRINTFLIKE1*/
#endif
    void add_error(const char*, ...);

#ifndef SGI   /* SGI peaks, and gives spurious messages. */
    /*PRINTFLIKE1*/
#endif
    void cat_error(const char*, ...);

#ifndef SGI   /* SGI peaks, and gives spurious messages. */
    /*PRINTFLIKE1*/
#endif
    void insert_error(const char*, ...);

#ifndef SGI   /* SGI peaks, and gives spurious messages. */
    /*PRINTFLIKE1*/
#endif
    void set_bug(const char*, ...);
#else
    void set_error(void);
    void add_error(void);
    void cat_error(void);
    void insert_error(void);
    void set_bug(void);
#endif

void str_set_error(const char* str);
void str_cat_error(const char* str);
void str_insert_error(const char* str);
void str_add_error(const char* str);

#ifdef REPORT_ALL_BUG_INFO
    void test_set_buffer_overflow_bug(int, const char*);
    void test_set_format_string_bug(int, const char*);
    void test_set_argument_bug(int, const char*);
    void test_set_infinite_float_bug(int, const char*);
    void test_set_infinite_double_bug(int, const char*);
    void test_set_nan_float_bug(int, const char*);
    void test_set_nan_double_bug(int, const char*);
    void test_set_cant_happen_bug(int, const char*);
    void test_set_bounds_bug(int, const char*);
    void test_set_overflow_bug(int, const char*);
    void test_set_underflow_bug(int, const char*);
    void test_set_sort_bug(int line, const char* file);
#else
    void set_buffer_overflow_bug(void);
    void set_format_string_bug(void);
    void set_argument_bug(void);
    void test_set_infinite_float_bug(void);
    void test_set_infinite_double_bug(void);
    void test_set_nan_float_bug(void);
     void test_set_nan_double_bug(void);
    void set_cant_happen_bug(void);
    void set_bounds_bug(void);
    void set_overflow_bug(void);
    void set_underflow_bug(void);
    void set_sort_bug(void);
#endif


#ifdef __cplusplus
#ifdef COMPILING_CPLUSPLUS_SOURCE
}
#endif
}
#endif

#endif

