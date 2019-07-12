/**
 * @file
 * @author Andrew Predoehl
 * @brief test program for minimum bipartite match function
 */
/*
 * $Id: test_mbm.cpp 22174 2018-07-01 21:49:18Z kobus $
 */

#include <graph_cpp/graph_min_bp.h>
#include <iostream>

#define NRW 6   /* number of rows       */
#define NCL NRW /* number of columns    */

int main( int argc, char** argv )
{
    int iii, jjj, nnn;
    kjb::Matrix wt( NRW, NCL );
    kjb::Int_vector as;
    double cost;

    const int w2[NRW * NCL] =   {   23, 42, 17, 93, 20, 17,
                                    87, 42, 23, 23, 90, 10,
                                    12, 23, 34, 45, 56, 67,
                                    87, 76, 15, 54, 43, 32,
                                    82, 64, 93, 11, 44, 10,
                                    23, 23, 23, 23, 23, 23  };

    kjb_c::kjb_disable_paging();

    /*
     * Solution:
     *
     * Pair row  0  with column  4  at cost  20
     * Pair row  1  with column  5  at cost  10
     * Pair row  2  with column  0  at cost  12
     * Pair row  3  with column  2  at cost  15
     * Pair row  4  with column  3  at cost  11
     * Pair row  5  with column  1  at cost  23
     *                                     ------
     * Total cost:                           91
     */

    nnn = 0;
    for( iii = 0; iii < NRW; ++iii )
    {
        for( jjj = 0; jjj < NCL; ++jjj )
        {
            wt.at( iii, jjj ) = w2[ nnn++ ];
        }
    }

    KJB( EPETE( kjb::min_bipartite_match( wt, &as, &cost ) ) );
    KJB( EPETE( as.get_length() == NRW ? NO_ERROR : ERROR ) );

    if ( cost != 91 ) return EXIT_FAILURE;
    if ( as[0] != 4 ) return EXIT_FAILURE;
    if ( as[1] != 5 ) return EXIT_FAILURE;
    if ( as[2] != 0 ) return EXIT_FAILURE;
    if ( as[3] != 2 ) return EXIT_FAILURE;
    if ( as[4] != 3 ) return EXIT_FAILURE;
    if ( as[5] != 1 ) return EXIT_FAILURE;

    if ( kjb_c::is_interactive() )
    {
        for( iii = 0; iii < NRW; ++iii )
        {
            for( jjj = 0; jjj < NCL; ++jjj )
            {
                std::cout << ' ' << int( wt.at( iii, jjj ) );
            }
            std::cout << '\n';
        }

        std::cout << "cost = " << cost << '\n';

        for( iii = 0; iii < NRW; ++iii )
        {
            std::cout   << "assign row " << iii << " to column " << as[ iii ]
                        << ".\n";
        }
    }

    //////////////////////////////////////////////////////////////////

    wt.at(0,0) *= -1; // this should provoke an error
    int rc = kjb::min_bipartite_match( wt, &as, &cost );
    if ( rc != kjb_c::ERROR ) return EXIT_FAILURE; // result must be ERROR.

    return EXIT_SUCCESS;
}

