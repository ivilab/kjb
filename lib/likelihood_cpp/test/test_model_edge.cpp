/* $Id: test_model_edge.cpp 21596 2017-07-30 23:33:36Z kobus $ */

#include "l/l_sys_debug.h"  /* For ASSERT */
#include "likelihood_cpp/model_edge.h"
#include <iostream>

int main(int argc, char **argv)
{
    using namespace kjb;

    Model_edge m1(10, 3, 20 ,5);
    ASSERT( fabs(m1.get_dx() - 10) < DBL_EPSILON );
    ASSERT( fabs(m1.get_dy() - 2) < DBL_EPSILON );

    Model_edge m2(30, 3, 20 ,5);
    ASSERT( fabs(m2.get_dx() - 10) < DBL_EPSILON );
    ASSERT( fabs(m2.get_dy() - (-2)) < DBL_EPSILON );

    Model_edge m3(30, 3, 30 ,5);
    ASSERT( fabs(m3.get_dx() - 0) < DBL_EPSILON );
    ASSERT( fabs(m3.get_dy() - 2) < DBL_EPSILON );

    Model_edge m4(30, 1, 40 , -3);
    ASSERT( fabs(m4.get_dx() - 10) < DBL_EPSILON );
    ASSERT( fabs(m4.get_dy() - (-4)) < DBL_EPSILON );

    return 0;
}
