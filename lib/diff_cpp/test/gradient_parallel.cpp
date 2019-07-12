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
   |  Author:  Ernesto Brau, Zewei Jiang
 * =========================================================================== */

/* $Id$ */

#include <diff_cpp/diff_gradient_mt.h>
#include <m_cpp/m_vector.h>
#include <m_cpp/m_matrix.h>
#include <l_cpp/l_test.h>
#include <prob_cpp/prob_distribution.h>
#include <prob_cpp/prob_pdf.h>
#include <l/l_sys_mal.h>
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
    size_t D;
    double er;

    ///////////////////////////////////
    // TEST SIMPLE 1-D FUNCTIONS
    ///////////////////////////////////

    // f(x) = 2x
    x.set(2.0);
    G = gradient_cfd_mt(two_x, x, vector<double>(1, dx));

    if(VERBOSE)
    {
        cout << "f(x) = 2x" << endl;
        cout << "x = " << x << endl;
        cout << "G = " << G << endl;
        cout << endl;
    }

    TEST_TRUE(G.get_length() == 1);
    TEST_TRUE(fabs(G[0] - 2) <= eps);

    // f(x) = x^3
    G = gradient_cfd_mt(x_cubed, x, vector<double>(1, dx));

    if(VERBOSE)
    {
        cout << "f(x) = x^3" << endl;
        cout << "x = " << x << endl;
        cout << "G = " << G << endl;
        cout << endl;
    }

    TEST_TRUE(G.get_length() == 1);
    TEST_TRUE(fabs(G[0] - 3*x[0]*x[0]) <= eps);


    ///////////////////////////////////
    // TEST SIMPLE N-D FUNCTIONS
    ///////////////////////////////////

    // f(x) = x^2
    D = 5;
    x = create_random_vector(D);
    G = gradient_cfd_mt(x_squared, x, vector<double>(D, dx));
    er = vector_distance(G, 2*x);

    if(VERBOSE)
    {
        cout << "f(x) = x^2" << endl;
        cout << "x = " << x << endl;
        cout << "G = " << G << endl;
        cout << "||G - D(f)|| = " << er << endl;
        cout << endl;
    }

    TEST_TRUE(G.get_length() == D);
    TEST_TRUE(er <= eps);


    ///////////////////////////////////
    // TEST GRADIENT ON -LOG OF NORMAL
    ///////////////////////////////////

    // f(x) = phi(x) (normal pdf)
    D = 100;
    Vector mu = create_random_vector(D);
    Matrix A = create_random_matrix(D, D);
    Matrix S = A*matrix_transpose(A);

    MV_normal_distribution P(mu, S);
    x = 2*mu;

    // force cache
    log_pdf(P, mu);

    G = gradient_cfd_mt(
                bind(negative_log_pdf, cref(P), _1),
                x, vector<double>(D, dx));

    er = vector_distance(G, matrix_inverse(S)*(x - mu));

    if(VERBOSE)
    {
        cout << "f(x) = phi(x) (normal pdf)" << endl;
        cout << "||G - D(f)|| = " << er << endl;
        cout << endl;
    }

    TEST_TRUE(er <= eps);

    ///////////////////////////////////
    // TEST GRADIENT ON -LOG OF NORMAL
    ///////////////////////////////////

    // f(x) = phi(x) (normal pdf)
    D = 100;
    mu = create_random_vector(D);
    Vector s = 10*create_random_vector(D);
    MV_normal_distribution Q(mu, s);
    x = 2*mu;

    // force cache
    log_pdf(Q, mu);

    G = gradient_ind_cfd_mt(
                    bind(negative_log_pdf_ind, cref(Q), _1, _2),
                    x, vector<double>(D, dx));

    S = Q.get_covariance_matrix();
    er = vector_distance(G, matrix_inverse(S)*(x - mu));

    if(VERBOSE)
    {
        cout << "f(x) = phi(x) (normal pdf)" << endl;
        cout << "||G - D(f)|| = " << er << endl;
        cout << endl;
    }

    TEST_TRUE(er <= eps);

    RETURN_VICTORIOUSLY();
}

