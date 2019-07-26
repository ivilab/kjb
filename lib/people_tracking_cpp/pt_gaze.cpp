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

/* $Id: pt_gaze.cpp 21596 2017-07-30 23:33:36Z kobus $ */

#include "l/l_sys_debug.h"  /* For ASSERT */
#include "people_tracking_cpp/pt_gaze.h"
#include "people_tracking_cpp/pt_detection_box.h"
#include "people_tracking_cpp/pt_box_trajectory.h"
#include "people_tracking_cpp/pt_complete_trajectory.h"
#include "people_tracking_cpp/pt_entity.h"
#include "m_cpp/m_vector_d.h"
#include "detector_cpp/d_facecom.h"

#include <vector>

using namespace kjb;
using namespace kjb::pt; 

bool kjb::pt::get_corresponding_entity
(
    const Bbox& face_box,
    const Box_trajectory_map& body_trajs,
    Entity_id& entity_id,
    size_t frame,
    double overlapping_threshold
)
{
    ASSERT(frame >= 1);
    Box_trajectory_map::const_iterator traj_it;
    double max_overlapping = 0.0;

    for(traj_it = body_trajs.begin(); traj_it != body_trajs.end(); traj_it++)
    {
        if(traj_it->second[frame-1])
        {
            const Bbox& cur_body_box = traj_it->second[frame-1]->value;
            double face_area = face_box.get_width()*face_box.get_height();
            double overlapping_area
                    = get_rectangle_intersection(face_box, cur_body_box); 
            double overlapping = overlapping_area/face_area;
            if(overlapping > max_overlapping)
            {
                // get the key
                entity_id = traj_it->first;
                max_overlapping = overlapping;
            }
        }
    }
    if(max_overlapping > overlapping_threshold)
        return true;
    else
        return false;
}

/** /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ */

void kjb::pt::estimate_gaze_direction_from_data
(
    const Box_trajectory_map& btrajs,
    Trajectory_map& trajs,
    const std::vector<std::vector<Face_detection> >& face_data
)
{
    IFT(trajs.duration() == face_data.size(), Runtime_error, 
        "Trajectory and face data does not have the same number of frames.");

    for(size_t i = 0; i < face_data.size(); i++)
    {
        for(size_t j = 0; j < face_data[i].size(); j++)
        {
            Entity_id entity_id; 
            if(get_corresponding_entity(face_data[i][j].box(),
                                        btrajs, entity_id, i+1))
            {
                // set the yaw and pitch 
                (trajs.find(entity_id)->second).at(i)->value.face_dir = 
                    Vector2(face_data[i][j].yaw(),face_data[i][j].pitch());
                (trajs.find(entity_id)->second).at(i)->value.face_com = true;
            }
        }
    }
}
