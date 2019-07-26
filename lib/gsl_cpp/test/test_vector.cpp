/**
 * @file
 * @brief Unit test for wrapper on GSL vector 
 * @author Andrew Predoehl
 */
/*
 * $Id: test_vector.cpp 20314 2016-02-02 06:20:49Z predoehl $
 */

#include <l_cpp/l_stdio_wrap.h>
#include <gsl_cpp/gsl_cpp_incl.h>

#include <iostream>
#include <fstream>
#include <string>

#include "test_expects.h"


namespace {


int fail( const std::string& msg )
{
    std::cerr << "Error:  " << msg << '\n';
    return EXIT_FAILURE;
}


int test_io()
{
    kjb::Temporary_File tf;

    kjb::Gsl_Vector pi( 10 );
    pi.at( 0 ) = 3.0;
    pi.at( 1 ) = 0.1;
    pi.at( 2 ) = 0.04;
    pi.at( 3 ) = 0.001;
    pi.at( 4 ) = 0.0005;
    pi.at( 5 ) = 0.00009;
    pi.at( 6 ) = 0.000002;
    pi.at( 7 ) = 0.0000006;
    pi.at( 8 ) = 0.00000005;
    pi.at( 9 ) = 0.000000004;

    do {
        std::ofstream fo( tf.get_filename().c_str() );
        if ( ! fo )
        {
            return fail( "unable to open temporary file for writing (weird)" );
        }
        fo << pi;
        if ( ! fo )
        {
            return fail( "could not write vector to temporary file" );
        }
    } while( 0 );
    fflush(tf);

    kjb::Gsl_Vector icecream( 10 );
    do {
        std::ifstream fi( tf.get_filename().c_str() );
        if ( ! fi )
        {
            return fail( "unable to open temporary file for reading (weird)" );
        }
        fi >> icecream;
        if ( ! fi )
        {
            return fail( "unable to read vector from temporary file" );
        }
    } while( 0 );

    kjb::Gsl_Vector diff = pi - icecream;

    if (kjb_c::is_interactive())
    {
        std::cout   << "start: " << pi
                    << "end:  " << icecream
                    << "diff: " << diff
                    << "norm diff: " << diff.l2_norm() << '\n';
    }

    we_expect_that( diff.l2_norm() < 1e-12 );

    return EXIT_SUCCESS;
}


const double pie_d[] = { 3.0, 0.1, 0.04, 0.001, 0.0006 };
const size_t pie_size = 5;


bool is_5_digits_of_pi( kjb::Gsl_Vector& testvec )
{
    const double EPS( 1e-12 );

    if ( testvec.size() != 5 ) return false;

    for( size_t iii = 0; iii < pie_size; ++iii )
    {
        if ( abs( pie_d[ iii ] - testvec.at( iii ) ) > EPS ) return false;
    }
    return true;
}


int test_iterator_ctor()
{
    kjb::Gsl_Vector pi1( pie_d, pie_d + pie_size );
    we_expect_that( is_5_digits_of_pi( pi1 ) );

    kjb::Vector pi2( pie_size, pie_d );
    kjb::Gsl_Vector pi3( pi2 );
    we_expect_that( is_5_digits_of_pi( pi3 ) );

    kjb::Gsl_Vector pi4( pi2.begin(), pi2.end() );
    we_expect_that( is_5_digits_of_pi( pi4 ) );

    std::vector< double > pi5( pie_d, pie_d + pie_size );
    kjb::Gsl_Vector pi6( pi5.begin(), pi5.end() );
    we_expect_that( is_5_digits_of_pi( pi6 ) );

    return EXIT_SUCCESS;
}


// this is a unit test on the GSL vectors themselves -- that they behave sanely
int test_vector()
{
    using kjb::Gsl_Vector;

    Gsl_Vector v1( 5 ), v2( 5 ), w1( 6 );
    w1.at( 0 ) = 17;
    // Store values into the vectors, see if we can read them out again
    for( int iii = 0; iii < 5; ++iii )
    {
        v1.at( iii ) = 3*iii;
        v2.at( iii ) = 4 * iii + 1;
    }
    for( int iii = 0; iii < 5; ++iii )
    {
        we_expect_that( v1.at( iii ) == 3*iii );
        we_expect_that( v2.at( iii ) == 4 * iii + 1 );
    }

    // Test that vectors can differ from each other
    we_expect_that( v1.size() != w1.size() );
    w1 = v1;
    we_expect_that( v1.size() == w1.size() );

    // Test that changing one vector does not affect a distinct vector
    for( int iii = 0; iii < 5; ++iii )
    {
        we_expect_that( w1.at( iii ) == 3*iii );
        we_expect_that( v2.at( iii ) == 4 * iii + 1 ); // is v2 unaffected?
    }

    // Test non-overwriting addition:  also verify summands are unaffected
    Gsl_Vector v1plusv2( v1 + v2 );
    for( int iii = 0; iii < 5; ++iii )
    {
        we_expect_that( v1.at( iii ) == 3*iii );
        we_expect_that( v2.at( iii ) == 4 * iii + 1 );
        we_expect_that( v1plusv2.at( iii ) == 7 * iii + 1 );
    }

    // Test overwriting addition
    v1 += v2;
    for( int iii = 0; iii < 5; ++iii )
    {
        we_expect_that( v1.at( iii ) == v1plusv2.at( iii ) );
        we_expect_that( v2.at( iii ) == 4 * iii + 1 );
    }

    // Test negation, test non-overwriting subtraction
    v2 -= v1;
    Gsl_Vector v3( v2 - v1 );
    for( int iii = 0; iii < 5; ++iii )
    {
        we_expect_that( v2.at( iii ) == -3 * iii );
        we_expect_that( v3.at( iii ) == -10 * iii - 1 );
    }

    // Test reversal
    v1.ow_reverse();
    for( int iii = 0; iii < 5; ++iii )
    {
        we_expect_that( v1.at( 4 - iii ) == v1plusv2.at( iii ) );
        we_expect_that( v2.at( iii ) == -3 * iii ); // unaffected
    }

    // Test scalar addition (1) overwriting
    v3 += 5.0;
    for( int iii = 0; iii < 5; ++iii )
    {
        we_expect_that( v3.at( iii ) == -10 * iii + 4 );
        we_expect_that( v2.at( iii ) == -3 * iii ); // unaffected
    }

    // Test scalar addition (2) non-overwriting
    Gsl_Vector v4 = v3 + 2.0;
    for( int iii = 0; iii < 5; ++iii )
    {
        we_expect_that( v3.at( iii ) == -10 * iii + 4 ); // unaffected
        we_expect_that( v4.at( iii ) == -10 * iii + 6 );
    }

    // Test scalar subtraction (1) overwriting
    v3 -= 3.0;
    for( int iii = 0; iii < 5; ++iii )
    {
        we_expect_that( v3.at( iii ) == -10 * iii + 1 );
        we_expect_that( v2.at( iii ) == -3 * iii ); // unaffected
    }

    // Test scalar subtraction (2) non-overwriting
    Gsl_Vector v5 = v3 - 2.0;
    for( int iii = 0; iii < 5; ++iii )
    {
        we_expect_that( v3.at( iii ) == -10 * iii + 1 ); // unaffected
        we_expect_that( v5.at( iii ) == -10 * iii - 1 );
    }

    // Test scaling (1) overwriting
    v2 *= -0.25;
    for( int iii = 0; iii < 5; ++iii )
    {
        we_expect_approx_equality( v2.at( iii ), 0.75 * iii );
    }

    // Test scaling (2) non-overwriting
    we_expect_approx_equality( (12 * v2).at( 3 ), 27.0 );
    for( int iii = 0; iii < 5; ++iii )
    {
        we_expect_approx_equality( v2.at( iii ), 0.75 * iii ); // unaffected
    }

    // Test negation
    Gsl_Vector v6 = -v4;
    for( int iii = 0; iii < 5; ++iii )
    {
        we_expect_that( v4.at( iii ) == -10 * iii + 6 ); // unaffected
        we_expect_that( v6.at( iii ) == 10 * iii - 6 );
    }

    // Test max and min
    we_expect_approx_equality( v4.min(), 6-40 );
    we_expect_approx_equality( v6.max(), 40-6 );

    // Test conversion to KJB Vector (the C++ kind)
    kjb::Vector k5( v5.vec() );
    Gsl_Vector vee_five( k5 );
    we_expect_approx_equality( (v5 - vee_five).l2_norm(), 0 );

    // Test for abnormal value detection
    we_expect_that( v1.is_normal() );
    double zero = 0;
    v1.at( 3 ) = 1/zero;
    we_expect_that( ! v1.is_normal() );
    v1.at( 3 ) *= 1;
    we_expect_that( ! v1.is_normal() );
#if 0 /* does not work on elgato (icpc/2013.1.039) */
    v1.at( 3 ) *= 0; /* zero times NaN is NaN!  how do you like them apples? */
    we_expect_that( ! v1.is_normal() );
#endif
    v1.at( 3 ) = 1;
    we_expect_that( v1.is_normal() );
    v1.at( 3 ) *= DBL_MAX;  /* big, but normal */
    we_expect_that( v1.is_normal() );
    v1.at( 3 ) *= DBL_MAX;  /* abnormally big */
    we_expect_that( ! v1.is_normal() );
    v1.at( 3 ) = HUGE_VAL;  /* also abnormally big */
    we_expect_that( ! v1.is_normal() );

    // Test the ctor that uses two iterators
    int rc2 = test_iterator_ctor();
    we_expect_that( EXIT_SUCCESS == rc2 );

    // Test vector I/O
    int rc3 = test_io();
    we_expect_that( EXIT_SUCCESS == rc3 );

    return EXIT_SUCCESS;
}







} // end anon ns


int main()
{
    int rc1 = test_vector();
    if ( rc1 != EXIT_SUCCESS )
    {
        return fail( "unit test for GSL vector wrap failed." );
    }

    std::cout << "Success!\n";
    return EXIT_SUCCESS;
}
