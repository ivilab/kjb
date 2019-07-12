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
#include <people_tracking_cpp/pt_box_likelihood.h>
#include <people_tracking_cpp/pt_optical_flow_likelihood.h>
#include <people_tracking_cpp/pt_position_prior.h>
#include <people_tracking_cpp/pt_scene_posterior.h>
#include <people_tracking_cpp/pt_scene_adapter.h>
#include <people_tracking_cpp/pt_integral_optimization.h>
#include <people_tracking_cpp/pt_util.h>
#include <people_tracking_cpp/pt_scene_init.h>
#include <st_cpp/st_perspective_camera.h>
#include <flow_cpp/flow_integral_flow.h>
#include <m_cpp/m_matrix.h>
#include <l_cpp/l_filesystem.h>
#include <l_cpp/l_test.h>
#include <l_cpp/l_util.h>
#include <l/l_sys_io.h>
#include "utils.h"
#include <vector>
#include <iomanip>
#include <iostream>
#include <algorithm>
#include <functional>
#include <boost/bind.hpp>

const bool VERBOSE = true;

using namespace std;
using namespace kjb;
using namespace kjb::pt;

/** @brief  Compute log|H|. */
double log_det_hess(const Scene& scene);

/** @brief  Print element in formatted form. */
template<class T> void print_element(const T& t, size_t width);

/** @brief  Main -- all the magic happens here. */
int main(int argc, char** argv)
{
    try
    {
        double img_width = 500;
        double img_height = 500;
        size_t num_frames = 300;
        size_t num_tracks = 1;

        // CREATE SCENE
        Box_data data(img_width, img_height);
        vector<Integral_flow> flows_x;
        vector<Integral_flow> flows_y;
        Facemark_data fm_data(num_frames);

        Scene scene1(Ascn(data), Perspective_camera(), 0.0, 0.0, 0.0);
        create_or_read_scene(argc, argv, num_frames, img_width, img_height,
                             data, fm_data, flows_x, flows_y, scene1, num_tracks);

        // WRITE DATA FOR EXAMINATION
        string outdir = "output/marginal_cpp";
        string boxdatadir = outdir + "/data/detections";
        string boxdatafmt = boxdatadir + "/%05d.txt";
        string ofxdatadir = outdir + "/data/flow/x_int_s4";
        string ofydatadir = outdir + "/data/flow/y_int_s4";
        string ofxdatafmt = ofxdatadir + "/%05d.txt";
        string ofydatafmt = ofydatadir + "/%05d.txt";
        string fmdatadir = outdir + "/data/deva_face/";
        string fmdatafmt = fmdatadir + "/%05d.txt";

        // write boxes
        kjb_c::kjb_mkdir(boxdatadir.c_str());
        vector<string> fnames = strings_from_format(boxdatafmt, num_frames);
        data.write(fnames);

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
        write_deva_facemarks(fm_data, fnames, img_width, img_height);

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

        // MAKE SECOND SCENE
        Scene scene2 = scene1;

        // MODIFY SINGLE TRACK IN ONE SCENE
        Target tg = *scene1.association.begin();
        IFT(tg.size() >= 4, Runtime_error, "First track too short; try again.");
        size_t mbox = tg.size()/2;
        Target::iterator pr_p = tg.begin();
        advance(pr_p, mbox);
        tg.erase(pr_p, tg.end());
        sync_state(tg);
        tg.set_changed_all();

        scene1.association.erase(scene1.association.begin());
        scene1.association.insert(tg);

        // UPDATE SECENE STATES
        //update_scene_state(scene1, fm_data, false);
        //update_scene_state(scene2, fm_data, false);
        update_facemarks(scene1.association, fm_data);
        update_facemarks(scene2.association, fm_data);

        // WRITE ORIGINAL SCENES
        kjb_c::kjb_mkdir((outdir + "/1/orig").c_str());
        kjb_c::kjb_mkdir((outdir + "/2/orig").c_str());
        write_scene(scene1, outdir + "/1/orig/");
        write_scene(scene2, outdir + "/2/orig/");

        // COMPUTE LIKELIHOODS
        Optimize_likelihood oplh(posterior);
        //oplh.record_log(true);
        //oplh.record_samples(true);
        //oplh.record_proposals(true);
        oplh.sampler().set_num_iterations(0);

        //oplh.set_output_directory("output/marginal_cpp/1");
        double mlh1 = oplh(scene1);
        double nlh1 = box_likelihood.at_noise(scene1);
        double fmnlh1 = fm_likelihood.at_noise(scene1);

        //oplh.set_output_directory("output/marginal_cpp/2");
        double mlh2 = oplh(scene2);
        double nlh2 = box_likelihood.at_noise(scene2);
        double fmnlh2 = fm_likelihood.at_noise(scene2);

        if(VERBOSE)
        {
            for(size_t i = 0; i < 15; ++i) cout << " ";
            print_element("scene1", 12);
            print_element("scene2", 12);
            cout << endl;

            print_element("dims", 15);
            print_element(dims(scene1), 12);
            print_element(dims(scene2), 12);
            cout << endl;

            print_element("boxes (asg)", 15);
            print_element(length(scene1.association), 12);
            print_element(length(scene2.association), 12);
            cout << endl;

            print_element("boxes (noise)", 15);
            print_element(length(scene1.association.get_available_data()), 12);
            print_element(length(scene2.association.get_available_data()), 12);
            cout << endl;

            print_element("BLH (asg)", 15);
            print_element(box_likelihood(scene1), 12);
            print_element(box_likelihood(scene2), 12);
            cout << endl;

            print_element("BLH (noise)", 15);
            print_element(nlh1, 12);
            print_element(nlh2, 12);
            cout << endl;

            print_element("BLH", 15);
            print_element(box_likelihood(scene1) + nlh1, 12);
            print_element(box_likelihood(scene2) + nlh2, 12);
            cout << endl;

            print_element("OFLH", 15);
            print_element(of_likelihood(scene1), 12);
            print_element(of_likelihood(scene2), 12);
            cout << endl;

            print_element("FFLH", 15);
            print_element(ff_likelihood(scene1), 12);
            print_element(ff_likelihood(scene2), 12);
            cout << endl;

            print_element("FMLH", 15);
            print_element(fm_likelihood(scene1), 12);
            print_element(fm_likelihood(scene2), 12);
            cout << endl;

            print_element("FMLH (noise)", 15);
            print_element(fm_likelihood.at_noise(scene1), 12);
            print_element(fm_likelihood.at_noise(scene2), 12);
            cout << endl;

            print_element("pos prior", 15);
            print_element(pos_prior(scene1), 12);
            print_element(pos_prior(scene2), 12);
            cout << endl;

            print_element("dir prior", 15);
            print_element(dir_prior(scene1), 12);
            print_element(dir_prior(scene2), 12);
            cout << endl;

            print_element("f.dir prior", 15);
            print_element(fdir_prior(scene1), 12);
            print_element(fdir_prior(scene2), 12);
            cout << endl;

            print_element("posterior", 15);
            print_element(posterior(scene1), 12);
            print_element(posterior(scene2), 12);
            cout << endl;

            print_element("log|H|", 15);
            print_element(log_det_hess(scene1), 12);
            print_element(log_det_hess(scene2), 12);
            cout << endl;

            print_element("MLH (asg)", 15);
            print_element(mlh1, 12);
            print_element(mlh2, 12);
            cout << endl;

            print_element("MLH", 15);
            print_element(mlh1 + nlh1 + fmnlh1, 12);
            print_element(mlh2 + nlh2 + fmnlh2, 12);
            cout << endl;
        }

        TEST_TRUE(mlh1 + nlh1 + fmnlh1 < mlh2 + nlh2 + fmnlh2);
        //TEST_TRUE(mlh1 + nlh1 + fmnlh1 < mlh3 + nlh3 + fmnlh3);
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

double log_det_hess(const Scene& scene)
{
    double log_det_H = 0.0;
    BOOST_FOREACH(const Target& target, scene.association)
    {
        assert(!target.hessian_diagonal().empty());

        using namespace std;
        using boost::bind;

        Vector ths = -target.hessian_diagonal();
        log_det_H += accumulate(
                        ths.begin(),
                        ths.end(), 0.0,
                        bind(plus<double>(), _1,
                             bind(static_cast<double(*)(double)>(log), _2)));
    }

    return log_det_H;
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

template<class T>
void print_element(const T& t, size_t width)
{
    cout << left << setw(width) << setfill(' ') << t;
}

