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

#include "l/l_sys_debug.h"  /* For ASSERT */
#include "bbb_cpp/bbb_likelihood.h"
#include "bbb_cpp/bbb_data.h"
#include "bbb_cpp/bbb_description.h"
#include "bbb_cpp/bbb_activity_library.h"
#include "bbb_cpp/bbb_trajectory.h"
#include "gp_cpp/gp_base.h"
#include "gp_cpp/gp_predictive.h"
#include "gp_cpp/gp_covariance.h"
#include "gp_cpp/gp_mean.h"
#include "gp_cpp/gp_sample.h"
#include "m_cpp/m_vector.h"
#include "prob_cpp/prob_distribution.h"
#include "prob_cpp/prob_sample.h"

#include <vector>
#include <map>
#include <utility>
#include <boost/foreach.hpp>
#include <boost/variant.hpp>

using namespace kjb;
using namespace kjb::bbb;

double Likelihood::operator()(const Description& desc) const
{
    return 0.0;
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

Data kjb::bbb::sample(const Likelihood& likelihood, const Description& desc)
{
    typedef Description::Act_tree Act_tree;
    typedef Activity_sequence::Activity Activity;
    typedef Physical_activity Pa;
    typedef std::map<size_t, const Pa*> Time_map;

    // make sure description has trajectories (and are indexed correctly)
    const size_t num_trajs = desc.root_activity().trajectories().size();
    IFT(num_trajs > 0, Illegal_argument,
        "Cannot sample data; empty description");
    ASSERT(num_trajs == *desc.root_activity().trajectories().rbegin() + 1);

    const Activity_library& lib = likelihood.library();
    Likelihood::Predictive pred = likelihood.predictive();

    // prepare predictive
    gp::Inputs temp_ins(2);
    Vector temp_outs(2);
    pred.set_train_data(temp_ins.begin(), temp_ins.end(), temp_outs.begin());

    // first build a set of trajectories for each person
    // separated by activity
    std::vector<Time_map> time_maps(num_trajs);
    BOOST_FOREACH(const Act_tree::value_type& pr, desc.tree_)
    {
        const Activity_sequence& aseq = pr.second;

        const size_t nacts = aseq.size();
        for(size_t i = 0; i < nacts; i++)
        {
            const Activity& act = aseq.activity(i);
            const Pa* pa_p = boost::get<Pa>(&act);
            if(pa_p)
            {
                const Pa& pa = *pa_p;
                const Traj_set& pa_set = pa.trajectories();
                const Trajectory& traj = pa.trajectory();

                // get individual trajectories for each activity
                BOOST_FOREACH(const Traj_set::value_type& j, pa_set)
                {
                    ASSERT(time_maps[j].count(traj.start()) == 0);
                    time_maps[j][traj.start()] = pa_p;
                }
            }
        }
    }

    // merge trajectories
    size_t D = time_maps.front().begin()->second->trajectory().dimensions();
    std::vector<Trajectory> trajs(num_trajs);
    for(size_t j = 0; j < num_trajs; j++)
    {
        const Time_map& tmap = time_maps[j];

        // sample first start-point
        size_t t0 = tmap.begin()->first;
        Vector m = tmap.begin()->second->trajectory().pos(t0);
        double var = lib.trajectory_kernel(
                                tmap.begin()->second->name()).signal_sigma();

        std::vector<Vector> x0(2);
        for(size_t d = 0; d < D; d++)
        {
            x0[d].set(kjb::sample(Normal_distribution(m[d], sqrt(var))));
        }

        trajs[j].set_dimensions(t0, x0.begin(), x0.end());
        ASSERT(trajs[j].size() == 1);

        // sample remaining endpoints + trajectories
        BOOST_FOREACH(const Time_map::value_type& pr, tmap)
        {
            const Physical_activity& pa = *pr.second;
            const size_t st = pa.start();
            const size_t ed = pa.end();
            ASSERT(st == pr.first);

            // if traj has size 1, no middle or endpoints to sample
            if(ed == st) continue;

            // set covariance function
            gp::Sqex sqex = lib.trajectory_kernel(pa.name());
            sqex = gp::Sqex(sqex.scale(), 0.1);
            pred.set_covariance_function(sqex);

            // get endpoints
            Vector ep1 = trajs[j].pos(trajs[j].end());
            m = pa.trajectory().pos(ed);
            var = sqex.signal_sigma();
            std::vector<Vector> ep2(D);

            // if traj has size 2, no middle to sample, only endpoints
            if(ed - st == 1)
            {
                for(size_t d = 0; d < D; d++)
                {
                    ep2[d].set(
                        kjb::sample(Normal_distribution(m[d], sqrt(var))));
                }

                trajs[j].append_dimensions(ep2.begin(), ep2.end());
                continue;
            }

            // training inputs
            gp::Inputs trins(2);
            trins[0].set(st);
            trins[1].set(ed);
            pred.set_train_inputs(trins.begin(), trins.end());

            // test inputs
            gp::Inputs teins = gp::make_inputs(st + 1, ed - 1, 1);
            pred.set_test_inputs(teins.begin(), teins.end());

            // al inputs
            gp::Inputs allins = gp::make_inputs(st, ed, 1);

            // sample
            std::vector<Vector> teouts(D);
            for(size_t d = 0; d < D; d++)
            {
                // set mean function
                gp::Manual mf(allins, pa.trajectory().dim(d));
                pred.set_mean_function(mf);

                // sample endpoint[d]
                // this is a vector of Vector because that's how Trajectory
                // requires it to be
                ep2[d].set(kjb::sample(Normal_distribution(m[d], sqrt(var))));

                // set training outputs (from endpoints)
                Vector trouts(2);
                trouts.set(ep1[d], ep2[d][0]);
                pred.set_train_outputs(trouts.begin(), trouts.end());

                // sample test outputs (between endpoints)
                teouts[d] = gp::sample(pred);
            }

            // set trajectory
            trajs[j].append_dimensions(teouts.begin(), teouts.end());
            trajs[j].append_dimensions(ep2.begin(), ep2.end());
            ASSERT(trajs[j].end() == ed);
        }
    }

    return Data(trajs.begin(), trajs.end());
}

