/**
 * @file
 * @brief test dynamic mutexes
 * @author Andrew Predoehl
 *
 * This program seems to work in both development and production mode.
 * If anyone spots the program failing in some gross way, please let me know.
 * It is a demonstration not only of using dynamic mutexes, but also the
 * serialized, bare-bones API interface to a few very low level library funs.
 */
/*
 * $Id: multi_mutex.cpp 15515 2013-10-05 09:00:51Z predoehl $
 */

#include <l_mt/l_mt_util.h>
#include <l_mt_cpp/l_mt_mutexlock.h>
#include <l_cpp/l_test.h>
#include <vector>
#include <algorithm>

#include <boost/shared_ptr.hpp>
#include <boost/make_shared.hpp>

namespace {

std::vector< boost::shared_ptr< kjb::Pthread_mutex > > mm;

bool interactive;

void* work (void* v)
{
    int i;
    int* p = (int*) v;

    NTX(p);
    i = *p;
    kjb_c::kjb_mt_free(v);
    v = p = 0;

    mm[i] -> unlock();

    if (interactive) /* everybody read it simultaneously, probably safe? */
    {
        kjb_c::kjb_mt_fprintf(stdout, "Unlock mutex %d\n", i);
    }

    return NULL;
}


void test()
{
    const int SZ = 20;
    std::vector<kjb_c::kjb_pthread_t> threads(SZ);
    std::vector<int> mi(SZ);

    mm.clear();
    for (int i = 0; i < SZ; ++i)
    {
        mi[i] = i;
        mm.push_back( boost::make_shared< kjb::Pthread_locked_mutex>() );
    }

    std::random_shuffle(mi.begin(), mi.end());

    for (int i = 0; i < SZ; ++i)
    {
        int *p;
        NTX(p = (int*) kjb_c::kjb_mt_malloc(sizeof(int)));
        *p = mi[i];
        ETX(kjb_c::kjb_pthread_create(& threads[i], NULL, & work, (void*) p));
    }

    for (int i = 0; i < SZ; ++i)
    {
        ETX(kjb_c::kjb_pthread_join(threads[i], NULL));
    }
}

}

int main(int argc, char** argv)
{
    interactive = kjb_c::is_interactive();

    try
    {
        test();
    }
    catch (const kjb::Exception& e)
    {
        e.print_details_exit();
    }
    RETURN_VICTORIOUSLY();
}

