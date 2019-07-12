/* $Id: g2_rotation.h 18278 2014-11-25 01:42:10Z ksimek $ */
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

/**
 * @file General-purpose rotation classes and functions.
 */
#ifndef KJB_G2_CPP_ROTATION_H
#define KJB_G2_CPP_ROTATION_H

#include <m_cpp/m_vector.h>

namespace kjb
{
class Matrix;

/**
 * This class facilitates rotating around an arbitrary axis, not necesarilly
 * through the origin.  Unlike quaternions, which are pure rotations, this
 * operation generally involves translation to the origin, rotation, and 
 * translation back.
 *
 * Currently, this class is quite thin, and it provides no value caching like quaternion
 * does.  Multiple calls to get_rotation_matrix() will recompute every time.
 */
class Rotation_axis
{
public:
    /**
     * Create from a rotation axis line.  Line is represented in vector form:
     *  
     *    l(t) =  p + t * dir
     *
     * @param pt  A point lying on the rotation axis.  Must have size = 3.
     * @param dir The direction of the rotation axis.  Must have size = 3.
     */
    Rotation_axis(const Vector& pt, const Vector& dir);

    /**
     * Rotate a point around this axis.
     *
     * @arg pt  The point to rotate.  Must have size = 3.
     * @arg angle The angle (in radians) to rotate about the axis.
     */
    Vector rotate(const Vector& pt, double angle) const;

    /**
     * Get the rigid transformation matrix that rotates by _a_ degrees around this axis.
     *
     * @note No values are cached, so multiple calls to this will re-compute the matrix.
     */
    Matrix get_transformation_matrix(double a) const;
private:
    Vector axis_pt_;
    Vector axis_dir_;
}; // class Rotation_axis

} // namespace kjb

#endif
