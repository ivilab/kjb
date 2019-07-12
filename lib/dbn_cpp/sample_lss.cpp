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

/* $Id: sample_lss.cpp 22559 2019-06-09 00:02:37Z kobus $ */

#include <m_cpp/m_vector.h>

#include <vector>
#include <string>
#include <fstream>
#include <algorithm>
#include <iterator>

#include <boost/none.hpp>
#include <boost/optional/optional.hpp>

#include "dbn_cpp/prior.h"
#include "dbn_cpp/posterior.h"
#include "dbn_cpp/linear_state_space.h"
#include "dbn_cpp/proposer.h"
#include "dbn_cpp/sample_lss.h"
#include "dbn_cpp/drift_sampler.h"

using namespace kjb;
using namespace kjb::ties;

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

std::vector<Linear_state_space> kjb::ties::generate_lss_samples
(
    const Linear_state_space& lss,
    const Posterior& posterior,
    size_t num_samples,
    boost::optional<const std::string&> sample_target_fp,
    boost::optional<const std::string&> sample_log_fp,
    bool not_record,
    size_t burn_in
)
{
    std::vector<Linear_state_space> samples(num_samples);
    const double init_sigma = 0.001;

    std::ofstream sample_target_ofs;
    std::ofstream sample_log_ofs;
    if(!not_record)  
    {
        assert(sample_target_fp && sample_log_fp);
        sample_target_ofs.open(sample_target_fp->c_str(), std::ofstream::app);
        sample_log_ofs.open(sample_log_fp->c_str(), std::ofstream::app);

        IFTD(!sample_target_ofs.fail(), IO_error, 
                "Can't open file %s", (sample_target_fp->c_str()));
        IFTD(!sample_log_ofs.fail(), IO_error, 
                "Can't open file %s", (sample_log_fp->c_str()));

        assert(sample_target_ofs && sample_log_ofs);
    }

    if(!lss.allow_drift())
    {
        const double clo_param_sigma = 0.0005; 
        Vector init_prop_sigmas((int)lss.init_state().size(), init_sigma);
        int num_params = lss.coupled_oscillators()[0].num_params();
        num_params += lss.num_polynomial_coefs();
        Vector clo_prop_sigmas(num_params, clo_param_sigma);

        Vector prop_sigmas;
        std::copy(clo_prop_sigmas.begin(), clo_prop_sigmas.end(), 
                  std::back_inserter(prop_sigmas));
        std::copy(init_prop_sigmas.begin(), init_prop_sigmas.end(), 
                  std::back_inserter(prop_sigmas));

        Lss_mh_proposer proposer(prop_sigmas);

        boost::mt19937 rng;
        ergo::mh_step<Linear_state_space> step(posterior, proposer);
        // add sample recorder
        step.add_recorder(ergo::make_sample_recorder(samples.begin()));

        if(!not_record)  
        {
            // target recorder
            ergo::target_recorder<std::ostream_iterator<double> > 
                target_recorder(std::ostream_iterator<double>(sample_target_ofs, "\n"));
            step.add_recorder(target_recorder);

            step.add_recorder(
                    ergo::make_mh_detail_recorder(
                        std::ostream_iterator<ergo::step_detail>(
                            sample_log_ofs, "\n")));
        }

        Linear_state_space lss_copy(lss);
        double lp = posterior(lss_copy);

        // If burnin is set 
        for(size_t i = 0; i < burn_in; i++)
        {
            step(lss_copy, lp);
        }

        for(size_t i = 0; i < num_samples; i++)
        {
            step(lss_copy, lp);
            if(!not_record)
            {
                sample_target_ofs.flush();
                sample_log_ofs.flush();
            }
        }
    }
    else
    {
        const size_t ctr_pt_length = 25;
        Drift_sampler sampler(posterior, 
                              lss,
                              ctr_pt_length,
                              burn_in, 
                              sample_log_fp, 
                              sample_target_fp);

        for(size_t i = 0; i < num_samples; i++)
        {
            sampler.sample();
            samples[i] = sampler.current();
        }
    }
    if(!not_record) 
    {
        sample_target_ofs << " =========================================\n";
    }

    return samples;
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

bool kjb::ties::sample_lss
(
    Linear_state_space& lss, 
    double max_val, 
    double min_val
)
{
    size_t counts = 0;
    bool valid_sample = false;
    const size_t MAX_TRIES = 1000;
    do
    {
        if(lss.allow_drift())
        {
            valid_sample = lss.sample_clo_from_gp_prior();
        }
        else
        {
            valid_sample = lss.sample_clo_from_ind_gauss_prior();
        }
        counts++;
        if(counts > MAX_TRIES) return valid_sample;
    } while(!valid_sample || !in_range(lss, max_val, min_val));
    return valid_sample;
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

bool kjb::ties::sample_lss(Lss_set& lss_set, double max_val, double min_val)
{
    std::vector<Linear_state_space>& lss_vec = lss_set.lss_vec();
    size_t num_lss = lss_vec.size();
    KJB(ASSERT(!lss_set.variances().empty()));
    KJB(ASSERT(!lss_set.fixed_clo()));
    if(num_lss == 0) return true;

    // make sure the mean is update to date
    lss_set.update_means();
    lss_set.update_variances();
    bool valid_sample = false;
    for(size_t i = 0; i < num_lss; i++)
    {
        valid_sample = sample_lss(lss_vec[i], max_val, min_val);
        if(!valid_sample) return valid_sample;
        lss_vec[i].sample_polynomial_coefs();
    }
    assert(valid_sample == true);
    return valid_sample;
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

double kjb::ties::predictive_prob
(
    const Posterior& posterior, 
    const Linear_state_space& lss,
    size_t num_samples
)
{
    if(num_samples == 0)
    {
        // get the status
        bool use_pred = posterior.get_use_pred();
        bool use_lss_prior = posterior.get_use_clo_ind_prior();
        bool use_init_prior = posterior.get_use_init_prior();
        // set the status 
        posterior.use_pred(true);
        posterior.use_clo_ind_prior(false);
        posterior.use_init_prior(false);
        double pr = posterior.likelihood().predictive_probability(lss);
        // reset the status 
        posterior.use_pred(use_pred);
        posterior.use_clo_ind_prior(use_lss_prior);
        posterior.use_init_prior(use_init_prior);
        return pr;
    }
    std::vector<Linear_state_space> samples = generate_lss_samples(lss, 
                                                         posterior, 
                                                         num_samples);
    return predictive_prob(posterior, samples);
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

double kjb::ties::predictive_prob
(
    const Posterior& posterior, 
    const std::vector<Linear_state_space>& samples
)
{
    size_t num_samples = samples.size();
    //std::vector<double> prs(num_samples); 
    double pr = 0.0;
    size_t N = 0;
    for(size_t i = 0; i < num_samples; i++)
    {
        // only use valid samples (not sure if this is 
        // mathmatically correct or not)
        double cur_pr = posterior.likelihood().predictive_probability(samples[i]);
        if(cur_pr != -std::numeric_limits<double>::max())
        {
            pr += cur_pr; 
            N++;
        }
    }

    return pr/N;
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

bool kjb::ties::sample_noise_data
(
    const Linear_state_space& lss, 
    Data& data, 
    bool noise_free
)
{
    data.group_index = lss.group_index();
    const State_vec_vec& states = lss.get_states();
    Vector& noise_sigmas = lss.noise_sigmas();
    size_t time_length = data.times.size();
    assert(states.size() == time_length);
   
    for(size_t m = 0; m < time_length; m++)
    {
        for(size_t j = 0; j < lss.obs_names().size(); j++)
        {
            // Get corresponding data
            const std::string& obs_name = lss.obs_names()[j];
            Vector_v& noisy_data = data.observables.at(obs_name);

            // Add in noise 
            Gaussian_distribution P(0.0, noise_sigmas[j]);
            for(size_t n = 0; n < lss.num_oscillators(); n++)
            {
                double model_val = states[m][j][n]; 
                if(noise_free)
                {
                    noisy_data[n][m] = model_val;
                }
                else
                {
                    if(std::isnan(model_val))
                    {
                        return false;
                    }
                    noisy_data[n][m] = model_val + sample(P);
                }
            }
        }
    }

    return true;
}
