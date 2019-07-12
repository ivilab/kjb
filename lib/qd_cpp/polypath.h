/**
 * @file
 * @brief Contains definition for class PolyPath
 * @author Andrew Predoehl
 */
/*
 * $Id: polypath.h 17555 2014-09-18 07:36:52Z predoehl $
 */

#ifndef POLYPATH_H_UOFARIZONAVISION
#define POLYPATH_H_UOFARIZONAVISION 1

#include <l_cpp/l_exception.h>
#include <m_cpp/m_vector_d.h>
#include <qd_cpp/pixpath.h>

#include <iosfwd>

namespace kjb
{
namespace qd
{

/**
 * @brief represents an open polygonal path with a tangent at each point
 *
 * The PixPath interface to this object represents the 8-connected "view" of
 * the path.  If you want to access the vertices, use the get_vertices()
 * method.  If you want to test whether a certain location along the path
 * is a vertex, use the is_a_vertex() predicate.
 *
 * This class defines a tangent at each point along the connected path.
 * Each tangent is represented as a radial unit vector, i.e., it could lie
 * anywhere on the unit circle.  The direction points generally from the
 * lower-numbered vertices to the higher-numbered vertices.  Thus the tangent
 * at location 0 is a unit vector pointing from vertex 0 to vertex 1.
 *
 * Tangents at non-vertex locations are defined similarly:  each non-vertex
 * location is bookended by vertices v, w, and the tangent goes in the
 * direction from v to w.  This is one reason why the input vertices must be
 * distinct.
 *
 * We also define a value that will serve as "tangent" at the vertices
 * themselves.  Let v be an interior vertex and let u, w be its predecessor
 * and successor vertices.  Let r be a unit vector in the (u,v) direction
 * and let s be a unit vector in the (v, w) direction.  We define the "tangent"
 * at v therefore to be a unit vector in the (r+s) direction.  We defined the
 * "tangent" at the first vertex above, and for the last vertex the definition
 * is similar.
 */
class PolyPath : public PixPath
{
    PixPath m_sparsepath;               ///< polygon vertices

    std::vector<size_t> sdmap;          ///< sparse-to-dense map
    std::vector<Vector2> m_tangent;     ///< radial tangent unit vector

    PolyPath(const PixPath&);

public:
    /// @brief static ctor:  build polygonal path from a sequence of vertices
    /// @throws Illegal_argument if input has a repeated pt. or a 180-deg bend.
    static PolyPath construct_from_vertices(const PixPath& vertices)
    {
        if (vertices.self_intersect())
        {
            KJB_THROW_2(Illegal_argument, "input must be distinct vertices");
        }
        return PolyPath(vertices);
    }


    /// @brief static ctor:  reconstruct a path with extra information
    static PolyPath construct_from_path(const PixPath& path)
    {
        return construct_from_vertices(path.cull_redundant_points());
    }


    /// @brief provide read-only access to the sequence of polygon vertices
    const PixPath& get_vertices() const
    {
        return m_sparsepath;
    }


    /**
     * @brief access unit vector at a point along path, spec by const_iterator
     * @param i const_iterator to a point in the path
     * @warning behavior is undefined if i does not point into this PolyPath.
     *
     * On points that are NOT vertices, the output represents a ray passing
     * from the nearest vertex prior to this path location, through the
     * nearest vertex subsequent to this path location.  Thus, it is directed
     * and the output (as a unit vector) can potentially lie in any of the
     * four quadrants.  If you prefer an axial direction rather than a radial
     * direction, you might find these other functions useful:
     *
     * @see get_unit_vector_2x_angle();
     * @see get_unit_vector_2x_angle_of_unit_vector()
     */
    Vector2 tangent_at(const_iterator i) const
    {
        return m_tangent.at(i - begin());
    }


    /// @brief access unit vector at a point along path, specified by index
    Vector2 tangent_at(size_t ix) const
    {
        return m_tangent.at(ix);
    }


    /// @brief test whether the point at a given index is a polygonal vertex
    bool is_a_vertex(size_t ix) const
    {
        return std::find(sdmap.begin(), sdmap.end(), ix) != sdmap.end();
    }


    int debug_print(std::ostream&) const;
};


/**
 * @brief double the angle relative to (1,0) and return a unit vector that way.
 * @param v nonzero vector input indicating a direction from the origin
 * @return unit vector repr direction from origin of twice the angle of input
 * @throws Illegal_argument if input has zero magnitude.
 *
 * Angles, as you would probably expect, measure counterclockwise rotation of
 * a ray relative to a ray from the origin, (0, 0), passing through (1, 0).
 *
 * The input must be a vector with nonzero magnitude.  Picture it with its
 * tail at (0, 0).  The output is a unit vector such that the angle of the
 * output vector is twice that of the angle of the input vector.
 * Examples:
 * Input = (1, 1), a 45 degree angle.  Therefore output = (0, 1), 90 degrees.
 * Input = (-1, 1), a 135 degree angle.  Therefore output = (0, -1), 270 deg.
 * Input = (1, epsilon), for very small epsilon.  Output near (1, 2*epsilon).
 * Input = (1, 1), a -315 degree angle.  Therefore output = (0, 1), -630 deg.
 *
 * The nice thing about this function is that it avoids all trig calls,
 * because it uses two trig identities, namely
 * cos 2A = 2 cos A cos A - 1
 * sin 2A = 2 sin A cos A
 * . . . and we can find cos A and sin A just by normalizing the input.
 *
 * If you can guarantee the input is a unit vector, you can use the
 * even faster version called get_unit_vector_2x_angle_of_unit_vector().
 *
 * The main application of this function is to convert a radial angle into a
 * representation of an axial angle.  The doubled angle behaves as if it is
 * cyclic with period 180 degrees.
 */
Vector2 get_unit_vector_2x_angle(const Vector2& v);


/// @brief like get_unit_vector_2x_angle() but return zero vector on zero input
inline Vector2 get_unit_vector_2x_angle_nothrow(const Vector2& v)
{
    return 0 == v.x() && 0 == v.y() ? v : get_unit_vector_2x_angle(v);
}


/// @brief like compute_unit_vector_2x_angle but for unit-magnitude input.
Vector2 get_unit_vector_2x_angle_of_unit_vector(const Vector2&);


/// @brief test whether a pixel path is convertible to PolyPath (maybe say why)
bool is_valid_as_polypath(const PixPath&, bool throw_failure = false);

}
}

#endif /* POLYPATH_H_UOFARIZONAVISION */

