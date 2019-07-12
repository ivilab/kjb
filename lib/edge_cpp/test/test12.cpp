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
//  Image imgb("001.jpg");
    Image img2 = img;
    Image img3 = img;
    Image img4 = img;
    Image img5 = img;

    std::vector<Vanishing_point> vpts;

    double focal_length;
    if(!kjb::relaxed_vanishing_point_estimation(vpts, focal_length, img, 0.95))
    {
        //KJB_THROW_2(KJB_error,"Error, could not compute vanishing points");
        std::cerr<<"Error, could not compute vanishing points"<<std::endl;
    }

    std::cout << vpts[0].is_at_infinity() << std::endl;
    std::cout << vpts[1].is_at_infinity() << std::endl;
    std::cout << vpts[2].is_at_infinity() << std::endl;

    kjb::Edge_set * edges;
    Edge_segment_set * edge_segments;
    Canny_edge_detector edge_detector(1.2, 2.55, 2.04, 20, true);
    edges = edge_detector.detect_edges(img);
    edges->remove_short_edges(10);
    edges->break_edges_at_corners(0.85, 7);
    edges->break_edges_at_corners(0.97, 60);
    edges->remove_short_edges(10);

    edge_segments = new Edge_segment_set(edges);
    std::cout << edge_segments->size() << std::endl;
    std::cout << "Num segs before: " << edge_segments->size() << std::endl;
    edge_segments->remove_overlapping_segments(*edges);
    std::cout << "Num segs after: " << edge_segments->size() << std::endl;

    Manhattan_world manhattan_world(vpts, focal_length);
    manhattan_world.assign_segments_to_vpts(*edge_segments, 0.16);
    manhattan_world.create_corners();


    manhattan_world.draw_corners3(img2);
    manhattan_world.draw_corners2(img4);
    //manhattan_world.draw_segments_with_vp_assignments(img3);
    //manhattan_world.draw_lines_from_segments_midpoint_to_vp(img5);
    edges->randomly_color(img3);
    edge_segments->randomly_color(img5);

    img2.write("corners3.jpg");
    img3.write("edges.jpg");
    img4.write("corners2.jpg");
    img5.write("edge_segments.jpg");

    std::cout << "Number of corners 2:" << manhattan_world.get_num_corners2() << std::endl;
    std::cout << "Number of corners 3:" << manhattan_world.get_num_corners3() << std::endl;
    std::cout << "Vpts" << std::endl;
    std::cout << "Segments num:" << edge_segments->size() << std::endl;

    manhattan_world.print_vanishing_points(std::cout);

    for(unsigned int k = 0; k < edge_segments->size(); k++)
    {
        std::cout << edge_segments->get_segment(k).get_line_params() << std::endl;
    }

    delete edges;
    delete edge_segments;

    return 0;
}

