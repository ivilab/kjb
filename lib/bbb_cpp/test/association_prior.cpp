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

#include <bbb_cpp/bbb_association.h>
#include <bbb_cpp/bbb_association_prior.h>
#include <bbb_cpp/bbb_activity_library.h>
#include <l_cpp/l_test.h>
#include <l_cpp/l_functors.h>
#include <m_cpp/m_vector.h>
#include <vector>
#include <iterator>
#include <iostream>

using namespace kjb;
using namespace std;

const bool VERBOSE = false;

int main(int argc, char** argv)
{
    bbb::Activity_library lib("/home/ernesto/.local/data/bbb/activity_library");
    bbb::Association_prior prior(lib);

    vector<size_t> trs(20);
    generate(trs.begin(), trs.end(), Increment<size_t>(0));
    bbb::Traj_set trajs(trs.begin(), trs.end());

    bbb::Intentional_activity meet("MEET", 0, 99, Vector(0.0, 0.0), trajs);
    bbb::Intentional_activity mt("MOVE-TO", 0, 99, Vector(0.0, 0.0), trajs);

    prior.set_parent(meet);
    bbb::Association ass_meet(trajs);
    TEST_SUCCESS(ass_meet = bbb::sample(prior));

    prior.set_parent(mt);
    bbb::Association ass_mt(trajs);
    TEST_SUCCESS(ass_mt = bbb::sample(prior));

    if(VERBOSE)
    {
        cout << "Original trajectories: ";
        copy(trajs.begin(), trajs.end(), ostream_iterator<size_t>(cout, " "));
        cout << endl;

        cout << "MEET ==========================" << endl;
        size_t K = ass_meet.num_groups();
        for(size_t k = 0; k < K; k++)
        {
            const bbb::Traj_set& trk = ass_meet.group(k).trajectories();
            cout << "Group " << k << " (" << ass_meet.group(k).role() << "): ";
            copy(trk.begin(), trk.end(), ostream_iterator<size_t>(cout, " "));
            cout << endl;
        }
        cout << endl;

        cout << "MOVE-TO ==========================" << endl;
        K = ass_mt.num_groups();
        for(size_t k = 0; k < K; k++)
        {
            const bbb::Traj_set& trk = ass_mt.group(k).trajectories();
            cout << "Group " << k << " (" << ass_mt.group(k).role() << "): ";
            copy(trk.begin(), trk.end(), ostream_iterator<size_t>(cout, " "));
            cout << endl;
        }
    }

    TEST_TRUE(ass_mt.num_groups() <= ass_meet.num_groups());

    RETURN_VICTORIOUSLY();
}

