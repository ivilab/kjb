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

Triangular_mesh create_cylinder_polymesh(std::vector<Polygon>& faces)
{
    Polygon p1a(3);
    p1a.add_point(1.0000, 0.0000, 0);
    p1a.add_point(0.9511, 0.3090, 0);
    p1a.add_point(1.0000, 0.0000, 1);

    Polygon p1b(3);
    p1b.add_point(0.9511, 0.3090, 0);
    p1b.add_point(1.0000, 0.0000, 1);
    p1b.add_point(0.9511, 0.3090, 1);

    Polygon p2a(3);
    p2a.add_point(0.9511, 0.3090, 0);
    p2a.add_point(0.8090, 0.5878, 0);
    p2a.add_point(0.9511, 0.3090, 1);

    Polygon p2b(3);
    p2b.add_point(0.8090, 0.5878, 0);
    p2b.add_point(0.9511, 0.3090, 1);
    p2b.add_point(0.8090, 0.5878, 1);

    Polygon p3a(3);
    p3a.add_point(0.8090, 0.5878, 0);
    p3a.add_point(0.5878, 0.8090, 0);
    p3a.add_point(0.8090, 0.5878, 1);

    Polygon p3b(3);
    p3b.add_point(0.5878, 0.8090, 0);
    p3b.add_point(0.8090, 0.5878, 1);
    p3b.add_point(0.5878, 0.8090, 1);

    Polygon p4a(3);
    p4a.add_point(0.5878, 0.8090, 0);
    p4a.add_point(0.3090, 0.9511, 0);
    p4a.add_point(0.5878, 0.8090, 1);

    Polygon p4b(3);
    p4b.add_point(0.3090, 0.9511, 0);
    p4b.add_point(0.5878, 0.8090, 1);
    p4b.add_point(0.3090, 0.9511, 1);

    Polygon p5a(3);
    p5a.add_point(0.3090, 0.9511, 0);
    p5a.add_point(0.0000, 1.0000, 0);
    p5a.add_point(0.3090, 0.9511, 1);

    Polygon p5b(3);
    p5b.add_point(0.0000, 1.0000, 0);
    p5b.add_point(0.3090, 0.9511, 1);
    p5b.add_point(0.0000, 1.0000, 1);

    Polygon p6a(3);
    p6a.add_point(0.0000, 1.0000, 0);
    p6a.add_point(-.3090, 0.9511, 0);
    p6a.add_point(0.0000, 1.0000, 1);

    Polygon p6b(3);
    p6b.add_point(-.3090, 0.9511, 0);
    p6b.add_point(0.0000, 1.0000, 1);
    p6b.add_point(-.3090, 0.9511, 1);

    Polygon p7a(3);
    p7a.add_point(-.3090, 0.9511, 0);
    p7a.add_point(-.5878, 0.8090, 0);
    p7a.add_point(-.3090, 0.9511, 1);

    Polygon p7b(3);
    p7b.add_point(-.5878, 0.8090, 0);
    p7b.add_point(-.3090, 0.9511, 1);
    p7b.add_point(-.5878, 0.8090, 1);

    Polygon p8a(3);
    p8a.add_point(-.5878, 0.8090, 0);
    p8a.add_point(-.8090, 0.5878, 0);
    p8a.add_point(-.5878, 0.8090, 1);

    Polygon p8b(3);
    p8b.add_point(-.8090, 0.5878, 0);
    p8b.add_point(-.5878, 0.8090, 1);
    p8b.add_point(-.8090, 0.5878, 1);

    Polygon p9a(3);
    p9a.add_point(-.8090, 0.5878, 0);
    p9a.add_point(-.9511, 0.3090, 0);
    p9a.add_point(-.8090, 0.5878, 1);

    Polygon p9b(3);
    p9b.add_point(-.9511, 0.3090, 0);
    p9b.add_point(-.8090, 0.5878, 1);
    p9b.add_point(-.9511, 0.3090, 1);

    Polygon p10a(3);
    p10a.add_point(-.9511, 0.3090, 0);
    p10a.add_point(-1.000, 0.0000, 0);
    p10a.add_point(-.9511, 0.3090, 1);

    Polygon p10b(3);
    p10b.add_point(-1.000, 0.0000, 0);
    p10b.add_point(-.9511, 0.3090, 1);
    p10b.add_point(-1.000, 0.0000, 1);

    Polygon p11a(3);
    p11a.add_point(-1.000, 0.0000, 0);
    p11a.add_point(-.9511, -.3090, 0);
    p11a.add_point(-1.000, 0.0000, 1);

    Polygon p11b(3);
    p11b.add_point(-.9511, -.3090, 0);
    p11b.add_point(-1.000, 0.0000, 1);
    p11b.add_point(-.9511, -.3090, 1);
    
    Polygon p12a(3);
    p12a.add_point(-.9511, -.3090, 0);
    p12a.add_point(-.8090, -.5878, 0);
    p12a.add_point(-.9511, -.3090, 1);

    Polygon p12b(3);
    p12b.add_point(-.8090, -.5878, 0);
    p12b.add_point(-.9511, -.3090, 1);
    p12b.add_point(-.8090, -.5878, 1);

    Polygon p13a(3);
    p13a.add_point(-.8090, -.5878, 0);
    p13a.add_point(-.5878, -.8090, 0);
    p13a.add_point(-.8090, -.5878, 1);

    Polygon p13b(3);
    p13b.add_point(-.5878, -.8090, 0);
    p13b.add_point(-.8090, -.5878, 1);
    p13b.add_point(-.5878, -.8090, 1);

    Polygon p14a(3);
    p14a.add_point(-.5878, -.8090, 0);
    p14a.add_point(-.3090, -.9511, 0);
    p14a.add_point(-.5878, -.8090, 1);

    Polygon p14b(3);
    p14b.add_point(-.3090, -.9511, 0);
    p14b.add_point(-.5878, -.8090, 1);
    p14b.add_point(-.3090, -.9511, 1);

    Polygon p15a(3);
    p15a.add_point(-.3090, -.9511, 0);
    p15a.add_point(-0.000, -1.000, 0);
    p15a.add_point(-.3090, -.9511, 1);

    Polygon p15b(3);
    p15b.add_point(-0.000, -1.000, 0);
    p15b.add_point(-.3090, -.9511, 1);
    p15b.add_point(-0.000, -1.000, 1);

    Polygon p16a(3);
    p16a.add_point(-0.000, -1.000, 0);
    p16a.add_point(0.3090, -.9511, 0);
    p16a.add_point(-0.000, -1.000, 1);

    Polygon p16b(3);
    p16b.add_point(0.3090, -.9511, 0);
    p16b.add_point(-0.000, -1.000, 1);
    p16b.add_point(0.3090, -.9511, 1);

    Polygon p17a(3);
    p17a.add_point(0.3090, -.9511, 0);
    p17a.add_point(0.5878, -.8090, 0);
    p17a.add_point(0.3090, -.9511, 1);

    Polygon p17b(3);
    p17b.add_point(0.5878, -.8090, 0);
    p17b.add_point(0.3090, -.9511, 1);
    p17b.add_point(0.5878, -.8090, 1);

    Polygon p18a(3);
    p18a.add_point(0.5878, -.8090, 0);
    p18a.add_point(0.8090, -.5878, 0);
    p18a.add_point(0.5878, -.8090, 1);

    Polygon p18b(3);
    p18b.add_point(0.8090, -.5878, 0);
    p18b.add_point(0.5878, -.8090, 1);
    p18b.add_point(0.8090, -.5878, 1);

    Polygon p19a(3);
    p19a.add_point(0.8090, -.5878, 0);
    p19a.add_point(0.9511, -.3090, 0);
    p19a.add_point(0.8090, -.5878, 1);

    Polygon p19b(3);
    p19b.add_point(0.9511, -.3090, 0);
    p19b.add_point(0.8090, -.5878, 1);
    p19b.add_point(0.9511, -.3090, 1);

    Polygon p20a(3);
    p20a.add_point(0.9511, -.3090, 0);
    p20a.add_point(1.0000, 0.0000, 0);
    p20a.add_point(0.9511, -.3090, 1);

    Polygon p20b(3);
    p20b.add_point(1.0000, 0.0000, 0);
    p20b.add_point(0.9511, -.3090, 1);
    p20b.add_point(1.0000, 0.0000, 1);


    Triangular_mesh cylmesh;
    cylmesh.add_face(p1a);
    cylmesh.add_face(p2a);
    cylmesh.add_face(p3a);
    cylmesh.add_face(p4a);
    cylmesh.add_face(p5a);
    cylmesh.add_face(p6a);
    cylmesh.add_face(p7a);
    cylmesh.add_face(p8a);
    cylmesh.add_face(p9a);
    cylmesh.add_face(p10a);
    cylmesh.add_face(p11a);
    cylmesh.add_face(p12a);
    cylmesh.add_face(p13a);
    cylmesh.add_face(p14a);
    cylmesh.add_face(p15a);
    cylmesh.add_face(p16a);
    cylmesh.add_face(p17a);
    cylmesh.add_face(p18a);
    cylmesh.add_face(p19a);
    cylmesh.add_face(p20a);

    cylmesh.add_face(p1b);
    cylmesh.add_face(p2b);
    cylmesh.add_face(p3b);
    cylmesh.add_face(p4b);
    cylmesh.add_face(p5b);
    cylmesh.add_face(p6b);
    cylmesh.add_face(p7b);
    cylmesh.add_face(p8b);
    cylmesh.add_face(p9b);
    cylmesh.add_face(p10b);
    cylmesh.add_face(p11b);
    cylmesh.add_face(p12b);
    cylmesh.add_face(p13b);
    cylmesh.add_face(p14b);
    cylmesh.add_face(p15b);
    cylmesh.add_face(p16b);
    cylmesh.add_face(p17b);
    cylmesh.add_face(p18b);
    cylmesh.add_face(p19b);
    cylmesh.add_face(p20b);

    cylmesh.create_adjacency_matrix();

    faces.push_back(p1a);
    faces.push_back(p2a);
    faces.push_back(p3a);
    faces.push_back(p4a);
    faces.push_back(p5a);
    faces.push_back(p6a);
    faces.push_back(p7a);
    faces.push_back(p8a);
    faces.push_back(p9a);
    faces.push_back(p10a);
    faces.push_back(p11a);
    faces.push_back(p12a);
    faces.push_back(p13a);
    faces.push_back(p14a);
    faces.push_back(p15a);
    faces.push_back(p16a);
    faces.push_back(p17a);
    faces.push_back(p18a);
    faces.push_back(p19a);
    faces.push_back(p20a);

    faces.push_back(p1b);
    faces.push_back(p2b);
    faces.push_back(p3b);
    faces.push_back(p4b);
    faces.push_back(p5b);
    faces.push_back(p6b);
    faces.push_back(p7b);
    faces.push_back(p8b);
    faces.push_back(p9b);
    faces.push_back(p10b);
    faces.push_back(p11b);
    faces.push_back(p12b);
    faces.push_back(p13b);
    faces.push_back(p14b);
    faces.push_back(p15b);
    faces.push_back(p16b);
    faces.push_back(p17b);
    faces.push_back(p18b);
    faces.push_back(p19b);
    faces.push_back(p20b);

    return cylmesh;
}


int main()
{
    std::vector<Polygon> faces;
    Polymesh cylmesh = create_cylinder_polymesh(faces);

    Int_vector rectMask;
    find_right_triangles(faces, rectMask);

    std::vector<Right_Triangle_Pair> rectangles;
    find_rectangles(faces, rectMask, cylmesh, rectangles);

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
    }

}
