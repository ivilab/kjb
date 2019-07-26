
/* $Id: g_quaternion.h 6946 2010-10-05 17:12:13Z ksimek $ */

/* =========================================================================== *
|
|  Copyright (c) 1994-2008 by Kobus Barnard (author).
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
* =========================================================================== */

#include "m_cpp/m_matrix.h"
#include "m_cpp/m_vector.h"
#include "g_cpp/g_orthogonal_corner.h"
#include "g_cpp/g_quaternion.h"
#include "g_cpp/g_line.h"

void kjb::get_3D_corner_orientation_from_2D_corner_lines
(
    const kjb::Vector & corner2D_1,
    const kjb::Vector & corner2D_2,
    const kjb::Vector & corner2D_3,
    const kjb::Vector & position_2D,
    const kjb::Vector & position_3D,
    double focal_length,
    int epsilon,
    kjb::Vector & corner3D_1,
    kjb::Vector & corner3D_2,
    kjb::Vector & corner3D_3
)
{
    Line line1(corner2D_1, position_2D);
    Line line2(corner2D_2, position_2D);
    Line line3(corner2D_3, position_2D);

    kjb::Vector N1 = line1.get_params();
    kjb::Vector N2 = line2.get_params();
    kjb::Vector N3 = line3.get_params();

    if(corner2D_1(0) < position_2D(0))
    {
        N1 = (-1)*N1;
    }
    if(corner2D_2(0) < position_2D(0))
    {
        N2 = (-1)*N2;
    }
    if(corner2D_3(0) < position_2D(0))
    {
        N3 = (-1)*N3;
    }

    if( (corner2D_1(0) == position_2D(0)) && (corner2D_1(1) > position_2D(1)))
    {
        N1 = (-1)*N1;
    }
    if( (corner2D_2(0) == position_2D(0)) && (corner2D_2(1) > position_2D(1)))
    {
        N2 = (-1)*N2;
    }
    if( (corner2D_3(0) == position_2D(0)) && (corner2D_3(1) > position_2D(1)))
    {
        N3 = (-1)*N3;
    }

    N1(2) = N1(2)/focal_length;
    N2(2) = N2(2)/focal_length;
    N3(2) = N3(2)/focal_length;

    N1 = N1.normalize();
    N2 = N2.normalize();
    N3 = N3.normalize();

    kjb::Vector cross1 = cross(N1, position_3D).normalize();
    kjb::Vector cross2 = cross(N2, position_3D).normalize();
    kjb::Vector cross3 = cross(N3, position_3D).normalize();

    double cosphi12 = dot(cross1, cross2);
    double cosphi23 = dot(cross2, cross3);
    double cosphi31 = dot(cross3, cross1);

    double theta1 = 0.0;
    double theta2 = 0.0;
    double theta3 = 0.0;

    if( fabs(cosphi23) < DBL_EPSILON )
    {
        theta1 = M_PI_2;
    }
    else
    {
        double arg_root = -( (cosphi12*cosphi31)/cosphi23);
        if(arg_root < 0)
        {
            KJB_THROW_2(KJB_error, "Get 3D corner from 2D corner, argument of square root is negative");
        }
        double tg_theta1 = epsilon*sqrt(arg_root);
        theta1 = atan(tg_theta1);
        if(fabs(tg_theta1) < DBL_EPSILON)
        {
            theta2 = M_PI_2;
            theta3 = M_PI_2;
        }
        else
        {
            theta2 = atan( -(cosphi12/tg_theta1));
            theta3 = atan( -(cosphi31/tg_theta1));
        }
    }

    Quaternion q(N1, theta1);
    Matrix rotation = q.get_rotation_matrix();

    kjb::Vector input_vector(4, 1.0);
    for(unsigned int i = 0; i < 3; i++)
    {
        input_vector(i) = cross1(i);
    }
    corner3D_1 = rotation*input_vector;

    q.set_axis_angle(N2, theta2);
    rotation = q.get_rotation_matrix();
    for(unsigned int i = 0; i < 3; i++)
    {
        input_vector(i) = cross2(i);
    }
    corner3D_2 = rotation*input_vector;

    q.set_axis_angle(N3, theta3);
    rotation = q.get_rotation_matrix();
    for(unsigned int i = 0; i < 3; i++)
    {
        input_vector(i) = cross3(i);
    }
    corner3D_3 = rotation*input_vector;


}
