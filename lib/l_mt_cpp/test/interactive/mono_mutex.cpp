/*
 * This demonstrates an "automatic" mutex, i.e., it lives on the stack, not in
 * static memory, and it is locked and unlocked by ten threads.
 *
 * $Id: mono_mutex.cpp 15719 2013-10-18 01:34:29Z predoehl $
 */
#include <l/l_incl.h>
#include <l_cpp/l_util.h>
#include <l_mt_cpp/l_mt_mutexlock.h>

namespace
{

// non-reentrant function; we serialize calls to it using the mutex.
void put()
{
    static int i;
    kjb_c::kjb_printf("print line %4d   (called by thread %d)\n",
                      i++, kjb_c::get_kjb_pthread_number());

    // nap does not work -- I guess signals and threads do not mix.
    //kjb_c::nap(10 * kjb_c::kjb_rand());
}

// this enforces serial access to the above thread-unsafe function put().
void serial_put(kjb::Pthread_mutex* q)
{
    kjb::Mutex_lock m(q);   /* lock the mutex                           */
    put();                  /* call the "critical" function             */
                            /* mutex is automatically unlocked by dtor  */
}

// thread worker function, which is given a pointer to mutex as a parameter
void* work(void* v)
{
    kjb::Pthread_mutex* q = static_cast<kjb::Pthread_mutex*>(v);
    KJB(NRN(q));

    serial_put(q);
    serial_put(q);
    serial_put(q);

    return q;
}

// test function launches ten threads and joins them up again.
int test()
{
    const int TCT = 10;
    kjb_c::kjb_pthread_t tid[TCT];
    kjb::Pthread_mutex q;

    for (int i = 0; i < TCT; ++i)
    {
        KJB(ERE(kjb_pthread_create(tid + i, NULL, & work, & q)));
    }
    for (int i = 0; i < TCT; ++i)
    {
        void *v;
        KJB(ERE(kjb_pthread_join(tid[i], &v)));
        KJB(NRE(v));
    }

    return kjb_c::NO_ERROR;
}

}

// main function starts the library nanny, catches errors, cleans up.
int main(int argc, char** argv)
{
    KJB(EPETE(kjb_init()));
    try
    {
        KJB(EPETE(test()));
    }
    catch (const kjb::Exception& e)
    {
        e.print_details_exit();
    }
    kjb_c::kjb_cleanup();
    return EXIT_SUCCESS;
}

