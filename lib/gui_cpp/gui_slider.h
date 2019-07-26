/* $Id: gui_slider.h 18034 2014-11-03 05:56:44Z ksimek $ */
/* {{{=========================================================================== *
   |
   |  Copyright (c) 1994-2012 by Kobus Barnard (author)
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
//
#ifndef KJB_CPP_GUI_SLIDER_H
#define KJB_CPP_GUI_SLIDER_H

#ifdef KJB_HAVE_OPENGL

#include <gr_cpp/gr_opengl.h>
#include <gui_cpp/gui_overlay.h>
#include <gui_cpp/gui_event_listener.h>
#include <boost/function.hpp>

namespace kjb
{
namespace gui
{

/**
 * A simple slider widget that can set/display a value between
 * two extremes.
 */
template <class Real>
class Slider : public kjb::gui::Interactive_overlay
{
typedef kjb::gui::Interactive_overlay Base;
public:
    Slider() :
        Base(),
        min_(0),
        max_(1),
        value_(0),
        callback_(),
        live_updating_(false),
        grabbed_(false)
    {
        if(value_ < min_ || value_ > max_)
            KJB_THROW_2(kjb::Illegal_argument, "initial value is out of bounds");
    }

    Slider(int x, int y,
            int width,
            Real min, Real max,
            Real value) :
        Base(x,y, width, std::max(INDICATOR_HEIGHT, BAR_HEIGHT)),
        min_(min),
        max_(max),
        value_(value),
        callback_(),
        live_updating_(false),
        grabbed_(false)
    {}

    void set_changed_callback(const boost::function1<void, Real>& callback, bool live_updating = false)
    {
        callback_ = callback;
        live_updating_ = live_updating;
    }

    virtual void render() const
    {
        // TODO: save and restore state
        glPushAttrib(GL_ENABLE_BIT);
        glPushAttrib(GL_CURRENT_BIT);
        glDisable(GL_LIGHTING);
        int bar_center = bar_center_();
        int bar_left = x();
        int bar_right = x() + width();
        int bar_bottom = bar_center - BAR_HEIGHT/2;
        int bar_top = bar_center + BAR_HEIGHT/2;

        static const kjb::Vector3 bar_color(248.0/255.0);
        static const kjb::Vector3 outline_color(221.0/255.0);
//        static const kjb::Vector3 outline_color(0);
        kjb::opengl::glColor(bar_color);

        glBegin(GL_QUADS);

        for(size_t i = 0; i < 2; ++i)
        {
            glVertex2i(bar_left, bar_bottom); 
            glVertex2i(bar_right, bar_bottom); 
            glVertex2i(bar_right, bar_top); 
            glVertex2i(bar_left, bar_top); 

            glEnd();
            if(i == 0)
            {
                kjb::opengl::glColor(outline_color);
                glBegin(GL_LINE_LOOP);
            }

        }

        // current indicator
        if(has_valid_state_())
        {
            int current_x = position_from_value_(value_);
            int indicator_left = current_x - INDICATOR_HEIGHT / 2;
            int indicator_right = current_x + INDICATOR_HEIGHT / 2;
            int indicator_bottom = bar_center - INDICATOR_HEIGHT / 2;
            int indicator_top = bar_center + INDICATOR_HEIGHT / 2;
            kjb::opengl::glColor(bar_color);
            glBegin(GL_QUADS);
            for(size_t i = 0; i < 2; ++i)
            {
                glVertex2i(indicator_left, indicator_bottom); 
                glVertex2i(indicator_right, indicator_bottom); 
                glVertex2i(indicator_right, indicator_top); 
                glVertex2i(indicator_left, indicator_top); 

                glEnd();
                if(i == 0)
                {
                    kjb::opengl::glColor(outline_color);
                    glBegin(GL_LINE_LOOP);
                }
            }
            
        }
        glPopAttrib();
        glPopAttrib();
    }

    virtual bool mouse_event(int button, int state, int u, int v)
    {
        kjb::Vector2 pos(u, v);

        if(!has_valid_state_()) return false;

        if(button != GLUT_LEFT_BUTTON) return false;
        if(u < x() || u > x() + width() || v < y() || v > y() + height()) return false;

        if(state == GLUT_DOWN)
        {
                grabbed_ = true;
//            kjb::Vector2 slider_pos(position_from_value_(value_), bar_center_());
////
//            // if down: (1) on slider?  (2) on timeline?
//            if((slider_pos - pos).magnitude() <= height() / 2.0)
//            {
//                grabbed_ = true;
//                return true;
//            }
//            else if(fabs(y - slider_pos[1]) < height() / 2.0)
//            {
//                grabbed_ = true;
//                set_value(value_from_position_(x));
//
//                // trigger "value changed" callback
//                if(callback_)
//                    callback_(get_value());
//                return true;
//            }
            redisplay();
            return true;
        }
        else if(state == GLUT_UP)
        {
            if(grabbed_)
            {
                grabbed_ = false;
                set_value(value_from_position_(u));

                // trigger "value changed" callback
                if(callback_)
                    callback_(get_value());
                redisplay();
            }
        }
        return false;
    }

    virtual bool mouse_double_event(int , int , int , int )
    {
        return false;
    }

    virtual bool motion_event(int x, int y)
    {
        if(!grabbed_) return false;

        if(!has_valid_state_()) return false;

        set_value(value_from_position_(x));

        // trigger callback if "live updates" are enabled
        if(callback_ && live_updating_)
            callback_(get_value());

        return true;
    }

    virtual bool passive_motion_event(int , int )
    {
        return false;
    }

    virtual bool keyboard_event(unsigned int , int , int )
    {
        // todo: consider moving on keys
        return false;
    }

    virtual bool special_event(int , int , int )
    {
        // todo: consider moving on keys
        return false;
    }

    void set_value(const Real& v)
    {
        value_ = v;
        redisplay();
    }

    void set_min(const Real& min)
    {
        min_ = min;
        redisplay();
    }

    void set_max(const Real& max)
    {
        max_ = max;
        redisplay();
    }

    const Real& get_value() const
    {
        return value_;
    }

    void redisplay() const
    {
#ifdef KJB_HAVE_GLUT
        glutPostRedisplay();
#endif
    }

private:
    Real value_from_position_(int pos) const
    {
        assert(has_valid_state_());

        double delta = pos - x();
        delta /= width();
        if(delta < 0.0) return min_;
        if(delta > 1.0) return max_;
        return delta * (max_ - min_) + min_;
    }

    int position_from_value_(const Real& v) const
    {
        assert(has_valid_state_());
        double delta = v - min_;
        delta /= (max_ - min_);
        if(delta < 0 || delta > 1.0)
            KJB_THROW_3(kjb::Illegal_argument, "value must be between %f and %f", (min_)(max_));
        return x() + delta * width();
    }

    inline int bar_center_() const
    {
        return y() + height() / 2.0;
    }

    bool has_valid_state_() const
    {
        if(fabs(max_ - min_) < DBL_EPSILON) return false;
        return true;

    }

private:
    Real min_;
    Real max_;
    Real value_;

    boost::function1<void, Real> callback_;
    bool live_updating_;
    bool grabbed_;

    static const int INDICATOR_HEIGHT;
    static const int BAR_HEIGHT;
};

template <class T>
const int Slider<T>::INDICATOR_HEIGHT = 15;
template <class T>
const int Slider<T>::BAR_HEIGHT = 10;

} // namespace gui
} // namespace kjb

#endif /* HAVE_OPENGL */
#endif

