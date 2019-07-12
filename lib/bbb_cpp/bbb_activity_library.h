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

#ifndef B3_ACTIVITY_LIBRARY_H
#define B3_ACTIVITY_LIBRARY_H

#include "l_cpp/l_exception.h"
#include "gp_cpp/gp_covariance.h"
#include "m_cpp/m_vector.h"
#include "m_cpp/m_matrix.h"

#include <map>
#include <set>
#include <string>
#include <vector>
#include <utility>
#include <algorithm>

namespace kjb {
namespace bbb {

class Activity_library
{
private:
    typedef std::map<std::string, double> Sd_map;
    typedef std::map<std::string, std::vector<double> > Sv_map;
    typedef std::map<std::string, std::pair<double, double> > Sp_map;
    typedef std::map<std::string, std::pair<Vector, Matrix> > Sc_map;
    typedef std::vector<std::string> S_vec;
    typedef std::vector<std::vector<double> > V_vec;

public:
    /** @brief  Constructor. */
    Activity_library(const std::string& lib_dp);

public:
    /** @brief  Get the name of the a-th activity. */
    const std::string& activity_name(size_t a) const
    {
        return activities_.at(a);
    }

    /** @brief  Get number of activities avialable. */
    size_t num_activities() const { return activities_.size(); }

    /** @brief  Get the name of the r-th role. */
    const std::string& role_name(size_t r) const { return roles_.at(r); }

    /** @brief  Get number of roles avialable. */
    size_t num_roles() const { return roles_.size(); }

    /** @brief  Is the given activity intentional?. */
    bool is_intentional(const std::string& act) const
    {
        return std::find(iacts_.begin(), iacts_.end(), act) != iacts_.end();
    }

    /** @brief  Is the given activity physical?. */
    bool is_physical(const std::string& act) const
    {
        return std::find(pacts_.begin(), pacts_.end(), act) != pacts_.end();
    }

    /** @brief  Get a list of the intentional activity names. */
    template<class Iter>
    void intentional_names(Iter out) const
    {
        std::copy(iacts_.begin(), iacts_.end(), out);
    }

    /** @brief  Get a list of the physical activity names. */
    template<class Iter>
    void physical_names(Iter out) const
    {
        std::copy(pacts_.begin(), pacts_.end(), out);
    }

public:
    /** @brief  Get the concentration parameter for an activity. */
    const Sd_map::mapped_type& group_concentration(const std::string& act) const
    {
        Sd_map::const_iterator pr_p = crp_params_.find(act);
        IFT(pr_p != crp_params_.end(), Illegal_argument,
            "Cannot get group concentration: activity does not exist.");

        return pr_p->second;
    }

    /** @brief  Get the distribution over roles for an activity. */
    const Sv_map::mapped_type& role_distribution(const std::string& act) const
    {
        Sv_map::const_iterator pr_p = role_dists_.find(act);
        IFT(pr_p != role_dists_.end(), Illegal_argument,
            "Cannot get role distribution: activity does not exist.");

        return pr_p->second;
    }

    /** @brief  Get the trajectory kernel for an activity. */
    gp::Sqex trajectory_kernel(const std::string& act) const
    {
        Sp_map::const_iterator pr_p = kernel_params_.find(act);
        IFT(pr_p != kernel_params_.end(), Illegal_argument,
            "Cannot get kernel: activity does not exist.");

        return gp::Sqex(pr_p->second.first, pr_p->second.second);
    }

    /** @brief  Get the MC parameters for an activity. */
    const Sc_map::mapped_type& markov_chain(const std::string& role) const
    {
        Sc_map::const_iterator pr_p = role_mcs_.find(role);
        IFT(pr_p != role_mcs_.end(), Illegal_argument,
            "Cannot get role chain: role does not exist.");

        return pr_p->second;
    }

    /** @brief  Sample the parameters of an activity. */
    Vector sample_parameters
    (
        const std::string& name,
        const std::string& pt_name,
        const Vector& pt_params
    ) const;

    /** @brief  Returns true if the given activity has a target. */
    bool has_target(const std::string& act) const
    {
        return act_target_.count(act);
    }

public:
    /** @brief  Get the index of a role (by name). */
    size_t role_index(const std::string& role) const
    {
        std::vector<std::string>::const_iterator str_p = std::find(
                                                            roles_.begin(),
                                                            roles_.end(),
                                                            role);
        IFT(str_p != roles_.end(), Illegal_argument, "Role does not exist");

        return std::distance(roles_.begin(), str_p);
    }

private:
    std::pair<S_vec, V_vec> read_strs_and_vecs(const std::string& path);

private:
    // lists
    std::vector<std::string> activities_;
    std::vector<std::string> iacts_;
    std::vector<std::string> pacts_;
    std::vector<std::string> roles_;

    // other parameters
    Sd_map crp_params_;
    Sv_map role_dists_;
    Sp_map kernel_params_;
    Sc_map role_mcs_;
    std::set<std::string> act_target_;
};

}} // namespace kjb::bbb

#endif /* B3_ACTIVITY_LIBRARY_H */

