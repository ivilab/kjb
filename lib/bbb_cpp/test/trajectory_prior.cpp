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
#include <bbb_cpp/bbb_trajectory_prior.h>
#include <bbb_cpp/bbb_activity_library.h>
#include <l_cpp/l_test.h>
#include <m_cpp/m_vector.h>
#include <algorithm>
#include <vector>
#include <iostream>
#include <boost/bind.hpp>

using namespace kjb;
using namespace std;

const bool VERBOSE = false;

int main(int argc, char** argv)
{
    // prepare library and prior
    bbb::Activity_library lib("/home/ernesto/.local/data/bbb/activity_library");
    bbb::Trajectory_prior prior(2, lib);
    size_t start = 0;
    size_t end = 149;
    Vector strpt = Vector().set(0.0, 0.0);
    Vector endpt = Vector().set(0.0, 0.0);

    prior.set_name("WALK");
    prior.set_start(start);
    prior.set_end(end);
    prior.set_endpoint_means(strpt, endpt);

    // sample a trajectory and saave positions
    bbb::Trajectory traj = bbb::sample(prior);
    vector<Vector> pts_walk(traj.size());
    traj.copy(pts_walk.begin());

    //// TEST BASIC CORRECTNESS
    TEST_TRUE(traj.start() == start);
    TEST_TRUE(traj.end() == end);
    TEST_TRUE(traj.dimensions() == 2);

    //// TEST AGAINST OTHER TYPES
    // run
    prior.set_name("RUN");
    traj = bbb::sample(prior);
    vector<Vector> pts_run(traj.size());
    traj.copy(pts_run.begin());

    // stand
    prior.set_name("STAND");
    traj = bbb::sample(prior);
    vector<Vector> pts_stand(traj.size());
    traj.copy(pts_stand.begin());

    // compute distance traveled
    vector<double> dists(pts_walk.size() - 1);
    transform(
        pts_walk.begin() + 1,
        pts_walk.end(),
        pts_walk.begin(),
        dists.begin(),
        static_cast<double(*)(const Vector&, const Vector&)>(vector_distance));
    double dist_walk = accumulate(dists.begin(), dists.end(), 0.0);

    transform(
        pts_run.begin() + 1,
        pts_run.end(),
        pts_run.begin(),
        dists.begin(),
        static_cast<double(*)(const Vector&, const Vector&)>(vector_distance));
    double dist_run = accumulate(dists.begin(), dists.end(), 0.0);

    transform(
        pts_stand.begin() + 1,
        pts_stand.end(),
        pts_stand.begin(),
        dists.begin(),
        static_cast<double(*)(const Vector&, const Vector&)>(vector_distance));
    double dist_stand = accumulate(dists.begin(), dists.end(), 0.0);

    if(VERBOSE)
    {
        cout << "STAND traveled " << dist_stand << " meters." << endl;
        cout << "WALK traveled " << dist_walk << " meters." << endl;
        cout << "RUN traveled " << dist_run << " meters." << endl;
    }

    TEST_TRUE(dist_stand < dist_walk && dist_walk < dist_run);

    RETURN_VICTORIOUSLY();
}

