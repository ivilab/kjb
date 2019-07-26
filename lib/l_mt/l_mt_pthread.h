/*
 * $Id: l_mt_pthread.h 17430 2014-09-01 18:43:22Z predoehl $
 */

#ifndef L_MT_PTHREAD_H_LIBKJB_INCLUDED
#define L_MT_PTHREAD_H_LIBKJB_INCLUDED 1

#include <l/l_sys_def.h>

#ifdef KJB_HAVE_PTHREAD
#include <pthread.h>
#endif

#ifdef __cplusplus
extern "C" {
#ifdef COMPILING_CPLUSPLUS_SOURCE
namespace kjb_c {
#endif
#endif

#ifdef KJB_HAVE_PTHREAD

/* -------------------------------------------------------------------------- */

/* ============================================================================
 *                             kjb_pthread_t
 *
 * Opaque thread identity structure
 *
 * This wraps up the POSIX thread (pthread) entity pthread_t for use in
 * the library.  The definition here is subject to change:  we could add
 * more fields in the future, so don't assume this will always be just
 * a simple typedef.
 *
 * Index: threads
 *
 * ----------------------------------------------------------------------------
*/
typedef pthread_t               kjb_pthread_t;

typedef pthread_attr_t          kjb_pthread_attr_t;

typedef pthread_mutex_t         kjb_pthread_mutex_t;

typedef pthread_mutexattr_t     kjb_pthread_mutexattr_t;

typedef pthread_key_t           kjb_pthread_key_t;

typedef pthread_once_t          kjb_pthread_once_t;

/* Mutex initializers */
#define KJB_PTHREAD_MUTEX_INITIALIZER \
                PTHREAD_MUTEX_INITIALIZER

/* The cleanup push and cleanup pop operations are often macros. */
#define kjb_pthread_cleanup_push(f, x) \
                pthread_cleanup_push(f, x)
#define kjb_pthread_cleanup_pop(x) \
                pthread_cleanup_pop(x)

#define KJB_PTHREAD_KEYS_MAX \
                ((PTHREAD_KEYS_MAX) - 1)

#define KJB_PTHREAD_ONCE_INIT \
                PTHREAD_ONCE_INIT

#define KJB_PTHREAD_CREATE_JOINABLE \
                PTHREAD_CREATE_JOINABLE
#define KJB_PTHREAD_CREATE_DETACHED \
                PTHREAD_CREATE_DETACHED


#else    /* ############################################################## */
/*
 * FAKE STUFF!  Use this when the system lacks pthread.h or for NO_LIBS builds.
 */

typedef void*                                         kjb_pthread_t;
typedef void*                                         kjb_pthread_attr_t;
typedef void*                                         kjb_pthread_mutex_t;
typedef void*                                         kjb_pthread_mutexattr_t;
typedef int                                           kjb_pthread_key_t;
typedef int                                           kjb_pthread_once_t;
#define KJB_PTHREAD_MUTEX_INITIALIZER                 NULL
#define kjb_pthread_cleanup_push(f, x)                do { ; } while(0)
#define kjb_pthread_cleanup_pop(x)                    do { ; } while(0)
#define KJB_PTHREAD_KEYS_MAX                          0
#define KJB_PTHREAD_ONCE_INIT                         0
#define KJB_PTHREAD_CREATE_JOINABLE                   0
#define KJB_PTHREAD_CREATE_DETACHED                   0

#endif /* KJB_HAVE_PTHREAD */



/* -------------------------------------------------------------------------
 * Stuff below this line should compile regardless of whether the system
 * has included pthread.h.
 * -------------------------------------------------------------------------
 */


/* BASIC PTHREAD CONTROL function ............................... */

int kjb_pthread_create(
    kjb_pthread_t* newthread,
    const kjb_pthread_attr_t* attr,
    void* (*start_routine)(void*),
    void* arg
);

int kjb_pthread_join(kjb_pthread_t tid, void** rval_p);

void kjb_pthread_exit(void* rval_p);



/* ADVANCED PTHREAD functions ................................... */

int get_kjb_pthread_self(kjb_pthread_t* tid);

int kjb_pthread_equal(kjb_pthread_t tid1, kjb_pthread_t tid2);

int kjb_pthread_cancel(kjb_pthread_t tid);

int kjb_pthread_attr_init(kjb_pthread_attr_t* attr_p);

int kjb_pthread_attr_destroy(kjb_pthread_attr_t* attr_p);

int kjb_pthread_attr_setdetachstate(kjb_pthread_attr_t* attr_p, int state);

int kjb_pthread_attr_getdetachstate(kjb_pthread_attr_t* attr_p, int* state);



/* MUTEX functions .............................................. */

int kjb_pthread_mutex_lock(kjb_pthread_mutex_t* mutex);

int kjb_pthread_mutex_trylock(kjb_pthread_mutex_t* mutex);

int kjb_pthread_mutex_unlock(kjb_pthread_mutex_t* mutex);

int kjb_pthread_mutex_init(
    kjb_pthread_mutex_t* mutex,
    kjb_pthread_mutexattr_t* attr
);

int kjb_pthread_mutex_destroy(kjb_pthread_mutex_t* mutex);



/* THREAD SPECIFIC DATA functions ................................ */

int kjb_pthread_key_create(
    kjb_pthread_key_t *key,
    void (*destr_function) (void *)
);

int kjb_pthread_key_delete(kjb_pthread_key_t key);

int kjb_pthread_setspecific(kjb_pthread_key_t key, const void *pointer);

void* kjb_pthread_getspecific(kjb_pthread_key_t key);

int kjb_pthread_once(
    kjb_pthread_once_t* once_control,
    void (*init_routine)(void)
);



/* Meta-wrapper functions:  they concern the wrapper itself.  Use sparingly.
   ................................................................ */

int kjb_multithread_wrapper_serialization_lock(void);

int kjb_multithread_wrapper_serialization_unlock(void);

int kjb_pthread_read_prng_seeds(kjb_uint16* s, size_t buf_length);

int get_kjb_pthread_number(void);

void reset_kjb_pthread_counter(void);

/* below:  end-of-file boilerplate */

#ifdef __cplusplus
#ifdef COMPILING_CPLUSPLUS_SOURCE
}
#endif
}
#endif

#endif
