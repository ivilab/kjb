/**
 * @file
 * @brief test hue-saturation-value color mixing
 * @author Andrew Predoehl
 *
 * What this shows:  if you make linear combinations of two colors, it does
 * not matter if you do it in RGB space or in HSY space.  That's because the
 * RGB to HSY transformation is a linear transformation.  I'm not sure how to
 * square that with the perceptual "magic" that HSY does, where if you move a
 * unit distance away you'll get a more or less constant perceptual change,
 * unlike RGB which could take you from extremely dark green to extremely dark
 * blue and you wouldn't even perceive a difference, but a very obvious move
 * from bright green to bright blue might be the same distance in RGB.
 * Anyway, this test proves that linear transformations act like linear
 * transformations.
 */
/*
 * $Id: test_mix.cpp 9333 2011-04-20 03:52:13Z predoehl $
 */

#include <l/l_sys_rand.h>
#include <i_cpp/i_cpp_incl.h>

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
    kjb::Image bands( CHUNK * NUMCHUNK, CHUNK * NUMCHUNK );

    const kjb::PixelRGBA gray( kjb::PixelRGBA::create_gray( 150 ) );
    bands.draw_aa_rectangle( 0,0,CHUNK*NUMCHUNK-1,CHUNK*NUMCHUNK-1, gray );

    kjb::Vector u( 3 ), v( 3 ), a(3), b(3), c(3);
    kjb::PixelRGBA pu(0,0,0), pv=pu, pa=pu, pb=pu, pc=pu;
    for( int chunkrow = 0; chunkrow < NUMCHUNK; ++chunkrow )
    {
        for( int chunkcol = 0; chunkcol < NUMCHUNK; ++chunkcol )
        {
            get_a_color( & u, & pu );
            get_a_color( & v, & pv );

            int cr1 = chunkrow * CHUNK, cr2 = cr1 + CHUNK/2 - 1;
            int cr3 = cr2 + 1, cr4 = cr1 + CHUNK - 1;
            bands.draw_aa_rectangle( cr1, 0,       cr4, CHUNK-1,   pu );
            bands.draw_aa_rectangle( cr1, 9*CHUNK, cr4, 10*CHUNK-1, pv );

            for ( int jj = 1; jj < 9; ++jj ) {
                a = u * (9-jj)/9.0 + v * jj/9.0;
                int ra = get_pixel_from_hsluma_space( a, &pa );
                assert( ra == EXIT_SUCCESS );
                bands.draw_aa_rectangle( cr1, jj*CHUNK, cr2, (1+jj)*CHUNK-1,
                                                                        pa );
            }

            /* to see what is going on better, temporarily tweak the constant
             * 9.0 there to, say, 10.0.
             */
            for ( int jj = 1; jj < 9; ++jj ) {
                bands.draw_aa_rectangle( cr3, jj*CHUNK, cr4, (1+jj)*CHUNK-1,
                                        pu * ((9-jj)/9.0) + pv * (jj/9.0) );
            }
        }
    }

    bands.display( "color mixing - RGB and HSY" );
    sleep( 30 );
    return EXIT_SUCCESS;
}
