/**
 * @file
 * @author Andrew Predoehl
 * @brief Implementation for intersection computation
 *
 * This is basically an implementation of the Bentley-Ottman
 * line segment intersection algorithm using data structures
 * now living in sweep.cpp.   See the introduction there.
 */
/*
 * $Id: intersection.cpp 21596 2017-07-30 23:33:36Z kobus $
 */

#include "l/l_sys_debug.h"  /* For ASSERT */
#include "l/l_error.h"
#include "l_cpp/l_exception.h"
#include "qd_cpp/intersection.h"
#include "qd_cpp/sweep.h"

#define INTERSECTION_CPP_DEBUG 0 /* 0=off, 1=active */

#if INTERSECTION_CPP_DEBUG
#include "i_cpp/i_image.h"
#include <iostream>
#endif


#include <set>
#include <algorithm>
#include <iterator>
#include <functional>

namespace
{

using kjb::qd::RatPoint;
using kjb::qd::RatPoint_line_segment;
using kjb::qd::sweep::SweepLine;
using kjb::qd::sweep::Static_segment_map;


#if INTERSECTION_CPP_DEBUG
void draw_line_seg(
    kjb::Image* i,
    int MAG,
    int RBIAS,
    int CBIAS,
    int width,
    const RatPoint_line_segment& s,
    const kjb::PixelRGBA& color
)
{
    using kjb::qd::dbl_ratio;
    i -> draw_line_segment(
            RBIAS+MAG*dbl_ratio(s.a.y), CBIAS+MAG*dbl_ratio(s.a.x),
            RBIAS+MAG*dbl_ratio(s.b.y), CBIAS+MAG*dbl_ratio(s.b.x),
            width, color);
}



void debug_sweep_viz(
    typename SweepLine<Static_segment_map>::SL const& sweep,
    const std::vector<RatPoint_line_segment>& segs,
    const RatPoint& sweep_location,
    const RatPoint_line_segment& lbump,
    const RatPoint_line_segment& rbump
)
{
    using kjb::qd::dbl_ratio;

    static int count = 10000;
    std::ostringstream fn;
    fn << "sweep_dump_";
    if (count < 10) fn << '0';
    fn << count++ << ".png";
    const int SIZE=500;
    kjb::Image i(SIZE, SIZE, 0,0,100);

    KJB(ASSERT(lbump.b.y > lbump.a.y));
    KJB(ASSERT(rbump.a.x > lbump.a.x));
    const RatPoint::Rat
        l = std::max(lbump.b.y - lbump.a.y, rbump.a.x - lbump.a.x),
        dcy = lbump.b.y + lbump.a.y, // double centroid y
        dcx = rbump.a.x + lbump.a.x;
    const int MAG = std::max(1.0, SIZE / dbl_ratio(l) * 0.90),
              RBIAS = (SIZE - MAG * dbl_ratio(dcy))/2,
              CBIAS = (SIZE - MAG * dbl_ratio(dcx))/2;

    // sweep line
    i.draw_point(   RBIAS + MAG * dbl_ratio(sweep_location.y),
                    CBIAS + MAG * dbl_ratio(sweep_location.x),
                    5, kjb::PixelRGBA(200,100,100));
    RatPoint_line_segment h(sweep_location, sweep_location);
    h.a.x = -1;
    h.b.x = 7;
    draw_line_seg(&i, MAG, RBIAS, CBIAS, 1, h, kjb::PixelRGBA(200,50,50));

    // all segments
    for (size_t j = 0; j < segs.size(); ++j)
    {
        const RatPoint_line_segment s = segs[j];
        draw_line_seg(&i, MAG, RBIAS, CBIAS, 1, s, kjb::PixelRGBA(200,200,50));
    }

    // sweep line segments
    for (kjb::qd::sweep::SweepLine<Static_segment_map>::const_iterator j=sweep.begin();
            j!=sweep.end(); ++j)
    {
        if (j -> is_fake()) continue;
        const RatPoint_line_segment s = segs[j -> true_index()];
        draw_line_seg(&i, MAG, RBIAS, CBIAS, 1, s, kjb::PixelRGBA(50,150,50));
    }
    i.write(fn.str());
}
#endif



struct Intersect_indices
{
    std::pair< size_t, size_t > m_indices;

public:
    Intersect_indices(size_t i, size_t j)
    :   m_indices(i < j ? std::make_pair(i, j) : std::make_pair(j, i))
    {
        if (i == j) KJB_THROW(kjb::Illegal_argument);
    }

    operator const std::pair<size_t, size_t>&() const { return m_indices; }
};


inline
bool operator<(const Intersect_indices& pp, const Intersect_indices& qq)
{
    const std::pair<size_t, size_t> &p(pp), &q(qq);
    // parentheses here are not necessary except to quiet the compiler.
    return      p.first < q.first
            ||  (p.first == q.first && p.second < q.second);
}



std::ostream& operator<<(std::ostream& o, const Intersect_indices& ii)
{
    const std::pair<size_t, size_t> &i(ii);
    return o << i.first << ',' << i.second;
}



std::ostream& operator<<(
    std::ostream& o,
    const std::set< Intersect_indices >& i
)
{
    o << "Intersections found:  (";
    std::copy(i.begin(), i.end(),
            std::ostream_iterator<Intersect_indices>(o, "), ("));
    return o << "end of list)\n";
}




std::vector< std::pair<size_t, size_t> > filter(
    const std::vector< std::pair<size_t, size_t> > hits,
    bool ignore_trivia,
    const kjb::qd::PixPath& path
)
{
    std::vector< std::pair<size_t, size_t> > vhits;
    vhits.reserve(hits.size());

    for (size_t j = 0; j < hits.size(); ++j)
    {
        const size_t i1 = hits.at(j).first, i2 = hits.at(j).second;
        KJB(ASSERT(i1 < i2));

        if (ignore_trivia)
        {
            if (1+i1 < i2)
            {
                // obviously nontrivial intersection
                vhits.push_back( std::make_pair(i1, i2) );
            }
            else
            {
                KJB(ASSERT(1+i1 == i2));
                const kjb::qd::PixPoint_line_segment s(path[i1], path[i1+1]),
                                                     t(path[i2], path[i2+1]);
                if (    is_degenerate(s)
                    ||  is_degenerate(t)
                    ||  are_sharing_a_continuum(s,t))
                {
                    // nontrivial intersection, but it wasn't obvious at first.
                    vhits.push_back( std::make_pair(i1, i2) );
                }
                // else it is only a trivial intersection and we ignore those.
            }
        }
        else
        {
            // unfiltered output -- we want all intersections even if trivial.
            vhits.push_back( std::make_pair(i1, i2) );
        }
    }
    return vhits;
}



void report_intersections(
    const std::set<size_t>& ulm_union,
    std::set< Intersect_indices >* hits
)
{
    KJB(ASSERT(hits));
    std::vector<size_t> erbody(ulm_union.begin(), ulm_union.end());

    while (! erbody.empty())
    {
        const size_t s = erbody.back();
        erbody.pop_back();
        for (size_t i = 0; i < erbody.size(); ++i)
        {
            hits -> insert( Intersect_indices(erbody[i], s) );
        }
    }
}


#if 0 /* formerly, ifdef DEBUGGING */
bool is_correct_based_on_brute_force_verification(
    const std::set< Intersect_indices >& boi, // bentley-ottman intersections
    const std::vector< RatPoint_line_segment >& sl
)
{
    using namespace std;

    vector< Intersect_indices > brute, ssd;

    // Brute-force check for edge intersections.
    for (size_t i = 0; i < sl.size()-1; ++i)
    {
        for (size_t j = 1+i; j < sl.size(); ++j)
        {
            if (kjb::qd::is_intersecting(sl[i], sl[j]))
            {
                brute.push_back( Intersect_indices(i, j) );
            }
        }
    }

    set_symmetric_difference(boi.begin(), boi.end(),
                             brute.begin(), brute.end(),
                             back_inserter(ssd));
    if (!ssd.empty())
    {
        kjb_c::set_error("get_intersections() runtime error -- results "
                "disagree with brute-force verification");
    }

    return ssd.empty();
}
#endif

}




namespace kjb
{
namespace qd
{



std::vector< std::pair<size_t, size_t> > get_intersections(
    const PixPath& p,
    bool filter_out_trivial_intersections // optional flag
)
{
    std::vector< RatPoint_line_segment > sl;
    sl.reserve(p.size()-1);

    for (size_t i = 1; i < p.size(); ++i)
    {
        sl.push_back(RatPoint_line_segment(p[i-1], p[i]));
    }

    return filter(get_intersections(sl), filter_out_trivial_intersections, p);
}



std::vector< std::pair<size_t, size_t> > get_intersections(
    const std::vector< RatPoint_line_segment >& sl
)
{
    using namespace sweep;

    if (sl.size() < 2) return std::vector< std::pair<size_t, size_t> >();

    std::set< Intersect_indices > hits;
    std::vector< RatPoint_line_segment > sls;
    std::transform(sl.begin(), sl.end(), std::back_inserter(sls), rectify);

    // Fill event queue and compute the bumpers.
    RatPoint_line_segment lbump(sls.front()), rbump(lbump);
    std::set< Event_p > eq = fill_queue_get_bumpers(sls, &lbump, &rbump);

    /*
     * Set up the sweep status line and a reverse-lookup table of iterators.
     * Add bumpers to the sweep line (via comparison functor)
     */
    RatPoint sweep_location = (* eq.begin()) -> p;
    const Static_segment_map ssm(&sls);
    typedef SweepLine<Static_segment_map>::SL Sweep;
    Sweep sweep(
        SweepCompare< Static_segment_map >(ssm, &sweep_location, lbump, rbump));
    sweep.insert(Shemp(Shemp::LBUMP));
    sweep.insert(Shemp(Shemp::RBUMP));
    std::vector< Sweep::iterator > rvs_sweep(sls.size(), sweep.end());

    while (! eq.empty())
    {
        // Steps below refer to page 26 of de Berg et al.
        sweep_location = (* eq.begin()) -> p;
        std::set<size_t> ulma[4]; // [0]=upper, [1]=lower, [2]=middle, [3]=all
        // steps 1, 2
        pull_events_here(&eq, sweep, ulma, ssm);

#if INTERSECTION_CPP_DEBUG
        std::cout << "Start of event ";
        debug_sweepline_dump(sweep, std::cout);
        debug_sweep_viz(sweep, sls, sweep_location, lbump, rbump);
        debug_event_dump(ulma, std::cout);
        std::cout << "Segments here: " << ulma[3].size() << '\n';
        std::copy(hits.begin(), hits.end(),
             std::ostream_iterator<Intersect_indices>(std::cout, " & "));
#endif

        // steps 3, 4
        if (!ulma[3].empty()) report_intersections(ulma[3], &hits);
        /* steps 5-7:  freshen the sweepline, count how many
         *             nondegenerate segments intersect the event point.
         */
        const size_t count = update_sweep< Static_segment_map >(&sweep, ulma, &rvs_sweep);

#if INTERSECTION_CPP_DEBUG
        std::cout << "\nAfter steps 5-6 ";
        debug_sweep_viz(sweep, sls, sweep_location, lbump, rbump);
        std::cout << "===============\n";
#endif

        // step 8:  find the neighbors of the event point.
        typedef Sweep::const_iterator SLCI;
        const std::pair< SLCI, SLCI >
                eir = get_event_neighbors< Static_segment_map >(sweep, sweep_location, ssm);
        if (0 == count)
        {
            // steps 9-10: no event-point intersectors between the neighbors
            find_new_event(&eq, *eir.first, *eir.second, ssm, sweep_location);
#ifdef DEBUGGING
            SLCI bgin(eir.first);
            KJB(ASSERT(++bgin == eir.second));
#endif
        }
        else
        {
            // steps 11-16: one or more intersector segments at event point.
            SLCI li = eir.first, ri = eir.second; // left, right intersectors
            find_new_event(&eq, *eir.first, *++li, ssm, sweep_location);
            find_new_event(&eq, *--ri, *eir.second, ssm, sweep_location);
            KJB(ASSERT(li != eir.second));
        }
    }
    KJB(ASSERT(2 == sweep.size())); // just the two bumpers

#if 0 /* formerly, ifdef DEBUGGING */

    // The results have been accurate for so long that I do not think
    // we need to test this anymore.  (Fall 2015)

    /*
     * Unless we are in a hurry, let's throw away the Bentley-Ottman
     * speedup and test for correctness.  Correctness is much more
     * important than a bad answer delivered quickly.  Hopefully we
     * will smoke out all the bugs in DEBUGGING mode and the production
     * mode code will be both correct and swift.  (Summer 2015)
     */
    if (! is_correct_based_on_brute_force_verification(hits, sls))
    {
        KJB_THROW(Runtime_error);
    }
#endif

    return std::vector< std::pair<size_t,size_t> >(hits.begin(), hits.end());
}


class is_endpoint_endpoint
:   public std::unary_function<const std::pair<size_t, size_t>&, bool>
{
    const std::vector< RatPoint_line_segment >* sl_;

public:

    is_endpoint_endpoint(const std::vector< RatPoint_line_segment >* sl)
    :   sl_(sl)
    {
        NTX(sl);
    }

    bool operator()(const std::pair<size_t, size_t>& p) const
    {
        const RatPoint_line_segment &s = sl_ -> at(p.first),
                                    &t = sl_ -> at(p.second);
        RatPoint_line_segment x(s);

        if (segment_intersection(s, t, &x))
        {
            return     is_degenerate(x)
                    && (s.a == x.a || s.b == x.a)
                    && (t.a == x.a || t.b == x.a);
        }
        KJB_THROW(Illegal_argument);
    }
};


std::vector< std::pair<size_t, size_t> > get_interior_intersections(
    const std::vector< RatPoint_line_segment >& sl
)
{
    const std::vector< std::pair<size_t, size_t> > all_i=get_intersections(sl);
    std::vector< std::pair<size_t, size_t> > ii;
    std::remove_copy_if(all_i.begin(), all_i.end(), std::back_inserter(ii),
                            is_endpoint_endpoint(&sl));
    return ii;
}


}
}

