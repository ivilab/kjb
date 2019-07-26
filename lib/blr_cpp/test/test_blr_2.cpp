
#include <blr_cpp/bayesian_linear_regression.h>
#include <prob_cpp/prob_distribution.h>
#include <prob_cpp/prob_sample.h>
#include <l_cpp/l_test.h>

using namespace kjb;
using namespace std;

bool equal(const Blr_param& b1, const Blr_param& b2)
{
    return vector_distance(b1.mu, b2.mu) < FLT_EPSILON && 
           fabs(det(b1.V) - det(b2.V)) < FLT_EPSILON &&
           fabs(b1.var_a - b2.var_a) < FLT_EPSILON &&
           fabs(b1.var_b - b2.var_b) < FLT_EPSILON &&
           vector_distance(b1.V_mu, b2.V_mu) < FLT_EPSILON &&
           fabs(b1.mu_V_mu - b2.mu_V_mu) < FLT_EPSILON;
}

void test_blr
(
    int D, 
    int N, 
    const Vector& mu, 
    const Matrix& V, 
    double a, 
    double b, 
    const Matrix& X,
    const std::string& type
)
{
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

    Vector y = X * B.first;

    // add noise
    Normal_distribution noise_dist(0.0, sqrt(B.second));
    for(int i = 0; i < N; i++)
    {
        y[i] += sample(noise_dist);
    }

    Bayesian_linear_regression blr(mu, V, a, b, type, X);
    blr.compute_posterior(y);

    Blr_param param = blr.get_posterior();

    size_t num_rows = X.get_num_rows();
    Vector row = X.get_row(num_rows - 1);

    // remove the last item
    blr.remove_input_output(row, y[num_rows - 1]);
    // add in the last item
    blr.add_input_output(row, y[num_rows-1]);
    blr.update_posterior();
    Blr_param new_param = blr.get_posterior();
    TEST_TRUE(equal(param, new_param));

    row = X.get_row(1);
    blr.remove_input_output(row, y[1]);
    blr.add_input_output(row, y[1]);
    blr.update_posterior();
    new_param = blr.get_posterior();
    TEST_TRUE(equal(param, new_param));

    row = X.get_row(4);
    blr.remove_input_output(row, y[4]);
    blr.add_input_output(row, y[4]);
    blr.update_posterior();
    new_param = blr.get_posterior();

    TEST_TRUE(equal(param, new_param));

    size_t num_samples = 100;
    std::vector<Vector> samples(num_samples);
    std::vector<double> sigmas(num_samples); 

    for(size_t i = 0; i < num_samples; i++)
    {
        std::pair<Vector, double> x = blr.get_sample_from_posterior();
        samples[i] = x.first;
        sigmas[i] = x.second;
    }


    Vector zero(D, 0.0);
    Vector sample_mean = std::accumulate(samples.begin(), samples.end(),
                                         zero);
    double sample_sigma = std::accumulate(sigmas.begin(), sigmas.end(), 0.0);
    sample_mean /= num_samples;
    sample_sigma /= num_samples;

    Vector diff = B.first - sample_mean;
    //std::cout << B.first << " vs: " << sample_mean << " : " << diff << std::endl;
    //std::cout << B.second <<" vs: " << sample_sigma << " : " 
              //<< B.second - sample_sigma << std::endl;
    Vector y_pred = X * sample_mean;
    double error = 0.0;
    for(int i = 0; i < N; i++)
    {
        error += std::pow((y[i] - y_pred[i]), 2);
        //std::cout << X(i, 1) <<  " " << y[i] << " " << y_pred[i] << std::endl;
    }
    error /= num_samples;
    std::cout << error << " " << sample_sigma << std::endl; 
    //TEST_TRUE(fabs((error - sample_sigma)/sample_sigma) < 0.01);
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

    std::string type("inverse-gamma");
    test_blr(D, N, mu, V, a/2.0, (a*b)/2.0, X, type);

    std::string ctype("inverse-chi-squared");
    test_blr(D, N, mu, V, a, b, X, ctype);

    return EXIT_SUCCESS;
}
