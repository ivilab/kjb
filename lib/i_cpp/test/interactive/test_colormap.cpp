/* $Id: test_colormap.cpp 17413 2014-08-29 01:20:58Z predoehl $ */
/* {{{=========================================================================== *
   |
   |  Copyright (c) 1994-2011 by Kobus Barnard (author)
   |
   |  Personal and educational use of this code is granted, provided that this
   |  header is kept intact, and that the authorship is not misrepresented, that
   |  its use is acknowledged in publications, and relevant papers are cited.
   |
   |  For other use contact the author (kobus AT cs DOT arizona DOT edu).
   |
   |  Please note that the code in this file has not necessarily been adequately
   |  tested. Naturally, there is no guarantee of performance, support, or fitness
   |  for any particular task. Nonetheless, I am interested in hearing about
   |  problems that you encounter.
   |
   |  Author:  Kyle Simek
 * =========================================================================== }}}*/

// vim: tabstop=4 shiftwidth=4 foldmethod=marker

#include <i_cpp/i_colormap.h>
#include <i_cpp/i_image.h>

void test_colormap(const std::string& name)
{
    const size_t width = 200;
    const size_t height = 200;

    kjb::Colormap map(name);
    kjb::Image img(width, height);

    for(size_t row = 0; row < height; row++)
    {
        for(size_t col = 0; col < width; col++)
        {  
            img(row, col) = map(((float) row)/height); 
        }
    }

    img.write(name + ".jpg");
}

int main()
{
    test_colormap("jet");
    test_colormap("hot");
    test_colormap("hsv");
    test_colormap("gray");
    test_colormap("cool");
}
