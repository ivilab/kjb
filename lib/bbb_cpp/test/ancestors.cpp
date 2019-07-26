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
#include <bbb_cpp/bbb_activity_sequence.h>
#include <bbb_cpp/bbb_intentional_activity.h>
#include <bbb_cpp/bbb_physical_activity.h>
#include <bbb_cpp/bbb_description.h>
#include <l_cpp/l_test.h>
#include <l_cpp/l_functors.h>
#include <m_cpp/m_vector.h>
#include <prob_cpp/prob_distribution.h>
#include <prob_cpp/prob_sample.h>
#include <vector>
#include <algorithm>
#include <iostream>
#include <utility>
#include <boost/variant.hpp>

using namespace kjb;
using namespace std;

const bool VERBOSE = false;

/** @brief  Get a physical activity UAR from a description. */
const bbb::Physical_activity& random_activity(const bbb::Description& desc);

/** @brief  Main program. */
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

    // sample description without endpoints
    //bbb::Description desc = bbb::sample(prior);
    bbb::Description desc(prior.root_activity());
    bbb::Sample_tree sample_tree(prior, desc);
    sample_tree(desc.root_activity());

    // get ancestors of random activity
    const bbb::Physical_activity& pa = random_activity(desc);
    std::vector<const bbb::Intentional_activity*> ancs;
    desc.ancestors(pa, back_inserter(ancs));

    // some tests
    TEST_TRUE(desc.is_root(pa) || ancs.back() == &desc.root_activity());

    if(pa.name() == "WALK" || pa.name() == "RUN")
    {
        TEST_TRUE(ancs.size() == 2);
        TEST_TRUE(ancs.front()->name() == "MOVE-TO");
    }
    else if(pa.name() == "STAND")
    {
        TEST_TRUE(ancs.size() == 2 || ancs.size() == 1);
    }
    else
    {
        TEST_TRUE(false);
    }

    RETURN_VICTORIOUSLY();
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

const bbb::Physical_activity& random_activity(const bbb::Description& desc)
{
    using namespace bbb;
    typedef Description::Act_tree::const_iterator const_iterator;
    typedef std::pair<const_iterator, const_iterator> it_pair;

    const Physical_activity* phact_p = 0;
    const Intentional_activity* inact_p = &desc.root_activity();
    while(1)
    {
        // first grab all children and put into big vector
        it_pair rg = desc.children(*inact_p);
        std::vector<const Activity_sequence::Activity*> all_children;
        for(; rg.first != rg.second; ++rg.first)
        {
            const Activity_sequence& aseq = rg.first->second;
            for(size_t j = 0; j < aseq.size(); j++)
            {
                all_children.push_back(&aseq.activity(j));
            }
        }

        // choose from all of them
        Categorical_distribution<size_t> U(0, all_children.size() - 1, 1);
        size_t n = sample(U);
        const Activity_sequence::Activity& act = *all_children[n];

        inact_p = boost::get<Intentional_activity>(&act);
        if(!inact_p)
        {
            phact_p = boost::get<Physical_activity>(&act);
            break;
        }
    }

    return *phact_p;
}

