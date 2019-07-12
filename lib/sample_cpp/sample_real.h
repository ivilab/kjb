/* $Id: sample_real.h 17393 2014-08-23 20:19:14Z predoehl $ */
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

#ifndef SAMPLE_REAL_H_INCLUDED
#define SAMPLE_REAL_H_INCLUDED

#include "sample_cpp/sample_step.h"
#include "sample_cpp/sample_default.h"
#include "l/l_sys_lib.h"

/**
 * @brief   Parameter getter for anything that
 *          implements operator[].
 */
template<class Model>
inline
double get_real_model_parameter(const Model& m, int i)
{
    using namespace kjb_c;
    ASSERT(i == 0);
    return m;
}

/**
 * @brief   Parameter setter for anything that
 *          implements operator[].
 */
template<class Model>
inline
void set_real_model_parameter(Model& m, int i, double x)
{
    using namespace kjb_c;
    ASSERT(i == 0);
    m = x;
}

/**
 * @brief   Parameter mover for anything that
 *          implements operator[] and whose
 *          elements are double.
 */
template<class Model>
inline
void move_real_model_parameter(Model& m, int i, double x)
{
    Move_model_parameter_as_plus<Model> mmpap(set_real_model_parameter<Model>,
                                              get_real_model_parameter<Model>);
    mmpap(m, i, x);
}


/**
 * @brief   Dimension for any model that implements
 *          the size() member function.
 */
template<class Model>
inline
int get_real_model_dimension(const Model& /*m*/)
{
    return 1;
}

/**
 * @brief   Approximates the gradient of a target distribution, evaluated
 *          at a certain location. The model in question must be a vector
 *          model.
 */
template<typename Model>
class Real_numerical_gradient : public Numerical_gradient<Model>
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
    Real_numerical_gradient
    (
        const Target_distribution& log_target,
        const Get_neighborhood&    get_neighborhood
    ) :
        Parent
        (
            log_target,
            move_real_model_parameter<Model>,
            get_neighborhood,
            get_real_model_dimension<Model>
        )
    {}
    /**
     * @brief   Construct this functor with a constant hood.
     *
     * @param   log_target          The log of the target
     *                              distribution.
     * @param   neighborhood        Neighborhood size.
     */
    Real_numerical_gradient
    (
        const Target_distribution& log_target,
        double                     neighborhood
    ) :
        Parent
        (
            log_target,
            move_real_model_parameter<Model>,
            Constant_parameter_evaluator<Model>(kjb::Vector(1, neighborhood)),
            get_real_model_dimension<Model>
        )
    {}
};

/**
 * @class Real_hmc_step
 *
 * @tparam Model The model type.  Must comply with VectorModel concept; i.e.,. must
 *                                have [] and .size() defined and its elements must
 *                                be convertible to double.
 *
 * Real_hmc_step is a functor that runs a single step of hybrid Monte Carlo on a
 * vector model
 */
template<typename Model,
         bool CONSTRAINED_TARGET = false,
         bool INCLUDE_ACCEPT_STEP = true,
         bool REVERSIBLE = true>
class Real_hmc_step : public Basic_hmc_step<Model, CONSTRAINED_TARGET, INCLUDE_ACCEPT_STEP, REVERSIBLE>
{
public:
    typedef Basic_hmc_step<Model, CONSTRAINED_TARGET, INCLUDE_ACCEPT_STEP, REVERSIBLE> Parent;

    typedef typename Parent::Target_distribution Target_distribution;
    typedef typename Parent::Gradient Gradient;
    typedef typename Parent::Get_step_size Get_step_size;
    typedef typename Parent::Get_lower_bounds Get_lower_bounds;
    typedef typename Parent::Get_upper_bounds Get_upper_bounds;

    /**
     * @brief   Creates a Real_hmc_step.
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
    Real_hmc_step
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
               move_real_model_parameter<Model>, 
               get_step_size,
               get_real_model_dimension<Model>,
               alpha)
    {}

    /**
     * @brief   Creates a Real_hmc_step with a constant step size.
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
     * @param step_size Step size for the leap frog algorithm.
     */
    Real_hmc_step
    (
        const Target_distribution& log_target, 
        int                        num_dynamics_steps,
        const Gradient&            gradient,
        double                     step_size,
        double                     alpha = 0.0
    ) :
        Parent(log_target,
               num_dynamics_steps,
               gradient,
               move_real_model_parameter<Model>, 
               Constant_parameter_evaluator<Model>(kjb::Vector(1, step_size)),
               get_real_model_dimension<Model>,
               alpha)
    {}

    /**
     * @brief   Creates a Real_hmc_step with a constant step size.
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
     * @param step_size Step size for the leap frog algorithm.
     */
    Real_hmc_step
    (
        const Target_distribution& log_target, 
        int                        num_dynamics_steps,
        const Gradient&            gradient,
        double                     step_size,
        double                     lower_bound,
        double                     upper_bound,
        double                     alpha = 0.0
    ) :
        Parent(log_target,
               num_dynamics_steps,
               gradient,
               move_real_model_parameter<Model>, 
               Constant_parameter_evaluator<Model>(kjb::Vector(1, step_size)),
               get_real_model_dimension<Model>,
               get_real_model_parameter<Model>,
               Constant_parameter_evaluator<Model>(kjb::Vector(1, lower_bound)),
               Constant_parameter_evaluator<Model>(kjb::Vector(1, upper_bound)),
               alpha)
    {}

    virtual ~Real_hmc_step()
    {}
};

/**
 * @class Real_sd_step
 *
 * @tparam Model    The model type. Must comply with HmcModel concept and
 *                  Real model concept.
 *
 * Real_sd_step is a functor that runs a single step of the stochastic
 * dynamics algorithm (i.e., an HMC step without an accept/reject step) on
 * a real model.
 */
template<class Model, bool REVERSIBLE = true>
struct Real_sd_step
{
    typedef Real_hmc_step<Model, false, REVERSIBLE> Type;
};

#endif /*SAMPLE_REAL_H_INCLUDED */

