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

/* $Id: prob_conditional_distribution.h 17393 2014-08-23 20:19:14Z predoehl $ */

#ifndef PROB_CONDITIONAL_DISTRIBUTION_H_INCLUDED
#define PROB_CONDITIONAL_DISTRIBUTION_H_INCLUDED


/** @file
 *
 * @author Kobus Barnard
 * @author Ernesto Brau
 *
 * @brief Definition of various conditional distibutions
 *
 * These distributions are meant to represent conditional distributions,
 * where the type of the conditioned variables is supplied as a template
 * parameter.
 */

#include "prob_cpp/prob_distribution.h"
#include "prob_cpp/prob_pdf.h"
#include "prob_cpp/prob_sample.h"

namespace kjb {

/*==================================================================================================
  Declaration of conditional distribution classes. There could
  potentially be dozens of these, due to the fact that the
  dependencies are not predefined.
  ==================================================================================================*/

/**
 * @brief   A conditional distribution
 *
 * This class implements a general conditional distribution / random
 * variable concept. Let p(x | y) be a conditional distribution, with
 * x and y random variables. This class represents both p(x | y) and
 * x | y. To use this class, we need to know the domain of x, the domain
 * of y and how x depends on y. 
 *
 * @tparam  TargetVariable  The target random variable type; must comply
 *                          with the KJB's Distribution concept.
 *
 * @tparam  GivenVariable   The given random variable type; must comply
 *                          with the KJB's Distribution concept.
 *
 * @tparam  DependenceFunc  This says how the target depends on the given.
 *                          Must be applicable (operator()-wise) to a
 *                          (GivenVariable, type(GivenVariable)) pair.
 */
template<class TargetVariable, class GivenVariable, class DependenceFunc>
class Conditional_distribution
{
private:
    DependenceFunc depfunc;

public:
    typedef typename Distribution_traits<TargetVariable>::type target_type;
    typedef typename Distribution_traits<GivenVariable>::type given_type;

    Conditional_distribution(DependenceFunc f) :
        depfunc(f)
    {}

    Conditional_distribution(const Conditional_distribution& cd) :
        depfunc(cd.depfunc)
    {}

    Conditional_distribution& operator=(const Conditional_distribution& cd)
    {
        if(&cd != this)
        {
            depfunc = cd.depfunc;
        }

        return *this;
    }

    /**
     * @brief   Gets the distribution when we fix the given variable;
     *          i.e., it applies the dependence function to y.
     */
    TargetVariable get_distribution(const given_type& y) const
    {
        return depfunc(y);
    }
};

/**
 * @brief   Gets the conditional pdf p(x | y) of the given
 *          ConditionalDistribution.
 */
template<class Target, class Given, class Func>
inline double conditional_pdf
(
    const Conditional_distribution<Target, Given, Func>& cd,
    const typename Distribution_traits<Target>::type& x,
    const typename Distribution_traits<Given>::type& y
)
{
    return pdf(cd.get_distribution(y), x);
}

/**
 * @brief   Gets the conditional log-pdf log p(x | y) of the given
 *          ConditionalDistribution.
 */
template<class Target, class Given, class Func>
inline double conditional_log_pdf
(
    const Conditional_distribution<Target, Given, Func>& cd,
    const typename Distribution_traits<Target>::type& x,
    const typename Distribution_traits<Given>::type& y
)
{
    return log_pdf(cd.get_distribution(y), x);
}

/**
 * @brief   Produces a sample from the given conditioanl distro,
 *          given a value for the given variable.
 */
template<class Target, class Given, class Func>
inline typename Distribution_traits<Target>::type conditional_sample
(
    const Conditional_distribution<Target, Given, Func>& cd,
    const typename Distribution_traits<Given>::type& y
)
{
    return sample(cd.get_distribution(y));
}


/********************************************************
 *          SOME DEPENDENCE FUNCTIONS                   *
 ********************************************************/

/**
 * @brief   Represents the dependence between X and Y
 *          in p(x | y), where (x,y) is a bivariate
 *          normal.
 *
 * We need five parameters to determine a bivariate normal:
 * three elements of the covariance matrix of the joint
 * (var_X, var_Y, * and cov_XY) and the two means
 * (mean_X, mean_Y). From these parameters, we can determine
 * p(x | y) for any value of y.
 *
 * Additionally, one can provide the conditional variance
 * directly, which means one wants the mean to be simply y.
 */
class Normal_on_normal_dependence
{
public:
    /**
     * @brief   Specify this conditional distributon by
     *          specifying the marginals and the covariance.
     */
    Normal_on_normal_dependence
    (
        const Normal_distribution& PX,
        const Normal_distribution& PY,
        double covariance_XY
    ) :
        X(PX),
        Y(PY),
        cov_XY(covariance_XY)
    {}

    /**
     * @brief   Specify this conditional distributon by
     *          specifying the conditional variance, meaning
     *          the mean will simply the given value y. This
     *          a common thing to do in our lab.
     */
    Normal_on_normal_dependence(double conditional_std_dev) :
        X(0, sqrt(conditional_std_dev * conditional_std_dev + 1)),
        Y(0, 1),
        cov_XY(1)
    {}

    Normal_on_normal_dependence(const Normal_on_normal_dependence& nond) :
        X(nond.X),
        Y(nond.Y),
        cov_XY(nond.cov_XY)
    {}

    Normal_on_normal_dependence& operator=(const Normal_on_normal_dependence& nond)
    {
        if(&nond != this)
        {
            X = nond.X;
            Y = nond.Y;
            cov_XY = nond.cov_XY;
        }

        return *this;
    }

    Normal_distribution operator()(double y) const
    {
        double var_X = X.standard_deviation() * X.standard_deviation();
        double var_Y = Y.standard_deviation() * Y.standard_deviation();

        double mu = X.mean() + ((cov_XY / var_Y) * (y - Y.mean()));
        double sigma = var_X - ((cov_XY * cov_XY) / var_Y);
        return Normal_distribution(mu, sqrt(sigma));
    }

private:
    Normal_distribution X;
    Normal_distribution Y;
    double cov_XY;
};

typedef Normal_on_normal_dependence Gaussian_on_gaussian_dependence;

typedef Conditional_distribution<Normal_distribution, Normal_distribution, Normal_on_normal_dependence> Normal_conditional_distribution;

typedef Normal_conditional_distribution Gaussian_conditional_distribution;

/**
 * @brief   Represents the dependence between X and Y
 *          in p(x | y), where (x,y) is a multivariate
 *          normal.
 *
 * We need five parameters to determine a bivariate normal:
 * three elements of the covariance matrix of the joint
 * (var_X, var_Y, * and cov_XY) and the two means
 * (mean_X, mean_Y). From these parameters, we can determine
 * p(x | y) for any value of y.
 *
 * Additionally, one can provide the conditional variance
 * directly, which means one wants the mean to be simply y.
 */
class MV_normal_on_normal_dependence
{
public:
    /**
     * @brief   Specify this conditional distributon by
     *          specifying the marginals and the covariance.
     */
    MV_normal_on_normal_dependence
    (
        const MV_normal_distribution& PX,
        const MV_normal_distribution& PY,
        const Matrix& covariance_XY
    ) :
        X(PX),
        Y(PY),
        cov_XY(covariance_XY)
    {
        if(cov_XY.get_num_rows() != X.get_dimension() || cov_XY.get_num_cols() != Y.get_dimension())
        {
            KJB_THROW_2(Dimension_mismatch, "Dimension of covariance matrix between X and Y is wrong.");
        }
    }

    /**
     * @brief   Specify this conditional distributon by
     *          specifying the conditional variance, meaning
     *          the mean will simply the given value y. This
     *          a common thing to do in our lab.
     */
    MV_normal_on_normal_dependence(const Matrix& conditional_cov, int dim_y) :
        X(Vector(conditional_cov.get_num_rows(), 0.0),
                 conditional_cov + create_identity_matrix(conditional_cov.get_num_rows())),
        Y(Vector(dim_y, 0.0), create_identity_matrix(dim_y)),
        cov_XY(create_identity_matrix(conditional_cov.get_num_rows(), dim_y))
    {}

    MV_normal_on_normal_dependence(const MV_normal_on_normal_dependence& nond) :
        X(nond.X),
        Y(nond.Y),
        cov_XY(nond.cov_XY)
    {}

    MV_normal_on_normal_dependence& operator=(const MV_normal_on_normal_dependence& nond)
    {
        if(&nond != this)
        {
            X = nond.X;
            Y = nond.Y;
            cov_XY = nond.cov_XY;
        }

        return *this;
    }

    MV_normal_distribution operator()(const Vector& y) const
    {
        const Vector& mu_X = X.get_mean();
        const Vector& mu_Y = Y.get_mean();
        const Matrix& Sigma_X = X.get_covariance_matrix();
        const Matrix& Sigma_Y = Y.get_covariance_matrix();

        Vector mu = mu_X + (cov_XY * matrix_inverse(Sigma_Y) * (y - mu_Y));
        Matrix Sigma = Sigma_X - (cov_XY * matrix_inverse(Sigma_Y) * matrix_transpose(cov_XY));

        return MV_normal_distribution(mu, Sigma);
    }

private:
    MV_normal_distribution X;
    MV_normal_distribution Y;
    Matrix cov_XY;
};

typedef MV_normal_on_normal_dependence MV_gaussian_on_gaussian_dependence;

typedef Conditional_distribution<MV_normal_distribution, MV_normal_distribution, MV_normal_on_normal_dependence> MV_normal_conditional_distribution;

typedef MV_normal_conditional_distribution MV_gaussian_conditional_distribution;


} //namespace kjb

#endif /*PROB_CONDITIONAL_DISTRIBUTION_H_INCLUDED */

