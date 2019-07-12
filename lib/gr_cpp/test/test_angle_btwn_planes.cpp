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


int main()
{
    Vector plane0_params;
    Vector plane1_params;
    Vector plane2_params;

    double angle;

    Polygon p1(3);
    p1.add_point(0,0,0);
    p1.add_point(1,1,1);
    p1.add_point(1,1,0);

    Polygon p2(3);
    p2.add_point(0,0,0);
    p2.add_point(0,1,1);
    p2.add_point(1,0,1);

    p1.fit_plane(plane1_params);
    p2.fit_plane(plane2_params);

    angle = get_angle_between_two_vectors(plane1_params, plane2_params);

    std::cout << "angle = " << angle << std::endl;
    
    Polygon p3(3);
    p3.add_point(1,0,0);
    p3.add_point(2,1,1);
    p3.add_point(2,1,0);

    p3.fit_plane(plane2_params);

    angle = get_angle_between_two_vectors(plane1_params, plane2_params);

    std::cout << "angle2 = " << angle << std::endl;

    std::cout << "DBL_EPSILON = " << DBL_EPSILON << std::endl;

/////////////////////////////////////////////////////////////////////////////////

    // Test check_if_faces_are_coplanar() when have parallel faces on opposite 
    // sides of origin.

    Polygon par0(3);
    par0.add_point(0,0,0);
    par0.add_point(0,1,0);
    par0.add_point(0,0,1);

    Polygon par1(3);
    par1.add_point(1,0,0);
    par1.add_point(1,1,0);
    par1.add_point(1,0,1);

    Polygon par2(3);
    par2.add_point(-1,0,0);
    par2.add_point(-1,1,0);
    par2.add_point(-1,0,1);

    par0.fit_plane(plane0_params);
    par1.fit_plane(plane1_params);
    par2.fit_plane(plane2_params);

    angle = get_angle_between_two_vectors(plane0_params, plane1_params);
    if(angle > 0 && angle < 3.14)
    {
        std::cout << "TEST FAILED: angle btwn par0 and par1 = " << angle << "\n";
    }

    angle = get_angle_between_two_vectors(plane0_params, plane2_params);
    if(angle > 0 && angle < 3.14)
    {
        std::cout << "TEST FAILED: angle btwn par0 and par2 = " << angle << "\n";
    }

    angle = get_angle_between_two_vectors(plane2_params, plane1_params);
    if(angle > 0 && angle < 3.14)
    {
        std::cout << "TEST FAILED: angle btwn par1 and par2 = " << angle << "\n";
    }

    if(check_if_faces_are_coplanar(plane0_params, plane1_params, 0.087266, 0.1))
    {
        std::cout << "TEST FAILED: found par0 and par1 to be coplanar!\n";
    }

    if(check_if_faces_are_coplanar(plane0_params, plane2_params, 0.087266, 0.1))
    {
        std::cout << "TEST FAILED: found par0 and par2 to be coplanar!\n";
    }

    if(check_if_faces_are_coplanar(plane1_params, plane2_params, 0.087266, 0.1))
    {
        std::cout << "TEST FAILED: found par1 and par2 to be coplanar!\n";
    }
}
