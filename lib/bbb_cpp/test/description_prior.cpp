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

#include <bbb_cpp/bbb_description_prior.h>
#include <bbb_cpp/bbb_activity_sequence_prior.h>
#include <bbb_cpp/bbb_parameter_prior.h>
#include <bbb_cpp/bbb_trajectory_prior.h>
#include <bbb_cpp/bbb_association_prior.h>
#include <bbb_cpp/bbb_activity_library.h>
#include <bbb_cpp/bbb_intentional_activity.h>
#include <bbb_cpp/bbb_description.h>
#include <l_cpp/l_test.h>
#include <l_cpp/l_functors.h>
#include <m_cpp/m_vector.h>
#include <vector>
#include <algorithm>
#include <iostream>

using namespace kjb;
using namespace std;

const bool VERBOSE = false;

int main(int argc, char** argv)
{
    // trajectories
    vector<size_t> trs(20);
    generate(trs.begin(), trs.end(), Increment<size_t>(0));
    bbb::Traj_set trajs(trs.begin(), trs.end());

    // times
    const size_t start = 0;
    const size_t end = 299;

    // prior
    bbb::Activity_library lib("/home/ernesto/.local/data/bbb/activity_library");
    bbb::Parameter_prior param_prior(lib);
    bbb::Activity_sequence_prior as_prior(param_prior, lib);
    bbb::Association_prior ass_prior(lib);
    bbb::Trajectory_prior traj_prior(2, lib);
    bbb::Intentional_activity meet("WALK-TO", start, end, Vector(0.0, 0.0), trajs);
    bbb::Description_prior prior(meet, ass_prior, as_prior, traj_prior, lib);

    // sample
    bbb::Description desc(meet);
    TEST_SUCCESS(desc = bbb::sample(prior));

    if(VERBOSE)
    {
        cout << desc << endl;
    }

    RETURN_VICTORIOUSLY();
}

