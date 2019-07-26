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

/* $Id: test_linalg.cpp 19553 2015-07-01 05:41:06Z jguan1 $ */

#include <l_cpp/l_test.h>
#include <iostream>
#include <cmath>
using namespace std;
#ifndef KJB_HAVE_GSL
int main(int argc, char** argv)
{
    cout << "gsl not installed." << endl;
    return 1;
}
#else

#include <gsl_cpp/gsl_linalg.h>

using namespace kjb;

int main(int argc, char** argv)
{
    Gsl_matrix A(2, 2);
    A.at(0, 0) = 1.0;
    A.at(0, 1) = 0.0;
    A.at(1, 0) = 0.0;
    A.at(1, 1) = 2.0;
    //std::cout << A << std::endl;
    Gsl_matrix eA(2, 2);
    //std::cout << "---------------------------\n";
    
    gsl_matrix_exponential(A, eA);
    //std::cout << eA << std::endl;
    TEST_TRUE(std::fabs(eA.at(0, 0) - std::exp(A.at(0, 0))) < FLT_EPSILON);
    TEST_TRUE(std::fabs(eA.at(1, 1) - std::exp(A.at(1, 1))) < FLT_EPSILON);

    return EXIT_SUCCESS;
}

#endif
