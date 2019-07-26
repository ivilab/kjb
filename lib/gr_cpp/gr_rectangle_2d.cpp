#include "gr_cpp/gr_rectangle_2d.h"
#include "gr_cpp/gr_opengl.h"
#include "gr_cpp/gr_opengl_headers.h"

namespace kjb {

std::vector<Vector> Rectangle_2d::get_corners() const
{
    std::vector<Vector> corners(4);
    corners[0].set(-m_width / 2.0, -m_height / 2.0);
    corners[1].set(-m_width / 2.0, m_height / 2.0);
    corners[2].set(m_width / 2.0, m_height / 2.0);
    corners[3].set(m_width / 2.0, -m_height / 2.0);

    Matrix R = ::kjb::geometry::get_rotation_matrix(m_orientation);
    for(size_t i = 0; i < corners.size(); i++)
    {
        corners[i] = m_center + R * (corners[i]);
    }

    return corners;
}

std::vector<Vector> Rectangle_2d::get_side_midpoints() const
{
    std::vector<Vector> midpoints(4);
    midpoints[0].set(m_width / 2.0, 0.0);
    midpoints[1].set(0.0, -m_height / 2.0);
    midpoints[2].set(-m_width / 2.0, 0.0);
    midpoints[3].set(0.0, m_height / 2.0);

    Matrix R = ::kjb::geometry::get_rotation_matrix(m_orientation);
    for(size_t i = 0; i < midpoints.size(); i++)
    {
        midpoints[i] = m_center + R * (midpoints[i]);
    }

    return midpoints;
}

void Rectangle_2d::wire_render() const
{
#ifdef KJB_HAVE_OPENGL
    std::vector<Vector> corners = get_corners();
    glPushAttrib(GL_ENABLE_BIT);
    glDisable(GL_LIGHTING);
    glBegin(GL_LINE_STRIP);
        ::kjb::opengl::glVertex(corners[0]);
        ::kjb::opengl::glVertex(corners[1]);
        ::kjb::opengl::glVertex(corners[2]);
        ::kjb::opengl::glVertex(corners[3]);
        ::kjb::opengl::glVertex(corners[0]);
    glEnd();
    glPopAttrib();

    GL_ETX();
#else
    KJB_THROW_2(Missing_dependency, "opengl");
#endif
}

} //namespace kjb

