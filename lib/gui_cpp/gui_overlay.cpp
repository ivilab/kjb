/* $Id: gui_overlay.cpp 18283 2014-11-25 05:05:59Z ksimek $ */
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

#include <gui_cpp/gui_overlay.h>
#include <gr_cpp/gr_opengl.h>
#include <gr_cpp/gr_opengl_headers.h>

namespace kjb {
namespace gui {
Overlay::Overlay() :
    Renderable(),
    x_pos_(),
    y_pos_(),
    width_(),
    height_()
{}
Overlay::Overlay(int x, int y, int width, int height) :
    Renderable(),
    x_pos_(x),
    y_pos_(y),
    width_(width),
    height_(height)
{}

void Overlay::set_width(int width)
{
    set_size(width, height_);
}

void Overlay::set_height(int height)
{
    set_size(width_, height);
}

void Overlay::set_size(int width, int height)
{
    width_ = width;
    height_ = height;
}

void Overlay::set_position(int x, int y)
{
    x_pos_ = x;
    y_pos_ = y;
}

Interactive_overlay::Interactive_overlay() :
    Overlay(),
    Event_listener()
{}

Interactive_overlay::Interactive_overlay(int x, int y, int width, int height) :
    Overlay(x, y, width, height),
    Event_listener()
{}

Overlay_callback_wrapper::Overlay_callback_wrapper(const boost::function0<void>& callback) :
    Overlay(0,0,100,100), // think more on this
    cb_(callback),
    enable_resizing_(false),
    orig_width_(),
    orig_height_()
{ }

Overlay_callback_wrapper::Overlay_callback_wrapper(int x, int y, int width, int height, const boost::function0<void>& callback) :
    Overlay(x,y,width,height),
    cb_(callback),
    enable_resizing_(true),
    orig_width_(width),
    orig_height_(height)
{ }

void Overlay_callback_wrapper::render() const
{
    glPushMatrix();

    glTranslatef(x(), y(), 0);

    if(enable_resizing_)
    {
        glScalef(width() / orig_width_, height() / orig_height_, 1.0);
    }

    cb_();

    glPopMatrix();
}

} // namespace gui 
} // namespace kjb 

#endif // KJB_HAVE_OPENGL
