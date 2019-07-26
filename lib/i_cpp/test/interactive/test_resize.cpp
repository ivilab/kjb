/**
 * @file
 * @brief test image scaling
 * @author'
 * @author Andy Predoehl
 */
/*
 * $Id: test_resize.cpp 15900 2013-10-25 02:14:07Z predoehl $
 */

#include <i_cpp/i_image.h>

int main(int argc, char** argv)
{
    kjb::Image I("image.jpg");
    kjb::Image J = kjb::scale_image(I, 2.0);
    kjb::Image k = kjb::scale_image(I, 0.5);

    if (kjb_c::is_interactive())
    {
        I.display("original");
        J.display("double the height");
        k.display("half the height");
        kjb_c::nap(10*1000); /* ten seconds */
    }

    return EXIT_SUCCESS;
}
