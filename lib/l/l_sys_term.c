
/* $Id: l_sys_term.c 22170 2018-06-23 23:01:50Z kobus $ */

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

#include "l/l_sys_sys.h"
#include "l/l_sys_def.h"
#include "l/l_sys_std.h"

#ifdef __C2MAN__
#    ifdef LINUX
         /* Won't always exist, especially without gcc, but perhaps we can
          * provided a stdbool.h where it is missing. If it is missing, this is
          * likely not necessary.
         */
#        include <stdbool.h>
#        define bool int
#        define _Bool int
#    endif
#endif

#define MAX_NUM_HISTORY_LINES_WRITEN 5000

#ifdef __C2MAN__
#    ifdef __va_list
#        undef __va_list
#    endif

#    define __va_list void*
#endif


#ifndef MAKE_DEPEND

#ifdef SYSV_SYSTEM
#    ifdef SUN5
#        ifdef __GNUC__
#            include <stdarg.h>
#        endif
#    endif

#    include <curses.h>
#    include <term.h>
#endif


#ifdef LINUX
#    include <curses.h>
#    include <term.h>
#    include <sys/ioctl.h>
#endif

#ifdef MAC_OSX
#    include <sys/ioctl.h>
#    include <sys/termios.h>
#    include <curses.h>
#    include <term.h>

#    define TCGETS TIOCGETA
#    define TCSETS TIOCSETA
#endif

#ifdef SGI
#    include <sys/termio.h>
#endif

#ifdef LINUX
#    include <termios.h>
#endif

#ifdef SUN_SYSTEM
#    include <sys/termios.h>
#    include <sys/types.h>
#endif

#ifdef HPUX
#    include <sys/termios.h>
#endif

#ifdef  NEXT
#    include <sgtty.h>
#endif

#ifdef  MS_OS
#    include <conio.h>

#ifdef __STDC__
#    define getche _getche
#    define putch _putch
#    define getch _getch
#endif 
#endif

#endif   /* ! MAKE_DEPEND */


#include "l/l_gen.h"

#include "l/l_io.h"
#include "l/l_queue.h"
#include "l/l_string.h"
#include "l/l_parse.h"
#include "l/l_sys_tsig.h"
#include "l/l_sys_time.h"
#include "l/l_sys_term.h"

#ifdef __cplusplus
extern "C" {
#endif

/* -------------------------------------------------------------------------- */

#ifdef SUN4  /* ------------------------------------------------ */
/* SUN4 */ char* tgetstr(char*, char**);
/* SUN4 */ int  tgetent(char*, char*);
/* SUN4 */ int  tgetflag(char*);
/* SUN4 */ int  tgetnum(char*);
/* SUN4 */ int  tputs(char*, int, int(*)(char));
#endif      /* ---------------------------------------------------- */


#ifdef NEXT  /* --------------------------------------------------- */
/* NEXT */ char* tgetstr(char*, char**);
/* NEXT */ int  tgetent(char*, char*);
/* NEXT */ int  tgetflag(char*);
/* NEXT */ int  tgetnum(char*);
/* NEXT */ int  tputs(char*, int, int(*)(char));
#endif      /* ---------------------------------------------------- */


/* -------------------------------------------------------------------------- */

#define MAX_CTL_SEQ_LEN           10

#ifdef NEXT
#ifndef L_ctermid
#    define L_ctermid  9
#endif
#endif

/* -------------------------------------------------------------------------- */

typedef enum Terminal_mode
{
    ORIGINAL_TERMINAL_MODE,
    RAW_WITH_ECHO_TERMINAL_MODE,
    RAW_WITH_NO_ECHO_TERMINAL_MODE
}
Terminal_mode;


typedef struct Terminal_settings
{
    Terminal_mode terminal_mode;
    Signal_info atn_vec;
    Signal_info tstp_vec;
    Signal_info term_vec;
    Signal_info quit_vec;
    struct termios term_flags;
}
Terminal_settings;

/* -------------------------------------------------------------------------- */

static Bool           fs_default_page_flag    = TRUE;
static Bool           fs_page_flag            = TRUE;
static Queue_element* fs_page_flag_stack_head = NULL;

#ifdef TRACK_MEMORY_ALLOCATION
    static int        fs_first_page_stack_use = TRUE;
#endif

/*
 * Currently, fs_use_smart_terminal is never reset. It is provided in case we
 * need to implement the capability of having the user disable using the
 * advanced terminal IO features.
 */
static int fs_use_smart_terminal = TRUE;

static int            fs_smart_terminal_capability                      = FALSE;
static FILE*          fs_tty_in                                         = NULL;
static FILE*          fs_tty_out                                        = NULL;
static int            fs_in_background                                  = FALSE;
static Queue_element* fs_term_buff_head                                 = NULL;
static Queue_element* fs_term_buff_tail                                 = NULL;
static int            fs_term_buff_count      = 0;
static int            fs_previous_sessions_term_buff_count  = 0;
static char           fs_input_up_arrow_sequence[ MAX_CTL_SEQ_LEN ]     = "";
static char           fs_input_down_arrow_sequence[ MAX_CTL_SEQ_LEN ]   = "";
static char           fs_input_left_arrow_sequence[ MAX_CTL_SEQ_LEN ]   = "";
static char           fs_input_right_arrow_sequence[ MAX_CTL_SEQ_LEN ]  = "";
static char           fs_output_up_arrow_sequence[ MAX_CTL_SEQ_LEN ]    = "";
static char           fs_output_down_arrow_sequence[ MAX_CTL_SEQ_LEN ]  = "";
static char           fs_output_left_arrow_sequence[ MAX_CTL_SEQ_LEN ]  = "";
static char           fs_output_right_arrow_sequence[ MAX_CTL_SEQ_LEN ] = "";
static unsigned char  fs_erase_char                                     = '\0';
static int            fs_high_light_flag                                = FALSE;
static char           fs_high_light_on_sequence[MAX_CTL_SEQ_LEN]        = "";
static char           fs_high_light_off_sequence[MAX_CTL_SEQ_LEN]       = "";
static char           fs_reentry_file_path[ MAX_FILE_NAME_SIZE ];

#if 0 /* was ifdef OBSOLETE */
    /*
    // Use of fs_vt100_output_kludge is probably obsolete as well, but I don't
    // have time to verify.
    */
    static int  fs_vt100_input_kludge = TRUE;
#endif

#ifdef UNIX
static int            fs_vt100_output_kludge         = FALSE;
#endif 

static Terminal_mode  fs_current_terminal_mode       = ORIGINAL_TERMINAL_MODE;
static Queue_element* fs_save_terminal_settings_head = NULL;

#ifdef UNIX
/* UNIX */      static int fs_initial_term_flags_set = FALSE;
#ifdef NEXT
/* next */      static struct sgttyb fs_initial_term_flags;
#else
/* NOT next */  static struct termios fs_initial_term_flags;
#endif
#endif

#ifdef TEST
#ifdef UNIX
/* TEST */ static FILE* fs_emulate_term_input_fp=NULL;
/* TEST */ static FILE* fs_term_input_fp=NULL;
#endif
#endif

/* -------------------------------------------------------------------------- */

#ifdef TRACK_MEMORY_ALLOCATION
    static void free_page_flag_stack(void);
#endif

static FILE* open_tty_in(void);
static void close_tty_in(void); 
static FILE* open_tty_out(void);
static void close_tty_out(void); 
static void setup_smart_terminal(void);

#ifdef UNIX
static int term_get_n_chars_guts(const char*, char*, size_t);

static int overwrite_term_get_n_chars_guts
(
    const char* ,
    char*       ,
    size_t
);

static int overwrite_term_get_char_guts(void);
#endif 

static int dumb_term_get_line(const char*, char*, size_t);

static int enhanced_term_get_line(const char*, char*, size_t);

static int enhanced_term_get_line_guts
(
    const char* ,
    char*       ,
    size_t
);

static int enhanced_term_getc_guts(void);

static void enhanced_term_put_buff(int, const char*, int);


static void enhanced_term_put_blanks(int, int);
static void term_line_end(void);
static void term_page_end(void);
static const char* term_get_buff(int);
static void add_line_to_reentry_queue(const char*);
static void write_line_reentry_history(void);

/* -----------------------------------------------------------------------------
 *                                  TPUTS
 *
 * Can't figure out a way declare TPUTS to keep everybody happy.   It is used
 * as a parameter and is expected to be int(*)(char), but if defined as such
 * compilers complain because they automatically promote char->int.
 */
#ifdef UNIX
#    ifdef SGI
         static int term_putc_fn_arg( int );
#        define TPUTS(x)  tputs(x, 1, term_putc_fn_arg)
#    else
#    ifdef MAC_OSX
         static int term_putc_fn_arg( int );
#        define TPUTS(x)  tputs(x, 1, term_putc_fn_arg)
#    else
#    ifdef LINUX
         static int term_putc_fn_arg( int );
#        define TPUTS(x)  tputs(x, 1, term_putc_fn_arg)
#    else
         static int term_putc_fn_arg( int );
#        define TPUTS(x)  tputs(x, 1, (int(*)(char))term_putc_fn_arg)
#    endif
#    endif
#    endif
#else
#    define TPUTS(x)  fputs(x, fs_tty_out)
#endif

/* ------------------------------------------------------------------------- */

int set_term_io_options(const char* option, const char* value)
{
    char lc_option[ 100 ];
    int  temp_boolean_value;
    int  result = NOT_FOUND;


    EXTENDED_LC_BUFF_CPY(lc_option, option);

    if (    (lc_option[ 0 ] == '\0')
         || (match_pattern(lc_option, "page"))
         || (match_pattern(lc_option, "paging"))
         || (match_pattern(lc_option, "pause"))   /* Obsolete synonym. */
       )
    {
        if (value == NULL)
        {
            return NO_ERROR;
        }
        else if (value[ 0 ] == '?')
        {
            ERE(pso("paging = %s\n", fs_default_page_flag ? "t" : "f"));
        }
        else if (value[ 0 ] == '\0')
        {
            ERE(pso("Screen output %s paged by default.\n",
                    fs_default_page_flag ? "is" : "is not"));
        }
        else
        {
            ERE(temp_boolean_value = get_boolean_value(value));
            fs_default_page_flag = (Bool)temp_boolean_value;
            kjb_use_default_paging();
        }
        result = NO_ERROR;
    }

    return result;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*
 * =============================================================================
 *                            kjb_use_default_paging
 *
 * Restores paging behaviour
 *
 * The paging behaviour is often temporarily disabled by the user when they
 * enter "n" to the "continue ? " query. This routine is used to reset the
 * behaviour to the default (which is normally user settable). THis routine is
 * usually used before processing the next user command in a command line
 * interface.
 *
 * Returns:
 *    This routine returns TRUE if the default is to page, FALSE, if the default
 *    is not to page, and ERROR if there are unexpected problems.
 *
 * ==> Currently an ERROR return is impossible, but reserved for future use.
 *
 * Related:
 *     kjb_enable_paging, kjb_disable_paging, kjb_restore_paging
 *
 * Index: output, paging, terminal I/O, pause
 *
 * -----------------------------------------------------------------------------
 */

int kjb_use_default_paging(void)
{
    fs_page_flag = fs_default_page_flag;

    return fs_default_page_flag;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*
 * =============================================================================
 *                            kjb_enable_paging
 *
 * Forces paging to occur.
 *
 * This routine forces paging to occur. The behavour will be reset to the
 * previous behavour with kjb_restore_paging().
 *
 * Returns:
 *    This routine returns NO_ERROR on success, and ERROR if there are
 *    unexpected problems.
 *
 * Related:
 *     kjb_use_default_paging, kjb_disable_paging, kjb_restore_paging
 *
 * Index: output, paging, terminal I/O, pause
 *
 * -----------------------------------------------------------------------------
 */

int kjb_enable_paging(void)
{
    Bool* save_page_flag_ptr;
    int result;


    SKIP_HEAP_CHECK_2();
    save_page_flag_ptr = BOOL_MALLOC(1);
    CONTINUE_HEAP_CHECK_2();
    NRE(save_page_flag_ptr);
    *save_page_flag_ptr = fs_page_flag;

#ifdef TRACK_MEMORY_ALLOCATION
    if (fs_first_page_stack_use)
    {
        fs_first_page_stack_use = FALSE;
        add_cleanup_function(free_page_flag_stack);
    }
#endif

    SKIP_HEAP_CHECK_2();
    result = insert_into_queue(&fs_page_flag_stack_head,
                               (Queue_element**)NULL,
                               (void*)save_page_flag_ptr);
    CONTINUE_HEAP_CHECK_2();
    ERE(result);

    fs_page_flag = TRUE;

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*
 * =============================================================================
 *                            kjb_disable_paging
 *
 * Forces paging not to occur.
 *
 * This routine forces paging not to occur. The behavour will be reset to the
 * previous behavour with kjb_restore_paging().
 *
 * Returns:
 *    This routine returns NO_ERROR on success, and ERROR if there are
 *    unexpected problems.
 *
 * Related:
 *     kjb_use_default_paging, kjb_enable_paging, kjb_restore_paging
 *
 * Index: output, paging, terminal I/O, pause
 *
 * -----------------------------------------------------------------------------
 */

int kjb_disable_paging(void)
{
    Bool* save_page_flag_ptr;
    int result;


    SKIP_HEAP_CHECK_2();
    save_page_flag_ptr = BOOL_MALLOC(1);
    CONTINUE_HEAP_CHECK_2();
    NRE(save_page_flag_ptr);
    *save_page_flag_ptr = fs_page_flag;

#ifdef TRACK_MEMORY_ALLOCATION
    if (fs_first_page_stack_use)
    {
        fs_first_page_stack_use = FALSE;
        add_cleanup_function(free_page_flag_stack);
    }
#endif

    SKIP_HEAP_CHECK_2();
    result = insert_into_queue(&fs_page_flag_stack_head,
                               (Queue_element**)NULL,
                               (void*)save_page_flag_ptr);
    CONTINUE_HEAP_CHECK_2();
    ERE(result);

    fs_page_flag = FALSE;

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*
 * =============================================================================
 *                            kjb_restore_paging
 *
 * Resets paging to behaviour.
 *
 * This routine resets paging to behaviour to the behaviour stored in the stack
 * maintained by this routine, kjb_disable_paging, and  kjb_enable_paging(). If
 * the entire stack of such behaviours has been used up, then this routine
 * silently does nothing.
 *
 * Returns:
 *    This routine returns TRUE if we are now paging, FALSE if we are not, and
 *    ERROR if there are unexpected problems.
 *
 * ==> Currently an ERROR return is impossible, but reserved for future use.
 *
 * Related:
 *     kjb_use_default_paging, kjb_enable_paging, kjb_disable_paging
 *
 * Index: output, paging, terminal I/O, pause
 *
 * -----------------------------------------------------------------------------
 */

int kjb_restore_paging(void)
{
    Queue_element* cur_elem;


    if (fs_page_flag_stack_head == NULL)
    {
        SET_CANT_HAPPEN_BUG();
        return ERROR;
    }

    NRE(cur_elem = remove_first_element(&fs_page_flag_stack_head,
                                        (Queue_element**)NULL));

    fs_page_flag = *((Bool*)(cur_elem->contents));

    SKIP_HEAP_CHECK_2();
    free_queue_element(cur_elem, kjb_free);
    CONTINUE_HEAP_CHECK_2();

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

#ifdef TRACK_MEMORY_ALLOCATION
/*
 * =============================================================================
 * STATIC                      free_page_flag_stack
 * -----------------------------------------------------------------------------
 */

static void free_page_flag_stack(void)
{
#ifdef TEST
    IMPORT int kjb_debug_level;
    IMPORT int kjb_fork_depth; 

    if (kjb_debug_level > 4)
    {
        /* Too dangerous to use KJB IO at this point.  */
        fprintf(stderr, "<<TEST>> Process %ld (fork depth %d) is freeing page flag stack.\n",
                 (long)MY_PID, kjb_fork_depth);
    }
#endif 

    free_queue(&fs_page_flag_stack_head, (Queue_element**)NULL, kjb_free);
}

#endif

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*
 * =============================================================================
 *                                  is_in_background
 *
 * Index: strings
 *
 * -----------------------------------------------------------------------------
 */

/*
//
//  Currently this function does not work for NEXT.
//
//  This function was moved from l_sys_std.c so that fs_tty_in could be made
//  static.
*/

int is_in_background (void)
{


    if (fs_tty_in == NULL)
    {
        /*
        // Open_tty_in sets fs_in_background. If the open fails we may very will
        // be in the background, but assume we are not.
        */

        open_tty_in();
    }

    return fs_in_background;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*
 * =============================================================================
 * STATIC               open_tty_in
 * -----------------------------------------------------------------------------
 */
/* all */     static FILE* open_tty_in(void)
/* all */     {
/* all */         FILE*        fp = NULL;
/* all */         static int   first_time = TRUE;
                  int must_close_it = FALSE; 
#ifndef MS_OS
/* NOT MS_OS */   char  term_name[ L_ctermid + ROOM_FOR_NULL ];
#endif
/* all */
/* all */         if (! first_time)
/* all */         {
/* all */             return fs_tty_in;
/* all */         }
/* all */         else
/* all */         {
/* all */             first_time = FALSE;
/* all */         }
/* all */
/* all */         fs_in_background = FALSE;
#ifdef TEST
#ifdef UNIX
/* TEST */        fs_emulate_term_input_fp = kjb_fopen("emulate_term_input", "rb");
#endif
#endif

#ifdef MS_OS
/* MS_OS */       fp=kjb_fopen("CON", "r");
/* MS_OS */
/* MS_OS */       if (fp != NULL)
/* MS_OS */       {
/* MS_OS */           setbuf(fp, NULL);
/* MS_OS */       }
#else
/* default */
/* default */

                  /* Marginally better for debuggers. Previously we did this for
                   * TEST only, but better to test and run with the same code? 
                  */
                  if (kjb_isatty(fileno(stdin)))
                  {
                      fp = stdin;
                  }

/* default */     if (fp == NULL)
/* default */     {
/* default */         if (ctermid (term_name ) == NULL)
/* default */         {
#ifdef TEST
/* default */             set_error("Ctermid call failed for %s.%S", term_name);
#else
/* default */             set_error("System call failure.");
#endif
/* default */
/* default */         }
/* default */         else
/* default */         {
#ifdef SYSV
/* SYSV */                kjb_signal(SIGTTOU, SIG_IGN);
#endif
/* default */             fp=kjb_fopen(term_name, "r");
                          must_close_it = TRUE; 
#ifdef SYSV
/* SYSV */                kjb_signal(SIGTTOU, SIG_DFL);
#endif
                      }
                  }
             
                  if (fp != NULL)
                  {
                      /* This does NOT work for NEXT. Currently on  */
                      /* NEXT the variable fs_in_background is always  */
                      /* the initial value of FALSE.                */
               
                      int foreground_process_group = tcgetpgrp(fileno(fp));
               
               
                      /* If the above is ERROR, we will rashly assume that we */
                      /* are being debugged, or in similar circumstances      */
                      /* where we it is desirable to treat the terminal which */
                      /* in some sense is not behaving as a termian, as a     */
                      /* terminal.                                            */
               
                      if (    (foreground_process_group > 0)
                           && (foreground_process_group != MY_GROUP)
                         )
                      {
                          fs_in_background = TRUE;
                      }
               
                      if (setvbuf(fp, (char*)NULL, _IONBF, 0) != 0)
                      {
#ifdef TEST
                          set_error("Setvbuf call failed for %F.%S",
                                    fp);
#else
                          set_error("System call failure.");
#endif
               
                          fp = NULL;
                      }
                  }
#endif
/* all */
/* all */          if (fp == NULL)
/* all */          {
/* all */              insert_error("Terminal for input not available");
/* all */          }
/* all */          else
/* all */          {
/* all */              fs_tty_in = fp;
/* all */              setup_smart_terminal();

                       if (must_close_it)
                       {
                           EPE(add_cleanup_function(close_tty_in)); 
                       }
/* all */          }
/* all */
/* all */          return fp;
/* all */      }

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

static void close_tty_in()
{
    EPE(kjb_fclose(fs_tty_in));
    fs_tty_in = NULL;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

#ifdef TEST
#ifdef UNIX
/* TEST */ int record_term_input(void)
/* TEST */ {
/* TEST */     char term_input_file[ 200 ];
/* TEST */     char pid_string[ 100 ];
/* TEST */     int pid;
/* TEST */
/* TEST */
/* TEST */     if (fs_term_input_fp == NULL)
/* TEST */     {
/* TEST */         BUFF_CPY(term_input_file, "term_input");
/* TEST */         pid = getpid();
/* TEST */         strcat(term_input_file, ".");
/* TEST */         sprintf(pid_string, "%ld", (long)pid);
/* TEST */         strcat(term_input_file, pid_string);
/* TEST */
/* TEST */         NRE(fs_term_input_fp = kjb_fopen(term_input_file, "w"));
/* TEST */
/* TEST */         kjb_fprintf(stderr,
/* TEST */                     "Terminal input is being recorded in %s.\n",
/* TEST */                     term_input_file);
/* TEST */     }
/* TEST */     return NO_ERROR;
/* TEST */ }
/* TEST */
#endif
#endif

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*
 * =============================================================================
 * STATIC                   open_tty_out
 * -----------------------------------------------------------------------------
 */

static FILE* open_tty_out(void)
{
    static int   first_time = TRUE;
    FILE*        fp = NULL;
    char term_name[ L_ctermid + ROOM_FOR_NULL ];
    int must_close_it = FALSE; 

    if (! first_time)
    {
        return fs_tty_out;
    }
    else
    {
        first_time = FALSE;
    }

#ifdef TEST
    /* This method, which is a reasonable alternative, works a
    // bit better with debuggers.
    */
    if (kjb_isatty(fileno(stderr)))
    {
        fp = stderr;
    }
#endif
    if (fp == NULL)
    {
        if (ctermid (term_name ) == NULL)
        {
#ifdef TEST
            set_error("Ctermid call failed for %s.%S", term_name);
#else
            set_error("System call failure.");
#endif

        }
        else
        {
            term_name[ L_ctermid ] = '\0';

            fp=kjb_fopen(term_name, "w");

            if (fp != NULL)
            {
                must_close_it = TRUE; 

                if (setvbuf(fp, (char*)NULL, _IONBF, 0) != 0)
                {
#ifdef TEST
                    set_error("Setvbuf call failed for %F.%S",
                              fp);
#else
                    set_error("System call failure.");
#endif
                }
            }
        }
    }
    /* The next bit can be relavent if we have used up all our */
    /* file descriptors, but have not opened fs_tty_out. To print */
    /* an error  message to this effect, we need a file.       */
    /* Similar reasoning could apply in other cases as well.   */

    if ((fp == NULL) && (kjb_isatty(fileno(stderr))))
    {
        fp = stderr;
    }

    if (fp == NULL)
    {
        insert_error("Terminal for output not available");
    }
    else
    {
        fs_tty_out = fp;
        setup_smart_terminal();

        if (must_close_it)
        {
            EPE(add_cleanup_function(close_tty_out)); 
        }
    }

    return fp;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

static void close_tty_out()
{
    EPE(kjb_fclose(fs_tty_out));
    fs_tty_out = NULL; 
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*
 * =============================================================================
 * STATIC               setup_smart_terminal
 * -----------------------------------------------------------------------------
 */

/* all */     static void setup_smart_terminal(void)
/* all */     {
/* all */         static int first_time = TRUE;
/* all */         FILE* fp;
#ifdef UNIX
/* UNIX */        IMPORT char PC;
#ifdef NEXT
/* next */        struct sgttyb term_flags;
#else
/* NOT next */    struct termios term_flags;
#endif
/* UNIX */        char termcap_buff[ 1024 ], *bp;
/* UNIX */        char control_sequence_buff[ MAX_CTL_SEQ_LEN ];
/* UNIX */        char* control_sequence_buff_ptr;
/* UNIX */        char* term_type;
/* UNIX */        char tgetstr_arg_one[ 100 ];
/* UNIX */        int vt100_type;
/* UNIX */        int res;
/* UNIX */        char* tgetstr_res;
/* UNIX */        int use_reverse_video = FALSE;
#endif
/* all */
/* all */         if (! first_time) return;
/* all */         first_time = FALSE;
/* all */
/* all */         if (fs_tty_in != NULL)
/* all */         {
/* all */             fp = fs_tty_in;
/* all */         }
/* all */         else if (fs_tty_out != NULL)
/* all */         {
/* all */             fp = fs_tty_out;
/* all */         }
/* all */         else
/* all */         {
/* all */             return;
/* all */         }
/* all */
/* all */         fs_smart_terminal_capability = FALSE;
/* all */
/* all */         setup_terminal_size();
#ifdef UNIX
/* UNIX */        kjb_signal(SIGWINCH, reset_terminal_size_on_sig_fn);
#ifdef NEXT
/* next */        ER(kjb_ioctl(fileno(fp), (IOCTL_REQUEST_TYPE)TIOCGETP, &term_flags));
/* next */        fs_initial_term_flags_set = TRUE;
/* next */        fs_initial_term_flags = term_flags;
/* next */
/* next */        fs_erase_char = term_flags.sg_erase;
#else
/* NOT next */    ER(kjb_ioctl(fileno(fp), (IOCTL_REQUEST_TYPE)TCGETS, (void*) &term_flags));
/* NOT next */
/* NOT next */    fs_initial_term_flags_set = TRUE;
/* NOT next */    fs_initial_term_flags = term_flags;
/* NOT next */
/* NOT next */    fs_erase_char = term_flags.c_cc[VERASE];
#endif
#ifndef NEXT
#endif
/* UNIX */
/* UNIX */        bp=termcap_buff;
/* UNIX */        NR(term_type=getenv("TERM"));
/* UNIX */        res=tgetent(bp, term_type);
/* UNIX */        if (res != 1) return;
/* UNIX */
/* UNIX */        if (    (HEAD_CMP_EQ(term_type, "vt1"))
/* UNIX */             || (HEAD_CMP_EQ(term_type, "vt2"))
/* UNIX */             || (HEAD_CMP_EQ(term_type, "vt-1"))
/* UNIX */             || (HEAD_CMP_EQ(term_type, "vt-2"))
#ifdef AIX
/* AIX  */             || (HEAD_CMP_EQ(term_type, "aixterm"))
#endif
#ifdef HPUX
/* HPUX */             || (HEAD_CMP_EQ(term_type, "dtterm"))
#endif
/* UNIX */             || (HEAD_CMP_EQ(term_type, "xterm"))
/* UNIX */             || (HEAD_CMP_EQ(term_type, "kterm"))
/* UNIX */           )
/* UNIX */        {
/* UNIX */            vt100_type = TRUE;
/* UNIX */        }
/* UNIX */        else vt100_type = FALSE;
/* UNIX */
/* UNIX */
/* UNIX */        control_sequence_buff_ptr = control_sequence_buff;
/* UNIX */        BUFF_CPY(tgetstr_arg_one, "pc");  /* Hack to avoid warning. */
/* UNIX */        tgetstr_res = tgetstr(tgetstr_arg_one,
/* UNIX */                              &control_sequence_buff_ptr);
/* UNIX */
/* UNIX */        if (tgetstr_res == (char*)NULL)
/* UNIX */        {
/* UNIX */            PC = '\0';
/* UNIX */        }
/* UNIX */        else
/* UNIX */        {
#ifdef LINUX
/* UNIX */            PC = *tgetstr_res;
#else
/* UNIX */            PC = *control_sequence_buff_ptr;
#endif
/* UNIX */        }
/* UNIX */
/* UNIX */        control_sequence_buff_ptr = control_sequence_buff;
/* UNIX */
/* UNIX */        if (use_reverse_video)
/* UNIX */        {
/* UNIX */            BUFF_CPY(tgetstr_arg_one, "mr");  /* Avoid warning. */
/* UNIX */            tgetstr_res = tgetstr(tgetstr_arg_one,
/* UNIX */                                  &control_sequence_buff_ptr);
/* UNIX */        }
/* UNIX */        else
/* UNIX */        {
/* UNIX */            BUFF_CPY(tgetstr_arg_one, "md");  /* Avoid warning. */
/* UNIX */            tgetstr_res = tgetstr(tgetstr_arg_one,
/* UNIX */                                  &control_sequence_buff_ptr);
/* UNIX */        }
/* UNIX */
/* UNIX */        if (tgetstr_res != NULL)
/* UNIX */        {
#ifdef LINUX
/* UNIX */            BUFF_CPY(fs_high_light_on_sequence, tgetstr_res);
#else
/* UNIX */            *control_sequence_buff_ptr = '\0';
/* UNIX */            BUFF_CPY(fs_high_light_on_sequence,
/* UNIX */                     control_sequence_buff);
#endif
/* UNIX */        }
/* UNIX */        else if (vt100_type)
/* UNIX */        {
/* UNIX */            BUFF_CPY(fs_high_light_on_sequence, "[1m");
/* UNIX */        }
/* UNIX */        else
/* UNIX */        {
/* UNIX */            fs_high_light_on_sequence[ 0 ] = '\0';
/* UNIX */        }
/* UNIX */
/* UNIX */        control_sequence_buff_ptr = control_sequence_buff;
/* UNIX */
/* UNIX */        BUFF_CPY(tgetstr_arg_one, "me");  /* Avoid warning. */
/* UNIX */        tgetstr_res = tgetstr(tgetstr_arg_one,
/* UNIX */                              &control_sequence_buff_ptr);
/* UNIX */
/* UNIX */        if (tgetstr_res != NULL)
/* UNIX */        {
#ifdef LINUX
/* UNIX */            BUFF_CPY(fs_high_light_off_sequence, tgetstr_res);
#else
/* UNIX */            *control_sequence_buff_ptr = '\0';
/* UNIX */            BUFF_CPY(fs_high_light_off_sequence,
/* UNIX */                     control_sequence_buff);
#endif
/* UNIX */        }
/* UNIX */        else if (vt100_type)
/* UNIX */        {
/* UNIX */            BUFF_CPY(fs_high_light_off_sequence, "[m");
/* UNIX */        }
/* UNIX */        else
/* UNIX */        {
/* UNIX */            fs_high_light_off_sequence[ 0 ] = '\0';
/* UNIX */        }
/* UNIX */
/* UNIX */        /* ---------------------------------------- */
/* UNIX */
/* UNIX */        control_sequence_buff_ptr = control_sequence_buff;
/* UNIX */
/* UNIX */        BUFF_CPY(tgetstr_arg_one, "ku");  /* Avoid warning. */
/* UNIX */        tgetstr_res = tgetstr(tgetstr_arg_one,
/* UNIX */                              &control_sequence_buff_ptr);
/* UNIX */        NR(tgetstr_res);
#ifdef LINUX
/* UNIX */        BUFF_CPY(control_sequence_buff, tgetstr_res);
#else
/* UNIX */        *control_sequence_buff_ptr = '\0';
#endif
/* UNIX */
#if 0 /* was ifdef OBSOLETE */
/* UNIX */        if ((strlen(control_sequence_buff) == 3) &&
/* UNIX */            ((vt100_type == TRUE) &&
/* UNIX */             (fs_vt100_input_kludge == TRUE)))
/* UNIX */        {
/* UNIX */            control_sequence_buff[ 1 ] = '[';
/* UNIX */        }
#else
/* UNIX */        if (strlen(control_sequence_buff) == 3)
/* UNIX */        {
/* UNIX */            /* Another attempt at the 'O' vs. '[' problem.          */
/* UNIX */            /* The * means anything can go into that slot on input. */
/* UNIX */            control_sequence_buff[ 1 ] = '*';
/* UNIX */        }
#endif
/* UNIX */        BUFF_CPY(fs_input_up_arrow_sequence, control_sequence_buff);
/* UNIX */
/* UNIX */        /* ---------------------------------------- */
/* UNIX */
/* UNIX */        control_sequence_buff_ptr = control_sequence_buff;
/* UNIX */
/* UNIX */        BUFF_CPY(tgetstr_arg_one, "up");  /* Avoid warning. */
/* UNIX */        tgetstr_res = tgetstr(tgetstr_arg_one,
/* UNIX */                              &control_sequence_buff_ptr);
/* UNIX */        NR(tgetstr_res);
#ifdef LINUX
/* UNIX */        BUFF_CPY(control_sequence_buff, tgetstr_res);
#else
/* UNIX */        *control_sequence_buff_ptr = '\0';
#endif
/* UNIX */
/* UNIX */        if ((strlen(control_sequence_buff) == 3) &&
/* UNIX */            ((vt100_type == TRUE) &&
/* UNIX */             (fs_vt100_output_kludge == TRUE)))
/* UNIX */        {
/* UNIX */            control_sequence_buff[ 1 ] = '[';
/* UNIX */        }
/* UNIX */
/* UNIX */        BUFF_CPY(fs_output_up_arrow_sequence, control_sequence_buff);
/* UNIX */
/* UNIX */        /* ---------------------------------------- */
/* UNIX */
/* UNIX */        control_sequence_buff_ptr = control_sequence_buff;
/* UNIX */
/* UNIX */        BUFF_CPY(tgetstr_arg_one, "kd");  /* Avoid warning. */
/* UNIX */        tgetstr_res = tgetstr(tgetstr_arg_one,
/* UNIX */                              &control_sequence_buff_ptr);
/* UNIX */        NR(tgetstr_res);
#ifdef LINUX
/* UNIX */        BUFF_CPY(control_sequence_buff, tgetstr_res);
#else
/* UNIX */        *control_sequence_buff_ptr = '\0';
#endif
/* UNIX */
#if 0 /* was ifdef OBSOLETE */
/* UNIX */        if ((strlen(control_sequence_buff) == 3) &&
/* UNIX */             ((vt100_type == TRUE) &&
/* UNIX */              (fs_vt100_input_kludge == TRUE)))
/* UNIX */        {
/* UNIX */             control_sequence_buff[ 1 ] = '[';
/* UNIX */        }
#else
/* UNIX */        if (strlen(control_sequence_buff) == 3)
/* UNIX */        {
/* UNIX */            /* Another attempt at the 'O' vs. '[' problem.          */
/* UNIX */            /* The * means anything can go into that slot on input. */
/* UNIX */            control_sequence_buff[ 1 ] = '*';
/* UNIX */        }
#endif
/* UNIX */        BUFF_CPY(fs_input_down_arrow_sequence,  control_sequence_buff);
/* UNIX */
/* UNIX */        /* ---------------------------------------- */
/* UNIX */
/* UNIX */        control_sequence_buff_ptr = control_sequence_buff;
/* UNIX */
/* UNIX */        BUFF_CPY(tgetstr_arg_one, "do");  /* Avoid warning. */
/* UNIX */        tgetstr_res = tgetstr(tgetstr_arg_one,
/* UNIX */                              &control_sequence_buff_ptr);
/* UNIX */        if (tgetstr_res != NULL)
/* UNIX */        {
#ifdef LINUX
/* UNIX */            BUFF_CPY(control_sequence_buff, tgetstr_res);
#else
/* UNIX */            *control_sequence_buff_ptr = '\0';
#endif
/* UNIX */
/* UNIX */
/* UNIX */            if ((strlen(control_sequence_buff) == 3) &&
/* UNIX */                ((vt100_type == TRUE) &&
/* UNIX */                 (fs_vt100_output_kludge == TRUE)))
/* UNIX */            {
/* UNIX */                control_sequence_buff[ 1 ] = '[';
/* UNIX */            }
/* UNIX */
/* UNIX */            BUFF_CPY(fs_output_down_arrow_sequence,
/* UNIX */                     control_sequence_buff);
/* UNIX */         }
/* UNIX */         else
/* UNIX */         {
/* UNIX */             BUFF_CPY(fs_output_down_arrow_sequence,
/* UNIX */                      fs_input_down_arrow_sequence);
/* UNIX */         }
/* UNIX */
/* UNIX */        /* ---------------------------------------- */
/* UNIX */
/* UNIX */        control_sequence_buff_ptr = control_sequence_buff;
/* UNIX */
/* UNIX */        BUFF_CPY(tgetstr_arg_one, "kl");  /* Avoid warning. */
/* UNIX */        tgetstr_res = tgetstr(tgetstr_arg_one,
/* UNIX */                              &control_sequence_buff_ptr);
/* UNIX */        NR(tgetstr_res);
#ifdef LINUX
/* UNIX */        BUFF_CPY(control_sequence_buff, tgetstr_res);
#else
/* UNIX */        *control_sequence_buff_ptr = '\0';
#endif
/* UNIX */
#if 0 /* was ifdef OBSOLETE */
/* UNIX */        if ((strlen(control_sequence_buff) == 3) &&
/* UNIX */            ((vt100_type == TRUE) &&
/* UNIX */             (fs_vt100_input_kludge == TRUE)))
/* UNIX */        {
/* UNIX */            control_sequence_buff[ 1 ] = '[';
/* UNIX */        }
#else
/* UNIX */        if (strlen(control_sequence_buff) == 3)
/* UNIX */        {
/* UNIX */            /* Another attempt at the 'O' vs. '[' problem.          */
/* UNIX */            /* The * means anything can go into that slot on input. */
/* UNIX */            control_sequence_buff[ 1 ] = '*';
/* UNIX */        }
#endif
/* UNIX */        BUFF_CPY(fs_input_left_arrow_sequence, control_sequence_buff);
/* UNIX */
/* UNIX */        /* ---------------------------------------- */
/* UNIX */
/* UNIX */        control_sequence_buff_ptr = control_sequence_buff;
/* UNIX */
/* UNIX */        BUFF_CPY(tgetstr_arg_one, "le");  /* Avoid warning. */
/* UNIX */        tgetstr_res = tgetstr(tgetstr_arg_one,
/* UNIX */                              &control_sequence_buff_ptr);
/* UNIX */        if (tgetstr_res != NULL)
/* UNIX */        {
#ifdef LINUX
/* UNIX */            BUFF_CPY(control_sequence_buff, tgetstr_res);
#else
/* UNIX */            *control_sequence_buff_ptr = '\0';
#endif
/* UNIX */
/* UNIX */
/* UNIX */            if ((strlen(control_sequence_buff) == 3) &&
/* UNIX */                ((vt100_type == TRUE) &&
/* UNIX */                 (fs_vt100_output_kludge == TRUE)))
/* UNIX */            {
/* UNIX */                control_sequence_buff[ 1 ] = '[';
/* UNIX */            }
/* UNIX */            BUFF_CPY(fs_output_left_arrow_sequence,
/* UNIX */                     control_sequence_buff);
/* UNIX */        }
/* UNIX */        else
/* UNIX */        {
/* UNIX */             BUFF_CPY(fs_output_left_arrow_sequence,
/* UNIX */                      fs_input_left_arrow_sequence);
/* UNIX */
/* UNIX */        }
/* UNIX */
/* UNIX */        /* ---------------------------------------- */
/* UNIX */
/* UNIX */         control_sequence_buff_ptr = control_sequence_buff;
/* UNIX */
/* UNIX */        BUFF_CPY(tgetstr_arg_one, "kr");  /* Avoid warning. */
/* UNIX */        tgetstr_res = tgetstr(tgetstr_arg_one,
/* UNIX */                              &control_sequence_buff_ptr);
/* UNIX */        NR(tgetstr_res);
#ifdef LINUX
/* UNIX */        BUFF_CPY(control_sequence_buff, tgetstr_res);
#else
/* UNIX */        *control_sequence_buff_ptr = '\0';
#endif
/* UNIX */
#if 0 /* was ifdef OBSOLETE */
/* UNIX */         if ((strlen(control_sequence_buff) == 3) &&
/* UNIX */             ((vt100_type == TRUE) &&
/* UNIX */              (fs_vt100_input_kludge == TRUE)))
/* UNIX */         {
/* UNIX */             control_sequence_buff[ 1 ] = '[';
/* UNIX */         }
#else
/* UNIX */         if (strlen(control_sequence_buff) == 3)
/* UNIX */         {
/* UNIX */             /* Another attempt at the 'O' vs. '[' problem.          */
/* UNIX */             /* The * means anything can go into that slot on input. */
/* UNIX */             control_sequence_buff[ 1 ] = '*';
/* UNIX */         }
#endif
/* UNIX */         BUFF_CPY(fs_input_right_arrow_sequence, control_sequence_buff);
/* UNIX */
/* UNIX */        /* ---------------------------------------- */
/* UNIX */
/* UNIX */         control_sequence_buff_ptr = control_sequence_buff;
/* UNIX */
/* UNIX */        BUFF_CPY(tgetstr_arg_one, "nd");  /* Avoid warning. */
/* UNIX */        tgetstr_res = tgetstr(tgetstr_arg_one,
/* UNIX */                              &control_sequence_buff_ptr);
/* UNIX */        NR(tgetstr_res);
#ifdef LINUX
/* UNIX */        BUFF_CPY(control_sequence_buff, tgetstr_res);
#else
/* UNIX */        *control_sequence_buff_ptr = '\0';
#endif
/* UNIX */         if ((strlen(control_sequence_buff) == 3) &&
/* UNIX */             ((vt100_type == TRUE) &&
/* UNIX */              (fs_vt100_output_kludge == TRUE)))
/* UNIX */         {
/* UNIX */             control_sequence_buff[ 1 ] = '[';
/* UNIX */         }
/* UNIX */
/* UNIX */         BUFF_CPY(fs_output_right_arrow_sequence,
/* UNIX */                  control_sequence_buff);
/* UNIX */
/* UNIX */         fs_smart_terminal_capability = TRUE;
#else
#ifdef MS_OS
/* MS_OS */        /* UNFINISHED */
/* MS_OS */
/* MS_OS */        fs_erase_char = ''; /* what ?? */
/* MS_OS */
/* MS_OS */        BUFF_CPY(fs_high_light_on_sequence, "" /* ?? */ );
/* MS_OS */        BUFF_CPY(fs_high_light_off_sequence, "" /* ?? */ );
/* MS_OS */        BUFF_CPY(fs_input_right_arrow_sequence, ""/*  ?? */ );
/* MS_OS */        BUFF_CPY(fs_output_right_arrow_sequence, ""/*  ?? */ );
/* MS_OS */        BUFF_CPY(fs_input_left_arrow_sequence, ""/*  ?? */ );
/* MS_OS */        BUFF_CPY(fs_output_left_arrow_sequence, ""/*  ?? */ );
/* MS_OS */        BUFF_CPY(fs_input_up_arrow_sequence, ""/*  ?? */ );
/* MS_OS */        BUFF_CPY(fs_output_up_arrow_sequence, ""/*  ?? */ );
/* MS_OS */        BUFF_CPY(fs_input_down_arrow_sequence, ""/*  ?? */ );
/* MS_OS */        BUFF_CPY(fs_output_down_arrow_sequence, ""/*  ?? */ );
/* MS_OS */
/* MS_OS */        fs_smart_terminal_capability = TRUE;
#else
/* default */
#endif
#endif
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*
 * =============================================================================
 *
 * -----------------------------------------------------------------------------
 */

/* all */     void setup_terminal_size(void)
/* all */     {
/* all */         IMPORT volatile int kjb_tty_cols, kjb_tty_rows;
/* all */         FILE* fp;
#ifdef UNIX
#ifndef SGI
/* NOT SGI */     struct winsize window_size;
/* NOT SGI */     int temp, res;
/* NOT SGI */     char termcap_buff[ 1024 ], *bp;
/* NOT SGI */     char* term_type;
/* NOT SGI */     char tgetnum_arg[ 100 ];
#endif
#endif
/* all */
/* all */         if (fs_tty_in != NULL)
/* all */         {
/* all */             fp = fs_tty_in;
/* all */         }
/* all */         else if (fs_tty_out != NULL)
/* all */         {
/* all */             fp = fs_tty_out;
/* all */         }
/* all */         else
/* all */         {
/* all */             return;
/* all */         }
/* all */
/* all */         /* Reset sane defaults, to start. On some systems, this is */
/* all */         /* that gets done. */
/* all */
/* all */         kjb_tty_cols = 80;
/* all */         kjb_tty_rows = 24;
#ifdef UNIX
#ifdef SGI
/* SGI */         setupterm(NULL, fileno(fp), NULL);
/* SGI */         kjb_tty_cols = tgetnum("cols") ;
/* SGI */         kjb_tty_rows = tgetnum("lines") ;
/* SGI */         reset_shell_mode();
#else
/* NOT SGI */     res=ioctl(fileno(fp), (int)TIOCGWINSZ, (void*)&window_size);
/* NOT SGI */
/* NOT SGI */     if ((res != EOF) &&
/* NOT SGI */         (window_size.ws_col != 0) &&
/* NOT SGI */         (window_size.ws_row != 0))
/* NOT SGI */     {
/* NOT SGI */         kjb_tty_cols = window_size.ws_col;
/* NOT SGI */         kjb_tty_rows = window_size.ws_row;
/* NOT SGI */     }
/* NOT SGI */     else
/* NOT SGI */     {
/* NOT SGI */         bp=termcap_buff;
/* NOT SGI */
/* NOT SGI */         NR(term_type=getenv("TERM"));
/* NOT SGI */         res=tgetent(bp, term_type);
/* NOT SGI */
/* NOT SGI */         if (res != 1) return;
/* NOT SGI */
/* NOT SGI */         BUFF_CPY(tgetnum_arg, "co"); /* Avoid warning. */
/* NOT SGI */         if ((temp = tgetnum(tgetnum_arg)) == EOF) return;
/* NOT SGI */
/* NOT SGI */         kjb_tty_cols = temp;
/* NOT SGI */
/* NOT SGI */         BUFF_CPY(tgetnum_arg, "li"); /* Avoid warning. */
/* NOT SGI */         if ((temp = tgetnum(tgetnum_arg)) == EOF) return;
/* NOT SGI */
/* NOT SGI */         kjb_tty_rows = temp;
/* NOT SGI */     }
/* NOT SGI */
#endif
#else
#ifdef MS_OS
/* MS_OS */        kjb_tty_rows = 25;
#else
#endif
#endif
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*
 * =============================================================================
 *                                  put_prompt
 *
 *
 *
 * Index: terminal I/O
 *
 * -----------------------------------------------------------------------------
 */

int put_prompt(const char* prompt)
{


    return term_put_n_raw_chars(prompt, strlen(prompt));
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*
 * =============================================================================
 *                                  term_puts
 *
 *
 *
 * Index: terminal I/O
 *
 * -----------------------------------------------------------------------------
 */

int term_puts(const char* line)
{

    return term_put_n_chars(line, strlen(line));
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*
 * =============================================================================
 *                                 term_put_n_chars
 *
 *
 * Index: terminal I/O
 *
 * -----------------------------------------------------------------------------
 */

int term_put_n_chars(const char* line, size_t len)
{
    IMPORT volatile int kjb_tty_cols;
    IMPORT volatile int num_term_chars;
    IMPORT volatile Bool halt_term_output;
    IMPORT volatile Bool pause_on_next;
    IMPORT volatile Bool term_line_wrap_flag;
    size_t              pos;
    size_t              byte_count;
    int         res;
    const char*         line_pos;
    int                 force_new_line;


    if (halt_term_output) return NO_ERROR;

    /*
     * If we don't do anything extra, then take a shortcut. Note that
     * it is a good idea to maintain the code below so that it works
     * even if this shortcut is not taken. IE, if the following lines
     * are commented out, everthing should still work.
     */
    if ((!fs_page_flag) && (!term_line_wrap_flag))
    {
        return term_put_n_raw_chars(line, len);
    }

    byte_count=0;

    while (len > 0)
    {
        line_pos = line;
        pos = 0;

        if (pause_on_next == TRUE)
        {
            term_page_end();
        }

        if (halt_term_output) return NO_ERROR;

        force_new_line = FALSE;

        while ( (pos<len) && (*line_pos != '\n'))
        {
            if (isprint((int)(*line_pos) ) )
            {
                num_term_chars++;
            }
            else if (*line_pos == '\r')
            {
                num_term_chars=0;
            }

            if (num_term_chars > kjb_tty_cols)
            {
                force_new_line = TRUE;
                break;
            }

            pos++;
            line_pos++;
        }

        /*
         * Without the following, we don't print new lines.
         *
         * Note: If the following is true, then we can't have
         * force_new_line true so we don't need to check.
         */
        if ((pos < len) && (*line_pos == '\n'))
        {
            pos++;
        }

        ERE(res=term_put_n_raw_chars(line, pos));
        byte_count += res;

        if (   (force_new_line)
               || ((line_pos < line + len) && (*line_pos == '\n'))
           )
        {
            term_line_end();
        }

        if ((force_new_line) && (term_line_wrap_flag))
        {
            term_put_n_raw_chars("\n", 1);
        }

        line += pos;
        len -= pos;
    }

    return byte_count;
}

/*
 * =============================================================================
 *                         term_put_n_raw_chars
 *
 *
 *
 * Index: terminal I/O
 *
 * -----------------------------------------------------------------------------
 */

int term_put_n_raw_chars(const char* line, size_t len)
{
    IMPORT volatile int term_io_since_last_input_attempt;
    size_t fwrite_res;

    NRE(open_tty_out());

    if (len == 0) return 0;

    fwrite_res = fwrite((const void*)line, 1, len, fs_tty_out);

    if (fwrite_res != len)
    {
        set_error("Write of %d bytes to terminal failed.%S", len);
        return ERROR;
    }

    ERE(kjb_fflush(fs_tty_out));

    term_io_since_last_input_attempt = TRUE;
    return len;
}

/*
 * =============================================================================
 *                              term_get_n_chars
 *
 *
 *
 * Index: terminal I/O
 *
 * -----------------------------------------------------------------------------
 */

#ifdef UNIX
/* UNIX */    int term_get_n_chars (const char* prompt, char* line, size_t max_len)
/* UNIX */    {
/* UNIX */        IMPORT int restart_io_after_sig[ SIGNAL_ARRAY_SIZE ];
/* UNIX */        IMPORT volatile int recorded_signal;
/* UNIX */        IMPORT volatile Bool pause_on_next;
/* UNIX */        IMPORT volatile int num_term_chars;
/* UNIX */        IMPORT volatile int num_term_lines;
/* UNIX */        IMPORT volatile int term_io_since_last_input_attempt;
/* UNIX */        int done;
/* UNIX */        int res;
/* UNIX */
/* UNIX */
/* UNIX */        term_io_since_last_input_attempt = FALSE;
/* UNIX */
/* UNIX */        NRE(open_tty_out());
/* UNIX */        NRE(open_tty_in());
/* UNIX */
/* UNIX */        if (max_len == 0) return 0;
/* UNIX */
/* UNIX */        ERE(term_set_raw_mode_with_echo());
/* UNIX */
/* UNIX */        done = FALSE;
/* UNIX */
/* UNIX */        while ( ! done)
/* UNIX */        {
/* UNIX */            recorded_signal = NOT_SET;
/* UNIX */
/* UNIX */            res = term_get_n_chars_guts(prompt, line, max_len);
/* UNIX */
/* UNIX */            if (res == INTERRUPTED)
/* UNIX */            {
/* UNIX */                if ((recorded_signal > 0) &&
/* UNIX */                    (recorded_signal < SIGNAL_ARRAY_SIZE) &&
/* UNIX */                    (restart_io_after_sig[recorded_signal]))
/* UNIX */                {
/* UNIX */                    done = FALSE;
/* UNIX */                }
/* UNIX */                else
/* UNIX */                {
/* UNIX */                    done = TRUE;
/* UNIX */                }
/* UNIX */
/* UNIX */            }
/* UNIX */            else
/* UNIX */            {
/* UNIX */                done = TRUE;
/* UNIX */            }
/* UNIX */
/* UNIX */            num_term_lines = 0;
/* UNIX */            num_term_chars = 0;
/* UNIX */            pause_on_next = FALSE;
/* UNIX */        }
/* UNIX */
/* UNIX */        term_reset();
/* UNIX */
/* UNIX */        return res;
/* UNIX */    }
/* UNIX */
/* UNIX */     /* Assuming max_len > 0, and fs_tty_in and fs_tty_out are OK !!! */
/* UNIX */     /* Also assuming raw mode with echo.                       */
/* UNIX */
/* UNIX */     static int term_get_n_chars_guts (const char* prompt, char* line, size_t max_len)
/* UNIX */     {
/* UNIX */         char* line_pos;
/* UNIX */         size_t i;
/* UNIX */         int c;
/* UNIX */         int forward_line;
/* UNIX */
/* UNIX */
/* UNIX */         ERE(put_prompt(prompt));
/* UNIX */
/* UNIX */         max_len--;
/* UNIX */         line_pos = line;
/* UNIX */
/* UNIX */
/* UNIX */         i=0;
/* UNIX */         forward_line = FALSE;
/* UNIX */         c = '\0';
/* UNIX */
/* UNIX */         while ( ( !forward_line ) && (i<max_len ) )
/* UNIX */         {
/* UNIX */              c = term_getc();
/* UNIX */
/* UNIX */              if (c == '') c = EOF;
/* UNIX */
/* UNIX */              if ((c == EOF) || (c == '\n') || (c == INTERRUPTED))
/* UNIX */              {
/* UNIX */                  forward_line = TRUE;
/* UNIX */              }
/* UNIX */              else
/* UNIX */              {
/* UNIX */                  *line_pos = c;
/* UNIX */                  line_pos++;
/* UNIX */                  i++;
/* UNIX */              }
/* UNIX */          }
/* UNIX */
/* UNIX */         *line_pos = '\0';
/* UNIX */
/* UNIX */         if (c == EOF)
/* UNIX */         {
/* UNIX */             rewind(fs_tty_in);
/* UNIX */         }
/* UNIX */         else if (c == INTERRUPTED)
/* UNIX */         {
/* UNIX */             *line = '\0';
/* UNIX */         }
/* UNIX */
/* UNIX */
/* UNIX */         if (c != '\n')
/* UNIX */         {
/* UNIX */             NRE(open_tty_out());
/* UNIX */             kjb_fputc(fs_tty_out, '\n');
/* UNIX */         }
/* UNIX */
/* UNIX */         if (c == EOF)
/* UNIX */         {
/* UNIX */             return EOF;
/* UNIX */         }
/* UNIX */         else if (c == INTERRUPTED)
/* UNIX */         {
/* UNIX */             return INTERRUPTED;
/* UNIX */         }
/* UNIX */         else return i;
/* UNIX */     }
/* UNIX */
/* UNIX */    int overwrite_term_get_n_chars (const char* prompt, char* line, size_t max_len)
/* UNIX */    {
/* UNIX */        IMPORT int restart_io_after_sig[ SIGNAL_ARRAY_SIZE ];
/* UNIX */        IMPORT volatile int recorded_signal;
/* UNIX */        IMPORT volatile Bool pause_on_next;
/* UNIX */        IMPORT volatile int num_term_lines, num_term_chars;
/* UNIX */        IMPORT volatile int term_io_since_last_input_attempt;
/* UNIX */        int done;
/* UNIX */        int res;
/* UNIX */
/* UNIX */
/* UNIX */        term_io_since_last_input_attempt = FALSE;
/* UNIX */
/* UNIX */        NRE(open_tty_out());
/* UNIX */        NRE(open_tty_in());
/* UNIX */
/* UNIX */        if (max_len == 0) return 0;
/* UNIX */
/* UNIX */        ERE(term_set_raw_mode_with_no_echo());
/* UNIX */
/* UNIX */        done = FALSE;
/* UNIX */
/* UNIX */        while ( ! done)
/* UNIX */        {
/* UNIX */            recorded_signal = NOT_SET;
/* UNIX */
/* UNIX */            res = overwrite_term_get_n_chars_guts(prompt, line,
/* UNIX */                                                  max_len);
/* UNIX */
/* UNIX */            if (res == INTERRUPTED)
/* UNIX */            {
/* UNIX */                if ((recorded_signal > 0) &&
/* UNIX */                    (recorded_signal <= SIGNAL_ARRAY_SIZE) &&
/* UNIX */                    (restart_io_after_sig[recorded_signal]))
/* UNIX */                {
/* UNIX */                    done = FALSE;
/* UNIX */                }
/* UNIX */                else
/* UNIX */                {
/* UNIX */                    done = TRUE;
/* UNIX */                }
/* UNIX */            }
/* UNIX */            else
/* UNIX */            {
/* UNIX */                done = TRUE;
/* UNIX */            }
/* UNIX */
/* UNIX */            num_term_lines = 0;
/* UNIX */            num_term_chars = 0;
/* UNIX */            pause_on_next = FALSE;
/* UNIX */
/* UNIX */            term_blank_out_line();
/* UNIX */        }
/* UNIX */
/* UNIX */        term_reset();
/* UNIX */
/* UNIX */        return res;
/* UNIX */    }
/* UNIX */
/* UNIX */     /* Assuming max_len > 0, and fs_tty_in and fs_tty_out are OK !!! */
/* UNIX */     /* Also assuming raw mode with no echo.                    */
/* UNIX */
/* UNIX */     static int overwrite_term_get_n_chars_guts(const char* prompt, char* line, size_t max_len)
/* UNIX */     {
/* UNIX */         char*        line_pos;
/* UNIX */         size_t       i;
/* UNIX */         int          c;
/* UNIX */         int          forward_line;
/* UNIX */
/* UNIX */
/* UNIX */         max_len--;
/* UNIX */
/* UNIX */         ERE(put_prompt(prompt));
/* UNIX */
/* UNIX */         line_pos = line;
/* UNIX */
/* UNIX */
/* UNIX */         i=0;
/* UNIX */         forward_line = FALSE;
/* UNIX */         c = '\0';
/* UNIX */
/* UNIX */         while ( ( !forward_line ) && (i<max_len ) )
/* UNIX */         {
/* UNIX */              c = term_getc();
/* UNIX */
/* UNIX */              if (c == '') c = EOF;
/* UNIX */
/* UNIX */              if ((c == EOF) || (c == INTERRUPTED) || (c == '\n'))
/* UNIX */              {
/* UNIX */                  forward_line = TRUE;
/* UNIX */              }
/* UNIX */              else
/* UNIX */              {
/* UNIX */                  fputc(c, fs_tty_out);  /* Echo. */
/* UNIX */                  *line_pos = c;
/* UNIX */                  line_pos++;
/* UNIX */                  i++;
/* UNIX */              }
/* UNIX */          }
/* UNIX */
/* UNIX */         *line_pos = '\0';
/* UNIX */
/* UNIX */         if (c == EOF)
/* UNIX */         {
/* UNIX */             rewind(fs_tty_in);
/* UNIX */         }
/* UNIX */         else if (c == INTERRUPTED)
/* UNIX */         {
/* UNIX */             *line = '\0';
/* UNIX */         }
/* UNIX */
/* UNIX */         if (c == EOF)
/* UNIX */         {
/* UNIX */             return EOF;
/* UNIX */         }
/* UNIX */         else if (c == INTERRUPTED)
/* UNIX */         {
/* UNIX */             return INTERRUPTED;
/* UNIX */         }
/* UNIX */         else return i;
/* UNIX */     }
/* UNIX */
/* UNIX */    int overwrite_term_get_char(void)
/* UNIX */    {
/* UNIX */        IMPORT int restart_io_after_sig[ SIGNAL_ARRAY_SIZE ];
/* UNIX */        IMPORT volatile int recorded_signal;
/* UNIX */        IMPORT volatile Bool pause_on_next;
/* UNIX */        IMPORT volatile int num_term_lines, num_term_chars;
/* UNIX */        IMPORT volatile int term_io_since_last_input_attempt;
/* UNIX */        int res, done;
/* UNIX */
/* UNIX */
/* UNIX */        term_io_since_last_input_attempt = FALSE;
/* UNIX */
/* UNIX */        NRE(open_tty_out());
/* UNIX */
/* UNIX */        /* Sets fs_tty_in */
/* UNIX */        ERE(term_set_raw_mode_with_no_echo());
/* UNIX */
/* UNIX */        done = FALSE;
/* UNIX */
/* UNIX */        while ( ! done)
/* UNIX */        {
/* UNIX */            recorded_signal = NOT_SET;
/* UNIX */
/* UNIX */
/* UNIX */            res = overwrite_term_get_char_guts();
/* UNIX */
/* UNIX */
/* UNIX */            if (res == INTERRUPTED)
/* UNIX */            {
/* UNIX */                if ((recorded_signal > 0) &&
/* UNIX */                    (recorded_signal < SIGNAL_ARRAY_SIZE) &&
/* UNIX */                    (restart_io_after_sig[recorded_signal]))
/* UNIX */                {
/* UNIX */                    done = FALSE;
/* UNIX */                }
/* UNIX */                else
/* UNIX */                {
/* UNIX */                    done = TRUE;
/* UNIX */                }
/* UNIX */
/* UNIX */            }
/* UNIX */            else
/* UNIX */            {
/* UNIX */                done = TRUE;
/* UNIX */            }
/* UNIX */
/* UNIX */            num_term_lines = 0;
/* UNIX */            num_term_chars = 0;
/* UNIX */            pause_on_next = FALSE;
/* UNIX */        }
/* UNIX */
/* UNIX */        term_reset();
/* UNIX */
/* UNIX */        return res;
/* UNIX */    }
/* UNIX */
/* UNIX */     /* Assuming fs_tty_in and fs_tty_out are OK !!!                  */
/* UNIX */     /* Also assuming raw mode with no echo.                    */
/* UNIX */
/* UNIX */     static int overwrite_term_get_char_guts(void)
/* UNIX */     {
/* UNIX */         int c;
/* UNIX */
/* UNIX */
/* UNIX */         if (fs_smart_terminal_capability)
/* UNIX */         {
/* UNIX */              c = enhanced_term_getc_guts();
/* UNIX */          }
/* UNIX */         else
/* UNIX */         {
/* UNIX */              c = term_getc();
/* UNIX */          }
/* UNIX */
/* UNIX */         if (c == EOF)
/* UNIX */         {
/* UNIX */             rewind(fs_tty_in);
/* UNIX */         }
/* UNIX */
/* UNIX */         term_blank_out_line();
/* UNIX */
/* UNIX */         return c;
/* UNIX */     }
/* UNIX */
#else
#ifdef MS_OS
/* MS_OS */ int term_get_n_chars (const char* prompt, char* line, size_t max_len)
/* MS_OS */ {
/* MS_OS */     IMPORT volatile Bool pause_on_next;
/* MS_OS */     IMPORT volatile int num_term_lines;
/* MS_OS */     IMPORT volatile int num_term_chars;
/* MS_OS */     IMPORT volatile int term_io_since_last_input_attempt;
/* MS_OS */     int c;
/* MS_OS */     size_t i;
/* MS_OS */
/* MS_OS */
/* MS_OS */     term_io_since_last_input_attempt = FALSE;
/* MS_OS */
/* MS_OS */     if (max_len == 0) return 0;
/* MS_OS */
/* MS_OS */     max_len--;
/* MS_OS */
/* MS_OS */     if (max_len == 0)
/* MS_OS */     {
/* MS_OS */         *line = '\0';
/* MS_OS */         return 0;
/* MS_OS */     }
/* MS_OS */
/* MS_OS */     ERE(put_prompt(prompt));
/* MS_OS */
/* MS_OS */     i=0;
/* MS_OS */
/* MS_OS */     while (    (i < max_len) && ((c=getche()) != EOF)
/* MS_OS */             && (c != '\r')
/* MS_OS */           )
/* MS_OS */     {
/* MS_OS */          *line = c;
/* MS_OS */          line++;
/* MS_OS */          i++;
/* MS_OS */      }
/* MS_OS */
/* MS_OS */     *line ='\0';
/* MS_OS */
/* MS_OS */     term_io_since_last_input_attempt = TRUE;
/* MS_OS */
/* MS_OS */     /* As getche bypasses the C library's handling of      */
/* MS_OS */     /* streams, and on PC's, an enter becomes a return and */
/* MS_OS */     /* a line feed, echoing the return is not enough to    */
/* MS_OS */     /* force a new line.                                   */
/* MS_OS */
/* MS_OS */     if (c != '\r')
/* MS_OS */     {
/* MS_OS */         putch('\r');
/* MS_OS */     }
/* MS_OS */
/* MS_OS */     putch('\n');
/* MS_OS */
/* MS_OS */     num_term_lines = 0;
/* MS_OS */     num_term_chars = 0;
/* MS_OS */     pause_on_next = FALSE;
/* MS_OS */
/* MS_OS */     if  (c == EOF)
/* MS_OS */     {
/* MS_OS */         return EOF;
/* MS_OS */     }
/* MS_OS */     else
/* MS_OS */     {
/* MS_OS */         return i;
/* MS_OS */     }
/* MS_OS */ }
/* MS_OS */
/* MS_OS */ int overwrite_term_get_n_chars(const char* prompt, char* line, size_t max_len)
/* MS_OS */ {
/* MS_OS */     IMPORT volatile int term_io_since_last_input_attempt;
/* MS_OS */     int res;
/* MS_OS */
/* MS_OS */
/* MS_OS */     term_io_since_last_input_attempt = FALSE;
/* MS_OS */
/* MS_OS */     NRE(open_tty_out());
/* MS_OS */
/* MS_OS */     ERE(res = term_get_n_chars(prompt, line, max_len));
/* MS_OS */
/* MS_OS */     term_blank_out_line();
/* MS_OS */
/* MS_OS */     return res;
/* MS_OS */ }
/* MS_OS */
/* MS_OS */
/* MS_OS */ int overwrite_term_get_char(void)
/* MS_OS */ {
/* MS_OS */     IMPORT volatile int pause_on_next;
/* MS_OS */     IMPORT volatile int num_term_lines;
/* MS_OS */     IMPORT volatile int num_term_chars;
/* MS_OS */     IMPORT volatile int term_io_since_last_input_attempt;
/* MS_OS */     int res;
/* MS_OS */
/* MS_OS */
/* MS_OS */     term_io_since_last_input_attempt = FALSE;
/* MS_OS */
/* MS_OS */     NRE(open_tty_out());
/* MS_OS */
/* MS_OS */     ERE(res = enhanced_term_getc());
/* MS_OS */
/* MS_OS */     num_term_lines = 0;
/* MS_OS */     num_term_chars = 0;
/* MS_OS */     pause_on_next = FALSE;
/* MS_OS */
/* MS_OS */     term_blank_out_line();
/* MS_OS */
/* MS_OS */     return res;
/* MS_OS */ }
/* MS_OS */
#else
/* default */  int term_get_n_chars (const char* prompt, char* line, size_t max_len)
/* default */  {
/* default */      IMPORT volatile Bool pause_on_next;
/* default */      IMPORT volatile int num_term_lines, num_term_chars;
/* default */      IMPORT volatile int term_io_since_last_input_attempt;
/* default */      int res;


                   /* Never reached? */
                   
                   UNTESTED_CODE();


/* default */
/* default */
/* default */      term_io_since_last_input_attempt = FALSE;
/* default */
/* default */      if (fs_tty_in == NULL)
/* default */      {
/* default */          NRE(open_tty_in());
/* default */      }
/* default */
/* default */      ERE(put_prompt(prompt));
/* default */
/* default */      ERE(res = fget_line(fs_tty_in, line, max_len));
/* default */
/* default */      num_term_lines = 0;
/* default */      num_term_chars = 0;
/* default */      pause_on_next = FALSE;
/* default */      term_io_since_last_input_attempt = TRUE;
/* default */
/* default */      return res;
/* default */  }
/* default */
/* default */   int overwrite_term_get_n_chars(const char* prompt, char* line, size_t max_len)
/* default */   {
/* default */       IMPORT volatile int term_io_since_last_input_attempt;
/* default */       int res;
/* default */
/* default */
/* default */       term_io_since_last_input_attempt = FALSE;
/* default */
/* default */       NRE(open_tty_out());
/* default */
/* default */       ERE(res = term_get_n_chars( prompt, line, max_len));
/* default */
/* default */       term_blank_out_line();
/* default */
/* default */       return res;
/* default */   }
/* default */
/* default */   int overwrite_term_get_char(void)
/* default */   {
/* default */       IMPORT volatile Bool pause_on_next;
/* default */       IMPORT volatile int num_term_lines, num_term_chars;
/* default */       IMPORT volatile int term_io_since_last_input_attempt;
/* default */       int res;
/* default */
/* default */
/* default */       term_io_since_last_input_attempt = FALSE;
/* default */
/* default */       NRE(open_tty_out());
/* default */
/* default */       ERE(res = enhanced_term_getc());
/* default */
/* default */       num_term_lines = 0;
/* default */       num_term_chars = 0;
/* default */       pause_on_next = FALSE;
/* default */
/* default */       term_blank_out_line();
/* default */
/* default */       return res;
/* default */   }
/* default */
#endif
#endif

/*
 * =============================================================================
 *                              move_cursor_up
 *
 *
 *
 * Index: terminal I/O
 *
 * -----------------------------------------------------------------------------
 */

#ifdef UNIX
/* UNIX */     int move_cursor_up (int count)
/* UNIX */     {
/* UNIX */         IMPORT volatile int term_io_since_last_input_attempt;
/* UNIX */         int i;
/* UNIX */
/* UNIX */
/* UNIX */         NRE(open_tty_out());
/* UNIX */
/* UNIX */         if (! fs_smart_terminal_capability) return ERROR;
/* UNIX */
/* UNIX */         fputs("\r", fs_tty_out);
/* UNIX */
/* UNIX */         for (i=0; i<count; i++)
/* UNIX */         {
/* UNIX */             TPUTS(fs_output_up_arrow_sequence);
/* UNIX */         }
/* UNIX */
/* UNIX */         term_io_since_last_input_attempt = TRUE;
/* UNIX */
/* UNIX */         return NO_ERROR;
/* UNIX */     }
/* UNIX */
/* UNIX */     int move_cursor_down (int count)
/* UNIX */
/* UNIX */     {
/* UNIX */         IMPORT volatile int term_io_since_last_input_attempt;
/* UNIX */         int i;
/* UNIX */
/* UNIX */
/* UNIX */         NRE(open_tty_out());
/* UNIX */
/* UNIX */         if (! fs_smart_terminal_capability) return ERROR;
/* UNIX */
/* UNIX */         fputs("\r", fs_tty_out);
/* UNIX */
/* UNIX */         for (i=0; i<count; i++)
/* UNIX */         {
/* UNIX */             TPUTS(fs_output_down_arrow_sequence);
/* UNIX */         }
/* UNIX */
/* UNIX */         term_io_since_last_input_attempt = TRUE;
/* UNIX */
/* UNIX */         return NO_ERROR;
/* UNIX */     }
/* UNIX */
#else
#ifdef MS_OS
/* MS_OS */  int move_cursor_up(int count)
/* MS_OS */  {
/* MS_OS */      IMPORT volatile int term_io_since_last_input_attempt;
/* MS_OS */      int i;
/* MS_OS */
/* MS_OS */
/* MS_OS */      NRE(open_tty_out());
/* MS_OS */
/* MS_OS */      if (! fs_smart_terminal_capability) return ERROR;
/* MS_OS */
/* MS_OS */      term_io_since_last_input_attempt = TRUE;
/* MS_OS */
/* MS_OS */      fputs("\r", fs_tty_out);
/* MS_OS */
/* MS_OS */        for (i=0; i<count; i++)
/* MS_OS */        {
/* MS_OS */            fputs(fs_output_up_arrow_sequence, fs_tty_out);
/* MS_OS */        }
/* MS_OS */
/* MS_OS */      return NO_ERROR;
/* MS_OS */  }
/* MS_OS */
/* MS_OS */  int move_cursor_down (count)
/* MS_OS */      int count;
/* MS_OS */  {
/* MS_OS */      IMPORT volatile int term_io_since_last_input_attempt;
/* MS_OS */      int i;
/* MS_OS */
/* MS_OS */
/* MS_OS */      NRE(open_tty_out());
/* MS_OS */
/* MS_OS */      if (! fs_smart_terminal_capability) return ERROR;
/* MS_OS */
/* MS_OS */      term_io_since_last_input_attempt = TRUE;
/* MS_OS */
/* MS_OS */      fputs("\r", fs_tty_out);
/* MS_OS */
/* MS_OS */      for (i=0; i<count; i++)
/* MS_OS */      {
/* MS_OS */          fputs(fs_output_down_arrow_sequence, fs_tty_out);
/* MS_OS */      }
/* MS_OS */
/* MS_OS */      return NO_ERROR;
/* MS_OS */  }
/* MS_OS */
#else
/* default */  int move_cursor_up(int count)
/* default */  {
/* default */      IMPORT volatile int term_io_since_last_input_attempt;
/* default */      int i;
/* default */
/* default */
/* default */      NRE(open_tty_out());
/* default */
/* default */      if (! fs_smart_terminal_capability) return ERROR;
/* default */
/* default */      fputs("\r", fs_tty_out);
/* default */
/* default */      term_io_since_last_input_attempt = TRUE;
/* default */
/* default */      for (i=0; i<count; i++)
/* default */      {
/* default */          fputs(fs_output_up_arrow_sequence, fs_tty_out);
/* default */      }
/* default */
/* default */      return NO_ERROR;
/* default */  }
/* default */
/* default */  int move_cursor_down (count)
/* default */    int count;
/* default */  {
/* default */      IMPORT volatile int term_io_since_last_input_attempt;
/* default */      int i;
/* default */
/* default */
/* default */      NRE(open_tty_out());
/* default */
/* default */      if (! fs_smart_terminal_capability) return ERROR;
/* default */
/* default */      fputs("\r", fs_tty_out);
/* default */
/* default */      term_io_since_last_input_attempt = TRUE;
/* default */
/* default */      for (i=0; i<count; i++)
/* default */      {
/* default */          fputs(fs_output_down_arrow_sequence, fs_tty_out);
/* default */      }
/* default */
/* default */      return NO_ERROR;
/* default */  }
/* default */
#endif
#endif

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*
 * =============================================================================
 *                                  term_beep
 *
 * Makes the terminal beep
 *
 * Ths routine makes the terminal beep once. See term_beep_beep() for more
 * flexibility.
 *
 * Related:
 *     term_beep_beep
 *
 * Index: I/O, input, terminal I/O
 *
 * -----------------------------------------------------------------------------
 */

void term_beep(void)
{

    NR(open_tty_out());

    fputc(0x07, fs_tty_out);
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*
 * =============================================================================
 *                                  term_beep_beep
 *
 * Makes the terminal beep
 *
 * Ths routine makes the terminal beep "num_beeps" times, with a pause of
 * "pause_in_ms" milli-seconds between each one.
 *
 * Related:
 *     term_beep_beep
 *
 * Index: I/O, input, terminal I/O
 *
 * -----------------------------------------------------------------------------
 */

void term_beep_beep(int num_beeps, int pause_in_ms)
{
    int i;

    if (num_beeps > 0)
    {
        term_beep();
        num_beeps--;
    }

    for (i=0; i<num_beeps; i++)
    {
        nap(pause_in_ms);
        term_beep();
    }
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*
 * =============================================================================
 *                               term_get_line
 *
 * Reads a line from the terminal.
 *
 * This routine reads a line from the terminal, providing a number of useful
 * features.   These include line re-entry, intelligent behaviour under
 * non-blocking reads, and intelligent behaviour on CTL-Z. If these features
 * are not desired, then there is no point in using this routine.
 *
 * If the higher level code has not called term_set_raw_mode_with_no_echo()
 * before the first call to term_get_line(), then term_get_line makes the call.
 * This sets the terminal into raw mode and takes over the echoing of
 * characters. This means that an inadvertant exit under some systems/shells
 * (including sh/csh) can leave the user without character echo. The mode can
 * be reset using term_reset(). However, this is automaticlally done in
 * kjb_cleanup(), and it is recommended that kjb_cleanup() is called instead.
 * kjb_cleanup() is called by kjb_exit(), as well as any abnmormal termination
 * trapped by the KJB library.  If the call to term_set_raw_mode_with_no_echo()
 * is in place before the call to term_get_line(), then the call is not made.
 *
 * The simplest way to handle some of these problems is to use the macro
 * KJB_INIT() before using this routine. KJB_INIT sets things up so that most
 * points of exit are covered by the appropriate cleanup.
 *
 * Any signals that are going to be used should be put in place BEFORE the
 * (perhaps implicit) call to term_set_raw_mode_with_no_echo(). This is because
 * term_set_raw_mode_with_no_echo() intercepts the signals. This comment does
 * not apply to signals which are set and unset without any intervening call to
 * term_get_line() provided that they reinstate the previous handler
 * (kjb_set_atn_trap and kjb_unset_atn_trap can be used to do this in the case
 * of SIGINT -- ideally such a facility should be available for signals in
 * general, but .... )
 *
 * If non-blocking reads are used, thes routine assumes that prompt and max_len
 * are not changed between unsuccessful reads. Once the user has entered
 * a line, then a "new" read begins, and it is OK to change the prompt and
 * the buffer.
 *
 * Returns:
 *    On success term_get_line returns the number of bytes placed in the
 *    buffer.  Since the new line chracter is consumed but not copied, the
 *    return is zero for null lines. Alternate return values are all negative.
 *    In the case of end of file, EOF returned (the user entered ^D).  If the
 *    line was truncated, then LINE_TRUNCATED is returned. Depending on the
 *    signal traps and options in place, INTERRUPTED is an additional possible
 *    return value. In the case of non-blocking reads WOULD_BLOCK is returned
 *    unless a complete line can be returned. ERROR is returned for unexpected
 *    problems and an error message is set.
 *
 * Index: I/O, input, terminal I/O
 *
 * -----------------------------------------------------------------------------
 */

/*
// FIX
//
// This routine and helpers are a hack job with respect to using signed and
// unsigned (size_t). Fixed up with casts and ASSERTS.
*/
int term_get_line(const char* prompt, char* line, size_t max_size)
{
    IMPORT volatile Bool pause_on_next;
    IMPORT volatile int num_term_lines;
    IMPORT volatile int num_term_chars;
    int                res;
    static int log_tty_input = NOT_SET;
    static FILE* tty_input_log_fp = NULL;

    NRE(open_tty_in());
    NRE(open_tty_out());

    if (log_tty_input == NOT_SET)
    {        
        char tty_input_log_name[ MAX_FILE_NAME_SIZE ]; 

        if (BUFF_GET_ENV("LOG_TTY_INPUT", tty_input_log_name) == NO_ERROR)
        {
            if (tty_input_log_name[ 0 ] == '\0') 
            {
                BUFF_CPY(tty_input_log_name, "tty_input.log");
            }
            NRE(tty_input_log_fp = kjb_fopen(tty_input_log_name, "w"));
            log_tty_input = TRUE;
        }
        else { log_tty_input = FALSE; }
    }

    /*
    // Allow an argument of zero max_size to force an open of the tty's, but
    // do nothing else.
    */
    if (max_size == 0)
    {
        return 0;
    }
    else if (max_size == 1)
    {
        *line = '\0';
        return 0;
    }


    if ((fs_use_smart_terminal) && (fs_smart_terminal_capability))
    {
        if (fs_current_terminal_mode != RAW_WITH_NO_ECHO_TERMINAL_MODE)
        {
            static int first_time  = TRUE;

            if (first_time)
            {
                first_time = FALSE;
                ERE(term_set_raw_mode_with_no_echo());
                add_cleanup_function(term_reset);
            }
            else
            {
                set_bug(
                     "Terminal mode incorrect on second call to term_get_line");
                return ERROR;
            }
        }

        res = enhanced_term_get_line(prompt, line, max_size);
    }
    else
    {
        res = dumb_term_get_line(prompt, line, max_size);
    }

    if (res != ERROR)
    {
        num_term_lines = 0;
        num_term_chars = 0;
        pause_on_next = FALSE;

        if (tty_input_log_fp != NULL) 
        {
            ERE(fput_line(tty_input_log_fp, line));

        }
    }

    return res;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*
 * =============================================================================
 * STATIC                      dumb_term_get_line
 * -----------------------------------------------------------------------------
 */

#ifdef UNIX
/* UNIX */    static int dumb_term_get_line(const char* prompt, char* line, size_t max_size)
/* UNIX */    {
/* UNIX */        IMPORT int restart_io_after_sig[ SIGNAL_ARRAY_SIZE ];
/* UNIX */        IMPORT volatile int recorded_signal;
/* UNIX */        IMPORT volatile int term_io_since_last_input_attempt;
/* UNIX */        IMPORT volatile int num_term_lines;
/* UNIX */        IMPORT volatile int num_term_chars;
/* UNIX */        IMPORT volatile Bool pause_on_next;
/* UNIX */        int done;
/* UNIX */        int res;
/* UNIX */
/* UNIX */
/* UNIX */        term_io_since_last_input_attempt = FALSE;
/* UNIX */

                  /*
                  // We are only here if either we don't want to use, or can't
                  // use smart terminal. In this case, the module should enforce
                  // original terminal settings.
                  */
#if 0 /* was ifdef OBSOLETE */
/* UNIX */        ERE(term_set_cooked_mode());
#endif

/* UNIX */
/* UNIX */        done = FALSE;
/* UNIX */
/* UNIX */        while (! done)
/* UNIX */        {
/* UNIX */            recorded_signal = NOT_SET;
/* UNIX */
/* UNIX */            ERE(put_prompt(prompt));
/* UNIX */
/* UNIX */            res = fget_line(fs_tty_in, line, max_size);
/* UNIX */
/* UNIX */            if (res == INTERRUPTED)
/* UNIX */            {
/* UNIX */                if ((recorded_signal > 0) &&
/* UNIX */                    (recorded_signal < SIGNAL_ARRAY_SIZE) &&
/* UNIX */                    (restart_io_after_sig[recorded_signal]))
/* UNIX */                {
/* UNIX */                    done = FALSE;
/* UNIX */                }
/* UNIX */                else
/* UNIX */                {
/* UNIX */                    done = TRUE;
/* UNIX */                }
/* UNIX */
/* UNIX */            }
/* UNIX */            else
/* UNIX */            {
/* UNIX */                done = TRUE;
/* UNIX */            }
/* UNIX */
/* UNIX */            num_term_lines = 0;
/* UNIX */            num_term_chars = 0;
/* UNIX */            pause_on_next = FALSE;
/* UNIX */        }
/* UNIX */
/* UNIX */        term_reset();
/* UNIX */
/* UNIX */        term_io_since_last_input_attempt = TRUE;
/* UNIX */
/* UNIX */        return res;
/* UNIX */    }
#else
/* default */ static int dumb_term_get_line(const char* prompt, char* line, size_t max_size)
/* default */ {
/* default */     IMPORT volatile Bool pause_on_next;
/* default */     IMPORT volatile int num_term_lines, num_term_chars;
/* default */     IMPORT volatile int term_io_since_last_input_attempt;
/* default */     int res;
/* default */
/* default */
/* default */     term_io_since_last_input_attempt = FALSE;
/* default */
/* default */     if (fs_tty_in == NULL)
/* default */     {
/* default */         NRE(open_tty_in());
/* default */     }
/* default */
/* default */     ERE(put_prompt(prompt));
/* default */
/* default */     ERE(res = fget_line(fs_tty_in, line, max_size));
/* default */
/* default */     term_io_since_last_input_attempt = TRUE;
/* default */     num_term_lines = 0;
/* default */     num_term_chars = 0;
/* default */     pause_on_next = FALSE;
/* default */
/* default */     return res;
/* default */ }
#endif

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*
 * =============================================================================
 * STATIC                       enhanced_term_get_line
 * -----------------------------------------------------------------------------
 */

static int enhanced_term_get_line(const char* prompt, char* line,
                                  size_t max_size)
{
#ifdef UNIX
    IMPORT int restart_io_after_sig[ SIGNAL_ARRAY_SIZE ];
    IMPORT volatile int recorded_signal;
    IMPORT volatile Bool pause_on_next;
    IMPORT volatile int num_term_lines, num_term_chars;
#endif
    int done;
    int res;


#if 0 /* was ifdef TRY_WITHOUT */
    /*
    // Keep this line. This is how we used to handle it, and how one probably
    // should handle it if one was not interested in supporting non-blocking
    // reads.
    */
    ERE(term_set_raw_mode_with_no_echo());
#endif

    done = FALSE;

    while ( ! done)
    {
        recorded_signal = NOT_SET;

        res = enhanced_term_get_line_guts(prompt, line, max_size);

        if (res == INTERRUPTED)
        {
            if ((recorded_signal > 0) &&
                (recorded_signal < SIGNAL_ARRAY_SIZE) &&
                (restart_io_after_sig[recorded_signal]))
            {
                done = FALSE;
            }
            else
            {
                done = TRUE;
            }
        }
        else
        {
            done = TRUE;
        }

        num_term_lines = 0;
        num_term_chars = 0;
        pause_on_next = FALSE;
    }

#if 0 /* was ifdef TRY_WITHOUT */
    /*
    // Keep this line. This is how we used to handle it, and how one probably
    // should handle it if one was not interested in supporting non-blocking
    // reads (and can live with the extra overhead and chance of missing
    // chars -- possible not such a good idea after all).
    */
    term_reset();
#endif

    return res;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*
 * =============================================================================
 * STATIC                      enhanced_term_get_line_guts
 * -----------------------------------------------------------------------------
 */

/* Assuming max_size_in > 0, and fs_tty_in and fs_tty_out are OK !!! */
/* Also assuming raw mode with no echo.                    */

static int enhanced_term_get_line_guts(const char* prompt, char* line,
                                       size_t max_size_in)
{
    IMPORT volatile int kjb_tty_cols;
    IMPORT volatile int term_io_since_last_input_attempt;
    static char         buff[ GET_LINE_BUFF_SIZE ];
    static int          buff_pos;
    static int          buff_size      = NOT_SET;
    static int          insert_mode;
    static int          prompt_size;
    static int          cur_buff_count;
    static int          max_size;
    int                 c, i;
    int                 cursor_pos;
    int                 move_amount;
    int                 forward_line;
    const char*         new_buff;
    int                 save_buff_count;
    int                 save_buff_size;
    int                result;


    if (fs_term_buff_count == 0)
    {
        /* Give the user something to back up to. */
        add_line_to_reentry_queue("");
    }

    forward_line = FALSE;
    c = '\0';

    /*
    // The following is true if this is a new read.
    */

    if (buff_size == NOT_SET)
    {
        prompt_size=strlen(prompt);

        if (prompt_size >= kjb_tty_cols)
        {
            set_error("Terminal width is too small.");
            return ERROR;
        }

        ASSERT(sizeof(buff) > (size_t)prompt_size + ROOM_FOR_NULL);
        ASSERT(max_size_in > ROOM_FOR_NULL);

        max_size = MIN_OF(max_size_in - ROOM_FOR_NULL,
                          sizeof(buff) - prompt_size - ROOM_FOR_NULL);

        *line = '\0';

        BUFF_CPY(buff, prompt);

        buff_size = prompt_size;
        buff_pos = prompt_size;
        insert_mode = TRUE;
        cur_buff_count = fs_term_buff_count + 1;
        term_io_since_last_input_attempt = FALSE;

        /*
        // Despite appearances this does not get duplicated a few lines down
        // because in this call we have term_io_since_last_input_attempt==FALSE
        */
        if (buff_size > 0)
        {
            fputs(buff, fs_tty_out);
        }
    }
    else if (term_io_since_last_input_attempt)
    {
        term_blank_out_line();

        if (buff_size > 0)
        {
            fputs(buff, fs_tty_out);
        }
    }

    while ( ( !forward_line ) && (buff_size < (max_size + prompt_size))  )
    {
        c = enhanced_term_getc_guts();

#ifdef NOT_NEEDED
        if (c == '')
        {
            c = EOF;
        }
#endif

        if (c == EOF)
        {
            /* Echo a phony carriage return. */
            fputc('\n', fs_tty_out);
            forward_line = TRUE;
        }
        else if ((c == INTERRUPTED) || (c == WOULD_BLOCK) || (c == ERROR))
        {
            forward_line = TRUE;
        }
        else if (c == INVALID_CONTROL_SEQUENCE)
        {
            term_beep();
        }
        else if ((c == '\n') || (c == '\r'))
        {
            forward_line = TRUE;
            buff[buff_size] = '\0';
            fputc(c, fs_tty_out);
        }
        else if ((c == UP_ARROW) || (c == DOWN_ARROW))
        {
            save_buff_count = cur_buff_count;

            if (c == UP_ARROW) cur_buff_count--;
            else cur_buff_count++;

            new_buff = term_get_buff(cur_buff_count);

            if (new_buff == NULL)
            {
                term_beep();
                cur_buff_count = save_buff_count;
            }
            else
            {
                save_buff_size = buff_size;
                cursor_pos=buff_pos;

                while (cursor_pos/kjb_tty_cols > 0)
                {
                    TPUTS(fs_output_up_arrow_sequence);
                    cursor_pos -= kjb_tty_cols;
                }
                fputc('\r', fs_tty_out);

                strcpy(buff, prompt);
                strcat(buff, new_buff);

                buff_size = strlen(buff);
                buff_pos = buff_size;

                enhanced_term_put_buff(0, buff, buff_size);

                cursor_pos = buff_pos;

                while (cursor_pos < save_buff_size)
                {
                    fputc(' ', fs_tty_out);
                    cursor_pos++;

                    if ( (cursor_pos % kjb_tty_cols) == 0)
                    {
                        fputc('\n', fs_tty_out);
                    }
                }

                while (cursor_pos/kjb_tty_cols>buff_pos/kjb_tty_cols)
                {
                    TPUTS(fs_output_up_arrow_sequence);
                    cursor_pos -= kjb_tty_cols;
                }

                while (cursor_pos>buff_pos)
                {
                    TPUTS(fs_output_left_arrow_sequence);
                    cursor_pos--;
                }

                while (cursor_pos<buff_pos)
                {
                    TPUTS(fs_output_right_arrow_sequence);
                    cursor_pos++;
                }
            }
        }
        else if (c == LEFT_ARROW)
        {
            if (buff_pos>prompt_size)
            {
                if (buff_pos % kjb_tty_cols == 0)
                {
                    TPUTS(fs_output_up_arrow_sequence);

                    for (i=1;i<kjb_tty_cols;i++)
                    {
                        TPUTS(fs_output_right_arrow_sequence);
                    }
                }
                else
                {
                    TPUTS(fs_output_left_arrow_sequence);
                }
                buff_pos--;
            }
            else
            {
                /* Can't assume that a downward motion will or */
                /* will not leave the cursor at left margin.   */
                /* Hence, set it there.                        */

                if ( (buff_size/kjb_tty_cols) > (buff_pos/kjb_tty_cols) )
                {
                    for (i=0; i<prompt_size; i++)
                    {
                        TPUTS(fs_output_left_arrow_sequence);
                        buff_pos--;
                    }
                }

                while ( (buff_size/kjb_tty_cols)>(buff_pos/kjb_tty_cols) )
                {
                    TPUTS(fs_output_down_arrow_sequence);
                    buff_pos += kjb_tty_cols;
                }

                while (buff_pos < buff_size)
                {
                    TPUTS(fs_output_right_arrow_sequence);
                    buff_pos++;
                }

                while (buff_pos > buff_size)
                {
                    TPUTS(fs_output_left_arrow_sequence);
                    buff_pos--;
                }
            }
        }
        else if (c == RIGHT_ARROW)
        {
            if (buff_pos < buff_size)
            {
                if ((buff_pos+1) % kjb_tty_cols == 0)
                {
                    TPUTS(fs_output_down_arrow_sequence);
                    fputc('\r', fs_tty_out);
                }
                else
                {
                    TPUTS(fs_output_right_arrow_sequence);
                }
                buff_pos++;
            }
            else if (buff_pos == buff_size)
            {
                /* Can't assume that a upward motion either will or */
                /* will not leave the cursor at left margin.        */
                /* Hence, set it there.                             */

                move_amount = buff_pos % kjb_tty_cols;

                for (i=0; i<move_amount; i++)
                {
                    TPUTS(fs_output_left_arrow_sequence);
                }

                buff_pos -= move_amount;

                while (buff_pos >= kjb_tty_cols)
                {
                    TPUTS(fs_output_up_arrow_sequence);
                    buff_pos -= kjb_tty_cols;
                }

                while (buff_pos < prompt_size)
                {
                    TPUTS(fs_output_right_arrow_sequence);
                    buff_pos++;
                }
            }
        }
        else if (c == '')
        {
            if (insert_mode) insert_mode = FALSE;
            else insert_mode = TRUE;
        }
        else if ((c == '') || (c == ''))
        {
            if (buff_pos >= kjb_tty_cols)
            {
                /* Can't assume that a downward motion will or */
                /* will not leave the cursor at left margin.   */
                /* Hence, set it there.                        */

                move_amount = buff_pos % kjb_tty_cols;

                for (i=0; i<move_amount; i++)
                {
                    TPUTS(fs_output_left_arrow_sequence);
                }

                buff_pos -= move_amount;

                while (buff_pos >= kjb_tty_cols)
                {
                    TPUTS(fs_output_up_arrow_sequence);
                    buff_pos -= kjb_tty_cols;
                }

                while (buff_pos < prompt_size)
                {
                    TPUTS(fs_output_right_arrow_sequence);
                    buff_pos++;
                }
            }
            else
            {
                move_amount = buff_pos - prompt_size;

                for (i=0; i<move_amount; i++)
                {
                    TPUTS(fs_output_left_arrow_sequence);
                }

                buff_pos -= move_amount;
            }
        }
        else if (c == '')
        {
            if ( (buff_size/kjb_tty_cols)>(buff_pos/kjb_tty_cols) )
            {
                /* Can't assume that a downward motion will or */
                /* will not leave the cursor at left margin.   */
                /* Hence, set it there.                        */

                move_amount = buff_pos % kjb_tty_cols;

                for (i=0; i<move_amount; i++)
                {
                    TPUTS(fs_output_left_arrow_sequence);
                }

                buff_pos -= move_amount;

                while ( (buff_size/kjb_tty_cols)>(buff_pos/kjb_tty_cols) )
                {
                    TPUTS(fs_output_down_arrow_sequence);
                    buff_pos += kjb_tty_cols;
                }

                move_amount = buff_size % kjb_tty_cols;

                for (i=0; i<move_amount; i++)
                {
                    TPUTS(fs_output_right_arrow_sequence);
                }

                buff_pos += move_amount;
            }
            else
            {
                move_amount = buff_size - buff_pos;

                for (i=0; i<move_amount; i++)
                {
                    TPUTS(fs_output_right_arrow_sequence);
                }

                buff_pos += move_amount;
            }
        }

        else if (c == ERASE_CHAR)
        {
            if (buff_pos <= prompt_size)
            {
                term_beep();
            }
            else
            {
                for (i=buff_pos-1;i<=buff_size;i++)
                {
                    buff[i]=buff[i+1];
                }
                buff_size--;

                if (buff_pos % kjb_tty_cols == 0)
                {
                    TPUTS(fs_output_up_arrow_sequence);

                    for (i=1;i<kjb_tty_cols;i++)
                    {
                        TPUTS(fs_output_right_arrow_sequence);
                    }
                }
                else
                {
                    TPUTS(fs_output_left_arrow_sequence);
                }

                buff_pos--;

                enhanced_term_put_buff(buff_pos, buff+buff_pos,
                                       (int)strlen(buff+buff_pos));

                enhanced_term_put_buff(buff_size, " ", 1);

                cursor_pos = buff_size+1;

                while (cursor_pos/kjb_tty_cols >buff_pos/kjb_tty_cols)
                {
                    TPUTS(fs_output_up_arrow_sequence);
                    cursor_pos -= kjb_tty_cols;
                }
                while (cursor_pos>buff_pos)
                {
                    TPUTS(fs_output_left_arrow_sequence);
                    cursor_pos--;
                }
                while (cursor_pos<buff_pos)
                {
                    TPUTS(fs_output_right_arrow_sequence);
                    cursor_pos++;
                }
            }
        }
        else if (c == '')
        {
            save_buff_size = buff_size;
            i=buff_pos;

            while (buff_pos/kjb_tty_cols >0)
            {
                TPUTS(fs_output_up_arrow_sequence);
                buff_pos -= kjb_tty_cols;
            }
            fputc('\r', fs_tty_out);

            strcpy(buff, prompt);

            buff_size = strlen(buff);

            enhanced_term_put_buff(0, buff, buff_size);
            buff_pos = buff_size;

            enhanced_term_put_blanks(buff_pos, save_buff_size - buff_pos);

            buff_pos = save_buff_size;

            while (buff_pos/kjb_tty_cols > buff_size/kjb_tty_cols)
            {
                TPUTS(fs_output_up_arrow_sequence);
                buff_pos -= kjb_tty_cols;
            }

            while (buff_pos > buff_size)
            {
                TPUTS(fs_output_left_arrow_sequence);
                buff_pos--;
            }

            while (buff_pos < buff_size)
            {
                TPUTS(fs_output_right_arrow_sequence);
                buff_pos++;
            }
        }
        else if (! isprint(c))
        {
            term_beep();
        }
        else
        {
            if (! insert_mode)
            {
                buff[buff_pos] = c;
                fputc(c, fs_tty_out);
                buff_pos++;

                if (buff_pos > buff_size)
                {
                    buff[buff_pos] = '\0';
                    buff_size++;

                    if (buff_size % kjb_tty_cols == 0)
                    {
                        fputc('\n', fs_tty_out);
                        TPUTS(fs_output_left_arrow_sequence);
                    }
                }
            }
            else
            {
                for (i = buff_size+1; i>buff_pos; i--)
                {
                    buff[i] = buff[i-1];
                }

                buff_size++;

                buff[buff_pos] = c;

                enhanced_term_put_buff(buff_pos, buff+buff_pos,
                                       (int)strlen(buff+buff_pos));

                cursor_pos = buff_size;
                buff_pos++;

                while (cursor_pos/kjb_tty_cols >buff_pos/kjb_tty_cols)
                {
                    TPUTS(fs_output_up_arrow_sequence);
                    cursor_pos -= kjb_tty_cols;
                }

                while (cursor_pos>buff_pos)
                {
                    TPUTS(fs_output_left_arrow_sequence);
                    cursor_pos--;
                }

                while (cursor_pos<buff_pos)
                {
                    TPUTS(fs_output_right_arrow_sequence);
                    cursor_pos++;
                }
            }
        }
    }

#ifdef UNIX
    if (c == INTERRUPTED)
    {
        /*
        // Quite likely anyhow.
        */
        term_io_since_last_input_attempt = TRUE;
    }

    if (c == WOULD_BLOCK)
    {
        *line = '\0';
        return c;
    }
#endif

    save_buff_size = buff_size;
    buff_size = NOT_SET;

    if ((c == EOF) || (c == INTERRUPTED) || (c == ERROR))
    {
        *line = '\0';
        return c;
    }
    else
    {
        ASSERT(max_size > 0);

        kjb_strncpy(line, buff + prompt_size, (size_t)(max_size + 1));

        if (buff[ prompt_size ] != '\0')
        {
            add_line_to_reentry_queue(buff+prompt_size);
        }

        ASSERT(save_buff_size >= prompt_size);

        result = MIN_OF(max_size, save_buff_size - prompt_size);

        return result;
    }
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*
 * =============================================================================
 * STATIC                      enhanced_term_put_blanks
 * -----------------------------------------------------------------------------
 */

static void enhanced_term_put_blanks(int cur_pos, int num_blanks)
{
    IMPORT volatile int kjb_tty_cols;
    /*
    // blanks must be at least  40  blanks
    */
    const char* blanks = "                                        ";
    /*                    1234567890123456789012345678901234567890   */



    while (num_blanks > 40)
    {
        enhanced_term_put_buff(cur_pos, blanks, 40);
        cur_pos += 40;
        num_blanks -= 40;

        while (cur_pos > kjb_tty_cols)
        {
            cur_pos -= kjb_tty_cols;
        }

    }

    enhanced_term_put_buff(cur_pos, blanks, num_blanks);
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*
 * =============================================================================
 * STATIC                      enhanced_term_put_buff
 * -----------------------------------------------------------------------------
 */

static void enhanced_term_put_buff(int cur_pos, const char* buff_pos,
                                   int buff_len)
{
    IMPORT volatile int kjb_tty_cols;


    cur_pos %= kjb_tty_cols;

    ASSERT(buff_len >= 0);
    ASSERT(kjb_tty_cols >= 1);

    if (cur_pos + buff_len < kjb_tty_cols)
    {
        fwrite((const void*)buff_pos, sizeof(char), (size_t)buff_len,
               fs_tty_out);
    }
    else
    {
        fwrite((const void*)buff_pos, 1, (size_t)(kjb_tty_cols - cur_pos),
               fs_tty_out);
        buff_len -= (kjb_tty_cols - cur_pos);
        buff_pos += (kjb_tty_cols - cur_pos);

        fputc('\n', fs_tty_out);

        while (buff_len >= kjb_tty_cols)
        {
            fwrite(buff_pos, 1, (size_t)kjb_tty_cols, fs_tty_out);
            fputc('\n', fs_tty_out);
            buff_len -= kjb_tty_cols;
            buff_pos += kjb_tty_cols;
        }

        fwrite((const void*)buff_pos, 1, (size_t) buff_len, fs_tty_out);
    }
 }

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*
 * =============================================================================
 * STATIC                      term_get_buff
 * -----------------------------------------------------------------------------
 */

static const char* term_get_buff(int sub_num)
{
    Queue_element* cur_elem;
    int            i;


    if ((sub_num <= 0) || (sub_num > fs_term_buff_count+1)) return NULL;
    else if (sub_num == fs_term_buff_count+1) return "";

    cur_elem = fs_term_buff_head;

    for (i=1; i<sub_num; ++i)
    {
        cur_elem = cur_elem->next;
    }

    return (char *)(cur_elem->contents);
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*
 * =============================================================================
 *                           enhanced_term_getc
 *
 * Gets a charactor from the key-board
 *
 * This routine gets a charactor from the key-board, with immediate return. It
 * translates some special characters such as arrow keys into symbolic form.
 *
 * If the higher level code has not called term_set_raw_mode_with_no_echo()
 * before the first call to enhanced_term_getc(), then enhanced_term_getc makes
 * the call.  This sets the terminal into raw mode and takes over the echoing of
 * characters. This means that an inadvertant exit under some systems/shells
 * (including sh/csh) can leave the user without character echo. The mode can be
 * reset using term_reset(). However, this is automaticlally done in
 * kjb_cleanup(), which itself is automatically called on program exit (as well
 * as by kjb_exit() and any abnmormal termination trapped by the KJB library.
 * If the call to term_set_raw_mode_with_no_echo() is in place before the call
 * to enhanced_term_getc(), then the call is not made.
 *
 * This routine assumes that the program is being run with the terminal in raw
 * mode, and thus it does not change the mode back, even if it set it. The
 * model that is being followed is that this routine assumest that it is simply
 * the first routine to be called that needs the raw mode, and thus takes the
 * responsibility of setting it.
 *
 * Although it may seem better to restore the terminal mode on exit, this leads
 * to a number of problems, especially with respect to typing ahead. Since I
 * normally use raw mode for I/O, it is best to enter into raw mode when needed
 * and restore the mode on program exit. Using the macro KJB_INIT() sets things
 * up so that most points of exit are covered by the appropriate cleanup.
 * Nonetheless it is not a bad idea to call kjb_cleanup() before program exit
 * (for systems without atexit()) and using kjb_exit() and kjb_abort() instead
 * of their standard counterparts. Howerver, on most platforms, not doing so
 * will not normally cause problems.
 *
 * In addition to the integer return values that getc() has, this routine will
 * return:
 *    ERROR              Unexpected read problem
 *    INTERRUPTED        Only when I/O is NOT being restarted on interupt
 *    WOULD_BLOCK        Only in non-blocking mode
 *
 *    ERASE_CHAR         As set by "stty erase" or default
 *    UP_ARROW           Based on TERM
 *    DOWN_ARROW         Based on TERM
 *    LEFT_ARROW         Based on TERM
 *    RIGHT_ARROW        Based on TERM
 *
 * Returns:
 *    Described above
 *
 * Index: I/O, terminal I/O, input
 *
 * -----------------------------------------------------------------------------
 */

int enhanced_term_getc(void)
{
    static int first_time = TRUE;


    if (fs_tty_in == NULL)
    {
        NRE(open_tty_in());
    }

    if ((fs_use_smart_terminal) && (fs_smart_terminal_capability))
    {
        if (fs_current_terminal_mode != RAW_WITH_NO_ECHO_TERMINAL_MODE)
        {
            if (first_time)
            {
                first_time = FALSE;
                ERE(term_set_raw_mode_with_no_echo());
                add_cleanup_function(term_reset);
            }
            else
            {
                set_bug("Terminal mode incorrect on second call to enhanced_term_getc");
                return ERROR;
            }
        }
    }

    return enhanced_term_getc_guts();
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*
 * =============================================================================
 * STATIC              enhanced_term_getc_guts
 * -----------------------------------------------------------------------------
 */

static int enhanced_term_getc_guts(void)
{
    int c;
    char* up_arrow_sequence_ptr;
    char* down_arrow_sequence_ptr;
    char* left_arrow_sequence_ptr;
    char* right_arrow_sequence_ptr;
    int   waiting_for_more;
    int   control_sequence;
    int   first_char;
    int   reset_no_blocking_flag;


    up_arrow_sequence_ptr = fs_input_up_arrow_sequence;
    down_arrow_sequence_ptr = fs_input_down_arrow_sequence;
    left_arrow_sequence_ptr = fs_input_left_arrow_sequence;
    right_arrow_sequence_ptr = fs_input_right_arrow_sequence;

    waiting_for_more = FALSE;
    first_char = TRUE;
    reset_no_blocking_flag = FALSE;

    do
    {
        control_sequence = FALSE;

        c = term_getc();

        if (c <= 0)
        {
#ifdef UNIX
            if ((! first_char) && (c == WOULD_BLOCK))
            {
                term_set_blocking();
                reset_no_blocking_flag = TRUE;
            }
            else
            {
                /*
                 * (c == INTERRUPTED) is among the possibilites here.
                 */
                break;
            }
#else
            break;
#endif
        }
        else if (c == '')
        {
            c = INTERRUPTED;
            break;
        }
        else if (c == '')
        {
            c = EOF;
            break;
        }
        else if ((c == fs_erase_char) || (c == '') || (c == ''))
        {
            c = ERASE_CHAR;
            break;
        }

        if (    (c == *up_arrow_sequence_ptr)
             || (*up_arrow_sequence_ptr == '*')
           )
        {
            control_sequence = TRUE;
            up_arrow_sequence_ptr++;
            if (*up_arrow_sequence_ptr == '\0')
            {
                c = UP_ARROW;
                break;
            }
            else
            {
                waiting_for_more = TRUE;
            }
         }

        if (    (c == *down_arrow_sequence_ptr)
             || (*down_arrow_sequence_ptr == '*')
           )
        {
            control_sequence = TRUE;
            down_arrow_sequence_ptr++;
            if (*down_arrow_sequence_ptr == '\0')
            {
                c = DOWN_ARROW;
                break;
            }
            else
            {
                waiting_for_more = TRUE;
            }
         }

        if (    (c == *left_arrow_sequence_ptr)
             || (*left_arrow_sequence_ptr == '*')
           )
        {
            control_sequence = TRUE;
            left_arrow_sequence_ptr++;
            if (*left_arrow_sequence_ptr == '\0')
            {
                c = LEFT_ARROW;
                break;
            }
            else
            {
                waiting_for_more = TRUE;
            }
         }

        if (    (c == *right_arrow_sequence_ptr)
             || (*right_arrow_sequence_ptr == '*')
           )
        {
            control_sequence = TRUE;
            right_arrow_sequence_ptr++;

            if (*right_arrow_sequence_ptr == '\0')
            {
                c = RIGHT_ARROW;
                break;
            }
            else
            {
                waiting_for_more = TRUE;
            }
         }

        if (    (waiting_for_more)
#ifdef UNIX
             && (c != WOULD_BLOCK)
#endif
             && ( ! control_sequence)
            )
        {
            c = INVALID_CONTROL_SEQUENCE;
            break;
        }

        first_char = FALSE;

     }
    while (waiting_for_more);

    if (c == EOF)
    {
        rewind(fs_tty_in);
    }

    if (reset_no_blocking_flag)
    {
        term_set_no_blocking();
    }

    return c;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*
 * =============================================================================
 *                                  term_getc
 *
 *
 *
 * Index: terminal I/O
 *
 *
 * -----------------------------------------------------------------------------
 */

int term_getc(void)
{
    IMPORT volatile int term_io_since_last_input_attempt;
#ifdef SUN5
    IMPORT volatile Bool term_no_block_flag;
#endif
#ifndef MS_OS
    int char_in;
#endif 

    term_io_since_last_input_attempt = FALSE;

#ifdef MS_OS
    if (fs_current_terminal_mode == RAW_WITH_NO_ECHO_TERMINAL_MODE)
    {
        return getch();
    }
    else
    {
        return getche();
    }

#else   /* Non MS_OS case follows. */

#ifdef UNIX

    if (fs_tty_in == NULL)
    {
        NRE(open_tty_in());
    }

#ifdef TEST
    if (fs_emulate_term_input_fp != NULL)
    {
         char_in = fgetc(fs_emulate_term_input_fp);

         if (char_in == '')
         {
             kill_myself(SIGINT);
         }

         term_io_since_last_input_attempt = TRUE;

         return char_in;
     }
#endif

#endif

    char_in = fgetc(fs_tty_in);

    if  (char_in == EOF)
    {
#ifdef SUN5
        if (term_no_block_flag)
        {
            return WOULD_BLOCK;
        }
#endif

        if (errno == 0)
        {
            term_io_since_last_input_attempt = TRUE;

            /*
            // Does not seem to be needed, but rather than test all permutations
            // leave it in.
            */
            clearerr(fs_tty_in);

            /*
            // Should not be needed with clearerr:
            //
            // rewind(fs_tty_in);
            */

            return EOF;
        }
#ifdef UNIX
        else if (errno == NON_BLOCKING_READ_FAILURE_ERROR_NUM)
        {
            return WOULD_BLOCK;
        }
#endif
        else if (errno == EINTR)
        {
#ifdef TEST
            if (fs_term_input_fp != NULL)
            {
                /*
                // FIX
                //
                // Actually, this does not quite cover the possiblity that
                // it was ctl-Z or some other entry which cause the interupt.
                */
                fputc('', fs_term_input_fp);
                (void)fflush(fs_term_input_fp);
            }
#endif
            term_io_since_last_input_attempt = TRUE;
            return INTERRUPTED;
        }
        else
        {
            set_error("Read from keyboard failed.");
            return ERROR;
        }
    }
    else
    {

#ifdef TEST
        if (fs_term_input_fp != NULL)
        {
            fputc(char_in, fs_term_input_fp);
            (void)fflush(fs_term_input_fp);
        }
#endif
        term_io_since_last_input_attempt = TRUE;

        return char_in;
    }

#endif     /*  #ifdef MS_OS ... #else ...   */

}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*
 * =============================================================================
 * STATIC                       term_line_end
 * -----------------------------------------------------------------------------
 */

static void term_line_end(void)
{
    IMPORT volatile int kjb_tty_rows;
    IMPORT volatile Bool io_atn_flag;
    IMPORT volatile int num_term_lines;
    IMPORT volatile int num_term_chars;
    IMPORT volatile Bool pause_on_next;


    num_term_lines++;
    num_term_chars = 0;

    if (    (io_atn_flag)
         ||
           (    (fs_page_flag)
             && (num_term_lines > (kjb_tty_rows-3))
           )
        )
    {
        pause_on_next = TRUE;
    }
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*
 * =============================================================================
 * STATIC                     term_page_end
 * -----------------------------------------------------------------------------
 */

static void term_page_end(void)
{
    IMPORT volatile int     num_term_lines;
    IMPORT volatile int     num_term_chars;
    IMPORT volatile Bool    pause_on_next;
    IMPORT volatile Bool    halt_term_output;
    Continue_query_response response;


    pause_on_next = FALSE;
    num_term_lines = 0;
    num_term_chars = 0;

    response = continue_query();

    if (    (response == NEGATIVE_RESPONSE)
         || (response == STRONG_NEGATIVE_RESPONSE)
       )
    {
        halt_term_output = TRUE;
    }
    else if (response == STRONG_POSITIVE_RESPONSE)
    {
        fs_page_flag = FALSE;
    }
    else if (response == DISABLE_QUERY)
    {
        fs_page_flag = FALSE;
        fs_default_page_flag = FALSE;
    }
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*
 * =============================================================================
 *                                toggle_high_light
 *
 * Toggles output high-lighting if argument is a terminal
 *
 * If "fp" is the terminal, then the high-light setting is toggled, and the
 * appropriate control sequence for high-lighting or normal text is output onto
 * the terminal as appropriate.  If "fp" is not a terminal, then nothing is
 * done.
 *
 * Returns:
 *     NO_ERROR on success and ERROR on failure. If "fp" is not a terminal,
 *     then NO_ERROR is returned, as a properly executed no-op is considered
 *     successfully.
 *
 * Related:
 *     set_high_light, unset_high_light
 *
 * Index: I/O, input, terminal I/O
 *
 * -----------------------------------------------------------------------------
 */

int toggle_high_light(FILE* fp)
{
    if (fs_high_light_flag) return unset_high_light(fp);
    else return set_high_light(fp);
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*
 * =============================================================================
 *                              set_high_light
 *
 * Sets output high-lighing if argument is a terminal.
 *
 * If "fp" is the terminal, then the high-light setting is set to TRUE, and the
 * appropriate control sequence for high-lighting is output onto the terminal.
 * If "fp" is not a terminal, then nothing is done.
 *
 * Returns:
 *     NO_ERROR on success and ERROR on failure. If "fp" is not a terminal,
 *     then NO_ERROR is returned, as a properly executed no-op is considered
 *     successfully.
 *
 * Related:
 *     toggle_high_light, unset_high_light
 *
 * Index: I/O, input, terminal I/O
 *
 * -----------------------------------------------------------------------------
 */

int set_high_light(FILE* fp)
{
    IMPORT volatile int term_io_since_last_input_attempt;


    if ( ! kjb_isatty(fileno(fp))) return NO_ERROR;

    NRE(open_tty_out());

    if ( ! fs_smart_terminal_capability) return ERROR;

    TPUTS(fs_high_light_on_sequence);
    fs_high_light_flag = TRUE;

    term_io_since_last_input_attempt = TRUE;

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*
 * =============================================================================
 *                              unset_high_light
 *
 * Unsets output high-lighing if argument is a terminal.
 *
 * If "fp" is the terminal, then the high-light setting is set to FALSE, and the
 * appropriate control sequence for normal text is output onto the terminal.
 * If "fp" is not a terminal, then nothing is done.
 *
 * Returns:
 *     NO_ERROR on success and ERROR on failure. If "fp" is not a terminal,
 *     then NO_ERROR is returned, as a properly executed no-op is considered
 *     successfully.
 *
 * Related:
 *     toggle_high_light, set_high_light
 *
 * Index: I/O, input, terminal I/O
 *
 * -----------------------------------------------------------------------------
 */

int unset_high_light(FILE* fp)
{
    IMPORT volatile int term_io_since_last_input_attempt;


    if ( ! kjb_isatty(fileno(fp))) return NO_ERROR;

    NRE(open_tty_out());

    if ( ! fs_smart_terminal_capability) return ERROR;

    TPUTS(fs_high_light_off_sequence);
    fs_high_light_flag = FALSE;

    term_io_since_last_input_attempt = TRUE;

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*
 * =============================================================================
 *                              term_blank_out_line
 *
 *
 *
 * Index: terminal I/O
 *
 * -----------------------------------------------------------------------------
 */

void term_blank_out_line (void)
{
    IMPORT volatile int kjb_tty_cols;
    IMPORT volatile int num_term_chars;
    /*   blanks must be at least  40  blanks  */
    const char* blanks = "                                        ";
    /*                    1234567890123456789012345678901234567890   */
    size_t num_blanks;


    num_term_chars = 0;

    num_blanks = kjb_tty_cols;

    term_put_n_raw_chars("\r", 1);

    while (num_blanks > 40)
    {
        term_put_n_raw_chars(blanks, (size_t)40);
        num_blanks -= 40;
    }

    term_put_n_raw_chars(blanks, num_blanks);

    term_put_n_raw_chars("\r", 1);
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*
 * =============================================================================
 *
 * -----------------------------------------------------------------------------
 */


/*
 * This routine should not call any routine which might call an L level error
 * routine (unless changes are made to enter_error_routine_guard() in
 * l_sys_err.c)
*/

#ifdef UNIX
/* UNIX */ void reset_initial_terminal_settings(void)
/* UNIX */ {
/* UNIX */
/* UNIX */
/* UNIX */      if ((fs_tty_in != NULL) && (fs_initial_term_flags_set))
/* UNIX */      {
#ifndef NEXT
/* NOT NEXT */      int foreground_process_group = tcgetpgrp(fileno(fs_tty_in));
/* NOT NEXT */
/* NOT NEXT */      /* If the above is ERROR, we will rashly assume that we */
/* NOT NEXT */      /* are being debugged, or in similar circumstances      */
/* NOT NEXT */      /* where we it is desirable to treat the terminal which */
/* NOT NEXT */      /* in some sense is not behaving as a terminal, as a    */
/* NOT NEXT */      /* terminal.                                            */
/* NOT NEXT */
/* NOT NEXT */      if (    (foreground_process_group > 0)
/* NOT NEXT */           && (foreground_process_group != MY_GROUP)
/* NOT NEXT */         )
/* NOT NEXT */      {
/* NOT NEXT */          /* Background! Don't do anything! */
/* NOT NEXT */          return;
/* NOT NEXT */      }
#endif
/* UNIX */          term_set_blocking();
#ifdef NEXT
/* NEXT */          ioctl(fileno(fs_tty_in), (int)TIOCSETP, &fs_initial_term_flags);
#else
/* NOT NEXT */      ioctl(fileno(fs_tty_in), TCSETS,
/* NOT NEXT */            (void*)&fs_initial_term_flags);
#endif
/* UNIX */      }
/* UNIX */  }
#else
/* default */ void reset_initial_terminal_settings()
/* default */ {
/* default */
/* default */ }
/* default */
#endif

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*
 * =============================================================================
 *
 * -----------------------------------------------------------------------------
 */

/*
// OBSOLETE soon
//
// All cooked mode stuff should be phased out eventually. It seems we really
// only need raw and original?
*/

/* all */     int term_set_cooked_mode(void)
/* all */     {
/* all */         Terminal_settings terminal_settings;
#ifdef UNIX
/* UNIX */
/* UNIX */        Signal_info save_atn_vec;
/* UNIX */        Signal_info save_tstp_vec;
/* UNIX */        Signal_info save_term_vec;
/* UNIX */        Signal_info save_quit_vec;
/* UNIX */        Signal_info new_atn_vec;
/* UNIX */        Signal_info new_tstp_vec;
/* UNIX */        Signal_info new_term_vec;
/* UNIX */        Signal_info new_quit_vec;
#ifdef NEXT
/* next */        struct sgttyb save_term_flags, term_flags;
#else
/* NOT NEXT */    struct termios save_term_flags, term_flags;
#endif
#endif
/* all */
/* all */         if (fs_tty_in == NULL)
/* all */         {
/* all */             open_tty_in();
/* all */
/* all */             if (fs_in_background) return BACKGROUND_READ;
/* all */             else if (fs_tty_in == NULL) return ERROR;
/* all */         }

#ifdef UNIX
#ifndef NEXT
/* NOT NEXT */    else
/* NOT NEXT */    {
/* NOT NEXT */        int foreground_process_group = tcgetpgrp(fileno(fs_tty_in));
/* NOT NEXT */
/* NOT NEXT */
/* NOT NEXT */        /* If the above is ERROR, we will rashly assume that we */
/* NOT NEXT */        /* are being debugged, or in similar circumstances      */
/* NOT NEXT */        /* where we it is desirable to treat the terminal which */
/* NOT NEXT */        /* in some sense is not behaving as a termian, as a     */
/* NOT NEXT */        /* terminal.                                            */
/* NOT NEXT */
/* NOT NEXT */        if (    (foreground_process_group > 0)
/* NOT NEXT */             && (foreground_process_group != MY_GROUP)
/* NOT NEXT */           )
/* NOT NEXT */        {
/* NOT NEXT */            return BACKGROUND_READ;
/* NOT NEXT */        }
/* NOT NEXT */    }
#endif
#endif

/* all */
/* all */         if ( ! ((fs_use_smart_terminal) && (fs_smart_terminal_capability)))
/* all */         {
/* all */             return NO_ERROR;    /* No-op ! */
/* all */         }
#ifdef UNIX
/* UNIX */        INIT_SIGNAL_INFO(new_atn_vec);
/* UNIX */        new_atn_vec.SIGNAL_HANDLER = cooked_mode_sig_fn;
/* UNIX */        SET_DONT_RESTART_IO_AFTER_SIGNAL(new_atn_vec);
/* UNIX */        kjb_sigvec(SIGINT, &new_atn_vec, &save_atn_vec);
/* UNIX */
/* UNIX */        INIT_SIGNAL_INFO(new_tstp_vec);
/* UNIX */        new_tstp_vec.SIGNAL_HANDLER = cooked_mode_sig_fn;
/* UNIX */        SET_DONT_RESTART_IO_AFTER_SIGNAL(new_tstp_vec);
/* UNIX */        kjb_sigvec(SIGTSTP, &new_tstp_vec, &save_tstp_vec);
/* UNIX */
/* UNIX */        INIT_SIGNAL_INFO(new_term_vec);
/* UNIX */        new_term_vec.SIGNAL_HANDLER = cooked_mode_sig_fn;
/* UNIX */        SET_DONT_RESTART_IO_AFTER_SIGNAL(new_term_vec);
/* UNIX */        kjb_sigvec(SIGTERM, &new_term_vec, &save_term_vec);
/* UNIX */
/* UNIX */        INIT_SIGNAL_INFO(new_quit_vec);
/* UNIX */        new_quit_vec.SIGNAL_HANDLER = cooked_mode_sig_fn;
/* UNIX */        SET_DONT_RESTART_IO_AFTER_SIGNAL(new_quit_vec);
/* UNIX */        kjb_sigvec(SIGQUIT, &new_quit_vec, &save_quit_vec);
/* UNIX */
#ifdef NEXT
/* next */        ERE(kjb_ioctl(fileno(fs_tty_in), (IOCTL_REQUEST_TYPE)TIOCGETP,
/* next */                      &save_term_flags));
/* next */
/* next */        term_flags = save_term_flags;
/* next */
/* next */        term_flags.sg_flags &= (~((unsigned short)CBREAK));
/* next */
/* next */        term_flags.sg_flags |= ((unsigned short)ECHO);
/* next */
/* next */        ERE(kjb_ioctl(fileno(fs_tty_in), (IOCTL_REQUEST_TYPE)TIOCSETP, &term_flags));
#else
/* NOT next */    ERE(kjb_ioctl(fileno(fs_tty_in), (IOCTL_REQUEST_TYPE)TCGETS,
/* NOT next */                  (void*)&save_term_flags));
/* NOT next */
/* NOT next */    term_flags = save_term_flags;
/* NOT next */
/* NOT next */    /* c_cc[VMIN] is EOF char in non-canonical mode ! */
/* NOT next */
/* NOT next */    if (fs_initial_term_flags_set)
/* NOT next */    {
/* NOT next */        term_flags.c_cc[VMIN] = fs_initial_term_flags.c_cc[VMIN];
/* NOT next */    }
/* NOT next */    else
/* NOT next */    {
/* NOT next */        term_flags.c_cc[VMIN] = 4;
/* NOT next */    }
/* NOT next */    term_flags.c_lflag |= ((long)ICANON);
/* NOT next */
/* NOT next */    term_flags.c_lflag |= ((long)ECHO);
#ifdef AIX
/* AIX */         term_flags.c_lflag |= ((long)ECHOCTL);
#endif
#ifdef SUN
/* SUN */         term_flags.c_lflag |= ((long)ECHOCTL);
#endif
/* NOT next */
/* NOT next */    ERE(kjb_ioctl(fileno(fs_tty_in), (IOCTL_REQUEST_TYPE)TCSETS,
/* NOT next */                  (void*)&term_flags));
#endif
/* UNIX */
/* UNIX */
/* UNIX */        terminal_settings.atn_vec = save_atn_vec;
/* UNIX */        terminal_settings.tstp_vec = save_tstp_vec;
/* UNIX */        terminal_settings.term_vec = save_term_vec;
/* UNIX */        terminal_settings.quit_vec = save_quit_vec;
/* UNIX */
/* UNIX */        terminal_settings.term_flags = save_term_flags;
#endif
/* all */         terminal_settings.terminal_mode = fs_current_terminal_mode;
/* all */
                  SKIP_HEAP_CHECK_2();
/* all */         ERE(alloc_insert_into_queue(&fs_save_terminal_settings_head,
/* all */                                     (Queue_element**)NULL,
/* all */                                     (void*)&terminal_settings,
/* all */                                     sizeof(terminal_settings)));
                  CONTINUE_HEAP_CHECK_2();
/* all */
/* all */         fs_current_terminal_mode = ORIGINAL_TERMINAL_MODE;
/* all */
/* all */         return NO_ERROR;
/* all */     }
/* all */
/* all */ /* /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */
/* all */
/* all */     int term_set_raw_mode_with_no_echo(void)
/* all */     {
/* all */         Terminal_settings terminal_settings;
#ifdef UNIX
/* UNIX */        Signal_info save_atn_vec;
/* UNIX */        Signal_info save_tstp_vec;
/* UNIX */        Signal_info save_term_vec;
/* UNIX */        Signal_info save_quit_vec;
/* UNIX */        Signal_info new_atn_vec;
/* UNIX */        Signal_info new_tstp_vec;
/* UNIX */        Signal_info new_term_vec;
/* UNIX */        Signal_info new_quit_vec;
#ifdef NEXT
/* next */        struct sgttyb save_term_flags, term_flags;
#else
/* NOT next */    struct termios save_term_flags, term_flags;
#endif
#endif
/* all */
/* all */         if (fs_tty_in == NULL)
/* all */         {
/* all */             open_tty_in();
/* all */
/* all */             if (fs_in_background) return BACKGROUND_READ ;
/* all */             else if (fs_tty_in == NULL) return ERROR;
/* all */         }

#ifdef UNIX
#ifndef NEXT
/* NOT NEXT */    else
/* NOT NEXT */    {
/* NOT NEXT */        int foreground_process_group = tcgetpgrp(fileno(fs_tty_in));
/* NOT NEXT */
/* NOT NEXT */        /* If the above is ERROR, we will rashly assume that we */
/* NOT NEXT */        /* are being debugged, or in similar circumstances      */
/* NOT NEXT */        /* where we it is desirable to treat the terminal which */
/* NOT NEXT */        /* in some sense is not behaving as a terminal, as a    */
/* NOT NEXT */        /* terminal.                                            */
/* NOT NEXT */
/* NOT NEXT */        if (    (foreground_process_group > 0)
/* NOT NEXT */             && (foreground_process_group != MY_GROUP)
/* NOT NEXT */           )
/* NOT NEXT */        {
/* NOT NEXT */            return BACKGROUND_READ;
/* NOT NEXT */        }
/* NOT NEXT */    }
#endif
#endif

/* all */
/* all */         if ( ! ((fs_use_smart_terminal) && (fs_smart_terminal_capability)))
/* all */         {
/* all */             return NO_ERROR;    /* No-op ! */
/* all */         }
#ifdef UNIX
/* UNIX */        INIT_SIGNAL_INFO(new_atn_vec);
/* UNIX */        new_atn_vec.SIGNAL_HANDLER = raw_mode_with_no_echo_sig_fn;
/* UNIX */        SET_DONT_RESTART_IO_AFTER_SIGNAL(new_atn_vec);
/* UNIX */        kjb_sigvec(SIGINT, &new_atn_vec, &save_atn_vec);
/* UNIX */
/* UNIX */        INIT_SIGNAL_INFO(new_tstp_vec);
/* UNIX */        new_tstp_vec.SIGNAL_HANDLER = raw_mode_with_no_echo_sig_fn;
/* UNIX */        SET_DONT_RESTART_IO_AFTER_SIGNAL(new_tstp_vec);
/* UNIX */        kjb_sigvec(SIGTSTP, &new_tstp_vec, &save_tstp_vec);
/* UNIX */
/* UNIX */        INIT_SIGNAL_INFO(new_term_vec);
/* UNIX */        new_term_vec.SIGNAL_HANDLER = raw_mode_with_no_echo_sig_fn;
/* UNIX */        SET_DONT_RESTART_IO_AFTER_SIGNAL(new_term_vec);
/* UNIX */        kjb_sigvec(SIGTERM, &new_term_vec, &save_term_vec);
/* UNIX */
/* UNIX */        INIT_SIGNAL_INFO(new_quit_vec);
/* UNIX */        new_quit_vec.SIGNAL_HANDLER = raw_mode_with_no_echo_sig_fn;
/* UNIX */        SET_DONT_RESTART_IO_AFTER_SIGNAL(new_quit_vec);
/* UNIX */        kjb_sigvec(SIGQUIT, &new_quit_vec, &save_quit_vec);
/* UNIX */
#ifdef NEXT
/* next */        ERE(kjb_ioctl(fileno(fs_tty_in), (IOCTL_REQUEST_TYPE)TIOCGETP,
/* next */                      &save_term_flags));
/* next */
/* next */        term_flags = save_term_flags;
/* next */
/* next */        term_flags.sg_flags |= ((unsigned short)CBREAK);
/* next */
/* next */        term_flags.sg_flags &= (~((unsigned short)ECHO));
/* next */
/* next */        ERE(kjb_ioctl(fileno(fs_tty_in), 
                                (IOCTL_REQUEST_TYPE)TIOCSETP, &term_flags));
#else
/* NOT next */    ERE(kjb_ioctl(fileno(fs_tty_in), (IOCTL_REQUEST_TYPE)TCGETS,
/* NOT next */                  (void*)&save_term_flags));
/* NOT next */
/* NOT next */    term_flags = save_term_flags;
/* NOT next */
/* NOT next */    term_flags.c_cc[VMIN] = 1;
/* NOT next */    term_flags.c_cc[VTIME] = 0;
/* NOT next */
/* NOT next */    term_flags.c_lflag &= (~((long)ICANON));
/* NOT next */
/* NOT next */    term_flags.c_lflag &= (~((long)ECHO)) ;
#ifdef SUN
/* SUN */         term_flags.c_lflag &= (~((long)ECHOCTL)) ;
#endif
/* NOT next */    ERE(kjb_ioctl(fileno(fs_tty_in), (IOCTL_REQUEST_TYPE)TCSETS,
/* NOT next */                  (void*)&term_flags));
#endif
/* UNIX */
/* UNIX */        terminal_settings.atn_vec = save_atn_vec;
/* UNIX */        terminal_settings.tstp_vec = save_tstp_vec;
/* UNIX */        terminal_settings.term_vec = save_term_vec;
/* UNIX */        terminal_settings.quit_vec = save_quit_vec;
/* UNIX */
/* UNIX */        terminal_settings.term_flags = save_term_flags;
#endif
/* all */         terminal_settings.terminal_mode = fs_current_terminal_mode;
/* all */
/* all */         fs_current_terminal_mode = RAW_WITH_NO_ECHO_TERMINAL_MODE;
/* all */
                  SKIP_HEAP_CHECK_2();
/* all */         ERE(alloc_insert_into_queue(&fs_save_terminal_settings_head,
/* all */                                     (Queue_element**)NULL,
/* all */                                     (void*)&terminal_settings,
/* all */                                     sizeof(terminal_settings)));
                  CONTINUE_HEAP_CHECK_2();
/* all */
/* all */         return NO_ERROR;
/* all */     }
/* all */
/* all */ /* /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */
/* all */
/* all */     int term_set_raw_mode_with_echo(void)
/* all */     {
/* all */         Terminal_settings terminal_settings;
#ifdef UNIX
/* UNIX */
/* UNIX */        Signal_info save_atn_vec;
/* UNIX */        Signal_info save_tstp_vec;
/* UNIX */        Signal_info save_term_vec;
/* UNIX */        Signal_info save_quit_vec;
/* UNIX */        Signal_info new_atn_vec;
/* UNIX */        Signal_info new_tstp_vec;
/* UNIX */        Signal_info new_term_vec;
/* UNIX */        Signal_info new_quit_vec;
#ifdef NEXT
/* next */        struct sgttyb save_term_flags, term_flags;
#else
/* NOT next */    struct termios save_term_flags, term_flags;
#endif
#endif
/* all */
/* all */         if (fs_tty_in == NULL)
/* all */         {
/* all */             open_tty_in();
/* all */
/* all */             if (fs_in_background) return BACKGROUND_READ;
/* all */             else if (fs_tty_in == NULL) return ERROR;
/* all */         }

#ifdef UNIX
#ifndef NEXT
/* NOT NEXT */    else
/* NOT NEXT */    {
/* NOT NEXT */        int foreground_process_group = tcgetpgrp(fileno(fs_tty_in));
/* NOT NEXT */
/* NOT NEXT */
/* NOT NEXT */        /* If the above is ERROR, we will rashly assume that we */
/* NOT NEXT */        /* are being debugged, or in similar circumstances      */
/* NOT NEXT */        /* where we it is desirable to treat the terminal which */
/* NOT NEXT */        /* in some sense is not behaving as a termian, as a     */
/* NOT NEXT */        /* terminal.                                            */
/* NOT NEXT */
/* NOT NEXT */        if (    (foreground_process_group > 0)
/* NOT NEXT */             && (foreground_process_group != MY_GROUP)
/* NOT NEXT */           )
/* NOT NEXT */        {
/* NOT NEXT */            return BACKGROUND_READ;
/* NOT NEXT */        }
/* NOT NEXT */    }
#endif
#endif
/* all */
/* all */         if ( ! ((fs_use_smart_terminal) && (fs_smart_terminal_capability)))
/* all */         {
/* all */             return NO_ERROR;    /* No-op ! */
/* all */         }
#ifdef UNIX
/* UNIX */        INIT_SIGNAL_INFO(new_atn_vec);
/* UNIX */        new_atn_vec.SIGNAL_HANDLER = raw_mode_with_echo_sig_fn;
/* UNIX */        SET_DONT_RESTART_IO_AFTER_SIGNAL(new_atn_vec);
/* UNIX */        kjb_sigvec(SIGINT, &new_atn_vec, &save_atn_vec);
/* UNIX */
/* UNIX */        INIT_SIGNAL_INFO(new_tstp_vec);
/* UNIX */        new_tstp_vec.SIGNAL_HANDLER = raw_mode_with_echo_sig_fn;
/* UNIX */        SET_DONT_RESTART_IO_AFTER_SIGNAL(new_tstp_vec);
/* UNIX */        kjb_sigvec(SIGTSTP, &new_tstp_vec, &save_tstp_vec);
/* UNIX */
/* UNIX */        INIT_SIGNAL_INFO(new_term_vec);
/* UNIX */        new_term_vec.SIGNAL_HANDLER = raw_mode_with_echo_sig_fn;
/* UNIX */        SET_DONT_RESTART_IO_AFTER_SIGNAL(new_term_vec);
/* UNIX */        kjb_sigvec(SIGTERM, &new_term_vec, &save_term_vec);
/* UNIX */
/* UNIX */        INIT_SIGNAL_INFO(new_quit_vec);
/* UNIX */        new_quit_vec.SIGNAL_HANDLER = raw_mode_with_echo_sig_fn;
/* UNIX */        SET_DONT_RESTART_IO_AFTER_SIGNAL(new_quit_vec);
/* UNIX */        kjb_sigvec(SIGQUIT, &new_quit_vec, &save_quit_vec);
/* UNIX */
#ifdef NEXT
/* next */        ERE(kjb_ioctl(fileno(fs_tty_in), 
                                (IOCTL_REQUEST_TYPE)TIOCGETP,
/* next */                      &save_term_flags));
/* next */
/* next */        term_flags = save_term_flags;
/* next */
/* next */        term_flags.sg_flags |= ((unsigned short)CBREAK);
/* next */
/* next */        term_flags.sg_flags |= ((unsigned short)ECHO);
/* next */
/* next */        ERE(kjb_ioctl(fileno(fs_tty_in), (IOCTL_REQUEST_TYPE)TIOCSETP,
                                &term_flags));
#else
/* NOT next */    ERE(kjb_ioctl(fileno(fs_tty_in), (IOCTL_REQUEST_TYPE)TCGETS,
/* NOT next */                       (void*)&save_term_flags));
/* NOT next */
/* NOT next */    term_flags = save_term_flags;
/* NOT next */    term_flags.c_cc[VMIN] = 1;
/* NOT next */    term_flags.c_cc[VTIME] = 0;
/* NOT next */    term_flags.c_lflag &= (~((long)ICANON));
/* NOT next */
/* NOT next */    term_flags.c_lflag |= ((long)ECHO) ;
#ifdef SUN
/* SUN */         term_flags.c_lflag |= ((long)ECHOCTL) ;
#endif
/* NOT next */
/* NOT next */    ERE(kjb_ioctl(fileno(fs_tty_in), (IOCTL_REQUEST_TYPE)TCSETS,
/* NOT next */                  (void*)&term_flags));
#endif
/* UNIX */
/* UNIX */
/* UNIX */        terminal_settings.atn_vec = save_atn_vec;
/* UNIX */        terminal_settings.tstp_vec = save_tstp_vec;
/* UNIX */        terminal_settings.term_vec = save_term_vec;
/* UNIX */        terminal_settings.quit_vec = save_quit_vec;
/* UNIX */
/* UNIX */        terminal_settings.term_flags = save_term_flags;
#endif
/* all */         terminal_settings.terminal_mode = fs_current_terminal_mode;
/* all */
                  SKIP_HEAP_CHECK_2();
/* all */         ERE(alloc_insert_into_queue(&fs_save_terminal_settings_head,
/* all */                                     (Queue_element**)NULL,
/* all */                                     (void*)&terminal_settings,
/* all */                                     sizeof(terminal_settings)));
                  CONTINUE_HEAP_CHECK_2();
/* all */
/* all */         fs_current_terminal_mode = RAW_WITH_ECHO_TERMINAL_MODE;
/* all */
/* all */         return NO_ERROR;
/* all */     }
/* all */
/* all */ /* /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */
/* all */
/* all */     void term_reset(void)
/* all */     {
/* all */         Terminal_settings terminal_settings;
/* all */         Queue_element* cur_elem;
#ifdef UNIX
/* UNIX */        Signal_info save_atn_vec;
/* UNIX */        Signal_info save_tstp_vec;
/* UNIX */        Signal_info save_term_vec;
/* UNIX */        Signal_info save_quit_vec;
#ifdef NEXT
/* next */        struct sgttyb save_term_flags;
#else
/* NOT next */    struct termios save_term_flags;
#endif
#endif
/* all */
/* all */         if (fs_tty_in == NULL)
/* all */         {
/* all */             return;
/* all */         }
/* all */
/* all */         if (fs_save_terminal_settings_head == NULL)
/* all */         {
/* all */             return;
/* all */         }
/* all */
/* all */         cur_elem = remove_first_element(&fs_save_terminal_settings_head,
/* all */                                         (Queue_element**)NULL);
/* all */
/* all */         terminal_settings = *((Terminal_settings*)cur_elem->contents);
/* all */
#ifdef UNIX
/* UNIX */        save_term_flags = terminal_settings.term_flags;
/* UNIX */
/* UNIX */        save_atn_vec = terminal_settings.atn_vec;
/* UNIX */        save_tstp_vec = terminal_settings.tstp_vec;
/* UNIX */        save_term_vec = terminal_settings.term_vec;
/* UNIX */        save_quit_vec = terminal_settings.quit_vec;
#ifdef NEXT
/* next */        kjb_ioctl(fileno(fs_tty_in), (IOCTL_REQUEST_TYPE)TIOCSETP,
                            &save_term_flags);
#else
/* NOT next */    kjb_ioctl(fileno(fs_tty_in), (IOCTL_REQUEST_TYPE)TCSETS,
/* NOT next */              (void*)&save_term_flags);
#endif
/* UNIX */        kjb_sigvec(SIGINT, &save_atn_vec, (Signal_info *)NULL);
/* UNIX */        kjb_sigvec(SIGTSTP, &save_tstp_vec, (Signal_info*)NULL);
/* UNIX */        kjb_sigvec(SIGTERM, &save_term_vec, (Signal_info*)NULL);
/* UNIX */        kjb_sigvec(SIGQUIT, &save_quit_vec, (Signal_info*)NULL);
#endif
/* all */         fs_current_terminal_mode = terminal_settings.terminal_mode;
/* all */
                  SKIP_HEAP_CHECK_2();
/* all */         free_queue_element(cur_elem, kjb_free);
                  CONTINUE_HEAP_CHECK_2();
/* all */     }
/* all */
/* all */ /* /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */
/* all */
/* all */     void term_rewind_setting_stack(void)
/* all */     {
/* all */         Terminal_settings     terminal_settings;
/* all */         Queue_element*        cur_elem;
#ifdef UNIX
/* UNIX */        Signal_info save_atn_vec;
/* UNIX */        Signal_info save_tstp_vec;
/* UNIX */        Signal_info save_term_vec;
/* UNIX */        Signal_info save_quit_vec;
#endif
/* all */
/* all */
/* all */         /*CONSTCOND*/
/* all */         while ( TRUE )
/* all */         {
/* all */             cur_elem =
/* all */                   remove_first_element(&fs_save_terminal_settings_head,
/* all */                                        (Queue_element**)NULL);
/* all */
/* all */             if (cur_elem == NULL) break;
/* all */
/* all */             terminal_settings =
/* all */                   *((Terminal_settings*)cur_elem->contents);
/* all */
#ifdef UNIX
/* UNIX */            save_atn_vec = terminal_settings.atn_vec;
/* UNIX */            save_tstp_vec = terminal_settings.tstp_vec;
/* UNIX */            save_term_vec = terminal_settings.term_vec;
/* UNIX */            save_quit_vec = terminal_settings.quit_vec;
/* UNIX */
/* UNIX */            kjb_sigvec(SIGINT, &save_atn_vec,   (Signal_info*)NULL);
/* UNIX */            kjb_sigvec(SIGTSTP, &save_tstp_vec, (Signal_info*)NULL);
/* UNIX */            kjb_sigvec(SIGTERM, &save_term_vec, (Signal_info*)NULL);
/* UNIX */            kjb_sigvec(SIGQUIT, &save_quit_vec, (Signal_info*)NULL);
#endif
/* all */
                     SKIP_HEAP_CHECK_2();
/* all */            free_queue_element(cur_elem, kjb_free);
                     CONTINUE_HEAP_CHECK_2();
/* UNIX */        }
/* all */     }
/* all */

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

#ifdef UNIX
/* UNIX */     static int term_putc_fn_arg(int c)
/* UNIX */
/* UNIX */    {
/* UNIX */        return fputc(c, fs_tty_out);
/* UNIX */    }
/* UNIX */
/* UNIX */
#endif

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*
 * =============================================================================
 *                             term_set_no_blocking
 *
 * Sets the terminal into non blocking mode on subsequent reads.
 *
 * Returns:
 *    On failure an error message is set, and ERROR is returned. Otherwise
 *    NO_ERROR is retuned.
 *
 * Index: terminal I/O, I/O
 *
 * -----------------------------------------------------------------------------
*/

void term_set_no_blocking(void)
{
    IMPORT volatile Bool term_no_block_flag;


    if (fs_tty_in == NULL)
    {
        NR(open_tty_in());
    }

    ER(set_no_blocking(fileno(fs_tty_in)));

    term_no_block_flag = TRUE;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*
 * =============================================================================
 *                              term_set_blocking
 *
 * Sets the terminal into blocking mode on subsequent reads.
 *
 * Returns:
 *    On failure an error message is set, and ERROR is returned. Otherwise
 *    NO_ERROR is retuned.
 *
 * Index: terminal I/O, I/O
 *
 * -----------------------------------------------------------------------------
 */


void term_set_blocking(void)
{
    IMPORT volatile Bool term_no_block_flag;


    if (fs_tty_in == NULL)
    {
        NR(open_tty_in());
    }

    ER(set_blocking(fileno(fs_tty_in)));

    term_no_block_flag = FALSE;

}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

#if 0 /* was ifdef OBSOLETE */
/*
 * =============================================================================
 *
 * -----------------------------------------------------------------------------
 */

/*
 *  This routine should not call any routines which might call an L level
 *  error routine. (Under the current reentry behaviour of those routines.)
 */

int term_input_queue_is_empty()
{
    if (fs_term_buff_head == NULL) return TRUE;

    /*
       If the rentry list was used, then the first entry would be an
       artificially inserted blank. (HACK)
    */
    if (fs_term_buff_head->next == NULL) return TRUE;     /* Skip initial blank */

    return FALSE;
}
#endif

/*
 * =============================================================================
 *
 * -----------------------------------------------------------------------------
 */

/*
 *  This routine should do without L routines where thay are not adding
 *  functionality, as it is used in bug handling.
 */

void print_term_input_queue(FILE* fp)
{
    Queue_element* cur_elem;
    int            num_to_skip = fs_previous_sessions_term_buff_count;


    cur_elem = fs_term_buff_head;

    /*
    // If the rentry list was used, then the first entry would be an
    // artificially inserted blank. Jump over this.
    */
    if (cur_elem != NULL) cur_elem = cur_elem->next;    /* Skip initial blank */

    while ((cur_elem != NULL) && (num_to_skip > 0))
    {
        num_to_skip--;
        cur_elem = cur_elem->next;
    }

    while (cur_elem != NULL)
    {
        fputs((char*)(cur_elem->contents), fp);
        fputs("\n", fp);

        cur_elem = cur_elem->next;
    }
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

static void add_line_to_reentry_queue(const char* line)
{

    SKIP_HEAP_CHECK_2();
    insert_at_end_of_queue(&fs_term_buff_head, &fs_term_buff_tail,
                           (void*)kjb_strdup(line));
    CONTINUE_HEAP_CHECK_2();

    fs_term_buff_count++;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

void read_line_reentry_history(const char* program_name)
{
    int            save_term_buff_count;
    FILE*          reentry_fp;
    Queue_element* save_begining;
    Queue_element* save_tail;
    char           line[ 1000 ];


    BUFF_CPY(fs_reentry_file_path, "~/.");
    BUFF_CAT(fs_reentry_file_path, program_name);
    BUFF_CAT(fs_reentry_file_path, "-history");

    add_cleanup_function(write_line_reentry_history);

    NR(reentry_fp = kjb_fopen(fs_reentry_file_path, "r"));

    if (fs_term_buff_count == 0) add_line_to_reentry_queue("");

    save_term_buff_count = fs_term_buff_count;
    save_begining = fs_term_buff_head->next;  /* First other than first "". */
    save_tail = fs_term_buff_tail;

    fs_term_buff_head->next = NULL;
    fs_term_buff_tail = fs_term_buff_head;

    while (BUFF_FGET_LINE(reentry_fp, line) != EOF)
    {
        add_line_to_reentry_queue(line);
    }

    if (save_begining != NULL)
    {
        fs_term_buff_tail->next = save_begining;
        fs_term_buff_tail       = save_tail;
    }

    fs_previous_sessions_term_buff_count = fs_term_buff_count - save_term_buff_count;

    EPE(kjb_fclose(reentry_fp));
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

static void write_line_reentry_history(void)
{
    IMPORT volatile Bool  halt_all_output;
    Bool                  save_halt_all_output = halt_all_output;
    FILE*                 reentry_fp;
    Queue_element*        cur_elem;
    int                   count;
    char                  time_buff[ 100 ];


    /*
    // Don't overwrite history if this is a script!
    */
    if ( ! is_interactive()) return;

    count = fs_term_buff_count;

    cur_elem = fs_term_buff_head;

    /*
    // If the rentry list was used, then the first entry would be an
    // artificially inserted blank. Jump over this.
    */
    if (cur_elem != NULL) cur_elem = cur_elem->next;    /* Skip initial blank */
    count--;

    if (cur_elem == NULL) return;

    halt_all_output = FALSE;

    NR(reentry_fp = kjb_fopen(fs_reentry_file_path, "w"));

    while ((cur_elem != NULL) && (count > MAX_NUM_HISTORY_LINES_WRITEN))
    {
        count--;
        cur_elem = cur_elem->next;
    }

    while (cur_elem != NULL)
    {
        EPE(kjb_fputs(reentry_fp, (char*)(cur_elem->contents)));
        EPE(kjb_fputs(reentry_fp, "\n"));

        cur_elem = cur_elem->next;
    }

    EPE(BUFF_GET_TIME(time_buff));
    EPE(kjb_fprintf(reentry_fp, "### Session ended at %s. ###\n", time_buff));
    EPE(kjb_fclose(reentry_fp));

    halt_all_output = save_halt_all_output;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

#ifdef TRACK_MEMORY_ALLOCATION

/*
// Only use at program exit to find memory leaks.
*/

void free_line_reentry_queue(void)
{
    fs_term_buff_count = 0;
    free_queue(&fs_term_buff_head, &fs_term_buff_tail, kjb_free);
}
#endif

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

#ifdef __cplusplus
}
#endif

