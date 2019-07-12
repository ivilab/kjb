/* $Id: g_cylinder_section.cpp 18278 2014-11-25 01:42:10Z ksimek $ */
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

#include <g_cpp/g_cylinder_section.h>
#include <sstream>

using namespace kjb;


/**
 * @param c  the Cylinder_section to copy into this one.
 */
Cylinder_section::Cylinder_section(const Cylinder_section& c)
: Cylinder(c.get_p1(), c.get_p2(), c.get_radius())
{
    this->angle_ = c.get_angle();
    this->angle_startpt_ = c.get_startpoint_of_angle();
    this->angle_endpt_ = c.get_endpoint_of_angle();
}

/** Frees all space allocated by this Cylinder_section. */
Cylinder_section::~Cylinder_section()
{
}

/**
 * Performs a deep copy of the center points of the cylinder bases,
 * the radius, the angle covered by the cylinder, and the bottom edge
 * points.
 *
 * @param c  the Cylinder_section to copy into this one.
 *
 * @return  A reference to this Cylinder_section.
 */
Cylinder_section& Cylinder_section::operator=(const Cylinder_section& c)
{
    if(this == &c) return *this;

    Cylinder::operator=(c);

//    this->p1_ = c.get_p1();
//    this->p2_ = c.get_p2();
//    this->r_ = c.get_radius();
    this->angle_ = c.get_angle();
    this->angle_startpt_ = c.get_startpoint_of_angle();
    this->angle_endpt_ = c.get_endpoint_of_angle();

    return *this;
}


void Cylinder_section::read(std::istream & in) 
{
    using std::ostringstream;
    using std::istringstream;

    const char * field_value;

    if(!(field_value = read_field_value(in, "angle")))
    {
        KJB_THROW_2(Illegal_argument,"Missing angle");
    }
    istringstream ist(field_value);
    ist >> angle_;
    if(angle_ < (-1 * TWO_PI) || angle_ > TWO_PI)
    {
        KJB_THROW_2(Illegal_argument, "Angle must be between -2*PI and 2*PI");
    }
    if(ist.fail())
    {
        KJB_THROW_2(Illegal_argument, "Missing angle");
    }
    ist.clear(std::ios_base::goodbit);
    //////////
    if(!(field_value = read_field_value(in, "angleStartpoint")))
    {
        KJB_THROW_2(Illegal_argument,"Missing angle startpoint");
    }
    ist.str(field_value);
    ist >> angle_startpt_(0) >> angle_startpt_(1) >> angle_startpt_(2);
    if(ist.fail())
    {
        KJB_THROW_2(Illegal_argument, "Missing angle startpoint");
    }
    ist.clear(std::ios_base::goodbit);
    //////////
    if(!(field_value = read_field_value(in, "angleEndpoint")))
    {
        KJB_THROW_2(Illegal_argument,"Missing angle endpoint");
    }
    ist.str(field_value);
    ist >> angle_endpt_(0) >> angle_endpt_(1) >> angle_endpt_(2);
    if(ist.fail())
    {
        KJB_THROW_2(Illegal_argument, "Missing angle endpoint");
    }
    ist.clear(std::ios_base::goodbit);
}

void Cylinder_section::write(std::ostream & out) const
{
    out << "radius: " << r_ << '\n'
        << "centertop: " << p1_(0) << ' ' << p1_(1) << ' ' << p1_(2) << '\n'
        << "centerbottom: " << p2_(0) << ' ' << p2_(1) << ' ' << p2_(2) << '\n'
        << "angle: " << angle_ << '\n'
        << "angleStartpoint: " << angle_startpt_(0) << ' ' << angle_startpt_(1) << ' ' << angle_startpt_(2) << '\n'
        << "angleEndpoint: " << angle_endpt_(0) << ' ' << angle_endpt_(1) << ' ' << angle_endpt_(2) << '\n';
}
