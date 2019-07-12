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

    Vector operator()()
    {
        return sample(d);
    }
};

int main(int argc, char** argv)
{
    if(argc != 2)
    {
        cout << "Please specify distribution.\n\n";
        return EXIT_SUCCESS;
    }

    vector<Vector> samples(5000);

    string dist = argv[1];

    if(dist == "gaussian" || dist == "normal")
    {
        generate(samples.begin(), samples.end(),
            sample_helper<MV_normal_distribution>(MV_normal_distribution(2)));
    }
    else if(dist == "mixture")
    {
        generate(samples.begin(), samples.end(),
             sample_helper<Mixture_distribution<MV_normal_distribution> >(
                Mixture_distribution<MV_normal_distribution>(
                    MV_normal_distribution(2),
                    MV_normal_distribution(Vector().set(0.0, -4.0),
                                          Vector().set(0.2, 5.0)), 0.2)
                    )
                );
    }
    else
    {
        cout << "Distribution not recognized.\n\n";
        return EXIT_SUCCESS;
    }

    copy(samples.begin(), samples.end(), ostream_iterator<Vector>(cout, "\n"));
}

