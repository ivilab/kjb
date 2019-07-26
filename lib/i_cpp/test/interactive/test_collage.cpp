/**
 * @file
 * @brief simple demo of collage generation
 * @author Andrew Predoehl
 */
/*
 * $Id: test_collage.cpp 20769 2016-07-28 15:31:21Z ernesto $
 */

#include <i_cpp/i_image.h>
#include <i_cpp/i_collage.h>
#include <l_cpp/l_util.h>

namespace {

int test1()
{
    // load a few images from ../input/
    const std::string path = std::string("..") + DIR_STR + "input" + DIR_STR;

    const kjb::Image    i2(path + "123_map.jpeg"),
                        i1(path + "123_trail.jpeg"),
                        i3(path + "test.jpeg"),
                        *images[4] = {&i1, &i2, &i3, &i2};

    // build a collage image
    kjb::Image c1(kjb::make_collage(images, 3, 1));
    kjb::Image c2(kjb::make_collage(images, 2, 2));
    kjb::Image c3(kjb::make_collage(images, 1, 3));

    // show it
    if (0 == kjb_c::kjb_fork())
    {
        c1.display("collage 3x1");
        c2.display("collage 2x2");
        c3.display("collage 1x3");
        while (1) {}
    }

    return kjb_c::NO_ERROR;
}

int test2()
{
    // load a few images from ../input/
    const std::string path = std::string("..") + DIR_STR + "input" + DIR_STR;

    kjb::Image    i2(path + "123_map.jpeg"),
                  i1(path + "123_trail.jpeg"),
                  i3(path + "test.jpeg");

    std::vector< kjb::Image > iii(4);
    iii.at(0).swap(i1);
    iii.at(1) = i2; // deep copy
    iii.at(2).swap(i3);
    iii.at(3).swap(i2);

    // build a collage image
    kjb::Image c1(kjb::make_collage(& iii.front(), 3, 1));
    kjb::Image c2(kjb::make_collage(& iii.front(), 2, 2));
    kjb::Image c3(kjb::make_collage(& iii.front(), 1, 3));

    // show it
    if (0 == kjb_c::kjb_fork())
    {
        c1.display("test2 3x1");
        c2.display("test2 2x2");
        c3.display("test2 1x3");
        while (1) {}
    }

    return kjb_c::NO_ERROR;
}


}

int main()
{
    try
    {
        KJB(EPETE(test1()));
        KJB(EPETE(test2()));
    }
    catch(const kjb::Exception& e)
    {
        e.print_details_exit();
    }
    return EXIT_SUCCESS;
}

