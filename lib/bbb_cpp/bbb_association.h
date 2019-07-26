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

#ifndef B3_ASSOCIATION_H
#define B3_ASSOCIATION_H

#include "bbb_cpp/bbb_traj_set.h"

#include <string>
#include <vector>
#include <algorithm>
#include <iterator>
#include <boost/foreach.hpp>

namespace kjb {
namespace bbb {

/**
 * @brief   Group
 * 
 * Class that represents a group of individuals.
 */
struct Group
{
public:
    Group(const std::string& role) : role_(role) {}

    void add(size_t j) { trajectories_.insert(j); }

    const Traj_set& trajectories() const { return trajectories_; }

    const std::string& role() const { return role_; }

private:
    std::string role_;
    Traj_set trajectories_;
};

/**
 * @class   Association
 *
 * Class represents an assignment of individuals to groups.
 */
class Association
{
public:
    /** @brief  Construct an associaiton of the given trajectories. */
    Association(const Traj_set& trajs) : trajs_(trajs) {}

    /** @brief  Insert values for this association. */
    template<class SetIt, class StrIt>
    void set(SetIt first_set, SetIt last_set, StrIt first_name);

    /** @brief  Get the kth group of this association. */
    const Group& group(size_t k) const { return groups_[k]; }

    /** @brief  Get the number of groups in this association. */
    size_t num_groups() const { return groups_.size(); }

private:
    /**
     * @brief   Quick check for consistency. Passing this is test is only
     *          a necessary condition for consistency.
     */
    void check_consistent() const;

    Traj_set trajs_;
    std::vector<Group> groups_;
};

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

template<class SetIt, class StrIt>
void Association::set(SetIt first_set, SetIt last_set, StrIt first_name)
{
    size_t K = std::distance(first_set, last_set);
    groups_.reserve(K);

    for(; first_set != last_set; ++first_set, ++first_name)
    {
        groups_.push_back(Group(*first_name));
        BOOST_FOREACH(size_t j, *first_set)
        {
            Traj_set::const_iterator j_p = trajs_.begin();
            std::advance(j_p, j);
            groups_.back().add(*j_p);
        }
    }

    check_consistent();
}

}} // namespace kjb::bbb

#endif /*B3_ASSOCIATION_H */

