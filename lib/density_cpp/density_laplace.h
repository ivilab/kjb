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

#ifndef DENSITY_LAPLCE_H
#define DENSITY_LAPLCE_H

/**
 * @file    Contains implementation of Laplace approximations to pdfs.
 */

#include <m_cpp/m_matrix.h>
#include <n_cpp/n_cholesky.h>
#include <l_cpp/l_exception.h>
#include <cmath>

namespace kjb {

/**
 * @brief   Approximates the value of the max of a pdf using the
 *          Laplace method.
 *
 * @param   H       The Hessian of -log(f), evaluated at its argmax (where
 *                  f is the pdf of interest).
 */
inline
double laplace_max_density(const Matrix& H)
{
    const size_t D = H.get_num_rows();

    IFT(D == static_cast<size_t>(H.get_num_cols()), Illegal_argument,
        "Cannot approximate marginal; Hessian matrix must be square.");

    // H is the inverse of the covariance matrix K;
    // that means that |K|^(-1/2) = |H|^(1/2)
    double det_H = exp(log_det(H));
    double p = std::sqrt(det_H) / std::pow(2*M_PI, D/2.0);

    return p;
}

/**
 * @brief   Approximates the value of the max of a log pdf using the
 *          Laplace method.
 *
 * @param   H       The Hessian of -log(f), evaluated at its argmax (where
 *                  f is the pdf of interest).
 */
inline
double laplace_max_log_density(const Matrix& H)
{
    const size_t D = H.get_num_rows();

    IFT(D == static_cast<size_t>(H.get_num_cols()), Illegal_argument,
        "Cannot approximate marginal; Hessian matrix must be square.");

    // H is the inverse of the covariance matrix K;
    // that means that |K|^(-1/2) = |H|^(1/2)
    // i.e., (-1/2)*log|K| = (1/2)*log|H| (hence the positive)
    double p = 0.5*log_det(H) - (D/2.0)*std::log(2*M_PI);

    return p;
}

} //namespace kjb

#endif /*DENSITY_LAPLCE_H */

