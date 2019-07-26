/**
 * @file
 * @author Andrew Predoehl
 * @brief Declaration for Bentley-Ottmann line intersection algorithm
 */
/*
 * $Id: intersection.h 20084 2015-11-16 04:21:13Z predoehl $
 */

#ifndef QD_CPP_INTERSECTION_INCLUDED_IVILAB_UARIZONAVISION
#define QD_CPP_INTERSECTION_INCLUDED_IVILAB_UARIZONAVISION 1

#include <qd_cpp/ratpoint.h>
#include <qd_cpp/pixpath.h>

#include <vector>
#include <utility>


namespace kjb
{
namespace qd
{



/**
 * @brief return all intersections of polygonal path segments
 *
 * @param p interpreted as a polygonal path of vertices (entries)
 *          connected by closed line segments.
 * @param filter_out_trivial_intersections an optional flag that, if set,
 *          removes from the output all the intersections between nondegenerate
 *          segments that touch only at their common endpoint.
 *
 * @return vector of pairs of indices of intersecting segments, where segment
 *         k is the closed line segment between points p[k] and p[k+1].
 *
 * In other words, we interpret the PixPath as a polygonal path (the entries
 * are the vertices), and the path edges are interpreted as closed line
 * segments between consecutive vertices.
 *
 * Since they are closed, each line
 * segment except the first and last shares its endpoints (vertices) with the
 * preceding and succeeding segment.  These trivial overlaps are filtered
 * out by default.  This filtering is somewhat complicated:  adjacent segments
 * always overlap trivially, but sometimes nontrivially.  The filter (when
 * selected) only omits trivial intersections.  A degenerate segment is
 * regarded to intersect nontrivially, and segments that share a continuum
 * are regarded to intersect nontrivially.
 *
 * It uses the Bentley-Ottmann line sweep algorithm.  The time requirements
 * are described in the comments for
 * get_intersections(const std::vector< RatPoint_line_segment >&)
 * which is used to implement this function.
 */
std::vector< std::pair<size_t, size_t> > get_intersections(
    const PixPath& p,
    bool filter_out_trivial_intersections = true
);



/**
 * @brief compute indices of segments that intersect
 *
 * @param sl list of closed (that is, termini-included) line segments.
 *
 * @return vector of pairs of indices of intersecting segments.  Thus each
 *         entry has a first and second field equal to different indices of
 *         intersecting segments in the input.
 *
 * It uses the Bentley-Ottmann line sweep algorithm, so it is efficient when
 * the number of intersections k is comparable to the number of vertices n.
 * In that case the complexity is O(n log n + k log n).
 * If you expect a lot of intersections, a brute-force approach will probably
 * be a little faster.
 */
std::vector< std::pair<size_t, size_t> > get_intersections(
    const std::vector< RatPoint_line_segment >& sl
);


/**
 * @brief compute indices of segments that intersect at interior of either
 *
 * @param sl list of closed line segments.
 *
 * @return vector of pairs of indices of segments where the intersection
 *         is, or includes, an interior point of one or both of the segments.
 *         This is a subset of the result of get_intersections(), excluding
 *         the endpoint-to-endpoint results.  What's left are the
 *         interior-interior and interior-endpoint touching segments.
 *
 * This differs from the semantics of param filter_out_trivial_intersections
 * in get_intersections for a PixPath, because of the handling of degenerate
 * segments and nonconsecutive segments.  This function does not care if two
 * nonconsecutive segments in sl intersect at their endpoints, and it does
 * not care if a degenerate segment touches the endpoint of another segment.
 * Neither can be considered an interior intersection.
 * (Both those cases are arguably nontrivial for a polygonal path.)
 */
std::vector< std::pair<size_t, size_t> > get_interior_intersections(
    const std::vector< RatPoint_line_segment >& sl
);


} // end ns qd
} // end ns kjb

#endif /* QD_CPP_INTERSECTION_INCLUDED_IVILAB_UARIZONAVISION */
