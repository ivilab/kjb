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

#include <people_tracking_cpp/pt_scene.h>
#include <people_tracking_cpp/pt_data.h>
#include <people_tracking_cpp/pt_scene_posterior.h>
#include <people_tracking_cpp/pt_scene_adapter.h>
#include <people_tracking_cpp/pt_scene_diff.h>
#include <people_tracking_cpp/pt_position_prior.h>
#include <people_tracking_cpp/pt_box_likelihood.h>
#include <people_tracking_cpp/pt_facemark_likelihood.h>
#include <people_tracking_cpp/pt_optical_flow_likelihood.h>
#include <people_tracking_cpp/pt_util.h>
#include <st_cpp/st_perspective_camera.h>
#include <diff_cpp/diff_gradient.h>
#include <diff_cpp/diff_gradient_mt.h>
#include <flow_cpp/flow_integral_flow.h>
#include <m_cpp/m_vector.h>
#include <m_cpp/m_matrix.h>
#include <l_cpp/l_test.h>
#include <l/l_sys_time.h>
#include "utils.h"
#include <vector>
#include <algorithm>
#include <functional>
#include <boost/bind.hpp>

const bool VERBOSE = true;

using namespace std;
using namespace kjb;
using namespace kjb::pt;

/** @brief  Main -- all the magic happens here. */
int main(int argc, char** argv)
{
    const double stsz = 0.01;
    const double eps = 1e-10;
    const double eps_lax = 1e-4;
    const size_t num_threads = 6;

#ifdef TEST
    kjb_c::kjb_init();
    kjb_c::kjb_l_set("heap-checking", "off");
    kjb_c::kjb_l_set("initialization-checking", "off");
#endif

    try
    {
        // create (or read) scene
        size_t num_frames = 400;
        double img_width = 1200;
        double img_height = 800;
        Box_data data(1, 1);
        Facemark_data fm_data;
        vector<Integral_flow> flows_x;
        vector<Integral_flow> flows_y;
        Scene scene(Ascn(data), Perspective_camera(), 0.0, 0.0, 0.0);
        create_or_read_scene(argc, argv, num_frames, img_width, img_height,
                             data, fm_data, flows_x, flows_y, scene);

        const Ascn& w = scene.association;
        for_each(w.begin(), w.end(), bind(&Target::set_changed_all, _1));
        update_facemarks(scene.association, fm_data);

        // create posterior
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

        bool vis_off = false;
        bool head_off = false;
        Scene_posterior posterior(
                        box_likelihood,
                        fm_likelihood,
                        of_likelihood, 
                        ff_likelihood, 
                        color_likelihood,
                        pos_prior,
                        dir_prior,
                        fdir_prior, 
                        vis_off,
                        head_off);

        //posterior.use_box_lh() = false;
        //posterior.use_fm_lh() = false;
        //posterior.use_of_lh() = false;
        //posterior.use_ff_lh() = false;
        //posterior.use_color_lh() = false;
        //posterior.use_pos_prior() = false;
        //posterior.use_dir_prior() = false;
        //posterior.use_fdir_prior() = false;
        //posterior.use_dim_prior() = false;

        // TEST STUFF
        Scene_adapter adapter(vis_off, !head_off);
        vector<double> step_sizes(adapter.size(&scene), stsz);
        Scene_gradient grad(posterior, step_sizes, num_threads);
        Scene_posterior_ind ind_post(posterior);

        // generic gradient
        kjb_c::init_real_time();
        //Vector g = gradient_cfd(posterior, scene, step_sizes, adapter);
        long t = kjb_c::get_real_time();
        adapter.reset();

        // generic independent gradient
        kjb_c::init_real_time();
        Vector g_i = gradient_ind_cfd(ind_post, scene, step_sizes, adapter);
        long t_i = kjb_c::get_real_time();
        adapter.reset();
        ind_post.reset();

        // MT generic gradient
        kjb_c::init_real_time();
        Vector g_mt = gradient_cfd_mt(
                        posterior, scene, step_sizes, adapter, num_threads);
        long t_mt = kjb_c::get_real_time();
        adapter.reset();

        // generic independent gradient
        kjb_c::init_real_time();
        Vector g_imt = gradient_ind_cfd_mt(
                          ind_post, scene, step_sizes, adapter, num_threads);
        long t_imt = kjb_c::get_real_time();
        adapter.reset();
        ind_post.reset();

        // specialized gradient
        kjb_c::init_real_time();
        Vector g_s = grad(scene);
        long t_s = kjb_c::get_real_time();

        Vector g = g_s;
        if(VERBOSE)
        {
            cout << "Scene size: " << step_sizes.size() << endl;
            cout << "||G|| = " << g.magnitude() << endl;
            cout << "||G_i|| = " << g_i.magnitude() << endl;
            cout << "||G_mt|| = " << g_mt.magnitude() << endl;
            cout << "||G_imt|| = " << g_imt.magnitude() << endl;
            cout << "||G_s|| = " << g_s.magnitude() << endl;

            cout << "||G_s - G|| = " << vector_distance(g_s, g) << endl;
            cout << "||G_s - G_i|| = " << vector_distance(g_s, g_i) << endl;
            cout << "||G_s - G_mt|| = " << vector_distance(g_s, g_mt) << endl;
            cout << "||G_s - G_imt|| = " << vector_distance(g_s, g_imt) << endl;
            cout << "Time for G: " << (t / 1000.0) << "s." << endl;
            cout << "Time for G_i: " << (t_i / 1000.0) << "s." << endl;
            cout << "Time for G_mt: " << (t_mt / 1000.0) << "s." << endl;
            cout << "Time for G_imt: " << (t_imt / 1000.0) << "s." << endl;
            cout << "Time for G_s: " << (t_s / 1000.0) << "s." << endl;
        }

        TEST_TRUE(vector_distance(g_s, g) <= eps_lax);
        TEST_TRUE(vector_distance(g_s, g_i) <= eps);
        TEST_TRUE(vector_distance(g_s, g_mt) <= eps_lax);
        TEST_TRUE(vector_distance(g_s, g_imt) <= eps);
    }
    catch(const kjb::Exception& ex)
    {
        ex.print_details();
        cerr << endl;
        return EXIT_FAILURE;
    }

    RETURN_VICTORIOUSLY();
}

