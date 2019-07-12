/**
 * @file
 * @author Andrew Predoehl
 * @brief test RAII class for wrapped pthread attribute
 */
/*
 * $Id: detach.cpp 17430 2014-09-01 18:43:22Z predoehl $
 */

#include <l/l_sys_rand.h>
#include <l/l_sys_term.h>
#include <l/l_sys_tsig.h>
#include <l/l_error.h>
#include <l/l_global.h>
#include <l_mt/l_mt_util.h>
#include <l_cpp/l_util.h>
#include <l_mt_cpp/l_mt_pt_attr.h>

namespace
{

void* chatter(void* v)
{
    KJB(NRN(v));
    const char* m = (const char*) v;
    for(int i=100; i--; )
    {
        const int s = 1+30*kjb_c::kjb_rand();
        kjb_c::kjb_mt_fprintf(stdout, "%d %s %d\n", i, m, s);
    }
    return 0; // return value is meaningless because thread is detached
}

}

int main(int argc, char** argv)
{
    kjb_c::kjb_pthread_t tid;
    kjb::Pthread_attr a;
    a.set_detached();
    kjb_c::kjb_disable_paging();
    kjb_c::kjb_pthread_create(&tid, a, chatter, (void*) "lol");
    kjb_c::kjb_pthread_create(&tid, a, chatter, (void*) "yolo");
    kjb_c::kjb_mt_fprintf(stdout, "Parent thread is exiting soon, "
                            "you kids keep the noise down.\n");
    kjb_c::nap(500); // shutdown in .5 seconds

    // No need to join because chatter threads are detached.
    // They will be snuffed out now, whether or not they are finished.
    // Because of the lengthy nap(), very likely they are finished.
    return 0;
}
