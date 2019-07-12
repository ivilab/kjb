/* =========================================================================== *
|
|  Copyright (c) 1994-2008 by Kobus Barnard (author).
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
| Authors:
|     Jinyan Guan, Ernesto Brau
|
* =========================================================================== */

/* $Id: pt_optical_flow_likelihood.cpp 21596 2017-07-30 23:33:36Z kobus $ */

#include "l/l_sys_debug.h"  /* For ASSERT */
#include "people_tracking_cpp/pt_optical_flow_likelihood.h"
#include "people_tracking_cpp/pt_body_2d_trajectory.h"
#include "people_tracking_cpp/pt_visibility.h"
#include "people_tracking_cpp/pt_body_2d.h"
#include "people_tracking_cpp/pt_scene.h"
#include "people_tracking_cpp/pt_target.h"
#include "people_tracking_cpp/pt_util.h"
#include "flow_cpp/flow_integral_flow.h"
#include "prob_cpp/prob_distribution.h"
#include "prob_cpp/prob_pdf.h"
#include "detector_cpp/d_bbox.h"
#include "m_cpp/m_vector.h"

#include <vector>
#include <boost/foreach.hpp>

using namespace kjb;
using namespace kjb::pt;

double Optical_flow_likelihood::operator()(const Scene& scene) const
{
    double ll = 0.0;
    BOOST_FOREACH(const Target& tg, scene.association)
    {
        ll += at_trajectory(tg);
    }

    return ll;
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

double Optical_flow_likelihood::at_trajectory(const Target& target) const
{
    const Body_2d_trajectory btraj = target.body_trajectory();
    size_t sf = target.get_start_time();
    size_t ef = target.get_end_time();

    double ll = 0.0;
    for(size_t cur_frame = sf; cur_frame <= ef - 1; cur_frame++)
    {
        ll += at_box(btraj[cur_frame - 1]->value, cur_frame);
    }

    return ll;
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

double Optical_flow_likelihood::at_box
(
    const Body_2d& b2d,
    size_t cur_frame
) const
{
    const Visibility& vis = b2d.visibility;
    const Vector& mdir = b2d.model_dir;

    if(vis.visible == 0.0) return 0.0;
    ASSERT(!mdir.empty());

    Vector flow_velocity = average_box_flow(
                                    m_flows_x[cur_frame - 1],
                                    m_flows_y[cur_frame - 1],
                                    b2d.body_bbox,
                                    vis);

    Vector model_velocity = mdir;
    model_velocity[1] = -model_velocity[1];

    double cll = 0.0;
    double error_x = flow_velocity[0] - model_velocity[0];
    cll += log_pdf(m_x_dist, error_x);
    cll -= log_pdf(m_bg_x_dist, flow_velocity[0]);

    double error_y = flow_velocity[1] - model_velocity[1];
    cll += log_pdf(m_y_dist, error_y);
    cll -= log_pdf(m_bg_y_dist, flow_velocity[1]);

    return cll * vis.visible;
}

