/* =========================================================================== *
|
|  Copyright (c) 1994-2008 by Kobus Barnard (author).
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
| Authors:
|     Jinyan Guan
|
* =========================================================================== */

/* $Id: drift_sampler.cpp 22559 2019-06-09 00:02:37Z kobus $ */

#include <prob_cpp/prob_distribution.h>
#include <prob_cpp/prob_sample.h>
#include <gp_cpp/gp_base.h>
#include <m_cpp/m_vector.h>

#include <vector>
#include <string>

#include "dbn_cpp/drift_sampler.h"
#include "dbn_cpp/linear_state_space.h"

using namespace kjb;
using namespace kjb::ties;

Drift_sampler::Pred_vec Drift_sampler::make_predictive
(
    const Linear_state_space& lss
) const
{
    size_t num_oscillators = lss.num_oscillators();
    size_t num_params = param_length(num_oscillators, lss.use_modal());
    // control outputs
    const std::vector<Vector>& c_outputs = model_.c_outputs;
    KJB(ASSERT(c_outputs.size() == num_params));

    // inputs
    gp::Inputs::const_iterator first_in = lss.get_gp_inputs().begin();
    gp::Inputs::const_iterator last_in = lss.get_gp_inputs().end();

    Pred_vec preds;
    for(size_t i = 0; i < c_outputs.size(); i++)
    {
        assert(c_outputs[i].size() == num_control_pts_);
        // predictive distribution
        preds.push_back(
            Pred(lss.get_mean_function(i),
                 lss.get_covariance_function(i),
                 c_inputs_.begin(),
                 c_inputs_.end(),
                 c_outputs[i].begin(),
                 c_outputs[i].end(),
                 first_in,
                 last_in));
    }
    return preds;
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

std::vector<Drift_sampler::Pred_vec> 
Drift_sampler::make_control_predictives(const Linear_state_space& lss) const
{
    const std::vector<Vector>& c_outs_all = model_.c_outputs;
    size_t num_oscillators = lss.num_oscillators();
    size_t num_params = param_length(num_oscillators, lss.use_modal());
    KJB(ASSERT(c_outs_all.size() == num_params));

    std::vector<Pred_vec> c_preds;
    c_preds.reserve(num_control_pts_);

    for(size_t n = 0; n < num_params; n++)
    {
        Drift_sampler::Pred_vec preds;
        gp::Inputs trins(num_control_pts_ - 1);
        Vector trouts(num_control_pts_ - 1);
        Vector tein;

        const Vector& c_outs = c_outs_all[n]; 
        preds.reserve(num_control_pts_);
        for(size_t i = 0; i < num_control_pts_; i++)
        {
            using std::copy;
            // train inputs are control inputs minus current one
            copy(c_inputs_.begin(), c_inputs_.begin() + i, trins.begin());
            copy(c_inputs_.begin() + i + 1, c_inputs_.end(), trins.begin() + i);

            // Train outputs are control outputs minus current one
            copy(c_outs.begin(), c_outs.begin() + i, trouts.begin());
            copy(c_outs.begin() + i + 1, c_outs.end(), trouts.begin() + i);

            // Test input is current input
            tein = c_inputs_[i];

            // Create predictive
            preds.push_back(
                Pred(lss.get_mean_function(n),
                     lss.get_covariance_function(n),
                     trins.begin(),
                     trins.end(),
                     trouts.begin(),
                     trouts.end(),
                     &tein,
                     &tein + 1));
        }
        c_preds.push_back(preds);
    }

    return c_preds;
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

ergo::mh_proposal_result Drift_sampler::propose
(
    const Model& m,
    Model& m_p
) const
{
    using namespace std;
    m_p = m;
    Linear_state_space& out = m_p.outputs;
    std::vector<Vector>& couts = m_p.c_outputs;
    size_t num_clo_params = couts.size();
    size_t num_total_params = num_clo_params * num_control_pts_;
    if(sample_index_ == num_total_params) 
    {
        sample_index_ = 0;
    }

    string move_name; 
    //log_ofs_ << "XXXXX" << sample_index_ << std::endl;
    size_t d = sample_index_ % num_clo_params;
    // DEBUG TODO SKIP THE COUPLING TERM
    /*if(d == 2 || d == 5)
    {
        sample_index_++;
        move_name = "-skip-"; 
        return ergo::mh_proposal_result(0.0, 0.0, move_name);
    }*/
    //log_ofs_ << "XXXXX d " << d << std::endl;
    assert(sample_index_ < num_total_params);
    std::stringstream sst;
    //size_t& cur_c = cur_c_[d];
    int cur_c = sample_index_ / num_clo_params;
    //log_ofs_ << "XXXXX cur_c " << cur_c << std::endl;
    sst << "-control-" << cur_c << "-";
    move_name = sst.str();

    assert(couts[d].size() == num_control_pts_);

    // sample current control point
    Vector trouts(num_control_pts_ - 1);
    copy(couts[d].begin(), couts[d].begin() + cur_c, trouts.begin());
    copy(couts[d].begin() + cur_c + 1, couts[d].end(), trouts.begin() + cur_c);

    c_preds_[d][cur_c].set_train_outputs(trouts.begin(), trouts.end());
    couts[d][cur_c] = gp::sample(c_preds_[d][cur_c])[0];

    // sample real outputs
    pred_[d].set_train_outputs(couts[d].begin(), couts[d].end());
    Vector params = gp::sample(pred_[d]);
    set_params(params, d, m_p.outputs);

    cur_c++;
    // Reset the cur_c_ to 0 after a cycle
    if(cur_c == num_control_pts_) cur_c = 0; 

    sample_index_++;
    // Symmetric proposal
    return ergo::mh_proposal_result(0.0, 0.0, move_name);
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

void Drift_sampler::set_params
(
    const Vector& params, 
    size_t index, 
    Linear_state_space& lss
) const
{
    Coupled_oscillator_v& clos = lss.coupled_oscillators();
    IFTD(index < clos.front().get_params().size(), Illegal_argument, 
            "index must be less than %d", (clos.front().num_params()));
    if(clos.empty() || params.empty()) return;

    // Loop through all the times 
    for(size_t i = 0; i < params.size(); i++)
    {
        clos[i].set_param(index, params[i]);
    }
    lss.changed_index() = 0;
}
