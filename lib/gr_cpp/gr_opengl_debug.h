/* $Id: gr_opengl_debug.h 18278 2014-11-25 01:42:10Z ksimek $ */
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

#ifndef KJB_CPP_GR_OPENGL_DEBUG
#define KJB_CPP_GR_OPENGL_DEBUG

namespace kjb
{
namespace opengl
{

// Note: These files are _intentionally_ in the global namespace to simplify calling from gdb.
// Function name prefixes mimick actual KJB namespaces

/** @brief Pop up a window displaying the opengl color buffer */
void kjb_opengl_debug_display_buffer();

}
}
#endif
