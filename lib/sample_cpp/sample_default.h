/* $Id: sample_default.h 17393 2014-08-23 20:19:14Z predoehl $ */
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
   |  Author:  Kyle Simek, Ernesto Brau
 * =========================================================================== */

#ifndef SAMPLE_DEFAULT_H_INCLUDED
#define SAMPLE_DEFAULT_H_INCLUDED

#include "sample_cpp/sample_concept.h"
#include "sample_cpp/sample_base.h"
#include "l_cpp/l_exception.h"
#include <boost/bind.hpp>


/* =================================================================================== *
 * DEFAULT GENERAL FUNCTIONS
 * =================================================================================== */

/**
 * @brief   Default move function; uses '+'.
 */
template<class Model>
struct Move_model_parameter_as_plus
{
    typedef typename Set_model_parameter<Model>::Type Set_param;
    typedef typename Get_model_parameter<Model>::Type Get_param;

    Move_model_parameter_as_plus(const Set_param& set, const Get_param& get) :
        set_p(set), get_p(get)
    {}

    void operator()(Model& m, size_t i, double dx) const
    {
        double x = get_p(m, i);
        set_p(m, i, x + dx);
    }

    Set_param set_p;
    Get_param get_p;
};


/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

/* =================================================================================== *
 * USEFUL, GENERIC PARAMETER EVALUATORS -- MOST LIKELY USED FOR PARAMETERS
 * =================================================================================== */

/**
 * @brief   Returns the same result no matter what model is
 *          received.
 */
template<class Model>
struct Constant_parameter_evaluator
{
    Constant_parameter_evaluator(const kjb::Vector& evaluations) :
        evals(evaluations)
    {}

    Constant_parameter_evaluator(double evaluation, size_t size) :
        evals(static_cast<int>(size), evaluation)
    {}

    const kjb::Vector& operator()(const Model& /*m*/) const
    {
        return evals;
    }

    kjb::Vector evals;
};

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

/* =================================================================================== *
 * DEFAULTS RELATED TO HYBRID MC AND STOCHASTIC DYNAMICS
 * =================================================================================== */

/**
 * @brief   Approximates the gradient and/or curvature of a target distribution, evaluated
 *          at a certain location. The user must provide the mechanisms to
 *          change the model (see constructor).
 * @note curvature is only computed in the direction of the axes.
 */
template<typename Model>
class Numerical_gradient
{
public:
    typedef typename Model_evaluator<Model>::Type Target_distribution;
    typedef typename Move_model_parameter<Model>::Type Move_parameter;
    typedef typename Model_parameter_evaluator<Model>::Type Get_neighborhood;
    typedef typename Model_dimension<Model>::Type Get_dimension;

    /**
     * @brief   Construct a gradient approximation functor.
     *
     * @param   log_target      The log of the target distribution.
     * @param   move_param      Mechanism to 'move' a parameter of
     *                          the model by a delta.
     * @param   get_neighbors   Mechamisn to get the neighborhood
     *                          of the current model location. We
     *                          allow this to be a function of the
     *                          current model.
     * @param   get_dim         Mechanism to obtain the dimension
     *                          of the model.
     */
    Numerical_gradient
    (
        const Target_distribution& log_target,
        const Move_parameter& move_param,
        const Get_neighborhood& get_neighbors,
        const Get_dimension& get_dim
    ) :
        l_target(log_target),
        move_parameter(move_param),
        get_neighborhood(get_neighbors),
        get_dimension(get_dim)
    {}

    /**
     * @brief   Evaluates gradient at q.
     */
    kjb::Vector operator()(const Model& q) const
    {
        return gradient(q);
    }

    kjb::Vector curvature(const Model& q) const
    {
        kjb::Vector result;
        gradient_curvature_dispatch_(q, boost::none, result);
        return result;
    }

    kjb::Vector gradient (const Model& q) const
    {
        kjb::Vector result;
        gradient_curvature_dispatch_(q, result, boost::none);
        return result;
    }

    void gradient_and_curvature(const Model& q, kjb::Vector& gradient, kjb::Vector& curvature) const
    {
        gradient_curvature_dispatch_(q, gradient, curvature);
    }

private:
    /**
     * Compute either gradient, curvature, or both.  Computing both together
     * is less expensive than computing them separately.  Pass boost::none
     * to either gradient or curvature to skip its computation.
     */
    void gradient_curvature_dispatch_(
            const Model& q,
            boost::optional<kjb::Vector&> gradient,
            boost::optional<kjb::Vector&> curvature) const;

private:
    Target_distribution l_target;
    Move_parameter move_parameter;
    Get_neighborhood get_neighborhood;
    Get_dimension get_dimension;
};

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

template<typename Model>
void Numerical_gradient<Model>::gradient_curvature_dispatch_(const Model& q, boost::optional<kjb::Vector&> gradient, boost::optional<kjb::Vector&> curvature) const
{
    // if neither curvature or gradient is requested, do nothing
    if(!gradient && !curvature) return;

    Model q2 = q;
    size_t dim = get_dimension(q2);
    kjb::Vector etas = get_neighborhood(q2);

    if(gradient)
        (*gradient).resize(dim);
    if(curvature)
        (*curvature).resize(dim);

    double t_center = 0;

    if(curvature)
        t_center = l_target(q2);


    for(size_t i = 0; i < dim; i++)
    {
        double eta = etas[i];
        // 0.0 is a sentinal value meaning "this
        // dimension will be ignored".  
        //
        //
        // So ignore it.
        if(eta == 0)
        {
            if(gradient)
                (*gradient)[i] = 0.0;
            if(curvature)
                (*curvature)[i] = 0.0;
            continue;
        }

        // compute value of target at q + eta
        move_parameter(q2, i, eta);
        double t_right = l_target(q2);

        // compute value of target at q - eta
        move_parameter(q2, i, -2 * eta);
        double t_left = l_target(q2);

        if(gradient)
        {
            (*gradient)[i] = (t_right - t_left) / (2 * eta);
            assert((*gradient)[i] == (*gradient)[i]); // nan test
        }
        if(curvature)
            (*curvature)[i] = ( t_left - 2*t_center + t_right) / (eta * eta);

        // move it back to its original position
        move_parameter(q2, i, etas[i]);
    }
}


/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

/**
 * @brief   Adapts a target distribution to be one with bounds.
 */
template<typename Model>
class Constrained_target
{
private:
    typedef typename Model_evaluator<Model>::Type Target;
    typedef typename Model_parameter_evaluator<Model>::Type Get_upper_bounds;
    typedef typename Model_parameter_evaluator<Model>::Type Get_lower_bounds;

public:
    /**
     * @brief   Construct this target distribution.
     *
     * @param   target      The original target distribution.
     * @param   get_upper   Mechanism to get the upper bounds for a model.
     * @param   get_lower   Mechanism to get the lower bounds for a model.
     */
    Constrained_target
    (
        const Target& target,
        const Get_upper_bounds& get_upper,
        const Get_lower_bounds& get_lower
    ) :
        m_target(target),
        m_get_upper_bounds(get_upper),
        m_get_lower_bounds(get_lower)
    {}

    /**
     * @brief   Apply the target distribution to a model.
     */
    double operator()(const Model& m) const
    {
        return m_target(m);
    }

    /**
     * @brief   Return the vector of upper bounds for a given model.
     */
    kjb::Vector get_upper_bounds(const Model& m) const
    {
        return m_get_upper_bounds(m);
    }

    /**
     * @brief   Return the vector of lower bounds for a given model.
     */
    kjb::Vector get_lower_bounds(const Model& m) const
    {
        return m_get_lower_bounds(m);
    }

private:
    Target m_target;
    Get_upper_bounds m_get_upper_bounds;
    Get_lower_bounds m_get_lower_bounds;
};

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

/* =================================================================================== *
 * OTHER DEFAULTS
 * =================================================================================== */

/**
 * @brief   Generic posterior class.
 *
 * This class represents an un-normalized log-posterior class, which is composed of the sum
 * of the log-prior and log-likelihood.  
 *
 * This class offers a "short circuiting" feature in which the likelihood 
 * is not evaluated where the prior is zero; in this way, the prior may 
 * "protect" the likelihood from invalid values that would result in an error.
 *
 */
template <class Model>
class Posterior
{
private:
    typedef typename Model_evaluator<Model>::Type Model_eval;

    Model_eval m_prior;
    Model_eval m_likelihood;

    bool m_short_circuit;
    double m_annealing_compensation;
public:
    Posterior(const Model_eval& prior, const Model_eval& likelihood) :
        m_prior(prior),
        m_likelihood(likelihood),
        m_short_circuit(false),
        m_annealing_compensation(1.0)
    {}

    void set_short_circuiting(bool sc)
    {
        m_short_circuit = sc;
    }

    double operator()(const Model& m) const
    {
        double p = m_prior(m);
        if(m_short_circuit && p < -DBL_MAX)
        {
            return p;
        }

        // multiply by annealing temperature to cancel-out the prior being divided
        // by temperature inside the sampling step
        p *= m_annealing_compensation;

        return p + m_likelihood(m);
    }

    /**
     * If this posterior is to be used in simulated annealing, often it is desirable to
     * only apply annealing to the likelihood function.  Since annealing by default is 
     * applied to the entire target distribution, this function allows you to set a 
     * "compensation factor" which will cancel-out the affect of annealing on the prior,
     * resulting in only the likelihood being annealed.
     *
     * You can pass this as a callback to Annealing_sampler's add_temperature_changed_callback()
     * so it is updated every time the temperature changes.
     */
    void set_annealing_compensation(double temperature)
    {
        m_annealing_compensation = temperature;
    }
};

#endif /*SAMPLE_DEFAULT_H_INCLUDED */

