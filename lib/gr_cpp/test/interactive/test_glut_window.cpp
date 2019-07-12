/* $Id$ */
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

#include <iostream>

#include <m_cpp/m_matrix.h>
#include <m_cpp/m_vector.h>
#include <l_cpp/l_exception.h>

#include <g_cpp/g_quaternion.h>
#include <gr_cpp/gr_opengl.h>
#include <gr_cpp/gr_primitive.h>

#include <gr_cpp/gr_glut.h>


#include <boost/shared_ptr.hpp>

using namespace std;
using kjb::opengl::Glut_window;
using boost::shared_ptr;

// These must be global, so we can pass to DEFINE_GLUT_CALLBACKS macro later.
// Glut_windows can't be copied and must be initialized 
// during construction. We aren't ready to initialize here,
// we we must make them constant.
shared_ptr<Glut_window> wnd; // use smart pointers when possible
shared_ptr<Glut_window> wnd2;

// These macros define a large set of C-style functions that simply call
// the Glut_window's member functions.  Later we'll associate
// these callbacks with these window's rendering context.
//DEFINE_GLUT_CALLBACKS(wnd);
//DEFINE_GLUT_CALLBACKS(wnd2);

// Look ma!  A functor as a draw callback!
struct Drawer
{
    void operator()()
    {
        glClearColor(0.0, 0.0, 0.0, 0.0);

        /* clear the drawing buffer */
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);   
    cout << "draw1" << endl;
        kjb::opengl::Teapot().render();

        glutSwapBuffers(); 
    }
};

// of course, c-style functions still work
void draw2()
{
    cout << "draw2" << endl;
    glClearColor(1.0, 1.0, 1.0, 1.0);

    /* clear the drawing buffer */
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);   

    kjb::opengl::Teapot().render();
    glutSwapBuffers(); 
}


void key_handler(unsigned char k, int x, int y)
{
    if(k == 'q')
        exit(0);
}

int main()
{
    using kjb::opengl::Glut;
    // set up glut (optional)
    Glut::set_display_mode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH);
    // All glut system-wide states are settable through Glut::
    // Examples:
//    Glut::set_init_window_size(w, h);
//    Glut::set_init_window_position(w, h);

// WINDOW #1
    // Step 1: Construct a window
    Glut_window wnd;

    // Step 2: set display handler
    wnd.set_display_callback(draw2);

    // Step 3: There is no step 3!


// WINDOW #2
    // lets explore a more complicated example
    Glut_window wnd2(
            100, 100, // window position
            200, 200, // window size
            "Test window" // titlebar caption
    );

    Drawer draw;
    // create a drawer object and pass it as out callback
    // (yes, functors can be used as callbacks!)
    Drawer draw_it;
    wnd2.set_display_callback(draw_it);

    // The last window created is the active one, so it receives all opengl commands:
    glEnable(GL_DEPTH_TEST);

    // What if we want to send opengl commands to the other window?
    // Just enable it as the "current" window:
    wnd.set_current(); 
    glEnable(GL_LIGHTING); 

    // Lets add a key handler for window 2:
    wnd2.set_current();
    wnd2.set_keyboard_callback(key_handler);

    /**
     * All common handlers are available:
     *  wnd2->set_reshape_callback()
     *  wnd2->set_keyboard_callback()
     *  wnd2->set_mouse_callback()
     *  wnd2->set_motion_callback()
     *  wnd2->set_passive_motion_callback()
     *  wnd2->special_callback()
     * Callbacks not available inside object (for now):
     *  Overlay
     *  Visibility
     *  Entry
     *  SpaceballMotion
     *  SpaceballRotate
     *  SpaceballButton
     *  ButtonBox
     *  Dials
     *  TabletMotion
     *  TabletButton
     *  MenuStatus
     *  Timer
     *
     */  
    

    glutMainLoop();

    return 0;
}

//display
//idle
//reshape
//keyboard
//mouse
//motion
//passivemotion
//special
//
