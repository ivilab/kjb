/* ======================================================================== *
 |                                                                          |
 | Copyright (c) 2010, by members of University of Arizona Computer Vision  |
 | group (the authors) including                                            |
 |       Kyle Simek, Andrew Predoehl                                        |
 |                                                                          |
 | For use outside the University of Arizona Computer Vision group please   |
 | contact Kobus Barnard.                                                   |
 |                                                                          |
 * ======================================================================== */

/* $Id: test.h 5908 2010-05-21 23:31:54Z ksimek $ */

#ifndef TEST_H
#define TEST_H

#include "l/l_sys_lib.h"
#include "l_cpp/l_exception.h"

//#define DB(x) std::cout << #x << ": \n" << x << std::endl;

void down_in_flames(
    const char* test_kind,
    const char* filename,
    int line_num,
    const char* bad_line
);

void print_victory(const char*);

/**
 * @brief utility function intended for reading time factor from argv[1].
 * @return NO_ERROR if argv1 contains a nonnegative integer, or argv1 equal
 * to NULL.  ERROR if tf equals NULL or argv1 is a string with other contents.
 *
 * Typical use:
 * @code
 * int time_factor = 1;
 * KJB(ERE(scan_time_factor(argv[1], &time_factor)));
 * @endcode
 */
int scan_time_factor(const char* argv1, int* tf);

/* The "do STUFF while(0)" pattern provides a few benefits:
 * 1. provides a closed scope for any local variable(s);
 * 2. wraps up an "if"  so that a stray "else" cannot bind to it; and
 * 3. makes the action a single grammatical statement, thus it forces one to
 *    use (correctly) a semicolon after the macro invocation --
 *    the idea being that we don't want to mask errors, even small ones.
 */

#define DOWN_IN_FLAMES(tst,src)                             \
    down_in_flames( (tst), __FILE__, __LINE__, (src) )

#define RETURN_VICTORIOUSLY()                               \
    do {    print_victory(__FILE__);                        \
            return EXIT_SUCCESS;                            \
    } while( 0 )

#define TEST_TRUE(line)                                     \
    do if (!(line)) { DOWN_IN_FLAMES( "TEST_TRUE",  #line ); } while( 0 )

#define TEST_FALSE(line)                                    \
    do if ( line )  { DOWN_IN_FLAMES( "TEST_FALSE", #line ); } while( 0 )

#define TEST_SUCCESS(line)                                  \
    do {                                                    \
        try { line; }                                       \
        catch(...)                                          \
        {                                                   \
            DOWN_IN_FLAMES( "TEST_SUCCESS", #line );        \
        }                                                   \
    } while ( 0 )


#define TEST_APPROX_EQUALITY(x,y)                            \
    do {                                                     \
        const double a(x), b(y);                             \
        if(!(a==b || fabs(a-b) <= (fabs(a)+fabs(b))*0.005 )) \
        {                                                    \
            DOWN_IN_FLAMES( "TEST_APPROX_EQUALITY", #x " approx == " #y ); \
        }                                                    \
    } while(0)


#define TEST_APPROX_ZERO(z) TEST_ZERO_WITH_TOLERANCE(z, 1e-10)


#define TEST_ZERO_WITH_TOLERANCE(z, t)                      \
    do {                                                    \
        if (!(fabs(z) <= (t)))                              \
        {                                                   \
            DOWN_IN_FLAMES( "TEST_ZERO_WITH_TOLERANCE", "fabs(" #z ")<=" #t );\
        }                                                   \
    } while(0)


#define TEST_FAIL(line)                                     \
    do {                                                    \
        bool assert_fail_failed = false;                    \
        try { line; }                                       \
        catch( kjb::Index_out_of_bounds& iob )              \
        {                                                   \
            /*  std::cout <<"Correctly caught: "; */        \
            /*  iob.print( std::cout );           */        \
            assert_fail_failed = true;                      \
        }                                                   \
        catch(...)                                          \
        {                                                   \
            assert_fail_failed = true;                      \
        }                                                   \
        if (!assert_fail_failed)                            \
        {                                                   \
            DOWN_IN_FLAMES( "TEST_FAIL", #line );           \
        }                                                   \
    } while( 0 )

#endif
