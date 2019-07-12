/* $Id: gr_glew.h 18278 2014-11-25 01:42:10Z ksimek $ */
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
#ifndef KJB_CPP_GR_GLEW_H
#define KJB_CPP_GR_GLEW_H

#include <string>

namespace kjb
{
namespace opengl
{

class Glew
{
public:
    static void init();

    static bool is_initialized()
    {
        return initialized_;
    }

    static void disable_warnings()
    {
        warnings_ = false;
    }

    static bool warnings_enabled()
    {
        return warnings_;
    }

    /**
     * Test if glut is initialized, and output a message if it isn't.
     * This is usually called when trying to construct an object what
     * requires an opengl context to be active.
     */
    static void test_initialized(const std::string& obj_name);

private:
    static bool initialized_;
    static bool warnings_;
};

} // namespace opengl
} // namespace kjb

#endif
