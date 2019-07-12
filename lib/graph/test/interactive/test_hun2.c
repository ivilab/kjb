#include <graph/hungarian.h>

/*
 * $Id: test_hun2.c 13646 2013-01-24 04:56:05Z predoehl $
 */

#define NRW 6   /* number of rows       */
#define NCL NRW /* number of columns    */

int main( int argc, char** argv )
{
    int iii, jjj, nnn, cost;
    Int_matrix *wt = NULL;
    Int_vector *as = NULL;

    const int w2[NRW * NCL] =   {   23, 42, 17, 93, 20, 17,
                                    87, 42, 23, 23, 90, 10,
                                    12, 23, 34, 45, 56, 67,
                                    87, 76, 15, 54, 43, 32,
                                    82, 64, 93, 11, 44, 10,
                                    23, 23, 23, 23, 23, 23  };

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

    EPETE( get_target_int_matrix( &wt, NRW, NCL ) );
    NPETE( wt );
    
    nnn = 0;
    for( iii = 0; iii < NRW; ++iii )
    {
        for( jjj = 0; jjj < NCL; ++jjj )
        {
            wt -> elements[ iii ][ jjj ] = w2[ nnn++ ];
        }
    }

    for( iii = 0; iii < NRW; ++iii )
    {
        for( jjj = 0; jjj < NCL; ++jjj )
        {
            printf( " %d", wt -> elements[ iii ][ jjj ] );
        }
        putchar( '\n' );
    }

    EPETE( int_hungarian( wt, &as, &cost ) );
    EPETE( as -> length == NRW ? NO_ERROR : ERROR );

    test_pso( "cost = %d\n", cost );

    for( iii = 0; iii < NRW; ++iii )
    {
        test_pso( "assign row %d to column %d.\n", iii, as[ iii ] );
    }

    return EXIT_SUCCESS;
}

