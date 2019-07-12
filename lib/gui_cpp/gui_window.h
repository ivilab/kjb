/* $Id: gui_window.h 18278 2014-11-25 01:42:10Z ksimek $ */
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

#ifndef KJB_CPP_GUI_WINDOW_H
#define KJB_CPP_GUI_WINDOW_H

#ifdef KJB_HAVE_OPENGL

#include <gui_cpp/gui_overlay.h>
#include <gui_cpp/gui_button.h>
#include <gui_cpp/gui_event_listener.h>
#include <boost/shared_ptr.hpp>
#include <boost/function.hpp>

namespace kjb
{
namespace gui
{

/**
 * A window overlay that implements resizing, dragable titlebar, and a close button
 * Window may contain anything that implements the Overlay interface; to be interactable, it must
 * implement the Interactive_overlay interface.
 */
class Window : public Interactive_overlay
{
typedef Window Self;
typedef Interactive_overlay Base;

private:
    // pass this to boost::shared_ptr's constructor to prevent destruction of
    // unner object
    struct null_deleter
    {
        void operator()(void const*) const
        { }
    };

    class Close_button : public Abstract_button
    {
        public:
            Close_button(int x, int y, int width, int height, const boost::function0<void>& callback);

            virtual void render_down() const;
            virtual void render_up() const
            {
                render_down();
            }

        private:
            void render_circle_() const;
            void render_plus_() const;
    };

public:
    Window(
            int x,
            int y,
            int width,
            int height);

    virtual void render() const;

// ATTACH METHODS
    /**
     * attach an interactive overlay by pointer
     */
    void attach(Interactive_overlay* overlay);

    /**
     * attach an interactive overlay by pointer
     */
    void attach(boost::shared_ptr<Interactive_overlay> overlay);

    /**
     * attach a non-interactive overlay by pointer
     */
    void attach(Overlay* overlay);

    /**
     * attach a non-interactive overlay by shared pointer
     */
    void attach(boost::shared_ptr<Overlay> overlay);

    /**
     * Set callback to call when click button is pressed
     */
    void set_close_callback(const boost::function0<void>& callback);

    /**
     * Called after this window is clicked.  Basically, any time this window
     * consumes a down-click event, this callback will fire.  This is separate from
     * the window's click handlers, which are called before this callback occurs.
     * This callback is used so window manager can re-arrange window order 
     * when a window is clicked.
     */
    void set_clicked_callback(const boost::function0<void>& callback);

// EVENT HANDLERS
    /**
     * handle mouse click.  clicks outside window will be ignored
     */
    bool mouse_event(int button, int state, int x, int y);

    bool mouse_double_event(int button, int state, int x, int y);

    bool motion_event(int x, int y);

    bool passive_motion_event(int x, int y);

    bool keyboard_event(unsigned int key, int x, int y);

    bool special_event(int key, int x, int y);

    void claim_focus();

    void yield_focus();

// WINDOW MODIFIERS

    /// Overrides base class's set_size() method to additionally resize 
    /// the inner overlay.
    virtual void set_size(int width, int height);

    /// Overrides base class's set_position() method to additionally set
    /// position of inner overlay.
    virtual void set_position(int x, int y);

    void redisplay();
    /** 
     * disable this window and hide it.  If a close callback
     * was provided during construction, it is called at the end
     * of this function
     */
    void close();

    void hide();

    void show();

    void hide_titlebar();

    void show_titlebar();

private:
    /// @brief dispatch that handles mouse click logic
    bool mouse_dispatch_(int button, int state, int x, int y);

    bool cursor_on_left_border(int cursor_x, int cursor_y) const;

    bool cursor_on_right_border(int cursor_x, int cursor_y) const;

    bool cursor_on_bottom_border(int cursor_x, int cursor_y) const;

    bool cursor_on_top_border(int cursor_x, int cursor_y) const;

    bool has_focus() const;


// SPECIALIZED CLICK FUNCTIONS
    /**
     * Attempt to handle click/release as a title bar click
     * @pre x, y is inside window
     */
#ifdef KJB_HAVE_GLUT
    bool title_bar_click_(int button, int state, int x, int y);
#endif

    /**
     * Attempt to handle click/release as a resize event.
     * @pre x, y is inside window
     */
#ifdef KJB_HAVE_GLUT
    bool resize_click_(int button, int state, int x, int y);
#endif

// SPECIALIZED DRAG FUNCTIONS
    /**
     * Respond to cursor motion by moving the window.
     *
     * @pre x, y is inside window
     */
    void drag_move_(int cursor_x, int cursor_y);

    /**
     * Respond to cursor motion by resizing the window.
     *
     * @pre x, y is inside window
     */
    void drag_resize_(int cursor_x, int cursor_y);

// GEOMETRY HELPERS
    /**
     * return true if point lies inside window (including border pixels)
     */
    inline bool point_inside_window_(int pt_x, int pt_y)
    {
        if( pt_x < x() ) return false;
        if( pt_x >= x() + width() ) return false;
        if( pt_y < y() ) return false;
        if( pt_y >= y() + height() ) return false;
        return true;
    }

    /**
     * Update size and position of inner overlay, relative to this window's
     * size.
     */
    void update_overlay_geometry_();

    void update_close_button_geometry_();

// WINDOW DECORATION RENDERING
    void render_background_() const;

    void render_border_() const;

    void render_title_bar_() const;

private:

    enum Click_state {click_none, click_resize, click_move};
    enum Resize_state {resize_none, resize_left, resize_right, resize_bottom, resize_top};

private:

    bool visible_;
    bool listening_;
    bool has_focus_;
    boost::function0<void> clicked_callback_;

    Close_button close_button_;
    boost::function0<void> close_callback_;

    boost::shared_ptr<Overlay> overlay_;
    boost::shared_ptr<Event_listener> listener_;

    Click_state click_state_;
    Resize_state x_resize_state_;
    Resize_state y_resize_state_;
    int move_offset_x_;
    int move_offset_y_;

    bool special_cursor_; // are we currently using a non-default cursor?

    bool show_titlebar_;
    static const int TITLE_BAR_HEIGHT;
    static const int BORDER_WIDTH;
    static const int BORDER_SLOP;
    static const int MIN_WINDOW_HEIGHT;
    static const int MIN_WINDOW_WIDTH;
    static const int PADDING;
    static const int CLOSE_BUTTON_HEIGHT;
    static const int CLOSE_BUTTON_WIDTH;
    static const int CLOSE_BUTTON_X_OFFSET;
    static const int CLOSE_BUTTON_Y_OFFSET;
};

}
}

#endif /* have_opengl */

#endif
