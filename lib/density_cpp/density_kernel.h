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

#ifndef DENSITY_KERNEL_H
#define DENSITY_KERNEL_H

/**
 * @file    Contains implementation of methods for kernel density estimation.
 */

#include <m_cpp/m_vector.h>
#include <m_cpp/m_matrix.h>
#include <iterator>
#include <cmath>

namespace kjb {

/**
 * @brief   Fixed kernel density estimator.
 *
 * This function approximates f(x), where f is the pdf of a random variable
 * that produced the samples in sequence [first, last).
 *
 * @param   x       The point where the pdf is to be estimated.
 * @param   first   Iterator to the first sample.
 * @param   last    One-past-the-end iterator in sequence of samples.
 * @param   K       The kernel to use in the approximation.
 * @param   h       Bandwidth.
 */
template <class Iterator, class Kernel>
double fkde
(
    double x,
    Iterator first,
    Iterator last,
    const Kernel& K,
    double h
)
{
    IFT(first != last, Illegal_argument, "FKDE: need at least one sample.");

    // compute sum
    size_t N = 0;
    double f = 0.0;
    for(; first != last; first++)
    {
        double x_i = *first;
        f += K((x - x_i) / h);
        N++;
    }

    return f / (N*h);
}

/**
 * @brief   Fixed kernel multivariate density estimator.
 *
 * This function approximates f(x), where f is the pdf of a random variable
 * that produced the samples in sequence [first, last).
 *
 * @param   x       The point where the pdf is to be estimated.
 * @param   first   Iterator to the first sample.
 * @param   last    One-past-the-end iterator in sequence of samples.
 * @param   K       The univariate kernel to use in the approximation.
 * @param   first_h Iterator to first element in sequence of bandwidths.
 */
template <class VecIterator, class Kernel, class RealIterator>
double fkde
(
    const Vector& x,
    VecIterator first,
    VecIterator last,
    const Kernel& K,
    RealIterator first_h
)
{
    IFT(first != last, Illegal_argument, "FKDE: need at least one sample.");
    IFT(x.get_length() == first->get_length(), Illegal_argument,
        "FKDE: samples and estimation point must have same dimension.");

    // compute sum
    size_t N = 0;
    double f = 0.0;
    for(; first != last; first++)
    {
        const Vector& x_i = *first;

        double f_i = 1.0;
        RealIterator dbl_p = first_h;
        for(size_t j = 0; j < x.get_length(); j++, dbl_p++)
        {
            const double& h_j = *dbl_p;
            f_i *= K((x[j] - x_i[j]) / h_j);
        }

        f += f_i;
        N++;
    }

    double h = 1.0;
    RealIterator dbl_p = first_h;
    for(size_t j = 0; j < x.get_length(); j++, dbl_p++)
    {
        const double& h_j = *dbl_p;
        h *= h_j;
    }

    return f / (N*h);
}

/**
 * @brief   Fixed kernel multivariate density estimator, single bandwidth.
 *
 * This function approximates f(x), where f is the pdf of a random variable
 * that produced the samples in sequence [first, last).
 *
 * @param   x       The point where the pdf is to be estimated.
 * @param   first   Iterator to the first sample.
 * @param   last    One-past-the-end iterator in sequence of samples.
 * @param   K       The univariate kernel to use in the approximation.
 * @param   h       Bandwidth to be used for all dimensions.
 */
template <class VecIterator, class Kernel>
inline double fkde_1h
(
    const Vector& x,
    VecIterator first,
    VecIterator last,
    const Kernel& K,
    double h
)
{
    std::vector<double> hs(x.get_length(), h);
    return fkde(x, first, last, K, hs.begin());
}

/**
 * @brief   Fixed kernel density estimator, using normal reference rule.
 *
 * This function approximates f(x), where f is the pdf of a random variable
 * that produced the samples in sequence [first, last).
 *
 * @param   x       The point where the pdf is to be estimated.
 * @param   first   Iterator to the first sample.
 * @param   last    One-past-the-end iterator in sequence of samples.
 * @param   K       The kernel to use in the approximation.
 * @param   s       Standard deviation of function.
 */
template <class Iterator, class Kernel>
inline double fkde_normal
(
    double x,
    Iterator first,
    Iterator last,
    const Kernel& K,
    double s
)
{
    const size_t N = std::distance(first, last);
    const double h = 1.06 * s * std::pow(N, -1.0/5);

    return fkde(x, first, last, K, h);
}

/**
 * @brief   Fixed kernel multivariate density estimator that uses the normal
 *          reference rule to compute bandwidth.
 *
 * This function approximates f(x), where f is the pdf of a random variable
 * that produced the samples in sequence [first, last).
 *
 * @param   x       The point where the pdf is to be estimated.
 * @param   first   Iterator to the first sample.
 * @param   last    One-past-the-end iterator in sequence of samples.
 * @param   K       The univariate kernel to use in the approximation.
 * @param   first_s Iterator to first element in sequence of variances.
 */
template <class VecIterator, class Kernel, class RealIterator>
inline double fkde_normal
(
    const Vector& x,
    VecIterator first,
    VecIterator last,
    const Kernel& K,
    RealIterator first_s
)
{
    const size_t D = x.get_length();
    const size_t N = std::distance(first, last);

    std::vector<double> h(D);
    for(size_t j = 0; j < D; j++, first_s++)
    {
        double s = *first_s;
        h[j] = std::sqrt(s) * std::pow(N, -1.0/(D + 4));
    }

    return fkde(x, first, last, K, h.begin());
}

} //namespace kjb

#endif /*DENSITY_KERNEL_H */

