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
#include <st_cpp/st_parapiped.h>
#include <st_cpp/st_perspective_camera.h>
#include <glut_cpp/glut_perspective_camera.h>
#include <glut_cpp/glut_parapiped.h>
#include <glut_cpp/glut_polymesh.h>
#include <gr_cpp/gr_find_shapes.h>
#include <gr_cpp/gr_triangular_mesh.h>
#include <st_cpp/st_set_camera_to_view_entire_model.h>
#include <iostream>
#include <fstream>

#include <time.h>

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

//Perspective_camera * camera = NULL;
Perspective_camera camera;
Triangular_mesh * tm = NULL;
char* fileName = NULL;
char cylFile[80];
char faceFile[80];

int gwidth = 600;
int gheight = 400;
static unsigned int object_selected = NONE;

bool isCylSection = true;
//bool isCylSection = false;

/** Store the planes here */
static std::vector<Polymesh_Plane> planes;
static std::vector<Cylinder_section> cyl;
/** Store the planes here */

static void display_glut()
{
    unsigned t0=clock();
    glDrawBuffer(GL_BACK);
    glReadBuffer(GL_BACK);

    glClearColor(0, 0, 0, 0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    camera.prepare_for_rendering(true);

    GLUquadricObj* myQuadric = 0;
    myQuadric = gluNewQuadric();

    if(isCylSection)
    {
        kjb::draw_cylinder_section(camera, myQuadric, cyl);
    }
    else
    {
        std::vector<Cylinder> cyl2;
        for(int i = 0; i < cyl.size(); i++)
        {
            cyl2.push_back(cyl[i]);
        }
        kjb::draw_cylinder(camera, myQuadric, cyl2);
    }

    glColor3f(1.0, 0.0, 0.0);

    /**** When you are done change this line with the function for rendering planes */
//    tm->solid_render();
    Polymesh & tpm = *tm;
    render_planes(tpm, planes);
    /**** When you are done change this line with the function for rendering planes */

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
//    std::cout << "Selecting the camera" << std::endl;
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
        Glut_polymesh::keyboard_callback(key);
    }

    if(key == 'w')
    {
        kjb_c::KJB_image * cimg = NULL;
        Base_gl_interface::capture_gl_view(&cimg);
        Image img(cimg);
        img.write("screenshot.jpg");
    }

    /*if(key == 'q')
    {
        string ppname = "parapiped";
        pp->write(ppname.c_str());
        camera->write("perspective_camera");
    }*/
}

static void pp_callback(int i)
{
    object_selected = POLYMESH_SELECTED;
}

int main(int argc, char* argv[])
{
  clock_t clock1 = clock();

  try{
      tm = new Triangular_mesh(argv[1]);
      fileName = argv[1];
  } catch(KJB_error e)
  {
      e.print(std::cout);
  }

  clock_t clock10 = clock();
  std::cout << "Reading trimesh from file: " << (clock10 - clock1) << std::endl;
  
//  (*tm).create_adjacency_matrix();
  (*tm).set_adjacency_matrix(fileName);

  /** Here is where your code goes! **/

  Polymesh & tpm = *tm;
  
  clock_t clock2 = clock();
  std::cout << "Reading polymesh: " << (clock2 - clock10) << std::endl;

  Vector cameraPos = set_camera_to_view_entire_model(camera, tpm, gwidth, gheight);
  camera.set_camera_centre_x(cameraPos(0));
  camera.set_camera_centre_y(cameraPos(1));
  camera.set_camera_centre_z(cameraPos(2));

//  find_planes(tpm, planes);
//  find_rectangles_to_render(tpm, planes);
//  find_cylinders_to_render(tpm, planes);

  strcpy(cylFile, fileName);
  strcpy(faceFile, fileName);
  strcat(cylFile, "_fittedCyl");
  strcat(faceFile, "_cylFaces");

  clock1 = clock();

  std::ifstream in_cyl(cylFile);
  std::ifstream in_face(faceFile);
  if(in_cyl.is_open() && in_face.is_open())
  {
      int num_cylinders = 0;
      in_cyl >> num_cylinders;
      int temp_cyl = 0;
      in_face >> temp_cyl;
      assert(temp_cyl == num_cylinders);
      for(int i = 0; i < num_cylinders; i++)
      {
          cyl.push_back(Cylinder_section(in_cyl)); 
          int num_faces = 0;
          in_face >> num_faces;
          std::vector<int> temp_faces;
          for(int j = 0; j < num_faces; j++)
          {
              int temp_face = 0;
              in_face >> temp_face;
              temp_faces.push_back(temp_face);
          }
          Vector plane_params;
          planes.push_back(kjb::Polymesh_Plane(tpm, plane_params, temp_faces));
      }
      // Construct cyl and planes from files.
      in_cyl.close();
      in_face.close();
  }
  else
  {
      find_cylinders_to_render_and_fit(faceFile, cylFile, tpm, planes, cyl);
  }

  clock2 = clock();
  std::cout << "Finding cylinders: " << (clock2 - clock1) << std::endl;
// 
//  find_right_triangles_to_render(tpm, planes);
//  find_adjacent_right_triangles_to_render(tpm, planes);
//  find_problem_triangles_to_render(tpm, planes);

  /** Here is where your code goes! **/


  glutInit(&argc, argv);
  glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH | GLUT_ACCUM
                                         | GLUT_ALPHA | GLUT_STENCIL);
  glutInitWindowSize(gwidth, gheight);
  glutCreateWindow("Display cylinders on model");
  glutDisplayFunc(display_glut);
  glutTimerFunc(10, timer_glut, 0);
  glutKeyboardFunc(keyboard_glut);
  kjb::opengl::default_init_opengl(gwidth, gheight);

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
/////
  int pp_menu = Glut_polymesh::create_glut_polymesh_submenu(pp_callback, tm, true, true, true, true, false, false, false);
//  int pp_menu = Glut_polymesh::create_glut_polymesh_submenu(pp_callback, pm, true, true, true, true, false, false, false);
  Glut_polymesh::update_translation_x_increment(5.0);
  Glut_polymesh::update_translation_y_increment(5.0);
  Glut_polymesh::update_translation_z_increment(5.0);
  Glut_polymesh::update_pitch_increment(0.5);   // 0.5, housing=0.05
  Glut_polymesh::update_yaw_increment(0.5);     // 0.5, housing=0.05
  Glut_polymesh::update_roll_increment(0.5);    // 0.5, housing=0.05

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
  ///////
  glutAttachMenu(GLUT_RIGHT_BUTTON);


  glutMainLoop();

  return 0;
}

