/* $Id: gui_overlay.h 18278 2014-11-25 01:42:10Z ksimek $ */
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

#ifndef KJB_CPP_GUI_OVERLAY_H
#define KJB_CPP_GUI_OVERLAY_H

#ifdef KJB_HAVE_OPENGL

#include <boost/function.hpp>
#include <gr_cpp/gr_renderable.h>
#include <gui_cpp/gui_event_listener.h>

namespace kjb
{
namespace gui
{
/**
 * Translatable, resizable 2d renderable overlay.
 *
 * Assumptions:
 *    Depth-checking won't be done; objects should be rendered back-to-front
 *    All 2d coordinates in render function are relative to the bottom-left
 *        of this object, and have pixel scale.
 *    Accessor functions for the overlay dimensions will always give screen coordinates.
 *    Origin is at bottom-left.
 */
class Overlay : public kjb::Renderable
{
public:
    Overlay();
    Overlay(int x, int y, int width, int height);

    virtual void render() const = 0;

    void set_width(int width);

    void set_height(int height);

    virtual void set_size(int width, int height);

    virtual void set_position(int x, int y);

    virtual int width() const { return width_; }
    virtual int height() const { return height_; }

    virtual int x() const { return x_pos_; }
    virtual int y() const { return y_pos_; }
private:
    int x_pos_, y_pos_;
    int width_, height_;
};

class Interactive_overlay : public Overlay, public Event_listener
{
public:
    Interactive_overlay();

    Interactive_overlay(int x, int y, int width, int height);
};

/**
 * Enables treatinga callback function as an overlay object.
 *
 * Callback function should render in 2D (in the XY plane, with z = 0).
 * Objects rendered in callback will have origin at their bottom left.
 *
 * This overlay is translatable.  If constructed using the first constructor, scaling
 * this overlay will have no effect on the rendering.  Using the second construtor
 * will set the modelview matrix so rendering will be scaled appropriately when this
 * overlay is resized
 */
class Overlay_callback_wrapper : public Overlay
{
public:
    Overlay_callback_wrapper(const boost::function0<void>& callback);

    Overlay_callback_wrapper(int x, int y, int width, int height, const boost::function0<void>& callback);

    virtual void render() const;

private:
    boost::function0<void> cb_;
    bool enable_resizing_;
    float orig_width_;
    float orig_height_;
};

} // namespace gui
} //namespace kjb

#endif /* KJB_HAVE_OPENGL */

#endif
