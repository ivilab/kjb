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

#include <people_tracking_cpp/pt_scene.h>
#include <people_tracking_cpp/pt_association.h>
#include <people_tracking_cpp/pt_target.h>
#include <people_tracking_cpp/pt_data.h>
#include <people_tracking_cpp/pt_complete_trajectory.h>
#include <people_tracking_cpp/pt_body_2d_trajectory.h>
#include <people_tracking_cpp/pt_box_likelihood.h>
#include <people_tracking_cpp/pt_optical_flow_likelihood.h>
#include <people_tracking_cpp/pt_position_prior.h>
#include <people_tracking_cpp/pt_scene_posterior.h>
#include <people_tracking_cpp/pt_visibility.h>
#include <people_tracking_cpp/pt_util.h>
#include <camera_cpp/perspective_camera.h>
#include <flow_cpp/flow_integral_flow.h>
#include <l_cpp/l_exception.h>
#include <l/l_sys_mal.h>
#include <l/l_sys_time.h>
#include <detector_cpp/d_bbox.h>
#include "utils.h"
#include <vector>
#include <algorithm>
#include <string>
#include <iostream>
#include <utility>
#include <boost/foreach.hpp>
#include <boost/bind.hpp>

using namespace std;
using namespace kjb;
using namespace kjb::pt;
using namespace boost;

/** @brief  Helper function. */
pair<const Target*, size_t> interesting_target_frame(const Scene& scene);

/** @brief  Main -- all the magic happens here. */
int main(int argc, char** argv)
{
    try
    {
        // create (or read) scene
        size_t num_frames = 500;
        double img_width = 1200;
        double img_height = 800;
        Box_data data(1, 1);
        Facemark_data fm_data;
        vector<Integral_flow> flows_x;
        vector<Integral_flow> flows_y;
        Scene scene(Ascn(data), Perspective_camera(), 0.0, 0.0, 0.0);
        create_or_read_scene(argc, argv, num_frames, img_width, img_height,
                             data, fm_data, flows_x, flows_y, scene);

        // set to changed
        const Ascn& ascn = scene.association;
        for_each(ascn.begin(), ascn.end(), bind(&Target::set_changed_all, _1));
        update_facemarks(scene.association, fm_data);

        // MAKE LIKELIHOODS AND POSTERIOR
        Box_likelihood box_likelihood(0.98, img_width, img_height);

        Facemark_likelihood fm_likelihood(fm_data, face_sd, img_width, img_height);

        Optical_flow_likelihood of_likelihood(
            flows_x, flows_y, img_width, img_height,
            scale_x, scale_y, bg_scale_x, bg_scale_y);

        Face_flow_likelihood ff_likelihood(
            flows_x, flows_y, img_width, img_height,
            scale_x, scale_y, 2*scale_x, 2*scale_y);

        Color_likelihood color_likelihood; 
        Position_prior pos_prior(gp_scale, gp_svar);
        Direction_prior dir_prior(gp_scale_dir, gp_svar_dir);
        Face_direction_prior fdir_prior(gp_scale_fdir, gp_svar_fdir);

        Scene_posterior posterior(
                            box_likelihood,
                            fm_likelihood,
                            of_likelihood, 
                            ff_likelihood, 
                            color_likelihood,
                            pos_prior,
                            dir_prior,
                            fdir_prior);

        // find a target and frame that has a detection box and facemarks
        pair<const Target*, size_t> pr = interesting_target_frame(scene);
        const Target& target = *pr.first;
        size_t t = pr.second;
        Trajectory& traj = target.trajectory();
        Body_2d_trajectory& btraj = target.body_trajectory();
        Face_2d_trajectory& ftraj = target.face_trajectory();
        const Complete_state& cst = traj[t - 1]->value;
        const Perspective_camera& cam = scene.camera;
        double ht = traj.height;
        double wt = traj.width;
        double gt = traj.girth;

        // TEST DURATION OF MODULES
        // THE ORDER OF A GRADIENT COMPUTATION IS:
        //   * project body
        //   * project face
        //   * compute box likelihood
        //   * compute facemark likelihood
        //   * compute optical flow likelihood
        //   * compute face flow likelihood
        //   * compute position prior
        //   * compute direction prior
        //   * compute face direction prior

        const size_t num_reps = 1000;
        cout << "TIME TO COMPLETE " << num_reps
             << " REPETITIONS OF THE FOLLOWING:\n";

        // body projection
        long tot_time = 0;
        for(size_t i = 1; i <= num_reps; ++i)
        {
            kjb_c::init_real_time();
            btraj[t - 1]->value = project_cstate(cst, cam, ht, wt, gt);
            tot_time += kjb_c::get_real_time();
        }
        cout << "Body projection: " << tot_time/1000.0 << endl;

        // face projection
        tot_time = 0;
        for(size_t i = 1; i <= num_reps; ++i)
        {
            kjb_c::init_real_time();
            const Deva_facemark* fm_p = ftraj[t - 1]->value.facemark;
            ftraj[t - 1]->value = project_cstate_face(cst, cam, ht, wt, gt);
            ftraj[t - 1]->value.facemark = fm_p;
            tot_time += kjb_c::get_real_time();
        }
        cout << "Face projection: " << tot_time/1000.0 << endl;

        // parts of face projection
        tot_time = 0;
        Vector_vec hpts;
        for(size_t i = 1; i <= num_reps; ++i)
        {
            kjb_c::init_real_time();
            Vector_vec pts = head_points(cst, ht, wt, gt);
            tot_time += kjb_c::get_real_time();
            hpts = pts;
        }
        cout << "  head_points(): " << tot_time/1000.0 << endl;

        tot_time = 0;
        for(size_t i = 1; i <= num_reps; ++i)
        {
            kjb_c::init_real_time();
            transform(
                hpts.begin(),
                hpts.end(),
                hpts.begin(),
                bind(project_point, cref(cam), _1));
            tot_time += kjb_c::get_real_time();
        }
        cout << "  project head pts: " << tot_time/1000.0 << endl;

        tot_time = 0;
        for(size_t i = 1; i <= num_reps; ++i)
        {
            kjb_c::init_real_time();
            Bbox head_box = compute_bounding_box(hpts.begin(), hpts.end());
            tot_time += kjb_c::get_real_time();
        }
        cout << "  compute_bounding_box(): " << tot_time/1000.0 << endl;

        tot_time = 0;
        for(size_t i = 1; i <= num_reps; ++i)
        {
            kjb_c::init_real_time();
            Vector_vec ffts = face_features(cst, ht, wt, gt);
            tot_time += kjb_c::get_real_time();
        }
        cout << "  face_features(): " << tot_time/1000.0 << endl;

        // body model dir
        tot_time = 0;
        for(size_t i = 1; i <= num_reps; ++i)
        {
            const Complete_state& cst2 = traj[t]->value;
            kjb_c::init_real_time();
            btraj[t - 1]->value.model_dir = model_direction(
                                                cst, cst2, cam, ht, wt, gt);
            tot_time += kjb_c::get_real_time();
        }
        cout << "Model direction: " << tot_time/1000.0 << endl;

        // face model dir
        tot_time = 0;
        for(size_t i = 1; i <= num_reps; ++i)
        {
            const Complete_state& cst2 = traj[t]->value;
            kjb_c::init_real_time();
            ftraj[t - 1]->value.model_dir = face_model_direction(
                                                cst, cst2, cam, ht, wt, gt);
            tot_time += kjb_c::get_real_time();
        }
        cout << "Face model direction: " << tot_time/1000.0 << endl;

        // update visibilities
        tot_time = 0;
        for(size_t i = 1; i <= num_reps; ++i)
        {
            kjb_c::init_real_time();
            update_visibilities(scene, t, true);
            tot_time += kjb_c::get_real_time();
        }
        cout << "Update visibilites: " << tot_time/1000.0 << endl;

        // box likelihood
        tot_time = 0;
        for(size_t i = 1; i <= num_reps; ++i)
        {
            kjb_c::init_real_time();
            box_likelihood.at_frame(target, t);
            tot_time += kjb_c::get_real_time();
        }
        cout << "Box likelihood: " << tot_time/1000.0 << endl;

        // FM likelihood
        tot_time = 0;
        for(size_t i = 1; i <= num_reps; ++i)
        {
            kjb_c::init_real_time();
            fm_likelihood.at_face(ftraj[t - 1]->value);
            tot_time += kjb_c::get_real_time();
        }
        cout << "FM likelihood: " << tot_time/1000.0 << endl;

        // OF likelihood
        tot_time = 0;
        for(size_t i = 1; i <= num_reps; ++i)
        {
            kjb_c::init_real_time();
            of_likelihood.at_box(btraj[t - 1]->value, t);
            tot_time += kjb_c::get_real_time();
        }
        cout << "OF likelihood: " << tot_time/1000.0 << endl;

        // FF likelihood
        tot_time = 0;
        for(size_t i = 1; i <= num_reps; ++i)
        {
            kjb_c::init_real_time();
            ff_likelihood.at_face(ftraj[t - 1]->value, t);
            tot_time += kjb_c::get_real_time();
        }
        cout << "FF likelihood: " << tot_time/1000.0 << endl;

        // position prior
        tot_time = 0;
        for(size_t i = 1; i <= num_reps; ++i)
        {
            kjb_c::init_real_time();
            pos_prior.local(target, t);
            tot_time += kjb_c::get_real_time();
        }
        cout << "Position prior: " << tot_time/1000.0 << endl;

        // direction prior
        tot_time = 0;
        for(size_t i = 1; i <= num_reps; ++i)
        {
            kjb_c::init_real_time();
            dir_prior.local(target, t);
            tot_time += kjb_c::get_real_time();
        }
        cout << "Direction prior: " << tot_time/1000.0 << endl;

        // face direction prior
        tot_time = 0;
        for(size_t i = 1; i <= num_reps; ++i)
        {
            kjb_c::init_real_time();
            fdir_prior.local(target, t);
            tot_time += kjb_c::get_real_time();
        }
        cout << "Face direction prior: " << tot_time/1000.0 << endl;

        // space prior
        tot_time = 0;
        for(size_t i = 1; i <= num_reps; ++i)
        {
            kjb_c::init_real_time();
            pos_prior.local_space(scene, target, t);
            tot_time += kjb_c::get_real_time();
        }
        cout << "Space prior: " << tot_time/1000.0 << endl;

        // endpoint prior
        tot_time = 0;
        for(size_t i = 1; i <= num_reps; ++i)
        {
            kjb_c::init_real_time();
            pos_prior.local_endpoints(scene, target, t);
            tot_time += kjb_c::get_real_time();
        }
        cout << "Ednpoints prior: " << tot_time/1000.0 << endl;

        // TEST MORE GLOBAL STUFF
        cout << endl;
        cout << "TIME TO COMPLETE ON A SCENE OF SIZE " << dims(scene) << endl;

        // full posterior
        kjb_c::init_real_time();
        posterior(scene);
        tot_time = kjb_c::get_real_time();
        cout << "Posterior evaluation: " << tot_time/1000.0 << endl;

        // copy scene
        kjb_c::init_real_time();
        Scene scene2 = scene;
        tot_time = kjb_c::get_real_time();
        cout << "Copy scene: " << tot_time/1000.0 << endl;

        // update boxes
        kjb_c::init_real_time();
        for_each(
            ascn.begin(),
            ascn.end(),
            bind(&Target::update_boxes, _1, cref(cam)));
        tot_time = kjb_c::get_real_time();
        cout << "Update boxes: " << tot_time/1000.0 << endl;

        // update faces
        kjb_c::init_real_time();
        for_each(
            ascn.begin(),
            ascn.end(),
            bind(&Target::update_faces, _1, cref(cam)));
        tot_time = kjb_c::get_real_time();
        cout << "Update faces: " << tot_time/1000.0 << endl;

        // update visibilities
        kjb_c::init_real_time();
        update_visibilities(scene);
        tot_time = kjb_c::get_real_time();
        cout << "Update visibilities: " << tot_time/1000.0 << endl;
    }
    catch(const kjb::Exception& ex)
    {
        ex.print_details();
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

pair<const Target*, size_t> interesting_target_frame(const Scene& scene)
{
    BOOST_FOREACH(const Target& target, scene.association)
    {
        BOOST_FOREACH(const Target::value_type& pr, target)
        {
            size_t t = pr.first;
            if(t != target.get_end_time()
                && target.face_trajectory()[t - 1]->value.facemark != 0)
            {
                return make_pair(&target, t);
            }
        }
    }

    KJB_THROW_2(
        Runtime_error,
        "Could not find a target with a box and a facemark in the same frame.");
}

