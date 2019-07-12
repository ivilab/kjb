/* $Id: gr_primitive.cpp 21596 2017-07-30 23:33:36Z kobus $ */
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

//#include <new>
//#include <cstdio>
#include "l/l_sys_debug.h"  /* For ASSERT. */
#include <algorithm>
#include "m_cpp/m_vector.h"
#include "l_cpp/l_exception.h"
#include "gr_cpp/gr_opengl.h"
#include "gr_cpp/gr_opengl_headers.h"
#include "gr_cpp/gr_glut.h"
#include "g_cpp/g_quaternion.h"

#ifdef KJB_HAVE_OPENGL

#include <gr_cpp/gr_primitive.h>

using namespace kjb::opengl;

using std::bad_alloc;
using kjb::Vector;

GLUquadricObj* Quadric::_quad = 0;
unsigned int Quadric::_num_instances = 0;

Quadric::Quadric(unsigned int slices, unsigned int rings) :
    Generic_renderable(),
    _solid_list_id(0),
    _wire_list_id(0),
    _mesh_dirty(true),
    _slices(slices),
    _rings(rings)
{
    _init_quadric();
    _num_instances++;
}

Quadric::Quadric(const Quadric& src) :
    Generic_renderable(),
    _solid_list_id(0),
    _wire_list_id(0),
    _mesh_dirty(true),
    _slices(src._slices),
    _rings(src._rings)
{
    _init_quadric();
    _num_instances++;
}

Quadric& Quadric::operator=(const Quadric& src)
{
    _solid_list_id = 0;
    _wire_list_id = 0;
    _mesh_dirty = true;
    _slices = src._slices;
    _rings = src._rings;

    return *this;
}

void Quadric::swap(Quadric& other)
{
    using std::swap;
    swap(_solid_list_id, other._solid_list_id);
    swap(_wire_list_id, other._wire_list_id);
    swap(_mesh_dirty, other._mesh_dirty);
    swap(_slices, other._slices);
    swap(_rings, other._rings);
}

Quadric::~Quadric()
{
    ASSERT(_num_instances > 0);
    _num_instances--;
    if(_num_instances == 0)
    {
        gluDeleteQuadric(_quad);
        _quad = 0;
    }

    if(_solid_list_id)
    {
        glDeleteLists(_solid_list_id, 2);
        _solid_list_id = 0;
        _wire_list_id = 0;
    }

    GL_ETX();

}

void Quadric::wire_render() const
{
    if(_mesh_dirty) _update_mesh();

    glPushAttrib(GL_POLYGON_BIT);
    _generic_render_in_opengl(_wire_list_id);
    glPopAttrib();
}

void Quadric::_init_quadric()
{
    if(_num_instances == 0)
    {
        ASSERT(_quad == 0);

        _quad = gluNewQuadric();

        if(!_quad) {
            throw bad_alloc();
        }

        gluQuadricCallback(_quad, GLU_ERROR, (void(*)()) (Quadric::_quad_error_callback));
        gluQuadricOrientation(_quad, GLU_OUTSIDE);
        gluQuadricDrawStyle(_quad, GLU_FILL );
        gluQuadricNormals(_quad, GLU_SMOOTH);

        GL_ETX();

    }
}

void Quadric::_generic_render_in_opengl(GLuint list_id) const
{
    glCallList(list_id);

    GL_ETX();
}

void Quadric::_quad_error_callback(unsigned int error_code)
{
    const GLubyte *estring;

    estring = gluErrorString(error_code);

    fprintf(stderr, "Quadric Error: %s\n", estring);
    abort();
}


void Quadric::_update_mesh() const
{
    if(_solid_list_id)
    {
        glDeleteLists(_solid_list_id, 2);
        _solid_list_id = 0;
        _wire_list_id = 0;
    }

    _solid_list_id = glGenLists(2);
    _wire_list_id = _solid_list_id + 1;


    glNewList(_solid_list_id, GL_COMPILE);
        _render();
    glEndList();

    glNewList(_wire_list_id, GL_COMPILE);
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        _render();
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glEndList();

    _mesh_dirty = false;

    GL_ETX();
}


Cylinder& Cylinder::operator=(const Cylinder& src)
{
    Cylinder other(src);
    swap(other);

    return *this;
}

void Cylinder::swap(Cylinder& other)
{
    using std::swap;
    Quadric::swap(other);
    swap(_base_r, other._base_r);
    swap(_top_r, other._top_r);
    swap(_height, other._height);
}

void Cylinder::_render() const
{
    gluCylinder(_quad, _base_r, _top_r, _height, _slices, _rings);

    GL_ETX();
}

Static_cylinder::Static_cylinder(
        float base_radius,
        float top_radius,
        float height,
        unsigned int slices,
        
        unsigned int rings) :
    wire_index_(),
    solid_index_()
{
    Cylinder tmp_cylinder(base_radius, top_radius, height, slices, rings);

    solid_index_ = glGenLists(1);
    glNewList(solid_index_, GL_COMPILE);
    tmp_cylinder.solid_render();
    glEndList();

    GL_ETX();

    wire_index_ = glGenLists(1);
    glNewList(wire_index_, GL_COMPILE);
    tmp_cylinder.wire_render();
    glEndList();

    GL_ETX();
}

void Static_cylinder::wire_render() const
{
    glCallList(wire_index_);
}

void Static_cylinder::solid_render() const
{
    glCallList(solid_index_);
}

Sphere& Sphere::operator=(const Sphere& src)
{
    Sphere other(src);
    swap(other);

    return *this;
}

void Sphere::swap(Sphere& other)
{
    using std::swap;
    Quadric::swap(other);
    swap(_r, other._r);
}

void Sphere::_render() const
{
    gluSphere(_quad, _r, _slices, _rings);

    GL_ETX();
}

Disk& Disk::operator=(const Disk& src)
{
    Disk other(src);
    swap(other);

    return *this;
}

void Disk::swap(Disk& other)
{
    using std::swap;
    Quadric::swap(other);
    swap(_inner_r, other._inner_r);
    swap(_outer_r, other._outer_r);
}

void Disk::_render() const
{
    gluDisk(_quad, _inner_r, _outer_r, _slices, _rings);

    GL_ETX();
}



void Arrow3d::_generic_render(const Generic_renderer& renderer) const
{
    using namespace kjb::opengl;
    using namespace kjb;

    Vector up(3, 0.0); up[2] = 1.0;

    ASSERT(fabs(_dir.magnitude() - 1.0) < FLT_EPSILON);
    Quaternion rotation;
    rotation.set_from_directions(up, _dir);

    glPushMatrix();
    glTranslate(_pos);
    glRotate(rotation);
    renderer(_shaft_cap);
    renderer(_shaft);
    glTranslatef(0, 0, _shaft_length);
//    glRotatef(180, 1.0, 0., 0.);
    renderer(_head_cap);
    renderer(_head);
    glPopMatrix();

    GL_ETX();

    // since tip of head is at end of arrow, 
    // make it "point" in reverse direction of shaft,
    // so tip points in shaft direction
//    rotation.set_from_cross_product(cross(up, -_dir));
//    glPushMatrix();
//    glRotate(rotation);
//    glPopMatrix();
}

void Teapot::solid_render() const 
{
#ifdef KJB_HAVE_GLUT
   float mat_diffuse[] = { 0.8f, 0.8f, 0.8f, 1.0f };
   glMaterialfv (GL_FRONT_AND_BACK, GL_DIFFUSE, mat_diffuse);
   glutSolidTeapot (1.0);

    GL_ETX();
#else
   KJB_THROW_2(Missing_dependency,"GLUT");
#endif
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */
/** @brief  Construct an ellipse. */
Ellipse::Ellipse(double r1, double r2, size_t slices) :
    m_rad1(r1), m_rad2(r2), m_slices(slices) {}

/** @brief  Render this ellipse on the xy-plane. */
void Ellipse::render() const
{
    solid_render();
}

/** @brief  Render this ellipse on the xy-plane. */
void Ellipse::solid_render() const
{
    glBegin(GL_POLYGON);
    render_points();
    glEnd();
}

/** @brief  Wire-render this ellipse on the xy-plane. */
void Ellipse::wire_render() const
{
    glBegin(GL_LINE_LOOP);
    render_points();
    glEnd();
}


void Ellipse::render_points() const
{
    const double arc_size = 2 * M_PI / m_slices;

    Vector r;
    for(size_t i = 0; i < m_slices; i++)
    {
        double angle = i * arc_size;
        r.set(m_rad1 * cos(angle), m_rad2 * sin(angle));
        glVertex(r);
    }
}

#endif /* KJB_HAVE_OPENGL */
