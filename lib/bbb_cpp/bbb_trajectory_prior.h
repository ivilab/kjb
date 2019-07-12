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

#ifndef B3_TRAJECTORY_PRIOR_H
#define B3_TRAJECTORY_PRIOR_H

#include "bbb_cpp/bbb_trajectory.h"
#include "bbb_cpp/bbb_activity_library.h"
#include "l_cpp/l_exception.h"
#include "gp_cpp/gp_predictive.h"
#include "gp_cpp/gp_mean.h"
#include "gp_cpp/gp_covariance.h"
#include "gp_cpp/gp_pdf.h"
#include "m_cpp/m_vector.h"

#include <vector>

namespace kjb {
namespace bbb {

/**
 * @class   Trajectory_prior
 *
 * Class represents the prior of a physical activity trajectory.
 */
class Trajectory_prior
{
private:
    typedef gp::Predictive_nl<gp::Zero, gp::Sqex> Prior;

public:
    /**
     * @brief   Crate a trajectory prior.
     *
     * @param   dim     The dimension of the trajectory
     */
    Trajectory_prior(size_t dim, const Activity_library& lib) :
        dim_(dim),
        start_(0),
        end_(0),
        lib_(lib),
        priors_dirty_(true)
    {
        IFT(dim != 0, Illegal_argument,
            "Cannot create Trajectory prior: dimension cannot be 0.");
    }

    /** @brief  Set the activity name. */
    void set_name(const std::string& name)
    {
        name_ = name;
        priors_dirty_ = true;
    }

    /** @brief  Set the start time. */
    void set_start(size_t start) { start_ = start; priors_dirty_ = true; }

    /** @brief  Set the end time. */
    void set_end(size_t end) { end_ = end; priors_dirty_ = true; }

    /** @brief  Set the endpoint means. */
    void set_endpoint_means(const Vector& mu_s, const Vector& mu_e)
    {
        IFT(mu_s.get_length() == dim_ && mu_e.get_length() == dim_,
            Illegal_argument, "Cannot set traj prior means; bad dimension.");

        mu_s_ = mu_s;
        mu_e_ = mu_e;

        priors_dirty_ = true;
    }

    /** @brief  Get the dimensionality of the trajectories of this prior. */
    size_t dimension() const { return dim_; }

    /** @brief  Get the activity name of the prior. */
    const std::string& name() const { return name_; }

    /** @brief  Get the start time of the prior. */
    size_t start() const { return start_; }

    /** @brief  Get the end time of the prior. */
    size_t end() const { return end_; }

    /** @brief  Get the start mean. */
    const Vector& start_mean() const { return mu_s_; }

    /** @brief  Get the end mean. */
    const Vector& end_mean() const { return mu_e_; }

    /** @brief  Get activity library. */
    const Activity_library& library() const { return lib_; }

    /** @brief  Evaluate this prior on a trajectory. */
    double operator()(const Trajectory& traj) const
    {
        IFT(traj.dimensions() == dim_, Illegal_argument,
            "Cannot compute trajectory prior; wrong trajectory dimension.");

        IFT(traj.size() == end_ - start_ + 1, Illegal_argument,
            "Cannot compute trajectory prior; wrong trajectory size.");

        update_priors();
        double p = 0.0;
        for(size_t d = 0; d < dim_; d++)
        {
            p += log_pdf(priors_[d], traj.dim(d));
        }

        return p;
    }

private:
    /** @brief  Update the underlying GP priors. */
    void update_priors() const;

    // sample() needs to access update_priors()
    friend Trajectory sample(const Trajectory_prior& prior);

    size_t dim_;
    std::string name_;
    size_t start_;
    size_t end_;
    Vector mu_s_;
    Vector mu_e_;
    const Activity_library& lib_;
    mutable std::vector<Prior> priors_;
    mutable bool priors_dirty_;
};

/** @brief  Draw a sample (a Trajectory) from a trajectory prior. */
inline
Trajectory sample(const Trajectory_prior& prior)
{
    prior.update_priors();

    const size_t D = prior.dimension();
    std::vector<Trajectory::vec_t> trajs(D);
    for(size_t d = 0; d < D; d++)
    {
        trajs[d] = sample(prior.priors_[d]);
    }

    Trajectory trajectory;
    trajectory.set_dimensions(prior.start(), trajs.begin(), trajs.end());

    return trajectory;
}

}} // namespace kjb::bbb

#endif /*B3_TRAJECTORY_PRIOR_H */

