/* =========================================================================== *
   |
   |  Copyright (c) 1994-2011 by Kobus Barnard (author)
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

/* $Id: flow_dense.cpp 18278 2014-11-25 01:42:10Z ksimek $ */

#include <flow_cpp/flow_dense.h>
#include <l_cpp/l_exception.h>
#include <cmath>

#include <algorithm>

using namespace kjb;

Vector kjb::average_flow
(
    const Matrix& x_flows, 
    const Matrix& y_flows,
    const Axis_aligned_rectangle_2d& roi
)
{
    IFT((x_flows.get_num_cols() == y_flows.get_num_cols()
        && x_flows.get_num_rows() == y_flows.get_num_rows()),
        Dimension_mismatch, 
        "Dimensions of x flows and y flows not match\n");

    double average_x = 0.0;
    double average_y = 0.0;
    const Vector& center = roi.get_center();
    int num_cols = x_flows.get_num_cols();
    int num_rows = x_flows.get_num_rows();

    double width = roi.get_width();
    double height = roi.get_height();

    size_t min_x = std::max(0, (int)std::floor(center[0] - width/2.0));
    size_t max_x = std::min(num_cols-1, (int)std::ceil(center[0] + width/2.0));
    size_t min_y = std::max(0, (int)std::floor(center[1] - height/2.0));
    size_t max_y = std::min(num_rows-1, (int)std::ceil(center[1] + height/2.0));

    size_t num_flow = 0; 
    for(size_t row = min_y; row < max_y; row++)
    {
        for(size_t col = min_x; col < max_x; col++)
        {
            average_x += x_flows(row, col);
            average_y += y_flows(row, col);
            num_flow++; 
        }
    }
    if(num_flow > 0)
    {
        average_x /= num_flow;
        average_y /= num_flow;
    }

    return Vector(average_x, average_y); 
} 

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

Matrix kjb::flow_magnitude(const Matrix& x_flows, const Matrix& y_flows)
{

    IFT((x_flows.get_num_cols() == y_flows.get_num_cols()
        && x_flows.get_num_rows() == y_flows.get_num_rows()),
        Dimension_mismatch, 
        "Dimensions of x flows and y flows not match\n");
    size_t num_cols = x_flows.get_num_cols();
    size_t num_rows = x_flows.get_num_rows();

    Matrix mag(num_rows, num_cols, 0.0);
    for(size_t row = 0; row < num_rows; row++)
    {
        for(size_t col = 0; col < num_cols; col++)
        {
            mag(row, col) = sqrt(x_flows(row, col)*x_flows(row, col) +
                                 y_flows(row, col)*y_flows(row, col));
        }
    }

    return mag; 
}
