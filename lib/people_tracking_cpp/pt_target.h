/* =========================================================================== *
|
|  Copyright (c) 1994-2008 by Kobus Barnard (author).
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
* =========================================================================== */

/* $Id: pt_target.h 21107 2017-01-23 05:45:47Z ernesto $ */

#ifndef PT_TARGET_H_
#define PT_TARGET_H_

#include <people_tracking_cpp/pt_detection_box.h>
#include <people_tracking_cpp/pt_complete_trajectory.h>
#include <people_tracking_cpp/pt_complete_state.h>
#include <people_tracking_cpp/pt_body_2d_trajectory.h>
#include <people_tracking_cpp/pt_face_2d_trajectory.h>
#include <people_tracking_cpp/pt_data.h>
#include <m_cpp/m_vector.h>
#include <mcmcda_cpp/mcmcda_track.h>
#include <camera_cpp/perspective_camera.h>
#include <gp_cpp/gp_base.h>
#include <gp_cpp/gp_mean.h>
#include <gp_cpp/gp_covariance.h>
#include <gp_cpp/gp_prior.h>
#include <gp_cpp/gp_normal.h>
#include <prob_cpp/prob_distribution.h>
#include <flow_cpp/flow_integral_flow.h>
#include <boost/optional.hpp>

namespace kjb {
namespace pt {

// forward declarations
class Scene;

/**
 * @brief   Class that represents a target moving through space.
 */
class Target : public mcmcda::Generic_track<Detection_box>
{
public:
    typedef gp::Prior<gp::Zero, gp::Squared_exponential> Gpp;

public:
    typedef Detection_box Element;

    /** @brief  Construct a target with the given height. */
    Target(double height, double width, double girth, size_t vlen) :
        traj(vlen, height, width, girth),
        btraj(vlen, height, width, girth),
        ftraj(vlen, height, width, girth),
        gp_pos_prior(
            gp::Zero(),
            gp::Sqex(40.0, 100.0),
            gp::Inputs::const_iterator(),
            gp::Inputs::const_iterator()),
        gp_dir_prior(
            gp::Zero(),
            gp::Sqex(80.0, 30),
            gp::Inputs::const_iterator(),
            gp::Inputs::const_iterator()),
        gp_fdir_prior(
            gp::Zero(),
            gp::Sqex(80.0, M_PI*M_PI/16.0),
            gp::Inputs::const_iterator(),
            gp::Inputs::const_iterator())
    {
        IFT(height > 0 && width > 0 && girth > 0, Illegal_argument,
            "Target dimensions must all be positive.");
    }

public:
    /** @brief  Get this target's current 3D positions. */
    Trajectory& trajectory() const
    {
        return traj;
    }

    /** @brief  Get this target's walking angles. */
    Angle_trajectory& wangle_trajectory() const
    {
        return atraj;
    }

    /** @brief  Get this target's current 2D body trajectory. */
    Body_2d_trajectory& body_trajectory() const
    {
        return btraj;
    }

    /** @brief  Get this target's current 2D body trajectory. */
    Face_2d_trajectory& face_trajectory() const
    {
        return ftraj;
    }

    /** @brief  Returns the prior distribution used to evaluate this target. */
    const Gpp& pos_prior() const
    {
        return gp_pos_prior;
    }

    /** @brief  Returns the prior distribution used to evaluate this target. */
    const Gpp& dir_prior() const
    {
        return gp_dir_prior;
    }

    /** @brief  Returns the prior distribution used to evaluate this target. */
    const Gpp& fdir_prior() const
    {
        return gp_fdir_prior;
    }

public:
    /** @brief  Set changed flags to unchanged. */
    void set_unchanged() const
    {
        set_changed_start(-1);
        set_changed_end(-1);
    }

//  This is now defined in the parent class Generic_track
//  so this class just inherits from it
//    /** @brief  Set changed flags to unchanged. */
//    void set_changed_all() const
//    {
//        set_changed_start(get_start_time());
//        set_changed_end(get_end_time());
//    }
//
    /** @brief  Return true if this target has changed. */
    bool changed() const
    {
        if(changed_start() == -1 || changed_end() == -1)
        {
            assert(changed_start() == -1 && changed_end() == -1);
            return false;
        }

        assert(changed_start() >= get_start_time()
                && changed_end() <= get_end_time());
        return true;
    }

private:
    /**
     * @brief   Smooth a variable of this trajectory at the given interval
     *
     * @param   sf      Frame where to start smoothing
     * @param   ef      Frame where to end smoothing
     * @param   nvar    Noise variance at each point in [sf, ef]
     * @param   scale   GP length scale for smoothing
     * @param   svar    GP signal variance for smoothing
     * @param   data    Function which receives a frame and retuns the
     *                  data (as an optional) at that frame.
     * @param   mean    Function which receives a frame and retuns the
     *                  mean at that frame.
     */

    template<class GetTraj, class GetMean>
    Vector smooth_trajectory
    (
        size_t sf,
        size_t ef,
        const std::vector<double>& nvar,
        double scale,
        double svar,
        const GetTraj& data,
        const GetMean& mean
    ) const;

    /**
     * @brief   Return the angle that is different than prev less than PI,
     *          and is cur +/- 2n pi
     */
    double closest_angle(double prev, double cur) const;

public:
    /** @brief  Fix trajectory to match track. */
    void estimate_trajectory
    (
        const Perspective_camera& cam,
        double nsx = 0.01,
        double nsz = 1.0
    ) const;

    /** @brief  Fix trajectory to match track. */
    void estimate_trajectory
    (
        const Perspective_camera& cam,
        const std::vector<Integral_flow>& x_flows,
        const std::vector<Integral_flow>& y_flows,
        const std::vector<Integral_flow>& back_x_flows,
        const std::vector<Integral_flow>& back_y_flows,
        size_t frame_rate,
        double nsx = 0.01,
        double nsz = 1.0
    ) const;

    /** @brief  Estimate height from boxes and camera. */
    void estimate_height(const Perspective_camera& cam) const;

    /** @brief  Estimate directions. */
    void estimate_directions(bool infer_head = true, double nsb = 0.1) const;

public:
    /** @brief  Updates internal distribution and caching mechanism. */
    void update_pos_gp(double sc, double sv) const
    {
        update_gp(gp_pos_prior, sc, sv);
    }

    /** @brief  Updates internal distribution and caching mechanism. */
    void update_dir_gp(double sc, double sv) const
    {
        update_gp(gp_dir_prior, sc, sv);
    }

    /** @brief  Updates internal distribution and caching mechanism. */
    void update_fdir_gp(double sc, double sv) const
    {
        update_gp(gp_fdir_prior, sc, sv);
    }

    /** @brief  Update box trajectory. */
    void update_boxes(const Perspective_camera& cam) const;

    /** @brief  Update face trajectory. */
    void update_faces(const Perspective_camera& cam) const;

    /** @brief  Update the walking angles. */
    void update_walking_angles() const;

    /** @brief  Updates internal distribution and caching mechanism. */
    Vector& hessian_diagonal() const
    {
        return hessian_;
    }

public:
    /**
     * @brief   Efficiently swap two targets.
     */
    friend
    void swap(Target& t1, Target& t2)
    {
        using std::swap;

        swap(t1.traj, t2.traj);
        swap(t1.atraj, t2.atraj);
        swap(t1.btraj, t2.btraj);
        swap(t1.ftraj, t2.ftraj);
        swap(t1.gp_pos_prior, t2.gp_pos_prior);
        swap(t1.gp_dir_prior, t2.gp_dir_prior);
        swap(t1.gp_fdir_prior, t2.gp_fdir_prior);
        swap(t1.hessian_, t2.hessian_);

        mcmcda::Generic_track<Detection_box>& m1 = t1;
        mcmcda::Generic_track<Detection_box>& m2 = t2;
        swap(m1, m2);
    }

    //friend
    //void update_scene_state(const Scene& scene, const Facemark_data& fmdata);

private:
    void update_gp(Gpp& gpp, double sc, double sv) const;

    ////////////////////////////////////////////////////////////////////
    // get_XXX() are helper functions used to get a generic smoothing //
    ////////////////////////////////////////////////////////////////////

    boost::optional<double> get_position(size_t fr, size_t dim) const
    {
        if(traj[fr - 1]) return traj[fr - 1]->value.position[dim];
        return boost::none;
    }

    boost::optional<double> get_bdir(size_t fr) const
    {
        if(traj[fr - 1]
            && traj[fr - 1]->value.body_dir != std::numeric_limits<double>::max())
        {
            return traj[fr - 1]->value.body_dir;
        }

        return boost::none;
    }

    boost::optional<double> get_fdir(size_t fr, size_t dim) const
    {
        assert(dim <= 1);
        if(traj[fr - 1]
            && traj[fr - 1]->value.face_dir[dim] != std::numeric_limits<double>::max())
        {
            return traj[fr - 1]->value.face_dir[dim];
        }

        return boost::none;
    }

    double get_zero(size_t) const
    {
        return 0.0;
    }

    double get_walking_angle(size_t fr) const
    {
        IFT(fr >= get_start_time() && fr <= get_end_time(),
                Illegal_argument,
                "frame out of bound in get_walking_angle");
        return atraj[fr - 1]->value;
    }

private:
    mutable Trajectory traj;
    mutable Body_2d_trajectory btraj;
    mutable Face_2d_trajectory ftraj;
    mutable Angle_trajectory atraj;
    mutable Gpp gp_pos_prior;
    mutable Gpp gp_dir_prior;
    mutable Gpp gp_fdir_prior;

    // I HATE THIS BUT IT'S NECESSARY!!
    mutable Vector hessian_;
};

/** @brief  Helper function -- gets the position from CS as a kjb::Vector. */
inline
Vector get_cs_position(const Complete_state& cs)
{
    //return Vector().set(cs.position[0], cs.position[2]);
    return Vector(cs.position[0], cs.position[2]);
}

/** @brief  Helper function -- gets the face_dir from CS as a kjb::Vector. */
inline
Vector get_cs_face_dir(const Complete_state& cs)
{
    //return Vector().set(cs.face_dir[0], cs.face_dir[1]);
    return Vector(cs.face_dir[0], cs.face_dir[1]);
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

/** @brief  Sync targets trajectory with endpoints. */
void sync_state(const Target& tg);

/**
 * @brief   Smooth a variable of this trajectory at the given interval
 *
 * @param   sf      Frame where to start smoothing
 * @param   ef      Frame where to end smoothing
 * @param   nvar    Noise variance at each point in [sf, ef]
 * @param   scale   GP length scale for smoothing
 * @param   svar    GP signal variance for smoothing
 * @param   data    Function which receives a frame and retuns the
 *                  data (as an optional) at that frame.
 * @param   mean    Function which receives a frame and retuns the
 *                  mean at that frame.
 */
/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

template<class GetTraj, class GetMean>
Vector Target::smooth_trajectory
(
    size_t sf,
    size_t ef,
    const std::vector<double>& nvar,
    double scale,
    double svar,
    const GetTraj& data,
    const GetMean& mean
) const
{
    typedef gp::Predictive<
                    gp::Real_function_adapter<size_t>,
                    gp::Sqex,
                    gp::Linear_gaussian> Gppred;

    IFT(nvar.size() <= ef - sf + 1, Illegal_argument,
        "Cannot smooth trajectory; wrong number of variances.");

    // fill buffer variances with 0.0, to ensure smoothness at the edges
    size_t wsf = sf - get_start_time() > scale ? sf - scale : get_start_time();
    size_t wef = std::min(get_end_time(), static_cast<int>(ef + scale));

    size_t nssz = nvar.size() + (sf - wsf) + (wef - ef);
    Vector ns(static_cast<int>(nssz), 0.0);
    assert(ns.size() >= nvar.size());
    std::copy(nvar.begin(), nvar.end(), ns.begin() + sf - wsf);

    // fill in data
    gp::Inputs trins; trins.reserve(ns.size());
    Vector trouts; trouts.reserve(ns.size());
    for(size_t t = wsf; t <= wef; ++t)
    {
        boost::optional<double> dp = data(t);

        if(dp)
        {
            trins.push_back(Vector().set(t));
            trouts.push_back(*dp);
        }
    }

    IFT(ns.size() == trins.size(), Illegal_argument,
        "Cannot smooth trajectory; wrong number of variances.");

    if(trins.empty()) return Vector();

    // and test inputs and GP posterior
    gp::Inputs teins = gp::make_inputs(sf, ef);
    Gppred gp_pred = make_predictive(
                            gp::Real_function_adapter<size_t>(mean),
                            gp::Sqex(scale, svar),
                            create_diagonal_matrix(ns),
                            trins, trouts, teins);

    // return values
    return gp_pred.normal().get_mean();
}

inline
size_t dims
(
    const Target& tg,
    bool respect_changed = true,
    bool infer_head = true
)
{
    size_t sf, ef;
    if(!respect_changed)
    {
        sf  = tg.get_start_time();
        ef = tg.get_end_time();
    }
    else if(tg.changed())
    {
        sf = tg.changed_start();
        ef = tg.changed_end();
    }
    else
    {
        return 0;
    }

    return infer_head ? 5*(ef - sf + 1) : 2*(ef - sf + 1);
}

}} //namespace kjb::pt

#endif /*PT_TARGET_H_ */

