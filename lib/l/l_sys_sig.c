
/* $Id: l_sys_sig.c 21520 2017-07-22 15:09:04Z kobus $ */

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

#include "l/l_sys_tsig.h"
#include "l/l_sys_sig.h"
#include "l/l_sys_term.h"
#include "l/l_io.h"
#include "l/l_queue.h"


#ifdef __cplusplus
extern "C" {
#endif

/* -------------------------------------------------------------------------- */

typedef struct Save_trap
{
    Signal_info signal_info;
    Queue_element* restart_stack_head;
}
Save_trap;

/* -------------------------------------------------------------------------- */

static Queue_element* fs_sig_stack_head[ SIGNAL_ARRAY_SIZE ];

#ifdef UNIX
static Queue_element* fs_save_mask_head = NULL;
#endif 

/* -------------------------------------------------------------------------- */

static int restart_on_sig_guts(int sig);
static int dont_restart_on_sig_guts(int sig);

/* -------------------------------------------------------------------------- */

int set_io_atn_trap(void)
{
    IMPORT volatile Bool io_atn_flag;


    io_atn_flag = FALSE;
    return set_atn_trap(io_atn_fn, RESTART_AFTER_SIGNAL);
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

int unset_io_atn_trap(void)
{
    IMPORT volatile Bool io_atn_flag;


    io_atn_flag = FALSE;
    return unset_atn_trap();
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

int set_iteration_atn_trap(void)
{
    IMPORT volatile Bool iteration_atn_flag;


    iteration_atn_flag = FALSE;
    return set_atn_trap(iteration_atn_fn, RESTART_AFTER_SIGNAL);
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

int unset_iteration_atn_trap(void)
{
    IMPORT volatile Bool iteration_atn_flag;


    iteration_atn_flag = FALSE;
    return unset_atn_trap();
}


/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

int set_default_abort_trap(void)
{

#ifdef TEST

#if 0 /* was ifdef TRY_WITHOUT */
    /* Kobus, Oct 25, 09.
     *
     * The mechanism that we use to cleanup before dumping core is now making
     * gdb forget the stack on some systems. On the other hand, the reason for
     * doing this, namely getting cleaner coredumps when we have multiple
     * processes, might be less critical than it was when the routine
     * set_default_abort_trap() was written (over a decade ago). Hence let's try
     * without this code. 
    */
    ERE(set_abort_trap(safe_default_sig_fn));
#endif 

    /*
    // FIX ?
    //
    // Overwrite the handler "safe_default_sig_fn" in the case of SIGABRT.  We
    // do this on the assumption that we will only abort on kjb_abort() and
    // that kjb_abort will set the signal to SIG_DFL after cleaning up but
    // before calling the abort. The reason for this HACK is so that only the
    // processes that called the abort will dump core. Its children will just
    // exit.
    */

    ERE(kjb_signal(SIGABRT, safe_exit_fn));

    return NO_ERROR;
#else
    return set_abort_trap(default_abort_fn);
#endif
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

int set_abort_trap(TRAP_FN_RETURN_TYPE (*abort_fn) (TRAP_FN_ARGS))
{

#ifdef UNIX
    ERE(set_sig_trap(SIGABRT, abort_fn, RESTART_AFTER_SIGNAL));
    ERE(set_sig_trap(SIGILL,  abort_fn, RESTART_AFTER_SIGNAL));
    ERE(set_sig_trap(SIGSEGV, abort_fn, RESTART_AFTER_SIGNAL));
    ERE(set_sig_trap(SIGFPE,  abort_fn, RESTART_AFTER_SIGNAL));
    ERE(set_sig_trap(SIGBUS,  abort_fn, RESTART_AFTER_SIGNAL));
#ifndef LINUX
    ERE(set_sig_trap(SIGSYS,  abort_fn, RESTART_AFTER_SIGNAL));
#endif
#endif
#ifdef SUN
#ifdef SIGLOST
    ERE(set_sig_trap(SIGLOST, abort_fn, RESTART_AFTER_SIGNAL));
#endif
#endif
    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

int unset_abort_trap(void)
{

#ifdef UNIX
    ERE(unset_sig_trap(SIGABRT));
    ERE(unset_sig_trap(SIGILL));
    ERE(unset_sig_trap(SIGSEGV));
    ERE(unset_sig_trap(SIGFPE ));
    ERE(unset_sig_trap(SIGBUS));
#ifndef LINUX
    ERE(unset_sig_trap(SIGSYS));
#endif
#endif
#ifdef SUN
#ifdef SIGLOST
    ERE(unset_sig_trap(SIGLOST));
#endif
#endif
    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

void allow_abort_sig(void)
{

#ifdef UNIX
    allow_sig(SIGABRT);
    allow_sig(SIGILL);
    allow_sig(SIGSEGV);
    allow_sig(SIGFPE );
    allow_sig(SIGBUS);
#ifndef LINUX
    allow_sig(SIGSYS);
#endif
#endif
#ifdef SUN
#ifdef SIGLOST
    allow_sig(SIGLOST);
#endif
#endif
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* ===========================================================================
 *                                set_atn_trap
 *
 * Trap abort-like signals, unless testing.
 *
 * This function attempts to be a portable wrapper on signal handling,
 * across different platforms.  Specifically, this function catches attempts
 * from the keyboard to abort the program.  The usual motive is that so we
 * can shut down the program in a consistent manner:  during testing, however,
 * you might prefer to get a core dump instead of letting the cleanup code
 * run.
 *
 * When the TEST preprocessor symbol is defined, the SIGQUIT signal will (on
 * Unix machines) immediately terminate the program and dump core; whereas
 * if TEST is undefined, SIGQUIT behaves the same as SIGINT.  
 *
 * This description is sort of vague and tentative -- please improve it if
 * you can.
 *
 * Returns:
 *      NO_ERROR iff successful.
 *
 * Index:
 *      signals
 *
 * ---------------------------------------------------------------------------
 */
int set_atn_trap(TRAP_FN_RETURN_TYPE (*fun_ptr) (TRAP_FN_ARGS),
                 int restart_arg)
{

    /*
    // Trap aborts like breaks, unless testing.
    */
#ifndef TEST
#ifdef UNIX
    ERE(set_sig_trap(SIGQUIT, fun_ptr, restart_arg));
#endif 
#endif
    ERE(set_sig_trap(SIGINT, fun_ptr, restart_arg));

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* ===========================================================================
 *                            unset_atn_trap
 *
 * Undoes the effect of set_atn_trap.  (This is a stub; please augment!)
 *
 * Index:
 *      signals
 *
 * ---------------------------------------------------------------------------
 */
int unset_atn_trap(void)
{
    /*
    // Trap aborts like breaks, unless testing.
    */
#ifndef TEST
#ifdef UNIX
    ERE(unset_sig_trap(SIGQUIT));
#endif 
#endif
    ERE(unset_sig_trap(SIGINT));

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

int set_sig_trap(int sig, void (*fun_ptr) (int), int restart_arg)
{
    Signal_info new_trap;
    Signal_info old_trap;
    Save_trap*  cur_save_trap_ptr;
    Save_trap*  save_trap_ptr;


    if (sig >= SIGNAL_ARRAY_SIZE)
    {
        SET_ARGUMENT_BUG();
        return ERROR;
    }

#ifdef UNIX
    INIT_SIGNAL_INFO(new_trap);
    new_trap.SIGNAL_HANDLER = fun_ptr;
    ERE(kjb_sigvec(sig, &new_trap, &old_trap));

#else     /* Not UNIX */

    new_trap.SIGNAL_HANDLER = fun_ptr;
    ERE(kjb_signal(sig, fun_ptr));

    /*
    // FIX
    //
    // It should be possible to get the old trap from most systems. For
    // example, the signal function provided by Borland C returns it. This
    // could be build into a version of kjb_sigvec. (In fact, perhaps the
    // entire logic that is so messy here could be built into kjb_sigvec?).
    // However, until we actually need this capability on Microsoft platforms,
    // just hack it with a placeholder.
    */

    old_trap.SIGNAL_HANDLER = SIG_DFL;
#endif

    /*
    // The cleanup of the queue is now automatically included in cleanup (as
    // one of the last functions). The reason why we do not handle it with a
    // call to add_cleanup_function() is that some of the possible cleanup
    // functions might (do!) indadvertantly call this routine, and we can't add
    // cleanup functions while cleaning up. The specific problem case is when
    // we are cleaning up, and test code finds something illegal like freeng a
    // pointer twice. In this case we want to prompt for abortion, but the
    // prompt routine will call this routine. If this routine has not yet been
    // called, then there is a problem. It is possible to fix this specific
    // problem, but low level cleanup for critical routines should be done by
    // default.
    */

    /*
    // If fs_sig_stack_head[sig] is NULL (should only be possible the first time
    // because removing the first element is trapped as an error in
    // unset_sig_trap), then save the old handler. Otherwise, overwrite the top
    // with the current trap, as it may have changed since it was stacked
    // (flags, mask, etc.)
    */

    if (fs_sig_stack_head[ sig ] == NULL)
    {
        save_trap_ptr = TYPE_MALLOC(Save_trap);
        save_trap_ptr->signal_info = old_trap;
        save_trap_ptr->restart_stack_head = NULL;

        insert_into_queue(&(fs_sig_stack_head[ sig ]), (Queue_element**)NULL,
                          (void*)save_trap_ptr);
    }
    else
    {
        cur_save_trap_ptr = (Save_trap*)fs_sig_stack_head[ sig ]->contents;
        cur_save_trap_ptr->signal_info = old_trap;
    }

    save_trap_ptr = TYPE_MALLOC(Save_trap);
    save_trap_ptr->signal_info = new_trap;
    save_trap_ptr->restart_stack_head = NULL;

    insert_into_queue(&(fs_sig_stack_head[ sig ]), (Queue_element**)NULL,
                      (void*)save_trap_ptr);

    if (restart_arg == RESTART_AFTER_SIGNAL)
    {
        return restart_on_sig(sig);
    }
    else if (restart_arg == DONT_RESTART_AFTER_SIGNAL)
    {
        return dont_restart_on_sig(sig);
    }
    else
    {
        SET_ARGUMENT_BUG();
        return ERROR;
    }
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

int unset_sig_trap(int sig)
{
    Queue_element* cur_trap_elem;
    Signal_info    signal_info;
    Save_trap*     saved_trap_ptr;
    Save_trap*     cur_saved_trap_ptr;
    Queue_element* cur_restart_head;


    if (sig >= SIGNAL_ARRAY_SIZE)
    {
        SET_ARGUMENT_BUG();
        return ERROR;
    }

    if (fs_sig_stack_head[ sig ] == NULL)
    {
        set_bug("Routine unset_sig_trap called before set_sig_trap.");
        return ERROR;
    }

    if (fs_sig_stack_head[ sig ]->next == NULL)
    {
        set_bug("No trap to unset in unset_sig_trap.\n");
        return ERROR;
    }

    cur_trap_elem = remove_first_element(&(fs_sig_stack_head[ sig ]),
                                         (Queue_element**)NULL);
    cur_saved_trap_ptr = (Save_trap*)cur_trap_elem->contents;

    free_queue(&(cur_saved_trap_ptr->restart_stack_head),
               (Queue_element **)NULL, kjb_free);
    free_queue_element(cur_trap_elem, kjb_free);

    saved_trap_ptr = (Save_trap *)(fs_sig_stack_head[ sig ]->contents);
    signal_info = saved_trap_ptr->signal_info;

#ifdef UNIX
    ERE(kjb_sigvec(sig, &signal_info, (Signal_info*)NULL));
#else
    ERE(kjb_signal(sig, signal_info.SIGNAL_HANDLER));
#endif

    cur_restart_head = saved_trap_ptr->restart_stack_head;

    if (cur_restart_head != NULL)
    {
        if ( *((int*)(cur_restart_head->contents)))
        {
            ERE(restart_on_sig_guts(sig));
        }
        else
        {
            ERE(dont_restart_on_sig_guts(sig));
        }
    }

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

#ifdef TRACK_MEMORY_ALLOCATION

void destroy_sig_queue(void)
{
    Queue_element* cur_head;
    Queue_element* cur_trap_elem;
    Save_trap*     cur_saved_trap_ptr;
    int            i;

    for (i=0; i<SIGNAL_ARRAY_SIZE; i++)
    {
        cur_head = fs_sig_stack_head[ i ];

        while((cur_trap_elem = remove_first_element(&cur_head,
                                                    (Queue_element**)NULL))
              != NULL
             )
        {
            cur_saved_trap_ptr = (Save_trap*)cur_trap_elem->contents;

            free_queue(&(cur_saved_trap_ptr->restart_stack_head),
                       (Queue_element **)NULL, kjb_free);

            free_queue_element(cur_trap_elem, kjb_free);
        }

        /* Kobus, 14/09/14: This is here so that we can use this routine to give
         * a child process a fresh signal stack so that freeing the back
         * pointers inhereted from the parent do not create confusion. 
        */
        fs_sig_stack_head[ i ] = NULL;
    }
}

#endif

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

void block_sig(int sig)
{
#ifndef SYSV_SIGNALS
    int cur_mask;
#endif

#ifdef UNIX

#ifdef SYSV_SIGNALS
    sighold(sig);
#else
    UNTESTED_CODE();
    cur_mask = sigblock( 0 );
    sigsetmask(cur_mask | sigmask(sig));
#endif

#else
    return;
#endif
}


/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

void allow_sig(int sig)
{
#ifndef SYSV_SIGNALS
    int cur_mask;
#endif

#ifdef UNIX

#ifdef SYSV_SIGNALS
    sigrelse(sig);
#else
    cur_mask = sigblock( 0 );
    sigsetmask( cur_mask & (~(sigmask(sig))));
#endif

#else
    return;
#endif
}


/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

void block_atn(void)
{
#ifndef SYSV_SIGNALS
    int cur_mask;
#endif

#ifdef UNIX

#ifdef SYSV_SIGNALS
    sighold(SIGINT);
    sighold(SIGQUIT);
#else
    cur_mask = sigblock( 0 );
    sigsetmask(cur_mask | sigmask(SIGINT) | sigmask(SIGQUIT));
#endif

#else
    return;
#endif
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

void allow_atn(void)
{
#ifndef SYSV_SIGNALS
    int cur_mask;
#endif

#ifdef UNIX

#ifdef SYSV_SIGNALS
    sigrelse(SIGINT);
    sigrelse(SIGQUIT);
#else
    cur_mask = sigblock( 0 );
    sigsetmask( cur_mask & (~(sigmask(SIGINT))) & (~(sigmask(SIGQUIT))));
#endif

#else
    return;
#endif
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* all */    int block_user_signals(void)
/* all */    {
#ifdef UNIX
#ifdef SYSV_SIGNALS
/* SYSV */       sigset_t mask, save_mask;
#else
/* not SYSV */   int save_mask;
#endif
#endif
/* all */        int result;
/* all */
/* all */
#ifdef UNIX
#ifdef SYSV_SIGNALS
/* SYSV */       ERE(kjb_sigemptyset( &mask ));
/* SYSV */       ERE(kjb_sigaddset( &mask, SIGINT ));
/* SYSV */       ERE(kjb_sigaddset( &mask, SIGQUIT ));
/* SYSV */       ERE(kjb_sigaddset( &mask, SIGTERM ));
/* SYSV */       ERE(kjb_sigaddset( &mask, SIGTSTP ));
/* SYSV */       ERE(kjb_sigprocmask(SIG_BLOCK, &mask, &save_mask));
#else
/* not SYSV */   save_mask = sigblock(sigmask(SIGINT));
/* not SYSV */   sigblock(sigmask(SIGQUIT));
/* not SYSV */   sigblock(sigmask(SIGTERM));
/* not SYSV */   sigblock(sigmask(SIGTSTP));
#endif
/* unix */       result = alloc_insert_into_queue(&fs_save_mask_head,
/* unix */                                   (Queue_element**)NULL, &save_mask,
/* unix */                                   sizeof(save_mask));
/* unix */       ERE(result);
#endif
/* all */        return NO_ERROR;
/* all */    }


/* all */    int allow_user_signals(void)
/* all */    {
#ifdef UNIX
#ifdef SYSV_SIGNALS
/* SYSV */       sigset_t mask, save_mask;
#else
/* not SYSV */   int mask, save_mask;
#endif
#endif
/* all */        int result;
/* all */
#ifdef UNIX
#ifdef SYSV_SIGNALS
/* SYSV */       ERE(kjb_sigemptyset( &mask ));
/* SYSV */       ERE(kjb_sigaddset( &mask, SIGINT ));
/* SYSV */       ERE(kjb_sigaddset( &mask, SIGQUIT ));
/* SYSV */       ERE(kjb_sigaddset( &mask, SIGTERM ));
/* SYSV */       ERE(kjb_sigaddset( &mask, SIGTSTP ));
/* SYSV */       ERE(kjb_sigprocmask(SIG_UNBLOCK, &mask, &save_mask));
#else
/* not SYSV */   save_mask = sigblock( 0 );
/* not SYSV */
/* not SYSV */   mask = save_mask & (~(sigmask(SIGINT)))
/* not SYSV */                    & (~(sigmask(SIGQUIT)))
/* not SYSV */                    & (~(sigmask(SIGTERM)))
/* not SYSV */                    & (~(sigmask(SIGTSTP)));
/* not SYSV */
/* not SYSV */   sigsetmask ( mask );
#endif
/* unix */       result = alloc_insert_into_queue(&fs_save_mask_head,
/* unix */                                        (Queue_element**)NULL,
/* unix */                                        &save_mask,
/* unix */                                        sizeof(save_mask));
/* unix */       ERE(result);
#endif
/* all */        return NO_ERROR;
/* all */    }


/* all */    int reset_signals(void)
/* all */    {
#ifdef UNIX
/* UNIX */       Queue_element* save_elem;
#ifdef SYSV_SIGNALS
/* SYSV */       sigset_t save_mask;
#else
/* not SYSV */   int save_mask;
#endif
#endif
/* all */        int res;
/* all */
/* all */
/* all */        res = NO_ERROR;
#ifdef UNIX
/* UNIX */       save_elem = remove_first_element(&fs_save_mask_head,
/* UNIX */                                        (Queue_element**)NULL);
/* UNIX */
/* UNIX */       if (save_elem == NULL)
/* UNIX */       {
/* UNIX */           set_bug("Nothing to reset in reset_signals.");
/* UNIX */           return ERROR;
/* UNIX */       }
/* UNIX */
/* UNIX */
#ifdef SYSV_SIGNALS
/* SYSV */       save_mask = *((sigset_t *)save_elem->contents);
/* SYSV */
/* SYSV */       if (sigprocmask(SIG_SETMASK, &save_mask, (sigset_t *)NULL) == EOF)
/* SYSV */       {
/* SYSV */           res = ERROR;
/* SYSV */       }
#else
/* not SYSV */   save_mask = *((int *)save_elem->contents);
/* not SYSV */
/* not SYSV */   sigsetmask(save_mask);
/* not SYSV */
#endif
/* UNIX */       free_queue_element(save_elem, kjb_free);
#endif
/* all */        return res;
/* all */    }

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

int dont_restart_on_atn(void)
{


    ERE(dont_restart_on_sig(SIGINT));

#ifndef TEST
#ifdef UNIX
    ERE(dont_restart_on_sig(SIGQUIT));
#endif 
#endif

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

int dont_restart_on_sig(int sig)
{
    int        cur_restart_flag;
    Save_trap* saved_trap_ptr;
    int        result;


    if ((sig <= 0) || (sig >= SIGNAL_ARRAY_SIZE))
    {
        SET_ARGUMENT_BUG();
        return ERROR;
    }

    if (fs_sig_stack_head[ sig ] == NULL)
    {
        set_bug("No trap is set in dont_restart_on_sig.");
        return ERROR;
    }

    saved_trap_ptr = (Save_trap *)(fs_sig_stack_head[ sig ]->contents);

    cur_restart_flag = FALSE;

    result = alloc_insert_into_queue(&(saved_trap_ptr->restart_stack_head),
                                     (Queue_element**)NULL,
                                     (void*)&cur_restart_flag,
                                     sizeof(cur_restart_flag));
    ERE(result);

    ERE(dont_restart_on_sig_guts(sig));

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

static int dont_restart_on_sig_guts(int sig)
{
    IMPORT int restart_io_after_sig[ SIGNAL_ARRAY_SIZE ];
#ifdef UNIX
    Signal_info cur_vec;
#endif


#ifdef UNIX
    ERE(kjb_sigvec(sig, (Signal_info *)NULL, &cur_vec));
    SET_DONT_RESTART_IO_AFTER_SIGNAL(cur_vec);
    ERE(kjb_sigvec(sig, &cur_vec, (Signal_info *)NULL));
#endif

    restart_io_after_sig[ sig ] = FALSE;

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

int restart_on_atn(void)
{


    ERE(restart_on_sig(SIGINT));

#ifndef TEST
#ifdef UNIX
    ERE(restart_on_sig(SIGQUIT));
#endif 
#endif

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

int restart_on_sig(int sig)
{
    int        cur_restart_flag;
    Save_trap* saved_trap_ptr;
    int result;


    if ((sig <= 0) || (sig >= SIGNAL_ARRAY_SIZE))
    {
        SET_ARGUMENT_BUG();
        return ERROR;
    }

    if (fs_sig_stack_head[ sig ] == NULL)
    {
        set_bug("No trap is set in restart_on_sig.");
        return ERROR;
    }

    saved_trap_ptr = (Save_trap *)(fs_sig_stack_head[ sig ]->contents);

    cur_restart_flag = TRUE;

    result = alloc_insert_into_queue(&(saved_trap_ptr->restart_stack_head),
                                     (Queue_element**)NULL,
                                     (void*)&cur_restart_flag,
                                     sizeof(cur_restart_flag));
    ERE(result);

    ERE(restart_on_sig_guts(sig));

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

static int restart_on_sig_guts(int sig)
{
    IMPORT int restart_io_after_sig[ SIGNAL_ARRAY_SIZE ];
#ifdef UNIX
    Signal_info cur_vec;
#endif


#ifdef UNIX
    ERE(kjb_sigvec(sig, (Signal_info *)NULL, &cur_vec));
    SET_RESTART_IO_AFTER_SIGNAL(cur_vec);
    ERE(kjb_sigvec(sig, &cur_vec, (Signal_info *)NULL));
#endif

    restart_io_after_sig[ sig ] = TRUE;

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

int reset_restart_on_atn(void)
{
    ERE(reset_restart_on_sig(SIGINT));

#ifndef TEST
#ifdef UNIX
    ERE(reset_restart_on_sig(SIGQUIT));
#endif 
#endif

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

int reset_restart_on_sig(int sig)
{
    Save_trap*     saved_trap_ptr;
    Queue_element* first_elem;
    Queue_element* new_restart_head;
    int            restart_flag;


    if (fs_sig_stack_head[ sig ] == NULL)
    {
        set_bug("No trap set in reset_restart_on_sig.");
        return ERROR;
    }

    saved_trap_ptr = (Save_trap *)(fs_sig_stack_head[ sig ]->contents);

    if (saved_trap_ptr->restart_stack_head == NULL)
    {
        set_bug("Unintialized stack in reset_restart_on_sig.");
        return ERROR;
    }

    if (saved_trap_ptr->restart_stack_head->next == NULL)
    {
        set_bug("Nothing to reset in reset_restart_on_sig.");
        return ERROR;
    }

    first_elem = remove_first_element(&(saved_trap_ptr->restart_stack_head),
                                      (Queue_element**)NULL);

    free_queue_element(first_elem, kjb_free);

    new_restart_head = saved_trap_ptr->restart_stack_head;

    restart_flag = *((int*) new_restart_head->contents);

    if (restart_flag)
    {
        ERE(restart_on_sig_guts(SIGINT));
    }
    else
    {
        ERE(dont_restart_on_sig_guts(SIGINT));
    }

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                               kill_myself
 *
 *
 *
 * Index:
 *     signals
 * 
 * -----------------------------------------------------------------------------
*/

/*
// CHECK
//
// This is a tricky problem. For the most part, it seems to work, but it could.
// at some stage bare additional consideration.
*/

#ifdef UNIX
#ifdef SYSV_SIGNALS /* Includes AIX, which has both SYSV and bsd facilities. */
/* SYSV */   int kill_myself(int sig)
/* SYSV */
/* SYSV */   {
/* SYSV */       IMPORT volatile int recorded_signal;
/* SYSV */       int res;
/* SYSV */       sigset_t mask, old_mask, blank_mask;
/* SYSV */
/* SYSV */
/* SYSV */       ERE(kjb_sigemptyset( &blank_mask ));
/* SYSV */       ERE(kjb_sigemptyset( &mask ));
/* SYSV */       ERE(kjb_sigaddset( &mask, sig ));
/* SYSV */       ERE(kjb_sigprocmask(SIG_BLOCK, &mask, &old_mask));
/* SYSV */
#if 0 /* was ifdef OLD_WAY */
/* SYSV */       ERE(kjb_sighold( sig ));
#endif
/* SYSV */
/* SYSV */       res = NO_ERROR;
/* SYSV */
/* SYSV */       if (kill(MY_PID, sig) != EOF)
/* SYSV */       {
/* SYSV */            /* Must ensure that sigpause gets a      */
/* SYSV */            /* signal: In the case of SIGTSTP, it    */
/* SYSV */            /* won't, a priora. Thus set a 0.2 sec   */
/* SYSV */            /* time alarm. */
/* SYSV */
/* SYSV */            if (sig == SIGTSTP)
/* SYSV */            {
/* SYSV */                set_ms_time_alarm(200);
/* SYSV */            }
/* SYSV */
#if 0 /* was ifdef OLD_WAY */
#ifdef AIX
/* AIX */             sigpause( 0 );   /* I believe this is OK, but ... */
#else
#ifdef HPUX
                      /*
                      // Sigpause on the HP is busted as far as I can tell.
                      // Basically the documentation describes two independent
                      // methods, and says that they conflict with each other.
                      // BOTH methods use sigpause and the args are different
                      // in the two cases!.
                      */
/* HPUX */            sigpause( 0L );
#else
/* SYSV */            sigpause( sig );   /* Works on SUN5 */
#endif
#endif
#else   /* Case not OLD_WAY follows */

                      sigsuspend( &blank_mask );
#endif
/* SYSV */            if (sig == SIGTSTP)
/* SYSV */            {
/* SYSV */                unset_time_alarm();
/* SYSV */            }
/* SYSV */
/* SYSV */            /* Now it is possible that the recorded signal has been */
/* SYSV */            /* set to the alarm signal, or even something else.   */
/* SYSV */
/* SYSV */            recorded_signal = sig;
/* SYSV */       }
/* SYSV */       else
/* SYSV */       {
/* SYSV */            res = ERROR;
/* SYSV */       }
/* SYSV */
/* SYSV */       if (kjb_sigprocmask(SIG_SETMASK,
/* SYSV */                           &old_mask,
/* SYSV */                           (sigset_t *)NULL) == ERROR)
/* SYSV */       {
/* SYSV */           res = ERROR;
/* SYSV */       }
/* SYSV */
/* SYSV */       return res;
/* SYSV */   }
#else
/* not SYSV */   int kill_myself(sig)
/* not SYSV */     int sig;
/* not SYSV */   {
/* not SYSV */       IMPORT volatile int recorded_signal;
/* not SYSV */       int old_mask;
/* not SYSV */       int res;
/* not SYSV */
/* not SYSV */
/* not SYSV */       res = NO_ERROR;
/* not SYSV */
/* not SYSV */       old_mask = sigblock(sigmask(sig));
/* not SYSV */
/* not SYSV */       if (kill(MY_PID, sig) != EOF)
/* not SYSV */       {
/* not SYSV */            /* Must ensure that sigpause gets a      */
/* not SYSV */            /* signal: In the case of SIGTSTP, it    */
/* not SYSV */            /* won't, a priora. Thus set a 0.2 sec   */
/* not SYSV */            /* time alarm. */
/* not SYSV */
/* not SYSV */            if (sig == SIGTSTP)
/* not SYSV */            {
/* not SYSV */                set_ms_time_alarm(200);
/* not SYSV */            }
/* not SYSV */            sigpause( 0 );
/* not SYSV */
/* not SYSV */            if (sig == SIGTSTP)
/* not SYSV */            {
/* not SYSV */                unset_time_alarm();
/* not SYSV */            }
/* not SYSV */
/* not SYSV */            /* It is possible that the recorded signal has been */
/* not SYSV */            /* set to the alarm signal, or even something else. */
/* not SYSV */
/* not SYSV */            recorded_signal = sig;
/* not SYSV */       }
/* not SYSV */       else
/* not SYSV */       {
/* not SYSV */            res = ERROR;
/* not SYSV */       }
/* not SYSV */
/* not SYSV */       sigsetmask(old_mask);
/* not SYSV */
/* not SYSV */       return res;
/* not SYSV */   }
#endif
#endif      /* UNIX */


/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                            record_signal
 *
 *
 *
 * Index:
 *     signals
 * 
 * -----------------------------------------------------------------------------
*/

/*ARGSUSED*/ /* We assume we have "sig" as "int" as first arg (always on UNIX) */
TRAP_FN_RETURN_TYPE record_signal( TRAP_FN_ARGS )
{
    IMPORT volatile int recorded_signal;


    recorded_signal = sig;

    /*
    // All UNIX systems so far have some way of making it so that we don't have
    // to reinstate the handler. We assume that this is being done. For SGI and
    // Solaris, this means assuming that sigaction() or sigset() was used in
    // place of signal().
    */

#ifndef UNIX
    KJB_SIGNAL(sig, record_signal);
#endif
}


/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                                io_atn_fn
 *
 *
 *
 * Index:
 *     signals
 * 
 * -----------------------------------------------------------------------------
*/

/*ARGSUSED*/  /* We assume we have "sig" as "int" as first arg (always on UNIX) */
TRAP_FN_RETURN_TYPE io_atn_fn( TRAP_FN_ARGS )
{
    IMPORT volatile int     recorded_signal;
    IMPORT volatile Bool    io_atn_flag;
    IMPORT volatile Bool    halt_all_output;


    recorded_signal = sig;

#ifdef TEST
    if (yes_no_query("io_atn_fn: Interupt processing? "))
#else
    if (yes_no_query("Interupt processing? "))
#endif
    {
        io_atn_flag = TRUE;
        halt_all_output = TRUE;
    }

    /*
    // All UNIX systems so far have some way of making it so that we don't have
    // to reinstate the handler. We assume that this is being done. For SGI and
    // Solaris, this means assuming that sigaction() or sigset() was used in
    // place of signal().
    */

#ifndef UNIX
    KJB_SIGNAL(sig, io_atn_fn);
#endif
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                             iteration_atn_fn
 *
 *
 *
 * Index:
 *     signals
 * 
 * -----------------------------------------------------------------------------
*/

/*ARGSUSED*/  /* We assume we have "sig" as "int" as first arg (always on UNIX) */
TRAP_FN_RETURN_TYPE iteration_atn_fn( TRAP_FN_ARGS )
{
    IMPORT volatile int recorded_signal;
    IMPORT volatile Bool iteration_atn_flag;


    recorded_signal = sig;

    if (yes_no_query("Interupt processing at end if next iteration? "))
    {
        iteration_atn_flag = TRUE;
    }

    /*
    // All UNIX systems so far have some way of making it so that we don't have
    // to reinstate the handler. We assume that this is being done. For SGI and
    // Solaris, this means assuming that sigaction() or sigset() was used in
    // place of signal().
    */

#ifndef UNIX
    KJB_SIGNAL(sig, io_atn_fn);
#endif
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                          yes_no_query_atn_fn
 *
 *
 *
 * Index:
 *     signals
 * 
 * -----------------------------------------------------------------------------
*/

/*ARGSUSED*/  /* We assume we have "sig" as "int" as first arg (always on UNIX) */
TRAP_FN_RETURN_TYPE yes_no_query_atn_fn( TRAP_FN_ARGS )
{
    IMPORT volatile int recorded_signal;


#ifndef UNIX
    /*
        Use on systems that insist on restarting IO routines on
        interupts.
    */
    term_blank_out_line();

    put_prompt("Enter either y (yes), or n (no) ");
#endif

    /*
    // All UNIX systems so far have some way of making it so that we don't have
    // to reinstate the handler. We assume that this is being done. For SGI and
    // Solaris, this means assuming that sigaction() or sigset() was used in
    // place of signal().
    */
#ifndef UNIX
    KJB_SIGNAL(sig, yes_no_query_atn_fn);
#endif

    recorded_signal = sig;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                            confirm_exit_sig_fn
 *
 *
 *
 * Index:
 *     signals
 * 
 * -----------------------------------------------------------------------------
*/

/*ARGSUSED*/  /* We assume we have "sig" as "int" as first arg (always on UNIX) */
TRAP_FN_RETURN_TYPE confirm_exit_sig_fn( TRAP_FN_ARGS )
{
    IMPORT volatile int recorded_signal;


    KJB_SIGNAL(sig, KJB_SIG_DFL);

    allow_sig(sig);

    if (yes_no_query("Terminate program ? (Y for yes, N for No) "))
    {
        term_puts("Program terminated.\n");
        kjb_exit_2(EXIT_SUCCESS);
        /*NOTREACHED*/
    }

    recorded_signal = sig;

    KJB_SIGNAL(sig, confirm_exit_sig_fn);
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                            sort_atn_fn
 *
 *
 *
 * Index:
 *     signals
 * 
 * -----------------------------------------------------------------------------
*/

/*ARGSUSED*/  /* We assume we have "sig" as "int" as first arg (always on UNIX) */
TRAP_FN_RETURN_TYPE sort_atn_fn( TRAP_FN_ARGS )
{
    IMPORT volatile Bool sort_atn_flag;
    IMPORT volatile int recorded_signal;


    recorded_signal = sig;

    sort_atn_flag = TRUE;

    /*
    // All UNIX systems that I have seen so far have some way of making it so
    // that we don't have to reinstate the handler. We assume that this is being
    // done. For SGI and Solaris, this means assuming that sigaction() or
    // sigset() was used in place of signal().
    */
#ifndef UNIX
    KJB_SIGNAL(sig, sort_atn_fn);
#endif
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                              safe_default_sig_fn
 *
 *
 *
 * Index:
 *     signals
 * 
 * -----------------------------------------------------------------------------
*/

/*ARGSUSED*/   /* We assume we have "sig" as "int" as first arg (always on UNIX) */
TRAP_FN_RETURN_TYPE safe_default_sig_fn(TRAP_FN_ARGS)
{


    (void)set_sig_trap(sig, SIG_DFL, RESTART_AFTER_SIGNAL);

    /*
     * This needs to go after the above line in case attempting to print causes
     * another  signal. We have seen at least one case where some problem with
     * corrupted memory meant that this was happening, causing recursion into
     * this routine when the following line was the first one. 
    */
    TEST_PSE(("Process %ld is in safe_default_sig_fn due to signal %d.\n",
              MY_PID, sig));

    kjb_cleanup_for_abort();

    kill_myself(sig);
}


/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                                 safe_exit_fn
 *
 *
 *
 * Index:
 *     signals
 * 
 * -----------------------------------------------------------------------------
*/

/*ARGSUSED*/   /* We assume we have "sig" as "int" as first arg (always on UNIX) */
TRAP_FN_RETURN_TYPE safe_exit_fn(TRAP_FN_DUMMY_ARGS)
{


    TEST_PSE(("Process %ld is in safe_exit_fn.\n", (long)MY_PID));

    kjb_exit(EXIT_FAILURE);
}


/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                                 default_abort_fn
 *
 *
 *
 * Index:
 *     signals
 * 
 * -----------------------------------------------------------------------------
*/

/*ARGSUSED*/   /* We assume we have "sig" as "int" as first arg (always on UNIX) */
TRAP_FN_RETURN_TYPE default_abort_fn(TRAP_FN_DUMMY_ARGS)
{
    IMPORT void (*kjb_bug_handler)(const char*);


    if (kjb_bug_handler != NULL)
    {
        dbw();
        (*kjb_bug_handler)("Trapped deadly signal.");
    }

    /* Don't use L level routines when not necessary. */
    /* They may be influenced by the the problem (or  */
    /* may even be the problem.) Since we are about   */
    /* to die, paging, etc, is irrelavent.            */

    fprintf(stderr, "\n*** Fatal internal error\n");
    fprintf(stderr, "*** Error Termination\n\n");

    kjb_exit(EXIT_FAILURE);
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                           reset_terminal_size_on_sig_fn
 *
 *
 *
 * Index:
 *     signals
 * 
 * -----------------------------------------------------------------------------
*/

/*ARGSUSED*/   /* We assume we have "sig" as "int" as first arg (always on UNIX) */
TRAP_FN_RETURN_TYPE reset_terminal_size_on_sig_fn( TRAP_FN_ARGS )
{
    IMPORT volatile int recorded_signal;


    setup_terminal_size();

    recorded_signal = sig;

    /*
    // All UNIX systems ported to at this point have some way of making it so
    // that we don't have to reinstate the handler. We assume that this is being
    // done. For SGI and Solaris, this means assuming that sigaction() or
    // sigset() was used in place of signal().
    */
#ifndef UNIX
    KJB_SIGNAL(sig, reset_terminal_size_on_sig_fn);
#endif
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                          cooked_mode_sig_fn
 *
 *
 *
 * Index:
 *     signals
 * 
 * -----------------------------------------------------------------------------
*/

/* Current assumption: Raw/cooked mode only applies to UNIX. */

#ifdef UNIX

/*ARGSUSED*/   /* We assume we have "sig" as "int" as first arg (always on UNIX) */
TRAP_FN_RETURN_TYPE cooked_mode_sig_fn( TRAP_FN_ARGS )
{
    IMPORT volatile int recorded_signal;
    IMPORT volatile Bool term_no_block_flag;
    IMPORT volatile int term_io_since_last_input_attempt;
    int reset_no_blocking_flag = FALSE;


    if (term_no_block_flag)
    {
        reset_no_blocking_flag = TRUE;
        term_set_blocking();
    }

    term_reset();

    allow_sig(sig);
    kill_myself(sig);

    /*
    // FIX ?
    //
    // A bit of a hack here. If we got here by a CTL-Z, and then were put in
    // the background, then we don't want to continue with terminal IO until
    // we are in the forground again. However, we want other connected
    // processes to be able to continue. So we periodically check and sleep.
    */
    while (term_set_cooked_mode() == BACKGROUND_READ)
    {
        nap(300);
    }

    if (reset_no_blocking_flag)
    {
        term_set_no_blocking();
    }

    recorded_signal = sig;

    term_io_since_last_input_attempt = TRUE;

    /*
    // All UNIX systems that I have encountered so far have some way of making
    // it so that we don't have to reinstate the handler. We assume that this is
    // being done. For SGI and Solaris, this means assuming that sigaction() or
    // sigset() was used in place of signal().
    */
#if 0 /* was ifndef UNIX */
    /*
        Note: Currently can't get here because we are in a UNIX only super
              super-section. Keep as is in case we decide to emulate UNIX
              behaviour on some system.
    */
    KJB_SIGNAL(sig, cooked_mode_sig_fn);
#endif
}


/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                              raw_mode_with_echo_sig_fn
 *
 *
 *
 * Index:
 *     signals
 * 
 * -----------------------------------------------------------------------------
*/

/* Still in UNIX only (for now) */

/*ARGSUSED*/   /* We assume we have "sig" as "int" as first arg (always on UNIX) */
TRAP_FN_RETURN_TYPE raw_mode_with_echo_sig_fn( TRAP_FN_ARGS )
{
    IMPORT volatile int recorded_signal;
    IMPORT volatile Bool term_no_block_flag;
    IMPORT volatile int term_io_since_last_input_attempt;
    int reset_no_blocking_flag = FALSE;


    if (term_no_block_flag)
    {
        reset_no_blocking_flag = TRUE;
    }

    term_reset();

    allow_sig(sig);
    kill_myself(sig);

    /*
    // FIX ?
    //
    // A bit of a hack here. If we got here by a CTL-Z, and then were put in
    // the background, then we don't want to continue with terminal IO until
    // we are in the forground again. However, we want other connected
    // processes to be able to continue. So we periodically check and sleep.
    */
    while (term_set_raw_mode_with_echo() == BACKGROUND_READ)
    {
        nap(300);
    }

    if (reset_no_blocking_flag)
    {
        term_set_no_blocking();
    }

    recorded_signal = sig;

    term_io_since_last_input_attempt = TRUE;

    /*
    // All UNIX systems that I have encountered so far have some way of making
    // it so that we don't have to reinstate the handler. We assume that this is
    // being done. For SGI and Solaris, this means assuming that sigaction() or
    // sigset() was used in place of signal().
    */

#if 0 /* was ifndef UNIX */
    /*
        Note: Currently can't get here because we are in a UNIX only super
              super-section. Keep as is in case we decide to emulate UNIX
              behaviour on some system.
    */
    KJB_SIGNAL(sig, raw_mode_with_echo_sig_fn);
#endif
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                            raw_mode_with_no_echo_sig_fn
 *
 *
 *
 * Index:
 *     signals
 * 
 * -----------------------------------------------------------------------------
*/

/* Still in UNIX only (for now) */

/*ARGSUSED*/   /* We assume we have "sig" as "int" as first arg (always on UNIX) */
TRAP_FN_RETURN_TYPE raw_mode_with_no_echo_sig_fn( TRAP_FN_ARGS )
{
    IMPORT volatile int recorded_signal;
    IMPORT volatile Bool term_no_block_flag;
    IMPORT volatile int term_io_since_last_input_attempt;
    int reset_no_blocking_flag = FALSE;


    if (term_no_block_flag)
    {
        term_set_blocking();
        reset_no_blocking_flag = TRUE;
    }

    term_reset();

    allow_sig(sig);
    kill_myself(sig);

    /*
    // FIX ?
    //
    // A bit of a hack here. If we got here by a CTL-Z, and then were put in
    // the background, then we don't want to continue with terminal IO until
    // we are in the forground again. However, we want other connected
    // processes to be able to continue. So we periodically check and sleep.
    */
    while (term_set_raw_mode_with_no_echo() == BACKGROUND_READ)
    {
        nap(300);
    }

    if (reset_no_blocking_flag)
    {
        term_set_no_blocking();
    }

    recorded_signal = sig;

    term_io_since_last_input_attempt = TRUE;

    /*
    // All UNIX systems dealt with so far have some way of making it so that we
    // don't have to reinstate the handler. We assume that this is being done.
    // For SGI and Solaris, this means assuming that sigaction() or sigset()
    // was used in place of signal().
    */

#if 0 /* was ifndef UNIX */
    /*
        Note: Currently can't get here because we are in a UNIX only super
              super-section. Keep as is in case we decide to emulate UNIX
              behaviour on some system.
    */
    KJB_SIGNAL(sig, raw_mode_with_no_echo_sig_fn);
#endif
}
#endif   /* UNIX */

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

#ifdef UNIX

/* =============================================================================
 *                              reset_term_before_default_sig_fn
 *
 *
 *
 * Index:
 *     signals
 * 
 * -----------------------------------------------------------------------------
*/

/*
//  Current assumption: Only need to catch signals to fix terminal settings on
//  UNIX.
*/

/*ARGSUSED*/  /* (We assume we have "sig" as "int" as first arg (always on UNIX)) */
TRAP_FN_RETURN_TYPE reset_term_before_default_sig_fn( TRAP_FN_ARGS )
{


    reset_initial_terminal_settings();

    allow_sig(sig);

    KJB_SIGNAL(sig, KJB_SIG_DFL);

    kill_myself(sig);

    KJB_SIGNAL(sig, reset_term_before_default_sig_fn);
}


/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*
//  Current assumption: Only need to catch signals to fix terminal settings on
//  UNIX. (We are still in UNIX only).
*/

/* =============================================================================
 *                          raw_mode_with_no_echo_default_sig_fn
 *
 *
 *
 * Index:
 *     signals
 * 
 * -----------------------------------------------------------------------------
*/

/*ARGSUSED*/   /* We assume we have "sig" as "int" as first arg (always on UNIX) */
TRAP_FN_RETURN_TYPE raw_mode_with_no_echo_default_sig_fn( TRAP_FN_ARGS )
{
    IMPORT volatile Bool term_no_block_flag;
    IMPORT volatile int term_io_since_last_input_attempt;
    IMPORT volatile int term_io_since_last_input_attempt;
    int reset_no_blocking_flag = FALSE;


    if (term_no_block_flag)
    {
        reset_no_blocking_flag = TRUE;
        term_set_blocking();
    }

    /*
    // Keep honest with the pushes and pulls.
    */
    term_reset();

    /*
    // What we really want to do.
    */
    reset_initial_terminal_settings();

    allow_sig(sig);

    KJB_SIGNAL(sig, KJB_SIG_DFL);

    kill_myself(sig);

    term_io_since_last_input_attempt = TRUE;

    if (reset_no_blocking_flag)
    {
        term_set_no_blocking();
    }

    term_set_raw_mode_with_no_echo();

    KJB_SIGNAL(sig, reset_term_before_default_sig_fn);
}

#endif

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                              kjb_signal
 *
 *
 *
 * Index:
 *     signals
 * 
 * -----------------------------------------------------------------------------
*/

int kjb_signal(int sig_num, void (*fn) (int))
{
    TRAP_FN_RETURN_TYPE (*result)(TRAP_FN_ARGS);

    result = KJB_SIGNAL(sig_num, fn);

    if (result == SIG_ERR )
    {
#ifdef TEST
        set_error("Call to KJB_SIGNAL failed.%S");
#else
        set_error("System call failure.");
#endif
        return ERROR;
    }
    else
    {
        return NO_ERROR;
    }
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

#ifdef UNIX

/* =============================================================================
 *                                  kjb_sigvec
 *
 *
 * Index:
 *     signals
 * 
 * -----------------------------------------------------------------------------
*/

int kjb_sigvec(int sig, struct sigaction *new_vec_ptr,
               struct sigaction *old_vec_ptr)
{
    int result;


    result = KJB_SIGVEC(sig, new_vec_ptr, old_vec_ptr);

    if (result == -1 )
    {
#ifdef TEST
        set_error("Call to KJB_SIGVEC failed.%S");
#else
        set_error("System call failure.");
#endif
        return ERROR;
    }
    else
    {
        return result;
    }
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*
 * =============================================================================
 *                                  kjb_sigemptyset
 *
 *
 *
 *
 * Index:
 *     signals
 * 
 * -----------------------------------------------------------------------------
 */

#ifdef SYSV_SIGNALS

int kjb_sigemptyset(sigset_t *mask_ptr)
{
    int result;


    result = sigemptyset(mask_ptr);

    if (result == -1 )
    {
#ifdef TEST
        set_error("Call to sigemptyset failed.%S");
#else
        set_error("System call failure.");
#endif
        return ERROR;
    }
    else
    {
        return result;
    }
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*
 * =============================================================================
 *                                  kjb_sigaddset
 *
 *
 *
 *
 * Index:
 *     signals
 * 
 * -----------------------------------------------------------------------------
 */

/* Still in #ifdef SYSV */

int kjb_sigaddset(sigset_t *mask_ptr, int sig)
{
    int result;


    result = sigaddset(mask_ptr, sig);

    if (result == -1 )
    {
#ifdef TEST
        set_error("Call to sigaddset failed.%S");
#else
        set_error("System call failure.");
#endif
        return ERROR;
    }
    else
    {
        return result;
    }
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*
 * =============================================================================
 *                                  kjb_sigprocmask
 *
 *
 *
 *
 * Index:
 *     signals
 * 
 * -----------------------------------------------------------------------------
 */

/* Still in #ifdef SYSV */

int kjb_sigprocmask(int sig, sigset_t *mask_ptr, sigset_t *old_mask_ptr)
{
    int result;


    result = sigprocmask(sig, mask_ptr, old_mask_ptr);

    if (result == -1 )
    {
#ifdef TEST
        set_error("Call to sigprocmask failed.%S");
#else
        set_error("System call failure.");
#endif
        return ERROR;
    }
    else
    {
        return result;
    }
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

#endif   /* End of #ifdef SYSV */

#endif   /* End of #ifdef UNIX */



#ifdef __cplusplus
}
#endif

