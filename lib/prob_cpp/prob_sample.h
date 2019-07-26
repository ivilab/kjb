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

/* $Id: prob_sample.h 21596 2017-07-30 23:33:36Z kobus $ */

#ifndef PROB_SAMPLE_H_INCLUDED
#define PROB_SAMPLE_H_INCLUDED

/** @file
 *
 * @author Kobus Barnard
 * @author Ernesto Brau
 * @author Jinyan Guan
 *
 * @brief Sampling functionality for the different distributions defined in
 * "prob_distributions.h".
 *
 * This relies heavily on the boost::random library.
 */

#include "l/l_sys_debug.h"  /* For ASSERT. */
#include "prob_cpp/prob_distribution.h"
#include "prob_cpp/prob_pdf.h"
#include "m_cpp/m_vector.h"
#include "m_cpp/m_vector_d.h"
#include "m_cpp/m_matrix_d.h"
#include "g_cpp/g_util.h"
#include "l_cpp/l_util.h"

#include <boost/random.hpp>
#include <algorithm>
#include <vector>

namespace kjb {

/// Random seed used to initialize the random number generator
extern const unsigned int DEFAULT_SEED;

// Basic random generator -- used for sampling -- belongs to the cpp file
//typedef boost::minstd_rand Base_generator_type;
typedef boost::mt19937 Base_generator_type;
extern Base_generator_type basic_rnd_gen;
//extern boost::uniform_01<Base_generator_type> uni01;

/* /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ */

/**
 * @brief   Seed random number generator
 */
inline void seed_sampling_rand(unsigned int x0)
{
    basic_rnd_gen.seed(x0);
//    uni01 = boost::uniform_01<Base_generator_type>(basic_rnd_gen);
}

/**
 * @brief   Sample from a uniform distribution
 */
inline double sample(const Uniform_distribution& dist)
{
    typedef boost::uniform_real<> Distribution_type;
    typedef boost::variate_generator<Base_generator_type&, Distribution_type> Rng;
    Rng rng(basic_rnd_gen, Distribution_type(dist.lower(), dist.upper()));
    return rng();
}

/**
 * @brief   Template sample function.
 *
 * This uses the probability transform to sample from the given
 * distribution. The only requirement for this function is that
 * the quantile() function be defined for the distribution.
 
 * It is recommended to directly sample from a
 * distribution in cases where it is more efficient to do so
 * by specializing the sample function (as is already done for
 * some distributions).
 */
template<class Distro>
inline double sample(const Distro& dist)
{
    // this sometimes results in infinite recursion inside of boost.  No ideas :-(
    double u = sample(Uniform_distribution());
    return quantile(dist, u);
}

/**
 * @brief   Sample from a Bernoulli distribution
 */
inline double sample(const Bernoulli_distribution& dist)
{
    typedef boost::bernoulli_distribution<> Distribution_type;
    typedef boost::variate_generator<Base_generator_type&, Distribution_type> Rng;

    Rng rng(basic_rnd_gen, Distribution_type(dist.success_fraction()));
    return rng();
}

/**
 * @brief   Sample from a Binomial distribution
 */
inline double sample(const Binomial_distribution& dist)
{
    typedef boost::binomial_distribution<> Distribution_type;
    typedef boost::variate_generator<Base_generator_type&, Distribution_type> Rng;

    Rng rng(basic_rnd_gen, Distribution_type(dist.trials(), dist.success_fraction()));
    return rng();
}

/**
 * @brief   Sample from a Chi-squared distribution
 */
inline double sample(const Chi_square_distribution& dist)
{
    typedef boost::gamma_distribution<> Distribution_type;
    typedef boost::variate_generator<Base_generator_type&, Distribution_type> Rng;

    // if X ~ Gamma(v/2, 2), then X ~ Chi-squared(v) from
    // https://en.wikipedia.org/wiki/Gamma_distribution
    Rng rng(basic_rnd_gen, Distribution_type(dist.degrees_of_freedom()/2.0, 2.0));
    return rng();
}

/**
 * @brief   Sample from a exponential distribution
 */
inline double sample(const Exponential_distribution& dist)
{
    typedef boost::exponential_distribution<> Distribution_type;
    typedef boost::variate_generator<Base_generator_type&, Distribution_type> Rng;

    Rng rng(basic_rnd_gen, Distribution_type(dist.lambda()));
    return rng();
}

/**
 * @brief   Sample from a Gaussian distribution
 */
inline double sample(const Gaussian_distribution& dist)
{
    typedef boost::normal_distribution<> Distribution_type;
    typedef boost::variate_generator<Base_generator_type&, Distribution_type> Rng;

    Rng rng(basic_rnd_gen, Distribution_type(dist.mean(), dist.standard_deviation()));
    return rng();
}

/**
 * @brief   Sample from a Poisson distribution
 */
inline double sample(const Poisson_distribution& dist)
{
    typedef boost::poisson_distribution<> Distribution_type;
    typedef boost::variate_generator<Base_generator_type&, Distribution_type> Rng;

    Rng rng(basic_rnd_gen, Distribution_type(dist.mean()));
    return rng();
}

/**
 * @brief   Sample from a Gamma distribution 
 */
inline double sample(const Gamma_distribution& dist)
{
    typedef boost::gamma_distribution<> Distribution_type;
    typedef boost::variate_generator<Base_generator_type&, Distribution_type> Rng;

    Rng rng(basic_rnd_gen, Distribution_type(dist.shape(), dist.scale()));
    return rng();
}

/**
 * @brief   Sample from a Inverse_gamma distribution 
 */
inline double sample(const Inverse_gamma_distribution& dist)
{
    typedef boost::gamma_distribution<> Distribution_type;
    typedef boost::variate_generator<Base_generator_type&, Distribution_type> Rng;

    Rng rng(basic_rnd_gen, Distribution_type(dist.shape(), 1.0/dist.scale()));
    return 1.0/rng();
}

/**
 * @brief   Sample from a Inverse_chi_squared distribution 
 */
inline double sample(const Inverse_chi_squared_distribution& dist)
{
    // Acoording to
    // https://en.wikipedia.org/wiki/Scaled_inverse_chi-squared_distribution
    // if X ~ scaled-inv-chi-squared(nu, t^2),
    // then X ~ inv-gamma(nu/2, nu * / t^2/2)
    double shape = dist.degrees_of_freedom() / 2.0;
    double scale = dist.degrees_of_freedom() * dist.scale() / 2.0;
    Inverse_gamma_distribution ig_dist(shape, scale);
    return sample(ig_dist);
}

#if BOOST_VERSION < 103400
inline double sample(const Geometric_distribution& dist)
{
#warning "sample(Geometric_distribution) not implemented in Boost prior to 1.034"
    KJB_THROW(Not_implemented);
}
#else
inline double sample(const Geometric_distribution& dist)
{
    // this might not be available in older boost versions but exactly which version it was added is kind of a pain.  
    // If you can't compile because this isn't available, look in /usr/local/boost/version.hpp and determine which version you have.
#if BOOST_VERSION >= 104700
    double dist_param = dist.success_fraction();
#else /*  BOOST_VERSION >= 103400 */
    KJB(UNTESTED_CODE());
    double dist_param = 1.0-dist.success_fraction();
#endif

    typedef boost::geometric_distribution<> Distribution_type;
    typedef boost::variate_generator<Base_generator_type&, Distribution_type> Rng;

    Rng rng(basic_rnd_gen, Distribution_type(dist_param));
    return rng();
}
#endif /* BOOST_VERSION < 10340 */

/**
 * @brief   Sample from a categorical distribution
 */
template<class T>
T sample(const Categorical_distribution<T>& dist)
{
    // TODO: This is currently O(log n), but approaches exist that
    // make this constant, see the Alias Table methods described here:
    // http://www.keithschwarz.com/darts-dice-coins/
    // Also boost's discrete_distribution implements most of what
    // this class does, so we could replace this with that.
    double u = sample(Uniform_distribution());

    using namespace boost;

    if(dist.cdf_.empty())
    {
        KJB_THROW_2(Runtime_error, "Attempting to sample from an empty set.");
    }

    ASSERT(dist.cdf_.size() == dist.db.size()); // init_cdf() was called
    double total_weight = dist.cdf_.back().second;
    u *= total_weight;

    typedef Categorical_distribution<T> Cd;
    typedef typename Cd::Cdf_pair Cdf_pair;
    typedef typename std::vector<Cdf_pair>::const_iterator Const_iterator;

    // use binary search to find first element in CDF that exceeds u
    Const_iterator r = 
        std::lower_bound(
            dist.cdf_.begin(),
            dist.cdf_.end(),
            u,
            typename Cd::cdf_pair_less_than_scalar());

    ASSERT(r != dist.cdf_.end());

    return *r->first;

}

/**
 * @brief   Pick an element uniformly at random (UAR) from a sequence,
 *          represented by a beginning iterator and a size.
 *
 * @return  An iterator to the randomly chosen element.
 */
template<class Iterator, class distance_type>
inline
Iterator element_uar(Iterator first, distance_type size)
{
    int dist = kjb::sample(kjb::Categorical_distribution<size_t>(0, size - 1, 1));
    Iterator p = first;
    std::advance(p, dist);

    return p;
}

/**
 * @brief   Sample from a multivariate normal distribution.
 */
Vector sample(const MV_gaussian_distribution& dist);

inline double sample(const Log_normal_distribution& dist)
{
    return exp(sample(dist.dist_));
}

/**
 * @brief   Sample from a mixture distribution
 */
template<class Distribution>
inline
typename Distribution_traits<Mixture_distribution<Distribution> >::type
    sample(const Mixture_distribution<Distribution>& dist)
{
    size_t i = sample(dist.coeffs);
    return sample(dist.dists[i - 1]);
}

/**
 * @brief   Sample uniformly from the surface of a unit-sphere in D-dimensional
 *          euclidean space
 */
template <size_t D> inline kjb::Vector_d<D> sample(const Uniform_sphere_distribution<D>& /* dist */)
{
    kjb::Vector_d<D> result;

    static const Gaussian_distribution norm_dist(0, 1);

    for(size_t d = 0; d < D; ++d)
    {
        result[d] = kjb::sample(norm_dist);
    }

    return result.normalize();
}


/**
 * Sample n i.i.d. D-dimensional unit vectors from a Von-mises fisher distribution.
 * 
 * @param it output iterator, stores a collection of kjb::Vector_d<D>
 */
template <size_t D, class Vector_d_iterator>
inline 
void sample(const Von_mises_fisher_distribution<D>& dist, size_t n, Vector_d_iterator it)
{
    size_t m = D;
    static const kjb::Vector_d<D> z_hat = kjb::create_unit_vector<D>(D-1);

    const Vector_d<D>& mu_ = dist.mu();
    double kappa_ = dist.kappa();

    // TODO: Quaternion may be slower than is needed here.  Consider
    // constructing the rotation matrix by hand or move q into a member variable
    // I replaced it with a function I wrote to find rotation matrices
    // between two directions in any dimension. I haven't tested it as
    // thoroughly as it needs, but I'm pretty confident it's correct.
    //                                       -- Ernesto [2014/10/28]
    //kjb::Quaternion q; q.set_from_directions(z_hat, mu_);
    Matrix_d<D, D> R = geometry::get_rotation_matrix(z_hat, mu_);

    double b = (-2*kappa_ + sqrt(4*kappa_*kappa_ + (m-1)*(m-1)))/(m-1);
    double x0 = (1-b)/(1+b);
    double c = kappa_*x0 + (m-1)*log(1-x0*x0);

    Uniform_sphere_distribution<D-1> subdir_dist;
    for(size_t i = 0; i < n; ++i)
    {
        double W, U;
        do
        {
            // rejection sampling for height on unit sphere,
            // distributed as f(w) = k * exp(kappa*w) ; for w \in [-1, 1], zero elsewhere
            // see Sungkyu Jung's treatment:.
            // http://www.stat.pitt.edu/sungkyu/software/randvonMisesFisher3.pdf
            double Z = kjb::sample(kjb::Beta_distribution((m-1)/2,(m-1)/2));
            W = (1-(1+b)*Z)/(1-(1-b)*Z);
            U = kjb::sample(kjb::Uniform_distribution());
        } while(kappa_*W + (m-1)*log(1-x0*W) - c < log(U));


        kjb::Vector_d<D-1> subdir = kjb::sample(subdir_dist);
        subdir *= sqrt(1- W*W);

        kjb::Vector_d<D> dir;
        std::copy(subdir.begin(), subdir.end(), dir.begin());
        dir.back() = W;

        //dir = q.rotate(dir);
        dir = R*dir;
        *it = dir;
        ++it;
    }
}

/**
 * Sample a D-dimensional unit vector from a Von-mises fisher distribution.
 */
template <size_t D>
Vector_d<D> sample(const Von_mises_fisher_distribution<D>& dist)
{
    Vector_d<D> result;
    sample(dist, 1, &result);
    return result;
}

/** @brief  Sample from a CRP. */
Crp::Type sample(const Chinese_restaurant_process& crp);

/**
 * @author Colin Dawson
 * @brief  Sample the number of occupied tables from a CRP (don't store the actual partition).
 */
    
size_t sample_occupied_tables(const Chinese_restaurant_process& crp);

/** 
 * @brief  Sample from a dirichlet distribution. 
 * 
 * Generate a sample from a Dirichlet distribution by using a random sample from
 * a gamma distribution, see page 14 of 
 * (http://mayagupta.org/publications/FrigyikKapilaGuptaIntroToDirichlet.pdf)
 * for details
 *
 * @author Jinyan Guan
 *
 */
Vector sample(const Dirichlet_distribution& dist);


/**
 * @brief  Sample from a standard Wishart distribution Wishart(dof, I).
 *         A helper function for generating samples from a Wishart distribution 
 * @param  dof    degree of freedom 
 * @param  dim    the dimension of the Scale matrix 
 * @param  C      the lower halp of the Cholesky decomposition of the Scale
 *                matrix 
 */
Matrix sample_standard_wishart(double dof, size_t dim, const Matrix& C);

/** 
 * @brief  Sample from a Wishart distribution. 
 *
 * Generate a sample from a Wishart distribution by using Bartlett decomposition, see 
 * http://www2.stat.duke.edu/~km68/materials/214.9%20(Wishart).pdf
 * for details. 
 * 
 * @author Jinyan Guan
 */
inline Matrix sample(const Wishart_distribution& dist)
{
    // construct the U matrix
    Matrix U = sample_standard_wishart(dist.nu_, dist.D_, dist.S_Cholesky_lower_);

    // return (UP')' UP' where PP' = S_
    return dist.S_Cholesky_lower_ * U.transpose() * 
           U * dist.S_Cholesky_lower_.transpose();
}

/** 
 * @brief  Sample from a Inverse Wishart distribution. 
 * 
 * Uses the method of generating samples from a Wishart distribution and the
 * following fact: 
 * If X ~ Wishart(Sigma, nu)
 * Then inv(X) ~ Inv-Wishart(inv(Sigma, nu)
 *
 * @author Jinyan Guan
 */
inline Matrix sample(const Inverse_wishart_distribution& dist)
{
    // construct the U matrix
    Matrix U = sample_standard_wishart(dist.nu_, dist.D_, dist.S_Cholesky_lower_);
    Matrix X = dist.S_Cholesky_lower_ * U.transpose() * 
               U * dist.S_Cholesky_lower_.transpose();
    return X.inverse();
}

/** 
 * @brief  Sample from a Normal-inverse-Wishart distribution
 *
 * @return std::pair<Vector, Matrix>(mean, sigma)
 * (mean, Sigma) ~ NIW(mu, kappa, S, nu) can be obtained as follows:
 * Sigma ~ IW(S, nu)
 * mean ~ Multivariate-Normal(mu, 1/kappa * Sigma)
 *
 *
 * @author Jinyan Guan
 */
inline std::pair<Vector, Matrix> sample(const Normal_inverse_wishart_distribution& dist)
{
    Matrix sigma = sample(dist.iw_);
    dist.normal_.set_covariance_matrix(sigma/dist.kappa_);
    Vector mean = sample(dist.normal_);
    return std::make_pair(mean, sigma);
}
    
/** 
 * @brief  Sample from a Normal-inverse-gamma distribution
 *
 * @return std::pair<Vector, Matrix>(mean, sigma)
 * (mean, Sigma) ~ NIG(mu, V, a, b) can be obtained as follows:
 * Sigma ~ IG(a, b)
 * mean ~ Multivariate-Normal(mu, Sigma * V)
 *
 *
 * @author Jinyan Guan
 */
inline std::pair<Vector, double> sample(const Normal_inverse_gamma_distribution& dist)
{
    double variance = sample(dist.ig_);
    dist.normal_.set_covariance_matrix(variance * dist.covariance_);
    Vector mean = sample(dist.normal_);
    return std::make_pair(mean, variance);
}

/** 
 * @brief  Sample from a Normal-inverse-chi-squared distribution
 *
 * @return std::pair<Vector, Matrix>(mean, sigma)
 * (mean, Sigma) ~ NICS(mu, V, a, b) can be obtained as follows:
 * Sigma ~ ICS(a, b)
 * mean ~ Multivariate-Normal(mu, Sigma * V)
 *
 *
 * @author Jinyan Guan
 */
inline std::pair<Vector, double> sample
(
    const Normal_inverse_chi_squared_distribution& dist
)
{
    double variance = sample(dist.ics_);
    dist.normal_.set_covariance_matrix(variance * dist.covariance_);
    Vector mean = sample(dist.normal_);
    return std::make_pair(mean, variance);
}

} //namespace kjb

#endif /*PROB_SAMPLE_H_INCLUDED */
