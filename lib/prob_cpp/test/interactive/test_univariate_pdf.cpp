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
|
* =========================================================================== */

/* $Id: test_univariate_pdf.cpp 20241 2016-01-20 22:34:46Z jguan1 $ */

#include "prob_cpp/prob_distribution.h"
#include "prob_cpp/prob_pdf.h"
#include "l_cpp/l_functors.h"
#include <string>
#include <vector>
#include <algorithm>
#include <functional>
#include <iterator>
#include <iostream>
#include <stdexcept>

using namespace std;
using namespace kjb;

template<class D>
struct pdf_helper
{
    D d;

    pdf_helper(const D& dd) : d(dd)
    {}

    double operator()(const typename Distribution_traits<D>::type& x)
    {
        try
        {
            return pdf(d, x);
        }
        catch(const std::domain_error& err)
        {
            return 0.0;
        }
    }
};

int main(int argc, char** argv)
{
    if(argc != 2)
    {
        cout << "Please specify distribution from:\n";
        cout << "    bernoulli\n"
             << "    beta\n"
             << "    binomial\n"
             << "    categorical-double\n"
             << "    categorical-int\n"
             << "    chi-square\n"
             << "    exponential\n"
             << "    gamma\n"
             << "    gaussian\n"
             << "    laplace\n"
             << "    poisson\n"
             << "    uniform\n"
             << "    mixture\n"
             << "To check correctness, plot output using Gnuplot or "
             << "Octave (or Matlab if you must).\n";
        return EXIT_SUCCESS;
    }

    vector<double> y;
    vector<double> x;

    string dist = argv[1];

    if(dist == "bernoulli")
    {
        x.push_back(0);
        x.push_back(1);
        y.resize(x.size());
        transform(x.begin(), x.end(), y.begin(),
            pdf_helper<Bernoulli_distribution>(Bernoulli_distribution(0.2)));
    }
    else if(dist == "beta")
    {
        x.resize(20);
        generate(x.begin(), x.end(), Increase_by<double>(0.0, 0.05));
        y.resize(x.size());
        transform(x.begin(), x.end(), y.begin(),
            pdf_helper<Beta_distribution>(Beta_distribution(2, 5)));
    }
    else if(dist == "binomial")
    {
        x.resize(51);
        generate(x.begin(), x.end(), Increment<double>());
        y.resize(x.size());
        transform(x.begin(), x.end(), y.begin(),
            pdf_helper<Binomial_distribution>(Binomial_distribution(50, 0.2)));
    }
    else if(dist == "categorical-double")
    {
        x.resize(4);
        x[0] = 0.1;
        x[1] = 0.2;
        x[2] = 0.3;
        x[3] = 0.4;
        y.resize(x.size());

        map<double, double> m;
        m[0.1] = 0.1;
        m[0.2] = 0.2;
        m[0.3] = 0.3;
        m[0.4] = 0.4;
        transform(x.begin(), x.end(), y.begin(),
            pdf_helper<Categorical_distribution<double> >(Categorical_distribution<double>(m)));
    }
    else if(dist == "categorical-int")
    {
        x.resize(4);
        generate(x.begin(), x.end(), Increment<int>(1));
        y.resize(x.size());
        transform(x.begin(), x.end(), y.begin(),
            pdf_helper<Categorical_distribution<int> >(Categorical_distribution<int>(Vector().set(0.2, 0.1, 0.05, 0.65))));
    }
    else if(dist == "chi-square")
    {
        x.resize(100);
        generate(x.begin(), x.end(), Increase_by<double>(0.0, 0.05));
        y.resize(x.size());
        transform(x.begin(), x.end(), y.begin(),
            pdf_helper<Chi_square_distribution>(Chi_square_distribution(3)));
    }
    else if(dist == "exponential")
    {
        x.resize(100);
        generate(x.begin(), x.end(), Increase_by<double>(0.0, 0.05));
        y.resize(x.size());
        transform(x.begin(), x.end(), y.begin(),
            pdf_helper<Exponential_distribution>(Exponential_distribution(1.5)));
    }
    else if(dist == "gamma")
    {
        x.resize(400);
        generate(x.begin(), x.end(), Increase_by<double>(0.0, 0.05));
        y.resize(x.size());
        transform(x.begin(), x.end(), y.begin(),
            pdf_helper<Gamma_distribution>(Gamma_distribution(2, 2.0)));
    }
    else if(dist == "gaussian" || dist == "normal")
    {
        x.resize(200);
        generate(x.begin(), x.end(), Increase_by<double>(-5.0, 0.05));
        y.resize(x.size());
        transform(x.begin(), x.end(), y.begin(),
            pdf_helper<Normal_distribution>(Normal_distribution(0.0, 1.0)));
    }
    else if(dist == "laplace")
    {
        x.resize(200);
        generate(x.begin(), x.end(), Increase_by<double>(-5.0, 0.05));
        y.resize(x.size());
        transform(x.begin(), x.end(), y.begin(),
            pdf_helper<Laplace_distribution>(Laplace_distribution(0.0, 1.0)));
    }
    else if(dist == "poisson")
    {
        x.resize(50);
        generate(x.begin(), x.end(), Increment<double>());
        y.resize(x.size());
        transform(x.begin(), x.end(), y.begin(),
            pdf_helper<Poisson_distribution>(Poisson_distribution(4)));
    }
    else if(dist == "uniform")
    {
        x.resize(20);
        generate(x.begin(), x.end(), Increase_by<double>(0.0, 0.05));
        y.resize(x.size());
        transform(x.begin(), x.end(), y.begin(),
            pdf_helper<Uniform_distribution>(Uniform_distribution()));
    }
    else if(dist == "mixture")
    {
        x.resize(200);
        generate(x.begin(), x.end(), Increase_by<double>(-5.0, 0.05));
        y.resize(x.size());
        transform(x.begin(), x.end(), y.begin(),
            pdf_helper<Mixture_distribution<Normal_distribution> >(
                    Mixture_distribution<Normal_distribution>(
                        Normal_distribution(0.0, 1.0),
                        Normal_distribution(-1.0, 0.4),
                        0.2)
                    )
                );
    }
    else
    {
        cout << "Distribution not recognized.\n\n";
        return EXIT_SUCCESS;
    }

    copy(y.begin(), y.end(), ostream_iterator<double>(cout, "\n"));
}

