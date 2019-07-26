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

#include <diff_cpp/diff_hessian.h>
#include <diff_cpp/diff_hessian_ind.h>
#include <m_cpp/m_vector.h>
#include <m_cpp/m_matrix.h>
#include <l_cpp/l_test.h>
#include <prob_cpp/prob_distribution.h>
#include <prob_cpp/prob_pdf.h>
#include "funcs.h"
#include <iostream>
#include <vector>
#include <boost/bind.hpp>

using namespace std;
using namespace kjb;
using namespace boost;

/** @brief  Main. */
int main(int argc, char** argv)
{
    const double dx = 0.00001;
    const double eps = 1e-2;

    ///////////////////////////////////
    // TEST SIMPLE N-D FUNCTIONS
    ///////////////////////////////////
    size_t D = 5;
    Vector x = create_random_vector(D);
    Matrix Hn = hessian(x_squared, x, vector<double>(D, dx));
    Matrix Hi = hessian_ind(x_squared_ind_2, x, vector<double>(D, dx));
    TEST_TRUE(Hn.get_num_rows() == Hi.get_num_rows());
    TEST_TRUE(Hn.get_num_cols() == Hi.get_num_cols());
    TEST_TRUE(max_abs_difference(Hn, Hi) <= eps);

    ///////////////////////////////////
    // TEST HESSIAN ON -LOG OF NORMAL
    ///////////////////////////////////

    // distribution
    D = 80;
    Vector mu = create_random_vector(D);
    Vector s = 10*create_random_vector(D);
    MV_normal_distribution P(mu, s);

    // hessians should be equal
    Hn = hessian(bind(negative_log_pdf, P, _1), mu, vector<double>(D, dx));
    Hi = hessian_ind(
            bind(negative_log_pdf_ind_2, P, _1, _2, _3),
            mu, vector<double>(D, dx));

    // test symmetric Hessian
    Matrix K = hessian_symmetric_ind(
                    bind(negative_log_pdf_ind_2, P, _1, _2, _3),
                    mu, vector<double>(D, dx));

    // test diagonal Hessian
    Vector h = hessian_ind_diagonal(
                    bind(negative_log_pdf_ind, P, _1, _2),
                    mu, vector<double>(D, dx), 0, D - 1);
    Matrix Hd = create_diagonal_matrix(h);

    // test for equality of Hn and Hi
    // equality of Hn and K
    TEST_TRUE(max_abs_difference(Hn, Hi) <= eps);
    TEST_TRUE(max_abs_difference(Hn, K) <= eps);
    TEST_TRUE(max_abs_difference(Hn, Hd) <= eps);

    RETURN_VICTORIOUSLY();
}


