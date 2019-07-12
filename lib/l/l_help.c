
/* $Id: l_help.c 21520 2017-07-22 15:09:04Z kobus $ */

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

#include "l/l_config.h"
#include "l/l_error.h"
#include "l/l_io.h"
#include "l/l_sys_sig.h"
#include "l/l_sys_term.h"
#include "l/l_string.h"
#include "l/l_justify.h"
#include "l/l_parse.h"
#include "l/l_queue.h"
#include "l/l_help.h"

#ifdef __cplusplus
extern "C" {
#endif

/* -------------------------------------------------------------------------- */

#ifndef DOCUMENT_DIR
#    define DOCUMENT_DIR "doc"
#endif

#ifndef HELP_DIR
#    define HELP_DIR "help"
#endif

#ifndef HELP_FILE_SUFFIX
#    ifdef MS_16_BIT_OS
#        define HELP_FILE_SUFFIX ".hlp"
#    else
#        define HELP_FILE_SUFFIX ".help"
#    endif
#endif

/* -------------------------------------------------------------------------- */

/*
//
//  To make up the strings for static fs_help_on_help_lines, erase the previous
//  ones, edit this text, format to 80 columns, copy it below, select it and
//  do:
//      '<,'>s/\/\/ *\(.*\)/ "\1\\n",/
//  (Remove the missing comma)
//
//  The text below should be separated from the // with four spaces
//
//  ----------------------------------------------------------------------------
//
//    The HELP MENU facility uses two kinds of screens. The first type of screen
//    consists of the help text for a given topic. These may be followed by a
//    the second type of screen which is a menu of additional sub-topics. A
//    sub-topic may be selected from this menu to see the text for the
//    sub-topic, which may be followed by a \"sub-sub-topic\" menu (and so on).
//
//    You may go onto the next page of the given topic text using  the SPACE
//    bar, the return key, or the \"f\" key. The \"b\" key will return you to
//    the preceding page. (On most terminals, the up/down arrow keys can also
//    be used to go back/forward).
//
//    Topics are selected from the topic menu by entering a topic number. In
//    addtion, the return key or SPACE bar can be used to display the
//    \"selected\" topic. The selected topic is marked by a \"*\" and
//    high-lighted (if your terminal supports high-lighting). After a topic is
//    displayed with the return key or SPACE bar, the next topic becomes the
//    \"selected\" topic. It is thus possible to see the entire contents of the
//    help file by simply entering SPACEs, or carriage returns.  If a topic is
//    selected by entering its number, it implicitly becomes the \"selected\"
//    topic.
//
//    On most terminals the \"selected\" topic can by changed for subsequent
//    display by using the up/down arrow keys.  To back-up to the previous
//    topic level, enter the letter \"p\". This returns one back to the
//    previous topic menu.  To exit the help menu facility, enter the letter
//    \"e\".
*/

/* -------------------------------------------------------------------------- */

static const char* fs_help_on_help_lines [ ] =
{
 "The HELP MENU facility uses two kinds of screens. The first type of screen\n",
 "consists of the help text for a given topic. These may be followed by a\n",
 "the second type of screen which is a menu of additional sub-topics. A\n",
 "sub-topic may be selected from this menu to see the text for the\n",
 "sub-topic, which may be followed by a \"sub-sub-topic\" menu (and so on).\n",
 "\n",
 "You may go onto the next page of the given topic text using  the SPACE\n",
 "bar, the return key, or the \"f\" key. The \"b\" key will return you to\n",
 "the preceding page. (On most terminals, the up/down arrow keys can also\n",
 "be used to go back/forward).\n",
 "\n",
 "Topics are selected from the topic menu by entering a topic number. In\n",
 "addtion, the return key or SPACE bar can be used to display the\n",
 "\"selected\" topic. The selected topic is marked by a \"*\" and\n",
 "high-lighted (if your terminal supports high-lighting). After a topic is\n",
 "displayed with the return key or SPACE bar, the next topic becomes the\n",
 "\"selected\" topic. It is thus possible to see the entire contents of the\n",
 "help file by simply entering SPACEs, or carriage returns.  If a topic is\n",
 "selected by entering its number, it implicitly becomes the \"selected\"\n",
 "topic.\n",
 "\n",
 "On most terminals the \"selected\" topic can by changed for subsequent\n",
 "display by using the up/down arrow keys.  To back-up to the previous\n",
 "topic level, enter the letter \"p\". This returns one back to the\n",
 "previous topic menu.  To exit the help menu facility, enter the letter\n",
 "\"e\". \n",
};

static int fs_num_help_on_help_lines = sizeof(fs_help_on_help_lines)/sizeof(char*);
static int   fs_help_topic_level       = 0;
static FILE* fs_help_fp                = NULL;
static Bool  fs_use_default_help_file  = TRUE;

/* -------------------------------------------------------------------------- */

static int set_default_help_file(const char* program_name);

static void close_help_file(void);

static int kjb_help_guts
(
    FILE*       help_fp,
    int         (*remote_get_help_topic_fn)(const char*, char*, size_t ),
    int         (*remote_get_topic_list_fn) (char*, size_t),
    const char* topic_line
);

static Help_request help_topic
(
    FILE*       help_fp,
    int         (*remote_get_help_topic_fn) (const  char*, char*, size_t ),
    const char* topic,
    const char* prev_topic
);

static Help_request do_help_topic
(
    FILE*       help_fp,
    int         (*remote_get_help_topic_fn) (const  char*, char*, size_t ),
    const char* topic,
    const char* prev_topic,
    const char* help_text
);

static Help_request do_help_topic_menu
(
    FILE*       help_fp,
    int         (*remote_get_help_topic_fn) (const  char*, char*, size_t ),
    const char* topic,
    const char* prev_topic,
    const char*       help_text
);

static void print_menu_item
(
    const char* marker,
    int         menu_item,
    char        sub_topics[ MAX_NUM_SUB_TOPICS ][100]
);


static int help_print_title(const char* topic, const char* prev_topic);

static int get_help_topic
(
    FILE*       help_fp,
    int         (*remote_get_help_topic_fn) (const char*, char* , size_t),
    const char* topic,
    char*       help_text,
    size_t      max_help_topic_size
);

static int find_help_topic(FILE* help_fp, const char* topic);

static void list_topics
(
    FILE* help_fp,
    int   (*remote_get_topic_list_fn)( char*, size_t)
);

static int next_help_topic
(
    FILE*  help_fp,
    char*  help_line,
    size_t max_len
);

static Help_request help_topic_query(int prev_page_ok, int next_page_ok);

static Help_request help_menu_query
(
    int* item_ptr,
    int  topic_count,
    int  cur_topic
);

static FILE* open_help_file(const char* root_name);

/* -------------------------------------------------------------------------- */

/* =============================================================================
 *                                 set_help_file
 *
 *
 *
 * -----------------------------------------------------------------------------
*/

int set_help_file(const char* help_file)
{
    FILE* help_fp;


    help_fp = kjb_fopen(help_file, "r");

    if (fs_help_fp == NULL)
    {
        if (help_fp == NULL)
        {
            add_error("Default help file will be used.");
            return ERROR;
        }
        else
        {
            add_cleanup_function(close_help_file);
            fs_help_fp = help_fp;
        }
    }
    else
    {
        if (help_fp == NULL)
        {
            add_error("Help file has not been changed.");
            return ERROR;
        }
        else
        {
            (void)kjb_fclose(fs_help_fp);  /* Ignore return--only reading. */
            fs_help_fp = help_fp;
        }
    }

    fs_use_default_help_file = FALSE;

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 * STATIC
 * -----------------------------------------------------------------------------
*/

static int set_default_help_file(const char* program_name)
{
    static char  previous_program_name[ 100 ] = { '\0' };
    FILE*        help_fp;


    if ( ! fs_use_default_help_file)
    {
        if (fs_help_fp == NULL)
        {
            SET_CANT_HAPPEN_BUG();
            return ERROR;
        }
        else return NO_ERROR;
    }

    if (STRCMP_EQ(program_name, previous_program_name))
    {
        return NO_ERROR;
    }
    else
    {
        BUFF_CPY(previous_program_name, program_name);
    }

    NRE(help_fp = open_help_file(program_name));

    /* Close previous help file. (OK if NULL).   */
    (void)kjb_fclose(fs_help_fp);  /* Ignore return--only reading. */

    fs_help_fp = help_fp;

    add_cleanup_function(close_help_file);

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

static void close_help_file(void)
{

    if (fs_help_fp != NULL)
    {
        (void)kjb_fclose(fs_help_fp);  /* Ignore return--only reading. */
        fs_help_fp = NULL;
    }
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                                   kjb_help
 *
 * Implements a simple help system.
 *
 * This routine implements a simple help system. It uses help files of a
 * special but simple format. The format is, by convention, documented at the
 * begining of each help file. In case no sample help file is available, the
 * format is desribed below.
 *
 * If a help file has been succesfully set with set_help_file(), then that help
 * file is used. If not, then a default help file is looked for on the basis of
 * the program name argument. In this case, the current directory, the users
 * help directory, and a shared help directory are searched, in that order for
 * a file named <program_name>.help. Currently, the help directory is
 * "~/doc/help", and the shared help directory is "~iis/doc/help", but these
 * locations are easily changed without forcing an update of the documentation.
 *
 * The routine also takes a help topic as a argument. If it is null, or
 * empty, then the top of the help topic tree is used, which by convention is
 * equivalent to "introduction" and <program_name>. If the topic is the special
 * topic "topics", then a list of topics is printed. Finally, if the topic is
 * the special topic "help", then help on using help is provided.
 *
 * If the high level input module "get_command_text()" is being used for input,
 * then help is called as part of pre-processing, and the code calling
 * get_command_text() will never see a command "h" or "help". Thus there is no
 * need to implement a user help command in this case. However, kjb_help() can
 * be additionally called in other contexts with no ill affect.
 *
 * Returns:
 *     ERROR if there are problems and NO_ERROR otherwise. If there is no help
 *     on the requested topic, then a message is printed to that effect, but it
 *     is not considered an error. One possible cause of error is if the help
 *     file is not available.
 *
 * Note:
 *    It is a waste to implement a user help command (as well as many other
 *    features) if get_command_text() is being used to get input.
 *
 * File:
 *     The help file format is as follows.
 *
 *     Formatting is largely under the control of the first column.
 *
 *     Lines with # in the first column are comment lines
 *
 *     Lines starting with  /  indicate the start of a help topic.  Whatever
 *     follows the / is the topic title, up to another /, which indicates the
 *     start of an alternate topic title.
 *
 *     There must be an "introduction" topic, which is the top of the topic
 *     tree. If the user does not specify a help topic, then this one is used.
 *     By convention, the program name is a synomym, and thus is an alternate
 *     name.
 *
 *     Lines starting with  &  indicate a "subroutine call" to the help topic
 *     which follows the &. When the help routine gets to one of these lines, a
 *     menu is created for all the & topics.
 *
 *     Text is left justified at the indent level in this file. Thus a new line
 *     is started whenever the indent changes. This file thus specifies the
 *     left indent, but not the number of lines. Since the help reader does
 *     line justification, formatting in the help file is not critical.
 *
 *     Lines starting with | are protected from the formatting described in the
 *     preceeding paragraph. This is necessary for making tables. The "|" is
 *     not printed.
 *
 *     Phrases within + are highlighted where screen support allows, and become
 *     underlined (or bold) in the man pages. To get a double "+" use two. This
 *     is proabably not the best choice, adn could change.
 *
 *     Lines starting with @ are used to restrice the lines that follow to a
 *     specific processing routine. If the @ is followed immediately by "help",
 *     then the subsequent lines are seen by "help", but not other readers of
 *     the file, such as the man page generator. Use "@all" to go back to
 *     normal mode.
 *
 *     Lines beginning with % are used to restrict the lines that follow to a
 *     specific system. The % should be followed by one or more system names
 *     (unix, dos), or "all" to reset back to lines relevant to all systems.
 *
 * Related:
 *     set_help_file(3), get_command_text(3)
 *
 * Index: help
 *
 * -----------------------------------------------------------------------------
*/

int kjb_help(const char* program_name, const char* topic_line)
{
    int res;


    kjb_clear_error();

    ERE(set_default_help_file(program_name));

    if (fs_help_fp == NULL)
    {
        insert_error("Help file is not available.");
        return ERROR;
    }

    rewind(fs_help_fp);

    if (topic_line == NULL) topic_line = "";

    res = kjb_help_guts(fs_help_fp,
                        (int (*) (const char*, char*, size_t)) NULL,
                        (int (*) (char*, size_t)) NULL,
                        topic_line);
    return res;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

static int kjb_help_guts(FILE* help_fp,
                         int (*remote_get_help_topic_fn)(const char *, char *, size_t),
                         int (*remote_get_topic_list_fn) (char *, size_t),
                         const char* topic_line)
{
    IMPORT volatile Bool term_line_wrap_flag;
    IMPORT volatile int recorded_signal;
    char                topic[ 200 ];
    char                prev_topic[ 200 ];
    Bool                save_term_line_wrap_flag;
    int                 res;
    int                 interactive;
    Help_request        help_res;


    res = NO_ERROR;

    if ((help_fp == NULL) &&
        ((remote_get_help_topic_fn == NULL) ||
         (remote_get_topic_list_fn == NULL)))
    {
        set_error("Help file is not available");
        return ERROR;
    }

    const_trim_beg(&topic_line);
    BUFF_CPY(topic, topic_line);
    remove_duplicate_chars(topic, ' ');
    trim_end(topic);

    if (topic[ 0 ] == '\0')
    {
        BUFF_CPY(topic, "introduction");
    }
    else extended_uc_lc(topic);

    if (STRCMP_EQ(topic, "topics"))
    {
        list_topics(help_fp, remote_get_topic_list_fn);
    }
    else
    {
        fs_help_topic_level = 0;
        kjb_disable_paging();
        prev_topic [ 0 ] = '\0';

        interactive = is_interactive();

        save_term_line_wrap_flag = term_line_wrap_flag;

        if (interactive)
        {
#if 0 /* was ifdef TRY_WITHOUT */
            if (block_user_signals() == ERROR)
            {
                kjb_restore_paging();
                return ERROR;
            }
#endif

            if (set_atn_trap(confirm_exit_sig_fn, DONT_RESTART_AFTER_SIGNAL)
                == ERROR)
            {
                reset_signals();
                kjb_restore_paging();
                set_bug("Unable to set raw mode in kjb_help_guts.");
                return ERROR;
            }

            if (term_set_raw_mode_with_no_echo() == ERROR)
            {
                reset_signals();
                unset_atn_trap();
                kjb_restore_paging();
                set_bug("Unable to set raw mode in kjb_help_guts.");
                return ERROR;
            }

            term_line_wrap_flag = FALSE;
        }

        recorded_signal = NOT_SET;

        help_res = help_topic(help_fp, remote_get_help_topic_fn,
                              topic, prev_topic);

        term_line_wrap_flag = save_term_line_wrap_flag;

        if (interactive)
        {
            term_reset();
            unset_atn_trap();
#if 0 /* was ifdef TRY_WITHOUT */
            reset_signals();
#endif
        }

        if (help_res == HELP_ERROR)
        {
            res = ERROR;
        }

        kjb_restore_paging();
    }

    return res;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

static Help_request help_topic
(
    FILE* help_fp,
    int (*remote_get_help_topic_fn) (const char *, char *, size_t),
    const char* topic,
    const char* prev_topic
)
{
    char         help_text[ MAX_HELP_TOPIC_LEN ];
    int          read_res;
    Help_request do_topic_res;


    read_res = get_help_topic(help_fp, remote_get_help_topic_fn,
                              topic, help_text, sizeof(help_text));

    /*
       Special case: If there was no help on help in the help file,
       then we will use the hard coded version. (If there is help on
       help in the help file, then we want to use that version).
    */
    if ((read_res == NOT_FOUND) &&
        ((STRCMP_EQ(topic, "help menu")) || (STRCMP_EQ(topic, "help"))))
    {
        int i;


        help_text[ 0 ] = '\0';

        for (i=0; i<fs_num_help_on_help_lines; i++)
        {
            BUFF_CAT(help_text, fs_help_on_help_lines[ i ]);
        }
    }
    else if (read_res == ERROR)
    {
        kjb_fprintf(stderr, "Error reading help file. ");
        kjb_fprintf(stderr, "Command aborted.\n");
        return EXIT_HELP;
    }
    else if (read_res == NOT_FOUND)
    {
        pso("No help found for \"%s\"\n", topic);
        return DONT_EXIT_HELP;
    }

    fs_help_topic_level++;

    do_topic_res = REPEAT_HELP_TOPIC;

    while (do_topic_res == REPEAT_HELP_TOPIC)
    {
        do_topic_res = do_help_topic(help_fp, remote_get_help_topic_fn,
                                     topic, prev_topic, help_text);
    }

    /*
        Doing the previous topic is "repeating" the topic of the
        level we are about to return to.
    */
    if (do_topic_res == PREVIOUS_HELP_TOPIC)
    {
        do_topic_res=REPEAT_HELP_TOPIC;
    }
    /*
        Just in case there is an error in the help file.
    */
    unset_high_light(stdout);

    fs_help_topic_level--;

    return do_topic_res;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 * STATIC                      do_help_topic
 * -----------------------------------------------------------------------------
*/

static Help_request do_help_topic
(
    FILE* help_fp,
    int (*remote_get_help_topic_fn)(const char *, char *, size_t),
    const char* topic,
    const char* prev_topic,
    const char* help_text
)
{
    IMPORT volatile int kjb_tty_rows;
    IMPORT volatile int kjb_tty_cols;
    static char         no_justify_chars [ ] =
                            { HELP_FILE_COMMENT_LINE_CHAR,
                              DONT_FORMAT_CHAR,
                              CONTINUE_WITH_HELP_TOPIC_CHAR,
                              SYSTEM_DEPENDENT_HELP_LINE_CHAR,
                              DOCUMENT_TYPE_ID_CHAR,
                              '\0'
                             };
    char                justified_help_text[ MAX_HELP_TOPIC_LEN ];
    int                 justify_res;
    Queue_element*      page_pos_stack         = NULL;
    Queue_element*      page_pos_stack_elem;
    Queue_element*      next_page_pos_stack_elem;
    char*               help_text_pos;
    char*               help_text_page_beg_pos;
    char*               save_help_text_pos;
    char                help_line[ 2000 ];
    char*               help_line_pos;
    char                system_name[ 200 ];
    char                doc_type[ 200 ];
    Bool                this_system = TRUE;
    Bool                this_doc_type = TRUE;
    int                 read_res;
    int                 find_res;
    Bool                line_finished;
    Help_request        query_res;
    Help_request        help_res;
    int                 count;
    Bool                prev_page_ok;
    Bool                next_page_ok;
    Bool                output_to_terminal;
    Bool                menu_item;
    int                 chunk_len;
    int                 char_count;
    Help_request        result;
    int                 save_tty_cols = kjb_tty_cols;
    int                 initial_tty_cols = kjb_tty_cols;
    char*               save_initial_help_pos;


    justify_res = left_justify(help_text, 
                               kjb_tty_cols,
                               0,    /* No indent level to turn off justify */
                               no_justify_chars,
                               TRUE,    /* Keep indent level */
                               0,    /* No extra indent */
                               justified_help_text,
                               sizeof(justified_help_text));

    if (justify_res == ERROR)
    {
        set_bug("Justify of help_text failed.");
        return EXIT_HELP;
    }

    help_text_pos = justified_help_text;
    save_initial_help_pos = justified_help_text;

    if (kjb_isatty(fileno(stdout))) output_to_terminal = TRUE;
    else output_to_terminal = FALSE;

    /*CONSTCOND*/
    while (TRUE)
    {
        if (kjb_tty_rows < 10)
        {
            set_error(
                "Insufficent room for help page. Perhaps window is too small?");
        }

        help_text_page_beg_pos = help_text_pos;
        save_help_text_pos = help_text_pos;

        count = 0;
        help_line[ 0 ] = '\0';
        read_res = NO_ERROR;

        while ((read_res >= 0) && (help_line[ 0 ] == '\0'))
        {
            read_res = sget_line(&help_text_pos, help_line, sizeof(help_line));
        }

        /*
           This code unfortunately will not write the title if the help text
           is null. Rather than risk breaking other things by fixing it, we
           assume that if the help text as read by "get_help_topic" is null,
           then it is fixed (replaced with a blank line).

           (But it is NOT fixed in get_help_topic? Should it be ? !!!);

           CHECK and FIX
        */
        if (read_res == EOF)
        {
            while (count < kjb_tty_rows-4)
            {
                count++;
                if (output_to_terminal) pso("\n");
            }
            next_page_ok = FALSE;

            if (help_text_page_beg_pos == justified_help_text)
            {
                prev_page_ok = FALSE;
            }
            else prev_page_ok = TRUE;

            query_res = help_topic_query(prev_page_ok, next_page_ok);
        }
        else if (help_line[ 0 ] == CONTINUE_WITH_HELP_TOPIC_CHAR)
        {
            query_res = do_help_topic_menu(help_fp,
                                           remote_get_help_topic_fn,
                                           topic, prev_topic,
                                           help_text_page_beg_pos);
        }
        else
        {
           menu_item = FALSE;

           help_print_title(topic, prev_topic);

           while (((output_to_terminal) && (count < kjb_tty_rows-4)) ||
                  (( ! output_to_terminal) &&
                   (read_res != EOF) && ( ! menu_item)))
           {
               help_line_pos = help_line;

               if (*help_line_pos == HELP_FILE_COMMENT_LINE_CHAR)
               {
                   /*EMPTY*/
                   ; /* Do nothing */
               }
               else if (*help_line_pos == CONTINUE_WITH_HELP_TOPIC_CHAR)
               {
                   menu_item = TRUE;

                   while (count < kjb_tty_rows-4)
                   {
                       count++;
                       if (output_to_terminal) pso("\n");
                   }
                   help_text_pos = save_help_text_pos;
               }
               else if (*help_line_pos == SYSTEM_DEPENDENT_HELP_LINE_CHAR)
               {
                   help_line_pos++;

                   this_system = FALSE;

                   while (BUFF_GET_TOKEN_OK(&help_line_pos, system_name))
                   {
                       extended_uc_lc(system_name);

                       if ((STRCMP_EQ(system_name, "all")) ||
                          (STRCMP_EQ(system_name, SYSTEM_NAME)) ||
                          (STRCMP_EQ(system_name, SYSTEM_NAME_TWO)))
                      {
                          this_system = TRUE;
                      }
                   }
               }
               else if (*help_line_pos == DOCUMENT_TYPE_ID_CHAR)
               {
                   help_line_pos++;

                   this_doc_type = FALSE;

                   while (BUFF_GET_TOKEN_OK(&help_line_pos, doc_type))
                   {
                       if ((STRCMP_EQ(doc_type, "all")) ||
                           (STRCMP_EQ(doc_type, "help")))
                      {
                          this_doc_type = TRUE;
                      }
                   }
               }
               else if (( ! this_system ) || (! this_doc_type))
               {
                   /*EMPTY*/
                   ; /* Do nothing */
               }
               else
               {
                   if (*help_line_pos == DONT_FORMAT_CHAR)
                   {
#if 0 /* was ifdef HOW_IT_WAS_BEFORE_MAY_30_2004 */
                       *help_line_pos = ' ';
#endif
                       help_line_pos++;
                   }

                   line_finished = FALSE;
                   /*
                   // char_count = kjb_tty_cols - 1;
                   */
                   char_count = kjb_tty_cols;  /* Changed the above, July, 96 */

                   while ( ! line_finished)
                   {
                       find_res = find_char(help_line_pos, HIGH_LIGHT_CHAR);

                       if (find_res != CHARACTER_NOT_IN_STRING)
                       {
                           *(help_line_pos+find_res - 1) = '\0';

                           if (char_count > 0)
                           {
                               chunk_len = strlen(help_line_pos);

                               if (char_count < chunk_len)
                               {
                                   *(help_line_pos+char_count) = '\0';
                               }

                               kjb_fputs(stdout, help_line_pos);
                               char_count -= chunk_len;
                               help_line_pos += find_res;

                               if (*help_line_pos != HIGH_LIGHT_CHAR)
                               {
                                   toggle_high_light(stdout);
                               }
                               else
                               {
                                   kjb_putc(HIGH_LIGHT_CHAR);
                                   help_line_pos++;
                                   char_count--;
                               }
                           }
                       }
                       else
                       {
                           line_finished = TRUE;

                           if (char_count > 0)
                           {
                               *(help_line_pos+char_count) = '\0';

                               kjb_fputs(stdout, help_line_pos);
                               pso("\n");
                           }
                       }
                   }
                   count++;
               }

               if (    (    (output_to_terminal) && (count < kjb_tty_rows-4))
                    || (    ( ! output_to_terminal)
                         && (read_res != EOF)
                         && ( ! menu_item)
                       )
                   )
               {
                   /* Read the next line in the input.   */

                   save_help_text_pos = help_text_pos;

                   read_res = sget_line(&help_text_pos,
                                        help_line, sizeof(help_line));
                   if (read_res == EOF)
                   {
                       while (count < kjb_tty_rows-4)
                       {
                           count++;
                           if (output_to_terminal) pso("\n");
                       }
                   }
               }
           }

           pso("\n");

           if (read_res == EOF) next_page_ok = FALSE;
           else next_page_ok = TRUE;

           if (help_text_page_beg_pos == justified_help_text)
           {
               prev_page_ok = FALSE;
           }
           else prev_page_ok = TRUE;

           query_res = help_topic_query(prev_page_ok, next_page_ok);
       }

       if ((query_res == EXIT_HELP) || (query_res == HELP_ERROR))
       {
           result = query_res;
           break;
       }

       if (query_res == HELP_ON_HELP)
       {
           help_res = help_topic(help_fp, remote_get_help_topic_fn,
                                 "help menu", topic);

           if ((help_res == EXIT_HELP) || (help_res == HELP_ERROR))
           {
               result = help_res;
               break;
           }
           help_text_pos = help_text_page_beg_pos;
       }
       else if (query_res == PREVIOUS_HELP_TOPIC)
       {
           result = PREVIOUS_HELP_TOPIC;
           break;
       }
       else if (query_res == REDRAW_CURRENT_HELP_PAGE)
       {
           help_text_pos = help_text_page_beg_pos;
       }
       else if (query_res == FORWARD_HELP_PAGE)
       {
           if (read_res == EOF)
           {
               result = DONT_EXIT_HELP;
               break;
           }
           count = 0;
           insert_into_queue(&page_pos_stack, (Queue_element**)NULL,
                             (void *)help_text_page_beg_pos);
       }
       else if (query_res == BACK_HELP_PAGE)
       {
           count = 0;

           if (page_pos_stack != NULL)
           {
               page_pos_stack_elem = remove_first_element(&page_pos_stack,
                                                         (Queue_element**)NULL);
               help_text_pos = (char*)(page_pos_stack_elem->contents);
               kjb_free( page_pos_stack_elem);
           }
           else
           {
               help_text_pos = help_text_page_beg_pos;
           }

           /*
           // If we go back, then we may be getting into some more unjustified
           // text, even if we did some rejustification. Thus we invalidate the
           // justification every time we go back, if the page size changed
           // at all while processing this topic.
           */
           if (save_tty_cols != initial_tty_cols)
           {
               save_tty_cols = -1;
           }
       }

        if (query_res == DONT_EXIT_HELP)
        {
            result = DONT_EXIT_HELP;
            break;
        }

        /*
        // IF we are here, then we are about to continue outputing some
        // part of the current help page. Other possibilities have lead to
        // "break"s. If we need to rejustify, then we can only do so from the
        // current position, as we may have pointers into previous parts of
        // the help page string.
        */
        if (kjb_tty_cols != save_tty_cols)
        {
            char  re_justified_help_text[ MAX_HELP_TOPIC_LEN ];
            size_t   offset = help_text_pos - save_initial_help_pos;


            ASSERT((size_t)offset <= sizeof(justified_help_text));

            if (left_justify(help_text_pos, kjb_tty_cols, 0,
                             no_justify_chars, TRUE, 0,
                             re_justified_help_text,
                             sizeof(re_justified_help_text))
                == ERROR)
            {
                set_bug("Justify of help_text failed.");
                return EXIT_HELP;
            }

            kjb_strncpy(help_text_pos, re_justified_help_text,
                        (int)(sizeof(justified_help_text) - offset));

            save_tty_cols = kjb_tty_cols;
        }
    }

    page_pos_stack_elem = page_pos_stack;

    while (page_pos_stack_elem != NULL)
    {
        next_page_pos_stack_elem = page_pos_stack_elem->next;
        kjb_free(page_pos_stack_elem);
        page_pos_stack_elem = next_page_pos_stack_elem;
    }

    return result;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 * STATIC                      do_help_topic_menu
 * -----------------------------------------------------------------------------
*/

static Help_request do_help_topic_menu
(
    FILE* help_fp,
    int (*remote_get_help_topic_fn)(const char *, char *, size_t),
    const char* topic,
    const char* prev_topic,
    const char* help_text
)
{
    IMPORT volatile int kjb_tty_rows;
    char                sub_topics[ MAX_NUM_SUB_TOPICS ][ 100 ];
    const char*         help_text_pos;
    char                help_line[ 2000 ];
    char*               help_line_pos;
    char                sub_topic[ 100 ];
    int                 read_res;
    Help_request        help_res;
    Help_request        query_res;
    int                 line_count;
    int                 topic_count;
    int                 cur_menu_item;
    Bool                rewrite_screen;
    Bool                output_to_terminal;


    cur_menu_item = 1;

    if (kjb_isatty(fileno(stdout))) output_to_terminal = TRUE;
    else output_to_terminal = FALSE;

    /*CONSTCOND*/
    while (TRUE)
    {
        help_text_pos = help_text;

        help_print_title(topic, prev_topic);

        pso("     Additional Help Topics. Use topic number to select one.\n");
        pso("     Or use a SPACE or RETURN to see current topic ");
        pso("(marked with a \"*\").\n\n");

        topic_count = 0;
        line_count = 0;

        read_res = const_sget_line(&help_text_pos,
                                   help_line, sizeof(help_line));

        while (read_res != EOF)
        {
            help_line_pos = help_line;

            if (*help_line_pos == CONTINUE_WITH_HELP_TOPIC_CHAR)
            {
                if (topic_count < MAX_NUM_SUB_TOPICS - 1)
                {
                    help_line_pos++;
                    trim_beg(&help_line_pos);
                    BUFF_CPY(sub_topic, help_line_pos);
                    trim_end(sub_topic);
                    extended_uc_lc(sub_topic);
                    remove_duplicate_chars(sub_topic, ' ');

                    BUFF_CPY(sub_topics[topic_count], sub_topic);
                    topic_count++;

                    if (topic_count == cur_menu_item)
                    {
                        set_high_light(stdout);
                        print_menu_item("*", topic_count, sub_topics);
                        unset_high_light(stdout);
                    }
                    else
                    {
                        print_menu_item(" ", topic_count, sub_topics);
                    }
                    pso("\n");
                }
                line_count++;
            }
            read_res = const_sget_line(&help_text_pos,
                                       help_line, sizeof(help_line));
        }

        while (line_count < kjb_tty_rows - 7)
        {
            line_count++;
            if (output_to_terminal) pso("\n");
        }

        pso("\n");

        rewrite_screen = FALSE;

        while ( ! rewrite_screen)
        {
            int menu_item = NOT_SET;

            query_res = help_menu_query(&menu_item, topic_count, cur_menu_item);

            rewrite_screen = TRUE;

            if (query_res == HELP_MENU_CHOICE)
            {
                if (menu_item == NOT_SET)
                {
                    SET_CANT_HAPPEN_BUG();
                    return HELP_ERROR;
                }

                cur_menu_item = menu_item;

                help_res = help_topic(help_fp, remote_get_help_topic_fn,
                                      sub_topics[menu_item-1],
                                      topic);

                if ((help_res == EXIT_HELP) || (help_res == HELP_ERROR))
                {
                    return help_res;
                }
                else if (help_res == PREVIOUS_HELP_TOPIC)
                {
                    return DONT_EXIT_HELP;
                }
            }
            else if (query_res == CONTINUE_WITH_TOPIC)
            {
                if (cur_menu_item > topic_count)
                {
                    return DONT_EXIT_HELP;
                }

                help_res = help_topic(help_fp, remote_get_help_topic_fn,
                                      sub_topics[ cur_menu_item - 1 ],
                                      topic);

                if ((help_res == EXIT_HELP) || (help_res == HELP_ERROR))
                {
                    return help_res;
                }
                else if (help_res == PREVIOUS_HELP_TOPIC)
                {
                    return DONT_EXIT_HELP;
                }
                else
                {
                    cur_menu_item++;
                }

                if ((cur_menu_item > topic_count) &&
                    ( ! output_to_terminal))
                {
                    rewrite_screen = FALSE;
                }
            }
            else if (query_res == BACK_HELP_SUB_TOPIC)
            {
                if (output_to_terminal)
                {
                    move_cursor_up(kjb_tty_rows-4-cur_menu_item);
                }

                cur_menu_item--;

                if (output_to_terminal)
                {
                    term_blank_out_line();
                    set_high_light(stdout);
                }

                print_menu_item("*", cur_menu_item, sub_topics);

                if (output_to_terminal)
                {
                    unset_high_light(stdout);
                    move_cursor_down(1);
                }

                /*
                // Looks like it can't happen, but it can.
                 */
                if (cur_menu_item < topic_count)
                {
                    if (output_to_terminal)
                    {
                        term_blank_out_line();
                    }

                    print_menu_item(" ", cur_menu_item + 1, sub_topics);
                }

                if (output_to_terminal)
                {
                    move_cursor_down(kjb_tty_rows-6-cur_menu_item);
                }

                rewrite_screen = FALSE;
            }
            else if (query_res == FORWARD_HELP_SUB_TOPIC)
            {
                if (output_to_terminal)
                {
                    move_cursor_up(kjb_tty_rows-6-cur_menu_item);
                }

                cur_menu_item++;

                if (output_to_terminal)
                {
                    term_blank_out_line();
                    set_high_light(stdout);
                }

                print_menu_item("*", cur_menu_item, sub_topics);

                if (output_to_terminal)
                {
                    unset_high_light(stdout);
                    move_cursor_up(1);
                    term_blank_out_line();
                }

                print_menu_item(" ", cur_menu_item - 1, sub_topics);

                if (output_to_terminal)
                {
                    move_cursor_down(kjb_tty_rows-4-cur_menu_item);
                }

                rewrite_screen = FALSE;
            }
            else if (query_res == HELP_ON_HELP)
            {
                help_res = help_topic(help_fp, remote_get_help_topic_fn,
                                      "help menu", topic);

                if ((help_res == EXIT_HELP) || (help_res == HELP_ERROR))
                {
                    return help_res;
                }
            }
            else if (query_res == REWRITE_HELP_MENU)
            {
                /*EMPTY*/
                ; /* DO NOTHING */
            }
            else 
            { 
                return query_res;
            }
        }
    }
    /*NOTREACHED*/
#ifdef __GNUC__
    return HELP_ERROR;
#endif
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

static void print_menu_item
(
    const char* marker,
    int         menu_item,
    char        sub_topics[ MAX_NUM_SUB_TOPICS ][ 100 ]
)
{


    if (menu_item < 10)
    {
        pso("  %s  %d: %s\r", marker, menu_item, sub_topics[menu_item-1]);
    }
    else
    {
        pso("  %s  %c: %s\r", marker, (int)('A'+menu_item-10),
            sub_topics[menu_item-1]);
    }
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

static int help_print_title(const char* topic, const char* prev_topic)
{
    IMPORT volatile int kjb_tty_cols;
    int                 num_dashes;
    char                uc_topic[ 200 ];
    int                 topic_len;
    int                 prev_topic_len  = 0;


    EXTENDED_UC_BUFF_CPY(uc_topic, topic);

    topic_len = strlen(topic);

    if (*prev_topic != '\0')
    {
        prev_topic_len = strlen(prev_topic) + 2;
    }

    num_dashes = (kjb_tty_cols - topic_len - prev_topic_len - 4)/2;

    rep_print(stdout, '-', num_dashes);

    pso("  ");

    if (*prev_topic != '\0')
    {
        kjb_fputs(stdout, prev_topic);
        pso(": ");
    }

    set_high_light(stdout);
    kjb_fputs(stdout, uc_topic);
    unset_high_light(stdout);
    pso("  ");

    num_dashes = kjb_tty_cols - topic_len - prev_topic_len - 4 - num_dashes;

    rep_print(stdout, '-', num_dashes);
    pso("\n\n");

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

static int get_help_topic
(
    FILE*       help_fp,
    int         (*remote_get_help_topic_fn) (const char *, char *, size_t),
    const char* topic,
    char*       help_text,
    size_t      max_help_topic_size
)
{
    int get_res;


    get_res = local_get_help_topic(help_fp, topic, help_text,
                                   max_help_topic_size);

    if (get_res == ERROR)
    {
        if (remote_get_help_topic_fn != NULL)
        {
            get_res = (*remote_get_help_topic_fn)(topic, help_text,
                                                  max_help_topic_size);
        }
    }

    return get_res;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

int local_get_help_topic(FILE* help_fp, const char* topic, char* help_text,
                         size_t max_help_topic_size)
{
    char* help_text_pos;
    char  help_line[ 2000 ];
    int   read_res;
    size_t total_char_count;


    if (help_fp == NULL) return ERROR;

    help_text_pos = help_text;

    read_res = find_help_topic(help_fp, topic);

    if (read_res != FOUND) return read_res;

    total_char_count = 0;

    ERE(read_res = BUFF_FGET_LINE(help_fp, help_line));

    if ((read_res == EOF) || (help_line[ 0 ] == HELP_TOPIC_CHAR))
    {
        /*
           Null help test. Add dummy line so that title gets printed.
        */
        strcpy(help_text, "\n");
    }

    while ((read_res != EOF) &&
           (help_line[ 0 ] != HELP_TOPIC_CHAR))
    {
        total_char_count += strlen(help_line);
        total_char_count++;  /* For end of line */

        if (total_char_count > max_help_topic_size - ROOM_FOR_NULL)
        {
            set_bug("Help buffer too small.");
            return ERROR;
        }
        else
        {
            str_build(&help_text_pos, help_line);
            str_build(&help_text_pos, "\n");
        }

        ERE(read_res = BUFF_FGET_LINE(help_fp, help_line));
    }

    return FOUND;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

static int find_help_topic(FILE* help_fp, const char* topic)
{
    char  help_line[ 2000 ];
    char* help_line_pos;
    char  test_topic[ 200 ];
    int   read_res;


    rewind(help_fp);

    ERE(read_res = BUFF_FGET_LINE(help_fp, help_line));

    while (read_res != EOF)
    {
        help_line_pos = help_line;
        trim_beg(&help_line_pos);

        if (*help_line_pos == HELP_TOPIC_CHAR)
        {
            help_line_pos++;
            trim_beg(&help_line_pos);

            while (BUFF_GEN_GET_TOKEN_OK(&help_line_pos, test_topic, "/"))
            {
                extended_uc_lc(test_topic);
                remove_duplicate_chars(test_topic, ' ');
                trim_end(test_topic);

                if (STRCMP_EQ(test_topic, topic)) return FOUND;

                trim_beg(&help_line_pos);
            }
        }

        ERE(read_res = BUFF_FGET_LINE(help_fp, help_line));
    }

     return NOT_FOUND;
 }

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

static void list_topics(FILE* help_fp,
                        int (*remote_get_topic_list_fn)(char *, size_t))
{
    int  res;
    char topic_list[ MAX_HELP_TOPIC_LIST_LEN ];


    res = local_get_topic_list(help_fp, topic_list, sizeof(topic_list));

    if (res == ERROR)
    {
        if (remote_get_topic_list_fn != NULL)
        {
            res = (*remote_get_topic_list_fn)(topic_list, sizeof(topic_list));
        }
    }

    if (res == ERROR)
    {
        kjb_fprintf(stderr, "Help is not available\n");
    }
    else
    {
        kjb_fputs(stdout, topic_list);
    }
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

int local_get_topic_list(FILE* help_fp, char* topic_list,
                         size_t max_topic_list_size)
{
    char  help_line[ 2000 ];
    char* help_line_pos;
    char* topic_list_pos;
    char  topic[ 200 ];
    char  topic_synonym[ 200 ];
    char* topic_pos;
    char* topic_synonym_pos;
    int   res;
    size_t char_count;


    if (help_fp == NULL) return ERROR;

    topic_list_pos = topic_list;

    char_count = 1;  /* strlen("\n") */
    str_build(&topic_list_pos, "\n");

    ERE(res = next_help_topic(help_fp, help_line, sizeof(help_line)));

    while (res != EOF)
    {
        help_line_pos = help_line;

        help_line_pos++;

        if (BUFF_GEN_GET_TOKEN_OK(&help_line_pos, topic, "/"))
        {
            topic_pos = topic;
            trim_beg(&topic_pos);
            trim_end(topic_pos);

            extended_uc_lc(topic_pos);

            char_count += strlen(topic_pos);
            if (char_count > max_topic_list_size - 1)
            {
                set_bug("Topic list buffer too small.");
                return ERROR;
            }
            str_build(&topic_list_pos, topic_pos);
        }

        if (BUFF_GEN_GET_TOKEN_OK(&help_line_pos, topic_synonym, "/"))
        {
            topic_synonym_pos = topic_synonym;
            trim_beg(&topic_synonym_pos);
            trim_end(topic_synonym_pos);

            extended_uc_lc(topic_synonym_pos);
            char_count += strlen(topic_synonym_pos) + strlen(" (");
            if (char_count > max_topic_list_size - 1)
            {
                set_bug("Topic list buffer too small.");
                return ERROR;
            }

            str_build(&topic_list_pos, " (");
            str_build(&topic_list_pos, topic_synonym_pos);

            while (BUFF_GEN_GET_TOKEN_OK(&help_line_pos, topic_synonym, "/"))
            {
                topic_synonym_pos = topic_synonym;
                trim_beg(&topic_synonym_pos);
                trim_end(topic_synonym_pos);

                extended_uc_lc(topic_synonym_pos);
                char_count += strlen(topic_synonym_pos) + strlen(" , ");
                if (char_count > max_topic_list_size - 1)
                {
                    set_bug("Topic list buffer too small.");
                    return ERROR;
                }

                str_build(&topic_list_pos, ", ");
                str_build(&topic_list_pos, topic_synonym_pos);
            }

            char_count += 1;   /*  strlen(")")  */

            if (char_count > max_topic_list_size - 1)
            {
                set_bug("Topic list buffer too small.");
                return ERROR;
            }

            str_build(&topic_list_pos, ")");
        }

        char_count += 1;  /*  strlen("\n")  */

        if (char_count > max_topic_list_size - 1)
        {
            set_bug("Topic list buffer too small.");
            return ERROR;
        }

        str_build(&topic_list_pos, "\n");

        res = next_help_topic(help_fp, help_line, sizeof(help_line));
    }

    char_count += 1;  /*  strlen("\n") */

    if (char_count > max_topic_list_size - 1)
    {
        set_bug("Topic list buffer too small.");
        return ERROR;
    }

    str_build(&topic_list_pos, "\n");

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

static int next_help_topic(FILE* help_fp, char* help_line, size_t max_len)
{
    int   res;
    char* help_line_pos;

    /*CONSTCOND*/
    while ( TRUE )
    {
        res = fget_line(help_fp, help_line, max_len);

        if ((res == EOF) || (res == ERROR))
        {
            return res;
        }

        help_line_pos = help_line;

        if (*help_line_pos == HELP_TOPIC_CHAR)
        {
            return res;
        }
    }

    /*NOTREACHED*/
#ifdef __GNUC__
    return HELP_ERROR;
#endif
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

static Help_request help_topic_query(int prev_page_ok, int next_page_ok)
{
#ifdef UNIX
    IMPORT volatile int recorded_signal;
#endif 
    int                 read_result;
    Help_request        query_result;
    Bool                good_input;


    if ( ! kjb_isatty(fileno(stdout)))
    {
        return FORWARD_HELP_PAGE;
    }

    good_input = FALSE;
    query_result = HELP_ERROR;

    while (! good_input )
    {
        put_prompt("Enter ");

        if (next_page_ok)
        {
            set_high_light(stdout);
            put_prompt("f");
            unset_high_light(stdout);
            put_prompt(" to go forward, ");
        }
        if (prev_page_ok)
        {
            set_high_light(stdout);
            put_prompt("b");
            unset_high_light(stdout);
            put_prompt(" to go back, ");
        }
        if (fs_help_topic_level > 1)
        {
            set_high_light(stdout);
            put_prompt("p");
            unset_high_light(stdout);
            put_prompt(" for previous, ");
        }

        set_high_light(stdout);
        put_prompt("q");
        unset_high_light(stdout);
        put_prompt(" to quit: ");

        if (is_interactive())
        {
            read_result = enhanced_term_getc();
        }
        else
        {
            read_result = fgetc(stdin);
        }

        term_blank_out_line();

        if (read_result == ERROR)
        {
            good_input = TRUE;
            query_result = HELP_ERROR;
        }
        else if (read_result == EOF)
        {
            good_input = TRUE;
            query_result = EXIT_HELP;
        }
        else
        {
            if ((read_result == '\n') || (read_result == ' '))
            {
                 good_input = TRUE;
                 query_result = FORWARD_HELP_PAGE;
             }
            else if ((read_result == 'f') || (read_result == DOWN_ARROW))
            {
                if ( ! next_page_ok)
                {
                    set_high_light(stdout);
                    term_puts("Last page. ");
                    unset_high_light(stdout);
                }
                 else
                 {
                     good_input = TRUE;
                     query_result = FORWARD_HELP_PAGE;
                 }
             }
            else if ((read_result == 'b') || (read_result == UP_ARROW))
            {
                if ( ! prev_page_ok)
                {
                    set_high_light(stdout);
                    term_puts("First page. ");
                    unset_high_light(stdout);
                }
                 else
                 {
                     good_input = TRUE;
                     query_result = BACK_HELP_PAGE;
                 }
             }
            else if (read_result == 'h')
            {
                 good_input = TRUE;
                 query_result = HELP_ON_HELP;
             }
            else if ((read_result == 'e') || (read_result == 'q'))
            {
                 good_input = TRUE;
                 query_result = EXIT_HELP;
             }
            else if (read_result == 'p')
            {
                if (fs_help_topic_level == 1)
                {
                    set_high_light(stdout);
                    term_puts("No previous topic. ");
                    unset_high_light(stdout);
                }
                else
                {
                    good_input = TRUE;
                    query_result = PREVIOUS_HELP_TOPIC;
                }
            }
            else if (read_result == INTERRUPTED)
            {
#ifdef UNIX
                if ((recorded_signal == SIGTSTP) ||
                    (recorded_signal == SIGWINCH))
                {
                    query_result = REDRAW_CURRENT_HELP_PAGE;
                    good_input = TRUE;
                }
                else
                {
                    /*EMPTY*/
                    ; /* Do nothing */
                }
#endif
            }
            else
            {
                set_high_light(stdout);
                term_puts("Invalid choice. ");
                unset_high_light(stdout);
            }
        }
         if (( ! good_input) && (read_result != INTERRUPTED))
         {
             term_beep();
         }
     }

     return query_result;
 }

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

static Help_request help_menu_query(int* item_ptr, int topic_count,
                                    int cur_topic)
{
#ifdef UNIX
    IMPORT volatile int recorded_signal;
#endif 
    int                 read_result;
    Help_request        query_result;
    Bool                good_input;
    int                 item;


    if ( ! kjb_isatty(fileno(stdout)))
    {
        return CONTINUE_WITH_TOPIC;
    }

    good_input = FALSE;
    query_result = HELP_ERROR;

    while ( ! good_input )
    {
        put_prompt("Enter number, ");

        set_high_light(stdout);
        put_prompt("b");
        unset_high_light(stdout);
        put_prompt(" to go back, ");

        if (fs_help_topic_level > 1)
        {
            set_high_light(stdout);
            put_prompt("p");
            unset_high_light(stdout);
            put_prompt(" for previous, ");
        }
        set_high_light(stdout);
        put_prompt("q");
        unset_high_light(stdout);
        put_prompt(" to quit: ");

        if (is_interactive())
        {
            read_result = enhanced_term_getc();
        }
        else
        {
            read_result = fgetc(stdin);
        }

        term_blank_out_line();

        if (read_result == ERROR)
        {
            good_input = TRUE;
            query_result = EXIT_HELP;
        }
        else if (read_result == EOF)
        {
            good_input = TRUE;
            query_result = EXIT_HELP;
        }
        else
        {
            if ((read_result > 0) && (isdigit(read_result)))
            {
                item = read_result - '0';

                if ((item>0) && (item <= topic_count))
                {
                    *item_ptr = item;
                    query_result = HELP_MENU_CHOICE;
                    good_input = TRUE;
                }
                else
                {
                    set_high_light(stdout);
                    term_puts("Invalid topic. ");
                    unset_high_light(stdout);
                }
            }
            else if ((read_result > 0) && (isupper(read_result)))
            {
                item = 10 + read_result - 'A';

                if ((item>0) && (item <= topic_count))
                {
                    *item_ptr = item;
                    query_result = HELP_MENU_CHOICE;
                    good_input = TRUE;
                }
                else
                {
                    set_high_light(stdout);
                    term_puts("Invalid topic. ");
                    unset_high_light(stdout);
                }
            }
            else if ((read_result == '\n') || (read_result == ' '))
            {
                good_input = TRUE;
                query_result = CONTINUE_WITH_TOPIC;
            }
            else if (read_result == 'b')
            {
                good_input = TRUE;
                query_result = BACK_HELP_PAGE;
            }
            else if (read_result == UP_ARROW)
            {
                if (cur_topic == 1)
                {
                    set_high_light(stdout);
                    term_puts("First topic. ");
                    unset_high_light(stdout);
                }
                else
                {
                    good_input = TRUE;
                    query_result = BACK_HELP_SUB_TOPIC;
                }
            }
            else if (read_result == DOWN_ARROW)
            {
                if (cur_topic >= topic_count)
                {
                    set_high_light(stdout);
                    term_puts("Last topic. ");
                    unset_high_light(stdout);
                }
                else
                {
                    good_input = TRUE;
                    query_result = FORWARD_HELP_SUB_TOPIC;
                }
            }
            else if (read_result == 'h')
            {
                good_input = TRUE;
                query_result = HELP_ON_HELP;
            }
            else if ((read_result == 'e') || (read_result == 'q'))
            {
                good_input = TRUE;
                query_result = EXIT_HELP;
            }
            else if (read_result == 'p')
            {
                if (fs_help_topic_level == 1)
                {
                    set_high_light(stdout);
                    term_puts("No previous topic. ");
                    unset_high_light(stdout);
                }
                else
                {
                    good_input = TRUE;
                    query_result = PREVIOUS_HELP_TOPIC;
                }
            }
            else if (read_result == INTERRUPTED)
            {
#ifdef UNIX
                if ((recorded_signal == SIGTSTP) ||
                    (recorded_signal == SIGWINCH))
                {
                    good_input = TRUE;
                    query_result = REWRITE_HELP_MENU;
                }
                else
                {
                    /*EMPTY*/
                    ; /* Do nothing */
                }
#endif
            }
            else
            {
                set_high_light(stdout);
                term_puts("Invalid choice. ");
                unset_high_light(stdout);
            }
        }

        if (( ! good_input) && (read_result != INTERRUPTED))
        {
            term_beep();
        }
    }

    return query_result;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

static FILE* open_help_file(const char* root_name)
{
    char help_file_name[ MAX_FILE_NAME_SIZE ];
    char help_file_dir[ MAX_FILE_NAME_SIZE ];

    BUFF_CPY(help_file_name, root_name);
    BUFF_CAT(help_file_name, HELP_FILE_SUFFIX);
    help_file_dir[ 0 ] = '\0';

#ifdef DOCUMENT_DIR
    BUFF_CAT(help_file_dir, DOCUMENT_DIR);
#ifdef HELP_DIR
    BUFF_CAT(help_file_dir, DIR_STR);
#endif
#endif

#ifdef HELP_DIR
    BUFF_CAT(help_file_dir, HELP_DIR);
#endif

    return open_config_file((char*)NULL, help_file_dir, help_file_name, "help");
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

int man_print_heading(FILE* fp, const char* topic)
{
    char uc_topic[ 200 ];


    EXTENDED_UC_BUFF_CPY(uc_topic, topic);
    ERE(fput_line(fp, uc_topic));

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

int man_print_title(FILE* fp, const char* title, const char* section)
{
    char        full_title[ 100 ];
    const char* heading           = "";
    size_t      head_len;
    size_t      title_len;


    BUFF_CPY(full_title, title);
    BUFF_CAT(full_title, "(");
    BUFF_CAT(full_title, section);
    BUFF_CAT(full_title, ")");


#if 0 /* was ifdef DEF_OUT */
    if (*section == '1')
    {
        heading = "Commands";
    }
    else if (*section == '3')
    {
        heading = "Library Routines";
    }
    else if (*section == '4')
    {
        heading = "File Formats";
    }
    else
    {
        heading = "Documentation";
    }
#endif

    title_len = strlen(full_title);
    head_len  = strlen(heading);

    if (head_len + title_len + 4 > MAN_OUTPUT_WIDTH)
    {
        ERE(kjb_fprintf(fp, "%.*s\n", MAN_OUTPUT_WIDTH, full_title));
    }
    else if (head_len + 2 * title_len + 6 > MAN_OUTPUT_WIDTH)
    {
        ERE(kjb_fprintf(fp, "%s    %s\n", full_title, heading));
    }
    else
    {
        unsigned int left_sep_len = (MAN_OUTPUT_WIDTH - 2*title_len - head_len) / 2;
        unsigned int right_sep_len =
                MAN_OUTPUT_WIDTH - left_sep_len - 2*title_len - head_len;

        ERE(kjb_fputs(fp, full_title));
        ERE(print_blanks(fp, left_sep_len));
        ERE(kjb_fputs(fp, heading));
        ERE(print_blanks(fp, right_sep_len));
        ERE(fput_line(fp, full_title));
    }

    ERE(kjb_fputs(fp, "\n"));

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

#ifdef __cplusplus
}
#endif

