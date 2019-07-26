
/* $Id: l_global.h 21520 2017-07-22 15:09:04Z kobus $ */

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

#ifndef L_GLOBAL_INCLUDED
#define L_GLOBAL_INCLUDED


#include "l/l_sys_def.h"    /* For system type. */

#ifdef UNIX
/*Some versions of makedepend cannot be trusted to honor the "-Y" option. */
#ifndef MAKE_DEPEND
#include <setjmp.h>
#endif
#endif

#include "l/l_sys_sig.h"    /* For SIGNAL_ARRAY_SIZE    */
#include "l/l_sys_err.h"    /* For default_bug_handler  */

#ifdef __cplusplus
extern "C" {
#ifdef COMPILING_CPLUSPLUS_SOURCE
namespace kjb_c {
#endif
#endif


#ifdef GLOBAL_DEF
#    undef GLOBAL_DEF
#endif

#ifdef GLOBAL_INIT
#    undef GLOBAL_INIT
#endif

#ifdef L_ALLOCATE_GLOBAL_STORAGE
#    define GLOBAL_DEF  EXPORT
#    define GLOBAL_INIT(x) x
#else
#    define GLOBAL_DEF  IMPORT
#    define GLOBAL_INIT(x)
#endif


/* Kobus---moved here from l_bits.h, so we can compile.
 * Predoehl:  unfortunately we cannot make this const int; that confuses g++.
 */
GLOBAL_DEF int kjb_endian_test GLOBAL_INIT( = 1);


/*
 * kjb_debug_level >= 2;  ---> Standard. "db" macros print via "kjb" routines.
 *
 * kjb_debug_level == 1;  ---> "db" macros print via system routines (used when
 *                             "kjb" routines can't be used - IE, when they
 *                             are being debugged!)
 *
 * kjb_debug_level == 0;  ---> Messages are blocked.
 */

#ifdef TEST
    GLOBAL_DEF int kjb_debug_level  GLOBAL_INIT( = 2 );
#else
    GLOBAL_DEF int kjb_debug_level  GLOBAL_INIT( = 0 );
#endif

#ifdef TEST
    GLOBAL_DEF int kjb_suppress_test_messages  GLOBAL_INIT( = 0 );
#endif 


/* 
 * This global currently has the single purpose of switching the action
 * of debug print statements to NOT use the library.
*/
GLOBAL_DEF int kjb_cleanup_started  GLOBAL_INIT( = 0 );

GLOBAL_DEF int kjb_fork_depth       GLOBAL_INIT( = 0 );

GLOBAL_DEF int kjb_program_major_version        GLOBAL_INIT( = 0 );
GLOBAL_DEF int kjb_program_minor_version        GLOBAL_INIT( = 0 );
GLOBAL_DEF int kjb_program_patch_level          GLOBAL_INIT( = 0 );

#ifdef TEST
#ifdef PROGRAMMER_is_kobus
GLOBAL_DEF int kjb_use_memcpy                  GLOBAL_INIT( = FALSE);
#else
GLOBAL_DEF int kjb_use_memcpy                  GLOBAL_INIT( = TRUE);
#endif 
#else
GLOBAL_DEF int kjb_use_memcpy                  GLOBAL_INIT( = TRUE);
#endif 

#ifdef TEST
    GLOBAL_DEF int abort_interactive_math_errors GLOBAL_INIT( = TRUE);
#else
    GLOBAL_DEF int abort_interactive_math_errors GLOBAL_INIT( = FALSE);
#endif

/* Shared by l_sys_err.c and l_sys_sig.c */
GLOBAL_DEF void (*kjb_bug_handler)(const char*) GLOBAL_INIT( = default_bug_handler );

GLOBAL_DEF volatile Bool halt_all_output        GLOBAL_INIT( = FALSE );
GLOBAL_DEF volatile Bool halt_term_output       GLOBAL_INIT( = FALSE );

/*
// This was introduced so that we can do our own line wrapping when needed, but
// I don't think there is currently a case where we have to do this. If
// term_line_wrap_flag is set to TRUE, then the pager code will insert returns
// to wrap the lines.
*/
GLOBAL_DEF volatile Bool term_line_wrap_flag    GLOBAL_INIT( = FALSE );

GLOBAL_DEF volatile Bool term_no_block_flag     GLOBAL_INIT( = FALSE );
GLOBAL_DEF volatile int num_term_lines         GLOBAL_INIT( = 0 );
GLOBAL_DEF volatile int num_term_chars         GLOBAL_INIT( = 0 );
GLOBAL_DEF volatile Bool pause_on_next          GLOBAL_INIT( = FALSE );
GLOBAL_DEF volatile int kjb_tty_rows           GLOBAL_INIT( = 24 );
GLOBAL_DEF volatile int kjb_tty_cols           GLOBAL_INIT( = 80 );

GLOBAL_DEF volatile int term_io_since_last_input_attempt GLOBAL_INIT( = FALSE );

#ifdef UNIX
    GLOBAL_DEF jmp_buf timer_interupt_env;
#endif

GLOBAL_DEF volatile Bool kjb_timed_out GLOBAL_INIT( = FALSE );

GLOBAL_DEF volatile int recorded_signal;

/*
// We make use of the fact that globals will be set to 0, which in this case,
// corresponds to restarting IO after a signal.
*/
GLOBAL_DEF int restart_io_after_sig [ SIGNAL_ARRAY_SIZE ];

GLOBAL_DEF volatile Bool io_atn_flag                GLOBAL_INIT( = FALSE );
GLOBAL_DEF volatile Bool iteration_atn_flag         GLOBAL_INIT( = FALSE );
GLOBAL_DEF volatile Bool sort_atn_flag              GLOBAL_INIT( = FALSE );

GLOBAL_DEF int kjb_comment_char                    GLOBAL_INIT( = '#' );
GLOBAL_DEF int kjb_header_char                     GLOBAL_INIT( = '!' );


#undef GLOBAL_DEF
#undef GLOBAL_INIT


#ifdef __cplusplus
#ifdef COMPILING_CPLUSPLUS_SOURCE
}
#endif
}
#endif

#endif


