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

/* $Id: prior.cpp 22559 2019-06-09 00:02:37Z kobus $ */

#include <prob_cpp/prob_distribution.h>
#include <prob_cpp/prob_pdf.h>
#include <gp_cpp/gp_base.h>
#include <gp_cpp/gp_mean.h>
#include <gp_cpp/gp_covariance.h>
#include <gp_cpp/gp_sample.h>
#include <m_cpp/m_vector.h>

#include <vector>
#include <limits>
#include <utility>

#include <boost/bind.hpp>

#include "dbn_cpp/prior.h"
#include "dbn_cpp/coupled_oscillator.h"
#include "dbn_cpp/linear_state_space.h"

using namespace kjb;
using namespace kjb::ties;

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

double Cluster_prior::operator()(const Lss_set& lsss) const
{
    double pr = 0.0;
    if(lsss.num_groups() > 1)
    {
        // prior of the cluster weights
        // p(pi_1, pi_2, ..., pi_K | lambda)
        Dirichlet_distribution weight_prior(lambdas_);
        Vector weight(lsss.get_group_weights());
        pr = log_pdf(weight_prior, weight);
        // prior of the cluster assignments
        // p(z_1, z_2, ... z_n | pi)
        Categorical_distribution<size_t> cluster_dist(weight, 0);
        for(size_t i = 0; i < lsss.lss_vec().size(); i++)
        {
            pr += log_pdf(cluster_dist, lsss.lss_vec()[i].group_index());
        }
    }
    return pr;
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

void Cluster_prior::sample_cluster_assignments
(
    const Lss_set& lss_set,
    bool from_prior
) const
{
    /*std::cout << "old assignment: ";
    std::copy(num_data_per_cluster_.begin(), num_data_per_cluster_.end(),
            std::ostream_iterator<size_t>(std::cout, " "));
    std::cout << std::endl;*/
    const std::vector<Linear_state_space>& lsss = lss_set.lss_vec();
    for(size_t i = 0; i < lss_set.lss_vec().size(); i++)
    {
        // save the old cluster
        int old_cluster = lsss[i].group_index();

        // get the new cluster
        size_t new_cluster;
        if(from_prior)
        {
            std::vector<double> weights = lss_set.get_group_weights();
            Categorical_distribution<size_t> cluster_dist(weights, 0);
            new_cluster = sample(cluster_dist);
        }
        else
        {
            // compute the responsibility of each clusters
            bool exclude_outcome = false;
            std::vector<double> resps = get_responsibilities(
                                                lss_set.group_params(), 
                                                lss_set.lss_vec()[i],
                                                exclude_outcome); // not incude outcome 
            /*std::copy(resps.begin(), resps.end(), std::ostream_iterator<double>(std::cout, " "));
            std::cout << std::endl;*/
            lss_set.lss_vec()[i].update_group_responsibilities(resps);
            // create a categorical distribution based on responsibilities
            Categorical_distribution<size_t> cluster_dist(resps, 0);
            new_cluster = sample(cluster_dist);
        }
        if(old_cluster != new_cluster)
        {
            lsss[i].group_index() = new_cluster;
            lss_set.update_lss_mean(i);
            lss_set.update_lss_variance(i);
            decrease_cluster_counts(old_cluster);
            increase_cluster_counts(new_cluster);
            //std::cout << i << " " << old_cluster << "->" << new_cluster << std::endl;
        }
    }
    /*std::cout << "new assignment: ";
    std::copy(num_data_per_cluster_.begin(), num_data_per_cluster_.end(),
            std::ostream_iterator<size_t>(std::cout, " "));
    std::cout << std::endl;
    std::cout << "XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX\n";*/
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

void Cluster_prior::sample_cluster_weights
(
    const Lss_set& lss_set
) const
{
    size_t num_groups = lss_set.num_groups();
    assert(num_groups == num_clusters_);
    assert(num_groups == lambdas_.size());
    std::vector<double> new_lambdas(num_groups, 0.0);
    for(size_t i = 0; i < lambdas_.size(); i++)
    {
        new_lambdas[i] = lambdas_[i] + num_data_per_cluster_[i];
    }
    Dirichlet_distribution dist(new_lambdas);
    Vector new_weights = sample(dist);
    // update the lss_set group weight
    for(size_t i = 0; i < num_groups; i++)
    {
        lss_set.group_weight(i) = new_weights(i);
    }
}


/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

Independent_blr_prior::Independent_blr_prior
(
    const Vector& shapes,
    const Vector& scales,
    const std::vector<Vector>& means,
    const std::vector<Matrix>& covariances,
    const std::vector<std::vector<Matrix> >& Xt,
    const std::vector<std::vector<Matrix> >& XtX,
    const std::string& prior_type,
    bool sample_cluster
) :
    blr_priors_(Xt.size()),
    sample_cluster_(sample_cluster) 
{
    size_t num_groups = Xt.size();
    size_t num_params = shapes.size();
    IFT(scales.size() == num_params &&
         means.size() == num_params &&
         covariances.size() == num_params, Illegal_argument, 
            "mean and sigma has different dimension in shared_mean_prior.");
    IFT(Xt.size() == XtX.size() && Xt.size() == num_groups, 
            Illegal_argument, 
            "the number of design matrix is different than the number of clusters");

    for(size_t c = 0; c < num_groups; c++)
    {
        std::vector<Bayesian_linear_regression> blrs;
        for(size_t i = 0; i < scales.size(); i++)
        {
            /*std::cout << "group " << c << " param " << i << " " 
                << Xt[c][i].get_num_cols() 
                << " : " << Xt[c][i].get_num_rows() << " ; " 
                << XtX[c][i].get_num_cols() 
                << " : " << XtX[c][i].get_num_rows() 
                << std::endl;*/
            blrs.push_back(Bayesian_linear_regression(
                                    means[i], 
                                    covariances[i].inverse(), 
                                    shapes[i], 
                                    scales[i], 
                                    prior_type,
                                    Matrix(),
                                    Xt[c][i],
                                    XtX[c][i]));
        }
        blr_priors_[c] = blrs;
    }
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

double Independent_blr_prior::operator()(const Lss_set& lsss) const
{
    // The Gaussian-inverse-gamma prior for each cluster's shared parameter
    // p(alpha, beta, gamma, sigma | B) 
    double pr = 0.0;
    for(size_t c = 0; c < lsss.num_groups(); c++)
    {
        const std::vector<Vector>& pred_coefs = lsss.pred_coefs(c);
        assert(pred_coefs.size() == blr_priors_[c].size());
        const std::vector<double>& variances = lsss.variances(c);
        // for each parameter (including both CLO and polynomial)
        for(size_t i = 0; i < variances.size(); i++)
        {
            assert(i < pred_coefs.size());
            const Vector& mean = pred_coefs[i];
            double variance = variances[i];
            if(std::isinf(variance) || std::isnan(variance) || variance < 0.0)
            {
                return -std::numeric_limits<double>::max();
            }
            else
            {
                double s_p = blr_priors_[c][i].log_prior(mean, variance);

                if(std::isinf(std::fabs(s_p)))
                {
                    s_p = -std::numeric_limits<double>::max();
                    return s_p;
                }
                pr += s_p;
            }
        }
    }
    return pr;
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

std::vector<std::vector<bool> > Independent_blr_prior::compute_posterior
(
    const Lss_set& lsss
) const 
{
    // indexed by [CLUSTER][PARAM]
    size_t num_clusters = blr_priors_.size();
    std::vector<std::vector<bool> > res(num_clusters);

    std::vector<std::vector<Vector> > lss_params = lsss.get_lss_params();
    assert(lss_params.size() == num_clusters);
    // update design matrix if the sampling over clusters
    if(sample_cluster_)
    {
        lsss.init_design_matrix();
        for(size_t c = 0; c < num_clusters; c++)
        {
            const std::vector<Matrix>& X_t = lsss.get_X_t(c);
            const std::vector<Matrix>& X_t_X = lsss.get_X_t_X(c);
            assert(X_t.size() == X_t_X.size());
            assert(X_t.size() == blr_priors_[c].size());
            for(size_t i = 0; i < blr_priors_[c].size(); i++)
            {
                blr_priors_[c][i].update_design_matrix(X_t[i], X_t_X[i]);
            }
        }
    }
    for(size_t c = 0; c < num_clusters; c++)
    {
        size_t num_params = lss_params[c].size();
        res[c] = std::vector<bool>(num_params, false);
        // update the clo params posterior
        for(size_t i = 0; i < num_params; i++)
        {
            if(lsss.allow_drift() && i < lsss.clo_param_size())
            {
                blr_priors_[c][i].X_T() = lsss.get_X_t(c)[i];
                blr_priors_[c][i].X_T_X() = lsss.get_X_t_X(c)[i];
                res[c][i] = blr_priors_[c][i].compute_posterior(
                                                lss_params[c][i], 
                                                lsss.get_y_T_K_inv_y(c)[i]);
            }
            else
            {
                //(!lsss.allow_drift() || i >= lsss.clo_param_size())
                res[c][i] = blr_priors_[c][i].compute_posterior(lss_params[c][i]);
            }

            //std::cout << "compute: " << c << " cluster: " << i << ": " << res[c][i] << std::endl;
        }
        /*while(i < clo_params[c].size())
        {
            // Reset the design matrix if has gp since design matrix 
            // depends on the covariance matrix 
            if(lsss.allow_drift())
            {
                // need to update X_T_K_inv, X_T_K_inv_X, and y_T_K_inv
                blr_priors_[c][i].X_T() = lsss.get_X_t(c)[i];
                blr_priors_[c][i].X_T_X() = lsss.get_X_t_X(c)[i];
                res[c][i] = blr_priors_[c][i].compute_posterior(
                                        clo_params[c][i], 
                                        lsss.get_y_T_K_inv_y(c)[i]);
            }
            else
            {
                res[c][i] = blr_priors_[c][i].compute_posterior(
                                                clo_params[c][i]);
            }
            i++;
        }
        // update the poly params posterior
        while(i < poly_coefs[c].size())
        {
            size_t index = i - clo_params[c].size();
            assert(index < poly_coefs[c].size());
            res[c][i] = blr_priors_[c][i].compute_posterior(poly_coefs[c][index]);
            i++;
        }
        while(i < num_params)
        {
            size_t index = i - clo_params[c].size() - poly_coefs[c].size();
            assert(index < outcomes[c].size());
            res[c][i] = blr_priors_[c][i].compute_posterior(outcomes[c][index]);
            i++;
        }*/
    }
    return res;
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

std::vector<std::pair<Vector, double> > 
Independent_blr_prior::generate_samples_from_cluster_prior
(
    size_t cluster, 
    const Lss_set& lsss
) const
{
    size_t num_oscillators = lsss.num_oscillators();
    size_t use_modal = lsss.use_modal();
    IFT(cluster < blr_priors_.size(), Illegal_argument, 
            "cluster index is out of bound");
    std::vector<std::pair<Vector, double> > res(blr_priors_[cluster].size());
    size_t damping_index_start = param_length(num_oscillators, use_modal) 
                                    - num_oscillators;
    std::cout << "daming_index_start: " << damping_index_start << std::endl;
    for(size_t i = 0; i < blr_priors_[cluster].size(); i++)
    {
        res[i] = blr_priors_[cluster][i].get_sample_from_prior();
        Vector mean = res[i].first;
        // make sure the ratio and stiffness to be positive
        while(i <= damping_index_start && mean[0] < 0)
        {
            res[i] = blr_priors_[cluster][i].get_sample_from_prior();
            mean = res[i].first;
        }
        if(use_modal && i == num_oscillators - 1)
        {
            // check valid mode angle
            assert(num_oscillators == 2);
            double pivot = (res[0].first[0] - M_PI/2.0) * (mean[0] - M_PI/2.0);
            /*while(pivot > 0)
            {
                res[i] = blr_priors_[cluster][i].get_sample_from_prior();
                pivot = (res[0].first[0] - M_PI/2.0) * (mean[0] - M_PI/2.0); 
            }*/
        }
    }

    return res;
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

std::vector<std::vector<std::pair<Vector, double> > >
Independent_blr_prior::generate_samples_from_prior(const Lss_set& lsss) const
{
    std::vector<std::vector<std::pair<Vector, double> > > samples;
    for(size_t cluster = 0; cluster < blr_priors_.size(); cluster++)
    {
        samples.push_back(generate_samples_from_cluster_prior(cluster, lsss));
    }
    return samples;
}


/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

/*double Independent_blr_prior::posterior(const Lss_set& lsss) const
{
    const std::vector<Vector>& pred_coefs = lsss.pred_coefs();
    assert(pred_coef.size() == blr_priors_.size());
    const std::map<size_t, double>& variances = lsss.clo_variances();
    double pr = 0.0;
    std::map<size_t, double>::const_iterator it = variances.begin();
    for(; it != variances.end(); it++)
    {
        size_t i = it->first;
        assert(i < pred_coefs.size());
        const Vector& mean = pred_coefs[i];
        double variance = it->second;
        if(std::isinf(variance) || std::isnan(variance) || variance < 0.0)
        {
            return -std::numeric_limits<double>::max();
        }
        else
        {
            double s_p = blr_priors_[i].log_posterior(mean, variance);

            if(std::isinf(std::abs(s_p)))
            {
                s_p = -std::numeric_limits<double>::max();
            }
            pr += s_p;
        }
    }
    return pr;
}*/

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

void Gaussian_mixture_prior::update_group_info(const Lss_set& lsss) const
{
    // update the number of groups
    size_t K = mixtures_.get_num_clusters();
    lsss.num_groups() = K;

    // update the group assignments
    update_group_assignments(lsss);

    // update the mixture distributions
    mixtures_.update_cluster_cache(lsss.lss_vec());
    size_t num_samples = 10;
    const std::vector<int>& groups = mixtures_.assignments();

    lsss.group_means().resize(K);
    lsss.group_covariances().resize(K);
    // update the group means and variances
    for(size_t k = 0; k < K; k++)
    {
        lsss.group_means()[k] = mixtures_.get_mixture_mean(k);
        lsss.group_covariances()[k] = mixtures_.get_mixture_covariance(k);
    }
    lsss.update_lss_group();

    // update the group weights
    lsss.group_weights() = mixtures_.get_mixture_weights();
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

void Gaussian_mixture_prior::generate_samples(const Lss_set& lss_set) const
{
    size_t N = lss_set.lss_vec().size();
    std::vector<Vector> param_samples;
    std::vector<int> groups;
    update_group_info(lss_set);
    mixtures_.generate_cluster_samples(N, param_samples, groups);

    for(size_t i = 0; i < N; i++)
    {
        size_t num_params = param_samples[i].size();
        // lss
        const Linear_state_space& lss = lss_set.lss_vec()[i];
        size_t num_oscillators = lss.num_oscillators();
        int params_per_osc = 1 + num_oscillators;
        BOOST_FOREACH(Coupled_oscillator& co, lss.coupled_oscillators())
        {
            for(size_t j = 0; j < num_params; j++)
            {
                if(j % params_per_osc == 0)
                {
                    // Make sure the frequency is negative 
                    co.set_param(j, -std::exp(param_samples[i][j]));
                }
                else
                {
                    co.set_param(j, param_samples[i][j]);
                }
            }
        }
        lss.changed_index() = 0;
        // group
        lss.group_index() = groups[i]; 
    }
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

Shared_scale_prior::Shared_scale_prior
(
    //const std::vector<size_t>& person_indices,
    const std::vector<double>& shapes,
    const std::vector<double>& scales
)  
{
    IFT(shapes.size() == scales.size(), Illegal_argument, 
            "shapes and scales of Shared_variance_prior must be the same size");
    for(size_t i = 0; i < scales.size(); i++)
    {
        P_vars_.push_back(Inverse_gamma_distribution(shapes[i], scales[i]));
    }
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

double Shared_scale_prior::operator()(const Lss_set& lsss) const
{
    const Double_v& gp_scales = lsss.gp_scales();
    assert(gp_scales.size() == P_vars_.size());
    double pr = 0.0;
    for(size_t i = 0; i < gp_scales.size(); i++)
    {
        double gp_scale = gp_scales[i];
        if(std::isinf(gp_scale) || std::isnan(gp_scale) || gp_scale < 0.0)
        {
            return -std::numeric_limits<double>::max();
        }
        else
        {
            double s_p = log_pdf(P_vars_[i], gp_scale);
            if(std::isinf(std::abs(s_p))) s_p = -std::numeric_limits<double>::max();
            pr += s_p;
        }
    }
    return pr;
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

Shared_noise_prior::Shared_noise_prior
(
    const std::vector<double>& shapes,
    const std::vector<double>& scales
)  
{
    IFT(scales.size() == scales.size(), Illegal_argument, 
            "shapes and scales of Shared_noise_prior must be the same size");
    for(size_t i = 0; i < scales.size(); i++)
    {
        P_vars_.push_back(Inverse_gamma_distribution(shapes[i], scales[i]));
    }
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

double Shared_noise_prior::operator()(const Lss_set& lsss) const
{
    const Vector& noise_sigmas = lsss.get_noise_sigmas();
    IFT(noise_sigmas.size() == P_vars_.size(), Runtime_error, 
                    "Shared_noise_prior is not initialized");
    double pr = 0.0;
    for(size_t i = 0; i < noise_sigmas.size(); i++)
    {
        double variance = noise_sigmas[i] * noise_sigmas[i];
        if(std::isinf(variance) || std::isnan(variance) || variance < 0.0)
        {
            return -std::numeric_limits<double>::max();
        }
        else
        {
            //double s_p = lss.get_times().size() * log_pdf(P_vars_[i], variance);
            // (std::log(P_vars_[i]) is for the jocobian term 
            double s_p = log_pdf(P_vars_[i], variance);// + std::log(variance); 
            if(std::isinf(std::abs(s_p))) 
            { 
                return -std::numeric_limits<double>::max();
            }
            pr += s_p;
        }
    }
    return pr;
}
