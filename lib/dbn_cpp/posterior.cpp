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

/* $Id: posterior.cpp 22559 2019-06-09 00:02:37Z kobus $ */

#include <prob_cpp/prob_pdf.h>
#include <prob_cpp/prob_util.h>
#include <boost/foreach.hpp>
#include <diff_cpp/diff_gradient.h>

#include "dbn_cpp/lss_set.h"
#include "dbn_cpp/adapter.h"
#include "dbn_cpp/gradient.h"
#include "dbn_cpp/posterior.h"
#include "dbn_cpp/thread_worker.h"
#include "dbn_cpp/linear_state_space.h"

#include <algorithm>
#include <cmath>

#include <boost/thread.hpp>
#include <boost/foreach.hpp>

using namespace kjb::ties;

#ifdef KJB_HAVE_CXX11
using std::cref;
using std::ref;
#else
#include <boost/ref.hpp>
using boost::cref;
using boost::ref;
#endif

double Posterior::operator()(const Linear_state_space& lss) const
{
    if(!lss.has_valid_params())
    {
        std::cerr << "Current Sample does not have valid params.\n";
        return -std::numeric_limits<double>::max();
    }

    double ll = 0.0;

    // Prior for each CLO param
    if(use_clo_drift_)
    {
        // drift prior
        assert(lss.allow_drift());
        try
        {
            ll += lss.get_drift_prior();
        }
        catch(Exception& ex)
        {
            std::cerr << "Error occured while computing the drift prior.\n";
            return -std::numeric_limits<double>::max();
        }
    }
    else if(use_clo_ind_prior_ && !lss.ignore_clo())
    {
        // independent gaussian prior
        IFT(!use_clo_group_prior_, Illegal_argument, 
                "use_clo_ind_prior is set and use_group_prior can not be true");
        assert(!use_clo_group_prior_);
        double lss_prior = lss.get_clo_prior();
        if(lss_prior == -std::numeric_limits<double>::max())
        {
            return -std::numeric_limits<double>::max();
        }
        ll += lss_prior;
    }
    else if(use_clo_group_prior_)
    {
        // group (multivariate gaussian) prior
        double lss_prior = lss.get_full_gaussian_prior();
        if(lss_prior == -std::numeric_limits<double>::max())
        {
            return -std::numeric_limits<double>::max();
        }
        ll += lss_prior;
    }
    //std::cout << "ll1: " <<  ll<< std::endl;

    // Prior for the initial state
    if(use_init_prior_ && !lss.ignore_clo())
    {
        double temp = init_prior_(lss);
        ll += temp;
    }

    // likelihood 
    if(use_likelihood_)
    {
        double temp = likelihood_(lss);
        ll += temp;
    }
    //std::cout << "ll2: " <<  ll<< std::endl;

    // predicitive posterior p(y' | x) (not used at the moment)

    if(lss.polynomial_dim_per_osc() > 0)
    {
        double poly_prior = lss.get_polynomial_prior();
        if(poly_prior == -std::numeric_limits<double>::max())
        {
            return -std::numeric_limits<double>::max();
        }
        ll += poly_prior;
    }

    return ll;
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

double Lss_set_posterior::operator()(const Lss_set& lsss) const 
{ 
    double pos = 0.0;
    size_t num_lss = posteriors_.size();
    const std::vector<Linear_state_space>& lss_vec = lsss.lss_vec();
    assert(lss_vec.size() == num_lss);

    // Normal Inverse Gamma prior for CLO parameters
    if(use_lss_hyper_prior_)
    {
        pos += lss_prior_(lsss);
        if(pos == -std::numeric_limits<double>::max())
            return pos;
    }

    // Inverse Gamma prior for noise sigma in likelihood
    if(use_noise_prior_)
    {
        pos += noise_prior_(lsss);
        if(pos == -std::numeric_limits<double>::max())
            return pos;
    }

    for(size_t i = 0; i < num_lss; i++)
    {
        pos += posteriors_[i](lss_vec[i]);
    }

    return pos;
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

double Lss_set_posterior::clo_prior_posterior(const Lss_set& lsss) const 
{ 
    double pos = 0.0;
    size_t num_lss = posteriors_.size();
    const std::vector<Linear_state_space>& lss_vec = lsss.lss_vec();
    assert(lss_vec.size() == num_lss);

    // Normal Inverse Gamma prior for CLO parameters
    pos += lss_prior_.mean_var_prior(lsss);
    if(pos == -std::numeric_limits<double>::max())
        return pos;

    // Independent Gaussian prior for CLO parameteres
    for(size_t i = 0; i < num_lss; i++)
    {
        pos += lss_vec[i].get_clo_prior();
    }

    return pos;
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

double Lss_set_pred_posterior::operator()(const Lss_set& lsss) const
{
    std::for_each(posteriors_.begin(), posteriors_.end(), 
                  boost::bind(&Posterior::use_clo_ind_prior, _1, true));
    std::for_each(posteriors_.begin(), posteriors_.end(), 
                  boost::bind(&Posterior::use_init_prior, _1, true));

    std::vector<Linear_state_space>& lss_vec = lsss.lss_vec();
    size_t N = lss_vec.size();
    size_t num_threads = num_lss_threads_ <= N ? num_lss_threads_ : N;
    size_t avail_cores = boost::thread::hardware_concurrency();
    num_threads = num_threads > avail_cores ? avail_cores : num_threads;
    const size_t num_samples = 100;
    // Optimize couple-specific parameters  
    assert(posteriors_.size() == N);

    typedef Hmc_step_thread<Posterior, 
                            Lss_adapter, 
                            Lss_gradient<Lss_adapter> > Step_thread;

    //const std::vector<std::string>& out_dirs = lsss.opt_out_dirs();

    std::vector<std::vector<Linear_state_space> > samples(N);
    Lss_adapter lss_adapter;
    std::string step_name("lss-pred-opt");
    bool optimize = true;
    Step_thread step_thrd(
                        lss_vec, 
                        lss_adapter, 
                        posteriors_,
                        grad_step_sizes_, 
                        hmc_step_sizes_,
                        *out_dirs_,
                        samples,
                        step_name,
                        not_record_,
                        optimize,
                        num_leapfrogs_, 
                        num_samples);

    // disable use_pred_ 
    std::for_each(posteriors_.begin(), posteriors_.end(), 
                  boost::bind(&Posterior::use_pred, _1, false));
    boost::exception_ptr ep;
    if(num_threads == 1)
    {
        step_thrd.run(0, N-1, ep);
    }
    else
    {
        boost::thread_group thrds;
        for(size_t t = 0; t < num_threads; t++)
        {
            size_t st = N/num_threads * t;
            size_t l = (t == num_threads - 1 ? N : 
                        (N/num_threads) * (t+1));
            size_t en = l - 1;
            thrds.create_thread(bind(&Step_thread::run, 
                                     boost::ref(step_thrd), 
                                     st, en, boost::ref(ep)));
        }
        thrds.join_all();
    }

    double pr = 0.0;
    for(size_t i = 0; i < posteriors_.size(); i++)
    {
        pr += predictive_prob(posteriors_[i], samples[i]);
    }

    return pr;

}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

double Lss_set_pred_posterior_mh::operator()(const Lss_set& lsss) const
{
    // Optimize couple-specific parameters  
    std::vector<Linear_state_space>& lss_vec = lsss.lss_vec();

    double pr = 0.0;
    const size_t num_samples = 100;
    for(size_t i = 0; i < posteriors_.size(); i++)
    {
        pr += predictive_prob(posteriors_[i], lss_vec[i], num_samples);
    }

    return pr;
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

double Lss_set_pred_posterior_mh_v2::operator()(const Vector& shared_params) const
{
    return 0.0;
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

void Lss_set_pred_posterior_mh_v2::set_params
(
    Lss_set& lss_set
) const
{
    const std::vector<double>& variances = lss_set.variances();
    const std::vector<Vector>& coefs = lss_set.pred_coefs();
    size_t var_size = variances.size();
    size_t coef_size = lss_set_.pred_coef_size();
    int N = var_size + coef_size;
    Vector params(N, 0.0);
    Vector stds(N, 1.0);
    Vector::iterator in_it = params.begin();
    Vector::iterator std_it = stds.begin();
    BOOST_FOREACH(const Vector& coef, coefs)
    {
        BOOST_FOREACH(const double& val, coef)
        {
            *in_it = val;
            in_it++;
            *std_it = 0.1;
            std_it++;
        }
    }

    for(size_t i = 0; i < variances.size(); i++)
    {
        *in_it = log(variances[i]);
        in_it++;
    }
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

void kjb::ties::posterior_worker
(
    const std::vector<Posterior>& posteriors,
    const std::vector<Linear_state_space>& lss_vec,
    size_t start,
    size_t end,
    std::vector<double>& pos
)
{
    size_t N = posteriors.size();
    assert(lss_vec.size() == N);
    assert(start < N && end < N);

    // prior 
    for(size_t i = start; i <= end; i++)
    {
        pos[i] = posteriors[i](lss_vec[i]);
    }
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

std::vector<double> kjb::ties::individual_posteriors
(
    const std::vector<Linear_state_space>& lss_vec,
    const std::vector<Posterior>& posteriors,
    size_t num_threads
)
{
    size_t N = posteriors.size();
    assert(lss_vec.size() == N); 

    std::vector<double> pos(N, 0.0); 
    if(num_threads > N)
    {
        num_threads = N;
    }

    if(num_threads == 1)
    {
        posterior_worker(posteriors, lss_vec, 0, N - 1, pos);
    }
    else
    {
        boost::thread_group thrds;
        size_t avail_cores = boost::thread::hardware_concurrency();
        num_threads = num_threads > avail_cores ? avail_cores : num_threads;
        for(size_t i = 0; i < num_threads; i++)
        {
            size_t st = N/num_threads * i;
            size_t l = (i == num_threads - 1 ? N : (N/num_threads) * (i+1));
            assert(l >= 1);
            size_t en = l - 1;
            thrds.create_thread(boost::bind(posterior_worker,
                                    boost::cref(posteriors),
                                    boost::cref(lss_vec),
                                    st, 
                                    en, 
                                    boost::ref(pos)));
        }
        thrds.join_all();
    }

    return pos;
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

double Lss_set_gp_pred::operator()(const Vector& gp_params) const
{
    // Transform the log-transformed parameter back 
    size_t num_params = lss_set_.clo_param_size();
    IFT(gp_params.size() == 2 * num_params, Dimension_mismatch,
            "Can not compute GP predictive probability for Lss_set: "
            "number of gp_params differs from the number of CLO params.");

    Vector gp_params_orig(gp_params);
    std::transform(gp_params.begin(), gp_params.end(), 
                   gp_params_orig.begin(), static_cast<double(*)(double)> (&::exp));
    // Make a copy of the current lss_set
    cur_lss_set_ = lss_set_;

    // update the gp params 
    for(size_t i = 0; i < num_params; i++)
    {
        for(size_t j = 0; j < cur_lss_set_.lss_vec().size(); j++)
        {
            cur_lss_set_.update_gp_scales(j, i, gp_params_orig[i]);
            cur_lss_set_.update_gp_sigvars(j, i, 
                                        gp_params_orig[num_params + i]);
        }
    }

    // Find the best CLO parameters under the current GP parameters
   
    size_t N = cur_lss_set_.lss_vec().size();
    std::vector<std::vector<Linear_state_space> > samples_all(N);
    std::vector<Linear_state_space>& lss_vec = cur_lss_set_.lss_vec();
    const size_t num_samples = 100;
    Drift_step_thread sampler(lss_vec, 
                              posteriors_, 
                              out_dirs_,
                              init_pro_sigma_,
                              init_pro_sigma_, // need to change to poly_term_sigma_
                              samples_all,
                              ctr_pt_length_,
                              num_burn_iters_,
                              num_sample_iters_,
                              not_record_,
                              num_samples);

    try
    {
        boost::exception_ptr ep;
        run_threads(sampler, num_threads_, cur_lss_set_.lss_vec().size(), ep);

        double pr = 0.0;
        for(size_t i = 0; i < samples_all.size(); i++)
        {
            assert(num_samples == samples_all[i].size());
            if(samples_all[i].empty())
            {
                pr += predictive_prob(posteriors_[i], lss_vec[i]);
            }
            else
            {
                pr += predictive_prob(posteriors_[i], samples_all[i]);
            }
        }

        return pr;
    }
    //catch(kjb::KJB_error& err)
    catch(boost::exception& err)
    {
        std::cerr << boost::current_exception_diagnostic_information(); 
        std::cerr << "------------------\n";
        return -std::numeric_limits<double>::max();
    }

}
