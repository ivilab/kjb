/* $Id: test_graph.cpp 11267 2011-12-03 01:47:55Z ksimek $ */
/* {{{=========================================================================== *
   |
   |  Copyright (c) 1994-2011 by Kobus Barnard (author)
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

#include <iostream>
using namespace std;

#ifndef KJB_HAVEGLUT_
int main(int argc, char** argv)
{
    cout << "opengl not installed" << endl;
    return 0;
}
#else
#include <gui_cpp/gui_graph.h>
#include <gui_cpp/gui_viewer.h>
#include <m_cpp/m_vector.h>

#include <functional>
#include <l_cpp/l_algorithm.h>

#include <boost/iterator/counting_iterator.hpp>


using namespace kjb;
using namespace kjb::gui;

float x_offset = 0;
float y_offset = 0;
float width = 600;
float height = 400;
Plot plot(x_offset, y_offset, width, height); // specify geometry of overlay

static const size_t N = 100;
Vector x(N);

static const size_t N_LARGE = 2 * N;
Vector x_large(N_LARGE);

float t = 0;
float t_incr = 0.01;

Plot::Data_set_iterator cos_dataset;

void update_plots()
{
    static int i = 0;
    static int i_incr = 1;

    Vector z(x_large.size());
    std::transform(
            x_large.begin(),
            x_large.end(),
            z.begin(),
            boost::bind(static_cast<double (*)(double)>(cos),
                boost::bind(plus<float>(), t, _1)));

    // update using iterator returned when constructed
    plot.update_dataset(
            cos_dataset,
            x_large.begin(),
            x_large.begin() + x_large.size() - N + i,
            z.begin());


    t += t_incr;
    i += i_incr;

    if(i >= N)
        i_incr = -1;
    else if(i <= 0)
        i_incr =  1;
}

int main(int , char** )
{
    try
    {
        // initialize input values in [0,2*PI]
        linspace(0, 2*M_PI, x.size(), x.begin());
        linspace(0, 4*M_PI, x_large.size(), x_large.begin());
        
        // sin function
        Vector y(x.size());
        std::transform(
                x.begin(),
                x.end(),
                y.begin(),
                boost::bind(static_cast<double (*)(double)>(sin), _1));

        // linear function
        Vector z(
            boost::make_counting_iterator(0),
            boost::make_counting_iterator(7));



        // Approach 1: Initialize from input and output iterators
        plot.add_dataset(x.begin(), x.end(), y.begin());

        // Approach 2: Initialized from output only.  Input values will be 1, 2, 3, ...
        plot.add_dataset(z.begin(), z.end());

        // Approach 3: Create an empty data set that we'll edit later
        cos_dataset = plot.add_dataset();

        // add labels
        plot.title("sin(x), cos(x), and exp(x)");
        plot.xlabel("Inputs");
        plot.ylabel("Output");

        // axes are automatically set by default, but you
        // can specify them manually, too.
//         plot.axis(-2.17, 10.24, -0.5, 0.5);

        // construct a viewer
        Viewer viewer(width, height);

        // attach the graph
        viewer.add_overlay(&plot);

        // add_overlay() can receive pointer or shared_pointer
        // Alternatively, you can copy an overlay into the viewer using
        // viewer.copy_overlay(plot).  Copy will be destroyed when viewer's
        // destructor is called.
        
        // add animation callback
        size_t msec = 25;
        bool redraw = true;
        viewer.add_timer(update_plots, msec, redraw);

        // test corner case for auto-scaling: horizontal lines
        Plot plot2(0, 0, width, height);
        Vector constant(101., 101.);
        plot2.add_dataset(constant.begin(), constant.end());

        Viewer viewer_2(width, height);
        viewer_2.copy_overlay(plot2);

        glutMainLoop();
    }
    catch(Exception& ex)
    {
        ex.print_details_abort();
    }
    catch(exception& ex)
    {
        cerr << ex.what() << endl;
        abort();
    }
    return 0;
}
#endif // have_glut
