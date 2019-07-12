#include <iostream>
#include <functional>
#include "sample_cpp/sample_sampler.h"
#include "sample_cpp/sample_proposer.h"
#include "sample_cpp/sample_recorder.h"
#include "sample_cpp/sample_vector.h"
#include "prob_cpp/prob_distribution.h"
#include "prob_cpp/prob_conditional_distribution.h"
#include "prob_cpp/prob_pdf.h"
#include "prob_cpp/prob_sample.h"
#include "l_cpp/l_functors.h"
#include "l_cpp/l_exception.h"

#include <boost/bind.hpp>

using namespace std;
using namespace kjb;

//const int d = 4;
const int num_samples = 5000;

typedef Conditional_distribution_proposer<MV_gaussian_conditional_distribution, Vector> MV_gaussian_proposer;
typedef Single_step_sampler<Vector > Sampler;

struct MV_gaussian_full_conditional_proposer
{
    MV_gaussian_full_conditional_proposer(const MV_gaussian_distribution& P) :
        m_P(P)
    {}

    MV_gaussian_full_conditional_proposer(const MV_gaussian_full_conditional_proposer& Q) :
        m_P(Q.m_P)
    {}

    MV_gaussian_full_conditional_proposer& operator=(const MV_gaussian_full_conditional_proposer& Q)
    {
        if(this != &Q)
        {
            m_P = Q.m_P;
        }

        return *this;
    }

    boost::optional<double> operator()(Vector& x, int i) const
    {
        MV_gaussian_conditional_distribution P = m_P.conditional(i);
        Vector x_not_i(x.get_length() - 1);
        std::copy(x.begin(), x.begin() + i, x_not_i.begin());
        std::copy(x.begin() + i + 1, x.end(), x_not_i.begin() + i);
        x[i] = conditional_sample(P, x_not_i)[0];

        return boost::none;
    }

    MV_gaussian_distribution m_P;
};

int main(int /* argc */, char** /* argv */)
{
    try
    {
        // create target
        Vector mu("test_all_simple_mu");
        Matrix Sigma("test_all_simple_sigma");
        MV_gaussian_distribution P(mu, Sigma);
        Model_evaluator<Vector>::Type log_target = boost::bind(static_cast<double (*)(const MV_gaussian_distribution&, const Vector&)>(kjb::log_pdf), P, _1);
        int d = mu.get_length();

        // create initial values
        Vector initial_model(d, 0.0);
        double initial_lt = log_target(initial_model);

        // The Expectation_recorder will pass each sample to a function and take the
        // average of the results. This is the Monte Carlo algorithm for approximating 
        // the expected value of the function under the target distribution.  Here, the 
        // function is simply the identity function.

        // Metropolis-Hastings sampler
        Matrix Sigma_Q = create_identity_matrix(d);
        MV_gaussian_proposer Q(MV_gaussian_conditional_distribution(MV_normal_on_normal_dependence(Sigma_Q, d)));
        Basic_mh_step<Vector> mh_step(log_target, Q);
        // (note the typedef for Sampler at the top of the file)
        Sampler mh_sampler(mh_step, initial_model, initial_lt);

        // add recorder
        Expectation_recorder<Vector, Vector> mh_recorder(Identity<Vector>(), Vector(d, 0.0));
        mh_sampler.add_recorder(&mh_recorder);

//        // Gibbs sampler
        MV_gaussian_full_conditional_proposer P_fc(P);
        Basic_gibbs_step<Vector> gibbs_step(log_target, P_fc, get_vector_model_dimension<Vector>);
        Sampler gibbs_sampler(gibbs_step, initial_model, initial_lt);
        Expectation_recorder<Vector, Vector> gibbs_recorder(Identity<Vector>(), Vector(d, 0.0));
        gibbs_sampler.add_recorder(&gibbs_recorder);

        // HMC sampler
        Vector_numerical_gradient<Vector> gradient(log_target, Constant_parameter_evaluator<Vector>(kjb::Vector(d, 0.001)));
        Vector_hmc_step<Vector> hmc_step(log_target, 100, gradient, Constant_parameter_evaluator<Vector>(kjb::Vector(d, 0.005)));
        Sampler hmc_sampler(hmc_step, initial_model, initial_lt);

        Expectation_recorder<Vector, Vector> hmc_recorder(Identity<Vector>(), Vector(d, 0.0));
        hmc_sampler.add_recorder(&hmc_recorder);

        // SD "sampler"
        Vector_sd_step<Vector>::Type sd_step(log_target, 1, gradient, Constant_parameter_evaluator<Vector>(kjb::Vector(d, 0.05)), 0.8);
        Sampler sd_sampler(sd_step, initial_model, initial_lt);

        Expectation_recorder<Vector, Vector> sd_recorder(Identity<Vector>(), Vector(d, 0.0));
        sd_sampler.add_recorder(&sd_recorder);


        // SD "sampler" ("optimized")
        Vector_sd_step<Vector, false>::Type sd_opt_step(log_target, 1, gradient, Constant_parameter_evaluator<Vector>(kjb::Vector(d, 0.05)), 0.8);
        Sampler sd_opt_sampler(sd_opt_step, initial_model, initial_lt);

        Expectation_recorder<Vector, Vector> sd_opt_recorder(Identity<Vector>(), Vector(d, 0.0));
        sd_opt_sampler.add_recorder(&sd_opt_recorder);

        // Run them...
        mh_sampler.run(num_samples);
        Vector mh_ex = mh_recorder.get();

        gibbs_sampler.run(num_samples);
        Vector gibbs_ex = gibbs_recorder.get();

        hmc_sampler.run(num_samples / 100);
        Vector hmc_ex = hmc_recorder.get();

        sd_sampler.run(num_samples);
        Vector sd_ex = sd_recorder.get();

        sd_opt_sampler.run(num_samples);
        Vector sd_opt_ex = sd_opt_recorder.get();

        // Output
        cout << "Real expectaton:\t\t\t\t" << mu << endl;
        cout << "Estimated expectation from " << num_samples << " MH samples:\t" << mh_ex << endl;
        cout << "Estimated expectation from " << num_samples << " Gibbs samples:\t" << gibbs_ex << endl;
        cout << "Estimated expectation from " << num_samples / 100 << " HMC samples:\t" << hmc_ex << endl;
        cout << "Estimated expectation from " << num_samples << " SD samples:\t" << sd_ex << endl;
        cout << "Estimated expectation from " << num_samples << " SD samples:\t" << sd_opt_ex << endl;
    }
    catch(const Exception& ex)
    {
        ex.print_details_exit();
    }

    return EXIT_SUCCESS;
}

