/* $Id: gr_glut.cpp 21596 2017-07-30 23:33:36Z kobus $ */
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

#include "l/l_sys_debug.h"  /* For ASSERT. */
#include "gr_cpp/gr_opengl.h"
#include "gr_cpp/gr_opengl_headers.h"
#include "gr_cpp/gr_glut.h"
#include "gr_cpp/gr_glew.h"
#include "boost/function.hpp"

#include <iostream>

#ifdef KJB_HAVE_GLUT
namespace kjb
{
namespace opengl
{

// STATIC MEMBER INITIALIZATION
#ifdef KJB_HAVE_BST_THREAD
boost::mutex* Glut::mutex = new boost::mutex;
boost::thread::id* Glut::thread_id_ = new boost::thread::id;
#endif

bool Glut::initialized_ = false;
bool Glut::warnings_ = true;

// reasonable default
unsigned int Glut::display_mode_ = GLUT_RGBA | GLUT_ALPHA | GLUT_DOUBLE | GLUT_DEPTH;
int Glut::init_h_ = -1;
int Glut::init_w_ = -1;
int Glut::init_x_ = -1;
int Glut::init_y_ = -1;

boost::function0<void>* Glut::idle_callback_ = new boost::function0<void>();

std::vector<boost::function0<void> > Glut::timer_callbacks_;
std::vector<size_t> Glut::timer_gaps_;

bool Glut::multithreading_enabled_ = false;

std::vector<bool> Glut::redisplay_flags_;
std::vector<int> Glut::redisplay_wnds_;
std::vector<boost::function0<void> > Glut::tasks_;

int Glut::cur_wnd_stack_ = -1;

const char* Glut_window::DEFAULT_WINDOW_TITLE = "Glut Window";

///////////////////////////////////////////////////
// Glut_window CALLBACK GLOBALS
//
// Opengl is pure C, which means there's no good
// way to call functors or class methods as callbacks
// (only global functions... ack!).
//
// We handle this by registering all Glut_window objects
// in a vector called glut_window_registry__ when they're constructed
// The vector is indexed on their window id for rapid lookup
// (which starts at 1 and increases incrementally).
//
// Then, we register a single global callback for all Glut_windows,
// which determines which window called it, and calls that window's 
// specific callback method.  This allows all opengl interaction to
// be encapsulated inside Glut_window.
//
// Admittedly, the implementation is ugly, but the interface that the user
// sees will be very clean.
/////////////////////////////////////////////////

static std::vector<Glut_window*> glut_window_registry__;


void Glut::set_display_mode(unsigned int display_mode)
{
    if(initialized_)
    {
        KJB_THROW_2(Runtime_error, "Can't set display mode on an initialized glut instance.  This must be called before creating any windows");
    }

    display_mode_ = display_mode;
}

void Glut::set_init_window_size(int w, int h)
{
    if(initialized_)
        KJB_THROW_2(Runtime_error, "Can't set initial window size after Glut has been initialized.  This must be called before creating any windows.");

    init_w_ = w; 
    init_h_ = h;
}

void Glut::set_init_window_position(int x, int y)
{
    if(initialized_)
        KJB_THROW_2(Runtime_error, "Can't set initial window position after Glut has been initialized.  This must be called before creating any windows.");

    init_x_ = x;
    init_y_ = y;
}

void Glut::init()
{
    init(0, 0);
}

void Glut::init(int argc, char** argv)
{
    glutInit(&argc, argv);
    glutInitDisplayMode(display_mode_);
    glutInitWindowSize(init_w_, init_h_);
    glutInitWindowPosition(init_x_, init_y_);

    if(multithreading_enabled_)
        glutIdleFunc(Glut::idle_t);

    initialized_ = true;

}

bool Glut::is_initialized()
{
    return initialized_;
}

bool Glut::is_running()
{
    return glutGetWindow() > 0;

}

void Glut::push_current_window()
{
    // max stack size is 1.  It is considered
    // a bug to push current window twice without
    // popping in-between.
    ASSERT(cur_wnd_stack_ == -1);
    cur_wnd_stack_ = glutGetWindow();
}

void Glut::pop_current_window()
{
    ASSERT(cur_wnd_stack_ != -1);

    if(cur_wnd_stack_ > 0)
        glutSetWindow(cur_wnd_stack_);

    cur_wnd_stack_ = -1;
}

void Glut::set_idle_callback(boost::function0<void> cb)
{
    *idle_callback_ = cb;

    if(!multithreading_enabled_)
    {
        if(cb)
            glutIdleFunc(Glut::idle);
        else
            glutIdleFunc(NULL);
    }
}

void Glut::add_timer_callback(size_t msec, boost::function0<void> cb)
{
    int i;
    if(timer_gaps_.size() > 0)
    {
        i = timer_gaps_.back();
        timer_gaps_.pop_back();
        timer_callbacks_[i] = cb;
    }
    else
    {
        i = timer_callbacks_.size();
        timer_callbacks_.push_back(cb);
    }


    glutTimerFunc(msec, Glut::timer, i);
}

void Glut::clear_idle_callback()
{
    idle_callback_->clear();

    if(!multithreading_enabled_)
    {
        glutIdleFunc(NULL);
    }
}



// TODO add obscure callbacks like timer, visible, etc.
void glut_window_display_callback(void)
{
    const int i = glutGetWindow();
    ASSERT(glut_window_registry__[i]);
    glut_window_registry__[i]->display_callback();
}

//void glut_window_idle_callback(void)
//{
//    const int i = glutGetWindow();
//    ASSERT(glut_window_registry__[i]);
//    glut_window_registry__[i]->idle_callback();
//}

void glut_window_reshape_callback(int width, int height)
{
    const int i = glutGetWindow();
    ASSERT(glut_window_registry__[i]);
    glut_window_registry__[i]->reshape_callback(width, height);
}
void glut_window_keyboard_callback(unsigned char key, int x, int y)
{
    const int i = glutGetWindow();
    ASSERT(glut_window_registry__[i]);
    glut_window_registry__[i]->keyboard_callback(key, x, y);
}
void glut_window_mouse_callback(int button, int state, int x, int y)
{
    const int i = glutGetWindow();
    ASSERT(glut_window_registry__[i]);
    glut_window_registry__[i]->mouse_callback(button, state, x, y);
}
void glut_window_motion_callback(int x, int y)
{
    const int i = glutGetWindow();
    ASSERT(glut_window_registry__[i]);
    glut_window_registry__[i]->motion_callback(x, y);
}
void glut_window_passive_motion_callback(int x, int y)
{
    const int i = glutGetWindow();
    ASSERT(glut_window_registry__[i]);
    glut_window_registry__[i]->passive_motion_callback(x, y);
}
void glut_window_special_callback(int key, int x, int y)
{
    const int i = glutGetWindow();
    ASSERT(glut_window_registry__[i]);
    glut_window_registry__[i]->special_callback(key, x, y);
}

// Default display handler for newly constructed windows.
void glut_window_null_display_callback()
{ }

// /////////////
// GLUT  //
// ////////
void Glut::task_pump()
{

    for(size_t i = 0; i < tasks_.size(); i++)
    {
        tasks_[i]();
    }

    tasks_.clear();
}

void Glut::task_pump_t()
{
#ifdef KJB_HAVE_BST_THREAD
    std::vector<boost::function0<void> > tasks_2;

    // important not to hold the lock while we call the callbacks.
    // (Unless you like deadlocks!)
    // So we copy the callbacks to a temporary vector and
    // then free the lock.
    {
        boost::lock_guard<boost::mutex> lock(*mutex);
        tasks_2 = tasks_;
        tasks_.clear();
    }


    for(size_t i = 0; i < tasks_2.size(); i++)
    {
        tasks_2[i]();
    }
#else
    KJB_THROW_2(kjb::Missing_dependency, "opengl");
#endif
}

void Glut::enable_multithreading()
{
#ifdef KJB_HAVE_BST_THREAD
    multithreading_enabled_ = true;

    // store thread id, so later we can check if we're in the GUI thread.
    // TODO: If context ever changes threads, will need to update this.
    //       But for now, it isn't possible to switch a context to a 
    //       different thread.
    *thread_id_ = boost::this_thread::get_id();

    if(initialized_)
        glutIdleFunc(Glut::idle_t);
#else
    KJB_THROW_2(Runtime_error, "Cannot enable multithreaded glut: compiled without Boost::thread.");
#endif
}

boost::thread::id Glut::thread_id()
{
#ifdef KJB_HAVE_BST_THREAD
    return *thread_id_;
#else
    KJB_THROW_2(Runtime_error, "Cannot enable multithreaded glut: compiled without Boost::thread.");
#endif
}


void Glut::post_redisplay_t(int wnd_handle)
{
#ifdef KJB_HAVE_BST_THREAD
    boost::lock_guard<boost::mutex> lock(*mutex);

    if(!multithreading_enabled_)
    {
        KJB_THROW_2(Runtime_error, "post_redisplay_t: Glut multithreading not enabled.");

    }
    
    // resize hash map if needed
    if((int) redisplay_flags_.size() <= wnd_handle)
        redisplay_flags_.resize(wnd_handle+1, false);

    // don't post twice
    if(redisplay_flags_[wnd_handle])
        return;
    redisplay_flags_[wnd_handle] = true;
    redisplay_wnds_.push_back(wnd_handle);
#else
    KJB_THROW_2(Missing_dependency, "boost::thread");
#endif
}

void Glut::push_task_t(boost::function0<void> task)
{
#ifdef KJB_HAVE_BST_THREAD
    boost::lock_guard<boost::mutex> lock(*mutex);
    if(!multithreading_enabled_)
    {
        KJB_THROW_2(Runtime_error, "push_task_t: Glut multithreading not enabled.");

    }
    tasks_.push_back(task);
#else
    KJB_THROW_2(Missing_dependency, "boost::thread");
#endif
}

void Glut::disable_warnings()
{
    warnings_ = false;
}

bool Glut::warnings_enabled()
{
    return warnings_;
}

void Glut::test_initialized(const std::string& obj_name)
{
#ifdef TEST
    if(!Glut::is_initialized() && Glut::warnings_enabled())
    {
        std::cerr << "<<TEST>> WARNING: Glut appears to be uninitialized, suggesting that an OpenGL context probably isn't active.\n";
        std::cerr << "<<TEST>>     Constructing a(n) " << obj_name << " will fail without an active OpenGL context.\n";
        std::cerr << "<<TEST>>     To solve this, create an active opengl context by either:\n";
        std::cerr << "<<TEST>>       (a) Construct a kjb::opengl::Glut_window.\n";
        std::cerr << "<<TEST>>       (b) Construct a glut window using the native GLUT calls.\n";
        std::cerr << "<<TEST>>       (c) Construct an opengl context by some other means.\n";
        std::cerr << "<<TEST>>     If using method (b) or (c), calling Glut::disable_warnings() will suppress this message.\n";
        std::cerr << "<<TEST>>     (This message won't appear when compiling in production mode.)" << std::endl;
    }
#endif
}

void Glut::idle()
{
    ASSERT(*idle_callback_);
    (*idle_callback_)();
}

void Glut::timer(int i)
{
    ASSERT(i < static_cast<int>(timer_callbacks_.size()));
    timer_callbacks_[i]();
    timer_gaps_.push_back(i);
}

void Glut::idle_t()
{
#ifdef KJB_HAVE_BST_THREAD
    ASSERT(multithreading_enabled_);
    // handle redisplay calls sent from other threads
    {
        boost::lock_guard<boost::mutex> lock(*mutex);

        // for each handle in list, redisplay
        for(size_t i = 0; i < redisplay_wnds_.size(); i++)
        {
            int handle = redisplay_wnds_[i];
            Glut_window* wnd = glut_window_registry__[handle];


            if(wnd)
            {
                wnd->redisplay();
            }
            else
            {
                // It's okay for wnd to be null here.
                // It just means that the window was destroyed before 
                // the redisplay event could be handled.
                ;
            }

            redisplay_flags_[handle] = false;
        }

        redisplay_wnds_.clear();

    }

    task_pump_t();
    
    // call user's idle callback or disable idle callback
    if(*idle_callback_)
    {
        (*idle_callback_)();
    }
    else
    {
        // don't consume all the cpu time.  Let the worker
        // threads to their work!
        boost::thread::yield();
    }
#else
    KJB_THROW_2(Missing_dependency, "boost::thread");
#endif
}


// //////////////
// 'STRUCTORS  //
// //////////////
Glut_window::Glut_window(
        const std::string& title 
        ) : 
    title_(title),
    width_(-1),
    height_(-1),
    x_(-1),
    y_(-1),
    handle_(-1),
    display_callback_(new boost::function0<void>()),
    reshape_callback_(new boost::function2<void, int, int>()),
    keyboard_callback_(new boost::function3<void, unsigned char, int, int>()),
    mouse_callback_(new boost::function4<void, int, int, int, int>()),
    motion_callback_(new boost::function2<void, int, int>()),
    passive_motion_callback_(new boost::function2<void, int, int>()),
    special_callback_(new boost::function3<void, int, int, int>())
{
    init();
}

Glut_window::Glut_window(
        int width,
        int height,
        const std::string& title
        ) :
    title_(title),
    width_(width),
    height_(height),
    x_(-1),
    y_(-1),
    handle_(-1),
    display_callback_(new boost::function0<void>()),
    reshape_callback_(new boost::function2<void, int, int>()),
    keyboard_callback_(new boost::function3<void, unsigned char, int, int>()),
    mouse_callback_(new boost::function4<void, int, int, int, int>()),
    motion_callback_(new boost::function2<void, int, int>()),
    passive_motion_callback_(new boost::function2<void, int, int>()),
    special_callback_(new boost::function3<void, int, int, int>())
{
    init();
}

Glut_window::Glut_window(
        int width,
        int height,
        int pos_x,
        int pos_y,
        const std::string& title
        ) :
    title_(title),
    width_(width),
    height_(height),
    x_(pos_x),
    y_(pos_y),
    handle_(-1),
    display_callback_(new boost::function0<void>()),
    reshape_callback_(new boost::function2<void, int, int>()),
    keyboard_callback_(new boost::function3<void, unsigned char, int, int>()),
    mouse_callback_(new boost::function4<void, int, int, int, int>()),
    motion_callback_(new boost::function2<void, int, int>()),
    passive_motion_callback_(new boost::function2<void, int, int>()),
    special_callback_(new boost::function3<void, int, int, int>())
{
    init();
}


Glut_window::~Glut_window()
{
    ASSERT(handle_ > 0);
    if(Glut::is_running())
        glutDestroyWindow(handle_);
    else
    {
        ; // glut already destroyed this (probably due to exit() being called)
    }
    glut_window_registry__[handle_] = NULL;

    delete display_callback_;
    delete reshape_callback_;
    delete keyboard_callback_;
    delete mouse_callback_;
    delete motion_callback_;
    delete passive_motion_callback_;
    delete special_callback_;
}

// ////////////////////
//  CALLBACK SETTERS //
// ////////////////////
void Glut_window::set_size(int width, int height)
{
#ifdef KJB_HAVE_BST_THREAD
    boost::lock_guard<boost::mutex> lock(*Glut::mutex);
#endif


    // save old active window
    int old_window = glutGetWindow();
    // set active window to this one
    glutSetWindow(handle_);
    glutReshapeWindow(width, height);
    // restore active window?
    glutSetWindow(old_window);
}

int Glut_window::get_width() const
{
    return width_;
}

int Glut_window::get_height() const
{
    return height_;
}


void Glut_window::set_position(int x, int y)
{
#ifdef KJB_HAVE_BST_THREAD
    boost::lock_guard<boost::mutex> lock(*Glut::mutex);
#endif

    x_ = x;
    y_ = y;


    // save old active window
    int old_window = glutGetWindow();
    // set active window to this one
    glutSetWindow(handle_);
    glutPositionWindow(x, y);
    // restore active window?
    glutSetWindow(old_window);
}

int Glut_window::get_handle() const
{
    return handle_;
}

void Glut_window::set_current() const
{
    glutSetWindow(handle_);
}

void Glut_window::redisplay() const
{
    // save old active window
    int old_window = glutGetWindow();
    // set active window to this one
    glutSetWindow(handle_);
    glutPostRedisplay();
    // restore active window?
    glutSetWindow(old_window);
}

void Glut_window::redisplay_t() const
{
    Glut::post_redisplay_t(handle_);
}

bool Glut_window::is_thread_safe() const
{
#ifdef KJB_HAVE_BST_THREAD
    return true;
#else
    return false;
#endif
}


/**
 * Set function to handle display events.  Note that most (all?) glut implementations
 * require this to be set, otherwise it will trigger a fatal error.
 *
 * @param cb Function or functor with signature "void f()".
 * @see http://www.opengl.org/resources/libraries/glut/spec3/node46.html
 */
void Glut_window::set_display_callback(boost::function0<void> cb)
{
    *display_callback_ = cb;

    if(*display_callback_)
        glutDisplayFunc(glut_window_display_callback);
    else
        clear_display_callback();

}

const boost::function0<void>& Glut_window::get_display_callback() const
{
    return *display_callback_;
}


/**
 * Set function to run when no events are present.  This sets the idle
 * callback for ALL glut windows (see note below).
 *
 * See set_display_callback for more information.
 *
 * @note Glut's architecture only allows a single idle callback for all windows, hence this method is static.  Setting this will disable any existing idle
 * callback and set a new one for ALL windows, including glut windows
 * not constructed using the Glut_window object wrapper.
 *
 * @param cb Function or functor with signature "void f()".
 * @see set_display_callback
 * @see http://www.opengl.org/resources/libraries/glut/spec3/node63.html
 */
void Glut_window::set_idle_callback(boost::function0<void> cb)
{
    Glut::set_idle_callback(cb);
}

/**
 * Set handler for window-resize events. 
 * See set_display_callback for more information.
 *
 * Function pointer parameters:
 *  <li> int width - The new window width
 *  <li> int height - The new window height
 *
 * @param cb Function or functor with signature "void f(int, int)".
 * @see set_display_callback
 * @see http://www.opengl.org/resources/libraries/glut/spec3/node48.html
 */
void Glut_window::set_reshape_callback(boost::function2<void, int, int> cb)
{
    *reshape_callback_ = cb;

    if(*reshape_callback_)
        glutReshapeFunc(glut_window_reshape_callback);
    else
        glutReshapeFunc(NULL);
}

/**
 * Set handler for key-press events. 
 * See set_display_callback for more information.
 *
 * Function pointer parameters:
 *  <li> unsigned char button - ASCII code of key pressed.
 *  <li> int x - The mouse x-coordinate when the event occurred.
 *  <li> int y - The mouse y-coordinate when the event occurred.
 *
 * @param cb Function or functor with signature "void f(unsigned char, int, int)".
 * @see set_display_callback
 * @see http://www.opengl.org/resources/libraries/glut/spec3/node49.html
 */
void Glut_window::set_keyboard_callback(boost::function3<void, unsigned char, int, int> cb)
{
    *keyboard_callback_ = cb;

    if(*keyboard_callback_)
        glutKeyboardFunc(glut_window_keyboard_callback);
    else
        glutKeyboardFunc(NULL);
}

/**
 * Set handler for active mouse click events. 
 * See set_display_callback for more information.
 *
 * Function pointer parameters:
 *  <li> int button - Which mouse button was pressed
 *  <li> int state - The state when the button was pressed (was shift key held, etc)
 *  <li> int x - The screen x-coordinate where the click occurred
 *  <li> int y - The screen y-coordinate where the click occurred
 *
 * @param cb Function or functor with signature "void f(int, int, int, int)".
 * @see set_display_callback
 * @see http://www.opengl.org/resources/libraries/glut/spec3/node50.html
 */
void Glut_window::set_mouse_callback(boost::function4<void, int, int, int, int> cb)
{
    *mouse_callback_ = cb;

    if(*mouse_callback_)
        glutMouseFunc(glut_window_mouse_callback);
    else
        glutMouseFunc(NULL);
}

/**
 * Set handler for active mouse motion (i.e. "drag") events. 
 * See set_display_callback for more information.
 *
 * Function pointer parameters:
 *  <li> int x - The mouse's current x-coordinate
 *  <li> int y - The mouse's current y-coordinate
 *
 * @param cb Function or functor with signature "void f(int, int)".
 * @see set_display_callback
 * @see http://www.opengl.org/resources/libraries/glut/spec3/node51.html
 */
void Glut_window::set_motion_callback(boost::function2<void, int, int> cb)
{
    *motion_callback_ = cb;

    if(*motion_callback_)
        glutMotionFunc(glut_window_motion_callback);
    else
        glutMotionFunc(NULL);
}

/**
 * Set handler for passive mouse motion events.
 * See set_display_callback for more information.
 *
 * Function pointer parameters:
 *  <li> int x - The mouse's current x-coordinate
 *  <li> int y - The mouse's current y-coordinate
 *
 * @param cb Function or functor with signature "void f(int, int)".
 * @see set_display_callback
 * @see http://www.opengl.org/resources/libraries/glut/spec3/node51.html
 */
void Glut_window::set_passive_motion_callback(boost::function2<void, int, int> cb)
{
    *passive_motion_callback_ = cb;

    if(*passive_motion_callback_)
        glutPassiveMotionFunc(glut_window_passive_motion_callback);
    else
        glutPassiveMotionFunc(NULL);
}

/**
 * Set handler for special key events. See set_display_callback for more information.
 *
 * Function pointer parameters:
 *  <li> int key - The code of the special key that was typed.
 *  <li> int x   - The mouse x-coordinate when the event occurred
 *  <li> int y   - The mouse y-coordinate when the event occurred
 *
 * @param cb Function or functor with signature "void f(int, int, int)".
 * @see set_display_callback
 * @see http://www.opengl.org/resources/libraries/glut/spec3/node54.html
 */
void Glut_window::set_special_callback(boost::function3<void, int, int, int> cb)
{
    *special_callback_ = cb;

    if(*special_callback_)
        glutSpecialFunc(glut_window_special_callback);
    else
        glutSpecialFunc(NULL);
}

void Glut_window::clear_display_callback()
{
    set_display_callback(glut_window_null_display_callback);
}

void Glut_window::clear_idle_callback()
{
    Glut::clear_idle_callback();
}

void Glut_window::clear_reshape_callback()
{
    reshape_callback_->clear();
    glutReshapeFunc(NULL);
}

void Glut_window::clear_keyboard_callback()
{
    keyboard_callback_->clear();
    glutKeyboardFunc(NULL);
}

void Glut_window::clear_mouse_callback()
{
    mouse_callback_->clear();
    glutMouseFunc(NULL);
}

void Glut_window::clear_motion_callback()
{
    motion_callback_->clear();
    glutMotionFunc(NULL);
}

void Glut_window::clear_passive_motion_callback()
{
    passive_motion_callback_->clear();
    glutPassiveMotionFunc(NULL);
}

void Glut_window::clear_special_callback()
{
    special_callback_->clear();
    glutSpecialFunc(NULL);
}

void Glut_window::clear_callbacks()
{
    clear_display_callback();
//        clear_idle_callback();
    clear_reshape_callback();
    clear_keyboard_callback();
    clear_mouse_callback();
    clear_motion_callback();
    clear_passive_motion_callback();
    clear_special_callback();
}

void Glut_window::init()
{
    // glut must be initialized before creating
    // a window
    if(!Glut::is_initialized())
    {
        Glut::init();
    }

    // CREATE WINDOW AND ADD IT TO THE GLOBAL WINDOW REGISTRY
    handle_ = glutCreateWindow(title_.c_str());

#ifdef KJB_HAVE_GLEW
    // for convenience, initialize glew here.  Calling twice is okay
    Glew::init();
#endif

    if(handle_ >= (int) glut_window_registry__.size())
    {
        
        // add slots to the end, filling them with null.
        // (we don't care about empty spots; they won't take up
        // much space, and lookup speed is most important since it 
        // will be called on every refresh)
        glut_window_registry__.resize(handle_ + 1, NULL);
    }

    glut_window_registry__[handle_] = this;

    set_display_callback(glut_window_null_display_callback);

    // SET SIZE AND POSITION
    
    if(width_ > 0 && height_ > 0)
        set_size(width_, height_);

    if(x_ > 0 && y_ > 0)
        set_position(x_, y_);

    // Default to orthographic projection with direct mapping between
    // world coordinates and pixel coordinates
    GLint vp[4];
    glGetIntegerv(GL_VIEWPORT, vp);
    GL_ETX();

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(vp[0], vp[2], vp[1], vp[3]);
    GL_ETX();
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    GL_ETX();

}
 
void Glut_window::display_callback()  const
{
     // consider removing this test if performance suffers
     // (no, must leave it, because it will be called before being registered).
    if(*display_callback_)
        (*display_callback_)();
}

void Glut_window::reshape_callback(int w, int h)
{

    // user-specified callback will usually also call this, but just to be safe
//        glViewport(0,0,w, h);
//      No!  This can cause hard-to-debug rendering errors
//      when rendering to offscreen buffers  --ksimek, 2013-10-13

    // ensure this window still knows its own dimensions
    width_ = w;
    height_ = h;

     // consider removing this test if performance suffers
    if(*reshape_callback_)
        (*reshape_callback_)(w, h);
}

void Glut_window::keyboard_callback(unsigned char k, int x, int y)  const
{
     // consider removing this test if performance suffers
    if(*keyboard_callback_)
        (*keyboard_callback_)(k, x, y);
}

void Glut_window::mouse_callback(int button, int state, int x, int y)  const
{
     // consider removing this test if performance suffers
    if(*mouse_callback_)
        (*mouse_callback_)(button, state, x, y);
}

void Glut_window::motion_callback(int x, int y)  const
{
     // consider removing this test if performance suffers
    if(*motion_callback_)
        (*motion_callback_)(x,y);
}

void Glut_window::passive_motion_callback(int x, int y)  const
{
     // consider removing this test if performance suffers
    if(*passive_motion_callback_)
        (*passive_motion_callback_)(x, y);
}

void Glut_window::special_callback(int k, int x, int y)  const
{
     // consider removing this test if performance suffers
    if(*special_callback_)
        (*special_callback_)(k, x, y);
}



} // namespace opengl
} // namespace kjb

#endif /* KJB_HAVE_OPENGL */
