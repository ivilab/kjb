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
|     Ernesto Brau, Jinyan Guan
|
* =========================================================================== */

/* $Id$ */

#include "l/l_sys_debug.h"  /* For ASSERT */
#include "people_tracking_cpp/pt_sample_scenes.h"
#include "people_tracking_cpp/pt_scene.h"
#include "people_tracking_cpp/pt_scene_adapter.h"
#include "people_tracking_cpp/pt_association.h"
#include "people_tracking_cpp/pt_target.h"
#include "people_tracking_cpp/pt_scene_diff.h"
#include "prob_cpp/prob_distribution.h"
#include "prob_cpp/prob_sample.h"
#include "l_cpp/l_exception.h"

#include <string>
#include <vector>
#include <algorithm>
#include <boost/foreach.hpp>
#include <boost/bind.hpp>
#include <boost/lexical_cast.hpp>

#ifdef KJB_HAVE_ERGO
#include <ergo/mh.h>
#endif

using namespace kjb;
using namespace kjb::pt;
using namespace kjb::mcmcda;

bool kjb::pt::all_new_targets(const Scene& scene)
{
    BOOST_FOREACH(const Target& tg, scene.association)
    {
        if(tg.changed()
                && (tg.changed_start() != tg.get_start_time()
                || tg.changed_end() != tg.get_end_time()))
        {
            return false;
        }
    }

    return true;
}

///* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */
//
//Vector get_trajectory(const Target& tg, size_t idx)
//{
//    const size_t st = tg.get_start_time();
//    const size_t et = tg.get_end_time();
//
//    Vector tj(et - st + 1);
//    for(size_t t = st; t <= et; ++t)
//    {
//        double x;
//        switch(idx)
//        {
//            case 0: x = tg.trajectory()[t - 1]->value.position[0];
//                    break;
//
//            case 1: x = tg.trajectory()[t - 1]->value.position[1];
//                    break;
//
//            case 2: x = tg.trajectory()[t - 1]->value.body_dir;
//                    break;
//
//            case 3: x = tg.trajectory()[t - 1]->value.face_dir[0];
//                    break;
//
//            case 4: x = tg.trajectory()[t - 1]->value.face_dir[1];
//                    break;
//        }
//
//        tj[t - st] = x;
//    }
//
//    return tj;
//}
//
///* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */
//
//void set_trajectory(Target& tg, size_t idx, const Vector& tj)
//{
//    const size_t st = tg.get_start_time();
//    const size_t et = tg.get_end_time();
//    assert(tj.size() == et - st + 1);
//
//    for(size_t t = st; t <= et; ++t)
//    {
//        double x = tj[t - st];
//        switch(idx)
//        {
//            case 0: tg.trajectory()[t - 1]->value.position[0] = x;
//                    break;
//
//            case 1: tg.trajectory()[t - 1]->value.position[1] = x;
//                    break;
//
//            case 2: tg.trajectory()[t - 1]->value.body_dir = x;
//                    break;
//
//            case 3: tg.trajectory()[t - 1]->value.face_dir[0] = x;
//                    break;
//
//            case 4: tg.trajectory()[t - 1]->value.face_dir[1] = x;
//                    break;
//        }
//    }
//}
//
/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

#ifdef KJB_HAVE_ERGO
ergo::mh_proposal_result Propose_person_size::operator()
(
    const Scene& in,
    Scene& out
)
{
    // number of unchanged tracks
    size_t scene_sz = std::count_if(
                            in.association.begin(),
                            in.association.end(),
                            boost::bind(&Target::changed, _1));

    Categorical_distribution<size_t> U_t(1, scene_sz, 1);
    Categorical_distribution<size_t> U_d(1, 3, 1);
    std::string prop_name;
    out = in;

    // start at first changed track
    Ascn::const_iterator itg_p = in.association.begin();
    Ascn::iterator otg_p = out.association.begin();
    while(!itg_p->changed()) { itg_p++; otg_p++; }

    // advance to kth changed track
    size_t k = sample(U_t);
    for(size_t i = 1; i < k; i++)
    {
        do{ itg_p++; otg_p++; } while(!itg_p->changed());
    }

    // change chosen size dimension
    size_t d = sample(U_d);
    switch(d)
    {
        case 1:
        {
            double dh = sample(N_height);
            otg_p->trajectory().height = itg_p->trajectory().height + dh;
            prop_name = "height";
            break;
        }

        case 2:
        {
            double dw = sample(N_width);
            otg_p->trajectory().width = itg_p->trajectory().width + dw;
            prop_name = "width";
            break;
        }

        case 3:
        {
            double dg = sample(N_girth);
            otg_p->trajectory().girth = itg_p->trajectory().girth + dg;
            prop_name = "girth";
            break;
        }

        default:
        {
            KJB_THROW_2(
                Runtime_error,
                "Cannot propose body dimension; invalid component.");
        }
    }

    otg_p->update_boxes(out.camera);
    if(m_infer_head) 
    {
        otg_p->update_faces(out.camera);
        for(size_t t = otg_p->get_start_time(); t <= otg_p->get_end_time(); ++t)
        {
            const Deva_facemark* fm = 
                itg_p->face_trajectory()[t - 1]->value.facemark;
            otg_p->face_trajectory()[t - 1]->value.facemark = fm;
        }
    }
    update_visibilities(out, m_infer_head);

    // symmetric proposal, so fwd and rev probs don't matter
    return ergo::mh_proposal_result(0.0, 0.0, prop_name);
}
#endif

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

#ifdef KJB_HAVE_ERGO
ergo::mh_proposal_result Propose_point_location::operator()
(
    const Scene& in,
    Scene& out
)
{
    // number of objects
    size_t scene_sz = in.objects.size();
    if(scene_sz == 0) return ergo::mh_proposal_result(0.0, 0.0, "NA");

    Categorical_distribution<size_t> U_t(0, scene_sz - 1, 1);
    std::string obj_name;
    out = in;

    size_t k = sample(U_t);
    double dx = sample(N_x);
    double dy = sample(N_y);
    double dz = sample(N_z);

    out.objects[k].trajectory()[0]->value.position[0] += dx;
    out.objects[k].trajectory()[0]->value.position[1] += dy;
    out.objects[k].trajectory()[0]->value.position[2] += dz;

    // symmetric proposal, so fwd and rev probs don't matter
    return ergo::mh_proposal_result(0.0, 0.0, 
                            boost::lexical_cast<std::string>(k));
}
#endif

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

#ifdef KJB_HAVE_ERGO

Sample_scenes::Hmc_step Sample_scenes::make_hmc_traj_step
(
    const Scene_adapter& scene_adapter,
    Scene& best_scene
) const
{
    const size_t scene_size = scene_adapter.size(&best_scene);

    // build gradient step size
    std::vector<double> grad_step_sizes; 
    if(estimate_grad_step_size_)
    {
        grad_step_sizes = trajectory_gradient_step_sizes(
                                                    best_scene, 
                                                    infer_head_);
    }
    else
    {
        grad_step_sizes.resize(scene_size, pos_grad_step_size_);
    }

    // build step size vector and bounds vectors
    std::vector<double> step_sizes(scene_size, pos_opt_step_size_);

    // build HMC step
    Scene_gradient sgrad(*posterior_p_, grad_step_sizes, nthreads_);
    Hmc_step traj_step(
                scene_adapter,
                *posterior_p_,
                sgrad,
                step_sizes,
                num_leapfrog_steps_,
                0.9);
    traj_step.rename("trajectory");

    return traj_step;
}

#endif

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

#ifdef KJB_HAVE_ERGO

//Sample_scenes::Mh_step Sample_scenes::make_mh_traj_step() const
//{
//    // build MH step
//    Propose_trajectory<gp::Zero, gp::Sqex> propose(20);
//    Mh_step traj_step(*posterior_p_, propose);
//    traj_step.rename("trajectory");
//
//    return traj_step;
//}
//
#endif

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

#ifdef KJB_HAVE_ERGO

Sample_scenes::Mh_step Sample_scenes::make_mh_size_step(bool infer_head) const
{
    // build size mh step and add recorders
    Propose_person_size proposer(height_sdv_, width_sdv_, girth_sdv_, infer_head);
    Mh_step size_step(*posterior_p_, proposer);
    size_step.rename("size");

    return size_step;
}
#endif

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

#ifdef KJB_HAVE_ERGO

Sample_scenes::Mh_step Sample_scenes::make_mh_pos_step() const
{
    // build size mh step and add recorders
    Propose_point_location proposer(pos_sdv_);
    Mh_step size_step(*posterior_p_, proposer);
    size_step.rename("location");

    return size_step;
}

#endif

