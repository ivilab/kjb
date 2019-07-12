/* $Id: gr_spline.h 18278 2014-11-25 01:42:10Z ksimek $ */
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

#ifndef KJB_gr_spline_H
#define KJB_gr_spline_H

#include <m2_cpp/m2_spline.h>
#include <gr_cpp/gr_renderable.h>
#include <gr_cpp/gr_opengl_headers.h>
#include <l_cpp/l_cloneable.h>
#include <m_cpp/m_vector.h>
#include <vector>

#ifdef KJB_HAVE_OPENGL


namespace kjb {
namespace opengl {
class Opengl_nurbs_object
{
public:
    Opengl_nurbs_object();
    Opengl_nurbs_object(GLUnurbsObj* nobj);
    Opengl_nurbs_object(const Opengl_nurbs_object& src);
    virtual ~Opengl_nurbs_object();

    Opengl_nurbs_object& operator=(const Opengl_nurbs_object& nobj);

    void set(GLUnurbsObj* nobj);
    GLUnurbsObj* get() const { return _my_nobj; }
protected:
    static GLvoid _nurbs_error (GLenum errorCode);
private:
    static GLUnurbsObj* _nobj;
    static int _nobj_ref_count;

    mutable GLUnurbsObj* _my_nobj;
};

class Opengl_nurbs_callable :
    public Opengl_callable,
    public virtual Cloneable
{
public:
    Opengl_nurbs_callable(GLenum type = 0);
    Opengl_nurbs_callable(const Opengl_nurbs_callable& src);
    virtual ~Opengl_nurbs_callable();

    virtual Opengl_nurbs_callable& operator=(const Opengl_nurbs_callable& src);
    virtual Opengl_nurbs_callable* clone() const = 0;

    /** @brief Use alternative nurbs renderer */
    void set_nurbs_object(GLUnurbsObj* nobj);

    GLUnurbsObj* get_nurbs_object() const;

    virtual void gl_call() const = 0;
protected:
    mutable Opengl_nurbs_object _nobj;

    GLenum _type;
};

class Nurbs_trim_curve
{
public:
    virtual void trim() const = 0;
};

class Gl_nurbs_curve : public Nurbs_curve, 
    public Opengl_nurbs_callable,
    public Nurbs_trim_curve,
    public Generic_renderable
{
public:
    Gl_nurbs_curve();
    Gl_nurbs_curve(
            uint num_knots,
            const float* knots,
            uint degree,
            std::vector<Vector> ctl_points,
            GLenum type);
    Gl_nurbs_curve(const Gl_nurbs_curve& src);
    virtual ~Gl_nurbs_curve(){}

    Gl_nurbs_curve& operator=(const Gl_nurbs_curve& src);
    Gl_nurbs_curve* clone() const;

    virtual void gl_call() const;

    virtual void solid_render() const {gl_call();}

    virtual void trim() const {gl_call();}
};


class Gl_nurbs_surface : 
    public Nurbs_surface,
    public Opengl_nurbs_callable,
    public Generic_renderable
{
public:
    Gl_nurbs_surface();
    Gl_nurbs_surface(
            uint num_knots_s,
            const float* knots_s,
            uint num_knots_t,
            const float* knots_t,
            uint degree_s,
            uint degree_t,
            const std::vector<std::vector<Vector> >& ctl_points,
            GLenum type = GL_MAP2_VERTEX_3);

    Gl_nurbs_surface(const Gl_nurbs_surface& src);
    virtual ~Gl_nurbs_surface(){}

    void init(std::vector<float>& pts, const std::vector<std::vector<kjb::Vector> >& ctl_points);

    Gl_nurbs_surface& operator=(const Gl_nurbs_surface& src);
    Gl_nurbs_surface* clone() const;

    virtual void gl_call() const;

    virtual void wire_render() const;
    virtual void solid_render() const {gl_call();}

protected:
    mutable std::vector< float > _ctl_points_flat;
};

class Gl_bezier_curve : 
    public Bezier_curve,
    public Opengl_nurbs_callable,
    public Generic_renderable
{
public:
    Gl_bezier_curve(int degree = 3, int dimension = 3,  GLenum type = GL_MAP1_VERTEX_3);
    Gl_bezier_curve(const Gl_bezier_curve& src);
    virtual ~Gl_bezier_curve() {}

    Gl_bezier_curve& operator=(const Gl_bezier_curve& src);
    Gl_bezier_curve* clone() const;

    Gl_nurbs_curve to_gl_nurbs() const;

    virtual void gl_call() const;

    virtual void solid_render() const {gl_call();}
};

/** 
 * Adds opengl functionality to Polybezier curves
 */
class Gl_polybezier_curve : 
    public Polybezier_curve,
    public Opengl_nurbs_callable,
    public Generic_renderable
{
public:
    Gl_polybezier_curve(int dimension = 3, GLenum type = GL_MAP1_VERTEX_3);
    Gl_polybezier_curve(const Gl_polybezier_curve& src);
    virtual ~Gl_polybezier_curve() {}

    Gl_polybezier_curve& operator=(const Gl_polybezier_curve& src);
    virtual Gl_polybezier_curve* clone() const {return new Gl_polybezier_curve(*this);}

    virtual Gl_nurbs_curve to_gl_nurbs() const;

    virtual void gl_call() const;

    virtual void solid_render() const {gl_call();}
};
} // namespace opengl
} // namespace kjb

#endif
#endif /* ----- #ifndef KJB_gr_spline_H  ----- */
