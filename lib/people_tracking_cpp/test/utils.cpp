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

#include "utils.h"
#include <people_tracking_cpp/pt_scene.h>
#include <people_tracking_cpp/pt_association.h>
#include <people_tracking_cpp/pt_target.h>
#include <people_tracking_cpp/pt_data.h>
#include <people_tracking_cpp/pt_detection_box.h>
#include <people_tracking_cpp/pt_box_likelihood.h>
#include <people_tracking_cpp/pt_facemark_likelihood.h>
#include <people_tracking_cpp/pt_optical_flow_likelihood.h>
#include <people_tracking_cpp/pt_face_flow_likelihood.h>
#include <people_tracking_cpp/pt_position_prior.h>
#include <people_tracking_cpp/pt_direction_prior.h>
#include <people_tracking_cpp/pt_camera_prior.h>
#include <people_tracking_cpp/pt_scene_posterior.h>
#include <people_tracking_cpp/pt_scene_generative_model.h>
#include <mcmcda_cpp/mcmcda_prior.h>
#include <detector_cpp/d_bbox.h>
#include <flow_cpp/flow_integral_flow.h>
#include <l_cpp/l_serialization.h>
#include <l_cpp/l_exception.h>
#include <l_cpp/l_filesystem.h>
#include <vector>
#include <iterator>
#include <boost/foreach.hpp>
#include <boost/bind.hpp>

using namespace kjb;
using namespace kjb::pt;
using namespace std;

const double lambda_N = 0.5;
const double lambda_A = 1.0;
const double face_sd = 15.0;
const double scale_x = 1.42;
const double scale_y = 0.96;
const double bg_scale_x = 2*scale_x;
const double bg_scale_y = 2*scale_y;
const double gp_scale = 75.0;
const double gp_svar = 1000.0;
const double gp_scale_dir = 30.0;
const double gp_svar_dir = 12*M_PI;
const double gp_scale_fdir = 30.0;
const double gp_svar_fdir = M_PI/2.0;

void make_typical_scene
(
    size_t num_frames,
    Scene& scene,
    Box_data& box_data,
    Facemark_data& fm_data,
    vector<Integral_flow>& flows_x,
    vector<Integral_flow>& flows_y,
    size_t max_tracks
)
{
    // params
    const double eta = 0.1;
    const double theta = num_frames / 2.0;
    const double kappa = eta / theta;
    double image_width = box_data.image_width();
    double image_height = box_data.image_height();

    // distributions
    mcmcda::Prior<Target> w_prior(kappa, theta, lambda_N, lambda_A);

    Box_likelihood box_likelihood(1.0, image_width, image_height);

    Facemark_likelihood fm_likelihood(fm_data, face_sd, image_width, image_height);

    Optical_flow_likelihood of_likelihood(
                    vector<Integral_flow>(), vector<Integral_flow>(),
                    image_width, image_height,
                    scale_x, scale_y, bg_scale_x, bg_scale_y);

    Face_flow_likelihood ff_likelihood(
                    vector<Integral_flow>(), vector<Integral_flow>(),
                    image_width, image_height,
                    scale_x, scale_y, bg_scale_x, bg_scale_y);

    Color_likelihood color_likelihood;

    // position and direction priors
    Position_prior pos_prior(gp_scale, gp_svar);
    Direction_prior dir_prior(gp_scale_dir, gp_svar_dir);
    Face_direction_prior fdir_prior(gp_scale_fdir, gp_svar_fdir);
    Camera_prior cam_prior(2.0, 0.5, 0.0, M_PI/10, image_width/2.0, 100.0);

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

    // sample
    sample(
        w_prior,
        cam_prior,
        posterior,
        num_frames,
        scene,
        box_data,
        fm_data.begin(),
        back_inserter(flows_x),
        back_inserter(flows_y),
        max_tracks);

    IFT(!scene.association.empty(), Runtime_error,
        "Bad luck: sampled empty association. Try again!");
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

vector<string> create_or_read_scene
(
    int argc,
    char** argv,
    size_t& num_frames,
    double& img_width,
    double& img_height,
    Box_data& box_data,
    Facemark_data& fm_data,
    vector<Integral_flow>& flows_x,
    vector<Integral_flow>& flows_y,
    Scene& scene,
    size_t max_tracks,
    bool read_frame
)
{
    string data_dp;
    string fm_dp;
    string feat_dp;
    string scene_dp;
    string gt_dp;
    string cam_fp;
    vector<string> frame_fps;

    if(argc == 5)
    {
        data_dp = argv[1];
        fm_dp = argv[2];
        feat_dp = argv[3];
        scene_dp = argv[4];
    }

    if(argc == 6)
    {
        data_dp = argv[1];
        fm_dp = argv[2];
        feat_dp = argv[3];
        gt_dp = argv[4];
        cam_fp = argv[5];
    }

    if(argc == 5 || argc == 6)
    {
        // READ DATA AND CREATE SCENE
        // create and read flow
        vector<string> x_of_fps = 
            file_names_from_format(feat_dp + "/x_int_s4/%05d.txt");
        vector<string> y_of_fps = 
            file_names_from_format(feat_dp + "/y_int_s4/%05d.txt");

        num_frames = x_of_fps.size() + 1;
        IFT(num_frames > 1, Runtime_error, "Could not read features.");

        flows_x.reserve(num_frames - 1);
        flows_y.reserve(num_frames - 1);
        copy(x_of_fps.begin(), x_of_fps.end(), back_inserter(flows_x));
        copy(y_of_fps.begin(), y_of_fps.end(), back_inserter(flows_y));
        //copy(x_of_fps.begin(), x_of_fps.begin() + 1, back_inserter(flows_x));
        //copy(y_of_fps.begin(), y_of_fps.begin() + 1, back_inserter(flows_y));

        img_width = flows_x.front().img_width();
        img_height = flows_x.front().img_height();

        if(read_frame)
        {
            string movie_dp = data_dp + "/../../";
            frame_fps = file_names_from_format(movie_dp + "/frames/%05d.jpg");
        }

        // create and read box data
        box_data = Box_data(img_width, img_height, 0.98);
        box_data.read(file_names_from_format(data_dp + "/%05d.txt"));
        assert(num_frames == box_data.size());

        // create and read FM data
        fm_data = parse_deva_facemarks(
                        file_names_from_format(fm_dp + "/%05d.txt"),
                        img_width,
                        img_height);
        assert(num_frames == fm_data.size());

        if(argc == 5)
        {
            // read scene
            read_scene(scene, scene_dp);
        }
        else
        {
            load(scene.camera, cam_fp);
            Box_trajectory_map gt_bt_map(num_frames);
            gt_bt_map.parse(gt_dp, "person");

            scene.association = make_gt_association(
                                                box_data,
                                                gt_bt_map,
                                                img_width,
                                                img_height,
                                                scene.camera);
                                                //scene.camera,
                                                //fm_data);
        }

        if(!scene.association.empty() && max_tracks != 0)
        {
            if(scene.association.size() > max_tracks)
            {
                Ascn::iterator tg_p = scene.association.begin();
                std::advance(tg_p, max_tracks);
                scene.association.erase(tg_p, scene.association.end());
            }
        }
    }
    else if(argc == 1)
    {
        // GENERATE SCENE
        box_data = Box_data(img_width, img_height, 0.99);
        fm_data.resize(num_frames);
        flows_x.reserve(num_frames - 1);
        flows_y.reserve(num_frames - 1);
        make_typical_scene(num_frames, scene, box_data,
                           fm_data, flows_x, flows_y, max_tracks);
    }
    else
    {
        std::string usg = "Usage: ";
        usg += argv[0];
        usg += " [ data-dir facemark-dir flow-dir";
        usg += " [ scene-dir | gt-btraj-dir cam-dir ] ]";

        KJB_THROW_2(Exception, usg);
    }

    return frame_fps;
}

