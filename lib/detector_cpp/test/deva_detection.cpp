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

/* $Id: deva_detection.cpp 21293 2017-03-07 02:32:04Z jguan1 $ */

#include <detector_cpp/d_deva_detection.h>
#include <l_cpp/l_test.h>
#include <iostream>
#include <string>
#include <fstream>
#include <vector>

using namespace kjb;
using namespace std;

int main(int argc, char** argv)
{
    string fpath("input/deva_detection.txt");
    ifstream ifs(fpath.c_str());
    if(ifs.fail())
    {
        cout << "Test failed! "
             << "(Cannot read file '" << fpath << "')"
             << endl; 
        return EXIT_FAILURE;
    }

    vector<Deva_detection> ddv = parse_deva_detection(ifs);
    TEST_TRUE(ddv.size() == 10);

    // first box (276.69 1 349.2 202.17)
    const Deva_detection& dv = ddv[0];
    TEST_TRUE(fabs(dv.full_body().get_left() - 276.69) < FLT_EPSILON); 
    TEST_TRUE(fabs(dv.full_body().get_top() - 202.17) < FLT_EPSILON);
    TEST_TRUE(fabs(dv.full_body().get_right() - 349.2) < FLT_EPSILON);
    TEST_TRUE(fabs(dv.full_body().get_bottom() - 1) < FLT_EPSILON);

    ifstream ifs2(fpath.c_str());
    if(ifs2.fail())
    {
        cout << "Test failed! "
             << "(Cannot read file '" << fpath << "')"
             << endl; 
        return EXIT_FAILURE;
    }
    vector<Deva_detection> ddv_2 = parse_deva_detection(ifs2, -1.5);
    TEST_TRUE(ddv_2.size() == 8); 

    RETURN_VICTORIOUSLY();
}
