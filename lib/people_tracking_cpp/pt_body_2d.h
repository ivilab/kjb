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

#ifndef PT_BODY_2D_H
#define PT_BODY_2D_H

#include <people_tracking_cpp/pt_complete_state.h>
#include <people_tracking_cpp/pt_visibility.h>
#include <detector_cpp/d_bbox.h>
#include <camera_cpp/perspective_camera.h>
#include <m_cpp/m_vector.h>
#include <vector>

namespace kjb {
namespace pt {

/** @brief  2D body information resulting from projecting the 3D body. */
struct Body_2d
{
    Bbox full_bbox;
    Bbox body_bbox;
    Visibility visibility;
    Vector model_dir;

    Body_2d() {}

    Body_2d
    (
        const Bbox& full_box,
        const Bbox& body_box,
        const Vector& mdir = Vector()
    ) :
        full_bbox(full_box), body_bbox(body_box), model_dir(mdir)
    {}
};

/** @brief  Project a complete state into a body box. */
Body_2d project_cstate
(
    const Complete_state& cs,
    const Perspective_camera& cam,
    double height,
    double width,
    double girth
);

/** @brief  Compute the (2D) model direction for the body. */
Vector model_direction
(
    const Complete_state& cs1,
    const Complete_state& cs2,
    const Perspective_camera& cam,
    double height,
    double width,
    double girth
);

/** @brief  Computes the cylinder of a bounding box given 2D points. */
Bbox cylinder_bounding_box
(
    const Vector_vec& pts,
    const Vector& bot,
    const Vector& top
);

}} //namespace kjb::pt

#endif /*PT_BODY_2D_H */

