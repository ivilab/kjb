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

/* $Id$ */

#include <diff_cpp/diff_fourier.h>
#include <m_cpp/m_matrix.h>
#include <l_cpp/l_test.h>
#include <iostream>

using namespace std;
using namespace kjb;

const bool VERBOSE = true;

/** @brief  Main. */
int main(int argc, char** argv)
{
    size_t D = 20;

    Matrix Q = fourier_basis(D, D);
    double dt = Q.abs_of_determinant();

    if(VERBOSE)
    {
        cout << "Q is " << D << " x " << D << endl;
        cout << "|Q| = " << dt << endl;
    }

    TEST_TRUE(fabs(dt  - 1.0) < 1e-10);

    RETURN_VICTORIOUSLY();
}

