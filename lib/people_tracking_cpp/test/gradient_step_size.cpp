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
#include <people_tracking_cpp/pt_scene_diff.h>
#include <detector_cpp/d_bbox.h>
#include <m_cpp/m_vector.h>
#include <l_cpp/l_test.h>

#include "utils.h"

using namespace std;
using namespace kjb;
using namespace kjb::pt;

bool VERBOSE = true;

/** @brief  Main -- all the magic happens here. */
int main(int argc, char** argv)
{
    double img_width = 500;
    double img_height = 500;
    size_t num_frames = 100;

    try
    {
        // DATA 
        Box_data data(img_width, img_height);
        vector<Integral_flow> flows_x;
        vector<Integral_flow> flows_y;
        Facemark_data fm_data(num_frames);

        // CREATE SCENE
        Scene scene(Ascn(data), Perspective_camera(), 0.0, 0.0, 0.0);
        create_or_read_scene(argc, argv, num_frames, img_width, img_height,
                             data, fm_data, flows_x, flows_y, scene);

        // TEST STEP SIZE
        vector<double> ss = trajectory_gradient_step_sizes(scene);
        double sdu = 0.0;
        double sdv = 0.0;
        size_t i = 0;
        size_t dir_dims = 0;
        BOOST_FOREACH(const Target& target, scene.association)
        {
            int sf = target.get_start_time();
            int ef = target.get_end_time();
            assert(sf != -1 && ef != -1);
            for(size_t t = (size_t)sf; t <= (size_t)ef; t++)
            {
                for(size_t d = 0; d < 2; d++, i++)
                {
                    size_t dd = (d == 0 ? 0 : 2);
                    double dx = ss[i];
                    Vector3 dxx(0.0, 0.0, 0.0);

                    // box before moving
                    Bbox box1 = target.body_trajectory()[t - 1]->value.full_bbox;

                    // move by dx
                    dxx[dd] = dx;
                    move_trajectory_at_frame(scene, target, t, dxx, false);

                    // box after moving
                    Bbox box2 = target.body_trajectory()[t - 1]->value.full_bbox;

                    double du = fabs(box1.get_bottom_center()[0]
                                        - box2.get_bottom_center()[0]);
                    double dv = fabs(box1.get_bottom_center()[1]
                                        - box2.get_bottom_center()[1]);

                    if(VERBOSE)
                    {
                        cout << "Frame " << t << "("
                             << (dd == 0 ? "x" : "z") << ")" << endl;
                        cout << "  du = " << du << endl;
                        cout << "  dv = " << dv << endl << endl;
                    }

                    if(dd == 0)
                    {
                        sdu += du;
                        //TEST_TRUE(du >= 0.4 && du <= 1.6);
                        TEST_TRUE(du >= 0.4 && du <= 2.0);
                    }
                    else
                    {
                        sdv += dv;
                        //TEST_TRUE(dv >= 0.4 && dv <= 1.6);
                        TEST_TRUE(dv >= 0.4 && dv <= 2.0);
                    }

                    // move back
                    dxx[dd] = -2*dx;
                    move_trajectory_at_frame(scene, target, t, dxx, false);
                }

                // skip the three direction step sizes
                i += 3;
                dir_dims += 3;
            }
        }

        size_t ii = i - dir_dims;
        if(VERBOSE)
        {
            cout << "Average du: " << (2*sdu/ii) << endl;
            cout << "Average dv: " << (2*sdv/ii) << endl;
        }

        TEST_TRUE(2*sdu/ii >= 0.75 && 2*sdu/ii <= 1.25);
        TEST_TRUE(2*sdv/ii >= 0.75 && 2*sdv/ii <= 1.25);
    }
    catch(const kjb::Exception& ex)
    {
        ex.print_details();
        cerr << endl;
        return EXIT_FAILURE;
    }

    RETURN_VICTORIOUSLY();
}

