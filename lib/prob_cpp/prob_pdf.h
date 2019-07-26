/* ======================================================================== *
 |                                                                          |
 | Copyright (c) 2007-2010, by members of University of Arizona Computer    |
 | Vision group (the authors) including                                     |
 |       Kobus Barnard, Kyle Simek, Ernesto Brau.                           |
 |                                                                          |
 | For use outside the University of Arizona Computer Vision group please   |
 | contact Kobus Barnard.                                                   |
 |                                                                          |
 * ======================================================================== */

/* $Id: prob_pdf.h 21596 2017-07-30 23:33:36Z kobus $ */

#ifndef PROB_PDF_H_INCLUDED
#define PROB_PDF_H_INCLUDED

/** @file
 *
 * @author Kobus Barnard
 * @author Ernesto Brau
 *
 * @brief PDFs and CDFs for the different distributions defined in
 * "prob_distribution.h".
 *
 * The pdf and cdf functions supplied by
 * boost are used when possible. This file contains pdf and cdf
 * functionality for the distributions written by us (i.e., those
 * that are not present in the boost library.
 */

#include "l/l_sys_debug.h"  /* For ASSERT. */
#include "m_cpp/m_special.h"
#include "prob_cpp/prob_distribution.h"

namespace kjb {

/*============================================================================
  pdf functions; notice that we import all boost pdfs here as well
  ----------------------------------------------------------------------------*/

// importing the boost::math::pdf into this namespace so that we
// can call the boost ones using the kjb:: prefix.
using boost::math::pdf;

#if BOOST_VERSION < 104600
/**
 * @brief   Computes the PMF of a geometric distribution at x.
 */
inline double pdf(const Geometric_distribution& dist, double x)
{
    return boost::math::pdf<boost::math::negative_binomial, double>(dist, x);
}
#endif


/**
 * @brief   Computes the PDF of a log-normal distribution at x.
 */
inline double pdf(const Log_normal_distribution& dist, double x)
{
    return pdf(dist.dist_, log(x)) / x;
}

/**
 * @brief   Computes the PMF of a categorical distribution at x.
 */
template<class T>
inline
double pdf(const Categorical_distribution<T>& dist, const T& x)
{
    typename std::map<T, double>::const_iterator i = dist.db.find(x);
    return i == dist.db.end() ? 0.0 : i->second/dist.total_weight_;
}

/**
 * @brief   Computes the joint PDF of a multivariate Gaussian at x.
 */
double pdf(const MV_gaussian_distribution& P, const Vector& x);

/**
 * @brief   Computes the PDF/PMF of a mixture distribution at x.
 */
template<class Distribution>
double pdf
(
    const Mixture_distribution<Distribution>& dist,
    const typename
        Distribution_traits<Mixture_distribution<Distribution> >::type& x
)
{
    double f = 0.0;
    for(size_t i = 0; i < dist.dists.size(); i++)
    {
        f += (pdf(dist.coeffs, i + 1) * pdf(dist.dists[i], x));
    }

    return f;
}


/**
 * @brief Compute pdf of a Gaussian distribution.  
 */
inline double pdf(const Gaussian_distribution& P, double x)
{
    return boost::math::pdf(P, x);
}

// forward declaration
inline double log_pdf(const Beta_binomial_distribution& dist, double k);

// Evaluat the probability density function of a beta bionmial distribution
inline double pdf(const Beta_binomial_distribution& dist, double k)
{
    return exp(log_pdf(dist, k));
}

// forward declaration
template <size_t D> 
inline double log_pdf(const Von_mises_fisher_distribution<D>& dist, const kjb::Vector_d<D>& v);

/// Evaluate the probability density function of a Von-mises Fisher distribtuion (x's magnitude is ignored/assumed 1.0)
template <size_t D>
inline 
double pdf(const Von_mises_fisher_distribution<D>& dist, const kjb::Vector_d<D>& v)
{
    return exp(log_pdf(dist, v));
}

// forward declaration
template <size_t D>
inline double log_pdf(const Uniform_sphere_distribution<D>& P, const kjb::Vector_d<D>& x);

/// Evaluate the probability density function of a uniform sphere distribution (x's magnitude is ignored / assumed 1.0)
template <size_t D>
inline double pdf(const Uniform_sphere_distribution<D>& P, const kjb::Vector_d<D>& x)
{
    return exp(log_pdf(P,x));
}

/*============================================================================
  cdf functions; notice that we import all boost cdfs here as well
  ----------------------------------------------------------------------------*/

// importing the boost::math::cdf into this namespace so that we
// can call the boost ones using the kjb:: prefix.
using boost::math::cdf;

/**
 * @brief   Computes the cdf of a log-normal distribution at x.
 */
inline double cdf(const Log_normal_distribution& dist, double x)
{
    return cdf(dist.dist_, log(x));
}

/**
 * @brief   Computes the CDF of a categorical distribution at x.
 */
template<class T>
double cdf(const Categorical_distribution<T>& dist, const T& x)
{
    // TODO: compute this in O(log n) time using the precomputed cdf_ vector and std::upper_bound.  This code should just work, but no time to test right now:
    // Update: actually, this probably won't work, because the CDF isn't guaranteed to be stored in key order.  this needs some thought, but getting a proper O(log n) algorithm shouldn't be that hard...  -ksimek, Aug 13, 2013
//    typedef Categorical_distribution<T> Cd;
//    return *--std::upper_bound(
//            dist.cdf_.begin(),
//            dist.cdf_.end(),
//            x,
//            Cd::cdf_key_less_than_scalar());

    // compute CDF on-the-fly in O(n) time.
    double F = 0.0;
    typename std::map<T, double>::const_iterator y = dist.db.upper_bound(x);
    for(typename std::map<T, double>::const_iterator i = dist.db.begin();
                                                              i != y; i++)
    {
        F += i->second / dist.total_weight_;
    }

    return F;
}

/**
 * @brief   Computes the CDF of a mixture distribution at x.
 */
template<class Distribution>
double cdf
(
    const Mixture_distribution<Distribution>& dist,
    const typename
        Distribution_traits<Mixture_distribution<Distribution> >::type& x
)
{
    double f = 0.0;
    for(size_t i = 0; i < dist.dists.size(); i++)
    {
        f += (cdf(dist.coeffs, i + 1) * cdf(dist.dists[i], x));
    }

    return f;
}

/*============================================================================
  quantile functions; notice that we import all boost quantile here as well
  ----------------------------------------------------------------------------*/

using boost::math::quantile;

/*============================================================================
  log-pdf functions; notice the generic one...
  ----------------------------------------------------------------------------*/

/**
 * @brief   Computes the log PDF of a distribution at x.
 *
 * This simply returns log(pdf(P, x)); please specialize for best
 * results.
 */
template<class Distribution>
inline double log_pdf
(
    const Distribution& P,
    const typename Distribution_traits<Distribution>::type& x
)
{
    return std::log(pdf(P, x));
}

/**
 * @brief Computes the log PDF of a Beta-binomial distribution at k.
 */
inline double log_pdf(const Beta_binomial_distribution& dist, double k)
{
    const double lcoeff = log_binomial_coefficient(dist.n, k);
    const double na = k + dist.alpha;
    const double nb = dist.n - k + dist.beta;

    const double lnumer = lgamma(na) + lgamma(nb) - lgamma(na+nb);
    const double ldenom = lgamma(dist.alpha) + lgamma(dist.beta) - lgamma(dist.alpha + dist.beta);

    return lcoeff + lnumer - ldenom;
}

/**
 * @brief   Computes the log PDF a normal distribution at x.
 *
 * This is a specialization of the generic log_pdf function. It is
 * specialized to avoid the computation of the log of the exponential.
 */
inline double log_pdf(const Gaussian_distribution& P, double x)
{
    double mu = P.mean();
    double variance = P.standard_deviation() * P.standard_deviation();

    return -(0.5 * log(2 * M_PI  * variance))
                - (((x - mu) * (x - mu)) / (2 * variance));
}

/**
 * @brief   Computes the log PDF a Laplace distribution at x.
 *
 * This is a specialization of the generic log_pdf function. It is
 * specialized to avoid the computation of the log of the exponential.
 */
inline double log_pdf(const Laplace_distribution& P, double x)
{
    double mu = P.location();
    double b = P.scale();

    return -log(2 * b) - (fabs(x - mu) / b);
}

/**
 * @brief   Computes the log PDF a normal distribution at x.
 *
 * This is a specialization of the generic log_pdf function. It is
 * specialized to avoid the computation of the log of the exponential.
 */
inline double log_pdf(const Binomial_distribution& P, double k)
{
    const double p = P.success_fraction();
    const double n = P.trials();
    double result = log_binomial_coefficient(n, k);
    result += k * log(p) + (n - k) * log(1-p);
    return result;
}

/**
 * @brief   Computes the log PDF a multivariate normal distribution at x.
 *
 * This is a specialization of the generic joint_log_pdf function. It is
 * specialized to avoid the computation of the log of the exponential.
 */
double log_pdf(const MV_gaussian_distribution& P, const Vector& x);


/**
 * @brief Computes the log PDF of a uniform distribution over the surface of the unit
 * sphere in D-dimensional Euclidean space.  The density is given in D-1 dimensional euclidean units.
 *
 * It returns 1/S, where S is the surface area of a D-dimensional unit-sphere
 */
template <size_t D>
inline double log_pdf(const Uniform_sphere_distribution<D>& /*dist*/, const kjb::Vector_d<D>& /*v*/)
{
    return -log(kjb_c::unit_sphere_surface_area(D));
}


template <size_t D>
double log_pdf(const Von_mises_fisher_distribution<D>& dist, const kjb::Vector_d<D>& v)
{
    static const double LOG_2_PI = log(2*M_PI);

    double kappa_ = dist.kappa();
    const kjb::Vector_d<D>& mu_ = dist.mu();

    double log_exponential = kappa_ * dot(v.normalized(), mu_);
    if(D == 3)
    {
        //TODO:  compute normalization inside constructor
        double log_normalization = log(kappa_) - (LOG_2_PI + log(exp(kappa_) - exp(-kappa_)));

#ifdef TEST
        // math check - implement normalization in 3 other ways
        double test_1 = log(kappa_ / (2 * M_PI * (exp(kappa_) - exp(-kappa_))));
        double test_2 = log(kappa_ / (4* M_PI * std::sinh(kappa_)));
        // general form
        double test_3 = (D/2.0 - 1) * log(kappa_) - (D/2.0)*log(2 * M_PI) - log(boost::math::cyl_bessel_i(D/2.0-1, kappa_));

        // THESE ALL MATCH AS OF May 18, 2012
        ASSERT(fabs(log_normalization - test_1) < FLT_EPSILON);
        ASSERT(fabs(log_normalization - test_2) < FLT_EPSILON);
        ASSERT(fabs(log_normalization - test_3) < FLT_EPSILON);
#endif
        return log_exponential + log_normalization;
    }
    else
    {
        //KJB_THROW(kjb::Not_implemented);

        // THIS SHOULD BE RIGHT AND IS TESTED FOR THE D==3 case:
        double log_normalization = (D/2.0 - 1) * log(kappa_) - (D/2.0)*log(2 * M_PI) - log(boost::math::cyl_bessel_i(D/2.0-1, kappa_));
        return log_exponential + log_normalization;
        // implementing this will be easy once the 3D version is confirmed:
        // just use the general form of the normalization factor (test_3 above)
        // and use the same equation for the log_exponential
    }
}

/** @brief  PDF of the CPR. */
double pdf
(
    const Chinese_restaurant_process& cpr,
    const Chinese_restaurant_process::Type& B
);

/** @brief  log-PDF of the CPR. */
double log_pdf
(
    const Chinese_restaurant_process& cpr,
    const Chinese_restaurant_process::Type& B
);

/** @brief  PDF of the Dirichlet distribution. */
double pdf(const Dirichlet_distribution& dist, const Vector& vals);

/** @brief  log-PDF of the Dirichlet distribution. */
double log_pdf(const Dirichlet_distribution& dist, const Vector& vals);

/** @brief  PDF of the Wishart distribution. */
double pdf(const Wishart_distribution& dist, const Vector& vals);

/** @brief  PDF of the Wishart distribution. */
double log_pdf(const Wishart_distribution& dist, const Vector& vals);

/** @brief  PDF of the inverse Wishart distribution. */
double pdf(const Inverse_wishart_distribution& dist, const Vector& vals);

/** @brief  PDF of the inverse Wishart distribution. */
double log_pdf(const Inverse_wishart_distribution& dist, const Vector& vals);

/** @brief  PDF of the Normal inverse gamma distribution. */
inline double pdf
(
     const Normal_inverse_gamma_distribution& dist, 
     const Vector& mean,
     double variance
)
{
    dist.normal_.set_covariance_matrix(variance * dist.covariance_);
    return pdf(dist.normal_, mean) * pdf(dist.ig_, variance);
}

/** @brief  log-PDF of the Normal inverse gamma distribution. */
inline double log_pdf
(
     const Normal_inverse_gamma_distribution& dist, 
     const Vector& mean,
     double variance
)
{
    dist.normal_.set_covariance_matrix(variance * dist.covariance_);
    return log_pdf(dist.normal_, mean) + log_pdf(dist.ig_, variance);
}

/** @brief  PDF of the Normal inverse chi-squared distribution. */
inline double pdf
(
     const Normal_inverse_chi_squared_distribution& dist, 
     const Vector& mean,
     double variance
)
{
    dist.normal_.set_covariance_matrix(variance * dist.covariance_);
    return pdf(dist.normal_, mean) * pdf(dist.ics_, variance);
}

/** @brief  log-PDF of the Normal inverse chi_squared distribution. */
inline double log_pdf
(
     const Normal_inverse_chi_squared_distribution& dist, 
     const Vector& mean,
     double variance
)
{
    dist.normal_.set_covariance_matrix(variance * dist.covariance_);
    return log_pdf(dist.normal_, mean) + log_pdf(dist.ics_, variance);
}

/** @brief  PDF of the Normal inverse Wishart distribution. */
inline double pdf
(
     const Normal_inverse_wishart_distribution& dist, 
     const Vector& mean,
     const Matrix& sigma
)
{
    dist.normal_.set_covariance_matrix(sigma / dist.kappa_);
    return pdf(dist.normal_, mean) * pdf(dist.iw_, sigma);
}

/** @brief  log-PDF of the Normal inverse Wishart distribution. */
inline double log_pdf
(
     const Normal_inverse_wishart_distribution& dist, 
     const Vector& mean,
     const Matrix& sigma
)
{
    dist.normal_.set_covariance_matrix(sigma / dist.kappa_);
    return log_pdf(dist.normal_, mean) + log_pdf(dist.iw_, sigma);
}

} //namespace kjb

#endif /*PROB_PDF_H_INCLUDED */

