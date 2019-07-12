/* $Id: g_util.h 18278 2014-11-25 01:42:10Z ksimek $ */
/* =========================================================================== *
   |
   |  Copyright (c) 1994-2010 by Kobus Barnard (author)
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
 * =========================================================================== */

#ifndef KJB_CPP_G_UTIL
#define KJB_CPP_G_UTIL

#include <m_cpp/m_vector.h>
#include <m_cpp/m_matrix.h>
#include <m_cpp/m_vector_d.h>
#include <m_cpp/m_matrix_d.h>

namespace kjb {
namespace geometry {

/**
 * @brief   Creates a translation matrix from the given vector.
 *
 * @param   v   The translation vector, which must be in 
 *              euclidean (non-homogeneous) coordinates!
 */
Matrix get_translation_matrix(const Vector& v);


/**
 * @brief   Creates a (2D) rotation matrix from the given angle.
 *
 * @param   theta   The angle.
 *
 * @note    If you want 3D rotations, use kjb::Quaternion.
 */
Matrix get_rotation_matrix(double theta);


/**
 * @brief   Creates a ND rotation matrix that rotates the first argument
 *          to the second one.
 *
 * @param   u   Starting vector
 * @param   v   Ending vector
 *
 * @note    If you want 3D rotations, use kjb::Quaternion.
 */
Matrix get_rotation_matrix(const Vector& u, const Vector& v);

/**
 * @brief   Creates a ND rotation matrix that rotates the first argument
 *          to the second one.
 *
 * @param   u   Starting vector
 * @param   v   Ending vector
 *
 * @note    If you want 3D rotations, use kjb::Quaternion.
 */
template<size_t D>
inline
Matrix_d<D, D> get_rotation_matrix(const Vector_d<D>& u, const Vector_d<D>& v)
{
    return Matrix_d<D, D>(
            get_rotation_matrix(
                Vector(u.begin(), u.end()),
                Vector(v.begin(), v.end())));
}

/**
 * @brief   Converts coordinates in projective space to coordinates in euclidean space.
 *
 * Converts coordinates in projective space to coordinates in euclidean space.  
 * i.e., converts from homogeneous to non-homogeneous coordinates. If the
 * given vector is already euclidean, this function simply returns it.
 *
 * This is a simple function, but is useful when passing to stl transform() to convert
 * a vector of homogeneous coordinates to non-homogeneous with a single line of code.
 *
 * @author Kyle Simek, Ernesto Brau
 */
Vector projective_to_euclidean(const Vector& v);


/**
 * @brief   Converts coordinates in (2D) projective space to coordinates in (2D) euclidean space.
 *
 * Converts coordinates in projective space to coordinates in euclidean space.  
 * i.e., converts from homogeneous to non-homogeneous coordinates. If the
 * given vector is already euclidean, this function simply returns it.
 *
 * This is a simple function, but is useful when passing to stl transform() to convert
 * a vector of homogeneous coordinates to non-homogeneous with a single line of code.
 *
 * @author Kyle Simek, Ernesto Brau
 */
Vector projective_to_euclidean_2d(const Vector& v);

/**
 * @brief   Converts coordinates in euclidean space to coordinates in projective space.
 *
 * Converts coordinates in euclidean space to coordinates in projective space.  
 * i.e., converts from non-homogeneous to homogeneous coordinates. If the
 * given vector is already homogeneous, this function simply returns it.
 *
 * This is a simple function, but is useful when passing to stl transform() to convert
 * a vector of non-homogeneous coordinates to homogeneous with a single line of code.
 *
 * @author Kyle Simek, Ernesto Brau
 */
Vector euclidean_to_projective(const Vector& v);


/**
 * @brief   Converts coordinates in (2D) euclidean space to coordinates in (2D) projective space.
 *
 * Converts coordinates in euclidean space to coordinates in projective space.  
 * i.e., converts from non-homogeneous to homogeneous coordinates. If the
 * given vector is already homogeneous, this function simply returns it.
 *
 * This is a simple function, but is useful when passing to stl transform() to convert
 * a vector of non-homogeneous coordinates to homogeneous with a single line of code.
 *
 * @author Ernesto Brau, Kyle Simek
 */
Vector euclidean_to_projective_2d(const Vector& v);

/**
is_point_in_polygon_new()  (updated cpp version)

Takes a polygon and a point. Determines wheter or not the point is in
the boundaries of the polygon.

NOTE: Points of the Polygon must be in cyclical order, otherwise you
might get the wrong answer

NOTE: if the point is on the border, returns true; 

Fun Fact: Works with non convex polygons! (except sometimes when
the point has exactly the y value of one of the polygon's 
verticies - working on that)

@param 
poly - a Nx2 Matrix of the points of a Polygon, in a cyclic order.
point - Vector(x,y) 

@returns 
false if point is outside polygon
true if point is inside polygon
*/
bool is_point_in_polygon_new(Matrix poly,Vector point);

/*
polygon_to_mask()

Takes a polygon and returns a height by width zero-1 matrix, where the 1's 
correspond to the area covered by the polygon, and 0's are where the
polygon is not. 

@param 
poly - a Nx2 Matrix of the points of a Convex Polygon, in a cyclic order.
height - height (rows) of mask Matrix
width - width (cols) of mask Matrix

*/
Matrix polygon_to_mask(Matrix poly,int height, int width);

}} // namespace kjb

#endif

