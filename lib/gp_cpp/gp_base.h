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

/* $Id: gp_base.h 21596 2017-07-30 23:33:36Z kobus $ */

#ifndef GP_BASE_H_INCLUDED
#define GP_BASE_H_INCLUDED

#include "l/l_sys_debug.h"   /* For ASSERT */
#include "m_cpp/m_matrix.h"
#include "m_cpp/m_vector.h"
#include "l_cpp/l_functors.h"

#include <vector>
#include <algorithm>
#include <boost/bind.hpp>

namespace kjb {
namespace gp {

// useful typedef
typedef std::vector<Vector> Inputs;

/**
 * @brief   Convenience function to generate a set of consecutive set of
 *          inputs with the given step. Even though this is a one-liner,
 *          it is not a simple one-liner and it's worth having this simplified
 *          version. This is the 1D version.
 */
inline
Inputs make_inputs(double s, double e, double step = 1)
{
    const size_t N = 1 + (e - s)/step;
    Inputs ins(N, Vector(1));
    std::for_each(
            ins.begin(),
            ins.end(),
            boost::bind(
                &Vector::set,
                _1,
                boost::bind(Increase_by<double>(s, step))));

    ASSERT(ins.front()[0] <= s);
    ASSERT(ins.back()[0] <= e);

    return ins;
}

/** @brief  Apply mean function to set of inputs. */
template<class Mean, class InIt>
inline
Vector apply_mf(const Mean& mf, InIt first, InIt last)
{
    const size_t N = std::distance(first, last);

    Vector mu(N);
    std::transform(first, last, mu.begin(), mf);

    return mu;
}

/** @brief  Apply covariance function to pair of sets of inputs. */
template<class Covariance, class InIt>
Matrix apply_cf
(
    const Covariance& cf,
    InIt first1,
    InIt last1,
    InIt first2,
    InIt last2
)
{
    const size_t N = std::distance(first1, last1);
    const size_t M = std::distance(first2, last2);

    Matrix K(N, M);

    size_t i = 0;
    for(InIt p = first1; p != last1; p++, i++)
    {
        size_t j = 0;
        for(InIt q = first2; q != last2; q++, j++)
        {
            K(i, j) = cf(*p, *q);
        }
    }

    return K;
}

/** @brief  Apply covariance function to pair of sets of inputs. */
template<class Covariance, class InIt>
inline
Matrix apply_cf(const Covariance& cf, InIt first, InIt last)
{
    return apply_cf(cf, first, last, first, last);
}

/**
 * @brief   Apply covariance function to pair of sets of inputs. This version
 *          adds noise to the diagonal in order to reduce chance of non-
 *          positive-definiteness due to numerical instability.
 */
template<class Covariance, class InIt>
Matrix apply_cf_noise(const Covariance& cf, InIt first, InIt last, double ns)
{
    //Matrix K = apply_cf(cf, first, last);
    Matrix K = apply_cf(cf, first, last, first, last);
    ASSERT(K.get_num_rows() == K.get_num_cols());

    for(size_t i = 0; i < K.get_num_rows(); ++i)
    {
        K(i, i) += ns;
    }

    return K;
}

}} //namespace kjb::gp

#endif /*GP_BASE_H_INCLUDED */

