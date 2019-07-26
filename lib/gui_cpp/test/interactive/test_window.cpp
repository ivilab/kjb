/* $Id: test_window.cpp 11267 2011-12-03 01:47:55Z ksimek $ */
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

#ifndef KJB_HAVE_GLUT
#include <iostream>
using namespace std;
int main()
{
    cout << "glut not installed" << endl;
}

#else
#include <gui_cpp/gui_window.h>
#include <gui_cpp/gui_viewer.h>
#include <gui_cpp/gui_graph.h>
#include <l_cpp/l_algorithm.h>
#include <vector>

using namespace kjb;
using namespace kjb::gui;

int main()
{
    try {
        kjb::gui::Viewer viewer(800,600);

        std::vector<double> x(50);
        std::vector<double> y(50);

        kjb::linspace(0, 2 * M_PI, 50, x.begin());

        for(size_t i = 0; i < x.size(); i++)
        {
            y[i] = sin(x[i]);
        }

        Window graph_window(100, 100, 400, 300);
        Plot plot(0, 0, 800, 600);
        plot.add_dataset(x.begin(), x.end(), y.begin());
        plot.title("Sin(x)");

        Window graph_window2(100, 100, 400, 300);
        Plot plot2(0, 0, 800, 600);
        plot2.add_dataset(x.begin(), x.end(), y.rbegin());
        plot2.title("Sin(2pi - x)");

        graph_window.attach(&plot);
        graph_window2.attach(&plot2);

        viewer.add_window(&graph_window);
        viewer.add_window(&graph_window2);
//        viewer.add_overlay(&graph_window);
//        viewer.add_event_listener(&graph_window);

        viewer.remove_window(&graph_window2);

        // should move plot 2 to top of stack
        viewer.add_window(&graph_window2);
        // should have no effect
        viewer.add_window(&graph_window2);

        // should move plot 1 to top of stack
        viewer.add_window(&graph_window);
        glutMainLoop();
    }
    catch (Exception& ex)
    {
        std::cout << ex.get_msg() << std::endl;
        abort();
    }
    catch (std::exception& ex)
    {
        std::cout << ex.what() << std::endl;
        abort();
    }

    return 0;
}
#endif // HAVE_GLUT
