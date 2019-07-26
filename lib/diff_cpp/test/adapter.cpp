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

#include <diff_cpp/diff_util.h>
#include <m_cpp/m_vector.h>
#include <l_cpp/l_test.h>
#include <iostream>

using namespace std;
using namespace kjb;

//const bool VERBOSE = false;

/** @brief  Main. */
int main(int argc, char** argv)
{
    const size_t D = 100;
    Vector x = create_random_vector(D);
    Vector_adapter<Vector> a;

    double v = 99.0;
    Vector vv = create_random_vector(D);

    // test size
    TEST_TRUE(D == a.size(&x));

    // test get
    for(size_t i = 0; i < D; i++)
    {
        TEST_TRUE(a.get(&x, i) == x[i]);
    }

    // test set
    for(size_t i = 0; i < D; i++)
    {
        a.set(&x, i, v);
        TEST_TRUE(x[i] == v);
    }

    // test move
    for(size_t i = 0; i < D; i++)
    {
        move_param(x, i, v, a);
        TEST_TRUE(x[i] == 2*v);
    }

    // test double set
    for(size_t i = 0; i < D; i++)
    {
        for(size_t j = 0; j < D; j++)
        {
            if(i == j) continue;

            a.set(&x, i, j, v, v);
            TEST_TRUE(x[i] == v);
            TEST_TRUE(x[j] == v);
        }
    }

    // test double move
    for(size_t i = 0; i < D; i++)
    {
        for(size_t j = 0; j < D; j++)
        {
            if(i == j) continue;

            a.set(&x, i, v);
            a.set(&x, j, v);
            move_params(x, i, j, v, v, a);
            TEST_TRUE(x[i] == 2*v);
            TEST_TRUE(x[j] == 2*v);
        }
    }

    // test full set
    a.set(&x, vv);
    TEST_TRUE(x == vv);

    // test full move
    move_params(x, vv, a);
    TEST_TRUE(x == 2*vv);

    RETURN_VICTORIOUSLY();
}

