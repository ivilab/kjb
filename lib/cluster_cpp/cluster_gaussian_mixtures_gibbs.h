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
/* $Id: cluster_gaussian_mixtures_gibbs.h 20635 2016-04-22 22:57:34Z jguan1 $ */

#ifndef KJB_CLUSTER_GAUSSIAN_MIXTURES_GIBBS_H
#define KJB_CLUSTER_GAUSSIAN_MIXTURES_GIBBS_H

#include <m_cpp/m_vector.h>
#include <m_cpp/m_matrix.h>
#include <n_cpp/n_cholesky.h>
#include <prob_cpp/prob_distribution.h>
#include <prob_cpp/prob_sample.h>
#include <l_cpp/l_functors.h>
#include <cluster_cpp/cluster_gaussian_mixtures.h>

#include <vector>
#include <algorithm>
#include <functional>
#include <limits>

#include <boost/bind.hpp>


namespace kjb {

/**
 * @brief   A class that does Gibbs step to infer the clusters from
 *          data that generated from a Gaussian Mixture distribution
 */
template<class Data>
class Gaussian_mixture_gibbs_step
{
public:

    /**
     * @brief   Constructor A gibbs step
     * @param   K           number of clusters
     * @param   gmc         A gaussian mixture model
     * @param   collapsed   If true do collapsed Gibbs step that integrates out
     *                      mixture distribution means and variances
     * @param   fixed_dist_params   If true, keeps the params of the mixture
     *                              distribution during sampling
     */
    Gaussian_mixture_gibbs_step
    (
        size_t K,
        const Gaussian_mixtures<Data>& gmc,
        bool collapsed = false,
        bool fixed_dist_params = false
    ) : 
        gmc_(gmc),
        collapsed_(collapsed),
        fixed_dist_params_(fixed_dist_params),
        old_prior_(-std::numeric_limits<double>::max())
    {
        gmc_.init_assignments(K);
        gmc_.update_counts();
    }

    /** 
     * @brief   Run one Gibbs step to update the parameters, the mixture
     *          weights and the assgiments. 
     */
    void operator()(const std::vector<Data>& data, double& lp) const 
    {
        // initialize old_prior_ at the first time
        /*if(old_prior_ == -std::numeric_limits<double>::max())
        {
            gmc_.update_cluster_cache(data);
            if(collapsed_)
            {
                gmc_.update_data_priors(data);
            }
            old_prior_ = gmc_.log_marginal(data);
        }*/

        // perform one step
        if(!collapsed_)
        {
            //std::cout << " not collapsed\n";
            gmc_.update_cluster_cache(data);
            if(!fixed_dist_params_)
            {
                gmc_.update_params();
                gmc_.update_mixture_weights();
            }
            gmc_.update_assignments(data);
            /*std::cout << " assignments: ";
            std::copy(gmc_.assignments().begin(), gmc_.assignments().end(),
                    std::ostream_iterator<int>(std::cout, " "));
            std::cout << "\n";*/
            lp = gmc_.log_hyper_prior();
        }
        else
        {   
            gmc_.update_data_priors(data);
            gmc_.collapsed_step(data);
            lp = gmc_.log_marginal(data);
        }
        /*double new_prior = gmc_.log_marginal(data);
        lp = lp - old_prior_ + new_prior;
        old_prior_ = new_prior;*/
        // update the old prior
        //std::cout << " log mar: " << new_prior << std::endl; 
    }

    /**
     * @brief   Reset the old prior to uninitialized value,
     *          used when the data is changed 
     */
    void reset_prior() const
    {
        old_prior_ = -std::numeric_limits<double>::max();
        gmc_.set_cluster_cache_dirty();
    }

    bool fixed_dist_params() const { return fixed_dist_params_; }

    bool collapsed() const { return collapsed_; } 

    void estimate_dist_params(size_t num_samples = 10) const
    {
        size_t K = gmc_.get_num_clusters();
        std::vector<Vector> mix_means(K);
        std::vector<Matrix> mix_covs(K);

        gmc_.estimate_dist_params(num_samples, mix_means, mix_covs);
        gmc_.set_mixture_distributions(mix_means, mix_covs);
    }

    void estimate_mixture_weights(size_t num_samples = 10) const
    {
        Vector weights;

        gmc_.estimate_mixture_weights(num_samples, weights);
        gmc_.set_mixture_weights(weights);
    }

private:
    const Gaussian_mixtures<Data>& gmc_;
    bool collapsed_;
    bool fixed_dist_params_;
    mutable double old_prior_;

}; // Gaussian_mixture_gibbs_step
 
}  //namespace kjb
#endif // KJB_CLUSTER_GAUSSIAN_MIXTURES_GIBBS_H
