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
/* $Id: cluster_gaussian_mixtures.h 21287 2017-03-06 19:50:38Z jguan1 $ */

#ifndef KJB_GAUSSIAN_MIXTURE_COMPONENT_H
#define KJB_GAUSSIAN_MIXTURE_COMPONENT_H

#include <m_cpp/m_vector.h>
#include <m_cpp/m_matrix.h>
#include <n_cpp/n_cholesky.h>
#include <prob_cpp/prob_distribution.h>
#include <prob_cpp/prob_sample.h>

#include <vector>
#include <algorithm>
#include <functional>

#include <boost/bind.hpp>

#define DEFAULT_CLUSTER 3

namespace kjb {

template<class Data>
class Gaussian_mixtures
{
public:

    /**
     * @brief   The constructor for Gaussian Mixture Component
     * @param   K:     Number of clusters
     * @param   N:     Number of data points
     * @param   D:     Dimension of a single data point
     * @param   lambda:  The prior of the mixture components
     * @param   mean_o:  The prior of the mean 
     * @param   kappa_o: How much we believe in the above prior
     * @param   S_o:     The prior of the covariance 
     * @param   v_o:    How much we believe in the above prior
     */
    Gaussian_mixtures
    (
        size_t N = 10,
        size_t D = 2,
        double lambda = 2.0,
        const Vector& mean_o = Vector(2, 0.0),
        double kappa_o = 0.0001,
        const Matrix& S_o = create_diagonal_matrix(2, 1.0),
        int v_o = 4,
        bool infinite = false
    );
    
    /** @breif    Return the number clusters */
    size_t get_num_clusters() const { return K_; }

    /** @brief    Return the weights of the mixture components. */
    const Vector& get_mixture_weights() const 
    { 
        return mixture_weights_; 
    }

    /** @brief    Set the mixture weights to the new value */
    void set_mixture_weights(const Vector& weights) const
    {
        mixture_weights_ = weights;
    }

    /** @brief    Return the mean of the k-th mixture* */
    const Vector& get_mixture_mean(size_t k) const 
    { 
        IFT(k < mixture_dists_.size(), Illegal_argument, "Index out of bound");
        return mixture_dists_[k].get_mean(); 
    }

    /** @brief    Return the covariance of the k-th mixture. */
    const Matrix& get_mixture_covariance(size_t k) const 
    {
        IFT(k < mixture_dists_.size(), Illegal_argument, "Index out of bound");
        return mixture_dists_[k].get_covariance_matrix(); 
    }

    /** @brief    Return the distribution of the k-th mixture .*/
    const MV_gaussian_distribution& get_mixture_dist(size_t k) const
    {
        IFT(k < mixture_dists_.size(), Illegal_argument, "Index out of bound");
        return mixture_dists_[k];
    }

    /** @brief    Set the mixture distributions */
    void set_mixture_distributions
    (
        const std::vector<Vector>& means,
        const std::vector<Matrix>& variances
    ) const
    {
        mixture_dists_.clear();
        size_t K = means.size();
        K_ = K;
        lambdas_.clear();
        lambdas_.resize(K_, lambda_/K_);
        assert(means.size() == variances.size());
        for(size_t k = 0; k < means.size(); k++)
        {
            mixture_dists_.push_back(
                    MV_gaussian_distribution(means[k], variances[k]));
        }
    }

    /** @brief    Generate K number of samples for the mixture distributions.*/
    void generate_mixture_distributions_from_prior(size_t K) const
    {
        K_ = K;
        mixture_dists_.clear();
        for(size_t k = 0; k < K; k++)
        {
            std::pair<Vector, Matrix> res = sample(niw_prior_);
            while(res.first[0] > 0.0 || res.first[3] > 0.0)
            {
                res = sample(niw_prior_);
            }
            mixture_dists_.push_back(
                    MV_gaussian_distribution(res.first, res.second));
        }
    }

    /** @brief   Generate K mixture weights */
    void generate_mixture_weights(size_t K) const
    {
        if(lambdas_.size() != K)
        {
            lambdas_.resize(K, lambda_/K);
            K_ = K;
        }
        Dirichlet_distribution weights_prior(lambdas_);
        mixture_weights_ = sample(weights_prior);
    }

    /** 
     * @brief    Estimate the mixture distribution params by sampling 
     *           from the posterior 
     */
    void estimate_dist_params
    (
        size_t num_samples,
        std::vector<Vector>& means,
        std::vector<Matrix>& covars
    ) const;

    /** 
     * @brief    Estimate the mixture weights by sampling from 
     *           its posterior 
     */
    void estimate_mixture_weights
    (
        size_t num_samples,
        Vector& weights
    ) const; 

    /** @brief    Return samples from the prior. */
    void generate_cluster_samples
    (
        size_t num_samples,
        std::vector<Vector>& samples,
        std::vector<int>& assignments
    ) const
    {
        if(K_ == 0) K_ = DEFAULT_CLUSTER;
        if(mixture_weights_.empty())
        {
            generate_mixture_weights(K_);
        }
        if(mixture_dists_.empty())
        {
            generate_mixture_distributions_from_prior(K_);
        }
        samples.resize(num_samples);
        assignments.resize(num_samples);
        Categorical_distribution<size_t> group_dist(mixture_weights_, 0);
        for(size_t i = 0; i < num_samples; i++)
        {
            size_t cluster = sample(group_dist);
            assignments[i] = cluster;
            samples[i] = sample(mixture_dists_[cluster]);
        }
    }

    /** @brief    Set the cache to be dirty */
    void set_cluster_cache_dirty() const
    {
        cluster_cache_dirty_ = true;
    }

    /** 
     *
     * @brief    Return the mixture assignment of each mixture.
     * @return   Z_ = {z_1, z_2, ..., z_N}, and z_i \in {1, 2, ..., K}
     */
    const std::vector<int>& assignments() const { return Z_; }

    /** 
     * @brief    Return the responsibility of each cluster to each point
     *           p(x_i, z_i = k | mu_k, sigma_k) 
     *            = p(z_i = k)p(x_i | mu_k, sigma_k)
     */
    std::vector<std::vector<double> > get_responsibilities
    (
        const std::vector<Data> data
    ) const 
    {
        size_t N = data.size();
        std::vector<std::vector<double> > resps(N);
        for(size_t i = 0; i < N; i++)
        {
            resps[i] = get_responsibilities(data[i]);
        }
        return resps;
    }

    /**
     * @brief   Run collapsed Gibbs step. 
     *          Sample the assignments directly by integrating out the weights
     *          and the parameters (means and sigmas) of each mixture. 
     *          For details see Murphy (2012) page 842 to page 843
     */
    void collapsed_step(const std::vector<Data>& data) const;

    /**
     * @brief   Return the log marginal probability of the data vectors
     *          assigned to component `k`
     *          The log marginal probability p(X) = p(x_1, x_2, ..., x_N) 
     *          is returned for the data vectors assigned to component `k`. 
     *          See (266) in Murphy's bayesGauss notes, p. 21.
     */
    double log_marginal_k(size_t k) const;

    /**
     * @brief   Add in the `i`th data to group `k`
     *          Update all the caches 
     *          If 'k' is 'K_', then a component is added. 
     */
    void add_data_to_cluster(const Data& data, size_t i, size_t k) const;

    /** @brief   Delete the `k` cluster */
    void delete_cluster(size_t k) const;

    /**
     * @brief   Add in the `i`th data to group `k`
     *          Update all the caches 
     */
    void del_data_from_cluster
    (
        const Data& data, 
        size_t i, 
        size_t k
    ) const;

    double log_prior(size_t i) const
    {
        return log_prior_x_[i]; 
    }

    /**
     * @brief   Return p(weights | lambda) p(Z_ |weights)
     */
    double log_hyper_prior() const
    {
        assert(!lambdas_.empty());
        assert(mixture_weights_.size() == lambdas_.size());
        Dirichlet_distribution weights_prior(lambdas_);
        double lp = log_pdf(weights_prior, mixture_weights_);
        for(size_t i = 0; i < Z_.size(); i++)
        {
            size_t cluster = Z_[i];
            assert(cluster < mixture_weights_.size());
            lp += std::log(mixture_weights_[cluster]);
        }
        assert(mixture_dists_.size() == mixture_weights_.size());
        for(size_t k = 0; k < mixture_dists_.size(); k++)
        {
            lp += log_pdf(niw_prior_, mixture_dists_[k].get_mean(),
                    mixture_dists_[k].get_covariance_matrix());
        }
        return lp;
    }

    /** 
     * @brief   A Helper function to compute the log of posterior predictive 
     */
    double log_post_pred_helper
    (
        int D, 
        int N, 
        double kappa, 
        double nu, 
        const Matrix& S
    ) const;

    /**
     * @brief   Return the log posterior predictive probabiilty of `X[i]` under
     *          component `k`
     */
    double log_post_pred_k
    (
        const Data& data, 
        size_t k
    ) const;

    /**
     * @brief   Return the log positerior predictive probability of `X[i]`
     */
    std::vector<double> log_post_pred
    (
        const Data& data
    ) const
    {
        if(cluster_cache_dirty_) update_cluster_cache(data);
        std::vector<double> log_post_preds(K_);
        for(size_t k = 0; k < K_; k++)
        {
            log_post_preds[k] = log_post_pred_k(data, k);
        }
        return log_post_preds;
    }

    /**
     * @brief   Return the log prior probability of component assignment 
     *          (24.24 Muphy)
     *          p(z_1, ..., z_N | lambda)
     */
    double log_prior_z() const 
    {
        double lp = log_prior_z_const_;
        double a = lambda_ / K_max_;
        for(size_t k = 0; k < K_max_; k++)
        {
            //assert(counts_[k] < log_gamma_N_k_lambda_.size());
            //lp += log_gamma_N_k_lambda_[counts_[k]];
            lp += lgamma(counts_[k] + a) - lgamma(a);
        }
        return lp;
    }

    /**
     * @brief   Return logp(x | z, mu, sigma, pi)
     */
    double log_prob_given_cluster(const std::vector<Data>& data) const
    {
        // check to see if the distribution is initialized 
        if(mixture_dists_.empty())
        {
            std::vector<Vector> mix_means(K_);
            std::vector<Matrix> mix_covs(K_);
            size_t num_samples = 10;
            estimate_dist_params(num_samples, mix_means, mix_covs);
            set_mixture_distributions(mix_means, mix_covs);
            Vector weights;
            estimate_mixture_weights(num_samples, weights);
            set_mixture_weights(weights);
        }
        double lp = 0.0;
        assert(data.size() == Z_.size());
        for(size_t i = 0; i < data.size(); i++)
        {
            size_t cluster = Z_[i];
            //lp += std::log(mixture_weights_[cluster]) + 
            lp += log_pdf(mixture_dists_[cluster], data[i]);
        }

        return lp;
    }

    /** 
     * @brief   Return the log marginal probability of all the data vectors
     *          given the component assigments.
     *          p(X | z) = p(x_1, x_2, ... x_N | z_1, z_2, ..., z_N)
     */
    double log_marginal(const std::vector<Data>& data) const
    {
        //assert(init_Z_);
        if(!init_Z_)
        {
            if(K_ > 0) init_assignments(K_);
            else 
            {
                std::string msg("Log marginal should not be computed"
                        "for non positive clusters.");
                KJB_THROW_2(Runtime_error, msg);
            }
        }
        if(cluster_cache_dirty_) update_cluster_cache(data);
        double lm = log_prior_z();
        for(size_t k = 0; k < K_; k++)
        {
            lm += log_marginal_k(k);
        }
        return lm;
    }

    /** @brief   Initialize the mixture assignments randomly. */
    void init_assignments(size_t K) const
    {
        K_ = K;
        if(K_ > 0)
        {
            randomly_assign_groups(K);
            init_Z_ = true;
        }
    }

    /** @brief   Update the assignments  */
    void set_assignments(const std::vector<int>& assigns) const
    {
        if(assigns.empty()) return;
        Z_.resize(assigns.size(), 0);
        std::copy(assigns.begin(), assigns.end(), Z_.begin());
    }

    /** @brief   Update the cluster related cached values */
    void update_cluster_cache(const std::vector<Data>& data) const;

    /** @brief   Update the number of data in each groups. */
    void update_counts() const;

    /** @brief   Update the means and variances of each mixtures. */
    void update_params() const;

    /** @brief   Update the weights of each mixture. */
    void update_mixture_weights() const;

    /** @brief   Return the responsibilities of each cluster to the data */
    std::vector<double> get_responsibilities(const Data& data) const;

    /** @brief   Update the assignment of the i-th data point. */
    size_t get_assignment(const Data& data) const;

    /** @brief   Update the assignment of each data point. */
    void update_assignments(const std::vector<Data>& data) const;

    /** @brief   Update data log priors */
    void update_data_priors(const std::vector<Data>& data) const;

    /** @breif   Randomly assign data points into different groups. */
    void randomly_assign_groups(size_t K) const;

private:
    mutable size_t K_;
    size_t K_max_;
    int N_;
    size_t D_;
   
    // Congugate priors for the mixture weights
    mutable std::vector<double> lambdas_;
    double lambda_; // = sum (lambdas) symmetric Diirichlet distribution
    //Dirichlet_distribution weights_prior_;
    // length of K (mixture weights)
    mutable Vector mixture_weights_;

    // Congugate priors for the Gaussians
    Vector mu_o_; // mu_o
    double kappa_o_; // kappa_o
    Matrix S_o_; // S_o_ 
    int v_o_; // v_o
    Normal_inverse_wishart_distribution niw_prior_; 

    // distribution of the mixture components
    mutable std::vector<MV_gaussian_distribution> mixture_dists_;
 
    // cluster assignments
    // length of N (z_1, z_2, ..., z_N) (-1 means the data is not assigned)
    mutable std::vector<int> Z_;
    // cluster member counts
    // length of K_max (min = 0, max = N)
    mutable std::vector<int> counts_;
  
    ///////////////////////////////////////////////////////////
    //          CACHE Variables  
    ///////////////////////////////////////////////////////////
    // consts 
    double log_pi_;
    double D_half_;
    double log_kappa_o_;
    double log_S_o_det_;
    double log_gamma_D_v_o_;
    double log_prior_z_const_;
    double log_cond_z_const_;
    std::vector<double> log_gamma_v_k_;
    Vector mu_numerator_const_;
    Matrix S_partial_const_;
    std::vector<double> log_gamma_N_k_lambda_;

    // varying cached variables
    mutable std::vector<Vector> mu_numerator_;
    mutable std::vector<Matrix> S_partial_;
    mutable std::vector<Vector> sum_x_k_;
    mutable std::vector<Matrix> sum_x_outer_k_;

    // flag for the assignment
    mutable bool init_Z_; 
    mutable bool cluster_cache_dirty_;
    mutable bool data_prior_cache_dirty_;

    // predictive prior of the data (need to be updated when data
    // is changed)
    // p(x_i |  mu_o, kappa_o, S_o_, v_o)
    mutable std::vector<double> log_prior_x_;

    //mutable std::vector<std::vector<double> > resps_;
    bool infinite_;

}; // Gaussian_mixtures


/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */
template <class Data> 
inline
Gaussian_mixtures<Data>::Gaussian_mixtures
(
    size_t N,
    size_t D,
    double lambda,
    const Vector& mean_o,
    double kappa_o,
    const Matrix& S_o,
    int v_o,
    bool infinite
) :
    K_(0),
    K_max_(N),
    N_(N),
    D_(D),
    //lambdas_(K_, lambda/K_),
    lambda_(lambda),
    //weights_prior_(lambdas_),
    //mixture_weights_(K, 1.0/K),
    mu_o_(mean_o),
    kappa_o_(kappa_o),
    S_o_(S_o),
    v_o_(v_o),
    niw_prior_(mu_o_, kappa_o_, S_o_, v_o_),
    Z_(N),
    counts_(K_max_),
    log_pi_(std::log(M_PI)),
    D_half_(D_/2.0),
    log_kappa_o_(std::log(kappa_o_)),
    log_S_o_det_(log_det(S_o_)),
    log_prior_z_const_(lgamma(lambda_) - lgamma(N_ + lambda)),
    log_cond_z_const_(std::log(N_ + lambda_ - 1)),
    mu_numerator_const_(kappa_o * mu_o_),
    S_partial_const_(S_o_ + kappa_o * outer_product(mu_o_, mu_o_)),
    mu_numerator_(K_max_),
    S_partial_(K_max_),
    init_Z_(false),
    cluster_cache_dirty_(true),
    data_prior_cache_dirty_(true),
    //resps_(N_, std::vector<double>()),
    infinite_(infinite)
{
    // cached variable
    log_gamma_D_v_o_ = 0.0;
    for(size_t i = 1; i <= D_; i++)
    {
        log_gamma_D_v_o_ = lgamma((v_o_ + 1 - i) /2.0); 
    }

    log_gamma_v_k_.resize(v_o_ + N_ + 2);

    // log_gamma_v_k_ = [lgamma((v_o_ - 1)/2.0),
    //                   lgamma((v_o_ + 1 - 1)/2.0),
    //                   lgamma((v_o_ + 2 - 1)/2.0),
    //                   ...
    //                   ]
    for(size_t i = 0; i <= v_o_ + N_ + 1; i++)
    {
        log_gamma_v_k_[i] = lgamma((v_o_ + i - 1)/2.0);
    }

    log_gamma_N_k_lambda_.resize(N_ + 1);
    double a = lambda_ / K_;
    for(size_t i = 0; i < N + 1; i++)
    {
        log_gamma_N_k_lambda_[i] = lgamma(i + a) - lgamma(a);
    }
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

template <class Data> 
void Gaussian_mixtures<Data>::randomly_assign_groups(size_t K) const
{
    if(init_Z_) return;
    // randomly assign the data into different groups 
    K_ = K;
    mixture_weights_.clear();
    mixture_weights_.resize(K, 1.0/K);
    lambdas_.clear();
    lambdas_.resize(K, lambda_/K);

    Categorical_distribution<size_t> group_dist(mixture_weights_, 0);
    for(size_t i = 0; i < N_; i++)
    {
        Z_[i] = sample(group_dist);
    }
    init_Z_ = true;
    cluster_cache_dirty_ = true;
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

template <class Data> 
void Gaussian_mixtures<Data>::update_counts() const
{
    counts_.resize(K_, 0);
    std::fill(counts_.begin(), counts_.end(), 0);
    for(size_t i = 0; i < N_; i++)
    {
        size_t group = Z_[i];
        assert(group < K_);
        counts_[group]++; 
        assert(counts_[group] <= N_);
    }
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

template <class Data> 
void Gaussian_mixtures<Data>::update_params() const
{
    assert(!cluster_cache_dirty_);
    if(mixture_dists_.size() != K_)
    {
        mixture_dists_.resize(K_, MV_gaussian_distribution(D_));
    }
    for(size_t k = 0; k < K_; k++)
    {
        // compute the new params
        double kappa = kappa_o_ + counts_[k];
        Vector mu = mu_numerator_[k] / kappa;
        double nu = v_o_ + counts_[k];
        Matrix S = S_partial_[k] - kappa * outer_product(mu, mu);
                   
        // construct the normal inverse wishart distribution
        Normal_inverse_wishart_distribution niw(mu, kappa, S, nu);
        std::pair<Vector, Matrix> x = sample(niw);
        // update the parameters
        mixture_dists_[k].set_mean(x.first);
        mixture_dists_[k].set_covariance_matrix(x.second);
    }
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

template <class Data> 
void Gaussian_mixtures<Data>::estimate_dist_params
(
    size_t num_samples,
    std::vector<Vector>& means,
    std::vector<Matrix>& covars
) const
{
    assert(num_samples > 0);
    means.resize(K_);
    covars.resize(K_);
    assert(!cluster_cache_dirty_);
    for(size_t k = 0; k < K_; k++)
    {
        Vector mean((int)D_, 0.0);
        Matrix var((int)D_, (int)D_, 0.0);
        for(size_t i = 0; i < num_samples; i++)
        {
            // compute the new params
            double kappa = kappa_o_ + counts_[k];
            Vector mu = mu_numerator_[k] / kappa;
            double nu = v_o_ + counts_[k];
            Matrix S = S_partial_[k] - kappa * outer_product(mu, mu);
                       
            // construct the normal inverse wishart distribution
            //Inverse_wishart_distribution iw(nu, S);
            Normal_inverse_wishart_distribution iw(mu, kappa, S, nu);
            // update the parameters
            std::pair<Vector, Matrix> x = sample(iw); 
            mean += x.first; 
            var += x.second;
        }
        means[k] = mean/num_samples;
        covars[k] = var/num_samples;
    }
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

template <class Data> 
void Gaussian_mixtures<Data>::update_mixture_weights() const
{
    std::vector<double> new_lambdas(lambdas_.begin(), lambdas_.end());
    for(size_t k = 0; k < K_; k++)
    {
        new_lambdas[k] += counts_[k];
        //std::cout << "counts_: " << counts_[k] << " ";
    }
    Dirichlet_distribution dir(new_lambdas);
    mixture_weights_ = sample(dir);
    //std::cout << "weights " << mixture_weights_ << std::endl;

}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

template <class Data> 
void Gaussian_mixtures<Data>::estimate_mixture_weights
(
    size_t num_samples,
    Vector& weights
) const
{
    assert(num_samples > 0);
   
    if(lambdas_.size() != K_)
    {
        lambdas_.resize(K_, lambda_/K_);
    }

    weights = Vector((int)K_, 0.0);
    for(size_t i = 0; i < num_samples; i++)
    {
        std::vector<double> new_lambdas(lambdas_.begin(), lambdas_.end());
        for(size_t k = 0; k < K_; k++)
        {
            new_lambdas[k] += counts_[k];
        }
        Dirichlet_distribution dir(new_lambdas);
        Vector x = sample(dir);
        weights += x;
    }
    weights /= num_samples;
    //std::copy(weights.begin(), weights.end(), 
            //mixture_weights_.begin());

}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

template <class Data>
std::vector<double> Gaussian_mixtures<Data>::get_responsibilities
(
     const Data& data
) const
{
    std::vector<double> resps(K_, 0.0);
    //double sum = 0.0;
    for(size_t k = 0; k < K_; k++)
    {
        /*std::cout << k << " : " << std::log(mixture_weights_[k]) 
                << " "<< log_pdf(mixture_dists_[k], data) << " \n";*/
        double log_p = std::log(mixture_weights_[k]) + 
                       log_pdf(mixture_dists_[k], data);
        resps[k] = log_p;
    }
    double sum = log_sum(resps.begin(), resps.end());
    for(size_t k = 0; k < K_; k++)
    {
        resps[k] = std::exp(resps[k] - sum);
    }

    /*std::cout << "resp: "; 
    std::copy(resps.begin(), resps.end(), std::ostream_iterator<double>(std::cout, " "));
    std::cout << "\n";*/

    return resps;
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

template <class Data> 
size_t Gaussian_mixtures<Data>::get_assignment
(
    const Data& data
) const
{
    // compute the new responsibilities
    std::vector<double> resps = get_responsibilities(data);
    Categorical_distribution<size_t> cdist(resps, 0);
    return sample(cdist);
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

template <class Data> 
void Gaussian_mixtures<Data>::update_assignments
(
    const std::vector<Data>& data
) const
{
    for(size_t i = 0; i < N_; i++)
    {
        // compute the new responsibilities
        Z_[i] = get_assignment(data[i]);
    }
    cluster_cache_dirty_ = true;
    update_counts();
    update_cluster_cache(data);
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

template <class Data> 
double Gaussian_mixtures<Data>::log_marginal_k(size_t k) const
{
    double kappa = kappa_o_ + counts_[k];
    double nu = v_o_ + counts_[k];
    Vector mu = mu_numerator_[k] / kappa;
    Matrix S = S_partial_[k] - kappa * outer_product(mu, mu);
    assert(k < counts_.size());
    double lm = - counts_[k] * D_half_ * log_pi_ 
                + D_half_ * log_kappa_o_ - D_half_ * std::log(kappa)
                + v_o_/2.0 * log_S_o_det_ - nu/2.0 * log_det(S);
    for(size_t i = 1; i <= D_; i++)
    {
        assert(nu - i < log_gamma_v_k_.size());
        lm += log_gamma_v_k_[nu - i];
    }
    lm -= log_gamma_D_v_o_;
    return lm; 
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

template <class Data> 
void Gaussian_mixtures<Data>::collapsed_step
(
    const std::vector<Data>& data
) const
{
    assert(data.size() == N_);
    // randomize the order of data 
    std::vector<size_t> indices(N_);
    for(size_t i = 0; i < N_; i++)
    {
        indices[i] = i;
    }
    std::random_shuffle(indices.begin(), indices.end());

    assert(accumulate(counts_.begin(), counts_.end(), 0) == N_);

    for(size_t id = 0; id < N_; id++)
    {
        size_t i = indices[id];
        int cluster_old = Z_[i];
        assert(cluster_old >= 0);
        size_t num_clusters_old = K_;
        //std::cout << "data [" << i << "] old k: " << cluster_old << " ";

        // cache some old values 
        Vector mu_numerator_old = mu_numerator_[cluster_old];
        Matrix S_partial_old = S_partial_[cluster_old];
        size_t old_count = counts_[cluster_old];

        // remove the data[i] statistics from its component
        del_data_from_cluster(data[i], i, cluster_old); 

        std::vector<double> log_prob_z(K_max_);
        std::vector<double> prob_z(K_max_);
        double log_prior = log_prior_x_[i];
        for(size_t k = 0; k < K_max_; k++)
        {
            // compute p(z_i = k | z\i, lambda) (24.26)
            log_prob_z[k] = log(counts_[k] + lambda_/K_max_) - log_cond_z_const_;
            if(k < K_)
            {
                log_prob_z[k] += log_post_pred_k(data[i], k);
            }
            else
            {
                // the data has not be assigned to any cluster yet 
                log_prob_z[k] += log_prior;
            } 
        }
        double log_sum_prob_z = log_sum(log_prob_z.begin(), log_prob_z.end());
        for(size_t k = 0; k < K_max_; k++)
        {
            prob_z[k] = std::exp(log_prob_z[k] - log_sum_prob_z);
        }

        // sample k_new
        Categorical_distribution<size_t> cdist(prob_z, 0);
        size_t cluster_new = sample(cdist);

        // The new cluster is greater than the current number of clusters
        if(cluster_new >= K_)
        {
            cluster_new = K_ - 1;
        }

        if(cluster_new == cluster_old && K_ == num_clusters_old)
        {
            // restore the old statistics 
            mu_numerator_[cluster_old] = mu_numerator_old;
            S_partial_[cluster_old] = S_partial_old;
            counts_[cluster_old] = old_count;
            // assignment 
            Z_[i] = cluster_old;
        }
        else
        {
            // Add the old statistcs back to component z_i = cluster_new
            add_data_to_cluster(data[i], i, cluster_new);
        }

        //std::cout << "new k: " << Z_[i];
    }
}
 
/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

template <class Data> 
void Gaussian_mixtures<Data>::delete_cluster(size_t k) const
{
    if(K_ == 0)
    {
        std::cout << "WARINING: there is no cluster to be deleted\n";
        return ;
    }
    assert(K_ >= 1);
    K_--;
    // if the delete cluster is not the last component
    if(k != K_)
    {
        // put stats from the last component into place of the one being removed
        mu_numerator_[k] = mu_numerator_[K_];
        S_partial_[k] = S_partial_[K_];
        counts_[k] = counts_[K_];
        // modifying the assignment 
        for(size_t i = 0; i < N_; i++)
        {
            if(Z_[i] == K_) 
                Z_[i] = k;
        }
        // distributions of each cluster
        /*if(!mixture_dists_.empty())
        {
            assert(mixture_dists_.size() == K_ + 1);
            mixture_dists_[k].set_mean(mixture_dists_[K_].get_mean());
            mixture_dists_[k].set_covariance_matrix(
                    mixture_dists_[K_].get_covariance_matrix());
            mixture_dists_.erase(mixture_dists_.end());
        }*/
    }
    // empty the last component
    mu_numerator_[K_] = mu_numerator_const_;;
    S_partial_[K_] = S_partial_const_;;
    counts_[K_] = 0;
    // delete the number of clusters
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

template <class Data> 
void Gaussian_mixtures<Data>::add_data_to_cluster
(
    const Data& data,
    size_t i,
    size_t k
) const
{
    if(k == K_)// || counts_[k] == 0)
    {
        K_++;
        mu_numerator_[k] = mu_numerator_const_;
        S_partial_[k] = S_partial_const_;
    }
    // counts
    counts_[k]++;
    // mean 
    mu_numerator_[k] += data;
    // variances
    S_partial_[k] += outer_product(data, data);
    // assingments
    Z_[i] = k;
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

template <class Data> 
void Gaussian_mixtures<Data>::del_data_from_cluster
(
    const Data& data,
    size_t i,
    size_t k
) const
{
    if(counts_[k] == 0)
    {
        std::cout << "CAN NOT DELETE AN ITEM (cluster is empty)\n";
        return;
    }
    // counts
    counts_[k]--;
    // assingments
    Z_[i] = -1;
    if(counts_[k] == 0)
    {
        // if the cluster is empty, delete this cluster
        delete_cluster(k);
    }
    else
    {
        // mean 
        mu_numerator_[k] -= data;
        // variances
        S_partial_[k] -= outer_product(data, data);
    }
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

template <class Data> 
double Gaussian_mixtures<Data>::log_post_pred_helper
(
    int D, 
    int N, 
    double kappa, 
    double nu, 
    const Matrix& S
) const
{
    // compute log(pi^{-ND/2} kappa^{-D/2} * |S|^{-nu/2})
    double t = - N*D_half_* log_pi_ - D_half_*std::log(kappa)
               - nu/2.0 * log_det(S);
    int start = nu - v_o_;
    assert(start >= 0 && start < log_gamma_v_k_.size());
    // compute sum_{i=1}^D lgamma((nu + i - 1) /2.0)
    for(size_t i = 1; i <= D; i++)
    {
        assert(start < log_gamma_v_k_.size());
        t += log_gamma_v_k_[start + i];
    }
    return t;
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

template <class Data> 
double Gaussian_mixtures<Data>::log_post_pred_k
(
    const Data& data, 
    size_t k
) const
{
    // Make sure the current parameters does not have data i
    double kappa = kappa_o_ + counts_[k];
    double nu = v_o_ + counts_[k];
    Vector mu = (mu_numerator_[k])/kappa;
    Matrix S = S_partial_[k] - kappa * outer_product(mu, mu);

    // add in the ith data
    Vector mu_wt_i = (mu_numerator_[k] + data)/ (kappa + 1);
    Matrix S_wt_i = S_partial_[k] + outer_product(data, data) -
                    (kappa + 1) * outer_product(mu_wt_i, mu_wt_i);

    double lp = log_post_pred_helper(D_, N_ + 1, kappa + 1, nu + 1, S_wt_i) 
                - log_post_pred_helper(D_, N_, kappa, nu, S);
    return lp;
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

template <class Data> 
void Gaussian_mixtures<Data>::update_cluster_cache
(   
    const std::vector<Data>& data
) const
{
    if(!cluster_cache_dirty_) return;
    // update the cache
    sum_x_k_.clear();
    sum_x_outer_k_.clear();
    sum_x_k_.resize(K_, Vector((int)D_, 0.0));
    sum_x_outer_k_.resize(K_, Matrix((int)D_, (int)D_, 0.0));
    for(size_t i = 0; i < N_; i++)
    {
        size_t group = Z_[i];
        assert(group < K_);
        Vector x = data[i];
        sum_x_k_[group] += x;
        sum_x_outer_k_[group] += outer_product(x, x);
    }

    // update the parameteres for mu_N and S_N
    for(size_t k = 0; k < K_; k++)
    {
        mu_numerator_[k] = mu_numerator_const_ + sum_x_k_[k];
        S_partial_[k] = S_partial_const_ + sum_x_outer_k_[k];
    }
    cluster_cache_dirty_ = false;
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

template <class Data> 
void Gaussian_mixtures<Data>::update_data_priors
(
    const std::vector<Data>& data
) const
{
    assert(N_ == data.size());
    log_prior_x_.clear();
    log_prior_x_.resize(N_);
    for(size_t i = 0; i < N_; i++)
    {
        double kappa = kappa_o_ + 1;
        int v = v_o_ + 1;
        //Vector cur_data(data[i]);
        Vector mu = (mu_numerator_const_ + data[i])/kappa;
        //Vector mu = (mu_numerator_const_ + cur_data)/kappa;
        Matrix S = S_partial_const_ + outer_product(data[i], data[i]) 
                    - kappa * outer_product(mu, mu);
        //Matrix S = S_partial_const_ + outer_product(cur_data, cur_data) 
        log_prior_x_[i] = log_post_pred_helper(D_, 1, kappa, v, S) -
                          log_post_pred_helper(D_, 0, kappa_o_, v_o_, S_o_);
    }
    data_prior_cache_dirty_ = false;
} 
}  //namespace kjb

#endif // KJB_GAUSSIAN_MIXTURE_COMPONENT_H
