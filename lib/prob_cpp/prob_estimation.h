/* ======================================================================== *
 |                                                                          |
 | Copyright (c) 2007-2010, by members of University of Arizona Computer    |
 | Vision group (the authors) including                                     |
 |       Kobus Barnard, Kyle Simek, Ernesto Brau.                           |
 |                                                                          |
 | For use outside the University of Arizona Computer Vision group please   |
 | contact Kobus Barnard.                                                   |
 |                                                                          |
 * ======================================================================== */

/* $Id: prob_estimation.h 21596 2017-07-30 23:33:36Z kobus $ */

#ifndef PROB_ESTIMATION_H_INCLUDED
#define PROB_ESTIMATION_H_INCLUDED

/** @file
 *
 * @author Kobus Barnard
 * @author Ernesto Brau
 *
 * @brief   Functions for estimating distributional parameters.
 *
 * Given data, we can estimate the parameters of a distribution in
 * many ways, such as maximum-likelihood. Here is the functionality
 * for this.
 */

#include "prob_cpp/prob_distribution.h"
#include "l_cpp/l_exception.h"

#include <algorithm>
#include <cmath>
#include <functional>
#include <boost/accumulators/accumulators.hpp>
#include <boost/accumulators/statistics/stats.hpp>
#include <boost/accumulators/statistics/mean.hpp>
#include <boost/accumulators/statistics/variance.hpp>
#include <boost/bind.hpp>
#include <boost/ref.hpp>

namespace kjb {

/* @brief   Returns the MLE of the parameters of a normal. */
template<class InIter>
inline
Normal_distribution mle_normal(InIter first, InIter last)
{
    namespace ba = boost::accumulators;

    ba::accumulator_set<double, ba::stats<ba::tag::variance> > acc;
    std::for_each(first, last, boost::bind<void>(boost::ref(acc), _1));

    return Normal_distribution(ba::mean(acc), std::sqrt(ba::variance(acc)));
}

/* @brief   Returns the MLE of the parameters of a Beta. */
template<class InIter>
Beta_distribution mle_beta(InIter first, InIter last)
{
    namespace ba = boost::accumulators;

    ba::accumulator_set<double, ba::stats<ba::tag::variance> > acc;
    std::for_each(first, last, boost::bind<void>(boost::ref(acc), _1));

    double m = ba::mean(acc);
    double v = ba::variance(acc);

    // compute parameters, if possible
    IFT(v < m*(1 - m), Runtime_error,
        "Cannot compute Beta distribution parameters with this method.");

    // boost computes alpha and beta this way
    //double alpha = m * ((m*(1 - m))/v - 1);
    //double beta = (1 - m) * ((m*(1 - m))/v - 1);
    double alpha = Beta_distribution::find_alpha(m, v);
    double beta = Beta_distribution::find_beta(m, v);

    return Beta_distribution(alpha, beta);
}

/* @brief   Returns the MLE of the parameters of a Gamma. */
template<class InIter>
Gamma_distribution mle_gamma(InIter first, InIter last)
{
    double N = std::distance(first, last);
    double sum = std::accumulate(first, last, 0.0);
    double lsum = std::accumulate(
                            first, last, 0.0,
                            boost::bind(
                                std::plus<double>(), _1,
                                boost::bind(
                                    static_cast<double(*)(double)>(std::log),
                                    _2)));

    double s = std::log(sum/N) - lsum/N;
    double k = (3 - s + std::sqrt((s - 3)*(s - 3) + 24*s)) / (12*s);
    double t = sum/(N*k);

    return Gamma_distribution(k, t);
}

/* @brief   Returns the MLE of the parameters of a V-M-F. */
template<size_t D, class InIter>
Von_mises_fisher_distribution<D> mle_vmf(InIter first, InIter last)
{
    const size_t N = std::distance(first, last);
    Vector_d<D> sum = accumulate(first, last, Vector_d<D>(0.0));
    double mag = sum.magnitude();

    // mu
    Vector_d<D> mu = sum / mag;

    // kappa
    double R = mag / N;
    double kappa = (R*(3 - R*R)) / (1 - R*R);

    return Von_mises_fisher_distribution<D>(mu, kappa);
}

} //namespace kjb


#endif /* PROB_ESTIMATION_H_INCLUDED */

