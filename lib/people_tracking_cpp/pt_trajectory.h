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

/* $Id: pt_trajectory.h 20005 2015-11-01 07:25:30Z jguan1 $ */

#ifndef PT_TRAJECTORY_H
#define PT_TRAJECTORY_H

#include <tracking_cpp/tracking_trajectory.h>

namespace kjb {
namespace pt {

using namespace kjb::tracking;

typedef Generic_trajectory_map<double> Angle_trajectory_map;
typedef Generic_trajectory<double> Angle_trajectory;
typedef Generic_trajectory_element<double> Angle_trajectory_element;

}} //namespace kjb::pt

#endif /*PT_TRAJECTORY_H */

