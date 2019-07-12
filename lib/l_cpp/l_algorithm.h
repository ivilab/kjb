/* $Id: l_algorithm.h 21776 2017-09-17 16:44:49Z clayton $ */
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

#ifndef KJB_L_CPP_ALGORITHM_H
#define KJB_L_CPP_ALGORITHM_H

#include "l/l_sys_debug.h"  /* For ASSERT */
/**
 * @file Functions similar in spirit to those in std's "algorithm" header 
 * (e.g. copy, generate, fill, etc)
 */
#include "l_cpp/l_functors.h"
#include "l_cpp/l_std_parallel.h"
#include "l_cpp/l_exception.h"

#include <map>
#include <iterator>
#include <algorithm>
#include <assert.h>
#include <cfloat>
#include <algorithm>
#include <vector>
#include <utility>

//#include <boost/lambda/lambda.hpp>
#include "l_cpp/l_functors.h"
//#include <boost/bind.hpp>
//#include <boost/function.hpp>
#include <boost/concept_check.hpp>
//#include <boost/iterator/transform_iterator.hpp>
//#include <boost/iterator/indirect_iterator.hpp>

namespace kjb
{

/**
 * Linearly interpolate a set of reference values.  
 *
 * Iterators represent a set of uniformly-spaced reference values, with the first
 * element corresponding to 0.0, and the last element corresponding to 1.0. 
 * Iterators may refer to any value that is closed under self-addition and
 * multiplication by scalar double.
 *
 * @param begin Iterator to first element of reference set
 * @param end Iterator to one-after-last element of reference set
 * @param x Value to interpolate; should be scaled to be within [0.0, 1.0].  
 *          Values outside this range will be clamped to [0.0, 1.0].
 *
 * Tags: Linear interpolation 
 */
template <class Iterator, class Real>
typename std::iterator_traits<Iterator>::value_type lerp(Iterator begin, Iterator end, Real x)
{
    typedef typename std::iterator_traits<Iterator>::value_type value_type; 
    x = std::max(Real(0.0), x);
    x = std::min(x, Real(1.0));

    int max_index = std::distance(begin, end) - 1;
    int integer = x * max_index;
    Real rational = x * max_index - integer;

    ASSERT(integer >= 0);
    ASSERT(integer <= (int) max_index);
    ASSERT(rational >= 0.0);
    ASSERT(rational < 1.0);

    if(integer == (int) max_index) 
    {
        ASSERT(rational < FLT_EPSILON);
        Iterator it = begin;
        std::advance(it, integer);
        return *it;
    }

    Iterator lower = begin;
    std::advance(lower, integer);

    Iterator upper = lower;
    ++upper;

    value_type interp = (1.0 - rational) * (*lower) + rational * (*upper);

    return interp;
}

/**
 * Evaluate a piecewise-linear function defined by a set of input/output samples.
 *
 * Function is specified as two random-acess iterators: one containing function inputs, the other containing function outputs.  Inputs must be partially ordered.  The both types must be equality and less-than comparable.
 *
 * @param in_begin begin iterator to function input values
 * @param in_end end itertor to function input values
 * @param out_begin begin iterator to function output values
 * @param out_end end iterator to function output values
 *
 * @param query_point A point in the domain of this function to evaluate 
 * @returns The function value evaluated at query point, interpolated between the two nearest keypoints
 *
 * @tparam Query_type Input point; must be convertible to Key_type
 *
 * @throws Index_out_of_bounds if query_point lies outside of the domain of the given function.
 */
template <class IIterator, class OIterator, class QueryType>
typename std::iterator_traits<OIterator>::value_type
lerp(IIterator ibegin, IIterator iend, OIterator obegin, const QueryType& query_point)
{
    size_t n = std::distance(ibegin, iend);
    OIterator oend = obegin + n;

    typedef typename std::iterator_traits<IIterator>::value_type InputType ;
    typedef typename std::iterator_traits<OIterator>::value_type OutputType;

    BOOST_CONCEPT_ASSERT((boost::Convertible<QueryType, InputType>));

    if(n == 0)
        KJB_THROW_2(Illegal_argument, "Function is empty.");

    IIterator iit = ibegin;
    std::advance(iit, n-1);

    const InputType& lower_bound = *ibegin;
    const InputType& upper_bound = *iit;
    // out of range
    if(!(lower_bound <= query_point && query_point <= upper_bound))
        KJB_THROW(Index_out_of_bounds);

    iit = std::upper_bound(ibegin, iend, query_point);
    const InputType& iupper = *iit;
    const InputType& ilower = *(--iit);

    size_t dist = std::distance(ibegin, iit);
    // get output lower/upper values
    OIterator oit = obegin;
    std::advance(oit, dist);
    const OutputType& olower = *oit;

    if(query_point == ilower)
        return olower;

    const OutputType& oupper = *(++oit);

    const InputType offset = query_point - ilower;
    const InputType  range_size = iupper - ilower;
    return olower + offset * (oupper - olower) / range_size;
}

/**
 * Evaluate a piecewise-linear function defined by a set of input/output samples.  This
 * version is optimized to perform multiple evaluations in a single pass, assuming the
 * query points are ordered.  If query points aren't ordered, behavior is undefined.
 *
 * @param ibegin begin iterator to function input values
 * @param iend end itertor to function input values
 * @param obegin begin iterator to function output values (an inputIterator)
 * @param i2begin begin iterator to set of ordered query points
 * @param i2end end iterator to set of ordered query points
 * @param o2begin output iterator for set of linearly interpolated output values.
 */
template <class IIterator, class OIterator, class I2Iterator, class O2Iterator>
void ordered_lerp(
        IIterator ibegin,
        IIterator iend,
        OIterator obegin,
        I2Iterator i2begin,
        I2Iterator i2end,
        O2Iterator o2begin)
{
    size_t n = std::distance(ibegin, iend);
    OIterator oend = obegin + n;

    size_t n2 = std::distance(i2begin, i2end);
    O2Iterator o2end = o2begin + n2; 

    typedef typename std::iterator_traits<IIterator>::value_type InputType;
    typedef typename std::iterator_traits<OIterator>::value_type OutputType;
    typedef typename std::iterator_traits<I2Iterator>::value_type Input2Type;
    // NOT_USED typedef typename std::iterator_traits<O2Iterator>::value_type Output2Type;

    BOOST_CONCEPT_ASSERT((boost::Convertible<InputType, Input2Type>));
    // NOTE_USED  BOOST_CONCEPT_ASSERT((boost::Convertible<OutputType, Output2Type>));

    if(n == 0)
        KJB_THROW_2(Illegal_argument, "Function is empty.");
    if(n2 == 0)
        return;

    IIterator iit = ibegin;
    std::advance(iit, n-1);

    IIterator i2it = i2begin;
    std::advance(i2it, n2-1);

    {
        const InputType& lower_bound = *ibegin;
        const InputType& upper_bound = *iit;

        // get range of query inputs
        const Input2Type& query_lower_bound = *i2begin;
        const Input2Type& query_upper_bound = *i2it;

        // out of range
        if(!(lower_bound <= query_lower_bound && query_upper_bound <= upper_bound))
            KJB_THROW(Index_out_of_bounds);

    }

    iit = ibegin;
    OIterator oit = obegin;

    i2it = i2begin;
    O2Iterator o2it = o2begin;

    // handle one or more query points equal to the lower bound.
    while(*i2it == *iit)
    {
        *o2it++ = *oit;
        *i2it++;
    }

    InputType* prev_input;
    OutputType* prev_output;
    Input2Type range_size;

    // fact: *i2it > *iit
    
    for(; i2it != i2end; ++i2it)
    {
        const Input2Type& query = *i2it;

        // make sure iit is the upper bound of the range we're inside
        // will always be true for the first iteration:
        if(query > *iit)
        {
            while(query > *iit)
            {
                prev_input = &*iit;
                prev_output = &*oit;
                ++iit;
                ++oit;
            }
            range_size = *iit - *prev_input;
        }

        // special case: exact hit
        if(query == *iit)
        {
            *o2it++ = *oit;
            continue;
        }

        // we are now between the lower and upper bound
        // 1. find out how far above minimum
        Input2Type distance = query - static_cast<Input2Type>(*prev_input);

        // 2. convert to percent
        distance /= range_size;

        // 3. lerp it
        *o2it++ = *prev_output + distance * (*oit - *prev_output);
    }
}

/**
 * Evaluate a piecewise-linear function defined by a set of input/output samples.
 *
 * Function is specified as a key-value map containing function outputs for various sampled inputs.  The key-values must be equality and less-than comparable.
 *
 * @param piecewise_function A set of key-value pairs representing input/output samples/changepoints
 * @param query_point A point in the domain of this function to evaluate 
 * @returns a key, value pair containing the interpolated input and output values.
 *
 * @tparam Query_type Input point; must be convertible to Key_type
 *
 * @throws Index_out_of_bounds if query_point lies outside of the domain of the given function.
 */
template <class Key_type, class Value_type, class Query_type>
Value_type lerp(const std::map<Key_type, Value_type>& piecewise_function, const Query_type& query_point)
{
    typedef std::map<Key_type, Value_type> Data_set;
    typedef typename Data_set::const_iterator Iterator;
    const Data_set& data_set = piecewise_function;

    BOOST_CONCEPT_ASSERT((boost::Convertible<Query_type, Key_type>));

    if(data_set.empty())
        KJB_THROW_2(Illegal_argument, "Function is empty.");

    // out of range
    if(!(data_set.begin()->first <= query_point && query_point <= data_set.rbegin()->first))
        KJB_THROW(Index_out_of_bounds);

    Iterator upper = data_set.upper_bound(query_point);
    Iterator lower = upper;
    advance(upper,  -1);

    if(query_point == lower->first)
        return lower->second;

    const Key_type offset = query_point - lower->first;
    const Key_type range_size = upper->first - lower->first;
    return lower->second + offset * (upper->second - lower->second) / range_size;
}

/**
 * A generator function that fills a range with linearly-spaced values between min and max, inclusive.  The functionality is intended to match that of matlab's linspace built-in.
 *
 * @param begin iterator to output collection
 * @param endpoint include endpoint in interval (closed interval)
 * @author Kyle Simek
 */
template <class ForwardIterator>
void linspace(double min, double max, size_t n, ForwardIterator begin, bool endpoint = true)
{
#if 0 /* This version accumulates precision error the longer the list */
    double incr;
    if(endpoint)
        incr = (max - min) / (n - 1);
    else
        incr = (max - min) / n;

    ForwardIterator end = begin;
    std::advance(end, n);
    std::generate(begin, end, Increase_by<double>(min, incr));
#else /* This version is a port of matlab's linspace, incurs no drift */
    double last = (endpoint ? n-1 : n);

    ForwardIterator it = begin;
    for(size_t i = 0; i < n-1; ++i)
        *it++ = min + i / last * (max - min);
    *it++ = max;
#endif
}

/**
 * Emulates matlab's logspace operator
 */
template <class ForwardIterator>
void logspace(double min, double max, size_t n, ForwardIterator begin)
{
    linspace(min, max, n, begin);

    ForwardIterator end = begin;
    std::advance(end, n);
    std::transform(begin, end, begin, std::bind1st(std::ptr_fun(static_cast<double(*)(double, double)>(pow)), 10));
}

/**
 * Alternative version of linspace that recieves a transforms function which
 * operates on the the linearly-spaced values.
 *
 * Bounary points will be included in the output and passed to the provided function.
 *
 
 * Input type must implement multiplication-by-scalar and addition.
 *
 * The function will always be evaluated at the extrema, therefore N must be 
 * at least 2.
 *
 * @param lower One end of the input range (not necessarilly lower)
 * @param upper
 * @param N numer of positions along range to evaluate (must be >= 2)
 * @param out_begin Iterator to output sequence.  Must have size at least N
 * @param f function to evaluate.  Must receive one argument of type InType, and return type must be the value_type of the output iterator.
 * @
 */
template <class InType, class OutIterator, class UnaryOperator>
void linspace(
        const InType& lower,
        const InType& upper,
        size_t N,
        const OutIterator& out_begin,
        const UnaryOperator& f)
{
    const InType* range[2];
    range[0] = &lower;
    range[1] = &upper;

    // create real-valued inputs
    std::vector<double> x(N);
    linspace(0.0, 1.0, N, x.begin());
//
//    // create a transform function converts real-valued inputs to
//    // InType-valued inputs.
//    boost::function1<InType, double> lerp = boost::bind(
//            static_cast< InType (*)(boost::indirect_iterator<InType const**> , boost::indirect_iterator<InType const**>, double)> (&::kjb::lerp),
//            boost::indirect_iterator<InType const **>(range),
//            boost::indirect_iterator<InType const **>(range+2),
//            _1);
//
//    // evaluate function at each point
//    transform(
//        boost::make_transform_iterator(x.begin(), lerp),
//        boost::make_transform_iterator(x.end(), lerp),
//        out_begin,
//        f);
//
//    OutIterator out_it = out_begin;
//    for(size_t i = 0; i < x.size(); ++i)
//    {
//        *out_it++ = f(::kjb::lerp(lower, upper, x[i]));
//    }

    OutIterator it = out_begin;
    InType diff = upper - lower;
#pragma omp parallel for
    for(size_t i = 0; i < x.size(); ++i)
        *it++ = f(lower + x[i] * diff);
}


/**
 * A generic version of linspace that operates on a generic type, using
 * linear interpolation.  Input type must imlement addition and 
 * multiplication-by-scalar.
 *
 * @param min One end of value range (not necessarilly lower than max)
 * @param max Other end of value range
 * @param n number of values to output (min and max are always output, so n must be >= 2)
 * @param begin iterator to output collection
 *
 * @note min does not need to be less than max.  if max < min, the sequence will simply be descending.
 *
 * @tparam InType Type to be interpolated
 * @tparam  OutIterator pointer to output colleciton.  InType must be
 *          convertible to OutIterator::value_type must be 
 * @author Kyle Simek
 */
template <class InType, class OutIterator>
void linspace(InType min, InType max, size_t n, OutIterator begin)
{
    ::kjb::linspace(
            min,
            max,
            n,
            begin,
            Identity<InType>()); // identity function
}
} // namespace kjb


namespace kjb_parallel
{
/**
 * Parallel version of kjb::linspace.  If openmp isn't enabled,
 * this is identical to kjb::linspace
 *
 * @sa kjb::linspace
 */
template <class InType, class OutIterator, class UnaryOperator>
void linspace(
        const InType& lower,
        const InType& upper,
        size_t N,
        const OutIterator& out_begin,
        const UnaryOperator& f)
{
    const InType* range[2];
    range[0] = &lower;
    range[1] = &upper;

    // create real-valued inputs
    std::vector<double> x(N);
    ::kjb::linspace(0.0, 1.0, N, x.begin());
//
//    // create a transform function converts real-valued inputs to
//    // InType-valued inputs.
//    boost::function1<InType, double> lerp = boost::bind(
//            static_cast< InType (*)(boost::indirect_iterator<InType const**> , boost::indirect_iterator<InType const**>, double)> (&::kjb::lerp),
//            boost::indirect_iterator<InType const **>(range),
//            boost::indirect_iterator<InType const **>(range+2),
//            _1);
//
//    // evaluate function at each point
//    transform(
//        boost::make_transform_iterator(x.begin(), lerp),
//        boost::make_transform_iterator(x.end(), lerp),
//        out_begin,
//        f);
//
//#pragma omp parallel for
//    for(size_t i = 0; i < x.size(); ++i)
//    {
//        out_begin[i] = f(lerp(lower, upper, x[i]));
//    }

    OutIterator it = out_begin;
    InType diff = upper - lower;
#pragma omp parallel for
    for(size_t i = 0; i < x.size(); ++i)
        *it++ = f(lower + x[i] * diff);
}

} // namespace kjb_parallel

#endif
