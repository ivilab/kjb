/**
 * @file
 * @brief test prog demonstrating use of alpha channel
 * @author Andrew Predoehl
 */
/*
 * $Id: test_alpha.cpp 15303 2013-09-06 22:24:39Z predoehl $
 */

#include <i_cpp/i_image.h>
#include <i_cpp/i_pixel.h>

int main (int argc, char** argv)
{
    kjb::Image i(100, 100, 0, 0, 0), j(i);

    kjb::enable_transparency(i);
    kjb::enable_transparency(j);

    kjb::PixelRGBA p(200, 0, 0, 255), q(0, 200, 0, 255);

    /* draw fading rectangles */
    for (int a = 0; a < 50; ++a)
    {
        q.extra.alpha = p.extra.alpha = 255 - 5*a;
        for (int b = 0; b < 50; ++b)
        {
            i( a+25, b+25 ) = p;
            j( b+40, a+40 ) = q;
        }
    }

    i.write("foo_image.tif");
    j.write("bar_image.tif");

    return 0;
}
