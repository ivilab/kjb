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

#include <diff_cpp/diff_optim.h>
#include <diff_cpp/diff_gradient.h>
#include <m_cpp/m_vector.h>
#include <l_cpp/l_test.h>
#include <vector>
#include <iostream>
#include <functional>
#include <boost/bind.hpp>
#include <boost/function.hpp>
#include "funcs.h"

using namespace std;
using namespace kjb;
using namespace boost;

const bool VERBOSE = false;

/** @brief  Main. */
int main(int argc, char** argv)
{
    const double eps = 1e-3;
    const size_t D = 50;
    Vector x = 100*create_random_vector(D);
    vector<double> ss(D, 0.01);
    vector<double> dx(D, 0.0001);

    typedef function1<double, const Vector&> F;
    F f = bind(negate<double>(), bind(x_squared, _1));

    // test simultaneous gradient ascent
    gradient_ascent(f, x, ss, bind(gradient_cfd<F, Vector>, f, _1, dx));

    if(VERBOSE)
    {
        //cout << "x = " << x << endl;
        cout << "||x|| = " << x.magnitude() << endl;
    }

    TEST_TRUE(x.magnitude() < eps);

    RETURN_VICTORIOUSLY();
}


