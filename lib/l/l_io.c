
/* $Id: l_io.c 21520 2017-07-22 15:09:04Z kobus $ */

/* =========================================================================== *
|
|  Copyright (c) 1994-2008 by Kobus Barnard (author). (Lindsay Martin
|  contributed to the documentation of this code).
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

#include "l/l_config.h"
#include "l/l_help.h"
#include "l/l_parse.h"
#include "l/l_queue.h"
#include "l/l_string.h"
#include "l/l_sys_term.h"
#include "l/l_sys_tsig.h"
#include "l/l_sys_scan.h"
#include "l/l_sys_sig.h"

#include "l/l_io.h"

#ifdef __cplusplus
extern "C" {
#endif

/* -------------------------------------------------------------------------- */

#ifndef MAX_PROGRAM_PROMPT_SIZE
#    define MAX_PROGRAM_PROMPT_SIZE  32
#endif

#define SINGLE_CHARACTER_BUFF_SIZE            (1 + ROOM_FOR_NULL)
#define FORMAT_BUFF_SIZE                      LARGE_IO_BUFF_SIZE
#define MAX_NUM_NESTED_READS                  20

/* -------------------------------------------------------------------------- */

typedef enum Pre_process_result
{
    PRE_PROCESS_INPUT,
    PRE_PROCESS_SUCCESS,
    PRE_PROCESS_NOT_APPLICABLE,
    PRE_PROCESS_ERROR,
    PRE_PROCESS_EOF
}
Pre_process_result;

/* -------------------------------------------------------------------------- */

static void set_default_program_prompt(const char* program_name);

static int affirmative(const char* input);

static int set_alternate_input(const char* file_name);

static void set_read_from_init_file(const char* program_name);

static long get_input_line(char* line, size_t max_len);

static long get_input_line_command(char* command, size_t max_len);

static Pre_process_result preprocess_command
(
    const char* program_name,
    char*       command,
    size_t      max_len
);

static int initialize_input_line_buffer(void);

#ifdef TRACK_MEMORY_ALLOCATION
    static void free_input_line_buff(void);
    static void free_alternate_input_file_queue(void);
    static void free_no_overwrite_stack(void);


#endif

static TRAP_FN_RETURN_TYPE alternate_file_read_atn_fn(TRAP_FN_ARGS);

/* -------------------------------------------------------------------------- */

static int            fs_use_default_program_prompt = TRUE;
static char           fs_kjb_program_prompt[ MAX_PROGRAM_PROMPT_SIZE ] = "";
static Queue_element* fs_alternate_input_left_over_stack     = NULL;
static Queue_element* fs_alternate_input_fp_stack     = NULL;
static int            fs_alternate_input_fp_stack_len = 0;
static size_t         fs_input_line_buff_size         = GET_LINE_BUFF_SIZE;
static char*          fs_loop_line_buff               = NULL;
static char*          fs_input_line_buff              = NULL;
static char*          fs_input_line_buff_pos          = NULL;
static char*          fs_cached_pp_command            = NULL;
static int            fs_echo_alternate_input         = FALSE;
static volatile int   fs_read_atn_flag = FALSE;
static int            fs_loop_count = 0;
static Queue_element* fs_loop_count_stack = NULL;
static Queue_element* fs_loop_line_buff_stack = NULL;
static int            fs_no_overwrite = FALSE;
static Queue_element* fs_no_overwrite_stack_head = NULL;

/* -------------------------------------------------------------------------- */

int set_io_options(const char* option, const char* value)
{
    char lc_option[ 100 ];
    int  result = NOT_FOUND;
    int  temp_boolean_value;


    EXTENDED_LC_BUFF_CPY(lc_option, option);

    if (    (lc_option[ 0 ] == '\0')
         || (match_pattern(lc_option, "echo-alternate-input"))
         || (match_pattern(lc_option, "echo-alternative-input"))
       )
    {
        if (value == NULL)
        {
            return NO_ERROR;
        }
        else if (value[ 0 ] == '?')
        {
            ERE(pso("echo-alternate-input = %s\n",
                    fs_echo_alternate_input ? "t" : "f"));
        }
        else if (value[ 0 ] == '\0')
        {
            ERE(pso("Alternative input %s echoed.\n",
                    fs_echo_alternate_input ? "is" : "is not" ));
        }
        else
        {
            ERE(temp_boolean_value = get_boolean_value(value));
            fs_echo_alternate_input = temp_boolean_value;
        }
        result = NO_ERROR;
    }

    if (    (lc_option[ 0 ] == '\0')
         || match_pattern(lc_option, "comment-char")
         || match_pattern(lc_option, "comment-character")
       )
    {
        IMPORT int kjb_comment_char;

        if (value == NULL)
        {
            return NO_ERROR;
        }
        else if (value[ 0 ] == '?')
        {
            ERE(pso("comment-char = '%c'\n", kjb_comment_char));
        }
        else if (value[ 0 ] == '\0')
        {
            ERE(pso("Comment character is '%c'.\n", kjb_comment_char));
        }
        else
        {
            if (strlen(value) != 1)
            {
                set_error("Expecting a single character for comment char.");
                return ERROR;
            }
            kjb_comment_char = *value;
        }
        result = NO_ERROR;
    }

    if (    (lc_option[ 0 ] == '\0')
         || match_pattern(lc_option, "header-char")
         || match_pattern(lc_option, "header-character")
       )
    {
        IMPORT int kjb_header_char;

        if (value == NULL)
        {
            return NO_ERROR;
        }
        else if (value[ 0 ] == '?')
        {
            ERE(pso("header-char = '%c'\n", kjb_header_char));
        }
        else if (value[ 0 ] == '\0')
        {
            ERE(pso("Header character is' %c'.\n", kjb_header_char));
        }
        else
        {
            if (strlen(value) != 1)
            {
                set_error("Expecting a single character for header char.");
                return ERROR;
            }
            kjb_header_char = *value;
        }
        result = NO_ERROR;
    }


    return result;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*
 * =============================================================================
 *                                set_program_prompt
 *
 * Routine to overide the default prompt
 *
 * This routine overides the default program prompt used by the high level IO
 * routine get_command_text(). For most applications, the default is fine, and
 * this routine is not needed.
 *
 * Related:
 *    get_command_text()
 *
 * Index:
 *    I/O
 *
 * -----------------------------------------------------------------------------
 */

void set_program_prompt(const char* prompt)
{


    BUFF_CPY(fs_kjb_program_prompt, prompt);
    fs_use_default_program_prompt = FALSE;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 * STATIC                      set_default_program_prompt
 * -----------------------------------------------------------------------------
*/

static void set_default_program_prompt(const char* program_name)
{


    if (fs_use_default_program_prompt)
    {
        BUFF_CPY(fs_kjb_program_prompt, program_name);
        BUFF_CAT(fs_kjb_program_prompt, "> ");
    }
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

void prompt_to_continue(void)
{
    char buff[ 2 ];


    if (is_interactive())
    {
        kjb_query("Enter any key to continue ... ", buff, sizeof(buff));
    }
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                               kjb_query
 *
 * Prompt the user for terminal input and returns the result.
 *
 * Writes the character string "prompt" on the terminal, and reads up to
 * input_size minus 1 characters from the terminal into "input". A NULL is
 * added to the end of input. The routine returns when a carriage return is
 * entered, or as soon as input_size minus 1 characters are read. The routine
 * also returns when the end-of-file character (often CTL-D) is entered.   If
 * input_size is 2, then the routine returns as soon as the user depresses a
 * key. If input_size is less then 2, then the return is immediate.
 *
 * Note:
 *    This routine reads from the terminal, regardless of whether or not stdin
 *    is a terminal.
 *
 * Note:
 *    It is programming error to make input_size greater than INT_MAX.
 *
 * Warning:
 *    Some keys generated escape sequences which are more than one character.
 *    Since this routine will only read up the input_size minus 1 character,
 *    such characters can be missed. Thus this routine is not a good choice for
 *    general purpose "cbreak" input.
 *
 * Returns:
 *    On error, kjb_query returns ERROR. Possible problems include failure to
 *    obtain a terminal device. On success, the number of characters read is
 *    returned.
 *
 * See also;
 *    term_get_line, term_getc, enhanced_term_getc, yes_no_query
 *
 * Index: I/O
 *
 * -----------------------------------------------------------------------------
 */

long kjb_query(const char* prompt  /* Prompt for the query      */,
                      char* input         /* A buffer for the reply.   */,
                      size_t input_size   /* Max number of characters to
                                             read including NULL*/        )
{



    return term_get_n_chars(prompt, input, input_size);
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

long overwrite_query(const char* prompt, char* input, size_t input_size)
{



    return overwrite_term_get_n_chars(prompt, input, input_size);
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

int yes_no_query(const char* prompt)
{
    char        input[ SINGLE_CHARACTER_BUFF_SIZE ];
    int         result;
    long read_result;
    int         yes_no_result;
    int         good_input;


    good_input = FALSE;

    read_result = overwrite_query(prompt, input, SINGLE_CHARACTER_BUFF_SIZE);

    /*
    // yes_no_result should only be set only when good_input is set.
    // However, in order to shut up some compiler's warning messages,
    // (and to defend against the impossible, we set it here.
    */
    yes_no_result = NOT_SET;

    while( ! good_input )
    {
        if (read_result == ERROR)
        {
            good_input = TRUE;
            yes_no_result = FALSE;
        }
        else if (read_result == EOF)
        {
            /*EMPTY*/
            ; /* Do nothing. */
        }
        else if (read_result == WOULD_BLOCK)
        {
            nap(100);
        }
        else if (read_result == INTERRUPTED)
        {
            /*EMPTY*/
            ; /* Do nothing. */
        }
        else if ((read_result != 0) && (read_result != 1))
        {
            set_bug("Unexpected read result (%R) in yes_no_query.",
                    read_result);
            good_input = TRUE; /* Not really, but we should get out of here! */
            yes_no_result = FALSE;
        }
        else
        {
            result =  affirmative(input);

            /*
             *  It user answered 'q', then the result is EOF. 
            */

            if (result == TRUE)
            {
                good_input = TRUE;
                yes_no_result = TRUE;
            }
            else if (result == FALSE)
            {
                good_input = TRUE;
                yes_no_result = FALSE;
            }
            else if (result == EOF)
            {
                read_result = overwrite_query("Confirm program exit (y/n/q)",
                                              input,
                                              SINGLE_CHARACTER_BUFF_SIZE);

                if ((read_result == 1) && (affirmative(input)))
#if 0 /* was ifdef HOW_IT_WAS_AUG_13 */
                /* What the hell--do some recursion. */
                if (yes_no_query("Confirm program exit (y/n)"))
#endif
                {
                    kjb_exit_2(EXIT_FAILURE);
                    /* NOTREACHED */
                }
            }
        }

        if ( !good_input )
        {
            term_beep();

#if 0 /* was ifdef HOW_IT_WAS */
            read_result = overwrite_query("Enter either y (yes) or n (no) ",
                                          input, SINGLE_CHARACTER_BUFF_SIZE);
#else
            read_result = overwrite_query(prompt, input,
                                          SINGLE_CHARACTER_BUFF_SIZE);
#endif

        }
    }

    if (yes_no_result == NOT_SET)
    {
        SET_CANT_HAPPEN_BUG();
        return FALSE;
    }

    return yes_no_result;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

int confirm_risky_action(const char* prompt)
{
     char      input[ 200 ];
     int       result;
     long read_result;
     int       yes_no_result;
     int       good_input;


     good_input = FALSE;

     /*
     // set_atn_trap(yes_no_query_atn_fn, DONT_RESTART_AFTER_SIGNAL);
     */

     read_result = BUFF_TERM_GET_LINE(prompt, input);

     /*
     // yes_no_result should only be set only when good_input is set.
     // However, in order to shut up some compiler's warning messages,
     // (and to defend against the impossible, we set it here.
     */
     yes_no_result = NOT_SET;

     while( ! good_input )
     {
         if (read_result == ERROR)
         {
              good_input = TRUE;
              yes_no_result = FALSE;
          }
         else if (read_result == EOF)
         {
             /*EMPTY*/
             ; /* Do nothing. */
         }
         else if (read_result == WOULD_BLOCK)
         {
             nap(100);
         }
         else if (read_result == INTERRUPTED)
         {
             /*EMPTY*/
             ; /* Do nothing. */
         }
         else if (read_result < 0)
         {
             set_bug("Unexpected read result (%R) in yes_no_query.",
                     read_result);
             good_input = TRUE; /* Not really, but we should get out of here! */
             yes_no_result = FALSE;
         }
         else
         {
             result =  affirmative(input);

             if (result == TRUE)
             {
                 good_input = TRUE;
                 yes_no_result = TRUE;
             }
             else if (result == FALSE)
             {
                 good_input = TRUE;
                 yes_no_result = FALSE;
             }
         }

         if ( !good_input )
         {
             read_result =
                 BUFF_TERM_GET_LINE("Enter either y (yes) or n (no) ", input);
         }
     }

     if (yes_no_result == NOT_SET)
     {
         kjb_abort();
     }

     /*
     // unset_atn_trap();
     */

     return yes_no_result;
 }

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

static int affirmative(const char* input)
{
    char input_cpy[ 100 ];
    char* input_pos;


    BUFF_CPY(input_cpy, input);
    input_pos = input_cpy;

    trim_beg(&input_pos);
    trim_end(input_pos);
    extended_uc_lc(input_pos);

    if (STRCMP_EQ(input_pos, "y"))    return TRUE;
    if (STRCMP_EQ(input_pos, "yes"))  return TRUE;
    if (STRCMP_EQ(input_pos, "ok"))   return TRUE;

    if (STRCMP_EQ(input_pos, "n"))    return FALSE;
    if (STRCMP_EQ(input_pos, "no"))   return FALSE;

    if (STRCMP_EQ(input_pos, "q"))    return EOF;
    if (STRCMP_EQ(input_pos, "quit")) return EOF;

    set_error("Invalid response to query. Try \"y\", \"n\" or \"q\".");
    return ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

long multi_fget_line(FILE*** fp_ptr_ptr, char* line, size_t max_len)
{
    long fget_res;


    while (**fp_ptr_ptr != NULL)
    {
        fget_res = fget_line(**fp_ptr_ptr, line, max_len);

        if (fget_res == EOF) (*fp_ptr_ptr)++;
        else return fget_res;
    }

    return EOF;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

int multi_fclose(FILE** fp_ptr)
{
    int result = NO_ERROR;
    Error_action save_error_action;


    /* Changed the error handling, but did not test it. */
    UNTESTED_CODE();

    save_error_action = get_error_action();

    while (*fp_ptr != NULL)
    {
        if (kjb_fclose(*fp_ptr) == ERROR)
        {
            /* From now on, we will add additional errors. */
            set_error_action(FORCE_ADD_ERROR_ON_ERROR);
            result = ERROR;
        }

        fp_ptr++;
    }

    set_error_action(save_error_action);

    return result;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*
 * =============================================================================
 *                                print_blanks
 *
 * Prints a number of blanks
 *
 * This routine prints n blanks on fp.
 *
 * Related:
 *   rep_print()
 *
 * -----------------------------------------------------------------------------
 */

int print_blanks(FILE* fp, int num_blanks)
{
    /*   blanks must be at least  40  blanks  */
    const char* blanks = "                                        ";
    /*                    1234567890123456789012345678901234567890   */

    if (num_blanks < 0) 
    {
        num_blanks = 0;
    }

    while (num_blanks > 40)
    {
        ERE(kjb_fwrite(fp, blanks, (size_t)40));
        num_blanks -= 40;
    }

    ERE(kjb_fwrite(fp, blanks, (size_t)num_blanks));

    return num_blanks;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*
 * =============================================================================
 *                                rep_print
 *
 * Prints a character a number of times
 *
 * This routine prints the character c n times on fp.
 *
 * Related:
 *    print_blanks()
 *
 * -----------------------------------------------------------------------------
 */

int rep_print(FILE* fp, int c, int n)
{
    IMPORT volatile Bool halt_all_output;
    IMPORT volatile Bool halt_term_output;


    if (halt_all_output || (halt_term_output && kjb_isatty(fileno(fp))))
    {
        return NO_ERROR;
    }

    while (n > 0)
    {
        ERE(kjb_fputc(fp, c));
        n--;
    }

    return n;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*
 * =============================================================================
 *                                count_real_lines
 *
 * Counts the number of lines that get_real_line() would read.
 *
 * This routine counts the number of lines that get_real_line() would read up to
 * EOF, or up to the next soft EOF.
 *
 * Author:
 *     Kobus Barnard
 *
 * Author:
 *     Lindsay Martin
 *
 * Index: I/O
 *
 * -----------------------------------------------------------------------------
*/

/*
// In spite of immediate appearances, this routine does not rely too much on the
// size of the line buffer because fget_line swallows the overflow. (It is
// possible for a line which starts of with more than LARGE_IO_BUFF_SIZE blanks
// to be misdiagnosed as a non-double line, but we can live with this.
*/

long count_real_lines(FILE* fp)
{
    char line[ LARGE_IO_BUFF_SIZE ];
    long ftell_res;
    int  get_res;
    long i;


    ftell_res = kjb_ftell(fp);

    if (ftell_res == ERROR)
    {
        insert_error("Ftell on %F failed in count_real_lines.", fp);
        return ERROR;
    }

    i=0;

    while ((get_res = BUFF_GET_REAL_LINE(fp, line)) != EOF)
    {
        if (get_res == TRUNCATED)
        {
            set_error("%F has a line exceeding %ld bytes.",
                      fp, sizeof(line));
            return ERROR;
        }
        else if (get_res < 0)
        {
            set_error("Unexpected return (%d) reading %F.",
                      get_res, fp);
            return ERROR;
        }

        i++;
    }

    ERE(kjb_fseek(fp, ftell_res, SEEK_SET));


    return i;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                count_data_lines_until_next_header
 *
 * Counts the number of data lines until the next data block header.
 *
 * Counts data lines from the current file position until the next data block
 * header line is encountered. Data lines are those that DO NOT begin with the
 * comment character which is a user settable option (default is '#'. Header
 * lines are lines that start with the comment character, and are immediately
 * followed by the header character which is also a user setable option (default
 * is '!').
 *
 * For example:
 *|  If comment_char = '#' and header_char = '!'
 *|  then:
 *|    2345.0               is a data line         (count is incremented)
 *|    # I am hungry        is a comment line      (ignored)
 *|                         is a blank line        (ignored)
 *|    #! rows=50 cols=3    is a data block header (function returns)
 *
 *
 * This function resets the file position to the position where the header
 * search started.
 *
 * Returns:
 *     The number of data lines until the next data clock header on success, or
 *     ERROR on failure, with "kjb_error" set to a descriptive message.
 *
 * Related:
 *     count_real_lines
 *
 * Authors:
 *     Kobus Barnard, Lindsay Martin
 *
 * Documentors:
 *     Lindsay Martin
 *
 * Index: I/O
 *
 * -----------------------------------------------------------------------------
*/

long count_data_lines_until_next_header(FILE* fp)
{
    IMPORT int kjb_comment_char;
    IMPORT int kjb_header_char;
    char  line[ LARGE_IO_BUFF_SIZE ];
    int   get_res;
    char*  line_pos;
    long ftell_res;
    long i;


    ftell_res = kjb_ftell(fp);

    if (ftell_res == ERROR)
    {
        insert_error("Ftell on %F failed in count_data_lines_until_next_header.", fp);
        return ERROR;
    }

    i=0;

    while ((get_res=BUFF_FGET_LINE(fp, line)) != EOF)
    {
        ERE(get_res);
        line_pos = line;
        trim_beg(&line_pos);

        if (*line_pos == '\0')
        {
            /*EMPTY*/
            ; /* Do nothing */
        }
        else if (*line_pos == kjb_comment_char)
        {
            line_pos++;
            trim_beg(&line_pos);

            if (*line_pos == kjb_header_char)
            {
                /* We've found a header line (or a soft EOF). Bail out of loop
                // and return */
                break;
            }
        }
        else
        {
            i++;
        }
    }

    ERE(kjb_fseek(fp, ftell_res, SEEK_SET));

    return i;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

int read_header_options(FILE* fp, char*** option_list_ptr, char*** value_list_ptr)
{
    IMPORT int kjb_comment_char;
    IMPORT int kjb_header_char;
    char line[ 1000 ];  /* Small buffer is OK, because truncation is OK. */
    int  read_res;

    /*CONSTCOND*/
    while (TRUE)
    {
        ERE(read_res = BUFF_FGET_LINE(fp, line));

        if (read_res == EOF)
        {
            return EOF;
        }
        else if (    (line[ 0 ] == kjb_comment_char)
                  && (line[ 1 ] == kjb_header_char)
                )
        {
            return parse_options(line + 2, option_list_ptr, value_list_ptr);
        }
        else if (    (line[ 0 ] != kjb_comment_char)
                  && ( ! all_blanks(line))
                )
        {
            /*
            // If it is not blank, and not a comment, then it must be data, and
            // we are thus past the header.
            */
            return EOF;
        }
    }

    /*NOTREACHED*/
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *
 *
 *
 *
 * -----------------------------------------------------------------------------
*/

long count_tokens_in_file(FILE* fp)
{
    char  line       [ LARGE_IO_BUFF_SIZE ];
    char  token_buff [ 200 ];
    long i;
    int   get_res;
    char*  line_pos;
    long ftell_res;


    /* FIX: Check if fp is a file, and set program error if it is NOT. */

    ftell_res = kjb_ftell(fp);

    if (ftell_res == ERROR)
    {
        insert_error("Ftell on %F failed in count_tokens_in_file.", fp);
        return ERROR;
    }

    i=0;

    while ((get_res=BUFF_GET_REAL_LINE(fp, line)) != EOF)
    {
        ERE(get_res);

        line_pos = line;

        while (BUFF_GET_TOKEN_OK(&line_pos, token_buff))
        {
            i++;
        }
    }

    ERE(kjb_fseek(fp, ftell_res, SEEK_SET));

    return i;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

long gen_count_tokens_in_file(FILE* fp, const char* terminators)
{
    char  line       [ LARGE_IO_BUFF_SIZE ];
    char  token_buff [ 200 ];
    long  i;
    int   get_res;
    char* line_pos;
    long  ftell_res;


    /* FIX: Check if fp is a file, and set program error if it is NOT. */

    ftell_res = kjb_ftell(fp);

    if (ftell_res == ERROR)
    {
        insert_error("Ftell on %F failed in gen_count_tokens_in_file.", fp);
        return ERROR;
    }

    i=0;

    while ((get_res = BUFF_GET_REAL_LINE(fp, line)) != EOF)
    {
        ERE(get_res);

        line_pos = line;

        while (BUFF_GEN_GET_TOKEN_OK(&line_pos, token_buff, terminators))
        {
            i++;
        }
    }

    ERE(kjb_fseek(fp, ftell_res, SEEK_SET));

    return i;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*
 * =============================================================================
 *                                get_real_line
 *
 * Reads a line from a stream ignoring blanks and comments
 *
 * This routine is similar to fget_line() except that blank lines and
 * comment lines are ignored. Comment lines are identified by having the
 * kjb_comment_char in the first column. Normally kjb_comment_char is '#';
 * however this is normally exposed to the user as option.
 *
 * The routine also looks for the comment character in the first column,
 * followed by !eof (e.g., #!eof), which acts like a soft end of file. In this
 * case the routine returns EOF, but a subsequent call will return the next
 * line. This is useful for laying out multiple data structures in a file,
 * separating them by the soft end of file. Then routines for reading the
 * sub-structures (e.g.  read_matrix()) can be used in sequence for reading the
 * sub-data structures.
 *
 * Macros:
 *    BUFF_GET_REAL_LINE(FILE fp, char line[])
 *
 *    The max_len parameter is set to sizeof(line) with the appropriate cast.
 *    Using sizeof to set the buffer size is recomended where applicable, as
 *    the code will not be broken if the buffer size changes. HOWEVER, neither
 *    this method, nor the macro, is  applicable if line is NOT a character
 *    array.  If line is declared by "char* line", then the size of line is the
 *    number of bytes in a character pointer (usually 4), which is NOT what is
 *    normally intended.  You have been WARNED!
 *
 * Returns:
 *    On success get_real_line() returns the number of bytes placed in the
 *    buffer, excluding the NULL.  Alternate return values are all negative. In
 *    the case of end of file or soft end of file, EOF returned. If the line was
 *    truncated, then TRUNCATED is returned. Depending on the signal traps and
 *    options in place, INTERRUPTED is an additional possible return value. In
 *    the case of non-blocking reads WOULD_BLOCK is returned unless a complete
 *    line can be returned. ERROR is returned for unexpected problems and an
 *    error message is set.
 *
 * Index: input, I/O
 *
 * -----------------------------------------------------------------------------
*/

long get_real_line(FILE* fp, char* line, size_t max_len)
{
    IMPORT int kjb_comment_char;
    long get_res;
    char*       line_pos;


    if (max_len <= 1)
    {
        SET_ARGUMENT_BUG();
        return ERROR;
    }

    *line = '\0';

    while ((get_res = fget_line(fp, line, max_len)) != EOF)
    {
        if (get_res < 0)
        {
            return get_res;
        }

        line_pos = line;
        trim_beg(&line_pos);

        if (*line_pos == '\0')
        {
            /*EMPTY*/
            ; /* Do nothing */
        }
        else if (line_pos[ 0 ] == kjb_comment_char)
        {
            if (line_pos[ 1 ] == '!')
            {
                line_pos += 2;

                trim_beg(&line_pos);
                trim_end(line_pos);
                extended_uc_lc(line_pos);

                if (STRCMP_EQ(line_pos, "eof"))
                {
                    return EOF;
                }
            }
        }
        else
        {
            return get_res;
        }
    }
    return get_res;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*
 * =============================================================================
 *                                string_count_real_lines
 *
 * Counts the number of lines that string_get_real_line() would parse.
 *
 * This routine counts the number of lines that string_get_real_line() would
 * parse up to the end of the string.
 *
 * Author:
 *     Kobus Barnard
 *
 * Author:
 *     Lindsay Martin
 *
 * Returns:
 *    If there are no more characters to be read, then EOF is returned. If the
 *    buffer is not large enough, SET_BUFFER_OVERFLOW_BUG() is called, and
 *    TRUNCATED is returned. Otherwise the number of "real" lines is returned.
 *
 * Index: I/O
 *
 * -----------------------------------------------------------------------------
*/

long string_count_real_lines(const char* buff)
{
    IMPORT int kjb_comment_char;
    char       line[ LARGE_IO_BUFF_SIZE ];
    const char*      buff_pos;
    long       i;
    int        get_res;
    char*      line_pos;


    i=0;

    buff_pos = buff;

    while ((get_res = BUFF_CONST_SGET_LINE(&buff_pos, line)) != EOF)
    {
        ERE(get_res);
        line_pos = line;
        trim_beg(&line_pos);

        if (*line_pos == '\0')
        {
            /*EMPTY*/
            ; /* Do nothing */
        }
        else if (*line_pos == kjb_comment_char)
        {
            /*EMPTY*/
            ; /* Do nothing */
        }
        else
        {
            i++;
        }
    }

    return i;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*
 * =============================================================================
 *                                string_get_real_line
 *
 * Reads a line from a string ignoring blanks and comments
 *
 * This routine is analogous to get_real_line() except that the "stream" is
 * string in memory that is cut up into lines on successive calls until a null
 * terminator is reached. Unlike get_real_line(), there no concept of soft EOF.
 * More specifically, the line is extracted from the string
 * starting at the postion *buff_ptr which is set to the updated location beyond
 * what has been parsed.
 *
 * Macros:
 *    BUFF_STRING_GET_REAL_LINE(const char**, char line[])
 *
 *    The max_len parameter is set to sizeof(line) with the appropriate cast.
 *    Using sizeof to set the buffer size is recomended where applicable, as
 *    the code will not be broken if the buffer size changes. HOWEVER, neither
 *    this method, nor the macro, is  applicable if line is NOT a character
 *    array.  If line is declared by "char* line", then the size of line is the
 *    number of bytes in a character pointer (usually 4), which is NOT what is
 *    normally intended.  You have been WARNED!
 *
 * Returns:
 *    If there are no more characters to be read, then EOF is returned. If the
 *    buffer is not large enough, SET_BUFFER_OVERFLOW_BUG() is called, and
 *    TRUNCATED is returned. Otherwise NO_ERROR is returned.
 *
 * Index: input, I/O
 *
 * -----------------------------------------------------------------------------
*/

long string_get_real_line
(
    const char** buff_ptr,
    char*        line,
    size_t       max_len
)
{
    IMPORT int kjb_comment_char;
    long get_res;
    char*       line_pos;


    while ((get_res = const_sget_line(buff_ptr, line, max_len)) != EOF)
    {
        ERE(get_res);

        line_pos = line;
        trim_beg(&line_pos);

        if (*line_pos == '\0')
        {
            /*EMPTY*/
            ; /* Do nothing */
        }
        else if (*line_pos == kjb_comment_char)
        {
            /*EMPTY*/
            ; /* Do nothing */
        }
        else
        {
            return get_res;
        }
    }
    return get_res;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

static int set_alternate_input(const char* file_name)
{
#ifdef TRACK_MEMORY_ALLOCATION
    static int first_time         = TRUE;
#endif
    FILE*      alternate_input_fp;
    int        res;
    int*       loop_count_ptr;


#ifdef TRACK_MEMORY_ALLOCATION
    if (first_time)
    {
        add_cleanup_function(free_alternate_input_file_queue);
        first_time = FALSE;
    }
#endif

    ERE(initialize_input_line_buffer());

    if (fs_alternate_input_fp_stack_len >= MAX_NUM_NESTED_READS)
    {
        set_error("Nesting of reads is limited to %d.", MAX_NUM_NESTED_READS);
        add_error("Perhaps there is a loop of reads?");
        return ERROR;
    }

    NRE(alternate_input_fp=kjb_fopen(file_name, "r"));

    ERE(set_atn_trap(alternate_file_read_atn_fn, DONT_RESTART_AFTER_SIGNAL));

    res = insert_into_queue(&fs_alternate_input_fp_stack,
                            (Queue_element**)NULL,
                            (void*)alternate_input_fp);

    if (res == ERROR)
    {
        Error_action save_error_action;

        save_error_action = get_error_action();
        set_error_action(FORCE_ADD_ERROR_ON_ERROR);
        kjb_fclose(alternate_input_fp);
        set_error_action(save_error_action);
        return ERROR;
    }

    loop_count_ptr = INT_MALLOC(1);

    if (loop_count_ptr == NULL)
    {
        Error_action save_error_action;

        save_error_action = get_error_action();
        set_error_action(FORCE_ADD_ERROR_ON_ERROR);
        kjb_fclose(alternate_input_fp);
        set_error_action(save_error_action);
        return ERROR;
    }

    *loop_count_ptr = fs_loop_count;

    res = insert_into_queue(&fs_loop_count_stack,
                            (Queue_element**)NULL,
                            (void*)loop_count_ptr);
    if (res == ERROR)
    {
        Error_action save_error_action;

        save_error_action = get_error_action();
        set_error_action(FORCE_ADD_ERROR_ON_ERROR);
        kjb_fclose(alternate_input_fp);
        set_error_action(save_error_action);
        return ERROR;
    }

    fs_loop_count = 0;

    res = insert_into_queue(&fs_loop_line_buff_stack,
                            (Queue_element**)NULL,
                            (void*)kjb_strdup(fs_loop_line_buff));
    fs_loop_line_buff[ 0 ] = '\0';

    if (res == ERROR)
    {
        Error_action save_error_action;

        save_error_action = get_error_action();
        set_error_action(FORCE_ADD_ERROR_ON_ERROR);
        kjb_fclose(alternate_input_fp);
        set_error_action(save_error_action);
        return ERROR;
    }

    res = insert_into_queue(&fs_alternate_input_left_over_stack,
                            (Queue_element**)NULL,
                            (void*)kjb_strdup(fs_input_line_buff_pos));
    fs_input_line_buff_pos += strlen(fs_input_line_buff_pos);

    if (res == ERROR)
    {
        Error_action save_error_action;

        save_error_action = get_error_action();
        set_error_action(FORCE_ADD_ERROR_ON_ERROR);
        kjb_fclose(alternate_input_fp);
        set_error_action(save_error_action);
        return ERROR;
    }

    fs_alternate_input_fp_stack_len++;

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*
 * =============================================================================
 *                                  get_command_text
 *
 * High level input routine
 *
 * This routine is the basic KJB library high level input routine. It provides
 * support for continued lines (last char is one a line is a "-"), multiple
 * commands on a line (commands are separated by semi-colons). In addition, this
 * routine provides support for an initialzation file and a history file
 * (described below).  then this routine provides support for editing and
 * re-entering previous lines. (If the program is not interactive, then this
 * routine reads from stdin). Finally, this routine implements a number of
 * commands. These commands are invisible from the calling program. This routine
 * only returns when the user enters a command which it does not know (or there
 * is an error in the processing of a command that it does know). Some of the
 * commans are best understood in terms of writing input "script" files. The
 * commands implemented are:
 *
 * |  quit / stop / exit / ctl-D  STOP      returns EOF
 * |  line with only whitespace   NULL      consumes line
 * |  Line begining with #        comment   consumes line
 * |  Line begining with *        comment   consumes line
 * |  Line begining with %        echo      echo's line
 * |
 * |  <   <file>                  read      takes input from <file>
 * |                                        until EOF. Recursion
 * |                                        is allowed.
 * |
 * |  ?                      user-input     promps user for next command
 * |
 * |  Line begining with !        system    executes rest, if in PATH
 * |
 * |  beep { < num > }            beep      Beep <num> times, with 1/2
 * |                                        second between beeps.
 * |
 * |  ...                         wait      User is prompted for return
 * |                                        to continue.
 * |
 * |  help { < topic > }          help      Uses kjb_help for help
 * |
 * |  loop <num> <command>        loop      Behaves as though the command
 * |                                        line was typed <num> times.
 * |
 * |  sleep { < num > }           sleep     sleeps for <num> seconds, or
 * |                                        1 if <num> is omitted.
 *
 * This list will likely be extended in the future.
 *
 * This routine implements a initialization file. On the first call, it first
 * reads the commands from an initialization file if it can find one. The first
 * place to look is the file pointed to in the environment variable
 * <PROGRAM>_INIT, where <PROGRAM> is the string program_name in upper case. If
 * that does not exist, then .<program> in the current directory is used, and
 * if that does not exist, then .<program> in the home directory is used.
 *
 * This routine implements the read of the command history from previous
 * sessions stored int the file ".<program>-history".
 *
 * This routine also implements an ad hoc method of synchronously suspending
 * program exection remotely. The original impetus for this is for scripts which
 * drive devices, etc, which are operating in a light controlled room. If
 * someone needs to enter the room, then the program needs to stop at a point
 * where entering the room is safe, and resuming execution is also safe. Thus,
 * if a file named "STOP" exists in the current directory, then the program will
 * stop between commands. When it stops, it will create a file named
 * "STOPPED_AND_WAITING". When the file "STOP" is removed, the program will
 * remove "STOPPED_AND_WAITING", and continue. This suspension method applies
 * even to the very first command, and thus the program can be started and
 * frozen before executing any commands, to be re-started for double remotely.
 * This may prove to be useful when one needs to exit the room before an
 * experiment begins (there are other ways to accomplish this, so it is not
 * clear if this will be used much).
 *
 * Notes:
 *    Currently the comment character does not change with the user settable
 *    comment charater as this is an option for data files. However, I could
 *    change my mind on this at some point.
 *
 * Macros:
 *    BUFF_GET_COMMAND_TEXT
 *
 * Returns:
 *    On success, this routine returnes the number of characters in "command".
 *    It returns EOF if the end if input has been reached, or the user entered
 *    stop/exit/quit/q/. It returns ERROR if there was an error in the read, or
 *    (much more likely) an error in a preprocessed command. If error is
 *    returned, then the reason is available to kjb_print_error. If the input
 *    command is larger than max_len, then TRUNCATED is returned.
 *
 * See also;
 *    kjb_help, kjb_print_error, BUFF_GET_COMMAND_TEXT
 *
 * Index: KJB library, High level routines, I/O, input
 *
 * -----------------------------------------------------------------------------
 */

long get_command_text
(
    const char* program_name /* Program name, used by help and for prompt. */,
    char* command            /* Buffer for command read. */,
    size_t max_len           /* Size of buffer.          */
)
{
    IMPORT volatile Bool halt_term_output;
    IMPORT volatile Bool halt_all_output;
    static int          first_time = TRUE;
    long         get_res;
    Pre_process_result  pp_res;


    if (first_time)
    {
        read_line_reentry_history(program_name);
        set_read_from_init_file(program_name);
        first_time = FALSE;
    }

    set_default_program_prompt(program_name);

    halt_term_output = FALSE;
    halt_all_output  = FALSE;

    /*
    // I don't see how this ever gets executed anymore! I have added the
    // UNTESTED_CODE macro to help figure out why this code is here. It looks
    // like dead code, leftover from some previous use.
    */
    if (fs_cached_pp_command != NULL)
    {
        UNTESTED_CODE();

        kjb_strncpy(command, fs_cached_pp_command, max_len);
        kjb_free(fs_cached_pp_command);
        fs_cached_pp_command = NULL;

        pp_res = PRE_PROCESS_INPUT;

        while (pp_res == PRE_PROCESS_INPUT)
        {
            pp_res = preprocess_command(program_name, command, max_len);
        }

        halt_term_output = FALSE;
        halt_all_output = FALSE;

        if (pp_res == PRE_PROCESS_EOF)
        {
            return EOF;
        }
        else if (pp_res == PRE_PROCESS_SUCCESS)
        {
            /*EMPTY*/
            ;  /* Do nothing */
        }
        else if (pp_res == PRE_PROCESS_ERROR)
        {
            return ERROR;
        }
        else
        {
            return strlen(command);
        }
    }

    halt_term_output = FALSE;
    halt_all_output = FALSE;

    while ((get_res = get_input_line_command(command, max_len)) >= 0)
    {
        pp_res = PRE_PROCESS_INPUT;

        while (pp_res == PRE_PROCESS_INPUT)
        {
            pp_res = preprocess_command(program_name, command, max_len);
        }

        halt_all_output = FALSE;
        halt_term_output = FALSE;

        if (pp_res == PRE_PROCESS_EOF)
        {
            return EOF;
        }
        else if (pp_res == PRE_PROCESS_SUCCESS)
        {
            /*EMPTY*/
            ;  /* Do nothing */
        }
        else if (pp_res == PRE_PROCESS_ERROR)
        {
            return ERROR;
        }
        else
        {
            return get_res;
        }
    }

    return get_res;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                             add_line_to_input_stream
 *
 * Puts text onto the stream where programs read commands
 *
 * This routine adds text onto the stream where programs read commands. A
 * typical use is to make text provided on the command line the initial input to
 * the program as it if was part of the input file. 
 *
 * Returns:
 *    ERROR on error, with an error message being set, otherwise NO_ERROR.
 *    Failure is not very likely. 
 *
 * Index:
 *     I/O, input 
 *
 * -----------------------------------------------------------------------------
*/

int add_line_to_input_stream(const char* line)
{


    ERE(initialize_input_line_buffer());

    kjb_strncat(fs_input_line_buff, line, fs_input_line_buff_size);

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 * STATIC                     set_read_from_init_file
 * -----------------------------------------------------------------------------
*/

static void set_read_from_init_file(const char* program_name)
{

    char init_file_name[ MAX_FILE_NAME_SIZE ];
    char init_file_path[ MAX_FILE_NAME_SIZE ];
    char init_file_env_var[ 1000 ];


    BUFF_CPY(init_file_name, ".");
    BUFF_CAT(init_file_name, program_name);

    EXTENDED_UC_BUFF_CPY(init_file_env_var, program_name);
    BUFF_CAT(init_file_env_var, "_INIT");

    if (get_config_file(init_file_env_var, (char*)NULL, init_file_name,
                        (char*)NULL, init_file_path, sizeof(init_file_path))
        != ERROR)
    {
        set_alternate_input(init_file_path);
    }
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 * STATIC                     get_input_line_command
 * -----------------------------------------------------------------------------
*/

static long get_input_line_command(char* command, size_t max_len)
{
    long   read_res;
    int           parse_res;


    ERE(initialize_input_line_buffer());

    parse_res = gen_match_quote_get_token(&fs_input_line_buff_pos,
                                          command, max_len, ";");

    while (parse_res == NO_MORE_TOKENS)
    {
        read_res = get_input_line(fs_input_line_buff, fs_input_line_buff_size);
        fs_input_line_buff_pos = fs_input_line_buff;

        if (read_res < 0)
        {
            fs_input_line_buff[ 0 ] = '\0';
            return read_res;
        }

        parse_res =
            gen_match_quote_get_token(&fs_input_line_buff_pos,
                                      command, max_len, ";");
    }

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

static long get_input_line(char* line, size_t max_len)
{
    FILE* alternate_input_fp;
    int   interactive;
    char  prompt[ MAX_PROGRAM_PROMPT_SIZE ];
    int   more_input;
    long  read_result;
    int   line_len;
    char* save_line;
    char  word[ 100 ];
    int   have_left_over = FALSE;


    if (fs_loop_count > 0)
    {
        kjb_strncpy(line, fs_loop_line_buff, fs_input_line_buff_size);
        fs_loop_count--;
        return strlen(line);
    }

    more_input = TRUE;
    save_line = line;

    /*
       Stop warning messages about read_result being used before set (it
       can't be, but the logic is a little tricky for software).
    */
    read_result = EOF;

    interactive = is_interactive();

    if (interactive)
    {
        BUFF_CPY(prompt, fs_kjb_program_prompt);
    }

    while (more_input)
    {
        /*
        // We break out as soon as we get a successful read.
        */
        while (fs_alternate_input_fp_stack != NULL)
        {
            Queue_element* removed_elem;


            alternate_input_fp = (FILE*)(fs_alternate_input_fp_stack->contents);

            if (fs_read_atn_flag)
            {
                pso("Interupting processing of %F\n", alternate_input_fp);
            }
            else
            {
                read_result = fget_line(alternate_input_fp, line, max_len);
            }

            if ((read_result == EOF) || fs_read_atn_flag)
            {
                int save_read_atn_flag = fs_read_atn_flag;

                kjb_fclose(alternate_input_fp); /*Ignore return--only reading.*/
                fs_read_atn_flag = FALSE;
                ERE(unset_atn_trap());

                NRE(removed_elem = remove_first_element(
                                                   &fs_alternate_input_fp_stack,
                                                   (Queue_element**)NULL));
                kjb_free(removed_elem);

                fs_loop_count = *((int*)(fs_loop_count_stack->contents));

                kjb_free(fs_loop_count_stack->contents);

                NRE(removed_elem = remove_first_element(&fs_loop_count_stack,
                                                        (Queue_element**)NULL));
                kjb_free(removed_elem);

                kjb_strncpy(fs_loop_line_buff,
                            (char*)(fs_loop_line_buff_stack->contents),
                            fs_input_line_buff_size);

                kjb_free(fs_loop_line_buff_stack->contents);

                NRE(removed_elem = remove_first_element(&fs_loop_line_buff_stack,
                                                        (Queue_element**)NULL));
                kjb_free(removed_elem);

                fs_alternate_input_fp_stack_len--;

                NRE(removed_elem = remove_first_element(
                                              &fs_alternate_input_left_over_stack,
                                              (Queue_element**)NULL));
                if (removed_elem != NULL)
                {
                    if (*((char*)removed_elem->contents) != '\0')
                    {
                        kjb_strncpy(line, (char*)removed_elem->contents,
                                    max_len);
                        have_left_over = TRUE;
                    }
                    kjb_free(removed_elem->contents);
                }

                kjb_free(removed_elem);

                if (have_left_over)
                {
                    return strlen(save_line);
                }

                if (save_read_atn_flag)
                {
                    fs_loop_count = 0;
                }

                if (fs_loop_count > 0)
                {
                    kjb_strncpy(line, fs_loop_line_buff, fs_input_line_buff_size);
                    fs_loop_count--;
                    return strlen(save_line);
                }
            }
            else
            {
                if ((interactive) && (fs_echo_alternate_input))
                {
                    term_puts(line);
                    term_puts("\n");
                }

                break;
            }
        }

        if (fs_alternate_input_fp_stack != NULL)
        {
            /*EMPTY*/
            ; /* Do nothing. (Already read a line, above) */
        }
        else if (interactive)
        {
            read_result = term_get_line(prompt, line, max_len);
        }
        else
        {
            read_result = fget_line(stdin, line, max_len);
        }

        if (read_result == TRUNCATED)
        {
            set_error("Input is too long. It is ignored.");
            return ERROR;
        }

        if (read_result < 0)
        {
            return read_result;
        }
        else
        {
            trim_end(line);
            line_len = strlen(line);

            if (line_len == 0) more_input = FALSE;

            else if (line[ line_len - 1 ] == '-')
            {
                line += (line_len - 1);
                max_len -= (line_len - 1);

                if (interactive) { strcpy(prompt, "Continue> "); }
            }
            else
            {
                max_len -= line_len;
                more_input = FALSE;
            }
        }
    }

    line = save_line;

    if (BUFF_GET_TOKEN_OK(&line, word))
    {
        extended_uc_lc(word);

        if (STRCMP_EQ(word, "loop"))
        {
            if (BUFF_GET_TOKEN_OK(&line, word))
            {
                if (ss1pi(word, &fs_loop_count) == ERROR)
                {
                    insert_error("Invalid loop command.");
                    return ERROR;
                }
                else if (fs_loop_count == 0)
                {
                    save_line[ 0 ] = '\0';
                    return NO_ERROR;
                }

                kjb_strncpy(fs_loop_line_buff, line, fs_input_line_buff_size);
                kjb_strncpy(save_line, fs_loop_line_buff, fs_input_line_buff_size);
                fs_loop_count--;
            }
            else
            {
                set_error("Loop command requires a count.");
                return ERROR;
            }
        }
    }

    return strlen(save_line);
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */


/* =============================================================================
 * STATIC                          preprocess_command
 * -----------------------------------------------------------------------------
*/

static Pre_process_result preprocess_command(const char* program_name, char* command, size_t max_len)
{
    char* command_pos;
    char  first_command_word[ 200 ];
    char  command_argument[ 200 ];


    check_for_freeze();

    if ( ! is_interactive() )
    {
        put_line(command);
    }

    command_pos = command;

    trim_beg(&command_pos);

    if (*command_pos == '\0')
    {
        return PRE_PROCESS_SUCCESS;
    }
    else if ((*command_pos == '*') || (*command_pos == '#'))
    {
        return PRE_PROCESS_SUCCESS;
     }
    else if (*command_pos == '%')
    {
        /*
        // If we are NOT interative, then we will echo the input as above. In
        // the case of the % comment, we want to force the echo even if we
        // are interactive.
        */
        if (is_interactive())
        {
            put_line(command);
        }

        return PRE_PROCESS_SUCCESS;
    }
    else if (*command_pos == '<')
    {
        command_pos++;
        trim_beg(&command_pos);
        trim_end(command_pos);

        if (set_alternate_input(command_pos) == ERROR)
        {
            return PRE_PROCESS_ERROR;
        }
        else
        {
            return PRE_PROCESS_SUCCESS;
        }
    }
    else if (*command_pos == '?')
    {
        if (is_interactive())
        {
            int res;

            command[ 0 ] = '\0';

            res = term_get_line("? ", command, max_len);

            if (res == ERROR)
            {
                return PRE_PROCESS_ERROR;
            }
            else
            {
                return PRE_PROCESS_INPUT;
            }
        }
        else
        {
            return PRE_PROCESS_ERROR;
        }
    }
    else if ((*command_pos == '$') || (*command_pos == '!'))
    {
        command_pos++;
        EPE(kjb_system(command_pos));
        return PRE_PROCESS_SUCCESS;
    }
    else
    {
        /*
           command_pos is pointing at a non-blank, non-null, so
           BUFF_GET_TOKEN will succeed in getting a token.
        */
        BUFF_GET_TOKEN(&command_pos, first_command_word);
        extended_uc_lc(first_command_word);

        if (STRCMP_EQ(first_command_word, "sleep"))
        {
            int num_seconds;


            trim_beg(&command_pos);
            BUFF_GET_TOKEN(&command_pos, command_argument);

            if (command_argument[ 0 ] != '\0')
            {
                if (ss1pi(command_argument, &num_seconds) == ERROR)
                {
                    return PRE_PROCESS_ERROR;
                }
            }
            else
            {
                num_seconds = 1;
            }

            sleep((unsigned int)num_seconds);

            return PRE_PROCESS_SUCCESS;
        }
        else if (STRCMP_EQ(first_command_word, "beep"))
        {
            int i;
            int beep_count;


            trim_beg(&command_pos);
            BUFF_GET_TOKEN(&command_pos, command_argument);

            if (command_argument[ 0 ] != '\0')
            {
                if (ss1pi(command_argument, &beep_count) == ERROR)
                {
                    return PRE_PROCESS_ERROR;
                }
            }
            else
            {
                beep_count = 1;
            }

            if (beep_count > 0)
            {
                term_beep();
                beep_count--;
            }

            for (i=0; i<beep_count; i++)
            {
                nap(500);
                term_beep();
            }

            return PRE_PROCESS_SUCCESS;
        }
        else if (STRCMP_EQ(first_command_word, "..."))
        {
            char buff[ 100 ];


            dont_restart_on_atn();
            BUFF_TERM_GET_LINE("Enter return to continue ... ", buff);
            command[ 0 ] = '\0';
            reset_restart_on_atn();
            return PRE_PROCESS_SUCCESS;
        }
        else if (    (STRCMP_EQ(first_command_word, "help"))
                  || (STRCMP_EQ(first_command_word, "h"))
                )
        {
            trim_beg(&command_pos);

            if (kjb_help(program_name, command_pos) == ERROR)
            {
                return PRE_PROCESS_ERROR;
            }
            else
            {
                return PRE_PROCESS_SUCCESS;
            }
        }
        else if (STRCMP_EQ(first_command_word, "stop"))
        {
            return PRE_PROCESS_EOF;
        }
        else if (STRCMP_EQ(first_command_word, "quit"))
        {
            return PRE_PROCESS_EOF;
        }
        else if (STRCMP_EQ(first_command_word, "q"))
        {
            return PRE_PROCESS_EOF;
        }
        else if (STRCMP_EQ(first_command_word, "exit"))
        {
            return PRE_PROCESS_EOF;
        }
    }

    return PRE_PROCESS_NOT_APPLICABLE;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

int set_input_line_buffer_size(int max_len)
{


    UNTESTED_CODE();

    if ((max_len > INT_MAX) || (max_len < 80))
    {
        SET_ARGUMENT_BUG();
        return ERROR;
    }

    if (fs_input_line_buff != NULL)
    {
        set_bug(
            "Input line buffer can only be set before any I/O has occured.\n");
        return ERROR;
    }

    fs_input_line_buff_size = max_len;

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

static int initialize_input_line_buffer(void)
{


    if (fs_input_line_buff == NULL)
    {
        NRE(fs_input_line_buff = STR_MALLOC(fs_input_line_buff_size));

        fs_loop_line_buff = STR_MALLOC(fs_input_line_buff_size);

        if (fs_loop_line_buff == NULL)
        {
            kjb_free(fs_input_line_buff);
            return ERROR;
        }

        *fs_input_line_buff = '\0';
        *fs_loop_line_buff = '\0';
        fs_input_line_buff_pos = fs_input_line_buff;

#ifdef TRACK_MEMORY_ALLOCATION
        add_cleanup_function(free_input_line_buff);
#endif
    }

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 * STATIC                          free_input_line_buff
 * -----------------------------------------------------------------------------
*/

#ifdef TRACK_MEMORY_ALLOCATION

static void free_input_line_buff(void)
{


    kjb_free(fs_input_line_buff);
    kjb_free(fs_loop_line_buff);

    fs_loop_line_buff      = NULL;
    fs_input_line_buff     = NULL;
    fs_input_line_buff_pos = NULL;
}

#endif

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

#ifdef TRACK_MEMORY_ALLOCATION
static void free_alternate_input_file_queue(void)
{
    Queue_element* cur_elem;


    do
    {
        cur_elem = remove_first_element(&fs_alternate_input_fp_stack,
                                        (Queue_element**)NULL);

        if (cur_elem != NULL)
        {
            /* Print error, as this is a void cleanup function. */
            EPE(kjb_fclose((FILE*)(cur_elem->contents)));

            kjb_free(cur_elem);
        }
    }
    while (cur_elem != NULL);

    fs_alternate_input_fp_stack_len = 0;

    free_queue(&fs_loop_count_stack, (Queue_element**)NULL, kjb_free);
    free_queue(&fs_loop_line_buff_stack, (Queue_element**)NULL, kjb_free);
    free_queue(&fs_alternate_input_left_over_stack, (Queue_element**)NULL,
               kjb_free);

    fs_alternate_input_fp_stack = NULL;
    fs_alternate_input_left_over_stack = NULL; 
}
#endif

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

Continue_query_response continue_query(void)
{
    IMPORT volatile int kjb_tty_cols;
    static int long_prompt_cutoff = NOT_SET;
    static int medium_prompt_cutoff = NOT_SET;
    static const char* long_prompt = "Continue? (SPACE or y for yes, n for no) ";
    static const char* medium_prompt = "Continue? ";
    static const char* short_prompt = "? ";
    const char* prompt;
    char input[ SINGLE_CHARACTER_BUFF_SIZE ];
    Continue_query_response result;
    int read_result;


    if (    (long_prompt_cutoff == NOT_SET)
         || (medium_prompt_cutoff == NOT_SET)
       )
    {
        long_prompt_cutoff = strlen(long_prompt) + 1;  /* 1 space for anser */
        medium_prompt_cutoff = strlen(medium_prompt) + 1;
    }

    if (kjb_tty_cols < medium_prompt_cutoff)
    {
        /*
        // If short prompt wont fit on a line, then tough. Let the user deal
        // with it, as this case is VERY pathalogical.
        */
        prompt = short_prompt;
    }
    else if (kjb_tty_cols < long_prompt_cutoff)
    {
        prompt = medium_prompt;
    }
    else
    {
        prompt = long_prompt;
    }

     /*
     // set_atn_trap(yes_no_query_atn_fn, DONT_RESTART_AFTER_SIGNAL);
     */

     read_result = overwrite_query(prompt, input, sizeof(input));

     result = QUERY_RESPONSE_NOT_SET;

     while (result == QUERY_RESPONSE_NOT_SET)
     {
         if (read_result == ERROR)
         {
             result = QUERY_ERROR;
         }
         else if (read_result == EOF)
         {
             /*
             // CHECK
             //
             // Not 100% sure on this choice.
             */
             result = POSITIVE_RESPONSE;
         }
         else if (read_result == INTERRUPTED)
         {
             result = STRONG_NEGATIVE_RESPONSE;
         }
         else if (read_result == WOULD_BLOCK)
         {
             /*
             // CHECK
             //
             // Don't know about this one either.
             */
             result = POSITIVE_RESPONSE;
         }
         else if (read_result == 0)
         {
             result = POSITIVE_RESPONSE;
         }
         else if (read_result != 1)
         {
             set_bug("Unexpected read result (%R) in continue_query.",
                     read_result);
             result = STRONG_NEGATIVE_RESPONSE;
         }
         else if ((*input == '\0') || (*input == ' '))
         {
             result = POSITIVE_RESPONSE;
         }
         else if (*input == 'y')
         {
             result = POSITIVE_RESPONSE;
         }
         else if (*input == 'Y')
         {
             result = STRONG_POSITIVE_RESPONSE;
         }
         else if ((*input == 'n') || (*input == 'q'))
         {
             result = NEGATIVE_RESPONSE;
         }
         else if ((*input == 'N') || (*input == 'Q'))
         {
             result = STRONG_NEGATIVE_RESPONSE;
         }
         else if ((*input == 'd') || (*input == 'D'))
         {
             result = DISABLE_QUERY;
         }

         if (result == QUERY_RESPONSE_NOT_SET)
         {
             read_result = overwrite_query("Enter either y (yes) or n (no) ",
                                           input, sizeof(input));
         }
     }

     /*
     // unset_atn_trap();
     */

     return result;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*ARGSUSED*/
static TRAP_FN_RETURN_TYPE alternate_file_read_atn_fn( TRAP_FN_DUMMY_ARGS )
{

#ifdef TEST
    if (yes_no_query("io_atn_fn: Interupt reading from alternative input (y/n/q)"))
#else
    if (yes_no_query("io_atn_fn: Interupt reading from alternative input (y/n/q)? "))
#endif
    {
        fs_read_atn_flag = TRUE;
    }

}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*
 * =============================================================================
 *                                read_int_from_file
 *
 * Reads an integer from a file
 *
 * This routine reads an integer from a file. The file should have exactly one
 * integer in it other than white space and comment lines. The value of the
 * integer pointed to by int_ptr is only changed if the routine is succeeds.
 *
 * Returns:
 *    ERROR on error, with an error message being set, otherwise NO_ERROR.
 *    Failures inlude the file does not exist, cannot be opened for reading, and
 *    does not have exactly one valid integer other than white space and
 *    comment lines.
 *
 * -----------------------------------------------------------------------------
 */

/* In retrospect, this routine looks a bit heavy duty, but does the job. */

int read_int_from_file(int* int_ptr, const char* file_name)
{
    FILE* data_fp;
    int   num_elements;
    char  line[ LARGE_IO_BUFF_SIZE ];
    char* line_pos;
    char  element_buff[ 200 ];
    int   scan_res, read_res;
    int   temp_int;


    NRE(data_fp = kjb_fopen(file_name, "r"));

    num_elements = 0;

    /*CONSTCOND*/
    while (TRUE)
    {
        read_res = BUFF_GET_REAL_LINE(data_fp, line);

        if (read_res == ERROR)
        {
            Error_action save_error_action = get_error_action();

            set_error_action(FORCE_ADD_ERROR_ON_ERROR);
            (void)kjb_fclose(data_fp);
            set_error_action(save_error_action);

            return ERROR;
        }
        else if (read_res == EOF)
        {
            break;
        }

        line_pos = line;

        while (BUFF_GEN_GET_TOKEN_OK(&line_pos, element_buff, " ,"))
        {
            num_elements++;
        }
    }

    if (num_elements != 1)
    {
        Error_action save_error_action = get_error_action();

        set_error("Expecting exactly one number in file %F.", data_fp);
        set_error_action(FORCE_ADD_ERROR_ON_ERROR);
        (void)kjb_fclose(data_fp);
        set_error_action(save_error_action);

        return ERROR;
    }

    rewind(data_fp);

    /*CONSTCOND*/
    while (TRUE)
    {
        read_res = BUFF_GET_REAL_LINE(data_fp, line);

        if (read_res == ERROR)
        {
            Error_action save_error_action = get_error_action();

            set_error_action(FORCE_ADD_ERROR_ON_ERROR);
            (void)kjb_fclose(data_fp);
            set_error_action(save_error_action);

            return ERROR;
        }
        else if (read_res == EOF)
        {
            break;
        }

        line_pos = line;

        while (BUFF_GEN_GET_TOKEN_OK(&line_pos, element_buff, " ,"))
        {
            scan_res = ss1i(element_buff, &temp_int);

            if (scan_res == ERROR)
            {
                Error_action save_error_action = get_error_action();

                insert_error("Error reading integer from %F.", data_fp);
                set_error_action(FORCE_ADD_ERROR_ON_ERROR);
                (void)kjb_fclose(data_fp);
                set_error_action(save_error_action);

                return ERROR;
            }
        }
    }

    ERE(kjb_fclose(data_fp));

    *int_ptr = temp_int;

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*
 * =============================================================================
 *                                read_dbl_from_file
 *
 * Reads a double from a file
 *
 * This routine reads an double from a file. The file should have exactly one
 * double in it other than white space and comment lines. The value of the
 * double pointed to by dbl_ptr is only changed if the routine is succeeds.
 *
 * Returns:
 *    ERROR on error, with an error message being set, otherwise NO_ERROR.
 *    Failures inlude the file does not exist, cannot be opened for reading, and
 *    does not have exactly one valid double other than white space and
 *    comment lines.
 *
 * Note: 
 *    This routine and read_double() are somewhat redundant. Currently, this
 *    routine insists that there is exactly one number in the file, but
 *    read_double() does not. 
 *
 * -----------------------------------------------------------------------------
 */

/*
 * This routine was adapted from read_int_from_file() which is overly heavy
 * duty, but does the job.
*/

int read_dbl_from_file(double* dbl_ptr, const char* file_name)
{
    FILE* data_fp;
    int   num_elements;
    char  line[ LARGE_IO_BUFF_SIZE ];
    char* line_pos;
    char  element_buff[ 200 ];
    int   scan_res, read_res;
    double   temp_dbl;


    NRE(data_fp = kjb_fopen(file_name, "r"));

    num_elements = 0;

    /*CONSTCOND*/
    while (TRUE)
    {
        read_res = BUFF_GET_REAL_LINE(data_fp, line);

        if (read_res == ERROR)
        {
            Error_action save_error_action = get_error_action();

            set_error_action(FORCE_ADD_ERROR_ON_ERROR);
            (void)kjb_fclose(data_fp);
            set_error_action(save_error_action);

            return ERROR;
        }
        else if (read_res == EOF)
        {
            break;
        }

        line_pos = line;

        while (BUFF_GEN_GET_TOKEN_OK(&line_pos, element_buff, " ,"))
        {
            num_elements++;
        }
    }

    if (num_elements != 1)
    {
        Error_action save_error_action = get_error_action();

        set_error("Expecting exactly one number in file %F.", data_fp);
        set_error_action(FORCE_ADD_ERROR_ON_ERROR);
        (void)kjb_fclose(data_fp);
        set_error_action(save_error_action);

        return ERROR;
    }

    rewind(data_fp);

    /*CONSTCOND*/
    while (TRUE)
    {
        read_res = BUFF_GET_REAL_LINE(data_fp, line);

        if (read_res == ERROR)
        {
            Error_action save_error_action = get_error_action();

            set_error_action(FORCE_ADD_ERROR_ON_ERROR);
            (void)kjb_fclose(data_fp);
            set_error_action(save_error_action);

            return ERROR;
        }
        else if (read_res == EOF)
        {
            break;
        }

        line_pos = line;

        while (BUFF_GEN_GET_TOKEN_OK(&line_pos, element_buff, " ,"))
        {
            scan_res = ss1snd(element_buff, &temp_dbl);

            if (scan_res == ERROR)
            {
                Error_action save_error_action = get_error_action();

                insert_error("Error reading double from %F.", data_fp);
                set_error_action(FORCE_ADD_ERROR_ON_ERROR);
                (void)kjb_fclose(data_fp);
                set_error_action(save_error_action);

                return ERROR;
            }
        }
    }

    ERE(kjb_fclose(data_fp));

    *dbl_ptr = temp_dbl;

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

int read_and_create_int_array(char* file_name, int* num_elements_ptr, int** array_ptr)
{
    FILE* data_fp;
    int   num_elements;
    int* data_array;
    int* data_array_pos;
    char  line[ LARGE_IO_BUFF_SIZE ];
    char* line_pos;
    char  element_buff[ 200 ];
    int   scan_res, read_res;


    NRE(data_fp = kjb_fopen(file_name, "r"));

    num_elements = 0;

    /*CONSTCOND*/
    while (TRUE)
    {
        read_res = BUFF_GET_REAL_LINE(data_fp, line);

        if (read_res == ERROR)
        {
            Error_action save_error_action = get_error_action();

            set_error_action(FORCE_ADD_ERROR_ON_ERROR);
            (void)kjb_fclose(data_fp);
            set_error_action(save_error_action);

            return ERROR;
        }
        else if (read_res == EOF)
        {
            break;
        }

        line_pos = line;

        while (BUFF_GEN_GET_TOKEN_OK(&line_pos, element_buff, " ,"))
        {
            num_elements++;
        }
    }

    if (num_elements == 0)
    {
        Error_action save_error_action = get_error_action();

        set_error("Expecting at least one number in file %F.", data_fp);
        set_error_action(FORCE_ADD_ERROR_ON_ERROR);
        (void)kjb_fclose(data_fp);
        set_error_action(save_error_action);

        return ERROR;
    }
    else
    {
        rewind(data_fp);
    }

    NRE(data_array = INT_MALLOC(num_elements));

    data_array_pos = data_array;

    /*CONSTCOND*/
    while (TRUE)
    {
        read_res = BUFF_GET_REAL_LINE(data_fp, line);

        if (read_res == ERROR)
        {
            Error_action save_error_action = get_error_action();

            set_error_action(FORCE_ADD_ERROR_ON_ERROR);
            (void)kjb_fclose(data_fp);
            set_error_action(save_error_action);
            kjb_free(data_array);

            return ERROR;
        }
        else if (read_res == EOF)
        {
            break;
        }

        line_pos = line;

        while (BUFF_GEN_GET_TOKEN_OK(&line_pos, element_buff, " ,"))
        {
            scan_res = ss1i(element_buff, data_array_pos);

            if (scan_res == ERROR)
            {
                Error_action save_error_action = get_error_action();

                insert_error("Error reading integer from %F.", data_fp);
                set_error_action(FORCE_ADD_ERROR_ON_ERROR);
                (void)kjb_fclose(data_fp);
                kjb_free(data_array);
                set_error_action(save_error_action);

                return ERROR;
            }
            data_array_pos++;
        }
    }

    ERE(kjb_fclose(data_fp));

    *num_elements_ptr = num_elements;
    *array_ptr = data_array;

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

int kjb_fread_4_bytes(FILE* fp, void* buff)
{

    return kjb_fread_exact(fp, buff, 4);
}


/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

int kjb_fread_4_bytes_backwards(FILE* fp, void* buff)
{
    char temp_buff[ 4 ];

    ERE(kjb_fread_exact(fp, temp_buff, 4));

    ((char*)buff)[ 0 ] = temp_buff[ 3 ];
    ((char*)buff)[ 1 ] = temp_buff[ 2 ];
    ((char*)buff)[ 2 ] = temp_buff[ 1 ];
    ((char*)buff)[ 3 ] = temp_buff[ 0 ];

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

int read_double(double* value_ptr, const char* file_name)
{
    FILE* fp;
    int   result;


    if ((file_name == NULL) || (*file_name == '\0'))
    {
        fp = stdin;
    }
    else
    {
        NRE(fp = kjb_fopen(file_name, "r"));
    }

    result = fp_read_double(value_ptr, fp);

    if ((file_name != NULL) && (*file_name != '\0'))
    {
        (void)kjb_fclose(fp);  /* Ignore return--only reading. */
    }

    return result;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

int fp_read_double(double* value_ptr, FILE* fp)
{
    char  line[ LARGE_IO_BUFF_SIZE ];
    char* line_pos;


    line_pos = line;
    trim_beg(&line_pos);
    trim_end(line_pos);

    ERE(BUFF_GET_REAL_LINE(fp, line));

    line_pos = line;

    trim_beg(&line_pos);
    trim_end(line_pos);

    return ss1snd(line_pos, value_ptr);
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

int output_int(const char* output_dir, const char* file_name, int value)
{
    char out_file_name[ MAX_FILE_NAME_SIZE ];
    FILE* fp;

    out_file_name[ 0 ] = '\0';

    if (output_dir != NULL)
    {
        BUFF_CAT(out_file_name, output_dir);
        BUFF_CAT(out_file_name, DIR_STR);
    }

    BUFF_CAT(out_file_name, file_name);

    if (skip_because_no_overwrite(file_name)) return NO_ERROR;

    NRE(fp = kjb_fopen(out_file_name, "w"));
    ERE(kjb_fprintf(fp, "%d\n", value));

    return kjb_fclose(fp);
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

int output_double
(
    const char* output_dir,
    const char* file_name,
    double      value
)
{
    char out_file_name[ MAX_FILE_NAME_SIZE ];
    FILE* fp;

    out_file_name[ 0 ] = '\0';

    if (output_dir != NULL)
    {
        BUFF_CAT(out_file_name, output_dir);
        BUFF_CAT(out_file_name, DIR_STR);
    }

    BUFF_CAT(out_file_name, file_name);

    if (skip_because_no_overwrite(file_name)) return NO_ERROR;

    NRE(fp = kjb_fopen(out_file_name, "w"));
    ERE(kjb_fprintf(fp, "%.10e\n", value));

    return kjb_fclose(fp);
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                                 push_no_overwrite
 *                               
 * Sets the current behavior regarding clobbering files
 *
 * This routine is similar to set_no_overwrite(), except that the previous value
 * is pushed onto a stack, so that it can be restored with pop_no_overwrite(). 
 *
 * Returns: 
 *     NO_ERROR on success and ERROR if there are problems, with an error
 *     message being set. 
 *
 * Index: I/O
 *
 * -----------------------------------------------------------------------------
*/ 

int push_no_overwrite(int no_overwrite)
{
    int* save_no_overwrite_ptr; 
    int result = NO_ERROR;
#ifdef TRACK_MEMORY_ALLOCATION
    static int  fs_first_no_overwrite_stack_use = TRUE; 
#endif 


#ifdef TRACK_MEMORY_ALLOCATION
    if (fs_first_no_overwrite_stack_use) 
    {
        fs_first_no_overwrite_stack_use = FALSE; 
        add_cleanup_function(free_no_overwrite_stack);
    }
#endif 

    SKIP_HEAP_CHECK_2(); 
    save_no_overwrite_ptr = TYPE_MALLOC(int);
    CONTINUE_HEAP_CHECK_2();

    if (save_no_overwrite_ptr == NULL)
    {
        add_error("Unable to push no overwrite action onto stack."); 
        result = ERROR;
    }
    else
    {
        *save_no_overwrite_ptr = get_no_overwrite();

        SKIP_HEAP_CHECK_2(); 
        result = insert_into_queue(&fs_no_overwrite_stack_head, 
                                   (Queue_element**)NULL, 
                                   (void*)save_no_overwrite_ptr);
        CONTINUE_HEAP_CHECK_2();
        
        if (result == ERROR)
        {
            add_error("Unable to push no overwrite onto stack."); 
        }
    }

    set_no_overwrite(no_overwrite); 

    return result;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                                 pop_no_overwrite
 *                               
 * Restores previous behavior regarding clobbering files
 *
 * This routine restores the error action to the behaviour before the last
 * push_no_overwrite().
 *
 * If the stack is NULL, then this is treated as a bug in development code, and
 * silently ignored otherwise. 
 *
 * Index: I/O
 *
 * -----------------------------------------------------------------------------
*/ 

void pop_no_overwrite(void)
{
    Queue_element* cur_elem;
    
    if (fs_no_overwrite_stack_head == NULL) 
    {
#ifdef TEST
        set_bug("No overwrite pop on empty stack.");
#endif 
        return;  
    }

    cur_elem = remove_first_element(&fs_no_overwrite_stack_head,
                                    (Queue_element**)NULL);

    if (cur_elem == NULL)
    {
        kjb_print_error();
        set_bug("Unable to pop no overwrite from stack."); 
        return;
    }

    set_no_overwrite(*((int*)(cur_elem->contents)));

    SKIP_HEAP_CHECK_2(); 
    free_queue_element(cur_elem, kjb_free); 
    CONTINUE_HEAP_CHECK_2();
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

#ifdef TRACK_MEMORY_ALLOCATION
/*
 * =============================================================================
 * STATIC                      free_no_overwrite_stack
 * -----------------------------------------------------------------------------
 */

static void free_no_overwrite_stack(void)
{
    free_queue(&fs_no_overwrite_stack_head, (Queue_element**)NULL,
               kjb_free); 

    fs_no_overwrite_stack_head = NULL; 
}

#endif 

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */
    
/* =============================================================================
 *                                 set_no_overwrite
 *                               
 * Sets the current behavior regarding clobbering files
 *
 * The routine sets the "no overwrite" action. The default value is 0 which
 * means that files are over-written. If the value is not zero, then routines
 * that write to a file as a complete operation and which consult this module
 * will not over write an existing file. By convention, if the value is 1, then
 * the abort from writing is silent, if the value is 2, then a warning message
 * is printed.
 *                               
 * Index: I/O
 *
 * -----------------------------------------------------------------------------
*/ 

void set_no_overwrite(int no_overwrite)
{

    fs_no_overwrite = no_overwrite;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                                  get_no_overwrite
 *
 * Returns the current behavior regarding clobbering files
 *
 * The routine returns the current setting of the "no overwrite" action. The
 * default value is 0 which means that files are over-written. If the value is
 * not zero, then routines that write to a file as a complete operation and
 * which consult this module will not over write an existing file. By
 * convention, if the value is 1, then the abort from writing is silent, if the
 * value is 2, then a warning message is printed.
 *                               
 * Index: I/O
 *
 * -----------------------------------------------------------------------------
*/ 

int get_no_overwrite(void)
{

    return fs_no_overwrite;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

int skip_because_no_overwrite(const char* file_name)
{
    if ( ! fs_no_overwrite) return FALSE;

    if (is_file(file_name))
    {
        if (fs_no_overwrite == 2)
        {
            warn_pso("Skipping write to %s because it already exists.\n", 
                     file_name);
        }
        return TRUE;
    }
    return FALSE;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

#ifdef __cplusplus
}
#endif

