
#include "l/l_incl.h"

/* ------------------------------------------------------------------------- */

#ifndef EXTRA_INDENT
#     define EXTRA_INDENT 3
#endif

/* ------------------------------------------------------------------------- */

static Help_request help_to_man_topic(FILE* help_fp, char* topic);

static Help_request do_help_to_man_topic
(
    FILE* help_fp,
    char* topic,
    const char* help_text
);

static Help_request do_help_to_man_topic_menu
(
    FILE* help_fp,
    const char* help_text
);


/* ------------------------------------------------------------------------- */

static char fs_no_justify_chars [ ] = { HELP_FILE_COMMENT_LINE_CHAR,
                                     CONTINUE_WITH_HELP_TOPIC_CHAR,
                                     SYSTEM_DEPENDENT_HELP_LINE_CHAR,
                                     DOCUMENT_TYPE_ID_CHAR,
                                     DONT_FORMAT_CHAR,
                                     '\0'
                                   };

static int fs_help_topic_level;

/* ------------------------------------------------------------------------- */

int main(int argc, char** argv)
{
    FILE* help_fp;
    char  topic[ 2000 ];
    char  prev_topic[ 2000 ];
    char  help_file_path[ MAX_FILE_NAME_SIZE ];
    char  title[ 1000 ];
    char  section[ 1000 ];
    char  flag;


    BUFF_CPY(help_file_path, "HELP");
    BUFF_CPY(title, "");
    BUFF_CPY(section, "1");

    argc--;
    argv++;

    while (((argc > 0) && (**argv != '-')) || ((argc > 1) && (**argv == '-')))
    {
        if (**argv == '-') flag = (*argv)[ 1 ];
        else               flag = '\0';

        switch (flag)
        {
            case 't':
                argv++;
                argc--;
                BUFF_CPY(title, *argv);
                break;
            case 's':
                argv++;
                argc--;
                BUFF_CPY(section, *argv);
                break;
            case 'f':
                argv++;
                argc--;
                /*FALLTRHOUGH*/
            default:
                BUFF_CPY(help_file_path, *argv);

                if (title[ 0 ] == '\0')
                {
                    char  temp_help_file_path[ MAX_FILE_NAME_SIZE ];
                    char  temp_help_file_name[ 200 ];
                    char  suffix[ 100 ];


                    BUFF_CPY(temp_help_file_path, help_file_path);

                    BUFF_GEN_GET_LAST_TOKEN(temp_help_file_path,
                                            temp_help_file_name, "/");

                    BUFF_GEN_GET_LAST_TOKEN(temp_help_file_name,
                                            suffix, ".");

                    if (    (IC_STRCMP_EQ(suffix,"help"))
                         || (IC_STRCMP_EQ(suffix,"hlp"))
                       )
                    {
                        int last_char_index =
                                            strlen(temp_help_file_name) - 1;

                        temp_help_file_name[ last_char_index ] = '\0';
                        BUFF_CPY(title, temp_help_file_name);
                    }
                    else
                    {
                        BUFF_CPY(title, help_file_path);
                    }
                }
        }

        argv++;
        argc--;
    }

    if (title[ 0 ] == '\0')
    {
        p_stderr("No title can be determined.\n");
        p_stderr("Use \"-t <title>\" to specify it.\n");
        kjb_exit(EXIT_FAILURE);
    }

    help_fp = kjb_fopen(help_file_path, "r");

    if (help_fp == NULL)
    {
        BUFF_CAT(help_file_path, ".help");
        NPETE(help_fp = kjb_fopen(help_file_path, "r"));
    }

    man_print_title(stdout, title, section);

    BUFF_CPY(topic, "introduction");

    fs_help_topic_level = 0;
    prev_topic [ 0 ] = '\0';

    help_to_man_topic(help_fp, topic);

    return EXIT_SUCCESS;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

static Help_request help_to_man_topic(FILE* help_fp, char* topic)
{
    char         help_text[ MAX_HELP_TOPIC_LEN ];
    char         justified_help_text[ MAX_HELP_TOPIC_LEN ];
    int          read_res;
    int          justify_res;
    Help_request do_topic_res;

    read_res = local_get_help_topic(help_fp, topic, help_text,
                                    MAX_HELP_TOPIC_LEN);

    if (read_res == ERROR)
    {
        kjb_fprintf(stderr, "Error reading help file. ");
        kjb_fprintf(stderr, "Command aborted.\n");
        return EXIT_HELP;
    }
    else if (read_res == NOT_FOUND)
    {
        kjb_fprintf(stderr, "No help found for \"%s\"\n", topic);
        return EXIT_HELP;
    }

    justify_res = left_justify(help_text, MAN_OUTPUT_WIDTH, 0, fs_no_justify_chars,
                               1,    /* Keep indent level */
                               EXTRA_INDENT,
                               justified_help_text,
                               sizeof(justified_help_text));

    if (justify_res == ERROR)
    {
        set_bug("Justify of help_text failed.");
        return EXIT_HELP;
    }

    fs_help_topic_level++;

    do_topic_res = do_help_to_man_topic(help_fp, topic,
                                        justified_help_text);

    fs_help_topic_level--;

    return do_topic_res;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

static Help_request do_help_to_man_topic
(
    FILE* help_fp,
    char* topic,
    const char* help_text
)
{
    const char* help_text_pos;
    const char*        help_text_page_beg_pos;
    const char*        save_help_text_pos;
    char         help_line[ 10000 ];
    char*        help_line_pos;
    char         sub_topic[ 2000 ];
    char         system_name[ 2000 ];
    char         doc_type[ 2000 ];
    int          this_system;
    int          this_doc_type;
    int          read_res;
    int          find_res;
    int          line_finished;
    Help_request query_res = HELP_ERROR;
    int          menu_item;
    int          high_light;


    this_system   = TRUE;
    this_doc_type = TRUE;
    high_light    = FALSE;
    help_text_pos = help_text;

    /*CONSTCOND*/
    while (TRUE)
    {
        help_text_page_beg_pos = help_text_pos;
        save_help_text_pos = help_text_pos;

        sub_topic[ 0 ] = '\0';

        read_res = BUFF_CONST_SGET_LINE(&help_text_pos, help_line);

        if (read_res == EOF)
        {
            /*EMPTY*/
            ;   /* Do nothing. */
        }
        else if (help_line[ 0 ] == CONTINUE_WITH_HELP_TOPIC_CHAR)
        {
            query_res = do_help_to_man_topic_menu(help_fp,
                                                  help_text_page_beg_pos);
        }
        else
        {
            menu_item = FALSE;

            man_print_heading(stdout, topic);

            while ((read_res != EOF) && ( ! menu_item))
            {
                help_line_pos = help_line;

                if (*help_line_pos == HELP_TOPIC_CHAR)
                {
                    return DONT_EXIT_HELP;
                }
                else if (*help_line_pos == HELP_FILE_COMMENT_LINE_CHAR)
                {
                    /*EMPTY*/
                    ;   /* Do nothing. */
                }
                else if (*help_line_pos == CONTINUE_WITH_HELP_TOPIC_CHAR)
                {
                    menu_item = TRUE;

                    /*
                    //  Reprocess choice list after this page.
                    */
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
                            (STRCMP_EQ(doc_type, "man")))
                       {
                           this_doc_type = TRUE;
                       }
                    }
                }
                else if (( ! this_system ) || ( ! this_doc_type ))
                {
                    /*EMPTY*/
                    ;   /* Do nothing. */
                }
                else
                {
                    if (*help_line_pos == DONT_FORMAT_CHAR)
                    {
#ifdef HOW_IT_WAS_BEFORE_MAY_30_2004
                        *help_line_pos = ' ';
#else
                        help_line_pos++;
#endif
                        print_blanks(stdout, EXTRA_INDENT);
                    }

                    if ((*help_line_pos == HIGH_LIGHT_CHAR) &&
                        (*(help_line_pos+1) != HIGH_LIGHT_CHAR))
                    {
                        if (high_light) high_light = FALSE;
                        else high_light = TRUE;
                        help_line_pos++;
                    }

                    line_finished = FALSE;

                    while ( ! line_finished)
                    {
                        find_res=find_char(help_line_pos, HIGH_LIGHT_CHAR);

                        if (find_res != CHARACTER_NOT_IN_STRING)
                        {
                            *(help_line_pos+find_res-1) = '\0';

                            if (high_light)
                            {
                                print_underlined(stdout, help_line_pos);
                            }
                            else
                            {
                                kjb_fputs(stdout, help_line_pos);
                            }

                            help_line_pos += find_res;

                            if (*help_line_pos != HIGH_LIGHT_CHAR)
                            {
                                high_light = !high_light;
                            }
                            else
                            {
                                kjb_putc(HIGH_LIGHT_CHAR);
                                help_line_pos++;
                            }
                        }
                        else
                        {
                            line_finished = TRUE;

                            if (high_light)
                            {
                                print_underlined(stdout, help_line_pos);
                            }
                            else
                            {
                                kjb_fputs(stdout, help_line_pos);
                            }

                            kjb_puts("\n");
                        }
                    }
                }

                if ((read_res != EOF) && ( ! menu_item))
                {
                    save_help_text_pos = help_text_pos;

                    read_res = BUFF_CONST_SGET_LINE(&help_text_pos, help_line);
                }
           }
           kjb_puts("\n");
        }

        if ((read_res == EOF) || (query_res == DONT_EXIT_HELP))
        {
            return DONT_EXIT_HELP;
        }

    }

    /*NOTREACHED*/
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

static Help_request do_help_to_man_topic_menu
(
    FILE* help_fp,
    const char* help_text
)
{
    char sub_topics[ MAX_NUM_SUB_TOPICS ][ 2000 ];
    const char*  help_text_pos;
    char         help_line[ 2000 ];
    char*        help_line_pos;
    char         sub_topic[ 2000 ];
    int          read_res;
    Help_request help_res;
    int          line_count;
    int          topic_count;
    int          cur_menu_item;
    int          rewrite_screen;


    cur_menu_item = 1;

    while (TRUE)
    {
        help_text_pos = help_text;

        topic_count = 0;
        line_count = 0;

        read_res = const_sget_line(&help_text_pos, help_line, sizeof(help_line));
        line_count++;

        while (read_res != EOF)
        {
            help_line_pos = help_line;

            if (*help_line_pos == CONTINUE_WITH_HELP_TOPIC_CHAR)
            {
                help_line_pos++;
                trim_beg(&help_line_pos);
                BUFF_CPY(sub_topic, help_line_pos);
                trim_end(sub_topic);
                extended_uc_lc(sub_topic);
                remove_duplicate_chars(sub_topic, ' ');

                BUFF_CPY(sub_topics[topic_count], sub_topic);
                topic_count++;
            }

            read_res = const_sget_line(&help_text_pos, help_line, sizeof(help_line));
            line_count++;
       }
        kjb_puts("\n");


        rewrite_screen = FALSE;

        while ( ! rewrite_screen)
        {
            rewrite_screen = TRUE;

            if (cur_menu_item > topic_count)
            {
                return DONT_EXIT_HELP;
            }

            help_res = help_to_man_topic(help_fp,
                                         sub_topics[cur_menu_item-1]);

            if (help_res == EXIT_HELP)
            {
                return EXIT_HELP;
            }
            else
            {
                cur_menu_item++;
            }

            if (cur_menu_item > topic_count)
            {
                rewrite_screen = FALSE;
            }
        }
    }

    /*NOTREACHED*/
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */


