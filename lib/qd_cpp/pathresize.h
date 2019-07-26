/**
 * @file
 * @brief Definitions for PixPath specializations to resize paths
 * @author Andrew Predoehl
 *
 * Suppose you have a PixPath (q.v.) and it represents the ordered vertices of
 * a polygonal path.  This header provides functionality to expand or shrink
 * the number of vertices in your path, without "much" affecting the set of
 * points in the corresponding polygonal path.
 *
 * In other words, you can add vertices if you need to, or remove them, and the
 * connect-the-dots result will resemble the connect-the-dots appearance of the
 * original, we hope.
 *
 * This also supports the concept of "too close."  You might know a threshold
 * value distance X, such that any vertices closer than X is unacceptable; one
 * or both must go.  The expander class lets you set a too-close value (so that
 * some vertices are removed) and will add new vertices if necessary, but not
 * too close, so that the final result has your goal size without any vertices
 * as close as or closer than X apart.  If this isn't possible, it throws.
 * The class PixPath_expander supports all these features.
 *
 * If you just need to remove vertices (maybe lots of them) to reach a given
 * goal size, use function polyline_approx().
 */
/*
 * $Id: pathresize.h 17756 2014-10-15 18:58:24Z predoehl $
 */

#ifndef PATHRESIZE_H_INCLUDED_PREDOEHL_UOFARIZONA_VISION
#define PATHRESIZE_H_INCLUDED_PREDOEHL_UOFARIZONA_VISION 1

#include <l/l_debug.h>
#include <m_cpp/m_vector.h>
#include <qd_cpp/pixpath.h>

#include <vector>


namespace kjb
{
namespace qd
{


/**
 * @brief Expand, if possible, a PixPath to fill a specified minimum size.
 *
 * This class will expand a PixPath to a specified size if at all possible.
 * The result will be a PixPath at least as large as the given size, or the
 * class will throw an exception.  The resulting path might be larger than sz.
 * Also, the result will not contain duplicate points.
 */
class PixPath_expander
:   public PixPath
{
    /// @brief if path is too small this will make it larger (excl cozy pts).
    static PixPath expanded_pp( const PixPath&, size_t, float );

    static PixPath all_from_list_a_with_some_from_list_b(
        const PixPath&,
        const PixPath&,
        size_t
    );

    static PixPath choose_random_redundancies(
        const PixPath&,
        const PixPath&,
        std::vector< size_t >&, // this parameter gets clobbered
        size_t
    );

    static PixPath choose_hermitic_redundancies(
        const PixPath&,
        const PixPath&,
        size_t
    );

    static size_t insertion_yield( const std::vector< float >&, float );

    static PixPath cull_squashed_points( const PixPath&, float );

public:
    /**
     * @brief ctor builds a new path with at least as many points as requested.
     * @param p path used as initial estimate of what output should be like
     * @param sz output should have at least this many points, or more
     * @param distance_too_close adjacent points in output must be more
     *                           distant than this (in units of pixels).
     *
     * First, it will eliminate points that are too close together.  Then, if
     * necessary, it will interpolate new points between existing points.
     */
    PixPath_expander( const PixPath& p, size_t sz, float distance_too_close )
    :   PixPath( expanded_pp( p, sz, distance_too_close ) )
    {}

    /// @brief return a new path after eliminating points that are too close
    static PixPath cull_cozy_points( const PixPath&, float );
};



/**
 * @brief compute polyline approximation to base_path with dynamic programming
 * @param[in] base_path sequence of points which we want to approximate by
 *                      a polygonal path
 * @param[in] goal_size desired size of the output subsequence -- if not
 *                      smaller than base_path.size() this "gives up" by
 *                      returning base_path and an error of zero.
 * @param[out] error    Optional output parameter equal to the error metric
 *                      for the approximate path.
 * @return new path with the minimum error according to the above criterion.
 *
 * This uses a dynamic programming solution described by Perez and Vidal (1994)
 * "Optimum polygonal approximation of digitized curves," Pattern Recognition
 * Letters 15, pp. 743-750.
 */
PixPath polyline_approx(
    const PixPath& base_path,
    size_t goal_size,
    float* error = 00
);



/**
 * @brief compute polyline approximation of base_path of the "right size."
 * @param base_path sequence of points defining a polygonal path
 * @param goal_size desired size of the output path
 * @param distance_too_close If any pair of sequential vertices is separated
 *                           by this distance or less, we eliminate one or both
 * @return new path with the assigned size
 * @see polyline_approx
 * @see PixPath_expander
 *
 * If the input path has size exceeding goal_size, this will use
 * polyline_approx to reduce the size.  If the input path size is smaller than
 * goal_size, this uses PixPath_expander to enlarge the path.
 * Also notice that a path might be the right size already but if it contains
 * vertices that are closer than distance_too_close, it will need to be
 * altered.
 */
PixPath rightsize(
    const PixPath& base_path,
    size_t goal_size,
    float distance_too_close
);



float sosq_error( const PixPath&, size_t, size_t );

PixPath reduce_pixels_pv(const PixPath&);

PixPath reduce_pixels_bfs(const PixPath&);

} // ns qd
} // ns kjb

#endif /* PATHRESIZE_H_INCLUDED_PREDOEHL_UOFARIZONA_VISION */
