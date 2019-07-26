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
#include <people_tracking_cpp/pt_target.h>
#include <people_tracking_cpp/pt_complete_trajectory.h>
#include <people_tracking_cpp/pt_complete_state.h>
#include <people_tracking_cpp/pt_association.h>
#include <m_cpp/m_vector.h>
#include <st_cpp/st_perspective_camera.h>
#include <l_cpp/l_functors.h>
#include <l_cpp/l_test.h>
#include <l/l_sys_io.h>
#include <iostream>
#include <vector>
#include <algorithm>
#include <string>
#include <boost/bind.hpp>
#include <boost/format.hpp>

#include "utils.h"

using namespace std;
using namespace kjb;
using namespace kjb::pt;

bool VERBOSE = true;
const double eps = 1e-6;

/** @brief  Compare two scenes for equality. */
bool compare_scenes(const Scene& sc1, const Scene& sc2);

/** @brief  Main -- all the magic happens here. */
int main(int argc, char** argv)
{
    const double img_width = 500;
    const double img_height = 500;
    const size_t N = 10;
    const size_t min_frames = 100;
    const size_t num_trajs = 2;
    string outdir = "output/write_scene_iterator_cpp/";

    try
    {
        // CREATE SCENES
        vector<Scene> scenes;
        vector<Box_data> datas(N, Box_data(img_width, img_height));
        vector<Facemark_data> fm_datas(N);
        vector<vector<Integral_flow> > iflows_xs(N);
        vector<vector<Integral_flow> > iflows_ys(N);
        for(size_t i = 0; i < N; i++)
        {
            Scene scene(Ascn(datas[i]), Perspective_camera(), 0.0, 0.0, 0.0);
            fm_datas[i].resize(min_frames);
            iflows_xs[i].reserve(min_frames);
            iflows_ys[i].reserve(min_frames);
            make_typical_scene(min_frames, scene, datas[i], fm_datas[i], 
                               iflows_xs[i], iflows_ys[i], num_trajs);
            scenes.push_back(scene);
        }
        

        // WRITE USING OUTPUT ITERATOR
        kjb_c::kjb_mkdir(outdir.c_str());
        copy(scenes.begin(), scenes.end(), Write_scene_iterator(outdir));

        // READ SCENES
        for(size_t i = 0; i < N; i++)
        {
            const Box_data& data = datas[i];
            Scene sc(Ascn(data), Perspective_camera(), 0.0, 0.0, 0.0);
            string curdir = outdir + boost::str(boost::format("/%04d") % (i+1));

            read_scene(sc, curdir);
            TEST_TRUE(compare_scenes(sc, scenes[i]));
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

bool compare_scenes(const Scene& sc1, const Scene& sc2)
{
    // compare cameras
    const Perspective_camera& cam1 = sc1.camera;
    const Perspective_camera& cam2 = sc2.camera;
    if(cam1.get_near() != cam2.get_near()
        || cam1.get_far() != cam2.get_far()
        || cam1.get_camera_centre() != cam2.get_camera_centre()
        || cam1.get_rotation_angles() != cam2.get_rotation_angles()
        || cam1.get_focal_length() != cam2.get_focal_length()
        || cam1.get_principal_point() != cam2.get_principal_point()
        || cam1.get_skew() != cam2.get_skew()
        || cam1.get_aspect_ratio() != cam2.get_aspect_ratio())
    {
        if(VERBOSE)
        {
            cout << "Failed comparing cameras"
                 << cam1.get_near() << " VS " << cam2.get_near() << endl
                 << cam1.get_far() << " VS " << cam2.get_far() << endl
                 << cam1.get_camera_centre() << " VS "
                    << cam2.get_camera_centre() << endl
                 << cam1.get_rotation_angles() << " VS "
                    << cam2.get_rotation_angles() << endl
                 << cam1.get_focal_length() << " VS "
                    << cam2.get_focal_length() << endl
                 << cam1.get_principal_point() << " VS "
                    << cam2.get_principal_point() << endl
                 << cam1.get_skew() << " VS " <<  cam2.get_skew() << endl
                 << cam1.get_aspect_ratio() << " VS "
                    << cam2.get_aspect_ratio() << endl;
        }

        return false;
    }

    // compare params
    if(sc1.kappa != sc2.kappa
            || sc1.theta != sc2.theta
            || sc1.lambda != sc2.lambda)
    {
        return false;
    }

    // compare associations
    const Ascn& as1 = sc1.association;
    const Ascn& as2 = sc2.association;
    if(&as1.get_data() != &as2.get_data())
    {
        return false;
    }

    if(as1.size() != as2.size())
    {
        return false;
    }

    Ascn::const_iterator tg_p1 = as1.begin();
    Ascn::const_iterator tg_p2 = as2.begin();
    for(; tg_p1 != as1.end(); tg_p1++, tg_p2++)
    {
        // compare targets
        const Target& tg1 = *tg_p1;
        const Target& tg2 = *tg_p2;
        if(tg1.size() != tg2.size())
        {
            return false;
        }

        // compare trajectories
        const Trajectory& tj1 = tg1.trajectory();
        const Trajectory& tj2 = tg2.trajectory();
        if(tj1.size() != tj2.size())
        {
            return false;
        }

        for(size_t i = 0; i < tj1.size(); i++)
        {
            if(tj1[i])
            {
                if(!tj2[i])
                {
                    return false;
                }

                const Complete_state& cs1 = tj1[i]->value;
                const Complete_state& cs2 = tj2[i]->value;
                if(vector_distance(cs1.position, cs2.position) > eps
                    || fabs(cs1.body_dir - cs2.body_dir) > eps
                    || vector_distance(cs1.face_dir, cs2.face_dir) > eps)
                {
                    if(VERBOSE)
                    {
                        cout << "Failed comparing " << i << "th trajectory "
                             << "element:\n"
                             << cs1.position << " VS " << cs2.position << endl
                             << cs1.body_dir << " VS " << cs2.body_dir << endl
                             << cs1.face_dir << " VS " << cs2.face_dir << endl;
                    }

                    return false;
                }
            }
            else
            {
                if(tj2[i])
                {
                    return false;
                }
            }
        }
    }

    return true;
}

