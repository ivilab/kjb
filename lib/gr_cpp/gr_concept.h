/* $Id: gr_concept.h 11211 2011-11-22 17:56:38Z ksimek $ */
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

#ifndef KJB_GR_CPP_GR_CONCEPT_H
#define KJB_GR_CPP_GR_CONCEPT_H

#include <boost/concept_check.hpp> 

namespace kjb 
{
template <class X>
struct RenderableObject
{
    BOOST_CONCEPT_USAGE(RenderableObject)
    {
        render(*i);
    }
private:
    const X* i;
};
}

#endif
