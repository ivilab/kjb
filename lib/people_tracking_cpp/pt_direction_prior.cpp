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
* =========================================================================== */

/* $Id: pt_direction_prior.cpp 21596 2017-07-30 23:33:36Z kobus $ */

#include "l/l_sys_debug.h"  /* For ASSERT */
#include "people_tracking_cpp/pt_direction_prior.h"
#include "people_tracking_cpp/pt_scene.h"
#include "people_tracking_cpp/pt_target.h"
#include "people_tracking_cpp/pt_complete_trajectory.h"
#include "gp_cpp/gp_base.h"
#include "gp_cpp/gp_mean.h"
#include "gp_cpp/gp_covariance.h"
#include "gp_cpp/gp_prior.h"
#include "gp_cpp/gp_pdf.h"
#include "prob_cpp/prob_distribution.h"
#include "m_cpp/m_vector.h"
#include "l_cpp/l_exception.h"
#include "l_cpp/l_util.h"
#include "l/l_sys_lib.h"
#include "prob_cpp/prob_pdf.h"
#include "prob_cpp/prob_stat.h"

#include <vector>
#include <boost/foreach.hpp>
#include <boost/bind.hpp>

using namespace kjb;
using namespace kjb::pt;

double Direction_prior::operator()(const Scene& scene) const
{
    double p = 0.0;
    BOOST_FOREACH(const Target& target, scene.association)
    {
        p += at_trajectory(target);
    }

    return p;
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

double Direction_prior::at_trajectory(const Target& target) const
{
    const Trajectory& traj = target.trajectory();
    //const Angle_trajectory& atraj = target.wangle_trajectory();
    int sf = target.get_start_time();
    int ef = target.get_end_time();

    Vector F(ef - sf + 1);
    for(size_t t = sf; t <= ef; t++)
    {
        //F[t - sf] = traj[t - 1]->value.body_dir - atraj[t - 1]->value;
        //F[t - sf] = traj[t - 1]->value.body_dir;
        F[t - sf] = traj[t - 1]->value.body_dir;
    }

    double p = 0.0;
    target.update_dir_gp(gpsc_, gpsv_);
    p = log_pdf(target.dir_prior(), F);

    return p;
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

double Direction_prior::local(const Target& target, size_t t) const
{
    const Trajectory& traj = target.trajectory();
    //const Angle_trajectory& atraj = target.wangle_trajectory();
    int sf = target.get_start_time();
    int ef = target.get_end_time();
    KJB(ASSERT(ef != -1 && sf != -1));
    ASSERT(m_local_sz % 2 != 0);

    // target too short -- use regular prior
    if(ef - sf + 1 <= m_local_sz)
    {
        return at_trajectory(target);
    }

    // initialize local prior
    if(local_dist_.normal().get_dimension() != m_local_sz)
    {
        gp::Inputs X = gp::make_inputs(1, m_local_sz);
        local_dist_.set_inputs(X.begin(), X.end());
    }

    // compute bounds, making sure window is in target
    int lb = static_cast<int>(t) - static_cast<int>(m_local_sz/2);
    int ub = static_cast<int>(t) + static_cast<int>(m_local_sz/2);

    if(lb < static_cast<int>(sf))
    {
        int d = static_cast<int>(sf) - lb;
        lb += d;
        ub += d;
    }
    else if(ub > static_cast<int>(ef))
    {
        int d = ub - static_cast<int>(ef);
        lb -= d;
        ub -= d;
    }

    ASSERT(lb >= sf && lb <= ef);

    // compute prior
    Vector F(ub - lb + 1);
    for(size_t i = lb; i <= ub; i++)
    {
        //F[i - lb] = traj[i - 1]->value.body_dir - atraj[i - 1]->value;
        F[i - lb] = traj[i - 1]->value.body_dir;
    }

    double p = log_pdf(local_dist_, F);
    return p;
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

double Face_direction_prior::operator()(const Scene& scene) const
{
    double p = 0.0;
    BOOST_FOREACH(const Target& target, scene.association)
    {
        p += at_trajectory(target);
    }

    return p;
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

double Face_direction_prior::at_trajectory(const Target& target) const
{
    const Trajectory& traj = target.trajectory();
    int sf = target.get_start_time();
    int ef = target.get_end_time();

    Vector F0(ef - sf + 1);
    Vector F1(ef - sf + 1);
    for(size_t t = sf; t <= ef; t++)
    {
        F0[t - sf] = traj[t - 1]->value.face_dir[0];
        F1[t - sf] = traj[t - 1]->value.face_dir[1];
    }

    double p = 0.0;
    target.update_fdir_gp(gpsc_, gpsv_);
    p += log_pdf(target.fdir_prior(), F0);
    p += log_pdf(target.fdir_prior(), F1);

    return p;
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

double Face_direction_prior::local(const Target& target, size_t t) const
{
    const Trajectory& traj = target.trajectory();
    int sf = target.get_start_time();
    int ef = target.get_end_time();
    KJB(ASSERT(ef != -1 && sf != -1));
    ASSERT(m_local_sz % 2 != 0);

    // target too short -- use regular prior
    if(ef - sf + 1 <= m_local_sz)
    {
        return at_trajectory(target);
    }

    // initialize local prior
    if(local_dist_.normal().get_dimension() != m_local_sz)
    {
        gp::Inputs X = gp::make_inputs(1, m_local_sz);
        local_dist_.set_inputs(X.begin(), X.end());
    }

    // compute bounds, making sure window is in target
    int lb = static_cast<int>(t) - static_cast<int>(m_local_sz/2);
    int ub = static_cast<int>(t) + static_cast<int>(m_local_sz/2);

    if(lb < static_cast<int>(sf))
    {
        int d = static_cast<int>(sf) - lb;
        lb += d;
        ub += d;
    }
    else if(ub > static_cast<int>(ef))
    {
        int d = ub - static_cast<int>(ef);
        lb -= d;
        ub -= d;
    }

    ASSERT(lb >= sf && lb <= ef);

    // compute prior
    Vector F0(ub - lb + 1);
    Vector F1(ub - lb + 1);
    for(size_t i = lb; i <= ub; i++)
    {
        F0[i - lb] = traj[i - 1]->value.face_dir[0];
        F1[i - lb] = traj[i - 1]->value.face_dir[1];
    }

    double p = log_pdf(local_dist_, F0);
    p += log_pdf(local_dist_, F1);

    return p;
}

