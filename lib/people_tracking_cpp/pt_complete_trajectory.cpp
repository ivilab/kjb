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

/* $Id: pt_complete_trajectory.cpp 21596 2017-07-30 23:33:36Z kobus $ */

#include "l/l_sys_debug.h"  /* For ASSERT */
#include "people_tracking_cpp/pt_complete_trajectory.h"
#include "people_tracking_cpp/pt_visibility.h"
#include "l_cpp/l_exception.h"
#include "l_cpp/l_util.h"
#include "m_cpp/m_vector.h"
#include "m_cpp/m_vector_d.h"

#include <string>
#include <sstream>
#include <vector>
#include <algorithm>
#include <functional>
#include <iterator>
#include <limits>

using namespace kjb;
using namespace kjb::pt;

namespace kjb {
namespace tracking {
template <>
bool Trajectory_element::parse(const std::string& line)
{
    using namespace std;

    istringstream istr(line);
    vector<double> elems;

    copy(istream_iterator<double>(istr), istream_iterator<double>(),
         back_inserter(elems));

    KJB(ASSERT(elems.size() > 3));
    IFT(elems.size() <= 7 && elems.size() != 6, Runtime_error,
        "Cannot read trajectory element: line has wrong format.");

    if(elems.back() == 0.0)
    {
        return false;
    }

    // defaults
    value.body_dir = std::numeric_limits<double>::max();
    value.face_dir[0] = value.face_dir[1] = std::numeric_limits<double>::max();

    copy(elems.begin(), elems.begin() + 3, value.position.begin());

    if(elems.size() > 4)
    {
        value.body_dir = elems[3];
    }

    if(elems.size() > 5)
    {
        copy(elems.begin() + 4, elems.begin() + 6, value.face_dir.begin());
    }

    return true;
} 

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

template <>
bool Trajectory::parse_header(const std::string& line)
{
    using namespace std;

    istringstream istr(line);
    vector<double> elems;

    copy(istream_iterator<double>(istr), istream_iterator<double>(),
         back_inserter(elems));

    IFT(istr.eof() || !istr.fail(), IO_error,
        "Trajectory line has invalid format.");

    if(elems.size() > 3)
    {
        return false;
    }

    height = elems[0];

    if(elems.size() > 1)
    {
        width = elems[1];
    }

    if(elems.size() > 2)
    {
        girth = elems[2];
    }

    return true;
}
}}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

double kjb::pt::get_initial_direction
(
    Trajectory& traj, 
    size_t frame1, 
    size_t frame2
)
{
    /*size_t sz = traj.size();
    size_t i = frame - 1;

    IFT(traj[i], Illegal_argument,
        "Cannot estimate direction; frame out of range");

    IFT(i + 1 != sz && traj[i + 1], Illegal_argument,
        "Cannot compute default direction for final frame of trajectory.");

    Vector from(traj[i]->value.position.begin(), traj[i]->value.position.end());
    Vector to(traj[i+1]->value.position.begin(), traj[i+1]->value.position.end());

    return body_direction(to - from);*/
    size_t sz = traj.size();

    IFT(traj[frame1 - 1], Illegal_argument,
        "Cannot estimate direction; frame out of range");

    IFT(frame2 != sz + 1 && traj[frame2 - 1], Illegal_argument,
        "Cannot compute default direction for final frame of trajectory.");

    Vector from(traj[frame1 - 1]->value.position.begin(), traj[frame1 - 1]->value.position.end());
    Vector to(traj[frame2 - 1]->value.position.begin(), traj[frame2 - 1]->value.position.end());

    return body_direction(to - from);
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

void kjb::pt::update_direction(Trajectory& traj, size_t frame, bool ow)
{
    size_t sz = traj.size();
    size_t t = frame - 1;
    if(traj[t])
    {
        if(ow || traj[t]->value.body_dir == std::numeric_limits<double>::max())
        {
            // if i'm at end or the next frame does not have valid element
            if(t + 1 == sz || !traj[t + 1])
            {
                // if i'm (also) at start or there's no previous valid element
                if(t == 0 || !traj[t - 1])
                {
                    traj[t]->value.body_dir = 0.0;
                    traj[t]->value.face_dir = Vector2(0.0, 0.0);
                }
                else
                { 
                    ASSERT(t != 0);
                    ASSERT(traj[t - 1]);
                    // we assume previous one is set
                    traj[t]->value.body_dir = traj[t - 1]->value.body_dir;
                    traj[t]->value.face_dir = traj[t - 1]->value.face_dir;
                }
            }
            else
            {
                double ang = get_initial_direction(traj, frame, frame + 1);
                //double ang = 0.0;
                traj[t]->value.body_dir = ang;
                traj[t]->value.face_dir = Vector2(0.0, 0.0);
            }
        }
    }
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

void kjb::pt::initialize_directions
(
    Trajectory& traj, 
    size_t st, 
    size_t et, 
    bool ow
)
{
    int sz = traj.size();
    int ret = et == 0 ? sz - 1 : et; 
    for(size_t i = st; i <= ret; i++)
    {
        update_direction(traj, i, ow);
    }
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

/*std::vector<Visibility> kjb::pt::get_visibilities(const Trajectory& traj)
{
    size_t sf = traj.start_time();
    size_t ef = traj.end_time();
    std::vector<Visibility> viss;
    viss.reserve(ef - sf + 1);
    for(size_t i = sf - 1; i <= ef - 1; i++)
    {
        ASSERT(traj[i]);
        viss.push_back((traj[i]->value).visibility); 
    }

    return viss; 
}*/

