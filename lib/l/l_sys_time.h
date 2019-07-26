
/* $Id: l_sys_time.h 4899 2009-11-28 20:50:49Z kobus $ */

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

#ifndef L_SYS_TIME_INCLUDED
#define L_SYS_TIME_INCLUDED


#include "l/l_def.h"

#ifdef __cplusplus
extern "C" {
#ifdef COMPILING_CPLUSPLUS_SOURCE
namespace kjb_c {
#endif
#endif


/* -------------------------------------------------------------------------- */

/* =============================================================================
 *                              BUFF_GET_TIME
 *
 * (MACRO) Sets up call to get_time
 *
 * The max_len parameter required by get_time is set to sizeof(time_buff).
 * Using sizeof to set the buffer size is recomended where applicable, as the
 * code will not be broken if the buffer size changes. HOWEVER, neither this
 * method, nor the macro, is applicable if line is NOT a character array.  If
 * time_buff is declared by "char *time_buff", then the size of line is the
 * number of bytes in a character pointer (usually 4), which is NOT what is
 * normally intended.  You have been WARNED!
 *
 * Index: time
 *
 * -----------------------------------------------------------------------------
*/

#ifdef __C2MAN__    /* Pretend we are a ".c" file to document MACROS. */
     int BUFF_GET_TIME(char);
     int BUFF_GET_TIME_2(char*, const char*);
#endif

#ifndef __C2MAN__  /* Just doing "else" confuses ctags for the first such line.
                      I have no idea what the problem is. */
#    define BUFF_GET_TIME(x)     get_time(x, sizeof(x))
#    define BUFF_GET_TIME_2(x,y) get_time_2(x, sizeof(x),y)
#endif


#define TIME_ERROR  ((time_t) -1)

int  init_cpu_time    (void);
int  display_cpu_time (void);
long get_cpu_time     (void);
long get_cpu_time_2   (void);
int  init_real_time   (void);
int  display_real_time(void);
long get_real_time    (void);
int get_time  (char* time_buff, size_t max_len);
int get_time_2(char* time_buff, size_t max_len, const char* format_str);


#ifdef __cplusplus
#ifdef COMPILING_CPLUSPLUS_SOURCE
}
#endif
}
#endif

#endif

