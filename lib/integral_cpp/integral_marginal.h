/* =========================================================================== *
   |
   |  Copyright (c) 1994-2011 by Kobus Barnard (author)
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

#ifndef INTEGRAL_MARGINAL_H
#define INTEGRAL_MARGINAL_H

#include <m_cpp/m_matrix.h>
#include <density_cpp/density_laplace.h>
#include <l_cpp/l_exception.h>
#include <cmath>

namespace kjb {

/**
 * @brief   Computes the Lapace-Metropolis approximation of the marginal
 *          likelihood.
 *
 * @tparam  M   The model type.
 * @tparam  P   The prior type. Must be callable and receive a M const-ref as
 *              input.
 * @tparam  L   The likelihood type. Must be callable and receive a M const-ref
 *              as input.
 *
 * @param   mx          The argmax of the joint.
 * @param   prior       The prior distribution.
 * @param   likelihood  The likelihood function.
 * @param   H           The hessian of the negative log of the joint evaluated
 *                      at mx.
 */
template<class M, class P, class L>
inline
double lm_marginal_likelihood
(
    const M& mx,
    const P& prior,
    const L& likelihood,
    const Matrix& H
)
{
    return (prior(mx) * likelihood(mx)) / laplace_max_density(H);
}

/**
 * @brief   Computes the Lapace-Metropolis approximation of the marginal
 *          log-likelihood.
 *
 * @tparam  M   The model type.
 * @tparam  P   The prior type. Must be callable and receive a M const-ref as
 *              input.
 * @tparam  L   The likelihood type. Must be callable and receive a M const-ref
 *              as input.
 *
 * @param   mx              The argmax of the joint.
 * @param   log_prior       The log-prior distribution.
 * @param   log_likelihood  The log-likelihood function.
 * @param   H               The hessian of the negative log of the joint
 *                          evaluated at mx.
 */
template<class M, class P, class L>
inline
double lm_marginal_log_likelihood
(
    const M& mx,
    const P& log_prior,
    const L& log_likelihood,
    const Matrix& H
)
{
    return log_prior(mx) + log_likelihood(mx) - laplace_max_log_density(H);
}

} //namespace kjb

#endif /*INTEGRAL_MARGINAL_H */

