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
|     Jinyan Guan
|
* =========================================================================== */

/* $Id: pt_facemark_likelihood.h 19393 2015-06-05 16:49:17Z ernesto $ */

#ifndef PT_FACEMARK_LIKELIHOOD_H_
#define PT_FACEMARK_LIKELIHOOD_H_

#include <people_tracking_cpp/pt_face_2d.h>
#include <people_tracking_cpp/pt_scene.h>
#include <people_tracking_cpp/pt_target.h>
#include <m_cpp/m_vector.h>
#include <prob_cpp/prob_distribution.h>
#include <l_cpp/l_util.h>

namespace kjb {
namespace pt {

/**
 * @brief   Class to compute facemark likelihood.
 */
class Facemark_likelihood
{
public:
    /** @brief  Construct an Facemark_likelihood with the given parameters. */
    Facemark_likelihood
    (
        const Facemark_data& fm_data,
        double eye_x_sigma,
        double eye_y_sigma,
        double nose_x_sigma,
        double nose_y_sigma,
        double mouth_x_sigma,
        double mouth_y_sigma,
        double img_width,
        double img_height
    ) :
        m_fm_data_p(&fm_data),
        m_eye_x_dist(0.0, eye_x_sigma),
        m_eye_y_dist(0.0, eye_y_sigma),
        m_nose_x_dist(0.0, nose_x_sigma),
        m_nose_y_dist(0.0, nose_y_sigma),
        m_mouth_x_dist(0.0, mouth_x_sigma),
        m_mouth_y_dist(0.0, mouth_y_sigma),
        m_img_width(img_width),
        m_img_height(img_height)
    {}

    /** @brief  Construct an Facemark_likelihood with the given parameters. */
    Facemark_likelihood
    (
        const Facemark_data& fm_data,
        double sigma,
        double img_width,
        double img_height
    ) :
        m_fm_data_p(&fm_data),
        m_eye_x_dist(0.0, sigma),
        m_eye_y_dist(0.0, sigma),
        m_nose_x_dist(0.0, sigma),
        m_nose_y_dist(0.0, sigma),
        m_mouth_x_dist(0.0, sigma),
        m_mouth_y_dist(0.0, sigma),
        m_img_width(img_width),
        m_img_height(img_height)
    {}

    /** @brief  Evaluate this likelihood at the given scene. */
    double operator()(const Scene& scene) const;

    /** @brief  Evaluate the noise likelihood at the given scene. */
    double at_noise(const Scene& scene) const
    {
        size_t nnfaces = num_facemarks() - num_assigned_facemarks(scene);
        return 5 * nnfaces * single_noise();
    }

    /** @brief  Evaluate likelihood at boxes of a trajectory. */
    double at_trajectory(const Target& target) const;

    /** @brief  Evaluate likelihood at a single box. */
    double at_face(const Face_2d& face) const;

    double single_noise() const
    {
        return -log(m_img_width) - log(m_img_height);
    }

    /** @brief  Returns the number of assigned faces. */
    size_t num_assigned_facemarks(const Scene& scene) const;

    /** @brief  Returns the total number of faces. */
    size_t num_facemarks() const { return length(*m_fm_data_p); }

    const Facemark_data& data() const { return *m_fm_data_p; }

    const Normal_distribution& eye_x_dist() const { return m_eye_x_dist; }
    const Normal_distribution& eye_y_dist() const { return m_eye_y_dist; }
    const Normal_distribution& nose_x_dist() const { return m_nose_x_dist; }
    const Normal_distribution& nose_y_dist() const { return m_nose_y_dist; }
    const Normal_distribution& mouth_x_dist() const { return m_mouth_x_dist; }
    const Normal_distribution& mouth_y_dist() const { return m_mouth_y_dist; }

private:
    double at_mark
    (
        const Vector& dmark,
        const Vector& fmark,
        const Normal_distribution& P_x,
        const Normal_distribution& P_y
    ) const;


private:
    // data members
    const Facemark_data* m_fm_data_p;
    Normal_distribution m_eye_x_dist;
    Normal_distribution m_eye_y_dist;
    Normal_distribution m_nose_x_dist;
    Normal_distribution m_nose_y_dist;
    Normal_distribution m_mouth_x_dist;
    Normal_distribution m_mouth_y_dist;
    double m_img_width;
    double m_img_height;
};

}} //namespace kjb::pt

#endif /*PT_FACEMARK_LIKELIHOOD_H_ */

