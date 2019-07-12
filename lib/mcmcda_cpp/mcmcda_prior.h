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

#ifndef MCMCDA_PRIOR_H_INCLUDED
#define MCMCDA_PRIOR_H_INCLUDED

#include <mcmcda_cpp/mcmcda_association.h>
#include <prob_cpp/prob_distribution.h>
#include <prob_cpp/prob_sample.h>
#include <cmath>
#include <vector>
#include <utility>

namespace kjb {
namespace mcmcda {

/**
 * @brief %Computes the prior of an MCMCDA association.
 *
 * This functor computes the prior of an association.
 */
template<class Track>
class Prior
{
private:
    double kappa_;
    double theta_;
    double lambda_N_;
    double lambda_A_;
    double lambda_O_;

public:
    /**
     * @brief   Constructs a MCMCDA prior functor
     *
     * @param   kappa       Expected number of tracks when video starts.
     * @param   theta       Average track length.
     * @param   lambda_N    Average number of false alarms per frame.
     * @param   lambda_A    Average number of detections per track per frame.
     * @param   lambda_O    Average number of detections per occluded track per
     *                      frame.
     */
    Prior(double kappa, double theta, double lambda_N, double lambda_A) :
        kappa_(kappa),
        theta_(theta),
        lambda_N_(lambda_N),
        lambda_A_(lambda_A),
        lambda_O_(0.1)
    {}

    /**
     * @brief   Apply this functor to an association.
     *
     * @param   w   The association in question.
     * @returns The log-prior probability of the given association
     */
    double operator()(const Association<Track>& w) const;

    /* ==================================================== *
     *                      GETTERS                         *
     * -----------------------------------------------------*/

    /** @brief  Get average number of before start of video. */
    double kappa() const { return kappa_; }

    /** @brief  Get average track length. */
    double theta() const { return theta_; }

    /** @brief  Get false detection rate. */
    double lambda_N() const { return lambda_N_; }

    /** @brief  Get detection rate. */
    double lambda_A() const { return lambda_A_; }

    /** @brief  Get occluded detection rate. */
    double lambda_O() const { return lambda_O_; }

    /* ==================================================== *
     *                      SETTERS                         *
     * -----------------------------------------------------*/

    /** @brief  Set average number of tracks per time. */
    void set_kappa(double kappa) { kappa_ = kappa; }

    /** @brief  Set average track length. */
    void set_theta(double theta) { theta_ = theta; }

    /** @brief  Set false detection rate. */
    void set_lambda_N(double lambda_N) { lambda_N_ = lambda_N; }

    /** @brief  Set detection rate. */
    void set_lambda_A(double lambda_A) { lambda_A_ = lambda_A; }

    /** @brief  Set occluded detection rate. */
    void set_lambda_O(double lambda_O) { lambda_O_ = lambda_O; }
};

/*============================================================================*
 *                      MEMBER FUNCTION DEFINITIONS                           *
 *----------------------------------------------------------------------------*/

template<class Track>
double Prior<Track>::operator()(const Association<Track>& w) const
{
    using std::log;

    size_t T = w.get_data().size();
    size_t m;
    size_t e;
    size_t d;
    size_t a;
    size_t n;
    size_t l;

    get_association_totals(w, m, e, d, a, n, l);

    double N_t_fac = 0.0;
    double e_t_fac = 0.0;
    double n_t_fac = 0.0;
    double a_it_fac = 0.0;
    for(size_t t = 1; t <= T; t++)
    {
        size_t N_t = w.get_data()[t - 1].size();
        size_t e_t = 0;
        size_t a_t = 0;
        BOOST_FOREACH(const Track& track, w)
        {
            if(track.get_start_time() == t) e_t++;

            size_t a_it = track.count(t);
            a_it_fac += lgamma(a_it + 1);

            a_t += a_it;
        }

        size_t n_t = N_t - a_t;

        N_t_fac += lgamma(N_t + 1);
        e_t_fac += lgamma(e_t + 1);
        n_t_fac += lgamma(n_t + 1);
    }

    // numerator
    double p = m * (log(kappa_) - lambda_A_);
    p += (e + d) * log(theta_);
    p += n * log(lambda_N_);
    p += a * log(lambda_A_);
    p += -(kappa_ + (T - 1)*kappa_*theta_ + l*theta_ + T*lambda_N_);

    // denominator
    p -= N_t_fac;
    p -= e_t_fac;
    p -= n_t_fac;
    p -= a_it_fac;

    return p;
}

/*============================================================================*
 *                              NON-MEMBER FUNCTION                           *
 *----------------------------------------------------------------------------*/

/**
 * @brief   Sample an association from the prior.
 *
 * @param   prior   The prior from which to sample.
 * @param   T       Length of the video.
 * @param   def     A dummy track. Needed in case track has no default
 *                  constructor.
 *
 * @returns A vector of tracks a vector specifying the number of false
 *          alarms in each frame. This function cannot return an object
 *          of type Association<Track>, since it has no access to the
 *          data.
 */
template<class Track>
std::pair<std::vector<Track>, std::vector<size_t> > sample
(
    const Prior<Track>& prior,
    size_t T,
    const Track& def
)
{
    const typename Track::Element* null = 0;
    IFT(T != 0, Illegal_argument, "Cannot sample from prior; T = 0.");

    Poisson_distribution P_e1(prior.kappa());
    Poisson_distribution P_e(prior.kappa() * prior.theta());
    Poisson_distribution P_l(prior.theta());
    Poisson_distribution P_a(prior.lambda_A());
    Poisson_distribution P_N(prior.lambda_N());

    std::vector<Track> tracks;
    std::vector<size_t> false_alarms(T);
    for(size_t t = 1; t <= T; t++)
    {
        // new tracks
        size_t e_t = t == 1 ? kjb::sample(P_e1) : kjb::sample(P_e);
        for(size_t r = 1; r <= e_t; r++)
        {
            // get track length
            size_t l_r = kjb::sample(P_l);

            // create track
            Track track = def;
            track.insert(std::make_pair(t, null));
            track.insert(std::make_pair(std::min(t + l_r, T), null));

            // add it to list of tracks
            tracks.push_back(track);
        }

        // add detections
        BOOST_FOREACH(Track& track, tracks)
        {
            size_t a_rt = kjb::sample(P_a);

            // all tracks already have 1 detection in first and last frames
            if(track.get_start_time() == t || track.get_end_time() == t)
            {
                if(a_rt != 0) a_rt--;
            }

            for(size_t i = 1; i <= a_rt; i++)
            {
                track.insert(std::make_pair(t, null));
            }
        }

        // false alarms
        size_t n_t = kjb::sample(P_N);
        false_alarms[t - 1] = n_t;
    }

    return std::make_pair(tracks, false_alarms);
}

/**
 * @brief   Sample an association from the prior.
 *
 * @param   prior   The prior from which to sample.
 * @param   T       Length of the video.
 *
 * @returns An association and a vector specifying the number of false
 *          alarms in each frame.
 */
template<class Track>
inline
std::pair<std::vector<Track>, std::vector<size_t> > sample
(
    const Prior<Track>& prior,
    size_t T
)
{
    Track def_track;
    return sample(prior, T, def_track);
}

/*============================================================================*
 *                         OLD PRIORS (KEPT JUST IN CASE)                     *
 *----------------------------------------------------------------------------*/

/*template<class Track>
double Prior<Track>::operator()(const Association<Track>& w) const
{
    int T = w.get_data().size();
    int M = w.size();
    int D = 0;
    int B = 0;
    int L = 0;
    int N = 0;
    int m_1;
    int n_1;
    double nblsum = 0.0;
    double outf = 0.0;
    for(int t = 1; t <= T; t++)
    {
        int b_t = 0;
        BOOST_FOREACH(const Track& track, w)
        {
            if(t != T)
            {
                if(track.get_end_time() == t)
                {
                    D++;
                }
            }

            if(track.get_start_time() == t)
            {
                b_t++;
            }

            if(track.get_start_time() <= t && track.get_end_time() >= t)
            {
                L++;
                double v_it = 0.9;
                int a_it = track.count(t);
                int fa_it = tgamma(a_it + 1);
                double outf_ti 
                    = ((exp(-m_lambda_A)*pow(m_lambda_A, a_it)*v_it)
                        + (exp(-m_lambda_O)*pow(m_lambda_O, a_it)*(1.0 - v_it)))
                                                                        / fa_it;

                outf += log(outf_ti);
            }
        }

        int m_t = w.count_live_points_at_time(t);
        if(t == 1)
        {
            m_1 = m_t;
        }

        int n_t = w.get_data()[t - 1].size() - m_t;
        N += n_t;
        if(t == 1)
        {
            n_1 = n_t;
        }

        if(t >= 2)
        {
            B += b_t;
            nblsum += lgamma(n_t + 1) + lgamma(b_t + 1);
        }
    }

    double num = M*log(m_mu) + (B + D)*log(m_tau) + N*log(m_lambda_N)
                    + -(m_mu + (T - 1)*m_mu*m_tau + L*m_tau + T*m_lambda_N);

    double den = lgamma(m_1 + 1) + lgamma(n_1 + 1) + nblsum;

    const double f = 4.0;
    double reward = 0.0;
    if(M > 0)
    {
        reward = ((double)L/M) * std::log(f);
    }
    
    return num - den + outf + reward;
}*/

/*template<class Track>
double Prior<Track>::operator()(const Association<Track>& w) const
{
    double log_prior = 0.0;
    int e_t_1 = 0;

    for(int t = 1; t <= static_cast<int>(w.get_data().size()); t++)
    {
        int z_t = 0;
        int a_t = 0;
        for(typename Association<Track>::const_iterator track_p = w.begin();
                                                        track_p != w.end();
                                                        track_p++)
        {
            if(track_p->get_end_time() == t)
            {
                z_t++;
            }

            if(track_p->get_start_time() == t)
            {
                a_t++;
            }
        }
        int c_t = std::max(0, e_t_1 - z_t);
        int d_t = w.count_live_points_at_time(t);
        int g_t = c_t + a_t - d_t;
        int f_t = w.get_data()[t - 1].size() - d_t;

        using std::log;
        log_prior += ((z_t * log(m_p_z)) + (c_t * log(1 - m_p_z))
                    + (d_t * log(m_p_d)) + (g_t * log(1 - m_p_d))
                    + (a_t * log(m_lambda_b)) + (f_t * log(m_lambda_f))
                    - lgamma(a_t + 1) - lgamma(f_t + 1));

        e_t_1 = d_t;
    }

    return log_prior;
}*/


}} // namespace kjb::mcmcda

#endif /*MCMCDA_PRIOR_H_INCLUDED */

