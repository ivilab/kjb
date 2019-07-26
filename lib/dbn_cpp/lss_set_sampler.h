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

/* $Id: lss_set_sampler.h 22559 2019-06-09 00:02:37Z kobus $ */

#ifndef KJB_TIES_LSS_SAMPLER_H
#define KJB_TIES_LSS_SAMPLER_H

#ifdef KJB_HAVE_ERGO
#include <ergo/hmc.h>
#include <ergo/record.h>
#else 
#error "ergo library is not available"
#endif

#include "dbn_cpp/lss_set.h"
#include "dbn_cpp/posterior.h"
#include "dbn_cpp/likelihood.h"
#include "dbn_cpp/prior.h"
#include "dbn_cpp/experiment.h"
#include "dbn_cpp/gradient.h"
#include "dbn_cpp/adapter.h"
#include "dbn_cpp/util.h"
#include "dbn_cpp/thread_worker.h"
#include "dbn_cpp/proposer.h"

#include <vector>

namespace kjb {
namespace ties {

typedef Shared_gradient<Lss_set_posterior, Shared_param_adapter> Shared_param_gradient;
typedef Shared_gradient_mt<Lss_set_posterior_mt, Shared_obs_noise_adapter> Obs_noise_gradient;
typedef Shared_gradient_mt<Lss_set_posterior_mt, Shared_obs_coef_adapter> Obs_coef_gradient;
typedef Shared_gradient<Independent_blr_posterior, Shared_param_adapter> Shared_ind_blr_prior_gradient;
typedef Shared_gradient<Shared_gp_scale_posterior, Shared_gp_scale_adapter> Shared_scale_gradient;

class Lss_set_sampler
{
public:
    /**
     * @brief   Creates a Lss_set_sampler based on experiment options
     */
    Lss_set_sampler
    (   
        const Ties_experiment& exp
    ); 

    /** @brief   Returns the lss_set_ */
    Lss_set& lss_set() { return lss_set_; }

    /** @brief   Returns the data */
    const std::vector<Data>& data() const { return data_; }

    /** @brief   Creates a mh step for the shared priors. */
    template<typename Func, typename Proposer>
    ergo::mh_step<Lss_set> shared_mh_step
    (
        const Func& func,
        const Proposer& proposer
    );

    /** @brief   Creates a hmc step for the shared priors. */
    template<typename Func, typename Adapter, typename Grad>
    ergo::hmc_step<Lss_set> shared_hmc_step
    ( 
         const Func& func,
         const Adapter& adapter,
         const Grad& grad,
         const std::string& name
    );

    /** @brief   Creates a hmc step for the shared scales. */
    ergo::hmc_step<Lss_set> shared_scale_hmc_step
    (
        const Shared_gp_scale_posterior& pos
    );

    /** @brief   Creates a mh step for the observe coefs. */
    ergo::mh_step<Lss_set> obs_coefs_mh_step
    (
        const Lss_set_obs_coef_proposer& prop
    ); 

    /** @brief   Creates a mh step for the observe coefs. */
    ergo::hmc_step<Lss_set> obs_coefs_hmc_step
    (
        const Shared_obs_coef_adapter& adapter
    );

    /** @brief   Creates a hmc step for the observe sigmas. */
    ergo::hmc_step<Lss_set> noise_sigma_hmc_step();

    /** @brief   Creates a mh step for the noise sigma. */
    ergo::mh_step<Lss_set> noise_sigma_mh_step
    (
        const Lss_set_pred_posterior_mh& evaluator,
        const Lss_set_noise_sigma_proposer& noise_proposer
    );
    /**
     * @brief   Learning the shared parameters by marginalizing out the 
     *          couple-specific parameters
     */
    Lss_set train_marginal(size_t num_iterations);

    /**
     * @brief   Learning the shared parameters (moderator coefficients and
     *          variance) by sampling from the joint distribution
     */
    Lss_set train_model(size_t num_iterations);

    /**
     * @brief   Learning the noise sigma 
     *
     */
    //Lss_set train_ind_clo(size_t num_iterations);

    /**
     * @brief   Learning the shared parameters by using the predictive
     *          distribution of the last 20% time points as the likelihood
     *          of the shared params (Use HMC to update the shared params)
     */
    Lss_set learn_pred_v2(size_t num_iterations);

    /**
     * @brief   Learning the shared parameters by using the predictive
     *          distribution of the last 20% time points as the likelihood
     *          of the shared params (Use MH to update the shared params)
     */
    Lss_set learn_pred_mh(size_t num_iterations);

    /**
     * @brief   Learning the shared GP parameters by optmizing the predictive
     *          distribution of the last 20% time points of each couple
     */
    Lss_set learn_gp(size_t num_iterations);

    /**
     * @brief   Testing the shared parameters by fitting the couple-specific
     *          params.
     */
    Lss_set test_pred(size_t num_iterations);

    /**
     * @brief   Testing the shared parameters by fitting the couple-specific
     *          params.
     */
    Lss_set test_model(size_t num_iterations);

    /**
     * @brief   Testing the shared GP parameters by fitting the couple-specific
     *          params on held-out data.
     */
    Lss_set test_gp(size_t num_iterations);

    /**
     * @brief   Compute the predictive probability of a Lss_set 
     */
    double pred_prob(const Lss_set& lsss, size_t num_samples)
    {
        return sum_predictive_prob(posteriors_, 
                                   lsss.lss_vec(), 
                                   num_samples);
    }

    /**
     * @brief   Returns samples of the predicted states.
     */
    std::vector<std::vector<Linear_state_space> > pred_samples
    (
        const Lss_set& lsss, 
        size_t num_samples
    );

    /** 
     * @brief   Returns a vector of output dirs for individual
     *          Linear_state_space
     */
    void generate_output_dirs(const std::string& out_dir);

private:

    size_t param_length_;
    std::vector<size_t> ids_;
    std::vector<Data> data_;
    Group_map group_map_;

    State_vec init_states_;
    Double_vv times_; 

    Lss_set lss_set_;

    // likelihoods and priors
    Init_prior init_prior_;
    Independent_blr_prior ind_blr_prior_;
    Shared_scale_prior lss_scale_prior_;
    Shared_noise_prior noise_prior_;
    //Gaussian_mixture_prior group_prior_;
    Cluster_prior cluster_prior_;
    Shared_lss_prior shared_prior_;

    std::vector<Likelihood> likelihoods_;
    std::vector<Posterior> posteriors_;
    Lss_set_posterior shared_posterior_;
    Lss_set_posterior_mt shared_posterior_mt_;
    Independent_blr_posterior prior_pos_;

    Init_state_adapter state_adapter_;
    
    std::ofstream shared_log_fs_;
    std::ofstream shared_bst_fs_;
    std::ofstream bst_fs_;
    ergo::best_target_recorder<std::ostream_iterator<double> >
                                                  best_target_recorder_;

    Lss_set_recorder record_sample_;
    Lss_set_recorder record_proposal_;

    const Ties_experiment& exp_;
    std::vector<double> state_hmc_sizes_;
    std::vector<double> person_hmc_sizes_;

    std::vector<std::string> lss_out_dirs_;

}; // class Lss_set_sampler

template<typename Func, typename Proposer>
ergo::mh_step<Lss_set> Lss_set_sampler::shared_mh_step
(
     const Func& func,
     const Proposer& proposer
)
{
    // MH shared param step
    ergo::mh_step<Lss_set> shared_prior_mh(func, proposer);

    // MH shared param recorders 
    shared_prior_mh.store_proposed(exp_.run.write_proposals);
    if(!exp_.not_record)
    {
        shared_prior_mh.add_recorder(best_target_recorder_);
        shared_prior_mh.add_recorder(
                ergo::make_mh_detail_recorder(
                    std::ostream_iterator<ergo::step_detail>(
                                                shared_log_fs_, "\n")));
    }
    return shared_prior_mh;
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

template<typename Func, typename Adapter, typename Grad>
ergo::hmc_step<Lss_set> Lss_set_sampler::shared_hmc_step
(
     const Func& func,
     const Adapter& adapter,
     const Grad& grad,
     const std::string& name
)
{
    // HMC shared param step
    size_t num_params = adapter.size(&lss_set_);
    ergo::hmc_step<Lss_set> shared_hmc(
                       adapter, 
                       func,
                       grad,
                       std::vector<double>(num_params, 
                                           exp_.run.shared_hmc_size),
                       exp_.run.shared_num_leapfrog_steps,
                       0.0);
    // HMC recorders 
    shared_hmc.store_proposed(exp_.run.write_proposals);
    if(!exp_.not_record)
    {
        shared_hmc.add_recorder(best_target_recorder_);
        shared_hmc.add_recorder(
                ergo::make_hmc_detail_recorder(
             std::ostream_iterator<ergo::step_detail>(shared_log_fs_, "\n")));
    }
    shared_hmc.rename(name);
    return shared_hmc;
}

}} // namespace kjb::ties

#endif // KJB_TIES_LSS_SAMPLER_H

