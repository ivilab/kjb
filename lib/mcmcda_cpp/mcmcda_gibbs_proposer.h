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

#ifndef MCMCDA_GIBBS_PROPOSER_H_INCLUDED
#define MCMCDA_GIBBS_PROPOSER_H_INCLUDED

#include <mcmcda_cpp/mcmcda_data.h>
#include <mcmcda_cpp/mcmcda_association.h>
#include <mcmcda_cpp/mcmcda_track.h>
#include <mcmcda_cpp/mcmcda_likelihood.h>
#include <mcmcda_cpp/mcmcda_prior.h>
#include <sample_cpp/sample_base.h>
#include <m_cpp/m_vector.h>
#include <l_cpp/l_util.h>
#include <numeric>
#include <algorithm>
#include <functional>
#include <vector>
#include <set>
#include <utility>
#include <boost/function.hpp>

namespace kjb {
namespace mcmcda {

/**
 * @brief   Gibbs proposal mechanism for tracking. Complies
 *          with Gibbs proposer concept.
 */
template<class Track, class Lhood>
class Gibbs_proposer
{
private:
    typedef typename Track::Element Element;
    typedef std::pair<int, typename std::set<Element>::const_iterator>
                                                                Location_pair;
    const Prior<Track>& m_prior;
    const Lhood& m_likelihood;
    Track def_track;
    int nsize;
    mutable double previous_prior;
    mutable double previous_likelihood;

public:
    /**
     * @brief   Ctor with default-constructible track
     */
    Gibbs_proposer
    (
        const Prior<Track>& prior,
        const Lhood& likelihood,
        int neighborhood_size = -1
    ) :
        m_prior(prior),
        m_likelihood(likelihood),
        nsize(neighborhood_size),
        previous_prior(-std::numeric_limits<double>::infinity())
    {}

    /**
     * @brief   Ctor
     */
    Gibbs_proposer
    (
        const Prior<Track>& prior,
        const Lhood& likelihood,
        const Track& default_track,
        int neighborhood_size = -1
    ) :
        m_prior(prior),
        m_likelihood(likelihood),
        def_track(default_track),
        nsize(neighborhood_size),
        previous_prior(-std::numeric_limits<double>::infinity())
    {}

    /**
     * @brief   Samples a new association from a given one, and computes
     *          'forward' and 'reverse' probabilities.
     */
    boost::optional<double> operator()
    (
        Association<Track>& w_p,
        size_t var
    ) const;

    /**
     * @brief   Tests whether a track is affected by current variable.
     */
    bool is_track_affected
    (
        const Track& track,
        const Location_pair& cur_location
    ) const;

    /**
     * @brief   Converts a variable index into a time and point;
     */
    Location_pair get_time_and_place
    (
        size_t var,
        const Data<Element>& data
    ) const;
};

/** @brief  Returns the dimension of an association. */
template<class Track>
inline size_t get_association_dimension(const Association<Track>& w)
{
    std::vector<size_t> dims(w.get_data().size());
    std::transform(w.get_data().begin(), w.get_data().end(), dims.begin(),
                   std::mem_fun_ref(&std::set<typename Track::Element>::size));

    return std::accumulate(dims.begin(), dims.end(), 0);
}

/*============================================================================*
 *                      MEMBER FUNCTION DEFINITIONS                           *
 *----------------------------------------------------------------------------*/

template<class Track, class Lhood>
boost::optional<double> Gibbs_proposer<Track, Lhood>::operator()
(
    Association<Track>& w_p,
    size_t var
) const
{
    using std::make_pair;
    using std::swap;
    typedef typename Track::iterator Tr_it;
    typedef typename Track::const_iterator Tr_cit;

    const Data<Element>& tr_data = w_p.get_data();
    Association<Track> w(tr_data);
    swap(w, w_p);

    /*KJB(ASSERT(w.is_valid(m_prior.get_v_bar(), m_prior.get_d_bar(),
                          m_likelihood.get_convert())));*/

    Location_pair tap = get_time_and_place(var, tr_data);
    int t = tap.first;
    typename std::set<Element>::const_iterator point_p = tap.second;

    typename Association<Track>::const_iterator owner_p;
    for(owner_p = w.begin(); owner_p != w.end(); owner_p++)
    {
        std::pair<Tr_cit, Tr_cit> pair_ps = owner_p->equal_range(t);
        bool found = false;
        for(Tr_cit pair_p = pair_ps.first; pair_p != pair_ps.second; pair_p++)
        {
            if(pair_p->second == &(*point_p))
            {
                found = true;
                break;
            }
        }

        if(found) break;
    }

    // Remove point from its track (making it noise)
    // This will be used to evaluate the noise hypothesis, and will
    // also be used as the basis for constructing the other hypotheses.
    bool was_noise;
    if(owner_p != w.end())
    {
        //w_p = Association<Track>(tr_data);
        //w_p.clear();
        std::remove_copy(w.begin(), w.end(),
                std::insert_iterator<Association<Track> >(w_p, w_p.begin()),
                *owner_p);

        Track track = *owner_p;
        std::pair<Tr_it, Tr_it> pair_ps = track.equal_range(t);
        for(Tr_it pair_p = pair_ps.first; pair_p != pair_ps.second; pair_p++)
        {
            if(pair_p->second == &(*point_p))
            {
                track.erase(pair_p);
                break;
            }
        }

        if(!track.empty())
        {
            w_p.insert(track);
        }

        was_noise = false;
    }
    else
    {
        w_p = w;
        was_noise = true;
    }

    std::vector<Track> affected_tracks;
    std::remove_copy_if(w_p.begin(), w_p.end(),
        std::back_inserter(affected_tracks),
        !boost::bind(&Gibbs_proposer<Track, Lhood>::is_track_affected,
                     this, _1, tap));

    // speed this damn thing up
    if(nsize != -1)
    {
        m_likelihood.set_limits(t - nsize/2, t + nsize/2);
    }

    // prior if assigning to noise
    double prior_noise;
    double likelihood_noise;
    if(previous_prior != -std::numeric_limits<double>::infinity()
            && was_noise && var != 0)
    {
        prior_noise = previous_prior;
        likelihood_noise = previous_likelihood;
    }
    else
    {
        prior_noise = m_prior(w_p);
        likelihood_noise = m_likelihood(w_p);
    }

    // prior if assigning to new track
    Track modified_track = def_track;
    //modified_track[t] = &(*point_p);
    modified_track.insert(make_pair(t, &(*point_p)));
    typename Association<Track>::iterator
            modified_p = w_p.insert(modified_track).first;
    double prior_new = m_prior(w_p);
    double likelihood_new = likelihood_noise
                - m_likelihood.at_noise_point(*point_p)
                + m_likelihood.at_track(modified_track);

    w_p.erase(modified_p);

    /*std::cout << "single-point-noise: " << m_likelihood.at_noise_point(*point_p) << std::endl;
    std::cout << "single-point-new: " << m_likelihood.at_track(modified_track) << std::endl;*/

    /*double prior_existing;
    if(!affected_tracks.empty())
    {
        // prior if assigning to existing track
        // (This is the same regardless of which track it gets
        // added to, so we'll just add it to the first track.)
        Track original_track = *affected_tracks.begin();
        modified_track = original_track;
        //modified_track[t] = &(*point_p);
        modified_track.insert(make_pair(t, &(*point_p)));
        w_p.erase(original_track);
        modified_p = w_p.insert(modified_track).first;
        prior_existing = m_prior(w_p);
        w_p.erase(modified_p);
        w_p.insert(original_track);
    }*/

    std::vector<double> densities(affected_tracks.size() + 2);
    std::vector<double> priors(affected_tracks.size() + 2);
    std::vector<double> likelihoods(affected_tracks.size() + 2);

    priors[0] = prior_noise;
    priors[1] = prior_new;
    likelihoods[0] = likelihood_noise;
    likelihoods[1] = likelihood_new;
    densities[0] = prior_noise + likelihood_noise;
    densities[1] = prior_new + likelihood_new;
    for(size_t i = 0; i < affected_tracks.size(); i++)
    {
        modified_track = affected_tracks[i];
        Track original_track = modified_track;
        //modified_track[t] = &(*point_p);
        modified_track.insert(make_pair(t, &(*point_p)));
        likelihoods[i + 2] = likelihood_noise
                             - m_likelihood.at_track(affected_tracks[i])
                             - m_likelihood.at_noise_point(*point_p)
                             + m_likelihood.at_track(modified_track);

        w_p.erase(original_track);
        modified_p = w_p.insert(modified_track).first;
        priors[i + 2] = m_prior(w_p);
        w_p.erase(modified_p);
        w_p.insert(original_track);

        densities[i + 2] = priors[i + 2] + likelihoods[i + 2];
    }

    // no if statement needed -- resetting limits is always OK
    m_likelihood.reset_limits();

    std::vector<double> probabilities(densities.begin(), densities.end());
    if(affected_tracks.empty())
    {
        probabilities[0] += log(point_p->prob_noise);
        probabilities[1] += log(1.0 - point_p->prob_noise);
    }

    double mx = *std::max_element(probabilities.begin(), probabilities.end());
    std::transform(probabilities.begin(), probabilities.end(),
                   probabilities.begin(),
                   std::bind2nd(std::minus<double>(), mx));
    std::transform(probabilities.begin(), probabilities.end(),
                   probabilities.begin(),
                   std::ptr_fun<double, double>(std::exp));

    double total = std::accumulate(probabilities.begin(),
                                   probabilities.end(), 0.0);
    std::transform(probabilities.begin(), probabilities.end(),
                   probabilities.begin(),
                   std::bind2nd(std::divides<double>(), total));

    int idx = sample(Categorical_distribution<int>(probabilities)) - 1;

    /*=======================================================*
     *                      REPORTING                        */

    /*std::cout << "TIME: " << t << "; BOX: " << std::distance(tr_data[t - 1].begin(), point_p) << std::endl;
    std::cout << "  POSITION: " << m_likelihood.get_convert()(*point_p) << std::endl;
    std::cout << "  P-BOX-NOISE: " << point_p->prob_noise << std::endl;
    std::cout << "  NOISE: " << likelihoods[0] << " (prior: " << priors[0] << ")" " == " << probabilities[0] << std::endl;
    std::cout << "  NEW: " << likelihoods[1] << " (prior: " << priors[1] << ")" " == " << probabilities[1] << std::endl;
    for(size_t i = 0; i < affected_tracks.size(); i++)
    {
        std::cout << "  PART OF " << i + 1 << ": " << likelihoods[i + 2] << " (prior: " << priors[i + 2] << ")" " == " << probabilities[i + 2] << std::endl;
    }
    std::cout << "  CHOSE: " << idx << std::endl;
    std::cout << std::endl;*/

    /*-------------------------------------------------------*/

    if(idx == 1)
    {
        modified_track.clear();
        //modified_track[t] = &(*point_p);
        modified_track.insert(make_pair(t, &(*point_p)));
        w_p.insert(modified_track);
    }
    else if(idx >= 2)
    {
        w_p.erase(affected_tracks[idx - 2]);
        //affected_tracks[idx - 2][t] = &(*point_p);
        affected_tracks[idx - 2].insert(make_pair(t, &(*point_p)));
        w_p.insert(affected_tracks[idx - 2]);
    }

    previous_prior = priors[idx];
    previous_likelihood = likelihoods[idx];

    return densities[idx];
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

template<class Track, class Lhood>
bool Gibbs_proposer<Track, Lhood>::is_track_affected
(
    const Track& track,
    const Location_pair& cur_location
) const
{
    int t = cur_location.first;
    typename std::set<Element>::const_iterator point_p = cur_location.second;
    const double v_bar = m_prior.get_v_bar();
    const int d_bar = m_prior.get_d_bar();
    int d;

    typename Track::const_iterator lb_pair_p = track.lower_bound(t);

    if(lb_pair_p == track.end())
    {
        typename Track::const_reverse_iterator last_pair_p = track.rbegin();
        d = t - last_pair_p->first;

        return is_neighbor(*last_pair_p->second, *point_p, d,
                           d_bar, v_bar, m_likelihood.get_noise_sigma(),
                           m_likelihood.get_convert());
    }

    if(lb_pair_p->first == t)
    {
        return is_neighbor(*point_p, *lb_pair_p->second, 0,
                           d_bar, v_bar, m_likelihood.get_noise_sigma(),
                           m_likelihood.get_convert());
    }

    if(lb_pair_p == track.begin())
    {
        d = lb_pair_p->first - t;

        return is_neighbor(*point_p, *lb_pair_p->second, d,
                           d_bar, v_bar, m_likelihood.get_noise_sigma(),
                           m_likelihood.get_convert());
    }

    typename Track::const_iterator next_pair_p = lb_pair_p--;
    d = next_pair_p->first - t;
    if(!is_neighbor(*next_pair_p->second, *point_p, d, d_bar, v_bar,
                    m_likelihood.get_noise_sigma(),
                    m_likelihood.get_convert()))
    {
        return false;
    }

    d = t - lb_pair_p->first;
    if(!is_neighbor(*point_p, *lb_pair_p->second, d, d_bar, v_bar,
                    m_likelihood.get_noise_sigma(), m_likelihood.get_convert()))
    {
        return false;
    }

    return true;
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

template<class Track, class Lhood>
typename Gibbs_proposer<Track, Lhood>::Location_pair
    Gibbs_proposer<Track, Lhood>::get_time_and_place
(
    size_t var,
    const Data<Element>& data
) const
{
    typename Data<Element>::const_iterator time_p = data.begin();
    int t = 1;
    size_t cur_var = 0;

    while(cur_var + time_p->size() <= var)
    {
        t++;
        cur_var += time_p->size();
        time_p++;
    }

    typename std::set<Element>::const_iterator point_p = time_p->begin();
    std::advance(point_p, var - cur_var);

    return std::make_pair(t, point_p);
}

}} //namespace kjb::psi

#endif /*MCMCDA_GIBBS_PROPOSER_H_INCLUDED */

