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

#include "bbb_cpp/bbb_trajectory_prior.h"
#include "l_cpp/l_exception.h"
#include "gp_cpp/gp_base.h"
#include "gp_cpp/gp_predictive.h"
#include "gp_cpp/gp_mean.h"
#include "gp_cpp/gp_covariance.h"
#include "m_cpp/m_vector.h"

#include <vector>
#include <algorithm>
#include <boost/bind.hpp>
#include <boost/ref.hpp>

using namespace kjb;
using namespace kjb::bbb;

void Trajectory_prior::update_priors() const
{
    IFT(!name_.empty() && end_ != start_, Runtime_error,
        "Cannot compute trajectory prior; name and times not set.");

    IFT(!mu_s_.empty() && !mu_e_.empty(), Runtime_error,
        "Cannot compute trajectory prior; endpoint means not set.");

    if(!priors_dirty_) return;

    // initialize priors if necessary
    if(priors_.empty())
    {
        Prior empty_prior = gp::make_predictive_nl(
                                            gp::Zero(),
                                            gp::Sqex(1.0, 1.0),
                                            gp::Inputs(2),
                                            Vector(2),
                                            gp::Inputs(1));


        priors_.resize(dim_, empty_prior);
    }

    // update GP inputs if necessary
    size_t len = end_ - start_ + 1;
    if(std::distance(priors_[0].tein_begin(), priors_[0].tein_end()) != len)
    {
        typedef gp::Inputs::const_iterator Input_it;

        // set test inputs
        gp::Inputs teins = gp::make_inputs(1, len);
        Input_it fin = teins.begin();
        Input_it lin = teins.end();
        std::for_each(
            priors_.begin(),
            priors_.end(),
            boost::bind(&Prior::set_test_inputs<Input_it>, _1, fin, lin));

        // set train inputs
        gp::Inputs trins(2);
        trins.front() = teins.front();
        trins.back() = teins.back();
        fin = trins.begin();
        lin = trins.end();

        std::for_each(
            priors_.begin(),
            priors_.end(),
            boost::bind(&Prior::set_train_inputs<Input_it>, _1, fin, lin));
    }

    // update train outputs
    Vector trouts(2, 0.0);
    for(size_t d = 0; d < dim_; d++)
    {
        trouts.front() = mu_s_[d];
        trouts.back() = mu_e_[d];

        priors_[d].set_train_outputs(trouts.begin(), trouts.end());
    }

    // update covariance function if necessary
    gp::Sqex sqex = lib_.trajectory_kernel(name_);
    std::for_each(
        priors_.begin(),
        priors_.end(),
        boost::bind(&Prior::set_covariance_function, _1, sqex));

    priors_dirty_ = false;
}

