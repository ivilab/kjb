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
#include <people_tracking_cpp/pt_util.h>
#include <camera_cpp/perspective_camera.h>
#include <m_cpp/m_vector.h>
#include <l_cpp/l_filesystem.h>
#include <vector>
#include <iostream>
#include <algorithm>

using namespace std;
using namespace kjb;
using namespace kjb::pt;

/** @brief  Main -- all the magic happens here. */
int main(int argc, char** argv)
{
    if(argc != 3)
    {
        cout << "gaze_intersections DATA-PATH SCENE-PATH\n";
        return EXIT_FAILURE;
    }

    string data_dp = argv[1];
    string scene_dp = argv[2];

    try
    {
        // read data (image width and height don't matter here)
        Box_data data(1920, 1080, 0.98);
        data.read(file_names_from_format(data_dp + "/%05d.txt"));

        // read scene
        Scene scene(Ascn(data), Perspective_camera(), 0.0, 0.0, 0.0);
        read_scene(scene, scene_dp);

        // compute points
        vector<Vector> pts = locations_3d(scene, 0.5);

        std::vector<double> look_ct(pts.size(), 0);
        for(size_t i = 0; i < pts.size(); ++i)
        {
            // how many people are looking at it
            BOOST_FOREACH(const Target& tg, scene.association)
            {
                size_t sf = tg.get_start_time();
                size_t ef = tg.get_end_time();
                for(size_t t = sf; t <= ef; ++t)
                {
                    if(tg.trajectory()[t - 1]->value.attn == 0)
                    {
                        if(looking(
                            tg.trajectory()[t - 1]->value,
                            tg.trajectory().height,
                            tg.trajectory().width,
                            tg.trajectory().girth,
                            pts[i], 0.4, true))
                        {
                            look_ct[i] = look_ct[i] + 1.0;
                        }
                    }
                }
            }

            //if(look_ct[i] < 10.0) look_ct[i] = 0.0;
        }
        //copy(pts.begin(), pts.end(), ostream_iterator<Vector>(cout, "\n"));
        for(size_t i = 0; i < pts.size(); i++)
        {
            std::cout << pts[i] << " " << look_ct[i] << std::endl;
        }
    }
    catch(const kjb::Exception& ex)
    {
        ex.print_details();
        cerr << endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

