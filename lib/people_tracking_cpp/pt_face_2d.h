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

#ifndef PT_FACE_2D_H
#define PT_FACE_2D_H

#include <detector_cpp/d_bbox.h>
#include <detector_cpp/d_deva_facemark.h>
#include <people_tracking_cpp/pt_visibility.h>
#include <m_cpp/m_vector.h>
#include <people_tracking_cpp/pt_complete_state.h>
#include <camera_cpp/perspective_camera.h>

#include <boost/optional.hpp>

namespace kjb {
namespace pt {

/** @brief  2D face information resulting from projecting the 3D head/face. */
struct Face_2d
{
    Bbox bbox;
    Vector left_eye;
    Vector right_eye;
    Vector nose;
    Vector left_mouth;
    Vector right_mouth;
    Vector model_dir;
    Visibility visibility;
    const Deva_facemark* facemark;

    Face_2d() {}

    Face_2d
    (
        const Bbox& box,
        const Vector& leye,
        const Vector& reye,
        const Vector& cnose,
        const Vector& lmouth,
        const Vector& rmouth,
        const Vector& mdir = Vector()
    ) :
        bbox(box),
        left_eye(leye),
        right_eye(reye),
        nose(cnose),
        left_mouth(lmouth),
        right_mouth(rmouth),
        model_dir(mdir),
        facemark(NULL)
    {}
};

/** @brief  Project a complete state into a face box. */
Face_2d project_cstate_face
(
    const Complete_state& cs,
    const Perspective_camera& cam,
    double height,
    double width,
    double girth
);

/** @brief  Compute the (2D) model direction for the face.. */
Vector face_model_direction
(
    const Complete_state& cs1,
    const Complete_state& cs2,
    const Perspective_camera& cam,
    double height,
    double width,
    double girth
);

/**
 * @brief   Returns the distance between a face_2d model 
 *          and a detection facemark.
 */
double face_distance
(
    const Face_2d& face_model,
    const Deva_facemark facemark
);

}} //namespace kjb::pt

#endif /*PT_FACE_2D_H */

