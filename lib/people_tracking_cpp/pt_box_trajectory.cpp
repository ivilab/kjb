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
   |  Author:  Kyle Simek, Ernesto Brau
 * =========================================================================== */

/*$Id: pt_box_trajectory.cpp 21596 2017-07-30 23:33:36Z kobus $ */

#include "people_tracking_cpp/pt_box_trajectory.h"
#include "people_tracking_cpp/pt_complete_trajectory.h"
#include "people_tracking_cpp/pt_body_2d.h"
#include "camera_cpp/perspective_camera.h"
#include "g_cpp/g_util.h"

#include <boost/foreach.hpp>

using namespace kjb;
using namespace kjb::pt;

Box_trajectory kjb::pt::get_body_box_trajectory
(
    const Trajectory& traj,
    const Perspective_camera& cam
)
{
    size_t sz = traj.size();
    Box_trajectory boxes(sz);
    boxes.height = traj.height;
    boxes.width = traj.width;
    boxes.girth = traj.girth;

    for(size_t i = 0; i < sz; i++)
    {
        if(traj[i])
        {
            Body_2d b2d = project_cstate(
                                    traj[i]->value, cam,
                                    traj.height,
                                    traj.width,
                                    traj.girth);

            boxes[i] = Box_trajectory_element();
            boxes[i]->value = b2d.full_bbox;
        }
    }

    return boxes;
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

Box_trajectory_map kjb::pt::get_body_box_trajectory_map
(
    const Trajectory_map& trajs,
    const Perspective_camera& cam
)
{
    Box_trajectory_map box_trajs;
    box_trajs.duration() = trajs.duration();

    BOOST_FOREACH(const Trajectory_map::value_type& p, trajs)
    {
        box_trajs[p.first] = get_body_box_trajectory(p.second, cam);
    }

    return box_trajs;
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

Box_trajectory kjb::pt::get_face_box_trajectory
(
    const Trajectory& traj,
    const Perspective_camera& cam
)
{
    size_t sz = traj.size();
    Box_trajectory boxes(sz);
    boxes.height = traj.height;
    boxes.width = traj.width;
    boxes.girth = traj.girth;

    const Matrix& P = cam.build_camera_matrix();
    double bsize = traj.height / 7.0;

    for(size_t i = 0; i < sz; i++)
    {
        if(traj[i])
        {
            const Vector3& pos = traj[i]->value.position;
            Vector v(pos[0], traj.height - (bsize / 2.0), pos[2], 1.0);
            Vector bcenter = geometry::projective_to_euclidean_2d(P * v);

            Vector u(pos[0], traj.height - bsize, pos[2], 1.0);
            Vector bbot = geometry::projective_to_euclidean_2d(P * u);

            double bsizepx = 2 * fabs(bcenter[1] - bbot[1]);
            boxes[i] = Box_trajectory_element();
            boxes[i]->value.set_center(bcenter);
            boxes[i]->value.set_height(bsizepx);
            boxes[i]->value.set_width(bsizepx);
        }
    }

    return boxes;
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

Box_trajectory_map kjb::pt::get_face_box_trajectory_map
(
    const Trajectory_map& trajs,
    const Perspective_camera& cam
)
{
    Box_trajectory_map box_trajs;
    box_trajs.duration() = trajs.duration();

    BOOST_FOREACH(const Trajectory_map::value_type& p, trajs)
    {
        box_trajs[p.first] = get_face_box_trajectory(p.second, cam);
    }

    return box_trajs;
}

