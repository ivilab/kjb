
/* $Id: l_sys_sig.h 21593 2017-07-30 16:48:05Z kobus $ */

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

#ifndef L_SYS_SIG_INCLUDED
#define L_SYS_SIG_INCLUDED


#include "l/l_def.h"


/*Some versions of makedepend cannot be trusted to honor the "-Y" option. */
#ifndef MAKE_DEPEND

#ifndef __USE_POSIX
#    define __USE_POSIX
#endif 

#include <signal.h>
#ifdef NeXT
#    include <sys/signal.h>
#endif
#endif   /* ! MAKE_DEPEND */

#ifdef __cplusplus
extern "C" {
#ifdef COMPILING_CPLUSPLUS_SOURCE
namespace kjb_c {
#endif
#endif


/*
// Depending on the system, the number of actual signals can vary. BSD type
// systems tend to have about 32, SUN5 defines about 45 ATP. We will simply
// set up for handling ones up to (SIGNAL_ARRAY_SIZE-1).
//
// IMPORTANT NOTE: If SIGNAL_ARRAY_SIZE is changed, then the global
// initialization in l_global.h
*/
#define SIGNAL_ARRAY_SIZE  50


#ifdef LINUX
#ifdef KJB_CPLUSPLUS
extern "C"
{
#endif
    extern int sighold(int);
    extern int sigrelse(int);
    extern void (*sigset(int, void (*)(int)))(int);
#ifdef KJB_CPLUSPLUS
}
#endif
#endif


#ifdef UNIX                /* UNIX */
#    ifdef MAC_OSX
#        define KJB_SIGNAL                signal
#        define KJB_SIGVEC                sigaction
#    else
#    ifdef SYSV_SIGNALS
#        define KJB_SIGNAL                sigset
#        define KJB_SIGVEC                sigaction
#    else     /* not SYSV (BSD ? ) */
#        define KJB_SIGNAL                signal
#        define KJB_SIGVEC                sigvec
#    endif
#    endif

#else     /* Non-unix systems follow. */

#ifdef MS_OS

#   define KJB_SIGNAL signal

#else

#    define KJB_SIGNAL signal

#endif

#endif    /*   #ifdef UNIX ... #else ...  */


#ifdef UNIX

#    define TRAP_FN_RETURN_TYPE            void
#    define MS_OS_USERENTRY

#    ifdef SUN4
#        define TRAP_FN_ARGS \
               int sig, int dummy1, struct sigcontext* dummy2, char* dummy3
#        define TRAP_FN_DUMMY_ARGS \
               int dummy_sig, int dummy1, struct sigcontext* dummy2, \
               char* dummy3

#    else
#        define TRAP_FN_ARGS          int sig
#        define TRAP_FN_DUMMY_ARGS    int __attribute__((unused)) dummy_sig
#    endif

#    ifdef SUN4
        /* Long way to say zero! (Keeps error checkers happy) */
#        define KJB_SIG_DFL ((TRAP_FN_RETURN_TYPE(*)(TRAP_FN_ARGS)) 0)
#    else
#        define KJB_SIG_DFL  SIG_DFL
#    endif

        /* End of signal handler definition for UNIX */
        /* ---------------------------------------------------- */
#else    /* Non-unix signal handler definition follows. */

#ifdef MS_OS

/*
#        define MS_OS_USERENTRY          _USERENTRY
*/
#        define MS_OS_USERENTRY          
#        define TRAP_FN_RETURN_TYPE      void
#        define TRAP_FN_ARGS             int sig
#        define TRAP_FN_DUMMY_ARGS       int dummy_sig
#        define KJB_SIG_DFL  SIG_DFL


#else  /* Default -- Likely won't work for any system! */

/* Need to CHECK */

#    define MS_OS_USERENTRY
#    define TRAP_FN_RETURN_TYPE       void
#    define TRAP_FN_ARGS              int sig
#    define TRAP_FN_DUMMY_ARGS        int dummy_sig


#endif     /* End of default definition of signal handler function. */

#endif
/* --------- End of definition of signal handler function. ---------- */


/* --------- Define stuff for more platforma independent signal installing */
#ifdef UNIX                /* UNIX */

#    ifdef SYSV_SIGNALS
#        define Signal_info           struct sigaction
#        define SIGNAL_HANDLER        sa_handler

#ifdef __lint
#        define INIT_SIGNAL_INFO(x)  {SET_TO_ZERO(x.sa_mask);x.sa_flags = 0;}
#else
#        define INIT_SIGNAL_INFO(x)                                            \
                  do                                                           \
                  {                                                            \
                      SET_TO_ZERO(x.sa_mask);                                  \
                      x.sa_flags = 0;                                          \
                  }                                                            \
                  while ( FALSE );
#endif /* #ifdef __lint ... #else .... */

#        ifndef SGI
#            define SET_DONT_RESTART_IO_AFTER_SIGNAL(x) \
                                                x.sa_flags &= (~SA_RESTART)
#            define SET_RESTART_IO_AFTER_SIGNAL(x) \
                                                x.sa_flags |= SA_RESTART
#        else   /* Case SGI */
            /*
             * SGI does not support restart after signal.
            */
#            define SET_DONT_RESTART_IO_AFTER_SIGNAL(x)
#            define SET_RESTART_IO_AFTER_SIGNAL(x)
#        endif  /*  #ifndef SGI .... #else  ...  */

#    else     /* not SYSV_SIGNALS (BSD ? ) */
#        define Signal_info           struct sigvec
#        define SIGNAL_HANDLER        sv_handler

#ifdef __lint
#        define INIT_SIGNAL_INFO(x)  { x.sv_mask=0; x.sv_flags=0; }
#else
#        define INIT_SIGNAL_INFO(x)                                            \
                  do                                                           \
                  {                                                            \
                      x.sv_mask=0;                                             \
                      x.sv_flags=0;                                            \
                  }                                                            \
                  while ( FALSE );
#endif

#        define SET_DONT_RESTART_IO_AFTER_SIGNAL(x)  x.sv_flags |= SV_INTERRUPT
#        define SET_RESTART_IO_AFTER_SIGNAL(x)     x.sv_flags &= (~SV_INTERRUPT)

#    endif   /* #ifdef SYSV_SIGNALS ...  #else .... */

#else     /* Non-unix systems follow. */

#ifdef MS_OS
#   define INIT_SIGNAL_INFO(x) 
#   define SIGNAL_HANDLER  signal_handler_fn  

    typedef struct Signal_info
    {
       TRAP_FN_RETURN_TYPE (MS_OS_USERENTRY *SIGNAL_HANDLER)(TRAP_FN_ARGS);
    }
    Signal_info;
#else     /* Default CASE. Likely broken on any system that gets to here. */
#    ifndef SIGINT
#        define SIGINT   2
#    endif
#endif    /* End of case NOT UNIX. */

#endif    /*   #ifdef UNIX ... #else ...  */

/* -------------------------------------------------------------------------- */


/* Desired default behaviour must be ZERO. */
#define RESTART_AFTER_SIGNAL         0
#define DONT_RESTART_AFTER_SIGNAL    1

int set_io_atn_trap         (void);
int unset_io_atn_trap       (void);
int set_iteration_atn_trap  (void);
int unset_iteration_atn_trap(void);
int set_default_abort_trap  (void);

int set_abort_trap(TRAP_FN_RETURN_TYPE (*)(TRAP_FN_ARGS));

int unset_abort_trap(void);
void allow_abort_sig(void);

int set_atn_trap  (TRAP_FN_RETURN_TYPE (*)(TRAP_FN_ARGS), int);
int unset_atn_trap(void);

int set_sig_trap
(
    int                 sig,
    TRAP_FN_RETURN_TYPE (*)(TRAP_FN_ARGS),
    int                 restart_arg
);

int  unset_sig_trap      (int sig);
void block_sig           (int sig);
void allow_sig           (int sig);
void block_atn           (void);
void allow_atn           (void);
int  block_user_signals  (void);
int  allow_user_signals  (void);
int  reset_signals       (void);
int  dont_restart_on_atn (void);
int  dont_restart_on_sig (int sig);
int  restart_on_atn      (void);
int  restart_on_sig      (int sig);
int  reset_restart_on_atn(void);
int  reset_restart_on_sig(int sig);
int  kill_myself         (int);

TRAP_FN_RETURN_TYPE record_signal      (TRAP_FN_ARGS);
TRAP_FN_RETURN_TYPE sort_atn_fn        (TRAP_FN_ARGS);
TRAP_FN_RETURN_TYPE io_atn_fn          (TRAP_FN_ARGS);
TRAP_FN_RETURN_TYPE iteration_atn_fn   (TRAP_FN_ARGS);
TRAP_FN_RETURN_TYPE yes_no_query_atn_fn(TRAP_FN_ARGS);
TRAP_FN_RETURN_TYPE confirm_exit_sig_fn(TRAP_FN_ARGS);
TRAP_FN_RETURN_TYPE default_abort_fn   (TRAP_FN_ARGS);
TRAP_FN_RETURN_TYPE safe_default_sig_fn(TRAP_FN_ARGS);
TRAP_FN_RETURN_TYPE safe_exit_fn       (TRAP_FN_ARGS);

TRAP_FN_RETURN_TYPE reset_terminal_size_on_sig_fn(TRAP_FN_ARGS);

#ifdef UNIX
   TRAP_FN_RETURN_TYPE cooked_mode_sig_fn          (TRAP_FN_ARGS);
   TRAP_FN_RETURN_TYPE raw_mode_with_no_echo_sig_fn(TRAP_FN_ARGS);
   TRAP_FN_RETURN_TYPE raw_mode_with_echo_sig_fn   (TRAP_FN_ARGS);
#endif

#ifdef UNIX
   TRAP_FN_RETURN_TYPE reset_term_before_default_sig_fn
   (
       TRAP_FN_ARGS
   );

   TRAP_FN_RETURN_TYPE raw_mode_with_no_echo_default_sig_fn
   (
       TRAP_FN_ARGS
   );
#endif

int kjb_signal
(
    int                 sig,
    TRAP_FN_RETURN_TYPE (MS_OS_USERENTRY *fn)(TRAP_FN_ARGS)
);

    int kjb_sigvec
    (
        int          sig,
        Signal_info* old_vec_ptr,
        Signal_info* cur_vec_ptr
    );
#ifdef SYSV_SIGNALS
    int kjb_sigemptyset(sigset_t* mask_ptr);
    int kjb_sigaddset  (sigset_t* mask_ptr, int sig);

    int kjb_sigprocmask
    (
        int       sig,
        sigset_t* mask_ptr,
        sigset_t* old_mask_ptr
    );
#endif    /* End of case SYSV_SIGNALS */


#ifdef TRACK_MEMORY_ALLOCATION
    void destroy_sig_queue(void);
#endif


#ifdef __cplusplus
#ifdef COMPILING_CPLUSPLUS_SOURCE
}
#endif
}
#endif

#endif

