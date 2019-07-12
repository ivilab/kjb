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
#include <people_tracking_cpp/pt_scene_info.h>
#include <people_tracking_cpp/pt_sample_scenes.h>
#include <people_tracking_cpp/pt_integral_optimization.h>
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
#include <sstream>

const bool VERBOSE = true;

using namespace std;
using namespace kjb;
using namespace kjb::pt;

/** @brief  Compare two scene info structs. */
bool compare_infos(const Scene_info& si1, const Scene_info& si2);

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

        Scene_info info0;
        info0.set(posterior, scene, false);
        TEST_TRUE(std::isnan(info0.marg_lh));

        if(VERBOSE)
        {
            cout << "Before optimization:" << endl;
            scene_info_table(info0, cout);
            cout << endl;
        }

        // get scene ready
        Optimize_likelihood optlh(posterior);
        optlh.sampler().set_num_iterations(0);
        optlh(scene);

        // create scene info and check
        Scene_info info1;
        info1.set(posterior, scene, true);
        TEST_TRUE(info1.is_valid(scene, posterior.infer_head()));
        TEST_FALSE(std::isnan(info1.marg_lh));

        if(VERBOSE)
        {
            cout << "After optimization:" << endl;
            scene_info_table(info1, cout);
            cout << endl;
        }

        // improved posterior test
        double feps = 1e-5;
        TEST_TRUE(info0.pt - feps <= info1.pt);

        // write info
        std::stringstream ss;
        ss << info1;

        // read info
        Scene_info info2;
        ss >> info2;
        TEST_TRUE(info2.is_valid(scene, posterior.infer_head()));
        TEST_TRUE(compare_infos(info1, info2));

        if(VERBOSE)
        {
            cout << "After reading:" << endl;
            scene_info_table(info2, cout);
            cout << endl;
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

bool compare_infos(const Scene_info& si1, const Scene_info& si2)
{
    const double feps = 1e-4;

    bool marg_lhs_comp;
    if(std::isnan(si1.marg_lh) && std::isnan(si2.marg_lh))
    {
        marg_lhs_comp = true;
    }
    else if(!std::isnan(si1.marg_lh) && !std::isnan(si2.marg_lh))
    {
        marg_lhs_comp = abs(si1.marg_lh - si2.marg_lh) <= feps;
    }
    else
    {
        return false;
    }

    return abs(si1.size_prior - si2.size_prior) <= feps
            && abs(si1.pos_prior - si2.pos_prior) <= feps
            && abs(si1.dir_prior - si2.dir_prior) <= feps
            && abs(si1.fdir_prior - si2.fdir_prior) <= feps
            && abs(si1.sp_prior - si2.sp_prior) <= feps
            && abs(si1.ep_prior - si2.ep_prior) <= feps
            && abs(si1.box_lh - si2.box_lh) <= feps
            && abs(si1.box_nlh - si2.box_nlh) <= feps
            && abs(si1.oflow_lh - si2.oflow_lh) <= feps
            && abs(si1.fflow_lh - si2.fflow_lh) <= feps
            && abs(si1.fmark_lh - si2.fmark_lh) <= feps
            && abs(si1.fmark_nlh - si2.fmark_nlh) <= feps
            && marg_lhs_comp
            && abs(si1.pt - si2.pt) <= feps
            && si1.dim == si2.dim
            && si1.num_aboxes == si2.num_aboxes
            && si1.num_nboxes == si2.num_nboxes
            && si1.num_afmarks == si2.num_afmarks
            && si1.num_nfmarks == si2.num_nfmarks;
}

