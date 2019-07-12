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

/* $Id: pt_face_2d_trajectory.cpp 21596 2017-07-30 23:33:36Z kobus $ */

#include "people_tracking_cpp/pt_trajectory.h"
#include "people_tracking_cpp/pt_complete_trajectory.h"
#include "people_tracking_cpp/pt_face_2d.h"
#include "people_tracking_cpp/pt_face_2d_trajectory.h"

#include <boost/foreach.hpp>

using namespace kjb;
using namespace kjb::pt;

Face_2d_trajectory kjb::pt::get_face_2d_trajectory
(
    const Trajectory& traj,
    const Perspective_camera& cam
)
{
    size_t sz = traj.size();
    Face_2d_trajectory face_2ds(sz);
    face_2ds.height = traj.height;
    face_2ds.width = traj.width;
    face_2ds.girth = traj.girth;

    for(size_t i = 0; i < sz; i++)
    {
        if(traj[i])
        {
            face_2ds[i] = Face_2d_trajectory_element();
            face_2ds[i]->value = project_cstate_face(
                                        traj[i]->value, 
                                        cam, 
                                        traj.height,
                                        traj.width, 
                                        traj.girth);
        }
    }

    return face_2ds;
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

Face_2d_trajectory_map get_face_2d_trajectory_map
(
    const Trajectory_map& trajs,
    const Perspective_camera& cam
)
{
    Face_2d_trajectory_map face_2d_trajs;
    face_2d_trajs.duration() = trajs.duration();

    BOOST_FOREACH(const Trajectory_map::value_type& p, trajs)
    {
        face_2d_trajs[p.first] = get_face_2d_trajectory(p.second, cam);
    }

    return face_2d_trajs;
}

