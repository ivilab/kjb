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

/* $Id: pt_body_2d_trajectory.h 18329 2014-12-02 04:29:44Z ksimek $ */

#ifndef PT_BODY_2D_TRAJECTORY_H
#define PT_BODY_2D_TRAJECTORY_H

#include <people_tracking_cpp/pt_complete_trajectory.h>
#include <people_tracking_cpp/pt_trajectory.h>
#include <people_tracking_cpp/pt_body_2d.h>

/** @brief   Classes and functions for dealing with trajectory files.  */

namespace kjb {
namespace pt {

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

typedef Generic_trajectory_map<Body_2d> Body_2d_trajectory_map;
typedef Generic_trajectory<Body_2d> Body_2d_trajectory;
typedef Generic_trajectory_element<Body_2d> Body_2d_trajectory_element;


/** @brief  Convert a Trajectory to a Body_2d_trajectory. */
Body_2d_trajectory get_body_2d_trajectory
(
    const Trajectory& traj,
    const Perspective_camera& cam
);

/** @brief  Convert a Trajectory_map to a body_2d_trajectory_map. */
Body_2d_trajectory_map get_body_2d_trajectory_map
(
    const Trajectory_map& trajs,
    const Perspective_camera& cam
);

}}

namespace kjb {
namespace tracking {

/** @brief  Specialize this parse for Body_2d. */
template <>
inline
bool kjb::pt::Body_2d_trajectory_element::parse(const std::string& /*line*/)
{
//    using namespace std;
//    istringstream istr(line);
//    vector<double> elems;
//
//    copy(istream_iterator<double>(istr), istream_iterator<double>(),
//         back_inserter(elems));
//
//    IFT(elems.size() == 3, Runtime_error, 
//            "Cannot read trajectory element: line has wrong format.");
//
//    if(elems.back() == 0.0)
//        return false;
//
//    value = Body_2d(elems[0], elems[1]); 

    return true;
}

/** @brief  Specialize this write for Body_2d. */
template <>
inline
void kjb::pt::Body_2d_trajectory_element::write(std::ofstream& /*ofs*/) const
{
    //ofs << value << " 1.0" << std::endl;
}

/** @brief  Specialize this write_invalid for Body_2d. */
template <>
inline
void kjb::pt::Body_2d_trajectory_element::write_invalid(std::ofstream& ofs)
{
    ofs << "0.0 0.0 0.0" << std::endl;
}

} }

#endif /*PT_BODY_2D_TRACJECTORY_H */

