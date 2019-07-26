#include "l/l_init.h"
#include "prob_cpp/prob_distribution.h"
#include "prob_cpp/prob_sample.h"
#include "prob_cpp/prob_histogram.h"
#include <boost/bind.hpp>
#include <algorithm>
#include <iterator>
#include <iostream>
#include <vector>
#include <map>

using namespace std;
using namespace boost;
using namespace kjb;

int main(int argc, char** argv)
{
    kjb_c::kjb_init();

    vector<double> samples(5000);
    Gamma_distribution P(2, 2);
    generate(samples.begin(), samples.end(),
        bind(static_cast<double (*)(const Gamma_distribution&)>(sample), P));

    Histogram hist(samples.begin(), samples.end(), 15);
    vector<double> bins(hist.num_bins());
    vector<int> freqs(hist.num_bins());

    transform(hist.as_map().begin(), hist.as_map().end(), bins.begin(),
              bind(&map<double, int>::value_type::first, _1));
    transform(hist.as_map().begin(), hist.as_map().end(), freqs.begin(),
              bind(&map<double, int>::value_type::second, _1));

    copy(samples.begin(), samples.end(), ostream_iterator<double>(cout, "\n"));
    cout << "\n\n";
    copy(bins.begin(), bins.end(), ostream_iterator<double>(cout, " "));
    cout << "\n";
    copy(freqs.begin(), freqs.end(), ostream_iterator<int>(cout, " "));
    cout << "\n\n";

    return EXIT_SUCCESS;
}

