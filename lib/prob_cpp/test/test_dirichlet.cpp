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

/* $Id: test_dirichlet.cpp 20241 2016-01-20 22:34:46Z jguan1 $ */

#include <prob_cpp/prob_distribution.h>
#include <prob_cpp/prob_pdf.h>
#include <prob_cpp/prob_sample.h>
#include <l_cpp/l_test.h>

#include <vector>

using namespace kjb;

int main(int argc, char** argv)
{
    const int D = 4;
    std::vector<double> alphas(D, 0.25);
    Dirichlet_distribution dist(alphas);
    
    const size_t num_samples = 1000;
    Vector sample_mean(D, 0.0);
    for(size_t i = 0; i < num_samples; i++)
    {
        Vector x = sample(dist);
        sample_mean += x;
        //std::cout << pdf(dist, x) << " : " << exp(log_pdf(dist, x)) << std::endl;
        TEST_TRUE(fabs(pdf(dist, x) - exp(log_pdf(dist, x)))/pdf(dist, x) < FLT_EPSILON);
    }
    sample_mean /= (num_samples - 1);
    Vector mean(alphas.begin(), alphas.end());
    mean /= mean.sum_vector_elements(); 
    //std::cout << sample_mean << " : " << mean << std::endl;
    TEST_TRUE(vector_distance(sample_mean, mean) < 1.0);
    return EXIT_SUCCESS;
}
