/* $Id: test_von_mises_fisher.cpp 17739 2014-10-14 12:14:54Z ernesto $ */
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

#include <prob_cpp/prob_distribution.h>
#include <prob_cpp/prob_sample.h>
#include <prob_cpp/prob_pdf.h>
#include <vector>
#include <iterator>

#include <gr_cpp/gr_opengl.h>
#include <gr_cpp/gr_glut.h>
#include <gr_cpp/gr_primitive.h>
#include <gui_cpp/gui_viewer.h>

static const size_t D = 3;
std::vector<kjb::Vector_d<D> > samples;

void display()
{
    glDisable(GL_LIGHTING);

    glColor3f(0.3, 0.3, 0.3);
    kjb::opengl::Sphere().wire_render();

    glBegin(GL_POINTS);
    glColor3f(0.0, 0.7, 0.0);
    for(size_t i = 0; i < samples.size(); ++i)
    {
        kjb::opengl::glVertex(samples[i]);
    }
    glEnd();
}

using namespace kjb;
int main(int argc, char** argv)
{
    static const size_t N = 10000;

//    kjb::Vector_d<D> mu = kjb::sample(Uniform_sphere_distribution<D>());
    kjb::Vector_d<D> mu = kjb::create_unit_vector<D>(D-1);
    double kappa = 50;

    kjb::Von_mises_fisher_distribution<D> dist(mu, kappa);
    kjb::sample(dist, N, std::back_inserter(samples));

    kjb::opengl::Glut_window wnd;
    kjb::gui::Viewer viewer(500, 500);
    viewer.attach(wnd);

    viewer.add_render_callback(display);
    glutMainLoop();

    return 0;
}
