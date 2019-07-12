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
   |  Author:  Ernesto Brau
 * =========================================================================== */

/* $Id: detection_box.cpp 12676 2012-07-16 00:53:03Z ernesto $ */

#include <people_tracking_cpp/pt_detection_box.h>
#include <l_cpp/l_test.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

using namespace kjb;
using namespace pt;
using namespace std;

int main(int argc, char** argv)
{
    // create file stream
    ifstream box_fs("input/detection_box_cpp.txt");
    if(!box_fs)
    {
        cout << "Test failed! "
             << "(Cannot read input file 'input/detection_box_cpp.txt')"
             << endl;
        return EXIT_FAILURE;
    }

    // read boxes from file
    vector<Detection_box> boxes;
    string box_line;
    while(getline(box_fs, box_line))
    {
        boxes.push_back(parse_detection_box(box_line));
    }

    // test num boxes
    TEST_TRUE(boxes.size() == 3);

    // test box equality
    TEST_FALSE(boxes[0] < boxes[0]);

    // test box less-than
    TEST_FALSE(boxes[0] < boxes[1] || boxes[0] < boxes[2]);

    // test box write and read
    const Detection_box& box1 = boxes[0];
    stringstream sstrm;
    sstrm << box1;

    Detection_box box2 = parse_detection_box(sstrm.str());
    TEST_FALSE(box1 < box2 || box2 < box1);

    RETURN_VICTORIOUSLY();
}

