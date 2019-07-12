
/* $Id: l_sys_time.c 14531 2013-05-28 05:39:18Z predoehl $ */

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
#include "l/l_sys_time.h"

/*
// FIX
//
// This file contains a messy combo of new and obsolete time functions. Should
// be cleaned up soon!
*/

#ifdef UNIX
#ifdef SYSV

#include <sys/times.h>

#else  /* ! SYSV */

#include <sys/time.h>
#include <sys/resource.h>
#include <sys/timeb.h>

#endif  /* SYSV ELSE not SYSV */

#endif   /* UNIX */

#ifdef MS_OS
#    include <sys/timeb.h>
#    include <time.h>

#    ifdef __STDC__
#        define timeb _timeb
#        define ftime _ftime
#    endif 

#endif

/* -------------------------------------------------------------------------- */

#ifdef __cplusplus
extern "C" {
#endif

/* -------------------------------------------------------------------------- */

#ifdef  SUN
/* Following   /usr/lib/lint/llib-lc     */
/*
time_t  time();
long    clock();
*/
#endif  /* SUN */

#ifdef NeXT
extern int ftime(struct timeb* tp);
#endif

#ifdef SYSV
#ifdef __GNUC__
extern long _sysconf(int);
#endif
#endif  /* SYSV */


/* What? ANSI standard enviroment? Well we still need this. */
#ifdef CLK_TCK
#    undef CLK_TCK
#endif

#define CLK_TCK (sysconf(_SC_CLK_TCK))   /* 3B2 clock ticks per second */


#endif   /* UNIX */


#ifdef UNIX
#ifdef SYSV

static clock_t fs_real_time_base;
static clock_t fs_user_time_base;
static clock_t fs_sys_time_base;

#else  /* ! SYSV */

static struct timeb fs_real_time_base;
static struct timeval fs_user_time_base;
static struct timeval fs_sys_time_base;

#endif   /* ! SYSV */
#endif  /* UNIX */

/* -------------------------------------------------------------------------- */

#ifdef UNIX
#ifndef SYSV
static int kjb_ftime(struct timeb* cur_time_ptr);
#endif
#endif

/* -------------------------------------------------------------------------- */

/* =============================================================================
 *                              init_cpu_time
 *
 * Resets the time base for cpu time
 *
 * Resets the time base used by get_cpu_time and display_cpu_time
 *
 * Index: time
 *
 * -----------------------------------------------------------------------------
*/

#ifdef UNIX
#ifdef SYSV
/* SYSV */    int init_cpu_time(void)
/* SYSV */    {
/* SYSV */        struct tms usage_stats;
/* SYSV */
/* SYSV */
#ifdef HPUX
/* SYSV */        /* On HPUX clock_t is defined to be unsigned, and yet      */
/* SYSV */        /* the documentation claim times it returns -1 on failure. */
/* SYSV */        /* I don't know if it will or not, but if it does, then    */
/* SYSV */        /* we should cast it for the compare.                      */
/* SYSV */
/* SYSV */        if ((long)times(&usage_stats) == -1)
#else
/* SYSV */        if (times(&usage_stats) == -1)
#endif
/* SYSV */        {
#ifdef TEST
/* SYSV */            set_error("TimeS failed.");
#else
/* sys */             set_error("System CPU time is not available.");
#endif
/* SYSV */            return ERROR;
/* SYSV */        }
/* SYSV */        else
/* SYSV */        {
/* SYSV */            fs_user_time_base = usage_stats.tms_utime;
/* SYSV */            fs_sys_time_base = usage_stats.tms_stime;
/* SYSV */
/* SYSV */            return NO_ERROR;
/* SYSV */        }
/* SYSV */    }
/* SYSV */
#else
/* ! SYSV */  int init_cpu_time(void)
/* ! SYSV */  {
/* ! SYSV */      struct rusage usage_stats;
/* ! SYSV */
/* ! SYSV */
/* ! SYSV */      if (getrusage(RUSAGE_SELF, &usage_stats) == EOF)
/* ! SYSV */      {
#ifdef TEST
/* ! SYSV */          set_error("Getrusage failed.");
#else
/* ! SYSV */          set_error("System call failure.");
#endif
/* ! SYSV */          return ERROR;
/* ! SYSV */      }
/* ! SYSV */      else
/* ! SYSV */      {
/* ! SYSV */          fs_user_time_base = usage_stats.ru_utime;
/* ! SYSV */          fs_sys_time_base = usage_stats.ru_stime;
/* ! SYSV */
/* ! SYSV */          return NO_ERROR;
/* ! SYSV */      }
/* ! SYSV */  }
#endif
#else
/* default */ int init_cpu_time(void)
/* default */ {
/* default */
/* default */
/* default */     set_bug("CPU time report not implemented on this platform.");
/* default */     return ERROR;
/* default */ }
#endif

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                              get_cpu_time
 *
 * Returns the cpu time in milliseconds since last reset
 *
 * Returns the cpu time used in milliseconds since last call to init_cpu_time.
 *
 * Index: time
 *
 * -----------------------------------------------------------------------------
*/

#ifdef UNIX
#ifdef SYSV
/* SYSV */    long get_cpu_time(void)
/* SYSV */    {
/* SYSV */        struct tms usage_stats;
/* SYSV */        clock_t user_time, sys_time;
/* SYSV */        clock_t cpu_time;
/* SYSV */
/* SYSV */
#ifdef HPUX
/* SYSV */        /* On HPUX clock_t is defined to be unsigned, and yet      */
/* SYSV */        /* the documentation claim times it returns -1 on failure. */
/* SYSV */        /* I don't know if it will or not, but if it does, then    */
/* SYSV */        /* we should cast it for the compare.                      */
/* SYSV */
/* SYSV */        if ((long)times(&usage_stats) == -1)
#else
/* SYSV */        if (times(&usage_stats) == -1)
#endif
/* SYSV */        {
#ifdef TEST
/* SYSV */            set_error("TimeS failed.");
#else
/* sys */             set_error("System CPU time is not available.");
#endif
/* SYSV */            return ERROR;
/* SYSV */        }
/* SYSV */        else
/* SYSV */        {
/* SYSV */            user_time = usage_stats.tms_utime-fs_user_time_base;
/* SYSV */            sys_time  = usage_stats.tms_stime-fs_sys_time_base;
/* SYSV */            cpu_time  = user_time + sys_time;

/* SYSV */            return (cpu_time * MILLI_SECONDS_PER_SECOND)/CLK_TCK;
/* SYSV */        }
/* SYSV */    }
#else
/* ! SYSV */  long get_cpu_time(void)
/* ! SYSV */  {
/* ! SYSV */      struct rusage usage_stats;
/* ! SYSV */      long user_secs, user_milli_secs, user_extra_micro_secs;
/* ! SYSV */      long sys_secs, sys_milli_secs, sys_extra_micro_secs;
/* ! SYSV */
/* ! SYSV */
/* ! SYSV */      if (getrusage(RUSAGE_SELF, &usage_stats) == EOF)
/* ! SYSV */      {
#ifdef TEST
/* ! SYSV */          set_error("Getrusage failed.");
#else
/* ! SYSV */          set_error("System call failure.");
#endif
/* ! SYSV */          return ERROR;
/* ! SYSV */      }
/* ! SYSV */      else
/* ! SYSV */      {
/* ! SYSV */          user_secs = usage_stats.ru_utime.tv_sec -
/* ! SYSV */                                         fs_user_time_base.tv_sec;
/* ! SYSV */
/* ! SYSV */          user_milli_secs = user_secs * MILLI_SECONDS_PER_SECOND;
/* ! SYSV */
/* ! SYSV */          user_extra_micro_secs = usage_stats.ru_utime.tv_usec -
/* ! SYSV */                                         fs_user_time_base.tv_usec;
/* ! SYSV */
/* ! SYSV */          user_milli_secs += (user_extra_micro_secs / (int)1000);
/* ! SYSV */
/* ! SYSV */          sys_secs = usage_stats.ru_stime.tv_sec -
/* ! SYSV */                                         fs_sys_time_base.tv_sec;
/* ! SYSV */
/* ! SYSV */          sys_milli_secs = sys_secs * MILLI_SECONDS_PER_SECOND;
/* ! SYSV */
/* ! SYSV */          sys_extra_micro_secs = usage_stats.ru_stime.tv_usec -
/* ! SYSV */                                         fs_sys_time_base.tv_usec;
/* ! SYSV */
/* ! SYSV */          sys_milli_secs += (sys_extra_micro_secs / (int)1000);
/* ! SYSV */
/* ! SYSV */          return (user_milli_secs + sys_milli_secs);
/* ! SYSV */      }
/* ! SYSV */  }
#endif

#else
/* default */ long get_cpu_time(void)
/* default */ {
/* default */
/* default */
/* default */     set_bug("CPU time report not implemented on this platform.");
/* default */     return ERROR;
/* default */ }
#endif

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                              get_cpu_time_2
 *
 * Returns the cpu time in milliseconds
 *
 * Returns the cpu time used in milliseconds. This routine differs from
 * get_cpu_time() in that the time at the last reset is not subracted off. Thus
 * this routine is used when computing cpu usage when routines called by the
 * code been measured may reset the base time.
 *
 * Index: time
 *
 * -----------------------------------------------------------------------------
*/

#ifdef UNIX
#ifdef SYSV
/* SYSV */    long get_cpu_time_2(void)
/* SYSV */    {
/* SYSV */        struct tms usage_stats;
/* SYSV */        clock_t user_time, sys_time;
/* SYSV */        clock_t cpu_time;
/* SYSV */
/* SYSV */
#ifdef HPUX
/* SYSV */        /* On HPUX clock_t is defined to be unsigned, and yet      */
/* SYSV */        /* the documentation claim times it returns -1 on failure. */
/* SYSV */        /* I don't know if it will or not, but if it does, then    */
/* SYSV */        /* we should cast it for the compare.                      */
/* SYSV */
/* SYSV */        if ((long)times(&usage_stats) == -1)
#else
/* SYSV */        if (times(&usage_stats) == -1)
#endif
/* SYSV */        {
#ifdef TEST
/* SYSV */            set_error("TimeS failed.");
#else
/* sys */             set_error("System CPU time is not available.");
#endif
/* SYSV */            return ERROR;
/* SYSV */        }
/* SYSV */        else
/* SYSV */        {
/* SYSV */            user_time = usage_stats.tms_utime;
/* SYSV */            sys_time  = usage_stats.tms_stime;
/* SYSV */            cpu_time  = user_time + sys_time;

/* SYSV */            return (cpu_time * MILLI_SECONDS_PER_SECOND)/CLK_TCK;
/* SYSV */        }
/* SYSV */    }
#else
/* ! SYSV */  long get_cpu_time_2(void)
/* ! SYSV */  {
/* ! SYSV */      struct rusage usage_stats;
/* ! SYSV */      long user_secs, user_milli_secs, user_extra_micro_secs;
/* ! SYSV */      long sys_secs, sys_milli_secs, sys_extra_micro_secs;
/* ! SYSV */
/* ! SYSV */
/* ! SYSV */      if (getrusage(RUSAGE_SELF, &usage_stats) == EOF)
/* ! SYSV */      {
#ifdef TEST
/* ! SYSV */          set_error("Getrusage failed.");
#else
/* ! SYSV */          set_error("System call failure.");
#endif
/* ! SYSV */          return ERROR;
/* ! SYSV */      }
/* ! SYSV */      else
/* ! SYSV */      {
/* ! SYSV */          user_secs = usage_stats.ru_utime.tv_sec;
/* ! SYSV */
/* ! SYSV */          user_milli_secs = user_secs * MILLI_SECONDS_PER_SECOND;
/* ! SYSV */
/* ! SYSV */          user_extra_micro_secs = usage_stats.ru_utime.tv_usec;
/* ! SYSV */
/* ! SYSV */          user_milli_secs += (user_extra_micro_secs / (int)1000);
/* ! SYSV */
/* ! SYSV */          sys_secs = usage_stats.ru_stime.tv_sec;
/* ! SYSV */
/* ! SYSV */          sys_milli_secs = sys_secs * MILLI_SECONDS_PER_SECOND;
/* ! SYSV */
/* ! SYSV */          sys_extra_micro_secs = usage_stats.ru_stime.tv_usec;
/* ! SYSV */
/* ! SYSV */          sys_milli_secs += (sys_extra_micro_secs / (int)1000);
/* ! SYSV */
/* ! SYSV */          return (user_milli_secs + sys_milli_secs);
/* ! SYSV */      }
/* ! SYSV */  }
#endif

#else
/* default */ long get_cpu_time_2(void)
/* default */ {
/* default */
/* default */
/* default */     set_bug("CPU time report not implemented on this platform.");
/* default */     return ERROR;
/* default */ }
#endif

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                              display_cpu_time
 *
 * Prints the cpu time in milliseconds since last reset
 *
 * Prints cpu time used in milliseconds since last call to init_cpu_time on
 * standard out. The cpu time is broken down into user and system cpu time.
 *
 * Index: time
 *
 * -----------------------------------------------------------------------------
*/

#ifdef UNIX

#ifdef SYSV
/* SYSV */    int display_cpu_time(void)
/* SYSV */    {
/* SYSV */        struct tms usage_stats;
/* SYSV */        clock_t user_time, sys_time;
/* SYSV */        long user_milli_secs, sys_milli_secs, total_milli_secs;
/* SYSV */
/* SYSV */
#ifdef HPUX
/* SYSV */        /* On HPUX clock_t is defined to be unsigned, and yet      */
/* SYSV */        /* the documentation claim times it returns -1 on failure. */
/* SYSV */        /* I don't know if it will or not, but if it does, then    */
/* SYSV */        /* we should cast it for the compare.                      */
/* SYSV */
/* SYSV */        if ((long)times(&usage_stats) == -1)
#else
/* SYSV */        if (times(&usage_stats) == -1)
#endif
/* SYSV */        {
#ifdef TEST
/* SYSV */            set_error("TimeS failed.");
#else
/* sys */             set_error("System CPU time is not available.");
#endif
/* SYSV */            return ERROR;
/* SYSV */        }
/* SYSV */        else
/* SYSV */        {
/* SYSV */            user_time = usage_stats.tms_utime-fs_user_time_base;
/* SYSV */
/* SYSV */            user_milli_secs = (user_time * MILLI_SECONDS_PER_SECOND) /
/* SYSV */                                                              CLK_TCK;
/* SYSV */
/* SYSV */            pso("User level CPU time since last reset is %ld.%03lds; ",
/* SYSV */                (long)(user_milli_secs / MILLI_SECONDS_PER_SECOND),
/* SYSV */                (long)(user_milli_secs % MILLI_SECONDS_PER_SECOND));
/* SYSV */
/* SYSV */            sys_time = usage_stats.tms_stime-fs_sys_time_base;
/* SYSV */
/* SYSV */            sys_milli_secs = (sys_time * MILLI_SECONDS_PER_SECOND) /
/* SYSV */                                                              CLK_TCK;
/* SYSV */
/* SYSV */            pso("System CPU is %ld.%03lds; ",
/* SYSV */                (long)(sys_milli_secs / MILLI_SECONDS_PER_SECOND),
/* SYSV */                (long)(sys_milli_secs % MILLI_SECONDS_PER_SECOND));
/* SYSV */
/* SYSV */            total_milli_secs = user_milli_secs + sys_milli_secs;
/* SYSV */
/* SYSV */            pso("Total CPU is %ld.%03lds\n",
/* SYSV */                (long)(total_milli_secs / MILLI_SECONDS_PER_SECOND),
/* SYSV */                (long)(total_milli_secs % MILLI_SECONDS_PER_SECOND));
/* SYSV */
/* SYSV */            return NO_ERROR;
/* SYSV */        }
/* SYSV */    }
#else
/* ! SYSV */  int display_cpu_time(void)
/* ! SYSV */  {
/* ! SYSV */      struct rusage usage_stats;
/* ! SYSV */      long user_secs, user_milli_secs, user_extra_micro_secs;
/* ! SYSV */      long sys_secs, sys_milli_secs, sys_extra_micro_secs;
/* ! SYSV */      long total_milli_secs;
/* ! SYSV */
/* ! SYSV */
/* ! SYSV */      if (getrusage(RUSAGE_SELF, &usage_stats) == EOF)
/* ! SYSV */      {
#ifdef TEST
/* ! SYSV */          set_error("Getrusage failed.");
#else
/* ! SYSV */          set_error("System call failure.");
#endif
/* ! SYSV */          return ERROR;
/* ! SYSV */      }
/* ! SYSV */      else
/* ! SYSV */      {
/* ! SYSV */          user_secs = usage_stats.ru_utime.tv_sec -
/* ! SYSV */                                         fs_user_time_base.tv_sec;
/* ! SYSV */
/* ! SYSV */          user_milli_secs = user_secs * MILLI_SECONDS_PER_SECOND;
/* ! SYSV */
/* ! SYSV */          user_extra_micro_secs = usage_stats.ru_utime.tv_usec -
/* ! SYSV */                                         fs_user_time_base.tv_usec;
/* ! SYSV */
/* ! SYSV */          user_milli_secs += (user_extra_micro_secs / (int)1000);
/* ! SYSV */
/* ! SYSV */
/* ! SYSV */          pso("User level CPU time since last reset is %ld.%03lds; ",
/* ! SYSV */              ((long)user_milli_secs / MILLI_SECONDS_PER_SECOND),
/* ! SYSV */              ((long)user_milli_secs % MILLI_SECONDS_PER_SECOND));
/* ! SYSV */
/* ! SYSV */          sys_secs = usage_stats.ru_stime.tv_sec -
/* ! SYSV */                                         fs_sys_time_base.tv_sec;
/* ! SYSV */
/* ! SYSV */          sys_milli_secs = sys_secs * MILLI_SECONDS_PER_SECOND;
/* ! SYSV */
/* ! SYSV */          sys_extra_micro_secs = usage_stats.ru_stime.tv_usec -
/* ! SYSV */                                           fs_sys_time_base.tv_usec;
/* ! SYSV */
/* ! SYSV */          sys_milli_secs += (sys_extra_micro_secs / (int)1000);
/* ! SYSV */
/* ! SYSV */          pso("System CPU is %ld.%03lds; ",
/* ! SYSV */              ((long)sys_milli_secs / MILLI_SECONDS_PER_SECOND),
/* ! SYSV */              ((long)sys_milli_secs % MILLI_SECONDS_PER_SECOND));
/* ! SYSV */
/* ! SYSV */          total_milli_secs = user_milli_secs + sys_milli_secs;
/* ! SYSV */
/* ! SYSV */          pso("Total CPU is %ld.%03lds\n",
/* ! SYSV */              ((long)total_milli_secs / MILLI_SECONDS_PER_SECOND),
/* ! SYSV */              ((long)total_milli_secs % MILLI_SECONDS_PER_SECOND));
/* ! SYSV */
/* ! SYSV */          return NO_ERROR;
/* ! SYSV */      }
/* ! SYSV */  }
#endif

#else
/* default */ int display_cpu_time(void)
/* default */ {
/* default */
/* default */
/* default */     set_bug("CPU time report not implemented on this platform.");
/* default */     return ERROR;
/* default */ }
#endif

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                              init_real_time
 *
 * Resets the time base for real time
 *
 * Resets the time base used by get_real_time and display_real_time.
 * Returns ERROR or NO_ERROR as appropriate.
 *
 * Index: time
 *
 * -----------------------------------------------------------------------------
*/

#ifdef UNIX
#ifdef SYSV
/* SYSV */    int init_real_time(void)
/* SYSV */    {
/* SYSV */        struct tms dummy_tms;
/* SYSV */        clock_t temp_time;
/* SYSV */
/* SYSV */
#ifdef HPUX
/* SYSV */        /* On HPUX clock_t is defined to be unsigned, and yet      */
/* SYSV */        /* the documentation claim times it returns -1 on failure. */
/* SYSV */        /* I don't know if it will or not, but if it does, then    */
/* SYSV */        /* we should cast it for the compare.                      */
/* SYSV */
/* SYSV */        if ((long)(temp_time = times(&dummy_tms)) == -1)
#else
/* SYSV */        if ((temp_time = times(&dummy_tms)) == EOF)
#endif
/* SYSV */        {
#ifdef TEST
/* SYSV */            set_error("TimeS failed.");
#else
/* sys */             set_error("System CPU time is not available.");
#endif
/* SYSV */            return ERROR;
/* SYSV */        }
/* SYSV */        else
/* SYSV */        {
/* SYSV */            fs_real_time_base = temp_time;
/* SYSV */            return NO_ERROR;
/* SYSV */        }
/* SYSV */    }
#else
/* ! SYSV */   int init_real_time(void)
/* ! SYSV */   {
/* ! SYSV */       struct timeb temp_time;
/* ! SYSV */
/* ! SYSV */
/* ! SYSV */       if (kjb_ftime( &temp_time) == ERROR)
/* ! SYSV */       {
/* ! SYSV */           set_bug("Ftime failed.");
/* ! SYSV */           return ERROR;
/* ! SYSV */       }
/* ! SYSV */       else
/* ! SYSV */       {
/* ! SYSV */           fs_real_time_base = temp_time;
/* ! SYSV */           return NO_ERROR;
/* ! SYSV */       }
/* ! SYSV */   }
#endif
#else
/* default */ int init_real_time(void)
/* default */ {
/* default */
/* default */
/* default */     set_bug("Real time report not implemented on this platform.");
/* default */     return ERROR;
/* default */ }
#endif

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                              get_real_time
 *
 * Returns the real time in milliseconds since last reset
 *
 * Returns the real time used in milliseconds since last call to init_real_time.
 *
 * Index: time
 *
 * -----------------------------------------------------------------------------
*/

#ifdef UNIX
#ifdef SYSV
/* SYSV */    long get_real_time(void)
/* SYSV */    {
/* SYSV */        struct tms dummy_tms;
/* SYSV */        clock_t temp_time, elapsed_time;
/* SYSV */
#ifdef HPUX
/* SYSV */        /* On HPUX clock_t is defined to be unsigned, and yet      */
/* SYSV */        /* the documentation claim times it returns -1 on failure. */
/* SYSV */        /* I don't know if it will or not, but if it does, then    */
/* SYSV */        /* we should cast it for the compare.                      */
/* SYSV */
/* SYSV */        if ((long)(temp_time = times(&dummy_tms)) == -1)
#else
/* SYSV */        if ((temp_time = times(&dummy_tms)) == EOF)
#endif
/* SYSV */        {
#ifdef TEST
/* SYSV */            set_error("TimeS failed.");
#else
/* sys */             set_error("System real time is not available.");
#endif
/* SYSV */            return ERROR;
/* SYSV */        }
/* SYSV */        else
/* SYSV */        {
/* SYSV */            elapsed_time =  temp_time - fs_real_time_base;
/* SYSV */            return (MILLI_SECONDS_PER_SECOND*elapsed_time) / CLK_TCK;
/* SYSV */        }
/* SYSV */    }
#else
/* ! SYSV */  long get_real_time(void)
/* ! SYSV */  {
/* ! SYSV */      struct timeb end_time;
/* ! SYSV */      long secs, milli_secs;
/* ! SYSV */
/* ! SYSV */
/* ! SYSV */      if (kjb_ftime( &end_time) == ERROR)
/* ! SYSV */      {
#ifdef TEST
/* ! SYSV */          set_bug("Ftime failed.");
#else
/* not sys */          set_error("System real time is not available.");
#endif
/* ! SYSV */          return ERROR;
/* ! SYSV */      }
/* ! SYSV */      else
/* ! SYSV */      {
/* ! SYSV */          milli_secs = end_time.millitm -
/* ! SYSV */                                   fs_real_time_base.millitm;
/* ! SYSV */
/* ! SYSV */          secs = end_time.time - fs_real_time_base.time;
/* ! SYSV */
/* ! SYSV */          return (secs * MILLI_SECONDS_PER_SECOND) + milli_secs;
/* ! SYSV */      }
/* ! SYSV */  }
#endif
#else
/* default */ long get_real_time(void)
/* default */ {
/* default */
/* default */
/* default */     set_bug("Real time report not implemented on this platform.");
/* default */     return ERROR;
/* default */ }
#endif

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                              display_real_time
 *
 * Prints the real time in milliseconds since last reset
 *
 * Prints real time used in milliseconds since last call to init_real_time on
 * standard out.
 *
 * Index: time
 *
 * -----------------------------------------------------------------------------
*/

#ifdef UNIX
#ifdef SYSV
/* SYSV */    int display_real_time(void)
/* SYSV */    {
/* SYSV */        struct tms dummy_tms;
/* SYSV */        clock_t temp_time, elapsed_time;
/* SYSV */        long total_milli_secs;
/* SYSV */
/* SYSV */
#ifdef HPUX
/* SYSV */        /* On HPUX clock_t is defined to be unsigned, and yet      */
/* SYSV */        /* the documentation claim times it returns -1 on failure. */
/* SYSV */        /* I don't know if it will or not, but if it does, then    */
/* SYSV */        /* we should cast it for the compare.                      */
/* SYSV */
/* SYSV */        if ((long)(temp_time = times(&dummy_tms)) == -1)
#else
/* SYSV */        if ((temp_time = times(&dummy_tms)) == EOF)
#endif
/* SYSV */        {
#ifdef TEST
/* SYSV */            set_error("TimeS failed.");
#else
/* sys */             set_error("System real time is not available.");
#endif
/* SYSV */            return ERROR;
/* SYSV */        }
/* SYSV */        else
/* SYSV */        {
/* SYSV */           elapsed_time =  temp_time - fs_real_time_base;
/* SYSV */           total_milli_secs =
/* SYSV */                    (MILLI_SECONDS_PER_SECOND*elapsed_time) / CLK_TCK;
/* SYSV */
/* SYSV */           pso("REAL time since last reset is %ld.%03lds\n",
/* SYSV */               ((long)total_milli_secs / MILLI_SECONDS_PER_SECOND),
/* SYSV */               ((long)total_milli_secs % MILLI_SECONDS_PER_SECOND));
/* SYSV */
/* SYSV */
/* SYSV */           return NO_ERROR;
/* SYSV */       }
/* SYSV */   }
#else
/* ! SYSV */  int display_real_time(void)
/* ! SYSV */  {
/* ! SYSV */      struct timeb end_time;
/* ! SYSV */      long secs, milli_secs, total_milli_secs;
/* ! SYSV */
/* ! SYSV */
/* ! SYSV */
/* ! SYSV */      if (kjb_ftime( &end_time) == ERROR)
/* ! SYSV */      {
#ifdef TEST
/* ! SYSV */          set_bug("Ftime failed.");
#else
/* not sys */          set_error("System real time is not available.");
#endif
/* ! SYSV */          return ERROR;
/* ! SYSV */      }
/* ! SYSV */      else
/* ! SYSV */      {
/* ! SYSV */          milli_secs = end_time.millitm -
/* ! SYSV */                                   fs_real_time_base.millitm;
/* ! SYSV */
/* ! SYSV */          secs = end_time.time - fs_real_time_base.time;
/* ! SYSV */
/* ! SYSV */          secs += milli_secs / MILLI_SECONDS_PER_SECOND;
/* ! SYSV */          total_milli_secs = milli_secs +
/* ! SYSV */                             secs * MILLI_SECONDS_PER_SECOND;
/* ! SYSV */
/* ! SYSV */          pso("REAL time since last reset is %ld.%03lds\n",
/* ! SYSV */              ((long)total_milli_secs / MILLI_SECONDS_PER_SECOND),
/* ! SYSV */              ((long)total_milli_secs % MILLI_SECONDS_PER_SECOND));
/* ! SYSV */
/* ! SYSV */          return NO_ERROR;
/* ! SYSV */      }
/* ! SYSV */  }
#endif
#else
/* default */ int display_real_time(void)
/* default */ {
/* default */
/* default */
/* default */     set_bug("Real time report not implemented on this platform.");
/* default */     return ERROR;
/* default */ }
#endif

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                              get_time
 *
 * Gets the current time as a string.
 *
 * This routine puts the current time into the buffer time_buff of size max_len.
 * The size must be at least 25.
 *
 * Related:
 *     get_time_2, BUFF_GET_TIME, BUFF_GET_TIME_2
 *
 * Index: time
 *
 * -----------------------------------------------------------------------------
*/

int get_time(char* time_buff, size_t max_len)
{
    return get_time_2(time_buff, max_len, (const char*) NULL);
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                              get_time_2
 *
 * Gets the current time as a string.
 *
 * This routine puts the current time into the buffer time_buff of size max_len.
 * The size must be at least 25. It supports the standard time formatting
 * options (see strftime(3)) via the argument format_str. If format_str is NULL,
 * then the format from ctime is used (same behaviour as get_time()).
 *
 * Related:
 *     BUFF_GET_TIME_2
 *
 * Index: time
 *
 * -----------------------------------------------------------------------------
*/

int get_time_2(char* time_buff, size_t max_len, const char* format_str)
{
    time_t current_time;
    char*  time_string;


    if (max_len <= 26)
    {
        SET_BUFFER_OVERFLOW_BUG();
        return ERROR;
    }

    if (time(&current_time) == TIME_ERROR) return ERROR;

    if (format_str == NULL)
    {
        time_string = ctime(&current_time);
        strcpy(time_buff, time_string);
        if (time_buff[ 24 ] == '\n') time_buff[ 24 ] = '\0';
    }
    else
    {
        struct tm current_tm;
        struct tm * current_tm_ptr = &current_tm;

#ifdef MS_OS
        if ((current_tm_ptr = localtime(&current_time)) == NULL)
#else 
        if (localtime_r(&current_time, current_tm_ptr) == NULL)
#endif 
        {
            SET_CANT_HAPPEN_BUG();
            return ERROR;
        }

        (void) strftime(time_buff, max_len, format_str, &current_tm);
    }

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

#ifdef UNIX
#ifndef SYSV

static int kjb_ftime(struct timeb* cur_time_ptr)
{
#ifdef MAC_OSX
    struct timeval tp;

    if (gettimeofday(&tp, (struct timezone*)NULL) == EOF)
    {
        set_error("Call to gettimeofday failed.%S.");
        return ERROR;
    }
    else
    {
        cur_time_ptr->time = tp.tv_sec;
        cur_time_ptr->millitm = tp.tv_usec / 1000;
        return NO_ERROR;
    }
#else
    if (ftime(cur_time_ptr) == EOF)
    {
        return ERROR;
    }
    else
    {
        set_error("Call to ftime failed.%S.");
        return ftime(cur_time_ptr);
    }
#endif
}

#endif
#endif

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

#ifdef __cplusplus
}
#endif

