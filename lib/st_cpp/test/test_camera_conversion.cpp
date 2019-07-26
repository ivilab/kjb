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

#define NONE 0
#define CAMERA_SELECTED 1
#define POLYMESH_SELECTED 2

using namespace kjb;

/**
 * Test converting OpenGL camera parameters into a parametric camera.
 */
class CameraConversionTest: public kjb::Readable, public kjb::Writeable
{

public:
    // Constants
    static const int kWidth = 640;
    static const int kHeight = 480;
    static const double kNear; // = 0.1;
    static const double kFar; // = 150;
    static const double kFov; // = 45.0; // degrees

    static unsigned int object_selected;

    CameraConversionTest()
    {
    }

    virtual ~CameraConversionTest()
    {
        delete camera;
    }

    static void Init()
    {
        // TODO: Read in the parameters from a file
        int height = kHeight;
        double near = kNear;
        double far = kFar;
        double fov = kFov; // degrees

        Vector eye(0, 0, 4);
        Vector look(0, 0, 0);
        Vector up(0.0, 1.0, 0.0);

        // Convert fov to radians
        fov *= (M_PI / 180.0);

        // Initialize the camera
        camera = new Perspective_camera(near, far);
        camera->set_look_at(eye, look, up);

        double focal_length = (height / 2.0) / tan(fov / 2.0);
        camera->set_focal_length(focal_length);

        // Initialize the parapiped
        Vector parapiped_center(0.5, 1, -2);
        m_parapiped.set_centre(parapiped_center);

        m_parapiped.set_width(2 * 1);
        m_parapiped.set_height(2 * 1);
        m_parapiped.set_length(2 * 0.3);

        Quaternion parapiped_orientation(Vector(0, 1, 0), M_PI * 0.2);
        m_parapiped.set_angles_from_quaternion(parapiped_orientation);
    }

    virtual void read(std::istream&) throw (kjb::Illegal_argument,
            kjb::IO_error)
    {
        using std::ostringstream;
        using std::istringstream;

        // TODO: finish reading parameters from a file
    }

    virtual void read(const char * fname) throw (kjb::IO_error,
            kjb::Illegal_argument)
    {
        Readable::read(fname);
    }

    virtual void write(std::ostream&) const throw (kjb::IO_error)
    {
        using std::ostringstream;
        using std::istringstream;

        // TODO: finish writing parameters to a file
    }

    virtual void write(const char * fname) const throw (kjb::IO_error)
    {
        Writeable::write(fname);
    }

    virtual int main_loop(int argc, char* argv[])
    {
        glutInit(&argc, argv);
        glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH | GLUT_ACCUM
                | GLUT_ALPHA | GLUT_STENCIL);
        glutInitWindowSize(kWidth, kHeight);
        glutCreateWindow("Test camera conversion");
        glutDisplayFunc(display_glut);
        glutTimerFunc(10, timer_glut, 0);
        glutKeyboardFunc(keyboard_glut);
        kjb::opengl::default_init_opengl(kWidth, kHeight);

        int
                camera_menu =
                        Glut_perspective_camera::create_glut_perspective_camera_submenu(
                                camera_callback, camera);
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

        int pp_menu = Glut_parapiped::create_glut_parapiped_submenu(
                pp_callback, &m_parapiped);
        Glut_parapiped::update_translation_x_increment(5.0);
        Glut_parapiped::update_translation_y_increment(5.0);
        Glut_parapiped::update_translation_z_increment(5.0);
        Glut_parapiped::update_pitch_increment(0.1);
        Glut_parapiped::update_yaw_increment(0.1);
        Glut_parapiped::update_roll_increment(0.1);

        static GLfloat base_amb[] =
        { 0.0f, 0.0f, 0.0f, 0.0f };
        glLightModelfv(GL_LIGHT_MODEL_AMBIENT, base_amb);

        static GLfloat ambient0[] =
        { 1.0f, 1.0f, 1.0f, 1.0f };
        static GLfloat diffuse0[] =
        { 0.8f, 0.8f, 0.8f, 1.0f };
        static GLfloat direction0[] =
        { 500.0f, 500.0f, -1000.0f, 0.0f };

        glEnable( GL_LIGHT0);
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
        glutAttachMenu( GLUT_RIGHT_BUTTON);

        if ( kjb_c::is_interactive() )
        {
            glutMainLoop();
        }

        return 0;
    }

private:
    static Parametric_parapiped m_parapiped;
    static Perspective_camera* camera;

    static void timer_glut(int)
    {
        glutPostRedisplay();
        glutTimerFunc(10, timer_glut, 0);
    }

    static void camera_callback(int)
    {
        object_selected = CAMERA_SELECTED;
    }

    static void main_menu_glut(int)
    {
    }

    static void keyboard_glut(unsigned char key, int, int)
    {
        if (object_selected == CAMERA_SELECTED)
        {
            Glut_perspective_camera::keyboard_callback(key);
        }
        else if (object_selected == POLYMESH_SELECTED)
        {
            Glut_parapiped::keyboard_callback(key);
        }
    }

    static void pp_callback(int)
    {
        object_selected = POLYMESH_SELECTED;
    }

    static void display_glut()
    {
        glDrawBuffer( GL_BACK);
        glReadBuffer(GL_BACK);

        glClearColor(0, 0, 0, 0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        camera->prepare_for_rendering(true);

        glColor3f(0.0, 1.0, 0.0);

        m_parapiped.solid_render();

        drawAxes();

        glFlush();

        glutSwapBuffers();
    }

    static void drawGLAxes(double n)
    {
        glBegin( GL_LINES); //draw the axis...
        glColor3f(1.0f, 0.0f, 0.0f);
        glVertex3d(0, 0, 0); //X axis
        glVertex3d(n, 0, 0);

        glColor3f(0.0f, 1.0f, 0.0f);
        glVertex3d(0, 0, 0); //Y axis
        glVertex3d(0, n, 0);

        glColor3f(0.0f, 0.0f, 1.0f);
        glVertex3d(0, 0, 0); //Z axis
        glVertex3d(0, 0, n);

        glEnd();
    }

    static void drawAxes(void)
    {
        double x, y, z;
        x = -3;
        y = -2;
        z = -6;

        glTranslated(x, y, z);

        glDisable( GL_DEPTH_TEST);
        drawGLAxes(0.5);
        glEnable(GL_DEPTH_TEST);
    }
};

const double CameraConversionTest::kNear = 0.1;
const double CameraConversionTest::kFar = 150;
const double CameraConversionTest::kFov = 45.0; // degrees

unsigned int CameraConversionTest::object_selected = NONE;
Parametric_parapiped CameraConversionTest::m_parapiped;
Perspective_camera* CameraConversionTest::camera = NULL;

int main(int argc, char* argv[])
{
    CameraConversionTest test;

    test.Init();

    return test.main_loop(argc, argv);
}
