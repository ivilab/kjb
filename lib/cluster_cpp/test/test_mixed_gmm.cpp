/* =========================================================================== *
|
|  Copyright (c) 1994-2008 by Kobus Barnard (author).
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
| Authors:
|     Jinyan Guan
|
* =========================================================================== */

/* $Id$ */

#include <m_cpp/m_vector.h>
#include <m_cpp/m_matrix.h>
#include <prob_cpp/prob_distribution.h>
#include <cluster_cpp/cluster_gaussian_mixtures.h>
#include <cluster_cpp/cluster_gaussian_mixtures_gibbs.h>
#include <l_cpp/l_test.h>
#include <iostream>
#include <fstream>
#include <boost/random.hpp>

using namespace kjb;
using namespace std;

// a class with mixture data comes from mixture distribution A and B 
template<class Data>
class Gaussian_mixtures_pred
{
public:
    Gaussian_mixtures_pred
    (
        const Gaussian_mixtures<Data>& x,
        const Gaussian_mixtures<Data>& y
    ) :
        x_(x),
        y_(y)
    {}

    void update_assignments(const std::vector<Data>& x, const std::vector<Data>& y)
    {
        std::vector<int> assigns(x.size());
        for(size_t i = 0; i < x.size(); i++)
        {
            std::vector<double> resps = get_responsibilities(x[i], y[i]);
            //std::copy(resps.begin(), resps.end(), ostream_iterator<double>(std::cout, " "));
            //std::cout << std::endl;
            Categorical_distribution<size_t> dist(resps, 0);
            size_t k = sample(dist);
            assigns[i] = k;
        }
        x_.set_assignments(assigns);
        y_.set_assignments(assigns);
    }

    std::vector<double> get_responsibilities(const Data& data_x, const Data& data_y)
    {
        size_t K = x_.get_num_clusters();
        assert(K == y_.get_num_clusters());
            
        std::vector<double> resps(K, 0.0);
        //double sum = 0.0;
        for(size_t k = 0; k < K; k++)
        {
            double log_p = std::log(x_.get_mixture_weights()[k]) + 
                           log_pdf(x_.get_mixture_dist(k), data_x);
                           log_pdf(y_.get_mixture_dist(k), data_y);
            resps[k] = log_p;
        }
        double sum = log_sum(resps.begin(), resps.end());
        for(size_t k = 0; k < K; k++)
        {
            resps[k] = std::exp(resps[k] - sum);
        }
        return resps;
    }

    std::vector<Data> generate_y_samples(const std::vector<Data>& x_samples)
    {
        std::vector<Data> y_samples(x_samples.size());
        for(size_t i = 0; i < y_samples.size(); i++)
        {
            std::vector<double> resps = x_.get_responsibilities(x_samples[i]);
            Categorical_distribution<size_t> dist(resps, 0);
            size_t k = sample(dist);
            y_samples[i] = sample(y_.get_mixture_dist(k));
        }
        return y_samples;
    }

    std::vector<double> get_responsibilities_x(const Data& data_x)
    {
        return x_.get_responsibilities(data_x);
    }
    std::vector<double> get_responsibilities_y(const Data& data_y)
    {
        return y_.get_responsibilities(data_y);
    }

    Data pred_y_val(const Data& x, const Data& y, bool marg_z)
    {
        std::vector<double> resp = x_.get_responsibilities(x);
        //std::vector<double> resp = get_responsibilities(x, y);
        //std::vector<double> resp = y_.get_responsibilities(y);
        //std::copy(resp.begin(), resp.end(), ostream_iterator<double>(std::cout, " "));
        //std::cout << std::endl;
        Categorical_distribution<size_t> dist(resp, 0);
        const size_t n = 1000;
        std::vector<Data> samples(n);
        std::vector<double>::iterator max_iter =
            std::max_element(resp.begin(), resp.end());
        size_t k = std::distance(resp.begin(), max_iter);

        if(!marg_z)
        {
            //return y_.get_mixture_dist(k).get_mean();
            //std::cout << "max z: " <<  k;
        }
        /*else
        {
            Vector temp(2, 0.0);
            for(size_t j = 0; j < resp.size(); j++)
            {
                temp += resp[j] * y_.get_mixture_dist(k).get_mean();
            }
            return temp/resp.size();
        }*/
        for(size_t i = 0; i < n; i++)
        {
            if(marg_z)
            {
                k = sample(dist);
            }
            samples[i] = sample(y_.get_mixture_dist(k));
        }
        Vector sum = std::accumulate(samples.begin(), samples.end(), 
                     Vector((int)samples.front().size(), 0.0));
        sum /= n;
        return sum;
    } 

    double get_error(const Vector& x, const Vector& pred_val, const Vector& obs_val)
    {
        size_t K = x_.get_num_clusters();
        std::vector<double> resp = x_.get_responsibilities(x);
        return vector_distance(pred_val, obs_val);
        /*double error = 0.0;
        for(size_t k = 0; k < K; k++)
        {
            error += resp[k] * vector_distance(pred_val, obs_val);
        }
        return error;*/
    }

private:
    const Gaussian_mixtures<Data>& x_;
    const Gaussian_mixtures<Data>& y_;
};

int main(int argc, char** argv)
{
    // create the Gaussian_mixture_pred
    // generate some samples from Mixture of Gaussians 
    int D = 1;
    double mu_scale = 4.0;
    double covar_scale = 0.7;
    double kappa_o = pow(covar_scale, 2) / pow(mu_scale, 2);
    double nu_o = D + 3;
    double lambda = 1.0;
    Vector mu_o(D, 0.0);
    Matrix S_o = create_diagonal_matrix(D, 100.0); 
                                        
    const size_t N = 10000;
    Gaussian_mixtures<Vector> x(N, D, lambda, mu_o, kappa_o, S_o, nu_o); 

    std::vector<Vector> means;
    std::vector<Matrix> variances;
    Vector m1(D, 0.0);
    //m1(1) = 0.0;
    //Vector m1(D, -100.0);
    Matrix s1 = create_diagonal_matrix(D, 1.0);
    means.push_back(m1);
    variances.push_back(s1);

    Vector m2(D, 50.0);
    //m2(1) = 0.0;
    //Vector m1(D, -100.0);
    Matrix s2 = create_diagonal_matrix(D, 1.0);
    means.push_back(m2);
    variances.push_back(s2);
    //means.push_back(m1);
    //variances.push_back(s1);
    x.set_mixture_distributions(means, variances);
    x.set_mixture_weights(Vector(2, 0.5));

    std::string x_means("x_means.txt");
    std::ofstream x_means_ofs(x_means.c_str());
    std::copy(means.begin(), means.end(), std::ostream_iterator<Vector>(x_means_ofs, "\n"));

    std::string x_variances("x_variances.txt");
    std::ofstream x_variances_ofs(x_variances.c_str());
    std::copy(variances.begin(), variances.end(), std::ostream_iterator<Matrix>(x_variances_ofs, "\n"));
   
    //D = 2;
    nu_o = D + 3;
    lambda = 1.0;
    Vector mu_o_y(D, 40.0);
    Matrix S_o_y = create_diagonal_matrix(D, 100.0); 
    Gaussian_mixtures<Vector> y(N, D, lambda, mu_o_y, kappa_o, S_o_y, nu_o); 
    means.clear();
    variances.clear();

    Vector m3(D, 0.0);
    //m3(1) = 0.0;
    Matrix s3 = create_diagonal_matrix(D, 10.0);
    //s3(0, 1) = 70;
    //s3(1, 0) = 70;
    //means.push_back(m1);
    //variances.push_back(s1);
    means.push_back(m3);
    variances.push_back(s3);

    Vector m4(D, 100.0);
    //m4(1) = 0.0;
    //m4[1] = 100.0;
    Matrix s4 = create_diagonal_matrix(D, 10.0);
    means.push_back(m4);
    variances.push_back(s4);
    //means.push_back(m2);
    //variances.push_back(s2);
    y.set_mixture_distributions(means, variances);
    y.set_mixture_weights(Vector(2, 0.5));

    std::string y_means("y_means.txt");
    std::ofstream y_means_ofs(y_means.c_str());
    std::copy(means.begin(), means.end(), std::ostream_iterator<Vector>(y_means_ofs, "\n"));

    std::string y_variances("y_variances.txt");
    std::ofstream y_variances_ofs(y_variances.c_str());
    std::copy(variances.begin(), variances.end(), std::ostream_iterator<Matrix>(y_variances_ofs, "\n"));

    // generate data_x and data_y
    size_t K = means.size();
    std::vector<Vector> x_samples;
    std::vector<int> assignments;
    x.generate_cluster_samples(N, x_samples, assignments);

    x.set_assignments(assignments);
    y.set_assignments(assignments);

    Gaussian_mixtures_pred<Vector> pred(x, y);
    //std::vector<Vector> y_samples = pred.generate_y_samples(x_samples);
    std::vector<Vector> y_samples(assignments.size());
    for(size_t i = 0; i < N; i++)
    {
        size_t k = assignments[i];
        assert(k < K);
        y_samples[i] = sample(y.get_mixture_dist(k));
    }

    std::string x_fp("x_samples.txt");
    std::string y_fp("y_samples.txt");
    ofstream x_ofs(x_fp.c_str());
    ofstream y_ofs(y_fp.c_str());
    std::copy(x_samples.begin(), x_samples.end(), std::ostream_iterator<Vector>(x_ofs, "\n"));
    std::copy(y_samples.begin(), y_samples.end(), std::ostream_iterator<Vector>(y_ofs, "\n"));
    std::string assign_fp("Z.txt");
    ofstream ofs(assign_fp.c_str());
    std::copy(assignments.begin(), assignments.end(), std::ostream_iterator<size_t>(ofs, "\n"));

    x.randomly_assign_groups(K);
   
    // test cluster assignment sampling 
    for(size_t i = 0; i < 1; i++)
    {
        pred.update_assignments(x_samples, y_samples);
    }
    std::vector<int> inferred_z = y.assignments();
    std::string inferred_fp("inferred_Z.txt");
    ofstream inferred_ofs(inferred_fp.c_str());
    std::copy(inferred_z.begin(), inferred_z.end(), std::ostream_iterator<size_t>(inferred_ofs, "\n"));

    // test predict y using data x with grouth truth cluster distributions
    std::vector<Vector> pred_y(N);
    std::vector<Vector> marg_pred_y(N, Vector(2, 0.0));
    double pred_y_err = 0.0;
    double marg_pred_y_err = 0.0;
    std::string err_fp("errors.txt");
    std::ofstream err_ofs(err_fp.c_str());
    for(size_t i = 0; i < N; i++)
    {
        pred_y[i] = pred.pred_y_val(x_samples[i], y_samples[i], false);
        //std::cout << " : " << inferred_z[i] << std::endl;
        //std::cout << "2: " << inferred_z[i] << " ";
        marg_pred_y[i] = pred.pred_y_val(x_samples[i], y_samples[i], true);
        double temp1 = pred.get_error(x_samples[i], pred_y[i],  y_samples[i]);
        double temp2 = pred.get_error(x_samples[i], marg_pred_y[i], y_samples[i]);
        pred_y_err += temp1;
        marg_pred_y_err += temp2;
        err_ofs << temp1 << " " << temp2 << std::endl;
    }
    std::cout << "pred_y_err: " << pred_y_err / N << std::endl;
    std::cout << "marg_pred_y_err: " << marg_pred_y_err / N << std::endl;
    std::string err_summary_fp("errors_total.txt");
    std::ofstream err_summary_ofs(err_summary_fp.c_str());
    err_summary_ofs << pred_y_err / N << " " << marg_pred_y_err / N << std::endl;

    std::string pred_y_fp("pred_y.txt");
    std::string marg_pred_y_fp("marg_pred_y.txt");
    ofstream pred_y_ofs(pred_y_fp.c_str());
    ofstream marg_pred_y_ofs(marg_pred_y_fp.c_str());
    std::copy(pred_y.begin(), pred_y.end(), std::ostream_iterator<Vector>(pred_y_ofs, "\n"));
    std::copy(marg_pred_y.begin(), marg_pred_y.end(), std::ostream_iterator<Vector>(marg_pred_y_ofs, "\n"));
    // test predict y using data x with learned prior from testing data 
    
    // test sampling the prior distributions
     
    return EXIT_SUCCESS;
}
