/* $Id: test_viewer.cpp 12001 2012-03-29 23:34:30Z ksimek $ */
/* {{{=========================================================================== *
   |
   |  Copyright (c) 1994-2012 by Kobus Barnard (author)
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
 * =========================================================================== }}}*/

// vim: tabstop=4 shiftwidth=4 foldmethod=marker

#include <gr_cpp/gr_glut.h>
#include <gui_cpp/gui_viewer.h>

void render_teapot_2()
{
    glPushMatrix();
    glTranslatef(-3, 0, 0);
    kjb::opengl::Teapot().render();
    glPopMatrix();
}

int main()
{
    kjb::opengl::Glut_window wnd;
    kjb::gui::Viewer viewer(800, 600);
    viewer.attach(wnd);

    kjb::Perspective_camera cam(1, 1000);
    cam.set_camera_centre(kjb::Vector(0, -3, 10));
    kjb::opengl::Teapot teapot;
    viewer.add_renderable(&teapot);
    viewer.add_render_callback(render_teapot_2);

    viewer.set_rotation_origin(kjb::Vector3(-3, 0, 0));
    viewer.set_camera(cam);


    glutMainLoop();
}
