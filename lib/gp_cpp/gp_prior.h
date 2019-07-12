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

#ifndef GP_PRIOR_H_INCLUDED
#define GP_PRIOR_H_INCLUDED

#include <gp_cpp/gp_base.h>
#include <m_cpp/m_matrix.h>
#include <m_cpp/m_vector.h>
#include <prob_cpp/prob_distribution.h>

namespace kjb {
namespace gp {

/** @brief  Represents the prior distribution induced by a Gaussian process. */
template<class Mean, class Covariance>
class Prior
{
public:
    /**
     * @brief   Construct the prior distribution for a GP with the given
     *          mean and covariance functions, at the given inputs.
     */
    template<class InIt>
    Prior(const Mean& mf, const Covariance& cf, InIt first, InIt last) :
        mean_func_(mf),
        cov_func_(cf),
        inputs_(first, last),
        dist_(1),
        mf_dirty_(true),
        cf_dirty_(true),
        in_dirty_(true)
    {}

    /** @brief  Get the mean function for the underlying GP. */
    const Mean& mean_function() const { return mean_func_; }

    /** @brief  Get the covariance function for the underlying GP. */
    const Covariance& covariance_function() const { return cov_func_; }

    /** @brief  Iterator to first input. */
    Inputs::const_iterator in_begin() const { return inputs_.begin(); }

    /** @brief  Iterator to last input. */
    Inputs::const_iterator in_end() const { return inputs_.end(); }

    /** @brief  Set the mean function for the underlying GP. */
    void set_mean_function(const Mean& mf)
    {
        mean_func_ = mf;
        mf_dirty_ = true;
    }

    /** @brief  Set the covariance function for the underlying GP. */
    void set_covariance_function(const Covariance& cf)
    {
        cov_func_ = cf;
        cf_dirty_ = true;
    }

    /** @brief  Set the inputs that determine the prior. */
    template<class InIt>
    void set_inputs(InIt first, InIt last)
    {
        inputs_.resize(std::distance(first, last));
        std::copy(first, last, inputs_.begin());
        in_dirty_ = true;
    }

    /**
     * @brief   Get the actual Gaussian distribution represented by this prior.
     */
    const MV_gaussian_distribution& normal() const { update(); return dist_; }

private:
    void update() const
    {
        Vector mu;
        Matrix K;

        if(in_dirty_ || mf_dirty_)
        {
            mu = apply_mf(mean_func_, inputs_.begin(), inputs_.end());
        }

        if(in_dirty_ || cf_dirty_)
        {
            K = apply_cf(cov_func_, inputs_.begin(), inputs_.end());
        }

        if(in_dirty_ || (mf_dirty_ && cf_dirty_))
        {
            dist_.set_covariance_matrix(K, mu);
        }
        else if(mf_dirty_)
        {
            assert(!in_dirty_);
            assert(!cf_dirty_);
            dist_.set_mean(mu);
        }
        else if(cf_dirty_)
        {
            assert(!in_dirty_);
            assert(!mf_dirty_);
            dist_.set_covariance_matrix(K);
        }
        else
        {
            assert(!in_dirty_);
            assert(!mf_dirty_);
            assert(!cf_dirty_);
        }

        mf_dirty_ = cf_dirty_ = in_dirty_ = false;
    }

    Mean mean_func_;
    Covariance cov_func_;
    Inputs inputs_;
    mutable MV_gaussian_distribution dist_;
    mutable bool mf_dirty_;
    mutable bool cf_dirty_;
    mutable bool in_dirty_;
};

/** @brief  Convenience function to create a GP prior. */
template<class Mean, class Covariance>
inline
Prior<Mean, Covariance> make_prior
(
    const Mean& mf, 
    const Covariance& cf, 
    const Inputs& inputs 
)
{
    return Prior<Mean, Covariance>(mf, cf, inputs.begin(), inputs.end());
}

}} //namespace kjb::gp

#endif /*GP_PRIOR_H_INCLUDED */

