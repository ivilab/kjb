/**
 * @file
 * @author Andrew Predoehl
 * @brief Support for points and line segments with rational coordinates
 */
/*
 * $Id: ratpoint.h 22174 2018-07-01 21:49:18Z kobus $
 */

#ifndef QD_CPP_RATPOINT_INCLUDED_IVILAB_UARIZONAVISION
#define QD_CPP_RATPOINT_INCLUDED_IVILAB_UARIZONAVISION 1

#include <l/l_sys_lib.h>
#include <l/l_sys_io.h>
#include <l/l_sys_debug.h>
#include <l_cpp/l_exception.h>
#include <l_cpp/l_util.h>
#include <qd_cpp/pixpoint.h>

#include <iosfwd>

#ifndef UNLIMITED_RATIONAL_PRECISION_QD_CPP_IVILAB
#define UNLIMITED_RATIONAL_PRECISION_QD_CPP_IVILAB 1
#endif

/* Kobus. Jan 13, 2017: I am having trouble with compiling this on my mac. 
*/
#ifdef MAC_OSX 
#include <iostream>
#ifdef UNLIMITED_RATIONAL_PRECISION_QD_CPP_IVILAB
#warning "Disabling UNLIMITED_RATIONAL_PRECISION_QD_CPP_IVILAB because it does not compile on macs."
#undef UNLIMITED_RATIONAL_PRECISION_QD_CPP_IVILAB
#endif
#endif 

#if UNLIMITED_RATIONAL_PRECISION_QD_CPP_IVILAB
#include <boost/multiprecision/cpp_int.hpp>
#else
#include <boost/rational.hpp>
#endif

namespace kjb
{
namespace qd
{


/// @brief very basic structure to represent X, Y points with rational coords.
struct RatPoint
{
#if UNLIMITED_RATIONAL_PRECISION_QD_CPP_IVILAB
    typedef boost::multiprecision::cpp_int      Rat_integral_t;
    typedef boost::multiprecision::cpp_rational Rat;
#else
    // PixPoint::Integer is TOO SMALL.  Even long long sometimes overflows.
    typedef long long                         Rat_integral_t;
    typedef boost::rational< Rat_integral_t > Rat;
#endif

    Rat x, y;

    /// @brief construct from one PixPoint (just copy it)
    RatPoint(const PixPoint& p)
    :   x(p.x),
        y(p.y)
    {}

    /// @brief construct from two rational cartesian coordinates
    RatPoint(const Rat& rx, const Rat& ry)
    :   x(rx),
        y(ry)
    {}

};


inline
RatPoint::Rat_integral_t i_numerator(const RatPoint::Rat& r)
{
#if UNLIMITED_RATIONAL_PRECISION_QD_CPP_IVILAB
    return boost::multiprecision::numerator(r);
#else
    return r.numerator();
#endif
}


inline
RatPoint::Rat_integral_t i_denominator(const RatPoint::Rat& r)
{
#if UNLIMITED_RATIONAL_PRECISION_QD_CPP_IVILAB
    return boost::multiprecision::denominator(r);
#else
    return r.denominator();
#endif
}



inline double dbl_ratio(const RatPoint::Rat& r)
{
#if UNLIMITED_RATIONAL_PRECISION_QD_CPP_IVILAB
    return r.convert_to<double>();
#else
    return i_numerator(r) / double(i_denominator(r));
#endif
}




/// @brief test equality of two RatPoints, totally unsurprising.
inline bool operator==(const RatPoint& a, const RatPoint& b)
{
    return a.x == b.x && a.y == b.y;
}

/// @brief test inequality of two RatPoints.
inline bool operator!=(const RatPoint& a, const RatPoint& b)
{
    return ! (a==b);
}

/// @brief "row-major" ordering of points
inline bool operator<(const RatPoint& a, const RatPoint& b)
{
    return a.y < b.y || (a.y==b.y && a.x < b.x);
}


/// @brief "row-major" ordering of points
inline bool operator<=(const RatPoint& a, const RatPoint& b)
{
    return a==b || a < b;
}


/// @brief subtraction of RatPoints as if they are position vectors
inline RatPoint operator-(const RatPoint& a, const RatPoint& b)
{
#if 0
    return RatPoint(a.x - b.x, a.y - b.y);
#else
    // apparently boost thinks this is not the same thing
    RatPoint::Rat dx(a.x - b.x), dy(a.y - b.y);
    return RatPoint(dx, dy);
#endif
}


/// @brief simple ascii putter
inline std::ostream& operator<<(std::ostream& s, const RatPoint& p)
{
    s << i_numerator(p.x);
    if (i_denominator(p.x) > 1) s << '/' << i_denominator(p.x);

    s << ", " << i_numerator(p.y);
    if (i_denominator(p.y) > 1) s << '/' << i_denominator(p.y);
    return s;
}


/// @brief basic line segment when endpoints are PixPoints (int coords)
struct PixPoint_line_segment
{
    PixPoint a, b;

    PixPoint_line_segment(const PixPoint& p, const PixPoint& q)
    :   a(p),
        b(q)
    {}
};


inline bool is_degenerate(const PixPoint_line_segment& s)
{
    return s.a == s.b;
}


/**
 * @brief test whether these segments lie on parallel lines or are collinear.
 *
 * Note a degenerate segment is parallel to every segment.
 */
inline
bool are_parallel(
    const PixPoint_line_segment& s,
    const PixPoint_line_segment& t
)
{
    // test if corresponding matrix is singular (zero determinant).
    return (s.b.x - s.a.x)*(t.a.y - t.b.y) == (s.b.y - s.a.y)*(t.a.x - t.b.x);
}


/**
 * @brief Test whether two closed line segments intersect
 * @return true iff the segments share a point (can be interior or endpoint)
 *
 * This will be a little faster than the rational version -- less math.
 */
bool is_intersecting(
    const PixPoint_line_segment&,
    const PixPoint_line_segment&
);



/// @brief test whether two segments share an infinity of common points
template <typename LINE_SEGMENT>
inline bool are_sharing_a_continuum(
    const LINE_SEGMENT& s,
    const LINE_SEGMENT& t
)
{
    return      !is_degenerate(s)
            &&  !is_degenerate(t)
            &&  are_parallel(s,t)
            &&  is_intersecting(s,t)
            &&  (   (is_on(s, t.a) && t.a != s.a && t.a != s.b) // t.a inside s
                ||  (is_on(s, t.b) && t.b != s.a && t.b != s.b) // t.b inside s
                ||  (is_on(t, s.a) && s.a != t.a && s.a != t.b) // s.a inside t
                ||  (is_on(t, s.b) && s.b != t.a && s.b != t.b) // s.b inside t
                ||  (t.a==s.a && t.b==s.b)                      // identical
                ||  (t.a==s.b && t.b==s.a)                      // reversed
                );
}


/// @brief if 2 segs intersect in a line segment, return it and true
template <typename LINE_SEGMENT>
inline bool are_sharing_a_continuum(
    const LINE_SEGMENT& s,
    const LINE_SEGMENT& t,
    LINE_SEGMENT* shared_continuum ///< output parameter
)
{
    if (    is_degenerate(s)
        ||  is_degenerate(t)
        ||  !are_parallel(s,t)
        ||  !is_intersecting(s,t)
       )
    {
        return false;
    }

    if ((t.a==s.a && t.b==s.b) || (t.a==s.b && t.b==s.a))
    {
        *shared_continuum = s;
        return true;
    }

    if (is_on(s, t.a) && t.a != s.a && t.a != s.b) // t.a inside s
    {
        if (is_on(t, s.a))
        {
            *shared_continuum = LINE_SEGMENT(s.a, t.a);
        }
        else if (is_on(t, s.b))
        {
            *shared_continuum = LINE_SEGMENT(t.a, s.b);
        }
        else
        {
            *shared_continuum = t;
        }
        return true;
    }

    if (is_on(s, t.b) && t.b != s.a && t.b != s.b) // t.b inside s
    {
        if (is_on(t, s.a))
        {
            *shared_continuum = LINE_SEGMENT(s.a, t.b);
        }
        else if (is_on(t, s.b))
        {
            *shared_continuum = LINE_SEGMENT(t.b, s.b);
        }
        else
        {
            *shared_continuum = t;
        }
        return true;
    }

    if (is_on(t, s.a) && s.a != t.a && s.a != t.b) // s.a inside t
    {
        if (is_on(s, t.a))
        {
            *shared_continuum = LINE_SEGMENT(s.a, t.a);
        }
        else if (is_on(s, t.b))
        {
            *shared_continuum = LINE_SEGMENT(s.a, t.b);
        }
        else
        {
            *shared_continuum = s;
        }
        return true;
    }
    if (is_on(t, s.b) && s.b != t.a && s.b != t.b) // s.b inside t
    {
        if (is_on(s, t.a))
        {
            *shared_continuum = LINE_SEGMENT(s.b, t.a);
        }
        else if (is_on(s, t.b))
        {
            *shared_continuum = LINE_SEGMENT(s.b, t.b);
        }
        else
        {
            *shared_continuum = s;
        }
        return true;
    }
    return false; // overlap only at the endpoint, not over a continuum
}



/// @brief closed line segment with rational coords
struct RatPoint_line_segment
{
    RatPoint a, b;

    RatPoint_line_segment(const RatPoint& p, const RatPoint& q)
    :   a(p),
        b(q)
    {}

    RatPoint_line_segment(const PixPoint_line_segment& s)
    :   a(s.a),
        b(s.b)
    {}
};


inline bool is_degenerate(const RatPoint_line_segment& s)
{
    return s.a == s.b;
}



inline double get_length(const RatPoint_line_segment& s)
{
    const RatPoint d(s.b.x - s.a.x, s.b.y - s.a.y);
    const RatPoint::Rat m2(d.x * d.x + d.y * d.y);
    return sqrt(dbl_ratio(m2));
}



RatPoint::Rat triangle_area(
    const RatPoint_line_segment&,
    const RatPoint&
);


bool is_on(const RatPoint_line_segment&, const RatPoint&);


bool is_intersecting(
    const RatPoint_line_segment&,
    const RatPoint_line_segment&
);


inline
bool are_parallel(
    const RatPoint_line_segment& s,
    const RatPoint_line_segment& t
)
{
    // test if corresponding matrix is singular (zero determinant).
    return (s.b.x - s.a.x)*(t.a.y - t.b.y) == (s.b.y - s.a.y)*(t.a.x - t.b.x);
}


/**
 * @brief find intersection point of nonparallel lines through these segments
 * @throws Illegal_argument if the segments are parallel
 * @returns intersection point of the lines (possibly off the segment).
 *
 * Note well that this applies to lines, not line segments, and this really
 * only works nicely for non-parallel lines; it throws an exception if the
 * inputs are parallel, which might be an unpleasant shock.  Note that if
 * either line segment input is degenerate, it is automatically parallel to
 * every other line, and will throw.
 */
RatPoint line_intersection(
    const RatPoint_line_segment&,
    const RatPoint_line_segment&
);


/**
 * @brief if segments intersect, return true and compute intersection
 *
 * @return true iff the segments have one or more common points.
 * @param s one segment to test (can be degenerate)
 * @param t other segment to test (can be degenerate)
 *
 * If the segments intersect, *intersection is set to the common points.
 * Therefore if the segments intersect at a unique point, then *intersection
 * is degenerate.
 * If the segments do not intersect this returns false, and *intersection
 * is not touched.
 *
 * @throws KJB_error if intersection is equal to NULL.
 */
inline bool segment_intersection(
    const RatPoint_line_segment& s,
    const RatPoint_line_segment& t,
    RatPoint_line_segment* intersection ///< output parameter
)
{
    NTX(intersection);
    if (!is_intersecting(s, t)) return false;

    if (is_degenerate(s))
    {
        *intersection = s;
    }
    else if (is_degenerate(t))
    {
        *intersection = t;
    }
    else if (are_parallel(s, t))
    {
        if (!are_sharing_a_continuum(s, t, intersection))
        {
            // not a continuum, just an endpoint
            if (s.a == t.a || s.a == t.b)
            {
                intersection -> a = intersection -> b = s.a;
            }
            else
            {
                KJB(ASSERT(s.b == t.a || s.b == t.b));
                intersection -> a = intersection -> b = s.b;
            }
        }
        // else intersection contains the shared continuum
    }
    else
    {
        intersection -> a = intersection -> b = line_intersection(s, t);
    }
    return true;
}


#if 0 /* maybe too trivial to commit */
/**
 * @brief test whether a point is collinear with the given segment.
 * @throws Illegal_argument if the line segment is degenerate
 * @returns true iff the query point is collinear (in line with the segment)
 *
 * Note that this tests whether the query point is on the line, not on the
 * line segment, and the segment thus should probably not be degenerate,
 * unless you like catching exceptions.
 */
inline bool line_intersection(
    const RatPoint_line_segment& s,
    const RatPoint& p
)
{
    if (is_degenerate(s)) KJB_THROW(Illegal_argument);
    return 0 == triangle_area(s, p);
}
#endif



RatPoint closest_point_on_seg(const RatPoint&, const RatPoint_line_segment&);



/// @brief really trivial convenience predicate -- true for degenerates also
inline
bool is_horizontal(const RatPoint_line_segment& s)
{
    return s.a.y == s.b.y;
}


/// @brief really trivial convenience predicate -- true for degenerates also
inline
bool is_vertical(const RatPoint_line_segment& s)
{
    return s.a.x == s.b.x;
}



} // end ns qd
} // end ns kjb

#endif /* QD_CPP_RATPOINT_INCLUDED_IVILAB_UARIZONAVISION */
