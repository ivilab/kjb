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

/* $Id$ */

#ifndef PT_CAMERA_PRIOR_H_
#define PT_CAMERA_PRIOR_H_

#include <camera_cpp/perspective_camera.h>
#include <prob_cpp/prob_distribution.h>
#include <prob_cpp/prob_pdf.h>

namespace kjb {
namespace pt {

/** @brief  Class that represents the camera prior. */
class Camera_prior
{
public:
    Camera_prior
    (
        double m_h,
        double s_h,
        double m_p,
        double s_p,
        double m_f,
        double s_f
    ) :
        N_h(m_h, s_h),
        N_p(m_p, s_p),
        N_f(m_f, s_f)
    {}

    double operator()(const Perspective_camera& camera) const
    {
        return log_pdf(N_h, camera.get_camera_centre()[1])
                + log_pdf(N_p, camera.get_pitch())
                + log_pdf(N_f, camera.get_focal_length());
    }

    const Normal_distribution& height_prior() const { return N_h; }
    const Normal_distribution& pitch_prior() const { return N_p; }
    const Normal_distribution& focal_length_prior() const { return N_f; }

private:
    Normal_distribution N_h;
    Normal_distribution N_p;
    Normal_distribution N_f;
};

}} //namespace kjb::pt

#endif /*PT_CAMERA_PRIOR_H_ */

