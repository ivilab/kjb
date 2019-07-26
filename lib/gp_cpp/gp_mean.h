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

/* $Id: gp_mean.h 20741 2016-07-04 18:43:54Z jguan1 $ */

#ifndef GP_MEAN_H_INCLUDED
#define GP_MEAN_H_INCLUDED

#include "gp_cpp/gp_base.h"
#include "m_cpp/m_vector.h"
#include "l_cpp/l_exception.h"
#include <algorithm>
#include <iterator>
#include <utility>
#include <map>
#include <boost/bind.hpp>
#include <boost/function.hpp>

namespace kjb {
namespace gp {

/**
 * @brief   Class that represents the constant mean function.
 */
class Constant
{
public:
    /** @brief  Construct with constant of c. */
    Constant(double c) : m_c(c) {}

    /** @brief  Apply this function to an input. */
    double operator()(const Vector& /*x1*/) const
    {
        return m_c;
    }

    /** @brief   Return the value of the constant */
    double value() const { return m_c; };

private:
    double m_c;
};

/**
 * @brief   Class that represents the zero mean function. Not realy necessary,
 *          but need it for backward-compatibility.
 */
class Zero : public Constant
{
public:
    /** @brief  Ctor. */
    Zero() : Constant(0) {}
};

/**
 * @class   Manual
 * @brief   Represents a mean where you specify the function as pairs.
 */
class Manual
{
public:
    /** @brief  Ctor. */
    Manual(const Inputs& X, const Vector& f)
    {
        IFT(X.size() == f.size(), Illegal_argument, 
            "Cannot create mean function;" 
            "inputs and outputs have different dimensionality.");

        for(size_t i = 0; i < X.size(); ++i)
            func_.insert(std::pair<Vector, double>(X[i], f[i]));
//        std::transform(
//                    X.begin(),
//                    X.end(),
//                    f.begin(),
//                    std::inserter(func_, func_.begin()),
//                    boost::bind(std::make_pair<const Vector, double>, _1, _2));
    }

    /** @brief  Apply this function to an input. */
    double operator()(const Vector& x) const
    {
        std::map<Vector, double>::const_iterator dbl_p = func_.find(x);

        IFT(dbl_p != func_.end(), Runtime_error,
            "Cannot compute mean; input not known.");

        return dbl_p->second;
    }

    void add(const Vector& input, double value)
    {
        func_[input] = value;
    }

private:
    std::map<Vector, double> func_;
};

/**
 * @brief   Adapts a real function into a mean function. That is, this class
 *          should be used when one has a function which receives a real
 *          (double, int, etc.), which is interpreted as a one-dimensional
 *          input.
 */
template<class Real>
struct Real_function_adapter
{
    typedef boost::function1<double, Real> Real_func;

    template<class RealFunc>
    Real_function_adapter(const RealFunc& func) : func_(func) {}

    double operator()(const Vector& x) const
    {
        return func_(x[0]);
    }

    Real_func func_;
};

}} //namespace kjb::gp

#endif /*GP_MEAN_H_INCLUDED */

