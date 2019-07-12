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

#ifndef GP_POSTERIOR_H_INCLUDED
#define GP_POSTERIOR_H_INCLUDED

#include <m_cpp/m_vector.h>
#include <l_cpp/l_exception.h>
#include <l_cpp/l_algorithm.h>
#include <gp_cpp/gp_base.h>
#include <gp_cpp/gp_prior.h>
#include <gp_cpp/gp_predictive.h>
#include <algorithm>
#include <iterator>
#include <vector>
#include <sstream>
#include <boost/bind.hpp>

#ifdef KJB_HAVE_ERGO
#include <ergo/mh.h>
#endif /* KJB_HAVE_ERGO */

namespace kjb {
namespace gp {

// forward declaration for ownership
template<class Mean, class Covariance, class Likelihood>
class Cp_sampler;

// forward declarations for friends
template<class Mean, class Covariance, class Likelihood>
class Posterior;

template<class M, class C, class L>
double pdf(const Posterior<M, C, L>&, const Vector&);

template<class M, class C, class L>
double log_pdf(const Posterior<M, C, L>&, const Vector&);

template<class M, class C, class L>
Vector sample(const Posterior<M, C, L>&);

template<class M, class C>
Vector sample(const Predictive_nl<M, C>&);

/**
 * @brief   Represents the posterior distribution induced by a
 *          Gaussian process and data.
 */
template<class Mean, class Covariance, class Likelihood>
class Posterior
{
private:
    typedef Posterior<Mean, Covariance, Likelihood> Self;

public:
    /**
     * @brief   Construct the posterior distribution for a GP with the given
     *          mean and covariance functions, inputs and outputs,
     *          and likelihood function.
     */
    template<class InIt, class OutIt>
    Posterior
    (
        const Mean& mf,
        const Covariance& cf,
        const Likelihood& likelihood,
        InIt first_in,
        InIt last_in,
        OutIt first_out,
        OutIt last_out
    );

    /** @brief  Get the mean function for the underlying GP. */
    const Mean& mean_function() const { return prior_.mean_function(); }

    /** @brief  Get the covariance function for the underlying GP. */
    const Covariance& covariance_function() const
    {
        return prior_.covariance_function();
    }

    /** @brief  Get the likelihood function used by this posterior. */
    const Likelihood& likelihood() const { return likelihood_; }

    /** @brief  Iterator to first input. */
    Inputs::const_iterator in_begin() const { return prior_.in_begin(); }

    /** @brief  Iterator to last input. */
    Inputs::const_iterator in_end() const { return prior_.in_end(); }

    /** @brief  Iterator to first output. */
    Vector::const_iterator out_begin() const { return outputs_.begin(); }

    /** @brief  Iterator to last output. */
    Vector::const_iterator out_end() const { return outputs_.end(); }

    /** @brief  Set the inputs. */
    template<class Iterator>
    void set_inputs(Iterator first, Iterator last)
    {
        const size_t N = std::distance(first, last);

        IFT(N == outputs_.size(), Dimension_mismatch,
            "Cannot set GP posterior inputs; wrong dimension.");

        prior_.set_inputs(first, last);

        sampler_dirty_ = true;
    }

    /** @brief  Set the outputs. */
    template<class Iterator>
    void set_outputs(Iterator first, Iterator last)
    {
        const size_t N = std::distance(first, last);

        IFT(N == outputs_.size(), Dimension_mismatch,
            "Cannot set GP posterior outputs; wrong dimension.");

        outputs_.assign(first, last);
        likelihood_.set_outputs(first, last);

        sampler_dirty_ = true;
    }

    /** @brief  Set the inputs and outputs. */
    template<class InIt, class OutIt>
    void set_data(InIt first_in, InIt last_in, OutIt first_out)
    {
        const size_t N = std::distance(first_in, last_in);
        OutIt last_out = first_out;
        std::advance(last_out, N);

        // inputs
        prior_.set_inputs(first_in, last_in);

        // outputs
        outputs_.assign(first_out, last_out);
        likelihood_.set_outputs(first_out, last_out);

        sampler_dirty_ = true;
    }

    /** @brief  Set the mean function. */
    void set_mean_function(const Mean& mf)
    {
        prior_.set_mean_function(mf);

        sampler_dirty_ = true;
    }

    /** @brief  Set the covariance function. */
    void set_covariance_function(const Covariance& cf)
    {
        prior_.set_covariance_function(cf);

        sampler_dirty_ = true;
    }

    /** @brief  Set the likelihood function. */
    void set_likelihood(const Likelihood& likelihood)
    {
        likelihood_ = likelihood;
        likelihood_.set_outputs(outputs_.begin(), outputs_.end());

        sampler_dirty_ = true;
    }

private:
    /** @brief  Prior for this posterior. */
    const Prior<Mean, Covariance>& prior() const
    {
        return prior_;
    }

    /** @brief  Prior for this posterior. */
    const Cp_sampler<Mean, Covariance, Likelihood>& sampler() const
    {
        if(sampler_dirty_)
        {
            sampler_ = Cp_sampler<Mean, Covariance, Likelihood>(
                        *this,
                        apply_mf(
                            prior_.mean_function(),
                            prior_.in_begin(), prior_.in_end()));
        }

        return sampler_;
    }

private:
    Likelihood likelihood_;
    Vector outputs_;
    mutable Prior<Mean, Covariance> prior_;
    mutable Cp_sampler<Mean, Covariance, Likelihood> sampler_;
    mutable bool sampler_dirty_;

private:
    friend
    double pdf<Mean, Covariance, Likelihood>(const Self&, const Vector&);

    friend
    double log_pdf<Mean, Covariance, Likelihood>(const Self&, const Vector&);

    friend
    Vector sample<Mean, Covariance, Likelihood>(const Self&);
};

/** @brief  Sampler for the posterior distribution of data modeled by a GP. */
template<class Mean, class Covariance>
class Cp_proposer
{
#ifdef KJB_HAVE_ERGO
public:
    typedef ergo::mh_proposal_result result_type;
#endif

private:
    typedef Predictive_nl<Mean, Covariance> Pred;
    typedef std::vector<size_t> Index_vec;

public:
    /** @brief  Create proposer, given controls. */
    template<class InputIt, class IndexIt>
    Cp_proposer
    (
        InputIt fin,
        InputIt lin,
        IndexIt fid,
        IndexIt lid,
        const Mean& mf,
        const Covariance& cf
    ) :
        inputs_(fin, lin),
        c_inds_(fid, lid),
        pred_(make_predictive(inputs_, c_inds_, mf, cf)),
        c_preds_(make_control_predictives(inputs_, c_inds_, mf, cf)),
        cur_c_(0)
    {}

    /** @brief  Create proposer, choose controls. */
    template<class InputIt>
    Cp_proposer
    (
        InputIt fin,
        InputIt lin,
        size_t num_cp,
        const Mean& mf,
        const Covariance& cf
    ) :
        inputs_(fin, lin),
        c_inds_(uniform_controls(inputs_.size(), num_cp)),
        pred_(make_predictive(inputs_, c_inds_, mf, cf)),
        c_preds_(make_control_predictives(inputs_, c_inds_, mf, cf)),
        cur_c_(0)
    {}

public:
    /** @brief  Set mean function. */
    void set_mean_function(const Mean& mf);

    /** @brief  Set covariance function. */
    void set_covariance_function(const Covariance& cf);

    /** @brief  Get current control piont index. */
    size_t current_control() const { return cur_c_; }

#ifdef KJB_HAVE_ERGO
public:
    /** @brief  Propose a new model. */
    ergo::mh_proposal_result operator()(const Vector& m, Vector& m_p) const;
#endif

private:
    /** @brief  Utility function to generate uniformly spaced controls. */
    static Index_vec uniform_controls(size_t np, size_t ncp)
    {
        IFT(ncp < np, Illegal_argument, "Too many controls.");
        IFT(ncp > 0, Illegal_argument, "Need at least 1 control.");

        Index_vec ids(ncp);
        linspace(0, np - 1, ncp, ids.begin());

        return ids;
    }

    /** @brief  Create the predictive distribution for proposing. */
    static Pred make_predictive
    (
        const Inputs& inputs,
        const Index_vec& c_indices,
        const Mean& mf,
        const Covariance& cf
    );

    /** @brief  Create the predictive distributions for the control points. */
    static std::vector<Pred> make_control_predictives
    (
        const Inputs& inputs,
        const Index_vec& c_indices,
        const Mean& mf,
        const Covariance& cf
    );

private:
    Inputs inputs_;
    Index_vec c_inds_;
    mutable Pred pred_;
    mutable std::vector<Pred> c_preds_;
    mutable size_t cur_c_;
};

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

template<class M, class C>
typename Cp_proposer<M, C>::Pred Cp_proposer<M, C>::make_predictive
(
    const Inputs& inputs,
    const Index_vec& c_indices,
    const M& mf,
    const C& cf
)
{
    // train (control) inputs
    const size_t num_ctrls = c_indices.size();
    Inputs c_inputs(num_ctrls);
    for(size_t i = 0; i < num_ctrls; ++i)
    {
        c_inputs[i] = inputs[c_indices[i]];
    }

    // train (control) outputs
    Vector c_outputs(static_cast<const int>(num_ctrls), 0.0);

    // predictive distribution
    return make_predictive_nl(mf, cf, c_inputs, c_outputs, inputs);
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

template<class M, class C>
std::vector<typename Cp_proposer<M, C>::Pred>
    Cp_proposer<M, C>::make_control_predictives
(
    const Inputs& inputs,
    const Index_vec& c_indices,
    const M& mf,
    const C& cf
)
{
    const size_t num_ctrls = c_indices.size();

    std::vector<Pred> preds;
    preds.reserve(num_ctrls);

    Inputs trins(num_ctrls - 1);
    Vector trouts(static_cast<const int>(num_ctrls) - 1, 0.0);
    Inputs teins(1);
    for(size_t i = 0; i < num_ctrls; i++)
    {
        // train inputs are control inputs minus current one
        for(size_t j = 0; j < i; ++j)
        {
            trins[j] = inputs[c_indices[j]];
        }

        for(size_t j = i + 1; j < num_ctrls; ++j)
        {
            trins[j - 1] = inputs[c_indices[j]];
        }

        // test input is current input
        teins[0] = inputs[c_indices[i]];

        // create predictive
        preds.push_back(make_predictive_nl(mf, cf, trins, trouts, teins));
    }

    return preds;
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

template<class M, class C>
void Cp_proposer<M, C>::set_mean_function(const M& mf)
{
    pred_.set_mean_function(mf);

    const size_t n = c_preds_.size();
    for(size_t i = 0; i < n; ++i)
    {
        c_preds_[i].set_mean_function(mf);
    }
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

template<class M, class C>
void Cp_proposer<M, C>::set_covariance_function(const C& cf)
{
    pred_.set_covariance_function(cf);

    const size_t n = c_preds_.size();
    for(size_t i = 0; i < n; ++i)
    {
        c_preds_[i].set_covariance_function(cf);
    }
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

#ifdef KJB_HAVE_ERGO
template<class M, class C>
ergo::mh_proposal_result Cp_proposer<M, C>::operator()
(
    const Vector& m,
    Vector& m_p
) const
{
    m_p = m;

    const size_t num_ctrls = c_inds_.size();

    // sample current control point
    Vector trouts(num_ctrls - 1);
    for(size_t j = 0; j < cur_c_; ++j)
    {
        trouts[j] = m_p[c_inds_[j]];
    }

    for(size_t j = cur_c_ + 1; j < num_ctrls; ++j)
    {
        trouts[j - 1] = m_p[c_inds_[j]];
    }

    c_preds_[cur_c_].set_train_outputs(trouts.begin(), trouts.end());
    m_p[c_inds_[cur_c_]] = gp::sample(c_preds_[cur_c_])[0];

    // sample real outputs
    trouts.resize(num_ctrls);
    for(size_t j = 0; j < num_ctrls; ++j)
    {
        trouts[j] = m_p[c_inds_[j]];
    }

    pred_.set_train_outputs(trouts.begin(), trouts.end());
    m_p = gp::sample(pred_);

    std::stringstream sst;
    sst << "control-" << cur_c_++;

    // reset the cur_c_ to 0 after a cycle
    if(cur_c_ == num_ctrls) cur_c_ = 0;

    return ergo::mh_proposal_result(0.0, 0.0, sst.str());
}
#endif /* KJB_HAVE_ERGO */

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

/** @brief  Sampler for the posterior distribution of data modeled by a GP. */
template<class Mean, class Covariance, class Likelihood>
class Cp_sampler
{
private:
    typedef Cp_sampler<Mean, Covariance, Likelihood> Self;

private:
    const static size_t DEFAULT_NUM_CONTROLS = 8;
    const static size_t DEFAULT_NUM_BURN_ITERATIONS = 100;

public:
    /** @brief  Create a sampler for the GP posterior. */
    Cp_sampler
    (
        const Posterior<Mean, Covariance, Likelihood>& post,
        const Vector& init_outputs,
        size_t num_ctrl_pts = DEFAULT_NUM_CONTROLS,
        size_t num_burn_its = DEFAULT_NUM_BURN_ITERATIONS
    ) :
        likelihood_(post.likelihood()),
        outputs_(init_outputs),
        log_target_(log_posterior(outputs_)),
        propose_(
            post.in_begin(),
            post.in_end(),
            num_ctrl_pts,
            post.mean_function(),
            post.covariance_function()),
#ifdef KJB_HAVE_ERGO
        step_(
            boost::bind(&Self::log_posterior, this, _1),
            boost::bind(propose_, _1, _2)),
#endif
        num_burn_its_(num_burn_its),
        burned_in_(false)
    {
#ifndef KJB_HAVE_ERGO
        KJB_THROW_2(kjb::Missing_dependency, "ergo");
#endif
    }

    /** @brief  Create a sampler from another sampler. */
    Cp_sampler(const Self& sampler) :
        likelihood_(sampler.likelihood_),
        outputs_(sampler.outputs_),
        log_target_(sampler.log_target_),
        propose_(sampler.propose_),
#ifdef KJB_HAVE_ERGO
        step_(
            boost::bind(&Self::log_posterior, this, _1),
            boost::bind(propose_, _1, _2)),
#endif
        num_burn_its_(sampler.num_burn_its_),
        burned_in_(sampler.burned_in_)
    {
#ifndef KJB_HAVE_ERGO
        KJB_THROW_2(kjb::Missing_dependency, "ergo");
#endif
    }

    /** @brief  Create a sampler from another sampler. */
    Self& operator=(const Self& sampler)
    {
        if(this == &sampler) return *this;

        likelihood_ = sampler.likelihood_;
        outputs_ = sampler.outputs_;
        log_target_ = sampler.log_target_;
        propose_ = sampler.propose_;
        num_burn_its_ = sampler.num_burn_its_;
        burned_in_ = sampler.burned_in_;

#ifdef KJB_HAVE_ERGO
        step_ = ergo::mh_step<Vector>(
                    boost::bind(&Self::log_posterior, this, _1),
                    boost::bind(propose_, _1, _2));
#else
        KJB_THROW_2(kjb::Missing_dependency, "ergo");
#endif

        return *this;
    }

public:
    /** @brief  Sample until burned in. */
    void burn_in() const;

    /** @brief  Generate a sample from the posterior distribution. */
    void sample() const
    {
#ifdef KJB_HAVE_ERGO
        step_(outputs_, log_target_);
#else
        KJB_THROW_2(kjb::Missing_dependency, "ergo");
#endif
    }

    /** @brief  Get current sample. */
    const Vector& current() const { return outputs_; }

private:
    /** @brief  Posterior for this sampler. */
    double log_posterior(const Vector& m) const { return likelihood_(m); }

private:
    // data members
    Likelihood likelihood_;
    mutable Vector outputs_;
    mutable double log_target_;
    Cp_proposer<Mean, Covariance> propose_;
#ifdef KJB_HAVE_ERGO
    ergo::mh_step<Vector> step_;
#endif
    size_t num_burn_its_;
    mutable bool burned_in_;
};

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

template<class M, class C, class L>
void Cp_sampler<M, C, L>::burn_in() const
{
#ifdef KJB_HAVE_ERGO
    if(burned_in_) return;

    for(size_t i = 0; i < num_burn_its_; i++)
    {
        step_(outputs_, log_target_);
    }

    burned_in_ = true;
#else
    KJB_THROW_2(kjb::Missing_dependency, "ergo");
#endif
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

// I had to put this outside the class body because there is a circular
// reference between Posterior and Cp_sampler due to caching
template<class M, class C, class L>
template<class InIt, class OutIt>
Posterior<M, C, L>::Posterior
(
    const M& mf,
    const C& cf,
    const L& likelihood,
    InIt first_in,
    InIt last_in,
    OutIt first_out,
    OutIt last_out
) :
    likelihood_(likelihood),
    outputs_(first_out, last_out),
    prior_(mf, cf, first_in, last_in),
    sampler_(*this, apply_mf(mf, first_in, last_in)),
    sampler_dirty_(false)
{
    size_t insz = std::distance(prior_.in_begin(), prior_.in_end());
    IFT(insz == outputs_.size(), Dimension_mismatch,
        "Cannot create GP posterior; inputs and outputs have different "
        "dimensions.");

    likelihood_.set_outputs(outputs_.begin(), outputs_.end());
}

/**
 * TODO Added from the old gp_posterior.h since TIES currently uses this function
 * Will make the ties to use the current control point sampler later 
 * @brief   Generate control inputs which are a uniform subset of the real
 *          inputs.
 */
template<class InIt>
Inputs uniform_control_inputs
(
    InIt first_in,
    InIt last_in,
    size_t num_control_points
)
{
    size_t N = std::distance(first_in, last_in);
    size_t K = N / num_control_points;
    size_t r = N % num_control_points;

    Inputs control_inputs(num_control_points);

    size_t i = 0;
    std::advance(first_in, r);
    while(first_in != last_in)
    {
        assert(i < num_control_points);

        control_inputs[i++] = *first_in;
        std::advance(first_in, K);
    }

    assert(i == num_control_points);

    return control_inputs;
}

}} //namespace kjb::gp

#endif /* GP_POSTERIOR_H_INCLUDED */

