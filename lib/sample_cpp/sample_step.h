/* $Id: sample_step.h 17393 2014-08-23 20:19:14Z predoehl $ */
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

#ifndef SAMPLE_STEP_H_INCLUDED
#define SAMPLE_STEP_H_INCLUDED

#include "sample_cpp/sample_concept.h"
#include "sample_cpp/sample_base.h"
#include "sample_cpp/sample_default.h"
#include "prob_cpp/prob_sample.h"
#include "l_cpp/l_exception.h"

/* ========================================================================= *
 * ========================================================================= *
 * ==                   METROPOLIS HASTINGS                               == *
 * ========================================================================= *
 * ========================================================================= */

/**
 * @class Abstract_mh_step
 *
 * @tparam Model The model type.  Must comply with BaseModel concept.
 *
 * Abstract_mh_step is a functor that runs a single step of metropolis hastings on a model.
 *
 * This class implements the logic for one Metropolis Hastings algorithm step.  A
 * subclass need only define the model's target distribution and a proposal mechanism. That
 * is, subclasses must implement get_log_target() and get_proposer().
 *
 * Optimizations to speed up MH (caching, for example) can be done in the implementation
 * of the proposer and the target distribution.
 */
template<typename Model, typename Proposer = typename Mh_model_proposer<Model>::Type >
class Abstract_mh_step
{
public:
    typedef typename Model_evaluator<Model>::Type Target_distribution;
//    typedef typename Mh_model_proposer<Model>::Type Proposer;

    BOOST_CONCEPT_ASSERT((BaseModel<Model>));

    //virtual const char* get_type() const{return "Abstract_mh_step";}
    /** @brief Runs a step of Metropolis-Hastings on a model m.  
     *
     * If the step is rejected, m remains unchanged; if the step is 
     * accepted, m will hold the new state.  Returns a structure 
     * describing the results of the step. 
     *
     * This three-parameter version also returns the acceptance probability, which
     * is useful when implementing adaptive MH. If this isn't needed, using operator()
     * is recommended.
     *
     * @param accept_prob The acceptance probability of the proposed model
     * @param proposed_state An output parameter that will receive a copy of the proposed state.  If ommitted, no copy occurs.
     *
     * @note This method uses swap(Model a, Model b), whose 
     * generalized implementation is very inefficient.  For best 
     * performance, swap() should be specialized for Model.
     **/
    Step_log<Model> run_step(Model& m, double lt_m, double& accept_prob, boost::optional<Model&> proposed_state_out = boost::none) const;

    /** @brief Runs a step of Metropolis-Hastings on a model m.  
     *
     * If the step is rejected, m remains unchanged; if the step is 
     * accepted, m will hold the new state.  Returns a structure 
     * describing the results of the step. 
     *
     * @note This method uses swap(Model a, Model b), whose 
     * generalized implementation is very inefficient.  For best 
     * performance, swap() should be specialized for Model.
     **/
    virtual Step_log<Model> operator()(Model& m, double lt_m) const;

    virtual double get_temperature() const { return 1.0; }

    virtual ~Abstract_mh_step(){}

    /// Propose a new model
    virtual Mh_proposal_result propose(const Model& m, Model& m_p) const = 0;

    /// Evaluate the log-target distribution for the given model
    virtual double l_target(const Model& m) const = 0;

    // optional callback when model is accepted
    virtual void on_accept() const {}
    // optional callback when model is rejected
    virtual void on_reject() const {}

protected:
    /**
     * returns true if the step should store intermediate values and return
     * them in the Step_log.  Useful for debugging, but incurs a small 
     * performance penalty.
     */
    virtual bool record_extra() const { return false; }

protected:
    mutable boost::optional<Model> tmp_model_;
};

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

template<typename Model, typename Proposer>
Step_log<Model> Abstract_mh_step<Model, Proposer>::run_step(Model& m, double lt_m, double& accept_prob, boost::optional<Model&> proposed_state_out) const
{
    // Just refactored this so
    // (a) no default construction is necessary
    // (b) the proposed object is only allocated once per the lifetime
    //     of the object, instead of once per iteration as previously
    // Kyle, December 11, 2011
    if(!tmp_model_)
    {
        // copy constructor is called
        tmp_model_ = m;
    }

    Model& m_p = *tmp_model_;
    double lt_m_p;
    double lt_final;

    // propose model and compute probabilities/densities
    Mh_proposal_result p_res = propose(m, m_p);

    // if the caller has requested a copy of the proposed state, copy it here
    if(proposed_state_out)
        *proposed_state_out = m_p;

    double q_m_p = p_res.fwd_prob;
    double q_m = p_res.rev_prob;

    if(p_res.no_change)
    {
        lt_m_p = lt_m;
    }
    else
    {
        // get log-target distribution of the proposed model
        lt_m_p = l_target(m_p);
    }

    // compute acceptance probability
    accept_prob = (lt_m_p - lt_m + q_m - q_m_p) / get_temperature();

    double u = std::log(kjb::sample(kjb::Uniform_distribution(0, 1)));

    bool accepted;

    if(u < accept_prob)
    {
        // Model type should specialize swap to get best performance 
        using std::swap;
        swap(m, m_p);
        lt_final = lt_m_p;
        accepted = true;
        on_accept();
    }
    else
    {
        lt_final = lt_m;
        accepted = false;
        on_reject();
    }

    Step_result<Model> result("mh-", p_res.type, accepted, lt_final, accepted ? &m : &m_p);
    // TODO: uncomment after NIPS
//    Step_result<Model> result("mh", p_res.type, accepted, lt_final, accepted ? &m : &m_p);

    if(record_extra())
    {
        result.extra["mh_p_fwd"] = lt_m_p;
        result.extra["mh_p_rev"] = lt_m;
        result.extra["mh_q_fwd"] = q_m_p;
        result.extra["mh_q_rev"] = q_m;
        result.extra["mh_accept"] = accept_prob;
        result.extra["no_change"]= p_res.no_change;
    }

    return Step_log<Model>(result);
}

template<typename Model, typename Proposer>
Step_log<Model> Abstract_mh_step<Model, Proposer>::operator()(Model& m, double lt_m) const
{
    double accept_prob;
    return run_step(m, lt_m, accept_prob);
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

/**
 * @class Basic_mh_step
 *
 * @tparam Model The model type.  Must comply with BaseModel concept.
 *
 * Basic_mh_step is a functor that runs a single step of metropolis hastings on a model.
 *
 * This class is a subclass of the abstract class Abstract_mh_step. It implements the
 * get_log_target and get_proposer member functions in the obvious way: it returns
 * references to private members of the correct types. These members are initialized
 * from references received in the constructor.
 */
template<typename Model, typename Proposer_type = typename Mh_model_proposer<Model>::Type >
class Basic_mh_step : public Abstract_mh_step<Model, Proposer_type>
{
public:
    typedef Abstract_mh_step<Model, Proposer_type> Parent;

    typedef typename Parent::Target_distribution Target_distribution;
    typedef Proposer_type Proposer;

    /**
     * @brief Initializes target and proposer functors (or functions)
     *        to the given arguments.
     * 
     * @param log_target Target_distribution object used to initialize 
     *                   internal target distribution used in operator().
     *
     * @param proposer Proposer_type object used to initialize internal
     *                 proposer used in operator().
     **/
    Basic_mh_step(const Target_distribution& log_target, const Proposer_type& proposer) : 
        Parent(),
        m_log_target(log_target),
        m_proposer(proposer),
        m_temperature(1.0),
        m_record_extra(false)
    {}

//    /**
//     * @brief Initializes target and proposer functors (or functions)
//     *        to the given arguments.
//     * 
//     * @param log_target Target_distribution object used to initialize 
//     *                   internal target distribution used in operator().
//     *
//     * @param proposer Proposer_type object used to initialize internal
//     *                 proposer used in operator().
//     *
//     * @param default_model If Model is not default-constructible, you need
//     *                      to provide a model to initialize new models.
//     **/
//    Basic_mh_step(const Target_distribution& log_target, const Proposer_type& proposer, const Model& default_model) : 
//        Parent(),
//        m_log_target(log_target),
//        m_proposer(proposer),
//        m_temperature(1.0),
//        m_record_extra(false)
//    {}
//
//    /**
//     * @brief   Copy-constructor
//     */
//    Basic_mh_step(const Basic_mh_step<Model, Proposer_type>& step) :
//        Parent(step),
//        m_log_target(step.m_log_target),
//        m_proposer(step.m_proposer),
//        m_temperature(1.0),
//        m_record_extra(false)
//    {}

    virtual ~Basic_mh_step(){}

//    /**
//     * @brief   Assignment
//     */
//    Basic_mh_step& operator=(const Basic_mh_step<Model, Proposer_type>& step)
//    {
//        if(&step != this)
//        {
//            m_log_target = step.m_log_target;
//            m_proposer = step.m_proposer;
//            m_temperature = step.m_temperature;
//        }
//
//        return *this;
//    }

    /// Propose a new model
    virtual Mh_proposal_result propose(const Model& m, Model& m_p) const
    {
        return m_proposer(m, m_p);
    }

    /// Evaluate the log-target distribution for the given model
    virtual double l_target(const Model& m) const
    {
        return m_log_target(m);
    }

    /// Set the log-target distribution.  Generally it's preferred
    /// to set this in the constructor, but this is occasionally
    /// useful when implementing copy constructor that needs to
    /// copy everything except the target, which you'll set yourself.
    /// This comes up when you're doing fancy stuff with the target
    /// distribution, like introducing coupling with other objects,
    /// and you need to maintain correct references, which the
    /// default copy constructor won't let you do. 
    void set_target(const Target_distribution& target)
    {
        m_log_target = target;
    }

    /**
     * Set to true if the step should store intermediate values and return
     * them in the Step_log.  Useful for debugging, but incurs a small 
     * performance penalty.
     */
    void record_extra(bool enable)
    {
        m_record_extra = enable;
    }

    /**
     * Set annealing temperature.  Increases acceptance rate; proposal distribution
     * is unaffected
     */
    void set_temperature(double T)
    { 
        m_temperature = T;
    }

    virtual double get_temperature() const
    {
        return m_temperature;
    }

protected:
    virtual bool record_extra() const
    {
        return m_record_extra;
    }

    Proposer_type& get_proposer_()
    {
        return m_proposer;
    }

    Target_distribution m_log_target;
    Proposer_type m_proposer;
    double m_temperature;

    bool m_record_extra;
};


/* ========================================================================= *
 * ========================================================================= *
 * ==                                GIBBS                                == *
 * ========================================================================= *
 * ========================================================================= */

/**
 * @class Abstract_gibbs_step
 *
 * @tparam Model The model type.  Must comply with BaseModel concept.
 *
 * Abstract_gibbs_step is a functor that runs a single step of Gibbs sampling on a model.
 *
 * This class implements the logic for one Gibbs algorithm step.  A
 * subclass need only define the model's target distribution and a proposal mechanism. That
 * is, subclasses must implement get_log_target() and get_proposer().
 */
template<typename Model>
class Abstract_gibbs_step
{
public:
    typedef typename Model_evaluator<Model>::Type Target_distribution;
    typedef typename Gibbs_model_proposer<Model>::Type Proposer;
    typedef typename Model_dimension<Model>::Type Get_dimension;

    BOOST_CONCEPT_ASSERT((BaseModel<Model>));

    /**
     * @brief Returns a reference to the target distribution. Returned 
     *        object must comply with the ModelEvaluator concept.
     */
    virtual const Target_distribution& get_log_target() const = 0;

    /**
     * @brief Returns a reference to the proposal mechanism. Returned object
     *        must comply with the GibbsModelProposer concept.
     */
    virtual const Proposer& get_proposer() const = 0;

    /**
     * @brief Returns a reference to a function that is used to obtain the
     *        dimension of the model. See Model_dimension for more info.
     */
    virtual const Get_dimension& get_dimension_function() const = 0;

    //virtual const char* get_type() const{return "Abstract_gibbs_step";}

    /**
     * @brief   Runs a step of Gibbs on a model m.  
     *
     * After this, m will hold the new state.  Returns a structure 
     * describing the results of the step. 
     */
    virtual Step_log<Model> operator()(Model& m, double lt_m) const;

    virtual ~Abstract_gibbs_step(){}
};


/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

template<class Model>
Step_log<Model> Abstract_gibbs_step<Model>::operator()(Model& m, double /* lt_m */) const
{
    const Target_distribution& l_target = get_log_target();
    const Proposer& propose = get_proposer();
    const Get_dimension& get_dimension = get_dimension_function();

    int num_vars = get_dimension(m);

    boost::optional<double> result;
    for(int i = 0; i < num_vars; i++)
    {
        result = propose(m, i);
    }

    if(!result)
    {
        result = l_target(m);
    }
    return Step_log<Model>(Step_result<Model>("gibbs", "gibbs", true, *result, &m));
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

/**
 * @class Basic_gibbs_step
 *
 * @tparam Model The model type.  Must comply with BaseModel concept.
 *
 * Basic_gibbs_step is a functor that runs a single step of metropolis hastings on a model.
 *
 * This class is a subclass of the abstract class Abstract_gibbs_step. It implements the
 * get_log_target and get_proposer member functions in the obvious way: it returns
 * references to private members of the correct types. These members are initialized
 * from references received in the constructor.
 */
template<typename Model>
class Basic_gibbs_step : public Abstract_gibbs_step<Model>
{
public:
    typedef Abstract_gibbs_step<Model> Parent;

    typedef typename Parent::Target_distribution Target_distribution;
    typedef typename Parent::Proposer Proposer;
    typedef typename Parent::Get_dimension Get_dimension;

    /**
     * @brief Initializes target and proposer functors (or functions)
     *        to the given arguments.
     * 
     * @param log_target Target_distribution object used to initialize 
     *                   internal target distribution used in operator().
     *
     * @param proposer Proposer object used to initialize internal
     *                 proposer used in operator().
     */
    Basic_gibbs_step
    (
        const Target_distribution& log_target,
        const Proposer& proposer,
        const Get_dimension& get_dimension
    ) :
        Parent(),
        m_log_target(log_target),
        m_proposer(proposer),
        m_get_dimension(get_dimension)
    {}

//    /**
//     * @brief   Copy-constructor
//     */
//    Basic_gibbs_step(const Basic_gibbs_step<Model>& step) :
//        Parent(step),
//        m_log_target(step.m_log_target),
//        m_proposer(step.m_proposer),
//        m_get_dimension(step.m_get_dimension)
//    {}

//    /**
//     * @brief   Assignment
//     */
//    Basic_gibbs_step& operator=(const Basic_gibbs_step<Model>& step)
//    {
//        if(&step != this)
//        {
//            m_log_target = step.m_log_target;
//            m_proposer = step.m_proposer;
//        }
//
//        return *this;
//    }
//
    virtual const Target_distribution& get_log_target() const{ return m_log_target; }

    virtual const Proposer& get_proposer() const{ return m_proposer; }

    virtual const Get_dimension& get_dimension_function() const{ return m_get_dimension; }

    virtual ~Basic_gibbs_step(){}

protected:
    Target_distribution m_log_target;
    Proposer m_proposer;
    Get_dimension m_get_dimension;
};


/* ========================================================================= *
 * ========================================================================= *
 * ==                                HMC                                  == *
 * ========================================================================= *
 * ========================================================================= */

/**
 * @class Abstract_hmc_step
 *
 * @tparam Model            The model type.
 * @tparam accept_step      Do a accept/reject step? If this is true 
 *                          this becomes a true HMC step.
 * @tparam reversible       Whether or not it's important for the leapfrog steps
                            to be reversible.  For true MCMC, this should be 
                            true, but passing false allows for fewer gradient 
                            evaluations, in some configurations (i.e. if we
                            accept_step = false, and alpha > 0).
 * @tparam last_p_ignore    If true, the last momentum update will be omitted.
 *                          For true MCMC, this should be false.
 *
 *
 * Abstract_hmc_step is a functor that runs a single step of HMC on a model.
 *
 * This class implements the logic for one HMC algorithm step.  A
 * subclass need only define the model's get_* mechanisms. Specifically,
 * it they must implement:
 * - get_log_target ---------- return the target distribution
 * - get_gradient ------------ return the function that computes the gradient
 *                             of the log target
 * - get_move_parameter ------ return the mechanism to change a parameter of
 *                             the model by a delta
 * - get_step_size_function -- return the mechanism for getting a vector of
 *                             step sizes for the leap-from algorithm
 * - get_dimension_function -- return the mechanism for getting the dimension
 *                             of the model
 *
 * True boolean template parameters control whether certain optimizations are on or off.
 */
template<typename Model,
         bool CONSTRAINED_TARGET = false,
         bool INCLUDE_ACCEPT_STEP = true,
         bool REVERSIBLE = true>
class Abstract_hmc_step
{
public:
    typedef typename Model_evaluator<Model>::Type Target_distribution;
    typedef typename Model_parameter_evaluator<Model>::Type Gradient;
    typedef typename Move_model_parameter<Model>::Type Move_parameter;
    typedef typename Model_parameter_evaluator<Model>::Type Get_step_size;
    typedef typename Model_dimension<Model>::Type Get_dimension;

    // constraint handling stuff
    typedef typename Get_model_parameter<Model>::Type Get_parameter;
    typedef typename Model_parameter_evaluator<Model>::Type Get_lower_bounds;
    typedef typename Model_parameter_evaluator<Model>::Type Get_upper_bounds;

    BOOST_CONCEPT_ASSERT((BaseModel<Model>));

    Abstract_hmc_step(
            int num_dynamics_steps,
            double alpha) :
        m_num_dynamics_steps(num_dynamics_steps),
        m_alpha(alpha),
        m_first_p_full(is_first_p_full(alpha, INCLUDE_ACCEPT_STEP, REVERSIBLE)),
        m_last_p_ignore(is_last_p_ignored(alpha, INCLUDE_ACCEPT_STEP, REVERSIBLE)),
        m_temperature(1.0)
    {}

    bool is_first_p_full(const bool alpha, const bool accept_step, const bool reversible)
    {
        // We'd like to avoid the final momentum update, if possible.  It 
        // isn't avoidable if we have an accept step.  If we discard
        // the accept step, we have two options:
        // There are two ways to skip the final momentum update:
        // (a) If the momentum is completely replaced in every step
        //     (i.e. alpha == 0), the final momentum doesn't matter, so 
        //     we can discard it without sacrificing anything.
        // (b) If we perform partial replacement of momenta (alpha > 0), 
        //     we can't discard the momentum update, so we'll roll it into 
        //     the first momentum update, sacrificing reversibility.
        //
        // If you use partial momentum updates, and reversibility must be
        // maintined, no optimizations may be used.  
        //
        // We can summarize this logic in a truth table, which we build below.
        // 
        // Inputs are encoded as follows:
        //     A = 1: accept/reject step is enabled
        //     A = 0: accept/reject step is skipped
        // 
        //     R = 1: reversibility must be maintained
        //     R = 0: reversibilty may be sacrificed.
        //
        //     P = 1: partial momenta updates are performed
        //     P = 0: momenta are fully replaced at each iteration
        //
        // Outputs:
        //     f = 0: first momenta update is a half-update
        //     f = 1:   "     "       "     "   full-update
        //     l = 0: last momemnta update is a half-update
        //     l = 1:  "     "        "     "   ignored
        //
        //  In other words, f=0 and l=0 means "use standard mcmc".  Any
        //  other combination of outputs implies an optimization is used.
        //
        //    A  R  P  ||  f  l    comments
        //   -------------------------------
        //    1  x  x  ||  0  0     true HMC
        //    0  0  0  ||  0  1     still reversible
        //    0  1  0  ||  0  1     
        //    0  0  1  ||  1  1     not reversible
        //    0  1  1  ||  0  0     
        //
        // This results in the following boolean expressions:
        //
        //    f = !A  ^ !(R ^ P)
        //    l = !A ^ !R ^ P
        //
        // Here, 'v' means "or",  '^' means "and".



        // Which of these methods to use 
        //
        const bool partial_updates = (alpha > 0.0);
        return !accept_step && !reversible && partial_updates;
    }

    bool is_last_p_ignored(const bool alpha, const bool accept_step, const bool reversible)
    {
        const bool partial_updates = (alpha > 0.0);
        // there's a truth table under is_first_p_full that explains this 
        // boolean logic.
        return !accept_step && !(reversible && partial_updates);
    }


    Abstract_hmc_step(const Abstract_hmc_step& ahs) :
        m_num_dynamics_steps(ahs.m_num_dynamics_steps),
        m_alpha(ahs.m_alpha),
        p(ahs.p),
        m_first_p_full(ahs.m_first_p_full),
        m_last_p_ignore(ahs.m_last_p_ignore),
        m_temperature(ahs.m_temperature)
    {}

//    Abstract_hmc_step& operator=(const Abstract_hmc_step& ahs)
//    {
//        m_num_dynamics_steps = ahs.m_num_dynamics_steps;
//        m_alpha = ahs.m_alpha;
//        p = ahs.p;
//        m_first_p_full = ahs.m_first_p_full;
//        m_last_p_ignore = ahs.m_last_p_ignore;
//        m_temperature = ahs.m_temperature;
//    }

    /** 
     * @brief Runs a step of Hybrid Monte-Carlo (HMC) on a model m.  
     *
     * If the step is rejected, m remains unchanged; if the step is 
     * accepted, m will hold the new state.  Returns a structure 
     * describing the results of the step. 
     *
     * @note This method uses swap(Model a, Model b), whose 
     * generalized implementation is very inefficient.  For best 
     * performance, swap() should be specialized for Model.
     */
    virtual Step_log<Model> operator()(Model& q, double lt_q) const;

    /**
     * @brief Returns a reference to the target distribution. Returned 
     *        object must comply with the ModelEvaluator concept.
     */
    virtual const Target_distribution& get_log_target() const = 0;

    /**
     * @brief Returns a reference to the mechanism to compute
     *        the gradient of the target distribution.
     */
    virtual const Gradient& get_gradient() const = 0;

    /**
     * @brief Returns a reference to the parameter-changing mechanism. 
     *        Must comply with the ChangeParameter concept.
     */
    virtual const Move_parameter& get_move_parameter() const = 0;

    /**
     * @brief Returns a reference to the mechanism for choosing
     *        the neighborhoods for each parameters.
     */
    virtual const Get_step_size& get_step_size_function() const = 0;

    /**
     * @brief Returns a reference to the mechanism for computing
     *        the dimension of the model.
     */
    virtual const Get_dimension& get_dimension_function() const = 0;

    void set_temperature(const double T) { m_temperature = T; }

    /// if true is returned, the step log will contain metadata about the step
    virtual bool record_extra() const = 0;
    //////////////// CONSTRAINT HANDLING STUFF ///////////////////////

    /**
     * @brief Returns a reference to the mechanism for getting
     *        the value of a dimension of the model.
     */
    virtual const Get_parameter& get_parameter_function() const = 0;

    /**
     * @brief Returns a reference to the mechanism for getting
     *        the upper bounds of a function.
     */
    virtual const Get_upper_bounds& get_upper_bounds_function() const = 0;

    /**
     * @brief Returns a reference to the mechanism for getting
     *        the lower bounds of a function.
     */
    virtual const Get_lower_bounds& get_lower_bounds_function() const = 0;

    //////////////////////////////////////////////////////////////////

    //////////////// GENERAL-PURPOSE CALLBACKS ///////////////////////

    /**
     * Callback that occurs every time the model state is updated during
     * leapfrom simulation of dynamics.  This is useful to apply constraints
     * to the model and/or momentum after they've been perturbed
     */
    virtual void post_move_callback(Model& /* q */, kjb::Vector& /* p */) const
    {
        return; // no-op
    }


    virtual ~Abstract_hmc_step(){}

protected:
    /**
     * @brief The number of leapfrog steps in the trajectory, "choosing a suitable 
     * trajectory length is crucial if HMC is to explore the stae space systematically, 
     * rather than by a random walk". "For a problem thought to be airly difficult, a trajectory
     * with m_length = 100 might be a suitable starting point. 
     */
    const int m_num_dynamics_steps;

    /**
     * @brief   Amount of stochastic update to apply to momentum.
     * @Note    Set to zero for strict HMC.
     */
    double m_alpha;

    /**
     * @brief   Momentum
     */
    mutable kjb::Vector p;

    /**
     * @brief Optimization constants. Set to false for true MCMC.
     */
    bool m_first_p_full;
    bool m_last_p_ignore;

    double m_temperature;
};


/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

template<class Model, bool CONSTRAINED_TARGET, bool INCLUDE_ACCEPT_STEP, bool REVERSIBLE>
Step_log<Model> Abstract_hmc_step<Model, CONSTRAINED_TARGET, INCLUDE_ACCEPT_STEP, REVERSIBLE>::operator()(Model& q, double lt_q) const
{
    // ///////////////////////////////////////////////////////////////
    // This variant of HMC is outlined in Horowitz (1991):
    //  "A generalized guided monte carlo algorithm."
    //  It allows us to choose between partial or complete stochastic 
    //  replacement of momenta, without changing the algorithm.  Neal's 
    //  version exhibits random walk when using "partial updates," so 
    //  we opted against it.
    // ////////////////////////////////////////////////////////////////
    
    // get all components (functors, callbacks, tunable params, etc)
    const Target_distribution& l_target = get_log_target();
    const Gradient& compute_gradient = get_gradient();
    const Move_parameter& move_parameter = get_move_parameter();
    const Get_step_size& get_step_size = get_step_size_function();
    const Get_dimension& get_dimension = get_dimension_function();

    // These are needed for constraint handling
    const Get_parameter& get_parameter = get_parameter_function();
    const Get_lower_bounds& get_lower_bounds = get_lower_bounds_function();
    const Get_upper_bounds& get_upper_bounds = get_upper_bounds_function();

    size_t hmc_dim = get_dimension(q);

    // make sure dimension of p is the same as of the model
    if(p.get_length() != static_cast<int>(hmc_dim))
    {
        p.zero_out(hmc_dim);
    }

    // Sample new momentum -- using a partial update.
    // Note: when m_alpha = 0 this algorithm becomes regular HMC
    p *= m_alpha;
    p += sqrt(1 - m_alpha * m_alpha) * kjb::sample(kjb::MV_gaussian_distribution(hmc_dim)) * sqrt(m_temperature);

    // Compute grad_U (which is -grad(log_target))
    // and multiply times epsilon (grad_U never appears alone)
    kjb::Vector epsilons = get_step_size(q);
    kjb::Vector grad_x_eps = -1.0 * compute_gradient(q);
    std::transform(grad_x_eps.begin(), grad_x_eps.end(), epsilons.begin(),
                   grad_x_eps.begin(), std::multiplies<double>());

    //////////////////////////////////////////////////////
    //                 leapfrog algorithm               //
    //////////////////////////////////////////////////////

    Model q_star = q;

    kjb::Vector p_star  = p;
    if(m_first_p_full)
    {
        // OPTIMIZATION BRANCH
        // We perform a full-step of the momentum.
        //
        // The normal algorithm calls for a 1/2 step here, but 
        // we offer an optimization that makes this a full step,
        // and eliminates the final 1/2 step later on.  This saves
        // one gradient evaluation per iteration, which may be significant.
        // Note: using this optimization, the algorithm is no longer true
        // MCMC, because the step is not volume-preserving (and thus not reversible)
        p_star -= grad_x_eps;
    }
    else
    {
        // We perform a half-step of the momentum 
        // (as per the normal leapfrog algorithm).
        p_star -= grad_x_eps / 2.0;
    }

    // needed for constraints
    kjb::Vector lower_bounds;
    kjb::Vector upper_bounds;

    // Alternate full steps for position and momentum   
    kjb::Vector etp(epsilons.size());
    for(int i = 0; i < m_num_dynamics_steps; i++)
    {
        // Perform a full update of the parameters
        // First compute epsilon x p_star
        std::transform(epsilons.begin(), epsilons.end(), p_star.begin(),
                       etp.begin(), std::multiplies<double>());

        if(CONSTRAINED_TARGET)
        {
            // These are needed to handle constraints
            lower_bounds = get_lower_bounds(q_star);
            upper_bounds = get_upper_bounds(q_star);
        }

        // should move_parameters do this loop in one step?
        // Could be faster in some implementations...
        // --Kyle, Feb 6 2011
        for(size_t d = 0; d < hmc_dim; d++)
        {
            double mpb;
            if(!CONSTRAINED_TARGET)
            {
                mpb = etp[d];
            }
            else
            {
                // HANDLING CONSTRAINTS
                // We need to fix the position and momentum until they stop
                // violating constraints. See Neal for details.
                double q_d_p, q_d;
                do
                {
                    q_d = get_parameter(q_star, d);
                    q_d_p = q_d + etp[d];
                    if(q_d_p < lower_bounds[d])
                    {
                        q_d_p = (2 * lower_bounds[d] - q_d_p);
                        p_star[d] *= -1;
                    }

                    if(q_d_p > upper_bounds[d])
                    {
                        q_d_p = (2 * upper_bounds[d] - q_d_p);
                        p_star[d] *= -1;
                    }
                }while(q_d_p < lower_bounds[d] || q_d_p > upper_bounds[d]);
                mpb = q_d_p - q_d;
            }
            move_parameter(q_star, d, mpb);
        }

        // default: no-op
        post_move_callback(q_star, p_star);

        // if (last_iteration && don't care about final value of p)
        if(i == m_num_dynamics_steps - 1 && m_last_p_ignore) 
        {
            /* do nothing */

            // OPTIMIZATION BRANCH
            // Don't bother performing the final gradient evaluation, because either
            // (a) the final momentum will be discarded, or 
            // (b) the final half-update of momentum was rolled into a full
            //     initial momentum update.
            // In either case, this is no longer true MCMC, but could be "close enough,"
            // and the benefits to running time may be worth it.
        }
        else
        {
            // update grad_U x epsilon.
            grad_x_eps = -1.0 * compute_gradient(q_star);
            epsilons = get_step_size(q_star);
            std::transform(grad_x_eps.begin(), grad_x_eps.end(), epsilons.begin(),
                           grad_x_eps.begin(), std::multiplies<double>());

        }

        // Make a full step for the momentum, except at the end of the trajectory 
        if(i != m_num_dynamics_steps - 1)
        {
            p_star -= grad_x_eps;
        }
    }

    if(m_last_p_ignore)
    {
        /* Do nothing */
        // OPTIMIZATION BRANCH (see above)
    }
    else
    {
        // Make a half step for momentum at the end. 
        p_star -= (grad_x_eps / 2.0);
    }

    // Negate momentum at end of trajectory to make the proposal symmetric
    p_star *= -1.0;

    //////////////////////////////////////////////////////////////
    //                  Accept or reject                        //
    //////////////////////////////////////////////////////////////

    double accept_prob;
    double U_q_star = -l_target(q_star);  // necessary even if accept_step = true, because sampler needs to know the log-target of the final model
    double U_q = -lt_q;

    // TODO: may need to divide by step size here...
    double K_p_star = p_star.magnitude_squared() / 2.0;
    double K_p = p.magnitude_squared() / 2.0;

    if(INCLUDE_ACCEPT_STEP)
    {

        // compute acceptance probability
        accept_prob = (U_q - U_q_star + K_p - K_p_star) / m_temperature;
    }
    else
    {
        // at least 0 ensures acceptance
        accept_prob = 0.00;
    }

    double r = std::log(kjb::sample(kjb::Uniform_distribution(0, 1)));
    bool accepted;
    double lt_final;

    if(r < accept_prob)
    {
        // update the position and momentum
        // Note: specialize swap to get best performance 
        using std::swap;
        swap(q, q_star);
        swap(p, p_star);

        // update log(target)
        lt_final = -U_q_star;
        accepted = true;
    }
    else
    {
        // Everything stays the same
        lt_final = lt_q;
        accepted = false;
    }

    // Negate momentum to avoid random walk.
    // true reversal of momentum only occurs when rejection occurs.
    // note: if alpha is not 0, this makes the step non-reversible
    p *= -1.0;

    Step_result<Model> result("hmc", accepted, lt_final);

    if(record_extra())
    {
        result.extra["hmc_u_rev"] = U_q;
        result.extra["hmc_u_fwd"] = U_q_star;
        result.extra["hmc_k_rev"] = K_p;
        result.extra["hmc_k_fwd"] = K_p_star;
        result.extra["hmc_accept"] = accept_prob;
    }

    return Step_log<Model>(result);
}

/**
 * @class Basic_hmc_step
 *
 * @tparam Model        The model type. Must comply with HmcModel concept.
 * @tparam accept_step  Do a accept/reject step? This is true by default,
 *                      which, if alpha = 0, makes this a true HMC step.
 *
 * Basic_hmc_step is a functor that runs a single step of the generalized Hybrid
 * Monte Carlo algorithm on a model.
 *
 * This class is a subclass of the abstract class Abstract_hmc_step. It implements the
 * get_* member functions from the abstract class in the obvious way: it returns
 * references to private members of the correct types. These members are initialized
 * from references received in the constructor.
 */
template<typename Model,
         bool CONSTRAINED_TARGET = false,
         bool INCLUDE_ACCEPT_STEP = true,
         bool REVERSIBLE = true>
class Basic_hmc_step : public Abstract_hmc_step<Model, CONSTRAINED_TARGET, INCLUDE_ACCEPT_STEP, REVERSIBLE>
{
public:
    typedef Abstract_hmc_step<Model, CONSTRAINED_TARGET,
                              INCLUDE_ACCEPT_STEP, REVERSIBLE> Parent;

    typedef typename Parent::Target_distribution Target_distribution;
    typedef typename Parent::Gradient Gradient;
    typedef typename Parent::Move_parameter Move_parameter;
    typedef typename Parent::Get_step_size Get_step_size;
    typedef typename Parent::Get_dimension Get_dimension;

    // constraint handling stuff
    typedef typename Parent::Get_parameter Get_parameter;
    typedef typename Parent::Get_lower_bounds Get_lower_bounds;
    typedef typename Parent::Get_upper_bounds Get_upper_bounds;

    /**
     * @brief   Creates a Basic_hmc_step by initializing target and
     *          proposer functors (or functions) to the given arguments.
     * 
     * @param log_target    Target_distribution object used to initialize 
     *                      internal target distribution used in operator().
     *
     * @param num_dynamics_steps    Number of dynamics steps to take before
     *                              accepting/rejecting.
     *
     * @param gradient  Mechanism to compute the gradient of the (log) target
     *                  distribution.
     *
     * @param move_parameter    Mechanism to change a parameter of the
     *                          of the model by a certain amount.
     *
     * @param get_step_size Mechanism to get the etas; that is the
     *                      sizes of the neighborhood used to compute
     *                      the gradients.
     *
     * @param get_dimension Mechanism to obtain the dimension of the
     *                      model.
     *
     * @param alpha Parameter which controls how much randomness there is
     *              in the update of the momentum.
     */
    Basic_hmc_step
    (
        const Target_distribution& log_target, 
        int                        num_dynamics_steps,
        const Gradient&            gradient,
        const Move_parameter&      move_parameter,
        const Get_step_size&       get_step_size,
        const Get_dimension&       get_dimension,
        double                     alpha = 0.0
    ) :
        Parent(num_dynamics_steps, alpha),
        m_log_target(log_target),
        m_gradient(gradient),
        m_move_parameter(move_parameter), 
        m_get_step_size(get_step_size),
        m_get_dimension(get_dimension),
        m_record_extra(false),
        m_get_parameter(),
        m_get_lower_bounds(),
        m_get_upper_bounds()
    {}

    Basic_hmc_step
    (
        const Target_distribution& log_target, 
        int                        num_dynamics_steps,
        const Gradient&            gradient,
        const Move_parameter&      move_parameter,
        const Get_step_size&       get_step_size,
        const Get_dimension&       get_dimension,
        const Get_parameter&       get_parameter,
        const Get_lower_bounds&     get_lower_bounds,
        const Get_upper_bounds&     get_upper_bounds,
        double                     alpha = 0.0
    ) :
        Parent(num_dynamics_steps, alpha),
        m_log_target(log_target),
        m_gradient(gradient),
        m_move_parameter(move_parameter), 
        m_get_step_size(get_step_size),
        m_get_dimension(get_dimension),
        m_record_extra(false),
        m_get_parameter(get_parameter),
        m_get_lower_bounds(get_lower_bounds),
        m_get_upper_bounds(get_upper_bounds)
    {}

    /**
     * Enable/disable recording of extra metadata in Step_log
     */
    void record_extra(bool enable)
    {
        m_record_extra = enable;
    }

//    /**
//     * @brief   Copy-constructor
//     */
//    Basic_hmc_step(const Basic_hmc_step<Model>& step) :
//        Parent(step),
//        m_log_target(step.m_log_target),
//        m_gradient(step.m_gradient),
//        m_move_parameter(step.m_move_parameter), 
//        m_get_step_size(step.m_get_step_size),
//        m_get_dimension(step.m_get_dimension)
//    {}

//    /**
//     * @brief   Assignment
//     */
//    Basic_hmc_step& operator=(const Basic_hmc_step<Model>& step)
//    {
//        if(&step != this)
//        {
//            Parent::operator=(step);
//            m_log_target = step.m_log_target;
//            m_gradient = step.m_gradient;
//            m_move_parameter = step.m_move_parameter; 
//            m_get_step_size = step.m_get_step_size;
//            m_get_dimension = step.m_get_dimension;
//        }
//
//        return *this;
//    }

    void set_post_move_callback(const boost::function2<void, Model&, kjb::Vector&>& f)
    {
        m_post_move_callback = f;
    }

    virtual ~Basic_hmc_step(){}

    virtual const Target_distribution& get_log_target() const{ return m_log_target; }

    virtual const Gradient& get_gradient() const{ return m_gradient; }

    virtual const Move_parameter& get_move_parameter() const{ return m_move_parameter; }

    virtual const Get_step_size& get_step_size_function() const{ return m_get_step_size; }

    virtual const Get_dimension& get_dimension_function() const{ return m_get_dimension; }

    /// if true is returned, the step log will contain metadata about the step
    virtual bool record_extra() const 
    {
        return m_record_extra;
    }

    // \/ \/ for constraint handling \/ \/
    virtual const Get_parameter& get_parameter_function() const{ return m_get_parameter; }

    virtual const Get_lower_bounds& get_lower_bounds_function() const{ return m_get_lower_bounds; }

    virtual const Get_upper_bounds& get_upper_bounds_function() const{ return m_get_upper_bounds; }

    virtual void post_move_callback(Model& q, kjb::Vector& p) const
    {
        if(m_post_move_callback)
            return m_post_move_callback(q,p);
    }

protected:
    Target_distribution m_log_target;
    Gradient m_gradient;
    Move_parameter m_move_parameter;
    Get_step_size m_get_step_size;
    Get_dimension m_get_dimension;

    bool m_record_extra;

    // constraint handling
    Get_parameter m_get_parameter;
    Get_lower_bounds m_get_lower_bounds;
    Get_upper_bounds m_get_upper_bounds;

    boost::function2<void, Model&, kjb::Vector&> m_post_move_callback;
};

/**
 * @class Basic_sd_step
 *
 * @tparam Model    The model type. Must comply with HmcModel concept.
 *
 * Basic_sd_step is a functor that runs a single step of the stochastic
 * dynamics algorithm (i.e., an HMC step without an accept/reject step).
 */
template<class Model, bool REVERSIBLE = true>
struct Basic_sd_step
{
    typedef Basic_hmc_step<Model, false, false, REVERSIBLE> Type;
};


#endif /*SAMPLE_STEP_H_INCLUDED */

