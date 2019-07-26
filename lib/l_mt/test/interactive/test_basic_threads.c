/*
 * $Id: test_basic_threads.c 21491 2017-07-20 13:19:02Z kobus $
 */

#include <l/l_sys_lib.h>
#include <l/l_sys_io.h>
#include <l/l_error.h>
#include <l/l_global.h>
#include <l/l_init.h>
#include <l_mt/l_mt_pthread.h>

#define HIVE_SIZE 50

kjb_pthread_mutex_t mtx = KJB_PTHREAD_MUTEX_INITIALIZER;

static void* thready_work(void* p)
{
    union { void* v; int i; } u;
    u.v = p;

    EPETE(kjb_pthread_mutex_lock(&mtx));
    kjb_printf("worker thread %d\n", u.i);
    EPETE(kjb_pthread_mutex_unlock(&mtx));

    return NULL;
}

int main(int argc, char** argv)
{
    int i;
    kjb_pthread_t worker_bees[HIVE_SIZE];

    if (! is_interactive()) 
    {
        p_stderr("This test program either needs to be adjusted for batch testing or moved.\n");
        p_stderr("Forcing failure to help increase the chances this gets done.\n"); 
        return EXIT_CANNOT_TEST;
    }
    

    EPETE(kjb_init());
    kjb_puts("home thread hello\n");

    /* start them */
    for (i = 0; i < HIVE_SIZE; ++i)
    {
        EPETE(kjb_pthread_create(worker_bees+i, NULL, thready_work, (void*)i));
    }

    EPETE(kjb_pthread_mutex_lock(&mtx));
    kjb_puts("all threads launched\n");
    EPETE(kjb_pthread_mutex_unlock(&mtx));

    /* stop them */
    for (i = 0; i < HIVE_SIZE; ++i)
    {
        EPETE(kjb_pthread_join(worker_bees[i], NULL));
    }
    kjb_cleanup();

    return EXIT_SUCCESS;
}

