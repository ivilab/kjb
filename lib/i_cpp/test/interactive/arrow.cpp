/**
 * @file
 * @brief demo program for drawing arrows
 * @author Andrew Predoehl
 */
/*
 * $Id: arrow.cpp 15381 2013-09-20 00:19:18Z predoehl $
 */

#include <i_cpp/i_image.h>

int main(int argc, char** argv)
{
    const int HALFSIZE = 100;
    kjb::Image i(HALFSIZE *2, HALFSIZE * 2, 0, 0, 0);
    kjb_c::Pixel red, yellow, white;
    red.r = 200;
    red.g = 0;
    red.b = 0;
    yellow = red;
    yellow.g = 200;
    white = yellow;
    white.b = 200;

    kjb::Vector c(2), d(2);
    c(0) = c(1) = HALFSIZE + 20;

    for (int j = 0; j < 270; j += 36)
    {
        // d is a displacement, not a coordinate
        d(0) = HALFSIZE/2 * cos(j * M_PI/180); // x displacement is rightward
        d(1) = HALFSIZE/2 * sin(j * M_PI/180); // y displacement is downward

        // note second param is c+d, not merely d.
        i.draw_arrow(c, c+d, red);

        // just to make the row/col vs. x/y variation explicit:
        int ctr_row = c(1),
            ctr_col = c(0);
        i.draw_point(ctr_row, ctr_col, 8, white);

        int tip_row = ctr_row + d(1), // note this is plus, not minus.
            tip_col = ctr_col + d(0);
        i.draw_point(tip_row, tip_col, 3, yellow);
    }

    i.display("some arrows (press ctrl+c to end)");
    while(1) { kjb_c::nap(1000); }

    return 0;
}

