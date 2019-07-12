/* $Id: prob_util.h 20234 2016-01-18 04:22:22Z jguan1 $ */
/* ======================================================================== *
 |                                                                          |
 | Copyright (c) 2007-2010, by members of University of Arizona Computer    |
 | Vision group (the authors) including                                     |
 |       Kobus Barnard, Kyle Simek, Ernesto Brau.                           |
 |                                                                          |
 | For use outside the University of Arizona Computer Vision group please   |
 | contact Kobus Barnard.                                                   |
 |                                                                          |
 * ======================================================================== */


#ifndef PROB_UTIL_H_INCLUDED
#define PROB_UTIL_H_INCLUDED

/** @file
 *
 * @author Ernesto Brau
 * @author Colin Dawson
 *
 * @brief Various utility functions for probabiliity-related stuff
 */

#include <iterator>
#include <algorithm>
#include <cmath>

namespace kjb{

    //forward declarations
    class Vector;
    class Matrix;
    
#if 0
NOTE: I moved this to sample_cpp to resolve circular dependency between prob_util.h and prob_distribution.h.  Besides, this is basically sampling, even
  though its name isnt sample()...
  Kyle May 19, 2012
/**
 * @brief   Pick an element uniformly at random (UAR) from a sequence,
 *          represented by a beginning iterator and a size.
 *
 * @return  An iterator to the randomly chosen element.
 */
template<class Iterator, class distance_type>
inline
Iterator element_uar(Iterator first, distance_type size)
{
    int dist = kjb::sample(kjb::Categorical_distribution<size_t>(0, size - 1, 1));
    Iterator p = first;
    std::advance(p, dist);

    return p;
}
#endif


/**
 * Take the sum of two probabilities represented in log-space without
 * risk of underflow.
 *
 * This is useful for marginalization and normalization of sets of very
 * small numbers.
 *
 * @author Kyle Simek
 */
inline double log_sum(double n1, double n2)
{
/*
 * the implementation below relies on this identity:
 *  
 * log{ x1 + x2 + x3 + ...} =
 *    pi + log(sum_i exp( log xi - pi))
 *
 * where pi = max_i (log(xi))
 */
    double minus_infinity = log(0.0);
    if(n1 < n2)
        std::swap(n1, n2);
    if(n1 == minus_infinity) return minus_infinity;
    return n1 + log(1.0 + exp(n2-n1));
}

/**
 * Take the sum of three probabilities represented in log-space without 
 * risk of underflow.
 *
 * In other words, return log(exp(n1) + exp(n2)), even if exponentiating
 * any of the numbers would normally result in underflow.
 *
 * This is useful for marginalization and normalization of sets of very
 * small numbers.
 *
 * @author Kyle Simek
 */
inline double log_sum(double n1, double n2, double n3)
{
/*
 * the implementation below relies on this identity:
 *  
 * log{ x1 + x2 + x3 + ...} =
 *    pi + log(sum_i exp( log xi - pi))
 *
 * where pi = max_i (log(xi))
 */
    double minus_infinity = log(0.0);
    if(n1 < n2)
        std::swap(n1, n2);
    if(n1 < n3)
        std::swap(n1, n3);
    if(n1 == minus_infinity) return minus_infinity;
    return n1 + log(1.0 + exp(n2-n1) + exp(n3 - n1));
}

/**
 * Take the log-sum of N probabilies represented in log-space without risk
 * of underflow.
 * In other words, return log(exp(n1) + exp(n2)), even if exponentiating
 * any of the numbers would normally result in underflow.
 *
 * This is useful for marginalization and normalization of sets of very
 * small numbers.
 *
 * @author Kyle Simek
 */
template<class Iterator>
double log_sum(Iterator first, Iterator last)
{
/*
 * the implementation below relies on this identity:
 *  
 * log{ x1 + x2 + x3 + ...} =
 *    pi + log(sum_i exp( log xi - pi))
 *
 * where pi = max_i (log(xi))
 *
 */
    double minus_infinity = log(0.0);
    double pi = *std::max_element(first, last);
    if(pi == minus_infinity) return minus_infinity;
    double accum = 0;

    for(; first != last; first++)
    {
        accum += exp(*first - pi);
    }
    return pi + log(accum);
}

/**
 *
 * Take the difference of two probabilities represented in log-space without
 * risk of underflow (copied from the analogous log_sum function; implemented
 * separately to be able to apply over two arrays without needing to multiply
 * everything in the array by -1
 *
 * @author Colin Dawson
 */
inline double log_diff(double n1, double n2)
{
/*
 * the implementation below relies on this identity:
 *  
 * log{ x1 - x2} = log(x2) + log(exp(log(x2/x1) - 1))
 *
 */
    if(n1 < n2)
        std::swap(n1, n2);

    return n2 + log(exp(n2-n1) - 1);
}
    
/**
 * @author Colin Dawson
 * @brief normalize a probability vector in log space
 *
 */
Vector log_normalize(const Vector& vec);
    
/**
 * @author Colin Dawson
 * @brief elementwise exponentiate a vector without normalizing
 *
 */
Vector ew_exponentiate(const Vector& vec);

/**
 * @author Colin Dawson
 * @brief take elementwise log of a vector
 *
 */
Vector ew_log(const Vector& m);

/**
 * @author Colin Dawson
 * @brief normalize and exponentiate a vector of unnormalized log probabilities
 *
 */
Vector log_normalize_and_exponentiate(const Vector& vec);
    
/**
 * @author Colin Dawson
 * @brief normalize the rows of a stochastic matrix in log space
 *
 */
Matrix log_normalize_rows(const Matrix& mat);
    
/**
 * @author Colin Dawson
 * @brief elementwise exponentiate a matrix without normalizing
 *
 */
Matrix ew_exponentiate(const Matrix& m);
    
/**
 * @author Colin Dawson
 * @brief take elementwise log of a matrix
 *
 */
Matrix ew_log(const Matrix& m);
    
/**
 * @author Colin Dawson
 * @brief row-wise normalize and exponentiate a matrix of log probabilities
 *
 */
Matrix log_normalize_and_exponentiate(const Matrix& mat);

/**
 * @author Colin Dawson
 * @brief marginalize down the columns of a matrix of log probabilities
 *
 */
Vector log_marginalize_over_cols(const Matrix& mat);
    
/**
 * @author Colin Dawson
 * @brief marginalize across the rows of a matrix of log probabilities
 *
 */
Vector log_marginalize_over_rows(const Matrix& mat);

/**
 * @author Jinyan Guan
 * @brief Return the multiviarate gamma function of a random variable 
 *        see https://en.wikipedia.org/wiki/Multivariate_gamma_function
 *        for the formula
 */
double multivariate_gamma_function(size_t D, double val);

/**
 * @author Jinyan Guan
 * @brief Return the LOG value of the multiviarate gamma function 
 *        of a random variable 
 */
double log_multivariate_gamma_function(size_t D, double val);
    
}; // namespace kjb

#endif /*PROB_UTIL_H_INCLUDED */

