/* $Id: g_camera.h 12639 2012-07-06 17:19:26Z ernesto $ */
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
   |  Author:  Kyle Simek, Ernesto Brau, Jinyan Guan
 * =========================================================================== }}}*/

// vim: tabstop=4 shiftwidth=4 foldmethod=marker
#ifndef KJB_CPP_G_CAMERA_H
#define KJB_CPP_G_CAMERA_H

#include <l_cpp/l_algorithm.h>
#include <m_cpp/m_matrix.h>
#include <m_cpp/m_matrix_d.h>
#include <m_cpp/m_vector.h>
#include <m_cpp/m_vector_d.h>
#include <vector>
#include <limits>

namespace kjb {

/**
 * Receives a point in homogeneous screen coordinates and back-projects it
 * onto its location on the plane at infinity. This 
 * can be interpreted as a 3D direction vector, d, which together with the
 * camera center, c, defines the 3D back-projection line, l:
 *
 *  l = c + d * t  ;  for all t
 *
 * @param homo_camera_coord Camera coordinate to be backprojected. Specified
 *                          in 2D homogeneous coordinates (size must be 3).
 * @param camera_matrix A 3x4 camera matrix for projecting 3d points to 2d.
 *
 * @return  The back-projected coordinate on the plane at infinity. 
 *          Alternatively, this is the 3D direction vector that defines the
 *          backprojection line (see above).
 */
Vector backproject
(
    const Vector& homo_camerascreen_coord,
    const Matrix& camera_matrix
);

/** @brief  Same as backproject(), but using Vector3. */
Vector3 backproject
(
    const Vector3& homo_camera_coord,
    const Matrix_d<3,4>& camera_matrix
);

/**
 * Same as backproject(), but the matrix passed-in is the inverse of the left
 * 3x3 submatrix of the camera matrix.  
 * This saves time when you've already computed the inverse matrix and you
 * want to backproject a large number of points.
 *
 * @param homo_screen_coord Screen coordinate to be backprojected. Specified
 *                          in 2D homogeneous coordinates (size must be 3).
 *
 * @param M_inv Let P be a 3x4 camera matrix P. M_inv is the inverse of the
 *              left 3x3 submatrix of P,  i.e.  M_inv = P(0:2, 0:2)^-1
 *
 * @sa backproject
 */
Vector backproject_with_m_inv
(
    const Vector& homo_screen_coord,
    const Matrix& M_inv
);

/** @brief  Same as backproject_with_m_inv(), but using Vector3. */
Vector3 backproject_with_m_inv
(
    const Vector3& homo_screen_coord,
    const Matrix_d<3,3>& M_inv
);

/**
 * @brief   Linearly-interpolate a two extrinsic camera matrices.
 *
 * Linearly-interpolate a twp extrinsic camera matrices.  "slerp" (quaternion
 * spherical linear interpolation) is used for interpolation of orientations,
 * while standard linear interpolation is used for translation.
 *
 r @param t linear interpolation parameter, must be scaled to be in [0,1]
 * @param use_slerp Use spherical interpolation of angles instead of linear.
 *                  Spherical exhibits constant rotational speed, while linear
 *                  speeds up in the middle, but is more efficient to compute
 */
Matrix lerp_extrinsic_camera_matrix(
        const Matrix& m1,
        const Matrix& m2,
        double t,
        bool use_slerp = false);

/**
 * @brief   Linearly-interpolate a set of extrinsic camera matrix at
 *          given timestamps.
 *
 * Linearly-interpolate a set of extrinsic camera matrix at 
 * given timestamps.
 *
 * "slerp" (quaternion spherical linear interpolation) is used 
 * for interpolation of orientations, while standard linear 
 * interpolation is used for translation.
 *
 * @pre timestamps is sorted ascending
 * @pre t >= timestamps.front() && t <= timestamps.back()
 */
Matrix lerp_extrinsic_camera_matrix(
        const std::vector<Matrix>& extrinsic,
        const std::vector<double>& timestamps,
        double t,
        bool use_slerp = false);

} // namespace kjb

#endif

