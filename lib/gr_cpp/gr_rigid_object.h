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
|     Joseph Schlecht, Luca Del Pero
|
* =========================================================================== */

/**
 * @file
 *
 * @author Luca Del Pero
 *
 */

#ifndef KJB_RIGID_OBJECT_H
#define KJB_RIGID_OBJECT_H

#include <m_cpp/m_vector.h>

#include "l_cpp/l_transformable.h"
#include "l_cpp/l_cloneable.h"
#include "g_cpp/g_quaternion.h"
#include <m_cpp/m_matrix.h>

#define RO_PITCH 0
#define RO_YAW 1
#define RO_ROLL 2

#define RO_ANGLE_EPSILON 1e-120

namespace kjb
{
    /* @class Rigid object: Implements basic transformation for rigid objects,
     * such translations and rotations. The object can be translated and rotated
     * around its three main axes using pitch, yaw and roll angles, following
     * classic Tait-Bryan formulation (ie, changing the pitch at a given moment
     * will rotate the object around its x-axis). Rotations are always performed in
     * the following order: pitch, yaw and roll. Quaternions are used to
     * perform rotations.
     */
    class Rigid_object : public Transformable, public Cloneable
    {
    typedef Rigid_object Self;
    public:
        Rigid_object() : Transformable(), Cloneable(),
        q(0.0, 0.0, 0.0, kjb::Quaternion::XYZR),temp_matrix(4,4)
        {

        }

        Rigid_object(const Rigid_object & ro);
        virtual Rigid_object& operator=(const Rigid_object& src);
        virtual ~Rigid_object() {}

        /** Swap contents with another Perspective_camera.
         * This is implemented to prevent deep copies, which allows it to run much more quickly than:
         *         Type tmp = b;
         *         b = a;
         *         a = tmp;
         */
        virtual void swap(Self& other)
        {
            q.swap(other.q);
            temp_matrix.swap(other.temp_matrix);
        }

        /** @brief Translates this rigid object */
        virtual void translate(double dx, double dy, double dz);

        /** @brief rotate this object around its x-axis by dpitch,
         * the y-axis by dyaw, and its z-axis by droll (in this order,
         * starting from the object's current position)
         */
        virtual void rotate(double dpitch, double dyaw, double droll);

        /** @brief computes the new values for the object's euler angles,
         * after a rotation of dpitch around the object's x-axis, a rotation
         * of dyaw around the object's y axis, and a rotation of droll
         * around the object's z axis (in this order, starting from
         * the object's current position). The state of this rigid
         * object is not changed
         */
        virtual void compute_new_euler_angles_on_rotations(double dpitch, double dyaw, double droll, kjb::Vector & angles) const;

        /** @brief rotate this object so that its pitch, yaw and roll
         *  match the input values
         */
        virtual void set_rotations(double pitch, double yaw, double roll);

        /** @brief rotate this object so that its pitch, yaw and roll
         *  match the input values, and translates it
         */
        virtual void set_rotations_and_translate(double pitch, double yaw, double roll, double dx, double dy, double dz);


        virtual Rigid_object * clone() const = 0;

        /** @ brief Transforms this rigid object by using the input
         *  3D transformation matrix, in homogeneous coordinates
         */
        virtual void transform(const kjb::Matrix & M) = 0;

        /** @brief Returns the current transformation matrix, that is used
         * to store the rotation to be applied to the object*/
        inline const Matrix & get_rotations() const
        {
            //return transformation_matrix;
            return q.get_rotation_matrix();
        }

        /** @brief returns vector [pitch, yaw, roll] */
        inline const Vector& get_euler_angles() const
        {
            return q.get_euler_angles();
        }

        /** @brief Sets the rotation mode of this rigid object.
         * All euler modes are supported (XYZ, ZYZ, etc). All modes
         * ar supported. For further details see @class Quaternion.
         * This class was adequately tested only in the case
         * of mode = XYZR*/
        inline void set_rotation_mode(kjb::Quaternion::Euler_mode imode)
        {
            q.set_euler_mode(imode);
        }

        /** @brief returns the quaternion defining this object's orientation */
        inline const kjb::Quaternion & get_orientation() const
        {
            return q;
        }

        /** @brief sets the orientation of this object from an input quaternion */
        void set_orientation(const Quaternion& orientation);

    private:

        Quaternion q;
        Matrix temp_matrix;

    };
}

#endif
