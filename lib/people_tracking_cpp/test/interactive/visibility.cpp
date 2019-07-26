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

/* $Id: visibility.cpp 19002 2015-05-05 14:23:57Z jguan1 $ */

#include <people_tracking_cpp/pt_scene.h>
#include <people_tracking_cpp/pt_association.h>
#include <people_tracking_cpp/pt_target.h>
#include <people_tracking_cpp/pt_complete_trajectory.h>
#include <people_tracking_cpp/pt_complete_state.h>
#include <people_tracking_cpp/pt_box_trajectory.h>
#include <people_tracking_cpp/pt_visibility.h>
#include <people_tracking_cpp/pt_data.h>
#include <people_tracking_cpp/pt_detection_box.h>
#include <flow_cpp/flow_integral_flow.h>
#include <st_cpp/st_perspective_camera.h>
#include <gr_cpp/gr_opengl.h>
#include <gr_cpp/gr_glut.h>
#include <gr_cpp/gr_opengl_headers.h>
#include <detector_cpp/d_bbox.h>
#include "utils.h"

using namespace std;
using namespace kjb;
using namespace kjb::pt;
using namespace kjb::opengl;

double win_width = 500;
double win_height = 500;

Scene scene(Ascn(), Perspective_camera(), 0.0, 0.0, 0.0);

const size_t num_trajs = 3;
Vector traj_colors[num_trajs] = { Vector().set(1.0, 0.0, 0.0),
                                  Vector().set(0.0, 1.0, 0.0),
                                  Vector().set(0.0, 0.0, 1.0) };

bool depth_mode;
Ascn::const_iterator cur_target_p;

/** @brief  Display scene. */
void display();

/** @brief  Reshape scene. */
void reshape(int, int);

/** @brief  Handle key input. */
void handle_key(unsigned char, int, int);

/** @brief  Main -- all the magic happens here. */
int main(int argc, char** argv)
{
    try
    {
        size_t num_frames = 1;
        Box_data data(1, 1);
        Facemark_data fm_data;
        vector<Integral_flow> flows_x;
        vector<Integral_flow> flows_y;
        Scene scene(Ascn(data), Perspective_camera(), 0.0, 0.0, 0.0);
        create_or_read_scene(argc, argv, num_frames, win_width, win_height,
                             data, fm_data, flows_x, flows_y, scene, num_trajs);

        // start in depth mode
        depth_mode = true;

#ifdef KJB_HAVE_GLUT
        // GL stuff
        Glut_window win(win_width, win_height);
        win.set_display_callback(display);
        win.set_reshape_callback(reshape);
        win.set_keyboard_callback(handle_key);
#endif
        glEnable(GL_DEPTH_TEST);
#ifdef KJB_HAVE_GLUT
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

void display()
{
    // clear stuff
    glClearColor(0.5, 0.5, 0.5, 0.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();

    size_t tr_i = 0;
    BOOST_FOREACH(const Target& target, scene.association)
    {
        const Trajectory& traj = target.trajectory();
        const Body_2d_trajectory& btraj = target.body_trajectory();
        const Face_2d_trajectory& ftraj = target.face_trajectory();
        //const Perspective_camera& cam = scene.camera;

        const Bbox& bbox = btraj[0]->value.body_bbox;
        const Bbox& fbox = ftraj[0]->value.bbox;
        double z = traj[0]->value.position[2];

        opengl::glColor(traj_colors[tr_i]);

        if(depth_mode)
        {
            glBegin(GL_QUADS);
                // draw body box
                glVertex3d(bbox.get_left(), bbox.get_top(), z);
                glVertex3d(bbox.get_right(), bbox.get_top(), z);
                glVertex3d(bbox.get_right(), bbox.get_bottom(), z);
                glVertex3d(bbox.get_left(), bbox.get_bottom(), z);

                // draw face box
                glVertex3d(fbox.get_left(), fbox.get_top(), z);
                glVertex3d(fbox.get_right(), fbox.get_top(), z);
                glVertex3d(fbox.get_right(), fbox.get_bottom(), z);
                glVertex3d(fbox.get_left(), fbox.get_bottom(), z);
            glEnd();

            if(&target == &(*cur_target_p))
            {
                glColor3d(1.0, 1.0, 1.0);
                glLineWidth(1.0);

                // draw body box outline
                glBegin(GL_LINE_LOOP);
                    glVertex3d(bbox.get_left(), bbox.get_top(), z + 0.001);
                    glVertex3d(bbox.get_right(), bbox.get_top(), z + 0.001);
                    glVertex3d(bbox.get_right(), bbox.get_bottom(), z + 0.001);
                    glVertex3d(bbox.get_left(), bbox.get_bottom(), z + 0.001);
                glEnd();

                // draw face box outline
                glBegin(GL_LINE_LOOP);
                    glVertex3d(fbox.get_left(), fbox.get_top(), z + 0.001);
                    glVertex3d(fbox.get_right(), fbox.get_top(), z + 0.001);
                    glVertex3d(fbox.get_right(), fbox.get_bottom(), z + 0.001);
                    glVertex3d(fbox.get_left(), fbox.get_bottom(), z + 0.001);
                glEnd();
            }
        }
        else
        {
            if(&target == &(*cur_target_p))
            {
                // get visibility info
                const Visibility& bvis = btraj[0]->value.visibility;
                const Visibility& fvis = ftraj[0]->value.visibility;

                const vector<Vector>& bcells = bvis.visible_cells;
                const vector<Vector>& fcells = fvis.visible_cells;

                const double bx_del = bvis.cell_width;
                const double by_del = bvis.cell_height;
                const double fx_del = fvis.cell_width;
                const double fy_del = fvis.cell_height;

                // render body cells
                glBegin(GL_QUADS);
                for(size_t i = 0; i < bcells.size(); i++)
                {
                    double cx = bcells[i][0];
                    double cy = bcells[i][1];
                    glVertex3d(cx - bx_del/2.0, cy + by_del/2.0, z);
                    glVertex3d(cx + bx_del/2.0, cy + by_del/2.0, z);
                    glVertex3d(cx + bx_del/2.0, cy - by_del/2.0, z);
                    glVertex3d(cx - bx_del/2.0, cy - by_del/2.0, z);
                }
                glEnd();

                // render body cells
                glBegin(GL_QUADS);
                for(size_t i = 0; i < fcells.size(); i++)
                {
                    double cx = fcells[i][0];
                    double cy = fcells[i][1];
                    glVertex3d(cx - fx_del/2.0, cy + fy_del/2.0, z);
                    glVertex3d(cx + fx_del/2.0, cy + fy_del/2.0, z);
                    glVertex3d(cx + fx_del/2.0, cy - fy_del/2.0, z);
                    glVertex3d(cx - fx_del/2.0, cy - fy_del/2.0, z);
                }
                glEnd();
            }
            else
            {
                glLineWidth(2.0);
                glBegin(GL_LINE_LOOP);
                    glVertex3d(bbox.get_left(), bbox.get_top(), z);
                    glVertex3d(bbox.get_right(), bbox.get_top(), z);
                    glVertex3d(bbox.get_right(), bbox.get_bottom(), z);
                    glVertex3d(bbox.get_left(), bbox.get_bottom(), z);
                glEnd();

                glBegin(GL_LINE_LOOP);
                    glVertex3d(fbox.get_left(), fbox.get_top(), z);
                    glVertex3d(fbox.get_right(), fbox.get_top(), z);
                    glVertex3d(fbox.get_right(), fbox.get_bottom(), z);
                    glVertex3d(fbox.get_left(), fbox.get_bottom(), z);
                glEnd();
            }
        }

        tr_i++;
    }

#ifdef KJB_HAVE_GLUT
    glutSwapBuffers();
#endif
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

void reshape(int w, int h)
{
    glViewport(0, 0, (GLsizei)w, (GLsizei)h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(-win_width / 2.0, win_width / 2.0,
            -win_height / 2.0, win_height / 2.0,
            -1.0, 10.0);
    glMatrixMode(GL_MODELVIEW);
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

void handle_key(unsigned char key, int, int)
{
    Trajectory& traj = cur_target_p->trajectory();

    switch(key)
    {
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
            traj[0]->value.position[0] -= 0.05;
            break;
        }

        case 'X':
        {
            traj[0]->value.position[0] += 0.05;
            break;
        }

        case 'z':
        {
            traj[0]->value.position[2] -= 0.05;
            break;
        }

        case 'Z':
        {
            traj[0]->value.position[2] += 0.05;
            break;
        }

        case 'n':
        {
            cur_target_p++;
            if(cur_target_p == scene.association.end())
            {
                cur_target_p = scene.association.begin();
            }

            break;
        }

        case 'm':
        case 'M':
        {
            depth_mode = !depth_mode;
            break;
        }

        case 'q':
        case 'Q':
        {
            exit(EXIT_SUCCESS);
        }
    }

    const Perspective_camera& cam = scene.camera;
    cur_target_p->update_boxes(cam);
    cur_target_p->update_faces(cam);

    update_visibilities(scene, 1, true);
#ifdef KJB_HAVE_GLUT
    glutPostRedisplay();
#endif
}

