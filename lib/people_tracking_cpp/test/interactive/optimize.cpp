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
#include <people_tracking_cpp/pt_target.h>
#include <people_tracking_cpp/pt_body_2d_trajectory.h>
#include <people_tracking_cpp/pt_data.h>
#include <people_tracking_cpp/pt_detection_box.h>
#include <people_tracking_cpp/pt_util.h>
#include <people_tracking_cpp/pt_scene_posterior.h>
#include <people_tracking_cpp/pt_position_prior.h>
#include <people_tracking_cpp/pt_box_likelihood.h>
#include <people_tracking_cpp/pt_facemark_likelihood.h>
#include <people_tracking_cpp/pt_optical_flow_likelihood.h>
#include <people_tracking_cpp/pt_scene_adapter.h>
#include <st_cpp/st_perspective_camera.h>
#include <flow_cpp/flow_integral_flow.h>
#include <m_cpp/m_matrix.h>
#include <diff_cpp/diff_optim.h>
#include <gr_cpp/gr_2D_bounding_box.h>
#include <i_cpp/i_image.h>
#include <l_cpp/l_test.h>
#include <l/l_sys_io.h>
#include <detector_cpp/d_bbox.h>
#include "utils.h"
#include <vector>
#include <algorithm>
#include <functional>
#include <boost/foreach.hpp>

using namespace std;
using namespace kjb;
using namespace kjb::pt;
using namespace boost;

/** @brief  Main -- all the magic happens here. */
int main(int argc, char** argv)
{
    string outdir = "output/optimize_cpp";
    kjb_c::kjb_mkdir(outdir.c_str());

    try
    {
        // CREATE SCENE
        size_t num_frames = 2;
        double img_width = 1200;
        double img_height = 800;
        Box_data data(1, 1);
        Facemark_data fm_data;
        vector<Integral_flow> flows_x;
        vector<Integral_flow> flows_y;
        Scene scene(Ascn(data), Perspective_camera(), 0.0, 0.0, 0.0);
        create_or_read_scene(argc, argv, num_frames, img_width, img_height,
                             data, fm_data, flows_x, flows_y, scene, 1);

        // MAKE LIKELIHOODS AND POSTERIOR
        Box_likelihood box_likelihood(1.0, img_width, img_height);

        const double face_sd = 1.0;
        Facemark_likelihood fm_likelihood(fm_data, face_sd, img_width, img_height);

        const double scale_x = 0.34990;
        const double scale_y = 0.17581;
        Optical_flow_likelihood of_likelihood(
            flows_x, flows_y, img_width, img_height,
            scale_x, scale_y, 2*scale_x, 2*scale_y);

        Face_flow_likelihood ff_likelihood(
            flows_x, flows_y, img_width, img_height,
            scale_x, scale_y, 2*scale_x, 2*scale_y);

        Color_likelihood color_likelihood; 

        const double gpsc = 100.0;
        const double gpsv = 10.0;
        Position_prior pos_prior(gpsc, gpsv);

        const double gpsc_dir = 200.0;
        const double gpsv_dir = M_PI*M_PI/64.0;
        Direction_prior dir_prior(gpsc_dir, gpsv_dir);

        const double gpsc_fdir = 200.0;
        const double gpsv_fdir = M_PI*M_PI/256.0;
        Face_direction_prior fdir_prior(gpsc_fdir, gpsv_fdir);

        Scene_posterior posterior(
                            box_likelihood,
                            fm_likelihood,
                            of_likelihood, 
                            ff_likelihood, 
                            color_likelihood,
                            pos_prior,
                            dir_prior,
                            fdir_prior);

        // TEST STUFF
        Scene_adapter adapter(&scene);
        vector<pair<double, double> > bounds(adapter.size(&scene));
        const double buf = 0.5;
        for(size_t i = 0; i < bounds.size(); i++)
        {
            bounds[i].first = adapter.get(&scene, i) - buf;
            bounds[i].second = adapter.get(&scene, i) + buf;
        }

        const size_t nbins = 30;
        Scene mxsc = scene;
        double mx = grid_maximize(posterior, bounds, nbins, adapter, mxsc);

        // DRAW RESULT ON IMAGE
        Image I_o(img_width, img_height, 0, 0, 0);
        Image I_m(img_width, img_height, 0, 0, 0);

        // draw data
        for(size_t t = 0; t < num_frames; t++)
        {
            BOOST_FOREACH(const Detection_box& dbox, data[t])
            {
                Bbox bbox = dbox.bbox;
                unstandardize(bbox, img_width, img_height);
                bbox.draw(I_o, 255.0, 255.0, 0.0, 1.0);
                bbox.draw(I_m, 255.0, 255.0, 0.0, 1.0);
            }
        }

        // draw model (initial)
        BOOST_FOREACH(const Target& tg, scene.association)
        {
            int sf = tg.get_start_time();
            int ef = tg.get_end_time();
            assert(sf != -1 && ef != -1);
            const Body_2d_trajectory& btraj = tg.body_trajectory();
            for(size_t t = (size_t)sf; t <= (size_t)ef; t++)
            {
                Bbox bbox = btraj[t - 1]->value.full_bbox;
                unstandardize(bbox, img_width, img_height);
                bbox.draw(I_o, 255.0, 0.0, 0.0, 1.0);
            }
        }

        // draw model (max)
        BOOST_FOREACH(const Target& tg, mxsc.association)
        {
            int sf = tg.get_start_time();
            int ef = tg.get_end_time();
            const Body_2d_trajectory& btraj = tg.body_trajectory();
            for(size_t t = sf; t <= ef; t++)
            {
                Bbox bbox = btraj[t - 1]->value.full_bbox;
                unstandardize(bbox, img_width, img_height);
                bbox.draw(I_m, 255.0, 0.0, 0.0, 1.0);
            }
        }

        kjb_c::kjb_mkdir(outdir.c_str());

        // write images and models
        I_o.write(outdir + "/initial.jpg");
        const string outdir_o = outdir + "/initial";
        write_scene(scene, outdir_o);

        I_m.write(outdir + "/max.jpg");
        const string outdir_m = outdir + "/max";
        write_scene(mxsc, outdir_m);

        // report values
        cout << "p(z_0) = " << posterior(scene) << endl;
        cout << "p(z*) = " << posterior(mxsc) << endl;

        // compute "plot"
        const double xmin = adapter.get(&mxsc, 0) - 1.0;
        const double xmax = adapter.get(&mxsc, 0) + 1.0;
        const double dx = 0.005;
        const size_t nx = ((xmax - xmin) / dx) + 1;

        Vector xx(nx);
        generate(xx.begin(), xx.end(), Increase_by<double>(xmin, dx));

        const double zmin = adapter.get(&mxsc, 1) - 2.0;
        const double zmax = adapter.get(&mxsc, 1) + 2.0;
        const double dz = 0.01;
        const size_t nz = ((zmax - zmin) / dz) + 1;

        Vector zz(nz);
        generate(zz.begin(), zz.end(), Increase_by<double>(zmin, dz));

        Matrix fxz(nx, nz);
        for(size_t i = 0; i < nx; i++)
        {
            adapter.set(&mxsc, 0, xx[i]);
            for(size_t j = 0; j < nz; j++)
            {
                adapter.set(&mxsc, 1, zz[j]);
                fxz(i, j) = posterior(mxsc);
            }
        }

        // write "plot"
        xx.write((outdir + "/x.txt").c_str());
        zz.write((outdir + "/z.txt").c_str());
        fxz.write((outdir + "/fxz.txt").c_str());

        cout << "Done. Check " << outdir << " for output..." << endl;
    }
    catch(const kjb::Exception& ex)
    {
        ex.print_details();
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

