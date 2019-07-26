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
   |  Author:  Jinyan Guan
 * =========================================================================== */

/* $Id$ */

#include "people_tracking_cpp/pt_trajectory.h"
#include "people_tracking_cpp/pt_complete_trajectory.h"
#include "people_tracking_cpp/pt_complete_state.h"
#include "people_tracking_cpp/pt_body_2d.h"
#include "people_tracking_cpp/pt_body_2d_trajectory.h"

#include <boost/foreach.hpp>

using namespace kjb;
using namespace kjb::pt;

Body_2d_trajectory kjb::pt::get_body_2d_trajectory
(
    const Trajectory& traj,
    const Perspective_camera& cam
)
{
    size_t sz = traj.size();
    Body_2d_trajectory body_2ds(sz);
    body_2ds.height = traj.height;
    body_2ds.width = traj.width;
    body_2ds.girth = traj.girth;

    for(size_t i = 0; i < sz; i++)
    {
        if(traj[i])
        {
            body_2ds[i] = Body_2d_trajectory_element();
            body_2ds[i]->value = project_cstate(
                                        traj[i]->value, 
                                        cam, 
                                        traj.height,
                                        traj.width, 
                                        traj.girth);
        }
    }

    return body_2ds;
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

Body_2d_trajectory_map get_body_2d_trajectory_map
(
    const Trajectory_map& trajs,
    const Perspective_camera& cam
)
{
    Body_2d_trajectory_map body_2d_trajs;
    body_2d_trajs.duration() = trajs.duration();

    BOOST_FOREACH(const Trajectory_map::value_type& p, trajs)
    {
        body_2d_trajs[p.first] = get_body_2d_trajectory(p.second, cam);
    }

    return body_2d_trajs;
}

