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

#define DEBUG

#include <vector>
#include <gr_cpp/gr_opengl.h>
#include <gr_cpp/gr_spline.h>
#include <m_cpp/m_vector.h>
#include <l/l_sys_rand.h>

void init();
void draw();
void reshape(int width, int height);

static void key_cb(unsigned char code, int x, int y);


using namespace std;
using namespace kjb;
using namespace kjb::opengl;
using kjb_c::pause_on_next;

vector<Vector> points;
Gl_nurbs_surface surf;

int main(int argc, char *argv[])
{
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGBA);
    glutInitWindowPosition(100,100);
    glutInitWindowSize(320,320);
    glutCreateWindow("KJB Bezier Test");
    glutDisplayFunc(draw);
//  glutIdleFunc(draw);
    glutReshapeFunc(reshape);

    glutKeyboardFunc(key_cb);

    //adding here the mouse processing callbacks
//    glutMouseFunc(mouse_btn);
//    glutMotionFunc(mouse_active_motion);
//    glutPassiveMotionFunc(mouse_passive_motion);
//  glutEntryFunc(mouse_entry);

    init();

    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_AUTO_NORMAL);
    glEnable(GL_NORMALIZE);
    glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, GL_TRUE);
    
    glutMainLoop();

    return 0;
}

void init()
{
    vector<Vector> ctl_pts;
    Vector v(3);

    v[0] = 0.5; v[1] = 0.0; v[2] = 0.0;
    ctl_pts.push_back(v);
    v[0] = 1.0; v[1] = 0.0; v[2] = 0.0;
    ctl_pts.push_back(v);
    v[0] = 1.0; v[1] = 0.0; v[2] = 0.0;
    ctl_pts.push_back(v);
    v[0] = 1.0; v[1] = 0.5; v[2] = 0.0;
    ctl_pts.push_back(v);
    v[0] = 1.0; v[1] = 1.0; v[2] = 0.0;
    ctl_pts.push_back(v);
    v[0] = 1.0; v[1] = 1.0; v[2] = 0.0;
    ctl_pts.push_back(v);
    v[0] = 0.5; v[1] = 1.0; v[2] = 0.0;
    ctl_pts.push_back(v);
    v[0] = 0.0; v[1] = 1.0; v[2] = 0.0;
    ctl_pts.push_back(v);
    v[0] = 0.0; v[1] = 1.0; v[2] = 0.0;
    ctl_pts.push_back(v);
    v[0] = 0.0; v[1] = 0.5; v[2] = 0.0;
    ctl_pts.push_back(v);
    v[0] = 0.0; v[1] = 0.0; v[2] = 0.0;
    ctl_pts.push_back(v);
    v[0] = 0.0; v[1] = 0.0; v[2] = 0.0;
    ctl_pts.push_back(v);
    v[0] = 0.5; v[1] = 0.0; v[2] = 0.0;
    ctl_pts.push_back(v);

    vector<vector<Vector> > surface_ctl_pts(10);

    for(int i = 0; i < 10; i++)
    {
        vector<Vector> cur(ctl_pts);
        for(int j = 0; j < ctl_pts.size(); j++)
        {
            Vector v(3);
            v[0] = 2*sin(i * 2 * M_PI / 5);
            v[1] = 2*cos(i * 2 * M_PI / 5);
            v[2] = i;
            cur[j] += v;
        }

        surface_ctl_pts[i] = cur;
    }

    float knots_s[] = {0,0,0,0,1,2,3,4,5,6,7,7,7,7};
    uint num_knots_s = 14;
    float knots_t[] = {0,0,0,0,1,1,1,2,2,2,3,3,3,4,4,4,4};
    uint num_knots_t = 17;

    surf = Gl_nurbs_surface (
        num_knots_s,
        knots_s,
        num_knots_t,
        knots_t,
        3,
        3,
        surface_ctl_pts,
        GL_MAP2_VERTEX_3
    );

    points = vector<Vector>(100, Vector(3));
    kjb_c::kjb_seed_rand_with_tod();
    for(int i = 0; i < 100; i++)
    {   
        float u = kjb_c::kjb_rand();
        float v = kjb_c::kjb_rand();
        points[i] = surf.evaluate(u, v);
    }
}


void draw()
{
    glClearColor(1.0, 1.0, 1.0, 0.0);

    GLfloat mat_diffuse[] = { 0.2, 0.9, 0.2, 1.0 };
    GLfloat mat_specular[] = { 1.0, 1.0, 1.0, 1.0 };
    GLfloat mat_shininess[] =  {100.0};

    glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, mat_diffuse);
    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, mat_specular);
    glMaterialfv(GL_FRONT_AND_BACK, GL_SHININESS, mat_shininess);

	/* clear the drawing buffer */
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);        
    Gl_bezier_curve c1(3, 3, GL_MAP1_VERTEX_3);



    glPointSize(5);
    glBegin(GL_POINTS);
        glColor3f(0.,0.,0.);

        for(int i = 0; i < 100; i++)
        {   
            glVertex(points[i]);
        }
    glEnd();

    glColor3f(1.,1.,1.);


    gluNurbsProperty(surf.get_nurbs_object(), GLU_SAMPLING_METHOD, GLU_PATH_LENGTH);
    gluNurbsProperty(surf.get_nurbs_object(), GLU_SAMPLING_TOLERANCE, 10);

    gluBeginSurface(surf.get_nurbs_object());
    surf.gl_call();
    gluEndSurface(surf.get_nurbs_object());

////    c2.print_svg();
//    for(float i = 0; i < 1.; i += .05)
//    {
//        Vector v = c2.evaluate(i);
////        std::cout << v.get_length() << std::endl;
////        std::cout << "c1(" << i << ") " << v[0] << ",\t" << v[1] << ",\t" << v[2] << std::endl;
//
//        glPointSize(5.);
//        glColor3f(i, 0, 0);
//        glBegin(GL_POINTS);
//        glVertex(v);
//        glEnd();
//    }

    glFlush();

    glutSwapBuffers(); 
}

void reshape(int width, int height)
{
    if(width == 0) width = 1;
    if(height == 0) height = 1;

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    
    glViewport(0, 0, width, height);
//    glOrtho(-.1,1.1, -.1, 1.1, -100.0, 100.0);
    gluPerspective(90,1,.1, 100);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    gluLookAt(0.0,0.0,10.0, 
              0.0,0.0,0.0,
              0.0,1.0,0.0);
}

static void key_cb(unsigned char code, int x, int y) 
{
    switch(code)
    {   
        case 'a':
            glRotatef(10, 0., 0., 1.);
            cout << "rotate" << endl;
            glutPostRedisplay();
            break;
        case 's':
            glRotatef(10, 0., 1., 0.);
            glutPostRedisplay();
            break;
        case 'd':
            glRotatef(-10, 0., 0., 1.);
            glutPostRedisplay();
            break;
        case 'w':
            glRotatef(-10, 0., 1., 0.);
            glutPostRedisplay();
            break;
        case 'W':
            glTranslatef(0,0,1.);
            cout << "translate" << endl;
            glutPostRedisplay();
            break;
        case 'S':
            glTranslatef(0,0,-1.);
            cout << "translate" << endl;
            glutPostRedisplay();
            break;

    }

}


