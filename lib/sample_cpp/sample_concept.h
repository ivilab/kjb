/* $Id: sample_concept.h 17393 2014-08-23 20:19:14Z predoehl $ */
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

#ifndef SAMPLE_CONCEPT_H_INCLUDED
#define SAMPLE_CONCEPT_H_INCLUDED

#include <boost/concept_check.hpp> 
#include <boost/concept_archetype.hpp> 
#include <sample_cpp/sample_base.h>


// Forward declarations 
// (should we just include sample_base.h?)
//struct Mh_proposal_result;
//template<class Model> class Step_log<Model>;

/**
 * Concept Requirements:
 *  
 *  Syntax:
 *      <li> Default constructible
 *      <li> Assignable
 */
template <class X>
struct BaseModel
{
    BOOST_CONCEPT_ASSERT((boost::Assignable<X>));
    //BOOST_CONCEPT_ASSERT((boost::DefaultConstructible<X>));
};


template <class Func, class Model>
struct ModelEvaluator :
    boost::UnaryFunction<Func, double, const Model&>
{
public:
    // Arg must be a BaseModel
    BOOST_CONCEPT_ASSERT((BaseModel<Model>));
};



template <class Func, class Model>
struct ModelProposer :
    boost::BinaryFunction<Func, Mh_proposal_result, const Model&, Model&>
{
public:
    // Model must be a BaseModel
    BOOST_CONCEPT_ASSERT((BaseModel<Model>));
};


/**
 * @TODO document this
 */
template <class X>
struct VectorModel : public BaseModel<X>
{
    typedef typename X::value_type Type;

    BOOST_CONCEPT_USAGE(VectorModel)
    {
        X* i;
        const X* c_i;

        const Type& c_v = (*c_i)[0];
        Type& v = (*i)[0];
        size_t size = dimensionality(v);
    }
};

template <class T>
size_t dimensionality(const T& v)
{
    return v.size();
}

/**
 * Model capable of being passed to a Gibbs step.
 */
/* template <class X>
struct GibbsModel : public BaseModel<X>
{
    BOOST_CONCEPT_USAGE(GibbsModel)
    {
        const X* c_i;

        size_t size = gibbs_size(*c_i);
    }
}; */

/**
 * Model capable of being passed to a HMC step.
 */
/* template <class X>
struct HmcModel : public BaseModel<X>
{
    BOOST_CONCEPT_USAGE(HmcModel)
    {
        const X* c_i;

        size_t size = hmc_size(*c_i);
    }
}; */

template <class X>
struct ModelRecorder
{
public:
    typedef typename X::Value_type Value_type;
    typedef typename X::Model_type Model;

    // Arg must be a BaseModel
    BOOST_CONCEPT_ASSERT((BaseModel<Model>));

    BOOST_CONCEPT_ASSERT((boost::BinaryFunction<X, void, const Model&, const Step_log<Model>&>));

    BOOST_CONCEPT_USAGE(ModelRecorder)
    {
        const X* i = NULL;
        const Value_type& vr = i->get(); // must have "get" function that returns const-reference to value_type;

        boost::ignore_unused_variable_warning(vr);
    }
};

template <class X>
struct Updatable
{
    // must have a typedef ::update_type
    typedef typename X::Update_type Update_type;

    BOOST_CONCEPT_USAGE(Updatable)
    {
        // must have a method called "update" that receives update_type
        i->update(*u);
    }

private:
    X* i;
    const Update_type* u;
};

template <class X>
struct Annealable
{
    BOOST_CONCEPT_USAGE(Annealable)
    {
        // must have a method called "set_temperature" that receives a double
        double t;
        i->set_temperature(t);
    }

private:
    X* i;
};


class base_model_archetype
{
public:
    typedef base_model_archetype self;

    base_model_archetype(){};
    self& operator=(const self&){ return *this; }
};


class vector_model_archetype : public base_model_archetype
{
public:
    typedef double Value_type;
    const Value_type& operator[](unsigned int /*i*/) const
    {
        static Value_type result = 0.0;
        return result;
    }

    Value_type& operator[](unsigned int /*i*/)
    {
        static Value_type result = 0.0;
        return result;
    }
};

template <class Model>
class model_evaluator_archetype :
    public boost::unary_function_archetype<const Model, double> 
{
private:
    model_evaluator_archetype(){}
public:
    model_evaluator_archetype(boost::detail::dummy_constructor c) : 
        boost::unary_function_archetype<const Model, double>(c){}
};

template <class Model>
class model_proposer_archetype :
    public boost::binary_function_archetype<const Model, Model, Mh_proposal_result>
{
  private:
    model_proposer_archetype(){}
  public:
    model_proposer_archetype(boost::detail::dummy_constructor c) : 
        boost::binary_function_archetype<const Model, Model, Mh_proposal_result>(c){}
};


template <class Model>
class model_recorder_archetype :
    public boost::binary_function_archetype<const Model, const Step_log<Model>, void>
{
  private:
    model_recorder_archetype(){}
  public:
    model_recorder_archetype(boost::detail::dummy_constructor c) : 
        boost::binary_function_archetype<const Model, const Step_log<Model>, void>(c){}
};

#endif /*SAMPLE_CONCEPT_H_INCLUDED */

