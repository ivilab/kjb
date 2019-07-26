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
   |  Author:  Kyle Simek
 * =========================================================================== */

#include <gr_cpp/gr_opengl.h>
#ifdef KJB_HAVE_OPENGL
#ifdef MAC_OSX
#    include <OpenGL/glu.h>
#    include <GLUT/glut.h>
#else 
    #ifdef WIN32
    #    include <GL/glu.h>
    #    include <glut.h>
    #else 
    #    include <GL/glu.h>       
    #    include <GL/glut.h>       
    #endif
#endif 
#endif /* KJB_HAVE_OPENGL */


#include <m_cpp/m_cpp_incl.h>
#include <st_cpp/st_calibrated_camera.h>
#include <g_cpp/g_camera_calibration.h>
#include <gr_cpp/gr_sprite.h>
#include <iostream>
#include <cstdlib>

using namespace kjb;
using namespace kjb::opengl;
using namespace std;

vector<Gl_sprite> sprites(36);
vector<Calibrated_camera> camera(36);
bool manual_operation = false;

int cur_view = 0;

void print_projection_matrix();
void print_modelview_matrix();

void idle();
void draw();
void reshape(int width, int height);

void draw_camera_axes();
void draw_image();
void draw_cube();
void key_cb(unsigned char _key, int x, int y);

int load()
{
    char buffer[512];
    for(int i = 0; i < 36; i++)
    {
        /* these were all calibrated using the top-left convention pixel coordinate convention */
        sprintf(buffer, "input/cameras_2/camera_%d.txt", i);

        Matrix camera_matrix(buffer);

        Calibration_descriptor cal;
            cal.image_width  = 530;
            cal.image_height = 397;
            cal.convention = Calibration_descriptor::TOP_LEFT;
            cal.world_origin_in_front = true;
            cal.look_down_positive_z = false;


        camera[i] = Calibrated_camera(
                camera_matrix,
                cal);

        sprintf(buffer, "input/cal_%d.tiff", i);
        sprites[i] = Sprite(kjb::Image(string(buffer)));

    }

    // initialize three cameras using bottom-left convention, to ensure they're
    // consistent with the top-left dudes
    Matrix camera_matrix;
    Calibration_descriptor cal;
        cal.image_width  = 530;
        cal.image_height = 397;
        cal.convention = Calibration_descriptor::BOTTOM_LEFT;
        cal.world_origin_in_front = true;
        cal.look_down_positive_z = false;

    sprintf(buffer, "input/cameras_2/camera_%d_bl.txt", 0);
    camera_matrix = Matrix(buffer);
    camera[0] = Calibrated_camera(camera_matrix, cal);

    sprintf(buffer, "input/cameras_2/camera_%d_bl.txt", 1);
    camera_matrix = Matrix(buffer);
    camera[1] = Calibrated_camera(camera_matrix, cal);

    sprintf(buffer, "input/cameras_2/camera_%d_bl.txt", 21);
    camera_matrix = Matrix(buffer);
    camera[21] = Calibrated_camera(camera_matrix, cal);

}

void initial_tests()
{
    int size = camera.size();
    for(int i = 0; i < 36; i++)
    {
        camera[i].prepare_for_rendering(false);
        Matrix modelview = opengl::get_modelview_matrix();
        Matrix projection = opengl::get_projection_matrix();

        Vector origin = projection * modelview * Vector(0.0,0.0,0.0,1.0);

        if(origin[3] < 0.0)
        {
            cerr << "Origin maps to negative w-coordinate:\n";
            cerr << origin << endl;
            abort();

        }
        origin /= origin[3];

//        if(
//            origin[0] > 1.0 || origin[0] < -1.0 ||
//            origin[1] > 1.0 || origin[1] < -1.0 ||
//            origin[2] > 1.0 || origin[2] < -1.0 )
//        {
//            cerr << "Origin maps outside of viewing volume:\n";
//            cerr << origin << endl;
//            abort();
//        }

    }

}

int main (int argc, char *argv[])
{
    kjb_c::kjb_init();
    // create a camera from a calibrated camera matrix
    // display image in window
    Image img("input/cal_0.tiff");
    size_t width = img.get_num_cols();
    size_t height = img.get_num_rows();

    kjb_c::kjb_l_set("page", "off");

    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGBA);
    glutInitWindowPosition(100,100);
    glutInitWindowSize(width,height);
    glutCreateWindow("Camera test");

    glutDisplayFunc(draw);
    glutIdleFunc(idle);
    glutReshapeFunc(reshape);
    glutKeyboardFunc(key_cb);


    // set up viewport
    glViewport(0, 0, width, height);

    glEnable (GL_BLEND);
    glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    /* *
     * questions:
     *      1. does img display upside-down normally? 
     *      2. does img ignore camera matrix?
     *      */


    load();
    initial_tests();

    if ( kjb_c::is_interactive() )
    {
        glutMainLoop();
    }

    return EXIT_SUCCESS;
}

void draw()
{
    glClearColor(1.0, 1.0, 1.0, 0.0);

    /* clear the drawing buffer */
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);        

    cout << "Drawing view #" << cur_view << "\n";
    draw_image();
    draw_cube();

    // these should point left and up;
    draw_camera_axes();

    glFlush();

    glutSwapBuffers(); 
}


void reshape(int width, int height)
{
    if(width == 0) width = 1;
    if(height == 0) height = 1;

    glViewport(0, 0, width, height);

}


void draw_camera_axes()
{

    camera[cur_view].prepare_for_rendering(false);

    glLoadIdentity();

    glBegin(GL_LINES);
    glVertex3f(0.0,0.0,-10);
    glVertex3f(1000.,0.0,-10);
    glVertex3f(0.0,0.0,-10);
    glVertex3f(0.0,1000.,-10);
    glEnd();


}
void draw_image()
{
    sprites[cur_view].set_position(0,0);
    sprites[cur_view].set_scale(1,1);
    sprites[cur_view].render();
}


void draw_cube()
{

    glDisable( GL_TEXTURE_2D);
    glEnable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);

    camera[cur_view].prepare_for_rendering(false);

    //print_projection_matrix();
    //print_modelview_matrix();
    // display 3d cube over image
    glBegin(GL_QUADS);
        glColor4f(1.0, 0.0, 0.0, 0.8);
        glVertex3f(0., 0., 0.);
        glVertex3f(0.,160., 0.);
        glVertex3f(160.,160., 0.);
        glVertex3f(160., 0., 0.);

        glColor4f(0.0, 1.0, 0.0, 0.8);
        glVertex3f(0., 0., 160.);
        glVertex3f(160., 0., 160.);
        glVertex3f(160.,160., 160.);
        glVertex3f(0.,160., 160.);

        glColor4f(0.0, 0.0, 1.0, 0.8);
        glVertex3f(0., 0., 0.);
        glVertex3f(0., 0.,160.);
        glVertex3f(0., 160.,160.);
        glVertex3f(0., 160., 0.);

        glColor4f(1.0, 1.0, 0.0, 0.8);
        glVertex3f(160., 0., 0.);
        glVertex3f(160., 160., 0.);
        glVertex3f(160., 160.,160.);
        glVertex3f(160., 0.,160.);

        glColor4f(0.0, 1.0, 1.0, 0.8);
        glVertex3f(0., 0., 0.);
        glVertex3f(160., 0., 0.);
        glVertex3f(160., 0., 160.);
        glVertex3f(0., 0., 160.);

        glColor4f(1.0, 0.0, 1.0, 0.8);
        glVertex3f(0., 160., 0.);
        glVertex3f(0., 160., 160.);
        glVertex3f(160., 160., 160.);
        glVertex3f(160., 160., 0.);
    glEnd();

    glColor3f(0.0, 0.0, 0.0);
    glBegin(GL_LINE_STRIP);
        glVertex3f(0,0,0);
        glVertex3f(0,160,0);
        glVertex3f(160,160,0);
        glVertex3f(160,0,0);
        glVertex3f(0,0,0);
    glEnd();

    glBegin(GL_LINE_STRIP);
        glVertex3f(0,0,160);
        glVertex3f(0,160,160);
        glVertex3f(160,160,160);
        glVertex3f(160,0,160);
        glVertex3f(0,0,160);
    glEnd();

    glBegin(GL_LINES);
        glVertex3f(0,0,0);
        glVertex3f(0,0,160);
        glVertex3f(160,0,0);
        glVertex3f(160,0,160);
        glVertex3f(160,160,0);
        glVertex3f(160,160,160);
        glVertex3f(0,160,0);
        glVertex3f(0,160,160);
    glEnd();
}

void idle()
{
    if(manual_operation)
        return;

    cur_view = (cur_view + 1) % 36;
    usleep(100000);
    glutPostRedisplay();
}

void key_cb(unsigned char _key, int x, int y) 
{
    switch(_key)
    {
        case 'n':
            cur_view = (cur_view + 1) % 36;
            manual_operation = true;
            glutPostRedisplay();
            break;
        case 'p':
            cur_view = (cur_view + 36-1) % 36;
            manual_operation = true;
            glutPostRedisplay();
            break;
        case 'o':
            print_projection_matrix();
            print_modelview_matrix();
            manual_operation = true;
            break;
        case 'g':
            manual_operation = false;
            break;
        case 'q':
            exit(0);
            break;
    }
}

void print_projection_matrix()
{
    double proj[16];

    glGetDoublev(GL_PROJECTION_MATRIX, proj);
    cout << "Projection_matrix:\n" ;
    cout << "[" ;
    cout << proj[0] << ", ";
    cout << proj[4] << ", ";
    cout << proj[8] << ", ";
    cout << proj[12] << "]\n";

    cout << "[" ;
    cout << proj[1] << ", ";
    cout << proj[5] << ", ";
    cout << proj[9] << ", ";
    cout << proj[13] << "]\n";
    cout << "[" ;
    cout << proj[2] << ", ";
    cout << proj[6] << ", ";
    cout << proj[10] << ", ";
    cout << proj[14] << "]\n";
    cout << "[" ;
    cout << proj[3] << ", ";
    cout << proj[7] << ", ";
    cout << proj[11] << ", ";
    cout << proj[15] << "]\n";
}

void print_modelview_matrix()
{
    double proj[16];

    glGetDoublev(GL_MODELVIEW_MATRIX, proj);
    cout << "Modelview matrix:\n" ;
    cout << "[" ;
    cout << proj[0] << ", ";
    cout << proj[4] << ", ";
    cout << proj[8] << ", ";
    cout << proj[12] << "]\n";

    cout << "[" ;
    cout << proj[1] << ", ";
    cout << proj[5] << ", ";
    cout << proj[9] << ", ";
    cout << proj[13] << "]\n";

    cout << "[" ;
    cout << proj[2] << ", ";
    cout << proj[6] << ", ";
    cout << proj[10] << ", ";
    cout << proj[14] << "]\n";
    cout << "[" ;
    cout << proj[3] << ", ";
    cout << proj[7] << ", ";
    cout << proj[11] << ", ";
    cout << proj[15] << "]\n";
}
