#include "prob_cpp/prob_distribution.h"
#include "prob_cpp/prob_sample.h"
#include <string>
#include <vector>
#include <algorithm>
#include <functional>
#include <iterator>
#include <iostream>

using namespace std;
using namespace kjb;

template<class D>
struct sample_helper
{
    D d;

    sample_helper(const D& dd) : d(dd)
    {}

    double operator()()
    {
        return sample(d);
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
             << "To check correctness, plot a histogram of the output "
             << "using Gnuplot or Octave (or Matlab if you must).\n";
        return EXIT_SUCCESS;
    }

    vector<double> samples(5000);

    string dist = argv[1];

    if(dist == "bernoulli")
    {
        generate(samples.begin(), samples.end(),
            sample_helper<Bernoulli_distribution>(Bernoulli_distribution(0.2)));
    }
    else if(dist == "beta")
    {
        generate(samples.begin(), samples.end(),
            sample_helper<Beta_distribution>(Beta_distribution(2, 5)));
    }
    else if(dist == "binomial")
    {
        generate(samples.begin(), samples.end(),
            sample_helper<Binomial_distribution>(
                    Binomial_distribution(50, 0.2)));
    }
    else if(dist == "categorical-double")
    {
        map<double, double> m;
        m[0.1] = 0.1;
        m[0.2] = 0.2;
        m[0.3] = 0.3;
        m[0.4] = 0.4;
        generate(samples.begin(), samples.end(),
            sample_helper<Categorical_distribution<double> >(m));
    }
    else if(dist == "categorical-int")
    {
        generate(samples.begin(), samples.end(),
            sample_helper<Categorical_distribution<int> >(
                        Vector().set(0.2, 0.1, 0.05, 0.65)));
    }
    else if(dist == "chi-square")
    {
        generate(samples.begin(), samples.end(),
            sample_helper<Chi_square_distribution>(Chi_square_distribution(3)));
    }
    else if(dist == "exponential")
    {
        generate(samples.begin(), samples.end(),
            sample_helper<Exponential_distribution>(
                        Exponential_distribution(1.5)));
    }
    else if(dist == "gamma")
    {
        generate(samples.begin(), samples.end(),
            sample_helper<Gamma_distribution>(Gamma_distribution(2, 2.0)));
    }
    else if(dist == "gaussian" || dist == "normal")
    {
        generate(samples.begin(), samples.end(),
            sample_helper<Normal_distribution>(Normal_distribution(0.0, 1.0)));
    }
    else if(dist == "laplace")
    {
        generate(samples.begin(), samples.end(),
            sample_helper<Laplace_distribution>(Laplace_distribution(0.0, 1.0)));
    }
    else if(dist == "poisson")
    {
        generate(samples.begin(), samples.end(),
            sample_helper<Poisson_distribution>(Poisson_distribution(4)));
    }
    else if(dist == "uniform")
    {
        generate(samples.begin(), samples.end(),
            sample_helper<Uniform_distribution>(Uniform_distribution()));
    }
    else if(dist == "mixture")
    {
        generate(samples.begin(), samples.end(),
            sample_helper<Mixture_distribution<Normal_distribution> >(
                Mixture_distribution<Normal_distribution>(
                    Normal_distribution(0.0, 1.0),
                    Normal_distribution(-1.0, 0.4),
                    0.2
                )
            )
        );
    }
    else
    {
        cout << "Distribution not recognized.\n\n";
        return EXIT_SUCCESS;
    }

    copy(samples.begin(), samples.end(), ostream_iterator<double>(cout, "\n"));
}

