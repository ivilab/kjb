
/* $Id$ */
/* =========================================================================== *
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
 * =========================================================================== */



#include <st_cpp/st_parapiped.h>
#include <st_cpp/st_perspective_camera.h>
#include <glut_cpp/glut_perspective_camera.h>
#include <glut_cpp/glut_parapiped.h>
#include <gr_cpp/gr_find_shapes.h>
#include <gr_cpp/gr_2D_bounding_box.h>
#include <iostream>

#define NONE 0
#define CAMERA_SELECTED 1
#define POLYMESH_SELECTED 2

using namespace kjb;

Parametric_parapiped * p = NULL;
Perspective_camera * camera = NULL;

int gwidth = 500;
int gheight = 400;
static unsigned int object_selected = NONE;

static void display_glut()
{
    glDrawBuffer(GL_BACK);
    glReadBuffer(GL_BACK);

    glClearColor(0, 0, 0, 0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    camera->prepare_for_rendering(true);

    glColor3f(1.0, 0.0, 0.0);

    p->solid_render();

    glFlush();

    glutSwapBuffers();

}

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

static void keyboard_glut(unsigned char key, int a, int b)
{
    if(object_selected == CAMERA_SELECTED)
    {
        Glut_perspective_camera::keyboard_callback(key);
    }
    else if(object_selected == POLYMESH_SELECTED)
    {
        Glut_parapiped::keyboard_callback(key);
    }

    if(key == 'b')
    {
        Image img;
        p->force_update();
        camera->prepare_for_rendering(true);
        camera->get_rendering_interface().capture_gl_view(img);
        Bounding_Box2D bb;
        std::vector<Vector> points3D;
        try{
            p->get_polymesh().get_all_vertices(points3D);
            get_projected_bbox_from_3Dpoints(bb, points3D, camera->get_rendering_interface(), gwidth, gheight);
            bb.draw(img);
            img.write("bbtest.jpg");
        } catch(KJB_error e)
        {
            std::cout << "Could not get bounding box" << std::endl;
            img.write("bbtest.jpg");
            e.print(std::cout);
            return;
        }
    }
}

static void pp_callback(int i)
{
    object_selected = POLYMESH_SELECTED;
}

int main(int argc, char* argv[])
{

  camera = new Perspective_camera();
  p = new  Parametric_parapiped(0, 0, -1000, 100,200,100);


  glutInit(&argc, argv);
  glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH | GLUT_ACCUM
                                         | GLUT_ALPHA | GLUT_STENCIL);
  glutInitWindowSize(gwidth, gheight);
  glutCreateWindow("Find planes");
  glutDisplayFunc(display_glut);
  glutTimerFunc(10, timer_glut, 0);
  glutKeyboardFunc(keyboard_glut);
  kjb::opengl::default_init_opengl(gwidth, gheight);

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

  int pp_menu = Glut_parapiped::create_glut_parapiped_submenu(pp_callback, p);
  Glut_parapiped::update_translation_x_increment(5.0);
  Glut_parapiped::update_translation_y_increment(5.0);
  Glut_parapiped::update_translation_z_increment(5.0);
  Glut_parapiped::update_pitch_increment(0.1);
  Glut_parapiped::update_yaw_increment(0.1);
  Glut_parapiped::update_roll_increment(0.1);

  static GLfloat base_amb[] = {0.0f, 0.0f, 0.0f, 0.0f};
  glLightModelfv(GL_LIGHT_MODEL_AMBIENT, base_amb);

  static GLfloat ambient0  [] = {1.0f, 1.0f, 1.0f, 1.0f};
  static GLfloat diffuse0  [] = {0.8f, 0.8f, 0.8f, 1.0f};
  static GLfloat direction0[] = {500.0f, 500.0f, -1000.0f, 0.0f};


  glEnable(GL_LIGHT0);
  glLightfv(GL_LIGHT0, GL_AMBIENT, ambient0);
  glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuse0);
  glLightfv(GL_LIGHT0, GL_POSITION, direction0);

  /*static GLfloat amb_dif[4] = {1.0f, 1.0f, 1.0f, 0.2f};
  glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, amb_dif);

  glEnable(GL_LIGHTING);
  glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, amb_dif);*/

  glutCreateMenu(main_menu_glut);
  glutAddSubMenu("Camera", camera_menu);
  glutAddSubMenu("Mesh", pp_menu);
  glutAttachMenu(GLUT_RIGHT_BUTTON);


    if ( kjb_c::is_interactive() )
    {
        glutMainLoop();
    }


  return 0;
}

