/* $Id: gr_glew.cpp 18278 2014-11-25 01:42:10Z ksimek $ */
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

#include <l_cpp/l_exception.h>
#include <gr_cpp/gr_glew.h>
#include <iostream>

#ifdef KJB_HAVE_GLEW
#include <GL/glew.h>
#endif

namespace kjb
{
namespace opengl
{

bool Glew::initialized_ = false;
bool Glew::warnings_ = true;

void Glew::init()
{
    using namespace std;
#ifdef KJB_HAVE_GLEW
    if(!initialized_)
    {
        GLenum err = glewInit();
        if (GLEW_OK != err)
        {
           /* Problem: glewInit failed, something is seriously wrong. */
           KJB_THROW_3(kjb::Runtime_error, "Error: %s", (glewGetErrorString(err)));
        }

        initialized_ = true;
    }
#else
    std::cerr << "WARNING: Glew::init() failed because Glew wasn't installed at compile time." << std::endl;
#endif
}


void Glew::test_initialized(const std::string& obj_name)
{
#ifdef TEST
    if(!Glew::is_initialized() && Glew::warnings_enabled())
    {
        std::cerr << "<<TEST>> WARNING: GLEW appears to be uninitialized.\n";
        std::cerr << "<<TEST>>     Constructing a(n) " << obj_name << " will fail if GLEW isn't initialized.\n";
        std::cerr << "<<TEST>>     To solve this, either:\n";
        std::cerr << "<<TEST>>       (a) Call kjb::opengl::Glew::init() before constructing a " << obj_name << ".\n";
        std::cerr << "<<TEST>>       (b) Use Glut_window, which calls Glew::init() automatically.\n";
        std::cerr << "<<TEST>>       (c) Call the native GLEW function, init_glew().\n";
        std::cerr << "<<TEST>>     If using method (c), calling Glew::disable_warnings() will suppress this message.\n";
        std::cerr << "<<TEST>>     (This message won't appear when compiling in production mode.)" << std::endl;
    }
#endif
}

} // namespace opengl
} // namespace kjb
