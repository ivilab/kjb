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
#include <bbb_cpp/bbb_traj_set.h>
#include <bbb_cpp/bbb_data.h>
#include <l_cpp/l_test.h>
#include <vector>
#include <algorithm>

using namespace kjb;
using namespace std;
using namespace boost;

const bool VERBOSE = false;

/** @brief  Compare a trajectory's address with trajectory pointer. */
bool compare_to_address
(
    const bbb::Trajectory& traj,
    const bbb::Trajectory* traj_p
);

/** @brief  Main function (duh!). */
int main(int argc, char** argv)
{
    //// CREATE DATA
    size_t D = 2;
    size_t start = 0;
    size_t T = 99;
    size_t K = 10;

    vector<bbb::Trajectory> trajs(K);
    for(size_t k = 0; k < K; k++)
    {
        vector<Vector> dims(D);
        generate(dims.begin(), dims.end(), bind(create_random_vector, T + 1));
        trajs[k].set_dimensions(start, dims.begin(), dims.end());
    }

    bbb::Data data(trajs.begin(), trajs.end());

    //// CREATE ASSOCIATIONS
    bbb::Traj_set first_half;
    bbb::Traj_set second_half;
    bbb::Traj_set every_other;
    vector<const bbb::Trajectory*> traj_ps(K/2);

    for(size_t k = 0; k < traj_ps.size(); k++)
    {
        first_half.insert(k);
        second_half.insert(data.size() - 1 - k);
        every_other.insert(2*k);
    }

    //// TEST ASSOCIATIONS
    first_half.trajectories(data, traj_ps.begin());
    TEST_TRUE(equal(
                data.begin(),
                data.begin() + traj_ps.size(),
                traj_ps.begin(),
                compare_to_address));

    second_half.trajectories(data, traj_ps.begin());
    TEST_TRUE(equal(
                data.begin() + traj_ps.size(),
                data.end(),
                traj_ps.begin(),
                compare_to_address));

    every_other.trajectories(data, traj_ps.begin());
    bool eq = true;
    for(size_t k = 0; k < traj_ps.size(); k++)
    {
        if(&data.trajectory(2*k) != traj_ps[k])
        {
            eq = false;
            break;
        }
    }

    TEST_TRUE(eq);

    RETURN_VICTORIOUSLY();
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

inline
bool compare_to_address
(
    const bbb::Trajectory& traj,
    const bbb::Trajectory* traj_p
)
{
    return &traj == traj_p;
}

