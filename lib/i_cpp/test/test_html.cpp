/**
 * @file
 * @brief unit test for html hex triplet output of pixel channel values
 * @author Andrew Predoehl
 */
/*
 * $Id: test_html.cpp 13572 2012-12-25 02:19:07Z predoehl $
 */

#include <l/l_def.h>
#include <l/l_sys_lib.h>
#include <i_cpp/i_pixel.h>

#include <iostream>
#include <cassert>

int main()
{
    kjb::PixelRGBA p1( 255, 0, 0 ); // red
    assert( 0 == strcmp( p1.as_hex_triplet().c_str(), "#ff0000" ) );

    kjb::PixelRGBA p2( 0, 255, 0 ); // green
    assert( 0 == strcmp( p2.as_hex_triplet().c_str(), "#00ff00" ) );

    kjb::PixelRGBA p3( 0, 0, 255 ); // blue
    assert( 0 == strcmp( p3.as_hex_triplet().c_str(), "#0000ff" ) );

    kjb::PixelRGBA p4( 0, 0, 0 );   // black
    assert( 0 == strcmp( p4.as_hex_triplet().c_str(), "#000000" ) );

    kjb::PixelRGBA p5( 255, 255, 255 ); // white
    assert( 0 == strcmp( p5.as_hex_triplet().c_str(), "#ffffff" ) );

    if ( kjb_c::is_interactive() )
    {
        std::cout << __FILE__ << ": success!\n";
    }

    return EXIT_SUCCESS;
}
