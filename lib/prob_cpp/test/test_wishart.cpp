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

/* $Id: test_wishart.cpp 20241 2016-01-20 22:34:46Z jguan1 $ */

#include <prob_cpp/prob_distribution.h>
#include <prob_cpp/prob_pdf.h>
#include <prob_cpp/prob_sample.h>
#include <m/m_mat_metric.h>
#include <l_cpp/l_test.h>

using namespace kjb;

int main(int argc, char** argv)
{
    const size_t D = 4;
    double nu = 10;
    Matrix r = create_random_matrix(D, D);
    Matrix S = r * r.transpose();

    //std::cout << S << std::endl;

    Wishart_distribution wd(nu, S);
    Matrix sample_mean = create_zero_matrix(D, D);
    const size_t num_samples = 1000;
    for(size_t i = 0; i < num_samples; i++)
    {
        Matrix x = sample(wd);
        TEST_TRUE(fabs(pdf(wd, x) - exp(log_pdf(wd, x))) < FLT_EPSILON);
        //std::cout << pdf(wd, x) << " " << log_pdf(wd, x) <<  " " << exp(log_pdf(wd, x)) << std::endl;
        sample_mean += x;
    }

    sample_mean /= (num_samples - 1);
    Matrix mean = nu * S;
    //std::cout << sample_mean << std::endl;
    //std::cout << mean << std::endl;

    TEST_TRUE(kjb_c::rms_matrix_difference(sample_mean.get_c_matrix(), 
                              mean.get_c_matrix()) < 1.0);
    //std::cout << x << std::endl;

    return EXIT_SUCCESS;
}
