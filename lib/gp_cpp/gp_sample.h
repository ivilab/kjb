/* =========================================================================== *
   |
   |  Copyright (c) 1994-2010 by Kobus Barnard (author)
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
   |  Author:  Ernesto Brau
 * =========================================================================== */

/* $Id$ */

#ifndef GP_SAMPLE_H_INCLUDED
#define GP_SAMPLE_H_INCLUDED

#include <gp_cpp/gp_prior.h>
#include <gp_cpp/gp_posterior.h>
#include <gp_cpp/gp_predictive.h>
#include <gp_cpp/gp_normal.h>
#include <m_cpp/m_vector.h>
#include <prob_cpp/prob_sample.h>

namespace kjb {
namespace gp {

/** @brief  Draw a sample from a GP prior distribution. */
template<class Mean, class Covariance>
inline
Vector sample(const Prior<Mean, Covariance>& P)
{
    return kjb::sample(P.normal());
}

/** @brief  Draws a sample (of outputs) from the GP posterior distribution. */
template<class Mean, class Covariance, class Likelihood>
inline
Vector sample(const Posterior<Mean, Covariance, Likelihood>& P)
{
    const Cp_sampler<Mean, Covariance, Likelihood>& sampler = P.sampler();
    sampler.burn_in();

    sampler.sample();
    return sampler.current();
}

/**
 * @brief   Draws a sample (of test outputs) from the posterior distribution
 *          of a linear-Gaussian GP.
 */
template<class Mean, class Covariance>
inline
Vector sample(const Posterior<Mean, Covariance, Linear_gaussian>& P)
{
    return kjb::sample(P.normal());
}

/**
 * @brief   Draws a sample (of test outputs) from the GP noise-less predictive
 *          distribution.
 */
template<class Mean, class Covariance>
inline
Vector sample(const Predictive_nl<Mean, Covariance>& P)
{
    return kjb::sample(P.normal());
}

/**
 * @brief   Draws a sample (of test outputs) from the GP predictive
 *          distribution.
 */
template<class Mean, class Covariance, class Likelihood>
inline
Vector sample(const Predictive<Mean, Covariance, Likelihood>& P)
{
    Posterior<Mean, Covariance, Likelihood> post(
                                            P.mean_function(),
                                            P.covariance_function(),
                                            P.trin_begin(),
                                            P.trin_end(),
                                            P.trout_begin(),
                                            P.trout_end());

    Predictive_nl<Mean, Covariance> pred(
                                    P.mean_function(),
                                    P.covariance_function(),
                                    P.trin_begin(),
                                    P.trin_end(),
                                    P.trout_begin(),
                                    P.trout_end(),
                                    P.tein_begin(),
                                    P.tein_end());

    Vector f = sample(post);
    pred.set_train_outputs(f.begin(), f.end());

    return sample(pred);
}

/**
 * @brief   Draws a sample (of test outputs) from the predictive distribution
 *          of a linear-Gaussian GP.
 */
template<class Mean, class Covariance>
inline
Vector sample(const Predictive<Mean, Covariance, Linear_gaussian>& P)
{
    return kjb::sample(P.normal());
}

}} //namespace kjb::gp

#endif /*GP_SAMPLE_H_INCLUDED */

