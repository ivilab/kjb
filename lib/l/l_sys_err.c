
/* $Id: l_sys_err.c 21650 2017-08-04 00:51:16Z kobus $ */

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
#include "l/l_sys_io.h"
#include "l/l_string.h"
#include "l/l_sys_str.h"
#include "l/l_sys_term.h"

#ifdef HANDLE_BUGS
#undef HANDLE_BUGS
#endif

#ifdef TEST
#define HANDLE_BUGS
#else
#ifdef DEBUGGING
#define HANDLE_BUGS
#endif
#endif


#ifdef HANDLE_BUGS
#    include "l/l_io.h"    /* yes_no_query() */
#endif

#include "l/l_sys_err.h"

#include "l/l_queue.h"

#ifdef __cplusplus
extern "C" {
#endif

/* -------------------------------------------------------------------------- */

static int   fs_guard_error_routines   = FALSE;
static int   fs_error_message_count    = 0;
static int   fs_last_message_index     = 0;
static char  fs_error_messages [ MAX_NUM_ERROR_MESS ][ ERROR_MESS_BUFF_SIZE ];
static Error_action fs_error_action = SET_ERROR_ON_ERROR;

static Queue_element* fs_error_action_stack_head = NULL;


/* -------------------------------------------------------------------------- */

#ifndef TEST
    static void log_error_and_input(const char* mess);
#endif

#ifdef TRACK_MEMORY_ALLOCATION
    static void free_error_action_stack(void);
#endif


/* -------------------------------------------------------------------------- */

/* =============================================================================
 *                                 push_error_action
 *
 * Modifies the behaviour of error storing routines
 *
 * This routine is similar to set_error_action(), except that the previous value
 * is pushed onto a stack, so that it can be restored with pop_error_action().
 *
 * Note:
 *     This routine currently consumes a small amount of memory. If allocation
 *     were to fail, this is essentially handled as a bug. However, the error
 *     action is set, and the routine does return. Ideally, since this routine
 *     is part of the error handling system, it should not really on memory
 *     allocation.
 *
 * Index: error handling
 *
 * -----------------------------------------------------------------------------
*/

void push_error_action(Error_action error_action)
{
    Error_action* save_error_action_ptr;
    int result;
#ifdef TRACK_MEMORY_ALLOCATION
    static int        fs_first_error_action_stack_use = TRUE;
#endif


    SKIP_HEAP_CHECK_2();
    save_error_action_ptr = TYPE_MALLOC(Error_action);
    CONTINUE_HEAP_CHECK_2();

    if (save_error_action_ptr == NULL)
    {
        kjb_print_error();
        set_bug("Unable to push error action onto stack.");
        return;
    }

    *save_error_action_ptr = get_error_action();

#ifdef TRACK_MEMORY_ALLOCATION
    if (fs_first_error_action_stack_use)
    {
        fs_first_error_action_stack_use = FALSE;
        add_cleanup_function(free_error_action_stack);
    }
#endif

    SKIP_HEAP_CHECK_2();
    result = insert_into_queue(&fs_error_action_stack_head,
                               (Queue_element**)NULL,
                               (void*)save_error_action_ptr);
    CONTINUE_HEAP_CHECK_2();

    if (result == ERROR)
    {
        kjb_print_error();
        set_bug("Unable to push error action onto stack.");
    }

    set_error_action(error_action);
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                                 pop_error_action
 *
 * Restores error storing behaviour to previous.
 *
 * This routine restores the error action to the behaviour before the last
 * push_error_action().
 *
 * If the stack is NULL, then this is treated as a bug in development code, and
 * silently ignored otherwise.
 *
 * Index: error handling
 *
 * -----------------------------------------------------------------------------
*/

void pop_error_action(void)
{
    Queue_element* cur_elem;

    if (fs_error_action_stack_head == NULL)
    {
#ifdef TEST
        set_bug("Error action pop on empty stack.");
#endif
        return;
    }

    cur_elem = remove_first_element(&fs_error_action_stack_head,
                                    (Queue_element**)NULL);

    if (cur_elem == NULL)
    {
        kjb_print_error();
        set_bug("Unable to pop error action from stack.");
        return;
    }

    set_error_action(*((Error_action*)(cur_elem->contents)));

    SKIP_HEAP_CHECK_2();
    free_queue_element(cur_elem, kjb_free);
    CONTINUE_HEAP_CHECK_2();
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

#ifdef TRACK_MEMORY_ALLOCATION
/*
 * =============================================================================
 * STATIC                      free_error_action_stack
 * -----------------------------------------------------------------------------
 */

static void free_error_action_stack(void)
{
    free_queue(&fs_error_action_stack_head, (Queue_element**)NULL,
               kjb_free);

    fs_error_action_stack_head = NULL; 
}

#endif

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */


/* =============================================================================
 *                                 set_error_action
 *
 * Modifies the behaviour of error storing routines
 *
 * This routine modifies the behaviour of error storing routines. Normally,
 * errors are stored until either kjb_print_error() or kjb_get_error() is called
 * (error_action==SET_ERROR_ON_ERROR). This routine can be used to change this
 * behaviour. For example, if we do not want the errors incurred while some
 * routine is being executed to overwrite current errors, we may
 * wish to either ignore the errors (error_action==IGNORE_ERROR_ON_ERROR) or
 * have them always added (error_action==FORCE_ADD_ERROR_ON_ERROR).
 *
 * Index: error handling
 *
 * -----------------------------------------------------------------------------
*/

void set_error_action(Error_action error_action)
{

    fs_error_action = error_action;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                                  get_error_action
 *
 * Returns the current error action
 *
 * The routine returns the current error action. It is normally used by low
 * level routines to save the current state, which is then reinstated with
 * set_error_action().
 *
 * Index: error handling
 *
 * -----------------------------------------------------------------------------
*/

Error_action get_error_action(void)
{

    return fs_error_action;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*
 * =============================================================================
 *                                  set_bug_handler
 *
 * Sets the bug handler used by set_bug
 *
 * This routine sets the bug handler used by set_bug. If this routine is not
 * used, then default_bug_handler is used.
 *
 * Index: debugging, error handling
 *
 * -----------------------------------------------------------------------------
 */

void set_bug_handler(void (*bug_handler)(const char*))
{
    IMPORT void (*kjb_bug_handler)(const char*);


    kjb_bug_handler = bug_handler;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                                default_bug_handler
 *
 * Default bug handling routine for set_bug
 *
 * This routine is the default bug handling routine for set_bug. Its action
 * depends on whether the development library (compiled with -DTEST) or the
 * production library is being used. If the production library is being used,
 * then a bug log is produced, the user is notified of the existence of the
 * file, and is asked to send the log-file to the author. The log-file should
 * contain enough information to reproduce the error, as it contains the
 * commands the user typed. In the case of the development library, the mesage
 * is printed, and the user is prompted as to whether or not they want an
 * abort.
 *
 * Index: debugging, error handling
 *
 * -----------------------------------------------------------------------------
*/

/*
 *  This routine should do without L routines where thay are not adding
 *  functionality, as it is used in bug handling.
 */

#ifndef HANDLE_BUGS
/*ARGSUSED*/
#endif
void default_bug_handler(const char* message)
{
    static int kjb_guard_bug_routines = FALSE;


    if (kjb_guard_bug_routines)
    {
#ifdef HANDLE_BUGS
        kjb_signal(SIGABRT, SIG_DFL);
        fputs("Re-entry into bug handler. Going for dirty abort\n", stderr);
        abort();   /* Don't use kjb_abort(), here ! */
        /*NOTREACHED*/
#else
        fputs("\n", stderr);
        fputs("Unrecoverable internal error trapped. Terminating program.\n",
              stderr);
        _exit(EXIT_FAILURE);
        /*NOTREACHED*/
#endif
    }

    kjb_guard_bug_routines = TRUE;

#ifdef KJB_HAVE_NATIVE_FFLUSH_NULL
    fflush((FILE*)NULL);
#endif


#ifdef HANDLE_BUGS
    fputs("\n", stderr);
    fputs(message, stderr);
    fputs("\n", stderr);

    kjb_abort();
    /*NOTREACHED*/
#else
    fputs("\n", stderr);
    fputs("Internal error trapped.\n", stderr);
    fputs(message, stderr);
    fputs("\n", stderr);

    if (is_interactive())
    {
        log_error_and_input(message);
    }

    /* Kobus. 11-09-05. I am not sure if the choice to carry on makes any sense.
     * If the program is running in a script, we may be watching the error exit.
     * I am adding an error exit for the moment.
    */
    fputs("\n", stderr);
    fputs("Terminating program with EXIT_BUG (new behavior for 17-07-01).\n", stderr);
    fputs("\n", stderr);
    _exit(EXIT_FAILURE);
    /*NOTREACHED*/
#endif

    fs_error_message_count = 0;
    kjb_guard_bug_routines = FALSE;
}

#ifndef TEST

/* =============================================================================
 * STATIC
 * -----------------------------------------------------------------------------
*/

/*
 *  This routine should make do without L routines where possible (without
 *  losing too much functionality) as it is used in bug handling.
*/

static void log_error_and_input(const char* mess)
{
    static int count = 1;
    char debug_file_name[ 100 ];     /* MAGIC NUM */
    FILE* fp;
#ifdef MY_PID
    long my_pid = MY_PID;
#else
    long my_pid = NOT_SET;
#endif

    if (count > 100)
    {
        fprintf(stderr, "Over 100 requests for internal error logging.\n");
        fprintf(stderr, "Likely there is a problem with the error handler.\n");
        fprintf(stderr, "Terminating the program.\n");
        _exit(EXIT_FAILURE);
    }

    if (KJB_IS_SET(my_pid))
    {
        ER(kjb_sprintf(debug_file_name, sizeof(debug_file_name),
                       "bug_command_log.%ld", my_pid));
    }
    else
    {
        ER(kjb_sprintf(debug_file_name, sizeof(debug_file_name),
                       "bug_command_log"));
    }

    NR(fp = fopen(debug_file_name, "a"));

    fprintf(stderr,
            "Dumping command history to file \"%s\" for debugging.\n",
            debug_file_name);
    fprintf(stderr, "Please send this file to the program author.\n");

    if (KJB_IS_SET(my_pid))
    {
        fprintf(fp, "# Internal error %d logged for process %ld.\n", count,
                my_pid);
    }
    else
    {
        fprintf(fp, "# Internal error %d logged.\n", count);
    }

    if (mess == NULL)
    {
        fprintf(fp, "# Error message is: NULL\n");
    }
    else
    {
        fprintf(fp, "# Error message is: %s\n", mess);
    }
    fprintf(fp, "# Terminal input follows\n");
    fprintf(fp, "##################################\n");

    print_term_input_queue(fp);

    fprintf(fp, "========================================================\n\n");

    fclose(fp);

    count++;
}

#endif

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                                kjb_print_error
 *
 * Prints stored error if there is one.
 *
 * This routine prints the error information which is stored by the routines
 * set_error(3), add_error(3), cat_error(), and insert_error(). If there is no
 * error, then nothing is done. (If you are using the development version of the
 * library, then a message is printed saying that kjb_print_error was called
 * with no message to print).   there is one. By convention, all KJB library
 * routines will set an error if they returned an error. (Some routines probably
 * break this convention.).  Once a message is printed with kjb_print_error,
 * then the current message is cleared.
 *
 * Index: error handling
 *
 * -----------------------------------------------------------------------------
*/

void kjb_print_error(void)
{


    if (fs_guard_error_routines)
    {
        return;
    }

    fs_guard_error_routines = TRUE;

    if (fs_error_message_count > 0)
    {
        int count;


        kjb_fputs(stderr, "\n");

        set_high_light(stderr);

        if (! is_interactive())
        {
            /*
            // Print something generic to scan for.
            */
            kjb_fputs(stderr, "Error reported during non-interactive run.\n");
        }

        for (count=0; count<fs_error_message_count; count++)
        {
            kjb_fputs(stderr, fs_error_messages[count]);
            kjb_fputs(stderr, "\n");
        }

        unset_high_light(stderr);

        kjb_fputs(stderr, "\n");
    }
#ifdef TEST
    else
    {
        TEST_PSE(("Call to kjb_print_error with nothing to print.\n"));
    }
#endif

    /*
    // Prevent double printing. Copy the code from kjb_clear_error, rather
    // than calling it, in order to simplify the re-entry guarding.
    */

    fs_error_message_count = 0;

    fs_guard_error_routines = FALSE;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                                kjb_get_error
 *
 * Puts the stored error into a buffer.
 *
 * This routine is similar to kjb_print_error, but the message is copied into a
 * buffer instead.
 *
 * Index: error handling
 *
 * -----------------------------------------------------------------------------
*/

void kjb_get_error(char* buff, size_t buff_len)
{
    int count;


    if (buff_len == 0)
    {
        SET_ARGUMENT_BUG();
        return;
    }

    if (fs_guard_error_routines)
    {
        /*
         * This happens alot. For example, when we do a kjb_print_error(), some
         * fo the file opens will fail. But we are not interested in those
         * messages, let alone having them overwrite the one that we are trying
         * to print.
        */
        return;
    }

    fs_guard_error_routines = TRUE;

    if ((buff == NULL) || (buff_len == 0)) return;

    *buff = '\0';

    for (count = 0; count < fs_error_message_count; count++)
    {
#if 0 /* was ifdef HOW_IT_WAS */
        if (count > 0)
        {
            kjb_strncat(buff, "\n", buff_len);
        }
        kjb_strncat(buff, fs_error_messages[count], buff_len);
#else
        kjb_strncat(buff, fs_error_messages[count], buff_len);

        if (count < fs_error_message_count - 1)
        {
            kjb_strncat(buff, "\n", buff_len);
        }
#endif
    }

    fs_error_message_count  = 0;

    fs_guard_error_routines = FALSE;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                                kjb_get_strlen_error
 *
 * Returns the string length of the error message.
 *
 * This routine is a complement to kjb_get_error, which requires a buffer
 * length.  What should that buffer length be so that no error messages are
 * truncated?  This function helps you figure it out:  it returns the number
 * of non-null characters in the error string.  Like strlen(), it does not
 * count the null at the end of the string.
 *
 * Author:  Andrew Predoehl
 *
 * Documenter:  Andrew Predoehl
 *
 * Index: error handling
 *
 * -----------------------------------------------------------------------------
*/

int kjb_get_strlen_error( void )
{
    int count, err_strlen = 0;

    /* guard is not needed since this function does not affect any state */

    for ( count = 0; count < fs_error_message_count; ++count )
    {
#if 0 /* ifdef HOW_IT_WAS */
        if ( count > 0 )
        {
            err_strlen += 1;
        }
        err_strlen += signed_strlen( fs_error_messages[ count ] );
#else
        err_strlen += signed_strlen( fs_error_messages[ count ] );

        if ( count < fs_error_message_count - 1 )
        {
            err_strlen += 1;
        }
#endif
    }

    return err_strlen;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                                kjb_clear_error
 *
 * Clears the KJB library error indicator.
 *
 * Index: error handling
 *
 * -----------------------------------------------------------------------------
*/

void kjb_clear_error(void)
{


    if (fs_guard_error_routines)
    {
        /*
         * This happens alot. For example, when we do a kjb_print_error(), some
         * fo the file opens will fail. But we are not interested in those
         * messages, let alone having them overwrite the one that we are trying
         * to print.
        */
        return;
    }

    /*
    // If we need to add any KJB library calls.
    //
    // fs_guard_error_routines = TRUE;
    */

    fs_error_message_count = 0;

    /*
    // If we need to add any KJB library calls.
    //
    // fs_guard_error_routines = FALSE;
    */
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */


/* =============================================================================
 *                                set_error
 *
 * Sets an error message
 *
 * This routine generally sets the kjb_error message, but the action can be
 * modified using set_error_action. When it sets the error message, it
 * overwrites the previous message. To build upon the previous messages, use
 * either add_error(), insert_error(), or cat_error().
 *
 * The argmuents to set_error is a format string followed by zero or more
 * corresponding arguments. See kjb_fprintf for some of the non-standard options
 * useful for error messages like %S, %F, %D. (%S is particularly helpful---it
 * accesses the error messages from the last unsuccessful system call).
 *
 * kjb_print_error can be used to print the message when needed. It adds the
 * return for every message string, so in general, message strings should not
 * have returns in them.
 *
 * Index: error handling
 *
 * -----------------------------------------------------------------------------
*/

/*PRINTFLIKE1*/
void set_error(const char* format_str, ...)
{
    va_list ap;
    char error_mess_buff[ ERROR_MESS_BUFF_SIZE ];


    if (fs_guard_error_routines)
    {
        /*
         * This happens alot. For example, when we do a kjb_print_error(), some
         * fo the file opens will fail. But we are not interested in those
         * messages, let alone having them overwrite the one that we are trying
         * to print.
        */
        return;
    }

    if (fs_error_action == IGNORE_ERROR_ON_ERROR) return;

    fs_guard_error_routines = TRUE;

    va_start(ap, format_str);

    kjb_vsprintf(error_mess_buff, sizeof(error_mess_buff), format_str, ap);

    va_end(ap);

    if (fs_error_action == SET_ERROR_ON_ERROR)
    {
        BUFF_CPY(fs_error_messages[ 0 ], error_mess_buff);

        fs_last_message_index  = 0;
        fs_error_message_count = 1;
    }

    fs_guard_error_routines = FALSE;

    if (    (fs_error_action == FORCE_ADD_ERROR_ON_ERROR)
         && (error_mess_buff[ 0 ] != '\0')
       )
    {
        if (fs_error_message_count > 0)
        {
            add_error("Additional problems follow:");
        }
        add_error(error_mess_buff);
    }
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                                 add_error
 *
 * Adds an additional line to the error messages
 *
 * This routine is similar to set_error, except that the line is added to the
 * set of message strings, each of which corresponds to a line on output. It is
 * valid to use add_error without a corresponding set_error, but doing so in
 * such a way that makes sense normally requires kjb_clear_error.
 *
 * The argmuents to add_error is a format string followed by zero or more
 * corresponding arguments. See kjb_fprintf for some of the non-standard options
 * useful for error messages like %S, %F, %D. (%S is particularly helpful---it
 * accesses the error messages from the last unsuccessful system call).
 *
 * Index: error handling
 *
 * -----------------------------------------------------------------------------
*/

/*PRINTFLIKE1*/
void add_error(const char* format_str, ...)
{
    va_list ap;
    char error_mess_buff[ ERROR_MESS_BUFF_SIZE ];


    if (fs_guard_error_routines)
    {
        /*
         * This happens alot. For example, when we do a kjb_print_error(), some
         * fo the file opens will fail. But we are not interested in those
         * messages, let alone having them overwrite the one that we are trying
         * to print.
        */
        return;
    }

    if (fs_error_action == IGNORE_ERROR_ON_ERROR) return;

    fs_guard_error_routines = TRUE;

    va_start(ap, format_str);

    kjb_vsprintf(error_mess_buff, sizeof(error_mess_buff), format_str, ap);

    va_end(ap);

    if (    (fs_error_action == SET_ERROR_ON_ERROR)
         || (fs_error_action == FORCE_ADD_ERROR_ON_ERROR)
       )
    {
        if (fs_error_message_count < MAX_NUM_ERROR_MESS)
        {
            BUFF_CPY(fs_error_messages[ fs_error_message_count ],error_mess_buff);
            fs_last_message_index = fs_error_message_count;
            fs_error_message_count++;
        }
    }
    else
    {
        SET_CANT_HAPPEN_BUG();
    }

    fs_guard_error_routines = FALSE;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*
 * =============================================================================
 *                                  cat_error
 *
 * Catenates to the current message string
 *
 * This routine is like add_error, except the string is added to the current
 * line as opposed to becomming a new line.
 *
 * The argmuents to cat_error is a format string followed by zero or more
 * corresponding arguments. See kjb_fprintf for some of the non-standard options
 * useful for error messages like %S, %F, %D. (%S is particularly helpful---it
 * accesses the error messages from the last unsuccessful system call).
 *
 * Index: error handling
 *
 * -----------------------------------------------------------------------------
 */

/*PRINTFLIKE1*/
void cat_error(const char* format_str, ...)
{
    va_list ap;
    char error_mess_buff[ ERROR_MESS_BUFF_SIZE ];


    if (fs_guard_error_routines)
    {
        /*
         * This happens alot. For example, when we do a kjb_print_error(), some
         * fo the file opens will fail. But we are not interested in those
         * messages, let alone having them overwrite the one that we are trying
         * to print.
        */
        return;
    }

    if (fs_error_action == IGNORE_ERROR_ON_ERROR) return;

    fs_guard_error_routines = TRUE;

    va_start(ap, format_str);

    kjb_vsprintf(error_mess_buff, sizeof(error_mess_buff), format_str, ap);

    va_end(ap);

    if (    (fs_error_action == SET_ERROR_ON_ERROR)
         || (fs_error_action == FORCE_ADD_ERROR_ON_ERROR)
       )
    {
        BUFF_CAT(fs_error_messages[ fs_last_message_index ], error_mess_buff);
    }
    else
    {
        SET_CANT_HAPPEN_BUG();
    }

    fs_guard_error_routines = FALSE;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*
 * =============================================================================
 *                                  insert_error
 *
 * Inserts a new error line before other error lines.
 *
 * This routine is similar to add_error, except that the line is added at the
 * begining of the error lines, not at the end.
 *
 * The argmuents to insert_error is a format string followed by zero or more
 * corresponding arguments. See kjb_fprintf for some of the non-standard options
 * useful for error messages like %S, %F, %D. (%S is particularly helpful---it
 * accesses the error messages from the last unsuccessful system call).
 *
 * Index: error handling
 *
 * -----------------------------------------------------------------------------
 */

/*PRINTFLIKE1*/
void insert_error(const char* format_str, ...)
{
    va_list ap;
    char error_mess_buff[ ERROR_MESS_BUFF_SIZE ];
    int last;


    if (fs_guard_error_routines)
    {
        /*
         * This happens alot. For example, when we do a kjb_print_error(), some
         * fo the file opens will fail. But we are not interested in those
         * messages, let alone having them overwrite the one that we are trying
         * to print.
        */
        return;
    }

    if (fs_error_action == IGNORE_ERROR_ON_ERROR) return;

    fs_guard_error_routines = TRUE;

    va_start(ap, format_str);

    kjb_vsprintf(error_mess_buff, sizeof(error_mess_buff), format_str, ap);

    va_end(ap);

    if (fs_error_action == SET_ERROR_ON_ERROR)
    {
        last = MIN_OF(fs_error_message_count, MAX_NUM_ERROR_MESS - 1);

        while (last > 0)
        {
            BUFF_CPY(fs_error_messages[ last ],
                     fs_error_messages[ last - 1 ]);
            last--;
        }

        BUFF_CPY(fs_error_messages[ 0 ], error_mess_buff);
        fs_last_message_index = 0;

        if (fs_error_message_count < MAX_NUM_ERROR_MESS - 1)
        {
            fs_error_message_count++;
        }
    }

    fs_guard_error_routines = FALSE;

    if (fs_error_action == FORCE_ADD_ERROR_ON_ERROR)
    {
        add_error(error_mess_buff);
    }

}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                                str_set_error
 *
 * Sets an error message
 *
 * Similar to set_error, except that there are no optional arguments. Useful
 * for sending an unknown format string, which if parsed as such, could cause a
 * crash.
 *
 * Index: error handling
 *
 * -----------------------------------------------------------------------------
*/

void str_set_error(const char* str)
{

    if (fs_guard_error_routines)
    {
        /*
         * This happens a lot. For example, when we do a kjb_print_error(), some
         * fo the file opens will fail. But we are not interested in those
         * messages, let alone having them overwrite the one that we are trying
         * to print.
        */
        return;
    }

    if (fs_error_action == IGNORE_ERROR_ON_ERROR) return;

    fs_guard_error_routines = TRUE;

    if (fs_error_action == SET_ERROR_ON_ERROR)
    {
        BUFF_CPY(fs_error_messages[ 0 ], str);

        fs_last_message_index  = 0;
        fs_error_message_count = 1;
    }

    fs_guard_error_routines = FALSE;

    if (    (fs_error_action == FORCE_ADD_ERROR_ON_ERROR)
         && (str[ 0 ] != '\0')
       )
    {
        if (fs_error_message_count > 0)
        {
            str_add_error("Additional problems follow:");
        }
        str_add_error(str);
    }
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                                 str_add_error
 *
 * Adds an additional line to the error messages
 *
 * Similar to add_error, except that there are no optional arguments. Useful
 * for sending an unknown format string, which if parsed as such, could cause a
 * crash.
 *
 * Index: error handling
 *
 * -----------------------------------------------------------------------------
*/

void str_add_error(const char* str)
{


    if (fs_guard_error_routines)
    {
        /*
         * This happens alot. For example, when we do a kjb_print_error(), some
         * fo the file opens will fail. But we are not interested in those
         * messages, let alone having them overwrite the one that we are trying
         * to print.
        */
        return;
    }

    if (fs_error_action == IGNORE_ERROR_ON_ERROR) return;

    fs_guard_error_routines = TRUE;

    if (    (fs_error_action == SET_ERROR_ON_ERROR)
         || (fs_error_action == FORCE_ADD_ERROR_ON_ERROR)
       )
    {
        if (fs_error_message_count < MAX_NUM_ERROR_MESS)
        {
            BUFF_CPY(fs_error_messages[ fs_error_message_count ], str);
            fs_last_message_index = fs_error_message_count;
            fs_error_message_count++;
        }
    }
    else
    {
        SET_CANT_HAPPEN_BUG();
    }

    fs_guard_error_routines = FALSE;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*
 * =============================================================================
 *                                  str_cat_error
 *
 * Catenates to the current message string
 *
 * Similar to cat_error, except that there are no optional arguments. Useful
 * for sending an unknown format string, which if parsed as such, could cause a
 * crash.
 *
 * Index: error handling
 *
 * -----------------------------------------------------------------------------
 */

void str_cat_error(const char* str)
{


    if (fs_guard_error_routines)
    {
        /*
         * This happens alot. For example, when we do a kjb_print_error(), some
         * fo the file opens will fail. But we are not interested in those
         * messages, let alone having them overwrite the one that we are trying
         * to print.
        */
        return;
    }

    if (fs_error_action == IGNORE_ERROR_ON_ERROR) return;

    fs_guard_error_routines = TRUE;

    if (    (fs_error_action == SET_ERROR_ON_ERROR)
         || (fs_error_action == FORCE_ADD_ERROR_ON_ERROR)
       )
    {
        BUFF_CAT(fs_error_messages[ fs_last_message_index ], str);
    }
    else
    {
        SET_CANT_HAPPEN_BUG();
    }

    fs_guard_error_routines = FALSE;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*
 * =============================================================================
 *                                  str_insert_error
 *
 * Inserts a new error line before other error lines.
 *
 * This routine is similar to add_error, except that the line is added at the
 * begining of the error lines, not at the end.
 *
 * Index: error handling
 *
 * -----------------------------------------------------------------------------
 */

void str_insert_error(const char* str)
{
    int last;


    if (fs_guard_error_routines)
    {
        /*
         * This happens alot. For example, when we do a kjb_print_error(), some
         * fo the file opens will fail. But we are not interested in those
         * messages, let alone having them overwrite the one that we are trying
         * to print.
        */
        return;
    }

    if (fs_error_action == IGNORE_ERROR_ON_ERROR) return;

    fs_guard_error_routines = TRUE;

    if (fs_error_action == SET_ERROR_ON_ERROR)
    {
        last = MIN_OF(fs_error_message_count, MAX_NUM_ERROR_MESS - 1);

        while (last > 0)
        {
            BUFF_CPY(fs_error_messages[ last ],
                     fs_error_messages[ last - 1 ]);
            last--;
        }

        BUFF_CPY(fs_error_messages[ 0 ], str);
        fs_last_message_index = 0;

        if (fs_error_message_count < MAX_NUM_ERROR_MESS)
        {
            fs_error_message_count++;
        }
    }

    fs_guard_error_routines = FALSE;

    if (fs_error_action == FORCE_ADD_ERROR_ON_ERROR)
    {
        str_add_error(str);
    }

}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*
 * =============================================================================
 *                                  set_bug
 *
 * Initiates bug handling
 *
 * This routine works somewhat like set_error. There is no add_bug, etc.
 * It is used when the "error" is likely to be a programmer error. Its action
 * depends on the setting of the bug handler (see set_bug_handler). If the bug
 * handler is set, then that routine is callded with the message formed from
 * the format string and arguments. By default, the bug handler is set to
 * default_bug_handler. See default_bug_handler for details.
 *
 * Macros
 *    SET_BUFFER_OVERFLOW_BUG(), SET_FORMAT_STRING_BUG(),
 *    SET_ARGUMENT_BUG(), SET_CANT_HAPPEN_BUG()
 *
 *    These paramterless macros provide the format string for set_error.
 *
 * Index: debugging, error handling
 *
 * -----------------------------------------------------------------------------
 */

/*PRINTFLIKE1*/
void set_bug(const char* format_str, ...)
{
    IMPORT void (*kjb_bug_handler)(const char*);
    va_list ap;
    char error_mess_buff[ ERROR_MESS_BUFF_SIZE ];
    int message_creation_result;


    va_start(ap, format_str);

    message_creation_result = kjb_vsprintf(error_mess_buff,
                                           sizeof(error_mess_buff),
                                           format_str, ap);

    va_end(ap);

    if (message_creation_result == ERROR)
    {
        BUFF_CPY(error_mess_buff, "Creation of bug error message failed. ");
        BUFF_CAT(error_mess_buff, "Original message is lost.");
    }

    if (kjb_bug_handler != NULL)
    {
        fs_error_message_count = 0;
        (*kjb_bug_handler)(error_mess_buff);
    }
    else
    {
#ifdef REPORT_ALL_BUG_INFO
        set_error(error_mess_buff);
#else
        set_error("Internal error trapped. Proceeding optimistically ...");
#endif
    }
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

#ifdef REPORT_ALL_BUG_INFO

void test_set_buffer_overflow_bug(int line, const char* file)
{

    set_bug("Buffer is too small on line %d of file %s.",
            line, file);
}

#else

void set_buffer_overflow_bug(void)
{


    set_bug("Buffer is too small.");
}

#endif

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

#ifdef REPORT_ALL_BUG_INFO

void test_set_format_string_bug(int line, const char* file)
{


    set_bug("Format string error on line %d of file %s.",
            line, file);
}

#else

void set_format_string_bug(void)
{


    set_bug("Format string error.");
}

#endif

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

#ifdef REPORT_ALL_BUG_INFO

void test_set_argument_bug(int line, const char* file)
{


    set_bug("Argument error on line %d of file %s.", line, file);
}
#else

void set_argument_bug(void)
{


    set_bug("Argument error.");
}

#endif

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

#ifdef REPORT_ALL_BUG_INFO

void test_set_nan_double_bug(int line, const char* file)
{


    set_bug("Unexpected NaN double on line %d of file %s.", line, file);
}
#else

void set_nan_double_bug(void)
{


    set_bug("Unexpected NaN double error.");
}

#endif

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

#ifdef REPORT_ALL_BUG_INFO

void test_set_infinite_double_bug(int line, const char* file)
{


    set_bug("Unexpected infinite double on line %d of file %s.", line, file);
}
#else

void set_infinite_double_bug(void)
{


    set_bug("Unexpected infinite double error.");
}

#endif

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

#ifdef REPORT_ALL_BUG_INFO

void test_set_nan_float_bug(int line, const char* file)
{


    set_bug("Unexpected NaN float on line %d of file %s.", line, file);
}
#else

void set_nan_float_bug(void)
{


    set_bug("Unexpected NaN float error.");
}

#endif

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

#ifdef REPORT_ALL_BUG_INFO

void test_set_infinite_float_bug(int line, const char* file)
{


    set_bug("Unexpected infinite float on line %d of file %s.", line, file);
}
#else

void set_infinite_float_bug(void)
{


    set_bug("Unexpected infinite float error.");
}

#endif

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

#ifdef REPORT_ALL_BUG_INFO

void test_set_cant_happen_bug(int line, const char* file)
{


    set_bug("Cant happen error on line %d of file %s.",
            line, file);
}
#else

void set_cant_happen_bug(void)
{


    set_bug("Can't happen error.");
}

#endif

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

#ifdef REPORT_ALL_BUG_INFO

void test_set_bounds_bug(int line, const char* file)
{


    set_bug("Sort check error on line %d of file %s.",
            line, file);
}
#else

void set_bounds_bug(void)
{


    set_bug("Sort check error.");
}

#endif

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

#ifdef REPORT_ALL_BUG_INFO

void test_set_overflow_bug(int line, const char* file)
{


    set_bug("Overflow problem on line %d of file %s.", line, file);
}
#else

void set_overflow_bug(void)
{


    set_bug("Argument error.");
}

#endif

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

#ifdef REPORT_ALL_BUG_INFO

void test_set_underflow_bug(int line, const char* file)
{


    set_bug("Overflow problem on line %d of file %s.", line, file);
}
#else

void set_underflow_bug(void)
{


    set_bug("Argument error.");
}

#endif

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

#ifdef REPORT_ALL_BUG_INFO

void test_set_sort_bug(int line, const char* file)
{


    set_bug("Sort check error on line %d of file %s.",
            line, file);
}
#else

void set_sort_bug(void)
{


    set_bug("Sort check error.");
}

#endif

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */


#ifdef __cplusplus
}
#endif

