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
|     Luca Del Pero
|
* =========================================================================== */

#include "gr_cpp/gr_rigid_object.h"
#include "m_cpp/m_matrix_stream_io.h"
#include "m_cpp/m_vector_stream_io.h"

using namespace kjb_c;
using namespace kjb;

/**
 *  @param  ro Rigid_object to copy into this one.
 */
Rigid_object::Rigid_object(const Rigid_object & ro)
: Transformable(), Cloneable(), temp_matrix(4,4)
{
    (*this) = ro;
}

/**
 *  @param  src Rigid_object to be assigned to this one
 */
Rigid_object& Rigid_object::operator=(const Rigid_object& src)
{
    q = src.q;
    return (*this);
}

/**
 * Translates this rigid object
 *
 * @param dx The amount of translation along the world's x axis
 * @param dy The amount of translation along the world's y axis
 * @param dz The amount of translation along the world's z axis
 */
void Rigid_object::translate(double dx, double dy, double dz)
{
    Matrix translation = Matrix::create_3d_homo_translation_matrix(dx, dy, dz);
    transform(translation);
}

/**
 * Rotates this object around its x-axis by the input angle dpitch,
 * around its y-axis by the input angle dyaw, around its z-axis by the input angle
 * droll. The rotations will add to the current state of the object,
 * ie if the inputs are dpitch = 0.1, dyaw =0.0 and, droll = 0.2,
 * the object will be rotated around the object's x-axis by
 * dpitch, and then around the object's z axis (that moved
 * after the first rotation by dpitch) by 0.2
 *
 * @param dpitch the amount of rotation around the object's x-axis
 * @param dyaw the amount of rotation around the object's y-axis
 * @param droll the amount of rotation around the object's z-axis
 */
void  Rigid_object::rotate(double dpitch, double dyaw, double droll)
{
    Quaternion q2(dpitch, dyaw, droll,q.get_euler_mode());
    q = q*q2;
    transform(q.get_rotation_matrix());
}

/** @brief computes the new values for the object's euler angles,
 * after a rotation of dpitch around the object's x-axis, a rotation
 * of dyaw around the object's y axis, and a rotation of droll
 * around the object's z axis (in this order). The state of this rigid
 * object is not changed
 *
 * @param dpitch the amount of rotation around the object's x-axis
 * @param dyaw the amount of rotation around the object's y-axis
 * @param droll the amount of rotation around the object's z-axis
 * @param angles will contain the new Euler angles (pitch, yaw, roll)
 */
void Rigid_object::compute_new_euler_angles_on_rotations(double dpitch, double dyaw, double droll, kjb::Vector & angles) const
{
    Quaternion q2(dpitch, dyaw, droll,q.get_euler_mode());
    q2 = q*q2;
    angles =  q2.get_euler_angles();
}

/**
 * Rotates this rigid object so that the amount of rotation around
 * its x-axis will match the input pitch, the amount of rotation
 * around its y-axis will match the input yaw and the amount
 * of rotation around its z-axis will match the input roll.
 * The rotations are done in this order starting from a configuration
 * where the object is aligned with the world's axis
 *
 * @param pitch The desired amount of rotation around the object's x axis
 * @param yaw The desired amount of rotation around the object's y axis
 * @param roll The desired amount of rotation around the object's z axis
 */
void Rigid_object::set_rotations(double pitch, double yaw, double roll)
{
    q.set_euler_angles(pitch, yaw, roll);
    transform(q.get_rotation_matrix());
}

/**
 * Rotates this rigid object around its x axis by the input
 * pitch angle, then around the object's y axis by the input
 * yaw angle, and last around the object's z axis by the
 * input roll angle. Rotations are performed IN THIS ORDER,
 * consider this if the resulting rotations are not as you
 * would expect (rotation is not transitive).
 * Last, the object is translated along the world's x-axis
 * by dx, along the world's y axis by dy, and along the
 * world's z axis by dz (The object is translated by these
 * amounts starting from its current location).
 * The method is provided to set rotations and translate
 * in an efficient way, by avoiding a matrix multiplication
 *
 * @param pitch The desired amount of rotation around the object's x axis
 * @param yaw The desired amount of rotation around the object's y axis
 * @param roll The desired amount of rotation around the object's z axis
 * @param dx the amount of translation along the world's x-axis
 * @param dy the amount of translation along the world's y-axis
 * @param dz the amount of translation along the world's z-axis
 */
void Rigid_object::set_rotations_and_translate(double pitch, double yaw, double roll, double dx, double dy, double dz)
{
    q.set_euler_angles(pitch, yaw, roll);
    temp_matrix = q.get_rotation_matrix();
    temp_matrix(0,3) = dx;
    temp_matrix(1,3) = dy;
    temp_matrix(2,3) = dz;
    transform(temp_matrix);
}

void Rigid_object::set_orientation(const Quaternion& orientation)
{
    q = orientation;
    temp_matrix = q.get_rotation_matrix();
    transform(temp_matrix);
}
