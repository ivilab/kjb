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

#include <st_cpp/st_draw_shape.h>
#include <glut_cpp/glut_perspective_camera.h>

#ifdef KJB_HAVE_GLUT
#ifdef MAC_OSX
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#endif
#endif

#define NONE 0
#define CAMERA_SELECTED 1
#define POLYMESH_SELECTED 2

using namespace kjb;

void init_onscreen_buffer(int argc, char **argv);

float gwidth = 600;
float gheight = 400;
kjb::Perspective_camera camera;
static unsigned int object_selected = NONE;


#ifdef KJB_HAVE_GLUT
static void timer_glut(int ignored)
{
    glutPostRedisplay();
    glutTimerFunc(10, timer_glut, 0);
}

static void camera_callback(int i)
{
    object_selected = CAMERA_SELECTED;
}

static void main_menu_glut(int id)
{
    using namespace kjb;
    /*_current_action = id;
    if(id == EXIT_ID)
    {
        delete pp;
        delete camera;
        exit(EXIT_SUCCESS);
    }*/
}

static void display_glut()
{
    unsigned t0=clock();
    glDrawBuffer(GL_BACK);
    glReadBuffer(GL_BACK);

    glClearColor(0, 0, 0, 0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    camera.prepare_for_rendering(true);

    GLuint MY_CLIP_PLANE = GL_CLIP_PLANE0;
    GLuint MY_CLIP_PLANE1 = GL_CLIP_PLANE1;

    // Draw first cylinder.
    kjb::Vector top(3,0.0);
    kjb::Vector bottom(3,0.0);
    top(0) = 0.0;
    bottom(0) = 0.0;

    top(1) = 40.0;
    bottom(1) = -20.0;

    top(2) = 0.0;
    bottom(2) = 0.0;

    double radius = 20;
    glColor3f(255,0,0);
    GLUquadricObj* myQuadric = 0;
    myQuadric = gluNewQuadric();
//    kjb::draw_cylinder(camera, myQuadric, top, bottom, radius);
    double scat_radius = 13.0;
    double angle = 3.14159;
    Vector scat_top(3);
    scat_top(0) = 281.5;
    scat_top(1) = 174;
    scat_top(2) = 428.5;
    Vector scat_bot(3);
    scat_bot(0) = 281.5;
    scat_bot(1) = 161.619;
    scat_bot(2) = 428.5;
    Vector scat_start(3);
    scat_start(0) = 281.5 + scat_radius;
    scat_start(1) = 161.619;
    scat_start(2) = 428.5;
    Vector scat_end(3);
    scat_end(0) = 281.5 - scat_radius;
    scat_end(1) = 161.619;
    scat_end(2) = 428.5;
    kjb::draw_cylinder_section(myQuadric, scat_top, scat_bot, scat_radius, angle, scat_start, scat_end, MY_CLIP_PLANE, MY_CLIP_PLANE1);

    glFlush();

    glutSwapBuffers();

}

static void keyboard_glut(unsigned char key, int a, int b)
{
    if(object_selected == CAMERA_SELECTED)
    {
        Glut_perspective_camera::keyboard_callback(key);
    }
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

int main(int argc, char **argv)
{
#ifdef KJB_HAVE_GLUT
    init_onscreen_buffer(argc, argv);
//    camera.set_camera_centre_z(400);
//    camera.set_camera_centre_y(0);
//////////
    camera.set_camera_centre_x(281.5);
    camera.set_camera_centre_y(167);
    camera.set_camera_centre_z(800);

    int camera_menu = Glut_perspective_camera::create_glut_perspective_camera_submenu(camera_callback, &camera);
    Glut_perspective_camera::update_width_increment(29);
    Glut_perspective_camera::update_height_increment(29);
    Glut_perspective_camera::update_aspect_ratio_increment(0.05);
    Glut_perspective_camera::update_focal_length_increment(20.0);
    Glut_perspective_camera::update_pitch_increment(0.03);
    Glut_perspective_camera::update_yaw_increment(0.03);
    Glut_perspective_camera::update_roll_increment(0.1);
    Glut_perspective_camera::update_skew_increment(0.05);
    Glut_perspective_camera::update_translation_x_increment(4.0);
    Glut_perspective_camera::update_translation_y_increment(4.0);
    Glut_perspective_camera::update_translation_z_increment(4.0);
    Glut_perspective_camera::update_world_scale_increment(0.1);

    static GLfloat base_amb[] = {0.0f, 0.0f, 0.0f, 0.0f};
    glLightModelfv(GL_LIGHT_MODEL_AMBIENT, base_amb);

    static GLfloat ambient0  [] = {1.0f, 1.0f, 1.0f, 1.0f};
    static GLfloat diffuse0  [] = {0.8f, 0.8f, 0.8f, 1.0f};
    static GLfloat direction0[] = {500.0f, 500.0f, -1000.0f, 0.0f};

    glEnable(GL_LIGHT0);
    glLightfv(GL_LIGHT0, GL_AMBIENT, ambient0);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuse0);
    glLightfv(GL_LIGHT0, GL_POSITION, direction0);

    glutCreateMenu(main_menu_glut);
    glutAddSubMenu("Camera", camera_menu);

    glutAttachMenu(GLUT_RIGHT_BUTTON);
//////////
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

