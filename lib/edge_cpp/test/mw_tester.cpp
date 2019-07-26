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
   |  Author:  Luca Del Pero
 * =========================================================================== */


#include <edge_cpp/features_manager.h>
#include <iostream>


using namespace kjb;
int main(int argc, char **argv)
{
    if(argc != 2)
    {
        std::cout << "Usage: ./mw_tester <path_to_image>" << std::endl;
        return 1;
    }

    Image img(argv[1]);
    Features_manager fm(img);

    const Manhattan_world & mw = fm.get_manhattan_world();
    const std::vector<Vanishing_point> & vpts = mw.get_vanishing_points();
    double focal_length = mw.get_focal_length();
    const std::vector < std::vector<Manhattan_segment> > & assignments = mw.get_assignments();

    /**
     * Assignments is a vector of size 4
     * assignments[0] -> a vector of all segments going to the first vanishing point
     * assignments[1] -> a vector of all segments going to the second vanishing point
     * assignments[2] -> a vector of all segments going to the third vanishing point
     * assignments[3] -> a vector of all segments going to NO vanishing point
     */
     fm.write("fm.txt");
     Features_manager fm2("fm.txt");

    return 1;
}
