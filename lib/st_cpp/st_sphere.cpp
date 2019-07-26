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
|     Jinyan Guan 
|
* =========================================================================== */

#include <st_cpp/st_sphere.h>

#include <iostream>
#include <sstream>
#include <typeinfo>

using namespace kjb;

Parametric_sphere::Parametric_sphere
(
    double center_x, 
    double center_y, 
    double center_z, 
    double radius
) throw (kjb::Illegal_argument) : Renderable_model(), Readable(), Writeable()
    //rendering_interface(center_x, center_y,  center_z, radius), m_center_v(4.0, 1.0)
{
    if(radius <= 0.0)
    {
        throw kjb::Illegal_argument("Sphere constructor, dimensions must be positive");
    }
    m_center_v(0) = center_x;
    m_center_v(1) = center_y;
    m_center_v(2) = center_z;
    m_radius = radius;
}

Parametric_sphere::Parametric_sphere
(
    const char* fname
) throw (kjb::Illegal_argument, kjb::IO_error): Renderable_model(false)
    //rendering_interface(0.0, 0.0, 0.0, 1.0), m_center_v(4.0, 1.0)
{
    Readable::read(fname);
}

Parametric_sphere::Parametric_sphere
(
    std::istream& in
) throw(kjb::Illegal_argument, kjb::IO_error): Renderable_model(true), 
    //rendering_interface(0.0, 0.0, 0.0, 1.0),
    m_center_v(4,1.0)
{
    Readable::read(in);
}

//read in a sphere from an istream 
void Parametric_sphere::read(std::istream& in) throw(kjb::Illegal_argument, kjb::IO_error)
{
    using std::ostringstream;
    using std::istringstream;

    const char* type_name = typeid(*this).name();
    const char* field_value;

    // Read Type 
    if (!(field_value = read_field_value(in, "Type")))
    {
        throw Illegal_argument("Missing Type field");
    }
    if (strncmp(field_value, type_name, strlen(type_name)) != 0)
    {
        ostringstream ost;
        ost << "Tried to read a '" << field_value << "' as a '"
            << type_name << "'";
        throw Illegal_argument(ost.str());
    }

    // Read center
    if (!(field_value = read_field_value(in, "center")))
    {
        throw Illegal_argument("Missing center");
    }
    istringstream ist(field_value);
    ist >> m_center_v[0] >> m_center_v[1] >> m_center_v[2];
    ist.clear(std::ios_base::goodbit);

    // Read radius
    if (!(field_value = read_field_value(in, "radius")))
    {
        throw Illegal_argument("Missing radius");
    }
    ist.str(field_value);
    ist >> m_radius;
    if (ist.fail() || (m_radius <= 0.0) )
    {
        throw Illegal_argument("Invalid radius");
    }
    ist.clear(std::ios_base::goodbit);

}

/**
 * @brief Save the Sphere parameters into a file 
 */
void Parametric_sphere::write(std::ostream& out) const throw (kjb::IO_error)
{
    out<< " Type: "<<typeid(*this).name() <<"\n"
        <<" center: "<< m_center_v [0] << " "
                     <<m_center_v(1) <<" "
                     <<m_center_v(2) <<"\n"
        <<" radius: "<< m_radius << "\n";
}

/*Abstract_renderable & Parametric_sphere::get_rendering_interface() const
{
    return rendering_interface;
}*/
