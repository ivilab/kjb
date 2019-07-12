/* $Id: sample_base.h 17393 2014-08-23 20:19:14Z predoehl $ */
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


#ifndef SAMPLE_BASE_H_INCLUDED
#define SAMPLE_BASE_H_INCLUDED

#include <deque>
#include <boost/function.hpp>
#include <limits>
#include <string>
#include <map>
#include <m_cpp/m_vector.h>
#include <boost/optional.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/foreach.hpp>


/*
 * ====================================================
 * GENERAL SAMPLING STUFF
 * ====================================================
 */

/* **********************************************
 * STEP LOGGING
 *
 * These objects a logging system for the outcome
 * of sampling moves.  All moves are required to 
 * return a move log.
 * **********************************************/

/**
 * @struct Step_result
 * @brief Structure for returning results of a single sampling move
 * @note The interface for this may change as we decide to add more information here.
 */
template <class Model>
struct Step_result
{
public:
    Step_result() :
        type(""),
        name(""),
        accept(false),
        lt(0),
        extra(),
        m_p(0)
    {}

    Step_result(const char* type_, bool accept_, double lt_) :
        type(type_),
        name(""),
        accept(accept_),
        lt(lt_),
        extra(),
        m_p(0)
    {}

    Step_result(const std::string& type_, const std::string name_, bool accept_, double lt_) :
        type(type_),
        name(name_),
        accept(accept_),
        lt(lt_),
        extra(),
        m_p(0)
    {}

    Step_result(const std::string& type_, const std::string name_, bool accept_, double lt_, const Model* m) :
        type(type_),
        name(name_),
        accept(accept_),
        lt(lt_),
        extra(),
        m_p(m)
    {}

    // step type (mh, hmc, other)
    std::string type;
    // step name (freeform, useful when many different steps of same type, or different sub-types of the same step, e.g. birth/death)
    std::string name;

    // accepted or not?
    bool accept;
    // log target
    double lt;

    // freeform key/value pairs
    std::map<std::string, double> extra;

    // pointer to proposed model
    const Model* m_p;
};

template <class Model>
inline std::ostream& operator<<(std::ostream& ost, const Step_result<Model>& result)
{
    std::string tmp = result.type;
    boost::trim(tmp);
    assert(!tmp.empty());

    // output full precision
    ost.precision(std::numeric_limits<double>::digits10);

    // TYPE
    tmp = result.type;
    boost::trim(tmp);
    if(tmp.empty())
//        ost << ". ";  // TODO: uncomment this after NIPS
        ost << ".";
    else
//        ost << tmp << " "; // TODO: uncomment this after NIPS
        ost << tmp << "";

    // NAME
    tmp = result.name;
    boost::trim(tmp);
    if(tmp.empty())
        ost << ". ";
    else
        ost << tmp << " ";

    ost << result.accept << " ";

    ost << result.lt;

    typedef std::map<std::string, double>::value_type Map_pair;

    BOOST_FOREACH(const Map_pair& pair, result.extra)
    {
        ost << ' ' << pair.first << '=' << pair.second;
    }

    return ost;
}


/** 
 * A deque of move summaries with concatenation functionality.
 * All sampling moves must return a Step_log describing what
 * happened.
 */
template <class Model>
class Step_log : public std::deque<Step_result<Model> >
{
public:
    Step_log() : std::deque<Step_result<Model> >(){}

    /** @brief construct from a single result object */
    explicit Step_log(const Step_result<Model>& result) : 
        std::deque<Step_result<Model> >(1, result)
    {}

    Step_log<Model>& operator+=(const Step_log<Model>& op)
    {
        this->insert(this->end(), op.begin(), op.end());
        return *this;
    }
};

template <class Model>
inline std::ostream& operator<<(std::ostream& ost, const Step_log<Model>& log)
{
    typename Step_log<Model>::const_iterator it = log.begin();
    for(; it != log.end(); it++)
    {
        ost << *it;

        if(it != log.end() - 1)
        {
            ost << "\n";
        }
    }

    return ost;
}


/* **********************************************
 * MODEL FUNCTIONS
 *
 * These types will permit passing both functions
 * and function objects to sampling algorithms.
 *
 * These all use the "Templated typedef" pattern
 * see: http://www.gotw.ca/gotw/079.htm
 *
 * **********************************************/

/**
 * @struct Model_evaluator
 *
 * Evaluates a model, returning a double.
 */
template <typename Model>
struct Model_evaluator
{
    typedef boost::function1<double, const Model&> Type;
};

/* **********************************************
 * SAMPLER FUNCTIONS
 *
 * These types will permit passing both functions
 * and functors as steps to a sampler.
 *
 * **********************************************/

template <typename Model>
struct Sampler_step
{
    typedef boost::function2<Step_log<Model>, Model&, double> Type;
};

/* **********************************************
 * OTHER
 * **********************************************/

/**
 * @struct Model_recorder
 *
 * After each sampler iteration, a Model_recorder is called to record the
 * current state of the model.
 *
 * Some possible application: Optimization (only record the current best),
 * archiving (record everything), integration.  
 *
 * @see sampler_recorder.h
 */
template <typename Model>
struct Model_recorder
{
   typedef typename boost::function2<void, const Model&, const Step_log<Model>&> Type;
};


/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

/*
 * ====================================================
 * INDEXABLE MODEL STUFF
 * ====================================================
 */

/**
 * @struct Get_model_parameter
 *
 * @brief   Gets the specified parameter of a model. For now, we assume
 *          all parameters are type double.
 */
template <typename Model>
struct Get_model_parameter
{
    typedef typename boost::function2<double, const Model&, size_t> Type;
};

/**
 * @struct Set_model_parameter
 *
 * @brief   Sets the specified parameter of a model. For now, we assume
 *          all parameters are type double.
 */
template <typename Model>
struct Set_model_parameter
{
    typedef typename boost::function3<void, Model&, size_t, double> Type;
};

/**
 * @struct Move_model_parameter
 *
 * @brief   Moves the specified parameter of a model. For now, we assume
 *          all parameters are type double.
 */
template <typename Model>
struct Move_model_parameter
{
    typedef typename boost::function3<void, Model&, size_t, double> Type;
};

/**
 * @struct  Model_dimension
 *
 * @brief   Returns the dimension of the model.
 */
template <typename Model>
struct Model_dimension
{
    typedef typename boost::function1<size_t, const Model&> Type;
};

/**
 * @struct Model_parameter_evaluator
 *
 * Evaluates each parameter of a model, returning a vector
 * of doubles (one for each parameter).
 */
template <typename Model>
struct Model_parameter_evaluator
{
    typedef boost::function1<kjb::Vector, const Model&> Type;
};


/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

/*
 * ====================================================
 * METROPOLIS SAMPLING STUFF
 * ====================================================
 */

/**
 * @brief   Indicates the result of an MH proposal. It is
 *          simply a pair of probabilities, forward and
 *          backward.
 */
struct Mh_proposal_result
{
public:
    Mh_proposal_result(const std::string& type_ = std::string("")) : fwd_prob(0.0), rev_prob(0.0), type(type_), no_change(false)
    {}

    /**
     * @param fp log forward proposal probability
     * @param rp log reverse proposal probability
     * @param type_ optional description of proposal (no spaces)
     */
    Mh_proposal_result(double fp, double rp, const std::string& type_ = std::string("")) : 
        fwd_prob(fp),
        rev_prob(rp),
        type(type_),
        no_change(false)
    {}

    /// named constructor - returns a proposal result indicating the model was not changed
    static Mh_proposal_result unchanged(const std::string& type_ = std::string(""))
    {
        Mh_proposal_result result(type_);
        result.no_change = true;
        return result;
    }

    bool operator==(const Mh_proposal_result& pr)
    {
        return fwd_prob == pr.fwd_prob && rev_prob == pr.rev_prob && no_change == pr.no_change;
    }

    bool operator!=(const Mh_proposal_result& pr)
    {
        return !this->operator==(pr);
    }

    Mh_proposal_result& operator+=(const Mh_proposal_result& pr)
    {
        fwd_prob += pr.fwd_prob;
        rev_prob += pr.rev_prob;

        return *this;
    }

     /// log forward proposal probability
    double fwd_prob;
    /// log reverse proposal probability
    double rev_prob;

    std::string type; ///< Name of proposal type

    /// proposal didn't modify the model
    bool no_change;
};

inline Mh_proposal_result operator+(const Mh_proposal_result& pr1, const Mh_proposal_result& pr2)
{
    Mh_proposal_result result(pr1);
    result += pr2;
    return result;
}


/**
 * @struct Mh_model_proposer
 *
 * Model_proposer is called at every iteration of a sampler; it proposes
 * a new model given an old one.
 */
template <typename Model>
struct Mh_model_proposer
{
    typedef boost::function2<Mh_proposal_result, const Model&, Model&> Type;
};

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

/*
 * ====================================================
 * GIBBS SAMPLING STUFF
 * ====================================================
 */

/**
 * @struct Gibbs_model_proposer
 *
 * Gibbs_model_proposer is called at every iteration of a Gibbs sampler; it
 * modifies the model by changing only one variable at a time.
 */
template <typename Model>
struct Gibbs_model_proposer
{
    typedef boost::function2<boost::optional<double>, Model&, size_t> Type;
};


#endif /*SAMPLE_BASE_H_INCLUDED */

