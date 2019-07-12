
/* $Id: l_justify.c 21520 2017-07-22 15:09:04Z kobus $ */

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
#include "l/l_justify.h"

#ifdef __cplusplus
extern "C" {
#endif

/* -------------------------------------------------------------------------- */

#define L_JUSTIFY_SMALL_BUFF_SIZE 2000

static char   fs_small_buff[ L_JUSTIFY_SMALL_BUFF_SIZE ];
static char*  fs_big_buff         = NULL;
static char*  fs_buff = fs_small_buff;
static size_t fs_buff_size = sizeof(fs_small_buff);

/* -------------------------------------------------------------------------- */

static void free_justify_big_buff(void);

/* -------------------------------------------------------------------------- */

/* =============================================================================
 *                                  left_justify
 *
 * Left justifies text
 *
 * This is a simple left_justify routine. More comprehensive code can be found
 * as part of the program "par". One day, I may even incorperate this (or other
 * formating) code into the KJB library.
 *
 * The input string parameter is written reformatted to the output string
 * parameter.   The rerortting strips returns and re-adds them so that if the
 * output string is printed it will be justified.
 *
 * The parameters no_format_indent and no_format_chars can be used for some
 * control over when formatting occurs indenting. If no_format_indent is
 * greater than 0, any line which has that many blanks (or more) followed by
 * non-blanks, will be left alone, except that the lines will be wrapped to fit
 * in width if the line is too long. Similarly, any line whose first no-white
 * space is a character in the string no_format_chars will be left alone,
 * except that they wil also be wrapped, and if wrapped, the specific character
 * which supressed the formatting will be added as the first character of the
 * new line. Finally, if formatting is suppressed due to no_format_indent, and
 * the line wraps, and the first character would be one of the no_format_chars,
 * then an extra space is inserted.
 *
 * The boolean parameter keep_indentation maintains the indent level of the
 * input. When the indent level of the input changes, then a new line is
 * forced, and the indentation is reset. This behaviour is often what is
 * required.
 *
 * If the the paramter extra_indent is positive, then that number of blanks is
 * added to the begining of each formated line. This is in addition to the
 * indent which is set by the input if keep_indentation is used. Thus in this
 * case, the indent structure of the input is maintained, but the exact indent
 * is more.
 *
 * Returns:
 *    NO_ERROR is returned on success. If the length of the output buffer as
 *    specified by the parameter max_len is not large enough to hold the
 *    result, then left_justify returns TRUNCATED. The output is null
 *    terminated, and is truncated.
 *
 * Index: justify
 *
 * -----------------------------------------------------------------------------
*/

int left_justify
(
    const char* in_str,
    int width,
    int no_format_indent, /* Number of initial blanks required to suppress
                             indenting. Use 0 to indent all.               */

    const char* no_format_chars, /* Character in colum one which suppress
                                   indenting. Use NULL or "" to disregard. */

    Bool keep_indentation,  /* If TRUE, indentation is maintained. A change in
                             indent forces a new line, and justification
                             continues at the new indent.  */

    int extra_indent, /* Extra indent for justified lines. If positive, all
                        justified lines are indented this amount above what
                        they would be otherwise. Lines for which indentation is
                        disabled through no_format_indent or no_format_chars
                        are not affected.  */

    char* out_str,  /* Output buffer. Should be size max_len or more. */

    long max_len  /* Size of output buffer. */
)
{
    int    blank_count;
    char*  line_in_pos;
    size_t input_size;
    int    room_left      = width;
    long   sget_line_res;
    int    current_indent = 0;

    /*
    // total_out_count will always be one extra as soon as we start an output
    // line in anticipation of the '\n' which will always be output.   When the
    // '\n' is finally output, total_out_count is not increased. The flag
    // need_new_line is TRUE when total_out_count is one extra due to the
    // reseverd space.
    */
    long total_out_count = 0;
    Bool need_new_line = FALSE;


    if ((width < 1) || (max_len < 1) || (width < 1))
    {
        SET_ARGUMENT_BUG();
        return ERROR;
    }

    if (max_len == 1)
    {
        *out_str = '\0';

        if (*in_str != '\0') return TRUNCATED;
        else                 return NO_ERROR;
    }

    max_len--;   /* Room for null  */

    if (no_format_chars == NULL) no_format_chars = "";

    if (extra_indent < 0) extra_indent = 0;

    if (extra_indent < width) current_indent = extra_indent;

    
    /* We start of with  a relatively small buffer that we bump up if we
     * encounter longer input. 
    */

    input_size = strlen(in_str) + ROOM_FOR_NULL;
    input_size += 500;   /* Some arbitrary amount extra. */

    if (input_size > fs_buff_size)
    {
        if (fs_big_buff == NULL)
        {
            add_cleanup_function(free_justify_big_buff);
        }
        else
        {
            kjb_free(fs_big_buff);
        }

        NRE(fs_big_buff = STR_MALLOC(input_size));
        fs_buff = fs_big_buff;
        fs_buff_size = input_size;
    }

    while (    ((sget_line_res = const_sget_line(&in_str, fs_buff, fs_buff_size)) != EOF)
            && (total_out_count < max_len))
    {
        char no_format_char = '\0';


        line_in_pos = fs_buff;

        if (*line_in_pos)
        {
            int no_format_char_num = find_char(no_format_chars, *line_in_pos);


            if (no_format_char_num)
            {
                no_format_char = no_format_chars[ no_format_char_num - 1 ];
            }
        }

        blank_count = trim_beg(&line_in_pos);

        if (    no_format_char
             ||
                (    (no_format_indent > 0)
                  && (blank_count >= no_format_indent)
                )
             ||
                 (*line_in_pos == '\0')
            )
        {
            line_in_pos = fs_buff;

            if (need_new_line) *out_str++ = '\n';

            total_out_count++;

            room_left = width;

            while (    (room_left > 0)
                    && (total_out_count < max_len)
                    && (*line_in_pos != '\0')
                  )
            {
                while (    (room_left > 0)
                        && (total_out_count < max_len)
                        && (*line_in_pos != '\0')
                      )
                {
                    *out_str++ = *line_in_pos++;
                    room_left--;
                    total_out_count++;
                }

                if ((total_out_count < max_len) && (*line_in_pos != '\0'))
                {
                    room_left = width;
                    *out_str++ = '\n';
                    total_out_count++;

                    /*
                    // If we are skipping formatting due to a no_format_char,
                    // then each subsequent non-formatted line should begin
                    // with that char.
                    //
                    // On the other hand, if we are skipping formatting for
                    // some other reason, then the first character should NOT
                    // be a no_format char. The "else if" clause checks this
                    // and switches it for a blank if it is the case.
                    */

                    if ((no_format_char) && (total_out_count < max_len))
                    {
                        *out_str++ = no_format_char;
                        total_out_count++;
                    }
                    else if (FIND_CHAR_YES(no_format_chars, *line_in_pos))
                    {
                        if (   (room_left > 0) && (total_out_count < max_len))
                        {
                            *out_str++ = ' ';
                            room_left--;
                            total_out_count++;
                        }
                    }
                }
            }

            room_left = width;
            *out_str++ = '\n';
            need_new_line = FALSE;
            /* total_out_count already accounted for. */
        }
        else
        {
            if (    keep_indentation
                 && ((blank_count + extra_indent) != current_indent)
                 && (extra_indent + blank_count < width)
               )
            {
                if (need_new_line)
                {
                    *out_str++ = '\n';
                    total_out_count++;
                }
                current_indent = blank_count + extra_indent;
                need_new_line = FALSE;
                room_left = width;
            }

            trim_end(line_in_pos);

            /*
            // We usually get out of here at the break flagged with "1" below.
            */
            while (total_out_count < max_len)
            {
                int  i;
                char* word_pos;

                i = 0;
                trim_beg( &line_in_pos );
                word_pos = line_in_pos;

                while ((*word_pos != '\0') && (*word_pos != ' '))
                {
                    i++;
                    word_pos++;
                }

                if (i == 0) break;  /* 1 */   /* We are done with this line. */

                while ((i > 0) && (total_out_count < max_len))
                {
                    /*
                    // If we already have some output on this line
                    // (room_left<width, then fit a blank plus word if we can.
                    // Otherwise start a new line, and then fall through to the
                    // case of not having any output on the line.
                    */

                    if (room_left < width)
                    {
                        if (room_left > i)  /* > because need room for ' '. */
                        {
                            *out_str++ = ' ';
                            total_out_count++;
                            room_left--;

                            while ((i > 0) && (total_out_count < max_len))
                            {
                                *out_str++ = *line_in_pos++;
                                i--;
                                room_left--;
                                total_out_count++;
                            }
                        }
                        else
                        {
                            room_left = width;
                            *out_str++ = '\n';
                            need_new_line = FALSE;
                        }
                    }

                    if ((i > 0) && (total_out_count < max_len))
                    {
                        int j=0;

                        /*
                        // Still have somethig to output, but must be at start
                        // of new line.
                        */

                        ASSERT(room_left == width);
                        ASSERT( ! need_new_line );

                        total_out_count++;
                        need_new_line = TRUE;

                        while (    (j<current_indent)
                                && (total_out_count < max_len))
                        {
                            *out_str++ = ' ';
                            j++;
                            total_out_count++;
                            room_left--;
                        }

                        /*
                        // If the character about to become the first character
                        // on the line is one of the special "first chars",
                        // then slip in a blank to make it the second char.
                        */
                        if (    (room_left == width)
                             && (FIND_CHAR_YES(no_format_chars, *line_in_pos))
                           )
                        {
                            if (total_out_count < max_len)
                            {
                                *out_str++ = ' ';
                                room_left--;
                                total_out_count++;
                            }
                        }

                        while (    (room_left > 0)
                                && (i > 0)
                                && (total_out_count < max_len)
                              )
                        {
                            *out_str++ = *line_in_pos++;
                            i--;
                            room_left--;
                            total_out_count++;
                        }
                    }
                }
            }
        }
    }

    if (need_new_line) *out_str++ = '\n';

    *out_str = '\0';

    if (sget_line_res == EOF)
    {
        return NO_ERROR;
    }
    else
    {
        return TRUNCATED;
    }
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

static void free_justify_big_buff(void)
{


    kjb_free(fs_big_buff);

    fs_big_buff = NULL; 
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */





#ifdef __cplusplus
}
#endif

