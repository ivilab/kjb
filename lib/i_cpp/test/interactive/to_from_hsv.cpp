/**
 * @file
 * @brief test that we can transform to and from RGB <=> HSV <=> HSY
 * @author Andrew Predoehl
 *
 * HSV means "hue, saturation, value" -- an alternative way to parameterize
 * color.  HSY stands for hue, saturation, and Y is for luma, an alternative
 * metric for lightness.
 *
 * Output of this should be an image with ten horizontal color bars.
 * Hopefully it will be imperceptible that each bar is really a row of 3 tiles.
 * The left column is the original, randomly-selected color.  The middle 
 * column is what you get after converting RGB ==> HSV and then converting the
 * result HSV ==> RGB.  The right column is what you get after RGB ==> HSY
 * then HSY ==> RGB.
 */
/*
 * $Id: to_from_hsv.cpp 9336 2011-04-20 20:45:22Z predoehl $
 */

#include <l/l_sys_rand.h>
#include <i_cpp/i_cpp_incl.h>

kjb_c::Pixel get_rgb_rand()
{
    return kjb::PixelRGBA(  55 + 200 * kjb_c::kjb_rand(),
                            55 + 200 * kjb_c::kjb_rand(),
                            55 + 200 * kjb_c::kjb_rand()
                        );
}

void get_a_color( kjb::Vector* u, kjb_c::Pixel* pu )
{
    int rc;
    do
    {
        u -> at( 0 ) = kjb_c::kjb_rand();
        u -> at( 1 ) = kjb_c::kjb_rand();
        u -> at( 2 ) = kjb_c::kjb_rand();
        rc = kjb::get_pixel_from_hsluma_space( *u, pu );
    }
    while ( EXIT_FAILURE == rc );
}


int main()
{
    kjb_c::kjb_seed_rand_with_tod();
    const int CHUNK = 50, NUMCHUNK = 10;
    kjb::Image bands( CHUNK * NUMCHUNK, CHUNK * 3 );

    kjb::Vector u( 3 );
    kjb::PixelRGBA pu(0,0,0), pv=pu, black(0,0,0);
    kjb::PixelHSVA q1;
    float h, s, v;
    for( int chunkrow = 0; chunkrow < NUMCHUNK; ++chunkrow )
    {
        int cr1 = chunkrow * CHUNK, cr2 = cr1 + CHUNK - 1;
        for( int chunkcol = 0; chunkcol < 3; ++chunkcol )
        {
            pu = get_rgb_rand();
            bands.draw_aa_rectangle( cr1, 0,       cr2, CHUNK-1,   pu );

            q1 = pu;
            q1.get_hsv( &h, &s, &v );
            kjb::PixelHSVA q2( h, s, v );
            bands.draw_aa_rectangle( cr1, CHUNK, cr2, 2*CHUNK-1,   q2 );

            u = kjb::hsluma_space( pu );
            int rc = kjb::get_pixel_from_hsluma_space( u, &pv );
            if ( EXIT_SUCCESS == rc )
            {
                bands.draw_aa_rectangle( cr1, 2*CHUNK, cr2, 3*CHUNK-1,   pv );
            }
            else
            {
                bands.draw_aa_rectangle( cr1, 2*CHUNK, cr2, 3*CHUNK-1, black );
            }
        }
    }

    bands.display( "transform to and fro" );
    sleep( 30 );
    return EXIT_SUCCESS;
}
