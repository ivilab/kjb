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

int main(int argc, char ** argv)
{
    using namespace kjb;

    long before, after;

    string name;

    name.append("~/data/furniture/ucbroom/");
    name.append(argv[1]);
    name.append(".jpg");

    string features("/Users/prusso83/data/furniture/ucbroom/features/");
    string corners(features);
    corners.append("images/");
    string corners2(corners);
    string vpts(corners);
    string assignments(corners);
    features.append(argv[1]);
    features.append("_features.txt");

    vpts.append(argv[1]);
    assignments.append(argv[1]);
    corners.append(argv[1]);
    corners2.append(argv[1]);
    corners.append("_corners3.jpg");
    corners2.append("_corners2.jpg");
    vpts.append("_vpts.jpg");
    assignments.append("_as.jpg");

    Image img(name.c_str());
    Image img2 = img;
    Image img3 = img;
    Image img4 = img;
    Image img5 = img;

    if(argc == 3)
    {
         Features_manager fm2(img);
         fm2.get_manhattan_world().draw_segments_with_vp_assignments(img2, 1.0);
         fm2.get_manhattan_world().draw_lines_from_segments_midpoint_to_vp(img3);
         fm2.get_manhattan_world().draw_corners3up(img4, false, 1.0);
         fm2.get_manhattan_world().draw_corners2up(img5, false, 1.0);
         fm2.write(features.c_str());
    }
    else
    {
        Features_manager fm(features.c_str());
        fm.get_manhattan_world().draw_segments_with_vp_assignments(img2, 1.0);
        fm.get_manhattan_world().draw_lines_from_segments_midpoint_to_vp(img3);
        fm.get_manhattan_world().draw_corners3up(img4, false, 1.0);
        fm.get_manhattan_world().draw_corners2up(img5, false, 1.0);
    }
    img2.write(assignments.c_str());
    img3.write(vpts.c_str());
    img4.write(corners.c_str());
    img5.write(corners2.c_str());

    return 0;
}

