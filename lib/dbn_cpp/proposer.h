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

/* $Id: proposer.h 22559 2019-06-09 00:02:37Z kobus $ */

#ifndef KJB_TIES_PROPOSER_H
#define KJB_TIES_PROPOSER_H

#include <prob_cpp/prob_distribution.h>
#include "dbn_cpp/lss_set.h"
#include "dbn_cpp/prior.h"
#include "dbn_cpp/posterior.h"
#include "cluster_cpp/cluster_gaussian_mixtures.h"
#include "cluster_cpp/cluster_gaussian_mixtures_gibbs.h"

#ifdef KJB_HAVE_ERGO
#include <ergo/mh.h>
#include <ergo/record.h>
#endif

namespace kjb {
namespace ties {

/**
 * @class Propose parameters for the mean of Lss_set 
 */
class Lss_set_mean_variance_step
{
public:
    Lss_set_mean_variance_step
    (
        const Independent_blr_prior& pos
    ) : 
        eval_(pos)
    {}

    /** @brief  Gibbs step to update the BMI-dependent coefs. */
    void operator()(Lss_set& lsss, double& lp) const;

    /** @brief  Returns the name of the step. */
    const std::string& name() const
    {
        static std::string nm = "mean-variance";
        return nm;
    }

private:
    mutable Independent_blr_posterior eval_;
}; 

/**
 * @class Propose parameters for the observation noise sigma of Lss_set 
 *
 */
class Lss_set_noise_sigma_step
{
public:
    Lss_set_noise_sigma_step
    (
        const Shared_noise_prior& prior,
        const std::vector<Likelihood>& likelihoods
    ) : 
        prior_(prior),
        likelihoods_(likelihoods)
    {}

    /** @brief  Gibbs step to update the BMI-dependent coefs. */
    void operator()(Lss_set& lsss, double& lp) const;

    /** @brief  Returns the name of the step. */
    const std::string& name() const
    {
        static std::string nm = "obs-noise-sigma";
        return nm;
    }

private:
    const Shared_noise_prior& prior_;
    const std::vector<Likelihood>& likelihoods_;
}; 

/**
 * @class Propose parameters for the observation noise sigma of Lss_set 
 *        based on the prediction error of the last 20% 
 *
 */
class Lss_set_noise_sigma_proposer
{
public:
    Lss_set_noise_sigma_proposer
    (
        const Vector& prop_sigmas
    ) : 
       prop_sigmas_(prop_sigmas)
    {}

    /** @brief  Update the sigma of the Gaussian noise. */
    ergo::mh_proposal_result operator()
    (
        const Lss_set& in,
        Lss_set& out
    ) const;

    Vector& prop_sigmas() const { return prop_sigmas_; }

private:
    mutable Vector prop_sigmas_;
}; 

/**
 * @class Gibbs step for sampling cluster assignment of a single lss
 *
 */
class Lss_cluster_step
{
public:
    Lss_cluster_step
    (
        const Cluster_prior& prior,
        const std::vector<Group_params>& group_params,
        bool exclude_outcome
    ) : 
        eval_(prior),
        group_params_(group_params),
        exclude_outcome_(exclude_outcome)
    {}

    /** @brief  update the assignments of the lss */
    void operator()(Linear_state_space& lss, double& lp) const;

    /** @brief  update the prior distribution of the lss */
    void update_lss_prior
    (
        Linear_state_space& lss, 
        const Group_params& param
    ) const;

    /** @brief  Returns the name of the step. */
    const std::string& name() const
    {
        static std::string nm = "cluster";
        return nm;
    }

private:
    const Cluster_prior& eval_;
    const std::vector<Group_params>& group_params_;
    bool exclude_outcome_;
};

/**
 * @class Gibbs step for sampling cluster assignments and weights
 *
 */
class Cluster_step
{
public:
    Cluster_step
    (
        const Cluster_prior& prior,
        const Independent_blr_posterior& blr_pos
    ) : 
        cluster_prior_(prior),
        blr_pos_(blr_pos)
    {}

    /** @brief  update the assignments of the lsss */
    void operator()(Lss_set& lsss, double& lp) const;

    /** 
     * @brief  Sample the cluster assignment of each Linear_state_space
     *         based on the current cluster_weights
     */
    double sample_cluster_assignments(const Lss_set& lss_set) const
    {
        bool from_prior = false;
        cluster_prior_.sample_cluster_assignments(lss_set, from_prior);
    }

    /** 
     * @brief   Update the cluster weights based on the current 
     *          number of data per cluster
     */
    void sample_cluster_weights(const Lss_set& lss_set) const
    {
        cluster_prior_.sample_cluster_weights(lss_set);
    }

    /** @brief  Returns the name of the step. */
    const std::string& name() const
    {
        static std::string nm = "cluster-step";
        return nm;
    }

private:
    const Cluster_prior& cluster_prior_;
    const Independent_blr_posterior& blr_pos_;
};

/**
 * @class Gibbs step for the groups from a GMM prior 
 *
 */
class Lss_set_group_step
{
public:
    Lss_set_group_step
    (
        size_t K,
        const Gaussian_mixture_prior& gm_prior,
        bool collapsed = false
    ) :
        mixture_prior_(gm_prior),
        step_(K, mixture_prior_.mixtures(), collapsed, 
              mixture_prior_.fixed_mixture_params())
    {}

    /** @brief  Gibbs step to update the group assignment. */
    void operator()(Lss_set& lsss, double& lp) const
    {
        // old marginal
        double old_hyper_prior = mixture_prior_(lsss);
        //std::cout << "\t\told prior : " << old_hyper_prior << " ";
        double old_gauss_prior = 0.0;
        BOOST_FOREACH(const Linear_state_space& lss, lsss.lss_vec())
        {
            old_gauss_prior += lss.get_full_gaussian_prior();
        }
        //std::cout << "old cond : " << old_gauss_prior << std::endl;
        double new_hyper_prior = 0.0;
        step_(lsss.lss_vec(), new_hyper_prior);
        //std::cout << "\t\tnew prior: " << new_hyper_prior << " ";

        // Set the prior fo each dyad by the current GM
        if(step_.collapsed())
        {
            mixture_prior_.estimate_dist_params();
            mixture_prior_.estimate_mixture_weights();
        }
        mixture_prior_.update_group_info(lsss);
        double new_gauss_prior = 0.0;
        BOOST_FOREACH(const Linear_state_space& lss, lsss.lss_vec())
        {
            new_gauss_prior += lss.get_full_gaussian_prior();
        }
        //std::cout << "new cond : " << new_gauss_prior << std::endl << std::endl;

        lp = lp - old_hyper_prior + new_hyper_prior;
        //std::cout << "prop lp : " << lp << std::endl;
    }

    /** @brief  Return the name of this step */
    const std::string& name() const 
    {
        static std::string nm = "group";
        return nm;
    }

    /**
     * @brief   Reset the old prior to uninitialized value,
     *          used when the data is changed 
     */
    void reset_prior() const 
    {
        step_.reset_prior();
    }

private:
    const Gaussian_mixture_prior& mixture_prior_;
    Gaussian_mixture_gibbs_step<Linear_state_space> step_;
};

/**
 * @class Propose parameters for Linear_state_space
 */
class Init_state_proposer
{
public:
    Init_state_proposer
    (
        double propose_sigma
    ) :
       P_dist_(0.0, propose_sigma),
       sample_index_(0)
    {}

    ergo::mh_proposal_result operator()
    (
        const Linear_state_space& in,
        Linear_state_space& out
    ) const;

private:
    Gaussian_distribution P_dist_;
    mutable size_t sample_index_;
};

/**
 * @class  Propose new polynommial coefs using a symmetric Gaussian proposer
 */
class Lss_polynomial_mh_proposer
{
public:
    Lss_polynomial_mh_proposer
    (
        double propose_sigma
    ) : 
        P_dist_(0.0, propose_sigma),
        sample_index_(0)
    {}

    ergo::mh_proposal_result operator()
    (   
        const Linear_state_space& in,
        Linear_state_space& out
    ) const;

private:
    Gaussian_distribution P_dist_;
    mutable size_t sample_index_;
};

/**
 * @class Propose parameters for Linear_state_space
 */
class Lss_mh_proposer
{
public:
    Lss_mh_proposer
    (
        const Vector& prop_sigmas,
        bool sample_init_state = true,
        bool sample_clo_params = true,
        bool sample_poly_terms = false,
        bool propose_joint_clo = false
    ) :
       prop_sigmas_(prop_sigmas),
       sample_index_(0),
       sample_init_state_(sample_init_state),
       sample_clo_params_(sample_clo_params),
       sample_poly_terms_(sample_poly_terms),
       propose_joint_clo_(propose_joint_clo),
       time_(0)
    {}

    /** @brief Update the Linear_state_space  */
    ergo::mh_proposal_result operator()
    (
        const Linear_state_space& in,
        Linear_state_space& out
    ) const;

    size_t& sample_index() const 
    {
        return sample_index_;
    }

//    void update_modal_angles
//    (
//        const Linear_state_space& in, 
//        Linear_state_space& out,
//        size_t& c_i
//    ) const;
//
    Vector& prop_sigmas() const { return prop_sigmas_; }

    long get_run_time() const { return time_; }

private:
    mutable Vector prop_sigmas_;
    mutable size_t sample_index_;
    bool sample_init_state_;
    bool sample_clo_params_;
    bool sample_poly_terms_;
    bool propose_joint_clo_;
    mutable long time_;
};

/**
 * @class Propose parameters for Lss_set
 */
class Lss_set_mh_proposer
{
public:
    Lss_set_mh_proposer
    (
        double bmi_coef_sigma,
        double var_sigma
    ) :
        bmi_coef_sigma_(bmi_coef_sigma),
        var_sigma_(var_sigma),
        sample_index_(0),
        group_id_(0)
    {}

    ergo::mh_proposal_result operator()
    (
        const Lss_set& in,
        Lss_set& out
    ) const;

private:
    double bmi_coef_sigma_;
    double var_sigma_;
    mutable size_t sample_index_;
    mutable size_t group_id_; 
};

/**
 * @class Propose parameters for Lss_set
 */
class Lss_set_obs_coef_proposer
{
public:
    Lss_set_obs_coef_proposer
    (
        const Vector& prop_sigma
    ) :
        p_dist_(Vector((int)prop_sigma.size(), 0.0), prop_sigma),
        osc_index_(0),
        obs_index_(1)
    {}

    ergo::mh_proposal_result operator()
    (
        const Lss_set& in,
        Lss_set& out
    ) const;

private:
    MV_gaussian_distribution p_dist_;
    mutable size_t osc_index_;
    mutable size_t obs_index_;
};

}} // namespace kjb::ties
#endif //KJB_TIES_PROPOSER_H

