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

/* $Id: local_likelihood.cpp 19228 2015-05-30 04:53:36Z jguan1 $ */

#include <people_tracking_cpp/pt_scene.h>
#include <people_tracking_cpp/pt_target.h>
#include <people_tracking_cpp/pt_box_likelihood.h>
#include <people_tracking_cpp/pt_facemark_likelihood.h>
#include <people_tracking_cpp/pt_optical_flow_likelihood.h>
#include <people_tracking_cpp/pt_face_flow_likelihood.h>
#include <people_tracking_cpp/pt_position_prior.h>
#include <people_tracking_cpp/pt_direction_prior.h>
#include <people_tracking_cpp/pt_data.h>
#include <people_tracking_cpp/pt_scene_posterior.h>
#include <people_tracking_cpp/pt_util.h>
#include <flow_cpp/flow_integral_flow.h>
#include <m_cpp/m_vector_d.h>
#include <l_cpp/l_test.h>
#include <l_cpp/l_exception.h>
#include <vector>
#include "utils.h"
#include <boost/foreach.hpp>

const bool VERBOSE = true;

using namespace std;
using namespace kjb;
using namespace kjb::pt;

/** @brief  Main -- all the magic happens here. */
int main(int argc, char** argv)
{
//#ifdef TEST
//    kjb_c::kjb_init();
//    kjb_c::kjb_l_set("heap-checking", "off");
//    kjb_c::kjb_l_set("initialization-checking", "off");
//#endif

    const double eps = 1e-6;
    const double eps_app_i = 0.4;
    const double eps_app_s = 0.5;

    try
    {
        // READ DATA AND CREATE SCENE
        size_t num_frames = 6;
        double img_width = 500;
        double img_height = 500;

        Box_data data(img_width, img_height, 0.99);
        vector<Integral_flow> flows_x;
        vector<Integral_flow> flows_y;
        Facemark_data fm_data(num_frames);

        // create scene
        Scene scene(Ascn(data), Perspective_camera(), 0.0, 0.0, 0.0);
        create_or_read_scene(argc, argv, num_frames, img_width, img_height,
                             data, fm_data, flows_x, flows_y, scene, 5);

        // create posterior
        Box_likelihood box_likelihood(1.0, img_width, img_height);

        Facemark_likelihood fm_likelihood(fm_data,face_sd, img_width, img_height);
        update_facemarks(scene.association, fm_data);

        Optical_flow_likelihood of_likelihood(
            flows_x, flows_y, img_width, img_height,
            scale_x, scale_y, bg_scale_x, bg_scale_y);

        Face_flow_likelihood ff_likelihood(
            flows_x, flows_y, img_width, img_height,
            scale_x, scale_y, bg_scale_x, bg_scale_y);

        Color_likelihood color_likelihood; 

        Position_prior pos_prior(gp_scale, gp_svar, num_frames);
        Position_prior pos_prior_app(gp_scale, gp_svar);

        Direction_prior dir_prior(gp_scale_dir, gp_svar_dir, num_frames);
        Direction_prior dir_prior_app(gp_scale_dir, gp_svar_dir);

        Face_direction_prior fdir_prior(gp_scale_fdir, gp_svar_fdir, num_frames);
        Face_direction_prior fdir_prior_app(gp_scale_fdir, gp_svar_fdir);

        Scene_posterior posterior(
                            box_likelihood,
                            fm_likelihood,
                            of_likelihood, 
                            ff_likelihood, 
                            color_likelihood,
                            pos_prior,
                            dir_prior,
                            fdir_prior);

        posterior.use_color_lh() = false;
        posterior.use_dim_prior() = false;

        //posterior.use_box_lh() = false;
        //posterior.use_fm_lh() = false;
        //posterior.use_of_lh() = false;
        //posterior.use_ff_lh() = false;
        //posterior.use_pos_prior() = false;
        //posterior.use_dir_prior() = false;
        //posterior.use_fdir_prior() = false;

        bool pvo = posterior.vis_off();

        double orig_pt = posterior(scene);
        double orig_pr = pos_prior(scene)
                            + dir_prior(scene)
                            + fdir_prior(scene);
        Vector3 dv(0.01, 0.0, 0.01);
        double db = 0.0001;
        Vector2 df(0.001, 0.001);

        // TEST SINGLE CHANGE
        BOOST_FOREACH(const Target& target, scene.association)
        {
            int sf = target.get_start_time();
            int ef = target.get_end_time();
            assert(sf != -1 && ef != -1);
            for(size_t t = (size_t)sf; t <= (size_t)ef; t++)
            {
                // subtract current locals
                double local_pt = orig_pt;
                double local_pr = orig_pr;
                double local_pr_app = orig_pr;

                local_pt -= posterior.local(target, scene, t);

                local_pr -= pos_prior.local(target, t);
                local_pr -= dir_prior.local(target, t);
                local_pr -= fdir_prior.local(target, t);

                local_pr_app -= pos_prior_app.local(target, t);
                local_pr_app -= dir_prior_app.local(target, t);
                local_pr_app -= fdir_prior_app.local(target, t);

                // change scene
                move_trajectory_at_frame(scene, target, t, dv, pvo);
                move_trajectory_dir_at_frame(scene, target, t, db, pvo);
                move_trajectory_face_dir_at_frame(scene, target, t, df, pvo);

                // add new locals
                local_pt += posterior.local(target, scene, t);

                local_pr += pos_prior.local(target, t);
                local_pr += dir_prior.local(target, t);
                local_pr += fdir_prior.local(target, t);

                local_pr_app += pos_prior_app.local(target, t);
                local_pr_app += dir_prior_app.local(target, t);
                local_pr_app += fdir_prior_app.local(target, t);

                // compute new globals
                double all_pt = posterior(scene);
                double all_pr = pos_prior(scene)
                                + dir_prior(scene)
                                + fdir_prior(scene);
                double all_pr_app = all_pr;

                // change scene back
                move_trajectory_at_frame(scene, target, t, -dv, pvo);
                move_trajectory_dir_at_frame(scene, target, t, -db, pvo);
                move_trajectory_face_dir_at_frame(scene, target, t, -df, pvo);

                if(VERBOSE)
                {
                    cout << "FRAME: " << t << endl;
                    cout << local_pt << " - " << all_pt
                         << " = " << local_pt - all_pt
                         << "   ... IN ["
                         << log(1 - eps) << ", " << log(1 + eps)
                         << "]" << endl;
                    cout << local_pr << " - " << all_pr
                         << " = " << local_pr - all_pr
                         << "   ... IN ["
                         << log(1 - eps) << ", " << log(1 + eps)
                         << "]" << endl;
                    cout << local_pr_app << " - " << all_pr_app
                         << " = " << local_pr_app - all_pr_app
                         << "   ... IN ["
                         << log(1 - eps_app_i) << ", " << log(1 + eps_app_s)
                         << "]" << endl << endl;
                }

                TEST_TRUE(local_pt - all_pt >= log(1 - eps)
                            && local_pt - all_pt <= log(1 + eps));

                TEST_TRUE(local_pr - all_pr >= log(1 - eps)
                            && local_pr - all_pr <= log(1 + eps));

                TEST_TRUE(local_pr_app - all_pr_app >= log(1 - eps_app_i)
                            && local_pr_app - all_pr_app <= log(1 + eps_app_s));
            }
        }

        // TEST DOUBLE CHANGE
        BOOST_FOREACH(const Target& target1, scene.association)
        {
            int sf1 = target1.get_start_time();
            int ef1 = target1.get_end_time();
            assert(sf1 != -1 && ef1 != -1);
            for(size_t t1 = (size_t)sf1; t1 <= (size_t)ef1; t1++)
            {
                BOOST_FOREACH(const Target& target2, scene.association)
                {
                    int sf2 = target2.get_start_time();
                    int ef2 = target2.get_end_time();
                    assert(sf2 != -1 && ef2 != -1);
                    for(size_t t2 = (size_t)sf2; t2 <= (size_t)ef2; t2++)
                    {
                        double local_pt = orig_pt;
                        local_pt -= posterior.local(
                                        target1, target2, scene, t1, t2);

                        move_trajectories_at_frames(
                            scene, target1, target2, t1, t2, dv, dv, pvo);
                        move_trajectory_dirs_at_frames(
                            scene, target1, target2, t1, t2, db, db, pvo);
                        move_trajectory_face_dirs_at_frames(
                            scene, target1, target2, t1, t2, df, df, pvo);

                        local_pt += posterior.local(
                                        target1, target2, scene, t1, t2);

                        double all_pt = posterior(scene);

                        move_trajectories_at_frames(
                            scene, target1, target2, t1, t2, -dv, -dv, pvo);
                        move_trajectory_dirs_at_frames(
                            scene, target1, target2, t1, t2, -db, -db, pvo);
                        move_trajectory_face_dirs_at_frames(
                            scene, target1, target2, t1, t2, -df, -df, pvo);

                        if(VERBOSE)
                        {
                            cout << "FRAME: " << t1 << ", " << t2 << endl;
                            cout << local_pt << " - " << all_pt
                                 << " = " << local_pt - all_pt
                                 << "   ... IN ["
                                 << log(1 - eps) << ", " << log(1 + eps)
                                 << "]" << endl << endl;
                        }

                        TEST_TRUE(local_pt - all_pt >= log(1 - eps)
                                    && local_pt - all_pt <= log(1 + eps));
                    }
                }
            }
        }
    }
    catch(const kjb::Exception& ex)
    {
        ex.print_details();
        cerr << endl;
        return EXIT_FAILURE;
    }

    RETURN_VICTORIOUSLY();
}

