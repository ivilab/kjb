/* =========================================================================== *
|
|  Copyright (c) 1994-2008 by Kobus Barnard (author).
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
| Authors:
|     Jinyan Guan
|
* =========================================================================== */

/* $Id: test_normal_inverse_gamma.cpp 20241 2016-01-20 22:34:46Z jguan1 $ */

#include <prob_cpp/prob_distribution.h>
#include <prob_cpp/prob_pdf.h>
#include <prob_cpp/prob_sample.h>
#include <m/m_mat_metric.h>
#include <l_cpp/l_test.h>

using namespace kjb;

int main(int argc, char** argv)
{
    const size_t D = 4;
    Vector mu((int)D, 0.0);
    Matrix r = create_random_matrix(D, D);
    Matrix S = r * r.transpose();
    const double a = 10.0;
    const double b = 2.0;

    Normal_inverse_gamma_distribution nig(mu, S, a, b);
    Vector mean_sample_mean((int)D, 0.0);
    double sigma_sample_mean = 0.0;

    const size_t num_samples = 10000;
    for(size_t i = 0; i < num_samples; i++)
    {
        std::pair<Vector, double> x = sample(nig);
        TEST_TRUE(fabs(pdf(nig, x.first, x.second) -
                    exp(log_pdf(nig, x.first, x.second))) < FLT_EPSILON);
        mean_sample_mean += x.first;
        sigma_sample_mean += x.second;
    }

    mean_sample_mean /= (num_samples);
    sigma_sample_mean /= (num_samples);
    double sigma_mean = b/(a - 1);

    //std::cout << "sample: " << mean_sample_mean << std::endl;
    //std::cout << "  real: " << mu << std::endl << std::endl;
    //std::cout << vector_distance(mean_sample_mean, mu) << std::endl;

    //std::cout << "sample: " << sigma_sample_mean << std::endl;
    //std::cout << "  real: " << sigma_mean << std::endl;

    TEST_TRUE(fabs(sigma_sample_mean - sigma_mean) < 1e-3);
    TEST_TRUE(vector_distance(mean_sample_mean, mu) < sigma_mean);
    //std::cout << x << std::endl;

    return EXIT_SUCCESS;
}
