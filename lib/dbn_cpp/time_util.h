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
| Authors:
|     Jinyan Guan
|
* =========================================================================== */

/* $Id: time_util.h 22543 2019-06-08 19:42:37Z kobus $ */

#ifndef TIES_TIME_UTIL_H
#define TIES_TIME_UTIL_H

#include <l/l_sys_def.h>
#include <l/l_sys_time.h>
#include <time.h>
#include <sys/time.h>
#include <stdio.h>

#ifdef MAC_OSX
#include <mach/clock.h>
#include <mach/mach.h>
#endif

namespace kjb {
namespace ties {
inline void get_current_time(struct timespec *ts) 
{
#ifdef MAC_OSX // OS X does not have clock_gettime, use clock_get_time
    clock_serv_t cclock;
    mach_timespec_t mts;
    host_get_clock_service(mach_host_self(), CALENDAR_CLOCK, &cclock);
    clock_get_time(cclock, &mts);
    mach_port_deallocate(mach_task_self(), cclock);
    ts->tv_sec = mts.tv_sec;
    ts->tv_nsec = mts.tv_nsec;
#else
    clock_gettime(CLOCK_MONOTONIC, ts);
#endif
}
}}
#endif
