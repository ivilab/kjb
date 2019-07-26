/**
 * @file
 * @author Andrew Predoehl
 * @brief test program for wrapper on GSL random num. generators
 */
/*
 * $Id: test_rng.cpp 9821 2011-06-27 00:52:55Z predoehl $
 */

#include <l/l_incl.h>
#include <gsl_cpp/gsl_rng.h>
#include "test_expects.h"

template< typename RNG >
void test()
{
    const int TSZ = 10000;

    RNG rng1, rng2;

    std::vector< unsigned long > sama( TSZ ), samb( TSZ );

    for( size_t iii = 0; iii < TSZ; ++iii )
        sama[ iii ] = rng1.get();

    std::string state = rng1.serialize();

    for( size_t iii = 0; iii < TSZ; ++iii )
        samb[ iii ] = rng1.get();

    rng1.deserialize( state );

    for( size_t iii = 0; iii < TSZ; ++iii ) {
        // verify that the two RNGs are initialized to the same state
        we_expect_that( sama[ iii ] == rng2.get() );
        // verify that deserialize() really resets state back to how it was
        we_expect_that( samb[ iii ] == rng1.get() );

        // this assertion could legitimately fail, but the probability is tiny
        we_expect_that( sama[ iii ] != samb[ iii ] );
    }

    // Verify that serialize itself leaves the state unchanged
    for( size_t iii = 0; iii < TSZ; ++iii )
        we_expect_that( samb[ iii ] == rng2.get() );

    //==========================================================

    /*
     * Test that the uniform distribution looks more or less uniform.
     */

    const int BINS = 10;
    std::vector< int > bins( BINS, 0 );
    const int mean = TSZ / BINS, sigma = sqrt( mean * ( BINS - 1 ) / BINS );

    for( size_t iii = 0; iii < TSZ; ++iii )
        bins.at( int( rng1.uniform() * BINS ) ) += 1;

    // 99.7% confidence integral (3 sigma) assuming binomial dist is normal
    // We almost can tighten this down to 95% but we can't quite make it.
    // Not surprising, since we are doing over 100 such conf. interval tests!
    for( int iii = 0; iii < BINS; ++iii ) {
        /*
        if ( kjb_c::is_interactive() )
            kjb_c::kjb_printf( "%d\n", bins.at( iii ) );
        */
        we_expect_that( abs( bins.at( iii ) - mean ) / sigma < 3 );
    }
}

int main()
{
    test< kjb::Gsl_rng_mt19937 >();
    test< kjb::Gsl_rng_ranlxs0 >();
    test< kjb::Gsl_rng_ranlxs1 >();
    test< kjb::Gsl_rng_ranlxs2 >();
    test< kjb::Gsl_rng_ranlxd1 >();
    test< kjb::Gsl_rng_ranlxd2 >();
    test< kjb::Gsl_rng_cmrg >();
    test< kjb::Gsl_rng_mrg >();
    test< kjb::Gsl_rng_taus2 >();
    test< kjb::Gsl_rng_gfsr4 >();

    if ( kjb_c::is_interactive() ) {
        kjb_c::kjb_puts( __FILE__ );
        kjb_c::kjb_puts( ": success!\n" );
    }

    return EXIT_SUCCESS;
}
