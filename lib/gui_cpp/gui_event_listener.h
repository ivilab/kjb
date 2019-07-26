/* $Id: gui_event_listener.h 11234 2011-11-28 04:44:53Z ksimek $ */
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

#ifndef KJB_CPP_GUI_EVENT_LISTENER
#define KJB_CPP_GUI_EVENT_LISTENER
/**
 * Abstract interface for objects that can consume Glut interaction events
 * (click, drag, hover, and keyboard).  Method names and their signatures 
 * come from Glut's API e.g. the drag method is called "motion" after 
 * Glut's glutMotionFunc() function.
 *
 * Returning "true" indicates that the function has consumed the event and
 * other listeners won't receive it.
 */
class Event_listener
{
public:
    /** Event called when mouse is clicked */
    virtual bool mouse_event(int button, int state, int x, int y) = 0;
    /** Event called when mouse is clicked */
    virtual bool mouse_double_event(int button, int state, int x, int y) = 0;
    /** Event called when mouse is dragged */
    virtual bool motion_event(int x, int y) = 0;
    /** Event called when mouse is moved without clicking ("hover") */
    virtual bool passive_motion_event(int x, int y) = 0;
    /** Event called when ascii keys are pressed */
    virtual bool keyboard_event(unsigned int key, int x, int y) = 0;
    /** Event called when other keys are pressed (arrows, pgup, f1, f2, ...) */
    virtual bool special_event(int key, int x, int y) = 0;
};

/**
 * Event listener that returns false for all events.
 *
 * Listener classes that only wish do implement one or two event
 * handlers can inherit from this as a convenient way to avoid 
 * writing a bunch of one-line methods.  
 *
 * Private inheritance is preferred,
 * so the compiler can still catch virtual override mismatch bugs.
 * For example:
 *  @code
 *  Click_listener : private Null_event_listener
 *  {
 *      typedef Null_event_listener Base;
 *  public:
 *      using Base::motion_event;
 *      using Base::passive_motion_event;
 *      using Base::keyboard_event;
 *      using Base::special_event;
 *
 *      // BUG!  signature doesn't match Event_listener::mouse_event
 *      // Luckilly, compiler will throw an error: "Can't instantiate
 *      // abstract class" .
 *      virtual bool mouse_event(int button, int x, int y) const
 *      {
 *          return true;
 *      }
 *  };
 *  @endcode
 */
class Null_event_listener : public Event_listener
{
public:
    virtual void reset_event()
    {
    }

    virtual bool mouse_event(int , int , int , int )
    {
        return false;
    }

    virtual bool mouse_double_event(int , int , int , int )
    {
        return false;
    }

    virtual bool motion_event(int , int )
    {
        return false;
    }

    virtual bool passive_motion_event(int , int )
    {
        return false;
    }

    virtual bool keyboard_event(unsigned int , int , int )
    {
        return false;
    }

    virtual bool special_event(int , int , int )
    {
        return false;
    }

};

#endif
