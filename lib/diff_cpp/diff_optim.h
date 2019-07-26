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

#ifndef DIFF_OPTIM_H
#define DIFF_OPTIM_H

#include <l/l_verbose.h>
#include <l_cpp/l_exception.h>
#include <m_cpp/m_vector.h>
#include <diff_cpp/diff_util.h>
#include <vector>
#include <utility>
#include <limits>
#include <algorithm>
#include <functional>

namespace kjb {

/**
 * @brief   Maximizes a function by evaluating at all points in a grid.
 *
 * @param   fcn         The function. Must recieve a const-ref to type M,
 *                      and return a double.
 * @param   bounds      Bounds (one pair of upper and lower bounds per
 *                      dimension)
 *                      of region of integration.
 * @param   nbins       Number of bins in grid.
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
double grid_maximize
(
    const F& fcn,
    const std::vector<std::pair<double, double> >& bounds,
    size_t nbins,
    const A& adapter,
    M& mxm
)
{
    const size_t D = bounds.size();
    IFT(D != 0, Illegal_argument, "Cannot optimize 0-dimensional function");

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
    M x = mxm;
    double mx = -std::numeric_limits<double>::max();
    while(next_point(bounds, bin_widths, nbins, indices, x, adapter))
    {
        double fx = fcn(x);
        if(fx > mx)
        {
            mx = fx;
            mxm = x;
        }
    }

    return mx;
}

/**
 * @brief   Maximizes a function by evaluating at all points in a grid.
 *
 * @param   fcn     Function of interest. Must receive a kjb::Vector
 *                  and return a double. Its type can be anything that
 *                  is callablle with the correct signature.
 * @param   bounds  Bounds (one pair of upper and lower bounds per
 *                  dimension)
 *                  of region of integration.
 * @param   nbins   Number of bins in grid.
 *
 * @warn    The runtime of this function grows exponentially with the number
 *          of dimensions. Please only use for proof-of-concept or testing.
 */
template<class F, class V>
inline
double grid_maximize
(
    const F& fcn,
    const std::vector<std::pair<double, double> >& bounds,
    size_t nbins,
    V& mxm
)
{
    const size_t D = bounds.size();
    if(mxm.size() != D)
    {
        mxm.resize(D);
    }

    return grid_maximize(fcn, bounds, nbins, Vector_adapter<V>(), mxm);
}

/**
 * @brief   Maximizes a function using a simple gradient ascent method.
 *
 * @param   x       Current value of model.
 *
 * @param   fcn     Function of interest. Must receive a const Model&
 *                  and return a double. Its type can be anything that
 *                  is callablle with the correct signature.
 *
 * @param   steps   Step sizes in each dimension.
 *
 * @param   grad    Gradient operator. Takes a const Model& and returns
 *                  a kjb::Vector.
 *
 * @param   adapter Allows us to treat our model as a vector. Must have
 *                  A::get, A::set, and A::size implemented. See
 *                  kjb::Vector_adapter for an example.
 */
template<class F, class M, class G, class A>
void gradient_ascent
(
    const F& fcn,
    M& x,
    const std::vector<double>& steps,
    const G& grad,
    const A& adapter
)
{
    const size_t D = adapter.size(&x);
    IFT(D == steps.size(), Illegal_argument,
        "cannot perform gradient ascent: wrong number of step sizes.");

    Vector g(D);
    std::vector<double> exg(D);

    // simultaneuous optimization
    double cur_f = fcn(x);
    double prev_f;
    do
    {
        // old f(x)
        prev_f = cur_f;

        // compute gradient and move
        g = grad(x);
        std::transform(
                steps.begin(),
                steps.end(),
                g.begin(),
                exg.begin(),
                std::multiplies<double>());

        // move model
        move_params(x, exg, adapter);

        // new f(x)
        cur_f = fcn(x);
    }
    while(cur_f > prev_f);

    std::transform(exg.begin(), exg.end(), exg.begin(), std::negate<double>());
    move_params(x, exg, adapter);
}

/**
 * @brief   Maximizes a function using a simple gradient ascent method.
 *
 * @param   x       Current value of variable.
 *
 * @param   fcn     Function of interest. Must receive a const kjb::Vector&
 *                  and return a double. Its type can be anything that
 *                  is callablle with the correct signature.
 *
 * @param   steps   Step sizes in each dimension.
 *
 * @param   grad    Gradient operator. Takes a const kjb::Vector& and returns
 *                  a kjb::Vector.
 */
template<class F, class V, class G>
inline
void gradient_ascent
(
    const F& fcn,
    V& x,
    const std::vector<double>& steps,
    const G& grad
)
{
    gradient_ascent(fcn, x, steps, grad, Vector_adapter<V>());
}

/**
 * @brief   Refine the maximum of a function 
 *
 * @param   x       Current value of model.
 *
 * @param   fcn     Function of interest. Must receive a const Model&
 *                  and return a double. Its type can be anything that
 *                  is callablle with the correct signature.
 *
 * @param   steps   Step sizes in each dimension.
 *
 * @param   adapter Allows us to treat our model as a vector. Must have
 *                  A::get, A::set, and A::size implemented. See
 *                  kjb::Vector_adapter for an example.
 */
template<class F, class M, class A>
void refine_max
(
    const F& fcn,
    M& x, 
    const std::vector<double>& steps,
    const A& adapter
)
{
    double cur_pt = fcn(x);
    bool at_max = false;
    while(!at_max)
    {
        at_max = true;
        for(size_t i = 0; i < adapter.size(&x); i++)
        {
            double xi = adapter.get(&x, i);
            // move right
            move_param(x, i, steps[i], adapter);
            double right_pt = fcn(x);

            // move left 
            move_param(x, i, -2.0*steps[i], adapter);
            double left_pt = fcn(x);

            // move back
            adapter.set(&x, i, xi);
            if(cur_pt >= left_pt && cur_pt >= right_pt) continue;

            at_max = false;
            if(left_pt > right_pt)
            {
                move_param(x, i, -steps[i], adapter);
                cur_pt = left_pt;
            }
            else
            {
                move_param(x, i, steps[i], adapter);
                cur_pt = right_pt;
            }
        }
    }
}


} //namespace kjb

#endif /*DIFF_OPTIM_H */


