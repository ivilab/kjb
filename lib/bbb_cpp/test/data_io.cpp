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
#include <m_cpp/m_vector.h>
#include <vector>
#include <algorithm>
#include <iostream>
#include <streambuf>

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
    bbb::Intentional_activity meet("MEET", start, end, Vector(0.0, 0.0), trajs);
    bbb::Description_prior prior(meet, ass_prior, as_prior, traj_prior, lib);

    // likelihood
    bbb::Likelihood likelihood(lib);

    // sample a description
    bbb::Description desc = bbb::sample(prior);

    // sample data
    bbb::Data data = bbb::sample(likelihood, desc);
    bbb::write(data, "output/data_io_cpp/data1.txt");

    bbb::Data data2;
    bbb::read(data2, "output/data_io_cpp/data1.txt");
    bbb::write(data2, "output/data_io_cpp/data2.txt");

    // quick test to see if files are the same
    std::ifstream ifs1("output/data_io_cpp/data1.txt");
    std::ifstream ifs2("output/data_io_cpp/data2.txt");
    std::string str1(
        (std::istreambuf_iterator<char>(ifs1)),
        std::istreambuf_iterator<char>());
    std::string str2(
        (std::istreambuf_iterator<char>(ifs2)),
        std::istreambuf_iterator<char>());

    if(VERBOSE)
    {
        cout << "DATA1" << endl;
        cout << "-----------------------------------------------------" << endl;
        cout << str1 << endl << endl;

        cout << "DATA2" << endl;
        cout << "-----------------------------------------------------" << endl;
        cout << str2 << endl;
    }

    TEST_TRUE(str1 == str2);

    RETURN_VICTORIOUSLY();
}

