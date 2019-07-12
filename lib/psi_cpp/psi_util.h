/* $Id: psi_util.h 21596 2017-07-30 23:33:36Z kobus $ */
/* {{{=========================================================================== *
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
   |  Author:  Kyle Simek
 * =========================================================================== }}}*/

// vim: tabstop=4 shiftwidth=4 foldmethod=marker
#ifndef PSI_V1_UTIL
#define PSI_V1_UTIL

#include "l/l_sys_debug.h"  /* For ASSERT */
#include "m_cpp/m_vector.h"
#include "gr_cpp/gr_2D_bounding_box.h"
#include "camera_cpp/perspective_camera.h"
#include "g_cpp/g_quaternion.h"
#include "detector_cpp/d_deva_detection.h"
#include "people_tracking_cpp/pt_util.h"

#include "l/l_word_list.h"
#include "l/l_sys_io.h" /* for kjb_glob */

#ifdef KJB_HAVE_UA_CARTWHEEL
#include <MathLib/Point3d.h>
#include <MathLib/Vector3d.h>
#include <Core/Visualization.h>
#endif

namespace kjb
{
namespace psi
{

typedef kjb::Axis_aligned_rectangle_2d Bbox;

#ifdef KJB_HAVE_UA_CARTWHEEL
inline kjb::Vector to_kjb(const CartWheel::Math::Point3d& pt)
{
    return kjb::Vector(pt.x, pt.y, pt.z);
}

inline kjb::Vector to_kjb(const CartWheel::Math::Vector3d& vec)
{
    return kjb::Vector(vec.x, vec.y, vec.z);
}

inline kjb::Quaternion to_kjb(const CartWheel::Math::Quaternion& q)
{
    CartWheel::Math::Vector3d imag = q.getV();
    double real = q.getS();
    return kjb::Quaternion(imag.x, imag.y, imag.z, real);
}
#endif

#ifdef KJB_HAVE_UA_CARTWHEEL
inline CartWheel::Math::Point3d to_cw_pt_3d(const kjb::Vector& pt)
{
    ASSERT(pt.size() == 3);
    return CartWheel::Math::Point3d(pt[0], pt[1], pt[2]);

}
#endif

#ifdef KJB_HAVE_UA_CARTWHEEL
inline CartWheel::Math::Vector3d to_cw_vec_3d(const kjb::Vector& vec)
{
    ASSERT(vec.size() == 3);
    return CartWheel::Math::Vector3d(vec[0], vec[1], vec[2]);

}
#endif

#ifdef KJB_HAVE_UA_CARTWHEEL
/**
 * Pass a kjb perspective camera to a cartwheel visualization context
 *
 * @note Camera focal length is defined in units of screen pixels,
 * so we need screen size to convert it to fovy angle.
 */
void set_camera(CartWheel::Visualization& vis, const kjb::Perspective_camera& cam, double WIDTH, double HEIGHT);
#endif

/** /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ **/
inline kjb::Perspective_camera make_camera(
        double height,
        double tilt,
        double focal_length)
{
    kjb::Perspective_camera cam(0.1, 100);
    cam.set_camera_centre(kjb::Vector(0.0,height,0.0));
    cam.set_orientation(kjb::Quaternion(tilt, 0.0,0.0, kjb::Quaternion::XYZR));
    cam.set_focal_length(focal_length);

    return cam;
}

/**
 * @brief Project the 3D point x to image coordinate using camera matrix P
 *        and make the top left as the origin
 */
inline
Vector project_and_unstandarize(const Vector& x, const Matrix& P, double w, double h)
{
    using namespace geometry;
    Vector x2 = projective_to_euclidean_2d(P * euclidean_to_projective(x));
    pt::unstandardize(x2, w, h);
    return x2;
}


enum Simulator_type {CARTWHEEL_SIMULATOR, CYLINDER_SIMULATOR};

const std::string& get_name(Simulator_type type);

std::istream& operator>>(std::istream& ist, Simulator_type& type);
std::ostream& operator<<(std::ostream& ost, Simulator_type type);

/** @brief move coordinate ssystem origin to center of image  */
void standardize(Deva_detection& boxes, double cam_width, double cam_height);

/**
 * @brief   Prune the deva boxes based on the average entity height 
 */
void prune_by_height
(
    std::vector<Deva_detection>& deva_boxes,
    double screen_width,
    double screen_height,
    const Perspective_camera& camera, 
    double avereage_height
);

} // namespace psi
} // namespace kjb
#endif
