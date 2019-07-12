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

/* $Id: likelihood.h 22559 2019-06-09 00:02:37Z kobus $ */

#ifndef KJB_TIES_LIKELIHOOD_H
#define KJB_TIES_LIKELIHOOD_H

#include <m_cpp/m_vector.h>
#include <prob_cpp/prob_distribution.h>
#include <prob_cpp/prob_sample.h>

#include "dbn_cpp/linear_state_space.h"
#include "dbn_cpp/lss_set.h"
#include "dbn_cpp/data.h"
#include "dbn_cpp/util.h"

namespace kjb {
namespace ties {

/**
 * @class   A class which represents the Likelihood model 
 */
class Likelihood 
{
public:

    /** @brief  Contructs a Likelihood from data. */
    Likelihood
    (
        const Data& data,
        size_t segment_length = 0
    ) : 
        data_(&data),
        sample_segment_length_(segment_length)
    {
        //sampled_time_indices_ = 
            //get_sampled_time_indices(data.times, sample_segment_length_);
    }

    /** @brief  Compute the likelihood. */
    double operator()(const Linear_state_space& lss) const
    {
        const Double_v& times = lss.get_times();
        if(sample_segment_length_ == 0) return log_prob(lss);
        double ll = 0.0;
        const size_t N = 2;
        Uniform_distribution p_dist;
        for(size_t i = 0; i < N; i++)
        {
            std::vector<size_t> sampled_times = 
                    get_sampled_time_indices(times, sample_segment_length_);
            ll += log_prob(lss, 0, sampled_times);
        }
        //std::cout << ll/N << std::endl;
        //std::cout << "XXXXXXXXXXXXXXXXXXXXXXXXXXXX" << std::endl;
        //assert(sampled_times.size() <= sample_segment_length_);
        return ll/N * sample_segment_length_;
    }

    /** @brief  Compute the predictive probability . */
    double predictive_probability(const Linear_state_space& lss) const
    {
        Linear_state_space lss_copy(lss);
        lss_copy.update_times(data_->times);
        size_t num_training = lss.get_times().size();
        /*std::vector<size_t> next_time_indices;
        for(size_t i = num_training; i < data_->times.size(); i++)
        {
            next_time_indices.push_back(i);
        }*/
        double pr = log_prob(lss_copy);
        //std::cout  << "trainig: " << pr; 
        double pp = log_prob(lss_copy, num_training);
        //std::cout << " PRED: " << pp << std::endl;
        return pp; //log_prob(lss_copy, next_time_indices);
    }

    /** @brief  Compute the squared errors of each output state. */
    Vector get_squared_errors(const Linear_state_space& lss) const; 

private:
    double log_prob
    (
        const Linear_state_space& lss, 
        size_t start_index = 0,
        const std::vector<size_t>& time_indices = std::vector<size_t>()
    ) const;
    const Data* data_;
    size_t sample_segment_length_;
    mutable std::vector<size_t> sampled_time_indices_;
};

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

inline
void likelihood_worker
(
    const std::vector<Likelihood>& likelihoods,
    const std::vector<Linear_state_space>& lss_vec,
    size_t start,
    size_t end,
    std::vector<double>& lls
)
{
    size_t N = likelihoods.size();
    assert(start < N && end < N);
    for(size_t i = start; i <= end; i++)
    {
        lls[i] = likelihoods[i](lss_vec[i]);
    }
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

std::vector<double> individual_likelihoods
(
    const Lss_set& lsss,
    const std::vector<Likelihood>& likelihoods,
    size_t num_threads
);

}} // namepsace kjb::ties

#endif // KJB_TIES_LIKELIHOOD_H

