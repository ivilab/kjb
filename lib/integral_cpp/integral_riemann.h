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

#ifndef INTEGRAL_RIEMANN_H
#define INTEGRAL_RIEMANN_H

#include <l/l_verbose.h>
#include <l_cpp/l_exception.h>
#include <m_cpp/m_vector.h>
#include <diff_cpp/diff_util.h>
#include <vector>
#include <utility>
#include <numeric>
#include <functional>

namespace kjb {

/**
 * @brief   Computes the integral of a function numerically using a
 *          Riemann sum.
 *
 * @param   fcn         Function of interest. Must receive a model (M)
 *                      and return a double. Its type can be anything that
 *                      is callablle with the correct signature.
 * @param   bounds      Bounds (one pair of upper and lower bounds per
 *                      dimension)
 *                      of region of integration.
 * @param   nbins       Number of bins for approximation.
 * @param   adapter     Allows us to treat our model as a vector. Must have
 *                      A::get, A::set, and A::size implemented. See
 *                      kjb::Vector_adapter for an example.
 * @param   def_model   An initialized model. Must be ready for getting and
 *                      setting.
 *
 * @warn    The runtime of this function grows exponentially with the number
 *          of dimensions. Please only use for proof-of-concept or testing.
 */
template<class F, class M, class A>
double riemann_sum  
(
    const F& fcn,
    const std::vector<std::pair<double, double> >& bounds,
    size_t nbins,
    const A& adapter,
    const M& def_model
)
{
    const size_t D = bounds.size();
    IFT(D != 0, Illegal_argument, "Cannot integrate 0-dimensional function");

    if(D > 8)
    {
        kjb_c::warn_pso("The dimensionality of this function is %d; this "
                        "approximation might take a really long time.", D);
    }

    std::vector<double> bin_widths(D);
    for(size_t i = 0; i < D; i++)
    {
        bin_widths[i] = (bounds[i].second - bounds[i].first)/nbins;
    }

    std::vector<size_t> indices;
    M x = def_model;
    double I = 0;
    while(next_point(bounds, bin_widths, nbins, indices, x, adapter))
    {
        I += fcn(x);
    }

    double b = std::accumulate(bin_widths.begin(), bin_widths.end(),
                               1.0, std::multiplies<double>());
    I *= b;

    return I;
}

/**
 * @brief   Computes the integral of a function numerically using a
 *          Riemann sum.
 *
 * @param   fcn     Function of interest. Must receive a model (A::Model_type)
 *                  by const-ref and return a double. Its type can be anything
 *                  that
 *                  is callablle with the correct signature.
 * @param   bounds  Bounds (one pair of upper and lower bounds per dimension)
 *                  of region of integration.
 * @param   nbins   Number of bins for approximation.
 * @param   adapter Allows us to treat our model as a vector. Must have
 *                  A::get, A::set, and A::size implemented. See
 *                  kjb::Vector_adapter for an example.
 *
 * @warn    The runtime of this function grows exponentially with the number
 *          of dimensions. Please only use for proof-of-concept or testing.
 */
template<class F, class A>
inline
double riemann_sum  
(
    const F& fcn,
    const std::vector<std::pair<double, double> >& bounds,
    size_t nbins,
    const A& adapter
)
{
    typedef typename A::Model_type M;
    return riemann_sum(fcn, bounds, nbins, adapter, M());
}

/**
 * @brief   Computes the integral of a function numerically using a
 *          Riemann sum.
 *
 * @param   fcn     Function of interest. Must receive a kjb::Vector
 *                  and return a double. Its type can be anything that
 *                  is callablle with the correct signature.
 * @param   bounds  Bounds (one pair of upper and lower bounds per dimension)
 *                  of region of integration.
 * @param   nbins   Number of bins for approximation.
 *
 * @warn    The runtime of this function grows exponentially with the number
 *          of dimensions. Please only use for proof-of-concept or testing.
 */
template<class F>
inline
double riemann_sum  
(
    const F& fcn,
    const std::vector<std::pair<double, double> >& bounds,
    size_t nbins
)
{
    return riemann_sum(fcn, bounds, nbins,
                       Vector_adapter<Vector>(),
                       Vector(bounds.size()));
}

} //namespace kjb

#endif /*INTEGRAL_RIEMANN_H */

