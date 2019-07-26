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

#ifndef B3_INTENTIONAL_ACTIVITY_H
#define B3_INTENTIONAL_ACTIVITY_H

#include "l/l_sys_debug.h"  /* For ASSERT */
#include "bbb_cpp/bbb_traj_set.h"
#include "m_cpp/m_vector.h"

#include <string>
#include <iostream>
#include <algorithm>
#include <iterator>

namespace kjb {
namespace bbb {

/**
 * @class   Intentional_activity
 *
 * Class represents a generic intentional activity.
 */
class Intentional_activity
{
public:
    typedef Vector Param;

public:
    /** @brief  Create an empty IA with the given name and roles. */
    Intentional_activity
    (
        const std::string& name,
        size_t start,
        size_t end,
        const Param& params,
        const Traj_set& trajs
    ) :
        name_(name),
        start_(start),
        end_(end),
        params_(params),
        trajs_(trajs)
    {}

    /** @brief  Gets the name of this IA. */
    const std::string& name() const { return name_; }

    /** @brief  Gets the start frame of this IA. */
    size_t start() const { return start_; }

    /** @brief  Gets the end frame of this IA. */
    size_t end() const { return end_; }

    /** @brief  Gets the size of this IA. */
    size_t size() const { return end() - start() + 1; }

    /** @brief  Get this IA's parameters. */
    const Param& parameters() const { return params_; }

    /** @brief  Get this IA's trajectories. */
    const Traj_set& trajectories() const { return trajs_; }

private:
    std::string name_;
    size_t start_;
    size_t end_;
    Param params_;
    Traj_set trajs_;
};

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

/** @brief  Push an intentional activity to an output stream. */
inline
std::ostream& operator<<(std::ostream& ost, const Intentional_activity& activity)
{
    // sanity check
    ASSERT(activity.trajectories().size() != 0);

    // output name and times
    ost << activity.name() << "(";
    ost << activity.start() << ", ";
    ost << activity.end() << ")";
    ost << " : ";

    // output params
    ost << "[";
    ost << activity.parameters();
    ost << "]";
    ost << " : ";

    // output member trajectories
    Traj_set::const_iterator szt_p = --activity.trajectories().end();
    ost << "{";
    std::copy(
        activity.trajectories().begin(), szt_p,
        std::ostream_iterator<Traj_set::value_type>(ost, ", "));
    ost << *szt_p << "}";

    return ost;
}

}} // namespace kjb::bbb

#endif /*B3_INTENTIONAL_ACTIVITY_H */

