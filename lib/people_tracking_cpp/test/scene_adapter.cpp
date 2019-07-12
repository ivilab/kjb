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

/* $Id: scene_adapter.cpp 18699 2015-03-23 15:13:14Z ernesto $ */

#include <people_tracking_cpp/pt_scene.h>
#include <people_tracking_cpp/pt_data.h>
#include <people_tracking_cpp/pt_scene_adapter.h>
#include <m_cpp/m_vector.h>
#include <l_cpp/l_test.h>
#include "utils.h"

using namespace std;
using namespace kjb;
using namespace kjb::pt;

bool VERBOSE = false;

/** @brief  Main -- all the magic happens here. */
int main(int argc, char** argv)
{
    try
    {
        // CREATE SCENE
        size_t num_frames = 40;
        double img_width = 500;
        double img_height = 500;

        Box_data data(img_width, img_height, 0.99);
        vector<Integral_flow> flows_x;
        vector<Integral_flow> flows_y;
        Facemark_data fm_data(num_frames);

        Scene scene(Ascn(data), Perspective_camera(), 0.0, 0.0, 0.0);
        create_or_read_scene(
            argc, argv, num_frames, img_width, img_height,
            data, fm_data, flows_x, flows_y, scene, 3);

        // TEST ADAPTER
        Scene_adapter adapter;
        size_t scene_sz = adapter.size(&scene);

        if(VERBOSE) std::cout << "scene size: " << scene_sz << std::endl;

        // test set and get
        for(size_t i = 0; i < scene_sz; i++)
        {
            double x = sample(Uniform_distribution(-2, 2));
            adapter.set(&scene, i, x);
            double y = adapter.get(&scene, i);

            TEST_TRUE(fabs(x - y) < DBL_EPSILON);
        }

        // more random tests
        adapter.set(&scene, 0, 10.0);
        adapter.set(&scene, 1, 20.0);
        adapter.set(&scene, 2, 0.3);
        adapter.set(&scene, 3, 0.4);
        adapter.set(&scene, 4, 0.5);
        size_t t = scene.association.begin()->get_start_time() - 1;
        const Vector3& pos
                = scene.association.begin()->trajectory()[t]->value.position;
        double bd = scene.association.begin()->trajectory()[t]->value.body_dir;
        const Vector2& fd
                = scene.association.begin()->trajectory()[t]->value.face_dir;

        if(VERBOSE)
        {
            std::cout << "pos = " << pos << std::endl;
            std::cout << "bd = " << bd << std::endl;
            std::cout << "fd = " << fd << std::endl;
        }

        TEST_TRUE(pos == Vector3(10.0, 0.0, 20.0));
        TEST_TRUE(bd == 0.3);
        TEST_TRUE(fd == Vector2(0.4, 0.5));

        Scene scene2 = scene;
        adapter.set(&scene2, 0, 20.0);
        adapter.set(&scene2, 1, 30.0);
        adapter.set(&scene2, 2, 0.4);
        adapter.set(&scene2, 3, 0.5);
        adapter.set(&scene2, 4, 0.6);
        t = scene2.association.begin()->get_start_time() - 1;
        const Vector3& pos2
                = scene2.association.begin()->trajectory()[t]->value.position;
        double bd2 = scene2.association.begin()->trajectory()[t]->value.body_dir;
        const Vector2& fd2
                = scene2.association.begin()->trajectory()[t]->value.face_dir;

        if(VERBOSE)
        {
            std::cout << "pos2 = " << pos2 << std::endl;
            std::cout << "bd2 = " << bd2 << std::endl;
            std::cout << "fd2 = " << fd2 << std::endl;
        }

        TEST_TRUE(pos2 == Vector3(20.0, 0.0, 30.0));
        TEST_TRUE(bd2 == 0.4);
        TEST_TRUE(fd2 == Vector2(0.5, 0.6));

        adapter.set(&scene2, scene_sz - 5, -10.0);
        adapter.set(&scene2, scene_sz - 4, -20.0);
        adapter.set(&scene2, scene_sz - 3, -0.3);
        adapter.set(&scene2, scene_sz - 2, -0.4);
        adapter.set(&scene2, scene_sz - 1, -0.5);
        t = scene2.association.rbegin()->get_end_time() - 1;
        const Vector3& pos3
            = scene2.association.rbegin()->trajectory()[t]->value.position;
        double bd3
            = scene2.association.rbegin()->trajectory()[t]->value.body_dir;
        const Vector2& fd3
            = scene2.association.rbegin()->trajectory()[t]->value.face_dir;

        if(VERBOSE)
        {
            std::cout << "pos3 = " << pos3 << std::endl;
            std::cout << "bd3 = " << bd3 << std::endl;
            std::cout << "fd3 = " << fd3 << std::endl;
        }

        TEST_TRUE(pos3 == Vector3(-10.0, 0.0, -20.0));
        TEST_TRUE(bd3 == -0.3);
        TEST_TRUE(fd3 == Vector2(-0.4, -0.5));

        // test double-set
        for(size_t i = 0; i < scene_sz; i++)
        {
            for(size_t j = 0; j < scene_sz; j++)
            {
                if(i == j) continue;

                double x1 = sample(Uniform_distribution(-1, 1));
                double x2 = sample(Uniform_distribution(-1, 1));
                adapter.set(&scene, i, j, x1, x2);
                double y1 = adapter.get(&scene, i);
                double y2 = adapter.get(&scene, j);

                if(VERBOSE)
                {
                    cout << "At (" << i << ", " << j << ")\n";
                    cout << "x1 = " << x1 << endl;
                    cout << "y1 = " << y1 << endl;
                    cout << "x2 = " << x2 << endl;
                    cout << "y2 = " << y2 << endl << endl;
                }

                TEST_TRUE(fabs(x1 - y1) < DBL_EPSILON);
                TEST_TRUE(fabs(x2 - y2) < DBL_EPSILON);
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

