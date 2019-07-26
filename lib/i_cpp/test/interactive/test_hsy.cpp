/**
 * @file
 * @brief test hue-saturation-luma polyhedron
 * @author Andrew Predoehl
 */
/*
 * $Id: test_hsy.cpp 9345 2011-04-21 03:28:07Z predoehl $
 */

#include <m_cpp/m_cpp_incl.h>
#include <i_cpp/i_cpp_incl.h>

int main()
{
    const int CHUNK = 100, NUMCHUNK = 10;
    kjb::Image circles( 1 + CHUNK * NUMCHUNK, 1 + CHUNK * NUMCHUNK );

    const kjb::PixelRGBA gray( kjb::PixelRGBA::create_gray( 150 ) );
    circles.draw_aa_rectangle( 0,0,CHUNK*NUMCHUNK,CHUNK*NUMCHUNK, gray );

    float luma = 0, step = 2.0 / ( CHUNK - 1 );
    for ( int chunkrow = 0; chunkrow < NUMCHUNK; ++chunkrow )
    {
        for( int chunkcol = 0; chunkcol < NUMCHUNK; ++chunkcol, luma +=0.0101)
        {

            int rorigin = CHUNK / 2 + chunkrow * CHUNK,
                corigin = CHUNK / 2 + chunkcol * CHUNK;

            kjb::Vector bc( 3, luma );

            float yy = -1;
            for( int rr = CHUNK-1 - CHUNK/2; rr >= -CHUNK/2; --rr, yy += step )
            {
                float xx = -1;
                for( int cc = -CHUNK/2; cc < CHUNK - CHUNK/2; ++cc,xx += step )
                {
                    bc[ 0 ] = xx; // red-cyan axis
                    bc[ 1 ] = yy; // green-blue axis
                    kjb::PixelRGBA p(0,0,0);
                    int rc = kjb::get_pixel_from_hsluma_space( bc, &p );
                    if ( EXIT_SUCCESS == rc )
                    {
                        circles( rr+rorigin, cc+corigin ) = p;
                    }
                }
            }
            circles( rorigin, corigin ) = kjb::PixelRGBA(0,0,0);
        }
    }

    circles.display( "hue, saturation, luma space" );
    sleep( 60 );
    return EXIT_SUCCESS;
}
