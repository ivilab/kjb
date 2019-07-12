/* $Id: g_cylinder.cpp 18278 2014-11-25 01:42:10Z ksimek $ */
/* ===========================================================================*
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
 |  Author:  Emily Hartley
 * ===========================================================================*/

// vim: tabstop=4 shiftwidth=4 foldmethod=marker


#include <g_cpp/g_cylinder.h>
#include <sstream>

using namespace kjb;

/**
 * Performs a deep copy of the center points of the bases of the 
 * cylinder and of the radius.
 *
 * @param  c  Cylinder to copy into this one.
 *
 * @return A reference to this cylinder.
 */
Cylinder& Cylinder::operator= (const Cylinder& c)
{
    p1_ = c.p1_;
    p2_ = c.p2_;
    r_ = c.r_;

    return *this;
}


void Cylinder::read(std::istream & in)
{
    using std::ostringstream;
    using std::istringstream;

    const char * field_value;

    if(!(field_value = read_field_value(in, "radius")))
    {
        KJB_THROW_2(Illegal_argument,"Missing radius");
    }
    istringstream ist(field_value);
    ist >> r_;
    if(r_ < 0)
    {
        KJB_THROW_2(Illegal_argument, "Radius must be bigger than 0");
    }
    if(ist.fail())
    {
        KJB_THROW_2(Illegal_argument, "Missing radius");
    }
    ist.clear(std::ios_base::goodbit);
    
    if(!(field_value = read_field_value(in, "centertop")))
    {
        KJB_THROW_2(Illegal_argument,"Missing center top");
    }
    ist.str(field_value);
    ist >> p1_(0) >> p1_(1) >> p1_(2);
    if(ist.fail())
    {
        KJB_THROW_2(Illegal_argument, "Missing center top");
    }
    ist.clear(std::ios_base::goodbit);

    if(!(field_value = read_field_value(in, "centerbottom")))
    {
        KJB_THROW_2(Illegal_argument,"Missing center bottom");
    }
    ist.str(field_value);
    ist >> p2_(0) >> p2_(1) >> p2_(2);
    if(ist.fail())
    {
        KJB_THROW_2(Illegal_argument, "Missing center bottom");
    }
    ist.clear(std::ios_base::goodbit);
}

void Cylinder::write(std::ostream & out) const
{
    out << "radius: " << r_ << '\n'
        << "centertop: " << p1_(0) << ' ' << p1_(1) << ' ' << p1_(2) << '\n'
        << "centerbottom: " << p2_(0) << ' ' << p2_(1) << ' ' << p2_(2) << '\n';
}
