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

#include <edge_cpp/line_segment_set.h>

using namespace std;

int main()
{
    using namespace kjb;

    Image img("001.jpg");

    Image img2(img);

    Canny_edge_detector edge_detector(1.0, 0.01*255, 0.008*255, 10, true);
    kjb::Edge_set *edges = edge_detector.detect_edges(img);
    edges->remove_short_edges(15);
    edges->break_edges_at_corners(0.89, 6);
    edges->remove_short_edges(30);
    Edge_segment_set edge_set(edges);

    edges->randomly_color(img);
    edge_set.randomly_color(img2);
    assert( edges->is_edge_set_consistenct() );

    img.write("001edges.jpg");
    img2.write("001segments.jpg");

    delete edges;


    return 0;
}

