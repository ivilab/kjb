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

#include "edge_cpp/vanishing_point_detector.h"
#include "sample/sample_misc.h"

using namespace std;

int main()
{
    using namespace kjb;

    Image img("001.jpg");
    Image img2 = img;

    Canny_edge_detector edge_detector(1.0, 0.01*255, 0.008*255, 30, true);
    Edge_set_ptr ptr = edge_detector(img);
    ptr->remove_short_edges(25);
    ptr->break_edges_at_corners(0.85, 7);
    ptr->break_edges_at_corners(0.97, 60);
    ptr->remove_short_edges(25);
    ptr->randomly_color(img2);
    img2.write("001edges.jpg");

    Edge_segment_set edge_set(&(*ptr));
    Vanishing_point_detector vpd(edge_set, 100, 100);

    img.write("001vertical.jpg");



    return 0;
}

