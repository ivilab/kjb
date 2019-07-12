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

#include <gr_cpp/gr_find_shapes.h>
#include <gr_cpp/gr_polygon.h>
#include <gr_cpp/gr_polymesh.h>
#include <gr_cpp/gr_polymesh_plane.h>
#include <gr_cpp/gr_parapiped.h>
#include <gr_cpp/gr_triangular_mesh.h>

#include <vector>
#include <time.h>

using namespace kjb;

Triangular_mesh * tm = NULL;
char* fileName = NULL;

static std::vector<std::vector<Vector> > points;


void test_parapiped()
{
    // create a parapiped polymesh.
    Parapiped pp(0,0,0,1,0,0,1,1,0,1,1,1);


    // Test Polymesh class get_all_edges() method

    // polymesh is a cube.
//    const Polymesh& polymesh = pp;
    std::vector<std::vector<Vector> > edges;

    clock_t clock1 = clock();
    pp.get_all_edges(edges);
    clock_t clock2 = clock();
    std::cout << "Time (find edges in cube): " << (clock2 - clock1) << std::endl;


    for(unsigned int i = 0; i < edges.size(); i++)
    {
        std::cout << "(" << edges[i][0](0) << ", " 
                         << edges[i][0](1) << ", " 
                         << edges[i][0](2) << ") and (" 
                         << edges[i][1](0) << ", " 
                         << edges[i][1](1) << ", " 
                         << edges[i][1](2) << ")\n";
    }
    
    std::cout << std::endl;


    // This adds a smaller rectangle onto the polymesh cube.
    Polygon p1(4);
    p1.add_point(1,0,1);
    p1.add_point(2,0,1);
    p1.add_point(2,0.5,1);
    p1.add_point(1,0.5,1);
    
    Polygon p2(4);
    p2.add_point(2,0,0);
    p2.add_point(2,0,1);
    p2.add_point(2,0.5,1);
    p2.add_point(2,0.5,0);

/*    Polygon p3(4);
    p3.add_point(1,0,1);
    p3.add_point(1,0,0);
    p3.add_point(1,0.5,0);
    p3.add_point(1,0.5,1);
*/
    Polygon p4(4);
    p4.add_point(1,0,1);
    p4.add_point(2,0,1);
    p4.add_point(1,0,0);
    p4.add_point(2,0,0);

    Polygon p5(4);
    p5.add_point(1,0.5,0);
    p5.add_point(2,0.5,0);
    p5.add_point(2,0.5,1);
    p5.add_point(1,0.5,1);

    Polygon p6(4);
    p6.add_point(1,0,0);
    p6.add_point(2,0,0);
    p6.add_point(2,0.5,0);
    p6.add_point(1,0.5,0);

    Polymesh polymesh1 = pp;

    polymesh1.add_face(p1);
    polymesh1.add_face(p2);
//    polymesh.add_face(p3);
    polymesh1.add_face(p4);
    polymesh1.add_face(p5);
    polymesh1.add_face(p6);

    const Polymesh& polymesh2 = polymesh1;
    std::vector<std::vector<Vector> > edges2;

    clock1 = clock();
    polymesh2.get_all_edges(edges2);
    clock2 = clock();
    std::cout << "Time (find edges in cube2): " << (clock2 - clock1) 
                                                << std::endl;

    for(unsigned int i = 0; i < edges2.size(); i++)
    {
        std::cout << "(" << edges2[i][0](0) << ", " 
                         << edges2[i][0](1) << ", " 
                         << edges2[i][0](2) << ") and (" 
                         << edges2[i][1](0) << ", " 
                         << edges2[i][1](1) << ", " 
                         << edges2[i][1](2) << ")\n";
    }
}

void test_polymesh_from_file(char* fname)
{
    // Test Polymesh class get_all_edges() method
    try{
        tm = new Triangular_mesh(fname);
    } catch(KJB_error e)
    {
        e.print(std::cout);
    }

    (*tm).set_adjacency_matrix(fileName);

    Polymesh & tpm = *tm;

    std::vector<std::vector<Vector> > edges;

    clock_t clock1 = clock();
    tpm.get_all_edges(edges);
    clock_t clock2 = clock();
    std::cout << "Time (find edges in cube): " << (clock2 - clock1) << std::endl;


    for(unsigned int i = 0; i < edges.size(); i++)
    {
        std::cout << "(" << edges[i][0](0) << ", " 
                         << edges[i][0](1) << ", " 
                         << edges[i][0](2) << ") and (" 
                         << edges[i][1](0) << ", " 
                         << edges[i][1](1) << ", " 
                         << edges[i][1](2) << ")\n";
    }
}



int main(int argc, char* argv[])
{
    std::cout << "TEST_PARAPIPED():" << std::endl;
    test_parapiped();
    std::cout << std::endl;

    std::cout << "TEST_POLYMESH_FROM_FILE():\n";
    fileName = argv[1];
    test_polymesh_from_file(fileName);
    std::cout << std::endl;
}
