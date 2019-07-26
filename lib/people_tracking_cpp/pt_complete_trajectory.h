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

/* $Id: pt_complete_trajectory.h 20021 2015-11-03 05:37:19Z jguan1 $ */

#ifndef PT_COMPLETE_TRAJECTORY_H
#define PT_COMPLETE_TRAJECTORY_H

#include <people_tracking_cpp/pt_trajectory.h>
#include <people_tracking_cpp/pt_complete_state.h>
#include <people_tracking_cpp/pt_visibility.h>
#include <l_cpp/l_exception.h>
#include <m_cpp/m_vector.h>
#include <vector>
#include <limits>
#include <fstream>
#include <string>
#include <ios>
#include <iomanip>

/** @file   Classes and functions for dealing with trajectory files. */

namespace kjb {
namespace pt {

typedef Generic_trajectory_element<Complete_state> Trajectory_element;
typedef Generic_trajectory<Complete_state> Trajectory;
typedef Generic_trajectory_map<Complete_state> Trajectory_map;

/** @brief  Gets default body direction at frame. */
//double get_initial_direction(Trajectory& traj, size_t frame);
double get_initial_direction(Trajectory& traj, size_t frame1, size_t frame2);

/** @brief  Set non-initialized direction in frame to default value. */
void update_direction(Trajectory& traj, size_t frame, bool ow);

/** @brief  Set non-initialized directions to default values. */
void initialize_directions(Trajectory& traj, size_t st = 0, size_t et = 0, bool ow = false);

/** @brief  Extract visibility information from a trajectory. */
//std::vector<Visibility> get_visibilities(const Trajectory& traj);

}} //namespace kjb::pt

namespace kjb {
namespace tracking {
/** @brief  Specialize this parse for Complete_state. */
template <>
bool kjb::pt::Trajectory_element::parse(const std::string& line);

/** @brief  Specialize this write for Complete_state. */
template <>
inline
void kjb::pt::Trajectory_element::write(std::ofstream& ofs) const
{
    IFT(value.body_dir != std::numeric_limits<double>::max()
            && value.face_dir[0] != std::numeric_limits<double>::max()
            && value.face_dir[1] != std::numeric_limits<double>::max(),
        Illegal_argument, "Cannot write trajectory: body or face"
                          " direction never initialized.");

    ofs << std::scientific;
    ofs << value.position
        << "  " << std::setw(16) << std::setprecision(8) << value.body_dir 
        << "  " << value.face_dir
        << " 1.0" << std::endl;
}

/** @brief  Specialize this write_invalid for Complete_state. */
template <>
inline
void kjb::pt::Trajectory_element::write_invalid(std::ofstream& ofs)
{
    ofs << Vector(6, -1.0) << "  " << 0.0 << std::endl;
}

/** @brief  Specialize this parse_header for Complete_state. */
template <>
bool kjb::pt::Trajectory::parse_header(const std::string& line);

/** @brief  Specialize this write_header for Complete_state. */
template <>
inline
void kjb::pt::Trajectory::write_header(std::ofstream& ofs) const
{
    ofs << height << " " << width << " " << girth << std::endl;
}

}}
#endif /*PT_COMPLETE_TRAJECTORY_H */

