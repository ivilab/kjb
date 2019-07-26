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

    unsigned int n = 2;
    unsigned int m = 30;

    kjb_c::kjb_seed_rand_with_tod();

    Int_vector iv(7);
    Vanishing_point_detector::sample_n_from_m_without_repetitions(n,m,iv);

    /*for(unsigned int i = 0; i < iv.size(); i++)
    {
        std::cout << iv(i) << " || ";
    }
    std::cout << std::endl;*/

    Line_segment lsa;
    lsa.init_from_end_points(0.0, 0.0, 1.0, 0.0);
    Line_segment lsb;
    lsb.init_from_end_points(0.0, 1.0, 0, 2.0);

    Vector ints;
    bool res = Line::find_line_intersection(lsa.get_line(), lsb.get_line(), ints);
    Vector real_ints(2,0.0);
    if( ! (ints == real_ints) )
    {
        throw KJB_error("Line intersection, Different point expected!");
    }
    if(!res)
    {
        throw KJB_error("Line intersection, Different result expected!");
    }


    lsa.init_from_end_points(0.0, 0.0, 1.0, 1.0);
    lsb.init_from_end_points(0.0, 2.0, 2.0, 0.0);
    real_ints(0) = 1.0;
    real_ints(1) = 1.0;
    res = Line::find_line_intersection(lsa.get_line(), lsb.get_line(), ints);
    if( ! (ints == real_ints) )
    {
        throw KJB_error("Line intersection, Different point expected!");
    }
    if(!res)
    {
        throw KJB_error("Line intersection, Different result expected!");
    }

    lsa.init_from_end_points(0.0, 0.0, 1.0, 1.0);
    lsb.init_from_end_points(1.0, 1.0, 2.0, 2.0);
    res = Line::find_line_intersection(lsa.get_line(), lsb.get_line(), ints);
    if(res)
    {
        throw KJB_error("Line intersection, Different result expected!");
    }

    return 0;
}

