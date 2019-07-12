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

void test_ints();

int main()
{
    using namespace kjb;

    Line_segment lsa;
    lsa.init_from_end_points(0.0, 0.0, 1.0, 0.0);

    kjb::Vector point(2, 0.0);

    double dist_point = lsa.get_distance_from_point(point);
    if(fabs(dist_point) > FLT_EPSILON)
    {
        throw KJB_error("Line segment-point distance, different value expected!");
    }
    double dist_line = lsa.get_line().find_distance_to_point(point);
    if(fabs(dist_line) > FLT_EPSILON)
    {
        throw KJB_error("Line-point distance, different value expected!");
    }

    point(0) = 1.0;
    dist_point = lsa.get_distance_from_point(point);
    if(fabs(dist_point) > FLT_EPSILON)
    {
        throw KJB_error("Line segment-point distance, different value expected!");
    }
    dist_line = lsa.get_line().find_distance_to_point(point);
    if(fabs(dist_line) > FLT_EPSILON)
    {
        throw KJB_error("Line-point distance, different value expected!");
    }


    point(0) = 0.5;
    dist_point = lsa.get_distance_from_point(point);
    if(fabs(dist_point) > FLT_EPSILON)
    {
        throw KJB_error("Line segment-point distance, different value expected!");
    }
    dist_line = lsa.get_line().find_distance_to_point(point);
    if(fabs(dist_line) > FLT_EPSILON)
    {
        throw KJB_error("Line-point distance, different value expected!");
    }

    point(0) = 10.0;
    dist_point = lsa.get_distance_from_point(point);
    if(fabs(dist_point - 9.0) > FLT_EPSILON)
    {
        throw KJB_error("Line segment-point distance, different value expected!");
    }
    dist_line = lsa.get_line().find_distance_to_point(point);
    if(fabs(dist_line) > FLT_EPSILON)
    {
        throw KJB_error("Line-point distance, different value expected!");
    }

    point(0) = -10.0;
    dist_point = lsa.get_distance_from_point(point);
    if(fabs(dist_point - 10.0) > FLT_EPSILON)
    {
        throw KJB_error("Line segment-point distance, different value expected!");
    }
    dist_line = lsa.get_line().find_distance_to_point(point);
    if(fabs(dist_line) > FLT_EPSILON)
    {
        throw KJB_error("Line-point distance, different value expected!");
    }

    point(0) = 0.0;
    point(1) = 2.0;
    dist_point = lsa.get_distance_from_point(point);
    if(fabs(dist_point - 2.0) > FLT_EPSILON)
    {
       throw KJB_error("Line segment-point distance, different value expected!");
    }
    dist_line = lsa.get_line().find_distance_to_point(point);
    if(fabs(dist_line - 2.0) > FLT_EPSILON)
    {
        throw KJB_error("Line-point distance, different value expected!");
    }

    point(0) = -3.0;
    point(1) = -4.0;
    dist_point = lsa.get_distance_from_point(point);
    if(fabs(dist_point - 5.0) > FLT_EPSILON)
    {
        throw KJB_error("Line segment-point distance, different value expected!");
    }
    dist_line = lsa.get_line().find_distance_to_point(point);
    if(fabs(dist_line - 4.0) > FLT_EPSILON)
    {
        throw KJB_error("Line-point distance, different value expected!");
    }

    point(0) = 13.0;
    point(1) = -5.0;
    dist_point = lsa.get_distance_from_point(point);
    if(fabs(dist_point - 13.0) > FLT_EPSILON)
    {
        throw KJB_error("Line segment-point distance, different value expected!");
    }
    dist_line = lsa.get_line().find_distance_to_point(point);
    if(fabs(dist_line - 5.0) > FLT_EPSILON)
    {
        throw KJB_error("Line-point distance, different value expected!");
    }

    Line_segment lsb;
    lsb.init_from_end_points(0.0, 0.0, 0.0, 1.0);

    point(0) = 0.0;
    point(1) = 0.0;

    dist_point = lsb.get_distance_from_point(point);
    if(fabs(dist_point) > FLT_EPSILON)
    {
        throw KJB_error("Line segment-point distance, different value expected!");
    }
    dist_line = lsb.get_line().find_distance_to_point(point);
    if(fabs(dist_line) > FLT_EPSILON)
    {
        throw KJB_error("Line-point distance, different value expected!");
    }

    point(1) = 1.0;
    dist_point = lsb.get_distance_from_point(point);
    if(fabs(dist_point) > FLT_EPSILON)
    {
        throw KJB_error("Line segment-point distance, different value expected!");
    }
    dist_line = lsb.get_line().find_distance_to_point(point);
    if(fabs(dist_line) > FLT_EPSILON)
    {
        throw KJB_error("Line-point distance, different value expected!");
    }

    point(1) = 0.5;
    dist_point = lsb.get_distance_from_point(point);
    if(fabs(dist_point) > FLT_EPSILON)
    {
        throw KJB_error("Line segment-point distance, different value expected!");
    }
    dist_line = lsb.get_line().find_distance_to_point(point);
    if(fabs(dist_line) > FLT_EPSILON)
    {
        throw KJB_error("Line-point distance, different value expected!");
    }

    point(1) = 10.0;
    dist_point = lsb.get_distance_from_point(point);
    if(fabs(dist_point - 9.0) > FLT_EPSILON)
    {
        throw KJB_error("Line segment-point distance, different value expected!");
    }
    dist_line = lsb.get_line().find_distance_to_point(point);
    if(fabs(dist_line) > FLT_EPSILON)
    {
        throw KJB_error("Line-point distance, different value expected!");
    }

    point(1) = -10.0;
    dist_point = lsb.get_distance_from_point(point);
    if(fabs(dist_point - 10.0) > FLT_EPSILON)
    {
        throw KJB_error("Line segment-point distance, different value expected!");
    }
    dist_line = lsb.get_line().find_distance_to_point(point);
    if(fabs(dist_line) > FLT_EPSILON)
    {
        throw KJB_error("Line-point distance, different value expected!");
    }

    point(1) = -3.0;
    point(0) = -4.0;
    dist_point = lsb.get_distance_from_point(point);
    if(fabs(dist_point - 5.0) > FLT_EPSILON)
    {
        throw KJB_error("Line segment-point distance, different value expected!");
    }
    dist_line = lsb.get_line().find_distance_to_point(point);
    if(fabs(dist_line - 4.0) > FLT_EPSILON)
    {
        throw KJB_error("Line-point distance, different value expected!");
    }

    point(1) = 13.0;
    point(0) = -5.0;
    dist_point = lsb.get_distance_from_point(point);
    if(fabs(dist_point - 13.0) > FLT_EPSILON)
    {
        throw KJB_error("Line segment-point distance, different value expected!");
    }
    dist_line = lsb.get_line().find_distance_to_point(point);
    if(fabs(dist_line - 5.0) > FLT_EPSILON)
    {
        throw KJB_error("Line-point distance, different value expected!");
    }

    Line_segment lsc;
    lsc.init_from_end_points(1.0, 1.0, 2.0, 2.0);

    point(0) = 1.5;
    point(1) = 1.5;
    dist_point = lsc.get_distance_from_point(point);
    dist_line = lsc.get_line().find_distance_to_point(point);
    if(fabs(dist_point) > FLT_EPSILON)
    {
       throw KJB_error("Line segment-point distance, different value expected!");
    }
    if(fabs(dist_line) > FLT_EPSILON)
    {
        throw KJB_error("Line-point distance, different value expected!");
    }

    point(0) = 3.0;
    point(1) = 3.0;
    dist_point = lsc.get_distance_from_point(point);
    dist_line = lsc.get_line().find_distance_to_point(point);
    if(fabs(dist_point - sqrt(2)) > FLT_EPSILON)
    {
        throw KJB_error("Line segment-point distance, different value expected!");
    }
    if(fabs(dist_line) > FLT_EPSILON)
    {
        throw KJB_error("Line-point distance, different value expected!");
    }

    point(0) = 0.0;
    point(1) = 2.0;
    dist_point = lsc.get_distance_from_point(point);
    dist_line = lsc.get_line().find_distance_to_point(point);
    if(fabs(dist_point - sqrt(2)) > FLT_EPSILON)
    {
        throw KJB_error("Line segment-point distance, different value expected!");
    }
    if(fabs(dist_line - sqrt(2)) > FLT_EPSILON)
    {
        throw KJB_error("Line-point distance, different value expected!");
    }

    point(0) = 2.0;
    point(1) = 0.0;
    dist_point = lsc.get_distance_from_point(point);
    dist_line = lsc.get_line().find_distance_to_point(point);
    if(fabs(dist_point - sqrt(2)) > FLT_EPSILON)
    {
        throw KJB_error("Line segment-point distance, different value expected!");
    }
    if(fabs(dist_line - sqrt(2)) > FLT_EPSILON)
    {
       throw KJB_error("Line-point distance, different value expected!");
    }

    point(0) = -3.0;
    point(1) = 4.0;
    dist_point = lsc.get_distance_from_point(point);
    dist_line = lsc.get_line().find_distance_to_point(point);
    if(fabs(dist_point - 5) > FLT_EPSILON)
    {
        throw KJB_error("Line segment-point distance, different value expected!");
    }
    if(fabs(dist_line - 4.9497474683058327) > FLT_EPSILON)
    {
       throw KJB_error("Line-point distance, different value expected!");
    }

    point(0) = -3.0;
    point(1) = 5.0;
    dist_point = lsc.get_distance_from_point(point);
    dist_line = lsc.get_line().find_distance_to_point(point);
    if(fabs(dist_point - sqrt(32.0)) > FLT_EPSILON)
    {
        throw KJB_error("Line segment-point distance, different value expected!");
    }
    if(fabs(dist_line - sqrt(32)) > FLT_EPSILON)
    {
        throw KJB_error("Line-point distance, different value expected!");
    }

    test_ints();

    std::cout << "Success" << std::endl;
    return 0;
}

void test_ints()
{
    using namespace kjb;
    Line_segment lsa;
    lsa.init_from_end_points(0.0, 0.0, 1.0, 0.0);

    kjb::Vector point(2, 0.0);

    double dist_line = 0.0;
    double dist_point = lsa.get_distance_from_point(point, &dist_line);
    if(fabs(dist_point) > FLT_EPSILON)
    {
        throw KJB_error("Line segment-point distance, different value expected!");
    }
    if(fabs(dist_line) > FLT_EPSILON)
    {
        throw KJB_error("Line-point distance, different value expected!");
    }

    point(0) = 1.0;
    dist_point = lsa.get_distance_from_point(point, &dist_line);
    if(fabs(dist_point) > FLT_EPSILON)
    {
        throw KJB_error("Line segment-point distance, different value expected!");
    }
    if(fabs(dist_line) > FLT_EPSILON)
    {
        throw KJB_error("Line-point distance, different value expected!");
    }


    point(0) = 0.5;
    dist_point = lsa.get_distance_from_point(point, &dist_line);
    if(fabs(dist_point) > FLT_EPSILON)
    {
        throw KJB_error("Line segment-point distance, different value expected!");
    }
    if(fabs(dist_line) > FLT_EPSILON)
    {
        throw KJB_error("Line-point distance, different value expected!");
    }

    point(0) = 10.0;
    dist_point = lsa.get_distance_from_point(point, &dist_line);
    if(fabs(dist_point - 9.0) > FLT_EPSILON)
    {
        throw KJB_error("Line segment-point distance, different value expected!");
    }
    if(fabs(dist_line) > FLT_EPSILON)
    {
        throw KJB_error("Line-point distance, different value expected!");
    }

    point(0) = -10.0;
    dist_point = lsa.get_distance_from_point(point, &dist_line);
    if(fabs(dist_point - 10.0) > FLT_EPSILON)
    {
        throw KJB_error("Line segment-point distance, different value expected!");
    }
    if(fabs(dist_line) > FLT_EPSILON)
    {
        throw KJB_error("Line-point distance, different value expected!");
    }

    point(0) = 0.0;
    point(1) = 2.0;
    dist_point = lsa.get_distance_from_point(point, &dist_line);
    if(fabs(dist_point - 2.0) > FLT_EPSILON)
    {
       throw KJB_error("Line segment-point distance, different value expected!");
    }
    if(fabs(dist_line - 2.0) > FLT_EPSILON)
    {
        throw KJB_error("Line-point distance, different value expected!");
    }

    point(0) = -3.0;
    point(1) = -4.0;
    dist_point = lsa.get_distance_from_point(point, &dist_line);
    if(fabs(dist_point - 5.0) > FLT_EPSILON)
    {
        throw KJB_error("Line segment-point distance, different value expected!");
    }
    if(fabs(dist_line - 4.0) > FLT_EPSILON)
    {
        throw KJB_error("Line-point distance, different value expected!");
    }

    point(0) = 13.0;
    point(1) = -5.0;
    dist_point = lsa.get_distance_from_point(point, &dist_line);
    if(fabs(dist_point - 13.0) > FLT_EPSILON)
    {
        throw KJB_error("Line segment-point distance, different value expected!");
    }
    if(fabs(dist_line - 5.0) > FLT_EPSILON)
    {
        throw KJB_error("Line-point distance, different value expected!");
    }

    Line_segment lsb;
    lsb.init_from_end_points(0.0, 0.0, 0.0, 1.0);

    point(0) = 0.0;
    point(1) = 0.0;

    dist_point = lsb.get_distance_from_point(point, &dist_line);
    if(fabs(dist_point) > FLT_EPSILON)
    {
        throw KJB_error("Line segment-point distance, different value expected!");
    }
    if(fabs(dist_line) > FLT_EPSILON)
    {
        throw KJB_error("Line-point distance, different value expected!");
    }

    point(1) = 1.0;
    dist_point = lsb.get_distance_from_point(point, &dist_line);
    if(fabs(dist_point) > FLT_EPSILON)
    {
        throw KJB_error("Line segment-point distance, different value expected!");
    }
    if(fabs(dist_line) > FLT_EPSILON)
    {
        throw KJB_error("Line-point distance, different value expected!");
    }

    point(1) = 0.5;
    dist_point = lsb.get_distance_from_point(point, &dist_line);
    if(fabs(dist_point) > FLT_EPSILON)
    {
        throw KJB_error("Line segment-point distance, different value expected!");
    }
    if(fabs(dist_line) > FLT_EPSILON)
    {
        throw KJB_error("Line-point distance, different value expected!");
    }

    point(1) = 10.0;
    dist_point = lsb.get_distance_from_point(point, &dist_line);
    if(fabs(dist_point - 9.0) > FLT_EPSILON)
    {
        throw KJB_error("Line segment-point distance, different value expected!");
    }
    if(fabs(dist_line) > FLT_EPSILON)
    {
        throw KJB_error("Line-point distance, different value expected!");
    }

    point(1) = -10.0;
    dist_point = lsb.get_distance_from_point(point, &dist_line);
    if(fabs(dist_point - 10.0) > FLT_EPSILON)
    {
        throw KJB_error("Line segment-point distance, different value expected!");
    }
    if(fabs(dist_line) > FLT_EPSILON)
    {
        throw KJB_error("Line-point distance, different value expected!");
    }

    point(1) = -3.0;
    point(0) = -4.0;
    dist_point = lsb.get_distance_from_point(point, &dist_line);
    if(fabs(dist_point - 5.0) > FLT_EPSILON)
    {
        throw KJB_error("Line segment-point distance, different value expected!");
    }
    if(fabs(dist_line - 4.0) > FLT_EPSILON)
    {
        throw KJB_error("Line-point distance, different value expected!");
    }

    point(1) = 13.0;
    point(0) = -5.0;
    dist_point = lsb.get_distance_from_point(point, &dist_line);
    if(fabs(dist_point - 13.0) > FLT_EPSILON)
    {
        throw KJB_error("Line segment-point distance, different value expected!");
    }
    if(fabs(dist_line - 5.0) > FLT_EPSILON)
    {
        throw KJB_error("Line-point distance, different value expected!");
    }

    Line_segment lsc;
    lsc.init_from_end_points(1.0, 1.0, 2.0, 2.0);

    point(0) = 1.5;
    point(1) = 1.5;
    dist_point = lsc.get_distance_from_point(point, &dist_line);
    if(fabs(dist_point) > FLT_EPSILON)
    {
       throw KJB_error("Line segment-point distance, different value expected!");
    }
    if(fabs(dist_line) > FLT_EPSILON)
    {
        throw KJB_error("Line-point distance, different value expected!");
    }

    point(0) = 3.0;
    point(1) = 3.0;
    dist_point = lsc.get_distance_from_point(point, &dist_line);
    if(fabs(dist_point - sqrt(2)) > FLT_EPSILON)
    {
        throw KJB_error("Line segment-point distance, different value expected!");
    }
    if(fabs(dist_line) > FLT_EPSILON)
    {
        throw KJB_error("Line-point distance, different value expected!");
    }

    point(0) = 0.0;
    point(1) = 2.0;
    dist_point = lsc.get_distance_from_point(point, &dist_line);
    if(fabs(dist_point - sqrt(2)) > FLT_EPSILON)
    {
        throw KJB_error("Line segment-point distance, different value expected!");
    }
    if(fabs(dist_line - sqrt(2)) > FLT_EPSILON)
    {
        throw KJB_error("Line-point distance, different value expected!");
    }

    point(0) = 2.0;
    point(1) = 0.0;
    dist_point = lsc.get_distance_from_point(point, &dist_line);
    if(fabs(dist_point - sqrt(2)) > FLT_EPSILON)
    {
        throw KJB_error("Line segment-point distance, different value expected!");
    }
    if(fabs(dist_line - sqrt(2)) > FLT_EPSILON)
    {
       throw KJB_error("Line-point distance, different value expected!");
    }

    point(0) = -3.0;
    point(1) = 4.0;
    dist_point = lsc.get_distance_from_point(point, &dist_line);
    if(fabs(dist_point - 5) > FLT_EPSILON)
    {
        throw KJB_error("Line segment-point distance, different value expected!");
    }
    if(fabs(dist_line - 4.9497474683058327) > FLT_EPSILON)
    {
       throw KJB_error("Line-point distance, different value expected!");
    }

    point(0) = -3.0;
    point(1) = 5.0;
    dist_point = lsc.get_distance_from_point(point, &dist_line);
    if(fabs(dist_point - sqrt(32.0)) > FLT_EPSILON)
    {
        throw KJB_error("Line segment-point distance, different value expected!");
    }
    if(fabs(dist_line - sqrt(32)) > FLT_EPSILON)
    {
        throw KJB_error("Line-point distance, different value expected!");
    }

    Line_segment lsd;
    lsd.init_from_end_points(0.0, 0.0, 2.0, 0.0);
    point(0) = 1.5;
    point(1) = 1.0;
    dist_point = lsd.get_distance_from_point(point, &dist_line);
    if(fabs(dist_point - 1.0) > FLT_EPSILON)
    {
        throw KJB_error("Line segment-point distance, different value expected!");
    }
    if(fabs(dist_line - 1.0) > FLT_EPSILON)
    {
       throw KJB_error("Line-point distance, different value expected!");
    }
}
