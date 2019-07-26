
/* $Id: l_sys_tsig.c 21520 2017-07-22 15:09:04Z kobus $ */

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

#include <time.h>

#ifdef UNIX
#include <sys/time.h>
#include <setjmp.h>
#endif

#endif   /* ! MAKE_DEPEND */

#include "l/l_sys_sig.h"
#include "l/l_sys_tsig.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ------------------------------------------------------------------------- */

#ifdef UNIX
/* UNIX */  int set_time_alarm(long time_in_seconds)
/* UNIX */
/* UNIX */  {
/* UNIX */      IMPORT volatile Bool kjb_timed_out;
/* UNIX */      struct itimerval time_interval;
/* UNIX */      Signal_info new_time_vec;
/* UNIX */      int res;
/* UNIX */
/* UNIX */      time_interval.it_interval.tv_sec = 0;
/* UNIX */      time_interval.it_interval.tv_usec = 0;
/* UNIX */      time_interval.it_value.tv_sec = (int)time_in_seconds;
/* UNIX */      time_interval.it_value.tv_usec = 0;
/* UNIX */
/* UNIX */
/* UNIX */      kjb_timed_out = FALSE;
/* UNIX */
/* UNIX */      res = setitimer(ITIMER_REAL, &time_interval,
/* UNIX */                      (struct itimerval *)NULL);
/* UNIX */
/* UNIX */      if (res == -1)
/* UNIX */      {
/* UNIX */          set_bug("setitimer failed in set_time_alarm.%S");
/* UNIX */          return ERROR;
/* UNIX */      }
/* UNIX */
/* UNIX */      INIT_SIGNAL_INFO(new_time_vec);
/* UNIX */      new_time_vec.SIGNAL_HANDLER = time_out_fn;
/* UNIX */      SET_DONT_RESTART_IO_AFTER_SIGNAL(new_time_vec);
/* UNIX */
/* UNIX */      return kjb_sigvec(SIGALRM, &new_time_vec, (Signal_info*)NULL);
/* UNIX */  }
/* UNIX */
/* UNIX */
#else
/* default */  int set_time_alarm(long time_in_seconds)
/* default */  {
/* default */      IMPORT volatile Bool kjb_timed_out;
/* default */
/* default */
/* default */      kjb_timed_out = FALSE;
/* default */      set_bug("Time alarms are not supported on this system.");
/* default */
/* default */      return ERROR;
/* default */  }

#endif            /*   #ifdef UNIX .... #else  ....  */

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

#ifdef UNIX
/* UNIX */
/* UNIX */  int unset_time_alarm(void)
/* UNIX */  {
/* UNIX */      IMPORT volatile Bool kjb_timed_out;
/* UNIX */      struct itimerval time_interval;
/* UNIX */      int res;
#ifdef LINUX
/* LINUX */     sigset_t mask, save_mask;
#endif
/* UNIX */
/* UNIX */
/* UNIX */      kjb_timed_out = FALSE;
/* UNIX */
/* UNIX */      time_interval.it_interval.tv_sec = 0;
/* UNIX */      time_interval.it_interval.tv_usec = 0;
/* UNIX */      time_interval.it_value.tv_sec = 0;
/* UNIX */      time_interval.it_value.tv_usec = 0;
/* UNIX */
/* UNIX */      res = setitimer(ITIMER_REAL, &time_interval,
/* UNIX */                      (struct itimerval *)NULL);
/* UNIX */
/* UNIX */      if (res == -1)
/* UNIX */      {
/* UNIX */          set_bug("setitimer failed in unset_time_alarm.%S");
/* UNIX */          return ERROR;
/* UNIX */      }
/* UNIX */
#ifdef LINUX
                ERE(kjb_sigemptyset(&mask));
                ERE(kjb_sigaddset(&mask, SIGALRM));
                ERE(kjb_sigprocmask(SIG_UNBLOCK, &mask, &save_mask));
#endif
/* UNIX */      return kjb_signal(SIGALRM, SIG_DFL);
/* UNIX */  }
/* UNIX */
#else
/* default */
/* default */  int unset_time_alarm(void)
/* default */  {
/* default */      IMPORT volatile Bool kjb_timed_out;
/* default */
/* default */
/* default */      kjb_timed_out = FALSE;
/* default */      set_bug("Time alarms are not supported on this system.");
/* default */
/* default */      return ERROR;
/* default */  }

#endif    /*   #ifdef UNIX .... #else  ....  */

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

#ifdef UNIX

/*ARGSUSED*/ /* Usually have "sig" as "int" as first arg (always on UNIX) */
TRAP_FN_RETURN_TYPE time_out_fn( TRAP_FN_ARGS )
{
    IMPORT volatile Bool kjb_timed_out;
    IMPORT volatile int recorded_signal;


    recorded_signal = sig;


    kjb_timed_out = TRUE;
}

#else

#if 0 /* was ifdef NOT_NEEDED_YET */

/* default */
/* default */  TRAP_FN_RETURN_TYPE time_out_fn()
/* default */  {
/* default */      IMPORT volatile Bool kjb_timed_out;
/* default */
/* default */
/* default */      kjb_timed_out = TRUE;
/* default */  }
/* default */

#endif

#endif    /*   #ifdef UNIX .... #else  ....  */

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

#ifdef UNIX
/* UNIX */
/* UNIX */  int set_time_alarm_long_jump(long time_in_seconds)
/* UNIX */  {
/* UNIX */      struct itimerval time_interval;
/* UNIX */      int res;
/* UNIX */
/* UNIX */
/* UNIX */      time_interval.it_interval.tv_sec = 0;
/* UNIX */      time_interval.it_interval.tv_usec = 0;
/* UNIX */      time_interval.it_value.tv_sec = (int)time_in_seconds;
/* UNIX */      time_interval.it_value.tv_usec = 0;
/* UNIX */
/* UNIX */      res = setitimer(ITIMER_REAL, &time_interval,
/* UNIX */                      (struct itimerval *)NULL);
/* UNIX */
/* UNIX */      if (res == -1)
/* UNIX */      {
/* UNIX */          set_bug("setitimer failed in set_time_alarm_long_jump.%S");
/* UNIX */          return ERROR;
/* UNIX */      }
/* UNIX */
/* UNIX */      return kjb_signal(SIGALRM, time_out_long_jump_fn);
/* UNIX */  }
#else

#if 0 /* was ifdef NOT_NEEDED_YET */

/* default */  int set_time_alarm_long_jump(long time_in_seconds)
/* default */  {
/* default */
/* default */
/* default */  }
/* default */

#endif

#endif            /*   #ifdef UNIX .... #else  ....  */

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

#ifdef UNIX

/*ARGSUSED*/ /* Usually have "sig" as "int" as first arg (always on UNIX) */
TRAP_FN_RETURN_TYPE time_out_long_jump_fn( TRAP_FN_ARGS )
{
    IMPORT jmp_buf timer_interupt_env;
    IMPORT volatile int recorded_signal;


    recorded_signal = sig;

    longjmp(timer_interupt_env, (int)(ERROR));
}


#else

#if 0 /* was ifdef NOT_NEEDED_YET */

/* default */  /*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */
/* default */
/* default */  TRAP_FN_RETURN_TYPE time_out_long_jump_fn()
/* default */  {
/* default */
/* default */
/* default */  }
/* default */

#endif

#endif            /*   #ifdef UNIX .... #else  ....  */

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*
 * =============================================================================
 *                                  nap
 *
 * Sleep for a given number of milli-seconds
 *
 * This routine implements a milli-second sleep.
 *
 * Note:
 *     On many systems a micro-second sleep (usleep) is available, but
 *     certainly not all! Using this routine provides a degree of portability.
 *     (Provided that you don't want to sleep for less than one milli-second).
 *
 *     As always with sleeps, the time slept can be more than the requested
 *     amount.
 *
 * Returns:
 *     NO_ERROR for a successful sleep, and ERROR if it can't be done.
 *
 * Index: signals, time, standard library
 *
 * -----------------------------------------------------------------------------
 */

#ifdef UNIX
#ifdef SYSV_SIGNALS
/* SYSV */  int nap(long time_in_milliseconds)
/* SYSV */  {
/* SYSV */      sigset_t blank_mask;
/* SYSV */
/* SYSV */
/* SYSV */      ERE(kjb_sigemptyset(&blank_mask));
/* SYSV */
/* SYSV */      ERE(set_ms_time_alarm(time_in_milliseconds));
/* SYSV */
#ifdef SUN5
                sigsuspend( &blank_mask );
#else
#ifdef HPUX
                sigsuspend( &blank_mask );
#else
#ifdef LINUX
                sigsuspend( &blank_mask );
#else
#ifdef MAC_OSX
                sigsuspend( &blank_mask );
#else
                UNTESTED_CODE();
                sigsuspend( &blank_mask );
#endif
#endif
#endif
#endif
/* SYSV */
/* SYSV */      ERE(unset_time_alarm());
/* SYSV */
/* SYSV */      return NO_ERROR;
/* SYSV */  }
/* SYSV */
/* SYSV */  /*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */
/* SYSV */
#else
/* UNIX */  int nap(long time_in_milliseconds)
/* UNIX */  {
/* UNIX */
/* UNIX */
/* UNIX */      ERE(set_ms_time_alarm(time_in_milliseconds));
/* UNIX */
/* UNIX */      pause();
/* UNIX */
/* UNIX */      ERE(unset_time_alarm());
/* UNIX */
/* UNIX */      return NO_ERROR;
/* UNIX */  }
/* UNIX */
/* UNIX */  /*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */
/* UNIX */
#endif
#else
/* default */  int nap(long time_in_milliseconds)
/* default */  {
/* default */
/* default */
/* default */      set_bug("Napping is not supported on this platform.");
/* default */      return ERROR;
/* default */  }
/* default */
/* default */
#endif

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

#ifdef UNIX
/* UNIX */  int set_ms_time_alarm(long time_in_ms)
/* UNIX */  {
/* UNIX */      IMPORT volatile Bool kjb_timed_out;
/* UNIX */      Signal_info new_time_vec;
/* UNIX */      struct itimerval time_interval;
/* UNIX */      long seconds, micro_seconds;
/* UNIX */      int res;
/* UNIX */
/* UNIX */
/* UNIX */      seconds = time_in_ms / MILLI_SECONDS_PER_SECOND;
/* UNIX */
/* UNIX */      micro_seconds = 1000 * (time_in_ms % MILLI_SECONDS_PER_SECOND);
/* UNIX */
/* UNIX */      time_interval.it_interval.tv_sec = 0;
/* UNIX */      time_interval.it_interval.tv_usec = 0;
/* UNIX */      time_interval.it_value.tv_sec = seconds;
/* UNIX */      time_interval.it_value.tv_usec = micro_seconds;
/* UNIX */
/* UNIX */
/* UNIX */      kjb_timed_out = FALSE;
/* UNIX */
/* UNIX */      res = setitimer(ITIMER_REAL, &time_interval,
/* UNIX */                      (struct itimerval *)NULL);
/* UNIX */
/* UNIX */      if (res == -1)
/* UNIX */      {
/* UNIX */          set_bug("setitimer failed in set_ms_time_alarm.%S");
/* UNIX */          return ERROR;
/* UNIX */      }
/* UNIX */
/* UNIX */      INIT_SIGNAL_INFO(new_time_vec);
/* UNIX */      new_time_vec.SIGNAL_HANDLER = time_out_fn;
/* UNIX */      SET_DONT_RESTART_IO_AFTER_SIGNAL(new_time_vec);
/* UNIX */      return kjb_sigvec(SIGALRM, &new_time_vec, (Signal_info*)NULL);
/* UNIX */  }
/* UNIX */
#endif

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */


#ifdef __cplusplus
}
#endif

