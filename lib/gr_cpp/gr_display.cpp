/* $Id: gr_display.cpp 21596 2017-07-30 23:33:36Z kobus $ */
/* {{{=========================================================================== *
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
 * =========================================================================== }}}*/

// vim: tabstop=4 shiftwidth=4 foldmethod=marker

#include "gr_cpp/gr_opengl.h"

#ifdef KJB_HAVE_OPENGL
#ifdef KJB_HAVE_GLUT

#include "gr_cpp/gr_display.h"
#include "gr_cpp/gr_glut.h"
#include "gr_cpp/gr_sprite.h"

#include <string>
#include <iostream>

namespace kjb
{
namespace opengl
{


/**
 * Self-destructing class for displaying Matrices and
 * images in a Glut window
 */
class Gl_display
{
typedef Gl_display Self;

public:
    template <class Gl_displayable>
    Gl_display(const Gl_displayable& mat, const std::string& title) :
        wnd_(mat.get_num_cols(), mat.get_num_rows(), -1, -1, title),
        img_sprite_(mat)
    {
        using namespace boost;

        wnd_.set_display_callback(bind(&Self::display, this));
        wnd_.set_keyboard_callback(bind(&Self::key, this, _1, _2, _3));

        glDisable(GL_LIGHTING);

        GL_EPETE();
        GL_ETX();
    }

    void display()
    {
        try
        {
            glClearColor(1,1,1,1);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            img_sprite_.render();
            glutSwapBuffers();
        }
        catch(Opengl_error& err)
        {
            std::cout << err.get_msg() << std::endl;
            exit(1);
        }

        GL_ETX();
    }

    void key(unsigned int k, int, int)
    {
        switch(k)
        {
            case 'q':
            case 'Q':
            case 27: // ESC
                delete this;
                break;
            default:
                break;
        }
    }
private:
    Glut_window wnd_;
    Sprite img_sprite_;
};


void gl_display(const Matrix& mat, const std::string& title)
{
    Glut::push_current_window();
    new Gl_display(mat, title);
    Glut::pop_current_window();
}
//
//void gl_display(const Int_matrix& mat, const std::string& title)
//{
//    new Gl_display<Int_matrix>(mat, title);
//}
//
//void gl_display(const Image& img, const std::string& title)
//{
//    new Gl_display(img, title);
//}

} // namespace opengl
} // namespace kjb

#endif
#endif /* KJB_HABE_OPENGL */
