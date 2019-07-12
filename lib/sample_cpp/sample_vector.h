/* $Id: sample_vector.h 17393 2014-08-23 20:19:14Z predoehl $ */
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

#ifndef SAMPLE_VECTOR_H_INCLUDED
#define SAMPLE_VECTOR_H_INCLUDED

#include <algorithm>
#include "sample_cpp/sample_step.h"
#include "sample_cpp/sample_default.h"

#include <wrap_lapack/wrap_lapack.h>
#include <prob_cpp/prob_distribution.h>
#include <m_cpp/m_matrix.h>
#include <n_cpp/n_cholesky.h>

/**
 * @brief   Parameter getter for anything that
 *          implements operator[].
 */
template<class Model>
inline
double get_vector_model_parameter(const Model& m, int i)
{
    return m[i];
}

/**
 * @brief   Parameter setter for anything that
 *          implements operator[].
 */
template<class Model>
inline
void set_vector_model_parameter(Model& m, int i, double x)
{
    m[i] = x;
}

/**
 * @brief   Parameter mover for anything that
 *          implements operator[] and whose
 *          elements are double.
 */
template<class Model>
inline
void move_vector_model_parameter(Model& m, int i, double x)
{
    Move_model_parameter_as_plus<Model> mmpap(set_vector_model_parameter<Model>,
                                              get_vector_model_parameter<Model>);
    mmpap(m, i, x);
}


/**
 * @brief   Dimension for any model that implements
 *          the size() member function.
 */
template<class Model>
inline
int get_vector_model_dimension(const Model& m)
{
    return m.size();
}

/**
 * @brief   Approximates the gradient of a target distribution, evaluated
 *          at a certain location. The model in question must be a vector
 *          model.
 */
template<typename Model>
class Vector_numerical_gradient : public Numerical_gradient<Model>
{
public:
    typedef Numerical_gradient<Model> Parent;

    typedef typename Parent::Target_distribution Target_distribution;
    typedef typename Parent::Get_neighborhood Get_neighborhood;

    /**
     * @brief   Construct this functor.
     *
     * @param   log_target          The log of the target
     *                              distribution.
     * @param   get_neighborhood    Mechanism to get the
     *                              neighborhood of the model.
     */
    Vector_numerical_gradient
    (
        const Target_distribution& log_target,
        const Get_neighborhood&    get_neighborhood
    ) :
        Parent
        (
            log_target,
            move_vector_model_parameter<Model>,
            get_neighborhood,
            get_vector_model_dimension<Model>
        )
    {}
};

/**
 * @class Vector_hmc_step
 *
 * @tparam Model The model type.  Must comply with VectorModel concept; i.e.,. must
 *                                have [] and .size() defined and its elements must
 *                                be convertible to double.
 *
 * Vector_hmc_step is a functor that runs a single step of hybrid Monte Carlo on a
 * vector model
 */
template<typename Model, bool CONSTRAIN_TARGET = false, bool INCLUDE_ACCEPT_STEP = true, bool REVERSIBLE = true>
class Vector_hmc_step : public Basic_hmc_step<Model, CONSTRAIN_TARGET, INCLUDE_ACCEPT_STEP, REVERSIBLE>
{
public:
    typedef Basic_hmc_step<Model, CONSTRAIN_TARGET, INCLUDE_ACCEPT_STEP, REVERSIBLE> Parent;

    typedef typename Parent::Target_distribution Target_distribution;
    typedef typename Parent::Gradient Gradient;
    typedef typename Parent::Get_step_size Get_step_size;
    typedef typename Parent::Get_lower_bounds Get_lower_bounds;
    typedef typename Parent::Get_upper_bounds Get_upper_bounds;

    /**
     * @brief   Creates a Vector_hmc_step.
     * 
     * @param log_target Target_distribution object used to initialize 
     *                   internal target distribution used in operator().
     *
     * @param num_dynamics_steps Number of dynamics steps to take before
     *                           accepting/rejecting.
     *
     * @param gradient  Mechanism to compute the gradient of the (log) target
     *                  distribution.
     *
     * @param get_step_size Mechanism to get the step sizes for the leap
     *                      frog algorithm.
     */
    Vector_hmc_step
    (
        const Target_distribution& log_target, 
        int                        num_dynamics_steps,
        const Gradient&            gradient,
        const Get_step_size&       get_step_size,
        double                     alpha = 0.0
    ) :
        Parent(log_target,
               num_dynamics_steps,
               gradient,
               move_vector_model_parameter<Model>, 
               get_step_size,
               get_vector_model_dimension<Model>,
               alpha)
    {}

    Vector_hmc_step
    (
        const Target_distribution& log_target, 
        int                        num_dynamics_steps,
        const Gradient&            gradient,
        const Get_step_size&       get_step_size,
        const Get_lower_bounds&     get_lower_bounds,
        const Get_upper_bounds&     get_upper_bounds,
        double                     alpha = 0.0
    ) :
        Parent(log_target,
               num_dynamics_steps,
               gradient,
               move_vector_model_parameter<Model>, 
               get_step_size,
               get_vector_model_dimension<Model>,
               get_vector_model_parameter<Model>,
               get_lower_bounds,
               get_upper_bounds,
               alpha)
    {}

    virtual ~Vector_hmc_step()
    {}
};

/**
 * @class Vector_sd_step
 *
 * @tparam Model    The model type. Must comply with HmcModel concept and
 *                  Vector mode concept.
 *
 * Vector_sd_step is a functor that runs a single step of the stochastic
 * dynamics algorithm (i.e., an HMC step without an accept/reject step) on
 * a vector model.
 */
template<class Model, bool REVERSIBLE = true>
struct Vector_sd_step
{
    typedef Vector_hmc_step<Model, false, REVERSIBLE> Type;
};

template<typename SimpleVector>
class Independent_gaussian_proposer
{
private:
    kjb::Vector scale_;

    /** @brief  Construct a Independent_gaussian_proposer. */
    Independent_gaussian_proposer(const kjb::Vector& covariance) : 
        scale_(covariance.size())
    {
        std::transform(covariance.begin(), covariance.end(), scale_.begin(), static_cast<double(*)(double)>(sqrt));
    }

    /** @brief  Propose a new value. */
    Mh_proposal_result operator()(const SimpleVector& m, SimpleVector& mp) const
    {
        if(m.size() != scale_.size())
        {
            KJB_THROW(kjb::Dimension_mismatch);
        }

        mp.resize(m.size());
        for(size_t i = 0; i < mp.size(); ++i)
        {
            mp[i] = m[i] + kjb::sample(kjb::STD_NORMAL) * scale_[i];
        }

        return Mh_proposal_result();
    }
};

/**
 * @class Mv_gaussian_proposer
 *
 * Simple multi-variate Gaussian random-walk proposer. Given a vector x,
 * this object will propose x' ~ N(x, s), where s is provided at construction.
 */
template<typename VectorModel>
class Mv_gaussian_proposer
{
private:
    typedef Mv_gaussian_proposer Self;
    kjb::Matrix chol_cov_;
    double log_lambda_;
    double sqrt_t_;

public:
    /** @brief  Construct a Mv_gaussian_proposer. */
    Mv_gaussian_proposer(const kjb::Matrix& covariance) :
        chol_cov_(kjb::cholesky_decomposition(covariance)),
        log_lambda_(0.0),
        sqrt_t_(1.0)
    {}

    void set_temperature(double t)
    {
        sqrt_t_ = sqrt(t);
    }

    /** @brief  Propose a new value. */
    Mh_proposal_result operator()(const VectorModel& m, VectorModel& mp) const
    {
        size_t N = m.size();
        if(N != chol_cov_.get_num_rows() ||
           N != mp.size())
        {
            KJB_THROW(kjb::Dimension_mismatch);
        }

        kjb::Vector delta(N);
        std::generate(delta.begin(), delta.end(), boost::bind(Self::gauss_sample_));
        // scale by temperature, global scale, and covariance matrix
        delta = sqrt_t_ * exp(log_lambda_/2.0) * chol_cov_ * delta;

        std::transform(
                m.begin(),
                m.end(),
                delta.begin(),
                mp.begin(),
                std::plus<double>());

        return Mh_proposal_result();
    }

    /**
     * get cholesky decomposition of covariance matrix _before_ incorporating global scale.  This should be multiplied by the global scale to get the actual covariance
     */
    const kjb::Matrix& get_unscaled_cholesky_covariance() const
    {
        return chol_cov_;
    }

    kjb::Matrix get_covariance() const
    {
        kjb::Matrix result = sqrt_t_ * exp(log_lambda_/2.0) * chol_cov_;
        result = result.transpose() * result;
        return result;
    }

    /**
     * get global scaling constant
     */
    double get_global_scale() const
    {
        return sqrt(exp(log_lambda_));
    }

    /**
     * Update the empirical estimate of the covariance matrix C given a new observation.
     * The update uses the following recurrence relation:
     *     C_i+1 = C_i + gamma * (delta * delta' - C_i)
     * Here, gamma is the learning rate, and should be << 1.0.  delta is
     * the difference between the observed point and the empirical mean
     * (which should also be maintained and updated, but this is the responsibility 
     * of the caller).
     *
     * In practice, the cholesky decomposition of the covariance matrix is updated, and
     * is done efficiently in O(n^2) by performing a rank-1 update.
     */
    template <class VectorType>
    void adapt_covariance(const VectorType& delta, double gamma)
    {
#ifdef TEST
//        kjb::Matrix ref = chol_cov_;
//        kjb::Matrix test = chol_cov_;
#endif 

        rank_1_update_2(chol_cov_, delta, gamma);
        
#ifdef TEST
        // try rank-one update a few different ways, to see if they're all equivalent
//
//        double diff;
//        rank_1_update_ref(ref, delta, gamma);
//        diff = max_abs_difference(chol_cov_,ref);
//        assert(diff < FLT_EPSILON);
//
//// This gives whacked results   The other one seems fine
////        if(gamma < 0.01)
////        {
////            rank_1_update(test, delta, gamma);
////            diff = max_abs_difference(test,ref);
////        }
//        assert(diff < FLT_EPSILON);
#endif /* TEST */
        
    }

    /**
     * update global scaling factor, lambda by a factor x using:
     *   log(lambda) += x
     *
     * In general x will be:
     *     x = gamma * (alpha - alpha_goal)
     * where alpha is the accept-probability of the current MH step, alpha_star is 
     * the desired acceptance rate, and gamma is the learning rate (<< 1.0)
     */
    void adapt_global_scale(double x)
    {
        log_lambda_ += x;
    }

    /**
     * reference: A tutorial on adaptive MCMC, Andrieu, Thoms 2008.
     */
    void rank_1_update(kjb::Matrix& C, const VectorModel& delta, double gamma)
    {
#ifdef KJB_HAVE_LAPACK
        int N = delta.size();
        kjb::Matrix tmp(N, 1);
        std::copy(delta.begin(), delta.end(), &tmp(0,0));
        lapack_solve_triangular(
                C.get_c_matrix(),
                tmp.get_c_matrix(),
                &tmp.get_underlying_representation_with_guilt());

        tmp = tmp * tmp.transpose();

        // M -= I
        for(size_t i = 0; i < N; ++i)
            tmp(i,i) -= 1.0;

        lower_triangular_product_(C, tmp, tmp);
        tmp *= gamma;
        C += tmp;
#else
        KJB_THROW_2(kjb::Missing_dependency, "lapack");
#endif
    }

    /**
     * From wikipedia's Cholesky decomposition page, section "Rank-1 update"
     */
    void rank_1_update_2(kjb::Matrix& C, const VectorModel& delta, double gamma)
    {
        // TODO: DEBUG ME
        // I'm failing when gamma == 1.
        // Questions to answer:
        //    why am I multiplying C by sqrt(1-gamma)?
        //    check rasmussen's adaptive mh paper
        VectorModel x = sqrt(gamma) * delta;
        size_t N = delta.size();

        C *= sqrt(1-gamma);

        for(size_t k = 0; k < N; ++k)
        {
            double& C_k = C(k,k);
            const double x_k = x[k];

            double r = sqrt(C_k*C_k+x_k * x_k);
            double c = r/C_k;
            double s = x_k/C_k;
            C_k = r;
            for(size_t k2 = k+1; k2 < N; ++k2)
            {
                C(k2,k) = (C(k2,k) + s*x[k2])/c;
                x[k2] = c*x[k2] - s*C(k2, k);
            }
        }
    }


    /**
     * long-form solution; slow--for reference only
     */
    void rank_1_update_ref(kjb::Matrix& C, const VectorModel& delta, double gamma)
    {
        kjb::Vector tmp(delta.begin(), delta.end());
        C = C * C.transpose();
        C += gamma * (kjb::outer_product(tmp, tmp)-C);
#ifdef TEST
        kjb::Matrix m_tmp = C;
#endif

        C = kjb::cholesky_decomposition(C);

#ifdef TEST
        assert(kjb::max_abs_difference(C * C.transpose(), m_tmp) < FLT_EPSILON);
#endif
    }

private:
    // utility
    static double gauss_sample_() { return kjb::sample(kjb::STD_NORMAL); }

    /**
     * Product of two lower-triangular matrices.
     * @pre op1 and op2 must be square and have same size
     */
    void lower_triangular_product_(
            const kjb::Matrix& op1,
            const kjb::Matrix& op2,
            kjb::Matrix& out)
    {
        const size_t N = op1.get_num_cols();
        kjb::Matrix tmp(N,N, 0.0);

        for(size_t r = 0; r < N; ++r)
        for(size_t c = 0; c <= r; ++c)
        for(size_t i = c; i <= r; ++i)
            tmp(r,c) += op1(r,i)*op2(i,c);
        out.swap(tmp);
    }
};

/**
 * @class Vector_srw_step
 *
 * Symmetric-random-walk Metropolis step.  Proposals come from a multivariate gaussian
 * distribution.
 *
 * @tparam VectorModel The model type.  Must comply with VectorModel concept; i.e.,. must
 *                                have [] and .size() defined and its elements must
 *                                be convertible to and from double.
 *
 * Vector_srw_step is a functor that runs a single step of symmetric random walk Metropolis on a
 * vector model
 */
template<typename VectorModel>
class Vector_srw_step : public Basic_mh_step<VectorModel, Mv_gaussian_proposer<VectorModel> >
{
public:
    typedef Basic_mh_step<VectorModel, Mv_gaussian_proposer<VectorModel> > Base;
    typedef Vector_srw_step Self;

    typedef typename Base::Target_distribution Target_distribution;
    typedef typename Base::Proposer Proposer;

    /**
     * @brief   Creates a Vector_srw_step.
     * 
     * @param log_target Target_distribution object used to initialize 
     *                   internal target distribution used in operator().
     * @param covariance Covariance matrix for multi-variate gaussian proposals
     */
    Vector_srw_step
    (
        const Target_distribution& target,
        const kjb::Matrix& covariance
    ) :
        Base(target,
               Proposer(covariance))
    {}

    /**
     * @brief   Creates a Vector_srw_step.
     * 
     * @param log_target Target_distribution object used to initialize 
     *                   internal target distribution used in operator().
     * @param variance Elements for a diagonal covariance matrix of an independent multivariate gaussian distribution
     */
    Vector_srw_step
    (
        const Target_distribution& target,
        const kjb::Vector& covariance
    ) :
        Base(target,
               Proposer(covariance))
    {}

    /**
     * Update proposal covariance.  This could be an expensive operation, as it will require matrix inversion 
     */
    inline void set_covariance(const kjb::Matrix& covariance)
    {
        Proposer& proposer = Base::get_proposer_();
        proposer = Mv_gaussian_proposer<VectorModel>(covariance);
    }

    inline void set_covariance(const kjb::Vector& covariance)
    {
        Proposer& proposer = Base::get_proposer_();
        proposer = Mv_gaussian_proposer<VectorModel>(covariance);
    }

    virtual ~Vector_srw_step()
    {}
};

#endif /*SAMPLE_VECTOR_H_INCLUDED */

