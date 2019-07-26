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

#include "bbb_cpp/bbb_association.h"
#include "bbb_cpp/bbb_traj_set.h"

#include <algorithm>

#include "l_cpp/l_exception.h"

using namespace kjb;
using namespace kjb::bbb;

void Association::check_consistent() const
{
    const Traj_set& trajs0 = group(0).trajectories();
    Traj_set all(trajs0.begin(), trajs0.end());

    size_t K = groups_.size();
    for(size_t k = 1; k < K; k++)
    {
        const Traj_set& trajsk = group(k).trajectories();
        all.insert(trajsk.begin(), trajsk.end());
    }

    bool eq = std::equal(trajs_.begin(), trajs_.end(), all.begin());

    IFT(eq, Runtime_error, "Association is inconsistent!");
}

