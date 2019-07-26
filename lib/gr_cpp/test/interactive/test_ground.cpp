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

#include <st_cpp/st_perspective_camera.h>
#include <gr_cpp/gr_opengl.h>
#include <gr_cpp/gr_glut.h>
#include <gr_cpp/gr_primitive.h>
#include <iostream>

using namespace std;
using namespace kjb;
using namespace kjb::opengl;

const double win_width = 768;
const double win_height = 576;

Perspective_camera camera(1.0, 100.0);
double rad = 1.0/8;

/** @brief  Display scene. */
void display();

/** @brief  Reshape scene. */
void reshape(int, int);

/** @brief  Main -- all the magic happens here. */
int main(int argc, char** argv)
{
    try
    {
        // camera
        camera.set_camera_centre(Vector(0.0, 4.0, 0.0));
        camera.set_focal_length(1.7*win_width);
        camera.set_pitch(M_PI/4.0);

        // GL stuff
        Glut_window win(win_width, win_height);

        win.set_display_callback(display);
        win.set_reshape_callback(reshape);

        glutMainLoop();
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
    glClearColor(0.0, 0.0, 0.0, 0.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // set camera
    camera.prepare_for_rendering(false);

    // draw ground
    glColor3d(1.0, 1.0, 1.0);
    render_xz_plane_grid(-10.0, 10.0, -50.0, 0.0);

    // draw ball
    glLineWidth(1.0);

    glColor3d(0.0, 0.0, 1.0);
    Sphere sph1(rad);
    glPushMatrix();
        glTranslated(0.6, rad, -4.75);
        sph1.wire_render();
    glPopMatrix();

    glColor3d(1.0, 0.0, 0.0);
    Sphere sph2(rad);
    glPushMatrix();
        glTranslated(-0.9, rad, -4.5);
        sph2.wire_render();
    glPopMatrix();

    glColor3d(1.0, 0.0, 0.0);
    Sphere sph3(rad);
    glPushMatrix();
        glTranslated(0.5, rad, -3.75);
        sph3.wire_render();
    glPopMatrix();

    glColor3d(1.0, 0.0, 0.0);
    Sphere sph4(rad);
    glPushMatrix();
        glTranslated(-1.0, rad, -3.5);
        sph4.wire_render();
    glPopMatrix();

    glutSwapBuffers();
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

void reshape(int w, int h)
{
    glViewport(0, 0, (GLsizei)w, (GLsizei)h);
    glMatrixMode(GL_MODELVIEW);
}

