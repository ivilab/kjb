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

/* $Id: prob_pdf.cpp 21596 2017-07-30 23:33:36Z kobus $ */

/** @file
 *
 * @author Kobus Barnard
 * @author Ernesto Brau
 * @author Jinyan Guan 
 *
 * @brief Definition of the non-inline pdf and cdf functions
 */

#include "prob_cpp/prob_pdf.h"
#include "prob_cpp/prob_distribution.h"
#include "l_cpp/l_exception.h"
#include "m_cpp/m_vector.h"
#include "m_cpp/m_matrix.h"
#include "n_cpp/n_solve.h"

#include <cmath>

#include <boost/math/special_functions/gamma.hpp>

namespace kjb {

double pdf(const MV_gaussian_distribution& P, const Vector& x)
{
    const Vector& mu = P.mean;
    const Matrix& sigma = P.cov_mat;
    double f;
    int k = mu.get_length();

    IFT(x.get_length() == k, Dimension_mismatch,
        "Cannot compute joint pdf: vector has incorrect dimension.");

    if(P.type == MV_gaussian_distribution::INDEPENDENT)
    {
        f = 1.0;
        for(int i = 0; i < k; i++)
        {
            f *= pdf(Gaussian_distribution(mu[i], sqrt(sigma(i, i))), x[i]);
        }
    }
    else
    {
        Vector y = x - mu;
        P.update_cov_inv();
        P.update_abs_det();
        double msm = dot(y, P.cov_inv * y);
        f = (1.0 / sqrt(pow(2 * M_PI, k) * P.abs_det)) * exp(-0.5 * msm);
    }

    return f;
}

/* /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ */

double log_pdf(const MV_gaussian_distribution& P, const Vector& x)
{
    const Vector& mu = P.mean;
    const Matrix& sigma = P.cov_mat;
    double f;
    int k = mu.get_length();

    IFT(x.get_length() == k, Dimension_mismatch,
        "Cannot compute joint pdf: vector has incorrect dimension.");

    if(P.type == MV_gaussian_distribution::INDEPENDENT)
    {
        f = 0.0;
        for(int i = 0; i < mu.get_length(); i++)
        {
            f += log_pdf(Gaussian_distribution(mu[i], sqrt(sigma(i, i))), x[i]);
        }
    }
    else
    {
        Vector y = x - mu;
        P.update_cov_inv();
        P.update_log_abs_det();
        double msm = dot(y, P.cov_inv * y);
        f = -(0.5 * msm) - (0.5 * P.log_abs_det)
                - ((k * std::log(2 * M_PI)) / 2.0);
    }

    return f;
}

/* /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ */

double pdf
(
    const Chinese_restaurant_process& cpr,
    const Chinese_restaurant_process::Type& B
)
{
    double th = cpr.concentration();
    const size_t n = cpr.num_customers();

    double p = 1.0;
    p *= tgamma(th);
    p *= pow(th, B.size());
    p /= tgamma(th + n);

    for(size_t b = 0; b < B.size(); b++)
    {
        p *= tgamma(B[b].size());
    }

    return p;
}

/* /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ */

double log_pdf
(
    const Chinese_restaurant_process& cpr,
    const Chinese_restaurant_process::Type& B
)
{
    const double th = cpr.concentration();
    const size_t n = cpr.num_customers();

    double p = 0.0;
    p += lgamma(th);
    p += B.size() * log(th);
    p -= lgamma(th + n);

    for(size_t b = 0; b < B.size(); b++)
    {
        p += lgamma(B[b].size());
    }

    return p;
}

/* /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ */

double pdf(const Dirichlet_distribution& dist, const Vector& val)
{
    IFT(val.size() == dist.alphas().size(), Illegal_argument, 
            "val must has the same dimension as alphas");
    // use the log_pdf to prevent overflow 
    return exp(log_pdf(dist, val));

}

/* /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ */

double log_pdf(const Dirichlet_distribution& dist, const Vector& val)
{
    IFT(val.size() == dist.alphas().size(), Illegal_argument, 
            "val must has the same dimension as alphas");
    double res = 0.0;
    for(size_t i = 0; i < val.size(); i++)
    {
        res += (dist.alphas()[i] - 1) * log(val[i]);
    }
    res -= dist.sum_log_gamma_alphas_;
    res += dist.log_gamma_sum_alphas_;
    return res;
}

/* /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ */

double pdf(const Wishart_distribution& dist, const Matrix& val)
{
    IFT(val.get_num_cols() == dist.D_, Illegal_argument,
            "val must have the same number of columns as S_");
    IFT(val.get_num_rows() == dist.D_, Illegal_argument,
            "val must have the same number of rows as S_");
  
    Matrix temp = val * dist.S_.inverse(); 
    return std::pow(val.abs_of_determinant(), (dist.nu_ - dist.D_ - 1)/2.0) *
           std::exp(-0.5 * temp.trace()) / dist.Z_; 
}

/* /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ */

double log_pdf(const Wishart_distribution& dist, const Matrix& val)
{
    IFT(val.get_num_cols() == dist.D_, Illegal_argument,
            "val must have the same number of columns as S_");
    IFT(val.get_num_rows() == dist.D_, Illegal_argument,
            "val must have the same number of rows as S_");
 
    Matrix temp = val * dist.S_.inverse();
    return (dist.nu_ - dist.D_ - 1) /2.0 * std::log(val.abs_of_determinant()) - 
           0.5 * temp.trace() - dist.log_Z_; 

}

/* /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ */

double pdf(const Inverse_wishart_distribution& dist, const Matrix& val)
{
    IFT(val.get_num_cols() == dist.D_, Illegal_argument,
            "val must have the same number of columns as S_");
    IFT(val.get_num_rows() == dist.D_, Illegal_argument,
            "val must have the same number of rows as S_");
  
    Matrix temp = dist.S_.inverse() * val.inverse();
    return std::pow(val.abs_of_determinant(), -(dist.nu_ + dist.D_ + 1)/2.0) *
           std::exp(-0.5 * temp.trace()) / dist.Z_; 
}

/* /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ */

double log_pdf(const Inverse_wishart_distribution& dist, const Matrix& val)
{
    IFT(val.get_num_cols() == dist.D_, Illegal_argument,
            "val must have the same number of columns as S_");
    IFT(val.get_num_rows() == dist.D_, Illegal_argument,
            "val must have the same number of rows as S_");
 
    Matrix temp = dist.S_.inverse() * val.inverse();
    return -(dist.nu_ + dist.D_ + 1) /2.0 * std::log(val.abs_of_determinant()) - 
           0.5 * temp.trace() - dist.log_Z_; 
}

} //namespace kjb

