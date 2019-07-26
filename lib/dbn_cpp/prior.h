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

/* $Id: prior.h 22559 2019-06-09 00:02:37Z kobus $ */

#ifndef KJB_TIES_PRIOR_H
#define KJB_TIES_PRIOR_H

#include <l/l_sys_debug.h>
#include <l/l_sys_def.h>
#include <l_cpp/l_util.h>
#include <l_cpp/l_functors.h>
#include <m_cpp/m_vector.h>
#include <prob_cpp/prob_distribution.h>
#include <prob_cpp/prob_pdf.h>
#include <prob_cpp/prob_sample.h>
#include <blr_cpp/bayesian_linear_regression.h>
#include <cluster_cpp/cluster_gaussian_mixtures.h>

#include <vector>
#include <algorithm>
#include <map>
#include <boost/bind.hpp>
#include <boost/foreach.hpp>
#include <boost/thread.hpp>

#include "dbn_cpp/coupled_oscillator.h"
#include "dbn_cpp/linear_state_space.h"
#include "dbn_cpp/likelihood.h"
#include "dbn_cpp/lss_set.h"

namespace kjb {
namespace ties {

enum Variance_type {INVERSE_GAMMA, INVERSE_CHI_SQUARED, INVALID_TYPE};
inline Variance_type get_prior_type(const std::string& name)
{
    if(name == "inverse-gamma") return INVERSE_GAMMA;
    else if(name == "inverse-chi-squared") return INVERSE_CHI_SQUARED;
    else return INVALID_TYPE;
}

class Cluster_prior
{
public:
    Cluster_prior
    (
        double lambda = 1.0,
        size_t num_clusters = 1
    ) :
        num_clusters_(num_clusters),
        lambdas_(num_clusters, lambda/num_clusters),
        num_data_per_cluster_(num_clusters, 0)
    {}

    /**
     * @brief   Initialize the number of data in each cluster
     */
    void init_num_data_per_cluster(const std::vector<size_t> cluster_assigns)
    {
        num_data_per_cluster_.clear();
        num_data_per_cluster_.resize(num_clusters_, 0);
        for(size_t i = 0; i < cluster_assigns.size(); i++)
        {
            size_t c_i = cluster_assigns[i];
            // insert the index to the map
            //index_map_[c_i].insert(std::make_pair(i, num_data_per_cluster_[c_i]));
            assert(c_i < num_clusters_);
            num_data_per_cluster_[c_i]++;
        }
        std::cout << "initialize num data per cluster: ";
        std::copy(num_data_per_cluster_.begin(), num_data_per_cluster_.end(),
                std::ostream_iterator<size_t>(std::cout, " "));
        std::cout << std::endl;
    }

    /** @brief   Return the prior of the cluster */
    double operator()(const Lss_set& lsss) const;

    /** @brief   Update the design matrix when param drift is allowed */
    void update_design_matrix(const Lss_set& lsss) const;

    /** @brief   Return the lambdas */
    const std::vector<double>& lambdas() const { return lambdas_; }

    /** @brief   Return the number of data points for each cluster */
    std::vector<size_t>& num_data_per_cluster() const 
    { 
        return num_data_per_cluster_;
    }

    /** 
     * @brief  Sample the cluster assignment of each Linear_state_space
     *         based on the current cluster_weights
     *         (We could improve the design by splitting this into two different
     *         parts, since sampling from the prior does not need lss_set)
     */
    void sample_cluster_assignments(const Lss_set& lss_set, bool from_prior) const; 

    /** 
     * @brief   Update the cluster weights based on the current 
     *          number of data per cluster
     */
    void sample_cluster_weights(const Lss_set& lss_set) const;

    /** 
     * @brief   Get a sample of the cluster weights from the prior
     *           
     */
    Vector sample_cluster_weights_from_prior() const
    {
        Dirichlet_distribution dist(lambdas_);
        return sample(dist);
    }

    /**
     * @brief   Decrease the number of elements in the cluster
     */
    void decrease_cluster_counts(size_t cluster_index) const
    {
        IFT(cluster_index < num_clusters_, 
                Illegal_argument, "cluster index out of bound");
        if(num_data_per_cluster_[cluster_index] >= 1)
        {
            num_data_per_cluster_[cluster_index]--;
        }
        else
        {
            std::cerr << "WARNING: should never be called\n";
        }
    }

    /**
     * @brief   Increase the number of elements in the cluster
     */
    void increase_cluster_counts(size_t cluster_index) const
    {
        IFT(cluster_index < num_clusters_, 
                Illegal_argument, "cluster index out of bound");
        num_data_per_cluster_[cluster_index]++;
    }

    /**
     * @brief   Return the index of data_index in cluster 
     */
    size_t get_and_remove_index_in_cluster
    (
        size_t cluster_index, size_t data_index
    ) const
    {
        IFT(cluster_index < num_clusters_, 
                Illegal_argument, "cluster index out of bound");
        std::map<size_t, size_t>::iterator it = 
            index_map_[cluster_index].find(data_index);
        IFT(it != index_map_[cluster_index].end(), Illegal_argument, 
                "Data index is not in the map");
        size_t index = it->second;
        index_map_[cluster_index].erase(it);

        decrease_cluster_counts(cluster_index);
        return index;
    }

    /**
     * @brief   Add data index to the right cluster 
     */
    void add_index_to_cluster(size_t cluster_index, size_t data_index) const
    {
        IFT(cluster_index < num_clusters_, 
                Illegal_argument, "cluster index out of bound");
        index_map_[cluster_index][data_index] = 
                            num_data_per_cluster_[cluster_index];
        increase_cluster_counts(cluster_index);
    }

private:
    size_t num_clusters_; 
    std::vector<double> lambdas_;
    mutable std::vector<size_t> num_data_per_cluster_;
    // lss-index: index-in-cluser
    mutable std::vector<std::map<size_t, size_t> > index_map_;
};

/**
 * @brief   A class represents the hyper prior of the priors of the lss parameters.
 */
class Independent_blr_prior
{
public:
    Independent_blr_prior
    (
        const Vector& shapes,
        const Vector& scales,
        const std::vector<Vector>& means,
        const std::vector<Matrix>& covariances,
        const std::vector<std::vector<Matrix> >& Xt,
        const std::vector<std::vector<Matrix> >& XtX,
        const std::string& prior_type = std::string("inverse-gamma"),
        bool sample_cluster = false
    ); 

    /** @brief    Return the log of the Normal Inverse Gamma prior. */
    double operator()(const Lss_set& lsss) const;

    /** @brief  Compute the params of the posterior based on the current y */
    std::vector<std::vector<bool> > compute_posterior
    (
        const Lss_set& lsss
    ) const;

    /** @brief   Generate samples from the prior for cluster */
    std::vector<std::pair<Vector, double> > 
    generate_samples_from_cluster_prior
    (
        size_t cluster, 
        const Lss_set& lsss
    ) const;

    /** @brief   Generate samples from the prior */
    std::vector<std::vector<std::pair<Vector, double> > >
    generate_samples_from_prior(const Lss_set& lsss) const;

    /** @brief   Return the bayesian linear regression prior */
    std::vector<std::vector<Bayesian_linear_regression> >& 
    blr_priors() const { return blr_priors_; }

private:
    // indexed by [GROUP][PARAM]
    mutable std::vector<std::vector<Bayesian_linear_regression> > blr_priors_;
    Variance_type type_;
    bool sample_cluster_;
};

/**
 * @brief   Gaussian mixture prior for group model
 */
class Gaussian_mixture_prior
{
public:
    Gaussian_mixture_prior
    (
        size_t N,
        size_t D,
        double lambda,
        const Vector& means_o,
        double kappa_o,
        const Matrix& S_o,
        int v_o, 
        size_t K, 
        bool collapsed = false,
        bool infinite = false,
        bool fixed_mixture_params = false
    ) : 
        mixtures_(N, D, lambda, means_o, kappa_o, S_o, v_o, infinite),
        collapsed_(collapsed),
        fixed_mixture_params_(fixed_mixture_params)
    {
        if(K > 0) mixtures_.randomly_assign_groups(K);
    }

    Gaussian_mixture_prior() : mixtures_(){}

    /** @brief   Return the mixture prior of the lsss  */
    double operator()(const Lss_set& lsss) const
    {
        //double temp =  mixtures_.log_marginal(lsss.lss_vec()) ;
        //std::cout << "log_marginal: " << temp << std::endl;
        //return temp;
        if(collapsed_)
        {
            return mixtures_.log_marginal(lsss.lss_vec());
        }
        else
        {
            return mixtures_.log_hyper_prior();
        }
    }

    /** @brief   Return a reference of the Gaussian mixture */
    const Gaussian_mixtures<Linear_state_space>& mixtures() const
    {
        return mixtures_;
    }

    void generate_mixtures(size_t K) const
    {
        // generate the mixture weights
        mixtures_.generate_mixture_weights(K);
        // generate the mixture distributions
        mixtures_.generate_mixture_distributions_from_prior(K);
    }

    /**
     * @brief    Update teh group assignments of a lsss */
    void update_group_assignments(const Lss_set& lsss) const
    {
        const std::vector<int>& assigns = mixtures_.assignments();
        assert(lsss.lss_vec().size() == assigns.size());
        /*std::cout << " assign: ";
        std::copy(assigns.begin(), assigns.end(), std::ostream_iterator<size_t>
                (std::cout, " "));
        std::cout << "\n";*/
        for(size_t i = 0; i < lsss.lss_vec().size(); i++)
        {
            assert(assigns[i] >= 0 && assigns[i] < lsss.num_groups());
            lsss.lss_vec()[i].group_index() = assigns[i];
        }
    }

    /** @brief   Update the group distributions. */
    void update_group_info(const Lss_set& lsss) const;

    /** @brief   Estimate the distribution params. */
    void estimate_dist_params(size_t num_samples = 10) const
    {
        size_t K = mixtures_.get_num_clusters();
        std::vector<Vector> mix_means(K);
        std::vector<Matrix> mix_covs(K);

        mixtures_.estimate_dist_params(num_samples, mix_means, mix_covs);
        mixtures_.set_mixture_distributions(mix_means, mix_covs);
    }

    /** @brief   Estimate the weights of the mixtures. */
    void estimate_mixture_weights(size_t num_samples = 10) const
    {
        Vector weights;

        mixtures_.estimate_mixture_weights(num_samples, weights);
        mixtures_.set_mixture_weights(weights);
    }

    /** @brief   Set the mixture distributions. */
    void set_mixture_distributions
    (
        const std::vector<Vector>& means,
        const std::vector<Matrix>& variances
    ) const
    {
        mixtures_.set_mixture_distributions(means, variances);
    }

    /**
     * @brief   Set the mixture weights. */
    void set_mixture_weights(const Vector& weights) const 
    {
        mixtures_.set_mixture_weights(weights);
    }

    void set_groups(const std::vector<int>& groups) const
    {
        mixtures_.set_assignments(groups);
    }

    /**
     * @brief   Return a vector of Linear_state_space with params 
     *          sampled from the mixture prior
     */
    void generate_samples(const Lss_set& lss_set) const;

    bool fixed_mixture_params() const { return fixed_mixture_params_;}

private:
    Gaussian_mixtures<Linear_state_space> mixtures_;
    bool collapsed_;
    bool fixed_mixture_params_;
};

/**
 * @brief   A class represents the prior of the sigmas of CLO parameters.
 */
class Shared_scale_prior
{
public:
    /** @brief  Constructs a Shared_scale_prior. */
    Shared_scale_prior
    (
        //const std::vector<size_t>& person_indices, 
        const std::vector<double>& shapes,
        const std::vector<double>& scales
    );

    /** @brief  Returns the prior of the sigmas of this Lss_set. */
    double operator()(const Lss_set& lss_set) const;

    /** @brief  Returns the distributions of this Shared_scale_prior. */
    const std::vector<Inverse_gamma_distribution>& get_dists() const
    {
        return P_vars_;
    }

private:
    std::vector<Inverse_gamma_distribution> P_vars_;
};

/**
 * @brief   class represents the prior distributions of the init state
 */
class Shared_noise_prior
{
public: 
    /** @brief  Constructs a Shared_noise_prior. */
    Shared_noise_prior
    (
        const std::vector<double>& shapes,
        const std::vector<double>& scales
    ); 

    /** @brief  Constructs a Shared_noise_prior. */
    Shared_noise_prior() {}

    /** @brief  Return the prior of the noise sigmas. */
    double operator()(const Lss_set& lsss) const;

    /** @brief  Return the prior distribution of the noise sigmas. */
    const std::vector<Inverse_gamma_distribution>& get_dists() const 
    {
        return P_vars_;
    }

    /** @brief  Return a sample of the noise sigmas. */
    Vector generate() const
    {
        Vector sigmas(P_vars_.size());
        for(size_t i = 0; i < P_vars_.size(); i++)
        {
            double temp = sample(P_vars_[i]);
            sigmas[i] = sqrt(temp);
        }
        return sigmas;
    }

private: 
    std::vector<Inverse_gamma_distribution> P_vars_;

}; // Shared_noise_prior

/**
 * @brief   A struct represents the prior of a Lss_set.
 */
struct Shared_lss_prior
{
    /** 
     * @brief   Constructs a Shared_lss_prior from Shared_mean_prior and
     *          Shared_variance_prior. 
     */
    Shared_lss_prior
    (
        const Independent_blr_prior& mean_var_pr,
        const Shared_scale_prior& scale_pr,
        //const Gaussian_mixture_prior& group_pr,
        const Cluster_prior& group_pr,
        bool l_clo = true,
        bool l_scale = false,
        bool l_group = false
    ) :
        mean_var_prior(mean_var_pr),
        scale_prior(scale_pr),
        group_prior(group_pr),
        learn_clo(l_clo),
        learn_scale(l_scale),
        learn_cluster(l_group)
    {}

    /** @brief  Returns the log value of the prior of a Lss_set. */
    double operator()(const Lss_set& lsss) const
    {
        double pr = 0.0;
        // Use normal-inverse-gamma prior when both mean and variance 
        // are unknown
        if(!learn_cluster)
        {
            if(learn_clo && !lsss.ignore_clo())
            {
                pr += mean_var_prior(lsss);
            }
            /*else
            {
                //std::cout << "SHOULD NOT BEING CALLED\n" << std::endl;
                //std::cout << "IND-CLO model\n";
            }*/
        }
        else
        {
            pr += group_prior(lsss);
        }

        // prior for the scale parameter of GP
        if(learn_scale)
        {
            double l_p = scale_prior(lsss);
            pr += l_p;
        }

        return pr;
    }

    const Independent_blr_prior& mean_var_prior;
    const Shared_scale_prior& scale_prior;
    //const Gaussian_mixture_prior& group_prior;
    const Cluster_prior& group_prior;
    mutable bool learn_clo; 
    mutable bool learn_scale;
    mutable bool learn_cluster;
};

/**
 * @brief   class represents the prior distributions of the init state
 */
class Init_prior
{
public: 

    /** @brief  Constructs a Init_prior. */
    Init_prior
    (
        double mean = 0.0,
        double sigma = 1.0,
        size_t num_oscillators = 2
    ) 
    {
        IFT(sigma > 0.0, Illegal_argument, "sigma can not be negative.");
        size_t num_params = 2 * num_oscillators;
        for(size_t i = 0; i < num_params; i++)
        {
            Ps_.push_back(Gaussian_distribution(mean, sigma));
        }
    }

    /** 
     * @brief   Returns the prior of the initial state of a Linear_state_space.
     */
    double operator()(const Linear_state_space& lss) const
    {
        const State_type& init_state = lss.init_state();
        KJB(ASSERT(init_state.size() == Ps_.size()));
        double pr = 0.0;
        for(size_t i = 0; i < Ps_.size(); i++)
        {
            pr += log_pdf(Ps_[i], init_state[i]);
        }
        return pr;
    }

    /** @brief   Returns the prior distributions .*/
    const std::vector<Gaussian_distribution> distributions() const
    {
        return Ps_;
    }

    /** @brief  Returns a sample of this prior. */
    State_type sample() const
    {
        State_type state(4);
        /*for(size_t i = 0; i < state.size(); i++)
        {
            state[i] = kjb::sample(Ps_[i]);
        }*/
        std::transform(Ps_.begin(), Ps_.end(), state.begin(),
                       static_cast<double(*)(const Gaussian_distribution&)>(
                                                                kjb::sample)); 
        return state;
    }

private:
    std::vector<Gaussian_distribution> Ps_; 

}; // Init_prior

}} // namespace kjb::ties
#endif

