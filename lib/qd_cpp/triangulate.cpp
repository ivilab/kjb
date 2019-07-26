/**
 * @file
 * @brief Implementation of monotone conversion for polygon
 * @author Andrew Predoehl
 *
 * See header for details and citations.
 */
/*
 * $Id: triangulate.cpp 21596 2017-07-30 23:33:36Z kobus $
 */

#include "l/l_sys_debug.h"  /* For ASSERT */
#include "l/l_debug.h"
#include "l_cpp/l_exception.h"
#include "qd_cpp/sweep.h"
#include "qd_cpp/triangulate.h"

#define MONOTONE_CPP_DEBUG 0 /* 0 = off, 1 = on */

#if MONOTONE_CPP_DEBUG
#include "qd_cpp/svg_dcel.h"
#include <fstream>
#include <iostream>
#endif

#include <boost/shared_ptr.hpp>
#include <vector>
#include <string>
#include <set>
#include <sstream>
#include <algorithm>
#include <functional>
#include <numeric>

namespace
{

using kjb::qd::Doubly_connected_edge_list;
using kjb::qd::RatPoint;
using kjb::qd::RatPoint_line_segment;

enum Emv { undefined, START, FINISH, SPLIT, MERGE, PLAIN_R, PLAIN_L };


class Dcel_subset
{
    const Doubly_connected_edge_list* dcel_;
    const std::vector<size_t>* members_;

public:
    Dcel_subset(
        const Doubly_connected_edge_list* d,
        const std::vector<size_t>* m
    )
    :   dcel_(d),
        members_(m)
    {
        NTX(d);
        NTX(m);
    }

    RatPoint_line_segment lookup_no_rectify(size_t i) const
    {
        return get_edge(*dcel_, members_ -> at(i));
    }

    RatPoint_line_segment lookup(size_t i) const
    {
        KJB(ASSERT(valid(i)));
        return kjb::qd::sweep::rectify(lookup_no_rectify(i));
    }

    bool valid(size_t i) const
    {
        return i < members_ -> size();
        //return std::binary_search(members_ -> begin(), members_ -> end(), i);
    }
};



// Sweep line data structure (aka status line) used in monotone plane sweep.
// Stores Shemps of DCEL edge indices (not fence indices).
typedef kjb::qd::sweep::SweepLine< Dcel_subset > Sweep;
typedef Sweep::const_iterator SLCI;




// Return edge index of edge left of current sweep location.
size_t left_bystander(
    const Sweep::SL& sweep,
    const RatPoint& sweep_loc,
    Dcel_subset def
)
{
    std::pair< SLCI, SLCI >
        eir = kjb::qd::sweep::get_event_neighbors(sweep, sweep_loc, def);
    KJB(ASSERT(eir.first -> is_true()));
    // left bumper guarantees this stays legit
    while (is_horizontal(def.lookup_no_rectify(eir.first -> true_index())))
    {
        --eir.first;
        KJB(ASSERT(eir.first -> is_true()));
    }
    // . . . but it shouldn't come to that anyway.
    return eir.first -> true_index();
}



/// @brief obtain a list of all edge indices incident to face fi.
std::vector<size_t> get_incident_edges(
    const Doubly_connected_edge_list &d,
    size_t fi
)
{
    std::vector<size_t> eee;

    if (fi >= d.get_face_table().size()) KJB_THROW(kjb::Index_out_of_bounds);

    // enumerate edges of outer component (must exist since fi > 0)
    if (fi > 0)
    {
        const size_t oce0 = d.get_face_table().at(fi).outer_component;
        eee.push_back(oce0);
        // cycle scan idiom
        for (size_t ocec = d.get_edge_table().at(oce0).next; ocec != oce0;
                    ocec = d.get_edge_table().at(ocec).next )
        {
            eee.push_back(ocec);
        }
    }

    // enumerate edges of inner components
    for (size_t j=0; j < d.get_face_table().at(fi).inner_components.size();
                    ++j)
    {
        const size_t ice0=d.get_face_table().at(fi).inner_components.at(j);
        eee.push_back(ice0);
        // cycle scan idiom
        for (size_t icec = d.get_edge_table().at(ice0).next; icec != ice0;
                    icec = d.get_edge_table().at(icec).next )
        {
            eee.push_back(icec);
        }
    }
    return eee;
}


/*
 * Returns a vector of in, out pairs (in successive entries,
 * i.e., [0], [1], and [even], [even+1]) of SL index of an in-edge to vi,
 * and a SL index of an out edge to vi.
 */
std::vector<size_t> get_nexus(
    size_t vi,
    const std::set<size_t>& sinc, ///< set of incident edges (segmap indices)
    const Doubly_connected_edge_list& d,
    const std::vector<size_t>& eee ///< sinc index to dcel-edge index
)
{
    KJB(ASSERT(0 == sinc.size() % 2));
    std::vector<size_t> nexus(sinc.size());

    /* Build a very small partial inverse table for eee.
     * The size is only as big as the edge-degree of vertex vi on this face.
     */
    std::map<size_t, size_t> inv_eee;
    for (std::set<size_t>::const_iterator i=sinc.begin(); i != sinc.end(); ++i)
    {
        inv_eee[eee[*i]] = *i;
    }

    size_t j = 0;
    for (std::set<size_t>::const_iterator i=sinc.begin(); i != sinc.end(); ++i)
    {
        const size_t eo = eee[*i];
        // queue outedge sweepline index
        if (d.get_edge_table().at(eo).origin == vi)
        {
            nexus.at(j++) = *i;
            const size_t ei = d.get_edge_table().at(eo).prev;
#ifdef DEBUGGING
            KJB(ASSERT(d.get_edge_table().at(
                        d.get_edge_table().at(ei).twin
                            ).origin == vi));
#endif
            KJB(ASSERT(inv_eee.find(ei) != inv_eee.end()));
            // queue matching inedge sweepline index
            nexus.at(j++) = inv_eee[ei];
        }
    }
    KJB(ASSERT(nexus.size() == j));
    return nexus;
}


size_t get_vertex(
    size_t si, ///< SL edge index incident to sweep_loc
    const RatPoint& sweep_loc,
    const Doubly_connected_edge_list& d,
    const std::vector<size_t>& eee ///< map SL edge index to DCEL edge index
)
{
    const size_t ei = eee.at(si);
    const RatPoint_line_segment r = get_edge(d, ei);
    KJB(ASSERT(r.a == sweep_loc || r.b == sweep_loc));
    const Doubly_connected_edge_list::Edge_record& h=d.get_edge_table().at(ei);
    return r.a==sweep_loc ? h.origin : d.get_edge_table().at(h.twin).origin;
}


/*
 * Classify the origin vertex of edge with index eo in d.
 *
 * The argument eo is an edge index, not a vertex index, because the status
 * depends on the context, namely the in-edge and out-edge that you are
 * considering.  For example, take the top point of an star-shaped polygon
 * enclosed in a square.  It would be a merge vertex from the perspective of
 * inside the square, but a start vertex if looking from the inside of the
 * star.  With nonsimple polygons you can have multiple statuses for the same
 * vertex even from perspectives of the same face.  Consider a triangle
 * "stuck" by a corner to the inside wall of a square (like an east-pointing
 * arrowhead located at 3-o'clock on a square clock face).  The tip vertex
 * will appear as both start and finish vertices when monotonizing the rest
 * of the square.
 */
enum Emv vtx_status(size_t eo, const Doubly_connected_edge_list& d)
{
    const size_t ei = d.get_edge_table().at(eo).prev;
    const RatPoint_line_segment ri = get_edge(d, ei), ro = get_edge(d, eo);
    KJB(ASSERT(ri.b == ro.a));

    const bool distal_out_lower = ro.b < ro.a,
               distal_in_lower = ri.a < ri.b;

    enum Emv vstat = distal_out_lower ? PLAIN_L : PLAIN_R;

    if (distal_out_lower == distal_in_lower)
    {
        /*
         * Both distal ends go down (both booleans false),
         * or both go up (both booleans true).
         */
        if (triangle_area(ro, ri.a) > 0)
        {
            vstat = distal_out_lower ? FINISH : START;
        }
        else
        {
            vstat = distal_out_lower ? MERGE : SPLIT;
        }
    }
    // else, vi really is plain
    return vstat;
}


// line segment putter
std::string str(const RatPoint_line_segment& s)
{
    std::ostringstream os;
    os << '(' << s.a << ")-to-(" << s.b << ')';
    return os.str();
}


struct Edge_link : public kjb::qd::sweep::Event_point
{
    bool distal_up_;
    Edge_link(const Event_point& e, bool distal_up)
    :   Event_point(e),
        distal_up_(distal_up)
    {}
};


// return segments needed to triangulate a y-monotone face
std::vector<RatPoint_line_segment> edges_to_tri_ymonotone(
    const Doubly_connected_edge_list& dcel,
    size_t fi
)
{
    KJB(ASSERT(is_face_ymonotone(dcel, fi)));

    std::vector<RatPoint_line_segment> acc;

    std::vector< Edge_link > q, stk;
    for (std::vector<size_t> eee = get_incident_edges(dcel, fi);
            ! eee.empty(); eee.pop_back())
    {
        const RatPoint_line_segment s = get_edge(dcel, eee.back());
        q.push_back( Edge_link(kjb::qd::sweep::Event_point(s.a, eee.back()),
                        s.a < s.b) );
    }
    std::sort(q.begin(), q.end());

    // push first two vertices onto The Stack
    stk.push_back(q[0]);
    stk.push_back(q[1]);

    // process remaining vertices save the last
    // This is guided by the pseudocode of de Berg et al., sec. 3.3, p. 57.
    for (size_t i = 2; i+1 < q.size(); ++i)
    {
        if (stk.back().distal_up_ == q[i].distal_up_)
        {
            // de Berg et al. steps 8, 9 -- very hand-waving exposition!

            // This is sort of like Richard III destroying his enemies by
            // making a new ally, and together they destroy his previous ally.
            // Could make up a story around it.  Latest vertex q[i] is Richard,
            // x is the stale ally, and stack top is the new ally.  Imagine a
            // masonic Richard who disposes of his enemies as Montresor from
            // The Cask of Amontillado does, by immuring them in a triangle.
            KJB(ASSERT(! stk.empty()));
            Edge_link x = stk.back();
            stk.pop_back();

            // Loop to try to build as many new walls as we can on this side.
            // Sign of cross product flips between right wall, left wall.
            for ( RatPoint_line_segment new_wall(q[i].p,q[i].p);
                   ! stk.empty() // One endpoint must come from the stack,
                && 0 < triangle_area(//... and wall must stay wholly in-bounds.
                        (new_wall=RatPoint_line_segment(q[i].p, stk.back().p)),
                            x.p) * ( q[i].distal_up_ ? 1 : -1 );
                )
            {
                // Good for us:  we CAN build new_wall in-bounds and trap x.
                // x is doomed, cries out, "for the love of god, Montresor!"
                acc.push_back(new_wall); // in pace requiescat!
                // Forget old value now in x -- it is now safely walled off in
                // a triangle, like Fortunato in The Cask of Amontillado.
                // "New ally" in stack top now becomes the stale ally x.
                x = stk.back();
                stk.pop_back();
            }
            // The last x escapes because we cannot build any more walls.
            stk.push_back(x);
        }
        else
        {
            // de Berg et al. steps 5-7.

            // Vertex on opposite wall can "see" everyone on the stack.
            // Thus we can nearly empty the stack, except for the last vertex.
            for (size_t j = 1; j < stk.size(); ++j)
            {
                acc.push_back(RatPoint_line_segment(q[i].p, stk[j].p));
            }
            stk.clear();
            stk.push_back(q[i-1]);
        }
        // de Berg et al. steps 7 and 10 (factored out).
        stk.push_back(q[i]);
    }
    // de Berg et al. step 11
    for (size_t j = 1; j+1 < stk.size(); ++j)
    {
        acc.push_back(RatPoint_line_segment(q.back().p, stk[j].p));
    }

    return acc;
}


class Shemp_eq
:   public std::binary_function<
        kjb::qd::sweep::Shemp,
        kjb::qd::sweep::Shemp,
        bool >
{
    kjb::qd::sweep::Shemp v_;
public:
    Shemp_eq(const kjb::qd::sweep::Shemp& a) : v_(a) {}

    bool operator()(const kjb::qd::sweep::Shemp& b) const
    {
        return v_.shemp_index() == b.shemp_index();
    }
};



class Is_troublemaker : public std::unary_function<size_t, bool>
{
    const Doubly_connected_edge_list& dcel_;

public:
    Is_troublemaker(const Doubly_connected_edge_list& d)
    :    dcel_(d)
    {}

    // Input is an outgoing edge (in dcel_) of test vertex.
    // See vtx_status() comments if this seems weird.
    bool operator()(size_t eo) const
    {
        const enum Emv vstat = vtx_status(eo, dcel_);
        return MERGE == vstat || SPLIT == vstat;
    }
};



// Parameter facial_edges must refer to the output of get_incident_edges().
// Returns true iff face fi of DCEL dcel is y-monotone.
bool is_face_ymono_impl(
    const Doubly_connected_edge_list& dcel,
    size_t fi,
    const std::vector<size_t>& facial_edges
)
{
    if (0 == fi)
    {
        return 0 == dcel.get_edge_table().size();
    }
    if (dcel.get_face_table().size() <= fi)
    {
        KJB_THROW_2(kjb::Illegal_argument, "Face index is out of range");
    }
    if (! dcel.get_face_table().at(fi).inner_components.empty())
    {
        return false;
    }

    return std::find_if(facial_edges.begin(), facial_edges.end(),
                        Is_troublemaker(dcel)) == facial_edges.end();
}


class Is_face_3ang : public std::unary_function<size_t, bool>
{
    const Doubly_connected_edge_list& d_;
public:
    Is_face_3ang(const Doubly_connected_edge_list& d) : d_(d) {}
    bool operator()(size_t fi) const { return is_face_triangle(d_, fi); }
};


/// @pre face fi of DCEL d must really be a triangle.
RatPoint::Rat area_of_dcel_tri(const Doubly_connected_edge_list& d, size_t fi)
{
    KJB(ASSERT(is_face_triangle(d, fi)));

    const size_t a = d.get_face_table().at(fi).outer_component,
                 b = d.get_edge_table().at(a).next,
                 c = d.get_edge_table().at(b).next,
                 vc = d.get_edge_table().at(c).origin;

    return triangle_area(get_edge(d, a), d.get_vertex_table().at(vc).location);
}


/// @brief accumulator for areas of a series of triangles
class Add_area_3ang
:   public std::binary_function<RatPoint::Rat, size_t, RatPoint::Rat>
{
    const Doubly_connected_edge_list& d_;

public:
    Add_area_3ang(const Doubly_connected_edge_list& d) : d_(d) {}

    RatPoint::Rat operator() (RatPoint::Rat sum, size_t f) const
    {
        return sum + area_of_dcel_tri(d_, f);
    }
};


}


namespace kjb
{
namespace qd
{


bool is_face_ymonotone(const Doubly_connected_edge_list& dcel, size_t fi)
{
    return is_face_ymono_impl(dcel, fi, get_incident_edges(dcel, fi));
}


#if MONOTONE_CPP_DEBUG
/**
 * @brief return svg circles to label vertices as merge-type, etc.
 *
 * This does not return a complete SVG element, just a single group of circles.
 */
std::string db_fence_labels(
    const Doubly_connected_edge_list& dcel,
    size_t fi,
    float scale
)
{
    using kjb::qd::dbl_ratio;
    using kjb::qd::SVG_UNCRAMP;

    // Cannot do this for face zero.
    if (0 == fi) KJB_THROW(kjb::Illegal_argument);

    std::ostringstream s;
    s << "<g stroke=\"gray\" stroke-width=\"" << 2 * scale << "\">\n";

    for (std::vector<size_t> eee = get_incident_edges(dcel, fi);
            ! eee.empty(); eee.pop_back() )
    {
        const Doubly_connected_edge_list::Vertex_record &v
            = dcel.get_vertex_table().at(
                  dcel.get_edge_table().at(eee.back()).origin);

        s << "<circle cx=\"" << SVG_UNCRAMP * dbl_ratio(v.location.x)
          << "\" cy=\"" << SVG_UNCRAMP * dbl_ratio(v.location.y)
          << "\" r=\"" << 7 * scale << "\" fill=\"";

        switch (vtx_status(eee.back(), dcel))
        {
            case START:
                s << "green"; break;
            case FINISH:
                s << "blue"; break;
            case SPLIT:
                s << "lightgreen"; break; // similar to start
            case MERGE:
                s << "#77f"; break; // similar to finish
            case PLAIN_L:
                s << "#030"; break; // dim port
            case PLAIN_R:
                s << "#400"; break; // dim starboard
            case undefined:
            default:
                s << "red"; break; // trouble!
        }
        s << "\" />\n";
    }
    s << "</g>\n";

    return s.str();
}
#endif


/*
 * This implements the algorithm of Lee and Preparata to make a face
 * y-monotone.  They use a plane-sweep approach, from small y to larger y.
 * The sweep line is implemented using the machinery of sweep.h, built to
 * support the Bentley-Ottman algorithm (see intersection.h) but useful for
 * other sweep-based algorithms.  It's a little more abstract than required.
 *
 * This version replaces an old version that could not handle holes,
 * inner components, or "wire intrusions."  This version can, though it is
 * more complicated, since it has to cope with the idea that each vertex
 * incident to the face is a "nexus" of one or maybe more edge-pairs.
 * I'm sorry it's 250 lines but I think it's better this way.
 *
 * The central rationale for monotonizing is explained below in the extensive
 * comment for functor "helper_coup," which uses a humorous fantasy as an
 * illustration to make the explanation vivid.  It's a joke, don't be angry.
 */
std::vector< RatPoint_line_segment > edges_to_ymonotonize(
    const Doubly_connected_edge_list& d_in,
    size_t fi
)
{
    // Check for unacceptable face indices.
    if (0 == fi)
    {
        KJB_THROW_2(kjb::Illegal_argument,
                    "DCEL face 0 is unbounded and cannot be made monotone.");
    }
    ETX(is_valid(d_in));

    // Get list of edges touching face fi, and look for an easy exit.
    std::vector< RatPoint_line_segment > edges_out;
    const std::vector< size_t > eee = get_incident_edges(d_in, fi);
    if (is_face_ymono_impl(d_in, fi, eee)) return edges_out;

    // Create functor to fetch edges incident to face fi
    Dcel_subset def(&d_in, &eee);
    std::vector< size_t > helper(eee.size(),Doubly_connected_edge_list::BLANK);
    std::vector<bool> is_it_merge(d_in.get_vertex_table().size(), false);

#if MONOTONE_CPP_DEBUG
    std::cout << "face " << fi << "\nEdge indices in fence "
                "(size:" << eee.size() << "):\n";
    std::copy(eee.begin(), eee.end(),
        std::ostream_iterator<size_t>(std::cout, ","));
    std::cout << '\n';
    do { static int count = 0;
    char b[32];
    snprintf(b, 32, "etym-in-%03d.svg", count++);
    std::string facesvg = kjb::qd::draw_dcel_as_svg(d_in);
    size_t jj = facesvg.find("<text ");
    KJB(ASSERT(jj < facesvg.size()));
    jj = facesvg.rfind("<!--", jj);
    KJB(ASSERT(jj < facesvg.size()));
    facesvg.insert(jj, "<!-- fence labels -->\n" + db_fence_labels(d_in, fi, 1));
    std::ofstream fs(b);
    fs << facesvg << "\n<!-- face index fi = " << fi << " -->\n"; } while(0);
#endif

    // set up event queue and sweep line
    RatPoint_line_segment lbump(def.lookup(0)), rbump(lbump);
    std::set< kjb::qd::sweep::Event_p > q; // event queue
    do { std::vector<RatPoint_line_segment> sl(eee.size(), lbump);
        using namespace kjb::qd::sweep;
        for (size_t k=0; k < eee.size(); ++k) sl[k] = def.lookup(k);
        std::set<Event_p> qq = fill_queue_get_bumpers(sl, &lbump, &rbump);
        q.swap(qq);
    } while(0);
    RatPoint sweep_loc = (* q.begin()) -> p;
    Sweep::SL sweep(
        kjb::qd::sweep::SweepCompare<Dcel_subset>(def,&sweep_loc,lbump,rbump));
    do { using kjb::qd::sweep::Shemp;
        sweep.insert(Shemp(Shemp::LBUMP));
        sweep.insert(Shemp(Shemp::RBUMP));
    } while(0);
    std::vector< Sweep::iterator > rvs_sweep(eee.size(), sweep.end());

    /*
     * The following functor will (1) replace the helper of edge sj with vj;
     *  (2) potentially punish the old helper by sticking a wire in its face,
     *      which we always do to merge vertices, but not just to them.  We
     *      have no choice -- this is the only way to neutralize their threat.
     *
     * Merge and Split vertices threaten the monotonicity of our polygon.
     * They must be monitored until the time is ripe to strike.
     * Then, they must be ruthlessly neutralized.
     *
     * We track merge vertices by lulling them into a state of complacency,
     * calling them "helper" soothingly.  "We need you to HELP us, comrade!"
     * fawns the leftward segment, posing as a needy dependent.  Then, when it
     * is time for that segment either to retire or to embrace a new helper, it
     * strikes!  It stabs the old helper in the face with a wire, tying it
     * off so it can no longer merge the precious bodily fluids of two regions
     * that any right-thinking person knows should be separate.  No merge
     * vertex can escape this strategy -- it will eventually be claimed as a
     * helper and later will be stabbed in the face.  We shall prevail!
     * Sometimes even two merge vertices are bound together this way, ha ha.
     * (This is not the definition of helper, though -- see de Berg et al.)
     *
     * Split vertices are just as harshly dealt with, but we do so as soon
     * as we encounter them.  In an ironic move, we might wire them to a
     * merge vertex, cancelling both their threats.  In any case we wire them
     * to the left edge's helper.  (That means sometimes an innocent vertex
     * is called upon by its polygon to make the ultimate sacrifice, in order
     * to guarantee the monotonicity of the faces of the planar partition.)
     *
     * This lunatic narrative describes the whole logic of the sweep algorithm,
     * not just the helper-replacement operation.   Don't take it seriously,
     * it is inspired by Dr. Strangelove.
     * (Do you recall what Clemenceau once said about geometry?)
     *
     * I realize it is unusual to make this a functor of an anonymous class,
     * but by doing so, we reduce the number of function-operator parameters
     * from seven down to three.  Seven to three:  think about that, Mandrake.
     * Note that several times we call this functor not because it replaces
     * the helper but because it performs the neutralize step.  Specifically,
     * this happens when we replace the helper of a segment that is about to
     * be deleted.  This is admittedly unnecessary but perfectly harmless.
     */
    struct
    {
        const RatPoint* const sweep_loc_;
        std::vector< size_t >* const helper_;
        const Doubly_connected_edge_list* const d_in_;
        std::vector< RatPoint_line_segment >* const out_;

        void operator()(size_t sj, size_t vj, bool neutralize_old_helper)
        {
            const size_t vh = helper_ -> at(sj);
            KJB(ASSERT(vh < Doubly_connected_edge_list::BLANK));
            if (neutralize_old_helper)
            {
                // Queue a wire, t, from old helper vertex vh to vertex vj.
                out_ -> push_back(
                    RatPoint_line_segment(*sweep_loc_,
                        d_in_-> get_vertex_table().at(vh).location));
            }
            helper_ -> at(sj) = vj;
        }
    }
    helper_coup = {&sweep_loc, &helper, &d_in, &edges_out};

#if MONOTONE_CPP_DEBUG
    do { std::cout << "Initial event queue:\n";
    const std::vector<kjb::qd::sweep::Event_p> iiq(q.begin(), q.end());
    for (size_t i=0; i<iiq.size(); ++i)
        std::cout << "Index:" << iiq[i]->index << "\tLoc:" << iiq[i]->p <<'\n';
    } while(0);
#endif

    // process the event queue -- main loop of vertical sweep
    while (! q.empty())
    {
        // Code here is similar to that of scan_for_roof() in dcel.cpp
        sweep_loc = (* q.begin()) -> p; // update the sweep location
        std::set<size_t> ulma[4]; // [0]=upper, [1]=lower, [2]=middle, [3]=all

        // Collect all edges incident to the current sweep location.
        kjb::qd::sweep::pull_events_here(&q, sweep, ulma, def);
        KJB(ASSERT(ulma[2].empty())); // middle set is always empty
        KJB(ASSERT(!ulma[3].empty())); // "all" set is never empty
        // cheapo partial check that uppers and lowers are a partition of all
        // which is another way of saying "no degenerate segments"
        KJB(ASSERT(ulma[0].size() + ulma[1].size() == ulma[3].size()));

        // Figure out vertex index vi of the vertex we have struck.
        const size_t vi = get_vertex(* ulma[3].begin(), sweep_loc, d_in, eee);
#ifdef DEBUGGING
        const std::vector<size_t> check_all(ulma[3].begin(), ulma[3].end());
        for (size_t i = 0; i < check_all.size(); ++i)
        {
            // verify all edges we collected "know" they are incident to vi
            KJB(ASSERT(get_vertex(check_all[i], sweep_loc, d_in, eee) == vi));
        }
#endif

#if MONOTONE_CPP_DEBUG
        std::cout << "Nexus at location " << sweep_loc << ", index " << vi
                    << ": (fence indices)\n";
        std::copy(ulma[3].begin(), ulma[3].end(),
            std::ostream_iterator<size_t>(std::cout, " "));
        std::cout << '\n';
#endif

        // enumerate SL indices of all out-edge, in-edge pairs inc. to vi
        std::vector<size_t> nexus = get_nexus(vi, ulma[3], d_in, eee);
        KJB(ASSERT(0 == nexus.size() % 2));
        KJB(ASSERT(nexus.size() == ulma[3].size()));

        // Process each in-edge, out-edge pair incident to vertex vi.
        while (! nexus.empty())
        {
            /* Edges are identified either by DCEL edge table index (could be a
             * big number) or by sweepline index.  In-edge has sweepline index
             * si, out-edge has sweepline index so.  Array eee[] maps sweepline
             * index to DCEL edge table index.  We don't store a full reverse
             * table because it could be quite large.
             */
            const size_t si = nexus.back(); // inward halfedge
            nexus.pop_back();
            KJB(ASSERT(! nexus.empty()));
            const size_t so = nexus.back(); // outward halfedge
            nexus.pop_back();
            const size_t ei = eee[si], eo = eee[so];

            // Sanity-check in-edge index ei, out-edge index eo.
            KJB(ASSERT(d_in.get_edge_table().at(ei).next == eo));
            KJB(ASSERT(d_in.get_edge_table().at(eo).prev == ei));
            KJB(ASSERT(d_in.get_edge_table().at(eo).origin == vi));
            KJB(ASSERT(d_in.get_edge_table().at(
                           d_in.get_edge_table().at(ei).twin
                                ).origin == vi));
            KJB(ASSERT(d_in.get_edge_table().at(ei).face == fi));
            KJB(ASSERT(d_in.get_edge_table().at(eo).face == fi));

            // What "kind" of vertex is vi with respect to edges ei, eo?
            const enum Emv vstat = vtx_status(eo, d_in);
            KJB(ASSERT(     START == vstat
                        ||  SPLIT == vstat
                        ||  FINISH == vstat
                        ||  MERGE == vstat
                        ||  PLAIN_L == vstat
                        ||  PLAIN_R == vstat
                    ));

#if MONOTONE_CPP_DEBUG
            std::cout << "Nexus pair (dcel indices): " << ei << ", " << eo
                << "; segments " << str(def.lookup_no_rectify(si)) << " and "
                << str(def.lookup_no_rectify(so))
                << "; status " << vstat
                << " (1=Sta,2=End,3=Spl,4=Mrg,5=PR,6=PL)\n";
#endif

            // si will be inserted for START, SPLIT, and PLAIN on left side.
            // ulma[0] lists the to-be-inserted edges.
            KJB(ASSERT(     FINISH==vstat || MERGE==vstat || PLAIN_R==vstat
                        ||  ulma[0].find(si) != ulma[0].end()));
            // 'so' will be deleted for FINISH, MERGE, and PLAIN on left side.
            // ulma[1] lists the to-be-deleted edges.
            KJB(ASSERT(     START==vstat || SPLIT==vstat || PLAIN_R==vstat
                        ||  ulma[1].find(so) != ulma[1].end()));

            // Unfortunately si is scheduled to be deleted for FINISH, MERGE,
            // and PLAIN on right side.  We will suppress this later.
            KJB(ASSERT(     START==vstat || SPLIT==vstat || PLAIN_L==vstat
                        ||  ulma[1].find(si) != ulma[1].end()));
            // Unfortunately 'so' is scheduled to be inserted for START, SPLIT,
            // and PLAIN on the right side.  We will suppress this later.
            KJB(ASSERT(     FINISH==vstat || MERGE==vstat || PLAIN_L==vstat
                        ||  ulma[0].find(so) != ulma[0].end()));

            if (START == vstat)
            {
                // Edge 'si' just appeared, so vi is its default helper.
                helper[si] = vi;
            }
            else if (FINISH == vstat)
            {
                // A retiring edge (like 'so') must "take care of" its helper,
                // if the helper is a merge vertex.  I.e., neutralize it.
                // There's no need to update helper[so]=vi but it is harmless.
                helper_coup(so, vi, is_it_merge[helper[so]]);
            }
            else if (MERGE == vstat)
            {
                // Mark this one as a troublemaker.
                is_it_merge.at(vi) = true;
                // Simutaneously, edge 'so' is retiring, just like a FINISH.
                helper_coup(so, vi, is_it_merge[helper[so]]);
                // Edge sj views vi from the left:  vi is its new helper.
                const size_t sj = left_bystander(sweep, sweep_loc, def);
                // Of course sj might need to neutralize its old helper if
                // the old helper was a troublemaker.
                helper_coup(sj, vi, is_it_merge[helper[sj]]);
            }
            else if (SPLIT == vstat)
            {
                // Edge sj views vi from the left.  vi will be its new helper.
                const size_t sj = left_bystander(sweep, sweep_loc, def);

                /* The old and new helpers can see each other, by construction.
                 * We can immediately neutralize this new troublemaker by
                 * attaching the helpers together (thus the 'true' argument).
                 * The old helper might not be a troublemaker: we do not care.
                 */
                helper_coup(sj, vi, true);
                helper[si] = vi;
            }
            else if (PLAIN_L == vstat)
            {
                // Face is locally to the right.
                // Edge 'so' is retiring -- behave similarly to FINISH case.
                helper_coup(so, vi, is_it_merge[helper[so]]);
                // Edge 'si' is starting -- behave similarly to START case.
                helper[si] = vi;
            }
            else
            {
                KJB(ASSERT(PLAIN_R == vstat));
                // Face is locally to the left.
                const size_t sj = left_bystander(sweep, sweep_loc, def);

                /* Edge sj views vi from the left.  vi will be its new helper.
                 * This case is similar to SPLIT except that vi is not a
                 * troublemaker, so we only neutralize the old helper if the
                 * old helper deserves it.
                 */
                helper_coup(sj, vi, is_it_merge[helper[sj]]);
            }

            /* Suppression.  Since we only ever query for left_bystander and
             * never for the right, we do not need right-edges in the sweep
             * line.  Here we scrub right edges from the insertion list
             * ulma[0], and we also scrub them from the deletion list ulma[1]
             * since the deletion could not succeed since you can't delete
             * something that was never there (it would be like delivering
             * a pardon to a prison inmate who turns out to have escaped --
             * an awkward situation).
             *
             * Suppression is not necessary with simple polygons, but it is
             * necessary in a general DCEL planar partition because they can
             * have "floating wires" and the like, where the right and left
             * half-edges are colocated.  A simple sweep line query could turn
             * up either one first.  If you get the wrong one, it spoils the
             * algorithm's correctness.  Thus you are obliged either to
             * complicate the sweep line query (testing for, and throwing away,
             * the right-bystander edges), or to suppress the right-bystander
             * edges before they reach the sweep line, as we do here.
             */
            if (START == vstat || PLAIN_R == vstat || SPLIT == vstat)
            {
                // Remove 'so' from list of segments to be inserted into sweep.
                ulma[0].erase(so);
            }
            if (MERGE == vstat || PLAIN_R == vstat || FINISH == vstat)
            {
                // More suppression.
                // si is not even in the sweep line, right?
                KJB(ASSERT(sweep.end()==std::find_if(sweep.begin(),sweep.end(),
                    Shemp_eq(kjb::qd::sweep::Shemp::ctor_from_true(si)))));
                // Remove si from list of segments we try to remove from sweep.
                ulma[1].erase(si);
            }
        }

        // Delete the retiring edges from sweep line, insert the starting ones.
        kjb::qd::sweep::update_sweep< Dcel_subset >(&sweep, ulma, &rvs_sweep);

#if MONOTONE_CPP_DEBUG
        std::cout << "Sweep line :";
        std::copy(sweep.begin(), sweep.end(),
            std::ostream_iterator<kjb::qd::sweep::Shemp>(std::cout, " : "));
        std::cout << "\nHelpers (slix,vx):";
        for (size_t i = 0; i < helper.size(); ++i)
            if (helper[i] < Doubly_connected_edge_list::BLANK)
                std::cout << " (" << i << ',' << helper[i] << ')';
        std::cout << '\n';
#endif
    }

    return edges_out;
}



Doubly_connected_edge_list make_a_face_ymonotone(
    const Doubly_connected_edge_list& dcel,
    size_t fi
)
{
    const std::vector< RatPoint_line_segment > e=edges_to_ymonotonize(dcel,fi);
    return ctor_from_edge_list(e).merge(dcel);
}


Doubly_connected_edge_list make_faces_ymonotone(
    const Doubly_connected_edge_list& dcel
)
{
    std::vector< RatPoint_line_segment > acc;

    for (size_t i = 1; i < dcel.get_face_table().size(); ++i)
    {
        const std::vector<RatPoint_line_segment>e=edges_to_ymonotonize(dcel,i);
        std::copy(e.begin(), e.end(), std::back_inserter(acc));
    }

    return ctor_from_edge_list(acc).merge(dcel);
}


/**
 * @brief return list of edges needed to triangulate a face
 * @param dcel   Planar subdivision, a superset of the face to triangulate
 * @param fi     Index of face of dcel to triangulate -- must be positive.
 *
 * The algorithm is by Garey, Johnson, Preparata and Tarjan, "Triangulating a
 * simple polygon," (Info. Proc. Lett., 7:175-179, 1978).  My implementation
 * is based on the exposition in de Berg et al., Computational Geometry/2e
 * (Springer, 2000).
 */
std::vector< RatPoint_line_segment > edges_to_triangulate(
    const Doubly_connected_edge_list& dcel,
    size_t fi
)
{
    /*
     * The implementation is in two steps.  First we make the face y-monotone.
     * This subdivides dcel face fi into y-monotone subfaces s1, s2, ..., sn.
     * Possibly n=1, because fi might already be y-monotone.
     * Then we triangulate the subfaces.  This function creates the
     * subfaces, and function edges_to_tri_ymonotone() triangulates them.
     */
    Doubly_connected_edge_list d2(dcel);
    std::vector< RatPoint_line_segment > acc;
    // list of y-monotone faces to be triangulated:
    std::vector< size_t > ym_faces_to_triangulate(1, fi);

    // monotonize, if necessary
    if (! is_face_ymonotone(dcel, fi))
    {
        acc = edges_to_ymonotonize(dcel, fi);
        KJB(ASSERT(! acc.empty()));
        d2 = ctor_from_edge_list(acc).merge(dcel);

        // build list of the subfaces
        std::set<size_t> subfaces;
        for (size_t i = 0; i < acc.size(); ++i)
        {
            size_t ej = lookup_edge(d2, acc[i]);
            KJB(ASSERT(ej < d2.get_edge_table().size()));
            size_t fj = d2.get_edge_table().at(ej).face,
                   ejt = d2.get_edge_table().at(ej).twin,
                   fjt = d2.get_edge_table().at(ejt).face;
            KJB(ASSERT(fj != fjt));
            KJB(ASSERT(is_face_ymonotone(d2, fj)));
            KJB(ASSERT(is_face_ymonotone(d2, fjt)));
            subfaces.insert(fj);
            subfaces.insert(fjt);
        }
        ym_faces_to_triangulate.clear();
        std::remove_copy_if(subfaces.begin(), subfaces.end(),
            std::back_inserter(ym_faces_to_triangulate), Is_face_3ang(d2));
    }

    // triangulate the y-monotone faces
    for (;!ym_faces_to_triangulate.empty(); ym_faces_to_triangulate.pop_back())
    {
        const std::vector<RatPoint_line_segment>
            v = edges_to_tri_ymonotone(d2, ym_faces_to_triangulate.back());
        std::copy(v.begin(), v.end(), std::back_inserter(acc));
    }

    return acc;
}


bool is_face_triangle(const Doubly_connected_edge_list& d, size_t fi)
{
    // Face must be bounded (unlike face zero), with no junk inside it.
    if (    0 < fi && fi < d.get_face_table().size()
        &&  d.get_face_table().at(fi).inner_components.empty() )
    {
        // get the three hypothetical edge indices (triangle legs)
        const size_t a = d.get_face_table().at(fi).outer_component,
                     b = d.get_edge_table().at(a).next,
                     c = d.get_edge_table().at(b).next;

        // Does the cycle close after these three?
        return a == d.get_edge_table().at(c).next;
    }
    return false;
}



/**
 * @brief compute the area of any finite face of a DCEL
 */
RatPoint::Rat area_of_face(const Doubly_connected_edge_list& dcel, size_t fi)
{
    // Heuristic speedup:  quick exit for simple input
    if (is_face_triangle(dcel, fi)) return area_of_dcel_tri(dcel, fi);

    // Triangulate the input via list of cutting edges e.
    const std::vector<RatPoint_line_segment> e = edges_to_triangulate(dcel,fi);
    const Doubly_connected_edge_list d2 = ctor_from_edge_list(e).merge(dcel);

    // Build list of the faces in the triangular partition.
    std::set<size_t> tri_faces;
    for (size_t i = 0; i < e.size(); ++i)
    {
        size_t ej = lookup_edge(d2, e[i]);
        KJB(ASSERT(ej < d2.get_edge_table().size()));
        size_t fj = d2.get_edge_table().at(ej).face,
               ejt = d2.get_edge_table().at(ej).twin,
               fjt = d2.get_edge_table().at(ejt).face;
        KJB(ASSERT(fj != fjt));
        KJB(ASSERT(is_face_triangle(d2, fj)));
        KJB(ASSERT(is_face_triangle(d2, fjt)));
        tri_faces.insert(fj);
        tri_faces.insert(fjt);
    }

    // Add the areas of those triangles.
    return std::accumulate(tri_faces.begin(), tri_faces.end(),
                            RatPoint::Rat(0), Add_area_3ang(d2));
}


}
}

