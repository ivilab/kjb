/**
 * @file
 * @author Andrew Predoehl
 * @brief Implementation for rational-coordinate points, line segments
 */
/*
 * $Id: ratpoint.cpp 21596 2017-07-30 23:33:36Z kobus $
 */

#include "l/l_sys_debug.h"  /* For ASSERT */
#include "l/l_sys_lib.h"
#include "l_cpp/l_util.h"
#include "qd_cpp/pixpath.h"
#include "qd_cpp/ratpoint.h"


namespace
{

using kjb::qd::RatPoint;


/**
 * @brief signed area of parallelogram def by a, b interp. as position vectors.
 *
 * Four vertices of parallelogram are in the XY plane, and are a, [0,0]T, b,
 * and a+b, disregarding the clockwise-ccw ordering of the vertices.
 *
 * @return signed area of parallelogram.  See notes about sign.
 *
 * If a or b is zero the result is zero.  If a or b as position vectors are
 * parallel, the result is zero.  In the following, assume neither is the case.
 * The result will be nonzero then.
 * One half-plane P is defined as the "left side" of vector a, i.e., the points
 * you would get by rotating a counter-clockwise from 0 to 180 degrees, and
 * scaling a by any positive value.
 * The other half-plane N is on the "right side" of vector a, including the
 * points you would get by rotating a clockwise from 0 to 180 degrees.
 *
 * If b lies in half-plane P ("left side" of a) the result is positive.
 * Otherwise it lies in N ("right side" of a) and the result is negative.
 */
inline RatPoint::Rat parallelogram_area(const RatPoint& a, const RatPoint& b)
{
    return a.x * b.y - a.y * b.x;
}


/// @brief dot product when a, b are interpreted as position vectors.
inline RatPoint::Rat dot_product(const RatPoint& a, const RatPoint& b)
{
    return a.x * b.x + a.y * b.y;
}



}




namespace kjb
{
namespace qd
{


bool is_intersecting(
    const PixPoint_line_segment& s,
    const PixPoint_line_segment& t
)
{
    if (s.a == t.a || s.a == t.b || s.b == t.a || s.b == t.b) return true;

    kjb::qd::PixPath p = kjb::qd::PixPath::reserve(4);
    p.push_back(s.a);
    p.push_back(s.b);
    p.push_back(t.a);
    p.push_back(t.b);
    return p.intersect_at_with(0, 2);
}


RatPoint line_intersection(
    const RatPoint_line_segment& s,
    const RatPoint_line_segment& t
)
{
    RatPoint::Rat a[4], ainv[4], b[2];

    // parametric equations of the system of lines: a u = b.
    // a describes the slopes of the lines,
    // b describes the spacing of their axis intercepts.
    // u is the vector of parameters, one per line

    a[0] = s.b.x - s.a.x;
    a[1] = t.a.x - t.b.x;
    a[2] = s.b.y - s.a.y;
    a[3] = t.a.y - t.b.y;

    // matrix a is nonsingular, right?  d is its determinant.
    const RatPoint::Rat d = a[0]*a[3] - a[1]*a[2];
    KJB(ASSERT(d != RatPoint::Rat(0)));

    // compute matrix inverse of a
    ainv[0] =  a[3] / d;
    ainv[1] = -a[1] / d;
#ifdef TEST
    ainv[2] = -a[2] / d; // not used in production
    ainv[3] =  a[0] / d; // not used in production

    // ainv * a equals the identity matrix, right?
    // this is exact, of course, because of the rational numeric type
    const RatPoint::Rat Z=0, U=1;
    KJB(ASSERT(ainv[0]*a[0] + ainv[1]*a[2] == U));
    KJB(ASSERT(ainv[0]*a[1] + ainv[1]*a[3] == Z));
    KJB(ASSERT(ainv[2]*a[0] + ainv[3]*a[2] == Z));
    KJB(ASSERT(ainv[2]*a[1] + ainv[3]*a[3] == U));
#endif

    b[0] = t.a.x - s.a.x;
    b[1] = t.a.y - s.a.y;

    // parameter value of intersection point, on parametric eq. of first line
    const RatPoint::Rat u = ainv[0]*b[0] + ainv[1]*b[1];

    return RatPoint(a[0]*u + s.a.x, a[2]*u + s.a.y);
}

/**
 * @brief find signed area of triangle defined by segment endpoints and apex.
 * @return signed area of triangle, possibly zero.  See notes below about sign.
 *
 * In case of a degenerate triangle (e.g., collinear legs) the result is zero.
 *
 * Suppose the triangle is not degenerate.  The sign of the result reveals the
 * ordering of the vertices.  If the vertices (s.a, s.b, apex) are listed in
 * counterclockwise order, the result is positive.  Clockwise order means
 * negative area.  Also the "clock" is laid out on a cartesian plane; if you
 * visualize the clock on using a traditional graphics or matrix layout, the
 * signs swap.
 */
RatPoint::Rat triangle_area(
    const RatPoint_line_segment& s,
    const RatPoint& apex
)
{
    return RatPoint::Rat(1,2) * parallelogram_area(s.b-s.a, apex-s.a);
}


/// @brief test whether a given point lies on a given segment
bool is_on(
    const RatPoint_line_segment& s, ///< test segment, could be degenerate
    const RatPoint& c               ///< test point
)
{
    if (c == s.a)
    {
        return true;
    }
    if (is_degenerate(s))
    {
        return false;
    }

    const RatPoint::Rat ZERO(0);
    const RatPoint ca(c - s.a), cb(c - s.b);

    return ZERO == parallelogram_area(ca, cb) && dot_product(ca, cb) <= ZERO;
}


bool is_intersecting(
    const RatPoint_line_segment& s,
    const RatPoint_line_segment& t
)
{
    // endpoint-endpoint intersection
    if (t.a == s.a || t.a == s.b || t.b == s.a || t.b == s.b)
    {
        return true;
    }
    // bounding-box quick reject
    if (    std::max(t.a.x, t.b.x) < std::min(s.a.x, s.b.x)
        ||  std::max(s.a.x, s.b.x) < std::min(t.a.x, t.b.x)
        ||  std::max(t.a.y, t.b.y) < std::min(s.a.y, s.b.y)
        ||  std::max(s.a.y, s.b.y) < std::min(t.a.y, t.b.y)
       )
    {
        return false; // bounding boxes are disjoint, so segments must be too.
    }
    // endpoint-interior intersection
    if (is_on(t, s.a) || is_on(t, s.b) || is_on(s, t.a) || is_on(s, t.b))
    {
        return true;
    }
    // if they are parallel and share a point we would have found it by now.
    if (are_parallel(s, t))
    {
        return false;
    }

    // standard test for crossing based on clockwise orientation
    // (e.g., Sedgewick Algorithms 2/e chap. 24, p.351.  CLR also has it.

    // In english:  consider two routes.
    // As you travel along S then go to T.a, you turn one way.
    // As you travel along S then go to T.b, you turn another way.
    // One should be a clockwise turn, the other counterclockwise.
    const bool  sta( parallelogram_area(s.b-s.a, t.a-s.a) > 0 ),
                stb( parallelogram_area(s.b-s.a, t.b-s.a) < 0 );
    if (sta != stb)
    {
        return false;
    }

    // Same as above but T and S swap roles.
    const bool  tsa( parallelogram_area(t.b-t.a, s.a-t.a) > 0 ),
                tsb( parallelogram_area(t.b-t.a, s.b-t.a) < 0 );
    if (tsa != tsb)
    {
        return false;
    }

    return true;
}



/**
 * @brief return point in domain closest to query point
 *
 * The domain is assumed to be a closed line segment.
 * The distance metric is Euclidean.
 */
RatPoint closest_point_on_seg(
    const RatPoint& query,
    const RatPoint_line_segment& domain
)
{
    if (is_degenerate(domain)) return domain.a; // no point is closer!

    const RatPoint dir = domain.b - domain.a;
    const RatPoint::Rat den = dir.x * dir.x + dir.y * dir.y,
                        xp = domain.a.x*domain.b.x + domain.a.y*domain.b.y,
                        am2 = domain.a.x*domain.a.x + domain.a.y*domain.a.y,
                        p = query.x * dir.x + query.y * dir.y,
                        t = (p - xp + am2) / den;

    /*
     * Is the nearest point in the segment's interior, or not?
     * First, test for "not":  parameter t has the answer.
     */
    if (t <= 0 || 1 <= t)
    {
        /*
         * Nearest point in domain is not in the segment interior.
         * Therefore return the nearest segment endpoint.
         */
        const RatPoint qa = query - domain.a,
                       qb = query - domain.b;
        const RatPoint::Rat dista2 = qa.x * qa.x + qa.y * qa.y,
                            distb2 = qb.x * qb.x + qb.y * qb.y;
        KJB(ASSERT(dista2 != distb2));
        return distb2 < dista2 ? domain.b : domain.a;
    }

    // Nearest point is within the segment interior.
    RatPoint c = domain.a;
    c.x *= 1 - t;
    c.y *= 1 - t;
    c.x += t * domain.b.x;
    c.y += t * domain.b.y;

    return c;
}



}
}

