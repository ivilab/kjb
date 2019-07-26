/**
 * @file
 * @brief declaration of doubly-connected edge list class and helper functions
 * @author Andrew Predoehl
 */
/*
 * $Id: dcel.h 20165 2015-12-09 21:32:07Z predoehl $
 */

#ifndef QD_CPP_DCEL_H_INCLUDED_IVILAB
#define QD_CPP_DCEL_H_INCLUDED_IVILAB 1

#include <l/l_sys_std.h>
#include <l_cpp/l_exception.h>
#include <qd_cpp/ratpoint.h>
#include <qd_cpp/pixpath.h>

#include <vector>
#include <set>
#include <map>
#include <iosfwd>


namespace kjb
{
namespace qd
{


/**
 * @brief data structure for a planar subdivision made by edges
 *
 * @section dcel_overview Quick overview
 *
 * This implements the planar geometric data structure of Muller and Preparata
 * (1978) called the Doubly-connected Edge List, or DCEL for short.  The plane
 * is subdivided by non-degenerate line segments into one or more faces
 * (maximal connected sets of points mutually reachable without crossing
 * any segments).  The segments are organized into paired structures usually
 * called "half-edges," and their endpoints are called vertices.  Every vertex
 * is the endpoint of a line segment.  The key insight of this structure, which
 * we abbreviate as DCEL, is the use of half-edges, which are incident to only
 * one face, do not cross other edges, and linked into non-crossing cycles.
 * Isolated segments are supported, but not isolated vertices.
 * The two key significant computations this code performs are computing the
 * face table and merging two DCELs.
 *
 * The tables of the DCEL are publicly readable, and straightforward.  There
 * are a few class operations supported by the basic structure:
 * - Query the structure's tables: vertices, half-edges, face, and the
 *   cycle-membership of the half-edges.
 * - Merge two DCELs (nontrivial)
 * - Shift all DCEL vertices using a full-rank affine transform
 * - Construct an empty DCEL, or a DCEL of one or multiple line segments
 * - Geometrically test whether a query point is one of the DCEL vertices
 * - Geometrically test whether a query line segment is a DCEL edge-pair
 * - Find the edge, if any, immediately over a vertex (its "roof" edge)
 * - Find the highest vertex of a cycle.
 *
 * Merging two DCELs is a nontrivial operation that creates any necessary new
 * vertices, edges, and faces based on the overlay of the input DCELs.
 * The current merge implementation resorts to brute force for some of its
 * steps, and thus this code is not as fast as could be.
 *
 *
 * @section dcel_details Detailed explanation
 *
 * Each vertex, half-edge and face gets a record in the respective table below.
 * Faces are maximal connected sets of points mutually reachable without
 * crossing any segments, and including points not in any segment.
 * For brevity I will refer to half-edges simply as edges.
 * Vertex table records are pairwise-distinct, as are edge and face records.
 * Obviously, the tables have finite length.  One consequence is that the
 * DCEL cannot represent infinite collections of vertices, edges, or faces.
 * Edges have finite length, and all faces have finite area except for the
 * ever-present unbounded outer face, which has face index zero.
 * An empty DCEL has no vertices or edges, just an unbounded outer face.
 *
 * Edges are directed.  Each edge e has an origin vertex, origin(e), and a
 * distinct destination vertex, destination(e), which are the endpoints of the
 * corresponding line segment.  Each edge e also has a distinct twin edge,
 * twin(e), such that origin(twin(e)) = destination(e) and
 * destination(twin(e)) = origin(e).  Each edge has a valid, distinct previous
 * edge and next edge.  Therefore, using a pigeonhole-principle argument, each
 * edge must be a member of some edge cycle.  For example, a DCEL made of
 * a single line segment consists of two twinned edges, forming a two-cycle.
 * We write cycle(e) to denote the set of edges in the cycle including e.
 * Naturally, e is a member of cycle(e).  The word "distinct" here means we
 * exclude the possibility that e=twin(e), or e=next(e), or e=previous(e),
 * or that origin(e)=destination(e):  none of those conditions may occur.
 *
 * Edges are closed:  their endpoints are considered part of the edge.
 * Because of twinning, every vertex is an endpoint of at least two edges.
 * Edges do not cross:  no point internal to e is in any other edge.
 * The endpoints of an edge are said to be "incident vertices,"
 * and the edges touching a vertex are said to be "incident edges."
 *
 * Since edges are directed, we can identify left and right sides of the edge
 * (when oriented forwards from origin to destination).  Because of twinning,
 * we adopt the convention that, for edge e, only the face to the left of e
 * is incident to it (i.e., touches).  Whereas the face on the right side of e
 * is incident to twin(e).  (I like to say that the right-side face is
 * "twincident" to e.)  We denote the face on the left side of e by face(e).
 * In general, face(e) and face(twin(e)) may differ.
 * However, we maintain the invariant that face(e)=face(next(e)) everywhere.
 * By induction, it holds for the whole cycle, and thus we may speak of _the_
 * face incident to a cycle of edges, since the same face lies to the left of
 * each edge.  We denote it face(cycle(e)).
 * However, a cycle does not have a single well-defined twincident face,
 * since possibly every edge may have a different twincident face.
 *
 * Note well!  "Left" and "right" sides of an edge are intuitive labels only
 * when the edges are visualized in a Cartesian layout, where the left side of
 * the x axis touches positive y values.  Whereas if you visualize your
 * coordinate system using a matrix-style or conventional graphics layout,
 * where y increases downwards, then you should mentally transpose "left" and
 * "right" as you read this documentation, as well as the words "clockwise"
 * and "counter-clockwise" (used below in the discussion of faces).
 *
 * Edge cycles do not cross.
 * As a simple consequence of the property face(e)=face(next(e)), edge cycles
 * of different faces cannot "cross" each other:  consider edges e and next(e).
 * There can be no other in-edge or out-edge incident to v = destination(e)
 * from a different cycle lying in the sector centered at v and bounded on the
 * left side of e and next(e).  (For the formal sticklers: the sector is really
 * bounded by rays from v coincident with e and next(e).)  The reason is simply
 * that if one or more such interlopers did exist, one of them would have
 * face(e) on its left side, contradicting the assumption that it is the of a
 * cycle of a different face.  However, we push this idea a step further:  we
 * allow no cycles to cross; nor may a cycle cross itself.  Thus in the above
 * scenario for e, there are _no_ edges incident to v in that sector.
 * We claim this is not a handicap:  when dealing with edges incident to v
 * and the same face, their previous and next fields always may be permuted to
 * preserve the same vertices, edges and faces and all the other invariants,
 * and also not cross previous-next links.
 *
 * When a set of line segments encloses a face f, there will be an entry for it
 * in the face table, with index of 1 or higher.  The index-0 face always
 * denotes the unbounded outer face that surrounds all vertices, edges, and
 * any other faces.
 * The line segments around f will induce at least two edge cycles,
 * one of which, k, will be incident to f and trace around the "outer" border
 * of f in a counter-clockwise direction.  (The meaning of "outer" here
 * probably is intuitive, but we define it later just in case.)  As argued
 * above, each edge in k is incident to f, thus has points of f to its left.
 * One representative edge of k will have its index recorded in the
 * outer_component field of f's face record.  Any edge in k will do.
 * The index-0 face, naturally, does not have a meaningful index in that field
 * since it has no outer component.
 *
 * Example:  imagine a DCEL representing a crude baseball field, with face 1
 * representing the baseball diamond with a vertex for each base, and covering
 * most of the infield, but excluding the pitcher's mound.
 * Face 2 covers the pitcher's mound -- imagine it represented by a many-sided
 * regular polygon.  Face 0 contains all points outside the diamond.
 * The outer border cycle of face 1 goes counter-clockwise from first base,
 * next to second base, next to third base, to home, in that order.  One edge
 * of this cycle has its index recorded in the face 1 outer_component field.
 * The inner_component vector is length 1 and contains an edge index of one
 * of the edges in the cycle incident to face 1 and tracing around the
 * pitcher's mound.  This cycle goes clockwise; if you were to walk around it,
 * following the cycle in order, your gaze would light on home plate, then
 * third base, second base, and first base, in that order.  Both these cycles
 * are incident to face 1.  Their twincident edges touch face 0 and face 2.
 *
 * If you need it, here is a slightly more formal definition of what makes
 * cycle k an outer border.
 * A cycle k is defined to be a border if there exists an edge e in k such that
 * face(e) and face(twin(e)) differ.  (The property needn't hold for all of k's
 * edges.  Border cycle k is defined as an outer border of its face when
 * there exists a point p on an edge of k such that there exists a planar curve
 * c connecting p to "infinity" (say, any point on the circumference of any
 * circle enclosing all vertices), such that the only intersection of c and f
 * is point p.  Informally, c does not have to cross any points of f to get
 * away.  A finite face f has exactly one outer border, since f must be
 * connected, cycles are disjoint, and edge records are distinct.  The index-0
 * face (the only face that is not finite) is unbounded and lacks an outer
 * border.
 *
 * If k is border of f but not an outer border of f, we may say it is an inner
 * border of f, but this is not a very useful concept since cycles need not be
 * borders at all.  Any cycle that is not an outer border is referred to as an
 * "inner component":  perhaps an inner border, or perhaps a cycle tracing the
 * perimeter of a shape without area (like a zig-zag or asterisk).  Observe
 * that for every edge on the latter kind of inner component, its incident and
 * "twincident" face are the same face.  Even on a border cycle, not every
 * edge in the cycle need be the boundary between two faces (e.g., a "lollipop"
 * shape with a stem segment sticking out or a face with an "ingrown hair").
 * Each inner component in face f is represented in f's "inner_components"
 * field by the index of one of its edges -- any edge in the cycle will do.
 * To be a little more formal, for each face f, its inner_components field is a
 * list, in arbitrary order, of edge indices e1, e2, . . ., en, such that
 * f = face(e1) = face(e2) = . . . = face(en).  When regarded as a set of sets
 * of edges, {cycle(e1), cycle(e2), . . . , cycle(en)} is a partition of all
 * edges incident to f that are not members of f's outer border cycle (if any).
 *
 * For example, if by chance f were the only bounded face, there would also
 * be an edge cycle tracing clockwise, incident to the index-0 face and
 * "twincident" to f on at least a few of its edges.  Face f would be a "hole"
 * in the index-0 face.  The index-0 face record would store the index of one
 * edge of that index in its inner_components field.  Any face can
 * have holes or other inner components.  A hole in g is a face h distinct from
 * g, such that a closed curve could be drawn around h consisting only of
 * interior points of g.  For each hole h within g, there is a clockwise cycle
 * incident to g that is an inner border of g, and a counter-clockwise cycle
 * incident to h that is an outer border of h.
 * One edge index of the clockwise cycle is listed in the "inner_components"
 * list of the face-record for g.  Also listed in the "inner_components" list
 * are non-border cycles that lie within g.
 * And, of course, one edge index of the outer border of h is listed in the
 * outer_components field of the face-record of h.
 *
 * This implementation of the DCEL is based on the exposition by de Berg et
 * al. (_Computational Geometry_, 2nd ed., Springer: 2000), and they refer
 * primarily to the work of Muller and Preparata (1978).
 *
 * @section dcel_invariant Summary of invariants
 *
 * To summarize the above invariants:
 * - For each vertex v, v = origin(outedge(v)).
 * - For each pair of distinct vtxs u and v, location(u) and location(v) differ
 * - For each face f with outer border, f = face(outer_component(f))
 * - For each edge e, e = twin(twin(e))
 * - For each edge e, e = next(prev(e)) = prev(next(e))
 * - For each pair of edges e, f, either e and f are disjoint or they share one
 *   point, a vertex v that is an endpoint of e and an endpoint of f.
 *
 * Let next^k denote k applications of next, e.g., next^2(e) = next(next(e)).
 * We similarly define prev^k.
 * - For each edge e, face(e) = face(next(e)) = face(next^k(e)) for all k > 0.
 * - For each edge e, face(e) = face(prev(e)) = face(prev^k(e)) for all k > 0.
 * - For each edge e, there exists a k > 1 such that e = next^k(e).
 *   The smallest such number is said to be the cycle length L for e's edge
 *   cycle, and e = prev^L(e), and for all 1 < k < L, e and prev^k(e) differ.
 * - Edge cycles do not cross:  to the left of e and next(e) there is no edge
 *   incident to destination(e).
 *
 * @sec dcel_lim Implementation limits
 *
 * The current implementation uses the C macro INT_MAX as a sentinel
 * to indicate a bad index, in a few places.
 * The symbol is abstracted as "BLANK."
 * Thus the class cannot represent a DCEL with INT_MAX or more edges.
 * Chances are this is not a problem for you.
 * It would not be very hard for a programmer to loosen this limitation.
 */
class Doubly_connected_edge_list
{
public:
    enum { BLANK = INT_MAX };

    struct Vertex_record
    {
        RatPoint location;
        size_t outedge;
        Vertex_record(const RatPoint& l,size_t oe): location(l), outedge(oe) {}
    };

    struct Edge_record // commonly known as a "half-edge" since it is directed.
    {
        size_t origin, twin; // these must always be correct
        size_t face, next, prev; // sometimes these are temporarily incorrect
        Edge_record(size_t o, size_t t, size_t f, size_t n, size_t p)
        :   origin(o), twin(t), face(f), next(n), prev(p)
        {}
        Edge_record(size_t o, size_t t)
        :   origin(o), twin(t), face(BLANK), next(BLANK), prev(BLANK)
        {}
    };

    struct Face_record
    {
        /**
         * The following field is not used in the index-0 face table entry.
         * In all other records, it contains the index of a representative edge
         * of the CCW edge cycle that forms the outer fence around the face.
         */
        size_t outer_component;

        /**
         * Each entry (if any) points to a representative edge of a CW edge
         * cycle of the outer boundary of a hole in the face.
         * Distinct entries point to distinct cycles.
         */
        std::vector< size_t > inner_components;

        Face_record(size_t o,const std::vector<size_t>&i=std::vector<size_t>())
        :   outer_component(o), inner_components(i)
        {}
    };

    const std::vector< Vertex_record >& get_vertex_table()const{return m_vtab;}

    const std::vector< Edge_record >& get_edge_table() const { return m_etab; }

    const std::vector< Face_record >& get_face_table() const { return m_ftab; }

    /// Default ctor creates a blank DCEL.
    Doubly_connected_edge_list()
    :   m_ftab(1, Face_record(0)),
        m_lookup_valid(1),      ///< actually it is valid, in a vacuous way
        m_cycle_table_valid(1), ///< ditto
        m_roof_table_valid(1)
    {}

    /// ctor builds a single-segment DCEL
    Doubly_connected_edge_list(const RatPoint_line_segment&);

    /// swap the state of two DCELs
    void swap(Doubly_connected_edge_list& dcel)
    {
        m_vtab.swap(dcel.m_vtab);
        m_etab.swap(dcel.m_etab);
        m_ftab.swap(dcel.m_ftab);

        // swap caches
        m_vertex_lookup.swap(dcel.m_vertex_lookup);
        std::swap(m_lookup_valid, dcel.m_lookup_valid);
        m_cycles.swap(dcel.m_cycles);
        m_edge_of_cyc.swap(dcel.m_edge_of_cyc);
        std::swap(m_cycle_table_valid, dcel.m_cycle_table_valid);

        // swap face-helper caches
        std::swap(m_cycletop, dcel.m_cycletop);
        std::swap(m_vertex_roof, dcel.m_vertex_roof);
        std::swap(m_roof_table_valid, dcel.m_roof_table_valid);
    }

    /**
     * @brief "named ctor" to initialize from an open pixpath
     *
     * This assumes the input path defines a list of vertices of a polygonal
     * path connected by line segments.  The first vertex and the last
     * vertex are not connected by a segment unless the input size is two.
     * If the input size is zero or one, the output is empty.
     */
    static Doubly_connected_edge_list ctor_open_path(const PixPath&);

    /// @brief "named ctor" to initialize from an open polygonal path
    static Doubly_connected_edge_list ctor_open_path(
        const std::vector<RatPoint>&
    );

    /// @brief "named ctor" to initialize a polygon
    template <typename PATH>
    static Doubly_connected_edge_list ctor_closed_path(const PATH& path)
    {
        Doubly_connected_edge_list d(ctor_open_path(path));
        if (2 < path.size())
        {
            Doubly_connected_edge_list
                s(RatPoint_line_segment(path.front(), path.back())),
                e(s.merge(d));
            d.swap(e);
        }
        return d;
    }


    /**
     * @brief "named ctor" to initialize from an XML formatted stream.
     * @see xml_output
     */
    static Doubly_connected_edge_list ctor_xml_stream(std::istream&);


    size_t lookup_vertex(const RatPoint&, bool* = 00) const;


    /// Create a new DCEL by merging this one with another (a big deal).
    Doubly_connected_edge_list merge(const Doubly_connected_edge_list& d) const
    {
        try
        {
            return brute_force_merge(d);
        }
        catch (const KJB_error& e)
        {
            e.print_details();
            throw;
        }
    }

    /// Transform by translation (rigid motion by an x and y offset)
    Doubly_connected_edge_list& translate(const RatPoint&);

    /**
     * @brief Transform using a row-major 3x3 homogeneous matrix on each vertex
     *
     * In this way we can perform affine transforms on the vertices.
     * That means we can translate, rotate, reflect, scale, and skew it.
     * Examples:
     * 1. Translate all points by x += 3, y -= 17: {1,0,3,  0,1,-17, 0,0,1}.
     * 2. Rotate points by 90 deg. around origin:  {0,-1,0, 1,0,0,   0,0,1}.
     * 3. Reflect points around line y=x:          {0,1,0,  1,0,0,   0,0,1}.
     * 4. Double size, fixed pt. at origin:        {2,0,0,  0,2,0,   0,0,1}.
     * 5. Skew to the right:                       {4,1,0,  0,4,0,   0,0,4}.
     *
     * @throws Illegal_argument if the transform matrix is not full rank.
     */
    Doubly_connected_edge_list& transform(const RatPoint::Rat[]);


    /// transform using a row-major 3x3 homogeneous matrix on each vertex
    Doubly_connected_edge_list& transform(const std::vector<RatPoint::Rat>& x)
    {
        if (x.size() != 9) KJB_THROW_2(Illegal_argument, "Need 3x3 RM matrix");
        return transform(& x.front());
    }


    /// Query with an edge index, return value is a cycle number
    int get_cycle(size_t ei) const
    {
        if (ei >= get_edge_table().size()) KJB_THROW(Index_out_of_bounds);
        if (!m_cycle_table_valid) build_cycle_table();
        return m_cycles.at(ei);
    }


    /// Query with a cycle number, return the index of some edge on it.
    size_t get_edge_of_cycle(int cn) const
    {
        if (cn < 0) KJB_THROW(Index_out_of_bounds);
        if (!m_cycle_table_valid) build_cycle_table();
        if (int(m_edge_of_cyc.size()) <= cn) KJB_THROW(Index_out_of_bounds);
        return m_edge_of_cyc.at(cn);
    }


    /// Return the number of (real) edge cycles in the DCEL
    size_t get_number_of_cycles() const
    {
        if (!m_cycle_table_valid) build_cycle_table();
        return m_edge_of_cyc.size();
    }

    /**
     * @return true iff vertex vi has an edge "over" it like a roof.
     *
     * Edge is a roof if it has interior points directly above vertex vi or
     * immediately to its right (i.e., in quadrant I if vi is origin).
     * "Over" and "above" mean "at a larger y coordinate and the same
     * x coordinate" -- a Cartesian-layout friendly definition, as is usual
     * in this header.  Imagine a ray of laser light shooting upwards from
     * the vertex, with a huge positive slope -- nearly vertically.  The roof
     * is the first segment the ray would hit.  If there is a junction of
     * segments directly over vi, the segment that would be hit would be
     * the one with the most negative slope (obvious if you draw a picture).
     *
     * Also if the vertex is the lower terminus of a vertical segment, that
     * segment is its roof, a "flagstaff" roof (because the segment is like a
     * flag staff and I'm using that term, not flagpole, due to Arizona pride).
     * This is possibly an ugly complication to the definition, but it does not
     * seem to matter very much -- we use the roof concept for computing the
     * DCEL faces and a flagstaff roof does not play a role there.
     * Note that no other segment incident to the vertex can be its roof.
     * If future work reveals that flagstaff roofs are unhelpful, the
     * definition could be tweaked to get rid of them.  If future work reveals
     * that they are helpful, this comment should be revised to explain why.
     *
     * The implementation of the above gets somewhat ugly.  This results are
     * computed lazily:  only when needed.  If not cached, the results take
     * time O(m log m + n), where m is the number of vertices and n is the
     * number of segments.  We use a plane-sweep implementation by recycling
     * the code used for the Bentley-Ottman algorithm.  With a good cache,
     * lookup is constant time of course.  The results usually *are* cached
     * since we need this information to compute faces.  The only time the
     * cache is bad, I think, is after you do an affine transformation.
     *
     * Example:  the line segment (0,1)-to-(2,1) is a roof to points (0,0)
     * and (1,0) but not (2,0) or (3,0).  The latter two have laser rays that
     * miss the segment, though the ray from point (2,0) barely misses it.
     */
    bool vertex_has_roof(size_t vi) const
    {
        if (!m_roof_table_valid) scan_for_roof();
        return m_vertex_roof.at(vi) & 1;
    }

    /// @brief get edge index of roof, if any (results undefined if no roof).
    /// @see vertex_has_roof
    size_t get_vertex_roof(size_t vi) const
    {
        if (!m_roof_table_valid) scan_for_roof();
        return m_vertex_roof.at(vi) >> 1;
    }

    /// @return vertex index of vertex at top of cycle, or top right if a tie
    size_t get_top_vertex_of_cycle(int cn) const
    {
        if (!m_roof_table_valid) scan_for_roof();
        return m_cycletop.at(cn);
    }

    /**
     * @brief fake "container" type to fool std::back_inserter().
     * @see push_back
     *
     * DCEL is not really a container, but we act a container of line
     *     segments so that we can have a working std::back_inserter().
     */
    typedef RatPoint_line_segment value_type;

    /**
     * @brief merge a line segment (works with back_inserter).
     * @see value_type
     *
     * Segment insertion has superlinear time complexity, so if you have a lot
     * of segments to insert, and you don't need to use back_inserter(),
     * it will be better to use ctor_from_edge_list().
     */
    void push_back(const RatPoint_line_segment& s)
    {
        Doubly_connected_edge_list ds(s), plus(ds.merge(*this));
        swap(plus);
    }

    /// @brief return a DCEL with face 1 isomorphic to given positive face.
    Doubly_connected_edge_list face_export(size_t) const;

#if 0
    class Edge_const_iterator;

    Edge_const_iterator e_begin() const;

    Edge_const_iterator e_end() const;
#endif

    std::string get_mutable_status() const;

private:
    std::vector< Vertex_record > m_vtab;

    std::vector< Edge_record > m_etab;

    // face zero is always the unbounded outermost face, outer_component is 0.
    std::vector< Face_record > m_ftab;

    // cached lookup table
    typedef std::map< RatPoint, size_t > VtxLookup;
    mutable VtxLookup m_vertex_lookup;
    mutable bool m_lookup_valid;

    // cached cycle table
    mutable bool m_cycle_table_valid;
    mutable std::vector< int > m_cycles, m_edge_of_cyc;

    // cached vertex-roof and cycle-top tables
    mutable bool m_roof_table_valid; // applies to m_cycletop and m_vertex_roof
    mutable std::vector< size_t > m_cycletop;
    // LSB of m_vertex_roof[i] is 0 if vertex i has no roof, else 1.
    // m_vertex_roof[i]>>1 is the edge index of the vertex roof, if any.
    mutable std::vector< size_t > m_vertex_roof;

    void build_cycle_table() const; // "const" since it just touches mutables.

    Doubly_connected_edge_list brute_force_merge(
        Doubly_connected_edge_list // copy
    )   const;


    void previous_next_link(size_t, size_t);

    void walk_cycle_and_set_its_face(size_t, size_t);

#if 1 /* this stuff is being retired soon */
    // utility functions used by merge
    bool consider_newbie_vtx(const RatPoint&, VtxLookup*, std::set<size_t>*);

    void intersect_red_blue_with_bentley_ottman(
        const Doubly_connected_edge_list& red,
        VtxLookup* pnewbies,
        std::set<size_t>* pbogus_voe
    );

    void intersect_red_blue_with_brute_force(
        const Doubly_connected_edge_list& red,
        VtxLookup* pnewbies,
        std::set<size_t>* pbogus_voe
    );
#endif

    void scan_for_roof() const; // "const" since it just touches mutables

    void sweep_edge_merge(const Doubly_connected_edge_list&);

    void rebuild_face_table();
};


/// @brief slowly test invariants on the structure, return ERROR or NO_ERROR
int is_valid(const Doubly_connected_edge_list&);


/// @brief outbound half-edges at a particular vertex (specified by number)
std::vector< size_t > out_edges(const Doubly_connected_edge_list&, size_t);


/// @brief Get the indices of twin edges of a given list of edges.
std::vector< size_t > twin_edges(
    const Doubly_connected_edge_list&,
    const std::vector< size_t >&
);


/// @brief inbound half-edges at a particular vertex (specified by number)
inline std::vector< size_t > in_edges(
    const Doubly_connected_edge_list& dcel,
    size_t vertex_index
)
{
    return twin_edges(dcel, out_edges(dcel, vertex_index));
}


/// @brief return number of segments (not edges) incident to this vertex
inline size_t vertex_degree(const Doubly_connected_edge_list& d, size_t v)
{
    return out_edges(d, v).size();
}


/**
 * @brief get line segment represented by a DCEL edge (specified by index)
 * @param d DCEL to be consulted.  The vertex table must be correct and the
 *          edge table must be partially correct: valid twin and origin fields.
 * @param ei Index of edge to be retrieved.
 * @return line segment with terminus a at the origin of edge number ei
 * @throws std::out_of_range if ei is not a valid edge index for d.
 */
inline
RatPoint_line_segment get_edge(const Doubly_connected_edge_list& d, size_t ei)
{
    const Doubly_connected_edge_list::Edge_record
        &e = d.get_edge_table().at(ei), &f = d.get_edge_table().at(e.twin);
    return RatPoint_line_segment( d.get_vertex_table().at(e.origin).location,
                                  d.get_vertex_table().at(f.origin).location );
}


/// @brief Print a text representation of the DCEL tables (but not the cache)
std::ostream& operator<<(
    std::ostream&,
    const Doubly_connected_edge_list&
);


/// @brief render DCEL as an HTML table element
std::string html_table(const Doubly_connected_edge_list&);


inline
bool is_empty(const Doubly_connected_edge_list& d)
{
    return 0 == d.get_edge_table().size();
}


/// @brief provide opposite corners of a nontilted rectangle, get a box.
inline
Doubly_connected_edge_list get_axis_aligned_rectangle(
    const RatPoint& corner11,
    const RatPoint& corner22
)
{
    std::vector< RatPoint > b(4, corner11);
    b[1].x = corner22.x;
    b[2] = corner22;
    b[3].y = corner22.y;
    return Doubly_connected_edge_list::ctor_closed_path(b);
}


/// compute diagonal of tight axis-aligned bounding box around DCEL
RatPoint_line_segment get_aa_bounding_box_diagonal(
    const Doubly_connected_edge_list&
);


/**
 * @brief predicate tests whether ei is part of a cycle delineating zero area.
 *
 * Possibly I am abusing the word "stick figure" -- a common XKCD stick figure
 * has a head that covers some area, and I do NOT mean such things.  I mean
 * asterisks, zig-zags, scrawny trees:  all perimeter but NO area.
 *
 * This is important in visualizations because it characterizes a cycle that
 * does not need to be drawn as a filled polygon.
 */
bool is_edge_of_stick_figure(const Doubly_connected_edge_list&, size_t);


/**
 * @brief look up an edge number by its location
 * @return edge index if found, else edge table size.
 *
 * Time complexity.  Let m be the number of vertices in the dcel, and d be the
 * edge degrees of both vertices of the edge, added.
 * Like lookup_vertex, if the vertex table is not yet built, time complexity
 * is O(m + d log d).  After the table is built, it is O(log m + d log d).
 */
size_t lookup_edge(
    const Doubly_connected_edge_list&,
    const RatPoint_line_segment&
);


/// Construct a DCEL from a list of segments (degenerates ignored).
Doubly_connected_edge_list ctor_from_edge_list(
    const std::vector<RatPoint_line_segment>&
);


/**
 * @brief Serialize DCEL as XML (returns complete file contents).
 * @see Doubly_connected_edge_list ctor_xml_stream(std::istream&);
 *
 * If you do not want the complete file contents, remove the first line,
 * which looks like this: <?xml version="1.0" ?> plus a newline.
 */
std::string xml_output(const Doubly_connected_edge_list&);


/// @brief test for identical DCEL layout, topology, and indexing
bool operator==(
    const Doubly_connected_edge_list&,
    const Doubly_connected_edge_list&
);


/**
 * @brief test if two DCELS have same edge position and topology.
 *
 * Basically would they plot the same way?  Are they geometrically identical,
 * disregarding the numbering schemes on the vertices, edges and faces?
 *
 * The pointer argument is for an optional output certificate which gives the
 * mapping from an edge index in the first DCEL to the isomorphic edge index
 * in the second DCEL.
 *
 * Time complexity is O(n log n) where n is the number of edges.
 */
bool is_isomorphic(
    const Doubly_connected_edge_list&,
    const Doubly_connected_edge_list&,
    std::vector<size_t>*
);


RatPoint::Rat common_denominator(const Doubly_connected_edge_list&);


#if 0
class Doubly_connected_edge_list::Edge_const_iterator
:   public std::iterator<std::bidirectional_iterator_tag,RatPoint_line_segment>
{
    const Doubly_connected_edge_list& dcel_;
    size_t edge_index_;
    Edge_const_iterator(const Doubly_connected_edge_list& d, size_t i)
    :   dcel_(d),
        edge_index_(i)
    {}
public:
    value_type operator*() const { return get_edge(dcel_, edge_index_); }
    bool operator==(Edge_const_iterator j) const
    {
        return edge_index_ == j.edge_index_;
    }
    bool operator!=(Edge_const_iterator j) const { return ! operator==(j); }
    // preincrement
    Edge_const_iterator& operator++() { ++edge_index_; return *this; }
    // postincrement
    Edge_const_iterator operator++(int)
    {
        return Edge_const_iterator(dcel_, edge_index_++);
    }
    Edge_const_iterator twin() const
    {
        return Edge_const_iterator(dcel_,
                                dcel_.get_edge_table()[edge_index_].twin);
    }

    friend Edge_const_iterator Doubly_connected_edge_list::e_begin() const;
    friend Edge_const_iterator Doubly_connected_edge_list::e_end() const;
};


inline Doubly_connected_edge_list::Edge_const_iterator
Doubly_connected_edge_list::e_begin() const
{
    return Edge_const_iterator(*this, 0);
}


inline Doubly_connected_edge_list::Edge_const_iterator
Doubly_connected_edge_list::e_end() const
{
    return Edge_const_iterator(*this, get_edge_table().size());
}
#endif


}
}

#endif /* QD_CPP_DCEL_H_INCLUDED_IVILAB */
