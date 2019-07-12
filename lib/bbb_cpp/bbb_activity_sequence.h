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

#ifndef B3_ACTIVITY_SEQUENCE_H
#define B3_ACTIVITY_SEQUENCE_H

#include "bbb_cpp/bbb_physical_activity.h"
#include "bbb_cpp/bbb_intentional_activity.h"

#include <string>
#include <vector>
#include <iostream>
#include <boost/variant.hpp>

namespace kjb {
namespace bbb {

/**
 * @class   Activity_sequence
 *
 * Class represents a sequence of activities.
 */
class Activity_sequence
{
public:
    typedef boost::variant<Physical_activity, Intentional_activity> Activity;
    typedef std::vector<Activity>::const_iterator const_iterator;

public:
    /** @brief  Create an empty activity sequence. */
    Activity_sequence(const std::string& role) : role_(role) {}

    /** @brief  Gets the role of this activity sequence. */
    const std::string& role() const { return role_; }

    /** @brief  Iterator to first activity. */
    const_iterator begin() const { return activities_.begin(); }

    /** @brief  Iterator to one-past-the-last activity. */
    const_iterator end() const { return activities_.end(); }

    /** @brief  Get the jth actiity. */
    const Activity& activity(size_t j) const { return activities_[j]; }

    /** @brief  Get the jth actiity. */
    size_t size() const { return activities_.size(); }

    /** @brief  Add intentional activity. */
    void add(const Intentional_activity& activity)
    { 
        activities_.push_back(activity);
    }

    /** @brief  Add physical activity. */
    void add(const Physical_activity& activity)
    { 
        activities_.push_back(activity);
    }

private:
    std::string role_;
    std::vector<Activity> activities_;
};

/**
 * @class   Output_activity
 *
 * Helper class used to output an activity of either type
 */
class Output_activity : public boost::static_visitor<>
{
public:
    Output_activity(std::ostream& ost) : ost_(&ost) {}

    /** @brief  Output an (generic) activity. */
    template<class A>
    void operator()(const A& act) { *ost_ << act; }

    std::ostream* ost_;
};

/** Helper class used to get the name of an activity. */
class Get_name : public boost::static_visitor<>
{
public:
    Get_name(std::string* nm) : name(nm) {}

    /** @brief  Get the name of a generic activity. */
    template<class A>
    void operator()(const A& act) const { *name = act.name(); }

    std::string* name;
};

/** Helper class used to get the start time of an activity. */
class Get_start : public boost::static_visitor<>
{
public:
    Get_start(size_t* s) : start(s) {}

    /** @brief  Get the start time of a generic activity. */
    template<class A>
    void operator()(const A& act) const { *start = act.start(); }

    size_t* start;
};

/** Helper class used to get the end time of an activity. */
class Get_end : public boost::static_visitor<>
{
public:
    Get_end(size_t* e) : end(e) {}

    /** @brief  Get the end time of a generic activity. */
    template<class A>
    void operator()(const A& act) const { *end =  act.end(); }

    size_t* end;
};

/** Helper class used to get the trajectories of an activity. */
class Get_trajectories : public boost::static_visitor<>
{
public:
    Get_trajectories(Traj_set* ts) : trajs(ts) {}

    /** @brief  Get the trajectories of a generic activity. */
    template<class A>
    void operator()(const A& act) const { *trajs = act.trajectories(); }

    Traj_set* trajs;
};

/** @brief  Determines whether two activities are the same. */
template<class A>
bool same_activity(const Activity_sequence::Activity& activity, const A& piact)
{
    return boost::get<A>(&activity) == &piact;
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

/** @brief  Push an activity sequence to an output stream. */
std::ostream& operator<<(std::ostream& ost, const Activity_sequence& aseq);

}} // namespace kjb::bbb

#endif /*B3_ACTIVITY_SEQUENCE_H */

