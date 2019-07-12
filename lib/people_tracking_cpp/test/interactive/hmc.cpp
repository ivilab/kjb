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

/* $Id: hmc.cpp 19393 2015-06-05 16:49:17Z ernesto $ */

#include <people_tracking_cpp/pt_scene.h>
#include <people_tracking_cpp/pt_association.h>
#include <people_tracking_cpp/pt_target.h>
#include <people_tracking_cpp/pt_data.h>
#include <people_tracking_cpp/pt_box_likelihood.h>
#include <people_tracking_cpp/pt_facemark_likelihood.h>
#include <people_tracking_cpp/pt_optical_flow_likelihood.h>
#include <people_tracking_cpp/pt_color_likelihood.h>
#include <people_tracking_cpp/pt_position_prior.h>
#include <people_tracking_cpp/pt_direction_prior.h>
#include <people_tracking_cpp/pt_scene_posterior.h>
#include <people_tracking_cpp/pt_integral_optimization.h>
#include <people_tracking_cpp/pt_util.h>
#include <people_tracking_cpp/pt_scene_init.h>
#include <st_cpp/st_perspective_camera.h>
#include <flow_cpp/flow_integral_flow.h>
#include <l_cpp/l_filesystem.h>
#include <l_cpp/l_exception.h>
#include <l/l_sys_mal.h>
#include <l/l_sys_io.h>
#include "utils.h"
#include <vector>
#include <string>
#include <iostream>
#include <boost/optional.hpp>
#include <boost/foreach.hpp>

#ifdef KJB_HAVE_ERGO

#include <ergo/record.h>

using namespace std;
using namespace kjb;
using namespace kjb::pt;
using namespace ergo;
using namespace boost;

const bool uss = false;
const bool rss = false;

/** @brief  Main -- all the magic happens here. */
int main(int argc, char** argv)
{
    try
    {
        // create (or read) scene
        size_t num_frames = 200;
        double img_width = 1200;
        double img_height = 800;
        Box_data data(1, 1);
        Facemark_data fm_data;
        vector<Integral_flow> flows_x;
        vector<Integral_flow> flows_y;
        Scene scene(Ascn(data), Perspective_camera(), 0.0, 0.0, 0.0);
        create_or_read_scene(argc, argv, num_frames, img_width, img_height,
                             data, fm_data, flows_x, flows_y, scene);

        // prepare output dir
        string outdir = "output/hmc_cpp";
        kjb_c::kjb_mkdir(outdir.c_str());

        // write data
        string datadir = outdir + "/data";
        kjb_c::kjb_mkdir(datadir.c_str());
        vector<string> fnames = strings_from_format(
                                                datadir + "/%05d.txt",
                                                num_frames);
        data.write(fnames);

        // reset scene and save ground truth scene
        Scene orig_scene = scene;
        const Perspective_camera& cam = scene.camera;
        BOOST_FOREACH(const Target& target, scene.association)
        {
            target.set_changed_all();
            target.update_pos_gp(gp_scale, gp_svar);
            target.update_dir_gp(gp_scale_dir, gp_svar_dir);
            target.update_fdir_gp(gp_scale_fdir, gp_svar_fdir);
        }

        update_facemarks(scene.association, fm_data);
        if(uss) update_scene_state(scene, fm_data);
        if(rss) refine_scene_state(scene, fm_data, face_sd);
        update_visibilities(scene);

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

        Scene_posterior posterior(
                            box_likelihood,
                            fm_likelihood,
                            of_likelihood, 
                            ff_likelihood, 
                            color_likelihood,
                            pos_prior,
                            dir_prior,
                            fdir_prior);

        // create sampler
        const size_t num_samples = 25;
        Sample_scenes sample_scenes(posterior);
        sample_scenes.set_num_iterations(num_samples);
        sample_scenes.set_num_leapfrog_steps(2);
        sample_scenes.set_hmc_step_size(0.0001);
        sample_scenes.set_num_threads(4);

        // write scene
        write_scene(scene, outdir + "/original/0001");

        // write GT scene
        write_scene(orig_scene, outdir + "/ground-truth/0001");

        // sample
        ofstream log_fs((outdir + "/sample_logs.txt").c_str());
        ofstream pd_fs((outdir + "/pp_detail.txt").c_str());
        ostream_iterator<step_detail> log_out(log_fs, "\n");
        size_t smp_n = 1;
        size_t prp_n = 1;
        Write_scene_iterator sample_out(outdir + "/samples", smp_n);
        Write_scene_iterator proposal_out(outdir + "/proposals", prp_n);
        std::ostream_iterator<std::string> pd_out(pd_fs, "\n==========\n\n");

        sample_scenes(
                    scene,
                    make_optional(log_out),
                    make_optional(sample_out),
                    make_optional(proposal_out), 
                    make_optional(pd_out));

        cout << "Ran HMC for " << num_samples << "." << endl;
        cout << "Output written to " << outdir << endl;

        // write GT scene
        write_scene(scene, outdir + "/best");
    }
    catch(const kjb::Exception& ex)
    {
        ex.print_details();
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

#else
int main() { std::cout << "Cannot run. Need libergo.\n"; return EXIT_FAILURE; }
#endif

