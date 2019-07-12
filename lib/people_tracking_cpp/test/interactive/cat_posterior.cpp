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
#include <st_cpp/st_perspective_camera.h>
#include <flow_cpp/flow_integral_flow.h>
#include <m_cpp/m_matrix.h>
#include <l_cpp/l_filesystem.h>
#include <l_cpp/l_test.h>
#include <l_cpp/l_serialization.h>
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

/** @brief  Compute total length of 2D container. */
//template<class C> size_t length(const C& container);

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

        // READ SCENE
        Box_data data(img_width, img_height, 0.98);
        vector<Integral_flow> flows_x;
        vector<Integral_flow> flows_y;
        Facemark_data fm_data(num_frames);

        Scene scene(Ascn(data), Perspective_camera(), 0.0, 0.0, 0.0);
        create_or_read_scene(argc, argv, num_frames, img_width, img_height,
                             data, fm_data, flows_x, flows_y, scene);

        // set all to changed
        std::for_each(
            scene.association.begin(),
            scene.association.end(),
            boost::bind(&Target::set_changed_all, _1));

        // MAKE LIKELIHOODS AND POSTERIOR
        // create likelihoods and prior
        Box_likelihood box_likelihood(1.0, img_width, img_height);

        double face_sd = 15.0;
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

        // UPDATE SECENE STATES
        update_facemarks(scene.association, fm_data);

        Optimize_likelihood oplh(posterior);
        oplh.sampler().set_num_iterations(0);

        //double mlh = oplh(scene);
        double mlh = 0.0;
        double nlh = box_likelihood.at_noise(scene);
        double fmnlh = fm_likelihood.at_noise(scene);

        if(VERBOSE)
        {
            print_element("dims", 15);
            print_element(dims(scene), 12);
            cout << endl;

            print_element("boxes (asg)", 15);
            print_element(length(scene.association), 12);
            cout << endl;

            print_element("boxes (noise)", 15);
            print_element(length(scene.association.get_available_data()), 12);
            cout << endl;

            print_element("BLH (asg)", 15);
            print_element(box_likelihood(scene), 12);
            cout << endl;

            print_element("BLH (noise)", 15);
            print_element(nlh, 12);
            cout << endl;

            print_element("BLH", 15);
            print_element(box_likelihood(scene) + nlh, 12);
            cout << endl;

            print_element("OFLH", 15);
            print_element(of_likelihood(scene), 12);
            cout << endl;

            print_element("FFLH", 15);
            print_element(ff_likelihood(scene), 12);
            cout << endl;

            print_element("FMLH", 15);
            print_element(fm_likelihood(scene), 12);
            cout << endl;

            print_element("FMLH (noise)", 15);
            print_element(fm_likelihood.at_noise(scene), 12);
            cout << endl;

            print_element("pos prior", 15);
            print_element(pos_prior(scene), 12);
            cout << endl;

            print_element("dir prior", 15);
            print_element(dir_prior(scene), 12);
            cout << endl;

            print_element("f.dir prior", 15);
            print_element(fdir_prior(scene), 12);
            cout << endl;

            print_element("size prior", 15);
            print_element(posterior.dimension_prior(scene), 12);
            cout << endl;

            double pos = posterior(scene);
            print_element("posterior", 15);
            print_element(pos, 12);
            cout << endl;

            //print_element("log|H|", 15);
            //print_element(log_det_hess(scene), 12);
            //cout << endl;

            if(mlh == 0.0)
            {
                print_element("MLH", 15);
                print_element(pos + nlh + fmnlh, 12);
                cout << endl;
            }
            else
            {
                print_element("MLH (asg)", 15);
                print_element(mlh, 12);
                cout << endl;

                print_element("MLH", 15);
                print_element(mlh + nlh + fmnlh, 12);
                cout << endl;
            }
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

template<class C>
size_t length(const C& cner)
{
    size_t len = 0;
    for(typename C::const_iterator p = cner.begin(); p != cner.end(); ++p)
    {
        len += p->size();
    }

    return len;
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

template<class T>
void print_element(const T& t, size_t width)
{
    cout << left << setw(width) << setfill(' ') << t;
}

