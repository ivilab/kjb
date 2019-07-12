/* $Id: gui_button.h 18278 2014-11-25 01:42:10Z ksimek $ */
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

#ifndef KJB_CPP_GUI_BUTTON_H
#define KJB_CPP_GUI_BUTTON_H

#ifdef KJB_HAVE_OPENGL

#include <gui_cpp/gui_overlay.h>
#include <boost/function.hpp>

namespace kjb
{
namespace gui
{

/**
 * An abstract button that implements standard click semantics.
 *
 * Subclass to implement render_up and render_down, the draw
 * methods for the unclicked and clicked button, respectively.
 */
class Abstract_button : public Overlay
{
typedef Overlay Base;

public:
    Abstract_button(int x, int y, int width, int height, const boost::function0<void>& callback);

    /** render a white rectangle with black border */
    virtual void render() const;

    virtual void render_up() const = 0;
    virtual void render_down() const = 0;

    void redisplay();

    bool motion_event(int cursor_x, int cursor_y);

    bool mouse_event(int button, int state, int cursor_x, int cursor_y);

    void set_callback(const boost::function0<void>& callback);

protected:
    bool inside_(int cursor_x, int cursor_y);

private:
    boost::function0<void> click_callback_;
    bool clicked_;
    bool over_;
};

/**
 * A simple black and white button
 */
class Simple_button : public Abstract_button
{
typedef Abstract_button Base;

public:
    Simple_button(int x, int y, int width, int height, const boost::function0<void>& callback);

    virtual void render_up() const
    {
        render_(false);
    }

    virtual void render_down() const
    {
        render_(true);
    }

protected:
    void render_(bool down) const;
};

}
}

#endif /* have_opengl */
#endif
