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


/**
 * @file test_chamfer.cpp
 *
 * Tests chamfer transform by drawing a line from cursor to 
 * nearest edge point, and drawing that point's normal vector.
 *
 **/

#include <m/m_incl.h>
#include <l/l_incl.h>
#include <l_cpp/l_cpp_incl.h>
#include <m_cpp/m_cpp_incl.h>
#include <gr_cpp/gr_sprite.h>
#include <i_cpp/i_image.h>

#include <edge/edge_detector_old.h>
#include <g/g_chamfer.h>
#include <stdlib.h>
#include <unistd.h> /* for usleep() */


#ifdef KJB_HAVE_OPENGL
/*
 * Sometimes glu.h includes glut.h--sometimes not.
 */
#ifdef MAC_OSX
#    include <OpenGL/glu.h>
#    include <GLUT/glut.h>
#else 
	#ifdef WIN32
	#	 include <GL/glu.h>
	#	 include <glut.h>
	#else 
	#    include <GL/glu.h>       
	#    include <GL/glut.h>       
	#endif
#endif
#endif

using namespace kjb;

struct Point { float x; float y;};
Point mouse_pt = {0,0};
Point nearest_pt;
Point nearest_norm;

int width, height;
GLenum mouse_modifiers = 0;

void mouse_btn(int button, int state, int x, int y);
void mouse_active_motion(int x, int y);
void mouse_passive_motion(int x, int y);
void mouse_all_motion(int x, int y);
void draw();
void idle();
void reshape(int width, int height);
void key_cb(unsigned char _key, int x, int y);


kjb_c::Edge_point_DEPRECATED* edges = 0;
size_t num_rows, num_cols;
kjb_c::Matrix* distance_map = 0;
kjb_c::Edge_point_DEPRECATED const*** edge_map = 0;

Image img;
Image chamfer_img;
Gl_sprite sprite;

void init()
{
    // load image
    img = Image("input/in.png");
    num_rows = img.get_num_rows();
    num_cols = img.get_num_cols();
    glutReshapeWindow(num_cols, num_rows);


    // find edges
    kjb_c::detect_edge_points_DEPRECATED(&edges, NULL, img.c_ptr(), 1, 10, 10);

    // do transform
    kjb_c::chamfer_transform_2(edges, num_rows, num_cols, 3, &distance_map, &edge_map);

    chamfer_img = Image(*distance_map);

    // load sprite
    sprite = Gl_sprite(img, -.99);
}

int main (int argc, char *argv[])
{
    kjb_c::kjb_init();

    kjb_c::kjb_l_set("page", "off");
    
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGBA);
    glutInitWindowPosition(100,100);
    glutInitWindowSize(320,320);
    glutCreateWindow("Default Title");
    glutDisplayFunc(draw);
    glutIdleFunc(idle);
    glutReshapeFunc(reshape);
    glutKeyboardFunc(key_cb);


    glutMouseFunc(mouse_btn);
    glutMotionFunc(mouse_active_motion);
    glutPassiveMotionFunc(mouse_passive_motion);
/*  glutEntryFunc(mouse_entry); */

    init();

    glutMainLoop();

    return EXIT_SUCCESS;
}

void mouse_btn(int button, int state, int x, int y)
{
    mouse_modifiers = glutGetModifiers();
    /* GLUT_ACTIVE_SHIFT, etc; */

}

void mouse_active_motion(int x, int y)
{
}

void mouse_passive_motion(int x, int y)
{
    // look up edge in edge map
    if(x >= num_cols || y >= num_rows) return;
    const kjb_c::Edge_point_DEPRECATED* edge = edge_map[y][x];

    // draw line to nearest edge
    const Point p = {x, y};
    const Point p2 = {edge->x, edge->y};
    const Point n = {edge->dx, edge->dy};

    mouse_pt = p;
    nearest_pt = p2;
    nearest_norm = n;

    glutPostRedisplay();
}



void draw()
{
    glClearColor(1.0, 1.0, 1.0, 0.0);

	/* clear the drawing buffer */
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);        

    sprite.render();

    glDisable(GL_DEPTH_TEST);
    glBegin(GL_LINES);
    glColor3f(0, 0, 0);
    glVertex2fv((float*) &mouse_pt);
    glVertex2fv((float*) &nearest_pt);
    glColor3f(1, 0, 0);
    glVertex2fv((float*) &nearest_pt);
    glVertex2f(
            nearest_pt.x + nearest_norm.x * 10,
            nearest_pt.y + nearest_norm.y * 10);

    glVertex2f(0, 0);
    glVertex2f(width, height);
    glEnd();


    glutSwapBuffers(); 
}

void reshape(int w, int h)
{
    if(w == 0) w = 1;
    if(h == 0) h = 1;

    width = w;
    height = h;

    /* default projection and camera for 2D applications: */
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    
    glViewport(0, 0, width, height);
    glOrtho(0,width, height, 0, -1.0, 1.0);
    glMatrixMode(GL_MODELVIEW);

    glLoadIdentity();
    gluLookAt(0.0,0.0,1.0, 
              0.0,0.0,0.0,
              0.0,1.0,0.0);
}

void idle()
{
    usleep(10000);
}

void cleanup()
{
    kjb_c::free_matrix(distance_map);

    kjb_c::free_edge_points_DEPRECATED(edges);
    kjb_c::free_2D_ptr_array((void ***) edge_map);
}

void key_cb(unsigned char _key, int x, int y) 
{
    switch(_key)
    {
        case 'q':
            cleanup();
            exit(0);
            break;
    }
}
