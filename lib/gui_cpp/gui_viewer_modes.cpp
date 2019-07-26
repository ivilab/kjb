/* $Id: gui_viewer_modes.cpp 18283 2014-11-25 05:05:59Z ksimek $ */
/* {{{=========================================================================== *
   |
   |  Copyright (c) 1994-2012 by Kobus Barnard (author)
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

#ifdef KJB_HAVE_OPENGL 
#include <gui_cpp/gui_viewer_modes.h>

namespace kjb
{
namespace gui
{

void set_visibility(kjb::gui::Viewer& viewer, const Index_range& items, bool visibility)
{
    for(size_t i = 0; i < items.size(); ++i)
    {
        viewer.set_renderable_visibility(items[i], visibility);
    }
}

bool exclusive_display_keys(kjb::gui::Viewer* viewer, boost::shared_ptr<int> state_ptr, const Index_range& items, unsigned char k, int , int )
{
    int& state = *state_ptr;
    size_t N = items.size();
    switch(k)
    {
        case '{':
            state = (state + 1) % N;
            set_visibility(*viewer, items, false);
            viewer->set_renderable_visibility(items[state], true);
            glutPostRedisplay();
            return true;
        case '}':
            state = (state + N-1) % N;
            set_visibility(*viewer, items, false);
            viewer->set_renderable_visibility(items[state], true);
            glutPostRedisplay();
            return true;
        default:
            return false;
    }
}

void enable_exclusive_display_mode(kjb::gui::Viewer& viewer, const kjb::Index_range& items)
{
    // disable all views
    set_visibility(viewer, items, false);

    // enable only the first
    if(items.size() > 0)
        viewer.set_renderable_visibility(items[0],true);

    boost::shared_ptr<int> state(new int);
    *state = 0;

    viewer.add_keyboard_listener(boost::bind(exclusive_display_keys, &viewer, state, items, _1, _2 ,_3));

    glutPostRedisplay();
}

} // namespace gui
} // namespace kjb
#endif // KJB_HAVE_OPENGL 
