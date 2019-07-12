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

#ifndef MCMCDA_PROPOSER_H_INCLUDED
#define MCMCDA_PROPOSER_H_INCLUDED

#include <mcmcda_cpp/mcmcda_association.h>
#include <mcmcda_cpp/mcmcda_track.h>
#include <mcmcda_cpp/mcmcda_data.h>
#include <m_cpp/m_vector.h>
#include <m_cpp/m_matrix.h>
#include <n_cpp/n_eig.h>
#include <prob_cpp/prob_distribution.h>
#include <prob_cpp/prob_sample.h>
#include <prob_cpp/prob_pdf.h>
#include <prob_cpp/prob_util.h>
#include <prob_cpp/prob_stat.h>
#include <l_cpp/l_util.h>
#include <l_cpp/l_exception.h>
#include <l/l_sys_lib.h>
#include <set>
#include <algorithm>
#include <iterator>
#include <functional>
#include <list>
#include <utility>
#include <limits>
#include <map>
#include <vector>
#include <cmath>
#include <boost/foreach.hpp>
#include <boost/bimap.hpp>
#include <boost/bimap/set_of.hpp>
#include <boost/bimap/multiset_of.hpp>

#ifdef KJB_HAVE_ERGO
#include <ergo/mh.h>
#else
// if we don't have libergo installed, this still compiles
namespace ergo {

struct mh_proposal_result
{
    double fwd;
    double rev;
    std::string name;

    mh_proposal_result(double fp, double rp, const std::string& nm = "") :
        fwd(fp), rev(rp), name(nm) {}
};

}
#endif

namespace kjb {
namespace mcmcda {

#warning "[Code police] scoped enums are a c++0x feature."
enum Move : size_t
{
    MCMCDA_BIRTH,
    MCMCDA_DEATH,
    //MCMCDA_SPLIT,
    //MCMCDA_MERGE,
    MCMCDA_EXTENSION,
    MCMCDA_REDUCTION,
    MCMCDA_SWITCH,
    MCMCDA_SECRETION,
    MCMCDA_ABSORPTION,
    MCMCDA_NUM_MOVES
};

using namespace boost::bimaps;

/** brief   MCMCDA proposer class. Chooses move and applies it. */
template <class Track>
class Proposer
{
private:
    typedef Association<Track> Assoc;
    typedef typename Track::Element Element;
    typedef typename Assoc::const_iterator Assoc_const_iterator;
    typedef typename Track::const_iterator Track_const_iterator;
    typedef typename Track::const_reverse_iterator Track_const_riterator;
    typedef boost::function1<Element, const std::vector<const Element*>&> Avg;
    typedef boost::bimap<multiset_of<double, std::greater<double> >,
                         set_of<const Element*> > Probability_map;
    typedef boost::function5<std::vector<double>, const Track*, int, 
                            const Element*, int, size_t> Feature_prob;


private:
    std::vector<Categorical_distribution<size_t> > m_p_move;
    double m_v_bar;
    int m_d_bar;
    int m_b_bar;
    double m_gamma;
    typename Data<Element>::Convert m_convert;
    Avg m_average;
    Feature_prob m_feature_prob;
    double m_noise_sigma;
    Track def_track;
    bool use_feature;

public:
    /* @brief   C-tor. */
    Proposer
    (
        const Categorical_distribution<size_t>& move_distribution,
        double v_bar,
        int d_bar,
        int b_bar,
        double gamma,
        const typename Data<Element>::Convert& convert_to_vector,
        const Avg& avg,
        double detection_noise_variance,
        const Track& empty_track
    ) :
        m_v_bar(v_bar),
        m_d_bar(d_bar),
        m_b_bar(b_bar),
        m_gamma(gamma),
        m_convert(convert_to_vector),
        m_average(avg),
        m_noise_sigma(detection_noise_variance),
        def_track(empty_track),
        use_feature(false)
    {
        make_p_move(move_distribution);
    }

    Proposer
    (
        const Categorical_distribution<size_t>& move_distribution,
        double v_bar,
        int d_bar,
        int b_bar,
        double gamma,
        const typename Data<Element>::Convert& convert_to_vector,
        const Avg& avg,
        const Feature_prob& feature_prob,
        double detection_noise_variance,
        const Track& empty_track
    ) :
        m_v_bar(v_bar),
        m_d_bar(d_bar),
        m_b_bar(b_bar),
        m_gamma(gamma),
        m_convert(convert_to_vector),
        m_average(avg),
        m_feature_prob(feature_prob),
        m_noise_sigma(detection_noise_variance),
        def_track(empty_track),
        use_feature(true)
    {
        make_p_move(move_distribution);
    }

    /* @brief   Apply this proposer. */
    ergo::mh_proposal_result operator()(const Assoc& w, Assoc& w_p) const;

    /** @brief  Return maximum velocity of targets. */
    double v_bar() const { return m_v_bar; }

    /** @brief  Return maximum number of missed frames. */
    int d_bar() const { return m_d_bar; }

    /** @brief  Return probability of stopping a growing track. */
    double gamma() const { return m_gamma; }

    /** @brief  Sample a move type. */
    //Move sample_move(const Assoc& w) const
    size_t sample_move(const Assoc& w) const
    {
        size_t K = w.size() < 2 ? w.size() : 2;
        return kjb::sample(m_p_move[K]);
    }

    /** @brief  Compute the log pdf of a move type. */
    double move_log_pdf(Move m, const Assoc& w) const
    {
        size_t K = w.size() < 2 ? w.size() : 2;
        return log_pdf(m_p_move[K], m);
    }

public:
    /** @brief  Propose birth. */
    double propose_birth(const Assoc& w, Assoc& w_p) const;

    /** @brief  Propose death. */
    double propose_death(const Assoc& w, Assoc& w_p) const;

    /** @brief  Propose split. */
    double propose_split(const Assoc& w, Assoc& w_p) const;

    /** @brief  Propose merge. */
    double propose_merge(const Assoc& w, Assoc& w_p) const;

    /** @brief  Propose exteision. */
    double propose_extension(const Assoc& w, Assoc& w_p) const;

    /** @brief  Propose reduction. */
    double propose_reduction(const Assoc& w, Assoc& w_p) const;

    /** @brief  Propose switch. */
    double propose_switch(const Assoc& w, Assoc& w_p) const;

    /** @brief  Propose secretion. */
    double propose_secretion(const Assoc& w, Assoc& w_p) const;

    /** @brief  Propose absorption. */
    double propose_absorption(const Assoc& w, Assoc& w_p) const;

public:
    /** @brief  Probability of birth. */
    double p_birth(const Assoc& w, const Assoc& w_p) const;

    /** @brief  Probability of death. */
    double p_death(const Assoc& w, const Assoc& w_p) const;

    /** @brief  Probability of split. */
    double p_split(const Assoc& w, const Assoc& w_p) const;

    /** @brief  Probability of merge. */
    double p_merge(const Assoc& w, const Assoc& w_p) const;

    /** @brief  Probability of extension. */
    double p_extension(const Assoc& w, const Assoc& w_p) const;

    /** @brief  Probability of reduction. */
    double p_reduction(const Assoc& w, const Assoc& w_p) const;

    /** @brief  Probability of switch. */
    double p_switch(const Assoc& w, const Assoc& w_p) const;

    /** @brief  Probability of secretion. */
    double p_secretion(const Assoc& w, const Assoc& w_p) const;

    /** @brief  Probability of absorption. */
    double p_absorption(const Assoc& w, const Assoc& w_p) const;

private:
    /** @brief  Determines if change constitutes valid birth. */
    bool is_valid_birth(const Assoc& w, const Assoc& w_p) const;

    /** @brief  Determines if change constitutes valid death. */
    bool is_valid_death(const Assoc& w, const Assoc& w_p) const;

    /** @brief  Determines if change constitutes valid split. */
    bool is_valid_split(const Assoc& w, const Assoc& w_p) const;

    /** @brief  Determines if change constitutes valid merge. */
    bool is_valid_merge(const Assoc& w, const Assoc& w_p) const;

    /** @brief  Determines if change constitutes valid extension. */
    bool  is_valid_extension(const Assoc& w, const Assoc& w_p) const;

    /** @brief  Determines if change constitutes valid reduction. */
    bool is_valid_reduction(const Assoc& w, const Assoc& w_p) const;

    /** @brief  Determines if change constitutes valid switch. */
    bool is_valid_switch(const Assoc& w, const Assoc& w_p) const;

    /** @brief  Determines if change constitutes valid secretion. */
    bool is_valid_secretion(const Assoc& w, const Assoc& w_p) const;

    /** @brief  Determines if change constitutes valid absorption. */
    bool is_valid_absorption(const Assoc& w, const Assoc& w_p) const;

public:
    /** @brief  Grows a track forward according to MCMCDA. */
    void grow_track_forward(Track& track, const Assoc& w) const;

    /** @brief  Grows a track backward according to MCMCDA. */
    void grow_track_backward(Track& track, const Assoc& w) const;

    /** @brief  Probability of growing a track forward to what it is. */
    double p_grow_track_forward
    (
        const Track& track,
        const Assoc& w,
        int t
    ) const;

    /** @brief  Probability of growing a track backward to what it is. */
    double p_grow_track_backward
    (
        const Track& track,
        const Assoc& w,
        int t
    ) const;

    /** @brief  Probability of not choosing any detections. */
    double p_choose_nothing
    (
        const Probability_map& probs
    ) const;

    /** @brief  Creates move distribution. */
    void make_p_move(const Categorical_distribution<size_t>& move_distribution)
    {
        std::vector<double> ps(MCMCDA_NUM_MOVES, 0.0);

        // distribution for K = 0
        ps[MCMCDA_BIRTH] = 1.0;
        m_p_move.push_back(Categorical_distribution<size_t>(ps, 0));

        // distribution for K = 1
        ps[MCMCDA_BIRTH] = 0.3;
        ps[MCMCDA_DEATH] = 0.1;
        ps[MCMCDA_EXTENSION] = 0.4;
        ps[MCMCDA_REDUCTION] = 0.1;
        ps[MCMCDA_SWITCH] = 0.0;
        ps[MCMCDA_SECRETION] = 0.1;
        ps[MCMCDA_ABSORPTION] = 0.0;
        m_p_move.push_back(Categorical_distribution<size_t>(ps, 0));

        /*std::fill(ps.begin(), ps.end(), 1.0 / (MCMCDA_NUM_MOVES - 2));
        //ps[MCMCDA_MERGE] = 0.0;
        ps[MCMCDA_SWITCH] = 0.0;
        ps[MCMCDA_ABSORPTION] = 0.0;
        m_p_move.push_back(Categorical_distribution<size_t>(ps, 0));*/

        // distribution for K > 1
        m_p_move.push_back(move_distribution);
    }

public:
    /** @brief  Helper function that gives probabilities of growth candidates.*/
    Probability_map get_growing_probabilities
    (
        const Track& track, 
        int t,
        std::set<const Element*>& candidates,
        const Vector& x,
        const Vector& vel,
        int d
    ) const;

    /** @brief  Helper function that gives score of a noise thingy. */
    double get_velocity_noise_score(int d, const Vector& vel) const
    {
        d = std::abs(d);
        double vel_sigma = vel.empty() ? d*d*m_v_bar*m_v_bar : 0.0;
        double sg_u = sqrt(m_noise_sigma + vel_sigma);
        double sg_v = sqrt(m_noise_sigma + vel_sigma);
        Normal_distribution N_u(0.0, sg_u);
        Normal_distribution N_v(0.0, sg_v);
        //double p_good = 0.5;
        return pdf(N_u, sg_u) * pdf(N_v, sg_v);
    }

    /** @brief  Estimate the velocity of the detections going forward. */
    std::pair<Vector, Vector> track_future
    (
        const Track& track,
        int t = -1
    ) const;

    /** @brief  Estimate the velocity of the detections going backward. */
    std::pair<Vector, Vector> track_past
    (
        const Track& track,
        int t = -1
    ) const;

    /** @brief  Estimate the velocity of the detections going forward. */
    std::pair<Vector, Vector> track_future_ls
    (
        const Track& track,
        int t = -1
    ) const;

    /** @brief  Estimate the velocity of the detections going backward. */
    std::pair<Vector, Vector> track_past_ls
    (
        const Track& track,
        int t = -1
    ) const;

    /** @brief  Estimate the velocity of the detections going forward. */
    std::pair<Vector, Vector> track_future_tls
    (
        const Track& track,
        int t = -1
    ) const;

    /** @brief  Estimate the velocity of the detections going backward. */
    std::pair<Vector, Vector> track_past_tls
    (
        const Track& track,
        int t = -1
    ) const;

private:
    /** @brief  Helper function that counts split points in an association. */
    size_t count_split_points(const Assoc& w) const;

    /** @brief  Helper function that counts merge pairs in an association. */
    size_t count_merge_pairs(const Assoc& w) const;

    /** @brief  Counts sufficiently-long tracks for secretion. */
    size_t count_secretion_tracks(const Assoc& w) const;

    /** @brief  Counts pairs of tracks that can be absorbed together. */
    size_t count_absorption_pairs(const Assoc& w) const;

    /** @brief  Returns the most negative double. */
    double negative_infinity() const
    {
        return -std::numeric_limits<double>::max();
    }

    // constants
    static const int GROW_FORWARD;
    static const int GROW_BACKWARD;

private:
    /** @brief  Information about birth performed immediately previously. */
    struct Birth_info
    {
        Assoc_const_iterator new_track_p;
    };

    /** @brief  Information about death performed immediately previously. */
    struct Death_info
    {
        size_t num_tracks;
    };

    /** @brief  Information about split performed immediately previously. */
    struct Split_info
    {
        size_t num_split_points;
    };

    /** @brief  Information about merge performed immediately previously. */
    struct Merge_info
    {
        size_t num_merge_pairs;
    };

    /** @brief  Information about extension performed immediately previously. */
    struct Extension_info
    {
        size_t num_tracks;
        Assoc_const_iterator extended_track_p;
        int direction;
        int previous_end;
    };

    /** @brief  Information about reduction performed immediately previously. */
    struct Reduction_info
    {
        size_t num_tracks;
        size_t reduced_track_size;
    };

    /** @brief  Information about switch performed immediately previously. */
    struct Switch_info
    {
        size_t num_switch_points;
    };

    /** @brief  Information about secretion performed immediately previously. */
    struct Secretion_info
    {
        size_t num_valid_tracks;
        size_t window_size;
    };

    /** @brief Information about absorption performed immediately previously. */
    struct Absorption_info
    {
        size_t num_valid_track_pairs;
    };

private:
    mutable Birth_info birth_info;
    mutable Death_info death_info;
    mutable Split_info split_info;
    mutable Merge_info merge_info;
    mutable Extension_info extension_info;
    mutable Reduction_info reduction_info;
    mutable Switch_info switch_info;
    mutable Secretion_info secretion_info;
    mutable Absorption_info absorption_info;
};

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

template <class Track>
const int Proposer<Track>::GROW_FORWARD = 1;

template <class Track>
const int Proposer<Track>::GROW_BACKWARD = 2;

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

template <class Track>
ergo::mh_proposal_result Proposer<Track>::operator()
(
    const Assoc& w,
    Assoc& w_p
) const
{
    double fwd, rev;
    bool keep_going = true;
    std::string move_name = "association-";

    while(keep_going)
    {
        //Move m = sample_move(w);
        size_t m = sample_move(w);

        switch(m)
        {
            case MCMCDA_BIRTH:
            {
                fwd = propose_birth(w, w_p);
                if(fwd == negative_infinity())
                {
                    break;
                }

                rev = p_death(w_p, w);
                if(rev == negative_infinity())
                {
                    break;
                }

                fwd += move_log_pdf(MCMCDA_BIRTH, w);
                rev += move_log_pdf(MCMCDA_DEATH, w_p);

                move_name += "birth";
                break;
            }

            case MCMCDA_DEATH:
            {
                fwd = propose_death(w, w_p);
                if(fwd == negative_infinity())
                {
                    break;
                }

                rev = p_birth(w_p, w);
                if(rev == negative_infinity())
                {
                    break;
                }

                fwd += move_log_pdf(MCMCDA_DEATH, w);
                rev += move_log_pdf(MCMCDA_BIRTH, w_p);

                move_name += "death";
                break;
            }

            /*case MCMCDA_SPLIT:
            {
                fwd = propose_split(w, w_p);
                if(fwd == negative_infinity())
                {
                    break;
                }

                rev = p_merge(w_p, w);
                if(rev == negative_infinity())
                {
                    break;
                }

                fwd += move_log_pdf(MCMCDA_SPLIT, w);
                rev += move_log_pdf(MCMCDA_MERGE, w_p);

                move_name += "split";
                break;
            }

            case MCMCDA_MERGE:
            {
                fwd = propose_merge(w, w_p);
                if(fwd == negative_infinity())
                {
                    break;
                }

                rev = p_split(w_p, w);
                if(rev == negative_infinity())
                {
                    break;
                }

                fwd += move_log_pdf(MCMCDA_MERGE, w);
                rev += move_log_pdf(MCMCDA_SPLIT, w_p);

                move_name += "merge";
                break;
            }*/

            case MCMCDA_EXTENSION:
            {
                fwd = propose_extension(w, w_p);
                if(fwd == negative_infinity())
                {
                    break;
                }

                rev = p_reduction(w_p, w);
                if(rev == negative_infinity())
                {
                    break;
                }

                fwd += move_log_pdf(MCMCDA_EXTENSION, w);
                rev += move_log_pdf(MCMCDA_REDUCTION, w_p);

                move_name += "extension";
                break;
            }

            case MCMCDA_REDUCTION:
            {
                fwd = propose_reduction(w, w_p);
                if(fwd == negative_infinity())
                {
                    break;
                }

                rev = p_extension(w_p, w);
                if(rev == negative_infinity())
                {
                    break;
                }

                fwd += move_log_pdf(MCMCDA_REDUCTION, w);
                rev += move_log_pdf(MCMCDA_EXTENSION, w_p);

                move_name += "reduction";
                break;
            }

            case MCMCDA_SWITCH:
            {
                fwd = propose_switch(w, w_p);
                if(fwd == negative_infinity())
                {
                    break;
                }

                fwd += move_log_pdf(MCMCDA_SWITCH, w);

                // recall that switch move is symmetric
                rev = fwd;

                move_name += "switch";
                break;
            }

            case MCMCDA_SECRETION:
            {
                fwd = propose_secretion(w, w_p);
                if(fwd == negative_infinity())
                {
                    break;
                }

                rev = p_absorption(w_p, w);
                if(rev == negative_infinity())
                {
                    break;
                }

                fwd += move_log_pdf(MCMCDA_SECRETION, w);
                rev += move_log_pdf(MCMCDA_ABSORPTION, w_p);
                //fwd = rev = 0.0;

                move_name += "secretion";
                break;
            }

            case MCMCDA_ABSORPTION:
            {
                fwd = propose_absorption(w, w_p);
                if(fwd == negative_infinity())
                {
                    break;
                }

                rev = p_secretion(w_p, w);
                if(rev == negative_infinity())
                {
                    break;
                }

                fwd += move_log_pdf(MCMCDA_ABSORPTION, w);
                rev += move_log_pdf(MCMCDA_SECRETION, w_p);
                //fwd = rev = 0.0;

                move_name += "absorption";
                break;
            }

            default:
            {
                KJB_THROW_3(Runtime_error,
                            "MCMCDA: move %d does not exist.", (m));
                break;
            }
        }

        if(fwd != negative_infinity() && rev != negative_infinity())
        {
            keep_going = false;
        }
    }

    return ergo::mh_proposal_result(fwd, rev, move_name);
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

template <class Track>
double Proposer<Track>::propose_birth(const Assoc& w, Assoc& w_p) const
{
    //KJB(ASSERT(w.is_valid(m_v_bar, m_d_bar, m_noise_sigma, m_convert)));

    Track new_track = def_track;

    // pick first point in new track
    size_t t_1 = kjb::sample(
        Categorical_distribution<size_t>(1, w.get_data().size(), 1));

    std::set<const Element*> L_1 = w.get_dead_points_at_time(t_1);

    if(L_1.empty())
    {
        return negative_infinity();
    }

    //const Element* x_1 = *element_uar(L_1.begin(), L_1.size());
    std::vector<double> ps(L_1.size(), 1.0);
    Categorical_distribution<size_t> P(ps, 0);
    size_t n = kjb::sample(P);
    typename std::set<const Element*>::const_iterator x_1_p = L_1.begin();

    std::advance(x_1_p, n);
    new_track.insert(std::make_pair(t_1, *x_1_p));

    // grow track
    grow_track_forward(new_track, w);
    if(new_track.real_size() == 1)
    {
        return negative_infinity();
    }

    // add new track to sampled association
    w_p = w;
    Assoc_const_iterator new_track_p = w_p.insert(new_track).first;

    // set changed flag
    new_track_p->set_changed_start(new_track_p->get_start_time());
    new_track_p->set_changed_end(new_track_p->get_end_time());

    // compute forward probability
    birth_info.new_track_p = new_track_p;
    death_info.num_tracks = w_p.size();
    return p_birth(w, w_p);
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

template <class Track>
double Proposer<Track>::propose_death(const Assoc& w, Assoc& w_p) const
{
    //KJB(ASSERT(w.is_valid(m_v_bar, m_d_bar, m_noise_sigma, m_convert)));

    if(w.empty())
    {
        std::cout << "WARNING: cannot propose death for empty association.\n";
        return std::log(0.0);
    }

    // remove track UAR
    w_p = w;
    Assoc_const_iterator new_track_p = element_uar(w.begin(), w.size());
    w_p.erase(*new_track_p);

    // compute forward probability
    death_info.num_tracks = w.size();
    birth_info.new_track_p = new_track_p;
    return p_death(w, w_p);
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

template <class Track>
double Proposer<Track>::propose_split(const Assoc& w, Assoc& w_p) const
{
    //KJB(ASSERT(w.is_valid(m_v_bar, m_d_bar, m_noise_sigma, m_convert)));

    if(w.empty())
    {
        std::cout << "WARNING: cannot propose split for empty association.\n";
        return std::log(0.0);
    }

    size_t nsp = count_split_points(w);
    if(nsp == 0)
    {
        return negative_infinity();
    }

    // pick random split point and compute it
    size_t n = kjb::sample(Categorical_distribution<size_t>(1, nsp, 1));

    w_p = w;
    Assoc_const_iterator track_p;
    Track_const_iterator rp;
    size_t count = 0;
    for(track_p = w_p.begin(); track_p != w_p.end(); track_p++)
    {
        size_t tsz = track_p->real_size();
        if(tsz >= 4)
        {
            if(count + tsz - 3 >= n)
            {
                int r = 0;
                int t = 0;
                Track_const_iterator pair_p;
                for(pair_p = track_p->begin(); pair_p != track_p->end();
                                               pair_p++)
                {
                    if(pair_p->first == t)
                    {
                        continue;
                    }
                    t = pair_p->first;

                    r++;
                    if(r == n - count + 1)
                    {
                        break;
                    }
                }
                rp = pair_p;
                rp++;
                break;
            }
            count += (tsz - 3);
        }
    }

    // split track into two tracks
    Track beg = def_track;
    Track fin = def_track;
    beg.insert(track_p->begin(), rp);
    fin.insert(rp, track_p->end());

    // set changed flags
    beg.set_changed_start(beg.get_start_time());
    beg.set_changed_end(beg.get_end_time());
    fin.set_changed_start(fin.get_start_time());
    fin.set_changed_end(fin.get_end_time());

    // remove split track and add what it got split into
    w_p.erase(track_p);
    w_p.insert(beg);
    w_p.insert(fin);

    // compute forward probability
    split_info.num_split_points = nsp;
    merge_info.num_merge_pairs = count_merge_pairs(w_p);
    return p_split(w, w_p);
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

template <class Track>
double Proposer<Track>::propose_merge(const Assoc& w, Assoc& w_p) const
{
    //KJB(ASSERT(w.is_valid(m_v_bar, m_d_bar, m_noise_sigma, m_convert)));

    if(w.size() < 2)
    {
        std::cout << "WARNING: cannot propose merge for single-track "
                     "association.\n";
        return std::log(0.0);
    }

    w_p = w;

    // compute merge candidates
    std::list<std::pair<Assoc_const_iterator, Assoc_const_iterator> > M;
    for(Assoc_const_iterator track_p1 = w_p.begin();
                             track_p1 != w_p.end();
                             track_p1++)
    {
        int t_f = track_p1->get_end_time();
        for(Assoc_const_iterator track_p2 = w_p.begin();
                                 track_p2 != w_p.end();
                                 track_p2++)
        {
            int t_1 = track_p2->get_start_time();
            if(track_p1 != track_p2 && t_f < t_1)
            {
                if(is_neighbor(track_p1->get_end_point(),
                               track_p2->get_start_point(),
                               t_1 - t_f, m_d_bar, m_v_bar,
                               m_noise_sigma, m_convert))
                {
                    M.push_back(std::make_pair(track_p1, track_p2));
                }
            }
        }
    }

    if(M.empty())
    {
        return negative_infinity();
    }

    // choose a candidate pair and merge its tracks into a single track
    std::pair<Assoc_const_iterator, Assoc_const_iterator> ttm
                                    = *element_uar(M.begin(), M.size());
    Track new_track = *ttm.first;
    new_track.insert(ttm.second->begin(), ttm.second->end());

    // set changed flags
    new_track.set_changed_start(ttm.second->get_start_time());
    new_track.set_changed_end(ttm.second->get_end_time());

    // remove merged tracks and add the result of merging them
    w_p.erase(ttm.first);
    w_p.erase(ttm.second);
    w_p.insert(new_track);

    // compute forward probability
    merge_info.num_merge_pairs = M.size();
    split_info.num_split_points = count_split_points(w_p);
    return p_merge(w, w_p);
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

template <class Track>
double Proposer<Track>::propose_extension
(
    const Assoc& w,
    Assoc& w_p
) const
{
    //KJB(ASSERT(w.is_valid(m_v_bar, m_d_bar, m_noise_sigma, m_convert)));

    if(w.empty())
    {
        std::cout << "WARNING: cannot propose extension for empty "
                     "association.\n";
        return std::log(0.0);
    }

    w_p = w;

    // choose track to extend
    Assoc_const_iterator track_p = element_uar(w_p.begin(), w_p.size());
    Track extended_track = *track_p;
    size_t original_track_size = extended_track.size();

    // grow track
    int dir;
    int prev_end;
    double u = kjb::sample(Uniform_distribution());
    if(u < 0.5)
    {
        dir = GROW_FORWARD;
        prev_end = extended_track.get_end_time();
        grow_track_forward(extended_track, w_p);
        extended_track.set_changed_start(prev_end);
        extended_track.set_changed_end(extended_track.get_end_time());
    }
    else
    {
        dir = GROW_BACKWARD;
        prev_end = extended_track.get_start_time();
        grow_track_backward(extended_track, w_p);
        extended_track.set_changed_start(extended_track.get_start_time());
        extended_track.set_changed_end(prev_end);
    }


    // if it did not grow...kill it!
    if(extended_track.size() == original_track_size)
    {
        return negative_infinity();
    }

    // replace track with extended version
    w_p.erase(track_p);
    Assoc_const_iterator extended_track_p = w_p.insert(extended_track).first;

    // compute forward probability
    extension_info.num_tracks = w.size();
    extension_info.extended_track_p = extended_track_p;
    extension_info.direction = dir;
    extension_info.previous_end = prev_end;
    reduction_info.num_tracks = w_p.size();
    reduction_info.reduced_track_size = extended_track.real_size();
    return p_extension(w, w_p);
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

template <class Track>
double Proposer<Track>::propose_reduction
(
    const Assoc& w,
    Assoc& w_p
) const
{
    //KJB(ASSERT(w.is_valid(m_v_bar, m_d_bar, m_noise_sigma, m_convert)));

    if(w.empty())
    {
        std::cout << "WARNING: cannot propose reduction for empty "
                     "association.\n";
        return std::log(0.0);
    }

    w_p = w;

    // choose track to reduce
    typename Assoc::iterator track_p = element_uar(w_p.begin(), w_p.size());

    // must be longer than two points
    if(track_p->real_size() <= 2)
    {
        return negative_infinity();
    }

    Track reduced_track = *track_p;
    size_t rtsz = reduced_track.real_size();
    size_t n = kjb::sample(Categorical_distribution<size_t>(2, rtsz - 1, 1));
    int t = reduced_track.get_nth_time(n);

    // choose direction and reduce
    int dir;
    if(kjb::sample(Uniform_distribution()) <= 0.5)
    {
        reduced_track.erase(reduced_track.upper_bound(t), reduced_track.end());
        dir = GROW_FORWARD;
    }
    else
    {
        reduced_track.erase(reduced_track.begin(), reduced_track.lower_bound(t));
        dir = GROW_BACKWARD;
    }

    // set changed flags
    //reduced_track.set_changed_start(-1);
    //reduced_track.set_changed_end(-1);
    reduced_track.set_changed_all();

    // replace track with reduced version
    Assoc_const_iterator orig_track_p = w.find(*track_p);
    w_p.erase(track_p);
    w_p.insert(reduced_track);

    // compute forward probability
    reduction_info.num_tracks = w.size();
    reduction_info.reduced_track_size = rtsz;
    extension_info.num_tracks = w_p.size();
    extension_info.extended_track_p = orig_track_p;
    extension_info.direction = dir;
    extension_info.previous_end = t;
    return p_reduction(w, w_p);
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

template <class Track>
double Proposer<Track>::propose_switch
(
    const Assoc& w,
    Assoc& w_p
) const
{
    //KJB(ASSERT(w.is_valid(m_v_bar, m_d_bar, m_noise_sigma, m_convert)));

    using namespace boost;

    typedef tuple<const Track&, int, int, const Track&, int, int> Switch_point;

    std::list<Switch_point> switch_points;
    for(Assoc_const_iterator track_p1 = w.begin();
                             track_p1 != w.end();
                             track_p1++)
    {
        Assoc_const_iterator track_p2 = track_p1;
        for(track_p2++; track_p2 != w.end(); track_p2++)
        {
            // get first element in last frame of track1
            Track_const_iterator last_p1
                            = track_p1->lower_bound(track_p1->rbegin()->first);
            for(Track_const_iterator pair_p1 = track_p1->begin();
                                     pair_p1 != last_p1;
                                     pair_p1++)
            {
                // advance until last element of current frame
                int curt = pair_p1->first;
                while(pair_p1->first == curt)
                {
                    pair_p1++;
                }
                pair_p1--;

                // get first element in last frame of track2
                Track_const_iterator last_p2
                            = track_p2->lower_bound(track_p2->rbegin()->first);
                for(Track_const_iterator pair_p2 = track_p2->begin();
                                         pair_p2 != last_p2;
                                         pair_p2++)
                {
                    // advance until last element of current frame
                    curt = pair_p2->first;
                    while(pair_p2->first == curt)
                    {
                        pair_p2++;
                    }
                    pair_p2--;

                    Track_const_iterator pair_q1 = pair_p1;
                    Track_const_iterator pair_q2 = pair_p2;

                    int t1 = pair_p1->first;
                    int t2 = pair_p2->first;
                    int tp1 = (++pair_q1)->first;
                    int tq1 = (++pair_q2)->first;
                    if(tp1 > t2 && tq1 > t1
                        && is_neighbor(*pair_p1->second,
                                       *pair_q2->second, tq1 - t1, m_d_bar,
                                       m_v_bar, m_noise_sigma, m_convert)
                        && is_neighbor(*pair_p2->second,
                                       *pair_q1->second, tp1 - t2, m_d_bar,
                                       m_v_bar, m_noise_sigma, m_convert))
                    {
                        switch_points.push_back(
                            Switch_point(*track_p1, t1, tp1,
                                         *track_p2, t2, tq1));
                    }
                }
            }
        }
    }

    if(switch_points.empty())
    {
        return negative_infinity();
    }

    Switch_point sp = *element_uar(switch_points.begin(), switch_points.size());
    Track track1_new = get<0>(sp);
    Track track2_new = get<3>(sp);
    swap_tracks(track1_new, track2_new, get<1>(sp),
                get<2>(sp), get<4>(sp), get<5>(sp));

    // set changed flags
    track1_new.set_changed_start(get<1>(sp));
    track1_new.set_changed_end(track1_new.get_end_time());
    track2_new.set_changed_start(get<4>(sp));
    track2_new.set_changed_end(track2_new.get_end_time());

    w_p = w;
    w_p.erase(get<0>(sp));
    w_p.erase(get<3>(sp));
    w_p.insert(track1_new);
    w_p.insert(track2_new);

    // compute forward probability
    switch_info.num_switch_points = switch_points.size();
    return p_switch(w, w_p);
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

template <class Track>
double Proposer<Track>::propose_secretion
(
    const Assoc& w,
    Assoc& w_p
) const
{
    //KJB(ASSERT(w.is_valid(m_v_bar, m_d_bar, m_noise_sigma, m_convert)));

    if(w.empty())
    {
        std::cout << "WARNING: cannot propose secretion for empty "
                     "association.\n";
        return std::log(0.0);
    }

    w_p = w;

    typedef typename Assoc::iterator Assoc_iterator;
    //typedef typename Track::iterator Track_iterator;

    std::vector<Assoc_iterator> long_tracks;
    for(Assoc_iterator track_p = w_p.begin(); track_p != w_p.end(); track_p++)
    {
        if(track_p->real_size() > 4)
        {
            long_tracks.push_back(track_p);
        }
        else
        {
            if(track_p->count(track_p->get_start_time()) > 1
                && track_p->count(track_p->get_end_time()) > 1)
            {
                long_tracks.push_back(track_p);
            }
        }
    }

    if(long_tracks.empty())
    {
        return negative_infinity();
    }

    // choose random track
    Assoc_iterator orig_p = *element_uar(long_tracks.begin(),
                                         long_tracks.size());
    Track orig_track = def_track;
    Track new_track = def_track;

    // choose separators
    size_t sz = orig_p->size();
    Track_const_iterator pair_p1 = element_uar(orig_p->begin(), sz);
    Track_const_iterator pair_p2 = element_uar(orig_p->begin(), sz);

    Track_const_iterator small_p, big_p;
    if(pair_p1->first < pair_p2->first)
    {
        small_p = pair_p1;
        big_p = pair_p2;
    }
    else if(pair_p1->first > pair_p2->first)
    {
        small_p = pair_p2;
        big_p = pair_p1;
    }
    else
    {
        if(std::distance(orig_p->begin(), pair_p1)
                < std::distance(orig_p->begin(), pair_p2))
        {
            small_p = pair_p1;
            big_p = pair_p2;
        }
        else
        {
            small_p = pair_p2;
            big_p = pair_p1;
        }
    }

    // do initial copying
    orig_track.insert(orig_p->begin(), ++small_p);

    Uniform_distribution U;
    if(kjb::sample(U) < 0.5)
    {
        orig_track.insert(++big_p, orig_p->end());
    }
    else
    {
        new_track.insert(++big_p, orig_p->end());
    }

    // coin flips in mid section
    size_t win_sz = 0;
    while(small_p != big_p)
    {
        if(kjb::sample(U) < 0.5)
        {
            orig_track.insert(*small_p);
        }
        else
        {
            new_track.insert(*small_p);
        }

        small_p++;
        win_sz++;
    }

    // make sure new tracks are valid
    if(!orig_track.is_valid(m_v_bar, m_d_bar, m_noise_sigma, m_convert)
        || !new_track.is_valid(m_v_bar, m_d_bar, m_noise_sigma, m_convert))
    {
        return negative_infinity();
    }

    // set changed flags
    orig_track.set_changed_start(orig_track.get_start_time());
    orig_track.set_changed_end(orig_track.get_end_time());
    new_track.set_changed_start(new_track.get_start_time());
    new_track.set_changed_end(new_track.get_end_time());

    // add new tracks / remove old one
    w_p.erase(orig_p);
    w_p.insert(orig_track);
    w_p.insert(new_track);

    // compute forward probability
    secretion_info.num_valid_tracks = long_tracks.size();
    secretion_info.window_size = win_sz;
    absorption_info.num_valid_track_pairs = count_absorption_pairs(w_p);
    return p_secretion(w, w_p);
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

template <class Track>
double Proposer<Track>::propose_absorption
(
    const Assoc& w,
    Assoc& w_p
) const
{
    //KJB(ASSERT(w.is_valid(m_v_bar, m_d_bar, m_noise_sigma, m_convert)));

    if(w.size() < 2)
    {
        std::cout << "WARNING: cannot propose absorption for single-track "
                     "association.\n";
        return std::log(0.0);
    }

    w_p = w;

    // compute merge candidates
    using namespace boost;
    typedef tuple<Assoc_const_iterator, Assoc_const_iterator, Track> iit_tuple;

    const int win_size = 20;
    std::vector<double> ps;
    ps.reserve((w.size()*(w.size()-1))/2);
    std::list<iit_tuple> M;
    Assoc w_e(w_p.get_data());
    for(Assoc_const_iterator track_p1 = w_p.begin();
                             track_p1 != w_p.end();
                             track_p1++)
    {
        Assoc_const_iterator track_p2 = track_p1;
        for(++track_p2; track_p2 != w_p.end(); ++track_p2)
        {
            // take longer track as "new" track
            Track new_track = def_track;

            int sf1 = track_p1->get_start_time();
            int ef1 = track_p1->get_end_time();
            int sf2 = track_p2->get_start_time();
            int ef2 = track_p2->get_end_time();

            //size_t sf = std::min(sf1, sf2);
            //size_t ef = std::max(ef1, ef2);

            int l1 = ef1 - sf1 + 1;
            int l2 = ef2 - sf2 + 1;

            bool olap = (sf1 <= ef2 && sf2 <= ef1);
            int chst, ched;
            if(l1 > l2)
            {
                new_track = *track_p1;
                new_track.insert(track_p2->begin(), track_p2->end());
                if(olap)
                {
                    chst = sf2;
                    ched = ef2;
                }
                else
                {
                    if(sf2 < sf1)
                    {
                        chst = sf2;
                        //ched = sf1;
                        int end = sf1 + win_size;
                        ched = end < ef1 ? end : ef1;
                    }
                    else
                    {
                        //chst = ef1;
                        int start = ef1 - win_size;
                        chst = start > sf1 ? start : sf1;
                        ched = ef2;
                    }
                }
            }
            else
            {
                new_track = *track_p2;
                new_track.insert(track_p1->begin(), track_p1->end());
                if(olap)
                {
                    chst = sf1;
                    ched = ef1;
                }
                else
                {
                    if(sf1 < sf2)
                    {
                        chst = sf1;
                        //ched = sf2;
                        int end = sf2 + win_size;
                        ched = end < ef2 ? end : ef2;
                    }
                    else
                    {
                        //chst = ef2;
                        int start = ef2 - win_size;
                        chst = start > sf2 ? start : sf2;
                        ched = ef1;
                    }
                }
            }

            new_track.set_changed_start(chst);
            new_track.set_changed_end(ched);

            double pg = p_grow_track_forward(new_track, w_e,
                                             new_track.get_start_time());
            if(pg != negative_infinity())
            {
                M.push_back(boost::make_tuple(track_p1, track_p2, new_track));
                ps.push_back(pg);
            }
        }
    }

    if(ps.empty())
    {
        return negative_infinity();
    }

    // normalize
    typename std::list<iit_tuple>::const_iterator iit_p = M.begin();
    for(size_t i = 0; i < ps.size(); i++, iit_p++)
    {
        ps[i] /= get<2>(*iit_p).real_size();
        ps[i] *= -1.0;
        ps[i] = std::sqrt(ps[i]);
        ps[i] *= -1.0;
    }

    double lsum = log_sum(ps.begin(), ps.end());
    std::transform(ps.begin(), ps.end(), ps.begin(),
                   std::bind2nd(std::minus<double>(), lsum));
    std::transform(ps.begin(), ps.end(), ps.begin(),
                   std::ptr_fun<double, double>(std::exp));

    // choose a candidate pair and merge its tracks into a single track
    size_t idx = kjb::sample(Categorical_distribution<size_t>(ps));
    typename std::list<iit_tuple>::iterator tuple_p = M.begin();
    std::advance(tuple_p, idx - 1);
    iit_tuple& ttm = *tuple_p;

    // figure out which track is which
    const Element* st_elem_p = get<0>(ttm)->begin()->second;
    if(get<0>(ttm)->begin()->second == get<2>(ttm).begin()->second)
    {
        st_elem_p = get<1>(ttm)->begin()->second;
    }

    const Element* en_elem_p = get<0>(ttm)->rbegin()->second;
    if(get<0>(ttm)->rbegin()->second == get<2>(ttm).rbegin()->second)
    {
        en_elem_p = get<1>(ttm)->rbegin()->second;
    }

    // compute length of overlap
    int last_start;
    int first_end;
    size_t i = 0;
    BOOST_FOREACH(const typename Track::value_type& pr, get<2>(ttm))
    {
        if(pr.second == st_elem_p)
        {
            last_start = i;
        }

        if(pr.second == en_elem_p)
        {
            first_end = i;
        }

        i++;
    }

    // remove merged tracks and add the result of merging them
    w_p.erase(get<0>(ttm));
    w_p.erase(get<1>(ttm));
    w_p.insert(get<2>(ttm));

    // compute forward probability
    absorption_info.num_valid_track_pairs = M.size();
    secretion_info.num_valid_tracks = count_secretion_tracks(w_p);
    secretion_info.window_size = std::abs(last_start - first_end + 1);
    return p_absorption(w, w_p);
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

template <class Track>
double Proposer<Track>::p_birth(const Assoc& w, const Assoc& w_p) const
{
    //KJB(ASSERT(w.is_valid(m_v_bar, m_d_bar, m_noise_sigma, m_convert)));
    //KJB(ASSERT(w_p.is_valid(m_v_bar, m_d_bar, m_noise_sigma, m_convert)));

#ifdef TEST
    if(!is_valid_birth(w, w_p))
    {
        std::cout << "WARNING: invalid birth." << std::endl;
        return std::log(0.0);
    }
#endif

    const Track& extra_track = *birth_info.new_track_p;
    double p = 0.0;

    // compute first point and probability of choosing that time first
    Track_const_iterator pair_p = extra_track.begin();
    int t_1 = pair_p->first;
    p -= std::log(w.get_data().size());

    // compute probability of choosing first point
    int num_valid = w.count_dead_points_at_time(t_1);

    if(num_valid == 0)
    {
        return negative_infinity();
    }

    p -= std::log(num_valid);

    // probability of growing track to what it is
    p += p_grow_track_forward(extra_track, w, t_1);

    return p;
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

template <class Track>
double Proposer<Track>::p_death(const Assoc& w, const Assoc& w_p) const
{
    //KJB(ASSERT(w.is_valid(m_v_bar, m_d_bar, m_noise_sigma, m_convert)));
    //KJB(ASSERT(w_p.is_valid(m_v_bar, m_d_bar, m_noise_sigma, m_convert)));

#ifdef TEST
    if(!is_valid_death(w, w_p))
    {
        std::cout << "WARNING: invalid death." << std::endl;
        return std::log(0.0);
    }
#endif

    return -std::log(death_info.num_tracks);
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

template <class Track>
double Proposer<Track>::p_split(const Assoc& w, const Assoc& w_p) const
{
    //KJB(ASSERT(w.is_valid(m_v_bar, m_d_bar, m_noise_sigma, m_convert)));
    //KJB(ASSERT(w_p.is_valid(m_v_bar, m_d_bar, m_noise_sigma, m_convert)));

#ifdef TEST
    if(!is_valid_split(w, w_p))
    {
        std::cout << "WARNING: invalid split." << std::endl;
        return std::log(0.0);
    }
#endif

    size_t nsp = split_info.num_split_points;
    if(nsp == 0)
    {
        return negative_infinity();
    }

    return -std::log(nsp);
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

template <class Track>
double Proposer<Track>::p_merge(const Assoc& w, const Assoc& w_p) const
{
    //KJB(ASSERT(w.is_valid(m_v_bar, m_d_bar, m_noise_sigma, m_convert)));
    //KJB(ASSERT(w_p.is_valid(m_v_bar, m_d_bar, m_noise_sigma, m_convert)));

#ifdef TEST
    if(!is_valid_merge(w, w_p))
    {
        std::cout << "WARNING: invalid merge." << std::endl;
        return std::log(0.0);
    }
#endif

    size_t nmp = merge_info.num_merge_pairs;
    if(nmp == 0)
    {
        return negative_infinity();
    }

    return -std::log(nmp);
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

template <class Track>
double Proposer<Track>::p_extension
(
    const Assoc& w,
    const Assoc& w_p
) const
{
    //KJB(ASSERT(w.is_valid(m_v_bar, m_d_bar, m_noise_sigma, m_convert)));
    //KJB(ASSERT(w_p.is_valid(m_v_bar, m_d_bar, m_noise_sigma, m_convert)));

#ifdef TEST
    if(!is_valid_extension(w, w_p))
    {
        std::cout << "WARNING: invalid extension." << std::endl;
        return std::log(0.0);
    }
#endif

    double p = -std::log(2 * extension_info.num_tracks);

    // probability of growing to what it is
    if(extension_info.direction == GROW_FORWARD)
    {
        p += p_grow_track_forward(*extension_info.extended_track_p, w,
                                  extension_info.previous_end);
    }
    else
    {
        p += p_grow_track_backward(*extension_info.extended_track_p, w,
                                  extension_info.previous_end);
    }

    return p;
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

template <class Track>
double Proposer<Track>::p_reduction
(
    const Assoc& w,
    const Assoc& w_p
) const
{
    //KJB(ASSERT(w.is_valid(m_v_bar, m_d_bar, m_noise_sigma, m_convert)));
    //KJB(ASSERT(w_p.is_valid(m_v_bar, m_d_bar, m_noise_sigma, m_convert)));

#ifdef TEST
    if(!is_valid_reduction(w, w_p))
    {
        std::cout << "WARNING: invalid reduction." << std::endl;
        return std::log(0.0);
    }
#endif

    return std::log(0.5 / (reduction_info.num_tracks
                            * (reduction_info.reduced_track_size - 2)));
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

template <class Track>
double Proposer<Track>::p_switch(const Assoc& w, const Assoc& w_p) const
{
    //KJB(ASSERT(w.is_valid(m_v_bar, m_d_bar, m_noise_sigma, m_convert)));
    //KJB(ASSERT(w_p.is_valid(m_v_bar, m_d_bar, m_noise_sigma, m_convert)));

#ifdef TEST
    if(!is_valid_switch(w, w_p))
    {
        std::cout << "WARNING: invalid switch." << std::endl;
        return std::log(0.0);
    }
#endif

    size_t nsp = switch_info.num_switch_points;
    if(nsp == 0)
    {
        return negative_infinity();
    }

    return -std::log(nsp);
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

template <class Track>
double Proposer<Track>::p_secretion
(
    const Assoc& w,
    const Assoc& w_p
) const
{
    //KJB(ASSERT(w.is_valid(m_v_bar, m_d_bar, m_noise_sigma, m_convert)));
    //KJB(ASSERT(w_p.is_valid(m_v_bar, m_d_bar, m_noise_sigma, m_convert)));

#ifdef TEST
    if(!is_valid_secretion(w, w_p))
    {
        std::cout << "WARNING: invalid secretion." << std::endl;
        return std::log(0.0);
    }
#endif

    if(secretion_info.num_valid_tracks == 0)
    {
        return negative_infinity();
    }

    double p = -std::log(secretion_info.num_valid_tracks);

    p += (secretion_info.window_size)*std::log(0.5);

    return p;
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

template <class Track>
double Proposer<Track>::p_absorption
(
    const Assoc& w,
    const Assoc& w_p
) const
{
    //KJB(ASSERT(w.is_valid(m_v_bar, m_d_bar, m_noise_sigma, m_convert)));
    //KJB(ASSERT(w_p.is_valid(m_v_bar, m_d_bar, m_noise_sigma, m_convert)));

#ifdef TEST
    if(!is_valid_absorption(w, w_p))
    {
        std::cout << "WARNING: invalid absorption." << std::endl;
        return std::log(0.0);
    }
#endif

    return -std::log(absorption_info.num_valid_track_pairs);
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

template <class Track>
inline
bool Proposer<Track>::is_valid_birth
(
    const Assoc& w,
    const Assoc& w_p
) const
{
    if(w_p.empty() || w_p.size() - w.size() != 1)
    {
        return false;
    }

    std::pair<Assoc_const_iterator, Assoc_const_iterator> diff_p;
    diff_p = std::mismatch(w.begin(), w.end(), w_p.begin());
    if(diff_p.first == w.end())
    {
        return true;
    }

    diff_p.second++;
    diff_p = std::mismatch(diff_p.first, w.end(), diff_p.second);
    if(diff_p.first == w.end())
    {
        return true;
    }

    return false;
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

template <class Track>
inline
bool Proposer<Track>::is_valid_death
(
    const Assoc& w,
    const Assoc& w_p
) const
{
    return is_valid_birth(w_p, w);
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

template <class Track>
inline
bool Proposer<Track>::is_valid_split
(
    const Assoc& w,
    const Assoc& w_p
) const
{
    if(w.empty() || w_p.size() - w.size() != 1)
    {
        return false;
    }

    std::set<Track, Compare_tracks<Track> > w_minus_w_p;
    std::set_difference(w.begin(), w.end(), w_p.begin(), w_p.end(),
        std::inserter(w_minus_w_p, w_minus_w_p.begin()),
        Compare_tracks<Track>());

    std::set<Track, Compare_tracks<Track> > w_p_minus_w;
    std::set_difference(w_p.begin(), w_p.end(), w.begin(), w.end(),
        std::inserter(w_p_minus_w, w_p_minus_w.begin()),
        Compare_tracks<Track>());

    if(w_minus_w_p.size() != 1 || w_p_minus_w.size() != 2)
    {
        return false;
    }

    Track merged_track = *w_p_minus_w.begin();
    merged_track.insert((++w_p_minus_w.begin())->begin(),
                        (++w_p_minus_w.begin())->end());

    if(*w_minus_w_p.begin() != merged_track)
    {
        return false;
    }

    return true;
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

template <class Track>
inline
bool Proposer<Track>::is_valid_merge
(
    const Assoc& w,
    const Assoc& w_p
) const
{
    return is_valid_split(w_p, w);
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

template <class Track>
bool Proposer<Track>::is_valid_extension
(
    const Assoc& w,
    const Assoc& w_p
) const
{
    if(w.empty() || w.size() != w_p.size())
    {
        return false;
    }

    // Find original track
    std::set<Track, Compare_tracks<Track> > diff;
    std::set_difference(w.begin(), w.end(), w_p.begin(), w_p.end(),
        std::inserter(diff, diff.begin()), Compare_tracks<Track>());
    if(diff.size() != 1)
    {
        return false;
    }

    const Track& original_track = *w.find(*diff.begin());

    // Find extended track
    diff.clear();
    std::set_difference(w_p.begin(), w_p.end(), w.begin(), w.end(),
        std::inserter(diff, diff.begin()), Compare_tracks<Track>());

    if(diff.size() != 1)
    {
        return false;
    }

    Assoc_const_iterator extended_track_p = w_p.find(*diff.begin());

    std::pair<Track_const_iterator, Track_const_iterator> diff_ps
        = std::mismatch(original_track.begin(), original_track.end(),
                        extended_track_p->begin());
    if(diff_ps.first == original_track.end())
    {
        return true;
    }

    std::pair<typename Track::const_reverse_iterator,
              typename Track::const_reverse_iterator>
        r_diff_ps = std::mismatch(original_track.rbegin(), original_track.rend(),
                                  extended_track_p->rbegin());
    if(r_diff_ps.first == original_track.rend())
    {
        return true;
    }

    return false;
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

template <class Track>
inline
bool Proposer<Track>::is_valid_reduction
(
    const Assoc& w,
    const Assoc& w_p
) const
{
    return is_valid_extension(w_p, w);
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

template <class Track>
bool Proposer<Track>::is_valid_switch
(
    const Assoc& w,
    const Assoc& w_p
) const
{
    if(w.size() < 2 || w.size() != w_p.size())
    {
        return false;
    }

    // Find original tracks
    std::set<Track, Compare_tracks<Track> > diff;
    std::set_difference(w.begin(), w.end(), w_p.begin(), w_p.end(),
        std::inserter(diff, diff.begin()), Compare_tracks<Track>());
    if(diff.size() != 2)
    {
        return false;
    }

    const Track& original_track_1 = *w.find(*diff.begin());
    const Track& original_track_2 = *w.find(*diff.rbegin());

    // Find swapped tracks
    diff.clear();
    std::set_difference(w_p.begin(), w_p.end(), w.begin(), w.end(),
        std::inserter(diff, diff.begin()), Compare_tracks<Track>());

    if(diff.size() != 2)
    {
        return false;
    }

    const Track& swapped_track_1 = *w_p.find(*diff.begin());
    const Track& swapped_track_2 = *w_p.find(*diff.rbegin());

    std::pair<Track_const_iterator, Track_const_iterator> track1_break
            = std::mismatch(original_track_1.begin(), original_track_1.end(),
                            swapped_track_1.begin());

    std::pair<Track_const_iterator, Track_const_iterator> track2_break
            = std::mismatch(original_track_2.begin(), original_track_2.end(),
                            swapped_track_2.begin());

    if(track1_break.first == original_track_1.end())
    {
        return false;
    }

    if(!std::equal(original_track_1.begin(), track1_break.first, 
                                             swapped_track_1.begin()) ||
       !std::equal(track1_break.first, original_track_1.end(),
                                             track2_break.second) ||
       !std::equal(original_track_2.begin(), track2_break.first,
                                             swapped_track_2.begin()) ||
       !std::equal(track2_break.first, original_track_2.end(),
                                             track1_break.second))
    {
        return false;
    }

    return true;
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

template <class Track>
inline
bool Proposer<Track>::is_valid_secretion
(
    const Assoc& w,
    const Assoc& w_p
) const
{
    if(w.empty() || w_p.size() - w.size() != 1)
    {
        return false;
    }

    std::set<Track, Compare_tracks<Track> > w_minus_w_p;
    std::set_difference(w.begin(), w.end(), w_p.begin(), w_p.end(),
        std::inserter(w_minus_w_p, w_minus_w_p.begin()),
        Compare_tracks<Track>());

    std::set<Track, Compare_tracks<Track> > w_p_minus_w;
    std::set_difference(w_p.begin(), w_p.end(), w.begin(), w.end(),
        std::inserter(w_p_minus_w, w_p_minus_w.begin()),
        Compare_tracks<Track>());

    if(w_minus_w_p.size() != 1 || w_p_minus_w.size() != 2)
    {
        return false;
    }

    /*Assoc_const_iterator orig_track_p = w_minus_w_p.begin();
    Assoc_const_iterator new_track_p1 = w_p_minus_w.begin();
    Assoc_const_iterator new_track_p2 = w_p_minus_w.end();
    new_track_p2--;

    Track merged_track = *new_track_p1;
    merged_track.insert(new_track_p2->begin(), new_track_p2->end());

    if(*orig_track_p != merged_track)
    {
        return false;
    }

    if(orig_track_p->begin()->second != new_track_p1->begin()->second
        && orig_track_p->begin()->second != new_track_p2->begin()->second)
    {
        return false;
    }*/

    return true;
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

template <class Track>
inline
bool Proposer<Track>::is_valid_absorption
(
    const Assoc& w,
    const Assoc& w_p
) const
{
    return is_valid_secretion(w_p, w);
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

template <class Track>
void Proposer<Track>::grow_track_forward
(
    Track& track,
    const Assoc& w
) const
{
    int t = track.get_end_time() + 1;

    if(t >= w.get_data().size()) return;

    for(;;)
    {
        int prev_t = track.get_end_time();
        int d = t - prev_t;
        if(d > m_b_bar) break;

        std::pair<Vector, Vector> fut = track_future_ls(track);
        const Vector& x = fut.first;
        const Vector& vel = fut.second;

        // get candidates and their probabilities
        //std::set<const Element*> dpt = w.dead_neighbors(x, prev_t, d,
        //                    m_b_bar, m_v_bar, m_noise_sigma, m_convert);
        std::set<const Element*> dpt = w.get_dead_points_at_time(t);
        Probability_map probs = 
            get_growing_probabilities(track, prev_t, dpt, x, vel, d);

        size_t num_added = 0;
        BOOST_FOREACH(const typename Probability_map::left_map::value_type& pr,
                      probs.left)
        {
            double p_det = std::exp(pr.first);
            if(p_det > 0.15)
            {
                if(kjb::sample(Uniform_distribution()) < p_det)
                {
                    track.insert(std::make_pair(t, pr.second));
                    num_added++;
                }
            }
        }

        // if nothing sampled
        if(num_added == 0)
        {
            // stop with probability gamma
            if(kjb::sample(Uniform_distribution()) < m_gamma) break;
        }

        if(t == w.get_data().size()) break;
        t++;
    }
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

template <class Track>
void Proposer<Track>::grow_track_backward
(
    Track& track,
    const Assoc& w
) const
{
    int t = track.get_start_time() - 1;

    if(t <= 1) return;

    for(;;)
    {
        int prev_t = track.get_start_time();
        int d = t - prev_t;
        if(std::abs(d) > m_b_bar) break;

        std::pair<Vector, Vector> fut = track_past_ls(track);
        const Vector& x = fut.first;
        const Vector& vel = fut.second;

        // get candidates and their probabilities
        //std::set<const Element*> dpt = w.dead_neighbors(x, prev_t, d,
        //                    m_b_bar, m_v_bar, m_noise_sigma, m_convert);
        std::set<const Element*> dpt = w.get_dead_points_at_time(t);
        Probability_map probs = 
            get_growing_probabilities(track, prev_t, dpt, x, vel, d);

        size_t num_added = 0;
        BOOST_FOREACH(const typename Probability_map::left_map::value_type& pr,
                      probs.left)
        {
            double p_det = std::exp(pr.first);

            if(kjb::sample(Uniform_distribution()) < p_det)
            {
                track.insert(std::make_pair(t, pr.second));
                num_added++;
            }
        }

        // if nothing sampled
        if(num_added == 0)
        {
            // stop with probability gamma
            if(kjb::sample(Uniform_distribution()) < m_gamma) break;
        }

        if(t == 1) break;
        t--;
    }
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

template <class Track>
double Proposer<Track>::p_grow_track_forward
(
    const Track& track,
    const Assoc& w,
    int t
) const
{
    double p = 0.0;

    // make sure 't' exists
    if(!track.count(t))
    {
        return std::log(0);
    }

    // current end
    Track_const_iterator eot = --track.end();
    if(eot->first == t)
    {
        return negative_infinity();
    }

    Track_const_iterator prev_p = track.upper_bound(t);
    Track_const_iterator next_p = prev_p--;
    t++;

    while(prev_p != eot)
    {
        int prev_t = prev_p->first;

        if(t - prev_t > m_d_bar) 
        {
            return negative_infinity();
        }

        std::pair<Vector, Vector> fut = track_future_ls(track, prev_t);
        const Vector& prev_x = fut.first;
        const Vector& vel = fut.second;

        std::set<const Element*> dpt = w.get_dead_points_at_time(t);
        Probability_map probs = get_growing_probabilities(track, prev_t, 
                                            dpt, prev_x, vel, t - prev_t);

        // start by assuming none got added...
        p += p_choose_nothing(probs);
        if(t == next_p->first)
        {
            // for each added correct by...
            while(next_p != track.end() && next_p->first == t)
            {
                const Element& x = *next_p->second;
                typename Probability_map::right_map::const_iterator
                                            pr_p = probs.right.find(&x);
                if(pr_p == probs.right.end())
                {
                    std::cout << "WARNING: this should never happen!\n";
                }
                else
                {
                    // ...multiply by prob of choosing and divide by
                    // prob of not choosing
                    if(p != negative_infinity())
                    {
                        p += (pr_p->second) - log(1.0 - exp(pr_p->second));
                    }
                }
                next_p++;
            }

            prev_p = next_p;
            prev_p--;
        }
        else
        {
            p += std::log(1.0 - m_gamma);
        }

        t++;
    }

    if(t > w.get_data().size())
    {
        return p;
    }

    // compute probability of stopping; add probabliities of stopping
    // in different ways
    double p_last = 0.0;

    int pt = prev_p->first;
    std::pair<Vector, Vector> fut = track_future_ls(track, pt);
    const Vector& px = fut.first;
    const Vector& vel = fut.second;
    //std::set<const Element*> dpt = w.dead_neighbors(px, pt, 1,
    //               m_d_bar, m_v_bar, m_noise_sigma, m_convert);
    std::set<const Element*> dpt = w.get_dead_points_at_time(pt + 1);
    Probability_map probs = 
        get_growing_probabilities(track, pt, dpt, px, vel, 1);

    double a_t_1 = std::exp(p_choose_nothing(probs)) * m_gamma;
    p_last = a_t_1;

    t++;
    int last_t = std::min(pt + m_d_bar, (int)w.get_data().size());
    for(; t <= last_t; t++)
    {
        //dpt = w.dead_neighbors(px, pt, t - pt,
        //      m_d_bar, m_v_bar, m_noise_sigma, m_convert);
        dpt = w.get_dead_points_at_time(t);
        probs = get_growing_probabilities(track, pt, dpt, px, vel, t - pt);
        double a_t = a_t_1 * (1 - m_gamma)
                           * std::exp(p_choose_nothing(probs));
        p_last += a_t;
        a_t_1 = a_t;
    }

    p += std::log(p_last);

    return p;
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

template <class Track>
double Proposer<Track>::p_grow_track_backward
(
    const Track& track,
    const Assoc& w,
    int t
) const
{
    double p = 0.0;

    // make sure 't' exists
    if(!track.count(t))
    {
        return std::log(0.0);
    }

    // current end
    Track_const_riterator eot = --track.rend();
    if(eot->first == t)
    {
        return negative_infinity();
    }

    Track_const_riterator prev_p(track.lower_bound(t));
    Track_const_riterator next_p = prev_p--;
    t--;

    while(prev_p != eot)
    {
        int prev_t = prev_p->first;

        std::pair<Vector, Vector> fut = track_past_ls(track, prev_t);
        const Vector& prev_x = fut.first;
        const Vector& vel = fut.second;

        std::set<const Element*> dpt = w.get_dead_points_at_time(t);
        Probability_map probs = get_growing_probabilities(track, prev_t, dpt, 
                                      prev_x, vel, t - prev_t);

        // start by assuming none got added
        p += p_choose_nothing(probs);
        if(t == next_p->first)
        {
            // for each added correct by...
            while(next_p != track.rend() && next_p->first == t)
            {
                const Element& x = *next_p->second;
                typename Probability_map::right_map::const_iterator
                                            pr_p = probs.right.find(&x);
                if(pr_p == probs.right.end())
                {
                    std::cout << "WARNING: this should never happen!\n";
                }
                else
                {
                    // ...multiply by prob of choosing and divide by
                    // prob of not choosing
                    if(p != negative_infinity())
                    {
                        p += (pr_p->second) - log(1.0 - exp(pr_p->second));
                    }
                }
                next_p++;
            }

            prev_p = next_p;
            prev_p--;
        }
        else
        {
            p += std::log(1.0 - m_gamma);
        }

        t--;
    }

    if(t < 1)
    {
        return p;
    }

    // compute probability of stopping; add probabliities of stopping
    // in different ways
    double p_last = 0.0;

    int pt = prev_p->first;
    std::pair<Vector, Vector> fut = track_past_ls(track, pt);
    const Vector& px = fut.first;
    const Vector& vel = fut.second;
    //std::set<const Element*> dpt = w.dead_neighbors(px, pt, -1,
    //               m_d_bar, m_v_bar, m_noise_sigma, m_convert);
    std::set<const Element*> dpt = w.get_dead_points_at_time(pt - 1);
    Probability_map probs = 
        get_growing_probabilities(track, pt, dpt, px, vel, -1);

    double a_t_1 = std::exp(p_choose_nothing(probs)) * m_gamma;
    p_last = a_t_1;

    t--;
    int last_t = std::max(pt - m_d_bar, 1);
    for(; t >= last_t; t--)
    {
        //dpt = w.dead_neighbors(px, pt, t - pt,
        //      m_d_bar, m_v_bar, m_noise_sigma, m_convert);
        dpt = w.get_dead_points_at_time(t);
        probs = 
            get_growing_probabilities(track, pt, dpt, px, vel, t - pt);
        double a_t = a_t_1 * (1 - m_gamma)
                           * std::exp(p_choose_nothing(probs));
        p_last += a_t;
        a_t_1 = a_t;
    }

    p += std::log(p_last);

    return p;
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

template <class Track>
double Proposer<Track>::p_choose_nothing
(
    const Probability_map& probabilities
) const
{
    double p = 0.0;
    BOOST_FOREACH(const typename Probability_map::left_map::value_type& pr,
                  probabilities.left)
    {
        p += std::log(1.0 - std::exp(pr.first));
    }

    return p;
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

template <class Track>
typename Proposer<Track>::Probability_map
Proposer<Track>::get_growing_probabilities
(
    const Track& track, 
    int t,
    std::set<const Element*>& candidates,
    const Vector& x,
    const Vector& vel,
    int d
) const
{
    Probability_map probabilities;
    int dist = std::abs(d);
    double vel_sigma = vel.empty() ? dist*dist*m_v_bar*m_v_bar : 0.0;
    double sg_u = sqrt(m_noise_sigma + vel_sigma);
    double sg_v = sqrt(m_noise_sigma + vel_sigma);
    Normal_distribution N_u(0.0, sg_u);
    Normal_distribution N_v(0.0, sg_v);

    Vector v_x = x;
    if(!vel.empty()) v_x += dist*vel;

    double noise_score = get_velocity_noise_score(d, vel);

    BOOST_FOREACH(const Element* e_p, candidates)
    {
        Vector v = m_convert(*e_p);
        double score = log_pdf(N_u, v_x[0]-v[0]) + log_pdf(N_v, v_x[1]-v[1]);
        double p = exp(score) / (exp(score) + noise_score);

        // probabilities of features 
        if(m_feature_prob)
        {
            std::vector<double> f_ps
                = m_feature_prob(&track, t, e_p, t + d, m_b_bar);

            // Take the geometric mean 
            double pp = 1.0;
            BOOST_FOREACH(double f_p, f_ps)
            {
                pp *= f_p;
            }
            
            p = std::pow(p * pp, 1.0/(1 + f_ps.size()));
        }

        typedef typename Probability_map::left_map::value_type value_type;
        double prob = 0.0;
        if(p == 0.0)
        {
            prob = negative_infinity();
        }
        else
        {
            prob = std::log(p);
        }

        probabilities.left.insert(value_type(prob, e_p));
    }

    return probabilities;
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

template <class Track>
std::pair<Vector, Vector> Proposer<Track>::track_future
(
    const Track& track,
    int t
) const
{
    // constants
    const size_t wsz = m_b_bar / 4;
    const double vel_thresh = 0.0;

    Track_const_riterator pair_p;
    if(t < 0)
    {
        pair_p = track.rbegin();
        t = pair_p->first;
    }
    else
    {
        //pair_p = Track_const_riterator(track.lower_bound(t));
        //pair_p--;
        pair_p = Track_const_riterator(track.upper_bound(t));
        IFT(pair_p->first == t, Illegal_argument,
            "The frame passed to track_future() must be valid.");
    }

    // average final points
    std::vector<const Element*> cur_elems;
    while(pair_p != track.rend() && pair_p->first == t)
    {
        cur_elems.push_back(pair_p->second);
        pair_p++;
    }

    Element x = m_average(cur_elems);
    if(pair_p == track.rend())
    {
        return std::make_pair(m_convert(x), Vector());
    }

    // compute direction in window
    size_t wnd = t - pair_p->first;
    while(wnd < wsz)
    {
        pair_p++;
        if(pair_p == track.rend()) break;
        wnd = t - pair_p->first;
    }

    Vector vel;
    if(wnd >= wsz)
    {
        std::vector<const Element*> cur_elems;
        while(pair_p != track.rend() && pair_p->first == t - wnd)
        {
            cur_elems.push_back(pair_p->second);
            pair_p++;
        }

        Element from = m_average(cur_elems);

        vel = m_convert(x) - m_convert(from);
        vel /= wnd;

        if(vel.magnitude() <= vel_thresh)
        {
            vel.set(0.0, 0.0);
        }
    }

    return std::make_pair(m_convert(x), vel);
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

template <class Track>
std::pair<Vector, Vector> Proposer<Track>::track_past
(
    const Track& track,
    int t
) const
{
    // constants
    const size_t wsz = m_b_bar / 4;
    const double vel_thresh = 0.0;

    Track_const_iterator pair_p;
    if(t < 0)
    {
        pair_p = track.begin();
        t = pair_p->first;
    }
    else
    {
        //pair_p = track.upper_bound(t);
        //pair_p--;
        pair_p = track.lower_bound(t);
        IFT(pair_p->first == t, Illegal_argument,
            "The frame passed to track_past() must be valid.");
    }

    // average final points
    std::vector<const Element*> cur_elems;
    while(pair_p != track.end() && pair_p->first == t)
    {
        cur_elems.push_back(pair_p->second);
        pair_p++;
    }

    Element x = m_average(cur_elems);
    if(pair_p == track.end())
    {
        return std::make_pair(m_convert(x), Vector());
    }

    // compute direction in window
    size_t wnd = pair_p->first - t;
    while(wnd < wsz)
    {
        pair_p++;
        if(pair_p == track.end()) break;
        wnd = pair_p->first - t;
    }

    Vector vel;
    if(wnd >= wsz)
    {
        std::vector<const Element*> cur_elems;
        while(pair_p != track.end() && pair_p->first == t + wnd)
        {
            cur_elems.push_back(pair_p->second);
            pair_p++;
        }

        Element from = m_average(cur_elems);

        vel = m_convert(x) - m_convert(from);
        vel /= wnd;

        if(vel.magnitude() <= vel_thresh)
        {
            vel.set(0.0, 0.0);
        }
    }

    return std::make_pair(m_convert(x), vel);
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

template <class Track>
std::pair<Vector, Vector> Proposer<Track>::track_future_ls
(
    const Track& track,
    int t
) const
{
    // constants
    const size_t wsz = m_b_bar;
    const double vel_thresh = 0.0;

    Track_const_riterator pair_p;
    if(t < 0)
    {
        pair_p = track.rbegin();
        t = pair_p->first;
    }
    else
    {
        //pair_p = Track_const_riterator(track.lower_bound(t));
        //pair_p--;
        pair_p = Track_const_riterator(track.upper_bound(t));
        IFT(pair_p->first == t, Illegal_argument,
            "The frame passed to track_future() must be valid.");
    }

    double x_N = m_convert(*pair_p->second)[0];
    double y_N = m_convert(*pair_p->second)[1];
    double sum_x = x_N;
    double sum_y = y_N;
    double sum_x_squared = x_N*x_N;
    double sum_xy = x_N*y_N;
    size_t N = 1;
    double x_1;
    double y_1;

    // average final points
    std::vector<const Element*> cur_elems;
    size_t wnd = t - pair_p->first;
    while(wnd < wsz)
    {
        if(pair_p->first == t)
        {
            cur_elems.push_back(pair_p->second);
        }
        pair_p++;
        if(pair_p == track.rend()) break;
        double x_i = m_convert(*pair_p->second)[0];
        double y_i = m_convert(*pair_p->second)[1];
        wnd = t - pair_p->first;
        sum_x += x_i;
        sum_y += y_i;
        sum_x_squared += (x_i * x_i);
        sum_xy += (x_i * y_i);
        x_1 = x_i;
        y_1 = y_i;
        N++;
    }

    if(wnd >= wsz)
    {
        Vector x;
        Vector vel;

        Element e_N = m_average(cur_elems);
        Vector p_N = m_convert(e_N);
        double den = (sum_x_squared - (sum_x*sum_x)/N);
        if(den != 0)
        {
            double m = (sum_xy - (sum_x*sum_y)/N) / den;
            double b = sum_y / N - m * sum_x / N;

            double f_1 = m * x_1 + b;
            double f_N = m * p_N[0] + b;

            x.set(p_N[0], f_N);
            vel.set(p_N[0] - x_1, f_N - f_1);
            vel /= wnd;
            if(vel.magnitude() <= vel_thresh)
            {
                vel.set(0.0, 0.0);
            }
        }
        else
        {
            x = p_N;
            vel.set(0.0, p_N[1] - y_1);
        }

        return std::make_pair(x, vel);
    }

    // if we can't fit a line we just
    // do it the bad way
    return track_future(track, t);
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

template <class Track>
std::pair<Vector, Vector> Proposer<Track>::track_past_ls
(
    const Track& track,
    int t
) const
{
    // constants
    const size_t wsz = m_b_bar;
    const double vel_thresh = 0.0;

    Track_const_iterator pair_p;
    if(t < 0)
    {
        pair_p = track.begin();
        t = pair_p->first;
    }
    else
    {
        //pair_p = track.upper_bound(t);
        //pair_p--;
        pair_p = track.lower_bound(t);
        IFT(pair_p->first == t, Illegal_argument,
            "The frame passed to track_future() must be valid.");
    }

    double x_N = m_convert(*pair_p->second)[0];
    double y_N = m_convert(*pair_p->second)[1];
    double sum_x = x_N;
    double sum_y = y_N;
    double sum_x_squared = x_N*x_N;
    double sum_xy = x_N*y_N;
    size_t N = 1;
    double x_1;
    double y_1;

    // average the final points 
    std::vector<const Element*> cur_elems;

    size_t wnd = pair_p->first - t;
    while(wnd < wsz)
    {
        if(pair_p->first == t)
        {
            cur_elems.push_back(pair_p->second);
        }
        pair_p++;
        if(pair_p == track.end()) break;
        double x_i = m_convert(*pair_p->second)[0];
        double y_i = m_convert(*pair_p->second)[1];
        wnd = pair_p->first - t;
        sum_x += x_i;
        sum_y += y_i;
        sum_x_squared += (x_i * x_i);
        sum_xy += (x_i * y_i);
        x_1 = x_i;
        y_1 = y_i;
        N++;
    }

    if(wnd >= wsz)
    {
        Vector x;
        Vector vel;

        Element e_N = m_average(cur_elems);
        Vector p_N = m_convert(e_N);
        double den = (sum_x_squared - (sum_x*sum_x)/N);
        if(den != 0)
        {
            double m = (sum_xy - (sum_x*sum_y)/N) / den;
            double b = sum_y / N - m * sum_x / N;

            double f_1 = m * x_1 + b;
            double f_N = m * p_N[0] + b;

            x.set(p_N[0], f_N);
            vel.set(p_N[0] - x_1, f_N - f_1);
            vel /= wnd;
            if(vel.magnitude() <= vel_thresh)
            {
                vel.set(0.0, 0.0);
            }
        }
        else
        {
            x = p_N;
            vel.set(0.0, p_N[1] - y_1);
        }

        return std::make_pair(x, vel);
    }

    // if we can't fit a line we just
    // do it the bad way
    return track_future(track, t);
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

template <class Track>
std::pair<Vector, Vector> Proposer<Track>::track_future_tls
(
    const Track& track,
    int t
) const
{
    // constants
    const size_t wsz = m_b_bar;
    const double vel_thresh = 0.0;

    Track_const_riterator pair_p;
    if(t < 0)
    {
        pair_p = track.rbegin();
        t = pair_p->first;
    }
    else
    {
        //pair_p = Track_const_riterator(track.lower_bound(t));
        //pair_p--;
        pair_p = Track_const_riterator(track.upper_bound(t));
        IFT(pair_p->first == t, Illegal_argument,
            "The frame passed to track_future() must be valid.");
    }

    Vector x = Vector().set(1, m_convert(*pair_p->second)[0]);;
    Vector y = Vector().set(1, m_convert(*pair_p->second)[1]);;
    size_t wnd = t - pair_p->first;
    while(wnd < wsz)
    {
        pair_p++;
        if(pair_p == track.rend()) break;

        wnd = t - pair_p->first;

        x.push_back(m_convert(*pair_p->second)[0]);
        y.push_back(m_convert(*pair_p->second)[1]);
    }

    if(wnd >= wsz)
    {
        double x_1 = x.back();
        double x_n = x.front();
        double x_bar = mean(x.begin(), x.end());
        double y_bar = mean(y.begin(), y.end());
        x -= Vector(x.get_length(), x_bar);
        y -= Vector(y.get_length(), y_bar);

        Matrix A(x.get_length(), 2);
        A.set_col(0, x);
        A.set_col(1, y);
        Matrix V;
        Vector d;
        diagonalize(matrix_transpose(A)*A, V, d, true);

        double a = V(0, 1);
        double b = V(1, 1);
        double r = a*x_bar + b*y_bar;
        double y_1 = (r - a*x_1)/b;
        double y_n = (r - a*x_n)/b;

        Vector vel = Vector().set(x_n - x_1, y_n - y_1) / wnd;
        Vector x_s = Vector().set(x_n, y_n);
        if(vel.magnitude() <= vel_thresh)
        {
            vel.set(0.0, 0.0);
        }

        return std::make_pair(x_s, vel);
    }

    // if we can't fit a line we just
    // do it the bad way
    return track_future(track, t);
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

template <class Track>
std::pair<Vector, Vector> Proposer<Track>::track_past_tls
(
    const Track& track,
    int t
) const
{
    // constants
    const size_t wsz = m_b_bar;
    const double vel_thresh = 0.0;

    Track_const_iterator pair_p;
    if(t < 0)
    {
        pair_p = track.begin();
        t = pair_p->first;
    }
    else
    {
        //pair_p = track.upper_bound(t);
        //pair_p--;
        pair_p = track.lower_bound(t);
        IFT(pair_p->first == t, Illegal_argument,
            "The frame passed to track_future() must be valid.");
    }

    Vector x = Vector().set(1, m_convert(*pair_p->second)[0]);;
    Vector y = Vector().set(1, m_convert(*pair_p->second)[1]);;
    size_t wnd = pair_p->first - t;
    while(wnd < wsz)
    {
        pair_p++;
        if(pair_p == track.end()) break;

        wnd = pair_p->first - t;

        x.push_back(m_convert(*pair_p->second)[0]);
        y.push_back(m_convert(*pair_p->second)[1]);
    }

    if(wnd >= wsz)
    {
        double x_1 = x.back();
        double x_n = x.front();
        double x_bar = mean(x.begin(), x.end());
        double y_bar = mean(y.begin(), y.end());
        x -= Vector(x.get_length(), x_bar);
        y -= Vector(y.get_length(), y_bar);

        Matrix A(x.get_length(), 2);
        A.set_col(0, x);
        A.set_col(1, y);
        Matrix V;
        Vector d;
        diagonalize(matrix_transpose(A)*A, V, d, true);

        double a = V(0, 1);
        double b = V(1, 1);
        double r = a*x_bar + b*y_bar;
        double y_1 = (r - a*x_1)/b;
        double y_n = (r - a*x_n)/b;

        Vector vel = Vector().set(x_n - x_1, y_n - y_1) / wnd;
        Vector x_s = Vector().set(x_n, y_n);
        if(vel.magnitude() <= vel_thresh)
        {
            vel.set(0.0, 0.0);
        }

        return std::make_pair(x_s, vel);
    }

    // if we can't fit a line we just
    // do it the bad way
    return track_future(track, t);
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

template <class Track>
size_t Proposer<Track>::count_split_points(const Assoc& w) const
{
    size_t nsp = 0;
    BOOST_FOREACH(const Track& track, w)
    {
        size_t track_sz = track.real_size();
        if(track_sz >= 4)
        {
            nsp += (track_sz - 3);
        }
    }

    return nsp;
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

template <class Track>
size_t Proposer<Track>::count_merge_pairs(const Assoc& w) const
{
    size_t nmp = 0;
    for(Assoc_const_iterator track_p1 = w.begin();
                             track_p1 != w.end();
                             track_p1++)
    {
        int t_f = track_p1->get_end_time();
        for(Assoc_const_iterator track_p2 = w.begin();
                                 track_p2 != w.end();
                                 track_p2++)
        {
            int t_1 = track_p2->get_start_time();
            if(track_p1 != track_p2 && t_f < t_1)
            {
                if(is_neighbor(track_p1->get_end_point(),
                               track_p2->get_start_point(),
                               t_1 - t_f, m_d_bar, m_v_bar,
                               m_noise_sigma, m_convert))
                {
                    nmp++;
                }
            }
        }
    }

    return nmp;
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

template <class Track>
size_t Proposer<Track>::count_secretion_tracks(const Assoc& w) const
{
    size_t nst = 0;
    BOOST_FOREACH(const Track& track, w)
    {
        if(track.real_size() >= 4)
        {
            nst++;
        }
        else
        {
            if(track.count(track.get_start_time()) > 1
                && track.count(track.get_end_time()) > 1)
            {
                nst++;
            }
        }
    }

    return nst;
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

template <class Track>
size_t Proposer<Track>::count_absorption_pairs(const Assoc& w) const
{
    /*size_t nap = 0;
    for(Assoc_const_iterator track_p1 = w.begin();
                             track_p1 != w.end();
                             track_p1++)
    {
        Assoc_const_iterator track_p2 = track_p1;
        for(track_p2++; track_p2 != w.end(); track_p2++)
        {
            Track new_track = *track_p1;
            new_track.insert(track_p2->begin(), track_p2->end());
            if(new_track.is_valid(m_v_bar, m_d_bar, m_noise_sigma, m_convert))
            {
                nap++;
            }
        }
    }

    return nap;*/
    return (w.size() * (w.size() - 1)) / 2;
}

}} //namespace kjb::mcmcda

#endif /*MCMCDA_PROPOSER_H_INCLUDED */

