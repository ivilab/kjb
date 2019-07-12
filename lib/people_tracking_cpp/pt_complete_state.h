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
   |  Author:  Ernesto Brau, Jinyan Guan
 * =========================================================================== */

/* $Id$ */

#ifndef PT_COMPLETE_STATE_H
#define PT_COMPLETE_STATE_H

#include <people_tracking_cpp/pt_visibility.h>
#include <camera_cpp/perspective_camera.h>
#include <m_cpp/m_vector.h>
#include <m_cpp/m_vector_d.h>
#include <m_cpp/m_matrix.h>
#include <detector_cpp/d_bbox.h>
#include <vector>
#include <limits>
#include <fstream>
#include <ios>
#include <iomanip>

#ifdef KJB_HAVE_TBB
#include <tbb/tbb.h>
#include <tbb/scalable_allocator.h>
#endif 

namespace kjb {
namespace pt {

// some typdefs
#ifdef KJB_HAVE_TBB
    typedef std::vector<Vector, tbb::scalable_allocator<Vector> > Vector_vec;
#else
    typedef std::vector<Vector> Vector_vec;
#endif

// forward declaration
class Target;

/**
 * @struct  Complete_state
 * @brief   Represents the state of an actor at a frame.
 */
struct Complete_state
{
    Vector3 position;
    double body_dir;
    Vector2 face_dir;
    const Target* attn;
    bool face_com;

    /** @brief  Construct an empty (and invalid) complete state. */
    Complete_state() :
        body_dir(std::numeric_limits<double>::max()),
        face_dir(std::numeric_limits<double>::max(),
                 std::numeric_limits<double>::max()),
        attn(0),
        face_com(false)
    {}

    /** @brief  Construct a (invalid) complete state with the given position. */
    Complete_state(const Vector3& pos) :
        position(pos),
        body_dir(std::numeric_limits<double>::max()),
        face_dir(std::numeric_limits<double>::max(),
                 std::numeric_limits<double>::max()),
        attn(0),
        face_com(false)
    {}

    /** @brief  Construct a complete state with the given parameters. */
    Complete_state
    (
        const Vector3& pos,
        double body_direction,
        const Vector2& face_directions
    ) :
        position(pos),
        body_dir(body_direction),
        face_dir(face_directions),
        attn(0),
        face_com(false)
    {}
};

/** @brief outputs the Complete_state. */
inline
std::ostream& operator<<(std::ostream& ost, const Complete_state& cs)
{
    ost << cs.position
        << "  " << std::setw(16) << std::setprecision(8) << cs.body_dir 
        << "  " << cs.face_dir; 
    return ost;
}

/** @brief  Gets the body direction from a direction vector. */
inline
double body_direction(const Vector& dir)
{
    if(dir.magnitude() < FLT_EPSILON) 
        return std::numeric_limits<double>::max();

    Vector2 dir2(dir[2], dir[0]);
    double ang = acos(dir2.normalized()[0]);

    if(dir2[1] < 0) ang = -ang;

    return ang;
}

/** @brief  Simple struct representing a head. */
struct Head
{
    double height;
    double width;
    double girth;
    Vector center;
    Matrix orientation;

    Head(const Complete_state& cs, double h, double w, double g) :
        height(h * HEIGHT_FRACTION),
        width(w * WIDTH_FRACTION),
        girth(g * GIRTH_FRACTION),
        center(cs.position[0], h - height/2, cs.position[2]),
        orientation(
            Matrix::create_3d_rotation_matrix(0.0, 0, 0, 1) *
            Matrix::create_3d_rotation_matrix(
                cs.face_dir[0]+cs.body_dir, 0, 1, 0) *
            Matrix::create_3d_rotation_matrix(cs.face_dir[1], 1, 0, 0))
        //orientation(
        //    cs.face_dir[1], cs.face_dir[0] + cs.body_dir, 0.0,
        //    Quaternion::XYZS)
    {}

    static const double HEIGHT_FRACTION;
    static const double WIDTH_FRACTION;
    static const double GIRTH_FRACTION;
};

/** @brief  Compute point on ellipse given body_dir and angle. */
inline
Vector ellipse_point
(
    const Complete_state& cs,
    double height,
    double width,
    double girth,
    double angle
)
{
    Matrix R(2, 2);
    R(0, 0) = cos(cs.body_dir);
    R(0, 1) = -sin(cs.body_dir);
    R(1, 0) = sin(cs.body_dir);
    R(1, 1) = cos(cs.body_dir);

    Vector p(0.5*girth*cos(angle), 0.5*width*sin(angle));
    p = R*p;

    return Vector(cs.position[0] + p[1], height, cs.position[2] + p[0]);
}

/** @brief  Computes points on ellipse given by position and dimensions. */
Vector_vec body_ellipse
(
    const Complete_state& cs,
    double h,
    double w,
    double g,
    size_t n = 6
);

/** @brief  Points on body cylinders. */
void cylinder_points
(
    const Complete_state& cs,
    double height,
    double width,
    double girth,
    Vector_vec& fpts,
    Vector_vec& bpts,
    Vector& botc,
    Vector& topc,
    Vector& bodc
);

/** @brief  Sample points on head surface. */
Vector_vec head_points
(
    const Complete_state& cs,
    double height,
    double width,
    double girth
);

/** @brief  Location of face features. */
Vector_vec face_features
(
    const Complete_state& cs,
    double height,
    double width,
    double girth
);

/** @brief  Find a point on face which is visible from the camera. */
Vector visible_head_point
(
    const Complete_state& cs,
    const Vector& cam_center,
    double height,
    double width,
    double girth
);

/** @brief  Converts a point on the head to world coordinates. */
inline
Vector head_to_world(const Vector& vh, const Head& h, bool scale = false)
{
    Vector vw = vh;
    if(scale)
    {
        vw[0] *= h.width/2;
        vw[1] *= h.height/2;
        vw[2] *= h.girth/2;
    }

    //vw = h.center + h.orientation.rotate(vw);
    vw = h.center + h.orientation*vw;

    return vw;
}

/** @brief  Determines whether an observer is looking at an point in space. */
bool looking
(
    const Complete_state& obs,
    double height,
    double width,
    double girth,
    const Vector& obj,
    double thresh,
    bool flt_obj
);

/** @brief  Find the intersection between two gazes, if it exists. */
Vector gaze_intersection
(
    const Complete_state& cs1,
    double height1,
    double width1,
    double girth1,
    const Complete_state& cs2,
    double height2,
    double width2,
    double girth2,
    double thresh
);

/** @brief  Return ture if the face dir is initialized.*/
inline
bool valid_face_dir(const Complete_state& cs)
{
    return cs.face_dir[0] != std::numeric_limits<double>::max() &&
           cs.face_dir[1] != std::numeric_limits<double>::max();
}

/** @brief  Return ture if the body dir is initialized.*/
inline
bool valid_body_dir(const Complete_state& cs)
{
    return cs.body_dir != std::numeric_limits<double>::max();
}

}} //namespace kjb::pt

#endif /*PT_COMPLETE_STATE_H */

