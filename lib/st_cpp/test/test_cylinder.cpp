/* $Id$ */
/* ===========================================================================*
 |
 |  Copyright (c) 1994-2010 by Kobus Barnard (author)
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
 |  Author:  Luca Del Pero
 * ===========================================================================*/

// This file tests the function draw_cylinder() (located in st_draw_shape.cpp)
// by calling the function with the parameters that are manually specified in 
// the code below.

#include <st_cpp/st_draw_shape.h>

#ifdef KJB_HAVE_GLUT
#ifdef MAC_OSX
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#endif
#endif

using namespace kjb;

void init_onscreen_buffer(int argc, char **argv);

float gwidth = 600;
float gheight = 400;
kjb::Perspective_camera camera;

int main(int argc, char **argv)
{
#ifdef KJB_HAVE_GLUT
    init_onscreen_buffer(argc, argv);
    camera.set_camera_centre_z(400);
    camera.set_camera_centre_y(0);
    if (kjb_c::is_interactive())
    {
        glutMainLoop();
    }
    return 0;
#else
    std::cout << "NO GLUT AVAILABLE!!!!" << std::endl;
    return 1;
#endif
}


#ifdef KJB_HAVE_GLUT
static void timer_glut(int ignored)
{
    glutPostRedisplay();
    glutTimerFunc(10, timer_glut, 0);
}

static void display_glut()
{
    unsigned t0=clock();
    glDrawBuffer(GL_BACK);
    glReadBuffer(GL_BACK);

    glClearColor(0, 0, 0, 0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Defines the radius and the two center points of the cylinder that
    // will be drawn.
    kjb::Vector top(3,0.0);
    kjb::Vector bottom(3,0.0);
    top(0) = 40.0;
    bottom(0) = 0.0;

    top(1) = 40.0;
    bottom(1) = 0.0;

    top(2) = 0.0;
    bottom(2) = 0.0;

    double radius = 20;
    glColor3f(1.0,0.0,0.0);
    GLUquadricObj* myQuadric = 0;
    myQuadric = gluNewQuadric();
    kjb::draw_cylinder(camera, myQuadric, top, bottom, radius);

// draw purple sphere
glColor3f(64,0,64);
GLUquadricObj* quadric = 0;
quadric = gluNewQuadric();
gluSphere(quadric, 1.0, 32, 32);

    glFlush();

    glutSwapBuffers();

}

static void keyboard_glut(unsigned char key, int a, int b)
{

}

static void reshape_glut(int w, int h)
{
    float s = (float)h / (float)gheight;
    glPointSize(s+0.49999);
    glLineWidth(s);

    glClearColor(0, 0, 0, 0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    gwidth = w;
    gheight = h;
    glViewport(0, 0, gwidth, gheight);

}

void init_onscreen_buffer(int argc, char **argv)
{
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH | GLUT_ACCUM
                                        | GLUT_ALPHA | GLUT_STENCIL);
    glutInitWindowSize(gwidth, gheight);
    glutCreateWindow("Cylinder");
    glutDisplayFunc(display_glut);
    glutReshapeFunc(reshape_glut);
    glutKeyboardFunc(keyboard_glut);
    glutTimerFunc(10, timer_glut, 0);

    kjb::opengl::default_init_opengl(gwidth, gheight);

}
#endif

