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

/* $Id: sample_lss.h 22559 2019-06-09 00:02:37Z kobus $ */

#ifndef TIES_SAMPLE_LSS_H
#define TIES_SAMPLE_LSS_H

#include <vector>
#include <string>

#include <boost/none.hpp>
#include <boost/optional/optional.hpp>

#include "dbn_cpp/posterior.h"
#include "dbn_cpp/linear_state_space.h"

namespace kjb {
namespace ties {

/** @brief   Generate samples of model using mh.  */
std::vector<Linear_state_space> generate_lss_samples
(
    const Linear_state_space& init_lss,
    const Posterior& posterior,
    size_t num_samples,
    boost::optional<const std::string&> sample_target_fp = boost::none,
    boost::optional<const std::string&> sample_log_fp = boost::none,
    bool not_record = true,
    size_t burn_in = 0
);

/** @brief    Sample the individual lss */
bool sample_lss
(
    Linear_state_space& lss, 
    double max_val, 
    double min_val
);

/** @brief    Sample the individual lss inside the lss_set */
bool sample_lss(Lss_set& lss_set, double max_val = DBL_MAX, double min_val = -DBL_MAX);

/** @brief  Computes and returns the log of the predictive probability. */
double predictive_prob
(
    const Posterior& posterior, 
    const Linear_state_space& lss_vec,
    size_t num_samples = 0
);

/** @brief  Computes and returns the log of the predictive probability. */
double predictive_prob
(
    const Posterior& posterior, 
    const std::vector<Linear_state_space>& samples
);

/** 
 * @brief  Computes and returns the sum of log of the predictive 
 *         probability. 
 */
inline 
double sum_predictive_prob
(
    const std::vector<Posterior>& posteriors, 
    const std::vector<Linear_state_space>& lss_vec,
    size_t num_samples = 100
)
{
    IFT(posteriors.size() == lss_vec.size(), Illegal_argument, 
            "posteriors dimension differs from the lss_vec");
    std::vector<double> prs(lss_vec.size());

    std::transform(posteriors.begin(), posteriors.end(), 
                   lss_vec.begin(), prs.begin(), 
                   boost::bind(predictive_prob, _1, _2, num_samples));

    return std::accumulate(prs.begin(), prs.end(), 0.0);
}

bool sample_noise_data
(
    const Linear_state_space& lss, 
    Data& data,
    bool noise_free = false
);

Double_vv get_sampled_params
(
    const Linear_state_space& lss, 
    const Double_v& times, 
    const Likelihood& ll
);

}} // namespace kjb::ties

#endif // TIES_SAMPLE_LSS_H

