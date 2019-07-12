
/* $Id: l_string.c 22170 2018-06-23 23:01:50Z kobus $ */

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
#include "l/l_parse.h"
#include "l/l_io.h"
#include "l/l_string.h"
#include "l/l_verbose.h"

#ifdef __cplusplus
extern "C" {
#endif

/* -------------------------------------------------------------------------- */

/* =============================================================================
 *                                 signed_strlen
 *
 * Returns length of string
 *
 * This routine returns the length of a string as an integer. This is only
 * different from "strlen" in that it returns an int (which is capped at
 * INT_MAX), instead of size_t. It exists mainly to cut down on stupid warnings
 * and unecessary promotions. Of course, sometimes the better solution is to
 * change what is being computed to size_t, if that is what you mean!
 *
 * This function should never be used if the string length can exceed INT_MAX,
 * which on 16 bit OS is not particularly big.
 *
 * If the length of the string exceeds INT_MAX, this is treated as a bug.
 *
 * Index: strings
 *
 * -----------------------------------------------------------------------------
*/

int signed_strlen(const char* str)
{
    int count = 0;

    while ((count < INT_MAX) && (*str))
    {
        str++;
        count++;
    }


#ifdef TEST
    if (count == INT_MAX)
    {
        SET_CANT_HAPPEN_BUG();    /* Not likely, anyway. */
    }
#endif

    return count;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*
 * FIXME: Sure, but this is too complicated? We got into this a long time ago
 * because compilers complain if we send a non constant string to this function.
 * But this should be OK, so I wonder if we tested it correctly (or understood
 * const rules correctly). 
 *
 * We want the exact same code for trim_beg() as const_trim_beg() so we
 * put the code in a macro defined here.
*/
#define TRIM_BEG_CODE() \
{\
    size_t count = 0;\
\
    while (IS_WHITE_SPACE(**string_arg_ptr))\
    {\
        if (**string_arg_ptr == '\0') break;\
\
        ++(*string_arg_ptr); \
        count++;\
    }\
\
    return count;\
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                                 trim_beg
 *
 * Moves a string pointer to the next non white space
 *
 * Destructively resets the begining of a string beyond all leading white space.
 *
 * Example:
 * |
 * |   This code outputs a single line "Hello World" on stdout.
 * |
 * |      char* str = "   Hello World";
 * |      char* str_pos = str;
 * |
 * |      trim_beg(&str_pos);   // Get rid of the three blanks
 * |      put_line(str_pos);    // Print on stdout with new line added
 * |
 *
 * Note:
 *    If the argument is (const char**) then best to use the routine
 *    const_trim_beg() as opposed to using a cast.
 *
 * Returns:
 *    The number of blanks removed.
 *
 * Related:
 *    const_gen_trim_beg, gen_trim_beg, gen_trim_end, const_trim_beg, trim_end, trim_len
 *
 * Index: strings
 *
 * -----------------------------------------------------------------------------
*/

size_t trim_beg
(
     char** string_arg_ptr /* Address of string pointer. */
)
TRIM_BEG_CODE()

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                              const_trim_beg
 *
 * Moves a string pointer to the next non white space
 *
 * Destructively resets the begining of a string beyond all leading white space.
 *
 * Example:
 * |
 * |   This code outputs a single line "Hello World" on stdout.
 * |
 * |      char* str = "   Hello World";
 * |      char* str_pos = str;
 * |
 * |      trim_beg(&str_pos);   // Get rid of the three blanks
 * |      put_line(str_pos);    // Print on stdout with new line added
 * |
 *
 * Returns:
 *    The number of blanks removed.
 *
 * Note:
 *    This routine is the same as trim_beg(), except that the constness of
 *    **string_arg_ptr is made explicit. If the argument is not a double pointer
 *    to const (i.e., it is (char**)), then use the routine trim_beg().  Neither
 *    trim_beg() or const_trim_beg() changes the characters in *string_arg_ptr.
 *
 * Related:
 *    const_gen_trim_beg, gen_trim_beg, gen_trim_end, trim_beg, trim_end, trim_len
 *
 * Index: strings
 *
 * -----------------------------------------------------------------------------
*/

size_t const_trim_beg
(
     const char** string_arg_ptr /* Address of string pointer. */
)
TRIM_BEG_CODE()

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                                 trim_end
 *
 * Trims white space from the end of string
 *
 * Destructively resets the end of a string so that trailing white space is
 * removed.
 *
 * Example:
 * |
 * |  This code outputs a single line "Hello World" on stdout.
 * |
 * |      char* str = "Hello World   ";
 * |      char* str_pos = str;
 * |
 * |      trim_end(str_pos);  // Get rid of the three trailing blanks
 * |      put_line(str_pos);  // Print on stdout with new line added
 * |
 *
 * Returns:
 *    The number of blanks removed.
 *
 * Related:
 *    const_gen_trim_beg, gen_trim_beg, gen_trim_end, const_trim_beg, trim_beg, trim_len
 *
 * Index: strings
 *
 * -----------------------------------------------------------------------------
*/

size_t trim_end(char* string_arg)
{
    char* local_copy;
    int   count      = 0;


    if (string_arg == NULL)
    {
        SET_ARGUMENT_BUG();
        return 0;
    }

    local_copy = string_arg;

    if (*string_arg == '\0') return count;

    while ( *string_arg != '\0')
    {
        ++string_arg;
    }

    --string_arg;

    /*
    // A smilar construct (const_trim_beg) crashes under gcc 3.2 with -O2.
    // Rewrite, optimistically assuming this is the end of this problem.
    //
    while ((IS_WHITE_SPACE(*string_arg)) &&
           (string_arg > local_copy ))
    {
        --string_arg;
        count++;
    }
    */
    while (IS_WHITE_SPACE(*string_arg))
    {
        if (string_arg <= local_copy) break;

        --string_arg;
        count++;
    }

    if (IS_WHITE_SPACE(*string_arg))
    {
        *string_arg = '\0';
        count++;
    }
    else
    {
        *(string_arg + 1) = '\0';
    }

    return count;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                                 trim_len
 *
 * length of string without leading or trailing white space.
 *
 * Returns the length of a string without leading or trailing white space.
 *
 * Related:
 *    trim_end, trim_beg
 *
 * Index: strings
 *
 * -----------------------------------------------------------------------------
*/

size_t trim_len(const char* str)
{
    size_t      result_len;
    const char* str_pos;


    /* Increment outside the macro as not to put side effects into macros.  */ 
    while (IS_WHITE_SPACE(*str)) str++; 

    result_len = strlen(str);

    str_pos = str + result_len;

    while ((str_pos > str) && (IS_WHITE_SPACE(*(str_pos-1))))
    {
        str_pos--;
        result_len--;
    }

    return result_len;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*
 * Based on trim_beg, so synchronize changes with that function.  
 *
 * We want the exact same code for get_trim_beg() as const_gen_trim_beg() so we
 * put the code in a macro defined here.
*/
#define GEN_TRIM_BEG_CODE() \
{\
    size_t count = 0;\
\
    while (FIND_CHAR_YES(terminators, **string_arg_ptr))\
    {\
        if (**string_arg_ptr == '\0') break;\
\
        ++(*string_arg_ptr); \
        count++;\
    }\
\
    return count;\
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                                 gen_trim_beg
 *
 * Moves a string pointer to the next non terminator
 *
 * Destructively resets the begining of a string beyond all leading terminators
 *
 * Note:
 *    If the argument is (const char**) then best to use the routine
 *    const_gen_trim_beg() as opposed to using a cast.
 *
 * Returns:
 *    The number of terminators removed.
 *
 * Related:
 *    const_gen_trim_beg, gen_trim_end, const_trim_beg, trim_end, trim_len
 *
 * Index: strings
 *
 * -----------------------------------------------------------------------------
*/

size_t gen_trim_beg
(
     char** string_arg_ptr, /* Address of string pointer. */
     const char* terminators
)
GEN_TRIM_BEG_CODE()

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                              const_gen_trim_beg
 *
 * Moves a string pointer to the next non terminator
 *
 * Destructively resets the begining of a string beyond all leading terminators
 *
 * Returns:
 *    The number of terminators removed.
 *
 * Note:
 *    This routine is the same as gen_trim_beg(), except that the constness of
 *    **string_arg_ptr is made explicit. If the argument is not a double pointer
 *    to const (i.e., it is (char**)), then use the routine gen_trim_beg().
 *    Neither gen_trim_beg() or const_gen_trim_beg() changes the characters in
 *    *string_arg_ptr.
 *
 * Related:
 *    gen_trim_beg, gen_trim_end, const_trim_beg, trim_beg, trim_end, trim_len
 *
 * Index: strings
 *
 * -----------------------------------------------------------------------------
*/

size_t const_gen_trim_beg
(
     const char** string_arg_ptr, /* Address of string pointer. */
     const char* terminators
)
GEN_TRIM_BEG_CODE()

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                                 gen_trim_end
 *
 * Trims terminators from the end of string
 *
 * Destructively resets the end of a string so that trailing terminators are 
 * removed.
 *
 * Returns:
 *    The number of terminators removed.
 *
 * Related:
 *    const_gen_trim_beg, gen_trim_beg, const_trim_beg, trim_beg, trim_end, trim_len
 *
 * Index: strings
 *
 * -----------------------------------------------------------------------------
*/

size_t gen_trim_end(char* string_arg, const char* terminators)
{
    const char* string_start = string_arg;
    int   count      = 0;

    if (string_arg == NULL)
    {
        SET_ARGUMENT_BUG();
        return 0;
    }


    if (*string_arg == '\0') return count;

    while ( *string_arg != '\0')
    {
        ++string_arg;
    }

    --string_arg;

    while (FIND_CHAR_YES(terminators, *string_arg))
    {
        if (string_arg <= string_start) break;

        --string_arg;
        count++;
    }

    if (FIND_CHAR_YES(terminators, *string_arg))
    {
        *string_arg = '\0';
        count++;
    }
    else
    {
        *(string_arg + 1) = '\0'; 
    }

    return count;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                             extended_uc_lc
 *
 * Translates a string to lower case
 *
 * Destructively changes characters in a NULL terminated string to all lower
 * case.
 *
 * Note:
 *    Underscores are translated to hyphens.
 *
 * Related:
 *    extended_lc_uc, extended_n_uc_lc, extended_n_lc_uc, extended_lc_strncpy,
 *    extended_uc_strncpy, extended_tolower, extended_toupper
 *
 * Index: strings, case conversion
 *
 * -----------------------------------------------------------------------------
*/

void extended_uc_lc(char* string_arg)
{

    while (*string_arg != '\0')
    {
        *string_arg = extended_tolower(*string_arg);
        ++string_arg;
    }
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                             extended_lc_uc
 *
 * Translates a string to upper case
 *
 * Destructively changes characters in a NULL terminated string to all upper
 * case.
 *
 * Note:
 *    Hypens are translated to underscores.
 *
 * Related:
 *    extended_uc_lc, extended_n_uc_lc, extended_n_lc_uc, extended_lc_strncpy,
 *    extended_uc_strncpy, extended_tolower, extended_toupper
 *
 * Index: strings, case conversion
 *
 * -----------------------------------------------------------------------------
*/

void extended_lc_uc(char* string_arg)
{

    while (*string_arg != '\0')
    {
        *string_arg = extended_toupper(*string_arg);
        ++string_arg;
    }
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                             extended_n_uc_lc
 *
 * Translates a string to lower case
 *
 * Destructively changes characters in a string to lower case, until either
 * "len" characters have been translated, OR a NULL has been reached.
 *
 * Note:
 *    Underscores are translated to hyphens.
 *
 * Related:
 *    n_lc_lc, extended_lc_uc, extended_uc_lc, extended_lc_strncpy,
 *    extended_uc_strncpy, extended_tolower, extended_toupper
 *
 * Index: strings, case conversion
 *
 * -----------------------------------------------------------------------------
*/

void extended_n_uc_lc(char* in_string, size_t len)
{


    ASSERT(len != 0);

    while ((*in_string != '\0') && (len > 0))
    {
        *in_string = extended_tolower( *in_string );
        ++in_string;
        len--;
    }
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                             extended_n_lc_uc
 *
 * Translates a string to upper case
 *
 * Destructively changes characters in a string to upper case, until either
 * "len" characters have been translated, OR a NULL has been reached.
 *
 * Note:
 *    Hypens are translated to underscores.
 *
 * Related:
 *    extended_n_uc_lc, extended_lc_uc, extended_uc_lc, extended_lc_strncpy,
 *    extended_uc_strncpy, extended_tolower, extended_toupper
 *
 * Index: strings, case conversion
 *
 * -----------------------------------------------------------------------------
*/

void extended_n_lc_uc(char* in_string, size_t len)
{


    ASSERT(len != 0);

    while ((*in_string != '\0') && (len > 0))
    {
        *in_string = extended_toupper( *in_string );
        ++in_string;
        len--;
    }
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                                 extended_tolower
 *
 * Converts a character to lower case.
 *
 * The reoutine extended_tolower differs from tolower in that it is gaurenteed
 * to return the input even if it is NOT an upper case character, except when it
 * is '_', in which case '-' is returned.
 *
 * Note:
 *    This routine is essentially one presented as "safe_tolower" in Harbison
 *    and Steele, page 321.
 *
 * Related:
 *    extended_lc_uc, extended_uc_lc, extended_n_uc_lc, extended_n_lc_uc,
 *    extended_lc_strncpy, extended_uc_strncpy, extended_toupper
 *
 * Index: strings, case conversion
 *
 * -----------------------------------------------------------------------------
*/

int extended_tolower(int c)
{


    if (isupper(c)) return tolower(c);
    else if (c == '_') return '-';
    else return c;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                                 extended_toupper
 *
 * Converts a character to upper case.
 *
 * The routine extended_toupper differs from tolower in that it is gaurenteed to
 * return the input even if it is NOT a lower case character, except when it is
 * '-' in which case '_' is returned.
 *
 * Note:
 *    This routine is patterned after one presented as "safe_toupper" in
 *    Harbison and Steele, page 321.
 *
 * Related:
 *    extended_lc_uc, extended_n_uc_lc, extended_n_lc_uc, extended_lc_strncpy,
 *    extended_uc_strncpy, extended_tolower
 *
 * Index: strings, case conversion
 *
 * -----------------------------------------------------------------------------
*/

int extended_toupper(int c)
{


    if (islower(c)) return toupper(c);
    else if (c == '-') return '_';
    else return c;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                            void_strcmp
 *
 * Compares the strings pointed to by s1 and s2
 *
 * The routine void_strcmp is similar to kjb_strcmp, except that the arguments
 * are pointers to void which are cast into pointers to characters. Like
 * kjb_strmcp (and unlike the system routine strmp()), the return values are
 * limited to EQUAL_STRINGS (0), FIRST_STRING_GREATER (1), SECOND_STRING_GREATER
 * (-1).
 *
 * Macros:
 *     STRCMP_EQ(s1, s2) evaluates to TRUE if s1 and s2 compare equal with
 *     kjb_strcmp, FALSE otherwise.
 *
 * Returns:
 *     EQUAL_STRINGS (0) if the strings compare equal
 *     FIRST_STRING_GREATER (0) if the first string is greater
 *     SECOND_STRING_GREATER (-1) if the second string is greater
 *
 * Related:
 *     kjb_memcmp, kjb_strncmp, kjb_ic_strcmp, kjb_ic_strncmp, head_cmp,
 *     ic_head_cmp,
 *
 * Index: strings
 *
 * -----------------------------------------------------------------------------
*/

int void_strcmp(const void* s1, const void* s2)
{

    return kjb_strcmp((const char*)s1, (const char*)s2);
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                            kjb_strcmp
 *
 * Compares the strings pointed to by s1 and s2
 *
 * The routine kjb_strcmp is similar to the system routine strcmp, except that
 * the return values are limited to EQUAL_STRINGS (0), FIRST_STRING_GREATER
 * (1), SECOND_STRING_GREATER (-1).
 *
 * Macros:
 *     STRCMP_EQ(s1, s2) evaluates to TRUE if s1 and s2 compare equal with
 *     kjb_strcmp, FALSE otherwise.
 *
 * Returns:
 *     EQUAL_STRINGS (0) if the strings compare equal
 *     FIRST_STRING_GREATER (0) if the first string is greater
 *     SECOND_STRING_GREATER (-1) if the second string is greater
 *
 * Related:
 *     kjb_memcmp, kjb_strncmp, kjb_ic_strcmp, kjb_ic_strncmp, head_cmp,
 *     ic_head_cmp,
 *
 * Index: strings
 *
 * -----------------------------------------------------------------------------
*/

int kjb_strcmp(const char* s1, const char* s2)
{
    /*
    // We want to be consistent with they system's strcmp in the case of 8 bit
    // characters, even if it is arguably broken.
    */
    int compare_res = strcmp(s1, s2);

    if (compare_res > 0)
    {
        return FIRST_STRING_GREATER;
    }
    else if (compare_res < 0)
    {
        return SECOND_STRING_GREATER;
    }
    else
    {
        return EQUAL_STRINGS;
    }

#if 0 /* was ifdef HOW_IT_WAS */

#ifdef TEST
    const char* save_s1 = s1;
    const char* save_s2 = s2;
#endif

    while (*s1 == *s2)
    {
        if (*s1 == '\0')
        {
            return EQUAL_STRINGS;                 /*   0   */
        }

        s1++;
        s2++;
    }

#ifdef TEST
    if (((*s1 == '-') && (*s2 == '_')) || ((*s2 == '-') && (*s1 == '_')))
    {
        TEST_PSE(("Comparison of %q to %q rejected \n", save_s1, save_s2));
        TEST_PSE(("due to non-equivalence of dash and underscore.\n"));
    }
#endif

    if (*s1 > *s2)
    {
        return FIRST_STRING_GREATER;              /*   1   */
    }
    else
    {
        return SECOND_STRING_GREATER;            /*   -1  */
    }
#endif
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                               kjb_memcmp
 *
 * Compares two regions of memory Byte-wise
 *
 * kjb_memcmp is similar to the system supplied routine memcmp, except that the
 * return values are limited to EQUAL_STRINGS (0), FIRST_STRING_GREATER (1),
 * SECOND_STRING_GREATER (-1).
 *
 * Returns:
 *     EQUAL_STRINGS (0) if the strings compare equal
 *     FIRST_STRING_GREATER (0) if the first string is greater
 *     SECOND_STRING_GREATER (-1) if the second string is greater
 *
 * Note:
 *   System supplied "mem" routines are normally advertised to be efficient.
 *   This routine may or may NOT be implemented using these routines. In the
 *   later case, no claims to efficiency are made.
 *
 * Related:
 *     kjb_strcmp, kjb_strncmp, kjb_ic_strcmp, kjb_ic_strncmp, head_cmp,
 *     ic_head_cmp,
 *
 * Index: strings
 *
 * -----------------------------------------------------------------------------
*/

int kjb_memcmp(const char* s1, const char* s2, size_t len)
{
    /*
    // We want to be consistent with they system's strncmp in the case of 8 bit
    // characters, even if it is arguably broken.
    */
    int compare_res = memcmp(s1, s2, len);

    if (compare_res > 0)
    {
        return FIRST_STRING_GREATER;
    }
    else if (compare_res < 0)
    {
        return SECOND_STRING_GREATER;
    }
    else
    {
        return EQUAL_STRINGS;
    }

#if 0 /* was ifdef HOW_IT_WAS */
    ASSERT(len != 0);

    while (len > 0)
    {
        if (*s1 > *s2)      return FIRST_STRING_GREATER;
        else if (*s1 < *s2) return SECOND_STRING_GREATER;

        s1++;
        s2++;
        len--;
    }

    return EQUAL_STRINGS;
#endif

}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                               kjb_strncmp
 *
 * Compares string s1 to s2 up to a specified number of characters
 *
 * Compares the strings pointed to by s1 and s2. A maximun of maxlen characters
 * are compared. Similar to the system routine strncmp, except that the return
 * values are limited to EQUAL_STRINGS (0), FIRST_STRING_GREATER (1), or
 * SECOND_STRING_GREATER (-1).
 *
 * Returns:
 *     EQUAL_STRINGS (0) if the strings compare equal
 *     FIRST_STRING_GREATER (0) if the first string is greater
 *     SECOND_STRING_GREATER (-1) if the second string is greater
 *
 * Macros:
 *     STRNCMP_EQ(s1, s2, max_len) evaluates to TRUE if s1 and s2 compare equal
 *     with kjb_strncmp, FALSE otherwise.
 *
 * Related:
 *     kjb_strcmp, kjb_memcmp, kjb_ic_strcmp, kjb_ic_strncmp, head_cmp,
 *     ic_head_cmp,
 *
 * Index: strings
 *
 * -----------------------------------------------------------------------------
*/

int kjb_strncmp(const char* s1, const char* s2, size_t max_len)
{
    /*
    // We want to be consistent with they system's strncmp in the case of 8 bit
    // characters, even if it is arguably broken.
    */
    int compare_res = strncmp(s1, s2, max_len);

    if (compare_res > 0)
    {
        return FIRST_STRING_GREATER;
    }
    else if (compare_res < 0)
    {
        return SECOND_STRING_GREATER;
    }
    else
    {
        return EQUAL_STRINGS;
    }

#if 0 /* was ifdef HOW_IT_WAS */

    ASSERT(max_len != 0);

    while ((*s1 == *s2) && (max_len > 0))
    {
        if (*s1 == '\0') return EQUAL_STRINGS;          /*   0   */

        s1++;
        s2++;
        max_len--;
    }

    if (max_len == 0)   return EQUAL_STRINGS;           /*   0   */
    else if (*s1 > *s2) return FIRST_STRING_GREATER;    /*   1   */
    else                return SECOND_STRING_GREATER;   /*   -1  */
#endif

}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                               kjb_ic_strcmp
 *
 * Compares string s1 to s2 ignoring case
 *
 * kjb_ic_strcmp is similar to kjb_strncmp, except that case is ignored.
 *
 * Returns:
 *     EQUAL_STRINGS (0) if the strings compare equal
 *     FIRST_STRING_GREATER (0) if the first string is greater
 *     SECOND_STRING_GREATER (-1) if the second string is greater
 *
 * Macros:
 *     IC_STRCMP_EQ_EQ(s1,s2) evaluates to TRUE if s1 and s2 compare equal with
 *     kjb_ic_strcmp, FALSE otherwise.
 *
 * Related:
 *     kjb_strcmp, kjb_memcmp, kjb_strncmp, kjb_ic_strncmp, head_cmp,
 *     ic_head_cmp,
 *
 * Index: strings, case conversion
 *
 * -----------------------------------------------------------------------------
*/

int kjb_ic_strcmp(const char* s1, const char* s2)
{

    /*
    // Could be inconsistent with system string compares if chars are not 7
    // bits.
    */

    while (extended_tolower(*s1) == extended_tolower(*s2))
    {
        if (*s1 == '\0')
        {
            return EQUAL_STRINGS;
        }
        ++s1;
        ++s2;
    }

    if (extended_tolower(*s1) > extended_tolower(*s2))
    {
        return FIRST_STRING_GREATER;
    }
    else
    {
        return SECOND_STRING_GREATER;
    }
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                               kjb_ic_strncmp
 *
 * Compares string s1 to s2 up to a max_len characters ignoring case
 *
 * kjb_ic_strncmp is similar to kjb_strcmp, except that case is ignored.
 *
 * Returns:
 *     EQUAL_STRINGS (0) if the strings compare equal
 *     FIRST_STRING_GREATER (0) if the first string is greater
 *     SECOND_STRING_GREATER (-1) if the second string is greater
 *
 * Macros:
 *     IC_STRCMP_EQ(s1,s2) evaluates to TRUE if s1 and s2 compare equal with
 *     kjb_ic_strcmp, FALSE otherwise.
 *
 * Related:
 *     kjb_strcmp, kjb_memcmp, kjb_strncmp, kjb_ic_strcmp, head_cmp,
 *     ic_head_cmp,
 *
 * Index: strings, case conversion
 *
 * -----------------------------------------------------------------------------
*/

int kjb_ic_strncmp(const char* s1, const char* s2, size_t max_len)
{


    ASSERT(max_len != 0);

    /*
    // Could be inconsistent with system string compares if chars are not 7
    // bits.
    */
    UNTESTED_CODE();

    while ((max_len > 0) && (extended_tolower(*s1) == extended_tolower(*s2)))
    {
        if (*s1 == '\0') return EQUAL_STRINGS;

        ++s1;
        ++s2;
        --max_len;
    }

    if (max_len == 0)
    {
        return EQUAL_STRINGS;
    }
    else if (extended_tolower(*s1) > extended_tolower(*s2))
    {
        return FIRST_STRING_GREATER;
    }
    else
    {
        return SECOND_STRING_GREATER;
    }
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                                head_cmp
 *
 * Compares the begining of input with the string test_head
 *
 * head_cmp compares the strings pointed to by input and test head, up until
 * the end of test_head. In effect, this routine tests if the first part of
 * input is the same as test_head. It returns EQUAL_HEADS 90),
 * FIRST_STRING_GREATER (1), or SECOND_STRING_GREATER (-1).
 *
 * Returns:
 *     EQUAL_HEADS (0) if the strings compare equal
 *     FIRST_STRING_GREATER (0) if the first string is greater
 *     SECOND_STRING_GREATER (-1) if the second string is greater
 *
 * Macros:
 *     HEAD_CMP_EQ(s1,s2) evaluates to TRUE if s1 and s2 compare equal with
 *     head_strcmp, FALSE otherwise.
 *
 * Related:
 *     kjb_strcmp, kjb_memcmp, kjb_strncmp, kjb_ic_strcmp, kjb_ic_strncmp,
 *     ic_head_cmp,
 *
 * Index: strings
 *
 * -----------------------------------------------------------------------------
*/

int head_cmp(const char* input, const char* test_head)
{


    /*
    // Could be inconsistent with system string compares if chars are not 7
    // bits.
    */

    while ((*input != '\0') && (*test_head != '\0') && (*input == *test_head))
    {
        ++input;
        ++test_head;
    }

    if (*test_head == '\0')          return EQUAL_HEADS;
    else if (*input == '\0')         return SECOND_STRING_GREATER;
    else if (*test_head > *input)    return SECOND_STRING_GREATER;
    else                             return FIRST_STRING_GREATER;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                              ic_head_cmp
 *
 * Compares the begining of input with test_head ignoring case
 *
 * ic_head_cmp is similar to head_cmp, except that case is ignored.
 *
 * Returns:
 *     EQUAL_HEADS (0) if the strings compare equal
 *     FIRST_STRING_GREATER (0) if the first string is greater
 *     SECOND_STRING_GREATER (-1) if the second string is greater
 *
 * Macros:
 *     IC_HEAD_CMP_EQ(s1,s2) evaluates to TRUE if s1 and s2 compare equal with
 *     head_strcmp, FALSE otherwise.
 *
 * Related:
 *     kjb_strcmp, kjb_memcmp, kjb_strncmp, kjb_ic_strcmp, kjb_ic_strncmp,
 *     head_cmp,
 *
 * Index: strings, case conversion
 *
 * -----------------------------------------------------------------------------
*/

int ic_head_cmp(const char* input, const char* test_head)
{

    /*
    // This routine could be inconsistent with system string compares if chars
    // are not 7 bits. (Not very well tested). 
    */

    while ((*input != '\0') && (*test_head != '\0') &&
           (extended_tolower(*input) == extended_tolower(*test_head)))
    {
        ++input;
        ++test_head;
    }

    if (*test_head == '\0')
    {
        return EQUAL_HEADS;
    }
    else if (*input == '\0')
    {
        return SECOND_STRING_GREATER;
    }
    else if (extended_tolower(*test_head) > extended_tolower(*input))
    {
        return SECOND_STRING_GREATER;
    }
    else return FIRST_STRING_GREATER;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

#ifdef METHOD_ONE

/* =============================================================================
 *                          ptr_strcmp
 *
 *
 *
 * Index: strings
 *
 * -----------------------------------------------------------------------------
*/

int ptr_strcmp(const char** str_ptr1, const char** str_ptr2)
{


    return kjb_strcmp(*str_ptr1, *str_ptr2);
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                          ptr_strncmp
 *
 *
 *
 * Index: strings
 *
 * -----------------------------------------------------------------------------
*/

int ptr_strncmp(const char** str_ptr1,  const char** str_ptr2, size_t len)
{


    return kjb_strncmp(*str_ptr1, *str_ptr2, len);
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                          ptr_ic_strcmp
 *
 *
 * Index: strings
 *
 *
 * -----------------------------------------------------------------------------
*/

int ptr_ic_strcmp(const char** str_ptr1, const char** str_ptr2)
{


    return kjb_ic_strcmp(*str_ptr1, *str_ptr2);
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                              ptr_head_cmp
 *
 *
 *
 * Index: strings
 *
 * -----------------------------------------------------------------------------
*/

int ptr_head_cmp(const char** s1_ptr, const char** s2_ptr)
{

    return head_cmp(*s1_ptr, *s2_ptr);
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                               ptr_ic_head_cmp
 *
 *
 *
 * Index: strings
 *
 * -----------------------------------------------------------------------------
*/

int ptr_ic_head_cmp(const char** s1_ptr, const char** s2_ptr)
{

    return ic_head_cmp(*s1_ptr, *s2_ptr);
}

#else

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                          ptr_strcmp
 *
 *
 *
 * Index: strings
 *
 * -----------------------------------------------------------------------------
*/

int ptr_strcmp(const void* str_ptr1, const void* str_ptr2)
{


    return kjb_strcmp(*((const char* const*)str_ptr1),
                      *((const char* const*)str_ptr2));
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                          ptr_strncmp
 *
 *
 *
 * Index: strings
 *
 * -----------------------------------------------------------------------------
*/

int ptr_strncmp(const void* str_ptr1, const void* str_ptr2, size_t len)
{


    return kjb_strncmp(*((const char* const*)str_ptr1),
                       *((const char* const*)str_ptr2),
                       len);
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                          ptr_ic_strcmp
 *
 *
 *
 * Index: strings
 *
 * -----------------------------------------------------------------------------
*/

int ptr_ic_strcmp(const void* str_ptr1, const void* str_ptr2)
{


    return kjb_ic_strcmp(*((const char* const*)str_ptr1),
                         *((const char* const*)str_ptr2));
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                              ptr_head_cmp
 *
 *
 *
 * Index: strings
 *
 * -----------------------------------------------------------------------------
*/

int ptr_head_cmp(const void* s1_ptr, const void* s2_ptr)
{

    return head_cmp(*((const char* const *)s1_ptr),
                    *((const char* const *)s2_ptr));
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                               ptr_ic_head_cmp
 *
 *
 *
 * Index: strings
 *
 * -----------------------------------------------------------------------------
*/

int ptr_ic_head_cmp(const void* s1_ptr, const void* s2_ptr)
{

    return ic_head_cmp(*((const char* const *)s1_ptr),
                       *((const char* const *)s2_ptr));
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

#endif


/* =============================================================================
 *                               rpad
 *
 *
 *
 * Index: strings
 *
 * -----------------------------------------------------------------------------
*/

void rpad(char* string, size_t cur_len, size_t total_len)
{
    size_t i;


    ASSERT(total_len != 0);

    string += cur_len;

    for (i=cur_len; i<total_len; ++i)
    {
        *string = ' ';
        string++;
    }
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                               rpad_cpy
 *
 *
 *
 * Index: strings
 *
 * -----------------------------------------------------------------------------
*/

void rpad_cpy(char* target_string, const char* source_string, size_t pad_len)
{


    ASSERT(pad_len != 0);

    while ((pad_len > 0) && (*source_string != '\0'))
    {
        *target_string = *source_string;
        ++target_string; ++ source_string;
        --pad_len;
    }

    while (pad_len > 0)
    {
        *target_string = ' ';
        ++target_string;
        --pad_len;
    }

    *target_string = '\0';
}


/* =============================================================================
 *                                kjb_strdup
 *
 * Duplicates a string
 *
 * This routine allocates enough storage for a copy of the input string, and
 * then copies the input string. The newly allocated string is returned.
 *
 * If TEST is #defined (unix only) then this routine is #define'd to be
 * debug_kjb_str_dup, which is the version available in the
 * development library. In development code memory is tracked so that memory
 * leaks can be found more easily. Furthermore, all memory freed is checked
 * that it was allocated by an L library routine.
 *
 * The routine kjb_free (accessed via the macro kjb_free) should be used to
 * dispose of the storage once it is no longer needed.
 *
 * It is an error for the input_string pointer to equal NULL.
 *
 * Returns:
 *    On error, this routine returns NULL, with an error message being set.
 *    One success it returns a pointer to the duplicated string.
 *
 * Related:
 *    kjb_malloc, print_allocated_memory, kjb_free
 *
 * Index: memory allocation, memory tracking, strings
 *
 * -----------------------------------------------------------------------------
*/

#ifdef TRACK_MEMORY_ALLOCATION

char* debug_kjb_str_dup(const char* input_string,
                        const char* file_name, int line_number)
{
    size_t input_string_len;
    char*  string_copy;

    NRN(input_string);

    input_string_len = strlen(input_string);

    NRN(string_copy = (char*)debug_kjb_malloc(input_string_len + ROOM_FOR_NULL,
                                              file_name, line_number));

    strcpy(string_copy, input_string);

    return string_copy;
}

/*  ==>                               /\                              */
/*  ==>  Development code above       ||                              */
#else   /* ------------------------------------------------------------------ */
/*  ==>  Production code below        ||                              */
/*  ==>                               \/                              */


char* kjb_strdup(const char* input_string)
{
    size_t input_string_len;
    char*  string_copy;


    NRN(input_string);

    input_string_len = strlen(input_string);

    NRN(string_copy = STR_MALLOC(input_string_len + ROOM_FOR_NULL));

    strcpy(string_copy, input_string);

    return string_copy;
}

#endif    /*   #ifdef TRACK_MEMORY_ALLOCATION ... #else ...   */

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                               str_trunc_cpy
 *
 * Copies a string, truncating if needed
 *
 * This routine copies a string without exceeding the buffer size, adding a
 * convenient "..." if the result is truncated. 
 *
 * Returns:
 *    None (void function)
 *
 * Bugs: 
 *    This routine should probably return TRUNCATED if the entire buffer is used
 *    and we still want to keep copying. 
 *
 * Related:
 *    kjb_strncpy, kjb_buff_cpy, str_trunc_cat, trunc_quote_cpy
 *
 * Index: strings
 *
 * -----------------------------------------------------------------------------
*/

/*
 * FIXME  TODO
 *
 * See kjb_strncpy. 
*/

void str_trunc_cpy(char* output_string,
                   const char* input_string,
                   size_t max_len)
{
    size_t cur_len;

    cur_len = strlen(input_string);

    /* We could later choose to do nothing if we are given nothing. But for now,
     * we at least need room for null.
    */

    ASSERT(max_len >= ROOM_FOR_NULL);

    if (max_len == ROOM_FOR_NULL)
    {
        *output_string = '\0';
    }
    else if (cur_len < max_len)
    {
        kjb_strncpy(output_string, input_string, cur_len + ROOM_FOR_NULL);
    }
    else if (max_len > 3)
    {
        kjb_strncpy(output_string, input_string, max_len-3);
        kjb_strncat(output_string, "...", max_len);
    }
    else
    {
        *output_string = '\0';
    }
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                               str_trunc_cat
 *
 * Concatenates strings, truncating if needed.
 *
 * This routine catenates one string to another without exceeding the buffer
 * size, adding a convenient "..." if the result is truncated. 
 *
 * Returns:
 *    None (void function)
 *
 * Bugs: 
 *    This routine should probably return TRUNCATED if the entire buffer is used
 *    and we still want to keep copying. 
 *
 * Related:
 *    kjb_strncpy, kjb_buff_cpy, str_trunc_cp, trunc_quote_cpy
 *
 * Index: strings
 *
 * -----------------------------------------------------------------------------
*/

/*
 * FIXME  TODO
 *
 * See kjb_strncpy. 
*/

void str_trunc_cat(char* output_string,
                   const char* input_string,
                   size_t max_len)
{
    size_t cur_len;

    cur_len = strlen(output_string);
    max_len -= cur_len;

    if (max_len < 1)
    {
        ; /* EMPTY */
    }
    else 
    {
        str_trunc_cpy(output_string + cur_len, input_string, max_len);
    }

#ifdef OLD_WAY
    if (max_len < 1)
    {
        ; /* EMPTY */
    }
    else if (max_len == 1)
    {
        *output_string = '\0';
    }
    else if (cur_len < max_len)
    {
        kjb_strncpy(output_string, input_string, cur_len + ROOM_FOR_NULL);
    }
    else if (max_len > 4) /* Too big. Must truncate. */
    {
        kjb_strncpy(output_string, input_string, max_len-3);
        kjb_strncat(output_string, "...", max_len);
    }
    else /* No room to add the dots. */
    {
        *output_string = '\0';
    }
#endif 
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                             trunc_quote_cpy
 *
 * Index: strings
 *
 * Bugs: 
 *    This routine should probably return TRUNCATED if the entire buffer is used
 *    and we still want to keep copying. 
 *
 * -----------------------------------------------------------------------------
*/

/*
 * FIXME  TODO
 *
 * See kjb_strncpy. 
*/

void trunc_quote_cpy(char* output_string,
                     const char* input_string,
                     size_t max_len)
{
#ifdef OLD_WAY
    size_t cur_len;
#endif 
    size_t len;

    if (max_len == ROOM_FOR_NULL)
    {
        *output_string = '\0';
    }
    else 
    {
        str_trunc_cpy(output_string,  "\"", max_len); 
        max_len--; 
        output_string++;
        str_trunc_cpy(output_string,  input_string, max_len); 
        len = strlen(output_string); 
        max_len -= len;
        output_string += len;
        str_trunc_cpy(output_string,  "\"", max_len); 
    }

#ifdef OLD_WAY

    len = strlen(input_string);
    cur_len = len + 2;

    ASSERT(max_len >= MIN_OF(cur_len + 1, 7));

    if (cur_len >= max_len)
    {
        kjb_strncpy(output_string, "\"", max_len-4);
        kjb_strncat(output_string, input_string, max_len-4);
        kjb_strncat(output_string, "...\"", max_len);
    }
    else
    {
        kjb_strncpy(output_string, "\"", max_len);
        kjb_strncat(output_string, input_string, max_len);
        kjb_strncat(output_string, "\"", max_len);
    }
#endif 
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                               kjb_buff_cpy
 *
 * Copies one string to another
 *
 * This routine is similar to kjb_strncpy except that truncated copying due to
 * buffer size is considered a bug and some additional checks are made in
 * development code. Specifically, the buffer size can't be 4 which, amoung
 * other things, gaurds against buffer sizes derived from the sizeof() operator
 * applied to pointers.
 *
 * Macros:
 *    BUFF_CPY(char s1[], char* s2)
 *
 *    The buffer size parameter is set to sizeof(s1). Using sizeof to set the
 *    buffer size is recomended where applicable, as the code will not be
 *    broken if the buffer size changes. HOWEVER, neither this method, nor the
 *    macro, is  applicable if s1 is NOT a character array. If s1 is declared
 *    by "char* s1", then the size of s1 is the number of bytes in a character
 *    pointer (usually 8), which is NOT what is normally intended.  You have
 *    been WARNED!
 *
 * Returns: 
 *    ERROR if buffer overflow is detected (treated as a bug), NO_ERROR
 *    otherwise. 
 *
 * Related:
 *     kjb_strncpy, kjb_strncat, extended_lc_strncpy, extended_uc_strncpy
 *
 * Index: strings
 *
 * -----------------------------------------------------------------------------
*/

int kjb_buff_cpy(char* s1, const char* s2, size_t max_len)
{ 
#ifdef TEST
    const char* save_s2 = s2;
    size_t      save_max_len = max_len;
#endif 

    /*
    dbs(s2); 
    dbi(strlen(s2));
    dbi(max_len);
    */

#ifdef TEST
    if (max_len == sizeof(char*))
    {
        warn_pso("Suspicious buffer length (%lu) at line %d of %s.\n",
                 (unsigned long)max_len, __LINE__, __FILE__);
        kjb_optional_abort();
    }
#endif

    /*
     * If size_t is unsigned (which is usually the case), then we check for
     * backwards compatibility bugs which manifest as max_len being larger than
     * the maximum positive signed int. It is possible that such a large size
     * was intended, but it is more likely a bug. We also treat max_len as 0 as
     * most likely a bug. 
    */
    if (    (max_len == 0)
#ifdef SIZE_T_IS_32_BITS
         || (max_len > INT32_MAX)
#else 
         || (max_len > INT64_MAX)
#endif
       )
    {
        SET_ARGUMENT_BUG();
        return ERROR;
    }

    --max_len;  /* Account for room needed by null. */

    while ((max_len > 0) && (*s2 != '\0'))
    {
        *s1 = *s2;
        ++s1;
        ++s2;
        max_len--; 
    }

    *s1 = '\0';

    if (((max_len <= 0)) && (*s2 != '\0'))
    {
#ifdef TEST
        dbw(); 
        TEST_PSE(("Trying to fit %q into a buffer of size %d.\n",
                  save_s2, save_max_len)); 
#endif 
        SET_BUFFER_OVERFLOW_BUG();
        return ERROR;
    }

    return NO_ERROR; 
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                               kjb_strncpy
 *
 * Copies one string to another
 *
 * kjb_strncpy is similar strncpy but s1 always gets null terminated - IE
 * max_len includes the null.  Also, assuming that max_len is the size of the
 * storage for s1, this routine will not write beyond that storage, including
 * the null. 
 *
 * This routine needs to be able to stop copying based on the length. For a
 * version that returns error on truncation, see kjb_buff_cpy(). 
 *
 * Macros:
 *    BUFF_CPY(char s1[], char* s2)
 *
 *    The buffer size parameter is set to sizeof(s1). Using sizeof to set the
 *    buffer size is recomended where applicable, as the code will not be
 *    broken if the buffer size changes. HOWEVER, neither this method, nor the
 *    macro, is  applicable if s1 is NOT a character array. If s1 is declared
 *    by "char* s1", then the size of s1 is the number of bytes in a character
 *    pointer (usually 8), which is NOT what is normally intended.  You have
 *    been WARNED!
 *
 * Returns:
 *    Unlike stncpy(), this routine does not return anything. The reason is due
 *    to the complexity of getting the interface just right on multiple
 *    archetectures given that supporting a negative integer return is nice, but
 *    that the number of copied characters is potentially MAX_SIZE_T. However,
 *    this might be changed in the future, if we get time to make various
 *    components consistent. 
 *
 * Bugs: 
 *    This routine should probably return TRUNCATED if the entire buffer is used
 *    and we still want to keep copying. 
 *
 * Related:
 *    kjb_buff_cpy, kjb_buff_cat, kjb_strncat, extended_lc_strncpy, extended_uc_strncpy
 *
 * Index: strings
 *
 * -----------------------------------------------------------------------------
*/

void kjb_strncpy(char* s1, const char* s2, size_t max_len)
{


    /*
     * If size_t is unsigned (which is usually the case), then we check for
     * backwards compatibility bugs which manifest as max_len being larger than
     * the maximum positive signed int. It is possible that such a large size
     * was intended, but it is more likely a bug. We also treat max_len as 0 as
     * most likely a bug. 
    */
    if (    (max_len == 0)
#ifdef SIZE_T_IS_32_BITS
         || (max_len > INT32_MAX)
#else 
         || (max_len > INT64_MAX)
#endif
       )
    {
        SET_ARGUMENT_BUG();
        return;
    }

    --max_len;  /* Account for room needed by null. */

    while ((max_len > 0) && (*s2 != '\0'))
    {
        *s1 = *s2;
        ++s1;
        ++s2;
        max_len--; 
    }

    *s1 = '\0';

    /*
     * FIXME  TODO
     *
     * It might be better if this routine and similar ones returned the number
     * of non-null characters copies, or ERROR. However, we then have the
     * standard annoying issue of restricting max_len to INT_MAX (which we do
     * anyway). Perhaps better to just return TRUNCATED if buffer overflow is
     * detected. This is the main point. We rarely care how many characters are
     * copied. 
    */
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

#ifndef kjb_memcpy

/* =============================================================================
 *                                kjb_memcpy
 *
 * Copies a specifed number of bytes from one location to another
 *
 * On most systems kjb_memcpy is a macro accessing the builtin memcpy.  If
 * implemented in the library, it is meant to behave similarly to the standard
 * memcpy.
 *
 * Index: strings
 *
 * -----------------------------------------------------------------------------
*/

void *kjb_memcpy(void* dest, const void* src, size_t max_len)
{
    char* s1 = (char*)dest;
    const char* s2 = (const char*)src;

#ifdef TEST
    if ((size_t)ABS_OF(s1 - s2) < max_len)
    {
        set_bug("Overlapping windows in kjb_memcpy.\n"); 
    }
#endif 

    while (max_len > 0)
    {
        *s1++ = *s2++;
        max_len--;
    }

    return dest;
}

#endif

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                               extended_lc_strncpy
 *
 * Makes a lower case copy of a string
 *
 * Routine extended_lc_strncpy is like kjb_strncpy except that the copy is
 * converted to lower case. Underscores are converted to hyphens. Only upper
 * case characters and underscores are changed.
 *
 * Note:
 *    Underscores are translated to hyphens.
 *
 * Macros:
 *    EXTENDED_LC_BUFF_CPY(char s1[], char* s2)
 *
 *    The buffer size parameter is set to sizeof(s1). Using sizeof to set the
 *    buffer size is recomended where applicable, as the code will not be
 *    broken if the buffer size changes. HOWEVER, neither this method, nor the
 *    macro, is  applicable if s1 is NOT a character array. If s1 is declared
 *    by "char* s1", then the size of s1 is the number of bytes in a character
 *    pointer (usually 8), which is NOT what is normally intended.  You have
 *    been WARNED!
 *
 * Bugs: 
 *    This routine should probably return TRUNCATED if the entire buffer is used
 *    and we still want to keep copying. 
 *
 * Related:
 *     kjb_strncat, kjb_strncpy, extended_uc_strncpy, extended_tolower,
 *     extended_toupper
 *
 * Index: strings, case conversion
 *
 * -----------------------------------------------------------------------------
*/

/*
 * FIXME  TODO
 *
 * See kjb_strncpy. 
*/

void extended_lc_strncpy(char* s1, const char* s2, size_t max_len)
{
    size_t count;


    /*
     * If size_t is unsigned (which is usually the case), then we check for
     * backwards compatibility bugs which manifest as max_len being larger than
     * the maximum positive signed int. It is possible that such a large size
     * was intended, but it is more likely a bug. We also treat max_len as 0 as
     * most likely a bug. 
    */
    if (    (max_len == 0)
#ifdef SIZE_T_IS_32_BITS
         || (max_len > INT32_MAX)
#else 
         || (max_len > INT64_MAX)
#endif
       )
    {
        SET_ARGUMENT_BUG();
        kjb_print_error();
        return;
    }

    --max_len;  /* Account for room needed by null. */

    count = 0;

    while ( (count < max_len) && (*s2 != '\0') )
    {
        *s1 = extended_tolower(*s2);
        ++s1;
        ++s2;
        count++;
    }

    *s1 = '\0';
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                               extended_uc_strncpy
 *
 * Makes an upper case copy of a string
 *
 * Routine extended_uc_strncpy is like kjb_strncpy except that the copy is
 * converted to upper case. Hypens are translated to underscores. Only lower
 * case characters and hyphens are changed.
 *
 * Note:
 *    Hypens are translated to underscores.
 *
 * Macros:
 *    EXTENDED_UC_BUFF_CPY(char s1[], char* s2)
 *
 *    The buffer size parameter is set to sizeof(s1). Using sizeof to set the
 *    buffer size is recomended where applicable, as the code will not be
 *    broken if the buffer size changes. HOWEVER, neither this method, nor the
 *    macro, is  applicable if s1 is NOT a character array. If s1 is declared
 *    by "char* s1", then the size of s1 is the number of bytes in a character
 *    pointer (usually 8), which is NOT what is normally intended.  You have
 *    been WARNED!
 *
 * Bugs: 
 *    This routine should probably return TRUNCATED if the entire buffer is used
 *    and we still want to keep copying. 
 *
 * Related:
 *     kjb_strncat, kjb_strncpy, extended_lc_strncpy, extended_tolower,
 *     extended_toupper
 *
 * Index: strings, case conversion
 *
 * -----------------------------------------------------------------------------
*/

/*
 * FIXME  TODO
 *
 * See kjb_strncpy. 
*/

void extended_uc_strncpy(char* s1, const char* s2, size_t max_len)
{
    size_t count;


    /*
     * If size_t is unsigned (which is usually the case), then we check for
     * backwards compatibility bugs which manifest as max_len being larger than
     * the maximum positive signed int. It is possible that such a large size
     * was intended, but it is more likely a bug. We also treat max_len as 0 as
     * most likely a bug. 
    */
    if (    (max_len == 0)
#ifdef SIZE_T_IS_32_BITS
         || (max_len > INT32_MAX)
#else 
         || (max_len > INT64_MAX)
#endif
       )
    {
        SET_ARGUMENT_BUG();
        kjb_print_error();
        return;
    }

    --max_len;  /* Account for room needed by null. */

    count = 0;

    while ( (count < max_len) && (*s2 != '\0') )
    {
        *s1 = extended_toupper(*s2);
        ++s1;
        ++s2;
        count++;
    }

    *s1 = '\0';
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                              kjb_buff_cat
 *
 * Catenates one string to another
 *
 * This routine is just like kjb_strncat except that truncated copying due to
 * buffer size is treated as a bug and some additional checks are made in
 * development code. Specifically, the buffer size must be greater than 8 which,
 * amoung other things, gaurds against buffer sizes derived from the sizeof()
 * operator applied to pointers.
 *
 * Macros:
 *    BUFF_CAT(char s1[], char* s2)
 *
 *    The buffer size parameter is set to sizeof(s1). Using sizeof to set the
 *    buffer size is recomended where applicable, as the code will not be
 *    broken if the buffer size changes. HOWEVER, neither this method, nor the
 *    macro, is  applicable if s1 is NOT a character array. If s1 is declared
 *    by "char* s1", then the size of s1 is the number of bytes in a character
 *    pointer (usually 8), which is NOT what is normally intended.  You have
 *    been WARNED!
 *
 * Bugs: 
 *    This routine should probably return TRUNCATED if the entire buffer is used
 *    and we still want to keep copying. 
 *
 * Related:
 *     kjb_strncpy
 *
 * Index: strings
 *
 * -----------------------------------------------------------------------------
*/

/*
 * FIXME  TODO
 *
 * See kjb_strncpy. 
*/

void kjb_buff_cat
    (
         char*       s1,
         const char* s2,
         size_t      max_len
    )
{

#ifdef TEST
    if (max_len == sizeof(char*))
    {
        warn_pso("Suspicious buffer length (%lu) at line %d of %s.\n",
                 (unsigned long)max_len, __LINE__, __FILE__);
        kjb_optional_abort();
    }
#endif

    /*
     * If size_t is unsigned (which is usually the case), then we check for
     * backwards compatibility bugs which manifest as max_len being larger than
     * the maximum positive signed int. It is possible that such a large size
     * was intended, but it is more likely a bug. We also treat max_len as 0 as
     * most likely a bug. 
    */
    if (    (max_len == 0)
#ifdef SIZE_T_IS_32_BITS
         || (max_len > INT32_MAX)
#else 
         || (max_len > INT64_MAX)
#endif
       )
    {
        SET_ARGUMENT_BUG();
        kjb_print_error();
        return;
    }

    --max_len;  /* Account for room needed by null. */

    while (( *s1 != '\0') && (max_len > 0))
    {
        ++s1;
        --max_len;
    }

    if (*s1 != '\0')
    {
        set_bug("First arg to kjb_strncat longer than max_len. Missing NULL?");
        return;
    }

    while ((max_len > 0) && (*s2 != '\0'))
    {
        *s1++ = *s2++;
        max_len--; 
    }

    *s1 = '\0';

    if (((max_len <= 0)) && (*s2 != '\0'))
    {
        SET_BUFFER_OVERFLOW_BUG();
    }

}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                              kjb_strncat
 *
 * Catenates one string to another
 *
 * kjb_strncat is similar to strncat but s1 always gets null terminated - IE
 * max_len includes the null.  Also, assuming that max_len is the size of the
 * storage for s1, this routine will not write beyond that storage, including
 * the null. 
 *
 * This routine needs to be able to stop copying based on max_len, even if the
 * source string has more characters. For a version that returns ERROR on
 * possible buffer overflow, see kjb_buff_cat(). 
 *
 * Macros:
 *    BUFF_CAT(char s1[], char* s2)
 *
 *    The buffer size parameter is set to sizeof(s1). Using sizeof to set the
 *    buffer size is recomended where applicable, as the code will not be
 *    broken if the buffer size changes. HOWEVER, neither this method, nor the
 *    macro, is  applicable if s1 is NOT a character array. If s1 is declared
 *    by "char* s1", then the size of s1 is the number of bytes in a character
 *    pointer (usually 8 or 4), which is NOT what is normally intended. In
 *    development mode we print a warning if the buffer size is the same as that
 *    of a pointer. 
 *
 * Returns:
 *    Unlike strncat(), this routine does not return anything. 
 *
 * Bugs: 
 *    This routine should probably return TRUNCATED if the entire buffer is used
 *    and we still want to keep copying. 
 *
 * Related:
 *     kjb_buff_cat, kjb_buff_cpy, kjb_strncpy 
 *
 * Index: strings
 *
 * -----------------------------------------------------------------------------
*/

/*
 * FIXME  TODO
 *
 * See kjb_strncpy. 
*/

void kjb_strncat(char* s1, const char* s2, size_t max_len)
{

#ifdef TEST
    if (max_len == sizeof(char*))
    {
        /* For now, we check that buffers are not the size of a pointer. This
         * can lead to spurious messages however. 
        */
        warn_pso("Suspicious buffer length (%lu) at line %d of %s.\n",
                 (unsigned long)max_len, __LINE__, __FILE__);
        warn_pso("This message checks against using a pointer in BUFF_CAT.\n");
        warn_pso("If your buffer is the size of a pointer, then increase it.\n");

        kjb_optional_abort();
    }
#endif

    /*
     * If size_t is unsigned (which is usually the case), then we check for
     * backwards compatibility bugs which manifest as max_len being larger than
     * the maximum positive signed int. It is possible that such a large size
     * was intended, but it is more likely a bug. We also treat max_len as 0 as
     * most likely a bug. 
    */
    if (    (max_len == 0)
#ifdef SIZE_T_IS_32_BITS
         || (max_len > INT32_MAX)
#else 
         || (max_len > INT64_MAX)
#endif
       )
    {
        SET_ARGUMENT_BUG();
        kjb_print_error();
        return;
    }

    --max_len;  /* Account for room needed by null. */

    while (( *s1 != '\0') && (max_len > 0))
    {
        ++s1;
        --max_len;
    }

    if (*s1 != '\0')
    {
        set_bug("First arg to kjb_strncat longer than max_len. Missing NULL?");
        kjb_print_error();
        return;
    }

    while ((max_len > 0) && (*s2 != '\0'))
    {
        *s1++ = *s2++;
        max_len--;
    }

    *s1 = '\0';
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                            cap_first_letter_cpy
 *
 *
 *
 * -----------------------------------------------------------------------------
*/

void cap_first_letter_cpy(char* s1, const char* s2, size_t max_len)
{


    ASSERT(max_len != 0);

    --max_len;  /* Account for room needed by null. */

    while ( (max_len > 0) && (*s2 != '\0') )
    {
        while ( (max_len > 0) && (*s2 != '\0') && (! isalpha((int)(*s2))))
        {
            *s1 = *s2;
            s1++;
            s2++;
            --max_len;
        }

        if (isalpha((int)(*s2)))
        {
            *s1 = extended_toupper(*s2);
            ++s1; ++s2;
            --max_len;
        }

        while ( (max_len > 0) && (*s2 != '\0') &&
                ((*s2 == '_') || (isalpha((int)(*s2)))))
        {
            *s1 = *s2;
            s1++;
            s2++;
            --max_len;
        }
    }

    *s1 = '\0';
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                               str_build
 *
 *
 *
 * Index: strings
 *
 * -----------------------------------------------------------------------------
*/

void str_build(char** s1_ptr, const char* s2)
{


    while (*s2 != '\0')
    {
        **s1_ptr = *s2;
        ++ (*s1_ptr);
        ++ s2;
    }
    **s1_ptr = '\0';
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                               str_n_build
 *
 *
 *
 * Index: strings
 *
 * -----------------------------------------------------------------------------
*/

void str_n_build(char** s1_ptr, const char* s2, size_t max_len)
{


    ASSERT(max_len != 0);

    while ((*s2 != '\0') && (max_len > 0))
    {
        **s1_ptr = *s2;
        ++(*s1_ptr);
        ++s2;
        --max_len;
    }

    **s1_ptr = '\0';
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                               str_char_build
 *
 *
 *
 * Index: strings
 *
 * -----------------------------------------------------------------------------
*/

void str_char_build(char** s1_ptr, int char_arg, size_t num)
{
    char c;


    c = (char) char_arg;

    while (num > 0)
    {
        **s1_ptr = c;
        ++ (*s1_ptr);
        --num;
    }

    **s1_ptr = '\0';
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                          byte_build
 *
 *
 *
 * Index: strings
 *
 * -----------------------------------------------------------------------------
*/

void byte_build(char** s1_ptr, unsigned char* s2, size_t max_len)
{


    while (max_len > 0)
    {
        **s1_ptr = *s2;
        ++(*s1_ptr);
        ++s2;
        --max_len;
    }
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                          increment_byte_copy
 *
 *
 *
 * -----------------------------------------------------------------------------
*/

void increment_byte_copy(char** s1_ptr, char** s2_ptr, size_t count)
{



    while (count > 0)
    {
        **s1_ptr = **s2_ptr;
        ++ (*s1_ptr);
        ++ (*s2_ptr);
        count--;
    }
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                             fill_with_blanks
 *
 *
 *
 * Index: strings
 *
 * -----------------------------------------------------------------------------
*/

void fill_with_blanks(char** str_ptr, size_t count)
{


    while (count > 0)
    {
        **str_ptr = ' ';
        ++(*str_ptr);
        count--;
    }
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                              find_string
 *
 * This tests whether second string str2 is a substring of first string str1.
 * It would be better to use macro FIND_STRING_YES(str1, str2) which returns a
 * usable boolean-style value; or use FIND_STRING_NO for its logical negation.
 *
 * If str2 is a substring of str1, then this
 * function returns the one-based position within str1 where it occurs.
 * If not, this returns constant value NOT_A_SUBSTRING, which is zero.
 * Input pointers must not equal NULL; if either does, we assume it is a bug.
 *
 * This routine is not efficient if str2 is long; it is a naive search.
 *
 * Documenter:  Andrew Predoehl
 *
 * Index: strings
 *
 * -----------------------------------------------------------------------------
*/

size_t find_string(const char* str1, const char* str2)
{
    size_t      len1;
    size_t      len2;
    size_t      i, j;
    size_t      count;
    const char* str1_cpy;
    const char* str2_cpy;

    if ( NULL == str1 || NULL == str2 )
    {
        SET_ARGUMENT_BUG();
    }

    len1 = strlen(str1);
    len2 = strlen(str2);

    if (len2 > len1)
    {
        return NOT_A_SUBSTRING;   /* 0 */
    }

    count = len1 - len2 + 1;

    for ( i=1; i<=count; ++i)
    {
        str1_cpy = str1;
        str2_cpy = str2;
        j=0;

        while ((j<len2) && (*str1_cpy == *str2_cpy))
        {
            ++j; ++str1_cpy; ++str2_cpy;
        }
        if (j == len2)
        {
            return i;
        }

        ++str1;
    }

    return NOT_A_SUBSTRING;    /* 0 */
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                               find_char
 *
 * Library users should prefer the macros FIND_CHAR_YES or
 * FIND_CHAR_NO to direct use of this function, which return boolean-style
 * values.
 *
 * This tests for the presence of a given character in a string.  If the
 * character is not present, this returns constant CHARACTER_NOT_IN_STRING,
 * which is zero at this writing.  Otherwise, this returns a one-based
 * position of the character in the string.  Input pointer must not equal NULL,
 * or it is assumed to be a bug.
 *
 * Documenter:  Andrew Predoehl
 *
 * Index: strings
 *
 * -----------------------------------------------------------------------------
*/

size_t find_char(const char* str, int test_char_arg)
{
    size_t i;
    char   test_char;

    if ( str == NULL )
    {
        SET_ARGUMENT_BUG();
        return CHARACTER_NOT_IN_STRING; 
    }

    test_char = (char)test_char_arg;
    i = 1;

    while ((*str != test_char) && (*str != '\0'))
    {
        str++;
        i++;
    }

    if (*str == '\0') return CHARACTER_NOT_IN_STRING;    /* 0 */
    else return i;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                               n_find_char
 *
 *
 *
 * Index: strings
 *
 * -----------------------------------------------------------------------------
*/

size_t n_find_char(const char* str, size_t len, int test_char)
{
    size_t i;


    for ( i=1; i<=len; ++i)
    {
        if      (*str == '\0')      return CHARACTER_NOT_IN_STRING; /* 0 */
        else if (*str == test_char) return i;

        ++str;
    }

    return CHARACTER_NOT_IN_STRING; /* 0 */
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                              find_char_pair
 *
 *
 *
 * Index: strings
 *
 * -----------------------------------------------------------------------------
*/

size_t find_char_pair(const char* str, int c1, int c2)
{
    const char* str2;
    size_t      count = 1;


    if (*str == '\0') return CHARACTER_NOT_IN_STRING;   /* 0 */

    str2 = str + 1;

    if (*str2 == '\0') return CHARACTER_NOT_IN_STRING;   /* 0 */

    while (*str2 != '\0')
    {
        if (*str == c1)
        {
            if (*str2 == c2) return count;
        }
        str++;
        str2++;
        count++;
    }

    return CHARACTER_NOT_IN_STRING;   /* 0 */
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                              count_char
 *
 *
 *
 * Index: strings
 *
 * -----------------------------------------------------------------------------
*/

size_t count_char(const char* str, int test_char)
{
    size_t count;


    count = 0;

    while (*str != '\0')
    {
        if (*str == test_char)
        {
            count++;
        }
        ++str;
    }

    return count;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                               word_in_phrase
 *
 *
 *
 * Index: strings
 *
 * -----------------------------------------------------------------------------
*/

int word_in_phrase(const char* phrase, const char* word)
{
    const char* phrase_temp_pos;
    char*       word_temp_pos;
    char        word_copy_buff[ 1000 ];
    char*       word_copy;


    BUFF_CPY(word_copy_buff, word);
    word_copy = word_copy_buff;

    trim_beg(&word_copy);
    trim_end(word_copy);

    while (*phrase != '\0')
    {
        phrase_temp_pos = phrase;
        word_temp_pos = word_copy;

        while (*phrase_temp_pos == *word_temp_pos)
        {
            if (*word_temp_pos == '\0') return TRUE;

            ++word_temp_pos;
            ++phrase_temp_pos;
        }

        if ((*word_temp_pos == '\0') &&
            (IS_WHITE_SPACE(*phrase_temp_pos)))
        {
            return TRUE;
        }

        const_next_token(&phrase);
    }

    return FALSE;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                          char_for_char_translate
 *
 *
 *
 * Index: strings
 *
 * -----------------------------------------------------------------------------
*/

void char_for_char_translate(char* str, int c1, int c2)
{


    while (*str != '\0')
    {
        if (*str == c1)
        {
            *str = c2;
        }
        ++str;
    }
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                          remove_duplicate_chars
 *
 *
 *
 * Index: strings
 *
 * -----------------------------------------------------------------------------
*/

void remove_duplicate_chars(char* str, int c)
{
    int   first_one;
    char* copy_pos;


    copy_pos = str;
    first_one = TRUE;

    while (*str != '\0')
    {
        if (*str == c)
        {
            if (first_one)
            {
                first_one = FALSE;
                *copy_pos = *str;
                ++copy_pos;
            }
        }
        else
        {
            first_one = TRUE;
            *copy_pos = *str;
            ++copy_pos;
        }
        ++str;
    }
    *copy_pos = '\0';
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                          str_delete
 *
 * Why this exists is not clear!
 *
 * And it can overflow!! 
 *
 * Perhaps improve, rename, or remove?
 *
 * Index: strings
 *
 * -----------------------------------------------------------------------------
*/

void str_delete(char* str, size_t num)
{
    char*  source_pos;


    source_pos = str  + num;

    while (*source_pos != '\0')
    {
        *str = *source_pos;
        ++str;
        ++source_pos;
    }

    *str = '\0';
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                               str_insert
 *
 * Why this exists is not clear!
 *
 * Perhaps improve, rename, or remove?
 *
 * Index: strings
 *
 * -----------------------------------------------------------------------------
*/

int str_insert(char* target_str, const char* str_to_insert, size_t max_len)
{
    size_t      target_str_len;
    size_t      str_to_insert_len;
    char*       target_str_pos;
    const char* str_to_insert_pos;
    char*       second_target_str_pos;
    size_t      i;


    ASSERT(max_len != 0);

    target_str_len = strlen(target_str);
    str_to_insert_len = strlen(str_to_insert);

    if (target_str_len + str_to_insert_len > max_len-1)
    {
        SET_BUFFER_OVERFLOW_BUG();
        return ERROR;
    }

    target_str_pos = target_str +
        (target_str_len + str_to_insert_len);

    second_target_str_pos = target_str + target_str_len ;

    for (i = 0; i <= target_str_len; ++i)
    {
        *target_str_pos = *second_target_str_pos;
        --target_str_pos;
        --second_target_str_pos;
    }

    target_str_pos = target_str;
    str_to_insert_pos = str_to_insert;

    for (i = 0; i<str_to_insert_len; ++i)
    {
        *target_str_pos = *str_to_insert_pos;
        ++target_str_pos;
        ++str_to_insert_pos;
    }

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                              sget_line
 *
 * Reads a line from a string into a buffer
 *
 * The routine sget_line copies the next line of a string into a buffer. The
 * string position is the set to the begining of the part which was not read.
 * If there is enough room in the buffer, all the characters up to the new
 * line are read. Otherwise, only max_len-1 characters are read. Regardless,
 * the buffer is terminated with a NULL. The new line character is processed,
 * but not copied. In other words, line will not contain a new line character.
 *
 * Returns:
 *    If there are no more characters to be read, then EOF is returned. If the
 *    buffer is not large enough, SET_BUFFER_OVERFLOW_BUG() is called, and
 *    TRUNCATED is returned. Otherwise NO_ERROR is returned.
 *
 * Note:
 *    If the argument is (const char**), then use const_sget_line() instead.
 *    Both sget_line() and const_sget_line() do the same thing. Neither change
 *    the actual string being parsed, only the position pointer. There are two
 *    versions to better communicate with various compilers.
 *
 * Note:
 *    If the number of characters read is required, then sget_line_2 should be
 *    used.
 *
 * Macros:
 *    BUFF_SGET_LINE(char** str_ptr, char line[])
 *
 *    The buffer size parameter is set to sizeof(line). Using sizeof to set the
 *    buffer size is recomended where applicable, as the code will not be
 *    broken if the buffer size changes. HOWEVER, neither this method, nor the
 *    macro, is  applicable if line is NOT a character array. If line is
 *    declared by "char* line", then the size of line is the number of bytes in
 *    a character pointer (usually 8), which is NOT what is normally intended.
 *    You have been WARNED!
 *
 * Index: strings
 *
 * -----------------------------------------------------------------------------
*/

int sget_line
(
     char** str_ptr, /* Address of string pointer.
                        Will be moved to next line. */
     char* line,     /* Buffer for the line. */
     size_t max_len /* Size of buffer. */
)
{
    return sget_line_2(str_ptr, line, max_len, (size_t*)NULL);
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                            const_sget_line
 *
 * Reads a line from a string into a buffer
 *
 * The routine sget_line copies the next line of a string into a buffer. The
 * string position is the set to the begining of the part which was not read.
 * If there is enough room in the buffer, all the characters up to the new
 * line are read. Otherwise, only max_len-1 characters are read. Regardless,
 * the buffer is terminated with a NULL. The new line character is processed,
 * but not copied. In other words, line will not contain a new line character.
 *
 * Returns:
 *    If there are no more characters to be read, then EOF is returned. If the
 *    buffer is not large enough, SET_BUFFER_OVERFLOW_BUG() is called, and
 *    TRUNCATED is returned. Otherwise NO_ERROR is returned.
 *
 * Note:
 *    If the argument is not (const char**), then use _sget_line() instead of
 *    casting.  Both sget_line() and const_sget_line() do the same thing.
 *    Neither change the actual string being parsed, only the position pointer.
 *    There are two versions to better communicate with various compilers.
 *
 * Note:
 *    If the number of characters read is required, then const_sget_line_2
 *    should be used.
 *
 * Macros:
 *    BUFF_CONST_SGET_LINE(char** str_ptr, char line[])
 *
 *    The buffer size parameter is set to sizeof(line). Using sizeof to set the
 *    buffer size is recomended where applicable, as the code will not be
 *    broken if the buffer size changes. HOWEVER, neither this method, nor the
 *    macro, is  applicable if line is NOT a character array. If line is
 *    declared by "char* line", then the size of line is the number of bytes in
 *    a character pointer (usually 8), which is NOT what is normally intended.
 *    You have been WARNED!
 *
 * Index: strings
 *
 * -----------------------------------------------------------------------------
*/

int const_sget_line
(
    const char** str_ptr, /* Address of string pointer.
                             Will be moved to next line. */
    char*        line,    /* Buffer for the line. */
    size_t       max_len  /* Size of buffer. */
)
{
    return const_sget_line_2(str_ptr, line, max_len, (size_t *)NULL);
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*
 * We want the exact same code for sget_line_2() as const_sget_line_2() so we
 * put the code in a macro defined here.
*/
#define SGET_LINE_2_CODE() \
{ \
    size_t count = 0; \
\
\
    if (max_len == 0)\
    {\
        SET_ARGUMENT_BUG();\
        return ERROR;\
    }\
\
    if (**str_ptr == '\0') return EOF; \
\
    max_len--;  /* Room for null */ \
\
    while (    (**str_ptr != '\0')\
            && (count < max_len) \
            && (**str_ptr != '\n')\
          )\
    {\
        count++; \
        *line = **str_ptr; \
        line++; \
        (*str_ptr)++; \
    }\
\
    *line = '\0'; \
\
    if ((count == max_len) && (**str_ptr != '\n') && (**str_ptr != '\0'))\
    {\
        while ((**str_ptr != '\0') && (**str_ptr != '\n'))\
        {\
            (*str_ptr)++;\
        }\
        SET_BUFFER_OVERFLOW_BUG();\
        return TRUNCATED;\
    }\
\
    if (**str_ptr == '\n') (*str_ptr)++; \
\
    if (num_chars_processed_ptr != NULL)\
    {\
        *num_chars_processed_ptr = count;\
    }\
\
    return NO_ERROR; \
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                              sget_line_2
 *
 * Reads a line from a string into a buffer
 *
 * The routine sget_line_2() is similar to sget_line() except that the number of
 * characters is reliably returned in the fourth argument.
 *
 * Note:
 *    If the argument is (const char**), then use const_sget_line_2() instead of
 *    casting.  Both sget_line_2() and const_sget_line_2() do the same thing.
 *    Neither change the actual string being parsed, only the position pointer.
 *    There are two versions to better communicate with various compilers.
 *
 * Note:
 *    If the number of characters read is not required, then
 *    num_chars_processed_ptr can be NULL, or, more elegantly, the routine
 *    sget_line() can be used.
 *
 * Returns:
 *    If there are no more characters to be read, then EOF is returned. If the
 *    buffer is not large enough, SET_BUFFER_OVERFLOW_BUG() is called, and
 *    TRUNCATED is returned. Otherwise NO_ERROR is returned.
 *
 * Related:
 *    sget_line, fget_line, dget_line
 *
 * Index: strings
 *
 * -----------------------------------------------------------------------------
*/

int sget_line_2
(
    char**  str_ptr,                 /* Address of string pointer.
                                        Will be moved to next line. */
    char*   line,                    /* Buffer for the line. */
    size_t  max_len,                 /* Size of buffer. */
    size_t* num_chars_processed_ptr  /* Number of characters returned */
)
SGET_LINE_2_CODE()

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                              const_sget_line_2
 *
 * Reads a line from a string into a buffer
 *
 * The routine const_sget_line_2() is similar to const_sget_line() except that
 * the number of characters is reliably returned in the fourth argument.
 *
 * Returns:
 *    If there are no more characters to be read, then EOF is returned. If the
 *    buffer is not large enough, SET_BUFFER_OVERFLOW_BUG() is called, and
 *    TRUNCATED is returned. Otherwise NO_ERROR is returned.
 *
 * Note:
 *    If the argument is not (const char**), then use sget_line_2() instead of
 *    casting.  Both sget_line() and const_sget_line() do the same thing.
 *    Neither change the actual string being parsed, only the position pointer.
 *    There are two versions to better communicate with various compilers.
 *
 * Note:
 *    If the number of characters read is not required, then
 *    num_chars_processed_ptr can be NULL, or, more elegantly, the routine
 *    const_sget_line() can be used instead.
 *
 * Related:
 *    sget_line, fget_line, dget_line
 *
 * Index: strings
 *
 * -----------------------------------------------------------------------------
*/

int const_sget_line_2
(
    const char** str_ptr,                 /* Address of string pointer.
                                             Will be moved to next line. */
    char*        line,                    /* Buffer for the line. */
    size_t       max_len,                 /* Size of buffer. */
    size_t*      num_chars_processed_ptr  /* Number of characters returned */
)
SGET_LINE_2_CODE()

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                          get_str_indent
 *
 *
 *
 * Index: strings
 *
 * -----------------------------------------------------------------------------
*/

size_t get_str_indent(const char* str)
{
    size_t count = 0;


    while ((*str == ' ') && (*str != '\0') )
    {
        str++;
        count++;
    }

    return count;

}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                          last_char
 *
 *
 *
 * Index: strings
 *
 * -----------------------------------------------------------------------------
*/

char last_char(const char* str)
{

    if (*str == '\0') return '\0';

    while (*str != '\0')
    {
        str++;
    }
    str--;

    return *str;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                          kjb_reverse
 *
 *
 *
 * Index: strings
 *
 * -----------------------------------------------------------------------------
*/

int kjb_reverse(const char* input, char* output, size_t output_buff_len)
{
    size_t input_len;


    ASSERT(output_buff_len != 0);

    input_len = strlen(input);

    if (input_len > (output_buff_len + ROOM_FOR_NULL))
    {
        SET_BUFFER_OVERFLOW_BUG();
        return ERROR;
    }

    input += (input_len - 1);

    while (input_len > 0)
    {
        *output = *input;
        input--;
        output++;
        input_len--;
    }
    *output = '\0';

    return NO_ERROR;
}


/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

int string_is_in_string_array(char* test_str, char** str_array)
{


    while (*str_array)
    {
        if (STRCMP_EQ(*str_array, test_str)) return TRUE;
        str_array++;
    }

    return FALSE;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

int is_pattern(const char* pattern)
{

    if (*pattern == '\0')
    {
        return TRUE;
    }
    else if (last_char(pattern) == '*')
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

int match_pattern(const char* pattern, const char* str)
{
    int result;

    if (*pattern == '\0')
    {
        return TRUE;
    }
    else if (last_char(pattern) == '*')
    {
        char* pattern_copy = kjb_strdup(pattern);

        pattern_copy[ strlen(pattern_copy) - 1 ] = '\0';

        result = HEAD_CMP_EQ(str, pattern_copy);

        kjb_free(pattern_copy);
    }
    else
    {
        result = STRCMP_EQ(pattern, str);
    }

    return result;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                              output_str
 *
 * Writes a string to file 
 *
 * This routine writes a string to a file (as a line), specified using a
 * directory and a name. 
 *
 * If the facility for blocking writing to exisiting files is being used, then
 * this routine will silently not overwrite the file if it exists.  
 *
 * Returns:
 *    NO_ERROR on success, and ERROR on failure, with an appropriate error
 *    message being set. 
 *
 * Index: 
 *     output, I/O
 *
 * -----------------------------------------------------------------------------
*/
int output_str
(
    const char* output_dir,
    const char* file_name,
    const char* value
)
{
    char out_file_name[ MAX_FILE_NAME_SIZE ];
    FILE* fp;

    BUFF_CPY(out_file_name, output_dir);
    BUFF_CAT(out_file_name, DIR_STR);
    BUFF_CAT(out_file_name, file_name);

    if (skip_because_no_overwrite(file_name)) 
    {
        return NO_ERROR;
    }

    NRE(fp = kjb_fopen(out_file_name, "w"));
    ERE(kjb_fputs(fp, value));
    ERE(kjb_fputs(fp, "\n"));

    return kjb_fclose(fp);
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */


#ifdef __cplusplus
}
#endif

