/**
 * @file
 * @author Andrew Predoehl
 * @brief Implementation of helper stuff for sweep-line algorithms
 *
 */
/*
 * $Id: sweep.cpp 21596 2017-07-30 23:33:36Z kobus $
 */

#include "l/l_sys_debug.h"  /* For ASSERT */
#include "l/l_sys_lib.h"
#include "qd_cpp/sweep.h"

#include <ostream>
#include <iterator>
#include <algorithm>


namespace kjb
{
namespace qd
{
namespace sweep
{


std::ostream& operator<<(std::ostream& o, const Event_point& e)
{
    o << "Event point p=(" << e.p << ")";
    if (e.index != SWEEP_BLANK) o << ", index=" << e.index;
    return o;
}


std::ostream& operator<<(std::ostream& o, const Shemp& s)
{
    switch (s.shemp_index())
    {
        case Shemp::VSPIKE:
            o << "probe";
            break;
        case Shemp::LBUMP:
            o << "left_bumper";
            break;
        case Shemp::RBUMP:
            o << "right bumper";
            break;
        default:
            o << s.shemp_index() - Shemp::NFAKE;
            break;
    }
    return o;
}


// twiddle segment so endpoint A precedes B (unless A==B).
RatPoint_line_segment rectify(
    const RatPoint_line_segment& si
)
{
    RatPoint_line_segment so(si);
    if (si.b < si.a) std::swap(so.a, so.b);
    return so;
}



std::set<Event_p> fill_queue_get_bumpers(
    const std::vector< RatPoint_line_segment>& csl,
    RatPoint_line_segment* lbump,
    RatPoint_line_segment* rbump,
    bool blank_exit
)
{
    NTX(lbump);
    NTX(rbump);
    KJB(ASSERT(lbump != rbump));
    KJB(ASSERT(! csl.empty()));

    // Initialize slash, a diagonal across a tight axis-aligned bounding box.
    RatPoint_line_segment slash(csl.front());

    // Scan segment list, record event points, grow slash incrementally.
    std::set<Event_p> eq;
    //Event_p_factory factory; // default factory
    for (size_t i = 0; i < csl.size(); ++i)
    {
        // Store endpoints in the event queue.
        const size_t exit = blank_exit ? SWEEP_BLANK : i;
#if 1
        const RatPoint_line_segment s(rectify(csl[i]));
        KJB(ASSERT(s.a <= s.b));
        eq.insert(boost::make_shared< Event_point >(s.a, i));
        eq.insert(boost::make_shared< Event_point >(s.b, exit));
#else
        eq.insert(factory(csl[i].a, i));
        eq.insert(factory(csl[i].b, exit));
#endif

        // Grow 'slash' so it emboxenates all segments seen so far.
        const RatPoint::Rat xmin = std::min(csl[i].a.x, csl[i].b.x),
                            ymin = std::min(csl[i].a.y, csl[i].b.y),
                            xmax = std::max(csl[i].a.x, csl[i].b.x),
                            ymax = std::max(csl[i].a.y, csl[i].b.y);
        if (slash.a.x > xmin) slash.a.x = xmin;
        if (slash.a.y > ymin) slash.a.y = ymin;
        if (slash.b.x < xmax) slash.b.x = xmax;
        if (slash.b.y < ymax) slash.b.y = ymax;
    }

    // Add a margin inside the slash bounding box, so it does NOT touch sl[*].
    // Margin is adaptive like this to make the visualization more pleasing.
    // If you didn't visualize and you were in a hurry, you could set it to 1.
    const RatPoint::Rat dx = slash.b.x - slash.a.x, dy = slash.b.y - slash.a.y,
                        margin = std::max(dx, dy) / 32;
    KJB(ASSERT(margin > 0));
    slash.a.x -= margin;
    slash.a.y -= margin;
    slash.b.x += margin;
    slash.b.y += margin;

    // Compute bumpers from slash.
    *lbump = *rbump = slash;
    lbump -> b.x = slash.a.x;
    rbump -> a.x = slash.b.x;

    KJB(ASSERT(lbump -> a < rbump -> a));
    KJB(ASSERT(lbump -> b < rbump -> b));
    KJB(ASSERT(lbump -> a < lbump -> b));
    KJB(ASSERT(rbump -> a < lbump -> b));

    return eq;
}



void classify_segment(
    const RatPoint_line_segment s,
    size_t index,
    std::set<size_t> ulma[], // functionally output-only
    const RatPoint sweep_location
)
{
    KJB(ASSERT(ulma));
    // Store index in the "all" set.
    ulma[3].insert(index);

    std::set<size_t> &upper(ulma[0]), &lower(ulma[1]), &middle(ulma[2]);

    KJB(ASSERT(s.a <= s.b)); // Did rectify() do its job?
    if (s.a == s.b)
    {
        upper.insert(index);
        lower.insert(index);
    }
    else if (sweep_location == s.a)
    {
        KJB(ASSERT(sweep_location < s.b));
        upper.insert(index);
    }
    else if (sweep_location == s.b)
    {
        KJB(ASSERT(s.a < sweep_location));
        lower.insert(index);
    }
    else
    {
if (!(s.a < sweep_location && sweep_location < s.b)) { std::cout << "trouble:  index=" << index << ", s.a=" << s.a << '=' << dbl_ratio(s.a.x) << ',' << dbl_ratio(s.a.y) << ", s.b=" << s.b << '=' << dbl_ratio(s.b.x) << ',' << dbl_ratio(s.b.y) << "\nulma[3] so far: "; std::copy(ulma[3].begin(), ulma[3].end(), std::ostream_iterator<size_t>(std::cout, ", ")); std::cout << std::endl; }
        KJB(ASSERT(s.a < sweep_location && sweep_location < s.b));
        middle.insert(index);
    }
}


std::ostream& debug_event_dump(
    const std::set<size_t> ulm[],
    std::ostream& o,
    const std::string& eol
)
{
#ifdef DEBUGGING
    o << "upper: ";
    std::copy(ulm[0].begin(), ulm[0].end(),
        std::ostream_iterator<size_t>(o, ", "));
    o << eol << "lower: ";
    std::copy(ulm[1].begin(), ulm[1].end(),
        std::ostream_iterator<size_t>(o, ", "));
    o << eol << "middle: ";
    std::copy(ulm[2].begin(), ulm[2].end(),
        std::ostream_iterator<size_t>(o, ", "));
    o << eol;
#endif
    return o;
}



std::ostream& debug_queue_dump(
    const std::set< Event_p >& queue,
    std::ostream& o,
    const std::string eol
)
{
#ifdef DEBUGGING
    for ( std::set< Event_p >::const_iterator j = queue.begin();
            j != queue.end(); ++j )
    {
        //   j points to an Event_p.
        //  *j is an Event_p which points to an Event_point.
        // **j is an Event_point -- that is what we want to print.
        o << **j << eol;
    }
#endif
    return o;
}


}
}
}

