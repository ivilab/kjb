/* $Id: prob_stat.h 21596 2017-07-30 23:33:36Z kobus $ */
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


#ifndef KJB_PROB_STAT
#define KJB_PROB_STAT
/**
 * @file generic functions for computing statistics.
 */

#include "l_cpp/l_exception.h"

#include <boost/concept_check.hpp>
#include <boost/function.hpp>
#include <iterator>
#include <numeric>
#include <algorithm>
#include <functional>
#include <vector>
#include <iterator>

#include "prob_cpp/prob_histogram.h"
#include "prob_cpp/prob_distribution.h"
#include "prob_cpp/prob_pdf.h"

namespace kjb 
{

/**
 * Compute the algebraic mean of a collection of elements.  Note that since the Null-value
 * in general is unknown, the collection must have at least one element.
 *
 * This version of mean is called by the simpler version, mean(begin, end).  This version
 * receives a value-type as a third parameter, allowing users to overload this signature
 * for specific Value_type's.  
 *
 * @precondition  difference(begin, end) > 0
 */
    template <class Iterator, class Value_type>
//typename std::iterator_traits<Iterator>::value_type 
Value_type
mean(Iterator begin, Iterator end, const Value_type& /* dummy */)
{
    using namespace boost;
    BOOST_CONCEPT_ASSERT((DefaultConstructible<Value_type>));
    BOOST_CONCEPT_ASSERT((InputIterator<Iterator>));

//    typedef typename std::iterator_traits<Iterator>::value_type value_type;
    typedef typename std::iterator_traits<Iterator>::difference_type difference_type;
    if(begin == end)
    {
        KJB_THROW_2(Illegal_argument, "Can't take mean of collection of zero size.");
    }

    difference_type size = std::distance(begin, end);

    const Value_type& first = *begin;
    std::advance(begin, 1);
    Value_type sum = std::accumulate(begin, end, first);

    return sum /= size;
}

/**
 * Compute the algebraic mean of a collection of elements.  Note that since the Null-value
 * in general is unknown, the collection must have at least one element.
 *
 * This function simply dispatches to a more type-specific version 
 * mean(Iterator begin, Iterator end, Value_type dummy).  Users wishing to specialize mean 
 * based on the value-type should overload the other version, since the
 * value-type is in the signature.
 *
 * @precondition  difference(begin, end) > 0
 */
template <class Iterator>
typename std::iterator_traits<Iterator>::value_type 
mean(Iterator begin, Iterator end = Iterator())
{
    using namespace boost;

    //typedef typename std::iterator_traits<Iterator>::value_type Value_type;

    BOOST_CONCEPT_ASSERT((InputIterator<Iterator>));

    return mean(begin, end, *begin);
}

/* ============================================================================ *
 *                                                                              *
 *                           STATISTICAL TESTING STUFF                          *
 *                                                                              *
 * ============================================================================ */

/* template<class Iterator>
struct Compute_statistic
{
}; */

/**
 * Performs a goodness-of-fit test based on the given continuous distribution. The
 * statistic used must follow a chi-square distribution.
 *
 * TODO: make more generic: the statistic should not be chi-square-distributed.
 *
 * @param   first       First element of sample set.
 * @param   last        Last element of sample set.
 * @param   dist        Distribution of interest.
 * @param   statistic   Returns a chi-square distributed statistic given two histograms.
 * @param   test        Test used; must follow chi-square distribution.
 * @param   alpha       Threshold for passing the test.
 * @param   num_params  number of parameters of this distribution.
 * @param   num_bins    Number of bins to use to compute histograms.
 *
 * @note    The range of samples must contain at least 10 elements.
 */
template<class InputIterator, class Distribution, class Statistic>
bool goodness_of_fit_test
(
    InputIterator first,
    InputIterator last,
    const Distribution& dist,
    const Statistic& statistic,
    //const boost::function3<double, InputIterator, InputIterator, InputIterator>& statistic,
    double alpha,
    int num_params,
    int num_bins
)
{
    if(num_params + 1 >= num_bins)
    {
        KJB_THROW_2(Runtime_error, "Not enough bins to perform goodness-of-fit test.");
    }

    if(alpha <= 0 || alpha >= 1)
    {
        KJB_THROW_2(Runtime_error, "The p-value must be in (0,1).");
    }

    int N = std::distance(first, last);
    if(N < 10)
    {
        KJB_THROW_2(Runtime_error, "Not enough data to perform test.");
    }

    Histogram h(first, last, num_bins);
    std::vector<int> expected;
    std::vector<int> observed;

    for(std::map<double, int>::const_iterator pair_p = h.as_map().begin(); pair_p != h.as_map().end(); pair_p++)
    {
        if(pair_p->second < 0)
        {
            KJB_THROW_2(Runtime_error, "Histogram values cannot negative.");
        }

        if(pair_p->second > 0)
        {
            double low_lim = pair_p->first;
            double up_lim = low_lim + h.bin_size();
            int val = round(N * (cdf(dist, up_lim) - cdf(dist, low_lim)));
            expected.push_back(val + (val == 0 ? 1 : 0));
            observed.push_back(pair_p->second);
        }
    }

    int num_ne_bins = expected.size();
    int dof = num_ne_bins - (num_params + 1);
    double X = statistic(observed.begin(), observed.end(), expected.begin());

    // william's correction
    double q = 1 + ((num_ne_bins + 1) / (6 * N));
    double X_alpha = quantile(Chi_square_distribution(dof), 1 - alpha);

    return !((X / q) > X_alpha);
}

/**
 * Performs a goodness-of-fit test based on the given discrete distribution. The
 * statistic used must follow a chi-square distribution.
 *
 * TODO: make more generic: the statistic should not be chi-square-distributed.
 *
 * @param   first       First element of sample set.
 * @param   last        Last element of sample set.
 * @param   dist        Distribution of interest.
 * @param   statistic   Returns a chi-square distributed statistic given two histograms.
 * @param   alpha       Threshold for passing the test.
 * @param   dof_redux   Degrees of freedom reduction.
 */
template<class InputIterator, class Distribution, class Statistic>
bool goodness_of_fit_test
(
    InputIterator first,
    InputIterator last,
    const Distribution& dist,
    const Statistic& statistic,
    double alpha,
    int dof_redux
)
{
    if(alpha <= 0 || alpha >= 1)
    {
        KJB_THROW_2(Runtime_error, "The p-value must be in (0,1).");
    }

    int N = std::distance(first, last);
    if(N < 10)
    {
        KJB_THROW_2(Runtime_error, "Not enough data to perform test.");
    }

    Histogram h(first, last, -1);
    std::vector<int> expected;
    std::vector<int> observed;

    for(std::map<double, int>::const_iterator pair_p = h.as_map().begin(); pair_p != h.as_map().end(); pair_p++)
    {
        if(pair_p->second < 0)
        {
            KJB_THROW_2(Runtime_error, "Histogram values cannot negative.");
        }

        if(pair_p->second > 0)
        {
            int val = round(N * pdf(dist, pair_p->first));
            expected.push_back(val + (val == 0 ? 1 : 0));
            observed.push_back(pair_p->second);
        }
    }

    double X = statistic(observed.begin(), observed.end(), expected.begin());
    double X_alpha = quantile(Chi_square_distribution(observed.size() - dof_redux), 1 - alpha);

    return !(X > X_alpha);
}

/**
 * @brief Compute the chi-square test value for a single element.
 */
inline
double chi2stat_helper(int O, int E)
{
    // With yates' correction
    return ((fabs(O - E) - 0.5) * (fabs(O - E) - 0.5)) / E;
}

/**
 * Compute the Chi-square statistic for two histograms, given by the
 * two ranges. The second range is taken as the true frequencies.
 *
 * @note    All values of both histograms must be positive.
 */
template<class InputIterator>
inline
double chi_square_statistic(InputIterator first1, InputIterator last1, InputIterator first2)
{
    std::vector<double> sum_elems(std::distance(first1, last1));
    std::transform(first1, last1, first2, sum_elems.begin(), std::ptr_fun(chi2stat_helper));

    return std::accumulate(sum_elems.begin(), sum_elems.end(), 0.0);
}

/**
 * @brief Compute the G-test value for a single element.
 */
inline
double gstat_helper(int O, int E)
{
    // Yates' correction
    double O_i = O;
    if(O > E)
    {
        O_i -= 0.5;
    }
    else if(E > O)
    {
        O_i += 0.5;
    }

    return O_i * std::log(O_i / E);
}

/**
 * Compute the G statistic (used in the G-test) for two histograms, given by the
 * two ranges. The second range is taken as the true frequencies.
 *
 * @note    All values of both histograms must be positive.
 */
template<class InputIterator>
inline
double g_statistic(InputIterator first1, InputIterator last1, InputIterator first2)
{
    std::vector<double> sum_elems(std::distance(first1, last1));
    std::transform(first1, last1, first2, sum_elems.begin(), std::ptr_fun(gstat_helper));

    return 2 * std::accumulate(sum_elems.begin(), sum_elems.end(), 0.0);
}

/**
 * Compute the goodness of fit test (using the Chi-square statistic) of a sample
 * with respect to the given continuous distribution.
 *
 * @param   first       First element of sample set.
 * @param   last        Last element of sample set.
 * @param   dist        Distribution of interest.
 * @param   alpha       Threshold for passing the test.
 * @param   num_params  number of parameters of this distribution.
 * @param   num_bins    Number of bins to use to compute histograms.
 */
template<class InputIterator, class Distribution>
inline
bool chi_square_test
(
    InputIterator first,
    InputIterator last,
    const Distribution& dist,
    double alpha,
    int num_params,
    int num_bins
)
{
    //boost::function3<double, InputIterator, InputIterator, InputIterator> stat = &chi_square_statistic;
    typedef std::vector<int>::const_iterator IntIterator;
    boost::function3<double, IntIterator, IntIterator, IntIterator> stat = &chi_square_statistic<IntIterator>;
    return goodness_of_fit_test(first, last, dist, stat, alpha, num_params, num_bins);
}

/**
 * Compute the goodness of fit test (using the Chi-square statistic) of a sample
 * with respect to the given discrete distribution.
 *
 * @param   first       First element of sample set.
 * @param   last        Last element of sample set.
 * @param   dist        Distribution of interest.
 * @param   alpha       Threshold for passing the test.
 * @param   dof_redux   Degrees of freedom reduction.
 */
template<class InputIterator, class Distribution>
bool chi_square_test
(
    InputIterator first,
    InputIterator last,
    const Distribution& dist,
    double alpha,
    int dof_redux
)
{
    //boost::function3<double, InputIterator, InputIterator, InputIterator> stat = &chi_square_statistic;
    typedef std::vector<int>::const_iterator IntIterator;
    boost::function3<double, IntIterator, IntIterator, IntIterator> stat = &chi_square_statistic<IntIterator>;
    return goodness_of_fit_test(first, last, dist, stat, alpha, dof_redux);
}

/**
 * Compute the goodness of fit test (using the G-test) of a sample
 * with respect to the given continuous distribution.
 *
 * @param   first       First element of sample set.
 * @param   last        Last element of sample set.
 * @param   dist        Distribution of interest.
 * @param   alpha       Threshold for passing the test.
 * @param   num_params  number of parameters of this distribution.
 * @param   num_bins    Number of bins to use to compute histograms.
 */
template<class InputIterator, class Distribution>
inline
bool g_test
(
    InputIterator first,
    InputIterator last,
    const Distribution& dist,
    double alpha,
    int num_params,
    int num_bins
)
{
    typedef std::vector<int>::const_iterator IntIterator;
    boost::function3<double, IntIterator, IntIterator, IntIterator> stat = &g_statistic<IntIterator>;
    return goodness_of_fit_test(first, last, dist, stat, alpha, num_params, num_bins);
}

/**
 * Compute the goodness of fit test (using the G-test) of a sample
 * with respect to the given discrete distribution.
 *
 * @param   first       First element of sample set.
 * @param   last        Last element of sample set.
 * @param   dist        Distribution of interest.
 * @param   alpha       Threshold for passing the test.
 * @param   dof_redux   Degrees of freedom reduction.
 */
template<class InputIterator, class Distribution>
bool g_test
(
    InputIterator first,
    InputIterator last,
    const Distribution& dist,
    double alpha,
    int dof_redux
)
{
    typedef std::vector<int>::const_iterator IntIterator;
    boost::function3<double, IntIterator, IntIterator, IntIterator> stat = &g_statistic<IntIterator>;
    return goodness_of_fit_test(first, last, dist, stat, alpha, dof_redux);
}

} // namespace kjb

#endif
