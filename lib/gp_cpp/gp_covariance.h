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

/* $Id: gp_covariance.h 20824 2016-08-31 16:19:00Z jguan1 $ */

#ifndef GP_COVARIANCE_H_INCLUDED
#define GP_COVARIANCE_H_INCLUDED

#include "m_cpp/m_vector.h"
#include "m_cpp/m_matrix.h"
#include <cmath>

namespace kjb {
namespace gp {

/**
 * @brief   Class that represents the squared exponential covariance function.
 */
class Squared_exponential
{
public:
    /**
     * @brief   Construct this object.
     *
     * @param   scales  Scale parameter in each dimension.
     */
    Squared_exponential
    (
        double scale,
        double signal_sigma
    ) :
        m_scale(scale),
        m_signal_sigma(signal_sigma)
    {
        IFT(m_scale > 0 && m_signal_sigma > 0, kjb::Illegal_argument,
            "GP scale and signal variance must be positive.");
    }

    /**
     * @brief   Returns the scale parameter in each dimension.
     */
    double scale() const
    {
        return m_scale;
    }

    /**
     * @brief   Returns the signal sigma parameter in each dimension.
     */
    double signal_sigma() const
    {
        return m_signal_sigma;
    }

    /**
     * @brief   Apply this function to two sets of points.
     */
    double operator()(const Vector& x1, const Vector& x2) const
    {
        double dst_sq = vector_distance_squared(x1, x2);
        return m_signal_sigma * std::exp(-dst_sq / (2.0 * m_scale * m_scale));
    }

private:
    double m_scale;
    double m_signal_sigma;
};

// "Squared_exponential" is too long
typedef Squared_exponential Sqex;

/**
 * @brief   Apply covariance function to pair of sets of inputs. Specialized
 *          version for Squared_exponential kernel, which is known to create
 *          non-positive-definite matrices.
 */
template<class InIt>
inline
Matrix apply_cf(const Squared_exponential& cf, InIt first, InIt last)
{
    double l = cf.scale();
    double s = cf.signal_sigma();
    double c = s * exp(-1.0/(l*l));
    double d = s - c;
    //double ns = 1e-3 * 1e-3;
    double ns = 1e-2 * 1e-2;
    return apply_cf_noise(cf, first, last, ns * d);
}

}} //namespace kjb::gp

#endif /*GP_COVARIANCE_H_INCLUDED */

