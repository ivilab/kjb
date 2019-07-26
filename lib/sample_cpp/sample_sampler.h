/* $Id: sample_sampler.h 18278 2014-11-25 01:42:10Z ksimek $ */
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

#ifndef SAMPLE_SAMPLER_H_INCLUDED
#define SAMPLE_SAMPLER_H_INCLUDED

#include "sample_cpp/sample_step.h"
#include "sample_cpp/sample_base.h"
#include "sample_cpp/sample_recorder.h"

#include <boost/lambda/lambda.hpp>
#include <boost/function.hpp>
#include <l_cpp/l_index.h>
#include <vector>

/**
 * @class Abstract_sampler
 *
 * @tparam Model The model type.  Must comply with BaseModel concept.
 *
 * A generic sampler.  Concrete classes must implement:
 *  
 *  (1) choose_step() - returns a sampler step function to run.
 *  (2) get_recorder() - returns a recorder object for keeping track of accepted samples.
 *
 * As far as the steps are concerned, they need only comply with the SamplerStep
 * concept. Similarly, the recorder must comply with ModelRecorder concept.
 */

template<typename Model>
class Abstract_sampler
{
private:
    typedef boost::function0<void> Callback;
public:
    typedef typename Sampler_step<Model>::Type Step;
    typedef Model Model_type;
    //typedef typename Sampler_callback<Model>::Type Callback;
    
    /**
     * @param initial_state Initial model to start the sampler with.
     * @param initial_log_target The log of the target distribution at intitial_state
     */
    Abstract_sampler(
            const Model& initial_state,
            double initial_log_target) :
        m_cur_model(initial_state),
        m_cur_log_target(initial_log_target)
    {}

    BOOST_CONCEPT_ASSERT((BaseModel<Model>));

    /** @brief Runs the sampler. This function runs the sampler
     *  for the indicated number of iterations. At each iteration
     *  it chooses a sampler step using choose_step() and invokes
     *  it using its operator(). The best model is saved and
     *  returned.
     *
     *  @param initial_state The initial value of the model; e.g.,
     *  the initial state of the Markov chain in MH.
     *
     *  @param num_iterations The number of iterations the sampler
     *  will do.
     */
    virtual void run(int num_iterations);
    
    /** 
     * Return the current state of the sampler.
     */
    Model& current_state()
    {
        return m_cur_model;
    }

    /** 
     * Return the current state of the sampler. (const version)
     */
    const Model& current_state() const
    {
        return m_cur_model;
    }

    /**
     * Return the value of the log-target distribution evaluated at
     * the current state (cached value, no cost incurred)
     */
    double current_log_target() const
    {
        return m_cur_log_target;
    }

    template <class Recorder>
    void add_recorder(Recorder r)
    {
        BOOST_CONCEPT_ASSERT((ModelRecorder<Recorder>));
        m_record.add(r);
    }

    template <class Recorder>
    void add_recorder(Recorder* r)
    {
        BOOST_CONCEPT_ASSERT((ModelRecorder<Recorder>));
        m_record.add(r);
    }

    template <class Callback>
    void add_record_callback(Callback cb)
    {
        add_recorder(Callback_recorder<Model>(cb));
    }

    //virtual const char* get_type() const{ return "Abstract_sampler"; }

//    template <class Recorder>
//    Recorder& get_recorder(size_t i)
//    { 
//        BOOST_CONCEPT_ASSERT((ModelRecorder<Recorder>));
//        return m_record.get_recorder<Recorder>(i);
//    }

    template <class Recorder>
    const Recorder& get_recorder(size_t i) const
    { 
        BOOST_CONCEPT_ASSERT((ModelRecorder<Recorder>));
        return m_record.template get_recorder<Recorder>(i);
    }

    /**
     * Add a callback to be called if the next/current step is accepted
     * This callback will be cleared at the end of the sampling iteration.
     */
    void if_accept(const Callback& cb)
    {
        using namespace boost::lambda;
        if(m_if_accept.empty())
            m_if_accept = cb;
        else
            // comma operator of boost::lambda chains two functions together
            m_if_accept = (m_if_accept , cb);
    }

    /**
     * Add a callback to be called if the next/current step is rejected.
     * This callback will be cleared at the end of the sampling iteration.
     */
    void if_reject(const Callback& cb)
    {
        using namespace boost::lambda;
        if(m_if_reject.empty())
            m_if_reject = cb;
        else
            // comma operator of boost::lambda chains two functions together
            m_if_reject = (m_if_reject , cb);
    }

    /**
     * Add a callback to be called every time a step is accepted.
     * If you want a callback to be called only if the _next_ step is
     * accepted, use if_accept().
     *
     * If this is called multiple times, the callbacks will be called
     * in sequence in the order they were added.
     *
     * At the moment there is no way to remove a callback after it has
     * been added.  This could be added in the future if there is interest.
     */
    void on_accept(const Callback& cb)
    {
        using namespace boost::lambda;
        if(m_on_accept.empty())
            m_on_accept = cb;
        else
            // comma operator of boost::lambda chains two functions together
            m_on_accept = (m_on_accept , cb);
    }

    /**
     * Add a callback to be called every time a step is rejected.
     * If you want a callback to be called only if the _next_ step is
     * rejected, use if_reject()
     *
     * If this is called multiple times, the callbacks will be called
     * in sequence in the order they were added.
     *
     * At the moment there is no way to remove a callback after it has
     * been added.  This could be added in the future if there is interest.
     */
    void on_reject(const Callback& cb)
    {
        using namespace boost::lambda;
        if(m_on_reject.empty())
            m_on_reject = cb;
        else
            // comma operator of boost::lambda chains two functions together
            m_on_reject = (m_on_reject , cb);
    }

protected:

    /** @brief Chooses a sampler step. Pure virtual method, should
     *  be overwritten.
     */
    virtual const Step& choose_step() const = 0;

private:
    Model m_cur_model;
    double m_cur_log_target;
    Multi_model_recorder<Model> m_record;
    //Callback pre_callback;
    //Callback post_callback;

    // one-off callbacks (cleared every iteration)
    Callback m_if_accept;
    Callback m_if_reject;

    // persistent callbacks (called every iteration)
    Callback m_on_accept;
    Callback m_on_reject;
};

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

template<typename Model>
void Abstract_sampler<Model>::run(int num_iterations)
{
    for(int i = 1; i <= num_iterations; i++)
    {
        const Step& step = choose_step();
        Step_log<Model> log = step(m_cur_model, m_cur_log_target);
        m_cur_log_target = log[0].lt;
        m_record(m_cur_model, log);

        if(log.back().accept)
        {
            if(!m_on_accept.empty())
                m_on_accept();
            if(!m_if_accept.empty())
                m_if_accept();
        }
        else
        {
            if(!m_on_reject.empty())
                m_on_reject();
            if(!m_if_reject.empty())
                m_if_reject();
        }

        // clear non-persisitent callbacks
        m_if_accept.clear();
        m_if_reject.clear();
    }
};

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

/**
 * @class Multi_step_sampler
 *
 * @tparam Model The model type.  Must comply with BaseModel concept.
 *
 * A sampler that chooses from a set of moves distributed under a categorical
 * distribution.
 */
template<typename Model>
class Multi_step_sampler : public Abstract_sampler<Model>
{
protected:
    struct null_deleter
    {
        void operator()(const void*) const{}
    };
public:
    typedef Abstract_sampler<Model> Parent;

    //typedef typename Sampler_callback<Model>::Type Callback;
    //
    typedef typename Abstract_sampler<Model>::Step Step;


    BOOST_CONCEPT_ASSERT((BaseModel<Model>));

    /**
     * @param initial_state Model to initialize the sampler with
     * @param initial_log_target Log of the value of the initial_state under the target distribution
     */
    Multi_step_sampler(
            const Model& initial_state,
            double initial_log_target) :
        Parent(initial_state, initial_log_target),
        m_steps(),
        m_step_callbacks(),
        m_name(),
        m_probabilities()
    {}

    /**
     * Initialize with a single step, whose probability will be 1.0.
     *
     * @param st A single step to use for all iterations
     * @param initial_state Model to initialize the sampler with
     * @param initial_log_target Log of the value of the initial_state under the target distribution
     */
    template <class StepType>
    Multi_step_sampler(
            const StepType& st,
            const Model& initial_state,
            double initial_log_target) : 
        Parent(initial_state, initial_log_target),
        m_steps(),
        m_step_callbacks(),
        m_name(),
        m_probabilities()
    {
        add_step(st, 1.0);
    }

    /**
     * Initialize with a list of steps and associated probabilities
     *
     * @param st A single step to use for all iterations
     * @param initial_state Model to initialize the sampler with
     * @param initial_log_target Log of the value of the initial_state under the target distribution
     */
    Multi_step_sampler(
            const std::vector<Step>& step_list,
            const std::vector<double>& prob_list,
            const Model& initial_state,
            double initial_log_target) :
        Parent(initial_state, initial_log_target),
        m_steps(), 
        m_step_callbacks(),
        m_name(),
        m_probabilities()
    {
        assert(step_list.size() == prob_list.size());
        for(size_t i = 0; i < step_list.size(); ++i)
        {
            add_step(step_list[i], prob_list[i]);
        }
    }

    /**
     * @brief   Add a new step with associated probability. This does NOT make
     *          sure probabilities add up to 1. It is up to the user to
     *          do that.
     */
   template <class StepType>
    void add_step(const StepType& step, double prob, const std::string& name = "")
    {
        BOOST_CONCEPT_ASSERT((boost::CopyConstructible<StepType>));

        boost::shared_ptr<StepType> tmp(new StepType(step));
        add_step(tmp, prob, name);
    }


    /**
     * @brief   Add a new step with associated probability. This does NOT make
     *          sure probabilities add up to 1. It is up to the user to
     *          do that.
     */
   template <class StepType>
    void add_step(StepType* step, double prob, const std::string& name = "")
    {
        BOOST_CONCEPT_ASSERT((boost::CopyConstructible<StepType>));

        add_step(boost::shared_ptr<StepType>(step, null_deleter()), prob, name);
    }

    /**
     * @brief   Add a new step with associated probability. This does NOT make
     *          sure probabilities add up to 1. It is up to the user to
     *          do that.
     */
    template <class StepType>
    void add_step(const boost::shared_ptr<StepType>& step, double prob, const std::string& name = "")
    {
        m_steps.push_back(step);
        m_step_callbacks.push_back(boost::ref(*step));
        m_name.push_back(name);
        m_probabilities.push_back(prob);
    }

    /**
     * Retrieve step
     */
    template <class StepType>
    const StepType& get_step(size_t i) const
    {
        return *boost::any_cast<boost::shared_ptr<StepType> >(m_steps.at(i));
    }

    //virtual const char* get_type() const
    //{ return "Multi_step_sampler"; }

protected:
    virtual const Step& choose_step() const
    {
        assert(m_steps.size() > 0);
        std::vector<size_t> indices = kjb::Index_range(0, m_steps.size() - 1).expand();

        // TODO: make this object a member to avoid constructing every time
        kjb::Categorical_distribution<size_t> P(
                indices, 
                m_probabilities);
        size_t s = kjb::sample(P);
        return m_step_callbacks[s];
    }

private:
    std::vector<boost::any> m_steps;
    std::vector<Step> m_step_callbacks;
    std::vector<std::string> m_name;
    std::vector<double> m_probabilities;
};


/**
 * @class Single_step_sampler
 *
 * @tparam Model The model type.  Must comply with BaseModel concept.
 *
 * A sampler with a single step type. In other words,
 * the choose_step member function simply returns the same step.
 */

template<typename Model>
class Single_step_sampler : public Abstract_sampler<Model>
{
public:
    typedef Abstract_sampler<Model> Parent;

    typedef typename Abstract_sampler<Model>::Step Step;

    BOOST_CONCEPT_ASSERT((BaseModel<Model>));


    /** @brief Constructs an object of this type from a MH step
     *
     *  @param step The Basic_mh_step that this sampler will
     *  run.
     *  @param initial_state The model to intialize the sample with
     *  @param initial_log_target The log probability of the initial_state 
     *         model under the target distribution.
     */
    Single_step_sampler(
            const Step& step,
            const Model& initial_state,
            double initial_log_target) :
        Parent(initial_state, initial_log_target),
        m_step(step)
    {}

protected:

    /**
     * @brief   Returns the step given in the
     *          constructor.
     */
    virtual const Step& choose_step() const
    {
        return m_step;
    }

private:
    Step m_step;
};

#endif /*SAMPLE_SAMPLER_H_INCLUDED */

