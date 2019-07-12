/* $Id: gui_selectable.h 11209 2011-11-22 06:07:10Z ksimek $ */
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

#ifndef KJB_CPP_GUI_LISTENER_H
#define KJB_CPP_GUI_LISTENER_H

#include <gr_cpp/gr_renderable.h>

namespace kjb
{
namespace gui
{
/**
 * Interface for a renderable object to be selectable with the
 * cursor, using opengl's  glSelect functionality.
 *
 * The render() method should call glLoadName() before rendering each 
 * selectable object (the caller will call glPushName(0) before calling this
 * object's render method, so you don't need to).
 *
 * If an object form this class is clicked during selection, process_hit
 * is called with the index of the rendered object.
 *
 * Objects implementing this interface can be attached to a Viewer by calling
 * Viewer::add_selection_listener
 */
class Selectable : public kjb::Renderable
{
public:
    /**
     * Process a hit stack returned by glSelectBuffer,  Most object will simply dereference index_stack
     * to get the index of the hit item.  Heirarchical objects will use the
     * first index to determine which object to dispatch to with the rest 
     * of the stack.
     * @param index_stack stack of indicies that make up this hit record
     * @param index_stack_end one past the end of the stack array.
     * @param button Which button was clicked. see glutMouseFunction for valued values
     * @param state Auxiliary state when click occurred (shift, ctrl, etc).  see glutMouseFunction for valid values.
     * @return true if successfully processed, which in most cases prevents other listeners from processing this hit.
     */
    virtual bool selection_hit(unsigned int* index_stack, unsigned int* index_stack_end, int button, int state) = 0;

    /**
     * Special render method.  if select_mode is true, glLoadName() should
     * be called before every selectable object.  If some objects aren't
     * selectable, you shouldn't render them when select_mode == true;
     */
    virtual void render(bool select_mode) const = 0;

    /**
     * alias for select(false)
     */
    virtual void render() const
    {
        render(false);
    }
};

}
}

#endif
