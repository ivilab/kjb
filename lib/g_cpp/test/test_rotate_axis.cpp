
/* $Id: test_quaternion.cpp 6414 2010-07-29 18:43:32Z ksimek $ */

#include <iostream>
#include <cmath>
#include "g_cpp/g_quaternion.h"
#include "g_cpp/g_orthogonal_corner.h"
#include "m_cpp/m_vector.h"


using namespace::kjb;

void test_corners();

int main(int argc, char *argv[])
{
    kjb::Vector axis(3);
    axis(0) = 0.1733;
    axis(1) = 0.9847;
    axis(2) = -0.017;
    double angle = 0.54;

    Quaternion q(axis, angle);

    Matrix rotation = q.get_rotation_matrix();

    kjb::Vector input(4);

    input(0) = 985.4755;
    input(1) = -170.6889;
    input(2) =  156.3985;
    input(3) = 1.0000;

    kjb::Vector output = rotation*input;

    kjb::Vector expected_output(4);
    expected_output(0) = 9.22945971e+02;
    expected_output(1) = -1.68944295e+02;
    expected_output(2) = -3.79981200e+02;
    expected_output(3) =   1.0000;

    double diff = expected_output.get_max_abs_difference(output);
    if(fabs(diff) >  0.0001)
    {
        KJB_THROW_2(KJB_error,"FAILED");
    }

    axis(0) = -0.9888;
    axis(1) = -0.0072;
    axis(2) = -0.1491;
    angle =  0.2854;

    input(0) = -0.7;
    input(1) = 1011.3;
    input(2) = -44.5;
    input(3) = 1;

    q.set_axis_angle(axis,angle);
    rotation = q.get_rotation_matrix();
    output = rotation*input;

    expected_output(0) = 4.18688409e+01;
    expected_output(1) = 9.58032799e+02;
    expected_output(2) = -3.24235386e+02;

    diff = expected_output.get_max_abs_difference(output);
    if(fabs(diff) > 0.0001)
    {
        KJB_THROW_2(KJB_error,"FAILED");
    }

    axis(0) = 0.4016;
    axis(1) = -0.9103;
    axis(2) = 0.1006;

    angle = 0.9444;


    input(0) = -914.7044;
    input(1) = -416.7594;
    input(2) =  -119.9360;

    q.set_axis_angle(axis,angle);
    rotation = q.get_rotation_matrix();
    output = rotation*input;

    expected_output(0) = -4.13802630e+02;
    expected_output(1) = -2.79823593e+02;
    expected_output(2) = -8.80467672e+02;

    diff = expected_output.get_max_abs_difference(output);
    if(fabs(diff) > 0.0001)
    {
        KJB_THROW_2(KJB_error,"FAILED");
    }

    axis(0) =  -0.0538;
    axis(1) = -0.9913;
    axis(2) =   0.1202;
    angle = 0.3362;

    input(0) = -1005.7;
    input(1) = 57.4;
    input(2) = 23.7;

    q.set_axis_angle(axis,angle);
    rotation = q.get_rotation_matrix();
    output = rotation*input;

    expected_output(0) = -9.59421972e+02;
    expected_output(1) = 1.47237587e+01;
    expected_output(2) = -3.07541265e+02;

    diff = expected_output.get_max_abs_difference(output);
    if(fabs(diff) > 0.0001)
    {
        KJB_THROW_2(KJB_error,"FAILED");
    }

    axis(0) = 0.2781;
    axis(1) = 0.9545;
    axis(2) = 0.1076;
    angle = 1.3119;

    input(0) = 959.2045;
    input(1) = -303.6168;
    input(2) = 214.4395;

    q.set_axis_angle(axis,angle);
    rotation = q.get_rotation_matrix();
    output = rotation*input;

    expected_output(0) =  4.75023117e+02;
    expected_output(1) = -3.55904660e+01;
    expected_output(2) = -9.11771403e+02;

    diff = expected_output.get_max_abs_difference(output);
    if(fabs(diff) > 0.0001)
    {
        KJB_THROW_2(KJB_error,"FAILED");
    }

    test_corners();
    std::cout << "Success" << std::endl;
    return 0;
}

void test_corners()
{
    kjb::Vector corner3D_1;
    kjb::Vector corner3D_2;
    kjb::Vector corner3D_3;
    unsigned int epsilon = 1;
    double focal_length = 387;
    kjb::Vector corner2D_1(3);
    kjb::Vector corner2D_2(3);
    kjb::Vector corner2D_3(3);
    kjb::Vector position_2D(3);
    kjb::Vector position_3D(3);

    corner2D_1(0) = -48.6290;
    corner2D_1(1) = 15.2430;
    corner2D_1(2) = 1.0;

    corner2D_2(0) = -58.5510;
    corner2D_2(1) = 26.9760;
    corner2D_2(2) = 1.0;

    corner2D_3(0) = -67.6270;
    corner2D_3(1) = 12.9400;
    corner2D_3(2) = 1.0;

    position_3D(0) = -151.1;
    position_3D(1) = 43.9;
    position_3D(2) = 1000.0;

    position_2D(0) = -58.4780;
    position_2D(1) =  16.9759;
    position_2D(2) = 1.0;

    kjb::get_3D_corner_orientation_from_2D_corner_lines
    (
        corner2D_1,
        corner2D_2,
        corner2D_3,
        position_2D,
        position_3D,
        focal_length,
        epsilon,
        corner3D_1,
        corner3D_2,
        corner3D_3
    );

    corner3D_1(3) = 0.0;
    corner3D_2(3) = 0.0;
    corner3D_3(3) = 0.0;
    corner3D_1 = corner3D_1.normalize();
    corner3D_2 = corner3D_2.normalize();
    corner3D_3 = corner3D_3.normalize();

    kjb::Vector expected_out(4, 0.0);
    expected_out(0) = 9.11705282e-01;
    expected_out(1) = -1.66898378e-01;
    expected_out(2) = -3.75417647e-01;

    double diff = expected_out.get_max_abs_difference(corner3D_1);
    if(fabs(diff) > 0.0001)
    {
        KJB_THROW_2(KJB_error, "Expected output does not match");
    }

    expected_out(0) = 4.13860560e-02;
    expected_out(1) = 9.46415843e-01;
    expected_out(2) = -3.20287755e-01;
    diff = expected_out.get_max_abs_difference(corner3D_2);
    if(fabs(diff) > 0.0001)
    {
        KJB_THROW_2(KJB_error, "Expected output does not match");
    }

    expected_out(0) = -4.08771455e-01;
    expected_out(1) = -2.76449645e-01;
    expected_out(2) = -8.69759444e-01;
    diff = expected_out.get_max_abs_difference(corner3D_3);
    if(fabs(diff) > 0.0001)
    {
        KJB_THROW_2(KJB_error, "Expected output does not match");
    }

    corner2D_1(0) = 32.5870;
    corner2D_1(1) = 96.483;
    corner2D_1(2) = 1.0;

    corner2D_2(0) = 23.499;
    corner2D_2(1) = 82.3017;
    corner2D_2(2) = 1.0;

    corner2D_3(0) = 13.518;
    corner2D_3(1) = 92.8435;
    corner2D_3(2) = 1.0;

    position_3D(0) = 30.4;
    position_3D(1) = 119.6;
    position_3D(2) = 1000.0;

    position_2D(0) = 23.503;
    position_2D(1) = 92.3017;
    position_2D(2) = 1.0;

    epsilon = 1;
    focal_length = 772;

    kjb::get_3D_corner_orientation_from_2D_corner_lines
    (
        corner2D_1,
        corner2D_2,
        corner2D_3,
        position_2D,
        position_3D,
        focal_length,
        epsilon,
        corner3D_1,
        corner3D_2,
        corner3D_3
    );

    corner3D_1(3) = 0.0;
    corner3D_2(3) = 0.0;
    corner3D_3(3) = 0.0;
    corner3D_1 = corner3D_1.normalize();
    corner3D_2 = corner3D_2.normalize();
    corner3D_3 = corner3D_3.normalize();

    expected_out(0) = 3.05508004e-01;
    expected_out(1) = 4.02106271e-02;
    expected_out(2) = -9.51340089e-01;

    diff = expected_out.get_max_abs_difference(corner3D_1);
    if(fabs(diff) > 0.0001)
    {
        KJB_THROW_2(KJB_error, "Expected output does not match");
    }

    expected_out(0) = -1.69833548e-03;
    expected_out(1) = -9.99085410e-01;
    expected_out(2) = -4.27253912e-02;
    diff = expected_out.get_max_abs_difference(corner3D_2);
    if(fabs(diff) > 0.0001)
    {
        KJB_THROW_2(KJB_error, "Expected output does not match");
    }

    expected_out(0) = -9.52180848e-01;
    expected_out(1) = 1.46743592e-02;
    expected_out(2) = -3.05182398e-01;
    diff = expected_out.get_max_abs_difference(corner3D_3);
    if(fabs(diff) > 0.0001)
    {
        KJB_THROW_2(KJB_error, "Expected output does not match");
    }

}
