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

#include <vector>
#include <fstream>
#include <iostream>

//#include <boost/format.hpp>

using namespace std;
using namespace kjb;

int main(int argc, char** argv)
{
    if(argc != 2)
    {
        std::cout << "Usage: " << argv[0] << " data-file\n";
        return EXIT_SUCCESS;
    }
    // generate some samples from Mixture of Gaussians 
    size_t K = 2;
    std::vector<Vector> samples;
    Matrix X(argv[1]);
    size_t N = X.get_num_rows();
    for(size_t i = 0; i < X.get_num_rows(); i++)
    {
        Vector m = X.get_row(i);
        //samples.push_back(Vector(m.begin(), m.begin() + 2));
        samples.push_back(m);
    }

    int D = samples[0].size();
    Vector prior_mean = std::accumulate(samples.begin(), 
                                        samples.end(), 
                                        Vector(D, 0.0))/N;
    double mu_scale = 4.0;
    double covar_scale = 0.7;
    //double kappa_o = pow(covar_scale, 2) / pow(mu_scale, 2);
    double kappa_o = 0.0001;
    //double kappa_o = 0.001;
    std::cout << kappa_o << std::endl;
    double nu_o = D + 3;
    //double nu_o = D + 2;
    double lambda = 100.0;
    Vector mu_o = prior_mean;
    Matrix S_x(D, D, 0.0); 
    for(size_t i = 0; i < N; i++)
    {
        Vector temp = samples[i] - prior_mean;
        S_x += outer_product(temp, temp);
    }
    S_x /= N;
    Matrix S_o(D, D, 0.0);
    for(size_t i = 0; i < D; i++)
    {
        S_o(i, i) = S_x(i, i);
    }
    mu_o = Vector(D, 0.0);
    mu_o[0] = -0.01;
    mu_o[3] = -0.01;
    std::cout << S_o << std::endl;
    //S_o = create_diagonal_matrix((int)D, 0.1);

    //bool collapsed = true;
    bool collapsed = false;
    Gaussian_mixtures<Vector> gmc(N, D, lambda, mu_o, kappa_o, S_o, nu_o); 
    Gaussian_mixture_gibbs_step<Vector> gibbs_step(K, gmc, collapsed);
    std::vector<int> init_Zs = gmc.assignments();
    gmc.update_data_priors(samples);
    gmc.update_cluster_cache(samples);

    const size_t num_iters = 1e2;
    double lp = 0.0;
    lp = gmc.log_marginal(samples);
    string fp = "ll.txt";
    std::ofstream ofs(fp.c_str());
    double best_lp = -DBL_MAX;
    std::vector<Vector> dist_means;
    std::vector<Matrix> dist_variances;
    Vector dist_weights;
    size_t est_samples = 1;
    std::vector<int> best_Zs;
    for(size_t i = 0; i < num_iters; i++)
    {
        gibbs_step(samples, lp);
        if(lp > best_lp)
        {
            best_Zs = gmc.assignments();
            best_lp = lp;
            gmc.estimate_dist_params(est_samples, dist_means, dist_variances);
            gmc.estimate_mixture_weights(est_samples, dist_weights);
        }
        ofs << best_lp << std::endl;
    }

    const std::vector<int>& cZs = gmc.assignments();

    std::ofstream cofs(string("clusters.txt").c_str());
    for(size_t i = 0; i < N; i++)
    {
        cofs << samples[i] << " " << best_Zs[i] << " " << cZs[i] << " " << init_Zs[i] << std::endl;
    }

    assert(dist_means.size() == dist_variances.size());
    assert(dist_means.size() == dist_weights.size());
    ofstream vofs(string("means.txt").c_str());
    ofstream mofs(string("variances.txt").c_str());
    for(size_t k = 0; k < dist_means.size(); k++)
    {
        std::string mean_fp;
        vofs << dist_means[k] << std::endl;
        mofs << dist_variances[k] << std::endl;
        //std::cout << weights[k] << std::endl;
    }

    // write couple ids in regime as group (For Debug purposes)
    boost::format cluster_fp("cluster_%d.txt");
    boost::format cluster_param_fp("cluster_params_%d.txt");
    for(size_t i = 0; i < best_Zs.size(); i++)
    {
        int regime = best_Zs[i];
        std::string fp = (cluster_fp % regime).str();
        std::string param_fp = (cluster_param_fp % regime).str();
        std::ofstream ofs(fp.c_str(), std::iostream::app);
        std::ofstream param_ofs(param_fp.c_str(), std::iostream::app);
        ofs << i << std::endl;
        param_ofs << samples[i] << std::endl;

    }
    
    return EXIT_SUCCESS;
}

