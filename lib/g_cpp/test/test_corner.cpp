
/* $Id: test_quaternion.cpp 6414 2010-07-29 18:43:32Z ksimek $ */

#include <iostream>
#include <cmath>
#include "g_cpp/g_quaternion.h"
#include "g_cpp/g_orthogonal_corner.h"
#include "m_cpp/m_vector.h"
#include <edge_cpp/line_segment.h>
#include <st_cpp/st_perspective_camera.h>
#include <gr_cpp/gr_offscreen.h>


using namespace::kjb;

void test_corners();

int main(int argc, char *argv[])
{
    test_corners();
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

    kjb::Image img("001.jpg");
    double princx = img.get_num_cols()/2.0;
    double princy = img.get_num_rows()/2.0;

    Matrix translation(3,3,0.0);
    translation(0,2) = princx;
    translation(1,2) = princy;
    translation(0,0) = 1;
    translation(1,1) = -1;
    translation(2,2) = 1;
    corner2D_1 = translation*corner2D_1;
    corner2D_2 = translation*corner2D_2;
    corner2D_3 = translation*corner2D_3;
    position_2D = translation*position_2D;

    Line_segment ls1;
    Line_segment ls2;
    Line_segment ls3;

    ls1.init_from_end_points(corner2D_1(0), corner2D_1(1), position_2D(0), position_2D(1) );
    ls2.init_from_end_points(corner2D_2(0), corner2D_2(1), position_2D(0), position_2D(1) );
    ls3.init_from_end_points(corner2D_3(0), corner2D_3(1), position_2D(0), position_2D(1) );

    ls1.draw(img, 0, 0, 255);
    ls2.draw(img, 0, 0, 255);
    ls3.draw(img, 0, 0, 255);

    corner3D_1 *= 300;
    corner3D_2 *= 300;
    corner3D_3 *= 300;

    kjb::Vector corner_pos(4, 1.0);
    for(unsigned int i = 0; i < 3; i++)
    {
        corner_pos(i) = position_3D(i);
    }

    kjb::Vector vertex_1 = corner_pos + corner3D_1;
    kjb::Vector vertex_2 = corner_pos + corner3D_2;
    kjb::Vector vertex_3 = corner_pos + corner3D_3;

    std::cout << corner3D_1.normalize() << std::endl;
    std::cout << corner3D_2.normalize() << std::endl;
    std::cout << corner3D_3.normalize() << std::endl;
    std::cout << corner_pos << std::endl;

    Perspective_camera camera;
    camera.set_focal_length(focal_length);
    //camera.set_principal_point_x(princx);
    //camera.set_principal_point_y(princy);


    static kjb::Offscreen_buffer* offscreen = 0;
    offscreen = kjb::create_and_initialize_offscreen_buffer(img.get_num_cols(), img.get_num_rows());
    //kjb::opengl::default_init_opengl(img.get_num_cols(), img.get_num_rows());
    camera.prepare_for_rendering(true);
    img.write("corner.jpg");

    corner3D_1(3) = 1.0;
    corner3D_2(3) = 1.0;
    corner3D_3(3) = 1.0;
    Base_gl_interface::set_gl_view(img);

    glBegin(GL_LINES);
    glColor3f(1.0,0.0,0.0);
    glVertex4d(vertex_1(0),vertex_1(1),-vertex_1(2),vertex_1(3));
    glVertex4d(corner_pos(0), corner_pos(1), -corner_pos(2), corner_pos(3));
    glVertex4d(vertex_2(0),vertex_2(1),-vertex_2(2),vertex_2(3));
    glVertex4d(corner_pos(0), corner_pos(1), -corner_pos(2), corner_pos(3));
    glVertex4d(vertex_3(0),vertex_3(1),-vertex_3(2),vertex_3(3));
    glVertex4d(corner_pos(0), corner_pos(1), -corner_pos(2), corner_pos(3));
    glEnd();

    //glFlush();

    kjb_c::KJB_image * capture = NULL;
    Base_gl_interface::capture_gl_view(&capture);
    Image img2(capture);
    ls1.draw(img2, 0, 0, 255);
    ls2.draw(img2, 0, 0, 255);
    ls3.draw(img2, 0, 0, 255);

    img2.write("capture.jpg");

    delete offscreen;

}
