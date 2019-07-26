#include "gsl_cpp/gsl_cpp_incl.h"

#include <i_cpp/i_cpp_incl.h>

int main()
{
    //kjb::Gsl_Qrng_Niederreiter qrng( 2 );
    //kjb::Gsl_Qrng_Sobol qrng( 2 );
    kjb::Gsl_Qrng_Halton qrng( 2 );
    //kjb::Gsl_Qrng_Rvs_Halton qrng( 2 );

    const int SZ = 1000;
    kjb::Image im( kjb::Image::create_zero_image( SZ, SZ ) );
    kjb::PixelRGBA p( 250, 250, 250 );

    for( int j = 0; j < 10*SZ; ++j ) {
        kjb::Vector xy = qrng.read();
        int r = std::min( SZ-1, std::max( 0, static_cast<int>( xy[1]*SZ+0.5)));
        int c = std::min( SZ-1, std::max( 0, static_cast<int>( xy[0]*SZ+0.5)));
        kjb::PixelRGBA q = im.at( r, c );
        im.at( r, c ) = (( p + q )*0.5 ).ow_clamp().ow_floor();
    }
    im.display( "that is what it is" );
    std::cout << "That was for the algorithm called " << qrng.name()
        << "\nDid all that look OK?\n";
    sleep(60 );
    return EXIT_SUCCESS;
}
