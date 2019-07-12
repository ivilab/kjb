
/* $Id: l_verbose.c 21520 2017-07-22 15:09:04Z kobus $ */

/* =========================================================================== *
|
|  Copyright (c) 1994-2014 by Kobus Barnard (author).
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
#include "l/l_sys_io.h"
#include "l/l_sys_str.h"
#include "l/l_parse.h"
#include "l/l_string.h"
#include "l/l_sys_scan.h"
#include "l/l_verbose.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ------------------------------------------------------------------------- */

#ifdef TEST
    static int fs_verbose_level = 5;
#else
    static int fs_verbose_level = 0;
#endif

static Bool fs_warning_enable = TRUE;
static int fs_interactive_verbose_equivalence = 10;

/* ------------------------------------------------------------------------- */

int set_verbose_options(const char* option, const char* value)
{
    char lc_option[ 100 ];
    int  temp_int_value;
    int  temp_boolean_value;
    int  result = NOT_FOUND;


    EXTENDED_LC_BUFF_CPY(lc_option, option);

    if (    (lc_option[ 0 ] == '\0')
         || match_pattern(lc_option, "verbose")
         || match_pattern(lc_option, "verbose-level")
       )
    {
        if (value == NULL)
        {
            return NO_ERROR;
        }
        else if (value[ 0 ] == '?')
        {
            ERE(pso("verbose = %d\n", fs_verbose_level));
        }
        else if (value[ 0 ] == '\0')
        {
            ERE(pso("Verbose level is %d.\n", fs_verbose_level));
        }
        else
        {
            ERE(ss1i(value, &temp_int_value));
            result = kjb_set_verbose_level(temp_int_value);
        }
        result = NO_ERROR;
    }

    if (    (lc_option[ 0 ] == '\0')
         || match_pattern(lc_option, "interactive-verbose-equivalence")
         || match_pattern(lc_option, "interactive-verbose")
       )
    {
        if (value == NULL)
        {
            return NO_ERROR;
        }
        else if (value[ 0 ] == '?')
        {
            ERE(pso("interactive-verbose-equivalence = %d\n",
                    fs_interactive_verbose_equivalence));
        }
        else if (value[ 0 ] == '\0')
        {
            ERE(pso("Interactive output is printed at verbose level %d.\n",
                    fs_interactive_verbose_equivalence));
        }
        else
        {
            ERE(ss1i(value, &temp_int_value));
            fs_interactive_verbose_equivalence = temp_int_value;
        }
        result = NO_ERROR;
    }

    if (    (lc_option[ 0 ] == '\0')
         || match_pattern(lc_option, "warnings")
         || match_pattern(lc_option, "enable-warnings")
       )
    {
        if (value == NULL)
        {
            return NO_ERROR;
        }
        else if (value[ 0 ] == '?')
        {
            ERE(pso("warnings = %s\n", fs_warning_enable ? "t" : "f"));
        }
        else if (value[ 0 ] == '\0')
        {
            ERE(pso("Warnings %s enabled.\n", fs_warning_enable ? "are" : "are not" ));
        }
        else
        {
            ERE(temp_boolean_value = get_boolean_value(value));
            ASSERT_IS_BOOL(temp_boolean_value);
            fs_warning_enable = (Bool)temp_boolean_value;
            ASSERT_IS_BOOL(fs_warning_enable);
        }
        result = NO_ERROR;
    }

    return result;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*
 * =============================================================================
 *                              kjb_set_verbose_level
 *
 * Sets the verbose level for the KJB library routines.
 *
 * This routine sets the verbose level for the KJB library routines. This
 * affects all calls to verbose_pso(3-KJB).
 *
 * Index: I/O, verbose, debugging
 *
 * -----------------------------------------------------------------------------
 */

int kjb_set_verbose_level(int new_level)
{


    fs_verbose_level = new_level;

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*
 * =============================================================================
 *                              kjb_get_verbose_level
 *
 * Returns the verbose level for the KJB library routines.
 *
 * This routine returns the verbose level for the KJB library routines.
 *
 * Index: I/O, verbose, debugging
 *
 * -----------------------------------------------------------------------------
 */

int kjb_get_verbose_level(void)
{


    return fs_verbose_level;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*
 * =============================================================================
 *                                 verbose_pso
 *
 * Printing to stdout if verbose level is high enough.
 *
 * This routine writes a formatted string to stdout if the first paramter is
 * equal to or exceeds the verbose level set by kjb_set_verbose_level.   When
 * verbose_pso prints, it prepends a string to each line which identifies that
 * the verbosity level cutoff. That string is "<< %d >> ". Otherwise it is
 * similar to pso. Specifically, it is similar to kjb_fprintf with respect to
 * extended formatting options and paging.
 *
 * Returns:
 *    Returns the number of characters written, which is zero in the case of
 *    the verbose level being smaller than the argument. ERROR is returned if
 *    there is an error.
 *
 * Related:
 *    verbose_puts, pso, kjb_fprintf
 *
 * Index: I/O, verbose, debugging
 *
 * -----------------------------------------------------------------------------
*/

/*PRINTFLIKE2*/
long verbose_pso(int cut_off, const char* format_str, ...)
{
    int        sprintf_res;
    va_list    ap;
    char       buff[ 1000 ];


    if (cut_off > fs_verbose_level) return 0;

    va_start(ap, format_str);

    sprintf_res = kjb_vsprintf(buff, sizeof(buff), format_str, ap);

    va_end(ap);

    if (sprintf_res == ERROR) return ERROR;

    return verbose_puts(cut_off, buff);
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*
 * =============================================================================
 *                                 verbose_puts
 *
 * Printing to stdout if verbose level is high enough.
 *
 * This routine writes a string to stdout if the first paramter is equal to or
 * exceeds the verbose level set by kjb_set_verbose_level.   When verbose_pso
 * prints, it prepends a string to each line which identifies that the verbosity
 * level cutoff. That string is "<< %d >> ". Otherwise it is similar to
 * kjb_puts.
 *
 * Returns:
 *    Returns the number of characters written, which is zero in the case of
 *    the verbose level being smaller than the argument. ERROR is returned if
 *    there is an error.
 *
 * Related:
 *    verbose_pso, kjb_puts
 *
 * Index: I/O, verbose, debugging
 *
 * -----------------------------------------------------------------------------
*/

long verbose_puts(int cut_off, const char* buff)
{
    static Bool  begining_of_line = TRUE;
    const char* buff_pos;
    int         out_res;
    int         result           = 0;


    if (cut_off > fs_verbose_level) return result;

    buff_pos = buff;

    /*CONSTCOND*/
    while (TRUE)
    {
        if (begining_of_line)
        {
            ERE(out_res = pso("<< %2d >> ", cut_off));
            result += out_res;
            begining_of_line = FALSE;
        }

        while (*buff_pos != '\n')
        {
            if (*buff_pos == '\0') break;

            kjb_putc(*buff_pos);
            result++;
            buff_pos++;
        }

        if (*buff_pos == '\0') break;

        kjb_putc(*buff_pos);
        buff_pos++;
        begining_of_line = TRUE;
        result++;

        if (*buff_pos == '\0') break;
    }

    kjb_flush();

    return result;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*
 * =============================================================================
 *                                 warn_pso
 *
 * Printing to stdout with the prefix "<< Warning >> ".
 *
 * This routine writes a formatted string to stdout with the prefix
 * "<< Warning >> " pre-pended to each line.  Otherwise it is similar to pso.
 * Specifically, it is similar to kjb_fprintf with respect to extended
 * formatting options and paging.
 *
 * Warnings can be suppressed by the user with "set warnings = off" or by the
 * programmer by setting up the approprate call to the option infrastructure.
 *
 * Returns:
 *    Returns the number of characters written.  ERROR is returned if there is
 *    an error.
 *
 * Related:
 *    warn_puts, pso, kjb_fprintf
 *
 * Index: I/O, warning messages, debugging
 *
 * -----------------------------------------------------------------------------
*/

/*PRINTFLIKE1*/
long warn_pso(const char* format_str, ...)
{
    int     sprintf_res;
    va_list ap;
    char    buff[ 10000 ];

    if ( ! fs_warning_enable) return NO_ERROR;

    va_start(ap, format_str);

    sprintf_res = kjb_vsprintf(buff, sizeof(buff), format_str, ap);

    va_end(ap);

    if (sprintf_res == ERROR) return ERROR;

    return warn_puts(buff);
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*
 * =============================================================================
 *                                 warn_puts
 *
 * Printing to stdout with prefix "<< Warning >> "
 *
 * This routine writes a string to stdout with the prefix "<< Warning >> "
 * pre-pended to each line. therwise it is similar to kjb_puts.
 *
 * Warnings can be suppressed by the user with "set warnings = off" or by the
 * programmer by setting up the approprate call to the option infrastructure.
 *
 * Returns:
 *    Returns the number of characters written.  ERROR is returned if there is
 *    an error.
 *
 * Related:
 *    warn_pso, kjb_puts
 *
 * Index: I/O, warning messages, debugging
 *
 * -----------------------------------------------------------------------------
*/

long warn_puts(const char* buff)
{
    static Bool begining_of_line   = TRUE;
    const char*      buff_pos;
#if 0 /* was ifdef HOW_IT_WAS */
    char       line_buff[ 10000 ];
    int        get_res;
#endif
    int        out_res;
    int        result = 0;

    if ( ! fs_warning_enable) return NO_ERROR;

    buff_pos = buff;

    /*CONSTCOND*/
    while (TRUE)
    {
        if (begining_of_line)
        {
            ERE(out_res = kjb_puts("<< Warning >> "));
            result += out_res;
            begining_of_line = FALSE;
        }

#if 0 /* was ifdef HOW_IT_WAS */
        while (*buff_pos == '\n')
        {
            begining_of_line = TRUE;
            ERE(out_res = pso("\n"));
            result += out_res;
            buff_pos++;
        }

        ERE(get_res = BUFF_GEN_GET_TOKEN(&buff_pos, line_buff, "\n"));
        if (get_res == NO_MORE_TOKENS) break;

        ERE(out_res = kjb_puts(line_buff));
        result += out_res;
#else
        while (*buff_pos != '\n')
        {
            if (*buff_pos == '\0') break;

            kjb_putc(*buff_pos);
            result++;
            buff_pos++;
        }

        if (*buff_pos == '\0') break;

        kjb_putc(*buff_pos);
        buff_pos++;
        begining_of_line = TRUE;
        result++;

        if (*buff_pos == '\0') break;
#endif
    }

    kjb_flush();

    return result;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*
 * =============================================================================
 *                                 interactive_pso
 *
 * Printing to stdout if interactive or verbose level is high enough.
 *
 * This routine writes a formatted string to stdout if the program is being run
 * interactively, or if the verbose level exceeds the level at which we force
 * interactive output to be converted to verbose output.
 *
 * Returns:
 *    Returns the number of characters written, which is zero in the case of
 *    the verbose level being smaller than the argument. ERROR is returned if
 *    there is an error.
 *
 * Related:
 *    interactive_puts, pso, kjb_fprintf
 *
 * Index: I/O, verbose, debugging
 *
 * -----------------------------------------------------------------------------
*/

/*PRINTFLIKE1*/
long interactive_pso(const char* format_str, ...)
{
    int        sprintf_res;
    va_list    ap;
    char       buff[ 10000 ];


    if (    (fs_interactive_verbose_equivalence > fs_verbose_level)
         && ( ! is_interactive())
       )
    {
        return 0;
    }

    va_start(ap, format_str);

    sprintf_res = kjb_vsprintf(buff, sizeof(buff), format_str, ap);

    va_end(ap);

    if (sprintf_res == ERROR) return ERROR;

    return interactive_puts(buff);
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*
 * =============================================================================
 *                                 interactive_puts
 *
 * Printing to stdout if interactive or verbose level is high enough.
 *
 * This routine writes a formatted string to stdout if the program is being run
 * interactively, or if the verbose level exceeds the level at which we force
 * interactive output to be converted to verbose output.
 *
 * Returns:
 *    Returns the number of characters written, which is zero in the case of
 *    the verbose level being smaller than the argument. ERROR is returned if
 *    there is an error.
 *
 * Related:
 *    interactive_pso, kjb_puts
 *
 * Index: I/O, verbose, debugging
 *
 * -----------------------------------------------------------------------------
*/

long interactive_puts(const char* buff)
{
    if (is_interactive())
    {
        return kjb_puts(buff);
    }
    else
    {
        return verbose_puts(fs_interactive_verbose_equivalence, buff);
    }
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

#ifdef __cplusplus
}
#endif

