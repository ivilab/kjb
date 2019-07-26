/* $Id$ */
/* =========================================================================== *
   |
   |  Copyright (c) 1994-2010 by Kobus Barnard (author)
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
 * =========================================================================== */

#include <string>
#include <fstream>
#include <iostream>
#include <ctime>

#include "i_cpp/i_image.h"
#include "edge_cpp/vanishing_point_detector.h"
#include "edge_cpp/features_manager.h"
#include "l/l_sys_rand.h"

using namespace std;

int main()
{
    using namespace kjb;

    long before, after;


    before = time(NULL);
    //Image img("001.jpg");
    Image img("001.tif");
    Image imgb("001.jpg");
    Image img2 = imgb;
    Image img3 = imgb;
    Image img4 = imgb;
    Image img5 = imgb;

    Features_manager fm(img);
    after = time(NULL);
    cout << "Total time : " << (after-before) << endl;
    fm.write("features");

    Features_manager fm2("features");

    fm2.get_manhattan_world().draw_corners3(img2);
    fm2.get_manhattan_world().draw_corners2(img4);
    fm2.get_manhattan_world().draw_segments_with_vp_assignments(img3);
    fm2.get_manhattan_world().draw_lines_from_segments_midpoint_to_vp(img5);
    //fm2.get_manhattan_world().get_corner_2(0).draw(img2);
    //fm2.get_manhattan_world().get_corner_2(0).draw(img3, true);
    img2.write("corners3.jpg");
    img3.write("assignments.jpg");
    img4.write("corners2.jpg");
    img5.write("vpts.jpg");

    std::cout << "Number of corners 2:" << fm2.get_manhattan_world().get_num_corners2() << std::endl;
    std::cout << "Number of corners 3:" << fm2.get_manhattan_world().get_num_corners3() << std::endl;
    //fm2.get_manhattan_world().print_corners(std::cout);


    return 0;
}

