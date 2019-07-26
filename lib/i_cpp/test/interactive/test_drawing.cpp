/*
 * $Id: test_drawing.cpp 16805 2014-05-15 20:13:44Z predoehl $
 */

#include "i/i_display.h"
#include "i_cpp/i_image.h"
#include "i_cpp/i_pixel.h"
#include <string>
#include <boost/lexical_cast.hpp>

const kjb::PixelRGBA red(200,0,0), green(0,200,0), blue(0,0,200), black(0,0,0);

int test_circle(int cx, int cy, int r, int lw)
{
    kjb::Image im("image.jpg");

    im.draw_circle(cx, cy, r, lw, red);

    using std::string;
    string temp = "";
    temp += "c = (" + boost::lexical_cast<string>(cx) + "," + boost::lexical_cast<string>(cy) + "), ";
    temp += "r = " + boost::lexical_cast<string>(r) + ", ";
    temp += "lw = " + boost::lexical_cast<string>(lw);

    return im.display(temp.c_str());
}

int test_disk(int cx, int cy, int r)
{
    kjb::Image im("image.jpg");

    im.draw_disk(cx, cy, r, blue);

    using std::string;
    string temp = "";
    temp += "c = (" + boost::lexical_cast<string>(cx) + "," + boost::lexical_cast<string>(cy) + "), ";
    temp += "r = " + boost::lexical_cast<string>(r) + ", ";

    return im.display(temp.c_str());
}



int main()
{
    std::vector<int> handles;

    int h;

    h = test_circle(10, 10, 50, 0);     handles.push_back(h);
    h = test_circle(100, 100, 20, 5);   handles.push_back(h);
    h = test_circle(152, 198, 8, 2);    handles.push_back(h);

    h = test_disk(10, 10, 50);          handles.push_back(h);
    h = test_disk(100, 100, 20);        handles.push_back(h);
    h = test_disk(152, 198, 8);         handles.push_back(h);

    sleep(10);
    std::for_each( handles.begin(), handles.end(),
                                std::ptr_fun(kjb_c::close_displayed_image) );

    #ifdef YES_WE_WANT_GARBAGE_POLICE
    fprintf(    stderr,
                "Garbage police report that %d Images were (ever) constructed "
                "via C++\nand %d of them are still alive now.\n", 
                kjb::Image::query_serial_counter(),
                kjb::Image::query_live_counter()
            );
    #endif
    return EXIT_SUCCESS;
}
