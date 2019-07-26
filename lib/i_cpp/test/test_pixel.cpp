/*
 * $Id: test_pixel.cpp 5850 2010-05-04 03:35:10Z predoehl $
 */

#include "i_cpp/i_pixel.h"

#include <cassert>

int main()
{
    const kjb::PixelRGBA pred(100,0,0);
    kjb::PixelRGBA pblu(0,0,100);
    kjb::PixelRGBA predder( 2.0*pred );

    /* make sure our ctor parameters got inside the structure */
    assert( 100.0 == pred.r );
    assert( 0.0 == pred.g );
    assert( 0.0 == pred.b );
    assert( 255.0 == pred.extra.alpha );

    /* make sure that scaling works */
    assert( 200.0 == predder.r );
    assert( 0.0 == predder.g );
    assert( 0.0 == predder.b );
    assert( 510.0 == predder.extra.alpha ); // surprised?

    /* default ctor -- prcp is potentially full of garbage values */
    kjb::PixelRGBA prcp;
    /* default assignment operator */
    prcp = pred;
    /* Did the default assignment operator do its job? */
    assert( 100.0 == prcp.r );
    assert( 0.0 == prcp.g );
    assert( 0.0 == prcp.b );
    assert( 255.0 == prcp.extra.alpha );
    /* tweak the copy; verify the tweak did not affect the original */
    prcp += kjb::PixelRGBA(1,2,3,4);
    /* verify the tweak actually happened */
    assert( 101.0 == prcp.r );
    assert( 2.0 == prcp.g );
    assert( 3.0 == prcp.b );
    assert( 259.0 == prcp.extra.alpha );
    /* but no aliasing allowed! */
    assert( 100.0 == pred.r );
    assert( 0.0 == pred.g );
    assert( 0.0 == pred.b );
    assert( 255.0 == pred.extra.alpha );

    /* by the way, the Pixel has gigantic dynamic range */
    predder *= 2;
    assert( 400.0 == predder.r );

    /* verify clamping */
    kjb::PixelRGBA pox( predder.clamp() );
    assert( 400.0 == predder.r );
    assert( 1020.0 == predder.extra.alpha );
    assert( 255.0 == pox.r );
    assert( 255.0 == pox.extra.alpha );

    /* again we add, again we verify no aliasing.  sort of redundant. */
    kjb::PixelRGBA ppur( pred + pblu );
    assert( 0.0 == pblu.r );
    assert( 0.0 == pblu.g );
    assert( 100.0 == pblu.b );
    assert( 255.0 == pblu.extra.alpha );
    assert( 100.0 == pred.r );
    assert( 0.0 == pred.g );
    assert( 0.0 == pred.b );
    assert( 255.0 == pred.extra.alpha );
    assert( 100.0 == ppur.r );
    assert( 0.0 == ppur.g );
    assert( 100.0 == ppur.b );
    assert( 510.0 == ppur.extra.alpha );

    kjb::PixelRGBA pg = kjb::PixelRGBA::create_gray( 123 );
    assert( 123.0 == pg.r );
    assert( 123.0 == pg.g );
    assert( 123.0 == pg.b );
    assert( 255.0 == pg.extra.alpha );

    return EXIT_SUCCESS;
}
