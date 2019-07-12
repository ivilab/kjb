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

/* $Id: prob_util.cpp 21596 2017-07-30 23:33:36Z kobus $ */

/*!
 * @file prob_util.cpp
 *
 * @author Colin Dawson 
 * @author Jinyan Guan
 */

#include "prob_cpp/prob_util.h"
#include "m_cpp/m_vector.h"
#include "m_cpp/m_matrix.h"

#include <boost/math/special_functions/gamma.hpp>

namespace kjb{

    Vector log_normalize(const Vector& vec)
    {
        Vector result;
        double total = log_sum(vec.begin(), vec.end());
        for(Vector::const_iterator it = vec.begin(); it != vec.end(); ++it)
        {
            result.push_back((*it) - total);
        }
        return result;
    }

    Vector ew_exponentiate(const Vector& vec)
    {
        Vector result;
        for(Vector::const_iterator it = vec.begin(); it != vec.end(); ++it)
        {
            result.push_back(exp(*it));
        }
        return result;
    }
    
    Vector ew_log(const Vector& vec)
    {
        Vector result;
        for(Vector::const_iterator it = vec.begin(); it != vec.end(); ++it)
        {
            result.push_back(log(*it));
        }
        return result;
    }
    
    Vector log_normalize_and_exponentiate(const Vector& vec)
    {
        Vector result;
        double total = log_sum(vec.begin(), vec.end());
        for(Vector::const_iterator it = vec.begin(); it != vec.end(); ++it)
        {
            result.push_back(exp((*it) - total));
        }
        return result;
    }
    
    Matrix ew_exponentiate(const Matrix& m)
    {
        size_t num_rows = m.get_num_rows();
        size_t num_cols = m.get_num_cols();
        Matrix result(num_rows, num_cols);
        for(size_t i = 0; i < num_rows * num_cols; ++i)
        {
            result[i] = exp(m[i]);
        }
        return result;
    }
    
    Matrix ew_log(const Matrix& m)
    {
        size_t num_rows = m.get_num_rows();
        size_t num_cols = m.get_num_cols();
        Matrix result(num_rows, num_cols);
        for(size_t i = 0; i < num_rows * num_cols; ++i)
        {
            result[i] = log(m[i]);
        }
        return result;
    }
    
    Matrix log_normalize_and_exponentiate(const Matrix& mat)
    {
        size_t num_rows = mat.get_num_rows();
        size_t num_cols = mat.get_num_cols();
        Matrix result(num_rows, num_cols);
        for(size_t i = 0; i < num_rows; ++i)
        {
            Vector row = mat.get_row(i);
            result.set_row(i, log_normalize_and_exponentiate(row));
        }
        return result;
    }

    Matrix log_normalize_rows(const Matrix& mat)
    {
        size_t num_rows = mat.get_num_rows();
        size_t num_cols = mat.get_num_cols();
        Matrix result(num_rows, num_cols);
        for(size_t i = 0; i < num_rows; ++i)
        {
            Vector row = mat.get_row(i);
            result.set_row(i, log_normalize(row));
        }
        return result;
    }

    Vector log_marginalize_over_rows(const Matrix& mat)
    {
        int num_cols = mat.get_num_cols();
        Vector result;
        for(int c = 0; c < num_cols; ++c)
        {
            Vector column = mat.get_col(c);
            result.push_back(log_sum(column.begin(), column.end()));
        }
        return result;
    }

    Vector log_marginalize_over_cols(const Matrix& mat)
    {
        int num_rows = mat.get_num_rows();
        Vector result;
        for(int r = 0; r < num_rows; ++r)
        {
            Vector row = mat.get_row(r);
            result.push_back(log_sum(row.begin(), row.end()));
        }
        return result;
    }
};

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

double kjb::multivariate_gamma_function(size_t D, double val)
{
    /*double gamma_D = std::pow(M_PI, D * (D-1)/4.0);
    for(int i = 1; i <= D; i++)
    {
        double temp = val + (1 - i)/2.0;
        gamma_D *= boost::math::tgamma(temp);
    }
    return gamma_D; */
    // USE the log gamma version to prevent overlow 
    return std::exp(log_multivariate_gamma_function(D, val));
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

double kjb::log_multivariate_gamma_function(size_t D, double val)
{
    double log_gamma_D = D * (D-1)/4.0 * std::log(M_PI); 
    for(int i = 1; i <= D; i++)
    {
        log_gamma_D += boost::math::lgamma(val + (1 - i)/2.0);
    }
    return log_gamma_D;
}
