/**
 * @file
 * @brief Contains definition for class DoubleCircle
 * @author Andrew Predoehl
 *
 * This not only supports a circle class with double type
 * center and radius (which is no big deal) but also implements
 * Welzl's randomized smallest-enclosing-disk algorithm, as
 * presented in de Berg, van Kreveld, Overmars, and Schwarzkopf,
 * Computational Geometry (Springer, 1999).
 */
/*
 * $Id: doublecircle.h 19808 2015-09-21 03:49:24Z predoehl $
 */

#ifndef DOUBLECIRCLE_H_UOFARIZONAVISION
#define DOUBLECIRCLE_H_UOFARIZONAVISION 1

#include <l/l_sys_sys.h>
#include <m_cpp/m_vector_d.h>
#include <qd_cpp/pixpoint.h>

#include <vector>


namespace kjb
{
namespace qd
{

/**
 * @brief Parameterized circle in the plane.
 */
struct DoubleCircle
{
    double radius;  ///< distance between center and points of the circle
    Vector2 center; ///< center of the circle

    /**
     * @brief ctor from a radius and center point
     *
     * @param rad   Radius of circle to construct (using its absolute value)
     * @param ctr   Center of circle to construct
     *
     * A negative radius is silently made positive.
     * A zero radius is OK but predicate is_degenerate() will be true.
     */
    DoubleCircle( double rad, const Vector2& ctr )
    :   radius( fabs( rad ) ),
        center( ctr )
    {}

    /// @brief ctor from 3 points
    DoubleCircle(
        const Vector2& p1,
        const Vector2& p2,
        const Vector2& p3
    );

    /// @brief ctor from points; OK if they are equal
    DoubleCircle( const Vector2& p1, const Vector2& p2 )
    :   radius( ( p1 - p2 ).magnitude() * 0.5 ),
        center( ( p1 + p2 ) * 0.5 )
    {}

    /// @brief swap representations of two circles
    void swap( DoubleCircle& that )
    {
        std::swap( radius, that.radius );
        center.swap( that.center );
    }

    /// @brief predicate to test for degeneracy
    bool is_degenerate() const
    {
        return      ! finite( radius )
                ||  radius <= 0
                ||  isnand( center.x() )
                ||  isnand( center.y() );
    }

    /// @brief predicate to test for non-degeneracy
    bool is_not_degenerate() const
    {
        return ! is_degenerate();
    }

    /**
     * @brief compute the smallest enclosing disc
     *
     * This algorithm is stochastic, and requires expected linear time.
     * It implements the VERY VERY CLEVER randomized algorithm of Welzl,
     * presented in de Berg, van Kreveld, Overmars, and Schwarzkopf,
     * Computational Geometry (Springer, 1999).
     */
    DoubleCircle( const std::vector< Vector2 >& );

    /**
     * @brief test whether a query point is outside this object's disc
     * @param query Point to test for nonmembership in the disc
     * @return true iff the query point is outside the disc (or that rounding
     *                  error makes it appear so.
     *
     * @warning due to roundoff error, points on the circle itself can produce
     * a true result sometimes.
     */
    bool outside_me_is( const Vector2& query ) const
    {
        return radius < ( query - center ).magnitude();
    }
};





}
}

namespace std
{

    /// @brief swap representations of two circles
    template<>
    inline void swap(
        kjb::qd::DoubleCircle& p1,
        kjb::qd::DoubleCircle& p2
    )
    {
        p1.swap( p2 );
    }

}



#endif /* DOUBLECIRCLE_H_UOFARIZONAVISION */
