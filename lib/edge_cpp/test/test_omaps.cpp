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

    try
    {
        string img_name("./010.jpg");
        string features("./010_bestfeatures.txt");
        Image img(img_name.c_str());
        Features_manager fm(features.c_str());
        const Edge_segment_set & segments = fm.get_edge_segments();
        Edge_set * edge_set = new Edge_set("010_edges.txt");
        Edge_segment_set * edge_segments = new Edge_segment_set(edge_set);
        edge_segments->remove_overlapping_segments(*edge_set);
        Edge_set * temp_edges = edge_segments->convert_to_edge_set(img.get_num_rows(), img.get_num_cols());
        Edge_segment_set * good_edges = new Edge_segment_set(temp_edges);
        good_edges->remove_frame_segments(img.get_num_rows(), img.get_num_cols(),*temp_edges);
        delete edge_set;
        delete edge_segments;


        std::vector<int> assignments;
        const std::vector<Vanishing_point> & vpts = fm.get_manhattan_world().get_vanishing_points();
        double focal_length = fm.get_manhattan_world().get_focal_length();
        for(unsigned int i = 0; i < good_edges->size(); i++)
        {
            assignments.push_back(assign_to_vanishing_point(MW_OUTLIER_THRESHOLD, &(good_edges->get_segment(i)), vpts));
        }
        Image img_with_segments(img);
        good_edges->randomly_color(img_with_segments);
        img_with_segments.write("010_segments.jpg");
        Image img_with_vpts(img);
        fm.get_manhattan_world().draw_lines_from_segments_midpoint_to_vp(img_with_vpts);
        img_with_vpts.write("010_vpts.jpg");


        delete good_edges;
        delete temp_edges;

    }
    catch(KJB_error e)
    {
        e.print(std::cout);
    }

    return 0;
}

