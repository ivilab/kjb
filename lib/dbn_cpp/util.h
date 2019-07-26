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

/* $Id: util.h 22559 2019-06-09 00:02:37Z kobus $ */

#ifndef TIES_UTIL_H
#define TIES_UTIL_H

#include <vector>
#include <utility>
#include <string>
#include <sstream>
#include <fstream>
#include <algorithm>

#include <boost/numeric/odeint.hpp>
#include <boost/bind.hpp>

#include "dbn_cpp/linear_state_space.h"
#include "dbn_cpp/lss_set.h"
#include "dbn_cpp/data.h"
#include "dbn_cpp/adapter.h"

#include "ergo/hmc.h"
#include "ergo/mh.h"

#include "dbn_cpp/base_line_models.h"


namespace kjb {
namespace ties {

/** @brief  Reads in the topic times from a file. */
std::vector<std::pair<size_t, size_t> > parse_topics
(
    const std::string& topics_fp
);

/**
 * @brief   Parse in the shared params 
 */
std::istream& parse_shared_params
(
    std::istream& ist,
    Group_params& group_params
);

/**
 * @brief   Parse in the shared params 
 */
inline void parse_shared_params
(
    const std::string& shared_fp,
    Group_params& group_params
)
{
    std::ifstream shared_ifs(shared_fp.c_str());
    IFTD(!shared_ifs.fail(), IO_error, 
            "Can't open file %s", (shared_fp.c_str()));
    parse_shared_params(shared_ifs, group_params);
}

/**
 * @brief   Plot the data into the out-dir
 */
void plot_data
(
    const Data& data,
    const std::string& out_dir
);

/** 
 * @brief   Plots the model. 
 */
void plot_model
(
    const Linear_state_space& lss, 
    const std::string& out_dir
);

/** 
 * @brief   Plots the data and model and returns the model 
 *          with predicted states.
 */
Linear_state_space plot_data_and_model
(
    const Data& data,
    const Linear_state_space& lss,
    const std::string& fname,
    bool plot_test = true
);

/** 
 * @brief   Plots the data and model and returns the model 
 *          with predicted states.
 */
std::vector<Linear_state_space> plot_data_and_model
(
    const std::vector<Data>& data,
    const Lss_set& lss_set,
    const std::string& out_dir
);

/** @brief  Plot the data and model in the directory. */
void plot_exp_dir
(
    const std::string& exp_dir, 
    const std::string& list_fp,
    const std::string& data_dir
);

/**
 * @brief   Compute the mean and variances of observable 
 *          specified by the "type"
 * @param   The name of the observable
 */
std::pair<Vector, Vector> get_mean_variances
(
    const Data& data,
    const std::string& type
);

/** 
 * @brief   Standardizes the data to have 1.0 variance and 0.0 mean. 
 * @return  mean, variance
 * @param   if convert is true, the data will be standardized. 
 */
std::pair<Vector, Vector> standardize
(
    Data& data, 
    const std::string& type,
    bool convert_mean = true,
    bool convert_var = false
);

/** 
 * @brief   Standardizes the data to have 1.0 variance and 0.0 mean. 
 * @return  mean, variance
 * @param   if convert is true, the data will be standardized. 
 */
std::pair<Vector, Vector> standardize
(
    std::vector<Data>& data_all, 
    const std::string& type,
    bool convert_mean,
    bool convert_var
);

/**
 * @brief   Given a list of errors for all the couples, compute the mean 
 *          and the variances. 
 */
std::pair<Vector, Vector> compute_mean_sem
(
    const std::vector<Vector>& errors
);

/** @brief  Compute the mean and variance of errors and record them. */
std::pair<Vector, Vector> compute_and_record_errors
(
    const std::vector<Vector>& errors,
    const std::string& distinguisher,
    const std::string& err_fp 
);

/** @brief  Compute the mean and variance of errors and record them. */
std::pair<Vector, Vector> compute_and_record_errors_latex
(
    const std::vector<Vector>& errors,
    const std::string& err_fp 
);

/** @brief  Returns the hmc step size. */
template <typename Adapt>
std::vector<double> get_hmc_step_size
(
    const Lss_set& lss_set, 
    const Adapt& adapter
);

/** @brief  Compute the and returns the error of two lss. */
std::vector<Vector> compute_error
(
    const Linear_state_space& real_lss,
    const Linear_state_space& lss,
    double train_percent = 1.0

);

/** @brief  Compute and return the fitting and predicting errors. */
std::vector<Vector> compute_error
(
    const Data& data, 
    const Linear_state_space& lss,
    double train_percent = 1.0
);

/** @brief  Compute and return the fitting and predicting errors. */
std::vector<Vector> compute_error
(
    const std::vector<Data>& data, 
    const Lss_set& trained_lss_set,
    double train_percent = 1.0
);

/** @brief  Compute and return the average errors of all the observables. */
inline Vector compute_ave_error
(
    const Data& data, 
    const Linear_state_space& lss,
    std::vector<Vector>& all_errors,
    double train_percent = 1.0
)
{
    all_errors = compute_error(data, lss, train_percent);
    if(all_errors.empty()) return Vector();
    Vector ave_error = std::accumulate(all_errors.begin() + 1,
                                       all_errors.end(),
                                       all_errors[0]);
    return ave_error / all_errors.size();
}

/** @brief  Compute and return the average errors of all the observables. */
inline Vector compute_ave_error
(
    const std::vector<Data>& data, 
    const Lss_set& trained_lss_set,
    std::vector<Vector>& all_errors,
    double train_percent = 1.0
)
{
    all_errors = compute_error(data, trained_lss_set, train_percent);
    if(all_errors.empty()) return Vector();
    Vector ave_error = std::accumulate(all_errors.begin() + 1,
                                       all_errors.end(),
                                       all_errors[0]);
    return ave_error / all_errors.size();
}

/** 
 * @brief   Read in the error file. 
 * @return  A vector of the errors of each partner in a couple 
 * 
 */
inline Vector read_couple_error
(
    const std::string& err_fp
)
{
    std::ifstream err_ifs(err_fp.c_str());
    IFTD(err_ifs.is_open(), IO_error,
            "Can't open file %s", (err_fp.c_str()));
    std::string line;
    getline(err_ifs, line);
    std::vector<double> elems;
    std::istringstream first_ist(line);
    std::copy(std::istream_iterator<double>(first_ist), 
              std::istream_iterator<double>(), 
              back_inserter(elems));
    assert(elems.size() >= 1);
    Vector error(elems.size() - 1);
    std::copy(elems.begin() + 1, elems.end(), error.begin());
    return error;
}

/** @brief  Output the error for each couple */
inline void write_couple_error
(
    const std::string& err_fp,
    const std::vector<Vector>& all_errors, 
    const std::vector<Data>& data
)
{
    std::ofstream err_of(err_fp.c_str());
    IFTD(err_of.is_open(), IO_error, 
            "can't open file %s", (err_fp.c_str()));
    for(size_t i = 0; i < all_errors.size(); i++)
    {
        err_of << std::setw(2) << data[i].dyid << " "
               << all_errors[i] << std::endl;
    }
}

/** @brief  Compute and return the average errors of all the observables. */
Vector compute_ave_error
(
    const Data& data, 
    const std::vector<Linear_state_space>& samples,
    std::vector<Vector>& all_errors,
    double train_percent = 1.0
);

/** @brief  Report error files. */
void report_errors
(
    const std::string& err_dp,
    const std::string& err_fp,
    const std::vector<Vector>& error,
    const std::vector<std::vector<Vector> >& obs_error,
    const std::vector<std::string>& obs_names,
    const std::vector<size_t>& dyids = std::vector<size_t>()
);

/** @brief  Report error files. */
void report_cross_validate_errors
(
    const std::string& err_dp,
    const std::string& obs_name,
    const std::string& distinguisher,
    const std::vector<std::string>& fold_dirs,
    size_t K,
    bool write_latex_error
);

/**
 * @brief   Report the difference between two models
 */
void report_model_pair_differences
(
    const std::string& err_dp_1,
    const std::string& err_dp_2,
    const std::string& obs_name,
    const std::string& distinguisher,
    const std::vector<std::string>& fold_dirs
);

/**
 * @brief   Report outcome errors 
 */
void report_outcome_errors
(
    const std::vector<std::string>& couple_err_fp, 
    const std::string& out_fp,
    const std::string& distinguisher
);

/** @brief  Read couple errors. */
bool read_couple_errors
(
     const std::string& lss_dir,
     const std::string& err_fp_str,
     const std::vector<std::string>& obs_names,
     Vector& ave_error,
     std::vector<Vector>& obs_errors
);

/**
 * @brief   Read in errors of multiple pairs in the following format
 *          ID_1, ERR_1A, ERR_1B
 *          ID_2, ERR_2A, ERR_2B
 *          
 */
std::vector<Vector> read_multiple_pair_error
(
    const std::string& in_fp
);

/** @brief  Estimates and computes the init states based on data. */
State_type estimate_init_states
(
    const Data& data, 
    double training_percent,
    const std::string& obs_name,
    const State_type& mean_state = State_type()
);

/** @brief  Estimates and computes the init states based on data. */
State_vec estimate_init_states
(
    const std::vector<Data>& data,
    double training_percent, 
    const std::string& obs_name,
    int polynomial_degree = -1
);

/** @brief  Return whether the moderator is shared or not. */
bool is_shared_moderator
(
    const Mod_map& moderators,
    const std::string& mod_name
);

/**
 * @brief   Estimate the average of the initial state 
 */
inline
State_type estimate_average_init_state
(
    const std::vector<Data>& data, 
    double training_percent,
    const std::string& obs_name
)
{
    if(data.empty()) return State_type();
    State_type init_state = estimate_init_states(data[0], training_percent, obs_name);
    for(size_t i = 1; i < init_state.size(); i++)
    {
        State_type state = estimate_init_states(data[i], training_percent, obs_name);
        for(size_t j = 0; j < state.size(); j++)
        {
            init_state[j] += state[j];
        }
    }
    BOOST_FOREACH(double& val, init_state)
    {
        val /= data.size();
    }

    return init_state;
}

/** @brief  Get the moderator at a certain percentile. 
 *          We only interested in percentile 0.25 and 0.75.
 *          The code right now does not generalize to other percentiles. 
 *
 *          Since there are two state for the percentile: 0.25 and 0.75,
 *          we can convert this problem to compute the all the subsets 
 *          of the moderators. 
 *          For example, if the moderator is "bmi", which is not shared across
 *          partners, therefore, we will have n moderator values, where n is
 *          the number of partners(oscillators). Then, the total number of 
 *          combinations of the moderators is 2^n. 
 *          If the moderator is "bmiave", which is shared among the partners,
 *          then no matter how many partners there are, we will only have one 
 *          "bmiave". Therefore, the number of combinations is 2 (2^1). 
 *
 *          Therefore, the totol number of combinations is 2^n. And the value of 
 *          n depends on whether the specified moderator is shared among the
 *          partners or not (See code for details).
 *
 * */
std::vector<std::vector<std::pair<double, double> > > 
get_percentile_moderator
(
    const std::vector<Data>& data,
    const std::vector<std::string>& moderator_strs,
    size_t num_oscillator,
    bool shared_moderator
);

/** @brief Compute the line for moderator and initial states */ 
std::vector<Vector> get_init_state_least_square_fitting_coef
(
    const std::vector<Data>& data,
    double training_percent,
    const std::vector<std::string>& moderator_strs,
    const std::string& obs_name,
    bool shared_moderator,
    size_t num_oscillators
);

/** @brief  Get the initial state that associate with the moderator. */
std::vector<State_type> get_init_state_from_moderator
(
    const std::vector<Mod_map>& moderators,
    const Line& line 
);

/** @brief  Get the indices sampled time points */
std::vector<size_t> get_sampled_time_indices 
(
    const Double_v& all_times, 
    size_t segment_length
);

/**
 * @brief   Smooth the data using a Gaussian filter 
 */
Data smooth_data(const Data data, size_t length, double sigma);

/** 
 * @brief   Check to see if the obesrvations are in the range of [min max]
 */
bool in_range(const Linear_state_space& lss, double max, double min);

/**
 * @brief   Randomize the starting states
 */
void randomize_starting_states
(   
    std::vector<Linear_state_space>& lss_set, 
    const std::vector<Data>& data_all,
    double training_percent,
    const std::string& obs_name
);

/**
 * @brief  Get the list of ids 
 */
std::vector<std::string> get_list_fps
(
    const std::string& out_dp, 
    const std::string& grouping_var,
    const std::vector<std::string>& data_groups, 
    const std::string& list_fp
);

/*
 * @brief   Read in all the samples
 */
std::vector<Linear_state_space> read_lss_samples
(
    const std::string& sample_dir,
    double start_time
);

/** 
 * @brief   Create the characteristic model based on the percentile values
 *          of the moderators
 */
void create_characteristic_models
(
    const Ties_experiment& exp, 
    const std::vector<Data>& data,
    const Lss_set& lss_set
);

/**
 * @brief   Return the probability of being in each cluster
 *          p(z_i | theta, outcome, theta_prior, outcome_prior)
 */
std::vector<double> get_responsibilities
(
    const std::vector<Group_params>& group_params,
    const Linear_state_space& lss,
    bool exclude_outcome = false
);

/**
 * @brief   Compute the errors of the predicted outcome variables
 */
std::vector<Vector> compute_outcome_pred_error
(
    const std::vector<Group_params>& group_params, 
    const Linear_state_space& lss,
    bool average,
    bool prior
);

inline std::ostream& operator <<
(
    std::ostream& ost, 
    const Group_params& group_param 
)
{
    ost << "variance" << std::endl; 
    std::setw(15);
    std::setprecision(10);
    std::copy(group_param.variances.begin(), 
              group_param.variances.end(),
              std::ostream_iterator<double>(ost, " " ));
    ost << std::endl;
    ost << "coef" << std::endl;
    std::copy(group_param.pred_coefs.begin(), 
              group_param.pred_coefs.end(), 
              std::ostream_iterator<Vector>(ost, "\n"));
    ost << "weight" << std::endl;
    ost << group_param.group_weight;
    ost << std::endl;
    return ost;
}

/**
 * @brief   Return the cluster assignment
 */
inline std::vector<size_t> get_cluster_assignments(const Lss_set& lss_set)
{
    std::vector<size_t> assigns(lss_set.lss_vec().size());
    for(size_t i = 0; i < lss_set.lss_vec().size(); i++)
    {
        assigns[i] = lss_set.lss_vec()[i].group_index();
    }
    return assigns;
}

inline Vector build_vector
(
    size_t clo_params, 
    double clo_val,
    size_t poly_params,
    double poly_val,
    size_t outcome_params,
    double outcome_val
)
{
    int n = clo_params + poly_params + outcome_params;
    Vector v(n, 0.0);
    size_t i = 0; 
    while(i < clo_params)
    {
        v[i] = clo_val;
        i++;
    }
    while(i < clo_params + poly_params)
    {
        v[i] = poly_val;
        i++;
    }
    while(i < n)
    {
        v[i] = outcome_val;
        i++;
    }
    std::cout << v << std::endl;
    return v;
}


/**
 * @brief  Construct the covariance of coefficients 
 */
inline std::vector<Matrix> get_coef_covariance
(
    const std::vector<Vector>& pred_coefs,
    double init_sigma
)
{
    std::vector<Matrix> cov(pred_coefs.size());
    for(size_t i = 0; i < pred_coefs.size(); i++)
    {
        cov[i] = create_diagonal_matrix(
                    Vector((int)pred_coefs[i].size(), 
                    init_sigma));
    }

    return cov;
}

/** @brief  Return false if the lss has nonneagive frequencies */
inline bool valid_frequency(const Linear_state_space& lss)
{
    BOOST_FOREACH(const Coupled_oscillator& clo, lss.coupled_oscillators())
    {
        for(size_t i = 0; i < clo.num_params(); i++)
        {
            if(i % lss.num_oscillators() == 0)
            {
                if(clo.get_param(i) >= 0.0)
                {
                    //std::cout << i << " " << clo.get_param(i) << std::endl;
                    return false;
                }
            }
        }
        if(!lss.allow_drift())
        {
            return true;
        }
    }
    return true;
}

/** 
 * @brief  Adapts the hmc step size based on the acceptance probability 
 *         and rate. 
 */
#ifdef KJB_HAVE_ERGO
template <typename Model, typename rng_t, bool ACCEPT_STEP, bool REVERSIBLE>
void adapt_hmc_step_size
(
    ergo::hmc_step<Model, rng_t, ACCEPT_STEP, REVERSIBLE>& step,
    std::ostream& log_fs,
    size_t cur_num_iters,
    size_t& num_accepted,
    size_t iter_to_check = 20,
    bool not_record = true
)
{
    if(!step.accepted())
    {
        // not accepted or the acceptance probability is nan
        if(step.acceptance_probability() < -500  || 
           std::isnan(step.acceptance_probability()))
        {
            std::vector<double> step_sizes = step.step_sizes();
            // The step size should not be smaller than 1e-15 
            if(step_sizes[0] > 1e-15)
            {
                if(!not_record) 
                {
                    log_fs << "too high " << step_sizes[0]; 
                }
                double ratio = 1.0 + kjb_c::kjb_rand() * 9.0;
                std::transform(step_sizes.begin(),
                               step_sizes.end(),
                               step_sizes.begin(),
                               std::bind2nd(std::divides<double>(), ratio));
                step.reset(step_sizes);
                if(!not_record) 
                {
                    log_fs << " after " << step.step_sizes().front() 
                           << std::endl; 
                }
            }
        }
    }
    else
    { 
        //accepted, but probability is too low  
        if(fabs(step.acceptance_probability()) < 1e-5)
        {
            std::vector<double> step_sizes = step.step_sizes();
            double ratio = kjb_c::kjb_rand() * 1e5;
            if(!not_record)
            {
                log_fs << "too low: " << step_sizes[0];
            }
            std::transform(step_sizes.begin(),
                           step_sizes.end(),
                           step_sizes.begin(),
                           std::bind2nd(std::multiplies<double>(), ratio));
            step.reset(step_sizes);
            if(!not_record)
            {
                log_fs << " after " << step.step_sizes().front() << std::endl; 
            }
        }
        num_accepted++;
    }

    if((cur_num_iters + 1) % iter_to_check == 0)
    {
        double p_accept = num_accepted * 1.0 / iter_to_check; 
        num_accepted = 0;
        if(p_accept > 0.9)
        {
            double ratio = 1.0 + kjb_c::kjb_rand() * 5.0;
            std::vector<double> step_sizes = step.step_sizes();
            log_fs << "acceptance too high: " << step_sizes[0]; 
            std::transform(step_sizes.begin(),
                           step_sizes.end(),
                           step_sizes.begin(),
                           std::bind2nd(std::multiplies<double>(), ratio));
            step.reset(step_sizes);
            if(!not_record)
            {
                log_fs << " after " << step.step_sizes().front() << std::endl; 
            }
        }
        else if(p_accept < 0.1)
        {
            std::vector<double> step_sizes = step.step_sizes();
            if(!not_record)
            {
                log_fs << "acceptance too low: " << step_sizes[0]; 
            }
            if(step_sizes[0] > 1e-13)
            {
                double ratio = 1.0 + kjb_c::kjb_rand() * 9.0;
                std::transform(step_sizes.begin(),
                               step_sizes.end(),
                               step_sizes.begin(),
                               std::bind2nd(std::divides<double>(), ratio));
                step.reset(step_sizes);
            }
            if(!not_record)
            {
                log_fs << " after " << step.step_sizes().front() << std::endl; 
            }
        }
    }
}
#endif 

/** @brief  Uses samples to compute the covariance matrix. */
template <typename Model, typename Adapter>
Matrix compute_covariance
(
    const std::vector<Model>& samples, 
    const Adapter& adapter
)
{
    if(samples.empty()) return Matrix();
    size_t N = adapter.size(&samples[0]);
    Matrix M(N, N);

    // compute the means
    std::vector<double> means(N, 0.0);
    size_t num_samples = samples.size();
    for(size_t i = 0; i < N; i++)
    {
        BOOST_FOREACH(const Model& m, samples)
        {
            means[i] += adapter.get(&m, i);
        }
        means[i] /= num_samples;
    }

    // compute the covariance
    for(size_t i = 0; i < N; i++)
    {
        // since the covariance is symmetric, we will compute one
        // half, and fill in the other other
        for(size_t j = 0; j <= i; j++)
        {
            double val = 0.0;
            for(size_t k = 0; k < samples.size(); k++)
            {
                const Model& m = samples[k];
                val += (adapter.get(&m, i) - means[i]) * 
                    (adapter.get(&m, j) - means[j]);
            }
            val /= (num_samples-1);
            M(i, j) = val;
            if(i != j)
            {
                M(j, i) = M(i, j);
            }
        }
    }

    return M;
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

template <typename Adapt>
std::vector<double> get_hmc_step_size
(
    const Lss_set& lss_set,
    const Adapt& adapter
)
{
    std::vector<double> steps(adapter.size(&lss_set));
    for(size_t i = 0; i < steps.size(); i++)
    {
        steps[i] = std::max(1e-6, fabs(adapter.get(&lss_set, i))/100.0);
    }
    return steps;
}


Matrix construct_diagonal_matrix(const std::vector<Matrix>& Ks);

}} //namespace kjb::ties

namespace boost { 
namespace numeric { 
namespace odeint {

template<>
struct is_resizeable<kjb::Vector> 
{
    typedef boost::true_type type;
    static const bool value = type::value;
};
}}} // boost::numeric::odeint

#endif

