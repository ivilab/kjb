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

#include <bbb_cpp/bbb_activity_library.h>
#include <bbb_cpp/bbb_intentional_activity.h>
#include <bbb_cpp/bbb_description.h>
#include <bbb_cpp/bbb_endpoints.h>
#include <l_cpp/l_test.h>
#include <l_cpp/l_functors.h>
#include <m_cpp/m_vector.h>
#include <m_cpp/m_matrix.h>
#include <vector>
#include <string>
#include <sstream>
#include <algorithm>
#include <iostream>

using namespace kjb;
using namespace std;

const bool VERBOSE = true;

/** @brief  Stringify a p-act. */
string to_string(const bbb::Physical_activity& act);

int main(int argc, char** argv)
{
    // trajectories
    vector<size_t> trs(6);
    generate(trs.begin(), trs.end(), Increment<size_t>(0));

    // prior
    bbb::Activity_library lib("/home/ernesto/.local/data/bbb/activity_library");
    bbb::Intentional_activity meet(
        "MEET", 0, 1, Vector(0.0, 0.0), bbb::Traj_set(trs.begin(), trs.end()));

    // sample
    bbb::Description desc(meet);
    bbb::read(desc, "input/sample_endpoints", lib, trs.begin(), trs.end());

    if(VERBOSE)
    {
        cout << desc << endl;
    }

    // endpoints
    bbb::Endpoint_set epts_info;
    bbb::trajectory_endpoints(epts_info, desc.root_activity(), desc);

    if(VERBOSE)
    {
        for(size_t i = 0; i < epts_info.size(); ++i)
        {
            cout << "x" << i << endl;

            cout << "  left: ";
            if(epts_info[i].left_p)
            {
                cout << to_string(*epts_info[i].left_p);
            }
            cout << endl;

            cout << "  right: ";
            if(epts_info[i].right_p)
            {
                cout << to_string(*epts_info[i].right_p);
            }
            cout << endl;

            cout << "  incoming: ";
            const vector<size_t>& inc = epts_info[i].incoming;
            copy(inc.begin(), inc.end(), ostream_iterator<size_t>(cout, ","));
            cout << endl;

            cout << endl;
        }
    }

    // endpoint mean and covariance
    using boost::tie;
    Vector ep_mux;
    Vector ep_muy;
    std::vector<size_t> tgts;
    tie(ep_mux, ep_muy, tgts) = bbb::endpoint_mean(epts_info, desc, lib);
    Matrix ep_cov = bbb::endpoint_covariance(epts_info, lib);

    if(VERBOSE)
    {
        cout << "EP mean" << endl;
        cout << ep_mux << endl;
        cout << ep_muy << endl << endl;

        cout << "EP covariance" << endl;
        cout << ep_cov << endl;
    }

    RETURN_VICTORIOUSLY();
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

string to_string(const bbb::Physical_activity& act)
{
    stringstream sstr;

    sstr << act.name();
    sstr << " [" << act.start() << ", " << act.end() << "]";
    sstr << " {";
    copy(
        act.trajectories().begin(),
        --act.trajectories().end(),
        ostream_iterator<size_t>(sstr, ", "));
    sstr << *act.trajectories().rbegin();
    sstr << "}";

    return sstr.str();
}

