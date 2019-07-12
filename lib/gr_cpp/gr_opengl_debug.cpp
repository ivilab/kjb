/* $Id: gr_opengl_debug.cpp 21596 2017-07-30 23:33:36Z kobus $ */
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
#include "gr_cpp/gr_opengl_headers.h"
#include "gr_cpp/gr_opengl_debug.h"
#include "m_cpp/m_matrix.h"
#include "l_cpp/l_exception.h"

namespace kjb
{
/**
 * This pops up a window to display the contents of the opengl color buffer during debugging.  
 *
 * This is intentionally in the global namespace to make it
 * callable inside gdb using "print kjb_opengl_debug_display_buffer()".
 *
 * Since it's difficult to know exactly the format of the active
 * buffer, this simply assumes it's grayscale and reads the
 * red component only.  This should be good enough for debugging.
 */
void kjb_opengl_debug_display_buffer()
{
#ifdef KJB_HAVE_OPENGL
    GLint vp[4] = {0};
    glGetIntegerv(GL_VIEWPORT, vp);

    int x = vp[0];
    int y = vp[1];
    int width = vp[2];
    int height = vp[3];
    

    float* pixels = new float[width * height];
    glReadPixels(x, y, width, height, GL_RED, GL_FLOAT, pixels);
    kjb::Matrix mat(height, width);

    int i = 0;
    for(int row = height - 1; row >= 0; row--)
    for(int col = 0; col < width; col++, i++)
    {
        mat(row, col) = pixels[i] * 255;
    }

    delete[] pixels;

    mat.display();
#else
    KJB_THROW_2(Missing_dependency, "opengl");
#endif
} // namespace opengl
} // namespace kjb
