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
    std::cout << filename << std::endl;

    base.append("orientation_maps/");
    string orientation_map(base);
    orientation_map.append("or_");
    orientation_map.append(argv[2]);
    orientation_map.append(".tif");

    string corners3_name = base;
    corners3_name.append("corner3_images/");
    corners3_name.append(argv[2]);
    corners3_name.append("_corners3.jpg");

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

    string good_values_name(base);
    good_values_name.append("good_vpts/");
    good_values_name.append(argv[2]);
    good_values_name.append(".txt");

    string features_file(base);
    features_file.append("features/");
    features_file.append(argv[2]);
    features_file.append("_features.txt");

    Image or_map(orientation_map.c_str());
    Image image(filename.c_str());
    Image corner3_image(image);
    Image vpts_image(image);
    Image assignments_image(image);
    Image segments_image(image);

    std::vector<Vanishing_point> vpts;

    double focal_length;
    if(!kjb::relaxed_vanishing_point_estimation(vpts, focal_length, or_map, 0.95))
    {
        KJB_THROW_2(KJB_error,"Error, could not compute vanishing points");
    }

    kjb::Edge_set * edges;
    Edge_segment_set * edge_segments;
    Canny_edge_detector edge_detector(1.2, 2.55, 2.04, 20, true);
    edges = edge_detector.detect_edges(or_map);
    edges->remove_short_edges(10);
    edges->break_edges_at_corners(0.85, 7);
    edges->break_edges_at_corners(0.97, 60);//0.97, 60
    edges->remove_short_edges(10); //10

    edge_segments = new Edge_segment_set(edges);
    edge_segments->remove_overlapping_segments(*edges);
    for(unsigned int i = 0; i < edge_segments->size(); i++)
    {
        std::cout << edge_segments->get_segment(i);
    }

    double outlier_threshold = 0.16;
    Manhattan_world * manhattan_world =  new Manhattan_world(vpts, focal_length);
    manhattan_world->assign_segments_to_vpts(*edge_segments, outlier_threshold);
    manhattan_world->create_corners();

    manhattan_world->draw_corners3(corner3_image);
    manhattan_world->draw_segments_with_vp_assignments(assignments_image);
    manhattan_world->draw_lines_from_segments_midpoint_to_vp(vpts_image);
    edge_segments->randomly_color(segments_image);
    manhattan_world->print_vanishing_points(std::cout);

    corner3_image.write(corners3_name.c_str());
    vpts_image.write(vpts_name.c_str());
    assignments_image.write(assignments_name.c_str());
    segments_image.write(segments_name.c_str());

    ofstream good_values(good_values_name.c_str());
    for(unsigned int i = 0; i < 3; i++)
    {
        vpts[i].write(good_values);
    }
    good_values << focal_length << std::endl;
    good_values.close();

    Features_manager fm(edges, edge_segments, manhattan_world);
    fm.set_edge_detection_parameters(1.2, 2.55, 2.04, 20, true);
    fm.set_manhattan_world_parameters(0.95, outlier_threshold);
    fm.write(features_file.c_str());

    return 0;
}

