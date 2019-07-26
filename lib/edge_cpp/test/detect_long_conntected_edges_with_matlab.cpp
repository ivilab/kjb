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
#include "edge_cpp/line_segment_set.h"
#include "l/l_sys_rand.h"

using namespace std;

int main(int argc, char ** argv)
{
    using namespace kjb;

    if(argc != 2)
    {
        std::cout << "Usage: <path_to_image>" << std::endl;
        return 1;
    }

    try
    {
        Line_segment_set segments;
        detect_long_connected_segments(segments, argv[1], 30);
    }
    catch(KJB_error e)
    {
        std::cout << "It did not work" << std::endl;
        e.print(std::cout);
        return 1;
    }
    catch(...)
    {
        std::cout << "It did not work" << std::endl;
        return 1;
    }
    std::cout << "It worked!" << std::endl;


    return 0;
}

