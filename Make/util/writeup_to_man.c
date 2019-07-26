
#include "l/l_incl.h"

/* ------------------------------------------------------------------------- */

static int writeup_to_man_guts
(
    FILE*       writeup_fp,
    const char* title,
    const char* section 
);

static int copy_script_comment_block
(
    char*       comment_block,
    const char* script_file_str,
    size_t      comment_block_size
);

/* ------------------------------------------------------------------------- */

int main(int argc, char** argv)
{
    FILE* writeup_fp;
    char  writeup_file_path[ MAX_FILE_NAME_SIZE ];
    char  title[ 100 ];
    char  section[ 100 ];
    char  flag;


    BUFF_CPY(writeup_file_path, "WRITEUP");
    BUFF_CPY(title, "");
    BUFF_CPY(section, "1");

    argc--;
    argv++;

    while (((argc > 0) && (**argv != '-')) || ((argc > 1) && (**argv == '-')))
    {
        if (**argv == '-')
        {
            flag = (*argv)[ 1 ];
        }
        else
        {
            flag = '\0';
        }

        switch (flag)
        {
            case 't':
                dbw();
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
                BUFF_CPY(writeup_file_path, *argv);

                if (title[ 0 ] == '\0')
                {
                    char  temp_writeup_file_path[ MAX_FILE_NAME_SIZE ];
                    char  temp_writeup_file_name[ 200 ];
                    char  suffix[ 100 ];


                    BUFF_CPY(temp_writeup_file_path, writeup_file_path);

                    BUFF_GEN_GET_LAST_TOKEN(temp_writeup_file_path,
                                            temp_writeup_file_name, "/");

                    BUFF_GEN_GET_LAST_TOKEN(temp_writeup_file_name,
                                            suffix, ".");

                    if (IC_STRCMP_EQ(suffix,"w"))
                    {
                        int last_char_index =
                                strlen(temp_writeup_file_name) - 1;

                        temp_writeup_file_name[ last_char_index ] = '\0';
                        BUFF_CPY(title, temp_writeup_file_name);
                    }
                    else
                    {
                        BUFF_CPY(title, writeup_file_path);
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

    writeup_fp = kjb_fopen(writeup_file_path, "r");

    if (writeup_fp == NULL)
    {
        char buff[ 1000 ];

        kjb_get_error(buff, sizeof(buff)); 

        BUFF_CAT(writeup_file_path, ".w");
        writeup_fp = kjb_fopen(writeup_file_path, "r");

        if (writeup_fp == NULL)
        {
            set_error(buff); 
            add_error("Also tried %s.", writeup_file_path); 
            kjb_print_error();
            return EXIT_FAILURE;
        }
    }

    EPETE(writeup_to_man_guts(writeup_fp, title, section));

    kjb_fclose(writeup_fp);

    return EXIT_SUCCESS;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

static int writeup_to_man_guts
(
    FILE*       writeup_fp,
    const char* title,
    const char* section 
)
{
    char  initial_buff[ 100000 ];
    char  writeup_buff[ 100000 ];
    char  formatted_buff[ 100000 ];
    const char* formatted_buff_pos;
    char  writeup_line[ 10000 ];
    char* writeup_line_pos;
    int   find_res;
    int   line_finished;
    int   high_light            = FALSE;
    int   last_char_index;
    int   read_res;
    size_t effective_buff_size = sizeof(initial_buff) - ROOM_FOR_NULL; 


    /*
     * We need a relatively native read because we are interested in the
     * comments. 
     *
     * However, the following attempts swallows the whole file, and we do not
     * attempt to read another line. So it is hard to declare a buffer that is
     * large enough, except that we can assume that the documentation is at the
     * begining. So, we assume that we have it, if we read effective_buff_size
     * characters. 
    */
    ERE(read_res = kjb_fread(writeup_fp, initial_buff, effective_buff_size));

    if (read_res < 0) kjb_exit(EXIT_FAILURE);
    ASSERT(read_res < sizeof(initial_buff)); 
    initial_buff[ read_res ] = '\0'; 

    if (HEAD_CMP_EQ(initial_buff, "#!/"))
    {
        int script_comment_res; 

        ERE(script_comment_res = copy_script_comment_block(writeup_buff, initial_buff, sizeof(writeup_buff)));

        if (script_comment_res == NOT_FOUND)
        {
            /*
             * If there is no comment block, then we want to output exactly
             * nothing. 
            */
            return NO_ERROR; 
        }
    }
    else
    {
        kjb_strncpy(writeup_buff, initial_buff, sizeof(writeup_buff));
    }

    EPETE(man_print_title(stdout, title, section));

    EPETE(left_justify(writeup_buff, MAN_OUTPUT_WIDTH, 0, "|", 1, 0,
                       formatted_buff,
                       sizeof(formatted_buff)));

    formatted_buff_pos = formatted_buff;

    while (BUFF_CONST_SGET_LINE(&formatted_buff_pos, writeup_line) != EOF)
    {
        writeup_line_pos = writeup_line;
        trim_end(writeup_line_pos);

        last_char_index = strlen(writeup_line_pos) - 1;

        if (    (writeup_line_pos[ 0 ] != ' ')
             && (writeup_line_pos[ last_char_index ] == ':')
           )
        {
            writeup_line_pos[ last_char_index ] = '\0';
            EPETE(man_print_heading(stdout, writeup_line_pos));
        }
        else
        {
            line_finished = FALSE;

            writeup_line_pos = writeup_line;

            if (*writeup_line_pos == DONT_FORMAT_CHAR)
            {
#ifdef HOW_IT_WAS_BEFORE_MAY_30_2004
                *writeup_line_pos = ' ';
#else
                writeup_line_pos++;
#endif
            }

            if ((*writeup_line_pos == HIGH_LIGHT_CHAR) &&
                (*(writeup_line_pos+1) != HIGH_LIGHT_CHAR))
            {
                if (high_light) high_light = FALSE;
                else high_light = TRUE;
                writeup_line_pos++;
            }


             while ( ! line_finished)
             {
                find_res=find_char(writeup_line_pos, HIGH_LIGHT_CHAR);

                if (find_res)
                {
                    *(writeup_line_pos+find_res-1) = '\0';

                    if (high_light)
                    {
                        print_underlined(stdout, writeup_line_pos);
                    }
                    else
                    {
                        kjb_puts(writeup_line_pos);
                    }

                    writeup_line_pos += find_res;

                    if (*writeup_line_pos != HIGH_LIGHT_CHAR)
                    {
                        high_light = !high_light;
                    }
                    else
                    {
                        kjb_putc(HIGH_LIGHT_CHAR);
                        writeup_line_pos++;
                    }
                }
                else
                {
                    line_finished = TRUE;

                    if (high_light)
                    {
                        print_underlined(stdout, writeup_line_pos);
                    }
                    else
                    {
                        kjb_puts(writeup_line_pos);
                    }
                    kjb_puts("\n");
                }
            }
        }
    }
    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

static int copy_script_comment_block
(
    char*       comment_block,
    const char* script_file_str,
    size_t      comment_block_size
)
{
    char line[ 1000 ];
    int before_block = TRUE;
    char* line_pos;

    /* *  Skip first line which is supposed to start with "#!/" */
    BUFF_CONST_SGET_LINE(&script_file_str, line);

    *comment_block = '\0';

    while (BUFF_CONST_SGET_LINE(&script_file_str, line) != EOF)
    {
        trim_end(line); 
        line_pos = line; 
        trim_beg(&line_pos); 

        /* If we are before a block, then must either be a blank line, or a
         * comment line which might start a block. 
        */
        if (*line_pos == '\0')
        {
            if (before_block) 
            {
                continue;
            }
            else
            {
                /* First non comment line signals the end of the block. */
                return NO_ERROR; 
            }
        }
        else if (*line_pos == '#') 
        {
            char* line_pos_2;


            while (*line_pos == '#') 
            {
                line_pos++;
            }

            /* Absorb the first blank (only). */
            if (*line_pos == ' ')
            {
                line_pos++;
            }

            line_pos_2 = line_pos; 

            trim_beg(&line_pos_2); 

            if (before_block)
            {
                /* Ignore leading empty lines. */
                if (*line_pos_2 == '\0') continue; 

                /* Quick and dirty check for SVN tag. Probably should use
                 * regexp and exactly as specified in the SVN documentation.
                */
                if (IC_HEAD_CMP_EQ(line_pos_2, "$Id")) continue;

                /* 
                 * We are looking for a line that ends in a colon. 
                */
                if (last_char(line_pos_2) == ':') 
                {
                    before_block = FALSE;
                }
                /* 
                 * We played with a single word being OK, but we need to first
                 * decide what we want comment blocks to look like, then revisit
                 * these heuristics.
                */
                else
                {
                    return NOT_FOUND; 
                }
            }
        }
        else if (before_block)
        {
            return NOT_FOUND;
        }
        else 
        {
            /* We are done. */
            return NO_ERROR;
        }

        kjb_strncat(comment_block, line_pos, comment_block_size);
        comment_block_size -= (strlen(line_pos));
        kjb_strncat(comment_block, "\n", comment_block_size);
        comment_block_size --;
    }

    return NO_ERROR;
}

