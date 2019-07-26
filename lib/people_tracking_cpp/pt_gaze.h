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

/* $Id: pt_gaze.h 20912 2016-10-30 17:52:31Z ernesto $ */

#ifndef PT_GAZE_H
#define PT_GAZE_H

#include <people_tracking_cpp/pt_complete_trajectory.h>
#include <people_tracking_cpp/pt_box_trajectory.h>
#include <people_tracking_cpp/pt_entity.h>
#include <m_cpp/m_vector_d.h>
#include <detector_cpp/d_facecom.h>
#include <detector_cpp/d_bbox.h>

#include <vector>

namespace kjb
{
namespace pt
{

/** @brief Get the corresponding person based on the face box */
bool get_corresponding_entity
(
    const Bbox& face_box,
    const Box_trajectory_map& body_trajs,
    Entity_id& entity_id,
    size_t index,
    double overlapping_threshold = 0.4
);

/** @brief Estimate the gaze direction from face.com */
void estimate_gaze_direction_from_data
(
    const Box_trajectory_map& btrajs,
    Trajectory_map& trajs,
    const std::vector<std::vector<Face_detection> >& face_data
);

/** @brief Estimate the gaze direction from face.com */
template<class InputIterator>
void estimate_gaze_direction_from_data
(
    InputIterator first,
    InputIterator last,
    const Box_trajectory_map& btrajs,
    Trajectory_map& trajs,
    size_t frame
)
{
    for(InputIterator it = first; it != last; it++)
    {
        Entity_id entity_id;
        if(get_corresponding_entity(it->box(), btrajs, entity_id, frame))
        {
            (trajs.find(entity_id)->second).at(frame)->value.face_dir
                = Vector2(it->yaw(), it->pitch());
        }
    }
}

}} //namespace kjb::pt

#endif /*PT_GAZE_H */

