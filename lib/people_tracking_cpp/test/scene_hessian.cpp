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
#include <people_tracking_cpp/pt_target.h>
#include <people_tracking_cpp/pt_scene_posterior.h>
#include <people_tracking_cpp/pt_scene_diff.h>
#include <people_tracking_cpp/pt_scene_adapter.h>
#include <people_tracking_cpp/pt_position_prior.h>
#include <people_tracking_cpp/pt_box_likelihood.h>
#include <people_tracking_cpp/pt_optical_flow_likelihood.h>
#include <people_tracking_cpp/pt_util.h>
#include <st_cpp/st_perspective_camera.h>
#include <diff_cpp/diff_hessian.h>
#include <diff_cpp/diff_hessian_ind.h>
#include <diff_cpp/diff_hessian_ind_mt.h>
#include <flow_cpp/flow_integral_flow.h>
#include <m_cpp/m_matrix.h>
#include <m/m_mat_metric.h>
#include <l_cpp/l_test.h>
#include <l/l_sys_time.h>
#include <vector>
#include <algorithm>
#include <functional>
#include <boost/foreach.hpp>
#include <boost/bind.hpp>
#include "utils.h"

const bool VERBOSE = true;

using namespace std;
using namespace kjb;
using namespace kjb::pt;

/** @brief  Main -- all the magic happens here. */
int main(int argc, char** argv)
{
#ifdef TEST
    kjb_c::kjb_init();
    kjb_c::kjb_l_set("heap-checking", "off");
    kjb_c::kjb_l_set("initialization-checking", "off");
#endif

    const double stsz = 0.01;
    const double eps = 1e-5;

    try
    {
        double img_width = 500;
        double img_height = 500;
        size_t num_frames = 100;
        size_t num_tracks = 3;

        // CREATE SCENE
        Box_data data(img_width, img_height);
        vector<Integral_flow> flows_x;
        vector<Integral_flow> flows_y;
        Facemark_data fm_data(num_frames);

        Scene scene(Ascn(data), Perspective_camera(), 0.0, 0.0, 0.0);
        create_or_read_scene(argc, argv, num_frames, img_width, img_height,
                             data, fm_data, flows_x, flows_y, scene, num_tracks);

        const Ascn& w = scene.association;
        for_each(w.begin(), w.end(), bind(&Target::set_changed_all, _1));
        update_facemarks(scene.association, fm_data);

        // MAKE LIKELIHOODS AND POSTERIOR
        // create likelihoods and prior
        Box_likelihood box_likelihood(1.0, img_width, img_height);

        Facemark_likelihood fm_likelihood(fm_data, face_sd, img_width, img_height);

        Optical_flow_likelihood of_likelihood(
            flows_x, flows_y, img_width, img_height,
            scale_x, scale_y, bg_scale_x, bg_scale_y);

        Face_flow_likelihood ff_likelihood(
            flows_x, flows_y, img_width, img_height,
            scale_x, scale_y, bg_scale_x, bg_scale_y);

        Color_likelihood color_likelihood;

        Position_prior pos_prior(gp_scale, gp_svar, num_frames);
        Direction_prior dir_prior(gp_scale_dir, gp_svar_dir, num_frames);
        Face_direction_prior fdir_prior(gp_scale_fdir, gp_svar_fdir, num_frames);

        // create posterior
        Scene_posterior posterior(
                            box_likelihood,
                            fm_likelihood,
                            of_likelihood, 
                            ff_likelihood, 
                            color_likelihood,
                            pos_prior,
                            dir_prior,
                            fdir_prior);

        posterior.use_dim_prior() = false;
        posterior.use_color_lh() = false;

        // TEST FULLL HESSIAN
        Scene_adapter adapter;
        vector<double> step_sizes(adapter.size(&scene), stsz);
        Scene_hessian hess(posterior, step_sizes);

        if(VERBOSE)
        {
            cout << "Scene size: " << step_sizes.size() << endl;
            cout << "Computing Hessian naive way (H1)..." << endl;
        }

        kjb_c::init_real_time();
        Matrix H1 = hessian_symmetric(posterior, scene, step_sizes, adapter);
        long t1 = kjb_c::get_real_time();

        if(VERBOSE)
        {
            cout << "Computing Hessian independent way (H2)..." << endl;
        }

        // TEST INDEPENDENT HESSIAN
        kjb_c::init_real_time();
        Matrix H2 = hess(scene);
        long t2 = kjb_c::get_real_time();

        double fn12 = kjb_c::frobenius_matrix_difference(
                                                H1.get_c_matrix(),
                                                H2.get_c_matrix());

        if(VERBOSE)
        {
            cout << "||H1 - H2|| = " << fn12 << endl;
            cout << "Time for H1: " << (t1 / 1000.0) << "s." << endl;
            cout << "Time for H2: " << (t2 / 1000.0) << "s." << endl;
        }

        TEST_TRUE(fn12 <= eps);

        // TEST DIAGONAL HESSIAN
        if(VERBOSE)
        {
            cout << "Computing Hessian diagonal (H3)..." << endl;
        }

        for_each(w.begin(), w.end(), bind(&Target::set_changed_all, _1));
        kjb_c::init_real_time();
        hess.set_diagonals(scene);
        long t3 = kjb_c::get_real_time();

        Vector d3(step_sizes.size());
        size_t i = 0;
        BOOST_FOREACH(const Target& tg, scene.association)
        {
            const Vector& h = tg.hessian_diagonal();
            copy(h.begin(), h.end(), d3.begin() + i);

            i += 5*(tg.changed_end() - tg.changed_start() + 1);
        }

        Vector d2 = H2.get_diagonal();
        if(VERBOSE)
        {
            cout << "||H2 - H3|| = " << vector_distance(d2, d3) << endl;
            cout << "Time for H3: " << (t3 / 1000.0) << "s." << endl;
        }

        TEST_TRUE(vector_distance(d2, d3) <= eps);

        size_t num_threads = 6; 
        Scene_hessian hess_mt(posterior, step_sizes, num_threads);
        // set target to be changed
        for_each(w.begin(), w.end(), bind(&Target::set_changed_all, _1));
        kjb_c::init_real_time();
        hess_mt.set_diagonals(scene);
        long t_mt = kjb_c::get_real_time();

        Vector d4(step_sizes.size());
        i = 0;
        BOOST_FOREACH(const Target& tg, scene.association)
        {
            const Vector& h = tg.hessian_diagonal();
            copy(h.begin(), h.end(), d4.begin() + i);

            i += 5*(tg.changed_end() - tg.changed_start() + 1);
        }

        if(VERBOSE)
        {
            cout << "||H3|| = " << d3.magnitude() << endl;
            cout << "||H4|| = " << d4.magnitude() << endl;
            cout << "||H2 - H4|| = " << vector_distance(d3, d4) << endl;
            cout << "Time for H4: " << (t_mt / 1000.0) << "s." << endl;
        }

        TEST_TRUE(vector_distance(d3, d4) <= eps);


    }
    catch(const kjb::Exception& ex)
    {
        ex.print_details();
        cerr << endl;
        return EXIT_FAILURE;
    }

    RETURN_VICTORIOUSLY();
}

