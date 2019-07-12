/**
 * @file
 * @brief Implementation of doubly-connected edge list class
 * @author Andrew Predoehl
 *
 * Implementation note:  the DCEL sometimes has its edge next and prev fields
 * broken across various function calls.
 *
 * Remember as you read this code that I am presupposing a Cartesian layout
 * everytime I say "left" or "right" or "clockwise" or "counterclockwise."
 * If you visualize things with a conventional graphics or matrix layout,
 * these terms reverse their senses.
 */
/*
 * $Id: dcel.cpp 21596 2017-07-30 23:33:36Z kobus $
 */

#include "l/l_sys_debug.h"  /* For ASSERT */
#include "qd_cpp/dcel.h"
#include "qd_cpp/sweep.h"
#include "qd_cpp/intersection.h"

#include <boost/foreach.hpp>
#include <boost/property_tree/xml_parser.hpp>

#if 0 /* this symbol is being retired */
/*
 * The merge routine must find line segment intersections amongst the DCELS
 * to be merged.  If the number of intersections is "asymptotically" less than
 * the number of edges, it can be faster to use the Bentley-Ottman line segment
 * intersection algorithm, implemented in qd_cpp.  In complicated
 * configurations with "dense" intersections, a brute-force approach is
 * sometimes faster (by a constant factor -- no asymptotic advantage).
 * This behavior has not been characterized empirically.
 */
#define USE_BENTLEY_OTTMAN 1
#endif

#include <set>
#include <map>
#include <algorithm>

#ifdef DEBUGGING
#include <fstream>
#endif

#define XML_TAG_DCEL "dcel"
#define XML_TAG_RATIONAL "rational"
#define XML_TAG_NUMERATOR "numerator"
#define XML_TAG_DENOMINATOR "denominator"
#define XML_TAG_OUTEDGE "outedge"
#define XML_TAG_LOCATION "location"
#define XML_TAG_X "x"
#define XML_TAG_Y "y"
#define XML_TAG_ORIGIN "origin"
#define XML_TAG_TWIN "twin"
#define XML_TAG_INCFACE "incface"
#define XML_TAG_NEXT "next"
#define XML_TAG_PREV "prev"
#define XML_TAG_OCOMPONENT "ocomponent"
#define XML_TAG_ICOMPONENTS "icomponents"
#define XML_TAG_EDGEINDEX "edgeindex"
#define XML_TAG_VERTEX "vertex"
#define XML_TAG_EDGE "edge"
#define XML_TAG_FACE "face"
#define XML_TAG_VERTICES "vertices"
#define XML_TAG_EDGES "edges"
#define XML_TAG_FACES "faces"

/*
 * This puts an XML element into ostream s, using tag t, value v, attribute a.
 * a and v must be "puttable."  t is expected to expand to a C-string literal,
 * namely one of the above XML_TAG_xxx macros.  Because we are using macros,
 * v can be a "puttable expression" like 1<<"cat"<<2<<"dogs" or similar.
 *
 * Annoying extra requirement:  either a should expand to the empty string or
 * it should expand to a string whose first character is a space.  If you
 * forget the space, the attribute will not be separated from the tag.
 * I am not willing to stuff a space in there all the time, that's ugly.
 */
#define BUILD_XML_ATTR_ELT(s,t,a,v) s <<"<" t << a <<">" << v <<"</" t ">\n"

// This does the same thing without any attribute
#define BUILD_XML_ELT(s,t,v) BUILD_XML_ATTR_ELT(s,t,"",v)


namespace
{
using kjb::qd::Doubly_connected_edge_list;
using kjb::qd::RatPoint;
using kjb::qd::RatPoint_line_segment;


// This routine is slow but safe to call even when dcel is temporarily broken,
// specifically, when the edge table's next and prev fields might be wrong.
// Consequently it might have to search through the whole edge table.
// TODO:  reduce or eliminate usage of this function
std::vector< size_t > safe_out_edges(
    const Doubly_connected_edge_list& dcel,
    size_t vix
)
{
    std::vector< size_t > oeix;
    for (size_t i = 0; i < dcel.get_edge_table().size(); ++i)
    {
        if (dcel.get_edge_table().at(i).origin == vix)
        {
            oeix.push_back(i);
        }
    }
    return oeix;
}


std::set< RatPoint > vertices_valid(const Doubly_connected_edge_list& dcel)
{
    std::set< RatPoint > vertices;
    const size_t nv = dcel.get_vertex_table().size();
    for (size_t i = 0; i < nv; ++i)
    {
        const Doubly_connected_edge_list::Vertex_record &v
            = dcel.get_vertex_table().at(i);
        if (!  vertices.insert(v.location).second)
        {
            KJB(ASSERT(vertices.size() == i));
            kjb_c::set_error("Duplicate vertex location in vertex %u", i);
            vertices.clear();
            break;
        }
        KJB(ASSERT(vertices.size() == 1+i));
        if (v.outedge >= dcel.get_edge_table().size())
        {
            kjb_c::set_error("Invalid outedge number in vertex %u", i);
            vertices.clear();
            break;
        }
        if (dcel.get_edge_table().at(v.outedge).origin != i)
        {
            kjb_c::set_error("Outedge edge_%u of vertex_%u claims to have "
                    "origin vertex_%u", v.outedge, i,
                    dcel.get_edge_table().at(v.outedge).origin);
            vertices.clear();
            break;
        }
    }
    return vertices;
}


bool edges_valid_partial(
    const Doubly_connected_edge_list& dcel
)
{
    for (size_t i = 0; i < dcel.get_edge_table().size(); ++i)
    {
        // check internal invariants of the bookkeeping
        const Doubly_connected_edge_list::Edge_record &e
            = dcel.get_edge_table().at(i);
        if (e.origin >= dcel.get_vertex_table().size())
        {
            kjb_c::set_error("Invalid vertex index in edge origin %u",i);
            return false;
        }
        if (e.twin >= dcel.get_edge_table().size())
        {
            kjb_c::set_error("Out-of-range twin field %u at edge %u",e.twin,i);
            return false;
        }
        if (dcel.get_edge_table().at(e.twin).twin != i)
        {
            kjb_c::set_error("Irreflexive twin indices at edge %u", i);
            return false;
        }
        if ( dcel.get_edge_table().at(e.twin).origin == e.origin)
        {
            // not sure if this is possible
            kjb_c::set_error("Dup. origin, destination at edge %u", i);
            return false;
        }
    }
    return true;
}


bool edges_valid(
    const Doubly_connected_edge_list& dcel
)
{
    using kjb::qd::PixPoint;
    std::set< PixPoint > prenex, orides;
    std::multimap< size_t, size_t > leaders, followers;
    std::vector< RatPoint_line_segment > segs;
    for (size_t i = 0; i < dcel.get_edge_table().size(); ++i)
    {
        // check internal invariants of the bookkeeping
        const Doubly_connected_edge_list::Edge_record &e
            = dcel.get_edge_table().at(i);
#if 0
        if (e.origin >= dcel.get_vertex_table().size())
        {
            kjb_c::set_error("Invalid vertex index in edge origin %u",i);
            return false;
        }
        if (dcel.get_edge_table().at(e.twin).twin != i)
        {
            kjb_c::set_error("Irreflexive twin indices at edge %u", i);
            return false;
        }
        const size_t dest = dcel.get_edge_table().at(e.twin).origin;
        if (dest == e.origin)
        {
            // not sure if this is possible
            kjb_c::set_error("Dup. origin, destination at edge %u", i);
            return false;
        }
#else
        if (!edges_valid_partial(dcel)) return false;
        const size_t dest = dcel.get_edge_table().at(e.twin).origin;
#endif

        leaders.insert(std::make_pair(e.next, i));
        followers.insert(std::make_pair(e.prev, i));

        if (dcel.get_edge_table().at(e.next).prev != i)
        {
            kjb_c::set_error("Bad next index at edge %u", i);
            return false;
        }
        if (dcel.get_edge_table().at(e.prev).next != i)
        {
            kjb_c::set_error("Bad prev index at edge %u", i);
            return false;
        }
        KJB(ASSERT(prenex.size() == i && orides.size() == i));
        if (! prenex.insert(PixPoint(e.prev, e.next)).second)
        {
            KJB(ASSERT(prenex.size() == i));
            kjb_c::set_error("Dup next/prev indices at edge %u", i);
            return false;
        }
        if (! orides.insert(PixPoint(e.origin, dest)).second)
        {
            KJB(ASSERT(orides.size() == i));
            kjb_c::set_error("Dup origin/destination at edge %u", i);
            return false;
        }
        KJB(ASSERT(prenex.size() == 1+i && orides.size() == 1+i));

        // look for crossings
        const RatPoint_line_segment si = get_edge(dcel, i);
#if 0
        // check for vertices splitting edges
        for (size_t j = 0; j < dcel.get_vertex_table().size(); ++j)
        {
            const RatPoint v = dcel.get_vertex_table().at(j).location;
            if (v == si.a)
            {
                // origin is ok
                if (e.origin == j) continue;
                kjb_c::set_error("Edge %u has extra origin %u", i, j);
                return false;
            }
            else if (v == si.b)
            {
                // destination is ok
                if (dest == j) continue;
                kjb_c::set_error("Edge %u has extra destination %u", i, j);
                return false;
            }
            else if (is_on(si, v))
            {
                // interior is not ok
                kjb_c::set_error("Vertex %u splits edge %u", j, i);
                return false;
            }
        }

        // check for interior edge intersection
        RatPoint_line_segment sx(si);
        for (size_t j = i+1; j < dcel.get_edge_table().size(); ++j)
        {
            if (j == e.twin) continue; // twins are allowed to intersect, duh
            const RatPoint_line_segment sj = get_edge(dcel, j);
            if (! segment_intersection(si, sj, &sx)) continue; // disjoint
            /*
             * I do believe we have by now ruled out the possibility that
             * parallel segments overlap, because that would cause a vertex to
             * touch an edge interior, which we detected earlier.
             */
            KJB(ASSERT(is_degenerate(sx)));
            const bool hits_interior_i = sx.a != si.a && sx.a != si.b,
                       hits_interior_j = sx.a != sj.a && sx.a != sj.b;
            if (hits_interior_i && hits_interior_j)
            {
                kjb_c::set_error("Unmarked edge crossing "
                                 "between edges %u, %u", i, j);
                return false;
            }
            // We already detected vertex-edge splits; therefore,
            KJB(ASSERT(hits_interior_i == hits_interior_j));
        }
#else
        if (i < e.twin) segs.push_back(si);
#endif
    }

    // Check leaders and followers -- 10 Oct 2015.
    // I am skeptical this test has any power, but I'm not certain it doesn't.
    while (!leaders.empty())
    {
        const size_t key = leaders.begin() -> first,
                     n = leaders.count(key);
        if (n != 1)
        {
            kjb_c::set_error("Edge %u has %u predecessors", key, n);
            return false;
        }
        leaders.erase(key);
    }
    while (!followers.empty())
    {
        const size_t key = followers.begin() -> first,
                     n = followers.count(key);
        if (n != 1)
        {
            kjb_c::set_error("Edge %u has %u successors", key, n);
            return false;
        }
        followers.erase(key);
    }

    // Use Bentley-Ottman to check for intersections.
    // Of course the endpoints will intersect.  If that is all, it's fine.
    // If any interiors intersect, return false.
    if (! get_interior_intersections(segs).empty())
    {
        kjb_c::set_error("segment error (overlap)");
        return false;
    }

    return true;
}





#if 0
// Obsolete -- roof table replaces this information

/* This computes argmax origin(e) over all edges e on the same cycle with ei.
 * Return value is an edge index, ej, such that cycle(ei) = cycle(ej), and
 * location(origin(edge[ej])) is larger, using operator< on RatPoint, than any
 * other edge in the same cycle.
 * Time complexity:  length of the cycle
 */
size_t get_edge_index_with_max_origin_of_cycle(
    const Doubly_connected_edge_list& d,
    size_t ei
)
{
    RatPoint maxo
        = d.get_vertex_table().at(d.get_edge_table().at(ei).origin).location;
    size_t imax = ei;

    // cycle-scan idiom
    for ( size_t ej = d.get_edge_table().at(ei).next; ej != ei;
                 ej = d.get_edge_table().at(ej).next )
    {
        const RatPoint& p
        = d.get_vertex_table().at(d.get_edge_table().at(ej).origin).location;
        if (maxo < p)
        {
            maxo = p;
            imax = ej;
        }
    }

    return imax;
}
#endif



/* Given a DCEL and an edge index, and edge-to-cycle map, return true IFF the
 * cycle of edge# ei runs counterclockwise around a face with positive area.
 * Canonical example:  picture on a clock face a vertex at the center, and
 * an outer-border cycle going CCW through it, say from the "5" to center,
 * then center to "7."   The clock-center is the peak vertex so we make the
 * key computation there.  Triangle 5-center-7 has positive area so the cycle
 * is identified an outer border provided one further condition is met.
 *
 * The further condition is that we must make sure the same cycle does not
 * pass through this peak vertex going clockwise too.  This is possible!
 * Imagine a hole comprising two thin triangles, occupying center-8-7 and
 * center-5-4.  The hole inner boundary could run 5-center-7-8-center-4-5.
 * Depending on which out-edge we get from the center, we might test the
 * triangle 5-center-7 (positive area) or 8-center-4 (negative area).
 * That is why we cannot simply judge by triangle area at the cycle peak of
 * one out-edge and its predecessor.
 *
 * Do not think that an outer border necessarily only passes through the peak
 * vertex once.  It can pass through the peak vertex an arbitrary number of
 * times:  imagine the outer border of triangle 4-center-8-4 except that there
 * are an arbitrary number of short edges radiating down partway from the
 * clock center, like rays of light shining down.
 *
 * But, every time the cycle passes through the peak vertex, it must go CCW
 * each time (i.e., make a left turn).  Because if it ever did not, it would
 * not have its face on its left anymore, and the outer boundary always has its
 * bounded face to its left.
 *
 * So, practically, what we do at the peak vertex is find all the out-edges,
 * and keep all the out-edges with the same cycle as that of edge# ei.
 * We test the triangle it forms with its predecessor.  If they are all
 * positive, the cycle forms an outer border.  If any is zero or negative,
 * it's an inner-component cycle.  Zero corresponds to a retroflex transit,
 * which can occur when the peak vertex is, e.g., stuck on an upward-pointing
 * edge like a little antenna on top of a car.  Which makes us return false.
 *
 * Related function:  is_edge_of_stick_figure
 */
bool is_on_outer_border(
    const Doubly_connected_edge_list& d,
    size_t ei
)
{
    const int c = d.get_cycle(ei);
#if 0
    const size_t imax = get_edge_index_with_max_origin_of_cycle(d, ei),
                 vmax = d.get_edge_table().at(imax).origin; // peak vertex
#else
    // We can use the new machinery here.
    const size_t vmax = d.get_top_vertex_of_cycle(c);
#endif

    // Get out-edges at peak, then remove those not part of cycle c.
    std::vector< size_t > oes( out_edges(d, vmax) ); // out-edges at peak
    for (size_t i = 0; i < oes.size(); ++i)
    {
        while (i < oes.size() && d.get_cycle(oes.at(i)) != c)
        {
            std::swap(oes.at(i), oes.back());
            oes.pop_back();
        }
    }
    /*KJB(ASSERT(std::find(oes.begin(), oes.end(), imax) != oes.end()));*/
    KJB(ASSERT(oes.size() > 0));

    // Check direction of each transit of cycle c through vertex vmax.
    // Outer-border = always counterclockwise
    // Inner-boundary = at least one is either clockwise or retroflex
    for (size_t i = 0; i < oes.size(); ++i)
    {
        const size_t ej = oes.at(i);
        const RatPoint_line_segment eo_max = get_edge(d, ej),
                                    ei_max = get_edge(d,
                                             d.get_edge_table().at(ej).prev);

        KJB(ASSERT(d.get_edge_table().at(ej).origin == vmax));
        KJB(ASSERT(eo_max.a == d.get_vertex_table().at(vmax).location));
        KJB(ASSERT(!is_degenerate(eo_max) && !is_degenerate(ei_max)));
        KJB(ASSERT(eo_max.a == ei_max.b));

        // Find the "signed area"; sign reveals CW or CCW order of vertices.
        // Negative means clockwise, which implies it is not an outer border.
        // Zero means retroflex, e.g., on a clock, to go 6-center-6; not outer.
        if (triangle_area(eo_max, ei_max.a) <= 0) return false;
    }
    return true; // ALL passes of cycle c through vmax go counterclockwise!
}




// mapping from cycle number to face index
typedef std::map< int, size_t > Facemap;


bool check_face_components(
    const Doubly_connected_edge_list& dcel,
    Facemap *facemap // cycle-id to face-ix map
)
{
    NTX(facemap);
    for (size_t i = 0; i < dcel.get_face_table().size(); ++i)
    {
        const Doubly_connected_edge_list::Face_record& f
            = dcel.get_face_table().at(i);

        // check outer component, except for index-0 face which has none
        if (i > 0)
        {
            const size_t ie = f.outer_component;
            Facemap::iterator j = facemap -> find(dcel.get_cycle(ie));
            if (facemap -> end() == j)
            {
                kjb_c::set_error("bad cycle on face %u", i);
                return false;
            }
            if (j -> second != i)
            {
                kjb_c::set_error("Bad outer component identity of face %u", i);
                return false;
            }
            if (! is_on_outer_border(dcel, ie))
            {
                kjb_c::set_error("Face %u outer component %u runs clockwise, "
                    "but outer components must run counterclockwise).", i, ie);
                return false;
            }
            facemap -> erase(j);
        }

        // check inner components
        for (size_t j = 0; j < f.inner_components.size(); ++j)
        {
            const size_t ie = f.inner_components.at(j);
            Facemap::iterator k;
            if ((k=facemap -> find(dcel.get_cycle(ie))) == facemap -> end())
            {
                kjb_c::set_error("bad cycle on face %u", i);
                return false;
            }
            if (k -> second != i)
            {
                kjb_c::set_error("Bad inner cycle identity %u of face %u",j,i);
                return false;
            }
            if (is_on_outer_border(dcel, ie))
            {
                kjb_c::set_error("Face %u inner cycle %u runs counterclockwise"
                            ", but inner cycles must run clockwise).", i, ie);
                return false;
            }
            facemap -> erase(k);
        }
    }

    // That process should have exhausted the cycle-to-face list
    if (!facemap -> empty())
    {
        kjb_c::set_error("Cycle number %d is not mentioned by face %u.  "
             "There is/are %u unmentioned cycle(s).",
             facemap -> begin() -> first, facemap -> begin() -> second,
             facemap -> size());
        return false;
    }

    return true;
}


// squared distance
inline RatPoint::Rat distance2(const RatPoint& p, const RatPoint& q)
{
    const RatPoint::Rat dx = p.x - q.x, dy = p.y - q.y;
    return dx*dx + dy*dy; // squared hypotenuse
}






inline int cycles_of_component(const std::pair<int, int>& Kcomponent_Ccycle)
{
    return Kcomponent_Ccycle.second;
}



std::vector<int>* get_inner_cycles(
    const Doubly_connected_edge_list& d,
    std::vector<int>* ic_storage
)
{
    NTX(ic_storage);
    for (size_t i = 0; i < d.get_face_table().size(); ++i)
    {
        const std::vector<size_t>& f=d.get_face_table().at(i).inner_components;
        for (size_t j = 0; j < f.size(); ++j)
        {
            ic_storage -> push_back( d.get_cycle( f.at(j) ) );
        }
    }
    KJB(ASSERT(* std::max_element(ic_storage -> begin(), ic_storage -> end())
                    < (int)d.get_number_of_cycles()));
    return ic_storage;
}



/* Build an undirected graph G that describes the face topology.
 * vertices(G) are the edge cycles in the DCEL, plus a "pseudocycle."
 * edges(G) form connected components for edge cycles incident to same face.
 * They bridge from an inner component -- from its maximum vertex, which is
 * topmost -- to the next cycle upwards, if any, on the same face.  If there
 * is no such cycle, we create a fictional outer cycle ("pseudocycle") that
 * it bridges to.  Thus every inner component cycle gets to bridge to some
 * other vertex in G.
 *
 * This returns an adjacency list, indexed by cycle number, adjacent to cycles
 * of the same face -- with one tweak.
 * In fact the last entry in the adjacency list corresponds to a "pseudocycle"
 * around everything, and adjacent to all the inner components of the index-0
 * face.  There's only one fiction like this, so I call it "THE pseudocycle."
 *
 * Graph G is undirected, and the adjacency list reflects that symmetry.
 *
 * Input inner_cycles is a "hint": an optional list of all cycle ID numbers of
 * inner components.  If provided, this routine will run a little faster.
 * If omitted, this will compute it separately based on the face table.
 * Do not omit the input if the DCEL has an unreliable face table.
 */
std::vector< std::set<int> > get_facial_adjacency_list(
    const Doubly_connected_edge_list& dcel,
    const std::vector<int>* inner_cycles // subset of cycles, order irrelevant
)
{
    std::vector<int> icopt;
    if (00 == inner_cycles) inner_cycles = get_inner_cycles(dcel, &icopt);

#if 0
    // find the top vertex of them all
    RatPoint top = dcel.get_vertex_table().at(0).location;
    for (size_t i = 1; i < dcel.get_vertex_table().size(); ++i)
    {
        const RatPoint& p(dcel.get_vertex_table().at(i).location);
        if (top.y < p.y) top=p;
    }
#endif

    /* Build graph G of cycles, with arcs forming connected components per
     * face.  Plus one is for the pseudocycle.
     */
    const int CYC_CT = 1 + dcel.get_number_of_cycles();
    std::vector< std::set<int> > adj_ls(CYC_CT); // G's vertices: edge cycles
    for (size_t i = 0; i < inner_cycles -> size(); ++i)
    {
        const int ci = inner_cycles -> at(i); // inner cycle ID
        const size_t v = dcel.get_top_vertex_of_cycle(ci);
        const int c_above = dcel.vertex_has_roof(v) ?
                            dcel.get_cycle(dcel.get_vertex_roof(v)) : CYC_CT-1;

        adj_ls.at(ci).insert(c_above);
        adj_ls.at(c_above).insert(ci);

        // In this manner, build the entire adjacency list of G.
    }
    return adj_ls;
}



/* Of the vector this returns:  index it by cycle number, get a connected
 * component number.  Connected component numbers are a permutation of face
 * numbers.
 *
 * Also, the last entry corresponds to the pseudocycle, which is associated
 * with the outermost, index-0 face.  The index of that entry does NOT
 * correspond to a real cycle.  See get_facial_adjacency_list() for more.
 *
 * Input inner_cycles is a "hint": an optional list of all cycle ID numbers of
 * inner components.  If provided, this routine will run a little faster.
 * If omitted, this will compute it separately based on the face table.
 * Do not omit the input if the DCEL has an unreliable face table.
 */
std::vector<int> get_facial_components(
    const Doubly_connected_edge_list& dcel,
    const std::vector<int>* inner_cycles // subset of cycles, order irrelevant
)
{
    // get graph G describing relationship between cycles and faces.
    const std::vector< std::set<int> > adj_ls
        = get_facial_adjacency_list(dcel, inner_cycles);

    // find connected components of G, using depth-first search (aka DFS).
    std::vector< bool > vmark(adj_ls.size(), false);
    std::vector< int > vcomponent(adj_ls.size()); // index by cycle
    int k = -1; // connected component number
    for (int c = 0; c < (int)adj_ls.size(); ++c) // scan through cycles
    {
        if (vmark.at(c)) continue;
        ++k;
        for (std::vector<int> dfs(1, c); !dfs.empty(); )
        {
            const int cyc = dfs.back();
            dfs.pop_back();
            if (vmark.at(cyc)) continue;
            vmark.at(cyc) = true;
            vcomponent.at(cyc) = k;
            // Continue search into the neighboring G-vertices (other cycles).
            std::copy(adj_ls.at(cyc).begin(), adj_ls.at(cyc).end(),
                        std::back_inserter(dfs));
        }
    }
    return vcomponent;
}


typedef std::multimap< int, int > MMII;
typedef std::pair< MMII::const_iterator, MMII::const_iterator > MMCR;


// this routine is slow but could be made faster by using a sweep paradigm
bool holes_valid(
    const Doubly_connected_edge_list& dcel,
    const Facemap& facemap,
    const std::vector<int>& inner_cycles // sorted list of inner cycles
)
{
    if (inner_cycles.empty()) return true;
    // Make a more easily searchable data structure for inner-cycle numbers.
    const std::set<int> incycs(inner_cycles.begin(), inner_cycles.end());

    // function that maps from cycle number to (permutation of index of) face
    const std::vector<int> fcomponents(
            get_facial_components(dcel, &inner_cycles));

    // separate cycles of each component into inner, outer parts
    MMII ics_of_k; // in-cycles of component k (key)

    // Loop through all cycles, and index them according to facial component.
    // Last index in fcomponents is not of a real cycle, it is the pseudocycle.
    // That is why there is the ugly plus-one in the loop condition.
    for (int cyc = 0; cyc+1 < (int)fcomponents.size(); ++cyc)
    {
        const int k = fcomponents.at(cyc);

        // Store face (i.e., connected-component) info for this cycle.
        if (incycs.find(cyc) != incycs.end())
        {
            // Grow a list of the clockwise inner-cycles of this component.
            ics_of_k.insert(std::make_pair(k, cyc));
        }
    }

    /* For each component, verify that each cycle in the component has an
     * edge with valid face field, that the face record stores one edge
     * from each cycle, and the face record references no other cycle.
     */
    std::set<int> used;
    for (size_t i = 0; i < dcel.get_face_table().size(); ++i)
    {
        const Doubly_connected_edge_list::Face_record& f
            = dcel.get_face_table().at(i);
        const size_t ei = i>0 ? f.outer_component : f.inner_components.front();
        const int c = dcel.get_cycle(ei), k = fcomponents.at(c);

        // Test if this face-to-k identity is fresh (k not used yet).
        if (used.find(k) == used.end())
        {
            used.insert(k); // Cancel it:  k has been "used up."
        }
        else
        {
            kjb_c::set_error("Face %u record is mixed with another face.", i);
            return false;
        }

        // I think we already checked that facemap is valid at ei's cycle
        KJB(ASSERT(   facemap.find(c) != facemap.end()
                   && facemap.find(c) -> second == i));

        // Get list of cycles for the G-component k of the cycle of edge ei.
        const MMCR range = ics_of_k.equal_range(k);
        std::vector<int> icc; // inner component cycles
        std::transform(range.first, range.second, std::back_inserter(icc),
                                                        cycles_of_component);
        // Check the number of components, inners plus outer except for index-0
        if (icc.size() != f.inner_components.size())
        {
            kjb_c::set_error("Face %u has %u actual components but lists %u.",
                    i, icc.size(), 1+f.inner_components.size());
            return false;
        }

        // Compare cycle lists
        for (size_t j = 0; j < icc.size(); ++j)
        {
            const Facemap::const_iterator m = facemap.find(icc.at(j));
            KJB(ASSERT(m != facemap.end()));
            if (m -> second != i)
            {
                kjb_c::set_error("Face %u contains a cycle %d recorded for "
                        "face ", i, m -> second);
                return false;
            }
        }
    }

    return true;
}



/*
 * Perform geometric sorting of edges incident to a vertex.
 * This is used (1) to check DCEL invariants, that edge cycles do not cross,
 * and (2) to insert a new edge and set up the prev/next pointers properly.
 * This does not rely on a DCEL to have correct prev/next/face edge fields.
 * However, the vertex table and the edges' origin and twin fields must be ok.
 */
class Star
{
    class Distal
    {
        const RatPoint p_;
    public:
        Distal(const RatPoint& point): p_(point) {}
        const RatPoint& get_point() const { return p_; }

        /* Order is geometrically counterclockwise, around the origin,
         * starting from 3 o'clock.  If d1_.p and d2_.p and the origin are
         * collinear, return false; we regard them as "equal."
         */
        bool operator<(const Distal& d) const
        {
            if (0 == p_.y && 0 <= p_.x)
            {
                return d.p_.x < 0 || d.p_.y != 0;
            }
            if (0 <= p_.y)
            {
                if (d.p_.y < 0) return true;
                return p_.x * d.p_.y > d.p_.x * p_.y;
            }
            if (d.p_.y >= 0) return false;
            return p_.x * d.p_.y > d.p_.x * p_.y;
        }
    };


    typedef std::map< Distal, size_t > DistalMap; // direction to edge ix
    typedef std::map< size_t, DistalMap::const_iterator > RvsDSI;

    const Doubly_connected_edge_list& dcel_;
    const RatPoint center_;

    // Store CCW-ordered lists of in-edges, out-edges.  Look up by
    // direction 2-vector, get edge index.
    DistalMap ie, oe;

    // Lookup by edge index, get iterator into DistalMap.  The iterator
    // order gives you clockwise, counterclockwise neighbors.
    RvsDSI rvs_;

    // Insert a range of star edges, given by edge index number.
    // If neither terminus of the edge is at center, throw Illegal_argument.
    // Return number of successful insertions
    template <typename I>
    size_t insert(I i, I end, DistalMap& edges)
    {
        size_t count = 0;
        while (i != end)
        {
            const size_t e_ix = *i++;
            const RatPoint_line_segment s = get_edge(dcel_, e_ix);
            const bool ca = center_ == s.a, cb = center_ == s.b;
            if (ca == cb)
            {
                std::ostringstream os;
                os << "Failure inserting into Star at edge index " << e_ix
                    << ", which has termini " << s.a << " and " << s.b
                    << " around center " << center_;
                KJB_THROW_2(kjb::Illegal_argument, os.str());
            }
            const DistalMap::value_type v = std::make_pair(
                    Distal((ca ? s.b : s.a) - center_), e_ix);
            std::pair< DistalMap::iterator, bool > j = edges.insert(v);
            rvs_[e_ix] = j.first;
            if (j.second) ++count;
        }
        return count;
    }

    struct Distal_edge
    {
        size_t operator()(const DistalMap::value_type& t) const
        {
            return t.second;
        }
    };

public:
    Star(const Doubly_connected_edge_list& dcel, const RatPoint& center)
    :   dcel_(dcel),
        center_(center)
    {}

    template <typename I> size_t insert_in(I i, I end)
    {
        return insert(i, end, ie);
    }

    template <typename I> size_t insert_out(I i, I end)
    {
        return insert(i, end, oe);
    }


    /*
     * This returns a list of paired edges, based only on geometry, that ought
     * to be previous/next pairs.  This can be used to test the geometric
     * validity of the previous/next pairings of edges incident to the center.
     * In the output vector v, v[i].first is the index of an out-edge, and
     * v[i].second is the index of an in-edge, whose "next" edge-index should
     * be v[i].first; that is for 0 <= i < v.size(), naturally.
     * The list covers all edges incident to the center.
     */
    std::vector< std::pair<size_t, size_t> > out_in_pairs() const
    {
        if (!is_valid()) KJB_THROW_2(kjb::Dimension_mismatch,"Unpaired edges");
        std::vector< std::pair<size_t, size_t> > v(oe.size());
        DistalMap::const_iterator i = oe.begin(), j = ie.begin();
        ++j;
        for (size_t k = 0; i != oe.end(); ++k)
        {
            if (ie.end() == j) j = ie.begin(); // j is cyclic
            v.at(k).first = i++ -> second;
            v.at(k).second = j++ -> second;
        }
        return v;
    }


    // Return edge index of the in-edge to center that is geometrically
    // counterclockwise-next from query out-edge.  By geometric logic,
    // that one should be cyclically previous to the query out-edge.
    // The query out-edge must have been previously inserted by calling
    // insert_out().
    // Although the prev/next fields are not consulted, the twin field
    // is required to be correct (as is true throughout class Star).
    size_t find_in_previous(size_t out_edge_ix) const
    {
        const size_t &in_edge_ix = dcel_.get_edge_table().at(out_edge_ix).twin;
        RvsDSI::const_iterator k = rvs_.find(in_edge_ix);
        if (rvs_.end() == k) KJB_THROW(kjb::Illegal_argument);
        DistalMap::const_iterator i = k -> second;
        KJB(ASSERT(i != ie.end() && i -> second == in_edge_ix));
        ++i; // get CCW-next in-edge
        return (i==ie.end() ? ie.begin() : i) -> second;
    }

    // Similar to find_in_previous, but input is the index of an in-edge,
    // and we return the clockwise-next out-edge incident to center.
    // It should be cyclically "next" to the query in-edge.
    size_t find_out_next(size_t in_edge_ix) const
    {
        const size_t &oedge_ix = dcel_.get_edge_table().at(in_edge_ix).twin;
        RvsDSI::const_iterator k = rvs_.find(oedge_ix);
        if (rvs_.end() == k) KJB_THROW(kjb::Illegal_argument);
        DistalMap::const_iterator j = k -> second;
        KJB(ASSERT(!oe.empty() && j != oe.end() && j->second == oedge_ix));
        if (j == oe.begin()) j = oe.end();
        --j; // get CW-next out-edge
        return j -> second;
    }

    bool is_valid() const { return ie.size() == oe.size(); }

    /*
     * Query DistalMap ie of star for first Distal equal or greater to query,
     * if it exists.  If it does not exist we return the edge table size,
     * which obviously is not a valid edge index -- so check the return value.
     */
    size_t lower_bound_in(const RatPoint& query_distal) const
    {
        DistalMap::const_iterator j = ie.lower_bound(Distal(query_distal));
        if (ie.end() == j) return dcel_.get_edge_table().size();
        return j -> second;
    }

    /*
     * This returns a list of in-edge indices sorted in CCW order.
     */
    std::vector<size_t> sorted_in_list() const
    {
        std::vector<size_t> l(ie.size());
        std::transform(ie.begin(), ie.end(), l.begin(), Distal_edge());
        return l;
    }
};



// This is slow and complicated.  Time complexity is O(mn) where m
// is the number of vertices and n is the number of edges.
// It tests whether the roof field of the vertex table is accurate.
bool vertex_roofs_valid(const Doubly_connected_edge_list& /*dcel*/)
{
#if 0 /* formerly ifdef DEBUGGING, but this is so slow I killed it outright.*/
    // first verify cycletop table
    const int CN = dcel.get_number_of_cycles();
    for (int c = 0; c < CN; ++c)
    {
        const size_t
            ei = dcel.get_edge_of_cycle(c),
            v = dcel.get_edge_table().at(
                get_edge_index_with_max_origin_of_cycle(dcel, ei)).origin,
            w = dcel.get_top_vertex_of_cycle(c);
        if (w != v)
        {
            kjb_c::set_error("Cycletop table bad:  cycle %d top vertex is %u"
                                " but table says %u", c, v, w);
            return false;
        }
    }

    // scan for pinnacle
    size_t itiptop = 0;
    RatPoint ptiptop = dcel.get_vertex_table().at(itiptop).location;
    for (size_t i = 1; i < dcel.get_vertex_table().size(); ++i)
    {
        const RatPoint p = dcel.get_vertex_table().at(i).location;
        KJB(ASSERT(p != ptiptop));
        if (ptiptop < p)
        {
            itiptop = i;
            ptiptop = p;
        }
    }

    // check the roof field of all other vertices
    for (size_t i = 0; i < dcel.get_vertex_table().size(); ++i)
    {
        Doubly_connected_edge_list::Vertex_record const&
            v = dcel.get_vertex_table().at(i);
        const RatPoint p = v.location;

        // if pinnacle-height then it is roofless
        if (p.y == ptiptop.y)
        {
            if (dcel.vertex_has_roof(i))
            {
                kjb_c::set_error("Maximum-y vertex cannot have a roof");
                return false;
            }
            continue; // this vertex is ok, skip to next vertex.
        }

        // Verify whether v has a roof and if it is correct.
        // This is fairly complicated.
        // Shoot a ray upward from v and see what, if anything, it hits.
        const RatPoint_line_segment ray(p, RatPoint(p.x, ptiptop.y));
        // Make a list of the edges it hits, keyed by distance squared
        typedef std::multimap< RatPoint::Rat, size_t > DR;
        DR rayhit;

        // Loop through all the edges looking for a ray intersection.
        for (size_t ej = 0; ej < dcel.get_edge_table().size(); ++ej)
        {
            const RatPoint_line_segment s = get_edge(dcel, ej);
            if (is_vertical(s)) // they are a special case
            {
                if (std::min(s.a, s.b) == p)
                {
                    // vertex is at base of a flagstaff, which is its roof.
                    rayhit.insert(std::make_pair(0, ej));
                    // we have a winner!  skip all remaining edges.
                    break;
                }
                // else s cannot be the roof, even if s.a.x == p.x.
            }
            else
            {
                RatPoint_line_segment rs(s); // used as output parameter
                RatPoint::Rat d2; // squared distance between p and s along ray
                /* About the compound condition below:
                 * - rs contains intersection of ray and edge ej;
                 * - if s touches (and is not a flagstaff) it is not a roof;
                 * - A quirk of our method is that the ray effectively drifts
                 *   right, so a would-be roof lacking interior points to the
                 *   right of the ray does not count as a proper roof.
                 */
                if (    segment_intersection(ray, s, &rs)
                    &&  0 < (d2 = distance2(p, rs.a))
                    &&  p.x < std::max(s.a.x, s.b.x))
                {
                    KJB(ASSERT(is_degenerate(rs))); // already did verticals
                    rayhit.insert(std::make_pair(d2, ej));
                }
            }
        }

        if (rayhit.empty())
        {
            if (dcel.vertex_has_roof(i))
            {
                kjb_c::set_error("Vertex %u has spurious roof", i);
                return false;
            }
            // else v is correctly marked as roofless
            continue; // this vertex is ok, skip to next vertex
        }

        /*
         * One of the edges in rayhit is the roof:  find it, and verify
         * that v.roof matches it.
         */
        if (! dcel.vertex_has_roof(i))
        {
            kjb_c::set_error("Vertex %u lacks expected roof", i);
            return false;
        }

        // Is the roof a flagstaff with v as its base?
        if (0 == rayhit.begin() -> first)
        {
            if (dcel.get_vertex_roof(i) != rayhit.begin() -> second)
            {
                kjb_c::set_error("Vertex %u has wrong roof "
                    "(expected flagstaff %u, found %u)",
                    i, rayhit.begin() -> second, dcel.get_vertex_roof(i));
                return false;
            }
            // else v has correct flagstaff roof
#ifdef DEBUGGING
            DR::const_iterator b = rayhit.begin(),
                               bub = rayhit.upper_bound(b -> first);
            KJB(ASSERT(++b == bub));
#endif
            continue; // this flagpolled vertex is ok, skip to next vertex
        }

        // Roof of v is not a flagstaff.  It's near the front of rayhit.
        // We are still not sure which edge it is though -- find out!
        // First we winnow through minimal-key records of rayhit.
        DR::const_iterator b = rayhit.begin(),
                           bub = rayhit.upper_bound(b -> first);
        const RatPoint q = line_intersection(get_edge(dcel,b->second),ray);
        // Candidates are all edges that pass through q going right to left.
        std::vector<size_t> candidate_roofs;
        for (; b != bub; ++b)
        {
            KJB(ASSERT(distance2(p, q) == b -> first));
            const size_t ek = b -> second;
            const RatPoint_line_segment s = get_edge(dcel, ek);
            KJB(ASSERT(is_on(s, q)));
            if (s.b.x < s.a.x)
            {
                KJB(ASSERT(triangle_area(s, p) > 0));
                candidate_roofs.push_back(ek);
            }
        }
        KJB(ASSERT(!candidate_roofs.empty()));

        size_t iroof = candidate_roofs.front();
        if (candidate_roofs.size() > 1)
        {
            /* Winnow through candidate_roofs.
             * We get here when ray hits a vertex directly above v.  One edge
             * incident to that vertex must be the correct roof.  Star class
             * will sort the edges by tangent.
             */
            Star star(dcel, q);
            star.insert_in(candidate_roofs.begin(), candidate_roofs.end());
            iroof = star.lower_bound_in(RatPoint(0, 1));
            KJB(ASSERT(iroof < dcel.get_edge_table().size()));
        }
        if (dcel.get_vertex_roof(i) != iroof)
        {
            kjb_c::set_error("Vertex %u has wrong roof "
                "(expected %u, found %u)", i, iroof, dcel.get_vertex_roof(i));
            return false;
        }
    }
#endif

    return true;
}



bool faces_valid(const Doubly_connected_edge_list& dcel)
{
    // Trivial check
    if (    dcel.get_face_table().size() > 1
        &&  dcel.get_face_table().at(0).inner_components.size() < 1)
    {
        kjb_c::set_error("Structure has more than one face, but index-0 "
                               " face does not list inner components.");
        return false;
    }

    // check vertex roofs
    const bool vrv = vertex_roofs_valid(dcel);
    if (!vrv) return false;

    /* Each cycle maps to a face.  Different cycles can map to the same face,
     * e.g., a bounded face with 2 holes will have three distinct cycles all
     * incident to the same face.
     */
    Facemap facemap; // key is cycle #, mapped val is face idx

    // partition the cycle #s into two sets, outer borders and inner cycles
    std::vector<int> outer_border, inner_cycles;

    // Scan all edges, look at cycle membership.  Edges of same cycle should
    // be incident to the same face, so verify that.
    for (size_t i = 0; i < dcel.get_edge_table().size(); ++i)
    {
        const Doubly_connected_edge_list::Edge_record &e
            = dcel.get_edge_table().at(i);
        const int c = dcel.get_cycle(i);
        if (facemap.find(c) == facemap.end())
        {
            // starting a new cycle
            facemap[c] = e.face;

            // if it is an outer border it represents a distinct face
            if (is_on_outer_border(dcel, i))
            {
                outer_border.push_back(c);
            }
            else
            {
                // It might surround a face (letter A), or not (letter T)
                inner_cycles.push_back(c);
            }
        }
        else if (facemap[c] != e.face)
        {
            // continuing an existing cycle
            kjb_c::set_error("Inconsistent faces in edge cycle");
            return false;
        }
    }

    // check that face record outer borders, inner components agree
    std::vector<int> ftab_ob, ftab_ic;
    for (size_t i = 0; i < dcel.get_face_table().size(); ++i)
    {
        const Doubly_connected_edge_list::Face_record& f
            = dcel.get_face_table().at(i);
        if (i > 0 && f.outer_component >= dcel.get_edge_table().size())
        {
            kjb_c::set_error("Invalid edge index in outer component "
                    "of face %u", i);
            return false;
        }
        if (i > 0) ftab_ob.push_back(dcel.get_cycle(f.outer_component));
        for (size_t j = 0; j < f.inner_components.size(); ++j)
        {
            if (f.inner_components.at(j) >= dcel.get_edge_table().size())
            {
               kjb_c::set_error("Invalid edge index in inner cycle "
                       "of face %u", i);
               return false;
            }
            ftab_ic.push_back(dcel.get_cycle(f.inner_components.at(j)));
        }
    }

    if (ftab_ob.size() != outer_border.size())
    {
        kjb_c::set_error("Face table contains %u outer borders "
                "but geometric analysis counts %u outer borders",
                ftab_ob.size(), outer_border.size());
        return false;
    }
    if (ftab_ic.size() != inner_cycles.size())
    {
        kjb_c::set_error("Face table contains %u inner cycles "
                "but geometric analysis counts %u inner cycles",
                 ftab_ic.size(), inner_cycles.size());
        return false;
    }

    std::sort(ftab_ob.begin(), ftab_ob.end());
    std::sort(ftab_ic.begin(), ftab_ic.end());
    std::sort(outer_border.begin(), outer_border.end());
    std::sort(inner_cycles.begin(), inner_cycles.end());

    KJB(ASSERT(outer_border.end()
                == std::unique(outer_border.begin(), outer_border.end())));
    KJB(ASSERT(inner_cycles.end()
                == std::unique(inner_cycles.begin(), inner_cycles.end())));
    if (ftab_ob.end() != std::unique(ftab_ob.begin(), ftab_ob.end()))
    {
        kjb_c::set_error("Face table outer_components entries contain"
                               "duplicates.");
        return false;
    }
    if (ftab_ic.end() != std::unique(ftab_ic.begin(), ftab_ic.end()))
    {
        kjb_c::set_error("Face table inner_components entries contain"
                               "duplicates.");
        return false;
    }

    // Since the OB, IB lists are same size and duplicate-free, then subset
    // inclusion implies set equality.  No need to test four times, just twice.
    if (!std::includes(outer_border.begin(), outer_border.end(),
                        ftab_ob.begin(), ftab_ob.end()))
    {
        kjb_c::set_error("Face table outer_components list is incomplete.");
        return false;
    }
    if (!std::includes(inner_cycles.begin(), inner_cycles.end(),
                        ftab_ic.begin(), ftab_ic.end()))
    {
        kjb_c::set_error("Face table inner_components list is incomplete.");
        return false;
    }

    // check the holes
    const bool h = holes_valid(dcel, facemap, inner_cycles);
    if (!h) return false;

    // check edges mentioned by face outer, inner components fields.
    // edge index must be (1) valid, (2) part of the cycle mapped back
    // to that same face, (3) that cycle must be used only for that face.
    // This function destroys facemap.
    const bool c = check_face_components(dcel, &facemap);
    if (!c) return false;

    return true;
}


// We use this function even when the DCEL does not meet all its invariants,
// specifically when the next/prev edge fields might be wrong.
// Thus we cannot use functions out_edges(), in_edges(), which rely on them.
// This is a slow function and we should try to eliminate its use.
Star build_star(
    const Doubly_connected_edge_list& dcel,
    size_t vertex_index,
    bool throw_if_bad_here = true
)
{
    const RatPoint &center = dcel.get_vertex_table().at(vertex_index).location;
    // safe_out_edges is the slow part -- it scans the entire edge table once.
    const std::vector< size_t > oute = safe_out_edges(dcel, vertex_index),
                                ine = kjb::qd::twin_edges(dcel, oute);
    Star star(dcel, center);
    star.insert_out(oute.begin(), oute.end());
    star.insert_in(ine.begin(), ine.end());
    if (throw_if_bad_here && !star.is_valid())
    {
        KJB_THROW_2(kjb::Runtime_error, "DCEL is malformed");
    }
    return star;
}



// Test, at each vertex, that the cycles do not cross.
// This is slow -- it takes time O(mn) when we have m vertices, n edges.
bool cycles_valid(const Doubly_connected_edge_list& dcel)
{
    const size_t nv = dcel.get_vertex_table().size();
    for (size_t i = 0; i < nv; ++i)
    {
        const std::vector< size_t > oute = safe_out_edges(dcel, i),
                                    ine = kjb::qd::twin_edges(dcel, oute);

        // Merge edge lists.  Check that oute and ine are disjoint.
        std::set< size_t > edges(oute.begin(), oute.end());
        edges.insert(ine.begin(), ine.end());
        KJB(ASSERT(oute.size() + ine.size() == edges.size()));

        // sort around vertex v location, say CCW starting from 3 o'clock.
        const Star star( build_star(dcel, i, false /* == do not throw */) );
        if (!star.is_valid())
        {
            kjb_c::set_error("Mismatched in, out edges");
            return false;
        }

        // check for crossing cycles, not sure I can envision every case.
        const std::vector< std::pair<size_t, size_t> > p = star.out_in_pairs();
        for (size_t j = 0; j < p.size(); ++j)
        {
            // Get in-out pair based on geometry; check that the table ordering
            // of prev/next is true to the geometry.
            const size_t &eout_ix = p.at(j).first, &ein_ix = p.at(j).second;

            if (eout_ix != dcel.get_edge_table().at(ein_ix).next)
            {
                kjb_c::set_error("Crossed cycle at edges %u, %u",
                        eout_ix, ein_ix);
                return false;
            }
            if (dcel.get_cycle(eout_ix) != dcel.get_cycle(ein_ix))
            {
                // maybe unreachable?
                kjb_c::set_error("Malformed cycle, edges %u, %u",
                        eout_ix, ein_ix);
                return false;
            }
        }
    }
    return true;
}



// DOES NOT RELY ON EDGE NEXT/PREV FIELDS.
// Consequently it might have to search through the whole edge field.
bool is_edge_btw_vxs(const Doubly_connected_edge_list& d, size_t v1, size_t v2)
{
    if (std::max(v1, v2) >= d.get_vertex_table().size())
    {
        KJB_THROW_2(kjb::Illegal_argument, "Vertex indices out of bounds");
    }
    for (size_t i = 0; i < d.get_edge_table().size(); ++i)
    {
        const Doubly_connected_edge_list::Edge_record& e
            = d.get_edge_table().at(i);
        if (v1 == e.origin && v2 == d.get_edge_table().at(e.twin).origin)
        {
            return true;
        }
    }
    return false;
}


// Check validity of an empty DCEL (no edges).
int is_tiny_valid(const Doubly_connected_edge_list& dcel)
{
    using kjb_c::ERROR;
    KJB(ASSERT(dcel.get_edge_table().empty())); // that's why we are here

    if (!dcel.get_vertex_table().empty())
    {
        kjb_c::set_error("Vertex or vertices without edges.");
        return ERROR;
    }
    if (dcel.get_face_table().size() != 1)
    {
        kjb_c::set_error("Bad face table size of %u",
                dcel.get_face_table().size());
        return ERROR;
    }
    if (!dcel.get_face_table().at(0).inner_components.empty())
    {
        kjb_c::set_error("Face table has spurious inner component(s).");
        return ERROR;
    }
    return kjb_c::NO_ERROR;
}



void graceful_import(
    const std::vector< RatPoint >& sl,
    std::vector< Doubly_connected_edge_list::Vertex_record >* pvtab,
    std::vector< Doubly_connected_edge_list::Edge_record >* petab
)
{
    const size_t N = sl.size();
    KJB(ASSERT(N > 2));
    KJB(ASSERT(pvtab && petab));

    pvtab -> clear();
    for (size_t i = 0; i < N; ++i)
    {
        pvtab -> push_back(
            Doubly_connected_edge_list::Vertex_record( sl[i], 2*i ) );
    }
    pvtab -> at(N-1).outedge -= 1;

    petab -> clear();
    for (size_t i = 0; i < N-1; ++i)
    {
        KJB(ASSERT(2*i == petab -> size()));
        // previous and next are valid for edges in the middle of the train
        petab -> push_back(
            Doubly_connected_edge_list::Edge_record(
                i, 1+2*i, 0, 2+2*i, 2*i-2));
        petab -> push_back(
            Doubly_connected_edge_list::Edge_record(
                1+i, 2*i, 0, 2*i-1, 3+2*i));
    }
    KJB(ASSERT(2*N-2 == petab -> size()));

    // fix the caboose
    petab -> at(0).prev = 1;
    petab -> at(1).next = 0;
    // fix the engine
    petab -> at(2*N-4).next = 2*N-3;
    petab -> at(2*N-3).prev = 2*N-4;
}


Doubly_connected_edge_list ctor_from_el(
    const RatPoint_line_segment el[],
    size_t size
)
{
    KJB(ASSERT(size >= 1));
    if (1 == size)
    {
        return Doubly_connected_edge_list(el[0]);
    }

    const size_t na = size/2;
    const Doubly_connected_edge_list
        a=ctor_from_el(el, na), b=ctor_from_el(el+na, size-na);
    return b.merge(a);
}


std::string xml_rational(const kjb::qd::RatPoint::Rat r)
{
    std::ostringstream v, ooo;
    BUILD_XML_ELT(v, XML_TAG_NUMERATOR, kjb::qd::i_numerator(r));
    BUILD_XML_ELT(v, XML_TAG_DENOMINATOR, kjb::qd::i_denominator(r));
    BUILD_XML_ELT(ooo, XML_TAG_RATIONAL, "\n" << v.str());
    return ooo.str();
}


std::string xml_dcel_vertex_guts(
    const Doubly_connected_edge_list& dcel,
    size_t i
)
{
    const Doubly_connected_edge_list::Vertex_record
        &v = dcel.get_vertex_table().at(i);
    std::ostringstream l, ooo;
    BUILD_XML_ELT(l, XML_TAG_X, "\n" << xml_rational(v.location.x));
    BUILD_XML_ELT(l, XML_TAG_Y, "\n" << xml_rational(v.location.y));
    BUILD_XML_ELT(ooo, XML_TAG_OUTEDGE, v.outedge);
    BUILD_XML_ELT(ooo, XML_TAG_LOCATION, "\n" << l.str());
    return ooo.str();
}


std::string xml_dcel_edge_guts(const Doubly_connected_edge_list& dcel,size_t i)
{
    std::ostringstream ooo;
    const Doubly_connected_edge_list::Edge_record
        &e = dcel.get_edge_table().at(i);
    BUILD_XML_ELT(ooo, XML_TAG_ORIGIN, e.origin);
    BUILD_XML_ELT(ooo, XML_TAG_TWIN, e.twin);
    BUILD_XML_ELT(ooo, XML_TAG_INCFACE, e.face);
    BUILD_XML_ELT(ooo, XML_TAG_NEXT, e.next);
    BUILD_XML_ELT(ooo, XML_TAG_PREV, e.prev);
    return ooo.str();
}


std::string xml_dcel_face_guts(const Doubly_connected_edge_list& dcel,size_t i)
{
    std::ostringstream m, ooo;
    const Doubly_connected_edge_list::Face_record
        &f = dcel.get_face_table().at(i);
    if (i > 0)
    {
        std::ostringstream k;
        BUILD_XML_ELT(k, XML_TAG_EDGEINDEX, f.outer_component);
        BUILD_XML_ELT(ooo, XML_TAG_OCOMPONENT, k.str());
    }
    for (size_t j = 0; j < f.inner_components.size(); ++j)
    {
        BUILD_XML_ELT(m, XML_TAG_EDGEINDEX, f.inner_components.at(j));
    }
    BUILD_XML_ELT(ooo, XML_TAG_ICOMPONENTS, "\n" << m.str());
    return ooo.str();
}


std::string xml_dcel_vertices_guts(const Doubly_connected_edge_list& dcel)
{
    std::ostringstream ooo;
    for (size_t i = 0; i < dcel.get_vertex_table().size(); ++i)
    {
        std::ostringstream vi; //vertex tag has embedded (redundant) index attr
        vi << " index='" << i << '\'';
        BUILD_XML_ATTR_ELT(ooo, XML_TAG_VERTEX, vi.str(),
                            "\n" << xml_dcel_vertex_guts(dcel,i));
    }
    return ooo.str();
}


std::string xml_dcel_edges_guts(const Doubly_connected_edge_list& dcel)
{
    std::ostringstream ooo;
    for (size_t i = 0; i < dcel.get_edge_table().size(); ++i)
    {
        std::ostringstream ei; // edge tag has embedded (redundant) index attr
        ei << " index='" << i << '\'';
        BUILD_XML_ATTR_ELT(ooo, XML_TAG_EDGE, ei.str(),
                            "\n" << xml_dcel_edge_guts(dcel, i));
    }
    return ooo.str();
}


std::string xml_dcel_faces_guts(const Doubly_connected_edge_list& dcel)
{
    std::ostringstream ooo;
    for (size_t i = 0; i < dcel.get_face_table().size(); ++i)
    {
        std::ostringstream fi; // face tag has embedded (redundant) index attr
        fi << " index='" << i << '\'';
        BUILD_XML_ATTR_ELT(ooo, XML_TAG_FACE, fi.str(),
                            "\n" << xml_dcel_face_guts(dcel, i));
    }
    return ooo.str();
}


std::string xml_dcel_guts(const Doubly_connected_edge_list& dcel)
{
    std::ostringstream ooo;
    BUILD_XML_ELT(ooo, XML_TAG_VERTICES, "\n" << xml_dcel_vertices_guts(dcel));
    BUILD_XML_ELT(ooo, XML_TAG_EDGES, "\n" << xml_dcel_edges_guts(dcel));
    BUILD_XML_ELT(ooo, XML_TAG_FACES, "\n" << xml_dcel_faces_guts(dcel));
    return ooo.str();
}


//struct eq_zero { bool operator()(size_t n) { return 0==n; } };
struct nonzero { bool operator()(size_t n) { return n != 0; } };

// Obtain a list of all edge indices incident to face fi.
// This code is similar to that of get_incident_edges() in monotone.cpp
// but the facemap thing mangles it so it seemed better not to share code.
std::vector<size_t> get_edges_map_faces(
    const Doubly_connected_edge_list &d,
    size_t fi,
    std::vector<size_t>* facemap
)
{
    KJB(ASSERT(facemap && facemap -> size() == d.get_face_table().size()));
    KJB(ASSERT(std::find_if(facemap -> begin(), facemap -> end(), nonzero())
                == facemap -> end()));

    if (0 == fi || fi >= d.get_face_table().size())
    {
        KJB_THROW(kjb::Index_out_of_bounds);
    }

    std::vector<size_t> eee;
    size_t facecount = 0;
    facemap -> at(fi) = ++facecount;

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
    // map twin's old faces to j in the new dcel, if twin isn't face fi.
    for (size_t j=0; j < d.get_face_table().at(fi).inner_components.size();
                    ++j)
    {
        const size_t ice0 = d.get_face_table().at(fi).inner_components.at(j),
                     t0 = d.get_edge_table().at(ice0).twin,
                     tf0 = d.get_edge_table().at(t0).face;

        eee.push_back(ice0);

        /* If this inner component is a hole (not just a stick figure), we must
         * allocate it a face number in the new dcel.  If the inner component
         * is purely a stick figure, face_allocated will always contain false.
         * If any twincident face differs from fi, then it's a hole with area
         * and we need to allocate a face number.  The inner component might
         * have many little faces in it, but they all get merged into one hole
         * face.  That is, we only allocate at most one face.  One face for a
         * component for area, zero for a stick figure.
         */
        bool face_allocated = false;
        if (tf0 != fi)
        {
            face_allocated = true;
            facemap -> at(tf0) = ++facecount;
        }

        // cycle scan idiom
        for (size_t icec = d.get_edge_table().at(ice0).next; icec != ice0;
                    icec = d.get_edge_table().at(icec).next )
        {
            eee.push_back(icec);

            const size_t t = d.get_edge_table().at(icec).twin,
                         tfc = d.get_edge_table().at(t).face;

            if (tfc != fi)
            {
                if (!face_allocated)
                {
                    ++facecount;
                    face_allocated = true;
                }
                facemap -> at(tfc) = facecount;
            }
        }
    }
    return eee;
}


void previous_next_link_impl(
    size_t e_engine,
    size_t e_caboose,
    std::vector< Doubly_connected_edge_list::Edge_record >* etab
)
{
    KJB(ASSERT(etab));
    etab -> at(e_engine).prev = e_caboose;
    etab -> at(e_caboose).next = e_engine;
}


// This repairs all previous and next linkages in the edge table.
// Time complexity is n log n for an input with n edges.
void previous_next_fixup(
    std::vector< Doubly_connected_edge_list::Edge_record >* etab,
    const Doubly_connected_edge_list& dcel
)
{
    NTX(etab);

    // Collect all out-edges, indexed by origin vertex
    typedef std::multimap< size_t, size_t > UUMM;
    UUMM vnout; // outedges of vertex
    for (size_t j = 0; j < dcel.get_edge_table().size(); ++j)
    {
        vnout.insert(std::make_pair(dcel.get_edge_table().at(j).origin, j));
    }

    // set up previous and next fields at each vertex
    while (! vnout.empty())
    {
        const size_t v = vnout.begin() -> first;
        std::pair< UUMM::iterator, UUMM::iterator > k = vnout.equal_range(v);
        KJB(ASSERT(k.first == vnout.begin()));

        // Build list of out-edges (from vnout) and in-edges (the twins) at v.
        std::vector<size_t> innies, outies;
        for (UUMM::iterator j = k.first; j != k.second; ++j)
        {
            KJB(ASSERT(v == j -> first));
            outies.push_back(j -> second);
            innies.push_back(dcel.get_edge_table().at(j -> second).twin);
        }
        vnout.erase(k.first, k.second);

        // Use class Star to sort the edges into previous-next pairs.
        Star star(dcel, dcel.get_vertex_table().at(v).location);
        star.insert_in(innies.begin(), innies.end());
        star.insert_out(outies.begin(), outies.end());
        KJB(ASSERT(star.is_valid()));
        for (std::vector< std::pair<size_t, size_t> > q = star.out_in_pairs();
                ! q.empty(); q.pop_back() )
        {
            previous_next_link_impl(q.back().first, q.back().second, etab);
        }
    }
}


class Dcel_edge
{
    const Doubly_connected_edge_list* dcel_;

public:
    Dcel_edge(const Doubly_connected_edge_list& d)
    :   dcel_(&d)
    {}

    RatPoint_line_segment lookup_no_rectify(size_t i) const
    {
        return get_edge(*dcel_, i);
    }

    RatPoint_line_segment lookup(size_t i) const
    {
        KJB(ASSERT(valid(i)));
        return kjb::qd::sweep::rectify(lookup_no_rectify(i));
    }

    bool valid(size_t i) const
    {
        return i < dcel_ -> get_edge_table().size();
    }
};


}



namespace kjb
{
namespace qd
{


#if 0 /* we hope to retire this stuff soon. */
/*
 * We are using Bentley-Ottman for a simpler problem, the so-called
 * "red/blue" line intersection problem -- segments really are
 * partitioned into a red set and a blue set, and we only care about
 * intersections of red and blue, not red-red or blue-blue.
 * So it's not the most efficient solution.  In fact brute-force can be
 * faster.  But you get an asymptotic improvement if the number of
 * intersections is less than quadratic.
 */
void Doubly_connected_edge_list::intersect_red_blue_with_bentley_ottman(
    const Doubly_connected_edge_list& red,
    VtxLookup* pnewbies, // new vertices discovered by red-blue intersections
    std::set<size_t>* pbogus_voe // vertex indices that have bogus 'outedge'
)
{
    Doubly_connected_edge_list* const pblue = this; // this dcel is Team Blue.
    const size_t NRED = red.get_edge_table().size(),
                 NBLUE = pblue -> get_edge_table().size();

    std::vector< RatPoint_line_segment > sl;
    sl.reserve((NRED+NBLUE)/2);

    // add red segments, but do not add twins of red segments.
    for (size_t i = 0; i < NRED; ++i)
    {
        if (i < red.get_edge_table().at(i).twin)
        {
            sl.push_back( get_edge(red, i) );
        }
    }
    const size_t SLPART = sl.size(); // segment list partition btw red, blue
    KJB(ASSERT( NRED/2 == SLPART ));

    // add blue segments, but not their twins
    for (size_t i = 0; i < NBLUE; ++i)
    {
        if (i < pblue -> get_edge_table().at(i).twin)
        {
            sl.push_back( get_edge(*pblue, i) );
        }
    }
    KJB(ASSERT( sl.size() == NRED/2 + NBLUE/2 ));

    // Bentley-Ottman uses time O(n log n + k log n), k is output size.
    // Iff the intersections are sparse (k is o(n*n)), you get a benefit.
    const std::vector< std::pair<size_t, size_t> > iei = get_intersections(sl);

    // Sift through B-O output, disregard red-red and blue-blue intersections.
    for (size_t k = 0; k < iei.size(); ++k)
    {
        size_t i = iei[k].first, j = iei[k].second;
        KJB(ASSERT(i < j));
        if (i >= SLPART || j < SLPART) continue; // "monochrome" intersection
        KJB(ASSERT(is_intersecting(sl[i], sl[j])));
        if (are_parallel(sl[i], sl[j])) continue; // a situation handled later.

        // If this intx. is a new discovery, save it to *pnewbies, *pbogus_voe
        pblue -> consider_newbie_vtx(
            line_intersection(sl[i], sl[j]), pnewbies, pbogus_voe);
    }
}



void Doubly_connected_edge_list::intersect_red_blue_with_brute_force(
    const Doubly_connected_edge_list& red,
    VtxLookup* pnewbies,
    std::set<size_t>* pbogus_voe
)
{
    Doubly_connected_edge_list* const pblue = this; // this dcel is Team Blue.
    const size_t NRED = red.get_edge_table().size(),
                 NBLUE = pblue -> get_edge_table().size();

    // Brute-force check for edge intersections.  Sometimes this is best.
    for (size_t i = 0; i < NRED; ++i)
    {
        const RatPoint_line_segment e = get_edge(red, i);
        for (size_t j = 0; j < NBLUE; ++j)
        {
            const RatPoint_line_segment f = get_edge(*pblue, j);
            if (is_intersecting(e, f) && ! are_parallel(e, f))
            {
                pblue -> consider_newbie_vtx(
                    line_intersection(e, f), pnewbies, pbogus_voe);
            }
        }
    }
}



bool Doubly_connected_edge_list::consider_newbie_vtx(
    const RatPoint& new_p,
    VtxLookup* newbies,
    std::set<size_t>* bogus_voe
)
{
    const size_t NV = get_vertex_table().size();
    if (NV==lookup_vertex(new_p, 00) && newbies->end() == newbies->find(new_p))
    {
        /* Warning:  note that the lookup cache flag, m_lookup_valid, is
         * deliberately neglected; instead "newbies" has the missing vertices.
         */
        bogus_voe -> insert(NV);
        (*newbies)[new_p] = NV;
        m_vtab.push_back(Vertex_record(new_p, 0));
        return true;
    }
    return false;
}
#endif


// Fill cycle table cache; this takes time O(n) in the size of the edge table.
// Edge table's next field must be accurate.
void Doubly_connected_edge_list::build_cycle_table() const
{
    const int FIRST_CYCLE = 0, BAD_CYCLE = FIRST_CYCLE-1;
    const size_t ne = get_edge_table().size();
    m_cycles.assign(ne, BAD_CYCLE); // fill with placeholders
    int cycle_num = FIRST_CYCLE;
    // Make up a table, indexed by edges, of cycle identity
    for (size_t ei = 0; ei < ne; ++ei)
    {
        if (m_cycles.at(ei) != BAD_CYCLE) continue; // marked
        m_cycles.at(ei) = cycle_num++;
        size_t watchdog = 0;

        // cycle scan idiom
        for (size_t ej = get_edge_table().at(ei).next; ej != ei;
                    ej = get_edge_table().at(ej).next)
        {
            KJB(ASSERT(ej > ei && BAD_CYCLE == m_cycles.at(ej)));
            m_cycles.at(ej) = m_cycles.at(ei);

            if (++watchdog > ne)
            {
                KJB_THROW_2(Runtime_error, "Bad prev/next fields");
            }
        }
    }

    // Compute reverse-lookup table
    m_edge_of_cyc.resize(cycle_num);
    for (size_t i = 0; i < m_cycles.size(); ++i)
    {
        m_edge_of_cyc.at(m_cycles.at(i)) = i;
    }

    m_cycle_table_valid = true;
}



void Doubly_connected_edge_list::walk_cycle_and_set_its_face(
    size_t edge_index,
    size_t face_index
)
{
    const size_t NE = m_etab.size();

#if 0
    Edge_record& e0 = m_etab.at(edge_index);
    e0.face = face_index;
    size_t count = 0;

    // cycle-scan idiom
    for (size_t i = e0.next; i != edge_index && count++ < NE; )
    {
        Edge_record& ei = m_etab.at(i);
        ei.face = face_index;
        i = ei.next;
    }
    // In a proper table, the cycle cannot be longer than the number of edges.
    if (count >= NE) KJB_THROW(kjb::Runtime_error);
#else
    m_etab.at(edge_index).face = face_index;

    // cycle-scan idiom
    for (size_t count = 0, ei = m_etab.at(edge_index).next;
         ei != edge_index; ei = m_etab.at(ei        ).next )
    {
        m_etab.at(ei).face = face_index;
        // In a proper table, the cycle cannot be longer than NE.
        if (++count >= NE) KJB_THROW(kjb::Runtime_error);
    }
#endif
}



// This *does* rely on the DCEL meeting its invariant that the 'next'
// field of the edge table are correct.  Thus this routine cannot be used
// for some internal code that has to run whilst the DCEL is temporarily
// in a partially incorrect state.  If you need a list of the out edges while
// the edge table might not have correct 'next' fields, use function
// safe_out_edges(), which is much, much slower (using brute force).
std::vector< size_t > out_edges(
    const Doubly_connected_edge_list& dcel,
    size_t vertex_index
)
{
    std::vector< size_t > voe;
    const std::vector< Doubly_connected_edge_list::Edge_record >& ET
        = dcel.get_edge_table();
    const size_t OEE = dcel.get_vertex_table().at(vertex_index).outedge;
    voe.push_back(OEE);

    // see, we use the next field!
    // star scan idiom
    for (size_t oe = ET.at(ET.at(OEE).twin).next; oe != OEE;
                oe = ET.at(ET.at(oe).twin).next )
    {
        KJB(ASSERT(ET.at(oe).origin == vertex_index));
        voe.push_back(oe);
    }

    return voe;
}


// return the twin edge indices of a list of edge indices
std::vector< size_t > twin_edges(
    const Doubly_connected_edge_list& dcel,
    const std::vector< size_t >& edge_list
)
{
    std::vector< size_t > twins(edge_list.size());

    for (size_t i = 0; i < edge_list.size(); ++i)
    {
        twins.at(i) = dcel.get_edge_table().at(edge_list.at(i)).twin;
    }
    return twins;
}


/**
 * @brief ctor builds a DCEL of a single segment.
 *
 * If the segment has distinct endpoints, then two vertices and two
 * edges are entered into a blank DCEL.
 *
 * If the segment is degenerate (i.e., just one point) the single point is NOT
 * entered, and the DCEL is blank, because our DCEL does not support lonesome
 * vertices.  Each vertex must be the origin and destination of distinct edges.
 */
Doubly_connected_edge_list::Doubly_connected_edge_list(
    const RatPoint_line_segment& s
)
{
    Doubly_connected_edge_list dcel;
    if (! is_degenerate(s))
    {
        dcel.m_vtab.push_back(Vertex_record(s.a, 0));
        dcel.m_vtab.push_back(Vertex_record(s.b, 1));
        dcel.m_etab.push_back(Edge_record(0, 1, 0, 1, 1));
        dcel.m_etab.push_back(Edge_record(1, 0, 0, 0, 0));
        dcel.m_ftab.at(0).inner_components.push_back(0);
        dcel.m_lookup_valid = false;
        dcel.m_cycle_table_valid = false;
        dcel.m_roof_table_valid = false;
    }
    swap(dcel);

    ETX(is_valid(*this));
}


/**
 * @brief look up a vertex index by its location
 * @param[in] query Point that might be a vertex in the DCEL
 * @param[out] found optional output parameter indicating whether the query
 *                   point is a vertex in the DCEL.
 * @return If query is a vertex in the DCEL, this returns its index in the
 *         vertex table.  If not, it returns the size of the vertex table,
 *         that is, get_vertex_table().size().
 *
 * This costs linear time the first time you perform the lookup, because it
 * builds and caches a map from location to index.
 * Second and later lookups to an unchanging DCEL are log time, the query time
 * of the cached map.
 */
size_t Doubly_connected_edge_list::lookup_vertex(
    const RatPoint& query,
    bool* found
)   const
{
    if (!m_lookup_valid)
    {
        m_vertex_lookup.clear();
        for (size_t i = 0; i < m_vtab.size(); ++i)
        {
            m_vertex_lookup[m_vtab[i].location] = i;
        }
        m_lookup_valid = true;
    }

    const VtxLookup::const_iterator i = m_vertex_lookup.find(query);
    if (m_vertex_lookup.end() == i)
    {
        if (found) *found = false;
        return get_vertex_table().size();
    }

    if (found) *found = true;
    return i -> second;
}


// implementation note:  html_table() is heavily coupled to details here.
std::ostream& operator<<(
    std::ostream& o,
    const Doubly_connected_edge_list::Vertex_record& v
)
{
    return o << "location=(" << v.location
             << ")=(" << dbl_ratio(v.location.x) << ','
             << dbl_ratio(v.location.y)
             << "), outedge=edge_" << v.outedge;
}


// implementation note:  html_table() is heavily coupled to details here.
std::ostream& operator<<(
    std::ostream& o,
    const Doubly_connected_edge_list::Edge_record& e
)
{
    const size_t BLANK = Doubly_connected_edge_list::BLANK;

    o << "origin=vertex_" << e.origin << ", twin=edge_" << e.twin;

    if (e.next != BLANK) o << ", next=edge_" << e.next;
    if (e.prev != BLANK) o << ", previous=edge_" << e.prev;
    if (e.face != BLANK) o << "\n          left incident face=" << e.face;

    return o;
}


// implementation note:  html_table() is heavily coupled to details here.
std::ostream& operator<<(std::ostream& o, const Doubly_connected_edge_list& d)
{
    o << "Complexity:  " << d.get_vertex_table().size() << " vertices, "
        << d.get_edge_table().size() << " edges, "
        << d.get_face_table().size() << " face(s).\n";

    for (size_t i = 0; i < d.get_vertex_table().size(); ++i)
    {
        o << "Vertex_" << i << " : " << d.get_vertex_table().at(i) << '\n';
    }

    for (size_t i = 0; i < d.get_edge_table().size(); ++i)
    {
        o << "Edge_" << i << " : " << d.get_edge_table().at(i) << '\n';
    }

    for (size_t i = 0; i < d.get_face_table().size(); ++i)
    {
        const Doubly_connected_edge_list::Face_record &f
            = d.get_face_table().at(i);
        o << "Face_" << i << " : outer component=";
        if (0 == i) o << "(none)"; else o << "edge_" << f.outer_component;
        o << ", inner component list=(";
        for (size_t j = 0; j < f.inner_components.size(); ++j)
        {
            if (j > 0) o << ", ";
            o << "edge_" << f.inner_components.at(j);
        }
        o << ")\n";
    }

    o << d.get_mutable_status();
    return o;
}


// This merge is evolving to be less brutish.
Doubly_connected_edge_list Doubly_connected_edge_list::brute_force_merge(
    Doubly_connected_edge_list dcel // copy the input, and augment with *this
)   const
{
    KJB(ASSERT(NO_ERROR == is_valid(*this)));
    KJB(ASSERT(NO_ERROR == is_valid(dcel)));

    // Handle trivial input
    if (is_empty(dcel)) return *this;
    if (is_empty(*this)) return dcel;

#if 0
    // common case:  caches are usually invalidated
    dcel.m_roof_table_valid = false;
    // Add new vertices at edge endpoints with bad 'outedge' field.
    const size_t nv_old = dcel.m_vtab.size(); // old number of vertices
    std::set<size_t> bogus_voe; // vertex indices that have bogus 'outedge'
    VtxLookup newbies;
    for (size_t i = 0; i < get_vertex_table().size(); ++i)
    {
        const RatPoint& v = get_vertex_table().at(i).location;
        bool found;
        dcel.lookup_vertex(v, &found);
        if (!found)
        {
            KJB(ASSERT(newbies.end()==newbies.find(v)));
            bogus_voe.insert(dcel.m_vtab.size());
            newbies[v] = dcel.m_vtab.size();
            dcel.m_vtab.push_back(Vertex_record(v, BLANK));
        }
    }

    /* Detect new vertices at edge interiors.  Temporarily store their
     * coordinates in dcel and also in 'newbies' (for faster lookup), also
     * store indices in bogus_voe, because the Vertex Out Edge field is bogus.
     */
#if USE_BENTLEY_OTTMAN
    // A less brutish approach:
    dcel.intersect_red_blue_with_bentley_ottman(*this,&newbies,&bogus_voe);
#else
    // Original approach, actually faster if there are lots of intersections:
    dcel.intersect_red_blue_with_brute_force(*this, &newbies, &bogus_voe);
#endif

    dcel.m_vertex_lookup.insert(newbies.begin(), newbies.end());
    newbies.clear(); // not necessary, just to show we are done with newbies.

    // For each new vertex, if it hits a dcel edge, split it.
    // No need to bother setting up previous and next links --
    // we will do a global fixup later.
    // New vertices are found in the dcel table with indices nv_old and up.
    for (size_t i = nv_old; i < dcel.m_vtab.size(); ++i)
    {
        const RatPoint &v = dcel.m_vtab.at(i).location;
        // search all edge table entries (table can grow, too).
        for (size_t j = 0; j < dcel.m_etab.size(); ++j)
        {
            const RatPoint_line_segment e = get_edge(dcel, j);
            if (!is_on(e, v) || v == e.a || v == e.b) continue;

            // shorten edge j -- j keeps its tail, but k replaces its head.
            const size_t k = dcel.m_etab.size(),
                         jtwin = dcel.m_etab.at(j).twin /*,
                         jnext = dcel.m_etab.at(j).next,
                         jtwinprev = dcel.m_etab.at(jtwin).prev */ ;

            dcel.m_etab.push_back( dcel.m_etab.at(j) ); // copy at k
            // k inherits correct next because it is a copy of j
            dcel.m_etab.at(k).origin = i;
//            dcel.previous_next_link(k, j);
//            dcel.previous_next_link(jnext, k);

            // shorten edge jtwin -- jtwin keeps its head, 1+k replaces tail
            dcel.m_etab.push_back( dcel.m_etab.at(jtwin) ); // copy at 1+k
            dcel.m_etab.at(k).twin = 1+k;
            dcel.m_etab.at(1+k).twin = k;
            dcel.m_etab.at(jtwin).origin = i;
//            dcel.previous_next_link(jtwin, 1+k);

            // prev-next links are tricky when jnext == jtwin.
/*
            if ( jnext == jtwin )
            {
                dcel.m_etab.at(k).next = 1+k;
            }
            else
            {
                dcel.previous_next_link(1+k, jtwinprev);
            }
*/

            // cycle table is spoiled
            dcel.m_cycle_table_valid = false;

            // Fix up vertex 'outedge' fields
            dcel.m_vtab.at( dcel.m_etab.at(1+k).origin ).outedge = 1+k;
            dcel.m_vtab.at(i).outedge = jtwin;
            bogus_voe.erase(i);
        }
    }

    // for each new edge, string it between all the dcel vertices it hits
    for (size_t i = 0; i < get_edge_table().size(); ++i)
    {
        const RatPoint_line_segment e = get_edge(*this, i);
        // collect all vertex indices that e hits
        typedef std::map< RatPoint::Rat, size_t > DR;
        DR vhits;
        for (size_t j = 0; j < dcel.m_vtab.size(); ++j)
        {
            const RatPoint& v = dcel.m_vtab.at(j).location;
            // v might still have a bogus 'outedge' field, unfortunately.
            if (is_on(e, v)) vhits[distance2( e.a, v )] = j;
        }
        KJB(ASSERT(vhits.size() > 1));
        for (DR::const_iterator j1 = vhits.begin(), j2 = j1++;
                                j1 != vhits.end(); j2 = j1++)
        {
            // is there already is an edge between them?
            const size_t v1 = j1 -> second, v2 = j2 -> second;
            const bool legit1 = bogus_voe.find(v1)==bogus_voe.end(),
                       legit2 = bogus_voe.find(v2)==bogus_voe.end();
            if (legit1 && legit2 && is_edge_btw_vxs(dcel, v1, v2)) continue;

            // add new edge between v1, v2
            const size_t ne = dcel.m_etab.size();
            dcel.m_etab.push_back(Edge_record(v1, ne+1));
            dcel.m_etab.push_back(Edge_record(v2, ne  ));
            dcel.m_vtab.at(v1).outedge = ne;
            dcel.m_vtab.at(v2).outedge = ne+1;
            if (!legit1) bogus_voe.erase(v1);
            if (!legit2) bogus_voe.erase(v2);

            // Faces might be wrong since a face might have been split.
            dcel.m_cycle_table_valid = false;
        }
    }
    KJB(ASSERT(bogus_voe.empty()));
#else
    // New routine is now smarter than brute force.
    dcel.sweep_edge_merge(*this);
#endif

#ifdef DEBUGGING
    const std::set< RatPoint > vv = vertices_valid(dcel);
    if (vv.empty()) KJB_THROW_2(Runtime_error, "Bad vertices during merge.\n"
                                                + kjb_get_error() );
#endif

    // Repair previous and next fields throughout edge table.
    // This, too, is less brutish than the original implementation.
    previous_next_fixup(&dcel.m_etab, dcel);

#ifdef DEBUGGING
    const bool ed_valid = edges_valid(dcel);
    if (!ed_valid) KJB_THROW_2(Runtime_error, "Bad edges during merge.\n"
                                                + kjb_get_error() );
#endif

    // update faces
    dcel.rebuild_face_table();

    KJB(ASSERT(NO_ERROR == is_valid(dcel)));
    return dcel;
}


int is_valid(const Doubly_connected_edge_list& dcel)
{
    using kjb_c::ERROR;
    if (dcel.get_edge_table().empty()) return is_tiny_valid(dcel);

    // test vertices in a separate routine
    const std::set< RatPoint > vertices = vertices_valid(dcel);
    if (vertices.empty()) return ERROR;

    // test edges in a separate routine
    const bool ed_valid = edges_valid(dcel);
    if (!ed_valid) return ERROR;

    // test cycles
    const bool cy_valid = cycles_valid(dcel);
    if (!cy_valid) return ERROR;

    // test faces
    const bool fs_valid = faces_valid(dcel);
    return fs_valid ? kjb_c::NO_ERROR : ERROR;
}



// This takes time O(n) in the size of the vertex table -- that's pretty quick.
Doubly_connected_edge_list& Doubly_connected_edge_list::translate(
    const RatPoint& delta
)
{
    if (0==delta.x && 0==delta.y) return *this;

    const size_t NV = m_vtab.size();
    for (size_t i = 0; i < NV; ++i)
    {
        Vertex_record& v = m_vtab.at(i);
        v.location = RatPoint(v.location.x + delta.x, v.location.y + delta.y);
    }
    m_lookup_valid = m_lookup_valid && 0==NV; // RHS is probably false usually
    return *this;
}


// shift every vertex using the given affine transform (a homogeneous matrix)
Doubly_connected_edge_list& Doubly_connected_edge_list::transform(
    const RatPoint::Rat xform[]
)
{
    const size_t NV = m_vtab.size();
    if (NV > 0)
    {
        RatPoint::Rat det = xform[0] * (xform[4]*xform[8] - xform[5]*xform[7]);
        det -= xform[1] * (xform[3]*xform[8] - xform[5]*xform[6]);
        det += xform[2] * (xform[3]*xform[7] - xform[4]*xform[6]);
        if (0 == det) KJB_THROW_2(Illegal_argument,"Affinity lacks full rank");

        for (size_t i = 0; i < NV; ++i)
        {
            Vertex_record& v = m_vtab.at(i);
            const RatPoint& p = v.location;
            const RatPoint::Rat D = p.x * xform[6] + p.y * xform[7] + xform[8];
            if (0 == D) KJB_THROW_2(Illegal_argument, "Div by zero in xform");

            v.location = RatPoint((p.x*xform[0] + p.y*xform[1] + xform[2])/D,
                                  (p.x*xform[3] + p.y*xform[4] + xform[5])/D);
        }
        // The following test is a bit silly; RHS is probably false.
        // It will be true if xform is kI for nonzero k, m_lookup_valid true.
        m_lookup_valid =    m_lookup_valid
                         && xform[0] == xform[4] && xform[4] == xform[8]
                         && xform[0] != 0 && 0 == xform[1] && 0 == xform[2]
                         && 0 == xform[3] && 0 == xform[5] && 0 == xform[6]
                         && 0 == xform[7];

        m_roof_table_valid = false;
    }
    return *this;
}



/*
 * Implementation:
 * Walk the cycle.  For each edge in the cycle, if each edge is "twincident"
 * to the same face as the face of ei, then the cycle traces a stick figure,
 * i.e., a set of points with no area.
 */
bool is_edge_of_stick_figure(const Doubly_connected_edge_list& d, size_t ei)
{
    if (ei >= d.get_edge_table().size()) KJB_THROW(Index_out_of_bounds);
    const size_t my_face = d.get_edge_table().at(ei).face;

    // cycle-scan idiom
    for (size_t ej=d.get_edge_table().at(ei).next; ej != ei;
                ej=d.get_edge_table().at(ej).next)
    {
        const size_t ejtwin = d.get_edge_table().at(ej).twin;
        if (my_face != d.get_edge_table().at(ejtwin).face) return false;
    }
    return true;
}



Doubly_connected_edge_list Doubly_connected_edge_list::ctor_open_path(
    const std::vector<RatPoint>& path
)
{
    Doubly_connected_edge_list d;

    // trivial input
    if (path.size() < 2) return d;
    const RatPoint_line_segment s0(path[0], path[1]);
    if (2 == path.size()) return s0;

    // Compute intersections -- are they merely trivial?
    std::vector< RatPoint_line_segment > sl(path.size()-1, s0);
    for (size_t i = 1; i < path.size(); ++i)
    {
        sl[i-1].b = path[i];
        if (i < path.size()-1) sl[i].a = path[i];
    }
    const std::vector< std::pair<size_t, size_t> > iv = get_intersections(sl);
    KJB(ASSERT(iv.size()+1 >= sl.size()));
    // compute compound AND of trivality-tests
    bool trivial = iv.size()+1 == sl.size();
    RatPoint_line_segment c(s0);
    for (size_t i = 0; trivial && i < iv.size(); ++i)
    {
        const size_t j = iv[i].first;
        KJB(ASSERT(j < iv[i].second && iv[i].second < sl.size()));
        // && clause below does not need parens but mutes nagging compiler
        if (   iv[i].second != 1+j
            || (segment_intersection(sl[j], sl[1+j], &c) && !is_degenerate(c)))
        {
            trivial = false;
        }
    }

    if (trivial)
    {
        // Importus gracilis
        graceful_import(path, & d.m_vtab, & d.m_etab);
        d.m_ftab[0].inner_components.assign(1, 0);
        d.m_lookup_valid = 0;
        d.m_cycle_table_valid = 0;
        d.m_roof_table_valid = 0;
    }
    else
    {
        // Importus robustus
#if 0
        for (size_t i = 0; i < path.size()-1; ++i)
        {
            Doubly_connected_edge_list
                s(RatPoint_line_segment(path[i], path[i+1])),
                e(s.merge(d));
            d.swap(e);
        }
#else
        Doubly_connected_edge_list e = ctor_from_edge_list(sl);
        d.swap(e);
#endif
    }
    KJB(ASSERT(NO_ERROR == is_valid(d)));
    return d;
}


Doubly_connected_edge_list Doubly_connected_edge_list::ctor_open_path(
    const PixPath& path
)
{
    Doubly_connected_edge_list d;
    if (path.size() < 2) return d;

    if (2 == path.size())
    {
        Doubly_connected_edge_list s(RatPoint_line_segment(path[0], path[1]));
        d.swap(s);
    }
    else if (get_intersections(path, 1).empty())
    {
        graceful_import(
            std::vector< RatPoint >(path.begin(), path.end()),
            & d.m_vtab,
            & d.m_etab);
        d.m_ftab[0].inner_components.assign(1, 0);
        d.m_lookup_valid = 0;
        d.m_cycle_table_valid = 0;
        d.m_roof_table_valid = 0;
    }
    else
    {
#if 0
        for (size_t i = 0; i < path.size()-1; ++i)
        {
            Doubly_connected_edge_list
                s(RatPoint_line_segment(path[i], path[i+1])),
                e(s.merge(d));
            d.swap(e);
        }
#else
        // This replicates some effort but I'd rather have the code reused.
        Doubly_connected_edge_list
            e=ctor_open_path(std::vector< RatPoint >(path.begin(),path.end()));
        d.swap(e);
#endif
    }
    KJB(ASSERT(NO_ERROR == is_valid(d)));
    return d;
}



size_t lookup_edge(
    const Doubly_connected_edge_list& d,
    const RatPoint_line_segment& s
)
{
    const size_t v1 = d.lookup_vertex(s.a), v2 = d.lookup_vertex(s.b);
    if (v1>= d.get_vertex_table().size() || v2>= d.get_vertex_table().size())
    {
        return d.get_edge_table().size();
    }
    std::vector<size_t> o1(out_edges(d, v1)), i1(in_edges(d, v2)), x1;
    std::sort(o1.begin(), o1.end());
    std::sort(i1.begin(), i1.end());
    std::set_intersection(o1.begin(), o1.end(), i1.begin(), i1.end(),
                            std::back_inserter(x1));

    if (x1.empty()) return d.get_edge_table().size();
    KJB(ASSERT(1 == x1.size()));
    return x1.front();
}



RatPoint_line_segment get_aa_bounding_box_diagonal(
    const Doubly_connected_edge_list& dcel
)
{
    if (is_empty(dcel))
    {
        KJB_THROW_2(Illegal_argument, "Empty dcel has no bounding box");
    }

    // Endpoint a is the min,min; b is the max,max.
    RatPoint_line_segment slash(get_edge(dcel, 0));

    for (size_t i = 0; i < dcel.get_vertex_table().size(); ++i)
    {
        const RatPoint& p = dcel.get_vertex_table().at(i).location;

        if (slash.a.x > p.x) slash.a.x = p.x;
        if (slash.a.y > p.y) slash.a.y = p.y;
        if (slash.b.x < p.x) slash.b.x = p.x;
        if (slash.b.y < p.y) slash.b.y = p.y;
    }
    KJB(ASSERT(slash.a.x <= slash.b.x));
    KJB(ASSERT(slash.a.y <= slash.b.y));
    return slash;
}



/**
 * @brief This fills in the roof field of the vertex table.
 *
 * It uses a sweep line algorithm, using the Bentley-Ottman machinery.
 * It's a lot simpler than Bentley-Ottman, but maybe that's not saying much.
 * This computation is important for building the face table efficiently.
 * Time complexity is O(n log n) for n edges.
 */
void Doubly_connected_edge_list::scan_for_roof() const
{
    using namespace kjb::qd::sweep;

    m_roof_table_valid = true;
    m_cycletop.assign(get_number_of_cycles(), m_vtab.size());
    m_vertex_roof.assign(m_vtab.size(), 0);
    if (is_empty(*this)) return;

    // rotate input 90 degrees CCW
    Doubly_connected_edge_list d(*this);
    RatPoint::Rat a[9] = {0, -1, 0, 1, 0, 0, 0, 0, 1};
    d.transform(a);

    // store edges and mapping
    typedef std::pair< size_t, size_t > II;
    std::vector< II > s2e(get_edge_table().size()/2);
    std::vector< RatPoint_line_segment > sl;
    sl.reserve(s2e.size());
    for (size_t ei = 0; ei < get_edge_table().size(); ++ei)
    {
        const size_t itwin = d.get_edge_table().at(ei).twin;
        KJB(ASSERT(ei != itwin));
        KJB(ASSERT(itwin == get_edge_table().at(ei).twin));
        if (itwin < ei) continue;
        s2e.at(sl.size()) = std::make_pair(ei, itwin);
        sl.push_back(rectify(get_edge(d, ei)));
    }

    // set up event queue and sweep line
    typedef SweepLine<Static_segment_map>::SL Sweep;
    typedef Sweep::const_iterator SLCI;
    Static_segment_map ssm(&sl);
    RatPoint_line_segment lbump(sl.front()), rbump(lbump);
    std::set<Event_p> q = fill_queue_get_bumpers(sl, &lbump, &rbump);
    RatPoint sweep_loc = (* q.begin()) -> p;
    Sweep sweep(
        SweepCompare< Static_segment_map >(ssm, &sweep_loc, lbump, rbump));
    sweep.insert(Shemp(Shemp::LBUMP));
    sweep.insert(Shemp(Shemp::RBUMP));
    std::vector< Sweep::iterator > rvs_sweep(sl.size(), sweep.end());

    // Sweep through segment termini (colocated with d's vertices of course)
    while (! q.empty())
    {
        // Code here is similar to that of intersection.cpp
        sweep_loc = (* q.begin()) -> p;
        std::set<size_t> ulma[4]; // [0]=upper, [1]=lower, [2]=middle, [3]=all

        pull_events_here< Static_segment_map >(&q, sweep, ulma, ssm);
        KJB(ASSERT(ulma[2].empty())); // middle set is always empty
        KJB(ASSERT(!ulma[3].empty())); // "all" set is not empty
        // cheapo partial check that uppers and lowers are a partition of all
        // which is another way of saying "no degenerate segments"
        KJB(ASSERT(ulma[0].size() + ulma[1].size() == ulma[3].size()));

        // Look up vertex index
        const II eiet = s2e[*ulma[3].begin()];
#ifdef DEBUGGING
        const RatPoint_line_segment r = get_edge(d, eiet.first);
        KJB(ASSERT(r.a == sweep_loc || r.b == sweep_loc));
#endif
        const size_t
            v1 = d.get_edge_table().at(eiet.first ).origin,
            v2 = d.get_edge_table().at(eiet.second).origin,
            vv = d.get_vertex_table().at(v1).location == sweep_loc ?  v1 : v2;
        KJB(ASSERT(d.get_vertex_table().at(vv).location == sweep_loc));

        // If there is a horizontal segment left of sweep_loc, handle it now.
        const std::vector<size_t> lowers(ulma[1].begin(), ulma[1].end()),
                                  all(ulma[3].begin(), ulma[3].end());
        for (size_t j = 0; j < lowers.size(); ++j)
        {
            if (is_horizontal(sl[lowers[j]]))
            {
                m_vertex_roof.at(vv) = s2e[lowers[j]].first << 1 | 1;
                break; // found the roof (a flagstaff), we're done here.
            }
        }
        // Otherwise use sweepline to obtain left neighbor
        if (0 == (m_vertex_roof.at(vv) & 1)) // vertex has no roof
        {
            const std::pair< SLCI, SLCI >
                eir = get_event_neighbors< Static_segment_map >(
                            sweep, sweep_loc, ssm);
            // Test its Shemp status:
            if (eir.first -> is_true())
            {
                const II ejet = s2e[eir.first -> true_index()];
                const RatPoint_line_segment r = get_edge(d, ejet.first);
                // r cannot be horizontal:  it must straddle the sweepline
                KJB(ASSERT(! is_horizontal(r)));
                m_vertex_roof.at(vv)
                    = (r.b < r.a ? ejet.first : ejet.second) << 1 | 1;
            }
            // else *eir.first is the left bumper, so vv lacks a leftward edge.
        }

        /*
         * Examine the cycle membership of each edge incident to this event pt.
         * If we have found its highest vertex, record that in the table.
         * The loop body looks arcane but it just means, the edges could belong
         * to one or two cycles.  Scan for peak of each cycle.
         */
        const RatPoint home_loc(sweep_loc.y, -sweep_loc.x); // unrotate
        for (size_t j = 0; j < all.size(); ++j)
        {
            const II ejet = s2e[all[j]];
            int c[2];
            c[0] = get_cycle(ejet.first);
            c[1] = get_cycle(ejet.second);
            for (int k = c[0]==c[1] ? 0 : 1; k >= 0; --k)
            {
                size_t &w = m_cycletop.at(c[k]);
                if (m_vtab.size()==w || m_vtab.at(w).location < home_loc) w=vv;
            }
        }

        update_sweep< Static_segment_map >(&sweep, ulma, &rvs_sweep);
    }

    // cycletop table is full of valid values; no sentinels remain.
    KJB(ASSERT(std::find(m_cycletop.begin(), m_cycletop.end(), m_vtab.size())
                == m_cycletop.end()));
}



Doubly_connected_edge_list ctor_from_edge_list(
    const std::vector<RatPoint_line_segment>& el
)
{
    if (el.empty())
    {
        return Doubly_connected_edge_list();
    }
    return ctor_from_el(& el.front(), el.size());
}



std::string xml_output(const Doubly_connected_edge_list& dcel)
{
    std::ostringstream ooo;
    ooo << "<?xml version='1.0' ?>\n";
    BUILD_XML_ELT(ooo, XML_TAG_DCEL, "\n" << xml_dcel_guts(dcel));
    return ooo.str();
}


Doubly_connected_edge_list Doubly_connected_edge_list::ctor_xml_stream(
    std::istream& ist
)
{
    typedef boost::property_tree::ptree::value_type PTV;

    if (ist.bad()) KJB_THROW_2(IO_error, "Cannot build from bad stream");
    Doubly_connected_edge_list dcel;
    const size_t z = 0;
    boost::property_tree::ptree pt;
    boost::property_tree::read_xml( ist, pt );

    // Read vertices
    BOOST_FOREACH(PTV const& v,pt.get_child(XML_TAG_DCEL "." XML_TAG_VERTICES))
    {
        // validate index attribute
        const size_t index = v.second.get("<xmlattr>.index", size_t(BLANK));
        if (BLANK == index) KJB_THROW_2(IO_error, "No vertex index");
        if (dcel.m_vtab.size() != index)
        {
            KJB_THROW_2(IO_error, "Bad vertex index");
        }

        // read vertex fields
        const RatPoint::Rat_integral_t
            rz = 0,
            xn = v.second.get( XML_TAG_LOCATION "." XML_TAG_X "."
                    XML_TAG_RATIONAL "." XML_TAG_NUMERATOR, rz),
            xd = v.second.get( XML_TAG_LOCATION "." XML_TAG_X "."
                    XML_TAG_RATIONAL "." XML_TAG_DENOMINATOR, rz),
            yn = v.second.get( XML_TAG_LOCATION "." XML_TAG_Y "."
                    XML_TAG_RATIONAL "." XML_TAG_NUMERATOR, rz),
            yd = v.second.get( XML_TAG_LOCATION "." XML_TAG_Y "."
                    XML_TAG_RATIONAL "." XML_TAG_DENOMINATOR, rz);
        if (0 == xd || 0 == yd) KJB_THROW_2(IO_error, "No vertex location");
        const RatPoint::Rat x(xn, xd), y(yn, yd);
        dcel.m_vtab.push_back(
            Vertex_record(
                RatPoint(x,y),
                v.second.get( XML_TAG_OUTEDGE, size_t(0))));
    }

    // Read edges
    BOOST_FOREACH( PTV const& v, pt.get_child(XML_TAG_DCEL "." XML_TAG_EDGES))
    {
        // validate index attribute
        const size_t index = v.second.get("<xmlattr>.index", size_t(BLANK));
        if (BLANK == index) KJB_THROW_2(IO_error, "No edge index");
        if (dcel.m_etab.size() != index)
        {
            KJB_THROW_2(IO_error, "Bad edge index");
        }

        // read edge fields
        if (v.first != XML_TAG_EDGE) KJB_THROW(IO_error);
        dcel.m_etab.push_back( Edge_record(
            v.second.get(XML_TAG_ORIGIN, z),
            v.second.get(XML_TAG_TWIN, z),
            v.second.get(XML_TAG_INCFACE, z),
            v.second.get(XML_TAG_NEXT, z),
            v.second.get(XML_TAG_PREV, z)));
    }

    bool face_0 = true;
    BOOST_FOREACH( PTV const& v, pt.get_child( XML_TAG_DCEL "." XML_TAG_FACES))
    {
        // validate index attribute
        const size_t index = v.second.get("<xmlattr>.index", size_t(BLANK));
        if (BLANK == index) KJB_THROW_2(IO_error, "No face index");

        if (face_0)
        {
            if (index != 0) KJB_THROW_2(IO_error, "Bad face 0 index");
        }
        else if (dcel.m_ftab.size() != index)
        {
            KJB_THROW_2(IO_error, "Bad face index");
        }

        if (v.first != XML_TAG_FACE)
        {
            KJB_THROW_2(IO_error, "Missing dcel.faces.face elt");
        }

        // face 0 record already exists in dcel, just need to populate it.
        if (!face_0)
        {
            dcel.m_ftab.push_back( Face_record(
                v.second.get( XML_TAG_OCOMPONENT "." XML_TAG_EDGEINDEX, z)));
        }

        BOOST_FOREACH( PTV const& w, v.second.get_child(XML_TAG_ICOMPONENTS))
        {
            if (w.first != XML_TAG_EDGEINDEX)
            {
                KJB_THROW_2(IO_error,
                    "Missing dcel.faces.face.icomponents.edgeindex elt");
            }
            std::istringstream stedix( w.second.data() );
            size_t edge_index;
            if (!(stedix >> edge_index))
            {
                KJB_THROW_2(IO_error,
                    "Bad dcel.faces.face.icomponents.edgeindex elt");
            }
            dcel.m_ftab.back().inner_components.push_back(edge_index);
        }
        face_0 = false;
    }

    if (!is_empty(dcel))
    {
        dcel.m_cycle_table_valid = false;
        dcel.m_roof_table_valid = false;
        dcel.m_lookup_valid = false;
    }

    KJB(ASSERT(NO_ERROR == is_valid(dcel)));
    return dcel;
}


bool operator==(
    const Doubly_connected_edge_list& d,
    const Doubly_connected_edge_list& e
)
{
    if (    d.get_vertex_table().size() != e.get_vertex_table().size()
        ||  d.get_edge_table().size() != e.get_edge_table().size()
        ||  d.get_face_table().size() != e.get_face_table().size())
    {
        return false;
    }
    for (size_t i = 0; i < d.get_vertex_table().size(); ++i)
    {
        const Doubly_connected_edge_list::Vertex_record
            &vd = d.get_vertex_table().at(i),
            &ve = e.get_vertex_table().at(i);
        if (vd.outedge != ve.outedge || vd.location != ve.location)
        {
            return false;
        }
    }
    for (size_t i = 0; i < d.get_edge_table().size(); ++i)
    {
        const Doubly_connected_edge_list::Edge_record
            &ed = d.get_edge_table().at(i),
            &ee = e.get_edge_table().at(i);
        if (    ed.origin != ee.origin
            ||  ed.twin != ee.twin
            ||  ed.face != ee.face
            ||  ed.next != ee.next
            ||  ed.prev != ee.prev)
        {
            return false;
        }
    }
    for (size_t i = 0; i < d.get_face_table().size(); ++i)
    {
        const Doubly_connected_edge_list::Face_record
            &fd = d.get_face_table().at(i),
            &fe = e.get_face_table().at(i);
        if (i > 0 && fd.outer_component != fe.outer_component)
        {
            return false;
        }
        if (fd.inner_components != fe.inner_components)
        {
            return false;
        }
    }
    return true;
}


bool is_isomorphic(
    const Doubly_connected_edge_list& a,
    const Doubly_connected_edge_list& b,
    std::vector<size_t>* p_isomorphism // optional output param for certificate
)
{
    const size_t NV=a.get_vertex_table().size(), NE=a.get_edge_table().size();

    if (    NE != b.get_edge_table().size()
        ||  NV != b.get_vertex_table().size()
        ||  a.get_face_table().size() != b.get_face_table().size()
       )
    {
        // gross discrepancy
        return false;
    }

    // Test for isomorphism at each vertex.
    std::vector<size_t> hab(NE, NE);
    for (size_t iva = 0; iva < NV; ++iva)
    {
        using kjb::qd::sweep::Event_point;

        const size_t
            ivb = b.lookup_vertex(a.get_vertex_table().at(iva).location);
        // not found?
        if (ivb >= b.get_vertex_table().size()) return false;

        // Get lists of outedges from iva, ivb with distal end raster-sorted.
        std::vector<Event_point> q[2];
        const Doubly_connected_edge_list* d[2] = {&a, &b};
        const size_t vx[2] = {iva, ivb};
        for (int i = 0; i < 2; ++i)
        {
            for (std::vector<size_t> l = out_edges(*d[i], vx[i]);
                    ! l.empty(); l.pop_back() )
            {
                q[i].push_back( Event_point(
                        // Location of this edge's distal terminus:
                        d[i] -> get_vertex_table().at(
                                d[i] -> get_edge_table().at(
                                        d[i] -> get_edge_table().at(
                                                l.back() // this edge's index
                                            ).twin // this edge's twin
                                    ).origin // this edge's distal vertex index
                            ).location,
                        // Index of this edge:
                        l.back()
                    ));
            }
            std::sort(q[i].begin(), q[i].end());
        }
        if (q[0].size() != q[1].size()) return false; // different edge degrees

        // compare qa, qb -- they should match
        while (!q[0].empty())
        {
            if (q[0].back().p != q[1].back().p) return false;
            hab[q[0].back().index] = q[1].back().index;//build edge permutation
            q[0].pop_back();
            q[1].pop_back();
        }
    }

    // if optional output param is provided, give it the certificate.
    if (p_isomorphism) p_isomorphism -> swap(hab);

    return true;
}


Doubly_connected_edge_list Doubly_connected_edge_list::face_export(
    size_t fi
)   const
{
    typedef std::map< size_t, size_t >::iterator UUMI;

    if (0 == fi) KJB_THROW_2(Illegal_argument, "cannot export face zero");
    if (fi >= m_ftab.size()) KJB_THROW(Illegal_argument);

    Doubly_connected_edge_list d;
    d.m_lookup_valid = d.m_cycle_table_valid = d.m_roof_table_valid = false;

    // old-to-new map for vertex indices and edge indices
    std::vector< size_t > vm(get_vertex_table().size(), BLANK),
                          em(get_edge_table().size(), BLANK),
                          fm(get_face_table().size(), 0);

    for ( std::vector<size_t> f = get_edges_map_faces(*this, fi, &fm);
            ! f.empty(); f.pop_back() )
    {
        // If this edge is already mapped, skip (and twin must be mapped too).
        if (em[f.back()] < BLANK)
        {
            KJB(ASSERT(em[get_edge_table().at(f.back()).twin] < BLANK));
            continue;
        }

        // eo = edges in old dcel, en = new dcel, index-0=on-fi, index-1=twin.
        size_t eo[2], en[2];
        eo[0] = f.back(); // old edge index, incident to face fi
        KJB(ASSERT(get_edge_table().at(eo[0]).face == fi));
        eo[1] = get_edge_table().at(eo[0]).twin;
        KJB(ASSERT(BLANK == em[eo[1]])); // Twin must also be absent.
        en[1] = en[0] = d.m_etab.size(); // new edge indices
        en[1] += 1;
        for (size_t j = 0; j < 2; ++j) // mimic edge on face fi, then its twin.
        {
            // old and new vertex indices
            size_t vo = get_edge_table().at(eo[j]).origin, vn;

            // Have we never seen this vertex before?
            if (BLANK == vm[vo])
            {
                // Never, so create new vertex record.
                vm[vo] = vn = d.m_vtab.size();
                d.m_vtab.push_back(
                    Vertex_record(
                        get_vertex_table().at(vo).location, en[j]));
            }
            else
            {
                // We have seen it (outer border must not be simple).
                vn = vm[vo];
            }

            // Create new edge record
            em[eo[j]] = en[j];
            const size_t fn = fm.at(get_edge_table().at(eo[j]).face);
            d.m_etab.push_back(
                Edge_record(vn, en[1-j], fn, BLANK, BLANK));
        }
    }

    previous_next_fixup(&d.m_etab, d);
    d.rebuild_face_table();

    KJB(ASSERT(NO_ERROR == is_valid(d)));

    return d;
}


inline void Doubly_connected_edge_list::previous_next_link(
    size_t e_engine,
    size_t e_caboose
)
{
    previous_next_link_impl(e_engine, e_caboose, &m_etab);
}



// This requires vertex and edge tables to be accurate, and rebuilds face
// table.  Time complexity is O(n log n) for n edges.
void Doubly_connected_edge_list::rebuild_face_table()
{
    typedef std::pair< MMII::iterator, MMII::iterator > MMR;

    // identify inner cycles (clockwise, and not necessarily surrounding area)
    std::set<int> cycle_hits, ics; // inner cycles (subset of cycles)
    for (size_t i = 0; i < m_etab.size(); ++i)
    {
        const int c = get_cycle(i);
        if (cycle_hits.find(c) != cycle_hits.end()) continue;
        cycle_hits.insert(c);
        if (! is_on_outer_border(*this, i)) ics.insert(c);
    }

    /* Get face topology.  Index fcomponents by cycle number, get its face
     * component, except for this:  the last entry corresponds to "the
     * pseudocycle," a token entry for an imaginary outer component around
     * everything.
     */
    const std::vector<int> vic(ics.begin(), ics.end()),
                           fcomponents( get_facial_components(*this, &vic) );

    /* Store the cycle for each face component, but do NOT store the index of
     * the last entry in fcomponents because it is not a real cycle number, it
     * is the index assigned to the "pseudocycle" that acts like the phantom
     * outer boundary around all edges, and is in the connected component of
     * the index-0 face.  Thus the weird "+1" in the for-loop test condition.
     */
    MMII c_of_k; // reverse lookup:  cycles of component k (key)
    for (int cyc = 0; cyc+1 < (int)fcomponents.size(); ++cyc)
    {
        // Store face (i.e., connected-component) info for this cycle.
        const int k = fcomponents.at(cyc);
        c_of_k.insert(std::make_pair(k, cyc));
    }

    // Revise the face table based on the face topology.  Throw out old table.
    while (m_ftab.size() > 1) m_ftab.pop_back();
    Doubly_connected_edge_list::Face_record& f0 = m_ftab.at(0);
    f0.inner_components.clear();

    // Step 1.  Handle index-0 face separately from later faces (if any)
    const int k_outer = fcomponents.back(); // last entry is for pseudocycle
    MMR r_outer = c_of_k.equal_range(k_outer);
    for (MMII::const_iterator r = r_outer.first; r != r_outer.second; ++r)
    {
        KJB(ASSERT(k_outer == r -> first));
        const int c = r -> second, ei = get_edge_of_cycle(c);
        KJB(ASSERT(ics.find(c) != ics.end()));
        KJB(ASSERT(ei >= 0));
        f0.inner_components.push_back(ei);
        walk_cycle_and_set_its_face(ei, 0);
    }
    c_of_k.erase(r_outer.first, r_outer.second);

    // Step 2.  Do the other faces
    for (MMR r; !c_of_k.empty(); c_of_k.erase(r.first, r.second))
    {
        r = c_of_k.equal_range(c_of_k.begin() -> first);
        const size_t fx = m_ftab.size();
        m_ftab.push_back(Face_record(0)); // add new face to face table.
        bool outer_set = false;
        // loop through all cycles of this face
        for (MMII::const_iterator s = r.first; s != r.second; ++s)
        {
            const int c = s -> second, ei = get_edge_of_cycle(c);
            KJB(ASSERT(ei >= 0));
            if (ics.end() == ics.find(c))
            {
                m_ftab.back().outer_component = ei;
                outer_set = true;
            }
            else
            {
                m_ftab.back().inner_components.push_back(ei);
            }
            walk_cycle_and_set_its_face(ei, fx);
        }
        KJB(ASSERT(outer_set)); // new face should have an outer boundary
    }
}


/**
 * @brief compute least common denominator of all x and y coordinates.
 *
 * If you enlarge the DCEL this much, the coordinates will be integers.
 *
 * @warning These integers get pretty crazy huge.
 */
RatPoint::Rat common_denominator(const Doubly_connected_edge_list& dcel)
{
    RatPoint::Rat denom(1);
    for (size_t i = 0; i < dcel.get_vertex_table().size(); ++i)
    {
        RatPoint p = dcel.get_vertex_table().at(i).location;
        RatPoint::Rat x(1), y(1), z(1);
        x /= i_denominator(p.x);
        y /= i_denominator(p.y);
        z /= denom;

        denom = i_denominator(x+y+z);
        std::cout << "denom = " << denom << '\n';
    }
    return denom;
}



/**
 * @brief import all vertices and edges of d into *this
 * @post vertex set contains all prior vertices, plus all vertices of
 *       d, plus all vertices induced by edge intersections.
 * @post edges cover all points covered by prior edges, or edges of d.
 * @post edges follow the rules of not overlapping or crossing at an
 *       interior point, and having proper twins, but the previous and
 *       next fields are neglected and in general not correct.
 * @post the face table is unreliable, and in general must be rebuilt.
 *
 * Implementation:  sweep line
 */
void Doubly_connected_edge_list::sweep_edge_merge(
    const Doubly_connected_edge_list& d // input
)
{
    using namespace sweep;

    // add foreign vertices
    KJB(ASSERT(!m_lookup_valid || m_vtab.size() == m_vertex_lookup.size()));
    for (size_t i = 0; i < d.get_vertex_table().size(); ++i)
    {
        const RatPoint &vp = d.get_vertex_table().at(i).location;
        const size_t vix = lookup_vertex(vp);
        KJB(ASSERT(vix <= m_vtab.size()));
        if (get_vertex_table().size() == vix)
        {
            // Add new vertex, with bogus outedge field
            m_vtab.push_back(Vertex_record(vp, BLANK));
            m_vertex_lookup[vp] = vix;
        }
    }
    KJB(ASSERT(m_lookup_valid && m_vtab.size() == m_vertex_lookup.size()));

    /*
     * Add foreign edges, though they will fail the no-interior-touching
     * invariants.  After this, we fix the no-interior-touching invariants
     * by sweeping through and fixing.
     */
    for (size_t i = 0; i < d.get_edge_table().size(); ++i)
    {
        const size_t &itwin = d.get_edge_table().at(i).twin;
        KJB(ASSERT(d.get_edge_table().at(itwin).twin == i));
        if (i < itwin)
        {
            const RatPoint_line_segment s = get_edge(d, i);
            const size_t ek = get_edge_table().size(), // new edge index
                         i_origin = lookup_vertex(s.a),
                         i_dest = lookup_vertex(s.b);
            KJB(ASSERT(std::max(i_origin, i_dest) < m_vtab.size()));
            m_etab.push_back(Edge_record(i_origin, 1+ek));
            m_etab.push_back(Edge_record(i_dest, ek));
            m_vtab[i_origin].outedge = ek;
            m_vtab[i_dest].outedge = 1+ek;
        }
    }
    m_cycle_table_valid = false;
    m_roof_table_valid = false;

#ifdef DEBUGGING
    /* Deliberately blot out the prev, next field so their wrongness is
     * more obvious.
     */
    for (size_t i = 0; i < get_edge_table().size(); ++i)
    {
        m_etab[i].prev = m_etab[i].next = m_etab[i].face = BLANK;
    }
#endif

    // Edge data structures used by sweep
    Dcel_edge ded(*this);
    KJB(ASSERT(! is_empty(*this)));
    RatPoint_line_segment lbump(ded.lookup(0)), rbump(lbump);

    /*
     * Load up the event queue.
     * - First load it up with bad indices, using fill_queue_get_bumpers().
     * - Queue stores an event for a segment's first endpoint, i.e., for half
     *   the half-edges.  Not all the half-edges: we skip twins systematically.
     * - Skipping twins causes the indices to be incorrect when we use the
     *   old fill_queue_get_bumpers() function, but we prefer to reuse code.
     * - Next, scan the queue and fix the indices.
     */
    typedef std::set< Event_p > EventQueue;
    EventQueue q;

    do {
        std::vector<RatPoint_line_segment> sls;
        std::vector<size_t> sref(get_edge_table().size()/2);
        std::vector<size_t>::iterator j = sref.begin();
        for (size_t i = 0; i < get_edge_table().size(); ++i)
        {
            if (i < m_etab.at(i).twin)
            {
                sls.push_back(get_edge(*this, i));
                *j++ = i;
            }
        }
        KJB(ASSERT(sref.end() == j));
        // The extra "1" means to leave blank the exit event's index field.
        EventQueue r( fill_queue_get_bumpers(sls, &lbump, &rbump, 1) );

        // Repair the non-blank indices.
        for (EventQueue::iterator i = r.begin(); i != r.end(); ++i)
        {
            if ((*i) -> index != SWEEP_BLANK)
            {
                (*i) -> index = sref.at((*i) -> index);
#ifdef DEBUGGING
                const RatPoint_line_segment s = ded.lookup((*i) -> index);
                KJB(ASSERT( s.a == (*i)->p || s.b == (*i)->p ));
#endif
            }
        }
        q.swap(r);
    } while(0);

    /* Some half-edges may turn out to be duplicates.  It's too hard to delete
     * edges during the sweep -- too hard to move up records from the tail,
     * considering they might also be listed in the sweep line, in the event
     * queue, and in the reverse indexes.  The duplicate edges do not hurt
     * anything during the remaining sweep, so we just identify them on this
     * list and delete them when the sweep is over.  This list then must be
     * processed from back to front (decreasing index) because we move
     * replacement edges into the vacated spaces from the back.  By killing
     * the condemned in this order, we avoid the gaffe of moving a bad edge
     * before we kill it.
     *
     * For each edge index i and its twin itwin, kill_list[i] and
     * kill_list[itwin] both are BLANK iff edge i and its twin are to be
     * preserved.  Otherwise one of them set to edge index j, where edge j and
     * the twin of j are a pair that is a duplicate of pair i and itwin.
     * Only one edge in the twin-pair is so marked, but both i and itwin are
     * replaced -- their fields are overwritten by the fields of the last
     * two edges in the edge table, appropriate adjustments are performed,
     * and then the table is shortened by two.
     */
    std::vector<size_t> kill_list(m_etab.size(), BLANK);

    // Set up sweep line
    typedef SweepLine< Dcel_edge > Sweep;
    typedef Sweep::const_iterator SLCI;
    RatPoint sweep_loc = (* q.begin()) -> p;
    Sweep::SL sweep( SweepCompare< Dcel_edge >(ded, &sweep_loc, lbump, rbump));
    sweep.insert(Shemp(Shemp::LBUMP));
    sweep.insert(Shemp(Shemp::RBUMP));
    std::vector< Sweep::iterator >
        rvs_sweep(get_edge_table().size(), sweep.end());

    // process events -- this is a lot like Bentley-Ottman
    while (! q.empty())
    {
        sweep_loc = (* q.begin()) -> p; // update the sweep location
        std::set<size_t> ulma[4]; // [0]=upper, [1]=lower, [2]=middle, [3]=all
        pull_events_here(&q, sweep, ulma, ded);
        KJB(ASSERT(ulma[3].size() > 0));
        // the following is true because DCEL has no degenerate segments.
        KJB(ASSERT(ulma[3].size()
                    == ulma[0].size() + ulma[1].size() + ulma[2].size()));

        /* If ulma is middle-only, Search for vertex at this event point.
         * Search might fail.
         */
        KJB(ASSERT(m_lookup_valid && m_vtab.size() == m_vertex_lookup.size()));
        if (ulma[0].empty() && ulma[1].empty())
        {
            // We have discovered the site of a new vertex.  Congratulations.
            KJB(ASSERT(ulma[2].size() >= 2));
            m_vertex_lookup[sweep_loc] = m_vtab.size();
            m_vtab.push_back(Vertex_record(sweep_loc, BLANK));
        }
        const size_t vix = lookup_vertex(sweep_loc);
        KJB(ASSERT(m_vtab.at(vix).location == sweep_loc));

        /* Middle-striking segments must be split, in 5 steps.
         * 1) remove it from the sweep line,
         * 2) shorten the existing halfedge-pair
         * 3) insert short version into sweep line
         * 4) create new edges to cover points below sweepline
         * 5) adjust ulma to reflect the current geometry and sweep status
         */
        for ( std::vector<size_t> mid(ulma[2].begin(), ulma[2].end());
                ! mid.empty(); mid.pop_back())
        {
            const size_t i = mid.back();
            // 1) remove long version from sweep line
            KJB(ASSERT(rvs_sweep.at(i) != sweep.end()));
            sweep.erase( rvs_sweep.at(i) );
            rvs_sweep.at(i) = sweep.end();
            // 2) shorten existing halfedge-pair
            const RatPoint_line_segment s = get_edge(*this, i);
            // sweep_loc is an interior point of s
            KJB(ASSERT(is_on(s, sweep_loc)));
            KJB(ASSERT(std::min(s.a, s.b) < sweep_loc));
            KJB(ASSERT(sweep_loc < std::max(s.a, s.b)));
            const size_t itwin = m_etab.at(i).twin,
                         // edge with origin not hit yet:
                         icut = s.a < sweep_loc ? itwin : i,
                         // ...but that is about to change.
                         ocut = m_etab.at(icut).origin,
                         j = m_etab.size();
            KJB(ASSERT(std::max(s.a, s.b) == m_vtab.at(ocut).location));
            KJB(ASSERT(ulma[2].end() == ulma[2].find(itwin))); // twin omitted
            m_etab.at(icut).origin = vix; // this shortens edge icut
            // vix might be new.  If so, it needs a valid outedge:
            m_vtab.at(vix).outedge = icut;
            // 3) insert short version back into sweep line
            std::pair< Sweep::SL::iterator, bool >
                y = sweep.insert( Shemp::ctor_from_true(i) );
            KJB(ASSERT(y.second));
            rvs_sweep.at(i) = y.first;
            // 4) create new edges
            //    New edge j dangles below sweepline.
            m_etab.push_back(Edge_record(vix, 1+j));
            kill_list.push_back(BLANK); // each edge gets a kill list entry
            m_etab.push_back(Edge_record(ocut, j)); // new twin
            // If ocut is bereaved, it needs an accurate outedge:
            m_vtab.at(ocut).outedge = 1+j;
            kill_list.push_back(BLANK);
            KJB(ASSERT(m_etab.size() == 2+j));
            KJB(ASSERT(kill_list.size() == m_etab.size()));
            // Each edge gets a reverse-lookup entry for the sweepline:
            rvs_sweep.resize(2+j, sweep.end());
            // TODO: check return values
            // 5) adjust ulma
            ulma[0].insert(j); // upper end of new edge j should be inserted
            ulma[1].insert(i); // edge i was a "middle" (that's how we got i),
            ulma[2].erase(i); //  . . . but it has become a "lower."
        }
        KJB(ASSERT(ulma[2].empty()));

        // Low-end segments need to be tested in case any of them are parallel.
        // If so, throw away one of them -- add to kill_list
        if (ulma[1].size() > 1)
        {
            std::vector< Event_point > hi_distal;
            for ( std::vector< size_t > l(ulma[1].begin(), ulma[1].end());
                    !l.empty(); l.pop_back())
            {
                hi_distal.push_back(
                    Event_point(ded.lookup(l.back()).a, l.back()));
            }
            std::sort(hi_distal.begin(), hi_distal.end());
            for (size_t i = 1; i < hi_distal.size(); ++i)
            {
                if (hi_distal[i-1].p == hi_distal[i].p)
                {
                    // kill off edge with index hi_distal[i].index:
                    KJB(ASSERT(hi_distal[i].index != hi_distal[i-1].index));
                    kill_list[hi_distal[i].index] = hi_distal[i-1].index;
                    // The twin doesn't need to be marked, but it's doomed too.
                }
            }
        }

        const size_t count=update_sweep< Dcel_edge >(&sweep, ulma, &rvs_sweep);

        // step 8:  find the neighbors of the event point.
        //typedef Sweep::const_iterator SLCI;
        const std::pair< SLCI, SLCI >
                eir = get_event_neighbors< Dcel_edge >(sweep, sweep_loc, ded);
        if (0 == count)
        {
            // steps 9-10: no event-point intersectors between the neighbors
            find_new_event(&q, *eir.first, *eir.second, ded, sweep_loc);
#ifdef DEBUGGING
            SLCI bgin(eir.first);
            KJB(ASSERT(++bgin == eir.second));
#endif
        }
        else
        {
            // steps 11-16: one or more intersector segments at event point.
            SLCI li = eir.first, ri = eir.second; // left, right intersectors
            find_new_event(&q, *eir.first, *++li, ded, sweep_loc);
            find_new_event(&q, *--ri, *eir.second, ded, sweep_loc);
            KJB(ASSERT(li != eir.second));
        }
    }

    // Delete redundant segments
    for ( ; ! kill_list.empty(); kill_list.pop_back())
    {
        if (kill_list.back() != BLANK)
        {
            /* - rewrite outedge field of terminal vertices so they forget.
             * - move final two edge entries up to fill their spaces.
             * - update outedge field of *those* terminal vertices.
             */
            const size_t ej = kill_list.size() - 1,
                         ejtwin = get_edge_table().at(ej).twin,
                         // Indices of the doomed's geometric equivalents:
                         ek = kill_list.back(),
                         ektwin = get_edge_table().at(ek).twin,
                         // Indices of the migrating edges:
                         eult = get_edge_table().size() - 1,
                         epenult = eult - 1,
                         eultwin = get_edge_table().at(eult).twin,
                         epenultwin = get_edge_table().at(epenult).twin,
                         // Vertex indices of the termini:
                         vj = get_edge_table().at(ej).origin,
                         vjtwin = get_edge_table().at(ejtwin).origin;

            KJB(ASSERT(ej != ek));
            KJB(ASSERT(ej != ejtwin));
            KJB(ASSERT(ek != ejtwin));
            KJB(ASSERT(ek != ektwin));
            KJB(ASSERT(ej != ektwin));
            KJB(ASSERT(ejtwin != ektwin));
            KJB(ASSERT(ektwin < get_edge_table().size()));
            KJB(ASSERT(ejtwin < get_edge_table().size()));
            KJB(ASSERT(get_edge_table().at(ejtwin).twin == ej));
            KJB(ASSERT(get_edge_table().at(ektwin).twin == ek));
            KJB(ASSERT(get_edge_table().at(eultwin).twin == eult));
            KJB(ASSERT(get_edge_table().at(epenultwin).twin == epenult));
            KJB(ASSERT(m_etab[ek].origin != m_etab[ektwin].origin));
#ifdef DEBUGGING
            const RatPoint_line_segment sj=ded.lookup(ej), sk=ded.lookup(ek);
            KJB(ASSERT(sj.a == sk.a && sj.b == sk.b)); // sj, sk are identical
            KJB(ASSERT(   vj == m_etab[ek].origin
                       || vj == m_etab[ektwin].origin));
            KJB(ASSERT(   vjtwin == m_etab[ek].origin
                       || vjtwin == m_etab[ektwin].origin));
#endif

            // Force vertex table to forget about old ej and ejtwin.
            m_vtab[m_etab[ek].origin].outedge = ek;
            m_vtab[m_etab[ektwin].origin].outedge = ektwin;

            if (eultwin != epenult)
            {
                // The migrants are not a twin pair.  That's ok.
                KJB(ASSERT(epenultwin != eult));
                // Twin fields thus migrate:
                m_etab[ej].twin = epenultwin;
                m_etab[ejtwin].twin = eultwin;
                // Also the migrants' twins need to know their new addresses.
                m_etab[epenultwin].twin = ej;
                m_etab[eultwin].twin = ejtwin;
            }
            else
            {
                // Migrants are a twin pair, so leave linked twin fields
                KJB(ASSERT(eult == epenultwin)); // sensible assertion
                KJB(ASSERT(epenult == eultwin)); // paranoiac assertion
            }
            // Origin fields migrate:
            m_etab[ej].origin = m_etab[epenult].origin;
            m_etab[ejtwin].origin = m_etab[eult].origin;
            // Force vertex table to forget about epenult, eult.
            m_vtab[m_etab[ej].origin].outedge = ej;
            m_vtab[m_etab[ejtwin].origin].outedge = ejtwin;
            // We've copied everything good, so let the old records go.
            m_etab.pop_back();
            m_etab.pop_back();
            KJB(ASSERT(get_edge_table().size() == epenult));
        }
    }
}


std::string Doubly_connected_edge_list::get_mutable_status() const
{
    const char *good="good", *bad="bad";

    return std::string("Mutables: vertex_lookup=")+(m_lookup_valid ? good:bad)
       + ", cycle_table=" + (m_cycle_table_valid ? good : bad)
       + ", vertex_roof=" + (m_roof_table_valid ? good : bad)
       + "\n";
}



// implementation note:  heavily coupled to details of operator<< for
// vertex, edge, and whole DCEL.
std::string html_table(const Doubly_connected_edge_list& d)
{
    std::ostringstream o, p;

    p << d;
    o << "<table><caption>" << p.str().substr(0, p.str().find("\n"))
      << "</caption>\n"
      "<tr><td>Vertices</td>"
      "<td>Location</td><td></td><td></td><td></td>" // x, y, x, y
      "<td>Outedge</td></tr>\n";
    for (size_t i = 0; i < d.get_vertex_table().size(); ++i)
    {
        o << "<tr><td>" << i << "</td><td>";
        std::ostringstream r;
        r << d.get_vertex_table().at(i);
        std::string s = r.str();
        s = s.substr(s.find("location=")+9);
        o << s.substr(0, s.find(",")) << ",</td><td>"; // rational x
        s = s.substr(s.find(",")+1);
        o << s.substr(0, s.find("=")) << "</td><td>"; // rational y
        s = s.substr(s.find("=(")+1);
        o << s.substr(0, s.find(",")) << ",</td><td>"; // decimal x
        s = s.substr(s.find(",")+1);
        o << s.substr(0, s.find(")")) << ")</td><td>"  // decimal y
          << s.substr(s.find("edge_")+5) << "</td></tr>\n";
    }

    o << "<tr><td></td></tr><tr><td>Edges</td><td>origin</td><td>twin</td>"
         "<td>next</td><td>previous</td><td>left inc face</td></tr>\n";
    for (size_t i = 0; i < d.get_edge_table().size(); ++i)
    {
        o <<"<tr><td>"<< i <<"</td><td>";
        std::ostringstream r;
        r << d.get_edge_table().at(i);
        std::string s = r.str();
        s = s.substr(s.find("vertex_")+7);
        o << s.substr(0, s.find(",")) << "</td><td>";
        s = s.substr(s.find("twin=edge_")+10);
        o << s.substr(0, s.find(",")) << "</td>"; // print twin
        const size_t n = s.find("next=edge_");
        if (n >= s.size()) { o << "</tr>\n"; continue; }
        s = s.substr(n+10); // advance to next
        const size_t p = s.find("previous=edge_");
        if (p >= s.size()) { o << "<td>" << s << "</td></tr>\n"; continue; }
        o << "<td>" << s.substr(0, s.find(",")) << "</td>"; // print next
        s = s.substr(p+14); // advance to previous
        const size_t f = s.find("face=");
        if (f >= s.size()) { o << "<td>" << s << "</td></tr>\n"; continue; }
        o << "<td>" << s.substr(0, s.find("left")) // print previous
          << "</td><td>" << s.substr(f+5) << "</td></tr>\n"; // print face
    }

    o << "<tr></tr><tr><td>Face</td><td>outer component</td>"
            "<td>inner components</td></tr>\n";
    for (size_t i = 0; i < d.get_face_table().size(); ++i)
    {
        o << "<tr><td>" << i << "</td><td>";
        if (i > 0) o << d.get_face_table().at(i).outer_component;
        o << "</td><td>";
        const size_t n = d.get_face_table().at(i).inner_components.size();
        for (size_t j = 0; j < n; ++j)
        {
            o << d.get_face_table().at(i).inner_components.at(j);
            if (j+1 < n) o << ", ";
        }
        o << "</td></tr>\n";
    }

    o << "</table>\n";

    return o.str();
}



}
}

