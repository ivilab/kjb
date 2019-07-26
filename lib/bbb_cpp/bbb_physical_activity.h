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

#ifndef B3_PHYSICAL_ACTIVITY_H
#define B3_PHYSICAL_ACTIVITY_H

#include "l/l_sys_debug.h"  /* For ASSERT */
#include "bbb_cpp/bbb_trajectory.h"
#include "bbb_cpp/bbb_traj_set.h"

#include <string>
#include <iostream>
#include <iterator>
#include <algorithm>

namespace kjb {
namespace bbb {

/**
 * @class   Physical_activity
 *
 * Class represents a generic physical activity.
 */
class Physical_activity
{
public:
    /** @brief  Create an empty PA. */
    Physical_activity
    (
        const std::string& name,
        const Trajectory& trajectory,
        const Traj_set& trajs
    ) :
        name_(name),
        trajectory_(trajectory),
        trajs_(trajs)
    {}

    /** @brief  Set this PA's trajectory. */
    void set_trajectory(const Trajectory& traj) { trajectory_ = traj; }

    /** @brief  Rename this PA. */
    void rename(const std::string& nm) { name_ = nm; }

    /** @brief  Get this PA's name. */
    const std::string& name() const { return name_; }

    /** @brief  Gets the start frame of this PA. */
    size_t start() const { return trajectory_.start(); }

    /** @brief  Gets the end frame of this PA. */
    size_t end() const { return trajectory_.end(); }

    /** @brief  Gets the size of this PA. */
    size_t size() const { return trajectory_.size(); }

    /** @brief  Get this PA's trajectory. */
    const Trajectory& trajectory() const { return trajectory_; }

    /** @brief  Get this PA's trajectory association. */
    const Traj_set& trajectories() const { return trajs_; }

private:
    std::string name_;
    Trajectory trajectory_;
    Traj_set trajs_;
};

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

/** @brief  Push an physical activity to an output stream. */
inline
std::ostream& operator<<(std::ostream& ost, const Physical_activity& activity)
{
    // sanity check
    ASSERT(activity.trajectories().size() != 0);

    // output name and times
    ost << activity.name() << "(";
    ost << activity.start() << ",";
    ost << activity.end() << ")";
    ost << " : ";

    // output member trajectories
    Traj_set::const_iterator szt_p = --activity.trajectories().end();
    ost << "{";
    std::copy(
        activity.trajectories().begin(), szt_p,
        std::ostream_iterator<Traj_set::value_type>(ost, ", "));
    ost << *szt_p << "}";

    // output trajectory
    //ost << activity.trajectory();

    return ost;
}

}} // namespace kjb::bbb

#endif /*B3_PHYSICAL_ACTIVITY_H */

