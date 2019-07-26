/**
 * @file
 * @brief Implementation of the PixPath class
 * @author Andrew Predoehl
 */

/*
 * $Id: doublecircle.cpp 21596 2017-07-30 23:33:36Z kobus $
 */


#include "l/l_sys_debug.h"  /* For ASSERT */
#include "l/l_debug.h"
#include "l/l_sys_lib.h"
#include "l/l_sys_io.h"
#include "l/l_error.h"
#include "l/l_global.h"
#include "qd_cpp/doublecircle.h"

#include <sstream>
#include <l_cpp/l_util.h>


namespace
{

using kjb::qd::DoubleCircle;
using kjb::Vector2;

bool are_all_inside(
    const DoubleCircle* circ,
    std::vector< Vector2 >::const_iterator ibegin,
    std::vector< Vector2 >::const_iterator iend
)
{
    DoubleCircle lil_bigger( *circ );
    lil_bigger.radius *= 1.0 + 1e-6;
    while( ibegin != iend )
    {
        if ( lil_bigger.outside_me_is( *ibegin++ ) )
        {
            return false;
        }
    }
    return true;
}


void status( const DoubleCircle* circ, const Vector2& pt )
{
    KJB( ASSERT( circ ) );
    std::ostringstream stat;
#if 0
    stat << "Circle center (" << circ -> center.str() << "), "
            "R=" << circ -> radius << "; "
            "Point (" << pt.str() << ")  "
            "dist " << ( circ -> center - pt ).mag()
         << " from circle center\n";
    kjb_c::pso( "%s", stat.str().c_str() );
#else
    stat << "Circle center (" << circ -> center << "), "
            "R=" << circ -> radius << "; "
            "Point (" << pt << ")  "
            "dist " << ( circ -> center - pt ).magnitude()
         << " from circle center\n";
    kjb_c::pso( "%s", stat.str().c_str() );
#endif
}

DoubleCircle minidisc_with_2_points(
    std::vector< Vector2 >::const_iterator ibegin,
    std::vector< Vector2 >::const_iterator iend,
    const Vector2& pboundary2
)
{
    KJB( ASSERT( 0 < iend - ibegin ) );
    const Vector2 pboundary1 = *--iend;
    DoubleCircle result( pboundary1, pboundary2 );

    std::vector< Vector2 > pts( ibegin, iend );
    std::random_shuffle( pts.begin(), pts.end() );

    for( size_t iii = 0; iii < pts.size(); ++iii )
    {
        if ( result.outside_me_is( pts[ iii ] ) )
        {
            result = DoubleCircle( pboundary1, pboundary2, pts[ iii ] );
        }
    }
    return result;
}


DoubleCircle minidisc_with_point(
    std::vector< Vector2 >::const_iterator ibegin,
    std::vector< Vector2 >::const_iterator iend
)
{
    KJB( ASSERT( 2 <= iend - ibegin ) );
    const Vector2 pboundary = *--iend;
    std::vector< Vector2 > pts( ibegin, iend );
    std::random_shuffle( pts.begin(), pts.end() );
    DoubleCircle result( pts[ 0 ], pboundary );

    for( size_t iii = 1; iii < pts.size(); ++iii )
    {
        if ( result.outside_me_is( pts[ iii ] ) )
        {
            result = minidisc_with_2_points(
                            pts.begin(), pts.begin() + iii + 1, pboundary );
        }
    }
    return result;
}




} // end anonymous ns


namespace kjb
{
namespace qd
{



DoubleCircle::DoubleCircle(
    const Vector2& p1,
    const Vector2& p2,
    const Vector2& p3
)
:   radius( 0 ),
    center( p1 )
{
    if ( p1 == p2 && p2 == p3 )
    {
        return;
    }

    if ( p1 == p2 || p2 == p3 )
    {
        operator=( DoubleCircle( p1, p3 ) );
        return;
    }
    if ( p1 == p3 )
    {
        operator=( DoubleCircle( p1, p2 ) );
        return;
    }

    const Vector2   pmid12( ( p1 + p2 ) * 0.5 ),
                    pdif12( p2 - p1 ),
                    pdif3( pmid12 - p3 );

    double t = pdif12.magnitude_squared()/4.0 - pdif3.magnitude_squared();
    t *= 0.5 / ( pdif12.x() * pdif3.y() - pdif12.y() * pdif3.x() );

    center = pmid12;
    center.x() -= t * pdif12.y();
    center.y() += t * pdif12.x();
    radius = ( center - p1 ).magnitude();
}


DoubleCircle::DoubleCircle( const std::vector< Vector2 >& pts )
:   radius( 0 ),
    center( pts.empty() ? Vector2(0,0) : pts.front() )
{
    if (pts.empty()) return; // degenerate circle at origin in this case

    // Input might have duplicates, so search for a point differing from pts[0]
    std::vector< Vector2 >::const_iterator i1;
    for( i1 = pts.begin() + 1; i1 != pts.end() && *i1 == pts.front(); ++i1 )
    {
        ;
    }
    if ( i1 == pts.end() )
    {
        return; // degenerate circle at pts[0] in this case
    }
    operator=( DoubleCircle( pts.front(), *i1 ) );

    // Copy the two distinct points and remaining points, shuffle all but 2.
    std::vector< Vector2 > otherpts( i1 - 1, pts.end() );
    std::random_shuffle( otherpts.begin() + 2, otherpts.end() );

    for( size_t iii = 2; iii < otherpts.size(); ++iii )
    {
        if ( outside_me_is( otherpts[ iii ] ) )
        {
            operator=( minidisc_with_point(
                            otherpts.begin(), otherpts.begin() + iii + 1 ) );
        }
    }
}



} // end namespace qd
} // end namespace kjb

