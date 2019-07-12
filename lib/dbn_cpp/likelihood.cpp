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

/* $Id: likelihood.cpp 22559 2019-06-09 00:02:37Z kobus $ */

#include <m_cpp/m_vector.h>
#include <prob_cpp/prob_distribution.h>
#include <prob_cpp/prob_sample.h>

#include <vector>
#include <limits>

#include "dbn_cpp/likelihood.h"
#include "dbn_cpp/linear_state_space.h"
#include "dbn_cpp/lss_set.h"

#include <boost/thread.hpp>
#include <boost/bind.hpp>

using namespace kjb::ties;

#include <boost/ref.hpp>

double Likelihood::log_prob
(
    const Linear_state_space& lss, 
    size_t start_index,
    const std::vector<size_t>& time_indices
) const
{
    double ll = 0.0;
    State_vec_vec preds;
    bool no_sample = true;
    try
    {
        if(time_indices.empty())
        {
            preds = lss.get_states();
        }
        else
        {
            no_sample = false;
            preds = lss.get_states(time_indices);
            IFT(preds.size() == time_indices.size(),
                    Runtime_error,
                    "states and times have different sizes.");
        }
    }
    catch(Exception& ex)
    {
        std::cerr << " Error in getting states\n";
        return -std::numeric_limits<double>::max();
    }
    size_t train_size = preds.size();
    //std::cout << "preds size: " << train_size << std::endl;

    const Obs_map& observables = data_->observables;
    const Vector& noise_sigmas = lss.noise_sigmas();
    IFT(noise_sigmas.size() == lss.obs_names().size(), Runtime_error,
            "Observation noise sigma and observation names has "
            "different dimension.");
    size_t num_oscillators = lss.num_oscillators(); 
    size_t num_obs = lss.obs_names().size();
    const Coupled_oscillator& clo = lss.coupled_oscillators()[0];

    for(size_t i = start_index; i < train_size; i++)
    {
        size_t real_index = no_sample ? i : time_indices[i];
        const State_vec& pred = preds[real_index]; 
        // compare the true observable conditioned on the real hidden states
        // to the observed observables 
        for(size_t j = 0; j < num_obs; j++)
        {
            const std::string& obs_name = lss.obs_names()[j];
            Obs_map::const_iterator obs_it = observables.find(obs_name);
            KJB(ASSERT(obs_it != observables.end()));
            // loop through all the observables 
            const Vector_v& vals = obs_it->second;
            KJB(ASSERT(vals.size() == num_oscillators));
            for(int k = vals.size() - 1; k >= 0; k--)
            {
                double data_val = vals[k][real_index];
                double model_val = pred[j][k];
                if(std::isnan(std::fabs(model_val)) || std::isinf(std::fabs(model_val)))
                {
                    std::cerr << " BAD SAMPLE " << model_val << std::endl;
                    return -std::numeric_limits<double>::max();
                }
                if(!invalid_data(data_val))
                {
                    double diff = model_val - data_val;
                    if(std::isinf(std::fabs(noise_sigmas[j])) || 
                        std::isnan(std::fabs(noise_sigmas[j])) ||
                        noise_sigmas[j] <= 0.0)
                    {
                        std::cerr << "Warning: Likelihood: obs-noise is " 
                                  << noise_sigmas[j] << std::endl;
                        return -std::numeric_limits<double>::max();
                    }
                    Gaussian_distribution P(0.0, noise_sigmas[j]);
                    ll += log_pdf(P, diff);
                    if(std::isinf(std::fabs(ll)))
                    {
                        ll = -std::numeric_limits<double>::max();
                        break;
                    }
                }
            }
        }
    }
    return ll;
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

std::vector<double> kjb::ties::individual_likelihoods
(
    const Lss_set& lsss,
    const std::vector<Likelihood>& likelihoods,
    size_t num_threads
) 
{
    const std::vector<Linear_state_space>& lss_vec = lsss.lss_vec();
    assert(lss_vec.size() == likelihoods.size()); 

    std::vector<double> lls(likelihoods.size(), 0.0); 

    size_t N = likelihoods.size();
    if(num_threads > N)
    {
        num_threads = N;
    }
   
    if(num_threads == 1)
    {
        likelihood_worker(likelihoods, lss_vec, 0, N - 1, lls);
    }
    else
    {
        boost::thread_group thrds;
        size_t avail_cores = boost::thread::hardware_concurrency();
        num_threads = num_threads > avail_cores ?  avail_cores : num_threads;
        for(size_t i = 0; i < num_threads; i++)
        {
            size_t st = N/num_threads * i;
            size_t l = (i == num_threads - 1 ? N : (N/num_threads) * (i+1));
            size_t en = l - 1;
            thrds.create_thread(boost::bind(likelihood_worker,
                                    boost::cref(likelihoods),
                                    boost::cref(lss_vec),
                                    st, 
                                    en,
                                    ref(lls)));
        }
        thrds.join_all();
    }
    return lls;
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

Vector Likelihood::get_squared_errors(const Linear_state_space& lss) const
{
    const State_vec_vec& preds = lss.get_states();
    size_t num_training = preds.size();
    const Obs_map& observables = data_->observables;
    size_t num_obs = lss.obs_names().size();
    size_t num_osc = lss.num_oscillators();

    Vector errors((int)num_obs, 0.0); 
    for(size_t i = 0; i < num_training; i++)
    {
        const State_vec& pred = preds[i]; 
        // compare the true observable conditioned on the real hidden states
        // to the observed observables 
        for(size_t j = 0; j < num_obs; j++)
        {
            double error_per_obs = 0.0;
            const std::string& obs_name = lss.obs_names()[j];
            Obs_map::const_iterator obs_it = observables.find(obs_name);
            KJB(ASSERT(obs_it != observables.end()));
            // loop through all the observables 
            const Vector_v& vals = obs_it->second;
            KJB(ASSERT(vals.size() == num_osc));
            // for each oscillator at the current observation
            for(size_t k = 0; k < num_osc; k++)
            {
                //const Vector& data_val = vals[k];
                double data_val = vals[k][i];
                if(!invalid_data(data_val))
                {
                    double model_val = pred[j][k];
                    double diff = model_val - data_val;
                    error_per_obs += diff * diff;
                }
            }
            errors[j] += error_per_obs/num_osc;
        }
    }
    return errors;
}

