/* =========================================================================== *
|
|  Copyright (c) 1994-2008 by Kobus Barnard (author).
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
| Authors:
|     Jinyan Guan
|
* =========================================================================== */
/* $Id: bayesian_linear_regression.h 21596 2017-07-30 23:33:36Z kobus $ */

#ifndef KJB_BAYESIAN_LINEAR_REGRESSION_H
#define KJB_BAYESIAN_LINEAR_REGRESSION_H

#include "l/l_sys_debug.h"  /* For ASSERT */
#include "m_cpp/m_vector.h"
#include "m_cpp/m_matrix.h"
#include "prob_cpp/prob_distribution.h"
#include "prob_cpp/prob_sample.h"
#include "prob_cpp/prob_pdf.h"

#include <string>
#include <limits>
#include <cmath>

namespace kjb
{

enum Variance_type {INVERSE_GAMMA, INVERSE_CHI_SQUARED, INVALID_TYPE};
inline Variance_type get_prior_type(const std::string& name)
{
    if(name == "inverse-gamma") return INVERSE_GAMMA;
    else if(name == "inverse-chi-squared") return INVERSE_CHI_SQUARED;
    else return INVALID_TYPE;
}

/**
 * @brief   A structure that holds the parameters of a Bayesian linear
 *          regression model
 * @param   mu prior mean of the mean
 * @param   V  prior precision of the mean 
 * @param   var_a scale parameter of the variance
 * @param   var_b shape parameter of the variance
 */
struct Blr_param
{
    Vector mu;
    Matrix V;
    double var_a;
    double var_b;

    Vector V_mu;
    double mu_V_mu;

    Blr_param(const Vector& mu_, const Matrix& V_, double a_, double b_)
        : mu(mu_), V(V_), var_a(a_), var_b(b_),
          V_mu(V * mu),
          mu_V_mu(dot(mu * V, mu))
    {}
};

class Bayesian_linear_regression
{
public:

    Bayesian_linear_regression
    (
        const Vector& mu,
        const Matrix& V,
        double var_a,
        double var_b,
        const std::string& prior_type = std::string("inverse-gamma"),
        const Matrix& X = Matrix(),
        const Matrix& X_T = Matrix(),
        const Matrix& X_T_X = Matrix()
    ) :
        prior_param_(Blr_param(mu, V, var_a, var_b)),
        posterior_param_(Blr_param(mu, V, var_a, var_b)),
        nig_prior_(get_nig(prior_param_)),
        nig_posterior_(get_nig(prior_param_)),
        nic_prior_(get_nic(prior_param_)),
        nic_posterior_(get_nic(prior_param_)),
        type_(get_prior_type(prior_type)),
        target_dirty_(true),
        D_(mu.size()),
        X_T_X_(mu.size(), mu.size(), 0.0),
        X_T_y_((int)mu.size(), 0.0),
        N_(0),
        N_2_(0.0),
        y_T_y_(0.0)
    {
        if(type_ == INVALID_TYPE) 
        {
            KJB_THROW_2(Illegal_argument, "prior-type must be either inverse-gamma"
                    " or inverse-chi-squared");
        }
        // check the dimention
        IFT(V.get_num_cols() == D_, Illegal_argument, "Blr V and mu mismatch");
        IFT(V.get_num_rows() == D_, Illegal_argument, "Blr V is not squared matrix");

        // compute the cache
        if(X_T.size() == 0)
        {
            if(X.size() > 0)
            {
                IFT(X.get_num_cols() == D_, Illegal_argument, 
                        "Blr X has wrong dimension");
                N_ = X.get_num_rows();
                X_T_ = X.transpose();
                X_T_X_ = X_T_ * X;
            }
        }
        else // X_T.size() > 0
        { 
            IFT(X_T_X != Matrix(), Illegal_argument, 
                    "Blr X_T_X should also be provided");
            X_T_ = X_T;
            X_T_X_ = X_T_X;
            IFT(X_T_.get_num_rows() == D_, Illegal_argument, 
                    "Blr X_T has wrong dimension");
            N_ = X_T_.get_num_cols();
        }
        N_2_ = N_/2.0;
        ASSERT(X_T_.get_num_cols() == N_);
        ASSERT(X_T_X_.get_num_rows() == D_);
    }

    /** @brief   Update X_T matrix */
    void update_design_matrix(const Matrix& X_T, const Matrix& X_T_X) 
    {
        X_T_ = X_T;
        if(X_T_X.size() > 0) X_T_X_ = X_T_X;
        N_ = X_T_.get_num_cols();
        N_2_ = N_/2.0;
        target_dirty_ = true;
    }

    /** @brief   Update X_T matrix */
    void update_design_matrix(const Matrix& X) 
    {
        X_T_ = X.transpose();
        if(X.get_num_rows() > 0) X_T_X_ = X_T_ * X;
        N_ = X_T_.get_num_cols();
        N_2_ = N_/2.0;
        target_dirty_ = true;
    }

    /**
     * @brief   Remove a input at target index
     */
    void remove_input(const Vector& input)
    {
        if(N_ == 0) return;
        // update the design matrix
        X_T_X_ -= outer_product(input, input);
        N_--;
        N_2_ = N_/2.0;

        // remove the data at the target index
        target_dirty_ = true;
    }

    /**
     * @brief   Remove a input-output pair at target_index
     */
    void remove_input_output(const Vector& input, double target)
    {
        if(N_ == 0) return;
        // remove input
        remove_input(input);

        y_T_y_ -= target * target; 
        X_T_y_ -= target * input;
        target_dirty_ = false;
    }

    /**
     * @brief   Add a input to the end 
     */
    void add_input(const Vector& data)
    {
        IFT(data.size() == D_, Illegal_argument, "added dimension mismatch");
        if(X_T_.size() == 0)
        {
            // Update X
            X_T_X_ = outer_product(data, data);
        }
        else
        {
            // Update X
            X_T_X_ += outer_product(data, data);
        }
        N_++;
        N_2_ = N_/2.0;
        target_dirty_ = true;

    }

    /**
     * @brief   Add a input-output pair to the end
     */
    void add_input_output(const Vector& data, double target)
    {
        add_input(data);
        y_T_y_ += target * target;
        X_T_y_ += target * data;
        target_dirty_ = false;
    }

    /**
     * @brief   Compute the params of the posterior based on the output y
     *
     */
    bool compute_posterior
    (
         const Vector& y, 
         double y_T_y = std::numeric_limits<double>::quiet_NaN() 
    )
    {
        IFT(y.size() == N_, Illegal_argument, "Blr y and X mismatch");
        if(N_ > 0)
        {
            X_T_y_ = X_T_ * y;
        }
        if(std::isnan(y_T_y))
        {
            y_T_y_ = dot(y, y);
        }
        else
        {
            y_T_y_ = y_T_y;
        }

        target_dirty_ = false;

        return update_posterior();
    }

    Matrix& X_T_X() { return X_T_X_; }
    Matrix& X_T() { return X_T_; }

    /**
     * @brief   Get the parameters of the posterior
     */
    const Blr_param& get_posterior() const { return posterior_param_; }

    /**
     * @brief   Get the parameters of the prior
     */
    const Blr_param& get_prior() const { return prior_param_; }

    /**
     * @brief   return the log of the prior of m and var
     */
    double log_prior(const Vector& m, double var) const
    {
        if(type_ == INVERSE_GAMMA)
        {
            return log_pdf(nig_prior_, m, var);
        }
        else
        {
            ASSERT(type_ == INVERSE_CHI_SQUARED);
            return log_pdf(nic_prior_, m, var);
        }
    }

    /**
     * @brief   return the log of the posterior of m and var
     */
    double log_posterior(const Vector& m, double var) const
    {
        IFT(!target_dirty_, Runtime_error, "Can not compute the posterior");
        if(type_ == INVERSE_GAMMA)
        {
            return log_pdf(nig_posterior_, m, var);
        }
        else
        {
            ASSERT(type_ == INVERSE_CHI_SQUARED);
            return log_pdf(nic_posterior_, m, var);
        }
    }

    /**
     * @brief   Return sample from the prior 
     */
    std::pair<Vector, double> get_sample_from_prior() const
    {
        if(type_ == INVERSE_GAMMA)
        {
            return sample(nig_prior_);
        }
        else
        {
            return sample(nic_prior_);
        }
    }

    /**
     * @brief   Return sample from the posterior
     */
    std::pair<Vector, double> get_sample_from_posterior() const
    {
        if(type_ == INVERSE_GAMMA)
        {
            return sample(nig_posterior_);
        }
        else
        {
            return sample(nic_posterior_);
        }
    }

    /**
     * @brief   Update the parameters of the posteriors based on the current
     *          cache
     */
    bool update_posterior()
    {
        // make sure the targets are up to date
        IFT(!target_dirty_, Runtime_error, "Target are dirty");
        posterior_param_.V = X_T_X_ + prior_param_.V;
        posterior_param_.mu = posterior_param_.V.inverse() * 
                                (prior_param_.V_mu + X_T_y_);
        double temp = y_T_y_ - dot(posterior_param_.mu * posterior_param_.V, 
                                   posterior_param_.mu);
        //std::cout << "temp: " << temp << std::endl;

        if(type_ == INVERSE_GAMMA)
        {
            posterior_param_.var_a = prior_param_.var_a + N_2_;
            posterior_param_.var_b = prior_param_.var_b + 
                                     (prior_param_.mu_V_mu + temp) / 2.0;
            //std::cout << "mu_V_mu: " << prior_param_.mu_V_mu << std::endl;
            //std::cout << "var_b: " << posterior_param_.var_b << std::endl;
            if(posterior_param_.var_b < 0.0) 
            {
                return false;
            }
            nig_posterior_ = get_nig(posterior_param_);
            return true;
        }
        else
        {
            ASSERT(type_ == INVERSE_CHI_SQUARED);
            posterior_param_.var_a = prior_param_.var_a + N_;
            posterior_param_.var_b = (prior_param_.var_a * prior_param_.var_b 
                             + prior_param_.mu_V_mu + temp)/posterior_param_.var_a;
            //std::cout << "var_b: " << posterior_param_.var_b << std::endl;
            if(posterior_param_.var_b < 0.0)
            {
                return false;
            }
            nic_posterior_ = get_nic(posterior_param_);
            return true;
        }
        return false;
    }

    size_t N() const {return N_; }
    const Matrix& X_T_const() const { return X_T_;}

private:
    /**
     * @brief   Return a Normal inverse gamma distribution from the param
     */
    Normal_inverse_gamma_distribution get_nig(const Blr_param& param)
    {
        return Normal_inverse_gamma_distribution (param.mu,
                                                  param.V.inverse(),
                                                  param.var_a,
                                                  param.var_b);
    }

    /**
     * @brief   Return a Normal inverse chi-squared distribution from the param
     */
    Normal_inverse_chi_squared_distribution get_nic(const Blr_param& param)
    {
        return Normal_inverse_chi_squared_distribution (param.mu,
                                                        param.V.inverse(),
                                                        param.var_a,
                                                        param.var_b);
    }

    // params of the bayesian (NIG or NIC) prior
    Blr_param prior_param_;
    // params of the bayesian (NIG or NIC) posterior
    Blr_param posterior_param_;

    Normal_inverse_gamma_distribution nig_prior_;
    Normal_inverse_gamma_distribution nig_posterior_;

    Normal_inverse_chi_squared_distribution nic_prior_;
    Normal_inverse_chi_squared_distribution nic_posterior_;

    // Type (inverse-gamma, inverse-chi-squared)
    Variance_type type_;

    // inputs (design matrix)
    Matrix X_T_; // D by N
    Matrix X_T_X_; // D by D

    // outputs 
    Vector y_; // N by 1
    size_t N_;
    double N_2_; // N / 2
    size_t D_;

    // data related cache
    Vector X_T_y_; // D by 1
    double y_T_y_;
    bool target_dirty_;

}; // class Bayesian_linear_regression

} //namepsace kjb
#endif // KJB_BAYESIAN_LINEAR_REGRESSION_H
