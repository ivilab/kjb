/* $Id: g_cylinder.h 13673 2013-01-29 23:17:18Z elh $ */
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
 |  Author:  Kyle Simek
 * ===========================================================================*/

// vim: tabstop=4 shiftwidth=4 foldmethod=marker

#ifndef KJB_G_CPP_CYLINDER_H
#define KJB_G_CPP_CYLINDER_H

#include <m_cpp/m_vector.h>
#include <l_cpp/l_readable.h>
#include <l_cpp/l_writeable.h>

namespace kjb
{
// Geometric cylinder class.  For renderable cylinder, see kjb::opengl::Cylinder
class Cylinder : public Readable, public Writeable
{
public:
    Cylinder(const Vector& p1, const Vector& p2, double radius) :
        Readable(),
        Writeable(),
        p1_(p1),
        p2_(p2),
        r_(radius)
    {
        if(p1_.size() != 3  || p2_.size() != 3)
        {
            KJB_THROW_2(Illegal_argument, 
                        "Cylinder() both points must be 3-dimensional.");
        }
    }

    Cylinder(const char * filename) 
    : Readable(), Writeable(), p1_(3, 0.0), p2_(3, 0.0)
    {
        read(filename);
    }
    
    Cylinder(std::istream & in) 
    : Readable(), Writeable(), p1_(3, 0.0), p2_(3, 0.0)
    {
        read(in);
    }

    /** @brief Copies a Cylinder into this one. */
    virtual Cylinder& operator= (const Cylinder& c);

    virtual void read(std::istream & in);

    virtual void read(const char * filename)
    {
        Readable::read(filename);
    }

    virtual void write(std::ostream & out) const;

    virtual void write(const char * filename) const
    {
        Writeable::write(filename);
    }

    const Vector& get_p1() const {return p1_;}
    const Vector& get_p2() const {return p2_;}
    double get_radius() const { return r_; }

// alternative parameterization: length, width, direction
    double get_length() const { return norm2(p1_ - p2_); }
    double get_width() const { return 2 * r_; }

    // Get direction vector from p1 to p2.
    Vector get_direction() const { return (p2_ - p1_).normalize(); }

//private:
protected:
    Vector p1_;
    Vector p2_;
    double r_;
};


} // namespace kjb

#endif
