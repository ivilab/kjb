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

#ifdef KJB_HAVE_OPENGL
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
#endif // KJB_HAVE_OPENGL
#include <gr_cpp/gr_primitive.h>
#include <unistd.h>

using namespace kjb;

void draw();
void idle();
void reshape(int width, int height);

int main(int argc, char* argv[])
{
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGBA);
    glutInitWindowPosition(100,100);
    glutInitWindowSize(320,320);
    glutCreateWindow("Primitive test");

    glutDisplayFunc(draw);
    glutIdleFunc(idle);
    glutReshapeFunc(reshape);

    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_AUTO_NORMAL);
    glEnable(GL_NORMALIZE);
    glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, GL_TRUE);

    glutMainLoop();

    return EXIT_SUCCESS;
}

enum {SOLID = 0, WIRE, SILHOUETTE};
int mode = SOLID;
float rotation = 0;
float rotation_increment = 1;
float scale = 1.0;
float scale_increment = .01;
float scale_max = 1.1;

void idle()
{
    rotation += rotation_increment;
    if(rotation > 360.0)
    {
        rotation -= 360;
        mode += 1;
        mode %= 3;
    }

    scale += scale_increment;
    if(scale <= 1.0 || scale >= scale_max)
    {
        scale_increment *= -1;
    }

    usleep(10000);

    glutPostRedisplay();

}

void render(const kjb::Generic_renderable& renderable)
{
    switch(mode)
    {
        case WIRE:
            renderable.wire_render();
            break;
        case SILHOUETTE:
            renderable.wire_occlude_render();
            break;
        case SOLID:
            renderable.solid_render();
            break;
    }
}

void draw()
{
    using namespace kjb::opengl;

    glClearColor(1.0, 1.0, 1.0, 0.0);

	/* clear the drawing buffer */
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);        

    Sphere sphere(1* scale);
    Cylinder cylinder(1, 1, 10 * scale);
    Cylinder cone(2, 0, 2 * scale);

    Vector position(3, 0.0);
    Vector direction(3, 0.0);
    direction[2] = 10.0;
    Arrow3d  arrow(position, direction * scale, 3);

    glColor3f(0.0, 0.0, 0.0);
    glPushMatrix();

    // make them spin!
    glRotatef(rotation, 1.0, 0.0, 0.0);

    // make them point up instead of into the camera;
    glRotatef(-90, 1.0, 0.0, 0.0);

    glPushMatrix();
    glTranslatef(-10, 0, 0);
    render(sphere);
    glPopMatrix();

    glPushMatrix();
    glTranslatef(-5, 0, 0);
    render(cylinder);
    glPopMatrix();

    glPushMatrix();
    glTranslatef(0, 0, 0);
    render(cone);
    glPopMatrix();

    glPushMatrix();
    glTranslatef(5, 0, 0);
    render(arrow);
    glPopMatrix();

    glPopMatrix();
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
    glOrtho(-15,15, -15, 15, -20, 20);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    gluLookAt(0.0,0.0,1.0, 
              0.0,0.0,0.0,
              0.0,1.0,0.0);

}


