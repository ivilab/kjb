/* =========================================================================== *
   |
   |  Copyright (c) 1994-2011 by Kobus Barnard (author)
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

#ifndef B3_TRAJECTORY_H
#define B3_TRAJECTORY_H

#include "l/l_sys_debug.h"  /* For ASSERT */
#include "m_cpp/m_vector.h"
#include "l_cpp/l_functors.h"
#include "l_cpp/l_exception.h"
#include <algorithm>
#include <vector>
#include <iterator>
#include <iostream>
#include <boost/bind.hpp>

namespace kjb {
namespace bbb {

/**
 * @class   Trajectory
 *
 * Class represents a trajectory in R^D.
 */
class Trajectory
{
public:
    typedef Vector vec_t;

public:
    /** @brief  Create an empty trajectory. */
    Trajectory() : start_(0) {}

    /** @brief  Set points of this trajectory. */
    template<class VecIter>
    void set_positions(size_t st, VecIter first, VecIter last);

    /** @brief  Set dimensions of this trajectory. */
    template<class VecIter>
    void set_dimensions(size_t st, VecIter first, VecIter last);

    /** @brief  Append by position. */
    template<class VecIter>
    void append_positions(VecIter first, VecIter last);

    /** @brief  Append by dimension. */
    template<class VecIter>
    void append_dimensions(VecIter first, VecIter last);

    /** @brief  Gets the position of this person at the given frame. */
    vec_t pos(size_t frame) const
    {
        IFT(dimensions() != 0, Runtime_error,
            "Cannot get position: trajector is empty.");

        IFT(frame >= start() && frame <= end(), Illegal_argument,
            "Cannot get position: frame out of range.");

        size_t t = frame - start();
        vec_t x(static_cast<int>(dimensions()), 0.0);
        std::transform(
            trajectory_.begin(),
            trajectory_.end(),
            x.begin(),
            boost::bind(
                static_cast<const double&(vec_t::*)(int) const>(&vec_t::at),
                _1, t));

        return x;
    }

    /** @brief  Gets the trajectory of this person. */
    template<class VecIter>
    void copy(VecIter out) const;

    /** @brief  Gets the trajectory of this person. */
    const vec_t& dim(size_t d) const
    {
        IFT(dimensions() != 0, Runtime_error,
            "Cannot get dim: trajectory is empty.");

        IFT(d < dimensions(), Illegal_argument,
            "Cannot get dth dimension of trajectory; d is too large.");

        return trajectory_[d];
    }

    /** @brief  Gets the dimensionality of this person's trajectory. */
    size_t dimensions() const
    {
        if(trajectory_.empty()) return 0;
        return trajectory_.size();
    }

    /** @brief  Gets the start frame of this person. */
    size_t start() const { return start_; }

    /** @brief  Gets the end frame of this person. */
    size_t end() const { return start() + size() - 1; }

    /** @brief  Gets the size of this person. */
    size_t size() const
    {
        if(dimensions() == 0) return 0;
        return trajectory_[0].size();
    }

private:
    size_t start_;
    std::vector<vec_t> trajectory_;
};

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

template<class VecIter>
void Trajectory::set_positions(size_t st, VecIter first, VecIter last)
{
    typedef typename std::iterator_traits<VecIter>::value_type Vec;

    //IFT(st > 0, Runtime_error, "Cannot set ponts: start is 0.");

    size_t sz = std::distance(first, last);
    IFT(sz > 0, Runtime_error, "Cannot set ponts: size is 0.");

    size_t dim = first->size();
    IFT(dim > 0, Runtime_error, "Cannot set ponts: dimension is 0.");

    start_ = st;
    trajectory_.resize(dim, vec_t(static_cast<int>(sz)));
    for(size_t d = 0; d < dim; d++)
    {
        std::transform(
                first, last,
                trajectory_[d].begin(),
                boost::bind(
                    static_cast<const double&(Vec::*)(int) const>(&Vec::at),
                    _1, d));
    }
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

template<class VecIter>
void Trajectory::set_dimensions(size_t st, VecIter first, VecIter last)
{
    //IFT(st > 0, Runtime_error, "Cannot set dims: start is 0.");

    size_t dim = std::distance(first, last);
    IFT(dim > 0, Runtime_error, "Cannot set dims: dimension is 0.");

    size_t sz = first->size();
    IFT(sz > 0, Runtime_error, "Cannot set dims: size is 0.");

    start_ = st;
    trajectory_.resize(dim);
    std::copy(first, last, trajectory_.begin());

    // check for inconsistencies
    for(size_t d = 0; d < dim; d++)
    {
        IFT(trajectory_[d].size() == size(), Illegal_argument,
            "Cannot set trajectory dimensions; they must be same size.");
    }
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

template<class VecIter>
void Trajectory::append_positions(VecIter first, VecIter last)
{
    IFT(dimensions() != 0, Runtime_error,
        "Cannot append positions: trajectory empty.");

    typedef typename std::iterator_traits<VecIter>::value_type Vec;

    size_t sz = std::distance(first, last);
    if(sz == 0) return;

    size_t dim = first->size();
    IFT(dim == dimensions(), Runtime_error,
        "Cannot append positions: dimension mismatch");

    size_t new_sz = size() + sz;
    for(size_t d = 0; d < dim; d++)
    {
        trajectory_[d].reserve(new_sz);
        std::transform(
                first, last,
                std::back_inserter(trajectory_[d]),
                boost::bind(
                    static_cast<const double&(Vec::*)(int) const>(&Vec::at),
                    _1, d));

        ASSERT(trajectory_[d].size() == size());
    }
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

template<class VecIter>
void Trajectory::append_dimensions(VecIter first, VecIter last)
{
    IFT(dimensions() != 0, Runtime_error,
        "Cannot append positions: trajectory empty.");

    size_t dim = std::distance(first, last);
    IFT(dim == dimensions(), Runtime_error,
        "Cannot append positions: dimension mismatch");

    size_t sz = first->size();
    if(sz == 0) return;

    size_t new_sz = size() + sz;
    for(size_t d = 0; d < dim; d++, first++)
    {
        trajectory_[d].reserve(new_sz);
        std::copy(
            first->begin(),
            first->end(),
            std::back_inserter(trajectory_[d]));

        ASSERT(trajectory_[d].size() == size());
    }
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

template<class VecIter>
void Trajectory::copy(VecIter out) const
{
    IFT(dimensions() != 0, Runtime_error, "Cannot copy: empty trajectory.");

    const size_t s = start();
    const size_t e = end();
    for(size_t t = s; t <= e; t++)
    {
        *out++ = pos(t);
    }
}

/** @brief  Push trajectory to output stream. */
std::ostream& operator<<(std::ostream& ost, const Trajectory& traj);

}} // namespace kjb::bbb

#endif /*B3_TRAJECTORY_H */

