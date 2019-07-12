/* $Id: gr_glut.h 21599 2017-07-31 00:44:30Z kobus $ */
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
   |  Author:  Kyle Simek
 * =========================================================================== */

#ifndef KJB_GR_CPP_GLUT_H
#define KJB_GR_CPP_GLUT_H

#ifdef KJB_HAVE_GLUT

#include "l_cpp/l_exception.h"
#include <string>
#include <vector>

#ifdef KJB_HAVE_BST_THREAD
#include <boost/thread.hpp>
#endif


namespace boost {

template <class R>
class function0;

template <class R, class T1, class T2>
class function2;

template <class R, class T1, class T2, class T3>
class function3;

template <class R, class T1, class T2, class T3, class T4>
class function4;

//class mutex;
} // namespace boost
/**
 * @file Object-oriented interface to some glut functionality
 */

/**
 * @example test/interactive/test_glut.cpp
 */

namespace kjb
{
namespace opengl
{

class Glut
{
// TODO: 
// menu handling
// idle callback
    public:
    static unsigned int get_display_mode()
    {
        return display_mode_;
    }
    /** 
     * Set glut's display mode using display flags.  See opengl's glutInitDisplayMode documentation for available flags.
     *
     * @note  All windows share the same framebuffer, so display mode is a Glut-wide setting, hence it's  being static here.
     *
     * @throws Runtime_error if glut has already been initialized and is initialized.
     *
     * @see http://www.opengl.org/documentation/specs/glut/spec3/node12.html
     */
    static void set_display_mode(unsigned int display_mode);

    static void set_init_window_size(int w, int h);

    static void set_init_window_position(int x, int y);

    static void init();

    static void init(int argc, char** argv);

    static bool is_initialized();

    static bool is_running();

    /**
     * Save the currently active window.
     *
     * @note Only one window may be pushed at a time, so you must
     * call pop_current_window() before calling this again.  Failing
     * to do so will cause a failed assert and will aboort.
     */
    static void push_current_window();

    /**
     * Restore a previously-saved current window.
     *
     * @note calling this twice will cause a failed assert and iwll abort.
     */
    static void pop_current_window();


    static void set_idle_callback(boost::function0<void> cb);

    static void add_timer_callback(size_t msec, boost::function0<void> cb);

    static void clear_idle_callback();

    /**
     * Enable multithreading features of the Glut class.
     *
     * Enables an alternative idle function, which runs our own thread-safe
     * event-pump.  User-defined idle functions will still run as usual.
     *
     * @note Calling post_redisplay_t without first calling this will result in 
     * an exception.
     */
    static void enable_multithreading();

    static boost::thread::id thread_id();

    /**
     * This is a thread-safe way to post a redisplay event for a window.
     * It can be called from any thread, and on the next idle event 
     * in the GUI thread, a redisplay event will be posted to glut.
     *
     * @note must enable multithreading first by calling Glut::enable_multithreaded
     */
    static void post_redisplay_t(int wnd_handle);


    /**
     * Add a callback to the task queue to be called on the next
     * idle.
     *
     * This allows tasks to be added from a worker thread while
     * having the tasks executed by the GUI thread.
     */
    static void push_task_t(boost::function0<void> task);


#ifdef KJB_HAVE_BST_THREAD
    /**
     * Mutex for accessing glut/opengl functionality.  If your application
     * makes opengl/glut calls from more than one thread, you MUST acquire
     * this lock before, and release it afterward.  
     *
     * @note Even with this mutex, Opengl calls from different threads will
     * likely fail, as an opengl context is associated with a specific thread.
     * Glut calls should be okay if they only affect the window system, not
     * opengl.
     *
     * @note This member is only available if compiling with Boost Thread 
     * library installed.
     */
    static boost::mutex* mutex;
#endif

    /**
     * Thread-safe version of task_pump().
     *
     * Run all tasks in the task queue.
     */
    static void task_pump_t();

    /**
     * Run all tasks in the task queue.
     */
    static void task_pump();

/////////////////////////////////////////////////////////////////
///////////////////  WARNINGS SYSTEM ////////////////////////////
/////////////////////////////////////////////////////////////////

    static void disable_warnings();

    static bool warnings_enabled();

    /**
     * Test if glut is initialized, and output a message if it isn't.
     * This is usually called when trying to construct an object what
     * requires an opengl context to be active.
     */
    static void test_initialized(const std::string& obj_name);

private:
    /** 
     * Master idle callback for single-threaded mode.  Simply calls 
     * the callback function passed into Glut::set_idle_callback
     *
     * TODO: move this to cpp file
     */
    static void idle();

    static void timer(int i);

    /**
     * Master idle callback for multithreaded mode.
     * Empties multithreaded event pumps and then calls user-defined
     * idle callback.
     *
     * @note Always called from GUI thread only.
     */
    static void idle_t();
private:

    static unsigned int display_mode_;
    static bool initialized_;
    static bool warnings_;

    static int init_h_;
    static int init_w_;
    static int init_x_;
    static int init_y_;

    static boost::function0<void>* idle_callback_;
    static std::vector<boost::function0<void> > timer_callbacks_;
    static std::vector<size_t> timer_gaps_;

    static bool multithreading_enabled_;
    static std::vector<bool> redisplay_flags_;
    static std::vector<int> redisplay_wnds_;

    static std::vector<boost::function0<void> > tasks_;



    static int cur_wnd_stack_;

#ifdef KJB_HAVE_BST_THREAD
    static boost::thread::id* thread_id_;
#endif

};


/**
 * OO interface to glut windows.  Supports multiple windows.
 */
class Glut_window
{
// TODO: 
// menu handling
// timers?
// stacking
// set_active
// visibility callback
// entry_callback
public:
    /** 
     * Create a window with default geometry
     */
    Glut_window( const std::string& title);

    Glut_window(
            int width,
            int height,
            const std::string& title = DEFAULT_WINDOW_TITLE
            );

    Glut_window(
            int width,
            int height,
            int pos_x,
            int pos_y,
            const std::string& title
            );

    ~Glut_window();

    /** 
     * @brief    Set size of window.
     *
     * Set window size.  Viewport is reset to the entire window.
     *
     * Can be called when this window is not the currently active window.
     *
     * Thread-safe.
     */
    void set_size(int width, int height);

    /// Get the window's width
    int get_width() const;

    /// Get the window's width
    int get_height() const;

    /**
     * Set window position.
     *
     * Can be called when this window is not the currently active window.
     *
     * Thread-safe.
     */
    void set_position(int x, int y);

    /// Get the ID the OpenGL uses to identify this window
    int get_handle() const;

    /** 
     * set this as the active window and opengl rendering context.  All future opengl commands will go to this window.
     *
     * @note In a multithreaded situation, set_current() could be called from
     *       multiple different threads, resulting in errors.  You should generally
     *       try to avoid this by making all opengl/glut calls from a single
     *       "GUI" thread.  If this isn't practical, you need to handle
     *       synchronization manually by aquiring a lock on Glut::mutex before 
     *       calling this, and release the long when you're done sending commands
     *       to this window.  
     */
    void set_current() const;

    /** 
     * Post redisplay event for this window.
     *
     * Can be called when this window is not the currently active window.
     *
     * This method is not thread-safe, and should only be called from the GUI
     * thread.  To redisplay from other threads, call call redisplay_t instead. Note the Glut must be initialized using Glut::enable_multithreaded() before calling redisplay_t.
     *
     *
     * @post Glut state is unchanged.
     * @see is_thread_safe
     */
    void redisplay() const;

    /*
     * A thread-safe version of redisplay().
     *
     * This makes no direct calls to glut or opengl, but enqueues
     * a redisplay event with the Glut class, which is processed
     * at the next idle event.
     *
     */
    void redisplay_t() const;

    /**
     * Return true if this was compiled with Boost Thread library.
     *
     * @note Not all methods are thread-safe. Ones that need exclusive
     * access to glut/opengl and are most likely to be called from
     * other threads.  For now, only the methods below are thread-safe:
     *  <li>redisplay()
     *  <li>set_position()
     *  <li>set_size()
     */
    bool is_thread_safe() const;



    // ////////////////////
    //  CALLBACK SETTERS //
    // ////////////////////
    /**
     * Set callback for display events.
     * @warning Not thread safe
     */
    void set_display_callback(boost::function0<void> cb);

    const boost::function0<void>& get_display_callback() const;

    /**
     * Set callback to be called when no events are pending.
     * @note This is sets the idle callback for ALL windows.  It is
     * not possible in glut to have window-specific idle callbacks.
     * @warning Not thread safe
     */
    static void set_idle_callback(boost::function0<void> cb);
    /**
     * Set callback for window resize events
     * @warning Not thread safe
     */
    void set_reshape_callback(boost::function2<void, int, int> cb);
    /**
     * Set callback for keypress events
     * @warning Not thread safe
     */
    void set_keyboard_callback(boost::function3<void, unsigned char, int, int> cb);
    /**
     * Set callback for click events
     * @warning Not thread safe
     */
    void set_mouse_callback(boost::function4<void, int, int, int, int> cb);
    /**
     * Set the callback for handling "active" motion, i.e. "drag" events
     * @warning Not thread safe
     */
    void set_motion_callback(boost::function2<void, int, int> cb);
    /**
     * Set the callback for handling passive mouse motion
     * @warning Not thread safe
     */
    void set_passive_motion_callback(boost::function2<void, int, int> cb);
    /**
     * Set the callback for handling special keys
     * @warning Not thread safe
     */
    void set_special_callback(boost::function3<void, int, int, int> cb);

    

    // /////////////////////
    //  CALLBACK CLEARERS //
    // /////////////////////

    /* Stop handling display events.
     *
     * @note this actually sets a no-op function to handle display
     * events, as setting no display callback will cause glut to
     * crash in most implementations.
     * @warning Not thread safe
     */
    void clear_display_callback() ;

    /** stop handling idle events
     *
     * @warning Not thread safe.
     */
    static void clear_idle_callback();

    /**
     * stop handling reshape events
     *
     * @warning Not thread safe
     */
    void clear_reshape_callback();

    /** stop handling keyboard events
     *
     * @warning Not thread safe
     */
    void clear_keyboard_callback();

    /**
     * stop handling mouse click events
     *
     * @warning Not thread safe
     */
    void clear_mouse_callback();

    /**
     * stop handling mouse motion (i.e. "drag") events
     *
     * @warning Not thread safe
     */
    void clear_motion_callback();

    /**
     * stop handling mouse passive motion events 
     *
     * @warning Not thread safe
     */
    void clear_passive_motion_callback();

    /**
     * stop handling special key events 
     *
     * @warning Not thread safe
     */
    void clear_special_callback();

    /**
     * remove all window callbacks (except idle, which is global
     *
     * @warning Not thread safe
     */
    void clear_callbacks();

    void init();

private:
    // prevent copying
    Glut_window(const Glut_window&){}
    Glut_window& operator=(const Glut_window&) { return *this; }


    // /////////////
    //  CALLBACKs //
    // //////////////////////////////////////////////////////////////
    //  These are intentionally not virtual, to encourage users to 
    //  pass their callback functions into this object using the 
    //  set_<callback>() methods, and to discourage
    //  subclassing Glut_window, which should be unnecessary.
    //
    //  If the no-subclassing rule is too much trouble, we can add virtual
    //  to these later.
    //
    //  Also, these are not callable directly, because only glut should
    //  be calling these, by way of the glut_window_<callback>_callback()
    //  global functions, which are friends.  
    //
    //  You can change these to be public if needed, but think twice 
    //  before changing them and ask if you really need to call these 
    //  directly, or if you should redesign your code to keep event 
    //  handlers separate from functions that do work.
    // //////////////////////////////////////////////////////////////

    /// Call display callback
    void display_callback()  const;

    /**
     * Perform some internal bookeeping, then call the user-specified 
     * reshape callback
     */
    void reshape_callback(int w, int h);

    void keyboard_callback(unsigned char k, int x, int y)  const;

    void mouse_callback(int button, int state, int x, int y)  const;

    void motion_callback(int x, int y)  const;

    void passive_motion_callback(int x, int y)  const;

    void special_callback(int k, int x, int y)  const;

    // ////////////////////////////////
    //  CALLBACK FRIEND DECLARATIONS //
    // ////////////////////////////////
    friend void glut_window_display_callback();
    friend void glut_window_reshape_callback(int, int);
    friend void glut_window_keyboard_callback(unsigned char, int, int);
    friend void glut_window_mouse_callback(int, int, int, int);
    friend void glut_window_motion_callback(int, int);
    friend void glut_window_passive_motion_callback(int, int);
    friend void glut_window_special_callback(int, int, int);

private:
    std::string title_;
    mutable int width_;
    mutable int height_;
    int x_;
    int y_;
    int handle_;


    boost::function0<void>* display_callback_;
    boost::function2<void, int, int>* reshape_callback_;
    boost::function3<void, unsigned char, int, int>* keyboard_callback_;
    boost::function4<void, int, int, int, int>* mouse_callback_;
    boost::function2<void, int, int>* motion_callback_;
    boost::function2<void, int, int>* passive_motion_callback_;
    boost::function3<void, int, int, int>* special_callback_;

    static const char* DEFAULT_WINDOW_TITLE;
};


} // opengl
} // kjb
#endif /* KJB_HAVE_GLUT */

#endif
