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
   |  Author:  Jinyan Guan
 * =========================================================================== */

#include <string>
#include <fstream>
#include <iostream>
#include <edge_cpp/line_segment_set.h>
#include <edge_cpp/collinear_segment_chain.h>
#include <edge_cpp/edge.h>

using namespace std;

int main(int argc, char** argv)
{
    if(argc != 6)
    {
        cout << "Usage: " << argv[0] << " input-image chain-out-image chain-segs-out-file"
                                        " edges-out-image edge-segs-out-image\n";
        return EXIT_SUCCESS;
    }

    const char* image_file = argv[1];
    const char* col_seg_file = argv[2];
    const char* col_seg_segs_file = argv[3];
    const char* edge_file = argv[4];
    const char* edge_seg_file = argv[5];

    using namespace kjb;
    Image img(image_file);

    Image col_seg_img(img.get_num_rows(), img.get_num_cols(), 255, 255, 255);
    Image edge_seg_img(img.get_num_rows(), img.get_num_cols(), 255, 255, 255);
    Image edge_img(img.get_num_rows(), img.get_num_cols(), 255, 255, 255);
    Image col_seg_segs_img(img.get_num_rows(), img.get_num_cols(), 255, 255, 255);

    Canny_edge_detector edge_detector(1.0, 0.01*255, 0.08*255, 10, true);
    kjb::Edge_set *edges = edge_detector.detect_edges(img);
    edges->remove_short_edges(5);
    edges->break_edges_at_corners(0.89, 3);
    edges->remove_short_edges(20);
    edges->randomly_color(edge_img);

    Edge_segment_set edge_set(edges, false);

    edge_set.randomly_color(edge_seg_img);
    const std::vector<Edge_segment>& edge_segments = edge_set.get_segments();

    double dist_threashold = 10.0;
    double angle_threashold = (10.0/180.0)*M_PI;
    std::vector<Collinear_segment_chain> collinear_set
        = find_collinear_segment_chains(edge_segments.begin(), edge_segments.end(),
                                        dist_threashold, angle_threashold);

    for(unsigned int i = 0; i < collinear_set.size(); i++)
    {
        const Collinear_segment_chain& col_s = collinear_set[i];
        col_s.randomly_color(col_seg_img, 1);

        for(size_t j = 0; j < col_s.get_segments().size(); j++)
        {
            col_s.get_segments()[j].randomly_color(col_seg_segs_img);
        }
    }

    //assert(edges->is_edge_set_consistent());

    edge_img.write(edge_file);
    edge_seg_img.write(edge_seg_file);
    col_seg_img.write(col_seg_file);
    col_seg_segs_img.write(col_seg_segs_file);

    delete edges;
    return EXIT_SUCCESS;
}

