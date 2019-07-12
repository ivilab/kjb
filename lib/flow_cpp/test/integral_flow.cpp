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
   |  Author:  Jinyan Guan, Ernesto Brau
 * =========================================================================== */

/* $Id: integral_flow.cpp 13194 2012-10-22 21:49:03Z ernesto $ */

#include <flow_cpp/flow_integral_flow.h>
#include <flow_cpp/flow_dense.h>
#include <m_cpp/m_matrix.h>
#include <m_cpp/m_vector.h>
#include <gr_cpp/gr_2D_bounding_box.h>
#include <prob_cpp/prob_distribution.h>
#include <prob_cpp/prob_sample.h>
#include <l_cpp/l_test.h>
#include <l/l_sys_time.h>
#include <l/l_init.h>
#include <string>
#include <iostream>

using namespace kjb;
using namespace std;

int main(int argc, char** argv)
{
    kjb_c::kjb_init();

    const string show_times_flag = "-show-times";
    const double rho = 0.99;

    bool show_times = false;
    string file_x = "";
    string file_y = "";

    if(argc == 2)
    {
        if(show_times_flag == argv[1]) show_times = true;
    }
    else if(argc == 3)
    {
        file_x = argv[1];
        file_y = argv[2];
    }
    else if(argc == 4)
    {
        file_x = argv[1];
        file_y = argv[2];
        if(show_times_flag == argv[3]) show_times = true;
    }

    size_t imw;
    size_t imh;

    Matrix F_x;
    Matrix F_y;

    if(file_x.empty())
    {
        imw = 1200;
        imh = 800;

        // create random flow image
        Vector f = Vector().set(sample(Normal_distribution(2, 0.2)),
                                sample(Normal_distribution(2, 0.2)));

        F_x.resize(imh, imw);
        F_y.resize(imh, imw);
        for(size_t i = 0; i < imh; i++)
        {
            for(size_t j = 0; j < imw; j++)
            {
                F_x(i, j) = f[0] + sample(Normal_distribution(0.0, 0.3));
                F_y(i, j) = f[1] + sample(Normal_distribution(0.0, 0.3));
            }
        }
    }
    else
    {
        F_x.read(file_x.c_str());
        F_y.read(file_y.c_str());
        imw = F_x.get_num_cols();
        imh = F_x.get_num_rows();
    }

    // create integral flow
    Integral_flow I_x(F_x);
    Integral_flow I_y(F_y);

    // create integral flow with subsampling
    const size_t ssr = 4;
    Integral_flow S_x(F_x, ssr);
    Integral_flow S_y(F_y, ssr);

    // use these throughout
    Vector real_flow;
    Vector int_flow;
    Vector ssi_flow;

    // TEST BOX SIZE OF IMAGE
    real_flow.set(sum_matrix_cols(F_x).sum_vector_elements(),
                  sum_matrix_rows(F_y).sum_vector_elements());
    int_flow.set(I_x.flow_sum(imw, imh), I_y.flow_sum(imw, imh));
    TEST_TRUE(vector_distance(real_flow, int_flow)
                    < (1 - rho)*real_flow.magnitude());

    // TEST BOX WITH DIMENSION < 1
    // x < 1
    real_flow.set(F_x.get_col(0).sum_vector_elements(),
                  F_y.get_col(0).sum_vector_elements());
    int_flow.set(I_x.flow_sum(0.5, imh), I_y.flow_sum(0.5, imh));
    TEST_TRUE(vector_distance(0.5*real_flow, int_flow)
                    < (1 - rho)*real_flow.magnitude());

    // y < 1
    real_flow.set(F_x.get_row(0).sum_vector_elements(),
                  F_y.get_row(0).sum_vector_elements());
    int_flow.set(I_x.flow_sum(imw, 0.5), I_y.flow_sum(imw, 0.5));
    TEST_TRUE(vector_distance(0.5*real_flow, int_flow)
                    < (1 - rho)*real_flow.magnitude());

    // TEST RANDOM BOXES
    double avg_time_real = 0.0;
    double avg_time_int = 0.0;
    double avg_time_ssi = 0.0;
    const size_t num_tests = 100;
    Categorical_distribution<size_t> U_x(0, imw - 1, 1);
    Categorical_distribution<size_t> U_y(0, imh - 1, 1);
    for(size_t it = 1; it <= num_tests; it++)
    {
        Vector p1 = Vector().set(sample(U_x), sample(U_y));
        Vector p2 = p1;
        while(fabs(p1[0] - p2[0]) < 100 || fabs(p1[1] - p2[1]) < 100)
            p2.set(sample(U_x), sample(U_y));

        Axis_aligned_rectangle_2d box(p1, p2);
        double w = box.get_width();
        double h = box.get_height();

        kjb_c::init_cpu_time();
        real_flow = w*h*average_flow(F_x, F_y, box);
        avg_time_real += kjb_c::get_cpu_time();

        kjb_c::init_cpu_time();
        int_flow = Vector().set(I_x.flow_sum(box), I_y.flow_sum(box));
        avg_time_int += kjb_c::get_cpu_time();

        kjb_c::init_cpu_time();
        ssi_flow = Vector().set(S_x.flow_sum(box), S_y.flow_sum(box));
        avg_time_ssi += kjb_c::get_cpu_time();

        //cout << "IT " << it << " (" << w << " X " << h << ")" << endl;

        //cout << real_flow << "    " << int_flow << endl;
        //cout << vector_distance(real_flow, int_flow) / real_flow.magnitude() << endl;
        // test accuracy of integral flow
        TEST_TRUE(vector_distance(real_flow, int_flow)
                        < (1 - rho)*real_flow.magnitude());

        //cout << real_flow << "    " << ssi_flow << endl;
        //cout << vector_distance(real_flow, ssi_flow) / real_flow.magnitude() << endl << endl;

        // test accuracy subsampling
        const double sg = 0.9;
        TEST_TRUE(vector_distance(real_flow, ssi_flow)
                        < (1 - sg)*real_flow.magnitude());
    }

    avg_time_real /= num_tests;
    avg_time_int /= num_tests;

    if(show_times)
    {
        cout << setprecision(10);
        cout << "Average computation times:\n";
        cout << "Full flow: " << avg_time_real / 1000.0 << "s.\n";
        cout << "Integral flow: " << avg_time_int / 1000.0 << "s.\n";
        cout << "Speedup: " << avg_time_real/avg_time_int << endl;
        cout << "Integral flow (with SS): " << avg_time_ssi / 1000.0 << "s.\n";
    }

    RETURN_VICTORIOUSLY();
}

