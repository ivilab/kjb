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
#include "bbb_cpp/bbb_description_prior.h"
#include "bbb_cpp/bbb_activity_sequence_prior.h"
#include "bbb_cpp/bbb_association_prior.h"
#include "bbb_cpp/bbb_trajectory_prior.h"
#include "bbb_cpp/bbb_description.h"
#include "bbb_cpp/bbb_intentional_activity.h"
#include "bbb_cpp/bbb_physical_activity.h"
#include "bbb_cpp/bbb_activity_sequence.h"
#include "bbb_cpp/bbb_endpoints.h"
#include "m_cpp/m_vector.h"
#include "m_cpp/m_matrix.h"
#include "prob_cpp/prob_distribution.h"
#include "prob_cpp/prob_sample.h"

#include <boost/variant.hpp>
#include <boost/tuple/tuple.hpp>

using namespace kjb;
using namespace kjb::bbb;

double Description_prior::operator()(const Description& desc) const
{
    return 0.0;
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

void Sample_tree::operator()(const Intentional_activity& root)
{
    // sample association
    Association_prior& ass_prior = prior_.association_prior();
    ass_prior.set_parent(root);
    Association ass = sample(ass_prior);

    // sample sequences
    As_prior& as_prior = prior_.activity_sequence_prior();
    size_t num_groups = ass.num_groups();
    for(size_t k = 0; k < num_groups; k++)
    {
        // sample activity sequnece
        const Group& grp = ass.group(k);
        as_prior.set_role(grp.role());
        as_prior.set_parent(root);
        as_prior.set_trajectories(grp.trajectories());
        Activity_sequence aseq = sample(as_prior);

        // insert into description
        typedef const Activity_sequence* Asp;
        Asp aseq_p = description_.add_child_sequence(&root, aseq);

        // sample sub-trees recursively
        size_t num_acts = aseq_p->size();
        for(size_t j = 0; j < num_acts; j++)
        {
            boost::apply_visitor(*this, aseq_p->activity(j));
        }
    }
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

Description kjb::bbb::sample(const Description_prior& prior)
{
    Description description(prior.root_activity());
    const Intentional_activity& root = description.root_activity();
    const Activity_library& lib = prior.library();

    // sample activities
    Sample_tree sample_tree(prior, description);
    sample_tree(root);

    //// sample endpoints
    Endpoint_set epts_info;
    trajectory_endpoints(epts_info, description.root_activity(), description);

    // first get mean and covariance
    Vector ep_mux;
    Vector ep_muy;
    std::vector<size_t> tgts;
    boost::tie(ep_mux, ep_muy, tgts) = endpoint_mean(epts_info, description, lib);
    Matrix ep_cov = endpoint_covariance(epts_info, lib);

    // get conditional prior
    endpoint_distribution(ep_mux, ep_muy, ep_cov, tgts);

    // sample endpoints x- and y-coordinates
    std::vector<Vector> enddims(2);
    MV_normal_distribution P(ep_mux, ep_cov);
    enddims[0] = sample(P);

    P.set_mean(ep_muy);
    enddims[1] = sample(P);

    std::vector<Vector> endpoints = get_transpose(enddims);

    // sample trajectories
    const size_t num_epts = epts_info.size();
    Trajectory_prior& traj_prior = prior.trajectory_prior();
    for(size_t i = 0; i < num_epts; i++)
    {
        const Endpoint& ei = epts_info[i];
        if(ei.right_p)
        {
            const Endpoint& nei = epts_info[i + 1];
            ASSERT(ei.right_p == nei.left_p);

            Physical_activity& act = const_cast<Physical_activity&>(*ei.right_p);
            traj_prior.set_name(act.name());
            traj_prior.set_start(act.start());
            traj_prior.set_end(act.end());
            traj_prior.set_endpoint_means(endpoints[i], endpoints[i + 1]);
            act.set_trajectory(sample(traj_prior));
        }
    }

    return description;
}

