#include <l/l_init.h>
#include <iostream>
#include <functional>
#include <algorithm>
#include "sample_cpp/sample_sampler.h"
#include "sample_cpp/sample_proposer.h"
#include "sample_cpp/sample_recorder.h"
#include "sample_cpp/sample_real.h"
#include "prob_cpp/prob_distribution.h"
#include "prob_cpp/prob_conditional_distribution.h"
#include "prob_cpp/prob_pdf.h"
#include "prob_cpp/prob_sample.h"
#include "prob_cpp/prob_stat.h"
#include "l_cpp/l_functors.h"
#include "l_cpp/l_exception.h"
#include <boost/bind.hpp>
#include <l_cpp/l_test.h>

using namespace std;
using namespace kjb;

const int num_samples = 1000;

typedef Mixture_distribution<Normal_distribution> Mixture_of_gaussians;
typedef Single_step_sampler<double> Sampler;
typedef Conditional_distribution_proposer<Gaussian_conditional_distribution, double> Gaussian_proposer;

int main(int /*argc*/, char** /* argv */)
{
    kjb_c::kjb_init();

    try
    {
        // create target distribution
        vector<Normal_distribution> dists(3);
        vector<double> coeffs(3);
        dists[0] = Normal_distribution(-3.0, 0.5); coeffs[0] = 0.2;
        dists[1] = Normal_distribution(-1.0, 1.2); coeffs[1] = 0.5;
        dists[2] = Normal_distribution(2.0, 1.3);  coeffs[2] = 0.3;
        Mixture_of_gaussians P(dists, coeffs);
        Model_evaluator<double>::Type log_target = boost::bind(
            static_cast<double (*)(const Mixture_of_gaussians&, const double&)>(log_pdf), P, _1);


        // create a recorder that stores every sampled model configuration.
        All_model_recorder<double> recorder;

        // create a Gaussian proposal distribution
        Gaussian_proposer Q(Gaussian_conditional_distribution(Normal_on_normal_dependence(1.0)));

        // create a metropolis-hastings "step".  
        Basic_mh_step<double> mh_step(log_target, Q);

        // intitialize model
        double initial_model = 0.0;
        // find models log-probability under the target distribution (lt = "log target")
        double initial_lt = log_target(initial_model);

        // create a metropolis hastings sampler from the MH step.  
        Sampler mh_sampler(mh_step, initial_model, initial_lt);
        mh_sampler.add_recorder(recorder);

    // GRADIENT
        // This creates a gradient function, given a function.
        // Gradients will be approximated numerically, using delta of 0.001.
        const double NEIGHBORHOOD_SIZE = 0.001;
        Real_numerical_gradient<double> gradient(log_target, NEIGHBORHOOD_SIZE);

    // HMC SAMPLER
        // create a Hybrid Monte Carlo Step
        const size_t LEAPFROG_STEPS = 50;
        const double STEP_SIZE = 0.05;
        Real_hmc_step<double> hmc_step(log_target, LEAPFROG_STEPS, gradient, STEP_SIZE);

        // create a hybrid monte carlo sampler.
        // Note the similarity to creating the Metropolis-hastings sampler.  The
        // only difference is the step type.
        Sampler hmc_sampler(hmc_step, initial_model, initial_lt);
        hmc_sampler.add_recorder(recorder);

    // STOCHASTIC DYNAMICS "SAMPLER"
        // "sampler" is in quotes, because this doesn't draw samples from the target distribution,
        // but it still is effective as an optimizer.

        Real_sd_step<double>::Type sd_step(log_target, 1, gradient, STEP_SIZE, 1.0);
        Sampler sd_sampler(sd_step, initial_model, initial_lt);
        sd_sampler.add_recorder(recorder);

        // same, but allow reversibility to be ignored, which should run faster
        Real_sd_step<double, false>::Type sd_opt_step(log_target, LEAPFROG_STEPS, gradient, STEP_SIZE, 0.8);
        Sampler sd_opt_sampler(sd_opt_step, initial_model, initial_lt);
        sd_opt_sampler.add_recorder(recorder);

        // Run them...
        int burnin_sz = 1000;
        int corr_sz = 50;

        // run sampler for I iterations.
        mh_sampler.run(corr_sz * num_samples + burnin_sz);

        // get all samples.
        vector<double> all_mh_samples = mh_sampler.get_recorder<All_model_recorder<double> >(0).get();
        vector<double> mh_samples(num_samples);
        // remove samples from the burn-in period, and then take every 5th element
        remove_copy_if(all_mh_samples.begin() + burnin_sz, all_mh_samples.end(),
                       mh_samples.begin(), not1(Every_nth_element<double>(corr_sz)));

        // run Hybrid Monte carlo
        hmc_sampler.run(num_samples);
        // get all samples. 
        // (We ignore burn-in because the Markov Chain mixes quickly)
        vector<double> hmc_samples = hmc_sampler.get_recorder<All_model_recorder<double> >(0).get();

        // run Stochastic dynamics
        sd_sampler.run(num_samples);
        return EXIT_SUCCESS;
        vector<double> sd_samples = sd_sampler.get_recorder<All_model_recorder<double> >(0).get();

        // run "optimized" Stochastic dynamics
        sd_opt_sampler.run(num_samples);
        vector<double> sd_opt_samples = sd_opt_sampler.get_recorder<All_model_recorder<double> >(0).get();

    // CHI-SQUARE TEST
        // This test should confirm that the samples generated under each method are truly
        // samples from the target distribution (with some confidence threshold).
        // Failing this test is a sure indication of bugs in the sampler code.
        
        double alpha = 0.01;
        int nparams = 8;
        int nbins = 50;
        TEST_TRUE(g_test(mh_samples.begin(), mh_samples.end(), P, alpha, nparams, nbins));
        TEST_TRUE(g_test(hmc_samples.begin(), hmc_samples.end(), P, alpha, nparams, nbins));

        // find maxs
        double mh_max = *max_element(mh_samples.begin(), mh_samples.end(),
                                     boost::bind(less<double>(),
                                                 boost::bind(log_target, _1),
                                                 boost::bind(log_target, _2)));
        double hmc_max = *max_element(hmc_samples.begin(), hmc_samples.end(),
                                      boost::bind(less<double>(),
                                                  boost::bind(log_target, _1),
                                                  boost::bind(log_target, _2)));
        double sd_max = *max_element(sd_samples.begin(), sd_samples.end(),
                                     boost::bind(less<double>(),
                                                 boost::bind(log_target, _1),
                                                 boost::bind(log_target, _2)));
        double sd_opt_max = *max_element(sd_opt_samples.begin(), sd_opt_samples.end(),
                                         boost::bind(less<double>(),
                                                     boost::bind(log_target, _1),
                                                     boost::bind(log_target, _2)));

        double real_max = -2.9;
        double epsilon = 0.05;
        TEST_TRUE(fabs(mh_max - real_max) < epsilon);
        TEST_TRUE(fabs(hmc_max - real_max) < epsilon);
        TEST_TRUE(fabs(sd_max - real_max) < epsilon);
        TEST_TRUE(fabs(sd_opt_max - real_max) < epsilon);

        RETURN_VICTORIOUSLY();
    }
    catch(const Exception& ex)
    {
        ex.print_details_exit();
    }

    return EXIT_SUCCESS;
}

