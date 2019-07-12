/*
 * This is a demonstration program showing how to use error-handling macros
 * ERN, NRN, and NPETE in a multi-thread program.
 *
 * $Id: failure.c 15729 2013-10-19 00:46:43Z predoehl $
 */

#include <l/l_incl.h>
#include <l_mt/l_mt_incl.h>

#define THREAD_CT 10

static void* t_fun(void* arg)
{
    char user[100];
    const char* msg = (const char*) arg;

    NRN(arg);
    ERN(BUFF_GET_USER_ID(user));

    if (kjb_rand() < 0.1)
    {
        /* that number is so small that we should panic! */
        set_error("FAILURE in thread %d!\n", get_kjb_pthread_number());
        return NULL;
    }

    kjb_mt_fprintf(stdout, "Hello, %s, from %s thread %d.\n",
                    user, msg, get_kjb_pthread_number());
    return arg;
}


int main(int argc, char** argv)
{
    int i;
    const char* arg[] = {"odd", "even"};
    kjb_pthread_t tid[THREAD_CT];

    EPETE(set_random_options("seed-2", "*"));

    for (i = 0; i < THREAD_CT; ++i)
    {
        EPETE(kjb_pthread_create(tid+i, NULL, t_fun, (void*) arg[i&1]));
    }
    for (i = 0; i < THREAD_CT; ++i)
    {
        void *v;
        EPETE(kjb_pthread_join(tid[i], &v));
        NPETE(v);
    }

    return EXIT_SUCCESS;
}
