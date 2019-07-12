/* =========================================================================== *
   |
   |  Copyright (c) 1994-2011 by Kobus Barnard (author)
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
   |  Author:  Daniel Vazquez
 * =========================================================================== */

/* $Id$ */

#include <iostream>

#include <people_tracking_cpp/pt_color_likelihood.h>
#include <people_tracking_cpp/pt_visibility.h>
#include <people_tracking_cpp/pt_body_2d.h>
#include <video_cpp/video_background.h>
#include <detector_cpp/d_bbox.h>
#include <i_cpp/i_image.h>
#include <i_cpp/i_pixel.h>
#include <m_cpp/m_matrix.h>
#include <gr_cpp/gr_2D_bounding_box.h>
#include <l_cpp/l_test.h>
#include <l/l_sys_io.h>

using namespace kjb;
using namespace kjb::pt;

std::vector<Matrix> get_bg_matrices(std::vector<std::string>& img_names)
{
    std::vector<Matrix> bg_vec(3);

    compute_median_background(img_names, bg_vec[0], bg_vec[1], bg_vec[2]);

    return bg_vec;
}

Image get_random_image(size_t num_rows, size_t num_cols)
{
    Matrix rand_red_mat = create_random_matrix(num_rows, num_cols);
    Matrix rand_blue_mat = create_random_matrix(num_rows, num_cols);
    Matrix rand_green_mat = create_random_matrix(num_rows, num_cols);

    rand_red_mat *= 255;
    rand_blue_mat *= 255;
    rand_green_mat *= 255;

    Image img;
    img.from_color_matrices(rand_red_mat, rand_green_mat, rand_red_mat);

    return img;
    
}

int main()
{
    size_t cols = 200;
    size_t rows = 200;

    std::string img_w_fn = "output/color_likelihood_cpp/img_w.jpg";
    std::string img_r_fn = "output/color_likelihood_cpp/img_r.jpg";

    kjb_c::kjb_mkdir("output/color_likelihood_cpp");

    std::vector<Matrix> bg_vec;

    Vector center(2,0.0f);

    Bbox full_bbox;
    Bbox body_bbox;

    std::vector<std::string> img_names(2);
    std::vector<std::string> img_bg_frames(2);

    Image img_w(rows, cols , 255, 255, 255);
    img_w.write(img_w_fn.c_str());

    Image img_r = get_random_image(200, 200);
    //Image img_r (rows, cols, 0, 0, 0);
    img_r.write(img_r_fn.c_str());


    //TEST 1 color_l_1 = 0 || P(img_r, img_r) - P(img_r, img_r)
    center[0] = img_r.get_num_cols() / 2;
    center[1] = img_r.get_num_rows() / 2;
    
    body_bbox.set_center(center);
    body_bbox.set_width(20);
    body_bbox.set_height(20);

    Body_2d body(full_bbox, body_bbox);

    img_names[0] = img_r_fn;
    img_names[1] = img_r_fn;
    
    img_bg_frames[0] = img_bg_frames[1] = img_names[0];// bg = img_r

    bg_vec = get_bg_matrices(img_names);

    Color_likelihood c_l_1;
    c_l_1.set_bg_r_matrix(&bg_vec[0]);
    c_l_1.set_bg_g_matrix(&bg_vec[1]);
    c_l_1.read_frames(img_bg_frames);

    size_t start_time = 1;
    double color_l_1 = c_l_1.at_box(body, body, start_time, start_time + 1);

    //std::cout << color_l_1 << std::endl;
    //// END TEST 1

    //// TEST 2 color_l_2 < 0 || P(img_r, img_w) - P(img_r, img_r)
    center[0] = img_r.get_num_cols() / 2;
    center[1] = img_r.get_num_rows() / 2;
    
    body.body_bbox.set_center(center);
    body.body_bbox.set_width(20);
    body.body_bbox.set_height(20);


    img_names[0] = img_r_fn;
    img_names[1] = img_w_fn;

    img_bg_frames[0] = img_bg_frames[1] = img_names[0];// bg = img_r

    bg_vec = get_bg_matrices(img_bg_frames);

    Color_likelihood c_l_2;
    c_l_2.set_bg_r_matrix(&bg_vec[0]);
    c_l_2.set_bg_g_matrix(&bg_vec[1]);
    c_l_2.read_frames(img_names);

    double color_l_2 = c_l_2.at_box(body, body, start_time, start_time + 1);

    //std::cout << color_l_2 << std::endl;
    //// END TEST 2

    //// TEST 3 color_l_3 > 0 || P(img_r, img_r) - P(img_r, img_w)
    center[0] = img_r.get_num_cols() / 2;
    center[1] = img_r.get_num_rows() / 2;
    
    body.body_bbox.set_center(center);
    body.body_bbox.set_width(20);
    body.body_bbox.set_height(20);


    img_names[0] = img_r_fn;
    img_names[1] = img_r_fn;

    img_bg_frames[0] = img_bg_frames[1] = img_w_fn;

    bg_vec = get_bg_matrices(img_bg_frames);

    Color_likelihood c_l_3;
    c_l_3.set_bg_r_matrix(&bg_vec[0]);
    c_l_3.set_bg_g_matrix(&bg_vec[1]);
    c_l_3.read_frames(img_names);

    double color_l_3 = c_l_3.at_box(body, body, start_time, start_time + 1);

    //std::cout << color_l_3 << std::endl;
    //// END TEST 3

    TEST_TRUE(color_l_1 <= DBL_EPSILON
                && color_l_2 < DBL_EPSILON
                && color_l_3 > DBL_EPSILON);
    RETURN_VICTORIOUSLY();
}

