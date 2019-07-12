
/* $Id: l_init.c 22504 2019-06-03 22:02:14Z kobus $ */

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
#include "l/l_sys_scan.h"
#include "l/l_init.h"

#ifdef LINUX_X86
#ifdef TEST
#ifndef MAKE_DEPEND
#include <fpu_control.h>
#endif
#endif
#endif

#ifdef __cplusplus
extern "C" {
#endif

/* -------------------------------------------------------------------------- */

static int fs_trap_fpu_exceptions = NOT_SET;

/* -------------------------------------------------------------------------- */

#ifdef TEST
static void check_defines(void);
#endif 

/* -------------------------------------------------------------------------- */

/*
 * =============================================================================
 *                                  kjb_init
 *
 * Performs some KJB library initialization
 *
 * This routine performs some KJB library initialization. As far as possible, I
 * try to minimize the reliance of the library routines on any initialization.
 * Thus mall routines should work in some fashion if this routine is Hence this
 * routine is not actually needed.  However, it does provide a few facilities
 * which are recomended, and can only be ecomically be provided near program
 * startup. It is also desirable to have a hook for future developments which
 * may require some additional initialization.
 *
 * Some details as to the current effects are provided here so that callers can
 * decide if they are better off without some subset of the effects, and proceed
 * acordingly. Currently we: Start a process to handle system routines. Doing
 * it this way reduces forking overhead on many systems.  2) On some systems, we
 * request that floating point exceptions be trapped.  3) We set up some signal
 * traps. Some routines put the terminal into a state which needs to be reset
 * before exit. This is arranged for normal exits, but if the program is
 * terminated by a signal, then it is best to clean up first.  Otherwise the
 * user can use a CTL-C to kill a program, and then be left a terminal that is
 * hard to comunicate with (whether this actually occurs depends on the shell,
 * it is the case for csh--but not tcsh). There are also several other cleanup
 * activities which we would like to have done. Thus we trap most ways to exit
 * the program. I also find it useful to modify the behaviour of program
 * crashes. If it is production code, then a bug log is produced instead of a
 * core dump. If the code is a development version (compiled with TEST), then
 * some clean up is done before dumping core. This makes it so that when there
 * are multiple processes running, we get a clean core dump.
 *
 * Returns:
 *     On success, return value is NO_ERROR.  A return value of ERROR indicates
 *     a failure performing one or more setup tasks, such as starting the new
 *     process or manipulating the signal handlers.  The cause of the error is
 *     written to the kjb_error message.
 *
 * Related:
 *     kjb_cleanup, add_cleanup_function, kjb_abort, set_default_abort_trap,
 *     set_sig_trap
 *
 * Index: KJB library
 *
 * -----------------------------------------------------------------------------
 */

int kjb_init(void)
{
    Signal_info  cur_atn_vec;
    int          result            = NO_ERROR;
    int save_kjb_debug_level; 


    /* 
     * We run kjb_init() at debug level 0 or 1 because one thinks of calling
     * this first, but then if we are debugging library level routines, then a
     * call to kjb_debug_level(1) could be after. 
    */
    save_kjb_debug_level = kjb_get_debug_level();
    kjb_set_debug_level(MIN_OF(save_kjb_debug_level, 1));

    {
        char kjb_verbose_str[ 100 ];
        int  temp_verbose; 
        
        if (BUFF_GET_ENV("KJB_VERBOSE", kjb_verbose_str) != ERROR)
        {
            if (*kjb_verbose_str != '\0') 
            {
                if (ss1pi(kjb_verbose_str, &temp_verbose) == ERROR)  
                {
                    kjb_print_error();
                }
                else
                {
                    EPE(kjb_set_verbose_level(temp_verbose));
                }
            }
        }
    }

    verbose_pso(2, "Verbose level at program startup is %d (IL4RT).\n", 
                kjb_get_verbose_level());

    kjb_clear_error();

    /*
     * The routine arrange_cleanup() uses atexit() (if it is available) to
     * ensure that kjb_cleanup() gets called, even if the program exits without
     * going through a kjb_exit().  Not generally necessary, as
     * arrange_cleanup() should be called by any routine that creates a cleanup
     * issue, but there is an advantage to ensuring that arrange_cleanup() gets
     * called right at the start, in case a program makes other calles to
     * atexit(). Note, however, that programs using KJB should really use
     * add_cleanup_function() instead, which removes the risk. 
    */
    arrange_cleanup(); 

#ifdef TEST
    /*
     * We check that our #defines are in sync with what the compiler knows.
     *
    */
    check_defines();
#endif 

    /*
    // Not absolutely necessary, but more efficient to do it in advance. Also,
    // if we do not do it in advance, we can get bogus memory leak reports, as
    // then the implicit fork will occur when memory has been allocated, and we
    // don't neccessarily arrange for the free by the child process.
    */
    
    if (create_system_command_process() == ERROR)
    {
        result = ERROR;
        add_error("Can't create process for system comands.");
    }

#define TRAP_FPU_EXCEPTIONS

#ifdef TRAP_FPU_EXCEPTIONS

#ifdef LINUX_X86
#ifdef __GNUC__
#if 0 /* was ifdef DOES_NOT_COMPILE_ANYMORE */
    /*
    __setfpucw(0x1372);
    */
    _FPU_SETCW(0x1372)

    fs_trap_fpu_exceptions = TRUE;
#else
    fs_trap_fpu_exceptions = FALSE;
#endif
#else
    fs_trap_fpu_exceptions = FALSE;
#endif
#else
    fs_trap_fpu_exceptions = FALSE;
#endif
#else
    fs_trap_fpu_exceptions = FALSE;
#endif

    /* Kobus, Oct 25, 09.
     *
     * The routine set_default_abort_trap() has been modified so that for TEST,
     * only SIGABRT gets the special treatment of cleaning up before dumping
     * core. We used to do this for other signals that dump core such as SEGV,
     * but the way do this now seems to make GDB forget the stack on some
     * systems, and it is not clear if this is even needed anymore.  
    */
    if (set_default_abort_trap() == ERROR) result = ERROR;

    if (set_sig_trap(SIGTERM, safe_default_sig_fn, RESTART_AFTER_SIGNAL) == ERROR)
    {
        result = ERROR;
    }

    /*
    // If the current attention trap is not the default, then we will assume
    // that we are supposed to leave it in place. Otherwise, we will set up a
    // trap. Note that this is different than the assumption we make in
    // kjb_fork().
    */
    if (kjb_sigvec(SIGINT, (Signal_info*)NULL, &cur_atn_vec) == ERROR)
    {
        result = ERROR;
    }
    else
    {
        if (cur_atn_vec.SIGNAL_HANDLER == SIG_DFL)
        {
            if (set_sig_trap(SIGINT, safe_default_sig_fn, RESTART_AFTER_SIGNAL) == ERROR)
            {
                result = ERROR;
            }
        }
    }

    if (result == ERROR)
    {
        insert_error("Program startup resulted in the following errors.");
    }

#ifdef MACHINE_CONSTANT_NOT_AVAILABLE
    warn_pso("At least one machine constant is not available.\n");
    warn_pso("A default value was substituted.\n");
    warn_pso("Some file (e.g. l_sys_def.h) should likely be adjusted.\n\n");
#endif

    kjb_set_debug_level(save_kjb_debug_level);

    return result;
}


/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

#ifdef TEST

static void check_defines(void)
{
    typedef union 
    {
        char c[ 4 ];
        kjb_int32 i;
    } int_union; 

    int_union x,y; 

    x.c[0] = x.c[1] = x.c[2] = x.c[3] = 0;
    y.c[0] = y.c[1] = y.c[2] = y.c[3] = 0;
    x.c[1] = 1;
    y.c[2] = 1;

    /*
    dbi(x.i);
    dbi(y.i);
    */

#ifdef MSB_FIRST
    ASSERT_IS_GREATER_INT(x.i, y.i); 
#else
    ASSERT_IS_GREATER_INT(y.i, x.i); 
#endif 

#ifdef LSB_FIRST
    ASSERT_IS_LESS_INT(x.i, y.i); 
#else
    ASSERT_IS_LESS_INT(y.i, x.i); 
#endif 

    /* It would be a rare machine if this were to fail, but we should know about
     * it.
    */
    ASSERT(sizeof(char) == 1);
    ASSERT(sizeof(unsigned char) == 1);

    ASSERT(sizeof(kjb_int16) == 2);
    ASSERT(sizeof(kjb_uint16) == 2);
    ASSERT(sizeof(kjb_int32) == 4);
    ASSERT(sizeof(kjb_uint32) == 4);


#ifdef HAVE_64_BIT_INT
    ASSERT(sizeof(kjb_int64) == 8);
    ASSERT(sizeof(kjb_uint64) == 8);
#endif 

#    ifdef LONG_IS_64_BITS
         ASSERT(sizeof(long) == 8);
#    endif 
    
#    ifdef LONG_IS_32_BITS
         ASSERT(sizeof(long) == 4);
#    endif 
    
#    ifdef INT_IS_64_BITS
         UNTESTED_CODE();
         ASSERT(sizeof(int) == 8);
#    endif 
    
#    ifdef INT_IS_32_BITS
         ASSERT(sizeof(int) == 4);
#    endif 
    
#    ifdef INT_IS_16_BITS
         UNTESTED_CODE();
         ASSERT(sizeof(int) == 2);
#    endif 
    
#    ifdef SHORT_IS_32_BITS
         UNTESTED_CODE();
         ASSERT(sizeof(short) == 4);
#    endif 
    
#    ifdef SHORT_IS_16_BITS
         ASSERT(sizeof(short) == 2);
#    endif 
    
#    ifdef SHORT_IS_8_BITS
         UNTESTED_CODE();
         ASSERT(sizeof(short) == 1);
#    endif 
    
#    ifdef PTR_IS_64_BITS
         ASSERT(sizeof(void*) == 8);
#    endif 
    
#    ifdef PTR_IS_32_BITS
         ASSERT(sizeof(void*) == 4);
#    endif 
    
#    ifdef PTR_IS_16_BITS
         UNTESTED_CODE();
         ASSERT(sizeof(void*) == 2);
#    endif 

}
    
#endif 


/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

#ifdef __cplusplus
}
#endif
 
