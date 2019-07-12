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

#include <bbb_cpp/bbb_trajectory.h>
#include <l_cpp/l_test.h>
#include <vector>
#include <algorithm>
#include <iostream>
#include <iterator>

using namespace kjb;
using namespace std;
using namespace boost;

const bool VERBOSE = false;

int main(int argc, char** argv)
{
    bbb::Trajectory traj1;
    bbb::Trajectory traj2;

    // cannot operate on empty trajectory
    TEST_FAIL(traj1.dim(0));
    TEST_FAIL(traj1.pos(1));

    // crate trajectories
    size_t D = 2;
    size_t T = 99;
    size_t start = 0;

    vector<Vector> dims(D);
    generate(dims.begin(), dims.end(), bind(create_random_vector, T + 1));
    vector<Vector> poss = get_transpose(dims);

    traj1.set_dimensions(start, dims.begin(), dims.end());
    traj2.set_positions(start, poss.begin(), poss.end());

    // test basics
    TEST_TRUE(traj1.start() == start);
    TEST_TRUE(traj1.end() == T);
    TEST_TRUE(traj1.size() == T - start + 1);
    TEST_TRUE(traj1.dimensions() == D);

    TEST_TRUE(traj1.start() == traj2.start());
    TEST_TRUE(traj1.end() == traj2.end());
    TEST_TRUE(traj1.size() == traj2.size());
    TEST_TRUE(traj1.dimensions() == traj2.dimensions());

    // test more complex stuff
    for(size_t d = 0; d < D; d++)
    {
        TEST_TRUE(equal(
                    traj1.dim(d).begin(),
                    traj1.dim(d).end(),
                    traj2.dim(d).begin()));
    }

    for(size_t t = 0; t <= T; t++)
    {
        TEST_TRUE(traj1.pos(t) == traj2.pos(t));
    }

    vector<Vector> pts1(traj1.size());
    vector<Vector> pts2(traj2.size());
    traj1.copy(pts1.begin());
    traj2.copy(pts2.begin());

    TEST_TRUE(equal(poss.begin(), poss.end(), pts1.begin()));
    TEST_TRUE(equal(pts1.begin(), pts1.end(), pts2.begin()));

    // append more points
    traj1.append_positions(poss.begin(), poss.end());
    traj2.append_dimensions(dims.begin(), dims.end());

    // test basics
    TEST_TRUE(traj1.start() == start);
    TEST_TRUE(traj1.end() == 2*T + 1);
    TEST_TRUE(traj1.size() == 2*T - start + 2);
    TEST_TRUE(traj1.dimensions() == D);

    TEST_TRUE(traj1.start() == traj2.start());
    TEST_TRUE(traj1.end() == traj2.end());
    TEST_TRUE(traj1.size() == traj2.size());
    TEST_TRUE(traj1.dimensions() == traj2.dimensions());

    // test more complex stuff
    for(size_t d = 0; d < D; d++)
    {
        TEST_TRUE(equal(
                    traj1.dim(d).begin(),
                    traj1.dim(d).end(),
                    traj2.dim(d).begin()));
    }

    for(size_t t = 1; t <= T; t++)
    {
        TEST_TRUE(traj1.pos(t) == traj2.pos(t));
    }

    pts1.resize(traj1.size());
    pts2.resize(traj2.size());
    traj1.copy(pts1.begin());
    traj2.copy(pts2.begin());

    TEST_TRUE(equal(poss.begin(), poss.end(), pts1.begin()));
    TEST_TRUE(equal(pts1.begin(), pts1.end(), pts2.begin()));

    RETURN_VICTORIOUSLY();
}

