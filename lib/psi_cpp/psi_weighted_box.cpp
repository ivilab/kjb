/* $Id: psi_weighted_box.cpp 18331 2014-12-02 04:30:55Z ksimek $ */
/* {{{=========================================================================== *
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
 * =========================================================================== }}}*/

// vim: tabstop=4 shiftwidth=4 foldmethod=marker

#include <psi_cpp/psi_weighted_box.h>
#include <gr_cpp/gr_opengl.h>
#include <gr_cpp/gr_opengl_headers.h>

namespace kjb
{
namespace psi
{

std::vector<Vector> get_corners(const Cuboid& c)
{
    const Vector& center = c.get_center();
    const Vector& dims = c.get_size();
    const Quaternion& orientation = c.get_orientation();

    std::vector<Vector> corners;

    Vector tmp = dims / 2;
    for(int x = -1; x <= 1; x+= 2)
    for(int y = -1; y <= 1; y+= 2)
    for(int z = -1; z <= 1; z+= 2)
    {
        Vector delta = tmp;
        delta[0] *= x;
        delta[1] *= y;
        delta[2] *= z;
        delta = orientation.rotate(delta);
        
        corners.push_back(center + delta);
    }

    return corners;
}

void render(const Cuboid& c)
{
#ifdef KJB_HAVE_OPENGL
    using namespace kjb::opengl;
    const Vector& center = c.get_center();
    const Vector& size = c.get_size();

//    double x = center[0];
//    double y = center[1];
//    double z = center[2];

    glPushMatrix();
    glTranslate(center);
    glRotate(c.get_orientation());
    glScalef(size[0], size[1], size[2]);
    // xy-faces
    glBegin(GL_QUADS);
    glVertex3f( - 0.5,  - 0.5,    0.5);
    glVertex3f(   0.5,  - 0.5,    0.5);
    glVertex3f(   0.5,    0.5,    0.5);
    glVertex3f( - 0.5,    0.5,    0.5);
    glVertex3f( - 0.5,  - 0.5,    0.5);
    glEnd();

    glBegin(GL_QUADS);
    glVertex3f( - 0.5,  - 0.5,  - 0.5);
    glVertex3f( - 0.5,    0.5,  - 0.5);
    glVertex3f(   0.5,    0.5,  - 0.5);
    glVertex3f(   0.5,  - 0.5,  - 0.5);
    glVertex3f( - 0.5,  - 0.5,  - 0.5);
    glEnd();

    // xz-faces
    glBegin(GL_QUADS);
    glVertex3f( - 0.5,   0.5,  - 0.5);
    glVertex3f( - 0.5,   0.5,    0.5);
    glVertex3f(   0.5,   0.5,    0.5);
    glVertex3f(   0.5,   0.5,  - 0.5);
    glVertex3f( - 0.5,   0.5,  - 0.5);
    glEnd();

    glBegin(GL_QUADS);
    glVertex3f( - 0.5, - 0.5,  - 0.5);
    glVertex3f(   0.5, - 0.5,  - 0.5);
    glVertex3f(   0.5, - 0.5,    0.5);
    glVertex3f( - 0.5, - 0.5,    0.5);
    glVertex3f( - 0.5, - 0.5,  - 0.5);
    glEnd();

    // yz-faces
    glBegin(GL_QUADS);
    glVertex3f(   0.5, - 0.5,  - 0.5);
    glVertex3f(   0.5,   0.5,  - 0.5);
    glVertex3f(   0.5,   0.5,    0.5);
    glVertex3f(   0.5, - 0.5,    0.5);
    glVertex3f(   0.5, - 0.5,  - 0.5);
    glEnd();

    glBegin(GL_QUADS);
    glVertex3f( - 0.5, - 0.5,  - 0.5);
    glVertex3f( - 0.5, - 0.5,    0.5);
    glVertex3f( - 0.5,   0.5,    0.5);
    glVertex3f( - 0.5,   0.5,  - 0.5);
    glVertex3f( - 0.5, - 0.5,  - 0.5);
    glEnd();

    glPopMatrix();
#else
    KJB_THROW_2(Missing_dependency, "opengl");
#endif
}

} // namespace psi
} // namespace kjb
