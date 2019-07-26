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
   |  Author:  Jinyan Guan
 * =========================================================================== */

/* $Id: experiment.h 22559 2019-06-09 00:02:37Z kobus $ */

#ifndef KJB_TIES_EXPERIMENT_H
#define KJB_TIES_EXPERIMENT_H

#include <m_cpp/m_vector.h>
#include <l_cpp/l_filesystem.h>

#include <string>
#include <vector>
#include <utility>

#include "dbn_cpp/coupled_oscillator.h"
#include "dbn_cpp/linear_state_space.h"

namespace kjb {
namespace ties {

struct Data_options
{
    std::string data_dp;
    std::vector<std::string> id_list_fp;
    std::string data_fname;
    std::string csv_fp;
    std::string distinguisher;
    std::string grouping_var;
    mutable size_t data_groups;
    size_t average_size;
    bool standardize_data; 
    bool mean_centered_data; 

    std::vector<std::string> outcomes;

    std::string get_group_info_fp() const
    {
        return data_dp + std::string("/group_info.txt");
    }

};

struct Lss_options
{
    bool drift;
    double init_period;
    double init_damping;
    size_t num_oscillators;
    int polynomial_degree;
    bool ignore_clo;
    size_t num_params;
    bool use_modal;
};

struct Lss_set_options
{
    bool shared_moderator;

    double mod_coef_prior_mean;
    double mod_coef_prior_sigma;

    // parameters for the moderator coefs
    std::vector<std::string> moderators; 
    std::vector<std::string> moderator_params;
    mutable std::vector<Vector> coef_means;
    mutable std::vector<Matrix> coef_covariances;

    /**
     * @brief   Returns all the moderators for the CLO parameters. 
     */
    std::vector<std::vector<std::string> > 
    get_all_moderators
    (   
        bool ignore_clo,
        size_t num_oscillators,
        size_t total_num_params
    ) const;

    /**
     * @brief   Return the means (0.0) of the poly coefficients 
     */
    Vector get_poly_coef_means(double mean = 0.0) const
    {
        int num_mods = moderators.size();
        return Vector(num_mods + 1, mean);
    }

    /**
     * @brief   Return the covariances of the poly coefficients
     */
    Matrix get_poly_coef_covariances
    (
        double sigma = 1.0
    ) const
    {
        int num_mods = moderators.size();
        return create_diagonal_matrix(Vector(num_mods + 1, sigma));
    }
};

struct Prior_options
{
    // file contains the prior information
    std::string prior_fp;

    // priors for the CLO params
    double gp_scale;
    double clo_sigma;
    double clo_sigma_shape;
    double clo_sigma_scale;

    // hyper priors for the GP prior 
    double gp_scale_shape;
    double gp_scale_scale;

    // priors for the polynomial coefs
    double poly_sigma;
    double poly_sigma_shape;
    double poly_sigma_scale;

    // priors for the outcome variable
    double outcome_sigma;
    double outcome_sigma_shape;
    double outcome_sigma_scale;

    // priors for the starting states
    double init_state_mean; 
    double init_state_sigma;

    // hyper priors of the observation variance
    double obs_noise_shape;
    double obs_noise_scale;

    // variance distribution type
    std::string variance_prior_type;

    // CLO is fixed
    bool fixed_clo;
    // variance is log transformed
    bool sigma_log_transformed;
};

struct Cluster_options
{
    // options for the group model
    double group_lambda;
    double group_kappa_o;
    size_t num_groups;
    bool infinite_clusters;
    bool collapsed_gibbs;
    bool fixed_mixture_params;
    bool joint_params;
    bool independent_params;
};

struct Likelihood_options
{
    double init_noise_sigma;
    std::vector<std::string> obs_names;
    double training_percent;
};

struct Run_options
{
    std::string in_dp;
    std::string iter_log_fname;
    std::string fit_err_fname;
    std::string trace_fname;
    bool write_samples;
    bool write_proposals;
    bool read_model;

    // Shared parameter sampling approach
    std::string shared_sample_approach;
    std::string obs_noise_sample_approach;
    std::string obs_coef_sample_approach;

    // HMC options 
    size_t num_hmc_iter;
    size_t shared_num_leapfrog_steps;
    size_t person_num_leapfrog_steps;
    size_t state_num_leapfrog_steps;
    double shared_hmc_size;
    double person_hmc_size;
    double state_hmc_size;
    double noise_sigma_hmc_size;
    double obs_coef_hmc_size;

    // gradident
    double shared_grad_size;
    double person_grad_size;
    double state_grad_size;
    double obs_noise_grad_size;
    double obs_coef_grad_size;
    bool estimate_grad_step;

    // MH propose options
    double state_prop_sigma;
    double clo_param_prop_sigma;
    double poly_term_prop_sigma;
    double mh_bmi_coef_sigma;
    double mh_var_sigma;
    double mh_obs_coef_sigma;
    double mh_noise_sigma;
    size_t mh_converge_iter;

    bool adapt_mh;
    bool fit_ind_clo;
    //bool learn_cluster;
    bool sample_cluster;
    std::string gp_params_fp;
    bool opt_shared_params;

    std::string obs_coef_fp;
    int num_folds;
    size_t num_sampled_length;
    size_t num_converge_iters;
    double maximum_running_minutes;

    // Sampling option during testing
    std::string test_sample_approach;
    std::string person_step_approach;

    bool linear_model;
    std::string run_name;
};

struct Drift_sampler_options
{
    // Drift sampler
    size_t num_sample_iters;
    size_t num_burn_iters;
    size_t ctr_pt_length;
};

struct Ties_experiment
{
    // general info
    Data_options data;
    Lss_options lss;
    Lss_set_options lss_set;
    Prior_options prior;
    Cluster_options cluster;
    Likelihood_options likelihood;
    Run_options run;
    Drift_sampler_options drift_sampler;

    // experiment directory
    std::string out_dp;
    std::string lss_state_dp;
    size_t rand_seed;
    size_t train_num_iterations;
    size_t test_num_iterations;
    size_t num_iterations;
    //size_t num_shared_pos_thrds;
    size_t num_person_thrds;

    // cross validation
    bool fold_thrd;
    size_t fold;

    // logging
    bool not_record;
    bool not_write_trace;
    bool sample_noise_sigma;
    bool write_latex_error;

    // model
    std::string model_name;
    std::string input_sub_dir;

    // options for the characteristics of the model
    size_t character_model_length;

    std::string fold_info_dp;
    bool run_average_model;
    bool run_line_model;
    bool fit_all_data;
};

void generate_model_name(Ties_experiment& exp);

}} // namspace kjb::ties
#endif // KJB_TIES_EXPERIMENT_H

