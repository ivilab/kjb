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

/* $Id: state.cpp 19236 2015-05-31 02:09:55Z ernesto $ */

#include <people_tracking_cpp/pt_scene.h>
#include <people_tracking_cpp/pt_association.h>
#include <people_tracking_cpp/pt_scene_viewer.h>
#include <detector_cpp/d_bbox.h>
#include <camera_cpp/perspective_camera.h>
#include <l_cpp/l_iterator.h>
#include <m_cpp/m_vector.h>
#include <m_cpp/m_vector_d.h>
#include <gr_cpp/gr_opengl.h>
#ifdef KJB_HAVE_GLUT
#include <gr_cpp/gr_glut.h>
#endif
#include <iostream>
#include <boost/bind.hpp>
#include <boost/ref.hpp>

using namespace std;
using namespace kjb;
using namespace kjb::pt;
using namespace kjb::opengl;
using namespace boost;

const_circular_iterator<Ascn::const_iterator> active_target_p;

/** @brief  Handle key input. */
void handle_key(Scene_viewer&, unsigned char, int, int);

/** @brief  Main -- all the magic happens here. */
int main(int argc, char** argv)
{
    try
    {
        double win_width = 800;
        double win_height = 600;

        // create fake boxes and scene
        Detection_box dbox1(Bbox(), 0.0, "fake");
        Detection_box dbox2(Bbox(Vector(2, 1.0)), 0.0, "fake");
        Scene scene(Ascn(), Perspective_camera(0.1, 10000), 1, 1, 1);

        // set up camera
        scene.camera.set_camera_centre(Vector(0.0, 2.0, 0.0));
        scene.camera.set_focal_length(3*win_width / 2.0);
        scene.camera.set_pitch(M_PI/15.0);

        // set up first target
        Target tg(1.75, 0.4, 0.3, 2);
        tg.insert(make_pair(1, &dbox1));
        tg.trajectory().front() = Trajectory_element();
        tg.trajectory().front()->value.position = Vector3(-1.0, 0.0, -5.0);
        tg.trajectory().front()->value.body_dir = 0.0;
        tg.trajectory().front()->value.face_dir = Vector2(0.0, 0.0);
        scene.association.insert(tg);

        // set up second target
        tg.clear();
        tg.insert(make_pair(1, &dbox2));
        tg.trajectory().front()->value.position = Vector3(1.0, 0.0, -5.0);
        scene.association.insert(tg);

        // set state
        active_target_p = make_const_circular_iterator(scene.association);
        find_looking_subjects(scene);

#ifdef KJB_HAVE_GLUT
        // set up viewer
        Vector red(1.0, 0.0, 0.0);
        Vector blue(0.0, 0.0, 1.0);

        Scene_viewer viewer(scene, win_width, win_height);
        viewer.set_target_style(*scene.association.begin(), red, 1.0);
        viewer.set_target_style(*scene.association.rbegin(), blue, 1.0);
        viewer.set_key_callback(bind(handle_key, ref(viewer), _1, _2, _3));

        glutMainLoop();
#endif
    }
    catch(const kjb::Exception& ex)
    {
        ex.print_details();
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

void handle_key(Scene_viewer& viewer, unsigned char key, int, int)
{
    Trajectory& traj = active_target_p->trajectory();
    Complete_state& cs = traj.front()->value;
    switch(key)
    {
        case 'n': ++active_target_p;
        break;

        case 'h':
        {
            traj.height -= 0.05;
            if(traj.height <= 0.0)
            {
                traj.height = 0.05;
            }

            break;
        }

        case 'H':
        {
            traj.height += 0.05;
            break;
        }

        case 'w':
        {
            traj.width -= 0.05;
            if(traj.width <= 0.0)
            {
                traj.width = 0.05;
            }

            break;
        }

        case 'W':
        {
            traj.width += 0.05;
            break;
        }

        case 'g':
        {
            traj.girth -= 0.05;
            if(traj.girth <= 0.0)
            {
                traj.girth = 0.05;
            }

            break;
        }

        case 'G':
        {
            traj.girth += 0.05;
            break;
        }

        case 'x':
        {
            cs.position[0] -= 0.05;
            break;
        }

        case 'X':
        {
            cs.position[0] += 0.05;
            break;
        }

        case 'z':
        {
            cs.position[2] -= 0.05;
            break;
        }

        case 'Z':
        {
            cs.position[2] += 0.05;
            break;
        }

        case 'r':
        {
            cs.body_dir -= M_PI / 48.0;
            if(cs.body_dir < -M_PI) cs.body_dir += 2*M_PI;
            break;
        }

        case 'R':
        {
            cs.body_dir += M_PI / 48.0;
            if(cs.body_dir > M_PI) cs.body_dir -= 2*M_PI;
            break;
        }

        case 't':
        {
            cs.face_dir[0] -= M_PI / 48.0;
            if(cs.face_dir[0] < -M_PI) cs.face_dir[0] += 2*M_PI;
            break;
        }

        case 'T':
        {
            cs.face_dir[0] += M_PI / 48.0;
            if(cs.face_dir[0] > M_PI) cs.face_dir[0] -= 2*M_PI;
            break;
        }

        case 'e':
        {
            cs.face_dir[1] -= M_PI / 48.0;
            if(cs.face_dir[1] < -M_PI/2) cs.face_dir[1] = -M_PI/2;
            break;
        }

        case 'E':
        {
            cs.face_dir[1] += M_PI / 48.0;
            if(cs.face_dir[1] > M_PI/2) cs.face_dir[1] = M_PI/2;
            break;
        }

        case 'q':
        case 'Q':
        {
            exit(EXIT_SUCCESS);
        }

        default:;
    }

    find_looking_subjects(viewer.scene());

#ifdef KJB_HAVE_GLUT
    glutPostRedisplay();
#endif
}

