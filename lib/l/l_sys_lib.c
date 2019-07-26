
/* $Id: l_sys_lib.c 22504 2019-06-03 22:02:14Z kobus $ */

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

/*Some versions of makedepend cannot be trusted to honor the "-Y" option. */
#ifndef MAKE_DEPEND

#ifdef UNIX
#    include <sys/wait.h>
#    include <pwd.h>
#endif

#include <signal.h>

#endif   /* ! MAKE_DEPEND */


#include "l/l_io.h"
#include "l/l_sys_term.h"
#include "l/l_sys_scan.h"
#include "l/l_sys_sig.h"
#include "l/l_sys_str.h"
#include "l/l_string.h"
#include "l/l_parse.h"
#include "l/l_sys_tsig.h"
#include "l/l_sys_lib.h"

#ifdef MS_OS
#    ifdef __STDC__
#    endif 
#endif  /* End of CASE MS_OS. */

#ifdef __cplusplus
extern "C" {
#endif

/* -------------------------------------------------------------------------- */

/*
#ifdef TEST
#ifdef LINUX_X86
#define REPORT_PROCESSES
#endif
#endif
*/

#define LOCK_ENABLE_FILE  "~colour/Lockfile/LOCK_ENABLED"
#define LOCK_REQUEST_FILE "~colour/Lockfile/STOP"
#define LOCK_GRANT_FILE   "~colour/Lockfile/STOPPED_AND_WAITING"

#define MAX_SYSTEM_COMMAND_LINE_SIZE  50000

/* -------------------------------------------------------------------------- */

static int fs_lock_enable          = FALSE;

#ifdef UNIX
static int fs_process_generation   = 0;
static int fs_system_command_pid       = NOT_SET;
static int fs_system_command_pipe[ 2 ] = { NOT_SET, NOT_SET };
static int fs_system_result_pipe [ 2 ] = { NOT_SET, NOT_SET };
static int fs_system_command_parent_pid = NOT_SET;
#endif 

/* ------------------------------------------------------------------------- */

static int set_lock_enable(int new_enable_value);
static void cleanup_lock(void);
static void remove_lock_enable_file(void);
static void kjb_cleanup_guts(int abort_flag);

#ifdef TEST
    static int debug_add_cleanup_function_guts
    (
        void        (*cleanup_fn)(void),
        const char* file_name,
        int         line_number
    );
#else
    static int add_cleanup_function_guts(void (*cleanup_fn)(void));
#endif

/* ------------------------------------------------------------------------- */

int set_lock_options(const char* option, const char* value)
{
    char lc_option[ 100 ];
    int  temp_boolean_value;
    int  result = NOT_FOUND;


    EXTENDED_LC_BUFF_CPY(lc_option, option);

    if ((lc_option[ 0 ] == '\0') || match_pattern(lc_option, "lock"))
    {
        if (value == NULL)
        {
            return NO_ERROR;
        }
        else if (value[ 0 ] == '?')
        {
            ERE(pso("lock = %s\n", fs_lock_enable ? "t" : "f"));
        }
        else if (value[ 0 ] == '\0')
        {
            ERE(pso("External program locking is %s.\n",
                    fs_lock_enable ? "enabled" : "disabled"));
        }
        else
        {
            ERE(temp_boolean_value = get_boolean_value(value));
            ERE(set_lock_enable(temp_boolean_value));
        }
        result = NO_ERROR;
    }

    return result;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

static int set_lock_enable(int new_enable_value)
{
    static int first_time = TRUE;


    if (first_time)
    {
        add_cleanup_function(cleanup_lock);
        first_time = FALSE;
    }

    if ( ! fs_lock_enable && new_enable_value)
    {
        FILE* enable_fp;


        if (get_path_type(LOCK_ENABLE_FILE) == PATH_IS_REGULAR_FILE)
        {
            set_error("Unable to enable lock : %s exists.", LOCK_ENABLE_FILE);
            return ERROR;
        }

        NRE(enable_fp = kjb_fopen(LOCK_ENABLE_FILE, "w"));
        kjb_fclose(enable_fp);
    }
    else if (fs_lock_enable && !new_enable_value)
    {
        remove_lock_enable_file();
    }

    fs_lock_enable = new_enable_value;

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

static void cleanup_lock(void)
{


    if (fs_lock_enable)
    {
        remove_lock_enable_file();
        fs_lock_enable = FALSE;
    }
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

static void remove_lock_enable_file(void)
{


    kjb_unlink(LOCK_ENABLE_FILE);

}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

int check_for_freeze(void)
{


    if ( ! fs_lock_enable) return NO_ERROR;

    if (get_path_type(LOCK_REQUEST_FILE) == PATH_IS_REGULAR_FILE)
    {
        FILE* wait_fp;


        NRE(wait_fp = kjb_fopen(LOCK_GRANT_FILE, "w"));
        EPE(kjb_fclose(wait_fp));

        while (get_path_type(LOCK_REQUEST_FILE) == PATH_IS_REGULAR_FILE)
        {
            nap(200);
        }

        EPE(kjb_unlink(LOCK_GRANT_FILE));
    }

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                                is_interactive
 *
 * Determines if we are running interactively
 *
 * This routine returns true if we are running interactively, and false
 * otherwise.
 *
 * Index: standard library, I/O
 *
 * -----------------------------------------------------------------------------
*/

int is_interactive(void)
{
    static int interactive = NOT_SET;


    if (interactive == NOT_SET)
    {
        if ((kjb_isatty(fileno(stdin)) && (! is_in_background())))
        {
            interactive = TRUE;
        }
        else
        {
            interactive = FALSE;
        }
    }

    return interactive;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                                 get_user_id
 *
 * Copies user id into a buffer
 *
 * This routine copies the user id into the buffer user_id, which has size
 * max_len.
 *
 * Macros:
 *    BUFF_GET_USER_ID
 *
 * Index: standard library
 *
 * Related:
 *    BUFF_GET_USER_ID
 *
 * -----------------------------------------------------------------------------
*/

int get_user_id(char* user_id, size_t max_len)
{
   uid_t user_id_num;
   struct passwd *user_passwd_ptr;

   if (max_len == 0)
   {
       SET_ARGUMENT_BUG();
       return ERROR;
   }

   *user_id = '\0';

   user_id_num = getuid();

   user_passwd_ptr = getpwuid(user_id_num);

   if (user_passwd_ptr == NULL)
   {
       set_error("Unable to determine user id.%S");
       return ERROR;
   }

   kjb_strncpy(user_id, (user_passwd_ptr->pw_name), max_len);

   return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                                 get_group
 *
 *
 *
 * -----------------------------------------------------------------------------
*/

#ifdef VMS

/* VMS */ int get_group(group, max_len)
/* VMS */     char*  group;
/* VMS */     size_t max_len;
/* VMS */ {
/* VMS */     unsigned int group_id;
/* VMS */
/* VMS */
/* VMS */
/* VMS */     if (max_len <= 0)
/* VMS */     {
/* VMS */         SET_ARGUMENT_BUG();
/* VMS */         return ERROR;
/* VMS */     }
/* VMS */
/* VMS */     group_id = getgid();
/* VMS */     sprintf(group, "%d", group_id);
/* VMS */
/* VMS */     return NO_ERROR;
/* VMS */ }

#else

/* default */ int get_group(char* group, size_t max_len)
/* default */
/* default */
/* default */ {
/* default */
/* default */
/* default */     if (max_len == 0)
/* default */     {
/* default */         SET_ARGUMENT_BUG();
/* default */         return ERROR;
/* default */     }
/* default */
/* default */     *group = '\0';
/* default */
/* default */     set_bug("Get_group is not implemented on this platform.");
/* default */
/* default */     return ERROR;
/* default */ }

#endif

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                                 kjb_get_env
 *
 * KJB library wrapper for getenv()
 *
 * This routine copies the value of the environment variable whose name is in
 * the string env_var, into the buffer env_val_buff, of size max_len.
 *
 * Returns :
 *    On success NO_ERROR is returned. On failure, ERROR is returned and an
 *    appropriate error message is set.
 *
 * Macros:
 *    BUFF_GET_ENV
 *
 * Index: standard library
 *
 * Related:
 *    BUFF_GET_ENV
 *
 * -----------------------------------------------------------------------------
*/

int kjb_get_env(const char* env_var, char* env_val_buff, size_t max_len)
{
    char* env_val;


    if ((env_val_buff != NULL) && (max_len <= 0))
    {
        SET_ARGUMENT_BUG();
        return ERROR;
    }

    env_val = getenv(env_var);

    if (env_val == NULL)
    {
        set_error("Can't get value for environment variable %s.", env_var);

        if (env_val_buff != NULL)
        {
            *env_val_buff = '\0';
        }

        return ERROR;
    }
    else
    {
        if (env_val_buff != NULL)
        {
            if (strlen(env_val) >= max_len)
            {
                set_error("Value for environment variable %s is too long.", env_var);
                return ERROR;
            }
            kjb_strncpy(env_val_buff, env_val, max_len);
        }
        return NO_ERROR;
    }
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                           create_system_command_process
 *
 * Starts up system command process
 *
 * This routine starts up the system command process. It is not essential to
 * call this routine, as it is called automatically when the first system
 * command request is made, but it is recommended that programs which use
 * kjb_system() call routine at program startup (technically, before much
 * memory has been allocated). This reduces the overall VM usage.
 *
 * Note:
 *    This process is automatically terminated on program exit.
 *
 * Returns:
 *     NO_ERROR on sucess and ERROR on failure.
 *
 * Index: standard library, processes
 *
 * -----------------------------------------------------------------------------
*/

int create_system_command_process(void)
{
#ifdef UNIX
    int system_pid;


    if (KJB_IS_SET(fs_system_command_pid))
    {
        return NO_ERROR;
    }

    if (    (pipe(fs_system_command_pipe) == EOF)
         || (pipe(fs_system_result_pipe) == EOF)
       )
    {
        set_error("Creation of pipe for system command processes failed.");
        return ERROR;
    }

    /*
    dbi(fs_system_command_pipe[ READ_END ]);
    dbi(fs_system_command_pipe[ WRITE_END ]);
    dbi(fs_system_result_pipe[ READ_END ]);
    dbi(fs_system_result_pipe[ WRITE_END ]);
    */

    ERE(system_pid = kjb_fork());

    if (IS_PARENT(system_pid))
    {
#ifdef TEST
        /* The parent prints, then naps. The child naps for less time then prints. Voodo to
         * get the messages separately on the mac.  */
        TEST_PSE(("Parent process in create_system_command_process() is %d (IL4RT).\n", MY_PID));
        nap(400);
#endif 

        fs_system_command_parent_pid = MY_PID;

        add_cleanup_function(destroy_system_command_process);

        if (close(fs_system_result_pipe[ WRITE_END ]) == -1)
        {
            /* Just print a message and hope for the best. */
            p_stderr("Can't close write end of system command result pipe.%S");
        }

        if (close(fs_system_command_pipe[ READ_END ]) == -1)
        {
            /* Just print a message and hope for the best. */
            p_stderr("Can't close read end of system command pipe.%S");
        }

        fs_system_command_pid = system_pid;

        return NO_ERROR;
    }
    else /* Child */
    {
        char  int_buff[ 100 ];
        char  line[ MAX_SYSTEM_COMMAND_LINE_SIZE ];
        char  error_str[ 10000 ];
        char* line_pos;
        char* save_line_pos;
        int   result;

#ifdef TEST
        /* The parent prints, then naps. The child naps for less then prints.
         * Voodo to get the messages separately on the mac.  */
        nap(200);
        TEST_PSE(("Child process in create_system_command_process() is %d (IL4RT).\n", MY_PID)); 
#endif

        if (close(fs_system_result_pipe[ READ_END ]) == -1)
        {
            /* Just print a message and hope for the best. */
            p_stderr("Can't close read end of system command result pipe.%S");
        }

        if (close(fs_system_command_pipe[ WRITE_END ]) == -1)
        {
            /* Just print a message and hope for the best. */
            p_stderr("Can't close write end of system command pipe.%S");
        }

        EPE(kjb_signal(SIGINT, SIG_IGN));

        /*CONSTCOND*/
        while ( TRUE )
        {
            line[ 0 ] = '\0';

            result = BUFF_DGET_LINE(fs_system_command_pipe[ READ_END ], line);

            if (result == ERROR)
            {
                kjb_print_error();
                continue;
            }

            if (result == INTERRUPTED) continue;

            /*
             * Normal organized cleanup results in line being "quit." If the
             * parent dies for any of a number of reason such as a seg fault,
             * then result will be EOF. 
            */
            if ((STRCMP_EQ(line, "quit") || (result == EOF)))
            {
                kjb_exit( EXIT_SUCCESS);
                /*NOTREACHED*/
            }

            line_pos = line;

            if (line_pos[ 0 ] == '!')
            {
                line_pos++;

                save_line_pos = line_pos;

                while (*line_pos != '\0')
                {
                    if (*line_pos == 0x01)
                    {
                        *line_pos = '\n';
                    }
                    line_pos++;
                }

                line_pos = save_line_pos;

                result = kjb_system_2(line_pos, NULL);

                if (kjb_sprintf(int_buff, sizeof(int_buff), "%d\n", result) == ERROR)
                {
                    kjb_print_error();
                    continue;
                }

                if (kjb_write(fs_system_result_pipe[ WRITE_END ], int_buff, strlen(int_buff)) == ERROR)
                {
                    kjb_print_error();
                    continue;
                }

                if (result == ERROR)
                {
                    int num_lines;

                    kjb_get_error(error_str, sizeof(error_str));

                    if (error_str[ strlen(error_str) - 1 ] != '\n')
                    {
                        BUFF_TRUNC_CAT(error_str, "\n");
                    }

                    num_lines = count_char(error_str, '\n');

                    if (kjb_sprintf(int_buff, sizeof(int_buff), "%d\n", num_lines) == ERROR)
                    {
                        kjb_print_error();
                        continue;
                    }

                    if (kjb_write(fs_system_result_pipe[ WRITE_END ], int_buff, strlen(int_buff)) == ERROR)
                    {
                        kjb_print_error();
                        continue;
                    }

                    if (kjb_write(fs_system_result_pipe[ WRITE_END ], error_str, strlen(error_str)) == ERROR)
                    {
                        kjb_print_error();
                        continue;
                    }

                }
            }
            else if (result == INTERRUPTED)
            {
                /*EMPTY*/
                ; /* Do nothing. */
            }

            else
            {
                p_stderr("Possible program problem.\n");
                p_stderr("Invalid command (%s) sent to system command process.\n",
                         line);
            }
        }     /* while ( TRUE ); */
    }             /* End of child code */

    /*NOTREACHED*/

#else /* Case NOT UNIX follows. */

    /*
     * An error message, or even an error return is not very interesting here.
    */
    return NO_ERROR;
#endif 
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

void destroy_system_command_process(void)
{
#ifdef UNIX
    Signal_info save_broken_pipe_vec;
    Signal_info new_broken_pipe_vec;
    int         write_res;


    if (fs_system_command_pid == NOT_SET) return;

    if (fs_system_command_parent_pid != MY_PID)
    {
        TEST_PSE(("Skipping cleanup of the system command process (not the originating process).\n"));
        return;
    }

#ifdef TEST 
    TEST_PSE(("Begin cleaning up system command process (%d) (IL4RT).\n",
              fs_system_command_pid));
#endif 

    INIT_SIGNAL_INFO(new_broken_pipe_vec);
    new_broken_pipe_vec.SIGNAL_HANDLER = SIG_IGN;
    kjb_sigvec(SIGPIPE, &new_broken_pipe_vec, &save_broken_pipe_vec);

    write_res = kjb_write(fs_system_command_pipe[ WRITE_END ], "quit\n", 5);

    kjb_sigvec(SIGPIPE, &save_broken_pipe_vec, (Signal_info*)NULL);

    if (write_res != ERROR)
    {
#ifdef TEST
        int waitpid_result;

        waitpid_result = kjb_waitpid(fs_system_command_pid);
        TEST_PSE(("Wait result is %d (IL4RT).\n", waitpid_result));
#else
        kjb_waitpid(fs_system_command_pid);
#endif
    }
#ifdef TEST
    else
    {
        TEST_PSE(("Write to system command process pipe failed.\n"));
    }
#endif

    TEST_PSE(("Done waiting for system command  process to finish (IL4RT).\n"));

    fs_system_command_pid = NOT_SET;
#endif 
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                                 kjb_system
 *
 * KJB library version of system(2)
 *
 * On the surface, this function is similar to the unix system(2) routine except
 * that we follow the KJB error reporting conventions, we ensure that the user's
 * PATH variable is searched (it usually is anyway), the success/failure of the
 * effort is returned, and there is slightly better error reporting on failure.
 * In addition, the stderr of the executed command is added to error messages.
 * In addition, we create a process to handle the requests which are fed to it
 * via a pipe to reduce forking. This strategy has not been analyzed recently as
 * to whether there is a worthwhile performance gain. The function
 * kjb_system_2() can be used as if this is not desired. (The function
 * kjb_system() is implemented using kjb_system_2(). 
 *
 * If the command succeeds, then NO_ERROR is returned, otherwise ERROR is
 * returned with an error message being set. This routine can fail if there is a
 * problem initiating the command, or if the command itself fails.
 *
 * Index: standard library, processes
 *
 * -----------------------------------------------------------------------------
*/

int kjb_system(const char* command)
{
    char line[ MAX_SYSTEM_COMMAND_LINE_SIZE ];
    int  result;
    int  i;
    int  num_error_lines;
    char int_buff[ 100 ];
    char error_str[ 10000 ];
#ifdef __GNUC__
#ifdef DEBUGGING
    const int min_debug_level_for_standard = 25;
#endif
#endif


#ifdef __GNUC__
#ifdef DEBUGGING
    /*
    *  Dec 23, 04:
    *
    *  Actually, it now seems that gdb prefers our deafult way of doing things,
    *  so I have set the threshold higher.
    *
    *  Original comment:
    *
    *    gdb can't handle child processes doing something intersting.
    *
    */

    if (kjb_debug_level >= min_debug_level_for_standard)
    {
        static int first_time = TRUE;

        if (first_time)
        {
            first_time = FALSE;

            TEST_PSE(("Using standard system routine because of gcc and DEBUGGING "));
            TEST_PSE(("and debug level at least %d.\n", min_debug_level_for_standard));
        }

        result = system(command);

        if (result == -1)
        {
            set_error("Execution of %q failed.", command); 
            result = ERROR;
        }

        return result;
    }
    else
    {
        static int first_time = TRUE;

        if (first_time)
        {
            first_time = FALSE;

            TEST_PSE(("Using kjb_system() despite gcc and DEBUGGING.\n"));
            TEST_PSE(("As a result, gdb may not work properly.\n"));
            TEST_PSE(("(This has not been verified with recent versions of gdb).\n"));
            TEST_PSE(("Use a debug level at least %d to force the standard system().\n",
                      min_debug_level_for_standard));
        }
    }

#endif
#endif

    if (fs_system_command_pid == NOT_SET)
    {
        ERE(create_system_command_process());
    }

    BUFF_CPY(line, "!");
    BUFF_CAT(line, command);

    for (i = 0; i < MAX_SYSTEM_COMMAND_LINE_SIZE; i++)
    {
        if (line[ i ] == '\n')
        {
            line[ i ] = 0x01;
        }
        else if (line[ i ] == '\0')
        {
            break;
        }
    }

    BUFF_CAT(line, "\n");

    if (kjb_write(fs_system_command_pipe[ WRITE_END ], line, strlen(line)) == ERROR)
    {
        if (check_child(fs_system_command_pid) == PROCESS_IS_DEAD)
        {
            add_error("System command process appears to be dead.");
        }
        return ERROR;
    }

    BUFF_DGET_LINE(fs_system_result_pipe[ READ_END ], int_buff);

    if (ss1i(int_buff, &result) == ERROR)
    {
        if (check_child(fs_system_command_pid) == PROCESS_IS_DEAD)
        {
            add_error("System command process appears to be dead.");
        }
        return ERROR;
    }

    if (result == NO_ERROR)
    {
        return NO_ERROR;
    }

    kjb_clear_error();

    BUFF_DGET_LINE(fs_system_result_pipe[ READ_END ], int_buff);
    ERE(ss1i(int_buff, &num_error_lines));

    for (i = 0; i < num_error_lines; i++)
    {
        BUFF_DGET_LINE(fs_system_result_pipe[ READ_END ], error_str);
        add_error(error_str);
    }


    /*
    kjb_print_error();
    */

    UNTESTED_CODE();
    return result;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                                 kjb_system_2
 *
 * A helper for kjb_system(), wrapper for system()
 *
 * This function is different than kjb_system() in that 1) it dodges the process
 * that sits in the background to handle incomming system call requests; and 2)
 * it provides a simple return code for the command executed (if argument
 * prog_rc_ptr is not NULL). It is different than system() in that output to
 * stderr is collected as error messages (until the space for them is used up,
 * in which case they should be dropped --- we should check this still work!). 
 *
 * If argument prog_rc_ptr is not NULL, then *prog_rc_ptr will be set to the
 * shell return codes by the following table (not everything has been checked),
 * and, if none of these apply, then the return code of what running program
 * chooses to return, as you would expect. 
 *
 * |  1     Catchall for general errors
 * |  2     Misuse of shell builtins 
 * |  126   Command invoked cannot execute
 * |  127   "command not found" 
 * |  128   Invalid argument to exit
 * |  128+n Fatal error signal "n"
 * |  255   Exit status out of range
 *
 * This function has historical baggage due to trying to do what seems hard to
 * do easily which is to separate out the notions of 1) the shell failing to get
 * the command(s) running with the various IO redirections; and 2) the
 * subsequently well formed command reports an error return code. This is not
 * implemented due to difficulties --- it seems that doing so in the general
 * case would require parsing composite commands into smaller commands, checking
 * that they can be executed, and checking the IO redirects. Currently, the
 * cases we cannot handle well are composite cases that we do not want to do
 * this analysis, and so we adopt the shell's version of the situation, which
 * means that something like:
 * |  ls < NON_EXISTANT
 * will have an error code of "1" which may or may not be what we want (but it
 * is consistant with the shell). However, 
 * | NON_EXISTANT 
 * will get return code "127", and 
 * | NON_PERMITTED 
 * will get return code "126". 
 *
 * Index: standard library, processes
 *
 * -----------------------------------------------------------------------------
*/

int kjb_system_2(const char* command, int* prog_rc_ptr)
{
    pid_t command_pid;
    char  error_file_name[ MAX_FILE_NAME_SIZE ];
    FILE* error_fp = NULL;
#ifdef PLAY_WITH_THE_OLD_WAY
    char  error_buff[ MAX_NUM_ERROR_MESS * ERROR_MESS_BUFF_SIZE ];
#else 
    char  error_buff[ GET_LINE_BUFF_SIZE ];
#endif 
    int   command_status                                          = 0;
    char  cmd_error_file_name[ MAX_FILE_NAME_SIZE ];
    FILE* cmd_error_fp                                            = NULL;
#ifdef PLAY_WITH_THE_OLD_WAY
    /* There does not seem to be any advantage to the old way. If the command
     * cannot execute, we should get either RC 126 or 127. This is probably
     * enough information for those that need it. 
    */
    int   use_system = TRUE; 
#endif 


#ifdef PLAY_WITH_THE_OLD_WAY 
    if (prog_rc_ptr != NULL)
    {
        *prog_rc_ptr = ERROR;
        
        /* These are cases that break the exec way of doing this. If we can use
         * exec, we consider doing so. */
        if ( ! (    (FIND_CHAR_YES(command, ';'))
                 || (FIND_CHAR_YES(command, '|'))
                 || (FIND_CHAR_YES(command, '<'))
                 || (FIND_CHAR_YES(command, '>'))
                 || (FIND_CHAR_YES(command, '\''))
           )   )
        {
            use_system = FALSE;
        }
    }
#endif 

    ERE(BUFF_GET_TEMP_FILE_NAME(error_file_name));
    ERE(BUFF_GET_TEMP_FILE_NAME(cmd_error_file_name));

    if ((command_pid = kjb_fork()) == ERROR)
    {
        insert_error("Execution of %q failed.", command);
        return ERROR;
    }

    /*
    dbs(cmd_error_file_name);
    dbs(error_file_name); 
    */

    if (IS_CHILD(command_pid))
    {
        int exit_status = EXIT_SUCCESS;

        NRE(cmd_error_fp = kjb_fopen(cmd_error_file_name, "w"));
        NRE(error_fp = kjb_fopen(error_file_name, "w"));

        /* If this succeeds, printing to stderr will then be via the temp files, so
         * debug output may not be what you expect (although it should come out
         * eventually in most cases). */
        if (dup2(fileno(cmd_error_fp), fileno(stderr)) == EOF)
        {
            UNTESTED_CODE();
            kjb_fclose(cmd_error_fp); 
            set_error("Unable to set stderr error to %s.%S\n", cmd_error_file_name);
            return ERROR;
        }

#ifdef PLAY_WITH_THE_OLD_WAY
        if (use_system)
        {
#endif 
            command_status = system(command);
            
            if (WIFEXITED(command_status))
            {
                command_status = WEXITSTATUS(command_status);
            }

            exit_status = command_status; 
#ifdef PLAY_WITH_THE_OLD_WAY
        }
        else if (kjb_exec(command) == ERROR)    /* Only return is ERROR ! */
        {
            kjb_get_error(error_buff, sizeof(error_buff));

            kjb_fputs(error_fp, error_buff);
            kjb_fputs(error_fp, "\n");

            /* Closest possible option? Shold be either 126 or 127 */
            exit_status = 127;
        }
#endif 
        kjb_fclose(error_fp);
        kjb_fclose(cmd_error_fp);

#ifdef HOW_IT_WAS_AUG_03_2017
        _exit(exit_status);   /* Don't use kjb_exit, because we don't want
                              // to clean up.
                              */
#else 
        /* I am not sure why I wrote that we do not want to cleanup, so let's
         * try cleaning up for a while. 
        */
        kjb_exit(exit_status);
#endif 
        /*NOTREACHED*/
    }
    else
    {
        int exit_argument = NOT_SET;
        int termination_signal;
        int waitpid2_result;
        int result = NO_ERROR;

        waitpid2_result = kjb_waitpid2(command_pid, &exit_argument,
                                       &termination_signal);

        /*
        dbi(waitpid2_result);
        dbi(exit_argument);
        dbi(termination_signal);
        */

        if (waitpid2_result != NO_ERROR) 
        {
            set_error("Execution of %q reported failure via kjb_waitpid2 return of (%d).", command,
                      exit_argument);
            result = ERROR;
        }
        else if (KJB_IS_SET(exit_argument)) 
        {
            if (get_path_type(error_file_name) == PATH_IS_REGULAR_FILE) 
            {
                 error_fp = kjb_fopen(error_file_name, "r");
            }
            if (get_path_type(cmd_error_file_name) == PATH_IS_REGULAR_FILE)
            {
                 cmd_error_fp = kjb_fopen(cmd_error_file_name, "r");
            }
             
            if (exit_argument != EXIT_SUCCESS)
            {
                set_error("Execution of %q reported failure status (%d).", command,
                          exit_argument);
            }

            if (error_fp != NULL)
            {
                while (BUFF_FGET_LINE(error_fp, error_buff) != EOF)
                {
                    if (exit_argument == EXIT_SUCCESS)
                    {
                        ERE(fput_line(stderr, error_buff));
                    }
                    else
                    {
                        add_error(error_buff);
                    }
                }
            }

            if (cmd_error_fp != NULL)
            {
                while (BUFF_FGET_LINE(cmd_error_fp, error_buff) != EOF)
                {
                    if (exit_argument == EXIT_SUCCESS)
                    {
                        ERE(fput_line(stderr, error_buff));
                    }
                    else
                    {
                        add_error(error_buff);
                    }
                }
            }

            push_error_action(IGNORE_ERROR_ON_ERROR); 
            kjb_fclose(error_fp);
            kjb_unlink(error_file_name);
            kjb_fclose(cmd_error_fp);
            kjb_unlink(cmd_error_file_name);
            pop_error_action(); 

            /* This was originally an attempt to have kjb_system_2() being able
             * to report the difference between the success of executing the
             * program, versus the program reporting success. But this is
             * looking pretty hard to do. For now, failure of any sort is an
             * ERROR, but the return code is massaged when we pass in a pointer
             * for it. 
            */
            if (prog_rc_ptr != NULL)
            { 
                *prog_rc_ptr = exit_argument;
            }

            if (exit_argument != EXIT_SUCCESS)
            {
                result = ERROR;
            }
        }
        else if (KJB_IS_SET(termination_signal))
        {
            set_error("Execution of %q terminated by signal (%d).",
                      command, termination_signal);
            result = ERROR;
        }
        else
        {
            if (prog_rc_ptr != NULL)
            { 
                *prog_rc_ptr = exit_argument;
            }
            result = NO_ERROR;
        }

        return result;
    }
    /*NOTREACHED*/
#ifdef __GNUC__
    return ERROR;
#endif

}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

#ifdef UNIX

/* =============================================================================
 *                                 kjb_exec
 *
 * KJB library version of execvp(2)
 *
 * This routine is essentially a replacement for exec(2), and is basically a
 * a wrapper for execvp(2).
 *
 * This routine does not return unless there is some problem, in which case
 * ERROR is returned with an appropriate error message being set.
 *
 * Index: standard library, processes
 *
 * -----------------------------------------------------------------------------
*/

/* UNIX */ int kjb_exec(const char* command)
/* UNIX */
/* UNIX */ {
/* UNIX */     int argc;
/* UNIX */     char** argv;
/* UNIX */     int i, ii;
/* UNIX */     char next_program_arg[ 2000 ];
/* UNIX */     const char* command_pos;
/* UNIX */
/* UNIX */     argc=0;
/* UNIX */     command_pos = command;
/* UNIX */
/* UNIX */     /* Dry run to count the number of arguments. */
/* UNIX */     while(BUFF_CONST_MQ_GET_TOKEN_OK(&command_pos,
/* UNIX */                                      next_program_arg))
/* UNIX */     {
/* UNIX */         argc++;
/* UNIX */     }
/* UNIX */
/* UNIX */     argv = N_TYPE_MALLOC(char*, argc+1);
/* UNIX */
/* UNIX */     if (argv == NULL)
/* UNIX */     {
/* UNIX */         set_error(
/* UNIX */               "Memory allocation failed preparing to execute %q.\n",
/* UNIX */               command);
/* UNIX */         return ERROR;
/* UNIX */     }
/* UNIX */
/* UNIX */     command_pos = command;
/* UNIX */     i = 0;
/* UNIX */
/* UNIX */     while(BUFF_CONST_MQ_GET_TOKEN_OK(&command_pos,
/* UNIX */                                      next_program_arg))
/* UNIX */     {
/* UNIX */         strip_quotes(next_program_arg);
/* UNIX */         argv[i] = kjb_strdup(next_program_arg);
/* UNIX */
/* UNIX */         if (argv[i] == NULL)
/* UNIX */         {
/* UNIX */             set_error(
/* UNIX */                "Memory allocation failed preparing to execute %q.\n",
/* UNIX */                command);
/* UNIX */
/* UNIX */             for (ii = 0; ii < i; ii++)
/* UNIX */             {
/* UNIX */                  kjb_free(argv[ii]);
/* UNIX */             }
/* UNIX */             kjb_free(argv);
/* UNIX */
/* UNIX */             return ERROR;
/* UNIX */         }
/* UNIX */
/* UNIX */         i++;
/* UNIX */     }
/* UNIX */
/* UNIX */     argv[argc]=NULL;
/* UNIX */
/* UNIX */     if (argv[0] == NULL)
/* UNIX */     {
/* UNIX */         set_error("Null command is ignored.");
/* UNIX */
/* UNIX */         for (ii = 0; ii < argc; ii++)
/* UNIX */         {
/* UNIX */             kjb_free(argv[ii]);
/* UNIX */         }
/* UNIX */         kjb_free(argv);
/* UNIX */
/* UNIX */         return ERROR;
/* UNIX */     }
/* UNIX */     else if (execvp(argv[0], argv) == EOF)      /* Error return */
/* UNIX */     {
/* UNIX */         set_error("Could not execute %q.%S", command);
/* UNIX */
/* UNIX */         for (ii = 0; ii < argc; ii++)
/* UNIX */         {
/* UNIX */             kjb_free(argv[ii]);
/* UNIX */         }
/* UNIX */         kjb_free(argv);
/* UNIX */
/* UNIX */         return ERROR;
/* UNIX */     }
/* UNIX */
/* UNIX */     _exit(EXIT_FAILURE);
/* UNIX */
/* UNIX */     /*NOTREACHED*/
/* UNIX */     return ERROR; /* Keep some compilers happy. */
/* UNIX */ }

#endif   /* UNIX */

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                                 kjb_waitpid
 *
 * KJB library process waiting
 *
 * The routine kjb_waitpid waits until a specified child process terminates or
 * a signal is received (by the process doing the waiting). Unlike the typical
 * system waitpid(), if the process only changes state, this routine continues
 * waiting. Once the child process terminates, then NO_ERROR is returned.  If
 * the reason for termination is required, then the routine kjb_waitpid2 should
 * be used.   If the waiting process is interrupted during the wait, then
 * INTERRUPTED is returned instead.
 *
 * Notes:
 *    See check_child/check_child2 for similar capability that has an immediate
 *    return.
 *
 * Returns:
 *    If the child process terminates, then NO_ERROR is returned.  If a signal
 *    interrupts the wait, then INTERRUPTED is returned. ERROR may be returned
 *    if there is a problem with the call.
 *
 * Related:
 *    kjb_waitpid2, check_child, check_child2, terminate_child_process
 *
 * Index: signals, processes, standard library
 *
 *
 * -----------------------------------------------------------------------------
*/

#ifdef UNIX

/* UNIX */ int kjb_waitpid(pid_t pid)
/* UNIX */
/* UNIX */ {
/* UNIX */     int dummy_exit_argument;
/* UNIX */     int dummy_termination_signal;
/* UNIX */
/* UNIX */      /* FIXME --- should be able to send in NULL args. */
/* UNIX */     return kjb_waitpid2(pid, &dummy_exit_argument,
/* UNIX */                         &dummy_termination_signal);
/* UNIX */
/* UNIX */ }
/* UNIX */

#endif     /* UNIX */

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                                 kjb_waitpid2
 *
 * KJB library process waiting
 *
 * The routine kjb_waitpid2 waits until a specified child process terminates or
 * a signal is received (by the process doing the waiting). Unlike the typical
 * system waitpid(), if the process only changes state, this routine continues
 * waiting. Once the child process terminates, then NO_ERROR is returned, and
 * the argument passed to exit2() is put into the location exit_argument_ptr,
 * or the signal causing termination is put into the location
 * termination_signal_ptr. Only one of these two is relavent. The other is
 * set to NOT_SET.   If the waiting process is interrupted during the wait,
 * then INTERRUPTED is returned instead.
 *
 * Notes:
 *    See check_child/check_child2 for similar capability that has an immediate
 *    return.
 *
 * Returns:
 *    If the child process terminates, then NO_ERROR is returned, and either
 *    *exit_argument_ptr or *termination_signal_ptr is set. If a signal
 *    interrupted the wait, then INTERRUPTED is returned. ERROR may be returned
 *    if there is a problem with the call.
 *
 * Related:
 *    kjb_waitpid, check_child, check_child2, terminate_child_process
 *
 * Index: signals, processes, standard library
 *
 *
 * ==> UNTESTED (other than comon cases).
 * ==> CHECK return values for next.
 * ==> CHECK setting of exit_argument_ptr and termination_signal_ptr
 *
 * -----------------------------------------------------------------------------
*/

#ifdef UNIX

 int kjb_waitpid2(pid_t pid,
                  int* exit_argument_ptr,
                  int* termination_signal_ptr)
 {
     int   errno_copy  = 0;
     pid_t waitpid_res;
     int   status      = 0;

     *exit_argument_ptr = NOT_SET;
     *termination_signal_ptr = NOT_SET;

     waitpid_res = pid;

     while (waitpid_res == pid)
     {
         waitpid_res = waitpid(pid, &status, 0);

         /*
         dbi(waitpid_res);
         dbi(status);
         dbi(WIFEXITED(status));
         */

         if (waitpid_res == -1)
         {
#ifdef error          /* GNU defines errno as a macro, causing warnings. */
             IMPORT int errno;
#endif

             errno_copy = errno;
         }
         else if ((waitpid_res == pid) && (WIFEXITED(status)))
         {
             *exit_argument_ptr = WEXITSTATUS(status);

#ifdef REPORT_PROCESSES
             TEST_PSE(("Process %ld exited with status %d (IL4RT).\n",
                       (long)pid, *exit_argument_ptr));
#endif
             return NO_ERROR;
         }
         else if ((waitpid_res == pid) && (WIFSIGNALED(status)))
         {
             *termination_signal_ptr = WTERMSIG(status);
             *exit_argument_ptr = WEXITSTATUS(status);

             TEST_PSE(("Process %ld exited due to signal %d (IL4RT).\n",
                       (long)pid, *termination_signal_ptr));

             return NO_ERROR;
         }
     }

     if (waitpid_res != -1)
     {
         SET_CANT_HAPPEN_BUG();
         return ERROR;
     }
     else if (errno_copy == 0)
     {
         return PROCESS_IS_DEAD;
     }
     else if (errno_copy == EINTR)
     {
         return INTERRUPTED;
     }
     else if (errno_copy == ECHILD)
     {
         /* I used to treat this as a bug, but Mike's videograb     */
         /* prevents zombies from forming. It is a big hassle to    */
         /* re-write the videograb code, and likely not worth it,   */
         /* as it will be obsolete soon enough.                     */
#ifdef STRICT_PROCESS_ACOUNTING
#ifdef TEST
         IMPORT int errno;
         char buff[ 1000 ];

         kjb_sprintf(buff, sizeof(buff), "%s\n%s\n%s\n%s\n",
                  "No children in kjb_waitpid2.",
                  "Likely they all have been waited on already.",
                  "Or SIGCHLD may have been disabled ",
                  "(Mike's videograb code does this).");
         set_bug(buff);
#else
         set_error("Waiting for a children, but there are none.");
         add_error("Alternately, all been waited on already.");
         add_error("This is likely due to a minor program problem.");
#endif
         return ERROR;
#else
#ifdef REPORT_PROCESSES
         TEST_PSE(("No children in kjb_waitpid2.\n"));
         TEST_PSE(("Likely they all have been waited on already.\n"));
         TEST_PSE(("Or SIGCHLD may have been disabled "));
#endif
         return PROCESS_IS_DEAD;
#endif
     }
     else
     {
#ifdef TEST
#ifdef error      /* GNU defines errno as a macro, causing warnings. */
         IMPORT int errno;
#endif
         char buff[ 100 ];

         errno = errno_copy; /* A hack, I admint */
         kjb_sprintf(buff, sizeof(buff),
                     "kjb_waitpid2 failed for process %ld.%S",
                     (long)pid);
         set_bug(buff);
#else
         set_error("Failed to get status of child proces.");
#endif
         return ERROR;
     }
          }

#endif     

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                                check_child
 *
 * Determines if child process has died.
 *
 * The routine returns NO_ERROR if the named child process is still alive,
 * PROCESS_IS_DEAD if it has died, INTERRUPTED if the wait was interrupted (not
 * likely in this case) and ERROR if there are problems with the call. Probably
 * the most likely cause of ERROR is if the process is not a child process of
 * the caller. If the reason for death is required, use check_child2 instead.
 *
 * Notes:
 *    Unlike kjb_waitpid/kjb_waitpid2, this routine returns immediately in all
 *    circumstances. If is necessary to wait until the child changes status,
 *    then one of these functions should be used.
 *
 * Related:
 *    kjb_waitpid, kjb_waitpid2, check_child2, terminate_child_process
 *
 * Index: signals, processes, standard library
 *
 * -----------------------------------------------------------------------------
*/

#ifdef UNIX

/* UNIX */ int check_child(pid_t pid)
/* UNIX */
/* UNIX */ {
/* UNIX */     int dummy_exit_argument;
/* UNIX */     int dummy_termination_signal;
/* UNIX */
/* UNIX */     /* FIXME --- should be able to send in NULL args. */
/* UNIX */     return check_child2(pid, &dummy_exit_argument,
/* UNIX */                           &dummy_termination_signal);
/* UNIX */ }

#endif     /* UNIX */

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                                check_child2
 *
 * The routine returns NO_ERROR if the named child process is still alive,
 * PROCESS_IS_DEAD if it has died, INTERRUPTED if the wait was interrupted (not
 * likely in this case) and ERROR if there are problems with the call. Probably
 * the most likely cause of ERROR is if the process is not a child process of
 * the caller.  In addition, if the child is dead, then the argument passed to
 * exit2() is put into the location exit_argument_ptr, OR the signal causing
 * termination is put into the location termination_signal_ptr. Only one of
 * these two is relavent. The other is set to NOT_SET.
 *
 * Notes:
 *    Unlike kjb_waitpid/kjb_waitpid2, this routine returns immediately in all
 *    circumstances. If is necessary to wait until the child changes status,
 *    then one of these functions should be used.
 *
 * Related:
 *    kjb_waitpid, kjb_waitpid2, check_child, terminate_child_process
 *
 * Index: signals, processes, standard library
 *
 * -----------------------------------------------------------------------------
*/

#ifdef UNIX

/* UNIX */ int check_child2(pid_t pid, int* exit_argument_ptr, int* termination_signal_ptr)
/* UNIX */
/* UNIX */
/* UNIX */
/* UNIX */ {
/* UNIX */     pid_t waitpid_res;
/* UNIX */     int options = WNOHANG;

#ifdef NeXT
/* NEXT */     union wait status;
#else
/* NOT NEXT */ int status = 0;
#endif

/* UNIX */
/* UNIX */     *exit_argument_ptr = NOT_SET;
/* UNIX */     *termination_signal_ptr = NOT_SET;
/* UNIX */

#ifdef NeXT
/* NEXT */     waitpid_res = wait4(pid, &status, options, NULL);
#else
/* NOT NEXT */ waitpid_res = waitpid(pid, &status, options);
#endif

/* UNIX */     if (waitpid_res == pid)
/* UNIX */     {

#ifdef NeXT

/* NEXT */         if (WIFEXITED(status))
/* NEXT */         {
/* NEXT */             *exit_argument_ptr = status.w_retcode;
/* NEXT */             return PROCESS_IS_DEAD;
/* NEXT */         }
/* NEXT */         else if (WIFSIGNALED(status))
/* NEXT */         {
/* NEXT */             *termination_signal_ptr =  status.w_termsig;
/* NEXT */             return PROCESS_IS_DEAD;
/* NEXT */         }
/* NEXT */         else
/* NEXT */         {
/* NEXT */             return NO_ERROR;
/* NEXT */         }

#else

/* NOT NEXT */
/* NOT NEXT */     if (WIFEXITED(status))
/* NOT NEXT */     {
/* NOT NEXT */         *exit_argument_ptr = WEXITSTATUS(status);
/* NOT NEXT */         return PROCESS_IS_DEAD;
/* NOT NEXT */     }
/* NOT NEXT */     else if (WIFSIGNALED(status))
/* NOT NEXT */     {
/* NOT NEXT */         *termination_signal_ptr = WTERMSIG(status);
/* NOT NEXT */         return PROCESS_IS_DEAD;
/* NOT NEXT */     }
/* NOT NEXT */     else
/* NOT NEXT */     {
/* NOT NEXT */         return NO_ERROR;
/* NOT NEXT */     }

#endif

/* UNIX */     }
/* UNIX */     else if (waitpid_res == 0)
/* UNIX */     {
/* UNIX */         return NO_ERROR;
/* UNIX */     }
/* UNIX */     else if (waitpid_res == -1)
/* UNIX */     {
#ifdef error      /* GNU defines errno as a macro, causing warnings. */
/* UNIX */         IMPORT int errno;
#endif
/* UNIX */
/* UNIX */         if (errno == 0)    /* Possible, because of WNOHANG */
/* UNIX */         {
#ifndef SUN5
                       UNTESTED_CODE();
#endif
/* UNIX */             return PROCESS_IS_DEAD;
/* UNIX */         }
/* UNIX */         else if (errno == ECHILD)
/* UNIX */         {
#ifndef HPUX
#ifndef SUN5
                       UNTESTED_CODE();
#endif
#endif
#ifdef REPORT_PROCESSES
/* UNIX */             TEST_PSE(("No children in check_child2.\n"));
#endif
/* UNIX */
/* UNIX */             return PROCESS_IS_DEAD;
/* UNIX */         }
/* UNIX */         else if (errno == EINTR)
/* UNIX */         {
/* UNIX */             return INTERRUPTED;
/* UNIX */         }
/* UNIX */         else
/* UNIX */         {
#ifdef TEST
/* UNIX */             char buff[ 100 ];
/* UNIX */
/* UNIX */             kjb_sprintf(buff, sizeof(buff),
/* UNIX */                         "check_child2 failed for process %ld.%S",
/* UNIX */                         (long)pid);
/* UNIX */             set_bug(buff);
#else
/* UNIX */             set_error("Failed to get status of child process.%S");
#endif
/* UNIX */             return ERROR;
/* UNIX */         }
/* UNIX */     }
/* UNIX */     SET_CANT_HAPPEN_BUG();
/* UNIX */     return ERROR;
/* UNIX */ }

#endif /* ------------   #ifdef UNIX   ------------    */

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

#define MAX_NUM_CLEANUP_FUNCTIONS      5000

static int fs_num_cleanup_functions = 0;
static void (*fs_cleanup_functions[ MAX_NUM_CLEANUP_FUNCTIONS ])(void);

#ifdef TEST
  static char fs_cleanup_call_file[ MAX_NUM_CLEANUP_FUNCTIONS ][ MAX_FILE_NAME_SIZE ];
  static int  fs_cleanup_call_line[ MAX_NUM_CLEANUP_FUNCTIONS ];
#endif

static int fs_doing_cleanup = FALSE;

/* ------------------------------------------------------------------------- */

void arrange_cleanup(void)
{
#ifdef KJB_HAVE_ATEXIT
#if 0 /* was ifdef HOW_IT_WAS_08_01_21 */
    /*
     * Changed the following bit of code clarity. We need to have this called as
     * soon as possible and by any routine that relies on cleanup. But possible
     * callers are outside this file. Thus the use of the counter
     * fs_num_cleanup_functions, which can only be incremented after calling
     * this routine, is safe in that kjb_cleanup() gets registered if needed,
     * but does not strictly ensure that there is only one registration. 
    */
    if (fs_num_cleanup_functions == 0)
    {
        atexit(kjb_cleanup);
    }
#else
    static int first_time = TRUE;

    if (first_time)
    {
        first_time = FALSE;
        atexit(kjb_cleanup);
    }
#endif 
#endif
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                                reset_cleanup_for_fork
 *
 * Forgets cleanup inherited by the child.
 *
 * We assume that any cleanup already in place is about global/static data,
 * largely freeing for the purposes of not having false positives in memory
 * allocation. So, it seems safe enough just to reset the counter when we fork. 
 *
 * Currently this routine is only used by kjb_fork(), and that is probably how
 * it should be. 
 *
 * Note this routine was added relatively recently compared to the other cleanup
 * code (May 2014). 
 *
 * Recent addemdum (Aug 2017). This routine is no longer used and will probably
 * be turfed soon.
 *
 * Index: standard library
 *
 * -----------------------------------------------------------------------------
*/
 
void reset_cleanup_for_fork()
{
    /*
    fs_num_cleanup_functions = 0;
    */
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                                add_cleanup_function
 *
 * Adds a function to be executed at program exit
 *
 * This function is somewhat similar to the system routine atexit(). However,
 * there are important differences, including  a much larger limit on the number
 * of functions allowed.
 *
 * Index: standard library
 *
 * -----------------------------------------------------------------------------
*/

#ifdef TEST
    int debug_add_cleanup_function(void (*cleanup_fn) (void),
                                   const char* file_name, int line_number)
#else
    int add_cleanup_function(void (*cleanup_fn) (void))
#endif
    {
        int flush_res;
        int result;
        Error_action save_error_action = get_error_action();

        /*
        // A hack to ensure that l_sys_io.c:initialize_cache is the first
        // cleanup function added.
        */
        flush_res = kjb_fflush((FILE*)NULL);

        if (flush_res == ERROR)
        {
            set_error_action(FORCE_ADD_ERROR_ON_ERROR);
        }

#ifdef TEST
        result = debug_add_cleanup_function_guts(cleanup_fn, file_name,
                                                 line_number);
#else
        result = add_cleanup_function_guts(cleanup_fn);
#endif

        if (flush_res == ERROR) result = ERROR;

        set_error_action(save_error_action);

        return result;
    }

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

#ifdef TEST
    static int debug_add_cleanup_function_guts
    (
        void (*cleanup_fn) (void),
        const char* file_name,
        int line_number
    )
#else
    static int add_cleanup_function_guts(void (*cleanup_fn) (void))
#endif
    {
#ifdef TEST
        extern int kjb_debug_level;
#endif
        static int  already_adding_cleanup_function = FALSE;


        arrange_cleanup();

        /*
        // We can't use any of the usual functions such as kjb_exit() or
        // set_bug() since these may themselves call kjb_cleanup(), causing
        // problems.
        */

        /*
        // First guard against re-entry.
        */

        if (fs_doing_cleanup)
        {
#ifdef TEST
            kjb_signal(SIGABRT, SIG_DFL);
            fprintf(stderr,
                 "<<TEST>> Process %ld called add_cleanup_function while cleaning up!\n",
                 (long)MY_PID);
            fprintf(stderr, "Call was from %s:%d.\n", file_name, line_number);
            abort();
#else
            fprintf(stderr, "Fatal error\n");
            _exit(EXIT_FAILURE);
#endif

            /*NOTREACHED*/
            return ERROR;
        }
        else if (already_adding_cleanup_function)
        {
#ifdef TEST
            kjb_signal(SIGABRT, SIG_DFL);
            fprintf(stderr,
                    "<<TEST>> Process %ld called add_cleanup_function recursively.\n",
                    (long)MY_PID);
            fprintf(stderr, "Second call was from %s:%d.\n",
                    file_name, line_number);
            abort();
#else
            fprintf(stderr, "Fatal error\n");
            _exit(EXIT_FAILURE);
#endif

            /*NOTREACHED*/
            return ERROR;
        }
        else
        {
            already_adding_cleanup_function = TRUE;

            if (fs_num_cleanup_functions >= MAX_NUM_CLEANUP_FUNCTIONS)
            {
#ifdef TEST
                kjb_signal(SIGABRT, SIG_DFL);

                fprintf(stderr,
                        "Max number of cleanup functions (%d) exceeded.\n",
                        MAX_NUM_CLEANUP_FUNCTIONS);
                abort();
#else
                fprintf(stderr, "Fatal error\n");
                _exit(EXIT_FAILURE);
#endif
                /*NOTREACHED*/
                return ERROR;
            }

            fs_cleanup_functions[ fs_num_cleanup_functions ] = cleanup_fn;

#ifdef TEST
            if (kjb_debug_level > 3)
            {
                fprintf(stderr, "<<TEST>> Process %ld adding cleanup[ %d ] : %s:%d (IL4RT).\n",
                       (long)MY_PID, fs_num_cleanup_functions, file_name,
                       line_number);
                fflush(stdout);
            }

            BUFF_CPY(fs_cleanup_call_file[ fs_num_cleanup_functions ], file_name);
            fs_cleanup_call_line[ fs_num_cleanup_functions ] = line_number;
#endif
            fs_num_cleanup_functions++;

            already_adding_cleanup_function = FALSE;

            return NO_ERROR;
        }
    }

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                                kjb_cleanup
 *
 * Calls all the cleanup functions
 *
 * This routine calls all the cleanup functions and performs a few extra cleanup
 * tasks.   This routine is normally called by other KJB library routines such
 * as kjb_exit(). Thus it is not normally necessary to call this function.
 * However, under some circumstances (atexit() is not available) an explicit
 * call at the very end of main() is warrented. Thus I normally include a call
 * to kjb_cleanup() as the last statement in main(). In this case, under normal
 * circumstances, kjb_cleanup() will be called twice, which is OK.
 *
 * Index: standard library
 *
 * -----------------------------------------------------------------------------
*/

void kjb_cleanup(void)
{

    /* Theoretically heap check 2 should not in place in the moment. */
    DISABLE_HEAP_CHECK_2();

    kjb_cleanup_guts( FALSE  /* abort_flag */ );
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                          kjb_cleanup_for_abort
 *
 * Calls the cleanup functions appropriate with aborting
 *
 * This routine is similar to kjb_cleanup, but several steps are omitted. For
 * example, it does not make sense to print out allocated memory in the case of
 * an abort. This function is not likely to be used outside the KJB library
 * code.
 *
 * Index: standard library, debugging
 *
 * -----------------------------------------------------------------------------
*/

void kjb_cleanup_for_abort(void)
{

    /* Theoretically heap check 2 should not in place in the moment. */
    DISABLE_HEAP_CHECK_2();

    /*
    // Don't check the heap while we cleanup.
    */
    (void)set_heap_options("heap-checking", "off");

    kjb_cleanup_guts( TRUE  /* abort_flag */ );
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

#ifdef TRACK_MEMORY_ALLOCATION
static void kjb_cleanup_guts(int abort_flag)
#else
/*ARGSUSED*/
static void kjb_cleanup_guts(int __attribute__((unused)) dummy_abort_flag)
#endif
{
#ifdef TEST
    IMPORT int kjb_debug_level;
    IMPORT int kjb_fork_depth; 
#endif 
    IMPORT int kjb_cleanup_started; 
    static int has_been_called = FALSE;
    int i;

#ifdef TEST
    if (kjb_debug_level > 2)
    {
        /* Too dangerous to use KJB IO at this point, so no
         * TEST_PSE(). */
        fprintf(stderr, "<<TEST>> Process %ld (fork depth %d) is cleaning up using %d functions (IL4RT).\n",
                (long)MY_PID, kjb_fork_depth, fs_num_cleanup_functions);
    }
#endif 


    /* Theoretically heap check 2 should not be in place in the moment. */
    DISABLE_HEAP_CHECK_2();

    /*
    // Guard against reentry.
    */
    if ( ! fs_doing_cleanup)
    {
        /* 
         * This global currently has the single purpose of switching the action
         * of debug print statements to NOT use the library.
        */
        kjb_cleanup_started = TRUE;

        fs_doing_cleanup = TRUE;


        /*
        // Important: Practically anything using the KJB library  (especially
        // I\O) should be inside the execute only once part, as the first call
        // to this routine dismantles a lot of stuff, and it has to be safe to
        // call it a second time.
        */

        if ( ! has_been_called )
        {
            /*
            // To help with debugging, write all output for non-cleanup stuff
            // before we clean up. This is especially useful when we are headed
            // for a core dump or an abort.
            */

            kjb_safe_fflush((FILE*)NULL);

#ifdef TRACK_MEMORY_ALLOCATION
            if ( ! abort_flag)
            {
                (void)optimize_free();
            }
#endif

            /*
            // Call the functions in reverse order of being set.
            */
            /* fs_num_cleanup_functions--; */

            while (fs_num_cleanup_functions > 0) 
            /* for (i = fs_num_cleanup_functions; i >= 0; i--) */
            {
                i = fs_num_cleanup_functions - 1;
#ifdef TEST
                if (kjb_debug_level > 3)
                {
                    /* Too dangerous to use KJB IO at this point, so no
                     * TEST_PSE(). */
                    fprintf(stderr, "<<TEST>> Fork depth %d process %ld doing cleanup[ %d ] : %s:%d (IL4RT).\n",
                             kjb_fork_depth, (long)MY_PID, i, fs_cleanup_call_file[ i ],
                             fs_cleanup_call_line[ i ]);
                }
#endif

                if (fs_cleanup_functions[ i ] != NULL)
                {
                    (*(fs_cleanup_functions[ i ]))();
                }
#ifdef TEST
                else
                {
                    fflush(stdout);
                    fprintf(stderr, "\n<<TEST>> NULL cleanup function.\n\n");
                    fflush(stderr);
                    abort();
                }
#endif
                fs_num_cleanup_functions--;
            }

            /*
            // Now do cleanup which is too too dangerous to add as a call to
            // add_cleanup_function() because they could be called by other
            // cleanup functions.
            //
            // Also, do cleanup that needs to be last, i.e., after
            // the "too dangerous ones", such as print_allocated_memory().
            //
            // Finally, do cleanup that should be done regardless of
            // circumstances like reset_initial_terminal_settings(); .
            */

#ifdef TRACK_MEMORY_ALLOCATION
            free_line_reentry_queue();
#if 0 /* was ifdef OBSOLETE */
            destroy_atn_queue();
#else
            destroy_sig_queue();
#endif
#endif


#ifdef UNIX
            if (fs_process_generation == 0)
            {
#ifdef TEST
                extern int kjb_debug_level;


                if (kjb_debug_level > 3)
                {
                    fprintf(stderr, 
                            "<<TEST>> Process %ld is resetting initial terminal settings (IL4RT).\n", 
                            (long)MY_PID);
                }
#endif
                reset_initial_terminal_settings();
            }
#endif


#ifdef TRACK_MEMORY_ALLOCATION
            if ( ! abort_flag) 
            {
                print_allocated_memory((FILE*)NULL);
            }
#endif

            has_been_called = TRUE;
        }

        fs_doing_cleanup = FALSE;
    }
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                                  kjb_optional_abort
 *
 * Calls kjb_abort if the enviroment variable KJB_OPTIONAL_ABORT is set.
 *
 * This routine calls kjb_abort if the enviroment variable KJB_OPTIONAL_ABORT is
 * set (to any value). It is used to support aborts that are associated with
 * warnings that might be false alarms. 
 *
 * Index: standard library, debugging
 *
 * -----------------------------------------------------------------------------
*/

void kjb_optional_abort(void)
{
    char env_buff[ 100 ];

    if (BUFF_GET_ENV("KJB_OPTIONAL_ABORT", env_buff) != ERROR)
    {
        kjb_abort();
    }
    else 
    {
        warn_pso("Optional abort skipped because the enviroment variable KJB_OPTIONAL_ABORT is NOT set.\n");
    }
}

/* =============================================================================
 *                                  kjb_abort
 *
 * The KJB library wrapper for abort
 *
 * This function is basically a wrapper for abort(). However, it makes sure that
 * cleanup is done before aborting, which is quite useful for debugging when
 * there are child processes involved, because the children would otherwise get
 * the abort signal as well, and in that case, which process does the corefile
 * belong to?
 *
 * In addition, kjb_abort() does not abort() in production code. In this case, a
 * simple error message is printed.
 *
 * Index: standard library, debugging
 *
 * -----------------------------------------------------------------------------
*/

#ifdef TEST
#    define TEST_OR_DEBUGGING
#else
#    ifdef DEBUGGING
#        define TEST_OR_DEBUGGING
#    endif
#endif

void kjb_abort(void)
{
#ifdef TEST_OR_DEBUGGING
    static int guard           = FALSE;
    int        abort_confirmed = FALSE;
    int        interactive_res;
    int        confirm_res;


    if (guard)
    {
        fprintf(stderr, "\n");
        fprintf(stderr, "Reentry of kjb_abort.\n");
        fprintf(stderr, "This is likely due to a second abort while cleaning up.\n");
        fprintf(stderr, "Going for a dirty abort.\n\n");
        KJB_SIGNAL(SIGABRT, SIG_DFL);
        abort();
    }
    else
    {
        guard = TRUE;
    }

    set_atn_trap(confirm_exit_sig_fn, DONT_RESTART_AFTER_SIGNAL);

    interactive_res = is_interactive();

    if (interactive_res == ERROR)
    {
        /*
         * Abort anyway, but copy the code so it is clear that we got an error
         * return from is_interactive(). 
        */
        kjb_cleanup_for_abort();
        kjb_signal(SIGABRT, SIG_DFL);
        abort();
    }
    else if (interactive_res == FALSE)
    {
        abort_confirmed = TRUE;
    }
    else
    {
        confirm_res = yes_no_query("Abort (y/n/q) ");

        if (confirm_res == ERROR)
        {
            /*
             * Abort anyway, but copy the code so it is clear that we got an
             * error return from is_interactive(). 
            */
            kjb_cleanup_for_abort();
            kjb_signal(SIGABRT, SIG_DFL);
            abort();
        }
        else if (confirm_res == TRUE)
        {
            abort_confirmed = TRUE;
        }
    }

    if (abort_confirmed)
    {
        kjb_cleanup_for_abort();

        kjb_signal(SIGABRT, SIG_DFL);

        if (kjb_fork_depth > 0)
        {
            fprintf(stderr, "Child process aborting and silently dumping core.\n");
        }

        abort(); 
    }

    guard = FALSE;

    unset_atn_trap();

#else

    fprintf(stderr, "Non-recoverable error.\n");
    exit(EXIT_BUG);
    /*NOTREACHED*/

#endif


}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                                   kjb_exit
 *
 * KJB library replacement for exit()
 *
 * This function is a replacement for exit, which ensures that all cleanup
 * is accomplished before exit. On ocasion, an exit without cleanup is desired,
 * and in this case, _exit() should be used. kjb_exit_2 can be used to exit with
 * the same cleanup which is used for abort (no heap checking being the main
 * difference, which only applies to development code).
 *
 * Index: standard library
 *
 * -----------------------------------------------------------------------------
*/

void kjb_exit(int exit_code)
{

#ifdef TEST
    /*
    // TEST_PSE(("Before cleanup\n"));
    // kjb_fflush(stdout);
    // sleep(1);
    */
#endif

    kjb_cleanup();

    /*
    // Don't use kjb library routines after this point! Cleanup has made it so
    // that they can no longer be used safely.
    */

#ifdef TEST
    /*
    // fprintf(stderr, "<<TEST>> Process %d is exiting (IL4RT).\n", MY_PID);
    // fflush(stdout);
    // sleep(1);
    */
#endif

    exit(exit_code);

    /*NOTREACHED*/
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                                   kjb_exit_2
 *
 * KJB library replacement for exit()
 *
 * This function is similar to kjb_exit, except some cleanup, most notably heap
 * cheking in the case of production code, is not done. If an exit with
 * absolutely no cleanup is needed, then _exit() should be used.
 *
 * Index: standard library
 *
 * -----------------------------------------------------------------------------
*/

void kjb_exit_2(int exit_code)
{

#ifdef TEST
    /*
    // TEST_PSE(("Before cleanup\n"));
    // kjb_fflush(stdout);
    // sleep(1);
    */
#endif

    kjb_cleanup_for_abort();

    /*
    // Don't use kjb library routines after this point! Cleanup has made it so
    // that they can no longer be used safely.
    */

#ifdef TEST
    /*
    // printff(stderr, "<<TEST>> Process %d is exiting (IL4RT).\n", MY_PID);
    // fflush(stdout);
    // sleep(1);
    */
#endif

    exit(exit_code);

    /*NOTREACHED*/
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                            terminate_child_process
 *
 * The routine terminates a child a process.
 *
 * The basic algorithm is to first check if the process is alive. If it is,
 * then send it SIGTERM, and give it a chance to clean up. We check up on it at
 * 200ms intervals for a total of 2 seconds.  If it is still alive, then send
 * it a SIGKILL. We then check up on it every 200ms until it is really dead.
 * The maximum wait is again set to 2 seconds.
 *
 * Related:
 *    kjb_waitpid2, check_child, check_child2
 *
 * Index: processes, signals, standard library
 *
 * -----------------------------------------------------------------------------
*/

void terminate_child_process(pid_t pid)
{
#ifdef UNIX
    int i, check_res;


#ifdef REPORT_PROCESSES
    TEST_PSE(("Terminating process %d (IL4RT).\n", (int)pid));
#endif

    if ((check_res = check_child(pid)) != PROCESS_IS_DEAD)
    {
#ifdef REPORT_PROCESSES
        TEST_PSE(("Sending process %d a SIGTERM (IL4RT).\n", (int)pid));
#endif
        kill(pid, SIGTERM);

        for (i = 0; i< 10; i++)
        {
            nap(200);
            if ((check_res = check_child(pid)) == PROCESS_IS_DEAD) break;
#ifdef REPORT_PROCESSES
            TEST_PSE(("Still alive (IL4RT).\n"));
#endif
        }
    }

    if (check_res != PROCESS_IS_DEAD)
    {
#ifdef REPORT_PROCESSES
        TEST_PSE(("Sending process %d a SIGKILL (IL4RT).\n", (int)pid));
#endif
        kill(pid, SIGKILL);

        for (i = 0; i< 10; i++)
        {
            nap(200);
            if ((check_res = check_child(pid)) == PROCESS_IS_DEAD) break;
#ifdef REPORT_PROCESSES
            TEST_PSE(("Still alive (IL4RT).\n"));
#endif
        }
    }

#ifdef TEST
#ifdef REPORT_PROCESSES
    if (check_res  != PROCESS_IS_DEAD)
    {
        TEST_PSE(("Still alive! Giving up! (IL4RT).\n"));
    }
    else
    {
        TEST_PSE(("Process %d is now dead (IL4RT).\n", (int)pid));
    }
#endif
#endif

#endif    /* UNIX */

}


/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

 /*
 // FIX
 //
 // Does not deal with zombies.
 */

int process_is_alive(int pid)
{
    int   result = FALSE;
#ifdef UNIX
    int   read_res;
    int   ps_pipe[2];
    int   ps_pid;
    char  ps_line_buff[ 20000 ];
    char* ps_line_buff_pos;
    char  user_field[ 2000 ];
    char  process_field[ 2000 ];
    int   test_pid;
#ifdef DEBUG_PROCESS_IS_ALIVE
    FILE* debug_fp;
    char  debug_file[ MAX_FILE_NAME_SIZE ];
#endif


    if (pipe(ps_pipe) < 0) 
    {
        set_error("Unable to create pipe to check if process is alive.%S"); 
        return ERROR;
    }

    ps_pid = kjb_fork();

    if (IS_CHILD(ps_pid))
    {
        dup2(ps_pipe[WRITE_END], fileno(stdout));
        kjb_exec("/bin/ps -eaf");
        kjb_exit(EXIT_SUCCESS);
    }
    else
    {
        close(ps_pipe[WRITE_END]);

#ifdef DEBUG_PROCESS_IS_ALIVE
        ERE(kjb_sprintf(debug_file, sizeof(debug_file),
                        "process_is_alive_debug: %d", MY_PID));
        NRE(debug_fp = kjb_fopen(debug_file, "w"));
#endif

        /* Skip first line */
        read_res = dget_line(ps_pipe[READ_END], ps_line_buff,
                             sizeof(ps_line_buff));

        if (read_res == ERROR)
        {
            result = ERROR;
        }
        else if (read_res < 0)
        {
            result = FALSE;
        }
        else
        {
            read_res = dget_line(ps_pipe[READ_END], ps_line_buff,
                                 sizeof(ps_line_buff));
        }

#ifdef DEBUG_PROCESS_IS_ALIVE
        kjb_fprintf(debug_fp, "Read_res is %d.\n", read_res);
#endif

        while(read_res > 0)
        {
#ifdef DEBUG_PROCESS_IS_ALIVE
            kjb_fputs(debug_fp, ps_line_buff);
            kjb_fputs(debug_fp, "\n");
#endif

            if (read_res == ERROR)
            {
                result = ERROR;
                break;
            }
            else if (read_res < 0)
            {
                result = FALSE;
                break;
            }

            ps_line_buff_pos = ps_line_buff;

            BUFF_GET_TOKEN(&ps_line_buff_pos, user_field);

#ifdef DEBUG_PROCESS_IS_ALIVE
            kjb_fputs(debug_fp, user_field);
            kjb_fputs(debug_fp, "\n");
#endif

            BUFF_GET_TOKEN(&ps_line_buff_pos, process_field);

#ifdef DEBUG_PROCESS_IS_ALIVE
            kjb_fputs(debug_fp, process_field);
            kjb_fputs(debug_fp, "\n");
#endif

            if (ss1pi(process_field, &test_pid) == ERROR)
            {
                result = ERROR;
                break;
            }
            else if (test_pid == pid)
            {
                result = TRUE;
                break;
            }

#ifdef DEBUG_PROCESS_IS_ALIVE
            kjb_fputs(debug_fp, "\n");
#endif

            read_res = dget_line(ps_pipe[READ_END], ps_line_buff,
                                 sizeof(ps_line_buff));

#ifdef DEBUG_PROCESS_IS_ALIVE
            kjb_fprintf(debug_fp, "Read_res is %d.\n", read_res);
#endif
        }

        close(ps_pipe[READ_END]);
        kjb_waitpid(ps_pid);

#ifdef DEBUG_PROCESS_IS_ALIVE
        ERE(kjb_fclose(debug_fp));
#endif
    }

#else /* Case NOT UNIX follows. */
    
    set_error("Unable to determine status of process %ld", pid);
    add_error("Calling a system function currently only implementated in unix.");
    result = ERROR;

#endif 
    
    return result;

}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*
// Note: This routine does not work on all systems if used in TEST mode.
// probably due terminal handling.
*/

/*
#define DEBUG_GET_IDLE_TIME
*/

int get_idle_time(void)
{
    int   result = ERROR;
#ifdef UNIX
    int   read_res;
    int   who_pipe[2];
    pid_t who_pid;
    char  who_line_buff[ 20000 ];
    char* who_line_buff_pos;
    char  field[ 2000 ];
    char* field_pos;
    char  minute_str[ 2000 ];
    char  hour_str[ 2000 ];
    int   minutes, hours;
#ifdef DEBUG_GET_IDLE_TIME
    FILE* debug_fp = NULL;
    static int first_time = TRUE;
#endif


#ifdef DEBUG_GET_IDLE_TIME
    if (first_time)
    {
        first_time = FALSE;
        NRE(debug_fp = kjb_fopen("who_file", "w"));
    }
    else
    {
        NRE(debug_fp = kjb_fopen("who_file", "a"));
    }
#endif

#ifndef SUN5
#ifndef LINUX_X86
    set_error("Process idle time not implemented on this system.");
    return ERROR;
#endif
#endif

    if (pipe(who_pipe) < 0)
    {
        set_error("Unable to create pipe to get idle time.%S"); 
        return ERROR;
    }

    who_pid = kjb_fork();

    if (IS_CHILD(who_pid))
    {
        dup2(who_pipe[WRITE_END], fileno(stdout));

#ifdef SUN5
        kjb_exec("/usr/bin/who -m -T");
#else
#ifdef LINUX_X86
        kjb_exec("/usr/bin/who -m -u");
#else
        SET_CANT_HAPPEN_BUG();
#endif
#endif
        kjb_exit(EXIT_SUCCESS);
    }
    else
    {
        close(who_pipe[WRITE_END]);

        read_res = dget_line(who_pipe[READ_END], who_line_buff,
                             sizeof(who_line_buff));

        if (read_res == ERROR)
        {
            result = ERROR;
        }
        else if (read_res < 0)
        {
            result = ERROR;
        }
        else
        {
            who_line_buff_pos = who_line_buff;

#ifdef DEBUG_GET_IDLE_TIME
            ERE(fput_line(debug_fp, who_line_buff));
#endif

            BUFF_GET_TOKEN(&who_line_buff_pos, field);
            BUFF_GET_TOKEN(&who_line_buff_pos, field);
            BUFF_GET_TOKEN(&who_line_buff_pos, field);
            BUFF_GET_TOKEN(&who_line_buff_pos, field);
            BUFF_GET_TOKEN(&who_line_buff_pos, field);
            BUFF_GET_TOKEN(&who_line_buff_pos, field);
#ifdef SUN5
            BUFF_GET_TOKEN(&who_line_buff_pos, field);
#endif

#ifdef DEBUG_GET_IDLE_TIME
            ERE(fput_line(debug_fp, field));
#endif

            if (STRCMP_EQ(field, "."))
            {
                result = 0;
            }
            else
            {
                field_pos = field;

                BUFF_GEN_GET_TOKEN(&field_pos, hour_str, ":");
                BUFF_GEN_GET_TOKEN(&field_pos, minute_str, ":");

                if (ss1pi(hour_str, &hours) == ERROR)
                {
                    result = ERROR;
                }
                else if (ss1pi(minute_str, &minutes) == ERROR)
                {
                    result = ERROR;
                }
                else
                {
                    result = minutes + 60 * hours;
                }
            }

#ifdef DEBUG_GET_IDLE_TIME
            kjb_fprintf(debug_fp, "%d\n", result);
#endif
        }

        close(who_pipe[READ_END]);
        kjb_waitpid(who_pid);

#ifdef DEBUG_GET_IDLE_TIME
        kjb_fclose(debug_fp);
#endif

    }

#else /* Case NOT UNIX follows. */
    
    set_error("Unable to determine process idle time.");
    add_error("Calling a system function currently only implementated in unix.");
    result = ERROR;

#endif 
    
    return result;
}


/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                                get_temp_file_name
 *
 * Finds an appropriate name for a temp file
 *
 * This routine is essentially a replacement for tmpnam(3C), but the name
 * includes the user name, which helps debugging. In addition, the error
 * messages are better and error handling is consistent with the rest of the KJB
 * library.
 *
 * Macros:
 *     BUFF_GET_TEMP_FILE_NAME
 *
 * Index: standard library
 *
 * Related:
 *    BUFF_GET_TEMP_FILE_NAME
 *
 * -----------------------------------------------------------------------------
*/

/*
// Only for UNIX for the time being.
*/

#define MAX_NUM_FIND_TEMP_FILE_TRIES   10000     /* MAGIC number */

int get_temp_file_name(char* temp_name, size_t max_len)
{
    static long count = 0;
    char        user_name[ 100 ];
    int         num_tries        = 0;
    char        temp_dir[ MAX_FILE_NAME_SIZE ];
    const char* separator;


    ERE(BUFF_GET_USER_ID(user_name));

    ERE(kjb_sprintf(temp_dir, sizeof(temp_dir), "%s%s%s", TEMP_DIR, DIR_STR,
                    user_name));

    ERE(kjb_mkdir(temp_dir));

    separator = DIR_STR;

    for( num_tries = 0; num_tries < MAX_NUM_FIND_TEMP_FILE_TRIES; ++num_tries )
    {
        count++;

        ERE(kjb_sprintf(temp_name, max_len, "%s%s%s%s%ld-%ld", TEMP_DIR,
                        DIR_STR, user_name, separator, (long)MY_PID, count));

        if (get_path_type(temp_name) == PATH_DOES_NOT_EXIST)
        {
            return NO_ERROR;
        }
    }

    set_bug("Unable to get a temporary file name.");

    return ERROR;

}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*
 * =============================================================================
 *                                  kjb_fork
 *
 * Replace for fork() for use with KJB library.
 *
 * This is a wrapper for fork(). As explained below, it is advisable that
 * kjb_fork() is used instead of fork() if KJB library IO is used. The main
 * difference from the library user's point of view is that on error, ERROR is
 * retured (not -1) with an error message set. This is simply for compatability
 * with the majority of the KJB routines. A test for a negative result is
 * perhaps the best way to ensure compatability. A second, very important
 * difference between fork() and kjb_fork(), is that all file buffers are
 * flushed before forking. This is very important. If a fork process inherits
 * unflushed buffers, data can be written twice.
 *
 * Kjb_fork() also ensures that certain cleanup tasks and signal handling that
 * should only be in place for the parent are not inherited by the child.
 * Specifically, it is assumes that the parent handles terminal IO.
 *
 * It would possible to relax this assumption, but it does not seem worth the
 * trouble. Every program I have ever written has the parent in charge of
 * terminal IO. Let me know if this is assumption needs to be relaxed.
 *
 * Returns:
 *    On success, kjb_fork() returns the same value as the underlying fork().
 *    IE, it returns the childs process id to the parent, and 0 to the child. On
 *    failure ERROR is returned, with an appropriate error message being set.
 *    NOTE : This is different than fork() which returns -1 on failure.
 *
 * Index: processes, signals, standard library
 *
 * -----------------------------------------------------------------------------
 */

pid_t kjb_fork(void)
{
    pid_t fork_res;

#ifdef UNIX

    ERE(kjb_fflush((FILE*)NULL));

    fork_res = fork();

    if (fork_res == -1)
    {
        set_error("Fork failed.%S");
        return ERROR;
    }

    if (IS_CHILD(fork_res))
    {
        IMPORT int kjb_fork_depth;
        Signal_info cur_atn_vec;
        Signal_info new_atn_vec;

        kjb_fork_depth++;

        
        /*
        nap(100);
        dbw();
        print_allocated_memory(stderr);
        dbw();
        */

        /* Kobus, May 2014. We want to reset the memory checking counter in
         * order to not inherit unfreed memory at the time of the fork.
         * Apparantly, this was either unfinished or wrong. So this version of
         * handling this issue is relatively new. 
         *
         * Kobus, Sep 2014. For this to work, we need to close up shop with respect to all state
         * stacks, and perhaps ideally, anything that allocates memory. But, for
         * now, we just do the ones that cause issues. 
        */
#ifdef TRACK_MEMORY_ALLOCATION
        destroy_sig_queue(); 
#endif 
        kjb_disable_paging();
        term_rewind_setting_stack();
        /*
        reset_cleanup_for_fork(); 
        */

        /* We used to reset the heap for fork, but this means anything allocated
         * before the fork will lead to bad error messages if freed after the
         * fork. Since this can happen in non-library code, we no longer do
         * this. 
         *
         * reset_heap_checking_for_fork(); 
        */

        fs_process_generation++;

        /*
        // If the current attention trap is not the default, then we will
        // assume that the parent has a plan with respect to cleaning up
        // (or not, as the case may be). Thus it seems that the best thing to
        // do is to ignore attentions.
        */
        kjb_sigvec(SIGINT, (Signal_info*)NULL, &cur_atn_vec);

        if (cur_atn_vec.SIGNAL_HANDLER != SIG_DFL)
        {
            INIT_SIGNAL_INFO(new_atn_vec);
            new_atn_vec.SIGNAL_HANDLER = SIG_IGN;
            kjb_sigvec(SIGINT, &new_atn_vec, (Signal_info*)NULL);

#ifdef TEST
            /*
            // TEST_PSE(("Process %ld is ignoring SIGINT ", (long)MY_PID));
            // TEST_PSE(("because parent has handler (IL4RT).\n"));
            */
#endif
        }
#ifdef TEST
        /*
        // else
        // {
        //     TEST_PSE(("Process %ld is leaving default SIGINT ",
        //              (long)MY_PID));
        //     TEST_PSE(("handler in place (IL4RT).\n"));
        // }
        */
#endif
    }
    /*
    else 
    {
        dbw();
        print_allocated_memory(stderr);
        dbw();
        nap(100);
    }
    */

#else /* Case NOT UNIX follows. */
    set_error("Unable to fork a process.");
    add_error("Calling a system function currently only implementated in unix.");
    fork_res = ERROR;
#endif 
    

    return fork_res;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

#ifdef TEST

int test_process_is_alive(void)
{
    char line[ 1000 ];
    char* line_pos;
    char pid_str[ 1000 ];
    int  pid;
    int  is_alive; 

    while (BUFF_TERM_GET_LINE("Enter PID ", line) != EOF)
    {
        line_pos = line;

        if ( ! BUFF_GET_TOKEN_OK(&line_pos, pid_str) ) continue;

        if (ss1pi(pid_str, &pid) == ERROR)
        {
            kjb_print_error();
        }
        else
        {
            is_alive = process_is_alive(pid);

            if (is_alive != ERROR)
            {
                pso("Process %d %s alive.\n",
                    pid, is_alive ? "is" : "is not");

            }
            else
            {
                kjb_print_error();
            }
        }
    }
    return NO_ERROR;
}

#endif

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

#ifdef __cplusplus
}
#endif

