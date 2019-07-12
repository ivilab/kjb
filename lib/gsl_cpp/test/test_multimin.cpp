/**
 * @file
 * @brief Unit test for GSL minimization plus C++ wrappers on GSL resources.
 * @author Andrew Predoehl
 */
/*
 * $Id: test_multimin.cpp 9821 2011-06-27 00:52:55Z predoehl $
 */

#include <l/l_incl.h>
#include <gsl_cpp/gsl_cpp_incl.h>

#include <iostream>
#include <string>
#include <cmath>

#include "test_expects.h"


namespace {

const int FDIMS = 2; /* the example function has dimensionality of two */

const bool VERBOSE = false;

double paraboloid( const gsl_vector* x, void* params )
{
    const double* p = (const double*) params;

    double u = gsl_vector_get( x, 0 );
    double v = gsl_vector_get( x, 1 );
    return (u - p[0])*(u - p[1])*p[2] + (v - p[3])*(v - p[4])*p[5];
}


void del_paraboloid( const gsl_vector* x, void* params, gsl_vector* df )
{
    const double* p = (const double*) params;

    double u = gsl_vector_get( x, 0 );
    double v = gsl_vector_get( x, 1 );
    gsl_vector_set( df, 0, (2*u - p[0] - p[1])*p[2] );
    gsl_vector_set( df, 1, (2*v - p[3] - p[4])*p[5] );
}


void paraboloid_and_gradient(
    const gsl_vector* x,
    void* params,
    double *f,
    gsl_vector* df
)
{
    *f = paraboloid( x, params );
    del_paraboloid( x, params, df );
}



void status_report( const kjb::Gsl_Multimin_fdf &tool, int iter )
{
    std::cout << iter << "  ";

    kjb::Gsl_Vector argmin( * tool.argmin() );
    for( int iii = 0; iii < argmin.size(); ++iii )
    {
        std::cout << ' ' << std::fixed << argmin.at( iii );
    }

    std::cout << '\t';
    kjb::Gsl_Vector gradient( * tool.gradient() );
    for( int iii = 0; iii < gradient.size(); ++iii )
    {
        std::cout << ' ' << gradient.at( iii );
    }
    std::cout << '\t' << gradient.l2_norm()
                << '\t' << tool.min() << '\n';
}


int fail( const std::string& msg )
{
    std::cerr << "Error:  " << msg << '\n';
    return EXIT_FAILURE;
}







/// @brief demo of GSL minimization:  find minimum of a paraboloid (easy!)
int test_minimization()
{
    const double parameters[] = { 2, 4, 4, 3, 5, 4 };
    const double x_root = 0.5 * (parameters[ 0 ]+parameters[ 1 ]),
                 y_root = 0.5 * (parameters[ 3 ]+parameters[ 4 ]);
    const double EPS = 1e-6; // used for checking roots

    /*
     * struct gsl_multimin_function_fdf  isn't wrapped b/c there's no need to.
     *
     * Well, no resource-releasing need at least; perhaps an aesthetic need
     * that I'm ignoring.
     */

    gsl_multimin_function_fdf parabo;
    parabo.n = FDIMS; /* number of dimensions in domain */
    parabo.f = &paraboloid;
    parabo.df = &del_paraboloid;
    parabo.fdf = &paraboloid_and_gradient;
    parabo.params = (void*) parameters;


    // Verify that failing to hand in an initial location causes it to throw
    bool caught_something = false;
    try
    {
        kjb::Gsl_Multimin_fdf toolz(
        gsl_multimin_fdfminimizer_conjugate_pr, &parabo, 00, .01, 1e-4 );
    }
    catch( kjb::KJB_error& e )
    {
        if ( VERBOSE )
        {
            e.print_details();
        }
        caught_something = true;
    }
    we_expect_that( caught_something );


    // Verify that if init value is incompatible with fun., it throws also
    caught_something = false;
    try
    {
        kjb::Gsl_Vector xbad( 1+FDIMS );
        gsl_set_error_handler_off();
        kjb::Gsl_Multimin_fdf tooly(
        gsl_multimin_fdfminimizer_conjugate_pr, &parabo, xbad, .01, 1e-4 );
    }
    catch( kjb::KJB_error& e )
    {
        if ( VERBOSE )
        {
            e.print_details();
        }
        caught_something = true;
    }
    we_expect_that( caught_something );



    // Now we actually run the minimizer with legit values

    /* start the minimization from pi,pi */
    kjb::Gsl_Vector x0( FDIMS );
    x0.at( 0 ) = M_PI;
    x0.at( 1 ) = M_PI;

    /*
     * YOu have a choice of minimization methods:
     * METHOD                                       NUMBER OF STEPS
     * ------------------------------------------------------------
     * gsl_multimin_fdfminimizer_conjugate_fr       6
     * gsl_multimin_fdfminimizer_conjugate_pr       6
     * gsl_multimin_fdfminimizer_vector_bfgs2       1
     * gsl_multimin_fdfminimizer_steepest_descent   106
     *
     * In the test code below, I optionally can cause a "setback" that muddles
     * up the above numbers.
     */
    kjb::Gsl_Multimin_fdf tool(
        //gsl_multimin_fdfminimizer_conjugate_fr,
        gsl_multimin_fdfminimizer_conjugate_pr,
        //gsl_multimin_fdfminimizer_vector_bfgs2,
        //gsl_multimin_fdfminimizer_steepest_descent,
        &parabo, x0, .01, 1e-4 );

    if ( VERBOSE )
    {
        std::cout << "Algorithm name:  " << tool.name()
            << "\nActual paraboloid minimum at (" << x_root << ", " << y_root
            << ")\nIt. Argument x\t\tGradient at x\t\tGrd. magnitude\tf(x)\n";
    }

    const double EPS_GRAD = 1e-3;
    size_t iter = 0;
    for( iter = 0; iter < 1000; ++iter )
    {
        #if 0
            // suddenly there's a terrible setback!
            if ( 4 == iter )
            {
                // throw in a monkey wrench
                kjb::Gsl_Vector xx( FDIMS );
                xx.at( 0 ) = 5;
                xx.at( 1 ) = 5;
                kjb::Gsl_Multimin_fdf( gsl_multimin_fdfminimizer_conjugate_pr,
                        &parabo, xx, .01, 1e-4 ).swap( tool );
            }
        #endif

        /* take a step; return a nonzero error code iff something went wrong */
        if ( tool.iterate() )
        {
            break;
        }

        if ( GSL_SUCCESS == tool.test_gradient( EPS_GRAD ) )
        {
            if ( VERBOSE )
            {
                std::cout <<  "Gradient magnitude has shrunk below "
                        << EPS_GRAD << " so we stop now.  Final results:\n";
            }
            break;
        }
        if ( VERBOSE )
        {
            status_report( tool, iter );
        }
    }

    if ( VERBOSE )
    {
        status_report( tool, iter );
    }

    kjb::Gsl_Vector argmin( * tool.argmin() );
    if ( EPS < abs(argmin.at( 0 ) - x_root) )
    {
        return fail( "Bad root (x direction)" );
    }
    if ( EPS < abs(argmin.at( 1 ) - y_root) )
    {
        return fail( "Bad root (y direction)" );
    }

    return EXIT_SUCCESS;
}


} // end anon ns


int main()
{
    int rc2 = test_minimization();
    if ( rc2 != EXIT_SUCCESS )
    {
        return fail( "unit test for GSL minimization failed." );
    }

    std::cout << "Success!\n";
    return EXIT_SUCCESS;
}
