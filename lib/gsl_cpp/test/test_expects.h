/**
 * @file
 * @brief Macros and inline functions for unit tests
 * @author Andrew Predoehl
 */

/*
 * $Id: test_expects.h 8428 2011-01-24 02:19:55Z predoehl $
 *
 * Recommended tab width:  4
 */


#ifndef TEST_EXPECT_H_PREDOEHL
#define TEST_EXPECT_H_PREDOEHL

#include "l/l_incl.h"  // not really needed; it's here just to be kjb-orthodox

#include <iostream>
#include <cassert>
#include <cmath>



/// @brief macro jumps to implementation of test assertions, with line number
#define we_expect_that(P)   we_expect_that_imp( (P), #P, __LINE__ )

/// @brief macro jumps to equality test, incluing line number
#define we_expect_approx_equality(A,B) \
                    we_expect_approx_equality_imp( (A), (B), #A, #B, __LINE__ )


inline void we_expect_that_imp( bool bb, const char* ss = 0, int from = -1 )
{
    if ( ! bb )
    {
        if ( from > 0 )
        {
            std::cerr << "Failure on line " << from << '\n';
        }
        if ( ss )
        {
            std::cerr << "Condition:  \"" << ss << "\"\n";
        }
    }
    assert( bb );
}

void we_expect_approx_equality_imp(
    double aa,
    double bb,
    const char* sa = 0,
    const char* sb = 0,
    int from = -1
)
{
    double dif = fabs( aa - bb ), sum = fabs( aa ) + fabs( bb ), eps = 5e-3;
    bool success = ( dif <= sum * eps ); // that is, dif == 0 or dif/sum <= eps
    if ( ! success )
    {
        std::cerr << "fail, dif = " << dif << ", sum = " << sum
                    << ", dif/sum = " << dif/sum << ", eps = " << eps << '\n';
        if ( from > 0 )
        {
            std::cerr << "on line " << from << '\n';
        }
        if ( sa )
        {
            std::cerr << "left hand side: \"" << sa << "\"\n";
        }
        if ( sb )
        {
            std::cerr << "right hand side: \"" << sb << "\"\n";
        }
        assert( success );
    }
}

#endif
