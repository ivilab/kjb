/* $Id: pgpe.cpp 18546 2015-02-09 20:41:08Z jguan1 $ */
/* =========================================================================== *
   |
   |  Copyright (c) 1994-2010 by Kobus Barnard (author)
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
   |  Author:  Jinyan Guan
 * =========================================================================== */

#include <l_cpp/l_test.h>
#include <m_cpp/m_vector.h>
#include <opt_cpp/opt_pgpe.h>

using namespace kjb;

class Evaluator
{
public:
    Evaluator(const Vector& data) :
        data_(data)
    {}

    double operator()(Vector params)
    {
        IFT(params.size() == data_.size(), Illegal_argument, 
               "dimention mismatch."); 
        double s = 0.0;
        for (unsigned int i = 0; i < params.size(); i++)
        {
            s += std::pow((params[i] - data_[i]), 2.0);
        }
        s = std::pow(s / params.size(), 0.5);
        return -s;
    }
private:
    Vector data_;
};

const bool VERBOSE = false;

int main (int argc, char ** argv)
{
    const int num_params = 10;
    Vector vals(num_params, 0.0);
    Vector stds(num_params, 1e10);

    Vector data(num_params, 0.0);
    kjb_c::kjb_seed_rand_with_tod();
    for(size_t i = 0; i < data.size(); i++)
    {
        data[i] = 1000.0 + kjb_c::kjb_rand() * 1.0;
    }
    if(VERBOSE)
        std::cout << "data: " << data << std::endl;

    Evaluator e(data); 
    std::string log_fp("./log.txt");
    if(VERBOSE)
        std::cout << "init score: " << e(vals) << std::endl;
    Pgpe<Evaluator> pgpe(vals, stds, e, 0.2, 0.2, 10000, log_fp);
    std::string params_fp("./params.txt");
    pgpe.run(params_fp);

    if(VERBOSE)
    {
        std::cout << "fitting params: " <<  pgpe.get_best_parameters() 
                  << std::endl;
        std::cout << "best score: " << pgpe.get_best_score()<<std::endl;
    }
    const double eps = 1e-5;
    TEST_TRUE(fabs(vector_distance(data, pgpe.get_best_parameters())) < eps);

    RETURN_VICTORIOUSLY();
    
}

