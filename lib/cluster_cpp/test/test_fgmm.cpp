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

/* $Id: test_gmm.cpp 20249 2016-01-21 16:06:01Z jguan1 $ */

#include <m_cpp/m_vector.h>
#include <m_cpp/m_matrix.h>
#include <prob_cpp/prob_distribution.h>
#include <cluster_cpp/cluster_gaussian_mixtures.h>
#include <cluster_cpp/cluster_gaussian_mixtures_gibbs.h>
#include <l_cpp/l_test.h>
#include <iostream>
#include <fstream>
#include <boost/random.hpp>

#include <vector>

using namespace std;
using namespace kjb;

int main(int argc, char** argv)
{
    // generate some samples from Mixture of Gaussians 
    int D = 2;
    Vector prior_mean(D, 0.0);
    double mu_scale = 4.0;
    double covar_scale = 0.7;
    double kappa_o = pow(covar_scale, 2) / pow(mu_scale, 2);
    double nu_o = D + 3;
    double lambda = 1.0;
    Vector mu_o(D, 0.0);
    Matrix S_o = create_diagonal_matrix(D, 100.0); 
                                        
    const size_t N = 200;
    Gaussian_mixtures<Vector> gmc(N, D, lambda, mu_o, kappa_o, S_o, nu_o); 
    std::vector<Vector> means;
    std::vector<Matrix> variances;
    Vector m1(D, 100.0);
    m1[1] = -100.0;
    Matrix s1 = create_diagonal_matrix(D, 100.0);
    means.push_back(m1);
    variances.push_back(s1);

    Vector m2(D, -100.0);
    Matrix s2 = create_diagonal_matrix(D, D);
    s2 *= 200.0;
    means.push_back(m2);
    variances.push_back(s2);

    Vector m3(D, 100.0);
    Matrix s3 = create_diagonal_matrix(D, 100.0);
    means.push_back(m3);
    variances.push_back(s3);

    Vector m4(D, -100.0);
    m4[1] = 100.0;
    Matrix s4 = create_diagonal_matrix(D, 200.0);
    means.push_back(m4);
    variances.push_back(s4);

    Vector m5(D, 0.0);
    Matrix s5 = create_diagonal_matrix(D, 200.0);
    means.push_back(m5);
    variances.push_back(s5);
    //gmc.generate_mixture_distributions_from_prior(K);
    gmc.set_mixture_distributions(means, variances);
    size_t K = means.size();
    gmc.randomly_assign_groups(K);
    std::vector<Vector> samples;
    std::vector<int> assignments;
    gmc.generate_cluster_samples(N, samples, assignments);

    //std::ofstream sofs(string("samples.txt").c_str());

    //bool collapsed = false;
    bool collapsed = true;
    bool fixed_dist_params = false;
    //bool fixed_dist_params = true;
    size_t num_clusters = means.size();
    //size_t num_clusters = 20;
    Gaussian_mixtures<Vector> gmc_test(N, D, lambda, mu_o, kappa_o, S_o, nu_o); 
    Gaussian_mixture_gibbs_step<Vector> gibbs_step
        (num_clusters, gmc_test, collapsed, fixed_dist_params);
    gmc_test.update_data_priors(samples);
    gmc_test.update_cluster_cache(samples);
    if(fixed_dist_params)
    {
        gmc_test.set_mixture_distributions(means, variances);
    }
    double lp = 0.0; 
    //gibbs_step(samples, lp);
    lp = gmc_test.log_marginal(samples);
    //std::cout << lp << std::endl;
    const size_t num_iters = 1e2;
    string fp = "ll.txt";
    std::ofstream ofs(fp.c_str());
    std::vector<int> best_Zs;
    double best_lp = -DBL_MAX;
    std::vector<Vector> dist_means;
    std::vector<Matrix> dist_variances;
    Vector dist_weights;
    for(size_t i = 0; i < num_iters; i++)
    {
        gibbs_step(samples, lp);
        if(!collapsed)
        std::cout << lp << " " << gmc_test.log_prob_given_cluster(samples) << " " << gmc_test.log_hyper_prior() << std::endl;
        if(lp > best_lp)
        {
            best_Zs = gmc_test.assignments();
            best_lp = lp;
        }
        ofs << best_lp << std::endl;
    }

    const std::vector<int>& Zs = gmc_test.assignments();

    if(collapsed)
    {
        gibbs_step.estimate_dist_params();
    }

    std::vector<std::vector<double> > resp = gmc_test.get_responsibilities(samples);

    std::ofstream cofs(string("clusters.txt").c_str());
    for(size_t i = 0; i < N; i++)
    {
        cofs << samples[i] << " " << best_Zs[i] << " " << Zs[i] << " " << assignments[i];
        /*for(size_t k = 0; k < K; k++)
        {
            cofs << resp[i][k] << " ";
        }*/
        cofs << std::endl;
    }


    assert(dist_means.size() == dist_variances.size());
    assert(dist_means.size() == dist_weights.size());
    ofstream vofs(string("means.txt").c_str());
    ofstream mofs(string("variances.txt").c_str());
    /*if(fixed_dist_params)
    {
        const Vector& weights = gmc_test.get_mixture_weights();
        std::cout << "mixture weights: " << weights << std::endl; 
    }
    else
    {
        std::cout << "mixture weights: " << dist_weights << std::endl; 
    }*/
    for(size_t k = 0; k < dist_means.size(); k++)
    {
        std::string mean_fp;
        vofs << dist_means[k] << std::endl;
        mofs << dist_variances[k] << std::endl;
        //std::cout << weights[k] << std::endl;
    }

    if(fixed_dist_params)
    {
        int diff = 0;
        for(size_t i = 0; i < N; i++)
        {
            diff += fabs(best_Zs[i] - assignments[i]);
        }
        TEST_TRUE(diff < N * 0.01);
    }

    // check the difference between assignments
    return EXIT_SUCCESS;
}

