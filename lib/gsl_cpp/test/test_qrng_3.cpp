/**
 * @file
 * @author Andrew Predoehl
 * @brief demonstrate static typing of Gsl_Qrng_basic prevents messes.
 */

/* $Id: test_qrng_3.cpp 9821 2011-06-27 00:52:55Z predoehl $
 */

#include <l/l_incl.h>
#include <gsl_cpp/gsl_qrng.h>


/*
 * The purpose of this test is mostly to make sure that my template hackery
 * did the job:  it prevents you from assigning qrngs of different types.
 */
int main()
{
    kjb::Gsl_Qrng_Sobol qs( 10 );
    kjb::Gsl_Qrng_Niederreiter qn( 10 ), qm( qn );
    kjb::Gsl_Qrng_Sobol qs2( 15 );

    /*
    kjb::Gsl_Qrng_Niederreiter qp( qs ); //   <------ that should not compile
    */

    qn = qm;

    qs = qs2;

    /*
    qn = qs;    //       <-----------  that should not compile
    */

    /*
    kjb::Gsl_Qrng_basic *r1 = &qs, *r2 = &qn;   //   <--- also no way
    */

    return EXIT_SUCCESS;
}
