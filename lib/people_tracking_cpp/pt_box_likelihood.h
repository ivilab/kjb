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
|     Ernesto Brau
|
* =========================================================================== */

/* $Id: pt_box_likelihood.h 18657 2015-03-18 14:43:48Z ernesto $ */

#ifndef PT_BOX_LIKELIHOOD_H_
#define PT_BOX_LIKELIHOOD_H_

#include <people_tracking_cpp/pt_detection_box.h>
#include <people_tracking_cpp/pt_scene.h>
#include <people_tracking_cpp/pt_target.h>
#include <people_tracking_cpp/pt_body_2d.h>
#include <detector_cpp/d_bbox.h>
#include <cmath>
#include <string>

namespace kjb {
namespace pt {

/**
 * @brief   Class that represents likelihood of a set of projected boxes given.
 *          detections. At the moment it only uses box size and optical flow;
 *          in the future, it may be use other data information.
 */
class Box_likelihood
{
public:
    Box_likelihood
    (
        double noise_weight,
        double image_width,
        double image_height,
        const std::string& dist_name = std::string("laplace")
    ) :
        noise_weight_(noise_weight),
        image_width_(image_width),
        image_height_(image_height),
        m_dist_name(dist_name)
    {}

    /** @brief  Evaluate this log-likelihood on the given scene. */
    double operator()(const Scene& scene) const;

    /** @brief  Evaluate this log-likelihood on the given scene. */
    double at_noise(const Scene& scene) const;

    /** @brief  Evaluate this log-likelihood on the given target. */
    double at_target(const Target& target) const;

    /** @brief  Evaluate this log-likelihood on the given frame. */
    double at_frame(const Target& target, size_t frame) const;

    /** @brief  Evaluate this log-likelihood on the given frame. */
    double at_model(const Detection_box& dbox, const Body_2d& model) const;

    /** @brief  Computes line paramters given type of box. */
    void get_params
    (
        const std::string& type,
        Vector& params_x,
        Vector& params_top,
        Vector& params_bot
    ) const;

private:
    /** @brief  Evaluate this log-likelihood on the given frame. */
    double single_box(const Detection_box& dbox, const Bbox& mbox) const;

    /** @brief  Evaluate this log-likelihood of a false alarm box. */
    double single_noise(const Detection_box& /* dbox */) const
    {
        return noise_weight_ * (-std::log(image_width_)
                                    - std::log(image_height_)
                                    - std::log(image_height_));
    }

    /*
     * @brief   Helper function that computes PDF of value given line
     *          parameters.
     */
    double get_log_pdf
    (
        const Vector& params,
        const std::string& dist_name,
        double model_height,
        double value
    ) const;

private:
    double noise_weight_;
    double image_width_;
    double image_height_;
    std::string m_dist_name;
};

}} //namespace kjb::pt

#endif /*PT_BOX_LIKELIHOOD_H_ */

