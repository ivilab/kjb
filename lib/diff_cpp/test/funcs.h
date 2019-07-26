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

#include <m_cpp/m_vector.h>
#include <prob_cpp/prob_distribution.h>
#include <prob_cpp/prob_pdf.h>

/** @brief  Helper function. */
inline
double two_x(const kjb::Vector& x)
{
    return 2*x[0];
}

/** @brief  Helper function. */
inline
double x_cubed(const kjb::Vector& x)
{
    return x[0]*x[0]*x[0];
}

/** @brief  Helper function. */
inline
double x_squared(const kjb::Vector& x)
{
    return dot(x, x);
}

/** @brief  Helper function. */
inline
double x_squared_ind_2(const kjb::Vector& x, size_t i, size_t j)
{
    if(i == j) return x[i]*x[i];

    return x[i]*x[i] + x[j]*x[j];
}

/** @brief  Helper function. */
inline
double negative_log_pdf
(
    const kjb::MV_normal_distribution& P,
    const kjb::Vector& x
)
{
    return -kjb::log_pdf(P, x);
}

/** @brief  Helper function. */
inline
double negative_log_pdf_ind
(
    const kjb::MV_normal_distribution& P,
    const kjb::Vector& x,
    size_t i
)
{
    double mu = P.get_mean()[i];
    double sg = sqrt(P.get_covariance_matrix()(i, i));

    kjb::Normal_distribution G(mu, sg);
    return -kjb::log_pdf(G, x[i]);
}

/** @brief  Helper function. */
inline
double negative_log_pdf_ind_2
(
    const kjb::MV_normal_distribution& P,
    const kjb::Vector& x,
    size_t i,
    size_t j
)
{
    double fi = negative_log_pdf_ind(P, x, i);
    if(i == j) return fi;

    double fj = negative_log_pdf_ind(P, x, j);
    return fi + fj;
}

