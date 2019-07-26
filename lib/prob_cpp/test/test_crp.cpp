/* ======================================================================== *
 |                                                                          |
 | Copyright (c) 2007-2010, by members of University of Arizona Computer    |
 | Vision group (the authors) including                                     |
 |       Kobus Barnard, Kyle Simek, Ernesto Brau.                           |
 |                                                                          |
 | For use outside the University of Arizona Computer Vision group please   |
 | contact Kobus Barnard.                                                   |
 |                                                                          |
 * ======================================================================== */

/* $Id: test_crp.cpp 20241 2016-01-20 22:34:46Z jguan1 $ */

#include <prob_cpp/prob_distribution.h>
#include <prob_cpp/prob_sample.h>
#include <prob_cpp/prob_pdf.h>
#include <l_cpp/l_test.h>
#include <l_cpp/l_functors.h>
#include <vector>
#include <set>
#include <algorithm>
#include <functional>
#include <iterator>
#include <boost/bind.hpp>

using namespace kjb;
using namespace std;
using namespace boost;

const bool VERBOSE = false;

int main(int argc, char** argv)
{
    size_t n = 50;
    double theta = 3.0;
    Chinese_restaurant_process crp(theta, n);

    Crp::Type B = sample(crp);

    size_t n_B = accumulate(
                    B.begin(), B.end(), 0,
                    bind(plus<size_t>(), _1, bind(&vector<size_t>::size, _2)));

    // test right number of customers
    TEST_TRUE(n == n_B);

    // test correct customer ids
    vector<size_t> custs(n);
    generate(custs.begin(), custs.end(), Increment<size_t>(0));

    if(VERBOSE) cout << "TABLES:" << endl;
    set<size_t> sample_custs;
    for(size_t b = 0; b < B.size(); b++)
    {
        sample_custs.insert(B[b].begin(), B[b].end());

        if(VERBOSE)
        {
            copy(B[b].begin(), B[b].end(), ostream_iterator<size_t>(cout, " "));
            cout << endl;
        }
    }

    TEST_TRUE(equal(custs.begin(), custs.end(), sample_custs.begin()));

    // (barely) test pdf
    double p = pdf(crp, B);
    if(VERBOSE) cout << "p = " << p << endl;
    TEST_TRUE(p > 0 && p <= 1.0);

    double lp = log_pdf(crp, B);
    if(VERBOSE) cout << "log(p) = " << lp << endl;
    TEST_TRUE(lp <= 0.0);
    TEST_TRUE(lp = log(p));

    RETURN_VICTORIOUSLY();
}

