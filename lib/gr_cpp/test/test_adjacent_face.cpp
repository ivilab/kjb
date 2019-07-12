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
#include <gr_cpp/gr_triangular_mesh.h>

#include <vector>

using namespace kjb;

Triangular_mesh * tm = NULL;

static std::vector<Polymesh_Plane> planes;

int main(int argc, char* argv[])
{
    tm = new Triangular_mesh(argv[1]);
    (*tm).create_adjacency_matrix();

    Polymesh & tpm = *tm;

    std::vector<Polygon>faces = tpm.get_faces();
    int num_faces = tpm.num_faces();

    std::vector<int> count_adj(num_faces, 0);
    int count_adj_problems = 0;

    for(int f = 0; f < num_faces; f++)
    {
        int num_pts = faces[f].get_num_points();
        if(num_pts != 3)
        {
            std::cout << "Face " << f << " has " << num_pts << " points!\n";
        }
        else
        {
            for(int e = 0; e < num_pts; e++)
            {
                int adj_face = (*tm).adjacent_face(f, e);


                int f0 = (*tm).adjacent_face(adj_face, 0);
                int f1 = (*tm).adjacent_face(adj_face, 1);
                int f2 = (*tm).adjacent_face(adj_face, 2);

                if((f != f0) && (f != f1) && (f != f2))
                {
                    std::cout << "ERROR: face " << f 
                              << " is adjacent to face " << adj_face 
                              << ", but not vice versa!" << std::endl;
                    count_adj_problems++;
                }

                count_adj[adj_face] += 1;
            }
        }
    }

    std::cout << "\n# of adjacency inconsistencies: " << count_adj_problems 
              << std::endl;
    std::cout << "\nFinished creating count array!\n" << std::endl;

    int count = 0;

    for(int f = 0; f < num_faces; f++)
    {
        if(count_adj[f] != 3)
        {
            std::cout << "Face " << f << " is adjacent to " << count_adj[f] 
                      << " other faces!" << std::endl;
            count++;
        }
    }

    std::cout << "Total number of faces: " << num_faces << std::endl;
    std::cout << "Number of faces with incorrect adjacencies: " << count 
              << std::endl;

//    find_adjacent_right_triangles_to_render(tpm, planes);
}
