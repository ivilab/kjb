
/* $Id: l_parse.h 21310 2017-03-19 20:58:40Z kobus $ */

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

#ifndef L_PARSE_INCLUDED
#define L_PARSE_INCLUDED


#include "l/l_def.h"

#ifdef __cplusplus
extern "C" {
#ifdef COMPILING_CPLUSPLUS_SOURCE
namespace kjb_c {
#endif
#endif


#define NO_MORE_TOKENS  0
#define STILL_MORE_TOKENS  1

typedef enum 
{
    TRIM_BEFORE, 
    TRIM_AFTER
}
Trim_order_t;


/* ------------------------------------------------------------------------- */

/* =============================================================================
 *                             BUFF_GET_TOKEN
 *
 * (MACRO) Sets up call to get_token
 *
 * The max_len parameter is set to sizeof(output_string).
 * Using sizeof to set the buffer size is recomended where applicable, as the
 * code will not be broken if the buffer size changes. HOWEVER, neither this
 * method, nor the macro, is  applicable if line is NOT a character array.  If
 * line is declared by "char *line", then the size of line is the number of
 * bytes in a character pointer (usually 4), which is NOT what is normally
 * intended.  You have been WARNED!
 *
 * This macro is the same as BUFF_CONST_GET_TOKEN() except that it should be
 * used when the argment is declared as (char**). Choosing the appropriate macro
 * between BUFF_GET_TOKEN() and BUFF_CONST_GET_TOKEN() is prefered over casting.
 *
 * | The macro BUFF_GET_TOKEN_OK expands to:
 * |            ( BUFF_GET_TOKEN(...) != NO_MORE_TOKENS )
 *
 * Example:
 *
 * |    char line[ GOOD_STRING_SIZE ];
 * |    char token[ GOOD_STRING_SIZE ];
 * |    char *line_pos;
 * |
 * |    // ....
 * |    //   CODE TO GET "line"
 * |    // ....
 * |
 * |    line_pos = line;
 * |
 * |    while (BUFF_GET_TOKEN_OK(&line_pos, token))
 * |    {
 * |        kjb_fprintf(stdout,"Token: %s\n",token);
 * |    }
 * |    kjb_puts(stdout,"All done.\n");
 * |
 * |    // Here, if the line was set to "A B C" by the CODE TO GET
 * |    // "line", then this example would output:
 * |    //     Token: A
 * |    //     Token: B
 * |    //     Token: C
 * |    //     All done.
 * |    //
 * |    // (It is worthwhile understanding this example if the L
 * |    //  parsing routines are to be used.)
 *
 * Related:
 *    get_token, const_get_token, gen_get_token, const_gen_get_token,
 *    match_quote_get_token, const_match_quote_get_token,
 *    gen_match_quote_get_token, const_gen_match_quote_get_token,
 *    match_get_token, const_match_get_token, gen_match_get_token,
 *    const_gen_match_get_token, gen_get_last_token
 *
 * Index: parsing, Macros
 *
 * -----------------------------------------------------------------------------
*/

#ifdef __C2MAN__ /* Pretend we are a ".c" file to document MACROS. */
    size_t BUFF_GET_TOKEN(const char** input_pos_ptr, char output_string[ ]);
#endif

#ifndef __C2MAN__  /* Just doing "else" confuses ctags for the first such line (only).
                      I have no idea what the problem is. */
#    define BUFF_GET_TOKEN(x, y) get_token(x, y, sizeof(y))
#endif


/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */


/* =============================================================================
 *                             BUFF_GET_TOKEN_OK
 *
 * (MACRO) Sets up call to get_token, then tests whether it was successful.
 *
 * This macro is ultimately an expression returning a value that indicates true
 * or false.  Please see the documentation for BUFF_GET_TOKEN for details.
 *
 * Index: parsing, Macros
 *
 * -----------------------------------------------------------------------------
*/
#ifdef __C2MAN__ /* Pretend we are a ".c" file to document MACROS. */
    int BUFF_GET_TOKEN_OK(const char** input_pos_ptr, char output_string[ ]);
#else
#    define BUFF_GET_TOKEN_OK(x, y) (BUFF_GET_TOKEN(x, y) > 0)
#endif


/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                             BUFF_CONST_GET_TOKEN
 *
 * (MACRO) Sets up call to const_get_token
 *
 * This macro is the same as BUFF_GET_TOKEN() except that it should be used when
 * the argment is declared as (const char**). Choosing the appropriate macro
 * between BUFF_GET_TOKEN() and BUFF_CONST_GET_TOKEN() is prefered over casting.
 *
 * BUFF_CONST_GET_TOKEN_OK() is defined in analogy with BUFF_GET_TOKEN_OK().
 *
 * Related:
 *    get_token, const_get_token, gen_get_token, const_gen_get_token,
 *    match_quote_get_token, const_match_quote_get_token,
 *    gen_match_quote_get_token, const_gen_match_quote_get_token,
 *    match_get_token, const_match_get_token, gen_match_get_token,
 *    const_gen_match_get_token, gen_get_last_token
 *
 * Index: parsing, Macros
 *
 * -----------------------------------------------------------------------------
*/
#ifdef __C2MAN__    /* Pretend we are a ".c" file to document MACROS. */
    size_t BUFF_CONST_GET_TOKEN(const char** input_pos_ptr, char output_string[ ]);
#else
#    define BUFF_CONST_GET_TOKEN(x, y) const_get_token(x, y, sizeof(y))

#    define BUFF_CONST_GET_TOKEN_OK(x, y) \
               (const_get_token(x, y, sizeof(y)) > 0)
#endif

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                             BUFF_MQ_GET_TOKEN
 *
 * (MACRO) Sets up call to match_quote_get_token
 *
 * The max_len parameter is set to sizeof(output_string).
 * Using sizeof to set the buffer size is recomended where applicable, as the
 * code will not be broken if the buffer size changes. HOWEVER, neither this
 * method, nor the macro, is  applicable if line is NOT a character array.  If
 * line is declared by "char *line", then the size of line is the number of
 * bytes in a character pointer (usually 4), which is NOT what is normally
 * intended.  You have been WARNED!
 *
 * | The macro BUFF_MQ_GET_TOKEN_OK expands to:
 * |            ( BUFF_MQ_GET_TOKEN(...) != NO_MORE_TOKENS )
 *
 * Note:
 *      This macro is the same as BUFF_CONST_MQ_GET_TOKEN() except that it
 *      should be used when the argment is declared as (char**). Choosing these
 *      macros based on the typing of the first argument is prefered over
 *      casting.
 *
 * Note:
 *    The documentation for BUFF_GET_TOKEN has a short example of which
 *    illustrates the general use of this grouup of parsing routines.
 *
 * Related:
 *    get_token, const_get_token, gen_get_token, const_gen_get_token,
 *    match_quote_get_token, const_match_quote_get_token,
 *    gen_match_quote_get_token, const_gen_match_quote_get_token,
 *    match_get_token, const_match_get_token, gen_match_get_token,
 *    const_gen_match_get_token, gen_get_last_token
 *
 *
 * Index: parsing, Macros
 *
 * -----------------------------------------------------------------------------
*/
#ifdef __C2MAN__    /* Pretend we are a ".c" file to document MACROS. */
    size_t BUFF_MQ_GET_TOKEN(const char** input_pos_ptr, char output_string[ ]);
#else
#    define BUFF_MQ_GET_TOKEN(x, y) \
                match_quote_get_token(x, y, sizeof(y))
#endif

#define BUFF_MQ_GET_TOKEN_OK(x, y) \
              (match_quote_get_token(x, y, sizeof(y)) > 0)

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                             BUFF_CONST_MQ_GET_TOKEN
 *
 * (MACRO) Sets up call to const_match_quote_get_token
 *
 * This macro is the same as BUFF_MQ_GET_TOKEN() except that it should be used
 * when the argment is declared as (const char**). Choosing the appropriate
 * macro between these is prefered over casting.
 *
 * | The macro BUFF_CONST_MQ_GET_TOKEN_OK expands to:
 * |            ( BUFF_CONST_MQ_GET_TOKEN(...) != NO_MORE_TOKENS )
 *
 * Note:
 *    The documentation for BUFF_GET_TOKEN has a short example of which
 *    illustrates the general use of this grouup of parsing routines.
 *
 * Related:
 *    get_token, const_get_token, gen_get_token, const_gen_get_token,
 *    match_quote_get_token, const_match_quote_get_token,
 *    gen_match_quote_get_token, const_gen_match_quote_get_token,
 *    match_get_token, const_match_get_token, gen_match_get_token,
 *    const_gen_match_get_token, gen_get_last_token
 *
 * Index: parsing, Macros
 *
 * -----------------------------------------------------------------------------
*/
#ifdef __C2MAN__    /* Pretend we are a ".c" file to document MACROS. */
    size_t BUFF_CONST_MQ_GET_TOKEN(const char** input_pos_ptr, char output_string[ ]);
#else
#    define BUFF_CONST_MQ_GET_TOKEN(x, y) \
                const_match_quote_get_token(x, y, sizeof(y))
#endif

#define BUFF_CONST_MQ_GET_TOKEN_OK(x, y) \
              (const_match_quote_get_token(x, y, sizeof(y)) > 0)

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                             BUFF_MATCH_GET_TOKEN
 *
 * (MACRO) Sets up call to match_quote_get_token
 *
 * The max_len parameter is set to sizeof(output_string).
 * Using sizeof to set the buffer size is recomended where applicable, as the
 * code will not be broken if the buffer size changes. HOWEVER, neither this
 * method, nor the macro, is  applicable if line is NOT a character array.  If
 * line is declared by "char *line", then the size of line is the number of
 * bytes in a character pointer (usually 4), which is NOT what is normally
 * intended.  You have been WARNED!
 *
 * | The macro BUFF_MATCH_GET_TOKEN_OK expands to:
 * |            ( BUFF_MATCH_GET_TOKEN(...) != NO_MORE_TOKENS )
 *
 * Note:
 *      This macro is the same as BUFF_CONST_MATCH_GET_TOKEN() except that it
 *      should be used when the argment is declared as (char**). Choosing these
 *      macros based on the typing of the first argument is prefered over
 *      casting.
 *
 * Note:
 *    The documentation for BUFF_GET_TOKEN has a short example of which
 *    illustrates the general use of this grouup of parsing routines.
 *
 * Related:
 *    get_token, gen_get_token, match_quote_get_token,
 *    gen_match_quote_get_token, match_get_token, gen_match_get_token,
 *    gen_get_last_token
 *
 * Index: parsing, Macros
 *
 * -----------------------------------------------------------------------------
*/
#ifdef __C2MAN__    /* Pretend we are a ".c" file to document MACROS. */
    size_t BUFF_MATCH_GET_TOKEN(const char** input_pos_ptr,
                                char   output_string[ ],
                                int    left_char,
                                int    right_char);
#else
#    define BUFF_MATCH_GET_TOKEN(w, x, y, z) \
                 match_get_token(w, x, sizeof(x), y, z)
#endif

#define BUFF_MATCH_GET_TOKEN_OK(w, x, y, z) \
                (match_get_token(w, x, sizeof(x), y, z) > 0)

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                             BUFF_CONST_MATCH_GET_TOKEN
 *
 * (MACRO) Sets up call to match_quote_get_token
 *
 * This macro is the same as BUFF_MATCH_GET_TOKEN() except that it should be
 * used when the argment is declared as (const char**). Choosing the appropriate
 * macro between these is prefered over casting.
 *
 * | The macro BUFF_CONST_MATCH_GET_TOKEN_OK expands to:
 * |            ( BUFF_CONST_MATCH_GET_TOKEN(...) != NO_MORE_TOKENS )
 *
 * Note:
 *    The documentation for BUFF_GET_TOKEN has a short example of which
 *    illustrates the general use of this grouup of parsing routines.
 *
 * Related:
 *    get_token, gen_get_token, match_quote_get_token,
 *    gen_match_quote_get_token, match_get_token, gen_match_get_token,
 *    gen_get_last_token
 *
 * Index: parsing, Macros
 *
 * -----------------------------------------------------------------------------
*/
#ifdef __C2MAN__    /* Pretend we are a ".c" file to document MACROS. */
    size_t BUFF_CONST_MATCH_GET_TOKEN(const char** input_pos_ptr,
                                char   output_string[ ],
                                int    left_char,
                                int    right_char);
#else
#    define BUFF_CONST_MATCH_GET_TOKEN(w, x, y, z) \
                 const_match_get_token(w, x, sizeof(x), y, z)
#endif

#define BUFF_CONST_MATCH_GET_TOKEN_OK(w, x, y, z) \
                (const_match_get_token(w, x, sizeof(x), y, z) > 0)

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                             BUFF_GEN_GET_TOKEN
 *
 * (MACRO) Sets up call to gen_get_token()
 *
 * The max_len parameter is set to sizeof(output_string).
 * Using sizeof to set the buffer size is recomended where applicable, as the
 * code will not be broken if the buffer size changes. HOWEVER, neither this
 * method, nor the macro, is  applicable if line is NOT a character array.  If
 * line is declared by "char *line", then the size of line is the number of
 * bytes in a character pointer (usually 4), which is NOT what is normally
 * intended.  You have been WARNED!
 *
 * | The macro BUFF_GEN_GET_TOKEN_OK expands to:
 * |            ( BUFF_GEN_GET_TOKEN(...) != NO_MORE_TOKENS )
 *
 * Note:
 *    This macro is the same as BUFF_CONST_GEN_GET_TOKEN() except that it should
 *    be used when the argment is declared as (char**). Choosing these macros
 *    based on the typing of the first argument is prefered over casting.
 *
 * Note:
 *    The documentation for BUFF_GET_TOKEN has a short example of which
 *    illustrates the general use of this grouup of parsing routines.
 *
 * Related:
 *    get_token, const_get_token, gen_get_token, const_gen_get_token,
 *    match_quote_get_token, const_match_quote_get_token,
 *    gen_match_quote_get_token, const_gen_match_quote_get_token,
 *    match_get_token, const_match_get_token, gen_match_get_token,
 *    const_gen_match_get_token, gen_get_last_token
 *
 * Index: parsing, Macros
 *
 * -----------------------------------------------------------------------------
*/
#ifdef __C2MAN__    /* Pretend we are a ".c" file to document MACROS. */
    size_t BUFF_GEN_GET_TOKEN(char** input_pos_ptr, char output_string[ ],
                              const char* terminators);
#else
#    define BUFF_GEN_GET_TOKEN(x, y, z) \
             gen_get_token(x, y, sizeof(y), z)
#endif

#define BUFF_GEN_GET_TOKEN_OK(x, y, z) \
              (gen_get_token(x, y, sizeof(y), z) > 0)


/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                             BUFF_CONST_GEN_GET_TOKEN
 *
 * (MACRO) Sets up call to const_gen_get_token()
 *
 * This macro is the same as BUFF_GEN_GET_TOKEN() except that the first argument
 * is (const char**) instead of (char**).
 *
 * | The macro BUFF_CONST_GEN_GET_TOKEN_OK expands to:
 * |            ( BUFF_CONST_GEN_GET_TOKEN(...) != NO_MORE_TOKENS )
 *
 * Note:
 *    The documentation for BUFF_GET_TOKEN has a short example of which
 *    illustrates the general use of this grouup of parsing routines.
 *
 * Related:
 *    get_token, const_get_token, gen_get_token, const_gen_get_token,
 *    match_quote_get_token, const_match_quote_get_token,
 *    gen_match_quote_get_token, const_gen_match_quote_get_token,
 *    match_get_token, const_match_get_token, gen_match_get_token,
 *    const_gen_match_get_token, gen_get_last_token
 *
 * Index: parsing, Macros
 *
 * -----------------------------------------------------------------------------
*/
#ifdef __C2MAN__    /* Pretend we are a ".c" file to document MACROS. */
    size_t BUFF_CONST_GEN_GET_TOKEN(const char** input_pos_ptr,
                                    char output_string[ ],
                                    const char* terminators);
#else
#    define BUFF_CONST_GEN_GET_TOKEN(x, y, z) \
             const_gen_get_token(x, y, sizeof(y), z)
#endif

#define BUFF_CONST_GEN_GET_TOKEN_OK(x, y, z) \
              (const_gen_get_token(x, y, sizeof(y), z) > 0)


/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                             BUFF_GEN_MQ_GET_TOKEN
 *
 * (MACRO) Sets up call to match_quote_get_token
 *
 * The max_len parameter is set to sizeof(output_string).
 * Using sizeof to set the buffer size is recomended where applicable, as the
 * code will not be broken if the buffer size changes. HOWEVER, neither this
 * method, nor the macro, is  applicable if line is NOT a character array.  If
 * line is declared by "char *line", then the size of line is the number of
 * bytes in a character pointer (usually 4), which is NOT what is normally
 * intended.  You have been WARNED!
 *
 * | The macro BUFF_GEN_MQ_GET_TOKEN_OK expands to:
 * |            ( BUFF_GEN_MQ_GET_TOKEN(...) != NO_MORE_TOKENS )
 *
 * Note:
 *    This macro is the same as BUFF_CONST_GEN_MQ_GET_TOKEN() except that it
 *    should be used when the argment is declared as (char**). Choosing these
 *    macros based on the typing of the first argument is prefered over casting.
 *
 * Note:
 *    The documentation for BUFF_GET_TOKEN has a short example of which
 *    illustrates the general use of this grouup of parsing routines.
 *
 * Related:
 *    get_token, gen_get_token, match_quote_get_token,
 *    gen_match_quote_get_token, match_get_token, gen_match_get_token,
 *    gen_get_last_token
 *
 * Index: parsing, Macros
 *
 * -----------------------------------------------------------------------------
*/
#ifdef __C2MAN__    /* Pretend we are a ".c" file to document MACROS. */
    size_t BUFF_GEN_MQ_GET_TOKEN(char** input_pos_ptr,
                                 char output_string[ ],
                                 const char* terminators);
#else
#    define BUFF_GEN_MQ_GET_TOKEN(x, y, z)  \
              gen_match_quote_get_token(x, y, sizeof(y), z)
#endif

#define BUFF_GEN_MQ_GET_TOKEN_OK(x, y, z) \
              (gen_match_quote_get_token(x, y, sizeof(y), z) > 0)


/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                             BUFF_CONST_GEN_MQ_GET_TOKEN
 *
 * (MACRO) Sets up call to match_quote_get_token
 *
 * This macro is the same as BUFF_GEN_MQ_GET_TOKEN_OK() except that it should be
 * used when the argment is declared as (const char**). Choosing the appropriate
 * macro between these is prefered over casting.
 *
 * | The macro BUFF_CONST_GEN_MQ_GET_TOKEN_OK expands to:
 * |            ( BUFF_CONST_GEN_MQ_GET_TOKEN(...) != NO_MORE_TOKENS )
 *
 * Note:
 *    The documentation for BUFF_GET_TOKEN has a short example of which
 *    illustrates the general use of this grouup of parsing routines.
 *
 * Related:
 *    get_token, const_get_token, gen_get_token, const_gen_get_token,
 *    match_quote_get_token, const_match_quote_get_token,
 *    gen_match_quote_get_token, const_gen_match_quote_get_token,
 *    match_get_token, const_match_get_token, gen_match_get_token,
 *    const_gen_match_get_token, gen_get_last_token
 *
 * Index: parsing, Macros
 *
 * -----------------------------------------------------------------------------
*/
#ifdef __C2MAN__    /* Pretend we are a ".c" file to document MACROS. */
    size_t BUFF_CONST_GEN_MQ_GET_TOKEN(char** input_pos_ptr,
                                 char output_string[ ],
                                 const char* terminators);
#else
#    define BUFF_CONST_GEN_MQ_GET_TOKEN(x, y, z)  \
              const_gen_match_quote_get_token(x, y, sizeof(y), z)
#endif

#define BUFF_CONST_GEN_MQ_GET_TOKEN_OK(x, y, z) \
              (const_gen_match_quote_get_token(x, y, sizeof(y), z) > 0)


/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                             BUFF_GEN_MATCH_GET_TOKEN
 *
 * (MACRO) Sets up call to match_quote_get_token
 *
 * The max_len parameter is set to sizeof(output_string).
 * Using sizeof to set the buffer size is recomended where applicable, as the
 * code will not be broken if the buffer size changes. HOWEVER, neither this
 * method, nor the macro, is  applicable if line is NOT a character array.  If
 * line is declared by "char *line", then the size of line is the number of
 * bytes in a character pointer (usually 4), which is NOT what is normally
 * intended.  You have been WARNED!
 *
 * | The macro BUFF_GEN_MATCH_GET_TOKEN_OK expands to:
 * |            ( BUFF_GEN_MATCH_GET_TOKEN(...) != NO_MORE_TOKENS )
 *
 * Note:
 *    This macro is the same as BUFF_CONST_GEN_MATCH_GET_TOKEN() except that it
 *    should be used when the argment is declared as (char**). Choosing these
 *    macros based on the typing of the first argument is prefered over casting.
 *
 * Note:
 *    This macro is the same as BUFF_CONST_GEN_GET_TOKEN() except that it should
 *    be used when the argment is declared as (char**). Choosing these macros
 *    based on the typing of the first argument is prefered over casting.
 *
 * Note:
 *    The documentation for BUFF_GET_TOKEN has a short example of which
 *    illustrates the general use of this grouup of parsing routines.
 *
 * Related:
 *    get_token, gen_get_token, match_quote_get_token,
 *    gen_match_quote_get_token, match_get_token, gen_match_get_token,
 *    gen_get_last_token
 *
 * Index: parsing, Macros
 *
 * -----------------------------------------------------------------------------
*/
#ifdef __C2MAN__    /* Pretend we are a ".c" file to document MACROS. */
    size_t BUFF_GEN_MATCH_GET_TOKEN(char**       input_pos_ptr,
                                    char         output_string[ ],
                                    const char*  terminators,
                                    int          left_char,
                                    int          right_char);
#else
#    define BUFF_GEN_MATCH_GET_TOKEN(v, w, x, y, z)  \
              gen_match_get_token(v, w, sizeof(w), x, y, z)
#endif

#define BUFF_GEN_MATCH_GET_TOKEN_OK(v, w, x, y, z)  \
              (gen_match_get_token(v, w, sizeof(w), x, y, z) > 0)


/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                             BUFF_CONST_GEN_MATCH_GET_TOKEN
 *
 * (MACRO) Sets up call to match_quote_get_token
 *
 * This macro is the same as BUFF_GEN_MATCH_GET_TOKEN() except that it should be
 * used when the argment is declared as (const char**). Choosing the appropriate
 * macro between these is prefered over casting.
 *
 * | The macro BUFF_CONST_GEN_MATCH_GET_TOKEN_OK expands to:
 * |            ( BUFF_CONST_GEN_MATCH_GET_TOKEN(...) != NO_MORE_TOKENS )
 *
 * Note:
 *    The documentation for BUFF_GET_TOKEN has a short example of which
 *    illustrates the general use of this grouup of parsing routines.
 *
 * Related:
 *    get_token, gen_get_token, match_quote_get_token,
 *    gen_match_quote_get_token, match_get_token, gen_match_get_token,
 *    gen_get_last_token
 *
 * Index: parsing, Macros
 *
 * -----------------------------------------------------------------------------
*/
#ifdef __C2MAN__    /* Pretend we are a ".c" file to document MACROS. */
    size_t BUFF_CONST_GEN_MATCH_GET_TOKEN(char**       input_pos_ptr,
                                    char         output_string[ ],
                                    const char*  terminators,
                                    int          left_char,
                                    int          right_char);
#else
#    define BUFF_CONST_GEN_MATCH_GET_TOKEN(v, w, x, y, z)  \
              const_gen_match_get_token(v, w, sizeof(w), x, y, z)
#endif

#define BUFF_CONST_GEN_MATCH_GET_TOKEN_OK(v, w, x, y, z)  \
              (const_gen_match_get_token(v, w, sizeof(w), x, y, z) > 0)


/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                             BUFF_GEN_GET_LAST_TOKEN
 *
 * (MACRO) Sets up call to gen_get_last_token
 *
 * The max_len parameter is set to sizeof(output_string).
 * Using sizeof to set the buffer size is recomended where applicable, as the
 * code will not be broken if the buffer size changes. HOWEVER, neither this
 * method, nor the macro, is  applicable if line is NOT a character array.  If
 * line is declared by "char *line", then the size of line is the number of
 * bytes in a character pointer (usually 4), which is NOT what is normally
 * intended.  You have been WARNED!
 *
 * | The macro BUFF_GEN_GET_LAST_TOKEN_OK expands to:
 * |            ( BUFF_GEN_GET_LAST_TOKEN(...) > 0)
 *
 * Related:
 *    get_token, gen_get_token, match_quote_get_token,
 *    gen_match_quote_get_token, match_get_token, gen_match_get_token,
 *    gen_get_last_token
 *
 * Index: parsing, Macros
 *
 * -----------------------------------------------------------------------------
*/
#ifdef __C2MAN__    /* Pretend we are a ".c" file to document MACROS. */
    size_t BUFF_GEN_GET_LAST_TOKEN(char*       input_string,
                                   char        output_string[ ],
                                   const char* terminators);
#else
#   define BUFF_GEN_GET_LAST_TOKEN(x, y, z) \
              gen_get_last_token(x, y, sizeof(y), z)
#endif

#define BUFF_GEN_GET_LAST_TOKEN_OK(x, y, z) \
              (gen_get_last_token(x, y, sizeof(y), z) > 0)

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                             BUFF_GEN_GET_LAST_TOKEN_2
 *
 * (MACRO) Sets up call to gen_get_last_token_2
 *
 * The max_len parameter is set to sizeof(output_string).
 * Using sizeof to set the buffer size is recomended where applicable, as the
 * code will not be broken if the buffer size changes. HOWEVER, neither this
 * method, nor the macro, is  applicable if line is NOT a character array.  If
 * line is declared by "char *line", then the size of line is the number of
 * bytes in a character pointer (usually 4), which is NOT what is normally
 * intended.  You have been WARNED!
 *
 * | The macro BUFF_GEN_GET_LAST_TOKEN_2_OK expands to:
 * |            ( BUFF_GEN_GET_LAST_TOKEN(...) < 0 )
 *
 * Related:
 *    get_token, gen_get_token, match_quote_get_token,
 *    gen_match_quote_get_token, match_get_token, gen_match_get_token,
 *    gen_get_last_token
 *
 * Index: parsing, Macros
 *
 * -----------------------------------------------------------------------------
*/
#ifdef __C2MAN__    /* Pretend we are a ".c" file to document MACROS. */
    size_t BUFF_GEN_GET_LAST_TOKEN_2(char*       input_string,
                                     char        output_string[ ],
                                     const char* terminators,
                                     Trim_order_t trim_order);
#else
#   define BUFF_GEN_GET_LAST_TOKEN_2(x, y, z, t) \
              gen_get_last_token_2(x, y, sizeof(y), z, t)
#endif

#define BUFF_GEN_GET_LAST_TOKEN_OK_2(x, y, z, t) \
              (gen_get_last_token_2(x, y, sizeof(y), z, t) > 0)

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                             BUFF_GEN_SPLIT_AT_LAST_TOKEN
 *
 * (MACRO) Sets up call to gen_split_at_last_token 
 *
 * The max_len parameter is set to sizeof(output_string).
 * Using sizeof to set the buffer size is recomended where applicable, as the
 * code will not be broken if the buffer size changes. HOWEVER, neither this
 * method, nor the macro, is  applicable if line is NOT a character array.  If
 * line is declared by "char *line", then the size of line is the number of
 * bytes in a character pointer (usually 4), which is NOT what is normally
 * intended.  You have been WARNED!
 *
 * Related:
 *    get_token, gen_get_token, match_quote_get_token,
 *    gen_match_quote_get_token, match_get_token, gen_match_get_token,
 *    gen_get_last_token
 *
 * Index: parsing, Macros
 *
 * -----------------------------------------------------------------------------
*/
#ifdef __C2MAN__    /* Pretend we are a ".c" file to document MACROS. */
    size_t BUFF_GEN_SPLIT_AT_LAST_TOKEN(const char* input_string,
                                        char        left_string[ ],
                                        char        right_string[ ],
                                        const char* terminators); 
#else
#   define BUFF_GEN_SPLIT_AT_LAST_TOKEN(x, y, z, t) \
              gen_split_at_last_token(x, y, sizeof(y), z, sizeof(z), t)
#endif

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                             BUFF_GEN_SPLIT_AT_LAST_TOKEN_2
 *
 * (MACRO) Sets up call to gen_split_at_last_token_2 
 *
 * The max_len parameter is set to sizeof(output_string).
 * Using sizeof to set the buffer size is recomended where applicable, as the
 * code will not be broken if the buffer size changes. HOWEVER, neither this
 * method, nor the macro, is  applicable if line is NOT a character array.  If
 * line is declared by "char *line", then the size of line is the number of
 * bytes in a character pointer (usually 4), which is NOT what is normally
 * intended.  You have been WARNED!
 *
 * Related:
 *    get_token, gen_get_token, match_quote_get_token,
 *    gen_match_quote_get_token, match_get_token, gen_match_get_token,
 *    gen_get_last_token
 *
 * Index: parsing, Macros
 *
 * -----------------------------------------------------------------------------
*/
#ifdef __C2MAN__    /* Pretend we are a ".c" file to document MACROS. */
    size_t BUFF_GEN_SPLIT_AT_LAST_TOKEN_2(const char* input_string,
                                          char        left_string[ ],
                                          char        right_string[ ],
                                          const char* terminators,
                                          Trim_order_t trim_order); 
#else
#   define BUFF_GEN_SPLIT_AT_LAST_TOKEN_2(x, y, z, t, o) \
              gen_split_at_last_token_2(x, y, sizeof(y), z, sizeof(z), t, o)
#endif

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

#define BUFF_ALPHA_GET_TOKEN(x, y) \
              alpha_get_token(x, y, sizeof(y))

#define BUFF_ALPHA_GET_TOKEN_OK(x, y) \
              (alpha_get_token(x, y, sizeof(y)) > 0)


#define BUFF_PARSE_ON_STRING(x, y, z) \
              parse_on_string(x, y, sizeof(y), z)

#define BUFF_UQ_CPY(x, y) \
              unquote_strncpy(x, y, sizeof(x))

/* -------------------------------------------------------------------------- */

void next_token      (char** string_arg_ptr);
void const_next_token(const char** string_arg_ptr);
void gen_next_token  (char** string_arg_ptr, const char* terminators);
int  is_more_input   (const char* input_line);

size_t get_token
(
    char** input_line_ptr,
    char*  output_string,
    size_t max_len
);

size_t const_get_token
(
    const char** input_line_ptr,
    char*        output_string,
    size_t       max_len
);

size_t match_quote_get_token
(
    char** input_line_ptr,
    char*  output_string,
    size_t max_len
);

size_t const_match_quote_get_token
(
    const char** input_line_ptr,
    char*        output_string,
    size_t       max_len
);

size_t match_get_token
(
    char**      input_line_ptr,
    char*       output_string,
    size_t      max_len,
    const char* left_char_str,
    const char* right_char_str
);

size_t const_match_get_token
(
    const char** input_line_ptr,
    char*        output_string,
    size_t       max_len,
    const char*  left_char_str,
    const char*  right_char_str
);

size_t gen_get_token
(
    char**      input_line_ptr,
    char*       output_string,
    size_t      max_len,
    const char* terminators
);

size_t const_gen_get_token
(
    const char** input_line_ptr,
    char*        output_string,
    size_t       max_len,
    const char*  terminators
);

size_t gen_char_get_token
(
    char** input_line_ptr,
    char*  output_string,
    size_t max_len,
    int    terminator
);

size_t const_gen_char_get_token
(
    const char** input_line_ptr,
    char*        output_string,
    size_t       max_len,
    int          terminator
);

size_t gen_match_quote_get_token
(
    char**      input_line_ptr,
    char*       output_string,
    size_t      max_len,
    const char* terminators
);

size_t const_gen_match_quote_get_token
(
    const char** input_line_ptr,
    char*        output_string,
    size_t       max_len,
    const char*  terminators
);

int    gen_match_get_token
(
    char**      input_line_ptr,
    char*       output_string,
    size_t      max_len,
    const char* terminators,
    const char* left_char_str,
    const char* right_char_str
);

int    const_gen_match_get_token
(
    const char** input_line_ptr,
    char*        output_string,
    size_t       max_len,
    const char*  terminators,
    const char*  left_char_str,
    const char*  right_char_str
);

size_t alpha_get_token
(
    char** input_line_ptr,
    char*  output_string,
    size_t max_len
);

size_t const_alpha_get_token
(
    const char** input_line_ptr,
    char*        output_string,
    size_t       max_len
);

int gen_get_last_token
(
    char*       input_line,
    char*       output_string,
    size_t      max_len,
    const char* terminators
);

int gen_get_last_token_2
(
    char*        input_line,
    char*        output_string,
    size_t       max_len,
    const char*  terminators,
    Trim_order_t trim_order_flag 
);

int gen_split_at_last_token
(
    const char* input_line,
    char*       left_str,
    size_t      left_str_size,
    char*       right_str,
    size_t      right_str_size,
    const char* terminators 
);

int gen_split_at_last_token_2
(
    const char*  input_line,
    char*        left_str,
    size_t       left_str_size,
    char*        right_str,
    size_t       right_str_size,
    const char*  terminators,
    Trim_order_t trim_order_flag 
);

size_t   parse_on_string
(
    char**      input_line_ptr,
    char*       output_string,
    size_t      max_len,
    const char* search_str
);

void unquote_strncpy(char* s1, const char* s2, size_t max_len);

void strip_quotes(char* input_string);
int all_blanks(const char* input_string);
int all_white_space(const char* input_string);
int all_digits(const char* string);
int all_n_digits(const char* string, size_t len);

int kjb_parse(const char* input_string, char*** args_ptr);

size_t count_tokens(const char* input_string);

size_t gen_count_tokens
(
    const char* input_string,
    const char* terminators
);

char* parse_next(char** input_string_ptr);
size_t get_next_token_len(const char* input_string);

int inc_copy_paren_exp(char** input_ptr, char** output_ptr);

#ifdef STANDARD_VAR_ARGS
int get_n_pos_int(int num_args, const char** arg_string_ptr, ...);
#else
int get_n_pos_int(void);
#endif

int parse_positive_integer_list
(
    const char* input,
    int         min_value,
    int         max_value,
    int**       output_array_ptr
);

int parse_options
(
    char*   input,
    char*** option_list_ptr,
    char*** value_list_ptr
);

int free_options(int num_options, char** option_list, char** value_list);

int ic_parse_key_words
(
    char*        input,
    int          num_key_words,
    const char** key_words,
    int*         key_words_found_list,
    int*         key_words_found_index
);

int get_boolean_value(const char* str);
int is_no_value_word(const char* str);

int parse_path
(
    const char* path,
    char*       dir_str,
    size_t      dir_str_size,
    char*       name,
    size_t      name_size 
);

int get_base_name
(
    const char*  name,
    char*        dir_str,
    size_t       dir_str_size,
    char*        base_name,
    size_t       max_base_name_size,
    char*        suffix,
    size_t       max_suffix_size,
    const char** suffixes
);

int get_base_path
(
    const char*  path,
    char*        base_path,
    size_t       max_base_path_len,
    char*        suffix,
    size_t       max_suffix_len,
    const char** suffixes
);


#ifdef __cplusplus
#ifdef COMPILING_CPLUSPLUS_SOURCE
}
#endif
}
#endif

#endif

