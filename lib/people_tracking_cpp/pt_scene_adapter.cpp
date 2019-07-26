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
|     Ernesto Brau
|
* =========================================================================== */

/* $Id$ */

#include "people_tracking_cpp/pt_scene_adapter.h"
#include "people_tracking_cpp/pt_scene.h"
#include "people_tracking_cpp/pt_target.h"
#include "people_tracking_cpp/pt_visibility.h"
#include "l_cpp/l_util.h"
#include "l/l_sys_lib.h"

#include <iostream>
#include <boost/foreach.hpp>

using namespace kjb;
using namespace kjb::pt;

void Scene_adapter::set(Scene* s, const Vector& x) const
{
    Scene& scene = *s;

    size_t i = 0;
    BOOST_FOREACH(const Target& tg, scene.association)
    {
        if(!tg.changed()) continue;

        size_t sf = tg.changed_start();
        size_t ef = tg.changed_end();
        for(size_t t = sf; t <= ef; t++)
        {
            tg.trajectory()[t - 1]->value.position[0] = x[i++];
            tg.trajectory()[t - 1]->value.position[2] = x[i++];
            if(m_infer_head)
            {
                tg.trajectory()[t - 1]->value.body_dir = x[i++];
                tg.trajectory()[t - 1]->value.face_dir[0] = x[i++];
                tg.trajectory()[t - 1]->value.face_dir[1] = x[i++];
            }
        }

        tg.update_boxes(scene.camera);
        if(m_infer_head)
        {
            tg.update_faces(scene.camera);
        }
    }

    update_visibilities(scene, m_infer_head);
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

void Scene_adapter::compute_vars(const Scene* s) const
{
    if(s == 0 || s == scene_p_) return;

    scene_p_ = s;
    tgt_ps_.clear();
    frames_.clear();
    dims_.clear();

    size_t dim_per_frame = 5;
    if(!m_infer_head) dim_per_frame = 2;
    BOOST_FOREACH(const Target& tg, scene_p_->association)
    {
        if(!tg.changed()) continue;

        for(size_t t = tg.changed_start(); t <= tg.changed_end(); ++t)
        {
            for(size_t d = 0; d < dim_per_frame; ++d)
            {
                tgt_ps_.push_back(&tg);
                frames_.push_back(t);
                dims_.push_back(d);
            }
        }
    }

    //IFT(!tgt_ps_.empty(), Illegal_argument,
    //    "Cannot adapt scene; no changed targets");
} 

