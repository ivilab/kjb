/* $Id$ */

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
|     Luca Del Pero
|
* =========================================================================== */

#include <st_cpp/st_parapiped.h>
#include <st_cpp/st_perspective_camera.h>
#include <gr_cpp/gr_offscreen.h>
#include <iostream>
#include <i/i_float_io.h>
#include <glut_cpp/glut_perspective_camera.h>
#include <glut_cpp/glut_parapiped.h>

using namespace std;
using namespace kjb;

#ifdef KJB_HAVE_OPENGL
#ifdef KJB_HAVE_X11
#ifdef MAC_OSX
#include <OpenGL/OpenGL.h>
#else
#include <GL/gl.h>
#include <GL/glu.h>
#endif
#endif
#endif

#ifdef KJB_HAVE_GLUT
#ifdef MAC_OSX
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#endif

#define NONE 0
#define CAMERA_SELECTED 1
#define PARAPIPED_SELECTED 2

#define WRITE_PARAPIPED 9
#define EXIT_ID 10
static kjb::Offscreen_buffer* offscreen = 0;
static kjb::Perspective_camera * camera = 0;
static kjb::Parametric_parapiped * pp = 0;
static float gwidth = 500;
static float gheight = 400;
static uint32_t _current_action = 0;
static unsigned int object_selected = NONE;

/* -------------------------------------------------------------------------- */

void init_onscreen_buffer(int argc, char **argv);

int main(int argc, char* argv[])
{
    using namespace kjb;

    offscreen = kjb::create_and_initialize_offscreen_buffer(gwidth, gheight);
    Perspective_camera camera2;
    camera2.prepare_for_rendering(true);
    glColor3f(255,0,0);
    glBegin(GL_LINES);
    glVertex4d(0.0, 0.0, -20.0,1.0 );
    glVertex4d(1000.0, 0.0, -20.0, 1.0);
    glEnd();
    //toy_params.wire_render(0);
    //ps[0]->wire_render();
    Image img;
    Base_gl_interface::capture_gl_view(img);
    img.write("rendering.jpg");

    return 1;


    if( argc == 1)
    {
        pp = new Parametric_parapiped(0.0,0.0,0.0, 100.0, 50.0, 10.0);
        camera = new Perspective_camera();
        camera->set_focal_length(1000);
        camera->set_camera_centre_z(300);
    }
    else
    {
        pp = new Parametric_parapiped("parapiped");
        camera = new Perspective_camera("perspective_camera");
    }


    init_onscreen_buffer(argc, argv);
    static GLfloat base_amb[] = {0.0f, 0.0f, 0.0f, 0.0f};
    glLightModelfv(GL_LIGHT_MODEL_AMBIENT, base_amb);

    static GLfloat ambient0  [] = {0.2f, 0.2f, 0.2f, 1.0f};
    static GLfloat diffuse0  [] = {0.8f, 0.8f, 0.8f, 1.0f};
    static GLfloat direction0[] = {0.0f, 0.0f, 0.0f, 1.0f};


    glEnable(GL_LIGHT0);
    glLightfv(GL_LIGHT0, GL_AMBIENT, ambient0);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuse0);
    glLightfv(GL_LIGHT0, GL_POSITION, direction0);

    static GLfloat amb_dif[4] = {0.7f, 0.6f, 0.5f, 0.2f};
    glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, amb_dif);

    glEnable(GL_LIGHTING);
    glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, amb_dif);


    if ( kjb_c::is_interactive() )
    {
        std::cout << "Glut main loop" << std::endl;
        glutMainLoop();
    }

    delete camera;
    delete pp;
    return 0;
}

static void display_glut()
{
    unsigned t0=clock();
    glDrawBuffer(GL_BACK);
    glReadBuffer(GL_BACK);

    glClearColor(0, 0, 0, 0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    camera->prepare_for_rendering(true);

    glColor3f(0.66,0.66,0.66);

    pp->solid_render();
    glFlush();

    glutSwapBuffers();

}

static void camera_callback(int i)
{
    object_selected = CAMERA_SELECTED;
}

static void pp_callback(int i)
{
    object_selected = PARAPIPED_SELECTED;
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
static void keyboard_glut(unsigned char key, int a, int b)
{
    if(object_selected == CAMERA_SELECTED)
    {
        Glut_perspective_camera::keyboard_callback(key);
    }
    else if(object_selected == PARAPIPED_SELECTED)
    {
        Glut_parapiped::keyboard_callback(key);
    }

    if(key == 'w')
    {
        string ppname = "parapiped";
        pp->write(ppname.c_str());
        camera->write("perspective_camera");
    }
}

static void timer_glut(int ignored)
{
    glutPostRedisplay();
    glutTimerFunc(10, timer_glut, 0);
}


static void main_menu_glut(int id)
{
    using namespace kjb;
    _current_action = id;
    if(id == EXIT_ID)
    {
        delete pp;
        delete camera;
        exit(EXIT_SUCCESS);
    }
}

void init_onscreen_buffer(int argc, char **argv)
{
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH | GLUT_ACCUM
                                        | GLUT_ALPHA | GLUT_STENCIL);
    glutInitWindowSize(gwidth, gheight);
    glutCreateWindow("Perspective camera");
    glutDisplayFunc(display_glut);
    glutReshapeFunc(reshape_glut);
    glutKeyboardFunc(keyboard_glut);
    glutTimerFunc(10, timer_glut, 0);

    int camera_menu = Glut_perspective_camera::create_glut_perspective_camera_submenu(camera_callback, camera);
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

    int pp_menu = Glut_parapiped::create_glut_parapiped_submenu(pp_callback, pp);
    Glut_parapiped::update_translation_x_increment(5.0);
    Glut_parapiped::update_translation_y_increment(5.0);
    Glut_parapiped::update_translation_z_increment(5.0);
    Glut_parapiped::update_pitch_increment(0.1);
    Glut_parapiped::update_yaw_increment(0.1);
    Glut_parapiped::update_roll_increment(0.1);

    glutCreateMenu(main_menu_glut);
    glutAddSubMenu("Camera", camera_menu);
    glutAddSubMenu("Parapiped", pp_menu);

    glutAttachMenu(GLUT_RIGHT_BUTTON);

    kjb::opengl::default_init_opengl(gwidth, gheight);

}

#else

//Temporary, fix it so that it works on Linux as well
int main(int argc, char **argv)
{
    std::cout << "Glut not supported, cannot run this test program" << std::endl;
    return 0;
}

#endif


/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

