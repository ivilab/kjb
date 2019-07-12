/* $Id: l_scope_guard.h 14082 2013-03-12 18:32:12Z jguan1 $ */
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

#ifndef KJB_L_CPP_SCOPE_GUARD_H
#define KJB_L_CPP_SCOPE_GUARD_H

#include <boost/function.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/bind.hpp>

namespace kjb
{

/**
 * A simple object that calls a callback when it leaves scope.  This is very useful to ensure
 * exception-safety when a resource is obtained at the beginning of a scope and it must be released
 * at the end.
 *
 * If this object is copied, the callback is called when all copies are deleted.
 *
 * @author Kyle Simek
 */
struct Scope_guard
{
    Scope_guard(const boost::function0<void>& callback) : 
        on_exit_(
                static_cast<void*>(0),
                boost::bind(callback))
    {}

    boost::shared_ptr<void> on_exit_;
};

} // namespace kjb;

#endif
