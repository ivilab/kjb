
#include <blr_cpp/bayesian_linear_regression.h>
#include <prob_cpp/prob_distribution.h>
#include <prob_cpp/prob_sample.h>
#include <gp_cpp/gp_base.h>
#include <gp_cpp/gp_covariance.h>
#include <l_cpp/l_test.h>

#include <iostream>
#include <fstream>
#include <iterator>
#include <boost/random.hpp>
#include <boost/format.hpp>

using namespace kjb;

std::vector<size_t> parse_lists(const std::string& id_fname)
{
    std::vector<size_t> lists;
    std::ifstream ifs(id_fname.c_str());
    IFTD(!ifs.fail(), IO_error, "Can't open file %s", (id_fname.c_str()));
    std::string line;
    while(std::getline(ifs, line))
    {
        if(!line.empty())
            lists.push_back(boost::lexical_cast<size_t>(line));
    }
    return lists;
}

void test_blr
(
    size_t index,
    const Vector& mu, 
    const Matrix& V, 
    double a, 
    double b, 
    const std::vector<Matrix>& y,
    const Vector& B,
    double variance,
    const Vector& bmiaves,
    const std::string& type = "inverse-gamma"
)
{
    const size_t D = B.size();
    size_t N = 0;
    size_t n = y.size();
    for(size_t i = 0; i < n; i++)
    {
        N += y[i].get_col(index).size();
    }
    std::cout << " N: " << N << std::endl;

    std::vector<Matrix> k(n);
    double scale = 100.0;
    double signal_sigma = 1.0;
    gp::Squared_exponential covariance(scale, signal_sigma);
    Matrix X_t_X(D, D, 0.0);
    double y_t_y = 0.0;
    Matrix X_t(D, N); 
    size_t counter = 0;
    Vector y_all;
    y_all.reserve(N);
    for(int i = 0; i < n; i++)
    {
        size_t T = y[i].get_num_rows();
        gp::Inputs inputs = gp::make_inputs(1, T);
        k[i] = gp::apply_cf(covariance, inputs.begin(), inputs.end());
        Matrix x((int)T, (int)D, 1.0); 
        if(D > 1)
        {
            for(size_t j = 0; j < T; j++)
            {
                x(j, 1) = bmiaves[i];
            }
        }
        Matrix k_inv = k[i].inverse();
        Matrix temp = x.transpose() * k_inv;
        X_t_X += temp * x;
        y_t_y += dot(y[i].get_col(index) * k_inv, y[i].get_col(index));

        size_t num_rows = y[i].get_num_rows();
        assert(temp.get_num_cols() == T);
        for(int n = 0; n < temp.get_num_cols(); n++)
        {
            for(int m = 0; m < temp.get_num_rows(); m++)
            {
                X_t(m, counter + n) = temp(m, n);
            }
            y_all.push_back(y[i](n, index));
        }
        counter += temp.get_num_cols();
    }

    /*Matrix K_inv = K.inverse();
    Matrix X((int)N, 1, 1.0);
    Matrix X_t = X.transpose() * K_inv;
    Matrix X_t_X = X_t * X;*/

    Bayesian_linear_regression blr(mu, V, a, b, type, Matrix(), X_t, X_t_X);
    //double y_t_y = dot(y_all * K_inv, y_all);
    blr.compute_posterior(y_all, y_t_y);

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

    Vector zero((int)D, 0.0);
    Vector sample_mean = std::accumulate(samples.begin(), samples.end(),
                                         zero);
    double sample_sigma = std::accumulate(sigmas.begin(), sigmas.end(), 0.0);
    sample_mean /= num_samples;
    sample_sigma /= num_samples;

    Vector diff = B - sample_mean;
    std::cout << B << " vs: " << sample_mean << " : " << diff << std::endl;
    std::cout << variance <<" vs: " << sample_sigma << " : " 
              << variance - sample_sigma << std::endl;
    Matrix X(N, D, 1.0);
    counter = 0;
    for(size_t i = 0; i < n; i++)
    {
        size_t T = y[i].get_num_rows();
        for(size_t j = 0; j  < T; j++)
        {
            X(counter++, 1) = bmiaves[i];
        }
    }

    Vector y_pred = X * sample_mean;
    double error = 0.0;
    for(int i = 0; i < N; i++)
    {
        error += std::pow((y_all[i] - y_pred[i]), 2);
        std::cout << X(i, 1) <<  " " << y_all[i] << " " << y_pred[i] << std::endl;
    }
    error /= num_samples;
    std::cout << error << " " << sample_sigma << std::endl; 
    //TEST_TRUE(fabs((error - sample_sigma)/sample_sigma) < 0.01);
}

int main(int argc, char** argv)
{
    if(argc < 6)
    {
        std::cout << "Usuage: " << argv[0] << " " << " y_dir list-file coef variance bmiaves\n";
        return EXIT_SUCCESS;
    }
    std::string ydir = argv[1];
    std::string list_fp = argv[2];
    std::string coef_fname= argv[3];
    std::string sigma_fname = argv[4];
    std::string bmiave_fname = argv[5];
    Matrix coefs(coef_fname);
    Vector variances(sigma_fname);
    Vector bmiaves(bmiave_fname);

    boost::format fp_fmt(ydir + "/%04d/com_params.txt");
    std::vector<size_t> ids = parse_lists(list_fp);
    std::vector<Matrix> data(ids.size());
    for(size_t i = 0; i < ids.size(); i++)
    {
        std::string fp = (fp_fmt % ids[i]).str();
        data[i] = Matrix(fp);
    }
    size_t num_params = data[0].get_num_cols();


    std::vector<Vector> mus(num_params);
    std::vector<Matrix> Sigmas(num_params);
    mus[0] = Vector(-0.09, 0.0);
    mus[1] = Vector(0.01, 0.01);
    mus[2] = Vector(0.0, 0.0);
    mus[3] = mus[0];
    mus[4] = mus[1];
    mus[5] = mus[2];
    Sigmas[0] = create_diagonal_matrix(Vector(0.01, 0.01));
    Sigmas[1] = create_diagonal_matrix(Vector(0.01, 0.01));
    Sigmas[2] = create_diagonal_matrix(Vector(0.01, 0.01));
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
    for(size_t i = 0; i < num_params; i++)
    {
        std::cout << "testing for param " << i << std::endl;
        Vector coef = coefs.get_row(i);
        double variance = variances(i);
        Matrix V = Sigmas[i].inverse();
        test_blr(i, mus[i], V, a, b, data, coef, variance, bmiaves);
        std::cout << "XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX\n";
    }

    return EXIT_SUCCESS;
}

