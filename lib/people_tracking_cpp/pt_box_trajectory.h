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
   |  Author:  Kyle Simek, Ernesto Brau, Jinyan Guan
 * =========================================================================== */

/* $Id: pt_box_trajectory.h 18329 2014-12-02 04:29:44Z ksimek $ */

#ifndef PT_BOX_TRAJECTORY_H
#define PT_BOX_TRAJECTORY_H

#include <people_tracking_cpp/pt_trajectory.h>
#include <people_tracking_cpp/pt_complete_trajectory.h>
#include <camera_cpp/perspective_camera.h>
#include <m_cpp/m_vector.h>
#include <l_cpp/l_exception.h>
#include <detector_cpp/d_bbox.h>
#include <vector>
#include <limits>
#include <fstream>
#include <string>
#include <ios>
#include <iomanip>

/** @file   Classes and functions for dealing with trajectory files. */

namespace kjb {
namespace pt {

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

typedef Generic_trajectory_map<Bbox> Box_trajectory_map;
typedef Generic_trajectory<Bbox> Box_trajectory;
typedef Generic_trajectory_element<Bbox> Box_trajectory_element;

/** @brief  Convert a Trajectory to a Box_trajectory. */
Box_trajectory get_body_box_trajectory
(
    const Trajectory& traj,
    const Perspective_camera& cam
);

/** @brief  Convert a Trajectory_map to a Box_trajectory_map. */
Box_trajectory_map get_body_box_trajectory_map
(
    const Trajectory_map& trajs,
    const Perspective_camera& cam
);

/** @brief  Convert a Trajectory to a Box_trajectory. */
Box_trajectory get_face_box_trajectory
(
    const Trajectory& traj,
    const Perspective_camera& cam
);

/** @brief  Convert a Trajectory_map to a Box_trajectory_map. */
Box_trajectory_map get_face_box_trajectory_map
(
    const Trajectory_map& trajs,
    const Perspective_camera& cam
);


}} //namespace kjb::pt

namespace kjb {
namespace tracking {

/** @brief  Specialize this parse for Bbox. */
template <>
inline
bool kjb::pt::Box_trajectory_element::parse(const std::string& line)
{
    using namespace std;
    istringstream istr(line);
    vector<double> elems;

    copy(istream_iterator<double>(istr), istream_iterator<double>(),
         back_inserter(elems));

    IFT(elems.size() == 5, Runtime_error, 
            "Cannot read trajectory element: line has wrong format.");

    if(elems.back() == 0.0)
        return false;

    // else
    Vector center(elems[0], elems[1]);
    double width = elems[2]; 
    double height = elems[3];
    value = Bbox(center, width, height);

    return true;
}

/** @brief  Specialize this write for Bbox. */
template <>
inline
void kjb::pt::Box_trajectory_element::write(std::ofstream& ofs) const
{
    ofs << value << " 1.0" << std::endl;
}

/** @brief  Specialize this write_invalid for Bbox. */
template <>
inline
void kjb::pt::Box_trajectory_element::write_invalid(std::ofstream& ofs)
{
    ofs << "-1.0 -1.0 0.0 0.0 0.0" << std::endl;
}

} }


#endif /*PT_BOX_TRAJECTORY_H */

