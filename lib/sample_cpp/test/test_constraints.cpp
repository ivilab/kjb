#include <l/l_init.h>
#include <iostream>
#include "sample_cpp/sample_sampler.h"
#include "sample_cpp/sample_proposer.h"
#include "sample_cpp/sample_recorder.h"
#include "sample_cpp/sample_real.h"
#include "prob_cpp/prob_distribution.h"
#include "prob_cpp/prob_pdf.h"
#include "prob_cpp/prob_stat.h"
#include "l_cpp/l_exception.h"
#include <l_cpp/l_test.h>

using namespace std;
using namespace kjb;

const int num_samples = 10000;

typedef Single_step_sampler<double> Sampler;

int main(int /*argc*/, char** /* argv */)
{
    kjb_c::kjb_init();

    try
    {
        // create target distribution
        Gaussian_distribution P(0.0, 5.0);
        Model_evaluator<double>::Type log_target = boost::bind(
            static_cast<double (*)(const Gaussian_distribution&, double)>(log_pdf), P, _1);


        // create a recorder that stores every sampled model configuration.
        All_model_recorder<double> recorder;

        // intitialize model
        double initial_model = 1.0;
        // find models log-probability under the target distribution (lt = "log target")
        double initial_lt = log_target(initial_model);

    // GRADIENT
        // This creates a gradient function, given a function.
        // Gradients will be approximated numerically, using delta of 0.001.
        const double NEIGHBORHOOD_SIZE = 0.001;
        Real_numerical_gradient<double> gradient(log_target, NEIGHBORHOOD_SIZE);

    // HMC SAMPLER
        // create a Hybrid Monte Carlo Step
        const size_t LEAPFROG_STEPS = 50;
        const double STEP_SIZE = 0.01;
        const double lo_bound = -1.0;
        const double up_bound = 1.0;
        Real_hmc_step<double, true> hmc_step(log_target, LEAPFROG_STEPS, gradient, STEP_SIZE, lo_bound, up_bound);
        //Real_hmc_step<double> hmc_step(log_target, LEAPFROG_STEPS, gradient, STEP_SIZE);

        // create a hybrid monte carlo sampler.
        // Note the similarity to creating the Metropolis-hastings sampler.  The
        // only difference is the step type.
        Sampler hmc_sampler(hmc_step, initial_model, initial_lt);
        hmc_sampler.add_recorder(recorder);

        // run Hybrid Monte carlo
        hmc_sampler.run(num_samples);
        // get all samples. 
        // (We ignore burn-in because the Markov Chain mixes quickly)
        vector<double> hmc_samples = hmc_sampler.get_recorder<All_model_recorder<double> >(0).get();
        //copy(hmc_samples.begin(), hmc_samples.end(), ostream_iterator<double>(cout, " "));

        // find min and max values
        double max_val = *max_element(hmc_samples.begin(), hmc_samples.end());
        double min_val = *min_element(hmc_samples.begin(), hmc_samples.end());
        TEST_TRUE(max_val < up_bound);
        TEST_TRUE(min_val > lo_bound);

        // find max target
        double hmc_max = *max_element(hmc_samples.begin(), hmc_samples.end(),
                                      boost::bind(less<double>(),
                                                  boost::bind(log_target, _1),
                                                  boost::bind(log_target, _2)));

        double real_max = 0.0;
        double epsilon = 0.05;
        TEST_TRUE(fabs(hmc_max - real_max) < epsilon);

        RETURN_VICTORIOUSLY();
    }
    catch(const Exception& ex)
    {
        ex.print_details_exit();
    }

    return EXIT_SUCCESS;
}

