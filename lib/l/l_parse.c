
/* $Id: l_parse.c 21520 2017-07-22 15:09:04Z kobus $ */

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

#include "l/l_gen.h"     /* Only safe as first include in a ".c" file. */

#include "l/l_math.h"      /* For get_random_integer_list */
#include "l/l_string.h"
#include "l/l_sys_scan.h"
#include "l/l_sys_io.h"
#include "l/l_parse.h"


#ifdef __cplusplus
extern "C" {
#endif

/* -------------------------------------------------------------------------- */

static int any_integer_is_positive(size_t num_ints, int* int_array);

/* -------------------------------------------------------------------------- */

/*
 * We want the exact same code for next_token() as const_next_token() so we put
 * the code in a macro defined here.
*/

#define NEXT_TOKEN_CODE() \
{\
    while (    (NOT_WHITE_SPACE(**string_arg_ptr)) \
            && (**string_arg_ptr != '\0')\
          )\
    {\
        ++(*string_arg_ptr); \
    }\
\
    while (    (IS_WHITE_SPACE(**string_arg_ptr))\
            && (**string_arg_ptr != '\0')\
          )\
     {\
         ++(*string_arg_ptr); \
     }\
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*
 * =============================================================================
 *                            next_token
 *
 * Sets the begining of string to next non-blank/tab
 *
 * Destructively resets the begining of a string to next non blank AFTER
 * next blank.
 *
 * Index: parsing
 *
 * -----------------------------------------------------------------------------
 */

void next_token(char** string_arg_ptr)
NEXT_TOKEN_CODE()

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*
 * =============================================================================
 *                            const_next_token
 *
 * Sets the begining of string to next non-blank/tab
 *
 * Destructively resets the begining of a string to next non blank AFTER
 * next blank.
 *
 * Note:
 *     This routine is the same as next_token, except that the constness of
 *     **string_arg_ptr is made explicit. Neither version changes the input
 *     string characters.
 *
 * Index: parsing
 *
 * -----------------------------------------------------------------------------
 */

void const_next_token(const char** string_arg_ptr)
NEXT_TOKEN_CODE()

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

void gen_next_token(char** string_arg_ptr, const char* terminators)
{

    while (    (FIND_CHAR_NO(terminators, **string_arg_ptr))
            && (**string_arg_ptr != '\0')
          )
     {
         ++(*string_arg_ptr);
     }

    while (    (FIND_CHAR_YES(terminators, **string_arg_ptr))
            && (**string_arg_ptr != '\0')
          )
    {
        ++(*string_arg_ptr);
    }
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

int is_more_input (const char* input_line)
{


    while ((IS_WHITE_SPACE(*input_line)) && (*input_line != '\0'))
    {
        ++input_line;
    }

    if ( *input_line == '\0' ) return FALSE;
    else return TRUE;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*
 * We want the exact same code for get_token() as const_get_token() so we put
 * the code in a macro defined here.
*/

#define GET_TOKEN_CODE() \
{\
     size_t i=0;\
\
    ASSERT(max_len != 0);\
\
    while (IS_WHITE_SPACE(**input_line_ptr))\
    {\
        if (**input_line_ptr == '\0') break;\
\
        ++(*input_line_ptr); \
    }\
\
     --max_len;  /* Account for room needed for NULL */ \
\
     while (    (**input_line_ptr != '\0') \
             && (NOT_WHITE_SPACE(**input_line_ptr)) \
             && ( i < max_len )\
           )\
     {\
         *output_string = **input_line_ptr; \
         ++ output_string; \
         ++i; \
         ++ ( *input_line_ptr ); \
     }\
\
     *output_string = '\0'; \
\
     return i;  /* i == 0 == NO_MORE_TOKENS <-->no more tokens */ \
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*
 * =============================================================================
 *                              get_token
 *
 * Gets the next non-whitespace chunk
 *
 * This routine parses the string pointed to by "input_line_ptr" by skipping
 * over white space (blanks, tabs), and then copying characters into the buffer
 * output_string up to the next terminating blank or tab. A maximun of
 * max_len-ONE characters are copied. If this limit is reached, then the
 * routine behaves as if a blank was found. The buffer output_string will
 * always be NULL terminated. The routine returns the number of characters
 * copied into output_string. If there is nothing other than white space in the
 * string beyond the parse location, then NO_MORE_TOKENS  (#defined as 0) is
 * returned. The string location pointer is set beyond the token just parsed.
 *
 * Macros:
 *     BUFF_GET_TOKEN, BUFF_GET_TOKEN_OK
 *
 * Note:
 *    Expressing the fact that the first argument to this routine should be (const
 *    char**) leads to no end of trouble. Hence there are two versions of this
 *    routine. This one should be used when the agument that you want to send is
 *    in fact (char**). This is preferable to casting which causes trouble with
 *    some C++ compilers.
 *
 * Note:
 *    The documentation for BUFF_GET_TOKEN has a short example of which
 *    illustrates the general use of this grouup of parsing routines.
 *
 * Returns:
 *     The number of characters copied to the buffer excluding the NULL. This
 *     means that if there are no characters left, then NO_MORE_TOKENS is
 *     returned which is #defined as 0. 
 *
 * Related:
 *     BUFF_GET_TOKEN(), BUFF_CONST_GET_TOKEN(), const_get_token()
 *
 * Index: parsing
 *
 * -----------------------------------------------------------------------------
 */

size_t get_token
(
    char** input_line_ptr,
    char*  output_string,
    size_t max_len
)
GET_TOKEN_CODE()

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*
 * =============================================================================
 *                              const_get_token
 *
 * Gets the next non-whitespace chunk
 *
 * This routine is the same as get_token() except that it the constness of the
 * string pointed to by the first argument is made explicit. If the argument is
 * in fact (const char**) then you should use this version and the corresponding
 * macro; otherwise use get_token().
 *
 * Macros:
 *     BUFF_CONST_GET_TOKEN, BUFF_CONST_GET_TOKEN_OK
 *
 * Note:
 *    Expressing the fact that the first argument to this routine should be (const
 *    char**) leads to no end of trouble. Hence there are two versions of this
 *    routine. This one should be used when the agument that you want to send is
 *    in fact (const char**). This is preferable to casting which causes trouble with
 *    some C++ compilers.
 *
 * Note:
 *    The documentation for BUFF_GET_TOKEN has a short example of which
 *    illustrates the general use of this grouup of parsing routines.
 *
 * Returns:
 *     The number of characters copied to the buffer excluding the NULL. This
 *     means that if there are no characters left, then NO_MORE_TOKENS is
 *     returned which is #defined as 0. 
 *
 * Related:
 *     BUFF_CONST_GET_TOKEN(), get_token()
 *
 * Index: parsing
 *
 * -----------------------------------------------------------------------------
 */

size_t const_get_token
(
    const char** input_line_ptr,
    char*        output_string,
    size_t       max_len
)
GET_TOKEN_CODE()

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*
 * We want the exact same code for match_quote_get_token() as
 * const_match_quote_get_token() so we put the code in a macro defined here.
*/

#define MATCH_QUOTE_GET_TOKEN_CODE() \
{\
     size_t i=0;\
     int match_quote; \
\
\
     match_quote = FALSE; \
\
     while (IS_WHITE_SPACE(**input_line_ptr))\
     {\
         if (**input_line_ptr == '\0') break;\
\
         ++(*input_line_ptr); \
     }\
\
\
     --max_len;  /* Account for room needed for NULL */ \
\
     while (    (**input_line_ptr != '\0') \
             && ((NOT_WHITE_SPACE(**input_line_ptr)) || match_quote) \
             && ( i < max_len)\
           )\
     {\
         if (**input_line_ptr == '"') \
         {\
             match_quote = !match_quote; \
         }\
\
         *output_string = **input_line_ptr; \
         ++output_string; \
         ++i; \
         ++(*input_line_ptr); \
     }\
\
     *output_string = '\0'; \
\
     return i;  /* i == 0 == NO_MORE_TOKENS <-->no more tokens */ \
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*
 * =============================================================================
 *                          match_quote_get_token
 *
 * Get the next non-white chunk, ignoring white in quotes.
 *
 * This routine is similar to get_token except that terminating blanks are NOT
 * sought inside of quote marks. The quotes are not stripped from the
 * retrieved chunk. Thus, whereas the routine get_token would parse
 * ->a "b c"<- into ->a<- and ->"b<- and ->c"<-, this routine would parse it
 * into ->a<- and ->"b c"<-. The routine strip_quotes can be used to remove the
 * quotes.
 *
 * Note:
 *     This routine is similar to const_match_quote_get_token() except that the
 *     argument has been declared as (char**). One should decide between these
 *     routines based on this, in preference to casting.
 *
 * Macros:
 *     BUFF_MQ_GET_TOKEN, BUFF_MQ_GET_TOKEN_OK
 *
 * Returns:
 *     The number of characters copied to the buffer excluding the NULL. This
 *     means that if there are no characters left, then NO_MORE_TOKENS is
 *     returned which is #defined as 0. 
 *
 * Note:
 *    The documentation for BUFF_GET_TOKEN has a short example of which
 *    illustrates the general use of this grouup of parsing routines.
 *
 * Related:
 *     BUFF_MQ_GET_TOKEN
 *
 * Index: parsing
 *
 * -----------------------------------------------------------------------------
 */

size_t match_quote_get_token
(
    char** input_line_ptr,
    char*  output_string,
    size_t max_len
)
MATCH_QUOTE_GET_TOKEN_CODE()

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*
 * =============================================================================
 *                          const_match_quote_get_token
 *
 * Get the next non-white chunk, ignoring white in quotes.
 *
 * This routine is similar to match_quote_get_token() except that the argument
 * has been declared as (const char**). One should decide between these routines
 * based on this, in preference to casting.
 *
 * Macros:
 *     BUFF_CONST_MQ_GET_TOKEN, BUFF_CONST_MQ_GET_TOKEN_OK
 *
 * Returns:
 *     The number of characters copied to the buffer excluding the NULL. This
 *     means that if there are no characters left, then NO_MORE_TOKENS is
 *     returned which is #defined as 0. 
 *
 * Note:
 *    The documentation for BUFF_GET_TOKEN has a short example of which
 *    illustrates the general use of this grouup of parsing routines.
 *
 * Related:
 *     BUFF_CONST_MQ_GET_TOKEN
 *
 * Index: parsing
 *
 * -----------------------------------------------------------------------------
 */

size_t const_match_quote_get_token
(
    const char** input_line_ptr,
    char*        output_string,
    size_t       max_len
)
MATCH_QUOTE_GET_TOKEN_CODE()

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*
 * =============================================================================
 *                          match_get_token
 *
 * Get the next non-white chunk, ignoring white inside match chars.
 *
 * This routine is similar to get_token except that terminating blanks are NOT
 * sought inside of paris of matching characters given in "left_char_str" and
 * "right_char_str" pairs. For example, if left_char_str is "{("
 * and right_char_str is '{(', then this routine would parse ->(a { b } ) c<-
 * into ->(a { b } )<- and ->c<-.
 *
 * Note:
 *     This routine is similar to const_match_get_token() except that the
 *     argument has been declared as (char**). One should decide between these
 *     routines based on this, in preference to casting.
 *
 * Macros:
 *     BUFF_MATCH_GET_TOKEN, BUFF_MATCH_GET_TOKEN_OK
 *
 * Returns:
 *     The number of characters copied to the buffer excluding the NULL. This
 *     means that if there are no characters left, then NO_MORE_TOKENS is
 *     returned which is #defined as 0. 
 *
 * Note:
 *    The documentation for BUFF_GET_TOKEN has a short example of which
 *    illustrates the general use of this grouup of parsing routines.
 *
 * Related:
 *     BUFF_MATCH_GET_TOKEN
 *
 * Index: parsing
 *
 * -----------------------------------------------------------------------------
 */

/*
// Although it is tempting to implement this in terms of gen_match_get_token,
// this gets a bit hairy because here we parse on white space which we don't
// implement as a string, but as a call to a system supplied macro. Hence, for
// now, I will just copy the code.
*/

size_t match_get_token
(
    char**      input_line_ptr,
    char*       output_string,
    size_t      max_len,
    const char* left_char_str,
    const char* right_char_str
)
{
#if 0 /* was ifdef HOW_IT_WAS_06_03_12 */
    size_t i = 0;
    size_t ii;
    int*   depth;
    size_t num_match_chars;


     UNTESTED_CODE();

     ASSERT(max_len != 0);

     if (left_char_str == NULL) left_char_str = "";
     if (right_char_str == NULL) right_char_str = "";

     num_match_chars = strlen(left_char_str);

     if (num_match_chars != strlen(right_char_str))
     {
         SET_ARGUMENT_BUG();
         return 0;
     }

     NPETE(depth = INT_MALLOC(num_match_chars));

     for (ii = 0; ii < num_match_chars; ii++) depth[ ii ] = 0;

     --max_len;  /* Account for room needed for NULL */

     while (    (**input_line_ptr != '\0')
             && (IS_WHITE_SPACE(**input_line_ptr))
           )
     {
         ++(*input_line_ptr);
     }

     while (    (**input_line_ptr != '\0')
             && (    (IS_WHITE_SPACE(**input_line_ptr))
                  || any_integer_is_positive(num_match_chars, depth) > 0)
             &&
                ( i < max_len )
           )
     {
         for (ii = 0; ii < num_match_chars; ii++)
         {
             if (**input_line_ptr == left_char_str[ ii ])
             {
                 (depth[ ii ])++;
             }
             else if (    (depth[ ii ] > 0)
                       && (**input_line_ptr == right_char_str[ ii ])
                     )
             {
                 (depth[ ii ])--;
             }
         }

         *output_string = **input_line_ptr;
         ++output_string;
         ++i;
         ++(*input_line_ptr);
     }

     *output_string = '\0';

     kjb_free(depth);

     return i;  /* i == 0 == NO_MORE_TOKENS */
#else
     UNTESTED_CODE();

     return gen_match_get_token(input_line_ptr, output_string, max_len,
                                WHITE_SPACE_STR,
                                left_char_str, right_char_str);
#endif
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*
 * =============================================================================
 *                          const_match_get_token
 *
 * Get the next non-white chunk, ignoring white inside match chars.
 *
 * This routine is similar to match_get_token() except that the argument has
 * been declared as (const char**). One should decide between these routines
 * based on this, in preference to casting.
 *
 * Macros:
 *     BUFF_CONST_MATCH_GET_TOKEN, BUFF_CONST_MATCH_GET_TOKEN_OK
 *
 * Returns:
 *     The number of characters copied to the buffer excluding the NULL. This
 *     means that if there are no characters left, then NO_MORE_TOKENS is
 *     returned which is #defined as 0. 
 *
 * Note:
 *    The documentation for BUFF_GET_TOKEN has a short example of which
 *    illustrates the general use of this grouup of parsing routines.
 *
 * Related:
 *     BUFF_MATCH_GET_TOKEN
 *
 * Index: parsing
 *
 * -----------------------------------------------------------------------------
 */

size_t const_match_get_token
(
    const char** input_line_ptr,
    char*        output_string,
    size_t       max_len,
    const char*  left_char_str,
    const char*  right_char_str
)
{
    UNTESTED_CODE();

    return const_gen_match_get_token(input_line_ptr, output_string, max_len,
                                     WHITE_SPACE_STR,
                                     left_char_str, right_char_str);
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

#define GEN_GET_TOKEN_CODE() \
{\
     size_t i=0;\
\
     ASSERT(max_len != 0);\
\
     --max_len;  /* Account for room needed for NULL */ \
\
     while ((**input_line_ptr != '\0') && \
            (FIND_CHAR_YES(terminators, (int)(**input_line_ptr))))\
      {\
          ++ (*input_line_ptr); \
      }\
\
     while ( (**input_line_ptr != '\0') &&\
             (FIND_CHAR_NO(terminators, (int)(**input_line_ptr))) && \
             ( i < max_len ))\
     {\
         *output_string = **input_line_ptr; \
         ++ output_string; \
         ++i; \
         ++(*input_line_ptr ); \
     }\
\
     *output_string = '\0'; \
\
     return i;  /* i == 0 == NO_MORE_TOKENS <-->no more tokens */ \
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*
 * =============================================================================
 *                              gen_get_token
 *
 * Get next chunk delimited by specified chars.
 *
 * This routine is similar to get_token except that the terminating characters
 * in the string "terminators" is used as opposed to white-space. If a blank is
 * to be used as a terminator, then it must be explicity in the string
 * "terminators".  Thus something like "a/b/c" can be parsed into a,b,c by
 * using a terminators argument "/".
 *
 * Note:
 *     This routine is similar to gen_get_token except that it makes explicit
 *     the constness of **input_line_ptr. You should choose between these
 *     routines based on the whether the first argument is (const char**) or
 *     (char**). This is prefered over casting as this breaks some C++
 *     compilers.
 *
 * Macros:
 *     BUFF_GEN_GET_TOKEN, BUFF_GEN_GET_TOKEN_OK
 *
 * Returns:
 *     The number of characters copied to the buffer excluding the NULL. This
 *     means that if there are no characters left, then NO_MORE_TOKENS is
 *     returned which is #defined as 0. 
 *
 * Note:
 *    The documentation for BUFF_GET_TOKEN has a short example of which
 *    illustrates the general use of this grouup of parsing routines.
 *
 * Related:
 *     BUFF_GEN_GET_TOKEN
 *
 * Index: parsing
 *
 * -----------------------------------------------------------------------------
 */

size_t gen_get_token
(
    char**      input_line_ptr,
    char*       output_string,
    size_t      max_len,
    const char* terminators
)
GEN_GET_TOKEN_CODE()

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*
 * =============================================================================
 *                              const_gen_get_token
 *
 * Get next chunk delimited by specified chars.
 *
 * This routine is similar to gen_get_token except that it makes explicit the
 * constness of **input_line_ptr. You should choose between these routines based
 * on the whether the first argument is (const char**) or (char**). This is
 * prefered over casting as this breaks some C++ compilers.
 *
 * Macros:
 *     BUFF_CONST_GEN_GET_TOKEN, BUFF_CONST_GEN_GET_TOKEN_OK
 *
 * Returns:
 *     The number of characters copied to the buffer excluding the NULL. This
 *     means that if there are no characters left, then NO_MORE_TOKENS is
 *     returned which is #defined as 0. 
 *
 * Note:
 *    The documentation for BUFF_GET_TOKEN has a short example of which
 *    illustrates the general use of this grouup of parsing routines.
 *
 * Related:
 *     BUFF_GEN_GET_TOKEN
 *
 * Index: parsing
 *
 * -----------------------------------------------------------------------------
 */

size_t const_gen_get_token
(
    const char** input_line_ptr,
    char*        output_string,
    size_t       max_len,
    const char*  terminators
)
GEN_GET_TOKEN_CODE()

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*
 * We want the exact same code for gen_char_get_token() as
 * const_gen_char_get_token() so we put the code in a macro defined here.
*/

#define GET_CHAR_TOKEN_CODE() \
{\
     size_t i               = 0;\
     char   terminator_char;\
\
\
     ASSERT(max_len != 0);\
\
     --max_len;  /* Account for room needed for NULL */ \
     terminator_char = (char)terminator; \
\
     while ((**input_line_ptr != '\0') && \
            (**input_line_ptr == terminator_char)) \
      {\
          ++ (*input_line_ptr); \
      }\
\
     while ( (**input_line_ptr != '\0') &&\
             (**input_line_ptr != terminator_char) && \
             ( i < max_len ))\
     {\
         *output_string = **input_line_ptr; \
         ++ output_string; \
         ++i; \
         ++ ( *input_line_ptr ); \
     }\
     *output_string = '\0'; \
\
     return i;  /* i == 0 == NO_MORE_TOKENS <-->no more tokens */ \
}


/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

size_t gen_char_get_token
(
    char** input_line_ptr,
    char*  output_string,
    size_t max_len,
    int    terminator
)
GET_CHAR_TOKEN_CODE()

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

size_t const_gen_char_get_token
(
    const char** input_line_ptr,
    char*        output_string,
    size_t       max_len,
    int          terminator
)
GET_CHAR_TOKEN_CODE()

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*
 * We want the exact same code for get_token() as const_get_token() so we put
 * the code in a macro defined here.
*/

#define GEN_MATCH_QUOTE_GET_TOKEN_CODE() \
{\
     size_t i           = 0;\
     int    match_quote;\
\
\
     ASSERT(max_len != 0);\
     --max_len;  /* Account for room needed for NULL */ \
     match_quote = FALSE; \
\
     while (    (**input_line_ptr != '\0')\
             && (FIND_CHAR_YES(terminators, **input_line_ptr))\
           )\
     {\
          ++ (*input_line_ptr); \
     }\
\
     while (    (**input_line_ptr != '\0')\
             && (( FIND_CHAR_NO(terminators, **input_line_ptr)) || match_quote)\
             && ( i < max_len )\
           )\
     {\
         if (**input_line_ptr == '"') \
         {\
             match_quote = !match_quote; \
         }\
\
         *output_string = **input_line_ptr; \
         ++output_string; \
         ++i; \
\
         ++ ( *input_line_ptr ); \
     }\
     *output_string = '\0'; \
\
     return i;  /* i == 0 == NO_MORE_TOKENS <-->no more tokens */ \
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*
 * =============================================================================
 *                             gen_match_quote_get_token
 *
 * Get next chunk delimited by specified chars
 *
 * This routine gets the next chunk delimited by specified chars except if
 * they are inside matching quotes. The quotes are not stripped from the input.
 *
 * This routine combines the behaviour of gen_get_token and
 * match_quote_get_token.
 *
 * Macros:
 *     BUFF_GEN_MQ_GET_TOKEN, BUFF_GEN_MQ_GET_TOKEN_OK
 *
 * Returns:
 *     The number of characters copied to the buffer excluding the NULL. This
 *     means that if there are no characters left, then NO_MORE_TOKENS is
 *     returned which is #defined as 0. 
 *
 * Note:
 *    The documentation for BUFF_GET_TOKEN has a short example of which
 *    illustrates the general use of this grouup of parsing routines.
 *
 * Related:
 *     BUFF_GEN_MQ_GET_TOKEN
 *
 * Index: parsing
 *
 * -----------------------------------------------------------------------------
 */

size_t gen_match_quote_get_token
(
    char**      input_line_ptr,
    char*       output_string,
    size_t      max_len,
    const char* terminators
)
GEN_MATCH_QUOTE_GET_TOKEN_CODE()

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*
 * =============================================================================
 *                             const_gen_match_quote_get_token
 *
 * Get next chunk delimited by specified chars
 *
 * This routine is similar to gen_match_quote_get_token() EXCEPT that the
 * argument has been declared as (const char**). One should decide between these
 * routines based on this, in preference to casting.
 *
 * Macros:
 *     BUFF_CONST_GEN_MQ_GET_TOKEN, BUFF_CONST_GEN_MQ_GET_TOKEN_OK
 *
 * Returns:
 *     The number of characters copied to the buffer excluding the NULL. This
 *     means that if there are no characters left, then NO_MORE_TOKENS is
 *     returned which is #defined as 0. 
 *
 * Note:
 *    The documentation for BUFF_GET_TOKEN has a short example of which
 *    illustrates the general use of this grouup of parsing routines.
 *
 * Related:
 *     BUFF_CONST_GEN_MQ_GET_TOKEN
 *
 * Index: parsing
 *
 * -----------------------------------------------------------------------------
 */

size_t const_gen_match_quote_get_token
(
    const char** input_line_ptr,
    char*        output_string,
    size_t       max_len,
    const char*  terminators
)
GEN_MATCH_QUOTE_GET_TOKEN_CODE()

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*
 * We want the exact same code for gen_match_get_token() as
 * const_gen_match_get_token() so we put the code in a macro defined here.
*/

#define GEN_MATCH_GET_TOKEN_CODE() \
{\
    size_t i = 0;\
    size_t ii;\
    int*   depth;\
    size_t num_match_chars;\
\
\
     ASSERT(max_len != 0);\
\
     if (left_char_str == NULL) left_char_str = "";\
     if (right_char_str == NULL) right_char_str = "";\
\
     num_match_chars = strlen(left_char_str);\
\
     if (num_match_chars != strlen(right_char_str))\
     {\
         SET_ARGUMENT_BUG();\
         return ERROR;\
     }\
\
     NRE(depth = INT_MALLOC(num_match_chars)); \
     for (ii = 0; ii < num_match_chars; ii++) depth[ ii ] = 0;\
\
     --max_len;  /* Account for room needed for NULL */ \
\
     while (    (**input_line_ptr != '\0')\
             && (FIND_CHAR_YES(terminators, **input_line_ptr))\
           )\
     {\
         ++(*input_line_ptr); \
     }\
\
     while (    (**input_line_ptr != '\0')\
             && (    (FIND_CHAR_NO(terminators, **input_line_ptr)) \
                  || any_integer_is_positive(num_match_chars, depth) > 0)\
             &&\
                ( i < max_len )\
           )\
     {\
         for (ii = 0; ii < num_match_chars; ii++) \
         {\
             if (**input_line_ptr == left_char_str[ ii ]) \
             {\
                 (depth[ ii ])++; \
             }\
             else if (    (depth[ ii ] > 0)\
                       && (**input_line_ptr == right_char_str[ ii ])\
                     )\
             {\
                 (depth[ ii ])--;\
             }\
         }\
\
         *output_string = **input_line_ptr; \
         ++output_string; \
         ++i; \
         ++(*input_line_ptr); \
     }\
\
     *output_string = '\0'; \
\
     kjb_free(depth); \
\
     return i;  /* i == 0 == NO_MORE_TOKENS */ \
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*
 * =============================================================================
 *                             gen_match_get_token
 *
 * Get next chunk delimited by specified chars.
 *
 * This routine gets the next chunk delimited by specified chars, except when
 * they are inside a pair of the match chars. The match chars are included in
 * the retrieved chunk.
 *
 * This routine combines the behaviour of gen_get_token and
 * match_get_token.
 *
 * Note:
 *     This routine is similar to const_gen_match_get_token() except that the
 *     argument has been declared as (char**). One should decide between these
 *     routines based on this, in preference to casting.
 *
 * Macros:
 *     BUFF_GEN_MATCH_GET_TOKEN, BUFF_GEN_MATCH_GET_TOKEN_OK
 *
 * Returns:
 *     The number of characters copied to the buffer excluding the NULL. This
 *     means that if there are no characters left, then NO_MORE_TOKENS is
 *     returned which is #defined as 0. 
 *
 * Note:
 *    The documentation for BUFF_GET_TOKEN has a short example of which
 *    illustrates the general use of this grouup of parsing routines.
 *
 * Related:
 *     BUFF_GEN_MATCH_GET_TOKEN
 *
 * Index: parsing
 *
 * -----------------------------------------------------------------------------
 */

int gen_match_get_token
(
    char**      input_line_ptr,
    char*       output_string,
    size_t      max_len,
    const char* terminators,
    const char* left_char_str,
    const char* right_char_str
)
GEN_MATCH_GET_TOKEN_CODE()

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*
 * =============================================================================
 *                             const_gen_match_get_token
 *
 * Get next chunk delimited by specified chars.
 *
 * This routine is similar to gen_match_get_token() except that the argument has
 * been declared as (const char**). One should decide between these routines
 * based on this, in preference to casting.
 *
 * Macros:
 *     BUFF_CONST_GEN_MATCH_GET_TOKEN, BUFF_CONST_GEN_MATCH_GET_TOKEN_OK
 *
 * Returns:
 *     The number of characters copied to the buffer excluding the NULL. This
 *     means that if there are no characters left, then NO_MORE_TOKENS is
 *     returned which is #defined as 0. 
 *
 * Note:
 *    The documentation for BUFF_CONST_GET_TOKEN has a short example of which
 *    illustrates the general use of this grouup of parsing routines.
 *
 * Related:
 *     BUFF_CONST_GEN_MATCH_GET_TOKEN
 *
 * Index: parsing
 *
 * -----------------------------------------------------------------------------
 */

int const_gen_match_get_token
(
    const char** input_line_ptr,
    char*        output_string,
    size_t       max_len,
    const char*  terminators,
    const char*  left_char_str,
    const char*  right_char_str
)
GEN_MATCH_GET_TOKEN_CODE()

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*
 * We want the exact same code for alpha_get_token() as const_alpha_get_token()
 * so we put the code in a macro defined here.
*/

#define ALPHA_GET_TOKEN_CODE() \
{\
     size_t i=0;\
\
     UNTESTED_CODE(); \
\
     ASSERT(max_len != 0);\
\
     while (IS_WHITE_SPACE(**input_line_ptr))\
     {\
         if (**input_line_ptr == '\0') break;\
\
         ++(*input_line_ptr); \
     }\
\
\
     --max_len;  /* Account for room needed for NULL */ \
\
     while ( (**input_line_ptr != '\0') &&\
             (isalpha((int)(**input_line_ptr))) &&\
             ( i < max_len ))\
     {\
         *output_string = **input_line_ptr; \
         ++ output_string; \
         ++i; \
         ++ ( *input_line_ptr ); \
     }\
     *output_string = '\0'; \
\
     return i;  /* i == 0 == NO_MORE_TOKENS <-->no more tokens */ \
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

size_t alpha_get_token
(
    char** input_line_ptr,
    char*  output_string,
    size_t max_len
)
ALPHA_GET_TOKEN_CODE()

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

size_t const_alpha_get_token
(
    const char** input_line_ptr,
    char*        output_string,
    size_t       max_len
)
ALPHA_GET_TOKEN_CODE()

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*
 * =============================================================================
 *                                gen_get_last_token
 *
 * Parse backwards
 *
 * The following may be hard to understand, but it is essentially the same thing
 * as gen_get_token, except that things start at the end and go to the begining.
 *
 * The string input_line is parsed from the end towards the begining. First all
 * characters in "terminators" are ignored until a chacter not in terminators
 * is found. Then the characters are checked for of the charcters in
 * "terminators". If one if found, or if the begining is reached, then the
 * characters not in terminators that have been traversed are copied
 * (forwards!) into output_string, provided that there are less than max_len of
 * them. If there are more than max_len-1, then only max_len-1 are copied.
 * Output_string is then terminated by NULL. A NULL is also placed into the
 * appropriate place of input_line so that the parsed characters are "removed"
 * from the end.
 *
 * Macros:
 *     BUFF_GEN_GET_LAST_TOKEN, BUFF_GEN_GET_LAST_TOKEN_OK
 *
 * Returns:
 *     STILL_MORE_TOKENS (1) if there is still more to parse, NO_MORE_TOKENS (0)
 *     if there is no more, and ERROR if there is an error (e.g., buffer
 *     overflow issue). 
 *
 * Note:
 *    The documentation for BUFF_GET_TOKEN has a short example of which
 *    illustrates the general use of this grouup of parsing routines.
 *
 * Related:
 *     BUFF_GEN_GET_LAST_TOKEN, gen_get_last_token_2, BUFF_GEN_GET_LAST_TOKEN_2,
 *     gen_split_at_last_token
 *
 * Index: parsing
 *
 * -----------------------------------------------------------------------------
 */

int gen_get_last_token(char* input_line, char* output_string,
                       size_t max_len, const char* terminators)
{

    /* Careful with KJB IO in this routine as KJB IO uses it. */

    return gen_get_last_token_2(input_line, output_string, 
                                max_len, terminators, TRIM_BEFORE);
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*
 * =============================================================================
 *                                gen_get_last_token_2
 *
 * Parse backwards
 *
 * The following may be hard to understand, but it is essentially the same
 * thing as gen_get_token, except that things are happening from the end to the
 * begining.
 *
 * The string input_line is parsed from the end towards the begining. First all
 * characters in "terminators" are ignored until a chacter not in terminators
 * is found. Then the characters are checked for of the charcters in
 * "terminators". If one if found, or if the begining is reached, then the
 * characters not in terminators that have been traversed are copied
 * (forwards!) into output_string, provided that there are less than max_len of
 * them. If there are more than max_len-1, then only max_len-1 are copied.
 * Output_string is then terminated by NULL. A NULL is also placed into the
 * appropriate place of input_line so that the parsed characters are "removed"
 * from the end.
 *
 * Macros:
 *     BUFF_GEN_GET_LAST_TOKEN, BUFF_GEN_GET_LAST_TOKEN_OK
 *
 * Returns:
 *     STILL_MORE_TOKENS (1) if there is still more to parse, NO_MORE_TOKENS (0)
 *     if there is no more, and ERROR if there is an error (e.g., buffer
 *     overflow issue). 
 *
 * Note:
 *    We used to return the number of characters copied into output string, but
 *    if we use TRIM_AFTER, it is possible that we are interested in parsing out
 *    the empty string, but the associated zero return would not mean that we
 *    are done. For example, if we are spliting a file from a path, and the path
 *    has a trailing slash, then we get an empty string with TRIM_AFTER. But
 *    since we would then remove terminators, we would have a non-zero return
 *    (unless the input string was already empty). 
 *
 * Related:
 *     BUFF_GEN_GET_LAST_TOKEN_2, gen_get_last_token, BUFF_GEN_GET_LAST_TOKEN,
 *     gen_split_at_last_token
 *
 *
 * Index: parsing
 *
 * -----------------------------------------------------------------------------
 */

int gen_get_last_token_2(char* input_line, char* output_string,
                         size_t max_len, const char* terminators,
                         Trim_order_t trim_order_flag)
{
     char*  end_ptr;
     size_t len = strlen(input_line); 

     /* Careful with KJB IO in this routine as KJB IO uses it. */

     if (trim_order_flag == TRIM_BEFORE)
     {
         int terminators_removed = gen_trim_end(input_line, terminators);
         len -= terminators_removed;
     }

     end_ptr = input_line + len; 

     if (end_ptr == input_line)
     {
         ASSERT(*end_ptr == '\0');

         if (output_string != NULL)
         {
             if (max_len < 1) { SET_BUFFER_OVERFLOW_BUG(); return ERROR; } 
             *output_string = '\0'; 
         }
         return ((len > 0) ? STILL_MORE_TOKENS : NO_MORE_TOKENS);
     }

     while (    (end_ptr != input_line) 
             && (FIND_CHAR_NO(terminators, *(end_ptr - 1))) 
           )
     {
         end_ptr--;
         len--; 
     }

     if (output_string != NULL)
     {
         ERE(kjb_buff_cpy(output_string, end_ptr, max_len));
     }

     *end_ptr = '\0';

     if (trim_order_flag == TRIM_AFTER)
     {
         int terminators_removed = gen_trim_end(input_line, terminators);
         len -= terminators_removed;
     }

     return ((len > 0) ? STILL_MORE_TOKENS : NO_MORE_TOKENS);
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*
 * =============================================================================
 *                                gen_split_at_last_token
 *
 * Parse backwards
 *
 * This routine is like gen_get_last_token except that we copy the strings on
 * either side of the parse point rather than change the string being parsed. If
 * we cannot split the string, the first result buffer will be a zero length
 * string (i.e., we go backward). 
 *
 * Macros:
 *     BUFF_GEN_SPLIT_AT_LAST_TOKEN, BUFF_GEN_SPLIT_AT_LAST_TOKEN_OK
 *
 * Returns:
 *     STILL_MORE_TOKENS (1) if there is still more to parse, NO_MORE_TOKENS (0)
 *     if there is no more, and ERROR if there is an error (e.g., buffer
 *     overflow issue). 
 *
 * Related:
 *     BUFF_SPLIT_AT_LAST_TOKEN, gen_get_last_token_2,
 *     BUFF_GEN_GET_LAST_TOKEN_2, gen_get_last_token, BUFF_GEN_GET_LAST_TOKEN,
 *
 *
 * Index: parsing
 *
 * -----------------------------------------------------------------------------
 */

int gen_split_at_last_token(const char* input_line, char* left_str, 
                            size_t left_str_size, char* right_str, size_t right_str_size,
                            const char* terminators)
{

    return gen_split_at_last_token_2(input_line, left_str, left_str_size, right_str, 
                                     right_str_size, terminators, TRIM_BEFORE);
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*
 * =============================================================================
 *                                gen_split_at_last_token_2
 *
 * Parse backwards
 *
 * This routine is like gen_get_last_token_2 except that we copy the strings on
 * either side of the parse point rather than change the string being parsed. If
 * we cannot split the string, the first result buffer will be a zero length
 * string (i.e., we go backward). 
 *
 * Macros:
 *     BUFF_GEN_SPLIT_AT_LAST_TOKEN, BUFF_GEN_SPLIT_AT_LAST_TOKEN_OK
 *
 * Returns:
 *     STILL_MORE_TOKENS (1) if there is still more to parse, NO_MORE_TOKENS (0)
 *     if there is no more, and ERROR if there is an error (e.g., buffer
 *     overflow issue). 
 *
 * Related:
 *     BUFF_SPLIT_AT_LAST_TOKEN, gen_get_last_token_2,
 *     BUFF_GEN_GET_LAST_TOKEN_2, gen_get_last_token, BUFF_GEN_GET_LAST_TOKEN,
 *
 *
 * Index: parsing
 *
 * -----------------------------------------------------------------------------
 */

int gen_split_at_last_token_2(const char* input_line, char* left_str, size_t left_str_size, 
                              char* right_str, size_t right_str_size, const char* terminators,
                              Trim_order_t trim_order_flag)
{
    char* input_line_copy; 
    int result = NOT_SET; 

    NRE(input_line_copy = kjb_strdup(input_line)); 

    result = gen_get_last_token_2(input_line_copy, right_str, right_str_size, 
                                  terminators, trim_order_flag);
     
    if ((result != ERROR) && (left_str != NULL))
    {
        if (kjb_buff_cpy(left_str, input_line_copy, left_str_size) == ERROR)
        {
            result = ERROR; 
        }
    }

    kjb_free(input_line_copy); 

    return result; 
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                              parse_on_string
 *
 * Copies characters up to a search string.
 *
 * This routine consumes characters of an input string up to a search string, or
 * all characters if the search string is not a substring. Consumed characters
 * are copied to the buffer output_string, stopping if the buffer size "max_len"
 * is reached. Regardless, the copied string is always NULL terminated. 
 *
 * Returns:
 *     The number of characters copied to the buffer excluding the NULL. This
 *     means that if there are no characters left, then NO_MORE_TOKENS is
 *     returned which is #defined as 0. 
 *
 * Index: parsing
 *
 * -----------------------------------------------------------------------------
*/

size_t parse_on_string
(
    char**      input_line_ptr,
    char*       output_string,
    size_t      max_len,
    const char* search_str
)
{
    size_t str_loc;

    UNTESTED_CODE(); 

    ASSERT(max_len != 0);
#if 0 /* was ifdef OBSOLETE */
    /* This is not quite right because we use kjb_strncpy() which bumps one from
     * max_len already.
    */
#endif 
     --max_len;  /* Account for room needed for NULL */


    str_loc = find_string(*input_line_ptr, search_str);

    if (str_loc == 0)
    {
        str_loc = strlen(*input_line_ptr);
    }
    else
    {
        str_loc--;
    }

    kjb_strncpy(output_string, *input_line_ptr,
                MIN_OF(max_len, str_loc + 1));

    (*input_line_ptr) += str_loc;

    /* 
     * TODO 
     *
     * Perhaps kjb_strncpy() should return the number of copied characters, and
     * then we would use that.
    */
    return MIN_OF(max_len-1, str_loc); 
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

void unquote_strncpy(char* s1, const char* s2, size_t max_len)
{
    size_t i=0;


    ASSERT(max_len != 0);

    --max_len;  /* Account for room needed by null. */

    while ( (i<max_len) && (*s2 != '\0') )
    {
       if (*s2 != '"')
       {
           *s1 = *s2;
           i++;
           s1++;
        }
       s2++;
   }
    *s1 = '\0';
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

void strip_quotes(char* input_string)
{
    char* new_input_string;
    Bool quote_found = FALSE;


    new_input_string = input_string;

    while (*input_string != '\0')
    {
        if (*input_string == '"')
        {
             quote_found = TRUE;
          }
         else
         {
             if (quote_found)
             {
                 *new_input_string = *input_string;
              }
              new_input_string++;
          }
         input_string++;
     }

    if (quote_found)
    {
         *new_input_string = *input_string;
     }
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

int all_blanks(const char* input_string)
{


    while ( (*input_string == ' ') && (*input_string != '\0'))
    {
        ++input_string;
    }

    if (*input_string == '\0') return TRUE;
    else return FALSE;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

int all_white_space(const char* input_string)
{


    while ((IS_WHITE_SPACE(*input_string)) && (*input_string != '\0'))
    {
        ++input_string;
    }

    if (*input_string == '\0') return TRUE;
    else return FALSE;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

int all_digits(const char* string)
{


    while (*string != '\0')
    {
        if ((*string < '0') || (*string > '9')) return FALSE;
        ++string;
    }
    return TRUE;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

int all_n_digits(const char* string, size_t len)
{


    while ((*string != '\0')  && (len > 0))
    {
        if ((*string < '0') || (*string > '9')) return FALSE;
        ++string;
        --len;
    }
    return TRUE;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */


int kjb_parse(const char* input_string, char*** args_ptr)
{
    int    i;
    int    num_args;
    char** args;
    char   input_copy[ 2000 ];
    char*  input_pos;


    num_args = count_tokens(input_string);

    if (num_args == 0)
    {
        return num_args;
    }

    if (strlen(input_string) >= sizeof(input_copy))
    {
        set_error("String is too long to parse.");
        add_error("It is limited to %lu characters.",
                  (unsigned long)(sizeof(input_copy)-1));
        return ERROR;
    }

    BUFF_CPY(input_copy, input_string);

    input_pos = input_copy;

    NRE(args = N_TYPE_MALLOC(char*, num_args));

    for (i=0;i<num_args;++i)
    {
        /*
         * Memory leak on falure. FIX
        */
        NRE(args[ i ] = parse_next(&input_pos));
    }

    *args_ptr = args;

    return num_args;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

size_t count_tokens(const char* input_string)
{
    size_t count;


    count = 0;

    /* Swallow white space */
    while ((*input_string != '\0') && (IS_WHITE_SPACE(*input_string)))
   {
       ++input_string;
   }

    while ( *input_string != '\0' )
    {
        ++count;

        /* Swallow non-blanks */
        while (    (NOT_WHITE_SPACE(*input_string))
                && (*input_string != '\0')
              )
        {
            ++input_string;
        }

        /* Swallow white space */
        while ((*input_string != '\0') && (IS_WHITE_SPACE(*input_string)))
        {
           ++input_string;
       }
    }
    return count;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

size_t gen_count_tokens(const char* input_string, const char* terminators)
{
    size_t count;


    count = 0;

    while (    (*input_string != '\0')
            && (FIND_CHAR_YES(terminators, (int)(*input_string)))
          )
    {
        input_string++;
    }

    while (*input_string != '\0')
    {
        count++;

        while (    (*input_string != '\0')
                && (FIND_CHAR_NO(terminators, (int)(*input_string)))
              )
        {
            input_string++;
        }

        while (    (*input_string != '\0')
                && (FIND_CHAR_YES(terminators, (int)(*input_string)))
              )
        {
            input_string++;
        }
    }

    return count;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

char* parse_next(char** input_string_ptr)
{
    char*  next;
    char*  input_copy;
    size_t next_len;

    while ((**input_string_ptr != '\0') &&
           (IS_WHITE_SPACE(**input_string_ptr)))
    {
       ++ (*input_string_ptr);
    }

    input_copy = *input_string_ptr;

    next_len = get_next_token_len(input_copy);

    NRN(next = STR_MALLOC(next_len + ROOM_FOR_NULL));

    kjb_strncpy(next, input_copy, next_len + ROOM_FOR_NULL);

    next[ next_len ] = '\0';

    *input_string_ptr += next_len;

    return next;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

size_t get_next_token_len (const char* input_string)
{
    size_t count;


    count = 0;

    /* Swallow blanks */
    while ((*input_string != '\0') && (IS_WHITE_SPACE(*input_string)))
    {
       ++input_string;
    }

    /* Swallow non - blanks */
    while (    (*input_string != '\0')
            && (NOT_WHITE_SPACE(*input_string))
          )
    {
       ++input_string;
       ++count;
   }

   return count;
  }

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

int inc_copy_paren_exp(char** input_ptr, char** output_ptr)
{
    int left_paren_count;
    char* save_input;


    save_input = *input_ptr;

    if (**input_ptr != '(') return NO_ERROR;
    else
    {
        left_paren_count = 1;
        **output_ptr = **input_ptr;
        (*input_ptr)++;
        (*output_ptr)++;
    }

    while ((**input_ptr != '\0') &&
           (left_paren_count != 0))
    {
        if (**input_ptr == '(')
        {
            left_paren_count++;
        }
        else if (**input_ptr == ')')
        {
            left_paren_count--;
        }

        **output_ptr = **input_ptr;
        (*input_ptr)++;
        (*output_ptr)++;
    }

    **output_ptr = '\0';

    if (left_paren_count != 0)
    {
        set_error("Mismatched parentheses in %s.", save_input);
        return ERROR;
    }

    else return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

int get_n_pos_int(int num_args, const char** arg_string_ptr, ...)
{
    va_list   ap;
    int i;
    int temp_val, *temp_arg_ptr;
    char arg_buff[ 200 ];


    if (num_args < 1) return NO_ERROR;

    va_start(ap, arg_string_ptr);

    for (i=0; i< num_args; i++)
    {
        if (BUFF_CONST_GET_TOKEN_OK(arg_string_ptr, arg_buff))
        {
            if (ss1pi(arg_buff, &temp_val) == ERROR)
            {
                va_end(ap);
                return ERROR;
            }
        }
        else
        {
            va_end(ap);
            set_error("Missing %d integer args.", (num_args - i));
            return ERROR;
        }

        temp_arg_ptr = (va_arg(ap, int*));

        *temp_arg_ptr = temp_val;
    }

    va_end(ap);

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*
 * =============================================================================
 *                            parse_positive_integer_list
 *
 * Parses a list of integers
 *
 * This routine parses a list of integer designators to create a list of
 * integers between min_value and max_value. The designators are separated by
 * commas. If there are no designators, or the single special designator "*", or
 * the single special designator "all", then all integers betwen min_value and
 * max_value, inclusive are returned. Valid designators include an integer, a
 * range of the form <num>-<num>, and #<num> which requests for <num> random
 * integers.
 *
 * Index: parsing
 *
 * -----------------------------------------------------------------------------
 */

int parse_positive_integer_list(const char* input, int min_value, int max_value,
                                int** output_array_ptr)
{
    int   count;
    int   min;
    int   max;
    int   i;
    int   pass;
    char* input_pos;
    char* save_input_pos;
    char  buff[ 200 ];
    char  max_buff[ 200 ];
    char  min_buff[ 200 ];
    char* buff_pos;
    int*  output_array     = NULL;  /* Keep error checkers happy. */
    int*  output_array_pos = NULL;  /* Keep error checkers happy. */
    char  input_copy[ 20000 ];


    if (min_value > max_value)
    {
        set_bug("Max less than mininum in parse_positive_integer_list.");
        return ERROR;
    }

    if (strlen(input) >= sizeof(input_copy))
    {
        set_error("String of integer designators is too long to parse.");
        add_error("It is limited to %lu characters.",
                  (unsigned long)(sizeof(input_copy)-1));
        return ERROR;
    }

    BUFF_CPY(input_copy, input);

    input_pos = input_copy;
    trim_beg(&input_pos);
    trim_end(input_pos);

    if (    (*input_pos == '\0')
         || (STRCMP_EQ(input_pos, "*"))
         || (IC_STRCMP_EQ(input_pos, "all"))
       )
    {
        NRE(output_array = INT_MALLOC(max_value - min_value + 1));
        output_array_pos = output_array;

        for (i = min_value; i <= max_value; i++)
        {
            *output_array_pos = i;
            output_array_pos++;
        }

        count = max_value - min_value + 1;
    }
    else if (*input_pos == '#')
    {
        int temp_result;

        input_pos++;

        ERE(ss1pi(input_pos, &count));

        NRE(output_array = INT_MALLOC(count));

        temp_result = get_random_integer_list(count, min_value, max_value,
                                              output_array);

        if (temp_result == ERROR)
        {
            kjb_free(output_array);
            return ERROR;
        }
    }
    else
    {
        count = 0;
        save_input_pos = input_pos;

        for (pass = 1; pass <= 2; pass++)
        {
            if (pass == 2)
            {
                input_pos = save_input_pos;
                NRE(output_array = INT_MALLOC(count));
                output_array_pos = output_array;
            }

            while (BUFF_GEN_GET_TOKEN_OK(&input_pos, buff, " ,"))
            {
                if (FIND_CHAR_YES(buff, '-'))
                {
                    buff_pos = buff;

                    if (*buff_pos == '-')
                    {
                        set_error("Expecting either a positive integer or ");
                        cat_error("range (eg 4-7) but recieved %q.", buff);
                        return ERROR;
                    }

                    BUFF_GEN_GET_TOKEN(&buff_pos, min_buff, "-");

                    buff_pos++;     /* Skip over the "-"   */

                    if (*buff_pos == '\0')
                    {
                        set_error(
                               "Missing second number after the \"-\" in %q.",
                               buff);
                        return ERROR;
                    }

                    BUFF_CPY(max_buff, buff_pos);

                    ERE(ss1pi(min_buff, &min));
                    ERE(ss1pi(max_buff, &max));

                    if (min > max)
                    {
                        set_error("First number larger than the second in %q.",
                                  buff);
                        return ERROR;
                    }

                    if ((min < min_value) || (max > max_value))
                    {
                        set_error("%q has numbers outside the range (%d-%d).",
                                  buff, min_value, max_value);
                        return ERROR;
                    }

                    if (pass == 1)
                        count += (max - min + 1);
                    else
                    {
                        for (i=min; i<=max; i++)
                        {
                            *output_array_pos = i;
                            output_array_pos++;
                        }
                    }
                }
                else
                {
                    ERE(ss1pi(buff, &min));

                    if ((min > max_value) || (min < min_value))
                    {
                        set_error("%q is outside the range (%d-%d).", buff,
                                  min_value, max_value);
                        return ERROR;
                    }

                    if (pass == 1)
                        count++;
                    else
                    {
                        *output_array_pos = min;
                        output_array_pos++;
                    }
                }
            }
        }
    }

    *output_array_ptr = output_array;

    return count;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*
 * =============================================================================
 *                                  parse_options
 *
 *
 *
 *
 * Index:
 *     set option handling 
 *
 * -----------------------------------------------------------------------------
 */

int parse_options(char* input, char*** option_list_ptr, char*** value_list_ptr)
{
    int    first_time         = TRUE;
    char*  stripped_input;
    char*  stripped_input_pos;
    char** option_list;
    char** option_list_pos;
    char** value_list;
    char** value_list_pos;
    int    max_num_options;
    char   buff[ 5000 ];
    char*  buff_pos;
    char   value_buff[ 5000 ];
    char   option_buff[ 5000 ];
    char*  input_pos;
    int    count = 0;
    char*  pre_processed_input;
    size_t input_size;


    max_num_options = count_char(input, '=');

    *option_list_ptr = NULL;
    *value_list_ptr = NULL;

    if (max_num_options <= 0)
    {
        return count;
    }

    input_size = strlen(input) + ROOM_FOR_NULL;
    
    /*
     * Make buffer bigger than pointer size to avoid tripping ASSERTS.
    */
    input_size = MAX_OF(input_size, 2 * sizeof(char*));

    NRE(stripped_input = STR_MALLOC(input_size));
    stripped_input_pos = stripped_input;
    *stripped_input_pos = '\0';

    pre_processed_input = STR_MALLOC(input_size);
    *pre_processed_input = '\0';

    option_list = N_TYPE_MALLOC(char*, max_num_options);
    value_list = N_TYPE_MALLOC(char*, max_num_options);

    if (    (option_list == NULL) || (value_list == NULL)
         || (pre_processed_input == NULL)
       )
    {
        kjb_free(stripped_input);
        kjb_free(pre_processed_input);
        kjb_free(option_list);
        kjb_free(value_list);
        return ERROR;
    }

    input_pos = input;
    trim_beg(&input_pos);

    if (*input_pos == '=')
    {
        set_error("Expected format is <x>=<y>.");
        kjb_free(stripped_input);
        kjb_free(pre_processed_input);
        kjb_free(option_list);
        kjb_free(value_list);
        return ERROR;
    }

    while (BUFF_GEN_MQ_GET_TOKEN_OK(&input_pos, buff, "="))
    {
        trim_end(buff);
        kjb_strncat(pre_processed_input, buff, input_size);

        trim_beg(&input_pos);

        if (*input_pos == '=')
        {
            kjb_strncat(pre_processed_input, "=", input_size);
            input_pos++;
            trim_beg(&input_pos);
        }

        if (*input_pos == '=')
        {
            set_error("Expected format is <x>=<y>.");
            kjb_free(stripped_input);
            kjb_free(pre_processed_input);
            kjb_free(option_list);
            kjb_free(value_list);
            return ERROR;
        }
    }

    option_list_pos = option_list;
    value_list_pos = value_list;

    input_pos = pre_processed_input;

    while (BUFF_MQ_GET_TOKEN_OK(&input_pos, buff))
    {
        if (FIND_CHAR_YES(buff, '='))
        {
            buff_pos = buff;
            trim_beg(&buff_pos);

            if (*buff_pos == '=')
            {
                free_options(count, option_list, value_list);
                kjb_free(stripped_input);
                set_error("No option before the \"=\" in %q.", buff);
                return ERROR;
            }

            BUFF_GEN_MQ_GET_TOKEN(&buff_pos, option_buff, "=");

            if (! BUFF_GEN_MQ_GET_TOKEN_OK(&buff_pos, value_buff,
                                           "="))
            {
                set_error("No option value after the \"=\" in %q.", buff);
                free_options(count, option_list, value_list);
                kjb_free(stripped_input);
                kjb_free(pre_processed_input);
                return ERROR;
            }

            strip_quotes(option_buff);
            strip_quotes(value_buff);

            *option_list_pos = kjb_strdup(option_buff);
            *value_list_pos = kjb_strdup(value_buff);

            if ((*option_list_pos == NULL) || (*value_list_pos == NULL))
            {
                free_options(count, option_list, value_list);
                kjb_free(stripped_input);
                kjb_free(pre_processed_input);
                kjb_free(*option_list_pos);
                kjb_free(*value_list_pos);
                return ERROR;
            }

            option_list_pos++;
            value_list_pos++;

            count++;
         }
         else
         {
             buff_pos = buff;

             if (! first_time)
             {
                 *stripped_input_pos = ' ';
                 stripped_input_pos++;
             }
             else
             {
                 first_time = FALSE;
             }

             while (*buff_pos)
             {
                 *stripped_input_pos = *buff_pos;
                 stripped_input_pos++;
                 buff_pos++;
             }

             *stripped_input_pos = '\0';
         }
     }

     *option_list_ptr = option_list;
     *value_list_ptr = value_list;

     strcpy(input, stripped_input);

     kjb_free(stripped_input);
     kjb_free(pre_processed_input);

     return count;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*
 * =============================================================================
 *                                  free_options
 *
 *
 *
 *
 * Index:
 *     set option handling 
 *
 * -----------------------------------------------------------------------------
 */

int free_options(int num_options, char** option_list, char** value_list)
{
    int i;
    char** option_list_pos;
    char** value_list_pos;


    if (option_list == NULL)
    {
        return NO_ERROR;
    }

    if (value_list == NULL)
    {
        return NO_ERROR;
    }

    option_list_pos = option_list;
    value_list_pos = value_list;

    for (i=0; i<num_options; i++)
    {
        kjb_free(*option_list_pos);
        kjb_free(*value_list_pos);
        option_list_pos++;
        value_list_pos++;
    }

    kjb_free(option_list);
    kjb_free(value_list);

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*
 * =============================================================================
 *                                  ic_parse_key_words
 *
 *
 *
 *
 * Index:
 *     set option handling 
 *
 * -----------------------------------------------------------------------------
 */

int ic_parse_key_words(char* input, int num_key_words, const char** key_words,
                       int* key_words_found_list, int* key_words_found_index)
{
    int   first_time         = TRUE;
    char* stripped_input;
    char* stripped_input_pos;
    char  buff[ 200 ];
    char* buff_pos;
    char* input_pos;
    int   i;
    int   j;
    int   count;
    int   found;
    int   duplicate;


    NRE(stripped_input = STR_MALLOC(strlen(input) + ROOM_FOR_NULL));

    stripped_input_pos = stripped_input;
    *stripped_input_pos = '\0';

    count = 0;
    input_pos = input;

    if (key_words_found_index != NULL)
    {
        for (j=0; j<num_key_words; j++)
        {
            key_words_found_index[ j ] = FALSE;
        }
    }

    while (BUFF_MQ_GET_TOKEN_OK(&input_pos, buff))
    {
        found = FALSE;
        i = 0;

        while ((i<num_key_words) && (! found))
        {
            if (IC_STRCMP_EQ(key_words[i], buff))
            {
                found = TRUE;

                if (key_words_found_index != NULL)
                {
                    key_words_found_index[ i ] = TRUE;
                }

                if (key_words_found_list != NULL)
                {
                    duplicate = FALSE;

                    for (j=0; j<count; j++)
                    {
                        if (key_words_found_list[j] == i)
                        {
                            kjb_printf("Ignoring duplicate %q.\n", buff);
                            duplicate = TRUE;
                        }
                    }

                    if (! duplicate)
                    {
                        key_words_found_list[count] = i;
                        count++;
                    }
                }
            }

            i++;
        }

        if (! found )
        {
            buff_pos = buff;
            trim_beg(&buff_pos);

            buff_pos = buff;

            if (! first_time)
            {
                *stripped_input_pos = ' ';
                stripped_input_pos++;
            }
            else
            {
                first_time = FALSE;
            }

            while (*buff_pos)
            {
                *stripped_input_pos++ = *buff_pos++;
            }

            *stripped_input_pos = '\0';
        }
    }

    strcpy(input, stripped_input);

    kjb_free(stripped_input);

    return count;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*
 * =============================================================================
 *                                  get_boolean_value
 *
 *
 *
 *
 * Index:
 *     parsing, set option handling 
 *
 * -----------------------------------------------------------------------------
 */

int get_boolean_value(const char* str)
{
    char lc_str[ 200 ];


    EXTENDED_LC_BUFF_CPY(lc_str, str);

    if ((STRCMP_EQ(lc_str, "t")) ||
        (STRCMP_EQ(lc_str, "true")) ||
        (STRCMP_EQ(lc_str, "1")) ||
        (STRCMP_EQ(lc_str, "y")) ||
        (STRCMP_EQ(lc_str, "yes")) ||
        (STRCMP_EQ(lc_str, "on")))
    {
        return TRUE;
    }
    else if ((STRCMP_EQ(lc_str, "f")) ||
             (STRCMP_EQ(lc_str, "false")) ||
             (STRCMP_EQ(lc_str, "0")) ||
             (STRCMP_EQ(lc_str, "n")) ||
             (STRCMP_EQ(lc_str, "no")) ||
             (STRCMP_EQ(lc_str, "nil")) ||
             (STRCMP_EQ(lc_str, "null")) ||
             (STRCMP_EQ(lc_str, "none")) ||
             (STRCMP_EQ(lc_str, "off")))
     {
         return FALSE;
     }
    else
    {
        set_error("%q is an invalid boolean value.", str);
        return ERROR;
    }

}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*
 * =============================================================================
 *                                  get_boolean_value
 *
 *
 *
 *
 * Index:
 *     parsing, set option handling 
 *
 * -----------------------------------------------------------------------------
 */

int is_no_value_word(const char* str)
{
    char lc_str[ 200 ];
    char* lc_str_pos;


    EXTENDED_LC_BUFF_CPY(lc_str, str);
    lc_str_pos = lc_str;

    trim_beg(&lc_str_pos);
    trim_end(lc_str_pos);

    return ((STRCMP_EQ(lc_str_pos, "none")) || (STRCMP_EQ(lc_str_pos, "off")));
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*
 * =============================================================================
 *                            parse_path
 *
 * Strips suffixes and directory from a file name
 *
 * This routine is similar to get_base_path except it also breaks down the path
 * into a leading directory string and a base file name.
 *
 * Returns:
 *   NO_ERROR on success, ERROR on failure.
 *
 * Related:
 *     get_base_path
 *
 * Index: parsing
 *
 * -----------------------------------------------------------------------------
 */

/* FIXME
 *
 * There is redundancy between this routine and the next two. Perhaps some
 * rationalization is in order? 
*/

int parse_path(const char* path, char* dir_str, size_t dir_str_size,
               char* name, size_t name_size)
{
    return gen_split_at_last_token(path, dir_str, dir_str_size, name, name_size, DIR_STR);  
}


/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*
 * =============================================================================
 *                            get_base_name
 *
 * Strips suffixes and directory from a file name
 *
 * This routine is similar to get_base_path except it also breaks down the path
 * into a leading directory string and a base file name.
 *
 * Returns:
 *   NO_ERROR on success, ERROR on failure.
 *
 * Related:
 *     get_base_path
 *
 * Index: parsing
 *
 * -----------------------------------------------------------------------------
 */

int get_base_name(const char* name, char* dir_str, size_t dir_str_size,
                  char* base_name, size_t base_name_size, char* suffix,
                  size_t suffix_size, const char** suffixes)
{
    char path_buff[MAX_FILE_NAME_SIZE ];
    char base_buff[MAX_FILE_NAME_SIZE ];
    int  result;


    ERE(result = get_base_path(name, path_buff, sizeof(path_buff), suffix,
                               suffix_size, suffixes));

    if ((base_name != NULL) || (dir_str != NULL))
    {
        size_t len;

        (void)gen_get_last_token(path_buff, base_buff, sizeof(base_buff),
                                 DIR_STR);

        len = strlen(path_buff);

        while ((len > 0) && (path_buff[ len - 1 ] == DIR_CHAR))
        {
            path_buff[ len - 1 ] = '\0';
            len--;
        }
    }

    if (base_name != NULL)
    {
        kjb_strncpy(base_name, base_buff, base_name_size);
    }

    if (dir_str != NULL)
    {
        kjb_strncpy(dir_str, path_buff, dir_str_size);
    }

    return result;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*
 * =============================================================================
 *                            get_base_path
 *
 * Strips suffix from file path
 *
 * This routine strips the suffix from a file path, where suffix is defined as
 * a period followed by one of the strings in the NULL terminated array of
 * strings in the argument "suffixes". This is useful if you want to strip an
 * image file suffix like ".tiff" from a file name, but you don't want to
 * strip a non image file suffix ending like ".backup".
 *
 * An example of how to set up suffixes is as follows:
 * |     static const char* suffixes[ ] =
 * |     {
 * |         "kiff", "kif", "mid", "miff", "tiff", "tif", "viff", "gif",
 * |          NULL
 * |     };
 *
 * Returns:
 *   NO_ERROR on success, ERROR on failure.
 *
 * Related:
 *   get_base_name
 *
 * Index: parsing
 *
 * -----------------------------------------------------------------------------
 */

int get_base_path(const char* path, char* base_path, size_t base_path_size,
                  char* suffix, size_t suffix_size, const char** suffixes)
{
    size_t      i;
    int         last_dot;
    size_t      base_size;
    const char* suffix_pos;
    const char* input_pos;


    last_dot = NOT_SET;
    i = 0;

    input_pos = path;

    while (*input_pos)
    {
        if (*input_pos == '.')
        {
            last_dot = i;
        }

        i++;
        input_pos++;
    }

    if (last_dot == NOT_SET)
    {
        suffix_pos = input_pos;   /* End of path string. */
        base_size = i + 1;
    }
    else
    {
        ASSERT(last_dot >= 0);

        suffix_pos = path + last_dot + 1;   /* Skip '.' */
        base_size = last_dot + 1;
    }

    if (base_path != NULL)
    {
        if (base_size > base_path_size)
        {
            SET_BUFFER_OVERFLOW_BUG();
            return ERROR;
        }
    }

    if (suffixes == NULL)
    {
        if (base_path != NULL)
        {
            kjb_strncpy(base_path, path, base_size);
        }

        if (suffix != NULL)
        {
            kjb_strncpy(suffix, suffix_pos, suffix_size);
        }
    }
    else
    {
        while (*suffixes != NULL)
        {
            if (STRCMP_EQ(*suffixes, suffix_pos))
            {
                if (base_path != NULL)
                {
                    kjb_strncpy(base_path, path, base_size);
                }

                if (suffix != NULL)
                {
                    kjb_strncpy(suffix, suffix_pos, suffix_size);
                }

                return NO_ERROR;
            }

            suffixes++;
        }

        if (*suffix_pos != '\0')
        {
            set_error("%q is not a known suffix.");
            return ERROR;
        }

        if (base_path != NULL)
        {
            kjb_strncpy(base_path, path, base_path_size);
        }

        if (suffix != NULL)
        {
            suffix[ 0 ] = '\0';
        }
    }

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

static int any_integer_is_positive(size_t num_ints, int* int_array)
{
    size_t i;


    for (i = 0; i < num_ints; i++)
    {
        if (int_array[ i ] > 0) return TRUE;
    }

    return FALSE;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

#ifdef __cplusplus
}
#endif

