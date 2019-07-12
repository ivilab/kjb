/* $Id: plot.cpp 11530 2012-01-25 17:39:14Z ksimek $ */
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

#include <string>
#include <vector>

#include <gui_cpp/gui_graph.h>
#include <gui_cpp/gui_viewer.h>
using namespace std;
using namespace kjb::gui;

/**
 * A simple command-line utility that that recieves file of data columns in stdin and plots them.
 *
 * File format is an x-value, followed by one or more y-values.
 * For example:
 *  x y1 y2 ... yn
 *  x y1 y2 ... yn
 *  ...
 *
 * This will draw n plots, corresponding to outputs y1...yn
 *
 * Points do not need to be in order.
 *
 * The example below plots four points from f(x) = x^2 and f(x) = x^3:
 * $ echo "0 0 0" > data
 * $ echo "1 1 1" > data
 * $ echo "2 4 16" > data
 * $ echo "3 9 27" > data
 * $ cat data | plot
 * $ # alternatively...
 * $ plot < data
 */
int main()
{
    vector<double> x;
    vector<vector<double> > y;

    string line;
    bool first_line = true;

    double nr = 0;
    while(getline(cin, line))
    {
        istringstream ist(line);
        double cur_x;
        ist >> cur_x;
        x.push_back(cur_x);

        int col = 0;
        while(ist)
        {
            if(first_line)
                y.resize(col+1);
            y.at(col).resize(nr+1);
            ist >> y.at(col).back();
            col++;
            ist.peek();
        }

        first_line = false;
        nr++;
    }

    Plot plot(0, 0, 400, 300);

    for(int col = 0; col < y.size(); col++)
    {
        plot.add_dataset(x.begin(), x.end(), y[col].begin());
    }

    Viewer viewer(400, 300);
    viewer.attach_overlay(&plot);

    glutMainLoop();
    return 0;
}
