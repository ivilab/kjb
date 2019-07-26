#include <l/l_init.h>
#include <l_cpp/l_test.h>
#include <prob_cpp/prob_distribution.h>
#include <prob_cpp/prob_sample.h>
#include <prob_cpp/prob_stat.h>
#include <vector>
#include <algorithm>
#include <boost/bind.hpp>

using namespace std;
using namespace kjb;

template<class D>
bool is_sample_good(const D& P, int nsamples, int nparams, double alpha, int nbins)
{
    vector<double> samples(nsamples);
    generate(samples.begin(), samples.end(),
             boost::bind(static_cast<double (*)(const D&)>(sample), P));
    return g_test(samples.begin(), samples.end(), P, alpha, nparams, nbins);
}

template<class D>
bool is_sample_good(const D& P, int nsamples, double alpha, int dofr)
{
    vector<double> samples(nsamples);
    generate(samples.begin(), samples.end(),
             boost::bind(static_cast<double (*)(const D&)>(sample), P));
    return chi_square_test(samples.begin(), samples.end(), P, alpha, dofr);
}

int main(int argc, char** argv)
{
    kjb_c::kjb_init();

    int num_samples = 5000;
    double alpha = 0.001;
    int num_bins = 25;

    //bernoulli distribution
    TEST_TRUE(is_sample_good(Bernoulli_distribution(0.2),
                             num_samples, alpha, 1));

    //beta distribution
    TEST_TRUE(is_sample_good(Beta_distribution(2, 5), num_samples,
                             2, alpha, num_bins));

    //binomial distribution
    TEST_TRUE(is_sample_good(Binomial_distribution(40, 0.4),
                             num_samples, alpha, 3));

    //categorical distribution with doubles
    map<double, double> m;
    m[0.1] = 0.1;
    m[0.2] = 0.2;
    m[0.3] = 0.3;
    m[0.4] = 0.4;
    TEST_TRUE(is_sample_good(Categorical_distribution<double>(m),
                             num_samples, alpha, 3));

    //chi-square distribution
    TEST_TRUE(is_sample_good(Chi_square_distribution(3), num_samples,
                             1, alpha, num_bins));

    //exponential distribution
    TEST_TRUE(is_sample_good(Exponential_distribution(1.5),
                             num_samples, 1, alpha, num_bins));

    //gamma distribution
    TEST_TRUE(is_sample_good(Gamma_distribution(2, 2.0), num_samples,
                             2, alpha, num_bins));

    //gaussian distribution
    TEST_TRUE(is_sample_good(Normal_distribution(0.0, 1.0),
                             num_samples, 2, alpha, num_bins));

    //laplace distribution
    TEST_TRUE(is_sample_good(Laplace_distribution(0.0, 1.0),
                             num_samples, 2, alpha, num_bins));

    //poisson distribution
    TEST_TRUE(is_sample_good(Poisson_distribution(4), num_samples, alpha, 2));

    //uniform distribution
    TEST_TRUE(is_sample_good(Uniform_distribution(0.0, 1.0),
                             num_samples, 2, alpha, num_bins));

    //mixture of two gaussians
    TEST_TRUE(is_sample_good(Mixture_distribution<Normal_distribution>(
        Normal_distribution(0.0, 1.0), Normal_distribution(-1.0, 0.4), 0.2),
        num_samples, 5, alpha, num_bins));

    RETURN_VICTORIOUSLY();
}

