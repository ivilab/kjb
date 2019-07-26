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

/* $Id: marginal_likelihood.h 22559 2019-06-09 00:02:37Z kobus $ */

#ifndef KJB_TIES_MARGINAL_LIKELIHOOD_H
#define KJB_TIES_MARGINAL_LIKELIHOOO_H 

#include <vector>
#include <string>

#include "dbn_cpp/prior.h"
#include "dbn_cpp/posterior.h"
#include "dbn_cpp/linear_state_space.h"
#include "dbn_cpp/thread_worker.h"
#include "dbn_cpp/lss_set.h"
#include "dbn_cpp/adapter.h"

#include <boost/shared_ptr.hpp>

namespace kjb{
namespace ties{

/**
 * @struct    A struct specify the sample options for computing the marginal
 *            likelihood. 
 */
struct Sample_options
{
    /** @brief   Default ctor.    */
    Sample_options() {}

    /**
     * @brief   Construct a Sample_options for using HMC.
     */
    Sample_options
    (
        size_t leapfrogs,
        const std::vector<double>& hmc_steps,
        const std::vector<double>& grad_steps,
        size_t threads = 1
    ) :
        sample_method("hmc"),
        num_leapfrogs(leapfrogs),
        hmc_step_sizes(hmc_steps),
        grad_step_sizes(grad_steps),
        num_threads(threads)
    {}

    /**
     * @brief   Construct a Sample_options for using MH.
     */
    Sample_options
    (
        double state_sigma, 
        double clo_sigma,
        size_t threads = 1
    ) :
        sample_method("mh"),
        init_state_sigma(state_sigma),
        clo_param_sigma(clo_sigma),
        num_threads(threads)
    {}

    std::string sample_method;
    // hmc 
    size_t num_leapfrogs;
    std::vector<double> hmc_step_sizes;
    std::vector<double> grad_step_sizes;
    // mh 
    double init_state_sigma;
    double clo_param_sigma;
    size_t num_threads;
};

/**
 * @class    A class to compute the marginal likelihood using Laplacian
 *           approximation.
 *           p(y|psi) = \int p(y|theta, psi) p(theta| psi) d theta
 *
 */
class Marginal_likelihood
{
public:
    Marginal_likelihood
    (
        const Shared_lss_prior& shared_prior,
        const std::vector<Posterior>& posteriors,
        const std::vector<std::string>& out_dirs,
        const Sample_options& opt,
        bool not_record = true
    ) : 
        shared_prior_(shared_prior),
        posteriors_(posteriors),
        sample_out_dirs_(out_dirs),
        opt_(opt),
        not_record_(not_record)
    {}

    /** @brief    Returns the marginal likelihood.  */
    double operator()(const Lss_set& lsss);

private:
    const Shared_lss_prior& shared_prior_;
    const std::vector<Posterior>& posteriors_;
    const std::vector<std::string>& sample_out_dirs_;
    std::vector<double> lss_hess_step_sizes_;
    const Sample_options& opt_;
    bool not_record_;
};

}}

#endif //KJB_TIES_MARGINAL_LIKELIHOOO_H 

