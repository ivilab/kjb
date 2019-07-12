/* $Id$ */
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

#include <algorithm>
#include <numeric>

#ifndef KJB_CPP_L_STD_PARALLEL

// need g++ 4.3 or newer
// need -fopenmp load flag
// need -D_GLIBCXX_PARALLEL compile flag
#ifdef KJB_USE_GNU_PARALLEL
namespace kjb_parallel_std = __gnu_parallel;
#else
namespace kjb_parallel_std = std;
#endif

#endif
