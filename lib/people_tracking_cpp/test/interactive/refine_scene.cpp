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

#include <people_tracking_cpp/pt_scene.h>
#include <people_tracking_cpp/pt_scene_init.h>
#include <people_tracking_cpp/pt_data.h>
#include <people_tracking_cpp/pt_target.h>
#include <people_tracking_cpp/pt_association.h>
#include <people_tracking_cpp/pt_util.h>
#include <flow_cpp/flow_integral_flow.h>
#include <camera_cpp/perspective_camera.h>
#include <l_cpp/l_exception.h>
#include <vector>
#include <exception>
#include <algorithm>
#include <boost/bind.hpp>
#include "utils.h"

using namespace kjb;
using namespace kjb::pt;

int main(int argc, char** argv)
{
    try
    {
        // create (or read) scene
        size_t num_frames = 100;
        double img_width = 600;
        double img_height = 500;
        Box_data data(1, 1);
        Facemark_data fm_data;
        std::vector<Integral_flow> flows_x;
        std::vector<Integral_flow> flows_y;
        Scene scene(Ascn(data), Perspective_camera(), 0.0, 0.0, 0.0);
        create_or_read_scene(
            argc, argv, num_frames, img_width, img_height,
            data, fm_data, flows_x, flows_y, scene, 0, true);

        std::for_each(
            scene.association.begin(),
            scene.association.end(),
            boost::bind(&Target::set_changed_all, _1));
        update_facemarks(scene.association, fm_data);

        // MAKE LIKELIHOODS AND POSTERIOR
        // create likelihoods and prior
        Box_likelihood box_likelihood(1.0, img_width, img_height);

        const double face_sd = 15.0;
        const double scale_x = 0.34990;
        const double scale_y = 0.17581;
        const double gp_scale = 100.0;
        const double gp_svar = 10.0;
        const double gp_scale_dir = 200.0;
        const double gp_svar_dir = M_PI*M_PI/64.0;
        const double gp_scale_fdir = 200.0;
        const double gp_svar_fdir = M_PI*M_PI/256.0;
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
        refine_scene_state(scene, posterior);

        std::cout << "writing..." << std::endl;

        write_scene(scene, "output/refine_scene_cpp");
    }
    catch(const kjb::Exception& ex)
    {
        ex.print_details();
        std::cerr << std::endl;
        return EXIT_FAILURE;
    }
    catch(const std::exception& sex)
    {
        std::cerr << sex.what() << std::endl;
    }
}
