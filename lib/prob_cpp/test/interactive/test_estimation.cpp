#include <prob_cpp/prob_distribution.h>
#include <prob_cpp/prob_sample.h>
#include <prob_cpp/prob_estimation.h>
#include <m_cpp/m_vector_d.h>
#include <algorithm>
#include <vector>
#include <iostream>
#include <boost/bind.hpp>

using namespace kjb;

typedef Von_mises_fisher_distribution<3> Vmf_distribution;

int main(int argc, char** argv)
{
    const double m = 10.0;
    const double s = 4.0;
    const double a = 2.0;
    const double b = 5.0;
    const double t = 3.0;
    const double k = 5.0;
    const Vector3 mu = Vector3(1.0, 1.0, 1.0).normalized();
    const double kappa = 20.0;

    const size_t N = 100000;

    Normal_distribution norm(m, s);
    Beta_distribution beta(a, b);
    Gamma_distribution gamma(t, k);
    Vmf_distribution vmf(mu, kappa);

    std::vector<double> n_samples(N);
    std::vector<double> b_samples(N);
    std::vector<double> g_samples(N);
    std::vector<Vector3> v_samples(N);

    std::generate_n(
        n_samples.begin(), N,
        boost::bind(
            static_cast<double(*)(const Normal_distribution&)>(sample), norm));

    std::generate_n(
        b_samples.begin(), N,
        boost::bind(
            static_cast<double(*)(const Beta_distribution&)>(sample), beta));

    std::generate_n(
        g_samples.begin(), N,
        boost::bind(
            static_cast<double(*)(const Gamma_distribution&)>(sample), gamma));

    //std::generate_n(
        //v_samples.begin(), N,
        //boost::bind(
            //static_cast<Vector3(*)(const Vmf_distribution&)>(sample), vmf));
    sample(vmf, N, v_samples.begin());

    Normal_distribution norm2 = mle_normal(n_samples.begin(), n_samples.end());
    Beta_distribution beta2 = mle_beta(b_samples.begin(), b_samples.end());
    Gamma_distribution gamma2 = mle_gamma(g_samples.begin(), g_samples.end());
    Vmf_distribution vmf2 = mle_vmf<3>(v_samples.begin(), v_samples.end());

    std::cout << "Original normal: N("
              << norm.mean() << ", " << norm.standard_deviation()
              << ")" << std::endl;
    std::cout << "Estimated normal: N("
              << norm2.mean() << ", " << norm2.standard_deviation()
              << ")" << std::endl << std::endl;

    std::cout << "Original beta: B("
              << beta.alpha() << ", " << beta.beta()
              << ")" << std::endl;
    std::cout << "Estimated beta: B("
              << beta2.alpha() << ", " << beta2.beta()
              << ")" << std::endl << std::endl;

    std::cout << "Original gamma: G("
              << gamma.scale() << ", " << gamma.shape()
              << ")" << std::endl;
    std::cout << "Estimated gamma: G("
              << gamma2.scale() << ", " << gamma2.shape()
              << ")" << std::endl << std::endl;

    std::cout << "Original VMF: V("
              << "(" << vmf.mu() << "), " << vmf.kappa()
              << ")" << std::endl;
    std::cout << "Estimated VMF: V("
              << "(" << vmf2.mu() << "), " << vmf2.kappa()
              << ")" << std::endl << std::endl;

    return EXIT_SUCCESS;
}

