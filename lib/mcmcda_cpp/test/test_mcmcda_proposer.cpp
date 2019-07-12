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

#include <mcmcda_cpp/mcmcda_data.h>
#include <mcmcda_cpp/mcmcda_association.h>
#include <mcmcda_cpp/mcmcda_proposer.h>
#include <m_cpp/m_vector.h>
#include <prob_cpp/prob_distribution.h>
#include <prob_cpp/prob_sample.h>
#include <l_cpp/l_test.h>
#include <vector>
#include <set>
#include <ergo/mh.h>
#include <ergo/rand.h>

using namespace kjb;
using namespace kjb::mcmcda;
using namespace std;

typedef Generic_track<Vector> Track;

set<Vector> random_frame_data(size_t frame);
Vector average_vector_ptrs(const vector<const Vector*>& vecs);
std::vector<double> feature_prob
(
    const Track*, 
    int t1, 
    const Vector* candidate,
    int t2, 
    size_t wsz
);

const size_t num_frames = 50;
const size_t pts_per_frame = 20;

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

int main(int argc, char** argv)
{
    seed_sampling_rand(4000);
    //ergo::rng<boost::mt19937>().seed(44000);
    ergo::global_rng<ergo::default_rng_t>().seed(44000);
    kjb_c::kjb_seed_rand(41000, 42000);
    kjb_c::kjb_seed_rand_2(43000);
    const int d_bar = 10;
    const double v_bar = 1.0 / num_frames;
    const double gamm = 0.1;
    const double noise_sigma = 0.1;

    // create data
    Data<Vector> data;
    data.resize(num_frames);
    for(size_t f = 0; f < num_frames; f++)
    {
        data[f] = random_frame_data(f);
    }

    // create initial empty association
    Association<Track> w(data);

    // create proposer
    Proposer<Track> proposer(
        Categorical_distribution<size_t>(
            MCMCDA_BIRTH, MCMCDA_NUM_MOVES - 1, 1),
        v_bar,
        d_bar,
        d_bar,
        gamm,
        Identity<Vector>(),
        average_vector_ptrs,
        feature_prob,
        noise_sigma,
        Track());

    const size_t num_iterations = 10000;
    for(size_t i = 1; i <= num_iterations; i++)
    {
        Association<Track> w_p(data);
        ergo::mh_proposal_result res = proposer(w, w_p);

        if(isinf(res.fwd) || isinf(res.rev))
        {
            cout << "FAIL: q(w, w') = 0 OR q(w', w) = 0.\n";
            cout << "FAIL: q(w, w') = " << res.fwd 
                 << " q(w', w) = " << res.rev 
                 << endl;;
            return EXIT_FAILURE;
        }

        swap(w, w_p);
    }

    RETURN_VICTORIOUSLY();
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

set<Vector> random_frame_data(size_t frame)
{
    set<Vector> data_t;
    Uniform_distribution U;
    for(size_t i = 0; i < pts_per_frame; i++)
    {
        double x = (frame + 0.5) / num_frames;
        double y = sample(U);
        data_t.insert(Vector().set(x, y));
    }

    return data_t;
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

Vector average_vector_ptrs(const vector<const Vector*>& vecs)
{
    Vector res(0.0, 0.0);
    for(size_t i = 0; i < vecs.size(); i++)
    {
        res += *vecs[i];
    }

    /*if(vecs.size() > 0)
    {
        res /= vecs.size();
    }
    */

    return res;
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

std::vector<double> feature_prob
(
    const Track*, 
    int t1, 
    const Vector* candidate,
    int t2, 
    size_t wsz
)
{
    std::vector<double> f_p;
    return f_p;
}

