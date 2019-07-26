
/* $Id: l_set_aux.c 15483 2013-10-03 00:50:45Z predoehl $ */

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
#include "l/l_set_aux.h"

/* For declared function pointers. Generally harmless. */
#ifdef __cplusplus
extern "C" {
#endif


/* -------------------------------------------------------------------------- */

/*
 * =============================================================================
 *                        process_option_string
 *
 * Processes a string of options
 *
 * This routine parses a string of options and sends them to (*option_fn). This
 * routine is similar to kjb_process_option_string(), but it does not use a
 * default value for option_fn (and thus option_fn cannot be NULL). This
 * routine is used in place of kjb_process_option_string() when additional
 * control over the loading of functions is required; specically when one wants
 * to diable the loading implied by the default option_fn.  Like
 * kjb_process_option_string(), the option function must be able to handle the
 * cases (<option>, <value>), (<option>, ""), (<option>,"?"), and ("", "").
 * Normally, these correspond to the cases of setting an option, requesting the
 * value of an option in a sentence, requesting the value of the option as it
 * would be typed to set it, and requesting the values of all options,
 * respectively.
 *
 * Returns:
 *    On success NO_ERROR is returned and on failure ERROR is returned.
 *
 * Index: options, parsing
 * -----------------------------------------------------------------------------
*/

int process_option_string(
    const char* arg /* String of options as {<opt>=<val>}   */,
    int (*option_fn) (const char *, const char *))
{
    char*  arg_copy;
    char*  arg_pos;
    int    num_options;
    char** option_list;
    char** option_list_pos;
    char** value_list;
    char** value_list_pos;
    int    i;
    int    result          = NO_ERROR;


    NRE(arg_copy = kjb_strdup(arg));

    arg_pos = arg_copy;

    trim_beg(&arg_pos);

    if (*arg_pos == '\0')
    {
        result = (*option_fn)("", "");

        if (result != ERROR)
        {
            result = pso("\n");
        }

        kjb_free(arg_copy);

        return result;
    }

    num_options = parse_options(arg_pos, &option_list, &value_list);

    if (num_options == ERROR)
    {
        result = ERROR;
    }
    else
    {
        char option_without_value[ 100 ];
        int  print_request = FALSE;

        while (    (result != ERROR)
                && (BUFF_GET_TOKEN_OK(&arg_pos, option_without_value))
              )
        {
            if ( !print_request)
            {
                print_request = TRUE;
                ERE(pso("\n"));
            }

            result = (*option_fn)(option_without_value, "");
        }

        if (result != ERROR)
        {
            option_list_pos = option_list;
            value_list_pos = value_list;

            for (i = 0; i<num_options; i++)
            {
                result = (*option_fn)(*option_list_pos, *value_list_pos);

                if (result == ERROR) break;

                value_list_pos++;
                option_list_pos++;
            }
        }

        if ((result != ERROR) && (print_request))
        {
            ERE(pso("\n"));
        }
    }

    kjb_free(arg_copy);
    free_options(num_options, option_list, value_list);

    return result;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

int call_set_fn(int num_set_fn,
                int (**set_fn_list) (const char *, const char *),
                const char* title,
                const char* option,
                const char* value)
{
    int i;
    int temp_result;
    int result = NOT_FOUND;


    if (is_pattern(option) && ((value[ 0 ] == '\0') || (value[ 0 ] == '?')))
    {
        /* Dry run */

        for (i=0; i<num_set_fn; i++)
        {
            ERE(temp_result = (*set_fn_list[ i ])(option, (char*)NULL));

            if (temp_result == NO_ERROR)
            {
                result = NO_ERROR;
                break;
            }
        }

        if (result == NOT_FOUND) return result;

        if (title != NULL)
        {
            ERE(pso("\n"));
            ERE(set_high_light(stdout));
            ERE(pso("     "));
            ERE(pso(title));
            ERE(unset_high_light(stdout));
            ERE(pso("\n\n"));
        }
    }

    for (i=0; i<num_set_fn; i++)
    {
        ERE(temp_result = (*set_fn_list[ i ])(option, value));

        if (temp_result == NO_ERROR) result = NO_ERROR;

        if (    (temp_result == NO_ERROR)
             && (is_pattern(option))
             && ((value[ 0 ] == '\0') || (value[ 0 ] == '?'))
             && (i < num_set_fn - 1)
           )
        {
            ERE(pso("\n"));
        }

    }

    return result;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

int parse_method_option(Method_option *methods,
                        int num_methods,
                        const char* method_option_str,
                        const char* method_message_str,
                        const char* value,
                        int* method_ptr)
{

    if (value[ 0 ] == '\0')
    {
        int method = *method_ptr;

        if (method == NOT_SET)
        {
            ERE(pso("No %s is set.\n", method_message_str));
        }
        else if ((method >= 0) && (method <num_methods))
        {
            ERE(pso("%s is %s.\n", method_message_str,
                    methods[ method ].long_name));
        }
        else
        {
            SET_ARGUMENT_BUG();
            return ERROR;
        }
    }
    else if (value[ 0 ] == '?')
    {
        int method = *method_ptr;

        if (method == NOT_SET)
        {
            ERE(pso("%s = off\n", method_option_str));
        }
        else if ((method >= 0) && (method <num_methods))
        {
            ERE(pso("%s = %s\n", method_option_str,
                    methods[ method ].long_name));
        }
        else
        {
            SET_ARGUMENT_BUG();
            return ERROR;
        }
    }
    else if (is_no_value_word(value))
    {
        *method_ptr = NOT_SET;
    }
    else
    {
        int i;
        char lc_value[ 100 ];
        int found = FALSE;

        EXTENDED_LC_BUFF_CPY(lc_value, value);

        for (i=0; i<num_methods; i++)
        {
            if (    (STRCMP_EQ(lc_value, methods[ i ].long_name))
                 || (STRCMP_EQ(lc_value, methods[ i ].short_name))
               )
            {
                *method_ptr = i;
                found = TRUE;
                break;
            }
        }

        if (!found)
        {
            set_error("%q is an invalid %s method.", value,
                      method_message_str);
            return ERROR;
        }
    }

    return NO_ERROR;
}

/* -------------------------------------------------------------------------- */

#ifdef __cplusplus
}
#endif



