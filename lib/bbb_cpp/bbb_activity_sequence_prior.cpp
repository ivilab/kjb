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

#include "bbb_cpp/bbb_activity_sequence_prior.h"
#include "bbb_cpp/bbb_parameter_prior.h"
#include "bbb_cpp/bbb_trajectory_prior.h"
#include "bbb_cpp/bbb_activity_sequence.h"
#include "bbb_cpp/bbb_intentional_activity.h"
#include "bbb_cpp/bbb_physical_activity.h"
#include "bbb_cpp/bbb_trajectory.h"
#include "prob_cpp/prob_distribution.h"
#include "prob_cpp/prob_sample.h"
#include "m_cpp/m_vector.h"
#include "m_cpp/m_matrix.h"

#include <vector>
#include <utility>

using namespace kjb;
using namespace bbb;

std::vector<size_t> As_prior::sample_markov_chain(size_t sz) const
{
    std::vector<size_t> chain(sz);

    std::pair<Vector, Matrix> mc = lib_.markov_chain(role_);
    const Vector& x0 = mc.first;
    const Matrix& K = mc.second;

    // first state
    Categorical_distribution<size_t> p0(x0, 0);
    chain.front() = kjb::sample(p0);

    // following states
    for(size_t t = 1; t < sz - 1; t++)
    {
        Vector pt = K.get_row(chain[t - 1]);
        Categorical_distribution<size_t> P(pt, 0);

        chain[t] = kjb::sample(P);
    }

    chain[sz - 1] = chain[sz - 2];

    return chain;
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

As_prior::Chain_pair As_prior::condense_chain
(
    const std::vector<size_t>& chain,
    size_t st
) const
{
    std::vector<size_t> cch(1, chain[0]);
    std::vector<size_t> times(1, st);

    for(size_t j = 1; j < chain.size(); ++j)
    {
        if(chain[j] != chain[j - 1])
        {
            cch.push_back(chain[j]);
            times.push_back(st + j);
        }
    }

    times.push_back(st + chain.size() - 1);
    return std::make_pair(cch, times);
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

Activity_sequence kjb::bbb::sample(const As_prior& prior)
{
    const Activity_library& lib = prior.library();

    Activity_sequence aseq(prior.role());

    // sample start and end
    const Intentional_activity& parent = prior.parent();
    size_t start = parent.start();
    size_t end = parent.end();

    //// get markov chain of indices and condense chain
    typedef std::vector<size_t> Idx_vector;
    Idx_vector ind_act_idxs = prior.sample_markov_chain(end - start + 1);
    As_prior::Chain_pair cch = prior.condense_chain(ind_act_idxs, start);
    const Idx_vector& act_idxs = cch.first;
    const Idx_vector& act_times = cch.second;

    //// generate activities
    size_t I = act_idxs.size();
    for(size_t i = 0; i < I; i++)
    {
        std::string name = lib.activity_name(act_idxs[i]);
        const Traj_set& trajs = prior.trajectories();
        size_t s = act_times[i];
        size_t e = act_times[i + 1];

        if(lib.is_intentional(name))
        {
            Parameter_prior& param_prior = prior.parameter_prior();
            param_prior.set_name(name);
            param_prior.set_parent(parent);

            Vector params = sample(param_prior);
            aseq.add(Intentional_activity(name, s, e, params, trajs));
        }
        else
        {
            //const Description& desc = prior.description();
            const Traj_set& trajs = prior.trajectories();

            // fill with zeroed-out trajectory for now
            Trajectory trajectory;
            std::vector<Vector> zeros(2, Vector((int)(e - s + 1), 0.0));
            trajectory.set_dimensions(s, zeros.begin(), zeros.end());
            aseq.add(Physical_activity(name, trajectory, trajs));
        }
    }

    return aseq;
}

