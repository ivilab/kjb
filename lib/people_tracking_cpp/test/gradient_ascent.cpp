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
#include <people_tracking_cpp/pt_scene_adapter.h>
#include <people_tracking_cpp/pt_position_prior.h>
#include <people_tracking_cpp/pt_box_likelihood.h>
#include <people_tracking_cpp/pt_optical_flow_likelihood.h>
#include <people_tracking_cpp/pt_scene_init.h>
#include <st_cpp/st_perspective_camera.h>
#include <flow_cpp/flow_integral_flow.h>
#include <m_cpp/m_matrix.h>
#include <diff_cpp/diff_optim.h>
#include <l_cpp/l_test.h>
#include <l/l_sys_time.h>
#include <l/l_sys_mal.h>
#include "utils.h"
#include <vector>
#include <utility>
#include <algorithm>

const bool VERBOSE = true;

using namespace std;
using namespace kjb;
using namespace kjb::pt;

/** @brief  Check if scene is max (wrt ss). */
bool we_is_at_max
(
    const Scene& sc,
    const Scene_posterior& pt,
    const vector<double>& ss
);

/** @brief  Main -- all the magic happens here. */
int main(int argc, char** argv)
{
    try
    {
        double img_width = 500;
        double img_height = 500;
        const size_t num_tracks = 2;
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
        update_scene_state(scene, fm_data);

        // MAKE LIKELIHOODS AND POSTERIOR
        Box_likelihood box_likelihood(
                        //Detection_box(Bbox(), 0.0, "temp"),
                        1.0, img_width, img_height);

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
        posterior.use_dim_prior() = false;

        // create gradient and adapter
        vector<double> dx = trajectory_gradient_step_sizes(scene);
        Scene_gradient grad(posterior, dx);

        Scene_adapter adapter(posterior.vis_off());
        const size_t D = adapter.size(&scene);
        vector<double> ss(D, 0.001);

        // TEST STUFF
        double pt1 = posterior(scene);

        kjb_c::init_cpu_time();
        gradient_ascent(posterior, scene, ss, grad, adapter);
        long t1 = kjb_c::get_cpu_time();

        double pt2 = posterior(scene);

        if(VERBOSE)
        {
            cout << "Before GA: " << pt1 << endl;
            cout << "After GA: " << pt2 << endl;
            cout << "Difference: " << pt2 - pt1 << endl;
            cout << "GA time: " << t1 / 1000.0 << endl;
        }

        TEST_TRUE(pt2  + 1e-6 >= pt1);

        dx = trajectory_gradient_step_sizes(scene);
        if(VERBOSE)
        {
            cout << "Before make_max() ";
            if(we_is_at_max(scene, posterior, dx))
            {
                cout << "we are already at max." << endl;
            }
            else
            {
                cout << "there are some dimensions not at max." << endl;
            }
        }

        make_max(scene, posterior, dx, true);
        TEST_TRUE(we_is_at_max(scene, posterior, dx));

        if(VERBOSE)
        {
            cout << "Final: " << posterior(scene) << endl;
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

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

bool we_is_at_max
(
    const Scene& scene,
    const Scene_posterior& pt,
    const vector<double>& ss
)
{
    Scene sc = scene;
    Scene_adapter aptr(pt.vis_off());
    const size_t D = aptr.size(&sc);

    double cur_pt = pt(sc);
    for(size_t i = 0; i < D; i++)
    {
        // check left
        aptr.set(&sc, i, aptr.get(&sc, i) - ss[i]);
        double left_pt = pt(sc);

        if(left_pt > cur_pt) return false;
        aptr.set(&sc, i, aptr.get(&sc, i) + ss[i]);

        // check right
        aptr.set(&sc, i, aptr.get(&sc, i) + ss[i]);
        double right_pt = pt(sc);

        if(right_pt > cur_pt) return false;
        aptr.set(&sc, i, aptr.get(&sc, i) - ss[i]);
    }

    return true;
}

