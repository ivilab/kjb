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
   |  Author:  Ernesto Brau
 * =========================================================================== */

/* $Id$ */

#include "l/l_sys_debug.h"  /* For ASSERT */
#include "people_tracking_cpp/pt_scene_init.h"
#include "people_tracking_cpp/pt_scene.h"
#include "people_tracking_cpp/pt_target.h"
#include "people_tracking_cpp/pt_complete_trajectory.h"
#include "people_tracking_cpp/pt_face_2d_trajectory.h"
#include "people_tracking_cpp/pt_scene_posterior.h"
#include "people_tracking_cpp/pt_data.h"
#include "people_tracking_cpp/pt_sample_scenes.h"
#include "people_tracking_cpp/pt_sample_scenes.h"
#include "people_tracking_cpp/pt_util.h"
#include "camera_cpp/perspective_camera.h"
#include "prob_cpp/prob_distribution.h"
#include "prob_cpp/prob_sample.h"
#include "detector_cpp/d_deva_facemark.h"

#include <boost/foreach.hpp>

using namespace kjb;
using namespace kjb::pt;

void kjb::pt::update_scene_state
(
    const Scene& scene,
    const Facemark_data& fmdata,
    bool infer_head
)
{
    // first estimation
    BOOST_FOREACH(const Target& tg, scene.association)
    {
        if(tg.changed_start() == tg.get_start_time()
                && tg.changed_end() == tg.get_end_time())
        {
            tg.estimate_height(scene.camera);
        }

        tg.estimate_trajectory(scene.camera);
        if(tg.changed())
        {
            initialize_directions(
                        tg.trajectory(), 
                        tg.changed_start(), 
                        tg.changed_end(), 
                        false);
        }
        tg.update_boxes(scene.camera);
        if(infer_head) tg.update_faces(scene.camera);
    }

    if(infer_head)
    {
        // Refine the trajectory use face information
        // update facemarks
        clear_facemarks(scene.association);
        update_facemarks(scene.association, fmdata);
    }
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

void kjb::pt::refine_scene_state
(
    const Scene& scene,
    const Scene_posterior& post
)
{
    const Box_data& data = static_cast<const Box_data&>(
                                scene.association.get_data());

    // set unchanged and refine using faces (if specified)
    bool something_changed = false;
    BOOST_FOREACH(const Target& tg, scene.association)
    {
        if(tg.changed())
        {
            refine_target_state(tg, scene.camera, data, post);
            tg.estimate_trajectory(scene.camera, 1e-5, 1e-5);
            tg.update_walking_angles();
            tg.estimate_directions(post.infer_head(), 0.5);
            tg.update_boxes(scene.camera);
            tg.update_faces(scene.camera);
            something_changed = true;
        }
    }

    // update facemarks one last time
    if(something_changed)
    {
        clear_facemarks(scene.association);
        update_facemarks(scene.association, post.fm_likelihood().data());
        update_visibilities(scene);
    }
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

//void kjb::pt::refine_target_state
//(
//    const Target& target,
//    const Perspective_camera& cam,
//    const Box_data& box_data,
//    const Scene_posterior& posterior
//)
//{
//    // although this should not be called if there is no changed,
//    // we should probably still check for it
//    if(!target.changed()) return;
//    size_t sf = target.changed_start();
//    size_t ef = target.changed_end();
//
//    // we don't want to use the prior
//    bool upp = posterior.use_pos_prior();
//    bool udp = posterior.use_dir_prior();
//    bool ufp = posterior.use_fdir_prior();
//    bool usp = posterior.use_dim_prior();
//    posterior.use_pos_prior() = false;
//    posterior.use_dir_prior() = false;
//    posterior.use_fdir_prior() = false;
//    posterior.use_dim_prior() = false;
//
//    // set up new scene
//    Scene scene(Ascn(box_data), cam, 1.0, 1.0, 1.0);
//    scene.association.insert(target);
//    const Target& tg = *scene.association.begin();
//
//    // set up sampler
//    const size_t num_samples = 100;
//    Sample_scenes sample_scenes(posterior, true);
//    sample_scenes.set_num_iterations(num_samples);
//    sample_scenes.set_num_leapfrog_steps(2);
//    sample_scenes.set_hmc_step_size(0.0025);
//
//    // inidividually optimize each frame (ignoring smoothness)
//    Trajectory& traj = tg.trajectory();
//    Face_2d_trajectory& ftraj = tg.face_trajectory();
//    for(size_t t = ef; t >= ef; --t)
//    {
//        // estimate_trajectory() needs to be called before this
//        // as well as update_faces() and update_facemarks()
//        assert(ftraj[t - 1]);
//        assert(traj[t - 1]);
//
//        // set body dir and face dir
//        const Deva_facemark* fmark_p = ftraj[t - 1]->value.facemark;
//        if(fmark_p != NULL)
//        {
//            traj[t - 1]->value.body_dir = fmark_p->yaw() * (M_PI/180);
//        }
//        else
//        {
//            // set the face direction probabilistically
//            const double p_face_backward = 0.9;
//            double p = sample(Uniform_distribution());
//            if(p < p_face_backward) traj[t - 1]->value.body_dir = -M_PI; 
//        }
//        traj[t - 1]->value.face_dir = Vector2(0.0, 0.0);
//
//        // optimize
//        if(t == ef && sf != ef) continue;
//
//        tg.set_changed_start(t);
//        if(t == ef - 1)
//        {
//            tg.set_changed_end(ef);
//        }
//        else
//        {
//            tg.set_changed_end(t);
//        }
//
//        sample_scenes(scene);
//    }
//
//    using std::swap;
//    swap(target.trajectory(), tg.trajectory());
//    swap(target.body_trajectory(), tg.body_trajectory());
//    swap(target.face_trajectory(), tg.face_trajectory());
//
//    posterior.use_pos_prior() = upp;
//    posterior.use_dir_prior() = udp;
//    posterior.use_fdir_prior() = ufp;
//    posterior.use_dim_prior() = usp;
//}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

void kjb::pt::refine_target_state
(
    const Target& target,
    const Perspective_camera& cam,
    const Box_data& box_data,
    const Scene_posterior& posterior
)
{
    // although this should not be called if there is no changed,
    // we should probably still check for it
    if(!target.changed()) return;
    size_t sf = target.changed_start();
    size_t ef = target.changed_end();

    // we don't want to use the prior
    bool upp = posterior.use_pos_prior();
    bool udp = posterior.use_dir_prior();
    bool ufp = posterior.use_fdir_prior();
    bool usp = posterior.use_dim_prior();
    bool uofl = posterior.use_of_lh();
    bool uffl = posterior.use_ff_lh();
    posterior.use_pos_prior() = false;
    posterior.use_dir_prior() = false;
    posterior.use_fdir_prior() = false;
    posterior.use_dim_prior() = false;
    posterior.use_of_lh() = false;
    posterior.use_ff_lh() = false;

    // set up new scene
    Scene scene(Ascn(box_data), cam, 1.0, 1.0, 1.0);
    scene.association.insert(target);
    const Target& tg = *scene.association.begin();

    // set up sampler
    const size_t num_samples = 20;
    Sample_scenes sample_scenes(posterior, true);
    sample_scenes.set_num_iterations(num_samples);
    sample_scenes.set_num_leapfrog_steps(2);
    sample_scenes.set_hmc_step_size(0.0025);

    // inidividually optimize each frame (ignoring smoothness)
    Trajectory& traj = tg.trajectory();
    Face_2d_trajectory& ftraj = tg.face_trajectory();
    for(size_t t = sf; t <= ef; ++t)
    {
        // estimate_trajectory() needs to be called before this
        // as well as update_faces() and update_facemarks()
        assert(traj[t - 1]);
        assert(ftraj[t - 1]);

        //if(traj[t - 1]->value.body_dir != std::numeric_limits<double>::max())
        if(traj[t - 1]->value.body_dir != 0.0)
        {
            // if the body_dir is not 0.0, it means we have fitted already 
            // just skip it
            continue;
        }

        // set body dir and face dir
        const Deva_facemark* fmark_p = ftraj[t - 1]->value.facemark;
        if(fmark_p != NULL)
        {
            traj[t - 1]->value.body_dir = fmark_p->yaw() * (M_PI/180);

            tg.set_changed_start(t);
            tg.set_changed_end(t);
            sample_scenes(scene);

            traj[t - 1]->value.body_dir += traj[t - 1]->value.face_dir[0];
            traj[t - 1]->value.face_dir[0] = 0.0;
        }
        else
        {
            // no boxes or facemarks: sampling would be futile
            // one or the other of these should be commented out
            // if set to boost::none, we need to call estimate_trajectory again
            // traj[t - 1] = boost::none;
            traj[t - 1]->value.body_dir = std::numeric_limits<double>::max();
            traj[t - 1]->value.face_dir[0] = std::numeric_limits<double>::max();
            traj[t - 1]->value.face_dir[1] = std::numeric_limits<double>::max();
        }
    }

    using std::swap;
    swap(target.trajectory(), tg.trajectory());
    swap(target.body_trajectory(), tg.body_trajectory());
    swap(target.face_trajectory(), tg.face_trajectory());

    posterior.use_pos_prior() = upp;
    posterior.use_dir_prior() = udp;
    posterior.use_fdir_prior() = ufp;
    posterior.use_dim_prior() = usp;
    posterior.use_of_lh() = uofl;
    posterior.use_ff_lh() = uffl;
}

