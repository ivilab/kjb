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

int main(int argc, char **argv)
{
    using namespace kjb;

    if(argc < 3)
    {
        std::cout << "Usage: <base_dir> <input_image>" << std::endl;
    }

    string base(argv[1]);
    base.append("/");

    string filename(base);
    filename.append(argv[2]);
    filename.append(".jpg");

    string features_file(base);
    features_file.append("orientation_maps/features/");
    features_file.append(argv[2]);
    features_file.append("_features.txt");

    base.append("validation/");

    string corners3_name = base;
    corners3_name.append("corner3_images/");
    corners3_name.append(argv[2]);
    corners3_name.append("_corners3.jpg");

    string features_out = base;
    features_out.append("features/");
    features_out.append(argv[2]);
    features_out.append("_features.txt");

    string vpts_name = base;
    vpts_name.append("vpts_images/");
    vpts_name.append(argv[2]);
    vpts_name.append("_vpts.jpg");

    string assignments_name = base;
    assignments_name.append("assignment_images/");
    assignments_name.append(argv[2]);
    assignments_name.append("_assignments.jpg");

    string segments_name = base;
    segments_name.append("segment_images/");
    segments_name.append(argv[2]);
    segments_name.append("_segments.jpg");

    Image image(filename.c_str());
    Image corner3_image(image);
    Image vpts_image(image);
    Image assignments_image(image);
    Image segments_image(image);

    Features_manager fm(features_file.c_str());

    fm.get_manhattan_world().draw_corners3(corner3_image);
    fm.get_manhattan_world().draw_segments_with_vp_assignments(assignments_image);
    fm.get_manhattan_world().draw_lines_from_segments_midpoint_to_vp(vpts_image);
    fm.get_edge_segments().randomly_color(segments_image);
    fm.get_manhattan_world().print_vanishing_points(std::cout);

    corner3_image.write(corners3_name.c_str());
    vpts_image.write(vpts_name.c_str());
    assignments_image.write(assignments_name.c_str());
    segments_image.write(segments_name.c_str());
    fm.write(features_out.c_str());


    return 0;
}

