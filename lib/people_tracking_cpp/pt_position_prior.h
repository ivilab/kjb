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

/* $Id: pt_position_prior.h 19963 2015-10-27 05:33:01Z jguan1 $ */

#ifndef PT_POSITION_PRIOR_H_
#define PT_POSITION_PRIOR_H_

#include <people_tracking_cpp/pt_scene.h>
#include <people_tracking_cpp/pt_target.h>
#include <gp_cpp/gp_prior.h>
#include <gp_cpp/gp_normal.h>

namespace kjb {
namespace pt {

/**
 * @brief   Class that represents the prior distribution of a trajectory.
 */
class Position_prior
{
private:
    typedef gp::Prior<gp::Zero, gp::Squared_exponential> Gpp;

public:
    /** @brief  Construct a prior. */
    Position_prior(double sc, double sv, size_t local_sz) :
        gpsc_(sc),
        gpsv_(sv),
        local_dist_(
            gp::Zero(),
            gp::Sqex(gpsc_, gpsv_),
            gp::Inputs::const_iterator(),
            gp::Inputs::const_iterator()),
        //sweight_(100.0),
        sweight_(1000.0),
        m_local_sz(local_sz)
    {
        if(m_local_sz % 2 == 0)
        {
            m_local_sz++;
        }

        if(m_local_sz <= 1)
        {
            m_local_sz = 3;
        }
    }

    /** @brief  Construct a prior. */
    Position_prior(double sc, double sv) :
        gpsc_(sc),
        gpsv_(sv),
        local_dist_(
            gp::Zero(),
            gp::Sqex(gpsc_, gpsv_),
            gp::Inputs::const_iterator(),
            gp::Inputs::const_iterator()),
        //sweight_(100.0),
        sweight_(1000.0),
        m_local_sz(8*gpsc_)
    {
        if(m_local_sz % 2 == 0)
        {
            m_local_sz++;
        }

        if(m_local_sz <= 1)
        {
            m_local_sz = 3;
        }
    }

    /** @brief  Evaluate this prior on the given scene. */
    double operator()(const Scene& scene) const;

    /** @brief  Evaluate prior on space occupation. */
    double at_space(const Scene& scene) const;

    /** @brief  Evaluate prior on starting/ending in the middle of the image. */
    double at_endpoints(const Scene& scene) const;

    /** @brief  Evaluate this prior on the given trajectory. */
    double at_trajectory(const Target& target) const;

    /** @brief  Approximate this prior around a frame. */
    double local(const Target& target, size_t t) const;

    /** @brief  Approximate this space prior around a frame. */
    double local_space(const Scene& scene, const Target& target, size_t t) const;

    /** @brief  Approximate this start/end prior around a frame. */
    double local_endpoints
    (
        const Scene& scene,
        const Target& target,
        size_t t
    ) const;

    /** @brief  Return the local window size. */
    size_t local_size() const { return m_local_sz; }

    /** @brief  Return the GP scale. */
    double scale() const { return gpsc_; }

    /** @brief  Return the GP signal variance. */
    double signal_variance() const { return gpsv_; }

private:
    double gpsc_;
    double gpsv_;
    mutable Gpp local_dist_;
    double sweight_;
    size_t m_local_sz;
};

}} //namespace kjb::pt

#endif /*PT_POSITION_PRIOR_H_ */

