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

#ifndef B3_ACTIVITY_SEQUENCE_PRIOR_H
#define B3_ACTIVITY_SEQUENCE_PRIOR_H

#include "bbb_cpp/bbb_activity_sequence.h"
#include "bbb_cpp/bbb_parameter_prior.h"
#include "bbb_cpp/bbb_traj_set.h"
#include "bbb_cpp/bbb_description.h"

#include <vector>
#include <string>
#include <utility>

namespace kjb {
namespace bbb {

/**
 * @class   Activity_sequence_prior
 *
 * Class represents the prior of a sequence of activities.
 */
class Activity_sequence_prior
{
private:
    typedef std::pair<std::vector<size_t>, std::vector<size_t> > Chain_pair;

public:
    /** @brief  Construct an activity sequence prior. */
    Activity_sequence_prior
    (
        Parameter_prior& param_prior,
        const Activity_library& lib
    ) :
        param_prior_p_(&param_prior),
        parent_p_(0),
        desc_p_(0),
        lib_(lib)
    {}

    /** @brief  Set role. */
    void set_role(const std::string& role) { role_ = role; }

    /** @brief  Set trajectories for this sequence. */
    void set_trajectories(const Traj_set& trajs) { trajs_ = trajs; }

    /** @brief  Set parent activity. */
    void set_parent(const Intentional_activity& act) { parent_p_ = &act; }

    /** @brief  Set description. */
    void set_description(const Description& desc) { desc_p_ = &desc; }

    /** @brief  Get parameter prior. */
    Parameter_prior& parameter_prior() const { return *param_prior_p_; }

    /** @brief  Get role. */
    const std::string& role() const { return role_; }

    /** @brief  Get trajectories for this sequence. */
    const Traj_set& trajectories() const { return trajs_; }

    /** @brief  Get parent activity. */
    const Intentional_activity& parent() const { return *parent_p_; }

    /** @brief  Get description. */
    const Description& description() const { return *desc_p_; }

    /** @brief  Get activity library. */
    const Activity_library& library() const { return lib_; }

private:
    friend Activity_sequence sample(const Activity_sequence_prior& prior);

    /** @brief  Simulate a mariov chain for T steps. */
    std::vector<size_t> sample_markov_chain(size_t T) const;

    /** @brief  Simulate a mariov chain for T steps. */
    Chain_pair condense_chain(const std::vector<size_t>& chain, size_t st) const;

    mutable Parameter_prior* param_prior_p_;
    std::string role_;
    Traj_set trajs_;
    const Intentional_activity* parent_p_;
    const Description* desc_p_;
    const Activity_library& lib_;
};

// useful typedef
typedef Activity_sequence_prior As_prior;

/** @brief  Sample an activity sequence from the given prior. */
Activity_sequence sample(const As_prior& prior);

}} // namespace kjb::bbb

#endif /*B3_ACTIVITY_SEQUENCE_PRIOR_H */

