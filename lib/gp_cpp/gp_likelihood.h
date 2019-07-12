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

#ifndef GP_LIKELIHOOD_H_INCLUDED
#define GP_LIKELIHOOD_H_INCLUDED

#include <m_cpp/m_matrix.h>
#include <m_cpp/m_vector.h>
#include <l_cpp/l_exception.h>
#include <prob_cpp/prob_distribution.h>
#include <prob_cpp/prob_pdf.h>

namespace kjb {
namespace gp {

class Linear_gaussian
{
public:
    /** @brief  Create a linear-Gaussian likelihood with the given values. */
    Linear_gaussian
    (
        const Matrix& covariance,
        const Matrix& scale,
        const Vector& offset
    ) :
        cov_(covariance),
        scale_(scale),
        offset_(offset),
        dist_(1),
        scale_id_(false),
        offset_zero_(false),
        cov_dirty_(true)
    {
        const size_t M = scale_.get_num_rows();

        IFT(cov_.get_num_rows() == M, Illegal_argument,
            "Cannot create LG likelihood: covariance matrix wrong size.");

        IFT(cov_.get_num_cols() == M, Illegal_argument,
            "Cannot create LG likelihood: covariance must be square.");

        IFT(offset_.get_length() == M, Illegal_argument,
            "Cannot create LG likelihood: offset is wrong size.");
    }

    /**
     * @brief   Create a linear-Gaussian likelihood with identity scale
     *          and zero offset.
     */
    Linear_gaussian(const Matrix& covariance) :
        cov_(covariance),
        scale_(create_identity_matrix(cov_.get_num_rows())),
        offset_(cov_.get_num_rows(), 0.0),
        dist_(1),
        scale_id_(true),
        offset_zero_(true),
        cov_dirty_(true)
    {
        IFT(cov_.get_num_rows() == cov_.get_num_cols(), Illegal_argument,
            "Cannot create LG likelihood: covariance matrix wrong size.");
    }

    /**
     * @brief   Create a linear-Gaussian likelihood with a single variance
     *          identity scale and zero offset.
     */
    Linear_gaussian(double std_dev, size_t N) :
        cov_(create_diagonal_matrix(N, std_dev*std_dev)),
        scale_(create_identity_matrix(N)),
        offset_(static_cast<int>(N), 0.0),
        dist_(1),
        scale_id_(true),
        offset_zero_(true),
        cov_dirty_(true)
    {}

    /** @brief  Set the covariance matrix of this likelihood. */
    void set_covariance(const Matrix& S) { cov_ = S; cov_dirty_ = true; }

    /** @brief  Set the scale matrix of this likelihood. */
    void set_scale(const Matrix& A) { scale_ = A; }

    /** @brief  Set the offset vector of this likelihood. */
    void set_offset(const Vector& mu) { offset_ = mu; }

    /** @brief  Set the data (outputs) for this likelihood. */
    template<class Iter>
    void set_outputs(Iter ft, Iter lt) { outputs_.assign(ft, lt); }

    /** @brief  Get the covariance matrix of this likelihood. */
    const Matrix& covariance() const { return cov_; }

    /** @brief  Get the scale matrix of this likelihood. */
    const Matrix& scale() const { return scale_; }

    /** @brief  Get the offset vector of this likelihood. */
    const Vector& offset() const { return  offset_; }

    /** @brief  Evaluate likelihood on a set of outputs. */
    double operator()(const Vector& f) const
    {
        IFT(!outputs_.empty(), Runtime_error,
            "Cannot evaluate likelihood: outputs not set.");

        update_normal(f);
        assert(dist_.get_dimension() == outputs_.get_length());

        return log_pdf(dist_, outputs_);
    }

private:
    /** @brief  Update internal normal distribution. */
    void update_normal(const Vector& f) const
    {
        IFT(f.size() == offset_.size(), Illegal_argument,
            "Cannot compute likelihood: argument wrong dimension.");

        Vector mean = f;
        if(!scale_id_)
        {
            mean = scale_ * mean;
        }

        if(!offset_zero_)
        {
            mean += offset_;
        }

        if(cov_dirty_)
        {
            dist_.set_covariance_matrix(cov_, mean);
            cov_dirty_ = false;
        }
        else
        {
            dist_.set_mean(mean);
        }
    }

    Matrix cov_;
    Matrix scale_;
    Vector offset_;
    Vector outputs_;
    mutable MV_normal_distribution dist_;
    bool scale_id_;
    bool offset_zero_;
    mutable bool cov_dirty_;
};

}} //namespace kjb::gp

#endif /*GP_LIKELIHOOD_H_INCLUDED */

