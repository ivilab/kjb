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
| Authors:
|     Jinyan Guan, Ernesto Brau
|
* =========================================================================== */

/* $Id: pt_optical_flow_likelihood.h 18178 2014-11-11 18:26:57Z ernesto $ */

#ifndef PT_OPTICAL_FLOW_LIKELIHOOD_H
#define PT_OPTICAL_FLOW_LIKELIHOOD_H

#include <flow_cpp/flow_integral_flow.h>
#include <people_tracking_cpp/pt_body_2d.h>
#include <people_tracking_cpp/pt_scene.h>
#include <people_tracking_cpp/pt_target.h>
#include <people_tracking_cpp/pt_visibility.h>
#include <prob_cpp/prob_distribution.h>
#include <vector>

namespace kjb {
namespace pt {

/**
 * @brief   Class to compute face optical flow likelihood.
 *
 * @warn    Features are not copied, so they must remain in scope while this
 *          class is alive.
 */
class Optical_flow_likelihood
{
public:
    /** @brief  Construct an OF likelihood with the given parameters. */
    Optical_flow_likelihood
    (
        const std::vector<Integral_flow>& flows_x,
        const std::vector<Integral_flow>& flows_y,
        double img_width,
        double img_height,
        double x_scale,
        double y_scale,
        double bg_x_scale,
        double bg_y_scale
    ) :
        m_flows_x(flows_x),
        m_flows_y(flows_y),
        m_width(img_width),
        m_height(img_height),
        m_x_dist(0.0, x_scale),
        m_y_dist(0.0, y_scale),
        m_bg_x_dist(0.0, bg_x_scale),
        m_bg_y_dist(0.0, bg_y_scale)
    {}

    /** @brief  Evaluate this likelihood at the given scene. */
    double operator()(const Scene& scene) const;

    /** @brief  Evaluate likelihood at boxes of a trajectory. */
    double at_trajectory(const Target& target) const;

    /** @brief  Evaluate likelihood at a single box. */
    double at_box(const Body_2d& b2d, size_t cur_frame) const; 

    /** @brief  Return the individual Laplace distribution for x. */
    const Laplace_distribution& x_dist() const { return m_x_dist; }

    /** @brief  Return the individual Laplace distribution for y. */
    const Laplace_distribution& y_dist() const { return m_y_dist; }

    /** @brief  Return the individual Laplace distribution for x. */
    const Laplace_distribution& bg_x_dist() const { return m_bg_x_dist; }

    /** @brief  Return the individual Laplace distribution for y. */
    const Laplace_distribution& bg_y_dist() const { return m_bg_y_dist; }

private:
    // data members
    const std::vector<Integral_flow>& m_flows_x;
    const std::vector<Integral_flow>& m_flows_y;
    double m_width;
    double m_height;
    Laplace_distribution m_x_dist;
    Laplace_distribution m_y_dist;
    Laplace_distribution m_bg_x_dist;
    Laplace_distribution m_bg_y_dist;
};

}} // namespace kjb::pt

#endif /*PT_OPTICAL_FLOW_LIKELIHOOD_H */

