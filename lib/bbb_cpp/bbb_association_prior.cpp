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

#include "bbb_cpp/bbb_association_prior.h"
#include "bbb_cpp/bbb_association.h"
#include "prob_cpp/prob_distribution.h"
#include "prob_cpp/prob_sample.h"

#include <vector>
#include <string>

using namespace kjb;
using namespace kjb::bbb;

Association kjb::bbb::sample(const Association_prior& prior)
{
    const Activity_library& lib = prior.library();

    size_t nc = prior.parent().trajectories().size();
    size_t alpha = lib.group_concentration(prior.parent().name());

    // sample tables
    Chinese_restaurant_process crp(alpha, nc);
    Crp::Type tables = sample(crp);

    // sample roles
    std::vector<double> ps = lib.role_distribution(prior.parent().name());
    Categorical_distribution<size_t> role_dist(ps, 0);
    std::vector<std::string> roles(tables.size());
    for(size_t k = 0; k < roles.size(); k++)
    {
        size_t r = sample(role_dist);
        roles[k] = lib.role_name(r);
    }

    Association ass(prior.parent().trajectories());
    ass.set(tables.begin(), tables.end(), roles.begin());

    return ass;
}

