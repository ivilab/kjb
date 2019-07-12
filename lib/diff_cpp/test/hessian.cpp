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
    const double eps = 1e-4;

    ///////////////////////////////////
    // TEST SIMPLE 1-D FUNCTIONS
    ///////////////////////////////////

    Vector x = Vector().set(2.0);
    Matrix H = hessian(two_x, x, vector<double>(1, dx));
    TEST_TRUE(H.get_num_rows() == 1 && H.get_num_cols() == 1);
    TEST_TRUE(fabs(H(0, 0) - 0) <= eps);

    H = hessian(x_cubed, x, vector<double>(1, dx));
    TEST_TRUE(H.get_num_rows() == 1 && H.get_num_cols() == 1);
    TEST_TRUE(fabs(H(0, 0) - 6*x[0]) <= eps);


    ///////////////////////////////////
    // TEST SIMPLE N-D FUNCTIONS
    ///////////////////////////////////
    size_t D = 5;
    x = create_random_vector(D);
    H = hessian(x_squared, x, vector<double>(D, dx));
    TEST_TRUE(H.get_num_rows() == D && H.get_num_cols() == D);
    TEST_TRUE(max_abs_difference(H, create_diagonal_matrix(D, 2.0)) <= eps);


    ///////////////////////////////////
    // TEST HESSIAN ON -LOG OF NORMAL
    ///////////////////////////////////

    // distribution
    D = 20;
    Vector mu = create_random_vector(D);
    Matrix A = create_random_matrix(D, D);
    Matrix S = A*matrix_transpose(A);
    MV_normal_distribution P(mu, S);

    // hessian is -inv(cov)
    H = hessian(bind(negative_log_pdf, P, _1), mu, vector<double>(D, dx));
    Matrix H_inv = matrix_inverse(H);
    Matrix S_inv = matrix_inverse(S);
    MV_normal_distribution Q(mu, H_inv);

    // test symmetric Hessian
    Matrix K = hessian_symmetric(bind(negative_log_pdf, P, _1),
                                 mu, vector<double>(D, dx));

    // test for equality of H and inv(S),
    // equality of H and K
    // equality of determinants,
    // and normal pdf values
    TEST_TRUE(max_abs_difference(H, S_inv) <= eps);
    TEST_TRUE(max_abs_difference(H, K) <= eps);
    TEST_TRUE(fabs(H_inv.abs_of_determinant() - S.abs_of_determinant()) <= eps);
    TEST_TRUE(fabs(negative_log_pdf(P, mu) - negative_log_pdf(Q, mu))
                                                    <= fabs(log(eps)));

    RETURN_VICTORIOUSLY();
}

