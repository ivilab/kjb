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
   |  Author:  Ernesto Brau
 * =========================================================================== */

#include <gp_cpp/gp_base.h>
#include <gp_cpp/gp_mean.h>
#include <gp_cpp/gp_covariance.h>
#include <gp_cpp/gp_predictive.h>
#include <prob_cpp/prob_distribution.h>
#include <m_cpp/m_vector.h>
#include <m_cpp/m_matrix.h>
#include <l_cpp/l_test.h>
#include <algorithm>
#include <boost/bind.hpp>
#include <boost/ref.hpp>
#include <iostream>

using namespace std;
using namespace kjb;

const bool VERBOSE = true;

typedef gp::Predictive_nl<gp::Manual, gp::Squared_exponential> Pred;

int main(int, char**)
{
    const size_t N = 100;

    // training data
    gp::Inputs X(N);
    Vector f(N);
    for(size_t n = 0; n < N; n++)
    {
        X[n].set(n);
        f[n] = n;
    }

    // (single) test input
    Vector x_s = Vector().set(N);

    // create predictive
    Vector mu = create_random_vector(N + 1);
    gp::Manual m1(X, Vector(mu.begin(), mu.end() - 1));
    m1.add(x_s, mu[N]);
    gp::Squared_exponential sqex(1.0, 1.0);

    Pred pred(m1, sqex, X.begin(), X.end(), f.begin(), f.end(), &x_s, &x_s + 1);
    const MV_normal_distribution& P = pred.normal();

    ////// test single-test mean
    double eps = 1e-5;

    // test size
    TEST_TRUE(P.get_dimension() == 1);

    // compute it specifically for a single test input
    Vector k_s(N);
    transform(
        X.begin(),
        X.end(),
        k_s.begin(),
        boost::bind<double>(boost::ref(sqex), x_s, _1));
    Matrix K = apply_cf(sqex, X.begin(), X.end());
    Vector m(mu.begin(), mu.end() - 1);
    double mu_s = mu[N] + dot(k_s, matrix_inverse(K) * (f - m));

    // compare with Pred's, which is computed in general
    if(VERBOSE) cout << P.get_mean()[0] << " == " << mu_s << endl;
    TEST_TRUE(fabs(P.get_mean()[0] - mu_s) < eps);

    ////// test single-test variance
    // compute it specifically for a single test input
    double k_ss = sqex(x_s, x_s);
    double v_s = k_ss - dot(k_s, matrix_inverse(K) * k_s);

    // compare with Pred's, which is computed in general
    if(VERBOSE) cout << P.get_covariance_matrix()(0, 0) << " == " << v_s << endl;
    TEST_TRUE(fabs(P.get_covariance_matrix()(0, 0) - v_s) < eps);

    RETURN_VICTORIOUSLY();
}

