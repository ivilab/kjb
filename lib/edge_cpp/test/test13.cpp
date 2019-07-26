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
   |  Author: Jinyan Guan 
 * =========================================================================== */

#include <string>

#include "g_cpp/g_line.h"
#include "edge_cpp/line_segment.h"

#include <utility> 
#include <iostream> 

using namespace kjb;
using namespace std;
int main(int argc, const char** argv)
{
    Line line(-1, 1, 0);
    const double p1[] = {0, 2, 1};
    const double p2[] = {2, 0, 1}; 
    Vector p1_v(3, p1);
    Vector p2_v(3, p2);

    Vector p_p1 = line.project_point_onto_line(p1_v);
    cout<<"p_p1: "<<p_p1<<endl;

    Vector p_p2 = line.project_point_onto_line(p2_v);
    cout<<"p_p2: "<<p_p2<<endl;
    
    Line_segment segment(p1_v, p2_v);
    pair<Vector, Vector> pts= Line_segment::project_line_segment_onto_line(segment, line);
    
    cout<<"first: "<<pts.first <<endl;
    cout<<"second: "<<pts.second <<endl;

    const double p3[] = {8.59449548e+01, 2.39208101e+02, 1};
    const double p4[] = {3.31055045e+02, 2.42377265e+02, 1};
    Vector p3_v(3, p3);
    Vector p4_v(3, p4);

    Line_segment image_line(p3_v, p4_v);
    Line_segment model_edge;
    model_edge.init_from_end_points(1.75000000e+02, 2.45000000e+02,  3.30000000e+02,  2.37000000e+02);
    std::pair<Vector, Vector> projected_points;
    double length_inside = 0;
    double length_outside = 0;

    Line_segment::project_line_segment_onto_line_segment(image_line, model_edge, projected_points, length_inside, length_outside);
    std::cout<<"Length_inside: "<<length_inside<<" Length_outside: "<<length_outside<<" projected: "<<projected_points.first<<"---"<<projected_points.second<<std::endl;
}
