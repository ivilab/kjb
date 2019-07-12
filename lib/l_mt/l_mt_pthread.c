/*
 * Implementation of our pthreads wrapper.
 *
 * File-wide convention:  every function whose name ends with "_unsafe" suffix
 * (which should be static) accesses local static data.  Thus one should
 * generally not call it unless one serializes access by holding the
 * 'thread_master_lock' mutex.
 *
 * $Id: l_mt_pthread.c 21596 2017-07-30 23:33:36Z kobus $
 */

#include "l/l_sys_debug.h"
#include "l/l_sys_lib.h"
#include "l/l_sys_io.h"
#include "l/l_sys_mal.h"
#include "l/l_sys_rand.h"
#include "l/l_error.h"
#include "l/l_debug.h"
#include "l/l_global.h"
#include "l/l_string.h"
#include "l_mt/l_mt_pthread.h"

/* Kobus: 17-03-05: FIXME. 
 * This is a bizarre construction for the pre-processor.  I hope it is portable.
 * Worth making it more standard sometime, which is very easy to do by making
 * predicate an argument to set_error().
*/
#define SET_MSG_RETURN_ERROR(predicate)                        \
    do                                                         \
    {                                                          \
        set_error("Cannot " predicate                          \
                     ":  program linked without libpthread."); \
        return ERROR;                                          \
    }                                                          \
    while(0)


/* The SCRAMBLE macro below requires an int of size at least 27 bits, sorry. */
#ifdef INT_IS_16_BITS
#error "KJB threads are not supported on 16-bit integer systems."
#endif

/* Constants below are from Press et al., Numerical Recipes in C (1988)
   1st ed., section 7.1, "Portable Random Number Generators," p. 211.
   Product 421m needs just 27 bits, including the sign bit.
 */
#define SCRAMBLE(m) (((kjb_uint16)(m) * 421 + 17117) % 81000 & 0xFFFF)


#ifdef __cplusplus
extern "C" {
#endif


/* This mutex is used to protect all static structures in lib/l_mt.
 */
static kjb_pthread_mutex_t thread_master_lock = KJB_PTHREAD_MUTEX_INITIALIZER;

/* This thing is like a combination mutex and boolean flag, packaged together
   to make first-time initialization very easy. */
static kjb_pthread_once_t fs_kjb_pthreads_set_up = KJB_PTHREAD_ONCE_INIT;

/* The flag below is set JUST ONCE to true in a serial manner, when you create
   a thread.  It is nevermore written to, but afterwards it is READ
   without any serialization.  I think that should be safe. */
static int fs_kjb_pthreads_active = FALSE;

static kjb_pthread_key_t fs_kjb_pthread_props_key;

static kjb_pthread_t fs_primal_tid;

static int fs_kjb_pthread_counter = 0;

#define SEED_CT 3 /* Number of 16-bit words needed to hold a PRNG seed (48b).*/

/* This structure stores all the properties we wish to associate with each
   thread.  The 'seed1' and 'seed2' fields contain the seeds for random number
   generators used by the threads.
   For convenience, we also store the thread's userland worker function,
   and its argument.
 */
struct Thread_props
{
    kjb_uint16 seed1[SEED_CT]; /* seed used by calls to kjb_rand()           */
    kjb_uint16 seed2[SEED_CT]; /* seed used by calls to kjb_rand_2()         */
    int thread_counter;        /* one-based serial number of this thread     */
    void* (*pfun)(void*);      /* pointer to the program the user wants to   */
                               /* run in a thread                            */
    void* arg;                 /* argument to the user's invocation of *pfun */
};

/* The seed architecture here is based on that found in l_sys_rand.c, which
   only works on UNIX but not on NeXT.
 */
#ifdef NeXT
#error "KJB threads are not supported on NeXT systems."
#endif

#ifndef UNIX
#error "KJB threads are not supported on non-Unix systems."
#endif

/* NAN is defined in C99, but this is potentially C89, so we define it here. */
#ifndef NAN
#define NAN (0/0)
#endif



#ifdef KJB_HAVE_PTHREAD

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\  */

/* ============================================================================
 * STATIC                      kjb_random_multithread
 * ----------------------------------------------------------------------------
*/
/* Generate a random number
 *
 * This generates a random number for this thread ('this' as determined by
 * lookup) and for the 'generator_ix' stream.
 * If any of the support mechanisms (the mutex, the thread lookup) fail, then
 * this calls the bug handler.
 *
 * Precondition:  threads are already set up.
 *
 * LOCK STATUS:  no, this does not block
 *
 * Returns:
 *     Uniform-distributed real sample in the range [0, 1).
 * ----------------------------------------------------------------------------
*/

static double kjb_random_multithread(int generator_ix)
{
    kjb_pthread_t tid;
    struct Thread_props* p;

    if (generator_ix < 0 || 1 < generator_ix) goto cleanup;
    EGC(get_kjb_pthread_self(&tid));

    if (kjb_pthread_equal(tid, fs_primal_tid))
    {
        /* primal thread runs the standard code */
        return generator_ix ? kjb_rand_st() : kjb_rand_2_st();
    }

    /* all other threads use erand48() based on their respective seeds. */
    p=(struct Thread_props*) kjb_pthread_getspecific(fs_kjb_pthread_props_key);
    NGC(p);
    return erand48(generator_ix ? p -> seed1 : p -> seed2);

cleanup:
    add_error("Invalid behavior in kjb_random_multithread");
    SET_CANT_HAPPEN_BUG();
    return NAN; /* This line generates a warning sometimes, but it is ok. */
}


/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\  */

/* ============================================================================
 * STATIC                      kjb_rand_multithread
 * ----------------------------------------------------------------------------
*/
/* Generate a random number from the thread's first stream
 *
 * This implements the action of 'kjb_rand()' for a multithreaded program
 * using the kjb_pthreads wrapper.  In other words, once multithreading is set
 * up, a call to kjb_rand() is redirected (using a function pointer) hither.
 *
 * Returns:
 *     Uniform-distributed real sample in the range [0, 1).
 * ----------------------------------------------------------------------------
*/

static double kjb_rand_multithread()
{
    return kjb_random_multithread(0);
}



/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\  */

/* ============================================================================
 * STATIC                      kjb_rand_2_multithread
 * ----------------------------------------------------------------------------
*/
/* Generate a random number from the thread's second stream
 *
 * This implements the action of 'kjb_rand_2()' for a multithreaded program
 * using the kjb_pthreads wrapper.  In other words, once multithreading is set
 * up, a call to kjb_rand() is redirected (using a function pointer) hither.
 *
 * Returns:
 *     Uniform-distributed real sample in the range [0, 1).
 * ----------------------------------------------------------------------------
*/

static double kjb_rand_2_multithread()
{
    return kjb_random_multithread(1);
}




/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\  */

/* ============================================================================
 * STATIC                      generate_pthread_create_error_message
 * ----------------------------------------------------------------------------
*/
/* Interpret the error value from pthread_create() into a string
 *
 * LOCK STATUS:  no, this does not block
 *
 * ----------------------------------------------------------------------------
*/
static int generate_pthread_create_error_message(int pthread_result)
{
    ASSERT(pthread_result);
    if (EAGAIN == pthread_result)
    {
        set_error("Cannot create thread:  EAGAIN, too many threads");
    }
    else if (EINVAL == pthread_result)
    {
        set_error("Cannot create thread:  EINVAL, bad attributes");
    }
    else if (EPERM == pthread_result)
    {
        set_error("Cannot create thread:  EPERM, lack of privileges"
                        " to set scheduling policy in attributes");
    }
    else
    {
        set_error("Cannot create thread:  error code %d", pthread_result);
    }
    return ERROR;
}



/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\  */

/* ============================================================================
 * STATIC                      do_thread_cleanup
 * ----------------------------------------------------------------------------
*/
/* Every new thread must do this at its end of execution.
 *
 * The input argument is a pointer to the thread's individual
 * struct Thread_props object.  This object needs to be freed.
 * Also, since libkjb uses static resource tracking, calls to kjb_free
 * must be serialized using the thread_master_lock.
 *
 * LOCK STATUS:  yes, this locks/unlocks the thread_master_lock.
 *
 * ----------------------------------------------------------------------------
*/
static void do_thread_cleanup(void* v)
{
    if (v)
    {
        EPETE(kjb_pthread_mutex_lock(&thread_master_lock));
        kjb_free(v);
        EPETE(kjb_pthread_mutex_unlock(&thread_master_lock));
    }
}


/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\  */

/* ============================================================================
 * STATIC                      launchpad
 * ----------------------------------------------------------------------------
*/
/* This is the function that is handed to the POSIX pthread_create call.
 * It in turn calls the user's specified thread-worker-function.
 * ----------------------------------------------------------------------------
 */
static void* launchpad(void* v)
{
    struct Thread_props* p = (struct Thread_props*) v;

    NPETE(p);
    EPETE(kjb_pthread_setspecific(fs_kjb_pthread_props_key, p));

    /* run the user's function -- it might not return. */
    return (* p -> pfun)(p -> arg);
}



/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\  */

/* ============================================================================
 * STATIC                      generate_prng_seeds_unsafe
 * ----------------------------------------------------------------------------
*/
/* Generate, for a new thread, seeds for a "random" number stream
 *
 * Precondition:  thread_master_lock ought to be held (thus "_unsafe" name).
 *
 * Precondition:  only thread zero should call this.  Is that a precondition?
 *
 * We _could_ achieve repeatable pseudorandom numbers even if pthread_create
 * fails unpredictably (though we do not do so).  We just would have to cache
 * the seeds we generate and support "un-generate" functionality, like ungetc()
 * of stdio.h.  It is not even difficult, but still it's probably too paranoid.
 * If it turns out you want that, the untested code is there below; you would
 * also need to add the "un-generate" call in kjb_pthread_create_unsafe().
 *
 * This implementation gets its seeds from kjb_rand_2_st and then shuffles the
 * bits further using an auxiliary linear congruential generator whose
 * coefficients were found in Numerical Recipes.  The idea is that we want to
 * make sure we get the new seeds far apart in the sequence generated by
 * drand48.  I hypothesize that even if SCRAMBLE(m) returned m, these
 * generators would still exhibit low correlation, but adding a second level
 * of shuffling seems like good insurance.  The extra time required by SCRAMBLE
 * is insignificant compared with the setup time of creating a new thread.
 *
 * Future maintainers should appreciate what is going on here:  this is a
 * transformation from the kjb_rand_2 seed to new seeds for each thread.
 * This transformation must be "good."  A bad transformation is one that takes
 * the current location in the kjb_rand_2 sequence (a huge cycle), and hops
 * a small (e.g., a few thousand) sequential places away, for some thread.
 * An ideal transformation would enable you to generate enough (say, a hundred)
 * threads, each with a seed that is far apart (ideally, CYCLE_LENGTH/200) from
 * the nearest other seed.  (Note: log10(CYCLE_LENGTH/200) is about 12.)
 *
 * LOCK STATUS:  Requires thread_master_lock to be held as a precondition.
 *
 * ----------------------------------------------------------------------------
 */

static void generate_prng_seeds_unsafe(struct Thread_props* tr)
{
#if 1
    int i;

    if (NULL == tr) return;

    /* generate seed 1 */
    for (i = 0; i < 3; ++i)
    {
        tr -> seed1[i] = SCRAMBLE( kjb_rand_2_st() * UINT16_MAX );
    }

    /* generate seed 2 */
    for (i = 0; i < 3; ++i)
    {
        tr -> seed2[i] = SCRAMBLE( kjb_rand_2_st() * UINT16_MAX );
    }

#else /* SUPPORT FOR "UN-GENERATE" -- TOO ELABORATE. */

    static int store_count = 0;
    static struct Thread_rec storage; /* we retain the last generated seeds */
#ifdef TEST
    static int generated_seeds_already = FALSE;
#endif

    ASSERT(0 <= store_count);
    UNTESTED_CODE();
    if (tr)
    {
        if (0 == store_count)
        {
            int i;
            /* generate seed 1 */
            for (i = 0; i < 3; ++i)
            {
                tr -> seed1[i] = storage.seed1[i]
                               = SCRAMBLE( kjb_rand_2_st() * UINT16_MAX );
            }

            /* generate seed 2 */
            for (i = 0; i < 3; ++i)
            {
                tr -> seed2[i] = storage.seed2[i]
                               = SCRAMBLE( kjb_rand_2_st() * UINT16_MAX );
            }
#ifdef TEST
            generated_seeds_already = TRUE;
#endif
        }
        else
        {
            /* copy seed 1 from storage */
            for (i = 0; i < 3; ++i)
            {
                tr -> seed1[i] = storage.seed1[i];
            }

            /* copy seed 2 from storage */
            for (i = 0; i < 3; ++i)
            {
                tr -> seed2[i] = storage.seed2[i];
            }
            --store_count;
        }
    }
    else
    {
        /* Re-use the seeds we just generated, because the last customer just
           called back to say she has decided not to use them after all. */
        ASSERT(0 == store_count);
        ASSERT(generated_seeds_already);
        ++store_count;
    }
#endif
}


/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\  */

/* ============================================================================
 * STATIC                      kjb_pthread_create_unsafe
 * ----------------------------------------------------------------------------
*/
/* Launch a new thread
 *
 * This calls pthread_create, the library call that we wrap.  If that call
 * fails, we generate an error message.  Otherwise we finish setting up the
 * new thread, although those details are delegated to another helper function.
 *
 * Precondition:  thread_master_lock ought to be held (thus "_unsafe" name).
 *
 * Postcondition:  this starts the new pthread.
 *
 * LOCK STATUS:  Requires thread_master_lock to be held as a precondition.
 *
 * Returns:
 *     This returns NO_ERROR if successful, otherwise it sets an error message
 *     and returns ERROR.
 * ----------------------------------------------------------------------------
*/

static int kjb_pthread_create_unsafe(
    kjb_pthread_t* newthread,
    const kjb_pthread_attr_t* attr,
    void* (*pfun)(void*),
    void* arg
)
{
    int rc;
    struct Thread_props* p = NULL;

    ASSERT(newthread);
    ASSERT(pfun);
    NRE(p = TYPE_MALLOC(struct Thread_props));

    /* build the Thread_props structure */
    generate_prng_seeds_unsafe(p);
    p -> thread_counter = ++fs_kjb_pthread_counter;
    p -> pfun = pfun;
    p -> arg = arg;

    if ((rc = pthread_create(newthread, attr, &launchpad, p)))
    {
        kjb_free(p);
        return generate_pthread_create_error_message(rc);
    }
    return NO_ERROR;
}



/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\  */

/* ============================================================================
 * STATIC                      first_time_setup_unsafe
 * ----------------------------------------------------------------------------
*/
/* Precondition:  thread_master_lock ought to be held (thus "_unsafe" name).
 *
 * Precondition:  this function has never been called before, and is now
 *                being called by the primal thread, thread zero.
 *
 * Postcondition:  this intializes the thread wrapper.  That means,
 * 1. we store the thread id (TID) of thread zero;
 * 2. we set the "active" flag to true;
 * 3. we set up the wrapper's key to thread-specific data (TSD);
 * 4. we change the function pointers inside l/l_sys_rand.c to point here.
 *
 * LOCK STATUS:  Requires thread_master_lock to be held as a precondition.
 *
 * ----------------------------------------------------------------------------
*/
static void first_time_setup_unsafe()
{
    ASSERT(!fs_kjb_pthreads_active);

    /* create key, which we will use to store struct Thread_props per thread */
    EPETE(kjb_pthread_key_create(&fs_kjb_pthread_props_key,do_thread_cleanup));

    /* hook into the random number generators */
    EPETE(kjb_set_rand_function(& kjb_rand_multithread));
    EPETE(kjb_set_rand_2_function(& kjb_rand_2_multithread));

    /* cache the calling thread's TID */
    EPETE(get_kjb_pthread_self(&fs_primal_tid));

    /* set the active flag */
    fs_kjb_pthreads_active = TRUE;
}



/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\  */

/* ============================================================================
 * STATIC                      is_a_child_thread
 * ----------------------------------------------------------------------------
*/
/* Test whether this thread is not thread zero.  Return FALSE if thread zero.
 * I optimistically believe this function does not require serialization,
 * even though it accesses static data!  (But just as a reader!)
 *
 * LOCK STATUS:  no, this does not block
 *
 * ----------------------------------------------------------------------------
 */
static int is_a_child_thread(void)
{
    kjb_pthread_t tid;

    /* Determine whether a second (or later) thread has ever been launched. */
    if (! fs_kjb_pthreads_active)
    {
        return FALSE;
    }

    /* Determine whether this is thread zero.  (precondition:  active) */
    EPETE(get_kjb_pthread_self(&tid));

    if (kjb_pthread_equal(tid, fs_primal_tid))
    {
        return FALSE;
    }
    return TRUE;
}



#endif


/* ######################################################################## *
                         PUBLIC FUNCTIONS BELOW HERE
 * ######################################################################## */



/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\  */

/* ============================================================================
 *                             kjb_pthread_create
 *
 * Create a new additional thread
 *
 * This creates a new thread of control, sharing code and static data with the
 * original thread.
 *
 * The 'newthread' parameter is a pointer used to pass back the thread
 * identifier structure created by this function, if it is successful.
 * Please treat that structure as opaque:  please do not access the fields.
 * The 'attr' parameter lets you set the thread attributes,
 * but give a NULL argument to use the default attributes.
 * The 'start_routine' parameter is a function pointer that you must provide
 * which points to the function for the thread to execute.
 * The 'arg' parameter serves as the argument passed to the *start_routine.
 * To learn more, see the man page for pthread_create(3), which this wraps.
 *
 * Remember that many parts of the library use static data and thus are not
 * thread-safe.  Be sure to serialize library-based IO and memory allocation,
 * or anything that uses IO or memory allocation.
 *
 * LOCK STATUS:  yes, this locks/unlocks the thread_master_lock.
 *
 * Returns:
 *     This returns NO_ERROR if successful, otherwise it sets an error message
 *     and returns ERROR.  There are a large number of unlikely ways the
 *     function can fail, some of them disastrous.  So, it would be wise to
 *     check the result code.
 *
 * Index: threads
 *
 * Author:  Andrew Predoehl
 *
 * Documenter:  Andrew Predoehl
 *
 * ----------------------------------------------------------------------------
*/

int kjb_pthread_create(
    kjb_pthread_t* newthread,       /* (for output) thread ID created     */
    const kjb_pthread_attr_t* attr, /* optional attributes for new thread */
    void* (*start_routine)(void*),  /* function the new thread will run   */
    void* arg                       /* optional argument to start routine */
)
{
#ifndef KJB_HAVE_PTHREAD
    SET_MSG_RETURN_ERROR("create thread");

#else
    int result1, result2;

    NRE(newthread);
    NRE(start_routine);

    ERE(kjb_pthread_mutex_lock(&thread_master_lock));

    /* Must do extra setup for the very first time (or the equivalent). */
    ERE(kjb_pthread_once(&fs_kjb_pthreads_set_up, &first_time_setup_unsafe));

    /* Now let's try to fulfill the user's request. */
    result1 = kjb_pthread_create_unsafe(newthread, attr, start_routine, arg);
    result2 = kjb_pthread_mutex_unlock(&thread_master_lock);

    /* If unlock fails, we probably ought to panic (debatable).  Not ERE! */
    EPETE(result2);

    /* If we remove the above panic, then acknowledge either error. */
    return (ERROR == result1 || ERROR == result2) ? ERROR : NO_ERROR;
#endif
}


/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\  */

/* ============================================================================
 *                             kjb_pthread_mutex_lock
 *
 * Lock a mutex
 *
 * Request control of a mutex, and block if some other thread has already
 * claimed it.  The thread will unblock after that other thread releases the
 * mutex.
 *
 * LOCK STATUS:  no, this does not necessarily touch thread_master_lock
 *               (unless it is an argument to this function).
 *
 * Returns:
 *     This returns NO_ERROR if successful, otherwise it sets an error message
 *     and returns ERROR.
 *
 * Index: threads
 *
 * Author:  Andrew Predoehl
 *
 * Documenter:  Andrew Predoehl
 *
 * ----------------------------------------------------------------------------
*/

int kjb_pthread_mutex_lock(kjb_pthread_mutex_t* mutex)
{
#ifndef KJB_HAVE_PTHREAD
    SET_MSG_RETURN_ERROR("lock mutex");

#else
    int rc, result = ERROR;
    NRE(mutex);
    switch (rc = pthread_mutex_lock(mutex))
    {
        case 0:
            result = NO_ERROR;
            break;
        case EINVAL:
            set_error("Cannot lock mutex:  EINVAL, mutex not initialized");
            break;
        case EDEADLK:
            set_error("Cannot lock mutex:  EDEADLK, mutex already locked");
            break;
        default:
            set_error("Cannot lock mutex:  error code %d", rc);
            break;
    }
    return result;
#endif
}


/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\  */

/* ============================================================================
 *                             kjb_pthread_mutex_trylock
 *
 * Nonblocking attempt to lock a mutex
 *
 * This will tentatively attempt to lock a mutex, but if the mutex is already
 * held, this routine will NOT block, but instead immediately return
 * the WOULD_BLOCK result code.
 *
 * LOCK STATUS:  no, this does not necessarily touch thread_master_lock
 *               (unless it is an argument to this function).
 *
 * Returns:
 *     NO_ERROR means this successfully locked the mutex.
 *     WOULD_BLOCK means the mutex is currently held, so we cannot lock it
 *                 ourselves right now.  (This is comparable to the pthreads
 *                 value EBUSY, but the numerical values are almost surely
 *                 different.)  This value is defined in l/l_def.h.
 *     ERROR indicates some sort of trouble, with a description in the global
 *           error message.
 *
 * Index: threads
 *
 * Author:  Andrew Predoehl
 *
 * Documenter:  Andrew Predoehl
 *
 * ----------------------------------------------------------------------------
*/

int kjb_pthread_mutex_trylock(kjb_pthread_mutex_t* mutex)
{
#ifndef KJB_HAVE_PTHREAD
    SET_MSG_RETURN_ERROR("trylock mutex");

#else
    int rc, result = ERROR;
    NRE(mutex);
    switch (rc = pthread_mutex_trylock(mutex))
    {
        case 0:
            result = NO_ERROR;
            break;
        case EBUSY:
            result = WOULD_BLOCK; /* this is a libkjb-specific code */
            break;
        case EINVAL:
            set_error("Cannot trylock mutex:  EINVAL, mutex not initialized");
            break;
        default:
            set_error("Cannot trylock mutex:  error code %d", rc);
            break;
    }
    return result;
#endif
}


/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\  */

/* ============================================================================
 *                             kjb_pthread_mutex_unlock
 *
 * Release a mutex
 *
 * This unlocks (releases) a given mutex.
 *
 * Returns:
 *     This returns NO_ERROR if successful, otherwise it sets an error message
 *     and returns ERROR.
 *
 * LOCK STATUS:  no, this does not block
 *
 * Index: threads
 *
 * Author:  Andrew Predoehl
 *
 * Documenter:  Andrew Predoehl
 *
 * ----------------------------------------------------------------------------
*/

int kjb_pthread_mutex_unlock(kjb_pthread_mutex_t* mutex)
{
#ifndef KJB_HAVE_PTHREAD
    SET_MSG_RETURN_ERROR("unlock mutex");

#else
    int rc, result = ERROR;
    NRE(mutex);
    switch (rc = pthread_mutex_unlock(mutex))
    {
        case 0:
            result = NO_ERROR;
            break;
        case EINVAL:
            set_error("Cannot unlock mutex:  EINVAL, mutex not initialized");
            break;
        case EPERM:
            set_error("Cannot unlock mutex:  EPERM, this thread is not owner");
            break;
        default:
            set_error("Cannot unlock mutex:  error code %d", rc);
            break;
    }
    return result;
#endif
}


/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\  */

/* ============================================================================
 *                             kjb_pthread_equal
 *
 * Test whether two thread IDs are the same
 *
 * Test whether two thread identity structures indicate the same thread.
 * A return value of TRUE means they both indicate the same thread.
 * The TRUE and FALSE return values are defined in l/l_def.h
 *
 * LOCK STATUS:  no, this does not block
 *
 * Returns:
 *     TRUE or FALSE
 *
 * Index: threads
 *
 * Author:  Andrew Predoehl
 *
 * Documenter:  Andrew Predoehl
 *
 * ----------------------------------------------------------------------------
*/

int kjb_pthread_equal(kjb_pthread_t tid1, kjb_pthread_t tid2)
{
#ifndef KJB_HAVE_PTHREAD
    SET_MSG_RETURN_ERROR("test thread identity");

#else
    return pthread_equal(tid1, tid2) ? TRUE : FALSE;
#endif
}


/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\  */

/* ============================================================================
 *                             get_kjb_pthread_self
 *
 * Find this thread's own identity structure
 *
 * Use this instead of pthread_self().
 * This looks up and returns (via an output pointer) the thread ID structure
 * for the calling thread's own identity.
 * The input pointer 'tid' must not equal to NULL.
 *
 * LOCK STATUS:  no, this does not block
 *
 * Returns:
 *     This returns NO_ERROR if successful, otherwise it sets an error message
 *     and returns ERROR.
 *
 * Index: threads
 *
 * Author:  Andrew Predoehl
 *
 * Documenter:  Andrew Predoehl
 *
 * ----------------------------------------------------------------------------
*/

int get_kjb_pthread_self(kjb_pthread_t* tid)
{
#ifndef KJB_HAVE_PTHREAD
    SET_MSG_RETURN_ERROR("find this thread's self-identity");

#else
    NRE(tid);
    *tid = pthread_self();
    return NO_ERROR;
#endif
}


/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\  */

/* ============================================================================
 *                             kjb_pthread_exit
 *
 * End this thread's execution
 *
 * This causes the calling thread to finish its own execution, returning a
 * return value in the form of a void pointer.  (The return value can be read
 * by some other thread that performs a thread join.)
 *
 * Returns:
 *     This returns NO_ERROR if successful, otherwise it sets an error message
 *     and returns ERROR.
 *
 * LOCK STATUS:  no, this does not block
 *
 * Index: threads
 *
 * Author:  Andrew Predoehl
 *
 * Documenter:  Andrew Predoehl
 *
 * ----------------------------------------------------------------------------
*/

void kjb_pthread_exit(void* rval_p)
{
#ifndef KJB_HAVE_PTHREAD
    /* 
     * Kobus. This does not return! 
     *
    SET_MSG_RETURN_ERROR("exit thread");
    */
#else
    pthread_exit(rval_p);
#endif
}


/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\  */

/* ============================================================================
 *                             kjb_pthread_join
 *
 * Merge some other thread's execution into this one
 *
 * This function blocks the calling thread, and waits for some other thread,
 * specified by 'tid,' to end.  The return value of that other thread can be
 * retrieved via the output pointer 'rval_p.'  If 'rval_p' equals NULL then
 * that return value is ignored.
 *
 * LOCK STATUS:  no, this does not block
 *
 * Returns:
 *     This returns NO_ERROR if successful, otherwise it sets an error message
 *     and returns ERROR.
 *
 * Index: threads
 *
 * Author:  Andrew Predoehl
 *
 * Documenter:  Andrew Predoehl
 *
 * ----------------------------------------------------------------------------
*/

int kjb_pthread_join(kjb_pthread_t tid, void** rval_p)
{
#ifndef KJB_HAVE_PTHREAD
    SET_MSG_RETURN_ERROR("join thread");

#else
    int rc, result = ERROR;
    switch (rc = pthread_join(tid, rval_p))
    {
        case 0:
            result = NO_ERROR;
            break;
        case EDEADLK:
            set_error("Cannot join thread:  EDEADLK, deadlock detected");
            break;
        case EINVAL:
            set_error("Cannot join thread:  EINVAL, thread isn't joinable");
            break;
        case ESRCH:
            set_error("Cannot join thread:  ESRCH, tid is unrecognized");
            break;
        default:
            set_error("Cannot join thread:  error code %d", rc);
            break;
    }
    return result;
#endif
}


/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\  */

/* ============================================================================
 *                             kjb_pthread_cancel
 *
 * Try to cancel some thread
 *
 * This attempts to abort the execution of some other thread, specified by
 * 'tid.'
 * This function does not block, it merely queues the request.
 * See the man page for pthread_cancel(3) for a discussion of what happens.
 *
 * LOCK STATUS:  no, this does not block
 *
 * Returns:
 *     This returns NO_ERROR if successful, otherwise it sets an error message
 *     and returns ERROR.
 *
 * Index: threads
 *
 * Author:  Andrew Predoehl
 *
 * Documenter:  Andrew Predoehl
 *
 * ----------------------------------------------------------------------------
*/

int kjb_pthread_cancel(kjb_pthread_t tid)
{
#ifndef KJB_HAVE_PTHREAD
    SET_MSG_RETURN_ERROR("cancel thread");

#else
    int rc, result = ERROR;
    switch (rc = pthread_cancel(tid))
    {
        case 0:
            result = NO_ERROR;
            break;
        case ESRCH:
            set_error("Cannot cancel thread:  ESRCH, tid is unrecognized");
            break;
        default:
            set_error("Cannot cancel thread:  error code %d", rc);
            break;
    }
    return result;
#endif
}




/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\  */

/* ============================================================================
 *                             kjb_pthread_mutex_init
 *
 * Initialize a mutex object
 *
 * This is used for mutexes not stored in static memory (i.e., on the stack or
 * heap).  A mutex needs to be properly initialized, and this will do it.
 *
 * LOCK STATUS:  no, this does not block
 *
 * Returns:
 *      NO_ERROR if successful.  Otherwise, this sets an error message and
 *      returns ERROR.
 *
 * Index: threads
 *
 * Author:  Andrew Predoehl
 *
 * Documenter:  Andrew Predoehl
 *
 * ----------------------------------------------------------------------------
*/
int kjb_pthread_mutex_init(
    kjb_pthread_mutex_t* mutex,
    kjb_pthread_mutexattr_t* attr
)
{
#ifndef KJB_HAVE_PTHREAD
    SET_MSG_RETURN_ERROR("initialize mutex");

#else
    int rc;
    NRE(mutex);
    if ((rc = pthread_mutex_init(mutex, attr)))
    {
        set_error("Unable to initialize mutex: code %d", rc);
        return ERROR;
    }
    return NO_ERROR;
#endif
}





/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\  */

/* ============================================================================
 *                             kjb_pthread_mutex_destroy
 *
 * Destroy a mutex object
 *
 * On some platforms, a mutex might require resource allocation, in which case
 * this will release the resources.  In any case, this tests whether the mutex
 * is held when you try to destroy it:  that is an error.
 *
 * LOCK STATUS:  no, this does not block
 *
 * Returns:
 *      NO_ERROR, if successful.  If the mutex is locked this returns ERROR and
 *      sets an error message.
 *
 * Index: threads
 *
 * Author:  Andrew Predoehl
 *
 * Documenter:  Andrew Predoehl
 *
 * ----------------------------------------------------------------------------
*/
int kjb_pthread_mutex_destroy(kjb_pthread_mutex_t* mutex)
{
#ifndef KJB_HAVE_PTHREAD
    SET_MSG_RETURN_ERROR("destroy mutex");

#else
    int rc, result = ERROR;
    switch (rc = pthread_mutex_destroy(mutex))
    {
        case 0:
            result = NO_ERROR;
            break;
        case EBUSY:
            set_error("Cannot destroy mutex:  EBUSY, mutex is locked.");
            break;
        default:
            set_error("Error %d while trying to destroy a mutex.", rc);
            break;
    }
    return result;
#endif
}





/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\  */

/* ============================================================================
 *                             kjb_multithread_wrapper_serialization_lock
 *
 * Lock the thread master lock
 *
 * WARNING!  This function is not intended for widespread use.  It
 * is used to serialize access to the resources used by the pthreads wrapper
 * itself.  Irresponsible use of this function will cause problems.
 * Once this lock is held, it is safe to access resources static to the
 * wrapper.  
 *
 * LOCK STATUS:  this locks the thread_master_lock (but does not unlock it).
 *
 * Returns:
 *      NO_ERROR, if successful.  If the mutex cannot be locked this returns
 *      ERROR and sets an error message.
 *
 * Index: threads
 *
 * Author:  Andrew Predoehl
 *
 * Documenter:  Andrew Predoehl
 *
 * ----------------------------------------------------------------------------
*/
int kjb_multithread_wrapper_serialization_lock()
{
    return kjb_pthread_mutex_lock(&thread_master_lock);
}




/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\  */

/* ============================================================================
 *                             kjb_multithread_wrapper_serialization_unlock
 *
 * Unlock the thread master lock
 *
 * WARNING!  This function is not intended for widespread use.  It
 * is used to serialize access to the resources used by the pthreads wrapper
 * itself.  Irresponsible use of this function will cause problems.
 *
 * LOCK STATUS:  this (heedlessly) attempts to unlock the thread_master_lock
 *
 * Returns:
 *      NO_ERROR, if successful.  If the mutex cannot be unlocked this returns
 *      ERROR and sets an error message.
 *
 * Index: threads
 *
 * Author:  Andrew Predoehl
 *
 * Documenter:  Andrew Predoehl
 *
 * ----------------------------------------------------------------------------
*/
int kjb_multithread_wrapper_serialization_unlock()
{
    return kjb_pthread_mutex_unlock(&thread_master_lock);
}





/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\  */

/* ============================================================================
 *                             kjb_pthread_key_create
 *
 * Create a key for thread-specific data
 *
 * This tries to generate a key for thread-specific data.  If successful,
 * each thread may use this key to access an associated void pointer.
 * The 'destr_function' pointer, if not equal to NULL, is a destructor function
 * called when the thread exits or is cancelled.  The argument passed to the
 * destructor function is the thread's associated void pointer.
 *
 * For example, very often the associated void pointer is a blob of memory
 * used by the thread, and the destr_function is equal to &free or
 * &kjb_mt_free.
 *
 * The total number of allowable keys is limited to KJB_PTHREAD_KEYS_MAX.
 * This is less than PTHREAD_KEYS_MAX because the wrapper itself uses one key.
 *
 * LOCK STATUS:  no, this does not block
 *
 * Returns:
 *      NO_ERROR, if successful.  Otherwise, ERROR and a message is generated.
 *
 * Related:  kjb_pthread_getspecific, kjb_pthread_setspecific
 *           kjb_pthread_key_delete, kjb_mt_free
 *
 * Index: threads
 *
 * Author:  Andrew Predoehl
 *
 * Documenter:  Andrew Predoehl
 *
 * ----------------------------------------------------------------------------
*/
int kjb_pthread_key_create(
    kjb_pthread_key_t *key,
    void (*destr_function) (void *)
)
{
#ifndef KJB_HAVE_PTHREAD
    SET_MSG_RETURN_ERROR("create TSD key");

#else
    int rc, result = ERROR;
    switch (rc = pthread_key_create(key, destr_function))
    {
        case 0:
            result = NO_ERROR;
            break;
        case EAGAIN:
            set_error("Cannot create pthread key:  EAGAIN, out of keys");
            break;
        default:
            set_error("Cannot create pthread key:  error code %d", rc);
            break;
    }
    return result;
#endif
}



/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\  */

/* ============================================================================
 *                             kjb_pthread_key_delete
 *
 * Delete a key for thread-specific data
 *
 * This eliminates the key regardless of whether there is associated data.
 * Even if there is data, the data destructor is not called.
 *
 * LOCK STATUS:  no, this does not block
 *
 * Returns:
 *      NO_ERROR, if successful.  Otherwise, ERROR and a message is generated.
 *
 * Index: threads
 *
 * Related:  kjb_pthread_getspecific, kjb_pthread_setspecific
 *           kjb_pthread_key_create
 *
 * Author:  Andrew Predoehl
 *
 * Documenter:  Andrew Predoehl
 *
 * ----------------------------------------------------------------------------
*/
int kjb_pthread_key_delete(kjb_pthread_key_t key)
{
#ifndef KJB_HAVE_PTHREAD
    SET_MSG_RETURN_ERROR("delete TSD key");

#else
    int rc, result = ERROR;
    switch (rc = pthread_key_delete(key))
    {
        case 0:
            result = NO_ERROR;
            break;
        case EINVAL:
            set_error("Cannot delete pthread key:  EINVAL, key is invalid.");
            break;
        default:
            set_error("Cannot delete pthread key:  code %d", rc);
            break;
    }
    return result;
#endif
}




/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\  */

/* ============================================================================
 *                             kjb_pthread_setspecific
 *
 * Associate data to a thread-specific key
 *
 * You can store thread-specific data, associated with a key, if you want to
 * store (presumably just a little) data that is, in spirit, not shared with
 * other threads.  For example, the random number generator seeds for each
 * thread are not shared; there are seed values specific to each thread.
 * See pthread_setspecific for an example of how this works.
 *
 * LOCK STATUS:  no, this does not block
 *
 * Returns:
 *      NO_ERROR, if successful.  Otherwise, ERROR and a message is generated.
 *
 * Related:  kjb_pthread_getspecific,
 *           kjb_pthread_key_create, kjb_pthread_key_delete
 *
 * Index: threads
 *
 * Author:  Andrew Predoehl
 *
 * Documenter:  Andrew Predoehl
 *
 * ----------------------------------------------------------------------------
*/
int kjb_pthread_setspecific(kjb_pthread_key_t key, const void *pointer)
{
#ifndef KJB_HAVE_PTHREAD
    SET_MSG_RETURN_ERROR("associate data to a TSD key");

#else
    int rc, result = ERROR;
    switch (rc = pthread_setspecific(key, pointer))
    {
        case 0:
            result = NO_ERROR;
            break;
        case EINVAL:
            set_error("Cannot set thread-specific data:  EINVAL, invalid key");
            break;
        default:
            set_error("Cannot do kjb_pthread_setspecific:  code %d", rc);
            break;
    }
    return result;
#endif
}





/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\  */

/* ============================================================================
 *                             kjb_pthread_getspecific
 *
 * Fetch data for a thread-specific key
 *
 * This function provides read access to thread-specific data that you have
 * associated with a key.  Please see the notes for kjb_pthread_setspecific.
 *
 * LOCK STATUS:  no, this does not block
 *
 * Returns:
 *      The associated pointer, if successful.  Otherwise, NULL, and an
 *      error message is set.
 *
 * Related:  kjb_pthread_setspecific,
 *           kjb_pthread_key_create, kjb_pthread_key_delete
 *
 * Index: threads
 *
 * Author:  Andrew Predoehl
 *
 * Documenter:  Andrew Predoehl
 *
 * ----------------------------------------------------------------------------
*/
void* kjb_pthread_getspecific(kjb_pthread_key_t key)
{
#ifndef KJB_HAVE_PTHREAD
    set_error("Cannot look up TSD key: program linked without libpthread."); 
    return NULL;                   
#else
    void* p = pthread_getspecific(key);
    if (NULL == p)
    {
        set_error("kjb_pthread_getspecific key lookup failed");
    }
    return p;
#endif
}





/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\  */

/* ============================================================================
 *                             kjb_pthread_once
 *
 * Perform some kind of initialization just once
 *
 * This works with a kjb_pthread_once_t flag as an atomic flip-binary-flag
 * routine, useful for initializing things.  The 'once_control' pointer must
 * point to a static or external variable initialized to KJB_PTHREAD_ONCE_INIT
 * and will execute (*init_routine)() just one time.  Subsequent invocations
 * with the same 'once_control' pointer will do nothing.
 *
 * LOCK STATUS:  no, this does not block
 *
 * Returns:
 *      ERROR if either input pointer equals NULL, otherwise NO_ERROR.
 *
 * Index: threads
 *
 * Author:  Andrew Predoehl
 *
 * Documenter:  Andrew Predoehl
 *
 * ----------------------------------------------------------------------------
*/
int kjb_pthread_once(
    kjb_pthread_once_t* once_control,
    void (*init_routine)(void)
)
{
#ifndef KJB_HAVE_PTHREAD
    SET_MSG_RETURN_ERROR("do one-time initialization");

#else
    NRE(once_control);
    NRE(init_routine);
    pthread_once(once_control, init_routine);
    return NO_ERROR;
#endif
}




/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\  */

/* ============================================================================
 *                             kjb_pthread_read_prng_seeds
 *
 * Read PRNG seeds for this thread
 *
 * When called from a thread started with kjb_pthread_create,
 * this writes the current state of the pseudo-random number generators (PRNGs)
 * into the buffer indicated by input pointer 's' (which is expected not to
 * equal NULL).  Parameter 'buf_length'
 * must indicate the number of cells (not bytes!) of the buffer, and if it
 * is insufficient this returns ERROR.
 *
 * Depending on the size of the seeds, which we are deliberately leaving
 * unspecified, this will write to locations s[0] up to s[buf_length-1] with
 * the seed contents, but it will write less than that if buf_length
 * exceeds the size of the seed containers.  As stated, if buf_length is
 * inadequate to hold all the seed bits, this function returns ERROR.
 *
 * This routine does NOT provide
 * access to the seed bits of the primal thread of the main() function, a.k.a.
 * thread zero.  If this routine is called from thread zero, this returns
 * ERROR.  From any other thread, this returns the current seed bits of that
 * thread, which we hope will be unique relative to the other threads and
 * past and future calls.
 *
 * This routine is very low-level and not intended to be used
 * except for library introspection and self-testing.
 * To get repeatable random numbers from library code, you need only
 * call set_random_options(), or kjb_seed_rand() and kjb_seed_rand_2().
 * The seeds for the subsequent threads are all derived from the kjb_rand_2()
 * stream of thread zero.
 *
 * Informally, as of Fall 2013, the library uses two PRNGs each with 48 bits
 * of state, and thus a buf_length of 6 is sufficient to hold all the seed
 * bits.  Future library revisions might change this.
 *
 * LOCK STATUS:  yes, this locks/unlocks the thread_master_lock.
 *
 * Returns:
 *      ERROR if unsuccessful, with a message; otherwise NO_ERROR.
 *
 * Index: threads, random
 *
 * Related:  set_random_options, kjb_seed_rand, kjb_seed_rand_2,
 *           kjb_seed_rand_with_tod, kjb_seed_rand_2_with_tod,
 *           get_rand_seed
 *
 * Author:  Andrew Predoehl
 *
 * Documenter:  Andrew Predoehl
 *
 * ----------------------------------------------------------------------------
*/
int kjb_pthread_read_prng_seeds(kjb_uint16* s, size_t buf_length)
{
#ifndef KJB_HAVE_PTHREAD
    SET_MSG_RETURN_ERROR("read prng seeds");

#else
    struct Thread_props* p;
    int result = ERROR;

    NRE(s);

    /* Buffer must hold seeds of two streams, each with SEED_CT words. */
    if (buf_length < 2*SEED_CT)
    {
        set_error("Buffer (size %u) is too small to store seed bits; "
                  "length %u required.", buf_length, 2*SEED_CT);
        return ERROR;
    }
    buf_length = MIN_OF(buf_length, 2*SEED_CT);

    ERE(kjb_pthread_mutex_lock(&thread_master_lock));

    if (!is_a_child_thread())
    {
        set_error("This function does not return seeds for thread zero.");
        NOTE_ERROR();
        goto cleanup;
    }

    /* all other threads use erand48() based on their respective seeds. */
    p=(struct Thread_props*) kjb_pthread_getspecific(fs_kjb_pthread_props_key);
    NGC(p);

    /* copy seed bits */
    kjb_memcpy((char*) s, (char*) p -> seed1, SEED_CT * sizeof(kjb_uint16));
    s += SEED_CT;
    kjb_memcpy((char*) s, (char*) p -> seed2, SEED_CT * sizeof(kjb_uint16));
    result = NO_ERROR;

cleanup:
    ERE(kjb_pthread_mutex_unlock(&thread_master_lock));
    return result;
#endif
}




/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\  */

/* ============================================================================
 *                             kjb_pthread_attr_init
 *
 * Initialize an attribute object
 *
 * This initializes an attribute object for a thread.  Such attributes are used
 * to control the behavior of new threads.  On some platforms, attribute
 * intialization requires a memory allocation, but on other platforms not so.
 * Thus this call could fail; the object should be destroyed when it is no
 * longer needed; and absent an intervening destruction, this function should
 * not be called twice on the same object.
 *
 * C++ developers: consider using class kjb::pthread_attr instead.
 *
 * LOCK STATUS:  no, this does not block
 *
 * Returns:
 *      ERROR if the initialization fails, with a message.  Otherwise NO_ERROR.
 *
 * Index: threads
 *
 * Related:
 *      kjb_pthread_attr_destroy
 *
 * Author:  Andrew Predoehl
 *
 * Documenter:  Andrew Predoehl
 *
 * ----------------------------------------------------------------------------
*/
int kjb_pthread_attr_init(kjb_pthread_attr_t* attr_p)
{
#ifndef KJB_HAVE_PTHREAD
    SET_MSG_RETURN_ERROR("initialize thread attribute");

#else
    int rc, result = ERROR;
    NRE(attr_p);
    switch (rc = pthread_attr_init(attr_p))
    {
        case 0:
            result = NO_ERROR;
            break;
        case ENOMEM:
            set_error("Cannot initialize attribute: out of memory");
            break;
        default:
            set_error("Cannot initialize attribute: error %d", rc);
            break;
    }
    return result;
#endif
}





/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\  */

/* ============================================================================
 *                             kjb_pthread_attr_destroy
 *
 * Destroy an attribute object
 *
 * This destroys an attribute object for a thread.  This is needed because
 * attributes may, on any platform, use dynamic resources that must be freed.
 *
 * C++ developers: consider using class kjb::pthread_attr instead.
 *
 * LOCK STATUS:  no, this does not block
 *
 * Returns:
 *      ERROR if destruction fails, with a message.  Otherwise NO_ERROR.
 *
 * Index: threads
 *
 * Related:
 *      kjb_pthread_attr_init
 *
 * Author:  Andrew Predoehl
 *
 * Documenter:  Andrew Predoehl
 *
 * ----------------------------------------------------------------------------
*/
int kjb_pthread_attr_destroy(kjb_pthread_attr_t* attr_p)
{
#ifndef KJB_HAVE_PTHREAD
    SET_MSG_RETURN_ERROR("destroy thread attribute");

#else
    int rc, result = ERROR;
    NRE(attr_p);
    switch (rc = pthread_attr_destroy(attr_p))
    {
        case 0:
            result = NO_ERROR;
            break;
        default:
            set_error("Cannot destroy attribute: error %d", rc);
            break;
    }
    return result;
#endif
}






/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\  */

/* ============================================================================
 *                             kjb_pthread_attr_setdetachstate
 *
 * Set the detach state of a thread attribute
 *
 * This controls the "detach" state (i.e., joinable or detached) of a
 * thread attribute object.  Valid values for 'state' are
 *
 * | KJB_PTHREAD_CREATE_JOINABLE
 * | KJB_PTHREAD_CREATE_DETACHED
 *
 * . . . with the first being the default (at least, as of this writing).
 *
 * LOCK STATUS:  no, this does not block
 *
 * Returns:
 *      ERROR if the state is invalid, or anything else goes wrong, with a
 *      message.  Otherwise NO_ERROR.
 *
 * Index: threads
 *
 * Related:
 *      kjb_pthread_attr_getdetachstate
 *
 * Author:  Andrew Predoehl
 *
 * Documenter:  Andrew Predoehl
 *
 * ----------------------------------------------------------------------------
*/
int kjb_pthread_attr_setdetachstate(kjb_pthread_attr_t* attr_p, int state)
{
#ifndef KJB_HAVE_PTHREAD
    SET_MSG_RETURN_ERROR("set thread attribute");

#else
    int rc, result = ERROR;
    NRE(attr_p);
    switch (rc = pthread_attr_setdetachstate(attr_p, state))
    {
        case 0:
            result = NO_ERROR;
            break;
        case EINVAL:
            set_error("Cannot set attribute's detach state: invalid state");
            break;
        default:
            set_error("Cannot set attribute's detach state: error %d", rc);
            break;
    }
    return result;
#endif
}



/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\  */

/* ============================================================================
 *                             kjb_pthread_attr_getdetachstate
 *
 * Get the detach state of a thread attribute
 *
 * This queries the "detach" state (i.e., joinable or detached) of a
 * thread attribute object.
 *
 * LOCK STATUS:  no, this does not block
 *
 * Returns:
 *      ERROR anything goes wrong, with a message.  Otherwise NO_ERROR.
 *
 * Index: threads
 *
 * Related:
 *      kjb_pthread_attr_setdetachstate
 *
 * Author:  Andrew Predoehl
 *
 * Documenter:  Andrew Predoehl
 *
 * ----------------------------------------------------------------------------
*/
int kjb_pthread_attr_getdetachstate(kjb_pthread_attr_t* attr_p, int* state)
{
#ifndef KJB_HAVE_PTHREAD
    SET_MSG_RETURN_ERROR("get thread attribute");

#else
    int rc, result = ERROR;
    NRE(attr_p);
    NRE(state);
    switch (rc = pthread_attr_getdetachstate(attr_p, state))
    {
        case 0:
            result = NO_ERROR;
            break;
        default:
            set_error("Cannot get attribute's detach state: error %d", rc);
            break;
    }
    return result;
#endif
}



/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\  */

/* ============================================================================
 *                             get_kjb_pthread_number
 *
 * Get this thread's identity index
 *
 * Sometimes a thread wants to know, "who am I?"  In other words, what is the
 * identity info of this thread?
 *
 * The native Pthreads way to answer to this question
 * is available via our wrapped function get_kjb_pthread_self().
 * Unfortunately it only returns a kjb_pthread_t object, which is
 * disappointing, because it cannot be printed, and it can only be compared
 * to that of other threads with kjb_pthread_equal().  You cannot order
 * these structures in a portable way.
 *
 * As an alternative, you can use this function to get an integer that
 * identifies this thread.  The original, primal thread that started the
 * program always and only gets value 0.  All threads created with
 * kjb_pthread_create() get a positive integer identity.  Unless the counter
 * is reset, the threads will all get identity integers increasing by one in
 * the order that they were created (with the caveat below).
 *
 * Use reset_kjb_pthread_counter() to re-start the identity values at one.
 *
 * Caveat:
 * I've observed some funny situations with threads that suggest
 * that thread identity sometimes is permuted slightly from thread creation
 * order -- I am not sure what to make of that.  The condition is intermittent
 * and hard to replicate, and lately it has gone away entirely, but who knows
 * if it could return.  So I would recommend threads use this function rather
 * than receive a creation order number as (or inside) its startup argument.
 *
 * LOCK STATUS:  no, this does not block
 *
 * Returns:
 *      Identity integer of the calling thread.
 *
 * Index: threads
 *
 * Related:
 *      get_kjb_pthread_self, reset_kjb_pthread_counter
 *
 * Author:  Andrew Predoehl
 *
 * Documenter:  Andrew Predoehl
 *
 * ----------------------------------------------------------------------------
*/
int get_kjb_pthread_number(void)
{
#ifndef KJB_HAVE_PTHREAD
    SET_MSG_RETURN_ERROR("get thread number");

#else
    struct Thread_props* p;

    if (!is_a_child_thread()) return 0;

    p=(struct Thread_props*) kjb_pthread_getspecific(fs_kjb_pthread_props_key);
    NPETE(p);
    return p -> thread_counter;
#endif
}


/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\  */

/* ============================================================================
 *                             reset_kjb_pthread_counter
 *
 * Reset the thread identity indices back to one.
 *
 * Please see the documentation for get_kjb_pthread_number().
 *
 * After calling this function, the next thread to be created with
 * kjb_pthread_create() will have a thread identity integer of one,
 * the next will be two, and so on,
 * even if they are not the first, second, etc. threads to be spawned.  
 * In other words, the counter starts over again.
 *
 * Obviously this could cause different threads to have the same identity
 * integer -- that is your problem to handle.
 *
 * LOCK STATUS:  no, this does not block
 *
 * Index: threads
 *
 * Related:
 *      get_kjb_pthread_self, get_kjb_pthread_number
 *
 * Author:  Andrew Predoehl
 *
 * Documenter:  Andrew Predoehl
 *
 * ----------------------------------------------------------------------------
*/
void reset_kjb_pthread_counter(void)
{
    fs_kjb_pthread_counter = 0;
}


#ifdef __cplusplus
}
#endif

