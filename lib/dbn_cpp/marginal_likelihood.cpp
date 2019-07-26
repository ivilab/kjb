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

/* $Id: marginal_likelihood.cpp 22559 2019-06-09 00:02:37Z kobus $ */

#include <n_cpp/n_cholesky.h>
#include <diff_cpp/diff_optim.h>
#include <vector>
#include <algorithm>
#include <map>

#include "dbn_cpp/marginal_likelihood.h"
#include "dbn_cpp/linear_state_space.h"
#include "dbn_cpp/thread_worker.h"
#include "dbn_cpp/gradient.h"
#include "dbn_cpp/adapter.h"
#include "dbn_cpp/posterior.h"
#include "dbn_cpp/lss_set.h"


using namespace kjb;
using namespace kjb::ties;

double Marginal_likelihood::operator()(const Lss_set& lsss)
{
    std::vector<Linear_state_space>& lss_vec = lsss.lss_vec();

    size_t num_lss = lss_vec.size();
    assert(posteriors_.size() == num_lss);

    // Set the means and sigmas for the ind_gaussian_prior
    lsss.update_means();
    lsss.update_variances();

    std::string step_name("lss-opt-step");
    const size_t num_samples = 100;
    std::vector<std::vector<Linear_state_space> > samples(num_lss);
    // Compute the covariance matrix
    std::vector<Matrix> lss_cov(num_lss);
    bool optimize = true;
    bool adapt = true;

    if(opt_.sample_method == "hmc")
    {
        typedef Hmc_step_thread<Posterior, Lss_adapter, 
                Lss_gradient<Lss_adapter> > Step_thread;

        Lss_adapter adapter;
        // Create the thread class 
        Step_thread hmc_step_thrd(
                lss_vec, adapter, posteriors_,
                opt_.grad_step_sizes, opt_.hmc_step_sizes,
                sample_out_dirs_,
                samples, 
                step_name,
                not_record_,
                optimize,
                opt_.num_leapfrogs,
                num_samples);

        using namespace boost;
        thread_group thrds;
        boost::exception_ptr ep;
        for(size_t i = 0; i < opt_.num_threads; i++)
        {
            size_t st = num_lss/opt_.num_threads * i;
            size_t l = (i == opt_.num_threads - 1 ? num_lss : 
                        (num_lss/opt_.num_threads) * (i+1));
            size_t en = l - 1;
            thrds.create_thread(bind(
                        &Step_thread::run, 
                        hmc_step_thrd, st, en, 
                        boost::ref(ep)));
        }
        // join threads
        thrds.join_all();

        assert(samples.size() == num_lss);
        for(size_t i = 0; i < num_lss; i++)
        {
            lss_cov[i] = compute_covariance(samples[i], adapter);
        }
    }
    else
    {
        assert(opt_.sample_method == "mh");
        Lss_set best_lss_set(lsss);
        Mh_step_thread step_thrd(lss_vec, 
                                 posteriors_, 
                                 sample_out_dirs_, 
                                 opt_.init_state_sigma, 
                                 opt_.clo_param_sigma, 
                                 opt_.clo_param_sigma,
                                 samples, 
                                 not_record_, 
                                 not_record_, 
                                 optimize, 
                                 Cluster_prior(),
                                 lsss.group_params(),
                                 adapt, 
                                 num_samples);
        using namespace boost;
        thread_group thrds;
        boost::exception_ptr ep;
        for(size_t i = 0; i < opt_.num_threads; i++)
        {
            size_t st = num_lss/opt_.num_threads * i;
            size_t l = (i == opt_.num_threads - 1 ? num_lss : 
                        (num_lss/opt_.num_threads) * (i+1));
            size_t en = l - 1;
            thrds.create_thread(bind(&Mh_step_thread::run, 
                                     step_thrd, 
                                     st, 
                                     en, 
                                     boost::ref(ep)));
        }
        // join threads
        thrds.join_all();
        assert(samples.size() == num_lss);

        Lss_adapter adapter; 
        for(size_t i = 0; i < num_lss; i++)
        {
            lss_cov[i] = compute_covariance(samples[i], adapter);
        }
    }

    double log_det_cov = 0.0;
    BOOST_FOREACH(const Matrix& m, lss_cov)
    {
        //double temp = log_det(m);
        double temp = log(m.abs_of_determinant());
        //std::cout << "log_det(m): " << temp << std::endl;
        log_det_cov += temp; 
    }

    double log_det_H = -log_det_cov; 

    std::cout << "log_det_H: " << log_det_H << std::endl;

    // compute the posteriors 
    std::vector<double> pos = individual_posteriors(lss_vec, 
                                                    posteriors_, 
                                                    opt_.num_threads);
    double lp = std::accumulate(pos.begin(), pos.end(), 0.0);

    size_t D = 0;
    size_t sigma_size = lsss.variances().size();
    for(size_t i = 0; i < lss_vec.size(); i++)
    {
        //TODO not including observation indices here yet
        D += lss_vec[i].init_state().size() + sigma_size;
    }
    double p = -0.5*log_det_H + (D/2.0)*std::log(2 * M_PI);

    // prior
    double pr = shared_prior_(lsss);
    double dl = p + (pr + lp); 

    return dl;
}

