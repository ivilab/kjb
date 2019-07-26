/**
 * @file
 * @brief unit test for the get channel function
 * @author Andrew Predoehl
 */
/*
 * $Id: test_channels.cpp 15900 2013-10-25 02:14:07Z predoehl $
 */

#include <i_cpp/i_image.h>

namespace {

int nnn = 0;

void show(const kjb::Image& iii)
{
    std::string title = "Image 0";
    title[6] += nnn++;

    if (kjb_c::kjb_fork()) return;
    iii.display(title);
    while(1) {}
}

int test()
{
    kjb::Image iii("image.jpg");
    const kjb::Matrix   mr(iii.get_channel(kjb::Image::RED)),
                        mg(iii.get_channel(kjb::Image::GREEN)),
                        mb(iii.get_channel(kjb::Image::BLUE)),
                        *chan[3] = {&mr, &mg, &mb};
    show(iii);

    for (int ccc = kjb::Image::RED; ccc < kjb::Image::END_CHANNELS; ++ccc)
    {
        kjb::Image jjj(*chan[ccc]);
        show(jjj);
    }

    return 0;
}


}

int main()
{
    try
    {
        return test();
    }
    catch (const kjb::Exception& e)
    {
        e.print_details_exit();
    }
}
