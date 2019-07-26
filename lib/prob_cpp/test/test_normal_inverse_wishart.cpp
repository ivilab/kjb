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

/* $Id: test_normal_inverse_wishart.cpp 20241 2016-01-20 22:34:46Z jguan1 $ */

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
    const double kappa = 100.0;
    double nu = 10;
    Matrix r = create_random_matrix(D, D);
    Matrix S = r * r.transpose();

    Normal_inverse_wishart_distribution niw(mu, kappa, S.inverse(), nu);
    Vector mean_sample_mean((int)D, 0.0);
    Matrix sigma_sample_mean = create_zero_matrix(D, D);

    const size_t num_samples = 10000;
    for(size_t i = 0; i < num_samples; i++)
    {
        std::pair<Vector, Matrix> x = sample(niw);
        TEST_TRUE(fabs(pdf(niw, x.first, x.second) -
                    exp(log_pdf(niw, x.first, x.second))) < FLT_EPSILON);
        mean_sample_mean += x.first;
        sigma_sample_mean += x.second;
    }

    mean_sample_mean /= (num_samples);
    sigma_sample_mean /= (num_samples);
    Matrix sigma_mean = S.inverse()/(nu - D - 1);

    //std::cout << "sample: " << mean_sample_mean << std::endl;
    //std::cout << "  real: " << mu << std::endl << std::endl;

    //std::cout << "sample: " << sigma_sample_mean << std::endl;
    //std::cout << "  real: " << sigma_mean << std::endl;

    TEST_TRUE(kjb_c::rms_matrix_difference(sigma_sample_mean.get_c_matrix(), 
                                           sigma_mean.get_c_matrix()) < 1.0);
    TEST_TRUE(vector_distance(mean_sample_mean, mu) < 1.0);
    //std::cout << x << std::endl;

    return EXIT_SUCCESS;
}
