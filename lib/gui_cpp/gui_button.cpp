/* $Id: gui_button.cpp 18283 2014-11-25 05:05:59Z ksimek $ */
/* {{{=========================================================================== *
   |
   |  Copyright (c) 1994-2014 by Kobus Barnard (author)
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

#ifdef KJB_HAVE_OPENGL

#include <gui_cpp/gui_button.h>
#include <gr_cpp/gr_opengl.h>
#include <gr_cpp/gr_opengl_headers.h>

namespace kjb {
namespace gui {

Abstract_button::Abstract_button(int x, int y, int width, int height, const boost::function0<void>& callback) :
    Base(x, y, width, height),
    click_callback_(callback),
    clicked_(false),
    over_(false)
{}

void Abstract_button::render() const
{
    if(clicked_ && over_)
    {
        render_down();
    }
    else
    {
        render_up();
    }
}

void Abstract_button::redisplay()
{
#ifdef KJB_HAVE_GLUT
    glutPostRedisplay();
#else
KJB_THROW_2(Missing_dependency, "glut");
#endif
}

bool Abstract_button::motion_event(int cursor_x, int cursor_y)
{
    if(inside_(cursor_x, cursor_y))
    {
        if(over_ == false)
        {
            over_ = true;
            redisplay();
        }
    }
    else
    {
        if(over_ == true)
        {
            over_ = false;
            redisplay();
        }
    }

    return false;
}

bool Abstract_button::mouse_event(int button, int state, int cursor_x, int cursor_y)
{
#ifdef KJB_HAVE_GLUT
    if(button != GLUT_LEFT_BUTTON) 
        return false;

    if(state == GLUT_DOWN)
    {
        if( !inside_(cursor_x, cursor_y) )
            return false;

        clicked_ = true;
        over_ = true;

        redisplay();
        return true;
    }
    else if(clicked_ == true && GLUT_UP)
    {
        // mouse down and mouse-up must both be inside button
        if( inside_(cursor_x, cursor_y) )
              click_callback_();
        clicked_ = false;

        redisplay();
        return true;
    }

    return false;
#else
KJB_THROW_2(Missing_dependency, "glut");
#endif
}

void Abstract_button::set_callback(const boost::function0<void>& callback)
{
    click_callback_ = callback;
}

bool Abstract_button::inside_(int cursor_x, int cursor_y)
{
    return ( cursor_x >= x() && cursor_x < x() + width() &&
             cursor_y >= y() && cursor_y < y() + height() );
}

Simple_button::Simple_button(int x, int y, int width, int height, const boost::function0<void>& callback) :
    Base(x, y, width, height, callback)
{}

void Simple_button::render_(bool down) const
{
    glPushAttrib(GL_ENABLE_BIT);
    glPushMatrix();
    glTranslatef(x(), y(), 1.0);

    glLineWidth(1.0);

    float bg_color = (down ? 0.0 : 1.0);
    float line_color = (down ? 1.0 : 0.0);

    for(int i = 0; i < 2; i++)
    {
        if(i == 0)
        {
            glBegin(GL_QUADS);
            glColor3f(bg_color, bg_color, bg_color);
        }
        else
        {
            glBegin(GL_LINE_LOOP);
            glColor3f(line_color, line_color, line_color);
        }

        glVertex2f(0.0, 0.0);
        glVertex2f(width(), 0.0);
        glVertex2f(width(), height());
        glVertex2f(0.0, height());
        glEnd();
    }

    glPopMatrix();
    glPopAttrib();
}


} // namespace kjb
} // namespace gui

#endif // KJB_HAVE_OPENGL
