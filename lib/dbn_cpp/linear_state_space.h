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

/* $Id: linear_state_space.h 22559 2019-06-09 00:02:37Z kobus $ */

#ifndef KJB_TIES_LINEAR_STATE_SPACE_H
#define KJB_TIES_LINEAR_STATE_SPACE_H

#include <l/l_sys_debug.h>
#include <l_cpp/l_util.h>
#include <l_cpp/l_exception.h>
#include <m_cpp/m_matrix.h>
#include <m_cpp/m_vector.h>
#include <gp_cpp/gp_prior.h>
#include <gp_cpp/gp_base.h>
#include <gp_cpp/gp_mean.h>
#include <gp_cpp/gp_covariance.h>
#include <gp_cpp/gp_sample.h>

#include <utility>
#include <map>
#include <iterator>
#include <algorithm>
#include <functional>

#include <boost/lexical_cast.hpp>
#include <boost/foreach.hpp>
#include <boost/thread.hpp>
#include <boost/bimap.hpp>
#include <boost/bimap/list_of.hpp>
#include <boost/assign.hpp>

#include "dbn_cpp/coupled_oscillator.h"
#include "dbn_cpp/typedefs.h"
#include "dbn_cpp/data.h"

namespace kjb {
namespace ties {

static const double DEFAULT_SAMPLING_RATE = 0.01;
static const double DEFAULT_NOISE_SIGMA = 0.5;

//enum Moderator_opt {INDEPENDENT, AVE, DIFF, LOG, NUM_OPT_TYPES};
typedef std::map<std::string, Double_v> Mod_map;


#ifdef KJB_HAVE_TBB
typedef std::vector<Coupled_oscillator, 
        tbb::scalable_allocator<Coupled_oscillator> > Coupled_oscillator_v;
#else 
typedef std::vector<Coupled_oscillator> Coupled_oscillator_v;
#endif

/** 
 * @class   A Linear_state_space class models the dynamics of 
 *          interpersonal emotion system
 */
class Linear_state_space
{
public:
    /** @brief   Constructs a Linear_state_space. */
    Linear_state_space
    (
        size_t num_oscillators = 2,
        size_t num_observables = 1,
        int polynomial_degree = -1,
        size_t num_outcomes = 0
    ) : 
        num_oscillators_(num_oscillators), 
        obs_coefs_(num_observables, 
                std::vector<Vector>(num_oscillators_, Vector(1, 1.0))),
        obs_noise_sigmas_((int)num_observables, DEFAULT_NOISE_SIGMA),
        changed_index_(0),
        poly_coefs_dirty_(true),
        obs_coefs_dirty_(true),
        ignore_clo_(false),
        group_index_(0)
    {
        if(polynomial_degree >= 0)
        {
            int D = polynomial_degree + 1;
            polynomial_coefs_.resize(num_oscillators, Vector(D, 0.0));
        }

        if(num_outcomes > 0)
        {
            //indexed by [outcome-type][oscillator-index]
            outcome_means_.resize(num_outcomes, 
                                  Vector((int)num_oscillators, 0.0));
            outcome_variances_.resize(num_outcomes, 
                                      Vector((int)num_oscillators, 10.0));
            outcomes_.resize(num_outcomes, Vector((int)num_oscillators, 0.0));
        }
    }

    /** @brief   Constructs a Linear_state_space. */
    Linear_state_space
    (
        const Double_v& times, 
        const State_type& init_state,
        const Coupled_oscillator_v& clos,
        const std::vector<std::string>& obs_names, 
        const Vector& noise_sigma = Vector(1, DEFAULT_NOISE_SIGMA),
        int polynomial_degree = -1,
        const std::vector<std::string>& outcome_names = 
                                std::vector<std::string>(),
        const Mod_map& outcomes = Mod_map(),
        int group = 0,
        bool ignore_clo = false
    ); 

    /** @brief   Read in a Linear_state_space from a file. */
    void read 
    (
        const std::string& indir,
        double start_time = 0
    );

    /** @brief   Return the time points */
    const Double_v& get_times() const { return times_; }

    /** @brief   Set the duration of the Linear_state_space. */
    void update_times(const Double_v& times, bool ow = false)
    {
        size_t old_size = times_.size();
        if(old_size < times.size())
        {
            // update the parameters 
            // when allow drift get_clo_params(times) will return the MEAN of 
            // the predicted params 
            bool use_modal = clos_.front().use_modal();
            Double_vv params = get_clo_params(times);
            size_t new_size = times.size();
            clos_.clear();
            BOOST_FOREACH(const Double_v& param, params)
            {
                clos_.push_back(Coupled_oscillator(param, use_modal));
            }

            times_ = times;
            update_obs_states();
        }
        else
        {
            times_ = times;
            if(!ow)
            {
                std::cout << " Shorten the times for Linear state space.\n";
                size_t new_size = times.size();
                changed_index_ = new_size - 1; 
                clo_states_.resize(new_size);
                clos_.resize(changed_index_);
                if(!poly_states_.empty()) poly_states_.resize(new_size);
                obs_states_.resize(new_size);
            }
            else
            {
                changed_index_ = 0;
                update_obs_states();
            }
        }
    }

    /** @brief   Updates the GP prior. */
    void update_gp() const; 

    /** @brief   Initialize the GP prior with mean functions. */
    void init_gp
    (
        const Double_v& gp_scales,
        const Double_v& gp_sigvars,
        const std::vector<gp::Constant>& mean_funcs 
    );

    /** @brief   Initialize the predictors. */
    void init_predictors
    (
        const Mod_map& mod_map,
        const std::vector<std::vector<std::string> >& mod_names
    );

    /** @brief   Return the drifted coupled oscillators. */
    Coupled_oscillator_v& coupled_oscillators() const 
    { 
        return clos_; 
    }

    /** @brief   Return the number of CLO params */
    size_t num_clo_params() const
    {
        if(clos_.empty()) return 0;
        return clos_.front().params().size();
    }

    /** @brief   Return multivariate observation-noise free states*/
    const State_vec_vec& get_states() const
    {
        update_obs_states();
        return obs_states_;
    }

    /** @brief   Return the states based on the times */
    State_vec_vec get_states(const std::vector<size_t>& time_indices) const
    {
        State_vec_vec obs_states;
        State_vec clo_states = get_clo_states(time_indices);
        if(!polynomial_coefs_.empty())
        {
            State_vec poly_states;
            Double_v times = get_time_values(time_indices);
            compute_poly_states(clo_states, poly_states, times);
            compute_obs_states(obs_states, poly_states);
            assert(poly_states.size() == times.size());
        }
        else
        {
            compute_obs_states(obs_states, clo_states);
        }
        return obs_states;
    }

    /** @brief   Return the intial state */
    State_type& init_state() const
    {
        assert(!clo_states_.empty());
        return clo_states_[0];
    }

    /**  
     * @brief    Return changed index, need to update all the states 
     *           from time changed_index to the end of time points
     */
    size_t& changed_index() const { return changed_index_; }

    /** @brief  Return true if the parameters are allowed to drift. */
    bool allow_drift() const { return drift_; }

    /** @brief  Return the observeble names. */
    const std::vector<std::string>& obs_names() const { return obs_names_; }

    /** @brief  Return the outcome variable names. */
    const std::vector<std::string>& outcome_names() const { return outcome_names_; }

    /** @brief  Return the observed states coefficients. */
    const std::vector<std::vector<Vector> >& obs_coefs() const { return obs_coefs_; }

    /** @brief  Set the observation coefs */
    void set_obs_coef(size_t i, size_t j, const Vector& val) const
    {
        IFT(i < obs_coefs_.size() && j < obs_coefs_[i].size(),
                Illegal_argument, "index size is out of bound.");
        obs_coefs_[i][j] = val;
        obs_coefs_dirty_ = true;
    }

    /** @brief  Set the observation coefs */
    void set_obs_coef(size_t i, size_t j, size_t k, double val) const
    {
        IFT(i < obs_coefs_.size() && j < obs_coefs_[i].size() &&
                k < obs_coef_dim(),
                Illegal_argument, "index size is out of bound.");
        obs_coefs_[i][j][k] = val;
        obs_coefs_dirty_ = true;
    }

    /** @brief  Returns the dimension of the obs coef 
     *          (length of the each coef)
     */
    size_t obs_coef_dim() const
    {
        if(obs_names_.size() == 1) return 0;
        return obs_coefs_[0][0].size();
    }

    /** @brief  Returns the number of observed coefficients. */
    size_t num_obs_coefs() const 
    {
        assert(obs_names_.size() >= 1);
        return (obs_names_.size() - 1) * num_oscillators_ * 
               obs_coef_dim();
    }

    /**
     * @brief   Return the ith param at time 
     */
    double get_param(size_t index, int time = -1) const;

    /** brief   Returns the number of oscillators. */
    size_t num_oscillators() const { return num_oscillators_; }

    const Double_v& gp_scales() const { return gp_scales_; }
    const Double_v& gp_sigvars() const { return gp_sigvars_; }

    /** @brief  Returns the predictors. */
    const std::vector<Vector>& get_predictors() const 
    {
        if(predictors_.empty())
        {
            KJB_THROW_2(Runtime_error, "predictors are not initialized");
        }
        return predictors_;
    }

    /** @brief  Return a reference for the predictor */
    std::vector<Vector>& predictors() 
    {
        return predictors_;
    }

    /** @brief  Writes out the Linear State Model an output directory. */
    void write(const std::string& outpath) const;

    /** 
     * @brief  Set the mean and variance of the normal prior. 
     * @param  i     index of the parameter
     * @param  mean  mean of the prior distribution
     */
    void set_clo_mean(size_t i, double mean) const
    {
        size_t num_params = clos_.front().params().size();
        if(clo_means_.size() != num_params)
        {
            clo_means_.resize(num_params);
        }
        IFT(i < num_params, Illegal_argument, "Index is out of bound.");
        clo_means_[i] = mean; 
    }

    /** 
     * @brief  Get the mean and variance of the normal prior. 
     * @param  i     index of the parameter
     */
    double get_clo_mean(size_t i) const
    {
        IFT(i < clos_.front().params().size(), Illegal_argument, 
                "Index is out of bound.");
        return clo_means_[i]; 
    }

    /** 
     * @brief  Set the mean and variance of the normal prior. 
     * @param  i     index of the parameter
     * @param  variance variance of the prior distribution
     */
    void set_clo_variance(size_t i, double variance) const
    {
        size_t num_params = clos_.front().params().size();
        if(clo_variances_.size() != num_params)
        {
            clo_variances_.resize(num_params);
        }
        IFT(i < num_params, Illegal_argument, "Index is out of bound.");
        clo_variances_[i] = variance; 
    }

    /** 
     * @brief  Get the mean and variance of the normal prior. 
     * @param  i     index of the parameter
     */
    double get_clo_variance(size_t i) const
    {
        IFT(i < clos_.front().params().size(), Illegal_argument, 
                "Index is out of bound.");
        return clo_variances_[i];
    }

    /** @brief  Update the parameters of the full gaussian prior. */
    void update_full_gaussian_prior
    (
        const Vector& new_mean,
        const Matrix& new_covariance
    ) const
    {
        means_ = new_mean;
        covariance_ = new_covariance;
    }

    /** @brief  Return the normal prior (assuming each param is independent). */
    double get_clo_prior() const;

    /** @brief  Return the normal prior (full covariance) */ 
    double get_full_gaussian_prior() const
    {
        if(clos_.empty()) return 0.0;
        MV_gaussian_distribution dist(means_, covariance_);
        Vector v(clos_[0].begin(), clos_[0].end());
        return log_pdf(dist, v);
    }

    /** 
     * @brief  Set the scale parameter of GP prior. 
     * @param  i   index of the parameter
     * @param  sc  the scale parameter
     */
    void set_gp_scale(size_t i, double sc) const 
    { 
        IFT(i < gp_scales_.size(), Illegal_argument, 
                "Index is bigger than the number of gp scales");
        if(fabs(sc - gp_scales_[i]) > FLT_EPSILON)
        {
            gp_scales_[i] = sc; 
            gp_changed_[i] = true;
        }
    }
    
    /** @brief  Set the signal variance of the GP prior. */
    void set_gp_sigvar(size_t i, double sv) const 
    { 
        IFT(i < gp_sigvars_.size(), Illegal_argument, 
                "Index is bigger than the number of gp sigvars");
        if(fabs(sv - gp_sigvars_[i]) > FLT_EPSILON)
        {
            gp_sigvars_[i] = sv; 
            gp_changed_[i] = true;
        }
    }

    /** @brief  Set the mean function for the i'th gp_prior. */
    void set_gp_mean(size_t i, const gp::Constant& mean_fun)
    {
        IFT(i < gp_priors_.size(), Illegal_argument, 
                "Index is bigger than the number of gp means");
        gp_priors_[i].set_mean_function(mean_fun);
    }

    /** @brief  Returns the smooth prior. */
    double get_drift_prior() const;

    /** @brief  Returns gp_input_ index.  */
    const gp::Inputs& get_gp_inputs() const { return gp_inputs_; }

    /** @brief  Returns the entire gp_outputs. */
    std::vector<Vector> get_gp_outputs() const
    {
        //size_t N = clos_.front().params().size();
        size_t N = clos_.size();
        std::vector<Vector> outputs; 
        outputs.reserve(N);
        BOOST_FOREACH(const Coupled_oscillator& clo, clos_)
        {
            const Double_v& params = clo.params();
            outputs.push_back(Vector(params.begin(), params.end()));
        }
        return get_transpose(outputs); 
    }

    /** @brief  Returns the outputs for the given inputs. */
    std::vector<Vector> get_gp_outputs(const gp::Inputs& inputs) const
    {
        size_t N = inputs.size();
        std::vector<Vector> outputs(N);
        for(size_t i = 0; i < N; i++)
        {
            size_t t = static_cast<size_t>(inputs[i][0]);
            IFT(t < clos_.size(), Runtime_error, 
                    "Controlled inputs is greater than actual input size.");
            outputs[i] = Vector(clos_[t].params().begin(), 
                                clos_[t].params().end());
        }
        return get_transpose(outputs); 
    }

    /** @brief  Returns the mean function of the Gp prior. */
    const gp::Constant& get_mean_function(size_t i)  const
    {
        IFT(i < gp_priors_.size(), Illegal_argument, 
                "Index out of bound of the gp priors.");
        return gp_priors_[i].mean_function();
    }

    /** @brief  Returns the covariance function of the Gp prior. */
    const gp::Squared_exponential& get_covariance_function(size_t i) const
    {
        update_gp();
        IFT(i < gp_priors_.size(), Illegal_argument, 
                "Index out of the bound of the gp priors.");
        return gp_priors_[i].covariance_function();
    }

    /** @brief  Retuns the GP prior. */
    const Gpp_v& gp_priors() const 
    { 
        update_gp(); 
        return gp_priors_; 
    }

    /** @brief  Returns the polynomial_coefs */
    const std::vector<Vector>& polynomial_coefs() const { return polynomial_coefs_; }

    void set_polynomial_coefs(size_t i, size_t j, double val ) 
    { 
        IFT(i < polynomial_coefs_.size(), Illegal_argument, "Out of bound.");
        IFT(j < polynomial_coefs_.front().size(), Illegal_argument, 
                "Out of bound.");
        polynomial_coefs_[i][j] = val; 
        poly_coefs_dirty_ = true;
        obs_coefs_dirty_ = true;
    }

    /** @brief  Returns the size of the polynomial coefs .*/
    size_t num_polynomial_coefs() const 
    { 
        size_t N = 0;
        BOOST_FOREACH(const Vector& v, polynomial_coefs_)
        {
            N += v.size(); 
        }
        return N;
    }

    /** @brief  Return the number of outcome types
     */
    size_t num_outcome_type() const
    {
        if(outcome_means_.empty()) return 0;
        return outcome_means_.size(); 
    }

    /**
     * @brief   Return the total number of outcome
     */
    size_t num_outcomes() const
    {
        size_t N = 0;
        BOOST_FOREACH(const Vector& v, outcome_means_)
        {
            N += v.size(); 
        }
        return N;
    }

    /** 
     * @brief  Return the size of the polynomial coefs
     */
    int polynomial_dim_per_osc() const 
    {
        if(polynomial_coefs_.empty()) return 0;
        return polynomial_coefs_.front().size();
    }

    /** 
     * @brief  Return the polynomial degree .
     *          0 --- p_0
     *          1 --- p_0 + t p_1
     *          2 --- p_0 + t p_1 + t^2 p_2
     */
    int polynomial_degree() const 
    {
        if(polynomial_coefs_.empty()) return -1;
        return polynomial_coefs_.front().size() - 1;
    }

    /** 
     * @brief  Set the mean and variance of the normal prior. 
     * @param  i     index of the parameter
     * @param  mean  mean of the prior distribution
     */
    void set_polynomial_coefs_mean
    (   
        size_t osc_index, 
        size_t poly_index,
        double mean
    ) const
    {
        if(polynomial_coefs_means_.size() != num_oscillators_)
        {
            polynomial_coefs_means_.resize(num_oscillators_);
        }
        size_t num_poly = polynomial_coefs_.front().size();
        if(polynomial_coefs_means_[osc_index].size() != num_poly)
        {
            polynomial_coefs_means_[osc_index].resize(num_poly);
        }

        IFT(osc_index < polynomial_coefs_means_.size(), Illegal_argument, 
                "Index is out of bound.");
        IFT(poly_index < polynomial_coefs_means_[osc_index].size(), 
                Illegal_argument, 
                "Index is out of bound.");
        polynomial_coefs_means_[osc_index][poly_index] = mean; 
    }

    /** 
     * @brief  Get the mean and variance of the normal prior. 
     * @param  i     index of the parameter
     * @param  mean  mean of the prior distribution
     */
    double get_polynomial_coefs_mean(size_t osc_index, size_t param_index) const
    {
        IFT(osc_index < polynomial_coefs_means_.size(), Illegal_argument, 
                "Index is out of bound.");
        IFT(osc_index < polynomial_dim_per_osc(), Illegal_argument, 
                "Index is out of bound.");
        return polynomial_coefs_means_[osc_index][param_index]; 
    }

    /** 
     * @brief  Set the mean and variance of the normal prior. 
     * @param  i     index of the parameter
     * @param  variance variance of the prior distribution
     */
    void set_polynomial_coefs_var
    (
        size_t osc_index, 
        size_t param_index, 
        double val
    ) const
    {
        if(polynomial_coefs_variances_.size() != num_oscillators_)
        {
            polynomial_coefs_variances_.resize(num_oscillators_, 
                    Vector(polynomial_dim_per_osc(), 0.0));
        }
        IFT(osc_index < polynomial_coefs_variances_.size(), Illegal_argument, 
                "Index is out of bound.");
        IFT(param_index < polynomial_dim_per_osc(), Illegal_argument, 
                "Index is out of bound.");
        polynomial_coefs_variances_[osc_index][param_index] = val; 
    }

    /** 
     * @brief  Get the mean and variance of the normal prior. 
     * @param  i     index of the parameter
     * @param  variance variance of the prior distribution
     */
    double get_polynomial_coefs_var(size_t osc_index, size_t param_index) const
    {
        IFT(osc_index < polynomial_coefs_variances_.size(), Illegal_argument, 
                "Index is out of bound.");
        IFT(osc_index < polynomial_dim_per_osc(), Illegal_argument, 
                "Index is out of bound.");
        return polynomial_coefs_variances_[osc_index][param_index];
    }

    /**
     * @brief   Get the normal prior for the polynomial coefs 
     */
    double get_polynomial_prior() const;


    /** @brief  Get the index of the oscillator */
    size_t get_outcome_osc_index(size_t index) const
    {
        return index % num_oscillators();
    }

    /** @brief  Get the index of the outcome type */
    size_t get_outcome_type_index(size_t index) const
    {
        return index / num_oscillators();
    }

    /** 
     * @brief  Set the mean of the normal prior of the outcome 
     * @param  index    index of the oscillator
     * @param  mean  mean of the prior distribution
     */
    void set_outcome_mean(size_t index, double mean) const
    {
        size_t osc_index = get_outcome_osc_index(index);
        size_t outcome_index = get_outcome_type_index(index);
        IFT(outcome_index < outcome_means_.size(), Illegal_argument, 
                "Index is out of bound.");
        IFT(osc_index < outcome_means_[outcome_index].size(), Illegal_argument, 
                "Index is out of bound.");
        outcome_means_[outcome_index][osc_index] = mean; 
    }

    /** 
     * @brief  Get the mean of the normal prior of the outcome
     * @param  index     index of the parameter
     * @param  mean           mean of the prior distribution
     */
    double get_outcome_mean(size_t index) const
    {
        size_t osc_index = get_outcome_osc_index(index);
        size_t outcome_index = get_outcome_type_index(index);
        IFT(outcome_index < outcome_means_.size(), Illegal_argument, 
                "Index is out of bound.");
        IFT(osc_index < outcome_means_[outcome_index].size(), Illegal_argument, 
                "Index is out of bound.");
        return outcome_means_[outcome_index][osc_index];
    }

    /** 
     * @brief  Set the variance of the normal prior. 
     * @param  index  index of the oscillator 
     * @param  val        variance of the prior distribution
     */
    void set_outcome_var(size_t index, double val) const
    {
        size_t osc_index = get_outcome_osc_index(index);
        size_t outcome_index = get_outcome_type_index(index);
        IFT(outcome_index < outcome_variances_.size(), Illegal_argument, 
                "Index is out of bound.");
        IFT(osc_index < outcome_variances_[outcome_index].size(), Illegal_argument, 
                "Index is out of bound.");
        outcome_variances_[outcome_index][osc_index] = val;
    }

    /** 
     * @brief  Get the variance of the normal prior of the outcome
     * @param  index   index of the oscillator
     */
    double get_outcome_var(size_t index) const
    {
        size_t osc_index = get_outcome_osc_index(index);
        size_t outcome_index = get_outcome_type_index(index);
        IFT(outcome_index < outcome_variances_.size(), Illegal_argument, 
                "Index is out of bound.");
        IFT(osc_index < outcome_variances_[outcome_index].size(), Illegal_argument, 
                "Index is out of bound.");
        return outcome_variances_[outcome_index][osc_index];
    }

    /**
     * @brief   Generate a sample for the outcome 
     */
    double sample_outcome(size_t outcome_index, size_t osc_index)
    {
        IFT(outcome_index < outcomes_.size(), Illegal_argument, 
                "Index is out of bound.");
        IFT(osc_index < outcomes_[outcome_index].size(), Illegal_argument, 
                "Index is out of bound.");
        Normal_distribution p(outcome_means_[outcome_index][osc_index], 
                              std::sqrt(outcome_variances_[outcome_index][osc_index]));
        double val = sample(p);
        if(outcomes_.size() != outcome_variances_.size())
        {
            outcomes_.resize(outcome_variances_.size(), outcome_means_.front());
        }
        outcomes_[outcome_index][osc_index] = val;
        return val;
    }
   
    /**
     * @brief   Return the outcome value
     */
    double get_outcome(size_t outcome_index, size_t osc_index) const
    {
        IFT(outcome_index < outcomes_.size(), Illegal_argument, 
                "Index is out of bound.");
        IFT(osc_index < outcomes_[outcome_index].size(), Illegal_argument, 
                "Index is out of bound.");
        return outcomes_[outcome_index][osc_index];
    }

    /**
     * @brief   Return the outcome variable at index
     */
    double get_outcome(size_t index) const
    {
        size_t osc_index = get_outcome_osc_index(index);
        size_t outcome_index = get_outcome_type_index(index);
        return get_outcome(outcome_index, osc_index);
    }

    /**
     * @brief   set the outcome value
     */
    void set_outcome(size_t outcome_index, size_t osc_index, double val)
    {
        IFT(outcome_index < outcomes_.size(), Illegal_argument, 
                "Index is out of bound.");
        IFT(osc_index < outcomes_[outcome_index].size(), Illegal_argument, 
                "Index is out of bound.");
        outcomes_[outcome_index][osc_index] = val;
    }

    /**
     * @brief  set the outcome value
     */
    void set_outcome(size_t index, double val)
    {
        size_t osc_index = get_outcome_osc_index(index);
        size_t outcome_index = get_outcome_type_index(index);
        set_outcome(outcome_index, osc_index, val);
    }

    /** @brief  Returns the noise sigmas .*/
    Vector& noise_sigmas() const { return obs_noise_sigmas_; }

    void parse_obs_coef(const std::string& obs_fname) const;

    /** @brief  Get the time points that are at the specific indices*/
    Double_v get_time_values(const std::vector<size_t>& indices) const
    {
        assert(indices.size() <= times_.size());
        Double_v sampled_times(indices.size());
        size_t i = 0;
        BOOST_FOREACH(size_t index, indices)
        {
            assert(index < times_.size());
            sampled_times[i++] = times_[index]; 
        }
        return sampled_times;
    }

    /** @brief  Convension to a Vector type */
    operator Vector() const 
    {
        if(clos_.empty()) return Vector();
        const Double_v& params = clos_[0].params();
        /*Vector temp(params.size());
        size_t param_per_osc = 1 + num_oscillators_;
        for(size_t i = 0; i < params.size(); i++)
        {
            if(i % param_per_osc == 0)
            {
                temp[i] = -std::exp(params[i]);
            }
            else
            {
                temp[i] = params[i];
            }
            temp[i] = params[i];
        }
        return temp;*/
        return Vector(clos_[0].params().begin(), 
                    clos_[0].params().end());
    }

    size_t& group_index() const { return group_index_; }

    void update_group_responsibilities(const std::vector<double>& resps)
    {
        if(group_resps_.size() != resps.size())
        {
            group_resps_.clear();
            group_resps_.resize(resps.size());
        }
        std::copy(resps.begin(), resps.end(), group_resps_.begin());
    }

    /**
     * @brief   Generate Linear_state_space from indpendent Gaussian prior
     */
    bool sample_clo_from_ind_gauss_prior();

    /** @brief  Sample CLO param from its gp prior */
    bool sample_clo_from_gp_prior();

    /** @brief  Generate random polynomial coefs */
    bool sample_polynomial_coefs();

    bool ignore_clo() const { return ignore_clo_; }

    bool use_modal() const { return clos_.front().use_modal(); }

    bool has_valid_params() const;

private:
    /** @brief   Returns the CLO states */
    const State_vec& get_clo_states() const
    {
        update_clo_states();
        IFT(changed_index_ == times_.size() - 1, Runtime_error,
                "CLO states failed to be updated.");
        return clo_states_;
    }

    /** @brief   Get the states at the specified time (sub-sample). */
    State_vec get_clo_states(const std::vector<size_t>& time_indices) const
    {
        IFT(time_indices.size() <= times_.size(), Illegal_argument,
                "The dimension of time_indices_ is greater than times_\n"
                "Please call update_times(time_indices) first");
        State_vec states;
        if(time_indices.empty()) return states;
        size_t starting_index = time_indices.front();
        assert(starting_index < times_.size());
        integrate_states(clos_, 
                         times_[starting_index],
                         times_, 
                         State_type(clo_states_[starting_index]),
                         states, 
                         0,
                         drift_,
                         time_indices);
        return states;
    }

    /** @brief   Returns the CLO states with time-polynomial coefs*/
    const State_vec& get_poly_states() const
    {
        // check if the polynomial coef is set 
        IFT(polynomial_coefs_.size() == num_oscillators_, 
                Runtime_error, 
                "Invalid polynomial_coefs_ in get_poly_states()");
        update_poly_states();
        IFT(poly_coefs_dirty_ == false, Runtime_error, 
                "Poly states is not udpated correctly.");

        return poly_states_;
    }

    /**
     * @brief   Returns the predicted coupled oscillator parameters. 
     */
    Double_vv get_clo_params(const Double_v& times) const;

    /** @brief   Updates the states of the linear state space. */
    void update_clo_states() const
    {
        if(ignore_clo_)
        {
            int length = clo_states_[0].size();
            clos_.clear();
            clo_states_.clear();
            clo_states_.resize(times_.size(), State_type(length, 0.0));
            assert(!times_.empty());
            changed_index_ = times_.size() - 1;
            obs_coefs_dirty_ = true;
            poly_coefs_dirty_ = true;
        }
        else
        {
            //std::cout << "changed_index: " << changed_index_ << std::endl;
            // check if CLO states is updated
            if(changed_index_ != times_.size() - 1)
            {
                // both obs_states_ and poly_states_(if there is) need 
                // to be updated if the clo_states_ is updated. 
                obs_coefs_dirty_ = true;
                poly_coefs_dirty_ = true;
                assert(!clo_states_.empty());
                integrate_states(clos_, 
                                 times_[0],
                                 times_, 
                                 State_type(init_state()),
                                 clo_states_, 
                                 changed_index_,
                                 drift_);
                changed_index_ = times_.size() - 1;
            }
        }
    }

    /** @brief   Updates the states based on the polynomial coefs. */
    void update_poly_states() const
    {
        if(polynomial_coefs_.empty()) return;
        // update the CLO states first 
        IFT(changed_index_ <= times_.size() - 1, Runtime_error, 
                "Invalid changed_index_ in update_poly_states()");

        if(changed_index_ != times_.size() - 1) 
        {
            update_clo_states();
        }

        IFT(changed_index_ == times_.size() - 1, Runtime_error, 
                "CLO states is out of date when calling update_poly_states.");

        if(poly_coefs_dirty_)
        {
            compute_poly_states(clo_states_, poly_states_, times_);
            poly_coefs_dirty_ = false;
            assert(poly_states_.size() == times_.size());
        }
    }

    /** @brief   Updates the states based on the output matrix. */
    void update_obs_states() const
    {
        // update the CLO states
        update_clo_states();
        IFT(changed_index_ == times_.size() - 1, Runtime_error, 
                "Invalid changed_index_ in get_states()");

        // update the states with polynomial coefs 
        if(!polynomial_coefs_.empty())
        {
            update_poly_states();
            IFT(!poly_coefs_dirty_, Runtime_error, 
                    "polynomial states is not updated.");
        }

        if(obs_coefs_dirty_)
        {
            if(polynomial_coefs_.empty())
            {
                assert(poly_states_.empty());
                compute_obs_states(obs_states_, clo_states_);
            }
            else
            {
                assert(!poly_states_.empty());
                compute_obs_states(obs_states_, poly_states_);
            }
            obs_coefs_dirty_ = false;
        }
    }

    /** @brief   Return the states based on the polynomial coefs. */
    void compute_poly_states
    (
        const State_vec& clo_states,
        State_vec& poly_states,
        const Double_v& times
    ) const;

    /** @brief   Return the states based on the output matrix. */
    void compute_obs_states
    (
        State_vec_vec& obs_states, 
        const State_vec& poly_states
    ) const;

    // basic members 
    Double_v times_;

    // cached states 
    mutable State_vec clo_states_;
    mutable State_vec poly_states_;
    mutable State_vec_vec obs_states_;

    // coupled oscillators 
    mutable Coupled_oscillator_v clos_;

    // normal prior params
    mutable std::vector<double> clo_means_;
    mutable std::vector<double> clo_variances_;

    mutable Vector means_;
    mutable Matrix covariance_; 

    // gp prior related memebers
    mutable Double_v gp_scales_;
    mutable Double_v gp_sigvars_;
    gp::Inputs gp_inputs_;
    mutable Gpp_v gp_priors_;
    bool drift_;
    bool gp_initialized_;
    mutable std::vector<bool> gp_changed_;

    // predictors are specific for each couple and are
    // determined by the moderator values 
    // and each CLO value can have their own predictors
    // indexed by [PARAM][MODERATOR]
    // [PARAM] includes both CLO params and the polynomials
    std::vector<Vector> predictors_;

    // number of oscilators 
    size_t num_oscillators_;

    // observable names
    std::vector<std::string> obs_names_;
    // (num_observables, num_oscillators, coef) 
    mutable std::vector<std::vector<Vector> > obs_coefs_;
    // observation sigma of each type of observations
    mutable Vector obs_noise_sigmas_; 

    mutable size_t changed_index_;
    mutable bool poly_coefs_dirty_;
    mutable bool obs_coefs_dirty_;
    mutable bool ignore_clo_;

    // polynomial coefs of each output state 
    // indexed by [OSC_INDEX][POLY_TERM]
    mutable std::vector<Vector> polynomial_coefs_;

    // normal prior for the polynomial coefs
    // indexed by [OSC_INDEX][POLY_TERM]
    mutable std::vector<Vector> polynomial_coefs_means_;
    mutable std::vector<Vector> polynomial_coefs_variances_;

    // normal prior for the polynomial coefs
    // indexed by [OUTCOME_TYPE][OSC_INDEX]
    std::vector<std::string> outcome_names_;
    mutable std::vector<Vector> outcome_means_;
    mutable std::vector<Vector> outcome_variances_;
    std::vector<Vector> outcomes_;

    // params for group
    mutable size_t group_index_;
    std::vector<double> group_resps_;
    std::vector<size_t> change_point_indices_;

/** @brief   Outputs a Linear_state_space to ostream. */
friend std::ostream& operator <<
    (
        std::ostream& ost, 
        const Linear_state_space& lss
    );
};

/**
 * @Class   A class to record a Linear_state_space
 */
class Linear_state_space_recorder
{
public: 
    /** @brief  Constructs a Linear_state_space_recorder. */
    Linear_state_space_recorder(const std::string& dir, size_t start_iter = 1) 
        : parent_dir_(dir), i_(start_iter) 
    {}

    /** @brief  Records a Linear_state_space. */
    void operator()(const Linear_state_space& lss) const
    {
        std::string cur_fname = boost::str(
                boost::format(parent_dir_ + "/%05d/") % i_++);
        lss.write(cur_fname);
    }

    /** @brief  Rest the directory index. */
    void reset() const {i_ = 1; }

private:

    std::string parent_dir_;
    mutable size_t i_;
};

/** @brief  Outputs a Linear_state_space to ostream. */
inline std::ostream& operator <<
(
    std::ostream& ost, 
    const Linear_state_space& lss
)
{
    BOOST_FOREACH(const Coupled_oscillator& val, lss.coupled_oscillators())
    {
        ost << val.params() << std::endl;
        if(!lss.allow_drift()) break;
    }
    return ost;
}

}} // namespace kjb::ties

#endif //KJB_TIES_LINEAR_STATE_SPACE_H
 
