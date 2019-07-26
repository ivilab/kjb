
/* $Id: i_display.c 21596 2017-07-30 23:33:36Z kobus $ */

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

#include "i/i_gen.h"     /* Only safe as first include in a ".c" file. */
#include "i/i_display.h"


#ifdef __cplusplus
extern "C" {
#endif

/* -------------------------------------------------------------------------- */

#define MAX_NUM_DISPLAYED_IMAGES    1000

/* -------------------------------------------------------------------------- */


typedef struct Displayed_image
{
    int   pid;
    char* temp_file_name;
}
Displayed_image;


/* -------------------------------------------------------------------------- */

#ifdef UNIX
static int slave_display_image_file
(
    char* file_name,
    char* title,
    char* display_program
);

static int slave_close_displayed_image(int image_num);
static void slave_close_all_displayed_images(void);

static int display_image_file
(
    const char* file_name,
    const char* title,
    const char* display_program
);
#endif   


/* -------------------------------------------------------------------------- */

#ifdef UNIX
static int              fs_image_display_parent_pid  =   NOT_SET;
static int              fs_image_display_pid         =   NOT_SET;
static int              fs_display_command_pipe[ 2 ] = { NOT_SET, NOT_SET };
static int              fs_display_result_pipe [ 2 ] = { NOT_SET, NOT_SET };
static Displayed_image* fs_image_display_info[ MAX_NUM_DISPLAYED_IMAGES ];
#endif   

static char fs_default_display_program[ 1000 ] = { '\0' };

/* -------------------------------------------------------------------------- */

int set_display_options(const char* option, const char* value)
{
    char lc_option[ 100 ];
    int  result = NOT_FOUND;


    EXTENDED_LC_BUFF_CPY(lc_option, option);

    if (    (lc_option[ 0 ] == '\0')
         || match_pattern(lc_option, "ddp")
         || match_pattern(lc_option, "default-display-program")
       )
    {
        if (value == NULL)
        {
            return NO_ERROR;
        }
        else if (value[ 0 ] == '?')
        {
            ERE(pso("default-display-program = %s\n",
                    fs_default_display_program[ 0 ] != '\0' ? fs_default_display_program : "<not set>")); 
        }
        else if (value[ 0 ] == '\0')
        {
            if (fs_default_display_program[ 0 ] == '\0')
            {
                ERE(pso("No default display program has been set.\n"));
            }
            else
            {
                ERE(pso("The default display program is %s\n",
                        fs_default_display_program));
            }
        }
        else
        {
            if (is_no_value_word(value))
            {
                fs_default_display_program[ 0 ] = '\0';
            }
            else
            {
                char* value_pos;
                char  title_option_str[ 1000 ];
                char  value_buff[ 1000 ];

                BUFF_CPY(value_buff, value);
                value_pos = value_buff;

                BUFF_GEN_GET_TOKEN(&value_pos, fs_default_display_program, ",");
                BUFF_GEN_GET_TOKEN(&value_pos, title_option_str, ",");

                if (title_option_str[ 0 ] != '\0')
                {
                    BUFF_CAT(fs_default_display_program, " -");
                    BUFF_CAT(fs_default_display_program, title_option_str);
                }
            }
        }
        result = NO_ERROR;
    }

    return result;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                              create_image_display
 *
 * Starts up image display process
 *
 * This routine starts up image display process. It is not essential to call
 * this routine, as it is called automatically when the first display request is
 * made, but it is recomended that programs which display images call this
 * routine at program startup (technically, before much memory has been
 * allocated). This reduces the overall VM usage.
 *
 * Note:
 *    This process, and all display processes are automatically terminated on
 *    program exit.
 *
 * Returns:
 *     NO_ERROR on sucess and ERROR on failure.
 *
 * Index: images, image I/O, image display
 *
 * -----------------------------------------------------------------------------
*/

int create_image_display(void)
{
#ifdef UNIX
    int display_pid;


    if (KJB_IS_SET(fs_image_display_pid))
    {
        dbp("Early exit of create_image_display, as we already have it set up.");

        return NO_ERROR;
    }

    if (    (pipe(fs_display_command_pipe) == EOF)
         || (pipe(fs_display_result_pipe) == EOF)
       )
    {
        set_error("Creation of pipe for display process failed.");
        return ERROR;
    }

    /*
    dbi(fs_display_command_pipe[ READ_END ]);
    dbi(fs_display_command_pipe[ WRITE_END ]);
    dbi(fs_display_result_pipe[ READ_END ]);
    dbi(fs_display_result_pipe[ WRITE_END ]);
    */

    display_pid = kjb_fork();

    if (display_pid == ERROR)
    {
        insert_error("Can't create process for image display.");
        return ERROR;
    }

    if (IS_PARENT(display_pid))
    {
        fs_image_display_parent_pid = MY_PID;

        add_cleanup_function(destroy_image_display);

        close(fs_display_result_pipe[ WRITE_END ]);
        close(fs_display_command_pipe[ READ_END ]);

        fs_image_display_pid = display_pid;

        return NO_ERROR;
    }
    else /* Child */
    {
        char        line[ 1000 ];
        char        command[ 1000 ];
        char*       line_pos;
        int         count;

        if (close(fs_display_result_pipe[ READ_END ]) == -1)
        {
            /* Just print a message and hope for the best. */
            p_stderr("Can't close read end of display result pipe.%S");
        }

        if (close(fs_display_command_pipe[ WRITE_END ]) == -1)
        {
            /* Just print a message and hope for the best. */
            p_stderr("Can't close write end of display command pipe.%S");
        }

        for (count=0; count<MAX_NUM_DISPLAYED_IMAGES; count++)
        {
            fs_image_display_info[ count ] = NULL;
        }

        EPE(kjb_signal(SIGINT, SIG_IGN));

        /*
        // Althought the normal for the child to finish up is with the "quit"
        // command, there are other ways to exit, such as through signal
        // handling inherited from the parent. Thus we close all the images
        // as a cleanup task.
        */
        add_cleanup_function(slave_close_all_displayed_images);

        /*CONSTCOND*/
        while ( TRUE )
        {
            long res;


            line[ 0 ] = '\0';
            EPE(res = BUFF_DGET_LINE(fs_display_command_pipe[ READ_END ], line));

#ifdef TEST
            /*
            // if (res < 0)
            //    {
            //     pso("Display process read from pipe returned %d.\n", res);
            //    }
            */
#endif

            if (res == INTERRUPTED) continue;

            line_pos = line;

            BUFF_GEN_GET_TOKEN(&line_pos, command, ", ");

            if ((STRCMP_EQ(command, "quit") || (res == EOF)))
            {
                /*
                // The exit closes all displayed images due to having added
                // slave_close_all_displayed_images to the cleanup functions
                */
                kjb_exit( EXIT_SUCCESS );
                /*NOTREACHED*/
            }
            else if (STRCMP_EQ(command, "display"))
            {
                char file_name [ MAX_FILE_NAME_SIZE ];
                char title[ 1000 ];
                char display_program[ 1000 ];
                int  display_result;
                char display_result_string[ 1001 ];


                BUFF_GEN_GET_TOKEN(&line_pos, file_name, ", ");
                BUFF_GEN_MQ_GET_TOKEN(&line_pos, title, ",");
                BUFF_GEN_GET_TOKEN(&line_pos, display_program, ",");

                display_result = slave_display_image_file(file_name, title,
                                                          display_program);

                kjb_sprintf(display_result_string,
                            sizeof(display_result_string),
                            "%d\n", display_result);

                kjb_write(fs_display_result_pipe[ WRITE_END ],
                          display_result_string,
                          strlen(display_result_string));
            }
            else if (STRCMP_EQ(command, "close"))
            {
                char image_num_string[ 1000 ];
                int  image_num;
                int  result;


                BUFF_GEN_GET_TOKEN(&line_pos, image_num_string, ", ");

                result = ss1i(image_num_string, &image_num);

                if (result == ERROR)
                {
                    kjb_fprintf(stderr, "Possible program problem.\n");
                    kjb_fprintf(stderr,
                                "Invalid image display number to close.\n");
                    kjb_print_error();
                }
                else
                {
                    EPE(slave_close_displayed_image(image_num));
                }
            }
            else
            {
                p_stderr("Possible program problem.\n");
                p_stderr("Invalid command (%s) sent to display process.\n",
                         command);
            }
        }     /* while ( TRUE ); */
    }             /* End of child code */

    /*NOTREACHED*/

#else
    /* 
     * There is no point in setting an error message or returning error, since
     * this is a standard initialization function that will be called even if
     * the user does not want to display an image.
    */
    return NO_ERROR;

#endif   

}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

int display_any_image
(
    const void* ip,
    const char* title,
    int         (*image_write_fn) (const void*, char *)
)
{
#ifdef UNIX
    char   temp_file_name[ MAX_FILE_NAME_SIZE ];
    char   line[ 1000 ];
    size_t line_len;
    int    image_num;
    int    result;
    char   translated_title[ 1000 ];


    if (fs_image_display_pid == NOT_SET)
    {
        ERE(create_image_display());
    }

    ERE(BUFF_GET_TEMP_FILE_NAME(temp_file_name));

    ERE((*image_write_fn)(ip, temp_file_name));

    BUFF_CPY(line, "display,");
    BUFF_CAT(line, temp_file_name);
    BUFF_CAT(line, ",");

    if ((title == NULL) || (title[ 0 ] == '\0'))
    {
        title = "Untitled";
    }

    BUFF_CPY(translated_title, title);
    char_for_char_translate(translated_title, '"', '\'');
    BUFF_CAT(line, "\"");
    BUFF_CAT(line, translated_title);
    BUFF_CAT(line, "\"");
    BUFF_CAT(line, ",");

    BUFF_CAT(line, fs_default_display_program);

    BUFF_CAT(line, "\n");

    line_len = strlen(line);

    result = kjb_write(fs_display_command_pipe[ WRITE_END ], line, line_len);

    if (result == ERROR)
    {
        kjb_unlink(temp_file_name);
        return ERROR;
    }
    else
    {
        BUFF_DGET_LINE(fs_display_result_pipe[ READ_END ], line);
        ERE(ss1i(line, &image_num));
        return image_num;
    }

#else

    set_error("Image display is only implemented in UNIX.");
    return ERROR;

#endif   

}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                             close_displayed_image
 *
 * Closes a displayed image
 *
 * This routine closes a displayed image. All images are closed on program exit,
 * and, in addition, may be closed asynchronously by the user. However, if you
 * want to close an image from within a program, then this routine can be used.
 *
 * Returns:
 *     NO_ERROR on success, and ERROR on failure, with an appropriate error
 *     message being set.
 *
 * Index: images, image I/O, image display
 *
 * -----------------------------------------------------------------------------
*/

int close_displayed_image(int image_num)
{
#ifdef UNIX
    char command[ 1000 ];
    int result;


    ERE(kjb_sprintf(command, sizeof(command), "close,%d\n", image_num));

    result = kjb_write(fs_display_command_pipe[ WRITE_END ], command,
                       strlen(command));

    if (result == ERROR)
    {
        add_error("Image display close failed." );
    }

    return result;

#else
    /* 
     * There is no point in setting an error message or returning error, since
     * this is a standard initialization function that will be called even if
     * the user does not want to display an image.
    */
    return NO_ERROR;

#endif   

}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

void destroy_image_display(void)
{
#ifdef UNIX
    Signal_info save_broken_pipe_vec;
    Signal_info new_broken_pipe_vec;
    int         write_res;


    if (fs_image_display_pid == NOT_SET) return;

    TEST_PSO(("Begin cleaning up image display process (%d).\n",
              fs_image_display_pid));

    if (fs_image_display_parent_pid != MY_PID)
    {
        TEST_PSO(("Aborting cleaning up image display process because not parent.\n"));
        return;
    }

    INIT_SIGNAL_INFO(new_broken_pipe_vec);
    new_broken_pipe_vec.SIGNAL_HANDLER = SIG_IGN;
    kjb_sigvec(SIGPIPE, &new_broken_pipe_vec, &save_broken_pipe_vec);

    write_res = kjb_write(fs_display_command_pipe[ WRITE_END ], "quit\n", 5);

    kjb_sigvec(SIGPIPE, &save_broken_pipe_vec, (Signal_info*)NULL);

    if (write_res != ERROR)
    {
#ifdef TEST
        int waitpid_result;

        waitpid_result = kjb_waitpid(fs_image_display_pid);
        TEST_PSO(("Wait result is %d.\n", waitpid_result));
#else
        kjb_waitpid(fs_image_display_pid);
#endif
    }
#ifdef TEST
    else
    {
        TEST_PSO(("Write to display process pipe failed.\n"));
    }
#endif

    TEST_PSO(("Done waiting for image display process to finish.\n"));

    fs_image_display_pid = NOT_SET;

#endif 

}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

#ifdef UNIX
static int slave_display_image_file
(
    char* file_name,
    char* title,
    char* display_program
)
{
    int              display_pid;
    Displayed_image* displayed_image_ptr;
    int              image_num           = NOT_SET;
    int              i;


    for (i=0; i<MAX_NUM_DISPLAYED_IMAGES; i++)
    {
        if (fs_image_display_info[ i ] == NULL)
        {
            image_num = i;
            break;
        }
    }

    if (image_num == NOT_SET)
    {
        set_error("Can't display more than %d images.",
                  MAX_NUM_DISPLAYED_IMAGES);
        return ERROR;
    }

    ERE(display_pid = kjb_fork());

    if (IS_PARENT(display_pid))
    {
        NRE(displayed_image_ptr = TYPE_MALLOC(Displayed_image));
        displayed_image_ptr->pid = display_pid;

        displayed_image_ptr->temp_file_name = kjb_strdup(file_name);

        if (displayed_image_ptr->temp_file_name == NULL)
        {
            kjb_free(displayed_image_ptr);
            return ERROR;
        }

        fs_image_display_info[ i ] = displayed_image_ptr;

        return image_num;
    }
    else /*  Child */
    {
        EPE(display_image_file(file_name, title, display_program));

        _exit( EXIT_FAILURE );   /* Don't use kjb_exit, or even exit(),
                                    because we don't want to clean up.
                                 */
    }

    /*NOTREACHED*/
    SET_CANT_HAPPEN_BUG();
    return ERROR;   /* Keep error checkers happy. */
}
#endif   


/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

#ifdef UNIX
static int slave_close_displayed_image(int image_num)
{
    int pid;


    if (    (image_num < 0)
         || (image_num >= MAX_NUM_DISPLAYED_IMAGES)
       )
    {
        set_bug("Image number for close (%d) is out of bounds (%d, %d).",
                image_num, 0, MAX_NUM_DISPLAYED_IMAGES - 1);
        return ERROR;
    }
    else
    {
        Displayed_image *image_info_ptr;


        image_info_ptr = fs_image_display_info[ image_num ];

        if (image_info_ptr != NULL)
        {
            pid = image_info_ptr->pid;
            kill(pid, SIGTERM);

            kjb_waitpid(pid);

            kjb_unlink(image_info_ptr->temp_file_name);
            kjb_free(image_info_ptr->temp_file_name);
            kjb_free(image_info_ptr);

            fs_image_display_info[ image_num ] = NULL;
        }
        else
        {
            set_bug("Closing a non-existing image (%d).", image_num);
            return ERROR;
        }
    }

    return NO_ERROR;
}
#endif 

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

#ifdef UNIX
static void slave_close_all_displayed_images(void)
{
    int i;
    int pid;


    for (i=0; i<MAX_NUM_DISPLAYED_IMAGES; i++)
    {
        if (fs_image_display_info[ i ] != NULL)
        {
            pid = fs_image_display_info[ i ]->pid;
            kill(pid, SIGTERM);

            TEST_PSO(("Waiting for process %d to finish.\n", pid));
            kjb_waitpid(pid);
            TEST_PSO(("Process %d has finished.\n", pid));

            kjb_unlink(fs_image_display_info[ i ]->temp_file_name);
            kjb_free(fs_image_display_info[ i ]->temp_file_name);
            kjb_free(fs_image_display_info [ i ] );
            fs_image_display_info[ i ] = NULL;
        }
    }
}
#endif 

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

int fork_display_any_image
(
    const void* ip,
    const char* title,
    int         (*image_write_fn) (const void *, char *)
)
{
#ifdef UNIX
    char temp_file_name[ MAX_FILE_NAME_SIZE ];
    int  cleanup_pid;
    int  display_pid;


    ERE(BUFF_GET_TEMP_FILE_NAME(temp_file_name));

    ERE((*image_write_fn)(ip, temp_file_name));

    verbose_pso(2, "Verbose level before fork for displaying images %d.\n", 
                kjb_get_verbose_level());

    ERE(cleanup_pid = kjb_fork());

    if (IS_CHILD(cleanup_pid))
    {
        display_pid = kjb_fork();

        if (display_pid < 0)
        {
            kjb_fprintf(stderr, "Fork of display process failed.%S");
            kjb_unlink(temp_file_name);
            return ERROR;
        }
        else if (IS_PARENT(display_pid))
        {
            kjb_waitpid(display_pid);
            kjb_unlink(temp_file_name);
            _exit(EXIT_SUCCESS);
            /*NOTREACHED*/
        }
        else
        {
            EPE(display_image_file(temp_file_name, title,
                                   fs_default_display_program));
            _exit(EXIT_SUCCESS);
            /*NOTREACHED*/
        }
    }

    return NO_ERROR;

#else

    set_error("Image display is only implemented in UNIX.");
    return ERROR;

#endif   

}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

#ifdef UNIX
static int display_image_file
(
    const char* file_name,
    const char* title,
    const char* display_program
)
{
    char  exec_string[ 1000 ];
    char  display_program_first_word[ 200 ];
    char  display_program_copy[ 2000 ];
    char* display_program_pos;
    static const char *display_programs[ ] =
    {
        "s2_display++ -title",
        "kjb_display -title",
        "display++ -title",
        "display -title",
        "xv -name"
    };
    int num_display_programs = sizeof(display_programs) / sizeof(char*);
    int i;


    if ((title == NULL) || (*title == '\0'))
    {
        title = "Untitled";
    }

    if ((display_program != NULL) && (display_program[ 0 ] != '\0'))
    {
        /*
        TEST_PSO(("Trying display command %s.\n", display_program));
        */

        /*
         * We are assuming that the title string is in quotes.
         */
        EPETE(kjb_sprintf(exec_string, sizeof(exec_string), "%s %s %s",
                          display_program, title, file_name));

        if (kjb_exec(exec_string) != ERROR)
        {
            SET_CANT_HAPPEN_BUG();
            return ERROR;
        }

        /*
        TEST_PSO(("Display command %s failed.\n", display_program));
        */
    }

    for (i=0; i<num_display_programs; i++)
    {
        /*
        TEST_PSO(("Trying display command %s.\n",
                  display_programs[ i ]));
        */

        /*
         * We are assuming that the title string is in quotes.
         */
        EPETE(kjb_sprintf(exec_string, sizeof(exec_string), "%s %s %s",
                          display_programs[ i ], title, file_name));

        if (kjb_exec(exec_string) != ERROR)
        {
            SET_CANT_HAPPEN_BUG();
            return ERROR;
        }

        /*
        TEST_PSO(("Display command %s failed.\n", display_programs[ i ]));
        */
    }

    set_error("Can't execute any display programs.");
    add_error("Tried ");

    if (display_program[ 0 ] != '\0')
    {
        BUFF_CPY(display_program_copy, display_program);
        display_program_pos = display_program_copy;
        BUFF_GET_TOKEN(&display_program_pos, display_program_first_word);

        cat_error("\"%s\", ", display_program_first_word);
    }

    for (i=0; i<num_display_programs - 1; i++)
    {
        BUFF_CPY(display_program_copy, display_programs[ i ]);
        display_program_pos = display_program_copy;
        BUFF_GET_TOKEN(&display_program_pos, display_program_first_word);

        cat_error("\"%s\", ", display_program_first_word);
    }

    BUFF_CPY(display_program_copy,
             display_programs[ num_display_programs - 1 ]);
    display_program_pos = display_program_copy;
    BUFF_GET_TOKEN(&display_program_pos, display_program_first_word);
    cat_error("\"%s\" in PATH.", display_program_first_word);

    return ERROR;
}
#endif 

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

#ifdef __cplusplus
}
#endif

