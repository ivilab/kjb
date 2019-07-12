/**
 * @file
 * @author Andrew Predoehl
 * @brief trivial wrapper on abort
 *
 * This file would not be necessary except that the build system is not very
 * flexible about where function implementations live relative to their
 * header declarations.
 */
/*
 * $Id: l_mt_mutexlock.cpp 15738 2013-10-19 06:28:50Z predoehl $
 */

#include <l/l_sys_lib.h>
#include <l_mt/l_mt_util.h>
#include <l_mt_cpp/l_mt_mutexlock.h>


namespace
{

void print_string_and_abort(const char* msg)
{
    kjb_c::kjb_mt_fprintf(stderr, "%s: aborting.\n", msg);
    kjb_c::kjb_abort();
}

}


namespace kjb
{

void Mutex_lock::fp_abort(const char* s)
{
    print_string_and_abort(s);
}

void Pthread_mutex::fp_abort(const char* s)
{
    print_string_and_abort(s);
}

}
