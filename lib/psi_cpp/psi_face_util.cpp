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

/* $Id: psi_face_util.cpp 21596 2017-07-30 23:33:36Z kobus $ */

#include "l/l_sys_debug.h"  /* For ASSERT */
#include "psi_cpp/psi_face_util.h"
//#include "psi_cpp/psi_skeleton.h"
//#include "psi_cpp/psi_skeleton_trajectory.h"
#include "g_cpp/g_camera.h"
#include "g_cpp/g_hull.h"
#include "g_cpp/g_util.h"
#include "people_tracking_cpp/pt_util.h"
#include "people_tracking_cpp/pt_box_trajectory.h"

#include <boost/foreach.hpp>

using namespace kjb;
using namespace kjb::psi;

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

Bbox kjb::psi::estimate_body_box
(
    const Bbox& face_box,
    bool standardized
)
{
    double body_height = face_box.get_height()*7.5;
    double body_width = face_box.get_width()*3.0;
    if(standardized)
    {
        Vector face_top = face_box.get_top_center();
        Vector body_center(face_top);
        body_center[1] = face_top[1] - body_height/2.0;
        return Bbox(body_center, body_width, body_height);
    }
    else
    {
        Vector face_top = face_box.get_bottom_center();
        Vector body_center(face_top);
        body_center[1] = face_top[1] + body_height/2.0;
        return Bbox(body_center, body_width, body_height);
    }
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

Matrix kjb::psi::create_gaze_cone_pts
(
    double height, 
    double radius, 
    const Vector& location,
    double pitch,
    double yaw,
    double roll,
    const Perspective_camera& camera
)
{
    const int num_cir_pts = 36;
    const double arc_size = 2.0 * M_PI / num_cir_pts;
    Matrix m(num_cir_pts + 1, 3);

    // The tip of the cone
    m(0, 0) = 0.0;
    m(0, 1) = height;
    m(0, 2) = 0.0; 

    // filled up the points on the circle
    Vector origin(3, 0.0);
    for(int i = 0; i < num_cir_pts; i++)
    {
        m(i, 0) = radius * cos(i * arc_size);
        m(i, 1) = radius * sin(i * arc_size);
        m(i, 2) = height;

    }
    Vector tip(3, 0.0);
    m.set_row(num_cir_pts, tip);

    // Rotate all the points 
    Quaternion face_orientation(-pitch, yaw, -roll, Quaternion::XYZR);
    Quaternion camera_orientation = camera.get_orientation();
    for(int i = 0; i < num_cir_pts+1; i++)
    {
        Vector gaze_dir = m.get_row(i);
        gaze_dir = face_orientation.rotate(gaze_dir);
        gaze_dir = camera_orientation.conj().rotate(gaze_dir);
        gaze_dir += location;
        m.set_row(i, gaze_dir);
    }
    return m;
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

Matrix kjb::psi::create_cylinder_pts
(
    double radius,
    double height,
    const Vector& location
)
{
    const int num_cir_pts = 36;
    const double arc_size = 2.0 * M_PI / num_cir_pts;
    Matrix m(num_cir_pts * 2, 3);

    // create a cylinder at the origin 
    for(int i = 0; i < num_cir_pts; i++)
    {
        // bottom circle 
        m(i, 0) = location[0] + radius * cos(i * arc_size);
        m(i, 1) = location[1];
        m(i, 2) = location[2] + radius * sin(i * arc_size);

        // top circle 
        m(i + num_cir_pts, 0) = location[0] + radius * cos(i * arc_size);
        m(i + num_cir_pts, 1) = location[1] + height; // y value
        m(i + num_cir_pts, 2) = location[2] + radius * sin(i * arc_size);
    }
    return m;
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

bool kjb::psi::get_corresponding_entity
(
    const Bbox& face_box,
    const pt::Box_trajectory_map& trajectories,
    pt::Entity_id& entity_id,
    size_t index,
    double overlapping_threshold
)
{
    pt::Box_trajectory_map::const_iterator traj_it;
    double max_overlapping = 0.0;

    for(traj_it = trajectories.begin(); traj_it != trajectories.end(); traj_it++)
    {
        if((traj_it->second).at(index))
        {
            const Bbox& cur_body_box = (*(traj_it->second).at(index)).value;
            double face_area = face_box.get_width()*face_box.get_height();
            double overlapping_area = get_rectangle_intersection(face_box, 
                                                                cur_body_box);
            double overlapping = overlapping_area/face_area;
            if(overlapping > max_overlapping)
            {
                // get the key
                entity_id = traj_it->first;
                max_overlapping = overlapping;
            }
        }
    }
    if(max_overlapping > overlapping_threshold)
        return true;
    else
        return false;
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

bool kjb::psi::get_corresponding_body_box
(
    const Bbox& face_box,
    Bbox& corr_body_box,
    const std::vector<Bbox>& body_boxes,
    double overlapping_threshold
)
{
    double max_overlapping = 0.0;
    size_t index = -1; 
    for(size_t i = 0; i < body_boxes.size(); i++)
    {
        double face_area = face_box.get_width()*face_box.get_height();
        double overlapping_area = get_rectangle_intersection(face_box, 
                                                        body_boxes[i]); 
        double overlapping = overlapping_area/face_area;
        if(overlapping > max_overlapping)
        {
            max_overlapping = overlapping;
            index = i;
        }
    }
    if(max_overlapping > overlapping_threshold)
    {
        corr_body_box = body_boxes[index];
        return true;
    }
    else
    {
        return false;
    }
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

bool kjb::psi::get_corresponding_face
(
    const std::vector<Face_detection>& faces,
    const Bbox& body_box,
    Face_detection& matched_face,
    double overlapping_threshold
)
{
    if(faces.empty()) return false;

    std::vector<Face_detection>::const_iterator face_it;
    double max_overlapping = 0.0;
    for(face_it = faces.begin(); face_it != faces.end(); face_it++)
    {
        const Bbox& cur_face_box = face_it->box();
        double face_area = cur_face_box.get_width()*cur_face_box.get_height();
        double overlapping_area = get_rectangle_intersection(cur_face_box, 
                                                             body_box);
        double overlapping = overlapping_area/face_area;
        if(overlapping > max_overlapping)
        {
            max_overlapping = overlapping;
            matched_face = *face_it;
        }
    }

    if(max_overlapping > overlapping_threshold)
        return true;
    else
        return false;

}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

//bool kjb::psi::get_holding_object_entity
//(
//    const Bbox& object_box,
//    const Skeleton_trajectory_map& trajectories,
//    Entity_id& entity_id,
//    size_t index,
//    double overlapping_threshold
//)
//{
//    Skeleton_trajectory_map::const_iterator traj_it;
//    double max_overlapping = 0.0;
//
//    for(traj_it = trajectories.begin(); traj_it != trajectories.end(); traj_it++)
//    {
//        if((traj_it->second).at(index))
//        {
//            const Psi_skeleton& cur_skeleton = 
//                (*(traj_it->second).at(index)).value;
//            const Bbox& cur_left_hand_box = cur_skeleton.get_body_part("HandLeft").box;
//            const Bbox& cur_right_hand_box = cur_skeleton.get_body_part("HandRight").box;
//            double object_area = object_box.get_width()*object_box.get_height();
//            double area_left = get_rectangle_intersection(object_box, cur_left_hand_box); 
//            double area_right = get_rectangle_intersection(object_box, cur_right_hand_box);
//            double overlapping_left = area_left/object_area;
//            double overlapping_right = area_right/object_area;
//            if(overlapping_left > max_overlapping || overlapping_right > max_overlapping)
//            {
//                // get the key
//                entity_id = traj_it->first;
//                if(overlapping_left > max_overlapping)
//                    max_overlapping = overlapping_left;
//                else
//                    max_overlapping = overlapping_right;
//            }
//        }
//    }
//    if(max_overlapping > overlapping_threshold)
//        return true;
//    else
//        return false;
//}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

bool kjb::psi::face_inside_body_box 
(
    const Face_detection& face,
    const Bbox& body_box,
    double face_area_threshold,
    double body_area_threshold
)
{
    const Bbox& fbox = face.box();
    double frea = fbox.get_area();
    double brea = body_box.get_area();
    const double small_thresh = frea * face_area_threshold;
    const double big_thresh = brea * body_area_threshold;
    double olap = get_rectangle_intersection(fbox, body_box);
    double distance = vector_distance(body_box.get_bottom_center(), 
            fbox.get_bottom_center());
    double dist_thresh = fbox.get_height();

    if(olap > small_thresh  && olap < big_thresh && distance < dist_thresh)
    {
        return true;
    }
    else
    {
        return false;
    }
    
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

void kjb::psi::standardize_face_boxes 
(
    std::vector<Face_detection>& faces,
    double image_width, 
    double image_height
)
{
    BOOST_FOREACH(Face_detection& face, faces)
    {
        pt::standardize(face.box(), image_width, image_height);
    }
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

void kjb::psi::draw_hull
(
    const Matrix& hull_pts, 
    const Matrix& camera_matrix, 
    double win_width,
    double win_height,
    Image& image,
    const Image::Pixel_type& vertex_pix,
    const Image::Pixel_type& facet_pix
)
{
    Matrix hull_vertices;
    std::vector<Matrix> hull_facets;
    get_convex_hull(hull_pts, hull_vertices, hull_facets); 
    Matrix pts_2d(hull_vertices.get_num_rows(), 2, 0.0);
    // Project
    for(int k = 0; k < hull_vertices.get_num_rows(); k++)
    {
        const Vector& pt = hull_vertices.get_row(k);
        Vector x = geometry::projective_to_euclidean_2d(
                        camera_matrix * 
                        geometry::euclidean_to_projective(pt)
                        );
        pt::unstandardize(x, win_width, win_height);
        pts_2d.set_row(k, x);
    }

    // find the convext of the 2D projections
    Matrix hull_vertices_2d;
    std::vector<Matrix> hull_facets_2d;
    get_convex_hull(pts_2d, hull_vertices_2d, hull_facets_2d); 

    // draw the vertices
    for(int k = 0; k < hull_vertices_2d.get_num_rows(); k++)
    {
        image.draw_point(hull_vertices_2d(k, 1), 
                         hull_vertices_2d(k, 0), 
                         2.0, 
                         vertex_pix);
    }

    // draw the facets
    for(size_t k = 0; k < hull_facets_2d.size(); k++)
    {
        ASSERT(hull_facets_2d[k].get_num_rows() == 2);
        image.draw_line_segment(hull_facets_2d[k](0, 1), 
                                hull_facets_2d[k](0, 0), 
                                hull_facets_2d[k](1, 1), 
                                hull_facets_2d[k](1, 0),
                                2.0, 
                                facet_pix);
    }
}

