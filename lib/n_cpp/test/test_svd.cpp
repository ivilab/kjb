/**
 * @file
 * @author Andrew Predoehl
 * @brief test the trivial SVD class
 */
/*
 * $Id: test_svd.cpp 11723 2012-02-16 17:40:31Z predoehl $
 */

#include <l/l_def.h>
#include <n_cpp/n_svd.h>

#include <iostream>
#include <cstdlib>

#define announce(V) annunciation_line( (V), __LINE__ )

namespace {

int annunciation_line ( int value, int line )
{
    if ( EXIT_SUCCESS == value )
    {
        std::cout << __FILE__ << " success!\n";
    }
    else
    {
        std::cout << __FILE__ << ":" << line << " failure\n";
    }
    return value;
}

}

int main()
{
    // test matrix chosen kind of randomly, except row 3 is not far from r1-r2.
    const int rank = 3;
    const double adat[] = { 5, 8, 1,
                            3, 3, 0,
                            2, 6, 1     };
    const kjb::Matrix amat( rank, rank, adat );

    // singular values as computed by Octave
    const double svs[] = { 12.0738, 1.7901, 0.1388 };
    const kjb::Vector singvs( rank, svs );

    // DUT:  The Device Undergoing Testing
    const kjb::Svd svd( amat );

    /*
    std::cout << "original matrix:\n";
    amat.write();

    std::cout << "\n left factor of svd:\n";
    svd.u().write();

    std::cout << "\n singular values of svd:\n";
    svd.d().write();

    std::cout << "\n right factor of svd:\n";
    svd.vt().write();

    std::cout << "\n reconstruction: \n";
    svd.reconstruction().write();

    std::cout << "\n reconstruction error:\n";
    (amat - svd.reconstruction()).write();

    */

    const double    sd = kjb::max_abs_difference( singvs, svd.d() ),

                    // reconstruction error
                    rd = kjb::max_abs_difference( amat, svd.reconstruction() ),

                    // is-u-columnwise-orthogonal error
                    ud = kjb::max_abs_difference(
                            kjb::create_identity_matrix( rank ),
                            svd.u() * svd.u().transpose()
                        ),

                    // is-v-orthogonal error
                    vd = kjb::max_abs_difference(
                            kjb::create_identity_matrix( rank ),
                            svd.vt() * svd.vt().transpose()
                        );

    /*
    std::cout << '\n' << sd
        << '\n' << rd
        << '\n' << ud
        << '\n' << vd
        << '\n';
    */

    if ( sd < 0 || 0.0001 < sd )
    {
        return announce( EXIT_FAILURE );
    }

    if ( rd < 0 || 1e-10 < rd )
    {
        return announce( EXIT_FAILURE );
    }

    if ( ud < 0 || 1e-10 < ud )
    {
        return announce( EXIT_FAILURE );
    }

    if ( vd < 0 || 1e-10 < vd )
    {
        return announce( EXIT_FAILURE );
    }

    return announce( EXIT_SUCCESS );
}
