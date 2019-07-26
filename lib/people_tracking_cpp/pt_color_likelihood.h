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

/* $Id: pt_color_likelihood.h 18657 2015-03-18 14:43:48Z ernesto $ */

#ifndef PT_COLOR_LIKELIHOOD_H_
#define PT_COLOR_LIKELIHOOD_H_

#include <people_tracking_cpp/pt_scene.h>
#include <people_tracking_cpp/pt_visibility.h>
#include <i_cpp/i_filter.h>
#include <m_cpp/m_vector.h>
#include <m_cpp/m_matrix.h>
#include <detector_cpp/d_bbox.h>
#include <vector>
#include <string>
#include <utility>
#include <boost/shared_array.hpp>

namespace kjb {
namespace pt {

/**
 * @brief   Class that represents color likelihood of a set of projected boxes
 *          given
 *          detections.
 */
class Color_likelihood
{
public:
    // Ctor
    Color_likelihood
    (
        size_t pixel_sampling = 1,
        size_t frame_sampling = 1
    ) : m_bg_r_p(NULL),
        m_bg_g_p(NULL),
        m_pixel_ssz(pixel_sampling),
        m_frame_ssz(frame_sampling)
    {}

    /** @brief  Evaluate this likelihood at the given scene. */
    double operator()(const Scene& scene) const;

    /** @brief  Evaluate likelihood at boxes of a trajectory. */
    double at_trajectory(const Target& target) const;

    /** @brief  Evaluate likelihood at a single box. */
    double at_box
    (
        const Body_2d& cur_b2d,
        const Body_2d& next_b2d,
        size_t cur_frame,
        size_t next_frame
    ) const;

    void read_frames(const std::vector<std::string>& frame_fps);

    void set_bg_r_matrix(const Matrix* bg_r_p)
    {
        m_bg_r_p = bg_r_p;
    }

    void set_bg_g_matrix(const Matrix* bg_g_p)
    {
        m_bg_g_p = bg_g_p;
    }

private:
    std::vector<Vector> get_foreground
    (
        size_t frame,
        const Bbox& box
    ) const;

    std::vector<Vector> get_background
    (
        const Bbox& box
    ) const;

    Matrix get_norm_hist_as_matrix
    (
        const Vector& loc,
        double x_delta,
        double y_delta,
        size_t frame,
        const std::pair<double, double>& range_1,
        const std::pair<double, double>& range_2,
        size_t num_bins,
        const Filter& filter,
        bool is_foreground
    ) const;

private:
    const Matrix* m_bg_r_p;
    const Matrix* m_bg_g_p;
    std::vector<boost::shared_array<float> > m_r_pixels;
    std::vector<boost::shared_array<float> > m_g_pixels;
    size_t m_pixel_ssz;
    size_t m_frame_ssz;
    size_t m_width;
    size_t m_height;

};

}} //namespace kjb::pt

#endif /* PT_COLOR_LIKELIHOOD_H_ */

