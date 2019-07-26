/* $Id: g_camera_calibration.h 18278 2014-11-25 01:42:10Z ksimek $ */
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

#ifndef KJB_G_CPP_CAMERA_CALIBRATION
#define KJB_G_CPP_CAMERA_CALIBRATION

#include <iosfwd>

namespace kjb
{
class Matrix;
class Vector;

struct Calibration_descriptor
{
    static const Calibration_descriptor STANDARD;
    enum Coord_convention { 
        TOP_LEFT, /// Origin at top-left, y-axis pointing down
        BOTTOM_LEFT, /// Origin at bottom-left, y-axis pointing up
        CENTER /// Origin at center of image, standard x and y axis

    };

    /// width of the calibration image
    int image_width;
    /// height of the calibration image
    int image_height;
    /// the image coordinate convention used to label image points
    Coord_convention convention;

    /// When the calibration image was captured, was the origin in front of the camera?
    bool world_origin_in_front;

    /** 
     * This influences how camera coordinates will be defined.
     * Setting this to "true" will make the z-axis point into the
     * screen, and the x-axis will point left to ensure a right-
     * handed coordinate system.  This is the convention of
     * Hartley and Zisserman.  Opengl has the x-axis pointing
     * out of the screen, corresponding to a "false" value, here.
     * Under this convention, the x-axis points right.
     */
    bool look_down_positive_z;
};

std::ostream& operator<<(std::ostream& ost, const Calibration_descriptor::Coord_convention& mode);

std::istream& operator>>(std::istream& ist, Calibration_descriptor::Coord_convention& mode);


/**
 * Standardize the intrinsic and extrinsic parameters :
 *   * Move the principal point offset relative to the center of the image, not the corner
 *   * Ensure the image y-axis points upward.  
 *   * Flip camera's z axis if world origin appears on wrong side of camera
 *   * Force det(rotation) to be 1.
 * 
 * The resulting decomposition should give identical mappings to calibrations that used different conventions but are otherwise identical.  
 *
 * @note The resulting intrinsic matrix might not have positive diagonal elements, indicating that the camera's internal reference frame differs from the calibration frame.  Hartley and zisserman describe this in terms of the extrinsic matrix, but it is more natural to push this manipulation into the intrinsic matrix.  
 * @note This assumes positive z-values are in front of the camera.  For opengl, you'll need to correct for this, since negative z-values are in front of the camera.  Negating the focal points should do it, but be aware that z-depths aren't represented in the intrinsic matrix, so you'll have to be smart with how you build the GL_PROJECTION matrix to get depth values correct.
 *
 * @param intrinsic [in/out] the intrinsic matrix to transform
 * @param rotation [in/out] the rotation matrix extrinsic parameter
 * @param translation [in/out] the translation extrinsic parameter
 * @param cal [in] The description of the calibration set-up, including image size, origin, and axis orientation.
 */
void standardize_camera_matrices(
        Matrix& intrinsic,
        Matrix& rotation,
        Vector& translation,
        const Calibration_descriptor& cal = Calibration_descriptor::STANDARD);

/**
 * Use LU decomposition to decompose the camera matrix into its intrinsic and extrinsic parts.  Ambiguity in the decomposition is resolved by forcing the diagonal elements of the intrinsic matrix to be positive.
 *
 * @param camera_matrix [in] The 3x4 camera matrix to be decomposed
 * @param intrinsic [out]   The output intrinsic camera matrix
 * @param rotation  [out]   The camera's extrinsic rotation
 * @param translation [out] The world origin relative to the camera coordinate frame.  The camera center in world coordinate frame can be found by   (- rotation.transpose() * translation).
 *
 * @post M = K [R | t] , where M is the input matrix, K is the intrinsic matrix, R is the rotation matrix, and t is the translation vector
 *
 * @author Kyle Simek
 */
void decompose_camera_matrix(
        const Matrix& camera_matrix, 
        Matrix& intrinsic,
        Matrix& rotation,
        Vector& translation);

};

#endif
