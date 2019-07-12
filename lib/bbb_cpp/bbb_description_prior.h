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

#ifndef B3_DESCRIPTION_PRIOR_H
#define B3_DESCRIPTION_PRIOR_H

#include "bbb_cpp/bbb_association_prior.h"
#include "bbb_cpp/bbb_activity_sequence_prior.h"
#include "bbb_cpp/bbb_trajectory_prior.h"
#include "bbb_cpp/bbb_description.h"
#include "bbb_cpp/bbb_physical_activity.h"
#include "bbb_cpp/bbb_intentional_activity.h"
#include "bbb_cpp/bbb_activity_library.h"

#include <boost/variant.hpp>

namespace kjb {
namespace bbb {

/**
 * @class   Description_prior
 *
 * Class represents the prior of a description of a video.
 */
class Description_prior
{
public:
    /** @brief  Create a desciption prior composed of the given priors. */
    Description_prior
    (
        const Intentional_activity& root,
        Association_prior& association_prior,
        Activity_sequence_prior& activity_sequence_prior,
        Trajectory_prior& trajectory_prior,
        const Activity_library& library
    ) :
        root_(root),
        ass_prior_p_(&association_prior),
        as_prior_p_(&activity_sequence_prior),
        traj_prior_p_(&trajectory_prior),
        lib_(library)
    {}

    /** @brief  Compute the log-prior of a description. */
    double operator()(const Description& desc) const;

    /** @brief  Root activity for this prior. */
    const Intentional_activity& root_activity() const { return root_; }

    /** @brief  Return the association prior used by this description prior. */
    Association_prior& association_prior() const { return *ass_prior_p_; }

    /** @brief  Return the sequence prior used by this description prior. */
    As_prior& activity_sequence_prior() const { return *as_prior_p_; }

    /** @brief  Return the trajectory prior used by this description prior. */
    Trajectory_prior& trajectory_prior() const { return *traj_prior_p_; }

    /** @brief  Get activity library. */
    const Activity_library& library() const { return lib_; }

private:
    Intentional_activity root_;
    mutable Association_prior* ass_prior_p_;
    mutable Activity_sequence_prior* as_prior_p_;
    mutable Trajectory_prior* traj_prior_p_;
    const Activity_library& lib_;
};

/**
 * @class   Sample_tree
 *
 * Helper class used to sample subtrees of a description.
 */
class Sample_tree : public boost::static_visitor<>
{
public:
    Sample_tree(const Description_prior& prior, Description& description) :
        prior_(prior), description_(description) {}

    /** @brief  Acenstrally sample from an intentional activity node. */
    void operator()(const Intentional_activity& root);

    /** @brief  Sample from a physical activity node (does nothing). */
    void operator()(const Physical_activity&) {}

private:
    const Description_prior& prior_;
    Description& description_;
};

/** @brief  Sample a description from the prior. */
Description sample(const Description_prior& prior);

}} // namespace kjb::bbb

#endif /*B3_DESCRIPTION_PRIOR_H */

