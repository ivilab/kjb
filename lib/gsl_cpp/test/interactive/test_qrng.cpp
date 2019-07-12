#include "gsl_cpp/gsl_cpp_incl.h"

#include <iostream>

class Gridly {
    std::string m_gr;

    int mono_ping( double w ) const
    {
        assert( 0 <= w && w < 1 );
        int z = static_cast< int >( w * EDGE );
        return std::min( std::max( z, 0 ), EDGE - 1 );
    }

public:
    enum { EDGE = 25 };

    Gridly()
    {
        for( int i = 0; i < EDGE; ++i )
            m_gr += std::string( EDGE, ' ' ) + '\n';

        assert( EDGE * ( 1 + EDGE ) == m_gr.size() );
    }

    void write() const
    {
        std::cout << m_gr;
    }

    void ping( double x, double y )
    {
        int r = mono_ping( y );
        int c = mono_ping( x );
        int i = r * ( EDGE + 1 ) + c;
        if ( ' ' == m_gr.at( i ) )
            m_gr.at( i ) = '1';
        else
            m_gr.at( i ) += 1; // follow the ascii road; but hopefully not far
    }
};

int main()
{
    //kjb::Gsl_Qrng_Niederreiter qrng( 2 );
    //kjb::Gsl_Qrng_Sobol qrng( 2 );
    kjb::Gsl_Qrng_Halton qrng( 2 );
    //kjb::Gsl_Qrng_Rvs_Halton qrng( 2 );
    Gridly g;
    //double xy[ 2 ];
    for( int j = 0; j < 500; ++j ) {
        //qrng.read( xy );
        kjb::Vector xy( qrng.read() );
        assert( 2 == xy.size() );
        std::cout << "Iter " << j << " yielded sample (" << xy[0] << ','
                << xy[1] << ")\n";
        g.ping( xy[0], xy[1] );
        g.write();
    }
    std::cout << "That was for the algorithm called " << qrng.name()
        << "\nDid all that look OK?\n";
    return EXIT_SUCCESS;
}
