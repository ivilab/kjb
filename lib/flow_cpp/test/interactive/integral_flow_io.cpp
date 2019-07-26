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

/* $Id: integral_flow_io.cpp 14083 2013-03-12 18:41:07Z jguan1 $ */

#include <flow_cpp/flow_integral_flow.h>
#include <m_cpp/m_vector.h>
#include <l_cpp/l_test.h>

#include <string> 
#include <ostream>
#include <fstream>

using namespace kjb;
using namespace std;

int main(int argc, char** argv)
{
    kjb_c::kjb_init();

    string x_file = "output/x_int_flow.txt";
    string y_file = "output/y_int_flow.txt";

    size_t num_cols = 1000;
    size_t num_rows = 200;
    Matrix x_flows(num_rows, num_cols, 1.22334);
    Matrix y_flows(num_rows, num_cols, 2.11221);

    Integral_flow x_iflow(x_flows);
    Integral_flow y_iflow(y_flows);

    x_iflow.write("output/x_int_flow.txt");
    y_iflow.write("output/y_int_flow.txt");


    Integral_flow xx_iflow(x_file);
    Integral_flow yy_iflow(y_file);

    // compare the values
    for(size_t i = 0; i < num_cols; i++)
    {
        for(size_t j = 0; j < num_rows; j++)
        {
            TEST_TRUE(fabs(xx_iflow.flow_sum(i, j) - 
                    x_iflow.flow_sum(i,j)) < FLT_EPSILON);
            TEST_TRUE(fabs(yy_iflow.flow_sum(i, j) - 
                    y_iflow.flow_sum(i,j)) < FLT_EPSILON);
        }
    }
   
    RETURN_VICTORIOUSLY();
}

