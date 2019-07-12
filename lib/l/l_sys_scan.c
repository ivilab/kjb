
/* $Id: l_sys_scan.c 21342 2017-03-26 03:55:31Z kobus $ */

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
#include "l/l_string.h"
#include "l/l_sys_scan.h"



#define ONE_VALUE_READ    1       /* Undef at end of this file. */


/*
 * 64 bits is a special case because it is the only one of the three where the
 * max number of digits is difference between signed and unsigned. 
 *
 * We consider the number of digits to avoid having to do 0 padded compares,
 * which would be a reasonable ulternative if we were to implement a padded
 * compare (we have not). 
*/
#define DIGIT_LIMIT_FOR_64_BIT_UNS_LONG 20
#define DIGIT_LIMIT_FOR_64_BIT_LONG     19
#define DIGIT_LIMIT_FOR_32_BIT_LONG     10
#define DIGIT_LIMIT_FOR_16_BIT_LONG      5

#define DIGIT_LIMIT_FOR_64_BIT_UNS_INT 20
#define DIGIT_LIMIT_FOR_64_BIT_INT     19
#define DIGIT_LIMIT_FOR_32_BIT_INT     10
#define DIGIT_LIMIT_FOR_16_BIT_INT      5

#define MAX_UNS_64_BIT_INT_STR  "18446744073709551616"
#define MAX_POS_64_BIT_INT_STR   "9223372036854775807"
#define MAX_NEG_64_BIT_INT_STR   "9223372036854775808"
#define MAX_UNS_32_BIT_INT_STR            "4294967295"
#define MAX_POS_32_BIT_INT_STR            "2147483647"
#define MAX_NEG_32_BIT_INT_STR            "2147483648"
#define MAX_UNS_16_BIT_INT_STR                 "65535"
#define MAX_POS_16_BIT_INT_STR                 "32767"
#define MAX_NEG_16_BIT_INT_STR                 "32768"


#ifdef LONG_IS_64_BITS
#    define NUMBER_OF_BITS_IN_LONG   64
#    define DIGIT_LIMIT_FOR_UNS_LONG DIGIT_LIMIT_FOR_64_BIT_UNS_INT
#    define DIGIT_LIMIT_FOR_LONG     DIGIT_LIMIT_FOR_64_BIT_INT
#    define MAX_UNS_LONG_STR         MAX_UNS_64_BIT_INT_STR
#    define MAX_POS_LONG_STR         MAX_POS_64_BIT_INT_STR
#    define MAX_NEG_LONG_STR         MAX_NEG_64_BIT_INT_STR
#else
#ifdef LONG_IS_32_BITS
#    define NUMBER_OF_BITS_IN_LONG   32
#    define DIGIT_LIMIT_FOR_UNS_LONG DIGIT_LIMIT_FOR_32_BIT_INT
#    define DIGIT_LIMIT_FOR_LONG     DIGIT_LIMIT_FOR_32_BIT_INT
#    define MAX_UNS_LONG_STR         MAX_UNS_32_BIT_INT_STR
#    define MAX_POS_LONG_STR         MAX_POS_32_BIT_INT_STR
#    define MAX_NEG_LONG_STR         MAX_NEG_32_BIT_INT_STR
#endif
#endif


#ifdef INT_IS_64_BITS /* Pretty rare. */
#    define NUMBER_OF_BITS_IN_INT    64
#    define DIGIT_LIMIT_FOR_INT      DIGIT_LIMIT_FOR_64_BIT_INT
#    define MAX_UNS_INT_STR          MAX_UNS_64_BIT_INT_STR
#    define MAX_POS_INT_STR          MAX_POS_64_BIT_INT_STR
#    define MAX_NEG_INT_STR          MAX_NEG_64_BIT_INT_STR
#else
#ifdef INT_IS_32_BITS
#    define NUMBER_OF_BITS_IN_INT    32
#    define DIGIT_LIMIT_FOR_INT      DIGIT_LIMIT_FOR_32_BIT_INT
#    define MAX_UNS_INT_STR          MAX_UNS_32_BIT_INT_STR
#    define MAX_POS_INT_STR          MAX_POS_32_BIT_INT_STR
#    define MAX_NEG_INT_STR          MAX_NEG_32_BIT_INT_STR
#else
#ifdef INT_IS_16_BITS
#    define NUMBER_OF_BITS_IN_INT    16
#    define DIGIT_LIMIT_FOR_INT      DIGIT_LIMIT_FOR_16_BIT_INT
#    define MAX_UNS_INT_STR          MAX_UNS_16_BIT_INT_STR
#    define MAX_POS_INT_STR          MAX_POS_16_BIT_INT_STR
#    define MAX_NEG_INT_STR          MAX_NEG_16_BIT_INT_STR
#endif
#endif
#endif

#ifdef SHORT_IS_16_BITS    /* Only case so far */
#    define NUMBER_OF_BITS_IN_SHORT    16
#    define DIGIT_LIMIT_FOR_SHORT      DIGIT_LIMIT_FOR_16_BIT_INT
#    define MAX_UNS_SHORT_STR          MAX_UNS_16_BIT_INT_STR
#    define MAX_POS_SHORT_STR          MAX_POS_16_BIT_INT_STR
#    define MAX_NEG_SHORT_STR          MAX_NEG_16_BIT_INT_STR
#endif

/* -------------------------------------------------------------------------- */

static int fs_scan_nan = FALSE; 
static int fs_scan_inf = FALSE; 

/* -------------------------------------------------------------------------- */

#ifdef __cplusplus
extern "C" {
#endif

/* -------------------------------------------------------------------------- */

int set_low_level_scan_options(const char* option, const char* value)
{
    char lc_option[ 100 ];
    int  temp_boolean_value;
    int  result             = NOT_FOUND;


    EXTENDED_LC_BUFF_CPY(lc_option, option);

    if (    (lc_option[ 0 ] == '\0')
         || (match_pattern(lc_option, "scan-inf"))
       )
    {
        if (value == NULL)
        {
            return NO_ERROR;
        }
        else if (value[ 0 ] == '?')
        {
            ERE(pso("scan-inf = %s\n", fs_scan_inf ? "t" : "f"));
        }
        else if (value[ 0 ] == '\0')
        {
            ERE(pso("inf %s considered a valid floating point number on scans.\n",
                    fs_scan_inf ? "is" : "is not" ));
        }
        else
        {
            ERE(temp_boolean_value = get_boolean_value(value));
            fs_scan_inf = temp_boolean_value;
        }
        result = NO_ERROR;
    }

    if (    (lc_option[ 0 ] == '\0')
         || (match_pattern(lc_option, "scan-nan"))
       )
    {
        if (value == NULL)
        {
            return NO_ERROR;
        }
        else if (value[ 0 ] == '?')
        {
            ERE(pso("scan-nan = %s\n", fs_scan_nan ? "t" : "f"));
        }
        else if (value[ 0 ] == '\0')
        {
            ERE(pso("NaN %s considered a valid floating point number on scans.\n",
                    fs_scan_nan ? "is" : "is not" ));
        }
        else
        {
            ERE(temp_boolean_value = get_boolean_value(value));
            fs_scan_nan = temp_boolean_value;
        }
        result = NO_ERROR;
    }

    return result;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*
 * =============================================================================
 *                                  ss1ul
 *
 * Scan an unsigned long from a string
 *
 * This routine reads an unsigned long from a string with more sophisticated
 * error reporting that available using scanf. If the contents of "str" is
 * exactly one number which will fit into an unsigned long it sets *unsigned
 * long_ptr to that number and returns NO_ERROR.  Otherwise and error message is
 * set, and ERROR is returned.  The contents of *unsigned long_ptr are only
 * changed on success.
 *
 * Note:
 *    For successful return, "str" cannot contain two numbers. For example, it
 *    can't be something like "1 2".
 *
 * Returns:
 *    If "str" represents a valid long, then NO_ERROR is returned. Otherwise the
 *    problem with "str" is reported in the set error message, and ERRROR is
 *    returned.  The problem with "str" can be printed with kjb_print_error.
 *
 * Macros:
 *    ss1u64, ss1u32, ss1u16
 *
 *    These macros can be used to force scanning of a certain integer size. The
 *    argument of these is normally a pointer to  kjb_int64, kjb_int32, or
 *    kjb_int16 as appropriate.
 *
 * Index: I/O, scanning
 *
 * -----------------------------------------------------------------------------
 */

int ss1ul(const char* input_str, unsigned long* ulong_ptr)
{
    unsigned long  local_ulong;
    char  str_buff[ 200 ];
    char* str;
    char* str_pos;


    BUFF_CPY(str_buff, input_str);
    str = str_buff;

    trim_beg(&str);
    trim_end(str);

    str_pos = str;

    if (*str_pos == '\0')
    {
        set_error("Expecting an integer but recieved nothing.");
        return ERROR;
    }

    if (*str_pos == '+') { str_pos++; }

    if (*str_pos == '\0')
    {
        set_error("Expecting an integer but recieved only \"+\".");
        return ERROR;
    }

    if ( ! all_digits(str_pos))
    {
        set_error("%q isn't a valid unsigned integer.", str_pos);
        return ERROR;
    }

    if (strlen(str_pos) >= DIGIT_LIMIT_FOR_UNS_LONG)
    {
        if ((strlen(str_pos) > DIGIT_LIMIT_FOR_UNS_LONG) ||
            (strcmp(str_pos, MAX_UNS_LONG_STR) > 0))
        {
            set_error("%q won't fit into a %d bit unsigned number.", str_pos,
                      NUMBER_OF_BITS_IN_LONG);
            return ERROR;
        }
    }

    if (sscanf(str, "%lu", &local_ulong) != ONE_VALUE_READ)
    {
        set_bug("Unexpected return from sscanf in ss1ul with argument %q.", str);
        return ERROR;
    }

    *ulong_ptr = local_ulong;

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*
 * =============================================================================
 *                                  ss1l
 *
 * Scan a long from a string
 *
 * This routine reads a long from a string with more sophisticated error
 * reporting that available using scanf. If the contents of "str" is exactly one
 * number which will fit into a long (i.e., it is between LONG_MIN and
 * LONG_MAX), it sets *long_ptr to that number and returns NO_ERROR.  Otherwise
 * and error message is set, and ERROR is returned.  The contents of *long_ptr
 * are only changed on success.
 *
 * Note:
 *    For successful return, "str" cannot contain two numbers. For example, it
 *    can't be something like "1 2".
 *
 * Returns:
 *    If "str" represents a valid long, then NO_ERROR is returned. Otherwise the
 *    problem with "str" is reported in the set error message, and ERRROR is
 *    returned.  The problem with "str" can be printed with kjb_print_error.
 *
 * Macros:
 *    ss1i64, ss1i32, ss1i16
 *
 *    These macros can be used to force scanning of a certain integer size. The
 *    argument of these is normally a pointer to  kjb_int64, kjb_int32, or
 *    kjb_int16 as appropriate.
 *
 * Index: I/O, scanning
 *
 * -----------------------------------------------------------------------------
 */

int ss1l(const char* input_str, long* long_ptr)
{
    long  local_long;
    int   negative        = FALSE;
    char  str_buff[ 200 ];
    char* str;
    char* str_pos;
    char* str_with_minus_pos; 


    BUFF_CPY(str_buff, input_str);
    str = str_buff;

    trim_beg(&str);
    trim_end(str);

    str_pos = str;

    if (*str_pos == '\0')
    {
        set_error("Expecting an integer but recieved nothing.");
        return ERROR;
    }

    if (*str_pos == '+')
    {
        str_pos++;
    }

    if (*str_pos == '\0')
    {
        set_error("Expecting an integer but recieved only \"+\".");
        return ERROR;
    }
    
    /* For better error messages. */
    str_with_minus_pos = str_pos; 

    if (*str_pos == '-')
    {
        str_pos++;
        negative = TRUE;
    }

    if (*str_pos == '\0')
    {
        set_error("Expecting an integer but recieved only \"-\".");
        return ERROR;
    }
    else if ( ! all_digits(str_pos))
    {
        set_error("%q isn't a valid integer.", str_pos);
        return ERROR;
    }

    if (strlen(str_pos) >= DIGIT_LIMIT_FOR_LONG)
    {
        if (negative)
        {
            if ((strlen(str_pos) > DIGIT_LIMIT_FOR_LONG) ||
                (strcmp(str_pos, MAX_NEG_LONG_STR) > 0))
            {
                set_error("%q won't fit into a %d bit signed number.", 
                          str_with_minus_pos,
                          NUMBER_OF_BITS_IN_LONG);
                return ERROR;
            }
         }
        else
        {
            if ((strlen(str_pos) > DIGIT_LIMIT_FOR_LONG) ||
               (strcmp(str_pos, MAX_POS_LONG_STR) > 0))
            {
                set_error("%q won't fit into a %d bit signed number.", str_pos,
                          NUMBER_OF_BITS_IN_LONG);
                return ERROR;
            }
         }
     }

   if (sscanf(str, "%ld", &local_long) != ONE_VALUE_READ)
   {
       set_bug("Unexpected return from sscanf in ss1l with argument %q.", str);
       return ERROR;
   }

    *long_ptr = local_long;

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*
 * =============================================================================
 *                                  ss1spl
 *
 * Scan a strictly positive long from a string
 *
 * This routine is similar to ss1l, except now we insist that the contents of
 * "str" represent a strictly positive number (still restricted to the LONG_MAX
 * in magnitude).
 *
 * Related:
 *    Use ss1ul() if what you really want is an unsigned long. Use ss1pl() if 0
 *    is considered positive. 
 *
 * Returns:
 *    If "str" represents a valid positive long, then NO_ERROR is returned.
 *    Otherwise the problem with "str" is reported in the set error message, and
 *    ERRROR is returned.  The problem with "str" can be printed with
 *    kjb_print_error.
 *
 * Macros:
 *    ss1spi64, ss1spi32, ss1spi16
 *
 *    These macros can be used to force scanning of a certain integer size. The
 *    argument of these is normally a pointer to  kjb_int64, kjb_int32, or
 *    kjb_int16 as appropriate.
 *
 * Index: I/O, scanning
 *
 * -----------------------------------------------------------------------------
 */

int ss1spl(const char* input_str, long* long_ptr)
{

    ERE(ss1pl(input_str, long_ptr));

    if (*long_ptr <= 0)
    {
        set_error("Expecting a strictly postive long integer but recieved %q.",
                  input_str);
        return ERROR;
        
    }

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*
 * =============================================================================
 *                                  ss1pl
 *
 * Scan a positive long from a string
 *
 * This routine is similar to ss1l, except now we insist that the contents of
 * "str" represent a positive number (still restricted to the LONG_MAX in
 * magnitude).
 *
 * Related:
 *    Use ss1ul() if what you really want is an unsigned long. Use ss1spl() if 0
 *    is not to be considered positive. 
 *
 * Returns:
 *    If "str" represents a valid positive long, then NO_ERROR is returned.
 *    Otherwise the problem with "str" is reported in the set error message, and
 *    ERRROR is returned.  The problem with "str" can be printed with
 *    kjb_print_error.
 *
 * Macros:
 *    ss1pi64, ss1pi32, ss1pi16
 *
 *    These macros can be used to force scanning of a certain integer size. The
 *    argument of these is normally a pointer to  kjb_int64, kjb_int32, or
 *    kjb_int16 as appropriate.
 *
 * Index: I/O, scanning
 *
 * -----------------------------------------------------------------------------
 */

int ss1pl(const char* input_str, long* long_ptr)
{
    long local_long;
    char  str_buff[ 200 ];
    char* str;


    BUFF_CPY(str_buff, input_str);
    str = str_buff;

    trim_beg(&str);
    trim_end(str);

    if (*str == '\0')
    {
        set_error("Expecting a positive integer but recieved nothing.");
        return ERROR;
    }

    if (*str == '+') { str++; }

    if (*str == '\0')
    {
        set_error("Expecting an integer but recieved only \"+\".");
        return ERROR;
    }

    if ( ! all_digits(str))
    {
        set_error("%q isn't a valid positive integer.", str);
        return ERROR;
    }

    if (strlen(str) >= DIGIT_LIMIT_FOR_LONG)
    {
        if ((strlen(str) > DIGIT_LIMIT_FOR_LONG) ||
            (strcmp(str, MAX_POS_LONG_STR) > 0))
        {
            set_error("%q won't fit into a %d bit signed number.", str,
                      NUMBER_OF_BITS_IN_LONG);
            return ERROR;
        }
    }

   if (sscanf(str, "%ld", &local_long) != ONE_VALUE_READ)
   {
       set_bug("Unexpected return from sscanf in ss1pl with argument %q.", str);
       return ERROR;
   }

    *long_ptr = local_long;

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*
 * =============================================================================
 *                                  ss1pl_2
 *
 * Scan a positive long from a string
 *
 * This routine is similar to except that the special strings "off" and
 * "none" give the NEGATIVE number NOT_SET. .
 *
 * Returns:
 *    If "str" represents a valid positive long, or the special values
 *    mentioned above, then NO_ERROR is returned.  Otherwise the problem with
 *    "str" is reported in the set error message, and ERRROR is returned.  The
 *    problem with "str" can be printed with kjb_print_error.
 *
 * Macros:
 *    ss1pi64_2, ss1pi32_2, ss1pi16_2
 *
 *    These macros can be used to force scanning of a certain integer size. The
 *    argument of these is normally a pointer to  kjb_int64, kjb_int32, or
 *    kjb_int16 as appropriate.
 *
 * Index: I/O, scanning
 *
 * -----------------------------------------------------------------------------
 */

int ss1pl_2(const char* input_str, long* long_ptr)
{


    if (is_no_value_word(input_str))
    {
        *long_ptr = NOT_SET;
        return NO_ERROR;
    }
    else
    {
        return ss1pl(input_str, long_ptr);
    }
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*
 * =============================================================================
 *                                  ss1ui
 *
 * Scan an unsigned int from a string
 *
 * This routine reads an unsigned int from a string with more sophisticated
 * error reporting that available using scanf. If the contents of "str" is
 * exactly one number which will fit into an unsigned it sets *unsigned int_ptr
 * to that number and returns NO_ERROR.  Otherwise and error message is set, and
 * ERROR is returned.  The contents of *unsigned int_ptr are only changed on
 * success.
 *
 * Note:
 *    For successful return, "str" cannot contain two numbers. For example, it
 *    can't be something like "1 2".
 *
 * Returns:
 *    If "str" represents a valid int, then NO_ERROR is returned. Otherwise the
 *    problem with "str" is reported in the set error message, and ERRROR is
 *    returned.  The problem with "str" can be printed with kjb_print_error.
 *
 * Macros:
 *    ss1u64, ss1u32, ss1u16
 *
 *    These macros can be used to force scanning of a certain integer size. The
 *    argument of these is normally a pointer to  kjb_int64, kjb_int32, or
 *    kjb_int16 as appropriate.
 *
 * Index: I/O, scanning
 *
 * -----------------------------------------------------------------------------
 */

int ss1ui(const char* input_str, unsigned int* uint_ptr)
{
    unsigned int  local_uint;
    char  str_buff[ 200 ];
    char* str;
    char* str_pos;


    BUFF_CPY(str_buff, input_str);
    str = str_buff;

    trim_beg(&str);
    trim_end(str);

    str_pos = str;

    if (*str_pos == '\0')
    {
        set_error("Expecting an integer but recieved nothing.");
        return ERROR;
    }

    if (*str_pos == '+')
    {
        str_pos++;
    }

    if (*str_pos == '\0')
    {
        set_error("Expecting an integer but recieved only \"+\".");
        return ERROR;
    }

    if ( ! all_digits(str_pos))
    {
        set_error("%q isn't a valid unsigned integer.", str_pos);
        return ERROR;
    }

    if (strlen(str_pos) >= DIGIT_LIMIT_FOR_INT)
    {
        if ((strlen(str_pos) > DIGIT_LIMIT_FOR_INT) ||
            (strcmp(str_pos, MAX_UNS_INT_STR) > 0))
        {
            set_error("%q won't fit into a %d bit unsigned number.", str_pos,
                      NUMBER_OF_BITS_IN_INT);
            return ERROR;
        }
    }

    if (sscanf(str, "%u", &local_uint) != ONE_VALUE_READ)
    {
        set_bug("Unexpected return from sscanf in ss1ui with argument %q.", str);
        return ERROR;
    }

    *uint_ptr = local_uint;

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*
 * =============================================================================
 *                                  ss1i
 *
 * Scan an int from a string
 *
 * This routine reads an int from a string with more sophisticated error
 * reporting that available using scanf. If the contents of "str" is exactly one
 * number which will fit into an int, it sets *int_ptr to that number and
 * returns NO_ERROR.  Otherwise and error message is set, and ERROR is returned.
 * The contents of *int_ptr are only changed on success.
 *
 * Note:
 *    For successful return, "str" cannot contain two numbers. For example, it
 *    can't be something like "1 2".
 *
 * Returns:
 *    If "str" is a valid int, then NO_ERROR is returned. Otherwise the problem
 *    with "str" is reported in the set error message, and ERRROR is returned.
 *    The problem with "str" can be printed with kjb_print_error.
 *
 * Macros:
 *    ss1i64, ss1i32, ss1i16
 *
 *    These macros can be used to force scanning of a certain integer size. The
 *    argument of these is normally a pointer to  kjb_int64, kjb_int32, or
 *    kjb_int16 as appropriate.
 *
 * Index: I/O, scanning
 *
 * -----------------------------------------------------------------------------
 */

int ss1i(const char* input_str, int* int_ptr)
{
    int   local_int;
    int   negative        = FALSE;
    char  str_buff[ 200 ];
    char* str;
    char* str_pos;
    char* str_with_minus_pos; 


    BUFF_CPY(str_buff, input_str);
    str = str_buff;

    trim_beg(&str);
    trim_end(str);

    str_pos = str;

    if (*str_pos == '\0')
    {
        set_error("Expecting an integer but recieved nothing.");
        return ERROR;
    }

    if (*str_pos == '+')
    {
        str_pos++;
    }

    if (*str_pos == '\0')
    {
        set_error("Expecting an integer but recieved only \"+\".");
        return ERROR;
    }

    /* For better error messages. */
    str_with_minus_pos = str_pos; 

    if (*str_pos == '-')
    {
        str_pos++;
        negative = TRUE;
    }

    if (*str_pos == '\0')
    {
        set_error("Expecting an integer but recieved only \"-\".");
        return ERROR;
    }
    else if ( ! all_digits(str_pos))
    {
        set_error("%q isn't a valid integer.", str_pos);
        return ERROR;
    }

    if (strlen(str_pos) >= DIGIT_LIMIT_FOR_INT)
    {
        if (negative)
        {
            if ((strlen(str_pos) > DIGIT_LIMIT_FOR_INT) ||
                (strcmp(str_pos, MAX_NEG_INT_STR) > 0))
            {
                set_error("%q won't fit into a %d bit signed number.", 
                          str_with_minus_pos,
                          NUMBER_OF_BITS_IN_INT);
                return ERROR;
            }
         }
        else
        {
            if ((strlen(str_pos) > DIGIT_LIMIT_FOR_INT) ||
               (strcmp(str_pos, MAX_POS_INT_STR) > 0))
            {
                set_error("%q won't fit into a %d bit signed number.", str_pos,
                          NUMBER_OF_BITS_IN_INT);
                return ERROR;
            }
         }
     }

   if (sscanf(str, "%d", &local_int) != ONE_VALUE_READ)
   {
       set_bug("Unexpected return from sscanf in ss1i with argument %q.", str);
       return ERROR;
   }

    *int_ptr = local_int;

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*
 * =============================================================================
 *                                  ss1spi
 *
 * Scan a strictly positive int from a string
 *
 * This routine is similar to ss1i, except now we insist that the contents of
 * "str" represent a strictly positive number (still restricted to the INT_MAX
 * in magnitude).
 *
 * Related:
 *    Use ss1ui() if what you really want is an unsigned int. Use ss1pi() if 0
 *    is considered positive. 
 *
 * Returns:
 *    If "str" represents a valid positive integer, then NO_ERROR is returned.
 *    Otherwise the problem with "str" is reported in the set error message, and
 *    ERRROR is returned.  The problem with "str" can be printed with
 *    kjb_print_error.
 *
 * Macros:
 *    ss1spi64, ss1spi32, ss1spi16
 *
 *    These macros can be used to force scanning of a certain integer size. The
 *    argument of these is normally a pointer to  kjb_int64, kjb_int32, or
 *    kjb_int16 as appropriate.
 *
 * Index: I/O, scanning
 *
 * -----------------------------------------------------------------------------
 */

int ss1spi(const char* input_str, int* int_ptr)
{

    ERE(ss1pi(input_str, int_ptr));

    if (*int_ptr <= 0)
    {
        set_error("Expecting a strictly postive integer but recieved %q.",
                  input_str);
        return ERROR;
        
    }

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*
 * =============================================================================
 *                                  ss1pi
 *
 * Scan a positive int from a string
 *
 * This routine is similar to ss1i, except now we insist that the contents of
 * "str" represent a positive number (still restricted to the INT_MAX in
 * magnitude).
 *
 * Related:
 *    Use ss1ui() if what you really want is an unsigned int. Use ss1pi() if 0
 *    is considered negative. 
 *
 * Returns:
 *    If "str" represents a valid positive integer, then NO_ERROR is returned.
 *    Otherwise the problem with "str" is reported in the set error message, and
 *    ERRROR is returned.  The problem with "str" can be printed with
 *    kjb_print_error.
 *
 * Macros:
 *    ss1pi64, ss1pi32, ss1pi16
 *
 *    These macros can be used to force scanning of a certain integer size. The
 *    argument of these is normally a pointer to  kjb_int64, kjb_int32, or
 *    kjb_int16 as appropriate.
 *
 * Index: I/O, scanning
 *
 * -----------------------------------------------------------------------------
 */

int ss1pi(const char* input_str, int* int_ptr)
{
    int   local_int       = NOT_SET;
    char  str_buff[ 200 ];
    char* str;


    BUFF_CPY(str_buff, input_str);
    str = str_buff;

    trim_beg(&str);
    trim_end(str);

    if (*str == '\0')
    {
        set_error("Expecting an integer but recieved nothing.");
        return ERROR;
    }

    if (*str == '+') { str++; }

    if (*str== '\0')
    {
        set_error("Expecting an integer but recieved only \"+\".");
        return ERROR;
    }

    if (*str == '\0')
    {
        set_error("Expecting a positive integer but recieved nothing.");
        return ERROR;
    }

    if ( ! all_digits(str))
    {
        set_error("%q isn't a valid positive integer.", str);
        return ERROR;
    }

    if (strlen(str) >= DIGIT_LIMIT_FOR_INT)
    {
        if ((strlen(str) > DIGIT_LIMIT_FOR_INT) ||
            (strcmp(str, MAX_POS_INT_STR) > 0))
        {
            set_error("%q won't fit into a %d bit signed number.", str,
                      NUMBER_OF_BITS_IN_INT);
            return ERROR;
        }
    }

    if (sscanf(str, "%d", &local_int) != ONE_VALUE_READ)
    {
        set_bug("Unexpected return from sscanf in ss1pi with argument %q.",
                str);
        return ERROR;
    }

    if (local_int == NOT_SET)
    {
        SET_CANT_HAPPEN_BUG();
    }

    *int_ptr = local_int;

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*
 * =============================================================================
 *                                  ss1pi_2
 *
 * Scan a positive int from a string
 *
 * This routine is similar to ss1ip1, except that the special strings "off" and
 * "none" give the NEGATIVE number NOT_SET.
 *
 * Returns:
 *    If "str" represents a valid positive integer, or "off", or "none", then
 *    NO_ERROR is returned.  Otherwise the problem with "str" is reported in
 *    the set error message, and ERRROR is returned.  The problem with "str"
 *    can be printed with kjb_print_error.
 *
 * Macros:
 *    ss1pi64_2, ss1pi32_2, ss1pi16_2
 *
 *    These macros can be used to force scanning of a certain integer size. The
 *    argument of these is normally a pointer to  kjb_int64, kjb_int32, or
 *    kjb_int16 as appropriate.
 *
 * Index: I/O, scanning
 *
 * -----------------------------------------------------------------------------
 */

int ss1pi_2(const char* input_str, int* int_ptr)
{


    if (is_no_value_word(input_str))
    {
        *int_ptr = NOT_SET;
        return NO_ERROR;
    }
    else
    {
        return ss1pi(input_str, int_ptr);
    }
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*
 * =============================================================================
 *                                  ss1us
 *
 * Scan an unsigned short from a string
 *
 * This routine reads an unsigned short from a string with more sophisticated
 * error reporting that available using scanf. If the contents of "str" is
 * exactly one number which will fit into an unsigned short it sets *unsigned
 * short_ptr to that number and returns NO_ERROR.  Otherwise and error message is
 * set, and ERROR is returned.  The contents of *unsigned short_ptr are only
 * changed on success.
 *
 * Note:
 *    For successful return, "str" cannot contain two numbers. For example, it
 *    can't be something like "1 2".
 *
 * Returns:
 *    If "str" represents a valid short, then NO_ERROR is returned. Otherwise the
 *    problem with "str" is reported in the set error message, and ERRROR is
 *    returned.  The problem with "str" can be printed with kjb_print_error.
 *
 * Macros:
 *    ss1u64, ss1u32, ss1u16
 *
 *    These macros can be used to force scanning of a certain integer size. The
 *    argument of these is normally a pointer to  kjb_int64, kjb_int32, or
 *    kjb_int16 as appropriate.
 *
 * Index: I/O, scanning
 *
 * -----------------------------------------------------------------------------
 */

int ss1us(const char* input_str, unsigned short* ushort_ptr)
{
    unsigned short  local_ushort;
    char  str_buff[ 200 ];
    char* str;
    char* str_pos;


    BUFF_CPY(str_buff, input_str);
    str = str_buff;

    trim_beg(&str);
    trim_end(str);

    str_pos = str;

    if (*str_pos == '\0')
    {
        set_error("Expecting an integer but recieved nothing.");
        return ERROR;
    }

    if (*str_pos == '+')
    {
        str_pos++;
    }

    if (*str_pos == '\0')
    {
        set_error("Expecting an integer but recieved only \"+\".");
        return ERROR;
    }

    if ( ! all_digits(str_pos))
    {
        set_error("%q isn't a valid unsigned integer.", str_pos);
        return ERROR;
    }

    if (strlen(str_pos) >= DIGIT_LIMIT_FOR_SHORT)
    {
        if ((strlen(str_pos) > DIGIT_LIMIT_FOR_SHORT) ||
            (strcmp(str_pos, MAX_UNS_SHORT_STR) > 0))
        {
            set_error("%q won't fit into a %d bit unsigned number.", str_pos,
                      NUMBER_OF_BITS_IN_SHORT);
            return ERROR;
        }
    }

    if (sscanf(str, "%hu", &local_ushort) != ONE_VALUE_READ)
    {
        set_bug("Unexpected return from sscanf in ss1us with argument %q.", str);
        return ERROR;
    }

    *ushort_ptr = local_ushort;

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*
 * =============================================================================
 *                                  ss1s
 *
 * Scan a short from a string
 *
 * This routine reads a short from a string with more sophisticated error
 * reporting that available using scanf. If the contents of "str" is exactly one
 * number which will fit into a short (i.e., it is between SHORT_MIN and
 * SHORT_MAX), it sets *short_ptr to that number and returns NO_ERROR.
 * Otherwise and error message is set, and ERROR is returned.  The contents of
 * *short_ptr are only changed on success.
 *
 * Note:
 *    For successful return, "str" cannot contain two numbers. For example, it
 *    can't be something like "1 2".
 *
 * Returns:
 *    If "str" represents a valid short, then NO_ERROR is returned. Otherwise
 *    the problem with "str" is reported in the set error message, and ERRROR is
 *    returned.  The problem with "str" can be printed with kjb_print_error.
 *
 * Macros:
 *    ss1i64, ss1i32, ss1i16
 *
 *    These macros can be used to force scanning of a certain integer size. The
 *    argument of these is normally a pointer to  kjb_int64, kjb_int32, or
 *    kjb_int16 as appropriate.
 *
 * Index: I/O, scanning
 *
 * -----------------------------------------------------------------------------
 */

int ss1s(const char* input_str, short* short_ptr)
{
    short local_short;
    int   negative        = FALSE;
    char  str_buff[ 200 ];
    char* str;
    char* str_pos;
    char* str_with_minus_pos; 


    BUFF_CPY(str_buff, input_str);
    str = str_buff;

    trim_beg(&str);
    trim_end(str);

    str_pos = str;

    if (*str_pos == '\0')
    {
        set_error("Expecting an integer but recieved nothing.");
        return ERROR;
    }

    if (*str_pos == '+')
    {
        str_pos++;
    }

    if (*str_pos == '\0')
    {
        set_error("Expecting an integer but recieved only \"+\".");
        return ERROR;
    }

    /* For better error messages. */
    str_with_minus_pos = str_pos; 

    if (*str_pos == '-')
    {
        str_pos++;
        negative = TRUE;
    }

    if (*str_pos == '\0')
    {
        set_error("Expecting an integer but recieved only \"-\".");
        return ERROR;
    }
    else if ( ! all_digits(str_pos))
    {
        set_error("%q isn't a valid integer.", str_pos);
        return ERROR;
    }

    if (strlen(str_pos) >= DIGIT_LIMIT_FOR_SHORT)
    {
        if (negative)
        {
            if ((strlen(str_pos) > DIGIT_LIMIT_FOR_SHORT) ||
                (strcmp(str_pos, MAX_NEG_SHORT_STR) > 0))
            {
                set_error("%q won't fit into a %d bit signed number.", 
                          str_with_minus_pos,
                          NUMBER_OF_BITS_IN_SHORT);
                return ERROR;
            }
         }
        else
        {
            if ((strlen(str_pos) > DIGIT_LIMIT_FOR_SHORT) ||
               (strcmp(str_pos, MAX_POS_SHORT_STR) > 0))
            {
                set_error("%q won't fit into a %d bit signed number.", str_pos,
                          NUMBER_OF_BITS_IN_SHORT);
                return ERROR;
            }
         }
     }

   if (sscanf(str, "%hd", &local_short) != ONE_VALUE_READ)
   {
       set_bug("Unexpected return from sscanf in ss1s with argument %q.", str);
       return ERROR;
   }

    *short_ptr = local_short;

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*
 * =============================================================================
 *                                  ss1sps
 *
 * Scan a strictly positive short from a string
 *
 * This routine is similar to ss1l, except now we insist that the contents of
 * "str" represent a strictly positive number (still restricted to the SHORT_MAX
 * in magnitude).
 *
 * Related:
 *    Use ss1us() if what you really want is an unsigned short. Use ss1pu() if 0
 *    is considered positive. 
 *
 * Returns:
 *    If "str" represents a valid strictly positive short, then NO_ERROR is
 *    returned.  Otherwise the problem with "str" is reported in the set error
 *    message, and ERRROR is returned.  The problem with "str" can be printed
 *    with kjb_print_error.
 *
 * Macros:
 *    ss1spi64, ss1spi32, ss1spi16
 *
 *    These macros can be used to force scanning of a certain integer size. The
 *    argument of these is normally a pointer to kjb_int32 or kjb_int16 as appropriate.
 *
 * Index: I/O, scanning
 *
 * -----------------------------------------------------------------------------
 */

int ss1sps(const char* input_str, short* short_ptr)
{
    ERE(ss1ps(input_str, short_ptr));

    if (*short_ptr <= 0)
    {
        set_error("Expecting a strictly postive short integer but recieved %q.",
                  input_str);
        return ERROR;
        
    }

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*
 * =============================================================================
 *                                  ss1ps
 *
 * Scan a positive short from a string
 *
 * This routine is similar to ss1l, except now we insist that the contents of
 * "str" represent a positive number (still restricted to the SHORT_MAX in
 * magnitude).
 *
 * Related:
 *    Use ss1us() if what you really want is an unsigned short. Use ss1spu() if
 *    0 is considered negative. 
 *
 * Returns:
 *    If "str" represents a valid positive short, then NO_ERROR is returned.
 *    Otherwise the problem with "str" is reported in the set error message, and
 *    ERRROR is returned.  The problem with "str" can be printed with
 *    kjb_print_error.
 *
 * Macros:
 *    ss1pi64, ss1pi32, ss1pi16
 *
 *    These macros can be used to force scanning of a certain integer size. The
 *    argument of these is normally a pointer to  kjb_int64, kjb_int32, or
 *    kjb_int16 as appropriate.
 *
 * Index: I/O, scanning
 *
 * -----------------------------------------------------------------------------
 */

int ss1ps(const char* input_str, short* short_ptr)
{
    short local_short;
    char  str_buff[ 200 ];
    char* str;


    BUFF_CPY(str_buff, input_str);
    str = str_buff;

    trim_beg(&str);
    trim_end(str);

    if (*str == '\0')
    {
        set_error("Expecting a positive integer but recieved nothing.");
        return ERROR;
    }

    if (*str == '+') { str++; }

    if (*str == '\0')
    {
        set_error("Expecting an integer but recieved only \"+\".");
        return ERROR;
    }

    if ( ! all_digits(str))
    {
        set_error("%q isn't a valid positive integer.", str);
        return ERROR;
    }

    if (strlen(str) >= DIGIT_LIMIT_FOR_SHORT)
    {
        if ((strlen(str) > DIGIT_LIMIT_FOR_SHORT) ||
            (strcmp(str, MAX_POS_SHORT_STR) > 0))
        {
            set_error("%q won't fit into a %d bit signed number.", str,
                      NUMBER_OF_BITS_IN_SHORT);
            return ERROR;
        }
     }

   if (sscanf(str, "%hd", &local_short) != ONE_VALUE_READ)
   {
       set_bug("Unexpected return from sscanf in ss1ps with argument %q.", str);
       return ERROR;
   }

    *short_ptr = local_short;

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*
 * =============================================================================
 *                                  ss1ps_2
 *
 * Scan a positive short from a string
 *
 * This routine is similar to except that the special strings "off" and
 * "none" give the NEGATIVE number NOT_SET. .
 *
 * Returns:
 *    If "str" represents a valid positive short, or the special values
 *    mentioned above, then NO_ERROR is returned.  Otherwise the problem with
 *    "str" is reported in the set error message, and ERRROR is returned.  The
 *    problem with "str" can be printed with kjb_print_error.
 *
 * Macros:
 *    ss1pi64_2, ss1pi32_2, ss1pi16_2
 *
 *    These macros can be used to force scanning of a certain integer size. The
 *    argument of these is normally a pointer to kjb_int32 or kjb_int16 as appropriate.
 *
 * Index: I/O, scanning
 *
 * -----------------------------------------------------------------------------
 */

int ss1ps_2(const char* input_str, short* short_ptr)
{


    if (is_no_value_word(input_str))
    {
        *short_ptr = NOT_SET;
        return NO_ERROR;
    }
    else
    {
        return ss1ps(input_str, short_ptr);
    }
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*
 * =============================================================================
 *                                  ss1f
 *
 * Scan a float from a string in fixed decimal format
 *
 * This routine reads a float from a string in fixed decimal format with more
 * sophisticated error reporting that available using scanf. If the contents of
 * "str" is exactly one number which is appropriate for a float, then it sets
 * *float_ptr to that number and returns NO_ERROR.  Otherwise and error message
 * is set, and ERROR is returned.  The contents of *float_ptr are only changed
 * on success.
 *
 * If str is either "off" or "none", then the special value DBL_NOT_SET is
 * returned. Otherwise, str must contain a single valid number, possibly
 * followed by a "%".  If str ends with a "%", then the number is assumed to be
 * a percentage, and the result is divided by 100.0.
 *
 * Note:
 *    For successful return, "str" cannot contain two numbers. For example, it
 *    can't be something like "1 2".
 *
 * Returns:
 *    If "str" represents a valid float, then NO_ERROR is returned. Otherwise
 *    the problem with "str" is reported in the set error message, and ERRROR is
 *    returned.  The problem with "str" can be printed with kjb_print_error.
 *
 * Index: I/O, scanning
 *
 * -----------------------------------------------------------------------------
 */

int ss1f(const char* input_str, float* float_ptr)
{
    char* last_char_ptr;
    int   percentage        = FALSE;
    char  str_buff[ 200 ];
    char* str;
    char* str_digits;


    if (is_no_value_word(input_str))
    {
        *float_ptr = FLT_NOT_SET;
        return NO_ERROR;
    }

    BUFF_CPY(str_buff, input_str);
    str = str_buff;

    trim_beg(&str);
    trim_end(str);

    if (*str == '\0')
    {
        set_error("Expecting a number but recieved nothing.");
        return ERROR;
    }
    else if (    ((fs_scan_nan) && ((STRCMP_EQ(str, "nan")) || (STRCMP_EQ(str, "NAN")) || (STRCMP_EQ(str, "NaN"))))
              || ((fs_scan_inf) && ((STRCMP_EQ(str, "inf")) || (STRCMP_EQ(str, "-inf")) || (STRCMP_EQ(str, "+inf"))))
              || ((fs_scan_inf) && ((STRCMP_EQ(str, "INF")) || (STRCMP_EQ(str, "-INF")) || (STRCMP_EQ(str, "+INF"))))
            )
    {
        ; /* Do nothing. */
    }
    else 
    {
        last_char_ptr = str + strlen(str) - 1;

        if (*last_char_ptr == '%')
        {
            percentage = TRUE;
            *last_char_ptr = '\0';
        }

        str_digits = str; 

        if (*str_digits == '+') 
        { 
            ++str_digits;

            if (*str_digits == '\0')
            {
                set_error("Expecting a fixed decimal number but recieved only \"+\".");
                return ERROR;
            }
        }
        else if (*str_digits == '-')
        {
            str_digits++;

            if (*str_digits == '\0')
            {
                set_error("Expecting a fixed decimal number but recieved only \"-\".");
                return ERROR;
            }
        }

        while ( (isdigit((int)(*str_digits))) &&
                (*str_digits != '\0') )
        {
             ++str_digits;
        }

        if (*str_digits == '.')  ++str_digits;

        if ( ! all_digits(str_digits))
        {
            set_error("%q isn't a valid fixed decimal number.", str);
            return ERROR;
        }
    }

    if (sscanf(str, "%f", float_ptr) != 1)
    {
        set_bug("Unexpected return from sscanf in ss1f with argument %q.", str);
        return ERROR;
    }

    if (percentage)
    {
        *float_ptr /= (float)100.0;
    }

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*
 * =============================================================================
 *                                  ss1snf
 *
 * Scan a float from a string in scientific notation
 *
 * This routine reads a float from a string in scientific notation with more
 * sophisticated error reporting that available using scanf. If the contents of
 * "str" is exactly one number which is appropriate for a float, then it sets
 * *float_ptr to that number and returns NO_ERROR.  Otherwise and error message
 * is set, and ERROR is returned.  The contents of *float_ptr are only changed
 * on success.
 *
 * If str is either "off" or "none", then the special value DBL_NOT_SET is
 * returned. Otherwise, str must contain a single valid number, possibly
 * followed by a "%".  If str ends with a "%", then the number is assumed to be
 * a percentage, and the result is divided by 100.0.
 *
 * Note:
 *    For successful return, "str" cannot contain two numbers. For example, it
 *    can't be something like "1 2".
 *
 * Returns:
 *    If "str" represents a valid float, then NO_ERROR is returned. Otherwise
 *    the problem with "str" is reported in the set error message, and ERRROR is
 *    returned.  The problem with "str" can be printed with kjb_print_error.
 *
 * Index: I/O, scanning
 *
 * -----------------------------------------------------------------------------
 */

int ss1snf(const char* input_str, float* float_ptr)
{
    char str_copy[ 200 ];
    char str_buff[ 100 ];
    char* str;
    int   percentage    = FALSE;
    char* last_char_ptr;


    if (is_no_value_word(input_str))
    {
        *float_ptr = FLT_NOT_SET;
        return NO_ERROR;
    }

    BUFF_CPY(str_buff, input_str);
    str = str_buff;

    trim_beg(&str);
    trim_end(str);

    if (*str == '\0')
    {
        set_error("Expecting a number but recieved nothing.");
        return ERROR;
    }
    else if (FIND_CHAR_YES(str, ' '))
    {
        set_error("Expecting a single number but recieved %q",
                  str);
        return ERROR;
    }

    last_char_ptr = str + strlen(str) - 1;

    if (*last_char_ptr == '%')
    {
        percentage = TRUE;
        *last_char_ptr = '\0';
    }

    BUFF_CPY(str_copy, str);
    extended_uc_lc(str_copy);
    char_for_char_translate(str_copy, 'd', 'e');

    if (is_scientific_notation_number(str_copy))
    {
        if (sscanf(str_copy, "%e", float_ptr) != 1)
        {
            set_bug("Unexpected return from sscanf in ss1snf with argument %q.",
                    str_copy);
            return ERROR;
        }
    }
    else
    {
        set_error("%q isn't a valid number.", str);
        return ERROR;
    }

    if (percentage)
    {
        *float_ptr /= (float)100.0;
    }

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*
 * =============================================================================
 *                                  ss1d
 *
 * Scan a double from a string in fixed decimal format
 *
 * This routine reads a double from a string in fixed decimal format with more
 * sophisticated error reporting that available using scanf. If the contents of
 * "str" is exactly one number which is appropriate for a double, then it sets
 * *double_ptr to that number and returns NO_ERROR.  Otherwise and error message
 * is set, and ERROR is returned.  The contents of *double_ptr are only changed
 * on success.
 *
 * If str is either "off" or "none", then the special value DBL_NOT_SET is
 * returned. Otherwise, str must contain a single valid number, possibly
 * followed by a "%".  If str ends with a "%", then the number is assumed to be
 * a percentage, and the result is divided by 100.0.
 *
 * Note:
 *    For successful return, "str" cannot contain two numbers. For example, it
 *    can't be something like "1 2".
 *
 * Returns:
 *    If "str" represents a valid double, then NO_ERROR is returned. Otherwise
 *    the problem with "str" is reported in the set error message, and ERRROR is
 *    returned.  The problem with "str" can be printed with kjb_print_error.
 *
 * Index: I/O, scanning
 *
 * -----------------------------------------------------------------------------
 */

int ss1d(const char* input_str, double* double_ptr)
{
    char  str_buff[ 200 ];
    char* str;
    char* str_digits;
    char* last_char_ptr;
    int   percentage      = FALSE;
    Always_double temp_double;


    if (is_no_value_word(input_str))
    {
        *double_ptr = DBL_NOT_SET;
        return NO_ERROR;
    }

    BUFF_CPY(str_buff, input_str);
    str = str_buff;

    trim_beg(&str);
    trim_end(str);

    if (*str == '\0')
    {
        set_error("Expecting a number but recieved nothing.");
        return ERROR;
    }
    else if (    ((fs_scan_nan) && ((STRCMP_EQ(str, "nan")) || (STRCMP_EQ(str, "NAN")) || (STRCMP_EQ(str, "NaN"))))
              || ((fs_scan_inf) && ((STRCMP_EQ(str, "inf")) || (STRCMP_EQ(str, "-inf")) || (STRCMP_EQ(str, "+inf"))))
              || ((fs_scan_inf) && ((STRCMP_EQ(str, "INF")) || (STRCMP_EQ(str, "-INF")) || (STRCMP_EQ(str, "+INF"))))
            )
    {
        ; /* Do nothing. */
    }
    else 
    {
        last_char_ptr = str + strlen(str) - 1;

        if (*last_char_ptr == '%')
        {
            percentage = TRUE;
            *last_char_ptr = '\0';
        }

        str_digits = str; 

        if (*str_digits == '+') 
        { 
            ++str_digits;

            if (*str_digits == '\0')
            {
                set_error("Expecting a fixed decimal number but recieved only \"+\".");
                return ERROR;
            }
        }
        else if (*str_digits == '-')
        {
            str_digits++;

            if (*str_digits == '\0')
            {
                set_error("Expecting a fixed decimal number but recieved only \"-\".");
                return ERROR;
            }
        }

        while ( (isdigit((int)(*str_digits))) &&
                (*str_digits != '\0') )
        {
             ++str_digits;
        }

        while ( (isdigit((int)(*str_digits))) &&
                (*str_digits != '\0') )
        {
             ++str_digits;
        }

        if (*str_digits == '.')  ++str_digits;

        if ( ! all_digits(str_digits))
        {
            set_error("%q isn't a valid fixed decimal number.",
                      str);
            return ERROR;
        }
    }

    if (sscanf(str, "%lf", &temp_double) != 1)
    {
        set_bug("Unexpected return from sscanf in ss1d with argument %q.", str);
        return ERROR;
    }

    *double_ptr = temp_double;

    if (percentage)
    {
        *double_ptr /= 100.0;
    }

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*
 * =============================================================================
 *                                  ss1snd
 *
 * Scan a double from a string in scientific notation
 *
 * This routine reads a double from a string in scientific notation with more
 * sophisticated error reporting that available using scanf. If the contents of
 * "str" is exactly one number which is appropriate for a double, then it sets
 * *double_ptr to that number and returns NO_ERROR.  Otherwise and error message
 * is set, and ERROR is returned.  The contents of *double_ptr are only changed
 * on success.
 *
 * If str is either "off" or "none", then the special value DBL_NOT_SET is
 * returned. Otherwise, str must contain a single valid number, possibly
 * followed by a "%".  If str ends with a "%", then the number is assumed to be
 * a percentage, and the result is divided by 100.0.
 *
 * Note:
 *    For successful return, "str" cannot contain two numbers. For example, it
 *    can't be something like "1 2".
 *
 * Returns:
 *    If "str" represents a valid double, then NO_ERROR is returned. Otherwise
 *    the problem with "str" is reported in the set error message, and ERRROR is
 *    returned.  The problem with "str" can be printed with kjb_print_error.
 *
 * Index: I/O, scanning
 *
 * -----------------------------------------------------------------------------
 */

int ss1snd(const char* input_str, double* double_ptr)
{
    char str_copy[ 200 ];
    char str_buff[ 200 ];
    char*         str;
    Always_double temp_double;
    int           percentage    = FALSE;
    char*         last_char_ptr;


    if (is_no_value_word(input_str))
    {
        *double_ptr = DBL_NOT_SET;
        return NO_ERROR;
    }

    BUFF_CPY(str_buff, input_str);
    str = str_buff;

    trim_beg(&str);
    trim_end(str);

    if (*str == '\0')
    {
        set_error("Expecting a number but recieved nothing.");
        return ERROR;
    }

    if (FIND_CHAR_YES(str, ' '))
    {
        set_error("Expecting a single number but recieved %q",
                  str);
        return ERROR;
    }

    last_char_ptr = str + strlen(str) - 1;

    if (*last_char_ptr == '%')
    {
        percentage = TRUE;
        *last_char_ptr = '\0';
    }

    BUFF_CPY(str_copy, str);
    extended_uc_lc(str_copy);
    char_for_char_translate(str_copy, 'd', 'e');

    if (is_scientific_notation_number(str_copy))
    {
        if (sscanf(str_copy, "%le", &temp_double) != 1)
        {
            set_bug("Unexpected return from sscanf in ss1snd with argument %q.",
                    str_copy);
            return ERROR;
        }
        *double_ptr = temp_double;
    }
    else
    {
        set_error("%q isn't a valid number.", str);
        return ERROR;
    }

    if (percentage)
    {
        *double_ptr /= 100.0;
    }

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

int is_scientific_notation_number(const char* str)
{
    const char* str_pos;
    int         found_a_digit = FALSE;


    str_pos = str;
    const_trim_beg(&str_pos);

    /* We can have either lower case, or upper case, but we cannot mix cases
     * unless we want to translate before using sscanf().
    */
    if (    ((fs_scan_nan) && ((STRCMP_EQ(str, "nan")) || (STRCMP_EQ(str, "NAN")) || (STRCMP_EQ(str, "NaN"))))
         || ((fs_scan_inf) && ((STRCMP_EQ(str, "inf")) || (STRCMP_EQ(str, "-inf")) || (STRCMP_EQ(str, "+inf"))))
         || ((fs_scan_inf) && ((STRCMP_EQ(str, "INF")) || (STRCMP_EQ(str, "-INF")) || (STRCMP_EQ(str, "+INF"))))
       )
    {
        return TRUE;
    }

    if ((*str_pos == '-') || (*str_pos == '+'))
    {
        str_pos++;
    }

    if (isdigit((int)(*str_pos))) found_a_digit = TRUE;

    while (isdigit((int)(*str_pos)))
    {
        str_pos++;
    }

    if (*str_pos == '.')
    {
        str_pos++;

        if (isdigit((int)(*str_pos))) found_a_digit = TRUE;

        while (isdigit((int)(*str_pos)))
        {
            str_pos++;
        }
    }

    if (*str_pos == 'e')
    {
        str_pos++;

        if ((*str_pos == '-') || (*str_pos == '+'))
        {
            str_pos++;
        }

        if ( ! isdigit((int)(*str_pos))) return FALSE;

        while (isdigit((int)(*str_pos)))
        {
            str_pos++;
        }
    }

    if ( ! found_a_digit ) return FALSE;

    if (all_white_space(str_pos))
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

#ifdef __cplusplus
}
#endif

