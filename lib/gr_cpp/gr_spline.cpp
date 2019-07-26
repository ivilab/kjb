/* $Id: gr_spline.cpp 21596 2017-07-30 23:33:36Z kobus $ */
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

#include "l/l_sys_debug.h"  /* For ASSERT. */
#include "gr_cpp/gr_opengl.h"
#include "gr_cpp/gr_spline.h"

#include <new>

#ifndef CALLBACK
#define CALLBACK
#endif

#ifdef KJB_HAVE_OPENGL

#include "gr_cpp/gr_opengl.h"

using namespace std;
using namespace kjb;
using namespace kjb::opengl;

GLUnurbsObj* Opengl_nurbs_object::_nobj = 0;
int Opengl_nurbs_object::_nobj_ref_count = 0;

Opengl_nurbs_object::Opengl_nurbs_object() :
    _my_nobj(0)
{
    if(_nobj_ref_count == 0)
    {
        _nobj = gluNewNurbsRenderer();
        if(!_nobj)
        {
            throw bad_alloc();
        }

        gluNurbsProperty(_nobj, GLU_SAMPLING_TOLERANCE, 50.0);
        gluNurbsCallback(_nobj, GLU_ERROR, (GLvoid (*) ()) (&Opengl_nurbs_object::_nurbs_error));
    }

    _nobj_ref_count++;

    _my_nobj = _nobj;
    
}

Opengl_nurbs_object::Opengl_nurbs_object(GLUnurbsObj* nobj) :
    _my_nobj(nobj)
{
    _nobj_ref_count++;
}

Opengl_nurbs_object::Opengl_nurbs_object(const Opengl_nurbs_object& src) :
    _my_nobj(src._my_nobj)
{
    _nobj_ref_count++;
}

Opengl_nurbs_object::~Opengl_nurbs_object()
{
    ASSERT(_nobj_ref_count > 0);
    _nobj_ref_count--;

    if(_nobj_ref_count == 0)
    {
        gluDeleteNurbsRenderer(_nobj);
    }
}

GLvoid CALLBACK Opengl_nurbs_object::_nurbs_error (GLenum errorCode)
{
    const GLubyte *estring;
    estring = gluErrorString(errorCode);
    fprintf(stderr, "Nurbs Error: %s\n", estring);
    abort();
}

Opengl_nurbs_object& Opengl_nurbs_object::operator=(const Opengl_nurbs_object& src)
{
    if(this == &src) return *this;

    _my_nobj = src._my_nobj;

    return *this;
}

void Opengl_nurbs_object::set(GLUnurbsObj* nobj)
{
    _my_nobj = nobj;
}

Opengl_nurbs_callable::Opengl_nurbs_callable(GLenum type) : 
    Opengl_callable(),
    Cloneable(),
    _nobj(),
    _type(type)
{
}

Opengl_nurbs_callable::Opengl_nurbs_callable(const Opengl_nurbs_callable& src) :
    Opengl_callable(src),
    Cloneable(src),
    _nobj(src._nobj),
    _type(src._type)
{

}

Opengl_nurbs_callable::~Opengl_nurbs_callable()
{}

Opengl_nurbs_callable& Opengl_nurbs_callable::operator=(const Opengl_nurbs_callable& src)
{
    if(this == &src) return *this;

    _nobj = src._nobj;
    _type = src._type;
    return *this;
}

void Opengl_nurbs_callable::set_nurbs_object(GLUnurbsObj* nobj)
{
    _nobj.set(nobj);
}

GLUnurbsObj* Opengl_nurbs_callable::get_nurbs_object() const
{
    return _nobj.get();
}

Gl_nurbs_curve::Gl_nurbs_curve() : 
    Nurbs_curve(),
    Opengl_nurbs_callable(),
    Nurbs_trim_curve(),
    Generic_renderable()
{ }

Gl_nurbs_curve::Gl_nurbs_curve(uint num_knots, const float* knots, uint degree, vector<Vector> ctl_points, GLenum type) :
    Nurbs_curve(num_knots, knots, degree, ctl_points),
    Opengl_nurbs_callable(type),
    Nurbs_trim_curve(),
    Generic_renderable()
{ }

Gl_nurbs_curve::Gl_nurbs_curve(const Gl_nurbs_curve& src) :
    Nurbs_curve(src),
    Opengl_nurbs_callable(src),
    Nurbs_trim_curve(src),
    Generic_renderable(src)
{ }

Gl_nurbs_curve& Gl_nurbs_curve::operator=(const Gl_nurbs_curve& src)
{
    if(this == &src) return *this;
    Nurbs_curve::operator=(src);
    Opengl_nurbs_callable::operator=(src);
    return *this;
}

Gl_nurbs_curve* Gl_nurbs_curve::clone() const
{
    return new Gl_nurbs_curve(*this);
}

void Gl_nurbs_curve::gl_call() const
{
    if(_type == 0)
    {
       KJB_THROW_2(Runtime_error, "Can't call nurbs curve -- not initialized.");
    }

    const int STRIDE = _ctl_points[0].get_length();

    switch(_type)
    {
        case GL_MAP1_INDEX:
        case GL_MAP1_TEXTURE_COORD_1:
            ASSERT(STRIDE == 1);
            break;
        case GL_MAP1_TEXTURE_COORD_2:
            ASSERT(STRIDE == 2);
            break;
        case GL_MAP1_VERTEX_3:
        case GL_MAP1_TEXTURE_COORD_3:
        case GL_MAP1_NORMAL:
            ASSERT(STRIDE == 3);
            break;
        case GL_MAP1_VERTEX_4:
        case GL_MAP1_TEXTURE_COORD_4:
        case GL_MAP1_COLOR_4:
            ASSERT(STRIDE == 4);
            break;
        default:
            KJB_THROW(Runtime_error);
    }

    float* ctl_points = new float[_ctl_points.size() * STRIDE];

    for(int i = 0; i < (int) _ctl_points.size(); i++)
    {
        for(int j = 0; j < STRIDE; j++)
        {
            ctl_points[i * STRIDE + j] = _ctl_points[i][j];
        }
    }

    gluNurbsCurve(_nobj.get(), _num_knots, _knots, STRIDE, ctl_points, _order, _type);

    delete[] ctl_points;
}


Gl_nurbs_surface::Gl_nurbs_surface() :
    Nurbs_surface(),
    Opengl_nurbs_callable(),
    _ctl_points_flat()
{
}


Gl_nurbs_surface::Gl_nurbs_surface(
        uint num_knots_s,
        const float* knots_s,
        uint num_knots_t,
        const float* knots_t,
        uint degree_s,
        uint degree_t,
        const vector<vector<Vector> >& ctl_points,
        GLenum type) :
    Nurbs_surface(num_knots_s, knots_s, num_knots_t, knots_t, degree_s, degree_t, ctl_points),
    Opengl_nurbs_callable(type),
    Generic_renderable(),
    _ctl_points_flat()
{
    init(_ctl_points_flat, ctl_points);
}

Gl_nurbs_surface::Gl_nurbs_surface(const Gl_nurbs_surface& src) :
    Nurbs_surface(src),
    Opengl_nurbs_callable(src),
    Generic_renderable(src),
    _ctl_points_flat(src._ctl_points_flat)
{
}

Gl_nurbs_surface& Gl_nurbs_surface::operator=(const Gl_nurbs_surface& src)
{
    if (this == &src) {
        return *this;
    }

    Nurbs_surface::operator=(src);
    Opengl_nurbs_callable::operator=(src);
    _ctl_points_flat = src._ctl_points_flat;

    return *this;
}

Gl_nurbs_surface* Gl_nurbs_surface::clone() const
{
    return new Gl_nurbs_surface(*this);
}

void Gl_nurbs_surface::init(std::vector<float>& pts, const std::vector<std::vector<kjb::Vector> >& ctl_points)
{
    const int STRIDE_T = ctl_points[0][0].get_length();
    const int STRIDE_S = ctl_points[0].size() * STRIDE_T;
    pts.resize(ctl_points.size() * STRIDE_S * STRIDE_T);

    for(size_t s = 0; s < ctl_points.size(); s++)
    {
        for(size_t t = 0; t < ctl_points[0].size(); t++)
        {
            for(int d = 0; d < ctl_points[0][0].get_length(); d++)
            {
#ifdef TEST
                pts.at(s * STRIDE_S + t * STRIDE_T + d) = ctl_points[s][t][d];
#else
                pts[s * STRIDE_S + t * STRIDE_T + d] = ctl_points[s][t][d];
#endif
            }
        }
    }
}

void Gl_nurbs_surface::wire_render() const
{
    gluNurbsProperty(_nobj.get(), GLU_DISPLAY_MODE, GLU_OUTLINE_POLYGON);
    render();
    gluNurbsProperty(_nobj.get(), GLU_DISPLAY_MODE, GLU_FILL);
}
void Gl_nurbs_surface::gl_call() const
{
    const int STRIDE_T = _ctl_points[0][0].get_length();
    const int STRIDE_S = _ctl_points[0].size() * STRIDE_T;

    if(_type == 0)
    {
       KJB_THROW_2(Runtime_error, "Can't call nurbs surface -- not initialized.");
    }


//    switch(_type)
//    {
//        case GL_MAP1_INDEX:
//        case GL_MAP1_TEXTURE_COORD_1:
//            ASSERT(STRIDE == 1);
//            break;
//        case GL_MAP1_TEXTURE_COORD_2:
//            ASSERT(STRIDE == 2);
//            break;
//        case GL_MAP1_VERTEX_3:
//        case GL_MAP1_TEXTURE_COORD_3:
//        case GL_MAP1_NORMAL:
//            ASSERT(STRIDE == 3);
//            break;
//        case GL_MAP1_VERTEX_4:
//        case GL_MAP1_TEXTURE_COORD_4:
//        case GL_MAP1_COLOR_4:
//            ASSERT(STRIDE == 4);
//            break;
//    }

    gluNurbsSurface(_nobj.get(), _num_knots_s, _knots_s, _num_knots_t, _knots_t, STRIDE_S, STRIDE_T, &_ctl_points_flat[0], _order_s, _order_t, _type);
}


Gl_bezier_curve::Gl_bezier_curve(int degree, int dimension, GLenum type) :
    Bezier_curve(degree, dimension),
    Opengl_nurbs_callable(type),
    Generic_renderable()
{
    switch(type)
    {
        case GL_MAP1_INDEX:
        case GL_MAP1_TEXTURE_COORD_1:
            if(dimension != 1)
            {
                KJB_THROW_2(Illegal_argument, "Opengl type doesn't match spline dimension.");
            }
            break;
        case GL_MAP1_TEXTURE_COORD_2:
            if(dimension != 2)
            {
                KJB_THROW_2(Illegal_argument, "Opengl type doesn't match spline dimension.");
            }
            break;
        case GL_MAP1_VERTEX_3:
        case GL_MAP1_TEXTURE_COORD_3:
        case GL_MAP1_NORMAL:
            if(dimension != 3)
            {
                KJB_THROW_2(Illegal_argument, "Opengl type doesn't match spline dimension.");
            }
            break;
        case GL_MAP1_VERTEX_4:
        case GL_MAP1_TEXTURE_COORD_4:
        case GL_MAP1_COLOR_4:
            if(dimension != 4)
            {
                KJB_THROW_2(Illegal_argument, "Opengl type doesn't match spline dimension.");
            }
            break;
        default:
            KJB_THROW(Runtime_error); // can't happen?
    }
}

Gl_bezier_curve::Gl_bezier_curve(const Gl_bezier_curve& src) :
    Bezier_curve(src),
    Opengl_nurbs_callable(src),
    Generic_renderable(src)
{ }

Gl_bezier_curve& Gl_bezier_curve::operator=(const Gl_bezier_curve& src)
{
    if(this == &src) return *this;

    Bezier_curve::operator=(src);
    Opengl_nurbs_callable::operator=(src);
    return *this;
}

Gl_bezier_curve* Gl_bezier_curve::clone() const
{
    return new Gl_bezier_curve(*this);
}

Gl_nurbs_curve Gl_bezier_curve::to_gl_nurbs() const
{
    Nurbs_curve tmp = Bezier_curve::to_nurbs();
    Gl_nurbs_curve result;
    result.Nurbs_curve::operator=(tmp);
    result.Opengl_nurbs_callable::operator=(*this);

    return result;
}

void Gl_bezier_curve::gl_call() const
{
    // TODO reimplement Gl_bezier_curve::gl_call() directly instead of converting to nurbs first
    to_gl_nurbs().gl_call();
}

Gl_polybezier_curve::Gl_polybezier_curve(int dimension, GLenum type) : 
    Polybezier_curve(dimension),
    Opengl_nurbs_callable(type),
    Generic_renderable()
{ }

Gl_polybezier_curve::Gl_polybezier_curve(const Gl_polybezier_curve& src) :
    Polybezier_curve(src),
    Opengl_nurbs_callable(src),
    Generic_renderable(src)
{

}

Gl_polybezier_curve& Gl_polybezier_curve::operator=(const Gl_polybezier_curve& src)
{
    if(this == &src) return *this;

    Polybezier_curve::operator=(src);
    Opengl_nurbs_callable::operator=(src);
    return *this;
}

Gl_nurbs_curve Gl_polybezier_curve::to_gl_nurbs() const
{
    Nurbs_curve tmp = Polybezier_curve::to_nurbs();
    Gl_nurbs_curve result;

    result.Nurbs_curve::operator=(tmp);
    result.Opengl_nurbs_callable::operator=(*this);

    return result;
}

void Gl_polybezier_curve::gl_call() const
{
    // TODO reimplement Gl_bezier_curve::gl_call() directly instead of converting to nurbs first
    to_gl_nurbs().gl_call();
}

#endif
