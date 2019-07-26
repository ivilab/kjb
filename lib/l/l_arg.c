
/* $Id: l_arg.c 21712 2017-08-20 18:21:41Z kobus $ */

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
#include "l/l_sys_scan.h"
#include "l/l_sys_term.h"
#include "l/l_sys_io.h"
#include "l/l_sys_str.h"
#include "l/l_arg.h"
#include "l/l_parse.h"


#ifdef __cplusplus
extern "C" {
#endif

/* -------------------------------------------------------------------------- */


/* =============================================================================
 *                             unparse_prog_args
 *
 * Process command-line arguments stored in conventional argc,argv format.
 *
 * Input argc is expected to be positive, and argv is expected not to equal
 * NULL; otherwise, this is treated as a bug.  All strings from argv[1] up to,
 * but not including, argv[argc], will be concatenated with space separators
 * to produce the output string.  We skip argv[0] because conventionally it is
 * just the name of the program being invoked.  The contents of argv are not
 * modified by this function.
 *
 * For example, suppose argc is 3 and argv is the array
 * | argv = [ "it"  "cluster"  "display"  NULL ].
 *
 * Then the output will be a pointer to the string "cluster display "; this
 * string uses heap memory which must be released using kjb_free().
 *
 * Returns:
 *    If successful, this function returns a pointer to a string allocated on
 *    the heap.  The caller is obliged to free this string using kjb_free().
 *    If the allocation fails, the return value is NULL.
 *
 * Author: Kobus Barnard
 *
 * Documenter: Andrew Predoehl
 *
 * Index:  input
 *
 * -----------------------------------------------------------------------------
*/
char* unparse_prog_args(int argc, char** argv)
{
    int    len;
    int    i;
    char** temp_argv;
    char*  result_string;
    char*  result_string_pos;


    if ((argc <= 0) || (argv == NULL))
    {
        SET_ARGUMENT_BUG();
        return NULL;
    }

    len = ROOM_FOR_NULL;

    argc--;
    argv++;
    temp_argv = argv;

    for (i=0; i<argc; i++)
    {
       len += strlen(*temp_argv);
       len += ROOM_FOR_BLANK;
       ++temp_argv;
   }

    /* Always have len>0, due to above "len = ROOM_FOR_NULL;" */
    NRN(result_string = STR_MALLOC(len));

    result_string_pos = result_string;
    *result_string_pos = '\0';

    for (i=0; i<argc; i++)
   {
       str_build(&result_string_pos, *argv);
       str_build(&result_string_pos, " ");

       ++argv;
   }

    return result_string;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

void free_args (int arg_count, char*** args_ptr)
{
    int i;


    if ((arg_count <= 0) || (args_ptr == NULL))
    {
        return;
    }

    for (i=0;i<arg_count; ++i)
    {
        kjb_free((*args_ptr)[i]);
    }

    kjb_free(*args_ptr);

    *args_ptr = NULL;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */


int print_args(FILE* fp, int arg_count, char** args)
{
    int i;


    for (i=0;i<arg_count; ++i)
    {
        ERE(fput_line(fp, args[i]));
    }

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

int get_string_arg(int* argc_ptr, char*** argv_ptr, const char* prompt_str,
                   const char* default_str, char* buff, size_t buff_size)
{


    if (default_str == NULL) default_str = "";

    kjb_strncpy(buff, default_str, buff_size);

    if ((argc_ptr != NULL) && (argv_ptr != NULL) && (*argc_ptr > 0))
    {
        kjb_strncpy(buff, **argv_ptr, buff_size);
        (*argc_ptr)--;
        (*argv_ptr)++;
    }
    else if (prompt_str != NULL)
    {
        char line[ 100 ];
        char prompt[ 100 ];
        char* line_pos;


        ERE(kjb_sprintf(prompt, sizeof(prompt), "Enter %s (%s) ", prompt_str,
                        default_str));

        BUFF_TERM_GET_LINE(prompt, line);

        line_pos = line;
        trim_beg(&line_pos);
        trim_end(line_pos);

        if (*line_pos != '\0')
        {
            kjb_strncpy(buff, line_pos, buff_size);
        }

    }

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

int get_integer_arg(int* argc_ptr, char*** argv_ptr, const char* prompt_str,
                    int default_value, int* int_ptr)
{
    int temp_int = default_value;



    if ((argc_ptr != NULL) && (argv_ptr != NULL) && (*argc_ptr > 0))
    {
        ERE(ss1i(**argv_ptr, &temp_int));

        (*argc_ptr)--;
        (*argv_ptr)++;
    }
    else if (prompt_str != NULL)
    {
        char prompt[ 100 ];
        char line[ 100 ];


        ERE(kjb_sprintf(prompt, sizeof(prompt), "Enter %s (%d) ", prompt_str,
                        default_value));

        /*CONSTCOND*/
        while (TRUE)
        {
            temp_int = default_value;
            BUFF_TERM_GET_LINE(prompt, line);

            if (    (trim_len(line) == 0)
                 || (ss1i(line, &temp_int) != ERROR)
               )
            {
                break;
            }
            else kjb_print_error();
        }

    }

    *int_ptr = temp_int;

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

int get_boolean_arg(int* argc_ptr, char*** argv_ptr, const char* prompt_str,
                    int default_value, int* boolean_ptr)
{
    int temp_boolean = default_value;


    if ((argc_ptr != NULL) && (argv_ptr != NULL) && (*argc_ptr > 0))
    {
        ERE(temp_boolean = get_boolean_value(**argv_ptr));

        (*argc_ptr)--;
        (*argv_ptr)++;
    }
    else if (prompt_str != NULL)
    {
        char prompt[ 100 ];
        char line[ 100 ];


        ERE(kjb_sprintf(prompt, sizeof(prompt), "Use %s ? (%s) ", prompt_str,
                        default_value ? "true": "false" ));

        /*CONSTCOND*/
        while (TRUE)
        {
            temp_boolean = default_value;
            BUFF_TERM_GET_LINE(prompt, line);

            if (    (trim_len(line) == 0)
                 || ((temp_boolean = get_boolean_value(line)) != ERROR)
               )
            {
                break;
            }
            else kjb_print_error();
        }
    }

    *boolean_ptr = temp_boolean;

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                             kjb_getopts
 *
 * Gets the next command option in GNU getopt style
 *
 * This routine is an interface to copy of GNU getopt_long_only. Because we copy
 * the code, the option handling is consistent even on systems that do not have
 * getopt(3), or a different style of it. More about this style can be found
 * from:
 * |     man -S 3 getopt_long_only 
 *
 * Returns:
 *    Each call to kjb_getopts() parses some of argv. When all are dealt with,
 *    then EOF is returned.  On error, this routine returns ERROR, with an error
 *    message being set.  On success it returns the short option as a char, 0
 *    for a long option, and if the options string begins with "-", then
 *    NON_OPTION_ARG for items argv[] that are not options. 
 *
 * Important style note:
 *    For many programs written by the vision lab, we do not use command line
 *    options, at least for options that specify different experimental
 *    parameters. For the later it recommended to implement a "set" command, and
 *    use the input file option handling support. But if you do that, typically
 *    it is then simpler to use "set" for all options. 
 *
 *    However, command line options are often the way to go for simple programs
 *    that do one thing such as filters. 
 *
 * Example:
 *    See kjb_image_program_driver() in i/i_driver.c for an example that uses
 *    many of the option reading features. 
 *
 * Bugs:
 *    It might be nice to modify the copied GNU code so that errors are set,
 *    rather than printed to stderr. 
 * -----------------------------------------------------------------------------
*/

int kjb_getopts
(
    int                  argc,
    char**               argv,
    const char*          opt_str,
    const struct Option* long_options,
    char*                value_buff,
    size_t               value_buff_size
)
{
    IMPORT char* gnu_optarg;
    IMPORT int   gnu_opterr;
    int          option;
    int          dummy_option_index = NOT_SET;


    /*
    // To suppress error messages, set external "opterr" to zero. However, to
    // get the error reporting already implemented, we would have to go into
    // l_getopt.c and change writes to stderr to set_error's. Not difficult, but
    // not really worth the trouble at this point.
    //
    // opterr = 0;
    */

    /* gnu_opterr = 1; */
    gnu_opterr = 1;

    *value_buff = '\0';

    /*
    option = gnu_getopt_long_only(argc, argv, opt_str, long_options,
                                  &dummy_option_index);
    */
    option = gnu_getopt_long_only(argc, argv, opt_str, long_options,
                                  &dummy_option_index);

    if (option == -1)
    {
        return EOF;
    }
    else if (option == '?')
    {
        return ERROR;
    }

    if (gnu_optarg != NULL)
    {
        kjb_strncpy(value_buff, gnu_optarg, value_buff_size);
    }

    return option;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

#ifdef __cplusplus
}
#endif

