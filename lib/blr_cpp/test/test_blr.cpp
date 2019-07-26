
#include <blr_cpp/bayesian_linear_regression.h>
#include <prob_cpp/prob_distribution.h>
#include <prob_cpp/prob_sample.h>
#include <l_cpp/l_test.h>

#include <boost/random.hpp>

using namespace kjb;
using namespace std;

void test_blr
(
    int D, 
    int N, 
    const Vector& mu, 
    const Matrix& V, 
    double a, 
    double b, 
    const Matrix& X,
    const string& type
)
{
    pair<Vector, double> B;
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

    Vector y = X * B.first;

    // add noise
    Normal_distribution noise_dist(0.0, sqrt(B.second));
    for(int i = 0; i < N; i++)
    {
        y[i] += sample(noise_dist);
    }

    Bayesian_linear_regression blr(mu, V, a, b, type, X);
    blr.compute_posterior(y);

    size_t num_samples = 100;
    vector<Vector> samples(num_samples);
    vector<double> sigmas(num_samples); 

    for(size_t i = 0; i < num_samples; i++)
    {
        pair<Vector, double> x = blr.get_sample_from_posterior();
        samples[i] = x.first;
        sigmas[i] = x.second;
    }

    Vector zero(D, 0.0);
    Vector sample_mean = accumulate(samples.begin(), samples.end(),
                                         zero);
    double sample_sigma = accumulate(sigmas.begin(), sigmas.end(), 0.0);
    sample_mean /= num_samples;
    sample_sigma /= num_samples;

    Vector diff = B.first - sample_mean;
    //cout << "mean: " << B.first << " vs: " << sample_mean << " : " << diff << endl;
    double sigma_diff = B.second - sample_sigma;
    //cout << "sigma: " << B.second <<" vs: " << sample_sigma << " : " << sigma_diff << endl << endl;
    Vector y_pred = X * sample_mean;
    double error = 0.0;
    for(int i = 0; i < N; i++)
    {
        error += pow((y[i] - y_pred[i]), 2);
        //cout << X(i, 1) <<  " " << y[i] << " " << y_pred[i] << endl;
    }
    error /= num_samples;
    //cout << error << " " << sample_sigma << endl; 
    TEST_TRUE(fabs((error - sample_sigma)/sample_sigma) < 0.1);
}

int main(int argc, char** argv)
{
    const int D = 2;
    const int N = 100;
    Vector mu(D, 0.0);
    Matrix Sigma = create_diagonal_matrix(D, 10.0);
    Matrix V = Sigma.inverse();
    const double a = 1.0;
    const double b = 1.0;
    Matrix X = 100*create_random_matrix(N, D);
    X.set_col(0, Vector(N, 1.0));

    string type("inverse-gamma");
    test_blr(D, N, mu, V, a/2.0, (a*b)/2.0, X, type);

    string ctype("inverse-chi-squared");
    test_blr(D, N, mu, V, a, b, X, ctype);

    return EXIT_SUCCESS;
}
