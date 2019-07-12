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


/*-----------------------------------------------------------------------------
 *  This program tests the Gl_sprite class
 *-----------------------------------------------------------------------------*/


#include    <stdlib.h>

#include <gr_cpp/gr_sprite.h>
#include <gr_cpp/gr_glut.h>
#include <boost/bind.hpp>

#include <string>

using namespace std;

using namespace kjb;
using namespace kjb::opengl;

void draw();
void reshape(int width, int height);

Sprite* sprite;
int main (int argc, char *argv[])
{
    Glut_window wnd(320, 320, "Sprite test");
    wnd.set_display_callback(draw);
    wnd.set_reshape_callback(boost::bind(reshape, _1, _2));

    sprite = new Sprite(Image(string("test.jpg")));
    glutMainLoop();

    return EXIT_SUCCESS;
}


void draw()
{
    glClearColor(1.0, 1.0, 1.0, 0.0);

	/* clear the drawing buffer */
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);        

    sprite->set_position(0,0);
    sprite->set_scale(1,1);
    sprite->render();

    sprite->set_u(sprite->get_width());
    sprite->set_scale(2,2);
    sprite->render();

    sprite->set_u(sprite->get_u() + sprite->get_width());
    sprite->set_scale(4,4);
    sprite->render();

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
    glOrtho(-.1,1.1, -.1, 1.1, -1.0, 1.0);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    gluLookAt(0.0,0.0,1.0, 
              0.0,0.0,0.0,
              0.0,1.0,0.0);

}
/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */
