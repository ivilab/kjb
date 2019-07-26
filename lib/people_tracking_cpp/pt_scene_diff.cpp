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
#include "people_tracking_cpp/pt_scene_diff.h"
#include "people_tracking_cpp/pt_scene_posterior.h"
#include "people_tracking_cpp/pt_scene_adapter.h"
#include "people_tracking_cpp/pt_scene.h"
#include "people_tracking_cpp/pt_target.h"
#include "people_tracking_cpp/pt_complete_trajectory.h"
#include "m_cpp/m_matrix.h"
#include "m_cpp/m_vector.h"
#include "diff_cpp/diff_hessian_ind.h"
#include "diff_cpp/diff_hessian_ind_mt.h"
#include "l_cpp/l_util.h"

#include <vector>
#include <algorithm>
#include <boost/foreach.hpp>

using namespace kjb;
using namespace kjb::pt;

void Scene_hessian::set_diagonals(const Scene& scene) const
{
    size_t i = 0;
    size_t dim = post_.adapter().infer_head() ? 5 : 2; 
    BOOST_FOREACH(const Target& tg, scene.association)
    {
        if(tg.changed())
        {
            // start + end times for use below -- cleaner this way
            size_t tgst = tg.get_start_time();
            size_t tget = tg.get_end_time();
            size_t chst = tg.changed_start();
            size_t chet = tg.changed_end();

            // sizes for use below -- cleaner this way
            //size_t tgsz = dim * (tget - tgst + 1);
            //size_t chsz = dim * (chet - chst + 1);
            size_t tgsz = dims(tg, false, post_.adapter().infer_head());
            size_t chsz = dims(tg, true, post_.adapter().infer_head());

            // old hessian and new hessian
            Vector& tghd = tg.hessian_diagonal();
            Vector chhd;
            if(nthreads_ == 1)
            {
                chhd = hessian_ind_diagonal(
                        post_, scene, dx_, post_.adapter(), i, i + chsz - 1);
            }
            else
            {
                chhd = hessian_ind_diagonal_mt(
                                    post_, scene, dx_,
                                    post_.adapter(),
                                    i, i + chsz - 1,
                                    nthreads_);
            }
            ASSERT(chhd.size() == chsz);
            //ASSERT(tghd.empty() || tghd.size() == tgsz);

            //////////////////// temp /////////////////////////////////////////
            //std::cout << "st: " << tgst << ", et: " << tget
            //          << ", cst: " << chst << ", cet: " << chet
            //          << ", chhd: " << chsz << ", tghd: " << tgsz << std::endl;
            //////////////////// temp /////////////////////////////////////////

            if(chst == tgst && chet == tget)
            {
                //// track is completely new (birth or secretion)
                // no old hessian
                //ASSERT(tghd.empty());

                // new hessian replaces old hessian
                tghd = chhd;
            }
            else if(chst > tgst && chet < tget)
            {
                //// middle of track is new (absorption)
                // old hessian was the size of the whole track
                ASSERT(tghd.size() == tgsz);

                // new hessian only replaces changed part (starting at chst)
                std::copy(
                    chhd.begin(),
                    chhd.end(),
                    tghd.begin() + dim*(chst - tgst));
            }
            else if(tg.changed_start() > tg.get_start_time())
            {
                //// end of the track is new (extension or absorption)
                ASSERT(chet == tget);

                // insert new hessian so that its first element replaces
                // the last element of the old hessian and goes from there
                tghd.resize(tgsz);
                std::copy(
                    chhd.begin(),
                    chhd.end(),
                    tghd.begin() + dim*(chst - tgst));
            }
            else
            {
                //// beginning of track is new (extension)
                ASSERT(chst == tgst);

                // insert new hessian so that its last element replaces
                // the first element of the old hessian and goes (back) from there
                Vector temp = tghd;
                tghd.resize(tgsz);
                std::copy(temp.rbegin(), temp.rend(), tghd.rbegin());
                std::copy(chhd.begin(), chhd.end(), tghd.begin());
            }

            i += chsz;
            post_.reset();

            ASSERT(tghd.size() == tgsz);
        }
    }
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

double kjb::pt::lm_marginal_likelihood(const Scene& scene, double pt, bool ih)
{
    double log_det_H = 0.0;
    BOOST_FOREACH(const Target& target, scene.association)
    {
        //ASSERT(!target.hessian_diagonal().empty());
        IFT(!target.hessian_diagonal().empty(), Runtime_error,
            "Cannot compute marginal likelihood; Hessian diagonals not set.");
        const double hssz = target.hessian_diagonal().size();
        for(size_t i = 0; i < hssz; ++i)
        {
            log_det_H += std::log(-target.hessian_diagonal()[i]);
        }
    }

    // get Metropolis-Laplace approximation
    double dl = 0.0;
    size_t D = dims(scene, false, ih);
    try
    {
        // manually compute, as we do not actually have a Hessian matrix
        double p = 0.5*log_det_H - (D/2.0)*std::log(2*M_PI);
        dl = pt - p;
    }
    catch(const KJB_error& kerr)
    {
        std::cerr << "WARNING: Hessian matrix not positive definite.\n";
        dl = pt - (D/-2.0)*std::log(2*M_PI);
    }

    return dl;
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

std::vector<double> kjb::pt::trajectory_gradient_step_sizes
(
    const Scene& scene,
    bool infer_head
)
{
    std::vector<double> step_sizes;

    const Matrix& C = scene.camera.build_camera_matrix();
    Pixel_move move_point(C);
    BOOST_FOREACH(const Target& target, scene.association)
    {
        if(!target.changed()) continue;

        const Trajectory& traj = target.trajectory();

        size_t start_time = target.changed_start();
        size_t end_time = target.changed_end();

        double dx, dz;
        for(size_t i = start_time - 1; i < end_time; i++)
        {
            ASSERT(traj[i]);
            
            Vector r(2, 0.0);
            r[0] = traj[i]->value.position[0];
            r[1] = traj[i]->value.position[2];
            move_point.set_point(r);

            // move right
            Vector s = move_point.x(1.0);
            ASSERT(s.size() == 2);
            dx = fabs(s[0] - r[0]);
            dz = fabs(s[1] - r[1]);

            // move left 
            s = move_point.x(-1.0);
            dx = std::max(fabs(s[0] - r[0]), dx);
            dz = std::max(fabs(s[1] - r[1]), dz);

            // move up
            s = move_point.y(1.0);
            dx = std::max(fabs(s[0] - r[0]), dx);
            dz = std::max(fabs(s[1] - r[1]), dz);

            // move down
            s = move_point.y(-1.0);
            dx = std::max(fabs(s[0] - r[0]), dx);
            dz = std::max(fabs(s[1] - r[1]), dz);

            step_sizes.push_back(dx);
            step_sizes.push_back(dz);

            // directions
            if(infer_head)
            {
                step_sizes.push_back(M_PI/12);
                step_sizes.push_back(M_PI/12);
                step_sizes.push_back(M_PI/12);
            }
        }
    }

    return step_sizes;
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

size_t kjb::pt::make_max
(
    const Scene& scene,
    const Scene_posterior& pt,
    const std::vector<double>& ss,
    bool infer_head
)
{
    bool vo = pt.vis_off();
    double cur_pt = pt(scene);
    bool at_max = false;
    size_t iters = 0;
    bool ih = infer_head;
    size_t dim = infer_head ? 5 : 2;
    while(!at_max)
    {
        at_max = true;

        size_t i = 0;
        BOOST_FOREACH(const Target& target, scene.association)
        {
            if(!target.changed()) continue;

            size_t sf = target.changed_start();
            size_t ef = target.changed_end();
            for(size_t t = sf; t <= ef; ++t)
            {
                double bad_pt = pt.local(target, scene, t);

                // trajectory
                for(size_t d = 0; d < dim; ++d, ++i)
                {
                    double dx = ss[i];

                    // move right
                    //adapter.set(&scene, i, x + dx);
                    move_variable_at_frame(scene, target, t, d, dx, vo, ih);

                    double good_pt = pt.local(target, scene, t);
                    double right_pt = cur_pt - bad_pt + good_pt;

                    // move left
                    move_variable_at_frame(scene, target, t, d, -2*dx, vo, ih);

                    good_pt = pt.local(target, scene, t);
                    double left_pt = cur_pt - bad_pt + good_pt;

                    // move back to original
                    move_variable_at_frame(scene, target, t, d, dx, vo, ih);

                    // got worse
                    if(cur_pt >= left_pt && cur_pt >= right_pt) continue;

                    // got better
                    at_max = false;
                    if(left_pt > right_pt)
                    {
                        move_variable_at_frame(scene, target, t, d, -dx, vo, ih);
                        cur_pt = left_pt;
                    }
                    else
                    {
                        move_variable_at_frame(scene, target, t, d, dx, vo, ih);
                        cur_pt = right_pt;
                    }
                }
            }
        }
        iters++;
    }

    //std::cout << " make max iters: " << iters << std::endl;
    return iters;
}

