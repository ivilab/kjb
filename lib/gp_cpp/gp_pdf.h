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

#ifndef GP_PDF_H_INCLUDED
#define GP_PDF_H_INCLUDED

#include <gp_cpp/gp_prior.h>
#include <gp_cpp/gp_posterior.h>
#include <gp_cpp/gp_predictive.h>
#include <gp_cpp/gp_normal.h>
#include <gp_cpp/gp_sample.h>
#include <m_cpp/m_vector.h>
#include <prob_cpp/prob_pdf.h>
#include <prob_cpp/prob_util.h>
#include <vector>

namespace kjb {
namespace gp {

/** @brief  Compute pdf of a GP prior distribution. */
template<class Mean, class Covariance>
inline
double pdf(const Prior<Mean, Covariance>& P, const Vector& f_s)
{
    return kjb::pdf(P.normal(), f_s);
}

/** @brief  Compute log-pdf of a GP prior distribution. */
template<class Mean, class Covariance>
inline
double log_pdf(const Prior<Mean, Covariance>& P, const Vector& f_s)
{
    return kjb::log_pdf(P.normal(), f_s);
}

/**
 * @brief   Evaluates the unnormalized pdf of the GP posterior on a set of
 *          outputs.
 */
template<class Mean, class Covariance, class Likelihood>
inline
double pdf(const Posterior<Mean, Covariance, Likelihood>& P, const Vector& f_s)
{
    const Likelihood& lhood = P.likelihood();

    double pr = pdf(P.prior(), f_s);
    double lh = exp(lhood(f_s));

    return pr * lh;
}

/**
 * @brief   Evaluates the log-unnormalized-pdf of the GP posterior on a set of
 *          outputs.
 */
template<class Mean, class Covariance, class Likelihood>
inline
double log_pdf
(
    const Posterior<Mean, Covariance, Likelihood>& P,
    const Vector& f_s
)
{
    const Likelihood& lhood = P.likelihood();

    double pr = log_pdf(P.prior(), f_s);
    double lh = lhood(f_s);

    return pr + lh;
}

/**
 * @brief   Evaluates the GP posterior (with linear-Gaussian likelihood) pdf
 *          on a set of test outputs.
 */
template<class Mean, class Covariance>
double pdf
(
    const Posterior<Mean, Covariance, Linear_gaussian>& P,
    const Vector& f
)
{
    return kjb::pdf(P.normal(), f);
}

/**
 * @brief   Evaluates the GP posterior (with linear-Gaussian likelihood)
 *          log-pdf on a set of test outputs.
 */
template<class Mean, class Covariance>
double log_pdf
(
    const Posterior<Mean, Covariance, Linear_gaussian>& P,
    const Vector& f
)
{
    return kjb::log_pdf(P.normal(), f);
}

/**
 * @brief   Evaluates the GP noise-less predictive pdf on a set of test
 *          outputs.
 */
template<class Mean, class Covariance>
inline
double pdf(const Predictive_nl<Mean, Covariance>& P, const Vector& f_s)
{
    return kjb::pdf(P.normal(), f_s);
}

/**
 * @brief   Evaluates the GP noise-less predictive log-pdf on a set of test
 *          outputs.
 */
template<class Mean, class Covariance>
inline
double log_pdf(const Predictive_nl<Mean, Covariance>& P, const Vector& f_s)
{
    return kjb::log_pdf(P.normal(), f_s);
}

/**
 * @brief   Evaluates the GP predictive pdf on a set of test
 *          outputs.
 */
template<class Mean, class Covariance, class Likelihood>
double pdf(const Predictive<Mean, Covariance, Likelihood>& P, const Vector& f_s)
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

    const size_t N = 100;
    double p = 0.0;
    for(size_t i = 0; i < N; i++)
    {
        Vector f = sample(post);
        pred.set_train_outputs(f.begin(), f.end());
        p += pdf(pred, f_s);
    }

    p /= N;

    return p;
}

/**
 * @brief   Evaluates the GP predictive log-pdf on a set of test
 *          outputs.
 */
template<class Mean, class Covariance, class Likelihood>
double log_pdf
(
    const Predictive<Mean, Covariance, Likelihood>& P,
    const Vector& f_s
)
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

    const size_t N = 100;
    std::vector<double> ps(N);
    for(size_t i = 0; i < N; i++)
    {
        Vector f = sample(post);
        pred.set_train_outputs(f.begin(), f.end());
        ps[i] = log_pdf(pred, f_s);
    }

    double p = log_sum(ps.begin(), ps.end()) - log(N);

    return p;
}

/**
 * @brief   Evaluates the GP predictive (with linear-Gaussian likelihood) pdf
 *          on a set of test outputs.
 */
template<class Mean, class Covariance>
double pdf
(
    const Predictive<Mean, Covariance, Linear_gaussian>& P,
    const Vector& f_s
)
{
    return kjb::pdf(P.normal(), f_s);
}

/**
 * @brief   Evaluates the GP predictive (with linear-Gaussian likelihood)
 *          log-pdf on a set of test outputs.
 */
template<class Mean, class Covariance>
double log_pdf
(
    const Predictive<Mean, Covariance, Linear_gaussian>& P,
    const Vector& f_s
)
{
    return kjb::log_pdf(P.normal(), f_s);
}

/**
 * @brief   Evaluates the GP predictive (with linear-Gaussian likelihood) pdf
 *          on a set of test outputs.
 */
template<class Mean, class Covariance>
double pdf
(
    const Marginal_likelihood<Mean, Covariance, Linear_gaussian>& P,
    const Vector& f_s
)
{
    return kjb::pdf(P.normal(), f_s);
}

/**
 * @brief   Evaluates the GP predictive (with linear-Gaussian likelihood)
 *          log-pdf on a set of test outputs.
 */
template<class Mean, class Covariance>
double log_pdf
(
    const Marginal_likelihood<Mean, Covariance, Linear_gaussian>& P,
    const Vector& f_s
)
{
    return kjb::log_pdf(P.normal(), f_s);
}

}} //namespace kjb::gp

#endif /*GP_PDF_H_INCLUDED */

