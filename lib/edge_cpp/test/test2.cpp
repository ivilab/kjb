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

#define TEST_DEG_TO_RADIAN M_PI/180.0

int main()
{
    using namespace kjb;

    Line_segment ls(200.0, 200.0, 360*TEST_DEG_TO_RADIAN, 100.0 );

    if(!ls.is_line_segment_consistent())
    {
        std::cout << "BAD LINE SEGMENT" << std::endl;
        throw KJB_error("Line segment test failed!");
    }
    else
    {
        std::cout << "GOOD" << std::endl;
    }

    Line_segment ls_a;
    ls_a.init_from_end_points(ls.get_start_x(), ls.get_start_y(), ls.get_end_x(), ls.get_end_y());
    if(!ls_a.is_line_segment_consistent())
    {
        std::cout << "BAD LINE SEGMENT" << std::endl;
        throw KJB_error("Line segment test failed!");
    }
    else
    {
        std::cout << "GOOD" << std::endl;
    }


    if((ls == ls_a))
    {
        std::cout << "GOOD, they are equal" << std::endl;
    }
    else
    {
        std::cout << "BAD, they are different" << std::endl;
        throw KJB_error("Line segment test failed!");
    }

    Line_segment ls_b;
    ls_b.init_from_slope_and_intercept(ls.get_start_x(), ls.get_end_x(), ls.get_slope(), ls.get_y_intercept());
    if(!ls_b.is_line_segment_consistent())
    {
        std::cout << "BAD LINE SEGMENT" << std::endl;
        throw KJB_error("Line segment test failed!");
    }
    else
    {
        std::cout << "GOOD" << std::endl;
    }

    if((ls == ls_b))
    {
        std::cout << "GOOD, they are equal" << std::endl;
    }
    else
    {
        std::cout << "BAD, they are different" << std::endl;
        throw KJB_error("Line segment test failed!");
    }

    Line_segment ls2(200.0, 200.0, -(M_PI - 50*TEST_DEG_TO_RADIAN) - 2*M_PI, 100.0 );
    if(!ls2.is_line_segment_consistent())
    {
        std::cout << "BAD LINE SEGMENT" << std::endl;
        throw KJB_error("Line segment test failed!");
    }
    else
    {
        std::cout << "GOOD" << std::endl;
    }

    Line_segment ls2_a;
    ls2_a.init_from_end_points(ls2.get_start_x(), ls2.get_start_y(), ls2.get_end_x(), ls2.get_end_y());
    Line_segment ls2_b;
    ls2_b.init_from_slope_and_intercept(ls2.get_start_x(), ls2.get_end_x(), ls2.get_slope(), ls2.get_y_intercept());
    if(!ls2_a.is_line_segment_consistent())
    {
        std::cout << "BAD LINE SEGMENT" << std::endl;
        throw KJB_error("Line segment test failed!");
    }
    else
    {
        std::cout << "GOOD" << std::endl;
    }

    if((ls2 == ls2_a))
    {
        std::cout << "GOOD, they are equal" << std::endl;
    }
    else
    {
        std::cout << "BAD, they are different" << std::endl;
        throw KJB_error("Line segment test failed!");
    }
    if(!ls2_b.is_line_segment_consistent())
    {
        std::cout << "BAD LINE SEGMENT" << std::endl;
        throw KJB_error("Line segment test failed!");
    }
    else
    {
        std::cout << "GOOD" << std::endl;
    }

    if((ls2 == ls2_b))
    {
        std::cout << "GOOD, they are equal" << std::endl;
    }
    else
    {
        std::cout << "BAD, they are different" << std::endl;
        throw KJB_error("Line segment test failed!");
    }

    Line_segment ls3(200.0, 200.0, M_PI_2+DBL_EPSILON, 100.0 );

    if(!ls3.is_line_segment_consistent())
    {
        std::cout << "BAD LINE SEGMENT" << std::endl;
        throw KJB_error("Line segment test failed!");
    }
    else
    {
        std::cout << "GOOD" << std::endl;
    }

    Line_segment ls3_a;
    ls3_a.init_from_end_points(ls3.get_start_x(), ls3.get_start_y(), ls3.get_end_x(), ls3.get_end_y());
    Line_segment ls3_b;
    ls3_b.init_vertical_segment(ls3.get_start_y(), ls3.get_end_y(), ls3.get_start_x());
    if(!ls3_a.is_line_segment_consistent())
    {
        std::cout << "BAD LINE SEGMENT" << std::endl;
        throw KJB_error("Line segment test failed!");
    }
    else
    {
        std::cout << "GOOD" << std::endl;
    }

    if((ls3 == ls3_a))
    {
        std::cout << "GOOD, they are equal" << std::endl;
    }
    else
    {
        std::cout << "BAD, they are different" << std::endl;
        throw KJB_error("Line segment test failed!");
    }
    if(!ls3_b.is_line_segment_consistent())
    {
        std::cout << "BAD LINE SEGMENT" << std::endl;
        throw KJB_error("Line segment test failed!");
    }
    else
    {
        std::cout << "GOOD" << std::endl;
    }

    if((ls3 == ls3_b))
    {
        std::cout << "GOOD, they are equal" << std::endl;
    }
    else
    {
        std::cout << "BAD, they are different" << std::endl;
        throw KJB_error("Line segment test failed!");
    }

    Line_segment ls4(200.0, 200.0, 315*TEST_DEG_TO_RADIAN +2*M_PI, 100.0 );

    if(!ls4.is_line_segment_consistent())
    {
        std::cout << "BAD LINE SEGMENT" << std::endl;
        throw KJB_error("Line segment test failed!");
    }
    else
    {
        std::cout << "GOOD" << std::endl;
    }

    Line_segment ls4_a;
    ls4_a.init_from_end_points(ls4.get_start_x(), ls4.get_start_y(), ls4.get_end_x(), ls4.get_end_y());
    Line_segment ls4_b;
    ls4_b.init_from_slope_and_intercept(ls4.get_start_x(), ls4.get_end_x(), ls4.get_slope(), ls4.get_y_intercept());
    if(!ls4_a.is_line_segment_consistent())
    {
        std::cout << "BAD LINE SEGMENT" << std::endl;
        throw KJB_error("Line segment test failed!");
    }
    else
    {
        std::cout << "GOOD" << std::endl;
    }

    if((ls4 == ls4_a))
    {
        std::cout << "GOOD, they are equal" << std::endl;
    }
    else
    {
        std::cout << "BAD, they are different" << std::endl;
        throw KJB_error("Line segment test failed!");
    }
    if(!ls4_b.is_line_segment_consistent())
    {
        std::cout << "BAD LINE SEGMENT" << std::endl;
        throw KJB_error("Line segment test failed!");
    }
    else
    {
        std::cout << "GOOD" << std::endl;
    }

    if((ls4 == ls4_b))
    {
        std::cout << "GOOD, they are equal" << std::endl;
    }
    else
    {
        std::cout << "BAD, they are different" << std::endl;
        throw KJB_error("Line segment test failed!");
    }

    Line_segment ls5(200.0, 200.0, 180*TEST_DEG_TO_RADIAN, 100.0 );

    if(!ls.is_line_segment_consistent())
    {
        std::cout << "BAD LINE SEGMENT" << std::endl;
        throw KJB_error("Line segment test failed!");
    }
    else
    {
        std::cout << "GOOD" << std::endl;
    }

    Line_segment ls5_a;
    ls5_a.init_from_end_points(ls5.get_start_x(), ls5.get_start_y(), ls5.get_end_x(), ls5.get_end_y());
    Line_segment ls5_b;
    ls5_b.init_from_slope_and_intercept(ls5.get_start_x(), ls5.get_end_x(), ls5.get_slope(), ls5.get_y_intercept());
    if(!ls5_a.is_line_segment_consistent())
    {
        std::cout << "BAD LINE SEGMENT" << std::endl;
        throw KJB_error("Line segment test failed!");
    }
    else
    {
        std::cout << "GOOD" << std::endl;
    }

    if((ls5 == ls5_a))
    {
        std::cout << "GOOD, they are equal" << std::endl;
    }
    else
    {
        std::cout << "BAD, they are different" << std::endl;
        throw KJB_error("Line segment test failed!");
    }
    if(!ls5_b.is_line_segment_consistent())
    {
        std::cout << "BAD LINE SEGMENT" << std::endl;
        throw KJB_error("Line segment test failed!");
    }
    else
    {
        std::cout << "GOOD" << std::endl;
    }

    if((ls5 == ls5_b))
    {
        std::cout << "GOOD, they are equal" << std::endl;
    }
    else
    {
        std::cout << "BAD, they are different" << std::endl;
        throw KJB_error("Line segment test failed!");
    }

    Line_segment ls6(5.0, 5.0, 213*TEST_DEG_TO_RADIAN, 21.0 );

    if(!ls6.is_line_segment_consistent())
    {
        std::cout << "BAD LINE SEGMENT" << std::endl;
        throw KJB_error("Line segment test failed!");
    }
    else
    {
        std::cout << "GOOD" << std::endl;
    }


    Line_segment ls6_a;
    ls6_a.init_from_end_points(ls6.get_start_x(), ls6.get_start_y(), ls6.get_end_x(), ls6.get_end_y());
    Line_segment ls6_b;
    ls6_b.init_from_slope_and_intercept(ls6.get_start_x(), ls6.get_end_x(), ls6.get_slope(), ls6.get_y_intercept());
    if(!ls6_a.is_line_segment_consistent())
    {
        std::cout << "BAD LINE SEGMENT" << std::endl;
        throw KJB_error("Line segment test failed!");
    }
    else
    {
        std::cout << "GOOD" << std::endl;
    }

    if((ls6 == ls6_a))
    {
        std::cout << "GOOD, they are equal" << std::endl;
    }
    else
    {
        std::cout << "BAD, they are different" << std::endl;
        throw KJB_error("Line segment test failed!");
    }
    if(!ls6_b.is_line_segment_consistent())
    {
        std::cout << "BAD LINE SEGMENT" << std::endl;
        throw KJB_error("Line segment test failed!");
    }
    else
    {
        std::cout << "GOOD" << std::endl;
    }

    if((ls6 == ls6_b))
    {
        std::cout << "GOOD, they are equal" << std::endl;
    }
    else
    {
        std::cout << "BAD, they are different" << std::endl;
        throw KJB_error("Line segment test failed!");
    }

    Line_segment ls7(5.0, 5.0, 240*TEST_DEG_TO_RADIAN, 13.02 );

    if(!ls7.is_line_segment_consistent())
    {
        std::cout << "BAD LINE SEGMENT" << std::endl;
        throw KJB_error("Line segment test failed!");
    }
    else
    {
        std::cout << "GOOD" << std::endl;
    }

    Line_segment ls7_a;
    ls7_a.init_from_end_points(ls7.get_start_x(), ls7.get_start_y(), ls7.get_end_x(), ls7.get_end_y());
    Line_segment ls7_b;
    ls7_b.init_from_slope_and_intercept(ls7.get_start_x(), ls7.get_end_x(), ls7.get_slope(), ls7.get_y_intercept());
    if(!ls7_a.is_line_segment_consistent())
    {
        std::cout << "BAD LINE SEGMENT" << std::endl;
        throw KJB_error("Line segment test failed!");
    }
    else
    {
        std::cout << "GOOD" << std::endl;
    }

    if((ls7 == ls7_a))
    {
        std::cout << "GOOD, they are equal" << std::endl;
    }
    else
    {
        std::cout << "BAD, they are different" << std::endl;
        throw KJB_error("Line segment test failed!");
    }
    if(!ls7_b.is_line_segment_consistent())
    {
        std::cout << "BAD LINE SEGMENT" << std::endl;
        throw KJB_error("Line segment test failed!");
    }
    else
    {
        std::cout << "GOOD" << std::endl;
    }

    if((ls7 == ls7_b))
    {
        std::cout << "GOOD, they are equal" << std::endl;
    }
    else
    {
        std::cout << "BAD, they are different" << std::endl;
        throw KJB_error("Line segment test failed!");
    }

    Line_segment ls8(5.0, 5.0, 270*TEST_DEG_TO_RADIAN, 10.0 );

    if(!ls8.is_line_segment_consistent())
    {
        std::cout << "BAD 270 LINE SEGMENT" << std::endl;
        throw KJB_error("Line segment test failed!");
    }
    else
    {
        std::cout << "GOOD" << std::endl;
    }

    Line_segment ls8_a;
    ls8_a.init_from_end_points(ls8.get_start_x(), ls8.get_start_y(), ls8.get_end_x(), ls8.get_end_y());
    Line_segment ls8_b;
    ls8_b.init_vertical_segment(ls8.get_start_y(), ls8.get_end_y(), ls8.get_start_x());
    if(!ls8_a.is_line_segment_consistent())
    {
        std::cout << "BAD LINE SEGMENT" << std::endl;
        throw KJB_error("Line segment test failed!");
    }
    else
    {
        std::cout << "GOOD" << std::endl;
    }

    if((ls8 == ls8_a))
    {
        std::cout << "GOOD, they are equal" << std::endl;
    }
    else
    {
        std::cout << "BAD, they are different" << std::endl;
        throw KJB_error("Line segment test failed!");
    }
    if(!ls8_b.is_line_segment_consistent())
    {
        std::cout << "BAD LINE SEGMENT" << std::endl;
        throw KJB_error("Line segment test failed!");
    }
    else
    {
        std::cout << "GOOD" << std::endl;
    }

    if((ls8 == ls8_b))
    {
        std::cout << "GOOD, they are equal" << std::endl;
    }
    else
    {
        std::cout << "BAD, they are different" << std::endl;
        throw KJB_error("Line segment test failed!");
    }

    Line_segment ls9(5.0, 5.0, 301*TEST_DEG_TO_RADIAN, 110.0 );

    if(!ls9.is_line_segment_consistent())
    {
        std::cout << "BAD LINE SEGMENT" << std::endl;
        throw KJB_error("Line segment test failed!");
    }
    else
    {
        std::cout << "GOOD" << std::endl;
    }

    Line_segment ls9_a;
    ls9_a.init_from_end_points(ls9.get_start_x(), ls9.get_start_y(), ls9.get_end_x(), ls9.get_end_y());
    Line_segment ls9_b;
    ls9_b.init_from_slope_and_intercept(ls9.get_start_x(), ls9.get_end_x(), ls9.get_slope(), ls9.get_y_intercept());
    if(!ls9_a.is_line_segment_consistent())
    {
        std::cout << "BAD LINE SEGMENT" << std::endl;
        throw KJB_error("Line segment test failed!");
    }
    else
    {
        std::cout << "GOOD" << std::endl;
    }

    if((ls9 == ls9_a))
    {
        std::cout << "GOOD, they are equal" << std::endl;
    }
    else
    {
        std::cout << "BAD, they are different" << std::endl;
        throw KJB_error("Line segment test failed!");
    }
    if(!ls9_b.is_line_segment_consistent())
    {
        std::cout << "BAD LINE SEGMENT" << std::endl;
        throw KJB_error("Line segment test failed!");
    }
    else
    {
        std::cout << "GOOD" << std::endl;
    }

    if((ls9 == ls9_b))
    {
        std::cout << "GOOD, they are equal" << std::endl;
    }
    else
    {
        std::cout << "BAD, they are different" << std::endl;
        throw KJB_error("Line segment test failed!");
    }

    Line_segment ls10(5.0, 5.0, 333*TEST_DEG_TO_RADIAN, 10.0 );

    if(!ls10.is_line_segment_consistent())
    {
        std::cout << "BAD LINE SEGMENT" << std::endl;
        throw KJB_error("Line segment test failed!");
    }
    else
    {
        std::cout << "GOOD" << std::endl;
    }

    Line_segment ls10_a;
    ls10_a.init_from_end_points(ls10.get_start_x(), ls10.get_start_y(), ls10.get_end_x(), ls10.get_end_y());
    Line_segment ls10_b;
    ls10_b.init_from_slope_and_intercept(ls10.get_start_x(), ls10.get_end_x(), ls10.get_slope(), ls10.get_y_intercept());
    if(!ls10_a.is_line_segment_consistent())
    {
        std::cout << "BAD LINE SEGMENT" << std::endl;
        throw KJB_error("Line segment test failed!");
    }
    else
    {
        std::cout << "GOOD" << std::endl;
    }

    if((ls10 == ls10_a))
    {
        std::cout << "GOOD, they are equal" << std::endl;
    }
    else
    {
        std::cout << "BAD, they are different" << std::endl;
        throw KJB_error("Line segment test failed!");
    }
    if(!ls10_b.is_line_segment_consistent())
    {
        std::cout << "BAD LINE SEGMENT" << std::endl;
        throw KJB_error("Line segment test failed!");
    }
    else
    {
        std::cout << "GOOD" << std::endl;
    }

    if((ls10 == ls10_b))
    {
        std::cout << "GOOD, they are equal" << std::endl;
    }
    else
    {
        std::cout << "BAD, they are different" << std::endl;
        throw KJB_error("Line segment test failed!");
    }

    Line_segment ls11(5.0, 5.0, 360*TEST_DEG_TO_RADIAN, 10.0 );

    if(!ls11.is_line_segment_consistent())
    {
        std::cout << "BAD LINE SEGMENT" << std::endl;
        throw KJB_error("Line segment test failed!");
    }
    else
    {
        std::cout << "GOOD" << std::endl;
    }

    Line_segment ls11_a;
    ls11_a.init_from_end_points(ls11.get_start_x(), ls11.get_start_y(), ls11.get_end_x(), ls11.get_end_y());
    Line_segment ls11_b;
    ls11_b.init_from_slope_and_intercept(ls11.get_start_x(), ls11.get_end_x(), ls11.get_slope(), ls11.get_y_intercept());
    if(!ls11_a.is_line_segment_consistent())
    {
        std::cout << "BAD LINE SEGMENT" << std::endl;
        throw KJB_error("Line segment test failed!");
    }
    else
    {
        std::cout << "GOOD" << std::endl;
    }

    if((ls11 == ls11_a))
    {
        std::cout << "GOOD, they are equal" << std::endl;
    }
    else
    {
        std::cout << "BAD, they are different" << std::endl;
        throw KJB_error("Line segment test failed!");
    }
    if(!ls11_b.is_line_segment_consistent())
    {
        std::cout << "BAD LINE SEGMENT" << std::endl;
        throw KJB_error("Line segment test failed!");
    }
    else
    {
        std::cout << "GOOD" << std::endl;
    }

    if((ls11 == ls11_b))
    {
        std::cout << "GOOD, they are equal" << std::endl;
    }
    else
    {
        std::cout << "BAD, they are different" << std::endl;
        throw KJB_error("Line segment test failed!");
    }

    Line_segment ls12(5.0, 5.0, 360*TEST_DEG_TO_RADIAN, 10.0 );

    if(!ls12.is_line_segment_consistent())
    {
        std::cout << "BAD LINE SEGMENT" << std::endl;
        throw KJB_error("Line segment test failed!");
    }
    else
    {
        std::cout << "GOOD" << std::endl;
    }

    Line_segment ls12_a;
    ls12_a.init_from_end_points(ls12.get_start_x(), ls12.get_start_y(), ls12.get_end_x(), ls12.get_end_y());
    Line_segment ls12_b;
    ls12_b.init_from_slope_and_intercept(ls12.get_start_x(), ls12.get_end_x(), ls12.get_slope(), ls12.get_y_intercept());
    if(!ls12_a.is_line_segment_consistent())
    {
        std::cout << "BAD LINE SEGMENT" << std::endl;
        throw KJB_error("Line segment test failed!");
    }
    else
    {
        std::cout << "GOOD" << std::endl;
    }

    if((ls12 == ls12_a))
    {
        std::cout << "GOOD, they are equal" << std::endl;
    }
    else
    {
        std::cout << "BAD, they are different" << std::endl;
        throw KJB_error("Line segment test failed!");
    }
    if(!ls12_b.is_line_segment_consistent())
    {
        std::cout << "BAD LINE SEGMENT" << std::endl;
        throw KJB_error("Line segment test failed!");
    }
    else
    {
        std::cout << "GOOD" << std::endl;
    }

    if((ls12 == ls12_b))
    {
        std::cout << "GOOD, they are equal" << std::endl;
    }
    else
    {
        std::cout << "BAD, they are different" << std::endl;
        throw KJB_error("Line segment test failed!");
    }

    Line_segment ls13(5.0, 5.0, 45*TEST_DEG_TO_RADIAN, 10.0 );

    if(!ls13.is_line_segment_consistent())
    {
        std::cout << "BAD LINE SEGMENT" << std::endl;
        throw KJB_error("Line segment test failed!");
    }
    else
    {
        std::cout << "GOOD" << std::endl;
    }

    Line_segment ls13_a;
    ls13_a.init_from_end_points(ls13.get_start_x(), ls13.get_start_y(), ls13.get_end_x(), ls13.get_end_y());
    Line_segment ls13_b;
    ls13_b.init_from_slope_and_intercept(ls13.get_start_x(), ls13.get_end_x(), ls13.get_slope(), ls13.get_y_intercept());
    if(!ls13_a.is_line_segment_consistent())
    {
        std::cout << "BAD LINE SEGMENT" << std::endl;
        throw KJB_error("Line segment test failed!");
    }
    else
    {
        std::cout << "GOOD" << std::endl;
    }

    if((ls13 == ls13_a))
    {
        std::cout << "GOOD, they are equal" << std::endl;
    }
    else
    {
        std::cout << "BAD, they are different" << std::endl;
        throw KJB_error("Line segment test failed!");
    }
    if(!ls13_b.is_line_segment_consistent())
    {
        std::cout << "BAD LINE SEGMENT" << std::endl;
        throw KJB_error("Line segment test failed!");
    }
    else
    {
        std::cout << "GOOD" << std::endl;
    }

    if((ls13 == ls13_b))
    {
        std::cout << "GOOD, they are equal" << std::endl;
    }
    else
    {
        std::cout << "BAD, they are different" << std::endl;
        throw KJB_error("Line segment test failed!");
    }

    Line_segment ls14(5.0, 5.0, 0*TEST_DEG_TO_RADIAN, 10.0 );

    if(!ls14.is_line_segment_consistent())
    {
        std::cout << "BAD LINE SEGMENT" << std::endl;
        throw KJB_error("Line segment test failed!");
    }
    else
    {
        std::cout << "GOOD" << std::endl;
    }

    Line_segment ls14_a;
    ls14_a.init_from_end_points(ls14.get_start_x(), ls14.get_start_y(), ls14.get_end_x(), ls14.get_end_y());
    Line_segment ls14_b;
    ls14_b.init_from_slope_and_intercept(ls14.get_start_x(), ls14.get_end_x(), ls14.get_slope(), ls14.get_y_intercept());
    if(!ls14_a.is_line_segment_consistent())
    {
        std::cout << "BAD LINE SEGMENT" << std::endl;
        throw KJB_error("Line segment test failed!");
    }
    else
    {
        std::cout << "GOOD" << std::endl;
    }

    if((ls14 == ls14_a))
    {
        std::cout << "GOOD, they are equal" << std::endl;
    }
    else
    {
        std::cout << "BAD, they are different" << std::endl;
        throw KJB_error("Line segment test failed!");
    }
    if(!ls14_b.is_line_segment_consistent())
    {
        std::cout << "BAD LINE SEGMENT" << std::endl;
        throw KJB_error("Line segment test failed!");
    }
    else
    {
        std::cout << "GOOD" << std::endl;
    }

    if((ls14 == ls14_b))
    {
        std::cout << "GOOD, they are equal" << std::endl;
    }
    else
    {
        std::cout << "BAD, they are different" << std::endl;
        throw KJB_error("Line segment test failed!");
    }

    Line_segment ls15(5.0, 5.0, 90*TEST_DEG_TO_RADIAN, 10.0 );

    if(!ls15.is_line_segment_consistent())
    {
        std::cout << "BAD 90 LINE SEGMENT" << std::endl;
        throw KJB_error("Line segment test failed!");
    }
    else
    {
        std::cout << "GOOD" << std::endl;
    }

    Line_segment ls15_a;
    ls15_a.init_from_end_points(ls15.get_start_x(), ls15.get_start_y(), ls15.get_end_x(), ls15.get_end_y());
    Line_segment ls15_b;
    ls15_b.init_vertical_segment(ls15.get_start_y(), ls15.get_end_y(), ls15.get_start_x());
    if(!ls15_a.is_line_segment_consistent())
    {
        std::cout << "BAD LINE SEGMENT" << std::endl;
        throw KJB_error("Line segment test failed!");
    }
    else
    {
        std::cout << "GOOD" << std::endl;
    }

    if((ls15 == ls15_a))
    {
        std::cout << "GOOD, they are equal" << std::endl;
    }
    else
    {
        std::cout << "BAD, they are different" << std::endl;
        throw KJB_error("Line segment test failed!");
    }
    if(!ls15_b.is_line_segment_consistent())
    {
        std::cout << "BAD LINE SEGMENT" << std::endl;
        throw KJB_error("Line segment test failed!");
    }
    else
    {
        std::cout << "GOOD" << std::endl;
    }

    if((ls15 == ls15_b))
    {
        std::cout << "GOOD, they are equal" << std::endl;
    }
    else
    {
        std::cout << "BAD, they are different" << std::endl;
        throw KJB_error("Line segment test failed!");
    }

    Line_segment ls16(5.0, 5.0, 45*TEST_DEG_TO_RADIAN, 10.0 );

    if(!ls16.is_line_segment_consistent())
    {
        std::cout << "BAD LINE SEGMENT" << std::endl;
        throw KJB_error("Line segment test failed!");
    }
    else
    {
        std::cout << "GOOD" << std::endl;
    }

    Line_segment ls16_a;
    ls16_a.init_from_end_points(ls16.get_start_x(), ls16.get_start_y(), ls16.get_end_x(), ls16.get_end_y());
    Line_segment ls16_b;
    ls16_b.init_from_slope_and_intercept(ls16.get_start_x(), ls16.get_end_x(), ls16.get_slope(), ls16.get_y_intercept());
    if(!ls16_a.is_line_segment_consistent())
    {
        std::cout << "BAD LINE SEGMENT" << std::endl;
        throw KJB_error("Line segment test failed!");
    }
    else
    {
        std::cout << "GOOD" << std::endl;
    }

    if((ls16 == ls16_a))
    {
        std::cout << "GOOD, they are equal" << std::endl;
    }
    else
    {
        std::cout << "BAD, they are different" << std::endl;
        throw KJB_error("Line segment test failed!");
    }
    if(!ls16_b.is_line_segment_consistent())
    {
        std::cout << "BAD LINE SEGMENT" << std::endl;
        throw KJB_error("Line segment test failed!");
    }
    else
    {
        std::cout << "GOOD" << std::endl;
    }

    if((ls16 == ls16_b))
    {
        std::cout << "GOOD, they are equal" << std::endl;
    }
    else
    {
        std::cout << "BAD, they are different" << std::endl;
        throw KJB_error("Line segment test failed!");
    }

    Line_segment ls17(5.0, 5.0, 135*TEST_DEG_TO_RADIAN, 10.0 );

    if(!ls17.is_line_segment_consistent())
    {
        std::cout << "BAD LINE SEGMENT" << std::endl;
        throw KJB_error("Line segment test failed!");
    }
    else
    {
        std::cout << "GOOD" << std::endl;
    }

    Line_segment ls17_a;
    ls17_a.init_from_end_points(ls17.get_start_x(), ls17.get_start_y(), ls17.get_end_x(), ls17.get_end_y());
    Line_segment ls17_b;
    ls17_b.init_from_slope_and_intercept(ls17.get_start_x(), ls17.get_end_x(), ls17.get_slope(), ls17.get_y_intercept());
    if(!ls17_a.is_line_segment_consistent())
    {
        std::cout << "BAD LINE SEGMENT" << std::endl;
        throw KJB_error("Line segment test failed!");
    }
    else
    {
        std::cout << "GOOD" << std::endl;
    }

    if((ls17 == ls17_a))
    {
        std::cout << "GOOD, they are equal" << std::endl;
    }
    else
    {
        std::cout << "BAD, they are different" << std::endl;
        throw KJB_error("Line segment test failed!");
    }
    if(!ls17_b.is_line_segment_consistent())
    {
        std::cout << "BAD LINE SEGMENT" << std::endl;
        throw KJB_error("Line segment test failed!");
    }
    else
    {
        std::cout << "GOOD" << std::endl;
    }

    if((ls17 == ls17_b))
    {
        std::cout << "GOOD, they are equal" << std::endl;
    }
    else
    {
        std::cout << "BAD, they are different" << std::endl;
        throw KJB_error("Line segment test failed!");
    }

    kjb::Image img = Image::create_zero_image(400,400);
    ls.randomly_color(img);
    ls2.randomly_color(img);
    ls3.randomly_color(img);
    ls4.randomly_color(img);
    ls5.randomly_color(img);
    img.write("ls.tiff");


    kjb::Image imga = Image::create_zero_image(400,400);
    ls_a.randomly_color(imga);
    ls2_a.randomly_color(imga);
    ls3_a.randomly_color(imga);
    ls4_a.randomly_color(imga);
    ls5_a.randomly_color(imga);
    imga.write("lsa.tiff");

    kjb::Image imgb = Image::create_zero_image(400,400);
    ls_b.randomly_color(imgb);
    ls2_b.randomly_color(imgb);
    ls3_b.randomly_color(imgb);
    ls4_b.randomly_color(imgb);
    ls5_b.randomly_color(imgb);
    imgb.write("lsb.tiff");

    return 0;
}

