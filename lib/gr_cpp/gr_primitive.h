/* $Id: gr_primitive.h 21599 2017-07-31 00:44:30Z kobus $ */
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
#ifndef KJB_CPP_PRIMITIVE_H
#define KJB_CPP_PRIMITIVE_H

#ifdef KJB_HAVE_OPENGL

/**
 * @file gr_primitive.h
 * Object-oriented interface for simple 3D primitives.
 */

#include "l/l_sys_debug.h"  /* For ASSERT */
#include "gr_cpp/gr_renderable.h"
#include "m_cpp/m_vector.h"
#include "gr_cpp/gr_opengl_headers.h"

namespace kjb 
{
namespace opengl 
{

/**
 * Base class for glu quadric types (sphere, cylinder, cone, etc).
 */
class Quadric : 
    public kjb::Generic_renderable
{
public:
    Quadric(unsigned int slices = DEFAULT_SLICES, unsigned int rings = DEFAULT_RINGS);

    Quadric(const Quadric& src);

    Quadric& operator=(const Quadric& other);

    void swap(Quadric&);

    virtual ~Quadric();

    void wire_render() const;

    void solid_render() const
    {
        if(_mesh_dirty) _update_mesh();

        _generic_render_in_opengl(_solid_list_id);
    }
protected:

    void _init_quadric();
    void _generic_render_in_opengl(unsigned int list_id) const;

    static void _quad_error_callback(unsigned int error_code);

    void _update_mesh() const;

    virtual void _render() const = 0;
protected:
    // we only store one glu quad object for all quadrics
    static GLUquadricObj* _quad;
    static unsigned int _num_instances;

    mutable unsigned int _solid_list_id;
    mutable unsigned int _wire_list_id;

    mutable bool _mesh_dirty;

    static const unsigned int DEFAULT_SLICES = 30;
    static const unsigned int DEFAULT_RINGS = 10;

    unsigned int  _slices;
    unsigned int  _rings;
};

/**
 * Cylinder centered around the z-axis.  Thin wrapper for gluCylinder
 */
class Cylinder : public Quadric
{
public:
    Cylinder(
            float base_radius = 1,
            float top_radius = 1,
            float height = 1,
            unsigned int slices = DEFAULT_SLICES,
            
            unsigned int rings = DEFAULT_RINGS) :
        Quadric(slices, rings),
        _base_r(base_radius),
        _top_r(top_radius),
        _height(height)
    {
        _update_mesh();
    }

    Cylinder(const Cylinder& src) :
        Quadric(src),
        _base_r(src._base_r),
        _top_r(src._top_r),
        _height(src._height)
    {
        _update_mesh();
    }

    Cylinder& operator=(const Cylinder& other);

    void swap(Cylinder&);

    virtual ~Cylinder(){};

    void set_base_radius(float r) {_base_r = r; _mesh_dirty = true;}
    void set_top_radius(float r) {_top_r = r; _mesh_dirty = true;}
    void set_height(float h) {_height = h; _mesh_dirty = true;}
private:
    float _base_r;
    float _top_r;
    float _height;

    virtual void _render() const;
};

/**
 * A cylinder whose dimensions cannot change over it's lifetime.  This
 * is significantly faster to render than a standard kjb::Cylinder, because
 * it uses OpenGL display lists.
 */
class Static_cylinder :
    public kjb::Generic_renderable
{
public:
    Static_cylinder(
            float base_radius = 1,
            float top_radius = 1,
            float height = 1,
            unsigned int slices = DEFAULT_SLICES,
            
            unsigned int rings = DEFAULT_RINGS);

    void wire_render() const;

    void solid_render() const;

private:
    unsigned int wire_index_;
    unsigned int solid_index_;

    static const unsigned int DEFAULT_SLICES = 30;
    static const unsigned int DEFAULT_RINGS = 10;
};

/**
 * Sphere centered at the origin.  For ellipsoids, use glScale*().
 */
class Sphere : public Quadric
{
public:
    Sphere(float radius = 1, unsigned int slices = DEFAULT_SLICES, unsigned int rings = DEFAULT_RINGS):
        Quadric(slices, rings),
        _r(radius)
    {
        _update_mesh();
    }


    Sphere(const Sphere& src):
        Quadric(src),
        _r(src._r)
    {
        _update_mesh();
    }

    Sphere& operator=(const Sphere& other);

    void swap(Sphere&);

    virtual ~Sphere(){};

    void set_radius(float r) 
    {
        _r = r; _mesh_dirty = true;
    }
private:

    float _r;

    virtual void _render() const;
};


class Disk : public Quadric
{
public:
    Disk(float outer_radius = 1, unsigned int slices = DEFAULT_SLICES, unsigned int rings = DEFAULT_RINGS):
        Quadric(slices, rings),
        _inner_r(0.0),
        _outer_r(outer_radius)
    {
        _update_mesh();
    }

    Disk(float inner_radius, float outer_radius, unsigned int slices = DEFAULT_SLICES, unsigned int rings = DEFAULT_RINGS):
        Quadric(slices, rings),
        _inner_r(inner_radius),
        _outer_r(outer_radius)
    {
        _update_mesh();
    }


    Disk(const Disk& src):
        Quadric(src),
        _inner_r(src._inner_r),
        _outer_r(src._outer_r)
    {
        _update_mesh();
    }

    Disk& operator=(const Disk& other);

    void swap(Disk&);

    virtual ~Disk(){};

    void set_inner_radius(float r) 
    {
        _inner_r = r; _mesh_dirty = true;
    }

    void set_outer_radius(float r) 
    {
        _outer_r = r; _mesh_dirty = true;
    }
private:

    float _inner_r;
    float _outer_r;

    virtual void _render() const;
};

/**
 * 3D arrow defined by a location and a direction.
 */
class Arrow3d : public kjb::Generic_renderable
{
public:
    Arrow3d() :
        _pos(3, 0.0),
        _length(0.01),
        _shaft_length(0.0),
        _width(1.0),
        _dir(3, 0.0),
        _shaft(),
        _head(),
        _shaft_cap(),
        _head_cap()
    {
        _dir[2] = 1.0;
    }

    Arrow3d(const kjb::Vector& position, const kjb::Vector& direction, float width = 1.0) :
        _pos(position),
        _length(direction.magnitude()),
        _shaft_length(0.0),
        _width(width),
        _dir(direction / _length),
        _shaft(),
        _head(),
        _shaft_cap(),
        _head_cap()
    { 
        double head_length = std::min(_length, 2 * _width);
        double head_width = head_length;


        _shaft_length = std::max(0.01, _length - head_length);
        _shaft = Cylinder(_width/2, _width/2, _shaft_length, 10);
        _head  = Cylinder(head_width/2, 0.0, head_length, 10);

        _shaft_cap = Disk(_width/2);
        _head_cap  = Disk(head_width/2);
    }

    void set_position(const kjb::Vector& position)
    {
        ASSERT(position.size() == 3);
        _pos = position;
    }

    void set_direction(const kjb::Vector& direction)
    {
        ASSERT(direction.size() == 3);
        _dir = direction;
    }

    void render() const 
    {
        _generic_render(Renderer());
    }

    void solid_render() const 
    {
        _generic_render(Solid_renderer());    
    }

    void wire_render() const 
    {
        _generic_render(Wire_renderer());
    }

    void wire_occlude_render() const
    {
        _generic_render(Wire_occlude_renderer());
    }

private:
    void _generic_render(const Generic_renderer& renderer) const;

    kjb::Vector _pos;
    float _length;
    float _shaft_length;
    float _width;
    kjb::Vector _dir;

    Cylinder _shaft;
    Cylinder _head;

    Disk _shaft_cap;
    Disk _head_cap;
};

class Teapot : public kjb::Generic_renderable
{
public:
    void solid_render() const;
};

/** @brief  Class that represents a renderable ellipse. */
class Ellipse : public Generic_renderable
{
public:
    /** @brief  Construct an ellipse. */
    Ellipse(double r1, double r2, size_t slices = DEFAULT_SLICES);

    /** @brief  Render this ellipse on the xy-plane. */
    void render() const;

    /** @brief  Render this ellipse on the xy-plane. */
    void solid_render() const;

    /** @brief  Wire-render this ellipse on the xy-plane. */
    void wire_render() const;

private:
    /** @brief  Helper function. */
    void render_points() const;

    double m_rad1;
    double m_rad2;
    size_t m_slices;

    static const size_t DEFAULT_SLICES = 32;
};

} // namespace opengl
} // namespace kjb
#endif
#endif
