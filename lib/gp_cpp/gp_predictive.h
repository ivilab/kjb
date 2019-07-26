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
   |  Author:  Ernesto Brau
 * =========================================================================== */

/* $Id$ */

#ifndef GP_PREDICTIVE_H_INCLUDED
#define GP_PREDICTIVE_H_INCLUDED

#include <gp_cpp/gp_base.h>
#include <m_cpp/m_matrix.h>
#include <m_cpp/m_vector.h>
#include <prob_cpp/prob_distribution.h>
#include <l_cpp/l_exception.h>
#include <algorithm>
#include <iterator>

namespace kjb {
namespace gp {

/**
 * @brief   Represents the predictive distribution induced by a
 *          Gaussian process and noise-less training data.
 */
template<class Mean, class Covariance>
class Predictive_nl
{
public:
    /**
     * @brief   Construct the predictive distribution for a GP with the given
     *          mean and covariance functions, training inputs and outputs,
     *          at the given test inputs.
     */
    template<class TrinIt, class TroutIt, class TeinIt>
    Predictive_nl
    (
        const Mean& mf,
        const Covariance& cf,
        TrinIt first_trin,
        TrinIt last_trin,
        TroutIt first_trout,
        TroutIt last_trout,
        TeinIt first_tein,
        TeinIt last_tein
    ) :
        mean_func_(mf),
        cov_func_(cf),
        train_in_(first_trin, last_trin),
        train_out_(first_trout, last_trout),
        test_in_(first_tein, last_tein),
        dist_(1),
        trin_dirty_(true),
        trout_dirty_(true),
        tein_dirty_(true),
        mf_dirty_(true),
        cf_dirty_(true)
    {
        IFT(!train_in_.empty() && !test_in_.empty(), Illegal_argument,
            "Cannot create GP predictive; inputs must not be empty.");

        IFT(train_in_.size() == train_out_.size(), Dimension_mismatch,
            "Cannot create GP predictive; inputs and outputs have different "
            "dimensions.");
    }

    /** @brief  Get the mean function for the underlying GP. */
    const Mean& mean_function() const { return mean_func_; }

    /** @brief  Get the covariance function for the underlying GP. */
    const Covariance& covariance_function() const { return cov_func_; }

    /** @brief  Iterator to first training input. */
    Inputs::const_iterator trin_begin() const { return train_in_.begin(); }

    /** @brief  Iterator to last training input. */
    Inputs::const_iterator trin_end() const { return train_in_.end(); }

    /** @brief  Iterator to first training output. */
    Vector::const_iterator trout_begin() const { return train_out_.begin(); }

    /** @brief  Iterator to last training output. */
    Vector::const_iterator trout_end() const { return train_out_.end(); }

    /** @brief  Iterator to first testing input. */
    Inputs::const_iterator tein_begin() const { return test_in_.begin(); }

    /** @brief  Iterator to last testing input. */
    Inputs::const_iterator tein_end() const { return test_in_.end(); }

    /** @brief  Set the training inputs. */
    template<class Iterator>
    void set_train_inputs(Iterator first, Iterator last)
    {
        const size_t N = std::distance(first, last);

        IFT(N == train_out_.size(), Dimension_mismatch,
            "Cannot set GP predictive train inputs; wrong dimension.");

        train_in_.resize(N);
        std::copy(first, last, train_in_.begin());
        trin_dirty_ = true;
    }

    /** @brief  Set the training outputs. */
    template<class Iterator>
    void set_train_outputs(Iterator first, Iterator last)
    {
        const size_t N = std::distance(first, last);

        IFT(N == train_in_.size(), Dimension_mismatch,
            "Cannot set GP predictive train outputs; wrong dimension.");

        train_out_.resize(N);
        std::copy(first, last, train_out_.begin());
        trout_dirty_ = true;
    }

    /** @brief  Set the training inputs and outputs. */
    template<class InIt, class OutIt>
    void set_train_data(InIt first_in, InIt last_in, OutIt first_out)
    {
        const size_t N = std::distance(first_in, last_in);
        OutIt last_out = first_out;
        std::advance(last_out, N);

        train_in_.resize(N);
        train_out_.resize(N);
        std::copy(first_in, last_in, train_in_.begin());
        std::copy(first_out, last_out, train_out_.begin());

        trin_dirty_ = true;
        trout_dirty_ = true;
    }

    /** @brief  Set the testing inputs. */
    template<class Iterator>
    void set_test_inputs(Iterator first, Iterator last)
    {
        const size_t N = std::distance(first, last);

        test_in_.resize(N);
        std::copy(first, last, test_in_.begin());
        tein_dirty_ = true;
    }

    /** @brief  Set the mean function. */
    void set_mean_function(const Mean& mf)
    {
        mean_func_ = mf;
        mf_dirty_ = true;
    }

    /** @brief  Set the covariance function. */
    void set_covariance_function(const Covariance& cf)
    {
        cov_func_ = cf;
        cf_dirty_ = true;
    }

    /** @brief  Return the MV normal distribution induced by this predictive. */
    const MV_normal_distribution& normal() const
    {
        update_cache();
        return dist_;
    }

private:
    /** @brief  Update cache. */
    void update_cache() const;

    Mean mean_func_;
    Covariance cov_func_;
    Inputs train_in_;
    Vector train_out_;
    Inputs test_in_;
    mutable MV_gaussian_distribution dist_;
    mutable Matrix K_tr_tr_inv_;
    mutable Matrix K_te_tr_;
    mutable Matrix K_tr_te_;
    mutable Matrix K_te_te_;
    mutable Matrix A_;
    mutable Vector m_tr_;
    mutable Vector m_te_;
    mutable Vector mu_;
    mutable Matrix Sigma_;
    mutable bool trin_dirty_;
    mutable bool trout_dirty_;
    mutable bool tein_dirty_;
    mutable bool mf_dirty_;
    mutable bool cf_dirty_;
};

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

template<class Mean, class Covariance>
void Predictive_nl<Mean, Covariance>::update_cache() const
{
    if(trin_dirty_ || mf_dirty_)
    {
        m_tr_ = apply_mf(mean_func_, train_in_.begin(), train_in_.end());
    }

    if(trin_dirty_ || cf_dirty_)
    {
        Matrix K = apply_cf(cov_func_, train_in_.begin(), train_in_.end());
        K_tr_tr_inv_ = matrix_inverse(K);
    }

    if(tein_dirty_ || mf_dirty_)
    {
        m_te_ = apply_mf(mean_func_, test_in_.begin(), test_in_.end());
    }

    if(tein_dirty_ || cf_dirty_)
    {
        K_te_te_ = apply_cf(cov_func_, test_in_.begin(), test_in_.end());
    }

    if(tein_dirty_ || trin_dirty_ || cf_dirty_)
    {
        K_te_tr_ = apply_cf(
                        cov_func_,
                        test_in_.begin(),
                        test_in_.end(),
                        train_in_.begin(),
                        train_in_.end());

        K_tr_te_ = matrix_transpose(K_te_tr_);

        A_ = K_te_tr_ * K_tr_tr_inv_;
        Matrix B = A_ * K_tr_te_;
        Sigma_ = K_te_te_ - B;
    }

    if(trin_dirty_ || tein_dirty_ || trout_dirty_ || mf_dirty_ || cf_dirty_)
    {
        mu_ = m_te_ + A_*(train_out_ - m_tr_);

        if(trin_dirty_ || tein_dirty_ || cf_dirty_)
        {
            dist_.set_covariance_matrix(Sigma_, mu_);
        }
        else
        {
            dist_.set_mean(mu_);
        }
    }

    trin_dirty_ = false;
    tein_dirty_ = false;
    trout_dirty_ = false;
    mf_dirty_ = false;
    cf_dirty_ = false;
}

/** @brief  Convenience function to create a NL predictive. */
template<class Mean, class Covariance>
inline
Predictive_nl<Mean, Covariance> make_predictive_nl
(
    const Mean& mean,
    const Covariance& covariance,
    const Inputs& trins,
    const Vector& trouts,
    const Inputs& teins
)
{
    typedef Predictive_nl<Mean, Covariance> Pred;
    return Pred(
            mean, covariance,
            trins.begin(), trins.end(),
            trouts.begin(), trouts.end(),
            teins.begin(), teins.end());
}

/**
 * @brief   Represents the predictive distribution induced by a
 *          Gaussian process and training data.
 */
template<class Mean, class Covariance, class Likelihood>
class Predictive
{
public:
    /**
     * @brief   Construct the predictive distribution for a GP with the given
     *          mean and covariance functions, training inputs and outputs,
     *          at the given test inputs.
     */
    template<class TrinIt, class TroutIt, class TeinIt>
    Predictive
    (
        const Mean& mf,
        const Covariance& cf,
        const Likelihood& likelihood,
        TrinIt first_trin,
        TrinIt last_trin,
        TroutIt first_trout,
        TroutIt last_trout,
        TeinIt first_tein,
        TeinIt last_tein
    ) :
        mean_func_(mf),
        cov_func_(cf),
        likelihood_(likelihood),
        train_in_(first_trin, last_trin),
        train_out_(first_trout, last_trout),
        test_in_(first_tein, last_tein)
    {
        IFT(!train_in_.empty() && !test_in_.empty(), Illegal_argument,
            "Cannot create GP predictive; inputs must not be empty.");

        IFT(train_in_.size() == train_out_.size(), Dimension_mismatch,
            "Cannot create GP predictive; inputs and outputs have different "
            "dimensions.");

        likelihood_.set_outputs(train_out_.begin(), train_out_.end());
    }

    /** @brief  Get the mean function for the underlying GP. */
    const Mean& mean_function() const { return mean_func_; }

    /** @brief  Get the covariance function for the underlying GP. */
    const Covariance& covariance_function() const { return cov_func_; }

    /** @brief  Get the likelihood function for the underlying GP. */
    const Likelihood& likelihood() const { return likelihood_; }

    /** @brief  Iterator to first training input. */
    Inputs::const_iterator trin_begin() const { return train_in_.begin(); }

    /** @brief  Iterator to last training input. */
    Inputs::const_iterator trin_end() const { return train_in_.end(); }

    /** @brief  Iterator to first training output. */
    Vector::const_iterator trout_begin() const { return train_out_.begin(); }

    /** @brief  Iterator to last training output. */
    Vector::const_iterator trout_end() const { return train_out_.end(); }

    /** @brief  Iterator to first testing input. */
    Inputs::const_iterator tein_begin() const { return test_in_.begin(); }

    /** @brief  Iterator to last testing input. */
    Inputs::const_iterator tein_end() const { return test_in_.end(); }

    /** @brief  Set the training inputs. */
    template<class Iterator>
    void set_train_inputs(Iterator first, Iterator last)
    {
        const size_t N = std::distance(first, last);

        IFT(N == train_in_.size(), Dimension_mismatch,
            "Cannot set GP predictive train inputs; wrong dimension.");

        train_in_.resize(N);
        std::copy(first, last, train_in_.begin());
    }

    /** @brief  Set the training outputs. */
    template<class Iterator>
    void set_train_outputs(Iterator first, Iterator last)
    {
        const size_t N = std::distance(first, last);

        IFT(N == train_out_.size(), Dimension_mismatch,
            "Cannot set GP predictive train outputs; wrong dimension.");

        train_out_.assign(first, last);
        likelihood_.set_outputs(first, last);
    }

    /** @brief  Set the training inputs and outputs. */
    template<class InIt, class OutIt>
    void set_train_data(InIt first_in, InIt last_in, OutIt first_out)
    {
        const size_t N = std::distance(first_in, last_in);
        OutIt last_out = first_out;
        std::advance(last_out, N);

        train_in_.assign(first_in, last_in);
        train_out_.assign(first_out, last_out);
        likelihood_.set_outputs(first_out, last_out);
    }

    /** @brief  Set the testing inputs. */
    template<class Iterator>
    void set_test_inputs(Iterator first, Iterator last)
    {
        const size_t N = std::distance(first, last);

        test_in_.resize(N);
        std::copy(first, last, test_in_.begin());
    }

    /** @brief  Set the mean function. */
    void set_mean_function(const Mean& mf) { mean_func_ = mf; }

    /** @brief  Set the covariance function. */
    void set_covariance_function(const Covariance& cf) { cov_func_ = cf; }

    /** @brief  Set the likelihood. */
    void set_likelihood(const Likelihood& lhood)
    {
        likelihood_ = lhood;
        likelihood_.set_outputs(train_out_.begin(), train_out_.end());
    }

private:
    Mean mean_func_;
    Covariance cov_func_;
    Likelihood likelihood_;
    Inputs train_in_;
    Vector train_out_;
    Inputs test_in_;
};

/** @brief  Convenience function to create a predictive. */
template<class Mean, class Covariance, class Likelihood>
inline
Predictive<Mean, Covariance, Likelihood> make_predictive
(
    const Mean& mean,
    const Covariance& covariance,
    const Likelihood& likelihood,
    const Inputs& trins,
    const Vector& trouts,
    const Inputs& teins
)
{
    typedef Predictive<Mean, Covariance, Likelihood> Pred;
    return Pred(
            mean, covariance, likelihood,
            trins.begin(), trins.end(),
            trouts.begin(), trouts.end(),
            teins.begin(), teins.end());
}

}} //namespace kjb::gp

#endif /*GP_PREDICTIVE_H_INCLUDED */

