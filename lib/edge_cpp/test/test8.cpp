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

    Image img("001.jpg");
    Image img2 = img;
    Image img3 = img;

    /*Features_manager fm;
    Canny_edge_detector edge_detector(1.2, 0.01*255, 0.008*255, 30, true);
    Edge_set_ptr ptr = edge_detector(img);
    fm.edges = *ptr;
    fm._edges_available = true;

    fm.edges.remove_short_edges(20);
    fm.edges.break_edges_at_corners(0.85, 7);
    fm.edges.break_edges_at_corners(0.97, 60);
    fm.edges.remove_short_edges(10);
    fm.edge_segments.init_from_edge_set(fm.edges);
    fm._edge_segments_available = true;

    std::vector<Vanishing_point> vpts;
    robustly_estimate_vanishing_points(vpts,img);
    fm.manhattan_world = new Manhattan_world(fm.edge_segments, vpts);
    fm._manhattan_world_available = true;
    fm.manhattan_world->draw_segments_with_vp_assignments(img3, 1.0);
    fm.manhattan_world->draw_lines_from_segments_midpoint_to_vp(img2, 1.0);

    fm.write("features");

    img2.write("001assigns.jpg");
    img3.write("001segs.jpg"); */


    return 0;
}

