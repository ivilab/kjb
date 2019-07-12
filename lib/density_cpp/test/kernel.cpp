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

#include <density_cpp/density_kernel.h>
#include <m_cpp/m_vector.h>
#include <prob_cpp/prob_distribution.h>
#include <prob_cpp/prob_sample.h>
#include <l_cpp/l_test.h>
#include <vector>
#include <algorithm>
#include <boost/bind.hpp>

using namespace std;
using namespace kjb;
using namespace boost;

int main(int argc, char** argv)
{
    // typedef the multivariate normal pdf for easy binding
    typedef double (*Norm_pdf)(const Normal_distribution&, double);
    typedef double (*Norm_sample)(const Normal_distribution&);

    // setup
    const size_t N = 100000;

    double mu = sample(Uniform_distribution(-10, 10));
    double sg = sample(Uniform_distribution(0, 3));
    Normal_distribution P(mu, sg);

    vector<double> x(N);
    generate(x.begin(), x.end(), bind(static_cast<Norm_sample>(sample), P));

    // test for all support
    Normal_distribution K;

    const double dy = 6*sg / 100;
    for(double y = mu - 3*sg; y <= mu + 3*sg; y+= dy)
    {
        // real pdf
        double f = pdf(P, y);

        // estimated pdf
        double f_de = fkde_normal(y, x.begin(), x.end(),
                                  bind(static_cast<Norm_pdf>(pdf), K, _1), sg);

        // they should be very close: using normal reference rule on a
        // normal pdf with many samples with exact sigma
        TEST_TRUE(fabs(1.0 - f/f_de) <= 0.20);
    }

    RETURN_VICTORIOUSLY();
}

