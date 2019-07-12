/**
 * @file
 * @brief Contains definition for classes PixPath, PixPathAc, DoubleCircle
 * @author Andrew Predoehl
 */
/*
 * $Id: pixpoint.h 21448 2017-06-28 22:00:33Z kobus $
 */

#ifndef PIXPOINT_H_UOFARIZONAVISION
#define PIXPOINT_H_UOFARIZONAVISION 1

#include <l_cpp/l_exception.h>
#include <m_cpp/m_vector_d.h>

#include <limits>
#include <string>
#include <sstream>
#include <cmath>
#include <algorithm>

//define HERE std::cout <<"!!!! Control at "<< __FILE__ <<':'<< __LINE__ <<'\n'

namespace kjb
{

/// @brief support for the path algorithm I call the quasi-Dijkstra method.
namespace qd
{


#ifdef TEST
const bool TEST_PIXPOINT_UNUSED = true;     ///< extra test for unused PixPoint
#else
const bool TEST_PIXPOINT_UNUSED = false;    ///< extra test for unused PixPoint
#endif

/**
 * @brief Representation of an (x,y) pair of pixel coordinates
 *
 * The following structure is used to represent a single pixel coordinate in
 * a trail image.  We use a vector of them (below) to represent a trail or
 * trail proposal within that context (i.e., without the full information
 * provided by a struct seg.
 *
 * Useful pixel methods include adjacency testing, a total order based on row-
 * major ordering, and both L-2 and L-infinity distances between points.
 *
 * This struct has a default ctor that sets the fields to a sentinel value (a
 * large number) which indicates that the x and y coordinates have not been
 * properly assigned valid values.
 *
 * To a limited extent, you can partially use these as 2-vectors, since you can
 * store negative coordinate values, and they have both + and - operators.
 *
 * @see str_to_PixPoint() to construct from a string, or from a stream.
 */
struct PixPoint
{

    typedef int Integer;    ///< any signed integer valued type is fine

    /// @brief exception thrown if the PixPoint is used while uninitialized
    struct Unused : public kjb::Result_error
    {
        Unused( const std::string& m, const char* f, int l )
        :   kjb::Result_error( m, f, l )
        {}
    };

    /// @brief throw exception if we try to operate on an unused point
    static void fault_if( bool b )
    {
        if ( TEST_PIXPOINT_UNUSED && b )
        {
            KJB_THROW_2( Unused, "Unused PixPoint" );
        }
    }

    Integer x,              ///< x coordinate of the pixel
            y;              ///< y coordinate of the pixel

    /// @brief ctor sets fields to sentinel values to indicated "unused."
    PixPoint()
    :   x( std::numeric_limits< Integer >::max() ),
        y( std::numeric_limits< Integer >::max() )
    {}

    /// @brief Basic ctor builds from two coordinates
    PixPoint( Integer xx, Integer yy )
    :   x( xx ),
        y( yy )
    {}

    /// @brief swap state of two pixpoints
    void swap( PixPoint& that )
    {
        std::swap( x, that.x );
        std::swap( y, that.y );
    }

    /// @brief Test whether a PixPoint is unused (untouched since default ctor)
    bool is_unused() const
    {
        return      std::numeric_limits< Integer >::max() == x
                &&  std::numeric_limits< Integer >::max() == y;
    }

    /// @brief convenience inline for better readability
    bool is_not_unused() const
    {
        return ! is_unused();
    }

    /// @brief another convenience inline that is not so unpositive.
    bool is_used() const
    {
        return ! is_unused();
    }

    /**
     * @brief This operator imposes a total order on PixPoints.
     * @param pp    Point to compare to this point
     * @return true iff pp is "larger" in the total order, i.e., its row major
     * index exceeds this point's row major index (whatever width you use).
     *
     * Although it does not really matter, this order reflects row-major
     * positioning of the pixels in an array.
     */
    bool operator<( const PixPoint& pp ) const
    {
        fault_if( is_unused() || pp.is_unused() );
        return      y < pp.y
                ||  ( y == pp.y && x < pp.x );  // unnecessary parentheses grr
    }

    /// @brief test whether two points are equivalent (same integer coords)
    bool operator==( const PixPoint& pp ) const
    {
        fault_if( is_unused() || pp.is_unused() );
        return x == pp.x && y == pp.y;
    }

    /// @brief test whether two points differ (different integer coords)
    bool operator!=( const PixPoint& pp ) const
    {
        return ! operator==( pp );
    }

    /// @brief add a PixPoint to this PixPoint, when interpreted as 2-vectors
    PixPoint& operator+=( const PixPoint& pp )
    {
        fault_if( is_unused() || pp.is_unused() );
        return operator=( operator+( pp ) );
    }

    /// @brief add two PixPoints when they are interpreted as 2-vectors
    PixPoint operator+( const PixPoint& pp ) const
    {
        fault_if( is_unused() || pp.is_unused() );
        return PixPoint( x+pp.x, y+pp.y );
    }

    /// @brief subtract two PixPoints when they are interpreted as 2-vectors
    PixPoint operator-( const PixPoint& pp ) const
    {
        fault_if( is_unused() || pp.is_unused() );
        return PixPoint( x-pp.x, y-pp.y );
    }

    /// @brief multiply components of a PixPoint by a factor (like 2-vector)
    PixPoint operator*( int factor ) const
    {
        fault_if( is_unused() );
        return PixPoint( factor * x, factor * y );
    }

    /// @brief integer-divide components of a PixPoint (like a 2-vector)
    PixPoint operator/( int divisor ) const
    {
        fault_if( is_unused() );
        return PixPoint( x / divisor, y / divisor );
    }

    /// @brief interp as 2-vec, compute dot prod as integer (beware overflow)
    Integer idot( const PixPoint& pp ) const
    {
        fault_if( is_unused() || pp.is_unused() );
        return x * pp.x + y * pp.y;
    }

    /**
     * @brief Distance squared between PixPoint objects, or from the origin
     *
     * @param pp    This method returns the L2-norm distance squared between
     *              this point and pp, if pp is specified.  If left
     *              unspecified, pp defaults to the origin.  L2-norm distance
     *              is Euclidean distance, if you didn't know that already.
     *
     * @return  Squared distance as a float.  It's float since float has a
     *          higher dynamic range than an int.  If this point or pp is
     *          unused, result is negative.
     *
     * Why use this method?  Because it is cheaper than dist() yet produces the
     * same partially ordered set.  Formally, for all PixPoints p, q, r
     * dist(p,r) <= dist(q,r) IFF dist2(p,r) <= dist2(q,r).  Proof is trivial.
     */
    float dist2( const PixPoint& pp = PixPoint( 0, 0 ) ) const
    {
        fault_if( is_unused() || pp.is_unused() );
        float dx = x-pp.x, dy = y-pp.y;
        return dx*dx + dy*dy;
    }

    /**
     * @brief Return distance between points, using the L2 norm.
     * @param pp    This method returns the L2-norm distance between pp and
     *              this point (or the origin, if omitted).
     *
     * @return Euclidean distance, unless this point or pp is unused, in which
     *          case the result is negative.
     *
     * This used to use sqrtf but &lt;cmath> lacks it, despite &lt;math.h>
     * promising it.  However, &lt;cmath> has overloaded versions, including
     * one for float.  In the spirit of separating interface from impl., less
     * is in fact more.
     */
    float dist( const PixPoint& pp = PixPoint( 0, 0 ) ) const
    {
        fault_if( is_unused() || pp.is_unused() );
        return sqrt( dist2(pp) );
    }

    /**
     * This returns between this point and pp, when we use the L-infinity
     * norm to compute distance.  What's that in ordinary terms?  It's the
     * length of the longer edge of the smallest axis-aligned bounding box
     * surrounding both points.  Confusing?  Maybe it's easier just to read the
     * code below.  Purpose?  It's the number of pixels in the Bressenham-alg.
     * drawn line from this point to pp, minus one.
     *
     * If this point or the argument is unused, this method returns a negative
     * value.
     *
     * It would be legitimate to give pp a default value of PixPoint(0,0) just
     * as I did for the dist2() method.  I will wait to do so until it is
     * somehow useful.
     *
     * @param pp    Point to which (from this point) we want to know distance
     * @return L-infinity norm distance from this point to pp.
     */
    Integer L_infinity_distance( const PixPoint& pp ) const
    {
        fault_if( is_unused() || pp.is_unused() );

        Integer dxs = x-pp.x, dx = std::max( dxs, -dxs ),
                dys = y-pp.y, dy = std::max( dys, -dys );
        return std::max( dx, dy );
    }

    /**
     * @brief test adjacency using eight-connectivity (or identity).
     * @param pp point to test whether it is adjacent to this one
     * @return true iff pp is adjacent to this point: equal or 1 step away
     *
     * This tests whether two pixels are adjacent under the usual 8-connected
     * criteria.  Note that a pixel is vacuously adjacent to itself, too.
     */
    bool adjacent8( const PixPoint& pp ) const
    {
        fault_if( is_unused() || pp.is_unused() );
        return L_infinity_distance( pp ) <= 1;
    }

    /// @brief Express a PixPoint as a string (ascii coordinate values)
    std::string str( const std::string& sep = ", " ) const
    {
        if ( is_unused() )
        {
            return std::string( "UNUSED" );
        }

        std::ostringstream ss;
        ss << x << sep << y;
        return ss.str();
    }

    /**
     * @brief Test whether a PixPoint has nonnegative x and y coordinates
     * @return true if this point is not unused and both its coords are nonneg.
     *
     * The lines x = 0 and y = 0 are partially in Quadrant I.
     * Wait, that's not a very precise statement.
     * The rays {(x,y) : x>=0, y=0} and {(x,y) : x=0, y>=0} are in Quadrant I.
     * Is that worse?  That's worse, isn't it?  Ah well, I give up.
     * The code is only 24 characters long; probably best just to read it.
     */
    bool in_quadrant_I() const
    {
        return 0 <= x && 0 <= y && is_not_unused();
    }

    /**
     * @brief Test whether a PixPoint has strictly positive x and y coords
     * @return true iff this PixPoint is not unused, both coords are positive
     *
     * We could rename this predicate as "in_interior_of_quadrant_I" (but let's
     * not do that).  The current name has the virtue of brevity; little else.
     */
    bool is_poz_poz() const
    {
        return 0 < x && 0 < y && is_not_unused();
    }

    // forward declaration
    struct Is_inbounds;

    /**
     * @brief return cross product of two PixPoints interpreted as 2-vectors.
     *
     * @param pp    other PixPoint (interpreted as 2-vector) to cross with
     *              this one.
     * @return magnitude of the resulting cross-product vector
     *
     * Actually we should call this the magnitude of the cross product, since
     * the cross product is really a vector product.  However, for vectors in
     * the X-Y plane, in a right-handed coordinate system, the direction is
     * always the Z direction, and this returns the magnitude.
     */
    Integer cross( const PixPoint& pp ) const
    {
        fault_if( is_unused() || pp.is_unused() );
        return x * pp.y - pp.x * y;
    }


    /// @brief return true iff this point lies on the same line as line(p1,p2).
    bool is_collinear( const PixPoint& p1, const PixPoint& p2 ) const
    {
        return 0 == operator-( p1 ).cross( operator-( p2 ) );
    }
};


/**
 * @brief Predicate functor tests whether a PixPoint is in a bounding box.
 *
 * Bounding box must be an axis-aligned rectangle.
 *
 * This is a particularly useful functor if you must verify a container full
 * of PixPoints is inbounds:  instantiate once, test on everyone.
 * There is example code here: @ref pathbb_ppoint.
 */
struct PixPoint::Is_inbounds : public std::unary_function< PixPoint, bool >
{

    const PixPoint  m_min_min,      ///< minimum in-bounds x and y coordinates
                    m_width_height; ///< dimensions of bounding box

    /**
     * @brief Establish the boundaries of the box:  LL corner and size
     *
     * @param min_min   The point just barely within bounds, with the smallest
     *                  legitimate x coordinate and y coordinate.
     * @param width_height  Width of the box as x coordinate, height of the
     *                      box as y coordinate.
     *
     * - min_min is just barely in bounds
     *      - For example, (min_min - PixPoint(1,0)) is out of bounds.
     * - the point min_min + width_height is just barely out of bounds.
     *      - For example, ( min_min + width_height - PixPoint(1,1) ) is
     *          inbounds provided width_height.x and width_height.y are
     *          positive.
     *
     * If either dimension of width_height is zero or negative, then the
     * predicate will always return false.
     */
    Is_inbounds( PixPoint min_min, PixPoint width_height )
    :   m_min_min( min_min ),
        m_width_height( width_height )
    {}

    /// @brief Predicate test:  is it inside the established boundaries?
    bool operator()( PixPoint querypoint ) const
    {
        PixPoint qp2 = querypoint - m_min_min;
        return qp2.in_quadrant_I() && (m_width_height - qp2).is_poz_poz();
    }

    /// @brief test whether this functor differs from another
    bool operator!=( const Is_inbounds& that_functor ) const
    {
        return      m_min_min != that_functor.m_min_min
                ||  m_width_height != that_functor.m_width_height;
    }
};


/// @brief convert the PixPoint to a floating point format
inline kjb::Vector2 to_vector2(const PixPoint& p)
{
    PixPoint::fault_if(p.is_unused());
    return kjb::Vector2(p.x, p.y);
}


/// @brief round to near integers, disregarding overrange values
inline PixPoint reckless_round(const kjb::Vector2& v)
{
    // It is reckless because the value might not fit in an integer,
    // and we don't even bother to check.
    return PixPoint(static_cast<PixPoint::Integer>(std::floor(v.x() + 0.5)),
                    static_cast<PixPoint::Integer>(std::floor(v.y() + 0.5)));
}

/// @brief truncate towards negative infinity, disregarding overrange values
inline PixPoint reckless_floor(const kjb::Vector2& v)
{
    // It is reckless because the value might not fit in an integer,
    // and we don't even bother to check.
    return PixPoint(static_cast<PixPoint::Integer>(std::floor(v.x())),
                    static_cast<PixPoint::Integer>(std::floor(v.y())));
}

/// @brief round the vector contents to integers, disregarding overrange values
inline PixPoint reckless_ceil(const kjb::Vector2& v)
{
    // It is reckless because the value might not fit in an integer,
    // and we don't even bother to check.
    return PixPoint(static_cast<PixPoint::Integer>(std::ceil(v.x())),
                    static_cast<PixPoint::Integer>(std::ceil(v.y())));
}




// The code for this function lives in pixpath.cpp (at this writing).
PixPoint str_to_PixPoint( std::istream& iss, const std::string& sep="," );

/// @brief See str_to_PixPoint( std::istream&, const std::string& ).
inline
PixPoint str_to_PixPoint( const std::string& spp, const std::string& sep="," )
{
    std::istringstream iss( spp );
    return str_to_PixPoint( iss, sep );
}

}   /* End of namespace qd block. */
}   /* End of namespace kjb block. */

namespace std
{


    /// @brief swap representations of two integer-valued XY pairs
    template<>
    inline void swap(
        kjb::qd::PixPoint& p1,
        kjb::qd::PixPoint& p2
    )
    {
        p1.swap( p2 );
    }
}

#endif
