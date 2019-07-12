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

#ifndef MCMCDA_LIKELIHOOD_H_INCLUDED
#define MCMCDA_LIKELIHOOD_H_INCLUDED

#include <mcmcda_cpp/mcmcda_association.h>
#include <mcmcda_cpp/mcmcda_track.h>
#include <gp_cpp/gp_multi_output.h>
#include <gp_cpp/gp_mean.h>
#include <gp_cpp/gp_covariance.h>
#include <m_cpp/m_vector.h>
#include <cmath>
#include <boost/function.hpp>

namespace kjb {
namespace mcmcda {

/**
 * @brief   Computes the GP-based likelihood of an association
 *
 * This functor computes the log-likelihood of an association,
 * based on a Gaussian process model of motion. Essentially, it
 * computes the marginal log-likelihood of the data of every
 * track and adds them together. It also computes the likelihood
 * of the noise (the unused data).
 */
template<class Track>
class Likelihood
{
private:
    typedef typename Track::Element Element;
    typedef typename Association<Track>::Available_data Available_data;
    typedef Independent_mo_gaussian_process<Zero, Squared_exponential> Gp;

private:
    Gp gp;
    double m_noise_sigma;
    typename Data<Element>::Convert m_convert;
    mutable Imogp_distribution mll;
    mutable Gp_inputs X;
    double noise_prob;
    mutable bool fixed_inputs;
    mutable bool limits_set;
    mutable int low_lim;
    mutable int up_lim;

public:
    /**
     * @brief   Constructor
     *
     * @param noise_sigma   The variance of the noise process
     * @param scale         The scale parameter of the GP.
     */
    Likelihood
    (
        double scale,
        double signal_sigma,
        double noise_sigma,
        const typename Data<Element>::Convert& to_vector
    ) :
        gp(std::vector<Zero>(2, Zero()),
           std::vector<Squared_exponential>(2,
               Squared_exponential(scale, signal_sigma))
           ),
        m_noise_sigma(noise_sigma),
        m_convert(to_vector),
        mll(Imogp_distribution::Mvn_vector(1, MV_normal_distribution(2))),
        fixed_inputs(false),
        limits_set(false)
    {
        double ssd = sqrt(signal_sigma);
        noise_prob = pdf(Normal_distribution(0, ssd), 1.3 * ssd);
    }

    /**
     * @brief   Applies this functor to the given association
     * @returns The log-likelihood of the association
     */
    double operator()(const Association<Track>& w) const;

    /**
     * @brief Returns the likelihood of the noise track.
     */
    double at_noise_point(const Element& pt) const
    {
        return m_convert(pt).get_length() * std::log(noise_prob);
    }

    /**
     * @brief Returns the likelihood of the noise track.
     */
    double at_noise(const Available_data& false_alarms) const;

    /**
     * @brief   Computes the GP log-likelihood of a track.
     */
    double at_track(const Track& track) const;

    /**
     * @brief   Return the noise sigma of this model.
     */
    double get_noise_sigma() const
    {
        return m_noise_sigma;
    }

    /**
     * @brief   Return the smoothness scale of this model.
     */
    const Gp& get_gp() const
    {
        return gp;
    }

    /**
     * @brief   Return the convert function.
     */
    const typename Data<Element>::Convert& get_convert() const
    {
        return m_convert;
    }

    /** @brief  Fixes the inputs for faster likelihood computation. */
    void fix_inputs(const Gp_inputs& ins, size_t dim) const
    {
        X = ins;
        std::vector<double> noise_sigmas(dim, m_noise_sigma);
        mll = gp.get_ml_distribution(X, noise_sigmas);
        fixed_inputs = true;
    }

    /** @brief  Fixes the inputs for faster likelihood computation. */
    void unfix_inputs() const
    {
        fixed_inputs = false;
    }

    /** @brief  Set limits on evaluation. */
    void set_limits(int low, int up) const
    {
        low_lim = low;
        up_lim = up;
        limits_set = true;
    }

    /** @brief  Unser limits on evaluation. */
    void reset_limits() const
    {
        limits_set = false;
    }
};

/*============================================================================*
 *                      MEMBER FUNCTION DEFINITIONS                           *
 *----------------------------------------------------------------------------*/

template<class Track>
double Likelihood<Track>::operator()(const Association<Track>& w) const
{
    double ll = 0.0;

    for(typename Association<Track>::const_iterator track_p = w.begin();
                                                    track_p != w.end();
                                                    track_p++)
    {
        ll += at_track(*track_p);
    }

    ll += at_noise(w.get_available_data());

    return ll;
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

template<class Track>
double Likelihood<Track>::at_noise(const Available_data& false_alarms) const
{
    double ll = 0;
    size_t t = 0;
    for(typename Available_data::const_iterator set_p = false_alarms.begin();
                                                set_p != false_alarms.end();
                                                set_p++)
    {
        if(limits_set && (++t < low_lim || t > up_lim)) continue;

        for(typename std::set<const Element*>::const_iterator
                                                elem_pp = set_p->begin();
                                                elem_pp != set_p->end();
                                                elem_pp++)
        {
            ll += at_noise_point(**elem_pp);

            /*if(t % 2 != 0)
            {
                ll += at_noise_point(**elem_pp);
            }*/
        }
    }

    return ll;
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

template<class Track>
double Likelihood<Track>::at_track(const Track& track) const
{
    bool recompute_inputs = !fixed_inputs || limits_set
                                          || track.size() != X.size();

    typename Track::const_iterator b_pair_p;
    typename Track::const_iterator e_pair_p;
    size_t track_size;

    if(limits_set)
    {
        b_pair_p = track.lower_bound(low_lim);
        e_pair_p = track.upper_bound(up_lim);
        track_size = std::distance(b_pair_p, e_pair_p);

        if(track_size == 0) return 0.0;
    }
    else
    {
        b_pair_p = track.begin();
        e_pair_p = track.end();
        track_size = track.size();
    }

    if(recompute_inputs)
    {
        //X.resize(track_size);
        X.clear();
        X.reserve(2*track_size);
    }

    Mogp_outputs Y;
    Y.reserve(2*track_size);

    int i = 0;
    for(typename Track::const_iterator pair_p = b_pair_p;
                                       pair_p != e_pair_p;
                                       pair_p++, i++)
    {
        int t = pair_p->first;
        //if(t % 2 != 0){}
        Vector avg(m_convert(*pair_p->second).size(), 0.0);
        size_t k = 0;
        while(pair_p != e_pair_p && pair_p->first == t)
        {
            if(recompute_inputs)
            {
                //X[i] = Vector(static_cast<double>(pair_p->first));
                X.push_back(Vector().set(t + k*0.001));
                avg += m_convert(*pair_p->second);
                k++;
                pair_p++;
            }
        }

        avg /= k;

        for(size_t j = 0; j < k; j++)
        {
            //Y[i] = m_convert(*pair_p->second);
            Y.push_back(avg);
        }

        pair_p--;
    }

    if(recompute_inputs)
    {
        std::vector<double> noise_sigmas(Y[0].get_length(), m_noise_sigma);
        mll = gp.get_ml_distribution(X, noise_sigmas);
    }

    return log_pdf(mll, Y);
}

}} //namespace kjb::mcmcda

#endif /*MCMCDA_LIKELIHOOD_H_INCLUDED */

