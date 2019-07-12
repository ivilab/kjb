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
   |  Author:  Ernesto Brau
 * =========================================================================== */

/* $Id$ */

#include <diff_cpp/diff_gradient_mt.h>
#include <diff_cpp/diff_gradient.h>
#include <m_cpp/m_vector.h>
#include <m_cpp/m_matrix.h>
#include <l_cpp/l_test.h>
#include <prob_cpp/prob_distribution.h>
#include <prob_cpp/prob_pdf.h>
#include <l/l_sys_mal.h>
#include <l/l_sys_time.h>
#include "funcs.h"
#include <iostream>
#include <vector>
#include <boost/bind.hpp>
#include <boost/ref.hpp>

using namespace std;
using namespace kjb;
using namespace boost;

const bool VERBOSE = false;

/** @brief  Main. */
int main(int argc, char** argv)
{
    const double dx = 0.0001;
    const double eps = 1e-5;

    kjb_c::kjb_l_set("heap-checking", "off");

    // declarations
    Vector x;
    Vector G;
    Vector M;
    size_t D;
    double er;

    ///////////////////////////////////
    // TEST SIMPLE 1-D FUNCTIONS
    ///////////////////////////////////
    // f(x) = 2x
    x.set(2.0);
    G = gradient_ffd_mt(two_x, x, vector<double>(1, dx));
    M = gradient_ffd(two_x, x, vector<double>(1, dx));
    er=vector_distance(G,M);
    if(VERBOSE)
    {
        cout << "f(x) = 2x" << endl;
        cout << "x = " << x << endl;
        cout << "G = " << G << endl;
        cout << endl;
    }

    TEST_TRUE(G.get_length() == 1);
    TEST_TRUE(fabs(G[0] - 2) <= eps);
    TEST_TRUE(er<=eps);

    // f(x) = x^3
    G = gradient_ffd_mt(x_cubed, x, vector<double>(1, dx));
    M = gradient_ffd(x_cubed, x, vector<double>(1, dx));
    er=vector_distance(G,M);
    if(VERBOSE)
    {
        cout << "f(x) = x^3" << endl;
        cout << "x = " << x << endl;
        cout << "G = " << G << endl;
        cout << endl;
    }

    TEST_TRUE(G.get_length() == 1);
    TEST_TRUE(fabs(G[0] - M[0]) <= eps);


    ///////////////////////////////////
    // TEST SIMPLE N-D FUNCTIONS
    ///////////////////////////////////
    // f(x) = x^2
    D = 5;
    x = create_random_vector(D);
    G = gradient_ffd_mt(x_squared, x, vector<double>(D, dx));
    M = gradient_ffd(x_squared, x, vector<double>(D, dx));
    er = vector_distance(G, M);

    if(VERBOSE)
    {
        cout << "f(x) = x^2" << endl;
        cout << "x = " << x << endl;
        cout << "G = " << G << endl;
        cout << endl;
    }

    TEST_TRUE(G.get_length() == D);
    TEST_TRUE(er <= eps);


    ///////////////////////////////////
    // TEST HESSIAN ON -LOG OF NORMAL
    ///////////////////////////////////
    // f(x) = phi(x) (normal pdf)
    D = 400;
    Vector mu = create_random_vector(D);
    x = 2*mu;
    Matrix A = create_random_matrix(D, D);
    Matrix S = A*matrix_transpose(A);
    MV_normal_distribution P(mu, S);

    // force cache
    log_pdf(P, mu);

    kjb_c::init_real_time();
    G = gradient_ffd_mt(
            bind(negative_log_pdf, cref(P), _1), x, vector<double>(D, dx));
    long t = kjb_c::get_real_time();

    double pgt = t / 1000.0;
    er = vector_distance(G, matrix_inverse(S)*(x - mu));

    kjb_c::init_real_time();
    M = gradient_ffd(
            bind(negative_log_pdf, cref(P), _1), x, vector<double>(D, dx));
    t = kjb_c::get_real_time();

    double sgt = t / 1000.0;
    er = vector_distance(G, M);
    if(VERBOSE)
    {
        cout << "f(x) = phi(x) (normal pdf)" << endl;
        //cout << "x = " << x << endl;
        //cout << "G = " << G << endl;
        cout << "Time for serial gradient: " << sgt << endl;
        cout << "Time for parallel gradient: " << pgt << endl;
        cout << endl;
    }

    TEST_TRUE(er <= eps);

#ifndef TEST
    TEST_TRUE(pgt < sgt);
#endif

    RETURN_VICTORIOUSLY();
}
