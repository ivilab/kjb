/* $Id: gr_display.h 21599 2017-07-31 00:44:30Z kobus $ */
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

#include "gr_cpp/gr_opengl_texture.h"

#ifndef KJB_CPP_GR_DISPLAY
#define KJB_CPP_GR_DISPLAY

// vim: tabstop=4 shiftwidth=4 foldmethod=marker
#include <m_cpp/m_matrix.h>
#include <i_cpp/i_image.h>

#ifdef KJB_HAVE_GLUT
namespace kjb
{
namespace opengl
{


void gl_display(const Matrix& mat, const std::string& title = "");
//
//
//void gl_display(const Image& img, const std::string& title = "");
//

} // namespace opengl
} // namespace kjb

#endif
#endif /* KJB_CPP_GR_DISPLAY */
