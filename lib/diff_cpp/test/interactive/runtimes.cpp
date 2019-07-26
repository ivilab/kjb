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

#include <diff_cpp/diff_gradient.h>
#include <diff_cpp/diff_gradient_mt.h>
#include <m_cpp/m_vector.h>
#include <m_cpp/m_matrix.h>
#include <l_cpp/l_test.h>
#include <prob_cpp/prob_distribution.h>
#include <prob_cpp/prob_pdf.h>
#include <prob_cpp/prob_sample.h>
#include <l/l_sys_mal.h>
#include <l/l_sys_time.h>
#include <iostream>
#include <vector>
#include <algorithm>
#include <boost/bind.hpp>
#include <boost/ref.hpp>

using namespace std;
using namespace kjb;
using namespace boost;

Normal_distribution get_random_normal();
double nlpdfs(const vector<Normal_distribution>&, const Vector&);
double nlpdfs1(const vector<Normal_distribution>&, const Vector&, size_t);

/** @brief  Main. */
int main(int argc, char** argv)
{
    kjb_c::kjb_l_set("heap-checking", "off");
    const size_t nt = 12;

    const double eps = 1e-5;
    const size_t D = 100;
    const size_t N = 10000;

    // create normal distros
    vector<Normal_distribution> Ps(N);
    generate(Ps.begin(), Ps.end(), get_random_normal);

    Vector x = create_random_vector(D);
    vector<double> dx(D, 0.0001);

    kjb_c::init_real_time();
    Vector G0 = gradient_cfd(bind(nlpdfs, cref(Ps), _1), x, dx);
    double t0 = kjb_c::get_real_time() / 1000.0;

    kjb_c::init_real_time();
    Vector G1 = gradient_cfd_mt(bind(nlpdfs, cref(Ps), _1), x, dx, nt);
    double t1 = kjb_c::get_real_time() / 1000.0;

    kjb_c::init_real_time();
    Vector G2 = gradient_ind_cfd(bind(nlpdfs1, cref(Ps), _1, _2), x, dx);
    double t2 = kjb_c::get_real_time() / 1000.0;

    kjb_c::init_real_time();
    Vector G3 = gradient_ind_cfd_mt(bind(nlpdfs1, cref(Ps), _1, _2), x, dx, nt);
    double t3 = kjb_c::get_real_time() / 1000.0;

    cout << "D = " << D << endl;
    cout << "N = " << N << endl;
    cout << "Serial gradient: " << t0 << endl;
    cout << "MT gradient: " << t1 << endl;
    cout << "Serial Independent gradient: " << t2 << endl;
    cout << "MT independent gradient: " << t3 << endl;

    // easy test...just in case
    G1.normalize();
    G2.normalize();
    G3.normalize();
    TEST_TRUE(vector_distance(G1, G2) <= eps);
    TEST_TRUE(vector_distance(G2, G3) <= eps);

    return EXIT_SUCCESS;
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

Normal_distribution get_random_normal()
{
    double mu = sample(Uniform_distribution(-5.0, 5.0));
    double s = sample(Uniform_distribution(0.0, 10.0));

    return Normal_distribution(mu, s);
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

double nlpdfs(const vector<Normal_distribution>& Ps, const Vector& x)
{
    const size_t D = x.size();
    const size_t N = Ps.size();

    double p = 0.0;
    for(size_t d = 0; d < D; d++)
    {
        for(size_t n = 0; n < N; n++)
        {
            p += log_pdf(Ps[n], x[d]);
        }
    }

    return -p;
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

double nlpdfs1(const vector<Normal_distribution>& Ps, const Vector& x, size_t d)
{
    const size_t N = Ps.size();

    double p = 0.0;
    for(size_t n = 0; n < N; n++)
    {
        p += log_pdf(Ps[n], x[d]);
    }

    return -p;
}

