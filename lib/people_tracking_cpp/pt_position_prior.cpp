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

/* $Id: pt_position_prior.cpp 21596 2017-07-30 23:33:36Z kobus $ */

#include "l/l_sys_debug.h"  /* For ASSERT */
#include "people_tracking_cpp/pt_position_prior.h"
#include "people_tracking_cpp/pt_scene.h"
#include "people_tracking_cpp/pt_target.h"
#include "people_tracking_cpp/pt_complete_trajectory.h"
#include "people_tracking_cpp/pt_util.h"
#include "gp_cpp/gp_base.h"
#include "gp_cpp/gp_mean.h"
#include "gp_cpp/gp_covariance.h"
#include "gp_cpp/gp_prior.h"
#include "gp_cpp/gp_pdf.h"
#include "prob_cpp/prob_distribution.h"
#include "m_cpp/m_vector.h"
#include "m_cpp/m_vector_d.h"
#include "camera_cpp/perspective_camera.h"
#include "l_cpp/l_exception.h"
#include "l_cpp/l_util.h"
#include "l/l_sys_lib.h"
#include "l/l_sys_io.h"
#include "prob_cpp/prob_pdf.h"
#include "prob_cpp/prob_stat.h"

#include <vector>
#include <boost/foreach.hpp>

using namespace kjb;
using namespace kjb::pt;

double Position_prior::operator()(const Scene& scene) const
{
    // compute space prior (tracks cannot occupy same space)
    double p = at_space(scene);
    p += at_endpoints(scene);

    // compute smoothness prior
    BOOST_FOREACH(const Target& target, scene.association)
    {
        p += at_trajectory(target);
    }

    return p;
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

double Position_prior::at_space(const Scene& scene) const
{
    const Ascn& ascn = scene.association;

    double p = 0;
    for(Ascn::const_iterator tg_p1 = ascn.begin(); tg_p1 != ascn.end(); ++tg_p1)
    {
        size_t sf1 = tg_p1->get_start_time();
        size_t ef1 = tg_p1->get_end_time();
        for(size_t t = sf1; t <= ef1; ++t)
        {
            const Vector3& pos1 = tg_p1->trajectory()[t - 1]->value.position;

            Ascn::const_iterator tg_p2 = tg_p1;
            for(++tg_p2; tg_p2 != ascn.end(); ++tg_p2)
            {
                size_t sf2 = tg_p2->get_start_time();
                size_t ef2 = tg_p2->get_end_time();

                if(t < sf2 || t > ef2) continue;

                const Vector3& pos2 = tg_p2->trajectory()[t - 1]->value.position;
                double dist = vector_distance(pos1, pos2);
                double thresh = tg_p1->trajectory().width/2
                                + tg_p2->trajectory().width/2;
                if(dist <= thresh)
                {
                    p -= sweight_ * (thresh*thresh - dist * dist);
                }
            }
        }
    }

    return p;
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

double Position_prior::at_endpoints(const Scene& scene) const
{
    const Ascn& ascn = scene.association;
    const Perspective_camera& cam = scene.camera;
    //const double prob_magic = -40;
    const double prob_magic = -80;
    const double th = 50;
    const double imw = 1920;
    const double imh = 1080;

    double p = 0.0;
    BOOST_FOREACH(const Target& target, ascn)
    {
        size_t sf = target.get_start_time();
        size_t ef = target.get_end_time();

        if(sf == 1 || ef == target.trajectory().size()) continue;

        // start and end 3D points
        const Vector3& sx = target.trajectory()[sf - 1]->value.position;
        const Vector3& ex = target.trajectory()[ef - 1]->value.position;

        // start and end 2D points
        Vector su = project_point(cam, Vector(sx.begin(), sx.end()));
        Vector eu = project_point(cam, Vector(ex.begin(), ex.end()));

        if(su[0] > -imw/2 + th && su[0] < imw/2 - th
                && su[1] > -imh/2 + th && su[1] < imh/2 - th)
        {
            p += prob_magic;
        }

        if(eu[0] > -imw/2 + th && eu[0] < imw/ - th
                && eu[1] > -imh/2 + th && eu[1] < imh/2 - th)
        {
            p += prob_magic;
        }
    }

    return p;
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

double Position_prior::at_trajectory(const Target& target) const
{
    const Trajectory& traj = target.trajectory();
    int sf = target.get_start_time();
    int ef = target.get_end_time();

    std::vector<Vector> F(ef - sf + 1);
    traj.transform(F.begin(), get_cs_position);
    std::vector<Vector> F_t = get_transpose(F);

    double p = 0.0;
    target.update_pos_gp(gpsc_, gpsv_);
    for(size_t d = 0; d < F_t.size(); d++)
    {
        p += log_pdf(target.pos_prior(), F_t[d]);
    }

    return p;
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

double Position_prior::local(const Target& target, size_t t) const
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
    Vector Fx(ub - lb + 1);
    Vector Fz(ub - lb + 1);
    for(size_t i = lb; i <= ub; i++)
    {
        Fx[i - lb] = traj[i - 1]->value.position[0];
        Fz[i - lb] = traj[i - 1]->value.position[2];
    }

    double p = log_pdf(local_dist_, Fx);
    p += log_pdf(local_dist_, Fz);

    return p;
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

double Position_prior::local_space
(
    const Scene& scene,
    const Target& target,
    size_t t
) const
{
    if(t < target.changed_start() || t > target.changed_end()) return 0.0;

    double p = 0.0;
    const Vector3& pos1 = target.trajectory()[t - 1]->value.position;
    BOOST_FOREACH(const Target& tg, scene.association)
    {
        if(&tg == &target) continue;
        if(t < tg.get_start_time() || t > tg.get_end_time()) continue;

        const Vector3& pos2 = tg.trajectory()[t - 1]->value.position;
        double dist = vector_distance(pos1, pos2);
        double thresh = target.trajectory().width/2 + tg.trajectory().width/2;
        if(dist <= thresh)
        {
            p -= sweight_ * (thresh*thresh - dist * dist);
        }
    }

    return p;
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

double Position_prior::local_endpoints
(
    const Scene& scene,
    const Target& target,
    size_t t
) const
{
    const Perspective_camera& cam = scene.camera;
    //const double prob_magic = -40;
    const double prob_magic = -80;
    const double th = 50;
    const double imw = 1920;
    const double imh = 1080;

    size_t sf = target.get_start_time();
    size_t ef = target.get_end_time();

    if(sf == 1 || ef == target.trajectory().size()) return 0.0;
    if(t != sf && t != ef) return 0.0;

    const Vector3& x = target.trajectory()[t - 1]->value.position;
    Vector u = project_point(cam, Vector(x.begin(), x.end()));

    if(u[0] > -imw/2 + th && u[0] < imw/2 - th
            && u[1] > -imh/2 + th && u[1] < imh/2 - th)
    {
        return prob_magic;
    }

    return 0.0;
}

