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

#include <edge_cpp/line_segment.h>

using namespace std;



int main()
{
    using namespace kjb;

    int dir;
    double delta;
    bool result = Line_segment::collision_detection
    (
        0.5,
        1.0,
        0.1,
        0.2,
        dir,
        delta
    );

    assert(!result);

    result = Line_segment::collision_detection
    (
        0.5,
        1.0,
        0.1,
        0.6,
        dir,
        delta
    );

    assert(result);
    assert( fabs(delta - 0.2) < FLT_EPSILON);
    assert(dir == -1);

    result = Line_segment::collision_detection
    (
        0.5,
        1.0,
        0.1,
        0.6,
        dir,
        delta
    );

    assert(result);
    assert( fabs(delta - 0.2) < FLT_EPSILON);
    assert(dir == -1);

    result = Line_segment::collision_detection
    (
        0.5,
        1.0,
        1.1,
        1.6,
        dir,
        delta
    );

    assert(!result);

    result = Line_segment::collision_detection
    (
        0.5,
        1.0,
        0.1,
        1.6,
        dir,
        delta
    );

    assert(result);
    assert( fabs(delta - (0.9/1.5)) < FLT_EPSILON);
    assert(dir == 1);

    result = Line_segment::collision_detection
    (
        0.5,
        1.0,
        0.1,
        1.2,
        dir,
        delta
    );

    assert(result);
    assert( fabs(delta - (0.7/1.1)) < FLT_EPSILON);
    assert(dir == -1);

    result = Line_segment::collision_detection
    (
        0.5,
        1.0,
        0.6,
        0.8,
        dir,
        delta
    );

    assert(result);
    assert( fabs(delta - 0.0) < FLT_EPSILON);

    result = Line_segment::collision_detection
    (
        0.5,
        1.0,
        0.9,
        1.9,
        dir,
        delta
    );

    assert(result);
    assert( fabs(delta - 0.1) < FLT_EPSILON);
    assert(dir == 1);

    result = Line_segment::collision_detection
    (
        -1.0,
        -0.5,
        -1.9,
        -0.9,
        dir,
        delta
    );

    assert(result);
    assert( fabs(delta - 0.1) < FLT_EPSILON);
    assert(dir == -1);

    result = Line_segment::collision_detection
    (
        -1.0,
         0.5,
        -0.2,
        0.7,
        dir,
        delta
    );

    assert(result);
    assert( fabs(delta - (0.7/0.9)) < FLT_EPSILON);
    assert(dir == 1);

    return 0;
}

