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

#include <diff_cpp/diff_optim.h>
#include <m_cpp/m_vector.h>
#include <l_cpp/l_test.h>
#include <prob_cpp/prob_distribution.h>
#include <prob_cpp/prob_pdf.h>
#include <iostream>
#include <vector>
#include <utility>
#include <boost/bind.hpp>

using namespace std;
using namespace kjb;
using namespace boost;

const bool VERBOSE = true;

double sdv1 = 0.4;
double sdv2 = 0.9;
double sdv3 = 0.7;

/** @brief  Normal pdf. Convenience function. */
inline
double normal_pdf(const Vector& x)
{
    static Normal_distribution N(0.0, sdv1);
    return pdf(N, x[0]);
}

/** @brief  Multivariate normal pdf. Convenience function. */
inline
double mv_normal_pdf(const Vector& x)
{
    static MV_normal_distribution N(Vector(3, 0.0),
            Vector().set(sdv1*sdv1, sdv2*sdv2, sdv3*sdv3));
    return pdf(N, x);
}

/** @brief  Main. */
int main(int argc, char** argv)
{
    const size_t nbins = 120;
    const double feps = 0.01;

    // test 1D function
    vector<pair<double, double> > bounds(1, make_pair(-6*sdv1, 6*sdv1));
    Vector xmx(bounds.size());
    double mx = grid_maximize(normal_pdf, bounds, nbins, xmx);
    const double eps1 = 12*sdv1/nbins;

    if(VERBOSE)
    {
        cout << "max: " << mx << endl;
        cout << "argmax: " << xmx << endl;
    }

    TEST_TRUE(fabs(mx - normal_pdf(Vector().set(0.0))) <= feps);
    TEST_TRUE(fabs(xmx[0] - 0.0) <= eps1);

    // test 3D function
    bounds.resize(3);
    bounds[0] = make_pair(-6*sdv1, 6*sdv1);
    bounds[1] = make_pair(-6*sdv2, 6*sdv2);
    bounds[2] = make_pair(-6*sdv3, 6*sdv3);
    const double eps2 = 12*sdv2/nbins;
    const double eps3 = 12*sdv3/nbins;

    xmx.resize(bounds.size());
    mx = grid_maximize(mv_normal_pdf, bounds, nbins, xmx);

    if(VERBOSE)
    {
        cout << "max: " << mx << endl;
        cout << "argmax: " << xmx << endl;
    }

    TEST_TRUE(fabs(mx - mv_normal_pdf(Vector(3, 0.0))) <= feps);
    TEST_TRUE(fabs(xmx[0] - 0.0) <= eps1);
    TEST_TRUE(fabs(xmx[1] - 0.0) <= eps2);
    TEST_TRUE(fabs(xmx[2] - 0.0) <= eps3);

    RETURN_VICTORIOUSLY();
}

