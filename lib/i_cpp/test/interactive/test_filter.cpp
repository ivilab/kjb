/*
 * $Id: test_filter.cpp 15900 2013-10-25 02:14:07Z predoehl $
 */

#include "i_cpp/i_image.h"
#include "i_cpp/i_filter.h"
#include "m_cpp/m_matrix.h"
#include "i/i_display.h"

int main(int argc, char** argv)
{
    try
    {
        kjb::Matrix M(5, 5);
        kjb::Filter K1(5, 5);

        for (int i = 0; i < M.get_num_rows(); i++)
        {
            for (int j = 0; j < M.get_num_cols(); j++)
            {
                M.at(i, j) = i * j;
                K1.at(i, j) = i * j;
            }
        }

        kjb::Filter K2(M);

        kjb::Image I1("image.jpg");
        kjb::Filter K3 = kjb::gaussian_filter(9, 3);
        kjb::Image I2 = I1 * K3;
        kjb::Filter K4 = kjb::laplacian_of_gaussian_filter(19, 0.5);
        kjb::Image I3 = I1 * K4;
        kjb::Image i4(kjb::get_inverted(I1));

        if (kjb_c::is_interactive())
        {
            int wh[4];
            wh[0] = I1.display("original image");
            wh[1] = I2.display("blurred image");
            wh[2] = I3.display("log 0.01");
            wh[3] = i4.display("inverted image");

            kjb_c::nap(10*1000); // ten seconds
            kjb_c::close_displayed_image(wh[0]);
            kjb_c::close_displayed_image(wh[1]);
            kjb_c::close_displayed_image(wh[2]);
            kjb_c::close_displayed_image(wh[3]);
        }
    }
    catch (kjb::Exception& e)
    {
        e.print_details_exit();
    }

    return EXIT_SUCCESS;
}

