/* $Id: gui_window.cpp 18278 2014-11-25 01:42:10Z ksimek $ */
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

#ifdef KJB_HAVE_OPENGL

#include <gui_cpp/gui_window.h>
#include <gr_cpp/gr_opengl.h>
#include <gr_cpp/gr_opengl_headers.h>
#include <boost/bind.hpp>

namespace kjb
{
namespace gui
{

const int Window::TITLE_BAR_HEIGHT = 20;
const int Window::BORDER_WIDTH = 1;
const int Window::BORDER_SLOP = 3;
const int Window::MIN_WINDOW_HEIGHT = TITLE_BAR_HEIGHT + BORDER_WIDTH * 2;
const int Window::MIN_WINDOW_WIDTH = BORDER_WIDTH * 2 + CLOSE_BUTTON_WIDTH + PADDING * 2;
const int Window::PADDING = 4;
const int Window::CLOSE_BUTTON_HEIGHT = TITLE_BAR_HEIGHT - PADDING*2;
const int Window::CLOSE_BUTTON_WIDTH = Window::CLOSE_BUTTON_HEIGHT;
const int Window::CLOSE_BUTTON_X_OFFSET = PADDING;
const int Window::CLOSE_BUTTON_Y_OFFSET = PADDING;

Window::Close_button::Close_button(int x, int y, int width, int height, const boost::function0<void>& callback) :
    Abstract_button(x, y, width, height, callback)
{}


void Window::Close_button::render_down() const
{
    glPushMatrix();
    glTranslatef(x(), y(), 0.0);

    glScalef(this->width()/2.0, this->height()/2.0, 1.0);
    glTranslatef(1.0, 1.0, 0.0);

    glColor3f(1.0, 0.3, 0.3);
    render_circle_();
    glColor3f(1.0, 1.0, 1.0);
    glScalef(0.7, 0.7, 1.0); // make x slightly smaller than circle
    glRotatef(45, 0, 0, 1.0);
    render_plus_();
    glPopMatrix();
}


void Window::Close_button::render_circle_() const
{
    static const float t_incr = M_PI / 10;

    glBegin(GL_TRIANGLE_FAN);
    glVertex2f(0.0, 0.0);
    for(float t = 0; t <= 2 * M_PI + t_incr; t += t_incr)
    {
        glVertex2f(cos(t), sin(t));
    }
    glEnd();
}

void Window::Close_button::render_plus_() const
{
    glBegin(GL_QUADS);
    glVertex2f( 0.2, 1.0);
    glVertex2f(-0.2, 1.0);
    glVertex2f(-0.2, -1.0);
    glVertex2f( 0.2, -1.0);

    glVertex2f(  1.0,  0.2);
    glVertex2f( -1.0,  0.2);
    glVertex2f( -1.0, -0.2);
    glVertex2f(  1.0, -0.2);
    glEnd();
}

Window::Window(
        int x,
        int y,
        int width,
        int height) :
    Interactive_overlay(x, y, width, height),
    visible_(true),
    listening_(true),
    has_focus_(true),
    clicked_callback_(),
    close_button_(
            0, 0, 1, 1, // junk, will fix in update_button_geometry_()
            boost::bind(&Self::close, this)),
    close_callback_(),
    overlay_(),
    listener_(),
    click_state_(click_none),
    x_resize_state_(resize_none),
    y_resize_state_(resize_none),
    move_offset_x_(),
    move_offset_y_(),
    special_cursor_(false),
    show_titlebar_(true)
{
    update_close_button_geometry_();
}


void Window::render() const
{
    if(!visible_) return;

    GLboolean old_lighting_state;
    glGetBooleanv(GL_LIGHTING, &old_lighting_state);

    if(old_lighting_state)
        glDisable(GL_LIGHTING);

    render_background_();
    render_border_();

    if(show_titlebar_)
    {
        render_title_bar_();
        close_button_.render();
    }

    if(old_lighting_state)
        glEnable(GL_LIGHTING);

    if(!overlay_) return;

    glPushAttrib(GL_SCISSOR_BIT);
    glEnable(GL_SCISSOR_TEST);
    glScissor(overlay_->x(), overlay_->y(), overlay_->width(), overlay_->height());
    overlay_->render();
    glPopAttrib();
}

void Window::attach(Interactive_overlay* overlay)
{
    overlay_ = boost::shared_ptr<Overlay>(overlay, null_deleter());
    listener_ = boost::shared_ptr<Event_listener>(overlay, null_deleter());
    update_overlay_geometry_();
}

void Window::attach(boost::shared_ptr<Interactive_overlay> overlay)
{
    overlay_ = overlay;
    listener_ = overlay;
    update_overlay_geometry_();
}

void Window::attach(Overlay* overlay)
{
    overlay_ = boost::shared_ptr<Overlay>(overlay, null_deleter());
    listener_.reset(); // set to null
    update_overlay_geometry_();
}

void Window::attach(boost::shared_ptr<Overlay> overlay)
{
    overlay_ = overlay;
    listener_.reset(); // set to null
    update_overlay_geometry_();
}

void Window::set_close_callback(const boost::function0<void>& callback)
{
    close_callback_ = callback;
}

void Window::set_clicked_callback(const boost::function0<void>& callback)
{
    clicked_callback_ = callback;
}

/**
 * Handle mouse events.  If a down-click event is consumed by the window the 
 * clicked_callback will also be called if specified
 */
bool Window::mouse_event(int button, int state, int x, int y)
{
#ifdef KJB_HAVE_GLUT
    bool consumed = mouse_dispatch_(button, state, x, y);

    if(consumed)
    {
        if(clicked_callback_)
            clicked_callback_();
        return true;
    }
    else
        return false;
#else
    KJB_THROW_2(Missing_dependency, "glut");
#endif
}

bool Window::mouse_double_event(int button, int state, int x, int y)
{
#ifdef KJB_HAVE_GLUT

    if(!point_inside_window_(x, y))
        return false;

    if(listener_) 
    {
        // TODO: handle clipping
        if(listener_->mouse_double_event(button, state, x, y))
            return true;
    }

    return true;
#else
    KJB_THROW_2(Missing_dependency, "glut");
#endif
}


bool Window::motion_event(int x, int y)
{
#ifdef KJB_HAVE_GLUT
    // TODO: handle clipping
    if(click_state_ == click_move)
    {
        drag_move_(x, y);
        return true;
    }
    else if(click_state_ == click_resize)
    {
        drag_resize_(x, y);
        return true;
    }
    else
    {
        if(!listener_) return false;

        return listener_->motion_event(x, y);
    }

#else
    KJB_THROW_2(Missing_dependency, "glut");
#endif
}

bool Window::passive_motion_event(int x, int y)
{
#ifdef KJB_HAVE_GLUT
    // set resize cursors if necessary
    if(cursor_on_left_border(x,y))
    {
        if(cursor_on_bottom_border(x,y))
        {
            glutSetCursor(GLUT_CURSOR_BOTTOM_LEFT_CORNER);
        }
        else if(cursor_on_top_border(x,y))
        {
            glutSetCursor(GLUT_CURSOR_TOP_LEFT_CORNER);
        }
        else
        {
            glutSetCursor(GLUT_CURSOR_LEFT_RIGHT);
        }
        special_cursor_ = true;
    }
    else if(cursor_on_right_border(x,y))
    {
        if(cursor_on_bottom_border(x,y))
        {
            glutSetCursor(GLUT_CURSOR_BOTTOM_RIGHT_CORNER);
        }
        else if(cursor_on_top_border(x,y))
        {
            glutSetCursor(GLUT_CURSOR_TOP_RIGHT_CORNER);
        }
        else
        {
            glutSetCursor(GLUT_CURSOR_LEFT_RIGHT);
        }
        special_cursor_ = true;
    } 
    else if(cursor_on_bottom_border(x,y) || cursor_on_top_border(x,y))
    {
        glutSetCursor(GLUT_CURSOR_UP_DOWN);
        special_cursor_ = true;
    }
    else if(special_cursor_)
    {
        glutSetCursor(GLUT_CURSOR_INHERIT);
        special_cursor_ = false;
    }

    if(!point_inside_window_(x, y))
        return false;

    if(listener_)
    {
        // TODO: handle clipping
        return listener_->passive_motion_event(x, y);
    }

    // don't let hover events fall through to lower layers
    return true;

#else
    KJB_THROW_2(Missing_dependency, "glut");
#endif
}

bool Window::keyboard_event(unsigned int key, int x, int y)
{
#ifdef KJB_HAVE_GLUT
    if(!listener_) return false;

    // TODO: handle focus
    if(this->has_focus())
        return listener_->keyboard_event(key, x, y);
    return false;

#else
    KJB_THROW_2(Missing_dependency, "glut");
#endif
}

bool Window::special_event(int key, int x, int y)
{
#ifdef KJB_HAVE_GLUT
    if(!listener_) return false;

    // TODO: handle focus
    if(this->has_focus())
        return listener_->special_event(key, x, y);
    return false;

#else
    KJB_THROW_2(Missing_dependency, "glut");
#endif
}




void Window::claim_focus()
{
    has_focus_ = true;
}

void Window::yield_focus()
{
    has_focus_ = false;
}

void Window::set_size(int width, int height)
{
    Base::set_size(width, height);
    update_close_button_geometry_();
    update_overlay_geometry_();
}

void Window::set_position(int x, int y)
{
    Base::set_position(x, y);
    update_close_button_geometry_();
    update_overlay_geometry_();
}

void Window::redisplay()
{
#ifdef KJB_HAVE_GLUT
    glutPostRedisplay();
#endif
}

void Window::hide()
{
    visible_ = false;
    redisplay();
}

void Window::show()
{
    visible_ = true;
    redisplay();
}

void Window::hide_titlebar()
{
    if(!show_titlebar_) return;
    show_titlebar_ = false;
    update_overlay_geometry_();
}

void Window::show_titlebar()
{
    if(show_titlebar_) return;
    show_titlebar_ = true;
    update_overlay_geometry_();
}

bool Window::cursor_on_left_border(int cursor_x, int cursor_y) const
{
    return (fabs(x() - cursor_x) <= BORDER_SLOP) && 
           y() <= cursor_y && cursor_y < y() + height()
           ;
}

bool Window::cursor_on_right_border(int cursor_x, int cursor_y) const
{
    const int right = x() + width() - 1;
    return (fabs(cursor_x - right) <= BORDER_SLOP) && 
           y() <= cursor_y && cursor_y < y() + height()
           ;
}

bool Window::cursor_on_bottom_border(int cursor_x, int cursor_y) const
{
    return (fabs(y() - cursor_y) <= BORDER_SLOP) && 
           x() <= cursor_x && cursor_x < x() + width()
           ;
}

bool Window::cursor_on_top_border(int cursor_x, int cursor_y) const
{
    const int top = y() + height() - 1;
    return (fabs(cursor_y - top) <= BORDER_SLOP) && 
           x() <= cursor_x && cursor_x < x() + width()
           ;
}

bool Window::has_focus() const
{
    // TODO: do this for real
    return has_focus_;
}

void Window::drag_move_(int cursor_x, int cursor_y)
{
    assert(click_state_ == click_move);
    set_position(cursor_x + move_offset_x_, cursor_y + move_offset_y_);

    redisplay();
}

void Window::update_overlay_geometry_()
{
    if(!overlay_) return;

    const int overlay_width = width() - BORDER_WIDTH * 2;
    const int overlay_height = height() - BORDER_WIDTH * 2 - (show_titlebar_ ? TITLE_BAR_HEIGHT : 0);

    overlay_->set_size(overlay_width, overlay_height);

    overlay_->set_position(x() + BORDER_WIDTH, y() + BORDER_WIDTH);
}

void Window::update_close_button_geometry_()
{
    close_button_.set_position(
        x() + BORDER_WIDTH + CLOSE_BUTTON_X_OFFSET,
        y() + height() - TITLE_BAR_HEIGHT - BORDER_WIDTH + CLOSE_BUTTON_Y_OFFSET );

    close_button_.set_size(
        CLOSE_BUTTON_WIDTH,
        CLOSE_BUTTON_HEIGHT);

}

void Window::render_background_() const
{
    glColor3f(1.0, 1.0, 1.0);
    glBegin(GL_QUADS);
        glVertex2f(x(), y());
        glVertex2f(x() + width(), y());
        glVertex2f(x() + width(), y() + height());
        glVertex2f(x(), y() + height());
    glEnd();
}

void Window::render_border_() const
{
    glColor3f(0.1, 0.1, 0.1);
    glBegin(GL_QUADS);
        // bottom line
        glVertex2i(x(), y());
        glVertex2i(x() + width(), y());
        glVertex2i(x() + width(), y() + BORDER_WIDTH);
        glVertex2i(x(), y() + BORDER_WIDTH);

        // top line
        glVertex2i(x(), y() + height() - BORDER_WIDTH);
        glVertex2i(x() + width(), y() + height() - BORDER_WIDTH);
        glVertex2i(x() + width(), y() + height());
        glVertex2i(x(), y() + height());

        // left line
        glVertex2i(x(), y());
        glVertex2i(x() + BORDER_WIDTH, y());
        glVertex2i(x() + BORDER_WIDTH, y() + height());
        glVertex2i(x(), y() + height());

        // right line
        glVertex2i(x() + width(), y());
        glVertex2i(x() + width(), y() + height());
        glVertex2i(x() + width() - BORDER_WIDTH, y() + height());
        glVertex2i(x() + width() - BORDER_WIDTH, y());
    glEnd();
}



bool Window::mouse_dispatch_(int button, int state, int x, int y)
{
#ifdef KJB_HAVE_GLUT
    if(resize_click_(button, state, x, y))
        return true;


    // don't send down-clicks to window if they occur outside
    // of the window.  However, up-clicks need to pass through so 
    // drags can be released, etc.
    //
    // Note that resize clicks are an exception, because the "slop"
    // margin allows clicks slightly outside the window to be accepted
    if(state == GLUT_DOWN && !point_inside_window_(x, y))
        return false;

    if(close_button_.mouse_event(button, state, x, y))
        return true;

    if(title_bar_click_(button, state, x, y))
        return true;


    // TODO: handle clipping
    if(listener_) 
    {
        if(listener_->mouse_event(button, state, x, y))
            return true;
    }

    if(state == GLUT_DOWN)
    {
        // don't let down-click events "fall through" to underneath
        return true;
    }

    return false;
#else
    KJB_THROW_2(Missing_dependency, "glut");
#endif
}


void Window::close() 
{
    visible_ = false;
    overlay_.reset();
    listener_.reset();
    clicked_callback_.clear();
    if(close_callback_)
        close_callback_();

#ifdef KJB_HAVE_GLUT
    glutPostRedisplay();
#endif
}


#ifdef KJB_HAVE_GLUT
bool Window::title_bar_click_(int button, int state, int cursor_x, int cursor_y)
{
    if(!show_titlebar_)
        return false;

    if(!button == GLUT_LEFT_BUTTON)
        return false;

    // are we releasing the title bar?
    if(state == GLUT_UP)
    {
        if(click_state_ == click_move)
        {
            click_state_ = click_none;
            return true;
        }
        else
        {
            return false;
        }
    }

    assert(state == GLUT_DOWN);
    assert(click_state_ == click_none);
    // are we clicking the title bar?
    if(cursor_y <  y() + height() - BORDER_WIDTH &&
       cursor_y >= y() + height() - BORDER_WIDTH - TITLE_BAR_HEIGHT)
    {
        click_state_ = click_move;
        move_offset_x_ = x() - cursor_x;
        move_offset_y_ = y() - cursor_y;
        return true;
    }

    return false;
}
#endif


#ifdef KJB_HAVE_GLUT
bool Window::resize_click_(int button, int state, int cursor_x, int cursor_y)
{
    if(button != GLUT_LEFT_BUTTON)
        return false;

    if(state == GLUT_UP)
    {
        if(click_state_ == click_resize)
        {
            click_state_ = click_none;
            x_resize_state_ = resize_none;
            y_resize_state_ = resize_none;
            return true;
        }
        else
        {
            return false;
        }
    }

    assert(state == GLUT_DOWN);
    assert(click_state_ == click_none);

    // left button down

    bool consumed = false;

    if(cursor_on_left_border(cursor_x, cursor_y))
    {
        click_state_ = click_resize;
        x_resize_state_ = resize_left;
        consumed = true;
    }
    else if(cursor_on_right_border(cursor_x, cursor_y))
    {
        click_state_ = click_resize;
        x_resize_state_ = resize_right;
        consumed = true;
    }
    else
    {
        x_resize_state_ = resize_none;
    }

    if(cursor_on_bottom_border(cursor_x, cursor_y))
    {
        click_state_ = click_resize;
        y_resize_state_ = resize_bottom;
        consumed = true;
    }
    else if(cursor_on_top_border(cursor_x, cursor_y))
    {
        click_state_ = click_resize;
        y_resize_state_ = resize_top;
        consumed = true;
    }
    else
    {
        y_resize_state_ = resize_none;
    }

    return consumed;
}
#endif


void Window::drag_resize_(int cursor_x, int cursor_y)
{
    assert(click_state_ = click_resize);
    assert(y_resize_state_ != resize_none || x_resize_state_ != resize_none);

    int left = x();
    int bottom = y();
    int top = bottom + height();
    int right = left + width();

    switch(y_resize_state_)
    {
        case resize_bottom:
        {
            int height = std::max(MIN_WINDOW_HEIGHT, top - cursor_y);
            bottom = top - height;
            set_position(left, bottom);
            set_height(height);
            break;
        }
        case resize_top:
        {
            int height = std::max(MIN_WINDOW_HEIGHT, cursor_y - bottom);
            top = bottom + height;
            set_height(height);
            break;
        }
        case resize_none:
            break;
        case resize_right:
        case resize_left:
        default:
            abort();
            break;
    }

    switch(x_resize_state_)
    {
        case resize_left:
        {
            int width = std::max(MIN_WINDOW_WIDTH, right - cursor_x);
            left = right - width;
            set_position(left, bottom);
            set_width(width);
            break;
        }
        case resize_right:
        {
            int width= std::max(MIN_WINDOW_WIDTH, cursor_x - left);
            right = left + width;
            set_width(width);
            break;
        }
        case resize_none:
            break;
        case resize_top:
        case resize_bottom:
        default:
            abort();
            break;
    }

#ifdef KJB_HAVE_GLUT
    glutPostRedisplay();
#endif
}


void Window::render_title_bar_() const
{
    // TODO: title
    glColor3f(0.8, 0.8, 0.8);
    glBegin(GL_QUADS);
        glVertex2i(
            x() + BORDER_WIDTH,
            y() + height() - BORDER_WIDTH);
        glVertex2i(
            x() + BORDER_WIDTH,
            y() + height() - TITLE_BAR_HEIGHT - BORDER_WIDTH);
        glVertex2i(
            x() + width() - BORDER_WIDTH,
            y() + height() - TITLE_BAR_HEIGHT - BORDER_WIDTH);
        glVertex2i(
            x() + width() - BORDER_WIDTH,
            y() + height() - BORDER_WIDTH);
        glVertex2i(
            x() + BORDER_WIDTH,
            y() + height() - BORDER_WIDTH);
        glVertex2i(
            x() + BORDER_WIDTH,
            y() + height() - BORDER_WIDTH);
    glEnd();
}

}
}

#endif /* have opengl */
