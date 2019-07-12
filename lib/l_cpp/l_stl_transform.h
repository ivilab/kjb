/* $Id: l_stl_transform.h 10621 2011-09-29 19:50:52Z predoehl $ */
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

/**
 * @file A set of general purpose unary functions for use with std::transform
 */
#ifndef KJB_CPP_L_STL_TRANSFORM
#define KJB_CPP_L_STL_TRANSFORM

namespace kjb
{

/// Convert to a pointer
struct to_ptr
{
template <class T> 
T* operator()(T& in) { return &in; }
};

} // namespace kjb
#endif
