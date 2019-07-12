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

/* $Id: posterior.h 22559 2019-06-09 00:02:37Z kobus $ */

#ifndef KJB_TIES_POSTERIOR_H
#define KJB_TIES_POSTERIOR_H

#include <prob_cpp/prob_distribution.h>
#include <diff_cpp/diff_gradient_mt.h>
#include <diff_cpp/diff_gradient.h>

#include "dbn_cpp/prior.h"
#include "dbn_cpp/likelihood.h"
#include "dbn_cpp/linear_state_space.h"
#include "dbn_cpp/lss_set.h"
#include "dbn_cpp/adapter.h"

#include <boost/optional.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/thread.hpp>
#include <boost/foreach.hpp>
#include <boost/random/mersenne_twister.hpp>

namespace kjb {
namespace ties {

/**
 * @brief   A class computes the posterior of the prior of CLO params
 *          p(mean, var | theta) \sim p(mean, var) p(theta | mean, var)
 */
class Independent_blr_posterior
{
public:
    Independent_blr_posterior
    (
        const Independent_blr_prior& prior
    ) : 
        prior_(prior)
    {}

    /**
     * @brief   Return the posterior 
     */
    double operator()(const Lss_set& lsss) const
    {
        double pr = prior_(lsss); // p(mean, var)
        //std::cout << "Ind_clo_posterior: prior: " << pr << " ";
        // p(theta | z_1, z_2, z_n, alpha, beta, gamma, sigma)
        BOOST_FOREACH(const Linear_state_space& lss, lsss.lss_vec())
        {
            if(!lss.allow_drift())
            {
                pr += lss.get_clo_prior();
            }
            else
            {
                try
                {
                    pr += lss.get_drift_prior();
                }
                catch(Exception& ex)
                {
                    std::cerr << "Error occured while computing the drift prior.\n";
                    return -std::numeric_limits<double>::max();
                }
            }
            if(lss.polynomial_dim_per_osc() >= 0)
            {
                pr += lss.get_polynomial_prior();
            }
        } // p(theta | mean, var)
        return pr;
    }

    /**
     * @brief   Return the prior class 
     */
    const Independent_blr_prior& prior() const { return prior_; }

private:
    const Independent_blr_prior& prior_;
};

/**
 * @class   A class computes the posterior of the GP scale of the CLO params
 *          p(s | theta) = p(s) p(theta | s)
 */
class Shared_gp_scale_posterior
{
public:
    Shared_gp_scale_posterior(const Shared_scale_prior& prior) : prior_(prior){}
    double operator()(const Lss_set& lsss) const
    {
        double pr = prior_(lsss); //p(s)
        BOOST_FOREACH(const Linear_state_space& lss, lsss.lss_vec())
        {
            //assert(lss.allow_drift());
            assert(lss.allow_drift());
            try
            {
                pr += lss.get_drift_prior();
            }
            catch(Exception& ex)
            {
                std::cerr << "Error occured while computing the drift prior.\n";
                return -std::numeric_limits<double>::max();
            }
        } // p(theta | s)
        return pr;
    }

private:
    const Shared_scale_prior& prior_;
};

/**
 * @class    Posterior of a Linear_state_space 
 * 
 */
class Posterior
{
public:
    /**
     * @brief    Construct a Posterior of a Linear_state space. 
     */
    Posterior
    (
        const Likelihood& likelihood,
        const Init_prior& init_prior,
        bool use_init_prior = true,
        bool use_clo_ind_prior = true,
        bool use_clo_group_prior = false,
        bool use_drift = false,
        bool use_likelihood = true,
        bool use_pred = false
    ) : 
        likelihood_(likelihood),
        init_prior_(init_prior),
        use_init_prior_(use_init_prior),
        use_clo_ind_prior_(use_clo_ind_prior),
        use_clo_group_prior_(use_clo_group_prior),
        use_clo_drift_(use_drift),
        use_likelihood_(use_likelihood),
        use_pred_(use_pred)
    {}

    /** @brief   Returns true if use_clo_ind_prior_ is set.  */
    bool get_use_clo_ind_prior() const 
    {
        return use_clo_ind_prior_;
    }

    /** @brief   Returns true if use_init_prior_ is set. */
    bool get_use_init_prior() const 
    {
        return use_init_prior_;
    }

    /** @brief   Returns true if use_likelihood_ is set. */
    bool get_use_likelihood() const 
    {
        return use_likelihood_;
    }

    /** @brief   Returns true if use_pred_ is set. */
    bool get_use_pred() const
    {
        return use_pred_;
    }

    /** @brief   Returns true if use_clo_drift_ is set. */
    bool get_use_clo_drift() const
    {
        return use_clo_drift_;
    }

    /** @brief   Returns true if use_clo_group_prior_ is set. */
    bool get_use_clo_group_prior() const
    {
        return use_clo_group_prior_;
    }

    /** @brief   Sets use_clo_ind_prior_ to "use". */
    void use_clo_ind_prior(bool use) const
    {
        use_clo_ind_prior_ = use;
    }

    /** @brief   Sets use_init_prior_ to "use". */
    void use_init_prior(bool use) const 
    {
        use_init_prior_ = use;
    }

    /** @brief   Sets use_likelihood_ to "use". */
    void use_likelihood(bool use) const 
    {
        use_likelihood_ = use;
    }

    /** @brief   Sets use_pred_ to "use".  */
    void use_pred(bool use) const
    {
        use_pred_ = use;
    }

    /** @brief   Sets use_clo_drift_ to "use".  */
    void use_clo_drift_prior(bool use) const
    {
        use_clo_drift_ = use;
    }

    /** @brief   Sets use_clo_group_prior_ to "use".  */
    void use_clo_group_prior(bool use) const
    {
        use_clo_group_prior_ = use;
    }

    /** @brief   Returns the likelihood. */
    const Likelihood& likelihood() const
    {
        return likelihood_;
    }

    /** @brief   Returns the posterior of a Linear_state_space. */
    double operator()(const Linear_state_space& lss) const;

private:
    Likelihood likelihood_;
    Init_prior init_prior_;
    mutable bool use_init_prior_;
    mutable bool use_clo_ind_prior_;
    mutable bool use_clo_group_prior_;
    mutable bool use_clo_drift_;
    mutable bool use_likelihood_;
    mutable bool use_pred_;
    mutable bool use_poly_prior_;
};

/**
 * @brief    Returns the posterior of each Linear_state_space 
 *           in a vector.
 *           (Note: the prior is implicitely computed)
 */
std::vector<double> individual_posteriors
(
    const std::vector<Linear_state_space>& lss_vec,
    const std::vector<Posterior>& posteriors,
    size_t num_threads
);

/**
 * @brief    Thread worker function of multi-threaded posterior.
 *
 */
void posterior_worker
(
    const std::vector<Posterior>& posteriors,
    const std::vector<Linear_state_space>& lss_vec,
    size_t start,
    size_t end,
    std::vector<double>& pos
);

/**
 * @brief    Thread worker function of multi-threaded posterior.
 *
 */
void posterior_worker_1
(
    const std::vector<Likelihood>& likelihoods,
    const Lss_set& lsss,
    size_t start,
    size_t end,
    std::vector<double>& pos
);

/**
 * @class Posterior for a Lss_set 
 */
class Lss_set_posterior
{
public:

    /**
     * @brief   Constructs a Lss_set_posterior
     */
    Lss_set_posterior
    (
        const Shared_lss_prior& lss_prior, 
        const Shared_noise_prior& noise_prior,
        const std::vector<Posterior>& posteriors
    ) : 
        lss_prior_(lss_prior),
        noise_prior_(noise_prior),
        posteriors_(posteriors),
        use_lss_hyper_prior_(true),
        use_noise_prior_(true)
    {}

    /** @brief   Returns the posterior of a Lss_set. */
    double operator()(const Lss_set& lsss) const;

    /** @brief   Return the posterior of the prior of CLO params. */
    double clo_prior_posterior(const Lss_set& lsss) const;

    /** @brief   Set the flag for lss prior. */
    void set_use_lss_hyper_prior(bool use) { use_lss_hyper_prior_ = use; }

    /** @brief   Set the flag for noise prior. */
    void set_use_noise_prior(bool use) { use_noise_prior_ = use; }

    /** @brief   Get the flag for lss prior. */
    bool get_use_lss_hyper_prior() const { return use_lss_hyper_prior_; }

    /** @brief   Get the flag for noise prior. */
    bool get_use_noise_prior() const { return use_noise_prior_; }

private:
    const Shared_lss_prior& lss_prior_;
    const Shared_noise_prior& noise_prior_;
    const std::vector<Posterior>& posteriors_;
    bool use_lss_hyper_prior_; 
    bool use_noise_prior_; 
};

/**
 * @class   Posterior of the Lss_set based on the predicitive 
 *          probability of the last 20% of the time points 
 *
 *          p(y' | y, phi) 
 */
class Lss_set_pred_posterior
{
public:
    /**
     * @brief   Constructs a Lss_set_pred_posterior
     */
    Lss_set_pred_posterior
    (
        const std::vector<Posterior>& posteriors,
        const std::vector<double>& grad_step_sizes,
        const std::vector<double>& hmc_step_sizes,
        bool not_record,
        size_t num_leapfrogs,
        size_t num_lss_threads,
        const std::vector<std::string>& out_dirs
    ) : 
        posteriors_(posteriors),
        grad_step_sizes_(grad_step_sizes),
        hmc_step_sizes_(hmc_step_sizes),
        not_record_(not_record),
        num_leapfrogs_(num_leapfrogs),
        num_lss_threads_(num_lss_threads),
        out_dirs_(&out_dirs)
    {}

    /** @brief  Retures the predictive posterior.  */
    double operator()(const Lss_set& lsss) const;

    /** @brief  Set the output dirs. */
    void set_out_dirs(const std::vector<std::string>& out_dirs) 
    {
        out_dirs_ = &out_dirs;
    }

private:
    const std::vector<Posterior>& posteriors_;
    const std::vector<double>& grad_step_sizes_;
    std::vector<double> hmc_step_sizes_;
    bool not_record_;
    size_t num_leapfrogs_;
    size_t num_lss_threads_;
    const std::vector<std::string>* out_dirs_;
};

/**
 * @brief   A class evaluates the predictive probability of Lss_set in which 
 *          each CLO-parameter drifts by Gaussian Process
 */

class Lss_set_gp_pred
{
public:
    Lss_set_gp_pred
    (
        const std::vector<Posterior>& posteriors,
        const Lss_set& lss_set,
        double init_sigma, 
        size_t ctr_pt_length,
        size_t num_burn_its, 
        size_t num_sample_its,
        const std::vector<std::string>& out_dirs,
        size_t num_threads = 1,
        bool not_record = false
    ) :
        posteriors_(posteriors),
        lss_set_(lss_set),
        cur_lss_set_(lss_set),
        init_pro_sigma_(init_sigma),
        ctr_pt_length_(ctr_pt_length),
        num_burn_iters_(num_burn_its),
        num_sample_iters_(num_sample_its),
        out_dirs_(out_dirs),
        num_threads_(num_threads),
        not_record_(not_record)
    {}

    double operator()(const Vector& gp_params) const; 

    const Lss_set& current_sample() const { return cur_lss_set_; }

private:
    const std::vector<Posterior>& posteriors_;
    Lss_set lss_set_;
    mutable Lss_set cur_lss_set_;
    double init_pro_sigma_;
    size_t ctr_pt_length_;
    size_t num_burn_iters_;
    size_t num_sample_iters_;
    std::vector<std::string> out_dirs_;
    size_t num_threads_;
    bool not_record_;

};

/**
 * @class Multi-threaded posterior for a Lss_set 
 *
 */
class Lss_set_posterior_mt
{
public:
    /**
     * @brief   Constructs a Lss_set_posterior_mt
     */
    Lss_set_posterior_mt
    (
        const Shared_lss_prior& lss_prior, 
        const Shared_noise_prior& noise_prior,
        const std::vector<Posterior>& posteriors,
        size_t num_threads
    ) : 
        lss_prior_(lss_prior),
        noise_prior_(noise_prior),
        posteriors_(posteriors),
        num_threads_(num_threads),
        use_lss_hyper_prior_(true),
        use_noise_prior_(true)
    {}

    /** @brief retures the likelihood of a Lss_set.  */
    double operator()(const Lss_set& lsss) const
    {
        double pos = 0.0;
        if(use_lss_hyper_prior_)
        {
            pos += lss_prior_(lsss);
            if(pos == -std::numeric_limits<double>::max())
                return pos;
            //std::cout << "pos1: " << pos << std::endl;
        }
        if(use_noise_prior_)
        {
            pos += noise_prior_(lsss);
            if(pos == -std::numeric_limits<double>::max())
                return pos;
            //std::cout << "pos2: " << pos << std::endl;
        }
        std::vector<double> prs = individual_posteriors(lsss.lss_vec(), 
                                                        posteriors_,
                                                        num_threads_);
        pos += std::accumulate(prs.begin(), prs.end(), 0.0);
        //std::cout << "pos3: " << pos << std::endl;
        return pos;
    }

    /** @brief   Set the flag for lss prior. */
    void set_use_lss_hyper_prior(bool use) { use_lss_hyper_prior_ = use; }

    /** @brief   Set the flag for noise prior. */
    void set_use_noise_prior(bool use) { use_noise_prior_ = use; }

    /** @brief   Get the flag for lss prior. */
    bool get_use_lss_hyper_prior() const { return use_lss_hyper_prior_; }

    /** @brief   Get the flag for noise prior. */
    bool get_use_noise_prior() const { return use_noise_prior_; }

private:
    const Shared_lss_prior& lss_prior_;
    const Shared_noise_prior& noise_prior_;
    const std::vector<Posterior>& posteriors_;
    size_t num_threads_;
    bool use_lss_hyper_prior_;
    bool use_noise_prior_;
};

/**
 * @class   Posterior of the Lss_set based on the predicitive 
 *          probability of the last 20% of the time points 
 *
 *          p(y' | y, phi) 
 */
class Lss_set_pred_posterior_mh
{
public:
    /**
     * @brief   Constructs a Lss_set_pred_posterior
     */
    Lss_set_pred_posterior_mh
    (
        const std::vector<Posterior>& posteriors,
        double init_sigma,
        double clo_param_sigma,
        bool not_record,
        size_t num_lss_threads,
        const std::vector<std::string>& out_dirs
    ) : 
        posteriors_(posteriors),
        init_pro_sigma_(init_sigma),
        clo_param_pro_sigma_(clo_param_sigma),
        not_record_(not_record),
        num_lss_threads_(num_lss_threads),
        out_dirs_(&out_dirs)
    {}

    /** @brief  Retures the predictive posterior.  */
    double operator()(const Lss_set& lsss) const;

    double operator()(const Lss_set& lsss, const Vector& variances) const;

    /** @brief  Set the output dirs. */
    void set_out_dirs(const std::vector<std::string>& out_dirs) 
    {
        out_dirs_ = &out_dirs;
    }

private:
    const std::vector<Posterior>& posteriors_;
    double init_pro_sigma_;
    double clo_param_pro_sigma_;
    bool not_record_;
    size_t num_lss_threads_;
    const std::vector<std::string>* out_dirs_;
};

/**
 * @class   Posterior of the Lss_set based on the predicitive 
 *          probability of the last 20% of the time points 
 *
 *          p(y' | y, phi) 
 */
class Lss_set_pred_posterior_mh_v2
{
public:
    /**
     * @brief   Constructs a Lss_set_pred_posterior
     */
    Lss_set_pred_posterior_mh_v2
    (
        const std::vector<Posterior>& posteriors,
        const Lss_set& lss_set,
        double init_sigma,
        double clo_param_sigma,
        bool not_record,
        size_t num_lss_threads,
        const std::vector<std::string>& out_dirs
    ) : 
        posteriors_(posteriors),
        lss_set_(lss_set),
        cur_lss_set_(lss_set),
        init_pro_sigma_(init_sigma),
        clo_param_pro_sigma_(clo_param_sigma),
        not_record_(not_record),
        num_lss_threads_(num_lss_threads),
        out_dirs_(&out_dirs)
    {}

    double operator()(const Vector& shared_params) const;

    void set_params(Lss_set& lss_set) const;

    const Lss_set& current_sample() const { return cur_lss_set_; }

    /** @brief  Set the output dirs. */
    void set_out_dirs(const std::vector<std::string>& out_dirs) 
    {
        out_dirs_ = &out_dirs;
    }

private:
    const std::vector<Posterior>& posteriors_;
    Lss_set lss_set_;
    mutable Lss_set cur_lss_set_;
    double init_pro_sigma_;
    double clo_param_pro_sigma_;
    bool not_record_;
    size_t num_lss_threads_;
    const std::vector<std::string>* out_dirs_;
};
}} // namespace kjb::ties

#endif // KJB_TIES_POSTERIOR_H

