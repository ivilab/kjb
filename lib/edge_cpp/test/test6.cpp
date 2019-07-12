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
#include "edge_cpp/features_manager.h"

using namespace std;

int main()
{
    using namespace kjb;

    Image img("001.jpg");
    Image img2 = img;
    Image img3 = img;
    Image imges1 = img;
    Image imges2 = img;


    Canny_edge_detector edge_detector(1.0, 0.01*255, 0.008*255, 30, true);
    Edge_set_ptr ptr = edge_detector(img);
    ptr->remove_short_edges(25);
    ptr->break_edges_at_corners(0.85, 7);
    ptr->break_edges_at_corners(0.97, 60);
    ptr->remove_short_edges(25);

    Edge_segment_set edge_set(&(*ptr));
    edge_set.write("edge_segments.txt");

    ptr->draw(img, 255, 0 , 0);

    img.write("imageio.jpg");

    ptr->write("edges.txt");

    Edge_set_ptr ptr2(new Edge_set());
    ptr2->read("edges.txt");
    ptr2->draw(img2, 255, 0, 0);
    img2.write("imageio2.jpg");


    /*Features_manager fm("features.txt");
    fm.write("ofeatures.txt");

    Features_manager fm2;
    fm2.read("ofeatures.txt");

    fm2.edges.draw(img3, 255, 0 , 0);
    img3.write("imageio3.jpg");

    fm.edge_segments.draw(imges1, 255, 0, 0, 1.0);
    fm2.edge_segments.draw(imges2, 255, 0, 0, 1.0);
    imges1.write("imagesio.jpg");
    imges2.write("imagesio2.jpg");*/

    return 0;
}

