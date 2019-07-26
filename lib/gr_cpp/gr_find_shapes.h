/* $Id$ */
/**
 * This work is licensed under a Creative Commons 
 * Attribution-Noncommercial-Share Alike 3.0 United States License.
 * 
 *    http://creativecommons.org/licenses/by-nc-sa/3.0/us/
 * 
 * You are free:
 * 
 *    to Share - to copy, distribute, display, and perform the work
 *    to Remix - to make derivative works
 * 
 * Under the following conditions:
 * 
 *    Attribution. You must attribute the work in the manner specified by the
 *    author or licensor (but not in any way that suggests that they endorse you
 *    or your use of the work).
 * 
 *    Noncommercial. You may not use this work for commercial purposes.
 * 
 *    Share Alike. If you alter, transform, or build upon this work, you may
 *    distribute the resulting work only under the same or similar license to
 *    this one.
 * 
 * For any reuse or distribution, you must make clear to others the license
 * terms of this work. The best way to do this is with a link to this web page.
 * 
 * Any of the above conditions can be waived if you get permission from the
 * copyright holder.
 * 
 * Apart from the remix rights granted under this license, nothing in this
 * license impairs or restricts the author's moral rights.
 */

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
|     Emily Hartley
|
* =========================================================================== */

#ifndef KJB_FIND_SHAPES_H
#define KJB_FIND_SHAPES_H

#include "gr_cpp/gr_polymesh_plane.h"
#include "gr_cpp/gr_polygon.h"
#include "gr_cpp/gr_right_triangle_pair.h"
#include "m_cpp/m_int_vector.h"
#include "g_cpp/g_cylinder.h"
#include "g_cpp/g_cylinder_section.h"
#include "g_cpp/g_circle.h"
#include <vector>

namespace kjb {

/**
 * @brief  Finds the coefficients of the plane of the form ax + by + cz + d = 0
 * given the normal vector and a point on the plane.
 */
void get_plane_parameters
(
    const Vector& normal, 
    const Vector& point_on_plane, 
    Vector&       plane_params
);

/**
 * @brief  Finds the coefficients of the plane of the form ax + by + cz + d = 0
 * given three points on the plane.
 */
void get_plane_parameters
(
    const Vector& pt1, 
    const Vector& pt2, 
    const Vector& pt3, 
    Vector&       plane_params
); 

/**
 * @brief Checks if two faces in a polymesh are coplanar.
 */
bool check_if_faces_are_coplanar
(
    const Vector& plane1_params, 
    const Vector& plane2_params, 
    double        tolerance, 
    double        distTolerance=1.0
);

/**
 * @brief Checks if 4 points are coplanar.
 */
bool check_if_4_points_are_coplanar
(
    const Vector& p1, 
    const Vector& p2, 
    const Vector& p3, 
    const Vector& p4, 
    double        tolerance
);

/**
 * @brief Calculates the smaller angle between the normal vectors of two planes.
 * The angle returned is in radians.
 */
double get_angle_between_two_vectors
(
    const Vector& plane1_params, 
    const Vector& plane2_params
) throw (Illegal_argument, KJB_error);

/////////////////////////////////////////////////////////////////////////////////////////

/**
 * @brief Finds all of the planes in a polymesh and stores each set of
 * coefficients along with the corresponding face indices in a Polymesh_Plane.
 */
void find_planes(const Polymesh& p, std::vector<Polymesh_Plane>& plane);

/**
 * @brief Renders each plane in a polymesh with a different color.
 */
void render_planes
(
    const Polymesh&                    p, 
    const std::vector<Polymesh_Plane>& planes
);

/**
 * @brief Finds all of the circles and the points that lie on them in the 
 * polymesh.
 */
void find_all_circles_in_polymesh
(
    Polymesh&                          p, 
    std::vector<Circle_in_3d>&         circles, 
    std::vector<std::vector<Vector> >& points
);

/////////////////////////////////////////////////////////////////////////////////////////

/**
 * @brief Creates an Int_vector mask representing the faces in a polymesh that
 * are right triangles.
 */
void find_right_triangles
(
    const std::vector<kjb::Polygon>& faces, 
    Int_vector&                      mask
);

/**
 * @brief Finds all pairs of right triangles in a polymesh that are 
 * coplanar, adjacent along their hypotenuses, and have the same vertices
 * along the shared edge (i.e. the triangle pairs form a rectangle).
 */
void find_rectangles
(
    const std::vector<kjb::Polygon>&  faces, 
    const Int_vector&                 mask, 
    const Polymesh&                   p, 
    std::vector<Right_Triangle_Pair>& rectangles
);

/**
 * @brief Creates an Int_vector mask representing the faces in a polymesh that
 * are part of a rectangle.
 */
void create_rectangle_mask
(
    const Polymesh&                         p, 
    const std::vector<Right_Triangle_Pair>& rectangles, 
    Int_vector&                             rectMask
);

/**
 * @brief Determines which rectangles are adjacent to eachother and if a 
 * group of adjacent rectangles form part of a cylinder.
 */
void find_adjacent_rectangles
(
    const Polymesh&                         p, 
    const Int_vector&                       cylMask, 
    const Int_vector&                       rectMask, 
    const std::vector<Right_Triangle_Pair>& rectangles, 
    int                                     startIndex, 
    int                                     rectIndex, 
    int                                     prevAdjFace, 
    double                                  width, 
    double                                  lengthTolerance, 
    double&                                 smallestAngle, 
    double&                                 largestAngle, 
    double&                                 sumAngles, 
    std::vector<int>&                       cyl_indices, 
    Vector&                                 edge_pt1, 
    Vector&                                 edge_pt2, 
    Vector&                                 edge_pt1_adj, 
    Vector&                                 edge_pt_adj
);

/**
 * @brief Determines which groups of faces in the polymesh form cylinders.
 */
void find_cylinders
(
    const Polymesh&                         p, 
    const std::vector<Right_Triangle_Pair>& rectangles, 
    std::vector<std::vector<int> >&         cyl_indices, 
    std::vector<double>&                    cylSumAngles, 
    std::vector<std::vector<Vector> >&      cylEdgePoints
);

/////////////////////////////////////////////////////////////////////////////////////////

/**
 * @brief Determines which points make up the 'top' and which points make up the
 * 'bottom' of the cylinder specified by the provided polymesh face indices.
 */
void find_top_and_bottom_points_of_cylinder
(
    const Polymesh&                         p, 
    const Int_vector&                       rectMask, 
    const std::vector<Right_Triangle_Pair>& rectangles, 
    const std::vector<int>&                 cyl_indices, 
    std::vector<Vector>&                    top_points, 
    std::vector<Vector>&                    bottom_points
);

/**
 * @brief Finds the center of a set of 3D points.
 */
void find_centroid_of_3d_points
(
    const std::vector<Vector>& points, 
    Vector&                    centroid
);

/**
 * @brief Uses singular value decomposition (SVD) to fit a plane to a set of 
 * 3D points.
 */
void fit_plane_to_3d_points
(
    const std::vector<Vector>& points, 
    const Vector&              centroid, 
    Vector&                    plane_params
);

/**
 * @brief Projects 3D points onto the best-fit 3D plane.
 */
void project_points_onto_plane
(
    std::vector<Vector>& points, 
    const Vector&        plane_params, 
    const Vector&        centroid, 
    std::vector<Vector>& projected_points
);

/*
 * @brief Maps a 3D plane to the X-Y plane.
 */
void translate_3d_plane_to_xy_plane
(
    const std::vector<Vector>& points, 
    const Vector&              plane_params, 
    std::vector<Vector>&       translated_points, 
    std::vector<Matrix>&       transformMatrices
);

/*
 * @brief Using the transformMatrices from translate_3d_plane_to_xy_plane(), 
 * maps a point on the X-Y plane back onto the original 3D plane.
 */
void translate_xy_point_to_3d_plane
(
    const Vector&              point, 
    const std::vector<Matrix>& transformMatrices, 
    Vector&                    translated_point
);

/**
 * @brief Determines the parameters of the cylinder formed by a set of 
 * faces in the polymesh.
 */
void fit_cylinder
(
    const Polymesh&                         p, 
    const Int_vector&                       rectMask, 
    const std::vector<Right_Triangle_Pair>& rectangles, 
    const std::vector<int>&                 cyl_indices, 
    double                                  cylAngle, 
    const std::vector<Vector>&              cylEdgePoints, 
    Cylinder_section&                       cyl
);


/////////////////////////////////////////////////////////////////////////////

void find_cylinders_to_render_and_fit
(
    char*                           faceFile, 
    char*                           cylFile, 
    const Polymesh&                 p,
    std::vector<Polymesh_Plane>&    plane, 
    std::vector<Cylinder_section>&  cyl
);

/**
 * @brief Renders each set of faces that forms a cylinder a different color.
 */
void find_cylinders_to_render
(
    const Polymesh&              p, 
    std::vector<Polymesh_Plane>& plane
);

/**
 * @brief Renders each pair of faces that forms a rectangle a different color.
 */
void find_rectangles_to_render
(
    const Polymesh&              p, 
    std::vector<Polymesh_Plane>& plane
);

/**
 * @brief Renders each face that is a right triangle in a different color.
 */
void find_right_triangles_to_render
(
    const Polymesh&              p, 
    std::vector<Polymesh_Plane>& plane
);

/**
 * @brief Renders each face that is a right triangle and adjacent to another 
 * face that is also a right triangle in a different color.
 */
void find_adjacent_right_triangles_to_render
(
    const Polymesh&              p, 
    std::vector<Polymesh_Plane>& plane
);

}

#endif
