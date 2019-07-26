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

#ifndef MCMCDA_TRACK_H_INCLUDED
#define MCMCDA_TRACK_H_INCLUDED

#include <mcmcda_cpp/mcmcda_data.h>
#include <map>
#include <l_cpp/l_exception.h>
#include <m_cpp/m_vector.h>
#include <prob_cpp/prob_distribution.h>
#include <prob_cpp/prob_sample.h>

namespace kjb {
namespace mcmcda {

/**
 * @brief   A class that represents a generic MCMCDA track.
 *
 * This class represents a track, in the MCMCDA sense. Specifically, it
 * is simply a map of ints to Elements. It is generic because "Track" is
 * a concept, and this is the simplest implementation of it.
 */
template<class E>
class Generic_track : private std::multimap<int, const E*>
{
public:
    typedef E Element;

private:
    typedef std::multimap<int, const Element*> Parent;
    mutable int chg_start_;
    mutable int chg_end_;

public:
    /**
     * @brief   Constructor
     */
    Generic_track() : chg_start_(-1), chg_end_(-1) {}

    /**
     * @brief   Range constructor
     */
    template<class Iterator>
    Generic_track(Iterator first, Iterator last) :
        Parent(first, last),
        chg_start_(-1),
        chg_end_(-1) {}

    // pass-throughs
    using Parent::empty;
    using Parent::begin;
    using Parent::end;
    using Parent::rbegin;
    using Parent::rend;
    using Parent::size;
    using Parent::count;
    using Parent::insert;
    using Parent::clear;
    using Parent::erase;
    using Parent::find;
    using Parent::equal_range;
    using Parent::lower_bound;
    using Parent::upper_bound;
    typedef typename Parent::iterator iterator;
    typedef typename Parent::const_iterator const_iterator;
    typedef typename Parent::reverse_iterator reverse_iterator;
    typedef typename Parent::const_reverse_iterator const_reverse_iterator;
    typedef typename Parent::value_type value_type;
    typedef typename Parent::key_type key_type;
    typedef typename Parent::mapped_type mapped_type;
    typedef typename Parent::reference reference;
    typedef typename Parent::const_reference const_reference;

    /**
     * @brief   Gets start time of track.
     */
    int get_start_time() const
    {
        IFT(!empty(), Index_out_of_bounds, "get_start_time: track empty.");
        return begin()->first;
    }

    /**
     * @brief   Gets end time of track.
     */
    int get_end_time() const
    {
        IFT(!empty(), Index_out_of_bounds, "get_end_time: track empty.");
        return rbegin()->first;
    }

    /**
     * @brief   Gets nth time of track
     */
    int get_nth_time(int n) const
    {
        size_t rsz = real_size();
        IFTD(!empty() && rsz > static_cast<size_t>(n) && n > 0,
             Index_out_of_bounds,
             "get_nth_time: cannot get %dth time, track has length %d.",
             (n)(rsz));

        const_iterator cur = begin();
        int k = 0;
        int t = 0;
        while(k < n)
        {
            if(cur->first != t)
            {
                k++;
                t = cur->first;
            }

            cur++;
        }

        return t;
    }

    /**
     * @brief   Gets a random time from track
     */
    int get_random_time() const
    {
        IFT(!empty(), Index_out_of_bounds, "get_random_time: track empty.");

        return get_nth_time(sample(
            Categorical_distribution<int>(1, real_size(), 1)));
    }

    /**
     * @brief   Given a time, gets next time
     */
    int get_next_time(int t) const
    {
        const_iterator ub = this->upper_bound(t);
        IFTD(ub != end(), Index_out_of_bounds,
             "get_next_time: track has no after points time %d.", (t));

        return ub->first;
    }

    /**
     * @brief   Given a time, gets previous time
     */
    int get_previous_time(int t) const
    {
        IFT(!empty(), Index_out_of_bounds, "get_previous_time: track is empty.");

        const_iterator ub = this->lower_bound(t);
        IFTD(ub == begin(), Index_out_of_bounds,
             "get_next_time: track has no before points time %d.", (t));
        ub--;

        return ub->first;
    }

    /**
     * @brief   Gets start point of track.
     */
    const Element& get_start_point() const
    {
        IFT(!empty(), Index_out_of_bounds, "get_start_point: track is empty.");
        return *(begin()->second);
    }

    /**
     * @brief   Gets end point of track.
     */
    const Element& get_end_point() const
    {
        IFT(!empty(), Index_out_of_bounds, "get_end_point: track is empty.");
        return *(rbegin()->second);
    }

    /** @brief  Get "real" size; i.e., number of frames with detections. */
    size_t real_size() const;

    /** @brief  Returns changed start flag. */
    int changed_start() const
    {
        return chg_start_;
    }

    /** @brief  Returns changed end flag. */
    int changed_end() const
    {
        return chg_end_;
    }

    /** @brief  Set the changed flag of this track. */
    void set_changed_start(int f) const
    {
        chg_start_ = f;
    }

    /** @brief  Set the changed flag of this track. */
    void set_changed_end(int f) const
    {
        chg_end_ = f;
    }

    /** @brief  Set changed flags to unchanged. */
    void set_changed_all() const
    {
        set_changed_start(get_start_time());
        set_changed_end(get_end_time());
    }

    /**
     * @brief   Determines whether this track is valid
     */
    bool is_valid
    (
        double v_bar,
        int d_bar,
        double sg,
        const typename Data<Element>::Convert& to_vector
    ) const;

    template<class Elem>
    friend bool operator<
    (
        const Generic_track<Elem>& t1,
        const Generic_track<Elem>& t2
    );

    template<class Elem>
    friend bool operator==
    (
        const Generic_track<Elem>& t1,
        const Generic_track<Elem>& t2
    );

    template<class Elem>
    friend bool operator!=
    (
        const Generic_track<Elem>& t1,
        const Generic_track<Elem>& t2
    );

    friend void swap(Generic_track<Element>& t1, Generic_track<Element>& t2)
    {
        using std::swap;
        swap(t1.chg_start_, t2.chg_start_);

        Parent& m1 = t1;
        Parent& m2 = t2;
        swap(m1, m2);
    }
};

/**
 * @brief   Mergest two tracks
 */
/*template<class Element>
inline Generic_track<Element> merge_tracks
(
    const Generic_track<Element>& track1,
    const Generic_track<Element>& track2,
    int t_f,
    int t_1
)
{
    Generic_track<Element> result;
    typename Generic_track<Element>::const_iterator it;

    it = track1.find(t_f);
    IFTD(it != track1.end(), KJB_error,
         "merge_tracks: track1 has no point at %d.", (t_f));
    result.insert(track1.begin(), it);

    it = track2.find(t_1);
    IFTD(it != track2.end(), KJB_error,
         "merge_tracks: track2 has no point at %d.", (t_1));
    result.insert(it, track2.end());

    return result;
}*/

/**
 * @brief   Swaps two tracks from a certain time
 */
template<class Element>
void swap_tracks
(
    Generic_track<Element>& track1,
    Generic_track<Element>& track2,
    int t1,
    int tp1,
    int t2,
    int tq1
)
{
    Generic_track<Element> temp_track = track1;
    typename Generic_track<Element>::iterator it;

    it = track1.upper_bound(t1);
    IFTD(it != track1.end(), KJB_error,
         "swap_tracks: track1 has no point after time %d.", (t1));
    track1.erase(it, track1.end());

    it = track2.find(tq1);
    IFTD(it != track2.end(), KJB_error,
         "swap_tracks: track2 has no point at time %d.", (tq1));
    track1.insert(it, track2.end());

    it = track2.upper_bound(t2);
    IFTD(it != track2.end(), KJB_error,
         "swap_tracks: track2 has no point after time %d.", (t2));
    track2.erase(it, track2.end());

    it = temp_track.find(tp1);
    IFTD(it != temp_track.end(), KJB_error,
         "swap_tracks: track1 has no point at time %d.", (tp1));
    track2.insert(it, temp_track.end());
}

/*============================================================================*
 *                      MEMBER FUNCTION DEFINITIONS                           *
 *----------------------------------------------------------------------------*/

template<class Element>
size_t Generic_track<Element>::real_size() const
{
    size_t sz = 0;
    int t = 0;
    for(const_iterator pp = begin(); pp != end(); pp++)
    {
        if(pp->first != t)
        {
            sz++;
            t = pp->first;
        }
    }

    return sz;
}

template<class Element>
bool Generic_track<Element>::is_valid
(
    double v_bar,
    int d_bar,
    double sg,
    const typename Data<Element>::Convert& to_vector
) const
        
{
    if(real_size() < 2)
    {
        return false;
    }

    typename Generic_track<Element>::const_iterator pair_p2 = begin();
    for(typename Generic_track<Element>::const_iterator pair_p1 = pair_p2++;
                                                        pair_p2 != end();
                                                        pair_p1 = pair_p2++)
    {
        int d = pair_p2->first - pair_p1->first;
        if(std::abs(d) > d_bar)
        {
            return false;
        }

        /*if(!is_neighbor(*pair_p1->second, *pair_p2->second,
                        d, d_bar, v_bar, sg, to_vector))
        {
            return false;
        }*/
    }

    return true;
}

/**
 * @brief   Compare two generic tracks.
 */
template<class E>
inline
bool operator==(const Generic_track<E>& t1, const Generic_track<E>& t2)
{
    const std::multimap<int, const E*>& m1 = t1;
    const std::multimap<int, const E*>& m2 = t2;

    return m1 == m2;
}

/**
 * @brief   Compare two generic tracks.
 */
template<class E>
inline
bool operator!=(const Generic_track<E>& t1, const Generic_track<E>& t2)
{
    const std::multimap<int, const E*>& m1 = t1;
    const std::multimap<int, const E*>& m2 = t2;

    return m1 != m2;
}

/**
 * @brief   Compare two generic tracks.
 */
template<class Element>
inline
bool operator<
(
    const Generic_track<Element>& t1,
    const Generic_track<Element>& t2
)
{
    const std::multimap<int, const Element*>& m1 = t1;
    const std::multimap<int, const Element*>& m2 = t2;
    return m1 < m2;
}

}} //namespace kjb::mcmcda

#endif /*MCMCDA_TRACK_H_INCLUDED */

