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

#ifndef DIFF_ADAPTER_H
#define DIFF_ADAPTER_H

#include <l_cpp/l_exception.h>
#include <iostream>
#include <vector>
#include <algorithm>
#include <utility>

namespace kjb {

/** @brief  Default adapter for the hessian function.
 *
 * This class makes any model type that behaves as a vector usable with
 * the hessian function. A model must implement operator[] for both const
 * and non const, and the size() member function.
 *
 * If your model does not behave this way, you must write your own adapter
 * which provides the same functionality.
 */
template<class Vec>
class Vector_adapter
{
public:
    typedef Vec Model_type;

public:
    /** @brief  Returns the ith element of x; i.e., x[i]. */
    double get(const Vec* x, size_t i) const
    {
        return (*x)[i];
    }

    /** @brief  Sets the ith element of x; i.e., x[i] = v. */
    void set(Vec* x, size_t i, double v) const
    {
        (*x)[i] = v;
    }

    /** @brief  Sets the ith element of x; i.e., x[i] = v. */
    void set(Vec* x, size_t i, size_t j, double v, double w) const
    {
        if(i == j)
        {
            std::cerr << "WARNING: setting same dimension to two "
                      << "different values."
                      << std::endl;
        }

        set(x, i, v);
        set(x, j, w);
    }

    /** @brief  Sets the all elements of x; i.e., x = v. */
    void set(Vec* x, const Vec& v) const;

    /** @brief  Returns the size of x; i.e., x.size(). */
    size_t size(const Vec* x) const
    {
        return x->size();
    }
};

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

template<class Vec>
void Vector_adapter<Vec>::set(Vec* x, const Vec& v) const
{
    const size_t D = x->size();
    IFT(D == v.size(), Illegal_argument,
        "cannot set model to vector: dimension must match.");

    for(size_t i = 0; i < D; i++)
    {
        set(x, i, v[i]);
    }
}

/** @brief  Helper function that moves a parameter by an amount. */
template<class Model, class Adapter>
inline
void move_param(Model& x, size_t i, double dv, const Adapter& aptr)
{
    aptr.set(&x, i, aptr.get(&x, i) + dv);
}

/** @brief  Helper function that moves a pair of parameters by an amount. */
template<class Model, class Adapter>
inline
void move_params
(
    Model& x,
    size_t i,
    size_t j,
    double dv,
    double dw,
    const Adapter& aptr
)
{
    aptr.set(&x, i, j, aptr.get(&x, i) + dv, aptr.get(&x, j) + dw);
}

/** @brief  Helper function that moves all parameters by specified vector. */
template<class Model, class Vec, class Adapter>
void move_params(Model& x, const Vec& dx, const Adapter& aptr)
{
    const size_t D = aptr.size(&x);
    IFT(D == dx.size(), Illegal_argument,
        "cannot move model by vector: dimension must match.");

    Vec vx(D);
    for(size_t i = 0; i < D; i++)
    {
        vx[i] = aptr.get(&x, i) + dx[i];
    }

    aptr.set(&x, vx);
}

/**
 * @brief   Gets the next point in a N-dimensional grid.
 */
template<class M, class A>
bool next_point
(
    const std::vector<std::pair<double, double> >& bounds,
    const std::vector<double>& widths,
    size_t nbins,
    std::vector<size_t>& indices,
    M& x,
    const A& adapter
)
{
    using namespace std;

    size_t D = bounds.size();

    // first time called
    if(indices.empty())
    {
        indices.resize(D, 0);
        for(size_t d = 0; d < D; d++)
        {
            //x[d] = bounds[d].first + widths[d]/2;
            adapter.set(&x, d, bounds[d].first + widths[d]/2);
        }

        return true;
    }

    // find element to change
    vector<size_t>::reverse_iterator s_p
        = find_if(indices.rbegin(), indices.rend(),
                  bind2nd(not_equal_to<size_t>(), nbins - 1));

    if(s_p == indices.rend())
    {
        return false;
    }

    // increment and reset everything after
    (*s_p)++;
    fill(indices.rbegin(), s_p, 0);

    // find corresponding point index
    size_t i = D - 1 - std::distance(indices.rbegin(), s_p);
    //x[i] += widths[i];
    move_param(x, i, widths[i], adapter);
    for(size_t d = i + 1; d < D; d++)
    {
        //x[d] = bounds[d].first + widths[d]/2;
        adapter.set(&x, d, bounds[d].first + widths[d]/2);
    }

    return true;
}

} //namespace kjb

#endif /*DIFF_ADAPTER_H */


