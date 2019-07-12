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
   |  Author: Jinyan Guan 
 * =========================================================================== */

/* $Id: psi_face_util.h 18278 2014-11-25 01:42:10Z ksimek $ */

#ifndef PSI_FACE_UTIL_H
#define PSI_FACE_UTIL_H

#include <i_cpp/i_image.h>
#include <m_cpp/m_matrix.h>
#include <m_cpp/m_vector.h>
#include <gr_cpp/gr_2D_bounding_box.h>
#include <people_tracking_cpp/pt_entity.h>
#include <people_tracking_cpp/pt_box_trajectory.h>
#include <people_tracking_cpp/pt_util.h>
//#include <psi_cpp/psi_skeleton_trajectory.h>
#include <camera_cpp/perspective_camera.h>
#include <detector_cpp/d_facecom.h>

namespace kjb
{
namespace psi
{

//typedef Axis_aligned_rectangle_2d Bbox; 

/**
 * @brief   Create a body bounding box based on the detected face box
 */
Bbox estimate_body_box
(
    const Bbox& face_box,
    bool standardized = true
);

/** 
 * @brief   Create a cone whose tip is at the location, 
 *          and the direction of the tip is specified by directio
 */
Matrix create_gaze_cone_pts
(
    double height, 
    double radius, 
    const Vector& location,
    double pitch,
    double yaw,
    double roll,
    const Perspective_camera& camera
);

/** 
 * @brief   Create a cone for the body parts specified by the body_box  
 *          and the direction of the tip is specified by direction
 */
inline 
Matrix create_body_part_cone_pts
(
    double cone_length,
    const Bbox& part_box,
    const Perspective_camera& camera
)
{
    Matrix m(5, 3);
    const Vector& center = part_box.get_center();
    double width = part_box.get_width();
    double height = part_box.get_height();
    const Matrix& camera_matrix = camera.build_camera_matrix();
    const Vector& camera_location = geometry::projective_to_euclidean(camera.get_camera_centre()); 

    Vector v1(center);
    Vector v2(center);
    Vector v3(center);
    Vector v4(center);

    // Assuming that the part_box is already standardized 
    v1(0) = v1(0) - width/2.0;
    v1(1) = v1(1) - height/2.0; // bottom left point
    v2(0) = v2(0) - width/2.0;
    v2(1) = v2(1) + height/2.0; // up left point
    v3(0) = v3(0) + width/2.0; 
    v3(1) = v3(1) + height/2.0; // up right point
    v4(0) = v4(0) + width/2.0;
    v4(1) = v4(1) - height/2.0; // bottom right point

    // get the homogeneous coordinate
    v1 = geometry::euclidean_to_projective_2d(v1);
    v2 = geometry::euclidean_to_projective_2d(v2);
    v3 = geometry::euclidean_to_projective_2d(v3);
    v4 = geometry::euclidean_to_projective_2d(v4);

    Vector v1p = backproject(v1, camera_matrix);
    Vector v2p = backproject(v2, camera_matrix);
    Vector v3p = backproject(v3, camera_matrix);
    Vector v4p = backproject(v4, camera_matrix);

    Vector v1l = camera_location + v1p*cone_length;
    Vector v2l = camera_location + v2p*cone_length;
    Vector v3l = camera_location + v3p*cone_length;
    Vector v4l = camera_location + v4p*cone_length;

    m.set_row(0, v1l);
    m.set_row(1, v2l);
    m.set_row(2, v3l);
    m.set_row(3, v4l);
    m.set_row(4, camera_location);
    return m;
}

/**
 * @brief   Create a cylinder for the body and return the points on the cylinder
 */
Matrix create_cylinder_pts
(
    double radius,
    double height,
    const Vector& location
);

/**
 * @brief   Get the corresponding person based on the location of the face 
 */
bool get_corresponding_entity
(
    const Bbox& face_box,
    const pt::Box_trajectory_map& trajectories,
    pt::Entity_id& entity_id,
    size_t index,
    double overlapping_threshold = 0.4
);

/**
 * @brief   Find the corresponding person box based on the location of the face 
 */
bool get_corresponding_body_box
(
    const Bbox& face_box,
    Bbox& corr_body_box,
    const std::vector<Bbox>& body_boxes,
    double overlapping_threshold = 0.4
);

//bool get_holding_object_entity
//(
//    const Bbox& object_box,
//    const Skeleton_trajectory_map& trajectories,
//    pt::Entity_id& entity_id,
//    size_t index,
//    double overlapping_threshold = 0.1
//);

/**
 * @brief   Get the corresponding face based on the location of the body box
 */
bool get_corresponding_face
(
    const std::vector<Face_detection> & faces,
    const Bbox& body_box,
    Face_detection& matched_face,
    double overlapping_threshold = 0.4
);

/**
 * @brief  Check if the face box belongs to the body bounding box  
 *         Note: the Face_detection face_box is in image coordinate 
 *         Assume the body_box is also in image coordiantes
 */
bool face_inside_body_box 
(
    const Face_detection& face,
    const Bbox& body_box,
    double face_area_threshold = 0.7,
    double body_area_threshold = 0.07
);

/**
 * @brief Standardize face boxes 
 *
 */
void standardize_face_boxes 
(
    std::vector<Face_detection>& faces,
    double image_width, 
    double image_height
);

/**
 * @brief   Draw hull for debugging purposes 
 */
void draw_hull
(
    const Matrix& hull_pts, 
    const Matrix& camera_matrix, 
    double win_width,
    double win_height,
    Image& image,
    const Image::Pixel_type& vertex_pix,
    const Image::Pixel_type& facet_pix
);

/**
 * @breif Draw face box
 */
inline
void draw_standardized_face_box
(
    const Face_detection& fd,
    Image& img, 
    Image::Pixel_type pixel,
    int line_width
)
{
    double win_width = img.get_num_cols();
    double win_height = img.get_num_rows();
    Bbox box(fd.box());
    pt::unstandardize(box, win_width, win_height);
    box.draw(img, pixel.r, pixel.g, pixel.b, line_width); 
}

} //namespace psi
} //namespace kjb

#endif
