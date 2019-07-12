/* $Id: gui_viewer.cpp 12029 2012-04-04 00:04:29Z ksimek $ */
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


#include <gui_cpp/gui_viewer.h>
#include <vector>

#ifdef KJB_HAVE_GLUT
namespace kjb
{
namespace gui 
{
std::vector<Viewer::Timer_entry> Viewer::timers_;
const size_t Viewer::TICK_PERIOD = 33;
}
}

#endif
