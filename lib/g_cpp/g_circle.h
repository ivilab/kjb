/* $Id$ */
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

#ifndef KJB_CIRCLE_H
#define KJB_CIRCLE_H

#include <m_cpp/m_vector.h>
#include <m_cpp/m_matrix.h>
#include <m/m_incl.h>
#include <i_cpp/i_image.h>
#include <i/i_draw.h>
#include <l_cpp/l_readable.h>
#include <l_cpp/l_writeable.h>

namespace kjb {

class Circle{
public:
    double GetRadius();
    const kjb::Vector & GetCenter();
    Circle(kjb::Vector * p1, kjb::Vector* p2, kjb::Vector* p3);
    //Contructor for radius & center
    Circle(Matrix* points);
    Circle(const std::vector<kjb::Vector> & ipoints);
    Circle(kjb::Vector* center, double radius);
    Circle();
    double CalcCircle(Vector* pt1, Vector* pt2, Vector* pt3);
    bool IsPerpendicular(Vector* pt1, Vector* pt2, Vector* pt3);
    Image draw_circle(Matrix* points_to_show);

private:    
    Circle computeCircleGivenPoints(Matrix* mp);
    Circle computeCircleGivenPoints(const std::vector<kjb::Vector> & ipoints);
    double radius;
    Vector center;
};

// Constructs a circle in 3d space which is defined by its center, its radius, 
// and the normal vector to the plane that the circle lies in.
class Circle_in_3d : public Readable, public Writeable
{
public:
    Circle_in_3d
    (
        const Vector& center, 
        const double  radius, 
        const Vector& normal
    ) : Readable(), 
        Writeable(), 
        circle_center(center),
        circle_radius(radius),
        circle_normal(normal)
    {
        if(circle_center.size() != 3)
        {
            KJB_THROW_2(Illegal_argument,
                        "The center point must be a 3d coordinate.");
        }

        if(circle_radius <= 0)
        {
            KJB_THROW_2(Illegal_argument,
                        "The radius must be a positive value.");
        }

        if(circle_normal.size() != 3)
        {
            KJB_THROW_2(Illegal_argument,
                        "The normal vector must have size 3.");
        }
    }

    Circle_in_3d(const char * filename)
    : Readable(), Writeable(), circle_center(3, 0.0), circle_normal(3, 0.0)
    {
        read(filename);
    }

    Circle_in_3d(std::istream & in)
    : Readable(), Writeable(), circle_center(3, 0.0), circle_normal(3, 0.0)
    {
        read(in);
    }

    virtual Circle_in_3d& operator= (const Circle_in_3d& c);

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

    const Vector& get_circle_center() const { return circle_center; }
    double get_circle_radius() const { return circle_radius; }
    const Vector& get_circle_normal() const { return circle_normal; }

protected:
    Vector circle_center;
    double circle_radius;
    Vector circle_normal;
};

} // namespace kjb
#endif
