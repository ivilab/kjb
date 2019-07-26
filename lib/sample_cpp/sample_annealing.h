/* $Id: sample_annealing.h 12839 2012-08-08 23:51:05Z ksimek $ */
/* {{{=========================================================================== *
   |
   |  Copyright (c) 1994-2011 by Kobus Barnard (author)
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
   |  Author:  Kyle Simek
 * =========================================================================== }}}*/

// vim: tabstop=4 shiftwidth=4 foldmethod=marker
#include <sample_cpp/sample_sampler.h>
#include <sample_cpp/sample_step.h>
#include <sample_cpp/sample_concept.h>

template <class Model>
class Annealing_sampler : public Multi_step_sampler<Model>
{
    typedef Multi_step_sampler<Model> Base;
    typedef boost::function1<void, double> Set_temperature_callback;

    // Copying this sampler will cause aliasing of the underlying steps, and
    // there's no obvious way to implemnt an array of arbitrary
    // concepts without this occurring.  For now, lets be safe and disable copying
    Annealing_sampler(const Annealing_sampler& s) { }
    Annealing_sampler& operator=(const Annealing_sampler& s) { return *this; }

public:
    using Base::add_step;

    /**
     * @param initial_state Model to initialize the sampler with
     * @param initial_log_target Log of the value of the initial_state under the target distribution
     */
   Annealing_sampler( 
            const Model& initial_state,
            double initial_log_target) :
        Base(initial_state, initial_log_target),
        temperature_(1.0)
    {}

    template <class StepType>
    void add_annealing_step(const StepType& step, double prob, const std::string& name = "")
    {
        BOOST_CONCEPT_ASSERT((Annealable<StepType>));

        boost::shared_ptr<StepType> tmp(new StepType(step));
        add_annealing_step(tmp, prob, name);
    }


    template <class StepType>
    void add_annealing_step(StepType* step, double prob, const std::string& name = "")
    {
        BOOST_CONCEPT_ASSERT((Annealable<StepType>));

        add_annealing_step(boost::shared_ptr<StepType>(step, Base::null_deleter()), prob, name);
    }

    template <class StepType>
    void add_annealing_step(const boost::shared_ptr<StepType>& step, double prob, const std::string& name = "")
    {
        BOOST_CONCEPT_ASSERT((Annealable<StepType>));

        step->set_temperature(temperature_);

        m_temperature_setters.push_back(boost::bind(&StepType::set_temperature, step.get(), _1));

        Base::add_step(step, prob, name);
    }

    /// Add an arbitrary function to be called when temperature changes
    void add_temperature_changed_callback(const boost::function1<void, double>& cb)
    {
        m_temperature_setters.push_back(cb);
    }

//   template <class StepType>
//    void add_step(const typename boost::reference_wrapper<StepType>& step_ref, double prob, const std::string& name)
//    {
//        BOOST_CONCEPT_ASSERT((Annealable<StepType>));
//
//        // probably unnecessary:
//        m_any_steps.push_back(step_ref);
//        
//        Base::add_step(step_ref, prob, name);
//
//        Set_temperature_callback set_temp;
//        set_temp = boost::bind(&StepType::set_temperature, step_ref, _1);
//
//        m_temperature_setters.push_back(set_temp);
//        m_temperature_setters.back()(temperature_);
//    }

   void set_temperature(double temperature)
   {
       if(temperature_ == temperature) return;

       temperature_ = temperature;
       for(size_t i = 0; i < m_temperature_setters.size(); i++)
       {
           m_temperature_setters[i](temperature_);
       }
   }
private:
    std::vector<Set_temperature_callback> m_temperature_setters;
    double temperature_;
};


template <class Proposer, class Model>
class Annealing_proposer_wrapper
{
public:
    BOOST_CONCEPT_ASSERT((ModelProposer<Proposer, Model>));

    Annealing_proposer_wrapper(const Proposer& p) :
        p_(p)
    {}

    Mh_proposal_result operator()(const Model& in, Model& out) const
    {
        return p_(in, out);
    }

    // ignore temperature
    void set_temperature(double t) {} 
private:
    const Proposer p_;
};

/**
 * Same as Basic_mh_sampler, but adds a public set_temperature()
 * method, which affects the accept/reject rate.  
 *
 * Proposer must comply with the Annealable concept, i.e. the 
 * "set_temperature()" method must be available.  Non-annealing
 * proposers can be wrapped with Annealing_proposer_wrapper, which
 * implements a null set_temperature method.
 */
template<typename Model, typename Proposer = typename Mh_model_proposer<Model>::Type >
class Annealing_mh_step : public Basic_mh_step<Model, Proposer>
{
    BOOST_CONCEPT_ASSERT((Annealable<Proposer>));

    typedef Basic_mh_step<Model, Proposer> Base;
    typedef typename Base::Target_distribution Target_distribution;

public:
    Annealing_mh_step(const Target_distribution& log_target, const Proposer& proposer) : 
        Base(log_target, proposer)
    {
        set_temperature(1.0);
    }

    /**
     * @brief   Copy-constructor
     */
    Annealing_mh_step(const Annealing_mh_step<Model, Proposer>& step) :
        Base(step)
    {
        set_temperature(1.0);
    }

    /**
     * @brief   Assignment
     */
    Annealing_mh_step& operator=(const Annealing_mh_step<Model, Proposer>& step)
    {
        Base::operator=(step);
        return *this;
    }


    /** Set annealing temperature of step and proposer */
    void set_temperature(double T)
    {
        Base::set_temperature(T);
        Base::m_proposer.set_temperature(T);
    }
};

