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

#include <density_cpp/density_laplace.h>
#include <m_cpp/m_matrix.h>
#include <m_cpp/m_vector.h>
#include <prob_cpp/prob_distribution.h>
#include <prob_cpp/prob_sample.h>
#include <diff_cpp/diff_hessian.h>
#include <l_cpp/l_test.h>
#include <vector>
#include <boost/bind.hpp>

using namespace std;
using namespace kjb;
using namespace boost;

/** @brief  Helper function. */
inline
double negative_log_pdf(const MV_normal_distribution& P, const Vector& x)
{
    return -log_pdf(P, x);
}

/** @brief  Main. */
int main(int argc, char** argv)
{
    try
    {
        const double eps = 1e-8;
        const double dx = 1e-3;
        const size_t D = 10;

        Vector mu = create_random_vector(D);
        Matrix A = create_random_matrix(D, D);
        Matrix S = A*matrix_transpose(A);
        MV_normal_distribution P(mu, S);

        Matrix H = matrix_inverse(S);
        double f = laplace_max_density(H);
        double lf = laplace_max_log_density(H);

        TEST_TRUE(fabs(f - pdf(P, mu)) <= eps);
        TEST_TRUE(lf - log_pdf(P, mu) <= log(1.0 + eps)
                    && lf - log_pdf(P, mu) >= log(1.0 - eps));

        H = hessian_symmetric(bind(negative_log_pdf, P, _1),
                              mu,
                              vector<double>(D, dx));

        f = laplace_max_density(H);
        lf = laplace_max_log_density(H);

        TEST_TRUE(fabs(f - pdf(P, mu)) <= eps);
        TEST_TRUE(lf - log_pdf(P, mu) <= log(1.0 + eps)
                    && lf - log_pdf(P, mu) >= log(1.0 - eps));
    }
    catch(const kjb::Exception& kex)
    {
        kex.print_details();
        return EXIT_FAILURE;
    }

    RETURN_VICTORIOUSLY();
}

