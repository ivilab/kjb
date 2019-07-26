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
#include <people_tracking_cpp/pt_association.h>
#include <people_tracking_cpp/pt_data.h>
#include <people_tracking_cpp/pt_box_likelihood.h>
#include <people_tracking_cpp/pt_facemark_likelihood.h>
#include <people_tracking_cpp/pt_optical_flow_likelihood.h>
#include <people_tracking_cpp/pt_face_flow_likelihood.h>
#include <people_tracking_cpp/pt_color_likelihood.h>
#include <people_tracking_cpp/pt_position_prior.h>
#include <people_tracking_cpp/pt_direction_prior.h>
#include <people_tracking_cpp/pt_camera_prior.h>
#include <people_tracking_cpp/pt_scene_posterior.h>
#include <people_tracking_cpp/pt_scene_generative_model.h>
#include <mcmcda_cpp/mcmcda_prior.h>
#include <st_cpp/st_perspective_camera.h>
#include <flow_cpp/flow_integral_flow.h>
#include <l_cpp/l_filesystem.h>
#include <l/l_sys_mal.h>
#include <l/l_sys_io.h>
#include <detector_cpp/d_bbox.h>
#include <vector>
#include <string>
#include <iterator>

using namespace std;
using namespace kjb;
using namespace kjb::pt;

/** @brief  Main -- all the magic happens here. */
int main(int argc, char** argv)
{
//#ifdef TEST
    //kjb_c::kjb_init();
    //kjb_c::kjb_l_set("heap-checking", "off");
    //kjb_c::kjb_l_set("initialization-checking", "off");
//#endif

    // global stuff
    const double image_width = 500;
    const double image_height = 500;
    const size_t num_frames = 200;

    // association prior params
    const double eta = 0.1;
    const double theta = num_frames / 2.0;
    const double kappa = eta / theta;
    const double lambda_N = 0.5;
    const double lambda_A = 1.5;

    // position prior and likelihood params
    const double face_sd = 1.0;
    const double scale_x = 0.34990;
    const double scale_y = 0.17581;
    const double gp_scale = 100.0;
    const double gp_svar = 10.0;
    const double gp_scale_dir = 200.0;
    const double gp_svar_dir = M_PI*M_PI/64.0;
    const double gp_scale_fdir = 200.0;
    const double gp_svar_fdir = M_PI*M_PI/256.0;

    //data
    Box_data box_data(image_width, image_height);
    Scene scene(Ascn(box_data), Perspective_camera(), 1.0, 1.0, 1.0);
    Facemark_data fm_data(num_frames);
    vector<Integral_flow> flows_x;
    vector<Integral_flow> flows_y;
    flows_x.reserve(num_frames - 1);
    flows_y.reserve(num_frames - 1);

    // distributions
    mcmcda::Prior<Target> w_prior(kappa, theta, lambda_N, lambda_A);

    Box_likelihood box_likelihood(1.0, image_width, image_height);

    Facemark_likelihood fm_likelihood(fm_data, face_sd, image_width, image_height);

    Optical_flow_likelihood of_likelihood(
                    vector<Integral_flow>(), vector<Integral_flow>(),
                    image_width, image_height,
                    scale_x, scale_y, 2*scale_x, 2*scale_y);

    Face_flow_likelihood ff_likelihood(
                    vector<Integral_flow>(), vector<Integral_flow>(),
                    image_width, image_height,
                    scale_x, scale_y, 2*scale_x, 2*scale_y);

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
    size_t max_tracks = 5;
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

    // write
    string outdir = "output/generate_scene_cpp";
    string scenedir = outdir + "/scene";
    string boxdatadir = outdir + "/data/detection_boxes/person";
    string boxdatafmt = boxdatadir + "/%05d.txt";
    string ofxdatadir = outdir + "/data/features/optical_flow/brox/x_int_s4";
    string ofydatadir = outdir + "/data/features/optical_flow/brox/y_int_s4";
    string ofxdatafmt = ofxdatadir + "/%05d.txt";
    string ofydatafmt = ofydatadir + "/%05d.txt";
    string fmdatadir = outdir + "/data/features/deva_face";
    string fmdatafmt = fmdatadir + "/%05d.txt";

    // write scene
    kjb_c::kjb_mkdir(scenedir.c_str());
    write_scene(scene, scenedir);

    // write boxes
    kjb_c::kjb_mkdir(boxdatadir.c_str());
    vector<string> fnames = strings_from_format(boxdatafmt, num_frames);
    box_data.write(fnames);

    // write OF features
    kjb_c::kjb_mkdir(ofxdatadir.c_str());
    kjb_c::kjb_mkdir(ofydatadir.c_str());
    vector<string> fnames_x = strings_from_format(ofxdatafmt, num_frames - 1);
    vector<string> fnames_y = strings_from_format(ofydatafmt, num_frames - 1);
    for(size_t i = 0; i < num_frames - 1; i++)
    {
        flows_x[i].write(fnames_x[i]);
        flows_y[i].write(fnames_y[i]);
    }

    // write FM data
    kjb_c::kjb_mkdir(fmdatadir.c_str());
    fnames = strings_from_format(fmdatafmt, num_frames);
    write_deva_facemarks(fm_data, fnames, image_width, image_height);

    return EXIT_SUCCESS;
}

