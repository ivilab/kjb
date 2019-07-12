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
#include <gr_cpp/gr_triangular_mesh.h>
#include <gr_cpp/gr_polymesh_plane.h>
#include <gr_cpp/gr_parapiped.h>

#include <vector>

using namespace kjb;

//Triangular_mesh * tm = NULL;

int main(int argc, char* argv[])
{
    try
    {
        //tm = new Triangular_mesh(argv[1]);
        Triangular_mesh tm(argv[1]);
        //(*tm).create_adjacency_matrix();
        tm.create_adjacency_matrix();

//      Polymesh & cylmesh = *tm;
        Polymesh & cylmesh = tm;

        std::vector<Polygon> faces = cylmesh.get_faces();

        Int_vector triMask;
        find_right_triangles(faces, triMask);

        std::vector<Right_Triangle_Pair> rectangles;
        find_rectangles(faces, triMask, cylmesh, rectangles);

        Int_vector rectMask; 
        create_rectangle_mask(cylmesh, rectangles, rectMask);
        

        std::vector<std::vector<int> > cyl_indices;
        std::vector<double> cylSumAngles;
        std::vector<std::vector<Vector> > cylEdgePoints;
        find_cylinders(cylmesh, rectangles, cyl_indices, 
                       cylSumAngles, cylEdgePoints);

        std::cout << "cyl_indices size = " << cyl_indices.size() << std::endl;

        for(unsigned int i = 0; i < cyl_indices.size(); i++)
        {
            Cylinder_section cyl(Vector(0,0,0), Vector(1,0,0), 1, 0, 
                                 Vector(1,1,1), Vector(1,1,1));
            fit_cylinder(cylmesh, rectMask, rectangles, cyl_indices[i], 
                         cylSumAngles[i], cylEdgePoints[i], cyl);
            std::cout << "Cylinder " << i << std::endl;
            std::cout << "  p1: " << cyl.get_p1() << std::endl;
            std::cout << "  p2: " << cyl.get_p2() << std::endl;
            std::cout << "  radius: " << cyl.get_radius() << std::endl;
            std::cout << std::endl;
        }
    } 
    catch(KJB_error e)
    {
        e.print(std::cout);
    }

}
