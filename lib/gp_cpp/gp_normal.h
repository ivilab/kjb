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

#ifndef GP_NORMAL_H_INCLUDED
#define GP_NORMAL_H_INCLUDED

#include <m_cpp/m_vector.h>
#include <m_cpp/m_matrix.h>
#include <l_cpp/l_exception.h>
#include <gp_cpp/gp_base.h>
#include <gp_cpp/gp_likelihood.h>
#include <gp_cpp/gp_predictive.h>
#include <gp_cpp/gp_posterior.h>
#include <prob_cpp/prob_distribution.h>
#include <iterator>
#include <algorithm>

namespace kjb {
namespace gp {

/**
 * @brief   Represents the predictive distribution induced by a
 *          Gaussian process and training data with linear-Gaussian
 *          noise.
 */
template<class Mean, class Covariance>
class Predictive<Mean, Covariance, Linear_gaussian>
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
        const Linear_gaussian& likelihood,
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
        test_in_(first_tein, last_tein),
        dist_(1),
        trin_dirty_(true),
        trout_dirty_(true),
        tein_dirty_(true),
        mf_dirty_(true),
        cf_dirty_(true),
        lh_dirty_(true)
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
    const Linear_gaussian& likelihood() const { return likelihood_; }

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

        train_in_.assign(first, last);
        trin_dirty_ = true;
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

        trout_dirty_ = true;
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

        trin_dirty_ = true;
        trout_dirty_ = true;
    }

    /** @brief  Set the testing inputs. */
    template<class Iterator>
    void set_test_inputs(Iterator first, Iterator last)
    {
        test_in_.assign(first, last);
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

    /** @brief  Set the likelihood. */
    void set_likelihood(const Linear_gaussian& likelihood)
    {
        likelihood_ = likelihood;
        likelihood_.set_outputs(train_out_.begin(), train_out_.end());
        lh_dirty_ = true;
    }

    /** @brief  Return the MV normal distribution induced by this predictive. */
    const MV_normal_distribution& normal() const
    {
        update();
        return dist_;
    }

private:
    /** @brief  Update cache. */
    void update() const;

    Mean mean_func_;
    Covariance cov_func_;
    Linear_gaussian likelihood_;
    Inputs train_in_;
    Vector train_out_;
    Inputs test_in_;
    mutable MV_gaussian_distribution dist_;
    mutable Vector m_;
    mutable Vector m_s_;
    mutable Matrix K_;
    mutable Matrix K_s_;
    mutable Matrix K_ss_;
    mutable Matrix A_conj_;
    mutable Matrix AKA_;
    mutable Matrix AKAS_;
    mutable Matrix KAAKAS_;
    mutable Vector Am_;
    mutable Vector Amu_;
    mutable Vector yAmu_;
    mutable Vector mu_;
    mutable Matrix Sigma_;
    mutable bool trin_dirty_;
    mutable bool trout_dirty_;
    mutable bool tein_dirty_;
    mutable bool mf_dirty_;
    mutable bool cf_dirty_;
    mutable bool lh_dirty_;
};

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

template<class Mean, class Covariance>
void Predictive<Mean, Covariance, Linear_gaussian>::update() const
{
    bool m_dirty = false;
    bool m_s_dirty = false;
    bool K_dirty = false;
    bool K_s_dirty = false;
    bool K_ss_dirty = false;
    bool S_dirty = lh_dirty_;
    bool A_dirty = lh_dirty_;
    bool u_dirty = lh_dirty_;
    bool A_conj_dirty = false;
    bool AKA_dirty = false;
    bool AKAS_dirty = false;
    bool KAAKAS_dirty = false;
    bool Am_dirty = false;
    bool Amu_dirty = false;
    bool yAmu_dirty = false;
    bool mu_dirty = false;
    bool Sigma_dirty = false;

    const Matrix& S = likelihood_.covariance();
    const Matrix& A = likelihood_.scale();
    const Vector& u = likelihood_.offset();

    if(trin_dirty_ || mf_dirty_)
    {
        m_ = apply_mf(mean_func_, train_in_.begin(), train_in_.end());
        m_dirty = true;
    }

    if(trin_dirty_ || cf_dirty_)
    {
        K_ = apply_cf(cov_func_, train_in_.begin(), train_in_.end());
        K_dirty = true;
    }

    if(tein_dirty_ || mf_dirty_)
    {
        m_s_ = apply_mf(mean_func_, test_in_.begin(), test_in_.end());
        m_s_dirty = true;
    }

    if(tein_dirty_ || cf_dirty_)
    {
        K_ss_ = apply_cf(cov_func_, test_in_.begin(), test_in_.end());
        K_ss_dirty = true;
    }

    if(trin_dirty_ || tein_dirty_ || cf_dirty_)
    {
        K_s_ = apply_cf(
                    cov_func_,
                    test_in_.begin(),
                    test_in_.end(),
                    train_in_.begin(),
                    train_in_.end());
        K_s_dirty = true;
    }

    if(A_dirty)
    {
        A_conj_ = matrix_transpose(A);
        A_conj_dirty = true;
    }

    if(A_dirty || K_dirty || A_conj_dirty)
    {
        AKA_ = A * K_ * A_conj_;
        AKA_dirty = true;
    }

    if(AKA_dirty || S_dirty)
    {
        AKAS_ = matrix_inverse(AKA_ + S);
        AKAS_dirty = true;
    }

    if(K_s_dirty || A_conj_dirty || AKAS_dirty || K_ss_dirty)
    {
        Matrix KA = K_s_ * A_conj_;
        Matrix AK = matrix_transpose(KA);
        KAAKAS_ = KA * AKAS_;
        Matrix KAAKASAK = KAAKAS_ * AK;
        Sigma_ = K_ss_ - KAAKASAK;

        KAAKAS_dirty = true;
        Sigma_dirty = true;
    }

    if(A_dirty || m_dirty)
    {
        Am_ = A * m_;
        Am_dirty = true;
    }

    if(Am_dirty || u_dirty)
    {
        Amu_ = Am_ - u;
        Amu_dirty = true;
    }

    if(trout_dirty_ || Amu_dirty)
    {
        yAmu_ = train_out_ - Amu_;
        yAmu_dirty = true;
    }

    if(KAAKAS_dirty || yAmu_dirty || m_s_dirty)
    {
        Vector KAAKASyAmu = KAAKAS_ * yAmu_;
        mu_ = m_s_ + KAAKASyAmu;
        mu_dirty = true;
    }

    if(mu_dirty || Sigma_dirty)
    {
        if(!Sigma_dirty)
        {
            dist_.set_mean(mu_);
        }
        else if(!mu_dirty)
        {
            dist_.set_covariance_matrix(Sigma_);
        }
        else
        {
            assert(mu_dirty && Sigma_dirty);
            dist_.set_covariance_matrix(Sigma_, mu_);
        }
    }

    trin_dirty_ = false;
    tein_dirty_ = false;
    trout_dirty_ = false;
    mf_dirty_ = false;
    cf_dirty_ = false;
    lh_dirty_ = false;
}

/** @brief  Convenience function to create a NL predictive. */
template<class Mean, class Covariance>
inline
Predictive<Mean, Covariance, Linear_gaussian> make_predictive
(
    const Mean& mean,
    const Covariance& covariance,
    double noise_stddev,
    const Inputs& trins,
    const Vector& trouts,
    const Inputs& teins
)
{
    typedef Predictive<Mean, Covariance, Linear_gaussian> Pred;
    return Pred(
            mean, covariance,
            Linear_gaussian(noise_stddev, trins.size()),
            trins.begin(), trins.end(),
            trouts.begin(), trouts.end(),
            teins.begin(), teins.end());
}

/** @brief  Convenience function to create a NL predictive. */
template<class Mean, class Covariance>
inline
Predictive<Mean, Covariance, Linear_gaussian> make_predictive
(
    const Mean& mean,
    const Covariance& covariance,
    const Matrix& noise_cov,
    const Inputs& trins,
    const Vector& trouts,
    const Inputs& teins
)
{
    typedef Predictive<Mean, Covariance, Linear_gaussian> Pred;
    return Pred(
            mean, covariance,
            Linear_gaussian(noise_cov),
            trins.begin(), trins.end(),
            trouts.begin(), trouts.end(),
            teins.begin(), teins.end());
}

/**
 * @brief   Represents the posterior distribution induced by a
 *          Gaussian process and training data with linear-Gaussian
 *          noise.
 */
template<class Mean, class Covariance>
class Posterior<Mean, Covariance, Linear_gaussian>
{
public:
    /**
     * @brief   Construct the predictive distribution for a GP with the given
     *          mean and covariance functions, training inputs and outputs,
     *          at the given test inputs.
     */
    template<class InIt, class OutIt>
    Posterior
    (
        const Mean& mf,
        const Covariance& cf,
        const Linear_gaussian& likelihood,
        InIt first_in,
        InIt last_in,
        OutIt first_out,
        OutIt last_out
    ) :
        pred_(
            mf, cf,
            likelihood,
            first_in,
            last_in,
            first_out,
            last_out,
            first_in,
            last_in)
    {}

    /** @brief  Get the mean function for the underlying GP. */
    const Mean& mean_function() const { return pred_.mean_function(); }

    /** @brief  Get the covariance function for the underlying GP. */
    const Covariance& covariance_function() const
    {
        return pred_.covariance_function();
    }

    /** @brief  Get the likelihood function for the underlying GP. */
    const Linear_gaussian& likelihood() const { return pred_.likelihood(); }

    /** @brief  Iterator to first training input. */
    Inputs::const_iterator in_begin() const { return pred_.trin_begin(); }

    /** @brief  Iterator to last training input. */
    Inputs::const_iterator in_end() const { return pred_.trin_end(); }

    /** @brief  Iterator to first training output. */
    Vector::const_iterator out_begin() const { return pred_.trout_begin(); }

    /** @brief  Iterator to last training output. */
    Vector::const_iterator out_end() const { return pred_.trout_end(); }

    /** @brief  Set the training inputs. */
    template<class Iterator>
    void set_inputs(Iterator first, Iterator last)
    {
        pred_.set_train_inputs(first, last);
        pred_.set_test_inputs(first, last);
    }

    /** @brief  Set the training outputs. */
    template<class Iterator>
    void set_outputs(Iterator first, Iterator last)
    {
        pred_.set_train_outputs(first, last);
    }

    /** @brief  Set the training inputs and outputs. */
    template<class InIt, class OutIt>
    void set_data(InIt first_in, InIt last_in, OutIt first_out)
    {
        pred_.set_train_data(first_in, last_in, first_out);
        pred_.set_test_inputs(first_in, last_in);
    }

    /** @brief  Set the mean function. */
    void set_mean_function(const Mean& mf) { pred_.set_mean_function(mf); }

    /** @brief  Set the covariance function. */
    void set_covariance_function(const Covariance& cf)
    {
        pred_.set_covariance_function(cf);
    }

    /** @brief  Set the likelihood. */
    void set_likelihood(const Linear_gaussian& likelihood)
    {
        pred_.set_likelihood(likelihood);
    }

    /** @brief  Return the MV normal distribution induced by this predictive. */
    const MV_normal_distribution& normal() const { return pred_.normal(); }

private:
    Predictive<Mean, Covariance, Linear_gaussian> pred_;
};

/** @brief  Convenience function to create a NL predictive. */
template<class Mean, class Covariance>
inline
Posterior<Mean, Covariance, Linear_gaussian> make_posterior
(
    const Mean& mean,
    const Covariance& covariance,
    double noise_stddev,
    const Inputs& inputs,
    const Vector& outputs
)
{
    typedef Posterior<Mean, Covariance, Linear_gaussian> Post;
    return Post(
            mean, covariance,
            Linear_gaussian(noise_stddev, inputs.size()),
            inputs.begin(), inputs.end(),
            outputs.begin(), outputs.end());
}

/** @brief  Convenience function to create a NL predictive. */
template<class Mean, class Covariance>
inline
Posterior<Mean, Covariance, Linear_gaussian> make_posterior
(
    const Mean& mean,
    const Covariance& covariance,
    const Matrix& noise_cov,
    const Inputs& inputs,
    const Vector& outputs
)
{
    typedef Posterior<Mean, Covariance, Linear_gaussian> Post;
    return Post(
            mean, covariance,
            Linear_gaussian(noise_cov),
            inputs.begin(), inputs.end(),
            outputs.begin(), outputs.end());
}

/**
 * @brief   Represents the marginal likelihood induced by a
 *          Gaussian process. For now, this is simply an empty class,
 *          and exists only to be specialized with a LG likelihood.
 *          If and when we need the generic class, it should go in its
 *          own file, e.g., gp_marginal_likelihood.h.
 */
template<class Mean, class Covariance, class Likelihood>
class Marginal_likelihood {};

/**
 * @brief   Represents the marginal likelihood induced by a
 *          Gaussian process with linear-Gaussian noise.
 */
template<class Mean, class Covariance>
class Marginal_likelihood<Mean, Covariance, Linear_gaussian>
{
public:
    /**
     * @brief   Construct the margina likelihood for a GP with the given
     *          mean and covariance functions, training inputs and outputs,
     *          at the given test inputs.
     */
    template<class InIt>
    Marginal_likelihood
    (
        const Mean& mf,
        const Covariance& cf,
        const Linear_gaussian& likelihood,
        InIt first_in,
        InIt last_in
    ) :
        prior_(mf, cf, first_in, last_in),
        likelihood_(likelihood),
        dist_(1),
        prior_dirty_(true),
        lh_dirty_(true)
    {}

    /** @brief  Get the mean function for the underlying GP. */
    const Mean& mean_function() const { return prior_.mean_function(); }

    /** @brief  Get the covariance function for the underlying GP. */
    const Covariance& covariance_function() const
    {
        return prior_.covariance_function();
    }

    /** @brief  Get the likelihood function for the underlying GP. */
    const Linear_gaussian& likelihood() const { return likelihood_; }

    /** @brief  Iterator to first training input. */
    Inputs::const_iterator in_begin() const { return prior_.in_begin(); }

    /** @brief  Iterator to last training input. */
    Inputs::const_iterator in_end() const { return prior_.in_end(); }

    /** @brief  Set the training inputs. */
    template<class Iterator>
    void set_inputs(Iterator first, Iterator last)
    {
        prior_.set_inputs(first, last);
        prior_dirty_ = true;
    }

    /** @brief  Set the mean function. */
    void set_mean_function(const Mean& mf)
    {
        prior_.set_mean_function(mf);
        prior_dirty_ = true;
    }

    /** @brief  Set the covariance function. */
    void set_covariance_function(const Covariance& cf)
    {
        prior_.set_covariance_function(cf);
        prior_dirty_ = true;
    }

    /** @brief  Set the likelihood. */
    void set_likelihood(const Linear_gaussian& likelihood)
    {
        likelihood_ = likelihood;
        lh_dirty_ = true;
    }

    /** @brief  Return the MV normal distribution induced by this ML. */
    const MV_normal_distribution& normal() const { update(); return dist_; }

private:
    /** @brief  Update internal representation. */
    void update() const;

    Prior<Mean, Covariance> prior_;
    Linear_gaussian likelihood_;
    mutable MV_gaussian_distribution dist_;
    mutable bool prior_dirty_;
    mutable bool lh_dirty_;
};

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

template<class Mean, class Covariance>
inline
void Marginal_likelihood<Mean, Covariance, Linear_gaussian>::update() const
{
    if(!lh_dirty_ && !prior_dirty_) return;

    const Vector& mu = prior_.normal().get_mean();
    const Matrix& K = prior_.normal().get_covariance_matrix();

    const Matrix& S = likelihood_.covariance();
    const Matrix& A = likelihood_.scale();
    const Vector& u = likelihood_.offset();

    Vector m = u + A*mu;
    Matrix C = S + A*K*matrix_transpose(A);

    dist_.set_covariance_matrix(C, m);

    lh_dirty_ = false;
}

/** @brief  Convenience function to create a NL predictive. */
template<class Mean, class Covariance>
inline
Marginal_likelihood<Mean, Covariance, Linear_gaussian> make_marginal_likelihood
(
    const Mean& mean,
    const Covariance& covariance,
    double noise_stddev,
    const Inputs& inputs
)
{
    typedef Marginal_likelihood<Mean, Covariance, Linear_gaussian> Ml;
    return Ml(
            mean, covariance,
            Linear_gaussian(noise_stddev, inputs.size()),
            inputs.begin(), inputs.end());
}

/** @brief  Convenience function to create a NL predictive. */
template<class Mean, class Covariance>
inline
Marginal_likelihood<Mean, Covariance, Linear_gaussian> make_marginal_likelihood
(
    const Mean& mean,
    const Covariance& covariance,
    const Matrix& noise_cov,
    const Inputs& inputs
)
{
    typedef Marginal_likelihood<Mean, Covariance, Linear_gaussian> Ml;
    return Ml(
            mean, covariance,
            Linear_gaussian(noise_cov),
            inputs.begin(), inputs.end());
}

}} //namespace kjb::gp

#endif /*GP_NORMAL_H_INCLUDED */

