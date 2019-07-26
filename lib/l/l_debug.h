
/* $Id: l_debug.h 21597 2017-07-31 00:28:31Z kobus $ */

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

#ifndef L_DEBUG_INCLUDED
#define L_DEBUG_INCLUDED


#include "l/l_def.h"


#ifdef TEST
#ifdef COMPILING_CPLUSPLUS_SOURCE
#    define TEST_PSO(x) kjb_c::test_pso x
#else 
#    define TEST_PSO(x) test_pso x
#endif 
#else
#    define TEST_PSO(x)
#endif


#define TEST_P_STDERR  TEST_PSE

#ifdef TEST
#ifdef COMPILING_CPLUSPLUS_SOURCE
#    define TEST_PSE(x) kjb_c::test_pse x
#else 
#    define TEST_PSE(x) test_pse x
#endif 
#else
#    define TEST_PSE(x)
#endif

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\  */

#ifndef SKIP_FOR_EXPORT
/*
 * SKIP_FOR_EXPORT is never defined, so code below is always active, unless it
 * is stripped out using the script clean_src. 
 *
 * Note that instances of using UNTESTED_CODE() are stripped when we export.
*/

/* ============================================================================
 *                            UNTESTED_CODE
 *
 * (MACRO) Prints a warning message, but only on the first time it is executed.
 *
 * This macro alerts the user that the library code currently being executing
 * is especially untested, and not to be regarded as reliable.  However, if
 * the flow of control passes again through this macro, the message is not
 * repeated.  Thus one may invoke this macro inside a loop without causing
 * pages of identical warning messages.
 *
 * Since this macro embeds a static variable in the context of wherever it
 * gets executed, you should be wary of invoking it inside of another macro.
 *
 * Related:
 *   test_pse
 *
 * Author:
 *   Kobus Barnard
 *
 * Documentor:  Andrew Predoehl
 *
 * Index:  debugging, Macros, warning messages, TEST
 *
 * ----------------------------------------------------------------------------
 */
#ifdef __C2MAN__
    void UNTESTED_CODE(void);
#endif
#ifndef __C2MAN__  /* I'm not using #else, following the example of 
                    * l/l_error.h line 63, which says that using #else would
                    * confuse ctags.
                    */

#ifdef TEST 

#define UNTESTED_CODE() \
            { \
                USING_KJB_C()                               \
                static int untested_first_time = TRUE;      \
                                                            \
                if (untested_first_time)                    \
                {                                           \
                    untested_first_time = FALSE;            \
                                                            \
                    flag_untested_code(__LINE__, __FILE__); \
                }                                           \
            }
#else  /* ! TEST */

#define UNTESTED_CODE() 

#endif  /* TEST */

#endif /* ! C2MAN */

#endif   /* ! SKIP_FOR_EXPORT i.e., case that we are NOT exported. */

#ifndef SKIP_FOR_EXPORT
/*
 * SKIP_FOR_EXPORT is never defined, so code below is always active.
 * Note that instances of UNTESTED_CODE() are stripped when we export. However,
 * here we protect the export process from the definition, which is thus
 * never used.
*/
#ifdef TEST 

/* We do not use KJB IO in this macro because often enough, the relevant
 * (suspect) code is in KJB IO. 
*/
#define SUSPECT_CODE() \
            { \
                USING_KJB_C()                              \
                static int suspect_first_time = TRUE;      \
                                                           \
                if (suspect_first_time)                    \
                {                                          \
                    suspect_first_time = FALSE;            \
                                                           \
                    flag_suspect_code(__LINE__, __FILE__); \
                }                                          \
            }
#else

#define SUSPECT_CODE() 

#endif 

#endif   /* Case that we are NOT exported. */



#ifdef __cplusplus
extern "C" {
#ifdef COMPILING_CPLUSPLUS_SOURCE
namespace kjb_c {
#endif
#endif
    
int  set_debug_options  (const char* option, const char* value);
int  kjb_set_debug_level(int new_level);
int  kjb_get_debug_level(void);
#ifdef TEST
int  set_suppress_test_messages(int new_level);
#endif 
void hex_print          (FILE*, void*, size_t);
void get_status_string  (int status_num, char* buff, size_t buff_size);

#ifdef TEST
#ifdef STANDARD_VAR_ARGS
#ifndef SGI /* SGI pretends it is lint and gives spurious messages. */
    /*PRINTFLIKE1*/
#endif
    void test_pse(const char*, ...);
    void test_pso(const char*, ...);
#else
    void test_pse(void);
    void test_pso(void);
#endif
#endif 

#ifdef TEST
void flag_untested_code(int line_num, const char* file_name);
void flag_suspect_code (int line_num, const char* file_name);
#endif 

#ifdef __cplusplus
#ifdef COMPILING_CPLUSPLUS_SOURCE
}
#endif
}
#endif

#endif

