/* $Id: options.h 17393 2014-08-23 20:19:14Z predoehl $ */
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
#ifndef KJB_EDGE2_OPTIONS_H
#define KJB_EDGE2_OPTIONS_H

#ifdef KJB_HAVE_BST_POPTIONS

#include <boost/program_options.hpp>
#include <boost/assign/list_of.hpp>
#include <edge_cpp/edge.h>

namespace kjb
{

boost::program_options::options_description
get_canny_edge_detector_options();

Canny_edge_detector make_canny_edge_detector(const boost::program_options::variables_map& options);

 
}

#endif /* KJB_HAVE_BST_POPTIONS */

#endif

