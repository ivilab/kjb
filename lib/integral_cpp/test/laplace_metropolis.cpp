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

#include <integral_cpp/integral_marginal.h>
#include <m_cpp/m_vector.h>
#include <m_cpp/m_matrix.h>
#include <l_cpp/l_test.h>
#include <prob_cpp/prob_distribution.h>
#include <prob_cpp/prob_pdf.h>
#include <prob_cpp/prob_sample.h>
#include <iostream>

using namespace std;
using namespace kjb;

/** @brief  Normal prior. */
inline
double p(const MV_normal_distribution& P, const Vector& t)
{
    return pdf(P, t);
}

/** @brief  Normal likelihood. */
inline
double f(const MV_normal_distribution& F, const Vector& t)
{
    return pdf(F, t);
}

/** @brief  Normal prior (log). */
inline
double log_p(const MV_normal_distribution& P, const Vector& t)
{
    return log_pdf(P, t);
}

/** @brief  Normal likelihood (log). */
inline
double log_f(const MV_normal_distribution& F, const Vector& t)
{
    return log_pdf(F, t);
}

/** @brief  Main. */
int main(int argc, char** argv)
{
    // initialize stuff
    const size_t D = 10;
    const double eps = 1e-10;

    // prior
    Vector mu_p = create_random_vector(D);

    Matrix A = 10.0*create_random_matrix(D, D);
    Matrix S_p = A*matrix_transpose(A);

    MV_normal_distribution P(mu_p, S_p);

    // Some data from the prior
    Vector x = sample(P);

    // likelihood
    double sg = 0.01;
    Matrix S_f = sg*create_identity_matrix(D);

    MV_normal_distribution F(x, S_f);

    // marginal likelihood
    Matrix S_g = S_p + S_f;
    MV_normal_distribution G(mu_p, S_g);
    double g_x = pdf(G, x);
    double log_g_x = log_pdf(G, x);

    // posterior
    Vector mu_pi = mu_p + S_p*matrix_inverse(S_g)*(x - mu_p);
    Matrix S_pi = S_p - S_p*matrix_inverse(S_g)*S_p;
    Matrix S_pi_inv = matrix_inverse(S_pi);
    double g_lm_x = lm_marginal_likelihood(
                                        mu_pi,
                                        bind(p, P, _1),
                                        bind(f, F, _1),
                                        S_pi_inv);

    double log_g_lm_x = lm_marginal_log_likelihood(
                                                mu_pi,
                                                bind(log_p, P, _1),
                                                bind(log_f, F, _1),
                                                S_pi_inv);

    TEST_TRUE(fabs(g_x - g_lm_x) <= eps);
    TEST_TRUE(fabs(log_g_x - log_g_lm_x) <= fabs(log(eps)));

    RETURN_VICTORIOUSLY();
}

