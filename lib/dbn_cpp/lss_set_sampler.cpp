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

/* $Id: lss_set_sampler.cpp 22559 2019-06-09 00:02:37Z kobus $ */

#ifdef KJB_HAVE_ERGO

#include <l/l_sys_time.h>
#include <opt_cpp/opt_pgpe.h>

#include <ergo/hmc.h>
#include <ergo/record.h>
#else 
#error "ergo library is not available"
#endif

#include "dbn_cpp/lss_set.h"
#include "dbn_cpp/lss_set_sampler.h"
#include "dbn_cpp/gradient.h"
#include "dbn_cpp/adapter.h"
#include "dbn_cpp/posterior.h"
#include "dbn_cpp/experiment.h"
#include "dbn_cpp/data.h"
#include "dbn_cpp/util.h"
#include "dbn_cpp/time_util.h"
#include "dbn_cpp/marginal_likelihood.h"
#include "dbn_cpp/proposer.h"

#include <vector>
#include <string>
#include <iterator>
#include <algorithm>
#include <limits>

#ifdef KJB_HAVE_BOOST
#include <boost/format.hpp>
#include <boost/thread.hpp>
#include <boost/bind.hpp>
#include <boost/exception_ptr.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/filesystem.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/progress.hpp>
#else 
#error "boost library is not available"
#endif

#define BURNIN_ITER 0

using namespace kjb;
using namespace kjb::ties;

Lss_set_sampler::Lss_set_sampler
(
    const Ties_experiment& exp
) : 
    param_length_(param_length(exp.lss.num_oscillators)),
    ids_(parse_list(exp.data.id_list_fp)),
    data_(parse_data(exp.data.data_dp, 
                     exp.data.id_list_fp,
                     exp.data.grouping_var)),
    init_states_(estimate_init_states(
                    data_, 
                    exp.likelihood.training_percent,
                    exp.likelihood.obs_names.front(),
                    exp.lss.polynomial_degree)),
    lss_set_(ids_,
            exp.lss_set.get_all_moderators(
                    exp.lss.ignore_clo,
                    exp.lss.num_oscillators,
                    exp.lss.num_params),
            init_states_,
            data_,
            exp.prior.gp_scale,
            exp.prior.clo_sigma,
            exp.prior.poly_sigma,
            exp.prior.outcome_sigma,
            exp.likelihood.obs_names,
            exp.prior.fixed_clo,
            exp.likelihood.training_percent,
            exp.lss.num_oscillators,
            exp.lss.init_period,
            exp.lss.init_damping,
            Vector((int)exp.likelihood.obs_names.size(), 
                          exp.likelihood.init_noise_sigma),
            exp.lss.drift,
            exp.cluster.num_groups,
            exp.lss.polynomial_degree, 
            exp.data.outcomes,
            exp.lss.ignore_clo,
            exp.data.grouping_var,
            exp.data.get_group_info_fp(),
            exp.lss.use_modal),
    init_prior_(exp.prior.init_state_mean, 
                exp.prior.init_state_sigma, 
                exp.lss.num_oscillators),
    ind_blr_prior_(build_vector(
                       lss_set_.clo_param_size(), 
                           exp.prior.clo_sigma_shape,
                       lss_set_.num_polynomial_params(), 
                           exp.prior.poly_sigma_shape,
                       lss_set_.num_outcomes(),
                           exp.prior.outcome_sigma_shape),
                   build_vector(
                       lss_set_.clo_param_size(), 
                           exp.prior.clo_sigma_scale,
                       lss_set_.num_polynomial_params(), 
                           exp.prior.poly_sigma_scale,
                       lss_set_.num_outcomes(),
                           exp.prior.outcome_sigma_scale),
                   lss_set_.pred_coefs(),
                   get_coef_covariance(lss_set_.pred_coefs(),
                      exp.lss_set.mod_coef_prior_sigma),
                   lss_set_.get_X_ts(),
                   lss_set_.get_X_t_Xs(),
                   exp.prior.variance_prior_type,
                   exp.run.sample_cluster),
    lss_scale_prior_(std::vector<double>(
                         lss_set_.clo_param_size(), 
                         exp.prior.gp_scale_shape), 
                     std::vector<double>(
                         lss_set_.clo_param_size() , 
                         exp.prior.gp_scale_scale)),
    noise_prior_(std::vector<double>(exp.likelihood.obs_names.size(), 
                                     exp.prior.obs_noise_shape),
                std::vector<double>(exp.likelihood.obs_names.size(), 
                                     exp.prior.obs_noise_scale)),
    shared_prior_(ind_blr_prior_, lss_scale_prior_, cluster_prior_),
    shared_posterior_(shared_prior_, noise_prior_, posteriors_),
    shared_posterior_mt_(shared_prior_, noise_prior_, posteriors_, 2),
    prior_pos_(ind_blr_prior_),
    state_adapter_(),
    best_target_recorder_(std::ostream_iterator<double>(shared_bst_fs_, "\n")),
    record_sample_(exp.out_dp + "/samples"),
    record_proposal_(exp.out_dp + "/proposals"),
    exp_(exp),
    state_hmc_sizes_(ids_.size()),
    person_hmc_sizes_(ids_.size()),
    lss_out_dirs_(ids_.size()),
    cluster_prior_(exp.cluster.group_lambda, exp.cluster.num_groups)
{
    using std::vector;
    if(!exp_.not_record)
    {
        std::string shared_log_fp = exp.out_dp + "/shared_sample_log.txt";
        std::string shared_bst_fp = exp.out_dp + "/shared_ll.txt";
        std::string bst_fp = exp.out_dp + "/ll.txt";
        shared_log_fs_.open(shared_log_fp.c_str());
        shared_bst_fs_.open(shared_bst_fp.c_str());
        bst_fs_.open(bst_fp.c_str(), std::ios::app);
        IFTD(shared_log_fs_.is_open(), IO_error, "can't open file %s", 
                            (shared_log_fp.c_str()));
        IFTD(shared_bst_fs_.is_open(), IO_error, "can't open file %s",
                            (shared_bst_fp.c_str()));
        IFTD(bst_fs_.is_open(), IO_error, "can't open file %s", 
                            (bst_fp.c_str()));
    }

    // Parse in data
    size_t num_lss = ids_.size();
    size_t num_params = lss_set_.clo_param_size();

    // If the initial state file is specified 
    if(exp_.lss_state_dp != "")
    {
        for(size_t i = 0; i < num_lss; i++)
        {
            boost::format in_fmt(exp_.lss_state_dp + "/%04d");
            std::string in_fp = (in_fmt % ids_[i]).str();
            Linear_state_space state_lss;
            state_lss.read(in_fp);
            lss_set_.lss_vec()[i].init_state() = state_lss.init_state();
        }
    }

    // Read in the group_map information 
    if(kjb_c::is_file(exp_.data.get_group_info_fp().c_str()))
    {
        std::ifstream ifs(exp_.data.get_group_info_fp().c_str());
        ifs >> group_map_; 
    }

    // Read in the model lss_set
    if(exp.run.read_model)
    {
        std::vector<Vector> pred_coefs(lss_set_.pred_coefs());
        try
        {
            std::cout << " read model from " << exp.run.in_dp << "\n";
            // get the group info
            lss_set_.read(exp.run.in_dp, data_, 
                          group_map_);
        }
        catch(Exception& err)
        {
            std::cerr << "Error in reading model Lss_set!\n";
            std::cerr << "Creating a Lss_set \n";
            // update the linear_state_space vector
            std::copy(pred_coefs.begin(), pred_coefs.end(), 
                   lss_set_.pred_coefs().begin());
            Vector noise_sigmas((int)exp.likelihood.obs_names.size(), 
                              exp.likelihood.init_noise_sigma);
            Double_v gp_scales;
            Double_v gp_sigvars(lss_set_.variances().begin(), 
                                lss_set_.variances().end());
            if(num_params > 0 && exp.lss.drift)
            {
                gp_scales.resize(num_params, exp.prior.gp_scale);
            }
            lss_set_.init_lss(init_states_, 
                              data_,
                              gp_scales,  
                              gp_sigvars,
                              exp.likelihood.obs_names,
                              exp.lss.num_oscillators,
                              exp.likelihood.training_percent,
                              exp.lss.init_period,
                              exp.lss.init_damping,
                              noise_sigmas,
                              exp.lss.polynomial_degree,
                              exp.lss.drift);
        }
    }


    // If the prior is specified
    if(exp.prior.prior_fp != "")
    {
        std::cout << "Read in prior from file: " << exp.prior.prior_fp << "\n";
        lss_set_.read_shared_params(exp.prior.prior_fp);
    }

    // If the obs_coef_fp is specified 
    if(exp.run.obs_coef_fp != "")
    {
        lss_set_.parse_obs_coef(exp.run.obs_coef_fp);
    }

    // construct posteriors
    for(size_t i = 0; i < num_lss; i++)
    {
        // likelihood
        Likelihood likelihood(data_[i], exp.run.num_sampled_length);
        likelihoods_.push_back(likelihood);
        bool use_init_prior = true;
        bool use_clo_ind_prior = exp_.run.fit_ind_clo || 
                                    //exp_.run.learn_cluster ||
                                    exp_.lss.drift ?
                                 false : true;
        bool use_likelihood = true;
        bool use_pred = false;
        bool use_drift = exp.lss.drift ? true : false;
        //bool use_group_prior = exp_.run.learn_cluster ? true : false;
        bool use_group_prior = false;
        posteriors_.push_back(Posterior(likelihood, 
                                        init_prior_,
                                        use_init_prior,
                                        use_clo_ind_prior, 
                                        use_group_prior,
                                        use_drift, 
                                        use_likelihood,
                                        use_pred));
    }

    // TODO we do not combine group model with GP model right now 
    shared_prior_.learn_scale = 
        exp_.lss.drift && exp_.prior.prior_fp == "" ? true : false;

    shared_prior_.learn_clo = (exp_.prior.fixed_clo || 
                               //exp_.run.learn_cluster ||
                               exp_.run.fit_ind_clo ) ?  false : true;
    // setting the prior for each Linear state space
    if(exp_.run.fit_ind_clo || 
            lss_set_.fixed_clo() || 
            exp.lss.drift)
            //exp_.run.learn_cluster)
    {
        std::for_each(posteriors_.begin(), posteriors_.end(), 
                      boost::bind(&Posterior::use_clo_ind_prior, _1, false));
    }

    /*if(exp_.run.learn_cluster)
    {
        if(exp_.cluster.collapsed_gibbs)
        {
            std::for_each(posteriors_.begin(), posteriors_.end(), 
                      boost::bind(&Posterior::use_clo_group_prior, _1, false));
        }
        else
        {
            std::for_each(posteriors_.begin(), posteriors_.end(), 
                      boost::bind(&Posterior::use_clo_group_prior, _1, true));
        }
    }*/

    // Set the prior for independent CLO model
    if(exp_.run.fit_ind_clo)
    {
        if(exp_.prior.prior_fp != "")
        {
            std::for_each(posteriors_.begin(), posteriors_.end(), 
                          boost::bind(&Posterior::use_clo_ind_prior, _1, true));
        }
        else
        {
            std::for_each(posteriors_.begin(), posteriors_.end(), 
                          boost::bind(&Posterior::use_clo_ind_prior, _1, false));
        }
        // Disable use_pred for each posteriors
        std::for_each(posteriors_.begin(), posteriors_.end(), 
                      boost::bind(&Posterior::use_pred, _1, false));
    }

    // drift prior
    if(exp_.lss.drift)
    {
        std::for_each(posteriors_.begin(), posteriors_.end(), 
                  boost::bind(&Posterior::use_clo_drift_prior, _1, true));
    }
    else
    {
        std::for_each(posteriors_.begin(), posteriors_.end(), 
                  boost::bind(&Posterior::use_clo_drift_prior, _1, false));
    }


    // If sampling over the noise sigma
    if(exp_.sample_noise_sigma || exp_.run.fit_ind_clo)
    {
        shared_posterior_.set_use_noise_prior(true);
        shared_posterior_mt_.set_use_noise_prior(true);
    }
    
    // Compute the hmc step sizes
    for(size_t i = 0; i < num_lss; i++)
    {
        double state_step_size = exp.run.state_hmc_size / 
                            std::pow(data_[i].times.size(), 3.0);
        double person_step_size = exp.run.person_hmc_size / 
                            std::pow(data_[i].times.size(), 3.0);

        state_hmc_sizes_[i] = state_step_size;
        person_hmc_sizes_[i] = person_step_size;
    }

    // randomize the starting cluster assignment
    if(exp_.run.sample_cluster && !exp_.run.read_model)
    {
        std::cout << "Sample cluster\n";

        // update cluster weights from prior 
        Vector weights = cluster_prior_.sample_cluster_weights_from_prior();
        assert(weights.size() == lss_set_.num_groups());
        for(size_t g = 0; g < lss_set_.num_groups(); g++)
        {
            lss_set_.group_weight(g) = weights[g];
        }

        // init cluster assigns
        std::vector<size_t> cluster_assigns = get_cluster_assignments(lss_set_);
        cluster_prior_.init_num_data_per_cluster(cluster_assigns);
        cluster_prior_.sample_cluster_assignments(lss_set_, true);
    }
    cluster_prior_.sample_cluster_assignments(lss_set_, false);

    // set up the output dirs for lss_out_dirs_
    generate_output_dirs(exp_.out_dp);

}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

ergo::hmc_step<Lss_set> Lss_set_sampler::shared_scale_hmc_step
(
    const Shared_gp_scale_posterior& pos
)
{
    // HMC shared scale step
    Shared_gp_scale_adapter shared_scale_adapter;
    size_t num_params = shared_scale_adapter.size(&lss_set_);
    ergo::hmc_step<Lss_set> shared_scale_hmc(
                        shared_scale_adapter, 
                        pos,
                        Shared_scale_gradient(
                            pos,
                            shared_scale_adapter,
                            std::vector<double>(num_params,
                                               exp_.run.shared_grad_size),
                            exp_.run.estimate_grad_step),
                        std::vector<double>(num_params, 
                                            exp_.run.shared_hmc_size),
                        exp_.run.shared_num_leapfrog_steps,
                        0.0);

    // HMC recorders 
    shared_scale_hmc.store_proposed(exp_.run.write_proposals);
    if(!exp_.not_record)
    {
        shared_scale_hmc.add_recorder(best_target_recorder_);
        shared_scale_hmc.add_recorder(
                ergo::make_hmc_detail_recorder(
             std::ostream_iterator<ergo::step_detail>(shared_log_fs_, "\n")));
    }
    shared_scale_hmc.rename(std::string("shared-scale-hmc"));

    // make sure the scale is positive
    std::vector<double> ubs(num_params);
    std::vector<double> lbs(num_params);
    fill(lbs.begin(), lbs.end(), FLT_EPSILON);
    fill(ubs.begin(), ubs.end(), std::numeric_limits<double>::max());
    shared_scale_hmc.set_lower_bounds(lbs);
    shared_scale_hmc.set_upper_bounds(ubs);
    return shared_scale_hmc;
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

ergo::mh_step<Lss_set> Lss_set_sampler::obs_coefs_mh_step
(
    const Lss_set_obs_coef_proposer& obs_proposer
)
{
    // MH shared obs_coefs step
    ergo::mh_step<Lss_set> shared_obs_coefs_mh(shared_posterior_mt_, 
                                               obs_proposer); 

    // MH shared obs_coefs recorders 
    shared_obs_coefs_mh.store_proposed(exp_.run.write_proposals);
    if(!exp_.not_record)
    {
        shared_obs_coefs_mh.add_recorder(best_target_recorder_);
        shared_obs_coefs_mh.add_recorder(
                ergo::make_mh_detail_recorder(
                    std::ostream_iterator<ergo::step_detail>(
                                                shared_log_fs_, "\n")));
    }
    return shared_obs_coefs_mh;
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

ergo::mh_step<Lss_set> Lss_set_sampler::noise_sigma_mh_step
(
    const Lss_set_pred_posterior_mh& evaluator,
    const Lss_set_noise_sigma_proposer& noise_proposer
)
{
    // MH shared noise_sigma step
    ergo::mh_step<Lss_set> shared_noise_sigma_mh(evaluator, 
                                                 noise_proposer); 

    // MH shared noise_sigma recorders 
    shared_noise_sigma_mh.store_proposed(exp_.run.write_proposals);
    if(!exp_.not_record)
    {
        shared_noise_sigma_mh.add_recorder(best_target_recorder_);
        shared_noise_sigma_mh.add_recorder(
                ergo::make_mh_detail_recorder(
                    std::ostream_iterator<ergo::step_detail>(
                                                shared_log_fs_, "\n")));
    }
    return shared_noise_sigma_mh;
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

ergo::hmc_step<Lss_set> Lss_set_sampler::obs_coefs_hmc_step
(
    const Shared_obs_coef_adapter& adapter
)
{
    // HMC shared obs-noise step
    size_t num_params = adapter.size(&lss_set_);
    ergo::hmc_step<Lss_set> hmc_step(
                       adapter, 
                       shared_posterior_mt_,
                       Obs_coef_gradient(
                           shared_posterior_mt_,
                           adapter,
                           std::vector<double>(num_params,
                                               exp_.run.obs_coef_grad_size),
                           exp_.run.estimate_grad_step,
                           2),
                       std::vector<double>(num_params, 
                                           exp_.run.obs_coef_hmc_size),
                       exp_.run.shared_num_leapfrog_steps,
                       0.0);
    // HMC recorders 
    hmc_step.store_proposed(exp_.run.write_proposals);
    if(!exp_.not_record)
    {
        hmc_step.add_recorder(best_target_recorder_);
        hmc_step.add_recorder(
                ergo::make_hmc_detail_recorder(
             std::ostream_iterator<ergo::step_detail>(shared_log_fs_, "\n")));
    }
    hmc_step.rename(std::string("obs-coef-hmc"));

    return hmc_step;
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

ergo::hmc_step<Lss_set> Lss_set_sampler::noise_sigma_hmc_step() 
{
    // HMC shared obs-noise step
    Shared_obs_noise_adapter obs_noise_adapter;
    size_t num_params_noise = obs_noise_adapter.size(&lss_set_);
    ergo::hmc_step<Lss_set> noise_sigma_hmc(
                       obs_noise_adapter, 
                       shared_posterior_mt_,
                       Obs_noise_gradient(
                           shared_posterior_mt_,
                           obs_noise_adapter,
                           std::vector<double>(num_params_noise,
                                               exp_.run.obs_noise_grad_size),
                           exp_.run.estimate_grad_step,
                           2),
                       std::vector<double>(num_params_noise, 
                                           exp_.run.noise_sigma_hmc_size),
                       exp_.run.shared_num_leapfrog_steps,
                       0.0);
    std::vector<double> ubs;
    std::vector<double> lbs;
    ubs.resize(num_params_noise);
    lbs.resize(num_params_noise);
    std::fill(lbs.begin(), lbs.end(), 0.0);
    std::fill(ubs.begin(), ubs.end(), std::numeric_limits<double>::max());
    noise_sigma_hmc.set_lower_bounds(lbs);
    noise_sigma_hmc.set_upper_bounds(ubs);
    // HMC recorders 
    noise_sigma_hmc.store_proposed(exp_.run.write_proposals);
    if(!exp_.not_record)
    {
        noise_sigma_hmc.add_recorder(best_target_recorder_);
        noise_sigma_hmc.add_recorder(
                ergo::make_hmc_detail_recorder(
             std::ostream_iterator<ergo::step_detail>(shared_log_fs_, "\n")));
    }
    noise_sigma_hmc.rename(std::string("obs-noise-hmc"));

    return noise_sigma_hmc;

}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

Lss_set Lss_set_sampler::train_marginal(size_t num_iterations)
{
    // optimize p(\theta, x_0 | Y, \alpha, \beta, \gamma, \sigma)
    std::vector<Linear_state_space>& lss_vec = lss_set_.lss_vec();
    size_t num_lss = lss_vec.size();
    std::vector<double> lss_grad_step_sizes(num_lss, exp_.run.person_grad_size);
                       
    Sample_options opt(exp_.run.state_prop_sigma, 
                       exp_.run.clo_param_prop_sigma, 
                       exp_.num_person_thrds); 

    std::vector<std::string> out_dirs(num_lss);
    boost::format out_fmt(exp_.out_dp + "/%04d/");
    for(size_t i = 0; i < num_lss; i++)
    {
        out_dirs[i] = (out_fmt % (ids_[i])).str();
        ETX(kjb_c::kjb_mkdir(out_dirs[i].c_str()));
    }
   
    Marginal_likelihood ml(shared_prior_, 
                           posteriors_, 
                           out_dirs,
                           opt,
                           exp_.not_record);

    Lss_set best_lss_set(lss_set_);
    Lss_set_mh_proposer proposer(exp_.run.mh_bmi_coef_sigma, 
                                 exp_.run.mh_var_sigma);
    ergo::mh_step<Lss_set> shared_step(ml, proposer);
    shared_step.add_recorder(best_target_recorder_);
    ergo::best_sample_recorder<Lss_set, Lss_set*> 
                        best_sample_recorder(&best_lss_set);
    shared_step.add_recorder(best_sample_recorder.replace());
    shared_step.add_recorder(
            ergo::make_mh_detail_recorder(
                std::ostream_iterator<ergo::step_detail>(shared_log_fs_, "\n")));
    double lp = shared_posterior_mt_(lss_set_);
    for(size_t i = 0; i < num_iterations; i++)
    {
        shared_step(lss_set_, lp); 
        shared_bst_fs_.flush();
        shared_log_fs_.flush();
        best_lss_set.write(exp_.out_dp);
    }
    return best_lss_set;
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

Lss_set Lss_set_sampler::train_model(size_t num_iterations)
{
    // write out the initial model (for debug)
    lss_set_.write(exp_.out_dp + "/init/");
    double max_training_time = (exp_.run.maximum_running_minutes * 60.0) * 0.7;
    std::cout << "Maximum training seconds: " << max_training_time << std::endl;

    // measurments of time
    struct timespec total_begin, total_finish;
    long long total_start; 
    const double NANOS = 1e9;
    get_current_time(&total_begin);
    total_start = total_begin.tv_sec * NANOS + total_begin.tv_nsec;

    ///////////////////////////////////////////////////////////
    // MH shared param step
    Lss_set_mh_proposer shared_param_proposer(exp_.run.mh_bmi_coef_sigma,
                                             exp_.run.mh_var_sigma);
    ergo::mh_step<Lss_set> shared_param_mh = 
                        shared_mh_step(shared_posterior_mt_,
                                             shared_param_proposer);
    // HMC shared param step
    Shared_param_adapter shared_adapter;
    size_t num_params = shared_adapter.size(&lss_set_);
    Shared_param_gradient shared_param_grad(
           shared_posterior_,
           shared_adapter,
           std::vector<double>(num_params,
                               exp_.run.shared_grad_size),
           exp_.run.estimate_grad_step);
    ergo::hmc_step<Lss_set> shared_param_hmc = shared_hmc_step(
                                                shared_posterior_mt_,
                                                shared_adapter,
                                                shared_param_grad,
                                                std::string("shared-param-hmc"));

    ///////////////////////////////////////////////////////////
    // Construct shared prior step
    ///////////////////////////////////////////////////////////

    // MH shared prior step
    ergo::mh_step<Lss_set> shared_prior_mh = 
                        shared_mh_step(prior_pos_,
                                       shared_param_proposer);
    // HMC shared prior step
    Shared_ind_blr_prior_gradient shared_prior_grad(
            Independent_blr_posterior(ind_blr_prior_),
            shared_adapter,
            std::vector<double>(num_params,
                               exp_.run.shared_grad_size),
            exp_.run.estimate_grad_step);
    ergo::hmc_step<Lss_set> shared_prior_hmc = 
                                shared_hmc_step(
                                        prior_pos_,
                                        shared_adapter,
                                        shared_prior_grad,
                                        std::string("shared-prior-hmc"));

    // Gibbs shared-prior step
    Lss_set_mean_variance_step mean_var_step(ind_blr_prior_); 

    // HMC shared scale step (parameter drift)
    Shared_gp_scale_posterior scale_pos(lss_scale_prior_);
    ergo::hmc_step<Lss_set> shared_scale_hmc = shared_scale_hmc_step(scale_pos);

    // Gibbs step for group 
    /*Lss_set_group_step group_step(exp_.cluster.num_groups, 
                                    group_prior_, 
                                    exp_.cluster.collapsed_gibbs);*/
    Cluster_step cluster_step(cluster_prior_, prior_pos_);

    ///////////////////////////////////////////////////////////
    // Construct obs coef step
    // MH shared obs_coefs step
    int D = lss_set_.lss_vec().front().obs_coef_dim();
    Lss_set_obs_coef_proposer obs_prop(Vector(D, exp_.run.mh_obs_coef_sigma));
    ergo::mh_step<Lss_set> shared_obs_coefs_mh = obs_coefs_mh_step(obs_prop);
    // HMC shared obs_coefs step
    size_t num_osc = lss_set_.num_oscillators();
    size_t num_obs = lss_set_.num_observables();
    size_t num_coef = lss_set_.obs_coef_dim();
    Shared_obs_coef_adapter adapter(num_obs, num_osc, num_coef);
    ergo::hmc_step<Lss_set> shared_obs_coefs_hmc = obs_coefs_hmc_step(adapter);

    ///////////////////////////////////////////////////////////
    // Construct obs noise sigma step
    // Gibbs shared obs-noise step
    Lss_set_noise_sigma_step noise_sigma_gibbs(noise_prior_, likelihoods_);
    // HMC shared obs-noise step
    ergo::hmc_step<Lss_set> noise_sigma_hmc = noise_sigma_hmc_step();
    // MH step based on the predictive distribution  
    // set up the predictive posterior 
    size_t num_lss = lss_set_.lss_vec().size();
    Lss_set_pred_posterior_mh shared_pred_post_mh(
                                        posteriors_,
                                        exp_.run.state_prop_sigma,
                                        exp_.run.clo_param_prop_sigma,
                                        exp_.not_record, 
                                        exp_.num_person_thrds,
                                        lss_out_dirs_);
    D = lss_set_.get_noise_sigmas().size();  
    Lss_set_noise_sigma_proposer noise_prop(Vector(D, exp_.run.mh_noise_sigma));
    ergo::mh_step<Lss_set> noise_sigma_mh = 
                        noise_sigma_mh_step(shared_pred_post_mh, noise_prop);

    // Gibbs recorder
    typedef std::ostream_iterator<ergo::step_detail> sd_it;
    ergo::default_detail_recorder<sd_it> gibbs_rec(
                                "gibbs", sd_it(shared_log_fs_, "\n"));

    ///////////////////////////////////////////////////////////
    // set up couple independent params step 
    bool optimize = false;
    size_t num_samples = 0;
    bool sample_state = exp_.lss_state_dp == "" ? true : false;
    bool sample_clo = exp_.prior.fixed_clo ? false : true; 
    std::cout << sample_clo << " sample_clo: " << std::endl;
    bool sample_poly_terms = exp_.lss.polynomial_degree >= 0 
                              && (!exp_.prior.fixed_clo) ? true : false;
    std::cout << sample_poly_terms << " sample_poly: " << std::endl;
    // compute the variance of the data
    Vector state_variance((int)exp_.lss.num_oscillators, 0.0);
    for(size_t i = 0; i < data_.size(); i++)
    {
        Obs_map::const_iterator it = data_[i].observables.begin();
        std::pair<Vector, Vector> stat = standardize(data_[i],
                                        it->first, false, false);
        state_variance += stat.second; 
    }
    state_variance /= data_.size();
    std::cout << "data state variance: " << state_variance << std::endl;

    // Set the state prop sigma based on the data 
    double state_prop_sigma = std::sqrt(std::accumulate(state_variance.begin(), 
                              state_variance.end(), 0.0)/ state_variance.size());
    std::cout << "State prop sigma: " << state_prop_sigma << std::endl;
    bool sample_cluster = false;
    
    Mh_step_thread person_thrd(
        lss_set_.lss_vec(), 
        posteriors_,
        lss_out_dirs_,
        state_prop_sigma,
        exp_.run.clo_param_prop_sigma,
        exp_.run.poly_term_prop_sigma,
        lss_set_.all_samples(),
        !exp_.not_record,
        !exp_.not_write_trace,
        optimize,
        cluster_prior_,
        lss_set_.group_params(),
        sample_cluster,
        exp_.run.adapt_mh,
        num_samples,
        sample_state,
        sample_clo, 
        sample_poly_terms,
        exp_.run.mh_converge_iter,
        max_training_time
    );

    // Parameter drift 
    Drift_step_thread person_thrd_drift(
        lss_set_.lss_vec(), 
        posteriors_, 
        lss_out_dirs_,
        exp_.run.state_prop_sigma, 
        exp_.run.poly_term_prop_sigma, 
        lss_set_.all_samples(),
        exp_.drift_sampler.ctr_pt_length,
        exp_.drift_sampler.num_burn_iters, 
        exp_.drift_sampler.num_sample_iters,
        exp_.not_record,
        optimize,
        num_samples,
        sample_state,
        sample_poly_terms,
        exp_.run.mh_converge_iter
    );

    // HMC step for Linear_state_space
    typedef Hmc_step_thread<Posterior, Lss_adapter, Lss_gradient<Lss_adapter> >
                                                                Hmc_step_thread;
    std::string step_name("couple-step");
    Lss_adapter lss_adapter(sample_state, sample_clo);
    std::vector<double> grad_sizes(num_lss, exp_.run.person_grad_size);
    Hmc_step_thread person_thrd_hmc(
        lss_set_.lss_vec(), 
        lss_adapter,
        posteriors_,
        grad_sizes,
        person_hmc_sizes_,
        lss_out_dirs_,
        lss_set_.all_samples(), 
        step_name,
        exp_.not_record,
        optimize,
        exp_.run.person_num_leapfrog_steps
    );

    // Best Lss_set
    Lss_set best_lss_set(lss_set_);

    // Logging file
    std::ofstream iter_log_ofs;
    if (exp_.run.iter_log_fname != "")
    {
        iter_log_ofs.open(exp_.run.iter_log_fname.c_str());
        IFTD(iter_log_ofs.is_open(), IO_error, "Can't open file %s", 
                                     (exp_.run.iter_log_fname.c_str()));
    }

    // Error file
    std::ofstream err_log_ofs;
    if (exp_.run.fit_err_fname != "")
    {
        err_log_ofs.open(exp_.run.fit_err_fname.c_str());
        IFTD(err_log_ofs.is_open(), IO_error, "Can't open file %s", 
                                     (exp_.run.fit_err_fname.c_str()));
    }

    // Logging file
    std::ofstream trace_ofs;
    if (exp_.run.trace_fname != "")
    {
        trace_ofs.open(exp_.run.trace_fname.c_str());
        IFTD(trace_ofs.is_open(), IO_error, "can't open file %s", 
                                     (exp_.run.trace_fname.c_str()));
    }

    // Initial log posterior
    double lp = shared_posterior_mt_(lss_set_);
    std::cout << " init_lp: " << lp << "\n";

    double best_lp = lp;
    double prev_best_lp = lp;
    size_t best_lp_iter = 0;

    // For shared_prior_hmc step
    size_t shared_num_iters = 0;
    size_t shared_num_accepted = 0;

    size_t shared_noise_num_iters = 0;
    size_t shared_noise_num_accepted = 0;

    size_t shared_obs_coef_num_iters = 0;
    size_t shared_obs_coef_num_accepted = 0;

    long long start, elapsed; 
    struct timespec begin, finish;
    double person_time = 0.0;
    double shared_prior_time = 0.0;
    double shared_scale_time = 0.0;
    double obs_coef_time = 0.0;
    double obs_noise_sigma_time = 0.0;
    double error_time = 0.0;
    double logging_time = 0.0;

    bool sstate = exp_.lss_state_dp == "" && !exp_.lss.ignore_clo ? true : false;
    Uniform_distribution sample_p;
    // Clear samples
    lss_set_.clear_samples();
    Lss_set sample_mean(lss_set_);
    size_t s_counts = 0;
    bool first = true;
    std::cout << "Training model" << std::endl;
    boost::progress_display progress_bar(num_iterations);
    for (size_t i = 0; i < num_iterations; i++)
    {
        ////////////////////////////////////////////////////////////////////
        //                Sampling over couple-specific parameters
        ////////////////////////////////////////////////////////////////////
        size_t avail_cores = boost::thread::hardware_concurrency();
        size_t num_threads = exp_.num_person_thrds > avail_cores ? 
                                avail_cores : exp_.num_person_thrds;
        boost::exception_ptr err;
        get_current_time(&begin);
        start = begin.tv_sec * NANOS + begin.tv_nsec;
        if (!exp_.prior.fixed_clo || (exp_.prior.fixed_clo && sstate))
        {
            size_t N = ids_.size();
            try
            {
                if(exp_.lss.drift)
                {
                    IFT(exp_.lss.drift, Illegal_argument, 
                            "CLO parameters must be allowed to drift. "
                            "This can be turned on by \"allow-drift\" option.");
                    run_threads(person_thrd_drift, N, N, err);
                }
                else
                {
                    if(exp_.run.person_step_approach == "MH")
                    {
                        //MH
                        run_threads(person_thrd, N, N, err);
                    }
                    else
                    {
                        // HMC 
                        assert(exp_.run.person_step_approach == "HMC");
                        run_threads(person_thrd_hmc, N, N, err);
                    }
                }
            }
            catch(boost::exception& err)
            {
                std::cerr << "exception happend in sampling over "
                          << "couple's CLO parameter in training.\n";
                std::cerr << boost::current_exception_diagnostic_information(); 
            }
       
            lp = shared_posterior_mt_(lss_set_);
            // Turn on use_pred for each posteriors
            /*std::for_each(posteriors_.begin(), posteriors_.end(), 
                          boost::bind(&Posterior::use_pred, _1, true));
            double lp_pred = shared_posterior_mt_(lss_set_);
            std::for_each(posteriors_.begin(), posteriors_.end(), 
                          boost::bind(&Posterior::use_pred, _1, false));*/
            if(!exp_.not_record)
            {
                iter_log_ofs << i <<  " after person: " << lp << "\n";
            }

            //if(i >= BURNIN_ITER && lp_pred > best_lp)
            if(i >= BURNIN_ITER) 
            {
                if(lp > best_lp)
                {
                    // keep track of the best posterior
                    best_lp = lp;
                    best_lss_set = lss_set_;
                }
            }
        }

        get_current_time(&finish);
        elapsed = finish.tv_sec * NANOS + finish.tv_nsec - start;
        person_time += elapsed/ NANOS; 

        ////////////////////////////////////////////////////////////////////
        //                Sampling over shared params / priors
        ////////////////////////////////////////////////////////////////////

        if (!exp_.run.fit_ind_clo)
        {
            // all the dyads share the same parameters
            if(exp_.prior.fixed_clo)
            {
                get_current_time(&begin);
                start = begin.tv_sec * NANOS + begin.tv_nsec;
                if(exp_.run.shared_sample_approach == "hmc")
                {
                    for(size_t j = 0; j < 5; j++)
                    {
                        shared_param_hmc(lss_set_, lp);
                        // adapt hmc step size
                        shared_num_iters++;
                        adapt_hmc_step_size<Lss_set>(shared_param_hmc, 
                                                     shared_log_fs_,
                                                     shared_num_iters, 
                                                     shared_num_accepted,
                                                     20,
                                                     exp_.not_record);
                        // check convergence
                        if(i >= BURNIN_ITER && lp > best_lp)
                        {
                            // keep track of the best posterior
                            //best_lp = lp_pred;
                            best_lp = lp;
                            best_lss_set = lss_set_;
                        }
                        if(!exp_.not_record)
                        {
                            iter_log_ofs << i << " after shared-param-hmc: "
                                         << lp << "\n";
                        }
                        // record proposals
                        if(exp_.run.write_proposals)
                        {
                            record_proposal_(
                                    *shared_param_hmc.proposed_model());
                        }
                    }
                }
                else
                {
                    Poisson_distribution iter_p(100);
                    size_t num_iter = sample(iter_p);
                    for(size_t j = 0; j < num_iter; j++)
                    {
                        if(i == 0 && j == 0)
                        {
                            std::cout << " Fixed-mean is selected, "
                                      << "using MH to sample the shared parameters.\n";
                            std::cout << " If you want to use HMC instead of MH, please specify " 
                                      << " [shared-param-sampling-method=hmc] "
                                      << "in the configuration file (.CFG).\n";
                        }
                        shared_param_mh(lss_set_, lp);
                        if(i >= BURNIN_ITER && lp > best_lp)
                        {
                            // keep track of the best posterior
                            best_lp = lp;
                            best_lss_set = lss_set_;
                        }
                        if(exp_.run.write_proposals)
                        {
                            record_proposal_(*shared_param_mh.proposed_model());
                        }
                        if(exp_.run.write_samples)
                        {
                            record_sample_(lss_set_);
                        }
                        if(!exp_.not_record)
                        {
                            iter_log_ofs << i <<  " after shared-param-mh: " 
                                         << lp << "\n";
                        }
                    }
                }
                get_current_time(&finish);
                elapsed = finish.tv_sec * NANOS + finish.tv_nsec - start;
                shared_prior_time += elapsed / NANOS; 
            }
            else if(exp_.prior.prior_fp == "")
            {
                // All the dyads share the same prior distributions
                get_current_time(&begin);
                start = begin.tv_sec * NANOS + begin.tv_nsec;

                if(exp_.run.shared_sample_approach == "gibbs")
                {
                    for(size_t j = 0; j < 1; j++)
                    {
                        mean_var_step(lss_set_, lp);
                        if(!exp_.not_record)
                        {
                            gibbs_rec(mean_var_step, lss_set_, lp);
                        }
                        // turn on use_pred for each posteriors
                        /*std::for_each(posteriors_.begin(), posteriors_.end(), 
                                  boost::bind(&Posterior::use_pred, _1, true));
                        double lp_pred = shared_posterior_mt_(lss_set_);
                        // turn on use_pred for each posteriors
                        std::for_each(posteriors_.begin(), posteriors_.end(), 
                              boost::bind(&Posterior::use_pred, _1, false));*/
                        // check values 
                        //if(i >= BURNIN_ITER && lp_pred > best_lp)
                        if(i >= BURNIN_ITER && lp > best_lp)
                        {
                                // Keep track of the best posterior
                                best_lp = lp;
                                best_lss_set = lss_set_;
                        }
                        if(!exp_.not_record)
                        {
                            iter_log_ofs << i << " after mean var: " 
                                         << lp << "\n";
                        }
                    }
                }
                else if(exp_.run.shared_sample_approach == "hmc")
                { 
                    // Use hmc to sample the shared parameters 
                    double pr = prior_pos_(lss_set_);
                    double best_pr = pr;
                    double ll = lp - pr;
                    for(size_t j = 0; j < 5; j++)
                    {
                        shared_prior_hmc(lss_set_, pr);
                        // adapt hmc step size
                        shared_num_iters++;
                        adapt_hmc_step_size<Lss_set>(shared_prior_hmc, 
                                                     shared_log_fs_,
                                                     shared_num_iters, 
                                                     shared_num_accepted,
                                                     20,
                                                     exp_.not_record);
                        if(!exp_.not_record)
                        {
                            iter_log_ofs << i << " after shared-prior-hmc: "
                                         << ll + pr << "\n";
                        }
                        // record the best value
                        if(pr > best_pr && i > BURNIN_ITER)
                        {
                            best_lp = ll + pr;
                            best_lss_set = lss_set_;
                        }
                        // record proposals
                        if(exp_.run.write_proposals)
                        {
                            record_proposal_(
                                    *shared_prior_hmc.proposed_model());
                        }
                        if(exp_.run.write_samples)
                        {
                            record_sample_(lss_set_);
                        }
                    }
                }
                else 
                { 
                    assert(exp_.run.shared_sample_approach == "mh");
                    std::cout << "[WARNING]: MH is selected to sample shared params/prior\n";

                    // Shared parameters without varaince 
                    double pr = prior_pos_(lss_set_);
                    double best_pr = pr;
                    double ll = lp - pr;
                    Poisson_distribution iter_p(100);
                    size_t num_iter = sample(iter_p);
                    for (size_t j = 0; j < num_iter; j++)
                    {
                        shared_prior_mh(lss_set_, pr);
                        if(!exp_.not_record)
                        {
                            iter_log_ofs << i <<  " after shared-prior-mh: " 
                                         << ll + pr << "\n";
                        }
                        if(pr > best_pr && i >= BURNIN_ITER)
                        {
                            best_lp = ll + pr;
                            best_lss_set = lss_set_;
                        }
                        if(exp_.run.write_proposals)
                        {
                            record_proposal_(*shared_prior_mh.proposed_model());
                        }
                    }
                }

                get_current_time(&finish);
                elapsed = finish.tv_sec * NANOS + finish.tv_nsec - start;
                shared_prior_time += elapsed / NANOS; 
            }
        }
        
        // Sampling over the scale parameters
        if (exp_.lss.drift && exp_.prior.prior_fp == "")
        {
            get_current_time(&begin);
            start = begin.tv_sec * NANOS + begin.tv_nsec;
            ////////////////////////////////////////////////////////////////////
            //                Sampling over scale parameters
            ////////////////////////////////////////////////////////////////////
            // Use hmc to sample the shared scale
            double pr = scale_pos(lss_set_);
            double ll = lp - pr; 
            size_t shared_scale_num_iters = 0;
            size_t shared_scale_num_accepted = 0;
            try
            {
                for(size_t j = 0; j < exp_.run.num_hmc_iter; j++)
                {
                    shared_scale_hmc(lss_set_, pr);
                    if(exp_.run.write_samples)
                    {
                        record_sample_(lss_set_);
                    }
                    if(exp_.run.write_proposals)
                    {
                        record_proposal_(*shared_scale_hmc.proposed_model());
                    }

                    const size_t iter_to_check = 10;
                    adapt_hmc_step_size<Lss_set>(shared_scale_hmc, 
                                                 shared_log_fs_,
                                                 shared_scale_num_iters, 
                                                 shared_scale_num_accepted,
                                                 iter_to_check,
                                                 exp_.not_record);
                    shared_scale_num_iters++;
                    if(!exp_.not_record)
                    {
                        shared_log_fs_.flush();
                        shared_bst_fs_.flush();
                    }
                }
            }
            catch(Exception& ex)
            {
                std::cerr << "Error occurs during shared_scale_hmc step\n";
                std::cerr << ex.what();
            }

            lp = ll + pr;
            if(!exp_.not_record)
            {
                iter_log_ofs << i <<  " after gp-scale : " << lp << "\n";
            }

            //if(i >= BURNIN_ITER && lp_pred > best_lp)
            if(i >= BURNIN_ITER && lp > best_lp)
            {
                // keep track of the best posterior
                //best_lp = lp_pred;
                best_lp = lp;
                best_lss_set = lss_set_;
            }
            get_current_time(&finish);
            elapsed = finish.tv_sec * NANOS + finish.tv_nsec - start;
            shared_scale_time += elapsed / NANOS; 
        }

        /////////////////////////////////////////////////////
        //    SAMPLING OVER OBS_COEFS
        /////////////////////////////////////////////////////

        if (exp_.likelihood.obs_names.size() > 1 && exp_.run.obs_coef_fp == "")
        {
            get_current_time(&begin);
            start = begin.tv_sec * NANOS + begin.tv_nsec;
            for(size_t j = 0; j < lss_set_.obs_coefs().size() * 5; j++)
            {
                if(exp_.run.obs_coef_sample_approach == "mh")
                {
                    shared_obs_coefs_mh(lss_set_, lp);
                    if(!exp_.not_record)
                    {
                        iter_log_ofs << i << " after obs coefs(MH): " << lp << "\n";
                    }
                }
                else
                {
                    assert(exp_.run.obs_coef_sample_approach == "hmc");
                    shared_obs_coefs_hmc(lss_set_, lp);
                    if(!exp_.not_record)
                    {
                        iter_log_ofs << i << " after obs coefs(HMC): " << lp << "\n";
                    }
                    adapt_hmc_step_size<Lss_set>(shared_obs_coefs_hmc, 
                                                 shared_log_fs_,
                                                 shared_obs_coef_num_iters, 
                                                 shared_obs_coef_num_accepted,
                                                 20,
                                                 exp_.not_record);
                    shared_obs_coef_num_iters++;
                }
                // turn on use_pred for each posteriors
                /*std::for_each(posteriors_.begin(), posteriors_.end(), 
                              boost::bind(&Posterior::use_pred, _1, true));
                double lp_pred = shared_posterior_mt_(lss_set_);
                // turn on use_pred for each posteriors
                std::for_each(posteriors_.begin(), posteriors_.end(), 
                              boost::bind(&Posterior::use_pred, _1, false));*/
                //if(i >= BURNIN_ITER && lp_pred > best_lp)
                if(i >= BURNIN_ITER) 
                {
                    if(lp > best_lp)
                    {
                        //best_lp = lp_pred;
                        best_lp = lp;
                        best_lss_set = lss_set_;
                    }
                }
            }
            get_current_time(&finish);
            elapsed = finish.tv_sec * NANOS + finish.tv_nsec - start;
            obs_coef_time += elapsed / NANOS; 
        }

        /////////////////////////////////////////////////////
        //    SAMPLING OVER THE NOISE SIGMAS
        ///////////////////////////////////////////////////////

        if(exp_.sample_noise_sigma)
        {
            get_current_time(&begin);
            start = begin.tv_sec * NANOS + begin.tv_nsec;
            bool sample_noise_mh = false;
            for(size_t j = 0; j < 1; j++)
            //for(size_t j = 0; j < exp_.likelihood.obs_names.size(); j++)
            {
                if(exp_.run.obs_noise_sample_approach == "gibbs")
                {
                    noise_sigma_gibbs(lss_set_, lp);
                    if(exp_.run.write_proposals)
                    {
                        record_proposal_(lss_set_);
                    }
                }
                else if(exp_.run.obs_noise_sample_approach == "mh")
                {
                    // only sample noise some times
                    if(sample(Uniform_distribution()) < 0.2)
                    {
                        // TODO need to fix lp
                        double cur_lp = lp; 
                        double pred_lp = shared_pred_post_mh(lss_set_);
                        double best_pred_lp = pred_lp;
                        iter_log_ofs << i << " before noise-sigma [mh]" << pred_lp << std::endl;
                        for(size_t k = 0; k < 20; k++)
                        {
                            //iter_log_ofs << i << " " << k << " noise_sigma\n";
                            noise_sigma_mh(lss_set_, pred_lp);
                            if(pred_lp > best_pred_lp)
                            {
                                //best_lp = lp_pred;
                                best_pred_lp = pred_lp;
                                best_lss_set = lss_set_;
                            }
                        }
                        iter_log_ofs << i << " after noise-sigma [mh]" << pred_lp << std::endl;
                        lp = shared_posterior_(lss_set_);
                        if(exp_.run.write_proposals)
                        {
                            record_proposal_(*noise_sigma_mh.proposed_model());
                        }
                        sample_noise_mh = true;
                        if(!exp_.not_record)
                        {
                            iter_log_ofs << i << " after noise-sigma ["
                                         << exp_.run.obs_noise_sample_approach << 
                                         "]: " << lp << "\n";
                        }

                    }
                }
                else
                {
                    noise_sigma_hmc(lss_set_, lp);
                    if(exp_.run.write_proposals)
                    {
                        record_proposal_(*noise_sigma_hmc.proposed_model());
                    }

                    adapt_hmc_step_size<Lss_set>(noise_sigma_hmc, 
                                                 shared_log_fs_,
                                                 shared_noise_num_iters, 
                                                 shared_noise_num_accepted,
                                                 20,
                                                 exp_.not_record);
                    shared_noise_num_iters++;
                }
                // turn on use_pred for each posteriors
                /*std::for_each(posteriors_.begin(), posteriors_.end(), 
                              boost::bind(&Posterior::use_pred, _1, true));
                double lp_pred = shared_posterior_mt_(lss_set_);
                // turn on use_pred for each posteriors
                std::for_each(posteriors_.begin(), posteriors_.end(), 
                              boost::bind(&Posterior::use_pred, _1, false));*/
                //if(i >= BURNIN_ITER && lp_pred > best_lp)
                if(i >= BURNIN_ITER) 
                {
                    if(lp > best_lp)
                    {
                        //best_lp = lp_pred;
                        best_lp = lp;
                        best_lss_set = lss_set_;
                    }
                }
                if(exp_.run.write_samples)
                {
                    record_sample_(lss_set_);
                }
                if(!exp_.not_record && !sample_noise_mh)
                {
                    iter_log_ofs << i << " after noise-sigma ["
                                 << exp_.run.obs_noise_sample_approach << 
                                 "]: " << lp << "\n";
                }
            }
            get_current_time(&finish);
            elapsed = finish.tv_sec * NANOS + finish.tv_nsec - start;
            obs_noise_sigma_time += elapsed / NANOS; 
        }

        // Sampling over the cluster
        if (exp_.run.sample_cluster)
        {
            get_current_time(&begin);
            start = begin.tv_sec * NANOS + begin.tv_nsec;
            for(size_t j = 0; j < 2; j++)
            {
                cluster_step(lss_set_, lp);
                if(!exp_.not_record)
                {
                    gibbs_rec(cluster_step, lss_set_, lp);
                }
                if(i >= BURNIN_ITER && lp > best_lp)
                {
                    // keep track of the best posterior
                    best_lp = lp;
                    best_lss_set = lss_set_;
                }

                if(!exp_.not_record)
                {
                    iter_log_ofs << i << " after group " << lp << "\n";
                }
            }
            get_current_time(&finish);
            elapsed = finish.tv_sec * NANOS + finish.tv_nsec - start;
            shared_prior_time += elapsed / NANOS; 
        }

        get_current_time(&begin);
        start = begin.tv_sec * NANOS + begin.tv_nsec;
        if(!exp_.not_record)
        {
            bst_fs_ << best_lp << "\n";
            iter_log_ofs.flush();
            bst_fs_.flush();
            shared_log_fs_.flush();
            shared_bst_fs_.flush();
        }

        // compute fitting error 
        std::vector<Vector> obs_errors;
        Vector error = compute_ave_error(data_, lss_set_, obs_errors);

        size_t num_osc = exp_.lss.num_oscillators;
        assert(error.size() == 2 * num_osc);
        double fit_err = 0.0;
        double pre_err = 0.0;
        for(size_t j = 0; j < num_osc; j++)
        {
            fit_err += error[j];
        }
        for(size_t j = num_osc; j < num_osc * 2; j++)
        {
            pre_err += error[j];
        }

        if(!exp_.not_record)
        {
            err_log_ofs << i <<  " " << fit_err << " " << pre_err << "\n";
            err_log_ofs.flush();
        }

        get_current_time(&finish);
        elapsed = finish.tv_sec * NANOS + finish.tv_nsec - start;
        error_time += elapsed / NANOS; 

        get_current_time(&begin);
        start = begin.tv_sec * NANOS + begin.tv_nsec;
        //write the intermediate results 
        if((i + 1) % 10 == 0) 
        {
            // Since there has been weird problem on IO on TACC, here are some
            // guards when we tried to write best_lss_set to the disk
            try
            {
                best_lss_set.write(exp_.out_dp + "/best_model/");
                lss_set_.write(exp_.out_dp);
            }
            catch(Exception& exp)
            {
                exp.print_details(); 
                std::cerr << "IO error happens when writing best_lss_set to "
                          << exp_.out_dp << std::endl;
            }
            if(!exp_.not_write_trace)
            {
                trace_ofs << i << " ";
                trace_ofs << lss_set_<< "\n";
                trace_ofs.flush();
            }
        }

        get_current_time(&finish);
        elapsed = finish.tv_sec * NANOS + finish.tv_nsec - start;
        logging_time += elapsed / NANOS; 

        // Check if converges 
        size_t iter_checker = i * 0.2;
        if(iter_checker > exp_.run.num_converge_iters)
        {
            iter_checker = exp_.run.num_converge_iters;
        }

        if(best_lp_iter > iter_checker)
        {
            if(fabs(best_lp - prev_best_lp) < 0.5)
            {
                break;
            }
            else
            {
                prev_best_lp = best_lp;
                best_lp_iter++;
            }
        }

        // Check the total time so far 
        double running_time_so_far = person_time + 
                                     shared_prior_time +
                                     shared_scale_time + 
                                     obs_coef_time + 
                                     obs_noise_sigma_time +
                                     error_time + 
                                     logging_time;
        if(running_time_so_far > max_training_time)
        {
            std::cout << "Exceeding the maximum training time " << max_training_time << " s"
                         << "\nFinishing training \n";
            break;
        }
        ++progress_bar;
    }


    // Close all the training files 
    shared_log_fs_.close();
    shared_bst_fs_.close();
    bst_fs_.close();
    iter_log_ofs.close();
    err_log_ofs.close();
    if(!exp_.not_write_trace) trace_ofs.close();

    get_current_time(&total_finish);
    long long total_elapsed = total_finish.tv_sec * NANOS + total_finish.tv_nsec - total_start;
    double total_seconds = total_elapsed / NANOS ; 
    std::cout << "[Training] Time in person threads: " << person_time << "s\n";
    std::cout << "[Training] Time in shared-prior: " << shared_prior_time << "s\n";
    std::cout << "[Training] Time in shared scale time: " << shared_scale_time << "s\n";
    std::cout << "[Training] Time in obs coef: " << obs_coef_time << "s\n";
    std::cout << "[Training] Time in obs noise: " << obs_noise_sigma_time << "s\n";
    std::cout << "[Training] Time in compute error: " << error_time << "s\n";
    std::cout << "[Training] Time in logging: " << logging_time << "s\n";
    std::cout << "[Training] Total time: " << total_seconds << "s\n";

    //return sample_mean;
    return best_lss_set;
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

Lss_set Lss_set_sampler::learn_pred_mh(size_t num_iterations)
{
    // set up the output dirs for individual CLO opt
    boost::format log_dir_fmt(exp_.out_dp + "/iters/00000/%04d/");
    for(size_t j = 0; j < ids_.size(); j++)
    {
        std::string out_dir = (log_dir_fmt % ids_[j]).str();
        ETX(kjb_c::kjb_mkdir(out_dir.c_str()));
        lss_out_dirs_[j] = out_dir;
    }

    // set up the predictive posterior 
    size_t num_lss = lss_set_.lss_vec().size();
    Lss_set_pred_posterior_mh shared_pred_post(
                                        posteriors_,
                                        exp_.run.state_prop_sigma,
                                        exp_.run.clo_param_prop_sigma,
                                        exp_.not_record, 
                                        exp_.num_person_thrds,
                                        lss_out_dirs_);

    Lss_set_mh_proposer lss_set_proposer(exp_.run.mh_bmi_coef_sigma,
                                         exp_.run.mh_var_sigma);

    // set up the shared_prior_step
    ergo::mh_step<Lss_set> shared_step(shared_pred_post, lss_set_proposer);

    // The best Lss_set 
    Lss_set best_lss_set(lss_set_);

    // recorders 
    ergo::best_sample_recorder<Lss_set, Lss_set*> best_sample_recorder(
                                                                &best_lss_set);
    shared_step.add_recorder(best_sample_recorder.replace());
    shared_step.store_proposed(exp_.run.write_proposals);
    if(!exp_.not_record)
    {
        shared_step.add_recorder(best_target_recorder_);
        shared_step.add_recorder(
                ergo::make_mh_detail_recorder(
             std::ostream_iterator<ergo::step_detail>(shared_log_fs_, "\n")));
    }
    shared_step.rename(std::string("shared-pred-step"));

    // logging file
    std::ofstream iter_log_ofs;
    if(exp_.run.iter_log_fname != "")
    {
        iter_log_ofs.open(exp_.run.iter_log_fname.c_str());
        IFTD(iter_log_ofs.is_open(), IO_error, "can't open file %s", 
                                     (exp_.run.iter_log_fname.c_str()));
    }

    double lp = shared_pred_post(lss_set_);
    size_t shared_num_iters = 0;

    boost::format log_dir_fmt_iter(exp_.out_dp + "/iters/%05d/%04d/");
    for(size_t i = 1; i <= num_iterations; i++)
    {
        std::cout << "iter: " << i << "\n";

        // set up the output dirs 
        for(size_t j = 0; j < ids_.size(); j++)
        {
            std::string out_dir = (log_dir_fmt_iter % i % ids_[j]).str();
            ETX(kjb_c::kjb_mkdir(out_dir.c_str()));
            lss_out_dirs_[j] = out_dir;
        }

        shared_pred_post.set_out_dirs(lss_out_dirs_);

        if(exp_.run.iter_log_fname == "")
        {
            std::cout << "iter: " << i << " ll: " << lp << " "; 
        }
        else
        {
            iter_log_ofs << "iter: " << i << " ll: " << lp << " ";
        }
            
        shared_step(lss_set_, lp);


        if(exp_.run.write_samples)
        {
            record_sample_(lss_set_);
        }
        if(exp_.run.write_proposals)
        {
            record_proposal_(*shared_step.proposed_model());
        }

        shared_num_iters++;
        if(!exp_.not_record)
        {
            shared_log_fs_.flush();
            shared_bst_fs_.flush();
        }
   
        // keep the model that maximize the last 20% predicting error
        std::vector<Vector> obs_errors;
        Vector err = compute_ave_error(data_, best_lss_set, obs_errors);
        assert(err.size() == 2 * exp_.lss.num_oscillators);
        double cur_err = err[2] + err[3];
        if(!exp_.not_record)
        {
            iter_log_ofs << " err: " << cur_err << "\n";
        }
        /*else
        {
            if(exp_.run.iter_log_fname == "")
            {
                std::cout << "\n";
            }
            else
            {
                iter_log_ofs << "\n";
            }
        }*/

        //write the intermediate results 
        best_lss_set.write(exp_.out_dp);
    }
    return best_lss_set;
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

Lss_set Lss_set_sampler::learn_gp(size_t num_iterations)
{
    std::cout << "learn gp: " << "\n";
    // set up the output dirs 
    boost::format log_dir_fmt(exp_.out_dp + "/%04d/");
    for(size_t i = 0; i < ids_.size(); i++)
    {
        std::string out_dir = (log_dir_fmt % ids_[i]).str();
        ETX(kjb_c::kjb_mkdir(out_dir.c_str()));
        lss_out_dirs_[i] = out_dir;
    }
    size_t avail_cores = boost::thread::hardware_concurrency();
    size_t num_threads = exp_.num_person_thrds > avail_cores ? 
                            avail_cores : exp_.num_person_thrds;

    // create the evaluator for GP 
    Lss_set_gp_pred gp_pred(posteriors_, 
                            lss_set_, 
                            exp_.run.state_prop_sigma, 
                            exp_.drift_sampler.num_burn_iters, 
                            exp_.drift_sampler.ctr_pt_length, 
                            exp_.drift_sampler.num_sample_iters,
                            lss_out_dirs_, 
                            num_threads, 
                            exp_.not_record);

    size_t num_params = lss_set_.clo_param_size();
    Vector gp_vals(num_params * 2);
    Vector stds(num_params * 2);
    for(size_t i = 0; i < num_params; i++)
    {
        if(exp_.run.gp_params_fp == "")
        {
            gp_vals[i] = log(exp_.prior.gp_scale);
            gp_vals[num_params + i] = log(exp_.prior.clo_sigma * exp_.prior.clo_sigma);
        }
        stds[i] = log(100);
        stds[num_params + i] = log(10);
    }
    if(exp_.run.gp_params_fp != "")
    {
        std::ifstream ifs(exp_.run.gp_params_fp.c_str());
        IFTD(ifs.is_open(), IO_error, "can't open file %s", (exp_.run.gp_params_fp.c_str()));
        std::string line;
        getline(ifs, line);
        std::istringstream istr(line);
        std::vector<double> elems((std::istream_iterator<double>(istr)), 
                                  (std::istream_iterator<double>()));
        std::copy(elems.begin(), elems.begin() + num_params, gp_vals.begin());
        std::copy(elems.begin() + num_params, elems.end(),
                  gp_vals.begin() + num_params);

        std::cout << "read in gp_vals: " << gp_vals << "\n";
    }

    std::cout << "create the lss_set_gp_pred\n";

    std::string log_fp = exp_.out_dp + "/pgpe_log.txt"; 
    //if(!exp_.not_record) log_fp = exp_.out_dp + "/pgpe_log.txt"; 

    std::cout << log_fp << "\n";
    Pgpe<Lss_set_gp_pred> pgpe(gp_vals, stds, gp_pred, 
                               0.2, 0.2, num_iterations,
                               log_fp);
    std::string param_fp = exp_.out_dp + "/gp_params.txt";
    pgpe.run(param_fp);
    std::cout << param_fp << "\n";

    // get the best parameteres
    Vector best_gp_params = pgpe.get_best_parameters();
    gp_pred(best_gp_params);

    return gp_pred.current_sample();
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

Lss_set Lss_set_sampler::test_pred(size_t num_iterations)
{
    // During testing, we don't need to use the niv prior
    shared_posterior_mt_.set_use_lss_hyper_prior(false);

    // set up the output dirs 
    boost::format log_dir_fmt(exp_.out_dp + "/%04d/");
    for(size_t i = 0; i < ids_.size(); i++)
    {
        std::string out_dir = (log_dir_fmt % ids_[i]).str();
        ETX(kjb_c::kjb_mkdir(out_dir.c_str()));
        lss_out_dirs_[i] = out_dir;
    }

    bool optimize = true;
    size_t num_lss = lss_set_.lss_vec().size();
    std::vector<std::vector<Linear_state_space> > samples(num_lss);
    std::vector<double> grad_sizes(num_lss, exp_.run.person_grad_size);
    std::string step_name("couple-step");

    typedef Hmc_step_thread<Posterior, Lss_adapter, Lss_gradient<Lss_adapter> >
                                                                    Step_thread;

    bool sample_init_state = exp_.lss_state_dp == "" ? true : false;
    bool sample_clo_params = lss_set_.fixed_clo() ? false : true;
    if(!sample_init_state && !sample_clo_params) return lss_set_;


    // logging file
    std::ofstream iter_log_ofs;
    if(exp_.run.iter_log_fname != "")
    {
        iter_log_ofs.open(exp_.run.iter_log_fname.c_str());
        IFTD(iter_log_ofs.is_open(), IO_error, "can't open file %s", 
                                     (exp_.run.iter_log_fname.c_str()));
    }

    // Disable use_pred for each posteriors
    std::for_each(posteriors_.begin(), posteriors_.end(), 
                  boost::bind(&Posterior::use_pred, _1, false));

    // initial log posterior
    double lp = shared_posterior_mt_(lss_set_);
    double best_lp = lp;

    using namespace boost;

    size_t avail_cores = boost::thread::hardware_concurrency();
    size_t num_threads = exp_.num_person_thrds > avail_cores ? 
                            avail_cores : exp_.num_person_thrds;
    size_t N = ids_.size();

    Lss_set final_best(lss_set_);

    for(size_t i = 0; i < num_iterations; i++)
    {
        // reseed random seed
        kjb_c::kjb_seed_rand_2_with_tod();
        long ltime = time(NULL);
        int stime = (unsigned) ltime /2;  
        srand(stime);
        ergo::global_rng<boost::mt19937>().seed(stime);

        Lss_set best_lss_set(lss_set_);
        Lss_adapter lss_adapter(sample_init_state, sample_clo_params);
        Step_thread step_thrd(
                best_lss_set.lss_vec(), 
                lss_adapter, posteriors_,
                grad_sizes, person_hmc_sizes_,
                lss_out_dirs_,
                samples, 
                step_name,
                exp_.not_record,
                optimize,
                exp_.run.person_num_leapfrog_steps);

        boost::exception_ptr err;
        run_threads(step_thrd, num_threads, N, err);
        lp = shared_posterior_mt_(best_lss_set);
        if(lp > best_lp)
        {
            best_lp = lp;
            final_best = best_lss_set;
        }
        if(!exp_.not_record)
        {
            // Check the best model 
            bst_fs_ << best_lp << "\n";
            bst_fs_.flush();
        }
    }

    // Reset the flag back 
    return final_best;
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

Lss_set Lss_set_sampler::test_model(size_t num_iterations)
{
    // During testing, we don't need to use the niv prior
    shared_posterior_mt_.set_use_lss_hyper_prior(false);
    shared_posterior_mt_.set_use_noise_prior(false);
    shared_posterior_.set_use_lss_hyper_prior(false);
    shared_posterior_.set_use_noise_prior(false);
    // The following is redundent 
    shared_prior_.learn_clo = false;
    shared_prior_.learn_scale = false;

    // logging file
    std::ofstream iter_log_ofs;
    if(exp_.run.iter_log_fname != "")
    {
        iter_log_ofs.open(exp_.run.iter_log_fname.c_str());
        IFTD(iter_log_ofs.is_open(), IO_error, "can't open file %s", 
                                     (exp_.run.iter_log_fname.c_str()));
    }

    // error files
    std::ofstream err_log_ofs;
    if(exp_.run.fit_err_fname != "")
    {
        err_log_ofs.open(exp_.run.fit_err_fname.c_str());
        IFTD(err_log_ofs.is_open(), IO_error, "can't open file %s", 
                                     (exp_.run.fit_err_fname.c_str()));
    }

    // initial log posterior
    double lp = shared_posterior_mt_(lss_set_);
    double best_lp = lp;
    double pre_best_lp = lp;

    using namespace boost;

    size_t avail_cores = boost::thread::hardware_concurrency();
    size_t num_threads = exp_.num_person_thrds > avail_cores ? 
                            avail_cores : exp_.num_person_thrds;

    Lss_set final_best(lss_set_);
    boost::exception_ptr err;

    bool optimize = true;
    size_t num_samples = 200;
    size_t num_lss = lss_set_.lss_vec().size();
    std::vector<std::vector<Linear_state_space> > samples(num_lss);
    size_t N = ids_.size();
    double max_testing_secs = (exp_.run.maximum_running_minutes * 60.0) * 0.80;
    double max_testing_hrs = (exp_.run.maximum_running_minutes/60.0) * 0.80;
    double remain_hrs = (exp_.run.maximum_running_minutes/60.0);
    std::cout << "Remaining hours: " << remain_hrs << std::endl;
    std::cout << "Maximum testing seconds: " << max_testing_secs << std::endl;
    std::cout << "Maximum testing hours: " << max_testing_hrs << std::endl;

    //std::vector<Gaussian_mixture_prior> group_priors(N);
    //std::fill(group_priors.begin(), group_priors.end(), group_prior_);

    Lss_set best_lss_set(lss_set_);
    // creating the sampler 
    bool sample_state = exp_.lss_state_dp == "" && !exp_.lss.ignore_clo ? true : false;
    bool sample_clo = lss_set_.fixed_clo() || exp_.lss.ignore_clo ? false : true; 
    bool sample_poly_terms = (!lss_set_.fixed_clo() 
                              && exp_.lss.polynomial_degree >= 0) ? true : false;

    // compute the variance of the data
    Vector state_variance((int)exp_.lss.num_oscillators, 0.0);
    for(size_t i = 0; i < data_.size(); i++)
    {
        Obs_map::const_iterator it = data_[i].observables.begin();
        std::pair<Vector, Vector> stat = standardize(data_[i],
                                        it->first, false, false);
        state_variance += stat.second; 
    }
    state_variance /= data_.size();
    std::cout << "data state variance: " << state_variance << std::endl;
    // set the state prop sigma based on the data 
    double state_prop_sigma = std::sqrt(std::accumulate(state_variance.begin(), 
                              state_variance.end(), 0.0)/ state_variance.size());
    std::cout << "Testing state prop sigma: " << state_prop_sigma << std::endl;
    // clear all current samples
    best_lss_set.clear_samples();
    

    Drift_step_thread gp_sampler(best_lss_set.lss_vec(), 
                                 posteriors_, 
                                 lss_out_dirs_,
                                 exp_.run.state_prop_sigma, 
                                 exp_.run.poly_term_prop_sigma, 
                                 best_lss_set.all_samples(),
                                 exp_.drift_sampler.ctr_pt_length,
                                 //exp_.drift_sampler.num_burn_iters, 
                                 0,
                                 exp_.drift_sampler.num_sample_iters,
                                 exp_.not_record,
                                 //true,
                                 optimize,
                                 num_samples,
                                 sample_state,
                                 sample_poly_terms,
                                 exp_.run.mh_converge_iter,
                                 max_testing_secs);

    std::cout << sample_clo << " test sample_clo: " << std::endl;
    Mh_step_thread mh_sampler(best_lss_set.lss_vec(), 
                              posteriors_,
                              lss_out_dirs_,
                              exp_.run.state_prop_sigma,
                              exp_.run.clo_param_prop_sigma,
                              exp_.run.poly_term_prop_sigma,
                              best_lss_set.all_samples(),
                              !exp_.not_record,
                              !exp_.not_write_trace,
                              optimize,
                              cluster_prior_,
                              lss_set_.group_params(),
                              exp_.run.sample_cluster,
                              exp_.run.adapt_mh,
                              num_samples,
                              sample_state,
                              sample_clo,
                              sample_poly_terms,
                              exp_.run.mh_converge_iter,
                              max_testing_secs);

    typedef Hmc_step_thread<Posterior, 
                            Lss_adapter, 
                            Lss_gradient<Lss_adapter> > Step_thread;
    std::vector<double> grad_sizes(num_lss, exp_.run.person_grad_size);
    Lss_adapter lss_adapter(sample_state, sample_clo);
    std::string step_name("ind-lss-step");
    Step_thread hmc_sampler(
        best_lss_set.lss_vec(), 
        lss_adapter,
        posteriors_,
        grad_sizes,
        person_hmc_sizes_,
        lss_out_dirs_,
        best_lss_set.all_samples(), 
        step_name,
        exp_.not_record,
        optimize,
        exp_.run.person_num_leapfrog_steps,
        num_samples,
        max_testing_secs
    );

    // We do not need to do any inference if we ignore the CLO part and the
    // polynomial coefficientse are shared.
    if (exp_.lss.ignore_clo && exp_.prior.fixed_clo)
    {
        return final_best; 
    }
    std::cout << "Testing model (total iterations=" << num_iterations << ")" << std::endl;
    boost::progress_display progress_bar(num_iterations);
    for (size_t i = 0; i < num_iterations; i++)
    {
        // Reseed random seed
        kjb_c::kjb_seed_rand_2_with_tod();
        long ltime = time(NULL);
        int stime = (unsigned) ltime/2;  
        srand(stime);
        ergo::global_rng<boost::mt19937>().seed(stime);

        lp = shared_posterior_mt_(best_lss_set);
        std::cout << "iter " << i <<  " : " << lp << std::endl;
        bool exceed_time = false;
        try
        {
            if(exp_.lss.drift)
            {
                std::cout << "lss.drift" << std::endl;
                run_threads(gp_sampler, num_threads, N, err);
                exceed_time = gp_sampler.exceed_runtime();
            }
            else
            {
                if(exp_.run.test_sample_approach == "MH")
                {
                    std::cout << "exp_.run.test_sample_approach == MH" << std::endl;
                    run_threads(mh_sampler, num_threads, N, err);
                    exceed_time = mh_sampler.exceed_runtime();
                }
                else 
                {
                    assert(exp_.run.test_sample_approach == "HMC");
                    run_threads(hmc_sampler, num_threads, N, err);
                    exceed_time = mh_sampler.exceed_runtime();
                }
            }
        }
        catch(boost::exception& err)
        {
            std::cerr << boost::diagnostic_information(err);
        }

        // Check the best model 
        lp = shared_posterior_mt_(best_lss_set);
        std::cout << "iter " << i <<  " : " << lp << std::endl;

        if(exp_.run.sample_cluster && !exp_.data.outcomes.empty())
        {
            best_lss_set.report_outcome_pred_errs(exp_.out_dp);
        }

        if(lp > best_lp)
        {
            best_lp = lp; 
            final_best = best_lss_set;
        }
        bst_fs_ << best_lp << "\n";
        bst_fs_.flush();
        final_best.write(exp_.out_dp);

        // compute fitting error 
        std::vector<Vector> obs_errors;
        Vector error = compute_ave_error(data_, best_lss_set, obs_errors);
        size_t num_osc = exp_.lss.num_oscillators;
        assert(error.size() == 2 * num_osc);
        double fit_err = 0.0;
        double pre_err = 0.0;
        for(size_t j = 0; j < num_osc; j++)
        {
            fit_err += error[j];
        }
        for(size_t j = num_osc; j < num_osc * 2; j++)
        {
            pre_err += error[j];
        }
        //if(!exp_.not_record)
        {
            err_log_ofs << i <<  " " << fit_err << " " << pre_err << "\n";
            err_log_ofs.flush();
        }
        if((i+1) % 15 == 0)
        {
            if(fabs(pre_best_lp - best_lp) < 0.1)
            {
                break;
            }
            else
            {
                pre_best_lp = best_lp;
            }
        }
        if(exceed_time)
        {
            std::cout << "[WARNING]: Exceeding total running time during testing\n";
            std::cout << "[WARNING]: Terminating and writing results after " 
                      << i + 1 << " iterations\n";
            break;
        }

        final_best.write(exp_.out_dp);
        // Reset the best_lss_set
        best_lss_set = lss_set_; 
        best_lss_set.clear_samples();
        ++progress_bar;
    }

    // Close all the training files 
    shared_log_fs_.close();
    shared_bst_fs_.close();
    bst_fs_.close();
    iter_log_ofs.close();
    err_log_ofs.close();

    return final_best;
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

Lss_set Lss_set_sampler::test_gp(size_t num_iterations)
{
    // During testing, we don't need to use the niv prior
    bool use_lss_hyper_prior = shared_posterior_mt_.get_use_lss_hyper_prior();
    shared_posterior_mt_.set_use_lss_hyper_prior(false);

    shared_prior_.learn_clo = false;
    shared_prior_.learn_scale = false;
    std::for_each(posteriors_.begin(), posteriors_.end(), 
                  boost::bind(&Posterior::use_clo_ind_prior, _1, false));
    boost::format log_dir_fmt(exp_.out_dp + "/%04d/");
    for(size_t i = 0; i < ids_.size(); i++)
    {
        std::string out_dir = (log_dir_fmt % ids_[i]).str();
        ETX(kjb_c::kjb_mkdir(out_dir.c_str()));
        lss_out_dirs_[i] = out_dir;
    }

    // Initial log posterior
    double lp = shared_posterior_mt_(lss_set_);
    double best_lp = lp; 
    std::cout << "init lp: " << lp << "\n";

    size_t avail_cores = boost::thread::hardware_concurrency();
    size_t num_threads = exp_.num_person_thrds > avail_cores ? 
                            avail_cores : exp_.num_person_thrds;

    // Create sampler
    Lss_set final_best(lss_set_);
    double max_testing_secs = (exp_.run.maximum_running_minutes * 60.0) * 0.90;
    std::cout << "Maximum testing seconds: " << max_testing_secs << std::endl;
    for(size_t i = 0; i < num_iterations; i++)
    {
        // reseed random seed
        kjb_c::kjb_seed_rand_2_with_tod();
        long ltime = time(NULL);
        int stime = (unsigned) ltime /2;  
        srand(stime);
        ergo::global_rng<boost::mt19937>().seed(stime);

        Lss_set best_lss_set(lss_set_);
        size_t num_lss = lss_set_.lss_vec().size();
        bool optimize = true;
        size_t num_samples = 0;
        bool sample_state = true;
        bool sample_poly_terms = (exp_.lss.polynomial_degree >= 0 && 
                                    !exp_.run.fit_ind_clo) ? true : false;
        std::cout << "sample_poly_terms: " << sample_poly_terms << std::endl;
        Drift_step_thread sampler(best_lss_set.lss_vec(), 
                                  posteriors_, 
                                  lss_out_dirs_,
                                  exp_.run.state_prop_sigma, 
                                  exp_.run.poly_term_prop_sigma, 
                                  best_lss_set.all_samples(),
                                  exp_.drift_sampler.ctr_pt_length,
                                  exp_.drift_sampler.num_burn_iters, 
                                  exp_.drift_sampler.num_sample_iters,
                                  exp_.not_record,
                                  optimize, 
                                  num_samples,
                                  sample_state,
                                  sample_poly_terms,
                                  max_testing_secs);
        try
        {
            boost::exception_ptr ep;
            run_threads(sampler, num_threads, ids_.size(), ep);
        }
        catch(boost::exception& err)
        {
            std::cerr << boost::diagnostic_information(err);
        }
        lp = shared_posterior_mt_(best_lss_set);
        if(lp > best_lp)
        {
            best_lp = lp; 
            final_best = best_lss_set;
        }

        // check the best model 
        bst_fs_ << best_lp << "\n";
        bst_fs_.flush();
    }

    // reset the flag back 
    shared_posterior_mt_.set_use_lss_hyper_prior(use_lss_hyper_prior);

    return final_best;
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

std::vector<std::vector<Linear_state_space> > Lss_set_sampler::pred_samples
(
    const Lss_set& lsss, 
    size_t num_samples
)
{
    const std::vector<Linear_state_space>& lss_vec = lsss.lss_vec();
    std::vector<std::vector<Linear_state_space> > all_samples(lss_vec.size());
    for(size_t i = 0; i < lss_vec.size(); i++)
    {
        all_samples[i] = generate_lss_samples(lss_vec[i], posteriors_[i], num_samples);
    }
    return all_samples;
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

void Lss_set_sampler::generate_output_dirs(const std::string& out_dir) 
{
    std::vector<std::string> out_dirs;
    boost::format log_dir_fmt(exp_.out_dp + "/%04d/");
    assert(lss_out_dirs_.size() == ids_.size());
    for(size_t i = 0; i < ids_.size(); i++)
    {
        std::string sub_out_dir = out_dir;
        if(exp_.data.grouping_var != "")
        {
            size_t group_index = lss_set_.lss_vec()[i].group_index();
            assert(!group_map_.empty());
            assert(group_map_.left.find(group_index) != group_map_.left.end());
            std::string group_name = group_map_.left.find(group_index)->second;
            boost::format cur_out_fmt(out_dir + "/" + group_name + "/%04d/");
            sub_out_dir = (cur_out_fmt % ids_[i]).str();
        }
        else
        {
            sub_out_dir = (log_dir_fmt % ids_[i]).str();
        }
        kjb_c::kjb_mkdir(sub_out_dir.c_str());
        lss_out_dirs_[i] = sub_out_dir;
    }
}
