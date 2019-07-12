/* =========================================================================== *
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
   |  Author:  Ernesto Brau
 * =========================================================================== */

/* $Id$ */

#include "bbb_cpp/bbb_trajectory.h"
#include <iostream>

using namespace kjb;
using namespace kjb::bbb;

std::ostream& kjb::bbb::operator<<(std::ostream& ost, const Trajectory& traj)
{
    for(size_t t = traj.start(); t <= traj.end(); t++)
    {
        ost << t << " " << traj.pos(t) << std::endl;
    }

    return ost;
}

