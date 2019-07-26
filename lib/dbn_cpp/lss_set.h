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

/* $Id: lss_set.h 22559 2019-06-09 00:02:37Z kobus $ */

#ifndef KJB_TIES_LSS_SET_H
#define KJB_TIES_LSS_SET_H

#include <map>
#include <vector>
#include <iterator>
#include <algorithm>

#include "dbn_cpp/linear_state_space.h"
#include "dbn_cpp/data.h"
#include "dbn_cpp/typedefs.h"

namespace kjb {
namespace ties {

/** 
 * @brief   Cached data used in the Bayesian linear model 
 */
struct Cached_data
{
    Cached_data
    (
        size_t param_size, 
        size_t num_data
    ) : 
        X_t_K_inv_(param_size, Matrix()),
        X_t_K_inv_X_(param_size, Matrix()),
        X_all_(param_size, std::vector<Matrix>(num_data)),
        K_inv_(param_size, std::vector<Matrix>(num_data)),
        y_T_K_inv_y_(param_size, 0.0)
    {}

    // indexed by [CLO_PARAM]
    std::vector<Matrix> X_t_K_inv_;
    // indexed by [CLO_PARAM]
    std::vector<Matrix> X_t_K_inv_X_;

    // USED for drifting version
    // indexed by [CLO_PARAM][CLO]
    std::vector<std::vector<Matrix> > X_all_;
    // indexed by [CLO_PARAM][CLO]
    std::vector<std::vector<Matrix> > K_inv_;
    // indexued by [CLO_PARAM]
    std::vector<double> y_T_K_inv_y_;
};

/**
 * @class   A Lss_set class which handles couple-shared parameters 
 */
class Lss_set
{
public:

    /**
     * @brief   Constucts a Lss_set. 
     */
    Lss_set
    (
        const std::vector<size_t>& ids,
        const std::vector<std::vector<std::string> >& mod_names,
        const State_vec& init_states,
        const std::vector<Data>& data,
        double gp_scale, 
        double clo_sigma,
        double poly_sigma,
        double outcome_sigma,
        const std::vector<std::string>& obs_names,
        bool fixed_clo = false,
        double training_percent = 0.8,
        size_t num_oscillators = 2,
        double init_period = DEFAULT_PERIOD,
        double init_damping = DEFAULT_DAMPING,
        const Vector& obs_noise_sigmas = Vector(1, DEFAULT_NOISE_SIGMA),
        bool allow_drift = false,
        size_t num_groups = 1,
        int polynomial_degree = -1,
        const std::vector<std::string>& outcome_names = std::vector<std::string>(),
        bool ingnore_clo = false,
        const std::string& categorical_moderator = std::string(""),
        const std::string& grouping_info_fp = std::string(""),
        bool use_modal = false
    );

    /**
     * @brief   Initialize each Linear_state_space.
     */
    void init_lss
    (
        const State_vec& init_states,
        const std::vector<Data>& data,
        const Double_v& gp_scales,
        const Double_v& gp_sigvars,
        const std::vector<std::string>& obs_names,
        size_t num_oscillators = 2,
        double training_percent = 0.8,
        double init_period = DEFAULT_PERIOD,
        double init_damping = DEFAULT_DAMPING,
        const Vector& obs_noise_sigmas = Vector(1, DEFAULT_NOISE_SIGMA),
        bool allow_drift = false,
        int polynomial_degree = -1, 
        const std::vector<std::string>& outcome_names = std::vector<std::string>(),
        bool ignore_clo = false,
        bool use_modal = false
    );

    /**
     * @brief   Read in a Lss_set from a input directory. 
     */
    void read 
    (
        const std::string& indir,
        const std::vector<Data>& data_all,
        const Group_map& group_map = Group_map()
    );

    /** @brief  Read in shared parameters from a file. */
    void read_shared_params(const std::string& fp);

    /**
     * @brief   Return a reference to the vector of Linear_state_space.
     */
    std::vector<Linear_state_space>& lss_vec() const
    {
        return lss_vec_;
    }

    /**
     * @brief   Return the number of pred coefficients
     */
    size_t pred_coef_size(size_t group = 0) const 
    {
        assert(group < params_.size());
        size_t sz = 0;
        if(ignore_clo()) return sz;
        BOOST_FOREACH(const Vector& vals, params_[group].pred_coefs)
        {
            sz += vals.size();
        }
        return sz;
    }

    /**
     * @brief   Return the number of polynomial parameters
     */
    size_t num_polynomial_params() const
    {
        if(lss_vec_.empty()) return 0;
        else return lss_vec_[0].num_polynomial_coefs();
    }

    /**
     * @brief   Return the number of outcomes
     */
    size_t num_outcomes() const 
    {
        if(lss_vec_.empty()) return 0;
        else return lss_vec_[0].num_outcomes();
    }

    /**
     * @brief   Return the number of outcome types
     */
    size_t num_outcome_type() const 
    {
        if(lss_vec_.empty()) return 0;
        else return lss_vec_[0].num_outcome_type();
    }

    /**
     * @brief   Return the number of polynomial parameters
     */
    int polynomial_dim_per_osc() const
    {
        if(lss_vec_.empty()) return 0;
        else return lss_vec_[0].polynomial_dim_per_osc();
    }

    /**
     * @brief   Return the number of couple-shared parameters.
     */
    size_t shared_param_size
    (
        size_t group = 0, 
        bool exclude_variance = false
    ) const
    {
        assert(group < params_.size());
        if(lss_vec_.empty()) return 0;
        size_t sz = pred_coef_size(group);
        if(!fixed_clo_ && !exclude_variance) 
        {
            sz += params_[group].variances.size(); 
        }
        return sz;
    }

    /** @brief    Update the Linear_state_space for one couple.  */
    void update_lss_mean(size_t lss_index) const;

    /** @brief    Update the Linear_state_space for one couple.  */
    void update_lss_variance(size_t lss_index) const;

    /** @brief    Update the mean for all couples. */
    void update_means() const
    {
        for(size_t i = 0; i < lss_vec_.size(); i++)
        {
            update_lss_mean(i);
        }
    }

    /** @brief    Update the variance for all couples. */
    void update_variances() const
    {
        for(size_t i = 0; i < lss_vec_.size(); i++)
        {
            update_lss_variance(i);
        }
    }

    /** @brief    Update the gp-scale of all couples. */
    void update_gp_scales
    (
        size_t lss_index, 
        size_t param_index, 
        double val
    ) const
    {
        IFTD(lss_index < lss_vec_.size(), Illegal_argument, 
                "Can not set gp-scale for dyad %d (out of bound",
                (lss_index));
        IFTD(param_index < clo_param_size_, Illegal_argument, 
                "Can out set gp-scale for param %d (out of bound)",
                (param_index));
        lss_vec_[lss_index].set_gp_scale(param_index, val);
        lss_vec_[lss_index].update_gp();
    }

    /** @brief    Update the gp-signal-variance of all couples. */
    void update_gp_sigvars
    (
        size_t lss_index, 
        size_t param_index, 
        double val
    ) const
    {
        IFTD(lss_index < lss_vec_.size(), Illegal_argument, 
                "Can not set gp-scale for dyad %d (out of bound",
                (lss_index));
        IFTD(param_index < clo_param_size_, Illegal_argument, 
                "Can out set gp-sigvar for param %d (out of bound)",
                (param_index));
        lss_vec_[lss_index].set_gp_sigvar(param_index, val);
        lss_vec_[lss_index].update_gp();
    }

    /** @brief  Update the GP information */
    void update_gps
    (
        const Double_v& gp_scales, 
        const Double_v& gp_sigvars
    ) const;

    /** @brief    Return the gp scales. */
    const Double_v& gp_scales() const
    {
        //KJB(ASSERT(lss_vec_[0].allow_drift()));
        return lss_vec_[0].gp_scales();
    }

    /** @brief    Return the gp signal variances. */
    const Double_v& gp_sigvars() const
    {
        //KJB(ASSERT(lss_vec_[0].allow_drift()));
        return lss_vec_[0].gp_sigvars();
    }

    /** @brief    Return the variance of each coupled oscillator param. */
    std::vector<double>& variances(size_t group = 0) const 
    { 
        assert(group < params_.size());
        return params_[group].variances; 
    }

    /** @brief    Return the number of the oscillators. */
    size_t num_oscillators() const 
    { 
        return lss_vec_.empty() ? 0 : lss_vec_[0].num_oscillators(); 
    }

    /** @brief    Return true if use modal reprsentation. */
    bool use_modal() const 
    { 
        return lss_vec_.empty() ? 0 : lss_vec_[0].use_modal(); 
    }

    /** @brief    Return the number of the observables. */
    size_t num_observables() const 
    {
        return lss_vec_.empty() ? 0 : lss_vec_[0].obs_names().size();
    }

    /** @brief    Return the dimension of obs_coefs for a single oscillator. */
    size_t obs_coef_dim() const
    {
        return lss_vec_.empty() ? 0 : lss_vec_[0].obs_coef_dim();
    }

    /** @brief    Return the number of the obs coefs . */
    size_t num_obs_coefs() const 
    {
        return lss_vec_.empty() ? 0 : lss_vec_[0].num_obs_coefs();
    }

    /** @brief  Return the observed states coefficients. */
    const std::vector<std::vector<Vector> >& obs_coefs() const 
    { 
        return lss_vec_.front().obs_coefs();
    }

    /** @brief  Sets the ith observable jth coef */
    void set_obs_coef(size_t i, size_t j, const Vector& val) const
    {
        BOOST_FOREACH(const Linear_state_space& lss, lss_vec_)
        {
            lss.set_obs_coef(i, j, val);
        }
    }

    /** @brief  Sets the ith observable jth coef */
    void set_obs_coef(size_t i, size_t j, size_t k, double val) const
    {
        BOOST_FOREACH(const Linear_state_space& lss, lss_vec_)
        {
            lss.set_obs_coef(i, j, k, val);
        }
    }

    /** @brief  Get the CLO parameters into a vector form */
    std::vector<std::vector<Vector> > get_lss_params() const;

    /** @brief    Return the mean of all the initial states. */
    State_type init_state_mean() const;

    /** @brief    Return true if CLO parameters have priors. */
    bool fixed_clo() const { return fixed_clo_; }

    /** @brief    Return the predictor coefficients. */
    std::vector<Vector>& pred_coefs(size_t group = 0) const 
    {
        assert(group < params_.size());
        return params_[group].pred_coefs;
    }

    double& group_weight(size_t group) const
    {
        assert(group < params_.size());
        return params_[group].group_weight;
    }

    /** @brief    Return the IDs  */
    const std::vector<size_t>& ids() const { return ids_; }

    /** @brief    Return teh size of the CLO parameters. */
    size_t clo_param_size() const { return clo_param_size_; }

    /**
     * @brief   Writes this Lss_set to a output directory.
     */
    void write
    (
        const std::string& out_dir
    ) const;

    /**
     * @brief   Output this Lss_set to an ostream. 
     */
    friend std::ostream& operator <<
    (
        std::ostream& ost, 
        const Lss_set& lsss
    );

    /** 
     * @brief   Return X^t K^{-1}. 
     */
    const std::vector<Matrix>& get_poly_X() const
    {
        return X_poly_;
    }

    /** 
     * @brief   Return X^t K^{-1}. 
     */
    const std::vector<Matrix>& get_X_t(size_t cluster = 0) const 
    { 
        IFT(cluster < cached_data_.size(), Illegal_argument, 
                "X_t is out of bounds");
        return cached_data_[cluster].X_t_K_inv_; 
    }

    /**
     * @brief   Return the X_t for all the clusters
     */
    std::vector<std::vector<Matrix> > get_X_ts() const
    {
        std::vector<std::vector<Matrix> > xts(num_groups_);
        for(size_t i = 0; i < num_groups_; i++)
        {
            xts[i] = cached_data_[i].X_t_K_inv_;
        }
        return xts;
    }

    /** 
     * @brief   Return X^t K^{-1} X.
     */
    const std::vector<Matrix>& get_X_t_X(size_t cluster = 0) const 
    { 
        IFT(cluster < cached_data_.size(), Illegal_argument, 
                "X_t_X is out of bounds");
        return cached_data_[cluster].X_t_K_inv_X_; 
    }

    /**
     * @brief   Return the X_t for all the clusters
     */
    std::vector<std::vector<Matrix> > get_X_t_Xs() const
    {
        std::vector<std::vector<Matrix> > xtxs(num_groups_);
        for(size_t i = 0; i < num_groups_; i++)
        {
            xtxs[i] = cached_data_[i].X_t_K_inv_X_;
        }
        return xtxs;
    }


    /**
     * @brief   Return y_T_K_inv_y_
     */
    const std::vector<double>& get_y_T_K_inv_y(size_t cluster = 1) const 
    {
        IFT(cluster < cached_data_.size(), Illegal_argument, 
                "y_T_K_inv_y is out of bounds");
        return cached_data_[cluster].y_T_K_inv_y_;
    }

    /**
     * @brief   Return the noise sigmas.
     */
    const Vector& get_noise_sigmas() const 
    { 
        return lss_vec_.front().noise_sigmas(); 
    }

    /**
     * @brief   Sets the noise sigmas to new values. 
     */
    void set_noise_sigmas(const Vector& sigmas)
    {
        BOOST_FOREACH(const Linear_state_space& lss, lss_vec_)
        {
            lss.noise_sigmas() = Vector(sigmas);
        }
    }

    /** 
     * @brief   Sets the noise sigma at a specific index
     */
    void set_noise_sigmas(size_t i, double value) 
    {
        IFT(i < get_noise_sigmas().size(), Illegal_argument, 
                "Index out of bound(set_noise_sigma");

        BOOST_FOREACH(const Linear_state_space& lss, lss_vec_)
        {
            lss.noise_sigmas()[i] = value;
        }
    }

    /**
     * @brief   Return the group of each individual
     */
    std::vector<int> group_assignments() const
    {
        size_t N = lss_vec_.size();
        std::vector<int> groups(N);
        for(size_t i = 0; i < N; i++)
        {
            groups[i] = lss_vec_[i].group_index();
        }

        return groups;
    }

    Vector& group_weights() const 
    {
        return group_weights_;
    }

    const std::vector<Vector>& get_group_means() const
    {
        return group_means_;
    }

    /**
     * @brief   Return the mean of each group
     */
    std::vector<Vector>& group_means() const
    {
        lss_group_dirty_ = true;
        return group_means_;
    }

    const std::vector<Matrix>& get_group_covariances() const
    {
        return group_covariances_;
    }

    /**
     * @brief   Return the covariance of each group
     */
    std::vector<Matrix>& group_covariances() const
    {
        lss_group_dirty_ = true;
        return group_covariances_;
    }

    /**
     * @brief   Return the number of group
     */
    size_t& num_groups() const { return num_groups_; }

    /**
     * @brief   Update group for each Linear_state_space 
     */
    void update_lss_group() const
    {
        //assert(!group_dirty_);
        if(!lss_group_dirty_) return;
        for(size_t i = 0; i < lss_vec_.size(); i++)
        {
            int group = lss_vec_[i].group_index();
            assert(group >= 0 && group < num_groups_);
            lss_vec_[i].group_index() = group;
            lss_vec_[i].update_full_gaussian_prior(
                    group_means_[group], group_covariances_[group]);
        }
        lss_group_dirty_ = false;
    }

    //bool& group_dirty() const { return group_dirty_; }

    /**
     * @brief   Parse the observation coefs 
     */
    void parse_obs_coef(const std::string& obs_fname) const;

    /** @brief  Read in the shared parameters from a file. */
    void parse_group_params
    (
        const std::string& param_fp,
        std::vector<Group_params>& params
    );

    /**
     * @brief   Initialize the design matrix 
     */
    void init_design_matrix() const;

    /**
     * @brief   Initialize the design matrix for the polynomial coefs
     */
    void init_poly_design_matrix();

    /**
     * @brief   Update the design matrix once the gp is changed
     */
    void update_covariance_matrix() const;

    /**
     * @brief   Return true if the CLO params are allowed to drift
     */
    bool allow_drift() const { return lss_vec_.front().allow_drift(); }

    /**
     * @brief   Return the names of the moderators of the CLO params
     */
    const std::vector<std::vector<std::string> >& get_mod_names() const
    {
        return mod_names_;
    }

    /**
     * @brief   Return the names of the moderators of the polynomials 
     */
    const std::vector<std::string>& get_poly_mod_names() const
    {
        return poly_mod_names_;
    }

    /**
     * @brief   Return the group params 
     */
    const std::vector<Group_params>& group_params() const 
    {
        return params_;
    }

    /**
     * @brief   Return the weight of group in a std vector
     */
    std::vector<double> get_group_weights() const 
    {
        std::vector<double> weights(params_.size());
        for(size_t i = 0; i < weights.size(); i++)
        {
            weights[i] = params_[i].group_weight;
        }
        return weights;
    }

    /**
     * @brief   Update the group params
     */
    void set_group_params(const std::vector<Group_params>& params) const
    {
        params_.clear();
        params_.resize(params.size());
        std::copy(params.begin(), params.end(), params_.begin());
    }

    bool ignore_clo() const { return lss_vec_[0].ignore_clo(); } 

    /** @brief  Return the keys of the polynomial coefs indices. */
    const std::map<size_t, std::vector<size_t> > poly_keys() const 
    {
        if(poly_keys_.empty()) build_poly_keys();
        return poly_keys_; 
    }

    /** @brief   Build the map for the polynomial coefs indices */
    void build_poly_keys() const;

    /** 
     * @brief   Check and validate teh predictors dimesion
     *
     * @param   size  if size is greater than the current predictor, 
     *                make the predictors' size to be this size 
     */
    void check_predictors_dimension();

    /** @brief   Return the group_map   */
    const Group_map& group_map() const 
    {
        return group_map_;
    }

    /** @brief   Return all the samples  */
    std::vector<std::vector<Linear_state_space> >& all_samples() const
    {
        return samples_all_;
    }

    /**
     * @brief   Clear all the samples
     */
    void clear_samples() const
    {
        BOOST_FOREACH(std::vector<Linear_state_space>& samples, samples_all_)
        {
            samples.clear();
        }
    }

    /** 
     * @brief   Return the sum of two lss_set (summing the group params)
     */
    void add(const Lss_set& lsss);

    /*
     * @brief   Deviding the shared parameters by N
     */
    void average(size_t n);

    void report_outcome_pred_errs(const std::string& out_dp) const;

private:
    /** @brief   Sets the indices and values of shared-parameter . */
    void set_indices_coefs
    (
        const std::vector<size_t>& indices,
        const std::vector<double>& coefs,
        std::vector<bool>& vals,
        std::map<size_t, double>& coef_map
    )
    {
        IFT(indices.size() == coefs.size(), Illegal_argument, 
                "The size of indices differs from the size of coefs");
        for(size_t i = 0; i < indices.size(); i++)
        {
            size_t param_index = indices[i];
            assert(param_index < vals.size());
            vals[param_index] = true;
            coef_map[param_index] = coefs[i];
        }
    }

    /**
     * @brief   Get the covariance matrix from the GP for the param index
     */
    Matrix get_covariance_matrix_inv
    (
        const Linear_state_space& lss, 
        size_t param_index
    ) const;

private:
    // index by couple 
    std::vector<size_t> ids_;
    size_t clo_param_size_;
    size_t polynomial_coef_size_;
    size_t outcome_size_;

    mutable std::vector<Linear_state_space> lss_vec_;

    // indexed by [CLUSTER]
    mutable std::vector<Group_params> params_;

    // params for categorical/group model
    mutable size_t num_groups_;
    mutable Vector group_weights_;
    mutable std::vector<Vector> group_means_;
    mutable std::vector<Matrix> group_covariances_;

    // indexed by [CLO_PARAM][MODERATOR]
    std::vector<std::vector<std::string> > mod_names_;
    std::vector<std::string> poly_mod_names_;

    // cached stuff, indexed by [CLUSTER]
    mutable std::vector<Cached_data> cached_data_;

    mutable std::map<size_t, std::vector<size_t> > poly_keys_;

    mutable std::vector<Matrix> X_poly_;

    bool fixed_clo_;
    mutable bool lss_group_dirty_;

    std::string categorical_moderator_;
    mutable Group_map group_map_;
    bool use_full_cov_;

    mutable std::vector<std::vector<Linear_state_space> > samples_all_;

}; // class Lss_set

/**
 * @class   A class records a sample of a Lss_set.
 */
class Lss_set_recorder
{
public: 
    Lss_set_recorder(const std::string& dir) : parent_dir(dir), i(1) 
    {}

    void operator()(const Lss_set& lss) const 
    {
        std::string cur_fname = 
            boost::str(boost::format(parent_dir+"/%04d/") % i++);
        lss.write(cur_fname);
    }
    void reset() const 
    {
        i = 1;
    }

private:
    std::string parent_dir;
    mutable size_t i;
}; // class Lss_set_recorder

}}

#endif
