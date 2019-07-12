/* $Id: g_cylinder_section.h 12850 2012-08-16 01:07:41Z elh $ */
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

/**
 * @file
 *
 * @author Emily Hartley
 *
 * @brief Cylinder_section: a section of a geometric cylinder
 */

#ifndef KJB_CYLINDER_SECTION_H
#define KJB_CYLINDER_SECTION_H

#include <g_cpp/g_cylinder.h>
#include <l_cpp/l_readable.h>
#include <l_cpp/l_writeable.h>

#define TWO_PI 6.28319

namespace kjb
{

/**
 * @class Cylinder_section
 *
 * @brief Cylinder_section: a section of a cylinder, specified by angle and position.
 */
class Cylinder_section : public Cylinder
{

public:
    /** @brief Constructs a cylinder section. */
    Cylinder_section(const Vector& p1, const Vector& p2, double radius, double angle, const Vector& angle_startpt, const Vector& angle_endpt) :
        Cylinder(p1, p2, radius),
        angle_(angle),
        angle_startpt_(angle_startpt),
        angle_endpt_(angle_endpt)
    {
        if(angle_ < (-1 * TWO_PI) || angle_ > TWO_PI)
        {
            KJB_THROW_2(Illegal_argument, "Cylinder_section(): angle must be between -2*PI and 2*PI");
        }

        if(angle_startpt_.size() < 3 || angle_startpt_.size() > 4)
        {
            KJB_THROW_2(Illegal_argument, "Cylinder_section(): start point of angle must be 3-dimensional");
        }

        if(angle_endpt_.size() < 3 || angle_endpt_.size() > 4)
        {
            KJB_THROW_2(Illegal_argument, "Cylinder_section(): end point of angle must be 3-dimensional");
        }
    }

    /** @brief Reads a cylinder section from an input file. */
    Cylinder_section(const char* fname) throw (kjb::Illegal_argument, kjb::IO_error) :
        Cylinder(fname),
        angle_startpt_(3, 0.0),
        angle_endpt_(3, 0.0)
    {
        
    }

    /** @brief Reads a cylinder section from an input file. */
    Cylinder_section(std::istream& in) throw (kjb::Illegal_argument, kjb::IO_error) :
        Cylinder(in),
        angle_startpt_(3, 0.0),
        angle_endpt_(3, 0.0)
    {
        read(in);
    }

    /** @brief Copy constructor. */
    Cylinder_section(const Cylinder_section& c);

    /** @brief Copies a cylinder section into this one. */
    virtual Cylinder_section& operator= (const Cylinder_section& c);

    /** @brief Destructor of a cylinder section. */
    virtual ~Cylinder_section();

    virtual void read(std::istream & in);

    virtual void write(std::ostream & out) const;

    double get_angle() const
    {
        return angle_;
    }

    const Vector& get_startpoint_of_angle() const
    {
        return angle_startpt_;
    }

    const Vector& get_endpoint_of_angle() const
    {
        return angle_endpt_;
    }

    
private:
    /* If angle is positive, then go counter-clockwise starting from
     * angle_startpt. 
     * If angle is negative, then go clockwise starting from 
     * angle_startpt.
     */
    double angle_;

    /* angle_startpt is a point on the base of the cylinder_section (on 
     * the side containing p2) that is located on the edge of the
     * cylinder_section.  The sign of the angle denotes the direction with
     * respect to this point. 
     */
    Vector angle_startpt_;

    /* angle_endpt is the other point on the base of the cylinder_section 
     * (on the side containing p2) that is located on the edge of the
     * cylinder_section. 
     */
    Vector angle_endpt_;
};

} // namespace kjb

#endif
