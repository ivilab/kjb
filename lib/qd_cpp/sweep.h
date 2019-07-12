/**
 * @file
 * @author Andrew Predoehl
 * @brief Helper functions and classes for sweep-line algorithms
 *
 * In this file there are support structures for easy implementation of the
 * Bentley-Ottmann line intersection algorithm as presented in Chapter 2 of
 * de Berg et al., _Computational Geometry_, 2nd ed., Springer (Berlin: 1997).
 * If you're writing an algorithm using a sweep line paradigm,
 * these functions can help you, but you have to be sweeping through
 * Ratpoint_line_segment objects, along the y-axis,
 * from small to large y coordinate (but actually in row-major order).
 *
 * This is the version 2 implementation.  The version 1 implementation
 * was longer and eventually discovered to be incorrect.  The version 2
 * implementation differs primarily in the implementation of the sweep
 * line data structure -- specifically, it uses a complicated comparison
 * functor instead of storing complicated cotangent objects.  The
 * comparison functor SweepCompare behaves like a "closure" (a term from LISP)
 * which seems to be necessary.  Specifically it requires a pointer to a
 * RatPoint location that is maintained equal to the sweep location.
 *
 * Another change from version 1 is that the sweep line set uses as keys simple
 * objects called Shemps -- basically a "costumed" integer.  A Shemp can be
 * either a transformed index to a real line segment, or it can be a helper
 * entity used to support the insertion and search operations.  The helper
 * entities are called Fake Shemps, and the "costumed" version of real segment
 * indices are called True Shemps.
 *
 * Two of the Fake Shemps are the sweep line bumpers.
 * The bumpers are extra pseudo-data added to the sweep line
 * at the far left and right, like the side walls of a loose axis-aligned
 * bounding box.  They intersect nothing and their endpoints are not
 * event points, so they have no effect on the output.  But they guarantee
 * that queries at event points within the sweepline can scan left or right
 * but never fall off the end of the sweepline, without any need to compare
 * against the begin() or end() iterators.  This greatly simplifies the code.
 * It is the user's responsibility to compute the bumpers, hand their
 * coordinates to the ctor of the sweep line comparison functor, and then
 * insert them into the sweep line.  Function fill_queue_get_bumpers() shows
 * how to compute the bumpers (you can use that function if you like).
 *
 * Events are passed in and out of the Event queue and sweepline via
 * the "ulma" array -- a four-element array where each element is
 * a std::set<size_t> storing segment indices.  The [0] element stores indices
 * of segments whose upper endpoint is at the event point -- i.e., new
 * segments just reached by the sweep.  Element [1] stores indices of segments
 * whose lower endpoint is at the event point -- i.e., departing segments
 * about to be removed from the sweep line.  Degenerate segments might be in
 * both [0] and [1] simultaneously.  Element [2] is for segments crossed in
 * the middle, so that an interior point of the segment is on the sweep line.
 * Element [4] is the union of first three ("all").
 * Note that upper, lower, middle, all has initialism ULMA, to help you
 * remember what information is in which set.
 *
 * The ULMA initialism was formed while thinking of a traditional graphics
 * style layout, where y coordinate increases downward, like the row number
 * of a matrix.  The sweep line sweeps downward.  Upper endpoints have
 * smaller y coordinate.  This is all contrary to the Cartesian assumptions
 * of up and down that pervade the DCEL documentation.  Sorry about that.
 * FIXME:  revise as Cartesian; refer to "luma" array, also a nice initialism.
 *
 * The sweep line requires a functor that turns a positive size_t value
 * into a rectified segment.  This could be very simple in the case of a
 * static set of segments, or more complicated for a dynamic set of segments.
 * The current implementation uses generic programming, i.e., templates.
 * Possibly an inheritance-based implementation would be better.
 *
 * - Contents:
 *
 *  - Public Functions:
 *   - rectify()                     -- convenience utility transforms segment
 *   - does_seg_straddle_sweepline() -- invariant test, pure functional
 *   - fill_queue_get_bumpers()      -- scan set of segments for initial setup
 *   - get_event_neighbors()         -- perform geometric query on sweep line
 *   - pull_events_here()            -- remove at least 1st event from queue
 *   - update_sweep()                -- ulma determines SL deletions,insertions
 *   - find_new_event()              -- (plain version) maybe generate new event
TENTATIVELY ELIMINATE *   - find_new_event()              -- (factory version) for fancy events
 *   - debug_queue_dump()            -- list event queue contents to a stream
 *   - debug_event_dump()            -- list ulma contents to a stream
 *   - debug_sweepline_dump()        -- list sweepline contents to a stream
 *
 *  - Classes and class templates:
 *   - Shemp               -- sweep line entity (handle to segment, or helper)
 *   - Event_point         -- basic event-queue entity, locus and index
 *   - Event_p_factory     -- basic functor used by plain find_new_event()
 *   - Static_segment_map  -- simplest possible sweep-to-segment adapter
 *   - SweepCompare        -- segment comparison functor of a sweep line
 *   - SweepLine::SL       -- sweep line geometric segment container
 *   - SweepLine::iterator -- iterator into SL (also there's a const version)
 *   - SweepLine::const_iterator
 *
 *  - Internally used Functions:  stuff you should not need to call
 *   - classify_segment()  -- put segment index in proper ulma bin,
 *                            called by pull_events_here()
 *
 *  - Internally used Classes:  stuff you should not need to instantiate
 *   - SweepLine           -- convenience template for classes using common
 *                            comparison functor (useful to typedef).
 *   - SweepLine::Erase    -- helper functor to erase segments from SL,
 *                            called by update_sweep()
 *   - SweepLine::Insert   -- helper functor to insert segments into SL,
 *                            called by update_sweep()
 */
/*
 * $Id: sweep.h 20176 2015-12-12 20:31:18Z predoehl $
 */

#ifndef QD_CPP_SWEEP_H_INCLUDED_IVILAB_UARIZONAVISION
#define QD_CPP_SWEEP_H_INCLUDED_IVILAB_UARIZONAVISION

#include <l/l_sys_lib.h>
#include <l_cpp/l_util.h>
#include <l_cpp/l_exception.h>
#include <qd_cpp/ratpoint.h>

#include <boost/shared_ptr.hpp>
#include <boost/make_shared.hpp>

#include <set>
#include <vector>
#include <algorithm>
#include <functional>

#ifdef DEBUGGING
#include <qd_cpp/svgwrap.h>
#include <ostream>
#include <sstream>
#else
#include <iosfwd>
#endif

namespace kjb
{
namespace qd
{
namespace sweep
{


/// @brief sentinel value used anytime we do not want to specify an index
const size_t SWEEP_BLANK = INT_MAX;


/**
 * @brief Sort a, b endpoints of a segment such that a <= b.
 *
 * As you might know, RatPoint objects follow row-major order.  This sorts
 * the endpoints, i.e., swaps them if necessary.
 * You don't have to use this, but you might find it useful.
 * The SegMap::lookup() function must return a segment that
 * would not be changed by this (idempotent) transformation.
 */
RatPoint_line_segment rectify(const RatPoint_line_segment&);



/// In other words, does the segment cross the sweepline?
inline
bool does_seg_straddle_sweepline(
    const RatPoint_line_segment& s, ///< rectified segment
    const RatPoint& p ///< sweepline locus
)
{
    KJB(ASSERT(s.a <= s.b)); // s must be rectified
    return s.a <= p && p <= s.b;
}



/**
 * This is what the Sweep line actually stores.
 *
 * The Sweep line has truck in two things:  handles of real segments the user
 * wants to sweep (true Shemps, in the parlance here), and helper constructs
 * that enable or simplify the sweep procedure.  By making a separate class
 * we make explicit which data are genuine indices of user segments, and which
 * are biased to carry this extra meaning.  In the old days, we started with a
 * fixed set of segments and made the helpers use indices larger than the
 * largest index of the input set.  Since the helpers never intersected true
 * inputs, the output set of (i,j) pairs of intersections never included them
 * and the i,j values did not need any bias to be applied or removed.
 * We cannot use that trick any longer if we want to sweep a dynamic set of
 * segments -- i.e., I want to be able to sweep a dynamic set of segments that
 * might grow larger as the sweep proceeds.  So the maximum value when we start
 * isn't generally going to be the maximum value when we finish.
 *
 * So each index of a user's segment gets to put on the "costume" of a +3 bias
 * and turns into a Shemp, in this case a true Shemp.  Also the helper entities
 * get to appear as Shemps, but they are fake Shemps because they are helpers.
 * All Shemps, true or fake, get to participate in the Sweep line computations.
 * When you want to go from a Shemp to the the SegMap, you can test whether
 * the Shemp is_true() or is_fake().  If it is true, you can take off the
 * "stupid man suit" and get the true index using method true_index().
 * For ordinary shemp-shemp interactions and comparisons, use shemp_index().
 *
 * FAKE SHEMPS:
 * shemp_==0:  VSPIKE, refers to a query at the sweep location
 * shemp_==1:  LBUMP, left bumper segment
 * shemp_==2:  RBUMP, right bumper segment
 *
 * TRUE SHEMPS:
 * shemp_ > 2:  refers to a user-provided segment with index (shemp_-3)
 *              (metaphorically:  a true segment wearing a Shemp costume.
 *              Unlike the Stooges, here we can encounter many true Shemps.
 *              We just need to differentiate them from the fakes.)
 *
 * Implementation goal:  I want this class to perform all its service at
 * compile time -- to generate errors if true segment indices are mixed with
 * Shemps.  I want its sizeof() to be the same as a size_t -- no runtime
 * cost in terms of space or time.
 *
 * About the name:  I was inspired by the closing credits of Evil Dead.
 * The Evil Dead producers were inspired by the Three Stooges, who became
 * just two when their partner Shemp died and they still had more short films
 * to shoot according to contract.  So they employed "fake Shemps" --
 * stand-in actors with appropriate costume and wig but not very visible --
 * to "enable" their last films.  I believe in finding vivid metaphors
 * when trying to encode or explain abstract concepts, so this Fake Shemp
 * terminology seems lively and illustrative, though a bit strained.
 * The phrase "stupid man suit" seems appropriate because the Stooges were
 * stage personas of intelligent actors behaving stupidly, and it's a quote
 * from the film Donnie Darko, when they are screening Evil Dead.
 *
 * Like the Three Stooges, this might at first seem really repulsive and crude
 * but on further thought, its crudeness reflects and thus comments on the
 * human condition.   So do not be repulsed:  all its methods can be inlined
 * effortlessly and the forced partitioning of True Shemps from Fake Shemps
 * makes the software clearer and makes maintenance more likely to be bug-free.
 * It's a comment on the human condition because we dim humans easily confuse
 * things of the same shape and size, and a true-segment index could easily
 * be confused with a biased index.  Easy to forget to bias or unbias an
 * integer.  Clearer if the containers force you label the indices as biased
 * or unbiased, explicitly.  It is much ado about very little, but more than
 * nothing.
 */
class Shemp
{
    size_t shemp_;

public:
    /// The fake Shemps:  vertical spike, left bumper, right bumper
    enum { VSPIKE, LBUMP, RBUMP, NFAKE };

    explicit Shemp(size_t ishemp) : shemp_(ishemp) {}

    size_t shemp_index() const { return shemp_; }

    /// "Why are you wearing that stupid man suit?  Take it off."
    size_t true_index() const { return shemp_ - NFAKE; }

    // DO NOT ADD operator size_t() !  Its ABSENCE is the whole point!!

    /// Is this one a Fake Shemp or a True Shemp?
    bool is_fake() const { return shemp_ < NFAKE; }
    bool is_true() const { return ! is_fake(); }

    /// named ctor to build a Shemp from a user's true segment index
    static Shemp ctor_from_true(size_t itrue)
    {
        return Shemp(itrue + NFAKE);
    }
};


std::ostream& operator<<(std::ostream&, const Shemp&);


/**
 * @brief basic unit of sweep
 *
 * This class might be useful to you, or maybe too bare bones.
 * You can use it as a base class for more complicated events.
 * All uses of Event_point in this header and sweep.cpp use pointers.
 */
struct Event_point
{
    RatPoint p;

    // Which segment does this hit?  true index into SegMap.  Not a Shemp!
    // . . . but possibly SWEEP_BLANK.
    size_t index;

    Event_point(const RatPoint& r, size_t ix) : p(r), index(ix) {}

    virtual ~Event_point() {}
};


/// @brief convenience typedef
typedef boost::shared_ptr<Event_point> Event_p;


/// @brief generator for plain Event_points, used by plain find_new_event
/*
struct Event_p_factory
{
    Event_p operator()(const RatPoint& p, size_t index) const
    {
        return boost::make_shared<Event_point>(p, index);
    }
};
*/


std::ostream& operator<<(std::ostream&, const Event_point&);


/// @brief typical lexicographical ordering of the individual fields
inline
bool operator<(const Event_point& p, const Event_point& q)
{
    // parentheses here are not necessary except to quiet the compiler.
    return       p.p <  q.p
            ||  (p.p == q.p && p.index < q.index);
}


/// @brief order smart pointers by the order of the referents
inline
bool operator<(const Event_p& p, const Event_p& q)
{
    return *p < *q;
}


/**
 * Here is a SegMap functor for the simple case of a static list of rectified
 * line segments.  It's a "closure" style functor that keeps a pointer
 * to a vector of segments.  Make sure the vector you construct with will
 * outlive the functor, naturally.
 *
 * You don't have to use this, but you do need a SegMap functor to pass
 * to the SweepCompare ctor.  This is an example of about the simplest
 * possible SegMap functor.  In general Sweep expects a lookup() method
 * and a valid() method.  Like most functors, this is painlessly passed
 * by value, because it is so lightweight.
 * The lookup() method may return a const reference or an actual object.
 *
 * A lookup method must return a rectified segment s (that is, s.a <= s.b).
 * In this specific example, we assume the input is already rectified.
 * A fancier functor might dynamically rectify the output.
 *
 * Remember that the Sweep client passes biased
 * indices (Shemps) in and out of the sweep line but that Sweep removes
 * the bias before accessing the SegMap functor.  No Shemps here!
 */
class Static_segment_map
{
    const std::vector< RatPoint_line_segment >* const segments_;

public:
    /// @param segments must point to a list of rectified segments
    /// (just for this class -- not a requirement for all SegMaps.)
    Static_segment_map(const std::vector< RatPoint_line_segment >* segments)
    :   segments_(segments)
    {
        NTX(segments);
    }

    /// Interface for general SegMap::lookup(size_t) has two rules:
    /// - This takes a true index as its parameter, not a Shemp.
    /// - Return value must be a rectified segment, or a reference to one.
    const RatPoint_line_segment& lookup(size_t i) const
    {
        return segments_ -> at(i);
    }

    /// Interface for general SegMap::valid(size_t):
    /// input is a true index as a parameter, not a Shemp.
    bool valid(size_t i) const
    {
        return i < segments_ -> size();
    }
};


/**
 * This functor class is used to implement the comparison in the
 * sweepline.  The sweepline set stores Shemps,  which
 * should be regarded as (usually) segment indices.
 *
 * The function operator implements a geometric comparison of
 * segments, obtained using get_segment().  Those segments might be real
 * or "fake Shemps," i.e., helper segments.
 *
 * The geometric comparison is conceptually simple, and is based on the
 * concept of an imaginary horizontal line through the sweep location
 * (*sweep_at_).  It is called the sweep line.  Segments are ordered
 * primarily by left-to-right spacing along the sweep line, and secondarily
 * by slope so that the order reflects left-to-right spacing immediately
 * beyond the sweep line.  The secondary criterion is used when the primary
 * criterion cannot be applied, that is, when the two segments intersect
 * on the sweep line.  Two overlapping segments resist both the primary
 * and secondary criteria, and those are compared using a "last resort"
 * criterion, simply by comparing segment index.  The secondary criterion
 * can be expressed as the comparison of cotangents of (nondegenerate)
 * segments, and sorting them left to right.
 *
 * Remember that std::set expects its members to be in increasing order,
 * and that the comparison here depends on the value of *sweep_at_.
 * So the code here must erase elements when they break the order invariant
 * of the set.  I think we do this carefully enough, but it is a bit risky,
 * like changing the rotator cap of a running engine.  I can't mentally rule
 * out a subtle bug in the overall algorithm on this delicate point.
 *
 * Current implementation stores static left and right bumpers.
 * If they need to be adaptive, use the reference versions but make sure
 * referent bumpers live somewhere else and outlive any functor that refers
 * to them.
 *
 * SegMap could have been an abstract class.  That might have been easier.
 */
template <typename SegMap>
class SweepCompare : public std::binary_function< Shemp, Shemp, bool >
{
    SegMap segmap_;
    const RatPoint* const sweep_at_;

    //const RatPoint_line_segment &l_bumper_, &r_bumper_; // adaptive (slower)
    const RatPoint_line_segment l_bumper_, r_bumper_; // static (faster)

    RatPoint_line_segment get_segment(Shemp i) const
    {
        if (i.is_true())
        {
            return segmap_.lookup(i.true_index());
        }
        else if (Shemp::VSPIKE == i.shemp_index())
        {
            const RatPoint above(sweep_at_ -> x, 1 + sweep_at_ -> y);
            return RatPoint_line_segment(*sweep_at_, above);
        }
        else if (Shemp::LBUMP == i.shemp_index())
        {
            return l_bumper_;
        }
        // else,
        KJB(ASSERT(Shemp::RBUMP == i.shemp_index()));
        return r_bumper_;
    }

public:
    SweepCompare(
        SegMap segmap,
        const RatPoint* sweep_location,
        const RatPoint_line_segment& lbump,
        const RatPoint_line_segment& rbump
    )
    :   segmap_(segmap),
        sweep_at_(sweep_location),
        l_bumper_(lbump),
        r_bumper_(rbump)
    {
        NTX(sweep_location);
    }

    /*
     * This works like less(segment i, segment j) -- reveals which segment
     * is to the left when the sweep is at the sweep location.  The sweep
     * location is basically like a horizontal line, but with the nuance
     * that really we use row-major ordering.  Effectively it is as if all
     * segments in the input were infinitesimally rotated clockwise (in a
     * cartesian plane) so that none of the inputs are perfectly horizontal
     * anymore.
     *
     * This comparison is a bit fragile in the sense that it depends on
     * the value of sweep_at_, and as sweep_at_ changes, the ordering of
     * the nodes will change.  That breaks the preconditions of the std::set
     * container, I suppose, which is meant to be ordered.  However, the
     * implementation of Bently-Ottman in this file is designed to remove
     * and, if necessary, replace nodes so that the order invariant is
     * always maintained.  It might be broken temporarily when segments
     * intersect, but I haven't seen any bad behavior.  Not sure.
     *
     * Part of the trickery we require is that every segment in the sweepline
     * straddles a horizontal line through *sweep_at_, at its interior or
     * at an endpoint.  If not at its interior, then either the top endpoint
     * must lie on or to the left of *sweep_at_, or the bottom endpoint must
     * lie on or to the right of *sweep_at_.  Both are true for horizontal
     * segments, where "top" and "bottom" are interpreted according to the
     * row-major ordering of coordinates -- "top" means "smaller y coordinate,
     * if they differ, otherwise smaller x coordinate."
     *
     * This implementation of Bentley-Ottman does not insert degenerate
     * segments (length zero) into the sweepline, but my hunch is that this
     * comparison function would still work ok; the degenerate segment would
     * test as horizontal.
     */
    bool operator()(Shemp i, Shemp j) const
    {
        using kjb::qd::line_intersection;
        const RatPoint_line_segment
            hz(RatPoint(0, sweep_at_ -> y), RatPoint(1, sweep_at_ -> y)),
            sa = get_segment(i), sb = get_segment(j);

        // straddle invariant
        KJB(ASSERT(does_seg_straddle_sweepline(sa, *sweep_at_)));
        KJB(ASSERT(does_seg_straddle_sweepline(sb, *sweep_at_)));

        /*
         * Test for the annoying case of horizontal segments.
         */
        if (is_horizontal(sa))
        {
            if (is_horizontal(sb))
            {
                // Comparison of last resort:  segment index
                return i.shemp_index() < j.shemp_index();
            }
            /*
             * Basically use primary sort criterion, spacing,
             * and we use <, not <=, because of
             * secondary criteria:  horizontal has +infinite cotangent.
             * (de Berg p.26 step 6, last sentence).
             */
            return *sweep_at_ < line_intersection(sb, hz);
        }

        /* Now sa is not horizontal, so we can find a single point of
         * intersection with the horizontal sweep LINE (not line segment).
         */
        const RatPoint ha = line_intersection(sa, hz);
        KJB(ASSERT(ha.y == sweep_at_ -> y));

        if (is_horizontal(sb))
        {
            /* Use primary sort criterion, spacing, and <=, not <, because of
             * secondary criteria:  horizontal has +infinite cotangent.
             * (de Berg p.26 step 6, last sentence).
             */
            return ha <= *sweep_at_;
        }

        /* sb is also tilted -- both segments intersect the sweep line and have
         * finite cotangents.
         */
        const RatPoint hb = line_intersection(sb, hz);
        KJB(ASSERT(ha.y == hb.y));

        /* If those points are distinct, the order is defined by x coordinate,
         * the primary criterion.
         */
        if (ha.x != hb.x) return ha.x < hb.x;

        /* If both segments hit the same point on the sweepline, use the
         * secondary criterion, cotangent.
         * Doing so compares their left-to-right order just beyond the
         * sweepline.
         */
        const RatPoint::Rat d = (sa.b.x - sa.a.x)*(sb.b.y - sb.a.y)
                                - (sb.b.x - sb.a.x)*(sa.b.y - sa.a.y);
        if (d != 0) return d < 0;

        /* The lines overlap, so their geometry is locally indistinguishable.
         * So we fall back to our comparison of last resort:  segment index.
         */
        return i.shemp_index() < j.shemp_index();
    }
};



template <typename SegMap>
struct SweepLine
{
    typedef typename std::set< Shemp, SweepCompare<SegMap> > SL;
    typedef typename SL::iterator iterator;
    typedef typename SL::const_iterator const_iterator;

    // Functor used by update_sweep()
    class Erase
    {
        SL* const psweep_;
        std::vector<iterator>* const p_rev_sw_;

    public:
        Erase(SL* p, std::vector<iterator>* r)
        :   psweep_(p),
            p_rev_sw_(r)
        {}

        // parameter is a true index, not a Shemp
        void operator()(size_t seg_index)
        {
            // Our records indicate you are indeed in the sweep line:
            KJB(ASSERT(p_rev_sw_ -> at(seg_index) != psweep_ -> end()));
            psweep_ -> erase( p_rev_sw_ -> at(seg_index) );
            p_rev_sw_ -> at(seg_index) = psweep_ -> end();
        }
    };


    // Functor used by update_sweep()
    class Insert
    {
        SL* const psweep_;
        std::vector<iterator>* const p_rev_sw_;

    public:
        Insert(SL* p, std::vector<iterator>* r)
        :   psweep_(p),
            p_rev_sw_(r)
        {}

        // parameter is a true index, not a Shemp
        void operator()(size_t seg_index)
        {
            // Our records indicate you are not already in the sweep line:
            KJB(ASSERT(p_rev_sw_ -> at(seg_index) == psweep_ -> end()));
            std::pair<iterator, bool>
                j = psweep_ -> insert( Shemp::ctor_from_true(seg_index) );
            KJB(ASSERT(j.second));
            p_rev_sw_ -> at(seg_index) = j.first;
        }
    };
};



/**
 * @brief Transfer segment endpoints into an ordered set, the event queue.
 *
 * Also it also does some funny business by computing two special synthethic
 * segments, the left bumper and the right bumper.  See file comments above.
 * This function computes their
 * coordinates since it has to scan all the segments endpoints anyway.
 *
 * This version does NOT accept a factory like find_new_event() -- maybe we
 * should do that but we haven't done it yet.  Instead, if the default
 * generation practice is not adequate, you'll have to go back into the
 * queue and regenerate the events.
 *
 * One small variation on the queue setup is that you may opt to fill the
 * index field of "exit" events with the SWEEP_BLANK value.  The "exit"
 * event is the event reached later in row-major order.  This makes sense
 * when you do not need to know the index of such events because you will
 * query the index (or indices) from the sweepline.  It is especially
 * useful when you *do not know* those indices, i.e., if you are dynamically
 * splitting segments during the sweep and the index might become incorrect
 * as a result of splitting.  This is precisely the case in the DCEL sweep
 * merge application of the sweep machinery.
 *
 * The blank feature is NOT useful when the sweepline cannot supply the
 * information you want and you must obtain it from the queue.  Such is the
 * case in the y-monotone sweep routine, where the sweepline contents are
 * an abridged version of the input polygon -- only the edges that form
 * left borders are in the sweepline, so the sweepline cannot tell us
 * which segments for right borders, because they've been "suppressed."
 * Thus in that application of the sweep machinery, we need correct exit
 * indices.
 *
 * The consequence of an index of SWEEP_BLANK is that the index is
 * overlooked by pull_events_here() -- it does not put the blank index into
 * the ulma array.  For more about a blank index, see find_new_event().
 */
std::set<Event_p> fill_queue_get_bumpers(
    const std::vector< RatPoint_line_segment >&,
    RatPoint_line_segment*,
    RatPoint_line_segment*,
    bool blank_exit = false
);



/**
 * This performs a geometric query of the sweep status line at the sweep
 * location.  It returns two sweepline const_iterators pointing to segments
 * strictly to the left (first) and right (second) of sweep_location, called
 * the neighbors.  The neighbors are "strictly" left and right in the sense
 * that they do not intersect sweep_location, and they are neighbors in the
 * sense that no other strictly leftward segment is closer than the left
 * neighbor, and similarly for the right neighbor.  Of course the left-right
 * perspective is only with respect to the horizontal sweep line.
 * Thanks to the sweep-line bumpers, these neighbors always exist.
 * They are by definition distinct.
 *
 * If you wish to enumerate all segments that intersect the sweep location,
 * simply increment the first field of the return value.  This is always safe
 * to do.  After you do that, the first and second fields can be regarded
 * as a [begin,end[ style range enumerating all segments (if any)
 * intersecting the sweepline at the sweep location.
 *
 * Of course both fields of the return pair point to Shemps, and either or
 * both might be fake.
 */
template <typename SegMap>
std::pair<
    typename SweepLine<SegMap>::const_iterator, /* aka SLCI */
    typename SweepLine<SegMap>::const_iterator
>
get_event_neighbors(
    const typename SweepLine<SegMap>::SL& sweep,
    const RatPoint& sweep_location,
    SegMap segmap
)
{
    typedef typename SweepLine<SegMap>::const_iterator SLCI;

    /*
     * This function profoundly relies on the presence of the bumpers.
     * The iterators boldly march towards the left and right borders of the
     * sweep status structure, knowing that the sweep location is
     * strictly between the bumpers.
     */
    SLCI il = sweep.lower_bound(Shemp(Shemp::VSPIKE)), ir = il--;
    /*
     * il, ir indicate segments to the left (il) and right (ir) of each other,
     * if not coincident -- possibly the bumpers, possibly segments
     * immediately neighboring sweep_location, possibly both on sweep_location.
     */
    KJB(ASSERT(ir != sweep.begin() && ir != sweep.end()));

    KJB(ASSERT(   il -> is_fake()
               || does_seg_straddle_sweepline(
                      segmap.lookup(il -> true_index()), sweep_location)));
    KJB(ASSERT(   ir -> is_fake()
               || does_seg_straddle_sweepline(
                      segmap.lookup(ir -> true_index()), sweep_location)));

    // scan left
    while (     il -> is_true()
           &&   is_on(segmap.lookup(il -> true_index()), sweep_location))
    {
        --il;
    }

    // scan right
    while (     ir -> is_true()
           &&   is_on(segmap.lookup(ir -> true_index()), sweep_location))
    {
        ++ir;
    }

    return std::make_pair(il, ir);
}



/**
 * Given a segment s that intersects sweep_location, we analyze its geometric
 * position, and store its index in entries of ulma[0..3] appropriately.
 *
 * Sets ulma[0..2] correspond to upper, lower, middle sets:  s is upper if
 * its smaller endpoint touches sweep_location, lower if its bigger endpoint
 * does so, and middle if sweep_location is an interior point.
 * Also we always store the index in ulma[3] (corresponding to all).
 * Note that iff s is degenerate, it is both upper and lower (not middle).
 *
 * The upper/lower/middle language is from the exposition of de Berg and
 * applies to a "graphics" visualization, not a cartesian one.  The upper
 * endpoint of a segment has smaller y coordinate that the lower endpoint, with
 * a graphics perspective.  The sweep line starts from the uppermost vertex.
 * Those who prefer a cartesian view must mentally mirror it all vertically.
 *
 * The code here is much streamlined because s is rectified.
 *
 * This is a low-level function and high-level code probably does not ever
 * need to call it.
 */
void classify_segment(
    const RatPoint_line_segment s,
    size_t index,
    std::set<size_t> ulma[], ///< functionally output-only (appended-to)
    const RatPoint sweep_location
);




/**
 * Move at least the first item from the event queue plus any colocated items.
 * Store the corresponding segment indices in ulma[0..2] (corresponding to
 * upper, lower, middle) appropriately.
 * Also store the indices in ulma[3] (corresponding to all).
 *
 * Also we copy all segment indices from the SweepLine status structure,
 * into ulma, provided that the segments touch the event point.
 * Note sweep is const and those indices are only copied, not moved.
 */
template <typename SegMap>
inline
void pull_events_here(
    std::set<Event_p>* eq, // mostly input (but it shrinks)
    typename SweepLine<SegMap>::SL const& sweep, // input (unchanged)
    std::set<size_t>* ulma, // ulma array -- output (grows)
    SegMap segmap
)
{
    KJB(ASSERT(eq && !eq -> empty()));
    const RatPoint sweep_location = (* eq -> begin()) -> p;

    // Pull from event queue.
    while (! eq -> empty() && sweep_location == (* eq -> begin()) -> p)
    {
        const size_t i = (* eq -> begin()) -> index;
        eq -> erase(eq -> begin());
        if (i != SWEEP_BLANK)
        {
            KJB(ASSERT(segmap.valid(i)));
            classify_segment(segmap.lookup(i), i, ulma, sweep_location);
        }
    }

    // query sweep line: probe at sweep_location and scan intersectors.
    typedef typename SweepLine<SegMap>::const_iterator SLCI;
    std::pair< SLCI, SLCI >
             eir = get_event_neighbors(sweep, sweep_location, segmap);
    while (++eir.first != eir.second)
    {
        KJB(ASSERT(eir.first -> is_true()));
        const size_t i = eir.first -> true_index();
        KJB(ASSERT(is_on(segmap.lookup(i), sweep_location)));
        classify_segment(segmap.lookup(i), i, ulma, sweep_location);
    }
}





/**
 * Update sweep by removing departing segments, inserting newborn segments,
 * and shifting segments whose interior is on the sweep line.  Degenerate
 * segments are never inserted, never removed.  This returns the number
 * of inserted segments (including re-insertions, i.e., shifted segments).
 *
 * *p_rev_sw is indexed by segment number, and points to the location in the
 * sweepline, iff that segment currently exists in the sweepline.  Otherwise
 * the sentinel iterator psweep->end() is stored at that index.
 * We read this array to erase segments from *psweep, and write to it when
 * we insert into *psweep.  In other words, it is used for reverse lookup,
 * from segment index to sweepline entry.
 *
 * "Why," perhaps you ask, "is p_ref_sw a parameter, and not a private member
 * of a class?"  Good question.  In the standard Bentley-Ottman sweep, this
 * reverse lookup array is only used by this function, and thus could have
 * been private, if this code were only used to implement Bentley-Ottman.
 * But in fact it is used for other things, so the reverse lookup array has
 * been left public.  This is specifically useful for DCEL merge function
 * sweep_edge_merge(), because DCEL merging causes new segments to come into
 * existence during the sweep, and thus we do not initially know the final
 * length needed for this array.  So, it cannot be private.
 */
template <typename SegMap>
size_t update_sweep(
    typename SweepLine<SegMap>::SL* psweep,
    const std::set<size_t> ulm3[],
    std::vector< typename SweepLine<SegMap>::iterator > *p_rev_sw
)
{
    using namespace std;
    typedef typename SweepLine< SegMap >::Erase Eraser;
    typedef typename SweepLine< SegMap >::Insert Inserter;

    /* Segment indices (possibly degenerate) at the current event point.
     * Degenerate segments are in both "upper" and "lower" sets (and only if).
     * They are like spontaneous abortions, never alive, never born nor dying.
     *
     * I tried other data structures like std::deque and std::list for the
     * temporary sets.  Deque was about the same.   In production mode,
     * vector was measurably faster than list; in debug mode, list appeared
     * to be faster.  This was just curiosity; nothing made much difference.
     *
     * I also tried a vector data structure storing degenerates, upper,
     * middle, lower -- last three nondegenerate, so this is a partition.
     * Each partition kept in sorted order.  The ranges are in order this
     * way so that "eraseable" and "insertable" elements are in a contiguous
     * range.  It was a little bit faster, but fragile and ugly.
     */
    const set<size_t> &upper(ulm3[0]), &lower(ulm3[1]), &middle(ulm3[2]);
    vector<size_t> dying, newborn, eraseable, insertable;

    /* Find "dying" segments -- they are in "lower" but not degenerate:
     * they lived a full life.  Use "dying" instead of "lower" after this.
     * (Degenerate segments hardly can be dying since they never were newborn.)
     */
    set_difference(lower.begin(), lower.end(), upper.begin(), upper.end(),
                        back_inserter(dying));

    // Find "newborn" segments -- they are in "upper" but not degenerate.
    set_difference(upper.begin(), upper.end(), lower.begin(), lower.end(),
                   back_inserter(newborn));

    // Erase the dying and the middles from sweep.
    set_union(dying.begin(), dying.end(), middle.begin(), middle.end(),
              back_inserter(eraseable));
    for_each(eraseable.begin(), eraseable.end(), Eraser(psweep, p_rev_sw));

    // Insert the newborns and reinsert the middles into sweep
    set_union(newborn.begin(), newborn.end(), middle.begin(), middle.end(),
              back_inserter(insertable));
    for_each(insertable.begin(), insertable.end(), Inserter(psweep, p_rev_sw));
    return insertable.size();
}



// plain version -- inserts new Event_point objects (base class)
/*
template <typename SegMap>
inline
void find_new_event(
    std::set<Event_p>* peq,
    Shemp shemp1,
    Shemp shemp2,
    SegMap segmap,
    const RatPoint& sweep_loc
)
{
    find_new_event(peq, Event_p_factory(), shemp1, shemp2, segmap, sweep_loc);
}
*/


/*
 * Factory version -- inserts objects derived from Event_point,
 * which are constructed using your factory, given location and segment index.
 *
 * When a new event is discovered, only its location is stored in the event
 * queue.  Its index is essentially omitted because it is set to SWEEP_BLANK.
 * All known applications of the sweep machinery lack a need to ascertain
 * *from the event queue* those involved indices.  That is because they
 * instead do so by querying the sweep line.  Storing the indices would just
 * clutter the event queue, and in some cases cause difficulty (e.g., the
 * DCEL sweep merge function).  It's within the realm of imagination,
 * however, that some future application might require an index there.
 * Such needs could almost surely be satisfied, but until they are specific
 * and concrete we will just set the index field to SWEEP_BLANK and return
 * nothing from this function.
 */
template <typename SegMap /*, typename EvFac */ >
inline
void find_new_event(
    std::set<Event_p>* peq,
    /* EvFac event_factory, */
    Shemp shemp1,
    Shemp shemp2,
    SegMap segmap,
    const RatPoint& sweep_location
)
{
    KJB(ASSERT(peq));
    // Shemps might be fake, specifically bumpers, but they're not vspike.
    KJB(ASSERT(Shemp::VSPIKE != shemp1.shemp_index()));
    KJB(ASSERT(Shemp::VSPIKE != shemp2.shemp_index()));

    // Fake Shemps do not generate events.
    if (shemp1.is_fake() || shemp2.is_fake())
    {
        return;
    }

    const size_t index1(shemp1.true_index()), index2(shemp2.true_index());
    KJB(ASSERT(segmap.valid(index1)));
    KJB(ASSERT(segmap.valid(index2)));

    RatPoint_line_segment s1(segmap.lookup(index1)),
                          s2(segmap.lookup(index2)),
                          common(s1);

    bool bx = kjb::qd::segment_intersection(s1, s2, &common);
    const RatPoint px = std::min(common.a, common.b);
    if (bx && sweep_location < px)
    {
        KJB(ASSERT(is_on(s1, px)));
        KJB(ASSERT(is_on(s2, px)));
        // insert one (no need to insert both)
        //peq -> insert(boost::make_shared<Event_point>(px, index1));
        peq -> insert(boost::make_shared<Event_point>(px, SWEEP_BLANK));
        //peq -> insert(event_factory(px, index1));
    }
}


/**
 * @brief print ulma array contents to a stream.
 *
 * Useful for debugging a sweep line algorithm.  Disappears in PRODUCTION.
 * That's a feature, not a bug!  Generates 3 lines of output, showing contents
 * of upper, lower, and middle queues, each queue on one line.
 *
 * @param eol End-of-line string, used 3 times.  If you're generating HTML
 * you might want to set this to "</p>\n<p>" or maybe just "<br>" instead of
 * the newline character.
 */
std::ostream& debug_event_dump(
    const std::set<size_t>*,
    std::ostream&,
    const std::string& eol = "\n"
);


/**
 * @brief print sweep line contents to a stream.
 *
 * Useful for debugging a sweep line algorithm.  Disappears in PRODUCTION.
 * That's a feature, not a bug!
 *
 * @param eol End-of-line string, used 3 times.  If you're generating HTML
 * you might want to set this to "</p>\n<p>" or maybe just "<br>" instead of
 * the newline character.
 */
template <typename Swp>
std::ostream& debug_sweepline_dump(
    Swp const& sweep,
    std::ostream& o,
    const std::string eol = "\n"
)
{
#ifdef DEBUGGING
    o << "sweepline: ";
    std::copy(sweep.begin(),sweep.end(), std::ostream_iterator<Shemp>(o,", "));
    o << eol;
#endif
    return o;
}


template <typename SegMap>
std::string debug_svg(
    SegMap segmap,
    RatPoint_line_segment& lbump,
    RatPoint_line_segment& rbump,
    typename SweepLine<SegMap>::SL const& sweep,
    const std::set<size_t> ulma[],
    const RatPoint& sweep_loc,
    const std::vector<size_t> scenery = std::vector<size_t>()
)
{
#ifdef DEBUGGING
    NTX(ulma);
    if (sweep.size() < 2) return ""; // invalid input

    const double H = get_length(RatPoint_line_segment(lbump.a, rbump.b))/300;
    std::set<size_t> scen(scenery.begin(), scenery.end());

    // This sets up the view box for us.
    SvgWrap w;
    w.set_svg(1, 0);
    w.m_origin_bias_x = dbl_ratio(lbump.a.x);
    w.m_origin_bias_y = dbl_ratio(lbump.a.y);
    w.m_width = dbl_ratio(rbump.b.x - lbump.b.x);
    w.m_height = dbl_ratio(lbump.b.y - lbump.a.y);
    w.m_magnify = 800 / std::max(w.m_width, w.m_height);

    std::ostringstream o;
    o << w()
      // Background
      << "<rect x='" << w.m_origin_bias_x << "' y='" << w.m_origin_bias_y
      << "' width='" << w.m_width << "' height='" << w.m_height
      << "' fill='white'/>\n"

      // Corners
        /*
      << "<circle cx=\"" << dbl_ratio(slash.a.x) << "\" "
          "cy=\"" << dbl_ratio(slash.a.y) << "\" "
          "r=\"" << 10*H << "\" fill=\"pink\" />\n"
      << "<circle cx=\"" << dbl_ratio(slash.b.x) << "\" "
          "cy=\"" << dbl_ratio(slash.b.y) << "\" "
          "r=\"" << 10*H << "\" fill=\"purple\" />\n"
        */

      // Sweep location -- seems to cause Gecko some trouble
      << "<circle cx='" << dbl_ratio(sweep_loc.x)
      << "' cy='" << dbl_ratio(sweep_loc.y)
      << "' r='" << 5*H << "' fill='yellow' />\n"

      // Lines
         "<g stroke-linecap='round' stroke-linejoin='round' "
         "stroke-width='" << H << "'>\n";

    // Line drawing functor
    struct
    {
        SegMap segmap;
        std::set<size_t>* scenery;

        std::string operator()(const std::vector<size_t>&v,const std::string&c)
        {
            if (v.empty()) return "";
            std::ostringstream q;
            q << "<g stroke='" << c << "'>\n";
            for (size_t i = 0; i < v.size(); ++i)
            {
                scenery -> erase(v[i]);
                const RatPoint_line_segment s = segmap.lookup(v[i]);
                q << "<path d='M " << dbl_ratio(s.a.x) <<','<< dbl_ratio(s.a.y)
                  << ' ' << dbl_ratio(s.b.x) << ',' << dbl_ratio(s.b.y)
                  << "' opacity='0.5'/>\n";
            }
            q << "</g>\n";
            return q.str();
        }
    }
    linedraw = {segmap, &scen};

    std::string actif;
    if (sweep.size() > 2)
    {
        std::vector<Shemp> s(sweep.begin(), sweep.end());
        // Drop bumpers
        s.pop_back();
        s[0] = s.back();
        s.pop_back();
        std::vector<size_t> v(s.size()), w;
        // Get segment indices from Shemps
        for (size_t i = 0; i < s.size(); ++i) v[i] = s[i].true_index();
        // Remove segments in ulma
        std::sort(v.begin(), v.end());
        std::set_difference(v.begin(), v.end(),
                        ulma[3].begin(), ulma[3].end(), std::back_inserter(w));
        actif += linedraw(w, "gray");
    }
    actif += linedraw(std::vector<size_t>(ulma[0].begin(),ulma[0].end()), "red");
    actif += linedraw(std::vector<size_t>(ulma[1].begin(),ulma[1].end()), "green");
    actif += linedraw(std::vector<size_t>(ulma[2].begin(),ulma[2].end()), "blue");

    // transfer "retired" segments, whom sweep line has passed, to new vector
    std::vector<size_t> retired;
    for (std::set<size_t>::iterator i = scen.begin(); i != scen.end(); )
    {
        if (segmap.lookup(*i).b < sweep_loc)
        {
            retired.push_back(*i);
            scen.erase(i++); // this is legal, and almost obligatory.
            // (After you erase *i you cannot increment i.)
        }
        else
        {
            ++i; // Do not refactor into 3rd clause of for loop.
        }
    }

    // beige is for future segments, antiquewhite is for retired segments
    o << linedraw(std::vector<size_t>(scen.begin(), scen.end()), "beige")
      << linedraw(std::vector<size_t>(retired.begin(), retired.end()), "antiquewhite")
      << actif // draw active segments ON TOP OF (later) than pale segments.
      << "</g></svg>\n";

    return o.str();
#else
    return "";
#endif
}


std::ostream& debug_queue_dump(
    const std::set< Event_p >& queue,
    std::ostream& o,
    const std::string eol = "\n"
);



} // end ns sweep
} // end ns qd
} // end ns kjb

#endif /* QD_CPP_SWEEP_H_INCLUDED_IVILAB_UARIZONAVISION */

