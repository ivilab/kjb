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
#include <gp_cpp/gp_prior.h>
#include <gp_cpp/gp_likelihood.h>
#include <gp_cpp/gp_normal.h>
#include <prob_cpp/prob_distribution.h>
#include <m_cpp/m_vector.h>
#include <m_cpp/m_matrix.h>
#include <l_cpp/l_test.h>
#include <algorithm>

using namespace std;
using namespace kjb;

const bool VERBOSE = true;

typedef MV_normal_distribution Mvn;
typedef gp::Prior<gp::Zero, gp::Sqex> Prior;
typedef gp::Marginal_likelihood<gp::Zero, gp::Sqex, gp::Linear_gaussian> Mlh;

bool compare_normals(const Mvn& N1, const Mvn& N2)
{
    double md = max_abs_difference(N1.get_mean(), N2.get_mean());
    double cd = max_abs_difference(
                        N1.get_covariance_matrix(),
                        N2.get_covariance_matrix());

    return md < FLT_EPSILON && cd < FLT_EPSILON;
}

int main(int, char**)
{
    const size_t N = 100;

    // training data
    gp::Inputs X(N);
    Vector f(N);
    Vector y(N);
    for(size_t n = 0; n < N; n++)
    {
        X[n].set(n);
        f[n] = n;
        y[n] = f[n];
    }

    //// test simple things...
    //// add more later
    gp::Zero zero;
    gp::Sqex sqex(1.0, 1.0);
    Mlh ml = gp::make_marginal_likelihood(zero, sqex, 0.0, X);
    Prior prior = gp::make_prior(zero, sqex, X);

    // test zero-variance
    TEST_TRUE(compare_normals(ml.normal(), prior.normal()));

    // test diagonal variance
    const double sg = 1.0;
    const size_t D = ml.normal().get_dimension();
    gp::Linear_gaussian lg = ml.likelihood();
    lg.set_covariance(create_diagonal_matrix(D, sg*sg));
    ml.set_likelihood(lg);

    MV_normal_distribution P = prior.normal();
    P.set_covariance_matrix(P.get_covariance_matrix() + lg.covariance());

    TEST_TRUE(compare_normals(ml.normal(), P));

    RETURN_VICTORIOUSLY();
}

