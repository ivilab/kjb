/* $Id: gr_2D_bounding_box.cpp 21596 2017-07-30 23:33:36Z kobus $ */
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
   |  Author:  Luca Del Pero
 * =========================================================================== */

#include "l_cpp/l_exception.h"
#include "l_cpp/l_util.h"
#include "i_cpp/i_image.h"
#include "gr_cpp/gr_camera.h"
#include "gr_cpp/gr_opengl.h"
#include "gr_cpp/gr_opengl_headers.h"
#include "gr_cpp/gr_2D_bounding_box.h"
#include "gr_cpp/gr_line_segment.h"

#include <iostream>
#include <cmath>
#include <iomanip>

using std::cout;
using std::endl;

namespace kjb{

void Bounding_Box2D::wire_render() const
{
#ifdef KJB_HAVE_OPENGL
    glPushAttrib(GL_ENABLE_BIT);
    glDisable(GL_LIGHTING);
    glBegin(GL_LINE_STRIP);
        ::kjb::opengl::glVertex(m_center + Vector().set(-m_width / 2.0, -m_height / 2.0));
        ::kjb::opengl::glVertex(m_center + Vector().set(-m_width / 2.0, m_height / 2.0));
        ::kjb::opengl::glVertex(m_center + Vector().set(m_width / 2.0, m_height / 2.0));
        ::kjb::opengl::glVertex(m_center + Vector().set(m_width / 2.0, -m_height / 2.0));
        ::kjb::opengl::glVertex(m_center + Vector().set(-m_width / 2.0, -m_height / 2.0));
    glEnd();
    glPopAttrib();

    GL_ETX();
#else
    KJB_THROW_2(Missing_dependency, "opengl");
#endif
}

void Bounding_Box2D::draw(kjb::Image & img, double ir, double ig, double ib, double iwidth) const
{
    kjb_c::Pixel px;
    px.r = ir;
    px.g = ig;
    px.b = ib;
/*    img.draw_aa_rectangle_outline(
        get_top(),      ///< Index of the first filled row of the rectangle
        get_left(),      ///< Index of the first filled col. of rectangle
        get_bottom(),       ///< Index of the last filled row of the rectangle
        get_right(),       ///< Index of the last filled col. of the rectangle
        px        ///< Pixel (color) with which to fill
    );*/

    Line_segment ls;
    if(fabs(m_width) > FLT_EPSILON)
    {
        ls.init_from_end_points(m_center(0) - (double)m_width/2.0,
                                m_center(1) - (double)m_height/2.0,
                                m_center(0) + (double)m_width/2.0,
                                m_center(1) - (double)m_height/2.0);
        ls.draw(img, ir, ig, ib, iwidth);

        ls.init_from_end_points(m_center(0) - (double)m_width/2.0,
                                m_center(1) + (double)m_height/2.0,
                                m_center(0) + (double)m_width/2.0,
                                m_center(1) + (double)m_height/2.0);
        ls.draw(img, ir, ig, ib, iwidth);
    }

    if(fabs(m_height) > FLT_EPSILON)
    {
        ls.init_from_end_points(m_center(0) - (double)m_width/2.0,
                                m_center(1) - (double)m_height/2.0,
                                m_center(0) - (double)m_width/2.0,
                                m_center(1) + (double)m_height/2.0);
        ls.draw(img, ir, ig, ib, iwidth);

        ls.init_from_end_points(m_center(0) + (double)m_width/2.0,
                                m_center(1) - (double)m_height/2.0,
                                m_center(0) + (double)m_width/2.0,
                                m_center(1) + (double)m_height/2.0);
        ls.draw(img, ir, ig, ib, iwidth);
    }
}

void Bounding_Box2D::write_corners_on(std::ostream& ofs)
{
    ofs << get_top_left() << ' ' << get_bottom_right();
}

void scale(kjb::Axis_aligned_rectangle_2d& box, const kjb::Vector& s)
{
    kjb::Vector c = box.get_center();
    double width = box.get_width();
    double height = box.get_height();

    c[0] *= s[0];
    width *= std::fabs(s[0]);


    c[1] *= s[1];
    height *= std::fabs(s[1]);

    box.set_center(c);
    box.set_width(width);
    box.set_height(height);
}

/**
 * Projects a set of 3D points onto the image plane,
 * and finds a bounding box (aligned with the image axes),
 * such that it contains all the projected points
 *
 * @param bb will contain the computed bounding box
 * @param points the set of 3D points to project. They can be in homogeneous coordinates
 * @param camera the camera to use when projecting
 * @param img_width the width of the image plane in pixels
 * @param img_height the height of the image plane in pixels
 */
void get_projected_bbox_from_3Dpoints
(
    Bounding_Box2D & bb,
    const std::vector<Vector> & points,
    const Base_gl_interface & camera,
    double img_width,
    double img_height
)
{
    bool found = false;
    double up = 0.0;
    double down = 0.0;
    double left = 0.0;
    double right = 0.0;
    double x = 0.0;
    double y = 0.0;
    double z = 0.0;

    if(points.size() == 0)
    {
        KJB_THROW_2(Illegal_argument,"Get projected bounding box, no input points");
    }

    camera.prepare_for_rendering(true);

    for(unsigned int i = 0; i < points.size(); i++)
    {
        camera.project_point(x, y, z,points[i], img_height);
        if((z < 0.0) || (z > 1.0))
        {
            continue;
        }
        if(!found)
        {
            left = x;
            right = x;
            up = y;
            down = y;
            found = true;
        }
        else
        {
            if(x < left)
            {
                left = x;
            }
            if(x > right)
            {
                right = x;
            }
            if(y < up)
            {
                up = y;
            }
              if(y > down)
            {
                down = y;
            }
        }
    }

    if(!found)
    {
        std::cout << "Bounding box outside the image, object is not visible" << std::endl;
        KJB_THROW_2(KJB_error,"Bounding box outside the image, object is not visible");
    }

    if( (right < 0.0) || (left > (img_width - 1)) || (down < 0.0) || (up > (img_height - 1) ) )
    {
        KJB_THROW_2(KJB_error,"Bounding box outside the image");
    }
    if(left < 0.0)
    {
        left = 0.0;
    }
    if(right > (img_width - 1.0))
    {
        right = img_width - 1.0;
    }
    if(up < 0.0)
    {
        up = 0.0;
    }
    if(down > (img_height - 1.0))
    {
        down = img_height - 1.0;
    }


    bb.set_centre_x( (right + left)/2.0 );
    bb.set_centre_y( (up + down)/2.0 );
    bb.set_width(fabs(right - left));
    bb.set_height(fabs(up - down));
}

std::ostream& operator<<(std::ostream& ost, const Axis_aligned_rectangle_2d& box)
{
    std::streamsize w = ost.width();
    std::streamsize p = ost.precision();
    std::ios::fmtflags f = ost.flags();

    ost << std::scientific;
    ost << box.m_center 
        << std::setw(16) << std::setprecision(8) << box.m_width 
        << std::setw(16) << std::setprecision(8) << box.m_height; 

    ost.width( w );
    ost.precision( p );
    ost.flags( f );
    return ost;
}

std::istream& operator>>(std::istream& ist, Axis_aligned_rectangle_2d& box)
{
    Vector center(2);
    ist >> center[0];
    ist >> center[1];

    box.set_center(center);
    ist >> box.m_width;
    ist >> box.m_height;

    return ist;
}


Bounding_Box2D intersect(const Bounding_Box2D& b1, const Bounding_Box2D& b2)
{
    KJB_THROW(Not_implemented); // test me!!
    KJB(UNTESTED_CODE());

   double left   = 0;
   double right  = 0;
   double bottom = 0;
   double top    = 0;


   left = MAX(b1.get_left(), b2.get_left());
   right = MIN(b1.get_right(), b2.get_right());
   bottom = MAX(b1.get_bottom(), b2.get_bottom());
   top = MIN(b1.get_top(), b2.get_top());

   return Bounding_Box2D(Vector(left, top), Vector(right, bottom));
}

double get_rectangle_intersection(const kjb::Bounding_Box2D& b1, const kjb::Bounding_Box2D& b2)
{
    double overlap1 = Line_segment::get_overlap(b1.get_left(), b1.get_right(), b2.get_left(), b2.get_right());
    double overlap2 = 0.0;

    if(b1.get_top() <= b1.get_bottom())
    {
        overlap2 = Line_segment::get_overlap(b1.get_top(), b1.get_bottom(), b2.get_top(), b2.get_bottom());
    }
    else
    {
        overlap2 = Line_segment::get_overlap(b1.get_bottom(), b1.get_top(), b2.get_bottom(), b2.get_top());
    }
    return overlap1*overlap2;
}


} // namespace kjb
