/* =========================================================================== *
|
|  Copyright (c) 1994-2008 by Jinyan Guan (author).
|
|  Personal and educational use of this code is granted, provided that this
|  header is kept intact, and that the authorship is not misrepresented, that
|  its use is acknowledged in publications, and relevant papers are cited.
|
|  For other use contact the author (Jinyan AT cs DOT arizona DOT edu).
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

/* $Id: gradient.h 22559 2019-06-09 00:02:37Z kobus $ */

#ifndef KJB_TIES_GRADIENT_H_ 
#define KJB_TIES_GRADIENT_H_

#include <diff_cpp/diff_gradient.h>
#include <diff_cpp/diff_gradient_mt.h>
#include "dbn_cpp/adapter.h"
#include "dbn_cpp/lss_set.h"
#include "dbn_cpp/likelihood.h"
#include "dbn_cpp/posterior.h"
#include "dbn_cpp/util.h"

namespace kjb {
namespace ties {

/**
 * @class    A Lss_gradient class to compute the gradient of a
 *           Linear_state_space.
 */
template <typename Adapter> 
class Lss_gradient
{
public:
    /**
     * @brief    Construct a Lss_gradient. 
     * @param    posterior    Posterior of a Linear_state_space.
     * @param    adapter      Adapter of a Linear_state_space
     * @param    step_size    The size of step used in computing the gradient.
     */
    Lss_gradient
    (
        const Posterior& posterior,
        const Adapter& adapter,
        double step_size
    ) : posterior_(posterior),
        adapter_(adapter),
        step_size_(step_size)
    {}

    /**
     * @brief    Returns the gradient of a Linear_state_space.
     */
    std::vector<double> operator()(const Linear_state_space& lss) const;

private:
    const Posterior& posterior_;
    const Adapter& adapter_;
    double step_size_;
};

/**
 * @class    A class to compute the gradient of a Linear_state_space usig
 *           multithreading. 
 */
template <typename Adapter> 
class Lss_gradient_mt
{
public:
    /**
     * @brief    Construct a Lss_gradient_mt.
     * @param    posterior    Posterior of a Linear_state_space.
     * @param    adapter      Adapter of a Linear_state_space
     * @param    step_size    The size of step used in computing the gradient.
     * @param    num_threads  The number of threads 
     */
    Lss_gradient_mt
    (
        const Posterior& posterior,
        const Adapter& adapter,
        double step_size,
        size_t num_threads
    ) :
        posterior_(posterior),
        adapter_(adapter),
        step_size_(step_size),
        num_threads_(num_threads)
    {}

    /**
     * @brief    Returns the gradient of a Linear_state_space.
     */
    std::vector<double> operator()(const Linear_state_space& lss) const;

private:
    const Posterior& posterior_;
    const Adapter& adapter_;
    double step_size_;
    size_t num_threads_;
    std::vector<size_t> param_indices_;

};

/**
 * @class    A class to compute the gradient of couple-shared parameters. 
 */
template <typename Func, typename Adapt>
class Shared_gradient
{
public:
    /**
     * @brief    Constructs a Shared_gradient. 
     * @param    posterior       Posterior of a Lss_set
     * @param    adapter         Adapter of a Lss_set
     * @param    grad_steps      Step sizes of computing the gradient. 
     * @param    estimate_step   If set, estimate the step to comput the
     *                           gradient. 
     */
    Shared_gradient
    (
        const Func& posterior,
        const Adapt& adapter,
        const std::vector<double>& step_sizes,
        bool estimate_step = false
    ) : posterior_(posterior),
        adapter_(adapter),
        steps_(step_sizes),
        estimate_step_(estimate_step)
    {}

    /**
     * @brief    Returns the gradient of the shared parameters in a Lss_set.
     */
    std::vector<double> operator()
    (
        const Lss_set& lsss
    ) const;

private:
    const Func& posterior_;
    const Adapt& adapter_;
    mutable std::vector<double> steps_;
    bool estimate_step_;
};

/**
 * @class    A class to compute the gradient of couple-shared parameters using
 *           multithreading.
 */
template <typename Func, typename Adapt>
class Shared_gradient_mt
{
public:
    /**
     * @brief    Constructs a Shared_gradient_mt.
     * @param    posterior       Posterior of a Lss_set
     * @param    adapter         Adapter of a Lss_set
     * @param    grad_steps      Step sizes of computing the gradient. 
     * @param    estimate_step   If set, estimate the step to comput the
     *                           gradient. 
     * @param    num_threads     Number of threads
     */
    Shared_gradient_mt
    (
        const Func& posterior,
        const Adapt& adapter,
        const std::vector<double>& grad_steps,
        bool estimate_step = false,
        size_t num_threads = 1
    ) : posterior_(posterior),
        adapter_(adapter),
        steps_(grad_steps),
        estimate_step_(estimate_step),
        num_threads_(num_threads)
    {}

    /**
     * @brief    Returns the gradient of the shared parameters in a Lss_set.
     */
    std::vector<double> operator()
    (
        const Lss_set& lsss
    ) const;

private:
    const Func& posterior_;
    const Adapt& adapter_;
    mutable std::vector<double> steps_;
    bool estimate_step_;
    size_t num_threads_;
};

/**
 * @class    A class to compute the gradient of the initial state of a
 *           Linear_state_space. 
 */
class Init_state_gradient
{
public:
    /**
     * @brief    Constructs a Init_state_gradient. 
     */
    Init_state_gradient
    (
        const Posterior& posterior,
        const Init_state_adapter& adapter, 
        double step_size
    ) : posterior_(posterior),
        adapter_(adapter),
        step_size_(step_size)
    {}

    /**
     * @brief    Returns the gradient of the initial state of a
     *           Linear_state_space. 
     */
    std::vector<double> operator()
    (
        const Linear_state_space& lss
    ) const
    {
        using namespace std;
        size_t num_params = adapter_.size(&lss);
        std::vector<double> step_sizes(num_params, step_size_);

        // save old options 
        bool use_clo_ind = posterior_.get_use_clo_ind_prior();
        bool use_clo_group = posterior_.get_use_clo_group_prior();
        bool use_clo_drift = posterior_.get_use_clo_drift();

        // disble clo priors
        posterior_.use_clo_ind_prior(false);
        posterior_.use_clo_group_prior(false);
        posterior_.use_clo_drift_prior(false);

        Vector G = gradient_ffd_mt(posterior_, lss, 
                                   step_sizes, adapter_,
                                   num_params);

        // reset to the old options
        posterior_.use_clo_ind_prior(use_clo_ind);
        posterior_.use_clo_group_prior(use_clo_group);
        posterior_.use_clo_drift_prior(use_clo_drift);

        std::vector<double> grads(G.size());
        std::copy(G.begin(), G.end(), grads.begin());

        return grads;
    }

private:
    const Posterior& posterior_;
    const Init_state_adapter& adapter_;
    double step_size_;
};

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

template <typename Adapter>
std::vector<double> Lss_gradient<Adapter>::operator()
(   
    const Linear_state_space& lss
) const 
{
    size_t N = adapter_.size(&lss);
    std::vector<double> step_sizes(N, step_size_);
    Vector G = gradient_ffd(posterior_, lss, step_sizes, adapter_);
    std::vector<double> grads(G.size());
    std::copy(G.begin(), G.end(), grads.begin());
    return grads;
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

template <typename Adapter>
std::vector<double> Lss_gradient_mt<Adapter>::operator()
(   
    const Linear_state_space& lss
) const 
{
    size_t N = adapter_.size(&lss);
    std::vector<double> step_sizes(N, step_size_);
    size_t avail_cores = boost::thread::hardware_concurrency();
    size_t num_threads = num_threads_ > avail_cores ? avail_cores : num_threads_;
    Vector G = gradient_ffd_mt(posterior_,
                               lss,
                               step_sizes,
                               adapter_,
                               num_threads);
    std::vector<double> grads(G.size());
    std::copy(G.begin(), G.end(), grads.begin());
    return grads;
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

template <typename Func, typename Adapt>
std::vector<double> Shared_gradient<Func, Adapt>::operator()
(
    const Lss_set& lsss
) const
{
    if(estimate_step_) steps_ = get_hmc_step_size(lsss, adapter_);
    Vector G = gradient_ffd(posterior_, lsss, steps_, adapter_);
    std::vector<double> grads(G.size());
    std::copy(G.begin(), G.end(), grads.begin());
    return grads;
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

template <typename Func, typename Adapt>
std::vector<double> Shared_gradient_mt<Func, Adapt>::operator()
(
    const Lss_set& lsss
) const
{
    if(estimate_step_) steps_ = get_hmc_step_size(lsss, adapter_);
    size_t avail_cores = boost::thread::hardware_concurrency();
    size_t num_threads = num_threads_ > avail_cores ? avail_cores : num_threads_;
    Vector G = gradient_ffd_mt(posterior_,
                               lsss,
                               steps_,
                               adapter_,
                               num_threads);
    std::vector<double> grads(G.size());
    std::copy(G.begin(), G.end(), grads.begin());
    return grads;
}

}} // namespace kjb::ties

#endif 
