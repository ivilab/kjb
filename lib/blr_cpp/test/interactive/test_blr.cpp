
#include <blr_cpp/bayesian_linear_regression.h>
#include <prob_cpp/prob_distribution.h>
#include <prob_cpp/prob_sample.h>
#include <l_cpp/l_test.h>
#include <boost/random.hpp>

using namespace kjb;

void test_blr
(
    int D, 
    int N, 
    const Vector& mu, 
    const Matrix& V, 
    double a, 
    double b, 
    const Matrix& X,
    const Vector& y,
    const Vector& B,
    double variance,
    const std::string& type = "inverse-gamma"
)
{
    Bayesian_linear_regression blr(mu, V, a, b, type, X);
    blr.compute_posterior(y);

    Blr_param param = blr.get_posterior();
    size_t num_samples = 100000;
    std::vector<Vector> samples(num_samples);
    std::vector<double> sigmas(num_samples); 

    if(get_prior_type(type) == INVERSE_GAMMA)
    {
        Normal_inverse_gamma_distribution dist(
                param.mu, param.V, param.var_a, param.var_b);

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

    Vector diff = B - sample_mean;
    std::cout << B << " vs: " << sample_mean << " : " << diff << std::endl;
    std::cout << variance <<" vs: " << sample_sigma << " : " 
              << variance - sample_sigma << std::endl;
    Vector y_pred = X * sample_mean;
    double error = 0.0;
    for(int i = 0; i < N; i++)
    {
        error += std::pow((y[i] - y_pred[i]), 2);
        std::cout << X(i, 1) << " " <<  X(i, 2) <<  " " << y[i] << " " << y_pred[i] << std::endl;
    }
    error /= num_samples;
    std::cout << error << " " << sample_sigma << std::endl; 
    //TEST_TRUE(fabs((error - sample_sigma)/sample_sigma) < 0.01);
}

int main(int argc, char** argv)
{
    if(argc < 5)
    {
        std::cout << "Usuage: " << argv[0] << " " << " y_file x_file coef variance \n";
        return EXIT_SUCCESS;
    }
    std::string fname = argv[1];
    std::string xfname = argv[2];
    std::string coef_fname= argv[3];
    std::string sigma_fname = argv[4];
    Matrix data(fname);
    Matrix X(xfname);
    Matrix coefs(coef_fname);
    Vector variances(sigma_fname);
    size_t num_params = data.get_num_cols();
    int D = X.get_num_cols();
    std::cout << "D: " << D << " X.get_num_cols(): " << X.get_num_cols() << std::endl;
    std::vector<Vector> mus(num_params);
    std::vector<Matrix> Sigmas(num_params);
    mus[0] = Vector(-0.01, -0.01, -0.01);
    //mus[0] = Vector(-0.09, 0.0);
    mus[1] = Vector(0.0, 0.0, 0.0);
    mus[2] = Vector(0.0, 0.01, 0.01);
    mus[3] = mus[0];
    mus[4] = mus[1];
    mus[5] = mus[2];
    Sigmas[0] = create_diagonal_matrix(Vector(0.1, 0.1, 0.1));
    Sigmas[1] = create_diagonal_matrix(Vector(0.01, 0.01, 0.01));
    Sigmas[2] = create_diagonal_matrix(Vector(0.1, 0.1, 0.1));
    Sigmas[3] = Sigmas[0];
    Sigmas[4] = Sigmas[1];
    Sigmas[5] = Sigmas[2];
    const double a = 100.0;
    const double b = 0.01;
    /*const double a = 100.0;
    const double b = 0.01;
    mus[0] = Vector(-0.02, 0.0001, 0.0001);
    mus[1] = Vector(0.0, 0.001, 0.001);
    mus[2] = mus[1];
    mus[3] = mus[0];
    mus[4] = mus[1];
    mus[5] = mus[2];
    Sigmas[0] = create_diagonal_matrix(Vector(0.01, 0.001, 0.001));
    Sigmas[1] = create_diagonal_matrix(Vector(0.001, 0.001, 0.001));
    Sigmas[2] = create_diagonal_matrix(Vector(0.001, 0.001, 0.001));
    Sigmas[3] = Sigmas[0];
    Sigmas[4] = Sigmas[1];
    Sigmas[5] = Sigmas[2];*/
    int N = X.get_num_rows();
    for(size_t i = 0; i < num_params; i++)
    {
        std::cout << "testing for param " << i << std::endl;
        Vector y = data.get_col(i);
        Vector coef = coefs.get_row(i);
        double variance = variances(i);
        Matrix V = Sigmas[i];
        test_blr(D, N, mus[i], V, a, b, X, y, coef, variance);
        std::cout << "XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX\n";
    }

    return EXIT_SUCCESS;
}

