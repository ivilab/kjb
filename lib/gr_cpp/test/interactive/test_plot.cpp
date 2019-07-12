/* =========================================================================== *
|
|  Copyright (c) 1994-2008 by Kobus Barnard (author).
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
|  Author: Ernesto Brau
|
* =========================================================================== */

/* $Id: test_plot.cpp 13030 2012-09-26 17:27:19Z ernesto $ */

#include <m_cpp/m_vector.h>
#include <gr_cpp/gr_opengl.h>
#include <gr_cpp/gr_plot.h>
#include <algorithm>
#include <cmath>

using namespace kjb;
using namespace kjb::opengl;

int main(int, char**)
{
    Vector x = create_uniformly_spaced_vector(0.0, 10.0, 100);
    Vector y(x.get_length());
    std::transform(x.begin(), x.end(), y.begin(),
                   static_cast<double(*)(double)>(std::sin));

    Glut_plot_window wnd(800, 500, "sine and cosine and stuff");
    wnd.add_plot(x, y);

    std::transform(x.begin(), x.end(), y.begin(),
                   static_cast<double(*)(double)>(std::cos));
    wnd.add_plot(x, y);

    y = 0.5*x;
    wnd.add_plot(&x, &y);

    glutMainLoop();
}

