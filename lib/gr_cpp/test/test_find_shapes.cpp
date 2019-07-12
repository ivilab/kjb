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

#include <vector>

using namespace kjb;


void test_parapiped()
{
    // create a parapiped polymesh.
    Parapiped pp(0,0,0,1,0,0,1,1,0,1,1,1);


    // Test Polymesh_Plane class and find_planes()

    // polymesh is a cube.
    const Polymesh& polymesh = pp;
    std::vector<Polymesh_Plane> planeVec;

    find_planes(polymesh, planeVec);

    for(unsigned int i = 0; i < planeVec.size(); i++)
    {
        std::vector<int> indices = planeVec[i].get_face_indices();

        std::cout << "Plane " << i << " indices: " << indices[0];
        for(unsigned int j = 1; j < indices.size(); j++)
        {
            std::cout << ", " << indices[j];
        }
        std::cout << std::endl;

        Vector planeParams = planeVec[i].get_plane_params();

        std::cout << "Plane " << i << " parameters: (" 
                  << planeParams(0) << ", " 
                  << planeParams(1) << ", " 
                  << planeParams(2) << ", " 
                  << planeParams(3) << ")" << std::endl;

    }
    
    std::cout << std::endl;


    // This adds a smaller rectangle onto the polymesh cube.
    Polygon p1(4);
    p1.add_point(1,0,1);
    p1.add_point(2,0,1);
    p1.add_point(2,0.5,1);
    p1.add_point(1,0.5,1);
    
    int longest0 = p1.get_index_of_longest_edge();
    std::cout << "longest edge: '" << longest0 << "' - expect 0\n" << std::endl;

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

    std::vector<Polymesh_Plane> planeVec2;
    find_planes(polymesh2, planeVec2);

    for(unsigned int i = 0; i < planeVec2.size(); i++)
    {
        std::vector<int> indices = planeVec2[i].get_face_indices();

        std::cout << "Plane " << i << " indices: " << indices[0];
        for(unsigned int j = 1; j < indices.size(); j++)
        {
            std::cout << ", " << indices[j];
        }
        std::cout << std::endl;

        Vector planeParams = planeVec2[i].get_plane_params();

        std::cout << "Plane " << i << " parameters: (" 
                  << planeParams(0) << ", " 
                  << planeParams(1) << ", " 
                  << planeParams(2) << ", " 
                  << planeParams(3) << ")" << std::endl;

    }
}

void test_triangular_polygons()
{
    // Test check_polygon_is_right_triangle on triangular polygons
    // Test get_index_of_longest_edge().

    // A right triangle
    Polygon t1(3);
    t1.add_point(0,0,0);
    t1.add_point(1,1,0);
    t1.add_point(2,0,0);

    // A right triangle
    Polygon t2(3);
    t2.add_point(0,0,0);
    t2.add_point(1,0,0);
    t2.add_point(0,2,0);

    // Not a right trianlge
    Polygon t3(3);
    t3.add_point(0,0,0);
    t3.add_point(1,2,0);
    t3.add_point(2,1,0);

    if(t1.check_polygon_is_right_triangle(0.0005))
    {
        std::cout << "t1 is a right triangle" << std::endl;
    }
    else
    {
        std::cout << "t1 is NOT a right triangle" << std::endl;
    }

    int longest1 = t1.get_index_of_longest_edge();
    std::cout << "longest edge: '" << longest1 << "' - expect 2\n" << std::endl;

    if(t2.check_polygon_is_right_triangle(0.0005))
    {
        std::cout << "t2 is a right triangle" << std::endl;
    }
    else
    {
        std::cout << "t2 is NOT a right triangle" << std::endl;
    }

    int longest2 = t2.get_index_of_longest_edge();
    std::cout << "longest edge: '" << longest2 << "' - expect 1\n" << std::endl;

    if(t3.check_polygon_is_right_triangle(0.0005))
    {
        std::cout << "t3 is a right triangle" << std::endl;
    }
    else
    {
        std::cout << "t3 is NOT a right triangle" << std::endl;
    }

    int longest3 = t3.get_index_of_longest_edge();
    std::cout << "longest edge: '" << longest3 << "' - expect 2\n" << std::endl;
}

void test_find_right_triangles()
{
    // Test find_right_triangles

    // Not right triangle
    Polygon tr1(3);
    tr1.add_point(0,0,0);
    tr1.add_point(1,1,0);
    tr1.add_point(0,1.5,0);

    // right triangle
    Polygon tr2(3);
    tr2.add_point(0,0,0);
    tr2.add_point(1,1,0);
    tr2.add_point(2,0,0);

    // right triangle
    Polygon tr3(3);
    tr3.add_point(1,1,0);
    tr3.add_point(2,0,0);
    tr3.add_point(3,1,0);

    // right triangle
    Polygon tr4(3);
    tr4.add_point(2,0,0);
    tr4.add_point(3,1,0);
    tr4.add_point(3,0,0);

    // right triangle
    Polygon tr5(3);
    tr5.add_point(3,0,0);
    tr5.add_point(4,0,0);
    tr5.add_point(3,1,0);

    // Not right triangle
    Polygon tr6(3);
    tr6.add_point(4,0,0);
    tr6.add_point(3.5,2,0);
    tr6.add_point(3,1,0);

    // Not right triangle
    Polygon tr7(3);
    tr7.add_point(4,0,0);
    tr7.add_point(3.5,2,0);
    tr7.add_point(5,0,0);

    std::vector<Polygon> faces2;
    faces2.push_back(tr1);
    faces2.push_back(tr2);
    faces2.push_back(tr3);
    faces2.push_back(tr4);
    faces2.push_back(tr5);
    faces2.push_back(tr6);
    faces2.push_back(tr7);

    std::cout << "Faces2 right triangles: ";
    for(unsigned int i = 0; i < faces2.size(); i++)
    {
        if(faces2[i].check_polygon_is_right_triangle(0.0005))
        {
            std::cout << "1 ";
        }
        else
        {
            std::cout << "0 ";
        }
    }
    std::cout << std::endl;

    Int_vector mask(1,0);

    find_right_triangles(faces2, mask);
    
    std::cout << "Size of mask: " << mask.size() << std::endl;

    std::cout << "Mask: ";
    for(int i = 0; i < mask.size(); i++)
    {
        std::cout << mask(i) << " ";
    }
    std::cout << "\n" << std::endl;

    // Not coplanar
    Polygon tr8(3);
    tr8.add_point(0,0,1);
    tr8.add_point(1,1,0);
    tr8.add_point(0,1.5,0);

    // Not coplanar
    Polygon tr9(3);
    tr9.add_point(4,0,0);
    tr9.add_point(3.5,2,0);
    tr9.add_point(5,0,1);

    // coplanar to tr9
    Polygon tr10(3);
    tr10.add_point(4,0,0);
    tr10.add_point(3.5,2,0);
    tr10.add_point(3,0,-1);

    faces2.push_back(tr8);
    faces2.push_back(tr9);
    faces2.push_back(tr10);

    // Test check_if_faces_are_coplanar().
    Vector params1;
    Vector params2;
    for(unsigned int i = 0; i < (faces2.size() - 1); i++)
    {
        faces2[i].fit_plane(params1);
        faces2[i+1].fit_plane(params2);
        // 5 degrees == 0.087266463 radians
        if(check_if_faces_are_coplanar(params1, params2, 0.087266))
        {
            std::cout << "Triangles " << i << " and " << i+1 
                      << " are coplanar" << std::endl;
        }
        else
        {
            std::cout << "Triangles " << i << " and " << i+1 
                      << " are NOT coplanar" << std::endl;
        }
    }
}

void test_find_rectangles()
{
    // Test find_rectangles().

    Polygon tri0(3);
    tri0.add_point(0,0,0);
    tri0.add_point(0,3,0);
    tri0.add_point(1,0,0);

    Polygon tri1(3);
    tri1.add_point(1,3,0);
    tri1.add_point(0,3,0);
    tri1.add_point(1,0,0);

    Polygon tri2(3);
    tri2.add_point(1,3,0);
    tri2.add_point(2,2,0);
    tri2.add_point(1,0,0);

    Polygon tri3(3);
    tri3.add_point(2,2,0);
    tri3.add_point(4,0,0);
    tri3.add_point(4,2,0);

    Polygon tri4(3);
    tri4.add_point(4,2,0);
    tri4.add_point(6,0,0);
    tri4.add_point(4,0,0);

    Polygon tri5(3);
    tri5.add_point(6,0,0);
    tri5.add_point(4,2,0);
    tri5.add_point(6,2,0);

//    Polygon tri6(3);
//    tri6.add_point(6,0,0);
//    tri6.add_point(4,2,0);
//    tri6.add_point(6,4,1);

    Polygon tri7(3);
    tri7.add_point(1,3,0);
    tri7.add_point(4,2,0);
    tri7.add_point(3,4,0);

    Polygon tri8(3);
    tri8.add_point(6,2,0);
    tri8.add_point(6,0,0);
    tri8.add_point(7,-1,0);

    Polygon tri9(3);
    tri9.add_point(6,2,0);
    tri9.add_point(8,0,0);
    tri9.add_point(7,-1,0);

    Polygon tri10(3);
    tri10.add_point(1,0,0);
    tri10.add_point(2,2,0);
    tri10.add_point(4,0,0);

    Polygon tri11(3);
    tri11.add_point(1,3,0);
    tri11.add_point(4,2,0);
    tri11.add_point(2,2,0);


    std::vector<Polygon> faces3;
    faces3.push_back(tri0);
    faces3.push_back(tri1);
    faces3.push_back(tri2);
    faces3.push_back(tri3);
    faces3.push_back(tri4);
    faces3.push_back(tri5);
//    faces3.push_back(tri6);
    faces3.push_back(tri7);
    faces3.push_back(tri8);
    faces3.push_back(tri9);
    faces3.push_back(tri10);
    faces3.push_back(tri11);

    Polymesh trimesh;
    trimesh.add_face(tri0);
    trimesh.add_face(tri1);
    trimesh.add_face(tri2);
    trimesh.add_face(tri3);
    trimesh.add_face(tri4);
    trimesh.add_face(tri5);
//    trimesh.add_face(tri6);
    trimesh.add_face(tri7);
    trimesh.add_face(tri8);
    trimesh.add_face(tri9);
    trimesh.add_face(tri10);
    trimesh.add_face(tri11);

    Int_vector mask(1,0);

    find_right_triangles(faces3, mask);

    std::cout << "Size of mask: " << mask.size() << std::endl;

    std::cout << "Mask: ";
    for(int i = 0; i < mask.size(); i++)
    {
        std::cout << mask(i);
    }
    std::cout << "\n(expected mask: 11011110100)\n" << std::endl;

    std::vector<Right_Triangle_Pair> rtp;

    find_rectangles(faces3, mask, trimesh, rtp);

}

void test_find_rectangles_2()
{
    // Test find_rectangles
    Polygon trp0(3);
    trp0.add_point(0,0,0);
    trp0.add_point(0,0,1);
    trp0.add_point(1,0,1);
    Polygon trp1(3);
    trp1.add_point(0,0,0);
    trp1.add_point(1,0,0);
    trp1.add_point(1,0,1);
    Polygon trp2(3);
    trp2.add_point(0,0,0);
    trp2.add_point(0,0,1);
    trp2.add_point(0,2,1);
    Polygon trp3(3);
    trp3.add_point(0,0,0);
    trp3.add_point(0,2,1);
    trp3.add_point(0,2,0);
    Polygon trp4(3);
    trp4.add_point(0,0,0);
    trp4.add_point(0,2,0);
    trp4.add_point(1,0,0);
    Polygon trp5(3);
    trp5.add_point(0,2,0);
    trp5.add_point(1,2,0);
    trp5.add_point(1,0,0);
    Polygon trp6(3);
    trp6.add_point(0,2,0);
    trp6.add_point(0,2,1);
    trp6.add_point(1,2,1);
    Polygon trp7(3);
    trp7.add_point(0,2,0);
    trp7.add_point(1,2,0);
    trp7.add_point(1,2,1);
    Polygon trp8(3);
    trp8.add_point(0,2,1);
    trp8.add_point(0,0,1);
    trp8.add_point(1,0,1);
    Polygon trp9(3);
    trp9.add_point(0,2,1);
    trp9.add_point(1,2,1);
    trp9.add_point(1,0,1);
    Polygon trp10(3);
    trp10.add_point(1,0,0);
    trp10.add_point(1,2,1);
    trp10.add_point(1,0,1);
    Polygon trp11(3);
    trp11.add_point(1,0,0);
    trp11.add_point(1,2,0);
    trp11.add_point(1,2,1);

    Polymesh tmesh;
    tmesh.add_face(trp0);
    tmesh.add_face(trp1);
    tmesh.add_face(trp2);
    tmesh.add_face(trp3);
    tmesh.add_face(trp4);
    tmesh.add_face(trp5);
    tmesh.add_face(trp6);
    tmesh.add_face(trp7);
    tmesh.add_face(trp8);
    tmesh.add_face(trp9);
    tmesh.add_face(trp10);
    tmesh.add_face(trp11);

    std::vector<Polygon> faces4 = tmesh.get_faces();

    Int_vector mask(1,0);

    find_right_triangles(faces4, mask);

    std::cout << "Size of mask: " << mask.size() << std::endl;

    std::cout << "Mask:           ";
    for(int i = 0; i < mask.size(); i++)
    {
        std::cout << mask(i);
    }
    std::cout << "\n(expected mask: 111111111111)\n" << std::endl;

    std::vector<Right_Triangle_Pair> rtp2;

    find_rectangles(faces4, mask, tmesh, rtp2);

    std::cout << "# rectangles found: " << rtp2.size() << std::endl;

    for(unsigned int i = 0; i < rtp2.size(); i++)
    {
        int tri_index1;
        int tri_index2;
        int hypot1;
        int hypot2;
        Vector par1;
        Vector par2;

        tri_index1 = rtp2[i].get_triangle1();
        tri_index2 = rtp2[i].get_triangle2();
        std::cout << "Triangle indices: " << tri_index1 << ", " 
                                          << tri_index2 << std::endl;

        tri_index1 = rtp2[i].get_triangle1();
        tri_index2 = rtp2[i].get_triangle2();
        std::cout << "Triangle indices (alt): " << tri_index1 << ", " 
                                                << tri_index2 << std::endl;

        hypot1 = rtp2[i].get_hypotenuse1();
        hypot2 = rtp2[i].get_hypotenuse2();
        std::cout << "hypotenuses: " << hypot1 << ", " << hypot2 << std::endl;

        par1 = rtp2[i].get_parallel_edges1();
        par2 = rtp2[i].get_parallel_edges2();
        std::cout << "parallel 1: " << par1 << std::endl;
        std::cout << "parallel 2: " << par2 << "\n" << std::endl;
        
    }
    
 
    std::vector<Polymesh_Plane> plane;
    int num_faces = tmesh.num_faces();
    std::cout << "Number of faces in polymesh: " << num_faces << std::endl;
    std::vector<int> all_indices(num_faces,0);
    std::cout << "all_indices: " << all_indices << std::endl;
    for(unsigned int i = 0; i < rtp2.size(); i++)
    {
        Vector plane1_params;
        std::vector<int> indices;
        int tri_index1;
        int tri_index2;

        tri_index1 = rtp2[i].get_triangle1();
        tri_index2 = rtp2[i].get_triangle2();

        indices.push_back(tri_index1);
        indices.push_back(tri_index2);

//        std::cout << "index1: " << tri_index1 << "  index2: " 
//                                << tri_index2 << std::endl;

        all_indices[tri_index1] = 1;
        all_indices[tri_index2] = 1;

//        faces4[tri_index1].fit_plane(plane1_params);

        plane.push_back(Polymesh_Plane(tmesh, plane1_params, indices));
    }
    std::cout << "all_indices: " << all_indices << std::endl;

    std::vector<int> indices;
    for(int i = 0; i < num_faces; i++)
    {
        if(all_indices[i] == 0)
        {
            indices.push_back(i);
        }
    }
    
    if(indices.size() > 0)
    {
        Vector plane1_params;
        plane.push_back(Polymesh_Plane(tmesh, plane1_params, indices));
    }

    std::cout << "Total planes found: " << plane.size() << std::endl;
}

int main()
{
    std::cout << "TEST_PARAPIPED():" << std::endl;
    test_parapiped();
    std::cout << std::endl;

    std::cout << "TEST_TRIANGULAR_POLYGONS():" << std::endl;
    test_triangular_polygons();
    std::cout << std::endl;

    std::cout << "TEST_FIND_RIGHT_TRIANGLES():" << std::endl;
    test_find_right_triangles();
    std::cout << std::endl;

    std::cout << "TEST_FIND_RECTANGLES():" << std::endl;
    test_find_rectangles();
    std::cout << std::endl;

    std::cout << "TEST_FIND_RECTANGLES_2():" << std::endl;
    test_find_rectangles_2();
    std::cout << std::endl;   

}
