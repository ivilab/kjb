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

#ifndef B3_TRAJ_SET_H
#define B3_TRAJ_SET_H

#include "bbb_cpp/bbb_data.h"

#include <set>
#include <boost/foreach.hpp>

namespace kjb {
namespace bbb {

/**
 * @class   Traj_set
 *
 * Class represents a set of trajectories; more specifically, a subset of
 * the trajectories in the data.
 */
class Traj_set
{
private:
    typedef std::set<size_t> Iset;

public:
    typedef Iset::const_iterator const_iterator;
    typedef Iset::const_reverse_iterator const_reverse_iterator;
    typedef Iset::value_type value_type;

public:
    /** @brief  Construct an empty trajectory seet. */
    Traj_set() {}

    /** @brief  Construct a trajectory set from the given trajectory indices. */
    template<class TrIt>
    Traj_set(TrIt first, TrIt last) : trajectories_(first, last) {}

    /** @brief  Add trajectory to this association. */
    template<class SizeType>
    void insert(const SizeType& j) { trajectories_.insert(j); }

    /** @brief  Add a sequence of indices to this association. */
    template<class TrIt>
    void insert(TrIt first, TrIt last) { trajectories_.insert(first, last); }

    /** @brief  Clear this trajectory set. */
    void clear() { trajectories_.clear(); }

    /** @brief  Iterator to first index. */
    const_iterator begin() const { return trajectories_.begin(); }

    /** @brief  Iterator to one-past-the-end index. */
    const_iterator end() const { return trajectories_.end(); }

    /** @brief  Iterator to reverse first index. */
    const_reverse_iterator rbegin() const { return trajectories_.rbegin(); }

    /** @brief  Iterator to reverse one-past-the-end index. */
    const_reverse_iterator rend() const { return trajectories_.rend(); }

    /** @brief  Number of trajectories in this set. */
    size_t size() const { return trajectories_.size(); }

    /**
     * @brief   Get a set of pointers to trajectory. OutIt must point to
     *          const Trajectory*.
     */
    template<class OutIt>
    void trajectories(const Data& data, OutIt output) const;

private:
    Iset trajectories_;
};

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

template<class OutIt>
void Traj_set::trajectories(const Data& data, OutIt output) const
{
    size_t N = data.size();
    BOOST_FOREACH(size_t j, trajectories_)
    {
        IFT(j < N, Runtime_error,
            "Cannot get trajectories; invalid association.");

        *output++ = &data.trajectory(j);
    }
}

}} // namespace kjb::bbb

#endif /*B3_TRAJ_SET_H */

