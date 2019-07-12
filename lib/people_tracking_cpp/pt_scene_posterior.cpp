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

/* $Id: pt_scene_posterior.cpp 21596 2017-07-30 23:33:36Z kobus $ */

#include "l/l_sys_debug.h"  /* For ASSERT */
#include "people_tracking_cpp/pt_scene_posterior.h"
#include "people_tracking_cpp/pt_scene.h"
#include "people_tracking_cpp/pt_target.h"
#include "people_tracking_cpp/pt_complete_trajectory.h"
#include "people_tracking_cpp/pt_complete_state.h"
#include "people_tracking_cpp/pt_body_2d_trajectory.h"
#include "people_tracking_cpp/pt_face_2d_trajectory.h"
#include "people_tracking_cpp/pt_body_2d.h"
#include "people_tracking_cpp/pt_face_2d.h"
#include "people_tracking_cpp/pt_association.h"
#include "prob_cpp/prob_distribution.h"
#include "prob_cpp/prob_pdf.h"
#include "detector_cpp/d_bbox.h"
#include "l_cpp/l_util.h"
#include "l/l_sys_lib.h"

#include <vector>
#include <numeric>
#include <algorithm>
#include <functional>
#include <utility>
#include <iostream>
#include <iomanip>
#include <boost/bind.hpp>
#include <boost/ref.hpp>
#include <boost/foreach.hpp>

using namespace kjb;
using namespace kjb::pt;

double Scene_posterior::operator()(const Scene& scene) const
{
    double bl = use_box_lh_ ? box_likelihood_(scene) : 0.0;
    double ml = use_fm_lh_ && m_infer_head ? fm_likelihood_(scene) : 0.0;
    double ol = use_of_lh_ ? of_likelihood_(scene) : 0.0;
    double fl = use_ff_lh_ && m_infer_head ? ff_likelihood_(scene) : 0.0;
    double cl = use_color_lh_ ? color_likelihood_(scene) : 0.0;
    double pp = use_pos_prior_ ? pos_prior_(scene) : 0.0;
    double rp = use_dir_prior_ && m_infer_head ? dir_prior_(scene) : 0.0;
    double fp = use_fdir_prior_ && m_infer_head  ? fdir_prior_(scene) : 0.0;
    double dp = use_dim_prior_ ? dimension_prior(scene) : 0.0;

    return bl + ml + ol + fl + cl + pp + rp + fp + dp;
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

double Scene_posterior::local
(
    const Target& target1,
    const Target& target2,
    const Scene& scene,
    size_t frame1,
    size_t frame2
) const
{
    KJB(ASSERT(target1.trajectory()[frame1 - 1]));
    KJB(ASSERT(target2.trajectory()[frame2 - 1]));

    // set up
    size_t sf1 = target1.get_start_time();
    size_t sf2 = target2.get_start_time();
    size_t ef1 = target1.get_end_time();
    size_t ef2 = target2.get_end_time();

    IFT(frame1 >= sf1 && frame1 <= ef1 && frame2 >= sf2 && frame2 <= ef2,
        Illegal_argument, "Local posterior: specified frames out of range.");

    // compute affected frames
    std::set<size_t> frames;

    size_t rsf1 = std::max((int)sf1, (int)frame1 - 1);
    for(size_t t = rsf1; t <= frame1; t++)
    {
        frames.insert(t);
    }

    size_t rsf2 = std::max((int)sf2, (int)frame2 - 1);
    for(size_t t = rsf2; t <= frame2; t++)
    {
        frames.insert(t);
    }

    // LIKELIHOODS
    double bl = 0.0;
    double ml = 0.0;
    double ol = 0.0;
    double fl = 0.0;
    double cl = 0.0;
    if(use_box_lh_ || use_fm_lh_ || use_of_lh_ || use_color_lh_ || use_ff_lh_)
    {
        BOOST_FOREACH(size_t t, frames)
        {
            BOOST_FOREACH(const Target& tg, scene.association)
            {
                // get trajectory and properties
                const Trajectory& tj = tg.trajectory();
                const Body_2d_trajectory& btj = tg.body_trajectory();
                const Face_2d_trajectory& ftj = tg.face_trajectory();

                // only continue if this exists
                if(!tj[t - 1]) continue;

                // box likelihood
                if(use_box_lh_ && (t == frame1 || t == frame2))
                {
                    bl += box_likelihood_.at_frame(tg, t);
                }

                // facemark likelihood
                if(m_infer_head && use_fm_lh_ && (t == frame1 || t == frame2))
                {
                    const Face_2d& f2d = ftj[t - 1]->value;
                    ml += fm_likelihood_.at_face(f2d);
                }

                // of likelihood
                if(use_of_lh_)
                {
                    if((int)t != tg.get_end_time())
                    {
                        const Body_2d& b2d = btj[t - 1]->value;
                        ol += of_likelihood_.at_box(b2d, t);
                    }
                }

                // ff likelihood
                if(m_infer_head && use_ff_lh_)
                {
                    if((int)t != tg.get_end_time())
                    {
                        const Face_2d& f2d = ftj[t - 1]->value;
                        fl += ff_likelihood_.at_face(f2d, t);
                    }
                }

                // color likelihood
                if(use_color_lh_ && (t == frame1 || t == frame2))
                {
                    if((int)t != tg.get_end_time())
                    {
                        const Body_2d& cb2d = btj[t - 1]->value;
                        const Body_2d& nb2d = btj[t]->value;
                        cl += color_likelihood_.at_box(cb2d, nb2d, t, t + 1);
                    }
                }
            }
        }
    }

    // SPACE PRIOR
    double sp = 0.0;
    if(use_pos_prior_)
    {
        sp = pos_prior_.local_space(scene, target1, frame1);
        if(frame1 != frame2)
        {
            sp += pos_prior_.local_space(scene, target2, frame2);
        }
    }

    // ENDPOINT PRIOR
    double ep = 0.0;
    if(use_pos_prior_)
    {
        ep = pos_prior_.local_endpoints(scene, target1, frame1);
        if(frame1 != frame2)
        {
            ep += pos_prior_.local_endpoints(scene, target2, frame2);
        }
    }

    // POSITION PRIOR
    double pp = 0.0;
    if(use_pos_prior_)
    {
        if(&target1 == &target2
            && std::abs((int)frame1 - (int)frame2) <= pos_prior_.local_size())
        {
            
            pp += pos_prior_.local(target1, (frame1 + frame2)/2);
        }
        else
        {
            pp += pos_prior_.local(target1, frame1)
                    + pos_prior_.local(target2, frame2);
        }
    }

    // BODY DIRECTION PRIOR
    double dp = 0.0;
    if(m_infer_head && use_dir_prior_)
    {
        if(&target1 == &target2
            && std::abs((int)frame1 - (int)frame2) <= dir_prior_.local_size())
        {
            
            dp += dir_prior_.local(target1, (frame1 + frame2)/2);
        }
        else
        {
            dp += dir_prior_.local(target1, frame1)
                    + dir_prior_.local(target2, frame2);
        }
    }

    // FACE DIRECTION PRIOR
    double fp = 0.0;
    if(m_infer_head && use_fdir_prior_)
    {
        if(&target1 == &target2
            && std::abs((int)frame1 - (int)frame2) <= fdir_prior_.local_size())
        {
            
            fp += fdir_prior_.local(target1, (frame1 + frame2)/2);
        }
        else
        {
            fp += fdir_prior_.local(target1, frame1)
                    + fdir_prior_.local(target2, frame2);
        }
    }

    return bl + ml + ol + fl + cl + sp + pp + dp + fp;
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

double Scene_posterior::dimension_prior(const Scene& scene) const
{
    double dp = 0.0;
    BOOST_FOREACH(const Target& target, scene.association)
    {
        dp += log_pdf(N_h, target.trajectory().height);

        // we want reasonable width and girth
        if(target.trajectory().width < 0.3 || 
           target.trajectory().girth < 0.2 ||
           target.trajectory().width > 0.7 || 
           target.trajectory().girth > 0.6)
            return -1e10;

        dp += log_pdf(N_w, target.trajectory().width);
        dp += log_pdf(N_g, target.trajectory().girth);
    }

    return dp;
}

