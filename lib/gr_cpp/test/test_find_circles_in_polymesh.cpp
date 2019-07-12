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
 |  Author:  Emily Hartley
 * =========================================================================== */

#include <iostream>
#include <fstream>

#include <gr_cpp/gr_find_shapes.h>
#include <gr_cpp/gr_polygon.h>
#include <gr_cpp/gr_polymesh.h>
#include <gr_cpp/gr_polymesh_plane.h>
#include <gr_cpp/gr_triangular_mesh.h>

#include <vector>
#include <time.h>

using namespace kjb;

Triangular_mesh * tm = NULL;

static std::vector<Circle_in_3d> circles;
static std::vector<std::vector<Vector> > points;
char* fileName = NULL;

int main(int argc, char* argv[])
{
    try{
        tm = new Triangular_mesh(argv[1]);
        fileName = argv[1];
    } catch(KJB_error e)
    {
        e.print(std::cout);
    }

    (*tm).set_adjacency_matrix(fileName);
    Polymesh & tpm = *tm;

    clock_t clock1 = clock();
    
    find_all_circles_in_polymesh(tpm, circles, points);
    
    clock_t clock2 = clock();
    std::cout << "Time finding circles in polymesh: " 
              << (clock2 - clock1) << std::endl;

    std::ofstream ofs("tmp_airVent_circleParams");
    if(!ofs.is_open())
    {
        KJB_THROW_2(IO_error, "Could not open file for writing circles");
    }

    ofs << "Number of circles: " << circles.size() << "\n";

    for(int i = 0; i < circles.size(); i++)
    {
        circles[i].write(ofs);
        ofs << "\n";
    }
    ofs.close();
}
