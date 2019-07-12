#ifndef KJB_PSI_BBOX_H
#define KJB_PSI_BBOX_H


/**
 * @file Various utility functions for converting 3D structures into 2D bouding boxes.
 */
#include <m_cpp/m_vector.h>
#include <camera_cpp/perspective_camera.h>
#include <g_cpp/g_util.h>
#include <g_cpp/g_cylinder.h>
#include <i_cpp/i_color_histogram.h>

#ifdef KJB_HAVE_UA_CARTWHEEL
#include <MathLib/Capsule.h>
#include <Control/CapsuleState.h>
#endif

#include <vector>
#include <psi_cpp/psi_weighted_box.h>
#include <psi_cpp/psi_util.h>

namespace kjb {
namespace psi {

#ifdef KJB_HAVE_UA_CARTWHEEL
typedef CartWheel::Math::Capsule Cw_capsule;
#endif

#ifdef KJB_HAVE_UA_CARTWHEEL
std::vector<Bbox>
get_bounding_boxes(
        const CartWheel::CapsuleState& capsules,
        const kjb::Perspective_camera& cam);
#endif

std::vector<Bbox>
get_bounding_boxes(
        const std::vector<kjb::Cylinder>& cylinders,
        const kjb::Perspective_camera& cam);


#ifdef KJB_HAVE_UA_CARTWHEEL
std::vector<kjb::Vector> capsules_to_3d_points(const std::vector<Cw_capsule*>& capsules);
#endif

std::vector<kjb::Vector> cylinder_to_3d_points(const kjb::Cylinder& c, double scale_long = 1.0, double angle_short = 0.0);

inline std::vector<kjb::Vector> box_to_3d_points(const Cuboid& box)
{
    return get_corners(box);
}


Bbox get_bounding_box(
        const std::vector<kjb::Vector>& points_3d,
        const kjb::Perspective_camera& cam);

#ifdef KJB_HAVE_UA_CARTWHEEL
inline Bbox get_bounding_box(const std::vector<Cw_capsule*>& capsules, const kjb::Perspective_camera& cam)
{
    std::vector<kjb::Vector> points_3d = capsules_to_3d_points(capsules);

    return get_bounding_box(points_3d, cam);
}
#endif

inline Bbox get_bounding_box(const kjb::Cylinder& cylinder, const kjb::Perspective_camera& cam, double scale_long = 1.0, double angle_short = 0.0)
{
    std::vector<kjb::Vector> points_3d = cylinder_to_3d_points(cylinder, scale_long, angle_short);

    return get_bounding_box(points_3d, cam);
}

inline Bbox get_bounding_box(const Cuboid box, const kjb::Perspective_camera& cam)
{
    std::vector<kjb::Vector> points_3d = box_to_3d_points(box);

    return get_bounding_box(points_3d, cam);
}


inline kjb::Vector project_point(const kjb::Perspective_camera& cam, const kjb::Vector& x)
{
    return kjb::geometry::projective_to_euclidean_2d(cam.build_camera_matrix() * kjb::geometry::euclidean_to_projective(x));
}

kjb::Color_histogram compute_color_histogram_from_box(const Bbox & box, const kjb::Image & img, unsigned int num_bins);


} // namespace psi
} // namespace kjb
#endif /*KJB_PSI_BBOX_H */

