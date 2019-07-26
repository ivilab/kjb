#ifndef MCMCDA_ASSOCIATION_H_INCLUDED
#define MCMCDA_ASSOCIATION_H_INCLUDED

#include <mcmcda_cpp/mcmcda_track.h>
#include <mcmcda_cpp/mcmcda_data.h>
#include <l_cpp/l_exception.h>
#include <l_cpp/l_functors.h>
#include <m_cpp/m_vector.h>
#include <set>
#include <algorithm>
#include <iterator>
#include <functional>
#include <string>
#include <fstream>
#include <boost/bind.hpp>
#include <boost/foreach.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/lambda/lambda.hpp>
#include <boost/algorithm/string.hpp>

namespace kjb {
namespace mcmcda {

/** 
 * @brief  Functor to compare two tracks. If this were not defined, an
 *         association would order its tracks using operator<(pair, pair),
 *         which would eventually order by the address of the detection.
 *         We use this one to avoid this comparison by address and instead
 *         use a value comparison. This way the order will not change every
 *         time we read an association.
 */
template<class Track>
struct Compare_tracks
{
    /** @brief  Compare two tracks. */
    bool operator()(const Track& t1, const Track& t2) const
    {
        return std::lexicographical_compare(t1.begin(), t1.end(),
            t2.begin(), t2.end(), boost::bind(
                &Compare_tracks<Track>::compare_elements, this, _1, _2));
    }

    /** @brief  Compare two track elements. */
    bool compare_elements
    (
        const typename Track::value_type& p1,
        const typename Track::value_type& p2
    ) const
    {
        if(p1.first < p2.first) return true;
        if(p2.first < p1.first) return false;
        return *p1.second < *p2.second;
    }
};

/**
 * @brief   A class that represents a MCMCDA association.
 *
 * This class represents an association, in the MCMCDA sense. Specifically, it
 * is simply a set of tracks and a pointer to a tracking data set.
 *
 * @see     Tracking_data
 * @see     Track
 */
template<class Track>
class Association : private std::set<Track, Compare_tracks<Track> >
{
public:
    typedef typename Track::Element Element;

private:
    typedef std::set<Track, Compare_tracks<Track> > Parent;
    typedef std::set<const Element*> Elementp_set;

private:
    const Data<Element>* Y;

public:
    typedef std::vector<Elementp_set> Available_data;

    /**
     * @brief   Default constructor -- meaningless association. Use
     *          with care!!
     */
    Association() : Y(0)
    {}

    /**
     * @brief   Constructor
     */
    Association(const Data<Element>& data) : Y(&data)
    {}

    /**
     * @brief   Constructor from sequence and data
     *
     * Copies the track set from iterator sequence and the pointer to the data
     * from given data reference.
     */
    template<class Iterator>
    Association
    (
        const Data<Element>& data,
        Iterator first,
        Iterator last
    ) :
        Parent(first, last), Y(&data)
    {}

    // pass-throughs
    using Parent::empty;
    using Parent::begin;
    using Parent::end;
    using Parent::rbegin;
    using Parent::rend;
    using Parent::size;
    using Parent::insert;
    using Parent::clear;
    using Parent::erase;
    using Parent::find;
    typedef typename Parent::iterator iterator;
    typedef typename Parent::const_iterator const_iterator;
    typedef typename Parent::const_reference const_reference;

    /** @brief   Returns data set const-ref. */
    const Data<Element>& get_data() const
    {
        return *Y;
    }

    /** @brief   Set data. */
    void set_data(const Data<Element>& data)
    {
        Y = &data;
    }

    /**
     * @brief   Computes "available data".
     *
     * The available data is the set of points that have do not belong to
     * any track.
     */
    Available_data get_available_data() const
    {
        using namespace std;

        Available_data a_data(Y->size());
        vector<int> time(Y->size());
        generate(time.begin(), time.end(), Increment<int>(1));
        transform(time.begin(), time.end(), a_data.begin(), boost::bind(
                &Association<Track>::get_dead_points_at_time, this, _1));

        return a_data;
    }

    /**
     * @brief   Computes points at time t that belong to some track.
     */
    Elementp_set get_live_points_at_time(int t) const;

    /**
     * @brief   Counts points at time t that belong to some track. This is
     *          usually faster than computing the points and getting size().
     */
    int count_live_points_at_time(int t) const;

    /**
     * @brief   Computes points at time t that do not belong to any track.
     */
    Elementp_set get_dead_points_at_time(int t) const
    {
        using namespace std;

        Elementp_set dead_data_t;
        Elementp_set live_data_t = get_live_points_at_time(t);
        Elementp_set data_t_addrs;
        transform((*Y)[t - 1].begin(), (*Y)[t - 1].end(),
            inserter(data_t_addrs, data_t_addrs.begin()), &boost::lambda::_1);

        set_difference(data_t_addrs.begin(), data_t_addrs.end(),
                       live_data_t.begin(), live_data_t.end(),
                       inserter(dead_data_t, dead_data_t.begin()));

        return dead_data_t;
    }

    /**
     * @brief   Counts points at time t that do not belong to any track. This is
     *          usually faster than computing the points and getting size().
     */
    int count_dead_points_at_time(int t) const
    {
        int cdt = (*Y)[t - 1].size();
        int clp = count_live_points_at_time(t);

        return cdt - clp;
    }

    /**
     * @brief   Computes points at time t that are valid starting points
    *           for a track.
     */
    Elementp_set valid_starting_points
    (
        int t,
        int d,
        int d_bar,
        double v_bar,
        double sg,
        const typename Data<Element>::Convert& to_vector
    ) const
    {
        IFT(t > 0 && d > 0, Illegal_argument,
            "valid_starting_points: t and d must be positive");

        IFTD(d <= d_bar, Illegal_argument,
             "valid_starting_points: d (%d) cannot be greater than d^bar (%d)",
             (d)(d_bar));

        using namespace std;
        Elementp_set dead_pts_t = get_dead_points_at_time(t);
        Elementp_set vsp;

        //remove_copy_if(dead_pts_t.begin(), dead_pts_t.end(),
        //    inserter(vsp, vsp.begin()), !boost::bind(
        //        &Data<Element>::neighborhood_size, Y, _1,
        //        t, d, d_bar, v_bar, sg, to_vector));

        BOOST_FOREACH(const Element* elem_p, dead_pts_t)
        {
            if(Y->neighborhood_size(*elem_p, t, d, d_bar,
                                    v_bar, sg, to_vector) != 0)
            {
                vsp.insert(elem_p);
            }
        }

        return vsp;
    }

    /**
     * @brief   Counts points at time t that are valid starting track points.
     *          This is usually faster than finding the points and getting
     *          size().
     */
    int count_valid_starting_points
    (
        int t,
        int d,
        int d_bar,
        double v_bar,
        double sg,
        const typename Data<Element>::Convert& to_vector
    ) const
    {
        IFT(t > 0 && d > 0, Illegal_argument,
            "valid_starting_points: t and d must be positive");

        IFTD(d <= d_bar, Illegal_argument,
             "valid_starting_points: d (%d) cannot be greater than d^bar (%d)",
             (d)(d_bar));

        Elementp_set dead_pts_t = get_dead_points_at_time(t);
        //int bad_pts = std::count_if(dead_pts_t.begin(), dead_pts_t.end(),
        //    !boost::bind(&Data<Element>::neighborhood_size, Y, _1,
        //                 t, d, d_bar, v_bar, sg, to_vector));

        int num_valid = 0;
        BOOST_FOREACH(const Element* elem_p, dead_pts_t)
        {
            if(Y->neighborhood_size(*elem_p, t, d, d_bar,
                                    v_bar, sg, to_vector) != 0)
            {
                num_valid++;
            }
        }


        return num_valid;
    }

    /**
     * @brief   Computes neighbors of vector that are available.
     */
    Elementp_set dead_neighbors
    (
        const Element& y,
        int t,
        int d,
        int d_bar,
        double v_bar,
        double sg,
        const typename Data<Element>::Convert& to_vector
    ) const
    {
        Elementp_set d_neighbors;
        Elementp_set neighbors = Y->neighborhood(y, t, d, d_bar,
                                                 v_bar, sg, to_vector);

        if(!neighbors.empty())
        {
            Elementp_set live_points = get_live_points_at_time(t + d);
            std::set_difference(neighbors.begin(), neighbors.end(),
                                live_points.begin(), live_points.end(),
                                std::inserter(d_neighbors, d_neighbors.begin()));
        }

        return d_neighbors;
    }

    /**
     * @brief   Computes neighbors of vector that are available. This is usually
     *          faster than computing the points and getting size().
     */
    int count_dead_neighbors
    (
        const Element& y,
        int t,
        int d,
        int d_bar,
        double v_bar,
        double sg,
        const typename Data<Element>::Convert& to_vector
    ) const
    {
        return dead_neighbors(y, t, d, d_bar, v_bar, sg, to_vector).size();
    }

    /**
     * @brief   Read association from file.
     */
    void read(const std::string& filename, const Track& def = Track());

    /**
     * @brief   Writes association to file.
     */
    void write(const std::string& filename) const;

    /**
     * @brief   Determines whether this association is valid
     */
    bool is_valid
    (
        double v_bar,
        int d_bar,
        double sg,
        const typename Data<Element>::Convert& convert
    ) const;

    /**
     * @brief   Efficiently swap two associations.
     */
    friend
    void swap(Association& a1, Association& a2)
    {
        using std::swap;

        swap(a1.Y, a2.Y);
        std::set<Track, Compare_tracks<Track> >& s1 = a1;
        std::set<Track, Compare_tracks<Track> >& s2 = a2;
        swap(s1, s2);
    }
};

/*============================================================================*
 *                      MEMBER FUNCTION DEFINITIONS                           *
 *----------------------------------------------------------------------------*/

template<class Track>
typename Association<Track>::Elementp_set
    Association<Track>::get_live_points_at_time(int t) const
{
    Elementp_set lp;

    for(const_iterator p = begin(); p != end(); p++)
    {
        typedef typename Track::const_iterator Tr_it;
        std::pair<Tr_it, Tr_it> erp = p->equal_range(t);
        for(Tr_it q = erp.first; q != erp.second; q++)
        {
            lp.insert(q->second);
        }
    }

    return lp;
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

template<class Track>
int Association<Track>::count_live_points_at_time(int t) const
{
    int count = 0;

    for(const_iterator p = begin(); p != end(); p++)
    {
        typename Track::const_iterator q = p->find(t);
        if(q != p->end())
        {
            count++;
        }
    }

    return count;
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

template<class Track>
void Association<Track>::read(const std::string& filename, const Track& def)
{
    using namespace std;

    this->clear();

    ifstream ifs(filename.c_str());
    IFTD(ifs, IO_error, "Cannot read association; could not open file %s",
         (filename.c_str()));

    size_t T = Y->size();
    string line;
    while(getline(ifs, line))
    {
        if(line.empty() || line[0] == '#')
        {
            continue;
        }

        istringstream istr(line);
        vector<string> strs;
        copy(istream_iterator<string>(istr),
             istream_iterator<string>(),
             back_inserter(strs));

        Track track = def;
        IFTD(strs.size() == T, Illegal_argument,
             "Cannot read association; file %s has incorrect format.",
             (filename.c_str()));

        for(int t = 0; t < strs.size(); t++)
        {
            vector<string> idxs;
            boost::split(idxs, strs[t], boost::is_any_of(","));

            for(size_t i = 0; i < idxs.size(); i++)
            {
                int idx = boost::lexical_cast<int>(idxs[i]);
                IFTD(
                    idx >= -1 && idx < static_cast<int>((*Y)[t].size()),
                    Illegal_argument,
                    "Cannot read association; "
                        "file %s has bad format (frame %d, idx %d).",
                    (filename.c_str())(t+1)(idx));

                if(i == 0)
                {
                    if(idx == -1)
                    {
                        continue;
                    }
                }
                else
                {
                    IFTD(idx != -1, Illegal_argument,
                         "Cannot read association; file %s has bad format.",
                         (filename.c_str()));
                }

                typename std::set<Element>::const_iterator vec_p
                                                        = (*Y)[t].begin();
                advance(vec_p, idx);
                track.insert(make_pair(t + 1, &(*vec_p)));
            }
        }

        this->insert(track);
    }

    ifs.close();
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

template<class Track>
void Association<Track>::write(const std::string& filename) const
{
    std::ofstream ofs(filename.c_str());
    if(!ofs)
    {
        KJB_THROW_3(IO_error,
                    "Cannot write association; could not open file %s",
                    (filename.c_str()));
    }

    typedef typename Track::const_iterator Tr_it;
    typedef std::set<Element> E_set;
    BOOST_FOREACH(const Track& track, *this)
    {
        for(Tr_it pair_p = track.begin(); pair_p != track.end(); pair_p++)
        {
            int t = pair_p->first;
            const Element& elem = *pair_p->second;

            if(pair_p == track.begin())
            {
                for(int s = 1; s < t; s++)
                {
                    ofs << "-1  ";
                }
            }

            const E_set& data_t = (*Y)[t - 1];
            typename E_set::const_iterator elem_p = data_t.find(elem);

            ofs << std::distance(data_t.begin(), elem_p);

            Tr_it pair_q = pair_p;
            pair_q++;
            if(pair_q == track.end())
            {
                for(int s = t + 1; s <= Y->size(); s++)
                {
                    ofs << "  -1";
                }
            }
            else
            {
                if(pair_q->first == t)
                {
                    ofs << ",";
                }
                else
                {
                    ofs << "  ";
                    for(int s = t + 1; s < pair_q->first; s++)
                    {
                        ofs << "-1  ";
                    }
                }
            }
        }

        ofs << std::endl;
    }

    ofs.close();
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

template<class Track>
bool Association<Track>::is_valid
(
    double v_bar,
    int d_bar,
    double sg,
    const typename Data<Element>::Convert& convert
) const
{
    // check whether distances between points are OK.
    const_iterator invalid_track_p = std::find_if(begin(), end(),
        !boost::bind(&Track::is_valid, _1, v_bar, d_bar, sg, convert));

    if(invalid_track_p != end())
    {
        return false;
    }

    // checks data is not repeated, etc.
    Available_data used_data(Y->size());
    for(const_iterator track_p = begin(); track_p != end(); track_p++)
    {
        for(typename Track::const_iterator pair_p = track_p->begin();
                                           pair_p != track_p->end();
                                           pair_p++)
        {
            if(pair_p->second == NULL)
            {
                return false;
            }

            // check to see if this points to something in data
            const std::set<Element>& data_t = (*Y)[pair_p->first - 1];
            if(std::count_if(data_t.begin(), data_t.end(),
                Compare_address<Element>(pair_p->second)) == 0)
            {
                return false;
            }

            // insert point and check whether or not already used :-)
            if(!used_data[pair_p->first - 1].insert(pair_p->second).second)
            {
                return false;
            }
        }
    }

    return true;
}

/**
 * @brief   Get total tracks, entrances, exits, true detections,
 *          noisy detections, and track lengths of a given association.
 */
template<class Track>
void get_association_totals
(
    const Association<Track>& w,
    size_t& m,
    size_t& e,
    size_t& d,
    size_t& a,
    size_t& n,
    size_t& l
)
{
    m = w.size();

    e = d = a = l = 0;

    const size_t T = w.get_data().size();
    BOOST_FOREACH(const Track& track, w)
    {
        int sf = track.get_start_time();
        int ef = track.get_end_time();
        assert(sf != -1 && ef != -1);

        if(sf != 1) e++;

        if(ef != T) d++;

        a += track.size();
        l += ef - sf + 1;
    }

    size_t N = 0;
    for(size_t t = 1; t <= T; t++)
    {
        N += w.get_data()[t - 1].size();
    }

    n = N - a;
}

}} //namespace kjb::mcmcda

#endif /*MCMCDA_ASSOCIATION_H_INCLUDED */

