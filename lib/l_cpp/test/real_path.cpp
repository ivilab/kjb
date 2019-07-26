/**
 * @file
 * @author Andrew Predoehl
 * @brief unit test for function kjb::realpath()
 */
/*
 * $Id: real_path.cpp 14701 2013-06-07 20:35:16Z predoehl $
 */

#include <l/l_sys_std.h>
#include <l/l_sys_lib.h>
#include <l/l_sys_io.h>
#include <l/l_init.h>
#include <l/l_error.h>
#include <l/l_global.h>
#include <l_cpp/l_filesystem.h>
#include <l_cpp/l_exception.h>
#include <l_cpp/l_util.h>

#define SHOULD_BE_TRUE(b) check((b), __FILE__, __LINE__)

namespace {

void check(bool false_means_failure, const char* file, unsigned line)
{
    if (false_means_failure == false)
    {
        kjb_c::kjb_fprintf(stderr, "Failure %s:%u\n", file, line);
        exit(EXIT_FAILURE);
    }
}

int test()
{
    const char  *p1 = "/usr/bin",
                *p2 = "/usr/bin/",
                *p3 = "/usr/lib",
                *p4 = "/usr/lib/../lib",
                *p5 = "/usr/lib/../bin/";

    // hopefully no embarrassing typos
    SHOULD_BE_TRUE(kjb_c::is_directory(p1));
    SHOULD_BE_TRUE(kjb_c::is_directory(p2));
    SHOULD_BE_TRUE(kjb_c::is_directory(p3));
    SHOULD_BE_TRUE(kjb_c::is_directory(p4));
    SHOULD_BE_TRUE(kjb_c::is_directory(p5));

    // is it reflexive?
    SHOULD_BE_TRUE(kjb::realpath(p1) == kjb::realpath(p1));
    SHOULD_BE_TRUE(kjb::realpath(p2) == kjb::realpath(p2));
    SHOULD_BE_TRUE(kjb::realpath(p3) == kjb::realpath(p3));
    SHOULD_BE_TRUE(kjb::realpath(p4) == kjb::realpath(p4));
    SHOULD_BE_TRUE(kjb::realpath(p5) == kjb::realpath(p5));

    // is it symmetric?
    SHOULD_BE_TRUE(kjb::realpath(p1) == kjb::realpath(p2));
    SHOULD_BE_TRUE(kjb::realpath(p2) == kjb::realpath(p1)); // that's enough

    // is it gullible?
    SHOULD_BE_TRUE(kjb::realpath(p1) != kjb::realpath(p3));
    SHOULD_BE_TRUE(kjb::realpath(p1) != kjb::realpath(p4));
    SHOULD_BE_TRUE(kjb::realpath(p3) != kjb::realpath(p2));
    SHOULD_BE_TRUE(kjb::realpath(p4) != kjb::realpath(p2));

    // is it actually pretty smart?
    SHOULD_BE_TRUE(kjb::realpath(p3) == kjb::realpath(p4));
    SHOULD_BE_TRUE(kjb::realpath(p2) == kjb::realpath(p5));
    SHOULD_BE_TRUE(kjb::realpath(p1) == kjb::realpath(p5));
    SHOULD_BE_TRUE(kjb::realpath(p4) == kjb::realpath(p3));
    SHOULD_BE_TRUE(kjb::realpath(p5) == kjb::realpath(p2));

    return kjb_c::NO_ERROR;
}

}


int main()
{
    kjb_c::kjb_init();

    try
    {
        KJB(EPE(test()));
    }
    catch (const kjb::Exception& e)
    {
        e.print_details_exit();
    }

    if (kjb_c::is_interactive()) kjb_c::kjb_printf("Success!\n");

    kjb_c::kjb_cleanup();

    return EXIT_SUCCESS;
}

