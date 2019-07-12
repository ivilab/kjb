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

#include <bbb_cpp/bbb_likelihood.h>
#include <bbb_cpp/bbb_description_prior.h>
#include <bbb_cpp/bbb_activity_sequence_prior.h>
#include <bbb_cpp/bbb_parameter_prior.h>
#include <bbb_cpp/bbb_trajectory_prior.h>
#include <bbb_cpp/bbb_association_prior.h>
#include <bbb_cpp/bbb_activity_library.h>
#include <bbb_cpp/bbb_intentional_activity.h>
#include <bbb_cpp/bbb_description.h>
#include <bbb_cpp/bbb_data.h>
#include <l_cpp/l_test.h>
#include <l_cpp/l_functors.h>
#include <l_cpp/l_word_list.h>
#include <l/l_sys_io.h>
#include <m_cpp/m_vector.h>
#include <vector>
#include <algorithm>
#include <iostream>
#include <streambuf>

using namespace kjb;
using namespace std;

const bool VERBOSE = false;

string desc_as_string(const string& dp);

int main(int argc, char** argv)
{
    // output paths
    string out_dp1 = "output/description_io_cpp/orig";
    string out_dp2 = "output/description_io_cpp/read";

    cerr << "Please make sure you remove all files from output dirs "
         << out_dp1
         << " and "
         << out_dp2
         << " or else this test will fail!"
         << endl;

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
    bbb::Intentional_activity meet("MEET", start, end, Vector(0.0, 0.0), trajs);
    bbb::Description_prior prior(meet, ass_prior, as_prior, traj_prior, lib);

    // likelihood
    bbb::Likelihood likelihood(lib);

    // sample a description and data
    bbb::Description desc = bbb::sample(prior);
    bbb::Data data = bbb::sample(likelihood, desc);

    // write description
    kjb_c::kjb_mkdir(out_dp1.c_str());
    TEST_SUCCESS(bbb::write(desc, out_dp1, lib, data.ibegin(), data.iend()));

    // read description
    bbb::Description desc2 = desc;
    TEST_SUCCESS(bbb::read(desc2, out_dp1, lib, data.ibegin(), data.iend()));

    // re-write
    kjb_c::kjb_mkdir(out_dp2.c_str());
    TEST_SUCCESS(bbb::write(desc2, out_dp2, lib, data.ibegin(), data.iend()));

    // read as strings and compare
    string dstr1 = desc_as_string(out_dp1);
    string dstr2 = desc_as_string(out_dp2);

    TEST_TRUE(dstr1 == dstr2);

    RETURN_VICTORIOUSLY();
}

string desc_as_string(const string& dp)
{
    Word_list wl(dp + "/*.txt");

    string outstr;
    for(size_t i = 0; i < wl.size(); ++i)
    {
        ifstream ifs(wl[i]);
        outstr.append(
            (istreambuf_iterator<char>(ifs)),
            istreambuf_iterator<char>());
        ifs.close();
    }

    return outstr;
}

