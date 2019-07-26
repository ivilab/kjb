/**
 * @file
 * @brief test hue-saturation-value cone
 * @author Andrew Predoehl
 */
/*
 * $Id: test_hsv.cpp 9333 2011-04-20 03:52:13Z predoehl $
 */

#include <i_cpp/i_cpp_incl.h>

int main()
{
    const int CHUNK = 100, NUMCHUNK = 10;
    kjb::Image circles( 6 + CHUNK * NUMCHUNK, 1 + CHUNK * NUMCHUNK );

    const kjb::PixelRGBA gray( kjb::PixelRGBA::create_gray( 150 ) );
    circles.draw_aa_rectangle( 0,0,5+CHUNK*NUMCHUNK,CHUNK*NUMCHUNK, gray );

    float value = 0;
    for( int chunkrow = 0; chunkrow < NUMCHUNK; ++chunkrow )
    {
        for( int chunkcol = 0; chunkcol < NUMCHUNK; ++chunkcol,
                                                        value += 0.010101 )
        {

            int rorigin = CHUNK / 2 + chunkrow * CHUNK,
                corigin = CHUNK / 2 + chunkcol * CHUNK,
                rr = CHUNK / 2;

            for( float ss = 0; ss <= 1; ss += 0.0526, rr -= 5 )
            {
                int cc = -CHUNK / 2;
                for( float hh = 0; hh <= 1; hh += 0.0526, cc += 5 )
                {

                    circles.draw_aa_rectangle( rr+rorigin, cc+corigin,
                                        rr+rorigin+5, cc+corigin+5, 
                                        kjb::PixelHSVA( hh, ss, value ) );
                }
            }
        }
    }

    circles.display( "hue, saturation, value" );
    sleep( 60 );
    return EXIT_SUCCESS;
}
