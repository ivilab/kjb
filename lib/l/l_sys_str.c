
/* $Id: l_sys_str.c 21656 2017-08-05 15:34:14Z kobus $ */

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
#include "l/l_string.h"
#include "l/l_parse.h"
#include "l/l_sys_scan.h"
#include "l/l_sys_str.h"
#include "l/l_verbose.h"

#ifdef __cplusplus
extern "C" {
#endif

/* -------------------------------------------------------------------------- */

/* =============================================================================
 *                              kjb_sprintf
 *
 * A version of sprintf that checks for buffer overflow and has some extras
 *
 * This routine is similar to sprintf(), except that it checks for buffer
 * overflow. The extra formatting items described in kjb_fprintf() are
 * available.  Thus is it is more similar to snprinf(), but that routine is not
 * universally available (at least at the time this routine was written). 
 *
 * We don't have a separate routine for when we want buffer overflow to be
 * checked because it should always be checked. 
 *
 * Warning:
 *    The second parameter of this routine is the size of the buffer. This is
 *    DIFFERENT than sprintf!
 *
 * Returns:
 *    If successful, this returns the length of the printed string (not
 *    including the null terminator character), just like sprintf.
 *    This returns ERROR if a problem is encountered.
 *
 * Related:
 *    kjb_fprintf
 *
 * Index: strings
 *
 * -----------------------------------------------------------------------------
*/

/*PRINTFLIKE3*/
long kjb_sprintf(char* buff, size_t max_len, const char* format_str, ...)
{
    int     result;
    va_list ap;


    va_start(ap, format_str);

    result = kjb_vsprintf(buff, max_len, format_str, ap);

    va_end(ap);

    return result;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                              kjb_vsprintf
 *
 * A version of vprintf that checks for buffer overflow and has some extras
 *
 * This routine is mostly use to construct others (e.g. kjb_sprintf()). However,
 * it may be useful outside this context, hence we export the routine. 
 *
 * This routine is similar to vprintf(), except that it checks for buffer
 * overflow. The extra formatting items described in kjb_fprintf() are
 * avaliable.  Thus is it is more similar to vnprintf(), but that routine is not
 * universally available (at least at the time this routine was written). 
 *
 * We don't have a separate routine for when we want buffer overflow to be
 * checked becuase it should always be checked. 
 *
 * Warning:
 *    The second parameter of this routine is the size of the buffer. This is
 *    DIFFERENT than vprintf!
 *
 * Returns:
 *    If successful, this returns the length of the printed string (not
 *    including the null terminator character), just like vsprintf.  This
 *    returns TRUNCATED if there was not enough room in the buffer (what was
 *    able to be written should be terminated with a NULL), and ERROR any other
 *    a problem is encountered.
 *
 * Note: 
 *    We changed (back?) to returning TRUNCATED instead of calling
 *    SET_BUFFER_OVERFLOW_BUG() in summer 2017. This routine is used in error
 *    handling, and having to truncate is not always a bug, so calling functions
 *    that want to treat this as a bug will have to check the return. 
 *
 * Related:
 *    kjb_fprintf, kjb_sprintf
 *
 * -----------------------------------------------------------------------------
*/

/*
 *  Note: This routine should always leave buff in a reaonable state (e.g.
 *        possibly a null terminated string) as it may be called from
 *        kjb_error.
*/

long kjb_vsprintf
(
    char*       buff,
    size_t      max_len,
    const char* format_str,
    va_list     ap
)
{
    long         save_max_len;
    char*        new_format_str_pos;
    int          format_item;
    char         type_char;
    int          num_precision_args;
    int          precision_arg1;
    int          precision_arg2;
    Bool         short_flag;
    Bool         long_flag;
#ifdef LONG_DOUBLE_OK
    Bool         long_double_flag;
#endif
    char         new_format_str[ MAX_FORMAT_STRING_SIZE ];
    char         trunc_buff[ 200 ];
    Bool         justify_flag;
    Bool         field_width_flag;
    Bool         comma_flag;
    Bool         precision_flag;
    Bool         field_width_arg_flag;
    int          field_width;
    size_t       field_size;
    int          temp_field_width;
    char*        buff_pos;
    size_t       temp_buff_len;
    char         temp_buff[ MAX_FORMAT_STRING_SIZE ];
    const char*  temp_buff_pos;
    long res;


    if ((buff == NULL) || (format_str == NULL) || (max_len < 1))
    {
        SET_ARGUMENT_BUG();
        return ERROR;
    }

#ifdef TEST
    if (max_len == sizeof(char*))
    {
        warn_pso("Suspicious buffer length (%lu) at line %d of %s.\n",
                 (unsigned long)max_len, __LINE__, __FILE__);
        kjb_optional_abort();
    }
#endif

    buff[ 0 ] =  '\0';
    buff_pos = buff;

    /* Save room for the terminating null. */
    max_len--;
    save_max_len = max_len;

    while (*format_str != '\0')
    {
        format_item = FALSE;

        while ((!format_item) &&
               (*format_str != '\0') &&
               (max_len > 0))
        {
            if (*format_str == '%')
            {
                format_item = TRUE;
            }
            else
            {
                *buff_pos++ = *format_str++;
                max_len--;
            }
        }

        *buff_pos = '\0';

        if ((max_len <= 0) && (*format_str != '\0'))
        {
            return TRUNCATED;
        }

        if (format_item)     /* Can't be at string end if true */
        {
            short_flag = FALSE;
            long_flag = FALSE;
#ifdef LONG_DOUBLE_OK
            long_double_flag = FALSE;
#endif
            num_precision_args = 0;
            justify_flag = field_width_flag = precision_flag = FALSE;
            comma_flag = field_width_arg_flag = FALSE;

            new_format_str_pos = new_format_str;
            *new_format_str_pos = *format_str;
            format_str++;
            new_format_str_pos++;

            /*
             * FIX: Currently we accept stuff like "-+  -  +". Not serious,
             *      as this stuff just gets passed down to sprintf anyway.
             *      BUT we DONT look at sprintf's return!
             */
            while (    (*format_str == '-')
                    || (*format_str == ' ')
                    || (*format_str == '0')
                    || (*format_str == '#')
                    || (*format_str == '+')
                    || (*format_str == ',')
                  )
            {
                if (*format_str == '-')
                {
                    justify_flag = TRUE;
                }

                if (*format_str == ',')
                {
                    comma_flag = TRUE;
                }
                else
                {
                    *new_format_str_pos = *format_str;
                    new_format_str_pos++;
                }

                format_str++;
            }

            if (*format_str == '*')
            {
                *new_format_str_pos = *format_str;
                new_format_str_pos++;
                format_str++;

                num_precision_args++;
                field_width_flag = TRUE;
                field_width_arg_flag = TRUE;
            }
            else
            {
                if (isdigit((int)(*format_str)))
                {
                    field_width_flag = TRUE;
                }

                while (isdigit((int)(*format_str)))
                {
                    *new_format_str_pos = *format_str;
                    new_format_str_pos++;
                    format_str++;
                }
            }

            if (*format_str == '.')
            {
                *new_format_str_pos = *format_str;
                new_format_str_pos++;
                format_str++;

                if (*format_str == '*')
                {
                    *new_format_str_pos = *format_str;
                    new_format_str_pos++;
                    format_str++;

                    num_precision_args++;
                    precision_flag = TRUE;
                }
                else
                {
                    if (isdigit((int)(*format_str)))
                    {
                        precision_flag = TRUE;
                    }

                    while (isdigit((int)(*format_str)))
                    {
                        *new_format_str_pos = *format_str;
                        new_format_str_pos++;
                        format_str++;
                    }
                }
            }

            if (*format_str == 'l')
            {
                long_flag = TRUE;
                *new_format_str_pos = *format_str;
                new_format_str_pos++;
                format_str++;
            }
            else if (*format_str == 'h')
            {
                short_flag = TRUE;
                *new_format_str_pos = *format_str;
                new_format_str_pos++;
                format_str++;
            }
            else if (*format_str == 'L')
            {
#ifdef LONG_DOUBLE_OK
                long_double_flag = TRUE;
                *new_format_str_pos = *format_str;
                new_format_str_pos++;
#endif
                format_str++;
            }

            type_char = *format_str;

            if (*format_str != '\0')
            {
                format_str++;
            }

            if (type_char == '\0')
            {
                *buff_pos = '\0';
                SET_FORMAT_STRING_BUG();
                return ERROR;
            }

            if ((type_char == 'd') || (type_char == 'i'))
            {
                *new_format_str_pos = type_char;
                new_format_str_pos++;
                *new_format_str_pos = '\0';

                if (num_precision_args == 0)
                {
                    /*
                    // Untested since changing "short"to "int" due to gcc
                    // complaining that there may be a problem because
                    // short gets promoted to int. Fair enough, but I don't
                    // know whether this is correct solution across all
                    // platforms?
                     */
                    if (short_flag)
                    {
#ifdef MS_OS            /* snprintf seems to be missing. */
                        res = sprintf(temp_buff, 
                                       new_format_str,
                                       va_arg(ap, int));
#else
                        res = snprintf(temp_buff, sizeof(temp_buff),
                                       new_format_str,
                                       va_arg(ap, int));
#endif
                    }
                    else if (long_flag)
                    {
#ifdef MS_OS            /* snprintf seems to be missing. */
                        res = sprintf(temp_buff, 
                                       new_format_str,
                                       va_arg(ap, long));
#else
                        res = snprintf(temp_buff, sizeof(temp_buff),
                                       new_format_str,
                                       va_arg(ap, long));
#endif
                    }
                    else
                    {
#ifdef MS_OS            /* snprintf seems to be missing. */
                        res = sprintf(temp_buff, 
                                       new_format_str,
                                       va_arg(ap, int));
#else
                        res = snprintf(temp_buff, sizeof(temp_buff),
                                       new_format_str,
                                       va_arg(ap, int));
#endif
                    }
                }
                else if (num_precision_args == 1)
                {
                    precision_arg1 = va_arg(ap, int);

                    /*
                    // Untested since changing "short"to "int" due to gcc
                    // complaining that there may be a problem because
                    // short gets promoted to int. Fair enough, but I don't
                    // know whether this is correct solution across all
                    // platforms?
                     */
                    if (short_flag)
                    {
#ifdef MS_OS            /* snprintf seems to be missing. */
                        res = sprintf(temp_buff, 
                                       new_format_str,
                                       precision_arg1,
                                       va_arg(ap, int));
#else
                        res = snprintf(temp_buff, sizeof(temp_buff),
                                       new_format_str,
                                       precision_arg1,
                                       va_arg(ap, int));
#endif
                    }
                    else if (long_flag)
                    {
#ifdef MS_OS            /* snprintf seems to be missing. */
                        res = sprintf(temp_buff, 
                                       new_format_str,
                                       precision_arg1,
                                       va_arg(ap, long));
#else
                        res = snprintf(temp_buff, sizeof(temp_buff),
                                       new_format_str,
                                       precision_arg1,
                                       va_arg(ap, long));
#endif
                    }
                    else
                    {
#ifdef MS_OS            /* snprintf seems to be missing. */
                        res = sprintf(temp_buff, 
                                       new_format_str,
                                       precision_arg1,
                                       va_arg(ap, int));
#else
                        res = snprintf(temp_buff, sizeof(temp_buff),
                                       new_format_str,
                                       precision_arg1,
                                       va_arg(ap, int));
#endif
                    }
                }
                else if (num_precision_args == 2)
                {
                    precision_arg1 = va_arg(ap, int);
                    precision_arg2 = va_arg(ap, int);

                    if (short_flag)
                    {
                        /*
                        // Untested since changing "short"to "int" due to gcc
                        // complaining that there may be a problem because
                        // short gets promoted to int. Fair enough, but I don't
                        // know whether this is correct solution across all
                        // platforms?
                         */
#ifdef MS_OS            /* snprintf seems to be missing. */
                        res = sprintf(temp_buff, 
                                       new_format_str,
                                       precision_arg1,
                                       precision_arg2,
                                       va_arg(ap, int));
#else
                        res = snprintf(temp_buff, sizeof(temp_buff),
                                       new_format_str,
                                       precision_arg1,
                                       precision_arg2,
                                       va_arg(ap, int));
#endif
                    }
                    else if (long_flag)
                    {
#ifdef MS_OS            /* snprintf seems to be missing. */
                        res = sprintf(temp_buff, 
                                       new_format_str,
                                       precision_arg1,
                                       precision_arg2,
                                       va_arg(ap, long));
#else
                        res = snprintf(temp_buff, sizeof(temp_buff),
                                       new_format_str,
                                       precision_arg1,
                                       precision_arg2,
                                       va_arg(ap, long));
#endif
                    }
                    else
                    {
#ifdef MS_OS            /* snprintf seems to be missing. */
                        res = sprintf(temp_buff, 
                                       new_format_str,
                                       precision_arg1,
                                       precision_arg2,
                                       va_arg(ap, int));
#else
                        res = snprintf(temp_buff, sizeof(temp_buff),
                                       new_format_str,
                                       precision_arg1,
                                       precision_arg2,
                                       va_arg(ap, int));
#endif
                    }
                }
                else
                {
                    *buff_pos = '\0';
                    SET_CANT_HAPPEN_BUG();
                    return ERROR;
                }

                if (res == EOF)
                {
                    set_bug("Format error in %q:%S", new_format_str);
                    return ERROR;
                }

                /*
                // FIX
                //
                // Currently, commas will not be inserted into anything other
                // than non-justified positive numbers. Note that commas do not
                // properly work with precision arguments!
                 */
                if (comma_flag && all_digits(temp_buff))
                {
                    char buff_copy[ 100 ];
                    char* buff_copy_pos = buff_copy;
                    int  len = strlen(temp_buff);
                    int first_chunk = (len - 1) % 3 + 1;
                    int i;

                    i = 0;

                    while (i<first_chunk)
                    {
                        *buff_copy_pos = temp_buff[ i ];
                        buff_copy_pos++;
                        i++;
                    }

                    while (i<len)
                    {
                        *buff_copy_pos++ = ',';
                        *buff_copy_pos++ = temp_buff[ i++ ];
                        *buff_copy_pos++ = temp_buff[ i++ ];
                        *buff_copy_pos++ = temp_buff[ i++ ];
                    }

                    *buff_copy_pos = '\0';

                    BUFF_CPY(temp_buff, buff_copy);
                }

                temp_buff_len = strlen(temp_buff);

                if (temp_buff_len > max_len)
                {
                    *buff_pos = '\0';
                    return TRUNCATED;
                }

                strcpy(buff_pos, temp_buff);
                buff_pos += temp_buff_len;
                max_len -= temp_buff_len;
            }
            else if ( (type_char == 'o') || (type_char == 'x') ||
                      (type_char == 'u') || (type_char == 'X') )
            {
                *new_format_str_pos = type_char;
                new_format_str_pos++;
                *new_format_str_pos = '\0';

                if (num_precision_args == 0)
                {
                    if (short_flag)
                    {
                        /*
                        // Untested since changing "short"to "int" due to gcc
                        // complaining that there may be a problem because
                        // short gets promoted to int. Fair enough, but I don't
                        // know whether this is correct solution across all
                        // platforms?
                         */
#ifdef MS_OS            /* snprintf seems to be missing. */
                        res = sprintf(temp_buff, 
                                       new_format_str,
                                       va_arg(ap, unsigned int));
#else
                        res = snprintf(temp_buff, sizeof(temp_buff),
                                       new_format_str,
                                       va_arg(ap, unsigned int));
#endif
                    }
                    else if (long_flag)
                    {
#ifdef MS_OS            /* snprintf seems to be missing. */
                        res = sprintf(temp_buff, 
                                       new_format_str,
                                       va_arg(ap, unsigned long));
#else
                        res = snprintf(temp_buff, sizeof(temp_buff),
                                       new_format_str,
                                       va_arg(ap, unsigned long));
#endif
                    }
                    else
                    {
#ifdef MS_OS            /* snprintf seems to be missing. */
                        res = sprintf(temp_buff, 
                                       new_format_str,
                                       va_arg(ap, unsigned int));
#else
                        res = snprintf(temp_buff, sizeof(temp_buff),
                                       new_format_str,
                                       va_arg(ap, unsigned int));
#endif
                    }
                }
                else if (num_precision_args == 1)
                {
                    precision_arg1 = va_arg(ap, int);

                    if (short_flag)
                    {
                        /*
                        // Untested since changing "short"to "int" due to gcc
                        // complaining that there may be a problem because
                        // short gets promoted to int. Fair enough, but I don't
                        // know whether this is correct solution across all
                        // platforms?
                         */
#ifdef MS_OS            /* snprintf seems to be missing. */
                        res = sprintf(temp_buff, 
                                       new_format_str,
                                       precision_arg1,
                                       va_arg(ap, unsigned int));
#else
                        res = snprintf(temp_buff, sizeof(temp_buff),
                                       new_format_str,
                                       precision_arg1,
                                       va_arg(ap, unsigned int));
#endif
                    }
                    else if (long_flag)
                    {
#ifdef MS_OS            /* snprintf seems to be missing. */
                        res = sprintf(temp_buff, 
                                       new_format_str,
                                       precision_arg1,
                                       va_arg(ap, unsigned long));
#else
                        res = snprintf(temp_buff, sizeof(temp_buff),
                                       new_format_str,
                                       precision_arg1,
                                       va_arg(ap, unsigned long));
#endif
                    }
                    else
                    {
#ifdef MS_OS            /* snprintf seems to be missing. */
                        res = sprintf(temp_buff, 
                                       new_format_str,
                                       precision_arg1,
                                       va_arg(ap, unsigned int));
#else
                        res = snprintf(temp_buff, sizeof(temp_buff),
                                       new_format_str,
                                       precision_arg1,
                                       va_arg(ap, unsigned int));
#endif
                    }
                }
                else if (num_precision_args == 2)
                {
                    precision_arg1 = va_arg(ap, int);
                    precision_arg2 = va_arg(ap, int);

                    if (short_flag)
                    {
                        /*
                        // Untested since changing "short"to "int" due to gcc
                        // complaining that there may be a problem because
                        // short gets promoted to int. Fair enough, but I don't
                        // know whether this is correct solution across all
                        // platforms?
                         */
#ifdef MS_OS            /* snprintf seems to be missing. */
                        res = sprintf(temp_buff, 
                                       new_format_str,
                                       precision_arg1,
                                       precision_arg2,
                                       va_arg(ap, unsigned int));
#else
                        res = snprintf(temp_buff, sizeof(temp_buff),
                                       new_format_str,
                                       precision_arg1,
                                       precision_arg2,
                                       va_arg(ap, unsigned int));
#endif
                    }
                    else if (long_flag)
                    {
#ifdef MS_OS            /* snprintf seems to be missing. */
                        res = sprintf(temp_buff, 
                                       new_format_str,
                                       precision_arg1,
                                       precision_arg2,
                                       va_arg(ap, unsigned long));
#else
                        res = snprintf(temp_buff, sizeof(temp_buff),
                                       new_format_str,
                                       precision_arg1,
                                       precision_arg2,
                                       va_arg(ap, unsigned long));
#endif
                    }
                    else
                    {
#ifdef MS_OS            /* snprintf seems to be missing. */
                        res = sprintf(temp_buff, 
                                       new_format_str,
                                       precision_arg1,
                                       precision_arg2,
                                       va_arg(ap, unsigned int));
#else
                        res = snprintf(temp_buff, sizeof(temp_buff),
                                       new_format_str,
                                       precision_arg1,
                                       precision_arg2,
                                       va_arg(ap, unsigned int));
#endif
                    }
                }
                else
                {
                    *buff_pos = '\0';
                    SET_CANT_HAPPEN_BUG();
                    return ERROR;
                }

                if (res == EOF)
                {
                    set_bug("Format error in %q:%S", new_format_str);
                    return ERROR;
                }

                /*
                // FIX
                //
                // Currently, commas will not be inserted into anything other
                // than non-justified positive numbers. Note that commas do not
                // properly work with precision arguments!
                 */
                if ((type_char=='u') && comma_flag && all_digits(temp_buff))
                {
                    char buff_copy[ 100 ];
                    char* buff_copy_pos = buff_copy;
                    int  len = strlen(temp_buff);
                    int first_chunk = (len - 1) % 3 + 1;
                    int i;

                    i = 0;

                    while (i<first_chunk)
                    {
                        *buff_copy_pos = temp_buff[ i ];
                        buff_copy_pos++;
                        i++;
                    }

                    while (i<len)
                    {
                        *buff_copy_pos++ = ',';
                        *buff_copy_pos++ = temp_buff[ i++ ];
                        *buff_copy_pos++ = temp_buff[ i++ ];
                        *buff_copy_pos++ = temp_buff[ i++ ];
                    }

                    *buff_copy_pos = '\0';

                    BUFF_CPY(temp_buff, buff_copy);
                }

                temp_buff_len = strlen(temp_buff);

                if (temp_buff_len > max_len)
                {
                    *buff_pos = '\0';
                    return TRUNCATED;
                }

                strcpy(buff_pos, temp_buff);
                buff_pos += temp_buff_len;
                max_len -= temp_buff_len;
            }
            else if ((type_char == 'e') || (type_char == 'f') ||
                     (type_char == 'g') || (type_char == 'E') ||
                     (type_char == 'G'))
            {
                *new_format_str_pos = type_char;
                new_format_str_pos++;
                *new_format_str_pos = '\0';

                if (num_precision_args == 0)
                {
#ifdef LONG_DOUBLE_OK
                    if (long_double_flag)
                    {
#ifdef MS_OS            /* snprintf seems to be missing. */
                        res = sprintf(temp_buff, 
                                       va_arg(ap, long_double));
#else
                        res = snprintf(temp_buff, sizeof(temp_buff), new_format_str,
                                       va_arg(ap, long_double));
#endif
                    }
                    else
                    {
#ifdef MS_OS            /* snprintf seems to be missing. */
                        res = sprintf(temp_buff, 
                                       va_arg(ap, Always_double));
#else
                        res = snprintf(temp_buff, sizeof(temp_buff), new_format_str,
                                       va_arg(ap, Always_double));
#endif
                    }
#else

#ifdef MS_OS            /* snprintf seems to be missing. */
                    res = sprintf(temp_buff, 
                                   new_format_str,
                                   va_arg(ap, Always_double));
#else
                    res = snprintf(temp_buff, sizeof(temp_buff),
                                   new_format_str,
                                   va_arg(ap, Always_double));
#endif

#endif
                }
                else if (num_precision_args == 1)
                {
                    precision_arg1 = va_arg(ap, int);

#ifdef LONG_DOUBLE_OK
                    if (long_double_flag)
                    {
#ifdef MS_OS            /* snprintf seems to be missing. */
                        res = sprintf(temp_buff, 
                                       new_format_str,
                                       precision_arg1,
                                       va_arg(ap, long_double));
#else
                        res = snprintf(temp_buff, sizeof(temp_buff), 
                                       new_format_str,
                                       precision_arg1,
                                       va_arg(ap, long_double));
#endif
                    }
                    else
                    {
#ifdef MS_OS            /* snprintf seems to be missing. */
                        res = sprintf(temp_buff, 
                                       new_format_str,
                                       precision_arg1,
                                       va_arg(ap, Always_double));
#else
                        res = snprintf(temp_buff, sizeof(temp_buff), 
                                       new_format_str,
                                       precision_arg1,
                                       va_arg(ap, Always_double));
#endif
                    }
#else

#ifdef MS_OS            /* snprintf seems to be missing. */
                    res = sprintf(temp_buff, 
                                   new_format_str, 
                                   precision_arg1,
                                   va_arg(ap, Always_double));
#else
                    res = snprintf(temp_buff, sizeof(temp_buff), 
                                   new_format_str, 
                                   precision_arg1,
                                   va_arg(ap, Always_double));
#endif

#endif
                }
                else if (num_precision_args == 2)
                {
                    precision_arg1 = va_arg(ap, int);
                    precision_arg2 = va_arg(ap, int);

#ifdef LONG_DOUBLE_OK
                    if (long_double_flag)
                    {
#ifdef MS_OS            /* snprintf seems to be missing. */
                        res = sprintf(temp_buff, 
                                       new_format_str,
                                       precision_arg1, precision_arg2,
                                       va_arg(ap, long_double));
#else
                        res = snprintf(temp_buff, sizeof(temp_buff), 
                                       new_format_str,
                                       precision_arg1, precision_arg2,
                                       va_arg(ap, long_double));
#endif
                    }
                    else
                    {
#ifdef MS_OS            /* snprintf seems to be missing. */
                        res = sprintf(temp_buff, 
                                       new_format_str,
                                       precision_arg1,
                                       precision_arg2,
                                       va_arg(ap, Always_double));
#else
                        res = snprintf(temp_buff, sizeof(temp_buff), 
                                       new_format_str,
                                       precision_arg1,
                                       precision_arg2,
                                       va_arg(ap, Always_double));
#endif
                    }
#else

#ifdef MS_OS        /* snprintf seems to be missing. */
                    res = sprintf(temp_buff, 
                                   new_format_str,
                                   precision_arg1,
                                   precision_arg2,
                                   va_arg(ap, Always_double));
#else
                    res = snprintf(temp_buff, sizeof(temp_buff), 
                                   new_format_str,
                                   precision_arg1,
                                   precision_arg2,
                                   va_arg(ap, Always_double));
#endif

#endif
                }
                else
                {
                    *buff_pos = '\0';
                    SET_CANT_HAPPEN_BUG();
                    return ERROR;
                }

                if (res == EOF)
                {
                    set_bug("Format error in %q:%S", new_format_str);
                    return ERROR;
                }

                temp_buff_len = strlen(temp_buff);

                if (temp_buff_len > max_len)
                {
                    *buff_pos = '\0';
                    return TRUNCATED;
                }

                strcpy(buff_pos, temp_buff);
                buff_pos += temp_buff_len;
                max_len -= temp_buff_len;
            }
            else if (type_char == 's')
            {
                if ((justify_flag) || (precision_flag) || (field_width_flag))
                {
                    *new_format_str_pos = type_char;
                    new_format_str_pos++;
                    *new_format_str_pos = '\0';

                    /*
                     *  Overflow is possible: FIX
                     */
                    if (num_precision_args == 0)
                    {
#ifdef MS_OS            /* snprintf seems to be missing. */
                        res = sprintf(temp_buff, 
                                       new_format_str,
                                       va_arg(ap, char*));
#else
                        res = snprintf(temp_buff, sizeof(temp_buff), 
                                       new_format_str,
                                       va_arg(ap, char*));
#endif
                    }
                    else if (num_precision_args == 1)
                    {
                        precision_arg1 = va_arg(ap, int);

#ifdef MS_OS            /* snprintf seems to be missing. */
                        res = sprintf(temp_buff, 
                                       new_format_str,
                                       precision_arg1, va_arg(ap, char*));
#else
                        res = snprintf(temp_buff, sizeof(temp_buff),
                                       new_format_str,
                                       precision_arg1, va_arg(ap, char*));
#endif
                    }
                    else if (num_precision_args == 2)
                    {
                        precision_arg1 = va_arg(ap, int);
                        precision_arg2 = va_arg(ap, int);

#ifdef MS_OS            /* snprintf seems to be missing. */
                        res = sprintf(temp_buff, 
                                       new_format_str,
                                       precision_arg1, precision_arg2,
                                       va_arg(ap, char*));
#else
                        res = snprintf(temp_buff, sizeof(temp_buff),
                                       new_format_str,
                                       precision_arg1, precision_arg2,
                                       va_arg(ap, char*));
#endif
                    }
                    else
                    {
                        *buff_pos = '\0';
                        SET_CANT_HAPPEN_BUG();
                        return ERROR;
                    }

                    if (res == EOF)
                    {
                        set_bug("Format error in %q:%S", new_format_str);
                        return ERROR;
                    }

                    temp_buff_pos = temp_buff;
                }
                else
                {
                    temp_buff_pos = va_arg(ap, char *);
                }

                temp_buff_len = strlen(temp_buff_pos);

                if (temp_buff_len > max_len)
                {
                    *buff_pos = '\0';
                    return TRUNCATED;
                }

                strcpy(buff_pos, temp_buff_pos);
                buff_pos += temp_buff_len;
                max_len -= temp_buff_len;
            }
            else if (type_char == 'S')
            {

#ifdef KJB_HAVE_STRERROR
                const char* sys_mess = strerror(errno);
#else

#ifndef errno  /* GNU defines errno as a macro, leading to warning messages. */
                IMPORT int   errno;
#endif
                IMPORT int   sys_nerr;
                IMPORT SYS_ERRLIST_TYPE sys_errlist[];
#endif

#ifdef KJB_HAVE_STRERROR
                if (sys_mess != NULL)
#else
                    if (    (errno != NO_SYSTEM_ERROR)
                            && (errno < sys_nerr)
                       )
#endif
                    {
                        temp_buff_pos = "\nSystem error message is: ";
                        temp_buff_len = strlen(temp_buff_pos);

                        if (temp_buff_len > max_len)
                        {
                            *buff_pos = '\0';
                            return TRUNCATED;
                        }

                        strcpy(buff_pos, temp_buff_pos);
                        buff_pos += temp_buff_len;
                        max_len -= temp_buff_len;

#ifdef KJB_HAVE_STRERROR
                        temp_buff_pos = sys_mess;
#else
                        temp_buff_pos = sys_errlist[errno];
#endif

                        temp_buff_len = strlen(temp_buff_pos);

                        if (temp_buff_len > max_len)
                        {
                            *buff_pos = '\0';
                            return TRUNCATED;
                        }

                        strcpy(buff_pos, temp_buff_pos);
                        buff_pos += temp_buff_len;
                        max_len -= temp_buff_len;
                    }
            }
            else if ((type_char == 'q') || (type_char == 't'))
            {
                field_width = sizeof(trunc_buff);

                if (num_precision_args == 1)
                {
                    temp_field_width = va_arg(ap, int);

                    if (temp_field_width < field_width)
                    {
                        field_width = temp_field_width;
                    }
                }
                else if (field_width_flag)
                {
                    *new_format_str_pos = '\0';

                    if (ss1pi(new_format_str + 1, &temp_field_width) == ERROR)
                    {
                        SET_FORMAT_STRING_BUG();
                        return ERROR;
                    }

                    if (temp_field_width < field_width)
                    {
                        field_width = temp_field_width;
                    }
                }

                if (type_char == 't')
                {
                    if (field_width < 5)
                    {
                        field_size = 5;
                    }
                    else field_size = field_width;

                    str_trunc_cpy(trunc_buff, va_arg(ap, char *), field_size);
                }
                else
                {
                    if (field_width < 7)
                    {
                        field_size = 7;
                    }
                    else field_size = field_width;

                    trunc_quote_cpy(trunc_buff, va_arg(ap, char *), field_size);
                }

                temp_buff_len = strlen(trunc_buff);

                if (temp_buff_len > max_len)
                {
                    *buff_pos = '\0';
                    return TRUNCATED;
                }

                strcpy(buff_pos, trunc_buff);
                buff_pos += temp_buff_len;
                max_len -= temp_buff_len;
            }
            else if (type_char == 'D')
            {
                temp_buff[ 0 ] = '\0';
                BUFF_GET_USER_FD_NAME(va_arg(ap, int), temp_buff);
                temp_buff_len = strlen(temp_buff);

                if (temp_buff_len > max_len)
                {
                    *buff_pos = '\0';
                    return TRUNCATED;
                }

                strcpy(buff_pos, temp_buff);
                buff_pos += temp_buff_len;
                max_len -= temp_buff_len;
            }
            else if (type_char == 'F')
            {
                temp_buff[ 0 ] = '\0';
                BUFF_GET_USER_FD_NAME(fileno(va_arg(ap, FILE *)), temp_buff);
                temp_buff_len = strlen(temp_buff);

                if (temp_buff_len > max_len)
                {
                    *buff_pos = '\0';
                    return TRUNCATED;
                }

                strcpy(buff_pos, temp_buff);
                buff_pos += temp_buff_len;
                max_len -= temp_buff_len;
            }
            else if (type_char == 'P')
            {
                temp_buff[ 0 ] = '\0';
                BUFF_GET_FD_NAME(fileno(va_arg(ap, FILE *)), temp_buff);
                temp_buff_len = strlen(temp_buff);

                if (temp_buff_len > max_len)
                {
                    *buff_pos = '\0';
                    return TRUNCATED;
                }

                strcpy(buff_pos, temp_buff);
                buff_pos += temp_buff_len;
                max_len -= temp_buff_len;
            }
            else if (type_char == 'R')
            {
                temp_buff[ 0 ] = '\0';

                get_status_string(va_arg(ap, int), temp_buff,
                                  sizeof(temp_buff));

                temp_buff_len = strlen(temp_buff);

                if (temp_buff_len > max_len)
                {
                    *buff_pos = '\0';
                    return TRUNCATED;
                }

                strcpy(buff_pos, temp_buff);
                buff_pos += temp_buff_len;
                max_len -= temp_buff_len;
            }
            else if (type_char == 'c')
            {
                *new_format_str_pos = type_char;
                new_format_str_pos++;
                *new_format_str_pos = '\0';

                if (field_width_arg_flag)
                {
                    precision_arg1 = va_arg(ap, int);

#ifdef MS_OS        /* snprintf seems to be missing. */
                    res = sprintf(temp_buff, new_format_str,
                                  precision_arg1, va_arg(ap, int));
#else
                    res = snprintf(temp_buff, sizeof(temp_buff),
                                   new_format_str,
                                   precision_arg1, va_arg(ap, int));
#endif
                }
                else
                {
#ifdef MS_OS        /* snprintf seems to be missing. */
                    res = sprintf(temp_buff, new_format_str, va_arg(ap, int));
#else
                    res = snprintf(temp_buff, sizeof(temp_buff),
                                   new_format_str,
                                   va_arg(ap, int));
#endif
                }

                if (res == EOF)
                {
                    set_bug("Format error in %q:%S", new_format_str);
                    return ERROR;
                }

                temp_buff_len = strlen(temp_buff);

                if (temp_buff_len > max_len)
                {
                    *buff_pos = '\0';
                    return TRUNCATED;
                }

                strcpy(buff_pos, temp_buff);
                buff_pos += temp_buff_len;
                max_len -= temp_buff_len;
            }
            else if (type_char == 'n')
            {
                *new_format_str_pos = type_char;
                new_format_str_pos++;
                *new_format_str_pos = '\0';

#ifdef MS_OS    /* snprintf seems to be missing. */
                res = sprintf(temp_buff, new_format_str, va_arg(ap, int*));
#else
                res = snprintf(temp_buff, sizeof(temp_buff), 
                               new_format_str, va_arg(ap, int*));
#endif

                if (res == EOF)
                {
                    set_bug("Format error in %q:%S", new_format_str);
                    return ERROR;
                }

                temp_buff_len = strlen(temp_buff);

                if (temp_buff_len > max_len)
                {
                    *buff_pos = '\0';
                    return TRUNCATED;
                }

                strcpy(buff_pos, temp_buff);
                buff_pos += temp_buff_len;
                max_len -= temp_buff_len;
            }
            else if (type_char == 'p')
            {
                *new_format_str_pos = type_char;
                new_format_str_pos++;
                *new_format_str_pos = '\0';

#ifdef MS_OS    /* snprintf seems to be missing. */
                res = sprintf(temp_buff, new_format_str, va_arg(ap, void*));
#else
                res = snprintf(temp_buff, sizeof(temp_buff), 
                               new_format_str, va_arg(ap, void*));
#endif

                if (res == EOF)
                {
                    set_bug("Format error in %q:%S", new_format_str);
                    return ERROR;
                }

                temp_buff_len = strlen(temp_buff);

                if (temp_buff_len > max_len)
                {
                    *buff_pos = '\0';
                    return TRUNCATED;
                }

                strcpy(buff_pos, temp_buff);
                buff_pos += temp_buff_len;
                max_len -= temp_buff_len;
            }
            else if (type_char == '%')
            {
#ifdef HOW_IT_WAS_16_09_30
                *new_format_str_pos = type_char;
                new_format_str_pos++;
                *new_format_str_pos = '\0';


                /* TODO. Replace with string copy to kill warning. */

#ifdef MS_OS    /* snprintf seems to be missing. */
                res = sprintf(temp_buff, new_format_str);
#else
                res = snprintf(temp_buff, sizeof(temp_buff), 
                               new_format_str);
#endif

                if (res == EOF)
                {
                    set_bug("Format error in %q:%S", new_format_str);
                    return ERROR;
                }

                temp_buff_len = strlen(temp_buff);

                if (temp_buff_len > max_len)
                {
                    *buff_pos = '\0';
                    return TRUNCATED;
                }

                strcpy(buff_pos, temp_buff);
                buff_pos += temp_buff_len;
                max_len -= temp_buff_len;
#else
                if (max_len < 1)
                {
                    *buff_pos = '\0';
                    return TRUNCATED;
                }

                *buff_pos = '%';
                buff_pos++;
                max_len--; 
#endif 
            }
            else
            {
                *buff_pos = '\0';
                SET_FORMAT_STRING_BUG();
                return ERROR;
            }
        }
    }

    *buff_pos = '\0';

    return (save_max_len - max_len);
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

#ifdef __cplusplus
}
#endif

