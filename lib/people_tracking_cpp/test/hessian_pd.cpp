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
#include <people_tracking_cpp/pt_scene_diff.h>
#include <people_tracking_cpp/pt_position_prior.h>
#include <people_tracking_cpp/pt_box_likelihood.h>
#include <people_tracking_cpp/pt_optical_flow_likelihood.h>
#include <people_tracking_cpp/pt_scene_adapter.h>
#include <people_tracking_cpp/pt_scene_init.h>
#include <flow_cpp/flow_integral_flow.h>
#include <m_cpp/m_matrix.h>
#include <diff_cpp/diff_optim.h>
#include <l_cpp/l_test.h>
#include <vector>
#include <algorithm>

#include "utils.h"
const bool VERBOSE = true;

using namespace std;
using namespace kjb;
using namespace kjb::pt;

/** @brief  Main -- all the magic happens here. */
int main(int argc, char** argv)
{
    try
    {
        double img_width = 500;
        double img_height = 500;
        const size_t num_tracks = 3;
        size_t num_frames = 50;

        // CREATE SCENE
        Box_data data(img_width, img_height);
        Facemark_data fm_data;
        vector<Integral_flow> flows_x;
        vector<Integral_flow> flows_y;
        Scene scene(Ascn(data), Perspective_camera(), 0.0, 0.0, 0.0);
        create_or_read_scene(argc, argv, num_frames, img_width, img_height,
                             data, fm_data, flows_x, flows_y, scene, num_tracks);

        // forget true 3D state
        update_scene_state(scene, fm_data, false);

        // MAKE LIKELIHOODS AND POSTERIOR
        Box_likelihood box_likelihood(1.0, img_width, img_height);

        Facemark_likelihood fm_likelihood(fm_data, face_sd, img_width, img_height);

        Optical_flow_likelihood of_likelihood(
                        flows_x, flows_y,
                        img_width, img_height,
                        scale_x, scale_y, 2*scale_x, 2*scale_y);

        Face_flow_likelihood ff_likelihood(
                        flows_x, flows_y,
                        img_width, img_height,
                        scale_x, scale_y, 2*scale_x, 2*scale_y);

        Color_likelihood color_likelihood;

        // position and direction priors
        Position_prior pos_prior(gp_scale, gp_svar);
        Direction_prior dir_prior(gp_scale_dir, gp_svar_dir);
        Face_direction_prior fdir_prior(gp_scale_fdir, gp_svar_fdir);

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

        // TEST HESSIAN PD-NESS
        // first get to max
        vector<double> dx = trajectory_gradient_step_sizes(scene);
        Scene_gradient grad(posterior, dx);

        vector<double> ss(dx.size(), 0.0001);
        Scene_adapter adapter(posterior.vis_off());
        gradient_ascent(posterior, scene, ss, grad, adapter);
        dx = trajectory_gradient_step_sizes(scene);
        make_max(scene, posterior, dx, true);

        // compute diagonal hessian
        Scene_hessian hess(posterior, dx);
        hess.set_diagonals(scene);

        // make sure it's PD
        Vector h(dx.size());
        size_t i = 0;
        BOOST_FOREACH(const Target& tg, scene.association)
        {
            const Vector& tgh = tg.hessian_diagonal();
            copy(tgh.begin(), tgh.end(), h.begin() + i);

            i += 5*(tg.changed_end() - tg.changed_start() + 1);
        }

        double mx = *max_element(h.begin(), h.end());

        if(VERBOSE)
        {
            cout << "Scene size: " << dx.size() << endl;
            cout << "max(h) = " << mx << endl;
        }

        TEST_TRUE(mx < 0);
    }
    catch(const kjb::Exception& ex)
    {
        ex.print_details();
        cerr << endl;
        return EXIT_FAILURE;
    }

    RETURN_VICTORIOUSLY();
}

