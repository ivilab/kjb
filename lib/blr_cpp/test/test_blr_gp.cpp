/* =========================================================================== *
   |
   |  Copyright (c) 1994-2010 by Kobus Barnard (author)
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
   |  Author:  Jinyan Guan
 * =========================================================================== */

/* $Id$ */

#include <blr_cpp/bayesian_linear_regression.h>
#include <prob_cpp/prob_distribution.h>
#include <prob_cpp/prob_sample.h>
#include <l_cpp/l_test.h>
#include <gp_cpp/gp_base.h>
#include <gp_cpp/gp_covariance.h>
#include <m_cpp/m_matrix.h>
#include <m_cpp/m_vector.h>

using namespace kjb;

void test_blr
(
    int D, 
    int n, 
    int T,
    const Vector& mu, 
    const Matrix& V, 
    double a, 
    double b, 
    const Matrix& X,
    const std::string& type
)
{
    int N = n * T;
    std::cout << "a: " << a << " b: " << b << std::endl;
    std::pair<Vector, double> B;
    if(get_prior_type(type) == INVERSE_GAMMA)
    {
        Normal_inverse_gamma_distribution prior(mu, V, a, b);
        B = sample(prior);
    }
    else
    {
        assert(get_prior_type(type) == INVERSE_CHI_SQUARED);
        Normal_inverse_chi_squared_distribution prior(mu, V, a, b);
        B = sample(prior);
    }

    // construct K 
    double var_o = B.second;
    Vector mu_o = B.first;
    std::vector<Matrix> k(n);
    double scale = 100.0;
    double signal_sigma = 1.0;
    gp::Squared_exponential covariance(scale, signal_sigma);
    gp::Inputs inputs = gp::make_inputs(1, T);
    for(int i = 0; i < n; i++)
    {
        k[i] = gp::apply_cf(covariance, inputs.begin(), inputs.end());
    }

    Matrix K(N, N, 0.0);
    size_t count = 0;
    for(int i = 0; i < n; i++)
    {
        for(int t = 0; t < T; t++)
        {
            for(int tt = 0; tt < T; tt++)
            {
                K(count + t, count + tt) = k[i](t, tt);
            }
        }
        count += T;
    }

    Vector mean = X * B.first;

    // add noise
    //MV_gaussian_distribution noise_dist(Vector(N, 0.0), var_o * K);
    MV_gaussian_distribution noise_dist(mean, var_o * K);
    Vector y = sample(noise_dist);
    Matrix K_inv = K.inverse();

    Matrix X_t = X.transpose() * K_inv;
    Matrix X_t_X = X_t * X;

    Bayesian_linear_regression blr(mu, V, a, b, type, Matrix(), X_t, X_t_X);
    double y_t_y = dot(y * K_inv, y);
    blr.compute_posterior(y, y_t_y);

    Blr_param param = blr.get_posterior();
    size_t num_samples = 100;
    std::vector<Vector> samples(num_samples);
    std::vector<double> sigmas(num_samples); 

    if(get_prior_type(type) == INVERSE_GAMMA)
    {
        Normal_inverse_gamma_distribution dist(
                param.mu, param.V, param.var_a, param.var_b);
        size_t num_samples = 100;

        for(size_t i = 0; i < num_samples; i++)
        {
            std::pair<Vector, double> x = sample(dist);
            samples[i] = x.first;
            sigmas[i] = x.second;
        }
    }
    else
    {
        Normal_inverse_chi_squared_distribution dist(
                param.mu, param.V, param.var_a, param.var_b);
        size_t num_samples = 100;

        for(size_t i = 0; i < num_samples; i++)
        {
            std::pair<Vector, double> x = sample(dist);
            samples[i] = x.first;
            sigmas[i] = x.second;
        }
    }

    Vector zero(D, 0.0);
    Vector sample_mean = std::accumulate(samples.begin(), samples.end(),
                                         zero);
    double sample_sigma = std::accumulate(sigmas.begin(), sigmas.end(), 0.0);
    sample_mean /= num_samples;
    sample_sigma /= num_samples;

    Vector diff = B.first - sample_mean;
    std::cout << "mean: " <<  B.first << " vs: " << sample_mean << " : " << diff << std::endl;
    std::cout << "variance: " << B.second <<" vs: " << sample_sigma << " : " 
              << B.second - sample_sigma << std::endl;
    Vector y_pred = X * sample_mean;
    double error = 0.0;
    for(int i = 0; i < N; i++)
    {
        error += std::pow((y[i] - y_pred[i]), 2);
        //std::cout << X(i, 1) <<  " " << y[i] << " " << y_pred[i] << std::endl;
        std::cout << X(i, 0) <<  " " << y[i] << " " << y_pred[i] << std::endl;
    }
    error /= N;
    std::cout << error << " " << sample_sigma << std::endl; 
    //TEST_TRUE(fabs((error - sample_sigma)/sample_sigma) < 0.01);
}

int main(int argc, char** argv)
{
    //const int D = 2;
    const int D = 1;
    const int n = 45;
    const int T = 59;
    Vector mu(D, 0.0);
    Matrix Sigma = create_diagonal_matrix(D, 10.0);
    Matrix V = Sigma.inverse();
    const double a = 200.0;
    const double b = 0.0001;
    int N = n * T;
    Matrix X(N, D); 
    int count = 0;
    // constrcut omega
    for(int i = 0; i < n; i++)
    {
        double coef = 100.0 * kjb_c::kjb_rand();
        for(int j = 0; j < T; j++)
        {
            X(count, 0) = 1.0;
            //X(count, 1) = coef;
            count++;
        }
    }
    //std::string ctype("inverse-chi-squared");
    //test_blr(D, n, T, mu, V, a, b, X, ctype);

    std::string type("inverse-gamma");
    test_blr(D, n, T, mu, V, a/2.0, (a*b)/2.0, X, type);
    return EXIT_SUCCESS;
}
