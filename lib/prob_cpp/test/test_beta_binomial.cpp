/* $Id: test_beta_binomial.cpp 20241 2016-01-20 22:34:46Z jguan1 $ */
/* {{{=========================================================================== *
   |
   |  Copyright (c) 1994-2012 by Kobus Barnard (author)
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
   |  Author:  Kyle Simek
 * =========================================================================== }}}*/

// vim: tabstop=4 shiftwidth=4 foldmethod=marker

#include <prob_cpp/prob_pdf.h>
#include <iostream>

int main()
{
    kjb::Matrix data("input/bbinopdf.mat");

    size_t errors = 0;
    for(size_t i = 0; i < data.get_num_rows(); ++i)
    {
        size_t k = data(i,0);
        size_t n = data(i,1);
        double a = data(i,2);
        double b = data(i,3);
        double p = data(i,4);

        kjb::Beta_binomial_distribution dist(n,a,b);
        double test_p = kjb::pdf(dist, k);

        if(fabs(test_p - p)/p > FLT_EPSILON)
        {
            ++errors;
            if(errors == 1)
                std::cerr << "First error found on iteration " << i << std::endl;
        }
    }

    if(errors == 0)
    {
        std::cout << "All tests passed." << std::endl;
        return 0;
    }
    else
    {
        std::cerr << "FAILED." << std::endl;
        std::cerr << errors << " errors out of " << data.get_num_rows()
                  << " tests." << std::endl;
        return 1;
    }
}

