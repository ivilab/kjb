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
#ifndef KJB_ST_SPHERE_INCLUDED
#define KJB_ST_SPHERE_INCLUDED

#include <gr_cpp/gr_sphere.h>
#include <m_cpp/m_vector.h>
#include <st_cpp/st_renderable_model.h>
#include <l_cpp/l_readable.h>
#include <l_cpp/l_writeable.h>



/**
 * @class ST_SPHERE models a 3D sphere and its graphic representation 
 */
namespace kjb
{
//class Perspective_camera;
class Parametric_sphere : public Renderable_model, public Readable, public Writeable
{
    public: 
        Parametric_sphere
        (
            double center_x, 
            double center_y, 
            double center_z, 
            double radius = 1.0
        ) throw (kjb::Illegal_argument);

        Parametric_sphere (){}
        
        /** @brief Constructs a parametric_parapiped from an input file. */
        Parametric_sphere (const char* fname) throw (kjb::Illegal_argument,
                    kjb::IO_error);

        Parametric_sphere (std::istream& in) throw (kjb::Illegal_argument, kjb::IO_error);

        Parametric_sphere (const Parametric_sphere& src);
        
        virtual Parametric_sphere & operator = (const Parametric_sphere& src);

        virtual void read(std::istream& in) throw (kjb::Illegal_argument, kjb::IO_error);

        virtual void read(const char * fname)
        {
            Readable::read(fname);
        }

        virtual void write(std::ostream& out) const throw (kjb::IO_error);
        virtual void write(const char* fname) const throw (kjb::IO_error)
        {
            Writeable::write(fname);
        }

        virtual Abstract_renderable& get_rendering_interface() const;
        virtual void update_rendering_representation() const throw(kjb::KJB_error);
        //Geodesic_sphere& get_triangular_mesh() {return rendering_interface; }
        
        virtual Parametric_sphere * clone() const {return new Parametric_sphere(*this); }
        virtual ~Parametric_sphere() {}

        /** @brief Returns the center location x */
        inline double get_center_x() const {return m_center_v[0]; }
        
        /** @brief Returns the center location y */
        inline double get_center_y() const {return m_center_v[1]; }

        /** @brief Returns the center location z */
        inline double get_center_z() const {return m_center_v[2]; }

        /** @brief Returns the center of the sphere */
        inline const Vector& get_center() const {return m_center_v;}

        /** @brief Returns the radius */ 
        inline double get_radius() const {return m_radius; }

        /** @brief Sets the center location x */
        inline void set_center_x(double x) {m_center_v[0] = x;} 
        
        /** @brief Sets the center location y */
        inline void set_center_y(double y) {m_center_v[1] = y;}

        /** @brief Sets the center location z */
        inline void set_center_z(double z) {m_center_v[2] = z;}

        /** @brief Sets the center location */
        inline void set_center(const Vector& center) {m_center_v = center; }

        /** @brief Sets the radius */ 
        inline void set_radius(double r) {m_radius = r; }
       
        /** @briefTransforms a point in world coordinates to a coordinate
         *  system where the parapiped centre is the origin, and the
         *  axes are defined by the sphere axes */
        void get_point_in_sphere_coordinates
        (
            const kjb::Vector & point_in_world_coordinates,
            kjb::Vector & point_in_sphere_coordinates
        ) const;

        /** @brief Transforms a point in parapiped coordinates to
         *  world coordinates */
        void get_point_in_world_coordinates
        (
            const kjb::Vector & point_in_sphere_coordinates,
            kjb::Vector & point_in_world_coordinates
        ) const;
        
        void propose_sphere_from_curve(); 

    protected: 
        //mutable Geodesic_sphere rendering_interface;
        Vector m_center_v; 
        double m_radius;
};
}
#endif
