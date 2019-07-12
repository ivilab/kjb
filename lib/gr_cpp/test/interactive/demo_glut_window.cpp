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

#include <gr_cpp/gr_sprite.h>
#include <gr_cpp/gr_glut.h>
#include <unistd.h>

#include <string>
#include <iostream>

using namespace std;
using namespace kjb::opengl;


float x = 0;
float y = 0;
float dx = 9.0/10;
float dy = 13.0/5.0;
float sx = 1;
float sy = 1;
float theta = 0;
float dtheta = 0.05;

kjb::opengl::Sprite smile;

const int HEIGHT = 300;
const int WIDTH = 300;

void display()
{
    glClearColor(1.0, 1.0, 1.0, 1.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);   

    x += dx;
    y += dy;
    if(y > HEIGHT || y < 0) dy *= -1;
    if(x > WIDTH || x < 0) dx *= -1;

    sx = sy = 1.0 + 0.5 * sin(theta);

    theta += dtheta;
    while(theta > 2 * M_PI)
        theta -= 2 * M_PI;


    smile.set_position(x, y);
    smile.set_scale(sx, sy);
    smile.render();

    glutSwapBuffers(); 
}

void idle()
{
    usleep(5000);
    glutPostRedisplay();

}

void key_handler(unsigned char k, int x, int y)
{
    if(k == 'q')
        exit(0);
}

int main()
{
    kjb::opengl::Glut::set_init_window_size(WIDTH, HEIGHT);
    Glut_window wnd;
    wnd.set_size(WIDTH, HEIGHT);

    // set up callbacks
    wnd.set_display_callback(display);
    wnd.set_keyboard_callback(key_handler);
    wnd.set_idle_callback(idle);


    // Initialize displayables
//    try{
        smile = Sprite(kjb::Image(string("input/smile.png")));
//    }
//    catch(Opengl_error& err)
//    {
//        cerr << err.get_msg() << endl;
//        abort();
//    }

    // optional calls to make it prettier
    smile.center_origin();
    smile.set_filters(GL_LINEAR, GL_LINEAR);

    glutMainLoop();

    return 0;
}

