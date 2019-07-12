/* =========================================================================== *
   |
   |  Copyright (c) 1994-2011 by Kobus Barnard (author)
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
   |  Author:  Ernesto Brau
 * =========================================================================== */

#include <l_cpp/l_test.h>
#include <m_cpp/m_vector.h>
#include <vector>
#include <boost/foreach.hpp>
#include <mcmcda_cpp/mcmcda_data.h>
#include <mcmcda_cpp/mcmcda_prior.h>
#include <mcmcda_cpp/mcmcda_association.h>

using namespace kjb;
using namespace kjb::mcmcda;
using namespace std;

typedef Generic_track<Vector> Track;

const bool VERBOSE = true;

int main(int argc, char** argv)
{

    size_t T = 200;
    Prior<Track> prior(0.1/100, 100, 0.5, 1.5);

    // sample association
    pair<vector<Track>, vector<size_t> > wfa = sample(prior, T);
    vector<Track>& tracks = wfa.first;
    const vector<size_t>& fa = wfa.second;

    // create random data given tracks and false alarms
    Data<Vector> Y;
    Y.resize(T);
    BOOST_FOREACH(Track& track, tracks)
    {
        BOOST_FOREACH(Track::value_type& pr, track)
        {
            const Track::value_type::first_type& t = pr.first;
            Track::value_type::second_type& v_p = pr.second;

            Vector x = create_random_vector(2);
            std::set<Vector>::const_iterator x_p = Y[t - 1].insert(x).first;
            v_p = &(*x_p);
        }
    }

    for(size_t t = 0; t < T; t++)
    {
        for(size_t i = 0; i < fa[t]; i++)
        {
            Y[t].insert(create_random_vector(2));
        }
    }

    // create association object using data and tracks
    Association<Track> w(Y, tracks.begin(), tracks.end());

    // test basic stuff
    TEST_TRUE(w.get_available_data().size() == fa.size());

    double pr = prior(w);
    TEST_FALSE(::isinf(pr));
    TEST_FALSE(::isnan(pr));
    TEST_TRUE(pr <= 0);

    if(VERBOSE) 
    {
        size_t m;
        size_t e;
        size_t d;
        size_t a;
        size_t n;
        size_t l;
        get_association_totals(w, m, e, d, a, n, l);

        cout << "P(w) = " << pr << endl;
        cout << "m = " << m << endl;
        cout << "e = " << e << endl;
        cout << "d = " << d << endl;
        cout << "a = " << a << endl;
        cout << "n = " << n << endl;
        cout << "l = " << l << endl;
    }

    RETURN_VICTORIOUSLY();
}

